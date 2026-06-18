#include "data_structure/array/dynamic_array.hpp"
#include "data_structure/array/static_array.hpp"
#include "data_structure/hash_map/hash_map.hpp"
#include "data_structure/hash_set/hash_set.hpp"
#include "data_structure/queue/deque.hpp"
#include "data_structure/tree/binary_search_tree.hpp"
#include "data_structure/tree/binary_tree.hpp"
#include "data_structure/tree/node.hpp"
#include "data_structure/tree/tree-visualizer.hpp"
import trie;
import graph;
#include <cassert>
#include <cstddef>
#include <functional>
#include <iostream>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

class Task {
public:
  std::string name;
  int priority;

  // For debugging
  friend std::ostream& operator<<(std::ostream& os, const Task& t) {
    return os << "[" << t.name << ", priority=" << t.priority << "]";
  }

  Task operator+(const Task& other) const { return {name + other.name, priority + other.priority}; }

  bool operator==(const Task&) const = default;
  // Task() = default;
  Task(std::string s, int priority) : name(std::move(s)), priority(priority) {}
  std::string toString() { return ""; }
};

// Task operator+(const Task& lhs, const Task& rhs) {
//   return Task(lhs.name + rhs.name, lhs.priority + rhs.priority);
// }
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
// void foo(tree::Node<int>& node) { std::cout << node.value() << ' '; };
// void baz(tree::Node<int>& node, size_t level) { std::cout << node.value() << ' '; };
// void bar(tree::Node<Task>& node) { std*::cout << node.value() << ' '; };
static_assert([] { return true; }());
int main() {

  // graph::UndirectedGraph<std::string> myGraph;
  //
  // graph::Node<std::string>* taipei = myGraph.addVertex("Taipei");
  // auto* tokyo = myGraph.addVertex("Tokyo");
  // auto* seoul = myGraph.addVertex("Seoul");
  //
  // // Connect them
  // myGraph.addEdge(taipei, tokyo);
  // myGraph.addEdge(tokyo, seoul);
  // // myGraph.removeEdge(taipei, tokyo);
  // // myGraph.removeEdge(taipei, seoul);
  // // myGraph.removeVertex(tokyo);
  // // See the result
  // graph::UndirectedGraph<std::string> g2 = myGraph;
  // g2 = myGraph;
  // g2.printGraph();
  // std::cout << g2.edgeCount() << '\n';
  //
  // graph::DirectedGraph<std::string> dg;
  // auto* newYork = dg.addVertex("NewYork");
  // auto* hawaii = dg.addVertex("Hawaii");
  // dg.addEdge(newYork, hawaii);
  // dg.printGraph();
  // std::cout << dg.edgeCount() << '\n';
  //
  // std::cout << "degree: " << myGraph.degree(taipei) << '\n';
  //------------------------------------------

  // 1. Create a Directed Graph for strings
  // graph::DirectedGraph<std::string> clothingGraph;
  //
  // // 2. Add all the vertex tasks to the graph
  // auto* underwear = clothingGraph.addVertex("Underwear");
  // auto* pants = clothingGraph.addVertex("Pants");
  // auto* socks = clothingGraph.addVertex("Socks");
  // auto* shoes = clothingGraph.addVertex("Shoes");
  // auto* shirt = clothingGraph.addVertex("Shirt");
  // auto* tie = clothingGraph.addVertex("Tie");
  // auto* jacket = clothingGraph.addVertex("Jacket");
  //
  // // 3. Establish dependency rules (Edges)
  // // Underwear -> Pants -> Shoes
  // clothingGraph.addEdge(underwear, pants);
  // clothingGraph.addEdge(pants, shoes);
  //
  // // Socks -> Shoes
  // clothingGraph.addEdge(socks, shoes);
  //
  // // Shirt -> Tie -> Jacket
  // clothingGraph.addEdge(shirt, tie);
  // clothingGraph.addEdge(tie, jacket);
  graph::DirectedGraph<std::string> clothingGraph{
      {"Underwear", "Pants"}, {"Pants", "Shoes"}, {"Socks", "Shoes"}, {"Shirt", "Tie"}, {"Tie", "Jacket"}
  };

  // 2. Define the dependency rules (Edges) as a collection of pairs
  // std::vector<std::pair<std::string, std::string>> clothingEdges = {
  //     {"Underwear", "Pants"}, {"Pants", "Shoes"}, {"Socks", "Shoes"}, {"Shirt", "Tie"}, {"Tie",
  //     "Jacket"}
  // };

  // 3. Construct the entire topology in a single, clean call
  // clothingGraph.fromEdges(clothingEdges);

  auto allOrders = graph::utils::allTopologicalOrders(clothingGraph);
  auto order = graph::utils::topologicalSort(clothingGraph);
  std::cout << order.second << '\n';
  // for (auto& order : allOrders) {
  //   for (const auto* e : order) { std::cout << e->val() << ' '; }
  //   std::cout << '\n';
  // }
  clothingGraph.printGraph();
  // std::cout << std::boolalpha << clothingGraph.hasCycle() << '\n';
  for (auto& order : allOrders) std::cout << order << '\n';
  //------------------------------------------
  // hashset::HashSet<int> hs;
  // hs.reserve(12);
  // hs.emplace(1);
  // hs.emplace(2);
  // hs.emplace(300);
  // hs.emplace(4);
  // hs.emplace(5);
  // hs.insert(6);
  // for (auto i : hs) { std::cout << i << '\n'; }

  std::unordered_map<int, int> mm;
  array::DynamicArray<int> preorder{9, 9, 20, 15, 7};  // root -> left -> right
  array::DynamicArray<int> inorder{9, 9, 15, 20, 7};   // left -> root -> right
  array::DynamicArray<int> postorder{9, 15, 7, 20, 9}; // left -> right -> root
  array::DynamicArray<int> levelorder{9, 9, 20, 15, 7};
  // array::StaticArray<int, 2> sa{1, 2};
  // array::StaticArray ssa = {1,2,3};
  // array::StaticArray ssa2{1,2,3};
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
  std::vector<int> v{1, 2, 4, 5, 6};
  array::DynamicArray<std::optional<int>> v2{1, 2, 3, 4, 5, 6};
  tree::BinaryTree<int> bt{2, 2, 3, 4, 4, 1, 2, 6, 8, 1, 1, 3, 2, 7, 3};
  // tree::BinarySearchTree<int> t = bt.toBinarySearchTree();
  // tree::BinarySearchTree<int> t = bt.toBinarySearchTree();
  // t.eraseAll(2);
  // t.eraseAll(3);
  // tree::BinaryTree<int> t{v.begin(), v.end()};
  // tree::BinarySearchTree<int> t;
  // const auto& null = std::nullopt;
  // t.fromArrayRepresentation({1, null, 1, null, null, -4, null, null, null, null, null, 5, 6, null,
  // null}); t.fromArrayRepresentation({40, 20, 60, 10, 30, 50, 70, null, null, 25, null, 30, 30, 30});
  // t.fromArrayRepresentation({40, 20, 60, 10, 30, 50, 70});
  // t.insert(1);
  // t.fromValues({1, 3, 4, 5, 6, 7, 8});
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
  // std::cout << "\ninorder (left -> root -> right): \n";
  // t.inorderTraverse(foo);
  // std::cout << "\ninorder reverse (right -> root -> left): \n";
  // t.inorderReverseTraverse(foo);
  // std::cout << "\npreorder (root -> left -> right): \n";
  // t.preorderTraverse(foo);
  // std::cout << "\npostorder (left -> right -> root): \n";
  // t.postorderTraverse(foo);
  // std::cout << "\nlevelorder: \n";
  // t.levelorderTraverse(baz);
  // tree::BinaryTree<Task, TaskHasher, TaskEqual> bt1{Task{"a", 1}, Task{"b", 2}, Task{"c", 3}};
  // tree::BinaryTree<Task, TaskHasher, TaskEqual> bt2;
  // bt1.merge(bt2);
  //
  // tree::TreeVisualizer tv;
  // tv.visualize(bt, true);
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
  // - re-implement linked list
  return 0;
}
