function(configure_shaders)
  find_program(glslcPath glslc)
  if(NOT glslcPath)
    message(FATAL_ERROR "glslc not found")
  endif()
  message(STATUS "Found glslc: ${glslcPath}")

  set(shaderSrcDir ${CMAKE_CURRENT_SOURCE_DIR}/shaders)
  set(shaderDstDir ${EXECUTABLE_OUTPUT_PATH}/data/shaders)

  # Create output directory.
  file(MAKE_DIRECTORY ${shaderDstDir})

  foreach(file IN LISTS ARGN)
    set(fileSrc ${shaderSrcDir}/${file})
    set(fileDst ${shaderDstDir}/${file}.spv)

    # Create a custom command to compile each shader to spir-v using glslc.
    add_custom_command(
      OUTPUT ${fileDst}
      DEPENDS ${fileSrc}
      COMMAND ${glslcPath} -O -o ${fileDst} ${fileSrc})

    # Keep a list of all ouput files.
    list(APPEND shaderArtifacts ${fileDst})
  endforeach()

  # Add a custom target that depends on all shaders files.
  # That way executables can easiliy depend on the shaders.
  add_custom_target(tria_shaders DEPENDS ${shaderArtifacts})
endfunction(configure_shaders)

# Shaders.
message(STATUS "Configuring shaders")
configure_shaders(
  triangle.vert
  triangle.frag)

# Wrapper dependency that depends on all types of data.
add_custom_target(tria_data)
add_dependencies(tria_data tria_shaders)
