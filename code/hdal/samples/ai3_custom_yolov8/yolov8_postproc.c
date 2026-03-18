/**
 * @file yolov8_postproc.c
 * @brief YOLOv8 post-processing implementation for Novatek AI3
 *
 * Implements YOLOv8 output decoding, NMS, and result formatting.
 * 
 * YOLOv8 output format: [1, 84, 8400]
 * - 84 = 4 (cx, cy, w, h) + 80 (class scores)
 * - 8400 = sum of all anchor points across feature maps
 *
 * Copyright 2026. All rights reserved.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "yolov8_postproc.h"
#include "vendor_ai_cpu/vendor_ai_cpu.h"

/********************************************************************
    LOCAL DEFINITIONS
********************************************************************/
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define CLAMP(x, lo, hi) (MIN(MAX(x, lo), hi))

// Working buffer for intermediate results
static FLOAT *g_float_buf = NULL;
static YOLOV8_DETECTION *g_det_buf = NULL;
static int *g_indices = NULL;

/********************************************************************
    LOCAL FUNCTIONS
********************************************************************/

/**
 * @brief Calculate IoU between two boxes
 */
static FLOAT calculate_iou(const YOLOV8_DETECTION *a, const YOLOV8_DETECTION *b)
{
    FLOAT x1 = MAX(a->x1, b->x1);
    FLOAT y1 = MAX(a->y1, b->y1);
    FLOAT x2 = MIN(a->x2, b->x2);
    FLOAT y2 = MIN(a->y2, b->y2);
    
    if (x2 <= x1 || y2 <= y1) {
        return 0.0f;
    }
    
    FLOAT intersection = (x2 - x1) * (y2 - y1);
    FLOAT area_a = (a->x2 - a->x1) * (a->y2 - a->y1);
    FLOAT area_b = (b->x2 - b->x1) * (b->y2 - b->y1);
    FLOAT union_area = area_a + area_b - intersection;
    
    return (union_area > 0.0f) ? (intersection / union_area) : 0.0f;
}

/**
 * @brief Quick sort partition for NMS
 */
static int partition_by_confidence(YOLOV8_DETECTION *arr, int *indices, int low, int high)
{
    FLOAT pivot = arr[indices[high]].confidence;
    int i = low - 1;
    int temp;
    
    for (int j = low; j < high; j++) {
        if (arr[indices[j]].confidence > pivot) {
            i++;
            temp = indices[i];
            indices[i] = indices[j];
            indices[j] = temp;
        }
    }
    temp = indices[i + 1];
    indices[i + 1] = indices[high];
    indices[high] = temp;
    
    return i + 1;
}

/**
 * @brief Quick sort by confidence (descending)
 */
static void quicksort_by_confidence(YOLOV8_DETECTION *arr, int *indices, int low, int high)
{
    if (low < high) {
        int pi = partition_by_confidence(arr, indices, low, high);
        quicksort_by_confidence(arr, indices, low, pi - 1);
        quicksort_by_confidence(arr, indices, pi + 1, high);
    }
}

/**
 * @brief Apply Non-Maximum Suppression
 */
static int apply_nms(YOLOV8_DETECTION *detections, int count, FLOAT nms_threshold,
                     YOLOV8_DETECTION *output, int max_output)
{
    if (count == 0) return 0;
    
    // Initialize indices
    for (int i = 0; i < count; i++) {
        g_indices[i] = i;
    }
    
    // Sort by confidence
    quicksort_by_confidence(detections, g_indices, 0, count - 1);
    
    // Mark suppressed detections
    int *suppressed = (int *)calloc(count, sizeof(int));
    if (!suppressed) return 0;
    
    int output_count = 0;
    
    for (int i = 0; i < count && output_count < max_output; i++) {
        int idx = g_indices[i];
        if (suppressed[idx]) continue;
        
        // Keep this detection
        memcpy(&output[output_count], &detections[idx], sizeof(YOLOV8_DETECTION));
        output_count++;
        
        // Suppress overlapping detections of same class
        for (int j = i + 1; j < count; j++) {
            int jdx = g_indices[j];
            if (suppressed[jdx]) continue;
            if (detections[jdx].class_id != detections[idx].class_id) continue;
            
            FLOAT iou = calculate_iou(&detections[idx], &detections[jdx]);
            if (iou > nms_threshold) {
                suppressed[jdx] = 1;
            }
        }
    }
    
    free(suppressed);
    return output_count;
}

/**
 * @brief Decode YOLOv8 output tensor
 * 
 * YOLOv8 output format: [1, 84, 8400] in CHW layout
 * Memory layout: 84 channels, each containing 8400 anchor values
 *   Channel 0: cx for all 8400 anchors
 *   Channel 1: cy for all 8400 anchors
 *   Channel 2: w for all 8400 anchors
 *   Channel 3: h for all 8400 anchors
 *   Channel 4..83: class scores for all 8400 anchors
 *
 * Access pattern: output[channel * num_anchors + anchor_idx]
 */
