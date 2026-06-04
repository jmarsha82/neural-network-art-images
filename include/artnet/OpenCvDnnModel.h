#pragma once

#include "artnet/ImageRecord.h"

#include <filesystem>
#include <memory>
#include <string>
#include <vector>

namespace artnet {

class OpenCvDnnModel {
public:
    OpenCvDnnModel(const std::filesystem::path& modelPath,
                   const std::filesystem::path& labelsPath);
    ~OpenCvDnnModel();

    OpenCvDnnModel(const OpenCvDnnModel&) = delete;
    OpenCvDnnModel& operator=(const OpenCvDnnModel&) = delete;
    OpenCvDnnModel(OpenCvDnnModel&&) noexcept;
    OpenCvDnnModel& operator=(OpenCvDnnModel&&) noexcept;

    [[nodiscard]] bool isLoaded() const noexcept;
    [[nodiscard]] std::vector<Prediction> classify(const std::filesystem::path& imagePath,
                                                   std::size_t topK = 5);
    [[nodiscard]] std::vector<float> embedding(const std::filesystem::path& imagePath);

private:
    std::vector<std::string> labels_;
    class Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace artnet
