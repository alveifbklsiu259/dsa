#pragma once
#include <algorithm>
#include <iostream>
#include <new>
#include <stdexcept>
#include <type_traits>
namespace array {

template <typename T> class DynamicArray {
private:
  T* m_data = nullptr;
  size_t m_length;
  size_t m_capacity;

  void assertBounds(int index) const {
    if (index < 0 || index >= m_length)
      throw std::out_of_range(
          "Index out of bounds, index: " + std::to_string(index) +
          ", size: " + std::to_string(m_length)
      );
  }

  void release() {
    clear();
    ::operator delete(m_data, std::align_val_t{alignof(T)});
    m_data = nullptr;
    m_length = 0;
    m_capacity = 0;
  }

  void deepCopy(const DynamicArray<T>& other) {
    m_length = other.m_length;
    m_capacity = other.m_capacity;
    m_data = allocate(m_capacity);
    for (int i = 0; i < m_length; i++) new (m_data + i) T(other.m_data[i]);
  }

  void move(DynamicArray<T>&& other) noexcept { // NOLINT
    m_length = other.m_length;
    m_capacity = other.m_capacity;
    m_data = other.m_data;
    other.m_capacity = 0;
    other.m_length = 0;
    other.m_data = nullptr;
  }

  static T* allocate(size_t capacity) noexcept {
    if (capacity == 0) return nullptr;
    return static_cast<T*>(::operator new(capacity * sizeof(T), std::align_val_t{alignof(T)}));
  }

public:
  DynamicArray() : m_capacity(2), m_length(0), m_data(allocate(2)) {};

  DynamicArray(size_t size) : DynamicArray(size, T{}) {}

  DynamicArray(size_t size, const T& value) : m_capacity(size), m_length(size) {
    if (size <= 0) throw std::invalid_argument("Size must be positive");
    m_data = allocate(m_capacity);
    for (size_t i = 0; i < m_length; i++) new (m_data + i) T(value);
  }

  DynamicArray(std::initializer_list<T> init)
      : m_capacity(init.size()), m_length(init.size()), m_data(allocate(init.size())) {
    int i = 0;
    for (const T& element : init) {
      new (m_data + i) T(element);
      i++;
    }
  }

  DynamicArray(const DynamicArray<T>& other) { deepCopy(other); }; // NOLINT

  DynamicArray(DynamicArray<T>&& other) noexcept { move(std::move(other)); }; // NOLINT

  DynamicArray<T>& operator=(const DynamicArray<T>& other) {
    if (&other == this) return *this;
    release();
    deepCopy(other);
    return *this;
  };

  DynamicArray<T>& operator=(DynamicArray<T>&& other) noexcept {
    if (&other == this) return *this;
    release();
    move(std::move(other));
    return *this;
  };

  ~DynamicArray() noexcept { release(); };

  [[nodiscard]] size_t getSize() const noexcept { return m_length; };
  [[nodiscard]] size_t getCapacity() const noexcept { return m_capacity; };

  void clear() {
    for (int i = 0; i < m_length; i++) { m_data[i].~T(); }
    m_length = 0;
  }

  void pushBack(const T& value) {
    if (m_length == m_capacity) reserve(m_capacity * 2);
    new (m_data + m_length) T(value);
    m_length++;
  }

  void pushBack(T&& value) {
    if (m_length == m_capacity) reserve(m_capacity * 2);
    new (m_data + m_length) T(std::move(value));
    m_length++;
  }

  void resize(size_t newSize) {
    if (newSize > m_length) {
      if (newSize > m_capacity) reserve(std::max(newSize, static_cast<size_t>(m_capacity * 2)));
      for (size_t i = m_length; i < newSize; i++) new (m_data + i) T{};
    } else if (newSize < m_length) {
      for (size_t i = newSize; i < m_length; i++) { m_data[i].~T(); }
    }
    m_length = newSize;
  }

  void reserve(size_t newCapacity) {
    if (newCapacity <= m_capacity) return;
    size_t i = 0;
    T* newData = allocate(newCapacity);

    try {
      constexpr bool canSafelyMove =
          std::is_nothrow_move_constructible_v<T> || !std::is_copy_constructible_v<T>;

      std::cout << (canSafelyMove ? "reserve by moving" : "reserve by copying") << '\n';

      while (i < m_length) {
        if constexpr (canSafelyMove) {
          new (newData + i) T(std::move(m_data[i]));
        } else {
          new (newData + i) T(m_data[i]);
        }
        i++;
      }
    } catch (...) {
      for (size_t j = 0; j < i; j++) { newData[j].~T(); }
      ::operator delete(newData, std::align_val_t{alignof(T)});
      throw;
    }

    for (size_t i = 0; i < m_length; i++) { m_data[i].~T(); }
    ::operator delete(m_data, std::align_val_t{alignof(T)});
    m_data = newData;
    m_capacity = newCapacity;
  }

  const T& operator[](int index) const {
    assertBounds(index);
    return m_data[index];
  }

  T& operator[](int index) {
    assertBounds(index);
    return m_data[index];
  }

  T* begin() noexcept { return m_data; }
  T* end() noexcept { return m_data + m_length; }

  const T* begin() const noexcept { return m_data; }
  const T* end() const noexcept { return m_data + m_length; }
};
} // namespace array
