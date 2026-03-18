cmake_minimum_required(VERSION 3.10)
project(ipcam_recording)

# Include the ipcam modules system if available
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../../cmake/modules.cmake")
    message(STATUS "Including ipcam modules.cmake for recording")
    include("${CMAKE_CURRENT_SOURCE_DIR}/../../cmake/modules.cmake")
else()
    message(STATUS "modules.cmake not found at ${CMAKE_CURRENT_SOURCE_DIR}/../../cmake/modules.cmake")
endif()

add_library(ipcam_recording SHARED)

target_sources(ipcam_recording PRIVATE 
    src/recording.cpp
    src/schedule_manager.cpp
    src/mp4_recorder.cpp
)

target_include_directories(ipcam_recording PUBLIC include)

# HDAL include path - required for video encoder access
if(DEFINED ENV{NVT_HDAL_DIR})
    target_include_directories(ipcam_recording PRIVATE $ENV{NVT_HDAL_DIR}/include)
else()
    # Fallback to relative path from SDK root
    target_include_directories(ipcam_recording PRIVATE 
        ${CMAKE_CURRENT_SOURCE_DIR}/../../../../hdal/include
    )
endif()

target_compile_features(ipcam_recording PUBLIC cxx_std_17)

# Link to required modules
target_link_libraries(ipcam_recording PUBLIC 
    ipcam_config
    ipcam_storage
    ipcam_platform
    ipcam_media
    ipcam_streaming
)

# Link external dependencies via ipcam system
# Requires: OpenSSL (for AES encryption), pthread (for threading)
if(COMMAND ipcam_link_externals)
    message(STATUS "Using ipcam_link_externals for ipcam_recording")
    ipcam_link_externals(ipcam_recording)
else()
    # Fallback if modules.cmake not available (standalone build)
    message(STATUS "ipcam_link_externals not available, linking manually")
    find_package(OpenSSL REQUIRED)
    target_link_libraries(ipcam_recording PRIVATE 
        OpenSSL::SSL 
        OpenSSL::Crypto
        pthread
    )
endif()

# Find and link spdlog for logging (header-only in SDK)
find_package(spdlog QUIET CONFIG)
if(spdlog_FOUND AND TARGET spdlog::spdlog_header_only)
    target_link_libraries(ipcam_recording PRIVATE spdlog::spdlog_header_only)
elseif(spdlog_FOUND AND TARGET spdlog::spdlog)
    # Check if it's header-only
    get_target_property(_spdlog_type spdlog::spdlog TYPE)
    if(_spdlog_type STREQUAL "INTERFACE_LIBRARY")
        target_link_libraries(ipcam_recording PRIVATE spdlog::spdlog)
    else()
        # Fall back to header-only include path
        target_include_directories(ipcam_recording PRIVATE ${SPDLOG_INCLUDE_DIRS})
        target_compile_definitions(ipcam_recording PRIVATE SPDLOG_HEADER_ONLY=1 FMT_HEADER_ONLY=1)
    endif()
else()
    # Try pkg-config for include path only (header-only library)
    find_package(PkgConfig QUIET)
    if(PkgConfig_FOUND)
        pkg_check_modules(SPDLOG spdlog)
        if(SPDLOG_FOUND)
            target_include_directories(ipcam_recording PRIVATE ${SPDLOG_INCLUDE_DIRS})
            # Don't link libraries - spdlog is header-only in this SDK
            target_compile_definitions(ipcam_recording PRIVATE SPDLOG_HEADER_ONLY=1 FMT_HEADER_ONLY=1)
        endif()
    endif()
endif()

# Find and link nlohmann_json
find_package(nlohmann_json QUIET)
if(nlohmann_json_FOUND)
    target_link_libraries(ipcam_recording PRIVATE nlohmann_json::nlohmann_json)
endif()

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()

# Install the shared library
install(TARGETS ipcam_recording
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)

# Install headers
install(DIRECTORY include/ipcam
    DESTINATION include
    FILES_MATCHING PATTERN "*.h"
)
