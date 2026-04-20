#pragma once

#include <vector>
#include <random>
#include "PFSPInstance.h"


class Individual {
public:
    // Create a random permutation of jobs.
    Individual(int numJobs, std::mt19937& rng);

    // Create from an existing permutation.
    explicit Individual(std::vector<int> genes);

    // Evaluate total flow time.
    void evaluate(const PFSPInstance& instance);

    int              getFitness()          const;
    int              getNumJobs()          const;
   
    const std::vector<int>& getGenes()     const;
    
    // Mutable access used by mutation operators.
    std::vector<int>& getMutableGenes();
    
    // Lower fitness is better.
    bool operator<(const Individual& other) const;

    // Print genes and fitness for debugging.
    void print() const;

private:
    std::vector<int> genes;    // Job permutation.
    int fitness;               // Total flow time.
};