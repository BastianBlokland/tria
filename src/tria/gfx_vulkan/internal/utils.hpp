#pragma once
#include "tria/gfx/err/driver_err.hpp"
#include <string>
#include <vulkan/vulkan.h>

namespace tria::gfx::internal {

auto getVkErrStr(VkResult errCode) noexcept -> std::string;

inline auto checkVkResult(VkResult result) -> void {
  if (result != VK_SUCCESS) {
    throw err::DriverErr{getVkErrStr(result)};
  }
}

template <typename CollectionType, typename SelectFunc>
auto collectionToStr(const CollectionType& col, SelectFunc func) -> std::string {
  auto result = std::string{};
  for (auto itr = col.begin(); itr != col.end(); ++itr) {
    const auto first = itr == col.begin();
    if (!first) {
      result.append(", ");
    }
    result.append(func(*itr));
  }
  return result;
}

} // namespace tria::gfx::internal
