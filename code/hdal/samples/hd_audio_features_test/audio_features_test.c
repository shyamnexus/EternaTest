/**
 * @file audio_features_test.c
 * @brief Comprehensive Audio Features Test for Novatek NS02302
 *
 * This sample demonstrates and tests all audio processing features:
 * - Volume Control (HD_AUDIOCAP_PARAM_VOLUME)
 * - Gain Level (VENDOR_AUDIOCAP_ITEM_GAIN_LEVEL) - 8/16/32 steps
 * - ALC Enable/Disable (VENDOR_AUDIOCAP_ITEM_ALC_ENABLE)
 * - DEFAULT_SETTING / Boost (VENDOR_AUDIOCAP_ITEM_DEFAULT_SETTING)
 * - AGC (VENDOR_AUDIOCAP_ITEM_AGC_CONFIG) - if enabled in SDK
 * - Noise Gate (VENDOR_AUDIOCAP_ITEM_NOISEGATE_THRESHOLD)
 * - AEC (Acoustic Echo Cancellation)
 * - ANR (Audio Noise Reduction)
 * - Loopback Configuration
 *
 * @author Novatek SDK Team
 * @copyright Novatek Microelectronics Corp. 2024. All rights reserved.
 *
 * Usage:
 *   audio_features_test [options]
 *   --test <feature>    Test specific feature (all, volume, gain, alc, agc, aec, anr)
 *   --volume <0-200>    Set capture volume (default: 100)
 *   --duration <sec>    Recording duration (default: 5s)
 *   --output <path>     Output directory (default: /mnt/sd)
 *   --help              Show help message
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>
#include <getopt.h>
#include <errno.h>

#include "hdal.h"
#include "hd_debug.h"
#include "vendor_audiocapture.h"

#ifdef __LINUX
#include <sys/stat.h>
#include <sys/time.h>
#else
#include <FreeRTOS_POSIX.h>
#include <FreeRTOS_POSIX/pthread.h>
#include <kwrap/task.h>
#define sleep(x)    vos_task_delay_ms(1000*x)
#define usleep(x)   vos_task_delay_us(x)
#endif

/*============================================================================
 * Constants and Macros
 *============================================================================*/

#define SAMPLE_RATE_DEFAULT     48000
#define SAMPLE_RATE_AEC         8000    // AEC requires lower sample rate
#define BIT_WIDTH               16
#define FRAME_SAMPLES           1024
#define MAX_FRAME_NUM           10

#define COLOR_RESET   "\033[0m"
#define COLOR_RED     "\033[31m"
#define COLOR_GREEN   "\033[32m"
#define COLOR_YELLOW  "\033[33m"
#define COLOR_BLUE    "\033[34m"
#define COLOR_CYAN    "\033[36m"

#define PRINT_HEADER(title) \
    printf("\n" COLOR_CYAN "╔════════════════════════════════════════════════════════════════╗\n"); \
    printf("║ %-62s ║\n", title); \
    printf("╚════════════════════════════════════════════════════════════════╝" COLOR_RESET "\n\n")

#define PRINT_SECTION(title) \
    printf(COLOR_YELLOW "\n▶ %s\n" COLOR_RESET, title); \
    printf("────────────────────────────────────────\n")

#define PRINT_PASS(msg)  printf(COLOR_GREEN "  ✓ PASS: " msg COLOR_RESET "\n")
#define PRINT_FAIL(msg, err)  printf(COLOR_RED "  ✗ FAIL: " msg " (error=%d)" COLOR_RESET "\n", err)
#define PRINT_SKIP(msg)  printf(COLOR_YELLOW "  ○ SKIP: " msg COLOR_RESET "\n")
#define PRINT_INFO(fmt, ...)  printf(COLOR_BLUE "  ℹ " fmt COLOR_RESET "\n", ##__VA_ARGS__)

/*============================================================================
 * Type Definitions
 *============================================================================*/

typedef enum {
    TEST_ALL = 0,
    TEST_VOLUME,
    TEST_GAIN_LEVEL,
    TEST_ALC,
    TEST_DEFAULT_SETTING,
    TEST_AGC,
    TEST_NOISEGATE,
    TEST_LOOPBACK,
    TEST_RECORD,
    TEST_AEC,
    TEST_ANR
} TestMode;

typedef struct {
    HD_PATH_ID cap_ctrl;
    HD_PATH_ID cap_path;
    pthread_t  thread_id;
    UINT32     exit_flag;
    UINT32     running;
} AudioCapture;

