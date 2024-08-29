#include <algorithm>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include "html2md.h"
#include "md4c-html.h"
#include "table.h"

using std::cerr;
using std::cout;
using std::ifstream;
using std::string;
using std::stringstream;
using std::vector;
using std::chrono::duration;
using std::chrono::high_resolution_clock;
using std::chrono::milliseconds;
namespace fs = std::filesystem;

namespace markdown {
void captureHtmlFragment(const MD_CHAR *data, const MD_SIZE data_size,
                         void *userData) {
  auto *str = static_cast<stringstream *>(userData);

  str->write(data, data_size);
}

string toHTML(const string &md) {
  stringstream html;

  static MD_TOC_OPTIONS options;

  md_html(md.c_str(), md.size(), &captureHtmlFragment, &html, MD_DIALECT_GITHUB,
          MD_HTML_FLAG_SKIP_UTF8_BOM, &options);

  return html.str();
};

string fromHTML(string &html) {
  static html2md::Options options;
  options.splitLines = false;

  html2md::Converter c(html, &options);
  return c.convert();
}
} // namespace markdown

namespace file {
string readAll(const string &name) {
  ifstream in(name);
  stringstream buffer;
  buffer << in.rdbuf();
  return buffer.str();
};
} // namespace file

// Log the error
void log(const string &file, const string &origMd, const string &generatedMd) {
  cerr << "Task " << fs::path(file).filename() << " failed:\nOriginal Md:\n"
       << origMd << "\nGenerated Markdown:\n"
       << generatedMd << '\n';
}

// Print "Running " + filename
void running(const string &file) {
  cout << "Running test " << fs::path(file).filename() << "...\t";
}

// Print "Passed!" in green
void passed() { cout << "\x1B[32mPassed!\033[0m\n"; }

// Print "Failed!" in red
void error() { cout << "\x1B[31mFailed!\033[0m\n"; }

void runTest(const string &file, short *errorCount) {
  // Read the markdown file
  const string md = file::readAll(file);

  running(file);

  // Convert the Md to HTML
  string html = markdown::toHTML(md);

  // Generate Md from the HTML
  string convertedMd = markdown::fromHTML(html);

  // Convert it back to HTML
  string testHTML = markdown::toHTML(convertedMd);

  // Compare original and result HTML
  if (html == testHTML)
    passed();
  else {
    error();
    log(file, md, convertedMd);
    ++*errorCount;
  }
}

void testOption(const char *name) {
  cout << "Test option \"" << name << "\"...\t";
}

bool testUnorderedList() {
  testOption("unorderedList");

  string html = "<ul><li>List</li></ul>";

  html2md::Options o;
  o.unorderedList = '*';

  html2md::Converter c(html, &o);

  auto md = c.convert();

  return md.find("* List\n") != string::npos;
}

bool testOrderedList() {
  testOption("orderedList");

  string html = "<ol><li>List</li></ol>";

  html2md::Options o;
  o.orderedList = ')';

  html2md::Converter c(html, &o);

  auto md = c.convert();

  return md.find("1) List\n") != string::npos;
}

bool testDisableTitle() {
  testOption("includeTitle");

  string html = "<title>HTML title</title>";

  html2md::Options o;
  o.includeTitle = false;

  html2md::Converter c(html, &o);

  auto md = c.convert();

  return md.empty() &&
         html2md::Convert(html).find("HTML title") != string::npos;
}

bool testFormatTable() {
  testOption("formatTable");

  constexpr const char *inputTable = "| 1 | 2 | 3 |\n"
                                     "| :-- | :-: | --: |\n"
                                     "| Hello | World | ! |\n"
                                     "| foo | bar | buzz |\n";

  constexpr const char *expectedOutput = "| 1     | 2     | 3    |\n"
                                         "|:------|:-----:|-----:|\n"
                                         "| Hello | World | !    |\n"
                                         "| foo   | bar   | buzz |\n";

  string formattedTable = formatMarkdownTable(inputTable);

  return formattedTable == expectedOutput;
}

int main(int argc, const char **argv) {
  // List to store all markdown files in this dir
  vector<string> files;

  static vector<string> markdownExtensions = {".md", ".markdown", ".mkd"};

  // Find the files
  for (const auto &p : fs::recursive_directory_iterator(DIR)) {
    if (std::find(markdownExtensions.begin(), markdownExtensions.end(),
                  p.path().extension()) != markdownExtensions.end() &&
        p.path().parent_path() == DIR)
      files.emplace_back(p.path().string());
  }

  // Test files passed as argument
  for (int i = 1; i < argc; i++) {
    // Check if the argument is a valid file path and ends with ".md"
    string file = argv[i];
    if (fs::is_regular_file(file) && file.find(".md") == file.size() - 3) {
      files.emplace_back(file);
    }
  }

  // Sort file names
  sort(files.begin(), files.end());

  // File name
  const char *errorFileName = DIR "/error.log";

  // Redirect errors to error.log
  FILE *errorFile = freopen(errorFileName, "w", stderr);
  if (!errorFile)
    cerr << "Failed to open " << errorFileName
         << " for whatever reason!\n"
            "Errors will be printed to the terminal instead of written to the "
            "mentioned file above.";

  // For measuring time.
  auto t1 = high_resolution_clock::now();

  // Count the errors
  short errorCount = 0;

  // Run the tests
  for (auto &file : files)
    runTest(file, &errorCount);

  // Test the options
  auto tests = {&testDisableTitle, &testUnorderedList, &testOrderedList,
                &testFormatTable};

  for (const auto &test : tests)
    if (!test()) {
      ++errorCount;
      error();
    } else
      passed();

  auto t2 = high_resolution_clock::now();

  /* Getting number of milliseconds as a double. */
  duration<double, std::milli> ms_double = t2 - t1;

  cout << files.size() + tests.size() << " tests executed in "
       << ms_double.count() << "ms. " << errorCount << " failed.\n";

  return 0;
}
