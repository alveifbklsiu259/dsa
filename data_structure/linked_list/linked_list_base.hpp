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
  size_t size = 0;
  NodeType* head = nullptr;

  void release() noexcept {
    while (head) {
      NodeType* temp = head;
      head = head->next;
      delete temp;
    }
  }

  void deepCopy(const LinkedListBase& other) {
    if (other.head == nullptr) {
      head = nullptr;
      size = 0;
      return;
    }
    size = other.size;
    head = new NodeType(other.head->value);
    NodeType* current = head;
    NodeType* otherCurrent = other.head->next;
    while (otherCurrent) {
      current->next = new NodeType(otherCurrent->value);
      current = current->next;
      otherCurrent = otherCurrent->next;
    }
  }

  void move(LinkedListBase&& other) noexcept {
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

  int getSize() const { return size; }
  bool isEmpty() const { return head == nullptr; }

  virtual void pushFront(const ValueType& value) {
    // std::cout << "inside LinkedListBase pushFront - lvalue" << std::endl;
    head = new NodeType(value, head);
    size++;
  }

  virtual void pushFront(ValueType&& value) {
    // std::cout << "inside LinkedListBase pushFront - rvalue" << std::endl;
    head = new NodeType(std::move(value), head);
    size++;
  }

  template <typename... Args> ValueType& emplaceFront(Args... args) {
    // std::cout << "inside LinkedListBase::emplaceFront" << std::endl;
    head = new NodeType(head, std::forward<Args>(args)...);
    size++;
    return head->value;
  }

  void popFront() {
    if (head == nullptr) throw EmptyListException();
    NodeType* temp = head;
    head = head->next;
    delete temp;
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
