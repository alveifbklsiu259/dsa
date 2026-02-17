#pragma once
#include "../hash_map/hash_map.hpp"
#include "../linked_list/doubly_linked_list.hpp"
#include "../queue/deque.hpp"
#include "../stack/dynamic_stack.hpp"
#include "./node.hpp"
#include <gsl/gsl>
#include <optional>

namespace tree {

namespace detail {

template <typename T, typename Seq>
concept BidirectionalSequence =
    std::ranges::bidirectional_range<Seq> && std::same_as<std::ranges::range_value_t<Seq>, T>;

template <typename T, typename Seq>
concept RandomAccessOptionalSequence =
    std::ranges::random_access_range<Seq> && std::same_as<std::ranges::range_value_t<Seq>, std::optional<T>>;

template <typename T, typename H>
concept Hasher =
    std::invocable<H, const T&> && std::convertible_to<std::invoke_result_t<H, const T&>, size_t>;

template <typename T, typename K>
concept KeyEqual = std::predicate<K, const T&, const T&>;

template <typename T, typename Iter>
  requires std::input_iterator<Iter> && std::same_as<std::remove_cvref_t<std::iter_value_t<Iter>>, T>
struct Frame {
  Iter inBegin;
  Iter inEnd;
  Iter seqBegin; // we use the name "seq" here in order to make it unambiguous, it can be either preorder,
                 // postorder or levelorder
  Iter seqEnd;
  Node<T>* parent;
  bool attachLeft;
};

} // namespace detail

// when we extend BinaryTree from BinarySearchTree, we need to have some check or rewrite tree-constructing
// methods like fromInPre. or maybe use that hook design pattern to insert check in the beginning

// maybe consider an interface that all trees implement

// https://github.com/yousinix/bst-ascii-visualization

// delete in BST https://leetcode.com/problems/delete-node-in-a-bst/description/

// maybe we can have different begin/end for inorder/preorder... by using the traversal methods we already
// have

// also check priority queue, should we make left,right, parent constexpr?
// copy/move constructors for hashmap
// should rewrite static array, it uses array underneath, also Deque uses std::array, should switch to
// array::StaticArray

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
class BinaryTree {

private:
  static constexpr auto defaultShouldBreak = [](Node<T>&) { return false; };
  using DefaultBreakPredicate = decltype(defaultShouldBreak);

  gsl::owner<Node<T>*> m_root = nullptr;
  Hasher m_hasher;
  KeyEqual m_keyEqual;

  template <std::invocable<Node<T>&> Callback, std::predicate<Node<T>&> ShouldBreak = DefaultBreakPredicate>
  bool preorderRecursive(Node<T>* node, Callback&& cb, ShouldBreak shouldBreak = defaultShouldBreak) const {
    if (node == nullptr) return true;

    cb(*node);
    if (shouldBreak(*node)) return false;

    if (!preorderRecursive(node->left, std::forward<Callback>(cb), shouldBreak)) { return false; }

    if (!preorderRecursive(node->right, std::forward<Callback>(cb), shouldBreak)) { return false; }

    return true;
  }

  template <std::invocable<Node<T>&> Callback, std::predicate<Node<T>&> ShouldBreak = DefaultBreakPredicate>
  bool preorderIterative(
      Node<T>* node,
      Callback&& cb, // NOLINT allow move-only callable
      ShouldBreak shouldBreak = defaultShouldBreak

  ) const {
    if (node == nullptr) return true;

    queue::Deque<Node<T>*> stack{node};
    while (!stack.empty()) {
      Node<T>* cur = stack.back();
      stack.popBack();

      cb(*cur);
      if (shouldBreak(*cur)) return false;

      if (cur->right) stack.pushBack(cur->right);
      if (cur->left) stack.pushBack(cur->left);
    }
    return true;
  }

