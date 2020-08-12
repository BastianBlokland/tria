#pragma once
#include "image.hpp"
#include "tria/gfx/context.hpp"
#include "tria/log/api.hpp"
#include <optional>
#include <vector>
#include <vulkan/vulkan.h>

namespace tria::gfx::internal {

class Device;

using SwapchainIdx     = uint32_t;
using SwapchainSize    = math::Vec<uint16_t, 2>;
using SwapchainVersion = uint32_t;

/*
 * Swapchain is responsible for managing the images that are presented to the surface (window).
 */
class Swapchain final {
public:
  Swapchain(log::Logger* logger, const Device* device, VSyncMode vSync);
  Swapchain(const Swapchain& rhs)     = delete;
  Swapchain(Swapchain&& rhs) noexcept = delete;
  ~Swapchain();

  auto operator=(const Swapchain& rhs) -> Swapchain& = delete;
  auto operator=(Swapchain&& rhs) noexcept -> Swapchain& = delete;

  [[nodiscard]] auto getImageSize() const noexcept { return m_size; }
  [[nodiscard]] auto getImageCount() const noexcept { return m_images.size(); }
  [[nodiscard]] auto getImage(SwapchainIdx idx) const -> const Image&;

  /* Number that is increment every time the swapchain is recreated, can be used to check if we
   * should recreate resources that are tied to the swapchain.
   */
  [[nodiscard]] auto getVersion() const noexcept { return m_version; }

  /* Acquire a new image to render into.
   * Returns: an index that can be used to retrieve the swapchain image.
   * Note: When function returns the image might not be ready yet for rendering into, but all cpu
   * side recording can already be done and the 'imgAvailable' sempahore will fire once the image is
   * ready.
   * Note: if we fail to acquire an image null-optional is returned (can happen when window
   * is minimized for example).
   */
  [[nodiscard]] auto acquireImage(VkSemaphore imgAvailable, bool forceReinit)
      -> std::optional<SwapchainIdx>;

  /* Present a rendered image to the surface (window).
   * Note: actual presenting will only occur when 'imgReady' semaphore is fired, so this can be
   * called before the rendering is done.
   */
  auto presentImage(VkSemaphore imgReady, SwapchainIdx imageIndex) -> bool;

private:
  log::Logger* m_logger;
  const Device* m_device;
  math::Vec<uint16_t, 2> m_size;
  VSyncMode m_vSync;
  VkSwapchainKHR m_vkSwapchain;
  std::vector<Image> m_images;
  bool m_swapchainOutOfDate;
  SwapchainVersion m_version;

  auto initSwapchain() -> bool;
};

using SwapchainUnique = std::unique_ptr<Swapchain>;

} // namespace tria::gfx::internal
