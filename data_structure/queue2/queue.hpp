#pragma once
#include <array>
#include <cstddef>

namespace queue {
template <typename T, size_t N> class Queue {
  template <typename, size_t> friend class Queue;

private:
  alignas(T) std::array<std::byte, sizeof(T) * N> m_data;
  size_t m_capacity = N;
  size_t m_length = 0;
  size_t m_head = 0;
  size_t m_tail = 0;

  template <size_t M> void copy(const Queue<T, M>& other);
  template <size_t M> void move(Queue<T, M>&& other);

  T* getBuffer(size_t i) noexcept;
  const T* getBuffer(size_t i) const noexcept;

public:
  Queue() = default;
  Queue(const Queue<T, N>& other);
  template <size_t M> Queue(const Queue<T, M>& other);

  Queue<T, N>& operator=(const Queue<T, N>& other);
  template <size_t M> Queue<T, N>& operator=(const Queue<T, M>& other);

  Queue(Queue<T, N>&& other) noexcept;
  template <size_t M> Queue(Queue<T, M>&& other) noexcept;

  Queue<T, N>& operator=(Queue<T, N>&& other) noexcept;
  template <size_t M> Queue<T, N>& operator=(Queue<T, M>&& other) noexcept;

  ~Queue() noexcept;

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

#include "./queue.tpp"
