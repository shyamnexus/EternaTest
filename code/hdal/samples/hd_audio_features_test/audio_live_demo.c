/**
 * @file audio_live_demo.c
 * @brief Audio Processing Demo - Compare With and Without Processing
 *
 * Records audio with selectable processing (AGC, HP Filter, Notch Filter)
 * and saves both raw and processed files so you can hear the difference.
 *
 * Usage:
 *   audio_live_demo --agc --hpf 100 --duration 10
 *   audio_live_demo --notch 50 --notch 60 --duration 5
 *   audio_live_demo --compare --agc --duration 10
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>
#include <getopt.h>
#include <errno.h>
#include <math.h>

#include "hdal.h"
#include "hd_debug.h"
#include "vendor_audiocapture.h"

/* Userspace audio libraries */
#include "agc/audlib_agc.h"
#include "audfilt/audlib_filt.h"
#include "aec/audlib_aec.h"  /* Contains AEC and NS (Noise Suppression) */

/* Low-level NS API (from libaec.a, used by uvc_cam) */
typedef struct {
    INT32 s32FrameSize;
    INT32 s32ChannelNum;
    INT32 s32SamplingRate;
} ST_AUD_NS_INFO;

typedef struct {
    UINT32 u32InternalBufSize;
    UINT32 u32InBufSize;
    UINT32 u32OutBufSize;
} ST_AUD_NS_RTN;

typedef enum {
    EN_AUD_NS_BANK_SCALE = 0,
    EN_AUD_NS_NOISE_SUPPRESS
} EN_AUD_NS_PARAM;

extern void AUD_NS_PreInit(ST_AUD_NS_INFO *pInfo, ST_AUD_NS_RTN *pRtn);
extern void AUD_NS_Init(void *pBuf, UINT32 BufSize);
extern void AUD_NS_Run(short *pIn, short *pOut);
extern void AUD_NS_SetParam(EN_AUD_NS_PARAM param, void *pValue);

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
 * Constants
 *============================================================================*/

#define SAMPLE_RATE         48000
#define BIT_WIDTH           16
#define FRAME_SAMPLES       1024
#define MAX_FRAME_NUM       10
#define MAX_NOTCH_FILTERS   4

#define COLOR_RESET   "\033[0m"
#define COLOR_RED     "\033[31m"
#define COLOR_GREEN   "\033[32m"
#define COLOR_YELLOW  "\033[33m"
#define COLOR_BLUE    "\033[34m"
#define COLOR_CYAN    "\033[36m"
#define COLOR_MAGENTA "\033[35m"

#define PRINT_HEADER(title) \
    printf("\n" COLOR_CYAN "╔════════════════════════════════════════════════════════════════╗\n"); \
    printf("║ %-62s ║\n", title); \
    printf("╚════════════════════════════════════════════════════════════════╝" COLOR_RESET "\n\n")

#define PRINT_INFO(fmt, ...)  printf(COLOR_BLUE "  ℹ " fmt COLOR_RESET "\n", ##__VA_ARGS__)
#define PRINT_OK(fmt, ...)    printf(COLOR_GREEN "  ✓ " fmt COLOR_RESET "\n", ##__VA_ARGS__)
#define PRINT_WARN(fmt, ...)  printf(COLOR_YELLOW "  ⚠ " fmt COLOR_RESET "\n", ##__VA_ARGS__)
#define PRINT_ERR(fmt, ...)   printf(COLOR_RED "  ✗ " fmt COLOR_RESET "\n", ##__VA_ARGS__)

/*============================================================================
 * Types
 *============================================================================*/

typedef struct {
    int enable_agc;
    int enable_hpf;
    int enable_notch;
    int enable_noisegate;
    int enable_ns;         /* Noise Suppression (libaec.a) */
    
    int hpf_freq;
    int notch_freqs[MAX_NOTCH_FILTERS];
    int notch_count;
    int noisegate_threshold;
    int ns_level;          /* NS reduction level: -3 to -40 dB */
    
    int volume;
    int duration;
    int sample_rate;
    int frame_samples;
    int compare_mode;  /* Save both raw and processed */
} DemoConfig;