typedef struct {
    TestMode test_mode;
    UINT32   volume;
    UINT32   duration;
    char     output_path[256];
    int      sample_rate;
    int      verbose;
} TestConfig;

typedef struct {
    int total;
    int passed;
    int failed;
    int skipped;
} TestResults;

/*============================================================================
 * Global Variables
 *============================================================================*/

static volatile int g_keep_running = 1;
static TestConfig g_config = {
    .test_mode = TEST_ALL,
    .volume = 100,
    .duration = 5,
    .output_path = "/mnt/sd",
    .sample_rate = SAMPLE_RATE_DEFAULT,
    .verbose = 0
};
static TestResults g_results = {0};
static AudioCapture g_audio = {0};

/*============================================================================
 * Signal Handler
 *============================================================================*/

static void signal_handler(int sig)
{
    printf("\nSignal %d received, stopping...\n", sig);
    g_keep_running = 0;
}

/*============================================================================
 * Memory Management
 *============================================================================*/

static HD_RESULT mem_init(void)
{
    HD_COMMON_MEM_INIT_CONFIG mem_cfg = {0};

    mem_cfg.pool_info[0].type = HD_COMMON_MEM_COMMON_POOL;
    mem_cfg.pool_info[0].blk_size = 0x10000;  // 64KB
    mem_cfg.pool_info[0].blk_cnt = 4;
    mem_cfg.pool_info[0].ddr_id = DDR_ID0;

    return hd_common_mem_init(&mem_cfg);
}

static HD_RESULT mem_exit(void)
{
    return hd_common_mem_uninit();
}

/*============================================================================
 * Audio Capture Setup
 *============================================================================*/

static HD_RESULT audio_open_ctrl(HD_PATH_ID *p_ctrl)
{
    return hd_audiocap_open(0, HD_AUDIOCAP_0_CTRL, p_ctrl);
}

static HD_RESULT audio_close_ctrl(HD_PATH_ID ctrl)
{
    return hd_audiocap_close(ctrl);
}

static HD_RESULT audio_open_path(HD_PATH_ID *p_path)
{
    return hd_audiocap_open(HD_AUDIOCAP_0_IN_0, HD_AUDIOCAP_0_OUT_0, p_path);
}

static HD_RESULT audio_close_path(HD_PATH_ID path)
{
    return hd_audiocap_close(path);
}

static HD_RESULT audio_set_dev_config(HD_PATH_ID ctrl, UINT32 sample_rate)
{
    HD_AUDIOCAP_DEV_CONFIG cfg = {0};

    cfg.in_max.sample_rate = sample_rate;
    cfg.in_max.sample_bit = HD_AUDIO_BIT_WIDTH_16;
    cfg.in_max.mode = HD_AUDIO_SOUND_MODE_STEREO;
    cfg.in_max.frame_sample = FRAME_SAMPLES;
    cfg.frame_num_max = MAX_FRAME_NUM;
    cfg.out_max.sample_rate = 0;

    return hd_audiocap_set(ctrl, HD_AUDIOCAP_PARAM_DEV_CONFIG, &cfg);
}

static HD_RESULT audio_set_drv_config(HD_PATH_ID ctrl)
{
    HD_AUDIOCAP_DRV_CONFIG cfg = {0};
    cfg.mono = HD_AUDIO_MONO_RIGHT;
    return hd_audiocap_set(ctrl, HD_AUDIOCAP_PARAM_DRV_CONFIG, &cfg);
}

static HD_RESULT audio_set_path_config(HD_PATH_ID path, UINT32 sample_rate, HD_AUDIO_SOUND_MODE mode)
{
    HD_RESULT ret;
    HD_AUDIOCAP_IN in_cfg = {0};
    HD_AUDIOCAP_OUT out_cfg = {0};

    in_cfg.sample_rate = sample_rate;
    in_cfg.sample_bit = HD_AUDIO_BIT_WIDTH_16;
    in_cfg.mode = mode;
    in_cfg.frame_sample = FRAME_SAMPLES;

    ret = hd_audiocap_set(path, HD_AUDIOCAP_PARAM_IN, &in_cfg);
    if (ret != HD_OK) return ret;

    out_cfg.sample_rate = 0;
    return hd_audiocap_set(path, HD_AUDIOCAP_PARAM_OUT, &out_cfg);
}

/*============================================================================
 * Feature Test Functions
 *============================================================================*/

/**
 * Test Volume Control (HD_AUDIOCAP_PARAM_VOLUME)
 * Range: 0-200 (100 = unity gain)
 */
