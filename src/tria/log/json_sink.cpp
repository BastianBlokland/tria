#include "internal/file_sink.hpp"
#include "internal/str_write.hpp"
#include "tria/log/metadata.hpp"

namespace tria::log {

class JsonSink final : public internal::FileSink {
public:
  JsonSink(std::FILE* fileHandle, bool closeFile, LevelMask mask) :
      FileSink{fileHandle, closeFile, mask} {
    constexpr auto startingBufferSize = 1024;
    m_buffer.reserve(startingBufferSize);
  }
  ~JsonSink() override = default;

  auto write(const Message& msg) noexcept -> void override {
    // Start the log object.
    m_buffer.append("{");

    // Text message.
    m_buffer.append(" \"message\": \"");
    m_buffer.append(msg.getMeta()->getTxt());
    m_buffer.append("\",");

    // Level.
    m_buffer.append(" \"level\": \"");
    m_buffer.append(getName(msg.getMeta()->getLevel()));
    m_buffer.append("\",");

    // Time.
    m_buffer.append(" \"timestamp\": \"");
    internal::writeIsoTime(&m_buffer, msg.getTime());
    m_buffer.append("\",");

    // File.
    m_buffer.append(" \"file\": \"");
    // TODO(bastian): We could probably normalize the file path at compile time.
    internal::writePathNormalized(&m_buffer, msg.getMeta()->getFile().data());
    m_buffer.append("\",");

    // Function.
    m_buffer.append(" \"func\": \"");
    m_buffer.append(msg.getMeta()->getFunc());
    m_buffer.append("\",");

    // Line.
    m_buffer.append(" \"line\": ");
    internal::writeInt(&m_buffer, msg.getMeta()->getLine());

    // Parameters.
    if (msg.hasParams()) {
      // Start the 'extra' object.
      m_buffer.append(", \"extra\": {");

      for (auto itr = msg.begin(); itr != msg.end(); ++itr) {
        m_buffer.append(" \"");
        m_buffer.append(itr->getKey());
        m_buffer.append("\": ");

        itr->writeValue(&m_buffer, ParamWriteMode::Json);

        auto isLast = itr == msg.end() - 1;
        if (!isLast) {
          m_buffer.append(",");
        }
      }

      // End the 'extra' object.
      m_buffer.append(" }");
    }

    // End the log object.
    m_buffer.append(" }\n");

    writeBuffer();
  }

private:
  std::string m_buffer;

  auto writeBuffer() noexcept -> void {
    writeToFile(m_buffer.data(), m_buffer.size());
    m_buffer.clear();
  }
};

auto makeConsoleJsonSink(LevelMask mask) -> SinkUnique {
  return internal::makeConsoleSink<JsonSink>(mask);
}

auto makeFileJsonSink(fs::path path, LevelMask mask) -> SinkUnique {
  return internal::makeFileSink<JsonSink>(std::move(path), mask);
}

} // namespace tria::log
