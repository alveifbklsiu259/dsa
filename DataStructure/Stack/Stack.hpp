#pragma once
#include <cstddef>
#include <stdexcept>

namespace Stack {

template <typename T, size_t N> class Stack {
  template <typename, size_t> friend class Stack;

private:
  size_t capacity = N;
  size_t length = 0;
  T data[N];

  template <size_t M> void copy(const Stack<T, M> &other);

  template <size_t M> void move(Stack<T, M> &&other) noexcept;

  void init();

public:
  Stack();

  template <size_t M> Stack(const Stack<T, M> &other);
  Stack(const Stack<T, N> &other);

  template <size_t M> Stack &operator=(const Stack<T, M> &other);
  Stack &operator=(const Stack<T, N> &other);

  template <size_t M> Stack(Stack<T, M> &&other) noexcept;
  Stack(Stack<T, N> &&other) noexcept;

  template <size_t M> Stack &operator=(Stack<T, M> &&other) noexcept;
  Stack &operator=(Stack<T, N> &&other) noexcept;

  ~Stack() = default;

  void push(const T &value);
  void push(T &&value);
  T pop();
  const T &peek() const;
  bool isEmpty() const noexcept;
  bool isFull() const noexcept;
  size_t getCapacity() const noexcept;
  size_t getSize() const noexcept;
};

} // namespace Stack

#include "Stack.tpp"