/**
 * @brief Audio recorder with userspace G.711 encoding
 * 
 * This sample captures PCM audio using HDAL and encodes it to G.711 μ-law or A-law
 * in userspace, since the kernel encoder only supports PCM.
 * 
 * Usage: ./audio_g711_record [codec] [duration]
 *   codec: 0=PCM(raw), 1=G.711 μ-law, 2=G.711 A-law
 *   duration: recording duration in seconds (0 = until 'q' pressed)
 * 
 * @file audio_g711_record.c
 * @author Based on Novatek hd_audio_record sample
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <pthread.h>
#include <time.h>
#include "hdal.h"
#include "hd_debug.h"

// ============================================================================
// G.711 Encoder Implementation
// ============================================================================

#define G711_BIAS       0x84    // Bias for linear code
#define G711_CLIP       8159    // Max. clipping value

// μ-law encoding table
static const unsigned char ulaw_encode_table[256] = {
     0,    0,    1,    1,    2,    2,    2,    2,    3,    3,    3,    3,    3,    3,    3,    3,
     4,    4,    4,    4,    4,    4,    4,    4,    4,    4,    4,    4,    4,    4,    4,    4,
     5,    5,    5,    5,    5,    5,    5,    5,    5,    5,    5,    5,    5,    5,    5,    5,
     5,    5,    5,    5,    5,    5,    5,    5,    5,    5,    5,    5,    5,    5,    5,    5,
     6,    6,    6,    6,    6,    6,    6,    6,    6,    6,    6,    6,    6,    6,    6,    6,
     6,    6,    6,    6,    6,    6,    6,    6,    6,    6,    6,    6,    6,    6,    6,    6,
     6,    6,    6,    6,    6,    6,    6,    6,    6,    6,    6,    6,    6,    6,    6,    6,
     6,    6,    6,    6,    6,    6,    6,    6,    6,    6,    6,    6,    6,    6,    6,    6,
     7,    7,    7,    7,    7,    7,    7,    7,    7,    7,    7,    7,    7,    7,    7,    7,
     7,    7,    7,    7,    7,    7,    7,    7,    7,    7,    7,    7,    7,    7,    7,    7,
     7,    7,    7,    7,    7,    7,    7,    7,    7,    7,    7,    7,    7,    7,    7,    7,
     7,    7,    7,    7,    7,    7,    7,    7,    7,    7,    7,    7,    7,    7,    7,    7,
     7,    7,    7,    7,    7,    7,    7,    7,    7,    7,    7,    7,    7,    7,    7,    7,
     7,    7,    7,    7,    7,    7,    7,    7,    7,    7,    7,    7,    7,    7,    7,    7,
     7,    7,    7,    7,    7,    7,    7,    7,    7,    7,    7,    7,    7,    7,    7,    7,
     7,    7,    7,    7,    7,    7,    7,    7,    7,    7,    7,    7,    7,    7,    7,    7
};

/**
 * @brief Encode 16-bit linear PCM sample to 8-bit G.711 μ-law
 * @param pcm_val 16-bit signed PCM sample
 * @return 8-bit μ-law encoded value
 */
static inline unsigned char linear_to_ulaw(short pcm_val)
{
    int sign, exponent, mantissa;
    unsigned char ulawbyte;

    // Get the sign and the absolute value
    sign = (pcm_val >> 8) & 0x80;
    if (sign != 0) {
        pcm_val = -pcm_val;
    }

    // Clip the magnitude
    if (pcm_val > G711_CLIP) {
        pcm_val = G711_CLIP;
    }

    // Add bias for linear code
    pcm_val += G711_BIAS;

    // Find the segment number (exponent)
    exponent = ulaw_encode_table[(pcm_val >> 7) & 0xFF];

    // Combine the sign, segment (exponent), and mantissa
    mantissa = (pcm_val >> (exponent + 3)) & 0x0F;
    ulawbyte = ~(sign | (exponent << 4) | mantissa);

    return ulawbyte;
}

