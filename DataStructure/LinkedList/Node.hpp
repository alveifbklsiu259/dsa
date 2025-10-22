#pragma once
#include <utility>

namespace linkedlist {
template <typename T> class NodeBase {
public:
  T value;
  explicit NodeBase(const T& value) : value(value) {};
  explicit NodeBase(T&& value) : value(std::move(value)) {};

  template <typename... Args> NodeBase(Args&&... args) : value(std::forward<Args>(args)...) {}
  NodeBase(const NodeBase<T>&) = delete;
  NodeBase<T>& operator=(const NodeBase<T>&) = delete;
  NodeBase(NodeBase<T>&& other) = delete;
  NodeBase<T>& operator=(NodeBase<T>&&) = delete;
  ~NodeBase() = default;
};

template <typename T> class SinglyLinkNode : public NodeBase<T> {
public:
  SinglyLinkNode<T>* next = nullptr;
  explicit SinglyLinkNode(const T& value, SinglyLinkNode<T>* next = nullptr)
      : NodeBase<T>(value), next(next) {}
  explicit SinglyLinkNode(T&& value, SinglyLinkNode<T>* next = nullptr)
      : NodeBase<T>(std::move(value)), next(next) {}

  template <typename... Args>
  explicit SinglyLinkNode(SinglyLinkNode<T>* next = nullptr, Args&&... args)
      : NodeBase<T>(std::forward<Args>(args)...), next(next) {}
};

template <typename T> class DoublyLinkNode : public NodeBase<T> {
public:
  DoublyLinkNode<T>* next = nullptr;
  DoublyLinkNode<T>* prev = nullptr;
  explicit DoublyLinkNode(
      const T& value,
      DoublyLinkNode<T>* next = nullptr,
      DoublyLinkNode<T>* prev = nullptr
  )
      : NodeBase<T>(value), next(next), prev(prev) {}
  explicit DoublyLinkNode(
      T&& value,
      DoublyLinkNode<T>* next = nullptr,
      DoublyLinkNode<T>* prev = nullptr
  )
      : NodeBase<T>(std::move(value)), next(next), prev(prev) {}
};

} // namespace linkedlist
