#include "Individual.h"
#include <numeric>    
#include <algorithm>  
#include <iostream>   
#include <stdexcept>  

// random permutation
Individual::Individual(int numJobs, std::mt19937& rng)
    : fitness(0)
{
    genes.resize(numJobs); // create genes as [0, 1, 2, ..., numJobs-1]
    std::iota(genes.begin(), genes.end(), 0);

    std::shuffle(genes.begin(), genes.end(), rng);  // randomly shuffle the order

}

// crossover from existing genes
Individual::Individual(std::vector<int> inputGenes)
    : genes(std::move(inputGenes)), fitness(0) // moves vector memory. fast 
{
}

void Individual::evaluate(const PFSPInstance& instance)
{
    const int n = instance.getNumJobs();
    const int m = instance.getNumMachines();

    if (static_cast<int>(genes.size()) != n) {
        throw std::runtime_error("Individual: gene count does not match instance job count.");
    }
    // using 2D vector C[machine][position] to store completion times
    std::vector<std::vector<int>> C(m, std::vector<int>(n, 0));

    // time table
    for (int machine = 0; machine < m; ++machine) {
        for (int pos = 0; pos < n; ++pos) {

            int job = genes[pos]; // job index 

            // previous machine's completion time
            int fromAbove = (machine == 0) ? 0 : C[machine - 1][pos];

            // time from the left
            int fromLeft  = (pos == 0)     ? 0 : C[machine][pos - 1];

            // can't start until both are done
            int startTime = std::max(fromAbove, fromLeft);

            // getting assigned 
            C[machine][pos] = startTime + instance.getProcessingTime(machine, job);
        }
    }

    // sum the last row = flow time, C[m-1][pos] = completion time
    fitness = 0;
    for (int pos = 0; pos < n; ++pos) {
        fitness += C[m - 1][pos];
    }
}

// Getters
int Individual::getFitness() const { return fitness; }
int Individual::getNumJobs() const { return static_cast<int>(genes.size()); }

const std::vector<int>& Individual::getGenes() const { return genes; }

// non-const access to genes — used by mutation operators #
std::vector<int>& Individual::getMutableGenes() { return genes; }

// let's us write if a<b or sort vectors
bool Individual::operator<(const Individual& other) const
{
    return fitness < other.fitness;
}

//prints the genes and fitness
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