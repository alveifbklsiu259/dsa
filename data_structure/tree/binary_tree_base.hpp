#pragma once
#include "../hash_map/hash_map.hpp"
#include "../queue/deque.hpp"
#include "./detail.hpp"
#include <gsl/gsl>

namespace tree {

template <typename T> class Node; // forward declaration, definition in node.hpp
namespace detail {
// TODO:
// when we extend BinaryTree from BinarySearchTree, we need to have some check or rewrite tree-constructing
// methods like fromInPre. or maybe use that hook design pattern to insert check in the beginning

// maybe consider an interface that all trees implement
// maybe findLast, eraseLast

// Implement different iterators:
// create a generic BinaryTreeIterator with different sub-classes
// - inIter
// - preIter
// - postIter
// - levelIter
// also maybe their reverse version
// - see https://leetcode.com/problems/binary-search-tree-iterator/description/
// Then methods that return Node<T>* can return iterator e.g. find
// then the BinaryTree template can take another parameter enum IterType that the clients can specify
// which default iterator to be returned, e.g. find returns InIter by default
// but also allow specify which iterator to be returned at the method level,
// e.g. find(const T& val, IterType = InIter)

// serialize/deserialize (https://leetcode.com/explore/learn/card/data-structure-tree/133/conclusion/995/),
// just a variant of to/fromArrayRepresentation

// check priority queue, should we make left,right, parent constexpr?
// copy/move constructors, swap method for hashmap
// should rewrite static array, it uses array underneath, also Deque uses std::array, should switch to
// array::StaticArray

/* - Constructing expression trees (prefix notation).
- postfix evaluation

Take a look at the postorder part for expression tree:
https://leetcode.com/explore/learn/card/data-structure-tree/134/traverse-a-tree/992/#in-order-traversal

what do prefix and postfix mean?

Inorder traversal can be used to evaluate arithmetic expressions stored in expression trees.
- what is expression tree? ( I think it might be a good candidate for dynamic SQL?)
- postfix, prefix?


- In spanning trees: given a connected graph with N nodes, any spanning tree of it will have exactly N − 1
edges.
- In disjoint set union (DSU) or Kruskal’s algorithm for MST, you stop once you’ve added N − 1 edges — that’s
when you’ve built a spanning tree.

what are spanning trees?

trie problems:
- https://leetcode.com/problems/word-search-ii/description/ */

template <typename T, typename Hasher = std::hash<T>, typename KeyEqual = std::equal_to<T>>
  requires detail::Hasher<T, Hasher> && detail::KeyEqual<T, KeyEqual>
class BinaryTreeBase {

protected:
  void setRoot(gsl::owner<Node<T>*> node) noexcept { m_root = node; }
  void setRoot(std::nullptr_t) noexcept { m_root = static_cast<gsl::owner<Node<T>*>>(nullptr); }

  Hasher getHasher() noexcept { return m_hasher; }
  Hasher getHasher() const noexcept { return m_hasher; }

  KeyEqual getKeyEqual() noexcept { return m_keyEqual; }
  KeyEqual getKeyEqual() const noexcept { return m_keyEqual; }

  bool keyEqual(const T& a, const T& b) const noexcept { return m_keyEqual(a, b); }

  bool eraseNode(Node<T>* target, Node<T>* parent) { // NOLINT
    if (target == nullptr || empty()) return false;
    bool hasBothChildren = target->left() != nullptr && target->right() != nullptr;

    if (hasBothChildren) {
      Node<T>* predecessorParent = target;
      Node<T>* predecessor = target->left();

      while (predecessor->right() != nullptr) {
        predecessorParent = predecessor;
        predecessor = predecessor->right();
      }

      target->setValue(predecessor->value());
      target = predecessor;
      parent = predecessorParent;
    }

    // target has at most one child now
    Node<T>* child = target->left() != nullptr ? target->left() : target->right();

    if (parent == nullptr) {
      assert(root() == nullptr);
      setRoot(child);
    } else if (parent->left() == target) {
      parent->setLeft(child);
    } else {
      parent->setRight(child);
    }

    delete target; // NOLINT;
    return true;
  }