typedef struct {
    HD_PATH_ID cap_ctrl;
    HD_PATH_ID cap_path;
    
    /* Processing state */
    int agc_initialized;
    int filt_initialized;
    int ns_initialized;
    
    /* NS state (low-level API) */
    uint8_t *ns_internal_buf;
    int16_t *ns_out_buf;
    INT32 ns_frame_size;       /* Frame size from AUD_NS_PreInit */
    INT32 ns_buf_unit_size;    /* Buffer unit size for stepping */
    
    /* Buffers */
    UINT64 cap_virt_addr;
    HD_AUDIOCAP_BUFINFO cap_buf_info;
    
    /* Stats */
    uint64_t frames_processed;
    int16_t peak_in;
    int16_t peak_out;
} AudioState;

/*============================================================================
 * Globals
 *============================================================================*/

static volatile int g_keep_running = 1;
static DemoConfig g_config = {
    .enable_agc = 0,
    .enable_hpf = 0,
    .enable_notch = 0,
    .enable_noisegate = 0,
    .enable_ns = 0,
    .hpf_freq = 100,
    .notch_count = 0,
    .noisegate_threshold = -40,
    .ns_level = -20,
    .volume = 100,
    .duration = 10,
    .sample_rate = SAMPLE_RATE,
    .frame_samples = FRAME_SAMPLES,
    .compare_mode = 1  /* Default to compare mode */
};
static AudioState g_audio = {0};

/*============================================================================
 * Signal Handler
 *============================================================================*/

static void signal_handler(int sig)
{
    printf("\n" COLOR_YELLOW "Signal %d received, stopping..." COLOR_RESET "\n", sig);
    g_keep_running = 0;
}

/*============================================================================
 * Memory Management
 *============================================================================*/

static HD_RESULT mem_init(void)
{
    HD_COMMON_MEM_INIT_CONFIG mem_cfg = {0};

    mem_cfg.pool_info[0].type = HD_COMMON_MEM_COMMON_POOL;
    mem_cfg.pool_info[0].blk_size = 0x40000;
    mem_cfg.pool_info[0].blk_cnt = 8;
    mem_cfg.pool_info[0].ddr_id = DDR_ID0;

    return hd_common_mem_init(&mem_cfg);
}

/*============================================================================
 * AGC Processing
 *============================================================================*/

static int init_agc(void)
{
    ER ret = audlib_agc_open();
    if (ret != 0) {  /* E_OK = 0 */
        PRINT_ERR("AGC: Failed to open (ret=%d)", (int)ret);
        return -1;
    }
    
    audlib_agc_set_config(AGC_CONFIG_ID_SAMPLERATE, g_config.sample_rate);
    audlib_agc_set_config(AGC_CONFIG_ID_CHANNEL_NO, 1);
    /* SDK DEFAULT SETTINGS */
    audlib_agc_set_config(AGC_CONFIG_ID_TARGET_LVL, AGC_DB(-6));   /* SDK default */
    audlib_agc_set_config(AGC_CONFIG_ID_MAXGAIN, AGC_DB(20));      /* SDK default */
    audlib_agc_set_config(AGC_CONFIG_ID_MINGAIN, AGC_DB(-10));     /* SDK default */
    audlib_agc_set_config(AGC_CONFIG_ID_ATTACK_TIME, AGC_TRESO_BASIS_100MS);  /* SDK default */
    audlib_agc_set_config(AGC_CONFIG_ID_DECAY_TIME, AGC_TRESO_BASIS_200MS);   /* SDK default */
    audlib_agc_set_config(AGC_CONFIG_ID_NG_THD, AGC_DB(-50));      /* SDK default */
    
    if (!audlib_agc_init()) {
        PRINT_ERR("AGC: Failed to initialize");
        audlib_agc_close();
        return -1;
    }
    
    g_audio.agc_initialized = 1;
    PRINT_OK("AGC SDK defaults: target=-6dB, gain=[-10,+20]dB, attack=100ms, decay=200ms, ng=-50dB");
    return 0;
}

static void process_agc(int16_t *in_buf, int16_t *out_buf, int sample_count)
{
    if (!g_audio.agc_initialized) {
        memcpy(out_buf, in_buf, sample_count * sizeof(int16_t));
        return;
    }
    
    AGC_BITSTREAM agc_io;
    agc_io.bitstram_buffer_in = (UINT64)(uintptr_t)in_buf;
    agc_io.bitstram_buffer_out = (UINT64)(uintptr_t)out_buf;
    agc_io.bitstram_buffer_length = sample_count;
    
    if (!audlib_agc_run(&agc_io)) {
        memcpy(out_buf, in_buf, sample_count * sizeof(int16_t));
    }
}

static void cleanup_agc(void)
{
    if (g_audio.agc_initialized) {
        audlib_agc_close();
        g_audio.agc_initialized = 0;
    }
}

