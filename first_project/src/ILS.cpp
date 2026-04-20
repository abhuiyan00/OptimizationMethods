#include "ILS.h"

#include <algorithm>
#include <numeric>
#include <iostream>
#include <iomanip>
#include <limits>

// Store references and cache instance dimensions.
ILS::ILS(const PFSPInstance& instance,
         const Config&       config,
         std::mt19937&       rng)
    : instance(instance), config(config), rng(rng),
      numJobs(instance.getNumJobs()),
      numMachines(instance.getNumMachines())
{}

// Flow time for a full permutation.
int ILS::computeFlowTime(const std::vector<int>& perm) const
{
    const int n = numJobs;
    const int m = numMachines;

    // Flat DP array is cache-friendlier than vector<vector<int>>.
    std::vector<int> C(m * n);

    for (int machine = 0; machine < m; ++machine) {
        for (int pos = 0; pos < n; ++pos) {
            int job       = perm[pos];
            int fromAbove = (machine == 0) ? 0 : C[(machine - 1) * n + pos];
            int fromLeft  = (pos == 0)     ? 0 : C[machine * n + (pos - 1)];
            C[machine * n + pos] = std::max(fromAbove, fromLeft)
                                 + instance.getProcessingTime(machine, job);
        }
    }

    int total = 0;
    for (int pos = 0; pos < n; ++pos)
        total += C[(m - 1) * n + pos];
    return total;
}

// Flow time for the first len jobs only.
int ILS::computePartialFlowTime(const std::vector<int>& perm, int len) const
{
    const int m = numMachines;

    std::vector<int> C(m * len);

    for (int machine = 0; machine < m; ++machine) {
        for (int pos = 0; pos < len; ++pos) {
            int job       = perm[pos];
            int fromAbove = (machine == 0) ? 0 : C[(machine - 1) * len + pos];
            int fromLeft  = (pos == 0)     ? 0 : C[machine * len + (pos - 1)];
            C[machine * len + pos] = std::max(fromAbove, fromLeft)
                                   + instance.getProcessingTime(machine, job);
        }
    }

    int total = 0;
    for (int pos = 0; pos < len; ++pos)
        total += C[(m - 1) * len + pos];
    return total;
}

// Classic NEH constructive heuristic.
std::vector<int> ILS::nehHeuristic() const
{
    const int n = numJobs;
    const int m = numMachines;

    // 1) Rank jobs by total processing time (descending).
    std::vector<std::pair<int,int>> jobWeights(n); // {totalTime, jobId}
    for (int j = 0; j < n; ++j) {
        int total = 0;
        for (int machine = 0; machine < m; ++machine)
            total += instance.getProcessingTime(machine, j);
        jobWeights[j] = {total, j};
    }

    // Larger total processing time first.
    std::sort(jobWeights.begin(), jobWeights.end(),
              [](const std::pair<int,int>& a, const std::pair<int,int>& b) {
                  return a.first > b.first;
              });

    // 2) Build sequence incrementally.
    std::vector<int> seq;
    seq.reserve(n);
    seq.push_back(jobWeights[0].second);

    // 3) Insert each next job where partial flow time is best.
    for (int k = 1; k < n; ++k) {
        int job     = jobWeights[k].second;
        int bestPos = 0;
        int bestFit = std::numeric_limits<int>::max();
        int len     = k + 1;  // Length after insertion.

        // Try all insertion positions 0..k.
        for (int pos = 0; pos <= k; ++pos) {
            // Build temporary sequence with job inserted at pos.
            std::vector<int> partial;
            partial.reserve(len);
            for (int i = 0; i < pos; ++i)  partial.push_back(seq[i]);
            partial.push_back(job);
            for (int i = pos; i < k; ++i)   partial.push_back(seq[i]);

            int fit = computePartialFlowTime(partial, len);
            if (fit < bestFit) {
                bestFit = fit;
                bestPos = pos;
            }
        }

        // Apply best insertion.
        seq.insert(seq.begin() + bestPos, job);
    }

    return seq;
}

