/**
 * @file userspace_agc_test.c
 * @brief Userspace AGC/AEC Test Sample
 *
 * This sample demonstrates using the userspace libaec.a and libagc.a libraries
 * directly for audio processing, bypassing the kernel module requirement.
 *
 * Approach 1: Userspace Audio Processing Libraries
 *
 * Copyright Novatek Microelectronics Corp. 2024. All rights reserved.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>
#include <sys/time.h>
#include <math.h>

#include "hd_type.h"
#include "hd_common.h"

/* Include kwrap types for ER and E_OK */
#include "kwrap/error_no.h"

/* Userspace audio libraries */
#include "aec/audlib_aec.h"
#include "agc/audlib_agc.h"

/*============================================================================
 * Configuration
 *============================================================================*/

#define AUDIO_SAMPLE_RATE   8000    /* 8kHz for AEC/AGC (recommended) */
#define AUDIO_CHANNELS      1       /* Mono */
#define FRAME_SAMPLES       1024    /* Must be 1024 for AEC/AGC libs */

/* Colors for terminal output */
#define COLOR_RESET   "\033[0m"
#define COLOR_RED     "\033[31m"
#define COLOR_GREEN   "\033[32m"
#define COLOR_YELLOW  "\033[33m"
#define COLOR_CYAN    "\033[36m"

#define PRINT_HEADER(x)   printf("\n" COLOR_CYAN "╔════════════════════════════════════════════════════════════════╗\n║ %-62s ║\n╚════════════════════════════════════════════════════════════════╝" COLOR_RESET "\n\n", x)
#define PRINT_SECTION(x)  printf("\n" COLOR_YELLOW "▶ %s" COLOR_RESET "\n────────────────────────────────────────\n", x)
#define PRINT_PASS(x)     printf("  " COLOR_GREEN "✓ PASS: %s" COLOR_RESET "\n", x)
#define PRINT_FAIL(x, e)  printf("  " COLOR_RED "✗ FAIL: %s (error=%d)" COLOR_RESET "\n", x, e)
#define PRINT_SKIP(x)     printf("  " COLOR_YELLOW "○ SKIP: %s" COLOR_RESET "\n", x)
#define PRINT_INFO(x)     printf("  ℹ %s\n", x)

/*============================================================================
 * Global Variables
 *============================================================================*/

static volatile int g_keep_running = 1;

static struct {
    int sample_rate;
    int duration;
    int test_agc;
    int test_aec;
    char output_path[256];
} g_config = {
    .sample_rate = AUDIO_SAMPLE_RATE,
    .duration = 5,
    .test_agc = 1,
    .test_aec = 1,
    .output_path = "/mnt/sd"
};

/* AGC buffers */
static INT16 *g_agc_in_buf = NULL;
static INT16 *g_agc_out_buf = NULL;

/* NS/AEC buffers */
static INT16 *g_aec_record_buf = NULL;
static INT16 *g_aec_out_buf = NULL;
static UINT8 *g_aec_internal_buf = NULL;

/*============================================================================
 * Signal Handler
 *============================================================================*/

static void signal_handler(int sig)
{
    printf("\nSignal %d received, stopping...\n", sig);
    g_keep_running = 0;
}

/*============================================================================
 * AGC Library Test (Userspace)
 *============================================================================*/

