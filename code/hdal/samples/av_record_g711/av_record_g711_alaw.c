/**
 * @brief Combined Audio/Video Recording with G.711 A-law Audio
 * 
 * This sample captures video using H.264/H.265 encoder and audio using G.711 A-law
 * encoding. Both streams are saved to separate files that can be muxed later.
 * 
 * Usage: ./av_record_g711_alaw [video_codec] [duration]
 *   video_codec: 0=H.265, 1=H.264 (default: 0)
 *   duration: recording duration in seconds (0 = until 'q' pressed)
 * 
 * Output files (on /mnt/sd/):
 *   - av_record_video.h26x  (H.264 or H.265 elementary stream)
 *   - av_record_audio_alaw.raw (G.711 A-law audio, 48kHz stereo)
 * 
 * To mux with ffmpeg:
 *   ffmpeg -i av_record_video.h265 -f alaw -ar 48000 -ac 2 -i av_record_audio_alaw.raw \
 *          -c:v copy -c:a aac output.mp4
 * 
 * @file av_record_g711_alaw.c
 * @author Based on Novatek HDAL samples
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
// G.711 A-law Encoder Implementation
// ============================================================================

/**
 * @brief Encode 16-bit linear PCM sample to 8-bit G.711 A-law
 * @param pcm_val 16-bit signed PCM sample
 * @return 8-bit A-law encoded value
 */
