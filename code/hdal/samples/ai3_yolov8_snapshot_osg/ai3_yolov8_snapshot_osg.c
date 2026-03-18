/**
 * @file ai3_yolov8_snapshot_osg.c
 * @brief YOLOv8 AI3 inference with live sensor + OSD bbox + JPEG snapshot
 *
 * This sample combines:
 *   1. Live sensor capture pipeline  (videocap -> videoproc)
 *   2. AI3 NPU YOLOv8 / SCRFD inference on each frame
 *   3. Drawing bounding boxes on the YUV frame via hd_gfx_draw_rect (OSG)
 *   4. Encoding the annotated frame as JPEG snapshot (videoenc)
 *   5. Setting VENDOR_VIDEOENC_SMART_BBOX for VLC overlay (optional)
 *
 * Based on:  ai3_custom_yolov8, hd_video_snapshot, ai3_turnkey_sample_stream_ipc
 *
 * Usage:
 *   ./ai3_yolov8_snapshot_osg <model.bin> [labels.txt]
 *   ./ai3_yolov8_snapshot_osg --scrfd <model.bin>
 *
 * Keys:
 *   s  - take a JPEG snapshot with bounding boxes drawn on YUV
 *   r  - toggle continuous AI inference (default: ON)
 *   d  - enter HDAL debug menu
 *   q  - quit
 *
 * Copyright 2026.  All rights reserved.
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <pthread.h>

#include "hdal.h"
#include "hd_debug.h"
#include "hd_gfx.h"
#include "vendor_ai.h"
#include "vendor_ai_cpu/vendor_ai_cpu.h"
#include "vendor_videoenc.h"
#include "yolov8_postproc.h"
#include "scrfd_postproc.h"

/*==========================================================================*/
/*  Platform                                                                */
/*==========================================================================*/
#define MAIN(argc, argv) int main(int argc, char **argv)
#define GETCHAR()        getchar()

/*==========================================================================*/
/*  Configuration                                                           */
/*==========================================================================*/
#define SEN1_VCAP_ID       0
#define USE_DDR            DDR_ID0
#define DEBUG_MENU         1

/* ---------- Sensor / Video Dimensions  --------- */
/* GC5603 full-res capture (same as hd_video_snapshot) */
#define SEN_OUT_FMT        HD_VIDEO_PXLFMT_RAW8
#define CAP_OUT_FMT        HD_VIDEO_PXLFMT_RAW8
#define VDO_SIZE_W         2960
#define VDO_SIZE_H         1664

/* 3A window config */
#define CA_WIN_NUM_W       32
#define CA_WIN_NUM_H       32
#define LA_WIN_NUM_W       32
#define LA_WIN_NUM_H       32

/* ---------- AI input  ---------- */
#define AI_INPUT_W         640
#define AI_INPUT_H         640

/* ---------- Buffer size helpers (from hd_video_snapshot) ---------- */
#define DBGINFO_BUFSIZE()  (0x200)
#define VDO_RAW_BUFSIZE(w, h, pxlfmt) \
    (ALIGN_CEIL_4((w) * HD_VIDEO_PXLFMT_BPP(pxlfmt) / 8) * (h))
#define VDO_CA_BUF_SIZE(ww, wh)  ALIGN_CEIL_4(((ww) * (wh) << 3) << 1)
#define VDO_LA_BUF_SIZE(ww, wh)  ALIGN_CEIL_4(((ww) * (wh) << 1) << 1)
#define VDO_YUV_BUFSIZE(w, h, pxlfmt) \
    (ALIGN_CEIL_4((w) * HD_VIDEO_PXLFMT_BPP(pxlfmt) / 8) * (h))
#define AI_NV12_BUFSIZE(w, h)  (ALIGN_CEIL_4(w) * (h) * 3 / 2)

/* ---------- Bbox colours (YUV) ---------- */
/* Green  (person) */
#define BBOX_COLOR_Y_0   149
#define BBOX_COLOR_U_0    43
#define BBOX_COLOR_V_0    21
/* Red (vehicle / other) */
#define BBOX_COLOR_Y_1    76
#define BBOX_COLOR_U_1    84
#define BBOX_COLOR_V_1   255
/* Blue (face) */
#define BBOX_COLOR_Y_2    29
#define BBOX_COLOR_U_2   255
#define BBOX_COLOR_V_2   107

/* Bbox line thickness in pixels */
#define BBOX_THICKNESS     4
/* Max boxes drawn per snapshot */
#define MAX_DRAW_BBOX      32

/* AI engine sysconfig (from hd_int.h / sample_ipc_ext.h) */
#ifndef VENDOR_AI_CFG
#define VENDOR_AI_CFG              0x000f0000
#endif
#define VENDOR_AI_CFG_ENABLE_CNN   0x00010000

/*==========================================================================*/
/*  Debug helpers                                                           */
/*==========================================================================*/
#define CHKPNT  printf("\033[37mCHK: %s:%d\033[0m\r\n", __func__, __LINE__)
#define DBGD(x) printf("\033[0;35m%s=%d\033[0m\r\n", #x, x)

/*==========================================================================*/
/*  Types                                                                   */
/*==========================================================================*/
typedef enum {
    MODEL_TYPE_YOLOV8 = 0,
    MODEL_TYPE_SCRFD,
} MODEL_TYPE;

typedef struct {
    UINTPTR pa;
    UINTPTR va;
    UINT32  size;
    UINTPTR blk;
} MEM_PARM;

typedef struct {
    /* --- sensor / cap / proc / enc path IDs --- */
    HD_VIDEOCAP_SYSCAPS cap_syscaps;
    HD_PATH_ID  cap_ctrl;
    HD_PATH_ID  cap_path;
    HD_PATH_ID  proc_ctrl;
    HD_PATH_ID  proc_path;
    HD_PATH_ID  enc_path;
    HD_DIM      cap_dim;
    HD_DIM      proc_max_dim;
    HD_DIM      proc_dim;
    HD_DIM      enc_max_dim;
    HD_DIM      enc_dim;

    /* --- AI --- */
    char          model_path[256];
    char          labels_path[256];
    MODEL_TYPE    model_type;
    MEM_PARM      model_mem;
    MEM_PARM      work_mem;
    MEM_PARM      ronly_mem;
    MEM_PARM     *out_mem;
    UINT32        proc_id;          /* AI3 proc ID */
    VENDOR_AI3_NET_INFO net_info;
    char          labels[YOLOV8_NUM_CLASSES * YOLOV8_LABEL_LEN];
    int           labels_loaded;

    /* --- results --- */
    YOLOV8_RESULTS  yolo_results;
    SCRFD_RESULTS   scrfd_results;

    /* --- thread control --- */
    pthread_t   ai_thread_id;
    pthread_t   snap_thread_id;
    UINT32      flow_start;
    UINT32      ai_exit;
    UINT32      snap_exit;
    UINT32      do_snap;
    UINT32      shot_count;
    UINT32      ai_running;

    /* Shared: latest detection results (protected by mutex) */
    pthread_mutex_t det_mutex;
    YOLOV8_RESULTS  latest_yolo;
    SCRFD_RESULTS   latest_scrfd;
    int             latest_valid;
} APP_CONTEXT;

