#pragma once
#include "../../data_structure/array/dynamic_array.hpp"
#include <algorithm>
#include <cstdint>
#include <iterator>

namespace sort {

namespace detail {

template <typename RandomIt, typename Compare>
concept Comparator =
    std::random_access_iterator<RandomIt> && std::indirect_strict_weak_order<Compare&, RandomIt>;

template <typename RandomIt>
concept IntegralIterator =
    std::random_access_iterator<RandomIt> && std::integral<std::iter_value_t<RandomIt>>;
} // namespace detail

enum class Order : uint8_t { Ascending, Descending };

template <std::random_access_iterator RandomIt, typename Compare = std::less<>>
  requires detail::Comparator<RandomIt, Compare>
void bubbleSort(RandomIt first, RandomIt last, Compare compare = Compare{}) {
  auto n = std::distance(first, last);
  for (decltype(n) i = 0; i < n - 1; i++) {
    bool swapped = false;
    for (RandomIt j = first; j < last - 1 - i; j++) {
      if (compare(*(j + 1), *j)) {
        std::iter_swap(j + 1, j);
        swapped = true;
      }
    }
    if (!swapped) break;
  }
};

template <std::random_access_iterator RandomIt, typename Compare = std::less<>>
  requires detail::Comparator<RandomIt, Compare>
void selectionSort(RandomIt first, RandomIt last, Compare compare = Compare{}) {
  auto n = std::distance(first, last);
  for (decltype(n) i = 0; i < n - 1; i++) {
    RandomIt bestIt = first + i;
    for (RandomIt j = first + i + 1; j < last; j++) {
      if (compare(*j, *bestIt)) bestIt = j;
    }
    if (bestIt != (first + i)) std::iter_swap(first + i, bestIt);
  }
}

template <std::random_access_iterator RandomIt, typename Compare = std::less<>>
  requires detail::Comparator<RandomIt, Compare>
void insertionSort(RandomIt first, RandomIt last, Compare compare = Compare{}) {
  for (RandomIt i = first + 1; i < last; i++) {
    auto current = *i;
    RandomIt j = i;
    while (j > first && compare(current, *(j - 1))) {
      *(j) = *(j - 1);
      j--;
    }
    *j = current;
  }
}

template <std::random_access_iterator RandomIt, typename Compare = std::less<>>
  requires detail::Comparator<RandomIt, Compare>
void heapSort(RandomIt first, RandomIt last, Compare compare = Compare{}) {
  // in-place heap sort uses O(1) auxiliary space, using priority queue would take O(n)
  auto n = std::distance(first, last);
  if (n <= 0) return;

  auto bubbleDown = [&](size_t idx, size_t heapSize) -> void {
    while (true) {
      size_t extreme = idx;
      size_t left = (idx * 2) + 1;
      size_t right = (idx * 2) + 2;
      if (left < heapSize && compare(*(first + extreme), *(first + left))) extreme = left;
      if (right < heapSize && compare(*(first + extreme), *(first + right))) extreme = right;
      if (extreme == idx) break;
      std::iter_swap(first + extreme, first + idx);
      idx = extreme;
    }
  };

  // bottom-up heapify
  for (int i = ((int)n / 2) - 1; i >= 0; --i) { bubbleDown(i, n); }

  for (size_t i = (size_t)n - 1; i > 0; --i) {
    std::iter_swap(first, first + i);
    bubbleDown(0, i);
  }
}

template <detail::IntegralIterator RandomIt>
void countingSort(RandomIt first, RandomIt last, Order order = Order::Ascending) {
  if (first == last) return;
  using ValueType = std::iter_value_t<RandomIt>;
  auto [min, max] = std::minmax_element(first, last);
  ValueType minVal = *min;
  ValueType maxVal = *max;
  size_t distance = std::distance(first, last);

  size_t range = static_cast<size_t>(maxVal - minVal + 1);
  array::DynamicArray<size_t> count(range, 0);
  for (RandomIt it = first; it != last; it++) count[*it - minVal]++;
  for (size_t i = 1; i < range; i++) count[i] += count[i - 1];

  array::DynamicArray<ValueType> output(distance);
  if (order == Order::Ascending) {
    for (RandomIt it = last; it != first;) {
      it--;
      size_t key = *it - minVal;
      size_t position = count[key] - 1;
      output[position] = *it;
      count[key]--;
    }
  } else {
    for (RandomIt it = first; it != last; it++) {
      size_t key = *it - minVal;
      size_t position = distance - count[key];
      output[position] = *it;
      count[key]--;
    }
  }
  std::move(output.begin(), output.end(), first);
}

} // namespace sort
