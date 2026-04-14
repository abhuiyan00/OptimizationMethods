#pragma once

#include "PFSPInstance.h"
#include "SearchResult.h"

// GreedySearch , deterministic, better than random search, but beatable by metaheuristics

class GreedySearch {
public:

    explicit GreedySearch(const PFSPInstance& instance);

    SearchResult run();

private:
    const PFSPInstance& instance;

    // sequence of jobs, and len tells how many jobs, computes total flow of time on last job
    int computePartialFlowTime(const std::vector<int>& sequence, int len) const;
};