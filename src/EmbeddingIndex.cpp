#include "artnet/EmbeddingIndex.h"

#include <algorithm>
#include <cmath>
#include <numeric>
#include <stdexcept>

namespace artnet {

EmbeddingIndex::EmbeddingIndex(std::vector<ImageRecord> records)
    : records_(std::move(records)) {}

const std::vector<ImageRecord>& EmbeddingIndex::records() const noexcept {
    return records_;
}

std::vector<SimilarityResult> EmbeddingIndex::findSimilar(std::size_t queryIndex,
                                                          std::size_t maxResults) const {
    if (queryIndex >= records_.size()) {
        throw std::out_of_range("queryIndex is outside the record list");
    }

    std::vector<SimilarityResult> results;
    const auto& query = records_[queryIndex].embedding;
    if (query.empty()) {
        return results;
    }

    for (std::size_t index = 0; index < records_.size(); ++index) {
        if (index == queryIndex || records_[index].embedding.empty()) {
            continue;
        }

        results.push_back({index, cosineSimilarity(query, records_[index].embedding)});
    }

    std::ranges::sort(results, [](const SimilarityResult& left, const SimilarityResult& right) {
        return left.score > right.score;
    });

    if (results.size() > maxResults) {
        results.resize(maxResults);
    }
    return results;
}

float cosineSimilarity(const std::vector<float>& left, const std::vector<float>& right) {
    if (left.empty() || right.empty() || left.size() != right.size()) {
        return 0.0f;
    }

    float dot = 0.0f;
    float leftNorm = 0.0f;
    float rightNorm = 0.0f;

    for (std::size_t index = 0; index < left.size(); ++index) {
        dot += left[index] * right[index];
        leftNorm += left[index] * left[index];
        rightNorm += right[index] * right[index];
    }

    if (leftNorm == 0.0f || rightNorm == 0.0f) {
        return 0.0f;
    }

    return dot / (std::sqrt(leftNorm) * std::sqrt(rightNorm));
}

} // namespace artnet
