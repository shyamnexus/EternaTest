/**
 * @file custom_vqa_sample.c
 * @brief Custom Video Quality Analysis using IVE Hardware Acceleration
 * @author Custom Implementation
 * @date 2026
 * 
 * This is a better VQA implementation that uses:
 * - IVE Hardware for histogram (exposure analysis)
 * - IVE Hardware for Sobel edge detection (blur analysis)  
 * - Laplacian variance for robust blur detection
 * - Confidence scores instead of binary flags
 * - Tamper detection via reference comparison
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <unistd.h>
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
#define MAIN(argc, argv) EXAMFUNC_ENTRY(custom_vqa_sample, argc, argv)
#endif

/*============================================================================
 * Configuration
 *============================================================================*/
#define IMG_WIDTH           320
#define IMG_HEIGHT          180
#define IMG_SIZE            (IMG_WIDTH * IMG_HEIGHT)

/* Thresholds */
#define OVEREXPOSE_THRESH   245     /* Pixel brightness threshold for overexposure */
#define UNDEREXPOSE_THRESH  40      /* Pixel brightness threshold for underexposure */
#define OVEREXPOSE_PERCENT  15.0f   /* Percent of pixels to trigger overexposure */
#define UNDEREXPOSE_PERCENT 50.0f   /* Percent of pixels to trigger underexposure */
#define MEAN_UNDEREXPOSE    50.0f   /* Mean brightness below this = underexposed */
#define MEAN_OVEREXPOSE     220.0f  /* Mean brightness above this = overexposed */
#define BLUR_VARIANCE_THRESH 100.0  /* Laplacian variance below this = blurry */
#define TAMPER_DIFF_THRESH   30.0   /* Scene difference threshold for tamper */

/* Memory buffers */
#define MEM_BUF_COUNT       5

/*============================================================================
 * Data Structures
 *============================================================================*/
typedef struct {
    UINTPTR va;
    UINTPTR pa;
    UINT32 size;
    HD_COMMON_MEM_VB_BLK blk;
} MemBlock;

/* Our enhanced VQA result - confidence scores instead of binary */
typedef struct {
    /* Exposure Analysis */
    float mean_brightness;          /* 0-255 */
    float overexposed_percent;      /* 0-100% */
    float underexposed_percent;     /* 0-100% */
    int overexposed;                /* Boolean flag */
    int underexposed;               /* Boolean flag */
    
    /* Blur Analysis */
    float laplacian_variance;       /* Higher = sharper */
    float edge_density;             /* From Sobel, 0-100% */
    float blur_confidence;          /* 0.0 - 1.0, higher = more blurry */
    int is_blurry;                  /* Boolean flag */
    
    /* Tamper Detection */
    float scene_diff;               /* Difference from reference */
    int tamper_detected;            /* Boolean flag */
    
    /* Timing */
    UINT32 histogram_time_us;
    UINT32 sobel_time_us;
    UINT32 laplacian_time_us;
    UINT32 total_time_us;
} CustomVQAResult;

/* Reference frame for tamper detection */
typedef struct {
    int initialized;
    float ref_mean_brightness;
    float ref_laplacian_variance;
    UINT32 histogram[256];
} TamperReference;

static TamperReference g_tamper_ref = {0};

/*============================================================================
 * Memory Management
 *============================================================================*/
static HD_RESULT mem_init(void)
{
    HD_COMMON_MEM_INIT_CONFIG mem_cfg = {0};
    
    /* Pool for input Y frame */
    mem_cfg.pool_info[0].type = HD_COMMON_MEM_USER_BLK;
    mem_cfg.pool_info[0].blk_size = IMG_SIZE;
    mem_cfg.pool_info[0].blk_cnt = 1;
    mem_cfg.pool_info[0].ddr_id = DDR_ID0;
    
    /* Pool for histogram output (256 x 4 bytes = 1KB) */
    mem_cfg.pool_info[1].type = HD_COMMON_MEM_USER_BLK;
    mem_cfg.pool_info[1].blk_size = 256 * sizeof(UINT32);
    mem_cfg.pool_info[1].blk_cnt = 1;
    mem_cfg.pool_info[1].ddr_id = DDR_ID0;
    
    /* Pool for Sobel output (16-bit = 2x size) */
    mem_cfg.pool_info[2].type = HD_COMMON_MEM_USER_BLK;
    mem_cfg.pool_info[2].blk_size = IMG_SIZE * 2;
    mem_cfg.pool_info[2].blk_cnt = 1;
    mem_cfg.pool_info[2].ddr_id = DDR_ID0;
    
    return hd_common_mem_init(&mem_cfg);
}