  template <std::invocable<Node<T>&> Callback, std::predicate<Node<T>&> ShouldBreak = DefaultBreakPredicate>
  bool preorderMorris(
      Node<T>* node,
      Callback&& cb, // NOLINT allow move-only callable
      ShouldBreak shouldBreak = defaultShouldBreak
  ) const {

    Node<T>* cur = node;
    while (cur) {
      if (!cur->left) {
        cb(*cur);
        if (shouldBreak(*cur)) return false;
        cur = cur->right;
      } else {
        Node<T>* predecessor = cur->left;
        while (predecessor->right && predecessor->right != cur) predecessor = predecessor->right;

        if (!predecessor->right) {
          cb(*cur);
          if (shouldBreak(*cur)) return false;
          predecessor->right = cur;
          cur = cur->left;
        } else {
          predecessor->right = nullptr;
          cur = cur->right;
        }
      }
    }
    return true;
  }

  template <std::invocable<Node<T>&> Callback, std::predicate<Node<T>&> ShouldBreak = DefaultBreakPredicate>
  bool inorderRecursive(Node<T>* node, Callback&& cb, ShouldBreak shouldBreak = defaultShouldBreak) const {
    if (node == nullptr) return true;

    if (!inorderRecursive(node->left, std::forward<Callback>(cb), shouldBreak)) return false;

    cb(*node);
    if (shouldBreak(*node)) return false;

    if (!inorderRecursive(node->right, std::forward<Callback>(cb), shouldBreak)) return false;

    return true;
  }

  template <std::invocable<Node<T>&> Callback, std::predicate<Node<T>&> ShouldBreak = DefaultBreakPredicate>
  bool inorderIterative(
      Node<T>* node,
      Callback&& cb, // NOLINT allow move-only callable
      ShouldBreak shouldBreak = defaultShouldBreak

  ) const {
    queue::Deque<Node<T>*> stack;
    Node<T>* cur = node;

    while (cur || !stack.empty()) {
      while (cur) {
        stack.pushBack(cur);
        cur = cur->left;
      }

      cur = stack.back();
      stack.popBack();

      cb(*cur);
      if (shouldBreak(*cur)) return false;

      cur = cur->right;
    }
    return true;
  }

  template <std::invocable<Node<T>&> Callback, std::predicate<Node<T>&> ShouldBreak = DefaultBreakPredicate>
  bool inorderMorris(
      Node<T>* node,
      Callback&& cb, // NOLINT allow move-only callable
      ShouldBreak shouldBreak = defaultShouldBreak
  ) const {
    Node<T>* cur = node;

    while (cur) {
      if (!cur->left) {
        cb(*cur);
        if (shouldBreak(*cur)) return false;
        cur = cur->right;
      } else {
        Node<T>* predecessor = cur->left;
        while (predecessor->right && predecessor->right != cur) predecessor = predecessor->right;

        if (!predecessor->right) {
          predecessor->right = cur;
          cur = cur->left;
        } else {
          predecessor->right = nullptr;
          cb(*cur);
          if (shouldBreak(*cur)) return false;
          cur = cur->right;
        }
      }
    }
    return true;
  }

  template <std::invocable<Node<T>&> Callback, std::predicate<Node<T>&> ShouldBreak = DefaultBreakPredicate>
  bool
  inorderReverseRecursive(Node<T>* node, Callback&& cb, ShouldBreak shouldBreak = defaultShouldBreak) const {
    if (node == nullptr) return true;

    if (!inorderReverseRecursive(node->right, std::forward<Callback>(cb), shouldBreak)) return false;

    cb(*node);
    if (shouldBreak(*node)) return false;

    if (!inorderReverseRecursive(node->left, std::forward<Callback>(cb), shouldBreak)) return false;

    return true;
  }

