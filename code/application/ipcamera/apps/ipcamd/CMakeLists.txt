ipcam_add_executable(ipcamd src/main.cpp)

# Add HDAL vendor library search path for linking
set(HDAL_VENDOR_LIB_DIR "${CMAKE_SOURCE_DIR}/../../hdal/vendor/output")
set(HDAL_LIB_DIR "${CMAKE_SOURCE_DIR}/../../hdal/output")
set(ROOTFS_LIB_DIR "${CMAKE_SOURCE_DIR}/../../../BSP/root-fs/rootfs/usr/lib")
set(FREETYPE_LIB_DIR "${CMAKE_SOURCE_DIR}/../../lib/external/__install/lib")
set(VOS_LIB_DIR "${CMAKE_SOURCE_DIR}/../../vos/output")
set(AUDIO_LIB_DIR "${CMAKE_SOURCE_DIR}/../../lib/output")
target_link_directories(ipcamd PRIVATE ${HDAL_LIB_DIR} ${HDAL_VENDOR_LIB_DIR} ${ROOTFS_LIB_DIR} ${FREETYPE_LIB_DIR} ${VOS_LIB_DIR} ${AUDIO_LIB_DIR})

# Add rpath-link for the linker to find transitive dependencies
target_link_options(ipcamd PRIVATE 
  "LINKER:-rpath-link,${HDAL_LIB_DIR}"
  "LINKER:-rpath-link,${HDAL_VENDOR_LIB_DIR}"
  "LINKER:-rpath-link,${ROOTFS_LIB_DIR}"
  "LINKER:-rpath-link,${FREETYPE_LIB_DIR}"
  "LINKER:-rpath-link,${VOS_LIB_DIR}"
  "LINKER:-rpath-link,${AUDIO_LIB_DIR}"
)

target_link_libraries(ipcamd PRIVATE 
  ipcam_platform 
  ipcam_media 
  ipcam_ai 
  ipcam_networking 
  ipcam_streaming 
  ipcam_storage 
  ipcam_recording
  ipcam_events  # Event system for actions/notifications
  ipcam_config 
  ipcam_utils
  ipcam_webserver
  ipcam_onvif  # ONVIF WS-Discovery and device services
  hdal
  vendor_media
  vos  # Required by vendor_ai3 for vos_file_* and vos_util_* functions
)

# Link external dependencies automatically
ipcam_link_externals(ipcamd)

# Install the ipcamd executable
install(TARGETS ipcamd
    RUNTIME DESTINATION bin
)

# Set RPATH so ipcamd can find shared libraries in ../lib
set_target_properties(ipcamd PROPERTIES
    INSTALL_RPATH "\$ORIGIN/../lib"
    BUILD_WITH_INSTALL_RPATH FALSE
    INSTALL_RPATH_USE_LINK_PATH TRUE
)
