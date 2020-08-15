# 'tria_tests' executable.
message(STATUS "Configuring tria_tests executable")
add_executable(tria_tests
  tria/asset/database_test.cpp
  tria/asset/graphic_test.cpp
  tria/asset/texture_ppm_test.cpp
  tria/asset/texture_tga_test.cpp
  tria/asset/mesh_obj_test.cpp
  tria/asset/shader_test.cpp
  tria/asset/utils.cpp

  tria/log/level_test.cpp
  tria/log/logger_test.cpp
  tria/log/param_test.cpp

  tria/math/base64_test.cpp
  tria/math/mat_test.cpp
  tria/math/pod_vector_test.cpp
  tria/math/quat_test.cpp
  tria/math/rnd_test.cpp
  tria/math/utils_test.cpp
  tria/math/vec_test.cpp

  tria/pal/utils_test.cpp

  tria/main.cpp)
target_compile_features(tria_tests PUBLIC cxx_std_17)
target_link_libraries(tria_tests PRIVATE Catch2::Catch2)
target_link_libraries(tria_tests PRIVATE tria_asset)
target_link_libraries(tria_tests PRIVATE tria_log)
target_link_libraries(tria_tests PRIVATE tria_math)
target_link_libraries(tria_tests PRIVATE tria_pal)

# Register tests to CTest.
message(STATUS "Registering tests to CTest")
catch_discover_tests(tria_tests)
