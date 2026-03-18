# tests for utils module
add_executable(test_utils ../src/utils.cpp)
add_test(NAME utils-smoke COMMAND test_utils)