  template <typename Ptr> class NodeWithParent {
  public:
    Ptr target = nullptr;
    Ptr parent = nullptr;
  };

private:
  // using NodeType for both Node<T> and const Node<T>
  template <typename NodeType> class DefaultBreak {
  public:
    constexpr bool operator()(NodeType& /*_node*/) const noexcept { return false; }
  };

  template <typename NodeType> class DefaultBreakWithLevel {
  public:
    constexpr bool operator()(NodeType& /*_node*/, size_t /*_level*/) const noexcept { return false; }
  };

  gsl::owner<Node<T>*> m_root = nullptr;
  Hasher m_hasher;
  KeyEqual m_keyEqual;

  template <
      typename NodeType, std::invocable<NodeType&> Callback,
      std::predicate<NodeType&> ShouldBreak = DefaultBreak<NodeType>>
  bool preorderRecursive(NodeType* node, Callback&& cb, ShouldBreak shouldBreak = {}) const {
    if (node == nullptr) return true;

    cb(*node);
    if (shouldBreak(*node)) return false;

    if (!preorderRecursive(node->left(), std::forward<Callback>(cb), shouldBreak)) { return false; }

    if (!preorderRecursive(node->right(), std::forward<Callback>(cb), shouldBreak)) { return false; }

    return true;
  }

  template <
      typename NodeType, std::invocable<NodeType&> Callback,
      std::predicate<NodeType&> ShouldBreak = DefaultBreak<NodeType>>
  bool preorderIterative(
      NodeType* node,
      Callback&& cb, // NOLINT allow move-only callable
      ShouldBreak shouldBreak = {}
  ) const {
    if (node == nullptr) return true;

    queue::Deque<NodeType*> stack{node};
    while (!stack.empty()) {
      NodeType* cur = stack.back();
      stack.popBack();

      cb(*cur);
      if (shouldBreak(*cur)) return false;

      if (cur->right()) stack.pushBack(cur->right());
      if (cur->left()) stack.pushBack(cur->left());
    }
    return true;
  }

  template <
      typename NodeType, std::invocable<NodeType&> Callback,
      std::predicate<NodeType&> ShouldBreak = DefaultBreak<NodeType>>
  bool preorderMorris(
      NodeType* node,
      Callback&& cb, // NOLINT allow move-only callable
      ShouldBreak shouldBreak = {}
  ) {

    NodeType* cur = node;
    while (cur) {
      if (!cur->left()) {
        cb(*cur);
        if (shouldBreak(*cur)) return false;
        cur = cur->right();
      } else {
        NodeType* predecessor = cur->left();
        while (predecessor->right() && predecessor->right() != cur) predecessor = predecessor->right();

        if (!predecessor->right()) {
          cb(*cur);
          if (shouldBreak(*cur)) return false;
          predecessor->setRight(cur);
          cur = cur->left();
        } else {
          predecessor->setRight(nullptr);
          cur = cur->right();
        }
      }
    }
    return true;
  }

  template <
      typename NodeType, std::invocable<NodeType&> Callback,
      std::predicate<NodeType&> ShouldBreak = DefaultBreak<NodeType>>
  bool inorderRecursive(NodeType* node, Callback&& cb, ShouldBreak shouldBreak = {}) const {
    if (node == nullptr) return true;

    if (!inorderRecursive(node->left(), std::forward<Callback>(cb), shouldBreak)) return false;

    cb(*node);
    if (shouldBreak(*node)) return false;

    if (!inorderRecursive(node->right(), std::forward<Callback>(cb), shouldBreak)) return false;

    return true;
  }

  template <
      typename NodeType, std::invocable<NodeType&> Callback,
      std::predicate<NodeType&> ShouldBreak = DefaultBreak<NodeType>>
  bool inorderIterative(
      NodeType* node,
      Callback&& cb, // NOLINT allow move-only callable
      ShouldBreak shouldBreak = {}

  ) const {
    queue::Deque<NodeType*> stack;
    NodeType* cur = node;

    while (cur || !stack.empty()) {
      while (cur) {
        stack.pushBack(cur);
        cur = cur->left();
      }

      cur = stack.back();
      stack.popBack();

      cb(*cur);
      if (shouldBreak(*cur)) return false;

      cur = cur->right();
    }
    return true;
  }

