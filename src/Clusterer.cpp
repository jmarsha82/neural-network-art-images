#include "artnet/Clusterer.h"

#ifdef ARTNET_USE_MLPACK
#include <mlpack.hpp>
#endif

#include <algorithm>
#include <limits>
#include <numeric>

namespace artnet {

Clusterer::Clusterer(int clusterCount)
    : clusterCount_(std::max(1, clusterCount)) {}

void Clusterer::assignClusters(std::vector<ImageRecord>& records) const {
    std::vector<std::size_t> embedded;
    for (std::size_t index = 0; index < records.size(); ++index) {
        if (!records[index].embedding.empty()) {
            embedded.push_back(index);
        }
    }

    if (embedded.empty()) {
        return;
    }

    const auto dimension = records[embedded.front()].embedding.size();
    const auto k = std::min<std::size_t>(static_cast<std::size_t>(clusterCount_), embedded.size());

#ifdef ARTNET_USE_MLPACK
    arma::mat data(dimension, embedded.size());
    for (std::size_t column = 0; column < embedded.size(); ++column) {
        const auto& vector = records[embedded[column]].embedding;
        for (std::size_t row = 0; row < dimension; ++row) {
            data(row, column) = vector[row];
        }
    }

    arma::Row<std::size_t> assignments;
    mlpack::KMeans<> kmeans;
    kmeans.Cluster(data, k, assignments);

    for (std::size_t column = 0; column < embedded.size(); ++column) {
        records[embedded[column]].cluster = static_cast<int>(assignments[column]);
    }
    return;
#else
    std::vector<std::vector<float>> centroids;
    for (std::size_t i = 0; i < k; ++i) {
        centroids.push_back(records[embedded[i]].embedding);
    }

    for (int iteration = 0; iteration < 12; ++iteration) {
        std::vector<std::vector<float>> sums(k, std::vector<float>(dimension, 0.0f));
        std::vector<int> counts(k, 0);

        for (auto recordIndex : embedded) {
            const auto& vector = records[recordIndex].embedding;
            std::size_t best = 0;
            float bestDistance = std::numeric_limits<float>::max();

            for (std::size_t cluster = 0; cluster < k; ++cluster) {
                float distance = 0.0f;
                for (std::size_t dim = 0; dim < dimension; ++dim) {
                    const auto diff = vector[dim] - centroids[cluster][dim];
                    distance += diff * diff;
                }
                if (distance < bestDistance) {
                    bestDistance = distance;
                    best = cluster;
                }
            }

            records[recordIndex].cluster = static_cast<int>(best);
            ++counts[best];
            for (std::size_t dim = 0; dim < dimension; ++dim) {
                sums[best][dim] += vector[dim];
            }
        }

        for (std::size_t cluster = 0; cluster < k; ++cluster) {
            if (counts[cluster] == 0) {
                continue;
            }
            for (std::size_t dim = 0; dim < dimension; ++dim) {
                centroids[cluster][dim] = sums[cluster][dim] / static_cast<float>(counts[cluster]);
            }
        }
    }
#endif
}

} // namespace artnet
