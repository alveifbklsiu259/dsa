#pragma once
#include <array>
#include <cstddef>

namespace stack {

template <typename T, size_t N> class StaticStack {
  template <typename, size_t> friend class Stack;

private:
  size_t m_capacity = N;
  size_t m_length = 0;
  alignas(T) std::array<std::byte, sizeof(T) * N> m_data;

  T* getBuffer(size_t i) noexcept;
  const T* getBuffer(size_t i) const noexcept;

  template <size_t M> void copy(const StaticStack<T, M>& other);

  template <size_t M> void move(StaticStack<T, M>&& other) noexcept(std::is_nothrow_move_constructible_v<T>);

  template <size_t M> void swap(StaticStack<T, M>& other) noexcept;

public:
  StaticStack() = default;

  template <size_t M> StaticStack(const StaticStack<T, M>& other);
  StaticStack(const StaticStack<T, N>& other);

  template <size_t M> StaticStack& operator=(const StaticStack<T, M>& other);
  StaticStack& operator=(const StaticStack<T, N>& other);

  template <size_t M>
  StaticStack(StaticStack<T, M>&& other) noexcept(std::is_nothrow_move_constructible_v<T>);
  StaticStack(StaticStack<T, N>&& other) noexcept(std::is_nothrow_move_constructible_v<T>);

  template <size_t M>
  StaticStack& operator=(StaticStack<T, M>&& other) noexcept(std::is_nothrow_move_constructible_v<T>);
  StaticStack& operator=(StaticStack<T, N>&& other) noexcept(std::is_nothrow_move_constructible_v<T>);

  ~StaticStack() noexcept;

  void push(const T& value);
  void push(T&& value);

  template <typename... Args> T& emplace(Args&&... args);

  T pop();

  const T& top() const;

  void clear() noexcept;

  [[nodiscard]] bool empty() const noexcept;
  [[nodiscard]] bool full() const noexcept;
  [[nodiscard]] size_t getCapacity() const noexcept;
  [[nodiscard]] size_t size() const noexcept;
};

} // namespace stack

#include "./static_stack.tpp"
