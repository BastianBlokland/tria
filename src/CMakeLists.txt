# Gfx (graphics library).
message(STATUS "Configuring gfx vulkan library")
add_library(tria_gfx STATIC
  tria/gfx_vulkan/internal/debug_messenger.cpp
  tria/gfx_vulkan/internal/device.cpp
  tria/gfx_vulkan/internal/utils.cpp
  tria/gfx_vulkan/context.cpp
  tria/gfx_vulkan/native_context.cpp)

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
target_include_directories(tria_gfx PRIVATE log)
target_link_libraries(tria_gfx PRIVATE tria_pal)
target_link_libraries(tria_gfx PRIVATE tria_log)

# Log (logging library).
message(STATUS "Configuring log library")
add_library(tria_log STATIC
  tria/log/json_sink.cpp
  tria/log/logger.cpp
  tria/log/param.cpp
  tria/log/pretty_sink.cpp)
target_compile_features(tria_log PRIVATE cxx_std_17)
target_include_directories(tria_log PUBLIC ${PROJECT_SOURCE_DIR}/include)
target_include_directories(tria_log PRIVATE log)
target_link_libraries(tria_log PRIVATE Threads::Threads)
target_link_libraries(tria_log PRIVATE tria_pal)

# Pal (Platform abstraction layer).
if(${TRIA_PLATFORM} STREQUAL "linux_xcb")

message(STATUS "Configuring linux xcb pal library")
  add_library(tria_pal STATIC
    tria/pal/native_platform.xcb.cpp
    tria/pal/platform.xcb.cpp
    tria/pal/utils.linux.cpp
    tria/pal/window.xcb.cpp)

    message(STATUS "Finding xcb package")
    find_package(xcb REQUIRED)
    target_link_libraries(tria_pal PRIVATE xcb)

elseif(${TRIA_PLATFORM} STREQUAL "win32")
  message(STATUS "Configuring win32 pal library")
  add_library(tria_pal STATIC
    tria/pal/native_platform.win32.cpp
    tria/pal/platform.win32.cpp
    tria/pal/utils.win32.cpp
    tria/pal/window.win32.cpp)

endif()
target_compile_features(tria_pal PRIVATE cxx_std_17)
target_include_directories(tria_pal PUBLIC ${PROJECT_SOURCE_DIR}/include)
target_include_directories(tria_pal PRIVATE pal)
target_include_directories(tria_pal PRIVATE tria_log)
target_link_libraries(tria_pal PRIVATE Threads::Threads)

