#include <fstream>
#include <iostream>
#include <map>
#include <string>

namespace {
  using Action = std::string;

  bool readAction(std::ifstream& ifile, Action& action, int& value) {
    ifile >> action;
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

  std::map<Action, int> travel;

  // initialize expected keys with 0
  travel["forward"] = 0;
  travel["up"] = 0;
  travel["down"] = 0;

  Action action;
  int value = 0;
  while (readAction(ifile, action, value)) {
    travel[action] += value; 
  }

  int horizontal_distance = travel["forward"];
  int depth = travel["down"] - travel["up"];
  
  std::cout << "horizontal distance: " << horizontal_distance << std::endl;
  std::cout << "depth: " << depth << std::endl;
  std::cout << "multiplied: " << depth * horizontal_distance << std::endl;

  return 0;
}
