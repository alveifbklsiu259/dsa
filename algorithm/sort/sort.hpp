#pragma once
#include <algorithm>
#include <iterator>

namespace sort {

template <std::random_access_iterator RandomIt> void bubbleSort(RandomIt first, RandomIt last) {
  auto n = std::distance(first, last);
  for (decltype(n) i = 0; i < n - 1; i++) {
    bool swapped = false;
    for (RandomIt j = first; j < last - 1 - i; j++) {
      if (*j > *(j + 1)) {
        std::iter_swap(j, j + 1);
        swapped = true;
      }
    }
    if (!swapped) break;
  }
};

template <std::random_access_iterator RandomIt> void selectionSort(RandomIt first, RandomIt last) {
  auto n = std::distance(first, last);
  for (decltype(n) i = 0; i < n - 1; i++) {
    RandomIt minIt = first + i;
    for (RandomIt j = first + i + 1; j < last; j++) {
      if (*j < *minIt) minIt = j;
    }
    if (minIt != (first + i)) std::iter_swap(first + i, minIt);
  }
}

} // namespace sort
