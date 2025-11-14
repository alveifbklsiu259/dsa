#pragma once
#include <algorithm>
#include <iterator>

namespace sort {

namespace detail {

template <typename RandomIt, typename Compare>
concept Comparator =
    std::random_access_iterator<RandomIt> &&
    std::strict_weak_order<Compare&, const std::iter_value_t<RandomIt>&, const std::iter_value_t<RandomIt>&>;

} // namespace detail

template <std::random_access_iterator RandomIt, typename Compare = std::greater<>>
  requires detail::Comparator<RandomIt, Compare>
void bubbleSort(RandomIt first, RandomIt last, Compare compare = Compare{}) {
  auto n = std::distance(first, last);
  for (decltype(n) i = 0; i < n - 1; i++) {
    bool swapped = false;
    for (RandomIt j = first; j < last - 1 - i; j++) {
      if (compare(*j, *(j + 1))) {
        std::iter_swap(j, j + 1);
        swapped = true;
      }
    }
    if (!swapped) break;
  }
};

template <std::random_access_iterator RandomIt, typename Compare = std::less<>>
void selectionSort(RandomIt first, RandomIt last, Compare compare = Compare{})
  requires detail::Comparator<RandomIt, Compare>
{
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

} // namespace sort
