//
// Created by Julian Schnabel on 04.12.19.
//

#include "Generator.h"

unsigned long long factorial(size_t n) {
    return (n == 1L || n == 0L) ? 1L : factorial(n - 1L) * n;
}

Generator::~Generator() {
    fileStream.close();
}

Generator::Generator(int argc, char **args) {
    if (parseArguments(argc, args)) {
        if (checkArguments()) {
            initTasks();
            totalN = (perm) ? factorial(elementCount)
                            : static_cast<unsigned long>(std::pow(elementCount,
                                                                  length));
        }
    }
}

void Generator::getCombinations(unsigned long start, unsigned long end) {
    // Fill with dummy elements
    std::string comb(length, ' ');
    std::string temp(length, ' ');

    for (auto i = start; i < end; ++i) {
        auto n = i;
        for (size_t j = 0; j < length; ++j) {
            comb[comb.size() - j - 1] = characterSet[n % elementCount];
            n /= elementCount;
        }
        temp = comb;
        for (auto f : tasks) (this->*f)(temp);
    }
}

void Generator::getPermutations() {
    std::string temp(length, ' ');
    std::string perm(std::begin(characterSet), std::end(characterSet));
    std::sort(std::begin(perm), std::end(perm));
    do {
        temp = perm;
        for (auto f : tasks) (this->*f)(temp);
    } while (std::next_permutation(std::begin(perm), std::end(perm)));
}

bool Generator::initTasks() {
    if (!appendT.empty()) tasks.push_back(&Generator::append);
    if (!prependT.empty()) tasks.push_back(&Generator::prepend);
    return true;
}

bool Generator::initFileStream() {
    if (!path.empty()) {
        fileStream.open(path);
        tasks.push_back(&Generator::addToQueue);
    }
    return fileStream.good();
}

void Generator::append(std::string &str) {
    str.append(appendT);
}

void Generator::prepend(std::string &str) {
    str.insert(0, prependT);
}

void Generator::console() {
    unsigned long todo = totalN;
    while (todo > 0) {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        std::queue<std::string> temp;
        { // Lock only taken for this section
            std::lock_guard<std::mutex> lock(m);
            std::swap(temp, printQueue);
        }
        todo -= temp.size();
        while (!temp.empty()) {
            std::cout << temp.front() << '\n';
            temp.pop();
        }
    }
}

void Generator::print() {
    unsigned long todo = totalN;
    while (todo > 0) {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        std::queue<std::string> temp;
        { // Lock only taken for this section
            std::lock_guard<std::mutex> lock(m);
            std::swap(temp, printQueue);
        }
        todo -= temp.size();
        while (!temp.empty()) {
            fileStream << temp.front() << '\n';
            temp.pop();
        }
    }
}

void Generator::start() {
    if (perm) {
        getPermutations();
    } else {
        std::thread first(&Generator::getCombinations, this, 0, totalN);
        first.join();
        if (fileMode) {
            std::thread second(&Generator::print, this);
            second.join();
        }
        if (log) {
            std::thread third(&Generator::console, this);
            third.join();
        }
    }
}

void Generator::memoryApproximation() {
    if (perm) {
        approxMemory = sizeof(char) *
                       (elementCount + appendT.length() + prependT.length() +
                        1) * totalN;
    } else {
        approxMemory = sizeof(char) *
                       (length + appendT.length() + prependT.length() + 1) *
                       totalN;
    }

}

bool Generator::confirmMemory() {
    // Approximate memory
    memoryApproximation();
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
    std::cout
            << "CBRUTE will generate a "
            << approxMemory / UD.second
            << UD.first << " text file" << "\n"
            << "Are you sure you want to continue? [y/n] : ";

    char input;
    std::cin >> input;
    if (tolower(input) == 'y') {
        return initFileStream();
    }
    return false;
}

