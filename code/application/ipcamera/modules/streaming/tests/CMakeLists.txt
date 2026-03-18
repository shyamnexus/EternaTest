# tests for streaming module
add_executable(test_streaming ../src/streaming.cpp)
add_test(NAME streaming-smoke COMMAND test_streaming)
