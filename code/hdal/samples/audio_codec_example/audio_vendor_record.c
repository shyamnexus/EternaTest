/**
    @brief Sample code demonstrating audio recording with VENDOR APIs.
    
    This example shows how to use vendor-specific APIs alongside HDAL
    to access advanced audio features like:
    - AGC (Automatic Gain Control)
    - ALC (Automatic Level Control)  
    - Gain level configuration
    - Default audio settings presets
    - External codec support (I2S)
    
    HDAL provides the basic pipeline operations while vendor APIs
    provide chip-specific extensions for advanced configuration.

    @file audio_vendor_record.c
    @author Generated Example
    @ingroup mhdal
    @note Demonstrates difference between HDAL and Vendor APIs

    Copyright Novatek Microelectronics Corp. 2018.  All rights reserved.
*/

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

/* HDAL headers - High-level abstraction */
#include "hdal.h"
#include "hd_debug.h"

/* Vendor headers - Chip-specific extensions */
#include "vendor_audiocapture.h"
#include "vendor_audioenc.h"

#include <kwrap/examsys.h>

#if defined(__LINUX)
#include <signal.h>
#include <pthread.h>
#else
#include <FreeRTOS_POSIX.h>
#include <FreeRTOS_POSIX/signal.h>
#include <FreeRTOS_POSIX/pthread.h>
#include <kwrap/task.h>
#define sleep(x)    vos_task_delay_ms(1000*x)
#define usleep(x)   vos_task_delay_us(x)
#endif

#define DEBUG_MENU 1

#define CHKPNT          printf("\033[37mCHK: %s, %s: %d\033[0m\r\n",__FILE__,__func__,__LINE__)
#define DBGH(x)         printf("\033[0;35m%s=0x%08X\033[0m\r\n", #x, x)
#define DBGD(x)         printf("\033[0;35m%s=%d\033[0m\r\n", #x, x)

///////////////////////////////////////////////////////////////////////////////
// Configuration Options
///////////////////////////////////////////////////////////////////////////////

/* Audio capture parameters */
#define AUDCAP_SAMPLE_RATE      HD_AUDIO_SR_48000
#define AUDCAP_BIT_WIDTH        HD_AUDIO_BIT_WIDTH_16
#define AUDCAP_SOUND_MODE       HD_AUDIO_SOUND_MODE_STEREO
#define AUDCAP_FRAME_SAMPLES    1024
#define AUDCAP_FRAME_NUM_MAX    10

/* External codec I2S configuration */
#define USE_EXTERNAL_CODEC      0       // Set to 1 for external I2S codec
#define I2S_BIT_CLK_RATIO       32
#define I2S_TDM_CHANNELS        2
#define I2S_OP_MODE             0       // 0=slave, 1=master

/* Vendor feature enables */
#define ENABLE_AGC              1       // Automatic Gain Control
#define ENABLE_ALC              1       // Automatic Level Control
#define ENABLE_GAIN_LEVEL       1       // Manual gain level setting
#define ENABLE_DEFAULT_PRESET   1       // Use vendor default audio preset

///////////////////////////////////////////////////////////////////////////////
// Memory Management (HDAL)
///////////////////////////////////////////////////////////////////////////////

static HD_RESULT mem_init(void)
{
    HD_RESULT ret = HD_OK;
    HD_COMMON_MEM_INIT_CONFIG mem_cfg = {0};

    /* Configure common memory pool (required by HDAL) */
    mem_cfg.pool_info[0].type = HD_COMMON_MEM_COMMON_POOL;
    mem_cfg.pool_info[0].blk_size = 0x1000;
    mem_cfg.pool_info[0].blk_cnt = 1;
    mem_cfg.pool_info[0].ddr_id = DDR_ID0;

    ret = hd_common_mem_init(&mem_cfg);
    return ret;
}

static HD_RESULT mem_exit(void)
{
    return hd_common_mem_uninit();
}

///////////////////////////////////////////////////////////////////////////////
// Audio Capture Configuration - HDAL + Vendor API combination
///////////////////////////////////////////////////////////////////////////////

