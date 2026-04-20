#include "Logger.h"
#include <iostream>
#include <iomanip>
#include <sstream>
#include <stdexcept>

Logger::Logger(const std::string& resultsDir)
    : resultsDir(resultsDir)
{}
// Join results directory with filename.
std::string Logger::path(const std::string& filename) const
{
    return resultsDir + "/" + filename;
}

void Logger::initSummary()
{
    std::ofstream file(path("summary.csv"));

    if (!file.is_open()) {
        throw std::runtime_error(
            "Logger: cannot create " + path("summary.csv") +
            "\nMake sure the results/ directory exists.");
    }

    // CSV header.
    file << "instance"       << ","
         << "method"         << ","
         << "rep"            << ","
         << "bestFitness"    << ","
         << "avgFitness"     << ","
         << "worstFitness"   << ","
         << "evaluations"    << ","
         << "timeMs"         << ","
         << "populationSize" << ","
         << "generations"    << ","
         << "mutationProb"   << ","
         << "crossoverProb"  << ","
         << "tournamentSize" << ","
         << "tabuSize"
         << "\n";

    std::cout << "Logger: summary file created at "
              << path("summary.csv") << "\n";
}

// Append one run to summary.csv.
void Logger::logRun(const std::string&  methodName,
                    const std::string&  instanceName,
                    int                 rep,
                    const SearchResult& result,
                    const Config&       cfg)
{
    std::ofstream file(path("summary.csv"), std::ios::app);

    if (!file.is_open()) {
        std::cerr << "Logger warning: cannot append to summary.csv\n";
        return;
    }

    file << instanceName                                          << ","
         << methodName                                           << ","
         << rep                                                  << ","
         << result.bestFitness                                   << ","
         << std::fixed << std::setprecision(2) << result.avgFitness  << ","
         << result.worstFitness                                  << ","
         << result.evaluations                                   << ","
         << std::fixed << std::setprecision(3) << result.timeMs      << ","
         << cfg.populationSize                                   << ","
         << cfg.generations                                      << ","
         << std::fixed << std::setprecision(3) << cfg.mutationProbability   << ","
         << std::fixed << std::setprecision(3) << cfg.crossoverProbability  << ","
         << cfg.tournamentSize                                   << ","
         << cfg.tabuSize
         << "\n";
}

// Write convergence history for a single run.
void Logger::logHistory(const std::string&  methodName,
                        const std::string&  instanceName,
                        int                 rep,
                        const SearchResult& result)
{
    std::string filename = "history_" + methodName + "_"
                         + instanceName + "_rep"
                         + std::to_string(rep) + ".csv";

    std::ofstream file(path(filename));

    if (!file.is_open()) {
        std::cerr << "Logger warning: cannot create " << filename << "\n";
        return;
    }

    // CSV header.
    file << "generation,best,average,worst\n";
    // One row per saved snapshot.
    for (int g = 0; g < static_cast<int>(result.history.size()); ++g) {
        const auto& snap = result.history[g];
        file << g                                                    << ","
             << snap.best                                            << ","
             << std::fixed << std::setprecision(2) << snap.avg       << ","
             << snap.worst
             << "\n";
    }
}