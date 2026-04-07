#pragma once

#include <string>
#include <vector>

// a comparable data container

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

    // snapshot container vector
    void addSnapshot(int best, double avg, int worst) {
        history.push_back({best, avg, worst});
    }
};