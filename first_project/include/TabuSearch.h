#pragma once

#include <deque>
#include <utility>    // std::pair
#include "PFSPInstance.h"
#include "Config.h"
#include "Individual.h"
#include "SearchResult.h"

// tabu is a single-solution local search metaheuristic

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

    //  Move is a position swap
    using Move = std::pair<int, int>;

    // tabu list - deque of recent moves, front = most recent, back = oldest, do not modify base
    using TabuList = std::deque<Move>;

    bool isTabu(const TabuList& tabuList, Move move) const;

    void addToTabu(TabuList& tabuList, Move move);

    Individual applySwap(const Individual& base, int pos1, int pos2) const;
};