#pragma once

#include "./Node.hpp"

namespace LinkedList {
template <typename T> class Iterator {
private:
  Node<T> *current;

public:
  explicit Iterator(Node<T> *node) : current(node) {}

  Iterator<T> &operator++() {
    current = current->next;
    return *this;
  }

  Iterator operator++(int) {
    current = *this;
    ++(*this);
    return current;
  }

  T &operator*() const { return current->value; }

  bool operator!=(const Iterator<T> &other) const {
    return current != other.current;
  }
};
} // namespace LinkedList