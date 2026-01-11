#include "./algorithm/sort/sort.hpp"
#include "data_structure/array/dynamic_array.hpp"
#include "data_structure/hash_map/hash_map.hpp"
#include "data_structure/hash_set/hash_set.hpp"
#include "data_structure/queue/deque.hpp"
#include "data_structure/queue/dynamic_queue.hpp"
#include "data_structure/stack/dynamic_stack.hpp"
#include <cmath>
#include <cstddef>
#include <deque>
#include <iostream>
#include <queue>
#include <ranges>
#include <stack>
#include <unordered_map>
#include <unordered_set>

int main() {
  std::deque<int> dq{1, 2, 3, 4, 5};
  queue::Deque<int> d{1, 2, 3, 4, 5};
  queue::Deque<int> d2 = d;
  for (size_t i = 0; i < 43; ++i) d2.emplaceBack(i + 100);
  d2.pushBack(100);
  // d.insert(d.begin(), {1, 2, 3, 4, 5});

  d2.emplace(d2.end() - 2, 3000);
  d2.printBlocks();
  d2.printInfo();
  d2.printBlock({0, 1, 2, 3, 4, 7});
  auto a = d2.emplace(d2.begin(), 123);

  // TODO:
  // - implement reverse iterator for dynamic array
  return 0;
}
