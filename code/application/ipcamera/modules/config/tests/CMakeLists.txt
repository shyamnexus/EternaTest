# tests for config module
add_executable(test_config_loader test_loader.cpp)
target_link_libraries(test_config_loader PRIVATE ipcam_config)
if(TARGET yaml-cpp)
	target_link_libraries(test_config_loader PRIVATE yaml-cpp)
endif()
add_test(NAME config-loader-smoke COMMAND test_config_loader)
