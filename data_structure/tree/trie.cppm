module;

#include "../hash_map/hash_map.hpp"
#include "../queue/deque.hpp"
#include "./detail.hpp"
#include <concepts>
#include <cstddef>
#include <iterator>
#include <ranges>
#include <type_traits>
#include <utility>

export module trie;
namespace tree::detail {
template <typename T> using ValT = std::ranges::range_value_t<T>;

template <typename Seq, typename Elem>
concept ReconstructibleSeq =
    std::ranges::forward_range<Seq> &&
    (std::constructible_from<Seq, array::DynamicArray<Elem>> ||
     requires(array::DynamicArray<Elem>& seq) { Seq(std::ranges::begin(seq), std::ranges::end(seq)); });
} // namespace tree::detail

namespace tree {

// forward declaration
export template <typename Seq, typename Hasher, typename KeyEqual>
  requires detail::ReconstructibleSeq<Seq, detail::ValT<Seq>> && detail::Hasher<detail::ValT<Seq>, Hasher> &&
           detail::KeyEqual<detail::ValT<Seq>, KeyEqual>
class Trie;

namespace detail {

template <typename T> struct IsTrieSpecialization : std::false_type {};
template <typename T, typename U, typename V> struct IsTrieSpecialization<Trie<T, U, V>> : std::true_type {};

template <typename T>
concept IsTrie = IsTrieSpecialization<T>::value;

template <bool IsConst, IsTrie ParentTrie> class TrieIterator {
  // for conversion constructor
  template <bool, IsTrie> friend class TrieIterator;

public:
  using iterator_category = std::forward_iterator_tag;
  using value_type = ParentTrie::value_type;
  using difference_type = ParentTrie::difference_type;
  using pointer = void;
  using view_type = ParentTrie::view_type;
  using reference = view_type;
  using allocator_type = ParentTrie::allocator_type;

private:
  using NodePtr =
      std::conditional_t<IsConst, const typename ParentTrie::NodeType*, typename ParentTrie::NodeType*>;
  using Element = ParentTrie::Element;
  using MapIterator = decltype(std::declval<NodePtr>()->children().begin());

  struct StackFrame {
    // NOLINTNEXTLINE(misc-non-private-member-variables-in-classes)
    NodePtr node;
    // NOLINTNEXTLINE(misc-non-private-member-variables-in-classes)
    MapIterator iter;
    constexpr bool operator==(const StackFrame& other) const noexcept {
      return node == other.node && iter == other.iter;
    }
  };

  queue::Deque<StackFrame> m_stack;
  array::DynamicArray<Element> m_path;

public:
  TrieIterator(allocator_type alloc)
  // TODO: make then pmr-aware
  // : m_stack(alloc), m_path(alloc)
  {};

  TrieIterator(const NodePtr nodePtr, allocator_type alloc = {})
  // TODO: make then pmr-aware
  // : m_stack(alloc), m_path(alloc)
  {
    if (nodePtr != nullptr) {
      m_stack.emplaceBack(nodePtr, nodePtr->children().begin());
      if (!nodePtr->endOfWord()) ++(*this);
    }
  }

  // conversion constructor
  TrieIterator(const TrieIterator<false, ParentTrie>& other)
    requires IsConst
      : m_stack(other.m_stack), m_path(other.m_path) {}

  reference operator*() const { return reference{m_path.data(), m_path.size()}; }

  TrieIterator& operator++() {
    while (!m_stack.empty()) {
      auto& [nodePtr, iter] = m_stack.back();

      if (iter != nodePtr->children().end()) {
        auto const& [key, nextNode] = *iter;
        ++iter;

        m_stack.emplaceBack(nextNode, nextNode->children().begin());
        m_path.emplaceBack(key);
        if (nextNode->endOfWord()) return *this;
      } else {
        m_stack.popBack();
        if (!m_path.empty()) m_path.popBack();
      }
    }
    return *this;
  }

  TrieIterator operator++(int) {
    TrieIterator snapshot = *this;
    ++(*this);
    return snapshot;
  }

  bool operator==(const TrieIterator& other) const noexcept {
    if (m_stack.empty() || other.m_stack.empty()) return m_stack.empty() == other.m_stack.empty();

    // the path to any specific node is unique
    return m_stack.back() == other.m_stack.back();
  };
  bool operator!=(const TrieIterator& other) const noexcept { return !(*this == other); }
};

} // namespace detail

template <typename T, typename Hasher = std::hash<T>, typename KeyEqual = std::equal_to<T>>
  requires detail::Hasher<T, Hasher> && detail::KeyEqual<T, KeyEqual>
class TrieNode {
public:
  using allocator_type = std::pmr::polymorphic_allocator<std::byte>;

private:
  allocator_type m_alloc;
  // TODO: make HashMap pmr-aware and propagate alloc
  hashmap::HashMap<T, TrieNode*, Hasher, KeyEqual> m_children;
  bool m_endOfWord = false;

public:
  TrieNode(Hasher hasher = {}, KeyEqual eq = {}, allocator_type alloc = {})
      : m_alloc(alloc), m_children({2, std::move(hasher), std::move(eq)}) {};

