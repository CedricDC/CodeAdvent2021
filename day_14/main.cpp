#include <chrono>
#include <fstream>
#include <iostream>
#include <limits>
#include <list>
#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>

// Assumpions:
// - every possible pair of characters has an insertion
namespace {

enum class Method { BRUTE_FORCE, SEQUENTIAL, TREE, TREE_OPTIMIZED };

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

    [[maybe_unused]] void printCounter(const CharCounter& counter) {
  for (const auto& [c, num] : counter) std::cout << c << " (" << num << "), " << std::endl;
  std::cout << std::endl;
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
        ++char_counter[inserted];
      }
    }

    ++char_counter[key[1]];
  }

  // need to add last character in operation stack
  ++char_counter[operation_stack.back().first];
  operation_stack.pop_back();

  return char_counter;
}

// Build a recursive tree of all possible transitions
struct PolymerTree {
  PolymerTree(const PolyMap& mapping);

  const std::map<char, std::size_t>& count(const std::string& key, int num_steps);

  // Node encode the generated character per key
  // Level 0 for a Node is level 1 seen from the key
  struct Node {
    Node(const std::string& key);

    void initialize(char c);

    void connectLeft(Node* left);

    void connectRight(Node* right);

    const std::map<char, std::size_t>& count(unsigned int level);

   private:
    void addCounter_(const CharCounter& new_counter, CharCounter& base_counter);

    std::string key_;

    Node* left_ = nullptr;
    Node* right_ = nullptr;

    std::vector<CharCounter> counter_;  // counter for each level down
  };

 private:
  // Will also take care of correctly destroying all elements of tree
  std::map<std::string, Node> node_list_;
};

CharCounter buildPolymerTree(const PolyMap& mapping, const std::string& start_string,
                             std::size_t num_steps) {
  PolymerTree tree(mapping);

  CharCounter char_counter;

  // initialise all counters somewhat inefficiently
  for (const auto& elem : mapping) char_counter[elem.second] = 0;

  // add starting string to char counter
  for (const auto& c : start_string) ++char_counter[c];

  // process subtree of character pairs one at a time
  if (num_steps > 0) {
    for (std::size_t i = 0; i < start_string.size() - 1; ++i) {
      std::string key;
      key.push_back(start_string[i]);
      key.push_back(start_string[i + 1]);

      // add counter
      for (const auto& elem : tree.count(key, num_steps - 1)) {
        char_counter[elem.first] += elem.second;
      }
    }
  }

  return char_counter;
}

// Build a recursive tree of all possible transitions, optimizing lookup
struct PolymerTreeOptimized {
  PolymerTreeOptimized(const PolyMap& mapping, std::size_t max_level);

  void addCount(const std::string& key, std::size_t level, CharCounter& char_counter);

  // Node encode the generated character per key
  // Level 0 for a Node is level 1 seen from the key
  struct Node {
    // vector encoding: counter for A - Z for each level
    using LocalCounter = std::vector<std::size_t>;

    Node(const std::string& key, std::size_t max_level);

    void initialize(char c);

    void connectLeft(Node* left);

    void connectRight(Node* right);

    void addCount(CharCounter& char_counter, std::size_t level);

   private:
    std::size_t* count_(unsigned int level);

    static constexpr std::size_t getStartIdx(std::size_t level) { return 26 * level; }
    static constexpr std::size_t getCharIdx(std::size_t level, char c) {
      return 26 * level + (c - 'A');
    }

    std::string key_;
    char generated_char_;

    Node* left_ = nullptr;
    Node* right_ = nullptr;

    LocalCounter counter_;
    std::vector<bool> counter_initialized_;
  };

 private:
  // Will also take care of correctly destroying all elements of tree
  std::map<std::string, Node> node_list_;
};

