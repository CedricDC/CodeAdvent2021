#include <algorithm>
#include <chrono>
#include <cmath>
#include <fstream>
#include <iostream>
#include <numeric>
#include <string>
#include <vector>

//
// In general, this is an integer linear programming problem, but
// maybe we can brute force it fairly easily
//
// fuel_t : fuel if at position t
//
// fuel_0 = sum(pos_i)
//
// fuel_t+1 = fuel_t + N_{pos_i < t} - N_{pos_i => t}
//
// In other words, stepping to the right is beneficial as long as
//    N_{pos_i < t} < N_{pos_i => t}
// --> optimal position is median!
//
// corner case: even number of crabs -> either side is fine
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

  // insertion could be optimized
  auto t_start = std::chrono::steady_clock::now();
  std::vector<int> positions;
  int pos;
  while (ifile >> pos) {
    positions.push_back(pos);
    ifile.get();
  }

  std::size_t num_crabs = positions.size();
  auto m_it = positions.begin() + (0.5 * num_crabs);
  std::nth_element(positions.begin(), m_it, positions.end());  // sort up to m'th element
  int optimal_position = *m_it;

  std::size_t fuel = 0;
  auto it = positions.cbegin();
  for (; it != m_it; ++it) {
    fuel += (optimal_position - *it);
  }
  for (; it != positions.cend(); ++it) {
    fuel += (*it - optimal_position);
  }
  auto t_end = std::chrono::steady_clock::now();

  std::cout << "Possible Optimal target position is at " << optimal_position << ", fuel cost is "
            << fuel << std::endl;
  std::cout << "Execution took "
            << std::chrono::duration_cast<std::chrono::microseconds>(t_end - t_start).count()
            << " us" << std::endl;

  return 0;
}
