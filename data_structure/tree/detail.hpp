#pragma once
#include <optional>
#include <ranges>

namespace tree {

template <typename T> class Node; // forward declaration, definition in node.hpp
} // namespace tree

namespace tree::detail {
template <typename T, typename Seq>
concept BidirectionalSequence =
    std::ranges::bidirectional_range<Seq> && std::same_as<std::ranges::range_value_t<Seq>, T>;

template <typename T, typename Seq>
concept RandomAccessOptionalSequence =
    std::ranges::random_access_range<Seq> && std::same_as<std::ranges::range_value_t<Seq>, std::optional<T>>;

template <typename T, typename H>
concept Hasher =
    std::invocable<H, const T&> && std::convertible_to<std::invoke_result_t<H, const T&>, size_t>;

template <typename T, typename K>
concept KeyEqual = std::predicate<K, const T&, const T&>;

template <typename T, typename Iter>
  requires std::input_iterator<Iter> && std::same_as<std::remove_cvref_t<std::iter_value_t<Iter>>, T>
struct Frame {
  Iter inBegin;
  Iter inEnd;
  Iter seqBegin; // we use the name "seq" here in order to make it unambiguous, it can be either preorder,
                 // postorder or levelorder
  Iter seqEnd;
  Node<T>* parent;
  bool attachLeft;
};

template <typename T, typename Compare>
concept Comparator = std::strict_weak_order<Compare&, T, T>;

} // namespace tree::detail
