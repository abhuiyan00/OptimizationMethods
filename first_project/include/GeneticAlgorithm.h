#pragma once

#include <random>
#include "PFSPInstance.h"
#include "Config.h"
#include "Population.h"
#include "SearchResult.h"

// GA with tournament selection, OX crossover, swap mutation, and elitism.

class GeneticAlgorithm {
public:
    // Runs GA and returns summary plus generation history.
    GeneticAlgorithm(const PFSPInstance& instance,
                     const Config&       config,
                     std::mt19937&       rng);
    SearchResult run();

private:
    const PFSPInstance& instance;
    const Config&       config;
    std::mt19937&       rng;    
    
    // Genetic operators.
    Individual tournamentSelect(const Population& pop);
    Individual orderCrossover(const Individual& parent1,
                              const Individual& parent2);
    void swapMutate(Individual& ind);
};