  TrieNode(allocator_type alloc) : TrieNode({}, {}, alloc) {}

  TrieNode(const TrieNode& other, allocator_type alloc)
      : m_alloc(alloc),
        m_children({other.m_children.size(), other.m_children.hashFunction(), other.m_children.keyEq()}),
        m_endOfWord(other.m_endOfWord) {
    try {
      for (auto const& [key, childNodePtr] : other.m_children) {
        TrieNode* newNode = m_alloc.new_object<TrieNode>(*childNodePtr);
        m_children.emplace(key, newNode);
      }
    } catch (...) {
      clear();
      throw;
    }
  }

  TrieNode(const TrieNode& other) : TrieNode(other, other.m_alloc) {}

  TrieNode(TrieNode&& other) noexcept
      : m_alloc(other.m_alloc), m_children(std::move(other.m_children)),
        m_endOfWord(std::exchange(other.m_endOfWord, false)) {}

  TrieNode(TrieNode&& other, allocator_type alloc)
      : m_alloc(alloc),
        m_children(
            m_alloc == other.m_alloc
                ? std::move(other.m_children)
                : hashmap::HashMap<
                      T, TrieNode*, Hasher,
                      KeyEqual>{other.m_children.size(), other.m_children.hashFunction(), other.m_children.keyEq()}
        ),
        m_endOfWord(std::exchange(other.m_endOfWord, false)) {
    if (alloc == other.m_alloc) return;

    try {
      for (auto const& [key, childNodePtr] : other.m_children) {
        TrieNode* newNode = m_alloc.new_object<TrieNode>(std::move(*childNodePtr));
        m_children.emplace(key, newNode);
      }
    } catch (...) {
      clear();
      throw;
    }
  }

  TrieNode& operator=(const TrieNode& other) {
    if (this == &other) return *this;

    TrieNode copy(other, m_alloc);
    swap(copy);
    return *this;
  }

  TrieNode& operator=(TrieNode&& other) {
    if (this == &other) return *this;

    if (m_alloc == other.m_alloc) {
      swap(other);
    } else {
      // reallocation then swap
      TrieNode copy(std::move(other), m_alloc);
      swap(copy);
    }
    return *this;
  }

  ~TrieNode() noexcept { clear(); };

  void clear() noexcept {
    for (auto& [key, childNodePtr] : m_children) {
      if (childNodePtr == nullptr) continue;
      m_alloc.delete_object(childNodePtr);
    }
    m_children.clear();
  }

  constexpr bool contains(const T& key) const noexcept { return m_children.contains(key); }

  [[nodiscard]] constexpr bool endOfWord() const noexcept { return m_endOfWord; }

