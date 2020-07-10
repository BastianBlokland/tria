#pragma once
#include "tria/log/sink.hpp"
#include <memory>

namespace tria::log {

/*
 * Logger is responsable for receiving messages to log and forwarding them to its sinks.
 * Publishing log messages is thread-safe and can be performed in parallel.
 * The logger uses a dedicated thread to process log messages and to invoke the sinks, because of
 * this the sinks themselves do not need to be threadsafe.
 */
class Logger final {
  class Impl;

public:
  Logger() = delete;

  template <typename... Sinks>
  explicit Logger(Sinks&&... sinks) : Logger(makeSinkVector(std::forward<Sinks>(sinks)...)) {}

  explicit Logger(std::vector<SinkUnique> sinks);

  Logger(const Logger& rhs)     = delete;
  Logger(Logger&& rhs) noexcept = default;
  ~Logger();

  auto operator=(const Logger& rhs) -> Logger& = delete;
  auto operator=(Logger&& rhs) noexcept -> Logger& = default;

  /* Publish a new log message.
   * Is thread-safe.
   */
  auto publish(Message msg) noexcept -> void;

private:
  std::unique_ptr<Impl> m_impl;
};

} // namespace tria::log