/**
 * Configure audio capture device using both HDAL and Vendor APIs.
 * 
 * HDAL APIs used:
 *   - hd_audiocap_open()  - Open device/path
 *   - hd_audiocap_set()   - Set standard parameters
 * 
 * Vendor APIs used:
 *   - vendor_audiocap_set() - Set chip-specific parameters
 *     - VENDOR_AUDIOCAP_ITEM_EXT           - External codec config
 *     - VENDOR_AUDIOCAP_ITEM_AGC_CONFIG    - AGC settings
 *     - VENDOR_AUDIOCAP_ITEM_ALC_ENABLE    - Enable/disable ALC
 *     - VENDOR_AUDIOCAP_ITEM_ALC_CONFIG    - ALC parameters
 *     - VENDOR_AUDIOCAP_ITEM_GAIN_LEVEL    - Gain level setting
 *     - VENDOR_AUDIOCAP_ITEM_DEFAULT_SETTING - Preset configurations
 */
static HD_RESULT set_cap_cfg_with_vendor(HD_PATH_ID *p_audio_cap_ctrl)
{
    HD_RESULT ret = HD_OK;
    HD_PATH_ID audio_cap_ctrl = 0;
    
    /* ===== HDAL: Standard device configuration ===== */
    HD_AUDIOCAP_DEV_CONFIG audio_dev_cfg = {0};
    HD_AUDIOCAP_DRV_CONFIG audio_drv_cfg = {0};

    /* Open control path (HDAL) */
    ret = hd_audiocap_open(0, HD_AUDIOCAP_0_CTRL, &audio_cap_ctrl);
    if (ret != HD_OK) {
        printf("hd_audiocap_open ctrl failed: %d\n", ret);
        return ret;
    }

    /* Set device max capabilities (HDAL) */
    audio_dev_cfg.in_max.sample_rate = AUDCAP_SAMPLE_RATE;
    audio_dev_cfg.in_max.sample_bit = AUDCAP_BIT_WIDTH;
    audio_dev_cfg.in_max.mode = AUDCAP_SOUND_MODE;
    audio_dev_cfg.in_max.frame_sample = AUDCAP_FRAME_SAMPLES;
    audio_dev_cfg.frame_num_max = AUDCAP_FRAME_NUM_MAX;
    ret = hd_audiocap_set(audio_cap_ctrl, HD_AUDIOCAP_PARAM_DEV_CONFIG, &audio_dev_cfg);
    if (ret != HD_OK) {
        printf("HDAL: set DEV_CONFIG failed: %d\n", ret);
        return ret;
    }
    printf("HDAL: Device config set (sample_rate=%d, bit=%d, mode=%d)\n",
           AUDCAP_SAMPLE_RATE, AUDCAP_BIT_WIDTH, AUDCAP_SOUND_MODE);

    /* Set driver config (HDAL) */
    audio_drv_cfg.mono = HD_AUDIO_MONO_RIGHT;
    ret = hd_audiocap_set(audio_cap_ctrl, HD_AUDIOCAP_PARAM_DRV_CONFIG, &audio_drv_cfg);
    if (ret != HD_OK) {
        printf("HDAL: set DRV_CONFIG failed: %d\n", ret);
        return ret;
    }

    /* ===== VENDOR: Chip-specific extensions ===== */

#if USE_EXTERNAL_CODEC
    /* Configure external I2S codec (VENDOR API) */
    {
        VENDOR_AUDIOCAP_INIT_CFG vendor_config = {0};
        
        snprintf(vendor_config.driver_name, VENDOR_AUDIOCAP_NAME_LEN-1, "nvt_aud_emu");
        vendor_config.aud_init_cfg.i2s_cfg.bit_clk_ratio = I2S_BIT_CLK_RATIO;
        vendor_config.aud_init_cfg.i2s_cfg.bit_width     = AUDCAP_BIT_WIDTH;
        vendor_config.aud_init_cfg.i2s_cfg.tdm_ch        = I2S_TDM_CHANNELS;
        vendor_config.aud_init_cfg.i2s_cfg.op_mode       = I2S_OP_MODE;
        
        ret = vendor_audiocap_set(audio_cap_ctrl, VENDOR_AUDIOCAP_ITEM_EXT, &vendor_config);
        if (ret != HD_OK) {
            printf("VENDOR: set EXT codec failed: %d\n", ret);
            return ret;
        }
        printf("VENDOR: External codec configured (I2S clk_ratio=%d, tdm_ch=%d)\n",
               I2S_BIT_CLK_RATIO, I2S_TDM_CHANNELS);
    }
#endif

#if ENABLE_DEFAULT_PRESET
    /* Apply vendor default audio preset (VENDOR API)
     * This configures optimal filter settings for the internal codec
     * Options include different gain levels and high-pass filter frequencies
     */
    {
        VENDOR_AUDIOCAP_DEFAULT_SETTING setting = VENDOR_AUDIOCAP_DEFAULT_SETTING_20DB_HP_16K;
        
        ret = vendor_audiocap_set(audio_cap_ctrl, VENDOR_AUDIOCAP_ITEM_DEFAULT_SETTING, &setting);
        if (ret != HD_OK) {
            printf("VENDOR: set DEFAULT_SETTING failed: %d\n", ret);
            /* Non-fatal - continue */
        } else {
            printf("VENDOR: Applied default preset (20dB gain, 16K high-pass filter)\n");
        }
    }
#endif

#if ENABLE_GAIN_LEVEL
    /* Set gain level granularity (VENDOR API) */
    {
        VENDOR_AUDIOCAP_GAIN_LEVEL gain_lvl = VENDOR_AUDIOCAP_GAIN_LEVEL32;
        
        ret = vendor_audiocap_set(audio_cap_ctrl, VENDOR_AUDIOCAP_ITEM_GAIN_LEVEL, &gain_lvl);
        if (ret != HD_OK) {
            printf("VENDOR: set GAIN_LEVEL failed: %d\n", ret);
            /* Non-fatal - continue */
        } else {
            printf("VENDOR: Gain level set to 32 steps\n");
        }
    }
#endif

#if ENABLE_ALC
    /* Enable Automatic Level Control (VENDOR API)
     * ALC automatically adjusts the recording gain to maintain consistent levels
     */
    {
        UINT32 alc_enable = 1;
        
        ret = vendor_audiocap_set(audio_cap_ctrl, VENDOR_AUDIOCAP_ITEM_ALC_ENABLE, &alc_enable);
        if (ret != HD_OK) {
            printf("VENDOR: set ALC_ENABLE failed: %d\n", ret);
            /* Non-fatal - continue */
        } else {
            printf("VENDOR: ALC enabled\n");
        }

        /* Configure ALC parameters */
        VENDOR_AUDIOCAP_ALC_CONFIG alc_cfg = {0};
        alc_cfg.decay_time = 5;      // Medium decay (0-10)
        alc_cfg.attack_time = 3;     // Fast attack (0-10)
        alc_cfg.max_gain = 21;       // +21 dB max
        alc_cfg.min_gain = -6;       // -6 dB min
        
        ret = vendor_audiocap_set(audio_cap_ctrl, VENDOR_AUDIOCAP_ITEM_ALC_CONFIG, &alc_cfg);
        if (ret != HD_OK) {
            printf("VENDOR: set ALC_CONFIG failed: %d\n", ret);
        } else {
            printf("VENDOR: ALC config (decay=%d, attack=%d, gain=%d~%ddB)\n",
                   alc_cfg.decay_time, alc_cfg.attack_time, alc_cfg.min_gain, alc_cfg.max_gain);
        }
    }
#endif

#if ENABLE_AGC
    /* Configure Automatic Gain Control (VENDOR API)
     * AGC adjusts gain based on target level and noise gate threshold
     */
    {
        VENDOR_AUDIOCAP_AGC_CONFIG agc_cfg = {0};
        agc_cfg.enable = TRUE;
        agc_cfg.target_lvl = -12;     // Target level in dB
        agc_cfg.ng_threshold = -45;   // Noise gate threshold in dB
        
        ret = vendor_audiocap_set(audio_cap_ctrl, VENDOR_AUDIOCAP_ITEM_AGC_CONFIG, &agc_cfg);
        if (ret != HD_OK) {
            printf("VENDOR: set AGC_CONFIG failed: %d\n", ret);
            /* Non-fatal - continue */
        } else {
            printf("VENDOR: AGC enabled (target=%ddB, noise_gate=%ddB)\n",
                   agc_cfg.target_lvl, agc_cfg.ng_threshold);
        }
    }
#endif

    /* Set volume (VENDOR API) */
    {
        VENDOR_AUDIOCAP_VOLUME vol = {0};
        vol.volume = 80;  // 0-100
        
        ret = vendor_audiocap_set(audio_cap_ctrl, VENDOR_AUDIOCAP_ITEM_VOLUME, &vol);
        if (ret != HD_OK) {
            printf("VENDOR: set VOLUME failed: %d\n", ret);
        } else {
            printf("VENDOR: Volume set to %d%%\n", vol.volume);
        }
    }

    *p_audio_cap_ctrl = audio_cap_ctrl;
    return HD_OK;
}

