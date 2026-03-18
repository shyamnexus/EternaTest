add_executable(capture_cli src/main.cpp)
target_link_libraries(capture_cli PRIVATE ipcam_utils)

target_compile_features(capture_cli PRIVATE cxx_std_17)
