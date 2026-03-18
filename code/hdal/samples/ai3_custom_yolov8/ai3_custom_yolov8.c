/**
 * @file ai3_custom_yolov8.c
 * @brief Custom YOLOv8 AI3 sample for Novatek NT98538
 *
 * Sample application demonstrating how to load and run custom
 * YOLOv8 models compiled for the Novatek AI3 NPU.
 *
 * Usage:
 *   ./ai3_custom_yolov8 <model.bin> <input.bin> [labels.txt]
 *
 * Example:
 *   ./ai3_custom_yolov8 /mnt/sd/models/yolov8n.bin /mnt/sd/input/test_640x640.bin /mnt/sd/models/coco_labels.txt
 *
 * Copyright 2026. All rights reserved.
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <pthread.h>

#include "hdal.h"
#include "hd_debug.h"
#include "vendor_ai.h"
#include "vendor_ai_cpu/vendor_ai_cpu.h"
#include "yolov8_postproc.h"
#include "scrfd_postproc.h"

/*-----------------------------------------------------------------------------*/
/* Configuration                                                               */
/*-----------------------------------------------------------------------------*/
#define USE_DDR                     DDR_ID0

// YOLOv8 default input dimensions
#define INPUT_WIDTH                 640
#define INPUT_HEIGHT                640
#define INPUT_CHANNELS              3

// Memory size calculations
// NPU preprocessor expects NV12 (YUV420) input - it handles YUV->RGB + 1/255 normalize in hardware
#define AI_NV12_BUFSIZE(w, h)       (ALIGN_CEIL_4(w) * (h) * 3 / 2)
#define AI_RGB_BUFSIZE(w, h)        (ALIGN_CEIL_4((w) * HD_VIDEO_PXLFMT_BPP(HD_VIDEO_PXLFMT_RGB888_PLANAR) / 8) * (h))

/*-----------------------------------------------------------------------------*/
/* Type Definitions                                                            */
/*-----------------------------------------------------------------------------*/
typedef enum {
    MODEL_TYPE_YOLOV8 = 0,
    MODEL_TYPE_SCRFD,
} MODEL_TYPE;

typedef struct _MEM_PARM {
    UINTPTR pa;
    UINTPTR va;
    UINT32 size;
    UINTPTR blk;
} MEM_PARM;

typedef struct _AI_CONTEXT {
    // Model
    char model_path[256];
    char input_path[256];
    char labels_path[256];
    MODEL_TYPE model_type;
    
    // Memory
    MEM_PARM model_mem;
    MEM_PARM input_mem;
    MEM_PARM work_mem;
    MEM_PARM ronly_mem;
    MEM_PARM *out_mem;
    
    // AI3 state
    UINT32 proc_id;
    VENDOR_AI3_NET_INFO net_info;
    
    // Labels (for YOLOv8)
    char labels[YOLOV8_NUM_CLASSES * YOLOV8_LABEL_LEN];
    int labels_loaded;
    
    // Results
    YOLOV8_RESULTS yolo_results;
    SCRFD_RESULTS scrfd_results;
} AI_CONTEXT;

static AI_CONTEXT g_ctx = {0};
static volatile int g_running = 1;

/*-----------------------------------------------------------------------------*/
/* Signal Handler                                                              */
/*-----------------------------------------------------------------------------*/
static void signal_handler(int sig)
{
    printf("\nReceived signal %d, exiting...\n", sig);
    g_running = 0;
}

/*-----------------------------------------------------------------------------*/
/* Memory Functions                                                            */
/*-----------------------------------------------------------------------------*/
static HD_RESULT mem_alloc(MEM_PARM *mem, CHAR *name, UINT32 size)
{
    HD_RESULT ret;
    UINTPTR pa = 0;
    void *va = NULL;
    
    ret = hd_common_mem_alloc(name, &pa, &va, size, USE_DDR);
    if (ret != HD_OK) {
        printf("[MEM] Failed to allocate %s (%u bytes): %d\n", name, size, ret);
        return ret;
    }
    
    mem->pa = pa;
    mem->va = (UINTPTR)va;
    mem->size = size;
    mem->blk = (UINTPTR)-1;
    
    printf("[MEM] Allocated %s: pa=0x%lx va=0x%lx size=%u\n", name, pa, (UINTPTR)va, size);
    return HD_OK;
}

