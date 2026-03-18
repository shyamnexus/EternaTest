add_executable(rtsp_client src/main.cpp)
target_link_libraries(rtsp_client PRIVATE ipcam_utils)

target_compile_features(rtsp_client PRIVATE cxx_std_17)
