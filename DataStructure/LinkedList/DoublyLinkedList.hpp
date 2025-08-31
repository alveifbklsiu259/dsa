#pragma once
#include "./DoublyLinkedListNode.hpp"
#include "./Exception.hpp"
#include "./Iterator.hpp"
#include "./Node.hpp"
#include <initializer_list>
#include <iostream>
#include <utility>
#include <vector>

namespace LinkedList {
template <typename T> class DoublyLinkedList {
private:
  int size = 0;
  DoublyLinkedListNode<T> *head = nullptr;

  void release() noexcept {
    while (head) {
      DoublyLinkedListNode<T> *temp = head;
      head = head->next;
      delete temp;
    }
  }

  void deepCopy(const DoublyLinkedList<T> &other) {
    if (!other.head) {
      head = nullptr;
      return;
    }

    size = other.size;
    head = new DoublyLinkedListNode<T>(other.head->value);
    DoublyLinkedListNode<T> *current = head;
    DoublyLinkedListNode<T> *otherCurrent = other.head->next;

    while (otherCurrent) {
      current->next = new DoublyLinkedListNode<T>(otherCurrent->value);
      current = current->next;
      otherCurrent = otherCurrent->next;
    }
  }

  void move(DoublyLinkedList<T> &other) noexcept {
    size = other.size;
    head = std::exchange(other.head, nullptr);
  }

public:
  DoublyLinkedList() = default;
  DoublyLinkedList(std::initializer_list<T> init) {
    for (const T &value : init)
      pushFront(value);
  }

  DoublyLinkedList(const DoublyLinkedList<T> &other) { deepCopy(other); }

  DoublyLinkedList<T> &operator=(const DoublyLinkedList<T> &other) {
    if (this == &other)
      return *this;

    release();
    deepCopy(other);
    return *this;
  }

  DoublyLinkedList(DoublyLinkedList<T> &&other) noexcept { move(other); }

  DoublyLinkedList<T> &operator=(DoublyLinkedList<T> &&other) noexcept {
    if (this == &other)
      return *this;
    release();
    move(other);
    return *this;
  }

  ~DoublyLinkedList() { release(); }

  int getSize() { return size; }

  void pushFront(const T &value) {
    DoublyLinkedListNode<T> *newNode =
        new DoublyLinkedListNode<T>(value, head, nullptr);
    if (head)
      head->prev = newNode;
    head = newNode;
    size += 1;
  }

  void popFront() {
    if (!head)
      throw EmptyListException();

    size -= 1;
    DoublyLinkedListNode<T> *temp = head;
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
    }
  }

  void print() {
    DoublyLinkedListNode<T> *current = head;
    while (current) {
      std::cout << current->value << " --> ";
      current = current->next;
    }

    std::cout << "nullptr" << std::endl;
  }

  Iterator<T> begin() { return Iterator<T>((Node<T> *)head); }
  Iterator<T> begin() const { return Iterator<T>((Node<T> *)head); }

  Iterator<T> end() { return Iterator<T>(nullptr); }
  Iterator<T> end() const { return Iterator<T>(nullptr); }
};
} // namespace LinkedList

// i think maybe a base LinkedList class then extends that by singly and doubly,
// because they're pretty much the same, the only difference is the Node they
// use and additional method for doubly

// I think we should also have a base node and extend (can we do that??)