#pragma once
#include "../array/dynamic_array.hpp"
#include <functional>
namespace queue {

namespace detail {

template <typename T, typename Compare>
concept Comparator = std::strict_weak_order<Compare&, const T&, const T&>;

template <typename T, typename S>
concept SnakeSequence = requires(T t, S s) {
  { s.emplace_back(t) } -> std::same_as<T&>;
  { s.pop_back() } -> std::same_as<void>;
};

template <typename T, typename S>
concept CamelSequence = requires(T t, S s) {
  { s.emplaceBack(t) } -> std::same_as<T&>;
  { s.popBack() } -> std::same_as<void>;
};

template <typename T, typename S>
concept Sequence = (SnakeSequence<T, S> || CamelSequence<T, S>) && requires(T t, S s, S& sRef, size_t i) {
  { s.empty() } -> std::same_as<bool>;
  { s.size() } -> std::convertible_to<size_t>;
  { s[i] } -> std::same_as<T&>;
  { swap(sRef, sRef) } -> std::same_as<void>; // require ADL swap
};

} // namespace detail

/**
 *  @tparam T  Type of element.
 *  @tparam S  Type of underlying sequence, defaults to array::DynamicArray<T>.
 *  @tparam Compare  Comparison function object type, defaults to
 *                    std::less<T>.
 */
template <typename T, typename S = array::DynamicArray<T>, typename Compare = std::less<T>>
  requires detail::Comparator<T, Compare> && detail::Sequence<T, S>
class PriorityQueue {
private:
  S m_data;
  Compare m_compare;
  size_t parent(size_t idx) { return idx == 0 ? 0 : (idx - 1) / 2; }
  size_t left(size_t idx) { return (2 * idx) + 1; }
  size_t right(size_t idx) { return (2 * idx) + 2; }

  void bubbleDown(size_t idx) {
    size_t n = size();

    while (true) {
      size_t leftIdx = left(idx);
      size_t rightIdx = right(idx);
      size_t extreme = idx;

      if (leftIdx < n && m_compare(m_data[extreme], m_data[leftIdx])) extreme = leftIdx;
      if (rightIdx < n && m_compare(m_data[extreme], m_data[rightIdx])) extreme = rightIdx;
      if (extreme == idx) break;
      std::swap(m_data[idx], m_data[extreme]);
      idx = extreme;
    }
  }

  void bubbleUp(size_t idx) {
    while (idx > 0) {
      size_t parentIdx = parent(idx);
      if (!m_compare(m_data[parentIdx], m_data[idx])) break;
      // if parent < current -> swap -> we get max heap
      // if parent > current -> swap -> we get min heap
      std::swap(m_data[idx], m_data[parentIdx]);
      idx = parentIdx;
    }
  }

  void bottomUpHeapify() {
    if (!m_data.empty()) {
      for (int i = (size() / 2) - 1; i >= 0; --i) bubbleDown(i);
    }
  }

  template <typename... Args> decltype(auto) emplaceBackData(Args&&... args) {
    if constexpr (requires { m_data.emplaceBack(std::forward<Args>(args)...); }) {
      return m_data.emplaceBack(std::forward<Args>(args)...);
    } else {
      return m_data.emplace_back(std::forward<Args>(args)...);
    }
  }

  void popBackData() {
    if constexpr (requires { m_data.popBack(); }) {
      m_data.popBack();
    } else {
      m_data.pop_back();
    }
  }

  template <std::input_iterator InputIt> void appendRange(InputIt first, InputIt last) {
    for (; first != last; ++first) emplaceBackData(*first);
  }

  [[nodiscard]] constexpr size_t headIndex() const noexcept { return 0; }
  [[nodiscard]] constexpr size_t tailIndex() const noexcept { return size() - 1; }

public:
  using value_type = typename S::value_type;
  using size_type = typename S::size_type;
  using reference = typename S::reference;
  using const_reference = typename S::const_reference;

  constexpr PriorityQueue() = default;

  constexpr PriorityQueue(const Compare& compare, S&& s = S{}) : m_data(std::move(s)), m_compare(compare) {}

  constexpr PriorityQueue(const Compare& compare, const S& seq) : m_data(seq), m_compare(compare) {}

  template <std::input_iterator InputIt>
  PriorityQueue(InputIt first, InputIt last, const Compare& compare = Compare{}) : m_compare(compare) {
    appendRange(first, last);
    bottomUpHeapify();
  }

  template <std::input_iterator InputIt>
  PriorityQueue(InputIt first, InputIt last, const Compare& compare, const S& seq)
      : m_compare(compare), m_data(seq) {
    appendRange(first, last);
    bottomUpHeapify();
  }

  template <std::input_iterator InputIt>
  PriorityQueue(InputIt first, InputIt last, const Compare& compare, S&& seq)
      : m_compare(compare), m_data(std::move(seq)) {
    appendRange(first, last);
    bottomUpHeapify();
  }

  constexpr PriorityQueue(const PriorityQueue& other) : m_data(other.m_data), m_compare(other.m_compare) {}

  constexpr PriorityQueue(PriorityQueue&& other) noexcept
      : m_data(std::move(other.m_data)), m_compare(std::move(other.m_compare)) {}

  PriorityQueue& operator=(const PriorityQueue& other) {
    if (&other == this) return *this;
    m_data = other.m_data;
    m_compare = other.m_compare;
    return *this;
  }

  PriorityQueue& operator=(PriorityQueue&& other) noexcept {
    if (&other == this) return *this;
    m_data = std::move(other.m_data);
    m_compare = std::move(other.m_compare);
    return *this;
  }

  constexpr ~PriorityQueue() = default;

  template <typename... Args> void emplace(Args&&... args) {
    emplaceBackData(std::forward<Args>(args)...);
    bubbleUp(tailIndex());
  }

  void push(const value_type& val) { emplace(val); }
  void push(value_type&& val) { emplace(std::move(val)); }

  void pop() {
    if (empty()) throw std::underflow_error("Priority Queue is empty.");
    std::swap(m_data[headIndex()], m_data[tailIndex()]);
    popBackData();
    if (!empty()) bubbleDown(headIndex());
  }

  [[nodiscard]] const_reference top() const {
    if (empty()) throw std::underflow_error("Priority Queue is empty.");
    return m_data[headIndex()];
  }

  void swap(PriorityQueue& other) noexcept {
    using std::swap;
    swap(m_data, other.m_data);
    swap(m_compare, other.m_compare);
  }

  [[nodiscard]] constexpr bool empty() const noexcept { return m_data.empty(); }
  [[nodiscard]] constexpr size_type size() const noexcept { return m_data.size(); }
};

template <typename T, typename S, typename Compare>
void swap(PriorityQueue<T, S, Compare>& a, PriorityQueue<T, S, Compare>& b) noexcept {
  a.swap(b); // for ADL
}
} // namespace queue
