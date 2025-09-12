
#pragma once
#include <cstddef>
#include <stdexcept>
namespace Queue {
template <typename T, size_t N> class Queue {
  template <typename, size_t> friend class Queue;

private:
  T m_data[N];
  size_t m_capacity = N;
  size_t m_length = 0;
  size_t m_head = 0;
  size_t m_tail = 0;

  template <size_t M> void copy(const Queue<T, M> &other);
  template <size_t M> void move(Queue<T, M> &&other);
  void init();

public:
  Queue();
  Queue(const Queue<T, N> &other);
  template <size_t M> Queue(const Queue<T, M> &other);

  Queue<T, N> &operator=(const Queue<T, N> &other);
  template <size_t M> Queue<T, N> &operator=(const Queue<T, M> &other);

  Queue(Queue<T, N> &&other) noexcept;
  template <size_t M> Queue(Queue<T, M> &&other) noexcept;

  Queue<T, N> &operator=(Queue<T, N> &&other) noexcept;
  template <size_t M> Queue<T, N> &operator=(Queue<T, M> &&other) noexcept;

  ~Queue() = default;

  void enqueue(const T &val);
  void enqueue(T &&val);
  T dequeue();
  const T &peek() const;
  size_t getSize() const noexcept;
  size_t getCapacity() const noexcept;
  bool isFull() const noexcept;
  bool isEmpty() const noexcept;
};
} // namespace Queue

#include "./Queue.tpp"
