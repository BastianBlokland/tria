# Sandbox.
message(STATUS "Configuring sandbox executable")
add_executable(sandbox
  sandbox/main.cpp)
target_compile_features(sandbox PUBLIC cxx_std_17)
if(MSVC)
  target_compile_options(sandbox PUBLIC /EHsc)
else()
  target_compile_options(sandbox PUBLIC -fexceptions)
endif()
target_link_libraries(sandbox PRIVATE tria_pal)
target_link_libraries(sandbox PRIVATE tria_log)
target_link_libraries(sandbox PRIVATE tria_gfx)
