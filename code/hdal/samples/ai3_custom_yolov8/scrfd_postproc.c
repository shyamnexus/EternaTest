/*
 * SCRFD Face Detection Post-Processing Implementation
 *
 * SCRFD-500M multi-stride anchor-based face detector.
 * Outputs are in CHW layout from NPU (same as ONNX).
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "hd_type.h"
#include "vendor_ai.h"
#include "vendor_ai_cpu/vendor_ai_cpu.h"
#include "scrfd_postproc.h"

/********************************************************************
    MACROS / CONSTANTS
********************************************************************/
#define CLAMP(x, lo, hi)    ((x) < (lo) ? (lo) : ((x) > (hi) ? (hi) : (x)))

/* Stride configurations */
static const INT32 STRIDES[SCRFD_NUM_STRIDES] = {8, 16, 32};
static const INT32 GRID_SIZES[SCRFD_NUM_STRIDES] = {80, 40, 20};  /* 640/stride */
static const INT32 ANCHOR_COUNTS[SCRFD_NUM_STRIDES] = {12800, 3200, 800};

/********************************************************************
    STATIC DATA
********************************************************************/
static SCRFD_FACE *g_face_buf = NULL;   /* Pre-NMS face buffer */

/********************************************************************
    INTERNAL FUNCTIONS
********************************************************************/

static FLOAT calculate_iou(const SCRFD_FACE *a, const SCRFD_FACE *b)
{
    FLOAT x1 = a->x1 > b->x1 ? a->x1 : b->x1;
    FLOAT y1 = a->y1 > b->y1 ? a->y1 : b->y1;
    FLOAT x2 = a->x2 < b->x2 ? a->x2 : b->x2;
    FLOAT y2 = a->y2 < b->y2 ? a->y2 : b->y2;
    
    FLOAT inter_w = x2 - x1;
    FLOAT inter_h = y2 - y1;
    if (inter_w <= 0 || inter_h <= 0) return 0.0f;
    
    FLOAT inter = inter_w * inter_h;
    FLOAT area_a = (a->x2 - a->x1) * (a->y2 - a->y1);
    FLOAT area_b = (b->x2 - b->x1) * (b->y2 - b->y1);
    
    return inter / (area_a + area_b - inter);
}

static int apply_nms(SCRFD_FACE *faces, int count, FLOAT threshold,
                     SCRFD_FACE *out_faces, int max_out)
{
    if (count == 0) return 0;
    
    /* Sort by confidence (simple selection sort) */
    for (int i = 0; i < count - 1; i++) {
        int max_idx = i;
        for (int j = i + 1; j < count; j++) {
            if (faces[j].confidence > faces[max_idx].confidence)
                max_idx = j;
        }
        if (max_idx != i) {
            SCRFD_FACE tmp = faces[i];
            faces[i] = faces[max_idx];
            faces[max_idx] = tmp;
        }
    }
    
    int *suppressed = (int *)calloc(count, sizeof(int));
    int out_count = 0;
    
    for (int i = 0; i < count && out_count < max_out; i++) {
        if (suppressed[i]) continue;
        
        out_faces[out_count++] = faces[i];
        
        for (int j = i + 1; j < count; j++) {
            if (!suppressed[j] && calculate_iou(&faces[i], &faces[j]) > threshold) {
                suppressed[j] = 1;
            }
        }
    }
    
    free(suppressed);
    return out_count;
}

/*
 * Classify outputs by their dimensions to identify score/bbox/kps tensors
 * per stride level.
 *
 * For each stride level (8, 16, 32):
 *   score: height=num_anchors, width=1   (or channel=1)
 *   bbox:  height=num_anchors, width=4   (or channel=4)
 *   kps:   height=num_anchors, width=10  (or channel=10)
 */
typedef struct {
    int score_idx;   /* index into outputs[] */
    int bbox_idx;
    int kps_idx;
    int num_anchors;
    int stride;
    int grid_size;
} STRIDE_INFO;

