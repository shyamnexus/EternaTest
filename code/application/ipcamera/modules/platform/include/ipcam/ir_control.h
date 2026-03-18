#pragma once
#include <string>
#include <cstdint>
#include <functional>
#include <atomic>
#include <thread>

namespace ipcam {
namespace platform {

// ============================================================================
// IR LED Control (PWM on P_GPIO11)
// ============================================================================
struct IRLedSettings {
    bool enabled = false;          // Master enable for IR LED (default: disabled)
    int brightness = 0;            // 0-100 duty cycle percentage (default: off)
    int pwm_channel = 11;          // PWM channel (default: 11 for P_GPIO11)
    int pwm_frequency = 1000;      // Target frequency in Hz
    bool auto_brightness = false;  // Auto-adjust based on distance/reflection
};

// ============================================================================
// IR Cut Filter Control
// ============================================================================
enum class IRCutState {
    Day,      // IR cut filter engaged (blocks IR) - color mode
    Night,    // IR cut filter disengaged (passes IR) - B/W mode  
    Unknown
};

// IR Cut uses pulse-based dual GPIO control for H-bridge motor driver:
//   gpio_pin_a: Pulse HIGH to switch to NIGHT mode (opens IR filter)
//   gpio_pin_b: Pulse HIGH to switch to DAY mode (closes IR filter)
struct IRCutSettings {
    bool enabled = false;          // Enable IR cut switching (default: disabled)
    int gpio_pin_a = 115;          // GPIO for IRC_A (night mode pulse) - L_GPIO19
    int gpio_pin_b = 114;          // GPIO for IRC_B (day mode pulse) - L_GPIO18
    int switch_delay_ms = 100;     // Pulse duration in milliseconds
};

// ============================================================================
// Day/Night Auto Switch Settings (includes SW CDS)
// ============================================================================
enum class DayNightAutoMode {
    Off,           // Manual control only (legacy)
    Auto,          // Use light sensor / ISP luma for detection (legacy)
    SwCds,         // Use ISP Software CDS (no hardware photoresistor needed)
    Schedule,      // Time-based switching
    External,      // External sensor trigger (GPIO input)
    Day,           // Force day mode (color, IR off)
    Night          // Force night mode (B&W, IR on)
};

struct DayNightAutoSettings {
    DayNightAutoMode mode = DayNightAutoMode::Off;  // Default: manual control
    
    // Basic thresholds (for Auto mode with hardware sensor)
    int day_to_night_threshold = 30;    // Luma threshold to switch to night (0-255)
    int night_to_day_threshold = 60;    // Luma threshold to switch to day (0-255)
    int switch_delay_sec = 5;           // Delay before switching (hysteresis)
    
    // Schedule settings (for Schedule mode)
    std::string schedule_night_start = "18:00";
    std::string schedule_day_start = "06:00";
    
    // Common settings
    bool sync_sensor_bw_mode = true;    // Switch sensor to B/W in night mode
    bool log_transitions = true;        // Log day/night transitions
    
    // SW CDS settings (for SwCds mode - ISP-based detection)
    int night_ev = 300;             // EV threshold for night mode (switch when EV < this)
    int day_ev = 600;               // EV threshold for day mode (switch when EV > this)
    int sensitivity = 5;            // Sensitivity (0-10)
    int max_ir_ev = 1200;           // Maximum IR lamp power limit
    int night_brightness = 80;      // IR LED brightness when switching to night (0-100)
    bool bw_mode_enabled = true;    // Enable B/W mode when in night mode
};

// ============================================================================
// Combined IR Control Status
// ============================================================================
struct IRControlStatus {
    // IR LED status
    bool ir_led_on = false;
    int ir_led_brightness = 0;
    
    // IR Cut filter status
    IRCutState ir_cut_state = IRCutState::Unknown;
    
    // Day/Night status
    bool is_night_mode = false;
    int current_luma = 128;            // Current ambient light level (0-255)
    int current_ev = 0;                // Current EV value from SW CDS
    std::string last_switch_time;      // Timestamp of last mode switch
    std::string last_switch_reason;    // Reason for last switch
    
    // SW CDS status
    bool sw_cds_initialized = false;
    bool sw_cds_active = false;
};

// ============================================================================
// IR Control Interface
// ============================================================================
class IRControl {
public:
    static IRControl& Instance();
    
    // Lifecycle
    bool Init();
    void Shutdown();
    bool IsInitialized() const { return initialized_; }
    
    // ========== IR LED Control ==========
    
    // Set IR LED brightness (0-100, 0=off)
    bool SetIRLedBrightness(int brightness);
    int GetIRLedBrightness() const { return ir_led_settings_.brightness; }
    
