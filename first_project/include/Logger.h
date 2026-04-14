#pragma once

#include <string>
#include <fstream>
#include "SearchResult.h"
#include "Config.h"

// class to write the csv files and summary
class Logger {
public:

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