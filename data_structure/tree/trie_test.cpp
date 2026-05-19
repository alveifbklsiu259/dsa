#include "../array/dynamic_array.hpp"
#include <catch2/catch_test_macros.hpp>
#include <memory_resource>
#include <string>
import trie;
import test;

TEST_CASE("Trie basic operations", "[trie]") {
  tree::Trie<std::string> t;

  SECTION("Starts empty") {
    REQUIRE(t.empty());
    REQUIRE(t.size() == 0);
  }

  SECTION("Insertion and searching") {
    t.insert("hello");
    t.insert("world");

    REQUIRE(t.size() == 2);
    REQUIRE(t.search("hello"));
    REQUIRE(t.search("world"));
    REQUIRE_FALSE(t.search("hell"));
    REQUIRE_FALSE(t.search("earth"));
  }

  SECTION("Prefix searching") {
    t.insert("apple");
    t.insert("app");

    REQUIRE(t.startsWith("app"));
    REQUIRE(t.startsWith("appl"));
    REQUIRE(t.startsWith("apple"));
    REQUIRE_FALSE(t.startsWith("apx"));
  }
}

TEST_CASE("Trie prefix retrieval", "[trie]") {
  tree::Trie<std::string> t;
  t.insert("car");
  t.insert("cart");
  t.insert("cat");
  t.insert("dog");

  SECTION("Get all with prefix") {
    auto results = t.getAllWithPrefix("ca");

    REQUIRE(results.size() == 3);

    bool foundCart = false;
    for (const auto& result : results) {
      if (result == "cart") foundCart = true;
    }
    REQUIRE(foundCart);
  }

  SECTION("Prefix with no results") {
    auto results = t.getAllWithPrefix("z");
    REQUIRE(results.empty());
  }
}

TEST_CASE("Trie iterator", "[trie]") {
  tree::Trie<std::string> t;
  t.insert("abc");
  t.insert("def");

  SECTION("Iteration visits all words") {
    int count = 0;
    for (auto it = t.begin(); it != t.end(); ++it) { count++; }
    REQUIRE(count == 2);
  }
}

TEST_CASE("Trie iterator returns views", "[trie][iterator]") {
  tree::Trie<std::string> t;
  t.insert("apple");

  auto it = t.begin();

  SECTION("Type verification") {
    using IteratorReference = decltype(*it);

    STATIC_CHECK(std::same_as<IteratorReference, std::string_view>);
    STATIC_CHECK(!std::same_as<IteratorReference, std::string>);
  }

  SECTION("View validity") {
    std::string_view view = *it;
    REQUIRE(view == "apple");
    REQUIRE(view.size() == 5);
  }
}

TEST_CASE("Trie with custom sequence returns spans", "[trie][iterator]") {
  using IntSeq = array::DynamicArray<int>;
  tree::Trie<IntSeq> t;
  t.insert({1, 2, 3});

  auto it = t.begin();
  if (it != t.end()) {
    using IteratorReference = decltype(*it);
    STATIC_CHECK(std::same_as<IteratorReference, std::span<const int>>);
  }
}

TEST_CASE("Trie Copy and Move", "[trie]") {
  tree::Trie<std::string> t1;
  t1.insert("move_me");

  SECTION("Copy constructor") {
    tree::Trie<std::string> t2(t1);
    t2.insert("bar");
    REQUIRE(t2.search("move_me"));
    REQUIRE(t2.search("bar"));
    REQUIRE(t1.search("move_me"));
    REQUIRE_FALSE(t1.search("bar"));
  }

  SECTION("Move constructor") {
    tree::Trie<std::string> t2 = std::move(t1);
    REQUIRE(t2.search("move_me"));
    REQUIRE(t1.empty());
  }

  SECTION("Copy assignment") {
    tree::Trie<std::string> t3;
    t3.insert("original");
    t3 = t1;
    REQUIRE(t3.search("move_me"));
    REQUIRE_FALSE(t3.search("original"));
    REQUIRE(t1.search("move_me"));
  }

  SECTION("Move assignment") {
    tree::Trie<std::string> t2;
    t2 = std::move(t1);

    REQUIRE(t2.search("move_me"));
    REQUIRE(t1.empty());
  }
}

