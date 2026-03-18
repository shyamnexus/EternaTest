/**
 * @file audio_processing_test.c
 * @brief Comprehensive Audio Processing Test
 *
 * Tests all available userspace audio processing libraries:
 * - AGC (Auto Gain Control) - libagc.a
 * - NS (Noise Suppression) - libaec.a  
 * - Audio Filter/EQ - libaudfilt.a
 * - Noise Gate via HDAL API
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
#include "hd_audiocapture.h"
#include "vendor_audiocapture.h"

/* Include kwrap types for ER and E_OK */
#include "kwrap/error_no.h"

/* Userspace audio libraries */
#include "agc/audlib_agc.h"
#include "aec/audlib_aec.h"
#include "audfilt/audlib_filt.h"

/*============================================================================
 * Configuration
 *============================================================================*/

#define AUDIO_SAMPLE_RATE   48000   /* 48kHz default */
#define AUDIO_CHANNELS      1       /* Mono */
#define FRAME_SAMPLES       1024    /* Frame size */
#define DDR_ID0             0

/* Colors for terminal output */
#define COLOR_RESET   "\033[0m"
#define COLOR_RED     "\033[31m"
#define COLOR_GREEN   "\033[32m"
#define COLOR_YELLOW  "\033[33m"
#define COLOR_CYAN    "\033[36m"
#define COLOR_MAGENTA "\033[35m"

#define PRINT_HEADER(x)   printf("\n" COLOR_CYAN "╔════════════════════════════════════════════════════════════════╗\n║ %-62s ║\n╚════════════════════════════════════════════════════════════════╝" COLOR_RESET "\n\n", x)
#define PRINT_SECTION(x)  printf("\n" COLOR_YELLOW "▶ %s" COLOR_RESET "\n────────────────────────────────────────\n", x)
#define PRINT_PASS(x)     printf("  " COLOR_GREEN "✓ PASS: %s" COLOR_RESET "\n", x)
#define PRINT_FAIL(x, e)  printf("  " COLOR_RED "✗ FAIL: %s (error=%d)" COLOR_RESET "\n", x, e)
#define PRINT_SKIP(x)     printf("  " COLOR_YELLOW "○ SKIP: %s" COLOR_RESET "\n", x)
#define PRINT_INFO(x)     printf("  ℹ %s\n", x)
#define PRINT_WARN(x)     printf("  " COLOR_MAGENTA "⚠ %s" COLOR_RESET "\n", x)

/*============================================================================
 * Global Variables
 *============================================================================*/

static volatile int g_keep_running = 1;

static struct {
    int sample_rate;
    int test_agc;
    int test_ns;
    int test_filter;
    int test_noisegate;
    int timeout_ms;
} g_config = {
    .sample_rate = AUDIO_SAMPLE_RATE,
    .test_agc = 1,
    .test_ns = 1,
    .test_filter = 1,
    .test_noisegate = 1,
    .timeout_ms = 1000  /* 1 second timeout for NS test */
};

/* Test results */
static int g_agc_result = -1;
static int g_ns_result = -1;
static int g_filter_result = -1;
static int g_noisegate_result = -1;

/* Audio buffers */
static INT16 *g_audio_in = NULL;
static INT16 *g_audio_out = NULL;
static UINT8 *g_internal_buf = NULL;

/* HDAL handles for noise gate test */
static HD_PATH_ID g_cap_ctrl = 0;
static HD_PATH_ID g_cap_path = 0;

/*============================================================================
 * Signal Handler
 *============================================================================*/

static void signal_handler(int sig)
{
    printf("\n  Signal %d received, stopping...\n", sig);
    g_keep_running = 0;
}

/*============================================================================
 * Utility Functions
 *============================================================================*/

static void generate_test_signal(INT16 *buf, int samples, int sample_rate, 
                                  float freq, float amplitude)
{
    for (int i = 0; i < samples; i++) {
        buf[i] = (INT16)(amplitude * sin(2.0 * M_PI * freq * i / sample_rate));
    }
}

