#pragma once
#include <cstddef>
#include <stdexcept>

namespace array {

template <typename T, size_t N> class StaticArray {
private:
  T m_data[N]; // NOLINT

  [[noreturn]] void assertBounds(int index) const {
    if (index < 0 || index >= N)
      throw std::out_of_range(
          "Index out of bounds, index: " + std::to_string(index) +
          ", capacity: " + std::to_string(N)
      );
  }

public:
  StaticArray() = default;

  StaticArray(std::initializer_list<T> init) {
    if (init.size() == 0) throw std::invalid_argument("Initializer list must not be empty");

    int i = 0;
    for (const T& item : init) { m_data[i++] = item; }

    while (i < N) { m_data[i++] = T(); }
  }

  T& at(int index) {
    assertBounds(index);
    return m_data[index];
  }

  const T& at(int index) const {
    assertBounds(index);
    return m_data[index];
  }

  void set(int index, T value) {
    assertBounds(index);
    m_data[index] = value;
  }

  [[nodiscard]] size_t size() const { return N; };

  const T& operator[](int index) const {
    assertBounds(index);
    return m_data[index];
  }

  T& operator[](int index) {
    return const_cast<T&>(static_cast<const StaticArray<T, N>&>(*this)[index]);
  }

  T* begin() noexcept { return m_data; }
  T* end() noexcept { return m_data + N; }

  const T* begin() const noexcept { return m_data; }
  const T* end() const noexcept { return m_data + N; }

  std::reverse_iterator<T*> rbegin() { return std::reverse_iterator<T*>(end()); }
  std::reverse_iterator<T*> rend() { return std::reverse_iterator<T*>(begin()); }

  std::reverse_iterator<const T*> rbegin() const { return std::reverse_iterator<const T*>(end()); }
  std::reverse_iterator<const T*> rend() const { return std::reverse_iterator<const T*>(begin()); }
};

} // namespace array