static HD_RESULT alloc_mem_block(MemBlock *blk, UINT32 size)
{
    blk->blk = hd_common_mem_get_block(HD_COMMON_MEM_USER_BLK, size, DDR_ID0);
    if (blk->blk == HD_COMMON_MEM_VB_INVALID_BLK) {
        printf("Failed to allocate block of size %u\n", size);
        return HD_ERR_NG;
    }
    
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
    if (blk->va) {
        hd_common_mem_munmap((void*)blk->va, blk->size);
    }
    if (blk->blk != HD_COMMON_MEM_VB_INVALID_BLK) {
        hd_common_mem_release_block(blk->blk);
    }
    memset(blk, 0, sizeof(*blk));
}

/*============================================================================
 * IVE Hardware Accelerated Functions
 *============================================================================*/

/**
 * Hardware-accelerated histogram calculation
 * Returns: histogram[256] with pixel counts
 */
static HD_RESULT ive_calc_histogram(MemBlock *input, MemBlock *output, UINT32 *time_us)
{
    IVE_SRC_IMAGE_S src = {0};
    IVE_DST_IMAGE_S dst = {0};
    UINT32 handle = 0;
    VOS_TICK t_start, t_end;
    HD_RESULT ret;
    
    src.enDataType = LIB_IVE_DATA_TYPE_U8C1;
    src.u64Pa = input->pa;
    src.u64Va = input->va;
    src.u32Width = IMG_WIDTH;
    src.u32Height = IMG_HEIGHT;
    src.u32Stride = IMG_WIDTH;
    
    dst.enDataType = LIB_IVE_DATA_TYPE_U32C1;
    dst.u64Pa = output->pa;
    dst.u64Va = output->va;
    dst.u32Width = 256;
    dst.u32Height = 1;
    dst.u32Stride = 256 * 4;
    
    hd_common_mem_flush_cache((void*)src.u64Va, IMG_SIZE);
    
    vos_perf_mark(&t_start);
    ret = NVT_IVE_Histo(&handle, &src, &dst, 1); /* blocking */
    vos_perf_mark(&t_end);
    
    if (time_us) {
        *time_us = vos_perf_duration(t_start, t_end);
    }
    
    hd_common_mem_flush_cache((void*)dst.u64Va, 256 * 4);
    
    return ret;
}

/**
 * Hardware-accelerated Sobel edge detection
 * Returns: edge magnitude image (16-bit)
 */
static HD_RESULT ive_calc_sobel(MemBlock *input, MemBlock *output, UINT32 *time_us)
{
    IVE_SRC_IMAGE_S src = {0};
    IVE_DST_IMAGE_S dst = {0};
    IVE_SOBEL_CTRL_S ctrl = {0};
    UINT32 handle = 0;
    VOS_TICK t_start, t_end;
    HD_RESULT ret;
    
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
    
    /* Sobel X kernel: [-1,0,1; -2,0,2; -1,0,1] */
    /* Arranged as 5x5 with zeros padding */
    ctrl.s8Mask[6] = -1; ctrl.s8Mask[7] = 0; ctrl.s8Mask[8] = 1;
    ctrl.s8Mask[11] = -2; ctrl.s8Mask[12] = 0; ctrl.s8Mask[13] = 2;
    ctrl.s8Mask[16] = -1; ctrl.s8Mask[17] = 0; ctrl.s8Mask[18] = 1;
    ctrl.gradient_out_format = 0; /* 8-bit packed */
    
    hd_common_mem_flush_cache((void*)src.u64Va, IMG_SIZE);
    
    vos_perf_mark(&t_start);
    ret = NVT_IVE_Sobel(&handle, &src, &dst, &ctrl, 1);
    vos_perf_mark(&t_end);
    
    if (time_us) {
        *time_us = vos_perf_duration(t_start, t_end);
    }
    
    hd_common_mem_flush_cache((void*)dst.u64Va, IMG_SIZE * 2);
    
    return ret;
}

