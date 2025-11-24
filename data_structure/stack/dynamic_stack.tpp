#include "dynamic_stack.hpp"

#define DYNAMIC_STACK_TEMPLATE template <typename T>

namespace stack {

DYNAMIC_STACK_TEMPLATE void DynamicStack<T>::push(const T& val) { m_data.pushBack(val); }

DYNAMIC_STACK_TEMPLATE void DynamicStack<T>::push(T&& val) { m_data.pushBack(std::move(val)); }

DYNAMIC_STACK_TEMPLATE T DynamicStack<T>::pop() {}
} // namespace stack
