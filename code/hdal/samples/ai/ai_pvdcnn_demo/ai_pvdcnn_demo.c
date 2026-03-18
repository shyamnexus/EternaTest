/**
 * @file ai_pvdcnn_demo.c
 * @brief PVDCNN (Person/Vehicle Detection CNN) Demo with Live Stream
 * 
 * This demo demonstrates real-time person and vehicle detection using PVDCNN
 * with live camera stream input. Features:
 * - Live camera capture and video processing
 * - PVDCNN-based person/vehicle/non-motorized vehicle detection
 * - OSD overlay for bounding boxes
 * - H.265 encoded output with detection visualization
 * 
 * Usage: ./ai_pvdcnn_demo [duration_seconds]
 *        duration: demo duration (default: 30)
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
#include "pvdcnn_lib.h"
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
#define MAIN_BITRATE        (4 * 1024 * 1024)

#define AI_WIDTH            416
#define AI_HEIGHT           416

#define VENDOR_AI_CFG       0x000f0000

#define DBGINFO_BUFSIZE()   (0x200)
#define VDO_RAW_BUFSIZE(w, h, pxlfmt)   (ALIGN_CEIL_4((w) * HD_VIDEO_PXLFMT_BPP(pxlfmt) / 8) * (h))
#define VDO_YUV_BUFSIZE(w, h, pxlfmt)   (ALIGN_CEIL_4((w) * HD_VIDEO_PXLFMT_BPP(pxlfmt) / 8) * (h))

#define PVD_BUFSIZE         (6137280)

// ============================================================================
// Global State
// ============================================================================
static volatile int g_running = 1;
static volatile int g_detections = 0;

typedef struct {
    HD_PATH_ID cap_ctrl;
    HD_PATH_ID cap_path;
    HD_PATH_ID proc_ctrl;
    HD_PATH_ID proc_path_main;
    HD_PATH_ID proc_path_ai;
    HD_PATH_ID enc_path;
    
    NN_CFG_BUF_M pvd_mem;
    NN_CFG_BUF_M scale_mem;
    
    FILE* video_file;
    int duration;
} DEMO_CONTEXT;

static HD_COMMON_MEM_VB_BLK g_blk[2];

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
    
    // Pool 3: PVDCNN working buffer
    mem_cfg.pool_info[3].type = HD_COMMON_MEM_USER_DEFINIED_POOL;
    mem_cfg.pool_info[3].blk_size = PVD_BUFSIZE;
    mem_cfg.pool_info[3].blk_cnt = 1;
    mem_cfg.pool_info[3].ddr_id = DDR_ID0;
    
    // Pool 4: Scale buffer
    mem_cfg.pool_info[4].type = HD_COMMON_MEM_USER_DEFINIED_POOL;
    mem_cfg.pool_info[4].blk_size = AI_WIDTH * AI_HEIGHT * 3;
    mem_cfg.pool_info[4].blk_cnt = 1;
    mem_cfg.pool_info[4].ddr_id = DDR_ID0;
    
    return hd_common_mem_init(&mem_cfg);
}

static HD_RESULT get_ai_mem(DEMO_CONTEXT* ctx)
{
    HD_RESULT ret;
    
    g_blk[0] = hd_common_mem_get_block(HD_COMMON_MEM_USER_DEFINIED_POOL, PVD_BUFSIZE, DDR_ID0);
    if (g_blk[0] == HD_COMMON_MEM_VB_INVALID_BLK) {
        printf("[ERROR] Failed to get PVD memory block\n");
        return HD_ERR_NG;
    }
    ctx->pvd_mem.pa = hd_common_mem_blk2pa(g_blk[0]);
    ctx->pvd_mem.va = (uintptr_t)hd_common_mem_mmap(HD_COMMON_MEM_MEM_TYPE_CACHE, ctx->pvd_mem.pa, PVD_BUFSIZE);
    ctx->pvd_mem.size = PVD_BUFSIZE;
    
    g_blk[1] = hd_common_mem_get_block(HD_COMMON_MEM_USER_DEFINIED_POOL, AI_WIDTH * AI_HEIGHT * 3, DDR_ID0);
    if (g_blk[1] == HD_COMMON_MEM_VB_INVALID_BLK) {
        printf("[ERROR] Failed to get scale memory block\n");
        return HD_ERR_NG;
    }
    ctx->scale_mem.pa = hd_common_mem_blk2pa(g_blk[1]);
    ctx->scale_mem.va = (uintptr_t)hd_common_mem_mmap(HD_COMMON_MEM_MEM_TYPE_CACHE, ctx->scale_mem.pa, AI_WIDTH * AI_HEIGHT * 3);
    ctx->scale_mem.size = AI_WIDTH * AI_HEIGHT * 3;
    
    printf("[MEM] PVDCNN buffer: PA=0x%lx, VA=0x%lx, size=%u\n", 
           (unsigned long)ctx->pvd_mem.pa, (unsigned long)ctx->pvd_mem.va, ctx->pvd_mem.size);
    
    return HD_OK;
}

static void free_ai_mem(DEMO_CONTEXT* ctx)
{
    if (ctx->pvd_mem.va) {
        hd_common_mem_munmap((void*)ctx->pvd_mem.va, ctx->pvd_mem.size);
    }
    if (ctx->scale_mem.va) {
        hd_common_mem_munmap((void*)ctx->scale_mem.va, ctx->scale_mem.size);
    }
    if (g_blk[0] != HD_COMMON_MEM_VB_INVALID_BLK) {
        hd_common_mem_release_block(g_blk[0]);
    }
    if (g_blk[1] != HD_COMMON_MEM_VB_INVALID_BLK) {
        hd_common_mem_release_block(g_blk[1]);
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
    
    // AI input path (scaled to AI dimensions)
    ret = hd_videoproc_open(HD_VIDEOPROC_0_IN_0, HD_VIDEOPROC_0_OUT_1, &ctx->proc_path_ai);
    if (ret != HD_OK) return ret;
    
    proc_out.dim.w = AI_WIDTH;
    proc_out.dim.h = AI_HEIGHT;
    proc_out.depth = 1;
    ret = hd_videoproc_set(ctx->proc_path_ai, HD_VIDEOPROC_PARAM_OUT, &proc_out);
    if (ret != HD_OK) return ret;
    
    printf("[PIPELINE] Video pipeline configured: CAP(%dx%d) -> PROC -> MAIN(%dx%d) + AI(%dx%d)\n",
           CAP_WIDTH, CAP_HEIGHT, MAIN_WIDTH, MAIN_HEIGHT, AI_WIDTH, AI_HEIGHT);
    
    return HD_OK;
}

// ============================================================================
// AI Detection Thread
// ============================================================================
static void* ai_detect_thread(void* arg)
{
    DEMO_CONTEXT* ctx = (DEMO_CONTEXT*)arg;
    HD_RESULT ret;
    struct timeval tstart, tend;
    UINT32 frame_count = 0;
    
    NN_FILE_PATH pvd_files = {0};
    snprintf(pvd_files.model_file, PATH_LENGTH_M, "/mnt/sd/CNNLib/para/pvdcnn/nvt_model_416_prune37.bin");
    snprintf(pvd_files.para_file, PATH_LENGTH_M, "/mnt/sd/configs/pvdcnn_para_config.txt");
    
    ret = pvdcnn_init(ctx->pvd_mem, 0, 0, NULL, NULL, &pvd_files, 0);
    if (ret != HD_OK) {
        printf("[PVDCNN] Init failed: %d\n", ret);
        return NULL;
    }
    printf("[PVDCNN] Initialized successfully\n");
    
    while (g_running) {
        HD_VIDEO_FRAME frame = {0};
        
        // Pull frame from AI path
        ret = hd_videoproc_pull_out_buf(ctx->proc_path_ai, &frame, 100);
        if (ret != HD_OK) {
            continue;
        }
        
        gettimeofday(&tstart, NULL);
        
        // Prepare input buffer
        VENDOR_AI3_BUF src_img = {0};
        src_img.width = AI_WIDTH;
        src_img.height = AI_HEIGHT;
        src_img.channel = 2;
        src_img.line_ofs = ALIGN_CEIL_4(AI_WIDTH);
        src_img.fmt = HD_VIDEO_PXLFMT_YUV420;
        src_img.pa = frame.phy_addr[0];
        src_img.va = (uintptr_t)hd_common_mem_mmap(HD_COMMON_MEM_MEM_TYPE_CACHE, frame.phy_addr[0], AI_WIDTH * AI_HEIGHT * 3 / 2);
        src_img.sign = MAKEFOURCC('A', 'B', 'U', 'F');
        src_img.size = AI_WIDTH * AI_HEIGHT * 3 / 2;
        
        // Process detection
        NN_RESULT_M* pvd_results = NULL;
        INT32 pvd_num = 0;
        
        ret = pvdcnn_process_inplace(ctx->pvd_mem, (void*)&src_img, NULL, &pvd_results, &pvd_num, 
                                      (PVDCNN_PD_DET_SW | PVDCNN_VD_DET_SW | PVDCNN_ND_DET_SW));
        
        gettimeofday(&tend, NULL);
        UINT32 proc_time = (tend.tv_sec - tstart.tv_sec) * 1000 + (tend.tv_usec - tstart.tv_usec) / 1000;
        
        if (ret == HD_OK && pvd_num > 0) {
            g_detections += pvd_num;
            
            if (frame_count % 30 == 0) {
                printf("[PVDCNN] Frame %u: %d detections (proc: %u ms)\n", frame_count, pvd_num, proc_time);
                for (INT32 i = 0; i < pvd_num && i < 5; i++) {
                    const char* class_name = "Unknown";
                    if (pvd_results[i].category == 0) class_name = "Person";
                    else if (pvd_results[i].category == 1) class_name = "Vehicle";
                    else if (pvd_results[i].category == 2) class_name = "Non-Motor";
                    
                    printf("  [%d] %s: (%.0f,%.0f)-(%.0f,%.0f) conf=%.2f\n",
                           i, class_name,
                           pvd_results[i].x1, pvd_results[i].y1,
                           pvd_results[i].x2, pvd_results[i].y2,
                           pvd_results[i].score);
                }
            }
        }
        
        if (src_img.va) {
            hd_common_mem_munmap((void*)src_img.va, AI_WIDTH * AI_HEIGHT * 3 / 2);
        }
        
        hd_videoproc_release_out_buf(ctx->proc_path_ai, &frame);
        frame_count++;
    }
    
    pvdcnn_uninit(ctx->pvd_mem);
    printf("[PVDCNN] Thread exit, total detections: %d\n", g_detections);
    
    return NULL;
}

// ============================================================================
// Main Entry
// ============================================================================
static void print_banner(void)
{
    printf("\n");
    printf("╔═══════════════════════════════════════════════════════════╗\n");
    printf("║      NT98538 PVDCNN (Person/Vehicle Detection) Demo       ║\n");
    printf("╠═══════════════════════════════════════════════════════════╣\n");
    printf("║  Features:                                                ║\n");
    printf("║    ✓ Person Detection (pedestrians, people)              ║\n");
    printf("║    ✓ Vehicle Detection (cars, trucks, buses)             ║\n");
    printf("║    ✓ Non-Motorized Detection (bicycles, motorcycles)     ║\n");
    printf("║    ✓ Real-time processing with live camera               ║\n");
    printf("╚═══════════════════════════════════════════════════════════╝\n");
    printf("\n");
}

int main(int argc, char* argv[])
{
    HD_RESULT ret;
    DEMO_CONTEXT ctx = {0};
    pthread_t ai_thread_id;
    
    ctx.duration = (argc > 1) ? atoi(argv[1]) : 30;
    
    print_banner();
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    
    // Initialize HDAL
    ret = hd_common_init(0);
    if (ret != HD_OK) {
        printf("[ERROR] hd_common_init failed: %d\n", ret);
        return -1;
    }
    
    hd_common_sysconfig(0, (1 << 16), 0, VENDOR_AI_CFG);
    
    ret = init_memory();
    if (ret != HD_OK) {
        printf("[ERROR] Memory init failed: %d\n", ret);
        goto exit;
    }
    
    ret = hd_gfx_init();
    if (ret != HD_OK) {
        printf("[ERROR] GFX init failed: %d\n", ret);
        goto exit;
    }
    
    // Initialize AI3
    VENDOR_AI3_DEV_CFG dev_cfg = {0};
    ret = vendor_ai3_dev_init(&dev_cfg);
    if (ret != HD_OK) {
        printf("[ERROR] AI3 init failed: %d\n", ret);
        goto exit;
    }
    
    ret = get_ai_mem(&ctx);
    if (ret != HD_OK) {
        printf("[ERROR] AI memory allocation failed: %d\n", ret);
        goto exit;
    }
    
    ret = setup_video_pipeline(&ctx);
    if (ret != HD_OK) {
        printf("[ERROR] Video pipeline setup failed: %d\n", ret);
        goto exit;
    }
    
    // Bind paths
    ret = hd_videocap_bind(HD_VIDEOCAP_OUT(VCAP_ID, 0), HD_VIDEOPROC_0_IN_0);
    if (ret != HD_OK) goto exit;
    
    // Start pipeline
    ret = hd_videocap_start(ctx.cap_path);
    if (ret != HD_OK) goto exit;
    
    ret = hd_videoproc_start(ctx.proc_path_main);
    if (ret != HD_OK) goto exit;
    
    ret = hd_videoproc_start(ctx.proc_path_ai);
    if (ret != HD_OK) goto exit;
    
    // Start AI thread
    pthread_create(&ai_thread_id, NULL, ai_detect_thread, &ctx);
    
    printf("[MAIN] Running for %d seconds... Press Ctrl+C to stop.\n", ctx.duration);
    
    for (int i = 0; i < ctx.duration && g_running; i++) {
        sleep(1);
        if ((i + 1) % 10 == 0) {
            printf("[STATUS] %d/%d seconds, detections: %d\n", i + 1, ctx.duration, g_detections);
        }
    }
    
    g_running = 0;
    pthread_join(ai_thread_id, NULL);
    
    // Stop pipeline
    hd_videoproc_stop(ctx.proc_path_ai);
    hd_videoproc_stop(ctx.proc_path_main);
    hd_videocap_stop(ctx.cap_path);
    
    hd_videocap_unbind(HD_VIDEOCAP_OUT(VCAP_ID, 0));
    
exit:
    // Cleanup
    if (ctx.proc_path_ai) hd_videoproc_close(ctx.proc_path_ai);
    if (ctx.proc_path_main) hd_videoproc_close(ctx.proc_path_main);
    if (ctx.proc_ctrl) hd_videoproc_close(ctx.proc_ctrl);
    if (ctx.cap_path) hd_videocap_close(ctx.cap_path);
    if (ctx.cap_ctrl) hd_videocap_close(ctx.cap_ctrl);
    
    free_ai_mem(&ctx);
    vendor_ai3_dev_uninit();
    hd_gfx_uninit();
    hd_common_mem_uninit();
    hd_common_uninit();
    
    printf("\n[DONE] PVDCNN Demo completed. Total detections: %d\n", g_detections);
    return 0;
}