/*============================================================================
 * CPU-based Analysis Functions
 *============================================================================*/

/**
 * Laplacian variance - gold standard for blur detection
 * Higher variance = sharper image
 * Lower variance = blurrier image
 */
static float calc_laplacian_variance(const UINT8 *y_plane, int w, int h)
{
    double sum = 0.0;
    double sum_sq = 0.0;
    int count = 0;
    
    /* Laplacian kernel: [0,1,0; 1,-4,1; 0,1,0] */
    for (int y = 1; y < h - 1; y++) {
        for (int x = 1; x < w - 1; x++) {
            int idx = y * w + x;
            int lap = -4 * y_plane[idx]
                    + y_plane[idx - w]      /* top */
                    + y_plane[idx + w]      /* bottom */
                    + y_plane[idx - 1]      /* left */
                    + y_plane[idx + 1];     /* right */
            sum += lap;
            sum_sq += (double)lap * lap;
            count++;
        }
    }
    
    if (count == 0) return 0.0f;
    
    double mean = sum / count;
    double variance = (sum_sq / count) - (mean * mean);
    
    return (float)variance;
}

/**
 * Analyze histogram for exposure metrics
 */
static void analyze_histogram(const UINT32 *histogram, CustomVQAResult *result)
{
    UINT64 total_pixels = 0;
    UINT64 weighted_sum = 0;
    UINT64 overexposed = 0;
    UINT64 underexposed = 0;
    
    for (int i = 0; i < 256; i++) {
        total_pixels += histogram[i];
        weighted_sum += (UINT64)i * histogram[i];
        
        if (i >= OVEREXPOSE_THRESH) {
            overexposed += histogram[i];
        }
        if (i <= UNDEREXPOSE_THRESH) {
            underexposed += histogram[i];
        }
    }
    
    if (total_pixels == 0) {
        result->mean_brightness = 0;
        result->overexposed_percent = 0;
        result->underexposed_percent = 0;
        return;
    }
    
    result->mean_brightness = (float)weighted_sum / total_pixels;
    result->overexposed_percent = 100.0f * overexposed / total_pixels;
    result->underexposed_percent = 100.0f * underexposed / total_pixels;
    
    /* Check both pixel distribution AND mean brightness */
    result->overexposed = (result->overexposed_percent > OVEREXPOSE_PERCENT) ||
                          (result->mean_brightness > MEAN_OVEREXPOSE);
    result->underexposed = (result->underexposed_percent > UNDEREXPOSE_PERCENT) ||
                           (result->mean_brightness < MEAN_UNDEREXPOSE);
}

/**
 * Analyze Sobel output for edge density
 */
static float calc_edge_density(const INT16 *sobel_output, int w, int h)
{
    int edge_count = 0;
    int total = w * h;
    const INT16 edge_thresh = 50; /* Sobel magnitude threshold */
    
    for (int i = 0; i < total; i++) {
        if (abs(sobel_output[i]) > edge_thresh) {
            edge_count++;
        }
    }
    
    return 100.0f * edge_count / total;
}

/**
 * Update tamper reference with current frame
 */
static void update_tamper_reference(const UINT32 *histogram, float brightness, float variance)
{
    g_tamper_ref.ref_mean_brightness = brightness;
    g_tamper_ref.ref_laplacian_variance = variance;
    memcpy(g_tamper_ref.histogram, histogram, 256 * sizeof(UINT32));
    g_tamper_ref.initialized = 1;
}

/**
 * Check for tampering against reference
 */
static void check_tamper(const UINT32 *histogram, float brightness, float variance, 
                         CustomVQAResult *result)
{
    if (!g_tamper_ref.initialized) {
        result->scene_diff = 0;
        result->tamper_detected = 0;
        return;
    }
    
    /* Calculate scene difference based on brightness and variance change */
    float brightness_diff = fabsf(brightness - g_tamper_ref.ref_mean_brightness);
    float variance_ratio = (g_tamper_ref.ref_laplacian_variance > 0) ?
                          variance / g_tamper_ref.ref_laplacian_variance : 1.0f;
    
    /* Large brightness change or sudden defocus = tamper */
    result->scene_diff = brightness_diff + fabsf(1.0f - variance_ratio) * 50.0f;
    result->tamper_detected = (result->scene_diff > TAMPER_DIFF_THRESH);
}

