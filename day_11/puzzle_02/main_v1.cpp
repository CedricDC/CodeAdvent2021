#include <array>
#include <chrono>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>

namespace {

struct Octopus {
  uint8_t value = 0;
  bool flashed = false;
};

// Given in problem description
constexpr int NUM_COLS = 10;
constexpr int NUM_ROWS = 10;
constexpr int NUM_OCTS = NUM_COLS * NUM_ROWS;

using Field = std::array<Octopus, NUM_COLS * NUM_ROWS>;

void printField(const Field& field) {
  int idx = 0;
  for (int row_idx = 0; row_idx < NUM_ROWS; ++row_idx) {
    for (int col_idx = 0; col_idx < NUM_COLS; ++col_idx) {
      std::cout << static_cast<int>(field[idx++].value);
    }
    std::cout << std::endl;
  }
}

bool increaseNeighbours(Field& field, int col_idx, int row_idx) {
  bool any_flash = false;

  // start top-left
  int idx = (col_idx - 1) + (row_idx - 1) * NUM_COLS;
  if (row_idx > 0) {
    // top left
    if (col_idx > 0 && ++field[idx].value == 10) any_flash = true;
    ++idx;

    // top
    if (++field[idx].value == 10) any_flash = true;
    ++idx;

    // top right
    if ((col_idx < NUM_COLS - 1) && ++field[idx].value == 10) any_flash = true;
    idx += (NUM_COLS - 2);
  } else {
    idx += NUM_COLS;
  }

  // left
  if (col_idx > 0 && ++field[idx].value == 10) any_flash = true;
  idx += 2;

  // right
  if ((col_idx < NUM_COLS - 1) && ++field[idx].value == 10) any_flash = true;
  idx += (NUM_COLS - 2);

  if (row_idx < NUM_ROWS - 1) {
    // bottom left
    if (col_idx > 0 && ++field[idx].value == 10) any_flash = true;
    ++idx;

    // bottom
    if (++field[idx].value == 10) any_flash = true;
    ++idx;

    // bottom right
    if ((col_idx < NUM_COLS - 1) && ++field[idx].value == 10) any_flash = true;
  }

  return any_flash;
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
  std::array<Octopus, NUM_COLS * NUM_ROWS> field;
  int linear_idx = 0;
  for (int row_idx = 0; row_idx < NUM_ROWS; ++row_idx) {
    for (int col_idx = 0; col_idx < NUM_COLS; ++col_idx) {
      field[linear_idx++].value = static_cast<uint8_t>(ifile.get() - '0');
    }
    ifile.get();  // skip newline character
  }

  std::size_t last_flashes = 0;
  std::size_t iteration = 0;

  while (last_flashes < NUM_OCTS) {
    last_flashes = 0;

    // increase all by 1
    for (auto& oct : field) ++oct.value;

    bool any_flashed = true;
    while (any_flashed) {
      any_flashed = false;

      // increase counters everywhere
      int linear_idx = 0;
      for (int row_idx = 0; row_idx < NUM_ROWS; ++row_idx) {
        for (int col_idx = 0; col_idx < NUM_COLS; ++col_idx) {
          if (field[linear_idx].value > 9 && !field[linear_idx].flashed) {
            field[linear_idx].flashed = true;
            any_flashed |= increaseNeighbours(field, col_idx, row_idx);
          }
          ++linear_idx;
        }
      }
    }

    // traverse one more time to count flashes and wrap counters
    for (auto& oct : field) {
      if (oct.flashed) {
        ++last_flashes;
        oct.flashed = 0;
        oct.value = 0;
      }
    }

    ++iteration;
  }

  auto t_end = std::chrono::steady_clock::now();

  std::cout << "First synced flash in iteration: " << iteration << std::endl;
  std::cout << "Execution took "
            << std::chrono::duration_cast<std::chrono::microseconds>(t_end - t_start).count()
            << " us" << std::endl;

  return 0;
}
