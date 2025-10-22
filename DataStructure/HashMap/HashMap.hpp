#pragma once
#include "../Array/DynamicArray.hpp"
#include "../LinkedList/SinglyLinkedList.hpp"
#include <concepts>
#include <functional>
#include <sstream>
#include <stdexcept>
#include <type_traits>
#include <utility>

namespace hashmap {

template <typename K, typename V> class HashMapKeyVal {
private:
  K m_key;
  V m_value;

public:
  template <typename Pair>
    requires std::constructible_from<K, decltype(std::get<0>(std::declval<Pair>()))> &&
                 std::constructible_from<V, decltype(std::get<1>(std::declval<Pair>()))>
  HashMapKeyVal(Pair&& p)
      : m_key(std::get<0>(std::forward<Pair>(p))), m_value(std::get<1>(std::forward<Pair>(p))) {}

  template <typename... Args>
  HashMapKeyVal(const K& key, Args&&... args) : m_key(key), m_value(std::forward<Args>(args)...) {}

  [[nodiscard]] K& getKey() noexcept { return m_key; }
  [[nodiscard]] const K& getKey() const noexcept { return m_key; }
  [[nodiscard]] V& getValue() noexcept { return m_value; }
  [[nodiscard]] const V& getValue() const noexcept { return m_value; }
};

template <
    typename K,
    typename V,
    typename Hasher = std::hash<K>,
    typename KeyEqual = std::equal_to<K>>
  requires std::predicate<KeyEqual, const K&, const K&> && std::invocable<Hasher, const K&> &&
           std::convertible_to<std::invoke_result_t<Hasher, const K&>, size_t>
class HashMap {

private:
  constexpr static const double maxLoadFactor = 0.75;
  using BucketList = linkedlist::SinglyLinkedList<HashMapKeyVal<K, V>>;
  Hasher m_hasher;
  KeyEqual m_keyEqual;
  size_t m_length = 0;
  array::DynamicArray<BucketList> m_table;

  template <bool IsConst> class ForwardIterator {

  private:
    using MapType = std::conditional_t<IsConst, const HashMap<K, V, Hasher>, HashMap<K, V, Hasher>>;

    using BucketIterator = std::
        conditional_t<IsConst, typename BucketList::ConstIterator, typename BucketList::Iterator>;

    MapType* m_map = nullptr;
    size_t m_bucketIdx;
    BucketIterator m_bucketIterator;

    void skipEmptyBucket() {
      if (m_map == nullptr) return;
      while (m_bucketIdx < m_map->m_table.getCapacity() &&
             m_bucketIterator == m_map->m_table[m_bucketIdx].end()) {
        m_bucketIdx++;
        if (m_bucketIdx < m_map->m_table.getCapacity())
          m_bucketIterator = m_map->m_table[m_bucketIdx].begin();
      }
    }

  public:
    using value_type = HashMapKeyVal<K, V>;
    using reference = std::conditional_t<IsConst, const value_type&, value_type&>;
    using pointer = std::conditional_t<IsConst, const value_type*, value_type*>;
    using difference_type = std::ptrdiff_t;
    using iterator_category = std::forward_iterator_tag;

    ForwardIterator(MapType* map, size_t idx, BucketIterator it)
        : m_map(map), m_bucketIdx(idx), m_bucketIterator(it) {
      skipEmptyBucket();
    }

    reference operator*() const { return *m_bucketIterator; }
    pointer operator->() const { return &(*m_bucketIterator); }

    ForwardIterator& operator++() {
      m_bucketIterator++;
      skipEmptyBucket();
      return *this;
    }

    ForwardIterator operator++(int) {
      ForwardIterator tmp = *this;
      ++(*this);
      return tmp;
    }

    bool operator==(const ForwardIterator<IsConst>& other) const {
      if (m_map == nullptr && other.m_map == nullptr) return true;
      return m_map == other.m_map && m_bucketIdx == other.m_bucketIdx &&
             m_bucketIterator == other.m_bucketIterator;
    }

    bool operator!=(const ForwardIterator<IsConst>& other) const { return !(*this == other); }
  };

  size_t spreadHash(const K& key) const {
    size_t hash = m_hasher(key);
    return hash ^ (hash >> 16);
  };

  BucketList& getList(const K& key) {
    size_t idx = getIndex(key);
    return m_table[idx];
  }

  const BucketList& getList(const K& key) const {
    size_t idx = getIndex(key);
    return m_table[idx];
  }