static APP_CONTEXT g_ctx = {0};
static volatile int g_running = 1;

/*==========================================================================*/
/*  Signal handler                                                          */
/*==========================================================================*/
static void signal_handler(int sig)
{
    (void)sig;
    printf("\nSignal %d received – exiting.\n", sig);
    g_running = 0;
}

/*==========================================================================*/
/*  Memory helpers                                                          */
/*==========================================================================*/
static HD_RESULT mem_alloc(MEM_PARM *m, char *name, UINT32 size)
{
    HD_RESULT ret;
    UINTPTR pa = 0;
    void *va = NULL;
    ret = hd_common_mem_alloc(name, &pa, &va, size, USE_DDR);
    if (ret != HD_OK) {
        printf("[MEM] alloc %s (%u) failed: %d\n", name, size, ret);
        return ret;
    }
    m->pa   = pa;
    m->va   = (UINTPTR)va;
    m->size = size;
    m->blk  = (UINTPTR)-1;
    return HD_OK;
}

static void mem_free(MEM_PARM *m)
{
    if (m->pa && m->va) {
        hd_common_mem_free(m->pa, (void *)m->va);
        m->pa = m->va = 0;
        m->size = 0;
    }
}

static INT32 load_file(const char *fn, UINTPTR va, UINT32 max)
{
    FILE *f = fopen(fn, "rb");
    if (!f) { printf("[FILE] open fail: %s\n", fn); return -1; }
    fseek(f, 0, SEEK_END);
    INT32 sz = ftell(f);
    fseek(f, 0, SEEK_SET);
    if (sz > (INT32)max) { fclose(f); return -1; }
    INT32 rd = fread((void *)va, 1, sz, f);
    fclose(f);
    hd_common_mem_flush_cache((void *)va, sz);
    return rd;
}

static INT32 get_file_size(const char *fn)
{
    FILE *f = fopen(fn, "rb");
    if (!f) return -1;
    fseek(f, 0, SEEK_END);
    INT32 sz = ftell(f);
    fclose(f);
    return sz;
}

/*==========================================================================*/
/*  HDAL memory pool init  (from hd_video_snapshot)                         */
/*==========================================================================*/
static HD_RESULT hdal_mem_init(void)
{
    HD_COMMON_MEM_INIT_CONFIG mem_cfg = {0};

    /* Pool 0: capture (RAW + 3A stats) */
    mem_cfg.pool_info[0].type     = HD_COMMON_MEM_COMMON_POOL;
    mem_cfg.pool_info[0].blk_size = DBGINFO_BUFSIZE()
                                  + VDO_RAW_BUFSIZE(VDO_SIZE_W, VDO_SIZE_H, CAP_OUT_FMT)
                                  + VDO_CA_BUF_SIZE(CA_WIN_NUM_W, CA_WIN_NUM_H)
                                  + VDO_LA_BUF_SIZE(LA_WIN_NUM_W, LA_WIN_NUM_H);
    mem_cfg.pool_info[0].blk_cnt  = 3;
    mem_cfg.pool_info[0].ddr_id   = USE_DDR;

    /* Pool 1: YUV (main proc output) */
    mem_cfg.pool_info[1].type     = HD_COMMON_MEM_COMMON_POOL;
    mem_cfg.pool_info[1].blk_size = DBGINFO_BUFSIZE()
                                  + VDO_YUV_BUFSIZE(VDO_SIZE_W, VDO_SIZE_H,
                                                    HD_VIDEO_PXLFMT_YUV420);
    mem_cfg.pool_info[1].blk_cnt  = 4;   /* need extra for AI pull */
    mem_cfg.pool_info[1].ddr_id   = USE_DDR;

    return hd_common_mem_init(&mem_cfg);
}

/*==========================================================================*/
/*  Sensor / videocap / videoproc / videoenc  (adapted from hd_video_snapshot) */
/*==========================================================================*/
static HD_RESULT set_cap_cfg(HD_PATH_ID *p_ctrl)
{
    HD_RESULT ret;
    HD_VIDEOCAP_DRV_CONFIG cap_cfg = {0};
    HD_VIDEOCAP_CTRL iq_ctl = {0};
    HD_PATH_ID ctrl = 0;

    snprintf(cap_cfg.sen_cfg.sen_dev.driver_name,
             HD_VIDEOCAP_SEN_NAME_LEN - 1, "nvt_sen_gc5603");
    cap_cfg.sen_cfg.sen_dev.if_type = HD_COMMON_VIDEO_IN_MIPI_CSI;
    cap_cfg.sen_cfg.sen_dev.pin_cfg.pinmux.sensor_pinmux = 0;
    cap_cfg.sen_cfg.sen_dev.pin_cfg.clk_lane_sel = HD_VIDEOCAP_SEN_CLANE_CSI(0, 0);
    cap_cfg.sen_cfg.sen_dev.pin_cfg.sen_2_serial_pin_map[0] = 0;
    cap_cfg.sen_cfg.sen_dev.pin_cfg.sen_2_serial_pin_map[1] = 1;
    for (int i = 2; i < 8; i++)
        cap_cfg.sen_cfg.sen_dev.pin_cfg.sen_2_serial_pin_map[i] = HD_VIDEOCAP_SEN_IGNORE;

    ret = hd_videocap_open(0, HD_VIDEOCAP_CTRL(SEN1_VCAP_ID), &ctrl);
    if (ret != HD_OK) return ret;
    ret |= hd_videocap_set(ctrl, HD_VIDEOCAP_PARAM_DRV_CONFIG, &cap_cfg);
    iq_ctl.func = HD_VIDEOCAP_FUNC_AE | HD_VIDEOCAP_FUNC_AWB;
    ret |= hd_videocap_set(ctrl, HD_VIDEOCAP_PARAM_CTRL, &iq_ctl);
    *p_ctrl = ctrl;
    return ret;
}

static HD_RESULT set_cap_param(HD_PATH_ID path, HD_DIM *dim)
{
    HD_RESULT ret;
    HD_VIDEOCAP_IN in = {0};
    in.sen_mode      = HD_VIDEOCAP_SEN_MODE_AUTO;
    in.frc           = HD_VIDEO_FRC_RATIO(30, 1);
    in.dim.w         = dim->w;
    in.dim.h         = dim->h;
    in.pxlfmt        = SEN_OUT_FMT;
    in.out_frame_num = HD_VIDEOCAP_SEN_FRAME_NUM_1;
    ret = hd_videocap_set(path, HD_VIDEOCAP_PARAM_IN, &in);
    if (ret != HD_OK) return ret;

    HD_VIDEOCAP_CROP crop = {0};
    crop.mode = HD_CROP_OFF;
    ret = hd_videocap_set(path, HD_VIDEOCAP_PARAM_OUT_CROP, &crop);

    HD_VIDEOCAP_OUT out = {0};
    out.pxlfmt = CAP_OUT_FMT;
    out.dir    = HD_VIDEO_DIR_NONE;
    ret = hd_videocap_set(path, HD_VIDEOCAP_PARAM_OUT, &out);
    return ret;
}

