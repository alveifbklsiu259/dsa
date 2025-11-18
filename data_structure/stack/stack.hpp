#pragma once
#include <array>
#include <cstddef>

namespace stack {

template <typename T, size_t N> class Stack {
  template <typename, size_t> friend class Stack;

private:
  size_t m_capacity = N;
  size_t m_length = 0;
  alignas(T) std::array<std::byte, sizeof(T) * N> m_data;

  T* getBuffer(size_t i) noexcept;
  const T* getBuffer(size_t i) const noexcept;

  template <size_t M> void copy(const Stack<T, M>& other);

  template <size_t M> void move(Stack<T, M>&& other) noexcept;

  template <size_t M> void swap(Stack<T, M>& other) noexcept;

public:
  Stack() = default;

  template <size_t M> Stack(const Stack<T, M>& other);
  Stack(const Stack<T, N>& other);

  template <size_t M> Stack& operator=(const Stack<T, M>& other);
  Stack& operator=(const Stack<T, N>& other);

  template <size_t M> Stack(Stack<T, M>&& other) noexcept;
  Stack(Stack<T, N>&& other) noexcept;

  template <size_t M> Stack& operator=(Stack<T, M>&& other) noexcept;
  Stack& operator=(Stack<T, N>&& other) noexcept;

  ~Stack() noexcept;

  void push(const T& value);
  void push(T&& value);

  template <typename... Args> T& emplace(Args&&... args);

  T pop();

  const T& top() const;

  void clear() noexcept;

  [[nodiscard]] bool isEmpty() const noexcept;
  [[nodiscard]] bool isFull() const noexcept;
  [[nodiscard]] size_t getCapacity() const noexcept;
  [[nodiscard]] size_t getSize() const noexcept;
};

} // namespace stack

#include "stack.tpp"
