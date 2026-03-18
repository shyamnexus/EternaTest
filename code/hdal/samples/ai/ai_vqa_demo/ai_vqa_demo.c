/**
 * @file ai_vqa_demo.c
 * @brief Video Quality Analysis Demo - Blur, Overexposure, Underexposure Detection
 * Usage: ./ai_vqa_demo [duration]
 * @author IP Camera Team
 * @date 2026
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>
#include "hdal.h"
#include "hd_debug.h"
#include "vqa_lib.h"

#define SENSOR_NAME     "nvt_sen_gc5603"
#define VCAP_ID         0
#define CAP_WIDTH       2960
#define CAP_HEIGHT      1664
#define MAIN_WIDTH      1920
#define MAIN_HEIGHT     1080
#define MAIN_FPS        30
#define VQA_WIDTH       320
#define VQA_HEIGHT      180
#define VQA_BUF_SIZE    (VQA_WIDTH * VQA_HEIGHT)

#define DBGINFO_BUFSIZE()   (0x200)
#define VDO_RAW_BUFSIZE(w, h, pxlfmt)   (ALIGN_CEIL_4((w) * HD_VIDEO_PXLFMT_BPP(pxlfmt) / 8) * (h))
#define VDO_YUV_BUFSIZE(w, h, pxlfmt)   (ALIGN_CEIL_4((w) * HD_VIDEO_PXLFMT_BPP(pxlfmt) / 8) * (h))

static volatile int g_running = 1;
static volatile int g_issues = 0;

typedef struct {
    HD_PATH_ID cap_ctrl, cap_path, proc_ctrl, proc_path_main, proc_path_vqa;
    UINTPTR vqa_buf_va, blur_buf_va;
    HD_COMMON_MEM_VB_BLK vqa_blk, blur_blk;
    int duration;
} DEMO_CTX;

static void signal_handler(int sig) { g_running = 0; }

static HD_RESULT init_memory(void) {
    HD_COMMON_MEM_INIT_CONFIG m = {0};
    m.pool_info[0].type = HD_COMMON_MEM_COMMON_POOL;
    m.pool_info[0].blk_size = DBGINFO_BUFSIZE() + VDO_RAW_BUFSIZE(CAP_WIDTH, CAP_HEIGHT, HD_VIDEO_PXLFMT_RAW10);
    m.pool_info[0].blk_cnt = 3; m.pool_info[0].ddr_id = DDR_ID0;
    m.pool_info[1].type = HD_COMMON_MEM_COMMON_POOL;
    m.pool_info[1].blk_size = DBGINFO_BUFSIZE() + VDO_YUV_BUFSIZE(MAIN_WIDTH, MAIN_HEIGHT, HD_VIDEO_PXLFMT_YUV420);
    m.pool_info[1].blk_cnt = 3; m.pool_info[1].ddr_id = DDR_ID0;
    m.pool_info[2].type = HD_COMMON_MEM_COMMON_POOL;
    m.pool_info[2].blk_size = DBGINFO_BUFSIZE() + VDO_YUV_BUFSIZE(VQA_WIDTH, VQA_HEIGHT, HD_VIDEO_PXLFMT_YUV420);
    m.pool_info[2].blk_cnt = 3; m.pool_info[2].ddr_id = DDR_ID0;
    m.pool_info[3].type = HD_COMMON_MEM_COMMON_POOL;
    m.pool_info[3].blk_size = NVT_VQA_Calc_buf_size(VQA_WIDTH, VQA_HEIGHT);
    m.pool_info[3].blk_cnt = 1; m.pool_info[3].ddr_id = DDR_ID0;
    m.pool_info[4].type = HD_COMMON_MEM_COMMON_POOL;
    m.pool_info[4].blk_size = VQA_BUF_SIZE;
    m.pool_info[4].blk_cnt = 1; m.pool_info[4].ddr_id = DDR_ID0;
    return hd_common_mem_init(&m);
}

static HD_RESULT setup_vqa(DEMO_CTX* ctx) {
    ctx->vqa_blk = hd_common_mem_get_block(HD_COMMON_MEM_COMMON_POOL, NVT_VQA_Calc_buf_size(VQA_WIDTH, VQA_HEIGHT), DDR_ID0);
    if (ctx->vqa_blk == HD_COMMON_MEM_VB_INVALID_BLK) return HD_ERR_NG;
    UINTPTR pa = hd_common_mem_blk2pa(ctx->vqa_blk);
    ctx->vqa_buf_va = (UINTPTR)hd_common_mem_mmap(HD_COMMON_MEM_MEM_TYPE_CACHE, pa, NVT_VQA_Calc_buf_size(VQA_WIDTH, VQA_HEIGHT));
    
    ctx->blur_blk = hd_common_mem_get_block(HD_COMMON_MEM_COMMON_POOL, VQA_BUF_SIZE, DDR_ID0);
    if (ctx->blur_blk == HD_COMMON_MEM_VB_INVALID_BLK) return HD_ERR_NG;
    pa = hd_common_mem_blk2pa(ctx->blur_blk);
    ctx->blur_buf_va = (UINTPTR)hd_common_mem_mmap(HD_COMMON_MEM_MEM_TYPE_CACHE, pa, VQA_BUF_SIZE);
    
    NVT_VQA_Init(0, ctx->vqa_buf_va);
    
    vqa_param_t param = {0};
    param.enable_param.en_too_light = 1;
    param.enable_param.en_too_dark = 1;
    param.enable_param.en_blur = 1;
    param.enable_param.en_blur_block_info = 1;
    param.global_param.width = VQA_WIDTH;
    param.global_param.height = VQA_HEIGHT;
    param.global_param.g_alarm_frame_num = 20;
    param.light_param.too_light_strength_th = 215;
    param.light_param.too_light_cover_th = 15;
    param.light_param.too_light_alarm_times = 5;
    param.light_param.too_dark_strength_th = 50;
    param.light_param.too_dark_cover_th = 75;
    param.light_param.too_dark_alarm_times = 5;
    param.contrast_param.blur_strength = 8;
    param.contrast_param.blur_cover_th = 60;
    param.contrast_param.blur_alarm_times = 1;
    param.contrast_param.blur_w_num = 160;
    param.contrast_param.blur_h_num = 94;
    NVT_VQA_Set_param(0, &param);
    
    printf("[VQA] Initialized with blur/exposure detection enabled\n");
    return HD_OK;
}

static void* vqa_thread(void* arg) {
    DEMO_CTX* ctx = (DEMO_CTX*)arg;
    int frame_cnt = 0;
    while (g_running) {
        HD_VIDEO_FRAME frame = {0};
        if (hd_videoproc_pull_out_buf(ctx->proc_path_vqa, &frame, 100) != HD_OK) continue;
        
        UINT8* y_buf = (UINT8*)hd_common_mem_mmap(HD_COMMON_MEM_MEM_TYPE_CACHE, frame.phy_addr[0], VQA_BUF_SIZE);
        vqa_res_t result = {0};
        result.res_blur_block = (UINT8*)ctx->blur_buf_va;
        
        if (NVT_VQA_Run(0, y_buf, &result) == HD_OK) {
            if (result.res_too_light || result.res_too_dark || result.res_blur) {
                g_issues++;
                if (frame_cnt % 30 == 0) {
                    printf("[VQA] Frame %d: Light=%u Dark=%u Blur=%u (cover=%u%%)\n",
                           frame_cnt, result.res_too_light, result.res_too_dark, 
                           result.res_blur, result.res_blur_cover);
                }
            }
        }
        hd_common_mem_munmap(y_buf, VQA_BUF_SIZE);
        hd_videoproc_release_out_buf(ctx->proc_path_vqa, &frame);
        frame_cnt++;
    }
    return NULL;
}

int main(int argc, char* argv[]) {
    DEMO_CTX ctx = {0};
    pthread_t tid;
    ctx.duration = argc > 1 ? atoi(argv[1]) : 30;
    
    printf("\n=== NT98538 Video Quality Analysis Demo ===\n");
    printf("Features: Blur Detection, Over/Under Exposure Detection\n\n");
    signal(SIGINT, signal_handler);
    
    hd_common_init(0);
    init_memory();
    setup_vqa(&ctx);
    
    HD_VIDEOCAP_DRV_CONFIG cap_cfg = {0};
    snprintf(cap_cfg.sen_cfg.sen_dev.driver_name, HD_VIDEOCAP_SEN_NAME_LEN-1, SENSOR_NAME);
    cap_cfg.sen_cfg.sen_dev.if_type = HD_COMMON_VIDEO_IN_MIPI_CSI;
    cap_cfg.sen_cfg.sen_dev.pin_cfg.clk_lane_sel = HD_VIDEOCAP_SEN_CLANE_CSI(0, 0);
    cap_cfg.sen_cfg.sen_dev.pin_cfg.sen_2_serial_pin_map[0] = 0;
    cap_cfg.sen_cfg.sen_dev.pin_cfg.sen_2_serial_pin_map[1] = 1;
    for (int i = 2; i < 8; i++) cap_cfg.sen_cfg.sen_dev.pin_cfg.sen_2_serial_pin_map[i] = HD_VIDEOCAP_SEN_IGNORE;
    
    hd_videocap_open(0, HD_VIDEOCAP_CTRL(VCAP_ID), &ctx.cap_ctrl);
    hd_videocap_set(ctx.cap_ctrl, HD_VIDEOCAP_PARAM_DRV_CONFIG, &cap_cfg);
    HD_VIDEOCAP_CTRL cc = {.func = HD_VIDEOCAP_FUNC_AE | HD_VIDEOCAP_FUNC_AWB};
    hd_videocap_set(ctx.cap_ctrl, HD_VIDEOCAP_PARAM_CTRL, &cc);
    hd_videocap_open(HD_VIDEOCAP_IN(VCAP_ID, 0), HD_VIDEOCAP_OUT(VCAP_ID, 0), &ctx.cap_path);
    HD_VIDEOCAP_IN ci = {.sen_mode = HD_VIDEOCAP_SEN_MODE_AUTO, .frc = HD_VIDEO_FRC_RATIO(MAIN_FPS, 1), .dim = {CAP_WIDTH, CAP_HEIGHT}, .pxlfmt = HD_VIDEO_PXLFMT_RAW10};
    hd_videocap_set(ctx.cap_path, HD_VIDEOCAP_PARAM_IN, &ci);
    HD_VIDEOCAP_OUT co = {.pxlfmt = HD_VIDEO_PXLFMT_RAW10};
    hd_videocap_set(ctx.cap_path, HD_VIDEOCAP_PARAM_OUT, &co);
    
    hd_videoproc_open(0, HD_VIDEOPROC_0_CTRL, &ctx.proc_ctrl);
    HD_VIDEOPROC_DEV_CONFIG pc = {.pipe = HD_VIDEOPROC_PIPE_RAWALL, .isp_id = VCAP_ID, .in_max = {.dim = {CAP_WIDTH, CAP_HEIGHT}, .pxlfmt = HD_VIDEO_PXLFMT_RAW10}};
    hd_videoproc_set(ctx.proc_ctrl, HD_VIDEOPROC_PARAM_DEV_CONFIG, &pc);
    hd_videoproc_open(HD_VIDEOPROC_0_IN_0, HD_VIDEOPROC_0_OUT_0, &ctx.proc_path_main);
    HD_VIDEOPROC_OUT po = {.dim = {MAIN_WIDTH, MAIN_HEIGHT}, .pxlfmt = HD_VIDEO_PXLFMT_YUV420};
    hd_videoproc_set(ctx.proc_path_main, HD_VIDEOPROC_PARAM_OUT, &po);
    hd_videoproc_open(HD_VIDEOPROC_0_IN_0, HD_VIDEOPROC_0_OUT_1, &ctx.proc_path_vqa);
    po.dim.w = VQA_WIDTH; po.dim.h = VQA_HEIGHT; po.depth = 1;
    hd_videoproc_set(ctx.proc_path_vqa, HD_VIDEOPROC_PARAM_OUT, &po);
    
    hd_videocap_bind(HD_VIDEOCAP_OUT(VCAP_ID, 0), HD_VIDEOPROC_0_IN_0);
    hd_videocap_start(ctx.cap_path);
    hd_videoproc_start(ctx.proc_path_main);
    hd_videoproc_start(ctx.proc_path_vqa);
    
    pthread_create(&tid, NULL, vqa_thread, &ctx);
    printf("[MAIN] Running %ds...\n", ctx.duration);
    for (int i = 0; i < ctx.duration && g_running; i++) sleep(1);
    g_running = 0;
    pthread_join(tid, NULL);
    
    hd_videoproc_stop(ctx.proc_path_vqa);
    hd_videoproc_stop(ctx.proc_path_main);
    hd_videocap_stop(ctx.cap_path);
    hd_videocap_unbind(HD_VIDEOCAP_OUT(VCAP_ID, 0));
    hd_videoproc_close(ctx.proc_path_vqa);
    hd_videoproc_close(ctx.proc_path_main);
    hd_videoproc_close(ctx.proc_ctrl);
    hd_videocap_close(ctx.cap_path);
    hd_videocap_close(ctx.cap_ctrl);
    
    if (ctx.vqa_buf_va) hd_common_mem_munmap((void*)ctx.vqa_buf_va, NVT_VQA_Calc_buf_size(VQA_WIDTH, VQA_HEIGHT));
    if (ctx.blur_buf_va) hd_common_mem_munmap((void*)ctx.blur_buf_va, VQA_BUF_SIZE);
    if (ctx.vqa_blk != HD_COMMON_MEM_VB_INVALID_BLK) hd_common_mem_release_block(ctx.vqa_blk);
    if (ctx.blur_blk != HD_COMMON_MEM_VB_INVALID_BLK) hd_common_mem_release_block(ctx.blur_blk);
    hd_common_mem_uninit();
    hd_common_uninit();
    
    printf("\n[DONE] Quality issues detected: %d\n", g_issues);
    return 0;
}
