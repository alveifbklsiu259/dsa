#include "data_structure/array/dynamic_array.hpp"
#include "data_structure/array/static_array.hpp"
#include "data_structure/hash_map/hash_map.hpp"
#include "data_structure/queue/deque.hpp"
#include "data_structure/tree/binary_search_tree.hpp"
#include "data_structure/tree/binary_tree.hpp"
#include "data_structure/tree/node.hpp"
#include "data_structure/tree/tree-visualizer.hpp"
import trie;
#include <cassert>
#include <cstddef>
#include <functional>
#include <iostream>
#include <memory_resource>
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
// void bar(tree::Node<Task>& node) { std::cout << node.value() << ' '; };

static_assert([] {
  // hashmap::HashMap<int, int> map;

  return true;
}());

struct Tracker : public std::pmr::memory_resource {
  size_t count = 0;
  void* do_allocate(size_t b, size_t a) override {
    count++;
    return std::malloc(b);
  }
  void do_deallocate(void* p, size_t, size_t) override { std::free(p); }
  bool do_is_equal(const memory_resource& o) const noexcept override { return &o == this; }
};

struct DetailedTracker : public std::pmr::memory_resource {
public:
  std::pmr::memory_resource* upstream;
  size_t allocation_count = 0;
  size_t total_bytes = 0;
  explicit DetailedTracker(std::pmr::memory_resource* res) : upstream(res) {}

  void* do_allocate(size_t bytes, size_t alignment) override {
    total_bytes += bytes;
    allocation_count++;
    return upstream->allocate(bytes, alignment);
  }

  void do_deallocate(void* p, size_t bytes, size_t alignment) override {
    upstream->deallocate(p, bytes, alignment);
  }

  bool do_is_equal(const memory_resource& o) const noexcept override { return this == &o; }
};

void test_allocator_injection() {
  Tracker track;
  auto* old_resource = std::pmr::set_default_resource(&track);

  std::pmr::monotonic_buffer_resource pool{1024};
  DetailedTracker dt(&pool);
  std::pmr::polymorphic_allocator<> alloc{&dt};

  tree::Trie<std::string> trie(alloc);
  trie.insert("a");
  trie.insert("apple");
  trie.insert("banana");
  trie.insert("application");
  trie.insert("lemon");
  trie.insert("durian");
  std::cout << "default tracker count: " << track.count << '\n';

  std::cout << "Exact bytes used by Trie: " << dt.total_bytes << " bytes\n";
  std::cout << "Number of small allocations: " << dt.allocation_count << "\n";
  // assert(s->get_allocator().resource() == &pool);

  // try {
  //   std::pmr::monotonic_buffer_resource pool{1024};
  //   std::pmr::polymorphic_allocator<> alloc{&pool};
  //
  //   auto* ptr = alloc.allocate_object<std::pmr::string>();
  //
  //   try {
  //     // 2. Explicitly construct the string, PASSING THE ALLOCATOR
  //     // This ensures it doesn't look at the (null) default resource.
  //     std::construct_at(ptr, "foo", alloc);
  //   } catch (...) {
  //     alloc.deallocate_object(ptr);
  //     throw;
  //   }
  //
  //   // Verify: The string's internal resource should match your pool
  //   assert(ptr->get_allocator().resource() == &pool);
  //
  //   // Cleanup
  //   std::destroy_at(ptr);
  //   alloc.deallocate_object(ptr);
  //
  // } catch (const std::bad_alloc& e) {
  //   assert(false && "Injection failed! Node tried to use default allocator.");
  // }

  std::pmr::set_default_resource(old_resource);
}

