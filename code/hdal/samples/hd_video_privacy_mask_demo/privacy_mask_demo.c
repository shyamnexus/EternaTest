/**
 * @file privacy_mask_demo.c
 * @brief Demo: ISP-level vs Encoder-level Privacy Masks
 * 
 * This demo shows two approaches to privacy masking:
 * 
 * 1. VIDEOCAP (ISP) Level - GLOBAL mask applied to ALL streams
 *    - Use: HD_VIDEOCAP_PARAM_OUT_MASK_ATTR
 *    - Applied at sensor/ISP output, before any encoding
 *    - ONE mask configuration affects ALL encoders
 *    - Recommended for privacy compliance
 * 
 * 2. VIDEOENC Level - PER-STREAM mask
 *    - Use: HD_VIDEOENC_PARAM_IN_MASK_ATTR  
 *    - Applied per encoder path
 *    - Each stream can have different masks (not recommended for privacy)
 *    - Used in video_liveview_with_osg.c sample
 * 
 * For true privacy protection, use VIDEOCAP level!
 */

#include <stdio.h>
#include <string.h>
#include "hdal.h"
#include "hd_debug.h"

// ============================================================================
// Configuration
// ============================================================================

#define USE_ISP_LEVEL_MASK  1   // 1 = Global ISP mask, 0 = Per-encoder mask

#define SEN_VCAP_ID         0
#define VDO_SIZE_W          2944
#define VDO_SIZE_H          1664

// Privacy mask region (normalized to main resolution)
#define MASK_X1     800
#define MASK_Y1     400
#define MASK_X2     1200
#define MASK_Y2     700
#define MASK_COLOR  0xFF000000  // Black
#define MASK_ALPHA  255

#define MAX_MASKS   4

// ============================================================================
// Method 1: ISP-Level (VIDEOCAP) Privacy Mask - GLOBAL for all streams
// ============================================================================

typedef struct {
    HD_PATH_ID vcap_mask_path[MAX_MASKS];
    int num_masks;
} ISP_MASK_CONTEXT;

/**
 * Initialize ISP-level privacy masks (applied to ALL streams)
 * This is the recommended approach for privacy compliance
 */
static HD_RESULT isp_mask_init(ISP_MASK_CONTEXT *ctx)
{
    HD_RESULT ret;
    
    memset(ctx, 0, sizeof(ISP_MASK_CONTEXT));
    
    // Open mask paths on VIDEOCAP output
    // These masks are applied at the ISP output, BEFORE going to any videoproc/encoder
    for (int i = 0; i < MAX_MASKS; i++) {
        ret = hd_videocap_open(HD_VIDEOCAP_0_CTRL, HD_MASK(i), &ctx->vcap_mask_path[i]);
        if (ret != HD_OK) {
            printf("ISP_MASK: Failed to open vcap mask %d: %d\n", i, ret);
            return ret;
        }
        ctx->num_masks++;
    }
    
    printf("ISP_MASK: Opened %d mask paths on VIDEOCAP (global)\n", ctx->num_masks);
    return HD_OK;
}

/**
 * Set a privacy mask region at ISP level
 * This mask will appear on ALL streams (main, sub, third, etc.)
 */
static HD_RESULT isp_mask_set_region(ISP_MASK_CONTEXT *ctx, int mask_id,
                                     int x1, int y1, int x2, int y2,
                                     unsigned int color, unsigned char alpha)
{
    HD_OSG_MASK_ATTR attr;
    
    if (mask_id < 0 || mask_id >= ctx->num_masks) {
        printf("ISP_MASK: Invalid mask_id %d\n", mask_id);
        return HD_ERR_NG;
    }
    
    memset(&attr, 0, sizeof(HD_OSG_MASK_ATTR));
    
    // Define 4-point polygon (rectangle)
    attr.position[0].x = x1;  attr.position[0].y = y1;  // Top-left
    attr.position[1].x = x2;  attr.position[1].y = y1;  // Top-right
    attr.position[2].x = x2;  attr.position[2].y = y2;  // Bottom-right
    attr.position[3].x = x1;  attr.position[3].y = y2;  // Bottom-left
    
    attr.type  = HD_OSG_MASK_TYPE_SOLID;
    attr.color = color;
    attr.alpha = alpha;
    
    // Apply at VIDEOCAP level - this affects ALL streams!
    HD_RESULT ret = hd_videocap_set(ctx->vcap_mask_path[mask_id], 
                                     HD_VIDEOCAP_PARAM_OUT_MASK_ATTR, &attr);
    if (ret != HD_OK) {
        printf("ISP_MASK: Failed to set mask %d: %d\n", mask_id, ret);
        return ret;
    }
    
    printf("ISP_MASK: Set mask %d at (%d,%d)-(%d,%d) - GLOBAL for all streams\n",
           mask_id, x1, y1, x2, y2);
    return HD_OK;
}

/**
 * Enable/start ISP-level masks
 */
