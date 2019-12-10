//
// Created by Julian Schnabel on 04.12.19.
//



#include <cmath>
#include <iostream>
#include <map>
#include "Generator.h"

template <typename Callable>
void Generator::getCombinations(Callable func) {
    // Fill with dummy elements
    std::string comb(length, ' ');
    auto total_n = static_cast<unsigned long>(std::pow(elementCount, length));
    combinations.reserve(total_n);
    for (auto i = 0UL; i < total_n; ++i) {
        auto n = i;
        for (size_t j = 0; j < length; ++j) {
            comb[comb.size() - j - 1] = charPool[n % elementCount];
            n /= elementCount;
        }
        func(comb);
    }
}

template <typename Callable>
void Generator::getPermutations(Callable func) {
    std::string perm(std::begin(charPool), std::end(charPool));
    std::sort(std::begin(perm), std::end(perm));
    do {
        func(perm);
    } while (std::next_permutation(std::begin(perm), std::end(perm)));
}

bool Generator::initFileSystem() {
    outStream.open(path);
    return outStream.good();
}

size_t factorial(size_t n) { return (n == 1L || n == 0L) ? 1L : factorial(n - 1L) * n; }

void Generator::start() {
    if (!ready) {
        throw std::string("generatorNotReadyException");
    }

    unsigned long total_n = 0;
    if (perm) {
        total_n = factorial(elementCount);
    } else {
        total_n = static_cast<unsigned long>(std::pow(elementCount, length));
    }

    if (memoryApproximation(total_n)) {
        if (perm) {
            getPermutations([this](auto &perm) {
                this->combinations.emplace_back(perm);
            });
        } else {
            getCombinations([this](auto &comb) {
                this->combinations.emplace_back(comb);
            });
        }
        if (!path.empty()) {
            for (auto &c : combinations) {
                outStream << c << "\n";
            }
        }
    }
}

bool Generator::memoryApproximation(unsigned long total_n) {
    if (perm) {
        approxMemory = sizeof(char) * elementCount * total_n;
    } else {
        approxMemory = sizeof(char) * length * total_n;
    }

    auto unit = "B";
    auto divisor = 1L;
    auto UD = std::make_pair(unit, divisor);
    if (approxMemory >= 1000 && approxMemory < 1000000) {
        UD.first = "KB";
        UD.second = 1000;
    } else if (approxMemory >= 1000000 && approxMemory < 1000000000) {
        UD.first = "MB";
        UD.second = 1000000;
    } else if (approxMemory >= 1000000000 && approxMemory < 1000000000000) {
        UD.first = "GB";
        UD.second = 1000000000;
    } else if (approxMemory >= 1000000000000) {
        UD.first = "TB";
        UD.second = 1000000000000;
    }
    std::cout << "This operation will produce an array of strings that will allocate " << approxMemory / UD.second
              << UD.first << "\n" << "Are you sure you want to continue? [y/n] : ";
    char input = 'y';

    std::cin >> input;
    return (std::tolower(input) == 'y');
}

Generator::~Generator() {
    outStream.close();
}

Generator::Generator(int argc, char **args) {
    bool ready = false;
    if (parseArguments(argc, args)) {
        if (checkArguments()) {
            if (path.empty()) {
                ready = true;
            } else {
                ready = initFileSystem();
            }
        }
    }
    this->ready = ready;
}

bool Generator::parseArguments(int argc, char **args) {
    std::map<std::string, std::string> map;
    auto length = std::make_pair("-l", "");
    auto characters = std::make_pair("-c", "");
    auto filePath = std::make_pair("-f", "");
    auto perm = std::make_pair("-perm", "");
    auto log = std::make_pair("-log", "");
    auto alphabetic = std::make_pair("-alphabetic", "");
    auto digit = std::make_pair("-digit", "");
    auto special = std::make_pair("-special", "");
    auto ascii = std::make_pair("-ascii", "");
    auto exclude = std::make_pair("-x", "");

    map.insert(length);
    map.insert(characters);
    map.insert(filePath);
    map.insert(perm);
    map.insert(log);
    map.insert(alphabetic);
    map.insert(digit);
    map.insert(special);
    map.insert(ascii);
    map.insert(exclude);

    for (int i = 1; i < argc - 1; ++i) {
        if (map.find(args[i]) != std::end(map)) {
            map.find(args[i])->second = args[i + 1];
        }
    }
    // Check last argument
    if (map.find(args[argc - 1]) != std::end(map)) map.find(args[argc - 1])->second = "1";

    try {
        this->perm = !(map.at(perm.first).empty());
        if (!this->perm) {
            this->length = std::stoul(map.at("-l"));
        }
        this->path = map.at(filePath.first);
        this->log = !(map.at(log.first).empty());
    } catch (std::invalid_argument &e) {
        return false;
    }
    // Check if full ascii set was activated
    if (!(map.at(ascii.first).empty())) {
        std::copy(this->ascii.begin(), this->ascii.end(), std::back_inserter(this->charPool));
    } else {
        // Initialize alphabetic character set
        if (map.at(alphabetic.first) == "u") {
            std::copy(this->alphaU.begin(), this->alphaU.end(), std::back_inserter(this->charPool));
        } else if (map.at(alphabetic.first) == "l") {
            std::copy(this->alphaL.begin(), this->alphaL.end(), std::back_inserter(this->charPool));
        } else if (!(map.at(alphabetic.first).empty())) {
            std::copy(this->alphaU.begin(), this->alphaU.end(), std::back_inserter(this->charPool));
            std::copy(this->alphaL.begin(), this->alphaL.end(), std::back_inserter(this->charPool));
        }

        // Initialize numeric character set
        if (!(map.at(digit.first).empty()))
            std::copy(this->digit.begin(), this->digit.end(), std::back_inserter(this->charPool));

        // Initialize special character set
        if (!(map.at(special.first).empty()))
            std::copy(this->special.begin(), this->special.end(), std::back_inserter(this->charPool));

        // Initialize character set
        std::string temp = map.at(characters.first);
        std::copy(temp.begin(), temp.end(), std::back_inserter(this->charPool));

        std::vector<char> tempX(std::begin(map.at(exclude.first)), std::end(map.at(exclude.first)));
        std::sort(std::begin(tempX), std::end(tempX));
        std::sort(std::begin(this->charPool), std::end(this->charPool));
        if (!(map.at(exclude.first).empty())) {
            this->charPool.erase(std::remove_if(std::begin(this->charPool), std::end(this->charPool), [&tempX](char c) {
                return std::binary_search(std::begin(tempX), std::end(tempX), c);
            }), std::end(this->charPool));

        }
    }

    // Remove duplicates
    this->charPool.erase(std::unique(std::begin(this->charPool), std::end(this->charPool)), std::end(this->charPool));
    this->elementCount = charPool.size();

    return true;
}

bool Generator::isLog() const {
    return log;
}

bool Generator::checkArguments() {
    if (perm && elementCount > 1) return true;
    return length > 1 && elementCount > 1;
}

bool Generator::isReady() const {
    return this->ready;
}

