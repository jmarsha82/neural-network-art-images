#include "artnet/Export.h"

#include <fstream>
#include <sstream>

namespace artnet {
namespace {

std::string csvEscape(const std::string& value) {
    if (value.find_first_of(",\"\n\r") == std::string::npos) {
        return value;
    }

    std::string escaped = "\"";
    for (char ch : value) {
        if (ch == '"') {
            escaped += "\"\"";
        } else {
            escaped += ch;
        }
    }
    escaped += '"';
    return escaped;
}

std::string jsonEscape(const std::string& value) {
    std::string escaped;
    for (char ch : value) {
        switch (ch) {
        case '\\':
            escaped += "\\\\";
            break;
        case '"':
            escaped += "\\\"";
            break;
        case '\n':
            escaped += "\\n";
            break;
        case '\r':
            escaped += "\\r";
            break;
        case '\t':
            escaped += "\\t";
            break;
        default:
            escaped += ch;
            break;
        }
    }
    return escaped;
}

} // namespace

std::string toCsv(const std::vector<ImageRecord>& records) {
    std::ostringstream out;
    out << "path,folder_label,top_prediction,top_confidence,cluster,embedding_dimensions\n";

    for (const auto& record : records) {
        const auto topLabel = record.predictions.empty() ? std::string{} : record.predictions.front().label;
        const auto topConfidence = record.predictions.empty() ? 0.0f : record.predictions.front().confidence;
        out << csvEscape(record.path.string()) << ','
            << csvEscape(record.folderLabel) << ','
            << csvEscape(topLabel) << ','
            << topConfidence << ','
            << record.cluster << ','
            << record.embedding.size() << '\n';
    }

    return out.str();
}

std::string toJson(const std::vector<ImageRecord>& records) {
    std::ostringstream out;
    out << "[\n";

    for (std::size_t index = 0; index < records.size(); ++index) {
        const auto& record = records[index];
        out << "  {\n"
            << "    \"path\": \"" << jsonEscape(record.path.string()) << "\",\n"
            << "    \"folderLabel\": \"" << jsonEscape(record.folderLabel) << "\",\n"
            << "    \"cluster\": " << record.cluster << ",\n"
            << "    \"embeddingDimensions\": " << record.embedding.size() << ",\n"
            << "    \"predictions\": [";

        for (std::size_t pred = 0; pred < record.predictions.size(); ++pred) {
            const auto& prediction = record.predictions[pred];
            out << "{\"label\":\"" << jsonEscape(prediction.label)
                << "\",\"confidence\":" << prediction.confidence << "}";
            if (pred + 1 < record.predictions.size()) {
                out << ", ";
            }
        }

        out << "]\n  }";
        if (index + 1 < records.size()) {
            out << ',';
        }
        out << '\n';
    }

    out << "]\n";
    return out.str();
}

void writeCsv(const std::filesystem::path& path, const std::vector<ImageRecord>& records) {
    std::ofstream out(path);
    out << toCsv(records);
}

void writeJson(const std::filesystem::path& path, const std::vector<ImageRecord>& records) {
    std::ofstream out(path);
    out << toJson(records);
}

} // namespace artnet
