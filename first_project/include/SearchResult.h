#pragma once

#include <string>
#include <vector>

// Shared result container used by all methods.

struct SearchResult {

    int    bestFitness   = 0;     
    int    worstFitness  = 0;     
    double avgFitness    = 0.0;   
    int    evaluations   = 0;     
    double timeMs        = 0.0;   

    struct GenerationStats {
        int    best;
        double avg;
        int    worst;
    };
    std::vector<GenerationStats> history;

    // Append one convergence snapshot.
    void addSnapshot(int best, double avg, int worst) {
        history.push_back({best, avg, worst});
    }
};