# Utility function to compile a set of shaders and copy them to output dir.
function(configure_shaders targetName shaderSrcDir)
  message(STATUS "Configuring shaders: ${targetName}")

  find_program(glslcPath glslc)
  if(NOT glslcPath)
    message(FATAL_ERROR "glslc not found")
  endif()
  message(STATUS "Found glslc: ${glslcPath}")

  set(shaderDstDir ${EXECUTABLE_OUTPUT_PATH}/${targetName})

  foreach(file IN LISTS ARGN)
    set(fileSrc ${shaderSrcDir}/${file})
    set(fileDst ${shaderDstDir}/${file}.spv)

    # Create output directory.
    get_filename_component(parentDir ${fileDst} DIRECTORY)
    file(MAKE_DIRECTORY ${parentDir})

    # Create a custom command to compile each shader to spir-v using glslc.
    add_custom_command(
      OUTPUT ${fileDst}
      DEPENDS ${fileSrc}
      COMMAND ${glslcPath} -O -o ${fileDst} ${fileSrc})

    # Keep a list of all output files.
    list(APPEND shaderArtifacts ${fileDst})
  endforeach()

  # Register the target that depends on all the shader outputs.
  add_custom_target(${targetName} DEPENDS ${shaderArtifacts})
endfunction(configure_shaders)

# Utility function to copy a set of data directories to the output.
function(configure_data targetName dataSrcDir)
  message(STATUS "Configuring data-directories: ${targetName}")

  # Register commands to copy all data directories to the output.
  foreach(dataDir IN LISTS ARGN)
    set(dirSrc ${dataSrcDir}/${dataDir})
    set(dirDst ${EXECUTABLE_OUTPUT_PATH}/${targetName}/${dataDir})

    add_custom_command(
      TARGET ${targetName} POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E copy_directory ${dirSrc} ${dirDst})
  endforeach()
endfunction(configure_data)

# Sandbox.
message(STATUS "Configuring sandbox executable")
add_executable(sandbox
  sandbox/main.cpp)
target_compile_features(sandbox PUBLIC cxx_std_17)
target_link_libraries(sandbox PRIVATE tria_asset)
target_link_libraries(sandbox PRIVATE tria_pal)
target_link_libraries(sandbox PRIVATE tria_log)
target_link_libraries(sandbox PRIVATE tria_gfx)
configure_shaders(sandbox_data ${CMAKE_CURRENT_SOURCE_DIR}/sandbox_data
  shaders/obj.vert
  shaders/obj_nrm.frag
  shaders/obj_tex.frag
  shaders/sky.vert
  shaders/sky.frag)
configure_data(sandbox_data ${CMAKE_CURRENT_SOURCE_DIR}/sandbox_data
  graphics
  models
  textures)
add_dependencies(sandbox sandbox_data)
