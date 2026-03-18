/**
 * @file osd_wrapper.h
 * @brief C wrapper for C++ OSD Overlay functions for ONVIF integration
 * 
 * Provides C-callable functions to interact with the ipcam::platform::OsdOverlay
 * class from ONVIF C code.
 */

#ifndef ONVIF_OSD_WRAPPER_H
#define ONVIF_OSD_WRAPPER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>

// ============================================================================
// OSD Types
// ============================================================================
typedef enum {
    ONVIF_OSD_TYPE_TEXT = 0,
    ONVIF_OSD_TYPE_IMAGE = 1,
    ONVIF_OSD_TYPE_DATE_TIME = 2
} OnvifOsdType;

typedef enum {
    ONVIF_OSD_POS_UPPER_LEFT = 0,
    ONVIF_OSD_POS_UPPER_RIGHT = 1,
    ONVIF_OSD_POS_LOWER_LEFT = 2,
    ONVIF_OSD_POS_LOWER_RIGHT = 3,
    ONVIF_OSD_POS_CUSTOM = 4
} OnvifOsdPosition;

// ============================================================================
// OSD Configuration
// ============================================================================
typedef struct {
    char token[64];
    OnvifOsdType type;
    OnvifOsdPosition position;
    int pos_x;              // Custom position X
    int pos_y;              // Custom position Y
    char text[256];         // For text type
    char date_format[64];   // For datetime type: "yyyy-MM-dd", etc.
    char time_format[64];   // For datetime type: "HH:mm:ss", etc.
    int font_size;
    uint32_t text_color;    // ARGB
    uint32_t bg_color;      // ARGB (0 = transparent)
    bool enabled;
} OnvifOsdConfig;

// ============================================================================
// Privacy Mask Configuration
// ============================================================================
typedef struct {
    char token[64];
    bool enabled;
    int x1, y1;             // Top-left corner
    int x2, y2;             // Bottom-right corner  
    uint32_t color;         // ARGB color
} OnvifPrivacyMask;

// ============================================================================
// OSD Control Functions
// ============================================================================

/**
 * @brief Get number of OSD elements
 * @param stream_idx Stream index, or -1 for all
 * @return Number of OSD elements
 */
int onvif_get_osd_count(int stream_idx);

/**
 * @brief Get OSD configuration
 * @param stream_idx Stream index
 * @param osd_idx OSD element index
 * @param config Output configuration
 * @return 0 on success, -1 on error
 */
int onvif_get_osd_config(int stream_idx, int osd_idx, OnvifOsdConfig *config);

/**
 * @brief Set OSD configuration
 * @param stream_idx Stream index
 * @param osd_idx OSD element index
 * @param config Configuration to set
 * @return 0 on success, -1 on error
 */
int onvif_set_osd_config(int stream_idx, int osd_idx, const OnvifOsdConfig *config);

/**
 * @brief Create new OSD element
 * @param stream_idx Stream index  
 * @param config Configuration for new OSD
 * @return OSD token on success, NULL on error
 */
const char* onvif_create_osd(int stream_idx, const OnvifOsdConfig *config);

/**
 * @brief Delete OSD element
 * @param token OSD token to delete
 * @return 0 on success, -1 on error
 */
int onvif_delete_osd(const char *token);

/**
 * @brief Enable/disable timestamp OSD
 * @param stream_idx Stream index, or -1 for all
 * @param enabled Enable flag
 * @return 0 on success, -1 on error
 */
int onvif_set_timestamp_enabled(int stream_idx, bool enabled);

/**
 * @brief Set timestamp format
 * @param stream_idx Stream index, or -1 for all
 * @param format strftime format string
 * @return 0 on success, -1 on error
 */
int onvif_set_timestamp_format(int stream_idx, const char *format);

/**
 * @brief Set timestamp position
 * @param stream_idx Stream index, or -1 for all
 * @param x X position
 * @param y Y position
 * @return 0 on success, -1 on error
 */
int onvif_set_timestamp_position(int stream_idx, int x, int y);

/**
 * @brief Set custom text OSD
 * @param stream_idx Stream index, or -1 for all
 * @param text Text to display
 * @return 0 on success, -1 on error
 */
int onvif_set_custom_text(int stream_idx, const char *text);

/**
 * @brief Enable/disable custom text OSD
 * @param stream_idx Stream index, or -1 for all
 * @param enabled Enable flag
 * @return 0 on success, -1 on error
 */
int onvif_set_custom_text_enabled(int stream_idx, bool enabled);

/**
 * @brief Set custom text position
 * @param stream_idx Stream index, or -1 for all
 * @param x X position
 * @param y Y position
 * @return 0 on success, -1 on error
 */
int onvif_set_custom_text_position(int stream_idx, int x, int y);

/**
 * @brief Enable/disable logo OSD
 * @param stream_idx Stream index, or -1 for all
 * @param enabled Enable flag
 * @return 0 on success, -1 on error
 */
int onvif_set_logo_enabled(int stream_idx, bool enabled);

/**
 * @brief Set logo position
 * @param stream_idx Stream index, or -1 for all
 * @param x X position
 * @param y Y position
 * @return 0 on success, -1 on error
 */
int onvif_set_logo_position(int stream_idx, int x, int y);

// ============================================================================
// Privacy Mask Functions
// ============================================================================

/**
 * @brief Get maximum number of privacy mask regions
 * @return Maximum regions per stream
 */
int onvif_get_max_privacy_masks(void);

/**
 * @brief Get privacy mask count for stream
 * @param stream_idx Stream index
 * @return Number of configured privacy masks
 */
int onvif_get_privacy_mask_count(int stream_idx);

/**
 * @brief Get privacy mask configuration
 * @param stream_idx Stream index
 * @param mask_idx Mask index (0-3)
 * @param mask Output mask configuration
 * @return 0 on success, -1 on error
 */
int onvif_get_privacy_mask(int stream_idx, int mask_idx, OnvifPrivacyMask *mask);

/**
 * @brief Set privacy mask configuration
 * @param stream_idx Stream index
 * @param mask_idx Mask index (0-3)
 * @param mask Mask configuration to set
 * @return 0 on success, -1 on error
 */
int onvif_set_privacy_mask(int stream_idx, int mask_idx, const OnvifPrivacyMask *mask);

/**
 * @brief Enable/disable privacy mask
 * @param stream_idx Stream index, or -1 for all
 * @param mask_idx Mask index
 * @param enabled Enable flag
 * @return 0 on success, -1 on error
 */
int onvif_set_privacy_mask_enabled(int stream_idx, int mask_idx, bool enabled);

/**
 * @brief Delete privacy mask
 * @param token Mask token to delete
 * @return 0 on success, -1 on error
 */
int onvif_delete_privacy_mask(const char *token);

/**
 * @brief Create privacy mask
 * @param stream_idx Stream index
 * @param mask Mask configuration
 * @return Mask token on success, NULL on error
 */
const char* onvif_create_privacy_mask(int stream_idx, const OnvifPrivacyMask *mask);

#ifdef __cplusplus
}
#endif

#endif /* ONVIF_OSD_WRAPPER_H */
