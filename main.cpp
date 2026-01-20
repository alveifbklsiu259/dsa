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
  // std::vector<int> v;
  // test same priority, can we ensure that the first one appears before the later ones if priority are the
  // sane?
  queue::PriorityQueue<Task, array::DynamicArray<Task>, TaskCompare> pq;
  // queue::PriorityQueue<Task, std::vector<Task>, TaskCompare> pq;
  pq.push({"Write report", 1});
  pq.push({"Fix bug", 2});
  pq.push({"Team meeting", 3});
  pq.push({"Code review", 4});

  queue::PriorityQueue<Task, array::DynamicArray<Task>, TaskCompare> pq2;
  // queue::PriorityQueue<Task, std::vector<Task>, TaskCompare> pq;
  pq2.push({"Write report", 10});
  pq2.push({"Fix bug", 11});
  pq2.push({"Team meeting", 12});
  pq2.push({"Code review", 13});

  pq.swap(pq2);

  // std::cout << "Top task: " << pq.top() << "\n";
  while (!pq.empty()) {
    std::cout << "Popped: " << pq.top() << "\n";
    pq.pop();
  }

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
