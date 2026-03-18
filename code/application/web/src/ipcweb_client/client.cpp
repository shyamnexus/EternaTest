#include "api_handler.h"
#include "Logger.h"
#include "socket.h"
#include <string.h>
int hon_client_get_int_by_id(char *func, int id, int *value) {
  int fd;
  int ret = 0;

  fd = cli_begin(func);
  /* Transmission parameters */
  sock_write(fd, &id, sizeof(int));
  sock_read(fd, value, sizeof(int));
  sock_read(fd, &ret, sizeof(int));
  /* End transmission parameters */
  ret = cli_end(fd);

  return ret;
}

int hon_client_set_int_by_id(char *func, int id, int value) {
  int fd;
  int ret = 0;

  fd = cli_begin(func);
  /* Transmission parameters */
  sock_write(fd, &id, sizeof(int));
  sock_write(fd, &value, sizeof(int));
  sock_read(fd, &ret, sizeof(int));
  /* End transmission parameters */
  ret = cli_end(fd);

  return ret;
}

int hon_client_get_int(char *func, int *value) {
  int fd;
  int ret = 0;

  fd = cli_begin(func);
  /* Transmission parameters */
  sock_read(fd, value, sizeof(int));
  sock_read(fd, &ret, sizeof(int));
  /* End transmission parameters */
  ret = cli_end(fd);

  return ret;
}

int hon_client_set_int(char *func, int value) {
  int fd;
  int ret = 0;

  fd = cli_begin(func);
  /* Transmission parameters */
  sock_write(fd, &value, sizeof(int));
  sock_read(fd, &ret, sizeof(int));
  /* End transmission parameters */
  ret = cli_end(fd);

  return ret;
}

int hon_client_get_string_by_id(char *func, int id, char **value) {
  int fd, len;
  int ret = 0;

  fd = cli_begin(func);
  /* Transmission parameters */
  sock_write(fd, &id, sizeof(id));
  sock_read(fd, &len, sizeof(int));
  sock_read(fd, *value, len);
  memset(*value + len, '\0', 1); // set terminator
  sock_read(fd, &ret, sizeof(int));
  /* End transmission parameters */
  ret = cli_end(fd);

  return ret;
}

int hon_client_set_string_by_id(char *func, int id, const char *value) {
  int fd, len;
  int ret = 0;

  fd = cli_begin(func);
  /* Transmission parameters */
  sock_write(fd, &id, sizeof(id));
  len = strlen(value) + 1;
  sock_write(fd, &len, sizeof(int));
  sock_write(fd, value, len);
  sock_read(fd, &ret, sizeof(int));
  /* End transmission parameters */
  ret = cli_end(fd);

  return ret;
}

int hon_client_get_string(char *func, char **value) {
  int fd, len;
  int ret = 0;

  fd = cli_begin(func);
  /* Transmission parameters */
  sock_read(fd, &len, sizeof(int));
  sock_read(fd, *value, len);
  memset(*value + len, '\0', 1); // set terminator
  sock_read(fd, &ret, sizeof(int));
  /* End transmission parameters */
  ret = cli_end(fd);

  return ret;
}

int hon_client_set_string(char *func, const char *value) {
  int fd, len;
  int ret = 0;

  fd = cli_begin(func);
  /* Transmission parameters */
  len = strlen(value) + 1;
  sock_write(fd, &len, sizeof(int));
  sock_write(fd, value, len);
  sock_read(fd, &ret, sizeof(int));
  /* End transmission parameters */
  ret = cli_end(fd);

  return ret;
}

int hon_isp_set(const char *json) {
  return hon_client_set_string((char *)__func__, json);
}

int hon_isp_get_scenario(int cam_id, char **value) {
  return hon_client_get_string_by_id((char *)__func__, cam_id, value);
}

int hon_isp_set_scenario(int cam_id, const char *value) {
  return hon_client_set_string_by_id((char *)__func__, cam_id, value);
}

int hon_isp_set_default(int cam_id) {
  return hon_client_set_int_by_id((char *)__func__, cam_id, 1);
}

int hon_isp_get_contrast(int cam_id, int *value) {
  return hon_client_get_int_by_id((char *)__func__, cam_id, value);
}

int hon_isp_set_contrast(int cam_id, int value) {
  return hon_client_set_int_by_id((char *)__func__, cam_id, value);
}

int hon_isp_get_brightness(int cam_id, int *value) {
  return hon_client_get_int_by_id((char *)__func__, cam_id, value);
}

int hon_isp_set_brightness(int cam_id, int value) {
  return hon_client_set_int_by_id((char *)__func__, cam_id, value);
}

int hon_isp_get_saturation(int cam_id, int *value) {
  return hon_client_get_int_by_id((char *)__func__, cam_id, value);
}

int hon_isp_set_saturation(int cam_id, int value) {
  return hon_client_set_int_by_id((char *)__func__, cam_id, value);
}

int hon_isp_get_sharpness(int cam_id, int *value) {
  return hon_client_get_int_by_id((char *)__func__, cam_id, value);
}

int hon_isp_set_sharpness(int cam_id, int value) {
  return hon_client_set_int_by_id((char *)__func__, cam_id, value);
}

int hon_isp_get_hue(int cam_id, int *value) {
  return hon_client_get_int_by_id((char *)__func__, cam_id, value);
}

int hon_isp_set_hue(int cam_id, int value) {
  return hon_client_set_int_by_id((char *)__func__, cam_id, value);
}

// isp exposure
int hon_isp_get_exposure_mode(int cam_id, char **value) {
  return hon_client_get_string_by_id((char *)__func__, cam_id, value);
}

int hon_isp_set_exposure_mode(int cam_id, const char *value) {
  return hon_client_set_string_by_id((char *)__func__, cam_id, value);
}

int hon_isp_get_gain_mode(int cam_id, char **value) {
  return hon_client_get_string_by_id((char *)__func__, cam_id, value);
}

int hon_isp_set_gain_mode(int cam_id, const char *value) {
  return hon_client_set_string_by_id((char *)__func__, cam_id, value);
}

int hon_isp_get_exposure_time(int cam_id, char **value) {
  return hon_client_get_string_by_id((char *)__func__, cam_id, value);
}

int hon_isp_set_exposure_time(int cam_id, const char *value) {
  return hon_client_set_string_by_id((char *)__func__, cam_id, value);
}

int hon_isp_get_exposure_gain(int cam_id, int *value) {
  return hon_client_get_int_by_id((char *)__func__, cam_id, value);
}

int hon_isp_set_exposure_gain(int cam_id, int value) {
  return hon_client_set_int_by_id((char *)__func__, cam_id, value);
}

int hon_isp_get_frame_rate(int cam_id, int *value) {
  return hon_client_get_int_by_id((char *)__func__, cam_id, value);
}

int hon_isp_set_frame_rate(int cam_id, int value) {
  return hon_client_set_int_by_id((char *)__func__, cam_id, value);
}

// night_to_day
int hon_isp_get_night_to_day(int cam_id, char **value) {
  return hon_client_get_string_by_id((char *)__func__, cam_id, value);
}

int hon_isp_set_night_to_day(int cam_id, const char *value) {
  return hon_client_set_string_by_id((char *)__func__, cam_id, value);
}

int hon_isp_get_fill_light_mode(int cam_id, char **value) {
  return hon_client_get_string_by_id((char *)__func__, cam_id, value);
}

int hon_isp_set_fill_light_mode(int cam_id, const char *value) {
  return hon_client_set_string_by_id((char *)__func__, cam_id, value);
}

int hon_isp_get_light_brightness(int cam_id, int *value) {
  return hon_client_get_int_by_id((char *)__func__, cam_id, value);
}

int hon_isp_set_light_brightness(int cam_id, int value) {
  return hon_client_set_int_by_id((char *)__func__, cam_id, value);
}

int hon_isp_get_night_to_day_filter_level(int cam_id, int *value) {
  return hon_client_get_int_by_id((char *)__func__, cam_id, value);
}

int hon_isp_set_night_to_day_filter_level(int cam_id, int value) {
  return hon_client_set_int_by_id((char *)__func__, cam_id, value);
}

int hon_isp_get_night_to_day_filter_time(int cam_id, int *value) {
  return hon_client_get_int_by_id((char *)__func__, cam_id, value);
}

int hon_isp_set_night_to_day_filter_time(int cam_id, int value) {
  return hon_client_set_int_by_id((char *)__func__, cam_id, value);
}

int hon_isp_get_IR_mode(int cam_id, char **value) {
  return hon_client_get_string_by_id((char *)__func__, cam_id, value);
}

int hon_isp_set_IR_mode(int cam_id, const char *value) {
  return hon_client_set_string_by_id((char *)__func__, cam_id, value);
}

int hon_isp_get_IR_start_time(int cam_id, char **value) {
  return hon_client_get_string_by_id((char *)__func__, cam_id, value);
}

int hon_isp_set_IR_start_time(int cam_id, const char *value) {
  return hon_client_set_string_by_id((char *)__func__, cam_id, value);
}

int hon_isp_get_IR_end_time(int cam_id, char **value) {
  return hon_client_get_string_by_id((char *)__func__, cam_id, value);
}

int hon_isp_set_IR_end_time(int cam_id, const char *value) {
  return hon_client_set_string_by_id((char *)__func__, cam_id, value);
}

int hon_isp_set_IR_brightness_mode(int cam_id, const char *value) {
  return hon_client_set_string_by_id((char *)__func__, cam_id, value);
}

int hon_isp_get_IR_brightness_mode(int cam_id, char **value) {
  return hon_client_get_string_by_id((char *)__func__, cam_id, value);
}





int hon_isp_restart_IR_monitoring_thread() {
  int fd;
  int ret = 0;

  fd = cli_begin((char *)__func__);
  /* Transmission parameters */
  sock_read(fd, &ret, sizeof(int));
  /* End transmission parameters */
  ret = cli_end(fd);

  return ret;
}

int hon_isp_start_IR_monitoring_thread() {
  int fd;
  int ret = 0;

  fd = cli_begin((char *)__func__);
  /* Transmission parameters */
  sock_read(fd, &ret, sizeof(int));
  /* End transmission parameters */
  ret = cli_end(fd);

  return ret;
}

int hon_isp_stop_IR_monitoring_thread() {
  int fd;
  int ret = 0;

  fd = cli_begin((char *)__func__);
  /* Transmission parameters */
  sock_read(fd, &ret, sizeof(int));
  /* End transmission parameters */
  ret = cli_end(fd);

  return ret;
}





// blc
int hon_isp_get_hdr(int cam_id, char **value) {
  return hon_client_get_string_by_id((char *)__func__, cam_id, value);
}

int hon_isp_set_hdr(int cam_id, const char *value) {
  return hon_client_set_string_by_id((char *)__func__, cam_id, value);
}

int hon_isp_get_blc_region(int cam_id, char **value) {
  return hon_client_get_string_by_id((char *)__func__, cam_id, value);
}

int hon_isp_set_blc_region(int cam_id, const char *value) {
  return hon_client_set_string_by_id((char *)__func__, cam_id, value);
}

int hon_isp_get_hlc(int cam_id, char **value) {
  return hon_client_get_string_by_id((char *)__func__, cam_id, value);
}

int hon_isp_set_hlc(int cam_id, const char *value) {
  return hon_client_set_string_by_id((char *)__func__, cam_id, value);
}

int hon_isp_get_hdr_level(int cam_id, int *value) {
  return hon_client_get_int_by_id((char *)__func__, cam_id, value);
}

int hon_isp_set_hdr_level(int cam_id, int value) {
  return hon_client_set_int_by_id((char *)__func__, cam_id, value);
}

int hon_isp_get_blc_strength(int cam_id, int *value) {
  return hon_client_get_int_by_id((char *)__func__, cam_id, value);
}