static HD_RESULT set_proc_cfg(HD_PATH_ID *p_ctrl, HD_DIM *max_dim)
{
    HD_RESULT ret;
    HD_VIDEOPROC_DEV_CONFIG cfg = {0};
    HD_VIDEOPROC_CTRL ctrl_p = {0};
    HD_PATH_ID ctrl = 0;

    ret = hd_videoproc_open(0, HD_VIDEOPROC_0_CTRL, &ctrl);
    if (ret != HD_OK) return ret;

    if (max_dim) {
        cfg.pipe              = HD_VIDEOPROC_PIPE_RAWALL;
        cfg.isp_id            = SEN1_VCAP_ID;
        cfg.ctrl_max.func     = 0;
        cfg.in_max.func       = 0;
        cfg.in_max.dim.w      = max_dim->w;
        cfg.in_max.dim.h      = max_dim->h;
        cfg.in_max.pxlfmt     = CAP_OUT_FMT;
        cfg.in_max.frc        = HD_VIDEO_FRC_RATIO(1, 1);
        ret = hd_videoproc_set(ctrl, HD_VIDEOPROC_PARAM_DEV_CONFIG, &cfg);
        if (ret != HD_OK) return HD_ERR_NG;
    }
    ctrl_p.func = 0;
    ret = hd_videoproc_set(ctrl, HD_VIDEOPROC_PARAM_CTRL, &ctrl_p);
    *p_ctrl = ctrl;
    return ret;
}

static HD_RESULT set_proc_param(HD_PATH_ID path, HD_DIM *dim)
{
    if (!dim) return HD_OK;
    HD_VIDEOPROC_OUT out = {0};
    out.func   = 0;
    out.dim.w  = dim->w;
    out.dim.h  = dim->h;
    out.pxlfmt = HD_VIDEO_PXLFMT_YUV420;
    out.dir    = HD_VIDEO_DIR_NONE;
    out.frc    = HD_VIDEO_FRC_RATIO(1, 1);
    out.depth  = 2;   /* depth >=1 to allow pull_out_buf */
    return hd_videoproc_set(path, HD_VIDEOPROC_PARAM_OUT, &out);
}

static HD_RESULT set_enc_cfg(HD_PATH_ID path, HD_DIM *max_dim, UINT32 max_br)
{
    HD_VIDEOENC_PATH_CONFIG pc = {0};
    if (!max_dim) return HD_OK;
    pc.max_mem.codec_type      = HD_CODEC_TYPE_JPEG;
    pc.max_mem.max_dim.w       = max_dim->w;
    pc.max_mem.max_dim.h       = max_dim->h;
    pc.max_mem.bitrate         = max_br;
    pc.max_mem.enc_buf_ms      = 3000;
    pc.max_mem.svc_layer       = HD_SVC_4X;
    pc.max_mem.ltr             = TRUE;
    pc.max_mem.rotate          = FALSE;
    pc.max_mem.source_output   = FALSE;
    pc.isp_id                  = SEN1_VCAP_ID;
    return hd_videoenc_set(path, HD_VIDEOENC_PARAM_PATH_CONFIG, &pc);
}

/* Always JPEG encode for snapshot */
static HD_RESULT set_enc_param(HD_PATH_ID path, HD_DIM *dim)
{
    if (!dim) return HD_OK;
    HD_RESULT ret;
    HD_VIDEOENC_IN in = {0};
    in.dir     = HD_VIDEO_DIR_NONE;
    in.pxl_fmt = HD_VIDEO_PXLFMT_YUV420;
    in.dim.w   = dim->w;
    in.dim.h   = dim->h;
    in.frc     = HD_VIDEO_FRC_RATIO(1, 1);
    ret = hd_videoenc_set(path, HD_VIDEOENC_PARAM_IN, &in);
    if (ret != HD_OK) return ret;

    HD_VIDEOENC_OUT out = {0};
    out.codec_type = HD_CODEC_TYPE_JPEG;
    out.jpeg.image_quality = 90;
    ret = hd_videoenc_set(path, HD_VIDEOENC_PARAM_OUT_ENC_PARAM, &out);
    return ret;
}

/*==========================================================================*/
/*  AI3 model loading  (adapted from ai3_custom_yolov8)                     */
/*==========================================================================*/
static HD_RESULT ai3_init(void)
{
    VENDOR_AI3_DEV_CFG dev_cfg = {0};
    HD_RESULT ret = vendor_ai3_dev_init(&dev_cfg);
    if (ret != HD_OK) { printf("[AI3] dev_init fail %d\n", ret); return ret; }

    VENDOR_AI3_VER ver = {0};
    if (vendor_ai3_dev_get(VENDOR_AI3_CFG_VER, &ver) == HD_OK) {
        printf("[AI3] vendor_ai ver: %s  kflow ver: %s\n",
               ver.vendor_ai_impl_version, ver.kflow_ai_impl_version);
    }
    return HD_OK;
}