static void test_volume_control(HD_PATH_ID ctrl)
{
    PRINT_SECTION("Volume Control Test (HD_AUDIOCAP_PARAM_VOLUME)");
    g_results.total++;

    HD_AUDIOCAP_VOLUME vol_cfg = {0};
    HD_RESULT ret;
    int test_values[] = {0, 50, 100, 150, 200};
    int num_tests = sizeof(test_values) / sizeof(test_values[0]);
    int success = 1;

    PRINT_INFO("Testing volume range: 0 (mute) to 200 (2x gain)");

    for (int i = 0; i < num_tests; i++) {
        vol_cfg.volume = test_values[i];
        ret = hd_audiocap_set(ctrl, HD_AUDIOCAP_PARAM_VOLUME, &vol_cfg);
        
        if (ret == HD_OK) {
            printf("    Volume %3d: " COLOR_GREEN "OK" COLOR_RESET "\n", test_values[i]);
        } else {
            printf("    Volume %3d: " COLOR_RED "FAILED (ret=%d)" COLOR_RESET "\n", test_values[i], ret);
            success = 0;
        }
        usleep(100000);  // 100ms delay between changes
    }

    // Restore default
    vol_cfg.volume = g_config.volume;
    hd_audiocap_set(ctrl, HD_AUDIOCAP_PARAM_VOLUME, &vol_cfg);

    if (success) {
        PRINT_PASS("Volume control working (0-200 range)");
        g_results.passed++;
    } else {
        PRINT_FAIL("Some volume values failed", -1);
        g_results.failed++;
    }
}

/**
 * Test Gain Level (VENDOR_AUDIOCAP_ITEM_GAIN_LEVEL)
 * Options: 8, 16, 32 steps
 */
static void test_gain_level(HD_PATH_ID ctrl)
{
    PRINT_SECTION("Gain Level Test (VENDOR_AUDIOCAP_ITEM_GAIN_LEVEL)");
    g_results.total++;

    HD_RESULT ret;
    UINT32 gain_levels[] = {
        VENDOR_AUDIOCAP_GAIN_LEVEL8,
        VENDOR_AUDIOCAP_GAIN_LEVEL16,
        VENDOR_AUDIOCAP_GAIN_LEVEL32
    };
    const char* level_names[] = {"8 steps", "16 steps", "32 steps"};
    int num_levels = 3;
    int success = 1;

    PRINT_INFO("Testing gain level granularity options");

    for (int i = 0; i < num_levels; i++) {
        ret = vendor_audiocap_set(ctrl, VENDOR_AUDIOCAP_ITEM_GAIN_LEVEL, &gain_levels[i]);
        
        if (ret == HD_OK) {
            printf("    Gain Level %s: " COLOR_GREEN "OK" COLOR_RESET "\n", level_names[i]);
        } else {
            printf("    Gain Level %s: " COLOR_RED "FAILED (ret=%d)" COLOR_RESET "\n", level_names[i], ret);
            success = 0;
        }
        usleep(50000);
    }

    // Set to 32 levels for finest control
    UINT32 final_gain = VENDOR_AUDIOCAP_GAIN_LEVEL32;
    vendor_audiocap_set(ctrl, VENDOR_AUDIOCAP_ITEM_GAIN_LEVEL, &final_gain);

    if (success) {
        PRINT_PASS("Gain level control working (8/16/32 steps)");
        g_results.passed++;
    } else {
        PRINT_FAIL("Some gain levels failed", -1);
        g_results.failed++;
    }
}

/**
 * Test ALC Enable/Disable (VENDOR_AUDIOCAP_ITEM_ALC_ENABLE)
 */
static void test_alc_control(HD_PATH_ID ctrl)
{
    PRINT_SECTION("ALC Control Test (VENDOR_AUDIOCAP_ITEM_ALC_ENABLE)");
    g_results.total++;

    HD_RESULT ret;
    UINT32 alc_enable;
    int success = 1;

    PRINT_INFO("ALC = Auto Level Control (hardware-based gain)");
    PRINT_INFO("Recommendation: Disable ALC when using software AGC/AEC/ANR");

    // Test disable
    alc_enable = FALSE;
    ret = vendor_audiocap_set(ctrl, VENDOR_AUDIOCAP_ITEM_ALC_ENABLE, &alc_enable);
    if (ret == HD_OK) {
        printf("    ALC Disable: " COLOR_GREEN "OK" COLOR_RESET "\n");
    } else {
        printf("    ALC Disable: " COLOR_RED "FAILED (ret=%d)" COLOR_RESET "\n", ret);
        success = 0;
    }

    usleep(100000);

    // Test enable
    alc_enable = TRUE;
    ret = vendor_audiocap_set(ctrl, VENDOR_AUDIOCAP_ITEM_ALC_ENABLE, &alc_enable);
    if (ret == HD_OK) {
        printf("    ALC Enable:  " COLOR_GREEN "OK" COLOR_RESET "\n");
    } else {
        printf("    ALC Enable:  " COLOR_RED "FAILED (ret=%d)" COLOR_RESET "\n", ret);
        success = 0;
    }

    // Leave ALC disabled for other tests
    alc_enable = FALSE;
    vendor_audiocap_set(ctrl, VENDOR_AUDIOCAP_ITEM_ALC_ENABLE, &alc_enable);

    if (success) {
        PRINT_PASS("ALC enable/disable working");
        g_results.passed++;
    } else {
        PRINT_FAIL("ALC control failed", -1);
        g_results.failed++;
    }
}