static HD_RESULT mem_free(MEM_PARM *mem)
{
    if (mem->pa && mem->va) {
        hd_common_mem_free(mem->pa, (void *)mem->va);
        mem->pa = 0;
        mem->va = 0;
        mem->size = 0;
    }
    return HD_OK;
}

static INT32 load_file(const char *filename, UINTPTR va, UINT32 max_size)
{
    FILE *fd = fopen(filename, "rb");
    if (!fd) {
        printf("[FILE] Cannot open: %s\n", filename);
        return -1;
    }
    
    fseek(fd, 0, SEEK_END);
    INT32 size = ftell(fd);
    fseek(fd, 0, SEEK_SET);
    
    if (size > (INT32)max_size) {
        printf("[FILE] File too large: %d > %u\n", size, max_size);
        fclose(fd);
        return -1;
    }
    
    INT32 read_size = fread((void *)va, 1, size, fd);
    fclose(fd);
    
    if (read_size != size) {
        printf("[FILE] Read error: %d != %d\n", read_size, size);
        return -1;
    }
    
    // Flush cache
    hd_common_mem_flush_cache((void *)va, size);
    
    printf("[FILE] Loaded %s (%d bytes)\n", filename, size);
    return size;
}

static INT32 get_file_size(const char *filename)
{
    FILE *fd = fopen(filename, "rb");
    if (!fd) return -1;
    
    fseek(fd, 0, SEEK_END);
    INT32 size = ftell(fd);
    fclose(fd);
    
    return size;
}

/*-----------------------------------------------------------------------------*/
/* AI3 Functions                                                               */
/*-----------------------------------------------------------------------------*/
static HD_RESULT ai3_init(void)
{
    HD_RESULT ret;
    VENDOR_AI3_DEV_CFG dev_cfg = {0};
    
    ret = vendor_ai3_dev_init(&dev_cfg);
    if (ret != HD_OK) {
        printf("[AI3] vendor_ai3_dev_init failed: %d\n", ret);
        return ret;
    }
    
    // Print version
    VENDOR_AI3_VER ver = {0};
    ret = vendor_ai3_dev_get(VENDOR_AI3_CFG_VER, &ver);
    if (ret == HD_OK) {
        printf("[AI3] vendor_ai version: %s\n", ver.vendor_ai_impl_version);
        printf("[AI3] kflow_ai version: %s\n", ver.kflow_ai_impl_version);
    }
    
    return HD_OK;
}

static HD_RESULT ai3_uninit(void)
{
    return vendor_ai3_dev_uninit();
}

