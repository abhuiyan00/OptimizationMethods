#pragma once

#include <vector>
#include <random>
#include "Individual.h"
#include "PFSPInstance.h"

// fix sized population

class Population {
public:

    // empty pop
    Population(int size, const PFSPInstance& instance, std::mt19937& rng);

    // initialize with random individuals
    void initialize();

    // returns a const ref to the lowest fitness 
    const Individual& getBest()  const;
    const Individual& getWorst() const;
    double            getAvgFitness() const;

    // ascending sort
    void sort();

    // returns the number of individuals
    int  getSize() const;

    // access individual by index
    Individual&       operator[](int index);
    const Individual& operator[](int index) const;

    // returns full vector
    std::vector<Individual>& getIndividuals();

// no copy of instance and rng, 
private:
    int                      size;
    const PFSPInstance&      instance;   
    std::mt19937&            rng;        
    std::vector<Individual>  individuals;
};