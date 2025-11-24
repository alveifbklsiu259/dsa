#include "./algorithm/sort/sort.hpp"
#include "data_structure/array/dynamic_array.hpp"
#include "data_structure/hash_map/hash_map.hpp"
#include "data_structure/hash_set/hash_set.hpp"
#include "data_structure/queue/queue.hpp"
#include <array>
#include <cmath>
#include <cstddef>
#include <forward_list>
#include <queue>
#include <stack>
#include <unordered_map>
#include <unordered_set>
int main() {
  array::DynamicArray<int> v{1, 2, 3, 4, 5, 6, 7};

  // v.erase(v.begin() + 2);
  auto a = v.begin() + 1;
  // auto b = v.cbegin() + 1;
  // array::DynamicArray<int>::DynamicArrayIterator<true> c = v.begin();

  // for (int n : v) std::cout << n << '\n';

  return 0;
}
