#include "data_structure/hash_map/hash_map.hpp"
#include "data_structure/hash_set/hash_set.hpp"
#include <cmath>
#include <forward_list>
#include <queue>
#include <stack>
#include <unordered_map>
#include <unordered_set>

int main() {
  hashset::HashSet<int> s;
  s.insert(1);
  s.insert(2);
  s.insert(3);
  s.emplace(4);

  std::cout << std::boolalpha << s.contains(4) << '\n';

  for (int n : s) { std::cout << n << '\n'; }

  // std::unordered_set<int> s2{1, 2, 3, 4, 5};
  // auto a = s2.emplace(6);

  // -----------------
  // hashset::HashSet<int> set{13};
  // std::cout << std::boolalpha << set.contains(13) << '\n';
  // -----------------
  // hashmap::HashMap<std::string, int> s;
  // std::pair<std::string, int> data{"foo", 10};
  // std::pair<std::string, int> data2{"bar", 20};
  // std::pair<std::string, int> data3{"baz", 30};
  // s.insert(data);
  // s.insert(data2);
  // s.insert(data3);
  // std::cout << s.erase("foo") << '\n';
  // std::cout << std::boolalpha << s.contains("foo") << '\n';
  // std::cout << std::boolalpha << s.contains("foo") << '\n';
  // std::cout << s["baz"] << '\n';
  // std::cout << s.at("bar") << '\n';
  // -------------------------
  return 0;
}
