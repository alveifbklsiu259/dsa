#pragma once
#include "../array/dynamic_array.hpp"

namespace queue {
template <typename T> class DynamicQueue {
private:
  array::DynamicArray<T> m_data;
  size_t m_length = 0;
  size_t m_head = 0;
  size_t m_tail = 0;

  void resize();

public:
  template <typename... Args> T& emplace(Args&&... args);
  void push(const T& val);
  void push(T&& val);

  void pop();

  const T& front() const;
  const T& back() const;

  void clear();

  [[nodiscard]] size_t getSize() const;
  [[nodiscard]] bool isEmpty() const;
  [[nodiscard]] bool isFull() const;
};
} // namespace queue

#include "./dynamic_queue.tpp"
