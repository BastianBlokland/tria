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
  add_custom_target(tria_shaders DEPENDS ${shaderArtifacts})
endfunction(configure_shaders)

function(configure_plainfiles)
  set(srcDir ${CMAKE_CURRENT_SOURCE_DIR})
  set(dstDir ${EXECUTABLE_OUTPUT_PATH}/data)

  # Create output directory.
  file(MAKE_DIRECTORY ${dstDir})

  # Copy each file to the output directory.
  foreach(file IN LISTS ARGN)
    set(fileSrc ${srcDir}/${file})
    set(fileDst ${dstDir}/${file})

    configure_file(${fileSrc} ${fileDst} COPYONLY)

    # Keep a list of all ouput files.
    list(APPEND plainArtifacts ${fileDst})
  endforeach()

  # Add a custom target that depends on all plain files.
  add_custom_target(tria_plainfiles DEPENDS ${plainArtifacts})
endfunction(configure_plainfiles)

# Shaders.
message(STATUS "Configuring shaders")
configure_shaders(
  triangle.vert
  triangle.frag)

# Plain files
message(STATUS "Configuring plain files")
configure_plainfiles(
  triangle.gfx)

# Wrapper dependency that depends on all types of data.
add_custom_target(tria_data)
add_dependencies(tria_data tria_shaders tria_plainfiles)