static HD_RESULT ai3_load_model(AI_CONTEXT *ctx)
{
    HD_RESULT ret;
    
    // Get model file size
    INT32 model_size = get_file_size(ctx->model_path);
    if (model_size <= 0) {
        printf("[AI3] Cannot get model size: %s\n", ctx->model_path);
        return HD_ERR_NG;
    }
    
    // Allocate and load model
    ret = mem_alloc(&ctx->model_mem, "model.bin", model_size);
    if (ret != HD_OK) return ret;
    
    if (load_file(ctx->model_path, ctx->model_mem.va, model_size) <= 0) {
        return HD_ERR_NG;
    }
    
    // Query model memory requirements
    VENDOR_AI3_MODEL_INFO model_info = {0};
    model_info.model_buf.pa = ctx->model_mem.pa;
    model_info.model_buf.va = ctx->model_mem.va;
    model_info.model_buf.size = ctx->model_mem.size;
    
    ret = vendor_ai3_dev_get(VENDOR_AI3_CFG_MODEL_INFO, &model_info);
    if (ret != HD_OK) {
        printf("[AI3] WARNING: vendor_ai3_dev_get(MODEL_INFO) failed: %d (continuing anyway)\n", ret);
        // Use default sizes if query fails
        model_info.proc_mem.buf[AI3_PROC_BUF_WORKBUF].size = 16 * 1024 * 1024;  // 16MB
        model_info.proc_mem.buf[AI3_PROC_BUF_RONLYBUF].size = 8 * 1024 * 1024;  // 8MB
    }
    
    printf("[AI3] Model requires: workbuf=%u, ronlybuf=%u\n",
           model_info.proc_mem.buf[AI3_PROC_BUF_WORKBUF].size,
           model_info.proc_mem.buf[AI3_PROC_BUF_RONLYBUF].size);
    
    // Allocate work buffers
    ret = mem_alloc(&ctx->work_mem, "ai_workbuf", 
                    model_info.proc_mem.buf[AI3_PROC_BUF_WORKBUF].size);
    if (ret != HD_OK) return ret;
    
    ret = mem_alloc(&ctx->ronly_mem, "ai_ronlybuf",
                    model_info.proc_mem.buf[AI3_PROC_BUF_RONLYBUF].size);
    if (ret != HD_OK) return ret;
    
    // Open network
    VENDOR_AI3_PROC_CFG proc_cfg = {0};
    proc_cfg.model_buf.pa = ctx->model_mem.pa;
    proc_cfg.model_buf.va = ctx->model_mem.va;
    proc_cfg.model_buf.size = ctx->model_mem.size;
    
    proc_cfg.proc_mem.buf[AI3_PROC_BUF_WORKBUF].pa = ctx->work_mem.pa;
    proc_cfg.proc_mem.buf[AI3_PROC_BUF_WORKBUF].va = ctx->work_mem.va;
    proc_cfg.proc_mem.buf[AI3_PROC_BUF_WORKBUF].size = ctx->work_mem.size;
    
    proc_cfg.proc_mem.buf[AI3_PROC_BUF_RONLYBUF].pa = ctx->ronly_mem.pa;
    proc_cfg.proc_mem.buf[AI3_PROC_BUF_RONLYBUF].va = ctx->ronly_mem.va;
    proc_cfg.proc_mem.buf[AI3_PROC_BUF_RONLYBUF].size = ctx->ronly_mem.size;
    
    proc_cfg.plugin[AI3_PLUGIN_CPU] = vendor_ai_cpu1_get_engine();
    
    ret = vendor_ai3_net_open(&ctx->proc_id, &proc_cfg, &ctx->net_info);
    if (ret != HD_OK) {
        printf("[AI3] vendor_ai3_net_open failed: %d\n", ret);
        return ret;
    }
    
    printf("[AI3] Network opened: proc_id=%u, in_cnt=%u, out_cnt=%u\n",
           ctx->proc_id, ctx->net_info.in_buf_cnt, ctx->net_info.out_buf_cnt);
    
    // Start network FIRST (required before set() calls)
    ret = vendor_ai3_net_start(ctx->proc_id);
    if (ret != HD_OK) {
        printf("[AI3] vendor_ai3_net_start failed: %d\n", ret);
        return ret;
    }
    printf("[AI3] Network started\n");
    
    // Allocate output buffers (after start)
    ctx->out_mem = (MEM_PARM *)malloc(sizeof(MEM_PARM) * ctx->net_info.out_buf_cnt);
    if (!ctx->out_mem) return HD_ERR_NOMEM;
    memset(ctx->out_mem, 0, sizeof(MEM_PARM) * ctx->net_info.out_buf_cnt);
    
    for (UINT32 i = 0; i < ctx->net_info.out_buf_cnt; i++) {
        VENDOR_AI3_BUF ai_buf = {0};
        ret = vendor_ai3_net_get(ctx->proc_id, ctx->net_info.out_path_list[i], &ai_buf);
        if (ret != HD_OK) {
            printf("[AI3] Failed to get output buffer %u info\n", i);
            return ret;
        }
        
        char name[32];
        snprintf(name, sizeof(name), "ai_out_%u", i);
        ret = mem_alloc(&ctx->out_mem[i], name, ai_buf.size);
        if (ret != HD_OK) return ret;
        
        // Set output buffer (after start)
        ai_buf.pa = ctx->out_mem[i].pa;
        ai_buf.va = ctx->out_mem[i].va;
        ai_buf.size = ctx->out_mem[i].size;
        
        ret = vendor_ai3_net_set(ctx->proc_id, ctx->net_info.out_path_list[i], &ai_buf);
        if (ret != HD_OK) {
            printf("[AI3] Failed to set output buffer %u\n", i);
            return ret;
        }
        
        printf("[AI3] Output %u: %ux%ux%u, fmt=0x%x, size=%u\n",
               i, ai_buf.width, ai_buf.height, ai_buf.channel, ai_buf.fmt, ai_buf.size);
    }
    
    return HD_OK;
}

