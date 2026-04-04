#pragma once
#include <gsl/gsl>

#include "../../algorithm/sort/sort.hpp"
#include "binary_tree_base.hpp"
#include <functional>
namespace tree {

template <typename T, typename Hasher, typename KeyEqual>
  requires detail::Hasher<T, Hasher> && detail::KeyEqual<T, KeyEqual>
class BinaryTree; // forward declaration, definition in binary_tree.hpp

/**
 *
 * Ordering rules:
 * - left < root <= right
 *
 * - The comparator (`Compare`) defines the tree’s ordering semantics.
 *   By default, `std::less<>` is used, which enforces the conventional
 *   BST invariant: values less than the current node go to the left
 *   subtree, values greater go to the right subtree.
 *
 * - If a different comparator is supplied (e.g. `std::greater<>`),
 *   the invariant is reversed accordingly: values considered "smaller"
 *   by the comparator go left, "larger" go right. In-order traversal
 *   will then yield elements in comparator-defined order (descending
 *   for `std::greater<>`).
 *
 * - Duplicate values are always inserted into the right subtree. This
 *   ensures a consistent policy and preserves traversal semantics
 *   (duplicates appear after the original value).
 *
 * Note: All operations (insert, erase, search, traversal) rely solely
 * on `Compare`. The type `T` does not need to define `<` or `>=` itself.
 */

template <
    typename T, typename Hasher = std::hash<T>, typename KeyEqual = std::equal_to<T>,
    typename Compare = std::less<>>
  requires detail::Hasher<T, Hasher> && detail::KeyEqual<T, KeyEqual> && detail::Comparator<T, Compare>
class BinarySearchTree : public detail::BinaryTreeBase<T, Hasher, KeyEqual> {
private:
  Compare m_compare;
  using ConstNodeWithParent =
      typename detail::BinaryTreeBase<T, Hasher, KeyEqual>::template NodeWithParent<const Node<T>*>;

  /**
   * Decide whether a value should be placed in or searched via the right subtree.
   * This uses the comparator consistently:
   * - With std::less<>, value >= node->value go right.
   * - With std::greater<>, value <= node->value go right.
   *
   * Note that if Compare is std::greater, the tree is built using that ordering.
   * This means numerically larger values are placed in the *left* subtree,
   * and smaller values are placed in the *right* subtree. In-order traversal
   * will therefore yield elements in descending order.
   */
  bool shouldGoRight(const T& val, const Node<T>* node) const noexcept {
    return !m_compare(val, node->value());
  }

  ConstNodeWithParent findFirstWithParent(const T& val) const noexcept override {
    if (this->empty()) return {};
    const Node<T>* cur = this->root();
    const Node<T>* parent = nullptr;

    while (cur != nullptr) {
      if (this->keyEqual(cur->value(), val)) return {cur, parent};
      parent = cur;
      if (shouldGoRight(val, cur)) {
        cur = cur->right();
      } else {
        cur = cur->left();
      }
    }
    return {};
  }

  ConstNodeWithParent findByNode(const Node<T>* target) const noexcept override {
    if (this->empty() || target == nullptr) return {};
    const Node<T>* cur = this->root();
    const Node<T>* parent = nullptr;

    while (cur != nullptr) {
      if (cur == target) return {cur, parent};
      parent = cur;
      if (shouldGoRight(target->value(), cur)) {
        cur = cur->right();
      } else {
        cur = cur->left();
      }
    }
    return {};
  }

  array::DynamicArray<ConstNodeWithParent> findAllWithParent(const T& val) const noexcept override {
    const Node<T>* cur = this->root();
    if (cur == nullptr) return 0;

    const Node<T>* parent = nullptr;
    size_t count = 0;
    array::DynamicArray<ConstNodeWithParent> matches;

    while (cur != nullptr) {
      if (this->keyEqual(cur->value(), val)) matches.pushBack({cur, parent});
      parent = cur;
      if (shouldGoRight(val, cur)) {
        cur = cur->right();
      } else {
        cur = cur->left();
      }
    }
    return matches;
  }

