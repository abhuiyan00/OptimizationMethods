#pragma once

// Shared runtime settings for all methods.

struct Config {

    // Genetic algorithm settings.
    int populationSize = 100;
    int generations = 100;
    double crossoverProbability = 0.7;  // Px
    double mutationProbability = 0.01;  // Pm
    int tournamentSize = 5;

    // Tabu Search settings.
    int tabuSize = 7;

    // ILS settings.
    int    ilsMaxIterations     = 2000;  // Outer-loop iterations.
    int    perturbationStrength = 4;     // Random swaps per perturbation.
    double ilsTimeLimitSec      = 0.0;   // 0 means no time limit.

    // General experiment settings.
    int randomSeed = 42;
    int repetitions = 10;
};