  template <std::invocable<Node<T>&> Callback, std::predicate<Node<T>&> ShouldBreak = DefaultBreakPredicate>
  bool inorderReverseIterative(
      Node<T>* node,
      Callback&& cb, // NOLINT allow move-only callable
      ShouldBreak shouldBreak = defaultShouldBreak
  ) const {
    if (node == nullptr) return true;

    Node<T>* cur = node;
    queue::Deque<Node<T>*> stack;

    while (cur || !stack.empty()) {
      while (cur) {
        stack.pushBack(cur);
        cur = cur->right;
      }

      cur = stack.back();
      stack.popBack();

      cb(*cur);
      if (shouldBreak(*cur)) return false;

      cur = cur->left;
    }
    return true;
  }

  template <std::invocable<Node<T>&> Callback, std::predicate<Node<T>&> ShouldBreak = DefaultBreakPredicate>
  bool inorderReverseMorris(
      Node<T>* node,
      Callback&& cb, // NOLINT allow move-only callable
      ShouldBreak shouldBreak = defaultShouldBreak
  ) const {
    Node<T>* cur = node;

    while (cur != nullptr) {
      if (!cur->right) {
        cb(*cur);
        if (shouldBreak(*cur)) return false;
        cur = cur->left;
      } else {
        Node<T>* predecessor = cur->right;
        while (predecessor->left && predecessor->left != cur) predecessor = predecessor->left;

        if (!predecessor->left) {
          predecessor->left = cur;
          cur = cur->right;
        } else {
          predecessor->left = nullptr;
          cb(*cur);
          if (shouldBreak(*cur)) return false;
          cur = cur->left;
        }
      }
    }
    return true;
  }

  template <std::invocable<Node<T>&> Callback, std::predicate<Node<T>&> ShouldBreak = DefaultBreakPredicate>
  bool postorderRecursive(Node<T>* node, Callback&& cb, ShouldBreak shouldBreak = defaultShouldBreak) const {
    if (node == nullptr) return true;

    if (!postorderRecursive(node->left, std::forward<Callback>(cb), shouldBreak)) return false;

    if (!postorderRecursive(node->right, std::forward<Callback>(cb), shouldBreak)) return false;

    cb(*node);

    return !shouldBreak(*node);
  }

  template <std::invocable<Node<T>&> Callback, std::predicate<Node<T>&> ShouldBreak = DefaultBreakPredicate>
  bool postorderIterative(
      Node<T>* node,
      Callback&& cb, // NOLINT allow move-only callable
      ShouldBreak shouldBreak = defaultShouldBreak
  ) const {
    if (node == nullptr) return true;

    queue::Deque<Node<T>*> s1{node};
    queue::Deque<Node<T>*> s2;

    while (!s1.empty()) {
      Node<T>* cur = s1.back();
      s1.popBack();
      s2.pushBack(cur);

      if (cur->left) s1.pushBack(cur->left);
      if (cur->right) s1.pushBack(cur->right);
    }

    while (!s2.empty()) {
      Node<T>* cur = s2.back();
      s2.popBack();

      cb(*cur);
      if (shouldBreak(*cur)) return false;
    }

    return true;
  }

  // may be a bit less efficient in time complexity (compare to iterative/recursive approaches)
  template <std::invocable<Node<T>&> Callback, std::predicate<Node<T>&> ShouldBreak = DefaultBreakPredicate>
  bool postorderMorris( // NOLINT
      Node<T>* node,
      Callback&& cb, // NOLINT allow move-only callable
      ShouldBreak shouldBreak = defaultShouldBreak
  ) const {
    if (node == nullptr) return true;

    auto reversePath = [](Node<T>* from, Node<T>* to) {
      Node<T>* prev = nullptr;
      Node<T>* cur = from;
      Node<T>* next = nullptr;
      while (cur != to) {
        next = cur->right;
        cur->right = prev;
        prev = cur;
        cur = next;
      }
      cur->right = prev;
    };

    auto visitReverse = [&](Node<T>* from, Node<T>* to) {
      reversePath(from, to);
      Node<T>* cur = to;
      while (true) {
        cb(*cur);
        if (shouldBreak(*cur)) return false;
        if (cur == from) break;
        cur = cur->right;
      }
      reversePath(to, from);
      return true;
    };

    Node<T> dummy(0);
    dummy.left = node;
    Node<T>* cur = &dummy;

    while (cur) {
      if (!cur->left) {
        cur = cur->right;
      } else {
        Node<T>* predecessor = cur->left;
        while (predecessor->right && predecessor->right != cur) predecessor = predecessor->right;

        if (!predecessor->right) {
          predecessor->right = cur;
          cur = cur->left;
        } else {
          predecessor->right = nullptr;
          if (!visitReverse(cur->left, predecessor)) return false;
          cur = cur->right;
        }
      }
    }
    return true;
  }