    // Turn IR LED on/off
    bool SetIRLedEnabled(bool enabled);
    bool IsIRLedEnabled() const { return ir_led_settings_.enabled; }
    
    // Force IR LED on/off (bypasses day/night mode and doesn't save config)
    // Use this for temporary feedback like reset button indication
    bool ForceIRLed(bool on);
    
    // Full IR LED settings
    bool SetIRLedSettings(const IRLedSettings& settings);
    IRLedSettings GetIRLedSettings() const { return ir_led_settings_; }
    
    // ========== IR Cut Filter Control ==========
    
    // Set IR cut filter state (Day=engaged/color, Night=disengaged/IR)
    bool SetIRCutState(IRCutState state);
    IRCutState GetIRCutState() const { return ir_cut_state_; }
    
    // Full IR cut settings
    bool SetIRCutSettings(const IRCutSettings& settings);
    IRCutSettings GetIRCutSettings() const { return ir_cut_settings_; }
    
    // ========== Day/Night Auto Control ==========
    
    // Switch to day mode (IR LED off, IR cut engaged, color mode)
    bool SwitchToDayMode();
    
    // Switch to night mode (IR LED on, IR cut disengaged, B/W mode)
    bool SwitchToNightMode();
    
    // Check if currently in night mode
    bool IsNightMode() const { return is_night_mode_; }
    
    // Set auto switching mode
    bool SetDayNightAutoMode(DayNightAutoMode mode);
    DayNightAutoMode GetDayNightAutoMode() const { return auto_settings_.mode; }
    
    // Full auto settings
    bool SetDayNightAutoSettings(const DayNightAutoSettings& settings);
    DayNightAutoSettings GetDayNightAutoSettings() const { return auto_settings_; }
    
    // ========== SW CDS (Software Color Day/Night Switching) ==========
    
    // Initialize SW CDS subsystem (called when mode switches to SwCds)
    bool InitSwCds();
    
    // Shutdown SW CDS
    void ShutdownSwCds();
    
    // Check if SW CDS is initialized
    bool IsSwCdsInitialized() const { return sw_cds_initialized_; }
    
    // Get current EV from SW CDS
    int GetCurrentEV() const { return current_ev_; }
    
    // Update current luma reading (called by ISP or external)
    void UpdateCurrentLuma(int luma);
    int GetCurrentLuma() const { return current_luma_; }
    
    // ========== Status ==========
    
    IRControlStatus GetStatus() const;
    
    // ========== Config ==========
    
    bool SaveToConfig();
    bool LoadFromConfig();
    bool ResetToDefaults();
    
    // ========== Callbacks ==========
    
    using DayNightCallback = std::function<void(bool is_night)>;
    void SetDayNightCallback(DayNightCallback callback);

private:
    IRControl() = default;
    ~IRControl() = default;
    IRControl(const IRControl&) = delete;
    IRControl& operator=(const IRControl&) = delete;
    
    // Hardware control
    bool ApplyPWMBrightness(int brightness);
    bool ClosePWM();
    bool SetGPIO(int pin, bool value);
    bool GetGPIO(int pin);
    
    // Internal version without lock (caller must hold mutex)
    bool SetIRCutStateInternal(IRCutState state);
    
    // Auto detection task
    void AutoDetectionLoop();
    void CheckAndSwitchMode();
    
    // SW CDS detection loop
    void SwCdsDetectionLoop();
    bool CheckSwCdsNight();
    bool CheckSwCdsDay();
    
    bool initialized_ = false;
    bool is_night_mode_ = false;
    int current_luma_ = 128;
    int current_ev_ = 0;
    
    IRLedSettings ir_led_settings_;
    IRCutSettings ir_cut_settings_;
    IRCutState ir_cut_state_ = IRCutState::Unknown;
    DayNightAutoSettings auto_settings_;
    
    std::string last_switch_time_;
    std::string last_switch_reason_;
    
    DayNightCallback day_night_callback_;
    
    // Auto detection thread
    std::atomic<bool> auto_thread_running_{false};
    std::thread auto_thread_;
    int luma_below_threshold_count_ = 0;
    int luma_above_threshold_count_ = 0;
    
    // SW CDS state
    bool sw_cds_initialized_ = false;
    std::atomic<bool> sw_cds_thread_running_{false};
    std::thread sw_cds_thread_;
    
    // Schedule thread
    std::atomic<bool> schedule_thread_running_{false};
    std::thread schedule_thread_;
    void ScheduleMonitorLoop();
};

// ============================================================================
// Helper Functions
// ============================================================================
std::string IRCutStateToString(IRCutState state);
IRCutState StringToIRCutState(const std::string& str);

std::string DayNightAutoModeToString(DayNightAutoMode mode);
DayNightAutoMode StringToDayNightAutoMode(const std::string& str);

} // namespace platform
} // namespace ipcam
