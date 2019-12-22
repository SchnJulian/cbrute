//
// Created by Julian Schnabel on 04.12.19.
//

#ifndef CBRUTE_GENERATOR_H
#define CBRUTE_GENERATOR_H


#include <cstdlib>
#include <string>
#include <vector>
#include <fstream>


class Generator {


public:
    Generator(int argc, char **args);

    ~Generator();

    bool parseArguments(int argc, char **args);

    bool initTasks();

    void start();

    [[nodiscard]] bool isLog() const;

    [[nodiscard]] bool isReady() const;


    bool confirmMemory();

    unsigned long getTotalN() const;

private:
    const std::string alphaL = "abcdefghijklmnopqrstuvwxyz";
    const std::string alphaU = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    const std::string digit = "0123456789";
    const std::string special = "!\"$%&'()*+,-./:;<=>?@[\\]^_`{|}~";
    const std::string ascii = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789!\"$%&'()*+,-./:;<=>?@[\\]^_`{|}~";
    std::string appendT;
    std::string prependT;
    bool log = false;
    std::vector<void (Generator::*)(std::string &comb)> tasks;

    void consoleA(std::string &comb);

    void consoleP(std::string &comb);

    void fileA(std::string &comb);

    void fileP(std::string &comb);

    void console(std::string &comb);

    void file(std::string &comb);

    unsigned long totalN = 0;
    size_t length = 0;
    size_t elementCount = 0;
    long double approxMemory = 0;
    bool perm = false;
    std::vector<char> charPool;
    std::ofstream fileStream;
    std::string path;
    bool ready = false;


    template <typename Callable>
    void getCombinations(Callable func);
    template <typename Callable>
    void getPermutations(Callable func);

    void memoryApproximation();

    bool checkArguments();


};


#endif //CBRUTE_GENERATOR_H
