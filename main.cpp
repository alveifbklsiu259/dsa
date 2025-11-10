#include "./algorithm/sort/sort.hpp"
#include "data_structure/hash_map/hash_map.hpp"
#include "data_structure/hash_set/hash_set.hpp"
#include "data_structure/stack/stack.hpp"
#include <array>
#include <cmath>
#include <cstddef>
#include <forward_list>
#include <queue>
#include <stack>
#include <unordered_map>
#include <unordered_set>
int main() {
  stack::Stack<int, 4> s;
  s.push(1);
  s.push(2);
  s.push(3);
  s.push(4);
  stack::Stack<int, 4> s2(s);

  std::cout << s2.top() << '\n';
  return 0;
}
