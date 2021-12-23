#include <chrono>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

namespace {

std::size_t numCols(std::ifstream& ifile) {
  std::string line;
  int pos_before = ifile.tellg();
  std::getline(ifile, line);
  ifile.seekg(pos_before);
  return line.size();
}

struct FloorTile {
  FloorTile(uint8_t h) : height(h) {}

  uint8_t height = 0;
  uint8_t higher_neighbours = 0;

  bool isLowPoint() const { return (higher_neighbours == 4); }
};

}  // namespace

// brute force without thinking, save full field in memory, process afterwards
// could be optimized by having two rotating rows
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

  std::vector<FloorTile> field;
  std::string line;
  while (std::getline(ifile, line)) {
    for (auto c : line) field.emplace_back(static_cast<uint8_t>(c - '0'));
  }

  // now search for low points
  std::size_t num_cols = line.size();
  std::size_t num_rows = field.size() / num_cols;
  std::size_t risk_level = 0;

  for (std::size_t row_idx = 0; row_idx < num_rows; ++row_idx) {
    for (std::size_t col_idx = 0; col_idx < num_cols; ++col_idx) {
      std::size_t full_idx = row_idx * num_cols + col_idx;

      if (col_idx > 0) {
        if (field[full_idx - 1].height < field[full_idx].height) {
          ++field[full_idx - 1].higher_neighbours;
        } else {
          ++field[full_idx].higher_neighbours;
        }
      }

      if (row_idx > 0) {
        if (field[full_idx - num_cols].height < field[full_idx].height) {
          ++field[full_idx - num_cols].higher_neighbours;

          // cell above has now seen all its neighbours, check if low point
          if (field[full_idx - num_cols].higher_neighbours ==
              4 - (row_idx == 1) - (col_idx == 0 || col_idx == num_cols - 1)) {
            risk_level += field[full_idx - num_cols].height + 1;
            //            std::cout << "Found new risk level of " << field[full_idx -
            //            num_cols].height + 1
            //                      << " at " << row_idx - 1 << ", " << col_idx << std::endl;
          }
        } else {
          ++field[full_idx].higher_neighbours;
        }
      }
    }
  }

  // check last row
  for (std::size_t col_idx = 0; col_idx < num_cols; ++col_idx) {
    std::size_t full_idx = (num_rows - 1) * num_cols + col_idx;

    if (field[full_idx].higher_neighbours == 3 - (col_idx == 0 || col_idx == num_cols - 1)) {
      risk_level += field[full_idx].height + 1;
      //      std::cout << "Found new risk level of " << (int)field[full_idx].height + 1
      //                << " in last row, col " << col_idx << std::endl;
    }
  }

  //  // final field
  //  for (std::size_t row_idx = 0; row_idx < num_rows; ++row_idx) {
  //    for (std::size_t col_idx = 0; col_idx < num_cols; ++col_idx) {
  //      std::size_t full_idx = row_idx * num_cols + col_idx;
  //
  //      std::cout << (int)field[full_idx].higher_neighbours << " ";
  //    }
  //    std::cout << std::endl;
  //  }

  auto t_end = std::chrono::steady_clock::now();

  std::cout << "Total risk level : " << risk_level << std::endl;
  std::cout << "Execution took "
            << std::chrono::duration_cast<std::chrono::microseconds>(t_end - t_start).count()
            << " us" << std::endl;

  return 0;
}
