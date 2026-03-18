/**
 * @file ai_fdcnn_demo.c
 * @brief FDCNN (Face Detection CNN) Demo with Live Stream
 * 
 * This demo demonstrates real-time face detection using FDCNN
 * with live camera stream input. Features:
 * - Live camera capture and video processing
 * - FDCNN-based face detection with landmark points
 * - Face bounding boxes with confidence scores
 * - Real-time processing with configurable sensitivity
 * 
 * Usage: ./ai_fdcnn_demo [duration_seconds]
 * 
 * @author IP Camera Team
 * @date 2026
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>
#include <sys/time.h>

#include "hdal.h"
#include "hd_debug.h"
#include "vendor_videoprocess.h"
#include "fdcnn_lib.h"
#include "ext_module_api.h"

// ============================================================================
// Configuration
// ============================================================================
#define SENSOR_NAME         "nvt_sen_gc5603"
#define VCAP_ID             0
#define CAP_WIDTH           2960
#define CAP_HEIGHT          1664
#define SEN_OUT_FMT         HD_VIDEO_PXLFMT_RAW10
#define CAP_OUT_FMT         HD_VIDEO_PXLFMT_RAW10

#define MAIN_WIDTH          1920
#define MAIN_HEIGHT         1080
#define MAIN_FPS            30

#define AI_WIDTH            640
#define AI_HEIGHT           480

#define VENDOR_AI_CFG       0x000f0000
#define FDCNN_BUFSIZE       (0x5FFE80)

#define DBGINFO_BUFSIZE()   (0x200)
#define VDO_RAW_BUFSIZE(w, h, pxlfmt)   (ALIGN_CEIL_4((w) * HD_VIDEO_PXLFMT_BPP(pxlfmt) / 8) * (h))
#define VDO_YUV_BUFSIZE(w, h, pxlfmt)   (ALIGN_CEIL_4((w) * HD_VIDEO_PXLFMT_BPP(pxlfmt) / 8) * (h))
#define AI_RGB_BUFSIZE(w, h)   (ALIGN_CEIL_4((w) * HD_VIDEO_PXLFMT_BPP(HD_VIDEO_PXLFMT_RGB888_PLANAR) / 8) * (h))

// ============================================================================
// Global State
// ============================================================================
static volatile int g_running = 1;
static volatile int g_faces_detected = 0;

typedef struct {
    HD_PATH_ID cap_ctrl;
    HD_PATH_ID cap_path;
    HD_PATH_ID proc_ctrl;
    HD_PATH_ID proc_path_main;
    HD_PATH_ID proc_path_ai;
    
    NN_CFG_BUF_M fd_mem;
    NN_CFG_BUF_M input_mem;
    
    int duration;
} DEMO_CONTEXT;

typedef struct {
    uintptr_t pa;
    uintptr_t va;
    UINT32 size;
    HD_COMMON_MEM_VB_BLK blk;
} MEM_BUF;

static MEM_BUF g_mem[2];

// ============================================================================
// Signal Handler
// ============================================================================
static void signal_handler(int sig)
{
    printf("\n[SIGNAL] Caught signal %d, stopping...\n", sig);
    g_running = 0;
}

// ============================================================================
// Memory Management
// ============================================================================
static HD_RESULT init_memory(void)
{
    HD_COMMON_MEM_INIT_CONFIG mem_cfg = {0};
    
    // Pool 0: RAW video capture
    mem_cfg.pool_info[0].type = HD_COMMON_MEM_COMMON_POOL;
    mem_cfg.pool_info[0].blk_size = DBGINFO_BUFSIZE() + VDO_RAW_BUFSIZE(CAP_WIDTH, CAP_HEIGHT, CAP_OUT_FMT);
    mem_cfg.pool_info[0].blk_cnt = 3;
    mem_cfg.pool_info[0].ddr_id = DDR_ID0;
    
    // Pool 1: YUV video processing
    mem_cfg.pool_info[1].type = HD_COMMON_MEM_COMMON_POOL;
    mem_cfg.pool_info[1].blk_size = DBGINFO_BUFSIZE() + VDO_YUV_BUFSIZE(MAIN_WIDTH, MAIN_HEIGHT, HD_VIDEO_PXLFMT_YUV420);
    mem_cfg.pool_info[1].blk_cnt = 3;
    mem_cfg.pool_info[1].ddr_id = DDR_ID0;
    
    // Pool 2: AI input buffer
    mem_cfg.pool_info[2].type = HD_COMMON_MEM_COMMON_POOL;
    mem_cfg.pool_info[2].blk_size = DBGINFO_BUFSIZE() + VDO_YUV_BUFSIZE(AI_WIDTH, AI_HEIGHT, HD_VIDEO_PXLFMT_YUV420);
    mem_cfg.pool_info[2].blk_cnt = 2;
    mem_cfg.pool_info[2].ddr_id = DDR_ID0;
    
    // Pool 3: FDCNN working buffer
    mem_cfg.pool_info[3].type = HD_COMMON_MEM_USER_DEFINIED_POOL;
    mem_cfg.pool_info[3].blk_size = FDCNN_BUFSIZE;
    mem_cfg.pool_info[3].blk_cnt = 1;
    mem_cfg.pool_info[3].ddr_id = DDR_ID0;
    
    // Pool 4: Input buffer
    mem_cfg.pool_info[4].type = HD_COMMON_MEM_USER_DEFINIED_POOL;
    mem_cfg.pool_info[4].blk_size = AI_RGB_BUFSIZE(AI_WIDTH, AI_HEIGHT);
    mem_cfg.pool_info[4].blk_cnt = 1;
    mem_cfg.pool_info[4].ddr_id = DDR_ID0;
    
    return hd_common_mem_init(&mem_cfg);
}

static HD_RESULT get_ai_mem(DEMO_CONTEXT* ctx)
{
    g_mem[0].blk = hd_common_mem_get_block(HD_COMMON_MEM_USER_DEFINIED_POOL, FDCNN_BUFSIZE, DDR_ID0);
    if (g_mem[0].blk == HD_COMMON_MEM_VB_INVALID_BLK) {
        printf("[ERROR] Failed to get FDCNN memory block\n");
        return HD_ERR_NG;
    }
    g_mem[0].pa = hd_common_mem_blk2pa(g_mem[0].blk);
    g_mem[0].va = (uintptr_t)hd_common_mem_mmap(HD_COMMON_MEM_MEM_TYPE_CACHE, g_mem[0].pa, FDCNN_BUFSIZE);
    g_mem[0].size = FDCNN_BUFSIZE;
    
    ctx->fd_mem.pa = g_mem[0].pa;
    ctx->fd_mem.va = g_mem[0].va;
    ctx->fd_mem.size = g_mem[0].size;
    
    g_mem[1].blk = hd_common_mem_get_block(HD_COMMON_MEM_USER_DEFINIED_POOL, AI_RGB_BUFSIZE(AI_WIDTH, AI_HEIGHT), DDR_ID0);
    if (g_mem[1].blk == HD_COMMON_MEM_VB_INVALID_BLK) {
        printf("[ERROR] Failed to get input memory block\n");
        return HD_ERR_NG;
    }
    g_mem[1].pa = hd_common_mem_blk2pa(g_mem[1].blk);
    g_mem[1].va = (uintptr_t)hd_common_mem_mmap(HD_COMMON_MEM_MEM_TYPE_CACHE, g_mem[1].pa, AI_RGB_BUFSIZE(AI_WIDTH, AI_HEIGHT));
    g_mem[1].size = AI_RGB_BUFSIZE(AI_WIDTH, AI_HEIGHT);
    
    ctx->input_mem.pa = g_mem[1].pa;
    ctx->input_mem.va = g_mem[1].va;
    ctx->input_mem.size = g_mem[1].size;
    
    printf("[MEM] FDCNN buffer: PA=0x%lx, size=%u\n", (unsigned long)ctx->fd_mem.pa, ctx->fd_mem.size);
    return HD_OK;
}

static void free_ai_mem(void)
{
    for (int i = 0; i < 2; i++) {
        if (g_mem[i].va) {
            hd_common_mem_munmap((void*)g_mem[i].va, g_mem[i].size);
        }
        if (g_mem[i].blk != HD_COMMON_MEM_VB_INVALID_BLK) {
            hd_common_mem_release_block(g_mem[i].blk);
        }
    }
}

// ============================================================================
// HDAL Pipeline Setup
// ============================================================================
static HD_RESULT setup_video_pipeline(DEMO_CONTEXT* ctx)
{
    HD_RESULT ret;
    HD_VIDEOCAP_DRV_CONFIG cap_cfg = {0};
    HD_VIDEOCAP_CTRL cap_ctrl = {0};
    HD_VIDEOCAP_IN cap_in = {0};
    HD_VIDEOCAP_OUT cap_out = {0};
    HD_VIDEOPROC_DEV_CONFIG proc_cfg = {0};
    HD_VIDEOPROC_CTRL proc_ctrl = {0};
    HD_VIDEOPROC_OUT proc_out = {0};
    
    // Video Capture Setup
    ret = hd_videocap_open(0, HD_VIDEOCAP_CTRL(VCAP_ID), &ctx->cap_ctrl);
    if (ret != HD_OK) return ret;
    
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
    
    cap_ctrl.func = HD_VIDEOCAP_FUNC_AE | HD_VIDEOCAP_FUNC_AWB;
    ret = hd_videocap_set(ctx->cap_ctrl, HD_VIDEOCAP_PARAM_CTRL, &cap_ctrl);
    if (ret != HD_OK) return ret;
    
    ret = hd_videocap_open(HD_VIDEOCAP_IN(VCAP_ID, 0), HD_VIDEOCAP_OUT(VCAP_ID, 0), &ctx->cap_path);
    if (ret != HD_OK) return ret;
    
    cap_in.sen_mode = HD_VIDEOCAP_SEN_MODE_AUTO;
    cap_in.frc = HD_VIDEO_FRC_RATIO(MAIN_FPS, 1);
    cap_in.dim.w = CAP_WIDTH;
    cap_in.dim.h = CAP_HEIGHT;
    cap_in.pxlfmt = SEN_OUT_FMT;
    cap_in.out_frame_num = HD_VIDEOCAP_SEN_FRAME_NUM_1;
    ret = hd_videocap_set(ctx->cap_path, HD_VIDEOCAP_PARAM_IN, &cap_in);
    if (ret != HD_OK) return ret;
    
    cap_out.pxlfmt = CAP_OUT_FMT;
    cap_out.dir = HD_VIDEO_DIR_NONE;
    ret = hd_videocap_set(ctx->cap_path, HD_VIDEOCAP_PARAM_OUT, &cap_out);
    if (ret != HD_OK) return ret;
    
    // Video Processing Setup
    ret = hd_videoproc_open(0, HD_VIDEOPROC_0_CTRL, &ctx->proc_ctrl);
    if (ret != HD_OK) return ret;
    
    proc_cfg.pipe = HD_VIDEOPROC_PIPE_RAWALL;
    proc_cfg.isp_id = VCAP_ID;
    proc_cfg.in_max.dim.w = CAP_WIDTH;
    proc_cfg.in_max.dim.h = CAP_HEIGHT;
    proc_cfg.in_max.pxlfmt = CAP_OUT_FMT;
    proc_cfg.in_max.frc = HD_VIDEO_FRC_RATIO(1, 1);
    ret = hd_videoproc_set(ctx->proc_ctrl, HD_VIDEOPROC_PARAM_DEV_CONFIG, &proc_cfg);
    if (ret != HD_OK) return ret;
    
    proc_ctrl.func = 0;
    ret = hd_videoproc_set(ctx->proc_ctrl, HD_VIDEOPROC_PARAM_CTRL, &proc_ctrl);
    if (ret != HD_OK) return ret;
    
    // Main output path
    ret = hd_videoproc_open(HD_VIDEOPROC_0_IN_0, HD_VIDEOPROC_0_OUT_0, &ctx->proc_path_main);
    if (ret != HD_OK) return ret;
    
    proc_out.dim.w = MAIN_WIDTH;
    proc_out.dim.h = MAIN_HEIGHT;
    proc_out.pxlfmt = HD_VIDEO_PXLFMT_YUV420;
    proc_out.dir = HD_VIDEO_DIR_NONE;
    proc_out.frc = HD_VIDEO_FRC_RATIO(1, 1);
    ret = hd_videoproc_set(ctx->proc_path_main, HD_VIDEOPROC_PARAM_OUT, &proc_out);
    if (ret != HD_OK) return ret;
    
    // AI input path
    ret = hd_videoproc_open(HD_VIDEOPROC_0_IN_0, HD_VIDEOPROC_0_OUT_1, &ctx->proc_path_ai);
    if (ret != HD_OK) return ret;
    
    proc_out.dim.w = AI_WIDTH;
    proc_out.dim.h = AI_HEIGHT;
    proc_out.depth = 1;
    ret = hd_videoproc_set(ctx->proc_path_ai, HD_VIDEOPROC_PARAM_OUT, &proc_out);
    if (ret != HD_OK) return ret;
    
    printf("[PIPELINE] Face detection pipeline: CAP(%dx%d) -> AI(%dx%d)\n",
           CAP_WIDTH, CAP_HEIGHT, AI_WIDTH, AI_HEIGHT);
    
    return HD_OK;
}

// ============================================================================
// Face Detection Thread
// ============================================================================
static void* face_detect_thread(void* arg)
{
    DEMO_CONTEXT* ctx = (DEMO_CONTEXT*)arg;
    HD_RESULT ret;
    UINT32 frame_count = 0;
    
    NN_FILE_PATH fd_files = {0};
    snprintf(fd_files.model_file, PATH_LENGTH_M, "/mnt/sd/CNNLib/para/fdcnn/nvt_model.bin");
    
    ret = fdcnn_init(ctx->fd_mem, 0, &fd_files, 0);
    if (ret != HD_OK) {
        printf("[FDCNN] Init failed: %d\n", ret);
        return NULL;
    }
    printf("[FDCNN] Initialized successfully\n");
    
    HD_URECT detect_size = {0, 0, AI_WIDTH, AI_HEIGHT};
    
    while (g_running) {
        HD_VIDEO_FRAME frame = {0};
        
        ret = hd_videoproc_pull_out_buf(ctx->proc_path_ai, &frame, 100);
        if (ret != HD_OK) continue;
        
        // Setup GFX image buffer
        HD_GFX_IMG_BUF input_image = {0};
        input_image.dim.w = AI_WIDTH;
        input_image.dim.h = AI_HEIGHT;
        input_image.format = HD_VIDEO_PXLFMT_YUV420;
        input_image.lineoffset[0] = ALIGN_CEIL_4(AI_WIDTH);
        input_image.lineoffset[1] = ALIGN_CEIL_4(AI_WIDTH);
        input_image.p_phy_addr[0] = frame.phy_addr[0];
        input_image.p_phy_addr[1] = frame.phy_addr[0] + AI_WIDTH * AI_HEIGHT;
        
        FDCNN_RESULT* fdcnn_results = NULL;
        UINT32 fdcnn_num = 0;
        
        ret = fdcnn_process(ctx->fd_mem, &input_image, &fdcnn_results, &detect_size, &fdcnn_num);
        
        if (ret == HD_OK && fdcnn_num > 0) {
            g_faces_detected += fdcnn_num;
            
            if (frame_count % 30 == 0) {
                printf("[FDCNN] Frame %u: %u faces detected\n", frame_count, fdcnn_num);
                for (UINT32 i = 0; i < fdcnn_num && i < 5; i++) {
                    printf("  [%u] Face: (%d,%d)-(%d,%d) conf=%.2f\n",
                           i, fdcnn_results[i].x, fdcnn_results[i].y,
                           fdcnn_results[i].x + fdcnn_results[i].w,
                           fdcnn_results[i].y + fdcnn_results[i].h,
                           fdcnn_results[i].score);
                    printf("       Landmarks: L-Eye(%d,%d) R-Eye(%d,%d) Nose(%d,%d)\n",
                           fdcnn_results[i].landmark_points[0].x, fdcnn_results[i].landmark_points[0].y,
                           fdcnn_results[i].landmark_points[1].x, fdcnn_results[i].landmark_points[1].y,
                           fdcnn_results[i].landmark_points[2].x, fdcnn_results[i].landmark_points[2].y);
                }
            }
        }
        
        hd_videoproc_release_out_buf(ctx->proc_path_ai, &frame);
        frame_count++;
    }
    
    fdcnn_uninit(ctx->fd_mem);
    printf("[FDCNN] Thread exit, total faces: %d\n", g_faces_detected);
    
    return NULL;
}

// ============================================================================
// Main Entry
// ============================================================================
static void print_banner(void)
{
    printf("\n");
    printf("╔═══════════════════════════════════════════════════════════╗\n");
    printf("║         NT98538 FDCNN (Face Detection CNN) Demo           ║\n");
    printf("╠═══════════════════════════════════════════════════════════╣\n");
    printf("║  Features:                                                ║\n");
    printf("║    ✓ Real-time face detection                            ║\n");
    printf("║    ✓ Facial landmark detection (5 points)                ║\n");
    printf("║    ✓ Confidence score output                             ║\n");
    printf("║    ✓ Live camera stream processing                       ║\n");
    printf("╚═══════════════════════════════════════════════════════════╝\n");
    printf("\n");
}

int main(int argc, char* argv[])
{
    HD_RESULT ret;
    DEMO_CONTEXT ctx = {0};
    pthread_t fd_thread_id;
    
    ctx.duration = (argc > 1) ? atoi(argv[1]) : 30;
    
    print_banner();
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    
    ret = hd_common_init(0);
    if (ret != HD_OK) {
        printf("[ERROR] hd_common_init failed: %d\n", ret);
        return -1;
    }
    
    hd_common_sysconfig(0, (1 << 16), 0, VENDOR_AI_CFG);
    
    ret = init_memory();
    if (ret != HD_OK) goto exit;
    
    ret = hd_gfx_init();
    if (ret != HD_OK) goto exit;
    
    VENDOR_AI3_DEV_CFG dev_cfg = {0};
    ret = vendor_ai3_dev_init(&dev_cfg);
    if (ret != HD_OK) goto exit;
    
    ret = get_ai_mem(&ctx);
    if (ret != HD_OK) goto exit;
    
    ret = setup_video_pipeline(&ctx);
    if (ret != HD_OK) goto exit;
    
    ret = hd_videocap_bind(HD_VIDEOCAP_OUT(VCAP_ID, 0), HD_VIDEOPROC_0_IN_0);
    if (ret != HD_OK) goto exit;
    
    ret = hd_videocap_start(ctx.cap_path);
    if (ret != HD_OK) goto exit;
    
    ret = hd_videoproc_start(ctx.proc_path_main);
    if (ret != HD_OK) goto exit;
    
    ret = hd_videoproc_start(ctx.proc_path_ai);
    if (ret != HD_OK) goto exit;
    
    pthread_create(&fd_thread_id, NULL, face_detect_thread, &ctx);
    
    printf("[MAIN] Running for %d seconds...\n", ctx.duration);
    
    for (int i = 0; i < ctx.duration && g_running; i++) {
        sleep(1);
        if ((i + 1) % 10 == 0) {
            printf("[STATUS] %d/%d seconds, faces detected: %d\n", i + 1, ctx.duration, g_faces_detected);
        }
    }
    
    g_running = 0;
    pthread_join(fd_thread_id, NULL);
    
    hd_videoproc_stop(ctx.proc_path_ai);
    hd_videoproc_stop(ctx.proc_path_main);
    hd_videocap_stop(ctx.cap_path);
    hd_videocap_unbind(HD_VIDEOCAP_OUT(VCAP_ID, 0));
    
exit:
    if (ctx.proc_path_ai) hd_videoproc_close(ctx.proc_path_ai);
    if (ctx.proc_path_main) hd_videoproc_close(ctx.proc_path_main);
    if (ctx.proc_ctrl) hd_videoproc_close(ctx.proc_ctrl);
    if (ctx.cap_path) hd_videocap_close(ctx.cap_path);
    if (ctx.cap_ctrl) hd_videocap_close(ctx.cap_ctrl);
    
    free_ai_mem();
    vendor_ai3_dev_uninit();
    hd_gfx_uninit();
    hd_common_mem_uninit();
    hd_common_uninit();
    
    printf("\n[DONE] Face Detection Demo completed. Total faces: %d\n", g_faces_detected);
    return 0;
}
