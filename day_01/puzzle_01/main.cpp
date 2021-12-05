#include <cstddef>
#include <fstream>
#include <iostream>
#include <string>

int main() {

  std::string filename = "input.txt";
  std::ifstream ifile(filename);

  if (!ifile.good()) {
    std::cout << "Could not open " << filename << std::endl;
    return 1;
  }

  int depth = 0;
  int depth_prev = 0;
  std::size_t counter = 0;

  ifile >> depth_prev;
  ifile >> depth;
  while (ifile.good()) {
    if (depth > depth_prev) ++counter;

    depth_prev = depth;
    ifile >> depth;
  }
  
  std::cout << "Number of increasing measurements: " << counter << std::endl;

  return 0;
}
