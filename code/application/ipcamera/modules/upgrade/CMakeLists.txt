ipcam_add_library(ipcam_upgrade SHARED)

target_sources(ipcam_upgrade PRIVATE
  src/upgrade_manager.cpp
  src/firmware_validator.cpp
  src/physical_memory.cpp
  src/system_controller.cpp
)

# =============================================================================
# Standalone Flasher Binary
# =============================================================================
# This is a minimal binary that runs AFTER the main app exits.
# It maps physical RAM (where firmware is staged) and writes to MTD partitions.
# Design follows the "two-binary" architecture for safe firmware updates.
#
# Build: cmake --build . --target nvt_flasher
# =============================================================================

add_executable(nvt_flasher src/nvt_flasher.cpp)

target_compile_features(nvt_flasher PRIVATE cxx_std_17)

# Minimal dependencies - this binary should be small and focused
target_compile_options(nvt_flasher PRIVATE
  -Os                    # Optimize for size
  -ffunction-sections    # Allow linker to remove unused functions
  -fdata-sections        # Allow linker to remove unused data
)

target_link_options(nvt_flasher PRIVATE
  -Wl,--gc-sections      # Remove unused sections
  -static-libstdc++      # Static link C++ library for standalone operation
)

# Install flasher binary to output
install(TARGETS nvt_flasher
  RUNTIME DESTINATION bin
)

# Install flasher script
install(PROGRAMS src/nvt_flasher.sh
  DESTINATION bin
)

# Include directories
target_include_directories(ipcam_upgrade PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_include_directories(ipcam_upgrade PRIVATE
  ${CMAKE_SOURCE_DIR}/modules/config/include
  ${CMAKE_SOURCE_DIR}/modules/utils/include
)

# HDAL and system library paths
set(HDAL_BASE_DIR "${CMAKE_SOURCE_DIR}/../../hdal")
set(HDAL_INCLUDE_DIR "${HDAL_BASE_DIR}/include")
set(HDAL_COMMON_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/../../lib/include")
set(ROOTFS_LIB_DIR "${CMAKE_SOURCE_DIR}/../../../BSP/root-fs/rootfs/usr/lib")
set(HDAL_VENDOR_OUTPUT_DIR "${HDAL_BASE_DIR}/vendor/output")

# Include HDAL headers for hd_common_mem functions
if(EXISTS ${HDAL_INCLUDE_DIR})
  target_include_directories(ipcam_upgrade PRIVATE
    ${HDAL_INCLUDE_DIR}
    ${HDAL_COMMON_INCLUDE_DIR}
  )
  target_compile_definitions(ipcam_upgrade PRIVATE HDAL_AVAILABLE=1)
  
  # Link HDAL for memory allocation
  target_link_directories(ipcam_upgrade PRIVATE
    ${HDAL_VENDOR_OUTPUT_DIR}
    ${ROOTFS_LIB_DIR}
  )
endif()

# OpenSSL for cryptographic operations (RSA, AES, SHA256)
find_package(OpenSSL QUIET)
if(OpenSSL_FOUND)
  target_link_libraries(ipcam_upgrade PRIVATE OpenSSL::SSL OpenSSL::Crypto)
  target_compile_definitions(ipcam_upgrade PRIVATE OPENSSL_AVAILABLE=1)
  message(STATUS "OpenSSL found for upgrade module crypto operations")
else()
  # Try SDK's bundled OpenSSL
  set(OPENSSL_SDK_DIR "${CMAKE_SOURCE_DIR}/../../lib/external/openssl-3.5.4")
  set(OPENSSL_INSTALL_DIR "${CMAKE_SOURCE_DIR}/../../lib/external/__install")
  if(EXISTS "${OPENSSL_INSTALL_DIR}/include/openssl/evp.h")
    target_include_directories(ipcam_upgrade PRIVATE ${OPENSSL_INSTALL_DIR}/include)
    target_link_directories(ipcam_upgrade PRIVATE ${OPENSSL_INSTALL_DIR}/lib)
    target_link_libraries(ipcam_upgrade PRIVATE ssl crypto)
    target_compile_definitions(ipcam_upgrade PRIVATE OPENSSL_AVAILABLE=1)
    message(STATUS "Using SDK bundled OpenSSL for upgrade module")
  else()
    message(WARNING "OpenSSL not found - secure firmware validation will be limited")
    target_compile_definitions(ipcam_upgrade PRIVATE OPENSSL_AVAILABLE=0)
  endif()
endif()

# Link dependencies
target_link_libraries(ipcam_upgrade
  PUBLIC
    ipcam_utils
  PRIVATE
    ipcam_config
    spdlog::spdlog
)
