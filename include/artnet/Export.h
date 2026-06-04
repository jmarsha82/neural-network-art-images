#pragma once

#include "artnet/ImageRecord.h"

#include <filesystem>
#include <string>
#include <vector>

namespace artnet {

[[nodiscard]] std::string toCsv(const std::vector<ImageRecord>& records);
[[nodiscard]] std::string toJson(const std::vector<ImageRecord>& records);

void writeCsv(const std::filesystem::path& path, const std::vector<ImageRecord>& records);
void writeJson(const std::filesystem::path& path, const std::vector<ImageRecord>& records);

} // namespace artnet