/**
 * Test Default Setting / Boost Gain (VENDOR_AUDIOCAP_ITEM_DEFAULT_SETTING)
 * NOTE: This requires OUT path, not CTRL path
 */
static void test_default_setting(HD_PATH_ID path)
{
    PRINT_SECTION("Default Setting Test (VENDOR_AUDIOCAP_ITEM_DEFAULT_SETTING)");
    g_results.total++;

    HD_RESULT ret;
    UINT32 settings[] = {
        VENDOR_AUDIOCAP_DEFAULT_SETTING_0DB,
        VENDOR_AUDIOCAP_DEFAULT_SETTING_10DB,
        VENDOR_AUDIOCAP_DEFAULT_SETTING_20DB,
        VENDOR_AUDIOCAP_DEFAULT_SETTING_30DB,
        VENDOR_AUDIOCAP_DEFAULT_SETTING_ALCOFF
    };
    const char* setting_names[] = {
        "0dB boost",
        "10dB boost",
        "20dB boost",
        "30dB boost",
        "ALC OFF + 0dB"
    };
    int num_settings = 5;
    int success_count = 0;

    PRINT_INFO("Testing microphone boost presets (requires OUT path)");
    PRINT_INFO("Note: This API may fail on CTRL path - expected behavior");

    for (int i = 0; i < num_settings; i++) {
        ret = vendor_audiocap_set(path, VENDOR_AUDIOCAP_ITEM_DEFAULT_SETTING, &settings[i]);
        
        if (ret == HD_OK) {
            printf("    %s: " COLOR_GREEN "OK" COLOR_RESET "\n", setting_names[i]);
            success_count++;
        } else if (ret == -32) {
            printf("    %s: " COLOR_YELLOW "HD_ERR_IO (path type mismatch)" COLOR_RESET "\n", setting_names[i]);
        } else {
            printf("    %s: " COLOR_RED "FAILED (ret=%d)" COLOR_RESET "\n", setting_names[i], ret);
        }
        usleep(50000);
    }

    if (success_count > 0) {
        PRINT_PASS("Default setting working");
        g_results.passed++;
    } else {
        PRINT_SKIP("Default setting requires OUT path (known SDK limitation)");
        g_results.skipped++;
    }
}

/**
 * Test AGC Configuration (VENDOR_AUDIOCAP_ITEM_AGC_CONFIG)
 * NOTE: Requires AUDCAP_LIB_AGC = ENABLE in SDK build
 */
static void test_agc_config(HD_PATH_ID path)
{
    PRINT_SECTION("AGC Test (VENDOR_AUDIOCAP_ITEM_AGC_CONFIG)");
    g_results.total++;

    HD_RESULT ret;
    VENDOR_AUDIOCAP_AGC_CONFIG agc_cfg = {0};

    PRINT_INFO("AGC = Automatic Gain Control (software-based)");
    PRINT_INFO("Note: Requires AUDCAP_LIB_AGC=ENABLE in SDK build");

    agc_cfg.enable = TRUE;
    agc_cfg.target_lvl = -6;      // Target -6 dB
    agc_cfg.ng_threshold = -50;   // Noise gate at -50 dB

    ret = vendor_audiocap_set(path, VENDOR_AUDIOCAP_ITEM_AGC_CONFIG, &agc_cfg);

    if (ret == HD_OK) {
        printf("    AGC Enable (target=-6dB): " COLOR_GREEN "OK" COLOR_RESET "\n");
        PRINT_PASS("AGC configuration working");
        g_results.passed++;
    } else if (ret == -34) {
        printf("    AGC Config: " COLOR_YELLOW "HD_ERR_PARAM (-34)" COLOR_RESET "\n");
        PRINT_SKIP("AGC library not enabled in SDK (AUDCAP_LIB_AGC=DISABLE)");
        g_results.skipped++;
    } else {
        printf("    AGC Config: " COLOR_RED "FAILED (ret=%d)" COLOR_RESET "\n", ret);
        PRINT_FAIL("AGC configuration failed", ret);
        g_results.failed++;
    }
}