static HD_RESULT isp_mask_start(ISP_MASK_CONTEXT *ctx)
{
    for (int i = 0; i < ctx->num_masks; i++) {
        HD_RESULT ret = hd_videocap_start(ctx->vcap_mask_path[i]);
        if (ret != HD_OK) {
            printf("ISP_MASK: Failed to start mask %d: %d\n", i, ret);
        }
    }
    printf("ISP_MASK: All masks started\n");
    return HD_OK;
}

/**
 * Disable/stop ISP-level masks
 */
static HD_RESULT isp_mask_stop(ISP_MASK_CONTEXT *ctx)
{
    for (int i = 0; i < ctx->num_masks; i++) {
        hd_videocap_stop(ctx->vcap_mask_path[i]);
    }
    printf("ISP_MASK: All masks stopped\n");
    return HD_OK;
}

/**
 * Cleanup ISP-level mask resources
 */
static HD_RESULT isp_mask_exit(ISP_MASK_CONTEXT *ctx)
{
    for (int i = 0; i < ctx->num_masks; i++) {
        if (ctx->vcap_mask_path[i]) {
            hd_videocap_close(ctx->vcap_mask_path[i]);
        }
    }
    memset(ctx, 0, sizeof(ISP_MASK_CONTEXT));
    printf("ISP_MASK: Cleanup complete\n");
    return HD_OK;
}

// ============================================================================
// Method 2: Encoder-Level Privacy Mask - PER-STREAM (from existing sample)
// ============================================================================

typedef struct {
    HD_PATH_ID enc_mask_path[MAX_MASKS];
    int num_masks;
} ENC_MASK_CONTEXT;

/**
 * Initialize encoder-level privacy masks (per-stream)
 * NOTE: Each encoder needs its own mask configuration!
 */
static HD_RESULT enc_mask_init(ENC_MASK_CONTEXT *ctx, HD_IN_ID enc_in_id)
{
    HD_RESULT ret;
    
    memset(ctx, 0, sizeof(ENC_MASK_CONTEXT));
    
    // Open mask paths on specific ENCODER input
    // These masks only apply to THIS encoder, not others
    for (int i = 0; i < MAX_MASKS; i++) {
        ret = hd_videoenc_open(enc_in_id, HD_MASK(i), &ctx->enc_mask_path[i]);
        if (ret != HD_OK) {
            printf("ENC_MASK: Failed to open enc mask %d: %d\n", i, ret);
            return ret;
        }
        ctx->num_masks++;
    }
    
    printf("ENC_MASK: Opened %d mask paths on encoder (per-stream only)\n", ctx->num_masks);
    return HD_OK;
}

/**
 * Set a privacy mask region at encoder level
 * WARNING: This only affects ONE stream! Other streams won't have this mask!
 */
static HD_RESULT enc_mask_set_region(ENC_MASK_CONTEXT *ctx, int mask_id,
                                     int x1, int y1, int x2, int y2,
                                     unsigned int color, unsigned char alpha,
                                     int stream_width, int stream_height)
{
    HD_OSG_MASK_ATTR attr;
    
    if (mask_id < 0 || mask_id >= ctx->num_masks) {
        printf("ENC_MASK: Invalid mask_id %d\n", mask_id);
        return HD_ERR_NG;
    }
    
    // Scale coordinates to this stream's resolution
    float scale_x = (float)stream_width / VDO_SIZE_W;
    float scale_y = (float)stream_height / VDO_SIZE_H;
    
    int sx1 = (int)(x1 * scale_x);
    int sy1 = (int)(y1 * scale_y);
    int sx2 = (int)(x2 * scale_x);
    int sy2 = (int)(y2 * scale_y);
    
    memset(&attr, 0, sizeof(HD_OSG_MASK_ATTR));
    
    attr.position[0].x = sx1;  attr.position[0].y = sy1;
    attr.position[1].x = sx2;  attr.position[1].y = sy1;
    attr.position[2].x = sx2;  attr.position[2].y = sy2;
    attr.position[3].x = sx1;  attr.position[3].y = sy2;
    
    attr.type  = HD_OSG_MASK_TYPE_SOLID;
    attr.color = color;
    attr.alpha = alpha;
    
    // Apply at ENCODER level - only affects THIS stream
    HD_RESULT ret = hd_videoenc_set(ctx->enc_mask_path[mask_id], 
                                     HD_VIDEOENC_PARAM_IN_MASK_ATTR, &attr);
    if (ret != HD_OK) {
        printf("ENC_MASK: Failed to set mask %d: %d\n", mask_id, ret);
        return ret;
    }
    
    printf("ENC_MASK: Set mask %d at (%d,%d)-(%d,%d) - THIS stream only!\n",
           mask_id, sx1, sy1, sx2, sy2);
    return HD_OK;
}

static HD_RESULT enc_mask_start(ENC_MASK_CONTEXT *ctx)
{
    for (int i = 0; i < ctx->num_masks; i++) {
        hd_videoenc_start(ctx->enc_mask_path[i]);
    }
    printf("ENC_MASK: All masks started\n");
    return HD_OK;
}