int hon_isp_set_blc_strength(int cam_id, int value) {
  return hon_client_set_int_by_id((char *)__func__, cam_id, value);
}

int hon_isp_get_hlc_level(int cam_id, int *value) {
  return hon_client_get_int_by_id((char *)__func__, cam_id, value);
}

int hon_isp_set_hlc_level(int cam_id, int value) {
  return hon_client_set_int_by_id((char *)__func__, cam_id, value);
}

int hon_isp_get_dark_boost_level(int cam_id, int *value) {
  return hon_client_get_int_by_id((char *)__func__, cam_id, value);
}

int hon_isp_set_dark_boost_level(int cam_id, int value) {
  return hon_client_set_int_by_id((char *)__func__, cam_id, value);
}

// white_blance
int hon_isp_get_white_blance_style(int cam_id, char **value) {
  return hon_client_get_string_by_id((char *)__func__, cam_id, value);
}

int hon_isp_set_white_blance_style(int cam_id, const char *value) {
  return hon_client_set_string_by_id((char *)__func__, cam_id, value);
}

int hon_isp_get_white_blance_red(int cam_id, int *value) {
  return hon_client_get_int_by_id((char *)__func__, cam_id, value);
}

int hon_isp_set_white_blance_red(int cam_id, int value) {
  return hon_client_set_int_by_id((char *)__func__, cam_id, value);
}

int hon_isp_get_white_blance_green(int cam_id, int *value) {
  return hon_client_get_int_by_id((char *)__func__, cam_id, value);
}

int hon_isp_set_white_blance_green(int cam_id, int value) {
  return hon_client_set_int_by_id((char *)__func__, cam_id, value);
}

int hon_isp_get_white_blance_blue(int cam_id, int *value) {
  return hon_client_get_int_by_id((char *)__func__, cam_id, value);
}

int hon_isp_set_white_blance_blue(int cam_id, int value) {
  return hon_client_set_int_by_id((char *)__func__, cam_id, value);
}

// enhancement
int hon_isp_get_noise_reduce_mode(int cam_id, char **value) {
  return hon_client_get_string_by_id((char *)__func__, cam_id, value);
}

int hon_isp_set_noise_reduce_mode(int cam_id, const char *value) {
  return hon_client_set_string_by_id((char *)__func__, cam_id, value);
}

int hon_isp_get_dehaze(int cam_id, char **value) {
  return hon_client_get_string_by_id((char *)__func__, cam_id, value);
}

int hon_isp_set_dehaze(int cam_id, const char *value) {
  return hon_client_set_string_by_id((char *)__func__, cam_id, value);
}

int hon_isp_get_gray_scale_mode(int cam_id, char **value) {
  return hon_client_get_string_by_id((char *)__func__, cam_id, value);
}

int hon_isp_set_gray_scale_mode(int cam_id, const char *value) {
  return hon_client_set_string_by_id((char *)__func__, cam_id, value);
}

int hon_isp_get_distortion_correction(int cam_id, char **value) {
  return hon_client_get_string_by_id((char *)__func__, cam_id, value);
}

int hon_isp_set_distortion_correction(int cam_id, const char *value) {
  return hon_client_set_string_by_id((char *)__func__, cam_id, value);
}

int hon_isp_get_spatial_denoise_level(int cam_id, int *value) {
  return hon_client_get_int_by_id((char *)__func__, cam_id, value);
}

int hon_isp_set_spatial_denoise_level(int cam_id, int value) {
  return hon_client_set_int_by_id((char *)__func__, cam_id, value);
}

int hon_isp_get_temporal_denoise_level(int cam_id, int *value) {
  return hon_client_get_int_by_id((char *)__func__, cam_id, value);
}

int hon_isp_set_temporal_denoise_level(int cam_id, int value) {
  return hon_client_set_int_by_id((char *)__func__, cam_id, value);
}

int hon_isp_get_dehaze_level(int cam_id, int *value) {
  return hon_client_get_int_by_id((char *)__func__, cam_id, value);
}

int hon_isp_set_dehaze_level(int cam_id, int value) {
  return hon_client_set_int_by_id((char *)__func__, cam_id, value);
}

int hon_isp_get_fec_level(int cam_id, int *value) {
  return hon_client_get_int_by_id((char *)__func__, cam_id, value);
}

int hon_isp_set_fec_level(int cam_id, int value) {
  return hon_client_set_int_by_id((char *)__func__, cam_id, value);
}

int hon_isp_get_ldch_level(int cam_id, int *value) {
  return hon_client_get_int_by_id((char *)__func__, cam_id, value);
}

int hon_isp_set_ldch_level(int cam_id, int value) {
  return hon_client_set_int_by_id((char *)__func__, cam_id, value);
}

// video_adjustment
int hon_isp_get_power_line_frequency_mode(int cam_id, char **value) {
  return hon_client_get_string_by_id((char *)__func__, cam_id, value);
}

int hon_isp_set_power_line_frequency_mode(int cam_id, const char *value) {
  return hon_client_set_string_by_id((char *)__func__, cam_id, value);
}

int hon_isp_get_image_flip(int cam_id, char **value) {
  return hon_client_get_string_by_id((char *)__func__, cam_id, value);
}

int hon_isp_set_image_flip(int cam_id, const char *value) {
  return hon_client_set_string_by_id((char *)__func__, cam_id, value);
}

// auto focus
int hon_isp_get_af_mode(int cam_id, char **value) {
  return hon_client_get_string_by_id((char *)__func__, cam_id, value);
}

int hon_isp_set_af_mode(int cam_id, const char *value) {
  return hon_client_set_string_by_id((char *)__func__, cam_id, value);
}

int hon_isp_get_zoom_level(int cam_id, int *value) {
  return hon_client_get_int_by_id((char *)__func__, cam_id, value);
}

int hon_isp_get_focus_level(int cam_id, int *value) {
  return hon_client_get_int_by_id((char *)__func__, cam_id, value);
}

int hon_isp_af_zoom_in(int cam_id) {
  int fd;
  int ret = 0;

  fd = cli_begin((char *)__func__);
  /* Transmission parameters */
  sock_write(fd, &cam_id, sizeof(int));
  sock_read(fd, &ret, sizeof(int));
  /* End transmission parameters */
  ret = cli_end(fd);

  return ret;
}

int hon_isp_af_zoom_out(int cam_id) {
  int fd;
  int ret = 0;

  fd = cli_begin((char *)__func__);
  /* Transmission parameters */
  sock_write(fd, &cam_id, sizeof(int));
  sock_read(fd, &ret, sizeof(int));
  /* End transmission parameters */
  ret = cli_end(fd);

  return ret;
}

int hon_isp_af_focus_in(int cam_id) {
  int fd;
  int ret = 0;

  fd = cli_begin((char *)__func__);
  /* Transmission parameters */
  sock_write(fd, &cam_id, sizeof(int));
  sock_read(fd, &ret, sizeof(int));
  /* End transmission parameters */
  ret = cli_end(fd);

  return ret;
}

int hon_isp_af_focus_out(int cam_id) {
  int fd;
  int ret = 0;

  fd = cli_begin((char *)__func__);
  /* Transmission parameters */
  sock_write(fd, &cam_id, sizeof(int));
  sock_read(fd, &ret, sizeof(int));
  /* End transmission parameters */
  ret = cli_end(fd);

  return ret;
}

int hon_isp_af_focus_once(int cam_id) {
  int fd;
  int ret = 0;

  fd = cli_begin((char *)__func__);
  /* Transmission parameters */
  sock_write(fd, &cam_id, sizeof(int));
  sock_read(fd, &ret, sizeof(int));
  /* End transmission parameters */
  ret = cli_end(fd);

  return ret;
}

// video
int hon_video_set(const char *json) {
  return hon_client_set_string((char *)__func__, json);
}

int hon_video_restart() {
  int fd;
  int ret = 0;

  fd = cli_begin((char *)__func__);
  /* Transmission parameters */
  sock_read(fd, &ret, sizeof(int));
  /* End transmission parameters */
  ret = cli_end(fd);

  return ret;
}

int hon_video_get_gop(int stream_id, int *value) {
  return hon_client_get_int_by_id((char *)__func__, stream_id, value);
}

int hon_video_set_gop(int stream_id, int value) {
  return hon_client_set_int_by_id((char *)__func__, stream_id, value);
}

int hon_video_get_max_rate(int stream_id, int *value) {
  return hon_client_get_int_by_id((char *)__func__, stream_id, value);
}

int hon_video_set_max_rate(int stream_id, int value) {
  return hon_client_set_int_by_id((char *)__func__, stream_id, value);
}

int hon_video_get_RC_mode(int stream_id, char **value) {
  return hon_client_get_string_by_id((char *)__func__, stream_id, value);
}

int hon_video_set_RC_mode(int stream_id, const char *value) {
  return hon_client_set_string_by_id((char *)__func__, stream_id, value);
}

int hon_video_get_output_data_type(int stream_id, char **value) {
  return hon_client_get_string_by_id((char *)__func__, stream_id, value);
}

int hon_video_set_output_data_type(int stream_id, const char *value) {
  return hon_client_set_string_by_id((char *)__func__, stream_id, value);
}

int hon_video_get_rc_quality(int stream_id, char **value) {
  return hon_client_get_string_by_id((char *)__func__, stream_id, value);
}

int hon_video_set_rc_quality(int stream_id, const char *value) {
  return hon_client_set_string_by_id((char *)__func__, stream_id, value);
}

int hon_video_get_smart(int stream_id, char **value) {
  return hon_client_get_string_by_id((char *)__func__, stream_id, value);
}

int hon_video_set_smart(int stream_id, const char *value) {
  return hon_client_set_string_by_id((char *)__func__, stream_id, value);
}

int hon_video_get_gop_mode(int stream_id, char **value) {
  return hon_client_get_string_by_id((char *)__func__, stream_id, value);
}

int hon_video_set_gop_mode(int stream_id, const char *value) {
  return hon_client_set_string_by_id((char *)__func__, stream_id, value);
}

int hon_video_get_stream_type(int stream_id, char **value) {
  return hon_client_get_string_by_id((char *)__func__, stream_id, value);
}

int hon_video_set_stream_type(int stream_id, const char *value) {
  return hon_client_set_string_by_id((char *)__func__, stream_id, value);
}

int hon_video_get_h264_profile(int stream_id, char **value) {
  return hon_client_get_string_by_id((char *)__func__, stream_id, value);
}

int hon_video_set_h264_profile(int stream_id, const char *value) {
  return hon_client_set_string_by_id((char *)__func__, stream_id, value);
}

int hon_video_get_resolution(int stream_id, char **value) {
  return hon_client_get_string_by_id((char *)__func__, stream_id, value);
}

int hon_video_set_resolution(int stream_id, const char *value) {
  return hon_client_set_string_by_id((char *)__func__, stream_id, value);
}

int hon_video_get_frame_rate(int stream_id, char **value) {
  return hon_client_get_string_by_id((char *)__func__, stream_id, value);
}

int hon_video_set_frame_rate(int stream_id, const char *value) {
  return hon_client_set_string_by_id((char *)__func__, stream_id, value);
}

int hon_video_get_frame_rate_in(int stream_id, char **value) {
  return hon_client_get_string_by_id((char *)__func__, stream_id, value);
}

int hon_video_set_frame_rate_in(int stream_id, const char *value) {
  return hon_client_set_string_by_id((char *)__func__, stream_id, value);
}

int hon_video_get_rotation(int *value) {
  return hon_client_get_int((char *)__func__, value);
}

int hon_video_set_rotation(int value) {
  return hon_client_set_int((char *)__func__, value);
}

// jpeg
int hon_video_get_enable_cycle_snapshot(int *value) {
  return hon_client_get_int((char *)__func__, value);
}

int hon_video_set_enable_cycle_snapshot(int value) {
  return hon_client_set_int((char *)__func__, value);
}

