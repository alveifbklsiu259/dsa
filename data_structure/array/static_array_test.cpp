#include "./static_array.hpp"
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_all.hpp>

TEST_CASE("StaticArray Initialization and Size", "[array][StaticArray]") {
  SECTION("Default construction zero-initializes") {
    array::StaticArray<int, 5> arr;
    REQUIRE(arr.size() == 5);
    for (int i : arr) CHECK(i == 0);
  }

  SECTION("Variadic constructor with exact size") {
    array::StaticArray arr{1, 2, 3}; // Uses CTAD
    REQUIRE(arr.size() == 3);
    CHECK(arr[0] == 1);
    CHECK(arr[2] == 3);
  }

  SECTION("Variadic constructor with partial initialization") {
    array::StaticArray<int, 5> arr{10, 20};
    CHECK(arr[0] == 10);
    CHECK(arr[1] == 20);
    // test no garbage values
    CHECK(arr[2] == 0);
    CHECK(arr[4] == 0);
  }
}

TEST_CASE("StaticArray Access and Bounds", "[array][StaticArray]") {
  array::StaticArray arr{10, 20, 30};

  SECTION("at() returns correct values and throws") {
    REQUIRE(arr.at(0) == 10);
    REQUIRE_THROWS_AS(arr.at(5), std::out_of_range);
  }

  SECTION("front and back access") {
    CHECK(arr.front() == 10);
    CHECK(arr.back() == 30);
  }
}

TEST_CASE("StaticArray Comparisons", "[array][StaticArray]") {
  array::StaticArray a{1, 2, 3};
  array::StaticArray b{1, 2, 3};
  array::StaticArray c{1, 2, 4};

  SECTION("Equality and Spaceship operators") {
    CHECK(a == b);
    CHECK(a != c);
    CHECK(a < c);
    CHECK(c > a);
  }
}

TEST_CASE("StaticArray Iterators", "[array][StaticArray]") {
  array::StaticArray arr{1, 2, 3, 4, 5};

  SECTION("Forward iteration") {
    int expected = 1;
    for (auto val : arr) CHECK(val == expected++);
  }

  SECTION("Reverse iteration") {
    auto it = arr.rbegin();
    CHECK(*it == 5);
    CHECK(*(++it) == 4);
  }
}

TEST_CASE("StaticArray Constexpr verification", "[array][StaticArray][constexpr]") {
  STATIC_REQUIRE(array::StaticArray<int, 3>{1, 2, 3}.size() == 3);
  STATIC_REQUIRE(array::StaticArray<int, 2>{10, 20}.at(1) == 20);

  constexpr array::StaticArray a{1, 2};
  constexpr array::StaticArray b{1, 2};
  STATIC_REQUIRE(a == b);
}

TEST_CASE("StaticArray with custom type", "[array][StaticArray]") {
  struct Foo {
    int val = 0;
  };
  array::StaticArray<Foo, 3> arr;

  SECTION("Forward iteration") {
    for (auto foo : arr) CHECK(foo.val == 0);
  }
}
