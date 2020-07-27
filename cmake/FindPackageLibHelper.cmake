# Helper function to find a library using pkg-config.

find_package(PkgConfig)
include(FindPackageHandleStandardArgs)

function(findPackageLib libName headerName)
  message(STATUS "Finding package library, lib: ${libName}, header: ${headerName}")

  pkg_check_modules(PC_${libName} QUIET ${libName})

  find_path(${libName}_INCLUDE_DIR NAMES ${headerName}
    HINTS
    ${PC_${libName}_INCLUDEDIR}
    ${PC_${libName}_INCLUDE_DIRS})

  find_library(${libName}_LIBRARY NAMES ${libName}
    HINTS
    ${PC_${libName}_LIBDIR}
    ${PC_${libName}_LIBRARY_DIRS})

  find_package_handle_standard_args(${libName}
    FOUND_VAR ${libName}_FOUND
    REQUIRED_VARS ${libName}_INCLUDE_DIR ${libName}_LIBRARY)

  mark_as_advanced(${libName}_INCLUDE_DIR ${libName}_LIBRARY)

endfunction(findPackageLib)
