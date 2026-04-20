#pragma once

#include "PFSPInstance.h"
#include "SearchResult.h"

// Deterministic constructive baseline.

class GreedySearch {
public:

    explicit GreedySearch(const PFSPInstance& instance);

    SearchResult run();

private:
    const PFSPInstance& instance;

    // Flow time for the first len jobs of a partial sequence.
    int computePartialFlowTime(const std::vector<int>& sequence, int len) const;
};