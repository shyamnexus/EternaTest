/**
 * @brief Test sample for AAC audio capture and encoding using vo-aacenc.
 *
 * This sample tests:
 * - Audio capture at 48kHz stereo/mono using HDAL
 * - Software AAC encoding using vo-aacenc library
 * - Output verification to SD card
 *
 * Usage: hd_audio_aac_test [mode] [bitrate]
 *   mode: 0=mono, 1=stereo (default: 1)
 *   bitrate: AAC bitrate in kbps (default: 64)
 *
 * @file audio_aac_test.c
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

// vo-aacenc headers
#include <vo-aacenc/voAAC.h>
#include <vo-aacenc/cmnMemory.h>

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
#define AAC_FRAME_SAMPLES   1024    // AAC-LC frame size
#define SAMPLE_RATE         48000   // Only 48kHz works on this hardware
#define TEST_DURATION_SEC   10      // Run test for 10 seconds

// ============================================================================
// AAC Encoder context
// ============================================================================
typedef struct {
    VO_AUDIO_CODECAPI   codec_api;
    VO_HANDLE           handle;
    VO_MEM_OPERATOR     mem_operator;
    AACENC_PARAM        params;
    unsigned char*      output_buffer;
    int                 output_buffer_size;
    int                 initialized;
} AAC_ENCODER;

// ============================================================================
// Test context
// ============================================================================
typedef struct {
    // Audio capture
    HD_PATH_ID      cap_ctrl;
    HD_PATH_ID      cap_path;
    
    // Audio encoder (HDAL - PCM passthrough)
    HD_PATH_ID      enc_path;
    
    // AAC encoder (software)
    AAC_ENCODER     aac_enc;
    
    // Thread control
    pthread_t       enc_thread_id;
    volatile UINT32 enc_exit;
    volatile UINT32 flow_start;
    
    // Audio parameters
    HD_AUDIO_SR         sample_rate;
    HD_AUDIO_SOUND_MODE mode;
    UINT32              frame_sample;
    UINT32              bitrate;  // in bps
    
} AUDIO_AAC_TEST;

// ============================================================================
// Memory callbacks for vo-aacenc
// ============================================================================
static VO_U32 aac_mem_alloc(VO_S32 uID, VO_MEM_INFO* pMemInfo)
{
    (void)uID;
    if (!pMemInfo) return 1;
    pMemInfo->VBuffer = malloc(pMemInfo->Size);
    return pMemInfo->VBuffer ? 0 : 1;
}

static VO_U32 aac_mem_free(VO_S32 uID, VO_PTR pMem)
{
    (void)uID;
    if (pMem) free(pMem);
    return 0;
}

static VO_U32 aac_mem_set(VO_S32 uID, VO_PTR pBuff, VO_U8 uValue, VO_U32 uSize)
{
    (void)uID;
    memset(pBuff, uValue, uSize);
    return 0;
}

static VO_U32 aac_mem_copy(VO_S32 uID, VO_PTR pDest, VO_PTR pSource, VO_U32 uSize)
{
    (void)uID;
    memcpy(pDest, pSource, uSize);
    return 0;
}

static VO_U32 aac_mem_check(VO_S32 uID, VO_PTR pBuffer, VO_U32 uSize)
{
    (void)uID;
    (void)pBuffer;
    (void)uSize;
    return 0;
}

// ============================================================================
// AAC Encoder functions
// ============================================================================
static int aac_encoder_init(AAC_ENCODER* enc, int sample_rate, int channels, int bitrate)
{
    VO_CODEC_INIT_USERDATA user_data;
    
    memset(enc, 0, sizeof(AAC_ENCODER));
    
    // Setup memory operator
    enc->mem_operator.Alloc = aac_mem_alloc;
    enc->mem_operator.Free = aac_mem_free;
    enc->mem_operator.Set = aac_mem_set;
    enc->mem_operator.Copy = aac_mem_copy;
    enc->mem_operator.Check = aac_mem_check;
    
    user_data.memflag = VO_IMF_USERMEMOPERATOR;
    user_data.memData = &enc->mem_operator;
    
    // Get AAC encoder API
    if (voGetAACEncAPI(&enc->codec_api) != VO_ERR_NONE) {
        LOG_ERR("Failed to get AAC encoder API");
        return -1;
    }
    
    // Initialize encoder
    if (enc->codec_api.Init(&enc->handle, VO_AUDIO_CodingAAC, &user_data) != VO_ERR_NONE) {
        LOG_ERR("Failed to initialize AAC encoder");
        return -1;
    }
    
    // Set encoder parameters
    enc->params.sampleRate = sample_rate;
    enc->params.bitRate = bitrate;
    enc->params.nChannels = channels;
    enc->params.adtsUsed = 1;  // Use ADTS header for streaming
    
    if (enc->codec_api.SetParam(enc->handle, VO_PID_AAC_ENCPARAM, &enc->params) != VO_ERR_NONE) {
        LOG_ERR("Failed to set AAC encoder parameters");
        enc->codec_api.Uninit(enc->handle);
        return -1;
    }
    
    // Allocate output buffer (max AAC frame is ~768 bytes per channel)
    enc->output_buffer_size = 2048;
    enc->output_buffer = (unsigned char*)malloc(enc->output_buffer_size);
    if (!enc->output_buffer) {
        LOG_ERR("Failed to allocate AAC output buffer");
        enc->codec_api.Uninit(enc->handle);
        return -1;
    }
    
    enc->initialized = 1;
    LOG_INFO("AAC encoder initialized: %dHz, %dch, %d bps", sample_rate, channels, bitrate);
    
    return 0;
}

static int aac_encoder_encode(AAC_ENCODER* enc, const short* pcm_data, int pcm_samples, 
                               unsigned char* output, int* output_size)
{
    VO_CODECBUFFER input_data;
    VO_CODECBUFFER output_data;
    VO_AUDIO_OUTPUTINFO output_info;
    
    if (!enc->initialized) {
        return -1;
    }
    
    // Setup input buffer
    input_data.Buffer = (VO_PBYTE)pcm_data;
    input_data.Length = pcm_samples * enc->params.nChannels * sizeof(short);
    
    // Setup output buffer
    output_data.Buffer = output;
    output_data.Length = *output_size;
    
    // Set input data
    if (enc->codec_api.SetInputData(enc->handle, &input_data) != VO_ERR_NONE) {
        return -1;
    }
    
    // Encode
    VO_U32 ret = enc->codec_api.GetOutputData(enc->handle, &output_data, &output_info);
    if (ret != VO_ERR_NONE) {
        if (ret == VO_ERR_INPUT_BUFFER_SMALL) {
            // Need more input data
            *output_size = 0;
            return 0;
        }
        return -1;
    }
    
    *output_size = output_data.Length;
    return 0;
}

static void aac_encoder_uninit(AAC_ENCODER* enc)
{
    if (enc->initialized) {
        enc->codec_api.Uninit(enc->handle);
        if (enc->output_buffer) {
            free(enc->output_buffer);
            enc->output_buffer = NULL;
        }
        enc->initialized = 0;
    }
}

// ============================================================================
// Memory initialization
// ============================================================================
static HD_RESULT mem_init(void)
{
    HD_RESULT ret = HD_OK;
    HD_COMMON_MEM_INIT_CONFIG mem_cfg = {0};

    mem_cfg.pool_info[0].type = HD_COMMON_MEM_COMMON_POOL;
    mem_cfg.pool_info[0].blk_size = 0x10000;  // 64KB blocks for audio
    mem_cfg.pool_info[0].blk_cnt = 4;
    mem_cfg.pool_info[0].ddr_id = DDR_ID0;

    ret = hd_common_mem_init(&mem_cfg);
    return ret;
}

static HD_RESULT mem_exit(void)
{
    return hd_common_mem_uninit();
}

// ============================================================================
// HDAL Audio capture configuration (PCM only - encoder will reject AAC)
// ============================================================================
static HD_RESULT set_cap_cfg(HD_PATH_ID *p_audio_cap_ctrl, 
                              HD_AUDIO_SR sample_rate, 
                              HD_AUDIO_SOUND_MODE mode,
                              UINT32 frame_sample)
{
    HD_RESULT ret = HD_OK;
    HD_PATH_ID audio_cap_ctrl = 0;
    HD_AUDIOCAP_DEV_CONFIG audio_dev_cfg = {0};
    HD_AUDIOCAP_DRV_CONFIG audio_drv_cfg = {0};

    ret = hd_audiocap_open(0, HD_AUDIOCAP_0_CTRL, &audio_cap_ctrl);
    if (ret != HD_OK) {
        LOG_ERR("hd_audiocap_open(CTRL) failed: %d", ret);
        return ret;
    }

    audio_dev_cfg.in_max.sample_rate = sample_rate;
    audio_dev_cfg.in_max.sample_bit = HD_AUDIO_BIT_WIDTH_16;
    audio_dev_cfg.in_max.mode = mode;
    audio_dev_cfg.in_max.frame_sample = frame_sample;
    audio_dev_cfg.frame_num_max = 10;
    audio_dev_cfg.out_max.sample_rate = 0;

    ret = hd_audiocap_set(audio_cap_ctrl, HD_AUDIOCAP_PARAM_DEV_CONFIG, &audio_dev_cfg);
    if (ret != HD_OK) {
        LOG_ERR("hd_audiocap_set(DEV_CONFIG) failed: %d", ret);
        return ret;
    }

    audio_drv_cfg.mono = HD_AUDIO_MONO_RIGHT;
    ret = hd_audiocap_set(audio_cap_ctrl, HD_AUDIOCAP_PARAM_DRV_CONFIG, &audio_drv_cfg);
    if (ret != HD_OK) {
        LOG_WARN("hd_audiocap_set(DRV_CONFIG) failed: %d", ret);
    }

    *p_audio_cap_ctrl = audio_cap_ctrl;
    return HD_OK;
}

static HD_RESULT set_cap_param(HD_PATH_ID audio_cap_path,
                                HD_AUDIO_SR sample_rate,
                                HD_AUDIO_SOUND_MODE mode,
                                UINT32 frame_sample)
{
    HD_RESULT ret = HD_OK;
    HD_AUDIOCAP_IN audio_cap_param = {0};

    audio_cap_param.sample_rate = sample_rate;
    audio_cap_param.sample_bit = HD_AUDIO_BIT_WIDTH_16;
    audio_cap_param.mode = mode;
    audio_cap_param.frame_sample = frame_sample;
    
    ret = hd_audiocap_set(audio_cap_path, HD_AUDIOCAP_PARAM_IN, &audio_cap_param);
    if (ret != HD_OK) {
        LOG_ERR("hd_audiocap_set(IN) failed: %d", ret);
    }
    return ret;
}

// ============================================================================
// HDAL Audio encoder configuration (PCM passthrough only)
// ============================================================================
static HD_RESULT set_enc_cfg(HD_PATH_ID audio_enc_path, 
                              HD_AUDIO_SR sample_rate,
                              HD_AUDIO_SOUND_MODE mode)
{
    HD_RESULT ret = HD_OK;
    HD_AUDIOENC_PATH_CONFIG audio_path_cfg = {0};

    // PCM passthrough only - hardware doesn't support AAC
    audio_path_cfg.max_mem.codec_type = HD_AUDIO_CODEC_PCM;
    audio_path_cfg.max_mem.sample_rate = sample_rate;
    audio_path_cfg.max_mem.sample_bit = HD_AUDIO_BIT_WIDTH_16;
    audio_path_cfg.max_mem.mode = mode;
    
    ret = hd_audioenc_set(audio_enc_path, HD_AUDIOENC_PARAM_PATH_CONFIG, &audio_path_cfg);
    if (ret != HD_OK) {
        LOG_ERR("hd_audioenc_set(PATH_CONFIG) failed: %d", ret);
    }
    return ret;
}

static HD_RESULT set_enc_param(HD_PATH_ID audio_enc_path, 
                                HD_AUDIO_SR sample_rate,
                                HD_AUDIO_SOUND_MODE mode)
{
    HD_RESULT ret = HD_OK;
    HD_AUDIOENC_IN audio_enc_in = {0};
    HD_AUDIOENC_OUT audio_enc_out = {0};

    audio_enc_in.sample_rate = sample_rate;
    audio_enc_in.sample_bit = HD_AUDIO_BIT_WIDTH_16;
    audio_enc_in.mode = mode;
    
    ret = hd_audioenc_set(audio_enc_path, HD_AUDIOENC_PARAM_IN, &audio_enc_in);
    if (ret != HD_OK) {
        LOG_ERR("hd_audioenc_set(IN) failed: %d", ret);
        return ret;
    }

    // PCM passthrough
    audio_enc_out.codec_type = HD_AUDIO_CODEC_PCM;
    ret = hd_audioenc_set(audio_enc_path, HD_AUDIOENC_PARAM_OUT, &audio_enc_out);
    if (ret != HD_OK) {
        LOG_ERR("hd_audioenc_set(OUT) failed: %d", ret);
    }
    return ret;
}

// ============================================================================
// Module initialization
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
    
    return HD_OK;
}

static HD_RESULT open_module(AUDIO_AAC_TEST *p_test)
{
    HD_RESULT ret;

    ret = hd_audiocap_open(HD_AUDIOCAP_0_IN_0, HD_AUDIOCAP_0_OUT_0, &p_test->cap_path);
    if (ret != HD_OK) {
        LOG_ERR("hd_audiocap_open(path) failed: %d", ret);
        return ret;
    }

    ret = hd_audioenc_open(HD_AUDIOENC_0_IN_0, HD_AUDIOENC_0_OUT_0, &p_test->enc_path);
    if (ret != HD_OK) {
        LOG_ERR("hd_audioenc_open failed: %d", ret);
        return ret;
    }

    return HD_OK;
}

static HD_RESULT close_module(AUDIO_AAC_TEST *p_test)
{
    if (p_test->cap_path) {
        hd_audiocap_close(p_test->cap_path);
    }
    if (p_test->cap_ctrl) {
        hd_audiocap_close(p_test->cap_ctrl);
    }
    if (p_test->enc_path) {
        hd_audioenc_close(p_test->enc_path);
    }
    return HD_OK;
}

static void exit_module(void)
{
    hd_audioenc_uninit();
    hd_audiocap_uninit();
}

// ============================================================================
// Encode thread - captures PCM and encodes to AAC
// ============================================================================
static void *encode_thread(void *arg)
{
    AUDIO_AAC_TEST *p_test = (AUDIO_AAC_TEST *)arg;
    HD_RESULT ret;
    HD_AUDIO_BS data_pull;
    
    FILE *fp = NULL;
    char filename[128];
    time_t start_time;
    UINT32 total_frames = 0;
    UINT32 total_aac_bytes = 0;
    int channels = (p_test->mode == HD_AUDIO_SOUND_MODE_STEREO) ? 2 : 1;
    
    // AAC output buffer
    unsigned char aac_buffer[2048];
    int aac_size;
    
    // PCM accumulation buffer for AAC (1024 samples per frame)
    int pcm_buffer_samples = 0;
    int pcm_buffer_size = AAC_FRAME_SAMPLES * channels * sizeof(short);
    short *pcm_buffer = (short *)malloc(pcm_buffer_size);
    
    if (!pcm_buffer) {
        LOG_ERR("Failed to allocate PCM buffer");
        return NULL;
    }

    // Wait for start signal
    while (p_test->flow_start == 0) {
        usleep(10000);
    }

    // Get encoder buffer info
    HD_AUDIOENC_BUFINFO buf_info = {0};
    ret = hd_audioenc_get(p_test->enc_path, HD_AUDIOENC_PARAM_BUFINFO, &buf_info);
    if (ret == HD_OK) {
        LOG_INFO("Encoder buffer: phy=0x%lx, size=%u", 
                 (unsigned long)buf_info.buf_info.phy_addr, buf_info.buf_info.buf_size);
    }

    // Open output file
    snprintf(filename, sizeof(filename), "/mnt/sd/audio_test_%dHz_%s_%dkbps.aac",
             (int)p_test->sample_rate,
             (p_test->mode == HD_AUDIO_SOUND_MODE_STEREO) ? "stereo" : "mono",
             p_test->bitrate / 1000);
    
    fp = fopen(filename, "wb");
    if (!fp) {
        LOG_ERR("Failed to open output file: %s", filename);
        free(pcm_buffer);
        return NULL;
    }
    
    LOG_INFO("Writing AAC audio to: %s", filename);
    LOG_INFO("=================================================");
    LOG_INFO("Starting audio capture/AAC encode test...");
    LOG_INFO("  Sample Rate: %d Hz", (int)p_test->sample_rate);
    LOG_INFO("  Mode: %s", (p_test->mode == HD_AUDIO_SOUND_MODE_STEREO) ? "stereo" : "mono");
    LOG_INFO("  Bitrate: %d kbps", p_test->bitrate / 1000);
    LOG_INFO("  Frame Samples: %d", p_test->frame_sample);
    LOG_INFO("=================================================");

    start_time = time(NULL);

    while (p_test->enc_exit == 0) {
        // Check duration
        if ((time(NULL) - start_time) >= TEST_DURATION_SEC) {
            LOG_INFO("Test duration (%d seconds) reached, stopping...", TEST_DURATION_SEC);
            p_test->enc_exit = 1;
            break;
        }

        // Pull PCM data from HDAL encoder
        memset(&data_pull, 0, sizeof(data_pull));
        ret = hd_audioenc_pull_out_buf(p_test->enc_path, &data_pull, 500);
        
        if (ret != HD_OK) {
            if (ret == HD_ERR_TIMEDOUT || ret == -12) {
                LOG_WARN("Pull timeout (no data for 500ms)");
                continue;
            }
            LOG_ERR("hd_audioenc_pull_out_buf failed: %d", ret);
            break;
        }

        if (data_pull.size > 0 && data_pull.phy_addr != 0) {
            // Map physical address to virtual address
            void *virt_addr = hd_common_mem_mmap(HD_COMMON_MEM_MEM_TYPE_CACHE, 
                                                  data_pull.phy_addr, data_pull.size);
            if (virt_addr == NULL) {
                LOG_ERR("Failed to map physical address 0x%lx", (unsigned long)data_pull.phy_addr);
                hd_audioenc_release_out_buf(p_test->enc_path, &data_pull);
                continue;
            }
            
            // Get PCM data
            short *pcm_data = (short *)virt_addr;
            int pcm_samples = data_pull.size / (channels * sizeof(short));
            int samples_processed = 0;
            
            while (samples_processed < pcm_samples) {
                // Copy samples to accumulation buffer
                int samples_to_copy = pcm_samples - samples_processed;
                int samples_needed = AAC_FRAME_SAMPLES - pcm_buffer_samples;
                
                if (samples_to_copy > samples_needed) {
                    samples_to_copy = samples_needed;
                }
                
                memcpy(pcm_buffer + (pcm_buffer_samples * channels),
                       pcm_data + (samples_processed * channels),
                       samples_to_copy * channels * sizeof(short));
                
                pcm_buffer_samples += samples_to_copy;
                samples_processed += samples_to_copy;
                
                // Encode when we have enough samples
                if (pcm_buffer_samples >= AAC_FRAME_SAMPLES) {
                    aac_size = sizeof(aac_buffer);
                    
                    if (aac_encoder_encode(&p_test->aac_enc, pcm_buffer, AAC_FRAME_SAMPLES,
                                           aac_buffer, &aac_size) == 0 && aac_size > 0) {
                        fwrite(aac_buffer, 1, aac_size, fp);
                        total_aac_bytes += aac_size;
                    }
                    
                    pcm_buffer_samples = 0;
                }
            }
            
            total_frames++;
            
            // Unmap virtual address
            hd_common_mem_munmap((void*)virt_addr, data_pull.size);
            
            // Release buffer
            ret = hd_audioenc_release_out_buf(p_test->enc_path, &data_pull);
            if (ret != HD_OK) {
                LOG_ERR("hd_audioenc_release_out_buf failed: %d", ret);
            }
        }

        // Progress report
        if (total_frames % 50 == 0) {
            LOG_INFO("Captured %u frames, encoded %u AAC bytes", total_frames, total_aac_bytes);
        }
    }

    LOG_INFO("=================================================");
    LOG_INFO("Test completed!");
    LOG_INFO("  Total PCM frames: %u", total_frames);
    LOG_INFO("  Total AAC bytes: %u", total_aac_bytes);
    LOG_INFO("  Output file: %s", filename);
    LOG_INFO("=================================================");

    if (fp) {
        fclose(fp);
    }
    free(pcm_buffer);

    return NULL;
}

// ============================================================================
// Main entry point
// ============================================================================
EXAMFUNC_ENTRY(hd_audio_aac_test, argc, argv)
{
    HD_RESULT ret;
    AUDIO_AAC_TEST test = {0};
    INT key;
    int channels;

    LOG_INFO("=================================================");
    LOG_INFO("HDAL Audio AAC Test Sample (using vo-aacenc)");
    LOG_INFO("=================================================");

    // Default configuration: 48kHz stereo, 64kbps
    test.sample_rate = HD_AUDIO_SR_48000;
    test.mode = HD_AUDIO_SOUND_MODE_STEREO;
    test.frame_sample = AAC_FRAME_SAMPLES;
    test.bitrate = 64000;  // 64 kbps

    // Parse arguments
    if (argc >= 2) {
        int mode = atoi(argv[1]);
        if (mode == 0) {
            test.mode = HD_AUDIO_SOUND_MODE_MONO;
        } else {
            test.mode = HD_AUDIO_SOUND_MODE_STEREO;
        }
    }

    if (argc >= 3) {
        test.bitrate = atoi(argv[2]) * 1000;  // Convert kbps to bps
        if (test.bitrate < 16000) test.bitrate = 16000;
        if (test.bitrate > 320000) test.bitrate = 320000;
    }

    channels = (test.mode == HD_AUDIO_SOUND_MODE_STEREO) ? 2 : 1;

    LOG_INFO("Test configuration:");
    LOG_INFO("  Sample Rate: 48000 Hz (fixed - only rate that works)");
    LOG_INFO("  Mode: %s", (test.mode == HD_AUDIO_SOUND_MODE_STEREO) ? "Stereo" : "Mono");
    LOG_INFO("  Bitrate: %d kbps", test.bitrate / 1000);
    LOG_INFO("  Frame Samples: %d", test.frame_sample);

    // Initialize AAC encoder first
    LOG_INFO("Initializing vo-aacenc...");
    if (aac_encoder_init(&test.aac_enc, SAMPLE_RATE, channels, test.bitrate) != 0) {
        LOG_ERR("Failed to initialize AAC encoder");
        return -1;
    }

    // Initialize HDAL
    LOG_INFO("Initializing HDAL...");
    ret = hd_common_init(0);
    if (ret != HD_OK) {
        LOG_ERR("hd_common_init failed: %d", ret);
        goto exit;
    }

    ret = mem_init();
    if (ret != HD_OK) {
        LOG_ERR("mem_init failed: %d", ret);
        goto exit;
    }

    ret = init_module();
    if (ret != HD_OK) {
        LOG_ERR("init_module failed: %d", ret);
        goto exit;
    }

    // Configure capture
    ret = set_cap_cfg(&test.cap_ctrl, test.sample_rate, test.mode, test.frame_sample);
    if (ret != HD_OK) {
        LOG_ERR("set_cap_cfg failed: %d", ret);
        goto exit;
    }

    ret = open_module(&test);
    if (ret != HD_OK) {
        LOG_ERR("open_module failed: %d", ret);
        goto exit;
    }

    ret = set_cap_param(test.cap_path, test.sample_rate, test.mode, test.frame_sample);
    if (ret != HD_OK) {
        LOG_ERR("set_cap_param failed: %d", ret);
        goto exit;
    }

    ret = set_enc_cfg(test.enc_path, test.sample_rate, test.mode);
    if (ret != HD_OK) {
        LOG_ERR("set_enc_cfg failed: %d", ret);
        goto exit;
    }

    ret = set_enc_param(test.enc_path, test.sample_rate, test.mode);
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

    // Start encoder and capture
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

    LOG_INFO("Test running for %d seconds. Press 'q' to stop early...", TEST_DURATION_SEC);
    
    while (test.enc_exit == 0) {
        key = NVT_EXAMSYS_GETCHAR();
        if (key == 'q' || key == 'Q' || key == 0x3) {
            LOG_INFO("User requested stop");
            test.enc_exit = 1;
            break;
        }
        usleep(100000);
    }

    // Wait for thread to finish
    pthread_join(test.enc_thread_id, NULL);

    // Stop
    LOG_INFO("Stopping capture...");
    hd_audiocap_stop(test.cap_path);

    LOG_INFO("Stopping encoder...");
    hd_audioenc_stop(test.enc_path);

    LOG_INFO("Unbinding...");
    hd_audiocap_unbind(HD_AUDIOCAP_0_OUT_0);

exit:
    // Cleanup
    aac_encoder_uninit(&test.aac_enc);
    close_module(&test);
    exit_module();
    mem_exit();
    hd_common_uninit();

    LOG_INFO("Test complete. Check /mnt/sd/ for AAC output file.");
    return 0;
}
