#include <bitset>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

int main() {

  std::string filename = "input.txt";
  std::ifstream ifile(filename);

  if (!ifile.good()) {
    std::cout << "Could not open " << filename << std::endl;
    return 1;
  }

  std::string line;

  // determine length of byte code
  std::getline(ifile, line);
  int N = line.size();
  std::vector<int> counters(N, 0);

  // place back at start
  ifile.seekg(0);
  while (std::getline(ifile, line)) {
    for (int i = 0; i < N; ++i) {
      if (line[i] == '0') {
        --counters[i];
      } else {
        ++counters[i];
      }
    }
  }

  // evaluate codes
  // !Note: index zero is shifted by N-1
  int gamma_rate = 0;
  int epsilon_rate = 0;
  for (int i = 0; i < N; ++i) {
    int value = (1 << (N-1-i));
    if (counters[i] > 0) {
      gamma_rate |= value;
    } else if (counters[i] < 0) {
      epsilon_rate |= value;
    } else {
      std::cout << "Problem not well posed, no most common bit" << std::endl;
    }
  }

  int power_consumption = gamma_rate * epsilon_rate;

  std::cout << "gamma: " << std::bitset<8 * sizeof(int)>(gamma_rate).to_string() << ", epsilon: " << std::bitset<8 * sizeof(int)>(epsilon_rate).to_string() << std::endl;
  std::cout << "power consumption: " << power_consumption << std::endl;


  return 0;
}
