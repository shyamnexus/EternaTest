/*
 * SCRFD Face Detection Post-Processing
 * 
 * SCRFD-500M outputs 9 tensors across 3 stride levels (8, 16, 32):
 *   - score_8   [1, 12800, 1]   face confidence at stride 8
 *   - score_16  [1, 3200, 1]    face confidence at stride 16
 *   - score_32  [1, 800, 1]     face confidence at stride 32
 *   - bbox_8    [1, 12800, 4]   bbox (dx,dy,dw,dh) at stride 8
 *   - bbox_16   [1, 3200, 4]    bbox at stride 16
 *   - bbox_32   [1, 800, 4]     bbox at stride 32
 *   - kps_8     [1, 12800, 10]  5 keypoints (x,y) at stride 8
 *   - kps_16    [1, 3200, 10]   keypoints at stride 16
 *   - kps_32    [1, 800, 10]    keypoints at stride 32
 *
 * Output order from NPU: score_8, score_16, score_32, bbox_8, bbox_16, bbox_32, kps_8, kps_16, kps_32
 * But NPU may reorder - we identify by dimensions.
 *
 * Anchor layout per stride:
 *   stride 8:  80x80 grid, 2 anchors per cell = 12800
 *   stride 16: 40x40 grid, 2 anchors per cell = 3200
 *   stride 32: 20x20 grid, 2 anchors per cell = 800
 */

#ifndef _SCRFD_POSTPROC_H_
#define _SCRFD_POSTPROC_H_

#include "hd_type.h"

#define SCRFD_MAX_FACES         200
#define SCRFD_NUM_KEYPOINTS     5
#define SCRFD_NUM_STRIDES       3
#define SCRFD_ANCHORS_PER_CELL  2

/* Face detection result */
typedef struct {
    FLOAT x1, y1, x2, y2;      /* Bounding box (normalized 0-1) */
    FLOAT confidence;
    FLOAT landmarks[10];        /* 5 keypoints: (x0,y0, x1,y1, ..., x4,y4) normalized */
} SCRFD_FACE;

/* Detection results */
typedef struct {
    INT32 count;
    SCRFD_FACE faces[SCRFD_MAX_FACES];
} SCRFD_RESULTS;

/* Post-processing config */
typedef struct {
    FLOAT conf_threshold;
    FLOAT nms_threshold;
    INT32 input_width;
    INT32 input_height;
} SCRFD_POSTPROC_CFG;

/* Per-output info */
typedef struct {
    UINTPTR va;
    UINT32 fmt;
    FLOAT scale_ratio;
    INT32 zero_point;
    UINT32 width;       /* number of channels/features */
    UINT32 height;      /* number of anchors */
    UINT32 channel;
    UINT32 size;
} SCRFD_OUTPUT_INFO;

/* Post-processing parameters */
typedef struct {
    INT32 num_outputs;
    SCRFD_OUTPUT_INFO outputs[9];   /* up to 9 output tensors */
    SCRFD_POSTPROC_CFG cfg;
    SCRFD_RESULTS *results;
} SCRFD_POSTPROC_PARM;

HD_RESULT scrfd_postproc_init(void);
HD_RESULT scrfd_postproc_uninit(void);
HD_RESULT scrfd_postproc_process(SCRFD_POSTPROC_PARM *p_parm);
void scrfd_print_results(const SCRFD_RESULTS *results);

#endif /* _SCRFD_POSTPROC_H_ */
