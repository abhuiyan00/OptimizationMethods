#pragma once 

// contains all parameters, can pass Config object everywhere

struct Config {

    // GA parameters
    int populationSize = 100;
    int generations = 100;
    double crossoverProbability = 0.7;  //Px
    double mutationProbability = 0.01;  //Pm
    int tournamentSize = 5;

    // Tabu
    int tabuSize = 7;

    // gneral parameters
    int randomSeed = 42;
    int repetitions = 10;
};