static HD_RESULT ai3_unload_model(AI_CONTEXT *ctx)
{
    if (ctx->proc_id > 0) {
        vendor_ai3_net_stop(ctx->proc_id);
        vendor_ai3_net_close(ctx->proc_id);
        ctx->proc_id = 0;
    }
    
    if (ctx->out_mem) {
        for (UINT32 i = 0; i < ctx->net_info.out_buf_cnt; i++) {
            mem_free(&ctx->out_mem[i]);
        }
        free(ctx->out_mem);
        ctx->out_mem = NULL;
    }
    
    mem_free(&ctx->ronly_mem);
    mem_free(&ctx->work_mem);
    mem_free(&ctx->model_mem);
    
    return HD_OK;
}

static HD_RESULT ai3_run_inference(AI_CONTEXT *ctx)
{
    HD_RESULT ret;
    
    // Allocate and load input
    // NPU preprocessor expects NV12 (YUV420) - it does YUV->RGB + resize + 1/255 normalize in hardware
    UINT32 input_size = AI_NV12_BUFSIZE(INPUT_WIDTH, INPUT_HEIGHT);
    ret = mem_alloc(&ctx->input_mem, "ai_input", input_size);
    if (ret != HD_OK) return ret;
    
    INT32 loaded = load_file(ctx->input_path, ctx->input_mem.va, input_size);
    if (loaded <= 0) {
        printf("[AI3] Failed to load input file (expected NV12 %ux%u = %u bytes)\n",
               INPUT_WIDTH, INPUT_HEIGHT, input_size);
        mem_free(&ctx->input_mem);
        return HD_ERR_NG;
    }
    printf("[AI3] Loaded NV12 input: %d bytes (expected %u for %ux%u)\n",
           loaded, input_size, INPUT_WIDTH, INPUT_HEIGHT);
    
    // Set input buffer
    VENDOR_AI3_BUF in_buf = {0};
    ret = vendor_ai3_net_get(ctx->proc_id, ctx->net_info.in_path_list[0], &in_buf);
    if (ret != HD_OK) {
        printf("[AI3] Failed to get input buffer info\n");
        mem_free(&ctx->input_mem);
        return ret;
    }
    
    // Debug: Print expected input format from model
    printf("[AI3] Model expects input: %ux%ux%u, fmt=0x%x, size=%u, line_ofs=%u\n",
           in_buf.width, in_buf.height, in_buf.channel, in_buf.fmt, in_buf.size, in_buf.line_ofs);
    
    in_buf.pa = ctx->input_mem.pa;
    in_buf.va = ctx->input_mem.va;
    in_buf.size = loaded;
    in_buf.width = INPUT_WIDTH;
    in_buf.height = INPUT_HEIGHT;
    in_buf.line_ofs = INPUT_WIDTH;
    
    ret = vendor_ai3_net_set(ctx->proc_id, ctx->net_info.in_path_list[0], &in_buf);
    if (ret != HD_OK) {
        printf("[AI3] Failed to set input buffer\n");
        mem_free(&ctx->input_mem);
        return ret;
    }
    
    printf("[AI3] Running inference...\n");
    
    // Run inference
    ret = vendor_ai3_net_proc(ctx->proc_id);
    if (ret != HD_OK) {
        printf("[AI3] vendor_ai3_net_proc failed: %d\n", ret);
        mem_free(&ctx->input_mem);
        return ret;
    }
    
    printf("[AI3] Inference complete\n");
    
    // Get all output buffers and flush cache
    for (UINT32 i = 0; i < ctx->net_info.out_buf_cnt; i++) {
        VENDOR_AI3_BUF tmp = {0};
        vendor_ai3_net_get(ctx->proc_id, ctx->net_info.out_path_list[i], &tmp);
        hd_common_mem_flush_cache((void *)tmp.va, tmp.size);
    }
    
    if (ctx->model_type == MODEL_TYPE_YOLOV8) {
        // --- YOLOv8 post-processing ---
        VENDOR_AI3_BUF out_buf = {0};
        ret = vendor_ai3_net_get(ctx->proc_id, ctx->net_info.out_path_list[0], &out_buf);
        if (ret != HD_OK) {
            printf("[AI3] Failed to get output buffer\n");
            mem_free(&ctx->input_mem);
            return ret;
        }
        
        printf("[AI3] Output: %ux%ux%u, fmt=0x%x, scale=%f, zero=%d\n",
               out_buf.width, out_buf.height, out_buf.channel,
               out_buf.fmt, out_buf.scale_ratio, out_buf.zero_point);
        
        YOLOV8_POSTPROC_PARM postproc = {0};
        postproc.out_addr = out_buf.va;
        postproc.out_fmt = out_buf.fmt;
        postproc.scale_ratio = out_buf.scale_ratio;
        postproc.zero_point = out_buf.zero_point;
        postproc.cfg.conf_threshold = 0.25f;
        postproc.cfg.nms_threshold = 0.45f;
        postproc.cfg.num_classes = YOLOV8_NUM_CLASSES;
        postproc.cfg.input_width = INPUT_WIDTH;
        postproc.cfg.input_height = INPUT_HEIGHT;
        postproc.results = &ctx->yolo_results;
        
        ret = yolov8_postproc_process(&postproc);
        if (ret != HD_OK) {
            printf("[POSTPROC] YOLOv8 post-processing failed: %d\n", ret);
        } else {
            yolov8_print_results(&ctx->yolo_results, ctx->labels_loaded ? ctx->labels : NULL);
        }
    } else if (ctx->model_type == MODEL_TYPE_SCRFD) {
        // --- SCRFD post-processing ---
        SCRFD_POSTPROC_PARM postproc = {0};
        postproc.num_outputs = ctx->net_info.out_buf_cnt;
        postproc.cfg.conf_threshold = 0.3f;  /* Lower threshold for NPU quantization */
        postproc.cfg.nms_threshold = 0.4f;
        postproc.cfg.input_width = INPUT_WIDTH;
        postproc.cfg.input_height = INPUT_HEIGHT;
        postproc.results = &ctx->scrfd_results;
        
        for (UINT32 i = 0; i < ctx->net_info.out_buf_cnt && i < 9; i++) {
            VENDOR_AI3_BUF out_buf = {0};
            ret = vendor_ai3_net_get(ctx->proc_id, ctx->net_info.out_path_list[i], &out_buf);
            if (ret != HD_OK) {
                printf("[AI3] Failed to get output %u\n", i);
                continue;
            }
            postproc.outputs[i].va = out_buf.va;
            postproc.outputs[i].fmt = out_buf.fmt;
            postproc.outputs[i].scale_ratio = out_buf.scale_ratio;
            postproc.outputs[i].zero_point = out_buf.zero_point;
            postproc.outputs[i].width = out_buf.width;
            postproc.outputs[i].height = out_buf.height;
            postproc.outputs[i].channel = out_buf.channel;
            postproc.outputs[i].size = out_buf.size;
        }
        
        ret = scrfd_postproc_process(&postproc);
        if (ret != HD_OK) {
            printf("[POSTPROC] SCRFD post-processing failed: %d\n", ret);
        } else {
            scrfd_print_results(&ctx->scrfd_results);
        }
    }
    
    mem_free(&ctx->input_mem);
    return HD_OK;
}