/**
 * Set audio capture path parameters (HDAL)
 */
static HD_RESULT set_cap_param(HD_PATH_ID audio_cap_path)
{
    HD_RESULT ret = HD_OK;
    HD_AUDIOCAP_IN audio_cap_param = {0};

    audio_cap_param.sample_rate = AUDCAP_SAMPLE_RATE;
    audio_cap_param.sample_bit = AUDCAP_BIT_WIDTH;
    audio_cap_param.mode = AUDCAP_SOUND_MODE;
    audio_cap_param.frame_sample = AUDCAP_FRAME_SAMPLES;
    
    ret = hd_audiocap_set(audio_cap_path, HD_AUDIOCAP_PARAM_IN, &audio_cap_param);
    return ret;
}

///////////////////////////////////////////////////////////////////////////////
// Audio Encoder Configuration - HDAL + Vendor API combination
///////////////////////////////////////////////////////////////////////////////

/**
 * Configure audio encoder path (HDAL)
 */
static HD_RESULT set_enc_cfg(HD_PATH_ID audio_enc_path, UINT32 enc_type)
{
    HD_RESULT ret = HD_OK;
    HD_AUDIOENC_PATH_CONFIG audio_path_cfg = {0};

    audio_path_cfg.max_mem.codec_type = enc_type;
    audio_path_cfg.max_mem.sample_rate = AUDCAP_SAMPLE_RATE;
    audio_path_cfg.max_mem.sample_bit = AUDCAP_BIT_WIDTH;
    audio_path_cfg.max_mem.mode = AUDCAP_SOUND_MODE;
    
    ret = hd_audioenc_set(audio_enc_path, HD_AUDIOENC_PARAM_PATH_CONFIG, &audio_path_cfg);
    return ret;
}