  template <
      typename NodeType, std::invocable<NodeType&> Callback,
      std::predicate<NodeType&> ShouldBreak = DefaultBreak<NodeType>>
  bool inorderMorris(
      NodeType* node,
      Callback&& cb, // NOLINT allow move-only callable
      ShouldBreak shouldBreak = {}
  ) {
    NodeType* cur = node;

    while (cur) {
      if (!cur->left()) {
        cb(*cur);
        if (shouldBreak(*cur)) return false;
        cur = cur->right();
      } else {
        NodeType* predecessor = cur->left();
        while (predecessor->right() && predecessor->right() != cur) predecessor = predecessor->right();

        if (!predecessor->right()) {
          predecessor->setRight(cur);
          cur = cur->left();
        } else {
          predecessor->setRight(nullptr);
          cb(*cur);
          if (shouldBreak(*cur)) return false;
          cur = cur->right();
        }
      }
    }
    return true;
  }

  template <
      typename NodeType, std::invocable<NodeType&> Callback,
      std::predicate<NodeType&> ShouldBreak = DefaultBreak<NodeType>>
  bool inorderReverseRecursive(NodeType* node, Callback&& cb, ShouldBreak shouldBreak = {}) const {
    if (node == nullptr) return true;

    if (!inorderReverseRecursive(node->right(), std::forward<Callback>(cb), shouldBreak)) return false;

    cb(*node);
    if (shouldBreak(*node)) return false;

    if (!inorderReverseRecursive(node->left(), std::forward<Callback>(cb), shouldBreak)) return false;

    return true;
  }

  template <
      typename NodeType, std::invocable<NodeType&> Callback,
      std::predicate<NodeType&> ShouldBreak = DefaultBreak<NodeType>>
  bool inorderReverseIterative(
      NodeType* node,
      Callback&& cb, // NOLINT allow move-only callable
      ShouldBreak shouldBreak = {}
  ) const {
    if (node == nullptr) return true;

    NodeType* cur = node;
    queue::Deque<NodeType*> stack;

    while (cur || !stack.empty()) {
      while (cur) {
        stack.pushBack(cur);
        cur = cur->right();
      }

      cur = stack.back();
      stack.popBack();

      cb(*cur);
      if (shouldBreak(*cur)) return false;

      cur = cur->left();
    }
    return true;
  }

  template <
      typename NodeType, std::invocable<NodeType&> Callback,
      std::predicate<NodeType&> ShouldBreak = DefaultBreak<NodeType>>
  bool inorderReverseMorris(
      NodeType* node,
      Callback&& cb, // NOLINT allow move-only callable
      ShouldBreak shouldBreak = {}
  ) const {
    NodeType* cur = node;

    while (cur != nullptr) {
      if (!cur->right()) {
        cb(*cur);
        if (shouldBreak(*cur)) return false;
        cur = cur->left();
      } else {
        NodeType* predecessor = cur->right();
        while (predecessor->left() && predecessor->left() != cur) predecessor = predecessor->left();

        if (!predecessor->left()) {
          predecessor->setLeft(cur);
          cur = cur->right();
        } else {
          predecessor->setLeft(nullptr);
          cb(*cur);
          if (shouldBreak(*cur)) return false;
          cur = cur->left();
        }
      }
    }
    return true;
  }

  template <
      typename NodeType, std::invocable<NodeType&> Callback,
      std::predicate<NodeType&> ShouldBreak = DefaultBreak<NodeType>>
  bool postorderRecursive(NodeType* node, Callback&& cb, ShouldBreak shouldBreak = {}) const {
    if (node == nullptr) return true;

    if (!postorderRecursive(node->left(), std::forward<Callback>(cb), shouldBreak)) return false;

    if (!postorderRecursive(node->right(), std::forward<Callback>(cb), shouldBreak)) return false;

    cb(*node);

    return !shouldBreak(*node);
  }