static int decode_output(const FLOAT *output, int num_anchors, int num_classes,
                         FLOAT conf_threshold, YOLOV8_DETECTION *detections)
{
    int det_count = 0;
    int debug_printed = 0;
    
    // YOLOv8 input size (used to normalize bbox coordinates)
    const FLOAT input_w = 640.0f;
    const FLOAT input_h = 640.0f;
    
    // CHW layout: output[channel * num_anchors + anchor]
    const FLOAT *cx_data  = output + 0 * num_anchors;
    const FLOAT *cy_data  = output + 1 * num_anchors;
    const FLOAT *w_data   = output + 2 * num_anchors;
    const FLOAT *h_data   = output + 3 * num_anchors;
    const FLOAT *cls_data = output + 4 * num_anchors;  // 80 channels of class scores
    
    // Debug: print a few anchor raw values (CHW layout)
    for (int d = 0; d < 3; d++) {
        int idx = d * (num_anchors / 3);  // sample from beginning, middle, end
        printf("[YOLOV8] Anchor[%d] raw(CHW): cx=%.2f cy=%.2f w=%.2f h=%.2f cls0=%.4f cls1=%.4f\n",
               idx,
               cx_data[idx], cy_data[idx], w_data[idx], h_data[idx],
               cls_data[0 * num_anchors + idx],
               cls_data[1 * num_anchors + idx]);
    }
    
    for (int i = 0; i < num_anchors && det_count < YOLOV8_MAX_DETECTIONS * 4; i++) {
        // Find best class (CHW: class c score for anchor i = cls_data[c * num_anchors + i])
        int best_class = 0;
        FLOAT best_score = 0.0f;
        
        for (int c = 0; c < num_classes; c++) {
            FLOAT score = cls_data[c * num_anchors + i];
            if (score > best_score) {
                best_score = score;
                best_class = c;
            }
        }
        
        // Skip if below threshold
        if (best_score < conf_threshold) continue;
        
        // Decode bounding box (cx, cy, w, h) -> (x1, y1, x2, y2)
        // YOLOv8 outputs are in pixel coordinates (0-640)
        FLOAT cx = cx_data[i];
        FLOAT cy = cy_data[i];
        FLOAT w  = w_data[i];
        FLOAT h  = h_data[i];
        
        // Normalize to 0-1 by dividing by input size
        FLOAT x1 = (cx - w / 2.0f) / input_w;
        FLOAT y1 = (cy - h / 2.0f) / input_h;
        FLOAT x2 = (cx + w / 2.0f) / input_w;
        FLOAT y2 = (cy + h / 2.0f) / input_h;
        
        detections[det_count].x1 = CLAMP(x1, 0.0f, 1.0f);
        detections[det_count].y1 = CLAMP(y1, 0.0f, 1.0f);
        detections[det_count].x2 = CLAMP(x2, 0.0f, 1.0f);
        detections[det_count].y2 = CLAMP(y2, 0.0f, 1.0f);
        detections[det_count].confidence = best_score;
        detections[det_count].class_id = best_class;
        
        // Debug: print first 5 detections
        if (debug_printed < 5) {
            printf("[YOLOV8] Det[%d] anchor=%d cls=%d conf=%.4f cx=%.1f cy=%.1f w=%.1f h=%.1f -> (%.3f,%.3f)-(%.3f,%.3f)\n",
                   det_count, i, best_class, best_score,
                   cx, cy, w, h,
                   detections[det_count].x1, detections[det_count].y1,
                   detections[det_count].x2, detections[det_count].y2);
            debug_printed++;
        }
        
        det_count++;
    }
    
    return det_count;
}

/********************************************************************
    PUBLIC FUNCTIONS
********************************************************************/

HD_RESULT yolov8_postproc_init(void)
{
    // Allocate working buffers
    size_t float_buf_size = YOLOV8_NUM_ANCHORS * (YOLOV8_BOX_PARAMS + YOLOV8_NUM_CLASSES) * sizeof(FLOAT);
    g_float_buf = (FLOAT *)malloc(float_buf_size);
    if (!g_float_buf) {
        printf("[YOLOV8] Failed to allocate float buffer\n");
        return HD_ERR_NOMEM;
    }
    
    g_det_buf = (YOLOV8_DETECTION *)malloc(YOLOV8_MAX_DETECTIONS * 4 * sizeof(YOLOV8_DETECTION));
    if (!g_det_buf) {
        printf("[YOLOV8] Failed to allocate detection buffer\n");
        free(g_float_buf);
        g_float_buf = NULL;
        return HD_ERR_NOMEM;
    }
    
    g_indices = (int *)malloc(YOLOV8_MAX_DETECTIONS * 4 * sizeof(int));
    if (!g_indices) {
        printf("[YOLOV8] Failed to allocate indices buffer\n");
        free(g_float_buf);
        free(g_det_buf);
        g_float_buf = NULL;
        g_det_buf = NULL;
        return HD_ERR_NOMEM;
    }
    
    printf("[YOLOV8] Post-processing initialized\n");
    return HD_OK;
}

