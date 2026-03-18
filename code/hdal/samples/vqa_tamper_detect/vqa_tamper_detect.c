/**
 * @file vqa_tamper_detect.c
 * @brief Professional Video Tampering Detection with IVE Hardware
 * @author Custom Implementation
 * @date 2026
 * 
 * Professional-grade tampering detection with:
 * - Configurable sensitivity (0-100)
 * - Dwell time / persistence check
 * - Defocus detection (blur)
 * - Masking/Blocking detection (lens covered)
 * - Scene change detection (camera moved)
 * - Exposure tampering (too dark/bright)
 * - Region of Interest (ROI) support
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <unistd.h>
#include <time.h>
#include "hdal.h"
#include "hd_debug.h"
#include "libive/libive.h"
#include <kwrap/perf.h>

#if defined(__LINUX)
#include <signal.h>
#include <pthread.h>
#define MAIN(argc, argv) int main(int argc, char** argv)
#else
#include <FreeRTOS_POSIX.h>
#include <FreeRTOS_POSIX/pthread.h>
#include <kwrap/util.h>
#define sleep(x) vos_util_delay_ms(1000*(x))
#include <kwrap/examsys.h>
#define MAIN(argc, argv) EXAMFUNC_ENTRY(vqa_tamper_detect, argc, argv)
#endif

/*============================================================================
 * Configuration Constants
 *============================================================================*/
#define IMG_WIDTH           320
#define IMG_HEIGHT          180
#define IMG_SIZE            (IMG_WIDTH * IMG_HEIGHT)

/* Default thresholds (will be adjusted by sensitivity) */
#define DEFAULT_BLUR_VARIANCE_THRESH    100.0f
#define DEFAULT_MASK_EDGE_DENSITY_THRESH 5.0f
#define DEFAULT_MASK_HISTOGRAM_CONC     80.0f   /* % pixels in narrow range */
#define DEFAULT_SCENE_CHANGE_THRESH     30.0f
#define DEFAULT_DARK_THRESH             40.0f
#define DEFAULT_BRIGHT_THRESH           220.0f
#define DEFAULT_DWELL_FRAMES            5       /* ~150ms at 30fps */

/*============================================================================
 * Tampering Event Types (Bitmask)
 *============================================================================*/
typedef enum {
    TAMPER_NONE           = 0x00,
    TAMPER_DEFOCUS        = 0x01,   /* Image blurred */
    TAMPER_MASKING        = 0x02,   /* Lens covered/blocked */
    TAMPER_SCENE_CHANGE   = 0x04,   /* Camera moved/redirected */
    TAMPER_TOO_DARK       = 0x08,   /* Underexposed */
    TAMPER_TOO_BRIGHT     = 0x10,   /* Overexposed/flashlight */
} TamperEventType;

/*============================================================================
 * Configuration Structure
 *============================================================================*/
typedef struct {
    /* Master switch */
    int enabled;
    
    /* Sensitivity 0-100 (higher = more sensitive = easier to trigger) */
    int sensitivity;
    
    /* Dwell time in frames (condition must persist this many frames) */
    int dwell_frames;
    
    /* Individual feature enables */
    int detect_defocus;
    int detect_masking;
    int detect_scene_change;
    int detect_dark;
    int detect_bright;
    
    /* Region of Interest (0,0,0,0 = full frame) */
    int roi_x;
    int roi_y;
    int roi_width;
    int roi_height;
    
    /* Calculated thresholds (based on sensitivity) */
    float blur_variance_thresh;
    float mask_edge_density_thresh;
    float mask_histogram_concentration;
    float scene_change_thresh;
    float dark_brightness_thresh;
    float bright_brightness_thresh;
} TamperConfig;

/*============================================================================
 * Detection State (for persistence/dwell tracking)
 *============================================================================*/
typedef struct {
    /* Consecutive frame counters for each event type */
    int defocus_count;
    int masking_count;
    int scene_change_count;
    int dark_count;
    int bright_count;
    
    /* Active alarms (persisted long enough) */
    UINT32 active_alarms;
    
    /* Reference data for scene change */
    int reference_valid;
    float ref_mean_brightness;
    float ref_laplacian_variance;
    float ref_edge_density;
    UINT32 ref_histogram[256];
} TamperState;

/*============================================================================
 * Current Frame Analysis Results
 *============================================================================*/