/*============================================================================
 * Audio Filter Processing
 *============================================================================*/

static int init_filter(void)
{
    AUDFILT_INIT filt_init;
    AUDFILT_EQPARAM eq_param;
    AUDFILT_CONFIG filt_cfg;
    
    filt_init.filt_ch = AUDFILT_CH_MONO;
    filt_init.smooth_enable = TRUE;
    
    if (!audlib_filt_open(&filt_init)) {
        PRINT_ERR("Filter: Failed to open");
        return -1;
    }
    
    if (!audlib_filt_init()) {
        PRINT_ERR("Filter: Failed to initialize");
        audlib_filt_close();
        return -1;
    }
    
    /* High-pass filter - gentler settings */
    if (g_config.enable_hpf) {
        memset(&eq_param, 0, sizeof(eq_param));
        eq_param.filt_type = AUDFILT_DESIGNTYPE_HIGHPASS;
        eq_param.sample_rate = g_config.sample_rate;
        eq_param.frequency = (float)g_config.hpf_freq;
        eq_param.Q = 0.5f;    /* Lower Q = gentler rolloff */
        eq_param.gain_db = 0;
        
        if (audlib_filt_design_filt(&eq_param, &filt_cfg, AUDFILT_DESGIN_CTRL_DEFAULT)) {
            audlib_filt_set_config(AUDFILT_SEL_HIGHPASS1, &filt_cfg);
            audlib_filt_enable_filt(AUDFILT_SEL_HIGHPASS1, TRUE);
            PRINT_OK("High-pass filter: %d Hz (Q=0.5)", g_config.hpf_freq);
        }
    }
    
    /* Notch filters - narrow band removal for hum */
    for (int i = 0; i < g_config.notch_count && i < MAX_NOTCH_FILTERS; i++) {
        memset(&eq_param, 0, sizeof(eq_param));
        eq_param.filt_type = AUDFILT_DESIGNTYPE_NOTCH;
        eq_param.sample_rate = g_config.sample_rate;
        eq_param.frequency = (float)g_config.notch_freqs[i];
        eq_param.Q = 5.0f;        /* SDK default - narrow notch */
        eq_param.gain_db = -40.0f; /* SDK default - moderate attenuation */
        
        AUDFILT_SELECTION notch_sel = (AUDFILT_SELECTION)(AUDFILT_SEL_NOTCH1 + i);
        if (audlib_filt_design_filt(&eq_param, &filt_cfg, AUDFILT_DESGIN_CTRL_DEFAULT)) {
            audlib_filt_set_config(notch_sel, &filt_cfg);
            audlib_filt_enable_filt(notch_sel, TRUE);
            PRINT_OK("Notch filter: %d Hz (Q=5, -40dB)", g_config.notch_freqs[i]);
        }
    }
    
    g_audio.filt_initialized = 1;
    return 0;
}

static void process_filter(int16_t *samples, int sample_count)
{
    if (!g_audio.filt_initialized) return;
    
    /* Filter API varies by architecture */
#ifdef _LP64
    /* 64-bit: API takes (buffer, byte_length) - note: bytes not samples! */
    audlib_filt_run((long)(uintptr_t)samples, sample_count * 2);  /* x2 for bytes */
#else
    /* 32-bit: use struct-based API */
    AUDFILT_BITSTREAM filt_io;
    filt_io.bitstram_buffer_in = (UINT32)(uintptr_t)samples;
    filt_io.bitstram_buffer_out = (UINT32)(uintptr_t)samples;
    filt_io.bitstram_buffer_length = sample_count * sizeof(int16_t);
    audlib_filt_run(&filt_io);
#endif
}

static void cleanup_filter(void)
{
    if (g_audio.filt_initialized) {
        audlib_filt_close();
        g_audio.filt_initialized = 0;
    }
}

/*============================================================================
 * Noise Suppression Processing (libaec.a - Low-level AUD_NS_* API)
 * Uses the same approach as uvc_cam.c which works with smaller frame sizes
 *============================================================================*/

