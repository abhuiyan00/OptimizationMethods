#pragma once

#include <string>
#include <vector>
#include "Config.h"
#include "Logger.h"

// ExperimentRunner coordinates the full experimental pipeline:
//
//   For each Taillard instance:
//     For each method (Greedy, Random, GA, TS):
//       Run cfg.repetitions times
//       Log every run to CSV via Logger
//
// It also runs the parameter sweep for the report:
//   Vary one parameter at a time while holding others fixed,
//   log results so you can plot Pm vs best fitness, etc.

class ExperimentRunner {
public:

    ExperimentRunner(const Config&      cfg,
                     const std::string& dataDir,
                     Logger&            logger);

    // Runs all six Taillard instances with all four methods.
    // Prints a progress summary to stdout as it goes.
    void runAll();

    // Varies GA parameters one at a time on one instance.
    // Results are written to results/params_{paramName}.csv
    // instanceFile: e.g. "data/tai20_5_0.txt"
    // instanceName: e.g. "tai20_5_0"   (used in filenames and CSV)
    void runParameterSweep(const std::string& instanceFile,
                           const std::string& instanceName);

private:
    Config             cfg;      // stored by VALUE so we can modify it
    std::string        dataDir;  // folder containing .txt benchmark files
    Logger&            logger;   // reference — shared logger

    // Runs one instance through all methods and logs everything.
    void runInstance(const std::string& instanceFile,
                     const std::string& instanceName);

    // Writes one parameter sweep result row to a dedicated CSV.
    void logParamRow(const std::string& csvPath,
                     const std::string& paramName,
                     double             paramValue,
                     const std::string& instanceName,
                     int                bestFitness,
                     double             avgFitness,
                     double             stdDev,
                     double             avgTimeMs);

    // Computes standard deviation of a vector of ints
    double stdDev(const std::vector<int>& values) const;
};