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
  int ans[1] = {1};
  std::vector<int> v;
  queue::Queue<int, 5> q;
  q.push(1);
  q.push(2);
  q.push(3);
  q.push(4);
  q.push(5);
  std::cout << q.front() << '\n';
  std::cout << q.pop() << '\n';
  std::cout << q.pop() << '\n';
  std::cout << q.pop() << '\n';
  std::cout << q.pop() << '\n';
  return 0;
}
