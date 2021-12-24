#include <chrono>
#include <fstream>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

namespace {

constexpr bool isOpeningChar(char c) { return (c == '(' || c == '[' || c == '{' || c == '<'); }
constexpr bool isClosingChar(char c) { return (c == ')' || c == ']' || c == '}' || c == '>'); }
constexpr bool isEol(char c) { return (c == '\n'); }

constexpr char getClosingChar(char c);

struct CharCounter {
  CharCounter(char opening_char)
      : opening_char(opening_char), closing_char(getClosingChar(opening_char)) {}
  char opening_char;
  char closing_char;
  unsigned int counter = 1;
};

[[maybe_unused]] void printStack(const std::vector<CharCounter>& stack);

constexpr unsigned int getCorruptingCharScore(char c);

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

  auto t_start = std::chrono::steady_clock::now();
  std::vector<CharCounter> c_stack;

  std::size_t corruption_score = 0;

  char c;
  while ((c = ifile.get(), ifile.good())) {
    if (isOpeningChar(c)) {
      if (c_stack.empty() || c_stack.back().opening_char != c) {
        c_stack.emplace_back(c);
      } else {
        ++c_stack.back().counter;
      }
    } else if (isEol(c)) {
      c_stack.clear();
    } else {  // is closing character (assume input valid)
      if (c_stack.empty() || c_stack.back().closing_char != c) {
        // corrupted line found!
        corruption_score += getCorruptingCharScore(c);

        // clear stack and discard rest of line
        c_stack.clear();
        ifile.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
      } else {  // found correct closing character
        if (--c_stack.back().counter == 0) {
          c_stack.pop_back();
        }
      }
    }
  }

  auto t_end = std::chrono::steady_clock::now();

  std::cout << "Final score from corrupting characters is " << corruption_score << std::endl;
  std::cout << "Execution took "
            << std::chrono::duration_cast<std::chrono::microseconds>(t_end - t_start).count()
            << " us" << std::endl;

  return 0;
}

namespace {

constexpr char getClosingChar(char c) {
  switch (c) {
    case '(':
      return ')';
    case '[':
      return ']';
    case '{':
      return '}';
    case '<':
      return '>';
    default:
      std::cout << "Invalid opening char unknown for " << c << std::endl;
  }

  return 0;
}

void printStack(const std::vector<CharCounter>& stack) {
  std::cout << "Current stack: " << std::endl;
  for (const auto& elem : stack) {
    std::cout << elem.opening_char << " (" << elem.counter << "); ";
  }
  std::cout << std::endl;
}

constexpr unsigned int getCorruptingCharScore(char c) {
  switch (c) {
    case ')':
      return 3;
    case ']':
      return 57;
    case '}':
      return 1197;
    case '>':
      return 25137;
    default:
      std::cout << "Value unknown for " << c << std::endl;
  }

  return 0;
}

}  // namespace
