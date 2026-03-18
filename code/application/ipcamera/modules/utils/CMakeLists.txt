add_library(ipcam_utils SHARED)
target_sources(ipcam_utils PRIVATE
  src/utils.cpp
  src/audit_logger.cpp
)
target_include_directories(ipcam_utils PUBLIC include)

target_link_libraries(ipcam_utils
  PRIVATE
    spdlog::spdlog
)

target_compile_features(ipcam_utils PUBLIC cxx_std_17)

if(BUILD_TESTING)
  add_subdirectory(tests)
endif()

# Install the shared library
install(TARGETS ipcam_utils
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)
