/**
 * @file md_osg_demo.c
 * @brief Motion Detection with Hardware OSG Overlay Demo for NT98538
 * 
 * This sample demonstrates motion detection with HARDWARE OSG OVERLAYS:
 * - MDBC (Background Comparison)
 * - Object Detection with bounding boxes
 * - OSG (On-Screen Graphics) hardware overlay using HD_STAMP path
 * - ARGB4444 overlay buffer with transparency support
 * - Hardware-blended overlays - zero CPU overhead during encoding
 * 
 * Key Feature: Uses HD_STAMP path with ARGB4444 overlay buffer that gets
 * hardware-blended by the encoder. This is the PROFESSIONAL method used
 * by commercial IP cameras for clean, efficient overlays.
 * 
 * Advantages over GFX draw:
 * - Zero CPU overhead (hardware blending)
 * - Full alpha transparency support
 * - Clean anti-aliased edges
 * - Separate overlay layer from video data
 * 
 * Pipeline:
 *   VideoCap -> VideoProc -> VideoEnc (with HD_STAMP overlay)
 *                    OUT_1 (160x120) -> MD Processing
 * 
 * Output:
 * - H.265 video file with hardware-blended bounding boxes
 * - Console logs with motion events
 * 
 * Usage: ./md_osg_demo [duration_seconds] [sensitivity]
 *        duration: recording duration (default: 30)
 *        sensitivity: 0=low, 1=med, 2=high, 3=super_high (default: 2)
 * 
 * @author IP Camera Team
 * @date 2026
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>

// Platform dependent
#if defined(__LINUX)
#include <signal.h>
#include <pthread.h>
#define MAIN(argc, argv)    int main(int argc, char** argv)
#define GETCHAR()           getchar()
#else
#include <FreeRTOS_POSIX.h>
#include <FreeRTOS_POSIX/signal.h>
#include <FreeRTOS_POSIX/pthread.h>
#include <kwrap/util.h>
#define sleep(x)            vos_util_delay_ms(1000*(x))
#define msleep(x)           vos_util_delay_ms(x)
#define usleep(x)           vos_util_delay_us(x)
#include <kwrap/examsys.h>
#define MAIN(argc, argv)    EXAMFUNC_ENTRY(md_osg_demo, argc, argv)
#define GETCHAR()           NVT_EXAMSYS_GETCHAR()
#endif

#include "hdal.h"
#include "hd_debug.h"
#include "vendor_videoprocess.h"
#include "vendor_md.h"
#include "libmd/libmd.h"

// OSG Overlay Configuration
// ARGB4444 format: 4 bits each for Alpha, Red, Green, Blue
#define BBOX_COLOR_RED      0xF00F   // ARGB4444: Full alpha, Red (A=F, R=F, G=0, B=0 -> 0xF F00 but packed as ARGB4444)
#define BBOX_COLOR_GREEN    0xF0F0   // ARGB4444: Full alpha, Green
#define BBOX_COLOR_YELLOW   0xFFF0   // ARGB4444: Full alpha, Yellow
#define BBOX_COLOR_CYAN     0xF0FF   // ARGB4444: Full alpha, Cyan
#define BBOX_COLOR_WHITE    0xFFFF   // ARGB4444: Full alpha, White
#define BBOX_COLOR_GRAY     0x8AAA   // ARGB4444: Semi-transparent, Grayish white (A=8, R=A, G=A, B=A)
#define BBOX_COLOR_FAINT    0x6CCC   // ARGB4444: Faint grayish white (A=6, R=C, G=C, B=C)
#define BBOX_THICKNESS      1        // Border thickness in pixels
#define MAX_DRAW_OBJECTS    16       // Max bounding boxes to draw per frame

// OSG Stamp buffer dimensions (overlay layer size)
#define STAMP_WIDTH         1920     // Same as video output width
#define STAMP_HEIGHT        1080     // Same as video output height

// ANSI Color codes for terminal output
#define COLOR_RESET   "\033[0m"
#define COLOR_RED     "\033[1;31m"
#define COLOR_GREEN   "\033[1;32m"
#define COLOR_YELLOW  "\033[1;33m"
#define COLOR_BLUE    "\033[1;34m"
#define COLOR_MAGENTA "\033[1;35m"
#define COLOR_CYAN    "\033[1;36m"
#define COLOR_WHITE   "\033[1;37m"

// ============================================================================
// Configuration
// ============================================================================
#define SENSOR_NAME         "nvt_sen_gc5603"
#define VCAP_ID             0

// Video dimensions - GC5603 native resolution
#define CAP_WIDTH           2960
#define CAP_HEIGHT          1664
#define SEN_OUT_FMT         HD_VIDEO_PXLFMT_RAW10
#define CAP_OUT_FMT         HD_VIDEO_PXLFMT_RAW10

// Output video dimensions (scaled down for encoding)
#define MAIN_WIDTH          1920
#define MAIN_HEIGHT         1080
#define MAIN_FPS            30
#define MAIN_BITRATE        (4 * 1024 * 1024)  // 4 Mbps

// Memory buffer size macros (from working sample)
#define DBGINFO_BUFSIZE()   (0x200)
#define VDO_RAW_BUFSIZE(w, h, pxlfmt)   (ALIGN_CEIL_4((w) * HD_VIDEO_PXLFMT_BPP(pxlfmt) / 8) * (h))
#define VDO_YUV_BUFSIZE(w, h, pxlfmt)   (ALIGN_CEIL_4((w) * HD_VIDEO_PXLFMT_BPP(pxlfmt) / 8) * (h))
#define VDO_CA_BUF_SIZE(win_num_w, win_num_h) ALIGN_CEIL_4((win_num_w * win_num_h << 3) << 1)
#define VDO_LA_BUF_SIZE(win_num_w, win_num_h) ALIGN_CEIL_4((win_num_w * win_num_h << 1) << 1)
#define CA_WIN_NUM_W        32
#define CA_WIN_NUM_H        32
#define LA_WIN_NUM_W        32
#define LA_WIN_NUM_H        32

// MD processing dimensions (must stay within 15.99x scale limit from 2960x1664)
// 2960/15.99 = 185, so MD_WIDTH must be >= 192
// 1664/15.99 = 104, so MD_HEIGHT must be >= 112
#define MD_WIDTH            240
#define MD_HEIGHT           160
#define MD_BUF_SIZE         (MD_WIDTH * MD_HEIGHT)

// MD head and info buffer sizes
#define MD_HEAD_BUFSIZE()   (0x40)
#define MD_INFO_BUFSIZE(w, h)  (ALIGN_CEIL_64(((((w >> 7) + 3) >> 2) << 2) * ((h + 15) >> 4)))

// Sub-region definitions (4 zones covering the frame)
#define NUM_SUB_REGIONS     4

// Motion detection thresholds
#define GLOBAL_ALARM_THRESHOLD  30   // % of frame with motion to trigger alarm
#define SUB_ALARM_THRESHOLD     20   // % of sub-region with motion

// Minimum object size for overlay drawing (in output resolution pixels)
// Objects smaller than this will be ignored - increase for fewer false positives
#define MIN_OBJECT_WIDTH    200  // Minimum width in pixels (at 1920x1080)
#define MIN_OBJECT_HEIGHT   200  // Minimum height in pixels (at 1920x1080)

// Maximum objects to draw - limit to reduce clutter
#define MAX_VISIBLE_OBJECTS  1   // Only show the largest object

// Temporal consistency - require object to persist across frames
#define OBJECT_CONFIRM_FRAMES  3   // Frames object must persist before showing
#define OBJECT_HOLDOFF_FRAMES  20  // Frames to keep showing after object disappears (increased for stability)

// Output files
#define OUTPUT_VIDEO_FILE   "/mnt/sd/md_overlay_output.h265"
#define OUTPUT_LOG_FILE     "/mnt/sd/md_overlay_log.txt"

// ============================================================================
// Global State
// ============================================================================
static volatile int g_running = 1;
static volatile int g_frame_count = 0;
static volatile int g_motion_frames = 0;
static volatile int g_alarm_count = 0;
static volatile int g_objects_detected = 0;

// Statistics
typedef struct {
    uint32_t total_frames;
    uint32_t motion_frames;
    uint32_t global_alarms;
    uint32_t sub_alarms[NUM_SUB_REGIONS];
    uint32_t max_objects;
    uint32_t crossline_events;
    uint64_t total_md_time_us;
    uint32_t min_md_time_us;
    uint32_t max_md_time_us;
} MD_STATS;

static MD_STATS g_stats = {0};
static pthread_mutex_t g_stats_mutex = PTHREAD_MUTEX_INITIALIZER;

// MD processing synchronization
static volatile int g_md_running = 0;
static volatile int g_md_frame_ready = 0;
static volatile int g_flow_start = 0;  // Signal when encoder/MD can start pulling

// Shared object detection results for overlay drawing
typedef struct {
    UINT32 obj_count;
    struct {
        UINT32 x;      // Scaled to MAIN_WIDTH
        UINT32 y;      // Scaled to MAIN_HEIGHT
        UINT32 w;
        UINT32 h;
    } objects[MAX_DRAW_OBJECTS];
} SHARED_OBJECTS;

static SHARED_OBJECTS g_shared_objects = {0};
static pthread_mutex_t g_objects_mutex = PTHREAD_MUTEX_INITIALIZER;

// Unified bounding box that encompasses all detected motion
// Smoothly expands/contracts as motion appears/disappears
typedef struct {
    float x, y, w, h;        // Current smoothed position (float for interpolation)
    float target_x, target_y, target_w, target_h;  // Target from detection union
    int confirm_count;       // Frames motion has been present
    int holdoff_count;       // Frames since motion disappeared
    int valid;               // Is there active motion
} UNIFIED_BBOX;

// Smoothing factor: 0.0 = no change, 1.0 = instant update
// Lower values = smoother but more lag, higher = more responsive but jumpier
#define SMOOTH_FACTOR  0.12f   // Blend 12% per frame for smooth expansion/contraction

static UNIFIED_BBOX g_unified_bbox = {0};

// ============================================================================
// Memory Block Management
// ============================================================================
typedef struct {
    UINTPTR pa;
    UINTPTR va;
    UINT32 size;
    HD_COMMON_MEM_VB_BLK blk;
} MEM_BLOCK;

static HD_RESULT alloc_mem_block(MEM_BLOCK* block, UINT32 size, const char* name)
{
    block->size = size;
    block->blk = hd_common_mem_get_block(HD_COMMON_MEM_USER_BLK, size, DDR_ID0);
    if (block->blk == HD_COMMON_MEM_VB_INVALID_BLK) {
        printf("[ERROR] Failed to allocate %s (%u bytes)\n", name, size);
        return HD_ERR_NG;
    }
    
    block->pa = hd_common_mem_blk2pa(block->blk);
    if (block->pa == 0) {
        hd_common_mem_release_block(block->blk);
        block->blk = HD_COMMON_MEM_VB_INVALID_BLK;
        return HD_ERR_NG;
    }
    
    block->va = (UINTPTR)hd_common_mem_mmap(HD_COMMON_MEM_MEM_TYPE_CACHE, block->pa, size);
    if (block->va == 0) {
        hd_common_mem_release_block(block->blk);
        block->blk = HD_COMMON_MEM_VB_INVALID_BLK;
        return HD_ERR_NG;
    }
    
    printf("[MEM] Allocated %s: PA=0x%lX VA=0x%lX size=%u\n", name, (unsigned long)block->pa, (unsigned long)block->va, size);
    return HD_OK;
}

static void free_mem_block(MEM_BLOCK* block)
{
    if (block->va) {
        hd_common_mem_munmap((void*)block->va, block->size);
        block->va = 0;
    }
    if (block->blk != HD_COMMON_MEM_VB_INVALID_BLK) {
        hd_common_mem_release_block(block->blk);
        block->blk = HD_COMMON_MEM_VB_INVALID_BLK;
    }
    block->pa = 0;
    block->size = 0;
}

// ============================================================================
// Signal Handler
// ============================================================================
static void signal_handler(int sig)
{
    printf("\n[SIGNAL] Caught signal %d, stopping...\n", sig);
    g_running = 0;
    g_md_running = 0;
}

// ============================================================================
// Utility Functions
// ============================================================================
#if 0  // Unused in simplified demo - available for extended version
static uint64_t get_time_us(void)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (uint64_t)tv.tv_sec * 1000000ULL + tv.tv_usec;
}
#endif

static const char* get_sensitivity_name(int level)
{
    switch (level) {
        case 0: return "LOW";
        case 1: return "MEDIUM";
        case 2: return "HIGH";
        case 3: return "SUPER_HIGH";
        default: return "UNKNOWN";
    }
}

static void print_banner(void)
{
    printf("\n");
    printf(COLOR_CYAN "╔══════════════════════════════════════════════════════════════╗\n" COLOR_RESET);
    printf(COLOR_CYAN "║" COLOR_WHITE "     NT98538 Motion Detection + Bounding Box Overlay Demo     " COLOR_CYAN "║\n" COLOR_RESET);
    printf(COLOR_CYAN "╠══════════════════════════════════════════════════════════════╣\n" COLOR_RESET);
    printf(COLOR_CYAN "║" COLOR_RESET "  Features:                                                   " COLOR_CYAN "║\n" COLOR_RESET);
    printf(COLOR_CYAN "║" COLOR_GREEN "    ✓ " COLOR_RESET "MDBC Background Comparison                              " COLOR_CYAN "║\n" COLOR_RESET);
    printf(COLOR_CYAN "║" COLOR_GREEN "    ✓ " COLOR_RESET "Object Detection with coordinates                       " COLOR_CYAN "║\n" COLOR_RESET);
    printf(COLOR_CYAN "║" COLOR_RED "    ✓ " COLOR_RESET "Hardware OSG overlay (professional IP camera method)   " COLOR_CYAN "║\n" COLOR_RESET);
    printf(COLOR_CYAN "║" COLOR_GREEN "    ✓ " COLOR_RESET "ARGB4444 overlay with full alpha transparency          " COLOR_CYAN "║\n" COLOR_RESET);
    printf(COLOR_CYAN "║" COLOR_GREEN "    ✓ " COLOR_RESET "Zero CPU overhead - hardware-blended overlays           " COLOR_CYAN "║\n" COLOR_RESET);
    printf(COLOR_CYAN "╚══════════════════════════════════════════════════════════════╝\n" COLOR_RESET);
    printf("\n");
}

#if 0  // Unused in simplified demo - available for extended version with software MD
static void print_motion_event(const char* type, const char* details)
{
    time_t now = time(NULL);
    struct tm* tm_info = localtime(&now);
    char time_str[32];
    strftime(time_str, sizeof(time_str), "%H:%M:%S", tm_info);
    
    // Color code based on event type
    const char* color = COLOR_YELLOW;
    if (strstr(type, "GLOBAL") != NULL) color = COLOR_RED;
    else if (strstr(type, "SUB") != NULL) color = COLOR_MAGENTA;
    else if (strstr(type, "OBJ") != NULL) color = COLOR_GREEN;
    else if (strstr(type, "CROSS") != NULL) color = COLOR_CYAN;
    
    printf(COLOR_WHITE "[%s] " COLOR_RESET "%s%s" COLOR_RESET ": %s\n", time_str, color, type, details);
}

static void print_object_detection(MD_OBJ_INFO_S* obj_info)
{
    if (obj_info->u32ObjNum == 0) return;
    
    printf(COLOR_GREEN "    ┌─ Objects Detected: %u\n" COLOR_RESET, obj_info->u32ObjNum);
    for (UINT32 i = 0; i < obj_info->u32ObjNum && i < 5; i++) {
        MD_OBJ_RST_S* obj = &obj_info->stObjRst[i];
        printf(COLOR_GREEN "    │  [%u] pos(%u,%u) size(%ux%u)\n" COLOR_RESET, 
               i, obj->u32XStart, obj->u32YStart,
               obj->u32XEnd - obj->u32XStart,
               obj->u32YEnd - obj->u32YStart);
    }
    if (obj_info->u32ObjNum > 5) {
        printf(COLOR_GREEN "    │  ... and %u more\n" COLOR_RESET, obj_info->u32ObjNum - 5);
    }
    printf("    └─\n");
}
#endif

static void print_statistics(void)
{
    pthread_mutex_lock(&g_stats_mutex);
    
    float motion_pct = g_stats.total_frames > 0 ? 
        (100.0f * g_stats.motion_frames / g_stats.total_frames) : 0;
    float avg_md_time = g_stats.total_frames > 0 ?
        (float)g_stats.total_md_time_us / g_stats.total_frames : 0;
    
    printf("\n");
    printf("╔══════════════════════════════════════════════════════════════╗\n");
    printf("║                    Motion Detection Statistics               ║\n");
    printf("╠══════════════════════════════════════════════════════════════╣\n");
    printf("║  Total Frames:      %-10d                               ║\n", g_stats.total_frames);
    printf("║  Frames with Motion: %-10d (%.1f%%)                      ║\n", 
           g_stats.motion_frames, motion_pct);
    printf("║  Global Alarms:     %-10d                               ║\n", g_stats.global_alarms);
    printf("║  Sub-Region Alarms:                                          ║\n");
    printf("║    Zone 0 (TL): %-6d  Zone 1 (TR): %-6d                  ║\n",
           g_stats.sub_alarms[0], g_stats.sub_alarms[1]);
    printf("║    Zone 2 (BL): %-6d  Zone 3 (BR): %-6d                  ║\n",
           g_stats.sub_alarms[2], g_stats.sub_alarms[3]);
    printf("║  Max Objects/Frame: %-10d                               ║\n", g_stats.max_objects);
    printf("║  Cross-Line Events: %-10d                               ║\n", g_stats.crossline_events);
    printf("╠══════════════════════════════════════════════════════════════╣\n");
    printf("║  MD Processing Time:                                         ║\n");
    printf("║    Average: %.2f ms  Min: %.2f ms  Max: %.2f ms            ║\n",
           avg_md_time / 1000.0f,
           g_stats.min_md_time_us / 1000.0f,
           g_stats.max_md_time_us / 1000.0f);
    printf("╚══════════════════════════════════════════════════════════════╝\n");
    
    pthread_mutex_unlock(&g_stats_mutex);
}

// ============================================================================
// Memory Management (reserved for future software MD processing)
// ============================================================================
#if 0  // Currently using hardware MD only, memory functions reserved for future use
typedef struct {
    UINTPTR pa;
    UINTPTR va;
    uint32_t size;
    HD_COMMON_MEM_VB_BLK blk;
} MEM_BLOCK;

static HD_RESULT alloc_mem_block(MEM_BLOCK* block, uint32_t size, const char* name)
{
    block->size = size;
    block->blk = hd_common_mem_get_block(HD_COMMON_MEM_USER_BLK, size, DDR_ID0);
    if (block->blk == HD_COMMON_MEM_VB_INVALID_BLK) {
        printf("[ERROR] Failed to allocate %s (%d bytes)\n", name, size);
        return HD_ERR_NG;
    }
    
    block->pa = hd_common_mem_blk2pa(block->blk);
    if (block->pa == 0) {
        hd_common_mem_release_block(block->blk);
        return HD_ERR_NG;
    }
    
    block->va = (UINTPTR)hd_common_mem_mmap(HD_COMMON_MEM_MEM_TYPE_CACHE, block->pa, size);
    if (block->va == 0) {
        hd_common_mem_release_block(block->blk);
        return HD_ERR_NG;
    }
    
    return HD_OK;
}

static void free_mem_block(MEM_BLOCK* block)
{
    if (block->va) {
        hd_common_mem_munmap((void*)block->va, block->size);
        block->va = 0;
    }
    if (block->blk != HD_COMMON_MEM_VB_INVALID_BLK) {
        hd_common_mem_release_block(block->blk);
        block->blk = HD_COMMON_MEM_VB_INVALID_BLK;
    }
}
#endif

// ============================================================================
// HDAL Pipeline Setup
// ============================================================================
typedef struct {
    // Control paths
    HD_PATH_ID cap_ctrl;
    HD_PATH_ID proc_ctrl;
    
    // Data paths
    HD_PATH_ID cap_path;
    HD_PATH_ID proc_path_main;    // Main stream -> encoder (normal bind, not pull/push)
    HD_PATH_ID proc_path_md;      // MD path for libmd processing (160x120 YUV)
    HD_PATH_ID enc_path;
    
    // OSG stamp path (hardware overlay)
    HD_PATH_ID stamp_path;
    HD_COMMON_MEM_VB_BLK stamp_blk;
    UINTPTR stamp_pa;             // Physical address of stamp buffer
    UINTPTR stamp_va;             // Virtual address of stamp buffer
    UINT32 stamp_size;
    unsigned short* stamp_buffer; // CPU-side ARGB4444 buffer for drawing
    
    // Output file
    FILE* video_file;
    FILE* log_file;
    
    // Encoder buffer info
    UINTPTR enc_vir_addr;
    HD_VIDEOENC_BUFINFO enc_buf_info;
    
    // MD processing buffers
    MEM_BLOCK md_src;       // YUV420 source (160x120 * 1.5)
    MEM_BLOCK md_temp;      // MDBC working buffer (160x120 * 48)
    MEM_BLOCK md_dst;       // Foreground mask output (160x120)
    
    // MD control structures
    MD_MDBC_CTRL_S mdbc_ctrl;
    MD_GALARM_CTRL_S galarm_ctrl;
    MD_SUBALARM_CTRL_S subalarm_ctrl;
    MD_SUBPARAM_CTRL_S sub_params[NUM_SUB_REGIONS];
    MD_OBJ_CTRL_S objdet_ctrl;
    MD_OBJ_INFO_S obj_info;
    MD_PT_INFO_S obj_pt_stack[MD_WIDTH * MD_HEIGHT];
    
    // MDBC first frame flag
    int mdbc_initialized;
    
    // OSG initialized flag
    int osg_initialized;
    
    // Configuration
    int sensitivity;
    int duration;
    
} DEMO_CONTEXT;

// ============================================================================
// OSG Buffer Size Calculation (from SDK sample)
// ============================================================================
static int query_osg_buf_size(void)
{
    HD_VIDEO_FRAME frame = {0};
    int stamp_size;
    
    frame.sign   = MAKEFOURCC('O','S','G','P');
    frame.dim.w  = STAMP_WIDTH;
    frame.dim.h  = STAMP_HEIGHT;
    frame.pxlfmt = HD_VIDEO_PXLFMT_ARGB4444;
    
    // Get required buffer size for a single image
    stamp_size = hd_common_mem_calc_buf_size(&frame);
    if (!stamp_size) {
        printf("[OSG] Failed to query buffer size\n");
        return -1;
    }
    
    // Ping pong buffer needs double size
    stamp_size *= 2;
    
    // Ping pong buffer size must be 128 aligned
    stamp_size = ALIGN_CEIL(stamp_size, 128);
    
    return stamp_size;
}

static HD_RESULT init_memory(UINT32 osg_stamp_size)
{
    HD_COMMON_MEM_INIT_CONFIG mem_cfg = {0};
    
    // Pool 0: video capture (RAW10 at full sensor resolution)
    mem_cfg.pool_info[0].type = HD_COMMON_MEM_COMMON_POOL;
    mem_cfg.pool_info[0].blk_size = DBGINFO_BUFSIZE() + VDO_RAW_BUFSIZE(CAP_WIDTH, CAP_HEIGHT, CAP_OUT_FMT)
                                    + VDO_CA_BUF_SIZE(CA_WIN_NUM_W, CA_WIN_NUM_H)
                                    + VDO_LA_BUF_SIZE(LA_WIN_NUM_W, LA_WIN_NUM_H);
    mem_cfg.pool_info[0].blk_cnt = 3;
    mem_cfg.pool_info[0].ddr_id = DDR_ID0;
    
    // Pool 1: video processing output (YUV420 at encode resolution)
    mem_cfg.pool_info[1].type = HD_COMMON_MEM_COMMON_POOL;
    mem_cfg.pool_info[1].blk_size = DBGINFO_BUFSIZE() + VDO_YUV_BUFSIZE(MAIN_WIDTH, MAIN_HEIGHT, HD_VIDEO_PXLFMT_YUV420)
                                    + MD_HEAD_BUFSIZE() + MD_INFO_BUFSIZE(MAIN_WIDTH, MAIN_HEIGHT);
    mem_cfg.pool_info[1].blk_cnt = 3;
    mem_cfg.pool_info[1].ddr_id = DDR_ID0;
    
    // Pool 2: MD path YUV output (160x120 for libmd)
    mem_cfg.pool_info[2].type = HD_COMMON_MEM_COMMON_POOL;
    mem_cfg.pool_info[2].blk_size = DBGINFO_BUFSIZE() + VDO_YUV_BUFSIZE(MD_WIDTH, MD_HEIGHT, HD_VIDEO_PXLFMT_YUV420);
    mem_cfg.pool_info[2].blk_cnt = 3;
    mem_cfg.pool_info[2].ddr_id = DDR_ID0;
    
    // Pool 3: OSG overlay buffer (ARGB4444 stamp for hardware blending)
    mem_cfg.pool_info[3].type = HD_COMMON_MEM_OSG_POOL;
    mem_cfg.pool_info[3].blk_size = osg_stamp_size;
    mem_cfg.pool_info[3].blk_cnt = 1;
    mem_cfg.pool_info[3].ddr_id = DDR_ID0;
    
    // Pool 4: User block for MD source buffer (YUV420 160x120 = 160*120*1.5)
    mem_cfg.pool_info[4].type = HD_COMMON_MEM_USER_BLK;
    mem_cfg.pool_info[4].blk_size = MD_BUF_SIZE * 2;  // YUV420: 1.5x, but allocate 2x for alignment
    mem_cfg.pool_info[4].blk_cnt = 1;
    mem_cfg.pool_info[4].ddr_id = DDR_ID0;
    
    // Pool 5: User block for MDBC temp/model buffer (48x frame size)
    mem_cfg.pool_info[5].type = HD_COMMON_MEM_USER_BLK;
    mem_cfg.pool_info[5].blk_size = MD_BUF_SIZE * 48;
    mem_cfg.pool_info[5].blk_cnt = 1;
    mem_cfg.pool_info[5].ddr_id = DDR_ID0;
    
    // Pool 6: User block for MD output buffer (foreground mask)
    mem_cfg.pool_info[6].type = HD_COMMON_MEM_USER_BLK;
    mem_cfg.pool_info[6].blk_size = MD_BUF_SIZE;
    mem_cfg.pool_info[6].blk_cnt = 1;
    mem_cfg.pool_info[6].ddr_id = DDR_ID0;
    
    printf("[MEMORY] Pool0: RAW10 %dx%d, Pool1: YUV420 %dx%d, Pool2: MD YUV %dx%d\n",
           CAP_WIDTH, CAP_HEIGHT, MAIN_WIDTH, MAIN_HEIGHT, MD_WIDTH, MD_HEIGHT);
    printf("[MEMORY] Pool3: OSG stamp buffer (%u bytes)\n", osg_stamp_size);
    printf("[MEMORY] Pool4-6: User blocks for libmd processing\n");
    
    return hd_common_mem_init(&mem_cfg);
}

static HD_RESULT setup_videocap(DEMO_CONTEXT* ctx)
{
    HD_RESULT ret;
    HD_VIDEOCAP_DRV_CONFIG cap_cfg = {0};
    HD_VIDEOCAP_CTRL cap_ctrl = {0};
    HD_VIDEOCAP_IN cap_in = {0};
    HD_VIDEOCAP_OUT cap_out = {0};
    
    // Open control path
    ret = hd_videocap_open(0, HD_VIDEOCAP_CTRL(VCAP_ID), &ctx->cap_ctrl);
    if (ret != HD_OK) return ret;
    
    // Configure sensor driver
    snprintf(cap_cfg.sen_cfg.sen_dev.driver_name, HD_VIDEOCAP_SEN_NAME_LEN-1, SENSOR_NAME);
    cap_cfg.sen_cfg.sen_dev.if_type = HD_COMMON_VIDEO_IN_MIPI_CSI;
    cap_cfg.sen_cfg.sen_dev.pin_cfg.pinmux.sensor_pinmux = 0;
    cap_cfg.sen_cfg.sen_dev.pin_cfg.clk_lane_sel = HD_VIDEOCAP_SEN_CLANE_CSI(0, 0);
    cap_cfg.sen_cfg.sen_dev.pin_cfg.sen_2_serial_pin_map[0] = 0;
    cap_cfg.sen_cfg.sen_dev.pin_cfg.sen_2_serial_pin_map[1] = 1;
    for (int i = 2; i < 8; i++) {
        cap_cfg.sen_cfg.sen_dev.pin_cfg.sen_2_serial_pin_map[i] = HD_VIDEOCAP_SEN_IGNORE;
    }
    
    ret = hd_videocap_set(ctx->cap_ctrl, HD_VIDEOCAP_PARAM_DRV_CONFIG, &cap_cfg);
    if (ret != HD_OK) return ret;
    
    // Enable AE/AWB
    cap_ctrl.func = HD_VIDEOCAP_FUNC_AE | HD_VIDEOCAP_FUNC_AWB;
    ret = hd_videocap_set(ctx->cap_ctrl, HD_VIDEOCAP_PARAM_CTRL, &cap_ctrl);
    if (ret != HD_OK) return ret;
    
    // Open data path
    ret = hd_videocap_open(HD_VIDEOCAP_IN(VCAP_ID, 0), HD_VIDEOCAP_OUT(VCAP_ID, 0), &ctx->cap_path);
    if (ret != HD_OK) return ret;
    
    // Configure input - use full sensor resolution
    cap_in.sen_mode = HD_VIDEOCAP_SEN_MODE_AUTO;
    cap_in.frc = HD_VIDEO_FRC_RATIO(MAIN_FPS, 1);
    cap_in.dim.w = CAP_WIDTH;
    cap_in.dim.h = CAP_HEIGHT;
    cap_in.pxlfmt = SEN_OUT_FMT;
    cap_in.out_frame_num = HD_VIDEOCAP_SEN_FRAME_NUM_1;
    ret = hd_videocap_set(ctx->cap_path, HD_VIDEOCAP_PARAM_IN, &cap_in);
    if (ret != HD_OK) return ret;
    
    // Configure output (no crop)
    cap_out.pxlfmt = CAP_OUT_FMT;
    cap_out.dir = HD_VIDEO_DIR_NONE;
    ret = hd_videocap_set(ctx->cap_path, HD_VIDEOCAP_PARAM_OUT, &cap_out);
    
    return ret;
}

static HD_RESULT setup_videoproc(DEMO_CONTEXT* ctx)
{
    HD_RESULT ret;
    HD_VIDEOPROC_DEV_CONFIG proc_cfg = {0};
    HD_VIDEOPROC_CTRL proc_ctrl = {0};
    HD_VIDEOPROC_OUT proc_out = {0};
    
    // Open control path
    ret = hd_videoproc_open(0, HD_VIDEOPROC_0_CTRL, &ctx->proc_ctrl);
    if (ret != HD_OK) {
        printf("  Failed to open proc ctrl: %d\n", ret);
        return ret;
    }
    
    // Configure device - input from sensor at full resolution
    proc_cfg.pipe = HD_VIDEOPROC_PIPE_RAWALL;
    proc_cfg.isp_id = VCAP_ID;
    proc_cfg.ctrl_max.func = 0;  // No extra functions at max level
    proc_cfg.in_max.func = 0;
    proc_cfg.in_max.dim.w = CAP_WIDTH;
    proc_cfg.in_max.dim.h = CAP_HEIGHT;
    proc_cfg.in_max.pxlfmt = CAP_OUT_FMT;
    proc_cfg.in_max.frc = HD_VIDEO_FRC_RATIO(1, 1);
    ret = hd_videoproc_set(ctx->proc_ctrl, HD_VIDEOPROC_PARAM_DEV_CONFIG, &proc_cfg);
    if (ret != HD_OK) {
        printf("  Failed to set proc dev config: %d\n", ret);
        return ret;
    }
    
    // Basic control (no 3DNR for now to match working sample)
    proc_ctrl.func = 0;
    ret = hd_videoproc_set(ctx->proc_ctrl, HD_VIDEOPROC_PARAM_CTRL, &proc_ctrl);
    if (ret != HD_OK) {
        printf("  Failed to set proc ctrl: %d\n", ret);
        return ret;
    }
    
    // Open main output path - PULL mode for drawing bboxes before encoding
    ret = hd_videoproc_open(HD_VIDEOPROC_0_IN_0, HD_VIDEOPROC_0_OUT_0, &ctx->proc_path_main);
    if (ret != HD_OK) {
        printf("  Failed to open proc main path: %d\n", ret);
        return ret;
    }
    
    // Configure main output for BIND mode (OSG overlay on encoder, not pull/push)
    proc_out.func = 0;
    proc_out.dim.w = MAIN_WIDTH;
    proc_out.dim.h = MAIN_HEIGHT;
    proc_out.pxlfmt = HD_VIDEO_PXLFMT_YUV420;
    proc_out.dir = HD_VIDEO_DIR_NONE;
    proc_out.frc = HD_VIDEO_FRC_RATIO(1, 1);
    proc_out.depth = 0;  // Normal bind mode (not pull)
    ret = hd_videoproc_set(ctx->proc_path_main, HD_VIDEOPROC_PARAM_OUT, &proc_out);
    if (ret != HD_OK) {
        printf("  Failed to set proc main out: %d\n", ret);
        return ret;
    }
    
    printf("[VIDEOPROC] Main path: %dx%d YUV420, BIND mode with OSG overlay\n", MAIN_WIDTH, MAIN_HEIGHT);
    
    // Open MD output path (OUT_1 at 160x120 for libmd processing)
    ret = hd_videoproc_open(HD_VIDEOPROC_0_IN_0, HD_VIDEOPROC_0_OUT_1, &ctx->proc_path_md);
    if (ret != HD_OK) {
        printf("  Failed to open proc MD path: %d\n", ret);
        return ret;
    }
    
    // Configure MD path output - small resolution for efficient MD processing
    HD_VIDEOPROC_OUT md_out = {0};
    md_out.func = 0;  // No special function needed for pull
    md_out.dim.w = MD_WIDTH;
    md_out.dim.h = MD_HEIGHT;
    md_out.pxlfmt = HD_VIDEO_PXLFMT_YUV420;
    md_out.dir = HD_VIDEO_DIR_NONE;
    md_out.frc = HD_VIDEO_FRC_RATIO(1, 1);
    md_out.depth = 1;  // Buffer depth for pull
    ret = hd_videoproc_set(ctx->proc_path_md, HD_VIDEOPROC_PARAM_OUT, &md_out);
    if (ret != HD_OK) {
        printf("  Failed to set proc MD out: %d\n", ret);
        return ret;
    }
    
    printf("[VIDEOPROC] MD path opened at %dx%d for libmd processing\n", MD_WIDTH, MD_HEIGHT);
    
    return ret;
}

static HD_RESULT setup_videoenc(DEMO_CONTEXT* ctx)
{
    HD_RESULT ret;
    HD_VIDEOENC_PATH_CONFIG enc_cfg = {0};
    HD_VIDEOENC_IN enc_in = {0};
    HD_VIDEOENC_OUT enc_out = {0};
    HD_H26XENC_RATE_CONTROL rc = {0};
    
    // Open encoder path
    ret = hd_videoenc_open(HD_VIDEOENC_0_IN_0, HD_VIDEOENC_0_OUT_0, &ctx->enc_path);
    if (ret != HD_OK) return ret;
    
    // Configure path
    enc_cfg.max_mem.codec_type = HD_CODEC_TYPE_H265;
    enc_cfg.max_mem.max_dim.w = MAIN_WIDTH;
    enc_cfg.max_mem.max_dim.h = MAIN_HEIGHT;
    enc_cfg.max_mem.bitrate = MAIN_BITRATE;
    enc_cfg.max_mem.enc_buf_ms = 3000;
    enc_cfg.max_mem.svc_layer = HD_SVC_DISABLE;
    enc_cfg.max_mem.ltr = FALSE;
    enc_cfg.max_mem.rotate = FALSE;
    enc_cfg.max_mem.source_output = FALSE;
    enc_cfg.isp_id = VCAP_ID;
    ret = hd_videoenc_set(ctx->enc_path, HD_VIDEOENC_PARAM_PATH_CONFIG, &enc_cfg);
    if (ret != HD_OK) return ret;
    
    // Configure input
    enc_in.dir = HD_VIDEO_DIR_NONE;
    enc_in.pxl_fmt = HD_VIDEO_PXLFMT_YUV420;
    enc_in.dim.w = MAIN_WIDTH;
    enc_in.dim.h = MAIN_HEIGHT;
    enc_in.frc = HD_VIDEO_FRC_RATIO(1, 1);
    ret = hd_videoenc_set(ctx->enc_path, HD_VIDEOENC_PARAM_IN, &enc_in);
    if (ret != HD_OK) return ret;
    
    // Configure H.265 encoding
    enc_out.codec_type = HD_CODEC_TYPE_H265;
    enc_out.h26x.profile = HD_H265E_MAIN_PROFILE;
    enc_out.h26x.level_idc = HD_H265E_LEVEL_5;
    enc_out.h26x.gop_num = MAIN_FPS;  // 1 second GOP
    enc_out.h26x.ltr_interval = 0;
    enc_out.h26x.ltr_pre_ref = 0;
    enc_out.h26x.gray_en = 0;
    enc_out.h26x.source_output = 0;
    enc_out.h26x.svc_layer = HD_SVC_DISABLE;
    enc_out.h26x.entropy_mode = HD_H265E_CABAC_CODING;
    ret = hd_videoenc_set(ctx->enc_path, HD_VIDEOENC_PARAM_OUT_ENC_PARAM, &enc_out);
    if (ret != HD_OK) return ret;
    
    // Configure rate control with Motion AQ
    rc.rc_mode = HD_RC_MODE_EVBR;  // EVBR for best motion handling
    rc.evbr.bitrate = MAIN_BITRATE;
    rc.evbr.frame_rate_base = MAIN_FPS;
    rc.evbr.frame_rate_incr = 1;
    rc.evbr.init_i_qp = 26;
    rc.evbr.max_i_qp = 45;
    rc.evbr.min_i_qp = 15;
    rc.evbr.init_p_qp = 28;
    rc.evbr.max_p_qp = 48;
    rc.evbr.min_p_qp = 15;
    rc.evbr.static_time = 3;           // Enter static mode after 3s no motion
    rc.evbr.ip_weight = 0;
    rc.evbr.key_p_period = MAIN_FPS * 2;
    rc.evbr.kp_weight = 0;
    rc.evbr.still_frame_cnd = 100;
    rc.evbr.motion_ratio_thd = 30;
    rc.evbr.motion_aq_str = -6;        // *** MOTION AQ ENABLED ***
    rc.evbr.still_i_qp = 32;
    rc.evbr.still_p_qp = 38;
    rc.evbr.still_kp_qp = 35;
    
    ret = hd_videoenc_set(ctx->enc_path, HD_VIDEOENC_PARAM_OUT_RATE_CONTROL, &rc);
    
    printf("[ENCODER] Motion AQ enabled: motion_aq_str = %d\n", rc.evbr.motion_aq_str);
    printf("[ENCODER] EVBR mode: %d Mbps, GOP=%d\n", MAIN_BITRATE / 1000000, MAIN_FPS);
    
    // Open stamp path for OSG overlay
    ret = hd_videoenc_open(HD_VIDEOENC_0_IN_0, HD_STAMP_0, &ctx->stamp_path);
    if (ret != HD_OK) {
        printf("[ERROR] Failed to open stamp path: %d\n", ret);
        return ret;
    }
    printf("[OSG] Stamp path opened on VIDEOENC_0\n");
    
    return ret;
}

// ============================================================================
// OSG Stamp Setup - Allocate and configure hardware overlay buffer
// ============================================================================
static HD_RESULT setup_osg_stamp(DEMO_CONTEXT* ctx)
{
    HD_RESULT ret;
    UINTPTR pa;
    
    // Allocate CPU-side buffer for drawing bounding boxes
    ctx->stamp_buffer = (unsigned short*)malloc(STAMP_WIDTH * STAMP_HEIGHT * sizeof(unsigned short));
    if (!ctx->stamp_buffer) {
        printf("[OSG] Failed to allocate stamp_buffer\n");
        return HD_ERR_NG;
    }
    
    // Clear to transparent (0x0000 = fully transparent in ARGB4444)
    memset(ctx->stamp_buffer, 0, STAMP_WIDTH * STAMP_HEIGHT * sizeof(unsigned short));
    
    // Get OSG buffer from OSG pool
    ctx->stamp_blk = hd_common_mem_get_block(HD_COMMON_MEM_OSG_POOL, ctx->stamp_size, DDR_ID0);
    if (ctx->stamp_blk == HD_COMMON_MEM_VB_INVALID_BLK) {
        printf("[OSG] Failed to get OSG block\n");
        return HD_ERR_NG;
    }
    
    // Translate to physical address
    pa = hd_common_mem_blk2pa(ctx->stamp_blk);
    if (pa == 0) {
        printf("[OSG] Failed to get PA from OSG block\n");
        return HD_ERR_NG;
    }
    ctx->stamp_pa = pa;
    
    // Check 64-byte alignment
    if (ctx->stamp_pa & 0x3f) {
        printf("[OSG] Warning: stamp_pa (0x%lx) is not 64-byte aligned\n", (unsigned long)ctx->stamp_pa);
    }
    
    printf("[OSG] Stamp buffer: PA=0x%lX, size=%u bytes\n", (unsigned long)ctx->stamp_pa, ctx->stamp_size);
    
    // Configure stamp buffer
    HD_OSG_STAMP_BUF buf = {0};
    buf.type = HD_OSG_BUF_TYPE_PING_PONG;
    buf.p_addr = ctx->stamp_pa;
    buf.size = ctx->stamp_size;
    
    ret = hd_videoenc_set(ctx->stamp_path, HD_VIDEOENC_PARAM_IN_STAMP_BUF, &buf);
    if (ret != HD_OK) {
        printf("[OSG] Failed to set stamp buffer: %d\n", ret);
        return ret;
    }
    
    // Configure stamp image - ARGB4444 for transparency
    HD_OSG_STAMP_IMG img = {0};
    img.fmt = HD_VIDEO_PXLFMT_ARGB4444;
    img.dim.w = STAMP_WIDTH;
    img.dim.h = STAMP_HEIGHT;
    img.p_addr = (uintptr_t)ctx->stamp_buffer;  // Initial image from CPU buffer
    
    ret = hd_videoenc_set(ctx->stamp_path, HD_VIDEOENC_PARAM_IN_STAMP_IMG, &img);
    if (ret != HD_OK) {
        printf("[OSG] Failed to set stamp image: %d\n", ret);
        return ret;
    }
    
    // Configure stamp attributes (position, alpha)
    HD_OSG_STAMP_ATTR attr = {0};
    attr.position.x = 0;  // Full screen overlay
    attr.position.y = 0;
    attr.alpha = 255;     // Full opacity (individual pixel alpha from ARGB4444)
    
    ret = hd_videoenc_set(ctx->stamp_path, HD_VIDEOENC_PARAM_IN_STAMP_ATTR, &attr);
    if (ret != HD_OK) {
        printf("[OSG] Failed to set stamp attr: %d\n", ret);
        return ret;
    }
    
    ctx->osg_initialized = 1;
    printf("[OSG] Hardware overlay initialized: %dx%d ARGB4444\n", STAMP_WIDTH, STAMP_HEIGHT);
    
    return HD_OK;
}

// ============================================================================
// MD Setup - Initialize libmd and allocate working buffers
// ============================================================================
static HD_RESULT setup_md(DEMO_CONTEXT* ctx)
{
    HD_RESULT ret;
    
    // Initialize vendor MD library
    ret = vendor_md_init();
    if (ret != HD_OK) {
        printf("[ERROR] vendor_md_init failed: %d\n", ret);
        return ret;
    }
    printf("[MD] vendor_md_init OK\n");
    
    // Allocate MD memory blocks
    ret = alloc_mem_block(&ctx->md_src, MD_BUF_SIZE * 2, "MD_SRC");
    if (ret != HD_OK) return ret;
    
    ret = alloc_mem_block(&ctx->md_temp, MD_BUF_SIZE * 48, "MD_TEMP");
    if (ret != HD_OK) return ret;
    
    ret = alloc_mem_block(&ctx->md_dst, MD_BUF_SIZE, "MD_DST");
    if (ret != HD_OK) return ret;
    
    // Configure MDBC parameters
    memset(&ctx->mdbc_ctrl, 0, sizeof(ctx->mdbc_ctrl));
    ctx->mdbc_ctrl.enSensiLevel = (LIB_MD_BC_SENSI_LEVEL)ctx->sensitivity;
    ctx->mdbc_ctrl.enConvgSpd = LIB_MD_BC_MED_SPD;
    ctx->mdbc_ctrl.enInFmt = LIB_MD_IMG_YUV420SP;
    ctx->mdbc_ctrl.enIsInit = LIB_MD_BC_INIT_MODE;
    ctx->mdbc_ctrl.enBgNum = LIB_MD_BG_NUM_8;
    ctx->mdbc_ctrl.stMorph[0].enMorph = LIB_MD_MORPH_ENABLE;
    ctx->mdbc_ctrl.stMorph[0].u8MorphThres = 4;
    ctx->mdbc_ctrl.stMorph[1].enMorph = LIB_MD_MORPH_ENABLE;
    ctx->mdbc_ctrl.stMorph[1].u8MorphThres = 0;
    ctx->mdbc_ctrl.stMorph[2].enMorph = LIB_MD_MORPH_ENABLE;
    ctx->mdbc_ctrl.stMorph[2].u8MorphThres = 8;
    ctx->mdbc_ctrl.stMorph[3].enMorph = LIB_MD_MORPH_BYPASS;
    ctx->mdbc_ctrl.stTempMem.u64Pa = ctx->md_temp.pa;
    ctx->mdbc_ctrl.stTempMem.u64Va = ctx->md_temp.va;
    ctx->mdbc_ctrl.stTempMem.u32Size = ctx->md_temp.size;
    ctx->mdbc_ctrl.u32TmpThres = 0;
    ctx->mdbc_initialized = 0;
    
    // Configure Global Alarm
    ctx->galarm_ctrl.u8AlarmThres = GLOBAL_ALARM_THRESHOLD;
    
    // Configure Sub-Region Alarm (4 quadrants)
    ctx->subalarm_ctrl.u8SubNum = NUM_SUB_REGIONS;
    ctx->subalarm_ctrl.pstSubParam = ctx->sub_params;
    
    // Top-left quadrant
    ctx->sub_params[0].enSubRegion = 1;
    ctx->sub_params[0].u32XStart = 0;
    ctx->sub_params[0].u32YStart = 0;
    ctx->sub_params[0].u32XEnd = MD_WIDTH / 2 - 1;
    ctx->sub_params[0].u32YEnd = MD_HEIGHT / 2 - 1;
    ctx->sub_params[0].u8SubAlarmThres = SUB_ALARM_THRESHOLD;
    
    // Top-right quadrant
    ctx->sub_params[1].enSubRegion = 1;
    ctx->sub_params[1].u32XStart = MD_WIDTH / 2;
    ctx->sub_params[1].u32YStart = 0;
    ctx->sub_params[1].u32XEnd = MD_WIDTH - 1;
    ctx->sub_params[1].u32YEnd = MD_HEIGHT / 2 - 1;
    ctx->sub_params[1].u8SubAlarmThres = SUB_ALARM_THRESHOLD;
    
    // Bottom-left quadrant
    ctx->sub_params[2].enSubRegion = 1;
    ctx->sub_params[2].u32XStart = 0;
    ctx->sub_params[2].u32YStart = MD_HEIGHT / 2;
    ctx->sub_params[2].u32XEnd = MD_WIDTH / 2 - 1;
    ctx->sub_params[2].u32YEnd = MD_HEIGHT - 1;
    ctx->sub_params[2].u8SubAlarmThres = SUB_ALARM_THRESHOLD;
    
    // Bottom-right quadrant
    ctx->sub_params[3].enSubRegion = 1;
    ctx->sub_params[3].u32XStart = MD_WIDTH / 2;
    ctx->sub_params[3].u32YStart = MD_HEIGHT / 2;
    ctx->sub_params[3].u32XEnd = MD_WIDTH - 1;
    ctx->sub_params[3].u32YEnd = MD_HEIGHT - 1;
    ctx->sub_params[3].u8SubAlarmThres = SUB_ALARM_THRESHOLD;
    
    // Configure Object Detection
    ctx->objdet_ctrl.u32ObjThres = 50;  // Minimum object size in pixels
    ctx->objdet_ctrl.enInRange = LIB_MD_IN_RANGE_0_1;
    ctx->objdet_ctrl.pstPtStk = ctx->obj_pt_stack;
    
    printf("[MD] Initialized with sensitivity: %s\n", get_sensitivity_name(ctx->sensitivity));
    printf("[MD] Sub-regions: 4 quadrants configured\n");
    printf("[MD] Global alarm threshold: %d%%\n", GLOBAL_ALARM_THRESHOLD);
    
    return HD_OK;
}

static void cleanup_md(DEMO_CONTEXT* ctx)
{
    free_mem_block(&ctx->md_src);
    free_mem_block(&ctx->md_temp);
    free_mem_block(&ctx->md_dst);
    vendor_md_uninit();
    printf("[MD] Cleanup complete\n");
}

// ============================================================================
// MD Processing Thread - Pulls YUV from MD path and runs libmd algorithms
// ============================================================================
static void* md_process_thread(void* arg)
{
    DEMO_CONTEXT* ctx = (DEMO_CONTEXT*)arg;
    HD_VIDEO_FRAME frame;
    HD_RESULT ret;
    LIB_MD_ERROR_CODE md_ret;
    UINT32 frame_count = 0;
    struct timeval tv_start, tv_end;
    
    // Image info structures for libmd
    MD_SRC_IMAGE_S src_img = {0};
    MD_DST_IMAGE_S dst_img = {0};
    
    src_img.u32Width = MD_WIDTH;
    src_img.u32Height = MD_HEIGHT;
    src_img.u32Stride = MD_WIDTH;
    
    dst_img.u32Width = MD_WIDTH;
    dst_img.u32Height = MD_HEIGHT;
    dst_img.u32Stride = MD_WIDTH;
    dst_img.u64Pa = ctx->md_dst.pa;
    dst_img.u64Va = ctx->md_dst.va;
    
    printf(COLOR_GREEN "[MD Thread] Started, waiting for pipeline...\n" COLOR_RESET);
    
    // Wait for flow to start
    while (g_flow_start == 0 && g_running) {
        usleep(100000);
    }
    
    if (!g_running) return NULL;
    
    g_md_running = 1;
    printf("[MD Thread] Processing at %dx%d resolution\n", MD_WIDTH, MD_HEIGHT);
    
    while (g_running && g_md_running) {
        // Pull YUV frame from MD path
        ret = hd_videoproc_pull_out_buf(ctx->proc_path_md, &frame, 100);  // 100ms timeout
        if (ret != HD_OK) {
            if (ret == HD_ERR_TIMEDOUT) continue;
            printf("[MD Thread] Pull error: %d\n", ret);
            continue;
        }
        
        frame_count++;
        gettimeofday(&tv_start, NULL);
        
        // Copy YUV data to our src buffer and flush cache
        UINTPTR frame_va = (UINTPTR)hd_common_mem_mmap(HD_COMMON_MEM_MEM_TYPE_CACHE,
                                                        frame.phy_addr[0], 
                                                        MD_BUF_SIZE * 3 / 2);
        if (frame_va) {
            memcpy((void*)ctx->md_src.va, (void*)frame_va, MD_BUF_SIZE * 3 / 2);
            hd_common_mem_munmap((void*)frame_va, MD_BUF_SIZE * 3 / 2);
        }
        
        hd_common_mem_flush_cache((void*)ctx->md_src.va, MD_BUF_SIZE * 2);
        
        src_img.u64Pa = ctx->md_src.pa;
        src_img.u64Va = ctx->md_src.va;
        
        // Set MDBC mode (init on first frame, then normal)
        if (!ctx->mdbc_initialized) {
            ctx->mdbc_ctrl.enIsInit = LIB_MD_BC_INIT_MODE;
        } else {
            ctx->mdbc_ctrl.enIsInit = LIB_MD_BC_NORM_MODE;
        }
        
        // Run MDBC (Background Comparison)
        md_ret = NVT_MD_MDBC(&src_img, &dst_img, &ctx->mdbc_ctrl);
        if (md_ret != LIB_MD_OK) {
            printf("[MD Thread] NVT_MD_MDBC error: %d\n", md_ret);
            hd_videoproc_release_out_buf(ctx->proc_path_md, &frame);
            continue;
        }
        
        if (!ctx->mdbc_initialized) {
            ctx->mdbc_initialized = 1;
            printf("[MD Thread] MDBC background model initialized\n");
            hd_videoproc_release_out_buf(ctx->proc_path_md, &frame);
            continue;  // Skip detection on first frame
        }
        
        // Run Global Alarm detection
        UINT8 global_alarm = 0;
        md_ret = NVT_MD_GlobalAlarm(&dst_img, &global_alarm, &ctx->galarm_ctrl);
        if (md_ret == LIB_MD_OK && global_alarm) {
            pthread_mutex_lock(&g_stats_mutex);
            g_stats.global_alarms++;
            g_stats.motion_frames++;
            pthread_mutex_unlock(&g_stats_mutex);
        }
        
        // Run Sub-Region Alarm detection
        UINT8 sub_alarms[NUM_SUB_REGIONS] = {0};
        md_ret = NVT_MD_SubAlarm(&dst_img, sub_alarms, &ctx->subalarm_ctrl);
        if (md_ret == LIB_MD_OK) {
            pthread_mutex_lock(&g_stats_mutex);
            for (int i = 0; i < NUM_SUB_REGIONS; i++) {
                if (sub_alarms[i]) {
                    g_stats.sub_alarms[i]++;
                    if (!global_alarm) g_stats.motion_frames++;
                }
            }
            pthread_mutex_unlock(&g_stats_mutex);
        }
        
        // Run Object Detection with Temporal Tracking
        memset(&ctx->obj_info, 0, sizeof(ctx->obj_info));
        md_ret = NVT_MD_ObjDet(&dst_img, &ctx->obj_info, &ctx->objdet_ctrl);
        
        pthread_mutex_lock(&g_objects_mutex);
        
        float scale_x = (float)MAIN_WIDTH / MD_WIDTH;
        float scale_y = (float)MAIN_HEIGHT / MD_HEIGHT;
        
        // Compute UNION bounding box of all detected objects
        // This single box expands to include all motion regions
        float union_min_x = MAIN_WIDTH;
        float union_min_y = MAIN_HEIGHT;
        float union_max_x = 0;
        float union_max_y = 0;
        int has_valid_objects = 0;
        
        if (md_ret == LIB_MD_OK && ctx->obj_info.u32ObjNum > 0) {
            pthread_mutex_lock(&g_stats_mutex);
            if (ctx->obj_info.u32ObjNum > g_stats.max_objects) {
                g_stats.max_objects = ctx->obj_info.u32ObjNum;
            }
            pthread_mutex_unlock(&g_stats_mutex);
            
            // Find union of all objects that pass size filter
            for (UINT32 i = 0; i < ctx->obj_info.u32ObjNum; i++) {
                MD_OBJ_RST_S* obj = &ctx->obj_info.stObjRst[i];
                
                UINT32 scaled_w = (UINT32)((obj->u32XEnd - obj->u32XStart) * scale_x);
                UINT32 scaled_h = (UINT32)((obj->u32YEnd - obj->u32YStart) * scale_y);
                
                // Skip objects smaller than minimum size
                if (scaled_w < MIN_OBJECT_WIDTH || scaled_h < MIN_OBJECT_HEIGHT) {
                    continue;
                }
                
                float obj_x1 = obj->u32XStart * scale_x;
                float obj_y1 = obj->u32YStart * scale_y;
                float obj_x2 = obj->u32XEnd * scale_x;
                float obj_y2 = obj->u32YEnd * scale_y;
                
                // Expand union to include this object
                if (obj_x1 < union_min_x) union_min_x = obj_x1;
                if (obj_y1 < union_min_y) union_min_y = obj_y1;
                if (obj_x2 > union_max_x) union_max_x = obj_x2;
                if (obj_y2 > union_max_y) union_max_y = obj_y2;
                has_valid_objects = 1;
            }
        }
        
        if (has_valid_objects) {
            // Clamp union box to frame boundaries
            if (union_min_x < 0) union_min_x = 0;
            if (union_min_y < 0) union_min_y = 0;
            if (union_max_x > MAIN_WIDTH) union_max_x = MAIN_WIDTH;
            if (union_max_y > MAIN_HEIGHT) union_max_y = MAIN_HEIGHT;
            
            // Set target to the union bounding box
            g_unified_bbox.target_x = union_min_x;
            g_unified_bbox.target_y = union_min_y;
            g_unified_bbox.target_w = union_max_x - union_min_x;
            g_unified_bbox.target_h = union_max_y - union_min_y;
            g_unified_bbox.holdoff_count = 0;
            
            if (!g_unified_bbox.valid) {
                // First detection - initialize position directly
                g_unified_bbox.x = g_unified_bbox.target_x;
                g_unified_bbox.y = g_unified_bbox.target_y;
                g_unified_bbox.w = g_unified_bbox.target_w;
                g_unified_bbox.h = g_unified_bbox.target_h;
                g_unified_bbox.confirm_count = 1;
                g_unified_bbox.valid = 1;
            } else {
                // Increment confirm count
                if (g_unified_bbox.confirm_count < OBJECT_CONFIRM_FRAMES) {
                    g_unified_bbox.confirm_count++;
                }
            }
        } else {
            // No motion detected - increment holdoff
            if (g_unified_bbox.valid) {
                g_unified_bbox.holdoff_count++;
                if (g_unified_bbox.holdoff_count >= OBJECT_HOLDOFF_FRAMES) {
                    g_unified_bbox.valid = 0;
                    g_unified_bbox.confirm_count = 0;
                }
            }
        }
        
        // Apply smooth interpolation toward target position
        if (g_unified_bbox.valid) {
            g_unified_bbox.x += (g_unified_bbox.target_x - g_unified_bbox.x) * SMOOTH_FACTOR;
            g_unified_bbox.y += (g_unified_bbox.target_y - g_unified_bbox.y) * SMOOTH_FACTOR;
            g_unified_bbox.w += (g_unified_bbox.target_w - g_unified_bbox.w) * SMOOTH_FACTOR;
            g_unified_bbox.h += (g_unified_bbox.target_h - g_unified_bbox.h) * SMOOTH_FACTOR;
        }
        
        // Output the unified box if confirmed
        if (g_unified_bbox.valid && g_unified_bbox.confirm_count >= OBJECT_CONFIRM_FRAMES) {
            g_shared_objects.objects[0].x = (UINT32)(g_unified_bbox.x + 0.5f);
            g_shared_objects.objects[0].y = (UINT32)(g_unified_bbox.y + 0.5f);
            g_shared_objects.objects[0].w = (UINT32)(g_unified_bbox.w + 0.5f);
            g_shared_objects.objects[0].h = (UINT32)(g_unified_bbox.h + 0.5f);
            g_shared_objects.obj_count = 1;
        } else {
            g_shared_objects.obj_count = 0;
        }
        
        pthread_mutex_unlock(&g_objects_mutex);
        
        gettimeofday(&tv_end, NULL);
        UINT32 md_time_us = (tv_end.tv_sec - tv_start.tv_sec) * 1000000 + 
                            (tv_end.tv_usec - tv_start.tv_usec);
        
        pthread_mutex_lock(&g_stats_mutex);
        g_stats.total_frames++;
        g_stats.total_md_time_us += md_time_us;
        if (g_stats.min_md_time_us == 0 || md_time_us < g_stats.min_md_time_us) {
            g_stats.min_md_time_us = md_time_us;
        }
        if (md_time_us > g_stats.max_md_time_us) {
            g_stats.max_md_time_us = md_time_us;
        }
        pthread_mutex_unlock(&g_stats_mutex);
        
        // Release frame back to videoproc
        hd_videoproc_release_out_buf(ctx->proc_path_md, &frame);
    }
    
    printf(COLOR_GREEN "[MD Thread] Stopped, processed %u frames\n" COLOR_RESET, frame_count);
    return NULL;
}

// ============================================================================
// Status Monitor Thread - Shows REAL MD stats from md_process_thread
// ============================================================================
static void* status_thread(void* arg)
{
    DEMO_CONTEXT* ctx = (DEMO_CONTEXT*)arg;
    int second = 0;
    UINT32 last_motion_frames = 0;
    UINT32 last_global_alarms = 0;
    UINT32 last_total_frames = 0;
    
    (void)ctx;
    
    printf(COLOR_GREEN "[Status Thread] Started - Showing REAL MD statistics\n" COLOR_RESET);
    
    while (g_running) {
        sleep(1);
        second++;
        
        pthread_mutex_lock(&g_stats_mutex);
        
        // Calculate per-second stats
        UINT32 frames_this_sec = g_stats.total_frames - last_total_frames;
        UINT32 motion_this_sec = g_stats.motion_frames - last_motion_frames;
        UINT32 alarms_this_sec = g_stats.global_alarms - last_global_alarms;
        
        last_total_frames = g_stats.total_frames;
        last_motion_frames = g_stats.motion_frames;
        last_global_alarms = g_stats.global_alarms;
        
        float motion_pct = frames_this_sec > 0 ? (100.0f * motion_this_sec / frames_this_sec) : 0;
        
        pthread_mutex_unlock(&g_stats_mutex);
        
        // Print status line
        printf(COLOR_CYAN "[%3d sec] " COLOR_RESET, second);
        printf("MD: %u fps, ", frames_this_sec);
        
        if (alarms_this_sec > 0) {
            printf(COLOR_RED "MOTION %.0f%%" COLOR_RESET, motion_pct);
        } else if (motion_this_sec > 0) {
            printf(COLOR_YELLOW "motion %.0f%%" COLOR_RESET, motion_pct);
        } else {
            printf(COLOR_GREEN "static" COLOR_RESET);
        }
        
        printf(", " COLOR_WHITE "Motion AQ: ACTIVE" COLOR_RESET "\n");
        
        // Show detailed events
        pthread_mutex_lock(&g_stats_mutex);
        if (alarms_this_sec > 0) {
            printf(COLOR_RED "         ⚠ GLOBAL ALARM! Motion in %u frames\n" COLOR_RESET, motion_this_sec);
        }
        
        // Check sub-region activity (show recent)
        for (int i = 0; i < NUM_SUB_REGIONS; i++) {
            if (g_stats.sub_alarms[i] > 0 && (second % 5 == 0)) {
                static const char* zone_names[] = {"Top-Left", "Top-Right", "Bottom-Left", "Bottom-Right"};
                printf(COLOR_YELLOW "         ► Zone %d (%s): %u alarms total\n" COLOR_RESET, 
                       i, zone_names[i], g_stats.sub_alarms[i]);
            }
        }
        
        if (g_stats.max_objects > 0 && (second % 3 == 0)) {
            printf(COLOR_MAGENTA "         ◆ Objects: up to %u detected\n" COLOR_RESET, g_stats.max_objects);
        }
        pthread_mutex_unlock(&g_stats_mutex);
    }
    
    printf(COLOR_GREEN "[Status Thread] Stopped\n" COLOR_RESET);
    return NULL;
}

// ============================================================================
// Draw Bounding Boxes on OSG ARGB4444 Buffer - Hardware Overlay
// Draws rectangles directly on the stamp buffer, which is then hardware-blended
// ============================================================================

// Helper: Draw a horizontal line on ARGB4444 buffer
static void draw_hline_argb4444(unsigned short* buf, int width, int height,
                                 int x1, int x2, int y, unsigned short color)
{
    if (y < 0 || y >= height) return;
    if (x1 < 0) x1 = 0;
    if (x2 >= width) x2 = width - 1;
    if (x1 > x2) return;
    
    for (int x = x1; x <= x2; x++) {
        buf[y * width + x] = color;
    }
}

// Helper: Draw a vertical line on ARGB4444 buffer
static void draw_vline_argb4444(unsigned short* buf, int width, int height,
                                 int x, int y1, int y2, unsigned short color)
{
    if (x < 0 || x >= width) return;
    if (y1 < 0) y1 = 0;
    if (y2 >= height) y2 = height - 1;
    if (y1 > y2) return;
    
    for (int y = y1; y <= y2; y++) {
        buf[y * width + x] = color;
    }
}

// Draw a hollow rectangle with specified thickness on ARGB4444 buffer
static void draw_rect_argb4444(unsigned short* buf, int width, int height,
                                int x, int y, int w, int h, 
                                unsigned short color, int thickness)
{
    // Clamp to buffer bounds
    if (x < 0) { w += x; x = 0; }
    if (y < 0) { h += y; y = 0; }
    if (x + w > width) w = width - x;
    if (y + h > height) h = height - y;
    if (w <= 0 || h <= 0) return;
    
    // Draw thick borders
    for (int t = 0; t < thickness; t++) {
        // Top horizontal line
        draw_hline_argb4444(buf, width, height, x, x + w - 1, y + t, color);
        // Bottom horizontal line
        draw_hline_argb4444(buf, width, height, x, x + w - 1, y + h - 1 - t, color);
        // Left vertical line
        draw_vline_argb4444(buf, width, height, x + t, y, y + h - 1, color);
        // Right vertical line
        draw_vline_argb4444(buf, width, height, x + w - 1 - t, y, y + h - 1, color);
    }
}

// Update OSG stamp buffer with current bounding boxes
static void update_osg_overlay(DEMO_CONTEXT* ctx)
{
    SHARED_OBJECTS local_objs;
    
    // Copy shared objects to local
    pthread_mutex_lock(&g_objects_mutex);
    memcpy(&local_objs, &g_shared_objects, sizeof(SHARED_OBJECTS));
    pthread_mutex_unlock(&g_objects_mutex);
    
    // Clear buffer to transparent
    memset(ctx->stamp_buffer, 0, STAMP_WIDTH * STAMP_HEIGHT * sizeof(unsigned short));
    
    // Draw each bounding box
    for (UINT32 i = 0; i < local_objs.obj_count; i++) {
        draw_rect_argb4444(ctx->stamp_buffer, STAMP_WIDTH, STAMP_HEIGHT,
                           local_objs.objects[i].x,
                           local_objs.objects[i].y,
                           local_objs.objects[i].w,
                           local_objs.objects[i].h,
                           BBOX_COLOR_FAINT,
                           BBOX_THICKNESS);
    }
    
    // Update the stamp image to hardware
    HD_OSG_STAMP_IMG img = {0};
    img.fmt = HD_VIDEO_PXLFMT_ARGB4444;
    img.dim.w = STAMP_WIDTH;
    img.dim.h = STAMP_HEIGHT;
    img.p_addr = (uintptr_t)ctx->stamp_buffer;
    
    HD_RESULT ret = hd_videoenc_set(ctx->stamp_path, HD_VIDEOENC_PARAM_IN_STAMP_IMG, &img);
    if (ret != HD_OK) {
        printf("[OSG] Failed to update stamp image: %d\n", ret);
    }
}

// ============================================================================
// Video Encode Thread - Pull encoded bitstream and write to file
// OSG overlay is hardware-blended, no YUV manipulation needed
// ============================================================================
static void* video_encode_thread(void* arg)
{
    DEMO_CONTEXT* ctx = (DEMO_CONTEXT*)arg;
    HD_VIDEOENC_BS bs_data;
    HD_RESULT ret;
    UINT32 total_bytes = 0;
    UINT32 frame_count = 0;
    UINT32 i_frames = 0;
    UINT32 p_frames = 0;
    UINT32 frames_with_overlay = 0;
    UINT32 last_obj_count = 0;
    
    printf(COLOR_GREEN "[Encode Thread] Started, waiting for flow_start...\n" COLOR_RESET);
    
    // Wait for pipeline to be fully started
    while (g_flow_start == 0 && g_running) {
        usleep(100000);  // 100ms
    }
    
    if (!g_running) return NULL;
    
    printf("[Encode Thread] Flow started, output: %s\n", OUTPUT_VIDEO_FILE);
    printf("[Encode Thread] Using hardware OSG overlay (zero CPU overhead)\n");
    
    // Get encoder buffer info (MUST be after hd_videoenc_start!)
    ret = hd_videoenc_get(ctx->enc_path, HD_VIDEOENC_PARAM_BUFINFO, &ctx->enc_buf_info);
    if (ret != HD_OK) {
        printf(COLOR_RED "[Encode Thread] Failed to get buffer info: %d\n" COLOR_RESET, ret);
        return NULL;
    }
    
    printf("[Encode Thread] Buffer info: PA=0x%lX, size=%u\n", 
           (unsigned long)ctx->enc_buf_info.buf_info.phy_addr,
           ctx->enc_buf_info.buf_info.buf_size);
    
    ctx->enc_vir_addr = (UINTPTR)hd_common_mem_mmap(HD_COMMON_MEM_MEM_TYPE_CACHE,
                                                     ctx->enc_buf_info.buf_info.phy_addr,
                                                     ctx->enc_buf_info.buf_info.buf_size);
    
    if (!ctx->enc_vir_addr) {
        printf(COLOR_RED "[Encode Thread] Failed to mmap encoder buffer!\n" COLOR_RESET);
        return NULL;
    }
    
    while (g_running) {
        // Update OSG overlay with current bounding boxes
        pthread_mutex_lock(&g_objects_mutex);
        UINT32 obj_count = g_shared_objects.obj_count;
        pthread_mutex_unlock(&g_objects_mutex);
        
        // Only update overlay when object count changes or we have objects
        if (obj_count > 0 || last_obj_count > 0) {
            update_osg_overlay(ctx);
            if (obj_count > 0) frames_with_overlay++;
        }
        last_obj_count = obj_count;
        
        // Pull encoded bitstream from encoder (bind mode - frames flow automatically)
        ret = hd_videoenc_pull_out_buf(ctx->enc_path, &bs_data, 100);  // 100ms timeout
        if (ret != HD_OK) {
            if (ret == HD_ERR_TIMEDOUT) continue;
            printf("[Encode Thread] Pull BS error: %d\n", ret);
            continue;
        }
        
        frame_count++;
        
        // Count I-frames vs P-frames based on size
        UINT32 frame_size = 0;
        for (UINT32 j = 0; j < bs_data.pack_num; j++) {
            frame_size += bs_data.video_pack[j].size;
        }
        if (frame_size > (MAIN_BITRATE / MAIN_FPS / 8) * 5) {
            i_frames++;
        } else {
            p_frames++;
        }
        
        // Write to file
        for (UINT32 i = 0; i < bs_data.pack_num; i++) {
            UINTPTR virt_addr = ctx->enc_vir_addr + 
                               (bs_data.video_pack[i].phy_addr - ctx->enc_buf_info.buf_info.phy_addr);
            UINT32 size = bs_data.video_pack[i].size;
            
            if (ctx->video_file) {
                fwrite((void*)virt_addr, 1, size, ctx->video_file);
                fflush(ctx->video_file);
                total_bytes += size;
            }
        }
        
        hd_videoenc_release_out_buf(ctx->enc_path, &bs_data);
        
        // Progress indicator every 30 frames
        if (frame_count % 30 == 0) {
            float mbps = (total_bytes * 8.0f) / (1000000.0f * frame_count / MAIN_FPS);
            printf(COLOR_WHITE "         Encoded: %u frames (%u with overlay), %.2f Mbps\n" COLOR_RESET,
                   frame_count, frames_with_overlay, mbps);
        }
    }
    
    if (ctx->enc_vir_addr) {
        hd_common_mem_munmap((void*)ctx->enc_vir_addr, ctx->enc_buf_info.buf_info.buf_size);
    }
    
    printf(COLOR_GREEN "[Encode Thread] Stopped\n" COLOR_RESET);
    printf("  Total frames: %u (I:%u P:%u)\n", frame_count, i_frames, p_frames);
    printf("  Frames with overlay: %u\n", frames_with_overlay);
    printf("  Total bytes:  %u (%.2f MB)\n", total_bytes, total_bytes / (1024.0f * 1024.0f));
    printf("  Avg bitrate:  %.2f Mbps\n", 
           frame_count > 0 ? (total_bytes * 8.0f) / (1000000.0f * frame_count / MAIN_FPS) : 0);
    
    return NULL;
}

// ============================================================================
// Main
// ============================================================================
MAIN(argc, argv)
{
    HD_RESULT ret;
    DEMO_CONTEXT ctx = {0};
    pthread_t status_tid, encode_tid;
    
    // Parse arguments
    ctx.duration = (argc > 1) ? atoi(argv[1]) : 30;
    ctx.sensitivity = (argc > 2) ? atoi(argv[2]) : 2;  // Default: HIGH
    
    if (ctx.sensitivity < 0 || ctx.sensitivity > 3) {
        ctx.sensitivity = 2;
    }
    
    // Setup signal handler
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    
    print_banner();
    printf("Configuration:\n");
    printf("  Duration:    %d seconds\n", ctx.duration);
    printf("  Sensitivity: %s\n", get_sensitivity_name(ctx.sensitivity));
    printf("  Resolution:  %dx%d @ %d fps\n", MAIN_WIDTH, MAIN_HEIGHT, MAIN_FPS);
    printf("  Bitrate:     %d Mbps\n", MAIN_BITRATE / 1000000);
    printf("  Overlay:     Hardware OSG (ARGB4444, %d px thick RED boxes)\n", BBOX_THICKNESS);
    printf("  Output:      %s\n", OUTPUT_VIDEO_FILE);
    printf("\n");
    
    // Query OSG stamp buffer size
    printf("[INIT] Querying OSG buffer size...\n");
    ctx.stamp_size = query_osg_buf_size();
    if (ctx.stamp_size <= 0) {
        printf("[ERROR] Failed to query OSG buffer size\n");
        return 1;
    }
    printf("  [OK] OSG stamp size: %u bytes\n", ctx.stamp_size);
    
    // Initialize HDAL
    printf("[INIT] Initializing HDAL...\n");
    ret = hd_common_init(0);
    if (ret != HD_OK) {
        printf("[ERROR] hd_common_init failed: %d\n", ret);
        return 1;
    }
    
    ret = init_memory(ctx.stamp_size);
    if (ret != HD_OK) {
        printf("[ERROR] Memory init failed: %d\n", ret);
        goto cleanup;
    }
    
    ret = hd_videocap_init();
    if (ret != HD_OK) goto cleanup;
    
    ret = hd_videoproc_init();
    if (ret != HD_OK) goto cleanup;
    
    ret = hd_videoenc_init();
    if (ret != HD_OK) goto cleanup;
    
    // Setup pipeline
    printf("[INIT] Setting up video capture...\n");
    ret = setup_videocap(&ctx);
    if (ret != HD_OK) {
        printf("[ERROR] Videocap setup failed: %d\n", ret);
        goto cleanup;
    }
    
    printf("[INIT] Setting up video processing (with hardware MD)...\n");
    ret = setup_videoproc(&ctx);
    if (ret != HD_OK) {
        printf("[ERROR] Videoproc setup failed: %d\n", ret);
        goto cleanup;
    }
    
    printf("[INIT] Setting up video encoder (EVBR with Motion AQ + OSG stamp)...\n");
    ret = setup_videoenc(&ctx);
    if (ret != HD_OK) {
        printf("[ERROR] Videoenc setup failed: %d\n", ret);
        goto cleanup;
    }
    
    // Setup OSG overlay
    printf("[INIT] Setting up OSG hardware overlay...\n");
    ret = setup_osg_stamp(&ctx);
    if (ret != HD_OK) {
        printf("[ERROR] OSG setup failed: %d\n", ret);
        goto cleanup;
    }
    
    // Setup MD processing (libmd)
    printf("[INIT] Setting up motion detection (libmd)...\n");
    ret = setup_md(&ctx);
    if (ret != HD_OK) {
        printf("[ERROR] MD setup failed: %d\n", ret);
        goto cleanup;
    }
    
    // Open output file
    ctx.video_file = fopen(OUTPUT_VIDEO_FILE, "wb");
    if (!ctx.video_file) {
        printf("[ERROR] Cannot open output file: %s\n", OUTPUT_VIDEO_FILE);
        goto cleanup;
    }
    
    // Bind pipeline - full bind including videoproc->encoder (OSG is on encoder)
    printf("[INIT] Binding pipeline...\n");
    ret = hd_videocap_bind(HD_VIDEOCAP_OUT(VCAP_ID, 0), HD_VIDEOPROC_0_IN_0);
    if (ret != HD_OK) {
        printf("  Failed to bind videocap->videoproc: %d\n", ret);
        goto cleanup;
    }
    printf("  [OK] Videocap->Videoproc bound\n");
    
    ret = hd_videoproc_bind(HD_VIDEOPROC_0_OUT_0, HD_VIDEOENC_0_IN_0);
    if (ret != HD_OK) {
        printf("  Failed to bind videoproc->videoenc: %d\n", ret);
        goto cleanup;
    }
    printf("  [OK] Videoproc->Videoenc bound (OSG overlay on encoder)\n");
    
    // Create threads FIRST (they will wait for flow_start)
    printf("[INIT] Creating worker threads...\n");
    pthread_t md_tid;
    pthread_create(&status_tid, NULL, status_thread, &ctx);
    pthread_create(&encode_tid, NULL, video_encode_thread, &ctx);
    pthread_create(&md_tid, NULL, md_process_thread, &ctx);
    
    // Start pipeline in correct order
    printf("[INIT] Starting pipeline...\n");
    ret = hd_videocap_start(ctx.cap_path);
    if (ret != HD_OK) {
        printf("  Failed to start videocap: %d\n", ret);
        goto cleanup;
    }
    printf("  [OK] Videocap started\n");
    
    ret = hd_videoproc_start(ctx.proc_path_main);
    if (ret != HD_OK) {
        printf("  Failed to start videoproc main: %d\n", ret);
        goto cleanup;
    }
    printf("  [OK] Videoproc main started\n");
    
    ret = hd_videoproc_start(ctx.proc_path_md);
    if (ret != HD_OK) {
        printf("  Failed to start videoproc MD: %d\n", ret);
        goto cleanup;
    }
    printf("  [OK] Videoproc MD path started\n");
    
    // Wait for AE/AWB to stabilize
    printf("[INIT] Waiting for AE/AWB to stabilize...\n");
    sleep(1);
    
    // Now start encoder and stamp path
    ret = hd_videoenc_start(ctx.stamp_path);
    if (ret != HD_OK) {
        printf("  Failed to start stamp path: %d\n", ret);
        goto cleanup;
    }
    printf("  [OK] Stamp path started (OSG overlay active)\n");
    
    ret = hd_videoenc_start(ctx.enc_path);
    if (ret != HD_OK) {
        printf("  Failed to start videoenc: %d\n", ret);
        goto cleanup;
    }
    printf("  [OK] Videoenc started\n");
    
    // Signal threads to start processing
    g_flow_start = 1;
    
    printf("\n");
    printf(COLOR_CYAN "╔══════════════════════════════════════════════════════════════╗\n" COLOR_RESET);
    printf(COLOR_CYAN "║" COLOR_GREEN "  Pipeline running! Press Ctrl+C to stop.                     " COLOR_CYAN "║\n" COLOR_RESET);
    printf(COLOR_CYAN "║" COLOR_RED "  Hardware OSG overlay ACTIVE - RED boxes on motion           " COLOR_CYAN "║\n" COLOR_RESET);
    printf(COLOR_CYAN "╚══════════════════════════════════════════════════════════════╝\n" COLOR_RESET);
    printf("\n");
    
    // Run for specified duration
    for (int i = 0; i < ctx.duration && g_running; i++) {
        sleep(1);
    }
    
    g_running = 0;
    g_md_running = 0;
    
    // Wait for threads
    pthread_join(md_tid, NULL);
    pthread_join(status_tid, NULL);
    pthread_join(encode_tid, NULL);
    
    // Print final statistics
    print_statistics();
    
cleanup:
    printf("\n" COLOR_YELLOW "[CLEANUP] Stopping pipeline...\n" COLOR_RESET);
    
    // Stop pipeline
    if (ctx.stamp_path) hd_videoenc_stop(ctx.stamp_path);
    if (ctx.enc_path) hd_videoenc_stop(ctx.enc_path);
    if (ctx.proc_path_md) hd_videoproc_stop(ctx.proc_path_md);
    if (ctx.proc_path_main) hd_videoproc_stop(ctx.proc_path_main);
    if (ctx.cap_path) hd_videocap_stop(ctx.cap_path);
    
    // Unbind
    hd_videoproc_unbind(HD_VIDEOPROC_0_OUT_0);
    hd_videocap_unbind(HD_VIDEOCAP_OUT(VCAP_ID, 0));
    
    // Close paths
    if (ctx.stamp_path) hd_videoenc_close(ctx.stamp_path);
    if (ctx.enc_path) hd_videoenc_close(ctx.enc_path);
    if (ctx.proc_path_md) hd_videoproc_close(ctx.proc_path_md);
    if (ctx.proc_path_main) hd_videoproc_close(ctx.proc_path_main);
    if (ctx.proc_ctrl) hd_videoproc_close(ctx.proc_ctrl);
    if (ctx.cap_path) hd_videocap_close(ctx.cap_path);
    if (ctx.cap_ctrl) hd_videocap_close(ctx.cap_ctrl);
    
    // Cleanup MD resources
    cleanup_md(&ctx);
    
    // Cleanup OSG resources
    if (ctx.stamp_buffer) {
        free(ctx.stamp_buffer);
        ctx.stamp_buffer = NULL;
    }
    if (ctx.stamp_blk != HD_COMMON_MEM_VB_INVALID_BLK) {
        hd_common_mem_release_block(ctx.stamp_blk);
    }
    printf("  [OK] OSG resources released\n");
    
    // Uninit HDAL
    hd_videoenc_uninit();
    hd_videoproc_uninit();
    hd_videocap_uninit();
    hd_common_mem_uninit();
    hd_common_uninit();
    
    // Close files
    if (ctx.video_file) fclose(ctx.video_file);
    
    printf(COLOR_GREEN "[DONE] Demo complete!\n" COLOR_RESET);
    printf("Output video: %s\n", OUTPUT_VIDEO_FILE);
    printf("Play with: ffplay -i %s\n", OUTPUT_VIDEO_FILE);
    
    return 0;
}
