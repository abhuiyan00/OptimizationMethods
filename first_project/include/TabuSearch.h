#pragma once

#include <deque>
#include <utility>    // std::pair
#include "PFSPInstance.h"
#include "Config.h"
#include "Individual.h"
#include "SearchResult.h"

// Tabu Search over swap moves.

class TabuSearch {
public:

    TabuSearch(const PFSPInstance& instance,
               const Config&       config,
               std::mt19937&       rng);

    SearchResult run();

private:
    const PFSPInstance& instance;
    const Config&       config;
    std::mt19937&       rng;

    // Move = swap two positions.
    using Move = std::pair<int, int>;

    // Recent moves, newest at front.
    using TabuList = std::deque<Move>;

    bool isTabu(const TabuList& tabuList, Move move) const;

    void addToTabu(TabuList& tabuList, Move move);

    Individual applySwap(const Individual& base, int pos1, int pos2) const;
};