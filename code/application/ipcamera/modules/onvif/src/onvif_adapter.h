/**
 * @file onvif_adapter.h
 * @brief Header file for Novatek ONVIF adapter/stub functions
 * 
 * Adapts legacy ONVIF code to Novatek platform APIs via wrappers.
 */

#ifndef ONVIF_ADAPTER_H
#define ONVIF_ADAPTER_H

// OSD IDs
#ifndef OSD_TIME_ID
#define OSD_TIME_ID 0
#endif

#ifndef OSD_CHANNEL_ID
#define OSD_CHANNEL_ID 1
#endif

#ifndef OSD_CHARACTER_ID  
#define OSD_CHARACTER_ID 2
#endif

#ifndef OSD_IMAGE_ID
#define OSD_IMAGE_ID 3
#endif

// Platform success codes
#ifndef NVT_SUCCESS
#define NVT_SUCCESS 0
#endif

// Helper declarations
int isValidIp4(const char* ip);

// Function declarations
// Video Getters
int nvt_adapter_video_get_resolution(int channel, char* value);
int nvt_adapter_video_get_frame_rate(int channel, int* value);
int nvt_adapter_video_get_max_rate(int channel, int* value);
int nvt_adapter_video_get_h264_profile(int channel, int* value);
int nvt_adapter_video_get_video_codec(int channel, int* codec_type);  // Returns 2=H264, 3=H265
int nvt_adapter_video_set_gop(int channel, int gop);
int nvt_adapter_video_get_gop(int channel, int* gop);

// Model Control
int nvt_adapter_model_start(void);
int nvt_adapter_model_stop(void);

// OSD Control
int nvt_adapter_osd_get_position_x(int id, int* pos);
int nvt_adapter_osd_get_position_y(int id, int* pos);
int nvt_adapter_osd_set_position_x(int id, int pos);
int nvt_adapter_osd_set_position_y(int id, int pos);
int nvt_adapter_osd_set_enabled(int id, int enabled);
int nvt_adapter_osd_set_display_text(int id, const char* text);
int nvt_adapter_osd_set_font_color_of_channelLocation(const char* color);
int nvt_adapter_osd_get_enabled(int id, int* enabled);
int nvt_adapter_osd_get_enable(int* enabled);
int nvt_adapter_osd_set_date_style(int id, const char* style);
int nvt_adapter_osd_set_time_style(int id, const char* style);
int nvt_adapter_osd_get_font_size(int* size);
int nvt_adapter_osd_set_font_size_with_id(int id, int size);
int nvt_adapter_osd_set_font_size(int size);
int nvt_adapter_osd_set_font_color_of_dateTime(const char* color);
int nvt_adapter_osd_set_font_color_of_channelName(const char* color);
int nvt_adapter_osd_restart(void);
int nvt_adapter_osd_get_start_time(int id, char* time_str);

// Parameter/Config
const char* nvt_adapter_param_get_string(const char* key, const char* default_value);
int nvt_adapter_param_get_int(const char* key, int default_value);
int nvt_adapter_param_set_string(const char* key, const char* value);
int nvt_adapter_param_set_int(const char* key, int value);

// Network
int nvt_adapter_network_dns_get(char* dns1, char* dns2);
int nvt_adapter_network_dns_set(const char* dns1, const char* dns2);
int nvt_adapter_network_ipv4_get(const char* interface, char* method, char* address, char* netmask, char* gateway);

// Video Setters (Legacy Stubs/Adapters)
int nvt_adapter_video_set_resolution(int channel, const char* resolution);
int nvt_adapter_video_set_frame_rate(int channel, int fps);
int nvt_adapter_video_set_max_rate(int channel, int bitrate);
int nvt_adapter_video_set_h264_profile(int channel, int profile);

// System/User
#include <time.h>
int nvt_adapter_system_update_user(const char* username, const char* password, int userLevel);
int nvt_adapter_system_register_user(const char* username, const char* password, int userLevel);
int nvt_adapter_system_set_time(const struct tm* time);

#endif // ONVIF_ADAPTER_H
