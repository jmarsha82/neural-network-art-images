#pragma once

#include "artnet/ImageRecord.h"

#include <cstddef>
#include <vector>

namespace artnet {

struct SimilarityResult {
    std::size_t recordIndex = 0;
    float score = 0.0f;
};

class EmbeddingIndex {
public:
    explicit EmbeddingIndex(std::vector<ImageRecord> records);

    [[nodiscard]] const std::vector<ImageRecord>& records() const noexcept;
    [[nodiscard]] std::vector<SimilarityResult> findSimilar(std::size_t queryIndex,
                                                            std::size_t maxResults) const;

private:
    std::vector<ImageRecord> records_;
};

[[nodiscard]] float cosineSimilarity(const std::vector<float>& left,
                                     const std::vector<float>& right);

} // namespace artnet
