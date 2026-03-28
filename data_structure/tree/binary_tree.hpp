#pragma once
#include "../hash_map/hash_map.hpp"
#include "../linked_list/doubly_linked_list.hpp"
#include "../queue/deque.hpp"
#include "../stack/dynamic_stack.hpp"
#include "./binary_tree_base.hpp"
#include "./detail.hpp"
#include "./node.hpp"
#include <gsl/gsl>
#include <optional>

namespace tree {

/**
 * @brief The tree supports duplicate values. However, when using
 *        `fromInPre`, `fromInPost`, or `fromInLevel` with duplicate values,
 *        the traversal sequences do not uniquely determine the tree
 *        structure. Multiple valid trees may exist, and this
 *        implementation will construct one deterministically, which
 *        may differ from the shape the user expects.
 *
 *        For example, given (value, inorderIdx):
 *          Inorder:   (9,0), (9,1), (15,2), (20,3), (7,4)
 *          Postorder: (9,0), (15,2), (7,4), (20,3), (9,1)
 *
 *        One valid tree is:
 *              9
 *               \
 *               20
 *              /  \
 *            15    7
 *           /
 *          9
 *
 *        But with:
 *          Inorder:   (9,0), (9,1), (15,2), (20,3), (7,4)
 *          Postorder: (9,1), (15,2), (7,4), (20,3), (9,0)
 *
 *        Another valid tree is:
 *              9
 *             / \
 *            9   20
 *               /  \
 *              15   7
 *
 *        Both satisfy the same inorder and postorder sequences.
 *
 *        If the tree does not contain duplicate values, you can safely
 *        build it with `fromInPre`, `fromInPost`, or `fromInLevel`.
 *
 *        If you need to guarantee a specific tree shape when duplicates
 *        are present, use `fromCompleteTree`, since the complete tree
 *        representation encodes both structure and values explicitly.
 */
template <typename T, typename Hasher = std::hash<T>, typename KeyEqual = std::equal_to<T>>
  requires detail::Hasher<T, Hasher> && detail::KeyEqual<T, KeyEqual>
class BinaryTree : public detail::BinaryTreeBase<T, Hasher, KeyEqual> {

private:
  template <typename Seq>
    requires detail::BidirectionalSequence<T, Seq>
  void validateSequences(const Seq& s1, const Seq& s2) const {
    if (std::size(s1) != std::size(s2)) throw std::invalid_argument("Two sequences are inconsistent");

    hashmap::HashMap<T, size_t, Hasher, KeyEqual> m1{2, this->getHasher(), this->getKeyEqual()};
    for (const T& val : s1) m1[val]++;

    hashmap::HashMap<T, size_t, Hasher, KeyEqual> m2{2, this->getHasher(), this->getKeyEqual()};
    for (const T& val : s2) m2[val]++;

    if (m1 != m2) throw std::invalid_argument("Two sequences are inconsistent");
  }

