#include "PFSPInstance.h"
#include <fstream>    // for readin
#include <sstream>    // for parsing
#include <iostream>   // for printing
#include <stdexcept>  // for errors

PFSPInstance::PFSPInstance(const std::string & filePath): numJobs(0), numMachines(0) // setting members
{
  loadFromFile(filePath);
}

void PFSPInstance::loadFromFile(const std::string & filePath) {
  std::ifstream file(filePath);

  if (!file.is_open()) {
    throw std::runtime_error("Cannot open file: " + filePath);
  }

  std::string line;

  std::getline(file, line);

  std::getline(file, line);
  {
    std::istringstream ss(line); // treat the line as a stream we can read from
    int seed, ub, lb;
    ss >> numJobs >> numMachines >> seed >> ub >> lb;
  }

  std::getline(file, line);

  processingTimes.assign(numMachines, std::vector < int > (numJobs, 0));

  for (int m = 0; m < numMachines; ++m) {
    std::getline(file, line);
    std::istringstream ss(line);

    for (int j = 0; j < numJobs; ++j) {
      ss >> processingTimes[m][j];
    }
  }

  file.close();

  std::cout << "Loaded: " << filePath <<
    "  (" << numJobs << " jobs, " <<
    numMachines << " machines)\n";
}

int PFSPInstance::getNumJobs() const {
  return numJobs;
}
int PFSPInstance::getNumMachines() const {
  return numMachines;
}

int PFSPInstance::getProcessingTime(int machine, int job) const {
  return processingTimes.at(machine).at(job);
}

void PFSPInstance::print() const {
  std::cout << "Processing times matrix (" <<
    numMachines << " machines x " <<
    numJobs << " jobs):\n";

  for (int m = 0; m < numMachines; ++m) {
    std::cout << "  Machine " << m << ": ";
    for (int j = 0; j < numJobs; ++j) {
      std::cout << processingTimes[m][j];
      if (j < numJobs - 1) std::cout << "\t"; // tab between values
    }
    std::cout << "\n";
  }
}