static HD_RESULT ai3_load_model(APP_CONTEXT *c)
{
    HD_RESULT ret;
    INT32 msz = get_file_size(c->model_path);
    if (msz <= 0) return HD_ERR_NG;

    ret = mem_alloc(&c->model_mem, "mdl", msz);
    if (ret != HD_OK) return ret;
    if (load_file(c->model_path, c->model_mem.va, msz) <= 0) return HD_ERR_NG;

    VENDOR_AI3_MODEL_INFO mi = {0};
    mi.model_buf.pa   = c->model_mem.pa;
    mi.model_buf.va   = c->model_mem.va;
    mi.model_buf.size = c->model_mem.size;
    ret = vendor_ai3_dev_get(VENDOR_AI3_CFG_MODEL_INFO, &mi);
    if (ret != HD_OK) {
        mi.proc_mem.buf[AI3_PROC_BUF_WORKBUF].size   = 16 * 1024 * 1024;
        mi.proc_mem.buf[AI3_PROC_BUF_RONLYBUF].size  =  8 * 1024 * 1024;
    }

    ret = mem_alloc(&c->work_mem, "ai_work",
                    mi.proc_mem.buf[AI3_PROC_BUF_WORKBUF].size);
    if (ret != HD_OK) return ret;
    ret = mem_alloc(&c->ronly_mem, "ai_ronly",
                    mi.proc_mem.buf[AI3_PROC_BUF_RONLYBUF].size);
    if (ret != HD_OK) return ret;

    VENDOR_AI3_PROC_CFG pc = {0};
    pc.model_buf.pa   = c->model_mem.pa;
    pc.model_buf.va   = c->model_mem.va;
    pc.model_buf.size = c->model_mem.size;
    pc.proc_mem.buf[AI3_PROC_BUF_WORKBUF].pa   = c->work_mem.pa;
    pc.proc_mem.buf[AI3_PROC_BUF_WORKBUF].va   = c->work_mem.va;
    pc.proc_mem.buf[AI3_PROC_BUF_WORKBUF].size = c->work_mem.size;
    pc.proc_mem.buf[AI3_PROC_BUF_RONLYBUF].pa   = c->ronly_mem.pa;
    pc.proc_mem.buf[AI3_PROC_BUF_RONLYBUF].va   = c->ronly_mem.va;
    pc.proc_mem.buf[AI3_PROC_BUF_RONLYBUF].size = c->ronly_mem.size;
    pc.plugin[AI3_PLUGIN_CPU] = vendor_ai_cpu1_get_engine();

    ret = vendor_ai3_net_open(&c->proc_id, &pc, &c->net_info);
    if (ret != HD_OK) { printf("[AI3] net_open fail %d\n", ret); return ret; }

    printf("[AI3] net_open OK: proc_id=%u, in=%u, out=%u\n",
           c->proc_id, c->net_info.in_buf_cnt, c->net_info.out_buf_cnt);

    ret = vendor_ai3_net_start(c->proc_id);
    if (ret != HD_OK) { printf("[AI3] net_start fail %d\n", ret); return ret; }
    printf("[AI3] net_start OK\n");

    /* allocate & register output buffers */
    c->out_mem = (MEM_PARM *)calloc(c->net_info.out_buf_cnt, sizeof(MEM_PARM));
    if (!c->out_mem) return HD_ERR_NOMEM;

    for (UINT32 i = 0; i < c->net_info.out_buf_cnt; i++) {
        VENDOR_AI3_BUF ab = {0};
        ret = vendor_ai3_net_get(c->proc_id, c->net_info.out_path_list[i], &ab);
        if (ret != HD_OK) { printf("[AI3] out_get[%u] fail %d\n", i, ret); return ret; }
        printf("[AI3] out[%u]: %ux%ux%u fmt=0x%x sz=%u\n",
               i, ab.width, ab.height, ab.channel, ab.fmt, ab.size);

        char nm[32]; snprintf(nm, sizeof(nm), "ai_o%u", i);
        ret = mem_alloc(&c->out_mem[i], nm, ab.size);
        if (ret != HD_OK) { printf("[AI3] out_alloc[%u] fail %d\n", i, ret); return ret; }

        ab.pa   = c->out_mem[i].pa;
        ab.va   = c->out_mem[i].va;
        ab.size = c->out_mem[i].size;
        ret = vendor_ai3_net_set(c->proc_id, c->net_info.out_path_list[i], &ab);
        if (ret != HD_OK) { printf("[AI3] out_set[%u] fail %d\n", i, ret); return ret; }
        printf("[AI3] out_set[%u] OK pa=0x%lx va=0x%lx sz=%u\n",
               i, (unsigned long)ab.pa, (unsigned long)ab.va, ab.size);
    }
    printf("[AI3] Model loaded. in=%u out=%u\n",
           c->net_info.in_buf_cnt, c->net_info.out_buf_cnt);
    return HD_OK;
}

static void ai3_unload(APP_CONTEXT *c)
{
    if (c->proc_id) {
        vendor_ai3_net_stop(c->proc_id);
        vendor_ai3_net_close(c->proc_id);
        c->proc_id = 0;
    }
    if (c->out_mem) {
        for (UINT32 i = 0; i < c->net_info.out_buf_cnt; i++)
            mem_free(&c->out_mem[i]);
        free(c->out_mem);
        c->out_mem = NULL;
    }
    mem_free(&c->ronly_mem);
    mem_free(&c->work_mem);
    mem_free(&c->model_mem);
}

/*==========================================================================*/
/*  AI inference on a YUV frame pulled from videoproc                       */
/*==========================================================================*/
static HD_RESULT ai3_infer_frame(APP_CONTEXT *c, HD_VIDEO_FRAME *frame)
{
    HD_RESULT ret;

    /* --- Set input from frame --- */
    VENDOR_AI3_BUF in_buf = {0};
    ret = vendor_ai3_net_get(c->proc_id, c->net_info.in_path_list[0], &in_buf);
    if (ret != HD_OK) return ret;

    in_buf.pa       = frame->phy_addr[0];
    in_buf.va       = 0; /* NPU only needs PA for DMA */
    in_buf.width    = frame->dim.w;
    in_buf.height   = frame->dim.h;
    in_buf.line_ofs = frame->loff[0];
    in_buf.size     = frame->loff[0] * frame->dim.h * 3 / 2;

    ret = vendor_ai3_net_set(c->proc_id, c->net_info.in_path_list[0], &in_buf);
    if (ret != HD_OK) return ret;

    /* --- Re-set output buffers (consumed after each proc) --- */
    for (UINT32 i = 0; i < c->net_info.out_buf_cnt; i++) {
        VENDOR_AI3_BUF ob = {0};
        vendor_ai3_net_get(c->proc_id, c->net_info.out_path_list[i], &ob);
        ob.pa   = c->out_mem[i].pa;
        ob.va   = c->out_mem[i].va;
        ob.size = c->out_mem[i].size;
        vendor_ai3_net_set(c->proc_id, c->net_info.out_path_list[i], &ob);
    }

    /* --- Run inference --- */
    ret = vendor_ai3_net_proc(c->proc_id);
    if (ret != HD_OK) {
        static int err_cnt = 0;
        if (err_cnt++ < 5)
            printf("[AI3] proc fail %d (proc_id=%u, cnt=%d)\n",
                   ret, c->proc_id, err_cnt);
        return ret;
    }

    /* flush output cache */
    for (UINT32 i = 0; i < c->net_info.out_buf_cnt; i++) {
        VENDOR_AI3_BUF tmp = {0};
        vendor_ai3_net_get(c->proc_id, c->net_info.out_path_list[i], &tmp);
        hd_common_mem_flush_cache((void *)tmp.va, tmp.size);
    }

    /* --- Post-process --- */
    if (c->model_type == MODEL_TYPE_YOLOV8) {
        VENDOR_AI3_BUF ob = {0};
        vendor_ai3_net_get(c->proc_id, c->net_info.out_path_list[0], &ob);

        YOLOV8_POSTPROC_PARM pp = {0};
        pp.out_addr         = ob.va;
        pp.out_fmt          = ob.fmt;
        pp.scale_ratio      = ob.scale_ratio;
        pp.zero_point       = ob.zero_point;
        pp.cfg.conf_threshold = 0.15f;
        pp.cfg.nms_threshold  = 0.45f;
        pp.cfg.num_classes    = YOLOV8_NUM_CLASSES;
        pp.cfg.input_width    = AI_INPUT_W;
        pp.cfg.input_height   = AI_INPUT_H;
        pp.results            = &c->yolo_results;
        ret = yolov8_postproc_process(&pp);

        /* copy to shared latest */
        pthread_mutex_lock(&c->det_mutex);
        memcpy(&c->latest_yolo, &c->yolo_results, sizeof(c->latest_yolo));
        c->latest_valid = 1;
        pthread_mutex_unlock(&c->det_mutex);

    } else if (c->model_type == MODEL_TYPE_SCRFD) {
        SCRFD_POSTPROC_PARM pp = {0};
        pp.num_outputs       = c->net_info.out_buf_cnt;
        pp.cfg.conf_threshold = 0.3f;
        pp.cfg.nms_threshold  = 0.4f;
        pp.cfg.input_width    = AI_INPUT_W;
        pp.cfg.input_height   = AI_INPUT_H;
        pp.results            = &c->scrfd_results;
        for (UINT32 i = 0; i < c->net_info.out_buf_cnt && i < 9; i++) {
            VENDOR_AI3_BUF ob = {0};
            vendor_ai3_net_get(c->proc_id, c->net_info.out_path_list[i], &ob);
            pp.outputs[i].va          = ob.va;
            pp.outputs[i].fmt         = ob.fmt;
            pp.outputs[i].scale_ratio = ob.scale_ratio;
            pp.outputs[i].zero_point  = ob.zero_point;
            pp.outputs[i].width       = ob.width;
            pp.outputs[i].height      = ob.height;
            pp.outputs[i].channel     = ob.channel;
            pp.outputs[i].size        = ob.size;
        }
        ret = scrfd_postproc_process(&pp);

        pthread_mutex_lock(&c->det_mutex);
        memcpy(&c->latest_scrfd, &c->scrfd_results, sizeof(c->latest_scrfd));
        c->latest_valid = 1;
        pthread_mutex_unlock(&c->det_mutex);
    }
    return ret;
}