static int classify_outputs(SCRFD_POSTPROC_PARM *p_parm, STRIDE_INFO strides[3])
{
    /* Initialize */
    for (int s = 0; s < 3; s++) {
        strides[s].score_idx = -1;
        strides[s].bbox_idx = -1;
        strides[s].kps_idx = -1;
        strides[s].num_anchors = ANCHOR_COUNTS[s];
        strides[s].stride = STRIDES[s];
        strides[s].grid_size = GRID_SIZES[s];
    }
    
    printf("[SCRFD] Classifying %d output tensors:\n", p_parm->num_outputs);
    for (int i = 0; i < p_parm->num_outputs; i++) {
        SCRFD_OUTPUT_INFO *o = &p_parm->outputs[i];
        printf("[SCRFD]   Output[%d]: w=%u h=%u ch=%u, fmt=0x%x, size=%u\n",
               i, o->width, o->height, o->channel, o->fmt, o->size);
        
        /*
         * NPU output dimensions for SCRFD:
         *   Score: w=1, h=1,  ch=12800  -> 1 feature (score), 12800 anchors
         *   Bbox:  w=1, h=4,  ch=12800  -> 4 features (dx,dy,dw,dh), 12800 anchors
         *   Kps:   w=1, h=10, ch=12800  -> 10 features (5 keypoints x2), 12800 anchors
         *
         * Anchor count is in the CHANNEL dimension.
         * Feature count is in the HEIGHT dimension.
         */
        int num_anchors = o->channel;
        int feat_count = o->height;
        
        /* Determine which stride level based on anchor count */
        int stride_idx = -1;
        for (int s = 0; s < 3; s++) {
            if (num_anchors == ANCHOR_COUNTS[s]) {
                stride_idx = s;
                break;
            }
        }
        
        if (stride_idx < 0) {
            printf("[SCRFD]   -> Unknown anchor count %d, skipping\n", num_anchors);
            continue;
        }
        
        /* Determine type based on feature count */
        if (feat_count == 1) {
            strides[stride_idx].score_idx = i;
            printf("[SCRFD]   -> score (stride %d, %d anchors)\n", STRIDES[stride_idx], num_anchors);
        } else if (feat_count == 4) {
            strides[stride_idx].bbox_idx = i;
            printf("[SCRFD]   -> bbox (stride %d, %d anchors)\n", STRIDES[stride_idx], num_anchors);
        } else if (feat_count == 10) {
            strides[stride_idx].kps_idx = i;
            printf("[SCRFD]   -> kps (stride %d, %d anchors)\n", STRIDES[stride_idx], num_anchors);
        } else {
            printf("[SCRFD]   -> unknown feat_count=%d\n", feat_count);
        }
    }
    
    /* Verify we found all required outputs */
    for (int s = 0; s < 3; s++) {
        if (strides[s].score_idx < 0 || strides[s].bbox_idx < 0) {
            printf("[SCRFD] ERROR: Missing score or bbox for stride %d\n", STRIDES[s]);
            return -1;
        }
    }
    
    return 0;
}

/*
 * Decode faces for one stride level.
 * 
 * Score tensor: CHW layout [1, num_anchors] -> score per anchor
 * Bbox tensor:  CHW layout [4, num_anchors] -> (dx, dy, dw, dh) per anchor
 * Kps tensor:   CHW layout [10, num_anchors] -> 5 keypoints (x,y) per anchor
 *
 * Anchor generation: 2 anchors per grid cell, both at cell center.
 * Bbox decode: x1 = (cx - dx*stride) / input_size, etc.
 */
