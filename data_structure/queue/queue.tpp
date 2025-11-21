#include "./queue.hpp"
#include <cstddef>
#include <memory>
#include <stdexcept>

#define QUEUE_TEMPLATE template <typename T, size_t N>

namespace queue {

QUEUE_TEMPLATE T* Queue<T, N>::getBuffer(size_t i) noexcept {
  return reinterpret_cast<T*>(m_data.data()) + i; // NOLINT
}

QUEUE_TEMPLATE const T* Queue<T, N>::getBuffer(size_t i) const noexcept {
  return reinterpret_cast<const T*>(m_data.data()) + i; // NOLINT
}

QUEUE_TEMPLATE template <size_t M>
void Queue<T, N>::move(Queue<T, M>&& other) noexcept(std::is_nothrow_move_constructible_v<T>) { // NOLINT
  static_assert(M <= N, "Source queue too large for target queue");
  m_length = other.m_length;
  m_head = 0;
  m_tail = m_length;
  size_t i = 0;

  try {
    while (i < m_length) {
      std::construct_at((getBuffer(i)), std::move(*other.getBuffer((i + other.m_head) % other.m_capacity)));
      i++;
    }
  } catch (...) {
    for (size_t j = 0; j < i; j++) std::destroy_at(getBuffer(j));
    m_length = 0;
    throw;
  }
  other.m_length = 0;
  other.m_head = 0;
  other.m_tail = 0;
}

QUEUE_TEMPLATE template <size_t M> void Queue<T, N>::copy(const Queue<T, M>& other) {
  static_assert(M <= N, "Source queue too large for target queue");
  m_length = other.m_length;
  m_head = 0;
  m_tail = m_length;
  size_t i = 0;
  try {
    while (i < m_length) {
      std::construct_at(getBuffer(i), *other.getBuffer((i + other.m_head) % other.m_capacity));
      i++;
    }
  } catch (...) {
    for (size_t j = 0; j < i; j++) std::destroy_at(getBuffer(i));
    m_length = 0;
    throw;
  }
}

QUEUE_TEMPLATE Queue<T, N>::Queue(const Queue<T, N>& other) { copy(other); }
QUEUE_TEMPLATE template <size_t M> Queue<T, N>::Queue(const Queue<T, M>& other) { copy(other); }

QUEUE_TEMPLATE Queue<T, N>& Queue<T, N>::operator=(const Queue<T, N>& other) {
  if (&other == this) return *this;
  copy(other);
  return *this;
}

QUEUE_TEMPLATE template <size_t M> Queue<T, N>& Queue<T, N>::operator=(const Queue<T, M>& other) {
  copy(other);
  return *this;
}

QUEUE_TEMPLATE Queue<T, N>::Queue(Queue<T, N>&& other) noexcept(std::is_nothrow_move_constructible_v<T>) {
  move(std::move(other));
}

QUEUE_TEMPLATE template <size_t M>
Queue<T, N>::Queue(Queue<T, M>&& other) noexcept(std::is_nothrow_move_constructible_v<T>) {
  move(std::move(other));
}

QUEUE_TEMPLATE Queue<T, N>&
Queue<T, N>::operator=(Queue<T, N>&& other) noexcept(std::is_nothrow_move_constructible_v<T>) {
  if (&other == this) return *this;
  move(std::move(other));
  return *this;
}

QUEUE_TEMPLATE template <size_t M>
Queue<T, N>& Queue<T, N>::operator=(Queue<T, M>&& other) noexcept(std::is_nothrow_move_constructible_v<T>) {
  move(std::move(other));
  return *this;
}

QUEUE_TEMPLATE Queue<T, N>::~Queue() noexcept { clear(); }

QUEUE_TEMPLATE void Queue<T, N>::push(const T& val) {
  if (isFull()) throw std::overflow_error("Queue is full");
  std::construct_at(getBuffer(m_tail), val);
  m_length++;
  m_tail = (m_tail + 1) % m_capacity;
}

QUEUE_TEMPLATE void Queue<T, N>::push(T&& val) {
  if (isFull()) throw std::overflow_error("Queue is full");
  std::construct_at(getBuffer(m_tail), std::move(val));
  m_length++;
  m_tail = (m_tail + 1) % m_capacity;
}

QUEUE_TEMPLATE template <typename... Args> T& Queue<T, N>::emplace(Args&&... args) {
  if (isFull()) throw std::overflow_error("Queue is full");
  T* newDataPtr = std::construct_at(getBuffer(m_tail), std::forward<Args>(args)...); // NOLINT
  m_tail = (m_tail + 1) % m_capacity;
  m_length++;
  return *newDataPtr;
}

QUEUE_TEMPLATE T Queue<T, N>::pop() {
  if (isEmpty()) throw std::underflow_error("Queue is empty");

  constexpr bool canSafelyMode = std::is_nothrow_move_constructible_v<T> || !std::is_copy_constructible_v<T>;
  T data = canSafelyMode ? std::move(*getBuffer(m_head)) : *getBuffer(m_head);
  m_length--;
  std::destroy_at(getBuffer(m_head));
  m_head = (m_head + 1) % m_capacity;
  return data;
}

QUEUE_TEMPLATE const T& Queue<T, N>::front() const {
  if (isEmpty()) throw std::underflow_error("Queue is empty");
  return *getBuffer(m_head);
}

QUEUE_TEMPLATE const T& Queue<T, N>::back() const {
  if (isEmpty()) throw std::underflow_error("Queue is empty");
  return *getBuffer(m_tail);
}

QUEUE_TEMPLATE void Queue<T, N>::clear() noexcept {
  while (!isEmpty()) {
    std::destroy_at(getBuffer(m_head));
    m_head = (m_head + 1) % m_capacity;
    m_length--;
  }
  m_head = m_tail = 0;
}

QUEUE_TEMPLATE size_t Queue<T, N>::getSize() const noexcept { return m_length; }
QUEUE_TEMPLATE size_t Queue<T, N>::getCapacity() const noexcept { return m_capacity; }
QUEUE_TEMPLATE bool Queue<T, N>::isFull() const noexcept { return m_length == m_capacity; }
QUEUE_TEMPLATE bool Queue<T, N>::isEmpty() const noexcept { return m_length == 0; }

} // namespace queue
