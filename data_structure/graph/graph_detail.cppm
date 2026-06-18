module;
#include <concepts>
#include <cstddef>
#include <ranges>
export module graph:detail;

namespace graph::detail {
template <typename T, typename H>
concept Hasher =
    std::invocable<H, const T&> && std::convertible_to<std::invoke_result_t<H, const T&>, size_t>;
template <typename T, typename K>
concept KeyEqual = std::predicate<K, const T&, const T&>;

template <typename Pair, typename T>
concept PairLike = requires(const Pair& pair) {
  { std::get<0>(pair) } -> std::convertible_to<T>;
  { std::get<1>(pair) } -> std::convertible_to<T>;
};

template <typename Seq, typename T>
concept PairLikeForwardRange =
    std::ranges::forward_range<Seq> && PairLike<std::ranges::range_value_t<Seq>, T>;
} // namespace graph::detail
