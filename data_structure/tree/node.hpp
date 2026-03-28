#pragma once
#include "binary_tree_base.hpp"
#include "detail.hpp"
#include <utility>

namespace tree {

template <typename T> class Node {
  template <typename U, typename Hasher, typename KeyEqual>
    requires detail::Hasher<U, Hasher> && detail::KeyEqual<U, KeyEqual>
  friend class detail::BinaryTreeBase;

  template <typename U, typename Hasher, typename KeyEqual>
    requires detail::Hasher<U, Hasher> && detail::KeyEqual<U, KeyEqual>
  friend class BinaryTree;

  template <typename U, typename Hasher, typename KeyEqual, typename Compare>
    requires detail::Hasher<U, Hasher> && detail::KeyEqual<U, KeyEqual> && detail::Comparator<U, Compare>
  friend class BinarySearchTree;

private:
  T m_value;
  Node* m_left = nullptr;
  Node* m_right = nullptr;

  void setValue(const T& val) noexcept { m_value = val; }
  void setValue(T&& val) noexcept { m_value = std::move(val); }

  void setLeft(Node<T>* left) noexcept { m_left = left; }
  void setRight(Node<T>* right) noexcept { m_right = right; }

public:
  constexpr Node() = default;
  constexpr Node(const Node& other) = delete;
  constexpr Node(Node&& other) noexcept = delete;
  constexpr Node& operator=(const Node& other) = delete;
  constexpr Node& operator=(Node&& other) noexcept = delete;
  constexpr ~Node() noexcept = default;

  constexpr explicit Node(const T& val, Node* left = nullptr, Node* right = nullptr) // NOLINT
      : m_value(val), m_left(left), m_right(right) {}

  constexpr explicit Node(T&& val, Node* left = nullptr, Node* right = nullptr) // NOLINT
      : m_value(std::move(val)), m_left(left), m_right(right) {}

  const T& value() const noexcept { return m_value; }

  Node<T>* left() noexcept { return m_left; }
  const Node<T>* left() const noexcept { return m_left; }

  Node<T>* right() noexcept { return m_right; }
  const Node<T>* right() const noexcept { return m_right; }
};
} // namespace tree
