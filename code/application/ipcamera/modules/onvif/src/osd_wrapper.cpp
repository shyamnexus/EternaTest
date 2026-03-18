/**
 * @file osd_wrapper.cpp
 * @brief C wrapper implementation for OSD Overlay functions
 * 
 * Implements the C wrapper functions by calling into the C++ OsdOverlay class.
 */

#include "ipcam/osd_wrapper.h"
#include "ipcam/osd_overlay.h"
#include <spdlog/spdlog.h>
#include <cstring>

using namespace ipcam::platform;

// Token storage for created OSDs
static int g_next_osd_token_id = 1;
static char g_last_created_osd_token[64] = {0};
static char g_last_created_mask_token[64] = {0};

extern "C" {

int onvif_get_osd_count(int stream_idx) {
    // Each stream has up to 3 OSD elements: timestamp, custom text, logo
    if (stream_idx < 0) {
        return OsdOverlay::kMaxStreams * 3;
    }
    return 3;
}

int onvif_get_osd_config(int stream_idx, int osd_idx, OnvifOsdConfig *config) {
    if (!config || stream_idx < 0 || stream_idx >= OsdOverlay::kMaxStreams) return -1;
    if (osd_idx < 0 || osd_idx >= 3) return -1;
    
    auto& osd = OsdOverlay::Instance();
    const auto& stream_config = osd.GetStreamConfig(stream_idx);
    
    memset(config, 0, sizeof(OnvifOsdConfig));
    
    switch (osd_idx) {
        case 0:  // Timestamp
            snprintf(config->token, sizeof(config->token), "OSD_Timestamp_%d", stream_idx);
            config->type = ONVIF_OSD_TYPE_DATE_TIME;
            config->enabled = stream_config.timestamp.enabled;
            config->pos_x = stream_config.timestamp.position_x;
            config->pos_y = stream_config.timestamp.position_y;
            strncpy(config->date_format, stream_config.timestamp.format.c_str(), sizeof(config->date_format) - 1);
            config->font_size = stream_config.timestamp.font_size;
            config->text_color = stream_config.timestamp.text_color;
            break;
            
        case 1:  // Custom text
            snprintf(config->token, sizeof(config->token), "OSD_Text_%d", stream_idx);
            config->type = ONVIF_OSD_TYPE_TEXT;
            config->enabled = stream_config.text.enabled;
            config->pos_x = stream_config.text.position_x;
            config->pos_y = stream_config.text.position_y;
            strncpy(config->text, stream_config.text.text.c_str(), sizeof(config->text) - 1);
            config->font_size = stream_config.text.font_size;
            config->text_color = stream_config.text.text_color;
            break;
            
        case 2:  // Logo
            snprintf(config->token, sizeof(config->token), "OSD_Logo_%d", stream_idx);
            config->type = ONVIF_OSD_TYPE_IMAGE;
            config->enabled = stream_config.logo.enabled;
            config->pos_x = stream_config.logo.position_x;
            config->pos_y = stream_config.logo.position_y;
            break;
    }
    
    return 0;
}

int onvif_set_osd_config(int stream_idx, int osd_idx, const OnvifOsdConfig *config) {
    if (!config || stream_idx < 0 || stream_idx >= OsdOverlay::kMaxStreams) return -1;
    if (osd_idx < 0 || osd_idx >= 3) return -1;
    
    auto& osd = OsdOverlay::Instance();
    
    switch (osd_idx) {
        case 0:  // Timestamp
            osd.SetTimestampEnabled(stream_idx, config->enabled);
            osd.SetTimestampPosition(stream_idx, config->pos_x, config->pos_y);
            osd.SetTimestampFormat(stream_idx, config->date_format);
            osd.SetTimestampFontSize(stream_idx, config->font_size);
            break;
            
        case 1:  // Custom text
            osd.SetCustomTextEnabled(stream_idx, config->enabled);
            osd.SetCustomTextPosition(stream_idx, config->pos_x, config->pos_y);
            osd.SetCustomText(stream_idx, config->text);
            break;
            
        case 2:  // Logo
            osd.SetLogoEnabled(stream_idx, config->enabled);
            osd.SetLogoPosition(stream_idx, config->pos_x, config->pos_y);
            break;
    }
    
    return 0;
}

const char* onvif_create_osd(int stream_idx, const OnvifOsdConfig *config) {
    if (!config || stream_idx < 0 || stream_idx >= OsdOverlay::kMaxStreams) {
        return nullptr;
    }
    
    // Generate new token
    snprintf(g_last_created_osd_token, sizeof(g_last_created_osd_token), 
             "OSD_%d_%d", stream_idx, g_next_osd_token_id++);
    
    auto& osd = OsdOverlay::Instance();
    
    switch (config->type) {
        case ONVIF_OSD_TYPE_TEXT:
            osd.SetCustomTextEnabled(stream_idx, config->enabled);
            osd.SetCustomTextPosition(stream_idx, config->pos_x, config->pos_y);
            osd.SetCustomText(stream_idx, config->text);
            break;
            
        case ONVIF_OSD_TYPE_DATE_TIME:
            osd.SetTimestampEnabled(stream_idx, config->enabled);
            osd.SetTimestampPosition(stream_idx, config->pos_x, config->pos_y);
            osd.SetTimestampFormat(stream_idx, config->date_format);
            break;
            
        case ONVIF_OSD_TYPE_IMAGE:
            osd.SetLogoEnabled(stream_idx, config->enabled);
            osd.SetLogoPosition(stream_idx, config->pos_x, config->pos_y);
            break;
    }
    
    return g_last_created_osd_token;
}

int onvif_delete_osd(const char *token) {
    if (!token) return -1;
    
    // Parse stream index from token
    int stream_idx = 0;
    if (sscanf(token, "OSD_Timestamp_%d", &stream_idx) == 1) {
        auto& osd = OsdOverlay::Instance();
        osd.SetTimestampEnabled(stream_idx, false);
        return 0;
    }
    if (sscanf(token, "OSD_Text_%d", &stream_idx) == 1) {
        auto& osd = OsdOverlay::Instance();
        osd.SetCustomTextEnabled(stream_idx, false);
        return 0;
    }
    if (sscanf(token, "OSD_Logo_%d", &stream_idx) == 1) {
        auto& osd = OsdOverlay::Instance();
        osd.SetLogoEnabled(stream_idx, false);
        return 0;
    }
    
    return -1;
}

int onvif_set_timestamp_enabled(int stream_idx, bool enabled) {
    auto& osd = OsdOverlay::Instance();
    osd.SetTimestampEnabled(stream_idx, enabled);
    return 0;
}

int onvif_set_timestamp_format(int stream_idx, const char *format) {
    if (!format) return -1;
    auto& osd = OsdOverlay::Instance();
    osd.SetTimestampFormat(stream_idx, format);
    return 0;
}

int onvif_set_timestamp_position(int stream_idx, int x, int y) {
    auto& osd = OsdOverlay::Instance();
    osd.SetTimestampPosition(stream_idx, x, y);
    return 0;
}

int onvif_set_custom_text(int stream_idx, const char *text) {
    if (!text) return -1;
    auto& osd = OsdOverlay::Instance();
    osd.SetCustomText(stream_idx, text);
    return 0;
}

int onvif_set_custom_text_enabled(int stream_idx, bool enabled) {
    auto& osd = OsdOverlay::Instance();
    osd.SetCustomTextEnabled(stream_idx, enabled);
    return 0;
}

int onvif_set_custom_text_position(int stream_idx, int x, int y) {
    auto& osd = OsdOverlay::Instance();
    osd.SetCustomTextPosition(stream_idx, x, y);
    return 0;
}

int onvif_set_logo_enabled(int stream_idx, bool enabled) {
    auto& osd = OsdOverlay::Instance();
    osd.SetLogoEnabled(stream_idx, enabled);
    return 0;
}

int onvif_set_logo_position(int stream_idx, int x, int y) {
    auto& osd = OsdOverlay::Instance();
    osd.SetLogoPosition(stream_idx, x, y);
    return 0;
}

// Privacy Mask Functions
int onvif_get_max_privacy_masks(void) {
    return 4;  // Maximum 4 privacy masks per stream
}

int onvif_get_privacy_mask_count(int stream_idx) {
    if (stream_idx < 0 || stream_idx >= OsdOverlay::kMaxStreams) return 0;
    
    auto& osd = OsdOverlay::Instance();
    const auto& config = osd.GetConfig();
    
    int count = 0;
    for (int i = 0; i < OsdStreamPrivacyMask::kMaxRegions; i++) {
        if (config.streams[stream_idx].privacy_mask.regions[i].enabled) {
            count++;
        }
    }
    return count;
}

int onvif_get_privacy_mask(int stream_idx, int mask_idx, OnvifPrivacyMask *mask) {
    if (!mask) return -1;
    if (stream_idx < 0 || stream_idx >= OsdOverlay::kMaxStreams) return -1;
    if (mask_idx < 0 || mask_idx >= OsdStreamPrivacyMask::kMaxRegions) return -1;
    
    auto& osd = OsdOverlay::Instance();
    const auto& config = osd.GetConfig();
    const auto& region = config.streams[stream_idx].privacy_mask.regions[mask_idx];
    
    snprintf(mask->token, sizeof(mask->token), "PrivacyMask_%d_%d", stream_idx, mask_idx);
    mask->enabled = region.enabled;
    mask->x1 = region.x1;
    mask->y1 = region.y1;
    mask->x2 = region.x2;
    mask->y2 = region.y2;
    mask->color = region.color;
    
    return 0;
}

int onvif_set_privacy_mask(int stream_idx, int mask_idx, const OnvifPrivacyMask *mask) {
    if (!mask) return -1;
    if (stream_idx < 0 || stream_idx >= OsdOverlay::kMaxStreams) return -1;
    if (mask_idx < 0 || mask_idx >= OsdStreamPrivacyMask::kMaxRegions) return -1;
    
    auto& osd = OsdOverlay::Instance();
    osd.SetPrivacyMaskRegion(stream_idx, mask_idx, mask->enabled,
                             mask->x1, mask->y1, mask->x2, mask->y2, mask->color);
    osd.ApplyPrivacyMaskToStream(stream_idx);
    
    return 0;
}

int onvif_set_privacy_mask_enabled(int stream_idx, int mask_idx, bool enabled) {
    if (stream_idx < 0 || stream_idx >= OsdOverlay::kMaxStreams) return -1;
    if (mask_idx < 0 || mask_idx >= OsdStreamPrivacyMask::kMaxRegions) return -1;
    
    auto& osd = OsdOverlay::Instance();
    const auto& config = osd.GetConfig();
    const auto& region = config.streams[stream_idx].privacy_mask.regions[mask_idx];
    
    osd.SetPrivacyMaskRegion(stream_idx, mask_idx, enabled,
                             region.x1, region.y1, region.x2, region.y2, region.color);
    osd.ApplyPrivacyMaskToStream(stream_idx);
    
    return 0;
}

int onvif_delete_privacy_mask(const char *token) {
    if (!token) return -1;
    
    int stream_idx = 0, mask_idx = 0;
    if (sscanf(token, "PrivacyMask_%d_%d", &stream_idx, &mask_idx) == 2) {
        auto& osd = OsdOverlay::Instance();
        osd.SetPrivacyMaskRegion(stream_idx, mask_idx, false, 0, 0, 0, 0, 0);
        osd.ApplyPrivacyMaskToStream(stream_idx);
        return 0;
    }
    
    return -1;
}

const char* onvif_create_privacy_mask(int stream_idx, const OnvifPrivacyMask *mask) {
    if (!mask || stream_idx < 0 || stream_idx >= OsdOverlay::kMaxStreams) {
        return nullptr;
    }
    
    // Find first available mask slot
    auto& osd = OsdOverlay::Instance();
    const auto& config = osd.GetConfig();
    
    for (int i = 0; i < OsdStreamPrivacyMask::kMaxRegions; i++) {
        if (!config.streams[stream_idx].privacy_mask.regions[i].enabled) {
            osd.SetPrivacyMaskRegion(stream_idx, i, mask->enabled,
                                     mask->x1, mask->y1, mask->x2, mask->y2, mask->color);
            osd.ApplyPrivacyMaskToStream(stream_idx);
            
            snprintf(g_last_created_mask_token, sizeof(g_last_created_mask_token),
                     "PrivacyMask_%d_%d", stream_idx, i);
            return g_last_created_mask_token;
        }
    }
    
    return nullptr;  // No available slots
}

} // extern "C"
