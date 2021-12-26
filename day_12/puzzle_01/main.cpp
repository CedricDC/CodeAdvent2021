#include <cctype>
#include <chrono>
#include <cstddef>
#include <fstream>
#include <iostream>
#include <map>
#include <string>
#include <vector>

// Assumption : each connection is only listed once
//
// Given the fairly small number of nodes, a brute force approach may be quite fast
//
// It feels like some kind of backwards search should be faster, but is it?
//   Begin at nodes (denoted as N-1) connected to end, and find (and save) all paths from there to
//   end After that, step through neighbours of (N-1) and find all valid paths to N-1. Combine the
//   two to see if still valid
//
//   Intuitively, there is a problem with this approach that we will detect invalid paths much
//   later. In fact, each step is almost a full search again. So, simply doing this once from start
//   to end appears faster.
//   The core difference to the usual approach is that we are not looking for the shortest path,
//   so starting from the end is not a good approach here.
//
namespace {

constexpr bool PRINT_PATHS = false;  // debug flag

const std::string START_TOKEN = "start";
const std::string END_TOKEN = "end";

struct Cave {
  Cave(const std::string& name)
      : name_(name), is_small_(std::islower(name[0])), is_end_(name.compare(END_TOKEN) == 0) {}

  // explore network starting from this cave, return number of paths to end found
  std::size_t explore() {
    std::size_t paths_found = 0;

    if (is_end_) return paths_found + 1;  // found new path!

    if (is_small_ && visited_) {
      return paths_found;  // small cave, hit dead end
    }
    visited_ = true;

    for (auto* cave_ptr : connections_) paths_found += cave_ptr->explore();

    // returning to a path where we haven't visited this cave yet
    // incorrect for large caves, but irrelevant for them
    visited_ = false;

    return paths_found;
  }

  // debug version saving paths taken
  std::size_t explore(std::vector<std::vector<std::string>>& paths) {
    std::size_t paths_found = 0;

    if (is_end_) {
      paths.back().push_back(name_);
      paths.push_back(paths.back());
      paths.back().pop_back();
      return paths_found + 1;  // found new path!
    }

    if (is_small_ && visited_) {
      return paths_found;  // small cave, hit dead end
    }
    visited_ = true;
    paths.back().push_back(name_);

    for (auto* cave_ptr : connections_) paths_found += cave_ptr->explore(paths);

    // returning to a path where we haven't visited this cave yet
    // incorrect for large caves, but irrelevant for them
    visited_ = false;

    paths.back().pop_back();

    return paths_found;
  }

  void addConnection(Cave* cave) { connections_.push_back(cave); }

 private:
  const std::string name_;
  bool is_small_;
  bool visited_ = false;
  bool is_end_ = false;

  std::vector<Cave*> connections_;
};

struct CaveMap {
  void addConnection(const std::string& token1, const std::string& token2) {
    Cave& cave_1 = getCave_(token1);
    Cave& cave_2 = getCave_(token2);

    cave_1.addConnection(&cave_2);
    cave_2.addConnection(&cave_1);

    if (start_ == nullptr) {
      if (token1.compare(START_TOKEN) == 0) {
        start_ = &cave_1;
      } else if (token2.compare(START_TOKEN) == 0) {
        start_ = &cave_2;
      }
    }
  }

  std::size_t size() const { return cave_register_.size(); }

  std::size_t explorePaths() {
    std::size_t num_paths = 0;

    if constexpr (PRINT_PATHS) {
      std::vector<std::vector<std::string>> paths(1);
      num_paths = getCave_(START_TOKEN).explore(paths);
      printPaths_(paths);
    } else {
      num_paths = getCave_(START_TOKEN).explore();
    }

    return num_paths;
  }

 private:
  // cave register owns caves
  std::map<std::string, Cave> cave_register_;

  Cave* start_ = nullptr;

  // Find id of existing cave, else create it and return id
  Cave& getCave_(const std::string& token) {
    // try_emplace returns std::pair<iterator (to std::pair), bool>
    return cave_register_.try_emplace(token, token).first->second;
  }

  void printPaths_(const std::vector<std::vector<std::string>>& paths) {
    std::cout << "Paths found: " << std::endl;
    for (const auto& path : paths) {
      if (path.empty()) continue;

      auto it = path.cbegin();
      std::cout << *(it++);
      while (it != path.cend()) {
        std::cout << " --> " << *(it++);
      }
      std::cout << std::endl;
    }
  }
};  // namespace

bool readTokens(std::ifstream& ifile, std::string& token1, std::string& token2) {
  if (!ifile.good()) return false;

  std::string line;
  std::getline(ifile, line);

  auto dash = line.find('-');
  token1.assign(line, 0, dash);
  token2.assign(line, dash + 1, std::string::npos);
  return true;
}

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

  CaveMap cave_map;

  // first, parse file
  std::string token1;
  std::string token2;
  while (readTokens(ifile, token1, token2)) {
    cave_map.addConnection(token1, token2);
  }

  std::cout << "Added " << cave_map.size() << " caves" << std::endl;
  std::size_t num_routes = cave_map.explorePaths();

  auto t_end = std::chrono::steady_clock::now();

  std::cout << "Total number of routes: " << num_routes << std::endl;
  std::cout << "Execution took "
            << std::chrono::duration_cast<std::chrono::microseconds>(t_end - t_start).count()
            << " us" << std::endl;

  return 0;
}
