//
// Created by Julian Schnabel on 04.12.19.
//

#include "Generator.h"

unsigned long long factorial(unsigned long long n) {
  return (n == 1L || n == 0L) ? 1L : factorial(n - 1L) * n;
}

void swap(char **str1_ptr, char **str2_ptr) {
  char *temp = *str1_ptr;
  *str1_ptr = *str2_ptr;
  *str2_ptr = temp;
}
Generator::~Generator() {
  //    fileStream.close();
  fclose(output_file);
}

Generator::Generator(int argc, char **args) {
  if (parse_arguments(argc, args)) {
    if (check_arguments()) {
      init_tasks();
      calculate_total_N();
    }
  }
}

void Generator::init_thread_tasks(unsigned long n) {
  thread_tasks = std::vector<
      std::vector<std::tuple<unsigned long, unsigned long, unsigned long>>>(n);
}

void Generator::get_combinations_MT(
    std::vector<std::tuple<unsigned long, unsigned long, unsigned long>>
        tasks_MT,
    unsigned long index) {
  for (auto &item : tasks_MT) {
    auto start = std::get<0>(item);
    auto end = std::get<1>(item);
    auto length = std::get<2>(item);

    char comb[length];

    for (auto i = start; i < end; ++i) {
      auto n = i;
      for (unsigned long j = 0; j < length; ++j) {
        comb[length - j - 1] = character_set[n % element_count];
        n /= element_count;
      }
      add_to_temp(comb, index);
    }
  }
}
void Generator::get_combinations_ST(unsigned long start, unsigned long end,
                                    unsigned long length) {
  // Fil with dummy elements

  char *comb = (char *)malloc(length * sizeof(char));
  char *temp = (char *)malloc(length * sizeof(char));
  for (auto i = start; i < end; ++i) {
    auto n = i;
    for (unsigned long j = 0; j < length; ++j) {
      comb[length - j - 1] = character_set[n % element_count];
      n /= element_count;
    }
    swap(&temp, &comb);
    temp = comb;
    for (auto f : tasks)
      (this->*f)(temp);
  }
}

void Generator::get_permutations() {
  std::string temp(length_min, ' ');
  std::string perm(std::begin(character_set), std::end(character_set));
  std::sort(std::begin(perm), std::end(perm));
  do {
    temp = perm;
    // for (auto f : tasks) (this->*f)(temp);
  } while (std::next_permutation(std::begin(perm), std::end(perm)));
}

void Generator::init_tasks() {

  if (append_str != NULL) {
    tasks.push_back(&Generator::append);
  }
  if (prepend_str != NULL) {
    tasks.push_back(&Generator::prepend);
  }
}

bool Generator::init_file_stream() {
  if (!path.empty()) {
    //        fileStream.open(path);
    output_file = fopen(path.c_str(), "w");
  }
  return fileStream.good();
}

void Generator::append(char *str) {
  str = (char *)realloc(str, sizeof(str) + sizeof(append_str));
  strcat(str, append_str);
}

void Generator::prepend(char *str) {
  str = (char *)realloc(str, sizeof(str) + sizeof(prepend_str));
  unsigned long len = strlen(prepend_str);
  memmove(str + len, str, strlen(str) + 1);
  memcpy(str, prepend_str, len);
}

void Generator::console() {
  // FIXME: Console output currently not working due to print-queue issues
  // Number of combinations to print
  unsigned long todo = total_N;
  while (todo > 0) {
    //  std::this_thread::sleep_for(std::chrono::milliseconds(50));
    std::queue<std::string> temp;
    { // Lock only taken for this section

      std::lock_guard<std::mutex> lock(m);
      std::swap(temp, print_queue);
    }
    todo -= temp.size();
    while (!temp.empty()) {
      std::cout << temp.front() << '\n';
      temp.pop();
    }
  }
}

void Generator::start() {
  // Start time
  begin = std::chrono::high_resolution_clock::now();
  todo = total_N;
  if (perm) {
    get_permutations();
  } else {
    init_threads();
    std::vector<std::thread> threads;
    //        FIXME only for debugging
    //        if (fileMode) {
    //        }
    std::thread second(&Generator::print_MT, this);
    // Running
    for (unsigned long i = 0; i < thread_count; ++i) {
      std::thread t(&Generator::get_combinations_MT, this, thread_tasks[i], i);
      threads.push_back(std::move(t));
    }
    for (auto &elem : threads) {
      if (elem.joinable())
        elem.join();
    }
    // Running end
    running = false;
    if (second.joinable())
      second.join();
    if (log) {
      std::thread third(&Generator::console, this);
      third.join();
    }
  }
  end = std::chrono::high_resolution_clock::now();
  duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - begin);

  print_report(std::cout);
}

