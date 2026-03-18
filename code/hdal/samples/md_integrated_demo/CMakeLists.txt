cmake_minimum_required(VERSION 3.10)
project(md_integrated_demo C)

# Cross-compilation settings
set(CMAKE_C_COMPILER aarch64-ca53-linux-gnueabi-gcc)
set(CMAKE_CXX_COMPILER aarch64-ca53-linux-gnueabi-g++)

# Build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Compiler flags
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -O2")
set(CMAKE_C_FLAGS_DEBUG "-g -O0")
set(CMAKE_C_FLAGS_RELEASE "-O2 -DNDEBUG")

# SDK root path
set(SDK_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../../../..")
set(HDAL_ROOT "${SDK_ROOT}/code/hdal")
set(LIB_ROOT "${SDK_ROOT}/code/lib")
set(OUTPUT_ROOT "${SDK_ROOT}/output")

# Include directories
include_directories(
    ${HDAL_ROOT}/include
    ${HDAL_ROOT}/vendor/include
    ${HDAL_ROOT}/vendor/media/include
    ${LIB_ROOT}/include
    ${LIB_ROOT}/libmd
    ${OUTPUT_ROOT}/include
)

# Library directories
link_directories(
    ${HDAL_ROOT}/vendor/output
    ${LIB_ROOT}/output
    ${OUTPUT_ROOT}/lib
)

# Source files
set(SOURCES
    md_integrated_demo.c
)

# Create executable
add_executable(md_integrated_demo ${SOURCES})

# Link libraries
target_link_libraries(md_integrated_demo
    hdal
    vendor_media
    nvtmd
    hd_common
    pthread
    m
    dl
    rt
)

# Installation
install(TARGETS md_integrated_demo
    RUNTIME DESTINATION bin
)

# Custom target to push to device
add_custom_target(push
    COMMAND adb push ${CMAKE_CURRENT_BINARY_DIR}/md_integrated_demo /mnt/sd/
    COMMAND adb shell chmod +x /mnt/sd/md_integrated_demo
    DEPENDS md_integrated_demo
    COMMENT "Pushing md_integrated_demo to device..."
)

# Custom target to run on device
add_custom_target(run
    COMMAND adb shell "cd /mnt/sd && ./md_integrated_demo 30 2"
    DEPENDS push
    COMMENT "Running md_integrated_demo on device..."
)
