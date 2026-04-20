# PFSP Metaheuristics Benchmark (C++)

This repository contains a C++ benchmark framework for the Permutation Flow Shop Scheduling Problem (PFSP) on Taillard-style instances. It implements multiple search strategies, runs repeated experiments, exports CSV logs, and generates publication-ready plots with Python.

## What This Project Does

- Loads PFSP benchmark instances from `data/`.
- Solves each instance with multiple methods:
	- Greedy constructive baseline
	- Random Search
	- Genetic Algorithm (GA)
	- Tabu Search (TS)
	- Iterated Local Search (ILS)
- Repeats stochastic methods for multiple seeds.
- Writes detailed run summaries and convergence history to `results/`.
- Runs parameter sweeps for GA, TS, and ILS on `tai20_5_0`.
- Generates comparison and sensitivity plots under `results/plots/`.

## Problem Setting

The project targets PFSP where a permutation of jobs is evaluated by total flow time (lower is better). A solution is a job order used on all machines.

The default benchmark set currently includes:

- `tai20_5_0`
- `tai20_10_0`
- `tai20_20_0`
- `tai100_10_0`
- `tai100_20_0`
- `tai500_20_0`

## Repository Structure

```text
.
|- CMakeLists.txt
|- src/
|- include/
|- data/
|- results/
|  |- summary.csv
|  |- history_*.csv
|  |- params_*.csv
|  |- plots/
|- generate_plots.py
```

## Implemented Methods

### 1) GreedySearch
- Deterministic constructive baseline.
- Builds a sequence incrementally using partial flow-time evaluation.
- One run per instance.

### 2) RandomSearch
- Samples random permutations under a fixed evaluation budget.
- Tracks best and worst encountered fitness.
- Saves snapshot history by evaluation chunks.

### 3) GeneticAlgorithm (GA)
- Population-based search with:
	- Tournament selection
	- Order crossover (OX)
	- Swap mutation
	- Elitism (best individual survives)
- Logs generation-level convergence snapshots.

### 4) TabuSearch (TS)
- Neighborhood search over pairwise swap moves.
- Uses tabu list memory to avoid short cycles.
- Includes aspiration behavior through admissibility checks in move selection.

### 5) Iterated Local Search (ILS)
- NEH-based initialization.
- Insertion local search to reach local optima.
- Random swap perturbation to escape local minima.
- Restart mechanism when no improvement persists.
- Optional time limit support for expensive cases.

## Default Configuration

Configuration is centralized in `include/Config.h`.

Key defaults:

- GA:
	- `populationSize = 100`
	- `generations = 100`
	- `crossoverProbability = 0.7`
	- `mutationProbability = 0.01`
	- `tournamentSize = 5`
- TS:
	- `tabuSize = 7`
- ILS:
	- `ilsMaxIterations = 2000`
	- `perturbationStrength = 4`
	- `ilsTimeLimitSec = 0.0` (disabled)
- General:
	- `randomSeed = 42`
	- `repetitions = 10`

## Build Instructions

### Option A: CMake (recommended)

From repository root:

```powershell
cmake -S . -B build
cmake --build build --config Release
```

Run:

```powershell
./build/pfsp
```

On Windows with a multi-config generator, executable may be under `build/Release/`.

### Option B: VS Code Task (MinGW)

Use the workspace build task:

- `C/C++: g++.exe build active file`

This builds `pfsp.exe` from the project sources with C++17 enabled.

## Running Experiments

Main entrypoint is `src/main.cpp`.

Current run flow:

1. Initialize logger and create `results/summary.csv`.
2. Run the full benchmark across 6 instances.
3. Run parameter sweep on `tai20_5_0`.

Execute binary from repository root so relative paths resolve correctly (`data/` and `results/`).

## Output Files

### `results/summary.csv`
One row per method x instance x repetition, containing:

- `instance`
- `method`
- `rep`
- `bestFitness`
- `avgFitness`
- `worstFitness`
- `evaluations`
- `timeMs`
- active config fields (`populationSize`, `generations`, `mutationProb`, `crossoverProb`, `tournamentSize`, `tabuSize`)

### `results/history_<Method>_<Instance>_rep<k>.csv`
Convergence snapshots with columns:

- `generation`
- `best`
- `average`
- `worst`

### `results/params_<Instance>.csv`
Parameter sweep results with columns:

- `paramName`
- `paramValue`
- `instance`
- `bestFitness`
- `avgFitness`
- `stdDev`
- `avgTimeMs`

## Plot Generation

`generate_plots.py` reads CSV outputs and writes figures in `results/plots/`.

### Python dependencies

```powershell
pip install pandas matplotlib numpy
```

### Run plotting

```powershell
python generate_plots.py
```

Generated artifacts include:

- Convergence curves (`convergence_*.png`)
- Instance-group comparison bars (`comparison_*.png`)
- Runtime comparison (`time_comparison.png`)
- Parameter sensitivity curves (`param_*.png`)
- Aggregated table (`summary_table.csv`)

## Reproducibility Notes

- Stochastic methods use deterministic seed sequences: `randomSeed + rep`.
- Repetitions are controlled via `Config::repetitions`.
- For fair comparisons, keep evaluation budgets and dataset set fixed.
- ILS may be restricted to selected instances when runtime is high.

## Data Format Notes

Instances are loaded by `PFSPInstance` from Taillard-style text files. The loader reads:

1. Header line (ignored)
2. Problem dimensions line (`numJobs`, `numMachines`, metadata)
3. Spacer line
4. Processing-time matrix (`numMachines` rows, `numJobs` columns)

## Extending the Framework

To add a new method:

1. Add header/source in `include/` and `src/`.
2. Return results through the shared `SearchResult` container.
3. Integrate runs and logging in `ExperimentRunner`.
4. Register source file in `CMakeLists.txt`.
5. Optionally add method color/order in `generate_plots.py`.

## Troubleshooting

- If `summary.csv` cannot be created, ensure `results/` exists.
- If plots are missing, verify matching `history_*.csv` and `summary.csv` files exist.
- If binary cannot find data, run from repository root (or adjust paths in `main.cpp`).

## License

No license file is currently included in this repository. Add a `LICENSE` file before external distribution.