/**
 * Set audio encoder parameters with vendor extensions
 */
static HD_RESULT set_enc_param_with_vendor(HD_PATH_ID audio_enc_path, UINT32 enc_type)
{
    HD_RESULT ret = HD_OK;
    
    /* ===== HDAL: Standard encoder parameters ===== */
    HD_AUDIOENC_IN audio_in_param = {0};
    HD_AUDIOENC_OUT audio_out_param = {0};

    audio_in_param.sample_rate = AUDCAP_SAMPLE_RATE;
    audio_in_param.sample_bit = AUDCAP_BIT_WIDTH;
    audio_in_param.mode = AUDCAP_SOUND_MODE;
    ret = hd_audioenc_set(audio_enc_path, HD_AUDIOENC_PARAM_IN, &audio_in_param);
    if (ret != HD_OK) {
        printf("HDAL: set enc PARAM_IN failed: %d\n", ret);
        return ret;
    }

    audio_out_param.codec_type = enc_type;
    audio_out_param.aac_adts = (enc_type == HD_AUDIO_CODEC_AAC) ? TRUE : FALSE;
    ret = hd_audioenc_set(audio_enc_path, HD_AUDIOENC_PARAM_OUT, &audio_out_param);
    if (ret != HD_OK) {
        printf("HDAL: set enc PARAM_OUT failed: %d\n", ret);
        return ret;
    }

    /* ===== VENDOR: Encoder-specific extensions ===== */
    
    /* Set AAC MPEG version (VENDOR API) - only for AAC codec */
    if (enc_type == HD_AUDIO_CODEC_AAC) {
        VENDOR_AUDIOENC_AAC_VER aac_ver = VENDOR_AUDIOENC_AAC_VER_MPEG4;
        
        ret = vendor_audioenc_set(audio_enc_path, VENDOR_AUDIOENC_ITEM_AAC_VER, &aac_ver);
        if (ret != HD_OK) {
            printf("VENDOR: set AAC_VER failed: %d\n", ret);
            /* Non-fatal */
        } else {
            printf("VENDOR: AAC version set to MPEG-4\n");
        }
    }

    /* Configure bitstream reserved size (VENDOR API) */
    {
        VENDOR_AUDIOENC_BS_RESERVED_SIZE_CFG bs_cfg = {0};
        bs_cfg.reserved_size = 256;  // Reserved header space
        
        ret = vendor_audioenc_set(audio_enc_path, VENDOR_AUDIOENC_ITEM_BS_RESERVED_SIZE, &bs_cfg);
        if (ret != HD_OK) {
            printf("VENDOR: set BS_RESERVED_SIZE failed: %d\n", ret);
        }
    }

    return HD_OK;
}

