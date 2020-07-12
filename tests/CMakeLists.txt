include(FetchContent)

# 'Catch2' testing framework.
message(STATUS "Fetching dependency: catch2")
FetchContent_Declare(
  catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG v2.12.3
  GIT_SHALLOW TRUE)
FetchContent_MakeAvailable(catch2)
list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/contrib)
include(Catch)

# 'tria_tests' executable.
message(STATUS "Configuring tria_tests executable")
add_executable(tria_tests
  tria/log/level_test.cpp
  tria/log/logger_test.cpp
  tria/log/param_test.cpp

  tria/pal/utils_test.cpp

  tria/main.cpp)
target_compile_features(tria_tests PUBLIC cxx_std_17)
target_link_libraries(tria_tests PRIVATE Catch2::Catch2)
target_link_libraries(tria_tests PRIVATE tria_log)

# Register tests to CTest.
message(STATUS "Registering tests to CTest")
catch_discover_tests(tria_tests)
