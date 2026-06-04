#pragma once

#include <filesystem>
#include <string>
#include <vector>

namespace artnet {

struct Prediction {
    std::string label;
    float confidence = 0.0f;
};

struct ImageRecord {
    std::filesystem::path path;
    std::string folderLabel;
    std::vector<float> embedding;
    std::vector<Prediction> predictions;
    int cluster = -1;
};

} // namespace artnet
