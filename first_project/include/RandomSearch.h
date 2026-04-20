#pragma once

#include <random>
#include "PFSPInstance.h"
#include "Config.h"
#include "SearchResult.h"

// Random baseline search.

class RandomSearch {
public:
    // Keep references to shared instance/config/RNG.
    RandomSearch(const PFSPInstance& instance,
                 const Config&       config,
                 std::mt19937&       rng);

    SearchResult run();

private:
    const PFSPInstance& instance;
    const Config&       config;
    std::mt19937&       rng;
};