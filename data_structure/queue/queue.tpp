#include "./queue.hpp"
#include <cstddef>
#include <stdexcept>

#define QUEUE_TEMPLATE template <typename T, size_t N>

namespace queue {

QUEUE_TEMPLATE void Queue<T, N>::init() {
  for (int i = 0; i < m_capacity; i++) m_data[i] = T{};
  m_length = 0;
  m_head = 0;
  m_tail = 0;
}

QUEUE_TEMPLATE template <size_t M> void Queue<T, N>::move(Queue<T, M>&& other) { // NOLINT
  m_length = other.m_length;
  m_head = 0;
  m_tail = m_length % m_capacity;
  for (int i = 0; i < m_length; i++)
    m_data[i] = std::move(other.m_data[(other.m_head + i) % other.m_capacity]);
  other.m_length = 0;
  other.m_head = 0;
  other.m_tail = 0;
}

QUEUE_TEMPLATE template <size_t M> void Queue<T, N>::copy(const Queue<T, M>& other) {
  static_assert(M <= N, "Source queue too large for target queue");
  m_length = other.m_length;
  m_head = 0;
  m_tail = m_length % m_capacity;
  for (int i = 0; i < m_length; i++)
    m_data[i] = other.m_data[(other.m_head + i) % other.m_capacity];
}

QUEUE_TEMPLATE Queue<T, N>::Queue() { init(); }

QUEUE_TEMPLATE Queue<T, N>::Queue(const Queue<T, N>& other) { copy(other); }
QUEUE_TEMPLATE template <size_t M> Queue<T, N>::Queue(const Queue<T, M>& other) { copy(other); }

QUEUE_TEMPLATE Queue<T, N>& Queue<T, N>::operator=(const Queue<T, N>& other) {
  if (&other == this) return *this;
  init();
  copy(other);
  return *this;
}

QUEUE_TEMPLATE template <size_t M> Queue<T, N>& Queue<T, N>::operator=(const Queue<T, M>& other) {
  init();
  copy(other);
  return *this;
}

QUEUE_TEMPLATE Queue<T, N>::Queue(Queue<T, N>&& other) noexcept { move(std::move(other)); }

QUEUE_TEMPLATE template <size_t M> Queue<T, N>::Queue(Queue<T, M>&& other) noexcept {
  move(std::move(other));
}

QUEUE_TEMPLATE Queue<T, N>& Queue<T, N>::operator=(Queue<T, N>&& other) noexcept {
  if (&other == this) return *this;
  if constexpr (!std::is_trivially_destructible_v<T>) init();
  move(std::move(other));
  return *this;
}

QUEUE_TEMPLATE template <size_t M>
Queue<T, N>& Queue<T, N>::operator=(Queue<T, M>&& other) noexcept {
  if constexpr (!std::is_trivially_destructible_v<T>) init();
  move(std::move(other));
  return *this;
}

QUEUE_TEMPLATE void Queue<T, N>::enqueue(const T& val) {
  if (isFull()) throw std::overflow_error("Queue is full");
  m_data[m_tail] = val;
  m_length++;
  m_tail = (m_tail + 1) % m_capacity;
}

QUEUE_TEMPLATE void Queue<T, N>::enqueue(T&& val) {
  if (isFull()) throw std::overflow_error("Queue is full");
  m_data[m_tail] = std::move(val);
  m_length++;
  m_tail = (m_tail + 1) % m_capacity;
}

QUEUE_TEMPLATE T Queue<T, N>::dequeue() {
  if (isEmpty()) throw std::underflow_error("Queue is empty");
  T data = m_data[m_head];
  m_length--;
  m_head = (m_head + 1) % m_capacity;
  return data;
}

QUEUE_TEMPLATE const T& Queue<T, N>::peek() const {
  if (isEmpty()) throw std::underflow_error("Queue is empty");
  return m_data[m_head];
}

QUEUE_TEMPLATE size_t Queue<T, N>::getSize() const noexcept { return m_length; }
QUEUE_TEMPLATE size_t Queue<T, N>::getCapacity() const noexcept { return m_capacity; }
QUEUE_TEMPLATE bool Queue<T, N>::isFull() const noexcept { return m_length == m_capacity; }
QUEUE_TEMPLATE bool Queue<T, N>::isEmpty() const noexcept { return m_length == 0; }

} // namespace queue
// - gsls