  template <std::invocable<Node<T>&> Callback, std::predicate<Node<T>&> ShouldBreak = DefaultBreakPredicate>
  bool levelorderIterative(
      Node<T>* node,
      Callback&& cb, // NOLINT allow move-only callable
      ShouldBreak shouldBreak = defaultShouldBreak
  ) const {
    if (node == nullptr) return true;

    queue::Deque<Node<T>*> q{node};
    while (!q.empty()) {
      Node<T>* cur = q.front();
      q.popFront();

      cb(*cur);
      if (shouldBreak(*cur)) return false;

      if (cur->left) q.pushBack(cur->left);
      if (cur->right) q.pushBack(cur->right);
    }
    return true;
  }

  template <typename Seq>
    requires detail::BidirectionalSequence<T, Seq>
  void validateSequences(const Seq& s1, const Seq& s2) const {
    if (std::size(s1) != std::size(s2)) throw std::invalid_argument("Two sequences are inconsistent");

    hashmap::HashMap<T, size_t, Hasher, KeyEqual> m1{2, m_hasher, m_keyEqual};
    for (const T& val : s1) m1[val]++;

    hashmap::HashMap<T, size_t, Hasher, KeyEqual> m2{2, m_hasher, m_keyEqual};
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

    hashmap::HashMap<T, queue::Deque<ConstIter>, Hasher, KeyEqual> inorderPos{2, m_hasher, m_keyEqual};

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
      if (!f.parent)
        root = node;
      else if (f.attachLeft)
        f.parent->left = node;
      else
        f.parent->right = node;

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
    if (m_root) clear();
    m_root = root;
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

    hashmap::HashMap<T, queue::Deque<ConstIter>, Hasher, KeyEqual> inorderPos{2, m_hasher, m_keyEqual};
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

      if (!f.parent)
        root = node;
      else if (f.attachLeft)
        f.parent->left = node;
      else
        f.parent->right = node;

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
    if (m_root) clear();
    m_root = root;
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

    hashmap::HashMap<T, queue::Deque<ConstIter>, Hasher, KeyEqual> inorderPos(2, m_hasher, m_keyEqual);
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

      if (!f.parent)
        root = node;
      else if (f.attachLeft)
        f.parent->left = node;
      else
        f.parent->right = node;

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
    if (m_root) clear();
    m_root = root;
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

      node->left = self(self, (idx * 2) + 1);
      node->right = self(self, (idx * 2) + 2);
      return node;
    };

    m_root = static_cast<gsl::owner<Node<T>*>>(buildTree(buildTree, 0));
  }

  template <typename Seq> void fromArrayRepresentationIterative(Seq&& seq) { // NOLINT
    size_t n = std::ranges::size(seq);

    constexpr bool isSeqLRef = std::is_lvalue_reference_v<Seq>;

    if constexpr (isSeqLRef) {
      m_root = new Node<T>(seq[0].value());
    } else {
      m_root = new Node<T>(std::move(seq[0].value()));
    }

    queue::Deque<std::pair<Node<T>*, size_t>> queue{{m_root, 0}};

    while (!queue.empty()) {
      auto [node, idx] = queue.front();
      queue.popFront();
      size_t leftIdx = (idx * 2) + 1;
      size_t rightIdx = (idx * 2) + 2;

      if (leftIdx < n && seq[leftIdx].has_value()) {
        if constexpr (isSeqLRef) {
          node->left = new Node<T>(seq[leftIdx].value()); // NOLINT
        } else {
          node->left = new Node<T>(std::move(seq[leftIdx].value())); // NOLINT
        }
        queue.pushBack({node->left, leftIdx});
      }

      if (rightIdx < n && seq[rightIdx].has_value()) {
        if constexpr (isSeqLRef) {
          node->right = new Node<T>(seq[rightIdx].value()); // NOLINT
        } else {
          node->right = new Node<T>(std::move(seq[rightIdx].value())); // NOLINT
        }
        queue.pushBack({node->right, rightIdx});
      }
    }
  }

