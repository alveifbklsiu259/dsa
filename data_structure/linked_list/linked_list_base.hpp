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
  size_t size = 0;          // NOLINT
  NodeType* head = nullptr; // NOLINT

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

  void clear() noexcept {
    while (head) {
      NodeType* temp = head;
      head = head->next;
      delete temp; // NOLINT
    }
    size = 0;
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

  template <typename Pred>
    requires std::predicate<Pred, const ValueType&>
  size_t removeIf(Pred pred) {
    size_t removedCount = 0;
    NodeType** link = &head;
    while (*link) {
      if (pred((*link)->value)) {
        NodeType* temp = *link;
        *link = (*link)->next;
        delete temp; // NOLINT
        size--;
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

  Iterator begin() noexcept(noexcept(Iterator(head))) { return Iterator(head); }
  ConstIterator begin() const noexcept(noexcept(Iterator(head))) { return ConstIterator(head); }

  Iterator end() noexcept(noexcept(Iterator(nullptr))) { return Iterator(nullptr); }
  ConstIterator end() const noexcept(noexcept(Iterator(nullptr))) { return ConstIterator(nullptr); }
};

} // namespace linkedlist
