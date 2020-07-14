#pragma once
#include "tria/fs.hpp"
#include <exception>
#include <string>
#include <string_view>

namespace tria::asset::err {

/*
 * Exception that is thrown when there is an error loading an asset.
 */
class AssetLoadErr final : public std::exception {
public:
  AssetLoadErr() = delete;
  AssetLoadErr(const fs::path& path, std::string_view msg) :
      m_path{path},
      m_innerMsg{msg},
      m_msg{std::string{"Asset load error: "} + std::string{msg} + ": " + m_path.string()} {}

  [[nodiscard]] auto what() const noexcept -> const char* override { return m_msg.c_str(); }

  [[nodiscard]] auto getInnerMsg() const noexcept -> const std::string_view { return m_innerMsg; }

  [[nodiscard]] auto getPath() const noexcept -> const fs::path& { return m_path; }

private:
  fs::path m_path;
  std::string m_innerMsg;
  std::string m_msg;
};

} // namespace tria::asset::err
