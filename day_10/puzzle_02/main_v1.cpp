#include <algorithm>
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

constexpr unsigned int getCompletionScore(char c);
std::size_t computeCompletionScore(const std::vector<CharCounter>& stack);

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
  std::vector<std::size_t> completion_scores;

  char c;
  while ((c = ifile.get(), ifile.good())) {
    if (isOpeningChar(c)) {
      if (c_stack.empty() || c_stack.back().opening_char != c) {
        c_stack.emplace_back(c);
      } else {
        ++c_stack.back().counter;
      }
    } else if (isEol(c)) {
      if (!c_stack.empty()) {  // line was incomplete
        completion_scores.push_back(computeCompletionScore(c_stack));
      }
      c_stack.clear();
    } else {  // is closing character (assume input valid)
      if (c_stack.empty() || c_stack.back().closing_char != c) {
        // corrupted line found, clear stack and discard rest of line
        c_stack.clear();
        ifile.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
      } else {  // found correct closing character
        if (--c_stack.back().counter == 0) {
          c_stack.pop_back();
        }
      }
    }
  }

  // last line does not have newline character: check if it was incomplete
  if (!c_stack.empty()) {
    completion_scores.push_back(computeCompletionScore(c_stack));
  }

  // assume there is at least one completion_score
  auto middle_it = completion_scores.begin() + completion_scores.size() / 2;
  std::nth_element(completion_scores.begin(), middle_it, completion_scores.end());
  std::size_t middle_score = *middle_it;
  auto t_end = std::chrono::steady_clock::now();

  std::cout << "Final score from completion " << middle_score << std::endl;
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

constexpr unsigned int getCompletionScore(char c) {
  switch (c) {
    case ')':
      return 1;
    case ']':
      return 2;
    case '}':
      return 3;
    case '>':
      return 4;
  }

  return 0;
}

std::size_t computeCompletionScore(const std::vector<CharCounter>& stack) {
  std::size_t score = 0;
  //  std::cout << "Computing score for ";
  for (auto crit = stack.crbegin(); crit != stack.crend(); ++crit) {
    unsigned int score_step = getCompletionScore(crit->closing_char);
    for (unsigned int i = 0; i < crit->counter; ++i) {
      //      std::cout << crit->closing_char;
      score = (5 * score + score_step);
    }
  }
  //  std::cout << ": score = " << score << std::endl;

  return score;
}

}  // namespace