  virtual std::pair<bool, Node<T>*> eraseNode(Node<T>* target, Node<T>* parent) { // NOLINT
    if (target == nullptr) return {false, parent};
    Node<T>* newTree = parent;

    if (!target->left && !target->right) {
      if (parent && parent->left == target)
        parent->left = nullptr;
      else if (parent && parent->right == target)
        parent->right = nullptr;
      delete target; // NOLINT

    } else if (!target->left) { // only right child
      if (parent == nullptr) {
        newTree = target->right;
      } else {
        if (parent->left == target)
          parent->left = target->right;
        else
          parent->right = target->right;
      }
      delete target;             // NOLINT
    } else if (!target->right) { // only left child
      if (parent == nullptr) {
        newTree = target->left;
      } else {
        if (parent->left == target)
          parent->left = target->left;
        else
          parent->right = target->left;
      }
      delete target; // NOLINT
    } else {         // has both right and left
      Node<T>* right = target->right;
      Node<T>* cur = target->left;
      while (cur->right) cur = cur->right;
      cur->right = right;

      if (parent == nullptr) {
        newTree = target->left;
      } else {
        if (parent->left == target)
          parent->left = target->left;
        else
          parent->right = target->left;
      }
      delete target; // NOLINT
    }
    return {true, newTree};
  }

  void deepCopyTree(const BinaryTree& other) {
    if (other.m_root == nullptr) return;

    m_root = new Node(other.m_root->value);
    queue::Deque<std::pair<Node<T>*, Node<T>*>> q{{m_root, other.m_root}};

    while (!q.empty()) {
      auto [cur, otherCur] = q.front();
      q.popFront();
      if (otherCur->left != nullptr) {
        cur->left = new Node(otherCur->left->value); // NOLINT
        q.pushBack({cur->left, otherCur->left});
      }
      if (otherCur->right != nullptr) {
        cur->right = new Node(otherCur->right->value); // NOLINT
        q.pushBack({cur->right, otherCur->right});
      }
    }
  }

public:
  BinaryTree(Hasher hasher = {}, KeyEqual eq = {}) : m_hasher(hasher), m_keyEqual(eq) {}

  BinaryTree(const BinaryTree& other) : m_hasher(other.m_hasher), m_keyEqual(other.m_keyEqual) {
    deepCopyTree(other);
  }

  BinaryTree& operator=(const BinaryTree& other) {
    if (&other == this) return *this;
    clear();
    deepCopyTree(other);
    return *this;
  }

  BinaryTree(BinaryTree&& other) noexcept
      : m_hasher(std::move(other.m_hasher)), m_keyEqual(std::move(other.m_keyEqual)), m_root(other.m_root) {
    other.m_root = static_cast<gsl::owner<Node<T>*>>(nullptr);
  }

  BinaryTree& operator=(BinaryTree&& other) noexcept {
    if (&other == this) return *this;
    clear();
    m_hasher = std::move(other.m_hasher);
    m_keyEqual = std::move(other.m_keyEqual);
    m_root = other.m_root;
    other.m_root = static_cast<gsl::owner<Node<T>*>>(nullptr);
    return *this;
  }

  virtual ~BinaryTree() noexcept { clear(); }

