#pragma once
#include "../array/dynamic_array.hpp"
#include <array>
#include <cstddef>
#include <gsl/gsl>
#include <iostream>

namespace queue {

template <typename T> class Deque {
private:
  class Block {
  public:
    static const size_t blockSize = 32;
    T* slot(size_t i) noexcept { return reinterpret_cast<T*>(m_block.data()) + i; }
    const T* slot(size_t i) const noexcept { return reinterpret_cast<const T*>(m_block.data()) + i; }

    Block() = default;
    Block(const Block&) = delete;
    Block& operator=(const Block&) = delete;
    Block(Block&&) = delete;
    Block& operator=(Block&&) = delete;
    ~Block() = delete;

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

    [[nodiscard]] size_t getElementTailGlobal() const noexcept {
      return (getElementHeadGlobal() + getElementSize()) % getElementCapacity();
    }

    [[nodiscard]] size_t getTailElementGlobalIndex() const noexcept {
      size_t elementCapacity = getElementCapacity();
      return (getElementTailGlobal() - 1 + elementCapacity) % elementCapacity;
    }

    [[nodiscard]] size_t getElementCapacity() const noexcept {
      return getBlockCapacity() * getElementsPerBlock();
    }

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

    [[nodiscard]] bool isBlocksFull() const noexcept { return m_blockSize == getBlockCapacity(); }
    [[nodiscard]] bool noFreeSlotLeft() const noexcept { return m_elementHeadLocal == 0; }
    [[nodiscard]] bool noFreeSlotRight() const noexcept { return m_elementTailLocal == 0; }

    [[nodiscard]] bool canConstructAtHead() const noexcept { return !isBlocksFull() || !noFreeSlotLeft(); }
    [[nodiscard]] bool canConstructAtTail() const noexcept { return !isBlocksFull() || !noFreeSlotRight(); }

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

    Block* allocateBlock() {
      Block* block = new Block(); // NOLINT
      // delete this line, it is for testing
      for (size_t i = 0; i < getElementsPerBlock(); i++) { new (block->slot(i)) int(9999); }
      return block;
    }

    // Block* allocateBlock() {
    //   // almost the same as returning new Block(); since Block is trivially-constructible
    //   return new Block(); // NOLINT
    //   // std::array is not trivially-constructible
    //   // Block* block = static_cast<Block*>(::operator new(sizeof(Block)));
    //   // return block;
    // }

    void deallocateBlock(Block* block) noexcept { ::operator delete(block); }

    void constructBlock(size_t blockIndex) {
      if (m_blocks[blockIndex] == nullptr) {
        m_blocks[blockIndex] = allocateBlock();
        m_blockSize++;
      }
    }

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

    private:
      using RawPtr = std::conditional_t<IsConst, const T*, T*>;
      IndexMap* m_map = nullptr;
      size_t m_offsetFromHead = 0;

      [[nodiscard]] size_t getPosition() const noexcept {
        return (m_map->getElementHeadGlobal() + m_offsetFromHead) % m_map->getElementCapacity();
      }

      // should be able to be constructed by itself or friends (i.e. IndexMap)
      IndexMapIterator(IndexMap* map, size_t offsetFromHead)
          : m_map(map), m_offsetFromHead(offsetFromHead) {};

      // conversion constructor Iterator -> ConstIterator
      IndexMapIterator(const IndexMapIterator<false>& other)
        requires IsConst
          : m_map(other.m_map), m_offsetFromHead(other.m_offsetFromHead) {}

    public:
      using value_type = T;
      using reference = std::conditional_t<IsConst, const T&, T&>;
      using pointer = RawPtr;
      using difference_type = std::ptrdiff_t;
      using iterator_category = std::random_access_iterator_tag;

      IndexMapIterator() = default;
      reference operator*() const { return *m_map->slotAt(getPosition()); }
      pointer operator->() const { return m_map->slotAt(getPosition()); }

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
        m_elementTailLocal = (m_elementTailLocal + 1) % getElementsPerBlock();
        m_elementSize++;
      } catch (...) { throw; }
      return ptr;
    }

