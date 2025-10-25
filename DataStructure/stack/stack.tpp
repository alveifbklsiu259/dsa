#include "./stack.hpp"
#include <stdexcept>
#include <utility>

#define STACK_TEMPLATE template <typename T, size_t N>

namespace stack {

STACK_TEMPLATE Stack<T, N>::Stack() { init(); }

STACK_TEMPLATE
template <size_t M> void Stack<T, N>::copy(const Stack<T, M>& other) {
  static_assert(M <= N, "Source stack too large for target stack");
  m_length = other.getSize();
  for (int i = 0; i < m_length; i++) m_data[i] = other.m_data[i];
}

STACK_TEMPLATE
template <size_t M> void Stack<T, N>::move(Stack<T, M>&& other) noexcept { // NOLINT
  static_assert(M <= N, "Source stack too large for target stack");
  m_length = other.getSize();
  for (int i = 0; i < m_length; i++) m_data[i] = std::move(other.m_data[i]);
  other.m_length = 0;
}

STACK_TEMPLATE void Stack<T, N>::init() {
  for (int i = 0; i < N; i++) m_data[i] = T{};
  m_length = 0;
}

STACK_TEMPLATE
template <size_t M> Stack<T, N>::Stack(const Stack<T, M>& other) { copy(other); }

STACK_TEMPLATE Stack<T, N>::Stack(const Stack<T, N>& other) { copy(other); }

STACK_TEMPLATE
template <size_t M> Stack<T, N>& Stack<T, N>::operator=(const Stack<T, M>& other) {
  init();
  copy(other);
  return *this;
}

STACK_TEMPLATE
Stack<T, N>& Stack<T, N>::operator=(const Stack<T, N>& other) {
  if (&other == this) return *this;
  init();
  copy(other);
  return *this;
}

STACK_TEMPLATE
template <size_t M> Stack<T, N>::Stack(Stack<T, M>&& other) noexcept { move(std::move(other)); }

STACK_TEMPLATE
Stack<T, N>::Stack(Stack<T, N>&& other) noexcept { move(std::move(other)); }

STACK_TEMPLATE
template <size_t M> Stack<T, N>& Stack<T, N>::operator=(Stack<T, M>&& other) noexcept {
  if (!std::is_trivially_destructible_v<T>) init();
  move(std::move(other));
  return *this;
}

STACK_TEMPLATE
Stack<T, N>& Stack<T, N>::operator=(Stack<T, N>&& other) noexcept {
  if (&other == this) return *this;
  if (!std::is_trivially_destructible_v<T>) init();
  move(std::move(other));
  return *this;
}

STACK_TEMPLATE void Stack<T, N>::push(const T& value) {
  if (isFull()) throw std::overflow_error("Stack is full");
  m_data[m_length++] = value;
}

STACK_TEMPLATE void Stack<T, N>::push(T&& value) {
  if (isFull()) throw std::overflow_error("Stack is full");
  m_data[m_length++] = std::move(value);
}

STACK_TEMPLATE T Stack<T, N>::pop() {
  if (isEmpty()) throw std::underflow_error("Stack is empty");
  return m_data[--m_length];
}

STACK_TEMPLATE const T& Stack<T, N>::peek() const {
  if (isEmpty()) throw std::underflow_error("Stack is empty");
  return m_data[m_length - 1];
}

STACK_TEMPLATE bool Stack<T, N>::isEmpty() const noexcept { return m_length == 0; }

STACK_TEMPLATE bool Stack<T, N>::isFull() const noexcept { return m_length == m_capacity; }

STACK_TEMPLATE
size_t Stack<T, N>::getCapacity() const noexcept { return m_capacity; }

STACK_TEMPLATE size_t Stack<T, N>::getSize() const noexcept { return m_length; }

} // namespace stack
