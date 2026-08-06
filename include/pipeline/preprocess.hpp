#pragma once

#include <algorithm>
#include <cmath>
#include <vector>
#include <opencv2/opencv.hpp>


namespace yolo {

inline cv::Mat Preprocess(const cv::Mat& image, int imgsz, bool classify, float& scale) {
    if (classify) {
        scale = 1.0f;
        const float r = imgsz / static_cast<float>(std::min(image.cols, image.rows));
        const int rw = static_cast<int>(std::round(image.cols * r));
        const int rh = static_cast<int>(std::round(image.rows * r));
        cv::Mat resized;
        cv::resize(image, resized, cv::Size(rw, rh), 0, 0, cv::INTER_AREA);
        cv::Mat crop = resized(cv::Rect((rw - imgsz) / 2, (rh - imgsz) / 2, imgsz, imgsz)).clone();
        cv::cvtColor(crop, crop, cv::COLOR_BGR2RGB);
        return crop;
    }
    scale = std::min(imgsz / static_cast<float>(image.cols), imgsz / static_cast<float>(image.rows));
    const int new_w = static_cast<int>(std::round(image.cols * scale));
    const int new_h = static_cast<int>(std::round(image.rows * scale));
    cv::Mat resized;
    cv::resize(image, resized, cv::Size(new_w, new_h));
    cv::Mat out = cv::Mat::zeros(imgsz, imgsz, CV_8UC3);  // top-left letterbox, pad bottom/right
    resized.copyTo(out(cv::Rect(0, 0, new_w, new_h)));
    cv::cvtColor(out, out, cv::COLOR_BGR2RGB);
    return out;
}

// HWC uint8 (RGB) -> CHW float32 normalized to [0, 1].
inline std::vector<float> ToBlob(const cv::Mat& rgb, int imgsz) {
    const int area = imgsz * imgsz;
    std::vector<float> blob(3 * area);
    for (int h = 0; h < imgsz; ++h) {
        for (int w = 0; w < imgsz; ++w) {
            const cv::Vec3b& px = rgb.at<cv::Vec3b>(h, w);
            for (int c = 0; c < 3; ++c) blob[c * area + h * imgsz + w] = px[c] / 255.0f;
        }
    }
    return blob;
}

}