/**
 * @brief Encode 16-bit linear PCM sample to 8-bit G.711 A-law
 * @param pcm_val 16-bit signed PCM sample
 * @return 8-bit A-law encoded value
 */
static inline unsigned char linear_to_alaw(short pcm_val)
{
    int sign, exponent, mantissa;
    unsigned char alawbyte;

    // Get the sign
    sign = ((~pcm_val) >> 8) & 0x80;
    if (sign == 0) {
        pcm_val = -pcm_val;
    }

    // Clip to 13-bit magnitude
    if (pcm_val > 32635) {
        pcm_val = 32635;
    }

    // Find the exponent
    if (pcm_val >= 256) {
        exponent = 1;
        while (pcm_val >= (512 << exponent) && exponent < 7) {
            exponent++;
        }
        mantissa = (pcm_val >> (exponent + 3)) & 0x0F;
    } else {
        exponent = 0;
        mantissa = pcm_val >> 4;
    }

    // Combine the sign, exponent, and mantissa, then toggle even bits
    alawbyte = sign | (exponent << 4) | mantissa;
    alawbyte ^= 0x55;

    return alawbyte;
}

/**
 * @brief Encode PCM buffer to G.711 μ-law
 * @param pcm_data Input 16-bit PCM samples
 * @param g711_data Output 8-bit μ-law data
 * @param sample_count Number of samples to encode
 */
static void encode_ulaw(const short *pcm_data, unsigned char *g711_data, unsigned int sample_count)
{
    for (unsigned int i = 0; i < sample_count; i++) {
        g711_data[i] = linear_to_ulaw(pcm_data[i]);
    }
}

/**
 * @brief Encode PCM buffer to G.711 A-law
 * @param pcm_data Input 16-bit PCM samples
 * @param g711_data Output 8-bit A-law data
 * @param sample_count Number of samples to encode
 */
static void encode_alaw(const short *pcm_data, unsigned char *g711_data, unsigned int sample_count)
{
    for (unsigned int i = 0; i < sample_count; i++) {
        g711_data[i] = linear_to_alaw(pcm_data[i]);
    }
}

// ============================================================================
// HDAL Audio Capture (using audiocap directly, bypassing audioenc for non-PCM)
// ============================================================================

#define CODEC_PCM   0
#define CODEC_ULAW  1
#define CODEC_ALAW  2

typedef struct {
    // Audio capture
    HD_PATH_ID cap_ctrl;
    HD_PATH_ID cap_path;
    
    // Encoder path (only used for PCM passthrough)
    HD_PATH_ID enc_path;
    
    // Settings
    int codec_type;     // CODEC_PCM, CODEC_ULAW, CODEC_ALAW
    int duration;       // Recording duration in seconds (0 = manual stop)
    
    // Thread control
    pthread_t thread_id;
    volatile int exit_flag;
    volatile int flow_start;
    
    // Statistics
    unsigned long bytes_written;
    unsigned long frames_captured;
} AUDIO_STREAM;

static volatile int g_exit_signal = 0;

static void signal_handler(int sig)
{
    (void)sig;
    g_exit_signal = 1;
}

static HD_RESULT mem_init(void)
{
    HD_RESULT ret;
    HD_COMMON_MEM_INIT_CONFIG mem_cfg = {0};

    mem_cfg.pool_info[0].type = HD_COMMON_MEM_COMMON_POOL;
    mem_cfg.pool_info[0].blk_size = 0x10000;  // 64KB blocks
    mem_cfg.pool_info[0].blk_cnt = 4;
    mem_cfg.pool_info[0].ddr_id = DDR_ID0;

    ret = hd_common_mem_init(&mem_cfg);
    return ret;
}

static HD_RESULT mem_exit(void)
{
    return hd_common_mem_uninit();
}