/*============================================================================
 * Main VQA Processing
 *============================================================================*/
static HD_RESULT run_custom_vqa(MemBlock *input, MemBlock *hist_buf, MemBlock *sobel_buf,
                                CustomVQAResult *result)
{
    HD_RESULT ret;
    VOS_TICK t_start, t_end;
    
    memset(result, 0, sizeof(*result));
    vos_perf_mark(&t_start);
    
    /* 1. Hardware-accelerated Histogram */
    ret = ive_calc_histogram(input, hist_buf, &result->histogram_time_us);
    if (ret != HD_OK) {
        printf("IVE Histogram failed: %d\n", ret);
        return ret;
    }
    
    /* Analyze exposure from histogram */
    analyze_histogram((UINT32*)hist_buf->va, result);
    
    /* 2. Hardware-accelerated Sobel edge detection */
    ret = ive_calc_sobel(input, sobel_buf, &result->sobel_time_us);
    if (ret != HD_OK) {
        printf("IVE Sobel failed: %d\n", ret);
        return ret;
    }
    
    /* Calculate edge density from Sobel */
    result->edge_density = calc_edge_density((INT16*)sobel_buf->va, IMG_WIDTH, IMG_HEIGHT);
    
    /* 3. CPU Laplacian variance (gold standard for blur) */
    VOS_TICK lap_start, lap_end;
    vos_perf_mark(&lap_start);
    result->laplacian_variance = calc_laplacian_variance((UINT8*)input->va, IMG_WIDTH, IMG_HEIGHT);
    vos_perf_mark(&lap_end);
    result->laplacian_time_us = vos_perf_duration(lap_start, lap_end);
    
    /* Calculate blur confidence (0-1, higher = more blurry) */
    float norm_variance = result->laplacian_variance / 500.0f; /* Normalize */
    if (norm_variance > 1.0f) norm_variance = 1.0f;
    result->blur_confidence = 1.0f - norm_variance;
    result->is_blurry = (result->laplacian_variance < BLUR_VARIANCE_THRESH);
    
    /* 4. Tamper detection */
    check_tamper((UINT32*)hist_buf->va, result->mean_brightness, 
                 result->laplacian_variance, result);
    
    vos_perf_mark(&t_end);
    result->total_time_us = vos_perf_duration(t_start, t_end);
    
    return HD_OK;
}

/*============================================================================
 * File I/O Helpers
 *============================================================================*/
static UINT32 load_file(const char *filename, UINTPTR va, UINT32 max_size)
{
    FILE *fd = fopen(filename, "rb");
    if (!fd) {
        printf("Cannot open: %s\n", filename);
        return 0;
    }
    
    fseek(fd, 0, SEEK_END);
    UINT32 size = ftell(fd);
    fseek(fd, 0, SEEK_SET);
    
    if (size > max_size) size = max_size;
    
    UINT32 read = fread((void*)va, 1, size, fd);
    fclose(fd);
    
    return read;
}

static void print_result(const CustomVQAResult *r)
{
    printf("\n========== Custom VQA Results ==========\n");
    printf("EXPOSURE:\n");
    printf("  Mean Brightness:    %.1f / 255\n", r->mean_brightness);
    printf("  Overexposed:        %.1f%% %s\n", r->overexposed_percent,
           r->overexposed ? "[ALARM]" : "");
    printf("  Underexposed:       %.1f%% %s\n", r->underexposed_percent,
           r->underexposed ? "[ALARM]" : "");
    
    printf("\nBLUR:\n");
    printf("  Laplacian Variance: %.1f %s\n", r->laplacian_variance,
           r->is_blurry ? "[BLURRY]" : "[SHARP]");
    printf("  Edge Density:       %.1f%%\n", r->edge_density);
    printf("  Blur Confidence:    %.2f\n", r->blur_confidence);
    
    printf("\nTAMPER:\n");
    printf("  Scene Difference:   %.1f %s\n", r->scene_diff,
           r->tamper_detected ? "[TAMPER DETECTED]" : "");
    
    printf("\nTIMING:\n");
    printf("  IVE Histogram:      %u us\n", r->histogram_time_us);
    printf("  IVE Sobel:          %u us\n", r->sobel_time_us);
    printf("  CPU Laplacian:      %u us\n", r->laplacian_time_us);
    printf("  Total:              %u us\n", r->total_time_us);
    printf("==========================================\n\n");
}