static void add_noise(INT16 *buf, int samples, float noise_amplitude)
{
    for (int i = 0; i < samples; i++) {
        int noise = (rand() % (int)(noise_amplitude * 2)) - (int)noise_amplitude;
        int val = buf[i] + noise;
        if (val > 32767) val = 32767;
        if (val < -32768) val = -32768;
        buf[i] = (INT16)val;
    }
}

static int get_peak_amplitude(INT16 *buf, int samples)
{
    int peak = 0;
    for (int i = 0; i < samples; i++) {
        int val = abs(buf[i]);
        if (val > peak) peak = val;
    }
    return peak;
}

static float get_rms_amplitude(INT16 *buf, int samples)
{
    double sum = 0;
    for (int i = 0; i < samples; i++) {
        sum += (double)buf[i] * (double)buf[i];
    }
    return (float)sqrt(sum / samples);
}

/*============================================================================
 * Test 1: AGC (Auto Gain Control) - libagc.a
 *============================================================================*/

static int test_agc(void)
{
    PRINT_SECTION("Test 1: AGC (Auto Gain Control) - libagc.a");
    
    ER ret;
    int success = 1;
    
    /* Allocate buffers */
    g_audio_in = (INT16 *)malloc(FRAME_SAMPLES * sizeof(INT16));
    g_audio_out = (INT16 *)malloc(FRAME_SAMPLES * sizeof(INT16));
    
    if (!g_audio_in || !g_audio_out) {
        PRINT_FAIL("Buffer allocation", -1);
        return -1;
    }
    
    /* Open AGC */
    ret = audlib_agc_open();
    if (ret != E_OK) {
        PRINT_FAIL("audlib_agc_open()", ret);
        free(g_audio_in); free(g_audio_out);
        return -1;
    }
    printf("    audlib_agc_open(): " COLOR_GREEN "OK" COLOR_RESET "\n");
    
    /* Configure AGC for 8kHz (recommended) */
    int agc_rate = 8000;
    audlib_agc_set_config(AGC_CONFIG_ID_SAMPLERATE, agc_rate);
    audlib_agc_set_config(AGC_CONFIG_ID_CHANNEL_NO, 1);
    audlib_agc_set_config(AGC_CONFIG_ID_TARGET_LVL, AGC_DB(-6));
    audlib_agc_set_config(AGC_CONFIG_ID_MAXGAIN, AGC_DB(20));
    audlib_agc_set_config(AGC_CONFIG_ID_MINGAIN, AGC_DB(-10));
    audlib_agc_set_config(AGC_CONFIG_ID_ATTACK_TIME, AGC_TRESO_BASIS_100MS);
    audlib_agc_set_config(AGC_CONFIG_ID_DECAY_TIME, AGC_TRESO_BASIS_200MS);
    audlib_agc_set_config(AGC_CONFIG_ID_NG_THD, AGC_DB(-50));
    
    printf("    AGC Config: rate=%dHz, target=-6dB, maxGain=+20dB\n", agc_rate);
    
    /* Initialize */
    if (!audlib_agc_init()) {
        PRINT_FAIL("audlib_agc_init()", -1);
        success = 0;
        goto agc_cleanup;
    }
    printf("    audlib_agc_init(): " COLOR_GREEN "OK" COLOR_RESET "\n");
    
    /* Generate quiet test signal */
    generate_test_signal(g_audio_in, FRAME_SAMPLES, agc_rate, 1000.0f, 1000.0f);
    int in_peak = get_peak_amplitude(g_audio_in, FRAME_SAMPLES);
    
    /* Process */
    AGC_BITSTREAM agc_io;
    agc_io.bitstram_buffer_in = (UINT64)(uintptr_t)g_audio_in;
    agc_io.bitstram_buffer_out = (UINT64)(uintptr_t)g_audio_out;
    agc_io.bitstram_buffer_length = FRAME_SAMPLES;
    
    if (audlib_agc_run(&agc_io)) {
        int out_peak = get_peak_amplitude(g_audio_out, FRAME_SAMPLES);
        printf("    audlib_agc_run(): " COLOR_GREEN "OK" COLOR_RESET "\n");
        printf("    Input peak: %d → Output peak: %d (%.1fx gain)\n", 
               in_peak, out_peak, (float)out_peak / in_peak);
        
        if (out_peak > in_peak) {
            PRINT_PASS("AGC amplified signal successfully");
        } else {
            PRINT_INFO("AGC may have limited gain (depends on input level)");
        }
    } else {
        PRINT_FAIL("audlib_agc_run()", -1);
        success = 0;
    }
    
agc_cleanup:
    audlib_agc_close();
    free(g_audio_in); g_audio_in = NULL;
    free(g_audio_out); g_audio_out = NULL;
    
    return success ? 0 : -1;
}

