cmake_minimum_required(VERSION 3.15)

# Option to enable/disable ONVIF module
option(IPCAMERA_ENABLE_ONVIF "Build ONVIF/WS-Discovery support" ON)
option(IPCAMERA_ENABLE_WEBSERVER "Build webserver module" ON)
option(IPCAMERA_ENABLE_EVENTS "Build event actions module" ON)

# Build utils first as it provides shared headers like result.h
add_subdirectory(utils)
add_subdirectory(platform)
add_subdirectory(media)
add_subdirectory(ai)
add_subdirectory(config)
add_subdirectory(networking)
add_subdirectory(streaming)
add_subdirectory(storage)
add_subdirectory(recording)
add_subdirectory(upgrade)

# Events module (depends on config, recording, storage, networking)
if(IPCAMERA_ENABLE_EVENTS)
    add_subdirectory(events)
endif()

if(IPCAMERA_ENABLE_WEBSERVER)
    add_subdirectory(webserver)
endif()

if(IPCAMERA_ENABLE_ONVIF)
    add_subdirectory(onvif)
endif()
