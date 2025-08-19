#include "./DataStructure/DynamicArray.hpp"
#include "./DataStructure/StaticArray.hpp"
#include <array>
#include <iostream>
#include <string>
#include <vector>

int main() {
  DynamicArray d = {1, 2, 3};
  d.push_back(4);

  for (const int i : d) {
    std::cout << i << std::endl;
  }

  return 0;
};
