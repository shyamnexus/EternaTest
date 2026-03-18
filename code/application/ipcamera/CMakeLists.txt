cmake_minimum_required(VERSION 3.15)

# Enable ccache if available for faster rebuilds
find_program(CCACHE_PROGRAM ccache)
if(CCACHE_PROGRAM)
    set(CMAKE_C_COMPILER_LAUNCHER "${CCACHE_PROGRAM}")
    set(CMAKE_CXX_COMPILER_LAUNCHER "${CCACHE_PROGRAM}")
    message(STATUS "Using ccache for compilation: ${CCACHE_PROGRAM}")
endif()

# Allow building with system compiler if IPCAMERA_USE_SYSTEM_COMPILER is set
option(IPCAMERA_USE_SYSTEM_COMPILER "Build using system compiler (Ubuntu x64)" OFF)

# Use aarch64-ca53 toolchain by default (this project is part of the SDK)
if(NOT IPCAMERA_USE_SYSTEM_COMPILER AND NOT CMAKE_TOOLCHAIN_FILE AND NOT DEFINED ENV{CMAKE_TOOLCHAIN_FILE})
  set(CMAKE_TOOLCHAIN_FILE "${CMAKE_CURRENT_SOURCE_DIR}/cmake/toolchains/aarch64-ca53.cmake" CACHE FILEPATH "Toolchain file")
endif()

project(ipcamera LANGUAGES C CXX)

# Set BUILD_TESTING to OFF by default unless explicitly requested
if(NOT DEFINED BUILD_TESTING)
  set(BUILD_TESTING OFF CACHE BOOL "Build the testing tree.")
endif()

# Basic options
option(IPCAMERA_BUILD_TESTS "Build ipcamera tests" OFF)
option(IPCAMERA_BUILD_EXAMPLES "Build example apps/tools" ON)

# Include helper modules if present
if(EXISTS "${CMAKE_SOURCE_DIR}/cmake/modules.cmake")
  include(${CMAKE_SOURCE_DIR}/cmake/modules.cmake)
endif()

# Add SDK library paths for cross-compilation
if(CMAKE_CROSSCOMPILING)
  get_filename_component(_sdk_root "${CMAKE_SOURCE_DIR}/../../.." ABSOLUTE)
  if(EXISTS "${_sdk_root}/code/lib/include")
    include_directories(SYSTEM "${_sdk_root}/code/lib/include")
    link_directories("${_sdk_root}/code/lib/output")
    message(STATUS "Added SDK library paths: ${_sdk_root}/code/lib")
  endif()
endif()

# Subdirectories
if(EXISTS "${CMAKE_SOURCE_DIR}/modules")
  add_subdirectory(modules)
endif()
if(EXISTS "${CMAKE_SOURCE_DIR}/apps")
  add_subdirectory(apps)
endif()

# Copy configs to build directory
add_custom_target(copy_configs ALL
  COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/configs
    ${CMAKE_BINARY_DIR}/configs
  COMMENT "Copying configuration files to build directory"
  VERBATIM
)

# Set default install prefix if not set
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
  set(CMAKE_INSTALL_PREFIX "/usr/local" CACHE PATH "Install prefix" FORCE)
endif()

# Install configuration files
install(DIRECTORY configs/config.factory.d/
    DESTINATION etc/ipcamera/config.factory.d
    FILES_MATCHING PATTERN "*.json"
)

# Create empty user config directory (will be populated at runtime)
install(DIRECTORY DESTINATION etc/ipcamera/config.d)

# Install init script
if(EXISTS "${CMAKE_SOURCE_DIR}/init.d/S50ipcamd")
  install(FILES init.d/S50ipcamd
      DESTINATION etc/init.d
      PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
  )
endif()