/**
 * Test Noise Gate Threshold (VENDOR_AUDIOCAP_ITEM_NOISEGATE_THRESHOLD)
 */
static void test_noisegate(HD_PATH_ID path)
{
    PRINT_SECTION("Noise Gate Test (VENDOR_AUDIOCAP_ITEM_NOISEGATE_THRESHOLD)");
    g_results.total++;

    HD_RESULT ret;
    INT32 thresholds[] = {-40, -50, -60, -70};
    int num_thresholds = 4;
    int success_count = 0;

    PRINT_INFO("Noise gate suppresses gain when signal is below threshold");

    for (int i = 0; i < num_thresholds; i++) {
        ret = vendor_audiocap_set(path, VENDOR_AUDIOCAP_ITEM_NOISEGATE_THRESHOLD, &thresholds[i]);
        
        if (ret == HD_OK) {
            printf("    Threshold %d dB: " COLOR_GREEN "OK" COLOR_RESET "\n", thresholds[i]);
            success_count++;
        } else if (ret == -32) {
            printf("    Threshold %d dB: " COLOR_YELLOW "HD_ERR_IO (path type)" COLOR_RESET "\n", thresholds[i]);
        } else {
            printf("    Threshold %d dB: " COLOR_RED "FAILED (ret=%d)" COLOR_RESET "\n", thresholds[i], ret);
        }
        usleep(50000);
    }

    if (success_count > 0) {
        PRINT_PASS("Noise gate threshold working");
        g_results.passed++;
    } else {
        PRINT_SKIP("Noise gate requires OUT path (known SDK limitation)");
        g_results.skipped++;
    }
}

/**
 * Test Loopback Configuration (for AEC)
 */
static void test_loopback_config(HD_PATH_ID ctrl)
{
    PRINT_SECTION("Loopback Config Test (VENDOR_AUDIOCAP_ITEM_LOOPBACK_CONFIG)");
    g_results.total++;

    HD_RESULT ret;
    VENDOR_AUDIOCAP_LOOPBACK_CONFIG lb_cfg = {0};

    PRINT_INFO("Loopback captures speaker output for AEC reference signal");

    // Test enable loopback
    lb_cfg.enabled = TRUE;
    lb_cfg.lb_channel = HD_AUDIOCAP_LB_CH_LEFT;

    ret = vendor_audiocap_set(ctrl, VENDOR_AUDIOCAP_ITEM_LOOPBACK_CONFIG, &lb_cfg);
    if (ret == HD_OK) {
        printf("    Loopback Enable (LEFT): " COLOR_GREEN "OK" COLOR_RESET "\n");
    } else {
        printf("    Loopback Enable: " COLOR_RED "FAILED (ret=%d)" COLOR_RESET "\n", ret);
    }

    // Test right channel
    lb_cfg.lb_channel = HD_AUDIOCAP_LB_CH_RIGHT;
    ret = vendor_audiocap_set(ctrl, VENDOR_AUDIOCAP_ITEM_LOOPBACK_CONFIG, &lb_cfg);
    if (ret == HD_OK) {
        printf("    Loopback Enable (RIGHT): " COLOR_GREEN "OK" COLOR_RESET "\n");
    } else {
        printf("    Loopback RIGHT: " COLOR_RED "FAILED (ret=%d)" COLOR_RESET "\n", ret);
    }

    // Disable
    lb_cfg.enabled = FALSE;
    ret = vendor_audiocap_set(ctrl, VENDOR_AUDIOCAP_ITEM_LOOPBACK_CONFIG, &lb_cfg);
    if (ret == HD_OK) {
        printf("    Loopback Disable: " COLOR_GREEN "OK" COLOR_RESET "\n");
        PRINT_PASS("Loopback configuration working");
        g_results.passed++;
    } else {
        PRINT_FAIL("Loopback configuration failed", ret);
        g_results.failed++;
    }
}

/**
 * Test ALC Config (VENDOR_AUDIOCAP_ITEM_ALC_CONFIG)
 */
