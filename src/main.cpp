#include <iostream>
#include <string>
#include "Config.h"
#include "Logger.h"
#include "ExperimentRunner.h"

int main(int argc, char* argv[])
{
    // -------------------------------------------------------
    // Configuration
    // -------------------------------------------------------
    Config cfg;
    // All defaults are set in Config.h.
    // To override from command line you could add argument parsing here,
    // but for the report just edit Config.h and recompile.

    // -------------------------------------------------------
    // Setup
    // -------------------------------------------------------
    const std::string dataDir    = "data";
    const std::string resultsDir = "results";

    Logger logger(resultsDir);
    logger.initSummary();   // creates results/summary.csv with header

    ExperimentRunner runner(cfg, dataDir, logger);

    // -------------------------------------------------------
    // Choose what to run.
    // Comment/uncomment sections as needed while developing.
    // -------------------------------------------------------

    // Option A: run all six instances with all four methods
    // This is what you need for the final report.
    runner.runAll();

    // Option B: run the parameter sweep on the easy instance.
    // Uncomment when you want to generate parameter data for the report.
    // The sweep varies Pm, Px, popSize, gens, tournamentSize one at a time.
    runner.runParameterSweep("data/tai20_5_0.txt", "tai20_5_0");

    // -------------------------------------------------------
    // Done
    // -------------------------------------------------------
    std::cout << "\n===========================================\n";
    std::cout << "  Experiment complete.\n";
    std::cout << "  summary.csv        -> comparison table\n";
    std::cout << "  history_*.csv      -> fitness-over-time graphs\n";
    std::cout << "  params_*.csv       -> parameter sweep charts\n";
    std::cout << "===========================================\n";

    return 0;
}