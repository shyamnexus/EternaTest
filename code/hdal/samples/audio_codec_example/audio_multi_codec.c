/**
 * @file audio_multi_codec.c
 * @brief Comprehensive audio recording example with multiple codec support
 * 
 * Demonstrates recording audio with different codecs:
 * - AAC-LC (Advanced Audio Coding)
 * - G.711 μ-law (PCMU)
 * - G.711 A-law (PCMA)
 * - PCM (Linear PCM s16le)
 * 
 * Features:
 * - Interactive codec selection
 * - Configurable sample rate, bit width, channels
 * - Simultaneous multi-codec recording
 * - File output with codec identification
 * - Real-time statistics
 * 
 * Usage:
 *   ./audio_multi_codec [codec] [sample_rate] [channels]
 *   
 *   codec: 0=AAC, 1=G.711u, 2=G.711a, 3=PCM, 4=All (default: 4)
 *   sample_rate: 8000, 16000, 32000, 48000 (default: 48000)
 *   channels: 1=mono, 2=stereo (default: 2)
 * 
 * Examples:
 *   ./audio_multi_codec                    # Record all codecs, 48kHz stereo
 *   ./audio_multi_codec 0 48000 2          # AAC only, 48kHz stereo
 *   ./audio_multi_codec 3 16000 1          # PCM only, 16kHz mono
 * 
 * Output files: /mnt/sd/audio_[codec]_[sr]_[ch].dat
 * 
 * Copyright Novatek Microelectronics Corp. 2025. All rights reserved.
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "hdal.h"
#include "hd_debug.h"

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
// Configuration
// ============================================================================

#define MAX_CODECS              4
#define DEBUG_MENU              1
#define OUTPUT_DIR              "/mnt/sd"

// ANSI color codes for terminal output
#define COLOR_RESET   "\033[0m"
#define COLOR_RED     "\033[0;31m"
#define COLOR_GREEN   "\033[0;32m"
#define COLOR_YELLOW  "\033[0;33m"
#define COLOR_BLUE    "\033[0;34m"
#define COLOR_MAGENTA "\033[0;35m"
#define COLOR_CYAN    "\033[0;36m"

// Debug macros
#define INFO(fmt, ...)    printf(COLOR_GREEN "[INFO] " COLOR_RESET fmt "\n", ##__VA_ARGS__)
#define WARN(fmt, ...)    printf(COLOR_YELLOW "[WARN] " COLOR_RESET fmt "\n", ##__VA_ARGS__)
#define ERROR(fmt, ...)   printf(COLOR_RED "[ERROR] " COLOR_RESET fmt "\n", ##__VA_ARGS__)
#define DEBUG(fmt, ...)   printf(COLOR_CYAN "[DEBUG] " COLOR_RESET fmt "\n", ##__VA_ARGS__)

// ============================================================================
// Data Structures
// ============================================================================

typedef struct {
    UINT32 codec_type;          // HD_AUDIO_CODEC_*
    const char *codec_name;     // Human-readable name
    const char *file_extension; // File extension
    BOOL enabled;               // Whether this codec is enabled
    
    // Statistics
    UINT64 total_bytes;         // Total bytes written
    UINT64 total_frames;        // Total frames encoded
    UINT32 min_frame_size;      // Minimum frame size
    UINT32 max_frame_size;      // Maximum frame size
    UINT32 start_time_ms;       // Recording start time
    
    // File handles
    FILE *fp_data;              // Output data file
    FILE *fp_info;              // Output info file (sizes, timestamps)
} CODEC_INFO;

typedef struct {
    // HDAL paths
    HD_PATH_ID cap_ctrl;
    HD_PATH_ID cap_path;
    HD_PATH_ID enc_path[MAX_CODECS];
    
    // Configuration
    HD_AUDIO_SR sample_rate;
    HD_AUDIO_BIT_WIDTH bit_width;
    HD_AUDIO_SOUND_MODE sound_mode;
    UINT32 frame_samples;
    
    // Codec information
    CODEC_INFO codecs[MAX_CODECS];
    UINT32 active_codec_count;
    
    // Threading
    pthread_t thread_id[MAX_CODECS];
    UINT32 exit_flag;
    UINT32 flow_start;
    
} AUDIO_RECORDER;

// ============================================================================
// Codec Configuration
// ============================================================================

static void init_codec_info(AUDIO_RECORDER *recorder) {
    // Codec 0: AAC-LC
    recorder->codecs[0].codec_type = HD_AUDIO_CODEC_AAC;
    recorder->codecs[0].codec_name = "AAC-LC";
    recorder->codecs[0].file_extension = "aac";
    
    // Codec 1: G.711 μ-law
    recorder->codecs[1].codec_type = HD_AUDIO_CODEC_ULAW;
    recorder->codecs[1].codec_name = "G.711u";
    recorder->codecs[1].file_extension = "g711u";
    
    // Codec 2: G.711 A-law
    recorder->codecs[2].codec_type = HD_AUDIO_CODEC_ALAW;
    recorder->codecs[2].codec_name = "G.711a";
    recorder->codecs[2].file_extension = "g711a";
    
    // Codec 3: PCM
    recorder->codecs[3].codec_type = HD_AUDIO_CODEC_PCM;
    recorder->codecs[3].codec_name = "PCM";
    recorder->codecs[3].file_extension = "pcm";
    
    // Initialize statistics
    for (int i = 0; i < MAX_CODECS; i++) {
        recorder->codecs[i].enabled = FALSE;
        recorder->codecs[i].total_bytes = 0;
        recorder->codecs[i].total_frames = 0;
        recorder->codecs[i].min_frame_size = UINT32_MAX;
        recorder->codecs[i].max_frame_size = 0;
        recorder->codecs[i].fp_data = NULL;
        recorder->codecs[i].fp_info = NULL;
    }
}

static const char* sample_rate_to_string(HD_AUDIO_SR sr) {
    switch (sr) {
        case HD_AUDIO_SR_8000:  return "8000";
        case HD_AUDIO_SR_16000: return "16000";
        case HD_AUDIO_SR_32000: return "32000";
        case HD_AUDIO_SR_48000: return "48000";
        default: return "unknown";
    }
}

static const char* channels_to_string(HD_AUDIO_SOUND_MODE mode) {
    return (mode == HD_AUDIO_SOUND_MODE_MONO) ? "mono" : "stereo";
}

// ============================================================================
// Memory Management
// ============================================================================

static HD_RESULT mem_init(void) {
    HD_COMMON_MEM_INIT_CONFIG mem_cfg = {0};
    
    // Config common pool
    mem_cfg.pool_info[0].type = HD_COMMON_MEM_COMMON_POOL;
    mem_cfg.pool_info[0].blk_size = 0x2000;  // 8KB blocks
    mem_cfg.pool_info[0].blk_cnt = 8;        // 8 blocks for multi-codec
    mem_cfg.pool_info[0].ddr_id = DDR_ID0;
    
    return hd_common_mem_init(&mem_cfg);
}

static HD_RESULT mem_exit(void) {
    return hd_common_mem_uninit();
}

// ============================================================================
// Audio Capture Configuration
// ============================================================================

static HD_RESULT set_cap_cfg(AUDIO_RECORDER *recorder) {
    HD_RESULT ret;
    HD_AUDIOCAP_DEV_CONFIG dev_cfg = {0};
    HD_AUDIOCAP_DRV_CONFIG drv_cfg = {0};
    
    ret = hd_audiocap_open(0, HD_AUDIOCAP_0_CTRL, &recorder->cap_ctrl);
    if (ret != HD_OK) {
        ERROR("Failed to open audiocap control: %d", ret);
        return ret;
    }
    
    // Set device config (max capabilities)
    dev_cfg.in_max.sample_rate = recorder->sample_rate;
    dev_cfg.in_max.sample_bit = recorder->bit_width;
    dev_cfg.in_max.mode = recorder->sound_mode;
    dev_cfg.in_max.frame_sample = recorder->frame_samples;
    dev_cfg.frame_num_max = 10;
    
    ret = hd_audiocap_set(recorder->cap_ctrl, HD_AUDIOCAP_PARAM_DEV_CONFIG, &dev_cfg);
    if (ret != HD_OK) {
        ERROR("Failed to set audiocap dev config: %d", ret);
        return ret;
    }
    
    // Set driver config
    drv_cfg.mono = HD_AUDIO_MONO_RIGHT;
    ret = hd_audiocap_set(recorder->cap_ctrl, HD_AUDIOCAP_PARAM_DRV_CONFIG, &drv_cfg);
    if (ret != HD_OK) {
        ERROR("Failed to set audiocap drv config: %d", ret);
        return ret;
    }
    
    INFO("Audio capture configured: %s Hz, %d-bit, %s",
         sample_rate_to_string(recorder->sample_rate),
         (recorder->bit_width == HD_AUDIO_BIT_WIDTH_16) ? 16 : 8,
         channels_to_string(recorder->sound_mode));
    
    return HD_OK;
}

static HD_RESULT set_cap_param(AUDIO_RECORDER *recorder) {
    HD_AUDIOCAP_IN cap_param = {0};
    
    cap_param.sample_rate = recorder->sample_rate;
    cap_param.sample_bit = recorder->bit_width;
    cap_param.mode = recorder->sound_mode;
    cap_param.frame_sample = recorder->frame_samples;
    
    return hd_audiocap_set(recorder->cap_path, HD_AUDIOCAP_PARAM_IN, &cap_param);
}

// ============================================================================
// Audio Encoder Configuration
// ============================================================================

static HD_RESULT set_enc_cfg(HD_PATH_ID enc_path, AUDIO_RECORDER *recorder, UINT32 codec_type) {
    HD_AUDIOENC_PATH_CONFIG path_cfg = {0};
    
    path_cfg.max_mem.codec_type = codec_type;
    path_cfg.max_mem.sample_rate = recorder->sample_rate;
    path_cfg.max_mem.sample_bit = recorder->bit_width;
    path_cfg.max_mem.mode = recorder->sound_mode;
    
    return hd_audioenc_set(enc_path, HD_AUDIOENC_PARAM_PATH_CONFIG, &path_cfg);
}

static HD_RESULT set_enc_param(HD_PATH_ID enc_path, AUDIO_RECORDER *recorder, UINT32 codec_type) {
    HD_RESULT ret;
    HD_AUDIOENC_IN in_param = {0};
    HD_AUDIOENC_OUT out_param = {0};
    
    // Input parameters
    in_param.sample_rate = recorder->sample_rate;
    in_param.sample_bit = recorder->bit_width;
    in_param.mode = recorder->sound_mode;
    
    ret = hd_audioenc_set(enc_path, HD_AUDIOENC_PARAM_IN, &in_param);
    if (ret != HD_OK) {
        ERROR("Failed to set encoder input params: %d", ret);
        return ret;
    }
    
    // Output parameters
    out_param.codec_type = codec_type;
    out_param.aac_adts = (codec_type == HD_AUDIO_CODEC_AAC) ? TRUE : FALSE;
    
    ret = hd_audioenc_set(enc_path, HD_AUDIOENC_PARAM_OUT, &out_param);
    if (ret != HD_OK) {
        ERROR("Failed to set encoder output params: %d", ret);
        return ret;
    }
    
    return HD_OK;
}

// ============================================================================
// File Management
// ============================================================================

static HD_RESULT open_output_files(CODEC_INFO *codec, AUDIO_RECORDER *recorder) {
    char data_path[256];
    char info_path[256];
    
    // Generate filenames
    snprintf(data_path, sizeof(data_path), 
             "%s/audio_%s_%s_%s.%s",
             OUTPUT_DIR,
             codec->codec_name,
             sample_rate_to_string(recorder->sample_rate),
             channels_to_string(recorder->sound_mode),
             codec->file_extension);
    
    snprintf(info_path, sizeof(info_path),
             "%s/audio_%s_%s_%s.info",
             OUTPUT_DIR,
             codec->codec_name,
             sample_rate_to_string(recorder->sample_rate),
             channels_to_string(recorder->sound_mode));
    
    // Open data file
    codec->fp_data = fopen(data_path, "wb");
    if (!codec->fp_data) {
        ERROR("Failed to open data file: %s", data_path);
        return HD_ERR_NG;
    }
    
    // Open info file
    codec->fp_info = fopen(info_path, "w");
    if (!codec->fp_info) {
        ERROR("Failed to open info file: %s", info_path);
        fclose(codec->fp_data);
        codec->fp_data = NULL;
        return HD_ERR_NG;
    }
    
    // Write header to info file
    fprintf(codec->fp_info, "# Audio Recording Info\n");
    fprintf(codec->fp_info, "# Codec: %s\n", codec->codec_name);
    fprintf(codec->fp_info, "# Sample Rate: %s Hz\n", sample_rate_to_string(recorder->sample_rate));
    fprintf(codec->fp_info, "# Channels: %s\n", channels_to_string(recorder->sound_mode));
    fprintf(codec->fp_info, "# Format: frame_num size_bytes timestamp_ms\n");
    fprintf(codec->fp_info, "#\n");
    fflush(codec->fp_info);
    
    INFO("Recording %s to: %s", codec->codec_name, data_path);
    
    return HD_OK;
}

static void close_output_files(CODEC_INFO *codec) {
    if (codec->fp_data) {
        fclose(codec->fp_data);
        codec->fp_data = NULL;
    }
    if (codec->fp_info) {
        fclose(codec->fp_info);
        codec->fp_info = NULL;
    }
}

// ============================================================================
// Encoding Thread (Per Codec)
// ============================================================================

static void *encode_thread(void *arg) {
    typedef struct {
        AUDIO_RECORDER *recorder;
        int codec_index;
    } THREAD_ARG;
    
    THREAD_ARG *thread_arg = (THREAD_ARG *)arg;
    AUDIO_RECORDER *recorder = thread_arg->recorder;
    int codec_idx = thread_arg->codec_index;
    CODEC_INFO *codec = &recorder->codecs[codec_idx];
    HD_PATH_ID enc_path = recorder->enc_path[codec_idx];
    
    HD_RESULT ret;
    HD_AUDIO_BS data_pull;
    UINTPTR vir_addr;
    HD_AUDIOENC_BUFINFO phy_buf;
    
    DEBUG("Encode thread started for %s (index %d)", codec->codec_name, codec_idx);
    
    // Wait for flow start
    while (recorder->flow_start == 0) {
        usleep(100000); // 100ms
    }
    
    // Open output files
    if (open_output_files(codec, recorder) != HD_OK) {
        ERROR("Failed to open output files for %s", codec->codec_name);
        free(thread_arg);
        return NULL;
    }
    
    // Query physical address of bitstream buffer
    ret = hd_audioenc_get(enc_path, HD_AUDIOENC_PARAM_BUFINFO, &phy_buf);
    if (ret != HD_OK) {
        ERROR("Failed to get buffer info for %s: %d", codec->codec_name, ret);
        close_output_files(codec);
        free(thread_arg);
        return NULL;
    }
    
    // Memory map for bitstream buffer
    vir_addr = (UINTPTR)hd_common_mem_mmap(HD_COMMON_MEM_MEM_TYPE_CACHE,
                                            phy_buf.buf_info.phy_addr,
                                            phy_buf.buf_info.buf_size);
    if (!vir_addr) {
        ERROR("Failed to mmap buffer for %s", codec->codec_name);
        close_output_files(codec);
        free(thread_arg);
        return NULL;
    }
    
    #define PHY2VIRT(pa) (vir_addr + (pa - phy_buf.buf_info.phy_addr))
    
    codec->start_time_ms = hd_gettime_ms();
    INFO("%s encoding started", codec->codec_name);
    
    // Main encoding loop
    while (recorder->exit_flag == 0) {
        // Pull encoded data (blocking)
        ret = hd_audioenc_pull_out_buf(enc_path, &data_pull, 100); // 100ms timeout
        
        if (ret == HD_OK) {
            UINT8 *ptr = (UINT8 *)PHY2VIRT(data_pull.phy_addr);
            UINT32 size = data_pull.size;
            UINT32 timestamp = hd_gettime_ms() - codec->start_time_ms;
            
            // Write bitstream data
            if (codec->fp_data) {
                fwrite(ptr, 1, size, codec->fp_data);
                fflush(codec->fp_data);
            }
            
            // Write frame info
            if (codec->fp_info) {
                fprintf(codec->fp_info, "%llu %u %u\n", 
                        codec->total_frames, size, timestamp);
                fflush(codec->fp_info);
            }
            
            // Update statistics
            codec->total_bytes += size;
            codec->total_frames++;
            if (size < codec->min_frame_size) codec->min_frame_size = size;
            if (size > codec->max_frame_size) codec->max_frame_size = size;
            
            // Release buffer
            ret = hd_audioenc_release_out_buf(enc_path, &data_pull);
            if (ret != HD_OK) {
                WARN("Failed to release buffer for %s: %d", codec->codec_name, ret);
            }
            
        } else if (ret != HD_ERR_TIMEDOUT) {
            ERROR("Pull error for %s: %d", codec->codec_name, ret);
            break;
        }
    }
    
    // Cleanup
    hd_common_mem_munmap((void *)vir_addr, phy_buf.buf_info.buf_size);
    close_output_files(codec);
    
    INFO("%s encoding stopped. Frames: %llu, Bytes: %llu", 
         codec->codec_name, codec->total_frames, codec->total_bytes);
    
    free(thread_arg);
    return NULL;
}

// ============================================================================
// Statistics Display
// ============================================================================

static void print_statistics(AUDIO_RECORDER *recorder) {
    printf("\n");
    printf(COLOR_CYAN "═══════════════════════════════════════════════════════════════\n" COLOR_RESET);
    printf(COLOR_CYAN "                    RECORDING STATISTICS                        \n" COLOR_RESET);
    printf(COLOR_CYAN "═══════════════════════════════════════════════════════════════\n" COLOR_RESET);
    
    for (int i = 0; i < MAX_CODECS; i++) {
        CODEC_INFO *codec = &recorder->codecs[i];
        if (!codec->enabled) continue;
        
        UINT32 duration_ms = hd_gettime_ms() - codec->start_time_ms;
        float duration_sec = duration_ms / 1000.0f;
        float avg_bitrate = (codec->total_bytes * 8.0f) / (duration_sec * 1000.0f); // kbps
        float avg_frame_size = codec->total_frames ? 
                               (float)codec->total_bytes / codec->total_frames : 0;
        
        printf("\n" COLOR_GREEN "Codec: %s\n" COLOR_RESET, codec->codec_name);
        printf("  Duration:       %.2f seconds\n", duration_sec);
        printf("  Total Frames:   %llu\n", codec->total_frames);
        printf("  Total Bytes:    %llu (%.2f MB)\n", 
               codec->total_bytes, codec->total_bytes / (1024.0f * 1024.0f));
        printf("  Avg Bitrate:    %.2f kbps\n", avg_bitrate);
        printf("  Avg Frame Size: %.2f bytes\n", avg_frame_size);
        printf("  Min Frame Size: %u bytes\n", codec->min_frame_size);
        printf("  Max Frame Size: %u bytes\n", codec->max_frame_size);
    }
    
    printf("\n");
    printf(COLOR_CYAN "═══════════════════════════════════════════════════════════════\n" COLOR_RESET);
    printf("\n");
}

// ============================================================================
// Module Initialization
// ============================================================================

static HD_RESULT init_modules(void) {
    HD_RESULT ret;
    
    ret = hd_audiocap_init();
    if (ret != HD_OK) {
        ERROR("Failed to init audiocap: %d", ret);
        return ret;
    }
    
    ret = hd_audioenc_init();
    if (ret != HD_OK) {
        ERROR("Failed to init audioenc: %d", ret);
        hd_audiocap_uninit();
        return ret;
    }
    
    return HD_OK;
}

static HD_RESULT exit_modules(void) {
    hd_audioenc_uninit();
    hd_audiocap_uninit();
    return HD_OK;
}

// ============================================================================
// Main Program
// ============================================================================

static void print_usage(const char *prog_name) {
    printf("\n");
    printf(COLOR_CYAN "═══════════════════════════════════════════════════════════════\n" COLOR_RESET);
    printf(COLOR_CYAN "         Multi-Codec Audio Recording Example                   \n" COLOR_RESET);
    printf(COLOR_CYAN "═══════════════════════════════════════════════════════════════\n" COLOR_RESET);
    printf("\n");
    printf("Usage: %s [codec] [sample_rate] [channels]\n\n", prog_name);
    printf("Arguments:\n");
    printf("  codec:       0=AAC, 1=G.711u, 2=G.711a, 3=PCM, 4=All (default: 4)\n");
    printf("  sample_rate: 8000, 16000, 32000, 48000 (default: 48000)\n");
    printf("  channels:    1=mono, 2=stereo (default: 2)\n");
    printf("\n");
    printf("Examples:\n");
    printf("  %s                    # All codecs, 48kHz stereo\n", prog_name);
    printf("  %s 0 48000 2          # AAC only, 48kHz stereo\n", prog_name);
    printf("  %s 3 16000 1          # PCM only, 16kHz mono\n", prog_name);
    printf("  %s 1 8000 1           # G.711u only, 8kHz mono\n", prog_name);
    printf("\n");
    printf("Output: %s/audio_[codec]_[sr]_[ch].[ext]\n", OUTPUT_DIR);
    printf("\n");
    printf(COLOR_CYAN "═══════════════════════════════════════════════════════════════\n" COLOR_RESET);
    printf("\n");
}

int main(int argc, char *argv[]) {
    HD_RESULT ret;
    AUDIO_RECORDER recorder = {0};
    int codec_select = 4; // Default: all codecs
    int sample_rate = 48000;
    int channels = 2;
    
    // Print usage if help requested
    if (argc > 1 && (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0)) {
        print_usage(argv[0]);
        return 0;
    }
    
    // Parse arguments
    if (argc >= 2) codec_select = atoi(argv[1]);
    if (argc >= 3) sample_rate = atoi(argv[2]);
    if (argc >= 4) channels = atoi(argv[3]);
    
    // Validate arguments
    if (codec_select < 0 || codec_select > 4) {
        ERROR("Invalid codec selection: %d (must be 0-4)", codec_select);
        print_usage(argv[0]);
        return 1;
    }
    
    // Set sample rate
    switch (sample_rate) {
        case 8000:  recorder.sample_rate = HD_AUDIO_SR_8000; break;
        case 16000: recorder.sample_rate = HD_AUDIO_SR_16000; break;
        case 32000: recorder.sample_rate = HD_AUDIO_SR_32000; break;
        case 48000: recorder.sample_rate = HD_AUDIO_SR_48000; break;
        default:
            ERROR("Invalid sample rate: %d (must be 8000, 16000, 32000, or 48000)", sample_rate);
            return 1;
    }
    
    // Set channels
    if (channels == 1) {
        recorder.sound_mode = HD_AUDIO_SOUND_MODE_MONO;
    } else if (channels == 2) {
        recorder.sound_mode = HD_AUDIO_SOUND_MODE_STEREO;
    } else {
        ERROR("Invalid channel count: %d (must be 1 or 2)", channels);
        return 1;
    }
    
    recorder.bit_width = HD_AUDIO_BIT_WIDTH_16;
    recorder.frame_samples = 1024;
    
    // Initialize codec info
    init_codec_info(&recorder);
    
    // Enable selected codecs
    if (codec_select == 4) {
        // Enable all codecs
        for (int i = 0; i < MAX_CODECS; i++) {
            recorder.codecs[i].enabled = TRUE;
            recorder.active_codec_count++;
        }
    } else {
        // Enable single codec
        recorder.codecs[codec_select].enabled = TRUE;
        recorder.active_codec_count = 1;
    }
    
    INFO("Starting audio recording:");
    INFO("  Sample Rate: %d Hz", sample_rate);
    INFO("  Channels: %s", (channels == 1) ? "Mono" : "Stereo");
    INFO("  Active Codecs: %d", recorder.active_codec_count);
    for (int i = 0; i < MAX_CODECS; i++) {
        if (recorder.codecs[i].enabled) {
            INFO("    - %s", recorder.codecs[i].codec_name);
        }
    }
    
    // Initialize HDAL
    ret = hd_common_init(0);
    if (ret != HD_OK) {
        ERROR("Failed to init HDAL: %d", ret);
        goto exit;
    }
    
    // Initialize memory
    ret = mem_init();
    if (ret != HD_OK) {
        ERROR("Failed to init memory: %d", ret);
        goto exit_common;
    }
    
    // Initialize modules
    ret = init_modules();
    if (ret != HD_OK) {
        goto exit_mem;
    }
    
    // Configure audio capture
    ret = set_cap_cfg(&recorder);
    if (ret != HD_OK) {
        goto exit_modules;
    }
    
    // Open audio capture path
    ret = hd_audiocap_open(HD_AUDIOCAP_0_IN_0, HD_AUDIOCAP_0_OUT_0, &recorder.cap_path);
    if (ret != HD_OK) {
        ERROR("Failed to open audiocap path: %d", ret);
        goto exit_cap_ctrl;
    }
    
    // Set capture parameters
    ret = set_cap_param(&recorder);
    if (ret != HD_OK) {
        ERROR("Failed to set capture params: %d", ret);
        goto exit_cap_path;
    }
    
    // Open and configure encoders for each enabled codec
    for (int i = 0; i < MAX_CODECS; i++) {
        if (!recorder.codecs[i].enabled) continue;
        
        // Open encoder path
        ret = hd_audioenc_open(HD_AUDIOENC_0_IN_0 + i, HD_AUDIOENC_0_OUT_0 + i, 
                               &recorder.enc_path[i]);
        if (ret != HD_OK) {
            ERROR("Failed to open encoder for %s: %d", recorder.codecs[i].codec_name, ret);
            goto exit_encoders;
        }
        
        // Configure encoder
        ret = set_enc_cfg(recorder.enc_path[i], &recorder, recorder.codecs[i].codec_type);
        if (ret != HD_OK) {
            ERROR("Failed to config encoder for %s: %d", recorder.codecs[i].codec_name, ret);
            goto exit_encoders;
        }
        
        // Set encoder parameters
        ret = set_enc_param(recorder.enc_path[i], &recorder, recorder.codecs[i].codec_type);
        if (ret != HD_OK) {
            ERROR("Failed to set encoder params for %s: %d", 
                  recorder.codecs[i].codec_name, ret);
            goto exit_encoders;
        }
        
        // Bind capture to encoder
        ret = hd_audiocap_bind(HD_AUDIOCAP_0_OUT_0, HD_AUDIOENC_0_IN_0 + i);
        if (ret != HD_OK) {
            ERROR("Failed to bind encoder for %s: %d", recorder.codecs[i].codec_name, ret);
            goto exit_encoders;
        }
    }
    
    // Create encoding threads
    for (int i = 0; i < MAX_CODECS; i++) {
        if (!recorder.codecs[i].enabled) continue;
        
        typedef struct {
            AUDIO_RECORDER *recorder;
            int codec_index;
        } THREAD_ARG;
        
        THREAD_ARG *arg = malloc(sizeof(THREAD_ARG));
        arg->recorder = &recorder;
        arg->codec_index = i;
        
        ret = pthread_create(&recorder.thread_id[i], NULL, encode_thread, arg);
        if (ret != 0) {
            ERROR("Failed to create thread for %s", recorder.codecs[i].codec_name);
            free(arg);
            goto exit_threads;
        }
    }
    
    // Start encoders
    for (int i = 0; i < MAX_CODECS; i++) {
        if (!recorder.codecs[i].enabled) continue;
        hd_audioenc_start(recorder.enc_path[i]);
    }
    
    // Start capture
    hd_audiocap_start(recorder.cap_path);
    
    // Signal threads to start
    recorder.flow_start = 1;
    
    INFO("Recording started. Press 'q' to stop, 's' for statistics");
    
    // Main loop
    while (1) {
        char key = getchar();
        
        if (key == 'q' || key == 'Q' || key == 0x3) {
            INFO("Stopping recording...");
            recorder.exit_flag = 1;
            break;
        }
        
        if (key == 's' || key == 'S') {
            print_statistics(&recorder);
        }
        
        #if (DEBUG_MENU == 1)
        if (key == 'd' || key == 'D') {
            hd_debug_run_menu();
            printf("\nPress 'q' to stop, 's' for statistics\n");
        }
        #endif
    }
    
    // Wait for threads to exit
exit_threads:
    for (int i = 0; i < MAX_CODECS; i++) {
        if (recorder.codecs[i].enabled && recorder.thread_id[i]) {
            pthread_join(recorder.thread_id[i], NULL);
        }
    }
    
    // Stop modules
    hd_audiocap_stop(recorder.cap_path);
    for (int i = 0; i < MAX_CODECS; i++) {
        if (recorder.codecs[i].enabled) {
            hd_audioenc_stop(recorder.enc_path[i]);
        }
    }
    
    // Unbind
    for (int i = 0; i < MAX_CODECS; i++) {
        if (recorder.codecs[i].enabled) {
            hd_audiocap_unbind(HD_AUDIOCAP_0_OUT_0);
        }
    }
    
exit_encoders:
    for (int i = 0; i < MAX_CODECS; i++) {
        if (recorder.codecs[i].enabled && recorder.enc_path[i]) {
            hd_audioenc_close(recorder.enc_path[i]);
        }
    }
    
exit_cap_path:
    hd_audiocap_close(recorder.cap_path);
    
exit_cap_ctrl:
    hd_audiocap_close(recorder.cap_ctrl);
    
exit_modules:
    exit_modules();
    
exit_mem:
    mem_exit();
    
exit_common:
    hd_common_uninit();
    
exit:
    // Print final statistics
    if (recorder.flow_start) {
        print_statistics(&recorder);
    }
    
    INFO("Program exited");
    return (ret == HD_OK) ? 0 : 1;
}
