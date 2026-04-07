#include "ExperimentRunner.h"
#include "PFSPInstance.h"
#include "Individual.h"
#include "Population.h"
#include "RandomSearch.h"
#include "GreedySearch.h"
#include "GeneticAlgorithm.h"
#include "TabuSearch.h"

#include <iostream>
#include <iomanip>
#include <fstream>
#include <numeric>
#include <cmath>
#include <algorithm>
#include <limits>

// ---------------------------------------------------------------
// Constructor — store cfg by value so runParameterSweep can
// modify a local copy without affecting the caller's config
// ---------------------------------------------------------------
ExperimentRunner::ExperimentRunner(const Config&      cfg,
                                   const std::string& dataDir,
                                   Logger&            logger)
    : cfg(cfg), dataDir(dataDir), logger(logger)
{}

// ---------------------------------------------------------------
// stdDev helper
// ---------------------------------------------------------------
double ExperimentRunner::stdDev(const std::vector<int>& v) const
{
    if (v.size() < 2) return 0.0;
    double mean = static_cast<double>(
        std::accumulate(v.begin(), v.end(), 0)) / v.size();
    double sq = 0.0;
    for (int x : v) { double d = x - mean; sq += d * d; }
    return std::sqrt(sq / v.size());
}

// ---------------------------------------------------------------
// runAll — iterates over all six Taillard instances
// ---------------------------------------------------------------
void ExperimentRunner::runAll()
{
    // The six required instances as (filename, display-name) pairs.
    // Adjust the filenames if your data/ folder uses different names.
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

// ---------------------------------------------------------------
// runInstance — runs all methods on one Taillard instance
// ---------------------------------------------------------------
void ExperimentRunner::runInstance(const std::string& instanceFile,
                                   const std::string& instanceName)
{
    std::cout << "\n--- Instance: " << instanceName << " ---\n";

    PFSPInstance instance(instanceFile);

    // ---------- Greedy (once, no repetitions) ----------
    {
        std::cout << "  Greedy...";
        GreedySearch gs(instance);
        SearchResult r = gs.run();
        logger.logRun("Greedy", instanceName, 0, r, cfg);
        logger.logHistory("Greedy", instanceName, 0, r);
        std::cout << "  best=" << r.bestFitness
                  << "  time=" << std::fixed << std::setprecision(2)
                  << r.timeMs << " ms\n";
    }

    // ---------- Random Search (repeated) ----------
    {
        std::cout << "  RandomSearch (" << cfg.repetitions << " reps)...\n";
        for (int rep = 0; rep < cfg.repetitions; ++rep) {
            std::mt19937 rng(cfg.randomSeed + rep);
            RandomSearch rs(instance, cfg, rng);
            SearchResult r = rs.run();
            logger.logRun("RandomSearch", instanceName, rep, r, cfg);
            // Only save history for rep 0 — saves disk space
            if (rep == 0) logger.logHistory("RandomSearch", instanceName, rep, r);
            std::cout << "    rep " << rep << "  best=" << r.bestFitness << "\n";
        }
    }

    // ---------- Genetic Algorithm (repeated) ----------
    {
        std::cout << "  GeneticAlgorithm (" << cfg.repetitions << " reps)...\n";
        for (int rep = 0; rep < cfg.repetitions; ++rep) {
            std::mt19937 rng(cfg.randomSeed + rep);
            GeneticAlgorithm ga(instance, cfg, rng);
            SearchResult r = ga.run();
            logger.logRun("GA", instanceName, rep, r, cfg);
            if (rep == 0) logger.logHistory("GA", instanceName, rep, r);
            std::cout << "    rep " << rep << "  best=" << r.bestFitness << "\n";
        }
    }

    // ---------- Tabu Search (repeated) ----------
    {
        std::cout << "  TabuSearch (" << cfg.repetitions << " reps)...\n";
        for (int rep = 0; rep < cfg.repetitions; ++rep) {
            std::mt19937 rng(cfg.randomSeed + rep);
            TabuSearch ts(instance, cfg, rng);
            SearchResult r = ts.run();
            logger.logRun("TS", instanceName, rep, r, cfg);
            if (rep == 0) logger.logHistory("TS", instanceName, rep, r);
            std::cout << "    rep " << rep << "  best=" << r.bestFitness << "\n";
        }
    }
}

// ---------------------------------------------------------------
// logParamRow — appends one row to a parameter sweep CSV
// ---------------------------------------------------------------
void ExperimentRunner::logParamRow(const std::string& csvPath,
                                   const std::string& paramName,
                                   double             paramValue,
                                   const std::string& instanceName,
                                   int                bestFitness,
                                   double             avgFitness,
                                   double             sd,
                                   double             avgTimeMs)
{
    // Open in append mode — header was written by runParameterSweep
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

// ---------------------------------------------------------------
// runParameterSweep
//
// Varies each GA parameter one at a time while holding the others
// at their defaults. For each value, runs cfg.repetitions times
// and logs the averaged results.
//
// Parameters swept:
//   Pm (mutationProb):   0.01, 0.05, 0.1, 0.2, 0.4
//   Px (crossoverProb):  0.3, 0.5, 0.7, 0.9, 1.0
//   populationSize:      20, 50, 100, 200
//   generations:         20, 50, 100, 200
//   tournamentSize:      2, 3, 5, 10, 20
// ---------------------------------------------------------------
void ExperimentRunner::runParameterSweep(const std::string& instanceFile,
                                         const std::string& instanceName)
{
    std::cout << "\n============================\n";
    std::cout << "  Parameter sweep: " << instanceName << "\n";
    std::cout << "============================\n";

    PFSPInstance instance(instanceFile);

    // CSV path for parameter sweep results
    std::string csvPath = "results/params_" + instanceName + ".csv";

    // Write header
    {
        std::ofstream f(csvPath);
        f << "paramName,paramValue,instance,bestFitness,"
             "avgFitness,stdDev,avgTimeMs\n";
    }

    // Helper lambda: runs GA cfg.repetitions times with the current cfg,
    // collects stats, and logs one row to the sweep CSV.
    // Captures cfg, instance, instanceName, csvPath by reference.
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

    // Save the baseline config so we can restore it after each sweep
    Config baseline = cfg;

    // ---- Sweep: Mutation probability (Pm) ----
    std::cout << "\n  Sweeping mutationProb (Pm):\n";
    for (double pm : {0.01, 0.05, 0.1, 0.2, 0.4}) {
        cfg = baseline;
        cfg.mutationProbability = pm;
        sweepGA("mutationProb", pm);
    }

    // ---- Sweep: Crossover probability (Px) ----
    std::cout << "\n  Sweeping crossoverProb (Px):\n";
    for (double px : {0.3, 0.5, 0.7, 0.9, 1.0}) {
        cfg = baseline;
        cfg.crossoverProbability = px;
        sweepGA("crossoverProb", px);
    }

    // ---- Sweep: Population size ----
    std::cout << "\n  Sweeping populationSize:\n";
    for (int ps : {20, 50, 100, 200}) {
        cfg = baseline;
        cfg.populationSize = ps;
        // Keep total budget constant: reduce generations proportionally
        cfg.generations = (baseline.populationSize * baseline.generations) / ps;
        sweepGA("populationSize", ps);
    }

    // ---- Sweep: Number of generations ----
    std::cout << "\n  Sweeping generations:\n";
    for (int gens : {20, 50, 100, 200}) {
        cfg = baseline;
        cfg.generations = gens;
        // Keep total budget constant: adjust popSize proportionally
        cfg.populationSize = (baseline.populationSize * baseline.generations)
                              / gens;
        sweepGA("generations", gens);
    }

    // ---- Sweep: Tournament size ----
    std::cout << "\n  Sweeping tournamentSize:\n";
    for (int ts : {2, 3, 5, 10, 20}) {
        cfg = baseline;
        cfg.tournamentSize = ts;
        sweepGA("tournamentSize", ts);
    }

    cfg = baseline;   // restore to original defaults

    std::cout << "\nParameter sweep complete. Written to " << csvPath << "\n";
}