/**
 * @brief Test sample for G.711 audio capture and encoding at 8kHz mono.
 *
 * This sample tests the HDAL audio pipeline with:
 * - Audio capture at 8kHz mono (as documented in SDK)
 * - Hardware G.711 U-law/A-law encoding
 * - Output verification to SD card
 *
 * Usage: hd_audio_g711_test [codec] [sample_rate] [mode]
 *   codec: 0=ULAW, 1=ALAW, 2=PCM (default: 0)
 *   sample_rate: 8000, 16000, 48000 (default: 8000)
 *   mode: 0=mono, 1=stereo (default: 0)
 *
 * @file audio_g711_test.c
 * @author Test
 * @ingroup mhdal
 *
 * Copyright Novatek Microelectronics Corp. 2024. All rights reserved.
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "hdal.h"
#include "hd_debug.h"
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

// ============================================================================
// Debug macros
// ============================================================================
#define CHKPNT      printf("\033[37mCHK: %s, %s: %d\033[0m\r\n", __FILE__, __func__, __LINE__)
#define DBGH(x)     printf("\033[0;35m%s=0x%08X\033[0m\r\n", #x, x)
#define DBGD(x)     printf("\033[0;35m%s=%d\033[0m\r\n", #x, x)

#define LOG_INFO(fmt, ...)  printf("\033[32m[INFO] " fmt "\033[0m\r\n", ##__VA_ARGS__)
#define LOG_WARN(fmt, ...)  printf("\033[33m[WARN] " fmt "\033[0m\r\n", ##__VA_ARGS__)
#define LOG_ERR(fmt, ...)   printf("\033[31m[ERROR] " fmt "\033[0m\r\n", ##__VA_ARGS__)

// ============================================================================
// Constants
// ============================================================================
#define MAX_BITSTREAM_NUM   1
#define BITSTREAM_SIZE      12800
#define TEST_DURATION_SEC   10    // Run test for 10 seconds

// ============================================================================
// Memory initialization
// ============================================================================
static HD_RESULT mem_init(void)
{
    HD_RESULT ret = HD_OK;
    HD_COMMON_MEM_INIT_CONFIG mem_cfg = {0};

    // config common pool
    mem_cfg.pool_info[0].type = HD_COMMON_MEM_COMMON_POOL;
    mem_cfg.pool_info[0].blk_size = 0x2000;  // 8KB blocks
    mem_cfg.pool_info[0].blk_cnt = 4;
    mem_cfg.pool_info[0].ddr_id = DDR_ID0;

    ret = hd_common_mem_init(&mem_cfg);
    return ret;
}

static HD_RESULT mem_exit(void)
{
    HD_RESULT ret = HD_OK;
    ret = hd_common_mem_uninit();
    return ret;
}

// ============================================================================
// Audio capture configuration
// ============================================================================
static HD_RESULT set_cap_cfg(HD_PATH_ID *p_audio_cap_ctrl, 
                              HD_AUDIO_SR sample_rate, 
                              HD_AUDIO_SOUND_MODE mode,
                              UINT32 frame_sample)
{
    HD_RESULT ret;
    HD_PATH_ID audio_cap_ctrl = 0;
    HD_AUDIOCAP_DEV_CONFIG audio_dev_cfg = {0};
    HD_AUDIOCAP_DRV_CONFIG audio_drv_cfg = {0};

    LOG_INFO("Opening audio capture control path...");
    ret = hd_audiocap_open(0, HD_AUDIOCAP_0_CTRL, &audio_cap_ctrl);
    if (ret != HD_OK) {
        LOG_ERR("hd_audiocap_open(CTRL) failed: %d", ret);
        return ret;
    }

    // Set audiocap device configuration
    LOG_INFO("Setting capture device config: SR=%d, mode=%d, frame_sample=%d",
             sample_rate, mode, frame_sample);
    
    audio_dev_cfg.in_max.sample_rate = sample_rate;
    audio_dev_cfg.in_max.sample_bit = HD_AUDIO_BIT_WIDTH_16;
    audio_dev_cfg.in_max.mode = mode;
    audio_dev_cfg.in_max.frame_sample = frame_sample;
    audio_dev_cfg.frame_num_max = 10;
    audio_dev_cfg.out_max.sample_rate = 0;  // No resampling

    ret = hd_audiocap_set(audio_cap_ctrl, HD_AUDIOCAP_PARAM_DEV_CONFIG, &audio_dev_cfg);
    if (ret != HD_OK) {
        LOG_ERR("hd_audiocap_set(DEV_CONFIG) failed: %d", ret);
        return ret;
    }

    // Set driver config (mono channel selection)
    audio_drv_cfg.mono = HD_AUDIO_MONO_RIGHT;
    ret = hd_audiocap_set(audio_cap_ctrl, HD_AUDIOCAP_PARAM_DRV_CONFIG, &audio_drv_cfg);
    if (ret != HD_OK) {
        LOG_WARN("hd_audiocap_set(DRV_CONFIG) failed: %d (may not be critical)", ret);
    }

    *p_audio_cap_ctrl = audio_cap_ctrl;
    LOG_INFO("Audio capture control configured successfully");
    return HD_OK;
}

static HD_RESULT set_cap_param(HD_PATH_ID audio_cap_path,
                                HD_AUDIO_SR sample_rate,
                                HD_AUDIO_SOUND_MODE mode,
                                UINT32 frame_sample)
{
    HD_RESULT ret = HD_OK;
    HD_AUDIOCAP_IN audio_cap_param = {0};

    LOG_INFO("Setting capture path params: SR=%d, mode=%d, frame_sample=%d",
             sample_rate, mode, frame_sample);

    audio_cap_param.sample_rate = sample_rate;
    audio_cap_param.sample_bit = HD_AUDIO_BIT_WIDTH_16;
    audio_cap_param.mode = mode;
    audio_cap_param.frame_sample = frame_sample;
    
    ret = hd_audiocap_set(audio_cap_path, HD_AUDIOCAP_PARAM_IN, &audio_cap_param);
    if (ret != HD_OK) {
        LOG_ERR("hd_audiocap_set(IN) failed: %d", ret);
        return ret;
    }

    LOG_INFO("Audio capture path configured successfully");
    return ret;
}

// ============================================================================
// Audio encoder configuration
// ============================================================================
static HD_RESULT set_enc_cfg(HD_PATH_ID audio_enc_path, 
                              UINT32 enc_type,
                              HD_AUDIO_SR sample_rate,
                              HD_AUDIO_SOUND_MODE mode)
{
    HD_RESULT ret = HD_OK;
    HD_AUDIOENC_PATH_CONFIG audio_path_cfg = {0};

    LOG_INFO("Setting encoder path config: codec=%d, SR=%d, mode=%d",
             enc_type, sample_rate, mode);

    audio_path_cfg.max_mem.codec_type = enc_type;
    audio_path_cfg.max_mem.sample_rate = sample_rate;
    audio_path_cfg.max_mem.sample_bit = HD_AUDIO_BIT_WIDTH_16;
    audio_path_cfg.max_mem.mode = mode;
    
    ret = hd_audioenc_set(audio_enc_path, HD_AUDIOENC_PARAM_PATH_CONFIG, &audio_path_cfg);
    if (ret != HD_OK) {
        LOG_ERR("hd_audioenc_set(PATH_CONFIG) failed: %d", ret);
        return ret;
    }

    LOG_INFO("Encoder path config set successfully");
    return ret;
}

static HD_RESULT set_enc_param(HD_PATH_ID audio_enc_path, 
                                UINT32 enc_type,
                                HD_AUDIO_SR sample_rate,
                                HD_AUDIO_SOUND_MODE mode)
{
    HD_RESULT ret = HD_OK;
    HD_AUDIOENC_IN audio_in_param = {0};
    HD_AUDIOENC_OUT audio_out_param = {0};

    LOG_INFO("Setting encoder input params: SR=%d, mode=%d", sample_rate, mode);

    // Set encoder input parameters
    audio_in_param.sample_rate = sample_rate;
    audio_in_param.sample_bit = HD_AUDIO_BIT_WIDTH_16;
    audio_in_param.mode = mode;
    
    ret = hd_audioenc_set(audio_enc_path, HD_AUDIOENC_PARAM_IN, &audio_in_param);
    if (ret != HD_OK) {
        LOG_ERR("hd_audioenc_set(IN) failed: %d", ret);
        return ret;
    }

    LOG_INFO("Setting encoder output params: codec=%d", enc_type);

    // Set encoder output parameters
    audio_out_param.codec_type = enc_type;
    audio_out_param.aac_adts = (enc_type == HD_AUDIO_CODEC_AAC) ? TRUE : FALSE;  // Enable ADTS for AAC
    
    ret = hd_audioenc_set(audio_enc_path, HD_AUDIOENC_PARAM_OUT, &audio_out_param);
    if (ret != HD_OK) {
        LOG_ERR("hd_audioenc_set(OUT) failed: %d", ret);
        return ret;
    }

    LOG_INFO("Encoder params set successfully");
    return ret;
}

// ============================================================================
// Test context structure
// ============================================================================
typedef struct _AUDIO_G711_TEST {
    // Capture
    HD_PATH_ID cap_ctrl;
    HD_PATH_ID cap_path;

    // Encoder
    HD_PATH_ID enc_path;
    UINT32 enc_type;

    // Thread control
    pthread_t enc_thread_id;
    UINT32 enc_exit;
    UINT32 flow_start;

    // Test parameters
    HD_AUDIO_SR sample_rate;
    HD_AUDIO_SOUND_MODE mode;
    UINT32 frame_sample;

    // Statistics
    UINT32 frames_captured;
    UINT32 bytes_encoded;
    UINT32 errors;

} AUDIO_G711_TEST;

// ============================================================================
// Module init/exit
// ============================================================================
static HD_RESULT init_module(void)
{
    HD_RESULT ret;

    LOG_INFO("Initializing hd_audiocap...");
    ret = hd_audiocap_init();
    if (ret != HD_OK) {
        LOG_ERR("hd_audiocap_init failed: %d", ret);
        return ret;
    }

    LOG_INFO("Initializing hd_audioenc...");
    ret = hd_audioenc_init();
    if (ret != HD_OK) {
        LOG_ERR("hd_audioenc_init failed: %d", ret);
        return ret;
    }

    LOG_INFO("Modules initialized successfully");
    return HD_OK;
}

static HD_RESULT open_module(AUDIO_G711_TEST *p_stream)
{
    HD_RESULT ret;

    // Configure and open capture control
    ret = set_cap_cfg(&p_stream->cap_ctrl, 
                      p_stream->sample_rate, 
                      p_stream->mode,
                      p_stream->frame_sample);
    if (ret != HD_OK) {
        LOG_ERR("set_cap_cfg failed: %d", ret);
        return ret;
    }

    // Open capture path
    LOG_INFO("Opening capture path...");
    ret = hd_audiocap_open(HD_AUDIOCAP_0_IN_0, HD_AUDIOCAP_0_OUT_0, &p_stream->cap_path);
    if (ret != HD_OK) {
        LOG_ERR("hd_audiocap_open(path) failed: %d", ret);
        return ret;
    }

    // Open encoder path
    LOG_INFO("Opening encoder path...");
    ret = hd_audioenc_open(HD_AUDIOENC_0_IN_0, HD_AUDIOENC_0_OUT_0, &p_stream->enc_path);
    if (ret != HD_OK) {
        LOG_ERR("hd_audioenc_open failed: %d", ret);
        return ret;
    }

    LOG_INFO("Modules opened successfully");
    return HD_OK;
}

static HD_RESULT close_module(AUDIO_G711_TEST *p_stream)
{
    HD_RESULT ret;

    if (p_stream->cap_path) {
        ret = hd_audiocap_close(p_stream->cap_path);
        if (ret != HD_OK) {
            LOG_WARN("hd_audiocap_close(path) failed: %d", ret);
        }
    }

    if (p_stream->cap_ctrl) {
        ret = hd_audiocap_close(p_stream->cap_ctrl);
        if (ret != HD_OK) {
            LOG_WARN("hd_audiocap_close(ctrl) failed: %d", ret);
        }
    }

    if (p_stream->enc_path) {
        ret = hd_audioenc_close(p_stream->enc_path);
        if (ret != HD_OK) {
            LOG_WARN("hd_audioenc_close failed: %d", ret);
        }
    }

    return HD_OK;
}

static HD_RESULT exit_module(void)
{
    HD_RESULT ret;

    ret = hd_audiocap_uninit();
    if (ret != HD_OK) {
        LOG_WARN("hd_audiocap_uninit failed: %d", ret);
    }

    ret = hd_audioenc_uninit();
    if (ret != HD_OK) {
        LOG_WARN("hd_audioenc_uninit failed: %d", ret);
    }

    return HD_OK;
}

// ============================================================================
// Encode thread - pulls encoded bitstream and writes to file
// ============================================================================
static void *encode_thread(void *arg)
{
    AUDIO_G711_TEST *p_test = (AUDIO_G711_TEST *)arg;
    HD_RESULT ret = HD_OK;
    HD_AUDIO_BS data_pull;
    UINTPTR vir_addr_main;
    HD_AUDIOENC_BUFINFO phy_buf_main;
    char file_path[128], codec_name[16];
    FILE *f_out = NULL;
    time_t start_time;

    #define PHY2VIRT_MAIN(pa) (vir_addr_main + (pa - phy_buf_main.buf_info.phy_addr))

    // Wait for flow to start
    while (p_test->flow_start == 0) {
        usleep(100000);  // 100ms
    }

    // Determine codec name
    switch (p_test->enc_type) {
        case HD_AUDIO_CODEC_ULAW:
            snprintf(codec_name, sizeof(codec_name), "g711u");
            break;
        case HD_AUDIO_CODEC_ALAW:
            snprintf(codec_name, sizeof(codec_name), "g711a");
            break;
        case HD_AUDIO_CODEC_PCM:
            snprintf(codec_name, sizeof(codec_name), "pcm");
            break;
        case HD_AUDIO_CODEC_AAC:
            snprintf(codec_name, sizeof(codec_name), "aac");
            break;
        case HD_AUDIO_CODEC_ADPCM:
            snprintf(codec_name, sizeof(codec_name), "adpcm");
            break;
        default:
            snprintf(codec_name, sizeof(codec_name), "unknown");
            break;
    }

    // Create output filename
    snprintf(file_path, sizeof(file_path), 
             "/mnt/sd/audio_test_%s_%dHz_%s.raw",
             codec_name,
             (int)p_test->sample_rate,
             (p_test->mode == HD_AUDIO_SOUND_MODE_MONO) ? "mono" : "stereo");

    // Query encoder buffer info (must be after start)
    ret = hd_audioenc_get(p_test->enc_path, HD_AUDIOENC_PARAM_BUFINFO, &phy_buf_main);
    if (ret != HD_OK) {
        LOG_ERR("Failed to get encoder buffer info: %d", ret);
        return NULL;
    }

    LOG_INFO("Encoder buffer: phy=0x%lx, size=%u", 
             (unsigned long)phy_buf_main.buf_info.phy_addr,
             phy_buf_main.buf_info.buf_size);

    // Memory map the buffer
    vir_addr_main = (UINTPTR)hd_common_mem_mmap(
        HD_COMMON_MEM_MEM_TYPE_CACHE,
        phy_buf_main.buf_info.phy_addr,
        phy_buf_main.buf_info.buf_size);

    if (vir_addr_main == 0) {
        LOG_ERR("Failed to mmap encoder buffer");
        return NULL;
    }

    // Open output file
    f_out = fopen(file_path, "wb");
    if (f_out == NULL) {
        LOG_ERR("Failed to open output file: %s", file_path);
    } else {
        LOG_INFO("Writing encoded audio to: %s", file_path);
    }

    LOG_INFO("=================================================");
    LOG_INFO("Starting audio capture/encode test...");
    LOG_INFO("  Codec: %s", codec_name);
    LOG_INFO("  Sample Rate: %d Hz", (int)p_test->sample_rate);
    LOG_INFO("  Mode: %s", (p_test->mode == HD_AUDIO_SOUND_MODE_MONO) ? "mono" : "stereo");
    LOG_INFO("  Frame Samples: %d", p_test->frame_sample);
    LOG_INFO("=================================================");

    start_time = time(NULL);

    // Pull encoded data loop
    while (p_test->enc_exit == 0) {
        ret = hd_audioenc_pull_out_buf(p_test->enc_path, &data_pull, 500);  // 500ms timeout

        if (ret == HD_OK) {
            UINT8 *ptr = (UINT8 *)PHY2VIRT_MAIN(data_pull.phy_addr);
            UINT32 size = data_pull.size;

            p_test->frames_captured++;
            p_test->bytes_encoded += size;

            // Write to file
            if (f_out != NULL) {
                fwrite(ptr, 1, size, f_out);
                fflush(f_out);
            }

            // Print progress every 50 frames
            if ((p_test->frames_captured % 50) == 0) {
                LOG_INFO("Captured %u frames, %u bytes total (frame size: %u)",
                         p_test->frames_captured, p_test->bytes_encoded, size);
            }

            // Release buffer
            ret = hd_audioenc_release_out_buf(p_test->enc_path, &data_pull);
            if (ret != HD_OK) {
                LOG_WARN("release_out_buf failed: %d", ret);
                p_test->errors++;
            }
        } else if (ret == HD_ERR_TIMEDOUT) {
            // Timeout is expected sometimes
            LOG_WARN("Pull timeout (no data for 500ms)");
        } else {
            LOG_ERR("pull_out_buf failed: %d", ret);
            p_test->errors++;
        }

        // Auto-stop after TEST_DURATION_SEC seconds
        if ((time(NULL) - start_time) >= TEST_DURATION_SEC) {
            LOG_INFO("Test duration (%d seconds) reached, stopping...", TEST_DURATION_SEC);
            break;
        }
    }

    // Print final statistics
    LOG_INFO("=================================================");
    LOG_INFO("Test completed!");
    LOG_INFO("  Total frames: %u", p_test->frames_captured);
    LOG_INFO("  Total bytes: %u", p_test->bytes_encoded);
    LOG_INFO("  Errors: %u", p_test->errors);
    if (p_test->frames_captured > 0) {
        LOG_INFO("  Avg frame size: %u bytes", p_test->bytes_encoded / p_test->frames_captured);
        
        // Calculate expected frame size
        UINT32 channels = (p_test->mode == HD_AUDIO_SOUND_MODE_MONO) ? 1 : 2;
        UINT32 expected_pcm_size = p_test->frame_sample * channels * 2;  // 16-bit
        UINT32 expected_g711_size = p_test->frame_sample * channels;     // 8-bit
        
        LOG_INFO("  Expected PCM frame size: %u bytes", expected_pcm_size);
        LOG_INFO("  Expected G.711 frame size: %u bytes", expected_g711_size);
    }
    LOG_INFO("=================================================");

    // Cleanup
    if (vir_addr_main != 0) {
        hd_common_mem_munmap((void *)vir_addr_main, phy_buf_main.buf_info.buf_size);
    }

    if (f_out != NULL) {
        fclose(f_out);
        LOG_INFO("Output file closed: %s", file_path);
    }

    return NULL;
}

// ============================================================================
// Query audio capabilities
// ============================================================================
static void query_audio_caps(void)
{
    HD_RESULT ret;
    HD_PATH_ID ctrl_path = 0;
    HD_AUDIOCAP_SYSCAPS syscaps = {0};
    HD_DEVCOUNT dev_count = {0};

    ret = hd_audiocap_open(0, HD_AUDIOCAP_0_CTRL, &ctrl_path);
    if (ret != HD_OK) {
        LOG_ERR("Failed to open control path for caps query");
        return;
    }

    // Get device count
    ret = hd_audiocap_get(ctrl_path, HD_AUDIOCAP_PARAM_DEVCOUNT, &dev_count);
    if (ret == HD_OK) {
        LOG_INFO("Audio capture device count: max=%u", dev_count.max_dev_count);
    }

    // Get system capabilities
    ret = hd_audiocap_get(ctrl_path, HD_AUDIOCAP_PARAM_SYSCAPS, &syscaps);
    if (ret == HD_OK) {
        LOG_INFO("Audio capture capabilities:");
        LOG_INFO("  Chip ID: 0x%x", syscaps.chip_id);
        LOG_INFO("  Max IN count: %u", syscaps.max_in_count);
        LOG_INFO("  Max OUT count: %u", syscaps.max_out_count);
        LOG_INFO("  Device caps: 0x%x", syscaps.dev_caps);
        
        // Check supported sample rates
        if (syscaps.max_in_count > 0) {
            UINT32 sr_caps = syscaps.support_in_sr[0];
            LOG_INFO("  Supported input sample rates:");
            if (sr_caps & 0x0001) LOG_INFO("    - 8000 Hz");
            if (sr_caps & 0x0002) LOG_INFO("    - 11025 Hz");
            if (sr_caps & 0x0004) LOG_INFO("    - 12000 Hz");
            if (sr_caps & 0x0008) LOG_INFO("    - 16000 Hz");
            if (sr_caps & 0x0010) LOG_INFO("    - 22050 Hz");
            if (sr_caps & 0x0020) LOG_INFO("    - 24000 Hz");
            if (sr_caps & 0x0040) LOG_INFO("    - 32000 Hz");
            if (sr_caps & 0x0080) LOG_INFO("    - 44100 Hz");
            if (sr_caps & 0x0100) LOG_INFO("    - 48000 Hz");
        }
    }

    hd_audiocap_close(ctrl_path);
}

static void query_encoder_caps(void)
{
    HD_RESULT ret;
    HD_PATH_ID ctrl_path = 0;
    HD_AUDIOENC_SYSCAPS syscaps = {0};
    HD_DEVCOUNT dev_count = {0};

    // Open a temporary encoder path just to query caps
    ret = hd_audioenc_open(0, HD_AUDIOENC_0_CTRL, &ctrl_path);
    if (ret != HD_OK) {
        LOG_WARN("Failed to open encoder control path for caps query: %d", ret);
        return;
    }

    // Get device count
    ret = hd_audioenc_get(ctrl_path, HD_AUDIOENC_PARAM_DEVCOUNT, &dev_count);
    if (ret == HD_OK) {
        LOG_INFO("Audio encoder device count: max=%u", dev_count.max_dev_count);
    }

    // Get system capabilities
    ret = hd_audioenc_get(ctrl_path, HD_AUDIOENC_PARAM_SYSCAPS, &syscaps);
    if (ret == HD_OK) {
        LOG_INFO("Audio encoder capabilities:");
        LOG_INFO("  Chip ID: 0x%x", syscaps.chip_id);
        LOG_INFO("  Max IN count: %u", syscaps.max_in_count);
        LOG_INFO("  Max OUT count: %u", syscaps.max_out_count);
        LOG_INFO("  Device caps: 0x%x", syscaps.dev_caps);
        
        // Check supported codecs
        if (syscaps.max_out_count > 0) {
            UINT32 codec_caps = syscaps.out_caps[0];
            LOG_INFO("  Supported output codecs (caps=0x%x):", codec_caps);
            if (codec_caps & 0x00010000) LOG_INFO("    - PCM (passthrough)");
            if (codec_caps & 0x00020000) LOG_INFO("    - AAC");
            if (codec_caps & 0x00040000) LOG_INFO("    - ADPCM");
            if (codec_caps & 0x00080000) LOG_INFO("    - G.711 U-law");
            if (codec_caps & 0x00100000) LOG_INFO("    - G.711 A-law");
        }
    }

    hd_audioenc_close(ctrl_path);
}

// ============================================================================
// Main entry point
// ============================================================================
EXAMFUNC_ENTRY(hd_audio_g711_test, argc, argv)
{
    HD_RESULT ret;
    AUDIO_G711_TEST test = {0};
    INT key;

    LOG_INFO("=================================================");
    LOG_INFO("HDAL Audio G.711 Test Sample");
    LOG_INFO("=================================================");

    // Parse command line arguments
    // Default: G.711 U-law, 8kHz, mono
    test.enc_type = HD_AUDIO_CODEC_ULAW;
    test.sample_rate = HD_AUDIO_SR_8000;
    test.mode = HD_AUDIO_SOUND_MODE_MONO;
    test.frame_sample = 160;  // 20ms @ 8kHz

    if (argc >= 2) {
        int codec = atoi(argv[1]);
        if (codec == 0) {
            test.enc_type = HD_AUDIO_CODEC_ULAW;
        } else if (codec == 1) {
            test.enc_type = HD_AUDIO_CODEC_ALAW;
        } else if (codec == 2) {
            test.enc_type = HD_AUDIO_CODEC_PCM;
        } else if (codec == 3) {
            test.enc_type = HD_AUDIO_CODEC_AAC;
            // AAC needs higher sample rate and more samples
            test.sample_rate = HD_AUDIO_SR_48000;
            test.frame_sample = 1024;
            test.mode = HD_AUDIO_SOUND_MODE_STEREO;
        } else if (codec == 4) {
            test.enc_type = HD_AUDIO_CODEC_ADPCM;
        }
    }

    if (argc >= 3) {
        int sr = atoi(argv[2]);
        if (sr == 8000) {
            test.sample_rate = HD_AUDIO_SR_8000;
            test.frame_sample = 160;  // 20ms @ 8kHz
        } else if (sr == 11025) {
            test.sample_rate = HD_AUDIO_SR_11025;
            test.frame_sample = 220;  // ~20ms @ 11025Hz
        } else if (sr == 12000) {
            test.sample_rate = HD_AUDIO_SR_12000;
            test.frame_sample = 240;  // 20ms @ 12kHz
        } else if (sr == 16000) {
            test.sample_rate = HD_AUDIO_SR_16000;
            test.frame_sample = 320;  // 20ms @ 16kHz
        } else if (sr == 22050) {
            test.sample_rate = HD_AUDIO_SR_22050;
            test.frame_sample = 440;  // ~20ms @ 22050Hz (4-byte aligned)
        } else if (sr == 24000) {
            test.sample_rate = HD_AUDIO_SR_24000;
            test.frame_sample = 480;  // 20ms @ 24kHz
        } else if (sr == 32000) {
            test.sample_rate = HD_AUDIO_SR_32000;
            test.frame_sample = 640;  // 20ms @ 32kHz
        } else if (sr == 44100) {
            test.sample_rate = HD_AUDIO_SR_44100;
            test.frame_sample = 880;  // ~20ms @ 44100Hz (4-byte aligned)
        } else if (sr == 48000) {
            test.sample_rate = HD_AUDIO_SR_48000;
            test.frame_sample = 1024;  // ~21ms @ 48kHz
        } else {
            LOG_WARN("Unknown sample rate %d, defaulting to 48000 Hz", sr);
            test.sample_rate = HD_AUDIO_SR_48000;
            test.frame_sample = 1024;
        }
    }

    if (argc >= 4) {
        int mode = atoi(argv[3]);
        if (mode == 0) {
            test.mode = HD_AUDIO_SOUND_MODE_MONO;
        } else {
            test.mode = HD_AUDIO_SOUND_MODE_STEREO;
        }
    }

    LOG_INFO("Test configuration:");
    LOG_INFO("  Codec: %s", 
             (test.enc_type == HD_AUDIO_CODEC_ULAW) ? "G.711 U-law" :
             (test.enc_type == HD_AUDIO_CODEC_ALAW) ? "G.711 A-law" :
             (test.enc_type == HD_AUDIO_CODEC_AAC) ? "AAC" :
             (test.enc_type == HD_AUDIO_CODEC_ADPCM) ? "ADPCM" : "PCM");
    LOG_INFO("  Sample Rate: %d Hz", (int)test.sample_rate);
    LOG_INFO("  Mode: %s", (test.mode == HD_AUDIO_SOUND_MODE_MONO) ? "Mono" : "Stereo");
    LOG_INFO("  Frame Samples: %d", test.frame_sample);

    // Initialize HDAL
    LOG_INFO("Initializing HDAL...");
    ret = hd_common_init(0);
    if (ret != HD_OK) {
        LOG_ERR("hd_common_init failed: %d", ret);
        goto exit;
    }

    // Initialize memory
    ret = mem_init();
    if (ret != HD_OK) {
        LOG_ERR("mem_init failed: %d", ret);
        goto exit;
    }

    // Initialize modules
    ret = init_module();
    if (ret != HD_OK) {
        LOG_ERR("init_module failed: %d", ret);
        goto exit;
    }

    // Query capabilities before opening
    query_audio_caps();
    query_encoder_caps();

    // Open modules
    ret = open_module(&test);
    if (ret != HD_OK) {
        LOG_ERR("open_module failed: %d", ret);
        goto exit;
    }

    // Set capture parameters
    ret = set_cap_param(test.cap_path, test.sample_rate, test.mode, test.frame_sample);
    if (ret != HD_OK) {
        LOG_ERR("set_cap_param failed: %d", ret);
        goto exit;
    }

    // Set encoder path config
    ret = set_enc_cfg(test.enc_path, test.enc_type, test.sample_rate, test.mode);
    if (ret != HD_OK) {
        LOG_ERR("set_enc_cfg failed: %d", ret);
        goto exit;
    }

    // Set encoder parameters
    ret = set_enc_param(test.enc_path, test.enc_type, test.sample_rate, test.mode);
    if (ret != HD_OK) {
        LOG_ERR("set_enc_param failed: %d", ret);
        goto exit;
    }

    // Bind capture to encoder
    LOG_INFO("Binding audiocap to audioenc...");
    ret = hd_audiocap_bind(HD_AUDIOCAP_0_OUT_0, HD_AUDIOENC_0_IN_0);
    if (ret != HD_OK) {
        LOG_ERR("hd_audiocap_bind failed: %d", ret);
        goto exit;
    }

    // Create encode thread
    LOG_INFO("Creating encode thread...");
    ret = pthread_create(&test.enc_thread_id, NULL, encode_thread, (void *)&test);
    if (ret != 0) {
        LOG_ERR("pthread_create failed: %d", ret);
        goto exit;
    }

    // Start encoder first, then capture
    LOG_INFO("Starting encoder...");
    ret = hd_audioenc_start(test.enc_path);
    if (ret != HD_OK) {
        LOG_ERR("hd_audioenc_start failed: %d", ret);
        goto exit;
    }

    LOG_INFO("Starting capture...");
    ret = hd_audiocap_start(test.cap_path);
    if (ret != HD_OK) {
        LOG_ERR("hd_audiocap_start failed: %d", ret);
        goto exit;
    }

    // Signal thread to start
    test.flow_start = 1;

    // Wait for user input or auto-complete
    LOG_INFO("Test running for %d seconds. Press 'q' to stop early...", TEST_DURATION_SEC);
    
    while (test.enc_exit == 0) {
        key = NVT_EXAMSYS_GETCHAR();
        if (key == 'q' || key == 'Q' || key == 0x3) {
            LOG_INFO("User requested stop");
            test.enc_exit = 1;
            break;
        }
        usleep(100000);  // 100ms
    }

    // Wait for thread to finish
    pthread_join(test.enc_thread_id, NULL);

    // Stop capture and encoder
    LOG_INFO("Stopping capture...");
    hd_audiocap_stop(test.cap_path);

    LOG_INFO("Stopping encoder...");
    hd_audioenc_stop(test.enc_path);

    // Unbind
    LOG_INFO("Unbinding...");
    hd_audiocap_unbind(HD_AUDIOCAP_0_OUT_0);

exit:
    // Close modules
    close_module(&test);

    // Exit modules
    exit_module();

    // Exit memory
    mem_exit();

    // Exit HDAL
    hd_common_uninit();

    LOG_INFO("Test complete. Check /mnt/sd/ for output file.");
    return 0;
}
