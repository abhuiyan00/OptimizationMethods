#include <iostream>
#include <string>
#include "Config.h"
#include "Logger.h"
#include "ExperimentRunner.h"

int main(int argc, char* argv[])
{
    Config cfg;
    const std::string dataDir    = "data";
    const std::string resultsDir = "results";

    Logger logger(resultsDir);
    logger.initSummary();

    ExperimentRunner runner(cfg, dataDir, logger);

    runner.runAll();

    runner.runParameterSweep("data/tai20_5_0.txt", "tai20_5_0");

    std::cout << "\n===========================================\n";
    std::cout << "  Experiment complete.\n";
    std::cout << "  summary.csv        -> comparison table\n";
    std::cout << "  history_*.csv      -> fitness-over-time graphs\n";
    std::cout << "  params_*.csv       -> parameter sweep charts\n";
    std::cout << "===========================================\n";

    return 0;
}