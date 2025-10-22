#pragma once
#include <concepts>
#include <type_traits>
#include <utility>

namespace linkedlist {

template <typename N>
concept Node = requires(N n) {
  n.value;
  { n.next } -> std::convertible_to<std::remove_const_t<N>*>;
};

template <Node NodeType> class ForwardIterator {
protected:
  NodeType* m_current = nullptr; // NOLINT

public:
  using value_type = std::remove_const_t<decltype(std::declval<NodeType>().value)>;
  using reference = std::conditional_t<std::is_const_v<NodeType>, const value_type&, value_type&>;
  using pointer = std::conditional_t<std::is_const_v<NodeType>, const value_type*, value_type*>;

  ForwardIterator() = default;
  explicit ForwardIterator(NodeType* node) : m_current(node) {}

  ForwardIterator<NodeType>& operator++() {
    m_current = m_current->next;
    return *this;
  }

  ForwardIterator<NodeType> operator++(int) {
    ForwardIterator<NodeType> snapshot = *this;
    ++(*this);
    return snapshot;
  }
  reference operator*() const { return m_current->value; }
  pointer operator->() const { return &m_current->value; }

  bool operator!=(const ForwardIterator<NodeType>& other) const {
    return m_current != other.m_current;
  }
  bool operator==(const ForwardIterator<NodeType>& other) const {
    return m_current == other.m_current;
  }
};

template <typename NodeType> class BidirectionalIterator : public ForwardIterator<NodeType> {
public:
  using ForwardIterator<NodeType>::ForwardIterator;

  BidirectionalIterator<NodeType>& operator--() {
    this->m_current = this->m_current->prev;
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
  NodeType* m_current = nullptr;

public:
  using value_type = std::remove_const_t<decltype(std::declval<NodeType>().value)>;
  using reference = std::conditional_t<std::is_const_v<NodeType>, const value_type&, value_type&>;
  using pointer = std::conditional_t<std::is_const_v<NodeType>, const value_type*, value_type*>;
  explicit ReverseIterator(NodeType* node) : m_current(node) {}

  ReverseIterator<NodeType>& operator++() {
    m_current = m_current->prev;
    return *this;
  }

  ReverseIterator<NodeType> operator++(int) {
    ReverseIterator<NodeType> snapshot = *this;
    ++(*this);
    return snapshot;
  }

  ReverseIterator<NodeType>& operator--() {
    m_current = m_current->next;
    return *this;
  }

  ReverseIterator<NodeType> operator--(int) {
    ReverseIterator<NodeType> snapshot = *this;
    --(*this);
    return snapshot;
  }

  bool operator==(const ReverseIterator<NodeType>& other) const {
    return m_current == other.m_current;
  }
  bool operator!=(const ReverseIterator<NodeType>& other) const {
    return m_current != other.m_current;
  }

  reference operator*() const { return m_current->value; }
  pointer operator->() const { return m_current->value; }
};

} // namespace linkedlist
