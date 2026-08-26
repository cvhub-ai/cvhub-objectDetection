#include "pipeline/inference.h"

#include <array>
#include <filesystem>
#include <iostream>
#include <regex>

#include <cstdint>
#include <cstring>

#include "coco_names.hpp"
#include "pipeline/preprocess.hpp"
#include "logger.h"

namespace yolo {

// IEEE 754 half-precision conversions (FP16 models store input/output as float16).
uint16_t FloatToHalf(float value) {
    uint32_t x;
    std::memcpy(&x, &value, sizeof(x));
    const uint32_t sign = (x >> 16) & 0x8000u;
    int32_t exponent = static_cast<int32_t>((x >> 23) & 0xffu) - 127 + 15;
    const uint32_t mantissa = x & 0x7fffffu;
    if (exponent >= 0x1f) return static_cast<uint16_t>(sign | 0x7c00u);  // overflow -> inf
    if (exponent <= 0) return static_cast<uint16_t>(sign);               // underflow -> 0
    return static_cast<uint16_t>(sign | (static_cast<uint32_t>(exponent) << 10) | (mantissa >> 13));
}

float HalfToFloat(uint16_t h) {
    const uint32_t sign = (h & 0x8000u) << 16;
    uint32_t exponent = (h >> 10) & 0x1fu;
    uint32_t mantissa = h & 0x3ffu;
    uint32_t bits;
    if (exponent == 0) {
        if (mantissa == 0) {
            bits = sign;
        } else {
            exponent = 127 - 15 + 1;
            while ((mantissa & 0x400u) == 0) { mantissa <<= 1; --exponent; }
            mantissa &= 0x3ffu;
            bits = sign | (exponent << 23) | (mantissa << 13);
        }
    } else if (exponent == 0x1f) {
        bits = sign | 0x7f800000u | (mantissa << 13);
    } else {
        bits = sign | ((exponent - 15 + 127) << 23) | (mantissa << 13);
    }
    float out;
    std::memcpy(&out, &bits, sizeof(out));
    return out;
}

// Return an output tensor as float, converting from float16 into `storage` if needed.
const float* OutputAsFloat(const Ort::Value& value, std::vector<float>& storage) {
    auto info = value.GetTensorTypeAndShapeInfo();
    if (info.GetElementType() == ONNX_TENSOR_ELEMENT_DATA_TYPE_FLOAT16) {
        const size_t count = info.GetElementCount();
        const uint16_t* half = value.GetTensorData<uint16_t>();
        storage.resize(count);
        for (size_t i = 0; i < count; ++i) storage[i] = HalfToFloat(half[i]);
        return storage.data();
    }
    return value.GetTensorData<float>();
}

