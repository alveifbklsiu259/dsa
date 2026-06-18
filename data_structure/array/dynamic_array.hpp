#pragma once
#include <algorithm>
#include <cassert>
#include <concepts>
#include <iostream>
#include <memory>
#include <new>
#include <ostream>
#include <stdexcept>
#include <type_traits>

namespace {
template <typename T>
concept Streamable = requires(std::ostream& os, const T& val) {
  { os << val } -> std::convertible_to<std::ostream&>;
};

template <typename T>
concept DereferenceStreamable = requires(const T& val) {
  { *val };
} && Streamable<std::remove_cvref_t<decltype(*std::declval<const T&>())>>;

template <typename T>
concept ValStreamable = requires(const T& val) {
  { val.val() };
} && Streamable<std::remove_cvref_t<decltype(std::declval<const T&>().val())>>;

template <typename T>
concept ValueStreamable = requires(const T& val) {
  { val.value() };
} && Streamable<std::remove_cvref_t<decltype(std::declval<const T&>().value())>>;

} // namespace

namespace array {

template <typename T> class DynamicArray {
private:
  T* m_data = nullptr;
  size_t m_length = 0;
  size_t m_capacity = 0;

  constexpr void checkBound(size_t index) const {
    if (index >= m_length)
      throw std::out_of_range(
          "Index out of bounds, index: " + std::to_string(index) + ", size: " + std::to_string(m_length)
      );
  }

  constexpr void assertBound(size_t index) const noexcept {
    assert(index < m_length && "Index out of bounds");
  }

  constexpr void release() noexcept {
    clear();
    deallocate(m_data);
    m_data = nullptr;
    m_length = 0;
    m_capacity = 0;
  }

  constexpr void deepCopy(const DynamicArray<T>& other) {
    m_length = other.m_length;
    m_capacity = other.m_capacity;
    m_data = allocate(m_capacity);
    for (size_t i = 0; i < m_length; i++) new (m_data + i) T(other.m_data[i]);
  }

  constexpr void move(DynamicArray<T>&& other) noexcept { // NOLINT
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

  static void deallocate(T* ptr) noexcept {
    if (ptr == nullptr) return;
    ::operator delete(ptr, std::align_val_t{alignof(T)});
  }

  // constexpr static T* allocate(size_t capacity) noexcept {
  //   if (capacity == 0) return nullptr;
  //   return std::allocator<T>{}.allocate(capacity);
  // }
  //
  // constexpr static void deallocate(T* ptr, size_t capacity) noexcept {
  //   if (ptr == nullptr) return;
  //
  //   // This IS constexpr-friendly
  //   std::allocator<T>{}.deallocate(ptr, capacity);
  // }

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
    using iterator_concept = std::random_access_iterator_tag;

    constexpr DynamicArrayIterator() = default;
    constexpr DynamicArrayIterator(const DynamicArrayIterator&) = default;
    constexpr DynamicArrayIterator(DynamicArrayIterator&&) noexcept = default;
    constexpr DynamicArrayIterator& operator=(const DynamicArrayIterator&) = default;
    constexpr DynamicArrayIterator& operator=(DynamicArrayIterator&&) noexcept = default;
    constexpr ~DynamicArrayIterator() noexcept = default;

    constexpr explicit DynamicArrayIterator(pointer p) : m_ptr(p) {};

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

    friend constexpr DynamicArrayIterator
    operator+(difference_type n, const DynamicArrayIterator& it) noexcept {
      return it + n;
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
  using reference = value_type&;
  using const_reference = const value_type&;
  using pointer = value_type*;
  using const_pointer = const value_type*;
  using iterator = DynamicArrayIterator<false>;
  using const_iterator = DynamicArrayIterator<true>;
  using reverse_iterator = std::reverse_iterator<iterator>;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;

  constexpr DynamicArray() : m_capacity(2), m_data(allocate(2)) {};

  constexpr DynamicArray(size_t size) : DynamicArray(size, T{}) {}

  constexpr DynamicArray(size_t size, const T& value) : m_capacity(size), m_length(size) {
    m_data = allocate(m_capacity);
    try {
      std::uninitialized_fill_n(m_data, m_length, value);
    } catch (...) {
      deallocate(m_data);
      throw;
    }
  }

  constexpr DynamicArray(std::initializer_list<T> init) : DynamicArray(init.begin(), init.end()) {}

  template <std::input_iterator InputIt>
    requires std::constructible_from<T, std::iter_value_t<InputIt>>
  constexpr DynamicArray(InputIt first, InputIt last) {
    size_t size = std::distance(first, last);
    if (size <= 0) {
      m_capacity = 0;
      m_length = 0;
      m_data = nullptr;
      return;
    }

    m_capacity = size;
    m_length = size;
    m_data = allocate(size);

    size_t i = 0;
    try {
      for (auto it = first; it != last; ++it) {
        new (m_data + i) T(*it);
        ++i;
      }
    } catch (...) {
      for (size_t j = 0; j < i; ++j) m_data[j].~T();
      deallocate(m_data);
      throw;
    }
  }

  constexpr DynamicArray(const DynamicArray<T>& other) { deepCopy(other); }; // NOLINT

  constexpr DynamicArray(DynamicArray<T>&& other) noexcept { move(std::move(other)); }; // NOLINT

  constexpr DynamicArray<T>& operator=(const DynamicArray<T>& other) {
    if (&other == this) return *this;
    release();
    deepCopy(other);
    return *this;
  };

  constexpr DynamicArray<T>& operator=(DynamicArray<T>&& other) noexcept {
    if (&other == this) return *this;
    release();
    move(std::move(other));
    return *this;
  };

  constexpr ~DynamicArray() noexcept { release(); };

  constexpr void clear() noexcept {
    for (size_t i = 0; i < m_length; i++) { m_data[i].~T(); }
    m_length = 0;
  }

  constexpr iterator erase(const_iterator pos) { return erase(pos, pos + 1); }

  constexpr iterator erase(const_iterator first, const_iterator last) {
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
    return iterator{m_data + start};
  }

  void popBack() { erase(cend() - 1); }

  template <typename... Args> constexpr iterator emplace(const_iterator pos, Args&&... args) {
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
    return iterator{m_data + idx};
  }

  template <typename... Args> constexpr T& emplaceBack(Args&&... args) {
    return *emplace(cend(), std::forward<Args>(args)...);
  }

  constexpr void pushBack(const T& value) { emplaceBack(value); }
  constexpr void pushBack(T&& value) { emplaceBack(std::move(value)); }

  constexpr void resize(size_t newSize) {
    if (newSize > m_length) {
      if (newSize > m_capacity) reserve(std::max(newSize, static_cast<size_t>(m_capacity * 2)));
      for (size_t i = m_length; i < newSize; i++) new (m_data + i) T{};
    } else if (newSize < m_length) {
      for (size_t i = newSize; i < m_length; i++) { m_data[i].~T(); }
    }
    m_length = newSize;
  }

  constexpr void reserve(size_t newCapacity) {
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
      deallocate(newData);
      throw;
    }

    for (size_t i = 0; i < m_length; i++) m_data[i].~T();
    deallocate(m_data);
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
    using std::swap;
    swap(m_data, other.m_data);
    swap(m_length, other.m_length);
    swap(m_capacity, other.m_capacity);
  }

  friend std::ostream& operator<<(std::ostream& os, const DynamicArray& arr) {
    /**
     * @brief Recursively unwraps and prints a generic element to an output stream.
     *
     * 1. Why `auto` instead of `std::function`:
     *    - `std::function` cannot have templated (`auto`) parameters, which this function requires.
     *
     * 2. Why `std::remove_cvref_t`:
     *    - The parameter `const auto& e` forces `decltype(e)` to resolve to a reference type
     *      (e.g., `const Node* const&`).
     *    - Type traits like `std::is_pointer_v` look only at the outermost wrapper and will
     *      evaluate to false on a reference-to-pointer.
     *    - Stripping `const`, `volatile`, and `&` exposes the raw underlying type for accurate
     *      `if constexpr` compile-time branching.
     */
    auto printElement = [&](auto&& self, const auto& e) -> void {
      using Element = std::remove_cvref_t<decltype(e)>;

      if constexpr (std::is_pointer_v<Element>) {
        if (e == nullptr) {
          os << "nullptr";
          return;
        }
        self(self, *e);
        return;
      } else if constexpr (ValStreamable<Element>) {
        os << e.val();
      } else if constexpr (ValueStreamable<Element>) {
        os << e.value();
      } else if constexpr (DereferenceStreamable<Element>) {
        os << *e;
      } else if constexpr (Streamable<Element>) {
        os << e;
      } else {
        os << "Unstreamable Type";
      }
    };

    os << "[";
    size_t n = arr.size();
    for (size_t i = 0; i < n; ++i) {
      printElement(printElement, arr[i]);
      if (i < n - 1) os << ", ";
    };
    os << "]";
    return os;
  }

  [[nodiscard]] constexpr size_t size() const noexcept { return m_length; };
  [[nodiscard]] constexpr size_t capacity() const noexcept { return m_capacity; };
  [[nodiscard]] constexpr bool empty() const noexcept { return m_length == 0; }

  constexpr pointer data() noexcept { return m_data; }
  constexpr const_pointer data() const noexcept { return m_data; }

  constexpr iterator begin() noexcept { return iterator{m_data}; }
  constexpr const_iterator begin() const noexcept { return const_iterator{m_data}; }
  constexpr const_iterator cbegin() const noexcept { return const_iterator{m_data}; }

  constexpr iterator end() noexcept { return iterator{m_data + m_length}; }
  constexpr const_iterator end() const noexcept { return const_iterator{m_data + m_length}; }
  constexpr const_iterator cend() const noexcept { return const_iterator{m_data + m_length}; }

  constexpr reverse_iterator rbegin() noexcept { return reverse_iterator{end()}; }
  constexpr const_reverse_iterator rbegin() const noexcept { return const_reverse_iterator{end()}; }
  constexpr const_reverse_iterator crbegin() const noexcept { return const_reverse_iterator{end()}; }

  constexpr reverse_iterator rend() noexcept { return reverse_iterator{begin()}; }
  constexpr const_reverse_iterator rend() const noexcept { return const_reverse_iterator{begin()}; }
  constexpr const_reverse_iterator crend() const noexcept { return const_reverse_iterator{begin()}; }
};

template <typename T> void swap(DynamicArray<T>& a, DynamicArray<T>& b) noexcept { a.swap(b); } // for ADL
} // namespace array
