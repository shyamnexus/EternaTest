/**
 * @file yolov8_postproc.h
 * @brief YOLOv8 post-processing header for Novatek AI3
 *
 * Post-processing implementation for YOLOv8 model outputs
 * including NMS, bounding box decoding, and result formatting.
 *
 * Copyright 2026. All rights reserved.
 */
#ifndef _YOLOV8_POSTPROC_H_
#define _YOLOV8_POSTPROC_H_

#include "hd_type.h"

#ifdef __cplusplus
extern "C" {
#endif

/********************************************************************
    MACRO CONSTANT DEFINITIONS
********************************************************************/
#define YOLOV8_MAX_DETECTIONS   100     ///< Maximum detections per frame
#define YOLOV8_NUM_CLASSES      80      ///< COCO dataset classes
#define YOLOV8_LABEL_LEN        64      ///< Max label string length

// YOLOv8n output tensor dimensions (after decode)
// Raw output: [1, 84, 8400] where 84 = 4 (bbox) + 80 (classes)
#define YOLOV8_NUM_ANCHORS      8400    ///< Number of anchor points
#define YOLOV8_BOX_PARAMS       4       ///< x_center, y_center, width, height

// Default thresholds
#define YOLOV8_DEFAULT_CONF_THRESHOLD   0.25f
#define YOLOV8_DEFAULT_NMS_THRESHOLD    0.45f

/********************************************************************
    TYPE DEFINITIONS
********************************************************************/

/**
 * @brief Single detection result
 */
typedef struct _YOLOV8_DETECTION {
    FLOAT x1;           ///< Left coordinate (0.0 - 1.0 normalized)
    FLOAT y1;           ///< Top coordinate
    FLOAT x2;           ///< Right coordinate  
    FLOAT y2;           ///< Bottom coordinate
    FLOAT confidence;   ///< Detection confidence
    INT32 class_id;     ///< Class index (0-79 for COCO)
} YOLOV8_DETECTION;

/**
 * @brief Detection results container
 */
typedef struct _YOLOV8_RESULTS {
    YOLOV8_DETECTION detections[YOLOV8_MAX_DETECTIONS];
    INT32 count;        ///< Number of valid detections
} YOLOV8_RESULTS;

/**
 * @brief Post-processing configuration
 */
typedef struct _YOLOV8_POSTPROC_CFG {
    FLOAT conf_threshold;   ///< Confidence threshold (default 0.25)
    FLOAT nms_threshold;    ///< NMS IoU threshold (default 0.45)
    INT32 input_width;      ///< Model input width (640)
    INT32 input_height;     ///< Model input height (640)
    INT32 num_classes;      ///< Number of classes (80 for COCO)
} YOLOV8_POSTPROC_CFG;

/**
 * @brief Post-processing input parameters
 */
typedef struct _YOLOV8_POSTPROC_PARM {
    UINTPTR out_addr;       ///< [in] Address of model output tensor
    UINT32 out_fmt;         ///< [in] Output format (fixed point info)
    FLOAT scale_ratio;      ///< [in] Quantization scale
    INT32 zero_point;       ///< [in] Quantization zero point
    YOLOV8_POSTPROC_CFG cfg;///< [in] Post-processing config
    YOLOV8_RESULTS *results;///< [out] Detection results
} YOLOV8_POSTPROC_PARM;

/********************************************************************
    FUNCTION PROTOTYPES
********************************************************************/

/**
 * @brief Initialize YOLOv8 post-processing
 * @return HD_OK on success
 */
HD_RESULT yolov8_postproc_init(void);

/**
 * @brief Uninitialize YOLOv8 post-processing
 * @return HD_OK on success
 */
HD_RESULT yolov8_postproc_uninit(void);

/**
 * @brief Process YOLOv8 model output
 * @param p_parm Post-processing parameters
 * @return HD_OK on success
 */
HD_RESULT yolov8_postproc_process(YOLOV8_POSTPROC_PARM *p_parm);

/**
 * @brief Get class label by ID
 * @param class_id Class index (0-79)
 * @param labels Label array (loaded from file)
 * @return Pointer to label string or "unknown"
 */
const char* yolov8_get_class_label(INT32 class_id, const char *labels);

/**
 * @brief Load COCO labels from file
 * @param filename Path to labels file
 * @param labels_buf Output buffer (must be YOLOV8_NUM_CLASSES * YOLOV8_LABEL_LEN bytes)
 * @return HD_OK on success
 */
HD_RESULT yolov8_load_labels(const char *filename, char *labels_buf);

/**
 * @brief Print detection results
 * @param results Detection results
 * @param labels Label array (can be NULL)
 */
void yolov8_print_results(const YOLOV8_RESULTS *results, const char *labels);

#ifdef __cplusplus
}
#endif

#endif /* _YOLOV8_POSTPROC_H_ */
