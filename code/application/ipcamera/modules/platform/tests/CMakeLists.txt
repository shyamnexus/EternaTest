# tests for platform module
add_executable(test_platform ../src/platform.cpp)
add_test(NAME platform-smoke COMMAND test_platform)
