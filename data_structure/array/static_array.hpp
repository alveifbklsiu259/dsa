#pragma once
#include <cstddef>
#include <stdexcept>

namespace array {

template <typename T, size_t N> class StaticArray {
  static_assert(N > 0, "StaticArray size must be greater than 0");

private:
  // value initialized, guaranteed no garbage values
  T m_data[N]{}; // NOLINT

  constexpr void checkBound(size_t index) const {
    if (index >= N)
      throw std::out_of_range(
          "Index out of bounds, index: " + std::to_string(index) + ", capacity: " + std::to_string(N)
      );
  }

public:
  using value_type = T;
  using size_type = size_t;
  using difference_type = ptrdiff_t;
  using reference = value_type&;
  using const_reference = const value_type&;
  using pointer = value_type*;
  using const_pointer = const value_type*;
  using iterator = value_type*;
  using const_iterator = const value_type*;
  using reverse_iterator = std::reverse_iterator<iterator>;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;

  constexpr StaticArray() = default;

  template <typename... Args> constexpr StaticArray(Args&&... args) : m_data{std::forward<Args>(args)...} {
    static_assert(sizeof...(args) <= N, "Too many arguments");
  }

  constexpr void fill(const T& val) noexcept {
    for (size_t i = 0; i < N; ++i) m_data[i] = val;
  }

  constexpr reference at(size_t i) {
    checkBound(i);
    return m_data[i];
  }

  constexpr const_reference at(size_t i) const {
    checkBound(i);
    return m_data[i];
  }

  constexpr reference operator[](size_t i) noexcept { return m_data[i]; }
  constexpr const_reference operator[](size_t i) const noexcept { return m_data[i]; }

  constexpr bool operator==(const StaticArray& other) const = default;
  constexpr auto operator<=>(const StaticArray& other) const = default;

  constexpr reference front() noexcept { return m_data[0]; }
  constexpr const_reference front() const noexcept { return m_data[0]; }

  constexpr reference back() noexcept { return m_data[N - 1]; }
  constexpr const_reference back() const noexcept { return m_data[N - 1]; }

  constexpr pointer data() noexcept { return m_data; }
  constexpr const_pointer data() const noexcept { return m_data; }

  [[nodiscard]] constexpr size_type size() const noexcept { return N; }
  [[nodiscard]] constexpr size_type maxSize() const noexcept { return N; }
  [[nodiscard]] constexpr bool empty() const noexcept { return N == 0; }

  constexpr iterator begin() noexcept { return &m_data[0]; }
  constexpr const_iterator begin() const noexcept { return &m_data[0]; }
  constexpr const_iterator cbegin() const noexcept { return &m_data[0]; }

  constexpr reverse_iterator rbegin() noexcept { return reverse_iterator(end()); }
  constexpr const_reverse_iterator rbegin() const noexcept { return const_reverse_iterator(end()); }
  constexpr const_reverse_iterator crbegin() const noexcept { return const_reverse_iterator(end()); }

  constexpr iterator end() noexcept { return begin() + N; }
  constexpr const_iterator end() const noexcept { return begin() + N; }
  constexpr const_iterator cend() noexcept { return begin() + N; }

  constexpr reverse_iterator rend() noexcept { return reverse_iterator(begin()); }
  constexpr const_reverse_iterator rend() const noexcept { return const_reverse_iterator(begin()); }
  constexpr const_reverse_iterator crend() const noexcept { return const_reverse_iterator(begin()); }

  constexpr void swap(array::StaticArray<T, N>& other) noexcept {
    using std::swap;
    swap(m_data, other.m_data);
  }
  constexpr friend void swap(array::StaticArray<T, N>& a, array::StaticArray<T, N>& b) noexcept { a.swap(b); }
};

// Deduction guide
template <typename T, typename... Args> StaticArray(T, Args...) -> StaticArray<T, 1 + sizeof...(Args)>;

} // namespace array
