module;
#include "../hash_map/hash_map.hpp"
#include "../hash_set/hash_set.hpp"
#include "../queue/deque.hpp"
#include <functional>
#include <initializer_list>
#include <iostream>
#include <ranges>
#include <stdexcept>

export module graph;
export import :utils;
export import :detail;

// TODO:
// have src/ folder
// graph iterator
/* implement graph
- Floyd-Warshall vs dijkstra
- prim's algo
- spanning tree
- minimum spanning tree */
// should consider whether graph can have self-loop
// get edges
// find by value
// add value_type

export namespace graph {

template <typename T> class Node {
public:
  using value_type = T;

private:
  value_type m_val;
  hashset::HashSet<Node<value_type>*> m_neighbors;

public:
  Node(const T& val) : m_val(val) {}
  Node(const Node& other) = delete;
  Node(Node&& other) = delete;
  Node& operator=(const Node&) = delete;
  Node& operator=(Node&&) = delete;
  ~Node() noexcept = default;

  constexpr hashset::HashSet<Node<value_type>*>& neighbors() noexcept { return m_neighbors; }
  constexpr const hashset::HashSet<Node<value_type>*>& neighbors() const noexcept { return m_neighbors; }
  constexpr value_type val() const noexcept { return m_val; }
};

template <typename T> class Graph {
protected:
  array::DynamicArray<Node<T>*> m_nodes;

public:
  Graph() = default;

  // I think it is fine to put copy/move constructor in the base abstract Graph
  // there's no need to worry client copying DirectedGraph from UndirectedGraph, because they can't anyway
  Graph(const Graph& other) {
    hashmap::HashMap<Node<T>*, Node<T>*> oldToNew;
    m_nodes.reserve(other.m_nodes.size());

    try {
      for (Node<T>* otherNode : other.m_nodes) {
        Node<T>* newNode = new Node<T>(otherNode->val()); // NOLINT
        m_nodes.pushBack(newNode);
        oldToNew[otherNode] = newNode;
      }

      for (size_t i = 0; i < m_nodes.size(); ++i) {
        Node<T>* otherNode = other.m_nodes[i];
        Node<T>* newNode = m_nodes[i];
        newNode->neighbors().reserve(otherNode->neighbors().size());

        for (Node<T>* nei : otherNode->neighbors()) { newNode->neighbors().insert(oldToNew[nei]); }
      }
    } catch (...) {
      clear();
      throw;
    }
  }

  Graph(Graph&& other) noexcept : m_nodes(std::move(other.m_nodes)) {}

  Graph& operator=(const Graph& other) = delete;
  Graph& operator=(Graph&& other) = delete;

  virtual ~Graph() noexcept { clear(); }

  constexpr void clear() noexcept {
    if (empty()) return;
    // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
    for (Node<T>* node : m_nodes) { delete node; }
    m_nodes.clear();
  }

  Node<T>* addVertex(const T& value) {
    Node<T>* newNode = new Node<T>(value); // NOLINT
    m_nodes.pushBack(newNode);
    return newNode;
  }

  bool removeVertex(Node<T>* vertex) {
    size_t targetIdx = m_nodes.size();
    for (size_t i = 0; i < m_nodes.size(); ++i) {
      Node<T>* node = m_nodes[i];
      if (node == vertex) {
        targetIdx = i;
      } else {
        node->neighbors().erase(vertex);
      }
    }

    if (targetIdx == m_nodes.size()) return false;

    // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
    delete m_nodes[targetIdx];
    m_nodes.erase(m_nodes.begin() + targetIdx);
    return true;
  }

  virtual void addEdge(Node<T>* src, Node<T>* dest) = 0;
  virtual void removeEdge(Node<T>* src, Node<T>* dest) = 0;

  template <
      std::ranges::forward_range Seq, typename Hasher = std::hash<T>, typename KeyEqual = std::equal_to<T>>
    requires detail::Hasher<T, Hasher> && detail::KeyEqual<T, KeyEqual> &&
             detail::PairLikeForwardRange<Seq, T>
  void fromEdges(const Seq& edges, Hasher hasher = {}, KeyEqual eq = {}) {
    size_t n = std::ranges::distance(edges);
    clear();
    hashmap::HashMap<T, Node<T>*, Hasher, KeyEqual> valueToNode{n, std::move(hasher), std::move(eq)};

    auto getOrCreateVertex = [&](const T& val) -> Node<T>* {
      if (valueToNode.contains(val)) return valueToNode[val];
      Node<T>* vertex = addVertex(val);
      valueToNode[val] = vertex;
      return vertex;
    };

    for (const auto& [val1, val2] : edges) {
      Node<T>* vertex1 = getOrCreateVertex(val1);
      Node<T>* vertex2 = getOrCreateVertex(val2);
      addEdge(vertex1, vertex2);
    }
  }

  bool hasEdge(Node<T>* src, Node<T>* dest) const noexcept {
    if (src == nullptr || dest == nullptr) return false;
    return src->neighbors().contains(dest);
  }

  array::DynamicArray<Node<T>*> getNeighbors(const Node<T>* vertex) const {
    if (vertex == nullptr) throw std::invalid_argument("vertex can not be nullptr");
    array::DynamicArray<Node<T>*> neighbors;
    neighbors.reserve(vertex->neighbors().size());
    for (Node<T>* nei : vertex->neighbors()) neighbors.pushBack(nei);
    return neighbors;
  }

  // should make sure node.val is streamable
  void printGraph() const noexcept {
    for (Node<T>* node : m_nodes) {
      std::cout << node->val() << " -> ";
      size_t n = node->neighbors().size();
      size_t i = 0;
      for (Node<T>* nei : node->neighbors()) {
        if (i != 0) std::cout << ", ";
        std::cout << nei->val();
        ++i;
      }
      std::cout << '\n';
    }
  }

  constexpr array::DynamicArray<const Node<T>*> nodes() const {
    array::DynamicArray<const Node<T>*> res;
    res.reserve(m_nodes.size());
    for (Node<T>* node : m_nodes) res.pushBack(node);
    return res;
  }

  [[nodiscard]] virtual constexpr bool hasCycle() const noexcept = 0;
  [[nodiscard]] constexpr virtual bool directed() const noexcept = 0;

  [[nodiscard]] constexpr size_t size() const noexcept { return m_nodes.size(); }
  [[nodiscard]] constexpr bool empty() const noexcept { return m_nodes.size() == 0; }

  [[nodiscard]] constexpr size_t vertexCount() const noexcept { return m_nodes.size(); }
  [[nodiscard]] virtual constexpr size_t edgeCount() const noexcept = 0;
};

/**
 * @tparam T The underlying data type stored within the graph nodes.
 * @brief A Directed Graph implementation where edges have a specific direction.
 *
 * @details This graph models relationships using the **Flow Direction** convention:
 * - An edge from node A to node B (`src -> dest`) means **A must happen BEFORE B**.
 * - Therefore, calling `node->neighbors()` returns the **downstream tasks** that depend on this node.
 * - To find prerequisites instead, you can inspect the incoming degrees (`inDegree`).
 *
 * @note This structure is perfectly optimized for post-order DFS topological sorting.
 */
template <typename T> class DirectedGraph : public Graph<T> {
public:
  using Graph<T>::Graph;
  DirectedGraph(std::initializer_list<std::pair<T, T>> edges) { this->fromEdges(edges); }
  DirectedGraph& operator=(DirectedGraph other) {
    this->swap(other);
    return *this;
  }

  void addEdge(Node<T>* src, Node<T>* dest) override {
    if (src == nullptr || dest == nullptr) return;
    src->neighbors().insert(dest);
  }

  void removeEdge(Node<T>* src, Node<T>* dest) override {
    if (src == nullptr || dest == nullptr) return;
    src->neighbors().erase(dest);
  }

  [[nodiscard]] constexpr bool directed() const noexcept override { return true; }

  [[nodiscard]] constexpr size_t edgeCount() const noexcept override {
    size_t count = 0;
    for (Node<T>* node : this->m_nodes) count += node->neighbors().size();
    return count;
  }

  constexpr size_t inDegree(const Node<T>* vertex) const noexcept {
    if (vertex == nullptr) return 0;
    size_t count = 0;
    for (const Node<T>* node : this->m_nodes) {
      if (node->neighbors().contains(vertex)) ++count;
    }
    return count;
  }

  constexpr size_t outDegree(const Node<T>* vertex) const noexcept {
    if (vertex == nullptr) return 0;
    return vertex->neighbors().size();
  }

  constexpr size_t totalDegree(const Node<T>* vertex) const noexcept {
    if (vertex == nullptr) return 0;
    return inDegree(vertex) + outDegree(vertex);
  }

  [[nodiscard]] constexpr bool hasCycle() const noexcept override {
    auto [valid, _] = graph::utils::topologicalSort(*this);
    return !valid;
  }

  constexpr void swap(DirectedGraph& other) noexcept {
    using std::swap;
    swap(this->m_nodes, other.m_nodes);
  }

  friend void swap(DirectedGraph& a, DirectedGraph& b) noexcept { a.swap(b); }
};

template <typename T> class UndirectedGraph : public Graph<T> {
public:
  using Graph<T>::Graph;
  UndirectedGraph(std::initializer_list<std::pair<T, T>> edges) { this->fromEdges(edges); }

  UndirectedGraph& operator=(UndirectedGraph other) {
    this->swap(other);
    return *this;
  }

  void addEdge(Node<T>* src, Node<T>* dest) override {
    if (src == nullptr || dest == nullptr) return;
    src->neighbors().insert(dest);
    if (src != dest) dest->neighbors().insert(src);
  }

  void removeEdge(Node<T>* src, Node<T>* dest) override {
    if (src == nullptr || dest == nullptr) return;
    src->neighbors().erase(dest);
    if (src != dest) dest->neighbors().erase(src);
  }

  [[nodiscard]] constexpr bool directed() const noexcept override { return false; }

  [[nodiscard]] constexpr size_t edgeCount() const noexcept override {
    size_t totalNeighbors = 0;
    size_t selfLoops = 0;
    for (Node<T>* node : this->m_nodes) {
      totalNeighbors += node->neighbors().size();
      if (node->neighbors().contains(node)) ++selfLoops;
    }
    return ((totalNeighbors - selfLoops) / 2) + selfLoops;
  }

  constexpr size_t degree(const Node<T>* vertex) const noexcept {
    if (vertex == nullptr) return 0;
    size_t count = 0;
    for (const Node<T>* node : vertex->neighbors()) {
      if (node == vertex) count += 2;
      else ++count;
    }
    return count;
  }

  [[nodiscard]] constexpr bool hasCycle() const noexcept override {
    hashset::HashSet<const Node<T>*> visited;
    visited.reserve(this->size());

    std::function<bool(const Node<T>*, const Node<T>*)> detectCycle = [&](const Node<T>* node,
                                                                          const Node<T>* parent) -> bool {
      visited.insert(node);
      for (const Node<T>* nei : node->neighbors()) {
        if (nei == parent) continue;
        if (visited.contains(nei)) return true;
        if (detectCycle(nei, node)) return true;
      }
      return false;
    };

    for (const Node<T>* node : this->m_nodes) {
      if (!visited.contains(node)) {
        if (detectCycle(node, nullptr)) return true;
      }
    }
    return false;
  }

  constexpr void swap(UndirectedGraph& other) noexcept {
    using std::swap;
    swap(this->m_nodes, other.m_nodes);
  }

  friend void swap(UndirectedGraph& a, UndirectedGraph& b) noexcept { a.swap(b); }
};

} // namespace graph
