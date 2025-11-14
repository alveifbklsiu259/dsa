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
  std::vector<int> v = {1, 2, 1, 3, 56, 7, 8, 9, 54, 1, 23, 5, 9, 54, 6, 12};
  sort::selectionSort(v.begin(), v.end(), std::less{});
  for (int n : v) std::cout << n << '\n';
  return 0;
}
