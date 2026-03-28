#include "./algorithm/sort/sort.hpp"
#include "data_structure/array/dynamic_array.hpp"
#include "data_structure/hash_map/hash_map.hpp"
#include "data_structure/hash_set/hash_set.hpp"
#include "data_structure/queue/deque.hpp"
#include "data_structure/queue/dynamic_queue.hpp"
#include "data_structure/queue/priority_queue.hpp"
#include "data_structure/queue/static_queue.hpp"
#include "data_structure/stack/dynamic_stack.hpp"
#include "data_structure/tree/binary_search_tree.hpp"
#include "data_structure/tree/binary_tree.hpp"
#include "data_structure/tree/node.hpp"
#include "data_structure/tree/tree-visualizer.hpp"
#include <algorithm>
#include <array>
#include <cassert>
#include <cmath>
#include <cstddef>
#include <deque>
#include <forward_list>
#include <functional>
#include <iostream>
#include <list>
#include <numeric>
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
  std::string toString() { return ""; }
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

void foo(tree::Node<int>& node) { std::cout << node.value() << ' '; };
void baz(tree::Node<int>& node, size_t level) { std::cout << node.value() << ' '; };
void bar(tree::Node<Task>& node) { std::cout << node.value() << ' '; };

int main() {
  array::DynamicArray<int> preorder{9, 9, 20, 15, 7};  // root -> left -> right
  array::DynamicArray<int> inorder{9, 9, 15, 20, 7};   // left -> root -> right
  array::DynamicArray<int> postorder{9, 15, 7, 20, 9}; // left -> right -> root
  array::DynamicArray<int> levelorder{9, 9, 20, 15, 7};
  std::vector<int> s;
  //   9
  //  / \
  // 9   20
  //    /  \
  //   15   7
  //
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
  tree::BinarySearchTree<int> t;
  // tree::BinarySearchTree<int> t;
  const auto& null = std::nullopt;
  // t.fromArrayRepresentation({1, null, 1, null, null, -4, null, null, null, null, null, 5, 6, null, null});
  // t.fromArrayRepresentation({40, 20, 60, 10, 30, 50, 70, null, null, 25, null, 30, 30, 30});
  // t.fromArrayRepresentation({40, 20, 60, 10, 30, 50, 70});
  // t.insert(1);
  t.fromValues({1, 3, 4, 5, 6, 7, 8});
  // t.insert(1);
  // t.insert(1);
  // t.insert(1);
  // t.insert(1);
  // t.insert(1);
  // t.insert(1);
  // t.fromInPre({1, 2}, {2, 1});
  // const tree::BinaryTree<int> t2{t};
  // auto* a = t2.findFirst(10);
  // auto* b = t2.findFirst(30);
  // auto* c = t2.lowestCommonAncestor(a, b);
  // std::cout << c->value() << '\n';
  // auto* found = t.findFirst(30);
  // t.eraseFirst(20);
  // t.eraseNode(found);
  // t.eraseAll(30);
  // const tree::BinarySearchTree<int> t2{t};
  // std::cout << std::boolalpha << (found != nullptr) << "\n";
  // t.eraseNode(found);
  std::cout << "\ninorder (left -> root -> right): \n";
  t.inorderTraverse(foo);
  std::cout << "\ninorder reverse (right -> root -> left): \n";
  t.inorderReverseTraverse(foo);
  std::cout << "\npreorder (root -> left -> right): \n";
  t.preorderTraverse(foo);
  std::cout << "\npostorder (left -> right -> root): \n";
  t.postorderTraverse(foo);
  std::cout << "\nlevelorder: \n";
  t.levelorderTraverse(baz);
  tree::TreeVisualizer tv;
  tv.visualize(t, true);
  // std::cout << std::boolalpha << t.validBST() << '\n';

  // std::cout << '\n';
  // auto found = t.findFirst(20);
  // std::cout << found->value() << '\n';

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