///////////////////////////////////////////////////////////////////////////////
// Stream Structure
///////////////////////////////////////////////////////////////////////////////

typedef struct _AUDIO_VENDOR_RECORD {
    /* HDAL paths */
    HD_PATH_ID cap_ctrl;
    HD_PATH_ID cap_path;
    HD_PATH_ID enc_path;
    
    /* Encoder config */
    UINT32 enc_type;
    
    /* Thread control */
    pthread_t enc_thread_id;
    UINT32 enc_exit;
    UINT32 flow_start;
    
    /* Recording duration (0 = manual stop) */
    UINT32 duration_sec;
    
} AUDIO_VENDOR_RECORD;

///////////////////////////////////////////////////////////////////////////////
// Module Lifecycle (HDAL)
///////////////////////////////////////////////////////////////////////////////

static HD_RESULT init_module(void)
{
    HD_RESULT ret;
    
    if ((ret = hd_audiocap_init()) != HD_OK) {
        printf("hd_audiocap_init failed: %d\n", ret);
        return ret;
    }
    
    if ((ret = hd_audioenc_init()) != HD_OK) {
        printf("hd_audioenc_init failed: %d\n", ret);
        return ret;
    }
    
    return HD_OK;
}

static HD_RESULT open_module(AUDIO_VENDOR_RECORD *p_stream)
{
    HD_RESULT ret;
    
    /* Configure capture with vendor extensions */
    ret = set_cap_cfg_with_vendor(&p_stream->cap_ctrl);
    if (ret != HD_OK) {
        printf("set_cap_cfg_with_vendor failed\n");
        return ret;
    }

    /* Open capture path (HDAL) */
    ret = hd_audiocap_open(HD_AUDIOCAP_0_IN_0, HD_AUDIOCAP_0_OUT_0, &p_stream->cap_path);
    if (ret != HD_OK) {
        printf("hd_audiocap_open path failed: %d\n", ret);
        return ret;
    }

    /* Open encoder path (HDAL) */
    ret = hd_audioenc_open(HD_AUDIOENC_0_IN_0, HD_AUDIOENC_0_OUT_0, &p_stream->enc_path);
    if (ret != HD_OK) {
        printf("hd_audioenc_open failed: %d\n", ret);
        return ret;
    }

    return HD_OK;
}

static HD_RESULT close_module(AUDIO_VENDOR_RECORD *p_stream)
{
    HD_RESULT ret;
    
    if ((ret = hd_audiocap_close(p_stream->cap_path)) != HD_OK)
        return ret;
    if ((ret = hd_audioenc_close(p_stream->enc_path)) != HD_OK)
        return ret;
        
    return HD_OK;
}

