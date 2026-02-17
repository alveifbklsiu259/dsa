#pragma once
#include <utility>

namespace tree {

template <typename T> class Node {
public:
  T value;               // NOLINT
  Node* left = nullptr;  // NOLINT
  Node* right = nullptr; // NOLINT
  constexpr Node() = default;
  constexpr Node(const Node& other) = delete;
  constexpr Node(Node&& other) noexcept = delete;
  constexpr Node& operator=(const Node& other) = delete;
  constexpr Node& operator=(Node&& other) noexcept = delete;
  constexpr ~Node() noexcept = default;

  constexpr explicit Node(const T& val, Node* left = nullptr, Node* right = nullptr) // NOLINT
      : value(val), left(left), right(right) {}

  constexpr explicit Node(T&& val, Node* left = nullptr, Node* right = nullptr) // NOLINT
      : value(std::move(val)), left(left), right(right) {}
};
} // namespace tree
