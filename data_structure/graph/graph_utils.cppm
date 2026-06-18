module;
#include "../array/dynamic_array.hpp"
#include "../hash_set/hash_set.hpp"
#include "../queue/deque.hpp"
#include <cstdint>
#include <ranges>
#include <utility>
export module graph:utils;

export namespace graph {
template <typename T> class DirectedGraph; // Forward declaration
template <typename T> class Node;          // Forward declaration

namespace utils {

enum class VisitState : uint8_t { Unvisited, Visiting, Visited };

/**
 *  @tparam T  The underlying data type stored within the graph nodes.
 *  @brief Computes a linear topological ordering of vertices using a post-order Depth-First Search (DFS).
 *  @param graph  A constant reference to the DirectedGraph container to be sorted.
 *  @return A std::pair where:
 *          - The first element (bool) is true if the sequence is a valid topological order (acyclic),
 *            and false if a cycle was detected.
 *          - The second element is the sorted array of constant node pointers (topological order).
 */
template <typename T>
std::pair<bool, array::DynamicArray<const Node<T>*>> topologicalSort(const DirectedGraph<T>& graph) {
  size_t n = graph.size();
  hashmap::HashMap<const Node<T>*, VisitState> visitState;
  visitState.reserve(n);
  for (const Node<T>* node : graph.nodes()) visitState[node] = VisitState::Unvisited;

  array::DynamicArray<const Node<T>*> topologicalOrder;

  std::function<bool(const Node<T>*)> detectCycle = [&](const Node<T>* node) -> bool {
    if (visitState[node] == VisitState::Visited) return false;
    if (visitState[node] == VisitState::Visiting) return true;
    visitState[node] = VisitState::Visiting;
    for (const Node<T>* nei : node->neighbors()) {
      if (detectCycle(nei)) return true;
    }
    visitState[node] = VisitState::Visited;
    topologicalOrder.pushBack(node);
    return false;
  };

  topologicalOrder.reserve(n);
  for (const Node<T>* node : graph.nodes()) {
    if (visitState[node] == VisitState::Unvisited) {
      if (detectCycle(node)) return {false, {}};
    }
  }

  std::ranges::reverse(topologicalOrder);

  return {true, topologicalOrder};
}

/**
 *  @tparam T  The underlying data type stored within the graph nodes.
 *  @brief Computes a linear topological ordering of vertices using a post-order Breath-First Search (BFS).
 *  @param graph  A constant reference to the DirectedGraph container to be sorted.
 *  @return A std::pair where:
 *          - The first element (bool) is true if the sequence is a valid topological order (acyclic),
 *            and false if a cycle was detected.
 *          - The second element is the sorted array of constant node pointers (topological order).
 */
template <typename T>
std::pair<bool, array::DynamicArray<const Node<T>*>> kahn(const DirectedGraph<T>& graph) {
  size_t n = graph.size();
  hashmap::HashMap<const Node<T>*, size_t> inDegree;
  inDegree.reserve(n);
  for (const Node<T>* node : graph.nodes()) inDegree[node] = 0;
  for (const Node<T>* node : graph.nodes()) {
    for (const Node<T>* nei : node->neighbors()) ++inDegree[nei];
  }
  array::DynamicArray<const Node<T>*> topologicalOrder;
  topologicalOrder.reserve(n);

  queue::Deque<const Node<T>*> q;
  for (auto [node, count] : inDegree) {
    if (count == 0) q.pushBack(node);
  }

  while (!q.empty()) {
    const Node<T>* node = q.front();
    q.popFront();
    topologicalOrder.pushBack(node);
    for (const Node<T>* nei : node->neighbors()) {
      --inDegree[nei];
      if (inDegree[nei] == 0) q.pushBack(nei);
    }
  }
  if (topologicalOrder.size() == n) return {true, topologicalOrder};
  return {false, {}};
}

template <typename T> struct IsNodePointerSpecialization : std::false_type {};
template <typename T> struct IsNodePointerSpecialization<Node<T>*> : std::true_type {
  using type = T;
};
template <typename T> struct IsNodePointerSpecialization<const Node<T>*> : std::true_type {
  using type = T;
};

template <typename T>
concept NodePointer = IsNodePointerSpecialization<std::remove_cvref_t<T>>::value;

template <std::ranges::forward_range Seq>
  requires NodePointer<std::ranges::range_value_t<Seq>>
bool isValidTopologicalOrder(const Seq& topologicalOrder) {
  using Element = std::remove_cvref_t<std::ranges::range_value_t<Seq>>;
  using T = IsNodePointerSpecialization<Element>::type;

  size_t n = topologicalOrder.size();
  hashset::HashSet<const Node<T>*> seen;
  seen.reserve(n);

  for (const Node<T>* node : topologicalOrder) {
    for (const Node<T>* nei : node->neighbors()) {
      if (seen.contains(nei)) return false;
    }
    seen.insert(node);
  }
  return true;
}

template <typename T>
array::DynamicArray<array::DynamicArray<const Node<T>*>> allTopologicalOrders(const DirectedGraph<T>& graph) {
  size_t n = graph.size();
  array::DynamicArray<array::DynamicArray<const Node<T>*>> allOrders;
  if (n == 0) return allOrders;

  hashmap::HashMap<const Node<T>*, size_t> indegree;
  indegree.reserve(n);
  for (size_t i = 0; i < n; ++i) indegree[graph.nodes()[i]] = 0;
  for (size_t i = 0; i < n; ++i) {
    const Node<T>* node = graph.nodes()[i];
    for (const Node<T>* nei : node->neighbors()) ++indegree[nei];
  }

  array::DynamicArray<const Node<T>*> currentOrder;
  currentOrder.reserve(n);

  hashset::HashSet<const Node<T>*> visited;
  visited.reserve(n);

  std::function<void()> findAllOrders = [&]() -> void {
    if (currentOrder.size() == n) {
      allOrders.pushBack(currentOrder);
      return;
    }

    for (const Node<T>* node : graph.nodes()) {
      if (visited.contains(node) || indegree[node] != 0) continue;
      for (const Node<T>* nei : node->neighbors()) --indegree[nei];
      visited.insert(node);
      currentOrder.pushBack(node);

      findAllOrders();
      visited.erase(node);
      currentOrder.popBack();
      for (const Node<T>* nei : node->neighbors()) ++indegree[nei];
    }
  };
  findAllOrders();

  return allOrders;
}

} // namespace utils

} // namespace graph
