cmake_minimum_required(VERSION 3.10)
project(ipcam_storage)

# Include the ipcam modules system if available
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../../cmake/modules.cmake")
    message(STATUS "Including ipcam modules.cmake")
    include("${CMAKE_CURRENT_SOURCE_DIR}/../../cmake/modules.cmake")
else()
    message(STATUS "modules.cmake not found at ${CMAKE_CURRENT_SOURCE_DIR}/../../cmake/modules.cmake")
endif()

add_library(ipcam_storage SHARED)
target_sources(ipcam_storage PRIVATE 
    src/storage.cpp
    src/credential_manager.cpp
    src/nas_manager.cpp
    src/ftp_manager.cpp
)
target_include_directories(ipcam_storage PUBLIC include)

target_compile_features(ipcam_storage PUBLIC cxx_std_17)

# Link to config module
target_link_libraries(ipcam_storage PUBLIC ipcam_config)

# Link external dependencies via ipcam system
# Note: ipcam_link_externals already provides sqlite3, ssl, crypto
if(COMMAND ipcam_link_externals)
    message(STATUS "Using ipcam_link_externals for ipcam_storage")
    ipcam_link_externals(ipcam_storage)
else()
    # Fallback if modules.cmake not available (standalone build)
    message(STATUS "ipcam_link_externals not available, linking manually")
    target_link_libraries(ipcam_storage PRIVATE sqlite3 ssl crypto curl)
endif()

# FTP manager requires libcurl
find_package(CURL QUIET)
if(CURL_FOUND)
    target_link_libraries(ipcam_storage PRIVATE CURL::libcurl)
else()
    target_link_libraries(ipcam_storage PRIVATE curl)
endif()

if(BUILD_TESTING)
  add_subdirectory(tests)
endif()

# Install the shared library
install(TARGETS ipcam_storage
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)
