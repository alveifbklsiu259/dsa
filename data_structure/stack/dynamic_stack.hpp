#pragma once
#include "../array/dynamic_array.hpp"

namespace stack {

template <typename T> class DynamicStack {
private:
  array::DynamicArray<T> m_data;

public:
  DynamicStack() = default;
  ~DynamicStack() = default;

  void push(const T& val);
  void push(T&& val);

  T pop();

  const T& top() const;
};
} // namespace stack

#include "./dynamic_stack.hpp"
