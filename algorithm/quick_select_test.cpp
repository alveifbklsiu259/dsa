#include "sort/sort.hpp"
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_range_equals.hpp>
#include <catch2/matchers/catch_matchers_vector.hpp>

import quick_select;

TEST_CASE("quickSelect core functionality", "[quick_select]") {

  SECTION("Finds the k-th largest elements in a standard unsorted vector") {
    array::DynamicArray<int> arr = {9, 2, 7, 4, 5, 1, 8, 3, 6};
    array::DynamicArray<int> sorted{arr};
    sort::quickSort(sorted.begin(), sorted.end(), std::greater<>());

    int k = 1;
    auto it1 = algo::quickSelect(arr.begin(), arr.end(), k);
    REQUIRE(*it1 == sorted[k - 1]);

    k = 3;
    auto it3 = algo::quickSelect(arr.begin(), arr.end(), k);
    REQUIRE(*it3 == sorted[k - 1]);

    k = 9;
    auto it9 = algo::quickSelect(arr.begin(), arr.end(), k);
    REQUIRE(*it9 == sorted[k - 1]);
  }

  SECTION("Handles a single-element array correctly") {
    array::DynamicArray<int> arr = {42};
    auto it = algo::quickSelect(arr.begin(), arr.end(), 1);
    REQUIRE(*it == 42);
    REQUIRE(it == arr.begin());
  }

  SECTION("Handles an array containing duplicate values safely without hanging") {
    array::DynamicArray<int> arr = {5, 5, 2, 5, 8, 2, 5, 5};
    array::DynamicArray<int> sorted{arr};
    sort::quickSort(sorted.begin(), sorted.end(), std::greater<>());

    int k = 1;
    auto itLargest = algo::quickSelect(arr.begin(), arr.end(), k);
    REQUIRE(*itLargest == sorted[k - 1]);

    k = 4;
    auto itMiddle = algo::quickSelect(arr.begin(), arr.end(), k);
    REQUIRE(*itMiddle == sorted[k - 1]);
  }

  SECTION("Works perfectly with a custom comparator (e.g., std::greater)") {
    array::DynamicArray<int> arr = {9, 2, 7, 4, 5, 1, 8, 3, 6};
    auto it = algo::quickSelect(arr.begin(), arr.end(), 1, std::greater<>{});
    REQUIRE(*it == 1);
  }
}

TEST_CASE("quickSelect exception and boundary guards", "[quick_select]") {

  SECTION("Throws exception when sequence is empty") {
    array::DynamicArray<int> emptyArr;
    REQUIRE_THROWS_AS(algo::quickSelect(emptyArr.begin(), emptyArr.end(), 1), std::invalid_argument);
  }

  SECTION("Throws exception when k is out of bounds") {
    array::DynamicArray<int> arr = {1, 2, 3};

    REQUIRE_THROWS_AS(algo::quickSelect(arr.begin(), arr.end(), 0), std::invalid_argument);
    REQUIRE_THROWS_AS(algo::quickSelect(arr.begin(), arr.end(), 4), std::invalid_argument);
  }
}

TEST_CASE("quickSelectCopy isolation functionality", "[quick_select]") {

  SECTION("Finds target value without modifying the caller's original sequence") {
    const array::DynamicArray<int> original = {15, 3, 9, 1, 7, 12};
    array::DynamicArray<int> copy(original);

    int value = algo::quickSelectCopy(copy.begin(), copy.end(), 2);

    REQUIRE(value == 12);
    REQUIRE_THAT(copy, Catch::Matchers::RangeEquals(original));
  }
}
