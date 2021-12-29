#include <chrono>
#include <fstream>
#include <iostream>
#include <limits>
#include <list>
#include <map>
#include <string>
#include <utility>
#include <vector>

// Assumpions:
// - every possible pair of characters has an insertion
namespace {

enum class Method { BRUTE_FORCE, SEQUENTIAL };

using PolyMap = std::map<std::string, char>;
using CharCounter = std::map<char, std::size_t>;

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

[[maybe_unused]] void printMapping(const PolyMap& mapping) {
  for (const auto& [key, value] : mapping) {
    std::cout << key << " --> " << value << std::endl;
  }
}

CharCounter buildPolymerBruteForce(const PolyMap& mapping, const std::string& start_string,
                                   std::size_t num_steps) {
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

  // print polymer
  //  std::cout << "Final polymer: " << std::endl;
  //  for (auto c : final_string) std::cout << c;
  //  std::cout << std::endl;

  return char_counter;
}

// Treat each character individually, starting from the back
//
// E.g. We know that the very last character will "react" num_steps times,
// each time with the character just generated previously
//
// After one step, the newly generated second-to-last character will react
// num_steps-1 times, etc
//
// For simplicity, we will keep track of this using a stack, where
// each element is (character, number of steps)
//
CharCounter buildPolymerSequential(const PolyMap& mapping, const std::string& start_string,
                                   std::size_t num_steps) {
  using Operation = std::pair<char, int>;  // end character, and how many steps to take
  std::map<char, std::size_t> char_counter;
  std::vector<Operation> operation_stack;

  for (auto c : start_string) operation_stack.emplace_back(c, num_steps);

  std::string key(2, ' ');
  while (operation_stack.size() > 1) {
    Operation op = operation_stack.back();
    operation_stack.pop_back();
    key[0] = operation_stack.back().first;
    key[1] = op.first;

    for (int i = op.second - 1; i >= 0; --i) {
      // newly generated character is start of next pair
      char inserted = mapping.at(key);

      if (i > 0) {
        key[0] = inserted;
        operation_stack.emplace_back(inserted, i);
      } else {
        ++char_counter[key[1]];
        ++char_counter[inserted];
      }
    }
  }

  return char_counter;
}

}  // namespace

int main(int argc, char** argv) {
  if (argc < 2) {
    std::cout << "Required input arguments: <filename> <num_steps> <method>" << std::endl;
    return 1;
  }

  std::string filename = argv[1];
  std::ifstream ifile(filename);

  unsigned int num_steps = 10;
  if (argc > 2) {
    num_steps = std::atol(argv[2]);
  }

  Method method = Method::BRUTE_FORCE;
  if (argc > 3) {
    method = static_cast<Method>(std::atoi(argv[3]));
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

  CharCounter char_counter;

  switch (method) {
    case Method::BRUTE_FORCE: {
      std::cout << "Brute force method" << std::endl;
      /*
       * option 1 : brute force
       */
      char_counter = buildPolymerBruteForce(mapping, start_string, num_steps);
    } break;
    case Method::SEQUENTIAL: {
      std::cout << "Sequential method" << std::endl;

      /*
       * option 2 : Approach for big polymers, cannot save whole thing in memory
       */
      char_counter = buildPolymerSequential(mapping, start_string, num_steps);
    } break;
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
