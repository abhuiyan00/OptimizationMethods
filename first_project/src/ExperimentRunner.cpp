#include "ExperimentRunner.h"
#include "PFSPInstance.h"
#include "Individual.h"
#include "Population.h"
#include "RandomSearch.h"
#include "GreedySearch.h"
#include "GeneticAlgorithm.h"
#include "TabuSearch.h"
#include "ILS.h"

#include <iostream>
#include <iomanip>
#include <fstream>
#include <numeric>
#include <cmath>
#include <algorithm>
#include <limits>

ExperimentRunner::ExperimentRunner(const Config&      cfg,
                                   const std::string& dataDir,
                                   Logger&            logger)
    : cfg(cfg), dataDir(dataDir), logger(logger)
{}

namespace {

bool shouldRunILS(const std::string& instanceName)
{
    return instanceName == "tai20_5_0"
        || instanceName == "tai20_10_0"
        || instanceName == "tai20_20_0";
}

}

// Population standard deviation for best-fitness samples.
double ExperimentRunner::stdDev(const std::vector<int>& v) const
{
    if (v.size() < 2) return 0.0;
    double mean = static_cast<double>(
        std::accumulate(v.begin(), v.end(), 0)) / v.size();
    double sq = 0.0;
    for (int x : v) { double d = x - mean; sq += d * d; }
    return std::sqrt(sq / v.size());
}

void ExperimentRunner::runAll()
{
    const std::vector<std::pair<std::string, std::string>> instances = {
        { dataDir + "/tai20_5_0.txt",   "tai20_5_0"   },
        { dataDir + "/tai20_10_0.txt",  "tai20_10_0"  },
        { dataDir + "/tai20_20_0.txt",  "tai20_20_0"  },
        { dataDir + "/tai100_10_0.txt", "tai100_10_0" },
        { dataDir + "/tai100_20_0.txt", "tai100_20_0" },
        { dataDir + "/tai500_20_0.txt", "tai500_20_0" },
    };

    std::cout << "\n============================\n";
    std::cout << "  Full experiment: 6 instances\n";
    std::cout << "============================\n\n";

    for (const auto& instancePair : instances) {
        const std::string& file = instancePair.first;
        const std::string& name = instancePair.second;
        runInstance(file, name);
    }

    std::cout << "\nAll runs complete. Results written to results/\n";
}

// Run all configured methods on one instance.
void ExperimentRunner::runInstance(const std::string& instanceFile,
                                   const std::string& instanceName)
{
    std::cout << "\n--- Instance: " << instanceName << " ---\n";

    PFSPInstance instance(instanceFile);

    auto printResult = [](const SearchResult& r) {
        std::cout << "  best=" << r.bestFitness
                  << "  worst=" << r.worstFitness
                  << "  avg=" << std::fixed << std::setprecision(1)
                  << r.avgFitness;
    };

    // Greedy: one deterministic run.
    {
        std::cout << "  Greedy...";
        GreedySearch gs(instance);
        SearchResult r = gs.run();
        logger.logRun("Greedy", instanceName, 0, r, cfg);
        logger.logHistory("Greedy", instanceName, 0, r);
        printResult(r);
        std::cout << "  time=" << std::fixed << std::setprecision(2)
                  << r.timeMs << " ms\n";
    }

    // Random search: repeated with different seeds.
    {
        std::cout << "  RandomSearch (" << cfg.repetitions << " reps)...\n";
        for (int rep = 0; rep < cfg.repetitions; ++rep) {
            std::mt19937 rng(cfg.randomSeed + rep);
            RandomSearch rs(instance, cfg, rng);
            SearchResult r = rs.run();
            logger.logRun("RandomSearch", instanceName, rep, r, cfg);
            if (rep == 0) logger.logHistory("RandomSearch", instanceName, rep, r);
            std::cout << "    rep " << rep << "  ";
            printResult(r);
            std::cout << "\n";
        }
    }

    // GA: repeated with different seeds.
    {
        std::cout << "  GeneticAlgorithm (" << cfg.repetitions << " reps)...\n";
        for (int rep = 0; rep < cfg.repetitions; ++rep) {
            std::mt19937 rng(cfg.randomSeed + rep);
            GeneticAlgorithm ga(instance, cfg, rng);
            SearchResult r = ga.run();
            logger.logRun("GA", instanceName, rep, r, cfg);
            if (rep == 0) logger.logHistory("GA", instanceName, rep, r);
            std::cout << "    rep " << rep << "  ";
            printResult(r);
            std::cout << "\n";
        }
    }

    // Tabu search: repeated with different seeds.
    {
        std::cout << "  TabuSearch (" << cfg.repetitions << " reps)...\n";
        for (int rep = 0; rep < cfg.repetitions; ++rep) {
            std::mt19937 rng(cfg.randomSeed + rep);
            TabuSearch ts(instance, cfg, rng);
            SearchResult r = ts.run();
            logger.logRun("TS", instanceName, rep, r, cfg);
            if (rep == 0) logger.logHistory("TS", instanceName, rep, r);
            std::cout << "    rep " << rep << "  ";
            printResult(r);
            std::cout << "\n";
        }
    }

    // ILS: repeated with different seeds.
    {
        if (!shouldRunILS(instanceName)) {
            std::cout << "  ILS skipped for this instance (too expensive on this machine)\n";
            return;
        }

        std::cout << "  ILS (" << cfg.repetitions << " reps)...\n";

        // Put a time cap on very large instances.
        Config ilsCfg = cfg;
        if (instance.getNumJobs() >= 500) {
            ilsCfg.ilsMaxIterations = 100000;   // high cap
            ilsCfg.ilsTimeLimitSec  = 30.0;     // 30 seconds max per rep
        }

        for (int rep = 0; rep < cfg.repetitions; ++rep) {
            std::mt19937 rng(cfg.randomSeed + rep);
            ILS ils(instance, ilsCfg, rng);
            SearchResult r = ils.run();
            logger.logRun("ILS", instanceName, rep, r, ilsCfg);
            if (rep == 0) logger.logHistory("ILS", instanceName, rep, r);
            std::cout << "    rep " << rep
                      << "  best=" << r.bestFitness
                      << "  worst=" << r.worstFitness
                      << "  avg=" << std::fixed << std::setprecision(1)
                      << r.avgFitness << "\n";
        }
    }
}

