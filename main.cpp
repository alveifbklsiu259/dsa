#include "./algorithm/sort/sort.hpp"
#include "data_structure/array/dynamic_array.hpp"
#include "data_structure/hash_map/hash_map.hpp"
#include "data_structure/hash_set/hash_set.hpp"
#include "data_structure/queue/deque.hpp"
#include "data_structure/queue/dynamic_queue.hpp"
#include "data_structure/queue/priority_queue.hpp"
#include "data_structure/queue/static_queue.hpp"
#include "data_structure/stack/dynamic_stack.hpp"
#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstddef>
#include <deque>
#include <iostream>
#include <queue>
#include <ranges>
#include <stack>
#include <unordered_map>
#include <unordered_set>
#include <utility>

class Task {
public:
  std::string name;
  int priority;

  // For debugging
  friend std::ostream& operator<<(std::ostream& os, const Task& t) {
    return os << "[" << t.name << ", priority=" << t.priority << "]";
  }

  // Task() = default;
  // Task(std::string  s, int priority): name(std::move(s)), priority(priority) {}
};

struct TaskCompare {
  bool operator()(const Task& lhs, const Task& rhs) const {
    return lhs.priority < rhs.priority;
    // returns true if lhs < rhs → max heap
  }
};
int main() {
  // array::DynamicArray<int> v2{1, 2, 0, 6, 3, 1, 7};
  array::DynamicArray<Task> v2{};
  v2.pushBack({.name = "Run errands", .priority = 2});
  v2.pushBack({.name = "Do the laundry", .priority = 1});
  v2.pushBack({.name = "Code review", .priority = 3});
  // sort::heapSort(v2.begin(), v2.end());

  sort::heapSort(v2.begin(), v2.end(), TaskCompare());
  for (auto& n : v2) std::cout << n << ' ';
  // TODO:
  // should we add
  // using value_type = T;
  // using size_type = size_t;
  // using reference = T&;
  // using const_reference = const T&;
  // to other containers like hashmap?
  // add constexpr
  return 0;
}
