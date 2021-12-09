#include <bitset>
#include <chrono>
#include <fstream>
#include <functional>
#include <iostream>
#include <limits>
#include <memory>
#include <string>
#include <vector>

// Two options:
// 1) reconstruct the value by looking at most common bit value
// 2) Build binary tree containing all values
//
// The problem with 1) is that not every value is actually present
// in the file. With that in mind, going with option 2
//
// There is probably an elegant way to do this...

namespace {

// 1 : left, 0 : right
// level == depth at root, and zero at lowest level
static constexpr bool goLeft(int value, int level) { return (value & (1 << level)); }

struct Node {
  std::unique_ptr<Node> left = nullptr;
  std::unique_ptr<Node> right = nullptr;

  std::size_t num_left = 0;   // number of values in left subtree
  std::size_t num_right = 0;  // number of values in right subtree

  int value = -1;
  std::size_t counter = 0;  // we can have multiple values per node

  Node(int value, std::size_t count = 1);

  void addValue(int new_value, int level, std::size_t total);
};

struct BinaryTree {
  BinaryTree(int depth, int root_value);

  //! add value to binary tree
  void addValue(int value);

  //! traverse the tree based on selector operation
  int search(std::function<int(const Node* const node)> selector);

  //! return number of values in tree
  std::size_t size() const;

 private:
  int depth_;
  std::size_t counter_ = 0;
  Node root_;
};

//! Search criteria
int searchOxygenRating(const Node* const node);
int searchScrubberRating(const Node* const node);

}  // namespace

int main(int argc, char** argv) {
  if (argc < 2) {
    std::cout << "Require filename as input argument" << std::endl;
    return 1;
  }

  auto t_start = std::chrono::steady_clock::now();
  std::string filename = argv[1];
  std::ifstream ifile(filename);

  if (!ifile.good()) {
    std::cout << "Could not open " << filename << std::endl;
    return 1;
  }

  // determine length of byte code and value of root by reading first line
  std::string line;
  std::getline(ifile, line);
  int N = line.size();
  if ((1 << N) > std::numeric_limits<int>::max()) {
    std::cout << "Oh oh, tree cannot handle these big values" << std::endl;
    return 1;
  }

  std::cout << "Root has value " << line << " (" << std::stoi(line, nullptr, 2) << ")" << std::endl;
  BinaryTree tree(N, std::stoi(line, nullptr, 2));
  int value = 0;
  while (std::getline(ifile, line)) {
    //    std::cout << "------------------------------" << std::endl;
    //    std::cout << "Adding value " << line << " (" << std::stoi(line, nullptr, 2) << ")" <<
    //    std::endl;
    tree.addValue(std::stoi(line, nullptr, 2));
  }
  std::cout << "Tree has a total size of " << tree.size() << std::endl;

  int oxygen_rating = tree.search(&searchOxygenRating);
  int scrubber_rating = tree.search(&searchScrubberRating);
  int life_support_rating = oxygen_rating * scrubber_rating;
  auto t_end = std::chrono::steady_clock::now();

  std::cout << "********************************" << std::endl;
  std::cout << "Oxygen Rating is : " << oxygen_rating << std::endl;
  std::cout << "Scrubber Rating is : " << scrubber_rating << std::endl;
  std::cout << "Life support Rating is : " << life_support_rating << std::endl;
  std::cout << "Total operation took "
            << std::chrono::duration_cast<std::chrono::microseconds>(t_end - t_start).count()
            << " [us]" << std::endl;

  return 0;
}

namespace {

Node::Node(int value, std::size_t count /*= 1*/) : value{value}, counter{count} {
  //    std::cout << "Creating node for " << value << " (" << counter << ")" << std::endl;
}

void Node::addValue(int new_value, int level, std::size_t total) {
  // check if leaf
  if (num_left == 0 && num_right == 0) {
    if (new_value == value) {
      ++counter;
      return;
    } else {  // move content of current node to a child node
      if (goLeft(value, level)) {
        num_left = total - 1;
        left = std::make_unique<Node>(value, num_left);
      } else {
        num_right = total - 1;
        right = std::make_unique<Node>(value, num_right);
      }

      // reset internal content
      value = -1;
      counter = 0;
    }
  }

  // propagate new value
  if (goLeft(new_value, level)) {
    if (++num_left == 1) {  // first value to go down that path
      left = std::make_unique<Node>(new_value);
    } else {
      left->addValue(new_value, level - 1, num_left);
    }
  } else {
    if (++num_right == 1) {  // first value to go down that path
      right = std::make_unique<Node>(new_value);
    } else {
      right->addValue(new_value, level - 1, num_right);
    }
  }
}

BinaryTree::BinaryTree(int depth, int root_value) : depth_{depth}, counter_{1}, root_(root_value) {}

void BinaryTree::addValue(int value) { root_.addValue(value, depth_ - 1, ++counter_); }

int BinaryTree::search(std::function<int(const Node* const node)> selector) {
  return selector(&root_);
}

std::size_t BinaryTree::size() const { return counter_; }

int searchOxygenRating(const Node* const node) {
  if (node->value > -1) return node->value;

  if (node->num_left >= node->num_right) {
    return searchOxygenRating(node->left.get());
  } else {
    return searchOxygenRating(node->right.get());
  }
};

int searchScrubberRating(const Node* const node) {
  if (node->value > -1) return node->value;

  if (node->num_left >= node->num_right) {
    return searchScrubberRating(node->right.get());
  } else {
    return searchScrubberRating(node->left.get());
  }
};

}  // namespace
