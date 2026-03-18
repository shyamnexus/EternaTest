/**
 * @file vqa_tamper_stream.c
 * @brief Professional Video Tampering Detection - Live Stream Version
 * @author Custom Implementation
 * @date 2026
 * 
 * Live camera tampering detection with:
 * - Configurable sensitivity (0-100)
 * - Dwell time / persistence check
 * - Defocus detection (blur)
 * - Masking/Blocking detection (lens covered)
 * - Scene change detection (camera moved)
 * - Exposure tampering (too dark/bright)
 * - IVE Hardware acceleration
 */

#include <sys/time.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "hdal.h"
#include "hd_debug.h"
#include "libive/libive.h"
#include <kwrap/perf.h>

#if defined(__FREERTOS)
#include <FreeRTOS_POSIX.h>
#include <FreeRTOS_POSIX/pthread.h>
#include <FreeRTOS_POSIX/signal.h>
#include <kwrap/task.h>
#define sleep(x)    vos_task_delay_ms(1000*x)
#define usleep(x)   vos_task_delay_us(x)
#include <kwrap/examsys.h>
#define MAIN(argc, argv)    EXAMFUNC_ENTRY(vqa_tamper_stream, argc, argv)
#else
#include <pthread.h>
#include <signal.h>
#define MAIN(argc, argv)    int main(int argc, char** argv)
#endif

/*============================================================================
 * Video Pipeline Configuration
 *============================================================================*/
#define HD_COMMON_MEM_VQA_POOL HD_COMMON_MEM_USER_DEFINIED_POOL
#define DBGINFO_BUFSIZE()   (0x200)

#define VDO_RAW_BUFSIZE(w, h, pxlfmt)   (ALIGN_CEIL_4((w) * HD_VIDEO_PXLFMT_BPP(pxlfmt) / 8) * (h))
#define RAW_COMPRESS_RATIO 41
#define VDO_NRX_BUFSIZE(w, h)           (ALIGN_CEIL_4(ALIGN_CEIL_64(w) / 64 * ((24*RAW_COMPRESS_RATIO+99)/100) * 4 * (h)))
#define VDO_CA_BUF_SIZE(win_num_w, win_num_h) ALIGN_CEIL_4((win_num_w * win_num_h << 3) << 1)
#define VDO_LA_BUF_SIZE(win_num_w, win_num_h) ALIGN_CEIL_4((win_num_w * win_num_h << 1) << 1)
#define VDO_YUV_BUFSIZE(w, h, pxlfmt)   (ALIGN_CEIL_4((w) * HD_VIDEO_PXLFMT_BPP(pxlfmt) / 8) * (h))

#define SEN_OUT_FMT         HD_VIDEO_PXLFMT_RAW10
#define CAP_OUT_FMT         HD_VIDEO_PXLFMT_RAW10
#define CA_WIN_NUM_W        32
#define CA_WIN_NUM_H        32
#define LA_WIN_NUM_W        32
#define LA_WIN_NUM_H        32

#define VDO_SIZE_W          2960
#define VDO_SIZE_H          1664
#define VDO_FRAME_FORMAT    HD_VIDEO_PXLFMT_YUV420

#define IMG_WIDTH           320
#define IMG_HEIGHT          180
#define IMG_SIZE            (IMG_WIDTH * IMG_HEIGHT)
#define IMG_BUF_SIZE        ((IMG_WIDTH * IMG_HEIGHT) * 3 / 2)

#define SEN1_VCAP_ID        0

/*============================================================================
 * Tampering Detection Configuration
 *============================================================================*/
#define DEFAULT_BLUR_VARIANCE_THRESH    100.0f
#define DEFAULT_MASK_EDGE_DENSITY_THRESH 5.0f
#define DEFAULT_MASK_HISTOGRAM_CONC     80.0f
#define DEFAULT_SCENE_CHANGE_THRESH     30.0f
#define DEFAULT_DARK_THRESH             50.0f
#define DEFAULT_BRIGHT_THRESH           220.0f
#define DEFAULT_DWELL_FRAMES            5

/*============================================================================
 * Tampering Event Types
 *============================================================================*/
typedef enum {
    TAMPER_NONE           = 0x00,
    TAMPER_DEFOCUS        = 0x01,
    TAMPER_MASKING        = 0x02,
    TAMPER_SCENE_CHANGE   = 0x04,
    TAMPER_TOO_DARK       = 0x08,
    TAMPER_TOO_BRIGHT     = 0x10,
} TamperEventType;

/*============================================================================
 * Configuration Structure
 *============================================================================*/
typedef struct {
    int enabled;
    int sensitivity;
    int dwell_frames;
    int detect_defocus;
    int detect_masking;
    int detect_scene_change;
    int detect_dark;
    int detect_bright;
    
    float blur_variance_thresh;
    float mask_edge_density_thresh;
    float mask_histogram_concentration;
    float scene_change_thresh;
    float dark_brightness_thresh;
    float bright_brightness_thresh;
} TamperConfig;

/*============================================================================
 * Detection State
 *============================================================================*/
typedef struct {
    int defocus_count;
    int masking_count;
    int scene_change_count;
    int dark_count;
    int bright_count;
    UINT32 active_alarms;
    
    int reference_valid;
    int frames_since_ref_update;     /* Frames since last reference update */
    float prev_brightness;           /* Previous frame brightness for transition detection */
    float brightness_change_rate;    /* Rate of brightness change (for day/night) */
    int day_night_transition;        /* Flag: gradual lighting change in progress */
    int scene_alarm_persist_count;   /* Frames scene alarm has been active */
    float ref_mean_brightness;
    float ref_laplacian_variance;
    float ref_edge_density;
    UINT32 ref_histogram[256];
} TamperState;

/*============================================================================
 * Frame Analysis Result
 *============================================================================*/
