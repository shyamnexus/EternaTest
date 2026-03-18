# tests for media module
add_executable(test_media ../src/media.cpp)
add_test(NAME media-smoke COMMAND test_media)

# MediaHub ring buffer and consumer unit tests
add_executable(test_media_hub test_media_hub.cpp ../src/media_hub.cpp)
target_include_directories(test_media_hub PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../include)
target_compile_features(test_media_hub PRIVATE cxx_std_17)

# MediaHub test does NOT require HDAL - disable HDAL pipeline for unit tests
target_compile_definitions(test_media_hub PRIVATE HDAL_PIPELINE_ENABLED=0)

# Link spdlog for logging
find_package(spdlog QUIET CONFIG)
if(spdlog_FOUND AND TARGET spdlog::spdlog_header_only)
    target_link_libraries(test_media_hub PRIVATE spdlog::spdlog_header_only)
elseif(spdlog_FOUND AND TARGET spdlog::spdlog)
    target_link_libraries(test_media_hub PRIVATE spdlog::spdlog)
else()
    find_package(PkgConfig QUIET)
    if(PkgConfig_FOUND)
        pkg_check_modules(SPDLOG spdlog)
        if(SPDLOG_FOUND)
            target_include_directories(test_media_hub PRIVATE ${SPDLOG_INCLUDE_DIRS})
            target_compile_definitions(test_media_hub PRIVATE SPDLOG_HEADER_ONLY=1 FMT_HEADER_ONLY=1)
        endif()
    endif()
endif()

target_link_libraries(test_media_hub PRIVATE pthread)
add_test(NAME media-hub-ring-buffer COMMAND test_media_hub)
