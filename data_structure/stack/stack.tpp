#include "./stack.hpp"
#include <stdexcept>
#include <utility>

#define STACK_TEMPLATE template <typename T, size_t N>

namespace stack {

STACK_TEMPLATE T* Stack<T, N>::getBuffer(size_t i) noexcept {
  return reinterpret_cast<T*>(m_data.data()) + i; // NOLINT
}

STACK_TEMPLATE const T* Stack<T, N>::getBuffer(size_t i) const noexcept {
  return reinterpret_cast<const T*>(m_data.data()) + i; // NOLINT
}

STACK_TEMPLATE
template <size_t M> void Stack<T, N>::copy(const Stack<T, M>& other) {
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
template <size_t M> void Stack<T, N>::move(Stack<T, M>&& other) noexcept { // NOLINT
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
template <size_t M> Stack<T, N>::Stack(const Stack<T, M>& other) { copy(other); }

STACK_TEMPLATE Stack<T, N>::Stack(const Stack<T, N>& other) { copy(other); }

STACK_TEMPLATE
template <size_t M> Stack<T, N>& Stack<T, N>::operator=(const Stack<T, M>& other) {
  Stack<T, M> s = other;
  swap(s);
  return *this;
}
// in my dynamic array, m_data is set to nullptr initially, is that correct?
// consider provide the default dynamic buffer
// make it dynamic
// refactor dynamic array,
// destructor should not throw
STACK_TEMPLATE
Stack<T, N>& Stack<T, N>::operator=(const Stack<T, N>& other) {
  if (&other == this) return *this;
  Stack<T, N> s = other;
  swap(s);
  return *this;
}

STACK_TEMPLATE template <size_t M> void Stack<T, N>::swap(Stack<T, M>& other) noexcept {
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
template <size_t M> Stack<T, N>::Stack(Stack<T, M>&& other) noexcept { move(std::move(other)); }

STACK_TEMPLATE
Stack<T, N>::Stack(Stack<T, N>&& other) noexcept { move(std::move(other)); }

STACK_TEMPLATE
template <size_t M> Stack<T, N>& Stack<T, N>::operator=(Stack<T, M>&& other) noexcept {
  Stack<T, M> s = std::move(other);
  swap(s);
  return *this;
}

STACK_TEMPLATE
Stack<T, N>& Stack<T, N>::operator=(Stack<T, N>&& other) noexcept {
  if (&other == this) return *this;
  Stack<T, N> s = std::move(other);
  swap(s);
  return *this;
}

STACK_TEMPLATE Stack<T, N>::~Stack() noexcept { clear(); }

STACK_TEMPLATE void Stack<T, N>::push(const T& value) {
  if (isFull()) throw std::overflow_error("Stack is full");
  new (getBuffer(m_length++)) T(value);
}

STACK_TEMPLATE void Stack<T, N>::push(T&& value) {
  if (isFull()) throw std::overflow_error("Stack is full");
  new (getBuffer(m_length++)) T(std::move(value));
}

STACK_TEMPLATE template <typename... Args> T& Stack<T, N>::emplace(Args&&... args) {
  if (isFull()) throw std::overflow_error("Stack is full");
  T* newDataPtr = new (getBuffer(m_length++)) T(std::forward<Args>(args)...); // NOLINT
  return *newDataPtr;
}

STACK_TEMPLATE T Stack<T, N>::pop() {
  if (isEmpty()) throw std::underflow_error("Stack is empty");
  m_length--;
  constexpr bool canSafelyMove =
      std::is_nothrow_move_constructible_v<T> || !std::is_copy_constructible_v<T>;
  T top = canSafelyMove ? std::move(*getBuffer(m_length)) : *getBuffer(m_length);
  getBuffer(m_length)->~T();
  return top;
}

STACK_TEMPLATE const T& Stack<T, N>::top() const {
  if (isEmpty()) throw std::underflow_error("Stack is empty");
  return *getBuffer(m_length - 1);
}

STACK_TEMPLATE void Stack<T, N>::clear() noexcept {
  for (size_t i = m_length; i > 0; i--) getBuffer(i - 1)->~T();
  m_length = 0;
}

STACK_TEMPLATE bool Stack<T, N>::isEmpty() const noexcept { return m_length == 0; }

STACK_TEMPLATE bool Stack<T, N>::isFull() const noexcept { return m_length == m_capacity; }

STACK_TEMPLATE
size_t Stack<T, N>::getCapacity() const noexcept { return m_capacity; }

STACK_TEMPLATE size_t Stack<T, N>::getSize() const noexcept { return m_length; }

} // namespace stack