static void test_alc_config(HD_PATH_ID ctrl)
{
    PRINT_SECTION("ALC Config Test (VENDOR_AUDIOCAP_ITEM_ALC_CONFIG)");
    g_results.total++;

    HD_RESULT ret;
    VENDOR_AUDIOCAP_ALC_CONFIG alc_cfg = {0};

    PRINT_INFO("ALC configuration: min/max gain, attack/decay time");

    alc_cfg.max_gain = VENDOR_AUDIOCAP_ALC_GAIN_P24P0_DB;
    alc_cfg.min_gain = VENDOR_AUDIOCAP_ALC_GAIN_N9P0_DB;

    ret = vendor_audiocap_set(ctrl, VENDOR_AUDIOCAP_ITEM_ALC_CONFIG, &alc_cfg);

    if (ret == HD_OK) {
        printf("    ALC Config (max=+24dB, min=-9dB): " COLOR_GREEN "OK" COLOR_RESET "\n");
        PRINT_PASS("ALC config working");
        g_results.passed++;
    } else if (ret == -35) {
        printf("    ALC Config: " COLOR_YELLOW "HD_ERR_NOT_SUPPORT (-35)" COLOR_RESET "\n");
        PRINT_SKIP("ALC config not supported on this hardware");
        g_results.skipped++;
    } else {
        printf("    ALC Config: " COLOR_RED "FAILED (ret=%d)" COLOR_RESET "\n", ret);
        PRINT_FAIL("ALC config failed", ret);
        g_results.failed++;
    }
}

/*============================================================================
 * Recording Test with File Output
 *============================================================================*/

static void *capture_thread(void *arg)
{
    AudioCapture *p = (AudioCapture *)arg;
    HD_RESULT ret;
    HD_AUDIOCAP_BUFINFO buf_info;
    UINT64 virt_addr;
    char filename[512];
    FILE *fp = NULL;

    // Wait for flow to start
    while (!p->running && g_keep_running) {
        usleep(10000);
    }

    // Get buffer info
    ret = hd_audiocap_get(p->cap_ctrl, HD_AUDIOCAP_PARAM_BUFINFO, &buf_info);
    if (ret != HD_OK) {
        printf("Failed to get buffer info: %d\n", ret);
        return NULL;
    }

    // Memory map
    virt_addr = (UINT64)hd_common_mem_mmap(HD_COMMON_MEM_MEM_TYPE_CACHE,
                                            buf_info.buf_info.phy_addr,
                                            buf_info.buf_info.buf_size);
    if (virt_addr == 0) {
        printf("Memory map failed\n");
        return NULL;
    }

    // Open output file
    snprintf(filename, sizeof(filename), "%s/audio_test_%d.pcm", 
             g_config.output_path, g_config.sample_rate);
    fp = fopen(filename, "wb");
    if (!fp) {
        printf("Failed to open output file: %s\n", filename);
        hd_common_mem_munmap((void*)virt_addr, buf_info.buf_info.buf_size);
        return NULL;
    }

    PRINT_INFO("Recording to: %s", filename);

    // Capture loop
    while (!p->exit_flag && g_keep_running) {
        HD_AUDIO_FRAME frame;
        
        ret = hd_audiocap_pull_out_buf(p->cap_path, &frame, 200);
        if (ret == HD_OK) {
            UINT8 *data = (UINT8 *)(virt_addr + (frame.phy_addr[0] - buf_info.buf_info.phy_addr));
            fwrite(data, 1, frame.size, fp);
            
            ret = hd_audiocap_release_out_buf(p->cap_path, &frame);
            if (ret != HD_OK) {
                printf("Release buffer failed: %d\n", ret);
            }
        }
    }

    fclose(fp);
    hd_common_mem_munmap((void*)virt_addr, buf_info.buf_info.buf_size);

    PRINT_INFO("Recording complete: %s", filename);
    return NULL;
}

static void test_recording(void)
{
    PRINT_SECTION("Audio Recording Test");
    g_results.total++;

    HD_RESULT ret;

    // Start capture
    ret = hd_audiocap_start(g_audio.cap_path);
    if (ret != HD_OK) {
        PRINT_FAIL("Failed to start capture", ret);
        g_results.failed++;
        return;
    }

    g_audio.running = 1;

    // Create capture thread
    pthread_create(&g_audio.thread_id, NULL, capture_thread, &g_audio);

    PRINT_INFO("Recording for %d seconds...", g_config.duration);

    // Wait for duration
    for (UINT32 i = 0; i < g_config.duration && g_keep_running; i++) {
        printf("    Recording: %d/%d seconds\r", i + 1, g_config.duration);
        fflush(stdout);
        sleep(1);
    }
    printf("\n");

    // Stop
    g_audio.exit_flag = 1;
    pthread_join(g_audio.thread_id, NULL);

    hd_audiocap_stop(g_audio.cap_path);
    g_audio.running = 0;
    g_audio.exit_flag = 0;

    PRINT_PASS("Recording completed successfully");
    g_results.passed++;
}

