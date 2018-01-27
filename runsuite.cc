#include <iostream>
#include <fstream>
#include <vector>
#include <unordered_set>
#include <cstdio>
using std::vector;
using std::string;
using std::ifstream;
using std::unordered_set;

int system_wrapper(std::string arg) {  
    int result = system(arg.c_str());
    return WEXITSTATUS(result);
}
bool is_empty(std::ifstream& pFile) {
    return pFile.peek() == std::ifstream::traits_type::eof();
}

class TestingFailure {
    std::string message;
  public:
    TestingFailure(std::string message): message(message) {}

    std::string getMessage() const { return message; }
};

vector<string> &getTestStems(vector<string> &stems) {
    char temp_path_c[128];
    FILE *f = popen("mktemp", "r");
    fscanf(f, "%s", temp_path_c);
    std::string temp_path = temp_path_c;

    if (system_wrapper("ls tests/*.in > " + temp_path + " 2> /dev/null") != 0) {
        if (system_wrapper("ls *.in > " + temp_path + " 2> /dev/null") != 0) {
            throw TestingFailure("ERROR: No tests found in 'tests' dir "
                                 "and no test files in curr dir");
        }
    }

    ifstream stemsFile (temp_path);
    if (stemsFile.is_open()) {
        string stem;
        while (stemsFile >> stem) {
            stems.push_back(stem.substr(0, stem.length() - 3));
        }
        stemsFile.close();
    } else {
        throw TestingFailure("ERROR: Cannot open temporary stem file");
    }
    system_wrapper("rm " + temp_path);
    return stems;
}

void printFile(const string fileName) {
    ifstream f (fileName);
    if (!f.is_open()) throw TestingFailure("ERROR: Could not open " + fileName);
    else if (!is_empty(f)) std::cout << "+=========+" + fileName + ": " << std::endl
                                     << f.rdbuf() << std::endl;
}

void runTest(const string &program, const string &stem, unordered_set<string> &options) {
    if (system_wrapper(program + " < " + stem + ".in > " + stem + ".act 2> " + stem + ".err") != 0) {
        if (stem.substr(stem.length() - 4) != ".bad")
            std::cout << stem + " failed" << std::endl;
    } else if (system_wrapper("[ -f " + stem + ".out ]") == 0 &&
               system_wrapper("diff " + stem + ".act " + stem + ".out > " + stem + ".diff")) {
        std::cout << stem + " failed" << std::endl;
    }

    if (options.count("show") || options.count("s")) {
        options.insert("o");
        options.insert("e");
    }
    if (options.count("output") || options.count("o")) {
        printFile(stem + ".act");
    }
    if (options.count("errors") || options.count("e")) {
        printFile(stem + ".err");
    }
    if (options.count("diff") || options.count("d")) {
        if (system_wrapper("[ -f " + stem + ".diff ]") == 0)
            printFile(stem + ".diff");
    }
    if (!options.count("keep") && !options.count("k")) {
        system_wrapper("rm -f " + stem + ".act " + stem + ".err " + stem + ".diff");
    }
}

int main(int argc, char **argv) {

    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " [program]" << std::endl;
        return 1;
    }
    unordered_set<string> options;
    vector<string> args;
    for (int i = 1; i < argc; ++i) {
        if (argv[i][0] == '-') {
            if (argv[i][1] == '-') options.insert(argv[i] + 1);
            else for (char *p = argv[i] + 1; *p != '\0'; ++p) options.insert(string(p, 1));
        } else args.push_back(argv[i]);
    }
    // for (auto &arg : args) std::cout << arg << std::endl;
    // for (auto &opt : options) std::cout << opt << std::endl;

    vector<string> stems;
    try {
        getTestStems(stems);
        // for (auto &stem : stems) std::cout << stem << std::endl;
    } catch (TestingFailure &f) {
        std::cerr << f.getMessage() << std::endl;
        return 1;
    }

    try {
        for (auto &stem : stems) runTest(args.front(), stem, options);
    } catch (TestingFailure &f) {
        std::cerr << f.getMessage() << std::endl;
        return 1;
    }

    return 0;
}