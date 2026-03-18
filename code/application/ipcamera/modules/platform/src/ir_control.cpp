#include "ipcam/ir_control.h"
#include "ipcam/config.h"
#include <spdlog/spdlog.h>
#include <fstream>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>

// SW CDS SDK headers
extern "C" {
#include "light_lib.h"
#include "vendor_isp.h"
}

namespace ipcam {
namespace platform {

namespace {
    std::mutex g_mutex;
    std::condition_variable g_thread_cv;  // For waking up sleeping threads
    
    // PWM constants for P_GPIO11 (PWM11)
    constexpr int PWM_CHANNEL_IR_LED = 11;
    constexpr int PWM_PERIOD_NS = 1000000;   // Period in nanoseconds (1kHz frequency)
    
    // PWM sysfs paths
    const char* PWM_CHIP_PATH = "/sys/class/pwm/pwmchip0";
    const char* PROC_GPIO_EXPORT = "/sys/class/gpio/export";
    const char* PROC_GPIO_UNEXPORT = "/sys/class/gpio/unexport";
    
    // Track PWM export state
    static bool g_pwm_exported = false;
    
    // SW CDS state
    static int g_sw_cds_device_handle = -1;
    static UINT32 g_sw_cds_channel_id = 0;
    static bool g_isp_initialized_by_us = false;  // Track if we called vendor_isp_init()
    
    // Helper to get current timestamp
    std::string GetTimestamp() {
        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);
        std::stringstream ss;
        ss << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S");
        return ss.str();
    }
    
    // P_GPIO offset (from documentation: P_GPIO(pin) = pin + 0x40)
    constexpr int P_GPIO_OFFSET = 0x40;
    int PGpioToLinuxGpio(int p_gpio_pin) {
        return p_gpio_pin + P_GPIO_OFFSET;
    }
    
    // SW CDS light info callback - called by the library to get sensor data
    int SwCdsLightInfoCallback(LIGHT_INFO* light_info) {
        HD_RESULT ret;
        static CA_DATA ca_data;  // static to persist after callback returns
        AWBT_CA awb_ca;
        AET_STATUS_INFO ae_status;
        AET_CURVE_GEN_MOVIE ae_curve_gen_movie;
        AWBT_STATUS awb_sta;
        
        memset(&ca_data, 0, sizeof(ca_data));
        memset(&awb_ca, 0, sizeof(awb_ca));
        awb_ca.id = static_cast<AWB_ID>(g_sw_cds_channel_id);
        ret = vendor_isp_get_awb(AWBT_ITEM_CA, &awb_ca);
        if (ret < 0) {
            spdlog::warn("SW CDS: Failed to get AWB CA data");
            return E_GET_DEV_FAIL;
        }
        
        light_info->ca = &ca_data;
        memcpy(&light_info->ca->win_num_x, &awb_ca.ca.win_num_x, sizeof(CA_DATA));
        
        memset(&ae_status, 0, sizeof(ae_status));
        ae_status.id = static_cast<AE_ID>(g_sw_cds_channel_id);
        ret = vendor_isp_get_ae(AET_ITEM_STATUS, &ae_status);
        if (ret < 0) {
            spdlog::warn("SW CDS: Failed to get AE status");
            return E_GET_DEV_FAIL;
        }
        
        light_info->iso_expt_fn[0] = ae_status.status_info.iso_gain[0];
        light_info->iso_expt_fn[1] = ae_status.status_info.expotime[0];
        
        memset(&ae_curve_gen_movie, 0, sizeof(ae_curve_gen_movie));
        ae_curve_gen_movie.id = static_cast<AE_ID>(g_sw_cds_channel_id);
        ret = vendor_isp_get_ae(AET_ITEM_CURVE_GEN_MOVIE, &ae_curve_gen_movie);
        light_info->iso_expt_fn[2] = ae_curve_gen_movie.curve_gen_movie.iso_calcoef;
        
        memset(&awb_sta, 0, sizeof(awb_sta));
        awb_sta.id = static_cast<AWB_ID>(g_sw_cds_channel_id);
        ret = vendor_isp_get_awb(AWBT_ITEM_STATUS, &awb_sta);
        if (ret < 0) {
            spdlog::warn("SW CDS: Failed to get AWB status");
            return E_GET_DEV_FAIL;
        }
        
        if ((awb_sta.status.mode != AWBALG_STATUS_FREEZE) && 
            (awb_sta.status.mode != AWBALG_STATUS_NIGHTMODE)) {
            light_info->light_awb_status.mode = AWB_ALG_MODE;
        } else {
            light_info->light_awb_status.mode = AWB_GRAY_MODE;
        }
        light_info->light_awb_status.r_gain = awb_sta.status.cur_r_gain;
        light_info->light_awb_status.g_gain = awb_sta.status.cur_g_gain;
        light_info->light_awb_status.b_gain = awb_sta.status.cur_b_gain;
        
        return E_OK;
    }
    