/*==========================================================================*/
/*  Software YUV420 NV12 bounding-box drawing helpers                       */
/*==========================================================================*/

/* Draw a horizontal line in the Y plane */
static inline void sw_hline_y(UINT8 *y_base, UINT32 stride,
                              UINT32 x1, UINT32 x2, UINT32 y,
                              UINT8 val, UINT32 thick)
{
    for (UINT32 t = 0; t < thick; t++) {
        UINT8 *row = y_base + (y + t) * stride;
        memset(row + x1, val, x2 - x1);
    }
}

/* Draw a vertical line in the Y plane */
static inline void sw_vline_y(UINT8 *y_base, UINT32 stride,
                              UINT32 x, UINT32 y1, UINT32 y2,
                              UINT8 val, UINT32 thick)
{
    for (UINT32 r = y1; r < y2; r++) {
        UINT8 *row = y_base + r * stride;
        memset(row + x, val, thick);
    }
}

/* Fill UV (NV12 interleaved) for a horizontal stripe */
static inline void sw_hline_uv(UINT8 *uv_base, UINT32 stride,
                               UINT32 x1, UINT32 x2, UINT32 y,
                               UINT8 u_val, UINT8 v_val, UINT32 thick)
{
    /* UV is half-res; NV12 interleaved U,V,U,V... */
    UINT32 ux1 = (x1 & ~1u);
    UINT32 ux2 = (x2 & ~1u);
    for (UINT32 t = 0; t < thick / 2; t++) {
        UINT8 *row = uv_base + ((y / 2) + t) * stride;
        for (UINT32 x = ux1; x < ux2; x += 2) {
            row[x]     = u_val;
            row[x + 1] = v_val;
        }
    }
}

/* Fill UV for a vertical stripe */
static inline void sw_vline_uv(UINT8 *uv_base, UINT32 stride,
                               UINT32 x, UINT32 y1, UINT32 y2,
                               UINT8 u_val, UINT8 v_val, UINT32 thick)
{
    UINT32 ux = (x & ~1u);
    for (UINT32 r = y1 / 2; r < y2 / 2; r++) {
        UINT8 *row = uv_base + r * stride;
        for (UINT32 t = 0; t < thick; t += 2) {
            row[ux + t]     = u_val;
            row[ux + t + 1] = v_val;
        }
    }
}

/* Draw a hollow rectangle on a NV12 frame (software, on virtual address) */
static void sw_draw_rect_nv12(UINT8 *y_va, UINT8 *uv_va, UINT32 stride,
                              UINT32 fw, UINT32 fh,
                              UINT32 x1, UINT32 y1, UINT32 x2, UINT32 y2,
                              UINT8 yc, UINT8 uc, UINT8 vc, UINT32 thick)
{
    /* Clamp to even boundaries and frame bounds */
    x1 = (x1 & ~1u);  x2 = (x2 & ~1u);
    y1 = (y1 & ~1u);  y2 = (y2 & ~1u);
    if (x2 + thick > fw) x2 = fw - thick;
    if (y2 + thick > fh) y2 = fh - thick;
    if (x1 >= x2 || y1 >= y2) return;

    /* Top edge */
    sw_hline_y (y_va,  stride, x1, x2, y1, yc, thick);
    sw_hline_uv(uv_va, stride, x1, x2, y1, uc, vc, thick);
    /* Bottom edge */
    sw_hline_y (y_va,  stride, x1, x2 + thick, y2, yc, thick);
    sw_hline_uv(uv_va, stride, x1, x2 + thick, y2, uc, vc, thick);
    /* Left edge */
    sw_vline_y (y_va,  stride, x1, y1, y2, yc, thick);
    sw_vline_uv(uv_va, stride, x1, y1, y2, uc, vc, thick);
    /* Right edge */
    sw_vline_y (y_va,  stride, x2, y1, y2 + thick, yc, thick);
    sw_vline_uv(uv_va, stride, x2, y1, y2 + thick, uc, vc, thick);
}

/*==========================================================================*/
/*  Draw bounding boxes on YUV420 frame (software, needs mmap'd VA)         */
/*==========================================================================*/
static void draw_bbox_yolo(APP_CONTEXT *c, HD_VIDEO_FRAME *frame, UINTPTR y_va)
{
    YOLOV8_RESULTS res;
    pthread_mutex_lock(&c->det_mutex);
    if (!c->latest_valid) {
        pthread_mutex_unlock(&c->det_mutex);
        printf("[GFX] No valid detections yet\n");
        return;
    }
    memcpy(&res, &c->latest_yolo, sizeof(res));
    pthread_mutex_unlock(&c->det_mutex);

    UINT32 fw     = frame->dim.w;
    UINT32 fh     = frame->dim.h;
    UINT32 stride = frame->loff[0];
    UINT8 *y_ptr  = (UINT8 *)y_va;
    UINT8 *uv_ptr = y_ptr + stride * fh;  /* NV12: UV plane follows Y */

    printf("[GFX] Drawing %d YOLOv8 boxes on %ux%u frame (va=0x%lx stride=%u)\n",
           res.count, fw, fh, (unsigned long)y_va, stride);

    for (int i = 0; i < res.count && i < MAX_DRAW_BBOX; i++) {
        YOLOV8_DETECTION *d = &res.detections[i];
        UINT32 x1 = (UINT32)(d->x1 * fw);
        UINT32 y1c = (UINT32)(d->y1 * fh);
        UINT32 x2 = (UINT32)(d->x2 * fw);
        UINT32 y2c = (UINT32)(d->y2 * fh);

        UINT8 cy, cu, cv;
        if (d->class_id == 0)           { cy = BBOX_COLOR_Y_0; cu = BBOX_COLOR_U_0; cv = BBOX_COLOR_V_0; }
        else if (d->class_id <= 7)      { cy = BBOX_COLOR_Y_1; cu = BBOX_COLOR_U_1; cv = BBOX_COLOR_V_1; }
        else                            { cy = BBOX_COLOR_Y_2; cu = BBOX_COLOR_U_2; cv = BBOX_COLOR_V_2; }

        printf("[GFX]   box[%d] cls=%d conf=%.2f (%u,%u)-(%u,%u) Y=%u U=%u V=%u\n",
               i, d->class_id, d->confidence, x1, y1c, x2, y2c, cy, cu, cv);

        sw_draw_rect_nv12(y_ptr, uv_ptr, stride, fw, fh,
                          x1, y1c, x2, y2c, cy, cu, cv, BBOX_THICKNESS);
    }
}