typedef struct {
    float mean_brightness;
    float laplacian_variance;
    float edge_density;
    float histogram_concentration;
    float scene_difference;
    UINT32 histogram[256];
    UINT32 raw_detections;
    UINT32 confirmed_alarms;
    UINT32 process_time_us;
} TamperResult;

/*============================================================================
 * Memory Block
 *============================================================================*/
typedef struct {
    UINTPTR va;
    UINTPTR pa;
    UINT32 size;
    HD_COMMON_MEM_VB_BLK blk;
} MemBlock;

/*============================================================================
 * Video Pipeline Structure
 *============================================================================*/
typedef struct {
    HD_VIDEOCAP_SYSCAPS cap_syscaps;
    HD_PATH_ID cap_ctrl;
    HD_PATH_ID cap_path;
    HD_DIM cap_dim;
    HD_DIM proc_max_dim;
    
    HD_VIDEOPROC_SYSCAPS proc_syscaps;
    HD_PATH_ID proc_ctrl;
    HD_PATH_ID proc_path;
    
    HD_DIM out_max_dim;
    HD_DIM out_dim;
} VIDEO_STREAM;

/*============================================================================
 * Global Variables
 *============================================================================*/
static VIDEO_STREAM g_stream = {0};
static TamperConfig g_config;
static TamperState g_state;
static volatile UINT32 g_running = 1;
static MemBlock g_hist_buf = {0};
static MemBlock g_sobel_buf = {0};

/*============================================================================
 * Configuration Helpers
 *============================================================================*/
static void apply_sensitivity(TamperConfig *cfg)
{
    float sens_factor = (100 - cfg->sensitivity) / 50.0f;
    if (sens_factor < 0.1f) sens_factor = 0.1f;
    
    cfg->blur_variance_thresh = DEFAULT_BLUR_VARIANCE_THRESH * sens_factor;
    cfg->mask_edge_density_thresh = DEFAULT_MASK_EDGE_DENSITY_THRESH * sens_factor;
    cfg->mask_histogram_concentration = DEFAULT_MASK_HISTOGRAM_CONC - (cfg->sensitivity * 0.3f);
    cfg->scene_change_thresh = DEFAULT_SCENE_CHANGE_THRESH * sens_factor;
    cfg->dark_brightness_thresh = DEFAULT_DARK_THRESH + (cfg->sensitivity * 0.2f);
    cfg->bright_brightness_thresh = DEFAULT_BRIGHT_THRESH - (cfg->sensitivity * 0.2f);
    
    if (cfg->mask_histogram_concentration < 50.0f) cfg->mask_histogram_concentration = 50.0f;
    if (cfg->dark_brightness_thresh > 80.0f) cfg->dark_brightness_thresh = 80.0f;
    if (cfg->bright_brightness_thresh < 180.0f) cfg->bright_brightness_thresh = 180.0f;
}

static void init_default_config(TamperConfig *cfg)
{
    memset(cfg, 0, sizeof(*cfg));
    cfg->enabled = 1;
    cfg->sensitivity = 50;
    cfg->dwell_frames = DEFAULT_DWELL_FRAMES;
    cfg->detect_defocus = 1;
    cfg->detect_masking = 1;
    cfg->detect_scene_change = 1;
    cfg->detect_dark = 1;
    cfg->detect_bright = 1;
    apply_sensitivity(cfg);
}

/*============================================================================
 * Memory Management
 *============================================================================*/
static HD_RESULT mem_init(void)
{
    HD_RESULT ret;
    HD_COMMON_MEM_INIT_CONFIG mem_cfg = {0};

    /* Capture pool */
    mem_cfg.pool_info[0].type = HD_COMMON_MEM_COMMON_POOL;
    mem_cfg.pool_info[0].blk_size = DBGINFO_BUFSIZE() +
        VDO_RAW_BUFSIZE(VDO_SIZE_W, VDO_SIZE_H, CAP_OUT_FMT) +
        VDO_CA_BUF_SIZE(CA_WIN_NUM_W, CA_WIN_NUM_H) +
        VDO_LA_BUF_SIZE(LA_WIN_NUM_W, LA_WIN_NUM_H);
    mem_cfg.pool_info[0].blk_cnt = 2;
    mem_cfg.pool_info[0].ddr_id = DDR_ID0;

    /* Video proc pool */
    mem_cfg.pool_info[1].type = HD_COMMON_MEM_COMMON_POOL;
    mem_cfg.pool_info[1].blk_size = DBGINFO_BUFSIZE() + VDO_YUV_BUFSIZE(VDO_SIZE_W, VDO_SIZE_H, VDO_FRAME_FORMAT);
    mem_cfg.pool_info[1].blk_cnt = 3;
    mem_cfg.pool_info[1].ddr_id = DDR_ID0;

    /* VQA/Tamper detection pool */
    mem_cfg.pool_info[2].type = HD_COMMON_MEM_VQA_POOL;
    mem_cfg.pool_info[2].blk_size = IMG_BUF_SIZE;
    mem_cfg.pool_info[2].blk_cnt = 2;
    mem_cfg.pool_info[2].ddr_id = DDR_ID0;

    /* Histogram buffer pool */
    mem_cfg.pool_info[3].type = HD_COMMON_MEM_VQA_POOL;
    mem_cfg.pool_info[3].blk_size = 256 * sizeof(UINT32);
    mem_cfg.pool_info[3].blk_cnt = 1;
    mem_cfg.pool_info[3].ddr_id = DDR_ID0;

    /* Sobel output pool */
    mem_cfg.pool_info[4].type = HD_COMMON_MEM_VQA_POOL;
    mem_cfg.pool_info[4].blk_size = IMG_SIZE * 2;
    mem_cfg.pool_info[4].blk_cnt = 1;
    mem_cfg.pool_info[4].ddr_id = DDR_ID0;

    ret = hd_common_mem_init(&mem_cfg);
    return ret;
}

