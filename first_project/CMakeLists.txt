cmake_minimum_required(3.16)
project(PFSP)

# features
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

# location of headers
include_directories(include)

# source files
add_executable(pfsp
    src/main.cpp
    src/Config.cpp
    src/PFSPInstance.cpp
    src/Individual.cpp
    src/Population.cpp
    src/RandomSearch.cpp
    src/GreedySearch.cpp
    src/GeneticAlgorithm.cpp
    src/TabuSearch.cpp
    src/Logger.cpp
    src/ExperimentRunner.cpp
    src/ILS.cpp
)
