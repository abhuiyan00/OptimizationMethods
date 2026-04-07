#pragma once

#include <string>
#include <vector>
#include "Config.h"
#include "Logger.h"

// ExperimentRunner coordinates experiments and parameter sweeps for Taillard instances.

class ExperimentRunner {
public:

    ExperimentRunner(const Config&      cfg,
                     const std::string& dataDir,
                     Logger&            logger);

    void runAll();

    // varies GA parameters one at a time on one instance
    void runParameterSweep(const std::string& instanceFile,
                           const std::string& instanceName);

private:
    Config             cfg;
    std::string        dataDir;
    Logger&            logger;

    void runInstance(const std::string& instanceFile,
                     const std::string& instanceName);

    void logParamRow(const std::string& csvPath,
                     const std::string& paramName,
                     double             paramValue,
                     const std::string& instanceName,
                     int                bestFitness,
                     double             avgFitness,
                     double             stdDev,
                     double             avgTimeMs);

    double stdDev(const std::vector<int>& values) const;
};