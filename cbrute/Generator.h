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

    bool is_file_mode() const;

    bool confirm_memory();

  private:
    // Auxiliary functions
    bool check_arguments();

    bool parse_arguments(int argc, char **args);

    bool init_file_stream();

    std::mutex m;

    // Modes
    bool file_mode = false;
    bool log = false;
    bool perm = false;

    // Constants
    const std::string alpha_lower = "abcdefghijklmnopqrstuvwxyz";
    const std::string alpha_upper = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    const std::string digit = "0123456789";
    const std::string special = "!\"$%&'()*+,-./:;<=>?@[\\]^_`{|}~";
    const std::string ascii = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUV"
                              "WXYZ0123456789!\"$%&'()*+,-./:;<=>?@[\\]^_`{|}~";
    static constexpr char DEFAULT_FILE_EXTENSION[] = ".txt";

    // Strings to append/prepend to
    char *append_str = nullptr;
    char *prepend_str = nullptr;

    // Output functions
    void console();

    std::queue<std::string> print_queue;

    // Number of possible permutations/combinations
    unsigned long total_N = 0;
    size_t length_min = 0;
    size_t length_max = 0;
    size_t element_count = 0;
    // Amount of memory required in bytes
    long double approx_memory = 0;

    void approximate_memory();
    std::pair<std::string, unsigned long> get_unit(unsigned long bytes);
    void calculate_total_N();

    std::vector<char> character_set;
    std::ofstream fileStream;
    FILE *output_file;
    std::string path;

    // Generator functions
    void get_combinations_ST(unsigned long start, unsigned long end,
                             unsigned long length);

    void get_permutations();

    // Task vector
    std::vector<void (Generator::*)(char *comb)> tasks;

    void init_tasks();

    void append(char *str);

    void prepend(char *str);

    bool init_threads();

    size_t get_N(size_t temp);

    // Multithreading support

    unsigned long thread_count;
    std::vector<
        std::vector<std::tuple<unsigned long, unsigned long, unsigned long>>>
        thread_tasks;
    void get_combinations_MT(
        std::vector<std::tuple<unsigned long, unsigned long, unsigned long>>
            tasks_MT,
        size_t index);

    void add_to_temp(const char *temp, size_t index);
    std::vector<std::queue<const char *>> temp_comb;
    void print_MT();
    bool running = true;
    unsigned long todo;
    void print_ST(char *str);
    bool manual_thread_override = false;
    unsigned long manual_thread_count = 0;
    void init_thread_tasks(unsigned long n);

    // Benchmark variables
    std::__1::chrono::steady_clock::time_point begin;
    std::__1::chrono::steady_clock::time_point end;
    std::__1::chrono::milliseconds duration;
    void print_report(std::ostream &os);
};


#endif //CBRUTE_GENERATOR_H
