//
// Created by Julian Schnabel on 04.12.19.
//

#ifndef CBRUTE_GENERATOR_H
#define CBRUTE_GENERATOR_H


#include <cstdlib>
#include <string>
#include <vector>
#include <fstream>

/**
USAGE:
cbrute [arguments]

OPTIONS:
-h				This help menu
-log				Print results to the console

ARGUMENTS:
-l <value>			Length of the combination
-c <values>		Character-set
-f <path>			Path for output file with the combinations
-perm			No repetitions. cbrute will generate every possible permutation of the given 					characters
A custom -l argument will be ignored
-ascii				Full (printable) ASCII character-set
-alphabetic <value>	Add full latin alphabet to character-set
        l	: lowercase letters
u 	: uppercase letters
-alphabetic		Add full latin alphabet to character-set with both uppercase and lowercase
        characters
-digit				Add digits from 0-9 to the character-set
-special			Add special characters to the character-set
*/




class Generator {

public:
    Generator(int argc, char **args);
    ~Generator();
    bool parseArguments(int argc, char **args);
    std::vector<std::string> combinations;

    bool initFileSystem();
    void start();

    [[nodiscard]] bool isLog() const;
    [[nodiscard]] bool isReady() const;
private:
    const std::string alphaL = "abcdefghijklmnopqrstuvwxyz";
    const std::string alphaU = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    const std::string digit = "0123456789";
    const std::string special = "!\"$%&'()*+,-./:;<=>?@[\\]^_`{|}~";
    const std::string ascii = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789!\"$%&'()*+,-./:;<=>?@[\\]^_`{|}~";
    bool log = false;

    size_t length = 0;
    size_t elementCount = 0;
    size_t approxMemory = 0;
    bool perm = false;
    std::vector<char> charPool;
    std::ofstream outStream;
    std::string path;
    bool ready = false;

    template <typename Callable>
    void getCombinations(Callable func);
    template <typename Callable>
    void getPermutations(Callable func);

    bool memoryApproximation(unsigned long total_n);

    bool checkArguments();


};


#endif //CBRUTE_GENERATOR_H