int hon_video_get_image_quality(int *value) {
  return hon_client_get_int((char *)__func__, value);
}

int hon_video_set_image_quality(int value) {
  return hon_client_set_int((char *)__func__, value);
}

int hon_video_get_snapshot_interval_ms(int *value) {
  return hon_client_get_int((char *)__func__, value);
}

int hon_video_set_snapshot_interval_ms(int value) {
  return hon_client_set_int((char *)__func__, value);
}

int hon_video_get_jpeg_resolution(char **value) {
  return hon_client_get_string((char *)__func__, value);
}

int hon_video_set_jpeg_resolution(const char *value) {
  return hon_client_set_string((char *)__func__, value);
}

// audio
int hon_audio_set(const char *json) {
  return hon_client_set_string((char *)__func__, json);
}

int hon_audio_restart() {
  int fd;
  int ret = 0;

  fd = cli_begin((char *)__func__);
  /* Transmission parameters */
  sock_read(fd, &ret, sizeof(int));
  /* End transmission parameters */
  ret = cli_end(fd);

  return ret;
}

int hon_audio_deinit(){
  int fd;
  int ret = 0;

  fd = cli_begin((char *)__func__);
  /* Transmission parameters */
  sock_read(fd, &ret, sizeof(int));
  /* End transmission parameters */
  ret = cli_end(fd);

  return ret;
}

int hon_audio_get_bit_rate(int stream_id, int *value) {
  return hon_client_get_int_by_id((char *)__func__, stream_id, value);
}

int hon_audio_set_bit_rate(int stream_id, int value) {
  return hon_client_set_int_by_id((char *)__func__, stream_id, value);
}

int hon_audio_get_sample_rate(int stream_id, int *value) {
  return hon_client_get_int_by_id((char *)__func__, stream_id, value);
}

int hon_audio_set_sample_rate(int stream_id, int value) {
  return hon_client_set_int_by_id((char *)__func__, stream_id, value);
}

int hon_audio_get_volume(int stream_id, int *value) {
  return hon_client_get_int_by_id((char *)__func__, stream_id, value);
}

int hon_audio_set_volume(int stream_id, int value) {
  return hon_client_set_int_by_id((char *)__func__, stream_id, value);
}

int hon_audio_get_enable_vqe(int stream_id, int *value) {
  return hon_client_get_int_by_id((char *)__func__, stream_id, value);
}

int hon_audio_set_enable_vqe(int stream_id, int value) {
  return hon_client_set_int_by_id((char *)__func__, stream_id, value);
}

int hon_audio_get_encode_type(int stream_id, char **value) {
  return hon_client_get_string_by_id((char *)__func__, stream_id, value);
}

int hon_audio_set_encode_type(int stream_id, const char *value) {
  return hon_client_set_string_by_id((char *)__func__, stream_id, value);
}

// system

int hon_system_capability_get_video(char *value) {
  int fd, len;
  int ret = 0;

  fd = cli_begin((char *)__func__);
  /* Transmission parameters */
  sock_read(fd, &len, sizeof(int));
  if (len != 0)
    sock_read(fd, value, len);
  memset(value + len, '\0', 1); // set terminator
  sock_read(fd, &ret, sizeof(int));
  /* End transmission parameters */
  ret = cli_end(fd);

  return ret;
}

int hon_system_capability_get_image_adjustment(char *value) {
  int fd, len;
  int ret = 0;

  fd = cli_begin((char *)__func__);
  /* Transmission parameters */
  sock_read(fd, &len, sizeof(int));
  if (len != 0)
    sock_read(fd, value, len);
  memset(value + len, '\0', 1); // set terminator
  sock_read(fd, &ret, sizeof(int));
  /* End transmission parameters */
  ret = cli_end(fd);

  return ret;
}

int hon_system_capability_get_image_blc(char *value) {
  int fd, len;
  int ret = 0;

  fd = cli_begin((char *)__func__);
  /* Transmission parameters */
  sock_read(fd, &len, sizeof(int));
  if (len != 0)
    sock_read(fd, value, len);
  memset(value + len, '\0', 1); // set terminator
  sock_read(fd, &ret, sizeof(int));
  /* End transmission parameters */
  ret = cli_end(fd);

  return ret;
}

int hon_system_capability_get_image_enhancement(char *value) {
  int fd, len;
  int ret = 0;

  fd = cli_begin((char *)__func__);
  /* Transmission parameters */
  sock_read(fd, &len, sizeof(int));
  if (len != 0)
    sock_read(fd, value, len);
  memset(value + len, '\0', 1); // set terminator
  sock_read(fd, &ret, sizeof(int));
  /* End transmission parameters */
  ret = cli_end(fd);

  return ret;
}

int hon_system_capability_get_image_exposure(char *value) {
  int fd, len;
  int ret = 0;

  fd = cli_begin((char *)__func__);
  /* Transmission parameters */
  sock_read(fd, &len, sizeof(int));
  if (len != 0)
    sock_read(fd, value, len);
  memset(value + len, '\0', 1); // set terminator
  sock_read(fd, &ret, sizeof(int));
  /* End transmission parameters */
  ret = cli_end(fd);

  return ret;
}

int hon_system_capability_get_image_night_to_day(char *value) {
  int fd, len;
  int ret = 0;

  fd = cli_begin((char *)__func__);
  /* Transmission parameters */
  sock_read(fd, &len, sizeof(int));
  if (len != 0)
    sock_read(fd, value, len);
  memset(value + len, '\0', 1); // set terminator
  sock_read(fd, &ret, sizeof(int));
  /* End transmission parameters */
  ret = cli_end(fd);

  return ret;
}

int hon_system_capability_get_image_video_adjustment(char *value) {
  int fd, len;
  int ret = 0;

  fd = cli_begin((char *)__func__);
  /* Transmission parameters */
  sock_read(fd, &len, sizeof(int));
  if (len != 0)
    sock_read(fd, value, len);
  memset(value + len, '\0', 1); // set terminator
  sock_read(fd, &ret, sizeof(int));
  /* End transmission parameters */
  ret = cli_end(fd);

  return ret;
}

int hon_system_capability_get_image_white_blance(char *value) {
  int fd, len;
  int ret = 0;

  fd = cli_begin((char *)__func__);
  /* Transmission parameters */
  sock_read(fd, &len, sizeof(int));
  if (len != 0)
    sock_read(fd, value, len);
  memset(value + len, '\0', 1); // set terminator
  sock_read(fd, &ret, sizeof(int));
  /* End transmission parameters */
  ret = cli_end(fd);

  return ret;
}

int hon_system_get_deivce_name(char **value) {
  return hon_client_get_string((char *)__func__, value);
}

int hon_system_get_telecontrol_id(char **value) {
  return hon_client_get_string((char *)__func__, value);
}

int hon_system_get_model(char **value) {
  return hon_client_get_string((char *)__func__, value);
}

int hon_system_get_serial_number(char **value) {
  return hon_client_get_string((char *)__func__, value);
}

int hon_system_get_firmware_version(char **value) {
  return hon_client_get_string((char *)__func__, value);
}

int hon_system_get_encoder_version(char **value) {
  return hon_client_get_string((char *)__func__, value);
}

int hon_system_get_web_version(char **value) {
  return hon_client_get_string((char *)__func__, value);
}

int hon_system_get_plugin_version(char **value) {
  return hon_client_get_string((char *)__func__, value);
}

int hon_system_get_channels_number(char **value) {
  return hon_client_get_string((char *)__func__, value);
}

int hon_system_get_hard_disks_number(char **value) {
  return hon_client_get_string((char *)__func__, value);
}

int hon_system_get_alarm_inputs_number(char **value) {
  return hon_client_get_string((char *)__func__, value);
}

int hon_system_get_alarm_outputs_number(char **value) {
  return hon_client_get_string((char *)__func__, value);
}

int hon_system_get_firmware_version_info(char **value) {
  return hon_client_get_string((char *)__func__, value);
}

int hon_system_get_manufacturer(char **value) {
  return hon_client_get_string((char *)__func__, value);
}

int hon_system_get_hardware_id(char **value) {
  return hon_client_get_string((char *)__func__, value);
}

int hon_system_set_deivce_name(const char *value) {
  return hon_client_set_string((char *)__func__, value);
}

int hon_system_set_telecontrol_id(const char *value) {
  return hon_client_set_string((char *)__func__, value);
}

// action

int hon_system_reboot() {
  int fd;
  int ret = 0;

  fd = cli_begin((char *)__func__);
  /* Transmission parameters */
  sock_read(fd, &ret, sizeof(int));
  /* End transmission parameters */
  ret = cli_end(fd);

  return ret;
}

int hon_system_factory_reset() {
  int fd;
  int ret = 0;

  fd = cli_begin((char *)__func__);
  /* Transmission parameters */
  sock_read(fd, &ret, sizeof(int));
  /* End transmission parameters */
  ret = cli_end(fd);

  return ret;
}

int hon_system_export_log(const char *path) {
  return hon_client_set_string((char *)__func__, path);
}

int hon_system_export_db(const char *path) {
  return hon_client_set_string((char *)__func__, path);
}

int hon_system_import_db(const char *path) {
  return hon_client_set_string((char *)__func__, path);
}

int hon_system_upgrade(const char *path) {
  return hon_client_set_string((char *)__func__, path);
}

// user
int hon_system_get_user_details(char *value, const char* sql_query) {
    // printf("\n============comes in %s in client.cpp============\n",__func__);


    int fd, value_len, param_len;
    int ret = 0;

    fd = cli_begin((char *)__func__);
    /* Transmission parameters */
    // param_len = strlen(sql_query)+1;
    param_len = strlen(sql_query);
    
    if (sock_write(fd, &param_len, sizeof(param_len)) == SOCKERR_CLOSED) {
  		return -1;
    }

    if (sock_write(fd, sql_query, param_len) == SOCKERR_CLOSED) {
      return -1;
    }
    
    sock_read(fd, &value_len, sizeof(int));
    if (value_len != 0) {
      sock_read(fd, value, value_len);
    }

    memset(value + value_len, '\0', 1); // set terminator
    
    sock_read(fd, &ret, sizeof(int));
    /* End transmission parameters */
    ret = cli_end(fd);
    return ret;
}
int hon_system_check_user_exists(const char* username, char*password,int* userlevel,bool need_password_userlevel) {
  int fd, user_name_len, password_len;
  int ret = 0;
//    Logger& logger = Logger::getInstance();
//  logger.log(Logger::INFO,"\n============in function hon_system_check_user_exists client side============\n");
  fd = cli_begin((char *)__func__);
  /* Transmission parameters */

// logger.log(Logger::INFO,"\n============in function hon_system_check_user_exists client side need_password_userlevel============\n",need_password_userlevel);
  sock_write(fd,&need_password_userlevel, sizeof(bool));
  user_name_len = strlen(username) + 1;
  if (sock_write(fd, &user_name_len, sizeof(user_name_len)) == SOCKERR_CLOSED) {
  		return -1;
    }
    if (sock_write(fd, username, user_name_len) == SOCKERR_CLOSED) {
  		return -1;
    }
    // logger.log(Logger::INFO,"\n============in function hon_system_check_user_exists client side username============\n",username);
  if(need_password_userlevel)
  {
    sock_read(fd, &password_len, sizeof(int));
    if (password_len != 0) {
      sock_read(fd, password, password_len);
    }

    memset(password + password_len, '\0', 1); // set terminator
    // logger.log(Logger::INFO,"\n============in function hon_system_check_user_exists client side paswword============\n",password);
    sock_read(fd, userlevel, sizeof(int));
    // logger.log(Logger::INFO,"\n============in function hon_system_check_user_exists client side userlevel============\n",userlevel);
  }
  sock_read(fd, &ret, sizeof(int));
  // logger.log(Logger::INFO,"\n============in function hon_system_check_user_exists client side ret============\n",ret);
  /* End transmission parameters */
  cli_end(fd);
  return ret;
}

