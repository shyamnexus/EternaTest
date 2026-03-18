#pragma once
#include <functional>
#include <atomic>
#include <thread>
#include <chrono>
#include <string>

namespace ipcam {
namespace platform {

/**
 * @brief Reset button configuration
 */
struct ResetButtonSettings {
    int gpio_pin = 12;                     // P_GPIO pin number (12 for P_GPIO12)
    int hold_duration_sec = 10;            // Seconds to hold for factory reset
    bool active_low = true;                // Button is active when GPIO is LOW
    bool enable_feedback = true;           // Enable LED feedback during reset sequence
    int debounce_ms = 50;                  // Debounce time in milliseconds
    int poll_interval_ms = 100;            // Polling interval for button state
};

/**
 * @brief Reset button controller with interrupt-based detection
 * 
 * Monitors P_GPIO12 for button press. When held for 10 seconds,
 * triggers a factory reset callback. Uses GPIO edge interrupt
 * for efficient detection.
 */
class ResetButton {
public:
    using ResetCallback = std::function<void()>;
    using ProgressCallback = std::function<void(int elapsed_seconds, int total_seconds)>;

    /**
     * @brief Get singleton instance
     */
    static ResetButton& Instance();

    /**
     * @brief Initialize the reset button monitor
     * @return true if initialization successful
     */
    bool Init();

    /**
     * @brief Shutdown the reset button monitor
     */
    void Shutdown();

    /**
     * @brief Check if the reset button module is initialized
     */
    bool IsInitialized() const { return initialized_; }

    /**
     * @brief Register callback for factory reset event
     * @param callback Function to call when reset is triggered
     */
    void SetResetCallback(ResetCallback callback);

    /**
     * @brief Register callback for reset progress (for LED feedback)
     * @param callback Function called every second during button hold
     */
    void SetProgressCallback(ProgressCallback callback);

    /**
     * @brief Update reset button settings
     * @param settings New settings to apply
     * @return true if settings were applied successfully
     */
    bool SetSettings(const ResetButtonSettings& settings);

    /**
     * @brief Get current settings
     */
    ResetButtonSettings GetSettings() const;

    /**
     * @brief Get current button state (for debugging)
     * @return true if button is currently pressed
     */
    bool IsButtonPressed() const;

    /**
     * @brief Get elapsed hold time in seconds
     * @return seconds the button has been held, 0 if not pressed
     */
    int GetHoldDuration() const;

    /**
     * @brief Get status information
     */
    struct Status {
        bool initialized;
        bool button_pressed;
        int hold_seconds;
        int required_seconds;
        std::string last_event;
    };
    Status GetStatus() const;

private:
    ResetButton() = default;
    ~ResetButton();
    
    // Prevent copying
    ResetButton(const ResetButton&) = delete;
    ResetButton& operator=(const ResetButton&) = delete;

    // GPIO operations
    bool SetupGPIO();
    bool CleanupGPIO();
    bool SetupInterrupt();
    bool ReadButtonState();
    int GetLinuxGpioNumber(int p_gpio_pin);
    
    // Thread function
    void MonitorLoop();
    void HandleButtonPress();
    void HandleButtonRelease();

    // Configuration
    ResetButtonSettings settings_;
    
    // State
    std::atomic<bool> initialized_{false};
    std::atomic<bool> running_{false};
    std::atomic<bool> button_pressed_{false};
    std::chrono::steady_clock::time_point press_start_time_;
    std::atomic<int> hold_seconds_{0};
    std::string last_event_;
    
    // Thread
    std::thread monitor_thread_;
    int gpio_fd_{-1};  // File descriptor for interrupt-based GPIO
    
    // Callbacks
    ResetCallback reset_callback_;
    ProgressCallback progress_callback_;
};

} // namespace platform
} // namespace ipcam
