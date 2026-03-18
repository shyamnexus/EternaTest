# AI Analytics Module
# Provides: Person/Vehicle Detection, Face Detection, Motion Detection, Line Crossing
# Video Quality Analysis, Object Tracking, Privacy Mosaic, AI-ISP Enhancement

ipcam_add_library(ipcam_ai SHARED)
target_sources(ipcam_ai PRIVATE 
  src/ai.cpp
  src/analytics.cpp
  src/motion_detection.cpp
  src/object_tracking.cpp
  src/privacy_mosaic.cpp
  src/aiisp.cpp
  src/tamper_detection.cpp
  src/npu_inference.cpp
  src/yolo_postproc.cpp
  src/scrfd_postproc.cpp
  src/line_crossing.cpp
  src/zone_intrusion.cpp
  src/counting.cpp
  src/face_recognition.cpp
  src/lpr.cpp
  src/heat_map.cpp
  src/pose_estimation.cpp
  src/face_attribute.cpp
  src/audio_classification.cpp
)

# Public include directories
target_include_directories(ipcam_ai 
  PUBLIC include
  PRIVATE ${CMAKE_SOURCE_DIR}/modules/config/include
  PRIVATE ${CMAKE_SOURCE_DIR}/modules/platform/include
  PRIVATE ${CMAKE_SOURCE_DIR}/modules/streaming/include
)

# Link to config and platform modules
target_link_libraries(ipcam_ai 
  PUBLIC ipcam_config
  PUBLIC ipcam_platform
)

# ============================================================================
# HDAL and AI Libraries Setup
# ============================================================================