    // Set ISP day/night mode
    bool SetISPDayNightMode(bool night_mode) {
        IQT_NIGHT_MODE iq_night_mode;
        memset(&iq_night_mode, 0, sizeof(iq_night_mode));
        iq_night_mode.id = static_cast<IQ_ID>(g_sw_cds_channel_id);
        iq_night_mode.mode = night_mode ? IQ_UI_NIGHT_MODE_ON : IQ_UI_NIGHT_MODE_OFF;
        
        int ret = vendor_isp_set_iq(IQT_ITEM_NIGHT_MODE, &iq_night_mode);
        if (ret < 0) {
            spdlog::error("Failed to set ISP night mode: {}", ret);
            return false;
        }
        
        spdlog::info("ISP set to {} mode", night_mode ? "night (mono)" : "day (color)");
        return true;
    }
}

// ============================================================================
// Singleton Instance
// ============================================================================
IRControl& IRControl::Instance() {
    static IRControl instance;
    return instance;
}

// ============================================================================
// Lifecycle
// ============================================================================
bool IRControl::Init() {
    std::unique_lock<std::mutex> lock(g_mutex);
    
    if (initialized_) {
        spdlog::warn("IRControl already initialized");
        return true;
    }
    
    spdlog::info("Initializing IR Control module");
    
    // Load config
    if (!LoadFromConfig()) {
        spdlog::warn("Failed to load IR config, using defaults");
    }
    
    // Initialize to day mode by default
    ir_cut_state_ = IRCutState::Day;
    is_night_mode_ = false;
    
    // Apply initial settings
    if (ir_led_settings_.enabled && is_night_mode_) {
        ApplyPWMBrightness(ir_led_settings_.brightness);
    } else {
        ClosePWM();
    }
    
    // Set IR cut to day mode (use internal version to avoid deadlock)
    SetIRCutStateInternal(IRCutState::Day);
    
    // Store the configured mode - we'll activate it after marking initialized
    DayNightAutoMode configured_mode = auto_settings_.mode;
    
    // Temporarily set to Off during init, will be properly activated below
    auto_settings_.mode = DayNightAutoMode::Off;
    
    initialized_ = true;
    spdlog::info("IR Control module initialized");
    
    // Release lock before activating auto mode (SetDayNightAutoMode acquires locks)
    lock.unlock();
    
    // Now properly activate the configured auto mode
    // This handles all the thread setup, SW CDS initialization, etc.
    if (configured_mode != DayNightAutoMode::Off) {
        spdlog::info("Activating configured IR auto mode: {}", DayNightAutoModeToString(configured_mode));
        
        // For "auto" mode without external luma source, use SW CDS which provides
        // actual light readings from the ISP sensor
        if (configured_mode == DayNightAutoMode::Auto) {
            spdlog::info("Auto mode uses SW CDS for light detection from ISP sensor");
            // Use SwCds mode which actually reads light levels from ISP
            if (!SetDayNightAutoMode(DayNightAutoMode::SwCds)) {
                spdlog::warn("SW CDS initialization failed, trying legacy auto mode");
                SetDayNightAutoMode(DayNightAutoMode::Auto);
            }
        } else {
            SetDayNightAutoMode(configured_mode);
        }
    }
    
    return true;
}

void IRControl::Shutdown() {
    // Check if initialized before acquiring lock
    {
        std::lock_guard<std::mutex> lock(g_mutex);
        if (!initialized_) {
            return;
        }
        initialized_ = false;  // Mark as shutting down early
    }
    
    spdlog::info("Shutting down IR Control module");
    
    // Signal all threads to stop BEFORE joining (outside lock)
    auto_thread_running_ = false;
    schedule_thread_running_ = false;
    sw_cds_thread_running_ = false;
    
    // Wake up any sleeping threads immediately
    g_thread_cv.notify_all();
    
    // Join threads (outside lock to avoid deadlock)
    if (auto_thread_.joinable()) {
        auto_thread_.join();
    }
    
    if (schedule_thread_.joinable()) {
        schedule_thread_.join();
    }
    
    // Shutdown SW CDS (thread already stopped by above)
    ShutdownSwCds();
    
    // Turn off IR LED (ignore errors during shutdown - system may be rebooting)
    ClosePWM();
    
    // Try to set IR cut to day mode, but don't log errors during shutdown
    // (system may already be in reboot state with GPIOs unavailable)
    try {
        SetIRCutStateInternal(IRCutState::Day);
    } catch (...) {
        // Ignore errors during shutdown
    }
    
    spdlog::info("IR Control shutdown complete");
}

// ============================================================================
// IR LED Control
// ============================================================================
bool IRControl::SetIRLedBrightness(int brightness) {
    if (brightness < 0 || brightness > 100) {
        spdlog::error("Invalid IR LED brightness: {} (must be 0-100)", brightness);
        return false;
    }
    
    std::lock_guard<std::mutex> lock(g_mutex);
    
    ir_led_settings_.brightness = brightness;
    
    // Only apply if IR LED is enabled and we're in night mode (or manual override)
    if (ir_led_settings_.enabled) {
        if (brightness > 0) {
            if (!ApplyPWMBrightness(brightness)) {
                spdlog::error("Failed to apply IR LED brightness");
                return false;
            }
        } else {
            ClosePWM();
        }
    }
    
    // Save to config
    config::Set<int>("ir.led.brightness", brightness);
    config::Save();
    
    spdlog::debug("IR LED brightness set to {}%", brightness);
    return true;
}

bool IRControl::SetIRLedEnabled(bool enabled) {
    std::lock_guard<std::mutex> lock(g_mutex);
    
    ir_led_settings_.enabled = enabled;
    
    if (enabled && is_night_mode_) {
        ApplyPWMBrightness(ir_led_settings_.brightness);
    } else if (!enabled) {
        ClosePWM();
    }
    
    config::Set<bool>("ir.led.enabled", enabled);
    config::Save();
    
    spdlog::info("IR LED {}", enabled ? "enabled" : "disabled");
    return true;
}

bool IRControl::ForceIRLed(bool on) {
    std::lock_guard<std::mutex> lock(g_mutex);
    
    // Directly control PWM without checking day/night mode or saving config
    // Used for temporary visual feedback (e.g., reset button indication)
    if (on) {
        int brightness = ir_led_settings_.brightness > 0 ? ir_led_settings_.brightness : 100;
        return ApplyPWMBrightness(brightness);
    } else {
        return ClosePWM();
    }
}

bool IRControl::SetIRLedSettings(const IRLedSettings& settings) {
    if (settings.brightness < 0 || settings.brightness > 100) {
        spdlog::error("Invalid IR LED brightness: {}", settings.brightness);
        return false;
    }
    if (settings.pwm_channel < 0 || settings.pwm_channel > 15) {
        spdlog::error("Invalid PWM channel: {}", settings.pwm_channel);
        return false;
    }
    
    std::lock_guard<std::mutex> lock(g_mutex);
    
    ir_led_settings_ = settings;
    
    // Apply to hardware
    if (settings.enabled && is_night_mode_) {
        ApplyPWMBrightness(settings.brightness);
    } else if (!settings.enabled) {
        ClosePWM();
    }
    
    // Save to config
    config::Set<bool>("ir.led.enabled", settings.enabled);
    config::Set<int>("ir.led.brightness", settings.brightness);
    config::Set<int>("ir.led.pwm_channel", settings.pwm_channel);
    config::Set<int>("ir.led.pwm_frequency", settings.pwm_frequency);
    config::Set<bool>("ir.led.auto_brightness", settings.auto_brightness);
    config::Save();
    
    spdlog::info("IR LED settings updated: enabled={}, brightness={}%", 
                 settings.enabled, settings.brightness);
    return true;
}

// ============================================================================
// IR Cut Filter Control (Pulse-based dual GPIO for motor driver)
// ============================================================================
// IR Cut uses two GPIOs for H-bridge motor driver:
//   - gpio_pin_a (L_GPIO19=115): IRC_A - pulse HIGH for NIGHT mode (open filter)
//   - gpio_pin_b (L_GPIO18=114): IRC_B - pulse HIGH for DAY mode (close filter)
// Motor needs a brief pulse (~100ms) to switch position, then GPIOs go LOW

// Internal version without lock (caller must hold the lock)
bool IRControl::SetIRCutStateInternal(IRCutState state) {
    if (!ir_cut_settings_.enabled) {
        spdlog::debug("IR cut control disabled");
        return false;
    }
    
    int gpio_a = ir_cut_settings_.gpio_pin_a;  // IRC_A for night
    int gpio_b = ir_cut_settings_.gpio_pin_b;  // IRC_B for day
    int pulse_ms = ir_cut_settings_.switch_delay_ms > 0 ? ir_cut_settings_.switch_delay_ms : 100;
    
    // During shutdown, GPIO access may fail - don't log errors
    bool quiet_mode = !initialized_;
    
    switch (state) {
        case IRCutState::Day:
            // Day mode: Pulse IRC_B, keep IRC_A low
            // This closes the IR cut filter (blocks IR light)
            spdlog::debug("IR cut: Switching to DAY (pulse GPIO{})", gpio_b);
            SetGPIO(gpio_a, 0);  // Ensure IRC_A is low
            if (!SetGPIO(gpio_b, 1)) {
                if (!quiet_mode) {
                    spdlog::error("Failed to set IR cut GPIO_B");
                }
                return false;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(pulse_ms));
            SetGPIO(gpio_b, 0);  // End pulse
            break;
            
        case IRCutState::Night:
            // Night mode: Pulse IRC_A, keep IRC_B low
            // This opens the IR cut filter (allows IR light)
            spdlog::debug("IR cut: Switching to NIGHT (pulse GPIO{})", gpio_a);
            SetGPIO(gpio_b, 0);  // Ensure IRC_B is low
            if (!SetGPIO(gpio_a, 1)) {
                if (!quiet_mode) {
                    spdlog::error("Failed to set IR cut GPIO_A");
                }
                return false;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(pulse_ms));
            SetGPIO(gpio_a, 0);  // End pulse
            break;
            
        default:
            if (!quiet_mode) {
                spdlog::error("Invalid IR cut state");
            }
            return false;
    }
    
    ir_cut_state_ = state;
    
    if (!quiet_mode) {
        spdlog::info("IR cut filter set to {} mode", IRCutStateToString(state));
    }
    return true;
}

// Public version with lock
bool IRControl::SetIRCutState(IRCutState state) {
    std::lock_guard<std::mutex> lock(g_mutex);
    return SetIRCutStateInternal(state);
}

bool IRControl::SetIRCutSettings(const IRCutSettings& settings) {
    std::lock_guard<std::mutex> lock(g_mutex);
    
    ir_cut_settings_ = settings;
    
    // Save to config
    config::Set<bool>("ir.cut.enabled", settings.enabled);
    config::Set<int>("ir.cut.gpio_pin_a", settings.gpio_pin_a);
    config::Set<int>("ir.cut.gpio_pin_b", settings.gpio_pin_b);
    config::Set<int>("ir.cut.switch_delay_ms", settings.switch_delay_ms);
    config::Save();
    
    spdlog::info("IR cut settings updated: enabled={}, gpio_a={}, gpio_b={}", 
                 settings.enabled, settings.gpio_pin_a, settings.gpio_pin_b);
    return true;
}

// ============================================================================
// Day/Night Mode Control
// ============================================================================
bool IRControl::SwitchToDayMode() {
    spdlog::info("Switching to DAY mode");
    
    // 1. Turn off IR LED
    ClosePWM();
    
    // 2. Engage IR cut filter (blocks IR)
    if (!SetIRCutState(IRCutState::Day)) {
        spdlog::warn("Failed to set IR cut to day mode");
    }
    
    // 3. Update state
    {
        std::lock_guard<std::mutex> lock(g_mutex);
        is_night_mode_ = false;
        last_switch_time_ = GetTimestamp();
        last_switch_reason_ = "Switched to Day mode";
    }
    
    // 4. Callback for sensor mode change (B/W -> Color)
    if (day_night_callback_) {
        day_night_callback_(false);
    }
    
    // 5. Set ISP to color mode (day mode)
    if (auto_settings_.sync_sensor_bw_mode) {
        SetISPDayNightMode(false);
    }
    
    spdlog::info("DAY mode active - IR LED OFF, IR cut engaged (color mode)");
    return true;
}

bool IRControl::SwitchToNightMode() {
    spdlog::info("Switching to NIGHT mode");
    
    // 1. Disengage IR cut filter (passes IR)
    if (!SetIRCutState(IRCutState::Night)) {
        spdlog::warn("Failed to set IR cut to night mode");
    }
    
    // 2. Wait for filter to switch before turning on IR LED
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    
    // 3. Turn on IR LED
    if (ir_led_settings_.enabled) {
        ApplyPWMBrightness(ir_led_settings_.brightness);
    }
    
    // 4. Update state
    {
        std::lock_guard<std::mutex> lock(g_mutex);
        is_night_mode_ = true;
        last_switch_time_ = GetTimestamp();
        last_switch_reason_ = "Switched to Night mode";
    }
    
    // 5. Callback for sensor mode change (Color -> B/W)
    if (day_night_callback_) {
        day_night_callback_(true);
    }
    
    // 6. Set ISP to B&W mode (night mode)
    if (auto_settings_.sync_sensor_bw_mode) {
        SetISPDayNightMode(true);
    }
    
    spdlog::info("NIGHT mode active - IR LED ON ({}%), IR cut disengaged (B/W mode)", 
                 ir_led_settings_.brightness);
    return true;
}

bool IRControl::SetDayNightAutoMode(DayNightAutoMode mode) {
    DayNightAutoMode old_mode;
    bool was_night_mode = false;
    
    // Get current state under lock
    {
        std::lock_guard<std::mutex> lock(g_mutex);
        old_mode = auto_settings_.mode;
        was_night_mode = is_night_mode_;
        auto_settings_.mode = mode;
        
        config::Set<std::string>("ir.auto.mode", DayNightAutoModeToString(mode));
        config::Save();
    }
    
    // Stop existing threads WITHOUT holding lock to avoid blocking
    // Signal threads to stop first
    if ((old_mode == DayNightAutoMode::Auto || old_mode == DayNightAutoMode::Schedule) 
        && auto_thread_running_) {
        auto_thread_running_ = false;
    }
    
    if (old_mode == DayNightAutoMode::SwCds && sw_cds_thread_running_) {
        sw_cds_thread_running_ = false;
    }
    
    if (old_mode == DayNightAutoMode::Schedule && schedule_thread_running_) {
        schedule_thread_running_ = false;
    }
    
    // Wake up any sleeping threads immediately
    g_thread_cv.notify_all();
    
    // Now join threads (outside lock to avoid blocking API)
    if (auto_thread_.joinable()) {
        auto_thread_.join();
    }
    if (sw_cds_thread_.joinable()) {
        sw_cds_thread_.join();
    }
    if (schedule_thread_.joinable()) {
        schedule_thread_.join();
    }
    
    // Shutdown SW CDS if it was running
    if (old_mode == DayNightAutoMode::SwCds) {
        ShutdownSwCds();
    }
    
    // Handle the new mode
    if (mode == DayNightAutoMode::Day) {
        // Force Day mode: IR off, IR cut to day, color mode
        ApplyPWMBrightness(0);  // IR LED off
        SetIRCutStateInternal(IRCutState::Day);
        
        // Set ISP to color mode
        if (auto_settings_.sync_sensor_bw_mode) {
            SetISPDayNightMode(false);
        }
        
        {
            std::lock_guard<std::mutex> lock(g_mutex);
            is_night_mode_ = false;
            last_switch_reason_ = "Forced Day mode";
            last_switch_time_ = GetTimestamp();
        }
        spdlog::info("Forced to Day mode: IR off, ISP color mode");
        
    } else if (mode == DayNightAutoMode::Night) {
        // Force Night mode: IR on, IR cut to night, B&W mode
        SetIRCutStateInternal(IRCutState::Night);
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        ApplyPWMBrightness(ir_led_settings_.brightness > 0 ? ir_led_settings_.brightness : 100);
        
        // Set ISP to B&W mode
        if (auto_settings_.sync_sensor_bw_mode) {
            SetISPDayNightMode(true);
        }
        
        {
            std::lock_guard<std::mutex> lock(g_mutex);
            is_night_mode_ = true;
            last_switch_reason_ = "Forced Night mode";
            last_switch_time_ = GetTimestamp();
        }
        spdlog::info("Forced to Night mode: IR on, ISP B&W mode");
        
    } else if (mode == DayNightAutoMode::Schedule) {
        // Sync ISP to current is_night_mode state before starting schedule
        if (auto_settings_.sync_sensor_bw_mode) {
            SetISPDayNightMode(was_night_mode);
        }
        // Start schedule monitoring thread
        schedule_thread_running_ = true;
        schedule_thread_ = std::thread(&IRControl::ScheduleMonitorLoop, this);
        
    } else if (mode == DayNightAutoMode::SwCds) {
        if (!InitSwCds()) {
            spdlog::warn("Failed to initialize SW CDS");
            std::lock_guard<std::mutex> lock(g_mutex);
            auto_settings_.mode = DayNightAutoMode::Off;
            return false;
        }
        
    } else if (mode == DayNightAutoMode::Auto) {
        // Sync ISP to current hardware state before starting auto detection
        // Check current light level and set appropriate mode
        if (auto_settings_.sync_sensor_bw_mode) {
            SetISPDayNightMode(was_night_mode);
        }
        // Legacy auto mode with threshold-based detection
        auto_thread_running_ = true;
        auto_thread_ = std::thread(&IRControl::AutoDetectionLoop, this);
        
    } else if (mode == DayNightAutoMode::Off) {
        // When turning off, sync ISP to match current hardware state
        if (auto_settings_.sync_sensor_bw_mode) {
            SetISPDayNightMode(is_night_mode_);
        }
    }
    
    spdlog::info("Day/Night auto mode set to: {}", DayNightAutoModeToString(mode));
    return true;
}

bool IRControl::SetDayNightAutoSettings(const DayNightAutoSettings& settings) {
    // Check if mode is changing - need to handle this outside the lock
    DayNightAutoMode old_mode;
    DayNightAutoMode new_mode = settings.mode;
    
    {
        std::lock_guard<std::mutex> lock(g_mutex);
        old_mode = auto_settings_.mode;
    }
    
    // If mode is changing, use SetDayNightAutoMode which handles thread management
    if (old_mode != new_mode) {
        spdlog::info("Day/Night mode changing from {} to {}", 
                     DayNightAutoModeToString(old_mode), 
                     DayNightAutoModeToString(new_mode));
        
        // IMPORTANT: First trigger the mode change which stops old threads
        // Do NOT update auto_settings_ before this, as SetDayNightAutoMode
        // reads auto_settings_.mode to determine what to stop
        if (!SetDayNightAutoMode(new_mode)) {
            spdlog::error("Failed to set day/night auto mode to {}", DayNightAutoModeToString(new_mode));
            return false;
        }
        
        // Now update the rest of the settings (mode already updated by SetDayNightAutoMode)
        {
            std::lock_guard<std::mutex> lock(g_mutex);
            // Preserve the mode that SetDayNightAutoMode set
            DayNightAutoMode current_mode = auto_settings_.mode;
            auto_settings_ = settings;
            auto_settings_.mode = current_mode;
        }
    } else {
        // Mode not changing, just update parameters
        std::lock_guard<std::mutex> lock(g_mutex);
        auto_settings_ = settings;
        
        // If SW CDS is active, update parameters
        if (sw_cds_initialized_ && settings.mode == DayNightAutoMode::SwCds) {
            if (sw_cds_set_parameter(settings.night_ev, settings.day_ev, settings.sensitivity) < 0) {
                spdlog::warn("Failed to update SW CDS parameters");
            }
            sw_cds_set_max_ir_ev(settings.max_ir_ev);
        }
    }
    
    // Save all settings to config (including SW CDS fields)
    config::Set<std::string>("ir.auto.mode", DayNightAutoModeToString(settings.mode));
    config::Set<int>("ir.auto.day_to_night_threshold", settings.day_to_night_threshold);
    config::Set<int>("ir.auto.night_to_day_threshold", settings.night_to_day_threshold);
    config::Set<int>("ir.auto.switch_delay_sec", settings.switch_delay_sec);
    config::Set<std::string>("ir.auto.schedule_night_start", settings.schedule_night_start);
    config::Set<std::string>("ir.auto.schedule_day_start", settings.schedule_day_start);
    config::Set<bool>("ir.auto.sync_sensor_bw_mode", settings.sync_sensor_bw_mode);
    // SW CDS fields
    config::Set<int>("ir.auto.night_ev", settings.night_ev);
    config::Set<int>("ir.auto.day_ev", settings.day_ev);
    config::Set<int>("ir.auto.sensitivity", settings.sensitivity);
    config::Set<int>("ir.auto.max_ir_ev", settings.max_ir_ev);
    config::Set<int>("ir.auto.night_brightness", settings.night_brightness);
    config::Set<bool>("ir.auto.bw_mode_enabled", settings.bw_mode_enabled);
    config::Save();
    
    spdlog::info("Day/Night auto settings updated");
    return true;
}

void IRControl::UpdateCurrentLuma(int luma) {
    std::lock_guard<std::mutex> lock(g_mutex);
    current_luma_ = std::clamp(luma, 0, 255);
}

// ============================================================================
// Status
// ============================================================================
IRControlStatus IRControl::GetStatus() const {
    IRControlStatus status;
    
    std::lock_guard<std::mutex> lock(g_mutex);
    
    status.ir_led_on = is_night_mode_ && ir_led_settings_.enabled;
    status.ir_led_brightness = ir_led_settings_.brightness;
    status.ir_cut_state = ir_cut_state_;
    status.is_night_mode = is_night_mode_;
    status.current_luma = current_luma_;
    status.current_ev = current_ev_;
    status.last_switch_time = last_switch_time_;
    status.last_switch_reason = last_switch_reason_;
    status.sw_cds_initialized = sw_cds_initialized_;
    status.sw_cds_active = sw_cds_thread_running_.load();
    
    return status;
}

// ============================================================================
// Config
// ============================================================================
bool IRControl::SaveToConfig() {
    std::lock_guard<std::mutex> lock(g_mutex);
    
    // IR LED
    config::Set<bool>("ir.led.enabled", ir_led_settings_.enabled);
    config::Set<int>("ir.led.brightness", ir_led_settings_.brightness);
    config::Set<int>("ir.led.pwm_channel", ir_led_settings_.pwm_channel);
    config::Set<int>("ir.led.pwm_frequency", ir_led_settings_.pwm_frequency);
    config::Set<bool>("ir.led.auto_brightness", ir_led_settings_.auto_brightness);
    
    // IR Cut
    config::Set<bool>("ir.cut.enabled", ir_cut_settings_.enabled);
    config::Set<int>("ir.cut.gpio_pin_a", ir_cut_settings_.gpio_pin_a);
    config::Set<int>("ir.cut.gpio_pin_b", ir_cut_settings_.gpio_pin_b);
    config::Set<int>("ir.cut.switch_delay_ms", ir_cut_settings_.switch_delay_ms);
    
    // Auto (includes SW CDS settings)
    config::Set<std::string>("ir.auto.mode", DayNightAutoModeToString(auto_settings_.mode));
    config::Set<int>("ir.auto.day_to_night_threshold", auto_settings_.day_to_night_threshold);
    config::Set<int>("ir.auto.night_to_day_threshold", auto_settings_.night_to_day_threshold);
    config::Set<int>("ir.auto.switch_delay_sec", auto_settings_.switch_delay_sec);
    config::Set<std::string>("ir.auto.schedule_night_start", auto_settings_.schedule_night_start);
    config::Set<std::string>("ir.auto.schedule_day_start", auto_settings_.schedule_day_start);
    config::Set<bool>("ir.auto.sync_sensor_bw_mode", auto_settings_.sync_sensor_bw_mode);
    // SW CDS fields (part of day_night_auto)
    config::Set<int>("ir.auto.night_ev", auto_settings_.night_ev);
    config::Set<int>("ir.auto.day_ev", auto_settings_.day_ev);
    config::Set<int>("ir.auto.sensitivity", auto_settings_.sensitivity);
    config::Set<int>("ir.auto.max_ir_ev", auto_settings_.max_ir_ev);
    config::Set<int>("ir.auto.night_brightness", auto_settings_.night_brightness);
    config::Set<bool>("ir.auto.bw_mode_enabled", auto_settings_.bw_mode_enabled);
    
    return config::Save();
}

bool IRControl::LoadFromConfig() {
    // IR LED
    ir_led_settings_.enabled = config::Get<bool>("ir.led.enabled", false);
    ir_led_settings_.brightness = config::Get<int>("ir.led.brightness", 0);
    ir_led_settings_.pwm_channel = config::Get<int>("ir.led.pwm_channel", PWM_CHANNEL_IR_LED);
    ir_led_settings_.pwm_frequency = config::Get<int>("ir.led.pwm_frequency", 1000);
    ir_led_settings_.auto_brightness = config::Get<bool>("ir.led.auto_brightness", false);
    
    // IR Cut (pulse-based dual GPIO for H-bridge motor)
    ir_cut_settings_.enabled = config::Get<bool>("ir.cut.enabled", false);
    ir_cut_settings_.gpio_pin_a = config::Get<int>("ir.cut.gpio_pin_a", 115);  // L_GPIO19 = IRC_A (night)
    ir_cut_settings_.gpio_pin_b = config::Get<int>("ir.cut.gpio_pin_b", 114);  // L_GPIO18 = IRC_B (day)
    ir_cut_settings_.switch_delay_ms = config::Get<int>("ir.cut.switch_delay_ms", 100);
    
    // Auto (includes SW CDS settings)
    auto mode_str = config::Get<std::string>("ir.auto.mode", "off");
    auto_settings_.mode = StringToDayNightAutoMode(mode_str);
    auto_settings_.day_to_night_threshold = config::Get<int>("ir.auto.day_to_night_threshold", 30);
    auto_settings_.night_to_day_threshold = config::Get<int>("ir.auto.night_to_day_threshold", 60);
    auto_settings_.switch_delay_sec = config::Get<int>("ir.auto.switch_delay_sec", 5);
    auto_settings_.schedule_night_start = config::Get<std::string>("ir.auto.schedule_night_start", "18:00");
    auto_settings_.schedule_day_start = config::Get<std::string>("ir.auto.schedule_day_start", "06:00");
    auto_settings_.sync_sensor_bw_mode = config::Get<bool>("ir.auto.sync_sensor_bw_mode", true);
    // SW CDS fields (part of day_night_auto)
    auto_settings_.night_ev = config::Get<int>("ir.auto.night_ev", 300);
    auto_settings_.day_ev = config::Get<int>("ir.auto.day_ev", 600);
    auto_settings_.sensitivity = config::Get<int>("ir.auto.sensitivity", 5);
    auto_settings_.max_ir_ev = config::Get<int>("ir.auto.max_ir_ev", 1200);
    auto_settings_.night_brightness = config::Get<int>("ir.auto.night_brightness", 80);
    auto_settings_.bw_mode_enabled = config::Get<bool>("ir.auto.bw_mode_enabled", true);
    
    return true;
}

bool IRControl::ResetToDefaults() {
    std::lock_guard<std::mutex> lock(g_mutex);
    
    ir_led_settings_ = IRLedSettings{};
    ir_cut_settings_ = IRCutSettings{};
    auto_settings_ = DayNightAutoSettings{};
    
    return SaveToConfig();
}

void IRControl::SetDayNightCallback(DayNightCallback callback) {
    std::lock_guard<std::mutex> lock(g_mutex);
    day_night_callback_ = std::move(callback);
}

// ============================================================================
// SW CDS (Software Color Day/Night Switching)
// ============================================================================
bool IRControl::InitSwCds() {
    if (sw_cds_initialized_) {
        spdlog::warn("SW CDS already initialized");
        return true;
    }
    
    spdlog::info("Initializing SW CDS (Software Color Day/Night Switching)");
    
    // Check if ISP is already initialized (by HDAL pipeline)
    // If vendor_isp_get_ver() returns a valid string, ISP is ready
    const char* isp_ver = vendor_isp_get_ver();
    if (isp_ver && strlen(isp_ver) > 0) {
        spdlog::info("ISP already initialized (version: {}), using existing handle", isp_ver);
        g_sw_cds_device_handle = 0;  // HD_OK - ISP is ready
        g_isp_initialized_by_us = false;
    } else {
        // Try to initialize ISP
        g_sw_cds_device_handle = vendor_isp_init();
        if (g_sw_cds_device_handle < 0) {
            spdlog::error("Failed to open ISP device for SW CDS");
            return false;
        }
        g_isp_initialized_by_us = true;
    }
    
    // Disable verbose debug output from SW CDS library
    sw_cds_set_dbg_out(0);
    
    // Initialize SW CDS library
    LIGHT_INIT_ITEM item;
    item.night_ev = auto_settings_.night_ev;
    item.day_ev = auto_settings_.day_ev;
    item.sensitive = auto_settings_.sensitivity;
    item.open_device = g_sw_cds_device_handle;
    item.light_info_cb_fp = (LIGHT_INFO_CB_FP)&SwCdsLightInfoCallback;
    
    if (sw_cds_init(g_sw_cds_channel_id, item) < 0) {
        spdlog::error("Failed to initialize SW CDS library");
        vendor_isp_uninit();
        g_sw_cds_device_handle = -1;
        return false;
    }
    
    // Set maximum IR LED EV limit
    sw_cds_set_max_ir_ev(auto_settings_.max_ir_ev);
    
    sw_cds_initialized_ = true;
    
    // Start SW CDS detection thread
    sw_cds_thread_running_ = true;
    sw_cds_thread_ = std::thread(&IRControl::SwCdsDetectionLoop, this);
    
    spdlog::info("SW CDS initialized: night_ev={}, day_ev={}, sensitivity={}", 
                 auto_settings_.night_ev, auto_settings_.day_ev, auto_settings_.sensitivity);
    return true;
}

void IRControl::ShutdownSwCds() {
    if (!sw_cds_initialized_) {
        return;
    }
    
    spdlog::info("Shutting down SW CDS");
    
    // Stop detection thread (signal + notify + join)
    sw_cds_thread_running_ = false;
    g_thread_cv.notify_all();  // Wake up sleeping thread
    if (sw_cds_thread_.joinable()) {
        sw_cds_thread_.join();
    }
    
    // Cleanup SW CDS library
    sw_cds_exit();
    
    // Close ISP device only if we initialized it
    if (g_isp_initialized_by_us && g_sw_cds_device_handle >= 0) {
        vendor_isp_uninit();
    }
    g_sw_cds_device_handle = -1;
    g_isp_initialized_by_us = false;
    
    sw_cds_initialized_ = false;
    spdlog::info("SW CDS shutdown complete");
}

void IRControl::SwCdsDetectionLoop() {
    spdlog::info("SW CDS detection thread started");
    
    while (sw_cds_thread_running_) {
        // Use condition variable for interruptible sleep
        {
            std::unique_lock<std::mutex> lock(g_mutex);
            g_thread_cv.wait_for(lock, std::chrono::seconds(1), [this]() {
                return !sw_cds_thread_running_.load();
            });
        }
        
        if (!sw_cds_initialized_ || !sw_cds_thread_running_) {
            break;
        }
        
        // Check if we're still in SwCds mode - if not, stop
        if (auto_settings_.mode != DayNightAutoMode::SwCds) {
            spdlog::info("SW CDS: Mode changed, stopping detection loop");
            break;
        }
        
        // Update current EV value
        int curr_ev = sw_cds_get_curr_ev();
        if (curr_ev >= 0) {
            current_ev_ = curr_ev;
        }
        
        if (is_night_mode_) {
            // Currently in night mode - check if should switch to day
            if (CheckSwCdsDay()) {
                spdlog::info("SW CDS: Environment is bright enough for day mode (EV={})", current_ev_);
                SwitchToDayMode();
            }
        } else {
            // Currently in day mode - check if should switch to night
            if (CheckSwCdsNight()) {
                spdlog::info("SW CDS: Environment is dark, switching to night mode (EV={})", current_ev_);
                
                // Check for shutdown before long operation
                if (!sw_cds_thread_running_) break;
                
                // Pre-callback before switching to night mode
                sw_cds_night_pre_cb();
                
                // Switch to night mode with SW CDS-specific settings
                // 1. Set ISP to night/mono mode
                if (auto_settings_.bw_mode_enabled) {
                    SetISPDayNightMode(true);
                }
                
                // Wait a bit for ISP to switch (interruptible)
                for (int i = 0; i < 10 && sw_cds_thread_running_; i++) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                }
                if (!sw_cds_thread_running_) break;
                
                // 2. Disengage IR cut filter
                SetIRCutStateInternal(IRCutState::Night);
                
                // 3. Turn on IR LED at 100% brightness (always on in night mode)
                ApplyPWMBrightness(100);
                
                // 4. Update state
                {
                    std::lock_guard<std::mutex> lock(g_mutex);
                    is_night_mode_ = true;
                    last_switch_time_ = GetTimestamp();
                    last_switch_reason_ = "SW CDS detected low light";
                }
                
                // Post-callback after switching to night mode
                sw_cds_night_post_cb();
                
                // Notify callback
                if (day_night_callback_) {
                    day_night_callback_(true);
                }
            }
        }
    }
    