bool Generator::parseArguments(int argc, char **args) {
    std::map<std::string, std::string> argumentMap;
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

    argumentMap.insert(length);
    argumentMap.insert(characters);
    argumentMap.insert(filePath);
    argumentMap.insert(perm);
    argumentMap.insert(log);
    argumentMap.insert(alphabetic);
    argumentMap.insert(digit);
    argumentMap.insert(special);
    argumentMap.insert(ascii);
    argumentMap.insert(exclude);
    argumentMap.insert(prepend);
    argumentMap.insert(append);

    // Iterate through arg string
    for (int i = 1; i < argc - 1; ++i) {
        if (argumentMap.find(args[i]) != std::end(argumentMap)) {
            argumentMap.find(args[i])->second = args[i + 1];
        }
    }
    // Check last argument
    if (argumentMap.find(args[argc - 1]) != std::end(argumentMap))
        argumentMap.find(args[argc - 1])->second = "1";

    try {
        this->perm = !(argumentMap.at(perm.first).empty());
        if (!this->perm) {
            this->length = std::stoul(argumentMap.at(length.first));
        }
        this->path = argumentMap.at(filePath.first);
        if (!path.empty()) {
            fileMode = true;
            if (path.find(defaultFileExtension) == std::string::npos) {
                // Add missing file extension
                path.append(defaultFileExtension);
            }
        }
        this->log = !(argumentMap.at(log.first).empty());
    } catch (std::invalid_argument &e) {
        return false;
    }

    // String to appendT to
    if (!(argumentMap.at(append.first).empty()))
        std::copy(std::begin(argumentMap.at(append.first)),
                  std::end(argumentMap.at(append.first)),
                  std::back_inserter(this->appendT));

    // String to prepend before
    if (!(argumentMap.at(prepend.first).empty()))
        std::copy(std::begin(argumentMap.at((prepend.first))),
                  std::end(argumentMap.at((prepend.first))),
                  std::back_inserter(this->prependT));

    // Check if full ascii set was activated
    if (!(argumentMap.at(ascii.first).empty())) {
        std::copy(std::begin(this->ascii), std::end(this->ascii),
                  std::back_inserter(this->characterSet));
    } else {
        // Initialize alphabetic character set
        if (argumentMap.at(alphabetic.first) == "u") {
            std::copy(std::begin(this->alphaU), std::end(this->alphaU),
                      std::back_inserter(this->characterSet));
        } else if (argumentMap.at(alphabetic.first) == "l") {
            std::copy(std::begin(this->alphaL), std::end(this->alphaL),
                      std::back_inserter(this->characterSet));
        } else if (!(argumentMap.at(alphabetic.first).empty())) {
            std::copy(std::begin(this->alphaU), std::end(this->alphaU),
                      std::back_inserter(this->characterSet));
            std::copy(std::begin(this->alphaL), std::end(this->alphaL),
                      std::back_inserter(this->characterSet));
        }

        // Initialize numeric character set
        if (!(argumentMap.at(digit.first).empty()))
            std::copy(std::begin(this->digit), std::end(this->digit),
                      std::back_inserter(this->characterSet));

        // Initialize special character set
        if (!(argumentMap.at(special.first).empty()))
            std::copy(std::begin(this->special), std::end(this->special),
                      std::back_inserter(this->characterSet));

        // Initialize character set
        std::string temp = argumentMap.at(characters.first);
        std::copy(std::begin(temp), std::end(temp),
                  std::back_inserter(this->characterSet));

        std::vector<char> tempX(std::begin(argumentMap.at(exclude.first)),
                                std::end(argumentMap.at(exclude.first)));
        std::sort(std::begin(tempX), std::end(tempX));
        std::sort(std::begin(this->characterSet), std::end(this->characterSet));
        if (!(argumentMap.at(exclude.first).empty())) {
            this->characterSet.erase(
                    std::remove_if(std::begin(this->characterSet),
                                   std::end(this->characterSet),
                                   [&tempX](char c) {
                                       return std::binary_search(
                                               std::begin(tempX),
                                               std::end(tempX), c);
                                   }), std::end(this->characterSet));

        }
    }

    // Remove duplicates
    this->characterSet.erase(
            std::unique(std::begin(this->characterSet),
                        std::end(this->characterSet)),
            std::end(this->characterSet));
    this->elementCount = characterSet.size();
    return true;
}

bool Generator::checkArguments() {
    if (perm && elementCount > 1) return true;
    return length > 1 && elementCount > 1;
}

unsigned long Generator::getTotalN() const {
    return totalN;
}

void Generator::addToQueue(std::string &str) {
    {
        std::lock_guard<std::mutex> lock(m);
        printQueue.push(str);
    }
}

bool Generator::isFileMode() const {
    return fileMode;
}