static HD_RESULT exit_module(void)
{
    HD_RESULT ret;
    
    if ((ret = hd_audiocap_uninit()) != HD_OK)
        return ret;
    if ((ret = hd_audioenc_uninit()) != HD_OK)
        return ret;
        
    return HD_OK;
}

///////////////////////////////////////////////////////////////////////////////
// Encode Thread - Pull bitstream data
///////////////////////////////////////////////////////////////////////////////

static void *encode_thread(void *arg)
{
    AUDIO_VENDOR_RECORD *p_stream = (AUDIO_VENDOR_RECORD *)arg;
    HD_RESULT ret = HD_OK;
    HD_AUDIO_BS data_pull;
    UINTPTR vir_addr_main;
    HD_AUDIOENC_BUFINFO phy_buf_main;
    char file_path_main[64], file_path_len[64], codec_name[8];
    FILE *f_out_main = NULL, *f_out_len = NULL;
    UINT32 total_bytes = 0;
    UINT32 frame_count = 0;
    
    #define PHY2VIRT_MAIN(pa) (vir_addr_main + (pa - phy_buf_main.buf_info.phy_addr))

    /* Wait for flow_start signal */
    while (p_stream->flow_start == 0) {
        sleep(1);
    }

    /* Get codec name for filename */
    switch (p_stream->enc_type) {
        case HD_AUDIO_CODEC_AAC:
            snprintf(codec_name, sizeof(codec_name), "aac");
            break;
        case HD_AUDIO_CODEC_ULAW:
            snprintf(codec_name, sizeof(codec_name), "g711u");
            break;
        case HD_AUDIO_CODEC_ALAW:
            snprintf(codec_name, sizeof(codec_name), "g711a");
            break;
        case HD_AUDIO_CODEC_PCM:
        default:
            snprintf(codec_name, sizeof(codec_name), "pcm");
            break;
    }

    /* Config output file paths */
    snprintf(file_path_main, sizeof(file_path_main), 
             "/mnt/sd/vendor_audio_%d_%d_%d_%s.dat", 
             AUDCAP_BIT_WIDTH, AUDCAP_SOUND_MODE, AUDCAP_SAMPLE_RATE, codec_name);
    snprintf(file_path_len, sizeof(file_path_len), 
             "/mnt/sd/vendor_audio_%d_%d_%d_%s.len", 
             AUDCAP_BIT_WIDTH, AUDCAP_SOUND_MODE, AUDCAP_SAMPLE_RATE, codec_name);

    /* Query physical address of bitstream buffer (HDAL)
       Note: This can ONLY be queried after hd_audioenc_start() is called! */
    hd_audioenc_get(p_stream->enc_path, HD_AUDIOENC_PARAM_BUFINFO, &phy_buf_main);

    /* Memory map the bitstream buffer (HDAL) */
    vir_addr_main = (UINTPTR)hd_common_mem_mmap(HD_COMMON_MEM_MEM_TYPE_CACHE, 
                                                  phy_buf_main.buf_info.phy_addr, 
                                                  phy_buf_main.buf_info.buf_size);
    if (vir_addr_main == 0) {
        printf("ERROR: mmap failed!\n");
        return NULL;
    }

    /* Open output files */
    f_out_main = fopen(file_path_main, "wb");
    if (f_out_main == NULL) {
        printf("ERROR: Cannot open %s\n", file_path_main);
    } else {
        printf("\n=== Recording to: %s ===\n", file_path_main);
    }

    f_out_len = fopen(file_path_len, "wb");
    if (f_out_len == NULL) {
        printf("WARNING: Cannot open %s\n", file_path_len);
    }

    printf("\nPress 'q' to stop recording...\n\n");

    /* Main recording loop - pull encoded bitstream */
    while (p_stream->enc_exit == 0) {
        /* Pull data from encoder (HDAL) */
        ret = hd_audioenc_pull_out_buf(p_stream->enc_path, &data_pull, -1); // blocking

        if (ret == HD_OK) {
            UINT8 *ptr = (UINT8 *)PHY2VIRT_MAIN(data_pull.phy_addr);
            UINT32 size = data_pull.size;
            UINT32 timestamp = hd_gettime_ms();

            /* Write bitstream data */
            if (f_out_main) {
                fwrite(ptr, 1, size, f_out_main);
                fflush(f_out_main);
            }

            /* Write frame length info */
            if (f_out_len) {
                fprintf(f_out_len, "%d %d\n", size, timestamp);
                fflush(f_out_len);
            }

            total_bytes += size;
            frame_count++;

            /* Progress indicator every 100 frames */
            if (frame_count % 100 == 0) {
                printf("  Recorded: %d frames, %d KB\r", frame_count, total_bytes / 1024);
                fflush(stdout);
            }

            /* Release buffer back to encoder (HDAL) */
            ret = hd_audioenc_release_out_buf(p_stream->enc_path, &data_pull);
            if (ret != HD_OK) {
                printf("ERROR: release_out_buf failed: %d\n", ret);
            }
        }
    }

    printf("\n\n=== Recording complete ===\n");
    printf("  Total frames: %d\n", frame_count);
    printf("  Total size: %d bytes (%.2f KB)\n", total_bytes, total_bytes / 1024.0);

    /* Cleanup */
    hd_common_mem_munmap((void *)vir_addr_main, phy_buf_main.buf_info.buf_size);
    
    if (f_out_main) fclose(f_out_main);
    if (f_out_len) fclose(f_out_len);

    return NULL;
}