static void draw_bbox_scrfd(APP_CONTEXT *c, HD_VIDEO_FRAME *frame, UINTPTR y_va)
{
    SCRFD_RESULTS res;
    pthread_mutex_lock(&c->det_mutex);
    if (!c->latest_valid) { pthread_mutex_unlock(&c->det_mutex); return; }
    memcpy(&res, &c->latest_scrfd, sizeof(res));
    pthread_mutex_unlock(&c->det_mutex);

    UINT32 fw     = frame->dim.w;
    UINT32 fh     = frame->dim.h;
    UINT32 stride = frame->loff[0];
    UINT8 *y_ptr  = (UINT8 *)y_va;
    UINT8 *uv_ptr = y_ptr + stride * fh;

    for (int i = 0; i < res.count && i < MAX_DRAW_BBOX; i++) {
        SCRFD_FACE *f = &res.faces[i];
        UINT32 x1 = (UINT32)(f->x1 * fw);
        UINT32 y1c = (UINT32)(f->y1 * fh);
        UINT32 x2 = (UINT32)(f->x2 * fw);
        UINT32 y2c = (UINT32)(f->y2 * fh);

        sw_draw_rect_nv12(y_ptr, uv_ptr, stride, fw, fh,
                          x1, y1c, x2, y2c,
                          BBOX_COLOR_Y_2, BBOX_COLOR_U_2, BBOX_COLOR_V_2,
                          BBOX_THICKNESS);
    }
}

/*==========================================================================*/
/*  Set VENDOR_VIDEOENC_SMART_BBOX for VLC overlay (like turnkey sample)    */
/*==========================================================================*/
static void set_smart_bbox_yolo(APP_CONTEXT *c)
{
    YOLOV8_RESULTS res;
    pthread_mutex_lock(&c->det_mutex);
    if (!c->latest_valid) { pthread_mutex_unlock(&c->det_mutex); return; }
    memcpy(&res, &c->latest_yolo, sizeof(res));
    pthread_mutex_unlock(&c->det_mutex);

    VENDOR_VIDEOENC_SMART_BBOX sb = {0};
    sb.base_resolution.w = VDO_SIZE_W;
    sb.base_resolution.h = VDO_SIZE_H;
    UINT8 n = (res.count > VENDOR_VIDEOENC_SMART_BBOX_MAX_NUM)
            ? VENDOR_VIDEOENC_SMART_BBOX_MAX_NUM : (UINT8)res.count;
    sb.bbox_num  = n;
    sb.timestamp = 33333;

    for (UINT8 i = 0; i < n; i++) {
        YOLOV8_DETECTION *d = &res.detections[i];
        sb.bbox[i].positions[VENDOR_VIDEOENC_SMART_BBOX_POS_TOP_LEFT].x =
            (UINT32)(d->x1 * VDO_SIZE_W);
        sb.bbox[i].positions[VENDOR_VIDEOENC_SMART_BBOX_POS_TOP_LEFT].y =
            (UINT32)(d->y1 * VDO_SIZE_H);
        sb.bbox[i].positions[VENDOR_VIDEOENC_SMART_BBOX_POS_BOTTOM_RIGHT].x =
            (UINT32)(d->x2 * VDO_SIZE_W);
        sb.bbox[i].positions[VENDOR_VIDEOENC_SMART_BBOX_POS_BOTTOM_RIGHT].y =
            (UINT32)(d->y2 * VDO_SIZE_H);
        /* class 0 = person (high priority), others lower */
        sb.bbox[i].class_id = (d->class_id == 0)
            ? VENDOR_VIDEOENC_SMART_ROI_CLASS0
            : VENDOR_VIDEOENC_SMART_ROI_CLASS1;
    }

    HD_RESULT r = vendor_videoenc_set(c->enc_path,
                                      VENDOR_VIDEOENC_PARAM_SMART_BBOX, &sb);
    if (r != HD_OK) {
        printf("[BBOX] vendor_videoenc_set smart_bbox err=%d\n", r);
    }
}

static void set_smart_bbox_scrfd(APP_CONTEXT *c)
{
    SCRFD_RESULTS res;
    pthread_mutex_lock(&c->det_mutex);
    if (!c->latest_valid) { pthread_mutex_unlock(&c->det_mutex); return; }
    memcpy(&res, &c->latest_scrfd, sizeof(res));
    pthread_mutex_unlock(&c->det_mutex);

    VENDOR_VIDEOENC_SMART_BBOX sb = {0};
    sb.base_resolution.w = VDO_SIZE_W;
    sb.base_resolution.h = VDO_SIZE_H;
    UINT8 n = (res.count > VENDOR_VIDEOENC_SMART_BBOX_MAX_NUM)
            ? VENDOR_VIDEOENC_SMART_BBOX_MAX_NUM : (UINT8)res.count;
    sb.bbox_num  = n;
    sb.timestamp = 33333;

    for (UINT8 i = 0; i < n; i++) {
        SCRFD_FACE *f = &res.faces[i];
        sb.bbox[i].positions[VENDOR_VIDEOENC_SMART_BBOX_POS_TOP_LEFT].x =
            (UINT32)(f->x1 * VDO_SIZE_W);
        sb.bbox[i].positions[VENDOR_VIDEOENC_SMART_BBOX_POS_TOP_LEFT].y =
            (UINT32)(f->y1 * VDO_SIZE_H);
        sb.bbox[i].positions[VENDOR_VIDEOENC_SMART_BBOX_POS_BOTTOM_RIGHT].x =
            (UINT32)(f->x2 * VDO_SIZE_W);
        sb.bbox[i].positions[VENDOR_VIDEOENC_SMART_BBOX_POS_BOTTOM_RIGHT].y =
            (UINT32)(f->y2 * VDO_SIZE_H);
        sb.bbox[i].class_id = VENDOR_VIDEOENC_SMART_ROI_CLASS0;
    }

    vendor_videoenc_set(c->enc_path, VENDOR_VIDEOENC_PARAM_SMART_BBOX, &sb);
}