  const Node<T>*
  lowestCommonAncestorImpl(const Node<T>* root, const Node<T>* a, const Node<T>* b) const noexcept override {
    while (root != nullptr) {
      if (shouldGoRight(a->value(), root) && shouldGoRight(b->value(), root)) {
        root = root->right();
      } else if (!shouldGoRight(a->value(), root) && !shouldGoRight(b->value(), root)) {
        root = root->left();
      } else {
        return root;
      }
    }
    return nullptr;
  }

  // move T instead of copy
  Node<T>* fromValuesRecursive(array::DynamicArray<T>& sortedVals, int start, int end) {
    if (start > end) return nullptr;
    int mid = start + ((end - start) / 2);

    // move mid to the index of the leftmost duplicate value -> all duplicates go right
    while (mid - 1 >= start && sortedVals[mid - 1] == sortedVals[mid]) --mid;

    Node<T>* node = new Node<T>(std::move(sortedVals[mid])); // NOLINT
    node->setLeft(fromValuesRecursive(sortedVals, start, mid - 1));
    node->setRight(fromValuesRecursive(sortedVals, mid + 1, end));
    return node;
  }

  Node<T>* fromValuesIterative(array::DynamicArray<T>& sortedVals, int start, int end) {
    if (sortedVals.size() == 0) return nullptr;
    int mid = start + ((end - start) / 2);

    // move mid to the index of the leftmost duplicate value -> all duplicates go right
    while (mid - 1 >= start && sortedVals[mid - 1] == sortedVals[mid]) --mid;

    Node<T>* root = new Node<T>(std::move(sortedVals[mid])); // NOLINT
    struct Frame {
      Node<T>* node;
      int start;
      int end;
    };
    queue::Deque<Frame> queue{{root, start, end}};

    while (!queue.empty()) {
      auto [cur, curStart, curEnd] = queue.front();
      queue.popFront();

      int mid = curStart + ((curEnd - curStart) / 2);
      // enforce duplicates rule
      while (mid - 1 >= curStart && sortedVals[mid - 1] == sortedVals[mid]) --mid;

      int leftEnd = mid - 1;

      if (curStart <= leftEnd) {
        int leftMid = curStart + ((leftEnd - curStart) / 2);

        // enforce duplicates rule
        while (leftMid - 1 >= curStart && sortedVals[leftMid - 1] == sortedVals[leftMid]) --leftMid;
        Node<T>* node = new Node(std::move(sortedVals[leftMid])); // NOLINT
        cur->setLeft(node);
        queue.pushBack({node, curStart, leftEnd});
      }

      int rightStart = mid + 1;
      if (rightStart <= curEnd) {
        int rightMid = rightStart + ((curEnd - rightStart) / 2);

        // enforce duplicates rule
        while (rightMid - 1 >= rightStart && sortedVals[rightMid - 1] == sortedVals[rightMid]) --rightMid;
        Node<T>* node = new Node(std::move(sortedVals[rightMid])); // NOLINT
        cur->setRight(node);
        queue.pushBack({node, rightStart, curEnd});
      }
    }
    return root;
  }

public:
  BinarySearchTree(
      std::initializer_list<T> values, Hasher hasher = {}, KeyEqual eq = {}, Compare compare = {}
  )
      : detail::BinaryTreeBase<T, Hasher, KeyEqual>(hasher, eq), m_compare(compare) {
    fromValues(values);
  }

  template <std::input_iterator InputIt>
    requires std::constructible_from<T, std::iter_value_t<InputIt>>
  BinarySearchTree(InputIt first, InputIt last, Hasher hasher = {}, KeyEqual eq = {}, Compare compare = {})
      : detail::BinaryTreeBase<T, Hasher, KeyEqual>(hasher, eq), m_compare(compare) {
    fromValues(first, last);
  }

  explicit BinarySearchTree(const BinaryTree<T, Hasher, KeyEqual>& bt, Compare compare = {})
      : detail::BinaryTreeBase<T, Hasher, KeyEqual>(bt.getHasher(), bt.getKeyEqual()), m_compare(compare) {
    array::DynamicArray<T> values;
    auto getValue = [&](auto&& node) { values.emplaceBack(node.value()); };
    bt.levelorderTraverse(getValue);

    sort::heapSort(values.begin(), values.end(), compare);
    fromValues(values.begin(), values.end());
  }

