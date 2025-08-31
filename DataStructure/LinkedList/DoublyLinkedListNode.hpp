#pragma once

namespace LinkedList {
template <typename T> class DoublyLinkedListNode {
public:
  T value;
  DoublyLinkedListNode<T> *next;
  DoublyLinkedListNode<T> *prev;

  DoublyLinkedListNode(const T &value, DoublyLinkedListNode<T> *next = nullptr,
                       DoublyLinkedListNode<T> *prev = nullptr)
      : value(value), next(next), prev(prev) {}

  DoublyLinkedListNode(const DoublyLinkedListNode<T> &) = delete;
  DoublyLinkedListNode<T> &operator=(DoublyLinkedListNode<T> &o) = delete;
  DoublyLinkedListNode(DoublyLinkedListNode<T> &&) = delete;
  DoublyLinkedListNode<T> &operator=(DoublyLinkedListNode<T> &&) = delete;

  ~DoublyLinkedListNode() = default;
};

} // namespace LinkedList
