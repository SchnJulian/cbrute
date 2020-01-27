//
// Created by Julian Schnabel on 04.12.19.
//

#ifndef CBRUTE_GENERATOR_H
#define CBRUTE_GENERATOR_H


#include <cstdlib>
#include <string>
#include <vector>
#include <fstream>
#include <cmath>
#include <iostream>
#include <queue>
#include <map>
#include <algorithm>
#include <thread>

class Generator {
public:

    Generator(int argc, char **args);

    ~Generator();

    void start();

    bool isFileMode() const;

    bool confirmMemory();

    unsigned long getTotalN() const;

private:
    // Auxiliary functions
    bool checkArguments();

    bool parseArguments(int argc, char **args);

    bool initFileStream();

    std::mutex m;

    // Modes
    bool fileMode = false;
    bool log = false;
    bool perm = false;

    // Constants
    const std::string alphaL = "abcdefghijklmnopqrstuvwxyz";
    const std::string alphaU = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    const std::string digit = "0123456789";
    const std::string special = "!\"$%&'()*+,-./:;<=>?@[\\]^_`{|}~";
    const std::string ascii = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789!\"$%&'()*+,-./:;<=>?@[\\]^_`{|}~";
    static constexpr char defaultFileExtension[] = ".txt";

    // Strings to append/prepend to
    std::string appendT;
    std::string prependT;

    // Output functions
    void console();

    std::queue<std::string> printQueue;

    void addToQueue(std::string &str);

    void print();

    // Number of possible permutations/combinations
    unsigned long totalN = 0;
    size_t lengthMin = 0;
    size_t length;
    size_t lengthMax = 0;
    size_t elementCount = 0;
    // Amount of memory required in bytes
    long double approxMemory = 0;

    void memoryApproximation();

    void calculateN();

    void calculateTotalN();

    std::vector<char> characterSet;
    std::ofstream fileStream;
    std::string path;

    // Generator functions
    void getCombinations(unsigned long start, unsigned long end);

    void getPermutations();

    // Task vector
    std::vector<void (Generator::*)(std::string &comb)> tasks;

    bool initTasks();

    void append(std::string &str);

    void prepend(std::string &str);

    size_t getN(size_t temp);
};


#endif //CBRUTE_GENERATOR_H