static int init_ns(void)
{
    ST_AUD_NS_INFO ns_info_pre;
    ST_AUD_NS_RTN ns_rtn;
    INT32 params[2];
    
    /* Configure NS parameters */
    ns_info_pre.s32SamplingRate = g_config.sample_rate;
    ns_info_pre.s32ChannelNum = 1;  /* Mono */
    
    /* Frame size depends on sample rate:
     * sr >= 19200: 512 samples
     * sr >= 8000:  256 samples
     */
    if (g_config.sample_rate > 19200) {
        ns_info_pre.s32FrameSize = 512;
    } else if (g_config.sample_rate >= 8000) {
        ns_info_pre.s32FrameSize = 256;
    } else {
        PRINT_ERR("NS: Sample rate %d too low (min 8000Hz)", g_config.sample_rate);
        return -1;
    }
    
    /* Get required buffer sizes */
    AUD_NS_PreInit(&ns_info_pre, &ns_rtn);
    
    PRINT_INFO("NS: FrameSize=%d, InternalBuf=%u, InBuf=%u, OutBuf=%u",
               ns_info_pre.s32FrameSize, ns_rtn.u32InternalBufSize,
               ns_rtn.u32InBufSize, ns_rtn.u32OutBufSize);
    
    /* Allocate buffers */
    g_audio.ns_internal_buf = (uint8_t *)malloc(ns_rtn.u32InternalBufSize);
    g_audio.ns_out_buf = (int16_t *)malloc(ns_rtn.u32OutBufSize);
    
    if (!g_audio.ns_internal_buf || !g_audio.ns_out_buf) {
        PRINT_ERR("NS: Buffer allocation failed");
        return -1;
    }
    
    /* Save frame size for processing */
    g_audio.ns_frame_size = ns_info_pre.s32FrameSize;
    g_audio.ns_buf_unit_size = ns_rtn.u32OutBufSize;
    
    /* Initialize NS */
    AUD_NS_Init((void *)g_audio.ns_internal_buf, ns_rtn.u32InternalBufSize);
    
    /* Set noise suppression level */
    params[0] = g_config.ns_level;
    AUD_NS_SetParam(EN_AUD_NS_NOISE_SUPPRESS, (void *)params);
    
    /* Use linear scaling (less CPU) */
    params[0] = 0;  /* 0 = less CPU, 1 = linear */
    AUD_NS_SetParam(EN_AUD_NS_BANK_SCALE, (void *)params);
    
    g_audio.ns_initialized = 1;
    PRINT_OK("Noise Suppression: level=%ddB @ %dHz (frame=%d samples)",
             g_config.ns_level, g_config.sample_rate, g_audio.ns_frame_size);
    return 0;
}

static void process_ns(int16_t *samples, int sample_count)
{
    if (!g_audio.ns_initialized) return;
    
    /* Process in frame_size chunks using low-level AUD_NS_Run() */
    int processed = 0;
    short *in_ptr = (short *)samples;
    short *out_ptr = (short *)g_audio.ns_out_buf;
    
    while (processed + g_audio.ns_frame_size <= sample_count) {
        AUD_NS_Run(in_ptr, out_ptr);
        
        /* Copy processed data back to input buffer (in-place) */
        memcpy(in_ptr, out_ptr, g_audio.ns_frame_size * sizeof(int16_t));
        
        in_ptr += g_audio.ns_frame_size;
        processed += g_audio.ns_frame_size;
    }
    
    /* If there's a remainder, we skip it (will be handled in next frame) */
}

static void cleanup_ns(void)
{
    if (g_audio.ns_initialized) {
        if (g_audio.ns_internal_buf) {
            free(g_audio.ns_internal_buf);
            g_audio.ns_internal_buf = NULL;
        }
        if (g_audio.ns_out_buf) {
            free(g_audio.ns_out_buf);
            g_audio.ns_out_buf = NULL;
        }
        g_audio.ns_initialized = 0;
    }
}

/*============================================================================
 * HDAL Audio Setup
 *============================================================================*/

