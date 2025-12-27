#include "./static_stack.hpp"
#include <stdexcept>
#include <utility>

#define STACK_TEMPLATE template <typename T, size_t N>

namespace stack {

STACK_TEMPLATE T* StaticStack<T, N>::getBuffer(size_t i) noexcept {
  return reinterpret_cast<T*>(m_data.data()) + i; // NOLINT
}

STACK_TEMPLATE const T* StaticStack<T, N>::getBuffer(size_t i) const noexcept {
  return reinterpret_cast<const T*>(m_data.data()) + i; // NOLINT
}

STACK_TEMPLATE
template <size_t M> void StaticStack<T, N>::copy(const StaticStack<T, M>& other) {
  static_assert(M <= N, "Source stack too large for target stack");
  m_length = other.getSize();
  size_t i = 0;
  try {
    while (i < m_length) {
      new (getBuffer(i)) T(*other.getBuffer(i));
      i++;
    }
  } catch (...) {
    for (size_t j = 0; j < i; j++) getBuffer(j)->~T();
    m_length = 0;
    throw;
  }
}

STACK_TEMPLATE
template <size_t M>
void StaticStack<T, N>::move(
    StaticStack<T, M>&& other
) noexcept(std::is_nothrow_move_constructible_v<T>) { // NOLINT
  static_assert(M <= N, "Source stack too large for target stack");
  m_length = other.getSize();
  size_t i = 0;
  try {
    while (i < m_length) {
      new (getBuffer(i)) T(std::move(*other.getBuffer(i)));
      i++;
    }
  } catch (...) {
    for (size_t j = 0; j < i; j++) getBuffer(j)->~T();
    m_length = 0;
    throw;
  }
  other.clear();
}

STACK_TEMPLATE
template <size_t M> StaticStack<T, N>::StaticStack(const StaticStack<T, M>& other) { copy(other); }

STACK_TEMPLATE StaticStack<T, N>::StaticStack(const StaticStack<T, N>& other) { copy(other); }

STACK_TEMPLATE
template <size_t M> StaticStack<T, N>& StaticStack<T, N>::operator=(const StaticStack<T, M>& other) {
  StaticStack<T, M> temp = other;
  swap(temp);
  return *this;
}
STACK_TEMPLATE
StaticStack<T, N>& StaticStack<T, N>::operator=(const StaticStack<T, N>& other) {
  if (&other == this) return *this;
  StaticStack<T, N> temp = other;
  swap(temp);
  return *this;
}

STACK_TEMPLATE template <size_t M> void StaticStack<T, N>::swap(StaticStack<T, M>& other) noexcept {
  static_assert(M <= N, "Source stack too large for target stack");
  size_t minLen = std::min(m_length, other.m_length);

  for (size_t i = 0; i < minLen; ++i) std::swap(*getBuffer(i), *other.getBuffer(i));

  if (m_length > other.m_length) {
    for (size_t i = other.m_length; i < m_length && i < M; ++i) {
      new (other.getBuffer(i)) T(std::move(*getBuffer(i)));
      getBuffer(i)->~T();
    }
  } else if (other.m_length > m_length) {
    for (size_t i = m_length; i < other.m_length; ++i) {
      new (getBuffer(i)) T(std::move(*other.getBuffer(i)));
      other.getBuffer(i)->~T();
    }
  }

  std::swap(m_length, other.m_length);
}

STACK_TEMPLATE
template <size_t M>
StaticStack<T, N>::StaticStack(StaticStack<T, M>&& other) noexcept(std::is_nothrow_move_constructible_v<T>) {
  move(std::move(other));
}

STACK_TEMPLATE
StaticStack<T, N>::StaticStack(StaticStack<T, N>&& other) noexcept(std::is_nothrow_move_constructible_v<T>) {
  move(std::move(other));
}

STACK_TEMPLATE
template <size_t M>
StaticStack<T, N>&
StaticStack<T, N>::operator=(StaticStack<T, M>&& other) noexcept(std::is_nothrow_move_constructible_v<T>) {
  StaticStack<T, M> temp = std::move(other);
  swap(temp);
  return *this;
}

STACK_TEMPLATE
StaticStack<T, N>&
StaticStack<T, N>::operator=(StaticStack<T, N>&& other) noexcept(std::is_nothrow_move_constructible_v<T>) {
  if (&other == this) return *this;
  StaticStack<T, N> temp = std::move(other);
  swap(temp);
  return *this;
}

STACK_TEMPLATE StaticStack<T, N>::~StaticStack() noexcept { clear(); }

STACK_TEMPLATE template <typename... Args> T& StaticStack<T, N>::emplace(Args&&... args) {
  if (isFull()) throw std::overflow_error("Stack is full");
  T* newDataPtr = new (getBuffer(m_length++)) T(std::forward<Args>(args)...); // NOLINT
  return *newDataPtr;
}

STACK_TEMPLATE void StaticStack<T, N>::push(const T& value) { emplace(value); }
STACK_TEMPLATE void StaticStack<T, N>::push(T&& value) { emplace(std::move(value)); }

STACK_TEMPLATE T StaticStack<T, N>::pop() {
  if (isEmpty()) throw std::underflow_error("Stack is empty");
  m_length--;
  constexpr bool preferMove = std::is_nothrow_move_constructible_v<T> || !std::is_copy_constructible_v<T>;
  T top = preferMove ? std::move(*getBuffer(m_length)) : *getBuffer(m_length);
  getBuffer(m_length)->~T();
  return top;
}

STACK_TEMPLATE const T& StaticStack<T, N>::top() const {
  if (isEmpty()) throw std::underflow_error("Stack is empty");
  return *getBuffer(m_length - 1);
}

STACK_TEMPLATE void StaticStack<T, N>::clear() noexcept {
  for (size_t i = m_length; i > 0; i--) getBuffer(i - 1)->~T();
  m_length = 0;
}

STACK_TEMPLATE bool StaticStack<T, N>::isEmpty() const noexcept { return m_length == 0; }

STACK_TEMPLATE bool StaticStack<T, N>::isFull() const noexcept { return m_length == m_capacity; }

STACK_TEMPLATE
size_t StaticStack<T, N>::getCapacity() const noexcept { return m_capacity; }

STACK_TEMPLATE size_t StaticStack<T, N>::getSize() const noexcept { return m_length; }

} // namespace stack
