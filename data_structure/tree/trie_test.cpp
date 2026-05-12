#include "./trie.hpp"
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_all.hpp>

TEST_CASE("Trie Initialization", "[trie][Trie]") {
  tree::Trie<std::string> trie;
  // trie.insert("apple");
  // trie.insert("application");
  //
  // SECTION("search()") {
  //   CHECK(trie.search("apple") == true);
  //   CHECK(trie.search("app") == false);
  //   CHECK(trie.search("banana") == false);
  //   CHECK(trie.search("application") == true);
  // }
  //
  // SECTION("startsWith()") {
  //   CHECK(trie.startsWith("app") == true);
  //   CHECK(trie.startsWith("appli") == true);
  //   CHECK(trie.startsWith("ape") == false);
  // }
}
