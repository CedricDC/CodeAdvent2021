#include <array>
#include <chrono>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

namespace {

using OctopusValue = uint8_t;

// Given in problem description
constexpr int NUM_COLS = 10;
constexpr int NUM_ROWS = 10;
constexpr int NUM_OCTS = NUM_COLS * NUM_ROWS;
constexpr int FLASH_LIMIT = 10;

using Field = std::array<OctopusValue, NUM_COLS * NUM_ROWS>;
using OctopusStack = std::vector<int>;

[[maybe_unused]] void printField(const Field& field) {
  int idx = 0;
  for (int row_idx = 0; row_idx < NUM_ROWS; ++row_idx) {
    for (int col_idx = 0; col_idx < NUM_COLS; ++col_idx) {
      std::cout << std::setw(2) << static_cast<int>(field[idx++]);
    }
    std::cout << std::endl;
  }
  std::cout << "---------------" << std::endl;
}

std::size_t increaseNeighbours(Field& field, OctopusStack& stack, int row_idx, int col_idx) {
  std::size_t size_start = stack.size();

  // start top-left
  int idx = (col_idx - 1) + (row_idx - 1) * NUM_COLS;
  if (row_idx > 0) {
    // top left
    if (col_idx > 0 && ++field[idx] == FLASH_LIMIT) stack.emplace_back(idx);
    ++idx;

    // top
    if (++field[idx] == FLASH_LIMIT) stack.emplace_back(idx);
    ++idx;

    // top right
    if ((col_idx < NUM_COLS - 1) && ++field[idx] == FLASH_LIMIT) stack.emplace_back(idx);
    idx += (NUM_COLS - 2);
  } else {
    idx += NUM_COLS;
  }

  // left
  idx = (col_idx - 1) + row_idx * NUM_COLS;
  if (col_idx > 0 && ++field[idx] == FLASH_LIMIT) stack.emplace_back(idx);
  idx += 2;

  // right
  if ((col_idx < NUM_COLS - 1) && ++field[idx] == FLASH_LIMIT) stack.emplace_back(idx);
  idx += (NUM_COLS - 2);

  if (row_idx < NUM_ROWS - 1) {
    // bottom left
    if (col_idx > 0 && ++field[idx] == FLASH_LIMIT) stack.emplace_back(idx);
    ++idx;

    // bottom
    if (++field[idx] == FLASH_LIMIT) stack.emplace_back(idx);
    ++idx;

    // bottom right
    if ((col_idx < NUM_COLS - 1) && ++field[idx] == FLASH_LIMIT) stack.emplace_back(idx);
  }

  std::size_t num_flashes = (stack.size() - size_start);

  return num_flashes;
}

}  // namespace

int main(int argc, char** argv) {
  if (argc < 2) {
    std::cout << "Required input arguments: <filename> (optional: <method> <num_steps>)"
              << std::endl;
    return 1;
  }

  std::string filename = argv[1];
  std::ifstream ifile(filename);

  if (!ifile.good()) {
    std::cout << "Could not open " << filename << std::endl;
    return 1;
  }

  auto t_start = std::chrono::steady_clock::now();
  Field field;
  int linear_idx = 0;
  for (int row_idx = 0; row_idx < NUM_ROWS; ++row_idx) {
    for (int col_idx = 0; col_idx < NUM_COLS; ++col_idx) {
      // convert char to int
      field[linear_idx++] = static_cast<uint8_t>(ifile.get() - '0');
    }
    ifile.get();  // skip newline character
  }

  int num_steps = 100;
  std::size_t total_flashes = 0;

  // next octopusses to process
  OctopusStack octopus_stack;
  octopus_stack.reserve(NUM_OCTS);

  for (int iter = 0; iter < num_steps; ++iter) {
    std::size_t num_flashes = 0;

    // increase all by 1 by respecting ones which just flashed, and add all new flashers to stack
    Field::iterator octopus = field.begin();
    for (int idx = 0; idx < NUM_OCTS; ++idx, ++octopus) {
      if (*octopus < FLASH_LIMIT - 1) {
        ++(*octopus);
      } else if (*octopus == FLASH_LIMIT - 1) {  // flashing!
        *octopus = FLASH_LIMIT;
        octopus_stack.emplace_back(idx);
      } else {  // flashed last time, reset to 0 and in*octopus by 1
        *octopus = 1;
      }
    }
    num_flashes += octopus_stack.size();

    // now update and process stack until empty
    while (!octopus_stack.empty()) {
      int idx = octopus_stack.back();
      int row_idx = idx / NUM_COLS;
      int col_idx = idx % NUM_COLS;
      octopus_stack.pop_back();
      num_flashes += increaseNeighbours(field, octopus_stack, row_idx, col_idx);
    }

    total_flashes += num_flashes;
  }

  auto t_end = std::chrono::steady_clock::now();

  std::cout << "Total number of flashes: " << total_flashes << std::endl;
  std::cout << "Execution took "
            << std::chrono::duration_cast<std::chrono::microseconds>(t_end - t_start).count()
            << " us" << std::endl;

  return 0;
}