static HD_RESULT setup_audio_capture(void)
{
    HD_RESULT ret;
    HD_AUDIOCAP_DEV_CONFIG dev_cfg = {0};
    HD_AUDIOCAP_DRV_CONFIG drv_cfg = {0};
    HD_AUDIOCAP_IN in_cfg = {0};
    HD_AUDIOCAP_OUT out_cfg = {0};
    HD_AUDIOCAP_VOLUME vol_cfg = {0};

    ret = hd_audiocap_open(0, HD_AUDIOCAP_0_CTRL, &g_audio.cap_ctrl);
    if (ret != HD_OK) {
        PRINT_ERR("Failed to open CTRL: %d", ret);
        return ret;
    }

    dev_cfg.in_max.sample_rate = g_config.sample_rate;
    dev_cfg.in_max.sample_bit = HD_AUDIO_BIT_WIDTH_16;
    dev_cfg.in_max.mode = HD_AUDIO_SOUND_MODE_MONO;
    dev_cfg.in_max.frame_sample = g_config.frame_samples;
    dev_cfg.frame_num_max = MAX_FRAME_NUM;
    
    ret = hd_audiocap_set(g_audio.cap_ctrl, HD_AUDIOCAP_PARAM_DEV_CONFIG, &dev_cfg);
    if (ret != HD_OK) {
        PRINT_ERR("Failed to set DEV_CONFIG: %d", ret);
        return ret;
    }

    drv_cfg.mono = HD_AUDIO_MONO_RIGHT;
    hd_audiocap_set(g_audio.cap_ctrl, HD_AUDIOCAP_PARAM_DRV_CONFIG, &drv_cfg);

    ret = hd_audiocap_open(HD_AUDIOCAP_0_IN_0, HD_AUDIOCAP_0_OUT_0, &g_audio.cap_path);
    if (ret != HD_OK) {
        PRINT_ERR("Failed to open path: %d", ret);
        return ret;
    }

    in_cfg.sample_rate = g_config.sample_rate;
    in_cfg.sample_bit = HD_AUDIO_BIT_WIDTH_16;
    in_cfg.mode = HD_AUDIO_SOUND_MODE_MONO;
    in_cfg.frame_sample = g_config.frame_samples;
    hd_audiocap_set(g_audio.cap_path, HD_AUDIOCAP_PARAM_IN, &in_cfg);

    out_cfg.sample_rate = 0;
    hd_audiocap_set(g_audio.cap_path, HD_AUDIOCAP_PARAM_OUT, &out_cfg);

    vol_cfg.volume = g_config.volume;
    hd_audiocap_set(g_audio.cap_ctrl, HD_AUDIOCAP_PARAM_VOLUME, &vol_cfg);

    /* Set noise gate if enabled */
    if (g_config.enable_noisegate) {
        INT32 threshold = g_config.noisegate_threshold;
        ret = vendor_audiocap_set(g_audio.cap_path, VENDOR_AUDIOCAP_ITEM_NOISEGATE_THRESHOLD, &threshold);
        if (ret == HD_OK) {
            PRINT_OK("Noise Gate: %d dB", threshold);
        }
    }

    ret = hd_audiocap_get(g_audio.cap_ctrl, HD_AUDIOCAP_PARAM_BUFINFO, &g_audio.cap_buf_info);
    if (ret != HD_OK) {
        PRINT_ERR("Failed to get buffer info: %d", ret);
        return ret;
    }

    g_audio.cap_virt_addr = (UINT64)hd_common_mem_mmap(HD_COMMON_MEM_MEM_TYPE_CACHE,
                                g_audio.cap_buf_info.buf_info.phy_addr,
                                g_audio.cap_buf_info.buf_info.buf_size);
    if (g_audio.cap_virt_addr == 0) {
        PRINT_ERR("Failed to mmap capture buffer");
        return HD_ERR_NG;
    }

    PRINT_OK("Audio capture: %d Hz, mono, 16-bit", g_config.sample_rate);
    return HD_OK;
}

/*============================================================================
 * Recording with Comparison
 *============================================================================*/

static void update_peak(int16_t *samples, int count, int16_t *peak)
{
    for (int i = 0; i < count; i++) {
        int16_t val = samples[i] > 0 ? samples[i] : -samples[i];
        if (val > *peak) *peak = val;
    }
}