/*============================================================================
 * Main Entry Point
 *============================================================================*/
MAIN(argc, argv)
{
    HD_RESULT ret;
    MemBlock input_buf = {0};
    MemBlock hist_buf = {0};
    MemBlock sobel_buf = {0};
    CustomVQAResult result;
    char input_file[128];
    int frame_num = 1;
    int test_iterations = 10;
    
    printf("===========================================\n");
    printf("  Custom VQA with IVE Hardware Accel\n");
    printf("  Resolution: %dx%d\n", IMG_WIDTH, IMG_HEIGHT);
    printf("===========================================\n\n");
    
    /* Parse args */
    if (argc >= 2) {
        frame_num = atoi(argv[1]);
    }
    if (argc >= 3) {
        test_iterations = atoi(argv[2]);
    }
    
    /* Initialize HDAL */
    ret = hd_common_init(0);
    if (ret != HD_OK) {
        printf("hd_common_init failed: %d\n", ret);
        return -1;
    }
    
    /* Initialize memory pools */
    ret = mem_init();
    if (ret != HD_OK) {
        printf("mem_init failed: %d\n", ret);
        goto exit_common;
    }
    
    /* Allocate buffers */
    ret = alloc_mem_block(&input_buf, IMG_SIZE);
    if (ret != HD_OK) goto exit_mem;
    
    ret = alloc_mem_block(&hist_buf, 256 * sizeof(UINT32));
    if (ret != HD_OK) goto exit_mem;
    
    ret = alloc_mem_block(&sobel_buf, IMG_SIZE * 2);
    if (ret != HD_OK) goto exit_mem;
    
    /* Initialize IVE hardware */
    ret = vendor_ive_init();
    if (ret != HD_OK) {
        printf("vendor_ive_init failed: %d\n", ret);
        goto exit_mem;
    }
    
    /* Load test image */
    snprintf(input_file, sizeof(input_file), "/mnt/sd/input/vqa_y_input_%d.bin", frame_num);
    printf("Loading: %s\n", input_file);
    
    UINT32 loaded = load_file(input_file, input_buf.va, IMG_SIZE);
    if (loaded == 0) {
        printf("Failed to load input file\n");
        goto exit_ive;
    }
    printf("Loaded %u bytes\n\n", loaded);
    
    /* Set first frame as reference for tamper detection */
    printf("Running initial frame as tamper reference...\n");
    ret = run_custom_vqa(&input_buf, &hist_buf, &sobel_buf, &result);
    if (ret == HD_OK) {
        update_tamper_reference((UINT32*)hist_buf.va, result.mean_brightness,
                               result.laplacian_variance);
        print_result(&result);
    }
    
    /* Run multiple iterations for timing comparison */
    printf("Running %d iterations for timing...\n", test_iterations);
    UINT32 total_time = 0;
    
    for (int i = 0; i < test_iterations; i++) {
        ret = run_custom_vqa(&input_buf, &hist_buf, &sobel_buf, &result);
        if (ret == HD_OK) {
            total_time += result.total_time_us;
            printf("  Iteration %d: %u us\n", i + 1, result.total_time_us);
        }
    }
    
    printf("\n=== SUMMARY ===\n");
    printf("Average time per frame: %u us (%.1f fps possible)\n",
           total_time / test_iterations,
           1000000.0f / (total_time / test_iterations));
    
    print_result(&result);
    
    printf("\nCustom VQA test completed successfully!\n");
    
exit_ive:
    vendor_ive_uninit();
    
exit_mem:
    free_mem_block(&sobel_buf);
    free_mem_block(&hist_buf);
    free_mem_block(&input_buf);
    hd_common_mem_uninit();
    
exit_common:
    hd_common_uninit();
    
    return 0;
}