///////////////////////////////////////////////////////////////////////////////
// Main Entry Point
///////////////////////////////////////////////////////////////////////////////

EXAMFUNC_ENTRY(hd_audio_vendor_record, argc, argv)
{
    HD_RESULT ret;
    INT key;
    AUDIO_VENDOR_RECORD stream = {0};
    UINT32 enc_type = 0;
    UINT32 duration = 0;

    printf("\n");
    printf("╔════════════════════════════════════════════════════════════════╗\n");
    printf("║     Audio Recording with Vendor APIs - Sample Application     ║\n");
    printf("╠════════════════════════════════════════════════════════════════╣\n");
    printf("║  This sample demonstrates using VENDOR APIs alongside HDAL    ║\n");
    printf("║  to access chip-specific audio features:                      ║\n");
    printf("║    - AGC (Automatic Gain Control)                             ║\n");
    printf("║    - ALC (Automatic Level Control)                            ║\n");
    printf("║    - Gain level configuration                                 ║\n");
    printf("║    - External codec support                                   ║\n");
    printf("╚════════════════════════════════════════════════════════════════╝\n");
    printf("\n");

    /* Parse arguments */
    if (argc >= 2) {
        enc_type = atoi(argv[1]);
        if (enc_type > 3) {
            printf("ERROR: Invalid codec type %d (valid: 0=AAC, 1=G711U, 2=G711A, 3=PCM)\n", enc_type);
            return 0;
        }
    } else {
        enc_type = 0;  // Default to AAC
    }

    if (argc >= 3) {
        duration = atoi(argv[2]);
    }

    /* Set encoder type */
    switch (enc_type) {
        case 0: stream.enc_type = HD_AUDIO_CODEC_AAC;  break;
        case 1: stream.enc_type = HD_AUDIO_CODEC_ULAW; break;
        case 2: stream.enc_type = HD_AUDIO_CODEC_ALAW; break;
        case 3: stream.enc_type = HD_AUDIO_CODEC_PCM;  break;
    }
    stream.duration_sec = duration;

    printf("Configuration:\n");
    printf("  Codec: %s\n", enc_type == 0 ? "AAC" : enc_type == 1 ? "G711-uLaw" : enc_type == 2 ? "G711-aLaw" : "PCM");
    printf("  Sample Rate: %d Hz\n", AUDCAP_SAMPLE_RATE);
    printf("  Bit Width: %d bits\n", AUDCAP_BIT_WIDTH == HD_AUDIO_BIT_WIDTH_16 ? 16 : 8);
    printf("  Channels: %s\n", AUDCAP_SOUND_MODE == HD_AUDIO_SOUND_MODE_STEREO ? "Stereo" : "Mono");
    printf("  Duration: %s\n", duration > 0 ? "Auto-stop" : "Manual (press 'q')");
    printf("\n");

    /* Initialize HDAL common */
    printf("[1/7] Initializing HDAL common...\n");
    ret = hd_common_init(0);
    if (ret != HD_OK) {
        printf("ERROR: hd_common_init failed: %d\n", ret);
        goto exit;
    }

    /* Initialize memory */
    printf("[2/7] Initializing memory pool...\n");
    ret = mem_init();
    if (ret != HD_OK) {
        printf("ERROR: mem_init failed: %d\n", ret);
        goto exit;
    }

    /* Initialize audio modules */
    printf("[3/7] Initializing audio modules...\n");
    ret = init_module();
    if (ret != HD_OK) {
        printf("ERROR: init_module failed: %d\n", ret);
        goto exit;
    }

    /* Open modules (includes vendor configuration) */
    printf("[4/7] Opening modules with vendor configuration...\n");
    ret = open_module(&stream);
    if (ret != HD_OK) {
        printf("ERROR: open_module failed: %d\n", ret);
        goto exit;
    }

    /* Set capture parameters */
    printf("[5/7] Setting capture parameters...\n");
    ret = set_cap_param(stream.cap_path);
    if (ret != HD_OK) {
        printf("ERROR: set_cap_param failed: %d\n", ret);
        goto exit;
    }

    /* Set encoder config */
    ret = set_enc_cfg(stream.enc_path, stream.enc_type);
    if (ret != HD_OK) {
        printf("ERROR: set_enc_cfg failed: %d\n", ret);
        goto exit;
    }

    /* Set encoder parameters with vendor extensions */
    ret = set_enc_param_with_vendor(stream.enc_path, stream.enc_type);
    if (ret != HD_OK) {
        printf("ERROR: set_enc_param_with_vendor failed: %d\n", ret);
        goto exit;
    }

    /* Bind capture to encoder (HDAL) */
    printf("[6/7] Binding audio pipeline...\n");
    hd_audiocap_bind(HD_AUDIOCAP_0_OUT_0, HD_AUDIOENC_0_IN_0);

    /* Create encode thread */
    printf("[7/7] Starting recording thread...\n");
    ret = pthread_create(&stream.enc_thread_id, NULL, encode_thread, &stream);
    if (ret < 0) {
        printf("ERROR: pthread_create failed\n");
        goto exit;
    }

    /* Start pipeline (HDAL) */
    hd_audioenc_start(stream.enc_path);
    hd_audiocap_start(stream.cap_path);

    /* Signal thread to start */
    stream.flow_start = 1;

    /* Wait for completion */
    if (duration > 0) {
        printf("\nRecording for %d seconds...\n", duration);
        sleep(duration);
        stream.enc_exit = 1;
    } else {
        printf("\nEnter 'q' to stop, 'd' for debug menu\n");
        while (1) {
            key = NVT_EXAMSYS_GETCHAR();
            if (key == 'q' || key == 0x3) {
                stream.enc_exit = 1;
                break;
            }
#if DEBUG_MENU
            if (key == 'd') {
                hd_debug_run_menu();
                printf("\nEnter 'q' to stop, 'd' for debug menu\n");
            }
#endif
        }
    }

    /* Wait for thread to finish */
    pthread_join(stream.enc_thread_id, NULL);

    /* Stop pipeline (HDAL) */
    hd_audiocap_stop(stream.cap_path);
    hd_audioenc_stop(stream.enc_path);

    /* Unbind (HDAL) */
    hd_audiocap_unbind(HD_AUDIOCAP_0_OUT_0);

exit:
    /* Cleanup */
    printf("\nCleaning up...\n");
    
    close_module(&stream);
    exit_module();
    mem_exit();
    hd_common_uninit();

    printf("Done.\n");
    return 0;
}
