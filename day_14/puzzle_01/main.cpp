#include <chrono>
#include <fstream>
#include <iostream>
#include <limits>
#include <list>
#include <map>
#include <string>

// Assumpions:
// - every possible pair of characters has an insertion
namespace {

using PolyMap = std::map<std::string, char>;

// return polymer length after N steps
constexpr std::size_t polyLength(std::size_t start_length, std::size_t num_steps) {
  return 1 + (1 << num_steps) * (start_length - 1);
}

PolyMap readMapping(std::ifstream& ifile) {
  PolyMap polymer_mapping;
  std::string key(2, 'x');

  while (ifile.good()) {
    key[0] = ifile.get();
    key[1] = ifile.get();
    ifile.ignore(4);  // skip " -> "
    polymer_mapping[key] = ifile.get();
    ifile.ignore(1, '\n');  // skip until end of line
  }

  return polymer_mapping;
}

void printMapping(const PolyMap& mapping) {
  for (const auto& [key, value] : mapping) {
    std::cout << key << " --> " << value << std::endl;
  }
}

}  // namespace

int main(int argc, char** argv) {
  if (argc < 2) {
    std::cout << "Required input arguments: <filename>" << std::endl;
    return 1;
  }

  std::string filename = argv[1];
  std::ifstream ifile(filename);

  unsigned int num_steps = 10;
  if (argc > 2) {
    num_steps = std::atol(argv[2]);
  }

  if (!ifile.good()) {
    std::cout << "Could not open " << filename << std::endl;
    return 1;
  }

  auto t_start = std::chrono::steady_clock::now();

  // parse file
  std::string start_string;
  std::getline(ifile, start_string);
  std::cout << "Start string: " << start_string << std::endl;

  // skip empty line
  ifile.ignore(1, '\n');

  // parse mappings
  auto mapping = readMapping(ifile);

  /*
   * option 1 : brute force
   */
  std::list<char> final_string;
  std::map<char, std::size_t> char_counter;
  for (auto c : start_string) {
    final_string.push_back(c);
    ++char_counter[c];
  }

  for (std::size_t iter = 0; iter < num_steps; ++iter) {
    std::string key(2, ' ');

    auto c_iter = final_string.begin();
    key[1] = *c_iter;
    while (++c_iter != final_string.end()) {
      key[0] = key[1];
      key[1] = *c_iter;

      char inserted = mapping.at(key);
      final_string.insert(c_iter, inserted);
      ++char_counter[inserted];
    }
  }

  // compute desired result
  std::size_t most_common = 0;
  std::size_t least_common = std::numeric_limits<std::size_t>::max();
  for (const auto& item : char_counter) {
    std::size_t count = item.second;
    if (most_common < count) {
      most_common = count;
    }
    if (least_common > count) {
      least_common = count;
    }
  }
  std::size_t score = most_common - least_common;

  auto t_end = std::chrono::steady_clock::now();

  std::cout << "Final score: " << score << std::endl;
  std::cout << "Execution took "
            << std::chrono::duration_cast<std::chrono::microseconds>(t_end - t_start).count()
            << " us" << std::endl;

  return 0;
}