  template <std::invocable<Node<T>&> Callback, std::predicate<Node<T>&> ShouldBreak = DefaultBreakPredicate>
  bool inorderTraverse(Callback&& cb, ShouldBreak shouldBreak = defaultShouldBreak) const {
    return inorderMorris(m_root, std::forward<Callback>(cb), shouldBreak);
  }

  /**
   * @brief Traverse the tree in **right -> node -> left** order,
   *        In BST, inorder is used to traverse the tree in a non-decreasing order,
   *        inorder-reverse can be used to traverse the tree in a non-increasing order.
   */
  template <std::invocable<Node<T>&> Callback, std::predicate<Node<T>&> ShouldBreak = DefaultBreakPredicate>
  bool inorderReverseTraverse(Callback&& cb, ShouldBreak shouldBreak = defaultShouldBreak) const {
    return inorderReverseMorris(m_root, std::forward<Callback>(cb), shouldBreak);
  }

  template <std::invocable<Node<T>&> Callback, std::predicate<Node<T>&> ShouldBreak = DefaultBreakPredicate>
  bool preorderTraverse(Callback&& cb, ShouldBreak shouldBreak = defaultShouldBreak) const {
    return preorderMorris(m_root, std::forward<Callback>(cb), shouldBreak);
  }

  template <std::invocable<Node<T>&> Callback, std::predicate<Node<T>&> ShouldBreak = DefaultBreakPredicate>
  bool postorderTraverse(Callback&& cb, ShouldBreak shouldBreak = defaultShouldBreak) const {
    return postorderIterative(m_root, std::forward<Callback>(cb), shouldBreak);
  }

