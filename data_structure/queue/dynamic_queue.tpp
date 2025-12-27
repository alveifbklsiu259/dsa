#include "../array/dynamic_array.hpp"
#include "./dynamic_queue.hpp"

#define DYNAMIC_QUEUE_TEMPLATE template <typename T>

namespace queue {

DYNAMIC_QUEUE_TEMPLATE void DynamicQueue<T>::resize() {
  size_t newCapacity = m_data.getCapacity() * 2;
  array::DynamicArray<T> newData;
  newData.reserve(newCapacity);

  constexpr bool preferMove = std::is_nothrow_move_constructible_v<T> || !std::is_copy_constructible_v<T>;

  for (size_t i = 0; i < m_length; i++) {
    if constexpr (preferMove) {
      newData.pushBack(std::move(m_data[(i + m_head) % m_data.getCapacity()]));
    } else {
      newData.pushBack(m_data[(i + m_head) % m_data.getCapacity()]);
    }
  }
  m_data = preferMove ? std::move(newData) : newData;
  m_head = 0;
  m_tail = m_length;
}

DYNAMIC_QUEUE_TEMPLATE template <typename... Args> T& DynamicQueue<T>::emplace(Args&&... args) {
  if (isFull()) resize();
  // not true emplace, array::DynamicArray does not have an emplace-at-index method.
  if (m_tail < m_head) {
    m_data[m_tail] = T(std::forward<Args>(args)...);
  } else {
    m_data.pushBack(T(std::forward<Args>(args)...));
  }

  m_length++;
  size_t oldTail = m_tail;
  m_tail = (m_tail + 1) % m_data.getCapacity();
  return m_data[oldTail];
}

DYNAMIC_QUEUE_TEMPLATE void DynamicQueue<T>::push(const T& val) { emplace(val); }
DYNAMIC_QUEUE_TEMPLATE void DynamicQueue<T>::push(T&& val) { emplace(std::move(val)); }

DYNAMIC_QUEUE_TEMPLATE void DynamicQueue<T>::pop() {
  if (isEmpty()) throw std::underflow_error("Queue is empty");
  m_length--;
  m_head = (m_head + 1) % m_data.getCapacity();
}

DYNAMIC_QUEUE_TEMPLATE const T& DynamicQueue<T>::front() const {
  if (isEmpty()) throw std::underflow_error("Queue is empty");
  return m_data[m_head];
}

DYNAMIC_QUEUE_TEMPLATE const T& DynamicQueue<T>::back() const {
  if (isEmpty()) throw std::underflow_error("Queue is empty");
  return m_data[(m_tail + m_data.getCapacity() - 1) % m_data.getCapacity()];
}

DYNAMIC_QUEUE_TEMPLATE void DynamicQueue<T>::clear() {
  m_data.clear();
  m_head = 0;
  m_tail = 0;
  m_length = 0;
}

DYNAMIC_QUEUE_TEMPLATE size_t DynamicQueue<T>::getSize() const { return m_length; }
DYNAMIC_QUEUE_TEMPLATE bool DynamicQueue<T>::isEmpty() const { return m_length == 0; }
DYNAMIC_QUEUE_TEMPLATE bool DynamicQueue<T>::isFull() const { return m_data.getCapacity() == m_length; }
} // namespace queue