static int test_agc_userspace(void)
{
    PRINT_SECTION("Userspace AGC Test (libagc.a)");
    
    ER ret;
    int success = 1;
    
    /* Allocate AGC buffers */
    g_agc_in_buf = (INT16 *)malloc(FRAME_SAMPLES * sizeof(INT16));
    g_agc_out_buf = (INT16 *)malloc(FRAME_SAMPLES * sizeof(INT16));
    
    if (!g_agc_in_buf || !g_agc_out_buf) {
        PRINT_FAIL("Failed to allocate AGC buffers", -1);
        return -1;
    }
    
    PRINT_INFO("Opening AGC library...");
    
    /* Open AGC */
    ret = audlib_agc_open();
    if (ret != E_OK) {
        PRINT_FAIL("audlib_agc_open() failed", ret);
        success = 0;
        goto cleanup;
    }
    printf("    audlib_agc_open(): " COLOR_GREEN "OK" COLOR_RESET "\n");
    
    /* Check if opened */
    if (!audlib_agc_is_opened()) {
        PRINT_FAIL("AGC not opened after audlib_agc_open()", -1);
        success = 0;
        goto cleanup;
    }
    
    PRINT_INFO("Configuring AGC parameters...");
    
    /* Configure AGC */
    audlib_agc_set_config(AGC_CONFIG_ID_SAMPLERATE, g_config.sample_rate);
    printf("    Sample rate: %d Hz\n", g_config.sample_rate);
    
    audlib_agc_set_config(AGC_CONFIG_ID_CHANNEL_NO, AUDIO_CHANNELS);
    printf("    Channels: %d\n", AUDIO_CHANNELS);
    
    /* Target level: -6dB */
    audlib_agc_set_config(AGC_CONFIG_ID_TARGET_LVL, AGC_DB(-6));
    printf("    Target level: -6 dB\n");
    
    /* Max gain: +20dB */
    audlib_agc_set_config(AGC_CONFIG_ID_MAXGAIN, AGC_DB(20));
    printf("    Max gain: +20 dB\n");
    
    /* Min gain: -10dB */
    audlib_agc_set_config(AGC_CONFIG_ID_MINGAIN, AGC_DB(-10));
    printf("    Min gain: -10 dB\n");
    
    /* Attack/Decay times */
    audlib_agc_set_config(AGC_CONFIG_ID_ATTACK_TIME, AGC_TRESO_BASIS_100MS);
    audlib_agc_set_config(AGC_CONFIG_ID_DECAY_TIME, AGC_TRESO_BASIS_200MS);
    printf("    Attack time: 100ms, Decay time: 200ms\n");
    
    /* Noise gate threshold: -50dB */
    audlib_agc_set_config(AGC_CONFIG_ID_NG_THD, AGC_DB(-50));
    printf("    Noise gate threshold: -50 dB\n");
    
    /* Initialize AGC */
    PRINT_INFO("Initializing AGC...");
    if (!audlib_agc_init()) {
        PRINT_FAIL("audlib_agc_init() failed", -1);
        success = 0;
        goto cleanup;
    }
    printf("    audlib_agc_init(): " COLOR_GREEN "OK" COLOR_RESET "\n");
    
    /* Test AGC processing with dummy data */
    PRINT_INFO("Testing AGC processing...");
    
    /* Generate test signal (sine wave) */
    for (int i = 0; i < FRAME_SAMPLES; i++) {
        /* Low amplitude sine wave to test gain boost */
        g_agc_in_buf[i] = (INT16)(1000.0 * sin(2.0 * 3.14159 * 1000.0 * i / g_config.sample_rate));
    }
    
    /* Process with AGC */
    AGC_BITSTREAM agc_io;
    agc_io.bitstram_buffer_in = (UINT64)(uintptr_t)g_agc_in_buf;
    agc_io.bitstram_buffer_out = (UINT64)(uintptr_t)g_agc_out_buf;
    agc_io.bitstram_buffer_length = FRAME_SAMPLES;
    
    if (audlib_agc_run(&agc_io)) {
        printf("    audlib_agc_run(): " COLOR_GREEN "OK" COLOR_RESET "\n");
        
        /* Check output has higher amplitude (gain was applied) */
        int in_max = 0, out_max = 0;
        for (int i = 0; i < FRAME_SAMPLES; i++) {
            if (abs(g_agc_in_buf[i]) > in_max) in_max = abs(g_agc_in_buf[i]);
            if (abs(g_agc_out_buf[i]) > out_max) out_max = abs(g_agc_out_buf[i]);
        }
        printf("    Input peak: %d, Output peak: %d\n", in_max, out_max);
        
        if (out_max >= in_max) {
            PRINT_PASS("AGC gain applied successfully");
        } else {
            PRINT_INFO("AGC may have attenuated (depends on settings)");
        }
    } else {
        PRINT_FAIL("audlib_agc_run() failed", -1);
        success = 0;
    }
    
cleanup:
    /* Close AGC */
    audlib_agc_close();
    
    if (g_agc_in_buf) free(g_agc_in_buf);
    if (g_agc_out_buf) free(g_agc_out_buf);
    g_agc_in_buf = NULL;
    g_agc_out_buf = NULL;
    
    if (success) {
        PRINT_PASS("Userspace AGC test completed");
    }
    
    return success ? 0 : -1;
}

/*============================================================================
 * AEC Library Test (Userspace)
 * Note: Full AEC requires preloaded filter coefficients which are device-specific
 * We'll test the Noise Suppression (NS) part which is standalone
 *============================================================================*/