    spdlog::info("SW CDS detection thread stopped");
}

bool IRControl::CheckSwCdsNight() {
    int ret = sw_cds_check_env_night();
    if (ret == E_NOT_READY) {
        spdlog::debug("SW CDS: Check night not ready");
        return false;
    }
    return ret == 1;
}

bool IRControl::CheckSwCdsDay() {
    int ret = sw_cds_check_env_day();
    if (ret == E_NOT_READY) {
        spdlog::debug("SW CDS: Check day not ready");
        return false;
    }
    
    if (ret == 1) {
        // Switch back to day mode
        // 1. Turn off IR LED
        ClosePWM();
        
        // 2. Engage IR cut filter
        SetIRCutState(IRCutState::Day);
        
        // 3. Set ISP to color mode
        if (auto_settings_.bw_mode_enabled) {
            SetISPDayNightMode(false);
        }
        
        // 4. Update state
        {
            std::lock_guard<std::mutex> lock(g_mutex);
            is_night_mode_ = false;
            last_switch_time_ = GetTimestamp();
            last_switch_reason_ = "SW CDS detected bright environment";
        }
        
        // Notify callback
        if (day_night_callback_) {
            day_night_callback_(false);
        }
        
        return true;
    }
    
    return false;
}

// ============================================================================
// Hardware Control - PWM via sysfs interface
// ============================================================================
bool IRControl::ApplyPWMBrightness(int brightness) {
    // Clamp brightness to 0-100
    brightness = std::clamp(brightness, 0, 100);
    
    int channel = ir_led_settings_.pwm_channel;
    std::string pwm_path = fmt::format("{}/pwm{}", PWM_CHIP_PATH, channel);
    
    // Export PWM channel if not already exported
    if (!g_pwm_exported) {
        std::string export_path = fmt::format("{}/export", PWM_CHIP_PATH);
        std::ofstream export_file(export_path);
        if (export_file.is_open()) {
            export_file << channel;
            export_file.close();
            // Wait for sysfs to create the pwm directory
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            g_pwm_exported = true;
            spdlog::info("PWM channel {} exported", channel);
        } else {
            // May already be exported, check if path exists
            if (std::ifstream(pwm_path + "/enable").good()) {
                g_pwm_exported = true;
            } else {
                spdlog::error("Failed to export PWM channel {}", channel);
                return false;
            }
        }
    }
    
    // Set period (in nanoseconds)
    std::string period_path = pwm_path + "/period";
    std::ofstream period_file(period_path);
    if (!period_file.is_open()) {
        spdlog::error("Failed to open {}", period_path);
        return false;
    }
    period_file << PWM_PERIOD_NS;
    period_file.close();
    
    // Set duty cycle (brightness 0-100 maps to 0-period nanoseconds)
    int duty_ns = (PWM_PERIOD_NS * brightness) / 100;
    std::string duty_path = pwm_path + "/duty_cycle";
    std::ofstream duty_file(duty_path);
    if (!duty_file.is_open()) {
        spdlog::error("Failed to open {}", duty_path);
        return false;
    }
    duty_file << duty_ns;
    duty_file.close();
    
    // Set polarity to normal
    std::string polarity_path = pwm_path + "/polarity";
    std::ofstream polarity_file(polarity_path);
    if (polarity_file.is_open()) {
        polarity_file << "normal";
        polarity_file.close();
    }
    
    // Enable PWM
    std::string enable_path = pwm_path + "/enable";
    std::ofstream enable_file(enable_path);
    if (!enable_file.is_open()) {
        spdlog::error("Failed to open {}", enable_path);
        return false;
    }
    enable_file << "1";
    enable_file.close();
    
    spdlog::info("PWM{} enabled: period={}ns, duty={}ns ({}%)", 
                 channel, PWM_PERIOD_NS, duty_ns, brightness);
    return true;
}