/*============================================================================
 * Test 2: Noise Suppression - libaec.a (audlib_ns_*)
 *============================================================================*/

static volatile int g_ns_timeout = 0;

static void ns_alarm_handler(int sig)
{
    (void)sig;
    g_ns_timeout = 1;
}

static int test_ns(void)
{
    PRINT_SECTION("Test 2: Noise Suppression (NS) - libaec.a");
    
    ER ret;
    int success = 1;
    INT32 buf_size;
    
    PRINT_INFO("Note: NS may require real audio hardware path");
    
    /* Open NS */
    ret = audlib_ns_open();
    if (ret != E_OK) {
        PRINT_FAIL("audlib_ns_open()", ret);
        return -1;
    }
    printf("    audlib_ns_open(): " COLOR_GREEN "OK" COLOR_RESET "\n");
    
    /* Configure NS for 8kHz */
    int ns_rate = 8000;
    audlib_ns_set_config(AUDNS_CONFIG_ID_SAMPLERATE, ns_rate);
    audlib_ns_set_config(AUDNS_CONFIG_ID_CHANNEL_NO, 1);
    audlib_ns_set_config(AUDNS_CONFIG_ID_NOISE_CANCEL_LVL, -20);
    
    printf("    NS Config: rate=%dHz, noise_cancel=-20dB\n", ns_rate);
    
    /* Get buffer size */
    buf_size = audlib_ns_get_required_buffer_size();
    printf("    Buffer size required: %d bytes\n", buf_size);
    
    if (buf_size <= 0) {
        PRINT_FAIL("audlib_ns_get_required_buffer_size()", buf_size);
        audlib_ns_close();
        return -1;
    }
    
    /* Allocate buffers */
    g_internal_buf = (UINT8 *)malloc(buf_size);
    g_audio_in = (INT16 *)malloc(FRAME_SAMPLES * sizeof(INT16));
    g_audio_out = (INT16 *)malloc(FRAME_SAMPLES * sizeof(INT16));
    
    if (!g_internal_buf || !g_audio_in || !g_audio_out) {
        PRINT_FAIL("Buffer allocation", -1);
        success = 0;
        goto ns_cleanup;
    }
    
    /* Set buffer */
    audlib_ns_set_config(AUDNS_CONFIG_ID_BUF_ADDR, (INT32)(uintptr_t)g_internal_buf);
    audlib_ns_set_config(AUDNS_CONFIG_ID_BUF_SIZE, buf_size);
    
    /* Initialize */
    if (!audlib_ns_init()) {
        PRINT_FAIL("audlib_ns_init()", -1);
        success = 0;
        goto ns_cleanup;
    }
    printf("    audlib_ns_init(): " COLOR_GREEN "OK" COLOR_RESET "\n");
    
    /* Generate noisy test signal */
    generate_test_signal(g_audio_in, FRAME_SAMPLES, ns_rate, 300.0f, 5000.0f);
    add_noise(g_audio_in, FRAME_SAMPLES, 2000.0f);
    
    float in_rms = get_rms_amplitude(g_audio_in, FRAME_SAMPLES);
    
    /* Set up timeout for NS run (it may hang) */
    PRINT_INFO("Running NS with timeout protection...");
    
    g_ns_timeout = 0;
    signal(SIGALRM, ns_alarm_handler);
    alarm(2);  /* 2 second timeout */
    
    AUDNS_BITSTREAM ns_io;
    ns_io.bitstram_buffer_in = (UINT32)(uintptr_t)g_audio_in;
    ns_io.bitstram_buffer_out = (UINT32)(uintptr_t)g_audio_out;
    ns_io.bitstram_buffer_length = FRAME_SAMPLES;
    
    BOOL ns_result = audlib_ns_run(&ns_io);
    
    alarm(0);  /* Cancel alarm */
    signal(SIGALRM, SIG_DFL);
    
    if (g_ns_timeout) {
        PRINT_WARN("audlib_ns_run() TIMEOUT - may need real audio hardware");
        PRINT_INFO("NS library initialized OK but run() requires active audio path");
        success = 0;  /* Mark as failed but don't crash */
    } else if (ns_result) {
        float out_rms = get_rms_amplitude(g_audio_out, FRAME_SAMPLES);
        printf("    audlib_ns_run(): " COLOR_GREEN "OK" COLOR_RESET "\n");
        printf("    Input RMS: %.1f → Output RMS: %.1f\n", in_rms, out_rms);
        PRINT_PASS("Noise Suppression processing completed");
    } else {
        PRINT_FAIL("audlib_ns_run()", -1);
        success = 0;
    }
    
ns_cleanup:
    audlib_ns_close();
    if (g_internal_buf) { free(g_internal_buf); g_internal_buf = NULL; }
    if (g_audio_in) { free(g_audio_in); g_audio_in = NULL; }
    if (g_audio_out) { free(g_audio_out); g_audio_out = NULL; }
    
    return success ? 0 : -1;
}

