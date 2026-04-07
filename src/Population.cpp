#include "Population.h"

#include <algorithm>
#include <numeric>
#include <stdexcept>

// constructor - store refrences, reserve vector memory
Population::Population(int size, const PFSPInstance& instance, std::mt19937& rng)
    : size(size), instance(instance), rng(rng)
{
    individuals.reserve(size);
}

// creates random individuals & evaluates
void Population::initialize()
{
    individuals.clear();   // remove individuals (re-runs)

    for (int i = 0; i < size; ++i) {
        // separate creation of random individual and copy
        individuals.emplace_back(instance.getNumJobs(), rng);

        individuals.back().evaluate(instance);
    }
}

// getting the best individual (lowest fitness)
const Individual& Population::getBest() const
{
    if (individuals.empty())
        throw std::runtime_error("Population::getBest called on empty population");

    auto it = std::min_element(individuals.begin(), individuals.end());
    return *it; // * dereferences the iterator
}

// getting the worst individual (highest fitness)
const Individual& Population::getWorst() const
{
    if (individuals.empty())
        throw std::runtime_error("Population::getWorst called on empty population");

    auto it = std::max_element(individuals.begin(), individuals.end());
    return *it;
}

// average fitness of the population
double Population::getAvgFitness() const
{
    if (individuals.empty()) return 0.0;
 
    int total = std::accumulate(
        individuals.begin(),
        individuals.end(),
        0,
        [](int sum, const Individual& ind) {
            return sum + ind.getFitness();          //lambda
        }
    );

    return static_cast<double>(total) / static_cast<double>(individuals.size());
}

// ascending sort
void Population::sort()
{
    std::sort(individuals.begin(), individuals.end());
}

// access individuals 
int Population::getSize() const
{
    return static_cast<int>(individuals.size());
}

Individual& Population::operator[](int index)
{
    return individuals.at(index);
}

const Individual& Population::operator[](int index) const
{
    return individuals.at(index);
}

std::vector<Individual>& Population::getIndividuals()
{
    return individuals;
}