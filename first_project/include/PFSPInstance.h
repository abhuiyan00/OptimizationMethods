#pragma once

#include <vector>
#include <string>

class PFSPInstance {

public:
    explicit PFSPInstance(const std::string& filePath); // explicit means can't accidentally convert a string to PFSPInstance
    
    int getNumJobs() const ;
    int getNumMachines() const ;
    
    int getProcessingTime(int machine, int job) const ; // returs processing time of jobs on machine
    
    void print() const; 

private:

    int numJobs;
    int numMachines;
    std::vector<std::vector<int>> processingTimes;

    void loadFromFile(const std::string& filePath);
};

