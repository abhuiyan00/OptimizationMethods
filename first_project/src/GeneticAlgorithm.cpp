#include "GeneticAlgorithm.h"

#include <algorithm>
#include <chrono>
#include <iostream>
#include <iomanip>

GeneticAlgorithm::GeneticAlgorithm(const PFSPInstance& instance,
                                   const Config&       config,
                                   std::mt19937&       rng)
    : instance(instance), config(config), rng(rng)
{}

// Main GA loop.
SearchResult GeneticAlgorithm::run()
{
    auto startTime = std::chrono::high_resolution_clock::now();

    SearchResult result;

    // Start from a random evaluated population.
    Population pop(config.populationSize, instance, rng);
    pop.initialize();

    for (int gen = 0; gen < config.generations; ++gen) {
        pop.sort();

        result.addSnapshot(
            pop.getBest().getFitness(),
            pop.getAvgFitness(),
            pop.getWorst().getFitness()
        );

        // Build next generation, then move it into the population.
        std::vector<Individual> nextGen;
        nextGen.reserve(config.populationSize);

        // Elitism: keep the current best.
        nextGen.push_back(pop[0]);

        // Fill the rest with selection, crossover, mutation, evaluation.
        while (static_cast<int>(nextGen.size()) < config.populationSize) {

            Individual parent1 = tournamentSelect(pop);
            Individual parent2 = tournamentSelect(pop);

            Individual child = [&]() -> Individual {
                std::uniform_real_distribution<double> prob(0.0, 1.0);
                if (prob(rng) < config.crossoverProbability) {
                    return orderCrossover(parent1, parent2);
                } else {
                    return parent1;
                }
            }();

            {
                std::uniform_real_distribution<double> prob(0.0, 1.0);
                if (prob(rng) < config.mutationProbability) {
                    swapMutate(child);
                }
            }

            // Fitness is invalid after crossover/mutation.
            child.evaluate(instance);
            // Move into next generation to avoid a copy.
            nextGen.push_back(std::move(child));
        }

        // Replace population contents.
        pop.getIndividuals() = std::move(nextGen);
    }

    pop.sort();

    auto endTime = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> elapsed = endTime - startTime;

    result.bestFitness  = pop.getBest().getFitness();
    result.worstFitness = pop.getWorst().getFitness();
    result.avgFitness   = pop.getAvgFitness();
    result.evaluations  = config.populationSize * config.generations;
    result.timeMs       = elapsed.count();

    return result;
}

// Tournament selection.
Individual GeneticAlgorithm::tournamentSelect(const Population& pop)
{
    std::uniform_int_distribution<int> dist(0, pop.getSize() - 1);

    int bestIdx = dist(rng);

    for (int i = 1; i < config.tournamentSize; ++i) {
        int challenger = dist(rng);
        if (pop[challenger].getFitness() < pop[bestIdx].getFitness()) {
            bestIdx = challenger;
        }
    }

    return pop[bestIdx];
}

// Order crossover (OX) for permutations.
Individual GeneticAlgorithm::orderCrossover(const Individual& parent1,
                                            const Individual& parent2)
{
    const int n = parent1.getNumJobs();
    const std::vector<int>& p1 = parent1.getGenes();
    const std::vector<int>& p2 = parent2.getGenes();

    std::vector<int> childGenes(n, -1);

    // Pick two cut points and copy that segment from parent1.
    std::uniform_int_distribution<int> dist(0, n - 1);
    int cut1 = dist(rng);
    int cut2 = dist(rng);
    if (cut1 > cut2) std::swap(cut1, cut2);

    std::vector<bool> inChild(n, false);

    for (int i = cut1; i <= cut2; ++i) {
        childGenes[i] = p1[i];
        inChild[p1[i]] = true;
    }

    // Fill remaining slots from parent2 order, skipping duplicates.

    int fillPos = (cut2 + 1) % n;
    int scanPos = (cut2 + 1) % n;

    // (n - (cut2 - cut1 + 1)) more genes
    for (int placed = 0; placed < n - (cut2 - cut1 + 1); ++placed) {

        while (inChild[p2[scanPos]]) {
            scanPos = (scanPos + 1) % n;
        }

        childGenes[fillPos] = p2[scanPos];
        inChild[p2[scanPos]] = true;

        fillPos = (fillPos + 1) % n;
        scanPos = (scanPos + 1) % n;
    }

    return Individual(std::move(childGenes));
}

// Swap-mutation on two random positions.
void GeneticAlgorithm::swapMutate(Individual& ind)
{
    const int n = ind.getNumJobs();
    std::uniform_int_distribution<int> dist(0, n - 1);

    int pos1 = dist(rng);
    int pos2;

    do {
        pos2 = dist(rng);
    } while (pos2 == pos1);

    std::vector<int>& genes = ind.getMutableGenes();
    std::swap(genes[pos1], genes[pos2]);

}