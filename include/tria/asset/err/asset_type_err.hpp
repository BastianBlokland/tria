#pragma once
#include <exception>
#include <string>
#include <string_view>

namespace tria::asset::err {

/*
 * Exception that is thrown when an unexpected asset type is encountered.
 */
class AssetTypeErr final : public std::exception {
public:
  AssetTypeErr() = delete;
  AssetTypeErr(std::string_view msg) : m_msg{std::string{"Asset type err: "} + std::string{msg}} {}

  [[nodiscard]] auto what() const noexcept -> const char* override { return m_msg.c_str(); }

private:
  std::string m_msg;
};

} // namespace tria::asset::err
