#pragma once

#include "./Exception.hpp"
#include "./Iterator.hpp"
#include "./Node.hpp"
#include <initializer_list>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

namespace LinkedList {

template <typename NodeType> class LinkedListBase {
protected:
  using ValueType = decltype(std::declval<NodeType>().value);
  int size = 0;
  NodeType *head = nullptr;

  void release() noexcept {
    while (head) {
      NodeType *temp = head;
      head = head->next;
      delete temp;
    }
  }

  void deepCopy(const LinkedListBase &other) {
    if (!other.head) {
      head = nullptr;
      size = 0;
      return;
    }
    size = other.size;
    head = new NodeType(other.head->value);
    NodeType *current = head;
    NodeType *otherCurrent = other.head->next;
    while (otherCurrent) {
      current->next = new NodeType(otherCurrent->value);
      current = current->next;
      otherCurrent = otherCurrent->next;
    }
  }

  void move(LinkedListBase &other) noexcept {
    size = other.size;
    head = std::exchange(other.head, nullptr);
    other.size = 0;
  }

public:
  LinkedListBase() = default;
  LinkedListBase(std::initializer_list<ValueType> init) {
    for (auto it = std::rbegin(init); it != std::rend(init); ++it)
      pushFront(*it);
  }

  LinkedListBase(const LinkedListBase &other) { deepCopy(other); }
  LinkedListBase &operator=(const LinkedListBase &other) {
    if (this != &other) {
      release();
      deepCopy(other);
    }
    return *this;
  }

  LinkedListBase(LinkedListBase &&other) noexcept { move(other); }
  LinkedListBase &operator=(LinkedListBase &&other) noexcept {
    if (this != &other) {
      release();
      move(other);
    }
    return *this;
  }

  ~LinkedListBase() { release(); }

  int getSize() const { return size; }
  bool isEmpty() const { return head == nullptr; }

  virtual void pushFront(const ValueType &value) {
    head = new NodeType(value, head);
    size++;
  }

  void popFront() {
    if (!head)
      throw EmptyListException();
    NodeType *temp = head;
    head = head->next;
    delete temp;
    size--;
  }

  ValueType &front() {
    if (!head)
      throw EmptyListException();
    return head->value;
  }

  const ValueType &front() const {
    if (!head)
      throw EmptyListException();
    return head->value;
  }

  void fromVector(const std::vector<ValueType> &vec) {
    for (auto it = vec.rbegin(); it != vec.rend(); ++it)
      pushFront(*it);
  }

  void print() const {
    NodeType *current = head;
    while (current) {
      std::cout << current->value << " --> ";
      current = current->next;
    }
    std::cout << "nullptr\n";
  }

  virtual void reverse() {
    NodeType *prev = nullptr;
    NodeType *current = head;
    NodeType *next = nullptr;

    while (current) {
      next = current->next;
      current->next = prev;
      prev = current;
      current = next;
    }
    head = prev;
  }

  ForwardIterator<NodeType> begin() { return ForwardIterator<NodeType>(head); }
  ForwardIterator<NodeType> begin() const {
    return ForwardIterator<NodeType>(head);
  }

  ForwardIterator<NodeType> end() { return ForwardIterator<NodeType>(nullptr); }
  ForwardIterator<NodeType> end() const {
    return ForwardIterator<NodeType>(nullptr);
  }
};

} // namespace LinkedList