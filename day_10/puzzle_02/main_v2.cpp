#include <algorithm>
#include <chrono>
#include <fstream>
#include <iostream>
#include <list>
#include <string>
#include <utility>
#include <vector>

namespace {

constexpr bool isOpeningChar(char c) { return (c == '(' || c == '[' || c == '{' || c == '<'); }
constexpr bool isClosingChar(char c) { return (c == ')' || c == ']' || c == '}' || c == '>'); }
constexpr bool isEol(char c) { return (c == '\n'); }

constexpr char getClosingChar(char c);

struct ChunkDelimiter {
  ChunkDelimiter(char opening_char)
      : opening_char(opening_char), closing_char(getClosingChar(opening_char)) {}
  char opening_char;
  char closing_char;
};

using CompletionStack = std::vector<ChunkDelimiter>;

constexpr unsigned int getCompletionScore(char c);
std::size_t computeCompletionScore(const CompletionStack& stack);

}  // namespace

// Pretty much the same as main.cpp, but trying to leverage the fact that
// the "constant level score" = 5 is higher than the higher character score.
// In other words, the middle score will have the middle number of completion
// characters, and we can skip computing the score for the other stacks.
// However, requires a bit of book keeping.
//
// Result : There appears a small gain, but barely noticeable, and frankly not
//          worth the cost in readability.
//
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
  std::list<CompletionStack> c_stacks;
  std::vector<std::size_t> completion_sizes;

  char c;
  CompletionStack* current_stack = &c_stacks.emplace_back();
  while ((c = ifile.get(), ifile.good())) {
    if (isOpeningChar(c)) {
      current_stack->emplace_back(c);
    } else if (isEol(c)) {
      if (!current_stack->empty()) {  // line was incomplete
        completion_sizes.emplace_back(current_stack->size());
        current_stack = &c_stacks.emplace_back();
      } else {
        current_stack->clear();
      }
    } else {  // is closing character (assume input valid)
      if (current_stack->empty() || current_stack->back().closing_char != c) {
        // corrupted line found, clear stack and discard rest of line
        current_stack->clear();

        ifile.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
      } else {  // found correct closing character
        current_stack->pop_back();
      }
    }
  }

  // last line does not have newline character: check if it was incomplete
  if (!current_stack->empty()) {
    completion_sizes.emplace_back(current_stack->size());
  }

  // Given that each character infers a "score" of 5 regardless of the type,
  // the middle score will be one of the lines with the middle number of characters
  auto middle_size_it = completion_sizes.begin() + completion_sizes.size() / 2;
  std::nth_element(completion_sizes.begin(), middle_size_it, completion_sizes.end());
  std::size_t size_of_interest = *middle_size_it;

  std::size_t small_counter = 0;
  std::vector<std::size_t> score_candidates;
  for (const auto& stack : c_stacks) {
    if (stack.size() < size_of_interest) {
      ++small_counter;
    } else if (stack.size() == size_of_interest) {
      score_candidates.push_back(computeCompletionScore(stack));
    }
  }

  if (score_candidates.empty()) {
    std::cout << "No matching score candidates found!" << std::endl;
    return 1;
  }

  // now, find the correct score by shifting the index by the number of smaller ones
  std::size_t corrected_index_of_interest = c_stacks.size() / 2 - small_counter;
  auto score_it = score_candidates.begin() + corrected_index_of_interest;
  std::nth_element(score_candidates.begin(), score_it, score_candidates.end());
  std::size_t middle_score = *score_it;
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

std::size_t computeCompletionScore(const CompletionStack& stack) {
  std::size_t score = 0;
  for (auto crit = stack.crbegin(); crit != stack.crend(); ++crit) {
    score = (5 * score + getCompletionScore(crit->closing_char));
  }

  return score;
}

}  // namespace
