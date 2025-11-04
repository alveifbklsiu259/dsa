#pragma once
#include "../hash_map/hash_map.hpp"
#include <concepts>
#include <functional>
#include <variant>

namespace hashset {

template <typename Key, typename Hasher = std::hash<Key>, typename KeyEqual = std::equal_to<Key>>
  requires std::predicate<KeyEqual, const Key&, const Key&> && std::invocable<Hasher, const Key&> &&
           std::convertible_to<std::invoke_result_t<Hasher, const Key&>, size_t>

class HashSet {

private:
  using DummyT = std::monostate;
  hashmap::HashMap<Key, DummyT, Hasher, KeyEqual> m_map;

public:
  class Iterator {

  private:
    using MapIterator = hashmap::HashMap<Key, DummyT, Hasher, KeyEqual>::Iterator;
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
    for (const Key& key : init) m_map.insert(key, DummyT{});
  };

  bool contains(const Key& key) const noexcept { return m_map.contains(key); }

  size_t erase(const Key& key) { return m_map.erase(key); }

  void clear() { m_map.clear(); }

  [[nodiscard]] size_t getSize() const noexcept { return m_map.getSize(); }

  std::pair<Iterator, bool> insert(const Key& key) {
    auto [it, inserted] = m_map.insert(key, DummyT{});
    return {Iterator(it), inserted};
  }

  std::pair<Iterator, bool> insert(Key&& key) {
    auto [it, inserted] = m_map.insert(std::move(key), DummyT{});
    return {Iterator(it), inserted};
  }

  template <typename... Args> std::pair<Iterator, bool> emplace(Args&&... args) {
    auto [it, inserted] = m_map.emplace(std::forward<Args>(args)..., DummyT{});
    return {Iterator(it), inserted};
  }

  Iterator begin() noexcept { return Iterator(m_map.begin()); }

  ConstIterator begin() const noexcept { return ConstIterator(m_map.begin()); }

  Iterator end() noexcept { return Iterator(m_map.end()); }
  ConstIterator end() const noexcept { return ConstIterator(m_map.end()); }
};
} // namespace hashset