typedef struct {
    float mean_brightness;
    float laplacian_variance;
    float edge_density;
    float histogram_concentration;  /* % of pixels in dominant 32-bin range */
    float scene_difference;
    UINT32 histogram[256];
    
    /* Raw detections (before dwell) */
    UINT32 raw_detections;
    
    /* Final alarms (after dwell) */
    UINT32 confirmed_alarms;
    
    /* Timing */
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
 * Global State
 *============================================================================*/
static TamperConfig g_config;
static TamperState g_state;

/*============================================================================
 * Helper: Map sensitivity (0-100) to threshold multipliers
 *============================================================================*/
static void apply_sensitivity(TamperConfig *cfg)
{
    /* Sensitivity 0 = hardest to trigger, 100 = easiest to trigger */
    /* We adjust thresholds inversely to sensitivity */
    
    float sens_factor = (100 - cfg->sensitivity) / 50.0f;  /* 0->2.0, 50->1.0, 100->0.0 */
    if (sens_factor < 0.1f) sens_factor = 0.1f;  /* Minimum clamp */
    
    /* Higher sens_factor = higher thresholds = harder to trigger */
    cfg->blur_variance_thresh = DEFAULT_BLUR_VARIANCE_THRESH * sens_factor;
    cfg->mask_edge_density_thresh = DEFAULT_MASK_EDGE_DENSITY_THRESH * sens_factor;
    cfg->mask_histogram_concentration = DEFAULT_MASK_HISTOGRAM_CONC - (cfg->sensitivity * 0.3f);
    cfg->scene_change_thresh = DEFAULT_SCENE_CHANGE_THRESH * sens_factor;
    cfg->dark_brightness_thresh = DEFAULT_DARK_THRESH + (cfg->sensitivity * 0.2f);
    cfg->bright_brightness_thresh = DEFAULT_BRIGHT_THRESH - (cfg->sensitivity * 0.2f);
    
    /* Clamp values */
    if (cfg->mask_histogram_concentration < 50.0f) cfg->mask_histogram_concentration = 50.0f;
    if (cfg->dark_brightness_thresh > 80.0f) cfg->dark_brightness_thresh = 80.0f;
    if (cfg->bright_brightness_thresh < 180.0f) cfg->bright_brightness_thresh = 180.0f;
}

/*============================================================================
 * Initialize default configuration
 *============================================================================*/
static void init_default_config(TamperConfig *cfg)
{
    memset(cfg, 0, sizeof(*cfg));
    
    cfg->enabled = 1;
    cfg->sensitivity = 50;  /* Medium */
    cfg->dwell_frames = DEFAULT_DWELL_FRAMES;
    
    /* Enable all detection types by default */
    cfg->detect_defocus = 1;
    cfg->detect_masking = 1;
    cfg->detect_scene_change = 1;
    cfg->detect_dark = 1;
    cfg->detect_bright = 1;
    
    /* Full frame ROI */
    cfg->roi_x = 0;
    cfg->roi_y = 0;
    cfg->roi_width = 0;  /* 0 = full width */
    cfg->roi_height = 0; /* 0 = full height */
    
    apply_sensitivity(cfg);
}

/*============================================================================
 * Memory Management
 *============================================================================*/
static HD_RESULT mem_init(void)
{
    HD_COMMON_MEM_INIT_CONFIG mem_cfg = {0};
    
    mem_cfg.pool_info[0].type = HD_COMMON_MEM_USER_BLK;
    mem_cfg.pool_info[0].blk_size = IMG_SIZE;
    mem_cfg.pool_info[0].blk_cnt = 2;
    mem_cfg.pool_info[0].ddr_id = DDR_ID0;
    
    mem_cfg.pool_info[1].type = HD_COMMON_MEM_USER_BLK;
    mem_cfg.pool_info[1].blk_size = 256 * sizeof(UINT32);
    mem_cfg.pool_info[1].blk_cnt = 1;
    mem_cfg.pool_info[1].ddr_id = DDR_ID0;
    
    mem_cfg.pool_info[2].type = HD_COMMON_MEM_USER_BLK;
    mem_cfg.pool_info[2].blk_size = IMG_SIZE * 2;
    mem_cfg.pool_info[2].blk_cnt = 1;
    mem_cfg.pool_info[2].ddr_id = DDR_ID0;
    
    return hd_common_mem_init(&mem_cfg);
}

static HD_RESULT alloc_mem_block(MemBlock *blk, UINT32 size)
{
    blk->blk = hd_common_mem_get_block(HD_COMMON_MEM_USER_BLK, size, DDR_ID0);
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
static HD_RESULT ive_calc_histogram(MemBlock *input, UINT32 *histogram, int x, int y, int w, int h)
{
    IVE_SRC_IMAGE_S src = {0};
    IVE_DST_IMAGE_S dst = {0};
    UINT32 handle = 0;
    MemBlock hist_buf = {0};
    HD_RESULT ret;
    
    ret = alloc_mem_block(&hist_buf, 256 * sizeof(UINT32));
    if (ret != HD_OK) return ret;
    
    /* If ROI specified, adjust source pointer */
    UINTPTR src_va = input->va;
    UINTPTR src_pa = input->pa;
    
    if (x > 0 || y > 0) {
        int offset = y * IMG_WIDTH + x;
        src_va += offset;
        src_pa += offset;
    }
    
    src.enDataType = LIB_IVE_DATA_TYPE_U8C1;
    src.u64Pa = src_pa;
    src.u64Va = src_va;
    src.u32Width = (w > 0) ? w : IMG_WIDTH;
    src.u32Height = (h > 0) ? h : IMG_HEIGHT;
    src.u32Stride = IMG_WIDTH;  /* Always full width stride */
    
    dst.enDataType = LIB_IVE_DATA_TYPE_U32C1;
    dst.u64Pa = hist_buf.pa;
    dst.u64Va = hist_buf.va;
    dst.u32Width = 256;
    dst.u32Height = 1;
    dst.u32Stride = 256 * 4;
    
    hd_common_mem_flush_cache((void*)src.u64Va, src.u32Width * src.u32Height);
    
    ret = NVT_IVE_Histo(&handle, &src, &dst, 1);
    
    if (ret == HD_OK) {
        hd_common_mem_flush_cache((void*)dst.u64Va, 256 * 4);
        memcpy(histogram, (void*)hist_buf.va, 256 * sizeof(UINT32));
    }
    
    free_mem_block(&hist_buf);
    return ret;
}

static HD_RESULT ive_calc_sobel(MemBlock *input, MemBlock *output)
{
    IVE_SRC_IMAGE_S src = {0};
    IVE_DST_IMAGE_S dst = {0};
    IVE_SOBEL_CTRL_S ctrl = {0};
    UINT32 handle = 0;
    
    src.enDataType = LIB_IVE_DATA_TYPE_U8C1;
    src.u64Pa = input->pa;
    src.u64Va = input->va;
    src.u32Width = IMG_WIDTH;
    src.u32Height = IMG_HEIGHT;
    src.u32Stride = IMG_WIDTH;
    
    dst.enDataType = LIB_IVE_DATA_TYPE_S16C1;
    dst.u64Pa = output->pa;
    dst.u64Va = output->va;
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

/* Calculate Laplacian variance for blur detection */
static float calc_laplacian_variance(const UINT8 *y_plane, int w, int h)
{
    double sum = 0.0, sum_sq = 0.0;
    int count = 0;
    
    for (int y = 1; y < h - 1; y++) {
        for (int x = 1; x < w - 1; x++) {
            int idx = y * w + x;
            int lap = -4 * y_plane[idx]
                    + y_plane[idx - w]
                    + y_plane[idx + w]
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

/* Calculate edge density from Sobel output */
static float calc_edge_density(const INT16 *sobel, int w, int h)
{
    int edge_count = 0;
    int total = w * h;
    const INT16 thresh = 50;
    
    for (int i = 0; i < total; i++) {
        if (abs(sobel[i]) > thresh) edge_count++;
    }
    
    return 100.0f * edge_count / total;
}

/* Analyze histogram for masking detection */
static void analyze_histogram(const UINT32 *histogram, float *mean_brightness, 
                              float *concentration)
{
    UINT64 total = 0, weighted_sum = 0;
    
    for (int i = 0; i < 256; i++) {
        total += histogram[i];
        weighted_sum += (UINT64)i * histogram[i];
    }
    
    *mean_brightness = (total > 0) ? (float)weighted_sum / total : 0;
    
    /* Calculate histogram concentration (for masking detection) */
    /* Find the dominant 32-bin range and check what % of pixels are there */
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

/* Calculate scene difference from reference */
static float calc_scene_difference(const UINT32 *current, const UINT32 *reference,
                                   float curr_brightness, float ref_brightness,
                                   float curr_edge, float ref_edge)
{
    /* Chi-square histogram distance */
    float chi_sq = 0.0f;
    for (int i = 0; i < 256; i++) {
        float c = (float)current[i];
        float r = (float)reference[i];
        if (c + r > 0) {
            chi_sq += (c - r) * (c - r) / (c + r);
        }
    }
    chi_sq /= 256.0f;  /* Normalize */
    
    /* Also factor in brightness and edge density changes */
    float brightness_diff = fabsf(curr_brightness - ref_brightness) / 255.0f * 100.0f;
    float edge_diff = fabsf(curr_edge - ref_edge);
    
    return chi_sq * 10.0f + brightness_diff * 0.5f + edge_diff * 0.3f;
}

/*============================================================================
 * Main Tampering Detection
 *============================================================================*/
static HD_RESULT detect_tampering(MemBlock *input, MemBlock *sobel_buf, TamperResult *result)
{
    HD_RESULT ret;
    VOS_TICK t_start, t_end;
    TamperConfig *cfg = &g_config;
    TamperState *state = &g_state;
    
    memset(result, 0, sizeof(*result));
    
    if (!cfg->enabled) {
        return HD_OK;
    }
    
    vos_perf_mark(&t_start);
    
    /* Get ROI parameters */
    int roi_x = cfg->roi_x;
    int roi_y = cfg->roi_y;
    int roi_w = (cfg->roi_width > 0) ? cfg->roi_width : IMG_WIDTH;
    int roi_h = (cfg->roi_height > 0) ? cfg->roi_height : IMG_HEIGHT;
    
    /* 1. Calculate histogram (IVE hardware) */
    ret = ive_calc_histogram(input, result->histogram, roi_x, roi_y, roi_w, roi_h);
    if (ret != HD_OK) return ret;
    
    analyze_histogram(result->histogram, &result->mean_brightness, 
                      &result->histogram_concentration);
    
    /* 2. Calculate edge density (IVE Sobel) */
    ret = ive_calc_sobel(input, sobel_buf);
    if (ret != HD_OK) return ret;
    
    result->edge_density = calc_edge_density((INT16*)sobel_buf->va, IMG_WIDTH, IMG_HEIGHT);
    
    /* 3. Calculate blur (CPU Laplacian) */
    result->laplacian_variance = calc_laplacian_variance((UINT8*)input->va, IMG_WIDTH, IMG_HEIGHT);
    
    /* 4. Calculate scene difference if reference exists */
    if (state->reference_valid) {
        result->scene_difference = calc_scene_difference(
            result->histogram, state->ref_histogram,
            result->mean_brightness, state->ref_mean_brightness,
            result->edge_density, state->ref_edge_density);
    }
    
    vos_perf_mark(&t_end);
    result->process_time_us = vos_perf_duration(t_start, t_end);
    
    /* ===== Detection Logic ===== */
    result->raw_detections = TAMPER_NONE;
    
    /* Defocus: Low Laplacian variance */
    if (cfg->detect_defocus && result->laplacian_variance < cfg->blur_variance_thresh) {
        result->raw_detections |= TAMPER_DEFOCUS;
    }
    
    /* Masking: Low edge density AND high histogram concentration */
    if (cfg->detect_masking) {
        if (result->edge_density < cfg->mask_edge_density_thresh &&
            result->histogram_concentration > cfg->mask_histogram_concentration) {
            result->raw_detections |= TAMPER_MASKING;
        }
    }
    
    /* Scene Change: High difference from reference */
    if (cfg->detect_scene_change && state->reference_valid) {
        if (result->scene_difference > cfg->scene_change_thresh) {
            result->raw_detections |= TAMPER_SCENE_CHANGE;
        }
    }
    
    /* Too Dark */
    if (cfg->detect_dark && result->mean_brightness < cfg->dark_brightness_thresh) {
        result->raw_detections |= TAMPER_TOO_DARK;
    }
    
    /* Too Bright */
    if (cfg->detect_bright && result->mean_brightness > cfg->bright_brightness_thresh) {
        result->raw_detections |= TAMPER_TOO_BRIGHT;
    }
    
    /* ===== Dwell Time / Persistence Logic ===== */
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

/* Set reference frame for scene change detection */
static void set_reference_frame(TamperResult *result)
{
    TamperState *state = &g_state;
    
    memcpy(state->ref_histogram, result->histogram, sizeof(state->ref_histogram));
    state->ref_mean_brightness = result->mean_brightness;
    state->ref_laplacian_variance = result->laplacian_variance;
    state->ref_edge_density = result->edge_density;
    state->reference_valid = 1;
    
    printf("Reference frame captured:\n");
    printf("  Brightness: %.1f, Variance: %.1f, Edge Density: %.1f%%\n",
           state->ref_mean_brightness, state->ref_laplacian_variance, 
           state->ref_edge_density);
}

/*============================================================================
 * File I/O
 *============================================================================*/
static UINT32 load_file(const char *filename, UINTPTR va, UINT32 max_size)
{
    FILE *fd = fopen(filename, "rb");
    if (!fd) return 0;
    
    fseek(fd, 0, SEEK_END);
    UINT32 size = ftell(fd);
    fseek(fd, 0, SEEK_SET);
    
    if (size > max_size) size = max_size;
    
    UINT32 read_bytes = fread((void*)va, 1, size, fd);
    fclose(fd);
    
    return read_bytes;
}

/*============================================================================
 * Output / Reporting
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

static void print_result(const TamperResult *r, int frame_num)
{
    printf("\n[Frame %d] Process Time: %u us\n", frame_num, r->process_time_us);
    printf("  Brightness:     %.1f / 255\n", r->mean_brightness);
    printf("  Blur Variance:  %.1f (thresh: %.1f)\n", r->laplacian_variance, g_config.blur_variance_thresh);
    printf("  Edge Density:   %.1f%% (thresh: %.1f%%)\n", r->edge_density, g_config.mask_edge_density_thresh);
    printf("  Hist Conc:      %.1f%% (thresh: %.1f%%)\n", r->histogram_concentration, g_config.mask_histogram_concentration);
    printf("  Scene Diff:     %.1f (thresh: %.1f)\n", r->scene_difference, g_config.scene_change_thresh);
    printf("  Raw Detections: %s\n", get_alarm_string(r->raw_detections));
    printf("  CONFIRMED:      %s\n", get_alarm_string(r->confirmed_alarms));
    
    if (r->confirmed_alarms != TAMPER_NONE) {
        printf("  >>> TAMPERING ALARM! <<<\n");
    }
}

static void print_config(const TamperConfig *cfg)
{
    printf("\n========== Tampering Detection Config ==========\n");
    printf("Enabled:        %s\n", cfg->enabled ? "YES" : "NO");
    printf("Sensitivity:    %d/100\n", cfg->sensitivity);
    printf("Dwell Frames:   %d\n", cfg->dwell_frames);
    printf("\nDetection Types:\n");
    printf("  Defocus:      %s\n", cfg->detect_defocus ? "ON" : "OFF");
    printf("  Masking:      %s\n", cfg->detect_masking ? "ON" : "OFF");
    printf("  Scene Change: %s\n", cfg->detect_scene_change ? "ON" : "OFF");
    printf("  Too Dark:     %s\n", cfg->detect_dark ? "ON" : "OFF");
    printf("  Too Bright:   %s\n", cfg->detect_bright ? "ON" : "OFF");
    printf("\nThresholds (adjusted for sensitivity):\n");
    printf("  Blur Variance:  < %.1f = blurry\n", cfg->blur_variance_thresh);
    printf("  Edge Density:   < %.1f%% = masked\n", cfg->mask_edge_density_thresh);
    printf("  Hist Conc:      > %.1f%% = uniform/masked\n", cfg->mask_histogram_concentration);
    printf("  Scene Change:   > %.1f = moved\n", cfg->scene_change_thresh);
    printf("  Dark Bright:    < %.1f = dark\n", cfg->dark_brightness_thresh);
    printf("  Bright Bright:  > %.1f = bright\n", cfg->bright_brightness_thresh);
    printf("\nROI: ");
    if (cfg->roi_width == 0 && cfg->roi_height == 0) {
        printf("Full Frame (%dx%d)\n", IMG_WIDTH, IMG_HEIGHT);
    } else {
        printf("(%d,%d) %dx%d\n", cfg->roi_x, cfg->roi_y, cfg->roi_width, cfg->roi_height);
    }
    printf("=================================================\n\n");
}

static void print_usage(const char *prog)
{
    printf("Usage: %s [options]\n\n", prog);
    printf("Options:\n");
    printf("  -s <0-100>    Sensitivity (default: 50)\n");
    printf("  -d <frames>   Dwell time in frames (default: 5)\n");
    printf("  -n <count>    Number of test frames (default: 10)\n");
    printf("  -f <path>     Input file pattern (default: /mnt/sd/input/vqa_y_input_%%d.bin)\n");
    printf("  --no-defocus  Disable defocus detection\n");
    printf("  --no-masking  Disable masking detection\n");
    printf("  --no-scene    Disable scene change detection\n");
    printf("  --no-dark     Disable dark detection\n");
    printf("  --no-bright   Disable bright detection\n");
    printf("  -h            Show this help\n");
}

/*============================================================================
 * Main Entry
 *============================================================================*/
MAIN(argc, argv)
{
    HD_RESULT ret;
    MemBlock input_buf = {0};
    MemBlock sobel_buf = {0};
    TamperResult result;
    char input_pattern[256] = "/mnt/sd/input/vqa_y_input_%d.bin";
    char input_file[256];
    int num_frames = 10;
    
    printf("=================================================\n");
    printf("  Professional Video Tampering Detection\n");
    printf("  with IVE Hardware Acceleration\n");
    printf("  Resolution: %dx%d\n", IMG_WIDTH, IMG_HEIGHT);
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
        } else if (strcmp(argv[i], "-n") == 0 && i + 1 < argc) {
            num_frames = atoi(argv[++i]);
        } else if (strcmp(argv[i], "-f") == 0 && i + 1 < argc) {
            strncpy(input_pattern, argv[++i], sizeof(input_pattern) - 1);
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
    
    print_config(&g_config);
    
    /* Initialize HDAL */
    ret = hd_common_init(0);
    if (ret != HD_OK) {
        printf("hd_common_init failed: %d\n", ret);
        return -1;
    }
    
    ret = mem_init();
    if (ret != HD_OK) {
        printf("mem_init failed\n");
        goto exit_common;
    }
    
    ret = alloc_mem_block(&input_buf, IMG_SIZE);
    if (ret != HD_OK) goto exit_mem;
    
    ret = alloc_mem_block(&sobel_buf, IMG_SIZE * 2);
    if (ret != HD_OK) goto exit_mem;
    
    ret = vendor_ive_init();
    if (ret != HD_OK) {
        printf("vendor_ive_init failed\n");
        goto exit_mem;
    }
    
    /* Process frames */
    printf("\nProcessing %d frames...\n", num_frames);
    
    for (int frame = 0; frame < num_frames; frame++) {
        snprintf(input_file, sizeof(input_file), input_pattern, frame);
        
        UINT32 loaded = load_file(input_file, input_buf.va, IMG_SIZE);
        if (loaded == 0) {
            printf("[Frame %d] Cannot load: %s\n", frame, input_file);
            continue;
        }
        
        /* First frame becomes reference */
        if (frame == 0) {
            ret = detect_tampering(&input_buf, &sobel_buf, &result);
            if (ret == HD_OK) {
                set_reference_frame(&result);
                print_result(&result, frame);
            }
            continue;
        }
        
        /* Process subsequent frames */
        ret = detect_tampering(&input_buf, &sobel_buf, &result);
        if (ret == HD_OK) {
            print_result(&result, frame);
        }
    }
    
    printf("\n========== Summary ==========\n");
    printf("Final Active Alarms: %s\n", get_alarm_string(g_state.active_alarms));
    printf("Dwell Counters:\n");
    printf("  Defocus:      %d / %d\n", g_state.defocus_count, g_config.dwell_frames);
    printf("  Masking:      %d / %d\n", g_state.masking_count, g_config.dwell_frames);
    printf("  Scene Change: %d / %d\n", g_state.scene_change_count, g_config.dwell_frames);
    printf("  Dark:         %d / %d\n", g_state.dark_count, g_config.dwell_frames);
    printf("  Bright:       %d / %d\n", g_state.bright_count, g_config.dwell_frames);
    printf("=============================\n");
    
    printf("\nTampering detection test completed!\n");
    
    vendor_ive_uninit();
    
exit_mem:
    free_mem_block(&sobel_buf);
    free_mem_block(&input_buf);
    hd_common_mem_uninit();
    
exit_common:
    hd_common_uninit();
    
    return 0;
}