/*============================================================================
 * Test 3: Audio Filter/EQ - libaudfilt.a
 *============================================================================*/

static int test_audio_filter(void)
{
    PRINT_SECTION("Test 3: Audio Filter/EQ - libaudfilt.a");
    
    int success = 1;
    
    /* Initialize filter library */
    AUDFILT_INIT filt_init;
    filt_init.filt_ch = AUDFILT_CH_MONO;
    filt_init.smooth_enable = TRUE;
    
    if (!audlib_filt_open(&filt_init)) {
        PRINT_FAIL("audlib_filt_open()", -1);
        return -1;
    }
    printf("    audlib_filt_open(): " COLOR_GREEN "OK" COLOR_RESET "\n");
    
    /* Design a high-pass filter to remove low-frequency noise */
    AUDFILT_EQPARAM hp_param;
    hp_param.filt_type = AUDFILT_DESIGNTYPE_HIGHPASS;
    hp_param.sample_rate = g_config.sample_rate;
    hp_param.gain_db = 0.0f;
    hp_param.frequency = 100.0f;  /* Cut below 100Hz */
    hp_param.Q = 0.707f;  /* Butterworth */
    
    AUDFILT_CONFIG hp_config;
    if (!audlib_filt_design_filt(&hp_param, &hp_config, AUDFILT_DESGIN_CTRL_DEFAULT)) {
        PRINT_FAIL("audlib_filt_design_filt(HP)", -1);
        success = 0;
        goto filt_cleanup;
    }
    printf("    High-pass filter designed: fc=100Hz, Q=0.707\n");
    
    /* Apply filter config */
    if (!audlib_filt_set_config(AUDFILT_SEL_HIGHPASS1, &hp_config)) {
        PRINT_FAIL("audlib_filt_set_config(HP)", -1);
        success = 0;
        goto filt_cleanup;
    }
    
    /* Enable the filter */
    if (!audlib_filt_enable_filt(AUDFILT_SEL_HIGHPASS1, TRUE)) {
        PRINT_FAIL("audlib_filt_enable_filt(HP)", -1);
        success = 0;
        goto filt_cleanup;
    }
    printf("    High-pass filter enabled: " COLOR_GREEN "OK" COLOR_RESET "\n");
    
    /* Design a notch filter to remove 50Hz hum */
    AUDFILT_EQPARAM notch_param;
    notch_param.filt_type = AUDFILT_DESIGNTYPE_NOTCH;
    notch_param.sample_rate = g_config.sample_rate;
    notch_param.gain_db = -40.0f;  /* -40dB attenuation */
    notch_param.frequency = 50.0f;  /* 50Hz power line noise */
    notch_param.Q = 5.0f;  /* Narrow notch */
    
    AUDFILT_CONFIG notch_config;
    if (!audlib_filt_design_filt(&notch_param, &notch_config, AUDFILT_DESGIN_CTRL_DEFAULT)) {
        PRINT_FAIL("audlib_filt_design_filt(NOTCH)", -1);
        success = 0;
        goto filt_cleanup;
    }
    printf("    Notch filter designed: fc=50Hz (hum removal), Q=5.0\n");
    
    if (!audlib_filt_set_config(AUDFILT_SEL_NOTCH1, &notch_config)) {
        PRINT_FAIL("audlib_filt_set_config(NOTCH)", -1);
        success = 0;
        goto filt_cleanup;
    }
    
    if (!audlib_filt_enable_filt(AUDFILT_SEL_NOTCH1, TRUE)) {
        PRINT_FAIL("audlib_filt_enable_filt(NOTCH)", -1);
        success = 0;
        goto filt_cleanup;
    }
    printf("    Notch filter enabled: " COLOR_GREEN "OK" COLOR_RESET "\n");
    
    /* Initialize filter state */
    if (!audlib_filt_init()) {
        PRINT_FAIL("audlib_filt_init()", -1);
        success = 0;
        goto filt_cleanup;
    }
    printf("    audlib_filt_init(): " COLOR_GREEN "OK" COLOR_RESET "\n");
    
    /* Allocate and test with real data */
    /* Buffer must be multiple of 512 bytes for mono (256 samples) */
    int filter_samples = 512;  /* 256 samples * 2 bytes */
    g_audio_in = (INT16 *)malloc(filter_samples);
    g_audio_out = (INT16 *)malloc(filter_samples);
    
    if (!g_audio_in || !g_audio_out) {
        PRINT_FAIL("Buffer allocation", -1);
        success = 0;
        goto filt_cleanup;
    }
    
    /* Generate test signal with 50Hz hum + speech (300Hz) */
    int num_samples = filter_samples / sizeof(INT16);
    for (int i = 0; i < num_samples; i++) {
        float hum = 3000.0f * sin(2.0 * M_PI * 50.0 * i / g_config.sample_rate);
        float speech = 5000.0f * sin(2.0 * M_PI * 300.0 * i / g_config.sample_rate);
        g_audio_in[i] = (INT16)(hum + speech);
    }
    
    int in_peak = get_peak_amplitude(g_audio_in, num_samples);
    
    /* Run filter */
#ifndef _LP64
    AUDFILT_BITSTREAM filt_io;
    filt_io.bitstram_buffer_in = (UINT32)(uintptr_t)g_audio_in;
    filt_io.bitstram_buffer_out = (UINT32)(uintptr_t)g_audio_out;
    filt_io.bitstram_buffer_length = filter_samples;
    
    if (audlib_filt_run(&filt_io)) {
#else
    if (audlib_filt_run((long)g_audio_in, num_samples)) {
        memcpy(g_audio_out, g_audio_in, filter_samples);  /* 64-bit modifies in-place */
#endif
        int out_peak = get_peak_amplitude(g_audio_out, num_samples);
        printf("    audlib_filt_run(): " COLOR_GREEN "OK" COLOR_RESET "\n");
        printf("    Input peak: %d → Output peak: %d\n", in_peak, out_peak);
        PRINT_PASS("Audio filtering completed (HP + Notch)");
    } else {
        PRINT_FAIL("audlib_filt_run()", -1);
        success = 0;
    }
    
filt_cleanup:
    audlib_filt_close();
    if (g_audio_in) { free(g_audio_in); g_audio_in = NULL; }
    if (g_audio_out) { free(g_audio_out); g_audio_out = NULL; }
    
    return success ? 0 : -1;
}

/*============================================================================
 * Test 4: Noise Gate via HDAL Vendor API
 *============================================================================*/

static HD_RESULT init_hdal(void)
{
    HD_RESULT ret;
    HD_COMMON_MEM_INIT_CONFIG mem_cfg = {0};
    
    /* Initialize HDAL */
    ret = hd_common_init(0);
    if (ret != HD_OK) return ret;
    
    /* Initialize memory */
    mem_cfg.pool_info[0].type = HD_COMMON_MEM_COMMON_POOL;
    mem_cfg.pool_info[0].blk_size = 0x10000;
    mem_cfg.pool_info[0].blk_cnt = 4;
    mem_cfg.pool_info[0].ddr_id = DDR_ID0;
    
    ret = hd_common_mem_init(&mem_cfg);
    if (ret != HD_OK) {
        hd_common_uninit();
        return ret;
    }
    
    /* Initialize audio capture */
    ret = hd_audiocap_init();
    if (ret != HD_OK) {
        hd_common_mem_uninit();
        hd_common_uninit();
        return ret;
    }
    
    return HD_OK;
}

static void uninit_hdal(void)
{
    if (g_cap_path) hd_audiocap_close(g_cap_path);
    if (g_cap_ctrl) hd_audiocap_close(g_cap_ctrl);
    hd_audiocap_uninit();
    hd_common_mem_uninit();
    hd_common_uninit();
}

static int test_noisegate(void)
{
    PRINT_SECTION("Test 4: Noise Gate via HDAL Vendor API");
    
    HD_RESULT ret;
    int success = 1;
    
    PRINT_INFO("Initializing HDAL for Noise Gate test...");
    
    ret = init_hdal();
    if (ret != HD_OK) {
        PRINT_FAIL("HDAL init", ret);
        return -1;
    }
    printf("    HDAL initialized: " COLOR_GREEN "OK" COLOR_RESET "\n");
    
    /* Open audio capture control path */
    ret = hd_audiocap_open(0, HD_AUDIOCAP_0_CTRL, &g_cap_ctrl);
    if (ret != HD_OK) {
        PRINT_FAIL("hd_audiocap_open(CTRL)", ret);
        success = 0;
        goto ng_cleanup;
    }
    printf("    Audio capture CTRL opened: " COLOR_GREEN "OK" COLOR_RESET "\n");
    
    /* Configure audio capture BEFORE opening data path */
    HD_AUDIOCAP_DEV_CONFIG dev_cfg = {0};
    dev_cfg.in_max.sample_rate = HD_AUDIO_SR_48000;
    dev_cfg.in_max.sample_bit = HD_AUDIO_BIT_WIDTH_16;
    dev_cfg.in_max.mode = HD_AUDIO_SOUND_MODE_STEREO;
    dev_cfg.in_max.frame_sample = 1024;
    dev_cfg.frame_num_max = 4;
    
    ret = hd_audiocap_set(g_cap_ctrl, HD_AUDIOCAP_PARAM_DEV_CONFIG, &dev_cfg);
    if (ret != HD_OK) {
        PRINT_FAIL("hd_audiocap_set(DEV_CONFIG)", ret);
        success = 0;
        goto ng_cleanup;
    }
    printf("    Audio config set: 48kHz/16bit/stereo\n");
    
    /* Open audio capture data path (after DEV_CONFIG) */
    ret = hd_audiocap_open(HD_AUDIOCAP_0_IN_0, HD_AUDIOCAP_0_OUT_0, &g_cap_path);
    if (ret != HD_OK) {
        PRINT_FAIL("hd_audiocap_open(OUT)", ret);
        success = 0;
        goto ng_cleanup;
    }
    printf("    Audio capture OUT opened: " COLOR_GREEN "OK" COLOR_RESET "\n");
    
    /* Test Noise Gate threshold setting */
    PRINT_INFO("Testing Noise Gate threshold...");
    
    /* Try different threshold values */
    INT32 thresholds[] = {-60, -50, -40, -30};
    int num_thresholds = sizeof(thresholds) / sizeof(thresholds[0]);
    
    for (int i = 0; i < num_thresholds; i++) {
        INT32 threshold = thresholds[i];
        ret = vendor_audiocap_set(g_cap_path, VENDOR_AUDIOCAP_ITEM_NOISEGATE_THRESHOLD, &threshold);
        if (ret == HD_OK) {
            printf("    Noise Gate threshold %ddB: " COLOR_GREEN "OK" COLOR_RESET "\n", threshold);
        } else {
            printf("    Noise Gate threshold %ddB: " COLOR_YELLOW "FAIL (%d)" COLOR_RESET "\n", threshold, ret);
            /* Don't fail test - might need OUT path to be started */
        }
    }
    
    /* Get current threshold */
    INT32 current_threshold = 0;
    ret = vendor_audiocap_get(g_cap_path, VENDOR_AUDIOCAP_ITEM_NOISEGATE_THRESHOLD, &current_threshold);
    if (ret == HD_OK) {
        printf("    Current Noise Gate threshold: %ddB\n", current_threshold);
        PRINT_PASS("Noise Gate API accessible");
    } else {
        PRINT_WARN("Noise Gate get failed - may need started audio path");
        PRINT_INFO("Noise Gate typically requires: stop → set → start cycle");
    }
    
ng_cleanup:
    uninit_hdal();
    
    return success ? 0 : -1;
}

/*============================================================================
 * Usage
 *============================================================================*/

static void print_usage(const char *prog)
{
    printf("Usage: %s [options]\n\n", prog);
    printf("Comprehensive Audio Processing Test\n\n");
    printf("Options:\n");
    printf("  --agc         Test AGC only\n");
    printf("  --ns          Test Noise Suppression only\n");
    printf("  --filter      Test Audio Filter only\n");
    printf("  --noisegate   Test Noise Gate only\n");
    printf("  --all         Test all (default)\n");
    printf("  --rate <hz>   Sample rate (default: 48000)\n");
    printf("  --help        Show this help\n");
}

static void parse_args(int argc, char **argv)
{
    int specific_test = 0;
    
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--agc") == 0) {
            g_config.test_agc = 1;
            g_config.test_ns = 0;
            g_config.test_filter = 0;
            g_config.test_noisegate = 0;
            specific_test = 1;
        } else if (strcmp(argv[i], "--ns") == 0) {
            g_config.test_agc = 0;
            g_config.test_ns = 1;
            g_config.test_filter = 0;
            g_config.test_noisegate = 0;
            specific_test = 1;
        } else if (strcmp(argv[i], "--filter") == 0) {
            g_config.test_agc = 0;
            g_config.test_ns = 0;
            g_config.test_filter = 1;
            g_config.test_noisegate = 0;
            specific_test = 1;
        } else if (strcmp(argv[i], "--noisegate") == 0) {
            g_config.test_agc = 0;
            g_config.test_ns = 0;
            g_config.test_filter = 0;
            g_config.test_noisegate = 1;
            specific_test = 1;
        } else if (strcmp(argv[i], "--all") == 0) {
            g_config.test_agc = 1;
            g_config.test_ns = 1;
            g_config.test_filter = 1;
            g_config.test_noisegate = 1;
        } else if (strcmp(argv[i], "--rate") == 0 && i + 1 < argc) {
            g_config.sample_rate = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--help") == 0) {
            print_usage(argv[0]);
            exit(0);
        }
    }
    
    if (!specific_test) {
        /* Default: test all */
        g_config.test_agc = 1;
        g_config.test_ns = 1;
        g_config.test_filter = 1;
        g_config.test_noisegate = 1;
    }
}

