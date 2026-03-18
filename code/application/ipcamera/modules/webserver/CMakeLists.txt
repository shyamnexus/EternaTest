cmake_minimum_required(VERSION 3.15)

# Option to allow configuration without all dependencies (for compile_commands.json generation)
option(IPCAMERA_ALLOW_MISSING_DEPS "Allow configuration to succeed even with missing dependencies" OFF)

# Try to find CivetWeb (will use system installation or SDK external libs)
# modules.cmake already searches SDK paths and sets IPCAM_CIVETWEB_FOUND
find_package(civetweb CONFIG)
if(NOT civetweb_FOUND)
  if(IPCAM_CIVETWEB_FOUND)
    # Use CivetWeb from SDK external libs (found by modules.cmake)
    message(STATUS "Using CivetWeb from SDK: ${CIVETWEB_LIBRARY}")
    add_library(civetweb::civetweb-cpp UNKNOWN IMPORTED)
    set_target_properties(civetweb::civetweb-cpp PROPERTIES
      IMPORTED_LOCATION "${CIVETWEB_LIBRARY}"
      INTERFACE_INCLUDE_DIRECTORIES "${CIVETWEB_INCLUDE_DIR}"
    )
    set(CIVETWEB_AVAILABLE TRUE)
  else()
    if(IPCAMERA_ALLOW_MISSING_DEPS)
      message(WARNING "CivetWeb not found - webserver will be built with stubs")
      set(CIVETWEB_AVAILABLE FALSE)
    else()
      message(FATAL_ERROR "CivetWeb not found. Please install libcivetweb-dev or build from source. Use -DIPCAMERA_ALLOW_MISSING_DEPS=ON to skip.")
    endif()
  endif()
else()
  message(STATUS "Found CivetWeb package")
  set(CIVETWEB_AVAILABLE TRUE)
endif()

# Create webserver library as SHARED to avoid nlohmann/json duplication
add_library(ipcam_webserver SHARED
  src/webserver.cpp
  src/api_handlers.cpp
  src/crypto/password_crypto.cpp
  src/handlers/crypto_handler.cpp
  src/handlers/status_handler.cpp
  src/handlers/info_handler.cpp
  src/handlers/config_handler.cpp
  src/handlers/network_handler.cpp
  src/handlers/stream_handler.cpp
  src/handlers/smtp_handler.cpp
  src/handlers/snmp_handler.cpp
  src/handlers/upnp_handler.cpp
  src/handlers/ssl_handler.cpp
  src/handlers/auth_user_handlers.cpp
  src/handlers/image_handler.cpp
  src/handlers/video_handler.cpp
  src/handlers/audio_handler.cpp
  src/handlers/osd_handler.cpp
  src/handlers/recording_handler.cpp
  src/handlers/schedule_handler.cpp
  src/handlers/storage_handler.cpp
  src/handlers/playback_handler.cpp
  # Analytics module - modular structure for expandability
  src/handlers/analytics/analytics_router.cpp
  src/handlers/analytics/motion_handler.cpp
  src/handlers/analytics/tampering_handler.cpp
  src/handlers/analytics/notification_handler.cpp
  src/handlers/analytics/smart_handler.cpp
  src/handlers/analytics/linecross_handler.cpp
  src/handlers/analytics/intrusion_handler.cpp
  src/handlers/analytics/face_handler.cpp
  src/handlers/analytics/lpr_handler.cpp
  src/handlers/analytics/heatmap_handler.cpp
  src/handlers/analytics/pose_handler.cpp
  src/handlers/analytics/audio_handler.cpp
  src/handlers/rtsp_handler.cpp
  src/handlers/ir_handler.cpp
  src/handlers/upgrade_handler.cpp
  src/handlers/events_handler.cpp
  src/handlers/system_handler.cpp
)

# Find nlohmann JSON (QUIET to suppress warnings if not found - it's header-only)
find_package(nlohmann_json 3.2.0 QUIET)

target_include_directories(ipcam_webserver 
  PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}/include
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/modules/config/include
    ${CMAKE_SOURCE_DIR}/modules/storage/include
    ${CMAKE_SOURCE_DIR}/modules/recording/include
    ${CMAKE_SOURCE_DIR}/modules/networking/include
    ${CMAKE_SOURCE_DIR}/modules/streaming/include
    ${CMAKE_SOURCE_DIR}/modules/platform/include
    ${CMAKE_SOURCE_DIR}/modules/media/include
    ${CMAKE_SOURCE_DIR}/modules/ai/include
    ${CMAKE_SOURCE_DIR}/modules/upgrade/include
    ${CMAKE_SOURCE_DIR}/modules/events/include
    ${CMAKE_SOURCE_DIR}/modules/utils/include
)

target_link_libraries(ipcam_webserver
  PUBLIC
    ipcam_config
    ipcam_platform
    ipcam_media
    ipcam_streaming
    ipcam_networking
    ipcam_storage
    ipcam_recording
    ipcam_ai
    ipcam_upgrade
    ipcam_events
    ipcam_utils
  PRIVATE
    spdlog::spdlog
)

# Link CivetWeb if available
if(CIVETWEB_AVAILABLE)
  target_link_libraries(ipcam_webserver PUBLIC civetweb::civetweb-cpp)
  target_compile_definitions(ipcam_webserver PRIVATE CIVETWEB_ENABLED=1)
else()
  target_compile_definitions(ipcam_webserver PRIVATE CIVETWEB_ENABLED=0)
endif()

if(nlohmann_json_FOUND)
  target_link_libraries(ipcam_webserver PRIVATE nlohmann_json::nlohmann_json)
else()
  # Fallback: header-only library
  target_include_directories(ipcam_webserver PRIVATE /usr/include)
endif()

target_compile_features(ipcam_webserver PUBLIC cxx_std_17)

# Set compiler flags
target_compile_options(ipcam_webserver PRIVATE
  -Wall
  -Wextra
  -Wpedantic
)