  template <std::invocable<Node<T>&> Callback, std::predicate<Node<T>&> ShouldBreak = DefaultBreakPredicate>
  bool levelorderTraverse(Callback&& cb, ShouldBreak shouldBreak = defaultShouldBreak) const {
    return levelorderIterative(m_root, std::forward<Callback>(cb), shouldBreak);
  }

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
    if (m_root) clear();
    size_t n = std::ranges::size(seq);
    if (n == 0 || !seq[0].has_value()) {
      m_root = static_cast<gsl::owner<Node<T>*>>(nullptr);
      return;
    }
    fromArrayRepresentationIterative(std::forward<Seq>(seq));
  }

  void fromArrayRepresentation(std::initializer_list<std::optional<T>> l) {
    fromArrayRepresentation(array::DynamicArray<std::optional<T>>(l));
  }

  // insert by level-order, filling the tree left to right
  virtual Node<T>* insert(const T& val) noexcept {
    gsl::owner<Node<T>*> newNode = new Node<T>(val);

    if (m_root == nullptr) {
      m_root = newNode;
      return newNode; // NOLINT
    }
    bool attached = false;

    auto insertNode = [&](Node<T>& node) {
      if (node.left == nullptr) {
        node.left = newNode;
        attached = true;
        return;
      }
      if (node.right == nullptr) {
        node.right = newNode;
        attached = true;
        return;
      }
    };
    auto shouldBreak = [&](Node<T>&) { return attached; };
    levelorderTraverse(insertNode, shouldBreak);
    return newNode;
  }

  virtual Node<T>* find(const T& val) const noexcept {
    Node<T>* found = nullptr;
    auto findNode = [&](Node<T>& node) {
      std::cout << node.value << '\n';
      if (m_keyEqual(node.value, val)) found = &node;
    };
    auto shouldBreak = [&](Node<T>&) { return found != nullptr; };

    levelorderTraverse(findNode, shouldBreak);
    return found;
  }

  virtual bool eraseFirst(const T& val) {
    if (m_root == nullptr) return false;

    Node<T>* target = nullptr;
    Node<T>* parent = nullptr;

    queue::Deque<std::pair<Node<T>*, Node<T>*>> q;
    q.pushBack({m_root, parent});

    while (!q.empty()) {
      auto [cur, par] = q.front();
      q.popFront();

      if (m_keyEqual(cur->value, val)) {
        target = cur;
        parent = par;
        break;
      }

      if (cur->left) q.pushBack({cur->left, cur});
      if (cur->right) q.pushBack({cur->right, cur});
    }

    auto [erased, newTree] = eraseNode(target, parent);
    if (erased && parent == nullptr) m_root = static_cast<gsl::owner<Node<T>*>>(newTree);
    return erased;
  }

  virtual size_t eraseAll(const T& val) {
    if (m_root == nullptr) return 0;

    size_t count = 0;
    queue::Deque<std::pair<Node<T>*, Node<T>*>> q{{m_root, nullptr}};
    array::DynamicArray<std::pair<Node<T>*, Node<T>*>> matches;
    while (!q.empty()) {
      auto [cur, parent] = q.front();
      q.popFront();
      if (m_keyEqual(cur->value, val)) matches.pushBack({cur, parent});
      if (cur->left) q.pushBack({cur->left, cur});
      if (cur->right) q.pushBack({cur->right, cur});
    }

    // process child nodes first to avoid dangling parent
    for (auto it = matches.rbegin(); it != matches.rend(); ++it) {
      auto [cur, parent] = *it;
      auto [erased, newTree] = eraseNode(cur, parent);
      if (erased) count++;
      if (erased && parent == nullptr) m_root = static_cast<gsl::owner<Node<T>*>>(newTree);
    }
    return count;
  }

  void clear() noexcept {
    if (m_root == nullptr) return;
    auto deleteNode = [](Node<T>& node) {
      delete &node; // NOLINT
    };
    postorderTraverse(deleteNode);
    m_root = static_cast<gsl::owner<Node<T>*>>(nullptr);
  }

  [[nodiscard]] bool heightBalanced() const noexcept {
    if (m_root == nullptr) return true;
    hashmap::HashMap<Node<T>*, int> height;

    bool balanced = true;

    auto isBalanced = [&](Node<T>& node) {
      int leftHeight = height[node.left];
      int rightHeight = height[node.right];
      if (std::abs(leftHeight - rightHeight) > 1) balanced = false;
      height[&node] = 1 + std::max(leftHeight, rightHeight);
    };

    auto shouldBreak = [&](Node<T>&) { return !balanced; };

    postorderTraverse(isBalanced, shouldBreak);
    return balanced;
  }

  [[nodiscard]] bool complete() const noexcept {
    if (m_root == nullptr) return true;
    queue::Deque<Node<T>*> queue;
    queue.pushBack(m_root);
    bool seenNull = false;

    while (!queue.empty()) {
      Node<T>* node = queue.front();
      queue.popFront();

      if (node != nullptr) {
        if (seenNull) return false;
        queue.pushBack(node->left);
        queue.pushBack(node->right);
      } else {
        seenNull = true;
      }
    }
    return true;
  }

  [[nodiscard]] size_t height() const noexcept {
    if (m_root == nullptr) return 0;
    size_t height = 0;
    queue::Deque<Node<T>*> q{m_root};

    while (!q.empty()) {
      size_t levelSize = q.size();

      for (size_t i = 0; i < levelSize; ++i) {
        Node<T>* node = q.front();
        q.popFront();
        if (node->left != nullptr) q.pushBack(node->left);
        if (node->right != nullptr) q.pushBack(node->right);
      }
      ++height;
    }
    return height;
  }

  [[nodiscard]] bool full() const noexcept {
    if (m_root == nullptr) return true;
    bool isFull = true;

    auto checkIsFull = [&](Node<T>& node) {
      bool hasLeft = node.left != nullptr;
      bool hasRight = node.right != nullptr;
      if (hasLeft ^ hasRight) isFull = false;
    };

    auto shouldBreak = [&](Node<T>&) { return !isFull; };
    levelorderTraverse(checkIsFull, shouldBreak);
    return isFull;
  }

  [[nodiscard]] constexpr bool empty() const noexcept { return m_root == nullptr; }

  [[nodiscard]] constexpr Node<T>* root() const noexcept { return m_root; } // NOLINT
};
} // namespace tree
