// Ultralytics 🚀 AGPL-3.0 License - https://ultralytics.com/license

//
// Shared drawing helper for the Ultralytics YOLO C++ examples. Mirrors the
// Python Annotator.box_label() so every example annotates detections identically:
// a box plus a filled label with contrasting text. Header-only: add
// examples/cpp/common to your include path and `#include "yolo_draw.hpp"`.

#pragma once

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <string>
#include <vector>
#include <opencv2/imgproc.hpp>

namespace yolo {

// Ultralytics color palette (https://docs.ultralytics.com/), returned as BGR for OpenCV.
// Indexed by class id so each class always gets the same, distinct color. Mirrors
// ultralytics.utils.plotting.Colors (hex palette), converted to BGR.
inline cv::Scalar Color(int class_id) {
    static const std::vector<cv::Scalar> palette = {
        cv::Scalar(0xFF, 0x2A, 0x04), cv::Scalar(0xEB, 0xDB, 0x0B), cv::Scalar(0xF3, 0xF3, 0xF3),
        cv::Scalar(0xB7, 0xDF, 0x00), cv::Scalar(0x68, 0x1F, 0x11), cv::Scalar(0xDD, 0x6F, 0xFF),
        cv::Scalar(0x4F, 0x44, 0xFF), cv::Scalar(0x00, 0xED, 0xCC), cv::Scalar(0x44, 0xF3, 0x00),
        cv::Scalar(0xFF, 0x00, 0xBD), cv::Scalar(0xFF, 0xB4, 0x00), cv::Scalar(0xBA, 0x00, 0xDD),
        cv::Scalar(0xFF, 0xFF, 0x00), cv::Scalar(0x00, 0xC0, 0x26), cv::Scalar(0xB3, 0xFF, 0x01),
        cv::Scalar(0xFF, 0x24, 0x7D), cv::Scalar(0x68, 0x00, 0x7B), cv::Scalar(0x6C, 0x1B, 0xFF),
        cv::Scalar(0x2F, 0x6D, 0xFC), cv::Scalar(0x0B, 0xFF, 0xA2),
    };
    return palette[std::abs(class_id) % static_cast<int>(palette.size())];
}

// Text color for a given box color, mirroring Annotator.get_txt_color():
// a fixed set of light palette colors use dark text; everything else uses white.
inline cv::Scalar TextColor(const cv::Scalar& bg) {
    // "dark_colors" in plotting.py: backgrounds that read better with dark text (BGR).
    static const int dark_bgr[][3] = {
        {235, 219, 11}, {243, 243, 243}, {183, 223, 0}, {221, 111, 255}, {0, 237, 204},
        {68, 243, 0}, {255, 255, 0}, {179, 255, 1}, {11, 255, 162},
    };
    const int b = static_cast<int>(bg[0]), g = static_cast<int>(bg[1]), r = static_cast<int>(bg[2]);
    for (const auto& c : dark_bgr) {
        if (c[0] == b && c[1] == g && c[2] == r) return cv::Scalar(104, 31, 17);
    }
    return cv::Scalar(255, 255, 255);
}

// Line width derived from image size, matching Annotator's default `lw`.
inline int LineWidth(const cv::Mat& image) {
    return std::max(static_cast<int>(std::round((image.rows + image.cols + image.channels()) / 2.0 * 0.003)), 2);
}

// Draw a detection box for `class_id` (color from the Ultralytics palette) and an
// optional filled label, exactly like Annotator.box_label() in the Python package.
inline void DrawBox(cv::Mat& image, const cv::Rect& box, const std::string& label, int class_id) {
    const cv::Scalar color = Color(class_id);
    const int lw = LineWidth(image);
    const double sf = lw / 3.0;       // font scale
    const int tf = std::max(lw - 1, 1);  // font thickness

    cv::Point p1(box.x, box.y);
    cv::rectangle(image, p1, cv::Point(box.x + box.width, box.y + box.height), color, lw, cv::LINE_AA);

    if (label.empty()) return;

    int baseline = 0;
    const cv::Size ts = cv::getTextSize(label, cv::FONT_HERSHEY_SIMPLEX, sf, tf, &baseline);
    const int w = ts.width;
    const int h = ts.height + 3;  // pad text height, like the Python code
    const bool outside = p1.y >= h;
    if (p1.x > image.cols - w) p1.x = image.cols - w;  // keep the label on-screen

    const cv::Point p2(p1.x + w, outside ? p1.y - h : p1.y + h);
    cv::rectangle(image, p1, p2, color, cv::FILLED, cv::LINE_AA);
    cv::putText(image, label, cv::Point(p1.x, outside ? p1.y - 2 : p1.y + h - 1),
                cv::FONT_HERSHEY_SIMPLEX, sf, TextColor(color), tf, cv::LINE_AA);
}

// Format a "<name> <conf>" label the same way the Python package does (2 decimals).
inline std::string Label(const std::string& name, float confidence) {
    int pct = static_cast<int>(std::round(confidence * 100.0f));
    return name + " " + std::to_string(pct / 100) + "." + (pct % 100 < 10 ? "0" : "") + std::to_string(pct % 100);
}

// Blend an instance mask (8-bit, image size) over the image in the class color.
inline void DrawMask(cv::Mat& image, const cv::Mat& mask, int class_id, float alpha = 0.5f) {
    if (mask.empty()) return;
    const cv::Scalar color = Color(class_id);
    cv::Mat colored(image.size(), CV_8UC3, color);
    cv::Mat blended;
    cv::addWeighted(image, 1.0f - alpha, colored, alpha, 0.0, blended);
    blended.copyTo(image, mask);
}

}  // namespace yolo
