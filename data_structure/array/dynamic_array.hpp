#pragma once
#include <algorithm>
#include <cassert>
#include <new>
#include <stdexcept>
#include <type_traits>
namespace array {

template <typename T> class DynamicArray {
private:
  T* m_data = nullptr;
  size_t m_length;
  size_t m_capacity;

  void checkBound(size_t index) const {
    if (index >= m_length)
      throw std::out_of_range(
          "Index out of bounds, index: " + std::to_string(index) + ", size: " + std::to_string(m_length)
      );
  }

  void assertBound(size_t index) const noexcept { assert(index < m_length && "Index out of bounds"); }

  void release() noexcept {
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
    for (size_t i = 0; i < m_length; i++) new (m_data + i) T(other.m_data[i]);
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
  template <bool IsConst> class DynamicArrayIterator {
    // Each instantiation of a class template is a distinct type
    template <bool> friend class DynamicArrayIterator;

  private:
    using RawPtr = std::conditional_t<IsConst, const T*, T*>;
    RawPtr m_ptr = nullptr;

  public:
    using value_type = T;
    using reference = std::conditional_t<IsConst, const T&, T&>;
    using pointer = RawPtr;
    using difference_type = std::ptrdiff_t;
    using iterator_category = std::random_access_iterator_tag;

    constexpr DynamicArrayIterator() = default;
    constexpr DynamicArrayIterator(const DynamicArrayIterator&) = default;
    constexpr DynamicArrayIterator(DynamicArrayIterator&&) = default;
    constexpr DynamicArrayIterator& operator=(const DynamicArrayIterator&) = default;
    constexpr DynamicArrayIterator& operator=(DynamicArrayIterator&&) = default;
    constexpr ~DynamicArrayIterator() = default;

    constexpr explicit DynamicArrayIterator(pointer p = nullptr) : m_ptr(p) {};

    // Conversion constructor for mutable iterator to const iterator, not a copy constructor.
    constexpr DynamicArrayIterator(const DynamicArrayIterator<false>& other)
      requires IsConst
        : m_ptr(other.m_ptr) {}

    constexpr reference operator*() const { return *m_ptr; }
    constexpr pointer operator->() const { return m_ptr; }

    constexpr reference operator[](difference_type n) const { return *(m_ptr + n); }

    constexpr DynamicArrayIterator& operator++() {
      m_ptr++;
      return *this;
    }

    constexpr DynamicArrayIterator operator++(int) {
      DynamicArrayIterator snapshot = *this;
      m_ptr++;
      return snapshot;
    }

    constexpr DynamicArrayIterator& operator--() {
      m_ptr--;
      return *this;
    }

    constexpr DynamicArrayIterator operator--(int) {
      DynamicArrayIterator snapshot = *this;
      m_ptr--;
      return snapshot;
    }

    constexpr DynamicArrayIterator& operator+=(difference_type n) {
      m_ptr += n;
      return *this;
    }

    constexpr DynamicArrayIterator& operator-=(difference_type n) {
      m_ptr -= n;
      return *this;
    }

    constexpr DynamicArrayIterator operator+(difference_type n) const {
      return DynamicArrayIterator(m_ptr + n);
    }
    constexpr DynamicArrayIterator operator-(difference_type n) const {
      return DynamicArrayIterator(m_ptr - n);
    }
    constexpr difference_type operator-(const DynamicArrayIterator& other) const {
      return m_ptr - other.m_ptr;
    }

    constexpr bool operator==(const DynamicArrayIterator& other) const { return m_ptr == other.m_ptr; }
    constexpr bool operator!=(const DynamicArrayIterator& other) const { return m_ptr != other.m_ptr; }
    constexpr bool operator>(const DynamicArrayIterator& other) const { return m_ptr > other.m_ptr; }
    constexpr bool operator<(const DynamicArrayIterator& other) const { return m_ptr < other.m_ptr; }
    constexpr bool operator>=(const DynamicArrayIterator& other) const { return m_ptr >= other.m_ptr; }
    constexpr bool operator<=(const DynamicArrayIterator& other) const { return m_ptr <= other.m_ptr; }
  };

  using value_type = T;
  using size_type = size_t;
  using reference = T&;
  using const_reference = const T&;
  using Iterator = DynamicArrayIterator<false>;
  using ConstIterator = DynamicArrayIterator<true>;
  using ReverseIterator = std::reverse_iterator<Iterator>;
  using ConstReverseIterator = std::reverse_iterator<ConstIterator>;

  DynamicArray() : m_capacity(2), m_length(0), m_data(allocate(2)) {};

  DynamicArray(size_t size) : DynamicArray(size, T{}) {}

  DynamicArray(size_t size, const T& value) : m_capacity(size), m_length(size) {
    if (size <= 0) throw std::invalid_argument("Size must be positive");
    m_data = allocate(m_capacity);
    for (size_t i = 0; i < m_length; i++) new (m_data + i) T(value);
  }

  DynamicArray(std::initializer_list<T> init)
      : m_capacity(init.size()), m_length(init.size()), m_data(allocate(init.size())) {
    size_t i = 0;
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

  void clear() noexcept {
    for (size_t i = 0; i < m_length; i++) { m_data[i].~T(); }
    m_length = 0;
  }

  Iterator erase(ConstIterator pos) { return erase(pos, pos + 1); }

  Iterator erase(ConstIterator first, ConstIterator last) {
    if (first < cbegin() || last > cend() || last < first)
      throw std::out_of_range("Erase positions out of range");

    size_t start = static_cast<size_t>(first - cbegin());
    size_t finish = static_cast<size_t>(last - cbegin());
    size_t count = finish - start;

    for (size_t i = start; i < finish; i++) m_data[i].~T();

    constexpr bool preferMove = std::is_nothrow_move_constructible_v<T> || !std::is_copy_constructible_v<T>;

    for (size_t i = finish; i < m_length; i++) {
      if constexpr (preferMove) {
        new (m_data + i - count) T(std::move(m_data[i]));
      } else {
        new (m_data + i - count) T(m_data[i]);
      }
      m_data[i].~T();
    }

    m_length -= count;
    return Iterator{m_data + start};
  }

  void popBack() { erase(cend() - 1); }

  template <typename... Args> Iterator emplace(ConstIterator pos, Args&&... args) {
    if (pos < cbegin() || pos > cend()) throw std::out_of_range("Emplace Position out of range");

    // this has to happen before calling reserve because reserve may render pos dangling.
    // dangling ptr arithmetic is Undefined Behavior.
    size_t idx = static_cast<size_t>(pos - cbegin());

    if (m_length == m_capacity) reserve(m_capacity == 0 ? 2 : m_capacity * 2);

    constexpr bool preferMove = std::is_nothrow_move_constructible_v<T> || !std::is_copy_constructible_v<T>;

    for (size_t i = m_length; i > idx; i--) {
      if constexpr (preferMove) {
        new (m_data + i) T(std::move(m_data[i - 1]));
      } else {
        new (m_data + i) T(m_data[i - 1]);
      }
      m_data[i - 1].~T();
    }

    new (m_data + idx) T(std::forward<Args>(args)...);
    m_length++;
    return Iterator{m_data + idx};
  }

  template <typename... Args> T& emplaceBack(Args&&... args) {
    return *emplace(cend(), std::forward<Args>(args)...);
  }

  void pushBack(const T& value) { emplaceBack(value); }
  void pushBack(T&& value) { emplaceBack(std::move(value)); }

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
      constexpr bool preferMove = std::is_nothrow_move_constructible_v<T> || !std::is_copy_constructible_v<T>;

      while (i < m_length) {
        if constexpr (preferMove) {
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

    for (size_t i = 0; i < m_length; i++) m_data[i].~T();
    ::operator delete(m_data, std::align_val_t{alignof(T)});
    m_data = newData;
    m_capacity = newCapacity;
  }

  constexpr const T& operator[](size_t index) const noexcept {
    assertBound(index);
    return m_data[index];
  }

  constexpr T& operator[](size_t index) noexcept {
    assertBound(index);
    return m_data[index];
  }

  const T& at(size_t index) const {
    checkBound(index);
    return m_data[index];
  }
  T& at(size_t index) {
    checkBound(index);
    return m_data[index];
  }

  constexpr const T& front() const noexcept { return operator[](0); }
  constexpr T& front() noexcept { return operator[](0); }

  constexpr const T& back() const noexcept { return operator[](m_length - 1); }
  constexpr T& back() noexcept { return operator[](m_length - 1); }

  constexpr void swap(DynamicArray& other) noexcept {
    std::swap(m_data, other.m_data);
    std::swap(m_length, other.m_length);
    std::swap(m_capacity, other.m_capacity);
  }

  [[nodiscard]] constexpr size_t size() const noexcept { return m_length; };
  [[nodiscard]] constexpr size_t capacity() const noexcept { return m_capacity; };
  [[nodiscard]] constexpr bool empty() const noexcept { return m_length == 0; }

  constexpr Iterator begin() noexcept { return Iterator{m_data}; }
  constexpr ConstIterator begin() const noexcept { return ConstIterator{m_data}; }
  constexpr ConstIterator cbegin() const noexcept { return ConstIterator{m_data}; }

  constexpr Iterator end() noexcept { return Iterator{m_data + m_length}; }
  constexpr ConstIterator end() const noexcept { return ConstIterator{m_data + m_length}; }
  constexpr ConstIterator cend() const noexcept { return ConstIterator{m_data + m_length}; }

  constexpr ReverseIterator rbegin() noexcept { return ReverseIterator{end()}; }
  constexpr ConstReverseIterator rbegin() const noexcept { return ConstReverseIterator{end()}; }
  constexpr ConstReverseIterator crbegin() const noexcept { return ConstReverseIterator{end()}; }

  constexpr ReverseIterator rend() noexcept { return ReverseIterator{begin()}; }
  constexpr ConstReverseIterator rend() const noexcept { return ConstReverseIterator{begin()}; }
  constexpr ConstReverseIterator crend() const noexcept { return ConstReverseIterator{begin()}; }
};

template <typename T> void swap(DynamicArray<T>& a, DynamicArray<T>& b) noexcept { a.swap(b); } // for ADL
} // namespace array
