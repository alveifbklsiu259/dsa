#include "./algorithm/sort/sort.hpp"
#include "data_structure/array/dynamic_array.hpp"
#include "data_structure/hash_map/hash_map.hpp"
#include "data_structure/hash_set/hash_set.hpp"
#include <array>
#include <cmath>
#include <cstddef>
#include <forward_list>
#include <queue>
#include <stack>
#include <unordered_map>
#include <unordered_set>

int main() {
  array::DynamicArray<int> d1{1, 2, 3, 4, 5};
  array::DynamicArray<int> d2 = d1;
  d2[0] = 6;

  array::DynamicArray<int> d3;
  d3 = d2;

  for (int n : d3) std::cout << n << '\n';

  return 0;
}
