#include <array>
#include <chrono>
#include <cmath>
#include <cstddef>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <numeric>
#include <string>
#include <vector>

namespace {

static constexpr int CYCLE = 7;
static constexpr int HATCHING = 2;

using PopType = std::size_t;  // overflow check triggered for 256 days

std::array<int, CYCLE> readPopulation(std::ifstream& ifile) {
  std::array<int, CYCLE> fish = {};

  // format is val,val,val,...,val
  int cycle_pos = 0;
  while (ifile >> cycle_pos) {
    ++fish[cycle_pos];
    ifile.get();  // skip comma
  }

  return fish;
}

PopType totalPopulation(const std::array<int, CYCLE>& init_pop, int days) {
  PopType final_population = 0;

  // upper bound on number of fish : each fish doubles every 7 days
  std::size_t num_fish = std::accumulate(init_pop.cbegin(), init_pop.cend(), int(0));
  std::size_t upper_bound = num_fish * ((std::size_t(2) << (days / CYCLE)) - 1);
  if (upper_bound > std::numeric_limits<PopType>::max()) {
    std::cout << "Number too big, will overflow!" << std::endl;
    std::cout << "Upper bound was " << upper_bound << " > " << std::numeric_limits<PopType>::max()
              << std::endl;
    return 0;
  }

  std::array<PopType, CYCLE> next_spawns;
  std::copy(init_pop.cbegin(), init_pop.cend(), next_spawns.begin());
  std::array<PopType, HATCHING> next_hatching = {};  // wait 2 days before entering the cycle

  auto current_spawn = next_spawns.begin();
  auto current_hatching = next_hatching.begin();

  for (int i = 0; i < days; ++i) {
    PopType hatching = *current_hatching;  // eggs that enter the cycle today
    *current_hatching = *current_spawn;    // eggs that are created today
    *current_spawn += hatching;

    // increase counter
    if (++current_spawn == next_spawns.end()) current_spawn = next_spawns.begin();
    if (++current_hatching == next_hatching.end()) current_hatching = next_hatching.begin();
  }

  // at end, add up all fish that are left
  final_population += std::accumulate(next_spawns.cbegin(), next_spawns.cend(), PopType(0));
  final_population += std::accumulate(next_hatching.cbegin(), next_hatching.cend(), PopType(0));

  return final_population;
}

}  // namespace

int main(int argc, char** argv) {
  if (argc < 3) {
    std::cout << "Required input arguments: <filename> <num_days>" << std::endl;
    return 1;
  }

  std::string filename = argv[1];
  std::ifstream ifile(filename);

  if (!ifile.good()) {
    std::cout << "Could not open " << filename << std::endl;
    return 1;
  }

  int days = std::atoi(argv[2]);
  std::array<int, CYCLE> initial_population = readPopulation(ifile);

  //  std::cout << "Initial population: " << std::endl;
  //  for (auto i : initial_population) std::cout << i << ", ";
  //  std::cout << std::endl;

  auto t_start = std::chrono::steady_clock::now();
  PopType total_population = totalPopulation(initial_population, days);
  auto t_end = std::chrono::steady_clock::now();
  std::cout << "final population after " << days << " days: " << total_population << std::endl;
  std::cout << "Execution took "
            << std::chrono::duration_cast<std::chrono::nanoseconds>(t_end - t_start).count()
            << " ns" << std::endl;

  return 0;
}
