#pragma once
#include "./Exception.hpp"
#include "./Iterator.hpp"
#include "./LinkedListBase.hpp"
#include "./Node.hpp"

namespace LinkedList {
template <typename T>
class DoublyLinkedList : public LinkedListBase<T, DoublyLinkNode<T>> {
private:
  DoublyLinkNode<T> *tail = nullptr;

public:
  void pushFront(const T &value) override {
    DoublyLinkNode<T> *newNode = new DoublyLinkNode<T>(value, this->head);
    if (this->head) {
      this->head->prev = newNode;
    }

    if (!tail)
      tail = newNode;

    this->head = newNode;
    this->size++;
  }

  void pushBack(const T &value) {
    DoublyLinkNode<T> *newNode = new DoublyLinkNode<T>(value, nullptr, tail);
    if (tail) {
      tail->next = newNode;
    } else {
      this->head = newNode;
    }

    tail = newNode;
    this->size++;
  }

  void popBack() {
    if (!tail)
      throw EmptyListException();

    DoublyLinkNode<T> *temp = tail;
    tail = tail->prev;
    if (tail) {
      tail->next = nullptr;
    } else {
      this->head = nullptr;
    }

    delete temp;
    this->size--;
  }

  T &back() {
    if (!tail)
      throw EmptyListException();
    return tail->value;
  }

  const T &back() const {
    if (!tail)
      throw EmptyListException();
    return tail->value;
  }

  void printReverse() const {
    DoublyLinkNode<T> *current = tail;
    while (current) {
      std::cout << current->value << " <-- ";
      current = current->prev;
    }
    std::cout << "nullptr" << std::endl;
  }

  void reverse() override {
    DoublyLinkNode<T> *current = this->head;
    tail = this->head;

    while (current) {
      std::swap(current->prev, current->next);
      if (!current->prev)
        this->head = current;
      current = current->prev;
    }
  }

  ReverseIterator<DoublyLinkNode<T>> rbegin() {
    return ReverseIterator<DoublyLinkNode<T>>(tail);
  }
  ReverseIterator<DoublyLinkNode<T>> rbegin() const {
    return ReverseIterator<DoublyLinkNode<T>>(tail);
  }

  ReverseIterator<DoublyLinkNode<T>> rend() {
    return ReverseIterator<DoublyLinkNode<T>>(nullptr);
  }
  ReverseIterator<DoublyLinkNode<T>> rend() const {
    return ReverseIterator<DoublyLinkNode<T>>(nullptr);
  }
};
} // namespace LinkedList