static HD_RESULT alloc_mem_block(MemBlock *blk, UINT32 size)
{
    blk->blk = hd_common_mem_get_block(HD_COMMON_MEM_VQA_POOL, size, DDR_ID0);
    if (blk->blk == HD_COMMON_MEM_VB_INVALID_BLK) return HD_ERR_NG;
    
    blk->pa = hd_common_mem_blk2pa(blk->blk);
    if (blk->pa == 0) {
        hd_common_mem_release_block(blk->blk);
        return HD_ERR_NG;
    }
    
    blk->va = (UINTPTR)hd_common_mem_mmap(HD_COMMON_MEM_MEM_TYPE_CACHE, blk->pa, size);
    if (blk->va == 0) {
        hd_common_mem_release_block(blk->blk);
        return HD_ERR_NG;
    }
    
    blk->size = size;
    return HD_OK;
}

static void free_mem_block(MemBlock *blk)
{
    if (blk->va) hd_common_mem_munmap((void*)blk->va, blk->size);
    if (blk->blk != HD_COMMON_MEM_VB_INVALID_BLK) hd_common_mem_release_block(blk->blk);
    memset(blk, 0, sizeof(*blk));
}

/*============================================================================
 * IVE Hardware Functions
 *============================================================================*/
static HD_RESULT ive_calc_histogram(UINTPTR src_pa, UINTPTR src_va, UINT32 *histogram)
{
    IVE_SRC_IMAGE_S src = {0};
    IVE_DST_IMAGE_S dst = {0};
    UINT32 handle = 0;
    HD_RESULT ret;
    
    src.enDataType = LIB_IVE_DATA_TYPE_U8C1;
    src.u64Pa = src_pa;
    src.u64Va = src_va;
    src.u32Width = IMG_WIDTH;
    src.u32Height = IMG_HEIGHT;
    src.u32Stride = IMG_WIDTH;
    
    dst.enDataType = LIB_IVE_DATA_TYPE_U32C1;
    dst.u64Pa = g_hist_buf.pa;
    dst.u64Va = g_hist_buf.va;
    dst.u32Width = 256;
    dst.u32Height = 1;
    dst.u32Stride = 256 * 4;
    
    hd_common_mem_flush_cache((void*)src.u64Va, IMG_SIZE);
    
    ret = NVT_IVE_Histo(&handle, &src, &dst, 1);
    
    if (ret == HD_OK) {
        hd_common_mem_flush_cache((void*)dst.u64Va, 256 * 4);
        memcpy(histogram, (void*)g_hist_buf.va, 256 * sizeof(UINT32));
    }
    
    return ret;
}

static HD_RESULT ive_calc_sobel(UINTPTR src_pa, UINTPTR src_va)
{
    IVE_SRC_IMAGE_S src = {0};
    IVE_DST_IMAGE_S dst = {0};
    IVE_SOBEL_CTRL_S ctrl = {0};
    UINT32 handle = 0;
    
    src.enDataType = LIB_IVE_DATA_TYPE_U8C1;
    src.u64Pa = src_pa;
    src.u64Va = src_va;
    src.u32Width = IMG_WIDTH;
    src.u32Height = IMG_HEIGHT;
    src.u32Stride = IMG_WIDTH;
    
    dst.enDataType = LIB_IVE_DATA_TYPE_S16C1;
    dst.u64Pa = g_sobel_buf.pa;
    dst.u64Va = g_sobel_buf.va;
    dst.u32Width = IMG_WIDTH;
    dst.u32Height = IMG_HEIGHT;
    dst.u32Stride = IMG_WIDTH * 2;
    
    ctrl.s8Mask[6] = -1; ctrl.s8Mask[7] = 0; ctrl.s8Mask[8] = 1;
    ctrl.s8Mask[11] = -2; ctrl.s8Mask[12] = 0; ctrl.s8Mask[13] = 2;
    ctrl.s8Mask[16] = -1; ctrl.s8Mask[17] = 0; ctrl.s8Mask[18] = 1;
    ctrl.gradient_out_format = 0;
    
    hd_common_mem_flush_cache((void*)src.u64Va, IMG_SIZE);
    
    HD_RESULT ret = NVT_IVE_Sobel(&handle, &src, &dst, &ctrl, 1);
    
    hd_common_mem_flush_cache((void*)dst.u64Va, IMG_SIZE * 2);
    
    return ret;
}

/*============================================================================
 * Analysis Functions
 *============================================================================*/
static float calc_laplacian_variance(const UINT8 *y_plane)
{
    double sum = 0.0, sum_sq = 0.0;
    int count = 0;
    
    for (int y = 1; y < IMG_HEIGHT - 1; y++) {
        for (int x = 1; x < IMG_WIDTH - 1; x++) {
            int idx = y * IMG_WIDTH + x;
            int lap = -4 * y_plane[idx]
                    + y_plane[idx - IMG_WIDTH]
                    + y_plane[idx + IMG_WIDTH]
                    + y_plane[idx - 1]
                    + y_plane[idx + 1];
            sum += lap;
            sum_sq += (double)lap * lap;
            count++;
        }
    }
    
    if (count == 0) return 0.0f;
    double mean = sum / count;
    return (float)((sum_sq / count) - (mean * mean));
}

static float calc_edge_density(const INT16 *sobel)
{
    int edge_count = 0;
    int total = IMG_WIDTH * IMG_HEIGHT;
    const INT16 thresh = 50;
    
    for (int i = 0; i < total; i++) {
        if (abs(sobel[i]) > thresh) edge_count++;
    }
    
    return 100.0f * edge_count / total;
}

