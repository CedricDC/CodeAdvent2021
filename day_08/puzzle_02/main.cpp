#include <bitset>
#include <chrono>
#include <cstddef>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

/*
 * Trying to optimize for time, we will assign each number a bit, e.g. eafb --> 00110011
 *
 * This allows to dynamically create and compare masks without having to deal with characters
 *
 * There are several ways to deduct the types. Since we always get a complete set of all
 * values 0-9, we can e.g.
 *
 * 1. Detect trivial values 1, 4, 7, 8
 * 2. iff x has 5 bits and popcnt(1 & x == 2), then x = 3
 * 3. (8 xor 3) gives left bar : iff x has 6 bits and popcnt((8^3)&x) == 1, then x == 9
 * 4. For x with 6 bits, if popcnt(1 & x) == 2, then x == 0, else x == 6
 * 5. For x with 5 bits, if popcnt(6 & x) == 4, then x == 2, else x == 5
 *
 */

namespace {

struct Numbers {
  uint8_t zero;
  uint8_t one;
  uint8_t two;
  uint8_t three;
  uint8_t four;
  uint8_t five;
  uint8_t six;
  uint8_t seven;
  uint8_t eight;
  uint8_t nine;

  [[maybe_unused]] void print() const;
};

// read single number from file
uint8_t readValue(std::ifstream& ifile);

// read the 10 unique numbers and initialize Numbers structure
Numbers readValues(std::ifstream& ifile);

// Identify which number the value matches
int identifyValue(const Numbers& numbers, int value);

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
  std::size_t total_counter = 0;

  while (ifile.good()) {
    // deduce all possible values
    Numbers numbers = readValues(ifile);

    // 0. done above: get 1, 4, 7, 8
    // 1. find 3
    if (__builtin_popcount(numbers.one & numbers.three) != 2) {
      if (__builtin_popcount(numbers.one & numbers.two) == 2) {
        std::swap(numbers.two, numbers.three);  // two was actually three
      } else {
        std::swap(numbers.five, numbers.three);  // five was actually three
      }
    }

    // 2. find 9
    uint8_t left_bar = numbers.eight ^ numbers.three;
    if (__builtin_popcount(left_bar & numbers.nine) != 1) {
      if (__builtin_popcount(left_bar & numbers.zero) == 1) {
        std::swap(numbers.zero, numbers.nine);  // zero was actually nine
      } else {
        std::swap(numbers.six, numbers.nine);  // six was actually nine
      }
    }

    // 3. find 0 / 6
    if (__builtin_popcount(numbers.one & numbers.zero) != 2) {
      std::swap(numbers.zero, numbers.six);
    }

    // 4. find 2 / 5
    if (__builtin_popcount(numbers.six & numbers.two) != 4) {
      std::swap(numbers.two, numbers.five);
    }

    // Skip | and space
    ifile.get();
    ifile.get();

    // identify the 4 query values
    int intermediate_value = 0;
    intermediate_value += 1000 * identifyValue(numbers, readValue(ifile));
    intermediate_value += 100 * identifyValue(numbers, readValue(ifile));
    intermediate_value += 10 * identifyValue(numbers, readValue(ifile));
    intermediate_value += identifyValue(numbers, readValue(ifile));

    total_counter += intermediate_value;
  }

  auto t_end = std::chrono::steady_clock::now();

  std::cout << "Got overall sum: " << total_counter << std::endl;
  std::cout << "Execution took "
            << std::chrono::duration_cast<std::chrono::microseconds>(t_end - t_start).count()
            << " us" << std::endl;

  return 0;
}

namespace {

void Numbers::print() const {
  std::cout << "0 is a " << std::bitset<8>(zero).to_string() << std::endl;
  std::cout << "1 is a " << std::bitset<8>(one).to_string() << std::endl;
  std::cout << "2 is a " << std::bitset<8>(two).to_string() << std::endl;
  std::cout << "3 is a " << std::bitset<8>(three).to_string() << std::endl;
  std::cout << "4 is a " << std::bitset<8>(four).to_string() << std::endl;
  std::cout << "5 is a " << std::bitset<8>(five).to_string() << std::endl;
  std::cout << "6 is a " << std::bitset<8>(six).to_string() << std::endl;
  std::cout << "7 is a " << std::bitset<8>(seven).to_string() << std::endl;
  std::cout << "8 is a " << std::bitset<8>(eight).to_string() << std::endl;
  std::cout << "9 is a " << std::bitset<8>(nine).to_string() << std::endl;
}

uint8_t readValue(std::ifstream& ifile) {
  uint8_t value = 0;
  bool valid_char = true;
  char c;
  do {  // read all lower case characters
    c = ifile.get();
    valid_char = (c >= 'a' && c <= 'z');
    if (valid_char) value |= (1 << (c - 'a'));
  } while (valid_char);

  //  std::cout << "Just read " << std::bitset<8>(value).to_string() << std::endl;

  return value;
}

Numbers readValues(std::ifstream& ifile) {
  // Write all values at their first possible occurrence, will fix later
  Numbers numbers;
  uint8_t num_fivebits = 0;
  uint8_t num_sixbits = 0;

  for (int i = 0; i < 10; ++i) {
    uint8_t value = readValue(ifile);
    int num_bits = __builtin_popcount(value);

    switch (num_bits) {
      case 2:
        numbers.one = value;
        break;
      case 3:
        numbers.seven = value;
        break;
      case 4:
        numbers.four = value;
        break;
      case 5: {
        switch (++num_fivebits) {
          case 1:
            numbers.two = value;
            break;
          case 2:
            numbers.three = value;
            break;
          case 3:
            numbers.five = value;
            break;
        }
      } break;
      case 6:
        switch (++num_sixbits) {
          case 1:
            numbers.zero = value;
            break;
          case 2:
            numbers.six = value;
            break;
          case 3:
            numbers.nine = value;
            break;
        }
        break;
      case 7:
        numbers.eight = value;
        break;
      default:
        std::cout << "You messed up" << std::endl;
    }
  }

  return numbers;
}

int identifyValue(const Numbers& numbers, int value) {
  int num_bits = __builtin_popcount(value);

  switch (num_bits) {
    case 2:
      return 1;
    case 3:
      return 7;
    case 4:
      return 4;
    case 5:
      if (value == numbers.two) return 2;
      if (value == numbers.three) return 3;
      if (value == numbers.five) return 5;
      break;
    case 6:
      if (value == numbers.zero) return 0;
      if (value == numbers.six) return 6;
      if (value == numbers.nine) return 9;
      break;
    case 7:
      return 8;
    default:
      std::cout << "You messed up" << std::endl;
  }

  return 0;
}

}  // namespace
