# Sandbox.
message(STATUS "Configuring sandbox executable")
add_executable(sandbox
  sandbox/main.cpp)
target_compile_features(sandbox PUBLIC cxx_std_17)
target_link_libraries(sandbox PRIVATE tria_asset)
target_link_libraries(sandbox PRIVATE tria_pal)
target_link_libraries(sandbox PRIVATE tria_log)
target_link_libraries(sandbox PRIVATE tria_gfx)
add_dependencies(sandbox tria_data)