    template <typename... Args> T* constructAtHead(Args&&... args) {
      ensureEnoughPrevBlocks();
      bool prevBlockAllocated = false;
      size_t prevElemHead = m_elementHeadLocal;
      size_t prevBlockHead = m_blockHead;
      T* ptr = nullptr;

      if (m_blockSize == 0) {
        ensureCurrentBlockAllocated();
      } else {
        prevBlockAllocated = ensurePrevBlockAllocated();
      }

      try {
        m_elementHeadLocal = (m_elementHeadLocal + getElementsPerBlock() - 1) % getElementsPerBlock();
        if (prevBlockAllocated) m_blockHead = (m_blockHead + getBlockCapacity() - 1) % getBlockCapacity();
        ptr = constructAt(getElementHeadGlobal(), std::forward<Args>(args)...);
      } catch (...) {
        m_elementHeadLocal = prevElemHead;
        m_blockHead = prevBlockHead;
        throw;
      }
      m_elementSize++;
      return ptr;
    }

    void destroyAt(size_t pos) noexcept {
      slotAt(pos)->~T();
      // delete this line, this is for testing
      new (slotAt(pos)) int(9999);
    }

    void destroyAtHead() {
      if (isEmpty()) throw std::underflow_error("Head block is empty");
      destroyAt(getElementHeadGlobal());
      bool blockToBeEmpty = m_elementHeadLocal == (getElementsPerBlock() - 1);
      if (blockToBeEmpty) {
        m_elementHeadLocal = 0;
        deallocateBlock(m_blocks[m_blockHead]);
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
        deallocateBlock(m_blocks[elementTailBlockIdx]);
        m_blocks[elementTailBlockIdx] = nullptr;
        m_blockSize--;
      }
      m_elementTailLocal = (m_elementTailLocal + getElementsPerBlock() - 1) % getElementsPerBlock();
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
          deallocateBlock(m_blocks[i]);
          m_blocks[i] = nullptr;
        }
      }

      m_blockHead = 0;
      m_blockSize = 0;
      m_elementHeadLocal = getElementsPerBlock() / 2;
      m_elementTailLocal = m_elementHeadLocal;
      m_elementSize = 0;
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
    ConstIterator begin() const noexcept { return Iterator{this, 0}; }
    ConstIterator cbegin() const noexcept { return Iterator{this, 0}; }

    Iterator end() noexcept { return Iterator{this, getElementSize()}; }
    ConstIterator end() const noexcept { return Iterator{this, getElementSize()}; }
    ConstIterator cend() const noexcept { return Iterator{this, getElementSize()}; }

    void i_printBlocks() const noexcept {
      for (size_t i = 0; i < getBlockCapacity(); i++) {
        if (m_blocks[i] == nullptr)
          std::cout << "-----" << ' ';
        else {
          if (i == m_blockHead) {
            std::cout << "Block(*)" << ' ';
          } else {
            std::cout << "Block" << ' ';
          }
        }
      }
      std::cout << '\n';
    }

    void i_printBlock(size_t i) const noexcept {
      Block* block = m_blocks[i];
      for (size_t i = 0; i < getElementsPerBlock(); i++) {
        T* slotData = block->slot(i);
        if (*slotData == 9999) {
          std::string s = "--";
          std::cout << i_padStart(s, 4, '-') << ' ';
        } else {
          std::string s = std::to_string(*slotData);
          std::cout << (i_padStart(s, 4, '0')) << " ";
        }
        if ((i + 1) % 4 == 0) std::cout << '\n';
      }
    }

    void i_printBlock(const std::vector<size_t>& indices) const noexcept {
      size_t colsPerRow = 4;
      size_t rows = getElementsPerBlock() / colsPerRow;
      for (size_t r = 0; r < rows; ++r) {
        for (size_t i : indices) {
          if (i > getBlockCapacity() - 1) continue;
          Block* block = m_blocks[i];
          size_t elemTailBlockIdx = (getBlockTail() - 1 + getBlockCapacity()) % getBlockCapacity();
          for (size_t c = 0; c < colsPerRow; ++c) {
            size_t elemIdx = c + (colsPerRow * r);
            if (block == nullptr) {
              std::string s = "*";
              if (elemIdx == m_elementTailLocal && i == getBlockTail() && noFreeSlotRight()) {
                // elemTail -> blue
                std::cout << "\033[34m" << (i_padStart(s, 4, '*')) << "\033[0m" << ' ';
              } else {
                std::cout << i_padStart(s, 4, '*') << ' ';
              }
            } else {
              T* slotData = block->slot(elemIdx);
              if (*slotData == 9999) {
                std::string s = "--";
                if (elemIdx == m_elementTailLocal && i == elemTailBlockIdx && !noFreeSlotRight()) {
                  if (elemIdx == m_elementHeadLocal) {
                    std::cout << "\033[31m" << (i_padStart(s, 2, '-')) << "\033[0m" << "\033[34m"
                              << (i_padStart(s, 2, '-')) << "\033[0m" << ' ';
                  } else {
                    // elemTail -> blue
                    std::cout << "\033[34m" << (i_padStart(s, 4, '-')) << "\033[0m" << ' ';
                  }
                } else {
                  std::cout << i_padStart(s, 4, '-') << ' ';
                }
              } else {
                std::string s = std::to_string(*slotData);
                if (elemIdx + (i * getElementsPerBlock()) ==
                    m_elementHeadLocal + (m_blockHead * getElementsPerBlock())) {
                  // elemHead -> red
                  std::cout << "\033[31m" << (i_padStart(s, 4, '0')) << "\033[0m" << ' ';
                } else {
                  std::cout << (i_padStart(s, 4, '0')) << ' ';
                }
              }
            }
            if (c == colsPerRow - 1) std::cout << " | ";
          }
        }
        std::cout << '\n';
      }
    }

    void i_printInfo() const noexcept {
      std::cout << "Head Index: " << m_blockHead << '\n';
      std::cout << "Tail Index: " << (m_blockHead + m_blockSize) % getBlockCapacity() << '\n';
      std::cout << "Block Size: " << m_blockSize << '\n';
      std::cout << "Block Capacity: " << getBlockCapacity() << '\n';
      std::cout << "Element Size: " << getElementSize() << '\n';
    }

    std::string i_padStart(const std::string& s, size_t length, char fill) const {
      if (s.size() >= length) return s;
      return std::string(length - s.size(), fill) + s;
    }

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

  Deque() = default;
  // begin, end, cbein, cend, erase, insert, iterator
  template <typename... Args> reference emplaceBack(Args&&... args) {
    return *(m_indexMap.constructAtTail(std::forward<Args>(args)...));
  }

  void pushBack(const T& val) { emplaceBack(val); }
  void pushBack(T&& val) { emplaceBack(std::move(val)); }

  template <typename... Args> reference emplaceFront(Args&&... args) {
    return *(m_indexMap.constructAtHead(std::forward<Args>(args)...));
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

  reference operator[](size_t index) { return m_indexMap.at(index); }
  const_reference operator[](size_t index) const { return m_indexMap.at(index); }
  reference at(size_t index) { return m_indexMap.at(index); }
  const_reference at(size_t index) const { return m_indexMap.at(index); }

  void i_printBlocks() const noexcept { m_indexMap.i_printBlocks(); }
  void i_printBlock(size_t i) const noexcept { m_indexMap.i_printBlock(i); }
  void i_printBlock(const std::vector<size_t>& indices) const noexcept { m_indexMap.i_printBlock(indices); }
  void i_printInfo() const noexcept { m_indexMap.i_printInfo(); }

  [[nodiscard]] bool isEmpty() noexcept { return getSize() == 0; }
  [[nodiscard]] size_t getSize() noexcept { return m_indexMap.getElementSize(); }

  Iterator begin() noexcept { return m_indexMap.begin(); }
  ConstIterator begin() const noexcept { return m_indexMap.begin(); }
  ConstIterator cbegin() const noexcept { return m_indexMap.cbegin(); }

  Iterator end() noexcept { return m_indexMap.end(); }
  ConstIterator end() const noexcept { return m_indexMap.end(); }
  ConstIterator cend() const noexcept { return m_indexMap.cend(); }
};
} // namespace queue
