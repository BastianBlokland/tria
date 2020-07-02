#pragma once
#include "tria/log/param.hpp"
#include <cassert>
#include <chrono>
#include <vector>

namespace tria::log {

class MetaData;

/*
 * Log message. Consists of three parts:
 * - Metadata (constant data that can be stored statically at the construction site).
 * - Timestamp (automatically collected in the constructor).
 * - Parameters (runtime parameters to include with the message).
 * Note: At this time the parameters are a heap allocation on every log with parameters, in the
 * future we could considering using some form of a thread-static ring buffer. Or alternatively some
 * small vector optimization.
 */
class Message final {
public:
  Message() noexcept = default;
  Message(const MetaData* meta, std::vector<Param> params) noexcept :
      m_meta{meta}, m_time{std::chrono::system_clock::now()}, m_params{std::move(params)} {
    assert(meta);
  }
  Message(const Message& rhs)     = default;
  Message(Message&& rhs) noexcept = default;

  auto operator=(const Message& rhs) -> Message& = default;
  auto operator=(Message&& rhs) noexcept -> Message& = default;

  [[nodiscard]] auto getMeta() const noexcept { return m_meta; }
  [[nodiscard]] auto getTime() const noexcept { return m_time; }
  [[nodiscard]] auto hasParams() const noexcept { return !m_params.empty(); }

  [[nodiscard]] auto begin() const noexcept { return m_params.begin(); }
  [[nodiscard]] auto end() const noexcept { return m_params.end(); }

private:
  const MetaData* m_meta;
  std::chrono::system_clock::time_point m_time;
  std::vector<Param> m_params;
};

} // namespace tria::log
