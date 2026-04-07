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

    // a. start from greedy solution 
    GreedySearch greedy(instance);
    SearchResult greedyResult = greedy.run();
    
    //reconstruct an Individual from the greedy result 
    Individual current(n, rng);
    current.evaluate(instance);

    {
        // run again greedy for best fitness
        GreedySearch g2(instance);
        SearchResult gr = g2.run();
        (void)gr;  // suppress "unused variable" warning
    }

    // start from a random, tabu search will improve it
    current = Individual(n, rng);
    current.evaluate(instance);

    Individual globalBest = current;

    // b. iteration budget calculation, same as GA
    const int neighbourhoodSize = n * (n - 1) / 2;
    const int totalBudget = config.populationSize * config.generations;

    const int iterations = totalBudget / neighbourhoodSize;

    // c. tabu list - starts empty, stores recent moves
    TabuList tabuList;

    // d. main loop
    for (int iter = 0; iter < iterations; ++iter) {

        int  bestNeighbourFitness = std::numeric_limits<int>::max();
        Move bestMove             = {0, 1};   // default, will be overwritten
        bool foundAllowedMove     = false;

        for (int i = 0; i < n - 1; ++i) {
            for (int j = i + 1; j < n; ++j) {

                Move move = {i, j};

                Individual neighbour = applySwap(current, i, j);
                neighbour.evaluate(instance);

                int f = neighbour.getFitness();

                bool tabu      = isTabu(tabuList, move);
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
            result.addSnapshot(
                globalBest.getFitness(),
                static_cast<double>(current.getFitness()),
                current.getFitness()
            );
            continue;
        }

        current = applySwap(current, bestMove.first, bestMove.second);
        current.evaluate(instance);

        addToTabu(tabuList, bestMove);

        if (current.getFitness() < globalBest.getFitness()) {
            globalBest = current;   // deep copy — Individual's vector is copied
        }

        result.addSnapshot(
            globalBest.getFitness(),
            static_cast<double>(current.getFitness()),
            current.getFitness()
        );
    }

    // e. results
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

// add a move to the front (new entry) and remove from the back if we exceed tabuSize
void TabuSearch::addToTabu(TabuList& tabuList, Move move)
{
    tabuList.push_front(move);

    if (static_cast<int>(tabuList.size()) > config.tabuSize) {
        tabuList.pop_back();
    }
}

// apply swap, new Individual with swapped genes, does not modify base
// performs the swap on the copy
Individual TabuSearch::applySwap(const Individual& base,
                                  int               pos1,
                                  int               pos2) const
{
    std::vector<int> newGenes = base.getGenes();

    std::swap(newGenes[pos1], newGenes[pos2]);

    return Individual(std::move(newGenes));
}