static HD_RESULT set_cap_cfg(HD_PATH_ID *p_cap_ctrl)
{
    HD_RESULT ret;
    HD_PATH_ID cap_ctrl = 0;
    HD_AUDIOCAP_DEV_CONFIG dev_cfg = {0};
    HD_AUDIOCAP_DRV_CONFIG drv_cfg = {0};

    ret = hd_audiocap_open(0, HD_AUDIOCAP_0_CTRL, &cap_ctrl);
    if (ret != HD_OK) {
        printf("Failed to open audiocap ctrl: %d\n", ret);
        return ret;
    }

    // Device config - 48kHz, 16-bit, stereo
    dev_cfg.in_max.sample_rate = HD_AUDIO_SR_48000;
    dev_cfg.in_max.sample_bit = HD_AUDIO_BIT_WIDTH_16;
    dev_cfg.in_max.mode = HD_AUDIO_SOUND_MODE_STEREO;
    dev_cfg.in_max.frame_sample = 1024;
    dev_cfg.frame_num_max = 10;
    
    ret = hd_audiocap_set(cap_ctrl, HD_AUDIOCAP_PARAM_DEV_CONFIG, &dev_cfg);
    if (ret != HD_OK) {
        printf("Failed to set audiocap dev config: %d\n", ret);
        return ret;
    }

    // Driver config
    drv_cfg.mono = HD_AUDIO_MONO_RIGHT;
    ret = hd_audiocap_set(cap_ctrl, HD_AUDIOCAP_PARAM_DRV_CONFIG, &drv_cfg);

    *p_cap_ctrl = cap_ctrl;
    return ret;
}

static HD_RESULT set_cap_param(HD_PATH_ID cap_path)
{
    HD_AUDIOCAP_IN cap_param = {0};

    cap_param.sample_rate = HD_AUDIO_SR_48000;
    cap_param.sample_bit = HD_AUDIO_BIT_WIDTH_16;
    cap_param.mode = HD_AUDIO_SOUND_MODE_STEREO;
    cap_param.frame_sample = 1024;
    
    return hd_audiocap_set(cap_path, HD_AUDIOCAP_PARAM_IN, &cap_param);
}

static HD_RESULT set_enc_cfg(HD_PATH_ID enc_path)
{
    HD_AUDIOENC_PATH_CONFIG path_cfg = {0};

    // Always use PCM in the kernel encoder
    path_cfg.max_mem.codec_type = HD_AUDIO_CODEC_PCM;
    path_cfg.max_mem.sample_rate = HD_AUDIO_SR_48000;
    path_cfg.max_mem.sample_bit = HD_AUDIO_BIT_WIDTH_16;
    path_cfg.max_mem.mode = HD_AUDIO_SOUND_MODE_STEREO;
    
    return hd_audioenc_set(enc_path, HD_AUDIOENC_PARAM_PATH_CONFIG, &path_cfg);
}

static HD_RESULT set_enc_param(HD_PATH_ID enc_path)
{
    HD_RESULT ret;
    HD_AUDIOENC_IN in_param = {0};
    HD_AUDIOENC_OUT out_param = {0};

    in_param.sample_rate = HD_AUDIO_SR_48000;
    in_param.sample_bit = HD_AUDIO_BIT_WIDTH_16;
    in_param.mode = HD_AUDIO_SOUND_MODE_STEREO;
    
    ret = hd_audioenc_set(enc_path, HD_AUDIOENC_PARAM_IN, &in_param);
    if (ret != HD_OK) return ret;

    // Always output PCM from kernel encoder
    out_param.codec_type = HD_AUDIO_CODEC_PCM;
    out_param.aac_adts = FALSE;
    
    return hd_audioenc_set(enc_path, HD_AUDIOENC_PARAM_OUT, &out_param);
}

