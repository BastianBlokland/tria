#include "native_context.hpp"
#include "internal/utils.hpp"
#include "native_canvas.hpp"
#include "tria/config.hpp"
#include "tria/pal/utils.hpp"
#include "tria/pal/window.hpp"
#include <algorithm>
#include <array>
#include <cstring>
#include <exception>

namespace tria::gfx {

using namespace internal;

namespace {

constexpr std::array<const char*, 1> g_validationLayers = {
    "VK_LAYER_KHRONOS_validation",
};

PFN_vkSetDebugUtilsObjectNameEXT g_funcSetDebugUtilsObjectName = nullptr;
PFN_vkCmdBeginDebugUtilsLabelEXT g_funcCmdBeginDebugUtilsLabel = nullptr;
PFN_vkCmdEndDebugUtilsLabelEXT g_funcCmdEndDebugUtilsLabel     = nullptr;

[[maybe_unused]] auto initDebugUtilsExtention(VkInstance vkInstance) -> void {
  // Load debug extension function pointers.
  g_funcSetDebugUtilsObjectName = reinterpret_cast<PFN_vkSetDebugUtilsObjectNameEXT>(
      vkGetInstanceProcAddr(vkInstance, "vkSetDebugUtilsObjectNameEXT"));
  g_funcCmdBeginDebugUtilsLabel = reinterpret_cast<PFN_vkCmdBeginDebugUtilsLabelEXT>(
      vkGetInstanceProcAddr(vkInstance, "vkCmdBeginDebugUtilsLabelEXT"));
  g_funcCmdEndDebugUtilsLabel = reinterpret_cast<PFN_vkCmdEndDebugUtilsLabelEXT>(
      vkGetInstanceProcAddr(vkInstance, "vkCmdEndDebugUtilsLabelEXT"));
}

[[maybe_unused]] auto checkValidationLayersSupport(const std::vector<VkLayerProperties>& available)
    -> bool {
  // Check if all validation layers are present in the list of available layers.
  for (const auto* required : g_validationLayers) {
    if (!std::any_of(available.begin(), available.end(), [required](const VkLayerProperties& l) {
          return std::strcmp(required, l.layerName) == 0;
        })) {
      return false;
    }
  }
  return true;
}

auto makeVkAppInfo(const std::string appName) noexcept -> VkApplicationInfo {
  VkApplicationInfo appInfo  = {};
  appInfo.sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO;
  appInfo.pApplicationName   = appName.c_str();
  appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
  appInfo.pEngineName        = ENGINE_NAME;
  appInfo.engineVersion = VK_MAKE_VERSION(ENGINE_VER_MAJOR, ENGINE_VER_MINOR, ENGINE_VER_PATCH);
  appInfo.apiVersion    = VK_API_VERSION_1_1;
  return appInfo;
}

auto getVkRequiredInstanceExtensions(bool enableValidation) noexcept -> std::vector<const char*> {
  auto result = std::vector<const char*>{VK_KHR_SURFACE_EXTENSION_NAME};
#if defined(TRIA_LINUX_XCB)
  result.push_back(VK_KHR_XCB_SURFACE_EXTENSION_NAME);
#elif defined(TRIA_WIN32)
  result.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
#else
  static_assert("Unsupported platform");
#endif
  if (enableValidation) {
    result.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
  }
  return result;
}

auto makeVkInstance(const std::string appName, bool enableValidation) -> VkInstance {
  auto appInfo         = makeVkAppInfo(appName);
  auto reqInstanceExts = getVkRequiredInstanceExtensions(enableValidation);

  VkInstanceCreateInfo createInfo    = {};
  createInfo.sType                   = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  createInfo.pApplicationInfo        = &appInfo;
  createInfo.enabledExtensionCount   = reqInstanceExts.size();
  createInfo.ppEnabledExtensionNames = reqInstanceExts.data();
  if (enableValidation) {
    createInfo.enabledLayerCount   = g_validationLayers.size();
    createInfo.ppEnabledLayerNames = g_validationLayers.data();
  }

  VkInstance result;
  checkVkResult(vkCreateInstance(&createInfo, nullptr, &result));
  return result;
}

} // namespace

NativeContext::NativeContext(log::Logger* logger) :
    m_logger{logger}, m_appName{pal::getCurExecutableName()} {

  const auto availableInstExts   = getVkAvailableInstanceExtensions();
  const auto availableInstLayers = getVkAvailableInstanceLayers();

#if defined(NDEBUG)
  auto enableValidationlayers = false;
#else
  auto enableValidationlayers = checkValidationLayersSupport(availableInstLayers);
#endif

  m_vkInstance = makeVkInstance(m_appName, enableValidationlayers);
  LOG_I(m_logger, "Vulkan instance created", {"validation", enableValidationlayers});

  if (enableValidationlayers) {
    initDebugUtilsExtention(m_vkInstance);
    m_dbgMessenger = std::make_unique<DebugMessenger>(m_logger, m_vkInstance, false);
  }
}

NativeContext::~NativeContext() {
  // Destroy created resources.
  m_dbgMessenger = nullptr;

  // TODO(bastian): 'vkDestroyInstance' on some drivers can block for a very long time when trying
  // to destroy an instance that's in an invalid state. At the moment the workaround is just to not
  // even try to destroy the instance when we have an uncaught exception. However this can lead to
  // resource leaks that could be prevented. Its not a very high priority issue as most likely the
  // app will just try to gracefully exit and not try to keep running.
  if (!std::uncaught_exception()) {

    vkDestroyInstance(m_vkInstance, nullptr);

    LOG_I(m_logger, "Vulkan instance destroyed");
  } else {
    LOG_E(m_logger, "Failed to cleanup vulkan context");
  }
}

auto NativeContext::createCanvas(
    const pal::Window* window,
    VSyncMode vSync,
    SampleCount samples,
    DepthMode depth,
    ClearMask clear) -> std::unique_ptr<NativeCanvas> {
  return std::make_unique<NativeCanvas>(m_logger, this, window, vSync, samples, depth, clear);
}

auto NativeContext::setDebugName(
    VkDevice vkDevice, VkObjectType vkType, uint64_t vkHandle, std::string_view name) const noexcept
    -> void {
  if (g_funcSetDebugUtilsObjectName) {
    VkDebugUtilsObjectNameInfoEXT nameInfo = {};
    nameInfo.sType                         = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
    nameInfo.objectType                    = vkType;
    nameInfo.objectHandle                  = vkHandle;
    nameInfo.pObjectName                   = name.data();
    g_funcSetDebugUtilsObjectName(vkDevice, &nameInfo);
  }
}

auto NativeContext::beginDebugLabel(
    VkCommandBuffer vkCmdBuffer, std::string_view msg, math::Color color) const noexcept -> void {
  if (g_funcCmdBeginDebugUtilsLabel) {
    VkDebugUtilsLabelEXT label = {};
    label.sType                = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT;
    label.pLabelName           = msg.data();
    color.memcpy(label.color);
    g_funcCmdBeginDebugUtilsLabel(vkCmdBuffer, &label);
  }
}

auto NativeContext::endDebugLabel(VkCommandBuffer vkCmdBuffer) const noexcept -> void {
  if (g_funcCmdEndDebugUtilsLabel) {
    g_funcCmdEndDebugUtilsLabel(vkCmdBuffer);
  }
}

} // namespace tria::gfx
