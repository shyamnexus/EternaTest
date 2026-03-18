# tests for ai module
add_executable(test_ai ../src/ai.cpp)
add_test(NAME ai-smoke COMMAND test_ai)
