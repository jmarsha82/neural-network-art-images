#include "artnet/ArtScanner.h"

#include <algorithm>
#include <cctype>

namespace artnet {
namespace {

std::string lower(std::string value) {
    std::ranges::transform(value, value.begin(), [](unsigned char ch) {
        return static_cast<char>(std::tolower(ch));
    });
    return value;
}

} // namespace

ArtScanner::ArtScanner(std::filesystem::path root)
    : root_(std::move(root)) {}

std::vector<ImageRecord> ArtScanner::scan() const {
    std::vector<ImageRecord> records;
    if (!std::filesystem::exists(root_)) {
        return records;
    }

    for (const auto& entry : std::filesystem::recursive_directory_iterator(root_)) {
        if (!entry.is_regular_file() || !isSupportedImageFile(entry.path())) {
            continue;
        }

        ImageRecord record;
        record.path = entry.path();
        record.folderLabel = inferFolderLabel(entry.path(), root_);
        records.push_back(std::move(record));
    }

    std::ranges::sort(records, [](const ImageRecord& left, const ImageRecord& right) {
        return left.path.string() < right.path.string();
    });
    return records;
}

const std::filesystem::path& ArtScanner::root() const noexcept {
    return root_;
}

bool isSupportedImageFile(const std::filesystem::path& path) {
    const auto ext = lower(path.extension().string());
    return ext == ".jpg" || ext == ".jpeg" || ext == ".png" || ext == ".bmp" ||
           ext == ".webp" || ext == ".tif" || ext == ".tiff";
}

std::string inferFolderLabel(const std::filesystem::path& imagePath,
                             const std::filesystem::path& root) {
    const auto parent = imagePath.parent_path();
    if (parent.empty() || parent == root) {
        return {};
    }

    return parent.filename().string();
}

} // namespace artnet
