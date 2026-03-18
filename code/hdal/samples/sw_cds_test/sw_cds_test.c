/**
 * SW CDS (Software Color Day/Night Switching) Test
 * 
 * This sample tests the SW CDS functionality with IR LED (PWM) and IR Cut control.
 * It demonstrates the complete day/night switching workflow using the NT98538 platform.
 * 
 * Usage:
 *   sw_cds_test           - Run SW CDS automatic detection
 *   sw_cds_test info      - Show current EV and light info only
 *   sw_cds_test day       - Force switch to day mode
 *   sw_cds_test night     - Force switch to night mode
 *   sw_cds_test led on    - Turn IR LED on
 *   sw_cds_test led off   - Turn IR LED off
 *   sw_cds_test led <0-100> - Set IR LED brightness
 * 
 * Hardware Mapping (NT98538):
 *   IR LED PWM:  Channel 11 (P_GPIO11)
 *   IR Cut A:    L_GPIO19 (GPIO 115) - Night mode pulse
 *   IR Cut B:    L_GPIO18 (GPIO 114) - Day mode pulse
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>

#include "light_lib.h"
#include "vendor_isp.h"

//============================================================================
// Platform-specific GPIO/PWM defines for NT98538
//============================================================================
#define PWM_CHIP_PATH      "/sys/class/pwm/pwmchip0"
#define PWM_CHANNEL_IR_LED 11
#define PWM_PERIOD_NS      1000  // 1 MHz PWM frequency
#define GPIO_SYSFS_PATH    "/sys/class/gpio"
#define GPIO_IRC_A         115   // L_GPIO19 - Night mode pulse
#define GPIO_IRC_B         114   // L_GPIO18 - Day mode pulse
#define IRC_PULSE_MS       100   // Pulse duration for IR cut motor

//============================================================================
// SW CDS Parameters
//============================================================================
#define DEFAULT_NIGHT_EV   200
#define DEFAULT_DAY_EV     300
#define DEFAULT_SENSITIVE  5
#define MAX_IR_EV          1200

//============================================================================
// Global Variables
//============================================================================
typedef enum {
    MODE_DAY = 0,
    MODE_NIGHT = 1
} DayNightMode;

static int g_running = 1;
static int g_isp_handle = -1;
static DayNightMode g_current_mode = MODE_DAY;
static CA_DATA g_ca_data;
static UINT32 g_channel_id = 0;

//============================================================================
// Signal Handler
//============================================================================
void signal_handler(int sig) {
    printf("\nReceived signal %d, shutting down...\n", sig);
    g_running = 0;
}

//============================================================================
// Helper: Write string to file
//============================================================================
static int write_to_file(const char* path, const char* value) {
    FILE* f = fopen(path, "w");
    if (!f) {
        return -1;
    }
    fprintf(f, "%s", value);
    fclose(f);
    return 0;
}

static int write_int_to_file(const char* path, int value) {
    char buf[32];
    snprintf(buf, sizeof(buf), "%d", value);
    return write_to_file(path, buf);
}

//============================================================================
// PWM Control for IR LED
//============================================================================
static int pwm_export(int channel) {
    char path[128];
    snprintf(path, sizeof(path), "%s/pwm%d", PWM_CHIP_PATH, channel);
    
    // Check if already exported
    if (access(path, F_OK) == 0) {
        printf("PWM%d already exported\n", channel);
        return 0;
    }
    
    snprintf(path, sizeof(path), "%s/export", PWM_CHIP_PATH);
    return write_int_to_file(path, channel);
}

static int pwm_set_period(int channel, int period_ns) {
    char path[128];
    snprintf(path, sizeof(path), "%s/pwm%d/period", PWM_CHIP_PATH, channel);
    return write_int_to_file(path, period_ns);
}

static int pwm_set_duty_cycle(int channel, int duty_ns) {
    char path[128];
    snprintf(path, sizeof(path), "%s/pwm%d/duty_cycle", PWM_CHIP_PATH, channel);
    return write_int_to_file(path, duty_ns);
}

static int pwm_enable(int channel, int enable) {
    char path[128];
    snprintf(path, sizeof(path), "%s/pwm%d/enable", PWM_CHIP_PATH, channel);
    return write_int_to_file(path, enable);
}

int set_ir_led_brightness(int brightness) {
    if (brightness < 0) brightness = 0;
    if (brightness > 100) brightness = 100;
    
    if (brightness == 0) {
        // Turn off LED
        pwm_enable(PWM_CHANNEL_IR_LED, 0);
        printf("IR LED OFF\n");
        return 0;
    }
    
    // Export and configure PWM
    pwm_export(PWM_CHANNEL_IR_LED);
    
    // Set period first (required before duty_cycle)
    pwm_set_period(PWM_CHANNEL_IR_LED, PWM_PERIOD_NS);
    
    // Calculate duty cycle based on brightness percentage
    int duty_ns = (PWM_PERIOD_NS * brightness) / 100;
    pwm_set_duty_cycle(PWM_CHANNEL_IR_LED, duty_ns);
    
    // Enable PWM
    pwm_enable(PWM_CHANNEL_IR_LED, 1);
    
    printf("IR LED ON - brightness: %d%%, duty: %d ns\n", brightness, duty_ns);
    return 0;
}

//============================================================================
// GPIO Control for IR Cut Filter
//============================================================================
static int gpio_export(int pin) {
    char path[128];
    snprintf(path, sizeof(path), "%s/gpio%d", GPIO_SYSFS_PATH, pin);
    
    // Check if already exported
    if (access(path, F_OK) == 0) {
        return 0;
    }
    
    snprintf(path, sizeof(path), "%s/export", GPIO_SYSFS_PATH);
    return write_int_to_file(path, pin);
}

static int gpio_set_direction(int pin, const char* direction) {
    char path[128];
    snprintf(path, sizeof(path), "%s/gpio%d/direction", GPIO_SYSFS_PATH, pin);
    return write_to_file(path, direction);
}

static int gpio_set_value(int pin, int value) {
    char path[128];
    snprintf(path, sizeof(path), "%s/gpio%d/value", GPIO_SYSFS_PATH, pin);
    return write_int_to_file(path, value);
}

int init_ir_cut(void) {
    // Export both GPIO pins
    if (gpio_export(GPIO_IRC_A) < 0) {
        printf("Warning: Failed to export GPIO%d\n", GPIO_IRC_A);
    }
    if (gpio_export(GPIO_IRC_B) < 0) {
        printf("Warning: Failed to export GPIO%d\n", GPIO_IRC_B);
    }
    
    // Set as outputs
    gpio_set_direction(GPIO_IRC_A, "out");
    gpio_set_direction(GPIO_IRC_B, "out");
    
    // Initialize to low
    gpio_set_value(GPIO_IRC_A, 0);
    gpio_set_value(GPIO_IRC_B, 0);
    
    printf("IR Cut initialized: IRC_A=GPIO%d, IRC_B=GPIO%d\n", GPIO_IRC_A, GPIO_IRC_B);
    return 0;
}

int set_ir_cut_day(void) {
    // Day mode: Pulse IRC_B to engage IR cut filter (blocks IR)
    printf("IR Cut: Switching to DAY mode (pulse GPIO%d)\n", GPIO_IRC_B);
    gpio_set_value(GPIO_IRC_A, 0);  // Ensure IRC_A is low
    gpio_set_value(GPIO_IRC_B, 1);  // Pulse IRC_B high
    usleep(IRC_PULSE_MS * 1000);
    gpio_set_value(GPIO_IRC_B, 0);  // End pulse
    return 0;
}

int set_ir_cut_night(void) {
    // Night mode: Pulse IRC_A to disengage IR cut filter (allows IR)
    printf("IR Cut: Switching to NIGHT mode (pulse GPIO%d)\n", GPIO_IRC_A);
    gpio_set_value(GPIO_IRC_B, 0);  // Ensure IRC_B is low
    gpio_set_value(GPIO_IRC_A, 1);  // Pulse IRC_A high
    usleep(IRC_PULSE_MS * 1000);
    gpio_set_value(GPIO_IRC_A, 0);  // End pulse
    return 0;
}

//============================================================================
// ISP Day/Night Mode Control
//============================================================================
int set_isp_mode(DayNightMode mode) {
    IQT_NIGHT_MODE iq_night_mode;
    memset(&iq_night_mode, 0, sizeof(iq_night_mode));
    iq_night_mode.id = (IQ_ID)g_channel_id;
    iq_night_mode.mode = (mode == MODE_NIGHT) ? IQ_UI_NIGHT_MODE_ON : IQ_UI_NIGHT_MODE_OFF;
    
    int ret = vendor_isp_set_iq(IQT_ITEM_NIGHT_MODE, &iq_night_mode);
    if (ret < 0) {
        printf("Failed to set ISP mode: %d\n", ret);
        return -1;
    }
    
    printf("ISP mode set to: %s\n", (mode == MODE_NIGHT) ? "NIGHT (mono)" : "DAY (color)");
    return 0;
}

//============================================================================
// Combined Day/Night Mode Switch
//============================================================================
void switch_to_day_mode(void) {
    printf("=== SWITCHING TO DAY MODE ===\n");
    
    // 1. Turn off IR LED
    set_ir_led_brightness(0);
    
    // 2. Engage IR cut filter
    set_ir_cut_day();
    
    // 3. Set ISP to color mode
    set_isp_mode(MODE_DAY);
    
    g_current_mode = MODE_DAY;
    printf("=== DAY MODE ACTIVE ===\n\n");
}

void switch_to_night_mode(int led_brightness) {
    printf("=== SWITCHING TO NIGHT MODE ===\n");
    
    // Call pre-callback for SW CDS library
    sw_cds_night_pre_cb();
    
    // 1. Set ISP to mono mode
    set_isp_mode(MODE_NIGHT);
    
    // Wait for ISP to switch
    usleep(500000);  // 500ms
    
    // 2. Disengage IR cut filter
    set_ir_cut_night();
    
    // 3. Turn on IR LED
    set_ir_led_brightness(led_brightness);
    
    // Call post-callback for SW CDS library
    sw_cds_night_post_cb();
    
    g_current_mode = MODE_NIGHT;
    printf("=== NIGHT MODE ACTIVE ===\n\n");
}

//============================================================================
// SW CDS Light Info Callback
//============================================================================
int light_info_callback(LIGHT_INFO* light_info) {
    HD_RESULT ret;
    AWBT_CA awb_ca;
    AET_STATUS_INFO ae_status;
    AET_CURVE_GEN_MOVIE ae_curve_gen_movie;
    AWBT_STATUS awb_sta;
    
    memset(&awb_ca, 0, sizeof(awb_ca));
    awb_ca.id = (AWB_ID)g_channel_id;
    ret = vendor_isp_get_awb(AWBT_ITEM_CA, &awb_ca);
    if (ret < 0) {
        return E_GET_DEV_FAIL;
    }
    
    light_info->ca = &g_ca_data;
    memcpy(&light_info->ca->win_num_x, &awb_ca.ca.win_num_x, sizeof(CA_DATA));
    
    memset(&ae_status, 0, sizeof(ae_status));
    ae_status.id = (AE_ID)g_channel_id;
    ret = vendor_isp_get_ae(AET_ITEM_STATUS, &ae_status);
    if (ret < 0) {
        return E_GET_DEV_FAIL;
    }
    
    light_info->iso_expt_fn[0] = ae_status.status_info.iso_gain[0];
    light_info->iso_expt_fn[1] = ae_status.status_info.expotime[0];
    
    memset(&ae_curve_gen_movie, 0, sizeof(ae_curve_gen_movie));
    ae_curve_gen_movie.id = (AE_ID)g_channel_id;
    ret = vendor_isp_get_ae(AET_ITEM_CURVE_GEN_MOVIE, &ae_curve_gen_movie);
    light_info->iso_expt_fn[2] = ae_curve_gen_movie.curve_gen_movie.iso_calcoef;
    
    memset(&awb_sta, 0, sizeof(awb_sta));
    awb_sta.id = (AWB_ID)g_channel_id;
    ret = vendor_isp_get_awb(AWBT_ITEM_STATUS, &awb_sta);
    if (ret < 0) {
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

//============================================================================
// Show Current Light Info
//============================================================================
void show_light_info(void) {
    int curr_ev = sw_cds_get_curr_ev();
    printf("Current EV: %d\n", curr_ev);
    printf("Current Mode: %s\n", g_current_mode == MODE_DAY ? "DAY" : "NIGHT");
}

//============================================================================
// Run SW CDS Auto Detection Loop
//============================================================================
void run_sw_cds_loop(void) {
    int ret;
    int led_brightness = 75;  // Default LED brightness for night mode
    
    printf("================================================\n");
    printf("SW CDS Auto Detection Started\n");
    printf("Night EV Threshold: %d\n", DEFAULT_NIGHT_EV);
    printf("Day EV Threshold: %d\n", DEFAULT_DAY_EV);
    printf("Sensitivity: %d\n", DEFAULT_SENSITIVE);
    printf("Press Ctrl+C to stop\n");
    printf("================================================\n\n");
    
    while (g_running) {
        // Get current EV
        int curr_ev = sw_cds_get_curr_ev();
        if (curr_ev >= 0) {
            printf("[EV: %5d] Mode: %s | ", curr_ev, 
                   g_current_mode == MODE_DAY ? "DAY  " : "NIGHT");
        }
        
        if (g_current_mode == MODE_DAY) {
            // Check if should switch to night
            ret = sw_cds_check_env_night();
            if (ret == E_NOT_READY) {
                printf("Night check: not ready\n");
            } else if (ret == 1) {
                printf("Night check: TRIGGERED!\n");
                switch_to_night_mode(led_brightness);
            } else {
                printf("Night check: no\n");
            }
        } else {
            // Check if should switch to day
            ret = sw_cds_check_env_day();
            if (ret == E_NOT_READY) {
                printf("Day check: not ready\n");
            } else if (ret == 1) {
                printf("Day check: TRIGGERED!\n");
                switch_to_day_mode();
            } else {
                printf("Day check: no\n");
            }
        }
        
        sleep(1);
    }
}

//============================================================================
// Print Usage
//============================================================================
void print_usage(const char* prog) {
    printf("SW CDS Test for NT98538 Platform\n");
    printf("================================\n");
    printf("Usage: %s [command] [args]\n\n", prog);
    printf("Commands:\n");
    printf("  (none)       Run SW CDS auto detection loop\n");
    printf("  info         Show current EV and light info\n");
    printf("  day          Force switch to day mode\n");
    printf("  night        Force switch to night mode\n");
    printf("  led on       Turn IR LED on (75%% brightness)\n");
    printf("  led off      Turn IR LED off\n");
    printf("  led <0-100>  Set IR LED brightness\n");
    printf("  ircut day    Pulse IR cut to day position\n");
    printf("  ircut night  Pulse IR cut to night position\n");
    printf("\nHardware:\n");
    printf("  IR LED PWM:  Channel %d\n", PWM_CHANNEL_IR_LED);
    printf("  IR Cut A:    GPIO %d (night pulse)\n", GPIO_IRC_A);
    printf("  IR Cut B:    GPIO %d (day pulse)\n", GPIO_IRC_B);
}

//============================================================================
// Main
//============================================================================
int main(int argc, char* argv[]) {
    int ret;
    LIGHT_INIT_ITEM item;
    
    // Setup signal handler
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    
    // Handle simple LED commands that don't need ISP
    if (argc >= 2 && strcmp(argv[1], "led") == 0) {
        if (argc < 3) {
            printf("Usage: %s led <on|off|0-100>\n", argv[0]);
            return 1;
        }
        if (strcmp(argv[2], "on") == 0) {
            return set_ir_led_brightness(75);
        } else if (strcmp(argv[2], "off") == 0) {
            return set_ir_led_brightness(0);
        } else {
            int brightness = atoi(argv[2]);
            return set_ir_led_brightness(brightness);
        }
    }
    
    // Handle IR cut commands
    if (argc >= 2 && strcmp(argv[1], "ircut") == 0) {
        init_ir_cut();
        if (argc < 3) {
            printf("Usage: %s ircut <day|night>\n", argv[0]);
            return 1;
        }
        if (strcmp(argv[2], "day") == 0) {
            return set_ir_cut_day();
        } else if (strcmp(argv[2], "night") == 0) {
            return set_ir_cut_night();
        }
        return 0;
    }
    
    // Initialize ISP
    printf("Initializing ISP...\n");
    g_isp_handle = vendor_isp_init();
    if (g_isp_handle < 0) {
        printf("ERROR: Failed to initialize ISP\n");
        return -1;
    }
    printf("ISP initialized (handle: %d)\n", g_isp_handle);
    
    // Initialize SW CDS library
    printf("Initializing SW CDS library...\n");
    sw_cds_set_dbg_out(1);  // Enable debug output
    
    memset(&item, 0, sizeof(item));
    item.night_ev = DEFAULT_NIGHT_EV;
    item.day_ev = DEFAULT_DAY_EV;
    item.sensitive = DEFAULT_SENSITIVE;
    item.open_device = g_isp_handle;
    item.light_info_cb_fp = (LIGHT_INFO_CB_FP)&light_info_callback;
    
    ret = sw_cds_init(g_channel_id, item);
    if (ret < 0) {
        printf("ERROR: Failed to initialize SW CDS library\n");
        vendor_isp_uninit();
        return -1;
    }
    
    sw_cds_set_max_ir_ev(MAX_IR_EV);
    printf("SW CDS initialized\n\n");
    
    // Initialize hardware
    init_ir_cut();
    
    // Start in day mode
    switch_to_day_mode();
    
    // Handle commands
    if (argc < 2) {
        // No arguments - run SW CDS loop
        run_sw_cds_loop();
    } else if (strcmp(argv[1], "info") == 0) {
        show_light_info();
    } else if (strcmp(argv[1], "day") == 0) {
        switch_to_day_mode();
    } else if (strcmp(argv[1], "night") == 0) {
        switch_to_night_mode(75);
    } else if (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0) {
        print_usage(argv[0]);
    } else {
        printf("Unknown command: %s\n", argv[1]);
        print_usage(argv[0]);
    }
    
    // Cleanup
    printf("\nCleaning up...\n");
    sw_cds_exit();
    vendor_isp_uninit();
    printf("Done.\n");
    
    return 0;
}
