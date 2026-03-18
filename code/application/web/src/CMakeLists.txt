cmake_minimum_required(VERSION 3.10)
project(ipcwebbackend LANGUAGES CXX)

if(COMMAND cmake_policy)
  cmake_policy(SET CMP0003 NEW)
endif()

add_library(minilog STATIC
    ../thirdparty/minilog/minilog.cpp
    ../thirdparty/minilog/minilog.h
)

add_library(restapi
    STATIC
    http_response.cpp
    rest_api.cpp
    common.cpp
    network_api.cpp
    certificate_utils.cpp
    storage_api.cpp
    video_api.cpp
    audio_api.cpp
    stream_api.cpp
    image_api.cpp
    system_api.cpp
    osd_api.cpp
    roi_api.cpp
    event_api.cpp
    peripherals_api.cpp
    ipcweb_client/socket.cpp
    ipcweb_client/client.cpp
    analytics_api.cpp
    Logger.cpp
)

target_compile_features(restapi PRIVATE cxx_std_14)
target_compile_options(restapi PRIVATE -fexceptions)
# Set global include and library directories (user pattern)
set(PROJECT_INCLUDE_DIRS
    $ENV{LIBRARY_DIR}/include
    $ENV{EXTERNAL}/$ENV{INSTALL_DIR}/include
    $ENV{INCLUDE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../include
    ${CMAKE_CURRENT_SOURCE_DIR}/../thirdparty/jwt-cpp/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../thirdparty/minilog
    ${CMAKE_CURRENT_SOURCE_DIR}
    $ENV{LIBRARY_DIR}/external/__install/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../thirdparty/cgicc/__install/include
)
set(PROJECT_LIBRARY_DIRS
    $ENV{LIBRARY_DIR}
    $ENV{EXTERNAL}/$ENV{INSTALL_DIR}/lib
    $ENV{LIBRARY_DIR}/external/__install/lib
    ${CMAKE_CURRENT_SOURCE_DIR}/../thirdparty/cgicc/__install/lib
)
include_directories(${PROJECT_INCLUDE_DIRS})
link_directories(${PROJECT_LIBRARY_DIRS})

add_executable(entry.cgi
    main.cpp)

target_compile_features(entry.cgi PRIVATE cxx_std_14)
add_definitions(-DUSE_RKIPC)
target_include_directories(entry.cgi PRIVATE
    ${SQLCIPHER_ABS_INCLUDE}
    ${OPENSSL_ABS_INCLUDE}
    $ENV{LIBRARY_DIR}/external/__install/include
)
## Prefer static libcgicc.a from local thirdparty install if available.
## Use find_file to locate the exact static archive and create an IMPORTED STATIC target.
find_file(CGICC_STATIC_PATH
    NAMES libcgicc.a
    PATHS ${CMAKE_CURRENT_SOURCE_DIR}/../thirdparty/cgicc/__install/lib
)

if(CGICC_STATIC_PATH)
    message(STATUS "Found static cgicc archive: ${CGICC_STATIC_PATH}")
    # Create an imported STATIC target so consumers can link against it portably
    add_library(cgicc_static STATIC IMPORTED)
    set_target_properties(cgicc_static PROPERTIES
        IMPORTED_LOCATION "${CGICC_STATIC_PATH}"
    )
    # Ensure the include directory is known to consumers of the imported target
    set_property(TARGET cgicc_static PROPERTY INTERFACE_INCLUDE_DIRECTORIES
        "${CMAKE_CURRENT_SOURCE_DIR}/../thirdparty/cgicc/__install/include"
    )
    target_link_libraries(entry.cgi PRIVATE restapi pthread cgicc_static minilog ssl crypto sqlite3)
else()
    message(STATUS "Static libcgicc.a not found; linking against cgicc (shared) if available")
    target_link_libraries(entry.cgi PRIVATE restapi pthread cgicc minilog ssl crypto sqlite3)
endif()
# If you want to use the shared library, replace libsqlite3.a with libsqlite3.so.3.49.2

option(IPCWEBBACKEND_INSTALL_ON_OEM_PARTITION "compile: install on oem partition" OFF)
if(IPCWEBBACKEND_INSTALL_ON_OEM_PARTITION)
install(TARGETS entry.cgi RUNTIME DESTINATION /www/cgi-bin)
else()
install(TARGETS entry.cgi RUNTIME DESTINATION www/cgi-bin)
endif()

if (${USE_RKIPC})
# install(DIRECTORY ${PROJECT_SOURCE_DIR}/www-rkipc/ DESTINATION www)
install(DIRECTORY ${PROJECT_SOURCE_DIR}/dev/ DESTINATION www)
endif()
