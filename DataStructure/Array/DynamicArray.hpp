#pragma once
#include <algorithm>
#include <iostream>
#include <new>
#include <stdexcept>
#include <type_traits>
namespace array {

template <typename T> class DynamicArray {
private:
  T* data = nullptr;
  size_t length;
  size_t capacity;

  void assertBounds(int index) const {
    if (index < 0 || index >= length)
      throw std::out_of_range(
          "Index out of bounds, index: " + std::to_string(index) +
          ", size: " + std::to_string(length)
      );
  }

  void release() {
    clear();
    ::operator delete(data);
    data = nullptr;
    length = 0;
    capacity = 0;
  }

  void deepCopy(const DynamicArray<T>& other) {
    length = other.length;
    capacity = other.capacity;
    data = static_cast<T*>(::operator new(capacity * sizeof(T)));
    for (int i = 0; i < length; i++) new (data + i) T(other.data[i]);
  }

  void move(DynamicArray<T>&& other) noexcept {
    length = other.length;
    capacity = other.capacity;
    data = other.data;
    other.capacity = 0;
    other.length = 0;
    other.data = nullptr;
  }

public:
  DynamicArray() : capacity(2), length(0) {
    data = static_cast<T*>(::operator new(sizeof(T) * capacity));
  };

  DynamicArray(size_t size) : capacity(size), length(0) {
    if (size <= 0) throw std::invalid_argument("Size must be positive");
    data = static_cast<T*>(::operator new(sizeof(T) * capacity));
  }

  DynamicArray(std::initializer_list<T> init) : capacity(init.size()), length(init.size()) {
    if (init.size() == 0) throw std::invalid_argument("Initializer list must not be empty");

    data = static_cast<T*>(::operator new(sizeof(T) * capacity));
    int i = 0;
    for (const T& element : init) {
      new (data + i) T(element);
      i++;
    }
  }

  DynamicArray(const DynamicArray<T>& other) { deepCopy(other); };

  DynamicArray(DynamicArray<T>&& other) noexcept { move(std::move(other)); };

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

  ~DynamicArray() { release(); };

  size_t getSize() const noexcept { return length; };
  size_t getCapacity() const noexcept { return capacity; };

  void clear() {
    for (int i = 0; i < length; i++) { data[i].~T(); }
    length = 0;
  }

  void push_back(const T& value) {
    if (length == capacity) reserve(capacity * 2);
    new (data + length) T(value);
    length++;
  }

  void push_back(T&& value) {
    if (length == capacity) reserve(capacity * 2);
    new (data + length) T(std::move(value));
    length++;
  }

  void resize(size_t newSize) {
    if (newSize > length) {
      if (newSize > capacity) reserve(std::max(newSize, static_cast<size_t>(capacity * 2)));
      for (int i = length; i < newSize; i++) new (data + i) T{};
    } else if (newSize < length) {
      for (int i = newSize; i < length; i++) { data[i].~T(); }
    }
    length = newSize;
  }

  void reserve(size_t newCapacity) {
    if (newCapacity <= capacity) return;
    size_t i = 0;
    T* newData = static_cast<T*>(::operator new(sizeof(T) * newCapacity));

    try {
      constexpr bool canSafelyMove =
          std::is_nothrow_move_constructible_v<T> || !std::is_copy_constructible_v<T>;

      std::cout << (canSafelyMove ? "reserve by moving" : "reserve by copying") << std::endl;

      while (i < length) {
        if constexpr (canSafelyMove) {
          new (newData + i) T(std::move(data[i]));
        } else {
          new (newData + i) T(data[i]);
        }
        i++;
      }
    } catch (...) {
      for (size_t j = 0; j < i; j++) { newData[j].~T(); }
      ::operator delete(newData);
      throw;
    }

    for (size_t i = 0; i < length; i++) { data[i].~T(); }
    ::operator delete(data);
    data = newData;
    capacity = newCapacity;
  }

  const T& operator[](int index) const {
    assertBounds(index);
    return data[index];
  }

  T& operator[](int index) {
    assertBounds(index);
    return data[index];
  }

  T* begin() { return data; }
  T* end() { return data + length; }

  const T* begin() const { return data; }
  const T* end() const { return data + length; }
};
} // namespace array
/**
 *
- std::is_nothrow_move_constructible optimization
std::vector prefers moving only if it’s noexcept, otherwise it falls back to copying (to maintain
strong exception safety). You could write: if constexpr (std::is_nothrow_move_constructible_v<T> ||
!std::is_copy_constructible_v<T>) {
    // move
} else {
    // copy
*/
