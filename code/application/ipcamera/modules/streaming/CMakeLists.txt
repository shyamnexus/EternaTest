ipcam_add_library(ipcam_streaming SHARED)
target_sources(ipcam_streaming PRIVATE 
    src/streaming.cpp
    src/go2rtc_manager.cpp
    src/rtsp_server.cpp
    src/hdal_video_source.cpp
    src/hdal_audio_source.cpp
    src/audio_frame_broadcaster.cpp
    src/audio_processor.cpp
    src/onvif_rtsp_source.cpp
    src/localhost_bypass_server.cpp
    src/onvif_metadata.cpp
)

# Link to required modules
target_link_libraries(ipcam_streaming PUBLIC ipcam_utils ipcam_config ipcam_platform)

# Link media module for audio encoder (AAC via vo-aacenc)
target_link_libraries(ipcam_streaming PRIVATE ipcam_media)

# HDAL access (same pattern as platform module)
set(HDAL_BASE_DIR "${CMAKE_SOURCE_DIR}/../../hdal")
set(HDAL_INCLUDE_DIR "${HDAL_BASE_DIR}/include")
if(EXISTS ${HDAL_INCLUDE_DIR})
    target_include_directories(ipcam_streaming PRIVATE ${HDAL_INCLUDE_DIR})
endif()

# ============================================================================
# Audio Processing Libraries (NS, AGC, HPF/Notch)
# ============================================================================
set(AUDIO_LIB_DIR "${CMAKE_SOURCE_DIR}/../../lib/output")
set(AUDIO_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/../../lib/include")

# Add include directory for audio library headers
if(EXISTS ${AUDIO_INCLUDE_DIR})
    target_include_directories(ipcam_streaming PRIVATE ${AUDIO_INCLUDE_DIR})
    message(STATUS "ipcam_streaming: Audio lib headers found at ${AUDIO_INCLUDE_DIR}")
endif()

# VOS include path for kwrap/type.h (required by audlib headers)
set(VOS_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/../../vos/include")
if(EXISTS ${VOS_INCLUDE_DIR})
    target_include_directories(ipcam_streaming PRIVATE ${VOS_INCLUDE_DIR})
    message(STATUS "ipcam_streaming: VOS headers found at ${VOS_INCLUDE_DIR}")
endif()

# libaec.a - Noise Suppression (NS) and Echo Cancellation (AEC)
find_library(LIBAEC_LIBRARY aec PATHS ${AUDIO_LIB_DIR} NO_DEFAULT_PATH)
if(LIBAEC_LIBRARY)
    target_link_libraries(ipcam_streaming PRIVATE ${LIBAEC_LIBRARY})
    target_compile_definitions(ipcam_streaming PRIVATE HAVE_LIBAEC=1)
    message(STATUS "ipcam_streaming: libaec found: ${LIBAEC_LIBRARY}")
else()
    message(WARNING "ipcam_streaming: libaec not found - NS/AEC will be disabled")
endif()

# libagc.a - Automatic Gain Control
find_library(LIBAGC_LIBRARY agc PATHS ${AUDIO_LIB_DIR} NO_DEFAULT_PATH)
if(LIBAGC_LIBRARY)
    target_link_libraries(ipcam_streaming PRIVATE ${LIBAGC_LIBRARY})
    target_compile_definitions(ipcam_streaming PRIVATE HAVE_LIBAGC=1)
    message(STATUS "ipcam_streaming: libagc found: ${LIBAGC_LIBRARY}")
else()
    message(WARNING "ipcam_streaming: libagc not found - AGC will be disabled")
endif()

# libaudfilt.a - Audio Filters (HPF, Notch, EQ)
find_library(LIBAUDFILT_LIBRARY audfilt PATHS ${AUDIO_LIB_DIR} NO_DEFAULT_PATH)
if(LIBAUDFILT_LIBRARY)
    target_link_libraries(ipcam_streaming PRIVATE ${LIBAUDFILT_LIBRARY})
    target_compile_definitions(ipcam_streaming PRIVATE HAVE_LIBAUDFILT=1)
    message(STATUS "ipcam_streaming: libaudfilt found: ${LIBAUDFILT_LIBRARY}")
else()
    message(WARNING "ipcam_streaming: libaudfilt not found - HPF/Notch will be disabled")
endif()

# Live555 RTSP server (found in modules.cmake)
if(IPCAM_LIVE555_FOUND)
    target_include_directories(ipcam_streaming PRIVATE ${LIVE555_INCLUDE_DIR})
    target_link_libraries(ipcam_streaming PRIVATE ${LIVE555_LIBRARIES})
    target_compile_definitions(ipcam_streaming PRIVATE RTSP_SERVER_ENABLED=1)
    message(STATUS "ipcam_streaming: Live555 RTSP server enabled")
else()
    target_compile_definitions(ipcam_streaming PRIVATE RTSP_SERVER_ENABLED=0)
    message(WARNING "ipcam_streaming: Live555 not found - RTSP server disabled")
endif()

# Link externals (sqlite3, ssl, crypto, etc.)
ipcam_link_externals(ipcam_streaming)

# Logging (spdlog - header only)
if(TARGET spdlog::spdlog)
    target_link_libraries(ipcam_streaming PRIVATE spdlog::spdlog)
endif()

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()

install(TARGETS ipcam_streaming
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)