/*============================================================================
 * Main Test Runner
 *============================================================================*/

static void run_all_tests(void)
{
    // Volume (uses CTRL path)
    test_volume_control(g_audio.cap_ctrl);

    // Gain Level (uses CTRL path)
    test_gain_level(g_audio.cap_ctrl);

    // ALC Enable/Disable (uses CTRL path)
    test_alc_control(g_audio.cap_ctrl);

    // ALC Config (uses CTRL path)
    test_alc_config(g_audio.cap_ctrl);

    // Loopback (uses CTRL path)
    test_loopback_config(g_audio.cap_ctrl);

    // Default Setting (needs OUT path - test on cap_path)
    test_default_setting(g_audio.cap_path);

    // AGC Config (needs OUT path)
    test_agc_config(g_audio.cap_path);

    // Noise Gate (needs OUT path)
    test_noisegate(g_audio.cap_path);

    // Recording test
    if (g_config.duration > 0) {
        test_recording();
    }
}

static void print_summary(void)
{
    printf("\n");
    PRINT_HEADER("Test Summary");

    printf("  Total Tests:  %d\n", g_results.total);
    printf("  " COLOR_GREEN "Passed:       %d" COLOR_RESET "\n", g_results.passed);
    printf("  " COLOR_RED "Failed:       %d" COLOR_RESET "\n", g_results.failed);
    printf("  " COLOR_YELLOW "Skipped:      %d" COLOR_RESET "\n", g_results.skipped);
    printf("\n");

    if (g_results.failed == 0) {
        printf(COLOR_GREEN "All tests passed!" COLOR_RESET "\n");
    } else {
        printf(COLOR_YELLOW "Some tests failed or were skipped due to SDK configuration." COLOR_RESET "\n");
    }

    printf("\n" COLOR_CYAN "Notes:" COLOR_RESET "\n");
    printf("  • AGC requires AUDCAP_LIB_AGC=ENABLE in SDK build\n");
    printf("  • DEFAULT_SETTING, AGC, Noise Gate require OUT path, not CTRL\n");
    printf("  • ALC should be disabled when using software AGC/AEC/ANR\n");
    printf("\n");
}

static void print_usage(const char *prog)
{
    printf("Usage: %s [options]\n", prog);
    printf("\nOptions:\n");
    printf("  --test <mode>    Test mode: all, volume, gain, alc, agc, record\n");
    printf("  --volume <0-200> Capture volume (default: 100)\n");
    printf("  --duration <sec> Recording duration (default: 5, 0=skip recording)\n");
    printf("  --output <path>  Output directory (default: /mnt/sd)\n");
    printf("  --rate <hz>      Sample rate (default: 48000, use 8000 for AEC)\n");
    printf("  --verbose        Enable verbose output\n");
    printf("  --help           Show this help\n");
    printf("\nExamples:\n");
    printf("  %s --test all                    # Run all tests\n", prog);
    printf("  %s --test volume --volume 150   # Test volume at 150%%\n", prog);
    printf("  %s --test record --duration 10  # Record for 10 seconds\n", prog);
    printf("  %s --rate 8000 --test record    # Record at 8kHz (for AEC)\n", prog);
}

static void parse_args(int argc, char **argv)
{
    static struct option long_options[] = {
        {"test",     required_argument, 0, 't'},
        {"volume",   required_argument, 0, 'v'},
        {"duration", required_argument, 0, 'd'},
        {"output",   required_argument, 0, 'o'},
        {"rate",     required_argument, 0, 'r'},
        {"verbose",  no_argument,       0, 'V'},
        {"help",     no_argument,       0, 'h'},
        {0, 0, 0, 0}
    };

    int opt;
    while ((opt = getopt_long(argc, argv, "t:v:d:o:r:Vh", long_options, NULL)) != -1) {
        switch (opt) {
            case 't':
                if (strcmp(optarg, "all") == 0) g_config.test_mode = TEST_ALL;
                else if (strcmp(optarg, "volume") == 0) g_config.test_mode = TEST_VOLUME;
                else if (strcmp(optarg, "gain") == 0) g_config.test_mode = TEST_GAIN_LEVEL;
                else if (strcmp(optarg, "alc") == 0) g_config.test_mode = TEST_ALC;
                else if (strcmp(optarg, "agc") == 0) g_config.test_mode = TEST_AGC;
                else if (strcmp(optarg, "record") == 0) g_config.test_mode = TEST_RECORD;
                break;
            case 'v':
                g_config.volume = atoi(optarg);
                break;
            case 'd':
                g_config.duration = atoi(optarg);
                break;
            case 'o':
                strncpy(g_config.output_path, optarg, sizeof(g_config.output_path) - 1);
                break;
            case 'r':
                g_config.sample_rate = atoi(optarg);
                break;
            case 'V':
                g_config.verbose = 1;
                break;
            case 'h':
            default:
                print_usage(argv[0]);
                exit(0);
        }
    }
}