static void run_recording(void)
{
    char raw_file[256], proc_file[256];
    FILE *fp_raw = NULL, *fp_proc = NULL;
    HD_RESULT ret;
    HD_AUDIO_FRAME cap_frame;
    int16_t *process_buf = NULL;
    int16_t *agc_out_buf = NULL;
    int duration_frames = (g_config.duration * g_config.sample_rate) / g_config.frame_samples;
    int frame_count = 0;
    
    snprintf(raw_file, sizeof(raw_file), "/mnt/sd/audio_raw_%dHz.pcm", g_config.sample_rate);
    snprintf(proc_file, sizeof(proc_file), "/mnt/sd/audio_processed_%dHz.pcm", g_config.sample_rate);
    
    PRINT_HEADER("Recording Audio");
    
    /* Print what features are enabled */
    printf("  " COLOR_CYAN "Processing Pipeline:" COLOR_RESET "\n");
    if (g_config.enable_agc) {
        printf("    → AGC (Auto Gain Control)\n");
    }
    if (g_config.enable_hpf) {
        printf("    → High-Pass Filter: %d Hz\n", g_config.hpf_freq);
    }
    for (int i = 0; i < g_config.notch_count; i++) {
        printf("    → Notch Filter: %d Hz\n", g_config.notch_freqs[i]);
    }
    if (g_config.enable_noisegate) {
        printf("    → Noise Gate: %d dB\n", g_config.noisegate_threshold);
    }
    if (g_config.enable_ns) {
        printf("    → Noise Suppression: %d dB\n", g_config.ns_level);
    }
    if (!g_config.enable_agc && !g_config.enable_hpf && g_config.notch_count == 0 && 
        !g_config.enable_noisegate && !g_config.enable_ns) {
        printf("    (No processing - passthrough)\n");
    }
    printf("\n");
    
    PRINT_INFO("Duration: %d seconds", g_config.duration);
    PRINT_INFO("Raw file:       %s", raw_file);
    PRINT_INFO("Processed file: %s", proc_file);
    printf("\n");
    
    fp_raw = fopen(raw_file, "wb");
    fp_proc = fopen(proc_file, "wb");
    if (!fp_raw || !fp_proc) {
        PRINT_ERR("Failed to open output files");
        if (fp_raw) fclose(fp_raw);
        if (fp_proc) fclose(fp_proc);
        return;
    }
    
    process_buf = (int16_t *)malloc(g_config.frame_samples * 2 * sizeof(int16_t));
    agc_out_buf = (int16_t *)malloc(g_config.frame_samples * 2 * sizeof(int16_t));
    if (!process_buf || !agc_out_buf) {
        PRINT_ERR("Failed to allocate buffers");
        goto cleanup;
    }
    
    /* Start capture */
    ret = hd_audiocap_start(g_audio.cap_path);
    if (ret != HD_OK) {
        PRINT_ERR("Failed to start capture: %d", ret);
        goto cleanup;
    }
    
    printf(COLOR_MAGENTA "  🎤 Recording... speak into the microphone!\n\n" COLOR_RESET);
    
    g_audio.peak_in = 0;
    g_audio.peak_out = 0;
    
    while (frame_count < duration_frames && g_keep_running) {
        ret = hd_audiocap_pull_out_buf(g_audio.cap_path, &cap_frame, 200);
        if (ret != HD_OK) continue;
        
        int16_t *cap_data = (int16_t *)(g_audio.cap_virt_addr + 
                                        (cap_frame.phy_addr[0] - g_audio.cap_buf_info.buf_info.phy_addr));
        int sample_count = cap_frame.size / sizeof(int16_t);
        
        /* Track input peak */
        update_peak(cap_data, sample_count, &g_audio.peak_in);
        
        /* Save raw audio */
        fwrite(cap_data, 1, cap_frame.size, fp_raw);
        
        /* Copy and process */
        memcpy(process_buf, cap_data, cap_frame.size);
        
        /* PROCESSING ORDER: Filter → NS → AGC
         * - HPF/Notch: Remove unwanted frequencies first
         * - NS: Reduce broadband noise
         * - AGC: Normalize volume last
         */
        if (g_config.enable_hpf || g_config.notch_count > 0) {
            process_filter(process_buf, sample_count);
        }
        
        if (g_config.enable_ns) {
            process_ns(process_buf, sample_count);
        }
        
        if (g_config.enable_agc) {
            process_agc(process_buf, agc_out_buf, sample_count);
            memcpy(process_buf, agc_out_buf, sample_count * sizeof(int16_t));
        }
        
        /* Track output peak */
        update_peak(process_buf, sample_count, &g_audio.peak_out);
        
        /* Save processed audio */
        fwrite(process_buf, 1, cap_frame.size, fp_proc);
        
        hd_audiocap_release_out_buf(g_audio.cap_path, &cap_frame);
        
        frame_count++;
        g_audio.frames_processed++;
        
        /* Progress bar */
        int progress = (frame_count * 100) / duration_frames;
        int bar_pos = progress * 40 / 100;
        printf("\r    [");
        for (int i = 0; i < 40; i++) {
            if (i < bar_pos) printf("=");
            else if (i == bar_pos) printf(">");
            else printf(" ");
        }
        float gain = g_audio.peak_in > 100 ? (float)g_audio.peak_out / g_audio.peak_in : 1.0f;
        printf("] %3d%% | Peak: %5d → %5d (%.1fx)", 
               progress, g_audio.peak_in, g_audio.peak_out, gain);
        fflush(stdout);
        
        /* Reset peaks every second */
        if (frame_count % (g_config.sample_rate / g_config.frame_samples) == 0) {
            g_audio.peak_in = 0;
            g_audio.peak_out = 0;
        }
    }
    
    printf("\n\n");
    
    hd_audiocap_stop(g_audio.cap_path);
    
    PRINT_OK("Recording complete!");
    printf("\n");
    
    /* Print playback instructions */
    PRINT_HEADER("How to Listen");
    
    printf("  " COLOR_CYAN "Option 1: Play on device (if speaker connected):" COLOR_RESET "\n");
    printf("    aplay -f S16_LE -r %d -c 1 /mnt/sd/audio_raw_%dHz.pcm\n", 
           g_config.sample_rate, g_config.sample_rate);
    printf("    aplay -f S16_LE -r %d -c 1 /mnt/sd/audio_processed_%dHz.pcm\n\n",
           g_config.sample_rate, g_config.sample_rate);
    
    printf("  " COLOR_CYAN "Option 2: Transfer to PC:" COLOR_RESET "\n");
    printf("    adb pull /mnt/sd/audio_raw_%dHz.pcm .\n", g_config.sample_rate);
    printf("    adb pull /mnt/sd/audio_processed_%dHz.pcm .\n\n", g_config.sample_rate);
    
    printf("  " COLOR_CYAN "Option 3: Convert to WAV (on PC with sox):" COLOR_RESET "\n");
    printf("    sox -r %d -b 16 -c 1 -e signed-integer audio_raw_%dHz.pcm raw.wav\n",
           g_config.sample_rate, g_config.sample_rate);
    printf("    sox -r %d -b 16 -c 1 -e signed-integer audio_processed_%dHz.pcm processed.wav\n\n",
           g_config.sample_rate, g_config.sample_rate);
    
    printf("  " COLOR_YELLOW "Listen to both files and compare the difference!" COLOR_RESET "\n\n");
    
cleanup:
    if (fp_raw) fclose(fp_raw);
    if (fp_proc) fclose(fp_proc);
    if (process_buf) free(process_buf);
    if (agc_out_buf) free(agc_out_buf);
}

