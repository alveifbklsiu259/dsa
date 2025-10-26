#include "./data_structure/hash_map/hash_map.hpp"
#include <cmath>
#include <stack>
int main() {
  // maybe change the color of comment, it is the same as virtual text now
  // can we have that same indent for ruff? (closing parenthesis ends in a new line)
  //
  std::stack<int> st;

  hashmap::HashMap<std::string, int> s;
  std::pair<std::string, int> data{"foo", 10};
  std::pair<std::string, int> data2{"bar", 20};
  std::pair<std::string, int> data3{"baz", 30};
  s.insert(data);
  s.insert(data2);
  s.insert(data3);
  std::cout << std::boolalpha << s.contains("foo") << '\n';
  std::cout << s["baz"] << '\n';
  std::cout << s.at("bar") << '\n';
  return 0;
}
// - add emplace to other container
