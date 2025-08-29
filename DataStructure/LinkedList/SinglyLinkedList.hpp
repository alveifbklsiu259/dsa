#pragma once

#include "./Exception.hpp"
#include "./Iterator.hpp"
#include "./Node.hpp"
#include <iostream>
#include <utility>
#include <vector>

namespace LinkedList {
template <typename T> class SinglyLinkedList {

private:
  int size = 0;
  Node<T> *head = nullptr;

  void release() noexcept {
    while (head) {
      Node<T> *temp = head;
      head = head->next;
      delete temp;
    };
  }

  void deepCopy(const SinglyLinkedList &other) {
    if (!other.head) {
      head = nullptr;
      return;
    }

    size = other.size;
    head = new Node<T>(other.head->value);
    Node<T> *current = head;
    Node<T> *otherCurrent = other.head->next;

    while (otherCurrent) {
      current->next = new Node<T>(otherCurrent->value);
      current = current->next;
      otherCurrent = otherCurrent->next;
    }
  }

  void move(SinglyLinkedList &other) noexcept {
    size = other.size;
    head = std::exchange(other.head, nullptr);
  }

public:
  SinglyLinkedList() = default;
  SinglyLinkedList(std::initializer_list<T> init) {
    for (const T &value : init)
      pushFront(value);
  }

  SinglyLinkedList(const SinglyLinkedList &other) { deepCopy(other); }

  SinglyLinkedList &operator=(const SinglyLinkedList &other) {
    if (this == &other)
      return *this;
    release();
    deepCopy(other);
    return *this;
  }

  SinglyLinkedList(SinglyLinkedList &&other) noexcept { move(other); }

  SinglyLinkedList &operator=(SinglyLinkedList &&other) noexcept {
    if (this == &other)
      return *this;
    release();
    move(other);
    return *this;
  }

  ~SinglyLinkedList() { release(); }

  int getSize() { return size; }

  void pushFront(const T &value) {
    head = new Node<T>(value, head);
    size += 1;
  }

  void popFront() {
    if (!head)
      throw EmptyListException();

    size -= 1;
    Node<T> *temp = head;
    head = head->next;
    delete temp;
  }

  T &front() {
    if (!head)
      throw EmptyListException();

    return head->value;
  }

  const T &front() const {
    if (!head)
      throw EmptyListException();

    return head->value;
  }

  bool isEmpty() { return head == nullptr; }

  void fromVector(const std::vector<T> &vec) {
    for (auto it = vec.rbegin(); it != vec.rend(); it++) {
      pushFront(*it);
    };
  }

  void print() {
    Node<T> *current = head;
    while (current) {
      std::cout << current->value << " --> ";
      current = current->next;
    }

    std::cout << "nullptr" << std::endl;
  }

  Iterator<T> begin() { return Iterator<T>(head); }
  Iterator<T> begin() const { return Iterator<T>(head); }

  Iterator<T> end() { return Iterator<T>(nullptr); }
  Iterator<T> end() const { return Iterator<T>(nullptr); }
};
} // namespace LinkedList
