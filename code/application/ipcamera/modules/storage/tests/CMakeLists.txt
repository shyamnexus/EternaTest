cmake_minimum_required(VERSION 3.16)
project(storage_tests CXX)

# Set C++17 standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Check if we're cross-compiling
if(CMAKE_CROSSCOMPILING)
    message(STATUS "Cross-compiling for ${CMAKE_SYSTEM_PROCESSOR}")
endif()

# Include directories
include_directories(../include)

# Create test executable - build storage.cpp directly with tests
add_executable(test_storage storage_test.cpp ../src/storage.cpp)

# Set target properties
target_compile_features(test_storage PRIVATE cxx_std_17)
target_compile_options(test_storage PRIVATE -Wall -Wextra)

# Manual SQLite3 linking for cross-compile
if(DEFINED ENV{LIBRARY_DIR})
    set(SQLITE3_LIBRARY_DIR "$ENV{LIBRARY_DIR}")
    message(STATUS "Using SQLite3 library from environment: ${SQLITE3_LIBRARY_DIR}")
    
    target_include_directories(test_storage PRIVATE "${SQLITE3_LIBRARY_DIR}/external/__install/include")
    target_link_libraries(test_storage PRIVATE "${SQLITE3_LIBRARY_DIR}/external/__install/lib/libsqlite3.a")
else()
    # System fallback
    find_library(SQLITE3_LIBRARIES NAMES sqlite3)
    if(NOT SQLITE3_LIBRARIES)
        message(FATAL_ERROR "SQLite3 library not found")
    endif()
    target_link_libraries(test_storage PRIVATE ${SQLITE3_LIBRARIES})
endif()

# Add pthread for std::mutex
find_package(Threads REQUIRED)
target_link_libraries(test_storage PRIVATE Threads::Threads)

# Add filesystem library if needed (GCC < 9)
target_link_libraries(test_storage PRIVATE stdc++fs)

# Add test
add_test(NAME storage-smoke COMMAND test_storage)

message(STATUS "Storage tests configured successfully")
message(STATUS "Test executable: test_storage")