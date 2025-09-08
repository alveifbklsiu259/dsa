#pragma once
#include <iostream>
#include <stdexcept>

namespace Array {

template <typename T> class DynamicArray {
private:
  T *data;
  int length;
  int capacity;

  void assertBounds(int index) const {
    int lastIndex = capacity - 1;
    if (index < 0 || index > lastIndex)
      throw std::out_of_range("Index out of bounds");
  }

  void resize(int newCapacity) {
    if (newCapacity < length)
      throw std::invalid_argument("New Capacity too small");

    T *newData = new T[newCapacity];

    if constexpr (std::is_move_constructible_v<T>) {
      std::cout << "resize by moving!" << std::endl;
      std::copy(std::make_move_iterator(data),
                std::make_move_iterator(data + length), newData);

      // same as
      // for (size_t i = 0; i < length; ++i)
      //   newData[i] = std::move(data[i]);
    } else {
      std::cout << "resize by copying!" << std::endl;
      std::copy(data, data + length, newData);
    };

    delete[] data;
    data = newData;
    capacity = newCapacity;
  }

public:
  DynamicArray() : capacity(1), length(0) { data = new T[1]; }

  DynamicArray(int size) : capacity(size), length(0) {
    if (size <= 0)
      throw std::invalid_argument("Size must be positive");
    data = new T[size];
  }

  DynamicArray(std::initializer_list<T> init)
      : capacity(init.size()), length(init.size()) {
    if (init.size() == 0)
      throw std::invalid_argument("Initializer list must not be empty");

    data = new T[capacity];
    std::copy(init.begin(), init.end(), data);
  }

  ~DynamicArray() { delete[] data; };

  int size() const { return length; };

  void push_back(const T &value) {
    if (length == capacity) {
      resize(capacity * 2);
    }
    data[length++] = value;
  }

  const T &operator[](int index) const {
    assertBounds(index);
    return data[index];
  }

  T &operator[](int index) {
    return const_cast<T &>(static_cast<const DynamicArray<T> &>(*this)[index]);
  }

  T *begin() { return data; }
  T *end() { return data + length; }

  const T *begin() const { return data; }
  const T *end() const { return data + length; }
};
} // namespace Array
