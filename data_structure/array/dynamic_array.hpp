#pragma once
#include <algorithm>
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

    DynamicArrayIterator() = default;
    DynamicArrayIterator(const DynamicArrayIterator&) = default;
    DynamicArrayIterator(DynamicArrayIterator&&) = default;
    DynamicArrayIterator& operator=(const DynamicArrayIterator&) = default;
    DynamicArrayIterator& operator=(DynamicArrayIterator&&) = default;
    ~DynamicArrayIterator() = default;

    explicit DynamicArrayIterator(pointer p = nullptr) : m_ptr(p) {};

    // Conversion constructor for mutable iterator to const iterator, not a copy constructor.
    DynamicArrayIterator(const DynamicArrayIterator<false>& other)
      requires IsConst
        : m_ptr(other.m_ptr) {}

    reference operator*() const { return *m_ptr; }
    pointer operator->() const { return m_ptr; }

    reference operator[](difference_type n) const { return *(m_ptr + n); }

    DynamicArrayIterator& operator++() {
      m_ptr++;
      return *this;
    }

    DynamicArrayIterator operator++(int) {
      DynamicArrayIterator snapshot = *this;
      m_ptr++;
      return snapshot;
    }

    DynamicArrayIterator& operator--() {
      m_ptr--;
      return *this;
    }

    DynamicArrayIterator operator--(int) {
      DynamicArrayIterator snapshot = *this;
      m_ptr--;
      return snapshot;
    }

    DynamicArrayIterator& operator+=(difference_type n) {
      m_ptr += n;
      return *this;
    }

    DynamicArrayIterator& operator-=(difference_type n) {
      m_ptr -= n;
      return *this;
    }

    DynamicArrayIterator operator+(difference_type n) const { return DynamicArrayIterator(m_ptr + n); }
    DynamicArrayIterator operator-(difference_type n) const { return DynamicArrayIterator(m_ptr - n); }
    difference_type operator-(const DynamicArrayIterator& other) const { return m_ptr - other.m_ptr; }

    bool operator==(const DynamicArrayIterator& other) const { return m_ptr == other.m_ptr; }
    bool operator!=(const DynamicArrayIterator& other) const { return m_ptr != other.m_ptr; }
    bool operator>(const DynamicArrayIterator& other) const { return m_ptr > other.m_ptr; }
    bool operator<(const DynamicArrayIterator& other) const { return m_ptr < other.m_ptr; }
    bool operator>=(const DynamicArrayIterator& other) const { return m_ptr >= other.m_ptr; }
    bool operator<=(const DynamicArrayIterator& other) const { return m_ptr <= other.m_ptr; }
  };

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

  [[nodiscard]] size_t getSize() const noexcept { return m_length; };
  [[nodiscard]] size_t getCapacity() const noexcept { return m_capacity; };
  [[nodiscard]] bool empty() const noexcept { return m_length == 0; }

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

  template <typename... Args> Iterator emplace(ConstIterator pos, Args&&... args) {
    if (pos < cbegin() || pos > cend()) throw std::out_of_range("Emplace Position out of range");

    if (m_length == m_capacity) reserve(m_capacity == 0 ? 2 : m_capacity * 2);
    size_t idx = static_cast<size_t>(pos - cbegin());

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

  const T& operator[](size_t index) const {
    checkBound(index);
    return m_data[index];
  }

  T& operator[](size_t index) {
    checkBound(index);
    return m_data[index];
  }

  Iterator begin() noexcept { return Iterator{m_data}; }
  ConstIterator begin() const noexcept { return ConstIterator{m_data}; }
  ConstIterator cbegin() const noexcept { return ConstIterator{m_data}; }

  Iterator end() noexcept { return Iterator{m_data + m_length}; }
  ConstIterator end() const noexcept { return ConstIterator{m_data + m_length}; }
  ConstIterator cend() const noexcept { return ConstIterator{m_data + m_length}; }

  ReverseIterator rbegin() noexcept { return ReverseIterator{end()}; }
  ConstReverseIterator rbegin() const noexcept { return ConstReverseIterator{end()}; }
  ConstReverseIterator crbegin() const noexcept { return ConstReverseIterator{end()}; }

  ReverseIterator rend() noexcept { return ReverseIterator{begin()}; }
  ConstReverseIterator rend() const noexcept { return ConstReverseIterator{begin()}; }
  ConstReverseIterator crend() const noexcept { return ConstReverseIterator{begin()}; }
};
} // namespace array