  template <typename Seq>
    requires detail::BidirectionalSequence<T, Seq>
  void fromInPreIterative(const Seq& inorder, const Seq& preorder) {
    validateSequences(inorder, preorder);
    // using Iter = decltype(std::begin(std::declval<const Seq&>()));
    using ConstIter = std::ranges::iterator_t<const Seq>;
    using Frame = detail::Frame<T, ConstIter>;

    ConstIter inBegin = std::begin(inorder);
    ConstIter inEnd = std::end(inorder);
    ConstIter preBegin = std::begin(preorder);
    ConstIter preEnd = std::end(preorder);

    if (inBegin == inEnd || preBegin == preEnd) return;

    hashmap::HashMap<T, queue::Deque<ConstIter>, Hasher, KeyEqual> inorderPos{
        2, this->getHasher(), this->getKeyEqual()
    };

    for (ConstIter it = inBegin; it != inEnd; ++it) inorderPos[*it].pushBack(it);

    stack::DynamicStack<Frame> st;
    gsl::owner<Node<T>*> root = static_cast<gsl::owner<Node<T>*>>(nullptr);
    st.push({inBegin, inEnd, preBegin, preEnd, root, false});

    while (!st.empty()) {
      Frame f = st.top();
      st.pop();

      if (f.inBegin == f.inEnd || f.seqBegin == f.seqEnd) continue;

      const T& rootVal = *f.seqBegin;
      gsl::owner<Node<T>*> node = new Node<T>(rootVal);
      if (!f.parent) root = node;
      else if (f.attachLeft) f.parent->setLeft(node);
      else f.parent->setRight(node);

      if (inorderPos[rootVal].empty()) {
        throw std::invalid_argument("Please make sure the inorder and preorder sequences are correct.");
      }
      ConstIter inRoot = inorderPos[rootVal].front();
      inorderPos[rootVal].popFront();

      int leftSize = std::distance(f.inBegin, inRoot);

      // right sub-tree
      st.push({std::next(inRoot), f.inEnd, std::next(f.seqBegin, leftSize + 1), f.seqEnd, node, false});
      // left sub-tree
      st.push({f.inBegin, inRoot, std::next(f.seqBegin), std::next(f.seqBegin, leftSize + 1), node, true});
    }
    if (!this->empty()) this->clear();
    this->setRoot(root);
  }

  template <typename Seq>
    requires detail::BidirectionalSequence<T, Seq>
  void fromInPostIterative(const Seq& inorder, const Seq& postorder) {
    validateSequences(inorder, postorder);
    using ConstIter = std::ranges::iterator_t<const Seq>;
    using Frame = detail::Frame<T, ConstIter>;

    ConstIter inBegin = std::begin(inorder);
    ConstIter inEnd = std::end(inorder);
    ConstIter postBegin = std::begin(postorder);
    ConstIter postEnd = std::end(postorder);

    if (inBegin == inEnd || postBegin == postEnd) return;

    hashmap::HashMap<T, queue::Deque<ConstIter>, Hasher, KeyEqual> inorderPos{
        2, this->getHasher(), this->getKeyEqual()
    };
    for (ConstIter it = inBegin; it != inEnd; ++it) inorderPos[*it].pushBack(it);

    gsl::owner<Node<T>*> root = static_cast<gsl::owner<Node<T>*>>(nullptr);
    stack::DynamicStack<Frame> st;

    st.push({inBegin, inEnd, postBegin, postEnd, root, false});

    while (!st.empty()) {
      Frame f = st.top();
      st.pop();

      if (f.inBegin == f.inEnd || f.seqBegin == f.seqEnd) continue;

      const T& rootVal = *std::prev(f.seqEnd);
      gsl::owner<Node<T>*> node = new Node<T>(rootVal);

      if (!f.parent) root = node;
      else if (f.attachLeft) f.parent->setLeft(node);
      else f.parent->setRight(node);

      if (inorderPos[rootVal].empty()) {
        throw std::invalid_argument("Please make sure the inorder and postorder sequences are correct.");
      }
      ConstIter inRoot = inorderPos[rootVal].front();
      inorderPos[rootVal].popFront();

      int leftSize = std::distance(f.inBegin, inRoot);

      // left sub-tree
      st.push({f.inBegin, inRoot, f.seqBegin, std::next(f.seqBegin, leftSize), node, true});

      // right sub-tree
      st.push(
          {std::next(inRoot), f.inEnd, std::next(f.seqBegin, leftSize), std::prev(f.seqEnd), node, false}
      );
    }
    if (!this->empty()) this->clear();
    this->setRoot(root);
  }

