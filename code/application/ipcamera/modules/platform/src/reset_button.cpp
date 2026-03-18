#include "ipcam/reset_button.h"
#include "ipcam/config.h"
#include <spdlog/spdlog.h>
#include <fstream>
#include <mutex>
#include <poll.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>

namespace ipcam {
namespace platform {

namespace {
    std::mutex g_mutex;
    
    // GPIO sysfs paths
    const char* GPIO_EXPORT_PATH = "/sys/class/gpio/export";
    const char* GPIO_UNEXPORT_PATH = "/sys/class/gpio/unexport";
    
    // P_GPIO offset for Linux GPIO number
    // P_GPIO(pin) = pin + 0x20 = pin + 32
    constexpr int P_GPIO_OFFSET = 32;
    
    // Helper to write string to sysfs file
    bool WriteToFile(const std::string& path, const std::string& value) {
        std::ofstream file(path);
        if (!file.is_open()) {
            spdlog::error("Failed to open {} for writing", path);
            return false;
        }
        file << value;
        file.close();
        return file.good();
    }
    
    // Helper to read string from sysfs file
    std::string ReadFromFile(const std::string& path) {
        std::ifstream file(path);
        if (!file.is_open()) {
            return "";
        }
        std::string value;
        file >> value;
        return value;
    }
    
