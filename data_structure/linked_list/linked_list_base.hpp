#pragma once
#include "./exception.hpp"
#include "./iterator.hpp"
#include <initializer_list>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

namespace linkedlist {

template <typename NodeType> class LinkedListBase {
protected:
  using ValueType = decltype(std::declval<NodeType>().value);
  size_t size = 0;          // NOLINT
  NodeType* head = nullptr; // NOLINT

  void release() noexcept {
    while (head) {
      NodeType* temp = head;
      head = head->next;
      delete temp; // NOLINT
    }
  }

  void deepCopy(const LinkedListBase& other) {
    if (other.head == nullptr) {
      head = nullptr;
      size = 0;
      return;
    }
    size = other.size;
    head = new NodeType(other.head->value); // NOLINT
    NodeType* current = head;
    NodeType* otherCurrent = other.head->next;
    while (otherCurrent) {
      current->next = new NodeType(otherCurrent->value); // NOLINT
      current = current->next;
      otherCurrent = otherCurrent->next;
    }
  }

  void move(LinkedListBase&& other) noexcept { // NOLINT
    size = other.size;
    head = std::exchange(other.head, nullptr);
    other.size = 0;
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
      release();
      deepCopy(other);
    }
    return *this;
  }

  LinkedListBase(LinkedListBase&& other) noexcept { move(std::move(other)); }
  LinkedListBase& operator=(LinkedListBase&& other) noexcept {
    if (this != &other) {
      release();
      move(std::move(other));
    }
    return *this;
  }

  ~LinkedListBase() { release(); }

  [[nodiscard]] int getSize() const noexcept { return size; }
  [[nodiscard]] bool isEmpty() const noexcept { return head == nullptr; }

  virtual void pushFront(const ValueType& value) {
    head = new NodeType(value, head); // NOLINT
    size++;
  }

  virtual void pushFront(ValueType&& value) {
    head = new NodeType(std::move(value), head); // NOLINT
    size++;
  }

  template <typename... Args> ValueType& emplaceFront(Args... args) {
    head = new NodeType(head, std::forward<Args>(args)...); // NOLINT
    size++;
    return head->value;
  }

  void popFront() {
    if (head == nullptr) throw EmptyListException();
    NodeType* temp = head;
    head = head->next;
    delete temp; // NOLINT
    size--;
  }

  ValueType& front() {
    if (head == nullptr) throw EmptyListException();
    return head->value;
  }

  const ValueType& front() const {
    if (head == nullptr) throw EmptyListException();
    return head->value;
  }

  void fromVector(const std::vector<ValueType>& vec) {
    for (auto it = vec.rbegin(); it != vec.rend(); ++it) pushFront(*it);
  }

  void print() const {
    NodeType* current = head;
    while (current) {
      std::cout << current->value << " --> ";
      current = current->next;
    }
    std::cout << "nullptr\n";
  }

  virtual void reverse() {
    NodeType* prev = nullptr;
    NodeType* current = head;
    NodeType* next = nullptr;

    while (current) {
      next = current->next;
      current->next = prev;
      prev = current;
      current = next;
    }
    head = prev;
  }

  Iterator begin() { return Iterator(head); }
  ConstIterator begin() const { return ConstIterator(head); }

  Iterator end() { return Iterator(nullptr); }
  ConstIterator end() const { return ConstIterator(nullptr); }
};

} // namespace linkedlist
