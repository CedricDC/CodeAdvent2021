#include <cstddef>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

// brute force
// returns sum over certain window, or -1 if not ready
struct SlidingSum {
  SlidingSum(std::size_t N) : N{N}, values(N, 0) {}

  int addValue(int value) {
    std::size_t idx = counter % N;

    if (++counter < N) {
      sum += value;
      values[idx] = value;
      return -1; // not ready yet
    } else {
      sum = sum + (value - values[idx]);
      values[idx] = value;
      return sum;
    }
  }

  const std::size_t N;
  std::vector<int> values;
  int sum = 0;
  std::size_t counter = 0;
};

struct SlidingWindow {
  SlidingWindow(std::size_t N) : N{N}, values(N, 0) {}

  bool isGreater(int value) {
    std::size_t idx = counter % N;

    // sum is greater if we newest element is greater than oldest one
    // first "comparing sum" is at N+1'th element
    bool is_greater = (++counter > N) && (value > values[idx]);
    values[idx] = value;

    return is_greater;
  }

  const std::size_t N;
  std::vector<int> values;
  int sum = 0;
  std::size_t counter = 0;
};


int main() {

  std::string filename = "input.txt";
  std::ifstream ifile(filename);

  if (!ifile.good()) {
    std::cout << "Could not open " << filename << std::endl;
    return 1;
  }

  std::size_t N = 3;
  SlidingWindow sum(N);

  int depth = 0;
  std::size_t counter = 0;

  ifile >> depth;
  while (ifile.good()) {
    if (sum.isGreater(depth)) ++counter;

    ifile >> depth;
  }
  
  std::cout << "Number of increasing sums: " << counter << std::endl;

  return 0;
}