  template <typename Seq>
    requires detail::BidirectionalSequence<T, Seq>
  void fromInLevelIterative(const Seq& inorder, const Seq& levelorder) {
    validateSequences(inorder, levelorder);
    using ConstIter = std::ranges::iterator_t<const Seq>;
    using Frame = detail::Frame<T, ConstIter>;

    ConstIter inBegin = std::begin(inorder);
    ConstIter inEnd = std::end(inorder);
    ConstIter levelBegin = std::begin(levelorder);
    ConstIter levelEnd = std::end(levelorder);

    if (inBegin == inEnd || levelBegin == levelEnd) return;

    hashmap::HashMap<T, queue::Deque<ConstIter>, Hasher, KeyEqual> inorderPos(
        2, this->getHasher(), this->getKeyEqual()
    );
    for (ConstIter it = inBegin; it != inEnd; ++it) inorderPos[*it].pushBack(it);

    gsl::owner<Node<T>*> root = static_cast<gsl::owner<Node<T>*>>(nullptr);

    stack::DynamicStack<Frame> st;

    st.push({inBegin, inEnd, levelBegin, levelEnd, root, false});

    // use a linked list for sub-sequence storage; unlike DynamicArray, it avoids iterator invalidation from
    // reallocation.
    // if we simply constructor the sub-sequence container inside for-loop, we risk dangling iterator.
    linkedlist::DoublyLinkedList<array::DynamicArray<T>> subsequencesPool;

    while (!st.empty()) {
      Frame f = st.top();
      st.pop();
      if (f.inBegin == f.inEnd || f.seqBegin == f.seqEnd) continue;

      const T& rootVal = *f.seqBegin;
      gsl::owner<Node<T>*> node = new Node<T>(rootVal);

      if (!f.parent) root = node;
      else if (f.attachLeft) f.parent->setLeft(node);
      else f.parent->setRight(node);

      if (inorderPos[rootVal].empty()) {
        throw std::invalid_argument("Please make sure the inorder and levelorder sequences are correct.");
      }
      ConstIter inRoot = inorderPos[rootVal].front();
      inorderPos[rootVal].popFront();

      ConstIter leftInBegin = f.inBegin;
      ConstIter leftInEnd = inRoot;
      ConstIter rightInBegin = std::next(inRoot);
      ConstIter rightInEnd = f.inEnd;

      array::DynamicArray<T>& leftLevel = subsequencesPool.emplaceBack();
      array::DynamicArray<T>& rightLevel = subsequencesPool.emplaceBack();

      for (ConstIter it = std::next(f.seqBegin); it != f.seqEnd; ++it) {
        auto& candidate = inorderPos[*it].front();
        if (candidate >= leftInBegin && candidate < leftInEnd) {
          leftLevel.pushBack(*it);
        } else if (candidate >= rightInBegin && candidate < rightInEnd) {
          rightLevel.pushBack(*it);
        }
      }

      // left sub-tree
      st.push({leftInBegin, leftInEnd, leftLevel.begin(), leftLevel.end(), node, true});

      // right sub-tree
      st.push({rightInBegin, rightInEnd, rightLevel.begin(), rightLevel.end(), node, false});
    }
    if (!this->empty()) this->clear();
    this->setRoot(root);
  }

  template <typename Seq>
    requires detail::RandomAccessOptionalSequence<T, Seq>
  void fromArrayRepresentationRecursive(Seq&& seq) { // NOLINT
    const size_t n = std::ranges::size(seq);

    auto buildTree = [&](auto&& self, size_t idx) -> gsl::owner<Node<T>*> {
      if (idx >= n) return nullptr;
      if (!seq[idx].has_value()) return nullptr;

      gsl::owner<Node<T>*> node = static_cast<gsl::owner<Node<T>*>>(nullptr);

      if constexpr (std::is_lvalue_reference_v<Seq>) {
        node = new Node<T>(seq[idx].value());
      } else {
        node = new Node<T>(std::move(seq[idx].value()));
      }

      node->setLeft(self(self, (idx * 2) + 1));
      node->setRight(self(self, (idx * 2) + 2));
      return node;
    };
    this->setRoot(buildTree(buildTree, 0)); // NOLINT
  }

