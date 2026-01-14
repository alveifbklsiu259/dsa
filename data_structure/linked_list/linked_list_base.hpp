#pragma once
#include "./exception.hpp"
#include "./iterator.hpp"
#include <concepts>
#include <initializer_list>
#include <iostream>
#include <string>
#include <utility>
#include <vector>
namespace linkedlist {

template <typename NodeType> class LinkedListBase {
protected:
  using ValueType = decltype(std::declval<NodeType>().value);
  size_t m_size = 0;          // NOLINT
  NodeType* m_head = nullptr; // NOLINT

  void deepCopy(const LinkedListBase& other) {
    if (other.m_head == nullptr) {
      m_head = nullptr;
      m_size = 0;
      return;
    }
    m_size = other.m_size;
    m_head = new NodeType(other.m_head->value); // NOLINT
    NodeType* current = m_head;
    NodeType* otherCurrent = other.m_head->next;
    while (otherCurrent) {
      current->next = new NodeType(otherCurrent->value); // NOLINT
      current = current->next;
      otherCurrent = otherCurrent->next;
    }
  }

  void move(LinkedListBase&& other) noexcept { // NOLINT
    m_size = other.m_size;
    m_head = std::exchange(other.m_head, nullptr);
    other.m_size = 0;
  }

public:
  using Iterator = ForwardIterator<NodeType>;
  using ConstIterator = ForwardIterator<const NodeType>;

  LinkedListBase() = default;
  LinkedListBase(std::initializer_list<ValueType> init) {
    for (auto it = std::rbegin(init); it != std::rend(init); ++it) pushFront(*it);
  }

  LinkedListBase(const LinkedListBase& other) { deepCopy(other); }
  LinkedListBase& operator=(const LinkedListBase& other) {
    if (this != &other) {
      clear();
      deepCopy(other);
    }
    return *this;
  }

  LinkedListBase(LinkedListBase&& other) noexcept { move(std::move(other)); }
  LinkedListBase& operator=(LinkedListBase&& other) noexcept {
    if (this != &other) {
      clear();
      move(std::move(other));
    }
    return *this;
  }

  ~LinkedListBase() noexcept { clear(); }

  [[nodiscard]] int size() const noexcept { return m_size; }
  [[nodiscard]] bool empty() const noexcept { return m_head == nullptr; }

  virtual void pushFront(const ValueType& value) {
    m_head = new NodeType(value, m_head); // NOLINT
    m_size++;
  }

  virtual void pushFront(ValueType&& value) {
    m_head = new NodeType(std::move(value), m_head); // NOLINT
    m_size++;
  }

  template <typename... Args> ValueType& emplaceFront(Args... args) {
    m_head = new NodeType(m_head, std::forward<Args>(args)...); // NOLINT
    m_size++;
    return m_head->value;
  }

  void clear() noexcept {
    while (m_head) {
      NodeType* temp = m_head;
      m_head = m_head->next;
      delete temp; // NOLINT
    }
    m_size = 0;
  }

  void popFront() {
    if (m_head == nullptr) throw EmptyListException();
    NodeType* temp = m_head;
    m_head = m_head->next;
    delete temp; // NOLINT
    m_size--;
  }

  ValueType& front() {
    if (m_head == nullptr) throw EmptyListException();
    return m_head->value;
  }

  const ValueType& front() const {
    if (m_head == nullptr) throw EmptyListException();
    return m_head->value;
  }

  void fromVector(const std::vector<ValueType>& vec) {
    for (auto it = vec.rbegin(); it != vec.rend(); ++it) pushFront(*it);
  }

  template <typename Pred>
    requires std::predicate<Pred, const ValueType&>
  size_t removeIf(Pred pred) {
    size_t removedCount = 0;
    NodeType** link = &m_head;
    while (*link) {
      if (pred((*link)->value)) {
        NodeType* temp = *link;
        *link = (*link)->next;
        delete temp; // NOLINT
        m_size--;
        removedCount++;
      } else {
        link = &((*link)->next);
      }
    }
    return removedCount;
  }

  size_t remove(const ValueType& value) {
    return removeIf([&](const ValueType& v) { return v == value; });
  }

  void print() const {
    NodeType* current = m_head;
    while (current) {
      std::cout << current->value << " --> ";
      current = current->next;
    }
    std::cout << "nullptr\n";
  }

  virtual void reverse() {
    NodeType* prev = nullptr;
    NodeType* current = m_head;
    NodeType* next = nullptr;

    while (current) {
      next = current->next;
      current->next = prev;
      prev = current;
      current = next;
    }
    m_head = prev;
  }

  Iterator begin() noexcept(noexcept(Iterator(m_head))) { return Iterator(m_head); }
  ConstIterator begin() const noexcept(noexcept(Iterator(m_head))) { return ConstIterator(m_head); }

  Iterator end() noexcept(noexcept(Iterator(nullptr))) { return Iterator(nullptr); }
  ConstIterator end() const noexcept(noexcept(Iterator(nullptr))) { return ConstIterator(nullptr); }
};

} // namespace linkedlist
