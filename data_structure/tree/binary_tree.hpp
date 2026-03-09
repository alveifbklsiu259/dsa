#pragma once
#include "../hash_map/hash_map.hpp"
#include "../linked_list/doubly_linked_list.hpp"
#include "../queue/deque.hpp"
#include "../stack/dynamic_stack.hpp"
#include "./detail.hpp"
#include "./node.hpp"
#include <gsl/gsl>
#include <optional>

namespace tree {

// TODO:
// when we extend BinaryTree from BinarySearchTree, we need to have some check or rewrite tree-constructing
// methods like fromInPre. or maybe use that hook design pattern to insert check in the beginning

// maybe consider an interface that all trees implement

// maybe we can have different begin/end for inorder/preorder... by using the traversal methods we already
// have

// also check priority queue, should we make left,right, parent constexpr?

// copy/move constructors, swap method for hashmap
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

protected:
  void setRoot(gsl::owner<Node<T>*> node) noexcept { m_root = node; }
  void setRoot(std::nullptr_t) noexcept { m_root = static_cast<gsl::owner<Node<T>*>>(nullptr); }

  bool keyEqual(const T& a, const T& b) const noexcept { return m_keyEqual(a, b); }

  bool eraseNode(Node<T>* target, Node<T>* parent) { // NOLINT
    if (target == nullptr || empty()) return false;
    bool hasBothChildren = target->left() != nullptr && target->right() != nullptr;

    if (hasBothChildren) {
      Node<T>* predecessorParent = target;
      Node<T>* predecessor = target->left();

      while (predecessor->right() != nullptr) {
        predecessorParent = predecessor;
        predecessor = predecessor->right();
      }

      target->setValue(predecessor->value());
      target = predecessor;
      parent = predecessorParent;
    }

    // target has at most one child now
    Node<T>* child = target->left() != nullptr ? target->left() : target->right();

    if (parent == nullptr) {
      assert(root() == nullptr);
      setRoot(child);
    } else if (parent->left() == target) {
      parent->setLeft(child);
    } else {
      parent->setRight(child);
    }

    delete target; // NOLINT;
    return true;
  }

private:
  // using NodeType for both Node<T> and const Node<T>
  template <typename NodeType> class DefaultBreak {
  public:
    constexpr bool operator()(NodeType& /*_node*/) const noexcept { return false; }
  };

  template <typename NodeType> class DefaultBreakWithLevel {
  public:
    constexpr bool operator()(NodeType& /*_node*/, size_t /*_level*/) const noexcept { return false; }
  };

  gsl::owner<Node<T>*> m_root = nullptr;
  Hasher m_hasher;
  KeyEqual m_keyEqual;

  template <
      typename NodeType, std::invocable<NodeType&> Callback,
      std::predicate<NodeType&> ShouldBreak = DefaultBreak<NodeType>>
  bool preorderRecursive(NodeType* node, Callback&& cb, ShouldBreak shouldBreak = {}) const {
    if (node == nullptr) return true;

    cb(*node);
    if (shouldBreak(*node)) return false;

    if (!preorderRecursive(node->left(), std::forward<Callback>(cb), shouldBreak)) { return false; }

    if (!preorderRecursive(node->right(), std::forward<Callback>(cb), shouldBreak)) { return false; }

    return true;
  }

  template <
      typename NodeType, std::invocable<NodeType&> Callback,
      std::predicate<NodeType&> ShouldBreak = DefaultBreak<NodeType>>
  bool preorderIterative(
      NodeType* node,
      Callback&& cb, // NOLINT allow move-only callable
      ShouldBreak shouldBreak = {}
  ) const {
    if (node == nullptr) return true;

    queue::Deque<NodeType*> stack{node};
    while (!stack.empty()) {
      NodeType* cur = stack.back();
      stack.popBack();

      cb(*cur);
      if (shouldBreak(*cur)) return false;

      if (cur->right()) stack.pushBack(cur->right());
      if (cur->left()) stack.pushBack(cur->left());
    }
    return true;
  }

