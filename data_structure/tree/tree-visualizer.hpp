#include "binary_tree.hpp"
#include <cmath>
#include <format>
#include <sstream>

namespace tree {

namespace detail {

template <typename T>
concept Streamable = requires(std::ostream& os, T val) {
  { os << val } -> std::same_as<std::ostream&>;
};

template <Streamable T> std::string genericToString(const T& val) {
  std::ostringstream oss;
  oss << val;
  return oss.str();
}

} // namespace detail

// strategy pattern?

class TreeVisualizer {
private:
  size_t m_minNodeSize;
  size_t m_minNodeGap;
  bool m_displayEmptyNode = false;

  /**
   * @param level The depth of the tree (0-indexed, where 0 is the root).
   *
   * @brief get the node count at a layer from a perfect binary tree, i.e. No empty node at any layer.
   */
  [[nodiscard]] constexpr static size_t getMaxNodesAtLevel(size_t level) {
    return static_cast<size_t>(std::pow(2, level));
  }

  template <typename T> size_t findMinNodeSize(const BinaryTree<T>& tree) const {
    size_t minNodeSize = m_minNodeSize;
    auto checkSize = [&](Node<T>& node) {
      minNodeSize = std::max(minNodeSize, detail::genericToString(node.value).size());
    };
    tree.levelorderTraverse(checkSize);
    return minNodeSize;
  }

  [[nodiscard]] size_t findMinNodeGap(bool isMinNodeSizeEven) const {
    size_t minNodeGap = isMinNodeSizeEven ? 4 : 3;
    if (m_minNodeSize <= minNodeGap) return minNodeGap;

    // make sure nodeSize and nodeGap are either both even or odd
    bool isMinNodeGapEven = m_minNodeGap % 2 == 0;
    if (isMinNodeGapEven == isMinNodeSizeEven) {
      minNodeGap = m_minNodeGap;
    } else {
      minNodeGap = m_minNodeGap + 1;
    }
    return minNodeGap;
  }

  /**
   * @param level The depth of the tree (0-indexed, where 0 is the root).
   *
   * Calculates the total horizontal width (in characters) that a node "owns"
   * at a given level. This "span" ensures that the branches and leaves
   * below it have enough room to spread out without overlapping.
   *
   * All the nodes in the same level share the same span.
   *
   * Example for a tree of height 3:
   *
   * Level 0 (Root): spans 4 leaf-columns (2^2) -> Wide footprint
   * Level 1:        spans 2 leaf-columns (2^1) -> Medium footprint
   * Level 2 (Leaf): spans 1 leaf-column  (2^0) -> Minimum footprint (1 node width)
   *
   *
   *                [node]                 Level 0
   *               /      \
   *          [node]      [node]           Level 1
   *          /   \       /    \
   *      [L-L] [L-R]  [R-L]  [R-R]        Level 2
   *
   * - At Level 0, the node "owns" the entire width of 4 leaf nodes + 3 gaps.
   * - At Level 1, each node "owns" 2 leaf nodes + 1 gap.
   * - At Level 2, each node "owns" exactly 1 node.
   *
   *  Even if the tree has 100 levels, at level index 96, it still has 3 levels from the floor.
   *
   * This span calculation ensures spacing like above, so nodes and branches
   * don’t overlap horizontally.
   */
  [[nodiscard]] size_t
  getHorizontalSpan(size_t level, size_t nodeSize, size_t nodeGap, size_t treeHeight) const { // NOLINT
    if (level >= treeHeight) throw std::out_of_range("Level exceeds tree height");
    const size_t levelsToBottom = treeHeight - level - 1;

    const size_t columnSpan = getMaxNodesAtLevel(levelsToBottom);
    assert(columnSpan >= 1);

    const size_t gapCount = columnSpan - 1;

    return (columnSpan * nodeSize) + (gapCount * nodeGap);
  }

  template <detail::Streamable T>
  std::string formatValue(Node<T>* node, size_t padSize, const std::string& emptyNodeValue) {
    if (node == nullptr) return std::format("{:->{}}", emptyNodeValue, padSize);

    if constexpr (std::is_integral_v<T>) {
      return std::format("{:0{}d}", node->value, padSize);
    } else if constexpr (std::is_floating_point_v<T>) {
      return std::format("{:0{}f}", node->value, padSize);
    }
    return std::format("{:->{}}", detail::genericToString(node->value), padSize);
  }

public:
  /**
   * @param minNodeSize fallback min node size, not guaranteed to be used.
   * @param minNodeGap  fallback min node gap, not guaranteed to be used.
   */
  TreeVisualizer(size_t minNodeSize = 1, size_t minNodeGap = 1, bool displayEmptyNode = false) // NOLINT
      : m_minNodeSize(minNodeSize), m_minNodeGap(minNodeGap), m_displayEmptyNode(displayEmptyNode) {}

