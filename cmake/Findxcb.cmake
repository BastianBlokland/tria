# CMake utility to find the xcb (https://xcb.freedesktop.org/) library.
# Based on the Valve implementation that is floating around the internet.

find_package(PkgConfig)

set(libName "xcb")
set(headerName "xcb/xcb.h")

include(FindPackageHandleStandardArgs)
pkg_check_modules(PC_${libName} QUIET ${libName})

find_path(${libName}_INCLUDE_DIR NAMES ${headerName}
  HINTS
  ${PC_${libName}_INCLUDEDIR}
  ${PC_${libName}_INCLUDE_DIRS}
)

find_library(${libName}_LIBRARY NAMES ${libName}
  HINTS
  ${PC_${libName}_LIBDIR}
  ${PC_${libName}_LIBRARY_DIRS}
)

find_package_handle_standard_args(${libName}
  FOUND_VAR ${libName}_FOUND
  REQUIRED_VARS ${libName}_INCLUDE_DIR ${libName}_LIBRARY)

mark_as_advanced(${libName}_INCLUDE_DIR ${libName}_LIBRARY)

set(XCB_FOUND true)
set(XCB_INCLUDE_DIRS, ${libName}_INCLUDE_DIR)
set(XCB_LIBRARIES, ${libName}_LIBRARY)