  template <
      typename NodeType, std::invocable<NodeType&> Callback,
      std::predicate<NodeType&> ShouldBreak = DefaultBreak<NodeType>>
  bool preorderMorris(
      NodeType* node,
      Callback&& cb, // NOLINT allow move-only callable
      ShouldBreak shouldBreak = {}
  ) const {

    NodeType* cur = node;
    while (cur) {
      if (!cur->left()) {
        cb(*cur);
        if (shouldBreak(*cur)) return false;
        cur = cur->right();
      } else {
        NodeType* predecessor = cur->left();
        while (predecessor->right() && predecessor->right() != cur) predecessor = predecessor->right();

        if (!predecessor->right()) {
          cb(*cur);
          if (shouldBreak(*cur)) return false;
          predecessor->setRight(cur);
          cur = cur->left();
        } else {
          predecessor->setRight(nullptr);
          cur = cur->right();
        }
      }
    }
    return true;
  }

  template <
      typename NodeType, std::invocable<NodeType&> Callback,
      std::predicate<NodeType&> ShouldBreak = DefaultBreak<NodeType>>
  bool inorderRecursive(NodeType* node, Callback&& cb, ShouldBreak shouldBreak = {}) const {
    if (node == nullptr) return true;

    if (!inorderRecursive(node->left(), std::forward<Callback>(cb), shouldBreak)) return false;

    cb(*node);
    if (shouldBreak(*node)) return false;

    if (!inorderRecursive(node->right(), std::forward<Callback>(cb), shouldBreak)) return false;

    return true;
  }

  template <
      typename NodeType, std::invocable<NodeType&> Callback,
      std::predicate<NodeType&> ShouldBreak = DefaultBreak<NodeType>>
  bool inorderIterative(
      NodeType* node,
      Callback&& cb, // NOLINT allow move-only callable
      ShouldBreak shouldBreak = {}

  ) const {
    queue::Deque<NodeType*> stack;
    NodeType* cur = node;

    while (cur || !stack.empty()) {
      while (cur) {
        stack.pushBack(cur);
        cur = cur->left();
      }

      cur = stack.back();
      stack.popBack();

      cb(*cur);
      if (shouldBreak(*cur)) return false;

      cur = cur->right();
    }
    return true;
  }

  template <
      typename NodeType, std::invocable<NodeType&> Callback,
      std::predicate<NodeType&> ShouldBreak = DefaultBreak<NodeType>>
  bool inorderMorris(
      NodeType* node,
      Callback&& cb, // NOLINT allow move-only callable
      ShouldBreak shouldBreak = {}
  ) const {
    NodeType* cur = node;

    while (cur) {
      if (!cur->left()) {
        cb(*cur);
        if (shouldBreak(*cur)) return false;
        cur = cur->right();
      } else {
        NodeType* predecessor = cur->left();
        while (predecessor->right() && predecessor->right() != cur) predecessor = predecessor->right();

        if (!predecessor->right()) {
          predecessor->setRight(cur);
          cur = cur->left();
        } else {
          predecessor->setRight(nullptr);
          cb(*cur);
          if (shouldBreak(*cur)) return false;
          cur = cur->right();
        }
      }
    }
    return true;
  }

  template <
      typename NodeType, std::invocable<NodeType&> Callback,
      std::predicate<NodeType&> ShouldBreak = DefaultBreak<NodeType>>
  bool inorderReverseRecursive(NodeType* node, Callback&& cb, ShouldBreak shouldBreak = {}) const {
    if (node == nullptr) return true;

    if (!inorderReverseRecursive(node->right(), std::forward<Callback>(cb), shouldBreak)) return false;

    cb(*node);
    if (shouldBreak(*node)) return false;

    if (!inorderReverseRecursive(node->left(), std::forward<Callback>(cb), shouldBreak)) return false;

    return true;
  }

