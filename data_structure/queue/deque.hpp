#pragma once
#include "../array/dynamic_array.hpp"
#include <array>
#include <cstddef>
#include <gsl/gsl>
#include <iostream>
#include <iterator>

#ifndef NDEBUG
constexpr int debugValue = 9999;
#endif

namespace queue {

template <typename T> class Deque {
private:
  class Block {
  public:
    static const size_t blockSize = 32;
    T* slot(size_t i) noexcept { return reinterpret_cast<T*>(m_block.data()) + i; } // NOLINT
    const T* slot(size_t i) const noexcept { return reinterpret_cast<const T*>(m_block.data()) + i; }

    Block() = default;
    Block(const Block&) = delete;
    Block& operator=(const Block&) = delete;
    Block(Block&&) = delete;
    Block& operator=(Block&&) = delete;
    ~Block() = default;

  private:
    alignas(T) std::array<std::byte, sizeof(T) * blockSize> m_block;
  };

  class IndexMap {
  private:
    array::DynamicArray<gsl::owner<Block*>> m_blocks;
    size_t m_blockHead = 0;
    size_t m_blockSize = 0;
    // range: [0 .. Block::blockSize), a local index in the head block
    size_t m_elementHeadLocal = getElementsPerBlock() / 2;
    // range: [0 .. Block::blockSize), a local index in the whose index is one left getBlockTail
    size_t m_elementTailLocal = m_elementHeadLocal;
    size_t m_elementSize = 0;

    void checkBound(size_t index) const {
      if (index >= getElementSize())
        throw std::out_of_range(
            "Index out of bounds, index: " + std::to_string(index) +
            ", size: " + std::to_string(getElementSize())
        );
    }

    [[nodiscard]] std::pair<size_t, size_t> posToOffsets(size_t globalPosition) const noexcept {
      size_t blockOffset = globalPosition / getElementsPerBlock();
      size_t elementOffset = globalPosition % getElementsPerBlock();
      size_t blockIndex = blockOffset % getBlockCapacity();
      return {blockIndex, elementOffset};
    }

    // blockTail points to the block one next to the block containing the tail element
    [[nodiscard]] size_t getBlockTail() const noexcept {
      return (m_blockHead + m_blockSize) % getBlockCapacity();
    }

    [[nodiscard]] size_t getElementHeadGlobal() const noexcept {
      return m_elementHeadLocal + (m_blockHead * getElementsPerBlock());
    }

    // elementTail points to the slot one position to the right of the actual tail element.
    [[nodiscard]] size_t getElementTailGlobal() const noexcept {
      return (getElementHeadGlobal() + getElementSize()) % getElementCapacity();
    }

    // the index of the actual tail element
    [[nodiscard]] size_t getTailElementGlobalIndex() const noexcept {
      size_t elementCapacity = getElementCapacity();
      return (getElementTailGlobal() - 1 + elementCapacity) % elementCapacity;
    }

    [[nodiscard]] size_t getElementCapacity() const noexcept {
      return getBlockCapacity() * getElementsPerBlock();
    }

    size_t wrapOffset(int target, int n) noexcept {
      int next = (target + n) % getElementsPerBlock();
      if (next < 0) next += getElementsPerBlock();
      return static_cast<size_t>(next);
    }

    void updateElementTailBy(int n) noexcept { m_elementTailLocal = wrapOffset(m_elementTailLocal, n); }

    void updateElementHeadBy(int n) noexcept { m_elementHeadLocal = wrapOffset(m_elementHeadLocal, n); }

    void reallocateMap(size_t newSize) {
      if (newSize <= getBlockCapacity()) return;
      array::DynamicArray<Block*> newBlocks(newSize, nullptr);

      for (size_t i = 0; i < getBlockCapacity(); ++i) {
        size_t idx = (m_blockHead + i) % getBlockCapacity();
        newBlocks[i] = m_blocks[idx];
      }
      m_blockHead = 0;
      m_blocks = std::move(newBlocks);
    }

    void ensureEnoughPrevBlocks() {
      if (isEmpty()) {
        reallocateMap(4);
      } else if (!canConstructAtHead()) {
        reallocateMap(getBlockSize() * 2);
      }
    }

    void ensureEnoughNextBlocks() {
      if (isEmpty()) {
        reallocateMap(4);
      } else if (!canConstructAtTail()) {
        reallocateMap(getBlockSize() * 2);
      }
    }

    bool ensureNextBlockAllocated() {
      if (noFreeSlotRight()) {
        constructBlock(getBlockTail());
        return true;
      }
      return false;
    }

    bool ensurePrevBlockAllocated() {
      if (noFreeSlotLeft()) {
        size_t blockIndex = (m_blockHead + getBlockCapacity() - 1) % getBlockCapacity();
        constructBlock(blockIndex);
        return true;
      }
      return false;
    }

    bool ensureCurrentBlockAllocated() {
      if (m_blockSize == 0) {
        constructBlock(m_blockHead);
        return true;
      }
      return false;
    }

    gsl::owner<Block*> allocateBlock() {
      Block* block = new Block(); // NOLINT
#ifndef NDEBUG
      for (size_t i = 0; i < getElementsPerBlock(); i++) { new (block->slot(i)) int(debugValue); }
#endif
      return block;
    }

    void deallocateBlock(gsl::owner<Block*> block) noexcept { delete block; }

    void deallocateBlocksFromHead(size_t newBlockHead) noexcept {
      while (m_blockHead != newBlockHead) {
        deallocateBlock(m_blocks[m_blockHead]); // NOLINT
        m_blocks[m_blockHead] = nullptr;
        m_blockHead = (m_blockHead + 1) % getBlockCapacity();
        m_blockSize--;
      }
    }

    void deallocateBlocksFromTail(size_t newTailAllocatedBlock) noexcept {
      size_t tailAllocatedBlock = (getBlockTail() + getBlockCapacity() - 1) % getBlockCapacity();

      while (tailAllocatedBlock != newTailAllocatedBlock) {
        deallocateBlock(m_blocks[tailAllocatedBlock]); // NOLINT
        m_blocks[tailAllocatedBlock] = nullptr;
        tailAllocatedBlock = (tailAllocatedBlock - 1) % getBlockCapacity();
        m_blockSize--;
      }
    }

    void constructBlock(size_t blockIndex) {
      if (m_blocks[blockIndex] == nullptr) {
        m_blocks[blockIndex] = allocateBlock(); // NOLINT
        m_blockSize++;
      }
    }

    [[nodiscard]] bool isBlocksFull() const noexcept { return m_blockSize == getBlockCapacity(); }
    [[nodiscard]] bool noFreeSlotLeft() const noexcept { return m_elementHeadLocal == 0; }
    [[nodiscard]] bool noFreeSlotRight() const noexcept { return m_elementTailLocal == 0; }

    [[nodiscard]] bool canConstructAtHead() const noexcept { return !isBlocksFull() || !noFreeSlotLeft(); }
    [[nodiscard]] bool canConstructAtTail() const noexcept { return !isBlocksFull() || !noFreeSlotRight(); }

    void deepCopy(const IndexMap& other) {
      reallocateMap(other.getBlockCapacity());
      for (size_t i = 0; i < other.getBlockCapacity(); ++i) {
        if (other.m_blocks[i] != nullptr) constructBlock(i);
      }

      size_t start = other.getElementHeadGlobal();
      size_t elementCapacity = getElementCapacity();
      size_t i = 0;

      try {
        for (; i < other.getElementSize(); ++i) {
          size_t idx = (start + i) % elementCapacity;
          constructAt(idx, *other.slotAt(idx));
        }
        m_blockHead = other.m_blockHead;
        m_elementHeadLocal = other.m_elementHeadLocal;
        m_elementTailLocal = other.m_elementTailLocal;
        m_elementSize = other.m_elementSize;
      } catch (...) {
        for (size_t j = 0; j < i; ++j) destroyAt((start + j) % elementCapacity);
        m_elementSize = 0;
        m_elementHeadLocal = getElementsPerBlock() / 2;
        m_elementTailLocal = m_elementHeadLocal;
        throw;
      }
    }

    void move(IndexMap&& other) noexcept { // NOLINT
      m_blocks = std::move(other.m_blocks);
      m_elementHeadLocal = other.m_elementHeadLocal;
      m_elementTailLocal = other.m_elementTailLocal;
      m_elementSize = other.m_elementSize;
      m_blockHead = other.m_blockHead;
      m_blockSize = other.m_blockSize;

      other.m_elementHeadLocal = getElementsPerBlock() / 2;
      other.m_elementTailLocal = other.m_elementHeadLocal;
      other.m_elementSize = 0;
      other.m_blockHead = 0;
      other.m_blockSize = 0;
    }

  public:
    template <bool IsConst> class IndexMapIterator {
      template <bool> friend class IndexMapIterator;
      friend class IndexMap;
      template <typename> friend class Deque;

    private:
      using RawPtr = std::conditional_t<IsConst, const T*, T*>;
      using MapPtr = std::conditional_t<IsConst, const IndexMap*, IndexMap*>;
      MapPtr m_map = nullptr;
      size_t m_offsetFromHead = 0;

      // should be able to be constructed by itself or friends (i.e. IndexMap)
      IndexMapIterator(MapPtr map, size_t offsetFromHead) : m_map(map), m_offsetFromHead(offsetFromHead) {};

    public:
      [[nodiscard]] size_t getPosition(std::ptrdiff_t n = 0) const noexcept {
        return (m_map->getElementHeadGlobal() + m_offsetFromHead + n) % m_map->getElementCapacity();
      }
      using value_type = T;
      using reference = std::conditional_t<IsConst, const T&, T&>;
      using pointer = RawPtr;
      using difference_type = std::ptrdiff_t;
      using iterator_category = std::random_access_iterator_tag;

      IndexMapIterator() = default;
      IndexMapIterator(const IndexMapIterator&) = default;
      IndexMapIterator& operator=(const IndexMapIterator&) = default;
      IndexMapIterator(IndexMapIterator&&) noexcept = default;
      IndexMapIterator& operator=(IndexMapIterator&&) noexcept = default;
      ~IndexMapIterator() noexcept = default;

      // conversion constructor Iterator -> ConstIterator
      IndexMapIterator(const IndexMapIterator<false>& other)
        requires IsConst
          : m_map(other.m_map), m_offsetFromHead(other.m_offsetFromHead) {}

      reference operator*() const { return *m_map->slotAt(getPosition()); }
      pointer operator->() const { return m_map->slotAt(getPosition()); }

      reference operator[](difference_type n) const { return *m_map->slotAt(getPosition(n)); }

      IndexMapIterator& operator+=(difference_type n) {
        m_offsetFromHead += n;
        return *this;
      }

      IndexMapIterator& operator++() {
        m_offsetFromHead++;
        return *this;
      }

      IndexMapIterator operator++(int) {
        IndexMapIterator snapshot = *this;
        m_offsetFromHead++;
        return snapshot;
      }

      IndexMapIterator operator+(difference_type n) const {
        return IndexMapIterator(m_map, m_offsetFromHead + n);
      }

      friend IndexMapIterator operator+(difference_type n, IndexMapIterator it) {
        it += n;
        return it;
      }

      IndexMapIterator& operator-=(difference_type n) {
        m_offsetFromHead -= n;
        return *this;
      }

      IndexMapIterator& operator--() {
        m_offsetFromHead--;
        return *this;
      }

      IndexMapIterator operator--(int) {
        IndexMapIterator snapshot = *this;
        m_offsetFromHead--;
        return snapshot;
      }

      IndexMapIterator operator-(difference_type n) const {
        return IndexMapIterator(m_map, m_offsetFromHead - n);
      }

      difference_type operator-(const IndexMapIterator& other) const {
        return m_offsetFromHead - other.m_offsetFromHead;
      }

      auto operator<=>(const IndexMapIterator& other) const {
        return m_offsetFromHead <=> other.m_offsetFromHead;
      }

      bool operator==(const IndexMapIterator& other) const {
        return m_offsetFromHead == other.m_offsetFromHead;
      }
    };

    using Iterator = IndexMapIterator<false>;
    using ConstIterator = IndexMapIterator<true>;
    using ReverseIterator = std::reverse_iterator<Iterator>;
    using ConstReverseIterator = std::reverse_iterator<ConstIterator>;

    IndexMap() = default;

    IndexMap(const IndexMap& other) { deepCopy(other); }

    IndexMap& operator=(const IndexMap& other) {
      if (&other == this) return *this;
      clear();
      deepCopy(other);
      return *this;
    }

    IndexMap(IndexMap&& other) noexcept { move(std::move(other)); }

    IndexMap& operator=(IndexMap&& other) noexcept {
      if (&other == this) return *this;
      clear();
      move(std::move(other));
      return *this;
    }

    ~IndexMap() noexcept { clear(); }

    T* slotAt(size_t pos) noexcept {
      auto [blockIndex, elementOffset] = posToOffsets(pos);
      return m_blocks[blockIndex]->slot(elementOffset);
    }

    const T* slotAt(size_t pos) const noexcept {
      auto [blockIndex, elementOffset] = posToOffsets(pos);
      return m_blocks[blockIndex]->slot(elementOffset);
    }

    template <typename... Args> T* constructAt(size_t pos, Args&&... args) {
      return new (static_cast<void*>(slotAt(pos))) T(std::forward<Args>(args)...); // NOLINT
    }

    template <typename... Args> T* constructAtTail(Args&&... args) {
      ensureEnoughNextBlocks();

      if (m_blockSize == 0) {
        ensureCurrentBlockAllocated();
      } else {
        ensureNextBlockAllocated();
      }
      T* ptr = nullptr;
      try {
        ptr = constructAt(getElementTailGlobal(), std::forward<Args>(args)...);
        updateElementTailBy(1);
        m_elementSize++;
      } catch (...) { throw; }
      return ptr;
    }

    template <typename... Args> T* constructAtHead(Args&&... args) {
      ensureEnoughPrevBlocks();
      bool prevBlockAllocated = false;
      size_t prevElementHead = m_elementHeadLocal;
      size_t prevBlockHead = m_blockHead;
      T* ptr = nullptr;

      if (m_blockSize == 0) {
        ensureCurrentBlockAllocated();
      } else {
        prevBlockAllocated = ensurePrevBlockAllocated();
      }

      try {
        updateElementHeadBy(-1);
        if (prevBlockAllocated) m_blockHead = (m_blockHead + getBlockCapacity() - 1) % getBlockCapacity();
        ptr = constructAt(getElementHeadGlobal(), std::forward<Args>(args)...);
      } catch (...) {
        m_elementHeadLocal = prevElementHead;
        m_blockHead = prevBlockHead;
        throw;
      }
      m_elementSize++;
      return ptr;
    }

    void destroyAt(size_t pos) noexcept {
      slotAt(pos)->~T();
#ifndef NDEBUG
      new (slotAt(pos)) int(debugValue);
#endif
    }

    void destroyRange(ConstIterator first, size_t count) noexcept {
      for (size_t i = 0; i < count; ++i) { destroyAt(first.getPosition(i)); }
    }

    void destroyAndShiftLeft(ConstIterator first, ConstIterator last) {
      size_t count = last - first;
      constexpr bool preferMove = std::is_nothrow_move_constructible_v<T> || !std::is_copy_constructible_v<T>;
      size_t elementsToShift = getElementSize() - first.m_offsetFromHead - count;

      for (size_t i = 0; i < elementsToShift; ++i) {
        if constexpr (preferMove) {
          *slotAt(first.getPosition(i)) = std::move(*slotAt(last.getPosition(i)));
        } else {
          *slotAt(first.getPosition(i)) = *slotAt(last.getPosition(i));
        }
      }
      for (size_t i = elementsToShift; i < elementsToShift + count; ++i) { destroyAt(first.getPosition(i)); }
    }

    void destroyAtHead() {
      if (isEmpty()) throw std::underflow_error("Head block is empty");
      destroyAt(getElementHeadGlobal());
      bool blockToBeEmpty = m_elementHeadLocal == (getElementsPerBlock() - 1);
      if (blockToBeEmpty) {
        m_elementHeadLocal = 0;
        deallocateBlock(m_blocks[m_blockHead]); // NOLINT
        m_blocks[m_blockHead] = nullptr;
        m_blockHead = (m_blockHead + 1) % getBlockCapacity();
        m_blockSize--;
      } else {
        m_elementHeadLocal++;
      }
      m_elementSize--;
    }

    void destroyAtTail() {
      if (isEmpty()) throw std::underflow_error("Tail block is empty");
      bool blockToBeEmpty = m_elementTailLocal == 1;
      destroyAt(getTailElementGlobalIndex());

      if (blockToBeEmpty) {
        auto [elementTailBlockIdx, _] = posToOffsets(getElementTailGlobal());
        deallocateBlock(m_blocks[elementTailBlockIdx]); // NOLINT
        m_blocks[elementTailBlockIdx] = nullptr;
        m_blockSize--;
      }
      updateElementTailBy(-1);
      m_elementSize--;
    }

    T& head() {
      if (isEmpty()) throw std::underflow_error("Head block is empty");
      return *slotAt(getElementHeadGlobal());
    }

    const T& head() const {
      if (isEmpty()) throw std::underflow_error("Head block is empty");
      return *slotAt(getElementHeadGlobal());
    }

    T& tail() {
      if (isEmpty()) throw std::underflow_error("Tail block is empty");
      return *slotAt(getTailElementGlobalIndex());
    }

    const T& tail() const {
      if (isEmpty()) throw std::underflow_error("Tail block is empty");
      return *slotAt(getTailElementGlobalIndex());
    }

    void clear() noexcept {
      size_t start = getElementHeadGlobal();
      size_t blockCapacity = getBlockCapacity();
      for (size_t i = 0; i < getElementSize(); ++i) {
        size_t idx = (start + i) % getElementCapacity();
        destroyAt(idx);
      }

      for (size_t i = 0; i < blockCapacity; ++i) {
        if (m_blocks[i] != nullptr) {
          deallocateBlock(m_blocks[i]); // NOLINT
          m_blocks[i] = nullptr;
        }
      }

      m_blockHead = 0;
      m_blockSize = 0;
      m_elementHeadLocal = getElementsPerBlock() / 2;
      m_elementTailLocal = m_elementHeadLocal;
      m_elementSize = 0;
    }

    Iterator erase(ConstIterator pos) { return erase(pos, pos + 1); }

    Iterator erase(ConstIterator first, ConstIterator last) {
      if (first < cbegin() || last > cend() || first > last) {
        throw std::out_of_range("Erase positions out of range");
      }

      size_t count = static_cast<size_t>(last - first);
      if (count == 0) return Iterator{this, first.m_offsetFromHead};

      if (first == cbegin()) {
        destroyRange(first, count);
        auto [newBlockHead, _] = posToOffsets(first.getPosition(count));
        deallocateBlocksFromHead(newBlockHead);
        updateElementHeadBy(count);
      } else {
        if (last == cend()) {
          destroyRange(first, count);
        } else {
          destroyAndShiftLeft(first, last);
        }

        size_t elementCapacity = getElementCapacity();
        auto [newTailAllocatedBlock, _] =
            posToOffsets((getTailElementGlobalIndex() - count + elementCapacity) % elementCapacity);

        deallocateBlocksFromTail(newTailAllocatedBlock);
        updateElementTailBy(-static_cast<int>(count));
      }

      m_elementSize -= count;
      return Iterator{this, first.m_offsetFromHead};
    }

    template <typename... Args> Iterator emplace(ConstIterator pos, Args&&... args) {
      if (pos < cbegin() || pos > cend()) { throw std::out_of_range("Emplace position out of range"); }

      if (pos == cbegin()) {
        constructAtHead(std::forward<Args>(args)...);
        return begin();
      }

      if (pos == cend()) {
        constructAtTail(std::forward<Args>(args)...);
        return Iterator{this, getElementSize() - 1};
      }

      // guaranteed to be > 0
      size_t offset = pos.m_offsetFromHead;

      constexpr bool preferMove = std::is_nothrow_move_constructible_v<T> || !std::is_copy_constructible_v<T>;

      bool fewerElementsOnLeft = pos - cbegin() <= cend() - pos;
      if (fewerElementsOnLeft) {
        //  move left
        ConstIterator first = cbegin();
        T head = preferMove ? std::move(*first) : *first;

        for (size_t i = 1; i < offset; ++i) {
          if constexpr (preferMove) {
            *slotAt(first.getPosition(i - 1)) = std::move(*slotAt(first.getPosition(i)));
          } else {
            *slotAt(first.getPosition(i - 1)) = *slotAt(first.getPosition(i));
          }
        }

        size_t insertionPosition = first.getPosition(offset - 1);
        destroyAt(insertionPosition);
        constructAt(insertionPosition, std::forward<Args>(args)...);
        constructAtHead(head);

      } else {
        // move right
        size_t elementsToShift = getElementSize() - offset;

        T tail = preferMove ? std::move(*slotAt(pos.getPosition(elementsToShift - 1)))
                            : *slotAt(pos.getPosition(elementsToShift - 1));

        for (size_t i = elementsToShift - 1; i > 0; --i) {
          if constexpr (preferMove) {
            *slotAt(pos.getPosition(i)) = std::move(*slotAt(pos.getPosition(i - 1)));
          } else {
            *slotAt(pos.getPosition(i)) = *slotAt(pos.getPosition(i - 1));
          }
        }

        size_t insertionPosition = pos.getPosition();
        destroyAt(insertionPosition);
        constructAt(insertionPosition, std::forward<Args>(args)...);
        constructAtTail(tail);
      }
      return Iterator{this, offset};
    }

    T& operator[](size_t index) {
      checkBound(index);
      size_t idx = (getElementHeadGlobal() + index) % getElementCapacity();
      return *slotAt(idx);
    }

    const T& operator[](size_t index) const {
      checkBound(index);
      size_t idx = (getElementHeadGlobal() + index) % getElementCapacity();
      return *slotAt(idx);
    }

    T& at(size_t index) { return operator[](index); }
    const T& at(size_t index) const { return operator[](index); }

    Iterator begin() noexcept { return Iterator{this, 0}; }
    ConstIterator begin() const noexcept { return ConstIterator{this, 0}; }
    ConstIterator cbegin() const noexcept { return ConstIterator{this, 0}; }

    Iterator end() noexcept { return Iterator{this, getElementSize()}; }
    ConstIterator end() const noexcept { return ConstIterator{this, getElementSize()}; }
    ConstIterator cend() const noexcept { return ConstIterator{this, getElementSize()}; }

    ReverseIterator rbegin() noexcept { return ReverseIterator{end()}; }
    ConstReverseIterator rbegin() const noexcept { return ConstReverseIterator{end()}; }
    ConstReverseIterator crbegin() const noexcept { return ConstReverseIterator{end()}; }

    ReverseIterator rend() noexcept { return ReverseIterator{begin()}; }
    ConstReverseIterator rend() const noexcept { return ConstReverseIterator{begin()}; }
    ConstReverseIterator crend() const noexcept { return ConstReverseIterator{begin()}; }

#ifndef NDEBUG
    static std::string colorize(const std::string& s, const char* colorCode) noexcept {
      return std::string(colorCode) + s + "\033[0m";
    }

    std::string
    formatBlock(const Block* block, size_t blockIdx, size_t elemIdx, size_t elemTailBlockIdx) const {
      // Empty slot
      if (block == nullptr) {
        std::string s = padStart("*", 4, '*');
        if (elemIdx == m_elementTailLocal && blockIdx == getBlockTail() && noFreeSlotRight()) {
          return colorize(s, "\033[34m"); // blue
        }
        return s;
      }

      // Occupied slot
      const T* slotData = block->slot(elemIdx);
      if (*slotData == debugValue) {
        std::string s = "--";
        if (elemIdx == m_elementTailLocal && blockIdx == elemTailBlockIdx && !noFreeSlotRight()) {
          if (elemIdx == m_elementHeadLocal) {
            return colorize(padStart(s, 2, '-'), "\033[31m") + // red
                   colorize(padStart(s, 2, '-'), "\033[34m");  // blue
          }
          return colorize(padStart(s, 4, '-'), "\033[34m"); // tail -> blue
        }
        return padStart(s, 4, '-');
      }

      // Normal data slot
      std::string s = std::to_string(*slotData);
      size_t globalIdx = elemIdx + (blockIdx * getElementsPerBlock());
      size_t headIdx = getElementHeadGlobal();
      if (globalIdx == headIdx) {
        return colorize(padStart(s, 4, '0'), "\033[31m"); // head -> red
      }
      return padStart(s, 4, '0');
    }

    void printBlocks() const noexcept {
      for (size_t i = 0; i < getBlockCapacity(); i++) {
        if (!m_blocks[i]) {
          std::cout << "----- ";
        } else {
          std::cout << "Block" << (i == m_blockHead ? "(*)" : "") << ' ';
        }
      }
      std::cout << '\n';
    }

    void printBlock(const std::vector<size_t>& indices, size_t colsPerRow = 4) const noexcept {
      size_t rows = getElementsPerBlock() / colsPerRow;
      size_t elemTailBlockIdx = (getBlockTail() - 1 + getBlockCapacity()) % getBlockCapacity();

      for (size_t r = 0; r < rows; ++r) {
        for (size_t i : indices) {
          if (i >= getBlockCapacity()) continue;
          Block* block = m_blocks[i];
          for (size_t c = 0; c < colsPerRow; ++c) {
            size_t elemIdx = c + (colsPerRow * r);
            std::cout << formatBlock(block, i, elemIdx, elemTailBlockIdx) << ' ';
            if (c == colsPerRow - 1) std::cout << " | ";
          }
        }
        std::cout << '\n';
      }
    }

    void printInfo() const noexcept {
      std::cout << "Head Index: " << m_blockHead << '\n'
                << "Tail Index: " << getBlockTail() << '\n'
                << "Block Size: " << m_blockSize << '\n'
                << "Block Capacity: " << getBlockCapacity() << '\n'
                << "Element Size: " << getElementSize() << '\n';
    }

    [[nodiscard]] std::string padStart(const std::string& s, std::size_t length, char fill) const {
      if (s.size() >= length) return s;

      const std::size_t padCount = length - s.size();

      bool isNegativeInt = !s.empty() && s[0] == '-' && std::all_of(s.begin() + 1, s.end(), [](char ch) {
        return std::isdigit(static_cast<unsigned char>(ch));
      });

      if (isNegativeInt && fill == '0') { return "-" + std::string(padCount, '0') + s.substr(1); }

      return std::string(padCount, fill) + s;
    }
#endif

    [[nodiscard]] bool isEmpty() const noexcept { return getElementSize() == 0; }
    [[nodiscard]] bool isFull() const noexcept {
      return getElementSize() == getBlockSize() * getElementsPerBlock();
    }
    [[nodiscard]] size_t getElementsPerBlock() const noexcept { return Block::blockSize; }

    [[nodiscard]] size_t getElementSize() const noexcept { return m_elementSize; }

    [[nodiscard]] size_t getBlockSize() const noexcept { return m_blockSize; }
    [[nodiscard]] size_t getBlockCapacity() const noexcept { return m_blocks.getSize(); }
  };

  IndexMap m_indexMap;

public:
  using value_type = T;
  using size_type = size_t;
  using reference = T&;
  using const_reference = const T&;
  using Iterator = IndexMap::template IndexMapIterator<false>;
  using ConstIterator = IndexMap::template IndexMapIterator<true>;
  using ReverseIterator = IndexMap::ReverseIterator;
  using ConstReverseIterator = IndexMap::ConstReverseIterator;

  Deque() = default;
  Deque(std::initializer_list<T> l) { insert(cbegin(), l); }

  template <typename... Args> Iterator emplace(ConstIterator pos, Args&&... args) {
    return m_indexMap.emplace(pos, std::forward<Args>(args)...);
  }

  template <typename... Args> reference emplaceBack(Args&&... args) {
    return *m_indexMap.emplace(cend(), std::forward<Args>(args)...);
  }

  void pushBack(const T& val) { emplaceBack(val); }
  void pushBack(T&& val) { emplaceBack(std::move(val)); }

  template <typename... Args> reference emplaceFront(Args&&... args) {
    return *m_indexMap.emplace(cbegin(), std::forward<Args>(args)...);
  }

  void pushFront(const T& val) { emplaceFront(val); }
  void pushFront(T&& val) { emplaceFront(std::move(val)); }

  void popFront() {
    if (isEmpty()) throw std::underflow_error("Dqeue is empty");
    m_indexMap.destroyAtHead();
  }

  void popBack() {
    if (isEmpty()) throw std::underflow_error("Deque is empty");
    m_indexMap.destroyAtTail();
  }

  reference front() {
    if (isEmpty()) throw std::underflow_error("Deque is empty");
    return m_indexMap.head();
  }

  const_reference front() const {
    if (isEmpty()) throw std::underflow_error("Deque is empty");
    return m_indexMap.head();
  }

  reference back() {
    if (isEmpty()) throw std::underflow_error("Deque is empty");
    return m_indexMap.tail();
  }

  const_reference back() const {
    if (isEmpty()) throw std::underflow_error("Deque is empty");
    return m_indexMap.tail();
  }

  Iterator erase(ConstIterator pos) { return m_indexMap.erase(pos); }
  Iterator erase(ConstIterator first, ConstIterator last) { return m_indexMap.erase(first, last); }

  Iterator insert(ConstIterator pos, const T& value) { return m_indexMap.emplace(pos, value); }
  Iterator insert(ConstIterator pos, T&& value) { return m_indexMap.emplace(pos, std::move(value)); }
  template <std::input_iterator InputIterator>
  Iterator insert(ConstIterator pos, InputIterator first, InputIterator last) {
    ConstIterator it = pos;

    // if user passed in iterators of deque
    if constexpr (requires(InputIterator it) { it.m_map; }) {
      // if first and last are from the same deque as the current one
      if (pos.m_map == first.m_map && pos.m_map == last.m_map) {
        std::vector<value_type> copy{first, last};
        for (value_type& v : copy) {
          emplace(it, v);
          it++;
        }
      }
    } else {
      for (; first != last; ++first) {
        emplace(it, *first);
        it++;
      }
    }
    return Iterator(&this->m_indexMap, pos.m_offsetFromHead);
  }

  Iterator insert(ConstIterator pos, std::initializer_list<T> l) { return insert(pos, l.begin(), l.end()); }

  void clear() noexcept { m_indexMap.clear(); }

  reference operator[](size_t index) { return m_indexMap.at(index); }
  const_reference operator[](size_t index) const { return m_indexMap.at(index); }
  reference at(size_t index) { return m_indexMap.at(index); }
  const_reference at(size_t index) const { return m_indexMap.at(index); }

#ifndef NDEBUG
  void printBlocks() const noexcept { m_indexMap.printBlocks(); }
  void printBlock(const std::vector<size_t>& indices) const noexcept { m_indexMap.printBlock(indices); }
  void printInfo() const noexcept { m_indexMap.printInfo(); }
#endif

  [[nodiscard]] bool isEmpty() noexcept { return getSize() == 0; }
  [[nodiscard]] size_t getSize() noexcept { return m_indexMap.getElementSize(); }

  Iterator begin() noexcept { return m_indexMap.begin(); }
  ConstIterator begin() const noexcept { return m_indexMap.begin(); }
  ConstIterator cbegin() const noexcept { return m_indexMap.cbegin(); }

  Iterator end() noexcept { return m_indexMap.end(); }
  ConstIterator end() const noexcept { return m_indexMap.end(); }
  ConstIterator cend() const noexcept { return m_indexMap.cend(); }

  ReverseIterator rbegin() noexcept { return m_indexMap.rbegin(); }
  ConstReverseIterator rbegin() const noexcept { return m_indexMap.rbegin(); }
  ConstReverseIterator crbegin() const noexcept { return m_indexMap.crbegin(); }

  ReverseIterator rend() noexcept { return m_indexMap.rend(); }
  ConstReverseIterator rend() const noexcept { return m_indexMap.rend(); }
  ConstReverseIterator crend() const noexcept { return m_indexMap.crend(); }
};
} // namespace queue
