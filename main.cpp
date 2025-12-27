#include "./algorithm/sort/sort.hpp"
#include "data_structure/array/dynamic_array.hpp"
#include "data_structure/hash_map/hash_map.hpp"
#include "data_structure/hash_set/hash_set.hpp"
#include "data_structure/queue/deque.hpp"
#include "data_structure/queue/dynamic_queue.hpp"
#include "data_structure/stack/dynamic_stack.hpp"
#include <cstddef>
#include <deque>
#include <iostream>
#include <queue>
#include <ranges>
#include <unordered_map>
#include <unordered_set>
int main() {
  std::queue<int> q;
  std::deque<int> dq;

  queue::Deque<int> d;
  for (size_t i = 0; i < 17; i++) { d.emplaceFront(i + 1); }
  // for (size_t i = 0; i < 16; i++, count--) { d.popBack(); }
  // d.i_printBlocks();
  // d.i_printInfo();
  // d.i_printBlock({0, 1, 2, 3, 4, 7});
  queue::Deque<int> d2;
  // test this:
  //
  //
  //
  // const queue::Deque<int> d2;
  // d2 = std::move(d)
  //
  //
  //
  d2 = std::move(d);
  // for (size_t i = 0; i < 12; i++) { d2.popFront(); }
  d2.i_printBlocks();
  d2.i_printInfo();
  d2.i_printBlock({0, 1, 2, 3, 4, 7});
  // auto a = d2.begin() + 1;
  // for (int& n : d2) { std::cout << n << '\n'; }
  return 0;
}
