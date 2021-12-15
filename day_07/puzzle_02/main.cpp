#include <algorithm>
#include <chrono>
#include <cmath>
#include <fstream>
#include <iostream>
#include <numeric>
#include <string>
#include <vector>

#include <iomanip>

//
// This would be a constrained integer quadratic programming problem
//
// I suspect there is an easier solution however.
//
// Based on summing up the values 1 ... |pos_i - x|, the total fuel at position x is:
//
// f(x) = sum( (|pos_i - x| + 1) * (|pos_i - x|) / 2)
//
// The move from x-1 to x is worth it if :
// f(x) - f(x-1) < 0
//
// The change between those two fuel consumptions can be expressed as:
// sum_{i < x}|pos_i - x| - sum_{i >= x}|pos_i - x| < 0
//
// Intuitively, the optimum would be the center of moments, but the integer
// constraint makes it less clear which side we should move to.
//
// Move to x is worth it
// sum_{i < x} |pos_i - x| < sum_{i >= x}|pos_i - x|
// sum_A (x - pos_i)       < sum_B (pos_i - x), using A := {i: i < x}, B := {i: i >= x}
// NA * x - sum_A(pos_i)   < sum_B(pos_i) - NB * x
// (NA + NB) * x           < sum_A(pos_i) + sum_B(pos_i)
// Ntot * x                < sum_tot(pos_i)
//                       x < sum_tot(pos_i) / Ntot
//
// x_optimum = floor(sum_tot(pos_i) / Ntot)
//
// Note : The above appears to be incorrect, require search in vicinity of mean
//        Reason TBD
//

namespace {

std::size_t computeFuel(const std::vector<int>& positions, int target_position) {
  std::size_t fuel = 0;
  for (auto pos : positions) {
    int distance = std::abs(pos - target_position);
    fuel += (distance + 1) * distance;
  }
  fuel = fuel / 2;
  return fuel;
}

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

  // insertion could be optimized
  auto t_start = std::chrono::steady_clock::now();
  std::vector<int> positions;
  std::size_t sum = 0;
  int pos;
  while (ifile >> pos) {
    positions.push_back(pos);
    sum += pos;
    ifile.get();
  }

  // this should be the theoretically optimal position
  {
    double optimal_position_d = static_cast<double>(sum) / positions.size();
    std::cout << "Optimal position: " << optimal_position_d << std::endl;

    {
      double fuel = 0;
      for (auto pos : positions) {
        double distance = std::abs(pos - optimal_position_d);
        fuel += (distance + 1) * distance;
      }
      fuel = fuel / 2;
      std::cout << "optimal fuel: " << std::setprecision(15) << fuel << std::endl;
    }
  }

  // This is restricted to integer locations
  int optimal_position = sum / positions.size();
  std::size_t fuel = computeFuel(positions, optimal_position);

  // search for optimum in vicinity (why is this needed??)
  std::size_t tmp_fuel = computeFuel(positions, optimal_position + 1);
  if (fuel > tmp_fuel) {
    for (int i = 2; (i < 100) && (fuel > tmp_fuel); ++i) {
      fuel = tmp_fuel;  // found new minimum!
      ++optimal_position;

      tmp_fuel = computeFuel(positions, optimal_position + i);
    }
  } else {
    tmp_fuel = computeFuel(positions, optimal_position - 1);
    for (int i = 2; (i < 100) && (fuel > tmp_fuel); ++i) {
      fuel = tmp_fuel;  // found new minimum!
      --optimal_position;

      tmp_fuel = computeFuel(positions, optimal_position - i);
    }
  }
  auto t_end = std::chrono::steady_clock::now();

  std::cout << "Possible Optimal target position is at " << optimal_position << ", fuel cost is "
            << fuel << std::endl;
  std::cout << "Execution took "
            << std::chrono::duration_cast<std::chrono::microseconds>(t_end - t_start).count()
            << " us" << std::endl;

  return 0;
}
