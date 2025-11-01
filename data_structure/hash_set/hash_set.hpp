#pragma once
#include "../hash_map/hash_map.hpp"
#include <concepts>
#include <functional>

namespace hashset {

template <typename Key, typename Hasher = std::hash<Key>, typename KeyEqual = std::equal_to<Key>>
  requires std::predicate<KeyEqual, const Key&, const Key&> && std::invocable<Hasher, const Key&> &&
           std::convertible_to<std::invoke_result_t<Hasher, const Key&>, size_t>

class HashSet {

private:
  hashmap::HashMap<Key, bool, Hasher, KeyEqual> m_map;

public:
  HashSet() = default;
  HashSet(std::initializer_list<Key> init) {
    for (const Key& key : init) m_map.insert(key, true);
  };

  bool contains(const Key& key) const noexcept { return m_map.contains(key); }

  size_t erase(const Key& key) { return m_map.erase(key); }

  void clear() { m_map.clear(); }
  [[nodiscard]] size_t getSize() const noexcept { return m_map.getSize(); }
  // add method

  // begin, end, clear, erase, empty, emplace, insert, size
  // what do they do: buecket, bucket_size, buecket_count, cbegin, cend
};
} // namespace hashset
//
// add noexcept to begin, end