// 1-shift insertion local search with first-improvement restart.
bool ILS::insertionLocalSearch(std::vector<int>& perm, int& fitness)
{
    const int n = numJobs;
    const int m = numMachines;
    bool anyImproved = false;
    bool improved = true;

    while (improved) {
        improved = false;

        for (int i = 0; i < n; ++i) {
            int removedJob = perm[i];

            // Remove one job and test all reinsertions.
            std::vector<int> reduced;
            reduced.reserve(n - 1);
            for (int p = 0; p < n; ++p)
                if (p != i) reduced.push_back(perm[p]);

            // Track best strict improvement for this removed job.
            int bestPos = -1;
            int bestFit = fitness;

            for (int j = 0; j < n; ++j) {
                if (j == i) continue;

                // Evaluate candidate layout without allocating a full candidate vector.
                std::vector<int> C(m * n);
                for (int machine = 0; machine < m; ++machine) {
                    for (int p = 0; p < n; ++p) {
                        int job;
                        if      (p < j)  job = reduced[p];
                        else if (p == j) job = removedJob;
                        else             job = reduced[p - 1];

                        int fromAbove = (machine == 0) ? 0 : C[(machine - 1) * n + p];
                        int fromLeft  = (p == 0)       ? 0 : C[machine * n + (p - 1)];
                        C[machine * n + p] = std::max(fromAbove, fromLeft)
                                           + instance.getProcessingTime(machine, job);
                    }
                }

                int flowTime = 0;
                for (int p = 0; p < n; ++p)
                    flowTime += C[(m - 1) * n + p];

                if (flowTime < bestFit) {
                    bestFit = flowTime;
                    bestPos = j;
                }
            }

            // Apply best improving insertion.
            if (bestPos != -1 && bestFit < fitness) {
                perm.clear();
                perm.reserve(n);
                for (int p = 0; p < bestPos; ++p)     perm.push_back(reduced[p]);
                perm.push_back(removedJob);
                for (int p = bestPos; p < n - 1; ++p)  perm.push_back(reduced[p]);

                fitness     = bestFit;
                improved    = true;
                anyImproved = true;
                break;  // Restart after first improvement.
            }
        }
    }

    return anyImproved;
}

// Apply random swaps as perturbation.
void ILS::perturb(std::vector<int>& perm, int strength)
{
    std::uniform_int_distribution<int> dist(0, numJobs - 1);
    for (int s = 0; s < strength; ++s) {
        int pos1 = dist(rng);
        int pos2;
        do { pos2 = dist(rng); } while (pos2 == pos1);
        std::swap(perm[pos1], perm[pos2]);
    }
}

// Main ILS flow: NEH init, local search, then perturb+improve loop.
SearchResult ILS::run()
{
    auto startTime = std::chrono::high_resolution_clock::now();
    SearchResult result;

    // Phase 1: build initial solution with NEH.
    std::vector<int> current = nehHeuristic();
    int currentFit = computeFlowTime(current);
    std::cout << "    NEH initial: " << currentFit << "\n";

    // Phase 2: move to first local optimum.
    insertionLocalSearch(current, currentFit);
    std::cout << "    After first LS: " << currentFit << "\n";

    std::vector<int> best = current;
    int bestFit  = currentFit;
    int worstFit = currentFit;   // Worst local optimum seen so far.
    double fitSum = currentFit;  // Running sum for average fitness.
    int fitCount  = 1;

    result.addSnapshot(bestFit, static_cast<double>(bestFit), worstFit);

    // Phase 3: iterative perturbation and improvement.
    int maxIter = config.ilsMaxIterations;
    int noImproveCount = 0;
    int noImproveLimit = std::max(200, maxIter / 5);

    for (int iter = 0; iter < maxIter; ++iter) {

        // Optional wall-time stop.
        if (config.ilsTimeLimitSec > 0.0) {
            auto now = std::chrono::high_resolution_clock::now();
            double elapsed = std::chrono::duration<double>(now - startTime).count();
            if (elapsed >= config.ilsTimeLimitSec) break;
        }

        // Perturb current solution.
        std::vector<int> candidate = current;
        perturb(candidate, config.perturbationStrength);

        // Improve perturbed candidate.
        int candFit = computeFlowTime(candidate);
        insertionLocalSearch(candidate, candFit);

        // Update aggregate stats.
        if (candFit > worstFit) worstFit = candFit;
        fitSum += candFit;
        fitCount++;

        // Strict acceptance rule.
        if (candFit < currentFit) {
            current    = candidate;
            currentFit = candFit;
            noImproveCount = 0;
        } else {
            noImproveCount++;
        }

        // Track global best.
        if (candFit < bestFit) {
            best    = candidate;
            bestFit = candFit;
        }

        // Sparse snapshots for convergence plots.
        if ((iter + 1) % 50 == 0) {
            result.addSnapshot(bestFit,
                               fitSum / fitCount,
                               worstFit);
        }

        // Restart from random solution if stuck.
        if (noImproveCount >= noImproveLimit) {
            std::vector<int> fresh(numJobs);
            std::iota(fresh.begin(), fresh.end(), 0);
            std::shuffle(fresh.begin(), fresh.end(), rng);
            currentFit = computeFlowTime(fresh);
            insertionLocalSearch(fresh, currentFit);
            current = fresh;
            if (currentFit > worstFit) worstFit = currentFit;
            fitSum += currentFit;
            fitCount++;
            noImproveCount = 0;
        }
    }

    // Final snapshot.
    result.addSnapshot(bestFit, fitSum / fitCount, worstFit);

    auto endTime = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> elapsed = endTime - startTime;

    result.bestFitness  = bestFit;
    result.worstFitness = worstFit;
    result.avgFitness   = fitSum / fitCount;
    result.evaluations  = fitCount;  // Number of local optima evaluated.
    result.timeMs       = elapsed.count();

    std::cout << "    ILS final best: " << bestFit
              << "  worst: " << worstFit
              << "  avg: " << std::fixed << std::setprecision(1)
              << (fitSum / fitCount)
              << "  time: " << std::setprecision(0) << elapsed.count() << " ms\n";

    return result;
}