void ExperimentRunner::logParamRow(const std::string& csvPath,
                                   const std::string& paramName,
                                   double             paramValue,
                                   const std::string& instanceName,
                                   int                bestFitness,
                                   double             avgFitness,
                                   double             sd,
                                   double             avgTimeMs)
{
    std::ofstream f(csvPath, std::ios::app);
    f << paramName    << ","
      << paramValue   << ","
      << instanceName << ","
      << bestFitness  << ","
      << std::fixed << std::setprecision(2) << avgFitness << ","
      << std::fixed << std::setprecision(2) << sd         << ","
      << std::fixed << std::setprecision(3) << avgTimeMs
      << "\n";
}

// Sweep selected GA, TS, and ILS parameters on one instance.
void ExperimentRunner::runParameterSweep(const std::string& instanceFile,
                                         const std::string& instanceName)
{
    std::cout << "\n============================\n";
    std::cout << "  Parameter sweep: " << instanceName << "\n";
    std::cout << "============================\n";

    PFSPInstance instance(instanceFile);

    std::string csvPath = "results/params_" + instanceName + ".csv";

    {
        std::ofstream f(csvPath);
        f << "paramName,paramValue,instance,bestFitness,"
             "avgFitness,stdDev,avgTimeMs\n";
    }

    // GA sweeps.

    auto sweepGA = [&](const std::string& paramName, double paramValue) {
        std::vector<int> bests;
        double timeSum = 0.0;

        for (int rep = 0; rep < cfg.repetitions; ++rep) {
            std::mt19937 rng(cfg.randomSeed + rep);
            GeneticAlgorithm ga(instance, cfg, rng);
            SearchResult r = ga.run();
            bests.push_back(r.bestFitness);
            timeSum += r.timeMs;
        }

        int    best = *std::min_element(bests.begin(), bests.end());
        double avg  = static_cast<double>(
            std::accumulate(bests.begin(), bests.end(), 0)) / bests.size();
        double sd   = stdDev(bests);
        double avgT = timeSum / bests.size();

        std::cout << "  " << std::setw(16) << paramName
                  << " = " << std::setw(6) << paramValue
                  << "   best=" << best
                  << "  avg=" << std::fixed << std::setprecision(1) << avg
                  << "  sd=" << std::setprecision(1) << sd << "\n";

        logParamRow(csvPath, paramName, paramValue,
                    instanceName, best, avg, sd, avgT);
    };

    Config baseline = cfg;

    std::cout << "\n  Sweeping mutationProb (Pm):\n";
    for (double pm : {0.01, 0.05, 0.1, 0.2, 0.4}) {
        cfg = baseline;
        cfg.mutationProbability = pm;
        sweepGA("mutationProb", pm);
    }

    std::cout << "\n  Sweeping crossoverProb (Px):\n";
    for (double px : {0.3, 0.5, 0.7, 0.9, 1.0}) {
        cfg = baseline;
        cfg.crossoverProbability = px;
        sweepGA("crossoverProb", px);
    }

    std::cout << "\n  Sweeping populationSize:\n";
    for (int ps : {20, 50, 100, 200}) {
        cfg = baseline;
        cfg.populationSize = ps;
        cfg.generations = (baseline.populationSize * baseline.generations) / ps;
        sweepGA("populationSize", ps);
    }

    std::cout << "\n  Sweeping generations:\n";
    for (int gens : {20, 50, 100, 200}) {
        cfg = baseline;
        cfg.generations = gens;
        cfg.populationSize = (baseline.populationSize * baseline.generations)
                              / gens;
        sweepGA("generations", gens);
    }

    std::cout << "\n  Sweeping tournamentSize:\n";
    for (int ts : {2, 3, 5, 10, 20}) {
        cfg = baseline;
        cfg.tournamentSize = ts;
        sweepGA("tournamentSize", ts);
    }

    // TS sweep.

    auto sweepTS = [&](const std::string& paramName, double paramValue) {
        std::vector<int> bests;
        double timeSum = 0.0;
        for (int rep = 0; rep < cfg.repetitions; ++rep) {
            std::mt19937 rng(cfg.randomSeed + rep);
            TabuSearch ts(instance, cfg, rng);
            SearchResult r = ts.run();
            bests.push_back(r.bestFitness);
            timeSum += r.timeMs;
        }
        int    best = *std::min_element(bests.begin(), bests.end());
        double avg  = static_cast<double>(std::accumulate(bests.begin(), bests.end(), 0))
                      / bests.size();
        double sd   = stdDev(bests);
        double avgT = timeSum / bests.size();
        std::cout << "  " << std::setw(16) << paramName
                  << " = " << paramValue
                  << "   best=" << best << "  avg=" << std::fixed
                  << std::setprecision(1) << avg << "\n";
        logParamRow(csvPath, paramName, paramValue, instanceName, best, avg, sd, avgT);
    };

    std::cout << "\n  Sweeping tabuSize (TS):\n";
    for (int tSize : {3, 5, 7, 10, 15, 20})
        { cfg = baseline; cfg.tabuSize = tSize; sweepTS("tabuSize", tSize); }

    // ILS sweeps.

    auto sweepILS = [&](const std::string& paramName, double paramValue) {
        std::vector<int> bests;
        double timeSum = 0.0;
        for (int rep = 0; rep < cfg.repetitions; ++rep) {
            std::mt19937 rng(cfg.randomSeed + rep);
            ILS ils(instance, cfg, rng);
            SearchResult r = ils.run();
            bests.push_back(r.bestFitness);
            timeSum += r.timeMs;
        }
        int    best = *std::min_element(bests.begin(), bests.end());
        double avg  = static_cast<double>(std::accumulate(bests.begin(), bests.end(), 0))
                      / bests.size();
        double sd   = stdDev(bests);
        double avgT = timeSum / bests.size();
        std::cout << "  " << std::setw(20) << paramName
                  << " = " << std::setw(6) << paramValue
                  << "   best=" << best
                  << "  avg=" << std::fixed << std::setprecision(1) << avg
                  << "  sd=" << std::setprecision(1) << sd << "\n";
        logParamRow(csvPath, paramName, paramValue, instanceName, best, avg, sd, avgT);
    };

    std::cout << "\n  Sweeping ILS perturbationStrength:\n";
    for (int ps : {2, 4, 6, 8, 12}) {
        cfg = baseline;
        cfg.perturbationStrength = ps;
        sweepILS("perturbStrength", ps);
    }

    std::cout << "\n  Sweeping ILS maxIterations:\n";
    for (int mi : {500, 1000, 2000, 5000}) {
        cfg = baseline;
        cfg.ilsMaxIterations = mi;
        sweepILS("ilsMaxIter", mi);
    }

    cfg = baseline;

    std::cout << "\nParameter sweep complete. Written to " << csvPath << "\n";
}