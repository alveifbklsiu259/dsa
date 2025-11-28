#pragma once
#include <array>
#include <cstddef>

namespace queue {
template <typename T, size_t N> class StaticQueue {
  template <typename, size_t> friend class Queue;

private:
  alignas(T) std::array<std::byte, sizeof(T) * N> m_data;
  size_t m_capacity = N;
  size_t m_length = 0;
  size_t m_head = 0;
  size_t m_tail = 0;

  template <size_t M> void copy(const StaticQueue<T, M>& other);
  template <size_t M> void move(StaticQueue<T, M>&& other) noexcept(std::is_nothrow_move_constructible_v<T>);

  T* getBuffer(size_t i) noexcept;
  const T* getBuffer(size_t i) const noexcept;

public:
  StaticQueue() = default;
  StaticQueue(const StaticQueue<T, N>& other);
  template <size_t M> StaticQueue(const StaticQueue<T, M>& other);

  StaticQueue<T, N>& operator=(const StaticQueue<T, N>& other);
  template <size_t M> StaticQueue<T, N>& operator=(const StaticQueue<T, M>& other);

  StaticQueue(StaticQueue<T, N>&& other) noexcept(std::is_nothrow_move_constructible_v<T>);
  template <size_t M>
  StaticQueue(StaticQueue<T, M>&& other) noexcept(std::is_nothrow_move_constructible_v<T>);

  StaticQueue<T, N>& operator=(StaticQueue<T, N>&& other) noexcept(std::is_nothrow_move_constructible_v<T>);
  template <size_t M>
  StaticQueue<T, N>& operator=(StaticQueue<T, M>&& other) noexcept(std::is_nothrow_move_constructible_v<T>);

  ~StaticQueue() noexcept;

  void push(const T& val);
  void push(T&& val);
  template <typename... Args> T& emplace(Args&&... args);
  T pop();
  const T& front() const;
  const T& back() const;
  void clear() noexcept;
  [[nodiscard]] size_t getSize() const noexcept;
  [[nodiscard]] size_t getCapacity() const noexcept;
  [[nodiscard]] bool isFull() const noexcept;
  [[nodiscard]] bool isEmpty() const noexcept;
};
} // namespace queue

#include "./static_queue.tpp"
