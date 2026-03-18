ipcam_add_library(ipcam_config SHARED)
target_sources(ipcam_config PRIVATE 
  src/config.cpp
  src/user_manager.cpp
)

# Link external dependencies automatically
ipcam_link_externals(ipcam_config)

# Link to ipcam_utils for Result type
target_link_libraries(ipcam_config PUBLIC ipcam_utils)

# Use nlohmann/json (header-only, already available via system)
find_package(nlohmann_json QUIET)
if(nlohmann_json_FOUND)
  target_link_libraries(ipcam_config PUBLIC nlohmann_json::nlohmann_json)
else()
  # Header-only fallback
  target_include_directories(ipcam_config PRIVATE /usr/include)
endif()

# Add SQLite3 for user database
# Note: Use library name only to avoid embedding absolute paths in cross-compilation
# The library paths are already set via link_directories in modules.cmake
target_link_libraries(ipcam_config PUBLIC sqlite3)

# Add OpenSSL for password hashing
# Note: Use library names only to avoid embedding absolute paths
target_link_libraries(ipcam_config PUBLIC ssl crypto)

if(IPCAMERA_BUILD_TESTS)
  add_subdirectory(tests)
endif()

# Install the shared library
install(TARGETS ipcam_config
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)
