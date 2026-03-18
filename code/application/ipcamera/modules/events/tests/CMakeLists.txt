# Events Module Tests CMakeLists.txt
cmake_minimum_required(VERSION 3.14)
project(events_tests CXX)

# If built standalone, set up the project
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    set(STANDALONE_BUILD TRUE)
    set(CMAKE_CXX_STANDARD 17)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
    
    # For standalone build, we need to include the parent directory
    # and build the events library from source
    message(STATUS "Building events tests in standalone mode")
    
    # Find required packages
    find_package(Threads REQUIRED)
    
    # Check if we should skip full library tests
    option(MINIMAL_TESTS "Build only minimal tests without full library dependencies" ON)
endif()

# Require GTest
find_package(GTest QUIET)

if(NOT GTest_FOUND)
    include(FetchContent)
    FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG        v1.14.0
    )
    FetchContent_MakeAvailable(googletest)
endif()

# Enable testing
enable_testing()

# If we have the full events library, build full tests
if(TARGET ipcam_events)
    # For cross-compilation, we need to link against HDAL libraries
    if(CMAKE_CROSSCOMPILING)
        # Set up library search paths for linking using absolute paths
        # tests is in code/application/ipcamera/modules/events/tests
        # hdal is in code/hdal (5 levels up, then into hdal)
        get_filename_component(EVENTS_TESTS_DIR "${CMAKE_CURRENT_SOURCE_DIR}" ABSOLUTE)
        get_filename_component(HDAL_OUTPUT_PATH "${EVENTS_TESTS_DIR}/../../../../../hdal/output" ABSOLUTE)
        get_filename_component(HDAL_VENDOR_PATH "${EVENTS_TESTS_DIR}/../../../../../hdal/vendor/output" ABSOLUTE)
        get_filename_component(ROOTFS_LIB_PATH "${EVENTS_TESTS_DIR}/../../../../../BSP/root-fs/rootfs/usr/lib" ABSOLUTE)
        
        message(STATUS "Events tests: Looking for HDAL in ${HDAL_OUTPUT_PATH}")
        message(STATUS "Events tests: Looking for vendor in ${HDAL_VENDOR_PATH}")
        
        # Collect all HDAL libraries using direct paths
        set(HDAL_LINK_LIBS)
        
        # Check for libhdal.so
        if(EXISTS "${HDAL_OUTPUT_PATH}/libhdal.so")
            list(APPEND HDAL_LINK_LIBS "${HDAL_OUTPUT_PATH}/libhdal.so")
            message(STATUS "Events tests: Found libhdal.so at ${HDAL_OUTPUT_PATH}/libhdal.so")
        endif()
        
        # Check for vendor libraries
        if(EXISTS "${HDAL_VENDOR_PATH}/libvendor_isp.so")
            list(APPEND HDAL_LINK_LIBS "${HDAL_VENDOR_PATH}/libvendor_isp.so")
            message(STATUS "Events tests: Found libvendor_isp.so at ${HDAL_VENDOR_PATH}/libvendor_isp.so")
        endif()
        
        if(EXISTS "${HDAL_VENDOR_PATH}/libvendor_media.so")
            list(APPEND HDAL_LINK_LIBS "${HDAL_VENDOR_PATH}/libvendor_media.so")
            message(STATUS "Events tests: Found libvendor_media.so at ${HDAL_VENDOR_PATH}/libvendor_media.so")
        endif()
        
        if(EXISTS "${HDAL_VENDOR_PATH}/libprebuilt_isp.so")
            list(APPEND HDAL_LINK_LIBS "${HDAL_VENDOR_PATH}/libprebuilt_isp.so")
            message(STATUS "Events tests: Found libprebuilt_isp.so at ${HDAL_VENDOR_PATH}/libprebuilt_isp.so")
        endif()
        
        # VOS library (needed by prebuilt_isp)
        get_filename_component(VOS_OUTPUT_PATH "${EVENTS_TESTS_DIR}/../../../../../vos/output" ABSOLUTE)
        if(EXISTS "${VOS_OUTPUT_PATH}/libvos.so")
            list(APPEND HDAL_LINK_LIBS "${VOS_OUTPUT_PATH}/libvos.so")
            message(STATUS "Events tests: Found libvos.so at ${VOS_OUTPUT_PATH}/libvos.so")
        endif()
        
        if(NOT HDAL_LINK_LIBS)
            message(WARNING "Events tests: HDAL libraries not found. Tests will only be runnable on target device.")
        endif()
    endif()
    
    # Common test utilities
    add_library(events_test_utils STATIC
        utils/mock_handlers.cpp
        utils/test_helpers.cpp
    )

    target_include_directories(events_test_utils PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/utils
        ${CMAKE_CURRENT_SOURCE_DIR}/../include
    )

    target_link_libraries(events_test_utils PUBLIC
        ipcam_events
        GTest::gtest
        GTest::gmock
    )

    # ============================================================================
    # Unit Tests (can be built individually)
    # ============================================================================

    # Test Event Types
    add_executable(test_event_types
        unit/test_event_types.cpp
    )
    target_link_libraries(test_event_types PRIVATE
        events_test_utils
        GTest::gtest_main
    )
    add_test(NAME EventTypesTest COMMAND test_event_types)

    # Test Event Rules
    add_executable(test_event_rules
        unit/test_event_rules.cpp
    )
    target_link_libraries(test_event_rules PRIVATE
        events_test_utils
        GTest::gtest_main
    )
    add_test(NAME EventRulesTest COMMAND test_event_rules)

    # Test Event Manager
    add_executable(test_event_manager
        unit/test_event_manager.cpp
    )
    target_link_libraries(test_event_manager PRIVATE
        events_test_utils
        GTest::gtest_main
    )
    add_test(NAME EventManagerTest COMMAND test_event_manager)

    # Test Action Handlers
    add_executable(test_action_handlers
        unit/test_action_handlers.cpp
    )
    target_link_libraries(test_action_handlers PRIVATE
        events_test_utils
        GTest::gtest_main
    )
    add_test(NAME ActionHandlersTest COMMAND test_action_handlers)

    # ============================================================================
    # Integration Tests
    # ============================================================================

    # Full event flow test
    add_executable(test_event_flow
        integration/test_event_flow.cpp
    )
    target_link_libraries(test_event_flow PRIVATE
        events_test_utils
        GTest::gtest_main
    )
    add_test(NAME EventFlowIntegrationTest COMMAND test_event_flow)

    # Rule engine integration test
    add_executable(test_rule_engine
        integration/test_rule_engine.cpp
    )
    target_link_libraries(test_rule_engine PRIVATE
        events_test_utils
        GTest::gtest_main
    )
    add_test(NAME RuleEngineIntegrationTest COMMAND test_rule_engine)

    # ============================================================================
    # Stress Tests
    # ============================================================================

    add_executable(test_event_stress
        stress/test_event_stress.cpp
    )
    target_link_libraries(test_event_stress PRIVATE
        events_test_utils
        GTest::gtest_main
    )
    add_test(NAME EventStressTest COMMAND test_event_stress)

    # ============================================================================
    # Standalone Test Runner (self-contained binary)
    # For target device execution
    # ============================================================================

    # Note: This test runner is designed to be run on the target device
    # where HDAL and other platform libraries are available.
    # It can be deployed alongside the main ipcamera application.

    add_executable(events_test_runner
        test_runner_main.cpp
        unit/test_event_types.cpp
        unit/test_event_rules.cpp
        unit/test_event_manager.cpp
        unit/test_action_handlers.cpp
        integration/test_event_flow.cpp
        integration/test_rule_engine.cpp
    )

    target_link_libraries(events_test_runner PRIVATE
        events_test_utils
        GTest::gtest
        GTest::gmock
        ${HDAL_LINK_LIBS}
    )

    # Install test binaries
    install(TARGETS events_test_runner
        RUNTIME DESTINATION bin/tests
    )

    # Custom target to run all events tests
    add_custom_target(run_events_tests
        COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
        DEPENDS test_event_types test_event_rules test_event_manager 
                test_action_handlers test_event_flow test_rule_engine
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    )

else()
    # Standalone build without full library - minimal tests only
    message(STATUS "Building minimal events tests (standalone mode)")
    message(STATUS "Full integration tests require building from main ipcamera project")
endif()
