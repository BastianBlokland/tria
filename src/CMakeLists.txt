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

target_compile_features(tria_gfx PRIVATE cxx_std_17)
if(MSVC)
  target_compile_options(tria_gfx PRIVATE /EHsc)
else()
  target_compile_options(tria_gfx PRIVATE -fexceptions)
endif()
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
if(MSVC)
  target_compile_options(tria_log PRIVATE /EHsc)
else()
  target_compile_options(tria_log PRIVATE -fexceptions)
endif()
target_include_directories(tria_log PUBLIC ${PROJECT_SOURCE_DIR}/include)
target_include_directories(tria_log PRIVATE log)
target_link_libraries(tria_log PRIVATE Threads::Threads)
target_link_libraries(tria_log PRIVATE tria_pal)

# Pal (Platform abstraction layer).
if(UNIX)
  message(STATUS "Configuring unix pal library")

  add_library(tria_pal STATIC
    tria/pal/native_platform.xcb.cpp
    tria/pal/platform.xcb.cpp
    tria/pal/utils.linux.cpp
    tria/pal/window.xcb.cpp)

  message(STATUS "Finding xcb package")
  find_package(xcb REQUIRED)
  target_link_libraries(tria_pal PRIVATE xcb)

elseif(WIN32)
  message(STATUS "Configuring win32 pal library")

  add_library(tria_pal STATIC
    tria/pal/native_platform.win32.cpp
    tria/pal/platform.win32.cpp
    tria/pal/utils.win32.cpp
    tria/pal/window.win32.cpp)

endif()
target_compile_features(tria_pal PRIVATE cxx_std_17)
if(MSVC)
  target_compile_options(tria_pal PRIVATE /EHsc)
else()
  target_compile_options(tria_pal PRIVATE -fexceptions)
endif()
target_include_directories(tria_pal PUBLIC ${PROJECT_SOURCE_DIR}/include)
target_include_directories(tria_pal PRIVATE pal)
target_include_directories(tria_pal PRIVATE tria_log)
target_link_libraries(tria_pal PRIVATE Threads::Threads)

