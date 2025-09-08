// Stack.tpp

#include "./Stack.hpp"
#define STACK_TEMPLATE template <typename T, size_t N>

namespace Stack {

STACK_TEMPLATE Stack<T, N>::Stack() { init(); }

STACK_TEMPLATE
template <size_t M> void Stack<T, N>::copy(const Stack<T, M> &other) {
  static_assert(M <= N, "Source stack too large for target stack");
  length = other.getSize();
  for (int i = 0; i < length; i++)
    data[i] = other.data[i];
}

STACK_TEMPLATE
template <size_t M> void Stack<T, N>::move(const Stack<T, M> &other) noexcept {
  static_assert(M <= N, "Source stack too large for target stack");
  length = other.getSize();
  for (int i = 0; i < length; i++)
    data[i] = std::move(other.data[i]);
}

STACK_TEMPLATE void Stack<T, N>::init() {
  for (int i = 0; i < N; i++)
    data[i] = T{};
}

STACK_TEMPLATE
template <size_t M> Stack<T, N>::Stack(const Stack<T, M> &other) {
  copy(other);
}

STACK_TEMPLATE Stack<T, N>::Stack(const Stack<T, N> &other) { copy(other); }

STACK_TEMPLATE
template <size_t M>
Stack<T, N> &Stack<T, N>::operator=(const Stack<T, M> &other) {
  copy(other);
  return *this;
}

STACK_TEMPLATE
Stack<T, N> &Stack<T, N>::operator=(const Stack<T, N> &other) {
  if (&other == this)
    return *this;
  init();
  copy(other);
  return *this;
}

STACK_TEMPLATE
template <size_t M> Stack<T, N>::Stack(Stack<T, M> &&other) noexcept {
  move(other);
}

STACK_TEMPLATE
Stack<T, N>::Stack(Stack<T, N> &&other) noexcept { move(other); }

STACK_TEMPLATE
template <size_t M>
Stack<T, N> &Stack<T, N>::operator=(Stack<T, M> &&other) noexcept {
  if (!std::is_trivially_destructible<T>::value)
    init();
  move(other);
  return *this;
}

STACK_TEMPLATE
Stack<T, N> &Stack<T, N>::operator=(Stack<T, N> &&other) noexcept {
  if (&other == this)
    return *this;
  if (!std::is_trivially_destructible<T>::value)
    init();
  move(other);
  return *this;
}

STACK_TEMPLATE void Stack<T, N>::push(const T &value) {
  if (isFull())
    throw std::overflow_error("Stack is full");
  data[length++] = value;
}

STACK_TEMPLATE void Stack<T, N>::push(T &&value) {
  if (isFull())
    throw std::overflow_error("Stack is full");
  data[length++] = std::move(value);
}

STACK_TEMPLATE T Stack<T, N>::pop() {
  if (isEmpty())
    throw std::underflow_error("Stack is empty");
  return data[--length];
}

STACK_TEMPLATE T Stack<T, N>::peek() {
  if (isEmpty())
    throw std::underflow_error("Stack is empty");
  return data[length - 1];
}

STACK_TEMPLATE bool Stack<T, N>::isEmpty() const noexcept {
  return length == 0;
}

STACK_TEMPLATE bool Stack<T, N>::isFull() const noexcept {
  return length == capacity;
}

STACK_TEMPLATE
size_t Stack<T, N>::getCapacity() const noexcept { return capacity; }

STACK_TEMPLATE size_t Stack<T, N>::getSize() const noexcept { return length; }

} // namespace Stack