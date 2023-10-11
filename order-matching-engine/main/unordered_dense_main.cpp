#include <iostream>
#include "ankerl/unordered_dense.h"

// Sanity check for including unordered_dense
int main() {
  auto map = ankerl::unordered_dense::map<int, int>();
  map[123] = 123;

  for (auto const& [key, val] : map) {
    std::cout << key << " => " << val << std::endl;
  }
  return 0;
}