static int decode_stride(SCRFD_POSTPROC_PARM *p_parm, STRIDE_INFO *si,
                         FLOAT conf_threshold, SCRFD_FACE *faces, int max_faces)
{
    SCRFD_OUTPUT_INFO *score_out = &p_parm->outputs[si->score_idx];
    SCRFD_OUTPUT_INFO *bbox_out = &p_parm->outputs[si->bbox_idx];
    SCRFD_OUTPUT_INFO *kps_out = (si->kps_idx >= 0) ? &p_parm->outputs[si->kps_idx] : NULL;
    
    int num_anchors = si->num_anchors;
    int grid = si->grid_size;
    int stride = si->stride;
    FLOAT input_w = (FLOAT)p_parm->cfg.input_width;
    FLOAT input_h = (FLOAT)p_parm->cfg.input_height;
    
    /* Convert score tensor fixed->float */
    FLOAT *score_float = (FLOAT *)malloc(num_anchors * sizeof(FLOAT));
    FLOAT *bbox_float = (FLOAT *)malloc(num_anchors * 4 * sizeof(FLOAT));
    FLOAT *kps_float = NULL;
    
    if (!score_float || !bbox_float) {
        if (score_float) free(score_float);
        if (bbox_float) free(bbox_float);
        return 0;
    }
    
    vendor_ai_cpu_util_fixed2float((void *)score_out->va, score_out->fmt,
                                    score_float, score_out->scale_ratio,
                                    num_anchors, score_out->zero_point);
    
    /* Debug: print score statistics for this stride */
    {
        FLOAT smin = score_float[0], smax = score_float[0];
        int n_above_50 = 0, n_above_30 = 0;
        for (int k = 0; k < num_anchors; k++) {
            if (score_float[k] < smin) smin = score_float[k];
            if (score_float[k] > smax) smax = score_float[k];
            if (score_float[k] > 0.5f) n_above_50++;
            if (score_float[k] > 0.3f) n_above_30++;
        }
        printf("[SCRFD] Stride %d scores: min=%.6f max=%.6f >30%%=%d >50%%=%d "
               "scale=%.6f zp=%d fmt=0x%x\n",
               stride, smin, smax, n_above_30, n_above_50,
               score_out->scale_ratio, score_out->zero_point, score_out->fmt);
        /* Print first few raw bytes for comparison */
        UINT8 *raw = (UINT8 *)score_out->va;
        printf("[SCRFD]   raw[0..7]: %u %u %u %u %u %u %u %u\n",
               raw[0], raw[1], raw[2], raw[3], raw[4], raw[5], raw[6], raw[7]);
    }
    
    vendor_ai_cpu_util_fixed2float((void *)bbox_out->va, bbox_out->fmt,
                                    bbox_float, bbox_out->scale_ratio,
                                    num_anchors * 4, bbox_out->zero_point);
    
    if (kps_out) {
        kps_float = (FLOAT *)malloc(num_anchors * 10 * sizeof(FLOAT));
        if (kps_float) {
            vendor_ai_cpu_util_fixed2float((void *)kps_out->va, kps_out->fmt,
                                            kps_float, kps_out->scale_ratio,
                                            num_anchors * 10, kps_out->zero_point);
        }
    }
    
    int face_count = 0;
    int debug_count = 0;
    
    /*
     * SCRFD output layout (from NPU):
     *   score: ch=num_anchors, h=1, w=1  -> data[anchor * 1 + 0] = data[anchor]
     *   bbox:  ch=num_anchors, h=4, w=1  -> data[anchor * 4 + feat]
     *   kps:   ch=num_anchors, h=10, w=1 -> data[anchor * 10 + feat]
     *
     * ONNX shape is [1, num_anchors, N] → CHW layout: [num_anchors][N]
     * This is DIFFERENT from YOLOv8 where shape was [1, N, num_anchors].
     */
    
    for (int anchor_idx = 0; anchor_idx < num_anchors && face_count < max_faces; anchor_idx++) {
        /*
         * SCRFD ONNX model already includes sigmoid in score output.
         * The output is a probability [0,1], NOT a logit. Do NOT apply sigmoid again.
         */
        FLOAT score = score_float[anchor_idx];
        
        if (score < conf_threshold) continue;
        
        /* Compute grid position */
        int cell_idx = anchor_idx / SCRFD_ANCHORS_PER_CELL;
        int gx = cell_idx % grid;
        int gy = cell_idx / grid;
        
        /* Anchor center in pixel coordinates */
        FLOAT cx = (gx + 0.5f) * stride;
        FLOAT cy = (gy + 0.5f) * stride;
        
        /* Decode bbox: distance-based (SCRFD uses distance format) */
        /* data[anchor * 4 + feat]: feat 0=left, 1=top, 2=right, 3=bottom */
        FLOAT dx = bbox_float[anchor_idx * 4 + 0] * stride;
        FLOAT dy = bbox_float[anchor_idx * 4 + 1] * stride;
        FLOAT dw = bbox_float[anchor_idx * 4 + 2] * stride;
        FLOAT dh = bbox_float[anchor_idx * 4 + 3] * stride;
        
        FLOAT x1 = (cx - dx) / input_w;
        FLOAT y1 = (cy - dy) / input_h;
        FLOAT x2 = (cx + dw) / input_w;
        FLOAT y2 = (cy + dh) / input_h;
        
        faces[face_count].x1 = CLAMP(x1, 0.0f, 1.0f);
        faces[face_count].y1 = CLAMP(y1, 0.0f, 1.0f);
        faces[face_count].x2 = CLAMP(x2, 0.0f, 1.0f);
        faces[face_count].y2 = CLAMP(y2, 0.0f, 1.0f);
        faces[face_count].confidence = score;
        
        /* Decode keypoints if available */
        /* data[anchor * 10 + feat]: feats 0,1=eye_l, 2,3=eye_r, 4,5=nose, 6,7=mouth_l, 8,9=mouth_r */
        if (kps_float) {
            for (int k = 0; k < 5; k++) {
                FLOAT kp_x = kps_float[anchor_idx * 10 + k * 2 + 0] * stride;
                FLOAT kp_y = kps_float[anchor_idx * 10 + k * 2 + 1] * stride;
                faces[face_count].landmarks[k * 2 + 0] = CLAMP((cx + kp_x) / input_w, 0.0f, 1.0f);
                faces[face_count].landmarks[k * 2 + 1] = CLAMP((cy + kp_y) / input_h, 0.0f, 1.0f);
            }
        } else {
            memset(faces[face_count].landmarks, 0, sizeof(faces[face_count].landmarks));
        }
        
        if (debug_count < 3) {
            printf("[SCRFD] stride=%d anchor=%d score=%.4f bbox=(%.3f,%.3f)-(%.3f,%.3f)\n",
                   stride, anchor_idx, score,
                   faces[face_count].x1, faces[face_count].y1,
                   faces[face_count].x2, faces[face_count].y2);
            debug_count++;
        }
        
        face_count++;
    }
    
    free(score_float);
    free(bbox_float);
    if (kps_float) free(kps_float);
    
    return face_count;
}