  template <typename Seq> void fromArrayRepresentationIterative(Seq&& seq) { // NOLINT
    size_t n = std::ranges::size(seq);

    constexpr bool isSeqLRef = std::is_lvalue_reference_v<Seq>;

    if constexpr (isSeqLRef) {
      this->setRoot(new Node<T>(seq[0].value()));
    } else {
      this->setRoot(new Node<T>(std::move(seq[0].value())));
    }

    queue::Deque<std::pair<Node<T>*, size_t>> queue{{this->root(), 0}};

    while (!queue.empty()) {
      auto [node, idx] = queue.front();
      queue.popFront();
      size_t leftIdx = (idx * 2) + 1;
      size_t rightIdx = (idx * 2) + 2;

      if (leftIdx < n && seq[leftIdx].has_value()) {
        if constexpr (isSeqLRef) {
          node->setLeft(new Node<T>(seq[leftIdx].value()));
        } else {
          node->setLeft(new Node<T>(std::move(seq[leftIdx].value())));
        }
        queue.pushBack({node->left(), leftIdx});
      }

      if (rightIdx < n && seq[rightIdx].has_value()) {
        if constexpr (isSeqLRef) {
          node->setRight(new Node<T>(seq[rightIdx].value()));
        } else {
          node->setRight(new Node<T>(std::move(seq[rightIdx].value())));
        }
        queue.pushBack({node->right(), rightIdx});
      }
    }
  }

public:
  // inherit all constructors
  using detail::BinaryTreeBase<T, Hasher, KeyEqual>::BinaryTreeBase;

  template <typename Seq>
    requires detail::BidirectionalSequence<T, Seq>
  void fromInPre(const Seq& inorder, const Seq& preorder) {
    fromInPreIterative(inorder, preorder);
  }

  void fromInPre(
      std::initializer_list<T> inorderL, // NOLINT
      std::initializer_list<T> preorderL
  ) {
    const array::DynamicArray<T> inorder(inorderL);
    const array::DynamicArray<T> preorder(preorderL);
    fromInPre(inorder, preorder);
  }

  template <typename Seq>
    requires detail::BidirectionalSequence<T, Seq>
  void fromInPost(const Seq& inorder, const Seq& postorder) {
    fromInPostIterative(inorder, postorder);
  }

  void fromInPost(
      std::initializer_list<T> inorderL, // NOLINT
      std::initializer_list<T> postorderL
  ) {
    const array::DynamicArray<T> inorder(inorderL);
    const array::DynamicArray<T> postorder(postorderL);
    fromInPost(inorder, postorder);
  }

  template <typename Seq>
    requires detail::BidirectionalSequence<T, Seq>
  void fromInLevel(const Seq& inorder, const Seq& levelorder) {
    fromInLevelIterative(inorder, levelorder);
  }

  void fromInLevel(
      std::initializer_list<T> inorderL, // NOLINT
      std::initializer_list<T> levelorderL
  ) {
    const array::DynamicArray<T> inorder(inorderL);
    const array::DynamicArray<T> levelorder(levelorderL);
    fromInLevel(inorder, levelorder);
  }

  /**
   * Build a binary tree from a level-order array representation
   * where each element is std::optional<T>. A nullopt entry means
   * "no node" at that position. This allows constructing arbitrary
   * binary trees.
   *
   * Example:
   * Input sequence:
   *   [1, null, 3, null, null, 4, null, null, null, null, null, 5, 6, null, null]
   *
   * Resulting tree:
   *       1
   *        \
   *         3
   *        /
   *       4
   *      / \
   *     5   6
   *
   */
  template <typename Seq>
    requires detail::RandomAccessOptionalSequence<T, Seq>
  void fromArrayRepresentation(Seq&& seq) {
    if (!this->empty()) this->clear();
    size_t n = std::ranges::size(seq);
    if (n == 0 || !seq[0].has_value()) {
      this->setRoot(nullptr);
      return;
    }
    fromArrayRepresentationIterative(std::forward<Seq>(seq));
  }

  void fromArrayRepresentation(std::initializer_list<std::optional<T>> l) {
    fromArrayRepresentation(array::DynamicArray<std::optional<T>>(l));
  }
};

} // namespace tree