CharCounter buildPolymerTreeOptimized(const PolyMap& mapping, const std::string& start_string,
                                      std::size_t num_steps) {
  CharCounter char_counter;

  // initialise all counters
  for (const auto& elem : mapping) char_counter[elem.second] = 0;

  // add starting string to char counter
  for (const auto& c : start_string) ++char_counter[c];

  // process subtree of character pairs one at a time
  if (num_steps > 0) {
    PolymerTreeOptimized tree(mapping, num_steps - 1);

    for (std::size_t i = 0; i < start_string.size() - 1; ++i) {
      std::string key;
      key.push_back(start_string[i]);
      key.push_back(start_string[i + 1]);

      tree.addCount(key, num_steps - 1, char_counter);
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
    case Method::TREE: {
      std::cout << "Tree method" << std::endl;

      /*
       * option 3 : Approach for big polymers, build linked tree
       */
      char_counter = buildPolymerTree(mapping, start_string, num_steps);
    } break;
    case Method::TREE_OPTIMIZED: {
      std::cout << "Optimized Tree method" << std::endl;

      /*
       * option 4 : Approach for big polymers, build linked tree, using contiguous memory
       */
      char_counter = buildPolymerTreeOptimized(mapping, start_string, num_steps);
    } break;
  }

  // print
  printCounter(char_counter);

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

namespace {

PolymerTree::PolymerTree(const PolyMap& mapping) {
  for (const auto& [key, c] : mapping) {
    // e.g. getting AB -> C

    // children generated : AC & CB
    std::string key_left;
    key_left.push_back(key[0]);
    key_left.push_back(c);

    std::string key_right;
    key_right.push_back(c);
    key_right.push_back(key[1]);

    // generate nodes if they don't exist yet, returns std::pair<iterator,bool>
    auto res = node_list_.emplace(key, key);
    auto left = node_list_.emplace(key_left, key_left);
    auto right = node_list_.emplace(key_right, key_right);

    // initialize generated child character
    res.first->second.initialize(c);

    // add connections
    res.first->second.connectLeft(&left.first->second);
    res.first->second.connectRight(&right.first->second);
  }
}

const CharCounter& PolymerTree::count(const std::string& key, int num_steps) {
  return node_list_.at(key).count(num_steps);
}

PolymerTree::Node::Node(const std::string& key) : key_(key) {}

void PolymerTree::Node::initialize(char c) {
  // level 0
  counter_.push_back(CharCounter{});
  counter_.front()[c] = 1;
}

void PolymerTree::Node::connectLeft(Node* left) { left_ = left; }

void PolymerTree::Node::connectRight(Node* right) { right_ = right; }

const CharCounter& PolymerTree::Node::count(unsigned int level) {
  // TODO : we can skip unused levels
  if (counter_.size() <= level) {
    // a bit more work right now, but this recursively fills elements further down the tree
    // and we don't have to worry about empty elements
    for (unsigned int i = counter_.size(); i <= level; ++i) {
      counter_.push_back(counter_.front());  // start with very top level
      auto& current_max = counter_.back();

      addCounter_(left_->count(i - 1), current_max);
      addCounter_(right_->count(i - 1), current_max);
    }
  }

  return counter_[level];
}

void PolymerTree::Node::addCounter_(const CharCounter& new_counter, CharCounter& base_counter) {
  for (const auto& elem : new_counter) {
    auto it = base_counter.find(elem.first);
    if (it == base_counter.end()) {
      base_counter[elem.first] = elem.second;
    } else {
      it->second += elem.second;
    }
  }
}

PolymerTreeOptimized::PolymerTreeOptimized(const PolyMap& mapping, std::size_t max_level) {
  for (const auto& [key, c] : mapping) {
    // e.g. getting AB -> C

    // children generated : AC & CB
    std::string key_left;
    key_left.push_back(key[0]);
    key_left.push_back(c);

    std::string key_right;
    key_right.push_back(c);
    key_right.push_back(key[1]);

    // generate nodes if they don't exist yet, returns std::pair<iterator,bool>
    auto res = node_list_.try_emplace(key, key, max_level);
    auto left = node_list_.try_emplace(key_left, key_left, max_level);
    auto right = node_list_.try_emplace(key_right, key_right, max_level);

    // initialize generated child character
    res.first->second.initialize(c);

    // add connections
    res.first->second.connectLeft(&left.first->second);
    res.first->second.connectRight(&right.first->second);
  }
}

void PolymerTreeOptimized::addCount(const std::string& key, std::size_t level,
                                    CharCounter& char_counter) {
  node_list_.at(key).addCount(char_counter, level);
}

PolymerTreeOptimized::Node::Node(const std::string& key, std::size_t max_level)
    : key_(key),
      generated_char_(' '),
      counter_((max_level + 1) * 26, 0),
      counter_initialized_((max_level + 1), false) {}

void PolymerTreeOptimized::Node::initialize(char c) {
  // level 0
  unsigned int level = 0;
  if (!counter_initialized_[level]) {
    generated_char_ = c;
    counter_[getCharIdx(level, generated_char_)] = 1;
    counter_initialized_[level] = true;
  }
}

void PolymerTreeOptimized::Node::connectLeft(Node* left) { left_ = left; }

void PolymerTreeOptimized::Node::connectRight(Node* right) { right_ = right; }

void PolymerTreeOptimized::Node::addCount(CharCounter& char_counter, std::size_t level) {
  std::size_t* local_counter = count_(level);
  for (unsigned int i = 0; i < 26; ++i, ++local_counter) {
    if (*local_counter > 0) char_counter[static_cast<char>('A' + i)] += *local_counter;
  }
}

std::size_t* PolymerTreeOptimized::Node::count_(unsigned int level) {
  std::size_t* lvl_start = counter_.data() + getStartIdx(level);

  if (!counter_initialized_[level]) {
    counter_[getCharIdx(level, generated_char_)] = 1;

    auto left_counter = left_->count_(level - 1);
    for (unsigned int i = 0; i < 26; ++i) *(lvl_start + i) += *(left_counter++);

    auto right_counter = right_->count_(level - 1);
    for (unsigned int i = 0; i < 26; ++i) *(lvl_start + i) += *(right_counter++);

    counter_initialized_[level] = true;
  }

  return lvl_start;
}

}  // namespace
