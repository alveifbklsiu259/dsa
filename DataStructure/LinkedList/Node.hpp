#pragma once

namespace LinkedList {
template <typename T> class Node {
public:
  T value;
  Node<T> *next;
  Node(const T &value, Node<T> *next = nullptr) : value(value), next(next){};
  Node(const Node<T> &) = delete;
  Node<T> &operator=(const Node<T> &) = delete;
  Node(Node<T> &&other) = delete;
  Node<T> &operator=(Node<T> &&) = delete;
  ~Node() = default;
};
} // namespace LinkedList
