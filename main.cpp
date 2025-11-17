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
  // std::vector<int> res = countingSort(v);
  // for (int n : res) std::cout << n << '\n';
  sort::countingSort(v.begin(), v.end(), sort::Order::Ascending);
  for (int n : v) std::cout << n << '\n';
  return 0;
}
