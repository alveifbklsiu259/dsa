#include "../array/dynamic_array.hpp"
#include <functional>

namespace queue {

namespace detail {

template <typename T, typename Compare>
concept Comparator = std::strict_weak_order<Compare&, const T&, const T&>;

template <typename T, typename S>
concept SnakeSequence = requires(T t, S s) {
  { s.push_back(t) } -> std::same_as<void>;
  { s.pop_back(t) } -> std::same_as<void>;
};

template <typename T, typename S>
concept CamelSequence = requires(T t, S s) {
  { s.pushBack(t) } -> std::same_as<void>;
  { s.popBack(t) } -> std::same_as<void>;
};

template <typename T, typename S>
concept Sequence = (SnakeSequence<T, S> || CamelSequence<T, S>) && requires(T t, S s) {
  { s.empty() } -> std::same_as<bool>;
  { s.size() } -> std::convertible_to<size_t>;
  { s[0] } -> std::same_as<T&>;
};

} // namespace detail

template <typename T, typename S = array::DynamicArray<T>, typename Compare = std::less<T>>
  requires detail::Comparator<T, Compare> && detail::Sequence<T, S>
class PriorityQueue {};
} // namespace queue
