#pragma once

#include <vector>
#include <random>
#include "Individual.h"
#include "PFSPInstance.h"

// Fixed-size population container.

class Population {
public:

    // Create an empty population with reserved capacity.
    Population(int size, const PFSPInstance& instance, std::mt19937& rng);

    // Fill with random, evaluated individuals.
    void initialize();

    // Access best/worst and aggregate fitness.
    const Individual& getBest()  const;
    const Individual& getWorst() const;
    double            getAvgFitness() const;

    // Sort by fitness ascending.
    void sort();

    // Current number of individuals.
    int  getSize() const;

    // Indexed access.
    Individual&       operator[](int index);
    const Individual& operator[](int index) const;

    // Direct access to underlying storage.
    std::vector<Individual>& getIndividuals();

    // Stored by reference to avoid heavy copies.
private:
    int                      size;
    const PFSPInstance&      instance;   
    std::mt19937&            rng;        
    std::vector<Individual>  individuals;
};