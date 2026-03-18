/**
 * @file ai_heat_map_demo.c
 * @brief Heat Map Demo - Generates motion activity heat map over time
 * Usage: ./ai_heat_map_demo [duration]
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
#include "vendor_md.h"
#include "libmd/libmd.h"

#define SENSOR_NAME     "nvt_sen_gc5603"
#define VCAP_ID         0
#define CAP_WIDTH       2960
#define CAP_HEIGHT      1664
#define MAIN_WIDTH      1920
#define MAIN_HEIGHT     1080
#define MAIN_FPS        30
#define MD_WIDTH        160
#define MD_HEIGHT       120
#define MD_BUF_SIZE     (MD_WIDTH * MD_HEIGHT)

#define HEAT_GRID_W     16
#define HEAT_GRID_H     12
#define HEAT_DECAY      0.99f

#define DBGINFO_BUFSIZE()   (0x200)
#define VDO_RAW_BUFSIZE(w, h, pxlfmt)   (ALIGN_CEIL_4((w) * HD_VIDEO_PXLFMT_BPP(pxlfmt) / 8) * (h))
#define VDO_YUV_BUFSIZE(w, h, pxlfmt)   (ALIGN_CEIL_4((w) * HD_VIDEO_PXLFMT_BPP(pxlfmt) / 8) * (h))

static volatile int g_running = 1;
static float g_heat_map[HEAT_GRID_H][HEAT_GRID_W] = {{0}};

typedef struct { UINTPTR pa, va; UINT32 size; HD_COMMON_MEM_VB_BLK blk; } MEM_BLOCK;

typedef struct {
    HD_PATH_ID cap_ctrl, cap_path, proc_ctrl, proc_path_main, proc_path_md;
    MEM_BLOCK md_temp, md_dst;
    MD_MDBC_CTRL_S mdbc_ctrl;
    int mdbc_initialized, duration;
} DEMO_CTX;

static void signal_handler(int sig) { g_running = 0; }

static HD_RESULT alloc_mem(MEM_BLOCK* b, UINT32 sz) {
    b->size = sz;
    b->blk = hd_common_mem_get_block(HD_COMMON_MEM_USER_BLK, sz, DDR_ID0);
    if (b->blk == HD_COMMON_MEM_VB_INVALID_BLK) return HD_ERR_NG;
    b->pa = hd_common_mem_blk2pa(b->blk);
    b->va = (UINTPTR)hd_common_mem_mmap(HD_COMMON_MEM_MEM_TYPE_CACHE, b->pa, sz);
    return HD_OK;
}

static void free_mem(MEM_BLOCK* b) {
    if (b->va) hd_common_mem_munmap((void*)b->va, b->size);
    if (b->blk != HD_COMMON_MEM_VB_INVALID_BLK) hd_common_mem_release_block(b->blk);
}

static HD_RESULT init_memory(void) {
    HD_COMMON_MEM_INIT_CONFIG m = {0};
    m.pool_info[0].type = HD_COMMON_MEM_COMMON_POOL;
    m.pool_info[0].blk_size = DBGINFO_BUFSIZE() + VDO_RAW_BUFSIZE(CAP_WIDTH, CAP_HEIGHT, HD_VIDEO_PXLFMT_RAW10);
    m.pool_info[0].blk_cnt = 3; m.pool_info[0].ddr_id = DDR_ID0;
    m.pool_info[1].type = HD_COMMON_MEM_COMMON_POOL;
    m.pool_info[1].blk_size = DBGINFO_BUFSIZE() + VDO_YUV_BUFSIZE(MAIN_WIDTH, MAIN_HEIGHT, HD_VIDEO_PXLFMT_YUV420);
    m.pool_info[1].blk_cnt = 3; m.pool_info[1].ddr_id = DDR_ID0;
    m.pool_info[2].type = HD_COMMON_MEM_COMMON_POOL;
    m.pool_info[2].blk_size = DBGINFO_BUFSIZE() + VDO_YUV_BUFSIZE(MD_WIDTH, MD_HEIGHT, HD_VIDEO_PXLFMT_YUV420);
    m.pool_info[2].blk_cnt = 3; m.pool_info[2].ddr_id = DDR_ID0;
    m.pool_info[3].type = HD_COMMON_MEM_USER_BLK;
    m.pool_info[3].blk_size = MD_BUF_SIZE * 48; m.pool_info[3].blk_cnt = 1; m.pool_info[3].ddr_id = DDR_ID0;
    m.pool_info[4].type = HD_COMMON_MEM_USER_BLK;
    m.pool_info[4].blk_size = MD_BUF_SIZE; m.pool_info[4].blk_cnt = 1; m.pool_info[4].ddr_id = DDR_ID0;
    return hd_common_mem_init(&m);
}

static void update_heat_map(UINT8* fg_mask) {
    int cell_w = MD_WIDTH / HEAT_GRID_W;
    int cell_h = MD_HEIGHT / HEAT_GRID_H;
    
    // Decay existing heat
    for (int gy = 0; gy < HEAT_GRID_H; gy++) {
        for (int gx = 0; gx < HEAT_GRID_W; gx++) {
            g_heat_map[gy][gx] *= HEAT_DECAY;
        }
    }
    
    // Add new motion heat
    for (int gy = 0; gy < HEAT_GRID_H; gy++) {
        for (int gx = 0; gx < HEAT_GRID_W; gx++) {
            int motion_count = 0;
            int total_pixels = 0;
            
            for (int y = gy * cell_h; y < (gy + 1) * cell_h && y < MD_HEIGHT; y++) {
                for (int x = gx * cell_w; x < (gx + 1) * cell_w && x < MD_WIDTH; x++) {
                    if (fg_mask[y * MD_WIDTH + x] > 0) motion_count++;
                    total_pixels++;
                }
            }
            
            float motion_ratio = total_pixels > 0 ? (float)motion_count / total_pixels : 0;
            g_heat_map[gy][gx] += motion_ratio * 0.1f;
            if (g_heat_map[gy][gx] > 1.0f) g_heat_map[gy][gx] = 1.0f;
        }
    }
}

static void print_heat_map(void) {
    const char* heat_chars = " .:-=+*#%@";
    
    printf("\n╔");
    for (int x = 0; x < HEAT_GRID_W; x++) printf("═");
    printf("╗\n");
    
    for (int y = 0; y < HEAT_GRID_H; y++) {
        printf("║");
        for (int x = 0; x < HEAT_GRID_W; x++) {
            int idx = (int)(g_heat_map[y][x] * 9);
            if (idx > 9) idx = 9;
            printf("%c", heat_chars[idx]);
        }
        printf("║\n");
    }
    
    printf("╚");
    for (int x = 0; x < HEAT_GRID_W; x++) printf("═");
    printf("╝\n");
}

static void save_heat_map(const char* filename) {
    FILE* f = fopen(filename, "w");
    if (!f) return;
    
    fprintf(f, "Heat Map Data (%dx%d grid)\n", HEAT_GRID_W, HEAT_GRID_H);
    fprintf(f, "Values range from 0.0 (no activity) to 1.0 (high activity)\n\n");
    
    for (int y = 0; y < HEAT_GRID_H; y++) {
        for (int x = 0; x < HEAT_GRID_W; x++) {
            fprintf(f, "%.3f ", g_heat_map[y][x]);
        }
        fprintf(f, "\n");
    }
    fclose(f);
    printf("[HEAT] Saved heat map to %s\n", filename);
}

static void* heat_thread(void* arg) {
    DEMO_CTX* ctx = (DEMO_CTX*)arg;
    int frame_cnt = 0;
    
    while (g_running) {
        HD_VIDEO_FRAME frame = {0};
        if (hd_videoproc_pull_out_buf(ctx->proc_path_md, &frame, 100) != HD_OK) continue;
        
        MD_SRC_IMAGE_S src = {.u64Pa = frame.phy_addr[0], .u32Width = MD_WIDTH, .u32Height = MD_HEIGHT, .u32Stride = MD_WIDTH};
        src.u64Va = (uintptr_t)hd_common_mem_mmap(HD_COMMON_MEM_MEM_TYPE_CACHE, frame.phy_addr[0], MD_BUF_SIZE*2);
        
        MD_DST_IMAGE_S dst = {.u64Pa = ctx->md_dst.pa, .u64Va = ctx->md_dst.va, .u32Width = MD_WIDTH, .u32Height = MD_HEIGHT, .u32Stride = MD_WIDTH};
        
        ctx->mdbc_ctrl.enIsInit = ctx->mdbc_initialized ? LIB_MD_BC_NORM_MODE : LIB_MD_BC_INIT_MODE;
        ctx->mdbc_initialized = 1;
        
        if (NVT_MD_MDBC(&src, &dst, &ctx->mdbc_ctrl) == LIB_MD_OK) {
            update_heat_map((UINT8*)dst.u64Va);
            
            if (frame_cnt % 90 == 0) { // Every 3 seconds at 30fps
                printf("\033[2J\033[H"); // Clear screen
                printf("=== NT98538 Heat Map Demo (Frame %d) ===\n", frame_cnt);
                print_heat_map();
            }
        }
        
        if (src.u64Va) hd_common_mem_munmap((void*)src.u64Va, MD_BUF_SIZE*2);
        hd_videoproc_release_out_buf(ctx->proc_path_md, &frame);
        frame_cnt++;
    }
    return NULL;
}

int main(int argc, char* argv[]) {
    DEMO_CTX ctx = {0};
    pthread_t tid;
    ctx.duration = argc > 1 ? atoi(argv[1]) : 30;
    
    printf("\n=== NT98538 Heat Map Demo ===\n");
    printf("Features: Real-time motion activity heat map visualization\n\n");
    signal(SIGINT, signal_handler);
    
    hd_common_init(0);
    init_memory();
    
    if (vendor_md_init() != HD_OK) return -1;
    if (alloc_mem(&ctx.md_temp, MD_BUF_SIZE * 48) != HD_OK) return -1;
    if (alloc_mem(&ctx.md_dst, MD_BUF_SIZE) != HD_OK) return -1;
    
    memset(&ctx.mdbc_ctrl, 0, sizeof(ctx.mdbc_ctrl));
    ctx.mdbc_ctrl.enSensiLevel = LIB_MD_BC_HIGH_SENSI;
    ctx.mdbc_ctrl.enConvgSpd = LIB_MD_BC_MED_SPD;
    ctx.mdbc_ctrl.enInFmt = LIB_MD_IMG_YUV420SP;
    ctx.mdbc_ctrl.enBgNum = LIB_MD_BG_NUM_8;
    ctx.mdbc_ctrl.stMorph[0].enMorph = LIB_MD_MORPH_ENABLE;
    ctx.mdbc_ctrl.stTempMem.u64Pa = ctx.md_temp.pa;
    ctx.mdbc_ctrl.stTempMem.u64Va = ctx.md_temp.va;
    ctx.mdbc_ctrl.stTempMem.u32Size = ctx.md_temp.size;
    
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
    
    hd_videoproc_open(0, HD_VIDEOPROC_0_CTRL, &ctx.proc_ctrl);
    HD_VIDEOPROC_DEV_CONFIG pc = {.pipe = HD_VIDEOPROC_PIPE_RAWALL, .isp_id = VCAP_ID, .in_max = {.dim = {CAP_WIDTH, CAP_HEIGHT}, .pxlfmt = HD_VIDEO_PXLFMT_RAW10}};
    hd_videoproc_set(ctx.proc_ctrl, HD_VIDEOPROC_PARAM_DEV_CONFIG, &pc);
    hd_videoproc_open(HD_VIDEOPROC_0_IN_0, HD_VIDEOPROC_0_OUT_0, &ctx.proc_path_main);
    HD_VIDEOPROC_OUT po = {.dim = {MAIN_WIDTH, MAIN_HEIGHT}, .pxlfmt = HD_VIDEO_PXLFMT_YUV420};
    hd_videoproc_set(ctx.proc_path_main, HD_VIDEOPROC_PARAM_OUT, &po);
    hd_videoproc_open(HD_VIDEOPROC_0_IN_0, HD_VIDEOPROC_0_OUT_1, &ctx.proc_path_md);
    po.dim.w = MD_WIDTH; po.dim.h = MD_HEIGHT; po.depth = 1;
    hd_videoproc_set(ctx.proc_path_md, HD_VIDEOPROC_PARAM_OUT, &po);
    
    hd_videocap_bind(HD_VIDEOCAP_OUT(VCAP_ID, 0), HD_VIDEOPROC_0_IN_0);
    hd_videocap_start(ctx.cap_path);
    hd_videoproc_start(ctx.proc_path_main);
    hd_videoproc_start(ctx.proc_path_md);
    
    pthread_create(&tid, NULL, heat_thread, &ctx);
    for (int i = 0; i < ctx.duration && g_running; i++) sleep(1);
    g_running = 0;
    pthread_join(tid, NULL);
    
    // Save final heat map
    save_heat_map("/mnt/sd/heat_map.txt");
    
    hd_videoproc_stop(ctx.proc_path_md);
    hd_videoproc_stop(ctx.proc_path_main);
    hd_videocap_stop(ctx.cap_path);
    hd_videocap_unbind(HD_VIDEOCAP_OUT(VCAP_ID, 0));
    hd_videoproc_close(ctx.proc_path_md);
    hd_videoproc_close(ctx.proc_path_main);
    hd_videoproc_close(ctx.proc_ctrl);
    hd_videocap_close(ctx.cap_path);
    hd_videocap_close(ctx.cap_ctrl);
    free_mem(&ctx.md_temp);
    free_mem(&ctx.md_dst);
    vendor_md_uninit();
    hd_common_mem_uninit();
    hd_common_uninit();
    
    printf("\n[DONE] Heat map generation completed\n");
    print_heat_map();
    return 0;
}