  template <
      typename NodeType, std::invocable<NodeType&> Callback,
      std::predicate<NodeType&> ShouldBreak = DefaultBreak<NodeType>>
  bool postorderIterative(
      NodeType* node,
      Callback&& cb, // NOLINT allow move-only callable
      ShouldBreak shouldBreak = {}
  ) const {
    if (node == nullptr) return true;

    queue::Deque<NodeType*> s1{node};
    queue::Deque<NodeType*> s2;

    while (!s1.empty()) {
      NodeType* cur = s1.back();
      s1.popBack();
      s2.pushBack(cur);

      if (cur->left()) s1.pushBack(cur->left());
      if (cur->right()) s1.pushBack(cur->right());
    }

    while (!s2.empty()) {
      NodeType* cur = s2.back();
      s2.popBack();

      cb(*cur);
      if (shouldBreak(*cur)) return false;
    }

    return true;
  }

  // may be a bit less efficient in time complexity (compare to iterative/recursive approaches)
  template <typename NodeType, std::invocable<NodeType&> Callback, std::predicate<NodeType&> ShouldBreak = DefaultBreak<NodeType>>
  bool postorderMorris( // NOLINT
      NodeType* node,
      Callback&& cb, // NOLINT allow move-only callable
      ShouldBreak shouldBreak = {}
  ) {
    if (node == nullptr) return true;

    auto reversePath = [](NodeType* from, NodeType* to) {
      NodeType* prev = nullptr;
      NodeType* cur = from;
      NodeType* next = nullptr;
      while (cur != to) {
        next = cur->right();
        cur->setRight(prev);
        prev = cur;
        cur = next;
      }
      cur->setRight(prev);
    };

    auto visitReverse = [&](NodeType* from, NodeType* to) {
      reversePath(from, to);
      NodeType* cur = to;
      bool keepGoing = true;
      while (true) {
        cb(*cur);
        if (shouldBreak(*cur)) {
          keepGoing = false;
          break;
        }
        if (cur == from) break;
        cur = cur->right();
      }
      reversePath(to, from);
      return keepGoing;
    };

    NodeType dummy(0);
    dummy.setLeft(node);
    NodeType* cur = &dummy;

    while (cur) {
      if (!cur->left()) {
        cur = cur->right();
      } else {
        NodeType* predecessor = cur->left();
        while (predecessor->right() && predecessor->right() != cur) predecessor = predecessor->right();

        if (!predecessor->right()) {
          predecessor->setRight(cur);
          cur = cur->left();
        } else {
          predecessor->setRight(nullptr);
          if (!visitReverse(cur->left(), predecessor)) return false;
          cur = cur->right();
        }
      }
    }
    return true;
  }

  template <
      typename NodeType, std::invocable<NodeType&, size_t /* level, starts from 0 */> Callback,
      std::predicate<NodeType&, size_t /* level, starts from 0 */> ShouldBreak = DefaultBreak<NodeType>>
  bool levelorderIterative(
      NodeType* node,
      Callback&& cb, // NOLINT allow move-only callable
      ShouldBreak shouldBreak = {}
  ) const {
    if (node == nullptr) return true;
    size_t level = 0;

    queue::Deque<NodeType*> q{node};
    while (!q.empty()) {
      size_t levelSize = q.size();
      for (size_t i = 0; i < levelSize; ++i) {
        NodeType* cur = q.front();
        q.popFront();

        cb(*cur, level);
        if (shouldBreak(*cur, level)) return false;

        if (cur->left()) q.pushBack(cur->left());
        if (cur->right()) q.pushBack(cur->right());
      }
      level++;
    }
    return true;
  }

  virtual NodeWithParent<const Node<T>*> findFirstWithParent(const T& val) const noexcept {
    if (empty()) return {};
    queue::Deque<std::pair<const Node<T>*, const Node<T>*>> q{{root(), nullptr}};

    while (!q.empty()) {
      auto [node, parent] = q.front();
      q.popFront();
      if (keyEqual(node->value(), val)) return {node, parent};

      if (node->left() != nullptr) q.pushBack({node->left(), node});
      if (node->right() != nullptr) q.pushBack({node->right(), node});
    }
    return {};
  }

