#include "artnet/OpenCvDnnModel.h"

#include <opencv2/dnn.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>

#include <algorithm>
#include <fstream>
#include <numeric>
#include <stdexcept>

namespace artnet {

class OpenCvDnnModel::Impl {
public:
    cv::dnn::Net net;
};

namespace {

std::vector<std::string> readLabels(const std::filesystem::path& labelsPath) {
    std::ifstream input(labelsPath);
    std::vector<std::string> labels;
    std::string line;
    while (std::getline(input, line)) {
        if (!line.empty()) {
            labels.push_back(line);
        }
    }
    return labels;
}

cv::Mat imageBlob(const std::filesystem::path& imagePath) {
    auto image = cv::imread(imagePath.string(), cv::IMREAD_COLOR);
    if (image.empty()) {
        return {};
    }

    cv::Mat blob;
    cv::dnn::blobFromImage(image,
                           blob,
                           1.0 / 255.0,
                           cv::Size(224, 224),
                           cv::Scalar(0.485, 0.456, 0.406),
                           true,
                           false);
    return blob;
}

std::vector<float> flatten(const cv::Mat& output) {
    std::vector<float> values;
    values.reserve(static_cast<std::size_t>(output.total()));
    const auto* data = output.ptr<float>();
    for (std::size_t index = 0; index < output.total(); ++index) {
        values.push_back(data[index]);
    }
    return values;
}

} // namespace

OpenCvDnnModel::OpenCvDnnModel(const std::filesystem::path& modelPath,
                               const std::filesystem::path& labelsPath)
    : labels_(readLabels(labelsPath)),
      impl_(std::make_unique<Impl>()) {
    impl_->net = cv::dnn::readNetFromONNX(modelPath.string());
    impl_->net.setPreferableBackend(cv::dnn::DNN_BACKEND_OPENCV);
    impl_->net.setPreferableTarget(cv::dnn::DNN_TARGET_CPU);
}

OpenCvDnnModel::~OpenCvDnnModel() = default;
OpenCvDnnModel::OpenCvDnnModel(OpenCvDnnModel&&) noexcept = default;
OpenCvDnnModel& OpenCvDnnModel::operator=(OpenCvDnnModel&&) noexcept = default;

bool OpenCvDnnModel::isLoaded() const noexcept {
    return impl_ != nullptr && !impl_->net.empty();
}

std::vector<Prediction> OpenCvDnnModel::classify(const std::filesystem::path& imagePath,
                                                 std::size_t topK) {
    if (!isLoaded()) {
        return {};
    }

    auto blob = imageBlob(imagePath);
    if (blob.empty()) {
        return {};
    }

    impl_->net.setInput(blob);
    auto output = flatten(impl_->net.forward());
    if (output.empty()) {
        return {};
    }

    std::vector<std::size_t> order(output.size());
    std::iota(order.begin(), order.end(), 0);
    const auto count = std::min(topK, order.size());
    std::partial_sort(order.begin(), order.begin() + static_cast<std::ptrdiff_t>(count), order.end(), [&output](auto left, auto right) {
        return output[left] > output[right];
    });

    std::vector<Prediction> predictions;
    for (std::size_t i = 0; i < count; ++i) {
        const auto classIndex = order[i];
        Prediction prediction;
        prediction.label = classIndex < labels_.size() ? labels_[classIndex] : std::to_string(classIndex);
        prediction.confidence = output[classIndex];
        predictions.push_back(std::move(prediction));
    }
    return predictions;
}

std::vector<float> OpenCvDnnModel::embedding(const std::filesystem::path& imagePath) {
    if (!isLoaded()) {
        return {};
    }

    auto blob = imageBlob(imagePath);
    if (blob.empty()) {
        return {};
    }

    impl_->net.setInput(blob);
    return flatten(impl_->net.forward());
}

} // namespace artnet
