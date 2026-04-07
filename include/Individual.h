#pragma once

#include <vector>
#include <random>
#include "PFSPInstance.h"


class Individual {
public:
    // constructs a random controlled permutation 
    Individual(int numJobs, std::mt19937& rng);

    // constructs an individual from existing permutation 
    explicit Individual(std::vector<int> genes);

    // total flow of time 
    void evaluate(const PFSPInstance& instance);

    int              getFitness()          const;
    int              getNumJobs()          const;
   
    const std::vector<int>& getGenes()     const;
    
    // non-const access to genes — used by mutation operators #
    // neturning by reference allows in-place modification #
    std::vector<int>& getMutableGenes();
    
    // Returns true if this individual is better (lower fitness) 
    bool operator<(const Individual& other) const;

    // debug helper
    void print() const;

private:
    std::vector<int> genes;    // the job permutation
    int fitness;               // total flow time — evaluate()
};