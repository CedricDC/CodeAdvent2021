#include <chrono>
#include <cmath>
#include <cstddef>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

namespace {

void printField(const std::vector<int>& field, int width) {
  auto it = field.cbegin();
  std::size_t counter = 0;
  while (it != field.cend()) {
    if (*it == 0)
      std::cout << "*";
    else
      std::cout << *it;

    if (++counter % width == 0) std::cout << std::endl;
    ++it;
  }
}

// brute force, use array
// assume we know the max values are < 1000
int findMaxOverlap(std::ifstream& ifile) {
  static constexpr int lengthArrow = 4;
  static constexpr int criticalDanger = 2;
  static constexpr int fieldSize = 1000;

  // row first indexing
  int N = fieldSize;
  std::vector<int> field(N * N, 0);
  auto getIdx = [N](int x, int y) { return x + y * N; };

  int xA = 0;
  int yA = 0;
  int xB = 0;
  int yB = 0;

  int dangerCount = 0;
  auto readLine = [&]() {
    char c;
    ifile >> xA >> c >> yA;
    ifile.seekg(lengthArrow, std::ios::cur);
    ifile >> xB >> c >> yB;

    //    std::cout << xA << "," << yA << " --> " << xB << "," << yB << std::endl;
  };

  int stepSize = 0;
  int numSteps = 0;

  while (ifile.good()) {
    readLine();

    // yA == yB : horizontal direction, else vertical direction
    if (yA == yB) {
      if (xA > xB) std::swap(xA, xB);

      // horizontal direction
      stepSize = 1;
      numSteps = 1 + (xB - xA);  // include end
    } else if (xA == xB) {
      if (yA > yB) std::swap(yA, yB);

      // vertical direction
      stepSize = N;
      numSteps = 1 + (yB - yA);  // include end
    } else {
      // diagonal
      stepSize = std::copysign(1, xB - xA) + std::copysign(N, yB - yA);
      numSteps = 1 + std::abs(yB - yA);
    }

    int idx = getIdx(xA, yA);  // starting point
    for (int i = 0; i < numSteps; ++i, idx += stepSize) {
      // only count each dangerous field once
      if (++field[idx] == criticalDanger) {
        ++dangerCount;
      }
    }
  }

  //  printField(field, N);

  return dangerCount;
}

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

  auto t_start = std::chrono::steady_clock::now();
  int maxCount = findMaxOverlap(ifile);
  auto t_end = std::chrono::steady_clock::now();
  std::cout << "areas with high danger: " << maxCount << std::endl;
  std::cout << "Execution took "
            << std::chrono::duration_cast<std::chrono::microseconds>(t_end - t_start).count()
            << " us" << std::endl;

  return 0;
}
