# Events Module - CMakeLists.txt

set(MODULE_NAME ipcam_events)

# Find nlohmann_json
find_package(nlohmann_json QUIET)

# Source files
set(SOURCES
    src/event_types.cpp
    src/event_rule.cpp
    src/event_manager.cpp
    src/action_handler.cpp
    src/actions/recording_action.cpp
    src/actions/snapshot_action.cpp
    src/actions/email_action.cpp
    src/actions/webhook_action.cpp
    src/actions/mqtt_action.cpp
    src/actions/ftp_action.cpp
    src/actions/io_action.cpp
)

# Header files
set(HEADERS
    include/ipcam/event_types.h
    include/ipcam/event_rule.h
    include/ipcam/event_manager.h
    include/ipcam/action_handler.h
)

# Create shared library (consistent with other modules)
add_library(${MODULE_NAME} SHARED ${SOURCES} ${HEADERS})

# Include directories
target_include_directories(${MODULE_NAME}
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# Find uuid library for UUID generation - prefer static to avoid runtime dependency
find_library(UUID_LIBRARY_STATIC NAMES libuuid.a
    PATHS ${CMAKE_SYSROOT}/usr/lib ${CMAKE_SYSROOT}/lib
    NO_DEFAULT_PATH
)
if(UUID_LIBRARY_STATIC)
    set(UUID_LIBRARY ${UUID_LIBRARY_STATIC})
    message(STATUS "Events module: Using static libuuid: ${UUID_LIBRARY_STATIC}")
else()
    find_library(UUID_LIBRARY NAMES uuid libuuid)
    message(STATUS "Events module: Using shared libuuid: ${UUID_LIBRARY}")
endif()

# Link dependencies
target_link_libraries(${MODULE_NAME}
    PUBLIC
        spdlog::spdlog
    PRIVATE
        ipcam_config
        ipcam_recording
        ipcam_storage
        ipcam_networking
        ipcam_platform
        ipcam_media
        ipcam_utils
        ${UUID_LIBRARY}
)

# Link nlohmann_json if found
if(nlohmann_json_FOUND)
    target_link_libraries(${MODULE_NAME} PUBLIC nlohmann_json::nlohmann_json)
endif()

# Optional ONVIF support
if(TARGET onvif)
    target_link_libraries(${MODULE_NAME} PRIVATE onvif)
    target_compile_definitions(${MODULE_NAME} PRIVATE ONVIF_ENABLED=1)
endif()

# Optional MQTT support (paho-mqtt-cpp)
find_package(PahoMqttCpp QUIET)
if(PahoMqttCpp_FOUND)
    target_compile_definitions(${MODULE_NAME} PRIVATE MQTT_ENABLED=1)
    target_link_libraries(${MODULE_NAME} PRIVATE PahoMqttCpp::paho-mqttpp3)
    message(STATUS "Events module: MQTT support enabled")
else()
    message(STATUS "Events module: MQTT support disabled (paho-mqtt-cpp not found)")
endif()

# Optional HTTP client for webhooks
# Using cpp-httplib (header-only)
target_compile_definitions(${MODULE_NAME} PRIVATE CPPHTTPLIB_OPENSSL_SUPPORT)

# Compiler options
target_compile_features(${MODULE_NAME} PUBLIC cxx_std_17)

# Install headers
install(DIRECTORY include/
    DESTINATION include
    FILES_MATCHING PATTERN "*.h"
)

# Tests
if(BUILD_TESTING)
    add_subdirectory(tests)
endif()
