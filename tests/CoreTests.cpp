#include "artnet/ArtScanner.h"
#include "artnet/Clusterer.h"
#include "artnet/EmbeddingIndex.h"
#include "artnet/Export.h"

#include <cassert>
#include <cmath>
#include <filesystem>
#include <iostream>
#include <vector>

namespace {

void testSupportedImages() {
    assert(artnet::isSupportedImageFile("painting.JPG"));
    assert(artnet::isSupportedImageFile("scan.webp"));
    assert(!artnet::isSupportedImageFile("notes.txt"));
}

void testFolderLabelInference() {
    const std::filesystem::path root = "C:/Art";
    const std::filesystem::path image = "C:/Art/Abstract/blue.png";
    assert(artnet::inferFolderLabel(image, root) == "Abstract");
}

void testCosineSimilarity() {
    const auto same = artnet::cosineSimilarity({1.0f, 0.0f}, {1.0f, 0.0f});
    const auto opposite = artnet::cosineSimilarity({1.0f, 0.0f}, {0.0f, 1.0f});
    assert(std::fabs(same - 1.0f) < 0.0001f);
    assert(std::fabs(opposite) < 0.0001f);
}

void testFindSimilarOrdersByScore() {
    std::vector<artnet::ImageRecord> records(3);
    records[0].embedding = {1.0f, 0.0f};
    records[1].embedding = {0.9f, 0.1f};
    records[2].embedding = {0.0f, 1.0f};

    artnet::EmbeddingIndex index(std::move(records));
    const auto results = index.findSimilar(0, 2);
    assert(results.size() == 2);
    assert(results[0].recordIndex == 1);
    assert(results[1].recordIndex == 2);
}

void testClustererAssignsClusters() {
    std::vector<artnet::ImageRecord> records(3);
    records[0].embedding = {0.0f, 0.0f};
    records[1].embedding = {0.1f, 0.0f};
    records[2].embedding = {10.0f, 10.0f};

    artnet::Clusterer(2).assignClusters(records);
    assert(records[0].cluster >= 0);
    assert(records[1].cluster >= 0);
    assert(records[2].cluster >= 0);
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
    assert(csv.find("\"C:/Art/a,b.png\"") != std::string::npos);
    assert(json.find("\"label\":\"painting\"") != std::string::npos);
}

} // namespace

int main() {
    testSupportedImages();
    testFolderLabelInference();
    testCosineSimilarity();
    testFindSimilarOrdersByScore();
    testClustererAssignsClusters();
    testExportIncludesPredictions();

    std::cout << "All core tests passed\n";
    return 0;
}
