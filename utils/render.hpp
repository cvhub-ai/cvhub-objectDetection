// Ultralytics 🚀 AGPL-3.0 License - https://ultralytics.com/license

//
// Shared result rendering for the Ultralytics YOLO C++ examples. Draws the
// detections (or the semantic map) onto an image exactly like the Python
// Annotator and prints them to the console, so every example's `main` shares
// one identical render path. Header-only: add examples/cpp/common to the path.

#pragma once

#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include <opencv2/imgproc.hpp>

#include "utils/cli.hpp"
#include "utils/draw.hpp"
#include "types.hpp"

namespace yolo {

// Annotate `canvas` for `task` (drawing `results`, or `semantic` for semantic
// segmentation) and print each detection to stdout.
inline void RenderAndPrint(cv::Mat& canvas, Task task, const std::vector<Result>& results,
                           const std::vector<std::string>& names, const cv::Mat& semantic) {
    switch (task) {
        case Task::Detect:
        {
            for (const Result& r : results) {
                const std::string name = NameOf(names, r.class_id);
                DrawBox(canvas, r.box, Label(name, r.confidence), r.class_id);
                std::cout << name << " " << std::fixed << std::setprecision(2) << r.confidence << " box=["
                          << r.box.x << ", " << r.box.y << ", " << r.box.width << ", " << r.box.height << "]"
                          << std::endl;
            }
            break;
        }
        case Task::Classify: {
            int y = 28;
            for (const Result& r : results) {
                std::ostringstream label;
                label << NameOf(names, r.class_id) << " " << std::fixed << std::setprecision(2) << r.confidence;
                cv::putText(canvas, label.str(), {12, y}, cv::FONT_HERSHEY_SIMPLEX, 0.7, {0, 0, 0}, 3, cv::LINE_AA);
                cv::putText(canvas, label.str(), {12, y}, cv::FONT_HERSHEY_SIMPLEX, 0.7, {255, 255, 255}, 1, cv::LINE_AA);
                std::cout << label.str() << std::endl;
                y += 28;
            }
            break;
        }
        default:
            std::cerr << "[yolo] task '" << TaskName(task) << "' is not supported." << std::endl;
    }
}

}  // namespace yolo
