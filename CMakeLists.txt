cmake_minimum_required(VERSION 3.15)

project(
  Tria
  VERSION 0.1.0
  LANGUAGES CXX)

# Set path for custom cmake modules.
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")

# Custom options.
set(LINTING "Off" CACHE BOOL "Should source linting be enabled")

# Print some diagnostic information.
message(STATUS "Configuring Tria")
message(STATUS "* Host system: ${CMAKE_HOST_SYSTEM}")
message(STATUS "* Host processor: ${CMAKE_SYSTEM_PROCESSOR}")
message(STATUS "* CMake version: ${CMAKE_VERSION}")
message(STATUS "* Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "* Linting: ${LINTING}")
message(STATUS "* Source path: ${PROJECT_SOURCE_DIR}")
message(STATUS "* Build path: ${PROJECT_BINARY_DIR}")
message(STATUS "* Ouput path: ${PROJECT_SOURCE_DIR}/bin")
message(STATUS "* Compiler: ${CMAKE_CXX_COMPILER_ID}")
message(STATUS "* Generator: ${CMAKE_GENERATOR}")

# Detect platform.
if(UNIX)
  # At the moment on linux only the xcb x11 display library is supported.
  set(TRIA_PLATFORM "linux_xcb")
  add_definitions(-DTRIA_LINUX)
  add_definitions(-DTRIA_LINUX_XCB)
  message(STATUS "Detected linux xcb platform")
elseif(WIN32)
  set(TRIA_PLATFORM "win32")
  add_definitions(-DTRIA_WIN32)
  add_definitions(-DWINVER=0x0600 -D_WIN32_WINNT=0x0600) # Target windows 'vista'.
  message(STATUS "Detected win32 platform")
else()
  message(FATAL_ERROR "Unsupported platform")
endif()

# Remove bin directory if it already exists.
if(EXISTS ${PROJECT_SOURCE_DIR}/bin)
  file(REMOVE_RECURSE ${PROJECT_SOURCE_DIR}/bin)
endif()

# Output executables to bin dir.
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY $<1:${PROJECT_SOURCE_DIR}/bin>)

set(CXX_FLAGS_WIN32 "\
  -DNOMINMAX \
  -D_CRT_SECURE_NO_WARNINGS \
  -D_SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING")

# Global compiler flags.
if(MSVC)
  message(STATUS "Setting msvc compiler flags")
  set(CMAKE_CXX_FLAGS "/std:c++17 /fp:fast /WX /wd4530 /wd26451 /EHsc ${CXX_FLAGS_WIN32}")
else()
  message(STATUS "Setting unix compiler flags")
  set(CXX_FLAGS_SHARED "-std=c++17 -ffast-math -mf16c -Werror -Wall -Wextra -fno-strict-aliasing")
  if(${TRIA_PLATFORM} STREQUAL "win32")
    set(CXX_FLAGS_SHARED "${CXX_FLAGS_SHARED} ${CXX_FLAGS_WIN32} -Wno-deprecated-declarations")
  endif()
  set(CMAKE_CXX_FLAGS_DEBUG "-g -fno-omit-frame-pointer ${CXX_FLAGS_SHARED}")
  set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG ${CXX_FLAGS_SHARED}")
endif()

# Generate a 'compile_commands.json' for use by clangd.
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Instead of a 'includes_CXX.rsp' file just pass the includes,
# reason is that MinGW does not respect that file.
set(CMAKE_CXX_USE_RESPONSE_FILE_FOR_INCLUDES OFF)

# Replace config variables in the config header.
configure_file("include/tria/config.hpp.in" "${PROJECT_SOURCE_DIR}/include/tria/config.hpp")

# Set a define that is the length of the path to the root directory. Usefull to make absolute paths
# inside the project relative to the root in a cheap way.
string(LENGTH "${CMAKE_SOURCE_DIR}/" SRC_PATH_LENGTH)
add_definitions("-DSRC_PATH_LENGTH=${SRC_PATH_LENGTH}")

include(FetchContent)

# External: 'simdjson' json parsing library.
message(STATUS "Fetching dependency: simdjson")
FetchContent_Declare(
  simdjson
  GIT_REPOSITORY https://github.com/simdjson/simdjson.git
  GIT_TAG 501fed6c4fb5c9a5bf30e90c4299de3477119b56)
set(SIMDJSON_JUST_LIBRARY ON CACHE INTERNAL "")
set(SIMDJSON_BUILD_STATIC ON CACHE INTERNAL "")
set(SIMDJSON_COMPETITION OFF CACHE INTERNAL "")
FetchContent_MakeAvailable(simdjson)

if(BUILD_TESTING)
  # Enable 'CTest' testing.
  message(STATUS "Enable CTest dependency")
  include(CTest)
  enable_testing()

  # External: 'Catch2' testing framework.
  message(STATUS "Fetching dependency: catch2")
  FetchContent_Declare(
    catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG v2.12.3
    GIT_SHALLOW TRUE)
  FetchContent_MakeAvailable(catch2)
  list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/contrib)
  include(Catch)
endif()

# Add dependency to the threads package.
message(STATUS "Finding threads package")
set(CMAKE_THREAD_PREFER_PTHREAD On)
find_package(Threads REQUIRED)

# Older version of libstdc++ require linking against the std::filesystem library manually.
if((UNIX AND NOT APPLE) OR MINGW)
  message(STATUS "Add linking to stdc++fs (filesystem library)")
  link_libraries(stdc++fs)
endif()

if(LINTING)
  # Use the clang-tidy linter if installed.
  find_program(CLANG_TIDY_BIN NAMES "clang-tidy" "clang-tidy-9")
  if(CLANG_TIDY_BIN)
    message(STATUS "Enabling clang-tidy linter")
    set(CMAKE_CXX_CLANG_TIDY ${CLANG_TIDY_BIN})
  endif()
endif()

# Add child cmake files.
add_subdirectory(src)
add_subdirectory(apps)
if(BUILD_TESTING)
  add_subdirectory(tests)
endif()