int hon_system_verify_security_ans(const char* username, char*password,char *security_ans,char *security_que) {
  int fd, user_name_len, password_len,security_ans_len,security_que_len;
  int ret = 0;
//    Logger& logger = Logger::getInstance();
//  logger.log(Logger::INFO,"\n============in function hon_system_check_user_exists client side============\n");
  fd = cli_begin((char *)__func__);
  /* Transmission parameters */

  user_name_len = strlen(username) + 1;
  if (sock_write(fd, &user_name_len, sizeof(user_name_len)) == SOCKERR_CLOSED) {
  		return -1;
    }
    if (sock_write(fd, username, user_name_len) == SOCKERR_CLOSED) {
  		return -1;
    }
     password_len = strlen(password) + 1;
  if (sock_write(fd, &password_len, sizeof(password_len)) == SOCKERR_CLOSED) {
  		return -1;
    }
    if (sock_write(fd, password, password_len) == SOCKERR_CLOSED) {
  		return -1;
    }
    security_ans_len = strlen(security_ans) + 1;
  if (sock_write(fd, &security_ans_len, sizeof(security_ans_len)) == SOCKERR_CLOSED) {
  		return -1;
    }
    if (sock_write(fd, security_ans, security_ans_len) == SOCKERR_CLOSED) {
  		return -1;
    }
    security_que_len = strlen(security_que) + 1;
  if (sock_write(fd, &security_que_len, sizeof(security_que_len)) == SOCKERR_CLOSED) {
  		return -1;
    }
    if (sock_write(fd, security_que, security_que_len) == SOCKERR_CLOSED) {
  		return -1;
    }
  sock_read(fd, &ret, sizeof(int));
  // logger.log(Logger::INFO,"\n============in function hon_system_check_user_exists client side ret============\n",ret);
  /* End transmission parameters */
  cli_end(fd);
  return ret;
}

int hon_system_get_user_num(int *value) {
  return hon_client_get_int((char *)__func__, value);
}

int hon_system_manage_user_record(char * sql_query) {
  

    int fd, param_len;
    int ret = 0, err = 0;

    fd = cli_begin((char *)__func__);
    /* Transmission parameters */
    param_len = strlen(sql_query)+1;
    // param_len = strlen(sql_query);
    
    if (sock_write(fd, &param_len, sizeof(param_len)) == SOCKERR_CLOSED) {
  		return -1;
    }

    if (sock_write(fd, sql_query, param_len) == SOCKERR_CLOSED) {
      return -1;
    }
    
    sock_read(fd, &err, sizeof(int));
    
    /* End transmission parameters */
    ret = cli_end(fd);

    // logger.log(Logger::INFO,"end func",__func__, ret);
    
    if (ret == 0) {
      return err;
    }
    
    return ret;
}
int hon_system_record_exists(char * sql_query) {
  

    int fd, param_len;
    int ret = 0, err = 0;

    fd = cli_begin((char *)__func__);
    /* Transmission parameters */
    param_len = strlen(sql_query)+1;
    // param_len = strlen(sql_query);
    
    if (sock_write(fd, &param_len, sizeof(param_len)) == SOCKERR_CLOSED) {
  		return -1;
    }

    if (sock_write(fd, sql_query, param_len) == SOCKERR_CLOSED) {
      return -1;
    }
    
    sock_read(fd, &err, sizeof(int));
    
    /* End transmission parameters */
    ret = cli_end(fd);

    // logger.log(Logger::INFO,"end func",__func__, ret);
    
    if (ret == 0) {
      return err;
    }
    
    return ret;
}

// osd
// osd.common
int hon_osd_get_is_presistent_text(int *value) {
  return hon_client_get_int((char *)__func__, value);
}

int hon_osd_set_is_presistent_text(int value) {
  return hon_client_set_int((char *)__func__, value);
}

int hon_osd_get_font_size(int *value) {
  return hon_client_get_int((char *)__func__, value);
}

int hon_osd_set_font_size(int value) {
  return hon_client_set_int((char *)__func__, value);
}

int hon_osd_get_boundary(int *value) {
  return hon_client_get_int((char *)__func__, value);
}

int hon_osd_set_boundary(int value) {
  return hon_client_set_int((char *)__func__, value);
}

int hon_osd_get_normalized_screen_width(int *value) {
  return hon_client_get_int((char *)__func__, value);
}

int hon_osd_get_normalized_screen_height(int *value) {
  return hon_client_get_int((char *)__func__, value);
}

int hon_osd_get_attribute(char **value) {
  return hon_client_get_string((char *)__func__, value);
}

int hon_osd_set_attribute(const char *value) {
  return hon_client_set_string((char *)__func__, value);
}

int hon_osd_get_font_color_mode(char **value) {
  return hon_client_get_string((char *)__func__, value);
}

int hon_osd_set_font_color_mode(const char *value) {
  return hon_client_set_string((char *)__func__, value);
}

int hon_osd_get_font_color(char **value) {
  return hon_client_get_string((char *)__func__, value);
}

int hon_osd_set_font_color(const char *value) {
  return hon_client_set_string((char *)__func__, value);
}

int hon_osd_get_alignment(char **value) {
  return hon_client_get_string((char *)__func__, value);
}

int hon_osd_set_alignment(const char *value) {
  return hon_client_set_string((char *)__func__, value);
}

int hon_osd_get_font_path(char **value) {
  return hon_client_get_string((char *)__func__, value);
}

int hon_osd_set_font_path(const char *value) {
  return hon_client_set_string((char *)__func__, value);
}

int hon_osd_set_font_color_of_channelName(const char *value) {
  return hon_client_set_string((char *)__func__, value);
}

int hon_osd_set_font_color_of_dateTime(const char *value) {
  return hon_client_set_string((char *)__func__, value);
}

int hon_osd_set_font_color_of_channelLocation(const char *value) {
  return hon_client_set_string((char *)__func__, value);
}

int hon_osd_get_font_color_of_channelName(char **value) {
  return hon_client_get_string((char *)__func__, value);
}

int hon_osd_get_font_color_of_dateTime(char **value) {
  return hon_client_get_string((char *)__func__, value);
}

int hon_osd_get_font_color_of_channelLocation(char **value) {
  return hon_client_get_string((char *)__func__, value);
}

// osd.x
int hon_osd_get_enabled(int id, int *value) {
  return hon_client_get_int_by_id((char *)__func__, id, value);
}

int hon_osd_set_enabled(int id, int value) {
  return hon_client_set_int_by_id((char *)__func__, id, value);
}

int hon_osd_get_position_x(int id, int *value) {
  return hon_client_get_int_by_id((char *)__func__, id, value);
}

int hon_osd_set_position_x(int id, int value) {
  return hon_client_set_int_by_id((char *)__func__, id, value);
}

int hon_osd_get_position_y(int id, int *value) {
  return hon_client_get_int_by_id((char *)__func__, id, value);
}

int hon_osd_set_position_y(int id, int value) {
  return hon_client_set_int_by_id((char *)__func__, id, value);
}

int hon_osd_get_height(int id, int *value) {
  return hon_client_get_int_by_id((char *)__func__, id, value);
}

int hon_osd_set_height(int id, int value) {
  return hon_client_set_int_by_id((char *)__func__, id, value);
}

int hon_osd_get_width(int id, int *value) {
  return hon_client_get_int_by_id((char *)__func__, id, value);
}

int hon_osd_set_width(int id, int value) {
  return hon_client_set_int_by_id((char *)__func__, id, value);
}

int hon_osd_get_display_week_enabled(int id, int *value) {
  return hon_client_get_int_by_id((char *)__func__, id, value);
}

int hon_osd_set_display_week_enabled(int id, int value) {
  return hon_client_set_int_by_id((char *)__func__, id, value);
}

int hon_osd_get_date_style(int id, char **value) {
  return hon_client_get_string_by_id((char *)__func__, id, value);
}

int hon_osd_set_date_style(int id, const char *value) {
  return hon_client_set_string_by_id((char *)__func__, id, value);
}

int hon_osd_get_time_style(int id, char **value) {
  return hon_client_get_string_by_id((char *)__func__, id, value);
}

int hon_osd_set_time_style(int id, const char *value) {
  return hon_client_set_string_by_id((char *)__func__, id, value);
}

int hon_osd_get_type(int id, char **value) {
  return hon_client_get_string_by_id((char *)__func__, id, value);
}

int hon_osd_set_type(int id, const char *value) {
  return hon_client_set_string_by_id((char *)__func__, id, value);
}

int hon_osd_get_display_text(int id, char **value) {
  return hon_client_get_string_by_id((char *)__func__, id, value);
}

int hon_osd_set_display_text(int id, const char *value) {
  return hon_client_set_string_by_id((char *)__func__, id, value);
}

int hon_osd_get_image_path(int id, char **value) {
  return hon_client_get_string_by_id((char *)__func__, id, value);
}

int hon_osd_set_image_path(int id, const char *value) {
  return hon_client_set_string_by_id((char *)__func__, id, value);
}

int hon_osd_restart() {
  int fd;
  int ret = 0;

  fd = cli_begin((char *)__func__);
  /* Transmission parameters */
  sock_read(fd, &ret, sizeof(int));
  /* End transmission parameters */
  ret = cli_end(fd);

  return ret;
}

// roi.x
int hon_roi_get_stream_type(int id, char **value) {
  return hon_client_get_string_by_id((char *)__func__, id, value);
}

int hon_roi_set_stream_type(int id, const char *value) {
  return hon_client_set_string_by_id((char *)__func__, id, value);
}

int hon_roi_get_name(int id, char **value) {
  return hon_client_get_string_by_id((char *)__func__, id, value);
}

int hon_roi_set_name(int id, const char *value) {
  return hon_client_set_string_by_id((char *)__func__, id, value);
}

int hon_roi_get_id(int id, int *value) {
  return hon_client_get_int_by_id((char *)__func__, id, value);
}

int hon_roi_set_id(int id, int value) {
  return hon_client_set_int_by_id((char *)__func__, id, value);
}

int hon_roi_get_enabled(int id, int *value) {
  return hon_client_get_int_by_id((char *)__func__, id, value);
}

int hon_roi_set_enabled(int id, int value) {
  return hon_client_set_int_by_id((char *)__func__, id, value);
}

int hon_roi_get_position_x(int id, int *value) {
  return hon_client_get_int_by_id((char *)__func__, id, value);
}

int hon_roi_set_position_x(int id, int value) {
  return hon_client_set_int_by_id((char *)__func__, id, value);
}

int hon_roi_get_position_y(int id, int *value) {
  return hon_client_get_int_by_id((char *)__func__, id, value);
}

int hon_roi_set_position_y(int id, int value) {
  return hon_client_set_int_by_id((char *)__func__, id, value);
}

int hon_roi_get_height(int id, int *value) {
  return hon_client_get_int_by_id((char *)__func__, id, value);
}

int hon_roi_set_height(int id, int value) {
  return hon_client_set_int_by_id((char *)__func__, id, value);
}

int hon_roi_get_width(int id, int *value) {
  return hon_client_get_int_by_id((char *)__func__, id, value);
}

int hon_roi_set_width(int id, int value) {
  return hon_client_set_int_by_id((char *)__func__, id, value);
}

int hon_roi_get_quality_level(int id, int *value) {
  return hon_client_get_int_by_id((char *)__func__, id, value);
}

int hon_roi_set_quality_level(int id, int value) {
  return hon_client_set_int_by_id((char *)__func__, id, value);
}

int hon_roi_set_all() {
  int fd;
  int ret = 0;

  fd = cli_begin((char *)__func__);
  /* Transmission parameters */
  sock_read(fd, &ret, sizeof(int));
  /* End transmission parameters */
  ret = cli_end(fd);

  return ret;
}