    // Get current timestamp as string
    std::string GetTimestamp() {
        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);
        char buf[64];
        strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", std::localtime(&time));
        return std::string(buf);
    }
}

// ============================================================================
// Singleton Instance
// ============================================================================
ResetButton& ResetButton::Instance() {
    static ResetButton instance;
    return instance;
}

ResetButton::~ResetButton() {
    Shutdown();
}

// ============================================================================
// GPIO Operations
// ============================================================================
int ResetButton::GetLinuxGpioNumber(int p_gpio_pin) {
    return p_gpio_pin + P_GPIO_OFFSET;
}

bool ResetButton::SetupGPIO() {
    int linux_gpio = GetLinuxGpioNumber(settings_.gpio_pin);
    std::string gpio_path = "/sys/class/gpio/gpio" + std::to_string(linux_gpio);
    
    spdlog::info("Reset button: Setting up GPIO {} (Linux GPIO {})", 
                 settings_.gpio_pin, linux_gpio);
    
    // Check if GPIO is already exported
    struct stat st;
    if (stat(gpio_path.c_str(), &st) != 0) {
        // Export the GPIO
        if (!WriteToFile(GPIO_EXPORT_PATH, std::to_string(linux_gpio))) {
            spdlog::error("Failed to export GPIO {}", linux_gpio);
            return false;
        }
        // Wait for sysfs to create the files
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    
    // Set direction to input
    std::string direction_path = gpio_path + "/direction";
    if (!WriteToFile(direction_path, "in")) {
        spdlog::error("Failed to set GPIO {} direction to input", linux_gpio);
        return false;
    }
    
    spdlog::debug("Reset button: GPIO {} configured as input", linux_gpio);
    return true;
}

bool ResetButton::SetupInterrupt() {
    // NOTE: Interrupt mode disabled due to GPIO noise issues
    // Using polling-only mode for reliable debouncing
    // The GPIO may be floating without proper external pull-up resistor
    
    int linux_gpio = GetLinuxGpioNumber(settings_.gpio_pin);
    spdlog::info("Reset button: Using polling mode for GPIO {} (interrupt mode disabled)", linux_gpio);
    
    // Don't set active_low in sysfs - we handle it in software for clarity
    // Don't open gpio_fd_ - we'll use pure polling
    gpio_fd_ = -1;
    
    return true;
}

bool ResetButton::CleanupGPIO() {
    if (gpio_fd_ >= 0) {
        close(gpio_fd_);
        gpio_fd_ = -1;
    }
    
    int linux_gpio = GetLinuxGpioNumber(settings_.gpio_pin);
    
    // Unexport the GPIO
    WriteToFile(GPIO_UNEXPORT_PATH, std::to_string(linux_gpio));
    
    spdlog::debug("Reset button: GPIO {} cleaned up", linux_gpio);
    return true;
}

bool ResetButton::ReadButtonState() {
    int linux_gpio = GetLinuxGpioNumber(settings_.gpio_pin);
    std::string value_path = "/sys/class/gpio/gpio" + std::to_string(linux_gpio) + "/value";
    
    std::string value = ReadFromFile(value_path);
    if (value.empty()) {
        return false;
    }
    
    bool gpio_high = (value[0] == '1');
    
    // If active_low, button is pressed when GPIO is LOW
    return settings_.active_low ? !gpio_high : gpio_high;
}

// ============================================================================
// Lifecycle
// ============================================================================
bool ResetButton::Init() {
    std::lock_guard<std::mutex> lock(g_mutex);
    
    if (initialized_) {
        spdlog::warn("Reset button already initialized");
        return true;
    }
    
    spdlog::info("Initializing Reset Button module (P_GPIO{})", settings_.gpio_pin);
    
    // Load configuration
    // gpio_pin in config is the P_GPIO number (e.g., 12 for P_GPIO12)
    // We convert to Linux GPIO number internally: P_GPIO12 -> 12 + 32 = 44
    settings_.gpio_pin = config::Get<int>("reset_button.gpio_pin", 12);  // Default: P_GPIO12
    settings_.hold_duration_sec = config::Get<int>("reset_button.hold_duration_sec", 10);
    settings_.active_low = config::Get<bool>("reset_button.active_low", true);
    settings_.enable_feedback = config::Get<bool>("reset_button.enable_feedback", true);
    settings_.debounce_ms = config::Get<int>("reset_button.debounce_ms", 50);
    settings_.poll_interval_ms = config::Get<int>("reset_button.poll_interval_ms", 100);
    
    int linux_gpio = GetLinuxGpioNumber(settings_.gpio_pin);
    spdlog::info("Reset button config: P_GPIO{} (Linux GPIO {}), hold_time={}s, active_low={}",
                 settings_.gpio_pin, linux_gpio, settings_.hold_duration_sec, settings_.active_low);
    
    // Setup GPIO
    if (!SetupGPIO()) {
        spdlog::error("Failed to setup reset button GPIO");
        return false;
    }
    
    // Setup interrupt (optional, will fall back to polling)
    SetupInterrupt();
    
    // Start monitor thread
    running_ = true;
    monitor_thread_ = std::thread(&ResetButton::MonitorLoop, this);
    
    initialized_ = true;
    last_event_ = GetTimestamp() + " - Initialized";
    
    spdlog::info("Reset Button module initialized - hold for {}s to factory reset",
                 settings_.hold_duration_sec);
    
    return true;
}

void ResetButton::Shutdown() {
    if (!initialized_) {
        return;
    }
    
    spdlog::info("Shutting down Reset Button module");
    
    running_ = false;
    
    if (monitor_thread_.joinable()) {
        monitor_thread_.join();
    }
    
    CleanupGPIO();
    
    initialized_ = false;
    spdlog::info("Reset Button module shutdown complete");
}

// ============================================================================
// Monitor Thread
// ============================================================================
void ResetButton::MonitorLoop() {
    spdlog::info("Reset button monitor thread started");
    
    // For proper debouncing, we need to see the same state for multiple consecutive reads
    // Increase debounce requirements to filter noise on floating GPIO
    const int STABLE_READS_REQUIRED = 5;  // Need 5 consecutive same readings
    const int SAMPLE_INTERVAL_MS = 50;    // Sample every 50ms
    
    int stable_count = 0;
    bool last_raw_state = false;
    bool debounced_state = false;
    bool last_debounced_state = false;
    
    // Initial read to get baseline
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    last_raw_state = ReadButtonState();
    debounced_state = last_raw_state;
    last_debounced_state = last_raw_state;
    
    spdlog::info("Reset button initial state: {} (active_low={})", 
                 last_raw_state ? "PRESSED" : "RELEASED", settings_.active_low);
    
    while (running_) {
        // Use consistent polling interval - don't use interrupts due to noise
        std::this_thread::sleep_for(std::chrono::milliseconds(SAMPLE_INTERVAL_MS));
        
        if (!running_) break;
        
        // Read current state
        bool current_raw = ReadButtonState();
        
        // Debounce: require consistent readings
        if (current_raw == last_raw_state) {
            stable_count++;
            if (stable_count >= STABLE_READS_REQUIRED) {
                // State is stable, update debounced state
                debounced_state = current_raw;
                stable_count = STABLE_READS_REQUIRED; // Cap to prevent overflow
            }
        } else {
            // State changed, reset stability counter
            stable_count = 0;
            last_raw_state = current_raw;
        }
        
        // Handle debounced state transitions
        if (debounced_state && !last_debounced_state) {
            // Button just pressed (debounced)
            HandleButtonPress();
        } else if (!debounced_state && last_debounced_state) {
            // Button just released (debounced)
            HandleButtonRelease();
        } else if (debounced_state && button_pressed_) {
            // Button still held - update hold duration
            auto now = std::chrono::steady_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::seconds>(
                now - press_start_time_).count();
            
            int new_seconds = static_cast<int>(duration);
            if (new_seconds != hold_seconds_) {
                hold_seconds_ = new_seconds;
                
                // Log progress
                spdlog::info("Reset button held: {}/{} seconds", 
                            new_seconds, settings_.hold_duration_sec);
                
                // Call progress callback
                if (progress_callback_) {
                    progress_callback_(new_seconds, settings_.hold_duration_sec);
                }
                
                // Check if reset threshold reached
                if (new_seconds >= settings_.hold_duration_sec) {
                    spdlog::warn("FACTORY RESET TRIGGERED - button held for {} seconds!",
                                new_seconds);
                    last_event_ = GetTimestamp() + " - FACTORY RESET TRIGGERED";
                    
                    // Call reset callback
                    if (reset_callback_) {
                        reset_callback_();
                    }
                    
                    // Reset state - wait for button release
                    button_pressed_ = false;
                    hold_seconds_ = 0;
                    debounced_state = false;
                    last_debounced_state = false;
                    
                    // Wait for button release before resuming monitoring
                    while (running_ && ReadButtonState()) {
                        std::this_thread::sleep_for(std::chrono::milliseconds(100));
                    }
                }
            }
        }
        
        last_debounced_state = debounced_state;
    }
    
    spdlog::info("Reset button monitor thread stopped");
}

void ResetButton::HandleButtonPress() {
    spdlog::info("Reset button PRESSED");
    button_pressed_ = true;
    press_start_time_ = std::chrono::steady_clock::now();
    hold_seconds_ = 0;
    last_event_ = GetTimestamp() + " - Button pressed";
}

void ResetButton::HandleButtonRelease() {
    int held_for = hold_seconds_.load();
    spdlog::info("Reset button RELEASED after {} seconds", held_for);
    
    if (held_for < settings_.hold_duration_sec) {
        spdlog::info("Button released before reset threshold ({}/{}s)", 
                    held_for, settings_.hold_duration_sec);
    }
    
    button_pressed_ = false;
    hold_seconds_ = 0;
    last_event_ = GetTimestamp() + " - Button released after " + std::to_string(held_for) + "s";
}

// ============================================================================
// Callbacks
// ============================================================================
void ResetButton::SetResetCallback(ResetCallback callback) {
    std::lock_guard<std::mutex> lock(g_mutex);
    reset_callback_ = std::move(callback);
}

void ResetButton::SetProgressCallback(ProgressCallback callback) {
    std::lock_guard<std::mutex> lock(g_mutex);
    progress_callback_ = std::move(callback);
}

// ============================================================================
// Settings
// ============================================================================
bool ResetButton::SetSettings(const ResetButtonSettings& settings) {
    std::lock_guard<std::mutex> lock(g_mutex);
    
    // Validate settings
    if (settings.gpio_pin < 0 || settings.gpio_pin > 25) {
        spdlog::error("Invalid P_GPIO pin: {}", settings.gpio_pin);
        return false;
    }
    if (settings.hold_duration_sec < 1 || settings.hold_duration_sec > 30) {
        spdlog::error("Invalid hold duration: {}s (must be 1-30)", settings.hold_duration_sec);
        return false;
    }
    
    settings_ = settings;
    
    // Save to config
    config::Set<int>("reset_button.gpio_pin", settings.gpio_pin);
    config::Set<int>("reset_button.hold_duration_sec", settings.hold_duration_sec);
    config::Set<bool>("reset_button.active_low", settings.active_low);
    config::Set<bool>("reset_button.enable_feedback", settings.enable_feedback);
    config::Set<int>("reset_button.debounce_ms", settings.debounce_ms);
    config::Set<int>("reset_button.poll_interval_ms", settings.poll_interval_ms);
    config::Save();
    
    spdlog::info("Reset button settings updated: P_GPIO{}, hold={}s", 
                 settings.gpio_pin, settings.hold_duration_sec);
    return true;
}

ResetButtonSettings ResetButton::GetSettings() const {
    return settings_;
}

// ============================================================================
// Status
// ============================================================================
bool ResetButton::IsButtonPressed() const {
    return button_pressed_;
}

int ResetButton::GetHoldDuration() const {
    return hold_seconds_;
}

ResetButton::Status ResetButton::GetStatus() const {
    return Status{
        .initialized = initialized_.load(),
        .button_pressed = button_pressed_.load(),
        .hold_seconds = hold_seconds_.load(),
        .required_seconds = settings_.hold_duration_sec,
        .last_event = last_event_
    };
}

} // namespace platform
} // namespace ipcam
