#pragma once

#include "PFSPInstance.h"
#include "Config.h"
#include "SearchResult.h"
#include <vector>
#include <random>
#include <chrono>

// ILS with NEH initialization and insertion local search.

class ILS {
public:
    ILS(const PFSPInstance& instance, const Config& config, std::mt19937& rng);
    SearchResult run();

private:
    const PFSPInstance& instance;
    const Config&       config;
    std::mt19937&       rng;

    int numJobs;
    int numMachines;

    // Flow time for a full permutation.
    int computeFlowTime(const std::vector<int>& perm) const;

    // Flow time for the first len jobs (used by NEH).
    int computePartialFlowTime(const std::vector<int>& perm, int len) const;

    // Build an initial solution with NEH.
    std::vector<int> nehHeuristic() const;

    // 1-shift insertion local search; updates perm and fitness in place.
    bool insertionLocalSearch(std::vector<int>& perm, int& fitness);

    // Apply random swaps to escape the current local optimum.
    void perturb(std::vector<int>& perm, int strength);
};