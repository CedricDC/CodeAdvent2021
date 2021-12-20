#include <bitset>
#include <chrono>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <string_view>

namespace {

enum class Method { SIMPLE, CHARWISE, BITWISE };

constexpr bool isTrivial(int char_count) { return (char_count < 5 || char_count > 6); }

}  // namespace

int main(int argc, char** argv) {
  if (argc < 2) {
    std::cout << "Required input arguments: <filename>" << std::endl;
    return 1;
  }

  Method method = Method::CHARWISE;

  if (argc > 2) {
    method = static_cast<Method>(std::atoi(argv[2]));
  }

  std::string filename = argv[1];
  std::ifstream ifile(filename);

  if (!ifile.good()) {
    std::cout << "Could not open " << filename << std::endl;
    return 1;
  }

  auto t_start = std::chrono::steady_clock::now();
  std::size_t counter = 0;

  // simplest
  switch (method) {
    case Method::SIMPLE: {
      std::string line;
      while (std::getline(ifile, line)) {
        std::istringstream iss(line);

        // discard 10 words and |
        std::string word;
        for (int i = 0; i < 11; ++i) {
          iss >> word;
        }

        // now counter letters of the four query words
        for (int i = 0; i < 4; ++i) {
          iss >> word;
          if (isTrivial(word.size())) ++counter;
        }
      }
      std::cout << "Counter: " << counter << std::endl;
    } break;
    case Method::CHARWISE: {
      while (ifile.good()) {
        char c;

        // first, find "|"
        while ((c = ifile.get()) != '|') {
        }
        c = ifile.get();  // found |, read space

        for (int i = 0; i < 4; ++i) {
          int num_chars = 0;
          do {  // read all lower case characters
            c = ifile.get();
            ++num_chars;
          } while (c >= 97 && c <= 122);
          --num_chars;  // remove "invalid character"
          if (isTrivial(num_chars)) ++counter;
        }
      }
      std::cout << "Counter: " << counter << std::endl;
    } break;
    case Method::BITWISE: {
      while (ifile.good()) {
        char c;

        // first, find "|"
        while ((c = ifile.get()) != '|') {
        }
        c = ifile.get();  // found |, read space

        for (int i = 0; i < 4; ++i) {
          int value = 0;
          bool valid_char = true;
          do {  // read all lower case characters
            c = ifile.get();
            valid_char = (c >= 'a' && c <= 'z');
            if (valid_char) value |= (1 << (c - 'a'));
          } while (valid_char);

          // gcc specific instruction to count bits, requires -march compiler flag
          int num_chars = __builtin_popcount(value);
          if (isTrivial(num_chars)) ++counter;
        }
      }
      std::cout << "Counter: " << counter << std::endl;
    } break;
  }
  auto t_end = std::chrono::steady_clock::now();

  std::cout << "Got number of trivial words: " << counter << std::endl;
  std::cout << "Execution took "
            << std::chrono::duration_cast<std::chrono::microseconds>(t_end - t_start).count()
            << " us" << std::endl;

  return 0;
}
