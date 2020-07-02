#include "internal/file_sink.hpp"
#include "internal/str_write.hpp"
#include "tria/log/metadata.hpp"
#include "tria/log/sink.hpp"
#include <algorithm>
#include <stdexcept>

namespace tria::log {

namespace {

/*
 * Ansi escape sequences (https://en.wikipedia.org/wiki/ANSI_escape_code).
 */

constexpr auto ansiReset() noexcept { return "\x1B[0m"; }
constexpr auto ansiBold() noexcept { return "\x1B[1m"; }
constexpr auto ansiFgBlackColor() noexcept { return "\x1B[30m"; }
constexpr auto ansiFgWhiteColor() noexcept { return "\x1B[37m"; }
constexpr auto ansiBgRedColor() noexcept { return "\x1B[41m"; }
constexpr auto ansiBgGreenColor() noexcept { return "\x1B[42m"; }
constexpr auto ansiBgYellowColor() noexcept { return "\x1B[43m"; }
constexpr auto ansiBgBlueColor() noexcept { return "\x1B[44m"; }
constexpr auto ansiFgGrayColor() noexcept { return "\x1B[90m"; }

} // namespace

class PrettySink final : public internal::FileSink {
public:
  PrettySink(std::FILE* fileHandle, bool closeFile, LevelMask mask, bool styleOutput) :
      FileSink{fileHandle, closeFile, mask}, m_styleOutput{styleOutput} {
    constexpr auto startingBufferSize = 1024;
    m_buffer.reserve(startingBufferSize);
  }
  ~PrettySink() override = default;

  auto write(const Message& msg) noexcept -> void override {
    // Write time.
    appendStyle(ansiFgGrayColor());
    internal::writeIsoTime(&m_buffer, msg.getTime());
    m_buffer.append(" ");

    // Write level.
    switch (msg.getMeta()->getLevel()) {
    case Level::Debug:
      appendStyle(ansiFgBlackColor());
      appendStyle(ansiBgBlueColor());
      break;
    case Level::Info:
      appendStyle(ansiFgBlackColor());
      appendStyle(ansiBgGreenColor());
      break;
    case Level::Warn:
      appendStyle(ansiFgBlackColor());
      appendStyle(ansiBgYellowColor());
      break;
    case Level::Error:
      appendStyle(ansiFgWhiteColor());
      appendStyle(ansiBgRedColor());
      break;
    }
    m_buffer.append("[");
    internal::writeLvl(&m_buffer, msg.getMeta()->getLevel());
    m_buffer.append("]");
    appendStyle(ansiReset());

    // Write text.
    m_buffer.append(" ");
    m_buffer.append(msg.getMeta()->getTxt());
    m_buffer.append("\n");

    if (msg.hasParams()) {

      const auto maxKeySize =
          std::max_element(
              msg.begin(),
              msg.end(),
              [](const Param& a, const Param& b) { return a.getKey().size() < b.getKey().size(); })
              ->getKey()
              .size();

      // Write parameters.
      for (const auto& param : msg) {

        m_buffer.append("  ");
        m_buffer.append(param.getKey());
        m_buffer.append(": ");

        // Pad all the parameters to align to the longest parameter.
        // Note this assumes that each character is the same size, so input has to be ascii.
        const auto padding = maxKeySize - param.getKey().size();
        for (auto i = 0U; i < padding; ++i) {
          m_buffer.append(" ");
        }

        appendStyle(ansiBold());

        param.writeValue(&m_buffer);
        m_buffer.append("\n");

        appendStyle(ansiReset());
      }
    }

    writeBuffer();
  }

private:
  std::string m_buffer;
  bool m_styleOutput;

  auto appendStyle(std::string_view styleStr) noexcept -> void {
    if (m_styleOutput) {
      m_buffer.append(styleStr);
    }
  }

  auto writeBuffer() noexcept -> void {
    writeToFile(m_buffer.data(), m_buffer.size());
    m_buffer.clear();
  }
};

auto makeConsolePrettySink(LevelMask mask, bool styleOutput) -> SinkPtr {
  return internal::makeConsoleSink<PrettySink>(mask, styleOutput);
}

auto makeFilePrettySink(fs::path path, LevelMask mask, bool styleOutput) -> SinkPtr {
  return internal::makeFileSink<PrettySink>(std::move(path), mask, styleOutput);
}

} // namespace tria::log
