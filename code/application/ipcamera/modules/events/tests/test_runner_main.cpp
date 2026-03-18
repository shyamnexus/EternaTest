/**
 * @file test_runner_main.cpp
 * @brief Main entry point for the self-contained test binary
 * 
 * This creates a standalone test executable that can be run on the device
 * to verify the event system functionality.
 */

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <iostream>
#include <cstring>

/**
 * @brief Print test banner
 */
void PrintBanner() {
    std::cout << "\n";
    std::cout << "╔══════════════════════════════════════════════════════════╗\n";
    std::cout << "║          IPCamera Events Module Test Suite               ║\n";
    std::cout << "╠══════════════════════════════════════════════════════════╣\n";
    std::cout << "║  Unit Tests:                                             ║\n";
    std::cout << "║    - Event Types                                         ║\n";
    std::cout << "║    - Event Rules                                         ║\n";
    std::cout << "║    - Event Manager                                       ║\n";
    std::cout << "║    - Action Handlers                                     ║\n";
    std::cout << "║  Integration Tests:                                      ║\n";
    std::cout << "║    - Event Flow                                          ║\n";
    std::cout << "║    - Rule Engine                                         ║\n";
    std::cout << "╚══════════════════════════════════════════════════════════╝\n";
    std::cout << "\n";
}

/**
 * @brief Print usage information
 */
void PrintUsage(const char* program_name) {
    std::cout << "Usage: " << program_name << " [OPTIONS]\n";
    std::cout << "\nOptions:\n";
    std::cout << "  --help              Show this help message\n";
    std::cout << "  --gtest_filter=PATTERN  Run only tests matching PATTERN\n";
    std::cout << "  --gtest_list_tests  List all available tests\n";
    std::cout << "  --gtest_repeat=N    Repeat tests N times\n";
    std::cout << "  --gtest_shuffle     Randomize test order\n";
    std::cout << "  --gtest_output=xml:FILE  Generate XML report\n";
    std::cout << "\nExamples:\n";
    std::cout << "  " << program_name << "                     # Run all tests\n";
    std::cout << "  " << program_name << " --gtest_filter='*Manager*'  # Run manager tests\n";
    std::cout << "  " << program_name << " --gtest_filter='*Integration*'  # Run integration tests\n";
    std::cout << "  " << program_name << " --gtest_list_tests   # List all tests\n";
    std::cout << "\n";
}

/**
 * @brief Custom event listener for better output
 */
class EventsTestListener : public ::testing::EmptyTestEventListener {
public:
    void OnTestProgramStart(const ::testing::UnitTest& unit_test) override {
        std::cout << "[==========] Running " << unit_test.test_to_run_count() 
                  << " tests from " << unit_test.test_suite_to_run_count() 
                  << " test suites.\n";
    }

    void OnTestSuiteStart(const ::testing::TestSuite& test_suite) override {
        std::cout << "[----------] " << test_suite.test_to_run_count() 
                  << " tests from " << test_suite.name() << "\n";
    }

    void OnTestStart(const ::testing::TestInfo& test_info) override {
        std::cout << "[ RUN      ] " << test_info.test_suite_name() << "." 
                  << test_info.name() << "\n";
    }

    void OnTestEnd(const ::testing::TestInfo& test_info) override {
        if (test_info.result()->Passed()) {
            std::cout << "[       OK ] " << test_info.test_suite_name() << "." 
                      << test_info.name() << " (" 
                      << test_info.result()->elapsed_time() << " ms)\n";
        } else {
            std::cout << "[  FAILED  ] " << test_info.test_suite_name() << "." 
                      << test_info.name() << " (" 
                      << test_info.result()->elapsed_time() << " ms)\n";
        }
    }

    void OnTestSuiteEnd(const ::testing::TestSuite& test_suite) override {
        std::cout << "[----------] " << test_suite.test_to_run_count() 
                  << " tests from " << test_suite.name() 
                  << " (" << test_suite.elapsed_time() << " ms total)\n\n";
    }

    void OnTestProgramEnd(const ::testing::UnitTest& unit_test) override {
        std::cout << "[==========] " << unit_test.test_to_run_count() 
                  << " tests ran. (" << unit_test.elapsed_time() << " ms total)\n";
        std::cout << "[  PASSED  ] " << unit_test.successful_test_count() << " tests.\n";
        
        if (unit_test.failed_test_count() > 0) {
            std::cout << "[  FAILED  ] " << unit_test.failed_test_count() << " tests:\n";
            for (int i = 0; i < unit_test.total_test_suite_count(); ++i) {
                const auto* suite = unit_test.GetTestSuite(i);
                for (int j = 0; j < suite->total_test_count(); ++j) {
                    const auto* test = suite->GetTestInfo(j);
                    if (test->result()->Failed()) {
                        std::cout << "[  FAILED  ]   " << suite->name() << "." 
                                  << test->name() << "\n";
                    }
                }
            }
        }
    }
};

/**
 * @brief Main function
 */
int main(int argc, char** argv) {
    // Check for help flag
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0) {
            PrintUsage(argv[0]);
            return 0;
        }
    }
    
    PrintBanner();
    
    // Initialize Google Test
    ::testing::InitGoogleTest(&argc, argv);
    ::testing::InitGoogleMock(&argc, argv);
    
    // Replace default listener with custom one (optional)
    // Uncomment to use custom listener:
    // ::testing::TestEventListeners& listeners = 
    //     ::testing::UnitTest::GetInstance()->listeners();
    // delete listeners.Release(listeners.default_result_printer());
    // listeners.Append(new EventsTestListener);
    
    // Run tests
    int result = RUN_ALL_TESTS();
    
    // Print summary
    std::cout << "\n";
    if (result == 0) {
        std::cout << "╔══════════════════════════════════════════════════════════╗\n";
        std::cout << "║                  ALL TESTS PASSED ✓                      ║\n";
        std::cout << "╚══════════════════════════════════════════════════════════╝\n";
    } else {
        std::cout << "╔══════════════════════════════════════════════════════════╗\n";
        std::cout << "║               SOME TESTS FAILED ✗                        ║\n";
        std::cout << "╚══════════════════════════════════════════════════════════╝\n";
    }
    std::cout << "\n";
    
    return result;
}
