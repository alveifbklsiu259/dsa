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
  class Iterator {

  private:
    using MapIterator = hashmap::HashMap<Key, bool, Hasher, KeyEqual>::Iterator;
    MapIterator m_it;

  public:
    using value_type = Key;
    using reference = const value_type&;
    using pointer = const value_type*;
    using difference_type = std::ptrdiff_t;
    using iterator_category = std::forward_iterator_tag;

    explicit Iterator(MapIterator it) : m_it(it) {}

    Iterator& operator++() {
      ++m_it;
      return *this;
    }

    Iterator operator++(int) {
      Iterator snapshot = *this;
      ++(*this);
      return snapshot;
    }

    reference operator*() const { return (*m_it).getKey(); }
    pointer operator->() const { return &((*m_it).getKey()); }

    bool operator==(const Iterator& other) const { return m_it == other.m_it; }
    bool operator!=(const Iterator& other) const { return m_it != other.m_it; }
  };

  using ConstIterator = Iterator;

  HashSet() = default;
  HashSet(std::initializer_list<Key> init) {
    for (const Key& key : init) m_map.insert(key, true);
  };

  bool contains(const Key& key) const noexcept { return m_map.contains(key); }

  size_t erase(const Key& key) { return m_map.erase(key); }

  void clear() { m_map.clear(); }

  [[nodiscard]] size_t getSize() const noexcept { return m_map.getSize(); }

  std::pair<Iterator, bool> insert(const Key& key) {
    auto [it, inserted] = m_map.insert(key, true);
    return {Iterator(it), inserted};
  }

  std::pair<Iterator, bool> insert(Key&& key) {
    auto [it, inserted] = m_map.insert(std::move(key), true);
    return {Iterator(it), inserted};
  }

  Iterator begin() { return Iterator(m_map.begin()); }

  ConstIterator begin() const { return ConstIterator(m_map.begin()); }

  Iterator end() { return Iterator(m_map.end()); }
  ConstIterator end() const { return ConstIterator(m_map.end()); }

  // iterator problem, unordered_map return std::pair<K, V> for for(auto a: m);
  // my current implementation returns HashMapKeyVal;
  //
  // also we may need a new iterator for hash set, because for (auto a: s) will return std::pair<K,
  // bool> if we don't hamdle this.

  // begin, end, clear, empty, emplace,
  // what do they do: buecket, bucket_size, buecket_count, cbegin, cend
};
} // namespace hashset
//
// add noexcept to begin, end
