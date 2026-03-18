/**
 * @file ai_privacy_mosaic_demo.c
 * @brief Privacy Mosaic Demo - Applies mosaic blur to detected regions
 * Usage: ./ai_privacy_mosaic_demo [duration]
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
#include "hd_gfx.h"

#define SENSOR_NAME     "nvt_sen_gc5603"
#define VCAP_ID         0
#define CAP_WIDTH       2960
#define CAP_HEIGHT      1664
#define MAIN_WIDTH      1920
#define MAIN_HEIGHT     1080
#define MAIN_FPS        30

#define MOSAIC_BLOCK_SIZE   16  // Mosaic block size in pixels
#define MAX_MOSAIC_REGIONS  4

#define DBGINFO_BUFSIZE()   (0x200)
#define VDO_RAW_BUFSIZE(w, h, pxlfmt)   (ALIGN_CEIL_4((w) * HD_VIDEO_PXLFMT_BPP(pxlfmt) / 8) * (h))
#define VDO_YUV_BUFSIZE(w, h, pxlfmt)   (ALIGN_CEIL_4((w) * HD_VIDEO_PXLFMT_BPP(pxlfmt) / 8) * (h))

static volatile int g_running = 1;
static volatile int g_frames_processed = 0;

typedef struct {
    UINT32 x, y, w, h;
    BOOL enabled;
} MOSAIC_REGION;

typedef struct {
    HD_PATH_ID cap_ctrl, cap_path, proc_ctrl, proc_path_main;
    HD_PATH_ID enc_path;
    MOSAIC_REGION regions[MAX_MOSAIC_REGIONS];
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
    m.pool_info[1].blk_cnt = 4; m.pool_info[1].ddr_id = DDR_ID0;
    return hd_common_mem_init(&m);
}

static void setup_mosaic_regions(DEMO_CTX* ctx) {
    // Define privacy mask regions (example: 4 corners)
    // Region 0: Top-left corner
    ctx->regions[0].x = 0; ctx->regions[0].y = 0;
    ctx->regions[0].w = MAIN_WIDTH / 4; ctx->regions[0].h = MAIN_HEIGHT / 4;
    ctx->regions[0].enabled = TRUE;
    
    // Region 1: Top-right corner
    ctx->regions[1].x = MAIN_WIDTH * 3 / 4; ctx->regions[1].y = 0;
    ctx->regions[1].w = MAIN_WIDTH / 4; ctx->regions[1].h = MAIN_HEIGHT / 4;
    ctx->regions[1].enabled = TRUE;
    
    // Region 2: Center (disabled by default)
    ctx->regions[2].x = MAIN_WIDTH / 3; ctx->regions[2].y = MAIN_HEIGHT / 3;
    ctx->regions[2].w = MAIN_WIDTH / 3; ctx->regions[2].h = MAIN_HEIGHT / 3;
    ctx->regions[2].enabled = FALSE;
    
    // Region 3: Bottom-left (disabled by default)
    ctx->regions[3].x = 0; ctx->regions[3].y = MAIN_HEIGHT * 3 / 4;
    ctx->regions[3].w = MAIN_WIDTH / 4; ctx->regions[3].h = MAIN_HEIGHT / 4;
    ctx->regions[3].enabled = FALSE;
    
    printf("[MOSAIC] Privacy regions configured:\n");
    for (int i = 0; i < MAX_MOSAIC_REGIONS; i++) {
        if (ctx->regions[i].enabled) {
            printf("  Region %d: (%u,%u) %ux%u - ENABLED\n", i,
                   ctx->regions[i].x, ctx->regions[i].y, ctx->regions[i].w, ctx->regions[i].h);
        }
    }
}

static void apply_mosaic(UINT8* yuv_buf, UINT32 width, UINT32 height, MOSAIC_REGION* region) {
    if (!region->enabled) return;
    
    UINT32 block_size = MOSAIC_BLOCK_SIZE;
    UINT32 x_start = region->x / block_size * block_size;
    UINT32 y_start = region->y / block_size * block_size;
    UINT32 x_end = (region->x + region->w + block_size - 1) / block_size * block_size;
    UINT32 y_end = (region->y + region->h + block_size - 1) / block_size * block_size;
    
    if (x_end > width) x_end = width;
    if (y_end > height) y_end = height;
    
    // Apply mosaic by averaging blocks (Y plane only for simplicity)
    for (UINT32 by = y_start; by < y_end; by += block_size) {
        for (UINT32 bx = x_start; bx < x_end; bx += block_size) {
            // Calculate average Y value for block
            UINT32 sum = 0, count = 0;
            for (UINT32 y = by; y < by + block_size && y < height; y++) {
                for (UINT32 x = bx; x < bx + block_size && x < width; x++) {
                    sum += yuv_buf[y * width + x];
                    count++;
                }
            }
            UINT8 avg = count > 0 ? sum / count : 128;
            
            // Fill block with average value
            for (UINT32 y = by; y < by + block_size && y < height; y++) {
                for (UINT32 x = bx; x < bx + block_size && x < width; x++) {
                    yuv_buf[y * width + x] = avg;
                }
            }
        }
    }
}

static void* mosaic_thread(void* arg) {
    DEMO_CTX* ctx = (DEMO_CTX*)arg;
    
    while (g_running) {
        HD_VIDEO_FRAME frame = {0};
        if (hd_videoproc_pull_out_buf(ctx->proc_path_main, &frame, 100) != HD_OK) continue;
        
        // Map frame buffer
        UINT8* yuv_buf = (UINT8*)hd_common_mem_mmap(HD_COMMON_MEM_MEM_TYPE_CACHE, 
                                                     frame.phy_addr[0], MAIN_WIDTH * MAIN_HEIGHT);
        if (yuv_buf) {
            // Apply mosaic to each enabled region
            for (int i = 0; i < MAX_MOSAIC_REGIONS; i++) {
                apply_mosaic(yuv_buf, MAIN_WIDTH, MAIN_HEIGHT, &ctx->regions[i]);
            }
            
            hd_common_mem_flush_cache(yuv_buf, MAIN_WIDTH * MAIN_HEIGHT);
            hd_common_mem_munmap(yuv_buf, MAIN_WIDTH * MAIN_HEIGHT);
        }
        
        hd_videoproc_release_out_buf(ctx->proc_path_main, &frame);
        g_frames_processed++;
        
        if (g_frames_processed % 300 == 0) {
            printf("[MOSAIC] Processed %d frames\n", g_frames_processed);
        }
    }
    return NULL;
}

int main(int argc, char* argv[]) {
    DEMO_CTX ctx = {0};
    pthread_t tid;
    ctx.duration = argc > 1 ? atoi(argv[1]) : 30;
    
    printf("\n=== NT98538 Privacy Mosaic Demo ===\n");
    printf("Features: Real-time mosaic privacy masking using GFX\n\n");
    signal(SIGINT, signal_handler);
    
    hd_common_init(0);
    init_memory();
    hd_gfx_init();
    setup_mosaic_regions(&ctx);
    
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
    HD_VIDEOPROC_OUT po = {.dim = {MAIN_WIDTH, MAIN_HEIGHT}, .pxlfmt = HD_VIDEO_PXLFMT_YUV420, .depth = 1};
    hd_videoproc_set(ctx.proc_path_main, HD_VIDEOPROC_PARAM_OUT, &po);
    
    hd_videocap_bind(HD_VIDEOCAP_OUT(VCAP_ID, 0), HD_VIDEOPROC_0_IN_0);
    hd_videocap_start(ctx.cap_path);
    hd_videoproc_start(ctx.proc_path_main);
    
    pthread_create(&tid, NULL, mosaic_thread, &ctx);
    printf("[MAIN] Running %ds...\n", ctx.duration);
    for (int i = 0; i < ctx.duration && g_running; i++) sleep(1);
    g_running = 0;
    pthread_join(tid, NULL);
    
    hd_videoproc_stop(ctx.proc_path_main);
    hd_videocap_stop(ctx.cap_path);
    hd_videocap_unbind(HD_VIDEOCAP_OUT(VCAP_ID, 0));
    hd_videoproc_close(ctx.proc_path_main);
    hd_videoproc_close(ctx.proc_ctrl);
    hd_videocap_close(ctx.cap_path);
    hd_videocap_close(ctx.cap_ctrl);
    hd_gfx_uninit();
    hd_common_mem_uninit();
    hd_common_uninit();
    
    printf("\n[DONE] Processed %d frames with privacy mosaic\n", g_frames_processed);
    return 0;
}