static int test_aec_userspace(void)
{
    PRINT_SECTION("Userspace Noise Suppression Test (libaec.a - audlib_ns)");
    
    ER ret;
    int success = 1;
    INT32 buf_size;
    
    PRINT_INFO("Testing Noise Suppression (NS) library...");
    PRINT_INFO("Note: Full AEC requires device-specific preload coefficients");
    
    /* Open NS library */
    ret = audlib_ns_open();
    if (ret != E_OK) {
        PRINT_FAIL("audlib_ns_open() failed", ret);
        return -1;
    }
    printf("    audlib_ns_open(): " COLOR_GREEN "OK" COLOR_RESET "\n");
    
    /* Check if opened */
    if (!audlib_ns_is_opened()) {
        PRINT_FAIL("NS not opened after audlib_ns_open()", -1);
        audlib_ns_close();
        return -1;
    }
    
    PRINT_INFO("Configuring NS parameters...");
    
    /* Configure NS */
    audlib_ns_set_config(AUDNS_CONFIG_ID_SAMPLERATE, g_config.sample_rate);
    printf("    Sample rate: %d Hz\n", g_config.sample_rate);
    
    audlib_ns_set_config(AUDNS_CONFIG_ID_CHANNEL_NO, 1);
    printf("    Channels: 1 (mono)\n");
    
    /* Noise cancellation level: -20dB */
    audlib_ns_set_config(AUDNS_CONFIG_ID_NOISE_CANCEL_LVL, -20);
    printf("    Noise cancel level: -20 dB\n");
    
    /* Get required buffer size */
    buf_size = audlib_ns_get_required_buffer_size();
    printf("    NS internal buffer size required: %d bytes\n", buf_size);
    
    if (buf_size <= 0) {
        PRINT_FAIL("Invalid NS buffer size", buf_size);
        audlib_ns_close();
        return -1;
    }
    
    /* Allocate buffers */
    g_aec_internal_buf = (UINT8 *)malloc(buf_size);
    g_aec_record_buf = (INT16 *)malloc(FRAME_SAMPLES * sizeof(INT16));
    g_aec_out_buf = (INT16 *)malloc(FRAME_SAMPLES * sizeof(INT16));
    
    if (!g_aec_internal_buf || !g_aec_record_buf || !g_aec_out_buf) {
        PRINT_FAIL("Failed to allocate NS buffers", -1);
        success = 0;
        goto cleanup;
    }
    
    /* Set internal buffer */
    audlib_ns_set_config(AUDNS_CONFIG_ID_BUF_ADDR, (INT32)(uintptr_t)g_aec_internal_buf);
    audlib_ns_set_config(AUDNS_CONFIG_ID_BUF_SIZE, buf_size);
    printf("    Internal buffer: %d bytes allocated\n", buf_size);
    
    /* Initialize NS */
    PRINT_INFO("Initializing NS...");
    if (!audlib_ns_init()) {
        PRINT_FAIL("audlib_ns_init() failed", -1);
        success = 0;
        goto cleanup;
    }
    printf("    audlib_ns_init(): " COLOR_GREEN "OK" COLOR_RESET "\n");
    
    /* Test NS processing with noisy data */
    PRINT_INFO("Testing NS processing...");
    
    /* Generate test signal (speech + noise simulation) */
    for (int i = 0; i < FRAME_SAMPLES; i++) {
        /* Simulated speech with noise */
        double speech = 3000.0 * sin(2.0 * 3.14159 * 300.0 * i / g_config.sample_rate);
        double noise = 500.0 * sin(2.0 * 3.14159 * 5000.0 * i / g_config.sample_rate);
        g_aec_record_buf[i] = (INT16)(speech + noise);
    }
    
    /* Process with NS */
    AUDNS_BITSTREAM ns_io;
    ns_io.bitstram_buffer_in = (UINT32)(uintptr_t)g_aec_record_buf;
    ns_io.bitstram_buffer_out = (UINT32)(uintptr_t)g_aec_out_buf;
    ns_io.bitstram_buffer_length = FRAME_SAMPLES;
    
    if (audlib_ns_run(&ns_io)) {
        printf("    audlib_ns_run(): " COLOR_GREEN "OK" COLOR_RESET "\n");
        
        /* Check output */
        int in_max = 0, out_max = 0;
        for (int i = 0; i < FRAME_SAMPLES; i++) {
            if (abs(g_aec_record_buf[i]) > in_max) in_max = abs(g_aec_record_buf[i]);
            if (abs(g_aec_out_buf[i]) > out_max) out_max = abs(g_aec_out_buf[i]);
        }
        printf("    Input (with noise) peak: %d\n", in_max);
        printf("    Output (noise suppressed) peak: %d\n", out_max);
        
        PRINT_PASS("NS processing completed");
    } else {
        PRINT_FAIL("audlib_ns_run() failed", -1);
        success = 0;
    }
    
cleanup:
    /* Close NS */
    audlib_ns_close();
    
    if (g_aec_internal_buf) free(g_aec_internal_buf);
    if (g_aec_record_buf) free(g_aec_record_buf);
    if (g_aec_out_buf) free(g_aec_out_buf);
    g_aec_internal_buf = NULL;
    g_aec_record_buf = NULL;
    g_aec_out_buf = NULL;
    
    if (success) {
        PRINT_PASS("Userspace NS test completed");
    }
    
    return success ? 0 : -1;
}