  template <
      typename NodeType, std::invocable<NodeType&> Callback,
      std::predicate<NodeType&> ShouldBreak = DefaultBreak<NodeType>>
  bool inorderReverseIterative(
      NodeType* node,
      Callback&& cb, // NOLINT allow move-only callable
      ShouldBreak shouldBreak = {}
  ) const {
    if (node == nullptr) return true;

    NodeType* cur = node;
    queue::Deque<NodeType*> stack;

    while (cur || !stack.empty()) {
      while (cur) {
        stack.pushBack(cur);
        cur = cur->right();
      }

      cur = stack.back();
      stack.popBack();

      cb(*cur);
      if (shouldBreak(*cur)) return false;

      cur = cur->left();
    }
    return true;
  }

  template <
      typename NodeType, std::invocable<NodeType&> Callback,
      std::predicate<NodeType&> ShouldBreak = DefaultBreak<NodeType>>
  bool inorderReverseMorris(
      NodeType* node,
      Callback&& cb, // NOLINT allow move-only callable
      ShouldBreak shouldBreak = {}
  ) const {
    NodeType* cur = node;

    while (cur != nullptr) {
      if (!cur->right()) {
        cb(*cur);
        if (shouldBreak(*cur)) return false;
        cur = cur->left();
      } else {
        NodeType* predecessor = cur->right();
        while (predecessor->left() && predecessor->left() != cur) predecessor = predecessor->left();

        if (!predecessor->left()) {
          predecessor->setLeft(cur);
          cur = cur->right();
        } else {
          predecessor->setLeft(nullptr);
          cb(*cur);
          if (shouldBreak(*cur)) return false;
          cur = cur->left();
        }
      }
    }
    return true;
  }

  template <
      typename NodeType, std::invocable<NodeType&> Callback,
      std::predicate<NodeType&> ShouldBreak = DefaultBreak<NodeType>>
  bool postorderRecursive(NodeType* node, Callback&& cb, ShouldBreak shouldBreak = {}) const {
    if (node == nullptr) return true;

    if (!postorderRecursive(node->left(), std::forward<Callback>(cb), shouldBreak)) return false;

    if (!postorderRecursive(node->right(), std::forward<Callback>(cb), shouldBreak)) return false;

    cb(*node);

    return !shouldBreak(*node);
  }

  template <
      typename NodeType, std::invocable<NodeType&> Callback,
      std::predicate<NodeType&> ShouldBreak = DefaultBreak<NodeType>>
  bool postorderIterative(
      NodeType* node,
      Callback&& cb, // NOLINT allow move-only callable
      ShouldBreak shouldBreak = {}
  ) const {
    if (node == nullptr) return true;

    queue::Deque<NodeType*> s1{node};
    queue::Deque<NodeType*> s2;

    while (!s1.empty()) {
      NodeType* cur = s1.back();
      s1.popBack();
      s2.pushBack(cur);

      if (cur->left()) s1.pushBack(cur->left());
      if (cur->right()) s1.pushBack(cur->right());
    }

    while (!s2.empty()) {
      NodeType* cur = s2.back();
      s2.popBack();

      cb(*cur);
      if (shouldBreak(*cur)) return false;
    }

    return true;
  }

  // may be a bit less efficient in time complexity (compare to iterative/recursive approaches)
  template <typename NodeType, std::invocable<NodeType&> Callback, std::predicate<NodeType&> ShouldBreak = DefaultBreak<NodeType>>
  bool postorderMorris( // NOLINT
      NodeType* node,
      Callback&& cb, // NOLINT allow move-only callable
      ShouldBreak shouldBreak = {}
  ) const {
    if (node == nullptr) return true;

    auto reversePath = [](NodeType* from, NodeType* to) {
      NodeType* prev = nullptr;
      NodeType* cur = from;
      NodeType* next = nullptr;
      while (cur != to) {
        next = cur->right();
        cur->setRight(prev);
        prev = cur;
        cur = next;
      }
      cur->setRight(prev);
    };

    auto visitReverse = [&](NodeType* from, NodeType* to) {
      reversePath(from, to);
      NodeType* cur = to;
      while (true) {
        cb(*cur);
        if (shouldBreak(*cur)) return false;
        if (cur == from) break;
        cur = cur->right();
      }
      reversePath(to, from);
      return true;
    };

    NodeType dummy(0);
    dummy.setLeft(node);
    NodeType* cur = &dummy;

    while (cur) {
      if (!cur->left()) {
        cur = cur->right();
      } else {
        NodeType* predecessor = cur->left();
        while (predecessor->right() && predecessor->right() != cur) predecessor = predecessor->right();

        if (!predecessor->right()) {
          predecessor->setRight(cur);
          cur = cur->left();
        } else {
          predecessor->setRight(nullptr);
          if (!visitReverse(cur->left(), predecessor)) return false;
          cur = cur->right();
        }
      }
    }
    return true;
  }