void Generator::approximate_memory() {
  if (perm) {
    approx_memory = sizeof(char) * (element_count) +
                    (sizeof(append_str) + sizeof(prepend_str)) * total_N;
  } else {
    for (unsigned long length_temp = length_max; length_temp <= length_max;
         length_temp++) {
      approx_memory += (sizeof(char) * (length_temp) + sizeof(append_str) +
                        sizeof(prepend_str)) *
                       total_N;
      // Calculate total count of combinations
    }
  }
}

bool Generator::confirm_memory() {
  // Approximate memory
  approximate_memory();
  auto UD = get_unit(approx_memory);
  std::cout << "CBRUTE will generate a " << approx_memory / UD.second
            << UD.first << " text file"
            << "\n"
            << "Are you sure you want to continue? [y/n] : ";

  char input;
  std::cin >> input;
  if (tolower(input) == 'y') {
    return init_file_stream();
  }
  return false;
}

bool Generator::parse_arguments(int argc, char **args) {
  std::map<std::string, std::string> argument_map;
  auto lengthMin = std::make_pair("-lmin", "");
  auto lengthMax = std::make_pair("-lmax", "");
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
  auto threads = std::make_pair("-threads", "");

  argument_map.insert(lengthMin);
  argument_map.insert(lengthMax);
  argument_map.insert(characters);
  argument_map.insert(filePath);
  argument_map.insert(perm);
  argument_map.insert(log);
  argument_map.insert(alphabetic);
  argument_map.insert(digit);
  argument_map.insert(special);
  argument_map.insert(ascii);
  argument_map.insert(exclude);
  argument_map.insert(prepend);
  argument_map.insert(append);
  argument_map.insert(threads);

  // Iterate through arg string
  for (int i = 1; i < argc - 1; ++i) {
    if (argument_map.find(args[i]) != std::end(argument_map)) {
      argument_map.find(args[i])->second = args[i + 1];
    }
  }
  // Check last argument
  if (argument_map.find(args[argc - 1]) != std::end(argument_map))
    argument_map.find(args[argc - 1])->second = "1";

  try {
    this->perm = !(argument_map.at(perm.first).empty());
    if (!this->perm) {
      this->length_min = std::stoul(argument_map.at(lengthMin.first));
      this->length_max = std::stoul(argument_map.at(lengthMax.first));
    }
    this->path = argument_map.at(filePath.first);
    if (!path.empty()) {
      file_mode = true;
      if (path.find(DEFAULT_FILE_EXTENSION) == std::string::npos) {
        // Add missing file extension
        path.append(DEFAULT_FILE_EXTENSION);
      }
    }
    this->log = !(argument_map.at(log.first).empty());
  } catch (std::invalid_argument &e) {
    return false;
  }

  // String to prepend before
  if (!(argument_map.at(prepend.first).empty())) {
    prepend_str = static_cast<char *>(
        malloc(argument_map.at(append.first).size() * sizeof(char)));
    strcpy(prepend_str, argument_map.at(prepend.first).c_str());
  }

  // String to append to
  if (!(argument_map.at(append.first).empty())) {
    append_str = static_cast<char *>(
        malloc(argument_map.at(append.first).size() * sizeof(char)));
    strcpy(append_str, argument_map.at(append.first).c_str());
  }

  // Manual thread count override
  if (!(argument_map.at(threads.first).empty())) {
    manual_thread_count =
        std::stoul(argument_map.at(threads.first), nullptr, 0);
    manual_thread_override = true;
  }

  // Check if full ascii set was activated
  if (!(argument_map.at(ascii.first).empty())) {
    std::copy(std::begin(this->ascii), std::end(this->ascii),
              std::back_inserter(this->character_set));
  } else {
    // Initialize alphabetic character set
    if (argument_map.at(alphabetic.first) == "u") {
      std::copy(std::begin(this->alpha_upper), std::end(this->alpha_upper),
                std::back_inserter(this->character_set));
    } else if (argument_map.at(alphabetic.first) == "l") {
      std::copy(std::begin(this->alpha_lower), std::end(this->alpha_lower),
                std::back_inserter(this->character_set));
    } else if (!(argument_map.at(alphabetic.first).empty())) {
      std::copy(std::begin(this->alpha_upper), std::end(this->alpha_upper),
                std::back_inserter(this->character_set));
      std::copy(std::begin(this->alpha_lower), std::end(this->alpha_lower),
                std::back_inserter(this->character_set));
    }

    // Initialize numeric character set
    if (!(argument_map.at(digit.first).empty()))
      std::copy(std::begin(this->digit), std::end(this->digit),
                std::back_inserter(this->character_set));

    // Initialize special character set
    if (!(argument_map.at(special.first).empty()))
      std::copy(std::begin(this->special), std::end(this->special),
                std::back_inserter(this->character_set));

    // Initialize character set
    std::string temp = argument_map.at(characters.first);
    std::copy(std::begin(temp), std::end(temp),
              std::back_inserter(this->character_set));

    std::vector<char> tempX(std::begin(argument_map.at(exclude.first)),
                            std::end(argument_map.at(exclude.first)));
    std::sort(std::begin(tempX), std::end(tempX));
    std::sort(std::begin(this->character_set), std::end(this->character_set));
    if (!(argument_map.at(exclude.first).empty())) {
      this->character_set.erase(std::remove_if(std::begin(this->character_set),
                                               std::end(this->character_set),
                                               [&tempX](char c) {
                                                 return std::binary_search(
                                                     std::begin(tempX),
                                                     std::end(tempX), c);
                                               }),
                                std::end(this->character_set));
    }
  }

  // Remove duplicates
  this->character_set.erase(std::unique(std::begin(this->character_set),
                                        std::end(this->character_set)),
                            std::end(this->character_set));
  this->element_count = character_set.size();
  return true;
}