static void analyze_histogram(const UINT32 *histogram, float *mean_brightness, float *concentration)
{
    UINT64 total = 0, weighted_sum = 0;
    
    for (int i = 0; i < 256; i++) {
        total += histogram[i];
        weighted_sum += (UINT64)i * histogram[i];
    }
    
    *mean_brightness = (total > 0) ? (float)weighted_sum / total : 0;
    
    UINT64 max_concentration = 0;
    for (int start = 0; start < 256 - 32; start += 8) {
        UINT64 range_sum = 0;
        for (int i = start; i < start + 32 && i < 256; i++) {
            range_sum += histogram[i];
        }
        if (range_sum > max_concentration) {
            max_concentration = range_sum;
        }
    }
    
    *concentration = (total > 0) ? 100.0f * max_concentration / total : 0;
}

static float calc_scene_difference(const UINT32 *current, const UINT32 *reference,
                                   float curr_brightness, float ref_brightness,
                                   float curr_edge, float ref_edge)
{
    /* 
     * Scene change detection focuses on STRUCTURAL changes, not lighting.
     * Key insight: Camera tampering (moved/covered) changes edge distribution,
     * while day/night just shifts brightness histogram uniformly.
     */
    
    /* 1. Histogram comparison (raw chi-square for sensitivity) */
    float chi_sq = 0.0f;
    for (int i = 0; i < 256; i++) {
        float c = (float)current[i];
        float r = (float)reference[i];
        if (c + r > 0) {
            chi_sq += (c - r) * (c - r) / (c + r);
        }
    }
    chi_sq /= 256.0f;  /* Normalize by bins */
    
    /* 2. Edge structure difference - indicates camera moved/scene changed */
    float edge_diff = fabsf(curr_edge - ref_edge);
    
    /* 3. Brightness difference (reduced weight for day/night robustness) */
    float brightness_diff = fabsf(curr_brightness - ref_brightness) / 255.0f * 100.0f;
    
    /* 
     * Combined score: chi-square dominates for histogram changes,
     * edge_diff adds structural info, brightness has low weight
     */
    return chi_sq * 8.0f + edge_diff * 0.5f + brightness_diff * 0.2f;
}

/*============================================================================
 * Adaptive Reference Update
 * Slowly adapts to gradual lighting changes (day/night) while detecting
 * sudden tampering events
 *============================================================================*/
#define ADAPTIVE_REF_INTERVAL   300   /* Update reference every 10 sec (30fps) */
#define GRADUAL_CHANGE_THRESH   1.0f  /* Brightness change per frame threshold */
#define ADAPTIVE_BLEND_FACTOR   0.02f /* Slow blend for reference update */

static void update_adaptive_reference(TamperResult *result, TamperState *state)
{
    state->frames_since_ref_update++;
    
    /* Detect gradual brightness change (day/night transition) */
    if (state->prev_brightness > 0) {
        float brightness_delta = result->mean_brightness - state->prev_brightness;
        
        /* Exponential moving average of brightness change rate */
        state->brightness_change_rate = 0.9f * state->brightness_change_rate + 
                                        0.1f * fabsf(brightness_delta);
        
        /* If brightness is changing gradually (< 1 unit/frame), it's day/night */
        state->day_night_transition = (state->brightness_change_rate > 0.1f && 
                                       state->brightness_change_rate < GRADUAL_CHANGE_THRESH);
    }
    state->prev_brightness = result->mean_brightness;
    
    /* Adaptive reference update - only if NO alarms active and scene is stable */
    if (state->active_alarms == TAMPER_NONE && 
        state->frames_since_ref_update >= ADAPTIVE_REF_INTERVAL) {
        
        /* Only update if scene is similar (not during actual tampering) */
        if (result->scene_difference < 20.0f) {
            /* Blend new reference with old (slow adaptation) */
            state->ref_mean_brightness = (1.0f - ADAPTIVE_BLEND_FACTOR) * state->ref_mean_brightness +
                                         ADAPTIVE_BLEND_FACTOR * result->mean_brightness;
            state->ref_edge_density = (1.0f - ADAPTIVE_BLEND_FACTOR) * state->ref_edge_density +
                                      ADAPTIVE_BLEND_FACTOR * result->edge_density;
            
            /* Blend histogram */
            for (int i = 0; i < 256; i++) {
                state->ref_histogram[i] = (UINT32)((1.0f - ADAPTIVE_BLEND_FACTOR) * state->ref_histogram[i] +
                                                    ADAPTIVE_BLEND_FACTOR * result->histogram[i]);
            }
            
            state->frames_since_ref_update = 0;
        }
    }
}

/*============================================================================
 * Main Tampering Detection
 *============================================================================*/
