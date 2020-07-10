#pragma once
#include "tria/fs.hpp"
#include "tria/log/level.hpp"
#include "tria/log/message.hpp"
#include <memory>

namespace tria::log {

/*
 * Abstract base class for a logging sink.
 * Note: implementations do not need to be thread-safe as the logger guarantees to only call it from
 * a single thread, note that thread can (and probably will) not be the same as the construction and
 * destructing thread. But construction / destruction will not be done in parallel with writing.
 */
class Sink {
public:
  Sink()                    = delete;
  Sink(const Sink& rhs)     = delete;
  Sink(Sink&& rhs) noexcept = delete;
  virtual ~Sink()           = default;

  auto operator=(const Sink& rhs) -> Sink& = delete;
  auto operator=(Sink&& rhs) noexcept -> Sink& = delete;

  /* Mask to indicate what messages to receive.
   */
  [[nodiscard]] auto getMask() const noexcept { return m_mask; }

  /* Write a message to the sink.
   * Note: Does not need to be reentrant as will not be called in parallel.
   */
  virtual auto write(const Message& msg) noexcept -> void = 0;

protected:
  Sink(LevelMask mask) noexcept : m_mask{mask} {}

private:
  LevelMask m_mask;
};

using SinkUnique = std::unique_ptr<Sink>;

template <typename T>
auto sinkVectorPush(std::vector<T>&) noexcept -> void {}

template <typename T, typename FirstSink, typename... OtherSinks>
auto sinkVectorPush(std::vector<T>& v, FirstSink&& first, OtherSinks&&... args) noexcept -> void {
  v.push_back(std::forward<FirstSink>(first));
  sinkVectorPush(v, std::forward<OtherSinks>(args)...);
}

template <typename... Sinks>
[[nodiscard]] auto makeSinkVector(Sinks&&... sinks) noexcept -> std::vector<SinkUnique> {
  auto result = std::vector<SinkUnique>{};
  sinkVectorPush(result, std::forward<Sinks>(sinks)...);
  return result;
}

/* JsonSink
 * Log sink that outputs every log as a structured json object.
 * This is very usefull to view the logs in an external log viewer or query the log using simple
 * commandline tools.
 *
 * For example to print the severity and the message for each log:
 * $ cat app.log | jq '{ level: .level,  msg: .message }'
 *
 * Or print all errors and warnings:
 * $ cat app.log | jq 'select((.level == "error") or (.level == "warning"))'
 *
 * Something a more advanced like printing the 'width' and the 'height' param for every resize log:
 * $ cat app.log | jq 'select(.message == "Resized") | { w: .extra.width, h: .extra.height }'
 *
 * Or follow a 'live' log:
 * $ tail --follow app.log | jq '.message'
 *
 * Output format (without the newlines):
 * { "message": "Example",
 *   "level": "info",
 *   "timestamp": "2020-06-29T05:49:07.401231Z",
 *   "file": "/path/main.cpp",
 *   "func": "int main(int, char **)",
 *   "line": 16,
 *   "extra": { "val": 42 } }
 */

[[nodiscard]] auto makeConsoleJsonSink(LevelMask mask = allLevelMask()) -> SinkUnique;
[[nodiscard]] auto makeFileJsonSink(fs::path path, LevelMask mask = allLevelMask()) -> SinkUnique;

/* PrettySink
 * Log sink that outputs every log as a (styled) pretty printed line.
 * Mostly usefull for logging to the console.
 *
 * Output format (including newlines):
 * 2020-06-30T06:38:59.780823Z [info] Window openend
 *   width:  512
 *   height: 512
 */

[[nodiscard]] auto makeConsolePrettySink(LevelMask mask = allLevelMask(), bool styleOutput = true)
    -> SinkUnique;
[[nodiscard]] auto
makeFilePrettySink(fs::path path, LevelMask mask = allLevelMask(), bool styleOutput = false)
    -> SinkUnique;

} // namespace tria::log