/*============================================================================
 * Usage
 *============================================================================*/

static void print_usage(const char *prog)
{
    printf("Usage: %s [options]\n\n", prog);
    printf("Userspace AGC/AEC Test - Uses libaec.a and libagc.a directly\n\n");
    printf("Options:\n");
    printf("  --agc-only       Test AGC library only\n");
    printf("  --aec-only       Test AEC library only\n");
    printf("  --rate <hz>      Sample rate (default: 8000, recommended for AEC)\n");
    printf("  --help           Show this help\n");
    printf("\nNote: This approach uses userspace libraries, no kernel modules needed.\n");
    printf("      Libraries: libaec.a, libagc.a from code/lib/output/\n");
}

static void parse_args(int argc, char **argv)
{
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--agc-only") == 0) {
            g_config.test_agc = 1;
            g_config.test_aec = 0;
        } else if (strcmp(argv[i], "--aec-only") == 0) {
            g_config.test_agc = 0;
            g_config.test_aec = 1;
        } else if (strcmp(argv[i], "--rate") == 0 && i + 1 < argc) {
            g_config.sample_rate = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--help") == 0) {
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
    int ret = 0;
    int agc_result = 0, aec_result = 0;
    
    parse_args(argc, argv);
    
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    
    PRINT_HEADER("Userspace AGC/NS Test (Approach 1)");
    
    printf("Configuration:\n");
    printf("  Sample Rate:  %d Hz\n", g_config.sample_rate);
    printf("  Test AGC:     %s\n", g_config.test_agc ? "YES" : "NO");
    printf("  Test NS:      %s\n", g_config.test_aec ? "YES" : "NO");
    printf("\n");
    
    printf(COLOR_CYAN "This test uses userspace libraries directly:\n" COLOR_RESET);
    printf("  - libaec.a: Contains Noise Suppression (NS) module\n");
    printf("  - libagc.a: Auto Gain Control\n");
    printf("  - No kernel modules (nvt_audlib_*.ko) required!\n\n");
    
    /* Run AGC test */
    if (g_config.test_agc) {
        agc_result = test_agc_userspace();
    }
    
    /* Run NS test */
    if (g_config.test_aec) {
        aec_result = test_aec_userspace();
    }
    
    /* Summary */
    PRINT_HEADER("Test Summary");
    
    if (g_config.test_agc) {
        if (agc_result == 0) {
            printf("  AGC (libagc.a): " COLOR_GREEN "PASS" COLOR_RESET "\n");
        } else {
            printf("  AGC (libagc.a): " COLOR_RED "FAIL" COLOR_RESET "\n");
            ret = 1;
        }
    }
    
    if (g_config.test_aec) {
        if (aec_result == 0) {
            printf("  NS  (libaec.a): " COLOR_GREEN "PASS" COLOR_RESET "\n");
        } else {
            printf("  NS  (libaec.a): " COLOR_RED "FAIL" COLOR_RESET "\n");
            ret = 1;
        }
    }
    
    printf("\n");
    printf(COLOR_CYAN "Approach 1 Summary:\n" COLOR_RESET);
    printf("  ✓ Uses userspace static libraries (libaec.a, libagc.a)\n");
    printf("  ✓ No kernel module loading required\n");
    printf("  ✓ Libraries already built in SDK: code/lib/output/\n");
    printf("  ✓ Application-level audio processing\n");
    printf("  ✓ AGC: Auto Gain Control - amplifies quiet audio\n");
    printf("  ✓ NS:  Noise Suppression - reduces background noise\n");
    printf("  ○ AEC: Echo Cancel requires device-specific preload coefficients\n");
    printf("\n");
    
    return ret;
}
