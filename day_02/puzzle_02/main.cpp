#include <fstream>
#include <iostream>
#include <map>
#include <string>

namespace {
  bool readAction(std::ifstream& ifile, std::string& action_name, int& value) {
    ifile >> action_name;
    ifile >> value;

    return ifile.good();
  }
}

// Note: order of application is irrelevant, can tally later
int main() {

  std::string filename = "input.txt";
  std::ifstream ifile(filename);

  if (!ifile.good()) {
    std::cout << "Could not open " << filename << std::endl;
    return 1;
  }

  int horizontal_distance = 0;
  int aim = 0;
  int depth = 0;

  std::string action_name;
  int value = 0;
  while (readAction(ifile, action_name, value)) {
    switch (action_name[0]) {
      case 'f': { // forward
        horizontal_distance += value;
        depth += aim * value;
        break;
      } 
      case 'u': { // up
        aim -= value;
        break;
      } 
      case 'd': { // down
        aim += value;
        break;
      } 
    }
  }

  std::cout << "horizontal distance: " << horizontal_distance << std::endl;
  std::cout << "depth: " << depth << std::endl;
  std::cout << "multiplied: " << depth * horizontal_distance << std::endl;

  return 0;
}
