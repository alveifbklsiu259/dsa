#include "./algorithm/sort/sort.hpp"
#include "data_structure/array/dynamic_array.hpp"
#include "data_structure/hash_map/hash_map.hpp"
#include "data_structure/hash_set/hash_set.hpp"
#include "data_structure/queue/deque.hpp"
#include "data_structure/queue/dynamic_queue.hpp"
#include "data_structure/queue/priority_queue.hpp"
#include "data_structure/queue/static_queue.hpp"
#include "data_structure/stack/dynamic_stack.hpp"
#include "data_structure/tree/binary_tree.hpp"
#include "data_structure/tree/node.hpp"
#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstddef>
#include <deque>
#include <forward_list>
#include <functional>
#include <iostream>
#include <list>
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

  bool operator==(const Task&) const = default;
  // Task() = default;
  Task(std::string s, int priority) : name(std::move(s)), priority(priority) {}
};
struct TaskHasher {
  size_t operator()(const Task& t) const noexcept {
    size_t h1 = std::hash<std::string>{}(t.name);
    size_t h2 = std::hash<int>{}(t.priority);
    return h1 ^ (h2 << 1);
  }
};

struct TaskEqual {
  bool operator()(const Task& a, const Task& b) const noexcept { return a == b; }
};

std::unordered_map<Task, int, TaskHasher, TaskEqual> m;
struct TaskCompare {
  bool operator()(const Task& lhs, const Task& rhs) const {
    return lhs.priority < rhs.priority;
    // returns true if lhs < rhs → max heap
  }
};

void foo(tree::Node<int>& node) { std::cout << node.value << ' '; };
void bar(tree::Node<Task>& node) { std::cout << node.value << ' '; };
int main() {

  array::DynamicArray<int> preorder{9, 9, 20, 15, 7};  // root -> left -> right
  array::DynamicArray<int> inorder{9, 9, 15, 20, 7};   // left -> root -> right
  array::DynamicArray<int> postorder{9, 15, 7, 20, 9}; // left -> right -> root
  array::DynamicArray<int> levelorder{9, 9, 20, 15, 7};
  // array::DynamicArray<int> inorder{4, 2, 5, 1, 3};
  // array::DynamicArray<int> levelorder{1, 2, 3, 4, 5};
  // array::DynamicArray<int> inorder{2, 1, 3};
  // array::DynamicArray<int> levelorder{1, 2, 3};

  //   9
  //  / \
  // 9   20
  //    /  \
  //   15   7
  //
  // std::unordered_map<int, int>();
  // t.fromInPre(inorder, preorder);
  // t.fromInPost(inorder, postorder);
  // std::vector<std::optional<int>> seq = {
  //     1,         // root
  //     2, 3,      // level 1
  //     4, 5, 6, 7 // level 2
  // };
  // array::DynamicArray<std::optional<int>> seq = {
  //     1,         // root
  //     2, 3,      // level 1
  //     4, 5, 6, 7 // level 2
  // };
  //       1
  //    /     \
  //   2       3
  //  / \     / \
  // 4   5   6   7
  array::DynamicArray<std::optional<int>> seq = {
      1,         // root
      2, 3,      // level 1
      2, 3, 1, 7 // level 2
  };
  //       1
  //    /     \
  //   2       3
  //  / \     / \
  // 2   3   1   7

  // array::DynamicArray<std::optional<int>> seq = {
  //     10,                                // root
  //     20,           30,                  // level 1
  //     std::nullopt, 40, std::nullopt, 70 // level 2
  // };
  //      10
  //   /      \
  // 20        30
  //  \          \
  //  40          70

  // t.fromInLevel(inorder, {1, 9, 2});
  // std::vector<std::optional<int>> s{1, 2, 3};
  // array::DynamicArray<std::optional<int>> s{1, std::nullopt, 3};
  // t.fromCompleteTree({1, std::nullopt, 3, std::nullopt, std::nullopt, 4, 5});
  tree::BinaryTree<int> t;
  const auto& null = std::nullopt;
  // t.fromArrayRepresentation({1, null, 1, null, null, 4, null, null, null, null, null, 5, 6, null, null});
  t.fromInPre({2, 1}, {1, 2});
  t.insert(3);
  t.insert(4);
  t.insert(5);
  std::cout << "\ninorder (left -> root -> right): \n";
  t.inorderTraverse(foo);
  std::cout << "\ninorder reverse (right -> root -> left): \n";
  t.inorderReverseTraverse(foo);
  std::cout << "\npreorder (root -> left -> right): \n";
  t.preorderTraverse(foo);
  std::cout << "\npostorder (left -> right -> root): \n";
  t.postorderTraverse(foo);
  std::cout << "\nlevelorder: \n";
  t.levelorderTraverse(foo);
  std::cout << '\n';

  // auto* found = t.find(1);
  // if (found != nullptr) std::cout << "Found! the value is: " << found->value << '\n';

  // tree::BinaryTree<Task, TaskHasher, TaskEqual> t;
  // const auto& null = std::nullopt;
  // // t.fromArrayRepresentation({1, null, 3, null, null, 4, null, null, null, null, null, 5, 6, null,
  // null}); t.fromArrayRepresentation(
  //     {Task{"a", 1}, null, Task{"c", 3}, null, null, Task{"d", 4}, null, null, null, null, null, Task{"e",
  //     5},
  //      Task{"f", 6}, null, null}
  // );
  // std::cout << "\ninorder: \n";
  // t.inorderTraverse(bar);
  // std::cout << "\npreorder: \n";
  // t.preorderTraverse(bar);
  // std::cout << "\npostorder: \n";
  // t.postorderTraverse(bar);
  // std::cout << "\nlevelorder: \n";
  // t.levelorderTraverse(bar);
  // std::cout << '\n';
  //
  // auto* found = t.find(Task{"a", 1});
  // if (found != nullptr) std::cout << "Found! the value is: " << found->value << '\n';
  // std::cout << '\n';
  // std::cout << std::boolalpha << t.isHeightBalanced() << '\n';

  // TODO:
  // should we add
  // using value_type = T;
  // using size_type = size_t;
  // using reference = T&;
  // using const_reference = const T&;
  // to other containers like hashmap?
  // add constexpr to containers
  // add ==, != operators to containers (element-wise)
  return 0;
}