TEST_CASE("Trie uses PMR allocator without global escapes", "[trie][pmr]") {
  test::FallbackTracker fallbackTracker;
  test::DefaultResourceGuard defaultResourceGuard(&fallbackTracker);

  std::pmr::monotonic_buffer_resource pool{2048};
  test::DetailedTracker customTracker(&pool);
  std::pmr::polymorphic_allocator<std::byte> alloc(&customTracker);

  std::string a = "apple";
  std::string b = "banana";

  SECTION("Trie and TrieNode use custom allocator") {
    tree::Trie<std::string> trie(alloc);

    size_t initialAllocationCount = customTracker.allocationCount();
    // root node
    REQUIRE(initialAllocationCount == 1);

    //  pool allocation
    REQUIRE(fallbackTracker.allocationCount() == 1);

    trie.insert(a);

    REQUIRE(customTracker.allocationCount() == a.size() + initialAllocationCount);
    REQUIRE(fallbackTracker.allocationCount() == 1);

    trie.insert(b);

    REQUIRE(customTracker.allocationCount() == a.size() + b.size() + initialAllocationCount);
    REQUIRE(fallbackTracker.allocationCount() == 1);
  }

  SECTION("Copy constructor propagates alternative allocator cleanly") {
    tree::Trie<std::string> t1(alloc);
    t1.insert(a);
    t1.insert(b);

    REQUIRE(customTracker.allocationCount() == 1 + a.size() + b.size());

    std::pmr::monotonic_buffer_resource pool2{2048};
    test::DetailedTracker customerTracker2(&pool2);
    std::pmr::polymorphic_allocator<std::byte> alloc2(&customerTracker2);

    // Deep copy using the second allocator
    tree::Trie<std::string> t2(t1, alloc2);

    REQUIRE(customerTracker2.allocationCount() == customTracker.allocationCount());
    REQUIRE(t2.search(a));
    REQUIRE(t2.search(b));
    REQUIRE(t1.search(a));

    // only allocating customerTracker and customerTracker2, no trie node is allocated in the global heap
    REQUIRE(fallbackTracker.allocationCount() == 2);
  }

  SECTION("Move constructor with different allocators triggers deep element reallocation") {
    std::pmr::monotonic_buffer_resource sourcePool{2048};
    test::DetailedTracker sourceTracker(&sourcePool);
    std::pmr::polymorphic_allocator<std::byte> sourceAlloc(&sourceTracker);

    tree::Trie<std::string> t1(sourceAlloc);
    t1.insert(a);

    std::pmr::monotonic_buffer_resource destPool{2048};
    test::DetailedTracker destTracker(&destPool);
    std::pmr::polymorphic_allocator<std::byte> destAlloc(&destTracker);

    // allocating sourcepool because of t1's root node
    REQUIRE(fallbackTracker.allocationCount() == 1);

    REQUIRE(destTracker.allocationCount() == 0);

    tree::Trie<std::string> t2(std::move(t1), destAlloc);

    REQUIRE(destTracker.allocationCount() == a.size() + 1);
    REQUIRE(destTracker.bytesAllocated() == sourceTracker.bytesAllocated());
    REQUIRE(t2.search(a));
    REQUIRE(t1.size() == 0);
  }

  SECTION("Move constructor with identical allocators performs an O(1) pointer swap") {
    tree::Trie<std::string> t1(alloc);
    t1.insert(a);

    size_t totalAllocationsBeforeMove = customTracker.allocationCount();

    tree::Trie<std::string> t2(std::move(t1), alloc);

    REQUIRE(customTracker.allocationCount() == totalAllocationsBeforeMove);
    REQUIRE(t2.search(a));
  }

  SECTION("Copy assignment uses destination's existing allocator") {
    tree::Trie<std::string> t1(alloc);
    t1.insert(a);

    std::pmr::monotonic_buffer_resource destPool{2048};
    test::DetailedTracker destTracker(&destPool);
    std::pmr::polymorphic_allocator<std::byte> destAlloc(&destTracker);

    tree::Trie<std::string> t2(destAlloc);
    t2.insert(b); // Setup some dummy tracking history
    size_t cusBeforeAssignmentCount = customTracker.allocationCount();
    size_t desBeforeAssignmentCount = destTracker.allocationCount();

    t2 = t1;

    REQUIRE(destTracker.allocationCount() > desBeforeAssignmentCount);
    REQUIRE(cusBeforeAssignmentCount == customTracker.allocationCount());
    REQUIRE(t2.search(a));
    REQUIRE_FALSE(t2.search(b));
  }
}
