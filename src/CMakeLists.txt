# Pal (Platform abstraction layer).
if(UNIX)
  message(STATUS "Configuring unix pal library")

  add_library(pal STATIC
    pal/platform.linux.xcb.cpp
    pal/utils.linux.cpp
    pal/window.linux.xcb.cpp)

  message(STATUS "Finding xcb package")
  find_package(xcb REQUIRED)
  target_link_libraries(pal PRIVATE xcb)

  message(STATUS "Finding threads package")
  set(CMAKE_THREAD_PREFER_PTHREAD On)
  find_package(Threads REQUIRED)
  target_link_libraries(pal PRIVATE Threads::Threads)

elseif(WIN32)
  message(STATUS "Configuring win32 pal library")

  add_library(pal STATIC
    pal/platform.win32.cpp
    pal/window.win32.cpp)

endif()
target_compile_features(pal PRIVATE cxx_std_17)
if(MSVC)
  target_compile_options(pal PRIVATE /EHsc)
else()
  target_compile_options(pal PRIVATE -fexceptions)
endif()
target_include_directories(pal PUBLIC ${PROJECT_SOURCE_DIR}/include)
target_include_directories(pal PRIVATE pal)
