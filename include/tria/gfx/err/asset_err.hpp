#pragma once
#include "tria/fs.hpp"
#include <exception>
#include <string>

namespace tria::gfx::err {

/*
 * Exception that is thrown when there is an error loading an asset.
 */
class AssetErr final : public std::exception {
public:
  AssetErr() = delete;
  AssetErr(fs::path path, const std::string& msg) :
      m_path{std::move(path)}, m_msg{std::string{"Asset error: "} + msg + ": " + m_path.string()} {}

  [[nodiscard]] auto what() const noexcept -> const char* override { return m_msg.c_str(); }

  [[nodiscard]] auto getPath() const noexcept -> const fs::path& { return m_path; }

private:
  fs::path m_path;
  std::string m_msg;
};

} // namespace tria::gfx::err
