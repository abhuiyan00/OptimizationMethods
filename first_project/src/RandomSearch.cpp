#include "RandomSearch.h"
#include "Individual.h"

#include <chrono>
#include <limits>
#include <iostream>

// Keep references to shared inputs.
RandomSearch::RandomSearch(const PFSPInstance& instance,
                           const Config&       config,
                           std::mt19937&       rng)
    : instance(instance), config(config), rng(rng)
{}

// Evaluate random permutations under a fixed evaluation budget.
SearchResult RandomSearch::run()
{
    auto startTime = std::chrono::high_resolution_clock::now();

    SearchResult result;

    const int totalEvals = config.populationSize * config.generations;

    // Save one snapshot per population-size chunk.
    const int snapshotInterval = config.populationSize;

    // Track global and snapshot-level stats.
    int bestFitness  = std::numeric_limits<int>::max();
    int worstFitness = 0;
    int snapshotBest  = bestFitness;
    int snapshotWorst = 0;
    double snapshotSumFitness = 0.0;
    int snapshotCount = 0;

    for (int eval = 0; eval < totalEvals; ++eval) {

        // Sample and evaluate one random permutation.
        Individual candidate(instance.getNumJobs(), rng);
        candidate.evaluate(instance);

        int f = candidate.getFitness();

        // Update global best/worst.
        if (f < bestFitness)  bestFitness  = f;
        if (f > worstFitness) worstFitness = f;

        // Update current snapshot stats.
        if (f < snapshotBest)  snapshotBest  = f;
        if (f > snapshotWorst) snapshotWorst = f;
        snapshotSumFitness += f;
        snapshotCount++;

        // Flush snapshot once interval is reached.
        if (snapshotCount == snapshotInterval) {
            double snapshotAvg = snapshotSumFitness / snapshotCount;
            result.addSnapshot(snapshotBest, snapshotAvg, snapshotWorst);

            // Reset snapshot accumulators.
            snapshotBest  = std::numeric_limits<int>::max();
            snapshotWorst = 0;
            snapshotSumFitness = 0.0;
            snapshotCount = 0;
        }
    }

    // Measure runtime.
    auto endTime = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> elapsed = endTime - startTime;

    // Fill final result.
    result.bestFitness  = bestFitness;
    result.worstFitness = worstFitness;
    result.avgFitness   = 0.0;  // not meaningful for random search
    result.evaluations  = totalEvals;
    result.timeMs       = elapsed.count();

    return result;
}