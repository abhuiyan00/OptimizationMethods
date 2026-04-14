#pragma once

#include <random>
#include "PFSPInstance.h"
#include "Config.h"
#include "SearchResult.h"

// RandomSearch, baseline 

class RandomSearch {
public:
    // runs random search 
    RandomSearch(const PFSPInstance& instance,
                 const Config&       config,
                 std::mt19937&       rng);

    SearchResult run();

private:
    const PFSPInstance& instance;
    const Config&       config;
    std::mt19937&       rng;
};