 // namespace

Predictor::Predictor(const Config& config) : config_(config) {
    Logger& logger = Logger::getInstance();
    logger.setOutputFile("log.txt");
    LOG_INFO("Loading onnx model ");
    session_options_.SetGraphOptimizationLevel(GraphOptimizationLevel::ORT_ENABLE_ALL);
    session_options_.SetIntraOpNumThreads(1);
#ifdef USE_CUDA
    if (config_.cuda) {
        OrtCUDAProviderOptions cuda_options;
        cuda_options.device_id = 0;
        session_options_.AppendExecutionProvider_CUDA(cuda_options);
    }
#endif

    const std::filesystem::path model_path = std::filesystem::u8path(config_.model_path);
    session_ = new Ort::Session(env_, model_path.c_str(), session_options_);

    Ort::AllocatorWithDefaultOptions allocator;
    for (size_t i = 0; i < session_->GetInputCount(); ++i) {
        input_name_storage_.emplace_back(session_->GetInputNameAllocated(i, allocator).get());
    }
    for (size_t i = 0; i < session_->GetOutputCount(); ++i) {
        output_name_storage_.emplace_back(session_->GetOutputNameAllocated(i, allocator).get());
    }
    for (const auto& s : input_name_storage_) input_names_.push_back(s.c_str());
    for (const auto& s : output_name_storage_) output_names_.push_back(s.c_str());

    // Detect a half-precision (FP16) model from its input element type.
    input_fp16_ = session_->GetInputTypeInfo(0).GetTensorTypeAndShapeInfo().GetElementType() ==
                  ONNX_TENSOR_ELEMENT_DATA_TYPE_FLOAT16;

    load_metadata(allocator);
}

Predictor::~Predictor() { delete session_; }

void Predictor::load_metadata(Ort::AllocatorWithDefaultOptions& allocator) {
    Ort::ModelMetadata metadata = session_->GetModelMetadata();

    auto task_meta = metadata.LookupCustomMetadataMapAllocated("task", allocator);
    task_ = task_meta ? TaskFromString(task_meta.get()) : Task::Detect;

    auto imgsz_meta = metadata.LookupCustomMetadataMapAllocated("imgsz", allocator);  // e.g. "[640, 640]"
    if (imgsz_meta) {
        std::string s = imgsz_meta.get();
        std::smatch m;
        if (std::regex_search(s, m, std::regex("\\d+"))) imgsz_ = std::stoi(m.str());
    }

    auto names_meta = metadata.LookupCustomMetadataMapAllocated("names", allocator);  // "{0: 'person', ...}"
    names_.clear();
    if (names_meta) {
        std::string s = names_meta.get();
        std::regex item("(\\d+)\\s*:\\s*['\"]([^'\"]*)['\"]");
        for (std::sregex_iterator it(s.begin(), s.end(), item), end; it != end; ++it) {
            int idx = std::stoi((*it)[1].str());
            if (static_cast<int>(names_.size()) <= idx) names_.resize(idx + 1);
            names_[idx] = (*it)[2].str();
        }
    }
    if (names_.empty()) names_ = CocoNames();
}

std::vector<Result> yolo::Predictor::PostprocessDetect(const float* data, const std::vector<int64_t>& shape,
                                             float scale, float conf_thr, float iou_thr) 
{
    std::vector<Result> results;
    const float inv = 1.0f / scale;
    const int dim1 = static_cast<int>(shape[1]);
    const int dim2 = static_cast<int>(shape[2]);
    const bool end2end = dim1 > dim2;

    if (end2end) {  // [1, 300, 6] = x1,y1,x2,y2,conf,cls (already NMS'd)
        for (int i = 0; i < dim1; ++i) {
            const float* row = data + i * dim2;
            const float conf = row[4];
            if (conf < conf_thr) continue;
            Result r;
            r.class_id = static_cast<int>(row[5]);
            r.confidence = conf;
            r.box = cv::Rect(static_cast<int>(row[0] * inv), static_cast<int>(row[1] * inv),
                             static_cast<int>((row[2] - row[0]) * inv), static_cast<int>((row[3] - row[1]) * inv));
            results.push_back(r);
        }
        return results;
    }

    // Grid [1, 4+nc, 8400]: index the channel-major buffer directly (avoids a full
    // transpose copy and a per-anchor cv::minMaxLoc) and argmax over the class scores.
    const int nc = dim1 - 4;
    const int a = dim2;
    std::vector<cv::Rect> boxes;
    std::vector<float> confidences;
    std::vector<int> classIds;
    for (int i = 0; i < a; ++i) {
        int best = 0;
        float best_score = data[4 * a + i];
        for (int c = 1; c < nc; ++c) {
            const float s = data[(4 + c) * a + i];
            if (s > best_score) { best_score = s; best = c; }
        }
        if (best_score < conf_thr) continue;
        const float cx = data[i], cy = data[a + i], w = data[2 * a + i], h = data[3 * a + i];
        boxes.emplace_back(static_cast<int>((cx - 0.5f * w) * inv), static_cast<int>((cy - 0.5f * h) * inv),
                           static_cast<int>(w * inv), static_cast<int>(h * inv));
        confidences.push_back(best_score);
        classIds.push_back(best);
    }
    std::vector<int> keep;
    cv::dnn::NMSBoxes(boxes, confidences, conf_thr, iou_thr, keep);
    for (int idx : keep) {
        Result r;
        r.class_id = classIds[idx];
        r.confidence = confidences[idx];
        r.box = boxes[idx];
        results.push_back(r);
    }
    return results;
}

std::vector<Result> Predictor::predict(const cv::Mat& image, cv::Mat& semantic) {
    float scale = 1.0f;
    Logger& logger = Logger::getInstance();
    logger.setOutputFile("log.txt");
    LOG_INFO("Preprocessing image for prediction");
    cv::Mat input = Preprocess(image, imgsz_, task_ == Task::Classify, scale);
    std::vector<float> blob = ToBlob(input, imgsz_);

    std::array<int64_t, 4> input_shape{1, 3, imgsz_, imgsz_};
    Ort::MemoryInfo memory = Ort::MemoryInfo::CreateCpu(OrtDeviceAllocator, OrtMemTypeCPU);

    // Feed FP32 directly, or convert the blob to FP16 for half-precision models.
    std::vector<uint16_t> half_blob;
    Ort::Value input_tensor{nullptr};
    if (input_fp16_) {
        half_blob.resize(blob.size());
        for (size_t i = 0; i < blob.size(); ++i) half_blob[i] = FloatToHalf(blob[i]);
        input_tensor = Ort::Value::CreateTensor(memory, half_blob.data(), half_blob.size() * sizeof(uint16_t),
                                                input_shape.data(), input_shape.size(),
                                                ONNX_TENSOR_ELEMENT_DATA_TYPE_FLOAT16);
    } else {
        input_tensor = Ort::Value::CreateTensor<float>(memory, blob.data(), blob.size(),
                                                       input_shape.data(), input_shape.size());
    }

    auto outputs = session_->Run(run_options_, input_names_.data(), &input_tensor, 1,
                                 output_names_.data(), output_names_.size());

    // Primary output (2D/3D) plus the 4D proto/semantic tensor when present.
    int main_idx = 0, aux_idx = -1;
    for (size_t i = 0; i < outputs.size(); ++i) {
        if (outputs[i].GetTensorTypeAndShapeInfo().GetShape().size() == 4) aux_idx = static_cast<int>(i);
        else main_idx = static_cast<int>(i);
    }
    std::vector<int64_t> shape = outputs[main_idx].GetTensorTypeAndShapeInfo().GetShape();
    std::vector<float> main_store, aux_store;  // backing storage for FP16 -> float conversion
    const float* data = OutputAsFloat(outputs[main_idx], main_store);

    return PostprocessDetect(data, shape, scale, config_.conf, config_.iou);
}

}  // namespace yolo