static void *capture_thread(void *arg)
{
    AUDIO_STREAM *stream = (AUDIO_STREAM *)arg;
    HD_RESULT ret;
    HD_AUDIO_BS data_pull;
    HD_AUDIOENC_BUFINFO buf_info;
    UINTPTR vir_addr;
    char file_path[128];
    FILE *f_out = NULL;
    const char *codec_name;
    time_t start_time;
    
    // G.711 encoding buffer
    unsigned char *g711_buf = NULL;
    unsigned int g711_buf_size = 0;

    // Wait for flow_start
    while (stream->flow_start == 0 && !g_exit_signal) {
        usleep(10000);
    }

    // Determine codec name and file extension
    switch (stream->codec_type) {
        case CODEC_ULAW:
            codec_name = "g711u";
            break;
        case CODEC_ALAW:
            codec_name = "g711a";
            break;
        default:
            codec_name = "pcm";
            break;
    }

    // Create output file path
    snprintf(file_path, sizeof(file_path), "/mnt/sd/audio_48000_stereo_%s.raw", codec_name);

    // Get buffer info
    ret = hd_audioenc_get(stream->enc_path, HD_AUDIOENC_PARAM_BUFINFO, &buf_info);
    if (ret != HD_OK) {
        printf("Failed to get encoder buffer info: %d\n", ret);
        return NULL;
    }

    // Map buffer to userspace
    vir_addr = (UINTPTR)hd_common_mem_mmap(HD_COMMON_MEM_MEM_TYPE_CACHE, 
                                            buf_info.buf_info.phy_addr, 
                                            buf_info.buf_info.buf_size);
    if (vir_addr == 0) {
        printf("Failed to mmap buffer\n");
        return NULL;
    }

    // Open output file
    f_out = fopen(file_path, "wb");
    if (!f_out) {
        printf("Failed to open output file: %s\n", file_path);
        hd_common_mem_munmap((void *)vir_addr, buf_info.buf_info.buf_size);
        return NULL;
    }

    printf("\n========================================\n");
    printf("Recording to: %s\n", file_path);
    printf("Codec: %s\n", codec_name);
    printf("Sample Rate: 48000 Hz\n");
    printf("Channels: 2 (Stereo)\n");
    printf("Bit Depth: 16-bit PCM -> %s\n", 
           stream->codec_type == CODEC_PCM ? "16-bit" : "8-bit");
    if (stream->duration > 0) {
        printf("Duration: %d seconds\n", stream->duration);
    } else {
        printf("Duration: Press 'q' to stop\n");
    }
    printf("========================================\n\n");

    start_time = time(NULL);

    // Main capture loop
    while (!stream->exit_flag && !g_exit_signal) {
        // Check duration
        if (stream->duration > 0) {
            time_t elapsed = time(NULL) - start_time;
            if (elapsed >= stream->duration) {
                printf("\nDuration reached (%d seconds)\n", stream->duration);
                break;
            }
        }

        // Pull data from encoder (blocking mode)
        ret = hd_audioenc_pull_out_buf(stream->enc_path, &data_pull, 100);
        
        if (ret == HD_OK && data_pull.size > 0) {
            unsigned char *pcm_ptr = (unsigned char *)(vir_addr + (data_pull.phy_addr - buf_info.buf_info.phy_addr));
            unsigned int pcm_size = data_pull.size;
            unsigned int sample_count = pcm_size / 2;  // 16-bit samples

            if (stream->codec_type == CODEC_PCM) {
                // Write raw PCM
                fwrite(pcm_ptr, 1, pcm_size, f_out);
                stream->bytes_written += pcm_size;
            } else {
                // Encode to G.711
                if (g711_buf_size < sample_count) {
                    g711_buf = realloc(g711_buf, sample_count);
                    g711_buf_size = sample_count;
                }

                if (g711_buf) {
                    if (stream->codec_type == CODEC_ULAW) {
                        encode_ulaw((const short *)pcm_ptr, g711_buf, sample_count);
                    } else {
                        encode_alaw((const short *)pcm_ptr, g711_buf, sample_count);
                    }
                    fwrite(g711_buf, 1, sample_count, f_out);
                    stream->bytes_written += sample_count;
                }
            }

            stream->frames_captured++;
            fflush(f_out);

            // Release buffer
            hd_audioenc_release_out_buf(stream->enc_path, &data_pull);

            // Print progress every 100 frames
            if (stream->frames_captured % 100 == 0) {
                double kb = stream->bytes_written / 1024.0;
                time_t elapsed = time(NULL) - start_time;
                printf("\rCaptured: %.1f KB (%lu frames, %ld sec)", kb, stream->frames_captured, elapsed);
                fflush(stdout);
            }
        } else if (ret == HD_ERR_TIMEDOUT) {
            // Timeout, just continue
            continue;
        } else if (ret != HD_OK) {
            printf("\nPull buffer error: %d\n", ret);
            break;
        }
    }

    printf("\n\nRecording complete!\n");
    printf("Total: %.2f KB (%lu frames)\n", stream->bytes_written / 1024.0, stream->frames_captured);
    printf("Output file: %s\n", file_path);

    // Playback hints
    if (stream->codec_type == CODEC_ULAW) {
        printf("\nTo play with ffplay:\n");
        printf("  ffplay -f mulaw -ar 48000 -ac 2 %s\n", file_path);
        printf("\nTo convert to WAV:\n");
        printf("  ffmpeg -f mulaw -ar 48000 -ac 2 -i %s output.wav\n", file_path);
    } else if (stream->codec_type == CODEC_ALAW) {
        printf("\nTo play with ffplay:\n");
        printf("  ffplay -f alaw -ar 48000 -ac 2 %s\n", file_path);
        printf("\nTo convert to WAV:\n");
        printf("  ffmpeg -f alaw -ar 48000 -ac 2 -i %s output.wav\n", file_path);
    } else {
        printf("\nTo play with ffplay:\n");
        printf("  ffplay -f s16le -ar 48000 -ac 2 %s\n", file_path);
        printf("\nTo convert to WAV:\n");
        printf("  ffmpeg -f s16le -ar 48000 -ac 2 -i %s output.wav\n", file_path);
    }

    // Cleanup
    if (g711_buf) free(g711_buf);
    if (f_out) fclose(f_out);
    hd_common_mem_munmap((void *)vir_addr, buf_info.buf_info.buf_size);

    return NULL;
}