static HD_RESULT detect_tampering(UINTPTR y_pa, UINTPTR y_va, TamperResult *result)
{
    HD_RESULT ret;
    VOS_TICK t_start, t_end;
    TamperConfig *cfg = &g_config;
    TamperState *state = &g_state;
    
    memset(result, 0, sizeof(*result));
    
    if (!cfg->enabled) return HD_OK;
    
    vos_perf_mark(&t_start);
    
    /* 1. Hardware-accelerated Histogram */
    ret = ive_calc_histogram(y_pa, y_va, result->histogram);
    if (ret != HD_OK) return ret;
    
    analyze_histogram(result->histogram, &result->mean_brightness, &result->histogram_concentration);
    
    /* 2. Hardware-accelerated Sobel */
    ret = ive_calc_sobel(y_pa, y_va);
    if (ret != HD_OK) return ret;
    
    result->edge_density = calc_edge_density((INT16*)g_sobel_buf.va);
    
    /* 3. CPU Laplacian variance */
    result->laplacian_variance = calc_laplacian_variance((UINT8*)y_va);
    
    /* 4. Scene difference */
    if (state->reference_valid) {
        result->scene_difference = calc_scene_difference(
            result->histogram, state->ref_histogram,
            result->mean_brightness, state->ref_mean_brightness,
            result->edge_density, state->ref_edge_density);
        
        /* Update adaptive reference for day/night handling */
        update_adaptive_reference(result, state);
    }
    
    vos_perf_mark(&t_end);
    result->process_time_us = vos_perf_duration(t_start, t_end);
    
    /* Detection Logic */
    result->raw_detections = TAMPER_NONE;
    
    if (cfg->detect_defocus && result->laplacian_variance < cfg->blur_variance_thresh) {
        result->raw_detections |= TAMPER_DEFOCUS;
    }
    
    if (cfg->detect_masking) {
        if (result->edge_density < cfg->mask_edge_density_thresh &&
            result->histogram_concentration > cfg->mask_histogram_concentration) {
            result->raw_detections |= TAMPER_MASKING;
        }
    }
    
    /* Scene change: Only if edge structure changed significantly
     * Suppressed during day/night transitions */
    if (cfg->detect_scene_change && state->reference_valid) {
        float edge_change = fabsf(result->edge_density - state->ref_edge_density);
        
        /* Only trigger if:
         * 1. Overall scene score exceeds threshold AND
         * 2. Edge structure changed (not just brightness) OR histogram changed significantly AND
         * 3. NOT during gradual day/night transition */
        int structural_change = (edge_change > 2.0f) || (result->scene_difference > cfg->scene_change_thresh * 1.5f);
        
        if (result->scene_difference > cfg->scene_change_thresh &&
            structural_change &&
            !state->day_night_transition) {
            result->raw_detections |= TAMPER_SCENE_CHANGE;
        }
    }
    
    /* Dark/Bright detection: Suppressed during day/night transitions */
    if (cfg->detect_dark && result->mean_brightness < cfg->dark_brightness_thresh) {
        if (!state->day_night_transition) {
            result->raw_detections |= TAMPER_TOO_DARK;
        }
    }
    
    if (cfg->detect_bright && result->mean_brightness > cfg->bright_brightness_thresh) {
        if (!state->day_night_transition) {
            result->raw_detections |= TAMPER_TOO_BRIGHT;
        }
    }
    
    /* Dwell Time Logic - REQUIRES CONSECUTIVE FRAMES */
    #define UPDATE_DWELL(type, counter) do { \
        if (result->raw_detections & type) { \
            state->counter++; \
            if (state->counter >= cfg->dwell_frames) { \
                result->confirmed_alarms |= type; \
            } \
        } else { \
            state->counter = 0; \
        } \
    } while(0)
    
    result->confirmed_alarms = TAMPER_NONE;
    
    UPDATE_DWELL(TAMPER_DEFOCUS, defocus_count);
    UPDATE_DWELL(TAMPER_MASKING, masking_count);
    UPDATE_DWELL(TAMPER_SCENE_CHANGE, scene_change_count);
    UPDATE_DWELL(TAMPER_TOO_DARK, dark_count);
    UPDATE_DWELL(TAMPER_TOO_BRIGHT, bright_count);
    
    state->active_alarms = result->confirmed_alarms;
    
    return HD_OK;
}

static void set_reference_frame(TamperResult *result)
{
    TamperState *state = &g_state;
    
    memcpy(state->ref_histogram, result->histogram, sizeof(state->ref_histogram));
    state->ref_mean_brightness = result->mean_brightness;
    state->ref_laplacian_variance = result->laplacian_variance;
    state->ref_edge_density = result->edge_density;
    state->reference_valid = 1;
    
    /* Initialize adaptive reference tracking */
    state->frames_since_ref_update = 0;
    state->prev_brightness = result->mean_brightness;
    state->brightness_change_rate = 0.0f;
    state->day_night_transition = 0;
    
    printf("Reference captured: Brightness=%.1f, Variance=%.1f, EdgeDensity=%.1f%%\n",
           state->ref_mean_brightness, state->ref_laplacian_variance, state->ref_edge_density);
}

/*============================================================================
 * Video Pipeline Setup
 *============================================================================*/
static HD_RESULT set_cap_cfg(HD_PATH_ID *p_video_cap_ctrl)
{
    HD_RESULT ret = HD_OK;
    HD_VIDEOCAP_DRV_CONFIG cap_cfg = {0};
    HD_PATH_ID video_cap_ctrl = 0;
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

    ret = hd_videocap_open(0, HD_VIDEOCAP_CTRL(SEN1_VCAP_ID), &video_cap_ctrl);
    if (ret != HD_OK) return ret;

    ret |= hd_videocap_set(video_cap_ctrl, HD_VIDEOCAP_PARAM_DRV_CONFIG, &cap_cfg);
    iq_ctl.func = HD_VIDEOCAP_FUNC_AE | HD_VIDEOCAP_FUNC_AWB;
    ret |= hd_videocap_set(video_cap_ctrl, HD_VIDEOCAP_PARAM_CTRL, &iq_ctl);
    
    *p_video_cap_ctrl = video_cap_ctrl;
    return ret;
}

static HD_RESULT set_cap_param(HD_PATH_ID video_cap_path, HD_DIM *p_dim)
{
    HD_RESULT ret = HD_OK;
    
    HD_VIDEOCAP_IN video_in_param = {0};
    video_in_param.sen_mode = HD_VIDEOCAP_SEN_MODE_AUTO;
    video_in_param.frc = HD_VIDEO_FRC_RATIO(30, 1);
    video_in_param.dim.w = p_dim->w;
    video_in_param.dim.h = p_dim->h;
    video_in_param.pxlfmt = SEN_OUT_FMT;
    video_in_param.out_frame_num = HD_VIDEOCAP_SEN_FRAME_NUM_1;
    ret = hd_videocap_set(video_cap_path, HD_VIDEOCAP_PARAM_IN, &video_in_param);
    if (ret != HD_OK) return ret;

    HD_VIDEOCAP_CROP video_crop_param = {0};
    video_crop_param.mode = HD_CROP_OFF;
    ret = hd_videocap_set(video_cap_path, HD_VIDEOCAP_PARAM_IN_CROP, &video_crop_param);

    HD_VIDEOCAP_OUT video_out_param = {0};
    video_out_param.pxlfmt = CAP_OUT_FMT;
    video_out_param.dir = HD_VIDEO_DIR_NONE;
    ret = hd_videocap_set(video_cap_path, HD_VIDEOCAP_PARAM_OUT, &video_out_param);

    return ret;
}

