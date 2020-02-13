#include <iostream>
#include "Generator.h"

void clear(){
#if defined _WIN32
    system("cls");
#elif defined (__LINUX__) || defined(__gnu_linux__) || defined(__linux__)
    system("clear");
#elif defined (__APPLE__)
    system("clear");
#endif
}

void help_menu() {
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
    -lmin <value>           Minimum length of the combination
    -lmax <value
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
    -prepend <value>        Prepend <string>
    -append <value>        Append <string>
    -threads <value>        Manual thread override: Calculate combinations with the amount of threads specified by
                            the user. (Not recommended by default!)
    )";
    std::cout << help << std::endl;
}

void logo(){
    std::string logo =
            R"(
          $$\                             $$\
          $$ |                            $$ |
 $$$$$$$\ $$$$$$$\   $$$$$$\  $$\   $$\ $$$$$$\    $$$$$$\
$$  _____|$$  __$$\ $$  __$$\ $$ |  $$ |\_$$  _|  $$  __$$\
$$ /      $$ |  $$ |$$ |  \__|$$ |  $$ |  $$ |    $$$$$$$$ |
$$ |      $$ |  $$ |$$ |      $$ |  $$ |  $$ |$$\ $$   ____|
\$$$$$$$\ $$$$$$$  |$$ |      \$$$$$$  |  \$$$$  |\$$$$$$$\
 \_______|\_______/ \__|       \______/    \____/  \_______|
)";
    std::cout  << "\n\n\n" << logo << "\n\n\n";
}

bool disclaimer(){
    std::string disclaimer =
            R"(
Any actions and or activities related to the material contained within this program is solely your responsibility.
The misuse of the information in this website can result in criminal charges brought against the persons in question.
The author will not be held responsible in the event any criminal charges be brought against any individuals misusing cbrute to break the law.

**Your usage of cbrute constitutes your agreement to the following terms.**

1. All the information provided on this site are for **educational purposes only**. cbrute's developer is no not responsible for any misuse of the information.
2. cbrute is a program related to **Computer Security** and not a tool that promotes hacking / cracking.
3. I reserve the right to modify the disclaimer at any time without notice.

)";
    std::cout << "\n\n\n" << disclaimer << "I have read and agree to the terms and conditions: [y/n]: ";
    char input;
    std::cin >> input;
    return (tolower(input) == 'y');
}

int main(int argc, char *argv[]) {

    logo();

    if (argc < 2) {
        std::cerr << "Not enough arguments. See -h for help." << std::endl;
        return 1;
    }

    // Legal disclaimer
    if(!disclaimer()){
        return 1;
    }
    clear();

    for (int i = 1; i < argc; ++i) {
        if (std::string(argv[i]) == "-h") {
          help_menu();
            return 0;
        }
    }

    auto generator = Generator(argc, argv);
    try {
      // Abort option
      if (generator.is_file_mode()) {
        if (!generator.confirm_memory()) {
          throw std::invalid_argument("AbortedByUser");
        }
      }

      generator.start();

    } catch (const std::exception &exc) {
        std::cerr << "Looks like something went wrong. See -h for help."
                  << std::endl;
        std::cerr << exc.what() << std::endl;
        return 1;
    }

    return 0;
}
