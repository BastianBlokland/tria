#include "native_context.hpp"
#include "internal/utils.hpp"
#include "tria/config.hpp"
#include "tria/gfx/err/driver_err.hpp"
#include "tria/pal/utils.hpp"
#include "tria/pal/window.hpp"
#include <algorithm>
#include <cstring>

namespace tria::gfx {

using namespace internal;

namespace {

constexpr std::array<const char*, 1> validationLayers = {"VK_LAYER_KHRONOS_validation"};

[[maybe_unused]] auto checkValidationLayersSupport(const std::vector<VkLayerProperties>& available)
    -> bool {
  // Check if all validation layers are present in the list of available layers.
  for (const auto* required : validationLayers) {
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
  appInfo.apiVersion    = VK_API_VERSION_1_2;
  return appInfo;
}

auto getVkAvailableInstanceExtensions() -> std::vector<VkExtensionProperties> {
  uint32_t extCount = 0;
  checkVkResult(vkEnumerateInstanceExtensionProperties(nullptr, &extCount, nullptr));
  auto result = std::vector<VkExtensionProperties>{extCount};
  checkVkResult(vkEnumerateInstanceExtensionProperties(nullptr, &extCount, result.data()));
  return result;
}

auto getVkAvailableInstanceLayers() -> std::vector<VkLayerProperties> {
  uint32_t layerCount = 0;
  checkVkResult(vkEnumerateInstanceLayerProperties(&layerCount, nullptr));
  auto result = std::vector<VkLayerProperties>{layerCount};
  checkVkResult(vkEnumerateInstanceLayerProperties(&layerCount, result.data()));
  return result;
}

auto getVkRequiredInstanceExtensions(bool enableValidation) -> std::vector<const char*> {
  auto result = std::vector<const char*>{VK_KHR_SURFACE_EXTENSION_NAME};
  switch (pal::getWindowSurfaceType()) {
  case pal::WindowSurfaceType::Xcb:
    result.push_back("VK_KHR_xcb_surface");
    break;
  case pal::WindowSurfaceType::Win32:
    result.push_back("VK_KHR_win32_surface");
    break;
  default:
    throw err::DriverErr{"Unsupported platform window surface type"};
  }
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
    createInfo.enabledLayerCount   = validationLayers.size();
    createInfo.ppEnabledLayerNames = validationLayers.data();
  }

  VkInstance result;
  checkVkResult(vkCreateInstance(&createInfo, nullptr, &result));
  return result;
}

} // namespace

NativeContext::NativeContext(log::Logger* logger) :
    m_logger{logger}, m_appName{pal::getCurExecutableName()} {

  auto availableInstExts   = getVkAvailableInstanceExtensions();
  auto availableInstLayers = getVkAvailableInstanceLayers();

  LOG_D(
      m_logger,
      "Vulkan init",
      {"availableInstExts",
       collectionToStr(availableInstExts, [](const auto& elem) { return elem.extensionName; })},
      {"availableInstLayers",
       collectionToStr(availableInstLayers, [](const auto& elem) { return elem.layerName; })});

#if defined(NDEBUG)
  auto enableValidationlayers = false;
#else
  auto enableValidationlayers = checkValidationLayersSupport(availableInstLayers);
#endif

  m_vkInstance = makeVkInstance(m_appName, enableValidationlayers);
  LOG_I(m_logger, "Vulkan instance created", {"validation", enableValidationlayers});

  if (enableValidationlayers) {
    m_dbgMessenger = std::make_unique<DebugMessenger>(m_vkInstance, m_logger);
  }

  m_device = getDevice(m_logger, m_vkInstance);
  if (!m_device) {
    throw err::DriverErr{"No device found with vulkan support"};
  }
}

NativeContext::~NativeContext() {
  // Destroy created resources.
  m_device       = nullptr;
  m_dbgMessenger = nullptr;

  // Destroy the vulkan instance.
  vkDestroyInstance(m_vkInstance, nullptr);
}

} // namespace tria::gfx
