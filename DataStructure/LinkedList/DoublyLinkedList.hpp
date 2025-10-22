#pragma once
#include "./Exception.hpp"
#include "./Iterator.hpp"
#include "./LinkedListBase.hpp"
#include "./Node.hpp"

namespace linkedlist {
template <typename T> class DoublyLinkedList : public LinkedListBase<DoublyLinkNode<T>> {
private:
  DoublyLinkNode<T>* tail = nullptr;

public:
  using iterator = BidirectionalIterator<DoublyLinkNode<T>>;
  using constIterator = BidirectionalIterator<const DoublyLinkNode<T>>;
  using reverseIterator = ReverseIterator<DoublyLinkNode<T>>;
  using constReverseIterator = ReverseIterator<DoublyLinkNode<T>>;

  void pushFront(const T& value) override {
    DoublyLinkNode<T>* newNode = new DoublyLinkNode<T>(value, this->head);
    if (this->head) { this->head->prev = newNode; }
    if (!tail) tail = newNode;

    this->head = newNode;
    this->size++;
  }

  void pushFront(T&& value) override {
    DoublyLinkNode<T>* newNode = new DoublyLinkNode<T>(std::move(value), this->head);
    if (this->head) { this->head->prev = newNode; }
    if (!tail) tail = newNode;

    this->head = newNode;
    this->size++;
  }

  template <typename... Args> T& emplaceFront(Args&&... args) {
    DoublyLinkNode<T>* newNode = new DoublyLinkNode<T>(T(std::forward<Args>(args)...), this->head);
    if (this->head) this->head->prev = newNode;
    if (!this->tail) this->tail = newNode;

    this->head = newNode;
    this->size++;
    return newNode->value;
  }

  void pushBack(const T& value) {
    DoublyLinkNode<T>* newNode = new DoublyLinkNode<T>(value, nullptr, tail);
    if (tail) {
      tail->next = newNode;
    } else {
      this->head = newNode;
    }

    tail = newNode;
    this->size++;
  }

  void pushBack(T&& value) {
    DoublyLinkNode<T>* newNode = new DoublyLinkNode<T>(value, nullptr, tail);
    if (tail) {
      tail->next = newNode;
    } else {
      this->head = newNode;
    }

    tail = newNode;
    this->size++;
  }

  template <typename... Args> T& emplaceBack(Args&&... args) {
    DoublyLinkNode<T>* newNode =
        new DoublyLinkNode<T>(T(std::forward<Args>(args)...), nullptr, tail);
    if (tail) {
      tail->next = newNode;
    } else {
      this->head = newNode;
    }

    tail = newNode;
    this->size++;
    return newNode->value;
  }

  void popBack() {
    if (!tail) throw EmptyListException();

    DoublyLinkNode<T>* temp = tail;
    tail = tail->prev;
    if (tail) {
      tail->next = nullptr;
    } else {
      this->head = nullptr;
    }

    delete temp;
    this->size--;
  }

  T& back() {
    if (!tail) throw EmptyListException();
    return tail->value;
  }

  const T& back() const {
    if (!tail) throw EmptyListException();
    return tail->value;
  }

  void printReverse() const {
    DoublyLinkNode<T>* current = tail;
    while (current) {
      std::cout << current->value << " <-- ";
      current = current->prev;
    }
    std::cout << "nullptr" << std::endl;
  }

  void reverse() override {
    DoublyLinkNode<T>* current = this->head;
    tail = this->head;

    while (current) {
      std::swap(current->prev, current->next);
      if (!current->prev) this->head = current;
      current = current->prev;
    }
  }

  iterator begin() { return iterator(this->head); }
  constIterator begin() const { return constIterator(this->head); }

  iterator end() { return iterator(tail); }
  constIterator end() const { return constIterator(tail); }

  reverseIterator rbegin() { return reverseIterator(tail); }
  constReverseIterator rbegin() const { return constReverseIterator(tail); }

  reverseIterator rend() { return reverseIterator(nullptr); }
  constReverseIterator rend() const { return constReverseIterator(nullptr); }
};
} // namespace linkedlist
