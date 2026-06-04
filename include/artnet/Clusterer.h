#pragma once

#include "artnet/ImageRecord.h"

#include <vector>

namespace artnet {

class Clusterer {
public:
    explicit Clusterer(int clusterCount = 8);

    void assignClusters(std::vector<ImageRecord>& records) const;

private:
    int clusterCount_;
};

} // namespace artnet
