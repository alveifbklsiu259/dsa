module;
#include "sort/sort.hpp"
#include <iterator>
export module quick_select;

export namespace algo {
template <std::random_access_iterator RandomIt, typename Compare = std::less<>, typename URNG = std::mt19937>
  requires sort::detail::Comparator<RandomIt, Compare>
RandomIt quickSelect(RandomIt first, RandomIt last, size_t k, Compare compare = {}, URNG* gen = nullptr) {
  size_t n = std::distance(first, last);
  if (n == 0) throw std::invalid_argument("can't find from empty sequence");
  if (k == 0 || k > n) {
    throw std::invalid_argument("k should not be 0 or larger than the size of the sequence");
  }

  std::optional<URNG> localGen;

  size_t indexOfKthElement = n - k;
  if (gen == nullptr) {
    std::random_device rd;
    localGen.emplace(rd());
    gen = &*localGen;
  }

  RandomIt left = first;
  RandomIt right = last;
  // while (true) {
  //   // because partition calculates offset instead of using the index of the sequence,
  //   // the returned value is a "local" index to the sequence passed in
  //   size_t localPivotIdx = sort::detail::partition(left, right, compare, *gen);
  //   size_t globalPivotIdx = std::distance(first, left) + localPivotIdx;
  //
  //   if (globalPivotIdx == indexOfKthElement) return (first + indexOfKthElement);
  //
  //   if (globalPivotIdx > indexOfKthElement) {
  //     right = left + localPivotIdx;
  //   } else {
  //     left = left + localPivotIdx + 1;
  //   }
  // }
  RandomIt kthIter = first + indexOfKthElement;
  while (true) {
    RandomIt piviotIt = sort::detail::partitionIter(left, right, compare, *gen);

    if (piviotIt == kthIter) return piviotIt;

    if (piviotIt > kthIter) {
      right = piviotIt;
    } else {
      left = piviotIt + 1;
    }
  }
}

// Quick select that does not mutate the original sequence,
// note that it returns a copy of the iterator value.
// It is impossible to return the iterator of the original sequence without mutating the original sequence,
// because quick Select does not actually search for anything. Instead, it picks a target destination slot
// and forces the sequence to adapt to it.
template <std::random_access_iterator RandomIt, typename Compare = std::less<>, typename URNG = std::mt19937>
  requires sort::detail::Comparator<RandomIt, Compare>
std::iter_value_t<RandomIt>
quickSelectCopy(RandomIt first, RandomIt last, size_t k, Compare compare = {}, URNG* gen = nullptr) {
  array::DynamicArray<std::iter_value_t<RandomIt>> copy(first, last);
  auto found = quickSelect(copy.begin(), copy.end(), k, compare, gen);
  return *found;
}
} // namespace algo
