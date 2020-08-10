#pragma once
#include <string>

/* Debug utilities.
 * Turn into no-ops when compiling in non-debug mode.
 */

#if defined(NDEBUG)
#define DBG_PHYSDEVICE_NAME(DEVICE, OBJ, NAME)
#else
#define DBG_PHYSDEVICE_NAME(DEVICE, OBJ, NAME)                                                     \
  (DEVICE)->setDebugName(                                                                          \
      VK_OBJECT_TYPE_PHYSICAL_DEVICE,                                                              \
      reinterpret_cast<uint64_t>(OBJ),                                                             \
      NAME + std::string("_physicaldevice"))
#endif

#if defined(NDEBUG)
#define DBG_DEVICE_NAME(DEVICE, OBJ, NAME)
#else
#define DBG_DEVICE_NAME(DEVICE, OBJ, NAME)                                                         \
  (DEVICE)->setDebugName(                                                                          \
      VK_OBJECT_TYPE_DEVICE, reinterpret_cast<uint64_t>(OBJ), NAME + std::string("_device"))
#endif

#if defined(NDEBUG)
#define DBG_IMG_NAME(DEVICE, OBJ, NAME)
#else
#define DBG_IMG_NAME(DEVICE, OBJ, NAME)                                                            \
  (DEVICE)->setDebugName(                                                                          \
      VK_OBJECT_TYPE_IMAGE, reinterpret_cast<uint64_t>(OBJ), NAME + std::string("_img"))
#endif

#if defined(NDEBUG)
#define DBG_IMGVIEW_NAME(DEVICE, OBJ, NAME)
#else
#define DBG_IMGVIEW_NAME(DEVICE, OBJ, NAME)                                                        \
  (DEVICE)->setDebugName(                                                                          \
      VK_OBJECT_TYPE_IMAGE_VIEW, reinterpret_cast<uint64_t>(OBJ), NAME + std::string("_imgview"))
#endif

#if defined(NDEBUG)
#define DBG_SAMPLER_NAME(DEVICE, OBJ, NAME)
#else
#define DBG_SAMPLER_NAME(DEVICE, OBJ, NAME)                                                        \
  (DEVICE)->setDebugName(                                                                          \
      VK_OBJECT_TYPE_SAMPLER, reinterpret_cast<uint64_t>(OBJ), NAME + std::string("_sampler"))
#endif

#if defined(NDEBUG)
#define DBG_SHADER_NAME(DEVICE, OBJ, NAME)
#else
#define DBG_SHADER_NAME(DEVICE, OBJ, NAME)                                                         \
  (DEVICE)->setDebugName(                                                                          \
      VK_OBJECT_TYPE_SHADER_MODULE,                                                                \
      reinterpret_cast<uint64_t>(OBJ),                                                             \
      NAME + std::string("_shader"))
#endif

#if defined(NDEBUG)
#define DBG_DESCPOOL_NAME(DEVICE, OBJ, NAME)
#else
#define DBG_DESCPOOL_NAME(DEVICE, OBJ, NAME)                                                       \
  (DEVICE)->setDebugName(                                                                          \
      VK_OBJECT_TYPE_DESCRIPTOR_POOL,                                                              \
      reinterpret_cast<uint64_t>(OBJ),                                                             \
      NAME + std::string("_descpool"))
#endif

#if defined(NDEBUG)
#define DBG_DESCLAYOUT_NAME(DEVICE, OBJ, NAME)
#else
#define DBG_DESCLAYOUT_NAME(DEVICE, OBJ, NAME)                                                     \
  (DEVICE)->setDebugName(                                                                          \
      VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT,                                                        \
      reinterpret_cast<uint64_t>(OBJ),                                                             \
      NAME + std::string("_desclayout"))
#endif

#if defined(NDEBUG)
#define DBG_DESCSET_NAME(DEVICE, OBJ, NAME)
#else
#define DBG_DESCSET_NAME(DEVICE, OBJ, NAME)                                                        \
  (DEVICE)->setDebugName(                                                                          \
      VK_OBJECT_TYPE_DESCRIPTOR_SET,                                                               \
      reinterpret_cast<uint64_t>(OBJ),                                                             \
      NAME + std::string("_descset"))
#endif

#if defined(NDEBUG)
#define DBG_SWAPCHAIN_NAME(DEVICE, OBJ, NAME)
#else
#define DBG_SWAPCHAIN_NAME(DEVICE, OBJ, NAME)                                                      \
  (DEVICE)->setDebugName(                                                                          \
      VK_OBJECT_TYPE_SWAPCHAIN_KHR,                                                                \
      reinterpret_cast<uint64_t>(OBJ),                                                             \
      NAME + std::string("_swapchain"))
#endif