static HD_RESULT set_proc_cfg(HD_PATH_ID *p_video_proc_ctrl, HD_DIM *p_max_dim)
{
    HD_RESULT ret = HD_OK;
    HD_VIDEOPROC_DEV_CONFIG video_cfg_param = {0};
    HD_VIDEOPROC_CTRL video_ctrl_param = {0};
    HD_PATH_ID video_proc_ctrl = 0;

    ret = hd_videoproc_open(0, HD_VIDEOPROC_0_CTRL, &video_proc_ctrl);
    if (ret != HD_OK) return ret;

    video_cfg_param.pipe = HD_VIDEOPROC_PIPE_RAWALL;
    video_cfg_param.isp_id = SEN1_VCAP_ID;
    video_cfg_param.ctrl_max.func = 0;
    video_cfg_param.in_max.func = 0;
    video_cfg_param.in_max.dim.w = p_max_dim->w;
    video_cfg_param.in_max.dim.h = p_max_dim->h;
    video_cfg_param.in_max.pxlfmt = CAP_OUT_FMT;
    video_cfg_param.in_max.frc = HD_VIDEO_FRC_RATIO(1, 1);
    ret = hd_videoproc_set(video_proc_ctrl, HD_VIDEOPROC_PARAM_DEV_CONFIG, &video_cfg_param);
    if (ret != HD_OK) return HD_ERR_NG;

    video_ctrl_param.func = 0;
    ret = hd_videoproc_set(video_proc_ctrl, HD_VIDEOPROC_PARAM_CTRL, &video_ctrl_param);
    
    *p_video_proc_ctrl = video_proc_ctrl;
    return ret;
}

static HD_RESULT set_proc_param(HD_PATH_ID video_proc_path, HD_DIM *p_dim)
{
    HD_RESULT ret = HD_OK;
    HD_VIDEOPROC_OUT video_out_param = {0};
    
    video_out_param.func = 0;
    video_out_param.dim.w = p_dim->w;
    video_out_param.dim.h = p_dim->h;
    video_out_param.pxlfmt = VDO_FRAME_FORMAT;
    video_out_param.dir = HD_VIDEO_DIR_NONE;
    video_out_param.frc = HD_VIDEO_FRC_RATIO(1, 1);
    video_out_param.depth = 1;
    
    ret = hd_videoproc_set(video_proc_path, HD_VIDEOPROC_PARAM_OUT, &video_out_param);
    return ret;
}

/*============================================================================
 * Output Helpers
 *============================================================================*/
static const char* get_alarm_string(UINT32 alarms)
{
    static char buf[256];
    buf[0] = '\0';
    
    if (alarms == TAMPER_NONE) {
        strcpy(buf, "NONE");
        return buf;
    }
    
    if (alarms & TAMPER_DEFOCUS) strcat(buf, "DEFOCUS ");
    if (alarms & TAMPER_MASKING) strcat(buf, "MASKING ");
    if (alarms & TAMPER_SCENE_CHANGE) strcat(buf, "SCENE_CHANGE ");
    if (alarms & TAMPER_TOO_DARK) strcat(buf, "TOO_DARK ");
    if (alarms & TAMPER_TOO_BRIGHT) strcat(buf, "TOO_BRIGHT ");
    
    return buf;
}

static void print_config(void)
{
    printf("\n========== Tampering Detection Config ==========\n");
    printf("Sensitivity:    %d/100\n", g_config.sensitivity);
    printf("Dwell Frames:   %d\n", g_config.dwell_frames);
    printf("Detection: Defocus=%s Masking=%s Scene=%s Dark=%s Bright=%s\n",
           g_config.detect_defocus ? "ON" : "OFF",
           g_config.detect_masking ? "ON" : "OFF",
           g_config.detect_scene_change ? "ON" : "OFF",
           g_config.detect_dark ? "ON" : "OFF",
           g_config.detect_bright ? "ON" : "OFF");
    printf("Thresholds: Blur<%.0f EdgeDens<%.0f%% HistConc>%.0f%% Scene>%.0f\n",
           g_config.blur_variance_thresh,
           g_config.mask_edge_density_thresh,
           g_config.mask_histogram_concentration,
           g_config.scene_change_thresh);
    printf("Exposure: Dark<%.0f Bright>%.0f\n",
           g_config.dark_brightness_thresh,
           g_config.bright_brightness_thresh);
    printf("=================================================\n\n");
}

static void print_usage(const char *prog)
{
    printf("Usage: %s [options]\n\n", prog);
    printf("Options:\n");
    printf("  -s <0-100>    Sensitivity (default: 50)\n");
    printf("  -d <frames>   Dwell time in frames (default: 5)\n");
    printf("  --no-defocus  Disable defocus detection\n");
    printf("  --no-masking  Disable masking detection\n");
    printf("  --no-scene    Disable scene change detection\n");
    printf("  --no-dark     Disable dark detection\n");
    printf("  --no-bright   Disable bright detection\n");
    printf("  -h            Show this help\n");
    printf("\nPress 'q' to quit during runtime.\n");
}

/*============================================================================
 * Signal Handler
 *============================================================================*/
#if defined(__LINUX)
static void sig_handler(int sig)
{
    printf("\nReceived signal %d, stopping...\n", sig);
    g_running = 0;
}
#endif

/*============================================================================
 * Tampering Detection Thread
 *============================================================================*/
