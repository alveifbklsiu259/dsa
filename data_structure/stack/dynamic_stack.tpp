#include "dynamic_stack.hpp"

#define DYNAMIC_STACK_TEMPLATE template <typename T>

namespace stack {

DYNAMIC_STACK_TEMPLATE template <typename... Args> T& DynamicStack<T>::emplace(Args&&... args) {
  return m_data.emplaceBack(std::forward<Args>(args)...);
}

DYNAMIC_STACK_TEMPLATE void DynamicStack<T>::push(const T& val) { m_data.pushBack(val); }

DYNAMIC_STACK_TEMPLATE void DynamicStack<T>::push(T&& val) { m_data.pushBack(std::move(val)); }

DYNAMIC_STACK_TEMPLATE void DynamicStack<T>::pop() {
  if (empty()) throw std::underflow_error("Stack is empty");
  m_data.erase(m_data.cend() - 1);
}

DYNAMIC_STACK_TEMPLATE const T& DynamicStack<T>::top() const {
  if (empty()) throw std::underflow_error("Stack is empty");
  return m_data[size() - 1];
}

DYNAMIC_STACK_TEMPLATE void DynamicStack<T>::clear() { m_data.clear(); }

DYNAMIC_STACK_TEMPLATE bool DynamicStack<T>::empty() const noexcept { return size() == 0; }

DYNAMIC_STACK_TEMPLATE size_t DynamicStack<T>::size() const noexcept { return m_data.size(); }

} // namespace stack
