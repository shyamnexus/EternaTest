# CMake toolchain file for NVT IPCamera aarch64-ca53 cross-compilation

set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR aarch64)

# Detect toolchain path from environment or use fallback
if(DEFINED ENV{CROSS_TOOLCHAIN_BIN_PATH})
  set(NVT_TOOLCHAIN_BIN_DIR "$ENV{CROSS_TOOLCHAIN_BIN_PATH}/")
else()
  set(NVT_TOOLCHAIN_BIN_DIR "/opt/ivot/aarch64-ca53-linux-gnueabihf-10.4.0/usr/bin/")
endif()

# Cross compiler tools
set(CMAKE_C_COMPILER   "${NVT_TOOLCHAIN_BIN_DIR}aarch64-ca53-linux-gnu-gcc")
set(CMAKE_CXX_COMPILER "${NVT_TOOLCHAIN_BIN_DIR}aarch64-ca53-linux-gnu-g++")
set(CMAKE_ASM_COMPILER "${NVT_TOOLCHAIN_BIN_DIR}aarch64-ca53-linux-gnu-as")
set(CMAKE_AR           "${NVT_TOOLCHAIN_BIN_DIR}aarch64-ca53-linux-gnu-ar")
set(CMAKE_NM           "${NVT_TOOLCHAIN_BIN_DIR}aarch64-ca53-linux-gnu-nm")
set(CMAKE_STRIP        "${NVT_TOOLCHAIN_BIN_DIR}aarch64-ca53-linux-gnu-strip")
set(CMAKE_OBJCOPY      "${NVT_TOOLCHAIN_BIN_DIR}aarch64-ca53-linux-gnu-objcopy")
set(CMAKE_OBJDUMP      "${NVT_TOOLCHAIN_BIN_DIR}aarch64-ca53-linux-gnu-objdump")
set(CMAKE_RANLIB       "${NVT_TOOLCHAIN_BIN_DIR}aarch64-ca53-linux-gnu-ranlib")

# Sysroot
if(DEFINED ENV{SYSROOT_PATH})
  set(CMAKE_SYSROOT "$ENV{SYSROOT_PATH}")
else()
  set(CMAKE_SYSROOT "/opt/ivot/aarch64-ca53-linux-gnueabihf-10.4.0/usr/aarch64-ca53-linux-gnu/sysroot")
endif()

# Platform-specific flags aligned with envconfig.sh
set(CMAKE_C_FLAGS "-march=armv8-a -mtune=cortex-a53 -ftree-vectorize -fno-builtin -fno-common -Wformat=1 -D_BSP_NS02302_ -D_NVT_IPC_SDK_" CACHE STRING "" FORCE)
set(CMAKE_CXX_FLAGS "-march=armv8-a -mtune=cortex-a53 -ftree-vectorize -fno-builtin -fno-common -Wformat=1 -D_BSP_NS02302_ -D_NVT_IPC_SDK_" CACHE STRING "" FORCE)

# Configure find_* commands to work in cross-compilation mode
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

# Build find root path list from environment variables
set(_find_root_paths "${CMAKE_SYSROOT}")

# Add NVT SDK specific paths to find root
if(DEFINED ENV{LIBRARY_DIR})
  list(APPEND _find_root_paths "$ENV{LIBRARY_DIR}" "$ENV{LIBRARY_DIR}/external/__install")
endif()

if(DEFINED ENV{EXTERNAL} AND DEFINED ENV{INSTALL_DIR})
  list(APPEND _find_root_paths "$ENV{EXTERNAL}/$ENV{INSTALL_DIR}")
endif()

if(DEFINED ENV{INCLUDE_DIR})
  list(APPEND _find_root_paths "$ENV{INCLUDE_DIR}")
endif()

# Add SDK library paths from workspace structure
get_filename_component(_sdk_root "${CMAKE_CURRENT_LIST_DIR}/../../../.." ABSOLUTE)
if(EXISTS "${_sdk_root}/code/lib/include")
  list(APPEND _find_root_paths "${_sdk_root}/code/lib")
  # Add include and library directories for curl and miniupnpc
  include_directories(SYSTEM "${_sdk_root}/code/lib/include")
  link_directories("${_sdk_root}/code/lib/output")
endif()

set(CMAKE_FIND_ROOT_PATH "${_find_root_paths}")

# Set target for better package detection
set(CMAKE_C_COMPILER_TARGET aarch64-ca53-linux-gnu)
set(CMAKE_CXX_COMPILER_TARGET aarch64-ca53-linux-gnu)