  constexpr hashmap::HashMap<T, TrieNode*, Hasher, KeyEqual>& children() { return m_children; }
  constexpr const hashmap::HashMap<T, TrieNode*, Hasher, KeyEqual>& children() const { return m_children; }

  void insert(const T& key) {
    if (m_children.contains(key)) return;
    // m_alloc is auto injected
    TrieNode* newNode = m_alloc.new_object<TrieNode>(m_children.hashFunction(), m_children.keyEq());
    m_children.insert(key, newNode);
  }

  constexpr TrieNode* operator[](const T& key) { return m_children[key]; }
  constexpr const TrieNode* operator[](const T& key) const { return m_children[key]; }

  constexpr void setEndOfWord(bool isEnd) noexcept { m_endOfWord = isEnd; }

  void swap(TrieNode& other) noexcept {
    using std::swap;
    // don't swap allocators
    swap(m_children, other.m_children);
    swap(m_endOfWord, other.m_endOfWord);
  }

  // for ADL
  friend void swap(TrieNode& a, TrieNode& b) noexcept { a.swap(b); }
};

export template <
    typename Seq, typename Hasher = std::hash<detail::ValT<Seq>>,
    typename KeyEqual = std::equal_to<detail::ValT<Seq>>>
  requires detail::ReconstructibleSeq<Seq, detail::ValT<Seq>> && detail::Hasher<detail::ValT<Seq>, Hasher> &&
           detail::KeyEqual<detail::ValT<Seq>, KeyEqual>
class Trie {
  template <bool IsConst, detail::IsTrie ParentTrie> friend class detail::TrieIterator;

public:
  using value_type = Seq;
  using size_type = size_t;
  using difference_type = ptrdiff_t;
  using reference = value_type;
  using const_reference = value_type;
  using pointer = void;
  using const_pointer = void;
  using iterator = detail::TrieIterator<false, Trie>;
  using const_iterator = detail::TrieIterator<true, Trie>;
  using allocator_type = std::pmr::polymorphic_allocator<std::byte>;
  using view_type = std::conditional_t<
      std::is_same_v<Seq, std::string>, std::string_view, std::span<const detail::ValT<Seq>>>;

private:
  allocator_type m_alloc;
  [[no_unique_address]] Hasher m_hasher;
  [[no_unique_address]] KeyEqual m_keyEqual;
  using Element = detail::ValT<value_type>;
  using NodeType = TrieNode<Element, Hasher, KeyEqual>;
  NodeType* m_root = nullptr;
  size_type m_size = 0;

  constexpr void collectSeq(
      const NodeType* node, array::DynamicArray<Element>& path, array::DynamicArray<value_type>& sequences
  ) const {
    using PathType = std::remove_cvref_t<decltype(path)>;

    if (node->endOfWord()) {
      if constexpr (std::constructible_from<value_type, PathType>) {
        sequences.emplaceBack(path);
      } else {
        sequences.emplaceBack(path.begin(), path.end());
      }
    }

    for (auto const& [key, childNodePtr] : node->children()) {
      path.emplaceBack(key);
      collectSeq(childNodePtr, path, sequences);
      path.popBack();
    }
  }

public:
  Trie(Hasher hasher = {}, KeyEqual eq = {}, allocator_type alloc = {})
      : m_alloc(alloc), m_hasher(std::move(hasher)), m_keyEqual(std::move(eq)),
        m_root(m_alloc.new_object<NodeType>(m_hasher, m_keyEqual)) {}

  Trie(allocator_type alloc) : Trie({}, {}, alloc) {}

  Trie(const Trie& other, allocator_type alloc)
      : m_alloc(alloc), m_hasher(other.m_hasher), m_keyEqual(other.m_keyEqual), m_root(nullptr),
        m_size(other.m_size) {
    if (other.m_root == nullptr) return;
    m_root = m_alloc.new_object<NodeType>(*other.m_root);
  }