  NodeWithParent<Node<T>*> findFirstWithParent(const T& val) noexcept {
    auto res = static_cast<const BinaryTreeBase<T, Hasher, KeyEqual>*>(this)->findFirstWithParent(val);
    return {const_cast<Node<T>*>(res.target), const_cast<Node<T>*>(res.parent)};
  }

  virtual NodeWithParent<const Node<T>*> findByNode(const Node<T>* target) const noexcept {
    if (empty() || target == nullptr) return {};
    queue::Deque<std::pair<const Node<T>*, const Node<T>*>> q{{root(), nullptr}};

    while (!q.empty()) {
      auto [node, parent] = q.front();
      q.popFront();
      if (node == target) return {target, parent};

      if (node->left() != nullptr) q.pushBack({node->left(), node});
      if (node->right() != nullptr) q.pushBack({node->right(), node});
    }
    return {};
  }

  NodeWithParent<Node<T>*> findByNode(Node<T>* target) noexcept {
    auto res = static_cast<const BinaryTreeBase<T, Hasher, KeyEqual>*>(this)->findByNode(target);
    return {const_cast<Node<T>*>(res.target), const_cast<Node<T>*>(res.parent)};
  }

  void deepCopyTree(const BinaryTreeBase& other) {
    if (other.empty()) return;

    setRoot(new Node(other.root()->value()));
    queue::Deque<std::pair<Node<T>*, const Node<T>*>> q{{root(), other.root()}};

    while (!q.empty()) {
      auto [cur, otherCur] = q.front();
      q.popFront();
      if (otherCur->left() != nullptr) {
        cur->setLeft(new Node(otherCur->left()->value()));
        q.pushBack({cur->left(), otherCur->left()});
      }
      if (otherCur->right() != nullptr) {
        cur->setRight(new Node(otherCur->right()->value()));
        q.pushBack({cur->right(), otherCur->right()});
      }
    }
  }

  // find by level-order
  virtual array::DynamicArray<NodeWithParent<const Node<T>*>> findAllWithParent(const T& val) const noexcept {
    if (empty()) return {};
    queue::Deque<std::pair<const Node<T>*, const Node<T>*>> q{{root(), nullptr}};
    array::DynamicArray<NodeWithParent<const Node<T>*>> matches;

    while (!q.empty()) {
      auto [node, parent] = q.front();
      q.popFront();
      if (keyEqual(node->value(), val)) matches.pushBack({node, parent});

      if (node->left() != nullptr) q.pushBack({node->left(), node});
      if (node->right() != nullptr) q.pushBack({node->right(), node});
    }
    return matches;
  }

  array::DynamicArray<NodeWithParent<Node<T>*>> findAllWithParent(const T& val) noexcept {
    array::DynamicArray<NodeWithParent<const Node<T>*>> matches =
        static_cast<const BinaryTreeBase<T, Hasher, KeyEqual>*>(this)->findAllWithParent(val);

    size_t n = matches.size();
    array::DynamicArray<NodeWithParent<Node<T>*>> res;
    res.reserve(n);
    for (size_t i = 0; i < n; ++i) {
      res.emplaceBack(const_cast<Node<T>*>(matches[i].target), const_cast<Node<T>*>(matches[i].parent));
    }
    return res;
  }

  virtual const Node<T>*
  lowestCommonAncestorImpl(const Node<T>* root, const Node<T>* a, const Node<T>* b) const noexcept {
    if (root == nullptr || root == a || root == b) return root;
    const Node<T>* left = lowestCommonAncestorImpl(root->left(), a, b);
    const Node<T>* right = lowestCommonAncestorImpl(root->right(), a, b);
    if (left != nullptr && right != nullptr) return root;
    return left != nullptr ? left : right;
  }

public:
  BinaryTreeBase(Hasher hasher = {}, KeyEqual eq = {}) : m_hasher(hasher), m_keyEqual(eq) {}

  BinaryTreeBase(const BinaryTreeBase& other) : m_hasher(other.m_hasher), m_keyEqual(other.m_keyEqual) {
    deepCopyTree(other);
  }