HD_RESULT yolov8_postproc_uninit(void)
{
    if (g_float_buf) {
        free(g_float_buf);
        g_float_buf = NULL;
    }
    if (g_det_buf) {
        free(g_det_buf);
        g_det_buf = NULL;
    }
    if (g_indices) {
        free(g_indices);
        g_indices = NULL;
    }
    
    printf("[YOLOV8] Post-processing uninitialized\n");
    return HD_OK;
}

HD_RESULT yolov8_postproc_process(YOLOV8_POSTPROC_PARM *p_parm)
{
    if (!p_parm || !p_parm->results) {
        return HD_ERR_PARAM;
    }
    
    if (!g_float_buf || !g_det_buf) {
        printf("[YOLOV8] Not initialized\n");
        return HD_ERR_NG;
    }
    
    // Set default config if not specified
    FLOAT conf_threshold = p_parm->cfg.conf_threshold > 0.0f ? 
                           p_parm->cfg.conf_threshold : YOLOV8_DEFAULT_CONF_THRESHOLD;
    FLOAT nms_threshold = p_parm->cfg.nms_threshold > 0.0f ?
                          p_parm->cfg.nms_threshold : YOLOV8_DEFAULT_NMS_THRESHOLD;
    INT32 num_classes = p_parm->cfg.num_classes > 0 ?
                        p_parm->cfg.num_classes : YOLOV8_NUM_CLASSES;
    INT32 num_anchors = YOLOV8_NUM_ANCHORS;
    INT32 total_channels = YOLOV8_BOX_PARAMS + num_classes;
    
    // Convert fixed-point to float
    // Output tensor: [1, 8400, 84] = 8400 rows x 84 columns
    int total_elements = num_anchors * total_channels;
    
    printf("[YOLOV8] Converting %d elements (fmt=0x%x, scale=%f, zp=%d)\n",
           total_elements, p_parm->out_fmt, p_parm->scale_ratio, p_parm->zero_point);
    
    HD_RESULT ret = vendor_ai_cpu_util_fixed2float(
        (void *)p_parm->out_addr,
        p_parm->out_fmt,
        g_float_buf,
        p_parm->scale_ratio,
        total_elements,
        p_parm->zero_point
    );
    
    if (ret != HD_OK) {
        printf("[YOLOV8] fixed2float conversion failed: %d\n", ret);
        return ret;
    }
    
    // Debug: print first few raw values to verify conversion
    printf("[YOLOV8] Raw float values [0..7]: ");
    for (int i = 0; i < 8 && i < total_elements; i++) {
        printf("%.4f ", g_float_buf[i]);
    }
    printf("\n");
    
    // Decode output tensor: [8400, 84]
    // Each row: [cx, cy, w, h, cls0, cls1, ..., cls79]
    // Coordinates are in pixel space (0-640), need to normalize to 0-1
    int pre_nms_count = decode_output(g_float_buf, num_anchors, num_classes,
                                       conf_threshold, g_det_buf);
    
    printf("[YOLOV8] Pre-NMS detections: %d\n", pre_nms_count);
    
    // Apply NMS
    p_parm->results->count = apply_nms(g_det_buf, pre_nms_count, nms_threshold,
                                        p_parm->results->detections, YOLOV8_MAX_DETECTIONS);
    
    printf("[YOLOV8] Post-NMS detections: %d\n", p_parm->results->count);
    
    return HD_OK;
}

HD_RESULT yolov8_load_labels(const char *filename, char *labels_buf)
{
    FILE *fd = fopen(filename, "r");
    if (!fd) {
        printf("[YOLOV8] Failed to open labels file: %s\n", filename);
        return HD_ERR_NG;
    }
    
    char *p_line = labels_buf;
    while (fgets(p_line, YOLOV8_LABEL_LEN, fd) != NULL) {
        // Remove newline
        size_t len = strlen(p_line);
        if (len > 0 && p_line[len - 1] == '\n') {
            p_line[len - 1] = '\0';
        }
        p_line += YOLOV8_LABEL_LEN;
    }
    
    fclose(fd);
    printf("[YOLOV8] Loaded labels from %s\n", filename);
    return HD_OK;
}

const char* yolov8_get_class_label(INT32 class_id, const char *labels)
{
    if (labels && class_id >= 0 && class_id < YOLOV8_NUM_CLASSES) {
        return &labels[class_id * YOLOV8_LABEL_LEN];
    }
    return "unknown";
}

void yolov8_print_results(const YOLOV8_RESULTS *results, const char *labels)
{
    printf("\n========== YOLOv8 Detection Results ==========\n");
    printf("Total detections: %d\n", results->count);
    printf("----------------------------------------------\n");
    
    for (int i = 0; i < results->count; i++) {
        const YOLOV8_DETECTION *det = &results->detections[i];
        const char *label = yolov8_get_class_label(det->class_id, labels);
        
        printf("%2d. [%s] conf=%.2f%% bbox=(%.3f, %.3f, %.3f, %.3f)\n",
               i + 1, label, det->confidence * 100.0f,
               det->x1, det->y1, det->x2, det->y2);
    }
    
    printf("==============================================\n\n");
}
