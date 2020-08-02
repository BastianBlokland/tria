#include "catch2/catch.hpp"
#include "tria/math/pod_vector.hpp"

namespace tria::math::tests {

struct TestPod {
  int val1;
  int val2;

  TestPod() noexcept = default;
  TestPod(int val1, int val2) : val1{val1}, val2{val2} {}
};

TEST_CASE("[math] - PodVector", "[math]") {

  SECTION("Default init vector has size 0") {
    auto vec = PodVector<char>{};
    CHECK(vec.empty());
    CHECK(vec.size() == 0U);
  }

  SECTION("Default init vector can be iterated") {
    auto vec = PodVector<char>{};
    auto sum = 0U;
    for (auto itr = vec.begin(); itr != vec.end(); ++itr) {
      sum += *itr;
    }
    CHECK(sum == 0U);
  }

  SECTION("Default init vector can be iterated using range for") {
    auto vec = PodVector<char>{};
    auto sum = 0U;
    for (const auto& c : vec) {
      sum += c;
    }
    CHECK(sum == 0U);
  }

  SECTION("Default init vector after push_back has size 1") {
    auto vec = PodVector<char>{};
    vec.push_back(42);
    CHECK(vec.size() == 1U);
  }

  SECTION("push_back places value at the back") {
    auto vec = PodVector<char>{};
    vec.push_back(42);
    CHECK(*vec.begin() == 42U);
    CHECK(vec[0] == 42U);
    CHECK(vec.front() == 42U);
    CHECK(vec.back() == 42U);
  }

  SECTION("emplace_back place value at the back") {
    auto vec = PodVector<TestPod>{};
    vec.emplace_back(42, 1337);
    CHECK(vec.begin()->val1 == 42U);
    CHECK(vec[0].val1 == 42U);
    CHECK(vec.front().val1 == 42U);
    CHECK(vec.back().val1 == 42U);
  }

  SECTION("Many values can be pushed back") {
    auto vec = PodVector<unsigned int>{};
    for (auto i = 0U; i != 1'000; ++i) {
      vec.push_back(i);
    }
    for (auto i = 0U; i != 1'000; ++i) {
      CHECK(vec[i] == i);
    }
  }

  SECTION("Clear resets size but not capacity") {
    auto vec = PodVector<int>{};
    vec.resize(512U);
    vec.clear();
    CHECK(vec.empty());
    CHECK(vec.size() == 0U);
    CHECK(vec.capacity() == 512U);
  }

  SECTION("Iterate after clear yields no values") {
    auto vec = PodVector<int>{};
    vec.push_back(42);
    vec.push_back(1337);
    vec.clear();

    auto sum = 0U;
    for (const auto& c : vec) {
      sum += c;
    }
    CHECK(sum == 0U);
  }

  SECTION("Reserve changes capacity") {
    auto vec = PodVector<unsigned int>{};
    vec.reserve(512U);
    CHECK(vec.capacity() == 512U);
    CHECK(vec.size() == 0U);
  }

  SECTION("Resize changes capacity and size") {
    auto vec = PodVector<unsigned int>{};
    vec.resize(512U);
    CHECK(vec.capacity() == 512U);
    CHECK(vec.size() == 512U);
  }

  SECTION("Reserve smaller then capacity does nothing") {
    auto vec = PodVector<unsigned int>{};
    vec.reserve(512U);
    vec.reserve(256U);
    CHECK(vec.capacity() == 512U);
  }

  SECTION("Reserve utilities a minimum size") {
    auto vec = PodVector<unsigned int>{};
    vec.reserve(1U);
    CHECK(vec.capacity() > 1U);
  }
}

} // namespace tria::math::tests
