#pragma once
#include <cstddef>
#include <stdexcept>

namespace Array {

template <typename T, size_t N> class StaticArray {
private:
  T data[N];

  void assertBounds(int index) const {
    if (index > N)
      throw std::out_of_range("Index out of bounds");
  }

public:
  StaticArray() = default;

  StaticArray(std::initializer_list<T> init) {
    if (init.size() == 0)
      throw std::invalid_argument("Initializer list must not be empty");

    int i = 0;
    for (const T &item : init) {
      data[i++] = item;
    }

    while (i < N) {
      data[i++] = T();
    }
  }

  T &at(int index) {
    assertBounds(index);
    return data[index];
  }

  const T &at(int index) const {
    assertBounds(index);
    return data[index];
  }

  void set(int index, T value) {
    assertBounds(index);
    data[index] = value;
  }

  int size() const { return N; };

  const T &operator[](int index) const {
    assertBounds(index);
    return data[index];
  }

  T &operator[](int index) {
    return const_cast<T &>(
        static_cast<const StaticArray<T, N> &>(*this)[index]);
  }

  T *begin() { return data; }
  T *end() { return data + N; }

  const T *begin() const { return data; }
  const T *end() const { return data + N; }

  std::reverse_iterator<T *> rbegin() {
    return std::reverse_iterator<T *>(end());
  }
  std::reverse_iterator<T *> rend() {
    return std::reverse_iterator<T *>(begin());
  }

  const std::reverse_iterator<const T *> rbegin() const {
    return std::reverse_iterator<const T *>(end());
  }
  const std::reverse_iterator<const T *> rend() const {
    return std::reverse_iterator<const T *>(begin());
  }
};

} // namespace Array
