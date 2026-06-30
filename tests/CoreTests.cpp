#include "artnet/ArtScanner.h"
#include "artnet/Clusterer.h"
#include "artnet/EmbeddingIndex.h"
#include "artnet/Export.h"

#include <cmath>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

namespace {

void require(bool condition, const std::string& message) {
    if (!condition) {
        throw std::runtime_error(message);
    }
}

void requireNear(float actual, float expected, float epsilon, const std::string& message) {
    require(std::fabs(actual - expected) <= epsilon, message);
}

void testSupportedImages() {
    require(artnet::isSupportedImageFile("painting.JPG"), "JPG files should be supported");
    require(artnet::isSupportedImageFile("scan.webp"), "WEBP files should be supported");
    require(artnet::isSupportedImageFile("archival.tiff"), "TIFF files should be supported");
    require(!artnet::isSupportedImageFile("notes.txt"), "Text files should not be supported");
}

void testFolderLabelInference() {
    const std::filesystem::path root = "C:/Art";
    const std::filesystem::path image = "C:/Art/Abstract/blue.png";
    const std::filesystem::path rootImage = "C:/Art/cover.png";
    require(artnet::inferFolderLabel(image, root) == "Abstract", "Nested image should use parent folder label");
    require(artnet::inferFolderLabel(rootImage, root).empty(), "Root-level image should not infer a label");
}

void testScannerFiltersAndSortsImages() {
    const auto root = std::filesystem::temp_directory_path() / "artnet_core_tests";
    std::filesystem::remove_all(root);
    std::filesystem::create_directories(root / "Zed");
    std::filesystem::create_directories(root / "Alpha");

    const auto writeEmptyFile = [](const std::filesystem::path& path) {
        std::ofstream out(path);
        require(out.good(), "Test fixture file should be writable");
    };

    writeEmptyFile(root / "Zed" / "last.PNG");
    writeEmptyFile(root / "Alpha" / "first.jpg");
    writeEmptyFile(root / "Alpha" / "ignore.txt");

    const artnet::ArtScanner scanner(root);
    const auto records = scanner.scan();

    require(scanner.root() == root, "Scanner should expose configured root");
    require(records.size() == 2, "Scanner should keep only supported image files");
    require(records[0].path.filename() == "first.jpg", "Scanner should sort records by path");
    require(records[0].folderLabel == "Alpha", "Scanner should infer the folder label");

    std::filesystem::remove_all(root);
}

void testScannerHandlesMissingRoot() {
    const auto root = std::filesystem::temp_directory_path() / "artnet_missing_core_tests";
    std::filesystem::remove_all(root);
    const artnet::ArtScanner scanner(root);
    require(scanner.scan().empty(), "Missing roots should scan as empty libraries");
}

void testCosineSimilarity() {
    const auto same = artnet::cosineSimilarity({1.0f, 0.0f}, {1.0f, 0.0f});
    const auto opposite = artnet::cosineSimilarity({1.0f, 0.0f}, {0.0f, 1.0f});
    requireNear(same, 1.0f, 0.0001f, "Identical vectors should have cosine similarity 1");
    requireNear(opposite, 0.0f, 0.0001f, "Orthogonal vectors should have cosine similarity 0");
    requireNear(artnet::cosineSimilarity({}, {1.0f}), 0.0f, 0.0001f, "Empty vectors should score 0");
    requireNear(artnet::cosineSimilarity({1.0f}, {1.0f, 2.0f}), 0.0f, 0.0001f, "Mismatched dimensions should score 0");
    requireNear(artnet::cosineSimilarity({0.0f, 0.0f}, {1.0f, 2.0f}), 0.0f, 0.0001f, "Zero-norm vectors should score 0");
}

void testFindSimilarOrdersByScore() {
    std::vector<artnet::ImageRecord> records(3);
    records[0].embedding = {1.0f, 0.0f};
    records[1].embedding = {0.9f, 0.1f};
    records[2].embedding = {0.0f, 1.0f};

    artnet::EmbeddingIndex index(std::move(records));
    const auto results = index.findSimilar(0, 2);
    require(results.size() == 2, "findSimilar should cap results to the requested count");
    require(results[0].recordIndex == 1, "Most similar record should come first");
    require(results[1].recordIndex == 2, "Less similar record should come second");
}

void testFindSimilarHandlesEmptyAndInvalidQueries() {
    std::vector<artnet::ImageRecord> records(2);
    records[1].embedding = {1.0f, 0.0f};

    artnet::EmbeddingIndex index(std::move(records));
    require(index.records().size() == 2, "EmbeddingIndex should retain records");
    require(index.findSimilar(0, 5).empty(), "Empty query embeddings should return no matches");

    bool threw = false;
    try {
        (void)index.findSimilar(5, 1);
    } catch (const std::out_of_range&) {
        threw = true;
    }
    require(threw, "Out-of-range query indices should throw");
}

void testClustererAssignsClusters() {
    std::vector<artnet::ImageRecord> records(3);
    records[0].embedding = {0.0f, 0.0f};
    records[1].embedding = {0.1f, 0.0f};
    records[2].embedding = {10.0f, 10.0f};

    artnet::Clusterer(2).assignClusters(records);
    require(records[0].cluster >= 0, "First embedded record should be clustered");
    require(records[1].cluster >= 0, "Second embedded record should be clustered");
    require(records[2].cluster >= 0, "Third embedded record should be clustered");
}

void testClustererSkipsRecordsWithoutEmbeddings() {
    std::vector<artnet::ImageRecord> records(2);
    records[1].embedding = {1.0f, 1.0f};

    artnet::Clusterer(0).assignClusters(records);
    require(records[0].cluster == -1, "Records without embeddings should keep their cluster");
    require(records[1].cluster == 0, "Cluster count should clamp to at least one");
}

void testExportIncludesPredictions() {
    artnet::ImageRecord record;
    record.path = "C:/Art/a,b.png";
    record.folderLabel = "Abstract";
    record.predictions.push_back({"painting", 0.75f});
    record.cluster = 3;
    record.embedding = {1.0f, 2.0f};

    const auto csv = artnet::toCsv({record});
    const auto json = artnet::toJson({record});
    require(csv.find("\"C:/Art/a,b.png\"") != std::string::npos, "CSV should quote paths containing commas");
    require(json.find("\"label\":\"painting\"") != std::string::npos, "JSON should include prediction labels");
}

void testExportEscapesValuesAndWritesFiles() {
    const auto root = std::filesystem::temp_directory_path() / "artnet_export_tests";
    std::filesystem::remove_all(root);
    std::filesystem::create_directories(root);

    artnet::ImageRecord record;
    record.path = root / "quote\"newline\n.png";
    record.folderLabel = "Line\nBreak";
    record.predictions.push_back({"quoted \"label\"", 0.5f});

    const auto csvPath = root / "records.csv";
    const auto jsonPath = root / "records.json";
    artnet::writeCsv(csvPath, {record});
    artnet::writeJson(jsonPath, {record});

    const auto csv = artnet::toCsv({record});
    const auto json = artnet::toJson({record});
    require(csv.find("\"\"newline") != std::string::npos, "CSV should escape embedded quotes");
    require(json.find("quoted \\\"label\\\"") != std::string::npos, "JSON should escape embedded quotes");
    require(std::filesystem::file_size(csvPath) > 0, "writeCsv should create a non-empty file");
    require(std::filesystem::file_size(jsonPath) > 0, "writeJson should create a non-empty file");

    std::filesystem::remove_all(root);
}

} // namespace

int main() {
    try {
        testSupportedImages();
        testFolderLabelInference();
        testScannerFiltersAndSortsImages();
        testScannerHandlesMissingRoot();
        testCosineSimilarity();
        testFindSimilarOrdersByScore();
        testFindSimilarHandlesEmptyAndInvalidQueries();
        testClustererAssignsClusters();
        testClustererSkipsRecordsWithoutEmbeddings();
        testExportIncludesPredictions();
        testExportEscapesValuesAndWritesFiles();

        std::cout << "All core tests passed\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "Test failed: " << error.what() << '\n';
        return 1;
    }
}
