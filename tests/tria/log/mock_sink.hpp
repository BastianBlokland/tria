#pragma once
#include "tria/log/level.hpp"
#include "tria/log/sink.hpp"
#include <stdexcept>
#include <vector>

namespace tria::log::tests {

class MockSink : public Sink {
public:
  MockSink(std::vector<Message>* output, LevelMask mask) : Sink{mask}, m_msgs{output} {
    if (!output) {
      throw std::invalid_argument{"Null output vector is not supported"};
    }
  }
  ~MockSink() override = default;

  auto write(const Message& msg) noexcept -> void override { m_msgs->push_back(msg); }

private:
  std::vector<Message>* m_msgs;
};

[[nodiscard]] auto makeMockSink(std::vector<Message>* output, LevelMask mask = allLevelMask())
    -> SinkUnique {
  return std::make_unique<MockSink>(output, mask);
}

} // namespace tria::log::tests
