#include "Logger.h"
#include <iostream>
#include <iomanip>
#include <sstream>
#include <stdexcept>

// ---------------------------------------------------------------
// Constructor
// ---------------------------------------------------------------
Logger::Logger(const std::string& resultsDir)
    : resultsDir(resultsDir)
{}

// ---------------------------------------------------------------
// path — builds a full path from the results directory + filename
// ---------------------------------------------------------------
std::string Logger::path(const std::string& filename) const
{
    return resultsDir + "/" + filename;
}

// ---------------------------------------------------------------
// initSummary
//
// Creates results/summary.csv and writes the CSV header.
// std::ofstream with default mode truncates (overwrites) any existing file.
// ---------------------------------------------------------------
void Logger::initSummary()
{
    std::ofstream file(path("summary.csv"));

    if (!file.is_open()) {
        throw std::runtime_error(
            "Logger: cannot create " + path("summary.csv") +
            "\nMake sure the results/ directory exists.");
    }

    // CSV header — each column name separated by commas
    // These column names match exactly what logRun() writes below,
    // so they always stay in sync.
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

// ---------------------------------------------------------------
// logRun
//
// Appends one data row to summary.csv.
// std::ios::app opens the file in APPEND mode — existing rows
// are preserved and the new row is added at the bottom.
// ---------------------------------------------------------------
void Logger::logRun(const std::string&  methodName,
                    const std::string&  instanceName,
                    int                 rep,
                    const SearchResult& result,
                    const Config&       cfg)
{
    // Open in append mode — do not overwrite existing rows
    std::ofstream file(path("summary.csv"), std::ios::app);

    if (!file.is_open()) {
        std::cerr << "Logger warning: cannot append to summary.csv\n";
        return;   // non-fatal — print warning but don't crash
    }

    // Write one CSV row with fixed-precision floating point values
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

// ---------------------------------------------------------------
// logHistory
//
// Writes a new CSV file containing the per-generation fitness history
// for one specific run.  Filename example:
//   history_GA_tai20_5_0_rep3.csv
// ---------------------------------------------------------------
void Logger::logHistory(const std::string&  methodName,
                        const std::string&  instanceName,
                        int                 rep,
                        const SearchResult& result)
{
    // Build filename: history_GA_tai20_5_0_rep0.csv
    std::string filename = "history_" + methodName + "_"
                         + instanceName + "_rep"
                         + std::to_string(rep) + ".csv";

    std::ofstream file(path(filename));

    if (!file.is_open()) {
        std::cerr << "Logger warning: cannot create " << filename << "\n";
        return;
    }

    // Header
    file << "generation,best,average,worst\n";

    // One row per snapshot in the history vector
    for (int g = 0; g < static_cast<int>(result.history.size()); ++g) {
        const auto& snap = result.history[g];
        file << g                                                    << ","
             << snap.best                                            << ","
             << std::fixed << std::setprecision(2) << snap.avg       << ","
             << snap.worst
             << "\n";
    }

    // No explicit close needed — the destructor closes the file when
    // 'file' goes out of scope at the end of this function.
    // This is RAII: Resource Acquisition Is Initialisation.
    // The file is a resource; its lifetime is tied to the object.
}