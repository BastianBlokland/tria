#pragma once
#include "tria/fs.hpp"
#include "tria/log/sink.hpp"

namespace tria::log::internal {

class FileSink : public Sink {
public:
  ~FileSink() override {
    // Flush all data to the underlying storage.
    std::fflush(m_fileHandle);

    if (m_closeFile) {
      std::fclose(m_fileHandle);
    }
  }

  auto write(const Message& msg) noexcept -> void override = 0;

protected:
  FileSink(std::FILE* fileHandle, bool closeFile, LevelMask mask) :
      Sink{mask}, m_fileHandle{fileHandle}, m_closeFile{closeFile} {
    if (!m_fileHandle) {
      throw std::invalid_argument{"Null file handle is not supported"};
    }
  }

  auto writeToFile(const char* data, size_t size) noexcept {
    assert(data);
    std::fwrite(data, size, 1, m_fileHandle);
  }

private:
  std::FILE* m_fileHandle;
  bool m_closeFile;
};

template <typename T, typename... Args>
[[nodiscard]] auto makeConsoleSink(LevelMask mask, Args&&... args) -> SinkPtr {
  return std::make_unique<T>(stdout, false, mask, std::forward<Args>(args)...);
}

template <typename T, typename... Args>
[[nodiscard]] auto makeFileSink(fs::path path, LevelMask mask, Args&&... args) -> SinkPtr {
#if defined(_WIN32)
  auto* file = _wfopen(path.c_str(), L"w");
#else
  auto* file = std::fopen(path.c_str(), "w");
#endif

  if (!file) {
    throw fs::filesystem_error{"Failed to create log file",
                               path.c_str(),
                               std::make_error_code(static_cast<std::errc>(errno))};
  }
  return std::make_unique<T>(file, true, mask, std::forward<Args>(args)...);
}

} // namespace tria::log::internal
