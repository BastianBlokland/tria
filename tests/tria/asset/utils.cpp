#include "utils.hpp"
#include "tria/fs.hpp"
#include <fstream>

#if defined(__MINGW32__)
#include <windows.h>
#endif

namespace tria::asset::tests {

auto writeFile(const fs::path& path, std::string data) -> void {
  auto file = std::ofstream{path.string(), std::ios::out | std::ios::binary};
  file.write(data.data(), data.size());
  file.close();
}

auto writeFile(const fs::path& path, math::RawData data) -> void {
  auto file = std::ofstream{path.string(), std::ios::out | std::ios::binary};
  file.write(reinterpret_cast<const char*>(data.data()), data.size());
  file.close();
}

auto deleteDir(const fs::path& path) -> void {
#if defined(__MINGW32__)
  // At the time of writing the remove_all that ships with MinGW fails when deleting a directory
  // with content. It actually succeeds to delete the nested content but then fails to delete the
  // directory itself. As a workaround we delete the directory with the win32 'RemoveDirectory'.
  try {
    fs::remove_all(path);
  } catch (...) {
    if (!RemoveDirectoryW(path.wstring().data())) {
      throw;
    }
  }
#else
  fs::remove_all(path);
#endif
}

} // namespace tria::asset::tests