/********************************************************************
    PUBLIC FUNCTIONS
********************************************************************/

HD_RESULT scrfd_postproc_init(void)
{
    /* Allocate pre-NMS face buffer */
    g_face_buf = (SCRFD_FACE *)malloc(SCRFD_MAX_FACES * 4 * sizeof(SCRFD_FACE));
    if (!g_face_buf) {
        printf("[SCRFD] Failed to allocate face buffer\n");
        return HD_ERR_NOMEM;
    }
    
    printf("[SCRFD] Post-processing initialized\n");
    return HD_OK;
}

HD_RESULT scrfd_postproc_uninit(void)
{
    if (g_face_buf) {
        free(g_face_buf);
        g_face_buf = NULL;
    }
    return HD_OK;
}

HD_RESULT scrfd_postproc_process(SCRFD_POSTPROC_PARM *p_parm)
{
    if (!p_parm || !p_parm->results) return HD_ERR_PARAM;
    if (!g_face_buf) return HD_ERR_NG;
    
    FLOAT conf_threshold = p_parm->cfg.conf_threshold > 0.0f ?
                           p_parm->cfg.conf_threshold : 0.25f;  /* Lower default for NPU quantization */
    FLOAT nms_threshold = p_parm->cfg.nms_threshold > 0.0f ?
                          p_parm->cfg.nms_threshold : 0.4f;
    
    /* Classify outputs into stride levels */
    STRIDE_INFO strides[3];
    if (classify_outputs(p_parm, strides) != 0) {
        printf("[SCRFD] Failed to classify outputs\n");
        return HD_ERR_NG;
    }
    
    /* Decode faces from each stride level */
    int total_faces = 0;
    int max_pre_nms = SCRFD_MAX_FACES * 4;
    
    for (int s = 0; s < SCRFD_NUM_STRIDES; s++) {
        int count = decode_stride(p_parm, &strides[s], conf_threshold,
                                  g_face_buf + total_faces,
                                  max_pre_nms - total_faces);
        printf("[SCRFD] Stride %d: %d faces above threshold\n", STRIDES[s], count);
        total_faces += count;
    }
    
    printf("[SCRFD] Pre-NMS total: %d faces\n", total_faces);
    
    /* Apply NMS */
    p_parm->results->count = apply_nms(g_face_buf, total_faces, nms_threshold,
                                        p_parm->results->faces, SCRFD_MAX_FACES);
    
    printf("[SCRFD] Post-NMS: %d faces\n", p_parm->results->count);
    
    return HD_OK;
}

void scrfd_print_results(const SCRFD_RESULTS *results)
{
    printf("\n========== SCRFD Face Detection Results ==========\n");
    printf("Total faces: %d\n", results->count);
    printf("--------------------------------------------------\n");
    
    for (int i = 0; i < results->count; i++) {
        const SCRFD_FACE *f = &results->faces[i];
        printf("%2d. [face] conf=%.2f%% bbox=(%.3f, %.3f, %.3f, %.3f)\n",
               i + 1, f->confidence * 100.0f,
               f->x1, f->y1, f->x2, f->y2);
        printf("    landmarks: L_eye=(%.3f,%.3f) R_eye=(%.3f,%.3f) nose=(%.3f,%.3f)\n",
               f->landmarks[0], f->landmarks[1],
               f->landmarks[2], f->landmarks[3],
               f->landmarks[4], f->landmarks[5]);
        printf("               L_mouth=(%.3f,%.3f) R_mouth=(%.3f,%.3f)\n",
               f->landmarks[6], f->landmarks[7],
               f->landmarks[8], f->landmarks[9]);
    }
    printf("==================================================\n");
}
