ipcam_add_library(ipcam_platform SHARED)
target_sources(ipcam_platform PRIVATE 
  src/platform.cpp
  src/isp_control.cpp
  src/ir_control.cpp
  src/reset_button.cpp
  src/hdal_wrapper.cpp
  # HDAL Pipeline (split into focused modules)
  src/hdal_pipeline_config.cpp
  src/hdal_pipeline_init.cpp
  src/hdal_pipeline_video.cpp
  src/hdal_pipeline_audio.cpp
  src/hdal_pipeline_lifecycle.cpp
  src/osd_overlay.cpp
  # Audio codecs
  src/g711_codec.cpp
  src/g726_codec.cpp
  src/aac_codec.cpp
  src/system_utils.cpp
  src/system_logger.cpp
)

# Additional include directories for config access
target_include_directories(ipcam_platform PRIVATE
  ${CMAKE_SOURCE_DIR}/modules/config/include
  ${CMAKE_SOURCE_DIR}/modules/media/include
  ${CMAKE_SOURCE_DIR}/modules/networking/include
  ${CMAKE_SOURCE_DIR}/modules/utils/include
)

# HDAL vendor ISP include directories
# CMAKE_SOURCE_DIR points to code/application/ipcamera
# HDAL is at code/hdal
set(HDAL_BASE_DIR "${CMAKE_SOURCE_DIR}/../../hdal")
set(HDAL_INCLUDE_DIR "${HDAL_BASE_DIR}/include")
set(HDAL_VENDOR_ISP_INCLUDE_DIR "${HDAL_BASE_DIR}/vendor/isp/include")
set(HDAL_VENDOR_MEDIA_INCLUDE_DIR "${HDAL_BASE_DIR}/vendor/media/include")
set(HDAL_COMMON_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/../../lib/include")
# VOS includes (kwrap/type.h and other kernel wrapper headers)
set(VOS_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/../../vos/include")
set(VOS_DRIVERS_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/../../vos/drivers/include")

message(STATUS "Checking for HDAL at: ${HDAL_VENDOR_ISP_INCLUDE_DIR}")

if(EXISTS ${HDAL_VENDOR_ISP_INCLUDE_DIR})
  target_include_directories(ipcam_platform PRIVATE
    ${HDAL_INCLUDE_DIR}
    ${HDAL_VENDOR_ISP_INCLUDE_DIR}
    ${HDAL_VENDOR_MEDIA_INCLUDE_DIR}
    ${HDAL_COMMON_INCLUDE_DIR}
    ${VOS_INCLUDE_DIR}
    ${VOS_DRIVERS_INCLUDE_DIR}
  )
  message(STATUS "HDAL vendor ISP headers found: ${HDAL_VENDOR_ISP_INCLUDE_DIR}")
  
  # Define preprocessor macro to enable HDAL integration
  target_compile_definitions(ipcam_platform PRIVATE HDAL_ISP_ENABLED=1)
  target_compile_definitions(ipcam_platform PRIVATE HDAL_PIPELINE_ENABLED=1)
  
  # Link vendor ISP library
  set(HDAL_VENDOR_ISP_LIB_DIR "${HDAL_BASE_DIR}/vendor/output")
  set(ROOTFS_LIB_DIR "${CMAKE_SOURCE_DIR}/../../../BSP/root-fs/rootfs/usr/lib")
  
  # Add link directories for the linker to find shared libraries
  target_link_directories(ipcam_platform PUBLIC 
    ${HDAL_VENDOR_ISP_LIB_DIR}
    ${ROOTFS_LIB_DIR}
  )
  
  if(EXISTS "${HDAL_VENDOR_ISP_LIB_DIR}/libvendor_isp.so")
    target_link_libraries(ipcam_platform PRIVATE vendor_isp)
    message(STATUS "Linking vendor_isp shared library from ${HDAL_VENDOR_ISP_LIB_DIR}")
  elseif(EXISTS "${HDAL_VENDOR_ISP_LIB_DIR}/libvendor_isp.a")
    target_link_libraries(ipcam_platform PRIVATE vendor_isp)
    message(STATUS "Linking vendor_isp static library from ${HDAL_VENDOR_ISP_LIB_DIR}")
  elseif(EXISTS "${ROOTFS_LIB_DIR}/libvendor_isp.so")
    target_link_libraries(ipcam_platform PRIVATE vendor_isp)
    message(STATUS "Linking vendor_isp from rootfs: ${ROOTFS_LIB_DIR}")
  else()
    message(WARNING "libvendor_isp not found - will use stubs")
    target_compile_definitions(ipcam_platform PRIVATE HDAL_ISP_ENABLED=0)
    target_compile_definitions(ipcam_platform PRIVATE HDAL_PIPELINE_ENABLED=0)
  endif()

  # Link prebuilt_isp for SW CDS (software color day/night switch) functions
  if(EXISTS "${HDAL_VENDOR_ISP_LIB_DIR}/libprebuilt_isp.so" OR EXISTS "${ROOTFS_LIB_DIR}/libprebuilt_isp.so")
    target_link_libraries(ipcam_platform PRIVATE prebuilt_isp)
    message(STATUS "Linking prebuilt_isp for SW CDS functionality")
  else()
    message(WARNING "libprebuilt_isp not found - SW CDS will be disabled")
  endif()
  
  # FreeType for OSD text rendering (configure first, independent of HDAL)
  set(FREETYPE_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/../../lib/include/freetype2")
  set(FREETYPE_LIB_DIR "${CMAKE_SOURCE_DIR}/../../lib/external/__install/lib")
  if(EXISTS ${FREETYPE_INCLUDE_DIR})
    target_include_directories(ipcam_platform PRIVATE ${FREETYPE_INCLUDE_DIR})
    target_link_directories(ipcam_platform PRIVATE ${FREETYPE_LIB_DIR})
    target_link_libraries(ipcam_platform PRIVATE freetype)
    message(STATUS "FreeType enabled for OSD text rendering")
  else()
    message(WARNING "FreeType not found - OSD text rendering disabled")
  endif()
  
  # Link HDAL core libraries for full pipeline support
  # Note: In this SDK, HDAL is a combined library (libhdal.so) not individual modules
  set(HDAL_LIB_DIR "${HDAL_BASE_DIR}/output")
  if(EXISTS "${HDAL_LIB_DIR}/libhdal.so" OR EXISTS "${ROOTFS_LIB_DIR}/libhdal.so")
    target_link_directories(ipcam_platform PUBLIC ${HDAL_LIB_DIR} ${ROOTFS_LIB_DIR})
    
    # HDAL combined library contains: hd_common, hd_videocapture, hd_videoprocess,
    # hd_videoencoder, hd_audiocapture, hd_audioencoder, etc.
    target_link_libraries(ipcam_platform PRIVATE
      hdal
      vendor_media
    )
    message(STATUS "Linking HDAL core libraries (libhdal.so, libvendor_media.so)")
  else()
    message(WARNING "HDAL core libraries not found - pipeline will use stubs")
    target_compile_definitions(ipcam_platform PRIVATE HDAL_PIPELINE_ENABLED=0)
  endif()
else()
  message(WARNING "HDAL vendor ISP headers not found - ISP control will use stubs")
  target_compile_definitions(ipcam_platform PRIVATE HDAL_ISP_ENABLED=0)
endif()

# vo-aacenc library for AAC-LC encoding (patents expired 2017-2018, royalty-free)
find_library(VO_AACENC_LIBRARY vo-aacenc PATHS ${CMAKE_SYSROOT}/usr/lib)
if(VO_AACENC_LIBRARY)
  target_include_directories(ipcam_platform PRIVATE ${CMAKE_SYSROOT}/usr/include)
  target_link_libraries(ipcam_platform PRIVATE ${VO_AACENC_LIBRARY})
  target_compile_definitions(ipcam_platform PRIVATE HAVE_VO_AACENC=1)
  message(STATUS "ipcam_platform: vo-aacenc found: ${VO_AACENC_LIBRARY}")
else()
  message(WARNING "ipcam_platform: vo-aacenc not found - AAC encoding will not be available")
endif()

# Link to config module for settings access
target_link_libraries(ipcam_platform PUBLIC ipcam_config)

# Link to networking module for NtpManager (DST config)
target_link_libraries(ipcam_platform PRIVATE ipcam_networking)

# Link spdlog for logging
find_package(spdlog QUIET)
if(spdlog_FOUND OR TARGET spdlog::spdlog)
  target_link_libraries(ipcam_platform PRIVATE spdlog::spdlog)
endif()

# Link external dependencies if available
ipcam_link_externals(ipcam_platform)

if(BUILD_TESTING)
  add_subdirectory(tests)
endif()

# Install the shared library
install(TARGETS ipcam_platform
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)
