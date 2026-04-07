#pragma once

#include <string>
#include <fstream>
#include "SearchResult.h"
#include "Config.h"

// Logger class for writing results to CSV files, both summary and history as .csv
class Logger {
public:

    // resultsDir: folder where all CSV files will be saved, on existing folder
    explicit Logger(const std::string& resultsDir);

    void initSummary();

    void logRun(const std::string&  methodName,
                const std::string&  instanceName,
                int                 rep,
                const SearchResult& result,
                const Config&       cfg);

    void logHistory(const std::string&  methodName,
                    const std::string&  instanceName,
                    int                 rep,
                    const SearchResult& result);

private:
    std::string resultsDir;

    std::string path(const std::string& filename) const;
};