static HD_RESULT enc_mask_stop(ENC_MASK_CONTEXT *ctx)
{
    for (int i = 0; i < ctx->num_masks; i++) {
        hd_videoenc_stop(ctx->enc_mask_path[i]);
    }
    return HD_OK;
}

static HD_RESULT enc_mask_exit(ENC_MASK_CONTEXT *ctx)
{
    for (int i = 0; i < ctx->num_masks; i++) {
        if (ctx->enc_mask_path[i]) {
            hd_videoenc_close(ctx->enc_mask_path[i]);
        }
    }
    memset(ctx, 0, sizeof(ENC_MASK_CONTEXT));
    return HD_OK;
}

// ============================================================================
// Comparison Summary
// ============================================================================

static void print_comparison(void)
{
    printf("\n");
    printf("================================================================\n");
    printf("    PRIVACY MASK COMPARISON: ISP vs ENCODER Level\n");
    printf("================================================================\n");
    printf("\n");
    printf("  ISP Level (HD_VIDEOCAP_PARAM_OUT_MASK_ATTR):\n");
    printf("    + ONE configuration applies to ALL streams\n");
    printf("    + True privacy protection - no way to bypass\n");
    printf("    + More efficient - mask applied once at source\n");
    printf("    + Coordinates in sensor resolution\n");
    printf("    - Cannot have different masks per stream\n");
    printf("\n");
    printf("  Encoder Level (HD_VIDEOENC_PARAM_IN_MASK_ATTR):\n");
    printf("    + Per-stream customization possible\n");
    printf("    + Can be disabled on specific streams\n");
    printf("    - Must configure EACH encoder separately\n");
    printf("    - Risk of forgetting to mask a stream\n");
    printf("    - Must scale coordinates per resolution\n");
    printf("    - NOT recommended for privacy compliance\n");
    printf("\n");
    printf("  RECOMMENDATION: Use ISP level for privacy masks!\n");
    printf("================================================================\n\n");
}

// ============================================================================
// Demo Main
// ============================================================================

#if 0  // Set to 1 to compile as standalone demo
int main(int argc, char **argv)
{
    print_comparison();
    
#if USE_ISP_LEVEL_MASK
    printf("Demo: Using ISP-level (global) privacy mask\n\n");
    
    ISP_MASK_CONTEXT isp_ctx;
    
    // Initialize HDAL (simplified - in real app, full pipeline needed)
    hd_common_init(0);
    hd_videocap_init();
    
    // Open ISP mask paths
    isp_mask_init(&isp_ctx);
    
    // Set privacy mask region - this will appear on ALL streams!
    isp_mask_set_region(&isp_ctx, 0, MASK_X1, MASK_Y1, MASK_X2, MASK_Y2, 
                        MASK_COLOR, MASK_ALPHA);
    
    // Start masks
    isp_mask_start(&isp_ctx);
    
    printf("\nPress Enter to stop...\n");
    getchar();
    
    // Cleanup
    isp_mask_stop(&isp_ctx);
    isp_mask_exit(&isp_ctx);
    
    hd_videocap_uninit();
    hd_common_uninit();
    
#else
    printf("Demo: Using encoder-level (per-stream) privacy mask\n\n");
    printf("NOTE: You must set mask on EACH encoder for full coverage!\n\n");
    
    ENC_MASK_CONTEXT enc_ctx;
    
    // In real app, would need to do this for EACH encoder:
    // - HD_VIDEOENC_0_IN_0 (main stream)
    // - HD_VIDEOENC_1_IN_0 (sub stream)  
    // - HD_VIDEOENC_2_IN_0 (third stream)
    
    // This demo only shows one encoder
    enc_mask_init(&enc_ctx, HD_VIDEOENC_0_IN_0);
    
    // Set mask for 1080p stream
    enc_mask_set_region(&enc_ctx, 0, MASK_X1, MASK_Y1, MASK_X2, MASK_Y2,
                        MASK_COLOR, MASK_ALPHA, 1920, 1080);
    
    enc_mask_start(&enc_ctx);
    
    printf("\nPress Enter to stop...\n");
    getchar();
    
    enc_mask_stop(&enc_ctx);
    enc_mask_exit(&enc_ctx);
    
#endif
    
    return 0;
}
#endif

// ============================================================================
// API for integration with ipcamera application
// ============================================================================

/**
 * For C++ integration, expose these functions:
 */
#ifdef __cplusplus
extern "C" {
#endif

// ISP-level (recommended)
int privacy_mask_isp_init(void);
int privacy_mask_isp_set(int mask_id, int x1, int y1, int x2, int y2, 
                         unsigned int color, unsigned char alpha);
int privacy_mask_isp_enable(int mask_id, int enable);
int privacy_mask_isp_exit(void);

// Encoder-level (legacy)
int privacy_mask_enc_init(int enc_id);
int privacy_mask_enc_set(int enc_id, int mask_id, int x1, int y1, int x2, int y2,
                         int stream_w, int stream_h, unsigned int color, unsigned char alpha);
int privacy_mask_enc_enable(int enc_id, int mask_id, int enable);
int privacy_mask_enc_exit(int enc_id);

#ifdef __cplusplus
}
#endif