  /**
   * To print a tree beautifully, we need to calculate the following components:
   *
   *    (A)         00004
   *                 / \                      ---
   *                /   \                      |
   *               /     \                     |
   *              /       \                    |
   *             /         \                  (C)
   *            /           \                  |
   *           /             \                 |
   *          /               \                |
   *         /                 \              ---
   *      00002      (B)      00006
   *       / \                 / \
   *      /   \               /   \
   * (D) / (E) \     (F)     /     \
   *    /       \           /       \
   * 00001     00003     00005     00007
   *
   * (A): levelLeadingPadding
   *      - Spaces before the first node on a row, ensuring centering.
   *
   * (B): interNodeGap
   *      - Horizontal gap between consecutive nodes at the same level.
   *
   * (C): branchRows
   *      - Number of vertical rows of slashes (/ \) to draw between levels.
   *
   * (D): branchLeadingPadding
   *      - Spaces before the first slash on a branch row, aligning branches.
   *
   * (E): intraBranchGap
   *      - Gap inside a single parent’s branch pair, i.e. between '/' and '\'.
   *
   * (F): interBranchGap
   *      - Gap between the '\' of one parent and the '/' of the next parent.
   *
   */

  template <detail::Streamable T>
  void visualize(const BinaryTree<T>& tree, std::optional<bool> displayEmptyNode = std::nullopt) {
    Node<T>* root = tree.root();
    if (root == nullptr) {
      std::cout << "Tree is empty" << '\n';
      return;
    }

    bool shouldDisplayEmptyNode = displayEmptyNode.value_or(m_displayEmptyNode);
    size_t minNodeSize = findMinNodeSize(tree);
    std::string emptyNodeValue = shouldDisplayEmptyNode ? "NULL" : std::string(minNodeSize, ' ');
    if (shouldDisplayEmptyNode) minNodeSize = std::max(minNodeSize, emptyNodeValue.size());

    bool isMinNodeSizeEven = minNodeSize % 2 == 0;
    size_t minNodeGap = findMinNodeGap(isMinNodeSizeEven);
    size_t halfNodeSize = minNodeSize / 2;
    size_t halfNodeGap = minNodeGap / 2;

    size_t treeHeight = tree.height();
    queue::Deque<Node<T>*> q{root};
    size_t level = 0;
    // make the tree a perfect tree i.e. no empty node except for the last level
    // to do that, push nullptr (left, right) to the queue even though the current node is nullptr
    while (!q.empty()) {
      array::DynamicArray<bool> hasLeft;
      array::DynamicArray<bool> hasRight;

      size_t levelCapacity = q.size();
      bool isLastLevel = level == treeHeight - 1;
      size_t nodeSpan = getHorizontalSpan(level, minNodeSize, minNodeGap, treeHeight);

      size_t levelLeadingPadding = (nodeSpan / 2) - halfNodeSize;
      size_t interNodeGap = nodeSpan - (2 * (halfNodeSize - halfNodeGap));
      size_t branchRows = (nodeSpan + 1) / 4;

      std::cout << std::string(levelLeadingPadding, ' ');

      for (size_t nodeIdx = 0; nodeIdx < levelCapacity; ++nodeIdx) {
        bool isLevelLastNode = nodeIdx == levelCapacity - 1;
        Node<T>* node = q.front();
        q.popFront();

        std::cout << formatValue(node, minNodeSize, emptyNodeValue);

        if (!isLevelLastNode) std::cout << std::string(interNodeGap, ' ');
        q.pushBack(node == nullptr ? nullptr : node->left);
        q.pushBack(node == nullptr ? nullptr : node->right);

        hasLeft.pushBack(node != nullptr && node->left != nullptr);
        hasRight.pushBack(node != nullptr && node->right != nullptr);
      }

      std::cout << '\n';

      if (isLastLevel) break;

      for (size_t r = 0; r < branchRows; ++r) {
        const size_t branchLeadingPadding = (nodeSpan / 2) - 1 - r;
        std::cout << std::string(branchLeadingPadding, ' ');

        for (size_t nodeIdx = 0; nodeIdx < levelCapacity; ++nodeIdx) {
          bool isLevelLastNode = nodeIdx == levelCapacity - 1;
          size_t intraBranchWidth = 2 * r;
          if (!isMinNodeSizeEven) intraBranchWidth++;

          size_t interBranchGap = interNodeGap + (2 * (halfNodeSize - 1 - r));

          std::cout << (hasLeft[nodeIdx] || shouldDisplayEmptyNode ? '/' : ' ');
          std::cout << std::string(intraBranchWidth, ' ');
          std::cout << (hasRight[nodeIdx] || shouldDisplayEmptyNode ? '\\' : ' ');

          if (!isLevelLastNode) std::cout << std::string(interBranchGap, ' ');
        }
        std::cout << '\n';
      }
      level++;
    }
    std::cout << '\n';
  }
};

} // namespace tree