bool IRControl::ClosePWM() {
    int channel = ir_led_settings_.pwm_channel;
    std::string pwm_path = fmt::format("{}/pwm{}", PWM_CHIP_PATH, channel);
    
    // Disable PWM
    std::string enable_path = pwm_path + "/enable";
    std::ofstream enable_file(enable_path);
    if (enable_file.is_open()) {
        enable_file << "0";
        enable_file.close();
        spdlog::info("PWM{} disabled", channel);
        return true;
    } else {
        spdlog::warn("PWM{} not exported or already disabled", channel);
        return false;
    }
}

bool IRControl::SetGPIO(int pin, bool value) {
    int linux_gpio = pin; // Config uses direct Linux GPIO numbers (e.g. 114, 115)
    std::string gpio_path = fmt::format("/sys/class/gpio/gpio{}", linux_gpio);
    
    // Export GPIO if not already exported
    if (!std::ifstream(gpio_path).good()) {
        std::ofstream export_file(PROC_GPIO_EXPORT);
        if (export_file.is_open()) {
            export_file << linux_gpio;
            export_file.close();
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }
    
    // Set direction to output
    std::string direction_path = gpio_path + "/direction";
    std::ofstream dir_file(direction_path);
    if (dir_file.is_open()) {
        dir_file << "out";
        dir_file.close();
    }
    
    // Set value
    std::string value_path = gpio_path + "/value";
    std::ofstream val_file(value_path);
    if (!val_file.is_open()) {
        // Only log error if we're still initialized (not during shutdown/reboot)
        if (initialized_) {
            spdlog::error("Failed to open {}", value_path);
        }
        return false;
    }
    
    val_file << (value ? "1" : "0");
    val_file.close();
    
    spdlog::debug("GPIO {} set to {}", linux_gpio, value);
    return true;
}

bool IRControl::GetGPIO(int pin) {
    int linux_gpio = pin; // Config uses direct Linux GPIO numbers
    std::string value_path = fmt::format("/sys/class/gpio/gpio{}/value", linux_gpio);
    
    std::ifstream val_file(value_path);
    if (!val_file.is_open()) {
        return false;
    }
    
    int value = 0;
    val_file >> value;
    return value != 0;
}

// ============================================================================
// Auto Detection Loop
// ============================================================================
void IRControl::AutoDetectionLoop() {
    spdlog::info("Day/Night auto detection thread started");
    
    while (auto_thread_running_) {
        // Use condition variable for interruptible sleep
        {
            std::unique_lock<std::mutex> lock(g_mutex);
            g_thread_cv.wait_for(lock, std::chrono::seconds(1), [this]() {
                return !auto_thread_running_;
            });
        }
        
        if (!initialized_ || !auto_thread_running_) {
            break;
        }
        
        CheckAndSwitchMode();
    }
    
    spdlog::info("Day/Night auto detection thread stopped");
}

void IRControl::CheckAndSwitchMode() {
    std::lock_guard<std::mutex> lock(g_mutex);
    
    // Only run auto detection for Auto or Schedule modes
    // For Off, Day, Night, SwCds, External - don't auto-switch
    if (auto_settings_.mode != DayNightAutoMode::Auto && 
        auto_settings_.mode != DayNightAutoMode::Schedule) {
        return;
    }
    
    bool should_be_night = false;
    
    if (auto_settings_.mode == DayNightAutoMode::Auto) {
        // Use luma threshold
        if (is_night_mode_) {
            // Currently night - check if should switch to day
            if (current_luma_ >= auto_settings_.night_to_day_threshold) {
                luma_above_threshold_count_++;
                luma_below_threshold_count_ = 0;
                
                if (luma_above_threshold_count_ >= auto_settings_.switch_delay_sec) {
                    should_be_night = false;
                    luma_above_threshold_count_ = 0;
                }
            } else {
                should_be_night = true;
                luma_above_threshold_count_ = 0;
            }
        } else {
            // Currently day - check if should switch to night
            if (current_luma_ <= auto_settings_.day_to_night_threshold) {
                luma_below_threshold_count_++;
                luma_above_threshold_count_ = 0;
                
                if (luma_below_threshold_count_ >= auto_settings_.switch_delay_sec) {
                    should_be_night = true;
                    luma_below_threshold_count_ = 0;
                }
            } else {
                should_be_night = false;
                luma_below_threshold_count_ = 0;
            }
        }
    } else if (auto_settings_.mode == DayNightAutoMode::Schedule) {
        // Use time-based schedule
        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);
        auto* tm = std::localtime(&time);
        
        int current_minutes = tm->tm_hour * 60 + tm->tm_min;
        
        // Parse schedule times
        int night_start_minutes = 18 * 60; // Default 18:00
        int day_start_minutes = 6 * 60;    // Default 06:00
        
        // Parse night start
        std::istringstream night_ss(auto_settings_.schedule_night_start);
        int h, m;
        char c;
        if (night_ss >> h >> c >> m) {
            night_start_minutes = h * 60 + m;
        }
        
        // Parse day start
        std::istringstream day_ss(auto_settings_.schedule_day_start);
        if (day_ss >> h >> c >> m) {
            day_start_minutes = h * 60 + m;
        }
        
        // Determine if it's night time
        if (night_start_minutes > day_start_minutes) {
            // Night spans midnight (e.g., 18:00 - 06:00)
            should_be_night = (current_minutes >= night_start_minutes || 
                              current_minutes < day_start_minutes);
        } else {
            // Night doesn't span midnight (unusual but supported)
            should_be_night = (current_minutes >= night_start_minutes && 
                              current_minutes < day_start_minutes);
        }
    }
    
    // Perform switch if needed
    if (should_be_night != is_night_mode_) {
        // Release lock before calling switch functions (they acquire it)
        g_mutex.unlock();
        
        if (should_be_night) {
            SwitchToNightMode();
        } else {
            SwitchToDayMode();
        }
        
        g_mutex.lock();
    }
}