  void rehash(size_t newCapacity) {
    array::DynamicArray<BucketList> newTable;
    newTable.resize(newCapacity);

    for (BucketList& list : m_table) {
      for (const HashMapKeyVal<K, V>& data : list) {
        size_t newIdx = spreadHash(data.getKey()) & (newCapacity - 1);
        newTable[newIdx].pushFront(HashMapKeyVal<K, V>{data.getKey(), data.getValue()});
      }
    }
    m_table = std::move(newTable);
  }

  size_t getIndex(const K& key) const noexcept {
    return spreadHash(key) & (m_table.getCapacity() - 1);
  }
  [[nodiscard]] size_t getCapacity() const noexcept { return m_table.getCapacity(); }

  [[noreturn]] void throwKeyNotFound(const K& key) const {
    if constexpr (requires(std::ostream& os, const K& k) { os << k; }) {
      std::ostringstream oss;
      oss << "Key not found: " << key;
      throw std::out_of_range(oss.str());
    } else {
      throw std::out_of_range("Key not found (unprintable key)");
    }
  }

public:
  using Iterator = ForwardIterator<false>;
  using ConstIterator = ForwardIterator<true>;

  HashMap() { m_table.resize(m_table.getCapacity()); }

  V& at(const K& key) {
    Iterator it = find(key);
    if (it == end()) throwKeyNotFound(key);
    return it->getValue();
  }

  const V& at(const K& key) const {
    ConstIterator it = find(key);
    if (it == end()) throwKeyNotFound(key);
    return it->getValue();
  }

  ConstIterator find(const K& key) const {
    size_t idx = getIndex(key);
    const BucketList& list = m_table[idx];
    for (auto it = list.begin(); it != list.end(); it++) {
      if (m_keyEqual(it->getKey(), key)) return ConstIterator(this, idx, it);
    }
    return end();
  }

  Iterator find(const K& key) {
    size_t idx = getIndex(key);
    BucketList& list = m_table[idx];

    for (auto it = list.begin(); it != list.end(); it++) {
      if (m_keyEqual(it->getKey(), key)) return Iterator(this, idx, it);
    }

    return end();
  }

  template <typename Pair>
    requires std::constructible_from<HashMapKeyVal<K, V>, Pair&&>
  std::pair<Iterator, bool> insert(Pair&& kv) {
    if (m_length >= m_table.getCapacity() * HashMap::maxLoadFactor) {
      rehash(m_table.getCapacity() * 2);
    }
    Iterator it = find(kv.first);
    if (it != end()) return {it, false};
    BucketList& list = getList(kv.first);
    list.pushFront(HashMapKeyVal<K, V>(std::forward<Pair>(kv)));
    m_length++;
    return {Iterator(this, getIndex(kv.first), list.begin()), true};
  }

  std::pair<Iterator, bool> insert(const K& key, const V& value) {
    return this->insert(std::pair<K, V>(key, value));
  }

  std::pair<Iterator, bool> insert(K&& key, V&& value) {
    return this->insert(std::pair<K, V>(std::move(key), std::move(value)));
  }

  template <typename... Args> std::pair<Iterator, bool> emplace(const K& key, Args&&... args) {
    if (m_length >= m_table.getCapacity() * HashMap::maxLoadFactor) {
      rehash(m_table.getCapacity() * 2);
    }
    Iterator it = find(key);
    if (it != end()) return {it, false};
    BucketList& list = getList(key);
    HashMapKeyVal<K, V>& kv = list.emplaceFront(key, std::forward<Args>(args)...);
    m_length++;
    return {Iterator(this, getIndex(key), list.begin()), true};
  }

  [[nodiscard]] bool contains(const K& key) const noexcept { return find(key) != end(); }

  V& operator[](const K& key) {
    Iterator it = find(key);
    if (it != end()) return it->getValue();
    BucketList& list = getList(key);
    list.pushFront(HashMapKeyVal<K, V>{key, V{}});
    return list.front().getValue();
  }

  Iterator begin() {
    for (size_t i = 0; i < m_table.getCapacity(); i++) {
      auto it = m_table[i].begin();
      if (it != m_table[i].end()) return Iterator(this, i, it);
    }
    return end();
  }

  ConstIterator begin() const {
    for (size_t i = 0; i < m_table.getCapacity(); i++) {
      auto it = m_table[i].begin();
      if (it != m_table[i].end()) return ConstIterator(this, i, it);
    }
    return end();
  }

  Iterator end() {
    return Iterator(nullptr, m_table.getCapacity(), typename BucketList::Iterator());
  }
  ConstIterator end() const {
    return ConstIterator(nullptr, m_table.getCapacity(), typename BucketList::ConstIterator());
  }
};
} // namespace hashmap