  BinaryTreeBase& operator=(const BinaryTreeBase& other) {
    if (&other == this) return *this;
    clear();
    deepCopyTree(other);
    return *this;
  }

  BinaryTreeBase(BinaryTreeBase&& other) noexcept
      : m_hasher(std::move(other.m_hasher)), m_keyEqual(std::move(other.m_keyEqual)), m_root(other.root()) {
    other.setRoot(nullptr);
  }

  BinaryTreeBase& operator=(BinaryTreeBase&& other) noexcept {
    if (&other == this) return *this;
    clear();
    m_hasher = std::move(other.m_hasher);
    m_keyEqual = std::move(other.m_keyEqual);
    setRoot(other.root());
    other.setRoot(nullptr);
    return *this;
  }

  virtual ~BinaryTreeBase() noexcept { clear(); }

  template <std::invocable<Node<T>&> Callback, std::predicate<Node<T>&> ShouldBreak = DefaultBreak<Node<T>>>
  bool inorderTraverse(Callback&& cb, ShouldBreak shouldBreak = {}) {
    return inorderMorris(root(), std::forward<Callback>(cb), shouldBreak);
  }

  template <
      std::invocable<const Node<T>&> Callback,
      std::predicate<const Node<T>&> ShouldBreak = DefaultBreak<const Node<T>>>
  bool inorderTraverse(Callback&& cb, ShouldBreak shouldBreak = {}) const {
    return inorderIterative(root(), std::forward<Callback>(cb), shouldBreak);
  }

  /**
   * @brief Traverse the tree in **right -> node -> left** order,
   *        In BST, inorder is used to traverse the tree in a non-decreasing order,
   *        inorder-reverse can be used to traverse the tree in a non-increasing order.
   */
  template <std::invocable<Node<T>&> Callback, std::predicate<Node<T>&> ShouldBreak = DefaultBreak<Node<T>>>
  bool inorderReverseTraverse(Callback&& cb, ShouldBreak shouldBreak = {}) {
    return inorderReverseMorris(root(), std::forward<Callback>(cb), shouldBreak);
  }

  template <
      std::invocable<const Node<T>&> Callback,
      std::predicate<const Node<T>&> ShouldBreak = DefaultBreak<const Node<T>>>
  bool inorderReverseTraverse(Callback&& cb, ShouldBreak shouldBreak = {}) const {
    return inorderIterative(root(), std::forward<Callback>(cb), shouldBreak);
  }

  template <std::invocable<Node<T>&> Callback, std::predicate<Node<T>&> ShouldBreak = DefaultBreak<Node<T>>>
  bool preorderTraverse(Callback&& cb, ShouldBreak shouldBreak = {}) {
    return preorderMorris(root(), std::forward<Callback>(cb), shouldBreak);
  }

  template <
      std::invocable<const Node<T>&> Callback,
      std::predicate<const Node<T>&> ShouldBreak = DefaultBreak<const Node<T>>>
  bool preorderTraverse(Callback&& cb, ShouldBreak shouldBreak = {}) const {
    return preorderIterative(root(), std::forward<Callback>(cb), shouldBreak);
  }

  template <std::invocable<Node<T>&> Callback, std::predicate<Node<T>&> ShouldBreak = DefaultBreak<Node<T>>>
  bool postorderTraverse(Callback&& cb, ShouldBreak shouldBreak = {}) {
    return postorderIterative(root(), std::forward<Callback>(cb), shouldBreak);
  }

  template <
      std::invocable<const Node<T>&> Callback,
      std::predicate<const Node<T>&> ShouldBreak = DefaultBreak<const Node<T>>>
  bool postorderTraverse(Callback&& cb, ShouldBreak shouldBreak = {}) const {
    return postorderIterative(root(), std::forward<Callback>(cb), shouldBreak);
  }

  template <std::invocable<Node<T>&> Callback, std::predicate<Node<T>&> ShouldBreak = DefaultBreak<Node<T>>>
  bool levelorderTraverse(
      Callback&& cb, ShouldBreak shouldBreak = {} // NOLINT allow move-only callable
  ) {
    return levelorderTraverse(
        [&](Node<T>& node, size_t) { cb(node); }, [&](Node<T>& node, size_t) { return shouldBreak(node); }
    );
  }

