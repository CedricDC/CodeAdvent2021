#include <array>
#include <chrono>
#include <fstream>
#include <iostream>
#include <iterator>
#include <map>
#include <sstream>
#include <string>
#include <vector>

// Assumptions:
//
// 1. The same number is never drawn twice
// 2. The same number does not appear twice on a grid
// 3. Grids are 5x5
// 4. Possible bingo values are 0-99
// 5. Only one board will win at a time
// 6. Input format is valid
//
namespace {

std::vector<int> readBingoNumbers(std::ifstream& ifile);

constexpr int NUM_VALUES = 100;
constexpr int GRID_SIZE = 5;

class Grid {
 public:
  Grid(int id);

  // add value to fill grid
  void add(int value);

  // bingo number was drawn, check result
  bool draw(int row, int col, int value);

  // get sum of currently remaining values
  std::size_t getRemainingSum() const;

  // return grid id
  int getId() const;

 private:
  int id_;
  std::array<int, GRID_SIZE> row_entries_;  // number of entries in each row
  std::array<int, GRID_SIZE> col_entries_;  // number of entries in each column
  std::size_t total_sum_ = 0;
  bool has_won_ = false;
};

class BingoMap {
 public:
  struct GridLink {
    GridLink(int row, int col, int id) : row{row}, col{col}, grid_id{id} {}
    int row;
    int col;
    int grid_id;
  };

  // load one grid from file
  void loadGrid(std::ifstream& ifile);

  // draw next value : if a grid has bingo, return a pointer to it, else nullptr
  const Grid* draw(int value);

 private:
  std::array<std::vector<GridLink>, NUM_VALUES> map_;
  std::vector<Grid> grids_;
  std::size_t players_left_ = 0;
};

}  // namespace

int main(int argc, char** argv) {
  if (argc < 2) {
    std::cout << "Require filename as input argument" << std::endl;
    return 1;
  }

  std::string filename = argv[1];
  std::ifstream ifile(filename);

  if (!ifile.good()) {
    std::cout << "Could not open " << filename << std::endl;
    return 1;
  }

  // read first line -> sequence of drawn numbers
  std::vector<int> sequence = readBingoNumbers(ifile);

  // load grids
  BingoMap bingo_map;
  while (ifile.good()) {
    bingo_map.loadGrid(ifile);
  }

  // draw one value at a time
  auto t_start = std::chrono::steady_clock::now();
  for (const int& number : sequence) {
    std::cout << "Drawing " << number << std::endl;
    const Grid* losing_grid = bingo_map.draw(number);
    if (losing_grid != nullptr) {
      std::cout << "Grid " << losing_grid->getId() << " won last with a remainder of "
                << losing_grid->getRemainingSum() << std::endl;
      std::cout << "Total score: " << number * losing_grid->getRemainingSum() << std::endl;
      break;
    }
  }
  auto t_end = std::chrono::steady_clock::now();
  std::cout << "Total time: "
            << std::chrono::duration_cast<std::chrono::microseconds>(t_end - t_start).count()
            << std::endl;

  return 0;
}

namespace {

std::vector<int> readBingoNumbers(std::ifstream& ifile) {
  std::vector<int> values;
  std::string line;
  std::getline(ifile, line);
  std::istringstream iss(line);

  std::string value_str;
  while (std::getline(iss, value_str, ',')) {
    values.push_back(std::stoi(value_str));
  }

  return values;
}

Grid::Grid(int id) : id_{id} {
  row_entries_.fill(GRID_SIZE);
  col_entries_.fill(GRID_SIZE);
}

void Grid::add(int value) { total_sum_ += value; }

bool Grid::draw(int row, int col, int value) {
  if (has_won_) return false;

  total_sum_ -= value;

  if ((--row_entries_[row] == 0) || (--col_entries_[col] == 0)) {
    //    std::cout << "Grid " << id_ << " screams BINGO!" << std::endl;
    has_won_ = true;
    return true;
  }

  return false;
}

std::size_t Grid::getRemainingSum() const { return total_sum_; }

int Grid::getId() const { return id_; }

void BingoMap::loadGrid(std::ifstream& ifile) {
  int grid_id = grids_.size();
  std::cout << "Filling grid " << grid_id << std::endl;

  auto& grid = grids_.emplace_back(grid_id);

  int value;
  for (int col_idx = 0; col_idx < GRID_SIZE; ++col_idx) {
    for (int row_idx = 0; row_idx < GRID_SIZE; ++row_idx) {
      ifile >> value;
      grid.add(value);
      map_[value].emplace_back(row_idx, col_idx, grid_id);
    }
  }

  ++players_left_;
}

// draw next value
// If a grid has bingo, return a pointer to it, else nullptr
const Grid* BingoMap::draw(int value) {
  for (auto& link : map_[value]) {
    if (grids_[link.grid_id].draw(link.row, link.col, value)) {
      // if last player to win, return
      if (--players_left_ == 0) return &grids_[link.grid_id];
    }
  }
  return nullptr;
}

}  // namespace
