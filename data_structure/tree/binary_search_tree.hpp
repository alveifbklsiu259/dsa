#pragma once
#include <gsl/gsl>

#include "binary_tree.hpp"
#include <functional>
namespace tree {
/**
 *
 * Ordering rules:
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
class BinarySearchTree : public BinaryTree<T, Hasher, KeyEqual> {
private:
  Compare m_compare;
  using ConstNodeWithParent =
      typename BinaryTree<T, Hasher, KeyEqual>::template NodeWithParent<const Node<T>*>;

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

public:
  // TODO:
  // need to enforce ordering rules for from methods

  BinarySearchTree(Hasher hasher = {}, KeyEqual eq = {}, Compare compare = {})
      : BinaryTree<T, Hasher, KeyEqual>(hasher, eq), m_compare(compare) {}

  BinarySearchTree(const BinarySearchTree& other)
      : BinaryTree<T, Hasher, KeyEqual>(other), m_compare(other.m_compare) {}

  BinarySearchTree& operator=(const BinarySearchTree& other) {
    if (&other == this) return *this;
    BinaryTree<T, Hasher, KeyEqual>::operator=(other);
    m_compare = other.m_compare;
    return *this;
  }

  BinarySearchTree(BinarySearchTree&& other) noexcept
      : BinaryTree<T, Hasher, KeyEqual>(std::move(other)), m_compare(std::move(other.m_compare)) {}

  BinarySearchTree& operator=(BinarySearchTree&& other) noexcept {
    if (&other == this) return *this;
    BinaryTree<T, Hasher, KeyEqual>::operator=(std::move(other));
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
};
} // namespace tree