  template <
      std::invocable<const Node<T>&> Callback,
      std::predicate<const Node<T>&> ShouldBreak = DefaultBreak<const Node<T>>>
  bool levelorderTraverse(
      Callback&& cb, ShouldBreak shouldBreak = {} // NOLINT allow move-only callable
  ) const {
    return levelorderTraverse(
        [&](const Node<T>& node, size_t) { cb(node); },
        [&](const Node<T>& node, size_t) { return shouldBreak(node); }
    );
  }

  template <
      std::invocable<Node<T>&, size_t /* level, starts from 0 */> Callback,
      std::predicate<Node<T>&, size_t /* level, starts from 0 */> ShouldBreak =
          DefaultBreakWithLevel<Node<T>>>
  bool levelorderTraverse(Callback&& cb, ShouldBreak shouldBreak = {}) {
    return levelorderIterative(root(), std::forward<Callback>(cb), shouldBreak);
  }

  template <
      std::invocable<const Node<T>&, size_t /* level, starts from 0 */> Callback,
      std::predicate<const Node<T>&, size_t /* level, starts from 0 */> ShouldBreak =
          DefaultBreakWithLevel<const Node<T>>>
  bool levelorderTraverse(Callback&& cb, ShouldBreak shouldBreak = {}) const {
    return levelorderIterative(root(), std::forward<Callback>(cb), shouldBreak);
  }

  array::DynamicArray<Node<T>*> getArrayRepresentation() const {
    array::DynamicArray<Node<T>*> res;
    queue::Deque<Node<T>*> q{root()};
    size_t treeHeight = height();
    size_t level = 0;

    while (!q.empty() && level < treeHeight) {
      size_t levelSize = q.size();
      for (size_t i = 0; i < levelSize; ++i) {
        Node<T>* node = q.front();
        q.popFront();
        res.pushBack(node);
        q.pushBack(node == nullptr ? nullptr : node->left());
        q.pushBack(node == nullptr ? nullptr : node->right());
      }
      ++level;
    }
    return res;
  }

  // insert by level-order, filling the tree left to right
  virtual Node<T>* insert(const T& val) noexcept {
    gsl::owner<Node<T>*> newNode = new Node<T>(val);

    if (empty()) {
      setRoot(newNode);
      return newNode; // NOLINT
    }

    bool attached = false;
    auto insertNode = [&](Node<T>& node) {
      if (node.left() == nullptr) {
        node.setLeft(newNode);
        attached = true;
        return;
      }
      if (node.right() == nullptr) {
        node.setRight(newNode);
        attached = true;
        return;
      }
    };
    auto shouldBreak = [&](Node<T>&) { return attached; };
    levelorderTraverse(insertNode, shouldBreak);
    return newNode;
  }

  Node<T>* findFirst(const T& val) noexcept {
    return const_cast<Node<T>*>(static_cast<const BinaryTreeBase*>(this)->findFirst(val));
  }

  const Node<T>* findFirst(const T& val) const noexcept {
    NodeWithParent result = findFirstWithParent(val);
    return result.target;
  }

  // maybe we can use the hook pattern instead of making them virtual
  virtual bool eraseNode(Node<T>* target) {
    auto [_target, parent] = findByNode(target);
    return eraseNode(target, parent);
  }

  virtual bool eraseFirst(const T& val) {
    auto [target, parent] = findFirstWithParent(val);
    return eraseNode(target, parent);
  }

  virtual size_t eraseAll(const T& val) {
    array::DynamicArray<NodeWithParent<Node<T>*>> matches = findAllWithParent(val);
    size_t count = 0;

    // process child nodes first to avoid dangling parent
    for (auto it = matches.rbegin(); it != matches.rend(); ++it) {
      auto [cur, parent] = *it;
      if (eraseNode(cur, parent)) count++;
    }
    return count;
  }