/*============================================================================
 * Help and Args
 *============================================================================*/

static void print_usage(const char *prog)
{
    printf("\n");
    PRINT_HEADER("Audio Processing Demo");
    
    printf("Usage: %s [options]\n\n", prog);
    
    printf(COLOR_YELLOW "Processing Options:" COLOR_RESET "\n");
    printf("  --agc               Enable AGC (auto gain control)\n");
    printf("  --ns [level]        Enable Noise Suppression (-20dB default, range: -3 to -40)\n");
    printf("  --hpf <hz>          Enable high-pass filter (default: 100 Hz)\n");
    printf("  --notch <hz>        Add notch filter (can use multiple times)\n");
    printf("  --noisegate <dB>    Enable noise gate (e.g., -40)\n");
    printf("\n");
    
    printf(COLOR_YELLOW "General Options:" COLOR_RESET "\n");
    printf("  --volume <0-200>    Capture volume (default: 100)\n");
    printf("  --duration <sec>    Duration in seconds (default: 10)\n");
    printf("  --rate <hz>         Sample rate (8000 for NS, 48000 default)\n");
    printf("  --help              Show this help\n");
    printf("\n");
    
    printf(COLOR_YELLOW "Examples:" COLOR_RESET "\n");
    printf("  %s --agc --duration 5                # AGC only, 5 seconds\n", prog);
    printf("  %s --hpf 100 --notch 50 --notch 60   # HP + remove 50/60Hz hum\n", prog);
    printf("  %s --agc --hpf 80 --noisegate -45    # Full processing chain\n", prog);
    printf("\n");
    
    printf(COLOR_YELLOW "What each feature does:" COLOR_RESET "\n");
    printf("  AGC:       Normalizes volume - boosts quiet, reduces loud\n");
    printf("  NS:        Reduces broadband background noise (fans, AC, etc.)\n");
    printf("  HPF:       Removes low-frequency rumble and wind noise\n");
    printf("  Notch:     Removes specific frequency (e.g., 50/60Hz AC hum)\n");
    printf("  NoiseGate: Mutes audio when signal is below threshold\n");
    printf("\n");
}

