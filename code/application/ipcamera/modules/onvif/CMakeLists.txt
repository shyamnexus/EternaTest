cmake_minimum_required(VERSION 3.15)

# ONVIF WS-Discovery module
project(ipcam_onvif C CXX)

# C++ wrapper for integration with ipcamera application
set(ONVIF_CPP_SOURCES
    src/onvif.cpp
    src/image_adjustment_wrapper.cpp
    src/video_control_wrapper.cpp
    src/audio_control_wrapper.cpp
    src/network_wrapper.cpp
    src/analytics_wrapper.cpp
    src/osd_wrapper.cpp
    src/user_wrapper.cpp
    src/onvif_profiles.cpp
    src/onvif_recording_wrapper.cpp  # C wrapper for RecordingService (Profile G)
)

# Source files from the onvif module
# Rockchip-specific functions are stubbed with dummy values
# TODO: Replace with Novatek implementations
set(ONVIF_SOURCES
    src/onvif_main.c
    src/wsDiscovery.c
    src/onvif_device.c
    src/onvif_media.c
    src/onvif_services.c
    src/onvif_adapter.c
    src/onvif_services_stubs.c  # Stub implementations for unsupported ONVIF services
    src/onvif_analytics.c       # Analytics with stubbed platform functions
    src/onvif_imaging.c         # Imaging service with Novatek ISP integration
    src/onvif_recording.c       # Recording Control and Replay (Profile G)
    # Media2 disabled - NVR will use Media1 (trt) instead which is fully working
    # src/onvif_media2_minimal.c
    src/onvif_events.c
)

# gSOAP generated sources
set(GSOAP_SOURCES
    gsoap/gsoap_createfile/soapC.c
    gsoap/gsoap_createfile/soapClient.c
    gsoap/gsoap_createfile/soapServer.c
    gsoap/gsoap_createfile/stdsoap2.c
    gsoap/gsoap_createfile/dom.c
    gsoap/custom/struct_timeval.c
)

# gSOAP plugin sources
set(GSOAP_PLUGIN_SOURCES
    gsoap/plugin/httpda.c
    gsoap/plugin/httpget.c
    gsoap/plugin/httppost.c
    gsoap/plugin/md5evp.c
    gsoap/plugin/mecevp.c
    gsoap/plugin/smdevp.c
    gsoap/plugin/threads.c
    gsoap/plugin/wsaapi.c
    gsoap/plugin/wsddapi.c
    gsoap/plugin/wsseapi.c
)

# Create the library (SHARED to match other modules)
add_library(ipcam_onvif SHARED
    ${ONVIF_CPP_SOURCES}
    ${ONVIF_SOURCES}
    ${GSOAP_SOURCES}
    ${GSOAP_PLUGIN_SOURCES}
)

# Include directories
target_include_directories(ipcam_onvif PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/include/ipcam
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/gsoap/gsoap_createfile
    ${CMAKE_CURRENT_SOURCE_DIR}/gsoap/plugin
    ${CMAKE_CURRENT_SOURCE_DIR}/gsoap/import
    ${CMAKE_CURRENT_SOURCE_DIR}/gsoap/custom
    ${CMAKE_CURRENT_SOURCE_DIR}/../isp/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../isp/include/ipcam
    ${CMAKE_CURRENT_SOURCE_DIR}/../media/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../media/include/ipcam
    ${CMAKE_CURRENT_SOURCE_DIR}/../ai/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../ai/include/ipcam
    ${CMAKE_CURRENT_SOURCE_DIR}/../recording/include  # For Profile G integration
    ${CMAKE_CURRENT_SOURCE_DIR}/../recording/include/ipcam
    ${CMAKE_CURRENT_SOURCE_DIR}/../streaming/include  # For RTSP server ONVIF interface
    ${CMAKE_CURRENT_SOURCE_DIR}/../streaming/include/ipcam
    ${CMAKE_SOURCE_DIR}/../../hdal/include
    ${CMAKE_SOURCE_DIR}/../../hdal/vendor/isp/include
    ${CMAKE_SOURCE_DIR}/../../hdal/vendor/media/include
    ${CMAKE_SOURCE_DIR}/../../vos/include
)

# Link libraries
target_link_libraries(ipcam_onvif PUBLIC
    ssl
    crypto
    pthread
    ipcam_utils
    ipcam_config
    ipcam_platform
    ipcam_networking
    ipcam_streaming
    ipcam_media
    ipcam_ai
    ipcam_recording  # For Profile G (Recording Control)
)

# Compiler definitions
target_compile_definitions(ipcam_onvif PRIVATE
    WITH_OPENSSL
    WITH_DOM
    WITH_PURE_VIRTUAL
)

# Disable some warnings for gSOAP generated code
if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(ipcam_onvif PRIVATE
        -Wno-unused-parameter
        -Wno-unused-variable
        -Wno-sign-compare
        -Wno-format
    )
endif()

# Export include directory
target_include_directories(ipcam_onvif INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)
