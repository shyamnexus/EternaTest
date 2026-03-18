add_library(ipcam_media SHARED)
target_sources(ipcam_media PRIVATE 
  src/media.cpp
  src/video_control.cpp
  src/audio_control.cpp
  src/audio_encoder.cpp
  src/media_hub.cpp
)
target_include_directories(ipcam_media 
  PUBLIC include
  PRIVATE ${CMAKE_SOURCE_DIR}/modules/config/include
  PRIVATE ${CMAKE_SOURCE_DIR}/modules/platform/include
)

# HDAL vendor includes for audio processing APIs and MediaHub (encoder pull, memory alloc)
set(HDAL_BASE_DIR "${CMAKE_SOURCE_DIR}/../../hdal")
set(HDAL_INCLUDE_DIR "${HDAL_BASE_DIR}/include")
set(HDAL_VENDOR_MEDIA_INCLUDE_DIR "${HDAL_BASE_DIR}/vendor/media/include")
set(HDAL_VENDOR_ISP_INCLUDE_DIR "${HDAL_BASE_DIR}/vendor/isp/include")
set(HDAL_COMMON_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/../../lib/include")
set(VOS_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/../../vos/include")
set(VOS_DRIVERS_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/../../vos/drivers/include")
set(ROOTFS_LIB_DIR "${CMAKE_SOURCE_DIR}/../../../BSP/root-fs/rootfs/usr/lib")
set(HDAL_LIB_DIR "${HDAL_BASE_DIR}/output")
set(HDAL_VENDOR_LIB_DIR "${HDAL_BASE_DIR}/vendor/output")

if(EXISTS ${HDAL_VENDOR_MEDIA_INCLUDE_DIR})
  target_include_directories(ipcam_media PRIVATE
    ${HDAL_INCLUDE_DIR}
    ${HDAL_VENDOR_MEDIA_INCLUDE_DIR}
    ${HDAL_VENDOR_ISP_INCLUDE_DIR}
    ${HDAL_COMMON_INCLUDE_DIR}
    ${VOS_INCLUDE_DIR}
    ${VOS_DRIVERS_INCLUDE_DIR}
  )
  message(STATUS "ipcam_media: HDAL vendor media headers found")

  # Enable HDAL pipeline integration for MediaHub (encoder pull + memory alloc)
  target_compile_definitions(ipcam_media PRIVATE HDAL_PIPELINE_ENABLED=1)

  # Link HDAL core libraries for hd_videoenc_* and hd_common_mem_* APIs
  if(EXISTS "${HDAL_LIB_DIR}/libhdal.so" OR EXISTS "${ROOTFS_LIB_DIR}/libhdal.so")
    target_link_directories(ipcam_media PRIVATE ${HDAL_LIB_DIR} ${HDAL_VENDOR_LIB_DIR} ${ROOTFS_LIB_DIR})
    target_link_libraries(ipcam_media PRIVATE hdal vendor_media)
    message(STATUS "ipcam_media: Linked HDAL libraries for MediaHub")
  else()
    message(WARNING "ipcam_media: HDAL libraries not found - MediaHub will use stubs")
    target_compile_definitions(ipcam_media PRIVATE HDAL_PIPELINE_ENABLED=0)
  endif()
else()
  message(WARNING "ipcam_media: HDAL headers not found - MediaHub will use stubs")
  target_compile_definitions(ipcam_media PRIVATE HDAL_PIPELINE_ENABLED=0)
endif()

# vo-aacenc is now provided by ipcam_platform (linked transitively via ipcam_platform)
# No need to link it here separately.

# Link to config module for settings access
target_link_libraries(ipcam_media 
  PUBLIC ipcam_config
  PRIVATE ipcam_platform
)

# Link spdlog for logging
find_package(spdlog QUIET)
if(spdlog_FOUND OR TARGET spdlog::spdlog)
  target_link_libraries(ipcam_media PRIVATE spdlog::spdlog)
endif()

target_compile_features(ipcam_media PUBLIC cxx_std_17)

if(BUILD_TESTING)
  add_subdirectory(tests)
endif()

# Install the shared library
install(TARGETS ipcam_media
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)