static inline unsigned char linear_to_alaw(short pcm_val)
{
    int sign, exponent, mantissa;
    unsigned char alawbyte;

    // Get the sign (A-law uses inverted sign)
    sign = ((~pcm_val) >> 8) & 0x80;
    if (sign == 0) {
        pcm_val = -pcm_val;
    }

    // Clip to 13-bit magnitude
    if (pcm_val > 32635) {
        pcm_val = 32635;
    }

    // Find the exponent and mantissa
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
// Configuration
// ============================================================================

#define SEN1_VCAP_ID        0
#define VDO_SIZE_W          2960
#define VDO_SIZE_H          1664
#define VIDEO_BITRATE       (4 * 1024 * 1024)  // 4 Mbps

#define AUDIO_SAMPLE_RATE   HD_AUDIO_SR_48000
#define AUDIO_BIT_WIDTH     HD_AUDIO_BIT_WIDTH_16
#define AUDIO_MODE          HD_AUDIO_SOUND_MODE_STEREO
#define AUDIO_FRAME_SAMPLE  1024

// Buffer size macros
#define DBGINFO_BUFSIZE()               (0x200)
#define VDO_RAW_BUFSIZE(w, h, pxlfmt)   (ALIGN_CEIL_4((w) * HD_VIDEO_PXLFMT_BPP(pxlfmt) / 8) * (h))
#define VDO_YUV_BUFSIZE(w, h, pxlfmt)   (ALIGN_CEIL_4((w) * HD_VIDEO_PXLFMT_BPP(pxlfmt) / 8) * (h))
#define VDO_CA_BUF_SIZE(win_num_w, win_num_h) ALIGN_CEIL_4((win_num_w * win_num_h << 3) << 1)
#define VDO_LA_BUF_SIZE(win_num_w, win_num_h) ALIGN_CEIL_4((win_num_w * win_num_h << 1) << 1)

#define SEN_OUT_FMT         HD_VIDEO_PXLFMT_RAW10
#define CAP_OUT_FMT         HD_VIDEO_PXLFMT_RAW10
#define CA_WIN_NUM_W        32
#define CA_WIN_NUM_H        32
#define LA_WIN_NUM_W        32
#define LA_WIN_NUM_H        32

// ============================================================================
// Data Structures
// ============================================================================

typedef struct _AV_RECORD {
    // Video capture
    HD_PATH_ID vcap_ctrl;
    HD_PATH_ID vcap_path;
    HD_DIM vcap_dim;

    // Video processor
    HD_PATH_ID vproc_ctrl;
    HD_PATH_ID vproc_path;
    HD_DIM vproc_max_dim;

    // Video encoder
    HD_PATH_ID venc_path;
    HD_DIM venc_dim;

    // Audio capture
    HD_PATH_ID acap_ctrl;
    HD_PATH_ID acap_path;

    // Audio encoder
    HD_PATH_ID aenc_path;

    // Settings
    UINT32 video_codec;    // 0=H.265, 1=H.264
    int duration;          // Recording duration (0=manual)

    // Threads
    pthread_t video_thread_id;
    pthread_t audio_thread_id;

    // Control flags
    volatile UINT32 exit_flag;
    volatile UINT32 flow_start;

    // Statistics
    unsigned long video_bytes;
    unsigned long video_frames;
    unsigned long audio_bytes;
    unsigned long audio_frames;

} AV_RECORD;

static volatile int g_exit_signal = 0;

static void signal_handler(int sig)
{
    (void)sig;
    g_exit_signal = 1;
}

// ============================================================================
// Memory Initialization
// ============================================================================

static HD_RESULT mem_init(void)
{
    HD_RESULT ret;
    HD_COMMON_MEM_INIT_CONFIG mem_cfg = {0};

    // Video capture pool
    mem_cfg.pool_info[0].type = HD_COMMON_MEM_COMMON_POOL;
    mem_cfg.pool_info[0].blk_size = DBGINFO_BUFSIZE() + 
                                     VDO_RAW_BUFSIZE(VDO_SIZE_W, VDO_SIZE_H, CAP_OUT_FMT) +
                                     VDO_CA_BUF_SIZE(CA_WIN_NUM_W, CA_WIN_NUM_H) +
                                     VDO_LA_BUF_SIZE(LA_WIN_NUM_W, LA_WIN_NUM_H);
    mem_cfg.pool_info[0].blk_cnt = 3;
    mem_cfg.pool_info[0].ddr_id = DDR_ID0;

    // Video processing pool
    mem_cfg.pool_info[1].type = HD_COMMON_MEM_COMMON_POOL;
    mem_cfg.pool_info[1].blk_size = DBGINFO_BUFSIZE() + 
                                     VDO_YUV_BUFSIZE(VDO_SIZE_W, VDO_SIZE_H, HD_VIDEO_PXLFMT_YUV420);
    mem_cfg.pool_info[1].blk_cnt = 3;
    mem_cfg.pool_info[1].ddr_id = DDR_ID0;

    // Audio pool
    mem_cfg.pool_info[2].type = HD_COMMON_MEM_COMMON_POOL;
    mem_cfg.pool_info[2].blk_size = 0x10000;  // 64KB blocks
    mem_cfg.pool_info[2].blk_cnt = 4;
    mem_cfg.pool_info[2].ddr_id = DDR_ID0;

    ret = hd_common_mem_init(&mem_cfg);
    return ret;
}

static HD_RESULT mem_exit(void)
{
    return hd_common_mem_uninit();
}

// ============================================================================
// Video Setup Functions
// ============================================================================

static HD_RESULT set_vcap_cfg(HD_PATH_ID *p_vcap_ctrl)
{
    HD_RESULT ret;
    HD_VIDEOCAP_DRV_CONFIG cap_cfg = {0};
    HD_PATH_ID vcap_ctrl = 0;
    HD_VIDEOCAP_CTRL iq_ctl = {0};

    snprintf(cap_cfg.sen_cfg.sen_dev.driver_name, HD_VIDEOCAP_SEN_NAME_LEN-1, "nvt_sen_gc5603");
    cap_cfg.sen_cfg.sen_dev.if_type = HD_COMMON_VIDEO_IN_MIPI_CSI;
    cap_cfg.sen_cfg.sen_dev.pin_cfg.pinmux.sensor_pinmux = 0;
    cap_cfg.sen_cfg.sen_dev.pin_cfg.clk_lane_sel = HD_VIDEOCAP_SEN_CLANE_CSI(0, 0);
    cap_cfg.sen_cfg.sen_dev.pin_cfg.sen_2_serial_pin_map[0] = 0;
    cap_cfg.sen_cfg.sen_dev.pin_cfg.sen_2_serial_pin_map[1] = 1;
    cap_cfg.sen_cfg.sen_dev.pin_cfg.sen_2_serial_pin_map[2] = HD_VIDEOCAP_SEN_IGNORE;
    cap_cfg.sen_cfg.sen_dev.pin_cfg.sen_2_serial_pin_map[3] = HD_VIDEOCAP_SEN_IGNORE;
    cap_cfg.sen_cfg.sen_dev.pin_cfg.sen_2_serial_pin_map[4] = HD_VIDEOCAP_SEN_IGNORE;
    cap_cfg.sen_cfg.sen_dev.pin_cfg.sen_2_serial_pin_map[5] = HD_VIDEOCAP_SEN_IGNORE;
    cap_cfg.sen_cfg.sen_dev.pin_cfg.sen_2_serial_pin_map[6] = HD_VIDEOCAP_SEN_IGNORE;
    cap_cfg.sen_cfg.sen_dev.pin_cfg.sen_2_serial_pin_map[7] = HD_VIDEOCAP_SEN_IGNORE;

    ret = hd_videocap_open(0, HD_VIDEOCAP_CTRL(SEN1_VCAP_ID), &vcap_ctrl);
    if (ret != HD_OK) {
        return ret;
    }

    ret = hd_videocap_set(vcap_ctrl, HD_VIDEOCAP_PARAM_DRV_CONFIG, &cap_cfg);
    if (ret != HD_OK) {
        return ret;
    }

    iq_ctl.func = HD_VIDEOCAP_FUNC_AE | HD_VIDEOCAP_FUNC_AWB;
    ret = hd_videocap_set(vcap_ctrl, HD_VIDEOCAP_PARAM_CTRL, &iq_ctl);

    *p_vcap_ctrl = vcap_ctrl;
    return ret;
}

static HD_RESULT set_vcap_param(HD_PATH_ID vcap_path, HD_DIM *p_dim)
{
    HD_RESULT ret;
    HD_VIDEOCAP_IN video_in_param = {0};
    HD_VIDEOCAP_CROP video_crop_param = {0};
    HD_VIDEOCAP_OUT video_out_param = {0};

    video_in_param.sen_mode = HD_VIDEOCAP_SEN_MODE_AUTO;
    video_in_param.frc = HD_VIDEO_FRC_RATIO(30, 1);
    video_in_param.dim.w = p_dim->w;
    video_in_param.dim.h = p_dim->h;
    video_in_param.pxlfmt = SEN_OUT_FMT;
    video_in_param.out_frame_num = HD_VIDEOCAP_SEN_FRAME_NUM_1;
    ret = hd_videocap_set(vcap_path, HD_VIDEOCAP_PARAM_IN, &video_in_param);
    if (ret != HD_OK) return ret;

    video_crop_param.mode = HD_CROP_OFF;
    ret = hd_videocap_set(vcap_path, HD_VIDEOCAP_PARAM_IN_CROP, &video_crop_param);
    if (ret != HD_OK) return ret;

    video_out_param.pxlfmt = CAP_OUT_FMT;
    video_out_param.dir = HD_VIDEO_DIR_NONE;
    ret = hd_videocap_set(vcap_path, HD_VIDEOCAP_PARAM_OUT, &video_out_param);

    return ret;
}

static HD_RESULT set_vproc_cfg(HD_PATH_ID *p_vproc_ctrl, HD_DIM *p_max_dim)
{
    HD_RESULT ret;
    HD_VIDEOPROC_DEV_CONFIG video_cfg_param = {0};
    HD_VIDEOPROC_CTRL video_ctrl_param = {0};
    HD_PATH_ID vproc_ctrl = 0;

    ret = hd_videoproc_open(0, HD_VIDEOPROC_0_CTRL, &vproc_ctrl);
    if (ret != HD_OK) return ret;

    if (p_max_dim != NULL) {
        video_cfg_param.pipe = HD_VIDEOPROC_PIPE_RAWALL;
        video_cfg_param.isp_id = SEN1_VCAP_ID;
        video_cfg_param.ctrl_max.func = 0;
        video_cfg_param.in_max.func = 0;
        video_cfg_param.in_max.dim.w = p_max_dim->w;
        video_cfg_param.in_max.dim.h = p_max_dim->h;
        video_cfg_param.in_max.pxlfmt = CAP_OUT_FMT;
        video_cfg_param.in_max.frc = HD_VIDEO_FRC_RATIO(1, 1);
        ret = hd_videoproc_set(vproc_ctrl, HD_VIDEOPROC_PARAM_DEV_CONFIG, &video_cfg_param);
        if (ret != HD_OK) return ret;
    }

    video_ctrl_param.func = 0;
    ret = hd_videoproc_set(vproc_ctrl, HD_VIDEOPROC_PARAM_CTRL, &video_ctrl_param);

    *p_vproc_ctrl = vproc_ctrl;
    return ret;
}

static HD_RESULT set_vproc_param(HD_PATH_ID vproc_path, HD_DIM *p_dim)
{
    HD_RESULT ret = HD_OK;

    if (p_dim != NULL) {
        HD_VIDEOPROC_OUT video_out_param = {0};
        video_out_param.func = 0;
        video_out_param.dim.w = p_dim->w;
        video_out_param.dim.h = p_dim->h;
        video_out_param.pxlfmt = HD_VIDEO_PXLFMT_YUV420;
        video_out_param.dir = HD_VIDEO_DIR_NONE;
        video_out_param.frc = HD_VIDEO_FRC_RATIO(1, 1);
        ret = hd_videoproc_set(vproc_path, HD_VIDEOPROC_PARAM_OUT, &video_out_param);
    }
    return ret;
}

static HD_RESULT set_venc_cfg(HD_PATH_ID venc_path, HD_DIM *p_max_dim, UINT32 max_bitrate)
{
    HD_RESULT ret;
    HD_VIDEOENC_PATH_CONFIG video_path_config = {0};

    if (p_max_dim != NULL) {
        video_path_config.max_mem.codec_type = HD_CODEC_TYPE_H264;
        video_path_config.max_mem.max_dim.w = p_max_dim->w;
        video_path_config.max_mem.max_dim.h = p_max_dim->h;
        video_path_config.max_mem.bitrate = max_bitrate;
        video_path_config.max_mem.enc_buf_ms = 3000;
        video_path_config.max_mem.svc_layer = HD_SVC_4X;
        video_path_config.max_mem.ltr = TRUE;
        video_path_config.max_mem.rotate = FALSE;
        video_path_config.max_mem.source_output = FALSE;
        video_path_config.isp_id = SEN1_VCAP_ID;
        ret = hd_videoenc_set(venc_path, HD_VIDEOENC_PARAM_PATH_CONFIG, &video_path_config);
        if (ret != HD_OK) {
            printf("set_venc_cfg failed: %d\n", ret);
            return ret;
        }
    }
    return HD_OK;
}

static HD_RESULT set_venc_param(HD_PATH_ID venc_path, HD_DIM *p_dim, UINT32 enc_type, UINT32 bitrate)
{
    HD_RESULT ret;
    HD_VIDEOENC_IN video_in_param = {0};
    HD_VIDEOENC_OUT video_out_param = {0};
    HD_H26XENC_RATE_CONTROL rc_param = {0};

    if (p_dim == NULL) return HD_ERR_NG;

    video_in_param.dir = HD_VIDEO_DIR_NONE;
    video_in_param.pxl_fmt = HD_VIDEO_PXLFMT_YUV420;
    video_in_param.dim.w = p_dim->w;
    video_in_param.dim.h = p_dim->h;
    video_in_param.frc = HD_VIDEO_FRC_RATIO(1, 1);
    ret = hd_videoenc_set(venc_path, HD_VIDEOENC_PARAM_IN, &video_in_param);
    if (ret != HD_OK) return ret;

    if (enc_type == 0) {
        // H.265
        video_out_param.codec_type = HD_CODEC_TYPE_H265;
        video_out_param.h26x.profile = HD_H265E_MAIN_PROFILE;
        video_out_param.h26x.level_idc = HD_H265E_LEVEL_5;
        video_out_param.h26x.gop_num = 30;
        video_out_param.h26x.ltr_interval = 0;
        video_out_param.h26x.ltr_pre_ref = 0;
        video_out_param.h26x.gray_en = 0;
        video_out_param.h26x.source_output = 0;
        video_out_param.h26x.svc_layer = HD_SVC_DISABLE;
        video_out_param.h26x.entropy_mode = HD_H265E_CABAC_CODING;
    } else {
        // H.264
        video_out_param.codec_type = HD_CODEC_TYPE_H264;
        video_out_param.h26x.profile = HD_H264E_HIGH_PROFILE;
        video_out_param.h26x.level_idc = HD_H264E_LEVEL_5_1;
        video_out_param.h26x.gop_num = 30;
        video_out_param.h26x.ltr_interval = 0;
        video_out_param.h26x.ltr_pre_ref = 0;
        video_out_param.h26x.gray_en = 0;
        video_out_param.h26x.source_output = 0;
        video_out_param.h26x.svc_layer = HD_SVC_DISABLE;
        video_out_param.h26x.entropy_mode = HD_H264E_CABAC_CODING;
    }

    ret = hd_videoenc_set(venc_path, HD_VIDEOENC_PARAM_OUT_ENC_PARAM, &video_out_param);
    if (ret != HD_OK) return ret;

    rc_param.rc_mode = HD_RC_MODE_CBR;
    rc_param.cbr.bitrate = bitrate;
    rc_param.cbr.frame_rate_base = 30;
    rc_param.cbr.frame_rate_incr = 1;
    rc_param.cbr.init_i_qp = 26;
    rc_param.cbr.min_i_qp = 10;
    rc_param.cbr.max_i_qp = 45;
    rc_param.cbr.init_p_qp = 26;
    rc_param.cbr.min_p_qp = 10;
    rc_param.cbr.max_p_qp = 45;
    rc_param.cbr.static_time = 4;
    rc_param.cbr.ip_weight = 0;
    ret = hd_videoenc_set(venc_path, HD_VIDEOENC_PARAM_OUT_RATE_CONTROL, &rc_param);

    return ret;
}

// ============================================================================
// Audio Setup Functions
// ============================================================================

static HD_RESULT set_acap_cfg(HD_PATH_ID *p_acap_ctrl)
{
    HD_RESULT ret;
    HD_PATH_ID acap_ctrl = 0;
    HD_AUDIOCAP_DEV_CONFIG dev_cfg = {0};
    HD_AUDIOCAP_DRV_CONFIG drv_cfg = {0};

    ret = hd_audiocap_open(0, HD_AUDIOCAP_0_CTRL, &acap_ctrl);
    if (ret != HD_OK) {
        printf("hd_audiocap_open ctrl failed: %d\n", ret);
        return ret;
    }

    dev_cfg.in_max.sample_rate = AUDIO_SAMPLE_RATE;
    dev_cfg.in_max.sample_bit = AUDIO_BIT_WIDTH;
    dev_cfg.in_max.mode = AUDIO_MODE;
    dev_cfg.in_max.frame_sample = AUDIO_FRAME_SAMPLE;
    dev_cfg.frame_num_max = 10;

    ret = hd_audiocap_set(acap_ctrl, HD_AUDIOCAP_PARAM_DEV_CONFIG, &dev_cfg);
    if (ret != HD_OK) {
        printf("hd_audiocap_set dev_config failed: %d\n", ret);
        return ret;
    }

    drv_cfg.mono = HD_AUDIO_MONO_RIGHT;
    ret = hd_audiocap_set(acap_ctrl, HD_AUDIOCAP_PARAM_DRV_CONFIG, &drv_cfg);

    *p_acap_ctrl = acap_ctrl;
    return ret;
}

static HD_RESULT set_acap_param(HD_PATH_ID acap_path)
{
    HD_AUDIOCAP_IN cap_param = {0};

    cap_param.sample_rate = AUDIO_SAMPLE_RATE;
    cap_param.sample_bit = AUDIO_BIT_WIDTH;
    cap_param.mode = AUDIO_MODE;
    cap_param.frame_sample = AUDIO_FRAME_SAMPLE;

    return hd_audiocap_set(acap_path, HD_AUDIOCAP_PARAM_IN, &cap_param);
}

static HD_RESULT set_aenc_cfg(HD_PATH_ID aenc_path)
{
    HD_AUDIOENC_PATH_CONFIG path_cfg = {0};

    // Always use PCM in kernel, encode G.711 in userspace
    path_cfg.max_mem.codec_type = HD_AUDIO_CODEC_PCM;
    path_cfg.max_mem.sample_rate = AUDIO_SAMPLE_RATE;
    path_cfg.max_mem.sample_bit = AUDIO_BIT_WIDTH;
    path_cfg.max_mem.mode = AUDIO_MODE;

    return hd_audioenc_set(aenc_path, HD_AUDIOENC_PARAM_PATH_CONFIG, &path_cfg);
}

static HD_RESULT set_aenc_param(HD_PATH_ID aenc_path)
{
    HD_RESULT ret;
    HD_AUDIOENC_IN in_param = {0};
    HD_AUDIOENC_OUT out_param = {0};

    in_param.sample_rate = AUDIO_SAMPLE_RATE;
    in_param.sample_bit = AUDIO_BIT_WIDTH;
    in_param.mode = AUDIO_MODE;

    ret = hd_audioenc_set(aenc_path, HD_AUDIOENC_PARAM_IN, &in_param);
    if (ret != HD_OK) return ret;

    out_param.codec_type = HD_AUDIO_CODEC_PCM;
    out_param.aac_adts = FALSE;

    return hd_audioenc_set(aenc_path, HD_AUDIOENC_PARAM_OUT, &out_param);
}

// ============================================================================
// Recording Threads
// ============================================================================

static void *video_encode_thread(void *arg)
{
    AV_RECORD *p_stream = (AV_RECORD *)arg;
    HD_RESULT ret;
    HD_VIDEOENC_BS data_pull;
    UINT32 j;
    UINTPTR vir_addr;
    HD_VIDEOENC_BUFINFO phy_buf;
    char file_path[64];
    FILE *f_out = NULL;
    time_t start_time;

    // Wait for flow_start
    while (p_stream->flow_start == 0 && !g_exit_signal) {
        usleep(10000);
    }

    // Query physical address of bitstream buffer
    ret = hd_videoenc_get(p_stream->venc_path, HD_VIDEOENC_PARAM_BUFINFO, &phy_buf);
    if (ret != HD_OK) {
        printf("Video: failed to get buffer info: %d\n", ret);
        return NULL;
    }

    // Map buffer to userspace
    vir_addr = (UINTPTR)hd_common_mem_mmap(HD_COMMON_MEM_MEM_TYPE_CACHE,
                                            phy_buf.buf_info.phy_addr,
                                            phy_buf.buf_info.buf_size);
    if (vir_addr == 0) {
        printf("Video: failed to mmap buffer\n");
        return NULL;
    }

    // Open output file
    snprintf(file_path, sizeof(file_path), "/mnt/sd/av_record_alaw_video.%s",
             p_stream->video_codec == 0 ? "h265" : "h264");

    f_out = fopen(file_path, "wb");
    if (!f_out) {
        printf("Video: failed to open output file: %s\n", file_path);
        hd_common_mem_munmap((void *)vir_addr, phy_buf.buf_info.buf_size);
        return NULL;
    }

    printf("Video: recording to %s\n", file_path);
    start_time = time(NULL);

    // Main encoding loop
    while (!p_stream->exit_flag && !g_exit_signal) {
        // Check duration
        if (p_stream->duration > 0) {
            time_t elapsed = time(NULL) - start_time;
            if (elapsed >= p_stream->duration) {
                break;
            }
        }

        // Pull encoded data
        ret = hd_videoenc_pull_out_buf(p_stream->venc_path, &data_pull, 100);

        if (ret == HD_OK) {
            for (j = 0; j < data_pull.pack_num; j++) {
                UINT8 *ptr = (UINT8 *)(vir_addr + (data_pull.video_pack[j].phy_addr - phy_buf.buf_info.phy_addr));
                UINT32 len = data_pull.video_pack[j].size;
                if (f_out) {
                    fwrite(ptr, 1, len, f_out);
                    fflush(f_out);
                }
                p_stream->video_bytes += len;
            }
            p_stream->video_frames++;

            ret = hd_videoenc_release_out_buf(p_stream->venc_path, &data_pull);
            if (ret != HD_OK) {
                printf("Video: release buffer error: %d\n", ret);
            }
        }
    }

    printf("Video: recording complete - %lu frames, %.2f MB\n",
           p_stream->video_frames, p_stream->video_bytes / (1024.0 * 1024.0));

    // Cleanup
    if (f_out) fclose(f_out);
    hd_common_mem_munmap((void *)vir_addr, phy_buf.buf_info.buf_size);

    return NULL;
}

static void *audio_encode_thread(void *arg)
{
    AV_RECORD *p_stream = (AV_RECORD *)arg;
    HD_RESULT ret;
    HD_AUDIO_BS data_pull;
    HD_AUDIOENC_BUFINFO buf_info;
    UINTPTR vir_addr;
    char file_path[64] = "/mnt/sd/av_record_audio_alaw.raw";
    FILE *f_out = NULL;
    time_t start_time;

    // G.711 encoding buffer
    unsigned char *g711_buf = NULL;
    unsigned int g711_buf_size = 0;

    // Wait for flow_start
    while (p_stream->flow_start == 0 && !g_exit_signal) {
        usleep(10000);
    }

    // Get buffer info
    ret = hd_audioenc_get(p_stream->aenc_path, HD_AUDIOENC_PARAM_BUFINFO, &buf_info);
    if (ret != HD_OK) {
        printf("Audio: failed to get buffer info: %d\n", ret);
        return NULL;
    }

    // Map buffer to userspace
    vir_addr = (UINTPTR)hd_common_mem_mmap(HD_COMMON_MEM_MEM_TYPE_CACHE,
                                            buf_info.buf_info.phy_addr,
                                            buf_info.buf_info.buf_size);
    if (vir_addr == 0) {
        printf("Audio: failed to mmap buffer\n");
        return NULL;
    }

    // Open output file
    f_out = fopen(file_path, "wb");
    if (!f_out) {
        printf("Audio: failed to open output file: %s\n", file_path);
        hd_common_mem_munmap((void *)vir_addr, buf_info.buf_info.buf_size);
        return NULL;
    }

    printf("Audio: recording G.711 A-law to %s\n", file_path);
    start_time = time(NULL);

    // Main capture loop
    while (!p_stream->exit_flag && !g_exit_signal) {
        // Check duration
        if (p_stream->duration > 0) {
            time_t elapsed = time(NULL) - start_time;
            if (elapsed >= p_stream->duration) {
                break;
            }
        }

        // Pull data from encoder
        ret = hd_audioenc_pull_out_buf(p_stream->aenc_path, &data_pull, 100);

        if (ret == HD_OK && data_pull.size > 0) {
            unsigned char *pcm_ptr = (unsigned char *)(vir_addr + (data_pull.phy_addr - buf_info.buf_info.phy_addr));
            unsigned int pcm_size = data_pull.size;
            unsigned int sample_count = pcm_size / 2;  // 16-bit samples

            // Encode to G.711 A-law
            if (g711_buf_size < sample_count) {
                g711_buf = realloc(g711_buf, sample_count);
                g711_buf_size = sample_count;
            }

            if (g711_buf) {
                encode_alaw((const short *)pcm_ptr, g711_buf, sample_count);
                fwrite(g711_buf, 1, sample_count, f_out);
                fflush(f_out);
                p_stream->audio_bytes += sample_count;
            }

            p_stream->audio_frames++;
            hd_audioenc_release_out_buf(p_stream->aenc_path, &data_pull);
        }
    }

    printf("Audio: recording complete - %lu frames, %.2f KB\n",
           p_stream->audio_frames, p_stream->audio_bytes / 1024.0);

    // Cleanup
    if (g711_buf) free(g711_buf);
    if (f_out) fclose(f_out);
    hd_common_mem_munmap((void *)vir_addr, buf_info.buf_info.buf_size);

    return NULL;
}

// ============================================================================
// Module Management
// ============================================================================

static HD_RESULT init_modules(void)
{
    HD_RESULT ret;

    ret = hd_videocap_init();
    if (ret != HD_OK) return ret;

    ret = hd_videoproc_init();
    if (ret != HD_OK) return ret;

    ret = hd_videoenc_init();
    if (ret != HD_OK) return ret;

    ret = hd_audiocap_init();
    if (ret != HD_OK) return ret;

    ret = hd_audioenc_init();
    if (ret != HD_OK) return ret;

    return HD_OK;
}

static HD_RESULT open_video_modules(AV_RECORD *p_stream)
{
    HD_RESULT ret;

    // Set video capture config
    ret = set_vcap_cfg(&p_stream->vcap_ctrl);
    if (ret != HD_OK) {
        printf("set_vcap_cfg failed: %d\n", ret);
        return ret;
    }

    // Set video processor config
    ret = set_vproc_cfg(&p_stream->vproc_ctrl, &p_stream->vproc_max_dim);
    if (ret != HD_OK) {
        printf("set_vproc_cfg failed: %d\n", ret);
        return ret;
    }

    // Open video paths
    ret = hd_videocap_open(HD_VIDEOCAP_IN(SEN1_VCAP_ID, 0), HD_VIDEOCAP_OUT(SEN1_VCAP_ID, 0), &p_stream->vcap_path);
    if (ret != HD_OK) return ret;

    ret = hd_videoproc_open(HD_VIDEOPROC_0_IN_0, HD_VIDEOPROC_0_OUT_0, &p_stream->vproc_path);
    if (ret != HD_OK) return ret;

    ret = hd_videoenc_open(HD_VIDEOENC_0_IN_0, HD_VIDEOENC_0_OUT_0, &p_stream->venc_path);
    if (ret != HD_OK) return ret;

    return HD_OK;
}

static HD_RESULT open_audio_modules(AV_RECORD *p_stream)
{
    HD_RESULT ret;

    // Set audio capture config
    ret = set_acap_cfg(&p_stream->acap_ctrl);
    if (ret != HD_OK) {
        printf("set_acap_cfg failed: %d\n", ret);
        return ret;
    }

    // Open audio paths
    ret = hd_audiocap_open(HD_AUDIOCAP_0_IN_0, HD_AUDIOCAP_0_OUT_0, &p_stream->acap_path);
    if (ret != HD_OK) return ret;

    ret = hd_audioenc_open(HD_AUDIOENC_0_IN_0, HD_AUDIOENC_0_OUT_0, &p_stream->aenc_path);
    if (ret != HD_OK) return ret;

    return HD_OK;
}

static void close_modules(AV_RECORD *p_stream)
{
    // Close video
    hd_videocap_close(p_stream->vcap_path);
    hd_videoproc_close(p_stream->vproc_path);
    hd_videoenc_close(p_stream->venc_path);

    // Close audio
    hd_audiocap_close(p_stream->acap_path);
    hd_audioenc_close(p_stream->aenc_path);
}

static void exit_modules(void)
{
    hd_videocap_uninit();
    hd_videoproc_uninit();
    hd_videoenc_uninit();
    hd_audiocap_uninit();
    hd_audioenc_uninit();
}

// ============================================================================
// Main
// ============================================================================

static void print_usage(const char *prog)
{
    printf("\n=== AV Record G.711 A-law ===\n");
    printf("Usage: %s [video_codec] [duration]\n", prog);
    printf("  video_codec: 0=H.265 (default), 1=H.264\n");
    printf("  duration:    Recording time in seconds (0=manual stop, default)\n");
    printf("\nOutput files on /mnt/sd/:\n");
    printf("  - av_record_alaw_video.h265 or .h264\n");
    printf("  - av_record_audio_alaw.raw (G.711 A-law, 48kHz stereo)\n");
    printf("\nTo mux with ffmpeg:\n");
    printf("  ffmpeg -i av_record_alaw_video.h265 -f alaw -ar 48000 -ac 2 -i av_record_audio_alaw.raw \\\n");
    printf("         -c:v copy -c:a aac output.mp4\n");
}

int main(int argc, char *argv[])
{
    HD_RESULT ret;
    AV_RECORD stream = {0};
    int key;

    // Parse arguments
    stream.video_codec = 0;  // Default H.265
    stream.duration = 0;     // Default manual stop

    if (argc >= 2) {
        stream.video_codec = atoi(argv[1]);
        if (stream.video_codec > 1) {
            printf("Invalid video codec: %d\n", stream.video_codec);
            print_usage(argv[0]);
            return 1;
        }
    }

    if (argc >= 3) {
        stream.duration = atoi(argv[2]);
        if (stream.duration < 0) stream.duration = 0;
    }

    printf("\n========================================\n");
    printf("=== AV Record with G.711 A-law Audio ===\n");
    printf("========================================\n");
    printf("Video: %s, %dx%d @ 30fps\n",
           stream.video_codec == 0 ? "H.265" : "H.264",
           VDO_SIZE_W, VDO_SIZE_H);
    printf("Audio: G.711 A-law, 48kHz stereo\n");
    printf("Duration: %s\n", stream.duration > 0 ? "" : "manual (press 'q' to stop)");
    if (stream.duration > 0) printf("%d seconds\n", stream.duration);
    printf("========================================\n\n");

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
    ret = init_modules();
    if (ret != HD_OK) {
        printf("init_modules failed: %d\n", ret);
        goto cleanup_mem;
    }

    // Setup video dimensions
    stream.vproc_max_dim.w = VDO_SIZE_W;
    stream.vproc_max_dim.h = VDO_SIZE_H;
    stream.vcap_dim.w = VDO_SIZE_W;
    stream.vcap_dim.h = VDO_SIZE_H;
    stream.venc_dim.w = VDO_SIZE_W;
    stream.venc_dim.h = VDO_SIZE_H;

    // Open video modules
    ret = open_video_modules(&stream);
    if (ret != HD_OK) {
        printf("open_video_modules failed: %d\n", ret);
        goto cleanup_modules;
    }

    // Open audio modules
    ret = open_audio_modules(&stream);
    if (ret != HD_OK) {
        printf("open_audio_modules failed: %d\n", ret);
        goto cleanup_close;
    }

    // Set video capture parameters
    ret = set_vcap_param(stream.vcap_path, &stream.vcap_dim);
    if (ret != HD_OK) {
        printf("set_vcap_param failed: %d\n", ret);
        goto cleanup_close;
    }

    // Set video processor parameters
    ret = set_vproc_param(stream.vproc_path, NULL);
    if (ret != HD_OK) {
        printf("set_vproc_param failed: %d\n", ret);
        goto cleanup_close;
    }

    // Set video encoder config and parameters
    ret = set_venc_cfg(stream.venc_path, &stream.venc_dim, VIDEO_BITRATE);
    if (ret != HD_OK) {
        printf("set_venc_cfg failed: %d\n", ret);
        goto cleanup_close;
    }

    ret = set_venc_param(stream.venc_path, &stream.venc_dim, stream.video_codec, VIDEO_BITRATE);
    if (ret != HD_OK) {
        printf("set_venc_param failed: %d\n", ret);
        goto cleanup_close;
    }

    // Set audio capture parameters
    ret = set_acap_param(stream.acap_path);
    if (ret != HD_OK) {
        printf("set_acap_param failed: %d\n", ret);
        goto cleanup_close;
    }

    // Set audio encoder config and parameters
    ret = set_aenc_cfg(stream.aenc_path);
    if (ret != HD_OK) {
        printf("set_aenc_cfg failed: %d\n", ret);
        goto cleanup_close;
    }

    ret = set_aenc_param(stream.aenc_path);
    if (ret != HD_OK) {
        printf("set_aenc_param failed: %d\n", ret);
        goto cleanup_close;
    }

    // Bind video modules
    hd_videocap_bind(HD_VIDEOCAP_OUT(SEN1_VCAP_ID, 0), HD_VIDEOPROC_0_IN_0);
    hd_videoproc_bind(HD_VIDEOPROC_0_OUT_0, HD_VIDEOENC_0_IN_0);

    // Bind audio modules
    hd_audiocap_bind(HD_AUDIOCAP_0_OUT_0, HD_AUDIOENC_0_IN_0);

    // Create encoding threads
    ret = pthread_create(&stream.video_thread_id, NULL, video_encode_thread, &stream);
    if (ret != 0) {
        printf("Failed to create video thread: %d\n", ret);
        goto cleanup_bind;
    }

    ret = pthread_create(&stream.audio_thread_id, NULL, audio_encode_thread, &stream);
    if (ret != 0) {
        printf("Failed to create audio thread: %d\n", ret);
        stream.exit_flag = 1;
        pthread_join(stream.video_thread_id, NULL);
        goto cleanup_bind;
    }

    // Start video modules
    hd_videocap_start(stream.vcap_path);
    hd_videoproc_start(stream.vproc_path);
    sleep(1);  // Wait for AE/AWB to stabilize
    hd_videoenc_start(stream.venc_path);

    // Start audio modules
    hd_audioenc_start(stream.aenc_path);
    hd_audiocap_start(stream.acap_path);

    // Signal threads to start
    stream.flow_start = 1;

    printf("\nRecording started. Press 'q' to stop.\n\n");

    // Wait for completion
    if (stream.duration > 0) {
        sleep(stream.duration + 1);
        stream.exit_flag = 1;
    } else {
        while (!g_exit_signal) {
            key = getchar();
            if (key == 'q' || key == 'Q' || key == 3) {
                stream.exit_flag = 1;
                break;
            }
        }
    }

    // Wait for threads to finish
    pthread_join(stream.video_thread_id, NULL);
    pthread_join(stream.audio_thread_id, NULL);

    // Stop modules
    hd_videocap_stop(stream.vcap_path);
    hd_videoproc_stop(stream.vproc_path);
    hd_videoenc_stop(stream.venc_path);
    hd_audiocap_stop(stream.acap_path);
    hd_audioenc_stop(stream.aenc_path);

cleanup_bind:
    // Unbind modules
    hd_videocap_unbind(HD_VIDEOCAP_OUT(SEN1_VCAP_ID, 0));
    hd_videoproc_unbind(HD_VIDEOPROC_0_OUT_0);
    hd_audiocap_unbind(HD_AUDIOCAP_0_OUT_0);

cleanup_close:
    close_modules(&stream);

cleanup_modules:
    exit_modules();

cleanup_mem:
    mem_exit();

cleanup_common:
    hd_common_uninit();

    printf("\n========================================\n");
    printf("Recording finished!\n");
    printf("Video: /mnt/sd/av_record_alaw_video.%s\n",
           stream.video_codec == 0 ? "h265" : "h264");
    printf("Audio: /mnt/sd/av_record_audio_alaw.raw\n");
    printf("\nTo mux with ffmpeg:\n");
    printf("  ffmpeg -i /mnt/sd/av_record_alaw_video.%s -f alaw -ar 48000 -ac 2 \\\n",
           stream.video_codec == 0 ? "h265" : "h264");
    printf("         -i /mnt/sd/av_record_audio_alaw.raw -c:v copy -c:a aac /mnt/sd/output.mp4\n");
    printf("========================================\n");

    return 0;
}