  Trie(const Trie& other) : Trie(other, other.m_alloc) {}

  Trie(Trie&& other, allocator_type alloc)
      : m_alloc(alloc), m_hasher(std::move(other.m_hasher)), m_keyEqual(std::move(other.m_keyEqual)),
        m_root(nullptr), m_size(std::exchange(other.m_size, 0)) {
    if (m_alloc == other.m_alloc) {
      m_root = std::exchange(other.m_root, nullptr);
    } else {
      if (other.m_root == nullptr) return;
      m_root = m_alloc.new_object<NodeType>(std::move(*other.m_root));
    }
  }

  Trie(Trie&& other) noexcept
      : m_alloc(other.m_alloc), m_hasher(std::move(other.m_hasher)), m_keyEqual(std::move(other.m_keyEqual)),
        m_root(std::exchange(other.m_root, nullptr)), m_size(std::exchange(other.m_size, 0)) {}

  Trie& operator=(const Trie& other) {
    if (this == &other) return *this;

    Trie copy(other, m_alloc);
    swap(copy);

    return *this;
  }

  Trie& operator=(Trie&& other) {
    if (this == &other) return *this;

    if (m_alloc == other.m_alloc) {
      swap(other);
    } else {
      // reallocation then swap
      Trie copy(std::move(other), m_alloc);
      swap(copy);
    }
    return *this;
  }

  ~Trie() noexcept { clear(); }

  void clear() noexcept {
    if (m_root == nullptr) return;
    m_alloc.delete_object(m_root);
    m_root = nullptr;
    m_size = 0;
  }

  constexpr void insert(const value_type& seq) {
    NodeType* node = m_root;
    for (const Element& e : seq) {
      if (!node->contains(e)) node->insert(e);
      node = (*node)[e];
    }
    node->setEndOfWord(true);
    ++m_size;
  }

  [[nodiscard]] constexpr bool search(const value_type& seq) const {
    NodeType* node = m_root;
    for (const Element& e : seq) {
      if (!node->contains(e)) return false;
      node = (*node)[e];
    }
    return node->endOfWord();
  }

  [[nodiscard]] constexpr bool startsWith(const value_type& seq) const {
    NodeType* node = m_root;
    for (const Element& e : seq) {
      if (!node->contains(e)) return false;
      node = (*node)[e];
    }
    return true;
  }

  [[nodiscard]] constexpr size_type size() const noexcept { return m_size; }

  [[nodiscard]] constexpr bool empty() const noexcept { return m_size == 0; }

  constexpr array::DynamicArray<value_type> getAllWithPrefix(const value_type& prefix) const {
    NodeType* node = m_root;
    array::DynamicArray<value_type> sequences;

    for (const Element& e : prefix) {
      if (!node->contains(e)) return sequences;
      node = (*node)[e];
    }

    array::DynamicArray<Element> path(std::ranges::begin(prefix), std::ranges::end(prefix));

    collectSeq(node, path, sequences);
    return sequences;
  }

  [[nodiscard]] allocator_type getAllocator() const noexcept { return m_alloc; }

  iterator begin() { return {m_root, m_alloc}; }
  iterator end() noexcept { return {m_alloc}; }

  const_iterator begin() const { return {m_root, m_alloc}; }
  const_iterator end() const noexcept { return {m_alloc}; }

  const_iterator cbegin() const { return {m_root, m_alloc}; }
  const_iterator cend() const noexcept { return {m_alloc}; }

  constexpr void swap(Trie& other) noexcept {
    using std::swap;
    // don't swap allocators
    swap(m_root, other.m_root);
    swap(m_hasher, other.m_hasher);
    swap(m_keyEqual, other.m_keyEqual);
    swap(m_size, other.m_size);
  }

  // for ADL
  constexpr friend void swap(Trie& a, Trie& b) noexcept { a.swap(b); }
};

} // namespace tree
