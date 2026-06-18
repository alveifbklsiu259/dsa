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

  template <bool IsConst> class Iterator {

  private:
    using MapIterator = std::conditional_t<
        IsConst, typename hashmap::HashMap<Key, DummyT, Hasher, KeyEqual>::const_iterator,
        typename hashmap::HashMap<Key, DummyT, Hasher, KeyEqual>::iterator>;

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

public:
  using value_type = Key;
  using size_type = size_t;
  using difference_type = ptrdiff_t;
  using reference = value_type&;
  using const_reference = const value_type&;
  using pointer = value_type*;
  using const_pointer = const value_type*;
  using iterator = Iterator<false>;
  using const_iterator = Iterator<true>;

  HashSet() = default;

  HashSet(size_t n, const Hasher& hasher = Hasher{}, const KeyEqual& eq = KeyEqual{})
      : m_map(n, hasher, eq) {}

  template <std::input_iterator InputIt>
    requires std::constructible_from<Key, std::iter_value_t<InputIt>>
  HashSet(
      InputIt first, InputIt last, size_t n = 2, const Hasher& hasher = Hasher{},
      const KeyEqual& eq = KeyEqual{}
  )
      : HashSet(n, hasher, eq) {
    for (auto it = first; it != last; ++it) emplace(*it);
  }

  HashSet(std::initializer_list<Key> init, const Hasher& hasher = Hasher{}, const KeyEqual& eq = KeyEqual{})
      : HashSet(init.begin(), init.end(), init.size(), hasher, eq) {};

  bool contains(const Key& key) const noexcept { return m_map.contains(key); }

  size_t erase(const Key& key) { return m_map.erase(key); }

  void clear() { m_map.clear(); }

  [[nodiscard]] size_t size() const noexcept { return m_map.size(); }

  std::pair<iterator, bool> insert(const Key& key) { return emplace(key); }
  std::pair<iterator, bool> insert(Key&& key) { return emplace(std::move(key)); }

  template <typename... Args> std::pair<iterator, bool> emplace(Args&&... args) {
    auto [it, inserted] = m_map.emplace(std::forward<Args>(args)..., DummyT{});
    return {iterator{it}, inserted};
  }

  constexpr void reserve(size_t n) { m_map.reserve(n); }

  iterator begin() noexcept { return iterator{m_map.begin()}; }
  const_iterator begin() const noexcept { return const_iterator{m_map.begin()}; }
  const_iterator cbegin() const noexcept { return const_iterator{m_map.cbegin()}; }

  iterator end() noexcept { return iterator{m_map.end()}; }
  const_iterator end() const noexcept { return const_iterator{m_map.end()}; }
  const_iterator cend() const noexcept { return const_iterator{m_map.cend()}; }
};
} // namespace hashset