# HDAL paths (same as platform module)
set(HDAL_BASE_DIR "${CMAKE_SOURCE_DIR}/../../hdal")
set(HDAL_INCLUDE_DIR "${HDAL_BASE_DIR}/include")
set(HDAL_VENDOR_ISP_INCLUDE_DIR "${HDAL_BASE_DIR}/vendor/isp/include")
set(HDAL_VENDOR_MEDIA_INCLUDE_DIR "${HDAL_BASE_DIR}/vendor/media/include")
set(HDAL_VENDOR_AI3_INCLUDE_DIR "${HDAL_BASE_DIR}/vendor/ai3/include")
set(HDAL_VENDOR_CV_INCLUDE_DIR "${HDAL_BASE_DIR}/vendor/cv/include")
set(HDAL_COMMON_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/../../lib/include")
set(VOS_BASE_DIR "${CMAKE_SOURCE_DIR}/../../vos")
set(VOS_INCLUDE_DIR "${VOS_BASE_DIR}/include")
set(VOS_DRIVERS_INCLUDE_DIR "${VOS_BASE_DIR}/drivers/include")
set(VOS_OUTPUT_DIR "${VOS_BASE_DIR}/output")

# SDK AI library paths
set(SDK_LIB_DIR "${CMAKE_SOURCE_DIR}/../../lib")
set(SDK_LIB_OUTPUT_DIR "${SDK_LIB_DIR}/__install/lib")
set(ROOTFS_LIB_DIR "${CMAKE_SOURCE_DIR}/../../../BSP/root-fs/rootfs/usr/lib")

# HDAL includes
if(EXISTS ${HDAL_INCLUDE_DIR})
  target_include_directories(ipcam_ai PRIVATE
    ${HDAL_INCLUDE_DIR}
    ${HDAL_VENDOR_ISP_INCLUDE_DIR}
    ${HDAL_VENDOR_MEDIA_INCLUDE_DIR}
    ${HDAL_VENDOR_AI3_INCLUDE_DIR}
    ${HDAL_VENDOR_CV_INCLUDE_DIR}
    ${HDAL_COMMON_INCLUDE_DIR}
    ${VOS_INCLUDE_DIR}
    ${VOS_DRIVERS_INCLUDE_DIR}
  )
  message(STATUS "AI module: HDAL headers found")
  target_compile_definitions(ipcam_ai PRIVATE HDAL_PIPELINE_ENABLED=1)
else()
  message(WARNING "AI module: HDAL headers not found - using stubs")
  target_compile_definitions(ipcam_ai PRIVATE HDAL_PIPELINE_ENABLED=0)
endif()

# AI library directories
target_link_directories(ipcam_ai PRIVATE
  ${SDK_LIB_OUTPUT_DIR}
  ${ROOTFS_LIB_DIR}
  ${HDAL_BASE_DIR}/output
  ${HDAL_BASE_DIR}/vendor/output
  ${VOS_OUTPUT_DIR}
)

# HDAL core library (for videoproc operations)
if(EXISTS "${ROOTFS_LIB_DIR}/libhdal.so" OR EXISTS "${HDAL_BASE_DIR}/output/libhdal.so")
  target_link_libraries(ipcam_ai PRIVATE hdal vendor_media)
  message(STATUS "AI module: Linking HDAL core libraries")
endif()

# VOS library (required by vendor_ai3)
if(EXISTS "${ROOTFS_LIB_DIR}/libvos.so" OR EXISTS "${VOS_OUTPUT_DIR}/libvos.so")
  target_link_libraries(ipcam_ai PRIVATE vos)
  target_compile_definitions(ipcam_ai PRIVATE VOS_ENABLED=1)
  message(STATUS "AI module: VOS library enabled")
else()
  message(WARNING "AI module: libvos not found")
  target_compile_definitions(ipcam_ai PRIVATE VOS_ENABLED=0)
endif()

# PVDCNN (Person/Vehicle Detection CNN)
if(EXISTS "${ROOTFS_LIB_DIR}/libpvdcnn_lib.so" OR EXISTS "${SDK_LIB_OUTPUT_DIR}/libpvdcnn_lib.so")
  target_link_libraries(ipcam_ai PRIVATE pvdcnn_lib)
  target_compile_definitions(ipcam_ai PRIVATE AI_PVDCNN_ENABLED=1)
  message(STATUS "AI module: PVDCNN (Person/Vehicle) detection enabled")
else()
  message(WARNING "AI module: libpvdcnn_lib not found - Person/Vehicle detection disabled")
  target_compile_definitions(ipcam_ai PRIVATE AI_PVDCNN_ENABLED=0)
endif()

# FDCNN (Face Detection CNN)
if(EXISTS "${ROOTFS_LIB_DIR}/libfdcnn_lib.so" OR EXISTS "${SDK_LIB_OUTPUT_DIR}/libfdcnn_lib.so")
  target_link_libraries(ipcam_ai PRIVATE fdcnn_lib)
  target_compile_definitions(ipcam_ai PRIVATE AI_FDCNN_ENABLED=1)
  message(STATUS "AI module: FDCNN (Face) detection enabled")
else()
  message(WARNING "AI module: libfdcnn_lib not found - Face detection disabled")
  target_compile_definitions(ipcam_ai PRIVATE AI_FDCNN_ENABLED=0)
endif()

# Motion Detection library (via vendor_cv/prebuilt_cv)
set(CV_LIB_DIR "${HDAL_BASE_DIR}/vendor/output")
if(EXISTS "${ROOTFS_LIB_DIR}/libvendor_cv.so" OR EXISTS "${CV_LIB_DIR}/libvendor_cv.so")
  target_link_libraries(ipcam_ai PRIVATE vendor_cv prebuilt_cv)
  target_compile_definitions(ipcam_ai PRIVATE AI_MD_ENABLED=1)
  message(STATUS "AI module: Motion detection (vendor_cv) enabled")
  # Note: Old VQA library removed - tamper detection now uses IVE hardware directly
else()
  message(WARNING "AI module: libvendor_cv not found - Motion detection will use fallback")
  target_compile_definitions(ipcam_ai PRIVATE AI_MD_ENABLED=0)
endif()

# Legacy VQA library removed - tamper detection uses IVE directly via libive
# (see tamper_detection.cpp which uses NVT_IVE_Histo/NVT_IVE_Sobel)

# TRKE (Tracking Engine) library
if(EXISTS "${ROOTFS_LIB_DIR}/libtrke.so" OR EXISTS "${SDK_LIB_OUTPUT_DIR}/libtrke.so")
  target_link_libraries(ipcam_ai PRIVATE trke vendor_trke)
  target_compile_definitions(ipcam_ai PRIVATE AI_TRKE_ENABLED=1)
  message(STATUS "AI module: TRKE (Tracking Engine) enabled")
else()
  message(STATUS "AI module: libtrke not found - using IOU tracker fallback")
  target_compile_definitions(ipcam_ai PRIVATE AI_TRKE_ENABLED=0)
endif()

# Vendor GFX library (for privacy mosaic)
if(EXISTS "${ROOTFS_LIB_DIR}/libvendor_gfx.so" OR EXISTS "${HDAL_BASE_DIR}/vendor/output/libvendor_gfx.so")
  target_link_libraries(ipcam_ai PRIVATE vendor_gfx)
  target_compile_definitions(ipcam_ai PRIVATE AI_GFX_ENABLED=1)
  message(STATUS "AI module: Vendor GFX (privacy mosaic) enabled")
else()
  message(WARNING "AI module: libvendor_gfx not found - Privacy mosaic disabled")
  target_compile_definitions(ipcam_ai PRIVATE AI_GFX_ENABLED=0)
endif()

# Vendor ISP library (for AIISP integration)
if(EXISTS "${ROOTFS_LIB_DIR}/libvendor_isp.so" OR EXISTS "${HDAL_BASE_DIR}/vendor/output/libvendor_isp.so")
  target_link_libraries(ipcam_ai PRIVATE vendor_isp)
  target_compile_definitions(ipcam_ai PRIVATE AI_ISP_ENABLED=1)
  message(STATUS "AI module: Vendor ISP (AIISP) enabled")
else()
  message(STATUS "AI module: libvendor_isp not found - AIISP ISP integration disabled")
  target_compile_definitions(ipcam_ai PRIVATE AI_ISP_ENABLED=0)
endif()

# Vendor AI3 library (AI engine wrapper)
# vendor_ai3 requires VOS library for vos_file_* and vos_util_* functions
# vendor_ai3 also requires vendor_ai3_pub and prebuilt_ai for utility functions
if((EXISTS "${ROOTFS_LIB_DIR}/libvendor_ai3.so" OR EXISTS "${HDAL_BASE_DIR}/vendor/output/libvendor_ai3.so") AND 
   (EXISTS "${ROOTFS_LIB_DIR}/libvos.so" OR EXISTS "${VOS_OUTPUT_DIR}/libvos.so"))
  # Link order matters: prebuilt_ai -> vendor_ai3_pub -> vendor_ai3
  target_link_libraries(ipcam_ai PRIVATE prebuilt_ai vendor_ai3_pub vendor_ai3)
  target_compile_definitions(ipcam_ai PRIVATE VENDOR_AI3_ENABLED=1)
  message(STATUS "AI module: Vendor AI3 engine enabled (with VOS)")
else()
  message(STATUS "AI module: Vendor AI3 disabled (requires VOS library)")
  target_compile_definitions(ipcam_ai PRIVATE VENDOR_AI3_ENABLED=0)
endif()

# Vendor video encoder (for SMART_BBOX overlay)
if(EXISTS "${ROOTFS_LIB_DIR}/libvendor_videoenc.so" OR EXISTS "${HDAL_BASE_DIR}/vendor/output/libvendor_videoenc.so")
  target_link_libraries(ipcam_ai PRIVATE vendor_videoenc)
  message(STATUS "AI module: Vendor videoenc for SMART_BBOX enabled")
endif()

# Link spdlog for logging
find_package(spdlog QUIET)
if(spdlog_FOUND OR TARGET spdlog::spdlog)
  target_link_libraries(ipcam_ai PRIVATE spdlog::spdlog)
endif()

# Link external dependencies if available
ipcam_link_externals(ipcam_ai)

target_compile_features(ipcam_ai PUBLIC cxx_std_17)

if(BUILD_TESTING)
  add_subdirectory(tests)
endif()

# Install the shared library
install(TARGETS ipcam_ai
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)
