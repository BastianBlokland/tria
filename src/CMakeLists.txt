# Asset (asset loading library).
message(STATUS "Configuring asset library")
add_library(tria_asset STATIC
  tria/asset/internal/graphic_loader.cpp
  tria/asset/internal/json.cpp
  tria/asset/internal/loader.cpp
  tria/asset/internal/mesh_obj_loader.cpp
  tria/asset/internal/raw_asset_loader.cpp
  tria/asset/internal/shader_loader.cpp
  tria/asset/internal/texture_ppm_loader.cpp
  tria/asset/internal/texture_tga_loader.cpp
  tria/asset/database.cpp
  tria/asset/database_impl.cpp)
target_compile_features(tria_asset PRIVATE cxx_std_17)
target_include_directories(tria_asset PUBLIC ${PROJECT_SOURCE_DIR}/include)
target_include_directories(tria_asset PRIVATE ${tinyobjloader_INCLUDE_DIRS})
target_link_libraries(tria_asset PRIVATE simdjson)
target_link_libraries(tria_asset PRIVATE tinyobjloader)
target_link_libraries(tria_asset PRIVATE tria_log)
target_link_libraries(tria_asset PRIVATE Threads::Threads)

# Gfx (graphics library).
message(STATUS "Configuring gfx vulkan library")
add_library(tria_gfx STATIC
  tria/gfx_vulkan/internal/buffer.cpp
  tria/gfx_vulkan/internal/debug_messenger.cpp
  tria/gfx_vulkan/internal/descriptor_manager.cpp
  tria/gfx_vulkan/internal/device.cpp
  tria/gfx_vulkan/internal/forward_technique.cpp
  tria/gfx_vulkan/internal/graphic.cpp
  tria/gfx_vulkan/internal/image.cpp
  tria/gfx_vulkan/internal/memory_pool.cpp
  tria/gfx_vulkan/internal/mesh.cpp
  tria/gfx_vulkan/internal/renderer.cpp
  tria/gfx_vulkan/internal/sampler.cpp
  tria/gfx_vulkan/internal/shader.cpp
  tria/gfx_vulkan/internal/stat_recorder.cpp
  tria/gfx_vulkan/internal/stopwatch.cpp
  tria/gfx_vulkan/internal/swapchain.cpp
  tria/gfx_vulkan/internal/texture.cpp
  tria/gfx_vulkan/internal/transferer.cpp
  tria/gfx_vulkan/internal/uniform_container.cpp
  tria/gfx_vulkan/internal/utils.cpp
  tria/gfx_vulkan/context.cpp
  tria/gfx_vulkan/native_context.cpp
  tria/gfx_vulkan/native_canvas.cpp
  tria/gfx_vulkan/canvas.cpp)

message(STATUS "Finding vulkan package")
find_package(Vulkan REQUIRED)
target_link_libraries(tria_gfx PRIVATE ${Vulkan_LIBRARIES})
target_include_directories(tria_gfx PRIVATE ${Vulkan_INCLUDE_DIRS})

# Set defines to tell vulkan what platform we are targetting.
if(${TRIA_PLATFORM} STREQUAL "linux_xcb")
  add_definitions(-DVK_USE_PLATFORM_XCB_KHR)
elseif(${TRIA_PLATFORM} STREQUAL "win32")
  add_definitions(-DVK_USE_PLATFORM_WIN32_KHR)
endif()

target_compile_features(tria_gfx PRIVATE cxx_std_17)
target_include_directories(tria_gfx PUBLIC ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(tria_gfx PRIVATE tria_log)
target_link_libraries(tria_gfx PRIVATE tria_math)
target_link_libraries(tria_gfx PRIVATE tria_pal)

# Log (logging library).
message(STATUS "Configuring log library")
add_library(tria_log STATIC
  tria/log/json_sink.cpp
  tria/log/logger.cpp
  tria/log/param.cpp
  tria/log/pretty_sink.cpp)
target_compile_features(tria_log PRIVATE cxx_std_17)
target_include_directories(tria_log PUBLIC ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(tria_log PRIVATE Threads::Threads)
target_link_libraries(tria_log PRIVATE tria_pal)

# Math (math utlities library).
message(STATUS "Configuring math library")
add_library(tria_math STATIC
  tria/math/base64.cpp
  tria/math/rnd.cpp
  tria/math/utils.cpp)
target_compile_features(tria_math PRIVATE cxx_std_17)
target_include_directories(tria_math PUBLIC ${PROJECT_SOURCE_DIR}/include)
target_include_directories(tria_math PRIVATE ${tinyobjloader_INCLUDE_DIRS})

# Pal (Platform abstraction layer).
if(${TRIA_PLATFORM} STREQUAL "linux_xcb")

message(STATUS "Configuring linux xcb pal library")
  add_library(tria_pal STATIC
    tria/pal/interrupt.linux.cpp
    tria/pal/native_platform.xcb.cpp
    tria/pal/platform.xcb.cpp
    tria/pal/utils.linux.cpp
    tria/pal/window.xcb.cpp)

    message(STATUS "Finding xcb (with xkb) package")
    find_package(xcb REQUIRED)
    find_package(xcb-xkb REQUIRED)
    target_link_libraries(tria_pal PRIVATE xcb)
    target_link_libraries(tria_pal PRIVATE xcb-xkb)

elseif(${TRIA_PLATFORM} STREQUAL "win32")
  message(STATUS "Configuring win32 pal library")
  add_library(tria_pal STATIC
    tria/pal/interrupt.win32.cpp
    tria/pal/native_platform.win32.cpp
    tria/pal/platform.win32.cpp
    tria/pal/utils.win32.cpp
    tria/pal/window.win32.cpp)

endif()
target_compile_features(tria_pal PRIVATE cxx_std_17)
target_include_directories(tria_pal PUBLIC ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(tria_pal PRIVATE tria_log)
target_link_libraries(tria_pal PRIVATE Threads::Threads)

