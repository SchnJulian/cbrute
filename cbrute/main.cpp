#include <iostream>
#include <chrono>
#include "Generator.h"

void helpmenu() {
    std::string help =
            R"(

### CBRUTE 0.1 ###

    Generate combinations/permutations with cbrute!

USAGE:
    cbrute [arguments]

OPTIONS:
    -h                      This help menu
    -log                    Print results to the console

ARGUMENTS:
    -l <value>              Length of the combination
    -c <values>             Character-set
    -f <path>               Path for output file with the combinations
    -perm                   No repetitions
                            cbrute will generate every possible permutation of the given characters
                            A custom -l argument will be ignored
    -ascii                  Full (printable) ASCII character-set
    -alphabetic <value>     Add full latin alphabet to character-set
                            l: lowercase letters
                            u: uppercase letters
    -alphabetic             Add full latin alphabet to character-set with both uppercase and lowercase
                            characters
    -digit                  Add digits from 0-9 to the character-set
    -special                Add special characters to the character-set
    -x <values>             Excluded characters
    -prepend <value>        Put results in front of a word
    -appendT <value>         Appends results to a word
    )";
    std::cout << help << std::endl;
}

int main(int argc, char *argv[]) {

    if (argc < 2) {
        std::cerr << "Not enough arguments. See -h for help." << std::endl;
        return 1;
    }

    for (int i = 1; i < argc; ++i) {
        if (std::string(argv[i]) == "-h") {
            helpmenu();
            return 0;
        }
    }

    auto generator = Generator(argc, argv);
    try {
        // Abort option
        if (!generator.confirmMemory()) {
            throw std::string("AbortedByUserException");
        }
        auto start = std::chrono::high_resolution_clock::now();
        generator.start();
        auto stop = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);

        std::cout << generator.getTotalN() << " combinations generated in " << duration.count() << " ms" << std::endl;

    } catch (const std::exception &exc) {
        std::cerr << "Looks like something went wrong. See -h for help." << std::endl;
        std::cerr << exc.what() << std::endl;
        return 1;
    }

    return 0;
}