  template <
      typename NodeType, std::invocable<NodeType&, size_t /* level, starts from 0 */> Callback,
      std::predicate<NodeType&, size_t /* level, starts from 0 */> ShouldBreak = DefaultBreak<NodeType>>
  bool levelorderIterative(
      NodeType* node,
      Callback&& cb, // NOLINT allow move-only callable
      ShouldBreak shouldBreak = {}
  ) const {
    if (node == nullptr) return true;
    size_t level = 0;

    queue::Deque<NodeType*> q{node};
    while (!q.empty()) {
      size_t levelSize = q.size();
      for (size_t i = 0; i < levelSize; ++i) {
        NodeType* cur = q.front();
        q.popFront();

        cb(*cur, level);
        if (shouldBreak(*cur, level)) return false;

        if (cur->left()) q.pushBack(cur->left());
        if (cur->right()) q.pushBack(cur->right());
      }
      level++;
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
    if (!empty()) clear();
    setRoot(root);
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
    if (!empty()) clear();
    setRoot(root);
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
    if (!empty()) clear();
    setRoot(root);
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
    setRoot(buildTree(buildTree, 0)); // NOLINT
  }

  template <typename Seq> void fromArrayRepresentationIterative(Seq&& seq) { // NOLINT
    size_t n = std::ranges::size(seq);

    constexpr bool isSeqLRef = std::is_lvalue_reference_v<Seq>;

    if constexpr (isSeqLRef) {
      setRoot(new Node<T>(seq[0].value()));
    } else {
      setRoot(new Node<T>(std::move(seq[0].value())));
    }

    queue::Deque<std::pair<Node<T>*, size_t>> queue{{root(), 0}};

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

  // findFirstImpl is marked with const because it is internally used by the two overloads of findFirst,
  // Node<T>* findFirst(const T& val) noexcept {}
  // const Node<T>* findFirst(const T& val) const noexcept {}
  virtual const Node<T>* findFirstImpl(const T& val) const noexcept {
    const Node<T>* found = nullptr;
    auto findNode = [&](const Node<T>& node) {
      if (keyEqual(node.value(), val)) found = &node;
    };
    auto shouldBreak = [&](const Node<T>&) { return found != nullptr; };

    levelorderTraverse(findNode, shouldBreak);
    return found;
  }

  // insert by level-order, filling the tree left to right
  virtual Node<T>* insertImpl(const T& val) noexcept {
    gsl::owner<Node<T>*> newNode = new Node<T>(val);

    if (empty()) {
      setRoot(newNode);
      return newNode; // NOLINT
    }

    bool attached = false;
    auto insertNode = [&](Node<T>& node) {
      if (node.left() == nullptr) {
        node.setLeft(newNode);
        attached = true;
        return;
      }
      if (node.right() == nullptr) {
        node.setRight(newNode);
        attached = true;
        return;
      }
    };
    auto shouldBreak = [&](Node<T>&) { return attached; };
    levelorderTraverse(insertNode, shouldBreak);
    return newNode;
  }

  void deepCopyTree(const BinaryTree& other) {
    if (other.empty()) return;

    setRoot(new Node(other.root()->value()));
    queue::Deque<std::pair<Node<T>*, Node<T>*>> q{{root(), other.root()}};

    while (!q.empty()) {
      auto [cur, otherCur] = q.front();
      q.popFront();
      if (otherCur->left() != nullptr) {
        cur->setLeft(new Node(otherCur->left()->value()));
        q.pushBack({cur->left(), otherCur->left()});
      }
      if (otherCur->right() != nullptr) {
        cur->setRight(new Node(otherCur->right()->value()));
        q.pushBack({cur->right(), otherCur->right()});
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
      : m_hasher(std::move(other.m_hasher)), m_keyEqual(std::move(other.m_keyEqual)), m_root(other.root()) {
    other.setRoot(nullptr);
  }

  BinaryTree& operator=(BinaryTree&& other) noexcept {
    if (&other == this) return *this;
    clear();
    m_hasher = std::move(other.m_hasher);
    m_keyEqual = std::move(other.m_keyEqual);
    setRoot(other.root());
    other.setRoot(nullptr);
    return *this;
  }

  virtual ~BinaryTree() noexcept { clear(); }

  template <std::invocable<Node<T>&> Callback, std::predicate<Node<T>&> ShouldBreak = DefaultBreak<Node<T>>>
  bool inorderTraverse(Callback&& cb, ShouldBreak shouldBreak = {}) {
    return inorderMorris(root(), std::forward<Callback>(cb), shouldBreak);
  }

  template <
      std::invocable<const Node<T>&> Callback,
      std::predicate<const Node<T>&> ShouldBreak = DefaultBreak<const Node<T>>>
  bool inorderTraverse(Callback&& cb, ShouldBreak shouldBreak = {}) const {
    return inorderMorris(root(), std::forward<Callback>(cb), shouldBreak);
  }

  /**
   * @brief Traverse the tree in **right -> node -> left** order,
   *        In BST, inorder is used to traverse the tree in a non-decreasing order,
   *        inorder-reverse can be used to traverse the tree in a non-increasing order.
   */
  template <std::invocable<Node<T>&> Callback, std::predicate<Node<T>&> ShouldBreak = DefaultBreak<Node<T>>>
  bool inorderReverseTraverse(Callback&& cb, ShouldBreak shouldBreak = {}) {
    return inorderReverseMorris(root(), std::forward<Callback>(cb), shouldBreak);
  }

  template <
      std::invocable<const Node<T>&> Callback,
      std::predicate<const Node<T>&> ShouldBreak = DefaultBreak<const Node<T>>>
  bool inorderReverseTraverse(Callback&& cb, ShouldBreak shouldBreak = {}) const {
    return inorderReverseMorris(root(), std::forward<Callback>(cb), shouldBreak);
  }

  template <std::invocable<Node<T>&> Callback, std::predicate<Node<T>&> ShouldBreak = DefaultBreak<Node<T>>>
  bool preorderTraverse(Callback&& cb, ShouldBreak shouldBreak = {}) {
    return preorderMorris(root(), std::forward<Callback>(cb), shouldBreak);
  }

  template <
      std::invocable<const Node<T>&> Callback,
      std::predicate<const Node<T>&> ShouldBreak = DefaultBreak<const Node<T>>>
  bool preorderTraverse(Callback&& cb, ShouldBreak shouldBreak = {}) const {
    return preorderMorris(root(), std::forward<Callback>(cb), shouldBreak);
  }

  template <std::invocable<Node<T>&> Callback, std::predicate<Node<T>&> ShouldBreak = DefaultBreak<Node<T>>>
  bool postorderTraverse(Callback&& cb, ShouldBreak shouldBreak = {}) {
    return postorderIterative(root(), std::forward<Callback>(cb), shouldBreak);
  }

  template <
      std::invocable<const Node<T>&> Callback,
      std::predicate<const Node<T>&> ShouldBreak = DefaultBreak<const Node<T>>>
  bool postorderTraverse(Callback&& cb, ShouldBreak shouldBreak = {}) const {
    return postorderIterative(root(), std::forward<Callback>(cb), shouldBreak);
  }

  template <std::invocable<Node<T>&> Callback, std::predicate<Node<T>&> ShouldBreak = DefaultBreak<Node<T>>>
  bool levelorderTraverse(
      Callback&& cb, ShouldBreak shouldBreak = {} // NOLINT allow move-only callable
  ) {
    return levelorderTraverse(
        [&](Node<T>& node, size_t) { cb(node); }, [&](Node<T>& node, size_t) { return shouldBreak(node); }
    );
  }

  template <
      std::invocable<const Node<T>&> Callback,
      std::predicate<const Node<T>&> ShouldBreak = DefaultBreak<const Node<T>>>
  bool levelorderTraverse(
      Callback&& cb, ShouldBreak shouldBreak = {} // NOLINT allow move-only callable
  ) const {
    return levelorderTraverse(
        [&](const Node<T>& node, size_t) { cb(node); },
        [&](const Node<T>& node, size_t) { return shouldBreak(node); }
    );
  }

  template <
      std::invocable<Node<T>&, size_t /* level, starts from 0 */> Callback,
      std::predicate<Node<T>&, size_t /* level, starts from 0 */> ShouldBreak =
          DefaultBreakWithLevel<Node<T>>>
  bool levelorderTraverse(Callback&& cb, ShouldBreak shouldBreak = {}) {
    return levelorderIterative(root(), std::forward<Callback>(cb), shouldBreak);
  }

  template <
      std::invocable<const Node<T>&, size_t /* level, starts from 0 */> Callback,
      std::predicate<const Node<T>&, size_t /* level, starts from 0 */> ShouldBreak =
          DefaultBreakWithLevel<const Node<T>>>
  bool levelorderTraverse(Callback&& cb, ShouldBreak shouldBreak = {}) const {
    return levelorderIterative(root(), std::forward<Callback>(cb), shouldBreak);
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
    if (!empty()) clear();
    size_t n = std::ranges::size(seq);
    if (n == 0 || !seq[0].has_value()) {
      setRoot(nullptr);
      return;
    }
    fromArrayRepresentationIterative(std::forward<Seq>(seq));
  }

  void fromArrayRepresentation(std::initializer_list<std::optional<T>> l) {
    fromArrayRepresentation(array::DynamicArray<std::optional<T>>(l));
  }

  array::DynamicArray<Node<T>*> getArrayRepresentation() const {
    array::DynamicArray<Node<T>*> res;
    queue::Deque<Node<T>*> q{root()};
    size_t treeHeight = height();
    size_t level = 0;

    while (!q.empty() && level < treeHeight) {
      size_t levelSize = q.size();
      for (size_t i = 0; i < levelSize; ++i) {
        Node<T>* node = q.front();
        q.popFront();
        res.pushBack(node);
        q.pushBack(node == nullptr ? nullptr : node->left());
        q.pushBack(node == nullptr ? nullptr : node->right());
      }
      ++level;
    }
    return res;
  }

  Node<T>* insert(const T& val) noexcept { return insertImpl(val); }

  Node<T>* findFirst(const T& val) noexcept { return const_cast<Node<T>*>(findFirstImpl(val)); }
  const Node<T>* findFirst(const T& val) const noexcept { return findFirstImpl(val); }

  virtual bool eraseNode(Node<T>* target) {
    if (empty() || target == nullptr) return false;
    queue::Deque<std::pair<Node<T>*, Node<T>*>> q{{root(), nullptr}};

    while (!q.empty()) {
      auto [node, parent] = q.front();
      q.popFront();
      if (node == target) return eraseNode(target, parent);

      if (node->left() != nullptr) q.pushBack({node->left(), node});
      if (node->right() != nullptr) q.pushBack({node->right(), node});
    }
    return false;
  }

  virtual bool eraseFirst(const T& val) {
    if (empty()) return false;
    queue::Deque<std::pair<Node<T>*, Node<T>*>> q{{root(), nullptr}};

    while (!q.empty()) {
      auto [cur, parent] = q.front();
      q.popFront();

      if (keyEqual(cur->value(), val)) return eraseNode(cur, parent);

      if (cur->left() != nullptr) q.pushBack({cur->left(), cur});
      if (cur->right() != nullptr) q.pushBack({cur->right(), cur});
    }
    return false;
  }

  virtual size_t eraseAll(const T& val) {
    if (empty()) return 0;

    size_t count = 0;
    queue::Deque<std::pair<Node<T>*, Node<T>*>> q{{root(), nullptr}};
    array::DynamicArray<std::pair<Node<T>*, Node<T>*>> matches;
    while (!q.empty()) {
      auto [cur, parent] = q.front();
      q.popFront();
      if (keyEqual(cur->value(), val)) matches.pushBack({cur, parent});
      if (cur->left() != nullptr) q.pushBack({cur->left(), cur});
      if (cur->right() != nullptr) q.pushBack({cur->right(), cur});
    }

    // process child nodes first to avoid dangling parent
    for (auto it = matches.rbegin(); it != matches.rend(); ++it) {
      auto [cur, parent] = *it;
      if (eraseNode(cur, parent)) count++;
    }
    return count;
  }

  void clear() noexcept {
    if (empty()) return;
    auto deleteNode = [](Node<T>& node) {
      delete &node; // NOLINT
    };
    postorderTraverse(deleteNode);
    setRoot(nullptr);
  }

  [[nodiscard]] bool heightBalanced() const noexcept {
    if (empty()) return true;
    hashmap::HashMap<Node<T>*, int> height;

    bool balanced = true;

    auto isBalanced = [&](const Node<T>& node) {
      int leftHeight = height[node.left()];
      int rightHeight = height[node.right()];
      if (std::abs(leftHeight - rightHeight) > 1) balanced = false;
      height[&node] = 1 + std::max(leftHeight, rightHeight);
    };

    auto shouldBreak = [&](const Node<T>&) { return !balanced; };

    postorderTraverse(isBalanced, shouldBreak);
    return balanced;
  }

  [[nodiscard]] bool complete() const noexcept {
    if (empty()) return true;
    queue::Deque<Node<T>*> queue;
    queue.pushBack(root());
    bool seenNull = false;

    while (!queue.empty()) {
      Node<T>* node = queue.front();
      queue.popFront();

      if (node != nullptr) {
        if (seenNull) return false;
        queue.pushBack(node->left());
        queue.pushBack(node->right());
      } else {
        seenNull = true;
      }
    }
    return true;
  }

  [[nodiscard]] size_t height() const noexcept {
    if (empty()) return 0;
    size_t height = 0;
    queue::Deque<const Node<T>*> q{root()};

    while (!q.empty()) {
      size_t levelSize = q.size();

      for (size_t i = 0; i < levelSize; ++i) {
        const Node<T>* node = q.front();
        q.popFront();
        if (node->left() != nullptr) q.pushBack(node->left());
        if (node->right() != nullptr) q.pushBack(node->right());
      }
      ++height;
    }
    return height;
  }

  [[nodiscard]] bool full() const noexcept {
    if (empty()) return true;
    bool isFull = true;

    auto checkIsFull = [&](const Node<T>& node) {
      bool hasLeft = node.left() != nullptr;
      bool hasRight = node.right() != nullptr;
      if (hasLeft ^ hasRight) isFull = false;
    };

    auto shouldBreak = [&](const Node<T>&) { return !isFull; };
    levelorderTraverse(checkIsFull, shouldBreak);
    return isFull;
  }

  [[nodiscard]] constexpr bool empty() const noexcept { return root() == nullptr; }

  [[nodiscard]] constexpr Node<T>* root() noexcept { return m_root; }             // NOLINT
  [[nodiscard]] constexpr const Node<T>* root() const noexcept { return m_root; } // NOLINT

  void swap(BinaryTree& other) noexcept {
    std::swap(m_root, other.m_root);
    std::swap(m_hasher, other.m_hasher);
    std::swap(m_keyEqual, other.m_keyEqual);
  }
};

template <typename T, typename Hasher, typename KeyEqual>
void swap(BinaryTree<T, Hasher, KeyEqual>& a, BinaryTree<T, Hasher, KeyEqual>& b) noexcept { // for ADL
  a.swap(b);
}
} // namespace tree