/*==========================================================================*/
/*  AI thread – continuously pulls YUV, runs inference, updates results     */
/*==========================================================================*/
static void *ai_thread(void *arg)
{
    APP_CONTEXT *c = (APP_CONTEXT *)arg;
    HD_RESULT ret;
    HD_VIDEO_FRAME frame = {0};

    while (!c->flow_start) usleep(100000);         /* wait for pipeline start */
    printf("[AI] AI inference thread running\n");

    while (!c->ai_exit) {
        if (!c->ai_running) { usleep(50000); continue; }

        /* Pull a YUV frame from videoproc */
        ret = hd_videoproc_pull_out_buf(c->proc_path, &frame, 500);
        if (ret != HD_OK) { usleep(10000); continue; }

        /* Run AI inference */
        ai3_infer_frame(c, &frame);

        /* Update smart bbox for VLC overlay */
        if (c->model_type == MODEL_TYPE_YOLOV8)
            set_smart_bbox_yolo(c);
        else
            set_smart_bbox_scrfd(c);

        /* Release frame */
        hd_videoproc_release_out_buf(c->proc_path, &frame);
    }
    printf("[AI] AI inference thread exiting\n");
    return NULL;
}

/*==========================================================================*/
/*  Snapshot thread – on demand, pulls YUV, draws bbox, encodes JPEG        */
/*==========================================================================*/
static void *snapshot_thread(void *arg)
{
    APP_CONTEXT *c = (APP_CONTEXT *)arg;
    HD_RESULT ret;
    HD_VIDEOENC_BS  bs_pull;
    HD_VIDEO_FRAME  frame = {0};
    UINTPTR vir_addr = 0;
    HD_VIDEOENC_BUFINFO buf_info;
    char filepath[64];
    FILE *fout;

    while (!c->flow_start) usleep(100000);

    /* map encoder BS buffer once */
    hd_videoenc_get(c->enc_path, HD_VIDEOENC_PARAM_BUFINFO, &buf_info);
    vir_addr = (UINTPTR)hd_common_mem_mmap(HD_COMMON_MEM_MEM_TYPE_CACHE,
                                            buf_info.buf_info.phy_addr,
                                            buf_info.buf_info.buf_size);
    if (!vir_addr) {
        printf("[SNAP] mmap enc BS failed!\n");
        return NULL;
    }
    #define PHY2VIRT(pa) (vir_addr + ((pa) - buf_info.buf_info.phy_addr))

    printf("[SNAP] Snapshot thread ready. Press 's' to snap.\n");

    while (!c->snap_exit) {
        if (!c->do_snap) { usleep(50000); continue; }
        c->do_snap = 0;

        printf("[SNAP] Taking snapshot #%u with bbox overlay...\n", c->shot_count);

        /* Pull a YUV frame */
        ret = hd_videoproc_pull_out_buf(c->proc_path, &frame, 1000);
        if (ret != HD_OK) {
            printf("[SNAP] pull_out fail %d\n", ret);
            continue;
        }

        /* mmap the YUV frame so we can draw on it in software */
        UINT32 yuv_size = frame.loff[0] * frame.dim.h * 3 / 2;
        UINTPTR yuv_va = (UINTPTR)hd_common_mem_mmap(
            HD_COMMON_MEM_MEM_TYPE_CACHE,
            frame.phy_addr[0], yuv_size);

        if (yuv_va) {
            /* --- Draw bounding boxes on the YUV frame (software) --- */
            if (c->model_type == MODEL_TYPE_YOLOV8)
                draw_bbox_yolo(c, &frame, yuv_va);
            else
                draw_bbox_scrfd(c, &frame, yuv_va);

            /* Flush CPU cache so HW encoder sees the drawn pixels */
            hd_common_mem_flush_cache((void *)yuv_va, yuv_size);
            hd_common_mem_munmap((void *)yuv_va, yuv_size);
        } else {
            printf("[SNAP] mmap YUV frame failed (pa=0x%lx sz=%u)\n",
                   (unsigned long)frame.phy_addr[0], yuv_size);
        }

        /* Push annotated frame to encoder for JPEG */
        ret = hd_videoenc_push_in_buf(c->enc_path, &frame, NULL, 0);
        if (ret != HD_OK) {
            printf("[SNAP] enc push fail %d\n", ret);
            hd_videoproc_release_out_buf(c->proc_path, &frame);
            continue;
        }

        /* Release the videoproc frame */
        hd_videoproc_release_out_buf(c->proc_path, &frame);

        /* Pull encoded JPEG */
        ret = hd_videoenc_pull_out_buf(c->enc_path, &bs_pull, -1);
        if (ret != HD_OK) {
            printf("[SNAP] enc pull fail %d\n", ret);
            continue;
        }

        /* Write JPEG to SD card */
        snprintf(filepath, sizeof(filepath),
                 "/mnt/sd/snap_bbox_%04u.jpg", c->shot_count);
        fout = fopen(filepath, "wb");
        if (fout) {
            for (UINT32 j = 0; j < bs_pull.pack_num; j++) {
                UINT8 *ptr = (UINT8 *)PHY2VIRT(bs_pull.video_pack[j].phy_addr);
                fwrite(ptr, 1, bs_pull.video_pack[j].size, fout);
            }
            fclose(fout);
            printf("[SNAP] Saved %s\n", filepath);
        } else {
            printf("[SNAP] fopen fail: %s\n", filepath);
        }

        hd_videoenc_release_out_buf(c->enc_path, &bs_pull);
        c->shot_count++;
    }

    hd_common_mem_munmap((void *)vir_addr, buf_info.buf_info.buf_size);
    printf("[SNAP] Snapshot thread exiting\n");
    return NULL;
}

/*==========================================================================*/
/*  Module init / open / close / exit  (from hd_video_snapshot)             */
/*==========================================================================*/
static HD_RESULT init_module(void)
{
    HD_RESULT ret;
    if ((ret = hd_videocap_init())  != HD_OK) return ret;
    if ((ret = hd_videoproc_init()) != HD_OK) return ret;
    if ((ret = hd_videoenc_init())  != HD_OK) return ret;
    if ((ret = hd_gfx_init())      != HD_OK) return ret;
    return HD_OK;
}

static HD_RESULT open_module(APP_CONTEXT *c)
{
    HD_RESULT ret;
    ret = set_cap_cfg(&c->cap_ctrl);
    if (ret != HD_OK) return ret;
    ret = set_proc_cfg(&c->proc_ctrl, &c->proc_max_dim);
    if (ret != HD_OK) return ret;

    if ((ret = hd_videocap_open(HD_VIDEOCAP_IN(SEN1_VCAP_ID, 0),
                                HD_VIDEOCAP_OUT(SEN1_VCAP_ID, 0),
                                &c->cap_path)) != HD_OK) return ret;
    if ((ret = hd_videoproc_open(HD_VIDEOPROC_0_IN_0,
                                 HD_VIDEOPROC_0_OUT_0,
                                 &c->proc_path)) != HD_OK) return ret;
    if ((ret = hd_videoenc_open(HD_VIDEOENC_0_IN_0,
                                HD_VIDEOENC_0_OUT_0,
                                &c->enc_path)) != HD_OK) return ret;
    return HD_OK;
}

