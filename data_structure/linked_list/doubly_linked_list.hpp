#pragma once
#include "./exception.hpp"
#include "./iterator.hpp"
#include "./linked_list_base.hpp"
#include "./node.hpp"
#include <gsl/gsl>

namespace linkedlist {
template <typename T> class DoublyLinkedList : public LinkedListBase<DoublyLinkNode<T>> {
private:
  DoublyLinkNode<T>* tail = nullptr; // NOLINT

public:
  using Iterator = BidirectionalIterator<DoublyLinkNode<T>>;
  using ConstIterator = BidirectionalIterator<const DoublyLinkNode<T>>;
  using ReverseIterator = linkedlist::ReverseIterator<DoublyLinkNode<T>>;
  using ConstReverseIterator = linkedlist::ReverseIterator<DoublyLinkNode<T>>;

  void pushFront(const T& value) override {
    gsl::owner<DoublyLinkNode<T>*> newNode = new DoublyLinkNode<T>(value, this->head);
    if (this->head) { this->head->prev = newNode; }
    if (tail == nullptr) tail = newNode;

    this->head = newNode;
    this->size++;
  }

  void pushFront(T&& value) override {
    gsl::owner<DoublyLinkNode<T>*> newNode = new DoublyLinkNode<T>(std::move(value), this->head);
    if (this->head) { this->head->prev = newNode; }
    if (tail == nullptr) tail = newNode;

    this->head = newNode;
    this->size++;
  }

  template <typename... Args> T& emplaceFront(Args&&... args) {
    gsl::owner<DoublyLinkNode<T>*> newNode =
        new DoublyLinkNode<T>(T(std::forward<Args>(args)...), this->head);
    if (this->head) this->head->prev = newNode;
    if (!this->tail) this->tail = newNode;

    this->head = newNode;
    this->size++;
    return newNode->value;
  }

  void pushBack(const T& value) {
    gsl::owner<DoublyLinkNode<T>*> newNode = new DoublyLinkNode<T>(value, nullptr, tail);
    if (tail) {
      tail->next = newNode;
    } else {
      this->head = newNode;
    }

    tail = newNode;
    this->size++;
  }

  void pushBack(T&& value) {
    gsl::owner<DoublyLinkNode<T>*> newNode = new DoublyLinkNode<T>(std::move(value), nullptr, tail);
    if (tail) {
      tail->next = newNode;
    } else {
      this->head = newNode;
    }

    tail = newNode;
    this->size++;
  }

  template <typename... Args> T& emplaceBack(Args&&... args) {
    gsl::owner<DoublyLinkNode<T>*> newNode =
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
    if (tail == nullptr) throw EmptyListException();

    DoublyLinkNode<T>* temp = tail;
    tail = tail->prev;
    if (tail) {
      tail->next = nullptr;
    } else {
      this->head = nullptr;
    }

    delete temp; // NOLINT
    this->size--;
  }

  T& back() {
    if (tail == nullptr) throw EmptyListException();
    return tail->value;
  }

  const T& back() const {
    if (tail == nullptr) throw EmptyListException();
    return tail->value;
  }

  void printReverse() const {
    DoublyLinkNode<T>* current = tail;
    while (current) {
      std::cout << current->value << " <-- ";
      current = current->prev;
    }
    std::cout << "nullptr" << '\n';
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

  Iterator begin() noexcept { return Iterator(this->head); }
  ConstIterator begin() const noexcept { return ConstIterator(this->head); }

  Iterator end() noexcept { return Iterator(tail); }
  ConstIterator end() const noexcept { return ConstIterator(tail); }

  ReverseIterator rbegin() noexcept { return ReverseIterator(tail); }
  ConstReverseIterator rbegin() const noexcept { return ConstReverseIterator(tail); }

  ReverseIterator rend() noexcept { return ReverseIterator(nullptr); }
  ConstReverseIterator rend() const noexcept { return ConstReverseIterator(nullptr); }
};
} // namespace linkedlist
