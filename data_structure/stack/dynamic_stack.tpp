#include "dynamic_stack.hpp"

#define DYNAMIC_STACK_TEMPLATE template <typename T>

namespace stack {

DYNAMIC_STACK_TEMPLATE template <typename... Args> T& DynamicStack<T>::emplace(Args&&... args) {
  return m_data.emplaceBack(std::forward<Args>(args)...);
}

DYNAMIC_STACK_TEMPLATE void DynamicStack<T>::push(const T& val) { m_data.pushBack(val); }

DYNAMIC_STACK_TEMPLATE void DynamicStack<T>::push(T&& val) { m_data.pushBack(std::move(val)); }

DYNAMIC_STACK_TEMPLATE void DynamicStack<T>::pop() {
  if (isEmpty()) throw std::underflow_error("Stack is empty");
  m_data.erase(m_data.cend() - 1);
}

DYNAMIC_STACK_TEMPLATE const T& DynamicStack<T>::top() const {
  if (isEmpty()) throw std::underflow_error("Stack is empty");
  return m_data[getSize() - 1];
}

DYNAMIC_STACK_TEMPLATE void DynamicStack<T>::clear() { m_data.clear(); }

DYNAMIC_STACK_TEMPLATE bool DynamicStack<T>::isEmpty() const noexcept { return getSize() == 0; }

DYNAMIC_STACK_TEMPLATE size_t DynamicStack<T>::getSize() const noexcept { return m_data.getSize(); }

} // namespace stack