static void print_usage(const char *prog)
{
    printf("Usage: %s [codec] [duration]\n", prog);
    printf("  codec:    0 = PCM (raw 16-bit)\n");
    printf("            1 = G.711 μ-law (8-bit)\n");
    printf("            2 = G.711 A-law (8-bit)\n");
    printf("  duration: Recording time in seconds (0 = until 'q' pressed)\n");
    printf("\nExample:\n");
    printf("  %s 1 10    # Record 10 seconds of G.711 μ-law\n", prog);
    printf("  %s 2 0     # Record G.711 A-law until 'q' pressed\n", prog);
}

int main(int argc, char *argv[])
{
    HD_RESULT ret;
    AUDIO_STREAM stream = {0};
    int key;

    // Parse arguments
    if (argc >= 2) {
        stream.codec_type = atoi(argv[1]);
        if (stream.codec_type < 0 || stream.codec_type > 2) {
            printf("Invalid codec type: %d\n", stream.codec_type);
            print_usage(argv[0]);
            return 1;
        }
    }
    
    if (argc >= 3) {
        stream.duration = atoi(argv[2]);
        if (stream.duration < 0) stream.duration = 0;
    }

    printf("\n=== G.711 Audio Recorder ===\n");
    printf("Codec: %s\n", 
           stream.codec_type == CODEC_ULAW ? "G.711 μ-law" :
           stream.codec_type == CODEC_ALAW ? "G.711 A-law" : "PCM");
    printf("Duration: %s\n", 
           stream.duration > 0 ? "" : "manual (press 'q' to stop)");
    if (stream.duration > 0) printf("%d seconds\n", stream.duration);

    // Setup signal handler
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    // Initialize HDAL
    ret = hd_common_init(0);
    if (ret != HD_OK) {
        printf("hd_common_init failed: %d\n", ret);
        return 1;
    }

    // Initialize memory
    ret = mem_init();
    if (ret != HD_OK) {
        printf("mem_init failed: %d\n", ret);
        goto cleanup_common;
    }

    // Initialize modules
    ret = hd_audiocap_init();
    if (ret != HD_OK) {
        printf("hd_audiocap_init failed: %d\n", ret);
        goto cleanup_mem;
    }

    ret = hd_audioenc_init();
    if (ret != HD_OK) {
        printf("hd_audioenc_init failed: %d\n", ret);
        goto cleanup_audiocap;
    }

    // Setup audio capture
    ret = set_cap_cfg(&stream.cap_ctrl);
    if (ret != HD_OK) {
        printf("set_cap_cfg failed: %d\n", ret);
        goto cleanup_audioenc;
    }

    // Open paths
    ret = hd_audiocap_open(HD_AUDIOCAP_0_IN_0, HD_AUDIOCAP_0_OUT_0, &stream.cap_path);
    if (ret != HD_OK) {
        printf("hd_audiocap_open failed: %d\n", ret);
        goto cleanup_audioenc;
    }

    ret = hd_audioenc_open(HD_AUDIOENC_0_IN_0, HD_AUDIOENC_0_OUT_0, &stream.enc_path);
    if (ret != HD_OK) {
        printf("hd_audioenc_open failed: %d\n", ret);
        goto cleanup_cap_path;
    }

    // Set capture parameters
    ret = set_cap_param(stream.cap_path);
    if (ret != HD_OK) {
        printf("set_cap_param failed: %d\n", ret);
        goto cleanup_enc_path;
    }

    // Set encoder config (always PCM)
    ret = set_enc_cfg(stream.enc_path);
    if (ret != HD_OK) {
        printf("set_enc_cfg failed: %d\n", ret);
        goto cleanup_enc_path;
    }

    // Set encoder parameters
    ret = set_enc_param(stream.enc_path);
    if (ret != HD_OK) {
        printf("set_enc_param failed: %d\n", ret);
        goto cleanup_enc_path;
    }

    // Bind capture to encoder
    ret = hd_audiocap_bind(HD_AUDIOCAP_0_OUT_0, HD_AUDIOENC_0_IN_0);
    if (ret != HD_OK) {
        printf("hd_audiocap_bind failed: %d\n", ret);
        goto cleanup_enc_path;
    }

    // Create capture thread
    ret = pthread_create(&stream.thread_id, NULL, capture_thread, &stream);
    if (ret != 0) {
        printf("pthread_create failed: %d\n", ret);
        goto cleanup_bind;
    }

    // Start modules
    ret = hd_audioenc_start(stream.enc_path);
    if (ret != HD_OK) {
        printf("hd_audioenc_start failed: %d\n", ret);
        goto cleanup_thread;
    }

    ret = hd_audiocap_start(stream.cap_path);
    if (ret != HD_OK) {
        printf("hd_audiocap_start failed: %d\n", ret);
        hd_audioenc_stop(stream.enc_path);
        goto cleanup_thread;
    }

    // Signal thread to start
    stream.flow_start = 1;

    // Wait for completion or user input
    if (stream.duration > 0) {
        // Wait for thread to complete
        pthread_join(stream.thread_id, NULL);
    } else {
        // Wait for 'q' key
        printf("Press 'q' to stop recording...\n");
        while (!g_exit_signal) {
            key = getchar();
            if (key == 'q' || key == 'Q' || key == 3) {  // 3 = Ctrl+C
                stream.exit_flag = 1;
                break;
            }
        }
        pthread_join(stream.thread_id, NULL);
    }

    // Stop modules
    hd_audiocap_stop(stream.cap_path);
    hd_audioenc_stop(stream.enc_path);

cleanup_thread:
    if (stream.thread_id && stream.duration > 0) {
        stream.exit_flag = 1;
        pthread_join(stream.thread_id, NULL);
    }

cleanup_bind:
    hd_audiocap_unbind(HD_AUDIOCAP_0_OUT_0);

cleanup_enc_path:
    hd_audioenc_close(stream.enc_path);

cleanup_cap_path:
    hd_audiocap_close(stream.cap_path);

cleanup_audioenc:
    hd_audioenc_uninit();

cleanup_audiocap:
    hd_audiocap_uninit();

cleanup_mem:
    mem_exit();

cleanup_common:
    hd_common_uninit();

    return 0;
}