// region_clip.x
int hon_region_clip_get_enabled(int id, int *value) {
  return hon_client_get_int_by_id((char *)__func__, id, value);
}

int hon_region_clip_set_enabled(int id, int value) {
  return hon_client_set_int_by_id((char *)__func__, id, value);
}

int hon_region_clip_get_position_x(int id, int *value) {
  return hon_client_get_int_by_id((char *)__func__, id, value);
}

int hon_region_clip_set_position_x(int id, int value) {
  return hon_client_set_int_by_id((char *)__func__, id, value);
}

int hon_region_clip_get_position_y(int id, int *value) {
  return hon_client_get_int_by_id((char *)__func__, id, value);
}

int hon_region_clip_set_position_y(int id, int value) {
  return hon_client_set_int_by_id((char *)__func__, id, value);
}

int hon_region_clip_get_height(int id, int *value) {
  return hon_client_get_int_by_id((char *)__func__, id, value);
}

int hon_region_clip_set_height(int id, int value) {
  return hon_client_set_int_by_id((char *)__func__, id, value);
}

int hon_region_clip_get_width(int id, int *value) {
  return hon_client_get_int_by_id((char *)__func__, id, value);
}

int hon_region_clip_set_width(int id, int value) {
  return hon_client_set_int_by_id((char *)__func__, id, value);
}

int hon_region_clip_set_all() {
  int fd;
  int ret = 0;

  fd = cli_begin((char *)__func__);
  /* Transmission parameters */
  sock_read(fd, &ret, sizeof(int));
  /* End transmission parameters */
  ret = cli_end(fd);

  return ret;
}

// network
int hon_network_ipv4_get(const char *interface, char *method, char *address,
                        char *netmask, char *gateway) {
  int fd, len;
  int ret = 0;

  fd = cli_begin((char *)__func__);
  /* Transmission parameters */
  // write
  len = strlen(interface) + 1;
  sock_write(fd, &len, sizeof(int));
  sock_write(fd, interface, len);
  // read
  sock_read(fd, &len, sizeof(int));
  sock_read(fd, method, len);
  memset(method + len, '\0', 1); // set terminator
  sock_read(fd, &len, sizeof(int));
  sock_read(fd, address, len);
  memset(address + len, '\0', 1); // set terminator
  sock_read(fd, &len, sizeof(int));
  sock_read(fd, netmask, len);
  memset(netmask + len, '\0', 1); // set terminator
  sock_read(fd, &len, sizeof(int));
  sock_read(fd, gateway, len);
  memset(gateway + len, '\0', 1); // set terminator

  sock_read(fd, &ret, sizeof(int));
  /* End transmission parameters */
  ret = cli_end(fd);

  return ret;
}

int hon_network_ipv4_set(char *interface, char *method, char *address,
                        char *netmask, char *gateway) {
  int fd, len;
  int ret = 0;

  fd = cli_begin((char *)__func__);
  /* Transmission parameters */
  // write
  len = strlen(interface) + 1;
  sock_write(fd, &len, sizeof(int));
  sock_write(fd, interface, len);
  if (method) {
    len = strlen(method) + 1;
    sock_write(fd, &len, sizeof(int));
    sock_write(fd, method, len);
  } else {
    len = 0;
    sock_write(fd, &len, sizeof(int));
  }
  if (address) {
    len = strlen(address) + 1;
    sock_write(fd, &len, sizeof(int));
    sock_write(fd, address, len);
  } else {
    len = 0;
    sock_write(fd, &len, sizeof(int));
  }
  if (netmask) {
    len = strlen(netmask) + 1;
    sock_write(fd, &len, sizeof(int));
    sock_write(fd, netmask, len);
  } else {
    len = 0;
    sock_write(fd, &len, sizeof(int));
  }
  if (gateway) {
    len = strlen(gateway) + 1;
    sock_write(fd, &len, sizeof(int));
    sock_write(fd, gateway, len);
  } else {
    len = 0;
    sock_write(fd, &len, sizeof(int));
  }
  // read
  sock_read(fd, &ret, sizeof(int));
  /* End transmission parameters */
  ret = cli_end(fd);

  return ret;
}

// int hon_network_ipv4_set_para(char *interface, char *method, char *address,
//                         char *netmask, char *gateway) {
//   int fd, len;
//   int ret = 0;

//   fd = cli_begin((char *)__func__);
//   /* Transmission parameters */
//   // write
//   len = strlen(interface) + 1;
//   sock_write(fd, &len, sizeof(int));
//   sock_write(fd, interface, len);
//   if (method) {
//     len = strlen(method) + 1;
//     sock_write(fd, &len, sizeof(int));
//     sock_write(fd, method, len);
//   } else {
//     len = 0;
//     sock_write(fd, &len, sizeof(int));
//   }
//   if (address) {
//     len = strlen(address) + 1;
//     sock_write(fd, &len, sizeof(int));
//     sock_write(fd, address, len);
//   } else {
//     len = 0;
//     sock_write(fd, &len, sizeof(int));
//   }
//   if (netmask) {
//     len = strlen(netmask) + 1;
//     sock_write(fd, &len, sizeof(int));
//     sock_write(fd, netmask, len);
//   } else {
//     len = 0;
//     sock_write(fd, &len, sizeof(int));
//   }
//   if (gateway) {
//     len = strlen(gateway) + 1;
//     sock_write(fd, &len, sizeof(int));
//     sock_write(fd, gateway, len);
//   } else {
//     len = 0;
//     sock_write(fd, &len, sizeof(int));
//   }
//   // read
//   sock_read(fd, &ret, sizeof(int));
//   /* End transmission parameters */
//   ret = cli_end(fd);

//   return ret;
// }


int hon_network_httpPort_get(int *httpState, int *httpPort) 
{
  int fd, len;
  int ret = 0;

  fd = cli_begin((char *)__func__);
  /* Transmission parameters */
  // read
  sock_read(fd, &len, sizeof(int));
  sock_read(fd, httpState, len);
  memset(httpState + len, '\0', 1); // set terminator
  sock_read(fd, &len, sizeof(int));
  sock_read(fd, httpPort, len);
  memset(httpPort + len, '\0', 1); // set terminator

  sock_read(fd, &ret, sizeof(int));
  /* End transmission parameters */
  ret = cli_end(fd);

  return ret;
}

int hon_network_httpsPort_get(int *httpsState, int *httpsPort) 
{
  int fd, len;
  int ret = 0;

  fd = cli_begin((char *)__func__);
  /* Transmission parameters */
  // read
  sock_read(fd, &len, sizeof(int));
  sock_read(fd, httpsState, len);
  memset(httpsState + len, '\0', 1); // set terminator
  sock_read(fd, &len, sizeof(int));
  sock_read(fd, httpsPort, len);
  memset(httpsPort + len, '\0', 1); // set terminator

  sock_read(fd, &ret, sizeof(int));
  /* End transmission parameters */
  ret = cli_end(fd);

  return ret;
}

bool isValidIp4(char* str)
{
    int32_t segs = 0;  /* Segment count. */
    int32_t chcnt = 0; /* Character count within segment. */
    int32_t accum = 0; /* Accumulator for segment. */
    /* Catch NULL pointer. */
    if (str == NULL)
        return false;
    /* Process every character in string. */
    while (*str != '\0')
    {
        /* Segment changeover. */
        if (*str == '.')
        {
            /* Must have some digits in segment. */
            if (chcnt == 0)
                return false;
            /* Limit number of segments. */
            if (++segs == 4)
                return false;
            /* Reset segment values and restart loop. */
            chcnt = accum = 0;
            str++;
            continue;
        }

        /* Check numeric. */
        if ((*str < '0') || (*str > '9'))
            return false;
        /* Accumulate and check segment. */
        if ((accum = accum * 10 + *str - '0') > 255)
            return false;
        /* Advance other segment specific stuff and continue loop. */
        chcnt++;
        str++;
    }
    /* Check enough segments and enough characters in last segment. */
    if (segs != 3)
        return false;
    if (chcnt == 0)
        return false;
    /* Address okay. */
    return true;
}







int hon_network_dns_get(char *ethernet_v4_dns, char *ethernet_v4_dns2) {
  int fd, len;
  int ret = 0;

  fd = cli_begin((char *)__func__);
  /* Transmission parameters */
  // read
  sock_read(fd, &len, sizeof(int));
  sock_read(fd, ethernet_v4_dns, len);
  memset(ethernet_v4_dns + len, '\0', 1); // set terminator
  sock_read(fd, &len, sizeof(int));
  sock_read(fd, ethernet_v4_dns2, len);
  memset(ethernet_v4_dns2 + len, '\0', 1); // set terminator

  sock_read(fd, &ret, sizeof(int));
  /* End transmission parameters */
  ret = cli_end(fd);

  return ret;
}

int hon_network_dns_set(char *dns1, char *dns2) {
  int fd, len;
  int ret = 0;

  fd = cli_begin((char *)__func__);
  /* Transmission parameters */
  // write
  len = strlen(dns1) + 1;
  sock_write(fd, &len, sizeof(int));
  sock_write(fd, dns1, len);
  len = strlen(dns2) + 1;
  sock_write(fd, &len, sizeof(int));
  sock_write(fd, dns2, len);

  sock_read(fd, &ret, sizeof(int));
  /* End transmission parameters */
  ret = cli_end(fd);

  return ret;
}

int hon_network_get_mac(const char *ifname, char *mac) {
  int fd, len;
  int ret = 0;

  fd = cli_begin((char *)__func__);
  /* Transmission parameters */
  // write
  len = strlen(ifname) + 1;
  sock_write(fd, &len, sizeof(int));
  sock_write(fd, ifname, len);
  // read
  sock_read(fd, &len, sizeof(int));
  sock_read(fd, mac, len);
  memset(mac + len, '\0', 1); // set terminator

  sock_read(fd, &ret, sizeof(int));
  /* End transmission parameters */
  ret = cli_end(fd);

  return ret;
}

int hon_network_nicspeed_get(const char *ifname, int *speed, int *duplex,
                            int *autoneg) {
  int fd, len;
  int ret = 0;

  fd = cli_begin((char *)__func__);
  /* Transmission parameters */
  // write
  len = strlen(ifname) + 1;
  sock_write(fd, &len, sizeof(int));
  sock_write(fd, ifname, len);
  // read
  sock_read(fd, speed, sizeof(int));
  sock_read(fd, duplex, sizeof(int));
  sock_read(fd, autoneg, sizeof(int));

  sock_read(fd, &ret, sizeof(int));
  /* End transmission parameters */
  ret = cli_end(fd);

  return ret;
}

int hon_network_nicspeed_set(const char *ifname, int speed, int duplex,
                            int autoneg) {
  int fd, len;
  int ret = 0;

  fd = cli_begin((char *)__func__);
  /* Transmission parameters */
  // write
  len = strlen(ifname) + 1;
  sock_write(fd, &len, sizeof(int));
  sock_write(fd, ifname, len);
  sock_write(fd, &speed, sizeof(int));
  sock_write(fd, &duplex, sizeof(int));
  sock_write(fd, &autoneg, sizeof(int));
  // read
  sock_read(fd, &ret, sizeof(int));
  /* End transmission parameters */
  ret = cli_end(fd);

  return ret;
}

int hon_network_nicspeed_support_get(const char *ifname, char *nic_supported) {
  int fd, len;
  int ret = 0;

  fd = cli_begin((char *)__func__);
  /* Transmission parameters */
  // write
  len = strlen(ifname) + 1;
  sock_write(fd, &len, sizeof(int));
  sock_write(fd, ifname, len);
  // read
  sock_read(fd, &len, sizeof(int));
  sock_read(fd, nic_supported, len);
  memset(nic_supported + len, '\0', 1); // set terminator
  sock_read(fd, &ret, sizeof(int));
  /* End transmission parameters */
  ret = cli_end(fd);

  return ret;
}

