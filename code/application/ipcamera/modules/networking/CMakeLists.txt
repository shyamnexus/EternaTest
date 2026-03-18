add_library(ipcam_networking SHARED)
target_sources(ipcam_networking PRIVATE 
    src/network_manager.cpp
    src/ntp_manager.cpp
    src/ssl_manager.cpp
    src/nginx_manager.cpp
    src/smtp_manager.cpp
    src/snmp_manager.cpp
    src/upnp_manager.cpp
    src/mdns_responder.cpp
)
target_include_directories(ipcam_networking PUBLIC include)

target_compile_features(ipcam_networking PUBLIC cxx_std_17)

# Link config, utils, and storage modules
target_link_libraries(ipcam_networking PUBLIC 
    ipcam_config 
    ipcam_utils
    ipcam_storage  # For CredentialManager
)

# Link curl library for SMTP functionality (PUBLIC for shared library)
target_link_libraries(ipcam_networking PUBLIC curl)

# Link miniupnpc library for UPnP functionality (if found by modules.cmake)
if(IPCAM_MINIUPNPC_FOUND)
  target_link_libraries(ipcam_networking PUBLIC ${MINIUPNPC_LIBRARY})
  target_include_directories(ipcam_networking PRIVATE ${MINIUPNPC_INCLUDE_DIR})
else()
  # Fallback: try direct linking
  target_link_libraries(ipcam_networking PUBLIC miniupnpc)
endif()

if(BUILD_TESTING)
  add_subdirectory(tests)
endif()

# Install the shared library
install(TARGETS ipcam_networking
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)