  void clear() noexcept {
    if (empty()) return;
    auto deleteNode = [](Node<T>& node) {
      delete &node; // NOLINT
    };
    postorderTraverse(deleteNode);
    setRoot(nullptr);
  }

  Node<T>* lowestCommonAncestor(Node<T>* a, Node<T>* b) noexcept {
    return const_cast<Node<T>*>(lowestCommonAncestorImpl(root(), a, b));
  }
  const Node<T>* lowestCommonAncestor(const Node<T>* a, const Node<T>* b) const noexcept {
    return lowestCommonAncestorImpl(root(), a, b);
  }

  [[nodiscard]] bool heightBalanced() const noexcept {
    if (empty()) return true;
    hashmap::HashMap<Node<T>*, int> height;

    bool balanced = true;

    auto isBalanced = [&](const Node<T>& node) {
      int leftHeight = height[node.left()];
      int rightHeight = height[node.right()];
      if (std::abs(leftHeight - rightHeight) > 1) balanced = false;
      height[&node] = 1 + std::max(leftHeight, rightHeight);
    };

    auto shouldBreak = [&](const Node<T>&) { return !balanced; };

    postorderTraverse(isBalanced, shouldBreak);
    return balanced;
  }

  [[nodiscard]] bool complete() const noexcept {
    if (empty()) return true;
    queue::Deque<Node<T>*> queue;
    queue.pushBack(root());
    bool seenNull = false;

    while (!queue.empty()) {
      Node<T>* node = queue.front();
      queue.popFront();

      if (node != nullptr) {
        if (seenNull) return false;
        queue.pushBack(node->left());
        queue.pushBack(node->right());
      } else {
        seenNull = true;
      }
    }
    return true;
  }

  [[nodiscard]] size_t height() const noexcept {
    if (empty()) return 0;
    size_t height = 0;
    queue::Deque<const Node<T>*> q{root()};

    while (!q.empty()) {
      size_t levelSize = q.size();

      for (size_t i = 0; i < levelSize; ++i) {
        const Node<T>* node = q.front();
        q.popFront();
        if (node->left() != nullptr) q.pushBack(node->left());
        if (node->right() != nullptr) q.pushBack(node->right());
      }
      ++height;
    }
    return height;
  }

  [[nodiscard]] bool full() const noexcept {
    if (empty()) return true;
    bool isFull = true;

    auto checkIsFull = [&](const Node<T>& node) {
      bool hasLeft = node.left() != nullptr;
      bool hasRight = node.right() != nullptr;
      if (hasLeft ^ hasRight) isFull = false;
    };

    auto shouldBreak = [&](const Node<T>&) { return !isFull; };
    levelorderTraverse(checkIsFull, shouldBreak);
    return isFull;
  }

  [[nodiscard]] bool symmetric() const noexcept {
    if (root() == nullptr) return true;
    queue::Deque<std::pair<Node<T>*, Node<T>*>> q{{root()->left(), root()->right()}};

    while (!q.empty()) {
      auto [a, b] = q.front();
      q.popFront();
      if (a == nullptr && b == nullptr) continue;
      if (a == nullptr || b == nullptr || a->value() != b->value()) return false;
      q.pushBack({a->left(), b->right()});
      q.pushBack({a->right(), b->left()});
    }
    return true;
  }

  [[nodiscard]] constexpr bool empty() const noexcept { return root() == nullptr; }

  [[nodiscard]] constexpr Node<T>* root() noexcept { return m_root; }             // NOLINT
  [[nodiscard]] constexpr const Node<T>* root() const noexcept { return m_root; } // NOLINT

  void swap(BinaryTreeBase& other) noexcept {
    using std::swap;
    swap(m_root, other.m_root);
    swap(m_hasher, other.m_hasher);
    swap(m_keyEqual, other.m_keyEqual);
  }
};
} // namespace detail

template <typename T, typename Hasher, typename KeyEqual>
void swap(
    detail::BinaryTreeBase<T, Hasher, KeyEqual>& a, detail::BinaryTreeBase<T, Hasher, KeyEqual>& b
) noexcept { // for ADL
  a.swap(b);
}
} // namespace tree
