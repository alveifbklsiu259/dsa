#pragma once
#include "../array/dynamic_array.hpp"

namespace stack {

template <typename T> class DynamicStack {
private:
  array::DynamicArray<T> m_data;

public:
  template <typename... Args> T& emplace(Args&&... args);
  void push(const T& val);
  void push(T&& val);

  void pop();

  const T& top() const;

  void clear();

  [[nodiscard]] bool empty() const noexcept;
  [[nodiscard]] size_t size() const noexcept;
};
} // namespace stack

#include "./dynamic_stack.tpp"