static void parse_args(int argc, char **argv)
{
    static struct option long_options[] = {
        {"agc",       no_argument,       0, 'a'},
        {"ns",        optional_argument, 0, 's'},
        {"hpf",       required_argument, 0, 'H'},
        {"notch",     required_argument, 0, 'n'},
        {"noisegate", required_argument, 0, 'g'},
        {"volume",    required_argument, 0, 'v'},
        {"duration",  required_argument, 0, 'd'},
        {"rate",      required_argument, 0, 'r'},
        {"help",      no_argument,       0, 'h'},
        {0, 0, 0, 0}
    };

    int opt;
    while ((opt = getopt_long(argc, argv, "as::H:n:g:v:d:r:h", long_options, NULL)) != -1) {
        switch (opt) {
            case 'a':
                g_config.enable_agc = 1;
                break;
            case 's':
                g_config.enable_ns = 1;
                if (optarg) {
                    g_config.ns_level = atoi(optarg);
                    if (g_config.ns_level > -3) g_config.ns_level = -3;
                    if (g_config.ns_level < -40) g_config.ns_level = -40;
                }
                break;
            case 'r':
                g_config.sample_rate = atoi(optarg);
                break;
            case 'H':
                g_config.enable_hpf = 1;
                g_config.hpf_freq = atoi(optarg);
                break;
            case 'n':
                if (g_config.notch_count < MAX_NOTCH_FILTERS) {
                    g_config.notch_freqs[g_config.notch_count++] = atoi(optarg);
                    g_config.enable_notch = 1;
                }
                break;
            case 'g':
                g_config.enable_noisegate = 1;
                g_config.noisegate_threshold = atoi(optarg);
                break;
            case 'v':
                g_config.volume = atoi(optarg);
                break;
            case 'd':
                g_config.duration = atoi(optarg);
                break;
            case 'h':
            default:
                print_usage(argv[0]);
                exit(0);
        }
    }
    
    /* NS requires 8kHz sample rate and 256-sample frames */
    if (g_config.enable_ns && g_config.sample_rate != 8000) {
        PRINT_WARN("NS requires 8kHz sample rate, overriding from %d to 8000", g_config.sample_rate);
        g_config.sample_rate = 8000;
    }
    if (g_config.enable_ns) {
        g_config.frame_samples = 256;  /* NS works with 256-sample frames at 8kHz */
    }
}

/*============================================================================
 * Main
 *============================================================================*/

int main(int argc, char **argv)
{
    HD_RESULT ret;

    parse_args(argc, argv);

    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    printf("\n");
    
    /* Initialize HDAL */
    ret = hd_common_init(0);
    if (ret != HD_OK) {
        PRINT_ERR("hd_common_init failed: %d", ret);
        return -1;
    }
    
    ret = mem_init();
    if (ret != HD_OK) {
        PRINT_ERR("mem_init failed: %d", ret);
        goto cleanup_common;
    }

    ret = hd_audiocap_init();
    if (ret != HD_OK) {
        PRINT_ERR("hd_audiocap_init failed: %d", ret);
        goto cleanup_mem;
    }
    
    PRINT_OK("HDAL initialized");

    /* Setup capture */
    ret = setup_audio_capture();
    if (ret != HD_OK) {
        goto cleanup_audiocap;
    }

    /* Initialize processing */
    if (g_config.enable_agc) {
        if (init_agc() != 0) {
            PRINT_WARN("AGC init failed, disabling");
            g_config.enable_agc = 0;
        }
    }

    if (g_config.enable_ns) {
        if (init_ns() != 0) {
            PRINT_WARN("NS init failed, disabling");
            g_config.enable_ns = 0;
        }
    }

    if (g_config.enable_hpf || g_config.notch_count > 0) {
        if (init_filter() != 0) {
            PRINT_WARN("Filter init failed, disabling");
            g_config.enable_hpf = 0;
            g_config.notch_count = 0;
        }
    }

    /* Run recording */
    run_recording();

    /* Cleanup */
    cleanup_agc();
    cleanup_ns();
    cleanup_filter();

    if (g_audio.cap_virt_addr) {
        hd_common_mem_munmap((void*)g_audio.cap_virt_addr, g_audio.cap_buf_info.buf_info.buf_size);
    }

    if (g_audio.cap_path) hd_audiocap_close(g_audio.cap_path);
    if (g_audio.cap_ctrl) hd_audiocap_close(g_audio.cap_ctrl);

cleanup_audiocap:
    hd_audiocap_uninit();
cleanup_mem:
    hd_common_mem_uninit();
cleanup_common:
    hd_common_uninit();

    printf(COLOR_GREEN "\nDemo complete!\n" COLOR_RESET);
    return 0;
}
