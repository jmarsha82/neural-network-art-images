#pragma once

#include "artnet/ImageRecord.h"

#include <filesystem>
#include <vector>

namespace artnet {

class ArtScanner {
public:
    explicit ArtScanner(std::filesystem::path root);

    [[nodiscard]] std::vector<ImageRecord> scan() const;
    [[nodiscard]] const std::filesystem::path& root() const noexcept;

private:
    std::filesystem::path root_;
};

[[nodiscard]] bool isSupportedImageFile(const std::filesystem::path& path);
[[nodiscard]] std::string inferFolderLabel(const std::filesystem::path& imagePath,
                                           const std::filesystem::path& root);

} // namespace artnet
