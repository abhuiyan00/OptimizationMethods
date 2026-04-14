#include "GreedySearch.h"
#include "Individual.h"

#include <set>
#include <limits>
#include <chrono>
#include <iostream>

GreedySearch::GreedySearch(const PFSPInstance& instance)
    : instance(instance)
{}

SearchResult GreedySearch::run()
{
    auto startTime = std::chrono::high_resolution_clock::now();
    const int n = instance.getNumJobs();

    // unscheduled pool as 'set' for O(log n), removes a specific job
    std::set<int> unscheduled;
    for (int j = 0; j < n; ++j) {
        unscheduled.insert(j);
    }

    std::vector<int> sequence;
    sequence.reserve(n);

    for (int pos = 0; pos < n; ++pos) {
        int bestJob         = -1;
        int bestPartialFlow = std::numeric_limits<int>::max();

        for (int candidate : unscheduled) {
            sequence.push_back(candidate);
            int partialFlow = computePartialFlowTime(sequence, pos + 1);
            if (partialFlow < bestPartialFlow) {
                bestPartialFlow = partialFlow;
                bestJob         = candidate;
            }
            sequence.pop_back();
        }

        sequence.push_back(bestJob);
        unscheduled.erase(bestJob);
    }
    // sqeuence is from existing consttuctor, evaluation
    Individual solution(sequence);
    solution.evaluate(instance);
    
    //calculate elapsed time
    auto endTime = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> elapsed = endTime - startTime;

    SearchResult result;
    result.bestFitness  = solution.getFitness();
    result.worstFitness = solution.getFitness();
    result.avgFitness   = solution.getFitness();
    result.evaluations  = n;
    result.timeMs       = elapsed.count();

    result.addSnapshot(solution.getFitness(),
                       static_cast<double>(solution.getFitness()),
                       solution.getFitness());

    return result;
}

// we stop at (len-1)
int GreedySearch::computePartialFlowTime(const std::vector<int>& sequence, int len) const
{
    const int m = instance.getNumMachines();
 
    // C[machine][pos] = completion time table, allocating as many columns we need
    std::vector<std::vector<int>> C(m, std::vector<int>(len, 0));

    for (int machine = 0; machine < m; ++machine) {
        for (int pos = 0; pos < len; ++pos) {

            int job      = sequence[pos];
            int fromAbove = (machine == 0) ? 0 : C[machine - 1][pos];
            int fromLeft  = (pos == 0)     ? 0 : C[machine][pos - 1];

            C[machine][pos] = std::max(fromAbove, fromLeft)
                            + instance.getProcessingTime(machine, job);
        }
    }

    // sum the completion times of the last machine
    int total = 0;
    for (int pos = 0; pos < len; ++pos) {
        total += C[m - 1][pos];
    }
    return total;
}