int hon_wifi_power_get(int *on) {
  int fd;
  int ret = 0;

  fd = cli_begin((char *)__func__);
  /* Transmission parameters */
  // read
  sock_read(fd, on, sizeof(int));
  sock_read(fd, &ret, sizeof(int));
  /* End transmission parameters */
  ret = cli_end(fd);

  return ret;
}

int hon_wifi_power_set(int on) {
  int fd;
  int ret = 0;

  fd = cli_begin((char *)__func__);
  /* Transmission parameters */
  // write
  sock_write(fd, &on, sizeof(int));
  // read
  sock_read(fd, &ret, sizeof(int));
  /* End transmission parameters */
  ret = cli_end(fd);

  return ret;
}

int hon_wifi_scan_wifi() {
  int fd;
  int ret = 0;

  fd = cli_begin((char *)__func__);
  /* Transmission parameters */
  sock_read(fd, &ret, sizeof(int));
  /* End transmission parameters */
  ret = cli_end(fd);

  return ret;
}

int hon_wifi_get_list(char **wifi_list) {
  int fd, len;
  int ret = 0;

  fd = cli_begin((char *)__func__);
  /* Transmission parameters */
  // read
  sock_read(fd, &len, sizeof(int));
  *wifi_list = (char *)malloc(len + 1);
  sock_read(fd, *wifi_list, len);
  memset(*wifi_list + len, '\0', 1); // set terminator
  sock_read(fd, &ret, sizeof(int));
  /* End transmission parameters */
  ret = cli_end(fd);

  return ret;
}

int hon_wifi_connect_with_ssid(const char *ssid, const char *psk) {
  int fd, len;
  int ret = 0;

  fd = cli_begin((char *)__func__);
  /* Transmission parameters */
  // write
  len = strlen(ssid) + 1;
  sock_write(fd, &len, sizeof(int));
  sock_write(fd, ssid, len);
  len = strlen(psk) + 1;
  sock_write(fd, &len, sizeof(int));
  sock_write(fd, psk, len);
  // read
  sock_read(fd, &ret, sizeof(int));
  /* End transmission parameters */
  ret = cli_end(fd);

  return ret;
}
int hon_wifi_forget_with_ssid(const char *ssid) {
  int fd, len;
  int ret = 0;

  fd = cli_begin((char *)__func__);
  /* Transmission parameters */
  // write
  len = strlen(ssid) + 1;
  sock_write(fd, &len, sizeof(int));
  sock_write(fd, ssid, len);
  // read
  sock_read(fd, &ret, sizeof(int));
  /* End transmission parameters */
  ret = cli_end(fd);

  return ret;
}

// storage
int hon_storage_record_start() {
  int fd;
  int ret = 0;

  fd = cli_begin((char *)__func__);
  /* Transmission parameters */
  sock_read(fd, &ret, sizeof(int));
  /* End transmission parameters */
  ret = cli_end(fd);

  return ret;
}

int hon_storage_record_stop() {
  int fd;
  int ret = 0;

  fd = cli_begin((char *)__func__);
  /* Transmission parameters */
  sock_read(fd, &ret, sizeof(int));
  /* End transmission parameters */
  ret = cli_end(fd);

  return ret;
}

int hon_storage_record_statue_get(int *value) {
  return hon_client_get_int((char *)__func__, value);
}

int hon_take_photo() {
  int fd;
  int ret = 0;

  fd = cli_begin((char *)__func__);
  /* Transmission parameters */
  sock_read(fd, &ret, sizeof(int));
  /* End transmission parameters */
  ret = cli_end(fd);

  return ret;
}

int hon_storage_get_schedule_plan(char *value, const char* sql_query) {
  // printf("\n============comes in %s in client.cpp============\n",__func__);


  int fd, value_len, param_len;
  int ret = 0;

  fd = cli_begin((char *)__func__);
  /* Transmission parameters */
  // param_len = strlen(sql_query)+1;
  param_len = strlen(sql_query);
  
  if (sock_write(fd, &param_len, sizeof(param_len)) == SOCKERR_CLOSED) {
    return -1;
  }

  if (sock_write(fd, sql_query, param_len) == SOCKERR_CLOSED) {
    return -1;
  }
  
  sock_read(fd, &value_len, sizeof(int));
  if (value_len != 0) {
    sock_read(fd, value, value_len);
  }

  memset(value + value_len, '\0', 1); // set terminator
  
  sock_read(fd, &ret, sizeof(int));
  /* End transmission parameters */
  ret = cli_end(fd);
  return ret;
}


//Set Schedule recording plan
int hon_storage_set_schedule_plan(const char *sql_query) {
  int fd, param_len;
  int ret = 0;

  fd = cli_begin((char *)__func__);
  /* Transmission parameters */
  param_len = strlen(sql_query) + 1;
  
  if (sock_write(fd, &param_len, sizeof(param_len)) == SOCKERR_CLOSED) {
    return -1;
  }

  if (sock_write(fd, sql_query, param_len) == SOCKERR_CLOSED) {
    return -1;
  }
  
  sock_read(fd, &ret, sizeof(int));
  /* End transmission parameters */
  ret = cli_end(fd);
  
  return ret;
}

int hon_storage_get_recordings(char **value, const char* sql_query) {
  int fd, value_len, param_len;
  int ret = 0;

  // Begin the client connection
  fd = cli_begin((char *)__func__);
  if (fd < 0) {
      // LOG_ERROR("Failed to begin client connection\n");
      return -1;
  }

  /* Transmission parameters */
  param_len = strlen(sql_query); // Get the length of the SQL query

  // Send the length of the SQL query
  if (sock_write(fd, &param_len, sizeof(param_len)) == SOCKERR_CLOSED) {
      cli_end(fd);
      return -1;
  }

  // Send the SQL query itself
  if (sock_write(fd, sql_query, param_len) == SOCKERR_CLOSED) {
      cli_end(fd);
      return -1;
  }

  // Read the length of the value (JSON string)
  if (sock_read(fd, &value_len, sizeof(int)) == SOCKERR_CLOSED) {
      cli_end(fd);
      return -1;
  }

  // Allocate memory for the value buffer
  *value = (char *)malloc(value_len + 1); // +1 for null terminator
  if (*value == NULL) {
      // LOG_ERROR("Memory allocation failed for value\n");
      cli_end(fd);
      return -1;
  }

  // Read the value (JSON string)
  if (sock_read(fd, *value, value_len) == SOCKERR_CLOSED) {
      free(*value);
      cli_end(fd);
      return -1;
  }
  (*value)[value_len] = '\0'; // Null-terminate the string

  // Read the return code
  if (sock_read(fd, &ret, sizeof(int)) == SOCKERR_CLOSED) {
      free(*value);
      cli_end(fd);
      return -1;
  }

  /* End transmission parameters */
  ret = cli_end(fd);

  return ret;
}

int hon_storage_get_sdcard_size(float *total_size, float *free_size, int *mount_status, int *free_size_mb) {
  int fd;
  int ret = 0;

  fd = cli_begin((char *)__func__);
  /* Transmission parameters */

  // Read the total size
  if (sock_read(fd, total_size, sizeof(int)) == SOCKERR_CLOSED) {
      cli_end(fd);
      return -1;
  }

  // Read the free size
  if (sock_read(fd, free_size, sizeof(int)) == SOCKERR_CLOSED) {
      cli_end(fd);
      return -1;
  }

  // Read the mount status
  if (sock_read(fd, mount_status, sizeof(int)) == SOCKERR_CLOSED) {
      cli_end(fd);
      return -1;
  }

  // Read the free size in MB
  if (sock_read(fd, free_size_mb, sizeof(int)) == SOCKERR_CLOSED) {
      cli_end(fd);
      return -1;
  }

  // Read the return code
  if (sock_read(fd, &ret, sizeof(int)) == SOCKERR_CLOSED) {
      cli_end(fd);
      return -1;
  }

  /* End transmission parameters */
  ret = cli_end(fd);

  return ret;
}

int hon_storage_get_recording_channel(int *recording_channel) {
  int fd;
  int ret = 0;

  fd = cli_begin((char *)__func__);
  /* Transmission parameters */

  // Read the recording channel
  if (sock_read(fd, recording_channel, sizeof(int)) == SOCKERR_CLOSED) {
      cli_end(fd);
      return -1;
  }

  // Read the return code
  if (sock_read(fd, &ret, sizeof(int)) == SOCKERR_CLOSED) {
      cli_end(fd);
      return -1;
  }

  /* End transmission parameters */
  ret = cli_end(fd);

  return ret;
}

int hon_storage_set_recording_channel(int value) {
  return hon_client_set_int((char *)__func__, value);
}

int hon_storage_set_FIFO_status(int value) {
  return hon_client_set_int((char *)__func__, value);
}

int hon_storage_set_free_size_mb(int free_size_mb) {
  return hon_client_set_int((char *)__func__, free_size_mb);
}

int hon_storage_get_recording_type(char **recording_type) {
  int fd;
  int ret = 0;
  int len = 0;

  fd = cli_begin((char *)__func__);
  /* Transmission parameters */

  // Read the length of the recording type string
  if (sock_read(fd, &len, sizeof(len)) == SOCKERR_CLOSED) {
      cli_end(fd);
      return -1;
  }

  // Allocate memory for the recording type string and read it
  *recording_type = (char *)malloc(len + 1);
  if (*recording_type == NULL) {
      // LOG_ERROR("Memory allocation failed\n");
      cli_end(fd);
      return -1;
  }
  if (sock_read(fd, *recording_type, len) == SOCKERR_CLOSED) {
      free(*recording_type);
      cli_end(fd);
      return -1;
  }
  (*recording_type)[len] = '\0'; // Null-terminate the string

  // Read the return code
  if (sock_read(fd, &ret, sizeof(int)) == SOCKERR_CLOSED) {
      free(*recording_type);
      cli_end(fd);
      return -1;
  }

  /* End transmission parameters */
  ret = cli_end(fd);

  return ret;
}

int hon_storage_set_recording_type(const char *value){
  return hon_client_set_string((char *)__func__, value);
}

int hon_storage_get_FIFO_status(int *value) {
  int fd;
  int ret = 0;

  fd = cli_begin((char *)__func__);
  /* Transmission parameters */

  // Read the FIFO status
  if (sock_read(fd, value, sizeof(int)) == SOCKERR_CLOSED) {
      cli_end(fd);
      return -1;
  }

  // Read the return code
  if (sock_read(fd, &ret, sizeof(int)) == SOCKERR_CLOSED) {
      cli_end(fd);
      return -1;
  }

  /* End transmission parameters */
  ret = cli_end(fd);

  return ret;
}

int hon_storage_format_sd_card(int value) {
  return hon_client_set_int((char *)__func__, value);
}

// event
int hon_event_ri_get_enabled(int *value) {
  return hon_client_get_int((char *)__func__, value);
}

int hon_event_ri_set_enabled(int value) {
  return hon_client_set_int((char *)__func__, value);
}

int hon_event_ri_get_position_x(int *value) {
  return hon_client_get_int((char *)__func__, value);
}

int hon_event_ri_set_position_x(int value) {
  return hon_client_set_int((char *)__func__, value);
}

int hon_event_ri_get_position_y(int *value) {
  return hon_client_get_int((char *)__func__, value);
}

int hon_event_ri_set_position_y(int value) {
  return hon_client_set_int((char *)__func__, value);
}

int hon_event_ri_get_width(int *value) {
  return hon_client_get_int((char *)__func__, value);
}

int hon_event_ri_set_width(int value) {
  return hon_client_set_int((char *)__func__, value);
}

int hon_event_ri_get_height(int *value) {
  return hon_client_get_int((char *)__func__, value);
}

int hon_event_ri_set_height(int value) {
  return hon_client_set_int((char *)__func__, value);
}