#if defined(NDEBUG)
#define DBG_FRAMEBUFFER_NAME(DEVICE, OBJ, NAME)
#else
#define DBG_FRAMEBUFFER_NAME(DEVICE, OBJ, NAME)                                                    \
  (DEVICE)->setDebugName(                                                                          \
      VK_OBJECT_TYPE_FRAMEBUFFER,                                                                  \
      reinterpret_cast<uint64_t>(OBJ),                                                             \
      NAME + std::string("_framebuffer"))
#endif

#if defined(NDEBUG)
#define DBG_BUFFER_NAME(DEVICE, OBJ, NAME)
#else
#define DBG_BUFFER_NAME(DEVICE, OBJ, NAME)                                                         \
  (DEVICE)->setDebugName(                                                                          \
      VK_OBJECT_TYPE_BUFFER, reinterpret_cast<uint64_t>(OBJ), NAME + std::string("_buffer"))
#endif

#if defined(NDEBUG)
#define DBG_MEM_NAME(DEVICE, OBJ, NAME)
#else
#define DBG_MEM_NAME(DEVICE, OBJ, NAME)                                                            \
  (DEVICE)->setDebugName(                                                                          \
      VK_OBJECT_TYPE_DEVICE_MEMORY,                                                                \
      reinterpret_cast<uint64_t>(OBJ),                                                             \
      NAME + std::string("_memory"))
#endif

#if defined(NDEBUG)
#define DBG_PIPELINE_NAME(DEVICE, OBJ, NAME)
#else
#define DBG_PIPELINE_NAME(DEVICE, OBJ, NAME)                                                       \
  (DEVICE)->setDebugName(                                                                          \
      VK_OBJECT_TYPE_PIPELINE, reinterpret_cast<uint64_t>(OBJ), NAME + std::string("_pipeline"))
#endif

#if defined(NDEBUG)
#define DBG_PIPELINELAYOUT_NAME(DEVICE, OBJ, NAME)
#else
#define DBG_PIPELINELAYOUT_NAME(DEVICE, OBJ, NAME)                                                 \
  (DEVICE)->setDebugName(                                                                          \
      VK_OBJECT_TYPE_PIPELINE_LAYOUT,                                                              \
      reinterpret_cast<uint64_t>(OBJ),                                                             \
      NAME + std::string("_pipelinelayout"))
#endif

#if defined(NDEBUG)
#define DBG_QUEUE_NAME(DEVICE, OBJ, NAME)
#else
#define DBG_QUEUE_NAME(DEVICE, OBJ, NAME)                                                          \
  (DEVICE)->setDebugName(                                                                          \
      VK_OBJECT_TYPE_QUEUE, reinterpret_cast<uint64_t>(OBJ), NAME + std::string("_queue"))
#endif

#if defined(NDEBUG)
#define DBG_COMMANDPOOL_NAME(DEVICE, OBJ, NAME)
#else
#define DBG_COMMANDPOOL_NAME(DEVICE, OBJ, NAME)                                                    \
  (DEVICE)->setDebugName(                                                                          \
      VK_OBJECT_TYPE_COMMAND_POOL,                                                                 \
      reinterpret_cast<uint64_t>(OBJ),                                                             \
      NAME + std::string("_commandpool"))
#endif

#if defined(NDEBUG)
#define DBG_COMMANDBUFFER_NAME(DEVICE, OBJ, NAME)
#else
#define DBG_COMMANDBUFFER_NAME(DEVICE, OBJ, NAME)                                                  \
  (DEVICE)->setDebugName(                                                                          \
      VK_OBJECT_TYPE_COMMAND_BUFFER,                                                               \
      reinterpret_cast<uint64_t>(OBJ),                                                             \
      NAME + std::string("_commandbuffer"))
#endif

#if defined(NDEBUG)
#define DBG_SEMPAHORE_NAME(DEVICE, OBJ, NAME)
#else
#define DBG_SEMPAHORE_NAME(DEVICE, OBJ, NAME)                                                      \
  (DEVICE)->setDebugName(                                                                          \
      VK_OBJECT_TYPE_SEMAPHORE, reinterpret_cast<uint64_t>(OBJ), NAME + std::string("_semaphore"))
#endif

#if defined(NDEBUG)
#define DBG_FENCE_NAME(DEVICE, OBJ, NAME)
#else
#define DBG_FENCE_NAME(DEVICE, OBJ, NAME)                                                          \
  (DEVICE)->setDebugName(                                                                          \
      VK_OBJECT_TYPE_FENCE, reinterpret_cast<uint64_t>(OBJ), NAME + std::string("_fence"))
#endif

#if defined(NDEBUG)
#define DBG_QUERYPOOL_NAME(DEVICE, OBJ, NAME)
#else
#define DBG_QUERYPOOL_NAME(DEVICE, OBJ, NAME)                                                      \
  (DEVICE)->setDebugName(                                                                          \
      VK_OBJECT_TYPE_QUERY_POOL,                                                                   \
      reinterpret_cast<uint64_t>(OBJ),                                                             \
      NAME + std::string("_querypool"))
#endif
