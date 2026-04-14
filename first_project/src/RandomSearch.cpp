#include "RandomSearch.h"
#include "Individual.h"

#include <chrono>
#include <limits>
#include <iostream>

// a constructor to store the ref.
RandomSearch::RandomSearch(const PFSPInstance& instance,
                           const Config&       config,
                           std::mt19937&       rng)
    : instance(instance), config(config), rng(rng)
{}

// run RandomSearch
SearchResult RandomSearch::run()
{
    auto startTime = std::chrono::high_resolution_clock::now(); // clock start

    SearchResult result;

    const int totalEvals = config.populationSize * config.generations;

    // snap of every popSize
    const int snapshotInterval = config.populationSize;

    // best solution track 
    int bestFitness  = std::numeric_limits<int>::max();
    int worstFitness = 0;
    int snapshotBest  = bestFitness;
    int snapshotWorst = 0;
    double snapshotSumFitness = 0.0;
    int snapshotCount = 0;

    for (int eval = 0; eval < totalEvals; ++eval) {

        // generate a random solution
        Individual candidate(instance.getNumJobs(), rng);
        candidate.evaluate(instance);

        int f = candidate.getFitness();

        // update global best/worst
        if (f < bestFitness)  bestFitness  = f;
        if (f > worstFitness) worstFitness = f;

        // update snapshot best/worst/sum
        if (f < snapshotBest)  snapshotBest  = f;
        if (f > snapshotWorst) snapshotWorst = f;
        snapshotSumFitness += f;
        snapshotCount++;

        // on snapshotInterval, records
        if (snapshotCount == snapshotInterval) {
            double snapshotAvg = snapshotSumFitness / snapshotCount;
            result.addSnapshot(snapshotBest, snapshotAvg, snapshotWorst);

            // resets for next snapshot
            snapshotBest  = std::numeric_limits<int>::max();
            snapshotWorst = 0;
            snapshotSumFitness = 0.0;
            snapshotCount = 0;
        }
    }

    // clock stop, calculate elapsed time
    auto endTime = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> elapsed = endTime - startTime;

    // result
    result.bestFitness  = bestFitness;
    result.worstFitness = worstFitness;
    result.avgFitness   = 0.0;  // not meaningful for random search
    result.evaluations  = totalEvals;
    result.timeMs       = elapsed.count();

    return result;
}