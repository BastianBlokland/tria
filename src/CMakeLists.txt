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
target_link_libraries(tria_log PRIVATE tria_pal)

# Pal (Platform abstraction layer).
if(UNIX)
  message(STATUS "Configuring unix pal library")

  add_library(tria_pal STATIC
    tria/pal/platform.linux.xcb.cpp
    tria/pal/utils.linux.cpp
    tria/pal/window.linux.xcb.cpp)

  message(STATUS "Finding xcb package")
  find_package(xcb REQUIRED)
  target_link_libraries(tria_pal PRIVATE xcb)

  message(STATUS "Finding threads package")
  set(CMAKE_THREAD_PREFER_PTHREAD On)
  find_package(Threads REQUIRED)
  target_link_libraries(tria_pal PRIVATE Threads::Threads)

elseif(WIN32)
  message(STATUS "Configuring win32 pal library")

  add_library(tria_pal STATIC
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