static void *tamper_thread(void *arg)
{
    HD_VIDEO_FRAME video_frame = {0};
    HD_RESULT ret;
    TamperResult result;
    int frame_count = 0;
    int skip_frames = 30;  /* Skip initial unstable frames */
    int is_reference_set = 0;
    UINT32 prev_alarms = TAMPER_NONE;  /* Track previous alarm state */
    
    (void)arg;
    
    printf("Tampering detection thread started...\n");
    printf("Skipping first %d frames for sensor stabilization...\n", skip_frames);
    
    while (g_running) {
        /* Pull frame from video processor */
        ret = hd_videoproc_pull_out_buf(g_stream.proc_path, &video_frame, -1);
        if (ret != HD_OK) {
            printf("pull_out_buf failed: %d\n", ret);
            continue;
        }
        
        frame_count++;
        
        /* Skip initial frames */
        if (frame_count <= skip_frames) {
            hd_videoproc_release_out_buf(g_stream.proc_path, &video_frame);
            continue;
        }
        
        /* Map frame memory */
        UINTPTR y_va = (UINTPTR)hd_common_mem_mmap(
            HD_COMMON_MEM_MEM_TYPE_CACHE,
            video_frame.phy_addr[0],
            IMG_SIZE);
        
        if (y_va == 0) {
            printf("mmap failed!\n");
            hd_videoproc_release_out_buf(g_stream.proc_path, &video_frame);
            continue;
        }
        
        /* Run tampering detection */
        ret = detect_tampering(video_frame.phy_addr[0], y_va, &result);
        
        if (ret == HD_OK) {
            /* Set reference on first valid frame */
            if (!is_reference_set) {
                set_reference_frame(&result);
                is_reference_set = 1;
            }
            
            /* Auto-reset reference after scene change alarm persists
             * This accepts the new scene as baseline after 5 seconds
             * 
             * IMPORTANT: Do NOT auto-reset if lens is covered/masked or too dark!
             * Those are active tampering events that should keep alarming.
             * Only reset for pure scene change (camera repositioned to valid new view)
             */
            #define SCENE_ALARM_RESET_FRAMES 150  /* 5 seconds at 30fps */
            int is_pure_scene_change = (result.confirmed_alarms & TAMPER_SCENE_CHANGE) &&
                                       !(result.confirmed_alarms & TAMPER_MASKING) &&
                                       !(result.confirmed_alarms & TAMPER_TOO_DARK) &&
                                       !(result.confirmed_alarms & TAMPER_DEFOCUS);
            
            if (is_pure_scene_change) {
                g_state.scene_alarm_persist_count++;
                if (g_state.scene_alarm_persist_count >= SCENE_ALARM_RESET_FRAMES) {
                    printf(">>> Scene change persisted for 5s, accepting new scene as reference...\n");
                    set_reference_frame(&result);
                    g_state.scene_alarm_persist_count = 0;
                }
            } else {
                g_state.scene_alarm_persist_count = 0;
            }
            
            /* Print status every ~1 second (30 frames) */
            if ((frame_count % 30) == 0) {
                printf("[Frame %d] Bright=%.0f Blur=%.0f Edge=%.1f%% Scene=%.1f | Raw: %s | ALARM: %s",
                       frame_count,
                       result.mean_brightness,
                       result.laplacian_variance,
                       result.edge_density,
                       result.scene_difference,
                       get_alarm_string(result.raw_detections),
                       get_alarm_string(result.confirmed_alarms));
                
                if (result.confirmed_alarms != TAMPER_NONE) {
                    printf(" <<< TAMPERING DETECTED!");
                }
                printf(" (%u us)\n", result.process_time_us);
            }
            
            /* Print alarm only on state CHANGE (new alarm triggered or cleared) */
            if (result.confirmed_alarms != prev_alarms) {
                if (result.confirmed_alarms != TAMPER_NONE) {
                    printf(">>> TAMPERING ALARM TRIGGERED: %s (Frame %d)\n", 
                           get_alarm_string(result.confirmed_alarms), frame_count);
                } else if (prev_alarms != TAMPER_NONE) {
                    printf(">>> TAMPERING ALARM CLEARED (Frame %d)\n", frame_count);
                }
                prev_alarms = result.confirmed_alarms;
            }
        }
        
        hd_common_mem_munmap((void*)y_va, IMG_SIZE);
        hd_videoproc_release_out_buf(g_stream.proc_path, &video_frame);
        
        /* ~30fps rate limiting */
        usleep(33000);
    }
    
    printf("Tampering detection thread stopped.\n");
    return NULL;
}

/*============================================================================
 * Main Entry
 *============================================================================*/
MAIN(argc, argv)
{
    HD_RESULT ret;
    pthread_t thread_id;
    
    printf("=================================================\n");
    printf("  Professional Video Tampering Detection\n");
    printf("  Live Stream Version with IVE Acceleration\n");
    printf("  Resolution: %dx%d @ 30fps\n", IMG_WIDTH, IMG_HEIGHT);
    printf("=================================================\n");
    
    /* Initialize config */
    init_default_config(&g_config);
    memset(&g_state, 0, sizeof(g_state));
    
    /* Parse arguments */
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-s") == 0 && i + 1 < argc) {
            g_config.sensitivity = atoi(argv[++i]);
            if (g_config.sensitivity < 0) g_config.sensitivity = 0;
            if (g_config.sensitivity > 100) g_config.sensitivity = 100;
            apply_sensitivity(&g_config);
        } else if (strcmp(argv[i], "-d") == 0 && i + 1 < argc) {
            g_config.dwell_frames = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--no-defocus") == 0) {
            g_config.detect_defocus = 0;
        } else if (strcmp(argv[i], "--no-masking") == 0) {
            g_config.detect_masking = 0;
        } else if (strcmp(argv[i], "--no-scene") == 0) {
            g_config.detect_scene_change = 0;
        } else if (strcmp(argv[i], "--no-dark") == 0) {
            g_config.detect_dark = 0;
        } else if (strcmp(argv[i], "--no-bright") == 0) {
            g_config.detect_bright = 0;
        } else if (strcmp(argv[i], "-h") == 0) {
            print_usage(argv[0]);
            return 0;
        }
    }
    
    print_config();
    