/*-----------------------------------------------------------------------------*/
/* Main                                                                        */
/*-----------------------------------------------------------------------------*/
static void print_usage(const char *prog)
{
    printf("Usage: %s [--scrfd] <model.bin> <input.bin> [labels.txt]\n", prog);
    printf("\n");
    printf("Arguments:\n");
    printf("  --scrfd     - Use SCRFD face detection post-processing\n");
    printf("  model.bin   - Compiled model file (.bin)\n");
    printf("  input.bin   - Input image (640x640 NV12/YUV420, 614400 bytes)\n");
    printf("  labels.txt  - Optional: Class labels file (YOLOv8 only)\n");
    printf("\n");
    printf("NOTE: The NPU preprocessor expects NV12 input and handles\n");
    printf("      YUV->RGB conversion + 1/255 normalization in hardware.\n");
    printf("\n");
    printf("Examples:\n");
    printf("  %s yolov8n.bin bus_nv12.bin coco_labels.txt\n", prog);
    printf("  %s --scrfd scrfd_face.bin face_nv12.bin\n", prog);
}

int main(int argc, char **argv)
{
    HD_RESULT ret;
    HD_COMMON_MEM_INIT_CONFIG mem_cfg = {0};
    
    printf("\n");
    printf("============================================\n");
    printf("  AI3 Custom Model Sample\n");
    printf("  Novatek NT98538\n");
    printf("============================================\n\n");
    
    // Parse arguments
    int arg_offset = 1;
    g_ctx.model_type = MODEL_TYPE_YOLOV8;
    
    if (argc > 1 && strcmp(argv[1], "--scrfd") == 0) {
        g_ctx.model_type = MODEL_TYPE_SCRFD;
        arg_offset = 2;
    }
    
    if (argc - arg_offset < 2) {
        print_usage(argv[0]);
        return -1;
    }
    
    strncpy(g_ctx.model_path, argv[arg_offset], sizeof(g_ctx.model_path) - 1);
    strncpy(g_ctx.input_path, argv[arg_offset + 1], sizeof(g_ctx.input_path) - 1);
    
    if (argc - arg_offset > 2) {
        strncpy(g_ctx.labels_path, argv[arg_offset + 2], sizeof(g_ctx.labels_path) - 1);
    }
    
    printf("[MAIN] Mode:  %s\n", g_ctx.model_type == MODEL_TYPE_SCRFD ? "SCRFD Face Detection" : "YOLOv8 Object Detection");
    printf("[MAIN] Model: %s\n", g_ctx.model_path);
    printf("[MAIN] Input: %s\n", g_ctx.input_path);
    printf("[MAIN] Labels: %s\n", g_ctx.labels_path[0] ? g_ctx.labels_path : "(none)");
    
    // Setup signal handler
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    
    // Initialize HDAL
    ret = hd_common_init(0);
    if (ret != HD_OK) {
        printf("[MAIN] hd_common_init failed: %d\n", ret);
        return -1;
    }
    
    // Initialize memory
    ret = hd_common_mem_init(&mem_cfg);
    if (ret != HD_OK) {
        printf("[MAIN] hd_common_mem_init failed: %d\n", ret);
        goto exit_common;
    }
    
    // Initialize AI3
    ret = ai3_init();
    if (ret != HD_OK) {
        printf("[MAIN] ai3_init failed: %d\n", ret);
        goto exit_mem;
    }
    
    // Initialize post-processing
    if (g_ctx.model_type == MODEL_TYPE_YOLOV8) {
        ret = yolov8_postproc_init();
        if (ret != HD_OK) {
            printf("[MAIN] yolov8_postproc_init failed: %d\n", ret);
            goto exit_ai3;
        }
        if (g_ctx.labels_path[0]) {
            ret = yolov8_load_labels(g_ctx.labels_path, g_ctx.labels);
            g_ctx.labels_loaded = (ret == HD_OK);
        }
    } else {
        ret = scrfd_postproc_init();
        if (ret != HD_OK) {
            printf("[MAIN] scrfd_postproc_init failed: %d\n", ret);
            goto exit_ai3;
        }
    }
    
    // Load model
    ret = ai3_load_model(&g_ctx);
    if (ret != HD_OK) {
        printf("[MAIN] ai3_load_model failed: %d\n", ret);
        goto exit_postproc;
    }
    
    // Run inference
    ret = ai3_run_inference(&g_ctx);
    if (ret != HD_OK) {
        printf("[MAIN] ai3_run_inference failed: %d\n", ret);
    }
    
    // Interactive mode
    if (g_running) {
        printf("\nPress 'r' to run again, 'q' to quit\n");
        int key;
        while (g_running) {
            key = getchar();
            if (key == 'r' || key == 'R') {
                ret = ai3_run_inference(&g_ctx);
                if (ret != HD_OK) {
                    printf("[MAIN] Inference failed: %d\n", ret);
                }
            } else if (key == 'q' || key == 'Q' || key == 3) {
                break;
            }
        }
    }
    
    // Cleanup
    ai3_unload_model(&g_ctx);
    
exit_postproc:
    yolov8_postproc_uninit();
    scrfd_postproc_uninit();
    
exit_ai3:
    ai3_uninit();
    
exit_mem:
    hd_common_mem_uninit();
    
exit_common:
    hd_common_uninit();
    
    printf("[MAIN] Done\n");
    return (ret == HD_OK) ? 0 : -1;
}