int main() {
  // char buffer[4096];
  // std::pmr::monotonic_buffer_resource pool{buffer, sizeof(buffer)};
  //
  // // 2. Pass the resource pointer to the constructor
  // // The TrieNode constructor takes 'allocator_type', which accepts a resource pointer.
  // tree::TrieNode<char> rootNode(std::hash<char>{}, std::equal_to<char>{}, &pool);
  //
  // // ************************
  //
  // std::pmr::monotonic_buffer_resource heap_pool{1024 * 1024}; // 1MB from heap
  //
  // tree::TrieNode<char> rootNode2({}, {}, &heap_pool);
  //
  // // ***********************
  // char buffer2[4096];
  // std::pmr::monotonic_buffer_resource pool2{buffer, sizeof(buffer2)};
  // std::pmr::polymorphic_allocator<std::byte> alloc(&pool2);
  // auto* rootNode3 = alloc.new_object<tree::TrieNode<char>>();

  // allocator_traits
  // allocate, construct, deallocate, destroy

  // polymorphic_allocator
  // allocate, construct, deallocate, destroy, new_object, delete_object

  // std::construct_at
  // std::destroy_at
  tree::Trie<std::string> trie;
  trie.insert("apple");
  trie.insert("app");
  trie.insert("application");
  trie.insert("applicant");
  for (auto it = trie.begin(); it != trie.end(); ++it) {
    auto a = *it;
    std::cout << a << '\n';
  }

  // tree::Trie<std::vector<int>> t2;
  // t2.insert({1});
  // t2.insert({1, 2});
  // t2.insert({1, 2, 3});
  //
  // for (auto it = t2.begin(); it != t2.end(); ++it) {
  //   auto a = *it;
  //   for (auto e : a) { std::cout << e << ' '; }
  //   std::cout << '\n';
  // }

  // // array::DynamicArray<std::string> matches = trie.getAllWithPrefix("app");
  // // for (std::string& match : matches) { std::cout << match << '\n'; }
  //
  // tree::Trie<std::vector<std::string>> t2;
  // t2.insert({"1", "2"});
  // t2.insert({"1", "2", "3"});
  // t2.insert({"1", "2", "6"});
  // array::DynamicArray<std::vector<std::string>> matches = t2.getAllWithPrefix({"1"});
  // for (auto& match : matches) {
  //   for (auto e : match) { std::cout << e; }
  //   std::cout << '\n';
  // }
  // tree::Trie<std::vector<int>> t2;
  // t2.insert({1, 2, 3, 4});
  // t2.insert({1, 2, 3, 4, 6});
  // t2.insert({1, 2, 3, 4, 7});
  // array::DynamicArray<std::vector<int>> matches = t2.getAllWithPrefix({1});
  // for (auto& match : matches) {
  //   for (auto e : match) { std::cout << e; }
  //   std::cout << '\n';
  // }

  // constexpr std::vector<int> v = {1,2,3};
  constexpr array::StaticArray<int, 2> sa{1, 2};
  // constexpr tree::TrieNode<char>
  // node; // │ ├╴  Constexpr variable cannot have non-literal type 'const
  // tree::TrieNode<char>' clang (constexpr_var_non_literal) [75, 34]
  // constexpr tree::TrieNode<char> node2(); // no error, why?
  // constexpr tree::Trie<std::string> trie;    // error
  // constexpr tree::Trie<std::string> trie2(); // works , why?
  constexpr tree::BinaryTree<int> tree;

  array::DynamicArray<int> preorder{9, 9, 20, 15, 7};  // root -> left -> right
  array::DynamicArray<int> inorder{9, 9, 15, 20, 7};   // left -> root -> right
  array::DynamicArray<int> postorder{9, 15, 7, 20, 9}; // left -> right -> root
  array::DynamicArray<int> levelorder{9, 9, 20, 15, 7};
  // std::cout << node.secret << '\n';
  // std::cout << node.children[1].secret << '\n';
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
  // t.fromArrayRepresentation({1, null, 1, null, null, -4, null, null, null, null, null, 5, 6, null, null});
  // t.fromArrayRepresentation({40, 20, 60, 10, 30, 50, 70, null, null, 25, null, 30, 30, 30});
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