#if defined(__LINUX)
    signal(SIGINT, sig_handler);
    signal(SIGTERM, sig_handler);
#endif
    
    /* Initialize HDAL */
    ret = hd_common_init(0);
    if (ret != HD_OK) {
        printf("hd_common_init failed: %d\n", ret);
        return -1;
    }
    
    /* Initialize memory */
    ret = mem_init();
    if (ret != HD_OK) {
        printf("mem_init failed\n");
        goto exit_common;
    }
    
    /* Allocate IVE buffers */
    ret = alloc_mem_block(&g_hist_buf, 256 * sizeof(UINT32));
    if (ret != HD_OK) {
        printf("Failed to allocate histogram buffer\n");
        goto exit_mem;
    }
    
    ret = alloc_mem_block(&g_sobel_buf, IMG_SIZE * 2);
    if (ret != HD_OK) {
        printf("Failed to allocate sobel buffer\n");
        goto exit_mem;
    }
    
    /* Initialize IVE */
    ret = vendor_ive_init();
    if (ret != HD_OK) {
        printf("vendor_ive_init failed\n");
        goto exit_mem;
    }
    
    /* Initialize video modules */
    ret = hd_videocap_init();
    if (ret != HD_OK) {
        printf("hd_videocap_init failed: %d\n", ret);
        goto exit_ive;
    }
    
    ret = hd_videoproc_init();
    if (ret != HD_OK) {
        printf("hd_videoproc_init failed: %d\n", ret);
        goto exit_cap_uninit;
    }
    
    /* Setup video capture */
    g_stream.cap_dim.w = VDO_SIZE_W;
    g_stream.cap_dim.h = VDO_SIZE_H;
    g_stream.proc_max_dim.w = VDO_SIZE_W;
    g_stream.proc_max_dim.h = VDO_SIZE_H;
    g_stream.out_dim.w = IMG_WIDTH;
    g_stream.out_dim.h = IMG_HEIGHT;
    
    ret = set_cap_cfg(&g_stream.cap_ctrl);
    if (ret != HD_OK) {
        printf("set_cap_cfg failed\n");
        goto exit_proc_uninit;
    }
    
    ret = hd_videocap_open(HD_VIDEOCAP_IN(SEN1_VCAP_ID, 0), HD_VIDEOCAP_OUT(SEN1_VCAP_ID, 0), &g_stream.cap_path);
    if (ret != HD_OK) {
        printf("videocap open failed\n");
        goto exit_cap_ctrl;
    }
    
    ret = set_cap_param(g_stream.cap_path, &g_stream.cap_dim);
    if (ret != HD_OK) {
        printf("set_cap_param failed\n");
        goto exit_cap;
    }
    
    /* Setup video processor */
    ret = set_proc_cfg(&g_stream.proc_ctrl, &g_stream.proc_max_dim);
    if (ret != HD_OK) {
        printf("set_proc_cfg failed\n");
        goto exit_cap;
    }
    
    ret = hd_videoproc_open(HD_VIDEOPROC_0_IN_0, HD_VIDEOPROC_0_OUT_0, &g_stream.proc_path);
    if (ret != HD_OK) {
        printf("videoproc open failed\n");
        goto exit_proc_ctrl;
    }
    
    ret = set_proc_param(g_stream.proc_path, &g_stream.out_dim);
    if (ret != HD_OK) {
        printf("set_proc_param failed\n");
        goto exit_proc;
    }
    
    /* Bind capture to processor */
    ret = hd_videocap_bind(HD_VIDEOCAP_OUT(SEN1_VCAP_ID, 0), HD_VIDEOPROC_0_IN_0);
    if (ret != HD_OK) {
        printf("bind failed\n");
        goto exit_proc;
    }
    
    /* Start pipeline */
    ret = hd_videocap_start(g_stream.cap_path);
    if (ret != HD_OK) {
        printf("videocap start failed\n");
        goto exit_unbind;
    }
    
    ret = hd_videoproc_start(g_stream.proc_path);
    if (ret != HD_OK) {
        printf("videoproc start failed\n");
        goto exit_cap_stop;
    }
    
    printf("\nLive tampering detection started. Press Ctrl+C to stop.\n\n");
    
    /* Start detection thread */
    pthread_create(&thread_id, NULL, tamper_thread, NULL);
    
    /* Wait for thread */
    pthread_join(thread_id, NULL);
    
    /* Cleanup */
    hd_videoproc_stop(g_stream.proc_path);
    
exit_cap_stop:
    hd_videocap_stop(g_stream.cap_path);
    
exit_unbind:
    hd_videocap_unbind(HD_VIDEOCAP_OUT(SEN1_VCAP_ID, 0));
    
exit_proc:
    hd_videoproc_close(g_stream.proc_path);
    
exit_proc_ctrl:
    hd_videoproc_close(g_stream.proc_ctrl);
    
exit_cap:
    hd_videocap_close(g_stream.cap_path);
    
exit_cap_ctrl:
    hd_videocap_close(g_stream.cap_ctrl);

exit_proc_uninit:
    hd_videoproc_uninit();

exit_cap_uninit:
    hd_videocap_uninit();
    
exit_ive:
    vendor_ive_uninit();
    
exit_mem:
    free_mem_block(&g_sobel_buf);
    free_mem_block(&g_hist_buf);
    hd_common_mem_uninit();
    
exit_common:
    hd_common_uninit();
    
    printf("\nTampering detection stopped.\n");
    return 0;
}
