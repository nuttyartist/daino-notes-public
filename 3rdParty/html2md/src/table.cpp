// Copyright (c) Tim Gromeyer
// Licensed under the MIT License - https://opensource.org/licenses/MIT

#include "table.h"

#include <iomanip>
#include <iostream>
#include <sstream>
#include <vector>

using std::string;
using std::vector;

const size_t MIN_LINE_LENGTH = 3; // Minimum length of line

void removeLeadingTrailingSpaces(string &str) {
  size_t firstNonSpace = str.find_first_not_of(' ');
  if (firstNonSpace == string::npos) {
    str.clear(); // Entire string is spaces
    return;
  }

  size_t lastNonSpace = str.find_last_not_of(' ');
  str = str.substr(firstNonSpace, lastNonSpace - firstNonSpace + 1);
}

string enlargeTableHeaderLine(const string &str, size_t length) {
  if (str.empty() || length < MIN_LINE_LENGTH)
    return "";

  size_t first = str.find_first_of(':');
  size_t last = str.find_last_of(':');

  if (first == 0 && first == last)
    last = string::npos;

  string line = string(length, '-');

  if (first == 0)
    line[0] = ':';
  if (last == str.length() - 1)
    line[length - 1] = ':';

  return line;
}

string formatMarkdownTable(const string &inputTable) {
  std::istringstream iss(inputTable);
  string line;
  vector<vector<string>> tableData;

  // Parse the input table into a 2D vector
  while (std::getline(iss, line)) {
    std::istringstream lineStream(line);
    string cell;
    vector<string> rowData;

    while (std::getline(lineStream, cell, '|')) {
      if (!cell.empty()) {
        removeLeadingTrailingSpaces(cell); // Use the trim function
        rowData.push_back(cell);
      }
    }

    if (!rowData.empty()) {
      tableData.push_back(std::move(rowData)); // Move rowData to avoid copying
    }
  }

  // Determine maximum width of each column
  vector<size_t> columnWidths(tableData[0].size(), 0);
  for (const auto &row : tableData) {
    if (columnWidths.size() < row.size()) {
      columnWidths.resize(row.size(), 0);
    }

    for (size_t i = 0; i < row.size(); ++i) {
      columnWidths[i] = std::max(columnWidths[i], row[i].size());
    }
  }

  // Build the formatted table
  std::ostringstream formattedTable;
  for (size_t rowNumber = 0; rowNumber < tableData.size(); ++rowNumber) {
    const auto &row = tableData[rowNumber];

    formattedTable << "|";

    for (size_t i = 0; i < row.size(); ++i) {
      if (rowNumber == 1) {
        formattedTable << enlargeTableHeaderLine(row[i], columnWidths[i] + 2)
                       << "|";
        continue;
      }
      formattedTable << " " << std::setw(columnWidths[i]) << std::left << row[i]
                     << " |";
    }
    formattedTable << "\n";
  }

  return formattedTable.str();
}