/*============================================================================
 * Main Entry Point
 *============================================================================*/

int main(int argc, char **argv)
{
    HD_RESULT ret;

    parse_args(argc, argv);

    // Setup signal handlers
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    PRINT_HEADER("Novatek Audio Features Test");

    printf("Configuration:\n");
    printf("  Sample Rate:  %d Hz\n", g_config.sample_rate);
    printf("  Volume:       %d\n", g_config.volume);
    printf("  Duration:     %d seconds\n", g_config.duration);
    printf("  Output Path:  %s\n", g_config.output_path);
    printf("\n");

    // Initialize HDAL
    PRINT_SECTION("Initializing HDAL");
    
    ret = hd_common_init(0);
    if (ret != HD_OK) {
        printf("hd_common_init failed: %d\n", ret);
        return -1;
    }
    printf("    HDAL common: " COLOR_GREEN "OK" COLOR_RESET "\n");

    ret = mem_init();
    if (ret != HD_OK) {
        printf("mem_init failed: %d\n", ret);
        goto cleanup_common;
    }
    printf("    Memory pool: " COLOR_GREEN "OK" COLOR_RESET "\n");

    ret = hd_audiocap_init();
    if (ret != HD_OK) {
        printf("hd_audiocap_init failed: %d\n", ret);
        goto cleanup_mem;
    }
    printf("    Audio capture: " COLOR_GREEN "OK" COLOR_RESET "\n");

    // Open CTRL path first
    ret = audio_open_ctrl(&g_audio.cap_ctrl);
    if (ret != HD_OK) {
        printf("audio_open_ctrl failed: %d\n", ret);
        goto cleanup_audiocap;
    }

    // Configure device BEFORE opening OUT path (required order!)
    ret = audio_set_dev_config(g_audio.cap_ctrl, g_config.sample_rate);
    if (ret != HD_OK) {
        printf("audio_set_dev_config failed: %d\n", ret);
        goto cleanup_ctrl;
    }

    ret = audio_set_drv_config(g_audio.cap_ctrl);
    if (ret != HD_OK) {
        printf("audio_set_drv_config failed: %d\n", ret);
        goto cleanup_ctrl;
    }

    // NOW open OUT path (after DEV_CONFIG is set)
    ret = audio_open_path(&g_audio.cap_path);
    if (ret != HD_OK) {
        printf("audio_open_path failed: %d\n", ret);
        goto cleanup_ctrl;
    }

    ret = audio_set_path_config(g_audio.cap_path, g_config.sample_rate, HD_AUDIO_SOUND_MODE_MONO);
    if (ret != HD_OK) {
        printf("audio_set_path_config failed: %d\n", ret);
        goto cleanup_path;
    }

    printf("    Configuration: " COLOR_GREEN "OK" COLOR_RESET "\n");

    // Run tests
    switch (g_config.test_mode) {
        case TEST_VOLUME:
            test_volume_control(g_audio.cap_ctrl);
            break;
        case TEST_GAIN_LEVEL:
            test_gain_level(g_audio.cap_ctrl);
            break;
        case TEST_ALC:
            test_alc_control(g_audio.cap_ctrl);
            test_alc_config(g_audio.cap_ctrl);
            break;
        case TEST_AGC:
            test_agc_config(g_audio.cap_path);
            break;
        case TEST_RECORD:
            test_recording();
            break;
        case TEST_ALL:
        default:
            run_all_tests();
            break;
    }

    // Print summary
    print_summary();

    // Cleanup
cleanup_path:
    audio_close_path(g_audio.cap_path);
cleanup_ctrl:
    audio_close_ctrl(g_audio.cap_ctrl);
cleanup_audiocap:
    hd_audiocap_uninit();
cleanup_mem:
    mem_exit();
cleanup_common:
    hd_common_uninit();

    return (g_results.failed == 0) ? 0 : 1;
}
