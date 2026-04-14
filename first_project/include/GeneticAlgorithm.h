#pragma once

#include <random>
#include "PFSPInstance.h"
#include "Config.h"
#include "Population.h"
#include "SearchResult.h"

// GA has Tournament Selection, Order Crossover, Swap Mutation, and Elitism

class GeneticAlgorithm {
public:
    // running the PSFPInstance returns per-generation information
    GeneticAlgorithm(const PFSPInstance& instance,
                     const Config&       config,
                     std::mt19937&       rng);
    SearchResult run();

private:
    const PFSPInstance& instance;
    const Config&       config;
    std::mt19937&       rng;    
    
    // genetic operators

    // tournament selection , ordercrossover with valid permutation, swap two random positions
    Individual tournamentSelect(const Population& pop);
    Individual orderCrossover(const Individual& parent1,
                              const Individual& parent2);
    void swapMutate(Individual& ind);
};