#include <cstring>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include "html2md.h"

using std::cerr;
using std::cin;
using std::cout;
using std::endl;
using std::fstream;
using std::ifstream;
using std::ios;
using std::string;
using std::stringstream;

namespace FileUtils {
bool exists(const std::string &name) {
  ifstream f(name.c_str());
  return f.good();
}

string readAll(const string &file) {
  ifstream in(file);
  stringstream buffer;
  buffer << in.rdbuf();

  if (in.bad()) {
    throw std::runtime_error("Error reading file: " + file);
  }

  return buffer.str();
}

void writeFile(const string &file, const string &content) {
  fstream out(file, ios::out);
  if (!out.is_open()) {
    throw std::runtime_error("Error writing file: " + file);
  }

  out << content;
  out.close();

  if (out.bad()) {
    throw std::runtime_error("Error writing file: " + file);
  }
}
} // namespace FileUtils

constexpr const char *const DESCRIPTION =
    " [Options] files...\n\n"
    "Simple and fast HTML to Markdown converter with table support.\n\n"
    "Options:\n"
    "  -h, --help\tDisplays this help information.\n"
    "  -v, --version\tDisplay version information and exit.\n"
    "  -o, --output\tSets the output file.\n"
    "  -i, --input\tSets the input text.\n"
    "  -p, --print\tPrint the generated Markdown.\n"
    "  -r, --replace\tOverwrite the output file (if it already exists) without "
    "asking.\n";

struct Options {
  bool print = false;
  bool replace = false;
  string inputFile;
  string outputFile;
  string inputText;
};

void printHelp(const string &programName) {
  cout << programName << DESCRIPTION;
}

void printVersion() { cout << "Version " << VERSION << endl; }

bool confirmOverride(const string &fileName) {
  while (true) {
    cout << fileName << " already exists, override? [y/n] ";
    string override;
    getline(cin, override);

    if (override.empty()) {
      continue;
    }

    if (override == "y" || override == "Y") {
      return true;
    } else if (override == "n" || override == "N") {
      return false;
    } else {
      cout << "Invalid input" << endl;
    }
  }
}

Options parseCommandLine(int argc, char **argv) {
  Options options;

  if (argc == 1) {
    printHelp(argv[0]);
    exit(EXIT_SUCCESS);
  }

  for (int i = 1; i < argc; i++) {
    string arg = argv[i];

    if (arg == "-h" || arg == "--help") {
      printHelp(argv[0]);
      exit(EXIT_SUCCESS);
    } else if (arg == "-v" || arg == "--version") {
      printVersion();
      exit(EXIT_SUCCESS);
    } else if (arg == "-p" || arg == "--print") {
      options.print = true;
    } else if (arg == "-r" || arg == "--replace") {
      options.replace = true;
    } else if (arg == "-o" || arg == "--output") {
      if (i + 1 < argc) {
        options.outputFile = argv[i + 1];
        i++;
      } else {
        cerr << "The" << arg << "option requires a file name!\n" << endl;
        exit(EXIT_FAILURE);
      }
    } else if (arg == "-i" || arg == "--input") {
      if (i + 1 < argc) {
        options.inputText = argv[i + 1];
        i++;
      } else {
        cerr << "The" << arg << "option requires HTML text!" << endl;
        exit(EXIT_FAILURE);
      }
    } else if (options.inputFile.empty()) {
      options.inputFile = arg;
    }
  }

  return options;
}

int main(int argc, char **argv) {
  Options options = parseCommandLine(argc, argv);

  string input;
  if (!options.inputText.empty()) {
    input = options.inputText;
  } else if (!options.inputFile.empty() &&
             FileUtils::exists(options.inputFile)) {
    input = FileUtils::readAll(options.inputFile);
  } else {
    cerr << "No valid input provided!" << endl;
    return EXIT_FAILURE;
  }

  html2md::Converter converter(input);
  string md = converter.convert();

  if (options.print) {
    cout << md << endl;
  }

  if (!options.outputFile.empty()) {
    if (FileUtils::exists(options.outputFile) && !options.replace) {
      if (confirmOverride(options.outputFile)) {
        FileUtils::writeFile(options.outputFile, md);
        cout << "Markdown written to " << options.outputFile << endl;
      } else {
        cout << "Markdown not written." << endl;
      }
    } else {
      FileUtils::writeFile(options.outputFile, md);
      cout << "Markdown written to " << options.outputFile << endl;
    }
  }

  return EXIT_SUCCESS;
}
