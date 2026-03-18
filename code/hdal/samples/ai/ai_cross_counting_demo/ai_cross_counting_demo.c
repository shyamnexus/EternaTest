/**
 * @file ai_cross_counting_demo.c
 * @brief Cross Counting Demo - Counts objects crossing multiple virtual lines
 * Usage: ./ai_cross_counting_demo [duration]
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
#define NUM_LINES       2

#define DBGINFO_BUFSIZE()   (0x200)
#define VDO_RAW_BUFSIZE(w, h, pxlfmt)   (ALIGN_CEIL_4((w) * HD_VIDEO_PXLFMT_BPP(pxlfmt) / 8) * (h))
#define VDO_YUV_BUFSIZE(w, h, pxlfmt)   (ALIGN_CEIL_4((w) * HD_VIDEO_PXLFMT_BPP(pxlfmt) / 8) * (h))

static volatile int g_running = 1;
static volatile int g_line_counts[NUM_LINES][2] = {{0}}; // [line][in/out]

typedef struct { UINTPTR pa, va; UINT32 size; HD_COMMON_MEM_VB_BLK blk; } MEM_BLOCK;

typedef struct {
    UINT32 x1, y1, x2, y2;  // Line endpoints
    int in_count, out_count;
} CROSS_LINE;

typedef struct {
    HD_PATH_ID cap_ctrl, cap_path, proc_ctrl, proc_path_main, proc_path_md;
    MEM_BLOCK md_temp, md_dst;
    MD_MDBC_CTRL_S mdbc_ctrl;
    MD_OBJ_CTRL_S objdet_ctrl;
    MD_OBJ_INFO_S obj_info;
    MD_PT_INFO_S obj_pt_stack[MD_WIDTH * MD_HEIGHT];
    MD_CROSS_CTRL_S cross_ctrl[NUM_LINES];
    CROSS_LINE lines[NUM_LINES];
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

static HD_RESULT setup_cross_counting(DEMO_CTX* ctx) {
    if (vendor_md_init() != HD_OK) return HD_ERR_NG;
    if (alloc_mem(&ctx->md_temp, MD_BUF_SIZE * 48) != HD_OK) return HD_ERR_NG;
    if (alloc_mem(&ctx->md_dst, MD_BUF_SIZE) != HD_OK) return HD_ERR_NG;
    
    memset(&ctx->mdbc_ctrl, 0, sizeof(ctx->mdbc_ctrl));
    ctx->mdbc_ctrl.enSensiLevel = LIB_MD_BC_HIGH_SENSI;
    ctx->mdbc_ctrl.enConvgSpd = LIB_MD_BC_MED_SPD;
    ctx->mdbc_ctrl.enInFmt = LIB_MD_IMG_YUV420SP;
    ctx->mdbc_ctrl.enBgNum = LIB_MD_BG_NUM_8;
    ctx->mdbc_ctrl.stMorph[0].enMorph = LIB_MD_MORPH_ENABLE;
    ctx->mdbc_ctrl.stTempMem.u64Pa = ctx->md_temp.pa;
    ctx->mdbc_ctrl.stTempMem.u64Va = ctx->md_temp.va;
    ctx->mdbc_ctrl.stTempMem.u32Size = ctx->md_temp.size;
    
    ctx->objdet_ctrl.u32ObjThres = 40;
    ctx->objdet_ctrl.enInRange = LIB_MD_IN_RANGE_0_1;
    ctx->objdet_ctrl.pstPtStk = ctx->obj_pt_stack;
    
    // Define two counting lines: horizontal at 1/3 and 2/3 of frame
    ctx->lines[0].x1 = 0; ctx->lines[0].y1 = MD_HEIGHT / 3;
    ctx->lines[0].x2 = MD_WIDTH - 1; ctx->lines[0].y2 = MD_HEIGHT / 3;
    
    ctx->lines[1].x1 = 0; ctx->lines[1].y1 = MD_HEIGHT * 2 / 3;
    ctx->lines[1].x2 = MD_WIDTH - 1; ctx->lines[1].y2 = MD_HEIGHT * 2 / 3;
    
    for (int i = 0; i < NUM_LINES; i++) {
        memset(&ctx->cross_ctrl[i], 0, sizeof(MD_CROSS_CTRL_S));
        ctx->cross_ctrl[i].enCrossMode = LIB_MD_CROSS_LINE;
        ctx->cross_ctrl[i].u32OverlapTh = 50;
        ctx->cross_ctrl[i].u32X1 = ctx->lines[i].x1;
        ctx->cross_ctrl[i].u32Y1 = ctx->lines[i].y1;
        ctx->cross_ctrl[i].u32X2 = ctx->lines[i].x2;
        ctx->cross_ctrl[i].u32Y2 = ctx->lines[i].y2;
        printf("[CROSS] Line %d: (%u,%u) -> (%u,%u)\n", i, 
               ctx->lines[i].x1, ctx->lines[i].y1, ctx->lines[i].x2, ctx->lines[i].y2);
    }
    
    return HD_OK;
}

static void* counting_thread(void* arg) {
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
            memset(&ctx->obj_info, 0, sizeof(ctx->obj_info));
            NVT_MD_ObjDet(&dst, &ctx->obj_info, &ctx->objdet_ctrl);
            
            if (ctx->obj_info.u32ObjNum > 0) {
                for (int line = 0; line < NUM_LINES; line++) {
                    MD_CROSS_OBJ_INFO_S obj_list[LIB_MD_MAX_OBJ_NUM] = {0};
                    for (UINT32 i = 0; i < ctx->obj_info.u32ObjNum && i < LIB_MD_MAX_OBJ_NUM; i++) {
                        obj_list[i].u32XStart = ctx->obj_info.stObjRst[i].u32XStart;
                        obj_list[i].u32YStart = ctx->obj_info.stObjRst[i].u32YStart;
                        obj_list[i].u32XEnd = ctx->obj_info.stObjRst[i].u32XEnd;
                        obj_list[i].u32YEnd = ctx->obj_info.stObjRst[i].u32YEnd;
                        obj_list[i].u8Valid = 1;
                    }
                    ctx->cross_ctrl[line].u8ObjNum = ctx->obj_info.u32ObjNum;
                    
                    LIB_MD_CROSS_RST result = LIB_MD_CROSS_NONE;
                    NVT_MD_CrossLine(obj_list, &result, &ctx->cross_ctrl[line]);
                    
                    if (result == LIB_MD_CROSS_IN_TO_OUT) {
                        ctx->lines[line].out_count++;
                        g_line_counts[line][1]++;
                        printf("[CROSS] Line %d: OUT (total: in=%d, out=%d)\n", line, ctx->lines[line].in_count, ctx->lines[line].out_count);
                    } else if (result == LIB_MD_CROSS_OUT_TO_IN) {
                        ctx->lines[line].in_count++;
                        g_line_counts[line][0]++;
                        printf("[CROSS] Line %d: IN (total: in=%d, out=%d)\n", line, ctx->lines[line].in_count, ctx->lines[line].out_count);
                    }
                    
                    memcpy(ctx->cross_ctrl[line].stPreObj, obj_list, sizeof(obj_list));
                    ctx->cross_ctrl[line].u8PreObjNum = ctx->obj_info.u32ObjNum;
                }
            }
        }
        
        if (src.u64Va) hd_common_mem_munmap((void*)src.u64Va, MD_BUF_SIZE*2);
        hd_videoproc_release_out_buf(ctx->proc_path_md, &frame);
        frame_cnt++;
        
        if (frame_cnt % 300 == 0) {
            printf("[STATUS] Frame %d: ", frame_cnt);
            for (int i = 0; i < NUM_LINES; i++) {
                printf("Line%d(in=%d,out=%d) ", i, ctx->lines[i].in_count, ctx->lines[i].out_count);
            }
            printf("\n");
        }
    }
    return NULL;
}

int main(int argc, char* argv[]) {
    DEMO_CTX ctx = {0};
    pthread_t tid;
    ctx.duration = argc > 1 ? atoi(argv[1]) : 30;
    
    printf("\n=== NT98538 Cross Counting Demo ===\n");
    printf("Features: Multi-line cross counting with in/out statistics\n\n");
    signal(SIGINT, signal_handler);
    
    hd_common_init(0);
    init_memory();
    setup_cross_counting(&ctx);
    
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
    
    pthread_create(&tid, NULL, counting_thread, &ctx);
    printf("[MAIN] Running %ds...\n", ctx.duration);
    for (int i = 0; i < ctx.duration && g_running; i++) sleep(1);
    g_running = 0;
    pthread_join(tid, NULL);
    
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
    
    printf("\n[DONE] Cross Counting Summary:\n");
    for (int i = 0; i < NUM_LINES; i++) {
        printf("  Line %d: IN=%d, OUT=%d, Net=%d\n", i, 
               ctx.lines[i].in_count, ctx.lines[i].out_count,
               ctx.lines[i].in_count - ctx.lines[i].out_count);
    }
    return 0;
}
