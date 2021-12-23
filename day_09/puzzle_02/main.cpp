#include <chrono>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

namespace {

struct FloorTile {
  FloorTile(uint8_t h) : height(h) {
    if (height == 9) visited = true;
  }

  uint8_t height = 0;
  bool visited = false;
};

std::size_t findBasin(const std::size_t num_cols, const std::size_t num_rows,
                      std::vector<FloorTile>& field, std::size_t row, std::size_t col) {
  std::size_t full_idx = col + row * num_cols;

  if (field[full_idx].visited) return 0;
  //  std::cout << "Checking " << row << ", " << col << std::endl;

  // mark as visited
  field[full_idx].visited = true;
  std::size_t basin_size = 1;

  // go right
  if (col < num_cols - 1) {
    basin_size += findBasin(num_cols, num_rows, field, row, col + 1);
  }

  // go down
  if (row < num_rows - 1) {
    basin_size += findBasin(num_cols, num_rows, field, row + 1, col);
  }

  // go left
  if (col > 0) {
    basin_size += findBasin(num_cols, num_rows, field, row, col - 1);
  }

  // go up
  if (row > 0) {
    basin_size += findBasin(num_cols, num_rows, field, row - 1, col);
  }

  return basin_size;
}

}  // namespace

//
// This really feels like the worst way to do this, but let's try to
// iterate through the whole field and find basins recursively everywhere
//
// Situation not completely clear:
// Is this impossible?
//
//  9999999
//  6666666
//  4346434
//  6666666
//
// --> we have two low points, and two basins
// According to description, assume this is never the case?
//
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

  std::vector<std::size_t> basin_sizes;

  for (std::size_t row_idx = 0; row_idx < num_rows; ++row_idx) {
    for (std::size_t col_idx = 0; col_idx < num_cols; ++col_idx) {
      std::size_t full_idx = row_idx * num_cols + col_idx;

      if (!field[full_idx].visited) {
        //        std::cout << "Starting new basin search at " << row_idx << ", " << col_idx <<
        //        std::endl;
        std::size_t new_basin_size = findBasin(num_cols, num_rows, field, row_idx, col_idx);
        basin_sizes.push_back(new_basin_size);
        //        std::cout << "Found new basin of size : " << new_basin_size << std::endl;
      }
    }
  }

  // find three biggest basins
  std::size_t max_basins[3] = {};  // sorted in ascending order
  for (auto s : basin_sizes) {
    if (s > max_basins[0]) max_basins[0] = s;
    if (s > max_basins[1]) std::swap(max_basins[0], max_basins[1]);
    if (s > max_basins[2]) std::swap(max_basins[1], max_basins[2]);
  }

  std::size_t result = max_basins[0] * max_basins[1] * max_basins[2];

  auto t_end = std::chrono::steady_clock::now();

  std::cout << "Final value: " << result << std::endl;
  std::cout << "Execution took "
            << std::chrono::duration_cast<std::chrono::microseconds>(t_end - t_start).count()
            << " us" << std::endl;

  return 0;
}
