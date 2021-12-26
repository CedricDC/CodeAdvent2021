#include <algorithm>
#include <chrono>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

//! Note: changed ordering here to simplify printing of characters
// Dots here are (y, x)

namespace {

enum class Fold { X, Y };

using FoldInstruction = std::pair<Fold, unsigned int>;
using Dot = std::pair<unsigned int, unsigned int>;

struct Manual {
  std::vector<FoldInstruction> read(std::ifstream& ifile);

  void applyInstructions(const std::vector<FoldInstruction>& instructions);

  void printImage(unsigned int num_rows, unsigned int num_cols) const;

 private:
  std::vector<Dot> dots_;
};

}  // namespace

int main(int argc, char** argv) {
  if (argc < 2) {
    std::cout << "Required input arguments: <filename>" << std::endl;
    return 1;
  }

  std::string filename = argv[1];
  std::ifstream ifile(filename);

  if (!ifile.good()) {
    std::cout << "Could not open " << filename << std::endl;
    return 1;
  }

  auto t_start = std::chrono::steady_clock::now();

  Manual manual;
  auto instructions = manual.read(ifile);
  manual.applyInstructions(instructions);

  // print to read image
  // 1. get bounds
  unsigned int num_rows =
      std::find_if(instructions.crbegin(), instructions.crend(),
                   [](const FoldInstruction& i) { return (i.first == Fold::Y); })
          ->second;
  unsigned int num_cols =
      std::find_if(instructions.crbegin(), instructions.crend(),
                   [](const FoldInstruction& i) { return (i.first == Fold::X); })
          ->second;
  manual.printImage(num_rows, num_cols);

  auto t_end = std::chrono::steady_clock::now();

  std::cout << "Execution took "
            << std::chrono::duration_cast<std::chrono::microseconds>(t_end - t_start).count()
            << " us" << std::endl;

  return 0;
}

namespace {

std::vector<FoldInstruction> Manual::read(std::ifstream& ifile) {
  std::string line;

  // read dots
  while (std::getline(ifile, line) && !line.empty()) {
    std::istringstream iss(line);
    std::pair<unsigned int, unsigned int> dot;
    iss >> dot.second;
    iss.get();  // remove comma
    iss >> dot.first;
    dots_.push_back(dot);
  }

  // read instructions
  std::vector<FoldInstruction> instructions;
  while (std::getline(ifile, line)) {
    // format is "fold along <dir>=<value>"
    Fold direction = (line[11] == 'x') ? Fold::X : Fold::Y;
    instructions.emplace_back(direction, std::stoi(line.substr(13)));
  }

  return instructions;
}

void Manual::applyInstructions(const std::vector<FoldInstruction>& instructions) {
  for (const auto& instruction : instructions) {
    unsigned int fold_line = instruction.second;

    // fold is new_idx = 2 * fold_idx - old_idx
    unsigned int fold_idx = 2 * fold_line;

    if (instruction.first == Fold::X) {
      for ([[maybe_unused]] auto& [y, x] : dots_) {
        if (x > fold_line) x = (fold_idx - x);
      }
    } else {  // Fold::Y
      for ([[maybe_unused]] auto& [y, x] : dots_) {
        if (y > fold_line) y = (fold_idx - y);
      }
    }
  }

  // now, sort, and keep unique ones
  std::sort(dots_.begin(), dots_.end());
  auto new_end = std::unique(dots_.begin(), dots_.end());
  dots_.erase(new_end, dots_.end());
}

void Manual::printImage(unsigned int num_rows, unsigned int num_cols) const {
  // assuming sorted dots (along y, i.e. along col)
  std::string row(num_cols, ' ');

  unsigned int row_idx = 0;
  for (const auto& dot : dots_) {
    if (dot.first != row_idx) {
      std::cout << row << std::endl;
      std::fill(row.begin(), row.end(), ' ');
      while (++row_idx < dot.first) {
        std::cout << row << std::endl;
      }
    }

    row[dot.second] = '#';
  }

  std::cout << row << std::endl;
  // Note : ignoring empty lines that may still come afterwards
}

}  // namespace