  explicit BinarySearchTree(BinaryTree<T, Hasher, KeyEqual>&& bt, Compare compare = {}) // NOLINT
      : detail::BinaryTreeBase<T, Hasher, KeyEqual>(bt.getHasher(), bt.getKeyEqual()), m_compare(compare) {
    array::DynamicArray<T> values;
    auto getValue = [&](auto&& node) { values.emplaceBack(std::move(node.value())); };
    bt.levelorderTraverse(getValue);

    sort::heapSort(values.begin(), values.end(), compare);
    fromValues(values.begin(), values.end());
  }

  BinarySearchTree(Hasher hasher = {}, KeyEqual eq = {}, Compare compare = {})
      : detail::BinaryTreeBase<T, Hasher, KeyEqual>(hasher, eq), m_compare(compare) {}

  BinarySearchTree(const BinarySearchTree& other)
      : detail::BinaryTreeBase<T, Hasher, KeyEqual>(other), m_compare(other.m_compare) {}

  BinarySearchTree& operator=(const BinarySearchTree& other) {
    if (&other == this) return *this;
    detail::BinaryTreeBase<T, Hasher, KeyEqual>::operator=(other);
    m_compare = other.m_compare;
    return *this;
  }

  BinarySearchTree(BinarySearchTree&& other) noexcept
      : detail::BinaryTreeBase<T, Hasher, KeyEqual>(std::move(other)), m_compare(std::move(other.m_compare)) {
  }

  BinarySearchTree& operator=(BinarySearchTree&& other) noexcept {
    if (&other == this) return *this;
    detail::BinaryTreeBase<T, Hasher, KeyEqual>::operator=(std::move(other));
    m_compare = std::move(other.m_compare);
    return *this;
  }

  ~BinarySearchTree() noexcept override = default;

  Node<T>* insert(const T& val) noexcept override {
    gsl::owner<Node<T>*> newNode = new Node<T>(val);

    if (this->root() == nullptr) {
      this->setRoot(newNode);
      return newNode; // NOLINT
    }

    Node<T>* cur = this->root();

    while (true) {
      if (shouldGoRight(val, cur)) {
        if (cur->right() == nullptr) {
          cur->setRight(newNode);
          break;
        }
        cur = cur->right();
      } else {
        if (cur->left() == nullptr) {
          cur->setLeft(newNode);
          break;
        }
        cur = cur->left();
      }
    }
    return newNode;
  }

  [[nodiscard]] bool validBST() const noexcept {
    std::optional<T> prev = std::nullopt;
    bool valid = true;

    auto cb = [&](const Node<T>& node) -> void {};
    auto shouldBreak = [&](const Node<T>& node) {
      if (prev.has_value()) {
        if (!m_compare(prev.value(), node.value())) {
          valid = false;
          return true;
        }
      }
      prev = node.value();
      return false;
    };

    this->inorderTraverse(cb, shouldBreak);
    return valid;
  }

  // creates a balanced BST from values
  template <std::input_iterator InputIt>
    requires std::constructible_from<T, std::iter_value_t<InputIt>>
  void fromValues(InputIt first, InputIt last) {
    if (!this->empty()) this->clear();
    int n = std::distance(first, last);
    array::DynamicArray<T> vals(first, last);
    sort::heapSort(vals.begin(), vals.end(), m_compare);
    this->setRoot(fromValuesIterative(vals, 0, n - 1));
  }

  // creates a balanced BST from values
  void fromValues(std::initializer_list<T> init) { fromValues(init.begin(), init.end()); }

  void swap(BinarySearchTree& other) noexcept {
    using std::swap;
    detail::BinaryTreeBase<T, Hasher, KeyEqual>::swap(other);
    swap(m_compare, other.m_compare);
  }
  // for ADL
  friend void swap(BinarySearchTree& a, BinarySearchTree& b) noexcept { a.swap(b); }
};
} // namespace tree