static HD_RESULT close_module(APP_CONTEXT *c)
{
    hd_videocap_close(c->cap_path);
    hd_videoproc_close(c->proc_path);
    hd_videoenc_close(c->enc_path);
    return HD_OK;
}

static void exit_module(void)
{
    hd_gfx_uninit();
    hd_videocap_uninit();
    hd_videoproc_uninit();
    hd_videoenc_uninit();
}

/*==========================================================================*/
/*  MAIN                                                                    */
/*==========================================================================*/
static void print_usage(const char *prog)
{
    printf("Usage:\n");
    printf("  %s <model.bin> [labels.txt]\n", prog);
    printf("  %s --scrfd <model.bin>\n", prog);
    printf("\nKeys:\n");
    printf("  s - Snapshot (JPEG with bbox drawn on frame)\n");
    printf("  r - Toggle AI inference ON/OFF\n");
    printf("  d - HDAL debug menu\n");
    printf("  q - Quit\n");
}

MAIN(argc, argv)
{
    HD_RESULT ret;
    INT key;
    APP_CONTEXT *c = &g_ctx;
    HD_DIM main_dim;

    printf("\n");
    printf("==============================================\n");
    printf("  AI3 YOLOv8 Snapshot + OSG BBox Sample\n");
    printf("  Novatek NT98538\n");
    printf("==============================================\n\n");

    /* --- Parse args --- */
    int aoff = 1;
    c->model_type = MODEL_TYPE_YOLOV8;
    if (argc > 1 && strcmp(argv[1], "--scrfd") == 0) {
        c->model_type = MODEL_TYPE_SCRFD;
        aoff = 2;
    }
    if (argc - aoff < 1) { print_usage(argv[0]); return -1; }

    strncpy(c->model_path, argv[aoff], sizeof(c->model_path) - 1);
    if (argc - aoff > 1)
        strncpy(c->labels_path, argv[aoff + 1], sizeof(c->labels_path) - 1);

    printf("[MAIN] Mode  : %s\n",
           c->model_type == MODEL_TYPE_SCRFD ? "SCRFD" : "YOLOv8");
    printf("[MAIN] Model : %s\n", c->model_path);
    printf("[MAIN] Labels: %s\n", c->labels_path[0] ? c->labels_path : "(none)");

    signal(SIGINT,  signal_handler);
    signal(SIGTERM, signal_handler);
    pthread_mutex_init(&c->det_mutex, NULL);

    /* --- Init HDAL --- */
    ret = hd_common_init(0);
    if (ret != HD_OK) { printf("common init fail %d\n", ret); goto done; }

    /* Enable AI/CNN engine – required when running standalone (no ipcamd) */
    hd_common_sysconfig(0, VENDOR_AI_CFG_ENABLE_CNN, 0, VENDOR_AI_CFG);

    ret = hdal_mem_init();
    if (ret != HD_OK) { printf("mem init fail %d\n", ret); goto done; }

    ret = init_module();
    if (ret != HD_OK) { printf("module init fail %d\n", ret); goto done; }

    /* --- Open video pipeline --- */
    c->proc_max_dim.w = VDO_SIZE_W;
    c->proc_max_dim.h = VDO_SIZE_H;
    ret = open_module(c);
    if (ret != HD_OK) { printf("open fail %d\n", ret); goto cleanup; }

    /* --- Get cap capabilities --- */
    hd_videocap_get(c->cap_ctrl, HD_VIDEOCAP_PARAM_SYSCAPS, &c->cap_syscaps);

    /* --- Set capture params --- */
    c->cap_dim.w = VDO_SIZE_W;
    c->cap_dim.h = VDO_SIZE_H;
    set_cap_param(c->cap_path, &c->cap_dim);

    /* --- Set proc params --- */
    main_dim.w = VDO_SIZE_W;
    main_dim.h = VDO_SIZE_H;
    c->proc_dim = main_dim;
    set_proc_param(c->proc_path, &c->proc_dim);

    /* --- Set enc params (JPEG) --- */
    c->enc_max_dim = main_dim;
    set_enc_cfg(c->enc_path, &c->enc_max_dim, 4 * 1024 * 1024);
    c->enc_dim = main_dim;
    set_enc_param(c->enc_path, &c->enc_dim);

    /* --- Bind cap -> proc --- */
    hd_videocap_bind(HD_VIDEOCAP_OUT(SEN1_VCAP_ID, 0), HD_VIDEOPROC_0_IN_0);

    /* --- Init AI3 & load model --- */
    ret = ai3_init();
    if (ret != HD_OK) { printf("ai3 init fail %d\n", ret); goto cleanup; }

    if (c->model_type == MODEL_TYPE_YOLOV8) {
        yolov8_postproc_init();
        if (c->labels_path[0]) {
            if (yolov8_load_labels(c->labels_path, c->labels) == HD_OK)
                c->labels_loaded = 1;
        }
    } else {
        scrfd_postproc_init();
    }

    ret = ai3_load_model(c);
    if (ret != HD_OK) { printf("model load fail %d\n", ret); goto cleanup; }

    /* --- Create threads --- */
    c->ai_running = 1;
    pthread_create(&c->ai_thread_id,   NULL, ai_thread,       c);
    pthread_create(&c->snap_thread_id, NULL, snapshot_thread,  c);

    /* --- Start pipeline --- */
    hd_videocap_start(c->cap_path);
    hd_videoproc_start(c->proc_path);
    sleep(1);   /* let AE/AWB stabilise */
    hd_videoenc_start(c->enc_path);
    c->flow_start = 1;

    /* --- Interactive loop --- */
    printf("\nReady.  s=snap  r=toggle-AI  d=debug  q=quit\n");
    while (g_running) {
        key = GETCHAR();
        switch (key) {
        case 's':
            c->do_snap = 1;
            break;
        case 'r':
            c->ai_running = !c->ai_running;
            printf("[MAIN] AI inference %s\n",
                   c->ai_running ? "ON" : "OFF");
            break;
#if DEBUG_MENU
        case 'd':
            hd_debug_run_menu();
            printf("\nReady.  s=snap  r=toggle-AI  d=debug  q=quit\n");
            break;
#endif
        case 'q': case 0x3:
            g_running = 0;
            break;
        default:
            break;
        }
    }

    /* --- Shutdown --- */
    c->ai_exit  = 1;
    c->snap_exit = 1;

    hd_videocap_stop(c->cap_path);
    hd_videoproc_stop(c->proc_path);
    hd_videoenc_stop(c->enc_path);
    hd_videocap_unbind(HD_VIDEOCAP_OUT(SEN1_VCAP_ID, 0));

    pthread_join(c->ai_thread_id,   NULL);
    pthread_join(c->snap_thread_id, NULL);

    ai3_unload(c);

cleanup:
    yolov8_postproc_uninit();
    scrfd_postproc_uninit();
    vendor_ai3_dev_uninit();
    close_module(c);
    exit_module();
    hd_common_mem_uninit();

done:
    hd_common_uninit();
    pthread_mutex_destroy(&c->det_mutex);
    printf("[MAIN] Done\n");
    return (ret == HD_OK) ? 0 : -1;
}
