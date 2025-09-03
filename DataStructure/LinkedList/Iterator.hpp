#pragma once

#include "./Node.hpp"

namespace LinkedList {
template <typename NodeType> class ForwardIterator {

protected:
  NodeType *current = nullptr;
  using ValueType = decltype(std::declval<NodeType>().value);

public:
  explicit ForwardIterator(NodeType *node) : current(node) {}

  ForwardIterator<NodeType> &operator++() {
    current = current->next;
    return *this;
  }

  ForwardIterator operator++(int) {
    ForwardIterator<NodeType> snapshot = *this;
    ++(*this);
    return snapshot;
  }

  ValueType &operator*() const { return current->value; }

  bool operator!=(const ForwardIterator<NodeType> &other) const {
    return current != other.current;
  }

  bool operator==(const ForwardIterator<NodeType> &other) const {
    return current == other.current;
  }
};

template <typename NodeType>
class BidirectionalIterator : public ForwardIterator<NodeType> {
public:
  using ForwardIterator<NodeType>::ForwardIterator;

  BidirectionalIterator<NodeType> &operator--() {
    this->current = this->current->prev;
    return *this;
  }

  BidirectionalIterator<NodeType> operator--(int) {
    BidirectionalIterator<NodeType> snapshot = *this;
    --(*this);
    return snapshot;
  }
};

template <typename NodeType> class ReverseIterator {
private:
  NodeType *current = nullptr;
  using ValueType = decltype(std::declval<NodeType>().value);

public:
  explicit ReverseIterator(NodeType *node) : current(node) {}

  ReverseIterator<NodeType> &operator++() {
    current = current->prev;
    return *this;
  }

  ReverseIterator<NodeType> operator++(int) {
    ReverseIterator<NodeType> snapshot = *this;
    ++(*this);
    return snapshot;
  }

  ReverseIterator<NodeType> &operator--() {
    current = current->next;
    return *this;
  }

  ReverseIterator<NodeType> operator--(int) {
    ReverseIterator<NodeType> snapshot = *this;
    --(*this);
    return snapshot;
  }

  bool operator==(const ReverseIterator<NodeType> &other) const {
    return current == other.current;
  }

  bool operator!=(const ReverseIterator<NodeType> &other) const {
    return current != other.current;
  }

  ValueType &operator*() const { return current->value; }
};

} // namespace LinkedList