module;

#include <cstdlib>
#include <memory_resource>
export module test;

export namespace test {
class DetailedTracker : public std::pmr::memory_resource {
private:
  std::pmr::memory_resource* m_upstream;
  size_t m_allocationCount = 0;
  size_t m_deallocationCount = 0;
  size_t m_bytesAllocated = 0;

  void* do_allocate(size_t bytes, size_t alignment) override {
    m_bytesAllocated += bytes;
    ++m_allocationCount;
    return m_upstream->allocate(bytes, alignment);
  }

  void do_deallocate(void* p, size_t bytes, size_t alignment) override {
    ++m_deallocationCount;
    m_upstream->deallocate(p, bytes, alignment);
  }

  [[nodiscard]] bool do_is_equal(const std::pmr::memory_resource& o) const noexcept override {
    return this == &o;
  }

public:
  explicit DetailedTracker(std::pmr::memory_resource* res) : m_upstream(res) {}
  [[nodiscard]] size_t allocationCount() const noexcept { return m_allocationCount; }
  [[nodiscard]] size_t deallocationCount() const noexcept { return m_deallocationCount; }
  [[nodiscard]] size_t bytesAllocated() const noexcept { return m_bytesAllocated; }
};

class FallbackTracker : public std::pmr::memory_resource {
private:
  size_t m_allocationCount = 0;

  void* do_allocate(size_t bytes, size_t alignment) override { // NOLINT
    m_allocationCount++;
    return std::malloc(bytes); // NOLINT
  }
  void do_deallocate(void* p, size_t, size_t) override { std::free(p); } // NOLINT
  [[nodiscard]] bool do_is_equal(const memory_resource& o) const noexcept override { return &o == this; }

public:
  [[nodiscard]] size_t allocationCount() const noexcept { return m_allocationCount; }
};

class DefaultResourceGuard {
private:
  std::pmr::memory_resource* m_old;

public:
  explicit DefaultResourceGuard(std::pmr::memory_resource* next)
      : m_old(std::pmr::set_default_resource(next)) {}
  ~DefaultResourceGuard() noexcept { std::pmr::set_default_resource(m_old); }

  DefaultResourceGuard(const DefaultResourceGuard&) = delete;
  DefaultResourceGuard& operator=(const DefaultResourceGuard&) = delete;
  DefaultResourceGuard(DefaultResourceGuard&&) = delete;
  DefaultResourceGuard& operator=(DefaultResourceGuard&&) = delete;
};

} // namespace test