int hon_event_ri_get_proportion(int *value) {
  return hon_client_get_int((char *)__func__, value);
}

int hon_event_ri_set_proportion(int value) {
  return hon_client_set_int((char *)__func__, value);
}

int hon_event_ri_get_sensitivity_level(int *value) {
  return hon_client_get_int((char *)__func__, value);
}

int hon_event_ri_set_sensitivity_level(int value) {
  return hon_client_set_int((char *)__func__, value);
}

int hon_event_ri_get_time_threshold(int *value) {
  return hon_client_get_int((char *)__func__, value);
}

int hon_event_ri_set_time_threshold(int value) {
  return hon_client_set_int((char *)__func__, value);
}

//Analytics
int hon_analytics_find_records(char *value, const char* sql_query) {
    // printf("\n============comes in %s in client.cpp============\n",__func__);


    int fd, value_len, param_len;
    int ret = 0;

    fd = cli_begin((char *)__func__);
    /* Transmission parameters */
    // param_len = strlen(sql_query)+1;
    param_len = strlen(sql_query);
    
    if (sock_write(fd, &param_len, sizeof(param_len)) == SOCKERR_CLOSED) {
  		return -1;
    }

    if (sock_write(fd, sql_query, param_len) == SOCKERR_CLOSED) {
      return -1;
    }
    
    sock_read(fd, &value_len, sizeof(int));
    if (value_len != 0) {
      sock_read(fd, value, value_len);
    }

    memset(value + value_len, '\0', 1); // set terminator
    
    sock_read(fd, &ret, sizeof(int));
    /* End transmission parameters */
    ret = cli_end(fd);
    return ret;
}

int hon_analytics_get_rules(char *value, char* sql_query) {
    // printf("\n============comes in %s in client.cpp============\n",__func__);

    int fd, value_len, param_len;
    int ret = 0;

    fd = cli_begin((char *)__func__);
    /* Transmission parameters */
    // param_len = strlen(sql_query)+1;
    param_len = strlen(sql_query);
    
    if (sock_write(fd, &param_len, sizeof(param_len)) == SOCKERR_CLOSED) {
  		return -1;
    }

    if (sock_write(fd, sql_query, param_len) == SOCKERR_CLOSED) {
      return -1;
    }
    
    sock_read(fd, &value_len, sizeof(int));
    if (value_len != 0) {
      sock_read(fd, value, value_len);
    }
    memset(value + value_len, '\0', 1); // set terminator
    sock_read(fd, &ret, sizeof(int));
    /* End transmission parameters */
    ret = cli_end(fd);
    return ret;

}

int hon_analytics_update_db(char* sql_query) {
    int fd, param_len;
    int ret = 0, err = 0;

    fd = cli_begin((char *)__func__);
    /* Transmission parameters */
    param_len = strlen(sql_query);
    // param_len = strlen(sql_query);
    
    if (sock_write(fd, &param_len, sizeof(param_len)) == SOCKERR_CLOSED) {
  		return -1;
    }

    if (sock_write(fd, sql_query, param_len) == SOCKERR_CLOSED) {
      return -1;
    }
    
    sock_read(fd, &err, sizeof(int));
    
    /* End transmission parameters */
    ret = cli_end(fd);

    // logger.log(Logger::INFO,"end func",__func__, ret);
    
    if (ret == 0) {
      return err;
    }
    
    return ret;

}


int hon_count_db_records(int *value, const char* sql_query) {
    // printf("\n============comes in %s in client.cpp============\n",__func__);

    int fd, param_len;
    int ret = 0;

    fd = cli_begin((char *)__func__);
    /* Transmission parameters */
    // param_len = strlen(sql_query)+1;
    param_len = strlen(sql_query);
    
    if (sock_write(fd, &param_len, sizeof(param_len)) == SOCKERR_CLOSED) {
  		return -1;
    }

    if (sock_write(fd, sql_query, param_len) == SOCKERR_CLOSED) {
      return -1;
    }
    
    sock_read(fd, value, sizeof(int));


    sock_read(fd, &ret, sizeof(int));
    if (ret != 0) {
      return ret;
    }
    /* End transmission parameters */
    ret = cli_end(fd);
    return ret;
}

int hon_update_analytics_module() {
  int fd, param = 1;
  int ret = 0;
  fd = cli_begin((char *)__func__);
  if (sock_write(fd, &param, sizeof(param)) == SOCKERR_CLOSED) {
    return -1;
  }
  sock_read(fd, &ret, sizeof(int));
  ret = cli_end(fd);
  return ret;
}

int hon_stop_analytics_module() {
  int fd, param = 1;
  int ret = 0;
  fd = cli_begin((char *)__func__);
  if (sock_write(fd, &param, sizeof(param)) == SOCKERR_CLOSED) {
    return -1;
  }
  sock_read(fd, &ret, sizeof(int));
  ret = cli_end(fd);
  return ret;
}

int hon_start_analytics_module() {
  int fd, param = 1;
  int ret = 0;
  fd = cli_begin((char *)__func__);
  if (sock_write(fd, &param, sizeof(param)) == SOCKERR_CLOSED) {
    return -1;
  }
  sock_read(fd, &ret, sizeof(int));
  ret = cli_end(fd);
  return ret;
}



int hon_update_camera_tamper_module() {
  int fd, param = 1;
  int ret = 0;
  fd = cli_begin((char *)__func__);
  if (sock_write(fd, &param, sizeof(param)) == SOCKERR_CLOSED) {
    return -1;
  }
  sock_read(fd, &ret, sizeof(int));
  ret = cli_end(fd);
  return ret;
}

int hon_stop_camera_tamper_module() {
  int fd, param = 1;
  int ret = 0;
  fd = cli_begin((char *)__func__);
  if (sock_write(fd, &param, sizeof(param)) == SOCKERR_CLOSED) {
    return -1;
  }
  sock_read(fd, &ret, sizeof(int));
  ret = cli_end(fd);
  return ret;
}

int hon_start_camera_tamper_module() {
  int fd, param = 1;
  int ret = 0;
  fd = cli_begin((char *)__func__);
  if (sock_write(fd, &param, sizeof(param)) == SOCKERR_CLOSED) {
    return -1;
  }
  sock_read(fd, &ret, sizeof(int));
  ret = cli_end(fd);
  return ret;
}

//*******************Network*********************

int hon_network_get_method(char **value) {
  return hon_client_get_string((char *)__func__, value);
}

int hon_network_set_method(const char *value) {
  return hon_client_set_string((char *)__func__, value);
}

int hon_network_smtp_get(char **value) {
  return hon_client_get_string((char *)__func__, value);
}

int hon_network_smtp_set(const char *value) {
  return hon_client_set_string((char *)__func__, value);
}

int hon_network_set_ipv4_address(const char *value) {
  return hon_client_set_string((char *)__func__, value);
}

int hon_network_set_ipv4_netmask(const char *value) {
  return hon_client_set_string((char *)__func__, value);
}

int hon_network_set_ipv4_gateway(const char *value) {
  return hon_client_set_string((char *)__func__, value);
}

int hon_network_get_dns_method(char **value) {
  return hon_client_get_string((char *)__func__, value);
}

int hon_network_set_dns_method(const char *value) {
  return hon_client_set_string((char *)__func__, value);
}

int hon_network_get_http_port(int *value) {
  return hon_client_get_int((char *)__func__, value);
}

int hon_network_get_https_port(int *value) {
  return hon_client_get_int((char *)__func__, value);
}

int hon_network_get_rtsp_port(int *value) {
  return hon_client_get_int((char *)__func__, value);
}

int hon_network_get_onvif_port(int *value) {
  return hon_client_get_int((char *)__func__, value);
}


int hon_network_set_http_port(int value) {
  return hon_client_set_int((char *)__func__, value);
}

int hon_network_set_https_port(int value) {
  return hon_client_set_int((char *)__func__, value);
}

int hon_network_set_rtsp_port(int value) {
  return hon_client_set_int((char *)__func__, value);
}

int hon_network_set_onvif_port(int value) {
  return hon_client_set_int((char *)__func__, value);
}

int hon_network_get_timezone(char **value) {
  return hon_client_get_string((char *)__func__, value);
}

int hon_network_set_timezone(const char *value) {
  return hon_client_set_string((char *)__func__, value);
}

int hon_network_get_ntp_enable(int *value) {
  return hon_client_get_int((char *)__func__, value);
}

int hon_network_set_ntp_enable(int value) {
  return hon_client_set_int((char *)__func__, value);
}

int hon_network_get_ntp_refresh_time_sec(int *value) {
  return hon_client_get_int((char *)__func__, value);
}

int hon_network_set_ntp_refresh_time_sec(int value) {
  return hon_client_set_int((char *)__func__, value);
}
int hon_network_get_ntp_server(char **value) {
  return hon_client_get_string((char *)__func__, value);
}

int hon_network_set_ntp_server(const char *value) {
  return hon_client_set_string((char *)__func__, value);
}
int hon_network_get_ntp_port(int *value) {
  return hon_client_get_int((char *)__func__, value);
}

int hon_network_set_ntp_port(int value) {
  return hon_client_set_int((char *)__func__, value);
}

int hon_network_get_onvif_timesync_flag(int *value) {
  return hon_client_get_int((char *)__func__, value);
}

int hon_network_set_onvif_timesync_flag(int value) {
  return hon_client_set_int((char *)__func__, value);
}

//********************************************************************
// SMTP Client Functions
//********************************************************************

int hon_smtp_get_config(char **value) {
  return hon_client_get_string((char *)"hon_smtp_get_config", value);
}

int hon_smtp_set_config(const char *json_config) {
  return hon_client_set_string((char *)"hon_smtp_set_config", json_config);
}

int hon_smtp_test_connection(char **error_msg) {
  int fd, ret, err, len;

  if ((fd = cli_begin((char *)"hon_smtp_test_connection")) < 0)
    return -1;

  // Read error message
  if (sock_read(fd, &len, sizeof(len)) == SOCKERR_CLOSED)
    goto out;
  
  if (len > 0 && error_msg != NULL) {
    *error_msg = (char *)malloc(len + 1);
    if (*error_msg) {
      if (sock_read(fd, *error_msg, len) == SOCKERR_CLOSED) {
        free(*error_msg);
        *error_msg = NULL;
        goto out;
      }
      (*error_msg)[len] = '\0';
    }
  } else if (len > 0) {
    // Discard error message if buffer not provided
    char *temp = (char *)malloc(len);
    if (temp) {
      sock_read(fd, temp, len);
      free(temp);
    }
  }

  // Read result
  if (sock_read(fd, &err, sizeof(int)) == SOCKERR_CLOSED)
    goto out;

  ret = err;
  cli_end(fd);
  return ret;

out:
  cli_end(fd);
  return -1;
}

int hon_smtp_send_test_email(const char *to_email, char **error_msg) {
  int fd, ret, err, len;

  if ((fd = cli_begin((char *)"hon_smtp_send_test_email")) < 0)
    return -1;

  // Write recipient email
  len = strlen(to_email);
  if (sock_write(fd, &len, sizeof(len)) == SOCKERR_CLOSED)
    goto out;
  if (sock_write(fd, to_email, len) == SOCKERR_CLOSED)
    goto out;

  // Read error message
  if (sock_read(fd, &len, sizeof(len)) == SOCKERR_CLOSED)
    goto out;
  
  if (len > 0 && error_msg != NULL) {
    *error_msg = (char *)malloc(len + 1);
    if (*error_msg) {
      if (sock_read(fd, *error_msg, len) == SOCKERR_CLOSED) {
        free(*error_msg);
        *error_msg = NULL;
        goto out;
      }
      (*error_msg)[len] = '\0';
    }
  } else if (len > 0) {
    char *temp = (char *)malloc(len);
    if (temp) {
      sock_read(fd, temp, len);
      free(temp);
    }
  }

  // Read result
  if (sock_read(fd, &err, sizeof(int)) == SOCKERR_CLOSED)
    goto out;

  ret = err;
  cli_end(fd);
  return ret;

out:
  cli_end(fd);
  return -1;
}

