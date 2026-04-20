#pragma once

#include <vector>
#include <string>

class PFSPInstance {

public:
    // Load a Taillard instance from file.
    explicit PFSPInstance(const std::string& filePath);
    
    int getNumJobs() const ;
    int getNumMachines() const ;
    
    // Processing time of job on a machine.
    int getProcessingTime(int machine, int job) const ;
    
    void print() const; 

private:

    int numJobs;
    int numMachines;
    std::vector<std::vector<int>> processingTimes;

    void loadFromFile(const std::string& filePath);
};

