#include "Individual.h"
#include <numeric>    
#include <algorithm>  
#include <iostream>   
#include <stdexcept>  

// Build a random job permutation.
Individual::Individual(int numJobs, std::mt19937& rng)
    : fitness(0)
{
    genes.resize(numJobs); // Start from 0..numJobs-1.
    std::iota(genes.begin(), genes.end(), 0);

    std::shuffle(genes.begin(), genes.end(), rng);

}

// Build from an existing permutation.
Individual::Individual(std::vector<int> inputGenes)
    : genes(std::move(inputGenes)), fitness(0) // Move avoids a copy.
{
}

void Individual::evaluate(const PFSPInstance& instance)
{
    const int n = instance.getNumJobs();
    const int m = instance.getNumMachines();

    if (static_cast<int>(genes.size()) != n) {
        throw std::runtime_error("Individual: gene count does not match instance job count.");
    }
    // DP table of completion times: C[machine][position].
    std::vector<std::vector<int>> C(m, std::vector<int>(n, 0));

    // Standard flow-shop completion recurrence.
    for (int machine = 0; machine < m; ++machine) {
        for (int pos = 0; pos < n; ++pos) {

            int job = genes[pos];

            // Completion time from previous machine.
            int fromAbove = (machine == 0) ? 0 : C[machine - 1][pos];

            // Completion time of previous job on same machine.
            int fromLeft  = (pos == 0)     ? 0 : C[machine][pos - 1];

            // Job can start only when both constraints are satisfied.
            int startTime = std::max(fromAbove, fromLeft);

            C[machine][pos] = startTime + instance.getProcessingTime(machine, job);
        }
    }

    // Total flow time is sum of completion times on the last machine.
    fitness = 0;
    for (int pos = 0; pos < n; ++pos) {
        fitness += C[m - 1][pos];
    }
}

// Getters.
int Individual::getFitness() const { return fitness; }
int Individual::getNumJobs() const { return static_cast<int>(genes.size()); }

const std::vector<int>& Individual::getGenes() const { return genes; }

// Mutable access used by mutation operators.
std::vector<int>& Individual::getMutableGenes() { return genes; }

// Lower fitness means better individual.
bool Individual::operator<(const Individual& other) const
{
    return fitness < other.fitness;
}

// Debug print.
void Individual::print() const
{
    std::cout << "Genes:   [";
    for (int i = 0; i < static_cast<int>(genes.size()); ++i) {
        std::cout << genes[i];
        if (i < static_cast<int>(genes.size()) - 1) std::cout << ", ";
    }
    std::cout << "]\n";
    std::cout << "Fitness: " << fitness << "\n";
}