int hon_smtp_get_enabled(int *value) {
  return hon_client_get_int((char *)"hon_smtp_get_enabled", value);
}

int hon_smtp_set_enabled(int value) {
  return hon_client_set_int((char *)"hon_smtp_set_enabled", value);
}

//********************************************************************
// UPnP Client Functions
//********************************************************************

int hon_upnp_discover(int timeout_ms) {
  int fd, ret, err;

  if ((fd = cli_begin((char *)"hon_upnp_discover")) < 0)
    return -1;

  // Write timeout
  if (sock_write(fd, &timeout_ms, sizeof(timeout_ms)) == SOCKERR_CLOSED)
    goto out;

  // Read result
  if (sock_read(fd, &err, sizeof(int)) == SOCKERR_CLOSED)
    goto out;

  ret = err;
  cli_end(fd);
  return ret;

out:
  cli_end(fd);
  return -1;
}

int hon_upnp_get_device_info(char **json_info) {
  return hon_client_get_string((char *)"hon_upnp_get_device_info", json_info);
}

int hon_upnp_add_port_mapping(const char *json_mapping, char **error_msg) {
  int fd, ret, err, len;

  if ((fd = cli_begin((char *)"hon_upnp_add_port_mapping")) < 0)
    return -1;

  // Write JSON mapping
  len = strlen(json_mapping);
  if (sock_write(fd, &len, sizeof(len)) == SOCKERR_CLOSED)
    goto out;
  if (sock_write(fd, json_mapping, len) == SOCKERR_CLOSED)
    goto out;

  // Read error message
  if (sock_read(fd, &len, sizeof(len)) == SOCKERR_CLOSED)
    goto out;
  
  if (len > 0 && error_msg != NULL) {
    *error_msg = (char *)malloc(len + 1);
    if (*error_msg) {
      if (sock_read(fd, *error_msg, len) == SOCKERR_CLOSED) {
        free(*error_msg);
        *error_msg = NULL;
        goto out;
      }
      (*error_msg)[len] = '\0';
    }
  } else if (len > 0) {
    char *temp = (char *)malloc(len);
    if (temp) {
      sock_read(fd, temp, len);
      free(temp);
    }
  }

  // Read result
  if (sock_read(fd, &err, sizeof(int)) == SOCKERR_CLOSED)
    goto out;

  ret = err;
  cli_end(fd);
  return ret;

out:
  cli_end(fd);
  return -1;
}

int hon_upnp_delete_port_mapping(int external_port, const char *protocol, char **error_msg) {
  int fd, ret, err, len;

  if ((fd = cli_begin((char *)"hon_upnp_delete_port_mapping")) < 0)
    return -1;

  // Write external port
  if (sock_write(fd, &external_port, sizeof(external_port)) == SOCKERR_CLOSED)
    goto out;

  // Write protocol
  len = strlen(protocol);
  if (sock_write(fd, &len, sizeof(len)) == SOCKERR_CLOSED)
    goto out;
  if (sock_write(fd, protocol, len) == SOCKERR_CLOSED)
    goto out;

  // Read error message
  if (sock_read(fd, &len, sizeof(len)) == SOCKERR_CLOSED)
    goto out;
  
  if (len > 0 && error_msg != NULL) {
    *error_msg = (char *)malloc(len + 1);
    if (*error_msg) {
      if (sock_read(fd, *error_msg, len) == SOCKERR_CLOSED) {
        free(*error_msg);
        *error_msg = NULL;
        goto out;
      }
      (*error_msg)[len] = '\0';
    }
  } else if (len > 0) {
    char *temp = (char *)malloc(len);
    if (temp) {
      sock_read(fd, temp, len);
      free(temp);
    }
  }

  // Read result
  if (sock_read(fd, &err, sizeof(int)) == SOCKERR_CLOSED)
    goto out;

  ret = err;
  cli_end(fd);
  return ret;

out:
  cli_end(fd);
  return -1;
}

int hon_upnp_list_port_mappings(char **json_list) {
  return hon_client_get_string((char *)"hon_upnp_list_port_mappings", json_list);
}

int hon_upnp_get_external_ip(char **external_ip) {
  return hon_client_get_string((char *)"hon_upnp_get_external_ip", external_ip);
}

int hon_upnp_add_camera_ports(int http_port, int https_port, int rtsp_port, int onvif_port, char **error_msg) {
  int fd, ret, err, len;

  if ((fd = cli_begin((char *)"hon_upnp_add_camera_ports")) < 0)
    return -1;

  // Write ports
  if (sock_write(fd, &http_port, sizeof(http_port)) == SOCKERR_CLOSED)
    goto out;
  if (sock_write(fd, &https_port, sizeof(https_port)) == SOCKERR_CLOSED)
    goto out;
  if (sock_write(fd, &rtsp_port, sizeof(rtsp_port)) == SOCKERR_CLOSED)
    goto out;
  if (sock_write(fd, &onvif_port, sizeof(onvif_port)) == SOCKERR_CLOSED)
    goto out;

  // Read error message
  if (sock_read(fd, &len, sizeof(len)) == SOCKERR_CLOSED)
    goto out;
  
  if (len > 0 && error_msg != NULL) {
    *error_msg = (char *)malloc(len + 1);
    if (*error_msg) {
      if (sock_read(fd, *error_msg, len) == SOCKERR_CLOSED) {
        free(*error_msg);
        *error_msg = NULL;
        goto out;
      }
      (*error_msg)[len] = '\0';
    }
  } else if (len > 0) {
    char *temp = (char *)malloc(len);
    if (temp) {
      sock_read(fd, temp, len);
      free(temp);
    }
  }

  // Read result
  if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED)
    goto out;

  ret = err;
  cli_end(fd);
  return ret;

out:
  cli_end(fd);
  return -1;
}

int hon_upnp_get_config(char **json_config) {
  return hon_client_get_string((char *)"hon_upnp_get_config", json_config);
}

int hon_upnp_set_config(const char *json_config) {
  return hon_client_set_string((char *)"hon_upnp_set_config", json_config);
}

int hon_upnp_get_enabled(int *value) {
  return hon_client_get_int((char *)"hon_upnp_get_enabled", value);
}

int hon_upnp_set_enabled(int value) {
  return hon_client_set_int((char *)"hon_upnp_set_enabled", value);
}

//********************************************************************
// SNMP Client Functions
//********************************************************************

int hon_snmp_get_config(char **json_config) {
  return hon_client_get_string((char *)"hon_snmp_get_config", json_config);
}

int hon_snmp_set_config(const char *json_config) {
  return hon_client_set_string((char *)"hon_snmp_set_config", json_config);
}

int hon_snmp_get_enabled(int *value) {
  return hon_client_get_int((char *)"hon_snmp_get_enabled", value);
}

int hon_snmp_set_enabled(int value) {
  return hon_client_set_int((char *)"hon_snmp_set_enabled", value);
}

int hon_snmp_start(char **error_msg) {
  if (!error_msg) {
    printf("ERROR: NULL parameter\n");
    return -1;
  }

  int fd = cli_begin((char *)"hon_snmp_start");
  if (fd < 0) return -1;

  int msg_len = 0;
  int ret = 0;

  if (sock_read(fd, &msg_len, sizeof(int)) == SOCKERR_CLOSED) {
    cli_end(fd);
    return -1;
  }

  if (msg_len > 0 && msg_len < 4096) {
    *error_msg = (char *)malloc(msg_len);
    if (*error_msg == NULL) {
      cli_end(fd);
      return -1;
    }
    
    if (sock_read(fd, *error_msg, msg_len) == SOCKERR_CLOSED) {
      free(*error_msg);
      *error_msg = NULL;
      cli_end(fd);
      return -1;
    }
  } else {
    *error_msg = NULL;
  }

  if (sock_read(fd, &ret, sizeof(int)) == SOCKERR_CLOSED) {
    if (*error_msg) {
      free(*error_msg);
      *error_msg = NULL;
    }
    cli_end(fd);
    return -1;
  }

  cli_end(fd);
  return ret;
}

int hon_snmp_stop(char **error_msg) {
  if (!error_msg) {
    printf("ERROR: NULL parameter\n");
    return -1;
  }

  int fd = cli_begin((char *)"hon_snmp_stop");
  if (fd < 0) return -1;

  int msg_len = 0;
  int ret = 0;

  if (sock_read(fd, &msg_len, sizeof(int)) == SOCKERR_CLOSED) {
    cli_end(fd);
    return -1;
  }

  if (msg_len > 0 && msg_len < 4096) {
    *error_msg = (char *)malloc(msg_len);
    if (*error_msg == NULL) {
      cli_end(fd);
      return -1;
    }
    
    if (sock_read(fd, *error_msg, msg_len) == SOCKERR_CLOSED) {
      free(*error_msg);
      *error_msg = NULL;
      cli_end(fd);
      return -1;
    }
  } else {
    *error_msg = NULL;
  }

  if (sock_read(fd, &ret, sizeof(int)) == SOCKERR_CLOSED) {
    if (*error_msg) {
      free(*error_msg);
      *error_msg = NULL;
    }
    cli_end(fd);
    return -1;
  }

  cli_end(fd);
  return ret;
}

int hon_snmp_restart(char **error_msg) {
  if (!error_msg) {
    printf("ERROR: NULL parameter\n");
    return -1;
  }

  int fd = cli_begin((char *)"hon_snmp_restart");
  if (fd < 0) return -1;

  int msg_len = 0;
  int ret = 0;

  if (sock_read(fd, &msg_len, sizeof(int)) == SOCKERR_CLOSED) {
    cli_end(fd);
    return -1;
  }

  if (msg_len > 0 && msg_len < 4096) {
    *error_msg = (char *)malloc(msg_len);
    if (*error_msg == NULL) {
      cli_end(fd);
      return -1;
    }
    
    if (sock_read(fd, *error_msg, msg_len) == SOCKERR_CLOSED) {
      free(*error_msg);
      *error_msg = NULL;
      cli_end(fd);
      return -1;
    }
  } else {
    *error_msg = NULL;
  }

  if (sock_read(fd, &ret, sizeof(int)) == SOCKERR_CLOSED) {
    if (*error_msg) {
      free(*error_msg);
      *error_msg = NULL;
    }
    cli_end(fd);
    return -1;
  }

  cli_end(fd);
  return ret;
}

int hon_snmp_get_status(char **json_status) {
  return hon_client_get_string((char *)"hon_snmp_get_status", json_status);
}

int hon_snmp_test(char **error_msg) {
  if (!error_msg) {
    printf("ERROR: NULL parameter\n");
    return -1;
  }

  int fd = cli_begin((char *)"hon_snmp_test");
  if (fd < 0) return -1;

  int msg_len = 0;
  int ret = 0;

  if (sock_read(fd, &msg_len, sizeof(int)) == SOCKERR_CLOSED) {
    cli_end(fd);
    return -1;
  }

  if (msg_len > 0 && msg_len < 4096) {
    *error_msg = (char *)malloc(msg_len);
    if (*error_msg == NULL) {
      cli_end(fd);
      return -1;
    }
    
    if (sock_read(fd, *error_msg, msg_len) == SOCKERR_CLOSED) {
      free(*error_msg);
      *error_msg = NULL;
      cli_end(fd);
      return -1;
    }
  } else {
    *error_msg = NULL;
  }

  if (sock_read(fd, &ret, sizeof(int)) == SOCKERR_CLOSED) {
    if (*error_msg) {
      free(*error_msg);
      *error_msg = NULL;
    }
    cli_end(fd);
    return -1;
  }

  cli_end(fd);
  return ret;
}
