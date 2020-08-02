#pragma once
#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <type_traits>
#include <utility>

namespace tria::math {

/*
 * Container for pod types.
 * Implements a sub-set of the std::vector interface.
 * Note: Does NOT zero-init values.
 */
template <typename T>
class PodVector final {
  static_assert(std::is_pod<T>::value, "Type has to be plain-old-data");

  constexpr static auto s_minCapacity = 8U;

public:
  using value_type      = T;
  using pointer         = T*;
  using const_pointer   = const T*;
  using iterator        = T*;
  using const_iterator  = const T*;
  using reference       = T&;
  using const_reference = const T&;
  using size_type       = size_t;
  using difference_type = size_t;

  PodVector() noexcept : m_size{}, m_capacity{}, m_data{nullptr} {}
  PodVector(size_t size) noexcept :
      m_size{size}, m_capacity{size}, m_data{static_cast<T*>(std::malloc(size * sizeof(T)))} {}
  PodVector(const PodVector& rhs) = delete;
  PodVector(PodVector&& rhs) noexcept {
    m_size     = rhs.m_size;
    m_capacity = rhs.m_capacity;
    m_data     = rhs.m_data;
    rhs.m_data = nullptr;
  }
  ~PodVector() { std::free(m_data); }

  auto operator=(const PodVector& rhs) -> PodVector& = delete;

  auto operator=(PodVector&& rhs) noexcept -> PodVector& {
    m_size     = rhs.m_size;
    m_capacity = rhs.m_capacity;
    m_data     = rhs.m_data;
    rhs.m_data = nullptr;
    return *this;
  }

  [[nodiscard]] auto operator[](size_t idx) noexcept -> T& { return m_data[idx]; }
  [[nodiscard]] auto operator[](size_t idx) const noexcept -> const T& { return m_data[idx]; }

  [[nodiscard]] auto data() noexcept -> T* { return m_data; }
  [[nodiscard]] auto data() const noexcept -> const T* { return m_data; }

  [[nodiscard]] auto begin() noexcept -> T* { return m_data; }
  [[nodiscard]] auto begin() const noexcept -> const T* { return m_data; }
  [[nodiscard]] auto cbegin() const noexcept -> const T* { return m_data; }

  [[nodiscard]] auto end() noexcept -> T* { return m_data + m_size; }
  [[nodiscard]] auto end() const noexcept -> const T* { return m_data + m_size; }
  [[nodiscard]] auto cend() const noexcept -> const T* { return m_data + m_size; }

  [[nodiscard]] auto front() noexcept -> T& { return *begin(); }
  [[nodiscard]] auto front() const noexcept -> const T& { return *begin(); }

  [[nodiscard]] auto back() noexcept -> T& { return *(end() - 1); }
  [[nodiscard]] auto back() const noexcept -> const T& { return *(end() - 1); }

  [[nodiscard]] auto empty() const noexcept { return m_size == 0U; }
  [[nodiscard]] auto size() const noexcept { return m_size; }
  [[nodiscard]] auto capacity() const noexcept { return m_capacity; }

  auto reserve(size_t capacity) noexcept {
    auto reqCapacity = capacity > s_minCapacity ? capacity : s_minCapacity;
    if (reqCapacity > m_capacity) {
      m_data     = static_cast<T*>(std::realloc(m_data, reqCapacity * sizeof(T)));
      m_capacity = reqCapacity;
    }
  }

  auto resize(size_t size) noexcept {
    reserve(size);
    m_size = size;
  }

  auto push_back(T data) noexcept {
    if (m_size == m_capacity) {
      reserve(m_size * 2U);
    }
    assert(m_size < m_capacity);
    m_data[m_size++] = data;
  }

  template <class... Args>
  auto emplace_back(Args... args) {
    if (m_size == m_capacity) {
      reserve(m_size * 2U);
    }
    assert(m_size < m_capacity);
    m_data[m_size++] = T(std::forward<Args>(args)...);
  }

  auto clear() { m_size = 0U; }

private:
  size_t m_size;
  size_t m_capacity;
  T* m_data;
};

using RawData = PodVector<char>;

} // namespace tria::math