// ============================================================================
// Schedule Monitor Loop
// ============================================================================
void IRControl::ScheduleMonitorLoop() {
    spdlog::info("Schedule monitor thread started");
    
    while (schedule_thread_running_) {
        // Use condition variable for interruptible sleep
        {
            std::unique_lock<std::mutex> lock(g_mutex);
            g_thread_cv.wait_for(lock, std::chrono::seconds(30), [this]() {
                return !schedule_thread_running_;
            });
        }
        
        if (!initialized_ || !schedule_thread_running_) {
            break;
        }
        
        if (auto_settings_.mode != DayNightAutoMode::Schedule) {
            break;
        }
        
        // Get current time
        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);
        auto* tm = std::localtime(&time);
        
        int current_minutes = tm->tm_hour * 60 + tm->tm_min;
        
        // Parse schedule times
        int night_start_minutes = 18 * 60; // Default 18:00
        int day_start_minutes = 6 * 60;    // Default 06:00
        
        // Parse night start
        std::istringstream night_ss(auto_settings_.schedule_night_start);
        int h, m;
        char c;
        if (night_ss >> h >> c >> m) {
            night_start_minutes = h * 60 + m;
        }
        
        // Parse day start
        std::istringstream day_ss(auto_settings_.schedule_day_start);
        if (day_ss >> h >> c >> m) {
            day_start_minutes = h * 60 + m;
        }
        
        // Determine if it should be night time
        bool should_be_night = false;
        if (night_start_minutes > day_start_minutes) {
            // Night spans midnight (e.g., 18:00 - 06:00)
            should_be_night = (current_minutes >= night_start_minutes || 
                              current_minutes < day_start_minutes);
        } else {
            // Night doesn't span midnight (unusual but supported)
            should_be_night = (current_minutes >= night_start_minutes && 
                              current_minutes < day_start_minutes);
        }
        
        // Perform switch if needed
        if (should_be_night != is_night_mode_) {
            if (should_be_night) {
                spdlog::info("Schedule: Switching to Night mode (time: {:02d}:{:02d})", 
                            tm->tm_hour, tm->tm_min);
                SwitchToNightMode();
            } else {
                spdlog::info("Schedule: Switching to Day mode (time: {:02d}:{:02d})", 
                            tm->tm_hour, tm->tm_min);
                SwitchToDayMode();
            }
        }
    }
    
    spdlog::info("Schedule monitor thread stopped");
}