bool Generator::check_arguments() {
  if (perm && element_count > 1)
    return true;
  return length_min > 1 && element_count > 1;
}

bool Generator::is_file_mode() const { return file_mode; }

void Generator::calculate_total_N() {
  for (unsigned long length_temp = length_min; length_temp <= length_max;
       length_temp++) {
    // Calculate total count of combinations
    total_N +=
        static_cast<unsigned long>(std::pow(character_set.size(), length_temp));
  }
}

unsigned long Generator::get_N(unsigned long temp) {
  return static_cast<unsigned long>(std::pow(character_set.size(), temp));
}

bool Generator::init_threads() {
  // v[] = tasklist for core
  // v[][] = task

  if (manual_thread_override) {
    this->thread_count = manual_thread_count;
    init_thread_tasks(thread_count);
  } else {
    this->thread_count = std::thread::hardware_concurrency() - 2;
    init_thread_tasks(thread_count);
  }
  auto combPerThread = total_N / thread_count;
  auto todo = total_N;
  auto lengthTemp = length_min;
  long long end = 0;
  for (int thread = 0; thread < thread_count; ++thread) {
    long long start = 0;
    unsigned long done = 0;
    while (todo > 0 && lengthTemp <= length_max && done < combPerThread) {
      start = end;
      end = start + combPerThread;
      if (end > get_N(lengthTemp)) {
        end = 0;
        end += get_N(lengthTemp);
      }
      auto e = std::make_tuple(start, end, lengthTemp);
      thread_tasks.at(thread).emplace_back(e);
      done += end - start;
      todo -= end - start;
      if (end == get_N(lengthTemp)) {
        lengthTemp++;
        start = 0;
        end = 0;
      }
    }
  }
  // Init different queues
  temp_comb = std::vector<std::queue<const char *>>(thread_count);
  return false;
}

void Generator::add_to_temp(const char *temp, unsigned long index) {
  char *t = static_cast<char *>(malloc(sizeof(temp)));
  strcpy(t, temp);
  temp_comb.at(index).push(t);
}

void Generator::print_MT() {
  while (todo > 0) {
    if (running) {
      std::this_thread::sleep_for(std::chrono::milliseconds(500));
      std::vector<std::queue<const char *>> temp(thread_count);
      { // Lock only taken for this section
        // Lock this area to avoid adding and printing to/from the queue
        // simultaneously
        std::lock_guard<std::mutex> lock(m);
        temp_comb.swap(temp);
      }
      for (int i = 0; i < thread_count; ++i) {
        while (!temp[i].empty()) {
          auto t = temp[i].front();
          // FIXME append / preprend support for const char pointers
          for (auto f : tasks)
            (this->*f)(const_cast<char *>(t));
          fprintf(output_file, "%s\n", temp[i].front());
          temp[i].pop();
          todo--;
        }
      }
    } else {
      for (int i = 0; i < thread_count; ++i) {
        while (!temp_comb[i].empty()) {
          fprintf(output_file, "%s\n", temp_comb[i].front());
          temp_comb[i].pop();
          todo--;
        }
      }
    }
  }
}

void Generator::print_report(std::ostream &os) {
  auto UD = get_unit(approx_memory);
  os << "\n---\n"
     << total_N << " "
     << "Combinations generated in " << duration.count() << " milliseconds."
     << std::endl
     << "Total disk usage: " << approx_memory / UD.second << " " << UD.first
     << std::endl
     << "Threads used: " << thread_count << std::endl
     << "Manual thread-count override: " << manual_thread_override << std::endl;
}

void Generator::print_ST(char *str) {
  fprintf(output_file, "%s\n", str);
  todo--;
}

std::pair<std::string, unsigned long> Generator::get_unit(unsigned long bytes) {
  std::string unit = "B";
  auto divisor = 1L;
  if (approx_memory >= 1000 && approx_memory < 1000000) {
    unit = "KB ";
    divisor = 1000;
  } else if (approx_memory >= 1000000 && approx_memory < 1000000000) {
    unit = "MB";
    divisor = 1000000;
  } else if (approx_memory >= 1000000000 && approx_memory < 1000000000000) {
    unit = "GB";
    divisor = 1000000000;
  } else if (approx_memory >= 1000000000000) {
    unit = "TB";
    divisor = 1000000000000;
  }
  return std::make_pair(unit, divisor);
}
