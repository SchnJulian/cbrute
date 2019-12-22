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
    for (auto i = 0UL; i < total_n; ++i) {
        auto n = i;
        for (size_t j = 0; j < length; ++j) {
            comb[comb.size() - j - 1] = charPool[n % elementCount];
            n /= elementCount;
        }
        for (auto f : func) (this->*f)(comb);
    }
}

template <typename Callable>
void Generator::getPermutations(Callable func) {
    std::string perm(std::begin(charPool), std::end(charPool));
    std::sort(std::begin(perm), std::end(perm));
    do {
        for (auto f : func) (this->*f)(perm);
    } while (std::next_permutation(std::begin(perm), std::end(perm)));
}

void Generator::console(std::string &comb) {
    std::cout << comb << '\n';
}

void Generator::file(std::string &comb) {
    fileStream << comb << '\n';
}


bool Generator::initTasks() {
    if (!appendT.empty()) tasks.push_back(&Generator::append);
    if (!prependT.empty()) tasks.push_back(&Generator::prepend);
    if (!path.empty()) {
        fileStream.open(path);
        tasks.push_back(&Generator::file);
        return fileStream.good();
    }
    if (log) {
        tasks.push_back(&Generator::console);
    }
    return true;
}

size_t factorial(size_t n) { return (n == 1L || n == 0L) ? 1L : factorial(n - 1L) * n; }

void Generator::start() {
    if (perm) {
        getPermutations(tasks);
    } else {
        getCombinations(tasks);
    }
}

void Generator::memoryApproximation() {

    if (perm) {
        totalN = factorial(elementCount);
    } else {
        totalN = static_cast<unsigned long>(std::pow(elementCount, length));
    }
    if (perm) {
        approxMemory = sizeof(char) * elementCount * totalN;
    } else {
        approxMemory = sizeof(char) * length * totalN;
    }
}

bool Generator::confirmMemory() {
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
    return (tolower(input) == 'y');
}

Generator::~Generator() {
    fileStream.close();
}

Generator::Generator(int argc, char **args) {
    bool ready = false;
    if (parseArguments(argc, args)) {
        if (checkArguments()) {
            ready = initTasks();
        }
    }
    // Approximate memory
    memoryApproximation();
    this->ready = ready;
    if (!ready) {
        throw std::string("generatorNotReadyException");
    }
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
    auto prepend = std::make_pair("-prepend", "");
    auto append = std::make_pair("-append", "");

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
    map.insert(prepend);
    map.insert(append);

    // Iterate through arg string
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
            this->length = std::stoul(map.at(length.first));
        }
        this->path = map.at(filePath.first);
        this->log = !(map.at(log.first).empty());
    } catch (std::invalid_argument &e) {
        return false;
    }

    // String to appendT to
    if (!(map.at(append.first).empty()))
        std::copy(std::begin(map.at(append.first)), std::end(map.at(append.first)), std::back_inserter(this->appendT));

    // String to prepend before
    if (!(map.at(prepend.first).empty()))
        std::copy(std::begin(map.at((prepend.first))), std::end(map.at((prepend.first))),
                  std::back_inserter(this->prependT));

    // Check if full ascii set was activated
    if (!(map.at(ascii.first).empty())) {
        std::copy(std::begin(this->ascii), std::end(this->ascii), std::back_inserter(this->charPool));
    } else {
        // Initialize alphabetic character set
        if (map.at(alphabetic.first) == "u") {
            std::copy(std::begin(this->alphaU), std::end(this->alphaU), std::back_inserter(this->charPool));
        } else if (map.at(alphabetic.first) == "l") {
            std::copy(std::begin(this->alphaL), std::end(this->alphaL), std::back_inserter(this->charPool));
        } else if (!(map.at(alphabetic.first).empty())) {
            std::copy(std::begin(this->alphaU), std::end(this->alphaU), std::back_inserter(this->charPool));
            std::copy(std::begin(this->alphaL), std::end(this->alphaL), std::back_inserter(this->charPool));
        }

        // Initialize numeric character set
        if (!(map.at(digit.first).empty()))
            std::copy(std::begin(this->digit), std::end(this->digit), std::back_inserter(this->charPool));

        // Initialize special character set
        if (!(map.at(special.first).empty()))
            std::copy(std::begin(this->special), std::end(this->special), std::back_inserter(this->charPool));

        // Initialize character set
        std::string temp = map.at(characters.first);
        std::copy(std::begin(temp), std::end(temp), std::back_inserter(this->charPool));

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

unsigned long Generator::getTotalN() const {
    return totalN;
}

void Generator::append(std::string &comb) {
    std::string buf = appendT;
    comb = buf.append(comb);
}

void Generator::prepend(std::string &comb) {
    comb.append(prependT);
}

