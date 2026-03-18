# modules.cmake - CMake helper macros for ipcamera project
# This project is part of the SDK and requires aarch64-ca53 cross-compilation

message(STATUS "IPCamera: Using SDK cross-compilation environment (aarch64-ca53)")

# Add SDK environment paths - these are set by envsetup.sh
if(DEFINED ENV{LIBRARY_DIR})
  include_directories(SYSTEM $ENV{LIBRARY_DIR}/include)
  include_directories(SYSTEM $ENV{LIBRARY_DIR}/external/__install/include)
  # Use link_directories only from output where libraries should be found
  # This prevents CMake from embedding absolute paths to build directories
  link_directories($ENV{LIBRARY_DIR}/output)
endif()

if(DEFINED ENV{EXTERNAL} AND DEFINED ENV{INSTALL_DIR})
  include_directories(SYSTEM $ENV{EXTERNAL}/$ENV{INSTALL_DIR}/include)
  link_directories($ENV{EXTERNAL}/$ENV{INSTALL_DIR}/lib)
endif()

if(DEFINED ENV{INCLUDE_DIR})
  include_directories(SYSTEM $ENV{INCLUDE_DIR})
endif()

# Helper macros for library and executable creation
macro(ipcam_add_library name)
  add_library(${name} ${ARGN})
  target_compile_features(${name} PUBLIC cxx_std_17)
  set_target_properties(${name} PROPERTIES POSITION_INDEPENDENT_CODE ON)
  target_include_directories(${name} PUBLIC 
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
  )
endmacro()

macro(ipcam_add_executable name)
  add_executable(${name} ${ARGN})
  target_compile_features(${name} PRIVATE cxx_std_17)
  target_include_directories(${name} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
endmacro()

# Find libraries in SDK paths
set(_SDK_LIB_HINTS)
if(DEFINED ENV{LIBRARY_DIR})
  list(APPEND _SDK_LIB_HINTS "$ENV{LIBRARY_DIR}" "$ENV{LIBRARY_DIR}/external/__install")
endif()
if(DEFINED ENV{EXTERNAL} AND DEFINED ENV{INSTALL_DIR})
  list(APPEND _SDK_LIB_HINTS "$ENV{EXTERNAL}/$ENV{INSTALL_DIR}")
endif()

# Find external libraries (using SDK paths only)
find_library(MINIUPNPC_LIBRARY NAMES miniupnpc HINTS ${_SDK_LIB_HINTS} PATH_SUFFIXES lib NO_DEFAULT_PATH)
find_path(MINIUPNPC_INCLUDE_DIR NAMES miniupnpc/miniupnpc.h HINTS ${_SDK_LIB_HINTS} PATH_SUFFIXES include NO_DEFAULT_PATH)

if(MINIUPNPC_LIBRARY AND MINIUPNPC_INCLUDE_DIR)
  set(IPCAM_MINIUPNPC_FOUND TRUE)
  message(STATUS "Found miniupnpc: ${MINIUPNPC_LIBRARY}")
endif()

# Find Live555 (static libraries with -fPIC for RTSP server)
set(LIVE555_INSTALL_DIR "${CMAKE_SOURCE_DIR}/../../lib/external/__install")
set(LIVE555_INCLUDE_DIR "${LIVE555_INSTALL_DIR}/include/live555")
set(LIVE555_LIB_DIR "${LIVE555_INSTALL_DIR}/lib")

if(EXISTS "${LIVE555_LIB_DIR}/libliveMedia.a" AND EXISTS "${LIVE555_INCLUDE_DIR}/liveMedia.hh")
  set(IPCAM_LIVE555_FOUND TRUE CACHE BOOL "Live555 found" FORCE)
  set(LIVE555_INCLUDE_DIR "${LIVE555_INCLUDE_DIR}" CACHE PATH "Live555 include directory" FORCE)
  set(LIVE555_LIBRARIES
    "${LIVE555_LIB_DIR}/libliveMedia.a"
    "${LIVE555_LIB_DIR}/libgroupsock.a"
    "${LIVE555_LIB_DIR}/libBasicUsageEnvironment.a"
    "${LIVE555_LIB_DIR}/libUsageEnvironment.a"
    CACHE STRING "Live555 libraries" FORCE
  )
  message(STATUS "Found Live555: ${LIVE555_LIB_DIR}")
else()
  set(IPCAM_LIVE555_FOUND FALSE)
  message(STATUS "Live555 not found - run: cd code/lib/external && make live555")
endif()

# Find CivetWeb (prefer SDK external libs, fallback to system)
find_library(CIVETWEB_LIBRARY NAMES civetweb-cpp libcivetweb-cpp civetweb libcivetweb HINTS ${_SDK_LIB_HINTS} PATH_SUFFIXES lib)
find_path(CIVETWEB_INCLUDE_DIR NAMES CivetServer.h civetweb.h HINTS ${_SDK_LIB_HINTS} PATH_SUFFIXES include)

if(CIVETWEB_LIBRARY AND CIVETWEB_INCLUDE_DIR)
  set(IPCAM_CIVETWEB_FOUND TRUE CACHE BOOL "CivetWeb found in SDK" FORCE)
  set(CIVETWEB_LIBRARY "${CIVETWEB_LIBRARY}" CACHE FILEPATH "CivetWeb library path" FORCE)
  set(CIVETWEB_INCLUDE_DIR "${CIVETWEB_INCLUDE_DIR}" CACHE PATH "CivetWeb include directory" FORCE)
  message(STATUS "Found CivetWeb: ${CIVETWEB_LIBRARY}")
endif()

# Find spdlog (prefer SDK external libs, fallback to system)
find_package(spdlog QUIET)
if(NOT spdlog_FOUND AND NOT TARGET spdlog::spdlog)
  find_path(SPDLOG_INCLUDE_DIR NAMES spdlog/spdlog.h HINTS ${_SDK_LIB_HINTS} PATH_SUFFIXES include)
  
  if(SPDLOG_INCLUDE_DIR)
    add_library(spdlog::spdlog INTERFACE IMPORTED)
    set_target_properties(spdlog::spdlog PROPERTIES
      INTERFACE_INCLUDE_DIRECTORIES "${SPDLOG_INCLUDE_DIR}"
    )
    message(STATUS "Found spdlog (header-only): ${SPDLOG_INCLUDE_DIR}")
  endif()
endif()


# Link external libraries to target (simple direct linking)
macro(ipcam_link_externals target)
  # Link miniupnpc if found
  if(IPCAM_MINIUPNPC_FOUND)
    target_link_libraries(${target} PRIVATE ${MINIUPNPC_LIBRARY})
    target_include_directories(${target} PRIVATE ${MINIUPNPC_INCLUDE_DIR})
  endif()

  # Link standard libraries (sqlite3, ssl, crypto)
  target_link_libraries(${target} PRIVATE sqlite3 ssl crypto)
endmacro()
