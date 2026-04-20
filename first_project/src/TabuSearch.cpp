#include "TabuSearch.h"
#include "GreedySearch.h"

#include <chrono>
#include <limits>
#include <iostream>
#include <algorithm>

TabuSearch::TabuSearch(const PFSPInstance& instance,
                       const Config&       config,
                       std::mt19937&       rng)
    : instance(instance), config(config), rng(rng)
{}

SearchResult TabuSearch::run()
{
    auto startTime = std::chrono::high_resolution_clock::now();
    SearchResult result;

    const int n = instance.getNumJobs();

    // Start from a random evaluated solution.
    Individual current(n, rng);
    current.evaluate(instance);

    Individual globalBest = current;

    const int neighbourhoodSize = n * (n - 1) / 2;
    const int totalBudget       = config.populationSize * config.generations;
    const int iterations        = std::max(1, totalBudget / neighbourhoodSize);

    TabuList tabuList;

    for (int iter = 0; iter < iterations; ++iter) {
        int  bestNeighbourFitness = std::numeric_limits<int>::max();
        Move bestMove             = {0, 1};
        bool foundAllowedMove     = false;

        for (int i = 0; i < n - 1; ++i) {
            for (int j = i + 1; j < n; ++j) {
                Move move      = {i, j};
                Individual nb  = applySwap(current, i, j);
                nb.evaluate(instance);
                int f          = nb.getFitness();

                bool tabu       = isTabu(tabuList, move);
                bool aspiration = (f < globalBest.getFitness());

                if (!tabu || aspiration) {
                    if (f < bestNeighbourFitness) {
                        bestNeighbourFitness = f;
                        bestMove             = move;
                        foundAllowedMove     = true;
                    }
                }
            }
        }

        if (!foundAllowedMove) {
            result.addSnapshot(globalBest.getFitness(),
                               static_cast<double>(current.getFitness()),
                               current.getFitness());
            continue;
        }

        current = applySwap(current, bestMove.first, bestMove.second);
        current.evaluate(instance);
        addToTabu(tabuList, bestMove);

        if (current.getFitness() < globalBest.getFitness())
            globalBest = current;

        result.addSnapshot(globalBest.getFitness(),
                           static_cast<double>(current.getFitness()),
                           current.getFitness());
    }

    auto endTime = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> elapsed = endTime - startTime;

    result.bestFitness  = globalBest.getFitness();
    result.worstFitness = current.getFitness();
    result.avgFitness   = static_cast<double>(globalBest.getFitness());
    result.evaluations  = iterations * neighbourhoodSize;
    result.timeMs       = elapsed.count();
    return result;
}

bool TabuSearch::isTabu(const TabuList& tabuList, Move move) const
{
    for (const Move& tabuMove : tabuList) {
        if (tabuMove == move) return true;
        if (tabuMove.first == move.second && tabuMove.second == move.first)
            return true;
    }
    return false;
}

// Push newest move; drop oldest if list grows too much.
void TabuSearch::addToTabu(TabuList& tabuList, Move move)
{
    tabuList.push_front(move);

    if (static_cast<int>(tabuList.size()) > config.tabuSize) {
        tabuList.pop_back();
    }
}

// Return a swapped copy without changing the base individual.
Individual TabuSearch::applySwap(const Individual& base,
                                  int               pos1,
                                  int               pos2) const
{
    std::vector<int> newGenes = base.getGenes();

    std::swap(newGenes[pos1], newGenes[pos2]);

    return Individual(std::move(newGenes));
}