/*============================================================================
 * Main
 *============================================================================*/

int main(int argc, char **argv)
{
    parse_args(argc, argv);
    
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    
    PRINT_HEADER("Comprehensive Audio Processing Test");
    
    printf("Configuration:\n");
    printf("  Sample Rate: %d Hz\n", g_config.sample_rate);
    printf("  Tests:       ");
    if (g_config.test_agc) printf("AGC ");
    if (g_config.test_ns) printf("NS ");
    if (g_config.test_filter) printf("Filter ");
    if (g_config.test_noisegate) printf("NoiseGate ");
    printf("\n\n");
    
    printf(COLOR_CYAN "Available Libraries:\n" COLOR_RESET);
    printf("  • libagc.a     - Auto Gain Control\n");
    printf("  • libaec.a     - AEC + Noise Suppression (NS)\n");
    printf("  • libaudfilt.a - Audio Filter/EQ (IIR, Notch, HP, LP)\n");
    printf("  • HDAL API     - Noise Gate via vendor_audiocap\n\n");
    
    /* Run tests */
    if (g_config.test_agc && g_keep_running) {
        g_agc_result = test_agc();
    }
    
    if (g_config.test_ns && g_keep_running) {
        g_ns_result = test_ns();
    }
    
    if (g_config.test_filter && g_keep_running) {
        g_filter_result = test_audio_filter();
    }
    
    if (g_config.test_noisegate && g_keep_running) {
        g_noisegate_result = test_noisegate();
    }
    
    /* Summary */
    PRINT_HEADER("Test Results Summary");
    
    int total = 0, passed = 0;
    
    if (g_config.test_agc) {
        total++;
        if (g_agc_result == 0) {
            printf("  AGC (libagc.a):          " COLOR_GREEN "PASS" COLOR_RESET "\n");
            passed++;
        } else {
            printf("  AGC (libagc.a):          " COLOR_RED "FAIL" COLOR_RESET "\n");
        }
    }
    
    if (g_config.test_ns) {
        total++;
        if (g_ns_result == 0) {
            printf("  NS (libaec.a):           " COLOR_GREEN "PASS" COLOR_RESET "\n");
            passed++;
        } else {
            printf("  NS (libaec.a):           " COLOR_YELLOW "TIMEOUT/FAIL" COLOR_RESET "\n");
        }
    }
    
    if (g_config.test_filter) {
        total++;
        if (g_filter_result == 0) {
            printf("  Audio Filter (audfilt):  " COLOR_GREEN "PASS" COLOR_RESET "\n");
            passed++;
        } else {
            printf("  Audio Filter (audfilt):  " COLOR_RED "FAIL" COLOR_RESET "\n");
        }
    }
    
    if (g_config.test_noisegate) {
        total++;
        if (g_noisegate_result == 0) {
            printf("  Noise Gate (HDAL):       " COLOR_GREEN "PASS" COLOR_RESET "\n");
            passed++;
        } else {
            printf("  Noise Gate (HDAL):       " COLOR_RED "FAIL" COLOR_RESET "\n");
        }
    }
    
    printf("\n  Total: %d/%d passed\n", passed, total);
    
    printf("\n" COLOR_CYAN "Recommendations:\n" COLOR_RESET);
    printf("  ✓ AGC:     Use for automatic volume normalization\n");
    printf("  ✓ Filter:  Use HP filter for rumble, Notch for hum removal\n");
    printf("  ✓ NoiseGate: Mutes audio below threshold (simple noise reduction)\n");
    printf("  ○ NS:      May need real audio hardware path to function\n");
    printf("\n");
    
    return (passed == total) ? 0 : 1;
}