// ============================================================================
// Helper Functions
// ============================================================================
std::string IRCutStateToString(IRCutState state) {
    switch (state) {
        case IRCutState::Day: return "day";
        case IRCutState::Night: return "night";
        default: return "unknown";
    }
}

IRCutState StringToIRCutState(const std::string& str) {
    if (str == "day") return IRCutState::Day;
    if (str == "night") return IRCutState::Night;
    return IRCutState::Unknown;
}

std::string DayNightAutoModeToString(DayNightAutoMode mode) {
    switch (mode) {
        case DayNightAutoMode::Off: return "off";
        case DayNightAutoMode::Auto: return "auto";
        case DayNightAutoMode::SwCds: return "sw_cds";
        case DayNightAutoMode::Schedule: return "schedule";
        case DayNightAutoMode::External: return "external";
        case DayNightAutoMode::Day: return "day";
        case DayNightAutoMode::Night: return "night";
        default: return "off";
    }
}

DayNightAutoMode StringToDayNightAutoMode(const std::string& str) {
    if (str == "off") return DayNightAutoMode::Off;
    if (str == "auto") return DayNightAutoMode::Auto;
    if (str == "sw_cds") return DayNightAutoMode::SwCds;
    if (str == "schedule") return DayNightAutoMode::Schedule;
    if (str == "external") return DayNightAutoMode::External;
    if (str == "day") return DayNightAutoMode::Day;
    if (str == "night") return DayNightAutoMode::Night;
    return DayNightAutoMode::Off;
}

} // namespace platform
} // namespace ipcam
