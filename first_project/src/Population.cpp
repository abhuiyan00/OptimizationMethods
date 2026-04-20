#include "Population.h"

#include <algorithm>
#include <numeric>
#include <stdexcept>

// Keep shared references and reserve storage.
Population::Population(int size, const PFSPInstance& instance, std::mt19937& rng)
    : size(size), instance(instance), rng(rng)
{
    individuals.reserve(size);
}

// Create random individuals and evaluate them.
void Population::initialize()
{
    individuals.clear();

    for (int i = 0; i < size; ++i) {
        individuals.emplace_back(instance.getNumJobs(), rng);

        individuals.back().evaluate(instance);
    }
}

// Best means lowest fitness.
const Individual& Population::getBest() const
{
    if (individuals.empty())
        throw std::runtime_error("Population::getBest called on empty population");

    auto it = std::min_element(individuals.begin(), individuals.end());
    return *it;
}

// Worst means highest fitness.
const Individual& Population::getWorst() const
{
    if (individuals.empty())
        throw std::runtime_error("Population::getWorst called on empty population");

    auto it = std::max_element(individuals.begin(), individuals.end());
    return *it;
}

// Average fitness across the current population.
double Population::getAvgFitness() const
{
    if (individuals.empty()) return 0.0;
 
    int total = std::accumulate(
        individuals.begin(),
        individuals.end(),
        0,
        [](int sum, const Individual& ind) {
            return sum + ind.getFitness();
        }
    );

    return static_cast<double>(total) / static_cast<double>(individuals.size());
}

// Sort from best to worst.
void Population::sort()
{
    std::sort(individuals.begin(), individuals.end());
}

// Access current individuals.
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