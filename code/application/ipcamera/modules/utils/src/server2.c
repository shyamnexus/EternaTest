// Copyright 2022 Rockchip Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <assert.h>
#include <fcntl.h>
#include <pthread.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/un.h>
#include <unistd.h>

#include "common.h"

// #include "event.h"
#include "log.h"
#include "network.h"
#include "socket.h"
#include "storage.h"
#include "system.h"

// set by CMakeList.txt
#include "audio.h"
#include "isp.h"
#include "osd.h"
// #include "region_clip.h"
// #include "roi.h"
#include "video.h"

#include "smtp_service.h"

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "server.c"

static int listen_fd = 0;
static pthread_t RkIpcServerTid = 0;
static int RkIpcServerRun = 0;

struct FunMap {
	char *fun_name;
	int (*fun)(int);
};

// int ser_rk_isp_set(int fd) {
// 	int len;
// 	char *json = NULL;

// 	if (sock_read(fd, &len, sizeof(int)) == SOCKERR_CLOSED)
// 		return -1;

// 	if (len) {
// 		json = (char *)malloc(len);
// 		if (sock_read(fd, json, len) == SOCKERR_CLOSED) {
// 			free(json);
// 			return -1;
// 		}

// 		LOG_DEBUG("isp json is %s\n", json);
// 		free(json);
// 	}

// 	return 0;
// }

// int ser_rk_video_set(int fd) {
// 	int len;
// 	char *json = NULL;

// 	if (sock_read(fd, &len, sizeof(int)) == SOCKERR_CLOSED)
// 		return -1;

// 	if (len) {
// 		json = (char *)malloc(len);
// 		if (sock_read(fd, json, len) == SOCKERR_CLOSED) {
// 			free(json);
// 			return -1;
// 		}

// 		LOG_DEBUG("video json is %s\n", json);
// 		free(json);
// 	}

// 	return 0;
// }

// int ser_rk_audio_set(int fd) {
// 	int len;
// 	char *json = NULL;

// 	if (sock_read(fd, &len, sizeof(int)) == SOCKERR_CLOSED)
// 		return -1;

// 	if (len) {
// 		json = (char *)malloc(len);
// 		if (sock_read(fd, json, len) == SOCKERR_CLOSED) {
// 			free(json);
// 			return -1;
// 		}

// 		LOG_DEBUG("audio json is %s\n", json);
// 		free(json);
// 	}

// 	return 0;
// }

// // isp scenario
// int ser_rk_isp_get_scenario(int fd) {
// 	int err = 0;
// 	int id, len;
// 	const char *value;

// 	if (sock_read(fd, &id, sizeof(id)) == SOCKERR_CLOSED)
// 		return -1;
// 	err = rk_isp_get_scenario(id, &value);
// 	len = strlen(value);
// 	LOG_DEBUG("len is %d, value is %s, addr is %p\n", len, value, value);
// 	if (sock_write(fd, &len, sizeof(len)) == SOCKERR_CLOSED)
// 		return -1;
// 	if (sock_write(fd, value, len) == SOCKERR_CLOSED)
// 		return -1;
// 	if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED)
// 		return -1;

// 	return 0;
// }
// int ser_rk_isp_set_default(int fd){
// 	int ret = 0;
// 	int id;

// 	if (sock_read(fd, &id, sizeof(id)) == SOCKERR_CLOSED)
// 		return -1;
// 	ret = rk_isp_set_default(id);
// 	if (sock_write(fd, &ret, sizeof(int)) == SOCKERR_CLOSED)
// 		return -1;

// 	return 0;
// }

// int ser_rk_isp_set_scenario(int fd) {
// 	int ret = 0;
// 	int id, len;
// 	char *value = NULL;

// 	if (sock_read(fd, &id, sizeof(id)) == SOCKERR_CLOSED)
// 		return -1;
// 	if (sock_read(fd, &len, sizeof(len)) == SOCKERR_CLOSED)
// 		return -1;
// 	if (len) {
// 		value = (char *)malloc(len);
// 		if (sock_read(fd, value, len) == SOCKERR_CLOSED) {
// 			free(value);
// 			return -1;
// 		}
// 		LOG_DEBUG("id is %d, value is %s\n", id, value);
// 		ret = rk_isp_set_scenario(id, value);
// 		free(value);
// 		if (sock_write(fd, &ret, sizeof(int)) == SOCKERR_CLOSED)
// 			return -1;
// 	}

// 	return 0;
// }

// // isp adjustment

// int ser_rk_isp_get_contrast(int fd) {
// 	int err = 0;
// 	int id;
// 	int value;

// 	if (sock_read(fd, &id, sizeof(id)) == SOCKERR_CLOSED)
// 		return -1;
// 	err = rk_isp_get_contrast(id, &value);
// 	LOG_DEBUG("value is %d\n", value);
// 	if (sock_write(fd, &value, sizeof(value)) == SOCKERR_CLOSED)
// 		return -1;
// 	if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED)
// 		return -1;

// 	return 0;
// }

// int ser_rk_isp_set_contrast(int fd) {
// 	int err = 0;
// 	int id;
// 	int value;

// 	if (sock_read(fd, &id, sizeof(id)) == SOCKERR_CLOSED)
// 		return -1;
// 	if (sock_read(fd, &value, sizeof(value)) == SOCKERR_CLOSED)
// 		return -1;
// 	LOG_DEBUG("value is %d\n", value);
// 	err = rk_isp_set_contrast(id, value);
// 	if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED)
// 		return -1;

// 	return 0;
// }

// int ser_rk_isp_get_brightness(int fd) {
// 	int err = 0;
// 	int id;
// 	int value;

// 	if (sock_read(fd, &id, sizeof(id)) == SOCKERR_CLOSED)
// 		return -1;
// 	err = rk_isp_get_brightness(id, &value);
// 	LOG_DEBUG("value is %d\n", value);
// 	if (sock_write(fd, &value, sizeof(value)) == SOCKERR_CLOSED)
// 		return -1;
// 	if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED)
// 		return -1;

// 	return 0;
// }

// int ser_rk_isp_set_brightness(int fd) {
// 	int err = 0;
// 	int id;
// 	int value;

// 	if (sock_read(fd, &id, sizeof(id)) == SOCKERR_CLOSED)
// 		return -1;
// 	if (sock_read(fd, &value, sizeof(value)) == SOCKERR_CLOSED)
// 		return -1;
// 	LOG_DEBUG("value is %d\n", value);
// 	err = rk_isp_set_brightness(id, value);
// 	if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED)
// 		return -1;

// 	return 0;
// }

// int ser_rk_isp_get_saturation(int fd) {
// 	int err = 0;
// 	int id;
// 	int value;

// 	if (sock_read(fd, &id, sizeof(id)) == SOCKERR_CLOSED)
// 		return -1;
// 	err = rk_isp_get_saturation(id, &value);
// 	LOG_DEBUG("value is %d\n", value);
// 	if (sock_write(fd, &value, sizeof(value)) == SOCKERR_CLOSED)
// 		return -1;
// 	if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED)
// 		return -1;

// 	return 0;
// }

// int ser_rk_isp_set_saturation(int fd) {
// 	int err = 0;
// 	int id;
// 	int value;

// 	if (sock_read(fd, &id, sizeof(id)) == SOCKERR_CLOSED)
// 		return -1;
// 	if (sock_read(fd, &value, sizeof(value)) == SOCKERR_CLOSED)
// 		return -1;
// 	LOG_DEBUG("value is %d\n", value);
// 	err = rk_isp_set_saturation(id, value);
// 	if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED)
// 		return -1;

// 	return 0;
// }

// int ser_rk_isp_get_sharpness(int fd) {
// 	int err = 0;
// 	int id;
// 	int value;

// 	if (sock_read(fd, &id, sizeof(id)) == SOCKERR_CLOSED)
// 		return -1;
// 	err = rk_isp_get_sharpness(id, &value);
// 	LOG_DEBUG("value is %d\n", value);
// 	if (sock_write(fd, &value, sizeof(value)) == SOCKERR_CLOSED)
// 		return -1;
// 	if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED)
// 		return -1;

// 	return 0;
// }

// int ser_rk_isp_set_sharpness(int fd) {
// 	int err = 0;
// 	int id;
// 	int value;

// 	if (sock_read(fd, &id, sizeof(id)) == SOCKERR_CLOSED)
// 		return -1;
// 	if (sock_read(fd, &value, sizeof(value)) == SOCKERR_CLOSED)
// 		return -1;
// 	LOG_DEBUG("value is %d\n", value);
// 	err = rk_isp_set_sharpness(id, value);
// 	if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED)
// 		return -1;

// 	return 0;
// }

// int ser_rk_isp_get_hue(int fd) {
// 	int err = 0;
// 	int id;
// 	int value;

// 	if (sock_read(fd, &id, sizeof(id)) == SOCKERR_CLOSED)
// 		return -1;
// 	err = rk_isp_get_hue(id, &value);
// 	LOG_DEBUG("value is %d\n", value);
// 	if (sock_write(fd, &value, sizeof(value)) == SOCKERR_CLOSED)
// 		return -1;
// 	if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED)
// 		return -1;

// 	return 0;
// }

// int ser_rk_isp_set_hue(int fd) {
// 	int err = 0;
// 	int id;
// 	int value;

// 	if (sock_read(fd, &id, sizeof(id)) == SOCKERR_CLOSED)
// 		return -1;
// 	if (sock_read(fd, &value, sizeof(value)) == SOCKERR_CLOSED)
// 		return -1;
// 	LOG_DEBUG("value is %d\n", value);
// 	err = rk_isp_set_hue(id, value);
// 	if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED)
// 		return -1;

// 	return 0;
// }

// // isp exposure
// int ser_rk_isp_get_exposure_mode(int fd) {
// 	int err = 0;
// 	int id, len;
// 	const char *value;

// 	if (sock_read(fd, &id, sizeof(id)) == SOCKERR_CLOSED)
// 		return -1;
// 	err = rk_isp_get_exposure_mode(id, &value);
// 	len = strlen(value);
// 	LOG_DEBUG("len is %d, value is %s, addr is %p\n", len, value, value);
// 	if (sock_write(fd, &len, sizeof(len)) == SOCKERR_CLOSED)
// 		return -1;
// 	if (sock_write(fd, value, len) == SOCKERR_CLOSED)
// 		return -1;
// 	if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED)
// 		return -1;

// 	return 0;
// }

// int ser_rk_isp_set_exposure_mode(int fd) {
// 	int ret = 0;
// 	int id, len;
// 	char *value = NULL;

// 	if (sock_read(fd, &id, sizeof(id)) == SOCKERR_CLOSED)
// 		return -1;
// 	if (sock_read(fd, &len, sizeof(len)) == SOCKERR_CLOSED)
// 		return -1;
// 	if (len) {
// 		value = (char *)malloc(len);
// 		if (sock_read(fd, value, len) == SOCKERR_CLOSED) {
// 			free(value);
// 			return -1;
// 		}
// 		LOG_DEBUG("id is %d, value is %s\n", id, value);
// 		ret = rk_isp_set_exposure_mode(id, value);
// 		free(value);
// 		if (sock_write(fd, &ret, sizeof(int)) == SOCKERR_CLOSED)
// 			return -1;
// 	}

// 	return 0;
// }

// int ser_rk_isp_get_gain_mode(int fd) {
// 	int err = 0;
// 	int id, len;
// 	const char *value;

// 	if (sock_read(fd, &id, sizeof(id)) == SOCKERR_CLOSED)
// 		return -1;
// 	err = rk_isp_get_gain_mode(id, &value);
// 	len = strlen(value);
// 	LOG_DEBUG("len is %d, value is %s, addr is %p\n", len, value, value);
// 	if (sock_write(fd, &len, sizeof(len)) == SOCKERR_CLOSED)
// 		return -1;
// 	if (sock_write(fd, value, len) == SOCKERR_CLOSED)
// 		return -1;
// 	if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED)
// 		return -1;

// 	return 0;
// }

// int ser_rk_isp_set_gain_mode(int fd) {
// 	int ret = 0;
// 	int id, len;
// 	char *value = NULL;

// 	if (sock_read(fd, &id, sizeof(id)) == SOCKERR_CLOSED)
// 		return -1;
// 	if (sock_read(fd, &len, sizeof(len)) == SOCKERR_CLOSED)
// 		return -1;
// 	if (len) {
// 		value = (char *)malloc(len);
// 		if (sock_read(fd, value, len) == SOCKERR_CLOSED) {
// 			free(value);
// 			return -1;
// 		}
// 		LOG_DEBUG("id is %d, value is %s\n", id, value);
// 		ret = rk_isp_set_gain_mode(id, value);
// 		free(value);
// 		if (sock_write(fd, &ret, sizeof(int)) == SOCKERR_CLOSED)
// 			return -1;
// 	}

// 	return 0;
// }

// int ser_rk_isp_get_exposure_time(int fd) {
// 	int err = 0;
// 	int id, len;
// 	const char *value;

// 	if (sock_read(fd, &id, sizeof(id)) == SOCKERR_CLOSED)
// 		return -1;
// 	err = rk_isp_get_exposure_time(id, &value);
// 	len = strlen(value);
// 	LOG_DEBUG("len is %d, value is %s, addr is %p\n", len, value, value);
// 	if (sock_write(fd, &len, sizeof(len)) == SOCKERR_CLOSED)
// 		return -1;
// 	if (sock_write(fd, value, len) == SOCKERR_CLOSED)
// 		return -1;
// 	if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED)
// 		return -1;

// 	return 0;
// }

// int ser_rk_isp_set_exposure_time(int fd) {
// 	int ret = 0;
// 	int id, len;
// 	char *value = NULL;

// 	if (sock_read(fd, &id, sizeof(id)) == SOCKERR_CLOSED)
// 		return -1;
// 	if (sock_read(fd, &len, sizeof(len)) == SOCKERR_CLOSED)
// 		return -1;
// 	if (len) {
// 		value = (char *)malloc(len);
// 		if (sock_read(fd, value, len) == SOCKERR_CLOSED) {
// 			free(value);
// 			return -1;
// 		}
// 		LOG_DEBUG("id is %d, value is %s\n", id, value);
// 		ret = rk_isp_set_exposure_time(id, value);
// 		free(value);
// 		if (sock_write(fd, &ret, sizeof(int)) == SOCKERR_CLOSED)
// 			return -1;
// 	}

// 	return 0;
// }

// int ser_rk_isp_get_exposure_gain(int fd) {
// 	int err = 0;
// 	int id;
// 	int value;

// 	if (sock_read(fd, &id, sizeof(id)) == SOCKERR_CLOSED)
// 		return -1;
// 	err = rk_isp_get_exposure_gain(id, &value);
// 	LOG_DEBUG("value is %d\n", value);
// 	if (sock_write(fd, &value, sizeof(value)) == SOCKERR_CLOSED)
// 		return -1;
// 	if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED)
// 		return -1;

// 	return 0;
// }

// int ser_rk_isp_set_exposure_gain(int fd) {
// 	int err = 0;
// 	int id;
// 	int value;

// 	if (sock_read(fd, &id, sizeof(id)) == SOCKERR_CLOSED)
// 		return -1;
// 	if (sock_read(fd, &value, sizeof(value)) == SOCKERR_CLOSED)
// 		return -1;
// 	LOG_DEBUG("value is %d\n", value);
// 	err = rk_isp_set_exposure_gain(id, value);
// 	if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED)
// 		return -1;

// 	return 0;
// }

// int ser_rk_isp_get_frame_rate(int fd) {
// 	int err = 0;
// 	int id;
// 	int value;

// 	if (sock_read(fd, &id, sizeof(id)) == SOCKERR_CLOSED)
// 		return -1;
// 	err = rk_isp_get_frame_rate(id, &value);
// 	LOG_DEBUG("value is %d\n", value);
// 	if (sock_write(fd, &value, sizeof(value)) == SOCKERR_CLOSED)
// 		return -1;
// 	if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED)
// 		return -1;

// 	return 0;
// }

// int ser_rk_isp_set_frame_rate(int fd) {
// 	int err = 0;
// 	int id;
// 	int value;

// 	if (sock_read(fd, &id, sizeof(id)) == SOCKERR_CLOSED)
// 		return -1;
// 	if (sock_read(fd, &value, sizeof(value)) == SOCKERR_CLOSED)
// 		return -1;
// 	LOG_DEBUG("value is %d\n", value);
// 	err = rk_isp_set_frame_rate(id, value);
// 	if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED)
// 		return -1;

// 	return 0;
// }

// // night to day
// int ser_rk_isp_get_night_to_day(int fd) {
// 	int err = 0;
// 	int id, len;
// 	const char *value;

// 	if (sock_read(fd, &id, sizeof(id)) == SOCKERR_CLOSED)
// 		return -1;
// 	err = rk_isp_get_night_to_day(id, &value);
// 	len = strlen(value);
// 	LOG_DEBUG("len is %d, value is %s, addr is %p\n", len, value, value);
// 	if (sock_write(fd, &len, sizeof(len)) == SOCKERR_CLOSED)
// 		return -1;
// 	if (sock_write(fd, value, len) == SOCKERR_CLOSED)
// 		return -1;
// 	if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED)
// 		return -1;

// 	return 0;
// }

// int ser_rk_isp_set_night_to_day(int fd) {
// 	int ret = 0;
// 	int id, len;
// 	char *value = NULL;

// 	if (sock_read(fd, &id, sizeof(id)) == SOCKERR_CLOSED)
// 		return -1;
// 	if (sock_read(fd, &len, sizeof(len)) == SOCKERR_CLOSED)
// 		return -1;
// 	if (len) {
// 		value = (char *)malloc(len);
// 		if (sock_read(fd, value, len) == SOCKERR_CLOSED) {
// 			free(value);
// 			return -1;
// 		}
// 		LOG_DEBUG("id is %d, value is %s\n", id, value);
// 		ret = rk_isp_set_night_to_day(id, value);
// 		free(value);
// 		if (sock_write(fd, &ret, sizeof(int)) == SOCKERR_CLOSED)
// 			return -1;
// 	}

// 	return 0;
// }

// int ser_rk_isp_get_fill_light_mode(int fd) {
// 	int err = 0;
// 	int id, len;
// 	const char *value;

// 	if (sock_read(fd, &id, sizeof(id)) == SOCKERR_CLOSED)
// 		return -1;
// 	err = rk_isp_get_fill_light_mode(id, &value);
// 	len = strlen(value);
// 	LOG_DEBUG("len is %d, value is %s, addr is %p\n", len, value, value);
// 	if (sock_write(fd, &len, sizeof(len)) == SOCKERR_CLOSED)
// 		return -1;
// 	if (sock_write(fd, value, len) == SOCKERR_CLOSED)
// 		return -1;
// 	if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED)
// 		return -1;

// 	return 0;
// }

// int ser_rk_isp_set_fill_light_mode(int fd) {
// 	int ret = 0;
// 	int id, len;
// 	char *value = NULL;

// 	if (sock_read(fd, &id, sizeof(id)) == SOCKERR_CLOSED)
// 		return -1;
// 	if (sock_read(fd, &len, sizeof(len)) == SOCKERR_CLOSED)
// 		return -1;
// 	if (len) {
// 		value = (char *)malloc(len);
// 		if (sock_read(fd, value, len) == SOCKERR_CLOSED) {
// 			free(value);
// 			return -1;
// 		}
// 		LOG_DEBUG("id is %d, value is %s\n", id, value);
// 		ret = rk_isp_set_fill_light_mode(id, value);
// 		free(value);
// 		if (sock_write(fd, &ret, sizeof(int)) == SOCKERR_CLOSED)
// 			return -1;
// 	}

// 	return 0;
// }

// int ser_rk_isp_get_light_brightness(int fd) {
// 	int err = 0;
// 	int id;
// 	int value;

// 	if (sock_read(fd, &id, sizeof(id)) == SOCKERR_CLOSED)
// 		return -1;
// 	err = rk_isp_get_light_brightness(id, &value);
// 	LOG_DEBUG("value is %d\n", value);
// 	if (sock_write(fd, &value, sizeof(value)) == SOCKERR_CLOSED)
// 		return -1;
// 	if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED)
// 		return -1;

// 	return 0;
// }

// int ser_rk_isp_set_light_brightness(int fd) {
// 	int err = 0;
// 	int id;
// 	int value;

// 	if (sock_read(fd, &id, sizeof(id)) == SOCKERR_CLOSED)
// 		return -1;
// 	if (sock_read(fd, &value, sizeof(value)) == SOCKERR_CLOSED)
// 		return -1;
// 	LOG_DEBUG("value is %d\n", value);
// 	err = rk_isp_set_light_brightness(id, value);
// 	if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED)
// 		return -1;

// 	return 0;
// }

// int ser_rk_isp_get_night_to_day_filter_level(int fd) {
// 	int err = 0;
// 	int id;
// 	int value;

// 	if (sock_read(fd, &id, sizeof(id)) == SOCKERR_CLOSED)
// 		return -1;
// 	err = rk_isp_get_night_to_day_filter_level(id, &value);
// 	LOG_DEBUG("value is %d\n", value);
// 	if (sock_write(fd, &value, sizeof(value)) == SOCKERR_CLOSED)
// 		return -1;
// 	if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED)
// 		return -1;

// 	return 0;
// }

// int ser_rk_isp_set_night_to_day_filter_level(int fd) {
// 	int err = 0;
// 	int id;
// 	int value;

// 	if (sock_read(fd, &id, sizeof(id)) == SOCKERR_CLOSED)
// 		return -1;
// 	if (sock_read(fd, &value, sizeof(value)) == SOCKERR_CLOSED)
// 		return -1;
// 	LOG_DEBUG("value is %d\n", value);
// 	err = rk_isp_set_night_to_day_filter_level(id, value);
// 	if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED)
// 		return -1;

// 	return 0;
// }

// int ser_rk_isp_get_night_to_day_filter_time(int fd) {
// 	int err = 0;
// 	int id;
// 	int value;

// 	if (sock_read(fd, &id, sizeof(id)) == SOCKERR_CLOSED)
// 		return -1;
// 	err = rk_isp_get_night_to_day_filter_time(id, &value);
// 	LOG_DEBUG("value is %d\n", value);
// 	if (sock_write(fd, &value, sizeof(value)) == SOCKERR_CLOSED)
// 		return -1;
// 	if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED)
// 		return -1;

// 	return 0;
// }

// int ser_rk_isp_set_night_to_day_filter_time(int fd) {
// 	int err = 0;
// 	int id;
// 	int value;

// 	if (sock_read(fd, &id, sizeof(id)) == SOCKERR_CLOSED)
// 		return -1;
// 	if (sock_read(fd, &value, sizeof(value)) == SOCKERR_CLOSED)
// 		return -1;
// 	LOG_DEBUG("value is %d\n", value);
// 	err = rk_isp_set_night_to_day_filter_time(id, value);
// 	if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED)
// 		return -1;

// 	return 0;
// }

// int ser_rk_isp_get_IR_mode(int fd) {
// 	int err = 0;
// 	int id, len;
// 	const char *value;

// 	if (sock_read(fd, &id, sizeof(id)) == SOCKERR_CLOSED)
// 		return -1;
// 	err = rk_isp_get_IR_mode(id, &value);
// 	len = strlen(value);
// 	LOG_DEBUG("len is %d, value is %s, addr is %p\n", len, value, value);
// 	if (sock_write(fd, &len, sizeof(len)) == SOCKERR_CLOSED)
// 		return -1;
// 	if (sock_write(fd, value, len) == SOCKERR_CLOSED)
// 		return -1;
// 	if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED)
// 		return -1;

// 	return 0;
// }

// int ser_rk_isp_get_IR_start_time(int fd) {
// 	int err = 0;
// 	int id, len;
// 	const char *value;

// 	if (sock_read(fd, &id, sizeof(id)) == SOCKERR_CLOSED)
// 		return -1;
// 	err = rk_isp_get_IR_start_time(id, &value);
// 	len = strlen(value);
// 	LOG_DEBUG("len is %d, value is %s, addr is %p\n", len, value, value);
// 	if (sock_write(fd, &len, sizeof(len)) == SOCKERR_CLOSED)
// 		return -1;
// 	if (sock_write(fd, value, len) == SOCKERR_CLOSED)
// 		return -1;
// 	if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED)
// 		return -1;

// 	return 0;
// }

// int ser_rk_isp_get_IR_end_time(int fd) {
// 	int err = 0;
// 	int id, len;
// 	const char *value;

// 	if (sock_read(fd, &id, sizeof(id)) == SOCKERR_CLOSED)
// 		return -1;
// 	err = rk_isp_get_IR_end_time(id, &value);
// 	len = strlen(value);
// 	LOG_DEBUG("len is %d, value is %s, addr is %p\n", len, value, value);
// 	if (sock_write(fd, &len, sizeof(len)) == SOCKERR_CLOSED)
// 		return -1;
// 	if (sock_write(fd, value, len) == SOCKERR_CLOSED)
// 		return -1;
// 	if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED)
// 		return -1;

// 	return 0;
// }

// int ser_rk_isp_set_IR_end_time(int fd) {
// 	int ret = 0;
// 	int id, len;
// 	char *value = NULL;

// 	if (sock_read(fd, &id, sizeof(id)) == SOCKERR_CLOSED)
// 		return -1;
// 	if (sock_read(fd, &len, sizeof(len)) == SOCKERR_CLOSED)
// 		return -1;
// 	if (len) {
// 		value = (char *)malloc(len);
// 		if (sock_read(fd, value, len) == SOCKERR_CLOSED) {
// 			free(value);
// 			return -1;
// 		}
// 		LOG_DEBUG("id is %d, value is %s\n", id, value);
// 		ret = rk_isp_set_IR_end_time(id, value);
// 		free(value);
// 		if (sock_write(fd, &ret, sizeof(int)) == SOCKERR_CLOSED)
// 			return -1;
// 	}

// 	return 0;
// }


// int ser_rk_isp_set_IR_start_time(int fd) {
// 	int ret = 0;
// 	int id, len;
// 	char *value = NULL;

// 	if (sock_read(fd, &id, sizeof(id)) == SOCKERR_CLOSED)
// 		return -1;
// 	if (sock_read(fd, &len, sizeof(len)) == SOCKERR_CLOSED)
// 		return -1;
// 	if (len) {
// 		value = (char *)malloc(len);
// 		if (sock_read(fd, value, len) == SOCKERR_CLOSED) {
// 			free(value);
// 			return -1;
// 		}
// 		LOG_DEBUG("id is %d, value is %s\n", id, value);
// 		ret = rk_isp_set_IR_start_time(id, value);
// 		free(value);
// 		if (sock_write(fd, &ret, sizeof(int)) == SOCKERR_CLOSED)
// 			return -1;
// 	}

// 	return 0;
// }

// int ser_rk_isp_get_IR_brightness_mode(int fd) {
// 	int err = 0;
// 	int id, len;
// 	const char *value;

// 	if (sock_read(fd, &id, sizeof(id)) == SOCKERR_CLOSED)
// 		return -1;
// 	err = rk_isp_get_IR_brightness_mode(id, &value);
// 	len = strlen(value);
// 	LOG_DEBUG("len is %d, value is %s, addr is %p\n", len, value, value);
// 	if (sock_write(fd, &len, sizeof(len)) == SOCKERR_CLOSED)
// 		return -1;
// 	if (sock_write(fd, value, len) == SOCKERR_CLOSED)
// 		return -1;
// 	if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED)
// 		return -1;

// 	return 0;
// }

// int ser_rk_isp_set_IR_brightness_mode(int fd) {
// 	int ret = 0;
// 	int id, len;
// 	char *value = NULL;

// 	if (sock_read(fd, &id, sizeof(id)) == SOCKERR_CLOSED)
// 		return -1;
// 	if (sock_read(fd, &len, sizeof(len)) == SOCKERR_CLOSED)
// 		return -1;
// 	if (len) {
// 		value = (char *)malloc(len);
// 		if (sock_read(fd, value, len) == SOCKERR_CLOSED) {
// 			free(value);
// 			return -1;
// 		}
// 		LOG_DEBUG("id is %d, value is %s\n", id, value);
// 		ret = rk_isp_set_IR_brightness_mode(id, value);
// 		free(value);
// 		if (sock_write(fd, &ret, sizeof(int)) == SOCKERR_CLOSED)
// 			return -1;
// 	}

// 	return 0;
// }








// int ser_rk_isp_set_IR_mode(int fd) {
// 	int ret = 0;
// 	int id, len;
// 	char *value = NULL;

// 	if (sock_read(fd, &id, sizeof(id)) == SOCKERR_CLOSED)
// 		return -1;
// 	if (sock_read(fd, &len, sizeof(len)) == SOCKERR_CLOSED)
// 		return -1;
// 	if (len) {
// 		value = (char *)malloc(len);
// 		if (sock_read(fd, value, len) == SOCKERR_CLOSED) {
// 			free(value);
// 			return -1;
// 		}
// 		LOG_DEBUG("id is %d, value is %s\n", id, value);
// 		ret = rk_isp_set_IR_mode(id, value);
// 		free(value);
// 		if (sock_write(fd, &ret, sizeof(int)) == SOCKERR_CLOSED)
// 			return -1;
// 	}

// 	return 0;
// }

// // isp blc
// int ser_rk_isp_get_hdr(int fd) {
// 	int err = 0;
// 	int id, len;
// 	const char *value;

// 	if (sock_read(fd, &id, sizeof(id)) == SOCKERR_CLOSED)
// 		return -1;
// 	err = rk_isp_get_hdr(id, &value);
// 	len = strlen(value);
// 	LOG_DEBUG("len is %d, value is %s, addr is %p\n", len, value, value);
// 	if (sock_write(fd, &len, sizeof(len)) == SOCKERR_CLOSED)
// 		return -1;
// 	if (sock_write(fd, value, len) == SOCKERR_CLOSED)
// 		return -1;
// 	if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED)
// 		return -1;

// 	return 0;
// }

// int ser_rk_isp_set_hdr(int fd) {
// 	int ret = 0;
// 	int id, len;
// 	char *value = NULL;

// 	if (sock_read(fd, &id, sizeof(id)) == SOCKERR_CLOSED)
// 		return -1;
// 	if (sock_read(fd, &len, sizeof(len)) == SOCKERR_CLOSED)
// 		return -1;
// 	if (len) {
// 		value = (char *)malloc(len);
// 		if (sock_read(fd, value, len) == SOCKERR_CLOSED) {
// 			free(value);
// 			return -1;
// 		}

// 		ret = rk_isp_set_hdr(id, value);
// 		free(value);
// 		if (sock_write(fd, &ret, sizeof(int)) == SOCKERR_CLOSED)
// 			return -1;
// 	}

// 	return 0;
// }

// int ser_rk_isp_get_blc_region(int fd) {
// 	int err = 0;
// 	int id, len;
// 	const char *value;

// 	if (sock_read(fd, &id, sizeof(id)) == SOCKERR_CLOSED)
// 		return -1;
// 	err = rk_isp_get_blc_region(id, &value);
// 	len = strlen(value);
// 	LOG_DEBUG("len is %d, value is %s, addr is %p\n", len, value, value);
// 	if (sock_write(fd, &len, sizeof(len)) == SOCKERR_CLOSED)
// 		return -1;
// 	if (sock_write(fd, value, len) == SOCKERR_CLOSED)
// 		return -1;
// 	if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED)
// 		return -1;

// 	return 0;
// }

// int ser_rk_isp_set_blc_region(int fd) {
// 	int ret = 0;
// 	int id, len;
// 	char *value = NULL;

// 	if (sock_read(fd, &id, sizeof(id)) == SOCKERR_CLOSED)
// 		return -1;
// 	if (sock_read(fd, &len, sizeof(len)) == SOCKERR_CLOSED)
// 		return -1;
// 	if (len) {
// 		value = (char *)malloc(len);
// 		if (sock_read(fd, value, len) == SOCKERR_CLOSED) {
// 			free(value);
// 			return -1;
// 		}
// 		LOG_DEBUG("id is %d, value is %s\n", id, value);
// 		ret = rk_isp_set_blc_region(id, value);
// 		free(value);
// 		if (sock_write(fd, &ret, sizeof(int)) == SOCKERR_CLOSED)
// 			return -1;
// 	}

// 	return 0;
// }

// int ser_rk_isp_get_hlc(int fd) {
// 	int err = 0;
// 	int id, len;
// 	const char *value;

// 	if (sock_read(fd, &id, sizeof(id)) == SOCKERR_CLOSED)
// 		return -1;
// 	err = rk_isp_get_hlc(id, &value);
// 	len = strlen(value);
// 	LOG_DEBUG("len is %d, value is %s, addr is %p\n", len, value, value);
// 	if (sock_write(fd, &len, sizeof(len)) == SOCKERR_CLOSED)
// 		return -1;
// 	if (sock_write(fd, value, len) == SOCKERR_CLOSED)
// 		return -1;
// 	if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED)
// 		return -1;

// 	return 0;
// }

// int ser_rk_isp_set_hlc(int fd) {
// 	int ret = 0;
// 	int id, len;
// 	char *value = NULL;

// 	if (sock_read(fd, &id, sizeof(id)) == SOCKERR_CLOSED)
// 		return -1;
// 	if (sock_read(fd, &len, sizeof(len)) == SOCKERR_CLOSED)
// 		return -1;
// 	if (len) {
// 		value = (char *)malloc(len);
// 		if (sock_read(fd, value, len) == SOCKERR_CLOSED) {
// 			free(value);
// 			return -1;
// 		}
// 		LOG_DEBUG("id is %d, value is %s\n", id, value);
// 		ret = rk_isp_set_hlc(id, value);
// 		free(value);
// 		if (sock_write(fd, &ret, sizeof(int)) == SOCKERR_CLOSED)
// 			return -1;
// 	}

// 	return 0;
// }

// int ser_rk_isp_get_hdr_level(int fd) {
// 	int err = 0;
// 	int id;
// 	int value;

// 	if (sock_read(fd, &id, sizeof(id)) == SOCKERR_CLOSED)
// 		return -1;
// 	err = rk_isp_get_hdr_level(id, &value);
// 	LOG_DEBUG("value is %d\n", value);
// 	if (sock_write(fd, &value, sizeof(value)) == SOCKERR_CLOSED)
// 		return -1;
// 	if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED)
// 		return -1;

// 	return 0;
// }

// int ser_rk_isp_set_hdr_level(int fd) {
// 	int err = 0;
// 	int id;
// 	int value;

// 	if (sock_read(fd, &id, sizeof(id)) == SOCKERR_CLOSED)
// 		return -1;
// 	if (sock_read(fd, &value, sizeof(value)) == SOCKERR_CLOSED)
// 		return -1;
// 	LOG_DEBUG("value is %d\n", value);
// 	err = rk_isp_set_hdr_level(id, value);
// 	if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED)
// 		return -1;

// 	return 0;
// }

// int ser_rk_isp_get_blc_strength(int fd) {
// 	int err = 0;
// 	int id;
// 	int value;

// 	if (sock_read(fd, &id, sizeof(id)) == SOCKERR_CLOSED)
// 		return -1;
// 	err = rk_isp_get_blc_strength(id, &value);
// 	LOG_DEBUG("value is %d\n", value);
// 	if (sock_write(fd, &value, sizeof(value)) == SOCKERR_CLOSED)
// 		return -1;
// 	if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED)
// 		return -1;

// 	return 0;
// }

// int ser_rk_isp_set_blc_strength(int fd) {
// 	int err = 0;
// 	int id;
// 	int value;

// 	if (sock_read(fd, &id, sizeof(id)) == SOCKERR_CLOSED)
// 		return -1;
// 	if (sock_read(fd, &value, sizeof(value)) == SOCKERR_CLOSED)
// 		return -1;
// 	LOG_DEBUG("value is %d\n", value);
// 	err = rk_isp_set_blc_strength(id, value);
// 	if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED)
// 		return -1;

// 	return 0;
// }

// int ser_rk_isp_get_hlc_level(int fd) {
// 	int err = 0;
// 	int id;
// 	int value;

// 	if (sock_read(fd, &id, sizeof(id)) == SOCKERR_CLOSED)
// 		return -1;
// 	err = rk_isp_get_hlc_level(id, &value);
// 	LOG_DEBUG("value is %d\n", value);
// 	if (sock_write(fd, &value, sizeof(value)) == SOCKERR_CLOSED)
// 		return -1;
// 	if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED)
// 		return -1;

// 	return 0;
// }

// int ser_rk_isp_set_hlc_level(int fd) {
// 	int err = 0;
// 	int id;
// 	int value;

// 	if (sock_read(fd, &id, sizeof(id)) == SOCKERR_CLOSED)
// 		return -1;
// 	if (sock_read(fd, &value, sizeof(value)) == SOCKERR_CLOSED)
// 		return -1;
// 	LOG_DEBUG("value is %d\n", value);
// 	err = rk_isp_set_hlc_level(id, value);
// 	if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED)
// 		return -1;

// 	return 0;
// }

// int ser_rk_isp_get_dark_boost_level(int fd) {
// 	int err = 0;
// 	int id;
// 	int value;

// 	if (sock_read(fd, &id, sizeof(id)) == SOCKERR_CLOSED)
// 		return -1;
// 	err = rk_isp_get_dark_boost_level(id, &value);
// 	LOG_DEBUG("value is %d\n", value);
// 	if (sock_write(fd, &value, sizeof(value)) == SOCKERR_CLOSED)
// 		return -1;
// 	if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED)
// 		return -1;

// 	return 0;
// }

// int ser_rk_isp_set_dark_boost_level(int fd) {
// 	int err = 0;
// 	int id;
// 	int value;

// 	if (sock_read(fd, &id, sizeof(id)) == SOCKERR_CLOSED)
// 		return -1;
// 	if (sock_read(fd, &value, sizeof(value)) == SOCKERR_CLOSED)
// 		return -1;
// 	LOG_DEBUG("value is %d\n", value);
// 	err = rk_isp_set_dark_boost_level(id, value);
// 	if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED)
// 		return -1;

// 	return 0;
// }

// // isp white_blance
// int ser_rk_isp_get_white_blance_style(int fd) {
// 	int err = 0;
// 	int id, len;
// 	const char *value;

// 	if (sock_read(fd, &id, sizeof(id)) == SOCKERR_CLOSED)
// 		return -1;
// 	err = rk_isp_get_white_blance_style(id, &value);
// 	len = strlen(value);
// 	LOG_DEBUG("len is %d, value is %s, addr is %p\n", len, value, value);
// 	if (sock_write(fd, &len, sizeof(len)) == SOCKERR_CLOSED)
// 		return -1;
// 	if (sock_write(fd, value, len) == SOCKERR_CLOSED)
// 		return -1;
// 	if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED)
// 		return -1;

// 	return 0;
// }

// int ser_rk_isp_set_white_blance_style(int fd) {
// 	int ret = 0;
// 	int id, len;
// 	char *value = NULL;

// 	if (sock_read(fd, &id, sizeof(id)) == SOCKERR_CLOSED)
// 		return -1;
// 	if (sock_read(fd, &len, sizeof(len)) == SOCKERR_CLOSED)
// 		return -1;
// 	if (len) {
// 		value = (char *)malloc(len);
// 		if (sock_read(fd, value, len) == SOCKERR_CLOSED) {
// 			free(value);
// 			return -1;
// 		}
// 		LOG_DEBUG("id is %d, value is %s\n", id, value);
// 		ret = rk_isp_set_white_blance_style(id, value);
// 		free(value);
// 		if (sock_write(fd, &ret, sizeof(int)) == SOCKERR_CLOSED)
// 			return -1;
// 	}

// 	return 0;
// }

// int ser_rk_isp_get_white_blance_red(int fd) {
// 	int err = 0;
// 	int id;
// 	int value;

// 	if (sock_read(fd, &id, sizeof(id)) == SOCKERR_CLOSED)
// 		return -1;
// 	err = rk_isp_get_white_blance_red(id, &value);
// 	LOG_DEBUG("value is %d\n", value);
// 	if (sock_write(fd, &value, sizeof(value)) == SOCKERR_CLOSED)
// 		return -1;
// 	if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED)
// 		return -1;

// 	return 0;
// }

// int ser_rk_isp_set_white_blance_red(int fd) {
// 	int err = 0;
// 	int id;
// 	int value;

// 	if (sock_read(fd, &id, sizeof(id)) == SOCKERR_CLOSED)
// 		return -1;
// 	if (sock_read(fd, &value, sizeof(value)) == SOCKERR_CLOSED)
// 		return -1;
// 	LOG_DEBUG("value is %d\n", value);
// 	err = rk_isp_set_white_blance_red(id, value);
// 	if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED)
// 		return -1;

// 	return 0;
// }

// int ser_rk_isp_get_white_blance_green(int fd) {
// 	int err = 0;
// 	int id;
// 	int value;

// 	if (sock_read(fd, &id, sizeof(id)) == SOCKERR_CLOSED)
// 		return -1;
// 	err = rk_isp_get_white_blance_green(id, &value);
// 	LOG_DEBUG("value is %d\n", value);
// 	if (sock_write(fd, &value, sizeof(value)) == SOCKERR_CLOSED)
// 		return -1;
// 	if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED)
// 		return -1;

// 	return 0;
// }

// int ser_rk_isp_set_white_blance_green(int fd) {
// 	int err = 0;
// 	int id;
// 	int value;

// 	if (sock_read(fd, &id, sizeof(id)) == SOCKERR_CLOSED)
// 		return -1;
// 	if (sock_read(fd, &value, sizeof(value)) == SOCKERR_CLOSED)
// 		return -1;
// 	LOG_DEBUG("value is %d\n", value);
// 	err = rk_isp_set_white_blance_green(id, value);
// 	if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED)
// 		return -1;

// 	return 0;
// }

// int ser_rk_isp_get_white_blance_blue(int fd) {
// 	int err = 0;
// 	int id;
// 	int value;

// 	if (sock_read(fd, &id, sizeof(id)) == SOCKERR_CLOSED)
// 		return -1;
// 	err = rk_isp_get_white_blance_blue(id, &value);
// 	LOG_DEBUG("value is %d\n", value);
// 	if (sock_write(fd, &value, sizeof(value)) == SOCKERR_CLOSED)
// 		return -1;
// 	if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED)
// 		return -1;

// 	return 0;
// }

// int ser_rk_isp_set_white_blance_blue(int fd) {
// 	int err = 0;
// 	int id;
// 	int value;

// 	if (sock_read(fd, &id, sizeof(id)) == SOCKERR_CLOSED)
// 		return -1;
// 	if (sock_read(fd, &value, sizeof(value)) == SOCKERR_CLOSED)
// 		return -1;
// 	LOG_DEBUG("value is %d\n", value);
// 	err = rk_isp_set_white_blance_blue(id, value);
// 	if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED)
// 		return -1;

// 	return 0;
// }

// // isp enhancement
// int ser_rk_isp_get_noise_reduce_mode(int fd) {
// 	int err = 0;
// 	int id, len;
// 	const char *value;

// 	if (sock_read(fd, &id, sizeof(id)) == SOCKERR_CLOSED)
// 		return -1;
// 	err = rk_isp_get_noise_reduce_mode(id, &value);
// 	len = strlen(value);
// 	LOG_DEBUG("len is %d, value is %s, addr is %p\n", len, value, value);
// 	if (sock_write(fd, &len, sizeof(len)) == SOCKERR_CLOSED)
// 		return -1;
// 	if (sock_write(fd, value, len) == SOCKERR_CLOSED)
// 		return -1;
// 	if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED)
// 		return -1;

// 	return 0;
// }

// int ser_rk_isp_set_noise_reduce_mode(int fd) {
// 	int ret = 0;
// 	int id, len;
// 	char *value = NULL;

// 	if (sock_read(fd, &id, sizeof(id)) == SOCKERR_CLOSED)
// 		return -1;
// 	if (sock_read(fd, &len, sizeof(len)) == SOCKERR_CLOSED)
// 		return -1;
// 	if (len) {
// 		value = (char *)malloc(len);
// 		if (sock_read(fd, value, len) == SOCKERR_CLOSED) {
// 			free(value);
// 			return -1;
// 		}
// 		LOG_DEBUG("id is %d, value is %s\n", id, value);
// 		ret = rk_isp_set_noise_reduce_mode(id, value);
// 		free(value);
// 		if (sock_write(fd, &ret, sizeof(int)) == SOCKERR_CLOSED)
// 			return -1;
// 	}

// 	return 0;
// }

// int ser_rk_isp_get_dehaze(int fd) {
// 	int err = 0;
// 	int id, len;
// 	const char *value;

// 	if (sock_read(fd, &id, sizeof(id)) == SOCKERR_CLOSED)
// 		return -1;
// 	err = rk_isp_get_dehaze(id, &value);
// 	len = strlen(value);
// 	LOG_DEBUG("len is %d, value is %s, addr is %p\n", len, value, value);
// 	if (sock_write(fd, &len, sizeof(len)) == SOCKERR_CLOSED)
// 		return -1;
// 	if (sock_write(fd, value, len) == SOCKERR_CLOSED)
// 		return -1;
// 	if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED)
// 		return -1;

// 	return 0;
// }

// int ser_rk_isp_set_dehaze(int fd) {
// 	int ret = 0;
// 	int id, len;
// 	char *value = NULL;

// 	if (sock_read(fd, &id, sizeof(id)) == SOCKERR_CLOSED)
// 		return -1;
// 	if (sock_read(fd, &len, sizeof(len)) == SOCKERR_CLOSED)
// 		return -1;
// 	if (len) {
// 		value = (char *)malloc(len);
// 		if (sock_read(fd, value, len) == SOCKERR_CLOSED) {
// 			free(value);
// 			return -1;
// 		}
// 		LOG_DEBUG("id is %d, value is %s\n", id, value);
// 		ret = rk_isp_set_dehaze(id, value);
// 		free(value);
// 		if (sock_write(fd, &ret, sizeof(int)) == SOCKERR_CLOSED)
// 			return -1;
// 	}

// 	return 0;
// }

// int ser_rk_isp_get_gray_scale_mode(int fd) {
// 	int err = 0;
// 	int id, len;
// 	const char *value;

// 	if (sock_read(fd, &id, sizeof(id)) == SOCKERR_CLOSED)
// 		return -1;
// 	err = rk_isp_get_gray_scale_mode(id, &value);
// 	len = strlen(value);
// 	LOG_DEBUG("len is %d, value is %s, addr is %p\n", len, value, value);
// 	if (sock_write(fd, &len, sizeof(len)) == SOCKERR_CLOSED)
// 		return -1;
// 	if (sock_write(fd, value, len) == SOCKERR_CLOSED)
// 		return -1;
// 	if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED)
// 		return -1;

// 	return 0;
// }

// int ser_rk_isp_set_gray_scale_mode(int fd) {
// 	int ret = 0;
// 	int id, len;
// 	char *value = NULL;

// 	if (sock_read(fd, &id, sizeof(id)) == SOCKERR_CLOSED)
// 		return -1;
// 	if (sock_read(fd, &len, sizeof(len)) == SOCKERR_CLOSED)
// 		return -1;
// 	if (len) {
// 		value = (char *)malloc(len);
// 		if (sock_read(fd, value, len) == SOCKERR_CLOSED) {
// 			free(value);
// 			return -1;
// 		}
// 		LOG_DEBUG("id is %d, value is %s\n", id, value);
// 		ret = rk_isp_set_gray_scale_mode(id, value);
// 		free(value);
// 		if (sock_write(fd, &ret, sizeof(int)) == SOCKERR_CLOSED)
// 			return -1;
// 	}

// 	return 0;
// }

// int ser_rk_isp_get_distortion_correction(int fd) {
// 	int err = 0;
// 	int id, len;
// 	const char *value;

// 	if (sock_read(fd, &id, sizeof(id)) == SOCKERR_CLOSED)
// 		return -1;
// 	err = rk_isp_get_distortion_correction(id, &value);
// 	len = strlen(value);
// 	LOG_DEBUG("len is %d, value is %s, addr is %p\n", len, value, value);
// 	if (sock_write(fd, &len, sizeof(len)) == SOCKERR_CLOSED)
// 		return -1;
// 	if (sock_write(fd, value, len) == SOCKERR_CLOSED)
// 		return -1;
// 	if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED)
// 		return -1;

// 	return 0;
// }

// int ser_rk_isp_set_distortion_correction(int fd) {
// 	int ret = 0;
// 	int id, len;
// 	char *value = NULL;

// 	if (sock_read(fd, &id, sizeof(id)) == SOCKERR_CLOSED)
// 		return -1;
// 	if (sock_read(fd, &len, sizeof(len)) == SOCKERR_CLOSED)
// 		return -1;
// 	if (len) {
// 		value = (char *)malloc(len);
// 		if (sock_read(fd, value, len) == SOCKERR_CLOSED) {
// 			free(value);
// 			return -1;
// 		}
// 		LOG_DEBUG("id is %d, value is %s\n", id, value);
// 		ret = rk_isp_set_distortion_correction(id, value);
// 		free(value);
// 		if (sock_write(fd, &ret, sizeof(int)) == SOCKERR_CLOSED)
// 			return -1;
// 	}

// 	return 0;
// }

// int ser_rk_isp_get_spatial_denoise_level(int fd) {
// 	int err = 0;
// 	int id;
// 	int value;

// 	if (sock_read(fd, &id, sizeof(id)) == SOCKERR_CLOSED)
// 		return -1;
// 	err = rk_isp_get_spatial_denoise_level(id, &value);
// 	LOG_DEBUG("value is %d\n", value);
// 	if (sock_write(fd, &value, sizeof(value)) == SOCKERR_CLOSED)
// 		return -1;
// 	if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED)
// 		return -1;

// 	return 0;
// }

// int ser_rk_isp_set_spatial_denoise_level(int fd) {
// 	int err = 0;
// 	int id;
// 	int value;

// 	if (sock_read(fd, &id, sizeof(id)) == SOCKERR_CLOSED)
// 		return -1;
// 	if (sock_read(fd, &value, sizeof(value)) == SOCKERR_CLOSED)
// 		return -1;
// 	LOG_DEBUG("value is %d\n", value);
// 	err = rk_isp_set_spatial_denoise_level(id, value);
// 	if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED)
// 		return -1;

// 	return 0;
// }

// int ser_rk_isp_get_temporal_denoise_level(int fd) {
// 	int err = 0;
// 	int id;
// 	int value;

// 	if (sock_read(fd, &id, sizeof(id)) == SOCKERR_CLOSED)
// 		return -1;
// 	err = rk_isp_get_temporal_denoise_level(id, &value);
// 	LOG_DEBUG("value is %d\n", value);
// 	if (sock_write(fd, &value, sizeof(value)) == SOCKERR_CLOSED)
// 		return -1;
// 	if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED)
// 		return -1;

// 	return 0;
// }

// int ser_rk_isp_set_temporal_denoise_level(int fd) {
// 	int err = 0;
// 	int id;
// 	int value;

// 	if (sock_read(fd, &id, sizeof(id)) == SOCKERR_CLOSED)
// 		return -1;
// 	if (sock_read(fd, &value, sizeof(value)) == SOCKERR_CLOSED)
// 		return -1;
// 	LOG_DEBUG("value is %d\n", value);
// 	err = rk_isp_set_temporal_denoise_level(id, value);
// 	if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED)
// 		return -1;

// 	return 0;
// }

// int ser_rk_isp_get_dehaze_level(int fd) {
// 	int err = 0;
// 	int id;
// 	int value;

// 	if (sock_read(fd, &id, sizeof(id)) == SOCKERR_CLOSED)
// 		return -1;
// 	err = rk_isp_get_dehaze_level(id, &value);
// 	LOG_DEBUG("value is %d\n", value);
// 	if (sock_write(fd, &value, sizeof(value)) == SOCKERR_CLOSED)
// 		return -1;
// 	if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED)
// 		return -1;

// 	return 0;
// }

// int ser_rk_isp_set_dehaze_level(int fd) {
// 	int err = 0;
// 	int id;
// 	int value;

// 	if (sock_read(fd, &id, sizeof(id)) == SOCKERR_CLOSED)
// 		return -1;
// 	if (sock_read(fd, &value, sizeof(value)) == SOCKERR_CLOSED)
// 		return -1;
// 	LOG_DEBUG("value is %d\n", value);
// 	err = rk_isp_set_dehaze_level(id, value);
// 	if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED)
// 		return -1;

// 	return 0;
// }

// int ser_rk_isp_get_fec_level(int fd) {
// 	int err = 0;
// 	int id;
// 	int value;

// 	if (sock_read(fd, &id, sizeof(id)) == SOCKERR_CLOSED)
// 		return -1;
// 	err = rk_isp_get_fec_level(id, &value);
// 	LOG_DEBUG("value is %d\n", value);
// 	if (sock_write(fd, &value, sizeof(value)) == SOCKERR_CLOSED)
// 		return -1;
// 	if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED)
// 		return -1;

// 	return 0;
// }

// int ser_rk_isp_set_fec_level(int fd) {
// 	int err = 0;
// 	int id;
// 	int value;

// 	if (sock_read(fd, &id, sizeof(id)) == SOCKERR_CLOSED)
// 		return -1;
// 	if (sock_read(fd, &value, sizeof(value)) == SOCKERR_CLOSED)
// 		return -1;
// 	LOG_DEBUG("value is %d\n", value);
// 	err = rk_isp_set_fec_level(id, value);
// 	if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED)
// 		return -1;

// 	return 0;
// }

// int ser_rk_isp_get_ldch_level(int fd) {
// 	int err = 0;
// 	int id;
// 	int value;

// 	if (sock_read(fd, &id, sizeof(id)) == SOCKERR_CLOSED)
// 		return -1;
// 	err = rk_isp_get_ldch_level(id, &value);
// 	LOG_DEBUG("value is %d\n", value);
// 	if (sock_write(fd, &value, sizeof(value)) == SOCKERR_CLOSED)
// 		return -1;
// 	if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED)
// 		return -1;

// 	return 0;
// }

// int ser_rk_isp_set_ldch_level(int fd) {
// 	int err = 0;
// 	int id;
// 	int value;

// 	if (sock_read(fd, &id, sizeof(id)) == SOCKERR_CLOSED)
// 		return -1;
// 	if (sock_read(fd, &value, sizeof(value)) == SOCKERR_CLOSED)
// 		return -1;
// 	LOG_DEBUG("value is %d\n", value);
// 	err = rk_isp_set_ldch_level(id, value);
// 	if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED)
// 		return -1;

// 	return 0;
// }

// // isp video_adjustment

// int ser_rk_isp_get_power_line_frequency_mode(int fd) {
// 	int err = 0;
// 	int id, len;
// 	const char *value;

// 	if (sock_read(fd, &id, sizeof(id)) == SOCKERR_CLOSED)
// 		return -1;
// 	err = rk_isp_get_power_line_frequency_mode(id, &value);
// 	len = strlen(value);
// 	LOG_DEBUG("len is %d, value is %s, addr is %p\n", len, value, value);
// 	if (sock_write(fd, &len, sizeof(len)) == SOCKERR_CLOSED)
// 		return -1;
// 	if (sock_write(fd, value, len) == SOCKERR_CLOSED)
// 		return -1;
// 	if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED)
// 		return -1;

// 	return 0;
// }

// int ser_rk_isp_set_power_line_frequency_mode(int fd) {
// 	int ret = 0;
// 	int id, len;
// 	char *value = NULL;

// 	if (sock_read(fd, &id, sizeof(id)) == SOCKERR_CLOSED)
// 		return -1;
// 	if (sock_read(fd, &len, sizeof(len)) == SOCKERR_CLOSED)
// 		return -1;
// 	if (len) {
// 		value = (char *)malloc(len);
// 		if (sock_read(fd, value, len) == SOCKERR_CLOSED) {
// 			free(value);
// 			return -1;
// 		}
// 		LOG_DEBUG("id is %d, value is %s\n", id, value);
// 		ret = rk_isp_set_power_line_frequency_mode(id, value);
// 		free(value);
// 		if (sock_write(fd, &ret, sizeof(int)) == SOCKERR_CLOSED)
// 			return -1;
// 	}

// 	return 0;
// }

// int ser_rk_isp_get_image_flip(int fd) {
// 	int err = 0;
// 	int id, len;
// 	const char *value;

// 	if (sock_read(fd, &id, sizeof(id)) == SOCKERR_CLOSED)
// 		return -1;
// 	err = rk_isp_get_image_flip(id, &value);
// 	len = strlen(value);
// 	LOG_DEBUG("len is %d, value is %s, addr is %p\n", len, value, value);
// 	if (sock_write(fd, &len, sizeof(len)) == SOCKERR_CLOSED)
// 		return -1;
// 	if (sock_write(fd, value, len) == SOCKERR_CLOSED)
// 		return -1;
// 	if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED)
// 		return -1;

// 	return 0;
// }

// int ser_rk_isp_set_image_flip(int fd) {
// 	int ret = 0;
// 	int id, len;
// 	char *value = NULL;

// 	if (sock_read(fd, &id, sizeof(id)) == SOCKERR_CLOSED)
// 		return -1;
// 	if (sock_read(fd, &len, sizeof(len)) == SOCKERR_CLOSED)
// 		return -1;
// 	if (len) {
// 		value = (char *)malloc(len);
// 		if (sock_read(fd, value, len) == SOCKERR_CLOSED) {
// 			free(value);
// 			return -1;
// 		}
// 		LOG_DEBUG("id is %d, value is %s\n", id, value);
// 		ret = rk_isp_set_image_flip(id, value);
// 		free(value);
// 		if (sock_write(fd, &ret, sizeof(int)) == SOCKERR_CLOSED)
// 			return -1;
// 	}

// 	return 0;
// }

// // auto focus
// int ser_rk_isp_get_af_mode(int fd) {
// 	int err = 0;
// 	int id, len;
// 	const char *value;

// 	if (sock_read(fd, &id, sizeof(id)) == SOCKERR_CLOSED)
// 		return -1;
// 	err = rk_isp_get_af_mode(id, &value);
// 	len = strlen(value);
// 	LOG_DEBUG("len is %d, value is %s, addr is %p\n", len, value, value);
// 	if (sock_write(fd, &len, sizeof(len)) == SOCKERR_CLOSED)
// 		return -1;
// 	if (sock_write(fd, value, len) == SOCKERR_CLOSED)
// 		return -1;
// 	if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED)
// 		return -1;

// 	return 0;
// }

// int ser_rk_isp_set_af_mode(int fd) {
// 	int ret = 0;
// 	int id, len;
// 	char *value = NULL;

// 	if (sock_read(fd, &id, sizeof(id)) == SOCKERR_CLOSED)
// 		return -1;
// 	if (sock_read(fd, &len, sizeof(len)) == SOCKERR_CLOSED)
// 		return -1;
// 	if (len) {
// 		value = (char *)malloc(len);
// 		if (sock_read(fd, value, len) == SOCKERR_CLOSED) {
// 			free(value);
// 			return -1;
// 		}
// 		LOG_DEBUG("id is %d, value is %s\n", id, value);
// 		ret = rk_isp_set_af_mode(id, value);
// 		free(value);
// 		if (sock_write(fd, &ret, sizeof(int)) == SOCKERR_CLOSED)
// 			return -1;
// 	}

// 	return 0;
// }

// int ser_rk_isp_get_zoom_level(int fd) {
// 	int err = 0;
// 	int id;
// 	int value;

// 	if (sock_read(fd, &id, sizeof(id)) == SOCKERR_CLOSED)
// 		return -1;
// 	err = rk_isp_get_zoom_level(id, &value);
// 	LOG_DEBUG("value is %d\n", value);
// 	if (sock_write(fd, &value, sizeof(value)) == SOCKERR_CLOSED)
// 		return -1;
// 	if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED)
// 		return -1;

// 	return 0;
// }

// int ser_rk_isp_get_focus_level(int fd) {
// 	int err = 0;
// 	int id;
// 	int value;

// 	if (sock_read(fd, &id, sizeof(id)) == SOCKERR_CLOSED)
// 		return -1;
// 	err = rk_isp_get_focus_level(id, &value);
// 	LOG_DEBUG("value is %d\n", value);
// 	if (sock_write(fd, &value, sizeof(value)) == SOCKERR_CLOSED)
// 		return -1;
// 	if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED)
// 		return -1;

// 	return 0;
// }

// int ser_rk_isp_af_zoom_in(int fd) {
// 	int ret = 0;
// 	int id;

// 	if (sock_read(fd, &id, sizeof(id)) == SOCKERR_CLOSED)
// 		return -1;
// 	LOG_DEBUG("id is %d\n", id);
// 	ret = rk_isp_af_zoom_in(id);
// 	if (sock_write(fd, &ret, sizeof(int)) == SOCKERR_CLOSED)
// 		return -1;

// 	return 0;
// }

// int ser_rk_isp_af_zoom_out(int fd) {
// 	int ret = 0;
// 	int id;

// 	if (sock_read(fd, &id, sizeof(id)) == SOCKERR_CLOSED)
// 		return -1;
// 	LOG_DEBUG("id is %d\n", id);
// 	ret = rk_isp_af_zoom_out(id);
// 	if (sock_write(fd, &ret, sizeof(int)) == SOCKERR_CLOSED)
// 		return -1;

// 	return 0;
// }

// int ser_rk_isp_af_focus_in(int fd) {
// 	int ret = 0;
// 	int id;

// 	if (sock_read(fd, &id, sizeof(id)) == SOCKERR_CLOSED)
// 		return -1;
// 	LOG_DEBUG("id is %d\n", id);
// 	ret = rk_isp_af_focus_in(id);
// 	if (sock_write(fd, &ret, sizeof(int)) == SOCKERR_CLOSED)
// 		return -1;

// 	return 0;
// }

// int ser_rk_isp_af_focus_out(int fd) {
// 	int ret = 0;
// 	int id;

// 	if (sock_read(fd, &id, sizeof(id)) == SOCKERR_CLOSED)
// 		return -1;
// 	LOG_DEBUG("id is %d\n", id);
// 	ret = rk_isp_af_focus_out(id);
// 	if (sock_write(fd, &ret, sizeof(int)) == SOCKERR_CLOSED)
// 		return -1;

// 	return 0;
// }

// int ser_rk_isp_af_focus_once(int fd) {
// 	int ret = 0;
// 	int id;

// 	if (sock_read(fd, &id, sizeof(id)) == SOCKERR_CLOSED)
// 		return -1;
// 	LOG_DEBUG("id is %d\n", id);
// 	ret = rk_isp_af_focus_once(id);
// 	if (sock_write(fd, &ret, sizeof(int)) == SOCKERR_CLOSED)
// 		return -1;

// 	return 0;
// }

// // audio
// int ser_rk_audio_restart(int fd) {
// 	int err = 0;

// 	LOG_DEBUG("restart begin\n");
// 	err = rk_audio_restart();
// 	LOG_DEBUG("restart end\n");
// 	if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED)
// 		return -1;

// 	return 0;
// }

// int ser_rkipc_audio_deinit(int fd){
// 	int err = 0;

// 	LOG_DEBUG("restart begin\n");
// 	err = rkipc_audio_deinit();
// 	LOG_DEBUG("restart end\n");
// 	if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED)
// 		return -1;

// 	return 0;
// }

// int ser_rk_audio_get_bit_rate(int fd) {
// 	int err = 0;
// 	int id;
// 	int value;

// 	if (sock_read(fd, &id, sizeof(id)) == SOCKERR_CLOSED)
// 		return -1;
// 	err = rk_audio_get_bit_rate(id, &value);
// 	LOG_DEBUG("value is %d\n", value);
// 	if (sock_write(fd, &value, sizeof(value)) == SOCKERR_CLOSED)
// 		return -1;
// 	if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED)
// 		return -1;

// 	return 0;
// }

// int ser_rk_audio_set_bit_rate(int fd) {
// 	int err = 0;
// 	int id;
// 	int value;

// 	if (sock_read(fd, &id, sizeof(id)) == SOCKERR_CLOSED)
// 		return -1;
// 	if (sock_read(fd, &value, sizeof(value)) == SOCKERR_CLOSED)
// 		return -1;
// 	LOG_DEBUG("value is %d\n", value);
// 	err = rk_audio_set_bit_rate(id, value);
// 	if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED)
// 		return -1;

// 	return 0;
// }

// int ser_rk_audio_get_sample_rate(int fd) {
// 	int err = 0;
// 	int id;
// 	int value;

// 	if (sock_read(fd, &id, sizeof(id)) == SOCKERR_CLOSED)
// 		return -1;
// 	err = rk_audio_get_sample_rate(id, &value);
// 	LOG_DEBUG("value is %d\n", value);
// 	if (sock_write(fd, &value, sizeof(value)) == SOCKERR_CLOSED)
// 		return -1;
// 	if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED)
// 		return -1;

// 	return 0;
// }

// int ser_rk_audio_set_sample_rate(int fd) {
// 	int err = 0;
// 	int id;
// 	int value;

// 	if (sock_read(fd, &id, sizeof(id)) == SOCKERR_CLOSED)
// 		return -1;
// 	if (sock_read(fd, &value, sizeof(value)) == SOCKERR_CLOSED)
// 		return -1;
// 	LOG_DEBUG("value is %d\n", value);
// 	err = rk_audio_set_sample_rate(id, value);
// 	if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED)
// 		return -1;

// 	return 0;
// }

// int ser_rk_audio_get_volume(int fd) {
// 	int err = 0;
// 	int id;
// 	int value;

// 	if (sock_read(fd, &id, sizeof(id)) == SOCKERR_CLOSED)
// 		return -1;
// 	err = rk_audio_get_volume(id, &value);
// 	LOG_DEBUG("value is %d\n", value);
// 	if (sock_write(fd, &value, sizeof(value)) == SOCKERR_CLOSED)
// 		return -1;
// 	if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED)
// 		return -1;

// 	return 0;
// }

// int ser_rk_audio_set_volume(int fd) {
// 	int err = 0;
// 	int id;
// 	int value;

// 	if (sock_read(fd, &id, sizeof(id)) == SOCKERR_CLOSED)
// 		return -1;
// 	if (sock_read(fd, &value, sizeof(value)) == SOCKERR_CLOSED)
// 		return -1;
// 	LOG_DEBUG("value is %d\n", value);
// 	err = rk_audio_set_volume(id, value);
// 	if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED)
// 		return -1;

// 	return 0;
// }

// int ser_rk_audio_get_enable_vqe(int fd) {
// 	int err = 0;
// 	int id;
// 	int value;

// 	if (sock_read(fd, &id, sizeof(id)) == SOCKERR_CLOSED)
// 		return -1;
// 	err = rk_audio_get_enable_vqe(id, &value);
// 	LOG_DEBUG("value is %d\n", value);
// 	if (sock_write(fd, &value, sizeof(value)) == SOCKERR_CLOSED)
// 		return -1;
// 	if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED)
// 		return -1;

// 	return 0;
// }

// int ser_rk_audio_set_enable_vqe(int fd) {
// 	int err = 0;
// 	int id;
// 	int value;

// 	if (sock_read(fd, &id, sizeof(id)) == SOCKERR_CLOSED)
// 		return -1;
// 	if (sock_read(fd, &value, sizeof(value)) == SOCKERR_CLOSED)
// 		return -1;
// 	LOG_DEBUG("value is %d\n", value);
// 	err = rk_audio_set_enable_vqe(id, value);
// 	if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED)
// 		return -1;

// 	return 0;
// }

// int ser_rk_audio_get_encode_type(int fd) {
// 	int err = 0;
// 	int id, len;
// 	const char *value;

// 	if (sock_read(fd, &id, sizeof(id)) == SOCKERR_CLOSED)
// 		return -1;
// 	err = rk_audio_get_encode_type(id, &value);
// 	len = strlen(value);
// 	LOG_DEBUG("len is %d, value is %s, addr is %p\n", len, value, value);
// 	if (sock_write(fd, &len, sizeof(len)) == SOCKERR_CLOSED)
// 		return -1;
// 	if (sock_write(fd, value, len) == SOCKERR_CLOSED)
// 		return -1;
// 	if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED)
// 		return -1;

// 	return 0;
// }

// int ser_rk_audio_set_encode_type(int fd) {
// 	int ret = 0;
// 	int id, len;
// 	char *value = NULL;

// 	if (sock_read(fd, &id, sizeof(id)) == SOCKERR_CLOSED)
// 		return -1;
// 	if (sock_read(fd, &len, sizeof(len)) == SOCKERR_CLOSED)
// 		return -1;
// 	if (len) {
// 		value = (char *)malloc(len);
// 		if (sock_read(fd, value, len) == SOCKERR_CLOSED) {
// 			free(value);
// 			return -1;
// 		}
// 		LOG_DEBUG("id is %d, value is %s\n", id, value);
// 		ret = rk_audio_set_encode_type(id, value);
// 		free(value);
// 		if (sock_write(fd, &ret, sizeof(int)) == SOCKERR_CLOSED)
// 			return -1;
// 	}

// 	return 0;
// }

// // video
// int ser_rk_video_restart(int fd) {
// 	int err = 0;

// 	LOG_DEBUG("restart begin\n");
// 	err = rk_video_restart();
// 	LOG_DEBUG("restart end\n");
// 	if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED)
// 		return -1;

// 	return 0;
// }

// int ser_rk_video_get_gop(int fd) {
// 	int err = 0;
// 	int id;
// 	int value;

// 	if (sock_read(fd, &id, sizeof(id)) == SOCKERR_CLOSED)
// 		return -1;
// 	err = rk_video_get_gop(id, &value);
// 	LOG_DEBUG("value is %d\n", value);
// 	if (sock_write(fd, &value, sizeof(value)) == SOCKERR_CLOSED)
// 		return -1;
// 	if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED)
// 		return -1;

// 	return 0;
// }

// int ser_rk_video_set_gop(int fd) {
// 	int err = 0;
// 	int id;
// 	int value;

// 	if (sock_read(fd, &id, sizeof(id)) == SOCKERR_CLOSED)
// 		return -1;
// 	if (sock_read(fd, &value, sizeof(value)) == SOCKERR_CLOSED)
// 		return -1;
// 	LOG_DEBUG("value is %d\n", value);
// 	err = rk_video_set_gop(id, value);
// 	if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED)
// 		return -1;

// 	return 0;
// }

// int ser_rk_video_get_max_rate(int fd) {
// 	int err = 0;
// 	int id;
// 	int value;

// 	if (sock_read(fd, &id, sizeof(id)) == SOCKERR_CLOSED)
// 		return -1;
// 	err = rk_video_get_max_rate(id, &value);
// 	LOG_DEBUG("value is %d\n", value);
// 	if (sock_write(fd, &value, sizeof(value)) == SOCKERR_CLOSED)
// 		return -1;
// 	if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED)
// 		return -1;

// 	return 0;
// }

// int ser_rk_video_set_max_rate(int fd) {
// 	int err = 0;
// 	int id;
// 	int value;

// 	if (sock_read(fd, &id, sizeof(id)) == SOCKERR_CLOSED)
// 		return -1;
// 	if (sock_read(fd, &value, sizeof(value)) == SOCKERR_CLOSED)
// 		return -1;
// 	LOG_DEBUG("value is %d\n", value);
// 	err = rk_video_set_max_rate(id, value);
// 	if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED)
// 		return -1;

// 	return 0;
// }

// int ser_rk_video_get_RC_mode(int fd) {
// 	int err = 0;
// 	int id, len;
// 	const char *value;

// 	if (sock_read(fd, &id, sizeof(id)) == SOCKERR_CLOSED)
// 		return -1;
// 	err = rk_video_get_RC_mode(id, &value);
// 	len = strlen(value);
// 	LOG_DEBUG("len is %d, value is %s, addr is %p\n", len, value, value);
// 	if (sock_write(fd, &len, sizeof(len)) == SOCKERR_CLOSED)
// 		return -1;
// 	if (sock_write(fd, value, len) == SOCKERR_CLOSED)
// 		return -1;
// 	if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED)
// 		return -1;

// 	return 0;
// }

// int ser_rk_video_set_RC_mode(int fd) {
// 	int ret = 0;
// 	int id, len;
// 	char *value = NULL;

// 	if (sock_read(fd, &id, sizeof(id)) == SOCKERR_CLOSED)
// 		return -1;
// 	if (sock_read(fd, &len, sizeof(len)) == SOCKERR_CLOSED)
// 		return -1;
// 	if (len) {
// 		value = (char *)malloc(len);
// 		if (sock_read(fd, value, len) == SOCKERR_CLOSED) {
// 			free(value);
// 			return -1;
// 		}
// 		LOG_DEBUG("id is %d, value is %s\n", id, value);
// 		ret = rk_video_set_RC_mode(id, value);
// 		free(value);
// 		if (sock_write(fd, &ret, sizeof(int)) == SOCKERR_CLOSED)
// 			return -1;
// 	}

// 	return 0;
// }

// int ser_rk_video_get_output_data_type(int fd) {
// 	int err = 0;
// 	int id, len;
// 	const char *value;

// 	if (sock_read(fd, &id, sizeof(id)) == SOCKERR_CLOSED)
// 		return -1;
// 	err = rk_video_get_output_data_type(id, &value);
// 	len = strlen(value);
// 	LOG_DEBUG("len is %d, value is %s, addr is %p\n", len, value, value);
// 	if (sock_write(fd, &len, sizeof(len)) == SOCKERR_CLOSED)
// 		return -1;
// 	if (sock_write(fd, value, len) == SOCKERR_CLOSED)
// 		return -1;
// 	if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED)
// 		return -1;

// 	return 0;
// }

// int ser_rk_video_set_output_data_type(int fd) {
// 	int ret = 0;
// 	int id, len;
// 	char *value = NULL;

// 	if (sock_read(fd, &id, sizeof(id)) == SOCKERR_CLOSED)
// 		return -1;
// 	if (sock_read(fd, &len, sizeof(len)) == SOCKERR_CLOSED)
// 		return -1;
// 	if (len) {
// 		value = (char *)malloc(len);
// 		if (sock_read(fd, value, len) == SOCKERR_CLOSED) {
// 			free(value);
// 			return -1;
// 		}
// 		LOG_DEBUG("id is %d, value is %s\n", id, value);
// 		ret = rk_video_set_output_data_type(id, value);
// 		free(value);
// 		if (sock_write(fd, &ret, sizeof(int)) == SOCKERR_CLOSED)
// 			return -1;
// 	}

// 	return 0;
// }

// int ser_rk_video_get_rc_quality(int fd) {
// 	int err = 0;
// 	int id, len;
// 	const char *value;

// 	if (sock_read(fd, &id, sizeof(id)) == SOCKERR_CLOSED)
// 		return -1;
// 	err = rk_video_get_rc_quality(id, &value);
// 	len = strlen(value);
// 	LOG_DEBUG("len is %d, value is %s, addr is %p\n", len, value, value);
// 	if (sock_write(fd, &len, sizeof(len)) == SOCKERR_CLOSED)
// 		return -1;
// 	if (sock_write(fd, value, len) == SOCKERR_CLOSED)
// 		return -1;
// 	if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED)
// 		return -1;

// 	return 0;
// }

// int ser_rk_video_set_rc_quality(int fd) {
// 	int ret = 0;
// 	int id, len;
// 	char *value = NULL;

// 	if (sock_read(fd, &id, sizeof(id)) == SOCKERR_CLOSED)
// 		return -1;
// 	if (sock_read(fd, &len, sizeof(len)) == SOCKERR_CLOSED)
// 		return -1;
// 	if (len) {
// 		value = (char *)malloc(len);
// 		if (sock_read(fd, value, len) == SOCKERR_CLOSED) {
// 			free(value);
// 			return -1;
// 		}
// 		LOG_DEBUG("id is %d, value is %s\n", id, value);
// 		ret = rk_video_set_rc_quality(id, value);
// 		free(value);
// 		if (sock_write(fd, &ret, sizeof(int)) == SOCKERR_CLOSED)
// 			return -1;
// 	}

// 	return 0;
// }

// // int ser_rk_video_get_smart(int fd) {
// // 	int err = 0;
// // 	int id, len;
// // 	const char *value;

// // 	if (sock_read(fd, &id, sizeof(id)) == SOCKERR_CLOSED)
// // 		return -1;
// // 	err = rk_video_get_smart(id, &value);
// // 	len = strlen(value);
// // 	LOG_DEBUG("len is %d, value is %s, addr is %p\n", len, value, value);
// // 	if (sock_write(fd, &len, sizeof(len)) == SOCKERR_CLOSED)
// // 		return -1;
// // 	if (sock_write(fd, value, len) == SOCKERR_CLOSED)
// // 		return -1;
// // 	if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED)
// // 		return -1;

// // 	return 0;
// // }

// // int ser_rk_video_set_smart(int fd) {
// // 	int ret = 0;
// // 	int id, len;
// // 	char *value = NULL;

// // 	if (sock_read(fd, &id, sizeof(id)) == SOCKERR_CLOSED)
// // 		return -1;
// // 	if (sock_read(fd, &len, sizeof(len)) == SOCKERR_CLOSED)
// // 		return -1;
// // 	if (len) {
// // 		value = (char *)malloc(len);
// // 		if (sock_read(fd, value, len) == SOCKERR_CLOSED) {
// // 			free(value);
// // 			return -1;
// // 		}
// // 		LOG_DEBUG("id is %d, value is %s\n", id, value);
// // 		ret = rk_video_set_smart(id, value);
// // 		free(value);
// // 		if (sock_write(fd, &ret, sizeof(int)) == SOCKERR_CLOSED)
// // 			return -1;
// // 	}

// // 	return 0;
// // }

// // int ser_rk_video_get_gop_mode(int fd) {
// // 	int err = 0;
// // 	int id, len;
// // 	const char *value;

// // 	if (sock_read(fd, &id, sizeof(id)) == SOCKERR_CLOSED)
// // 		return -1;
// // 	err = rk_video_get_gop_mode(id, &value);
// // 	len = strlen(value);
// // 	LOG_DEBUG("len is %d, value is %s, addr is %p\n", len, value, value);
// // 	if (sock_write(fd, &len, sizeof(len)) == SOCKERR_CLOSED)
// // 		return -1;
// // 	if (sock_write(fd, value, len) == SOCKERR_CLOSED)
// // 		return -1;
// // 	if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED)
// // 		return -1;

// // 	return 0;
// // }

// // int ser_rk_video_set_gop_mode(int fd) {
// // 	int ret = 0;
// // 	int id, len;
// // 	char *value = NULL;

// // 	if (sock_read(fd, &id, sizeof(id)) == SOCKERR_CLOSED)
// // 		return -1;
// // 	if (sock_read(fd, &len, sizeof(len)) == SOCKERR_CLOSED)
// // 		return -1;
// // 	if (len) {
// // 		value = (char *)malloc(len);
// // 		if (sock_read(fd, value, len) == SOCKERR_CLOSED) {
// // 			free(value);
// // 			return -1;
// // 		}
// // 		LOG_DEBUG("id is %d, value is %s\n", id, value);
// // 		ret = rk_video_set_gop_mode(id, value);
// // 		free(value);
// // 		if (sock_write(fd, &ret, sizeof(int)) == SOCKERR_CLOSED)
// // 			return -1;
// // 	}

// // 	return 0;
// // }

// int ser_rk_video_get_stream_type(int fd) {
// 	int err = 0;
// 	int id, len;
// 	const char *value;

// 	if (sock_read(fd, &id, sizeof(id)) == SOCKERR_CLOSED)
// 		return -1;
// 	err = rk_video_get_stream_type(id, &value);
// 	len = strlen(value);
// 	LOG_DEBUG("len is %d, value is %s, addr is %p\n", len, value, value);
// 	if (sock_write(fd, &len, sizeof(len)) == SOCKERR_CLOSED)
// 		return -1;
// 	if (sock_write(fd, value, len) == SOCKERR_CLOSED)
// 		return -1;
// 	if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED)
// 		return -1;

// 	return 0;
// }

// int ser_rk_video_set_stream_type(int fd) {
// 	int ret = 0;
// 	int id, len;
// 	char *value = NULL;

// 	if (sock_read(fd, &id, sizeof(id)) == SOCKERR_CLOSED)
// 		return -1;
// 	if (sock_read(fd, &len, sizeof(len)) == SOCKERR_CLOSED)
// 		return -1;
// 	if (len) {
// 		value = (char *)malloc(len);
// 		if (sock_read(fd, value, len) == SOCKERR_CLOSED) {
// 			free(value);
// 			return -1;
// 		}
// 		LOG_DEBUG("id is %d, value is %s\n", id, value);
// 		ret = rk_video_set_stream_type(id, value);
// 		free(value);
// 		if (sock_write(fd, &ret, sizeof(int)) == SOCKERR_CLOSED)
// 			return -1;
// 	}

// 	return 0;
// }

// // int ser_rk_video_get_h264_profile(int fd) {
// // 	int err = 0;
// // 	int id, len;
// // 	const char *value;

// // 	if (sock_read(fd, &id, sizeof(id)) == SOCKERR_CLOSED)
// // 		return -1;
// // 	err = rk_video_get_h264_profile(id, &value);
// // 	len = strlen(value);
// // 	LOG_DEBUG("len is %d, value is %s, addr is %p\n", len, value, value);
// // 	if (sock_write(fd, &len, sizeof(len)) == SOCKERR_CLOSED)
// // 		return -1;
// // 	if (sock_write(fd, value, len) == SOCKERR_CLOSED)
// // 		return -1;
// // 	if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED)
// // 		return -1;

// // 	return 0;
// // }

// // int ser_rk_video_set_h264_profile(int fd) {
// // 	int ret = 0;
// // 	int id, len;
// // 	char *value = NULL;

// // 	if (sock_read(fd, &id, sizeof(id)) == SOCKERR_CLOSED)
// // 		return -1;
// // 	if (sock_read(fd, &len, sizeof(len)) == SOCKERR_CLOSED)
// // 		return -1;
// // 	if (len) {
// // 		value = (char *)malloc(len);
// // 		if (sock_read(fd, value, len) == SOCKERR_CLOSED) {
// // 			free(value);
// // 			return -1;
// // 		}
// // 		LOG_DEBUG("id is %d, value is %s\n", id, value);
// // 		ret = rk_video_set_h264_profile(id, value);
// // 		free(value);
// // 		if (sock_write(fd, &ret, sizeof(int)) == SOCKERR_CLOSED)
// // 			return -1;
// // 	}

// // 	return 0;
// // }

// int ser_rk_video_get_resolution(int fd) {
// 	int err = 0;
// 	int id, len;
// 	char *value = malloc(20);

// 	if (sock_read(fd, &id, sizeof(id)) == SOCKERR_CLOSED)
// 		return -1;
// 	err = rk_video_get_resolution(id, &value);
// 	len = strlen(value);
// 	LOG_DEBUG("len is %d, value is %s, addr is %p\n", len, value, value);
// 	if (sock_write(fd, &len, sizeof(len)) == SOCKERR_CLOSED)
// 		return -1;
// 	if (sock_write(fd, value, len) == SOCKERR_CLOSED)
// 		return -1;
// 	if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED)
// 		return -1;
// 	free(value);

// 	return 0;
// }

// int ser_rk_video_set_resolution(int fd) {
// 	int ret = 0;
// 	int id, len;
// 	char *value = NULL;

// 	if (sock_read(fd, &id, sizeof(id)) == SOCKERR_CLOSED)
// 		return -1;
// 	if (sock_read(fd, &len, sizeof(len)) == SOCKERR_CLOSED)
// 		return -1;
// 	if (len) {
// 		value = (char *)malloc(len);
// 		if (sock_read(fd, value, len) == SOCKERR_CLOSED) {
// 			free(value);
// 			return -1;
// 		}
// 		LOG_DEBUG("id is %d, value is %s\n", id, value);
// 		ret = rk_video_set_resolution(id, value);
// 		free(value);
// 		if (sock_write(fd, &ret, sizeof(int)) == SOCKERR_CLOSED)
// 			return -1;
// 	}

// 	return 0;
// }

// int ser_rk_video_get_frame_rate(int fd) {
// 	int err = 0;
// 	int id, len;
// 	char *value = malloc(20);

// 	if (sock_read(fd, &id, sizeof(id)) == SOCKERR_CLOSED)
// 		return -1;
// 	err = rk_video_get_frame_rate(id, &value);
// 	len = strlen(value);
// 	LOG_DEBUG("len is %d, value is %s, addr is %p\n", len, value, value);
// 	if (sock_write(fd, &len, sizeof(len)) == SOCKERR_CLOSED)
// 		return -1;
// 	if (sock_write(fd, value, len) == SOCKERR_CLOSED)
// 		return -1;
// 	if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED)
// 		return -1;
// 	free(value);

// 	return 0;
// }

// int ser_rk_video_set_frame_rate(int fd) {
// 	int ret = 0;
// 	int id, len;
// 	char *value = NULL;

// 	if (sock_read(fd, &id, sizeof(id)) == SOCKERR_CLOSED)
// 		return -1;
// 	if (sock_read(fd, &len, sizeof(len)) == SOCKERR_CLOSED)
// 		return -1;
// 	if (len) {
// 		value = (char *)malloc(len);
// 		if (sock_read(fd, value, len) == SOCKERR_CLOSED) {
// 			free(value);
// 			return -1;
// 		}
// 		LOG_DEBUG("id is %d, value is %s\n", id, value);
// 		ret = rk_video_set_frame_rate(id, value);
// 		free(value);
// 		if (sock_write(fd, &ret, sizeof(int)) == SOCKERR_CLOSED)
// 			return -1;
// 	}

// 	return 0;
// }

// // int ser_rk_video_get_frame_rate_in(int fd) {
// // 	int err = 0;
// // 	int id, len;
// // 	char *value = malloc(20);

// // 	if (sock_read(fd, &id, sizeof(id)) == SOCKERR_CLOSED)
// // 		return -1;
// // 	err = rk_video_get_frame_rate_in(id, &value);
// // 	len = strlen(value);
// // 	LOG_DEBUG("len is %d, value is %s, addr is %p\n", len, value, value);
// // 	if (sock_write(fd, &len, sizeof(len)) == SOCKERR_CLOSED)
// // 		return -1;
// // 	if (sock_write(fd, value, len) == SOCKERR_CLOSED)
// // 		return -1;
// // 	if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED)
// // 		return -1;
// // 	free(value);

// // 	return 0;
// // }

// // int ser_rk_video_set_frame_rate_in(int fd) {
// // 	int ret = 0;
// // 	int id, len;
// // 	char *value = NULL;

// // 	if (sock_read(fd, &id, sizeof(id)) == SOCKERR_CLOSED)
// // 		return -1;
// // 	if (sock_read(fd, &len, sizeof(len)) == SOCKERR_CLOSED)
// // 		return -1;
// // 	if (len) {
// // 		value = (char *)malloc(len);
// // 		if (sock_read(fd, value, len) == SOCKERR_CLOSED) {
// // 			free(value);
// // 			return -1;
// // 		}
// // 		LOG_DEBUG("id is %d, value is %s\n", id, value);
// // 		ret = rk_video_set_frame_rate_in(id, value);
// // 		free(value);
// // 		if (sock_write(fd, &ret, sizeof(int)) == SOCKERR_CLOSED)
// // 			return -1;
// // 	}

// // 	return 0;
// // }

// int ser_rk_video_get_rotation(int fd) {
// 	int err = 0;
// 	int value;

// 	err = rk_video_get_rotation(&value);
// 	LOG_DEBUG("value is %d\n", value);
// 	if (sock_write(fd, &value, sizeof(value)) == SOCKERR_CLOSED)
// 		return -1;
// 	if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED)
// 		return -1;

// 	return 0;
// }

// int ser_rk_video_set_rotation(int fd) {
// 	int err = 0;
// 	int value;

// 	if (sock_read(fd, &value, sizeof(value)) == SOCKERR_CLOSED)
// 		return -1;
// 	LOG_DEBUG("value is %d\n", value);
// 	err = rk_video_set_rotation(value);
// 	if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED)
// 		return -1;

// 	return 0;
// }

// // jpeg

// // int ser_rk_video_get_enable_cycle_snapshot(int fd) {
// // 	int err = 0;
// // 	int value;

// // 	err = rk_video_get_enable_cycle_snapshot(&value);
// // 	LOG_DEBUG("value is %d\n", value);
// // 	if (sock_write(fd, &value, sizeof(value)) == SOCKERR_CLOSED)
// // 		return -1;
// // 	if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED)
// // 		return -1;

// // 	return 0;
// // }

// // int ser_rk_video_set_enable_cycle_snapshot(int fd) {
// // 	int err = 0;
// // 	int value;

// // 	if (sock_read(fd, &value, sizeof(value)) == SOCKERR_CLOSED)
// // 		return -1;
// // 	LOG_DEBUG("value is %d\n", value);
// // 	err = rk_video_set_enable_cycle_snapshot(value);
// // 	if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED)
// // 		return -1;

// // 	return 0;
// // }

// int ser_rk_video_get_image_quality(int fd) {
// 	int err = 0;
// 	int value;

// 	err = rk_video_get_image_quality(&value);
// 	LOG_DEBUG("value is %d\n", value);
// 	if (sock_write(fd, &value, sizeof(value)) == SOCKERR_CLOSED)
// 		return -1;
// 	if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED)
// 		return -1;

// 	return 0;
// }

// int ser_rk_video_set_image_quality(int fd) {
// 	int err = 0;
// 	int value;

// 	if (sock_read(fd, &value, sizeof(value)) == SOCKERR_CLOSED)
// 		return -1;
// 	LOG_DEBUG("value is %d\n", value);
// 	err = rk_video_set_image_quality(value);
// 	if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED)
// 		return -1;

// 	return 0;
// }

// int ser_rk_video_get_snapshot_interval_ms(int fd) {
// 	int err = 0;
// 	int value;

// 	err = rk_video_get_snapshot_interval_ms(&value);
// 	LOG_DEBUG("value is %d\n", value);
// 	if (sock_write(fd, &value, sizeof(value)) == SOCKERR_CLOSED)
// 		return -1;
// 	if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED)
// 		return -1;

// 	return 0;
// }

// int ser_rk_video_set_snapshot_interval_ms(int fd) {
// 	int err = 0;
// 	int value;

// 	if (sock_read(fd, &value, sizeof(value)) == SOCKERR_CLOSED)
// 		return -1;
// 	LOG_DEBUG("value is %d\n", value);
// 	err = rk_video_set_snapshot_interval_ms(value);
// 	if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED)
// 		return -1;

// 	return 0;
// }

// int ser_rk_video_get_jpeg_resolution(int fd) {
// 	int err = 0;
// 	int len;
// 	char *value = malloc(20);

// 	err = rk_video_get_jpeg_resolution(&value);
// 	len = strlen(value);
// 	LOG_DEBUG("len is %d, value is %s, addr is %p\n", len, value, value);
// 	if (sock_write(fd, &len, sizeof(len)) == SOCKERR_CLOSED)
// 		return -1;
// 	if (sock_write(fd, value, len) == SOCKERR_CLOSED)
// 		return -1;
// 	if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED)
// 		return -1;
// 	free(value);

// 	return 0;
// }

// int ser_rk_video_set_jpeg_resolution(int fd) {
// 	int ret = 0;
// 	int len;
// 	char *value = NULL;

// 	if (sock_read(fd, &len, sizeof(len)) == SOCKERR_CLOSED)
// 		return -1;
// 	if (len) {
// 		value = (char *)malloc(len);
// 		if (sock_read(fd, value, len) == SOCKERR_CLOSED) {
// 			free(value);
// 			return -1;
// 		}
// 		LOG_DEBUG("value is %s\n", value);
// 		ret = rk_video_set_jpeg_resolution(value);
// 		free(value);
// 		if (sock_write(fd, &ret, sizeof(int)) == SOCKERR_CLOSED)
// 			return -1;
// 	}

// 	return 0;
// }

// int ser_rk_osd_get_is_presistent_text(int fd) {
// 	int err = 0;
// 	int value;

// 	err = rk_osd_get_is_presistent_text(&value);
// 	LOG_DEBUG("value is %d\n", value);
// 	if (sock_write(fd, &value, sizeof(value)) == SOCKERR_CLOSED)
// 		return -1;
// 	if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED)
// 		return -1;

// 	return 0;
// }

// int ser_rk_osd_set_is_presistent_text(int fd) {
// 	int err = 0;
// 	int value;

// 	if (sock_read(fd, &value, sizeof(value)) == SOCKERR_CLOSED)
// 		return -1;
// 	LOG_DEBUG("value is %d\n", value);
// 	err = rk_osd_set_is_presistent_text(value);
// 	if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED)
// 		return -1;

// 	return 0;
// }

// int ser_rk_osd_get_font_size(int fd) {
// 	int err = 0;
// 	int value;

// 	err = rk_osd_get_font_size(&value);
// 	LOG_DEBUG("value is %d\n", value);
// 	if (sock_write(fd, &value, sizeof(value)) == SOCKERR_CLOSED)
// 		return -1;
// 	if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED)
// 		return -1;

// 	return 0;
// }

// int ser_rk_osd_set_font_size(int fd) {
// 	int err = 0;
// 	int value;

// 	if (sock_read(fd, &value, sizeof(value)) == SOCKERR_CLOSED)
// 		return -1;
// 	LOG_DEBUG("value is %d\n", value);
// 	err = rk_osd_set_font_size(value);
// 	if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED)
// 		return -1;

// 	return 0;
// }

// int ser_rk_osd_get_boundary(int fd) {
// 	int err = 0;
// 	int value;

// 	err = rk_osd_get_boundary(&value);
// 	LOG_DEBUG("value is %d\n", value);
// 	if (sock_write(fd, &value, sizeof(value)) == SOCKERR_CLOSED)
// 		return -1;
// 	if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED)
// 		return -1;

// 	return 0;
// }

// int ser_rk_osd_set_boundary(int fd) {
// 	int err = 0;
// 	int value;

// 	if (sock_read(fd, &value, sizeof(value)) == SOCKERR_CLOSED)
// 		return -1;
// 	LOG_DEBUG("value is %d\n", value);
// 	err = rk_osd_set_boundary(value);
// 	if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED)
// 		return -1;

// 	return 0;
// }

// int ser_rk_osd_get_normalized_screen_width(int fd) {
// 	int err = 0;
// 	int value;

// 	err = rk_osd_get_normalized_screen_width(&value);
// 	LOG_DEBUG("value is %d\n", value);
// 	if (sock_write(fd, &value, sizeof(value)) == SOCKERR_CLOSED)
// 		return -1;
// 	if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED)
// 		return -1;

// 	return 0;
// }

// int ser_rk_osd_get_normalized_screen_height(int fd) {
// 	int err = 0;
// 	int value;

// 	err = rk_osd_get_normalized_screen_height(&value);
// 	LOG_DEBUG("value is %d\n", value);
// 	if (sock_write(fd, &value, sizeof(value)) == SOCKERR_CLOSED)
// 		return -1;
// 	if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED)
// 		return -1;

// 	return 0;
// }

// int ser_rk_osd_get_attribute(int fd) {
// 	int err = 0;
// 	int len;
// 	const char *value;

// 	err = rk_osd_get_attribute(&value);
// 	len = strlen(value);
// 	LOG_DEBUG("len is %d, value is %s, addr is %p\n", len, value, value);
// 	if (sock_write(fd, &len, sizeof(len)) == SOCKERR_CLOSED)
// 		return -1;
// 	if (sock_write(fd, value, len) == SOCKERR_CLOSED)
// 		return -1;
// 	if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED)
// 		return -1;

// 	return 0;
// }

// int ser_rk_osd_set_attribute(int fd) {
// 	int ret = 0;
// 	int len;
// 	char *value = NULL;

// 	if (sock_read(fd, &len, sizeof(len)) == SOCKERR_CLOSED)
// 		return -1;
// 	if (len) {
// 		value = (char *)malloc(len);
// 		if (sock_read(fd, value, len) == SOCKERR_CLOSED) {
// 			free(value);
// 			return -1;
// 		}
// 		LOG_DEBUG("value is %s\n", value);
// 		ret = rk_osd_set_attribute(value);
// 		free(value);
// 		if (sock_write(fd, &ret, sizeof(int)) == SOCKERR_CLOSED)
// 			return -1;
// 	}

// 	return 0;
// }

// int ser_rk_osd_get_font_color_mode(int fd) {
// 	int err = 0;
// 	int len;
// 	const char *value;

// 	err = rk_osd_get_font_color_mode(&value);
// 	len = strlen(value);
// 	LOG_DEBUG("len is %d, value is %s, addr is %p\n", len, value, value);
// 	if (sock_write(fd, &len, sizeof(len)) == SOCKERR_CLOSED)
// 		return -1;
// 	if (sock_write(fd, value, len) == SOCKERR_CLOSED)
// 		return -1;
// 	if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED)
// 		return -1;

// 	return 0;
// }

// int ser_rk_osd_set_font_color_mode(int fd) {
// 	int ret = 0;
// 	int len;
// 	char *value = NULL;

// 	if (sock_read(fd, &len, sizeof(len)) == SOCKERR_CLOSED)
// 		return -1;
// 	if (len) {
// 		value = (char *)malloc(len);
// 		if (sock_read(fd, value, len) == SOCKERR_CLOSED) {
// 			free(value);
// 			return -1;
// 		}
// 		LOG_DEBUG("value is %s\n", value);
// 		ret = rk_osd_set_font_color_mode(value);
// 		free(value);
// 		if (sock_write(fd, &ret, sizeof(int)) == SOCKERR_CLOSED)
// 			return -1;
// 	}

// 	return 0;
// }

// int ser_rk_osd_get_font_color(int fd) {
// 	int err = 0;
// 	int len;
// 	const char *value;

// 	err = rk_osd_get_font_color(&value);
// 	len = strlen(value);
// 	LOG_DEBUG("len is %d, value is %s, addr is %p\n", len, value, value);
// 	if (sock_write(fd, &len, sizeof(len)) == SOCKERR_CLOSED)
// 		return -1;
// 	if (sock_write(fd, value, len) == SOCKERR_CLOSED)
// 		return -1;
// 	if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED)
// 		return -1;

// 	return 0;
// }

// int ser_rk_osd_set_font_color(int fd) {
// 	int ret = 0;
// 	int len;
// 	char *value = NULL;

// 	if (sock_read(fd, &len, sizeof(len)) == SOCKERR_CLOSED)
// 		return -1;
// 	if (len) {
// 		value = (char *)malloc(len);
// 		if (sock_read(fd, value, len) == SOCKERR_CLOSED) {
// 			free(value);
// 			return -1;
// 		}
// 		LOG_DEBUG("value is %s\n", value);
// 		ret = rk_osd_set_font_color(value);
// 		free(value);
// 		if (sock_write(fd, &ret, sizeof(int)) == SOCKERR_CLOSED)
// 			return -1;
// 	}

// 	return 0;
// }

// int ser_rk_osd_get_alignment(int fd) {
// 	int err = 0;
// 	int len;
// 	const char *value;

// 	err = rk_osd_get_alignment(&value);
// 	len = strlen(value);
// 	LOG_DEBUG("len is %d, value is %s, addr is %p\n", len, value, value);
// 	if (sock_write(fd, &len, sizeof(len)) == SOCKERR_CLOSED)
// 		return -1;
// 	if (sock_write(fd, value, len) == SOCKERR_CLOSED)
// 		return -1;
// 	if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED)
// 		return -1;

// 	return 0;
// }

// int ser_rk_osd_set_alignment(int fd) {
// 	int ret = 0;
// 	int len;
// 	char *value = NULL;

// 	if (sock_read(fd, &len, sizeof(len)) == SOCKERR_CLOSED)
// 		return -1;
// 	if (len) {
// 		value = (char *)malloc(len);
// 		if (sock_read(fd, value, len) == SOCKERR_CLOSED) {
// 			free(value);
// 			return -1;
// 		}
// 		LOG_DEBUG("value is %s\n", value);
// 		ret = rk_osd_set_alignment(value);
// 		free(value);
// 		if (sock_write(fd, &ret, sizeof(int)) == SOCKERR_CLOSED)
// 			return -1;
// 	}

// 	return 0;
// }

// int ser_rk_osd_get_font_path(int fd) {
// 	int err = 0;
// 	int len;
// 	const char *value;

// 	err = rk_osd_get_font_path(&value);
// 	len = strlen(value);
// 	LOG_DEBUG("len is %d, value is %s, addr is %p\n", len, value, value);
// 	if (sock_write(fd, &len, sizeof(len)) == SOCKERR_CLOSED)
// 		return -1;
// 	if (sock_write(fd, value, len) == SOCKERR_CLOSED)
// 		return -1;
// 	if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED)
// 		return -1;

// 	return 0;
// }

// int ser_rk_osd_set_font_path(int fd) {
// 	int ret = 0;
// 	int len;
// 	char *value = NULL;

// 	if (sock_read(fd, &len, sizeof(len)) == SOCKERR_CLOSED)
// 		return -1;
// 	if (len) {
// 		value = (char *)malloc(len);
// 		if (sock_read(fd, value, len) == SOCKERR_CLOSED) {
// 			free(value);
// 			return -1;
// 		}
// 		LOG_DEBUG("value is %s\n", value);
// 		ret = rk_osd_set_font_path(value);
// 		free(value);
// 		if (sock_write(fd, &ret, sizeof(int)) == SOCKERR_CLOSED)
// 			return -1;
// 	}

// 	return 0;
// }

// int ser_rk_osd_get_enabled(int fd) {
// 	int err = 0;
// 	int id;
// 	int value;

// 	if (sock_read(fd, &id, sizeof(id)) == SOCKERR_CLOSED)
// 		return -1;
// 	err = rk_osd_get_enabled(id, &value);
// 	LOG_DEBUG("value is %d\n", value);
// 	if (sock_write(fd, &value, sizeof(value)) == SOCKERR_CLOSED)
// 		return -1;
// 	if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED)
// 		return -1;

// 	return 0;
// }

// int ser_rk_osd_set_enabled(int fd) {
// 	int err = 0;
// 	int id;
// 	int value;

// 	if (sock_read(fd, &id, sizeof(id)) == SOCKERR_CLOSED)
// 		return -1;
// 	if (sock_read(fd, &value, sizeof(value)) == SOCKERR_CLOSED)
// 		return -1;
// 	LOG_DEBUG("value is %d\n", value);
// 	err = rk_osd_set_enabled(id, value);
// 	if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED)
// 		return -1;

// 	return 0;
// }

// int ser_rk_osd_get_position_x(int fd) {
// 	int err = 0;
// 	int id;
// 	int value;

// 	if (sock_read(fd, &id, sizeof(id)) == SOCKERR_CLOSED)
// 		return -1;
// 	err = rk_osd_get_position_x(id, &value);
// 	LOG_DEBUG("value is %d\n", value);
// 	if (sock_write(fd, &value, sizeof(value)) == SOCKERR_CLOSED)
// 		return -1;
// 	if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED)
// 		return -1;

// 	return 0;
// }

// int ser_rk_osd_set_position_x(int fd) {
// 	int err = 0;
// 	int id;
// 	int value;

// 	if (sock_read(fd, &id, sizeof(id)) == SOCKERR_CLOSED)
// 		return -1;
// 	if (sock_read(fd, &value, sizeof(value)) == SOCKERR_CLOSED)
// 		return -1;
// 	LOG_DEBUG("value is %d\n", value);
// 	err = rk_osd_set_position_x(id, value);
// 	if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED)
// 		return -1;

// 	return 0;
// }

// int ser_rk_osd_get_position_y(int fd) {
// 	int err = 0;
// 	int id;
// 	int value;

// 	if (sock_read(fd, &id, sizeof(id)) == SOCKERR_CLOSED)
// 		return -1;
// 	err = rk_osd_get_position_y(id, &value);
// 	LOG_DEBUG("value is %d\n", value);
// 	if (sock_write(fd, &value, sizeof(value)) == SOCKERR_CLOSED)
// 		return -1;
// 	if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED)
// 		return -1;

// 	return 0;
// }

// int ser_rk_osd_set_position_y(int fd) {
// 	int err = 0;
// 	int id;
// 	int value;

// 	if (sock_read(fd, &id, sizeof(id)) == SOCKERR_CLOSED)
// 		return -1;
// 	if (sock_read(fd, &value, sizeof(value)) == SOCKERR_CLOSED)
// 		return -1;
// 	LOG_DEBUG("value is %d\n", value);
// 	err = rk_osd_set_position_y(id, value);
// 	if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED)
// 		return -1;

// 	return 0;
// }

// int ser_rk_osd_get_height(int fd) {
// 	int err = 0;
// 	int id;
// 	int value;

// 	if (sock_read(fd, &id, sizeof(id)) == SOCKERR_CLOSED)
// 		return -1;
// 	err = rk_osd_get_height(id, &value);
// 	LOG_DEBUG("value is %d\n", value);
// 	if (sock_write(fd, &value, sizeof(value)) == SOCKERR_CLOSED)
// 		return -1;
// 	if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED)
// 		return -1;

// 	return 0;
// }

// int ser_rk_osd_set_height(int fd) {
// 	int err = 0;
// 	int id;
// 	int value;

// 	if (sock_read(fd, &id, sizeof(id)) == SOCKERR_CLOSED)
// 		return -1;
// 	if (sock_read(fd, &value, sizeof(value)) == SOCKERR_CLOSED)
// 		return -1;
// 	LOG_DEBUG("value is %d\n", value);
// 	err = rk_osd_set_height(id, value);
// 	if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED)
// 		return -1;

// 	return 0;
// }

// int ser_rk_osd_get_width(int fd) {
// 	int err = 0;
// 	int id;
// 	int value;

// 	if (sock_read(fd, &id, sizeof(id)) == SOCKERR_CLOSED)
// 		return -1;
// 	err = rk_osd_get_width(id, &value);
// 	LOG_DEBUG("value is %d\n", value);
// 	if (sock_write(fd, &value, sizeof(value)) == SOCKERR_CLOSED)
// 		return -1;
// 	if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED)
// 		return -1;

// 	return 0;
// }

// int ser_rk_osd_set_width(int fd) {
// 	int err = 0;
// 	int id;
// 	int value;

// 	if (sock_read(fd, &id, sizeof(id)) == SOCKERR_CLOSED)
// 		return -1;
// 	if (sock_read(fd, &value, sizeof(value)) == SOCKERR_CLOSED)
// 		return -1;
// 	LOG_DEBUG("value is %d\n", value);
// 	err = rk_osd_set_width(id, value);
// 	if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED)
// 		return -1;

// 	return 0;
// }

// int ser_rk_osd_get_display_week_enabled(int fd) {
// 	int err = 0;
// 	int id;
// 	int value;

// 	if (sock_read(fd, &id, sizeof(id)) == SOCKERR_CLOSED)
// 		return -1;
// 	err = rk_osd_get_display_week_enabled(id, &value);
// 	LOG_DEBUG("value is %d\n", value);
// 	if (sock_write(fd, &value, sizeof(value)) == SOCKERR_CLOSED)
// 		return -1;
// 	if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED)
// 		return -1;

// 	return 0;
// }

// int ser_rk_osd_set_display_week_enabled(int fd) {
// 	int err = 0;
// 	int id;
// 	int value;

// 	if (sock_read(fd, &id, sizeof(id)) == SOCKERR_CLOSED)
// 		return -1;
// 	if (sock_read(fd, &value, sizeof(value)) == SOCKERR_CLOSED)
// 		return -1;
// 	LOG_DEBUG("value is %d\n", value);
// 	err = rk_osd_set_display_week_enabled(id, value);
// 	if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED)
// 		return -1;

// 	return 0;
// }

// int ser_rk_osd_get_date_style(int fd) {
// 	int err = 0;
// 	int id, len;
// 	const char *value;

// 	if (sock_read(fd, &id, sizeof(id)) == SOCKERR_CLOSED)
// 		return -1;
// 	err = rk_osd_get_date_style(id, &value);
// 	len = strlen(value);
// 	LOG_DEBUG("len is %d, value is %s, addr is %p\n", len, value, value);
// 	if (sock_write(fd, &len, sizeof(len)) == SOCKERR_CLOSED)
// 		return -1;
// 	if (sock_write(fd, value, len) == SOCKERR_CLOSED)
// 		return -1;
// 	if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED)
// 		return -1;

// 	return 0;
// }

// int ser_rk_osd_set_date_style(int fd) {
// 	int ret = 0;
// 	int id, len;
// 	char *value = NULL;

// 	if (sock_read(fd, &id, sizeof(id)) == SOCKERR_CLOSED)
// 		return -1;
// 	if (sock_read(fd, &len, sizeof(len)) == SOCKERR_CLOSED)
// 		return -1;
// 	if (len) {
// 		value = (char *)malloc(len);
// 		if (sock_read(fd, value, len) == SOCKERR_CLOSED) {
// 			free(value);
// 			return -1;
// 		}
// 		LOG_DEBUG("id is %d, value is %s\n", id, value);
// 		ret = rk_osd_set_date_style(id, value);
// 		free(value);
// 		if (sock_write(fd, &ret, sizeof(int)) == SOCKERR_CLOSED)
// 			return -1;
// 	}

// 	return 0;
// }

// int ser_rk_osd_get_time_style(int fd) {
// 	int err = 0;
// 	int id, len;
// 	const char *value;

// 	if (sock_read(fd, &id, sizeof(id)) == SOCKERR_CLOSED)
// 		return -1;
// 	err = rk_osd_get_time_style(id, &value);
// 	len = strlen(value);
// 	LOG_DEBUG("len is %d, value is %s, addr is %p\n", len, value, value);
// 	if (sock_write(fd, &len, sizeof(len)) == SOCKERR_CLOSED)
// 		return -1;
// 	if (sock_write(fd, value, len) == SOCKERR_CLOSED)
// 		return -1;
// 	if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED)
// 		return -1;

// 	return 0;
// }

// int ser_rk_osd_set_time_style(int fd) {
// 	int ret = 0;
// 	int id, len;
// 	char *value = NULL;

// 	if (sock_read(fd, &id, sizeof(id)) == SOCKERR_CLOSED)
// 		return -1;
// 	if (sock_read(fd, &len, sizeof(len)) == SOCKERR_CLOSED)
// 		return -1;
// 	if (len) {
// 		value = (char *)malloc(len);
// 		if (sock_read(fd, value, len) == SOCKERR_CLOSED) {
// 			free(value);
// 			return -1;
// 		}
// 		LOG_DEBUG("id is %d, value is %s\n", id, value);
// 		ret = rk_osd_set_time_style(id, value);
// 		free(value);
// 		if (sock_write(fd, &ret, sizeof(int)) == SOCKERR_CLOSED)
// 			return -1;
// 	}

// 	return 0;
// }

// int ser_rk_osd_get_type(int fd) {
// 	int err = 0;
// 	int id, len;
// 	const char *value;

// 	if (sock_read(fd, &id, sizeof(id)) == SOCKERR_CLOSED)
// 		return -1;
// 	err = rk_osd_get_type(id, &value);
// 	len = strlen(value);
// 	LOG_DEBUG("len is %d, value is %s, addr is %p\n", len, value, value);
// 	if (sock_write(fd, &len, sizeof(len)) == SOCKERR_CLOSED)
// 		return -1;
// 	if (sock_write(fd, value, len) == SOCKERR_CLOSED)
// 		return -1;
// 	if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED)
// 		return -1;

// 	return 0;
// }

// int ser_rk_osd_set_type(int fd) {
// 	int ret = 0;
// 	int id, len;
// 	char *value = NULL;

// 	if (sock_read(fd, &id, sizeof(id)) == SOCKERR_CLOSED)
// 		return -1;
// 	if (sock_read(fd, &len, sizeof(len)) == SOCKERR_CLOSED)
// 		return -1;
// 	if (len) {
// 		value = (char *)malloc(len);
// 		if (sock_read(fd, value, len) == SOCKERR_CLOSED) {
// 			free(value);
// 			return -1;
// 		}
// 		LOG_DEBUG("id is %d, value is %s\n", id, value);
// 		ret = rk_osd_set_type(id, value);
// 		free(value);
// 		if (sock_write(fd, &ret, sizeof(int)) == SOCKERR_CLOSED)
// 			return -1;
// 	}

// 	return 0;
// }

// int ser_rk_osd_get_display_text(int fd) {
// 	int err = 0;
// 	int id, len;
// 	const char *value;

// 	if (sock_read(fd, &id, sizeof(id)) == SOCKERR_CLOSED)
// 		return -1;
// 	err = rk_osd_get_display_text(id, &value);
// 	len = strlen(value);
// 	LOG_DEBUG("len is %d, value is %s, addr is %p\n", len, value, value);
// 	if (sock_write(fd, &len, sizeof(len)) == SOCKERR_CLOSED)
// 		return -1;
// 	if (sock_write(fd, value, len) == SOCKERR_CLOSED)
// 		return -1;
// 	if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED)
// 		return -1;

// 	return 0;
// }

// int ser_rk_osd_set_display_text(int fd) {
// 	int ret = 0;
// 	int id, len;
// 	char *value = NULL;

// 	if (sock_read(fd, &id, sizeof(id)) == SOCKERR_CLOSED)
// 		return -1;
// 	if (sock_read(fd, &len, sizeof(len)) == SOCKERR_CLOSED)
// 		return -1;
// 	if (len) {
// 		value = (char *)malloc(len);
// 		if (sock_read(fd, value, len) == SOCKERR_CLOSED) {
// 			free(value);
// 			return -1;
// 		}
// 		LOG_DEBUG("id is %d, value is %s\n", id, value);
// 		ret = rk_osd_set_display_text(id, value);
// 		free(value);
// 		if (sock_write(fd, &ret, sizeof(int)) == SOCKERR_CLOSED)
// 			return -1;
// 	}

// 	return 0;
// }

// int ser_rk_osd_get_image_path(int fd) {
// 	int err = 0;
// 	int id, len;
// 	const char *value;

// 	if (sock_read(fd, &id, sizeof(id)) == SOCKERR_CLOSED)
// 		return -1;
// 	err = rk_osd_get_image_path(id, &value);
// 	len = strlen(value);
// 	LOG_DEBUG("len is %d, value is %s, addr is %p\n", len, value, value);
// 	if (sock_write(fd, &len, sizeof(len)) == SOCKERR_CLOSED)
// 		return -1;
// 	if (sock_write(fd, value, len) == SOCKERR_CLOSED)
// 		return -1;
// 	if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED)
// 		return -1;

// 	return 0;
// }

// int ser_rk_osd_get_font_color_of_channelName(int fd){
// 	int err = 0;
// 	int len;
// 	const char *value;
// 	err = rk_osd_get_font_color_of_channelName(&value);
// 	len = strlen(value);
// 	LOG_DEBUG("len is %d, value is %s, addr is %p\n", len, value, value);
// 	if (sock_write(fd, &len, sizeof(len)) == SOCKERR_CLOSED)
// 		return -1;
// 	if (sock_write(fd, value, len) == SOCKERR_CLOSED)
// 		return -1;
// 	if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED)
// 		return -1;

// 	return 0;

// }

// int ser_rk_osd_get_font_color_of_dateTime(int fd){
// 	int err = 0;
// 	int len;
// 	const char *value;
// 	err = rk_osd_get_font_color_of_dateTime(&value);
// 	len = strlen(value);
// 	LOG_DEBUG("len is %d, value is %s, addr is %p\n", len, value, value);
// 	if (sock_write(fd, &len, sizeof(len)) == SOCKERR_CLOSED)
// 		return -1;
// 	if (sock_write(fd, value, len) == SOCKERR_CLOSED)
// 		return -1;
// 	if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED)
// 		return -1;

// 	return 0;

// }

// int ser_rk_osd_get_font_color_of_channelLocation(int fd){
// 	int err = 0;
// 	int len;
// 	const char *value;
// 	err = rk_osd_get_font_color_of_channelLocation(&value);
// 	len = strlen(value);
// 	LOG_DEBUG("len is %d, value is %s, addr is %p\n", len, value, value);
// 	if (sock_write(fd, &len, sizeof(len)) == SOCKERR_CLOSED)
// 		return -1;
// 	if (sock_write(fd, value, len) == SOCKERR_CLOSED)
// 		return -1;
// 	if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED)
// 		return -1;

// 	return 0;

// }

// int ser_rk_osd_set_font_color_of_channelName(int fd) {
// 	int ret = 0;
// 	int len;
// 	char *value = NULL;

// 	if (sock_read(fd, &len, sizeof(len)) == SOCKERR_CLOSED)
// 		return -1;
// 	if (len) {
// 		value = (char *)malloc(len);
// 		if (sock_read(fd, value, len) == SOCKERR_CLOSED) {
// 			free(value);
// 			return -1;
// 		}
// 		LOG_DEBUG("value is %s\n", value);
// 		ret = rk_osd_set_font_color_of_channelName(value);
// 		free(value);
// 		if (sock_write(fd, &ret, sizeof(int)) == SOCKERR_CLOSED)
// 			return -1;
// 	}

// 	return 0;
// }

// int ser_rk_osd_set_font_color_of_channelLocation(int fd) {
// 	int ret = 0;
// 	int len;
// 	char *value = NULL;

// 	if (sock_read(fd, &len, sizeof(len)) == SOCKERR_CLOSED)
// 		return -1;
// 	if (len) {
// 		value = (char *)malloc(len);
// 		if (sock_read(fd, value, len) == SOCKERR_CLOSED) {
// 			free(value);
// 			return -1;
// 		}
// 		LOG_DEBUG("value is %s\n", value);
// 		ret = rk_osd_set_font_color_of_channelLocation(value);
// 		free(value);
// 		if (sock_write(fd, &ret, sizeof(int)) == SOCKERR_CLOSED)
// 			return -1;
// 	}

// 	return 0;
// }

// int ser_rk_osd_set_font_color_of_dateTime(int fd) {
// 	int ret = 0;
// 	int len;
// 	char *value = NULL;

// 	if (sock_read(fd, &len, sizeof(len)) == SOCKERR_CLOSED)
// 		return -1;
// 	if (len) {
// 		value = (char *)malloc(len);
// 		if (sock_read(fd, value, len) == SOCKERR_CLOSED) {
// 			free(value);
// 			return -1;
// 		}
// 		LOG_DEBUG("value is %s\n", value);
// 		ret = rk_osd_set_font_color_of_dateTime(value);
// 		free(value);
// 		if (sock_write(fd, &ret, sizeof(int)) == SOCKERR_CLOSED)
// 			return -1;
// 	}

// 	return 0;
// }




// int ser_rk_osd_set_image_path(int fd) {
// 	int ret = 0;
// 	int id, len;
// 	char *value = NULL;

// 	if (sock_read(fd, &id, sizeof(id)) == SOCKERR_CLOSED)
// 		return -1;
// 	if (sock_read(fd, &len, sizeof(len)) == SOCKERR_CLOSED)
// 		return -1;
// 	if (len) {
// 		value = (char *)malloc(len);
// 		if (sock_read(fd, value, len) == SOCKERR_CLOSED) {
// 			free(value);
// 			return -1;
// 		}
// 		LOG_DEBUG("id is %d, value is %s\n", id, value);
// 		ret = rk_osd_set_image_path(id, value);
// 		free(value);
// 		if (sock_write(fd, &ret, sizeof(int)) == SOCKERR_CLOSED)
// 			return -1;
// 	}

// 	return 0;
// }

// int ser_rk_osd_get_style(int fd) {
// 	int err = 0;
// 	int id, len;
// 	const char *value;

// 	if (sock_read(fd, &id, sizeof(id)) == SOCKERR_CLOSED)
// 		return -1;
// 	err = rk_osd_get_style(id, &value);
// 	len = strlen(value);
// 	LOG_DEBUG("len is %d, value is %s, addr is %p\n", len, value, value);
// 	if (sock_write(fd, &len, sizeof(len)) == SOCKERR_CLOSED)
// 		return -1;
// 	if (sock_write(fd, value, len) == SOCKERR_CLOSED)
// 		return -1;
// 	if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED)
// 		return -1;

// 	return 0;
// }

// int ser_rk_osd_set_style(int fd) {
// 	int ret = 0;
// 	int id, len;
// 	char *value = NULL;

// 	if (sock_read(fd, &id, sizeof(id)) == SOCKERR_CLOSED)
// 		return -1;
// 	if (sock_read(fd, &len, sizeof(len)) == SOCKERR_CLOSED)
// 		return -1;
// 	if (len) {
// 		value = (char *)malloc(len);
// 		if (sock_read(fd, value, len) == SOCKERR_CLOSED) {
// 			free(value);
// 			return -1;
// 		}
// 		LOG_DEBUG("id is %d, value is %s\n", id, value);
// 		ret = rk_osd_set_style(id, value);
// 		free(value);
// 		if (sock_write(fd, &ret, sizeof(int)) == SOCKERR_CLOSED)
// 			return -1;
// 	}

// 	return 0;
// }

// int ser_rk_osd_restart(int fd) {
// 	int err = 0;

// 	LOG_DEBUG("restart begin\n");
// 	err = rk_osd_restart();
// 	LOG_DEBUG("restart end\n");
// 	if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED)
// 		return -1;

// 	return 0;
// }

// int ser_rk_IR_monitoring_restart(int fd) {
// 	int err = 0;

// 	LOG_DEBUG("restart begin\n");
// 	rk_isp_restart_IR_monitoring_thread();
// 	LOG_DEBUG("restart end\n");
// 	if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED)
// 		return -1;

// 	return 0;
// }

// int ser_rk_isp_start_IR_monitoring_thread(int fd) {
// 	int err = 0;

// 	LOG_DEBUG("start begin\n");
// 	rk_isp_start_IR_monitoring_thread();
// 	LOG_DEBUG("start end\n");
// 	if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED)
// 		return -1;

// 	return 0;
// }

// int ser_rk_isp_stop_IR_monitoring_thread(int fd) {
// 	int err = 0;

// 	LOG_DEBUG("stop begin\n");
// 	rk_isp_stop_IR_monitoring_thread();
// 	LOG_DEBUG("stop end\n");
// 	if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED)
// 		return -1;

// 	return 0;
// }
	

// // roi.x
// // int ser_rk_roi_get_stream_type(int fd) {
// // 	int err = 0;
// // 	int id, len;
// // 	const char *value;

// // 	if (sock_read(fd, &id, sizeof(id)) == SOCKERR_CLOSED)
// // 		return -1;
// // 	err = rk_roi_get_stream_type(id, &value);
// // 	len = strlen(value);
// // 	LOG_DEBUG("len is %d, value is %s, addr is %p\n", len, value, value);
// // 	if (sock_write(fd, &len, sizeof(len)) == SOCKERR_CLOSED)
// // 		return -1;
// // 	if (sock_write(fd, value, len) == SOCKERR_CLOSED)
// // 		return -1;
// // 	if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED)
// // 		return -1;

// // 	return 0;
// // }

// // int ser_rk_roi_set_stream_type(int fd) {
// // 	int ret = 0;
// // 	int id, len;
// // 	char *value = NULL;

// // 	if (sock_read(fd, &id, sizeof(id)) == SOCKERR_CLOSED)
// // 		return -1;
// // 	if (sock_read(fd, &len, sizeof(len)) == SOCKERR_CLOSED)
// // 		return -1;
// // 	if (len) {
// // 		value = (char *)malloc(len);
// // 		if (sock_read(fd, value, len) == SOCKERR_CLOSED) {
// // 			free(value);
// // 			return -1;
// // 		}
// // 		LOG_DEBUG("id is %d, value is %s\n", id, value);
// // 		ret = rk_roi_set_stream_type(id, value);
// // 		free(value);
// // 		if (sock_write(fd, &ret, sizeof(int)) == SOCKERR_CLOSED)
// // 			return -1;
// // 	}

// // 	return 0;
// // }

// // int ser_rk_roi_get_name(int fd) {
// // 	int err = 0;
// // 	int id, len;
// // 	const char *value;

// // 	if (sock_read(fd, &id, sizeof(id)) == SOCKERR_CLOSED)
// // 		return -1;
// // 	err = rk_roi_get_name(id, &value);
// // 	len = strlen(value);
// // 	LOG_DEBUG("len is %d, value is %s, addr is %p\n", len, value, value);
// // 	if (sock_write(fd, &len, sizeof(len)) == SOCKERR_CLOSED)
// // 		return -1;
// // 	if (sock_write(fd, value, len) == SOCKERR_CLOSED)
// // 		return -1;
// // 	if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED)
// // 		return -1;

// // 	return 0;
// // }

// // int ser_rk_roi_set_name(int fd) {
// // 	int ret = 0;
// // 	int id, len;
// // 	char *value = NULL;

// // 	if (sock_read(fd, &id, sizeof(id)) == SOCKERR_CLOSED)
// // 		return -1;
// // 	if (sock_read(fd, &len, sizeof(len)) == SOCKERR_CLOSED)
// // 		return -1;
// // 	if (len) {
// // 		value = (char *)malloc(len);
// // 		if (sock_read(fd, value, len) == SOCKERR_CLOSED) {
// // 			free(value);
// // 			return -1;
// // 		}
// // 		LOG_DEBUG("id is %d, value is %s\n", id, value);
// // 		ret = rk_roi_set_name(id, value);
// // 		free(value);
// // 		if (sock_write(fd, &ret, sizeof(int)) == SOCKERR_CLOSED)
// // 			return -1;
// // 	}

// // 	return 0;
// // }

// // int ser_rk_roi_get_id(int fd) {
// // 	int err = 0;
// // 	int id;
// // 	int value;

// // 	if (sock_read(fd, &id, sizeof(id)) == SOCKERR_CLOSED)
// // 		return -1;
// // 	err = rk_roi_get_id(id, &value);
// // 	LOG_DEBUG("value is %d\n", value);
// // 	if (sock_write(fd, &value, sizeof(value)) == SOCKERR_CLOSED)
// // 		return -1;
// // 	if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED)
// // 		return -1;

// // 	return 0;
// // }

// // int ser_rk_roi_set_id(int fd) {
// // 	int err = 0;
// // 	int id;
// // 	int value;

// // 	if (sock_read(fd, &id, sizeof(id)) == SOCKERR_CLOSED)
// // 		return -1;
// // 	if (sock_read(fd, &value, sizeof(value)) == SOCKERR_CLOSED)
// // 		return -1;
// // 	LOG_DEBUG("value is %d\n", value);
// // 	err = rk_roi_set_id(id, value);
// // 	if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED)
// // 		return -1;

// // 	return 0;
// // }

// // int ser_rk_roi_get_enabled(int fd) {
// // 	int err = 0;
// // 	int id;
// // 	int value;

// // 	if (sock_read(fd, &id, sizeof(id)) == SOCKERR_CLOSED)
// // 		return -1;
// // 	err = rk_roi_get_enabled(id, &value);
// // 	LOG_DEBUG("value is %d\n", value);
// // 	if (sock_write(fd, &value, sizeof(value)) == SOCKERR_CLOSED)
// // 		return -1;
// // 	if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED)
// // 		return -1;

// // 	return 0;
// // }

// // int ser_rk_roi_set_enabled(int fd) {
// // 	int err = 0;
// // 	int id;
// // 	int value;

// // 	if (sock_read(fd, &id, sizeof(id)) == SOCKERR_CLOSED)
// // 		return -1;
// // 	if (sock_read(fd, &value, sizeof(value)) == SOCKERR_CLOSED)
// // 		return -1;
// // 	LOG_DEBUG("value is %d\n", value);
// // 	err = rk_roi_set_enabled(id, value);
// // 	if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED)
// // 		return -1;

// // 	return 0;
// // }

// // int ser_rk_roi_get_position_x(int fd) {
// // 	int err = 0;
// // 	int id;
// // 	int value;

// // 	if (sock_read(fd, &id, sizeof(id)) == SOCKERR_CLOSED)
// // 		return -1;
// // 	err = rk_roi_get_position_x(id, &value);
// // 	LOG_DEBUG("value is %d\n", value);
// // 	if (sock_write(fd, &value, sizeof(value)) == SOCKERR_CLOSED)
// // 		return -1;
// // 	if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED)
// // 		return -1;

// // 	return 0;
// // }

// // int ser_rk_roi_set_position_x(int fd) {
// // 	int err = 0;
// // 	int id;
// // 	int value;

// // 	if (sock_read(fd, &id, sizeof(id)) == SOCKERR_CLOSED)
// // 		return -1;
// // 	if (sock_read(fd, &value, sizeof(value)) == SOCKERR_CLOSED)
// // 		return -1;
// // 	LOG_DEBUG("value is %d\n", value);
// // 	err = rk_roi_set_position_x(id, value);
// // 	if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED)
// // 		return -1;

// // 	return 0;
// // }

// // int ser_rk_roi_get_position_y(int fd) {
// // 	int err = 0;
// // 	int id;
// // 	int value;

// // 	if (sock_read(fd, &id, sizeof(id)) == SOCKERR_CLOSED)
// // 		return -1;
// // 	err = rk_roi_get_position_y(id, &value);
// // 	LOG_DEBUG("value is %d\n", value);
// // 	if (sock_write(fd, &value, sizeof(value)) == SOCKERR_CLOSED)
// // 		return -1;
// // 	if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED)
// // 		return -1;

// // 	return 0;
// // }

// // int ser_rk_roi_set_position_y(int fd) {
// // 	int err = 0;
// // 	int id;
// // 	int value;

// // 	if (sock_read(fd, &id, sizeof(id)) == SOCKERR_CLOSED)
// // 		return -1;
// // 	if (sock_read(fd, &value, sizeof(value)) == SOCKERR_CLOSED)
// // 		return -1;
// // 	LOG_DEBUG("value is %d\n", value);
// // 	err = rk_roi_set_position_y(id, value);
// // 	if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED)
// // 		return -1;

// // 	return 0;
// // }

// // int ser_rk_roi_get_height(int fd) {
// // 	int err = 0;
// // 	int id;
// // 	int value;

// // 	if (sock_read(fd, &id, sizeof(id)) == SOCKERR_CLOSED)
// // 		return -1;
// // 	err = rk_roi_get_height(id, &value);
// // 	LOG_DEBUG("value is %d\n", value);
// // 	if (sock_write(fd, &value, sizeof(value)) == SOCKERR_CLOSED)
// // 		return -1;
// // 	if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED)
// // 		return -1;

// // 	return 0;
// // }

// // int ser_rk_roi_set_height(int fd) {
// // 	int err = 0;
// // 	int id;
// // 	int value;

// // 	if (sock_read(fd, &id, sizeof(id)) == SOCKERR_CLOSED)
// // 		return -1;
// // 	if (sock_read(fd, &value, sizeof(value)) == SOCKERR_CLOSED)
// // 		return -1;
// // 	LOG_DEBUG("value is %d\n", value);
// // 	err = rk_roi_set_height(id, value);
// // 	if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED)
// // 		return -1;

// // 	return 0;
// // }

// // int ser_rk_roi_get_width(int fd) {
// // 	int err = 0;
// // 	int id;
// // 	int value;

// // 	if (sock_read(fd, &id, sizeof(id)) == SOCKERR_CLOSED)
// // 		return -1;
// // 	err = rk_roi_get_width(id, &value);
// // 	LOG_DEBUG("value is %d\n", value);
// // 	if (sock_write(fd, &value, sizeof(value)) == SOCKERR_CLOSED)
// // 		return -1;
// // 	if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED)
// // 		return -1;

// // 	return 0;
// // }

// // int ser_rk_roi_set_width(int fd) {
// // 	int err = 0;
// // 	int id;
// // 	int value;

// // 	if (sock_read(fd, &id, sizeof(id)) == SOCKERR_CLOSED)
// // 		return -1;
// // 	if (sock_read(fd, &value, sizeof(value)) == SOCKERR_CLOSED)
// // 		return -1;
// // 	LOG_DEBUG("value is %d\n", value);
// // 	err = rk_roi_set_width(id, value);
// // 	if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED)
// // 		return -1;

// // 	return 0;
// // }

// // int ser_rk_roi_get_quality_level(int fd) {
// // 	int err = 0;
// // 	int id;
// // 	int value;

// // 	if (sock_read(fd, &id, sizeof(id)) == SOCKERR_CLOSED)
// // 		return -1;
// // 	err = rk_roi_get_quality_level(id, &value);
// // 	LOG_DEBUG("value is %d\n", value);
// // 	if (sock_write(fd, &value, sizeof(value)) == SOCKERR_CLOSED)
// // 		return -1;
// // 	if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED)
// // 		return -1;

// // 	return 0;
// // }

// // int ser_rk_roi_set_quality_level(int fd) {
// // 	int err = 0;
// // 	int id;
// // 	int value;

// // 	if (sock_read(fd, &id, sizeof(id)) == SOCKERR_CLOSED)
// // 		return -1;
// // 	if (sock_read(fd, &value, sizeof(value)) == SOCKERR_CLOSED)
// // 		return -1;
// // 	LOG_DEBUG("value is %d\n", value);
// // 	err = rk_roi_set_quality_level(id, value);
// // 	if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED)
// // 		return -1;

// // 	return 0;
// // }

// // int ser_rk_roi_set_all(int fd) {
// // 	int err = 0;

// // 	LOG_DEBUG("begin\n");
// // 	err = rk_roi_set_all();
// // 	LOG_DEBUG("end\n");
// // 	if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED)
// // 		return -1;

// // 	return 0;
// // }

// // region_clip.x

// // int ser_rk_region_clip_get_enabled(int fd) {
// // 	int err = 0;
// // 	int id;
// // 	int value;

// // 	if (sock_read(fd, &id, sizeof(id)) == SOCKERR_CLOSED)
// // 		return -1;
// // 	err = rk_region_clip_get_enabled(id, &value);
// // 	LOG_DEBUG("value is %d\n", value);
// // 	if (sock_write(fd, &value, sizeof(value)) == SOCKERR_CLOSED)
// // 		return -1;
// // 	if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED)
// // 		return -1;

// // 	return 0;
// // }

// // int ser_rk_region_clip_set_enabled(int fd) {
// // 	int err = 0;
// // 	int id;
// // 	int value;

// // 	if (sock_read(fd, &id, sizeof(id)) == SOCKERR_CLOSED)
// // 		return -1;
// // 	if (sock_read(fd, &value, sizeof(value)) == SOCKERR_CLOSED)
// // 		return -1;
// // 	LOG_DEBUG("value is %d\n", value);
// // 	err = rk_region_clip_set_enabled(id, value);
// // 	if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED)
// // 		return -1;

// // 	return 0;
// // }

// // int ser_rk_region_clip_get_position_x(int fd) {
// // 	int err = 0;
// // 	int id;
// // 	int value;

// // 	if (sock_read(fd, &id, sizeof(id)) == SOCKERR_CLOSED)
// // 		return -1;
// // 	err = rk_region_clip_get_position_x(id, &value);
// // 	LOG_DEBUG("value is %d\n", value);
// // 	if (sock_write(fd, &value, sizeof(value)) == SOCKERR_CLOSED)
// // 		return -1;
// // 	if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED)
// // 		return -1;

// // 	return 0;
// // }

// // int ser_rk_region_clip_set_position_x(int fd) {
// // 	int err = 0;
// // 	int id;
// // 	int value;

// // 	if (sock_read(fd, &id, sizeof(id)) == SOCKERR_CLOSED)
// // 		return -1;
// // 	if (sock_read(fd, &value, sizeof(value)) == SOCKERR_CLOSED)
// // 		return -1;
// // 	LOG_DEBUG("value is %d\n", value);
// // 	err = rk_region_clip_set_position_x(id, value);
// // 	if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED)
// // 		return -1;

// // 	return 0;
// // }

// // int ser_rk_region_clip_get_position_y(int fd) {
// // 	int err = 0;
// // 	int id;
// // 	int value;

// // 	if (sock_read(fd, &id, sizeof(id)) == SOCKERR_CLOSED)
// // 		return -1;
// // 	err = rk_region_clip_get_position_y(id, &value);
// // 	LOG_DEBUG("value is %d\n", value);
// // 	if (sock_write(fd, &value, sizeof(value)) == SOCKERR_CLOSED)
// // 		return -1;
// // 	if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED)
// // 		return -1;

// // 	return 0;
// // }

// // int ser_rk_region_clip_set_position_y(int fd) {
// // 	int err = 0;
// // 	int id;
// // 	int value;

// // 	if (sock_read(fd, &id, sizeof(id)) == SOCKERR_CLOSED)
// // 		return -1;
// // 	if (sock_read(fd, &value, sizeof(value)) == SOCKERR_CLOSED)
// // 		return -1;
// // 	LOG_DEBUG("value is %d\n", value);
// // 	err = rk_region_clip_set_position_y(id, value);
// // 	if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED)
// // 		return -1;

// // 	return 0;
// // }

// // int ser_rk_region_clip_get_height(int fd) {
// // 	int err = 0;
// // 	int id;
// // 	int value;

// // 	if (sock_read(fd, &id, sizeof(id)) == SOCKERR_CLOSED)
// // 		return -1;
// // 	err = rk_region_clip_get_height(id, &value);
// // 	LOG_DEBUG("value is %d\n", value);
// // 	if (sock_write(fd, &value, sizeof(value)) == SOCKERR_CLOSED)
// // 		return -1;
// // 	if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED)
// // 		return -1;

// // 	return 0;
// // }

// // int ser_rk_region_clip_set_height(int fd) {
// // 	int err = 0;
// // 	int id;
// // 	int value;

// // 	if (sock_read(fd, &id, sizeof(id)) == SOCKERR_CLOSED)
// // 		return -1;
// // 	if (sock_read(fd, &value, sizeof(value)) == SOCKERR_CLOSED)
// // 		return -1;
// // 	LOG_DEBUG("value is %d\n", value);
// // 	err = rk_region_clip_set_height(id, value);
// // 	if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED)
// // 		return -1;

// // 	return 0;
// // }

// // int ser_rk_region_clip_get_width(int fd) {
// // 	int err = 0;
// // 	int id;
// // 	int value;

// // 	if (sock_read(fd, &id, sizeof(id)) == SOCKERR_CLOSED)
// // 		return -1;
// // 	err = rk_region_clip_get_width(id, &value);
// // 	LOG_DEBUG("value is %d\n", value);
// // 	if (sock_write(fd, &value, sizeof(value)) == SOCKERR_CLOSED)
// // 		return -1;
// // 	if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED)
// // 		return -1;

// // 	return 0;
// // }

// // int ser_rk_region_clip_set_width(int fd) {
// // 	int err = 0;
// // 	int id;
// // 	int value;

// // 	if (sock_read(fd, &id, sizeof(id)) == SOCKERR_CLOSED)
// // 		return -1;
// // 	if (sock_read(fd, &value, sizeof(value)) == SOCKERR_CLOSED)
// // 		return -1;
// // 	LOG_DEBUG("value is %d\n", value);
// // 	err = rk_region_clip_set_width(id, value);
// // 	if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED)
// // 		return -1;

// // 	return 0;
// // }

// // int ser_rk_region_clip_set_all(int fd) {
// // 	int err = 0;

// // 	LOG_DEBUG("begin\n");
// // 	err = rk_region_clip_set_all();
// // 	LOG_DEBUG("end\n");
// // 	if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED)
// // 		return -1;

// // 	return 0;
// // }

// network
int ser_rk_network_ipv4_get(int fd) {
	int err = 0;
	int len, ret;
	char *interface;
	char method[64];
	char address[64];
	char netmask[64];
	char gateway[64];

	// read
	if (sock_read(fd, &len, sizeof(int)) == SOCKERR_CLOSED)
		return -1;
	if (len == 0)
		return -1;
	interface = (char *)malloc(len);
	if (sock_read(fd, interface, len) == SOCKERR_CLOSED) {
		free(interface);
		return -1;
	}
	LOG_DEBUG("len is %d, interface is %s\n", len, interface);
	// get
	ret = rk_network_ipv4_get(interface, method, address, netmask, gateway);
	LOG_DEBUG("method is %s, address is %s, netmask is %s, gateway is %s\n", method, address,
	          netmask, gateway);
	free(interface);
	// write
	len = strlen(method);
	LOG_DEBUG("method len is %d\n", len);
	if (sock_write(fd, &len, sizeof(len)) == SOCKERR_CLOSED)
		return -1;
	if (sock_write(fd, method, len) == SOCKERR_CLOSED)
		return -1;
	len = strlen(address);
	LOG_DEBUG("address len is %d\n", len);
	if (sock_write(fd, &len, sizeof(len)) == SOCKERR_CLOSED)
		return -1;
	if (sock_write(fd, address, len) == SOCKERR_CLOSED)
		return -1;
	len = strlen(netmask);
	LOG_DEBUG("netmask len is %d\n", len);
	if (sock_write(fd, &len, sizeof(len)) == SOCKERR_CLOSED)
		return -1;
	if (sock_write(fd, netmask, len) == SOCKERR_CLOSED)
		return -1;
	len = strlen(gateway);
	LOG_DEBUG("gateway len is %d\n", len);
	if (sock_write(fd, &len, sizeof(len)) == SOCKERR_CLOSED)
		return -1;
	if (sock_write(fd, gateway, len) == SOCKERR_CLOSED)
		return -1;
	if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED)
		return -1;

	return ret;
}

int ser_rk_network_ipv4_set(int fd) {
	int err = 0;
	int len, ret;
	char *interface;
	char *method = NULL;
	char *address = NULL;
	char *netmask = NULL;
	char *gateway = NULL;

	// read
	if (sock_read(fd, &len, sizeof(int)) == SOCKERR_CLOSED)
		return -1;
	interface = (char *)malloc(len);
	if (sock_read(fd, interface, len) == SOCKERR_CLOSED) {
		free(interface);
		return -1;
	}
	if (sock_read(fd, &len, sizeof(int)) == SOCKERR_CLOSED)
		return -1;
	if (len) {
		method = (char *)malloc(len);
		if (sock_read(fd, method, len) == SOCKERR_CLOSED) {
			free(method);
			return -1;
		}
	}
	if (sock_read(fd, &len, sizeof(int)) == SOCKERR_CLOSED)
		return -1;
	if (len) {
		address = (char *)malloc(len);
		if (sock_read(fd, address, len) == SOCKERR_CLOSED) {
			free(address);
			return -1;
		}
	}
	if (sock_read(fd, &len, sizeof(int)) == SOCKERR_CLOSED)
		return -1;
	if (len) {
		netmask = (char *)malloc(len);
		if (sock_read(fd, netmask, len) == SOCKERR_CLOSED) {
			free(netmask);
			return -1;
		}
	}
	if (sock_read(fd, &len, sizeof(int)) == SOCKERR_CLOSED)
		return -1;
	if (len) {
		gateway = (char *)malloc(len);
		if (sock_read(fd, gateway, len) == SOCKERR_CLOSED) {
			free(gateway);
			return -1;
		}
	}
	// set
	// LOG_DEBUG("interface is %s, method is %s, address is %s, netmask is %s,
	// gateway is %s\n",
	// 			interface, method, address, netmask, gateway);
	// ret = rk_network_ipv4_set(interface, method, address, netmask, gateway);
	ret = rk_network_ipv4_set_para(interface, method, address, netmask, gateway);

	free(interface);
	if (method)
		free(method);
	if (address)
		free(address);
	if (netmask)
		free(netmask);
	if (gateway)
		free(gateway);
	// write
	if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED)
		return -1;

	return ret;
}

int ser_rk_network_dns_get(int fd) {
	int err = 0;
	int len, ret;
	char dns1[64];
	char dns2[64];

	// get
	ret = rk_network_dns_get(dns1, dns2);
	LOG_DEBUG("dns1 is %s, dns2 is %s\n", dns1, dns2);
	// write
	len = strlen(dns1);
	if (sock_write(fd, &len, sizeof(len)) == SOCKERR_CLOSED)
		return -1;
	if (sock_write(fd, dns1, len) == SOCKERR_CLOSED)
		return -1;
	len = strlen(dns2);
	if (sock_write(fd, &len, sizeof(len)) == SOCKERR_CLOSED)
		return -1;
	if (sock_write(fd, dns2, len) == SOCKERR_CLOSED)
		return -1;
	if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED)
		return -1;

	return ret;
}

int ser_rk_network_dns_set(int fd) {
	int err = 0;
	int len, ret;
	char *dns1;
	char *dns2;

	// read
	if (sock_read(fd, &len, sizeof(int)) == SOCKERR_CLOSED)
		return -1;
	dns1 = (char *)malloc(len);
	if (sock_read(fd, dns1, len) == SOCKERR_CLOSED) {
		free(dns1);
		return -1;
	}
	if (sock_read(fd, &len, sizeof(int)) == SOCKERR_CLOSED)
		return -1;
	dns2 = (char *)malloc(len);
	if (sock_read(fd, dns2, len) == SOCKERR_CLOSED) {
		free(dns2);
		return -1;
	}
	// set
	LOG_DEBUG("dns1 is %s, dns2 is %s\n", dns1, dns2);
	ret = rk_network_dns_set(dns1, dns2);
	// write
	if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED)
		return -1;

	return ret;
}

int ser_rk_network_get_mac(int fd) {
	int err = 0;
	int len, ret;
	char *interface;
	char mac[64];

	// read
	if (sock_read(fd, &len, sizeof(int)) == SOCKERR_CLOSED)
		return -1;
	if (len == 0)
		return -1;

	interface = (char *)malloc(len);
	if (sock_read(fd, interface, len) == SOCKERR_CLOSED) {
		free(interface);
		return -1;
	}
	// get
	ret = rk_network_get_mac(interface, mac);
	LOG_DEBUG("interface is %s, mac is %s\n", interface, mac);
	free(interface);
	// write
	len = strlen(mac);
	if (sock_write(fd, &len, sizeof(len)) == SOCKERR_CLOSED)
		return -1;
	if (sock_write(fd, mac, len) == SOCKERR_CLOSED)
		return -1;
	if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED)
		return -1;

	return ret;
}

int ser_rk_network_nicspeed_get(int fd) {
	int err = 0;
	int len, ret, speed, duplex, autoneg;
	char *interface;

	// read
	if (sock_read(fd, &len, sizeof(int)) == SOCKERR_CLOSED)
		return -1;
	if (len == 0)
		return -1;

	interface = (char *)malloc(len);
	if (sock_read(fd, interface, len) == SOCKERR_CLOSED) {
		free(interface);
		return -1;
	}
	// get
	ret = rk_network_nicspeed_get(interface, &speed, &duplex, &autoneg);
	LOG_DEBUG("interface is %s, speed is %d, duplex is %d, autoneg is %d\n", interface, speed,
	          duplex, autoneg);
	free(interface);
	// write
	if (sock_write(fd, &speed, sizeof(speed)) == SOCKERR_CLOSED)
		return -1;
	if (sock_write(fd, &duplex, sizeof(duplex)) == SOCKERR_CLOSED)
		return -1;
	if (sock_write(fd, &autoneg, sizeof(autoneg)) == SOCKERR_CLOSED)
		return -1;
	if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED)
		return -1;

	return ret;
}

int ser_rk_network_nicspeed_set(int fd) {
	int err = 0;
	int ret, len, speed, duplex, autoneg;
	char *interface;

	// read
	if (sock_read(fd, &len, sizeof(int)) == SOCKERR_CLOSED)
		return -1;
	interface = (char *)malloc(len);
	if (sock_read(fd, interface, len) == SOCKERR_CLOSED) {
		free(interface);
		return -1;
	}
	if (sock_read(fd, &speed, sizeof(int)) == SOCKERR_CLOSED)
		return -1;
	if (sock_read(fd, &duplex, sizeof(int)) == SOCKERR_CLOSED)
		return -1;
	if (sock_read(fd, &autoneg, sizeof(int)) == SOCKERR_CLOSED)
		return -1;

	// get
	LOG_DEBUG("len is %d, interface is %s, speed is %d, duplex is %d, autoneg is %d\n", len,
	          interface, speed, duplex, autoneg);
	ret = rk_network_nicspeed_set(interface, speed, duplex, autoneg);
	free(interface);
	// write
	if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED)
		return -1;

	return ret;
}

int ser_rk_network_nicspeed_support_get(int fd) {
	int err = 0;
	int len, ret;
	char *interface;
	char nic_supported[256];

	// read
	if (sock_read(fd, &len, sizeof(int)) == SOCKERR_CLOSED)
		return -1;
	if (len == 0)
		return -1;

	interface = (char *)malloc(len);
	if (sock_read(fd, interface, len) == SOCKERR_CLOSED) {
		free(interface);
		return -1;
	}
	// get
	ret = rk_network_nicspeed_support_get(interface, nic_supported);
	LOG_DEBUG("interface is %s, nic_supported is %s\n", interface, nic_supported);
	free(interface);
	// write
	len = strlen(nic_supported);
	if (sock_write(fd, &len, sizeof(len)) == SOCKERR_CLOSED)
		return -1;
	if (sock_write(fd, nic_supported, len) == SOCKERR_CLOSED)
		return -1;
	if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED)
		return -1;

	return ret;
}

//********************************************************************
int ser_rk_network_get_method(int fd) {
	int err = 0;
	int len;
	const char *value;

	err = rk_network_get_method(&value);
	len = strlen(value);
	LOG_DEBUG("len is %d, value is %s, addr is %p\n", len, value, value);
	if (sock_write(fd, &len, sizeof(len)) == SOCKERR_CLOSED)
		return -1;
	if (sock_write(fd, value, len) == SOCKERR_CLOSED)
		return -1;
	if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED)
		return -1;

	return 0;
}


int ser_rk_network_set_method(int fd) {
	int ret = 0;
	int len;
	char *value = NULL;

	if (sock_read(fd, &len, sizeof(len)) == SOCKERR_CLOSED)
		return -1;
	if (len) {
		value = (char *)malloc(len);
		if (sock_read(fd, value, len) == SOCKERR_CLOSED) {
			free(value);
			return -1;
		}
		LOG_DEBUG("value is %s\n", value);
		ret = rk_network_set_method(value);
		free(value);
		if (sock_write(fd, &ret, sizeof(int)) == SOCKERR_CLOSED)
			return -1;
	}

	return 0;
}

int ser_rk_network_set_ipv4_address(int fd) {
	int ret = 0;
	int len;
	char *value = NULL;

	if (sock_read(fd, &len, sizeof(len)) == SOCKERR_CLOSED)
		return -1;
	if (len) {
		value = (char *)malloc(len);
		if (sock_read(fd, value, len) == SOCKERR_CLOSED) {
			free(value);
			return -1;
		}
		LOG_DEBUG("value is %s\n", value);
		ret = rk_network_set_ipv4_address(value);
		free(value);
		if (sock_write(fd, &ret, sizeof(int)) == SOCKERR_CLOSED)
			return -1;
	}

	return 0;
}

int ser_rk_network_get_dns_method(int fd) {
	int err = 0;
	int len;
	const char *value;

	err = rk_network_get_dns_method(&value);
	len = strlen(value);
	LOG_DEBUG("len is %d, value is %s, addr is %p\n", len, value, value);
	if (sock_write(fd, &len, sizeof(len)) == SOCKERR_CLOSED)
		return -1;
	if (sock_write(fd, value, len) == SOCKERR_CLOSED)
		return -1;
	if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED)
		return -1;

	return 0;
}


int ser_rk_network_set_dns_method(int fd) {
	int ret = 0;
	int len;
	char *value = NULL;

	if (sock_read(fd, &len, sizeof(len)) == SOCKERR_CLOSED)
		return -1;
	if (len) {
		value = (char *)malloc(len);
		if (sock_read(fd, value, len) == SOCKERR_CLOSED) {
			free(value);
			return -1;
		}
		LOG_DEBUG("value is %s\n", value);
		ret = rk_network_set_dns_method(value);
		free(value);
		if (sock_write(fd, &ret, sizeof(int)) == SOCKERR_CLOSED)
			return -1;
	}

	return 0;
}


int ser_rk_network_get_http_port(int fd) {
	int err = 0;
	int value;

	err = rk_network_get_http_port(&value);
	LOG_DEBUG("value is %d\n", value);
	if (sock_write(fd, &value, sizeof(value)) == SOCKERR_CLOSED)
		return -1;
	if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED)
		return -1;

	return 0;
}

int ser_rk_network_set_http_port(int fd) {
	int err = 0;
	int value;

	if (sock_read(fd, &value, sizeof(value)) == SOCKERR_CLOSED)
		return -1;
	LOG_DEBUG("value is %d\n", value);
	err = rk_network_set_http_port(value);
	if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED)
		return -1;

	return 0;
}

int ser_rk_network_get_https_port(int fd) {
	int err = 0;
	int value;

	err = rk_network_get_https_port(&value);
	LOG_DEBUG("value is %d\n", value);
	if (sock_write(fd, &value, sizeof(value)) == SOCKERR_CLOSED)
		return -1;
	if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED)
		return -1;

	return 0;
}

int ser_rk_network_set_https_port(int fd) {
	int err = 0;
	int value;

	if (sock_read(fd, &value, sizeof(value)) == SOCKERR_CLOSED)
		return -1;
	LOG_DEBUG("value is %d\n", value);
	err = rk_network_set_https_port(value);
	if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED)
		return -1;

	return 0;
}

int ser_rk_network_get_rtsp_port(int fd) {
	int err = 0;
	int value;

	err = rk_network_get_rtsp_port(&value);
	LOG_DEBUG("value is %d\n", value);
	if (sock_write(fd, &value, sizeof(value)) == SOCKERR_CLOSED)
		return -1;
	if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED)
		return -1;

	return 0;
}

int ser_rk_network_set_rtsp_port(int fd) {
	int err = 0;
	int value;

	if (sock_read(fd, &value, sizeof(value)) == SOCKERR_CLOSED)
		return -1;
	LOG_DEBUG("value is %d\n", value);
	err = rk_network_set_rtsp_port(value);
	if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED)
		return -1;

	return 0;
}

int ser_rk_network_get_onvif_port(int fd) {
	int err = 0;
	int value;

	err = rk_network_get_onvif_port(&value);
	LOG_DEBUG("value is %d\n", value);
	if (sock_write(fd, &value, sizeof(value)) == SOCKERR_CLOSED)
		return -1;
	if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED)
		return -1;

	return 0;
}

int ser_rk_network_set_onvif_port(int fd) {
	int err = 0;
	int value;

	if (sock_read(fd, &value, sizeof(value)) == SOCKERR_CLOSED)
		return -1;
	LOG_DEBUG("value is %d\n", value);
	err = rk_network_set_onvif_port(value);
	if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED)
		return -1;

	return 0;
}

int ser_rk_network_smtp_get(int fd) {
	int ret = 0;
	int len = 0;
	char *json = (char *)malloc(2048); 
	if (!json) {
		LOG_ERROR("Failed to allocate memory for json\n");
		return -1;
	}

	memset(json, 0, 2048); 

	// get
	ret = smtp_get_config_json(json, 2048);
	LOG_DEBUG("json is %s\n", json);
	LOG_DEBUG("strlen(json) is %ld\n", strlen(json));

	// write
	len = strlen(json);
	LOG_DEBUG("strlen(json) is %d\n", len);

	if (sock_write(fd, &len, sizeof(len)) == SOCKERR_CLOSED) {
		free(json); // Free allocated memory before returning
		return -1;
	}
	if (sock_write(fd, json, len) == SOCKERR_CLOSED) {
		free(json); // Free allocated memory before returning
		return -1;
	}
	if (sock_write(fd, &ret, sizeof(int)) == SOCKERR_CLOSED) {
		free(json); // Free allocated memory before returning
		return -1;
	}

	free(json); // Free allocated memory after use
	return 0;
}

int ser_rk_network_smtp_set(int fd) {
    int ret = 0;
    int len;
    char *value = NULL;

    // Read JSON string length
    if (sock_read(fd, &len, sizeof(len)) == SOCKERR_CLOSED)
        return -1;
    if (len > 0) {
        value = (char *)malloc(len + 1);
        if (!value)
            return -1;
        if (sock_read(fd, value, len) == SOCKERR_CLOSED) {
            free(value);
            return -1;
        }
        value[len] = '\0'; // Ensure null-terminated
        ret = rk_network_smtp_set(value);
        free(value);
        if (sock_write(fd, &ret, sizeof(int)) == SOCKERR_CLOSED)
            return -1;
    }
    return 0;
}



// //********************************************************************

// // int ser_rk_wifi_power_get(int fd) {
// // 	int err = 0;
// // 	int on;

// // 	// get
// // 	err = rk_wifi_power_get(&on);
// // 	LOG_DEBUG("on is %d\n", on);
// // 	// write
// // 	if (sock_write(fd, &on, sizeof(int)) == SOCKERR_CLOSED)
// // 		return -1;
// // 	if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED)
// // 		return -1;

// // 	return 0;
// // }

// // int ser_rk_wifi_power_set(int fd) {
// // 	int err = 0;
// // 	int on;

// // 	// read
// // 	if (sock_read(fd, &on, sizeof(int)) == SOCKERR_CLOSED)
// // 		return -1;
// // 	// get
// // 	LOG_DEBUG("on is %d\n", on);
// // 	on = rk_wifi_power_set(on);
// // 	// write
// // 	if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED)
// // 		return -1;

// // 	return 0;
// // }

// // int ser_rk_wifi_scan_wifi(int fd) {
// // 	int err = 0;

// // 	LOG_DEBUG("begin\n");
// // 	err = rk_wifi_scan_wifi();
// // 	LOG_DEBUG("end\n");
// // 	if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED)
// // 		return -1;

// // 	return 0;
// // }

// // int ser_rk_wifi_get_list(int fd) {
// // 	int err, len = 0;
// // 	char *wifi_list;

// // 	// get
// // 	err = rk_wifi_get_list(&wifi_list);
// // 	LOG_DEBUG("strlen(wifi_list) is %ld\n", strlen(wifi_list));
// // 	// write
// // 	len = strlen(wifi_list);
// // 	if (sock_write(fd, &len, sizeof(len)) == SOCKERR_CLOSED)
// // 		return -1;
// // 	if (sock_write(fd, wifi_list, len) == SOCKERR_CLOSED)
// // 		return -1;
// // 	if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED)
// // 		return -1;
// // 	free(wifi_list);

// // 	return 0;
// // }

// // int ser_rk_wifi_connect_with_ssid(int fd) {
// // 	int len, ret;
// // 	char *ssid, *psk;

// // 	// read
// // 	if (sock_read(fd, &len, sizeof(int)) == SOCKERR_CLOSED)
// // 		return -1;
// // 	LOG_DEBUG("1 len is %d\n", len);
// // 	ssid = (char *)malloc(len);
// // 	if (sock_read(fd, ssid, len) == SOCKERR_CLOSED) {
// // 		free(ssid);
// // 		return -1;
// // 	}
// // 	if (sock_read(fd, &len, sizeof(int)) == SOCKERR_CLOSED)
// // 		return -1;
// // 	LOG_DEBUG("2 len is %d\n", len);
// // 	psk = (char *)malloc(len);
// // 	if (sock_read(fd, psk, len) == SOCKERR_CLOSED) {
// // 		free(psk);
// // 		return -1;
// // 	}

// // 	// get
// // 	LOG_DEBUG("ssid is %s, psk is %s\n", ssid, psk);
// // 	ret = rk_wifi_connect_with_ssid(ssid, psk);
// // 	free(ssid);
// // 	free(psk);
// // 	// write
// // 	if (sock_write(fd, &ret, sizeof(int)) == SOCKERR_CLOSED)
// // 		return -1;

// // 	return 0;
// // }

// // int ser_rk_wifi_forget_with_ssid(int fd) {
// // 	int len, ret;
// // 	char *ssid;

// // 	// read
// // 	if (sock_read(fd, &len, sizeof(int)) == SOCKERR_CLOSED)
// // 		return -1;
// // 	ssid = (char *)malloc(len);
// // 	if (sock_read(fd, ssid, len) == SOCKERR_CLOSED) {
// // 		free(ssid);
// // 		return -1;
// // 	}

// // 	// get
// // 	LOG_DEBUG("ssid is %s\n", ssid);
// // 	ret = rk_wifi_forget_with_ssid(ssid);
// // 	free(ssid);
// // 	// write
// // 	if (sock_write(fd, &ret, sizeof(int)) == SOCKERR_CLOSED)
// // 		return -1;

// // 	return 0;
// // }
// //time
// int ser_rk_network_get_timezone(int fd) {
// 	int err = 0;
// 	int len;
// 	const char *value;

// 	err = rk_network_get_timezone(&value);
// 	len = strlen(value);
// 	LOG_DEBUG("len is %d, value is %s, addr is %p\n", len, value, value);
// 	if (sock_write(fd, &len, sizeof(len)) == SOCKERR_CLOSED)
// 		return -1;
// 	if (sock_write(fd, value, len) == SOCKERR_CLOSED)
// 		return -1;
// 	if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED)
// 		return -1;

// 	return 0;
// }

// int ser_rk_network_set_timezone(int fd) {
// 	int ret = 0;
// 	int len;
// 	char *value = NULL;

// 	if (sock_read(fd, &len, sizeof(len)) == SOCKERR_CLOSED)
// 		return -1;
// 	if (len) {
// 		value = (char *)malloc(len);
// 		if (sock_read(fd, value, len) == SOCKERR_CLOSED) {
// 			free(value);
// 			return -1;
// 		}
// 		LOG_DEBUG("value is %s\n", value);
// 		ret = rk_network_set_timezone(value);
// 		free(value);
// 		if (sock_write(fd, &ret, sizeof(int)) == SOCKERR_CLOSED)
// 			return -1;
// 	}

// 	return 0;
// }
// int ser_rk_network_get_ntp_enable(int fd) {
// 	int err = 0;
// 	int value;

// 	err = rk_network_get_ntp_enable(&value);
// 	LOG_DEBUG("value is %d\n", value);
// 	if (sock_write(fd, &value, sizeof(value)) == SOCKERR_CLOSED)
// 		return -1;
// 	if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED)
// 		return -1;

// 	return 0;
// }

// int ser_rk_network_set_ntp_enable(int fd) {
// 	int err = 0;
// 	int value;

// 	if (sock_read(fd, &value, sizeof(value)) == SOCKERR_CLOSED)
// 		return -1;
// 	LOG_DEBUG("value is %d\n", value);
// 	err = rk_network_set_ntp_enable(value);
// 	if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED)
// 		return -1;

// 	return 0;
// }
// int ser_rk_network_get_ntp_refresh_time_sec(int fd) {
// 	int err = 0;
// 	int value;

// 	err = rk_network_get_ntp_refresh_time_sec(&value);
// 	LOG_DEBUG("value is %d\n", value);
// 	if (sock_write(fd, &value, sizeof(value)) == SOCKERR_CLOSED)
// 		return -1;
// 	if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED)
// 		return -1;

// 	return 0;
// }

// int ser_rk_network_set_ntp_refresh_time_sec(int fd) {
// 	int err = 0;
// 	int value;

// 	if (sock_read(fd, &value, sizeof(value)) == SOCKERR_CLOSED)
// 		return -1;
// 	LOG_DEBUG("value is %d\n", value);
// 	err = rk_network_set_ntp_refresh_time_sec(value);
// 	if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED)
// 		return -1;

// 	return 0;
// }
// int ser_rk_network_get_ntp_server(int fd) {
// 	int err = 0;
// 	int len;
// 	const char *value;

// 	err = rk_network_get_ntp_server(&value);
// 	len = strlen(value);
// 	LOG_DEBUG("len is %d, value is %s, addr is %p\n", len, value, value);
// 	if (sock_write(fd, &len, sizeof(len)) == SOCKERR_CLOSED)
// 		return -1;
// 	if (sock_write(fd, value, len) == SOCKERR_CLOSED)
// 		return -1;
// 	if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED)
// 		return -1;

// 	return 0;
// }

// int ser_rk_network_set_ntp_server(int fd) {
// 	int ret = 0;
// 	int len;
// 	char *value = NULL;

// 	if (sock_read(fd, &len, sizeof(len)) == SOCKERR_CLOSED)
// 		return -1;
// 	if (len) {
// 		value = (char *)malloc(len);
// 		if (sock_read(fd, value, len) == SOCKERR_CLOSED) {
// 			free(value);
// 			return -1;
// 		}
// 		LOG_DEBUG("value is %s\n", value);
// 		ret = rk_network_set_ntp_server(value);
// 		free(value);
// 		if (sock_write(fd, &ret, sizeof(int)) == SOCKERR_CLOSED)
// 			return -1;
// 	}

// 	return 0;
// }
// int ser_rk_network_get_ntp_port(int fd) {
// 	int err = 0;
// 	int value;

// 	err = rk_network_get_ntp_port(&value);
// 	LOG_DEBUG("value is %d\n", value);
// 	if (sock_write(fd, &value, sizeof(value)) == SOCKERR_CLOSED)
// 		return -1;
// 	if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED)
// 		return -1;

// 	return 0;
// }

// int ser_rk_network_set_ntp_port(int fd) {
// 	int err = 0;
// 	int value;

// 	if (sock_read(fd, &value, sizeof(value)) == SOCKERR_CLOSED)
// 		return -1;
// 	LOG_DEBUG("value is %d\n", value);
// 	err = rk_network_set_ntp_port(value);
// 	if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED)
// 		return -1;

// 	return 0;
// }

// int ser_rk_network_get_onvif_timesync_flag(int fd) {
// 	int err = 0;
// 	int value;

// 	err = rk_network_get_onvif_timesync_flag(&value);
// 	LOG_DEBUG("value is %d\n", value);
// 	if (sock_write(fd, &value, sizeof(value)) == SOCKERR_CLOSED)
// 		return -1;
// 	if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED)
// 		return -1;

// 	return 0;
// }

// int ser_rk_network_set_onvif_timesync_flag(int fd) {
// 	int err = 0;
// 	int value;

// 	if (sock_read(fd, &value, sizeof(value)) == SOCKERR_CLOSED)
// 		return -1;
// 	LOG_DEBUG("value is %d\n", value);
// 	err = rk_network_set_onvif_timesync_flag(value);
// 	if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED)
// 		return -1;

// 	return 0;
// }
// // storage
// int ser_rk_storage_record_start(int fd) {
// 	int err = 0;

// 	LOG_DEBUG("begin\n");
// 	err = rk_storage_record_start();
// 	if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED)
// 		return -1;
// 	LOG_DEBUG("end\n");

// 	return 0;
// }

// int ser_rk_storage_record_stop(int fd) {
// 	int err = 0;

// 	LOG_DEBUG("begin\n");
// 	err = rk_storage_record_stop();
// 	if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED)
// 		return -1;
// 	LOG_DEBUG("end\n");

// 	return 0;
// }

// int ser_rk_storage_record_statue_get(int fd) {
// 	int err = 0;
// 	int value =0;

// 	err = rk_storage_record_statue_get(&value);
// 	LOG_DEBUG("value is %d\n", value);
// 	if (sock_write(fd, &value, sizeof(value)) == SOCKERR_CLOSED)
// 		return -1;
// 	if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED)
// 		return -1;

// 	return 0;
// }

// int ser_rk_storage_format_sd_card(int fd){
// 	int err = 0;
// 	int value = 0;
// 	LOG_DEBUG("begin\n");
// 	if (sock_read(fd, &value, sizeof(value)) == SOCKERR_CLOSED)
// 		return -1;
// 	LOG_DEBUG("value is %d\n", value);
// 	err = rk_storage_format_sd_card(value);
// 	if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED)
// 		return -1;
// 	LOG_DEBUG("end\n");
// 	return 0;

// }

// int ser_rk_storage_get_schedule_plan(int fd) {
// 	int err = 0;
// 	int len, sql_query_len;
// 	char* sql_query = NULL;
// 	char value[5000] = "";

// 	LOG_INFO("\n============comes in %s============\n",__func__);
	
// 	memset(value, '\0', 1); // set terminator

// 	if (sock_read(fd, &sql_query_len, sizeof(int)) == SOCKERR_CLOSED) { //receive lenght of sql query
// 		LOG_ERROR("\nCannot read sql query len\n");
// 		return -1;
// 	}

// 	sql_query = (char *)malloc(sql_query_len);
// 	if (sql_query == NULL) {
// 		LOG_ERROR("\nmalloc failed in %s\n", __func__);
// 		return -1;
// 	}

// 	if (sock_read(fd, sql_query, sql_query_len) == SOCKERR_CLOSED) { // receive the sql query after receivng the length
// 		free(sql_query);
// 		LOG_ERROR("\nCannot read sql_query\n");
// 		return -1;
// 	}
// 	sql_query[strlen(sql_query)] = '\0'; // add NULL terminator to the end of sequence
// 	// memset(sql_query + sql_query_len, '\0', 1);  // add NULL terminator to the end of sequence
// 	err = rk_storage_get_schedule_plan(value, sql_query); //run the query and store the results in value
// 	len = strlen(value);
// 	LOG_DEBUG("len is %d, value is %s, addr is %p\n", len, value, value);
// 	if (sock_write(fd, &len, sizeof(len)) == SOCKERR_CLOSED) { // send length of the value
// 		free(sql_query);
// 		return -1;
// 	}
// 	if (sock_write(fd, value, len) == SOCKERR_CLOSED) { //send the value itself
// 		free(sql_query);
// 		return -1;
// 	}
// 	if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED)
// 	{
// 		free(sql_query);
// 		return -1;
// 	}
// 	free(sql_query);
// 	return 0;
// }

// int ser_rk_storage_set_schedule_plan(int fd) {
// 	int err = 0;
// 	int len, sql_query_len;
// 	char* sql_query = NULL;

// 	LOG_INFO("\n============comes in %s============\n",__func__);

// 	if (sock_read(fd, &sql_query_len, sizeof(int)) == SOCKERR_CLOSED) { //receive lenght of sql query
// 		LOG_ERROR("\nCannot read sql query len\n");
// 		return -1;
// 	}

// 	sql_query = (char *)malloc(sql_query_len);
// 	if (sql_query == NULL) {
// 		LOG_ERROR("\nmalloc failed in %s\n", __func__);
// 		return -1;
// 	}

// 	if (sock_read(fd, sql_query, sql_query_len) == SOCKERR_CLOSED) { // receive the sql query after receivng the length
// 		free(sql_query);
// 		LOG_ERROR("\nCannot read sql_query\n");
// 		return -1;
// 	}
// 	sql_query[strlen(sql_query)] = '\0'; // add NULL terminator to the end of sequence

// 	err = rk_storage_set_schedule_plan(sql_query); //run the query and store the results in value
// 	if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED) {
// 		free(sql_query);
// 		return -1;
// 	}
// 	free(sql_query);
// 	return 0;
// }

// int ser_rk_storage_get_recordings(int fd) {
//     int err = 0;
//     int sql_query_len;
//     char *sql_query = NULL;
//     char *value = NULL;

//     LOG_INFO("\n============comes in %s============\n", __func__);

//     // Read the length of the SQL query
//     if (sock_read(fd, &sql_query_len, sizeof(int)) == SOCKERR_CLOSED) {
//         LOG_ERROR("\nCannot read SQL query length\n");
//         return -1;
//     }

//     // Allocate memory for the SQL query
//     sql_query = (char *)malloc(sql_query_len + 1); // +1 for null terminator
//     if (sql_query == NULL) {
//         LOG_ERROR("\nmalloc failed in %s\n", __func__);
//         return -1;
//     }

//     // Read the SQL query
//     if (sock_read(fd, sql_query, sql_query_len) == SOCKERR_CLOSED) {
//         free(sql_query);
//         LOG_ERROR("\nCannot read SQL query\n");
//         return -1;
//     }
//     sql_query[sql_query_len] = '\0'; // Null-terminate the SQL query

//     // Call the function to get the recording data
//     err = rk_storage_get_recordings(&value, sql_query);
//     if (err != 0 || value == NULL) {
//         free(sql_query);
//         LOG_ERROR("\nError in rk_storage_get_recordings\n");
//         return -1;
//     }

//     // Get the length of the value
//     int len = strlen(value);
//     // LOG_INFO("len is %d, value is %s, addr is %p\n", len, value, value);

//     // Send the length of the value
//     if (sock_write(fd, &len, sizeof(len)) == SOCKERR_CLOSED) {
//         free(sql_query);
//         free(value);
//         return -1;
//     }

//     // Send the value in chunks
//     int chunk_size = 4096; // Define a reasonable chunk size
//     int bytes_sent = 0;
//     while (bytes_sent < len) {
//         int to_send = (len - bytes_sent > chunk_size) ? chunk_size : (len - bytes_sent);
//         if (sock_write(fd, value + bytes_sent, to_send) == SOCKERR_CLOSED) {
//             free(sql_query);
//             free(value);
//             return -1;
//         }
//         bytes_sent += to_send;
//     }

//     // Send the error code
//     if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED) {
//         free(sql_query);
//         free(value);
//         return -1;
//     }

//     // Free allocated memory
//     free(sql_query);
//     free(value);

//     return 0;
// }

// int ser_rk_storage_get_sdcard_size(int fd) {
//     int err = 0;
//     float total_size = 0.0f;
// 	float free_size = 0.0f;
// 	int mounted = 0;
// 	int free_size_mb = 0;

//     LOG_INFO("\n============comes in %s============\n", __func__);

//     // Call the rk_storage_get_sdcard_size function to get the SD card size details
//     err = rk_storage_get_sdcard_size(&total_size, &free_size, &mounted, &free_size_mb);
//     if (err != 0) {
//         LOG_ERROR("Failed to get SD card size\n");
//         if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED)
//             return -1;
//         return err;
//     }

//     LOG_DEBUG("total_size: %.1f GB, free_size: %.1f GB\n", total_size, free_size);

//     // Write the total size to the socket
//     if (sock_write(fd, &total_size, sizeof(total_size)) == SOCKERR_CLOSED) {
//         return -1;
//     }

//     // Write the free size to the socket
//     if (sock_write(fd, &free_size, sizeof(free_size)) == SOCKERR_CLOSED) {
//         return -1;
//     }

// 	// Write the mounted status to the socket
// 	if (sock_write(fd, &mounted, sizeof(mounted)) == SOCKERR_CLOSED) {
// 		return -1;
// 	}
// 	// Write the free size in MB to the socket
// 	if (sock_write(fd, &free_size_mb, sizeof(free_size_mb)) == SOCKERR_CLOSED) {
// 		return -1;
// 	}

//     // Write the error code to the socket
//     if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED) {
//         return -1;
//     }

//     return 0;
// }

// int ser_rk_storage_get_recording_channel(int fd) {
//     int err = 0;
//     int recording_channel = -1;

//     LOG_INFO("\n============comes in %s============\n", __func__);

//     // Call the rk_storage_get_recording_channel function to get the recording channel
//     err = rk_storage_get_recording_channel(&recording_channel);
//     if (err != 0) {
//         LOG_ERROR("Not recording\n");
//         if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED)
//             return -1;
//         return err;
//     }

//     LOG_DEBUG("recording_channel: %d\n", recording_channel);

//     // Write the recording channel to the socket
//     if (sock_write(fd, &recording_channel, sizeof(recording_channel)) == SOCKERR_CLOSED) {
//         return -1;
//     }

//     // Write the error code to the socket
//     if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED) {
//         return -1;
//     }

//     return 0;
// }

// int ser_rk_storage_set_recording_channel(int fd) {
// 	int err = 0;
// 	int recording_channel = -1;

// 	// Read the recording channel from the socket
// 	if (sock_read(fd, &recording_channel, sizeof(recording_channel)) == SOCKERR_CLOSED) {
// 		return -1;
// 	}

// 	LOG_DEBUG("recording_channel: %d\n", recording_channel);

// 	// Call the rk_storage_set_recording_channel function to set the recording channel
// 	err = rk_storage_set_recording_channel(recording_channel);
// 	if (err != 0) {
// 		LOG_ERROR("Failed to set recording channel\n");
// 		if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED)
// 			return -1;
// 		return err;
// 	}

// 	// Write the error code to the socket
// 	if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED) {
// 		return -1;
// 	}

// 	return 0;
// }

// int ser_rk_storage_get_recording_type(int fd) {
//     int err = 0;
//     char *recording_type = NULL;

//     LOG_INFO("\n============comes in %s============\n", __func__);

//     // Call the rk_storage_get_recording_type function to get the recording type
//     err = rk_storage_get_recording_type(&recording_type);
//     if (err != 0 || recording_type == NULL) {
//         LOG_ERROR("Failed to get recording type\n");
//         err = -1;
//         if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED)
//             return -1;
//         return err;
//     }

//     LOG_DEBUG("recording_type: %s\n", recording_type);

//     // Write the length of the recording type string to the socket
//     int len = strlen(recording_type);
//     if (sock_write(fd, &len, sizeof(len)) == SOCKERR_CLOSED) {
//         return -1;
//     }

//     // Write the recording type string to the socket
//     if (sock_write(fd, recording_type, len) == SOCKERR_CLOSED) {
//         return -1;
//     }

//     // Write the error code to the socket
//     if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED) {
//         return -1;
//     }

//     return 0;
// }

// int ser_rk_storage_set_recording_type(int fd){

// 	int ret = 0;
// 	int len;
// 	char *value = NULL;

// 	if (sock_read(fd, &len, sizeof(len)) == SOCKERR_CLOSED)
// 		return -1;
// 	if (len) {
// 		value = (char *)malloc(len);
// 		if (sock_read(fd, value, len) == SOCKERR_CLOSED) {
// 			free(value);
// 			return -1;
// 		}
// 		LOG_DEBUG("value is %s\n", value);
// 		ret = rk_storage_set_recording_type(value);
// 		free(value);
// 		if (sock_write(fd, &ret, sizeof(int)) == SOCKERR_CLOSED)
// 			return -1;
// 	}

// 	return 0;
// }

// int ser_rk_storage_get_FIFO_status(int fd) {
//     int err = 0;
//     int fifo_status = 0;

//     LOG_INFO("\n============comes in %s============\n", __func__);

//     // Call the rk_storage_get_FIFO_status function to get the FIFO status
//     err = rk_storage_get_FIFO_status(&fifo_status);
//     if (err != 0) {
//         LOG_ERROR("Failed to get FIFO status\n");
//         if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED)
//             return -1;
//         return err;
//     }

//     LOG_DEBUG("FIFO status: %d\n", fifo_status);

//     // Write the FIFO status to the socket
//     if (sock_write(fd, &fifo_status, sizeof(fifo_status)) == SOCKERR_CLOSED) {
//         return -1;
//     }

//     // Write the error code to the socket
//     if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED) {
//         return -1;
//     }

//     return 0;
// }

// int ser_rk_storage_set_FIFO_status(int fd){
// 	int ret = 0;
// 	int fifo_status = 0;

// 	if (sock_read(fd, &fifo_status, sizeof(fifo_status)) == SOCKERR_CLOSED)
// 		return -1;
// 	LOG_DEBUG("fifo_status is %d\n", fifo_status);
// 	ret = rk_storage_set_FIFO_status(fifo_status);
// 	if (sock_write(fd, &ret, sizeof(int)) == SOCKERR_CLOSED)
// 		return -1;

// 	return 0;
// }

// int ser_rk_storage_set_free_size_mb(int fd){
// 	int ret = 0;
// 	int free_size_mb = 0;

// 	if (sock_read(fd, &free_size_mb, sizeof(free_size_mb)) == SOCKERR_CLOSED)
// 		return -1;
// 	LOG_DEBUG("free_size_mb is %d\n", free_size_mb);
// 	ret = rk_storage_set_free_size_mb(free_size_mb);
// 	if (sock_write(fd, &ret, sizeof(int)) == SOCKERR_CLOSED)
// 		return -1;

// 	return 0;
// }
// // int ser_rk_take_photo(int fd) {
// // 	int err = 0;

// // 	LOG_DEBUG("begin\n");
// // 	err = rk_take_photo();
// // 	if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED)
// // 		return -1;
// // 	LOG_DEBUG("end\n");

// // 	return 0;
// // }

// // event
// // int ser_rk_event_ri_get_enabled(int fd) {
// // 	int err = 0;
// // 	int value;

// // 	err = rk_event_ri_get_enabled(&value);
// // 	LOG_DEBUG("value is %d\n", value);
// // 	if (sock_write(fd, &value, sizeof(value)) == SOCKERR_CLOSED)
// // 		return -1;
// // 	if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED)
// // 		return -1;

// // 	return 0;
// // }

// // int ser_rk_event_ri_set_enabled(int fd) {
// // 	int err = 0;
// // 	int value;

// // 	if (sock_read(fd, &value, sizeof(value)) == SOCKERR_CLOSED)
// // 		return -1;
// // 	LOG_DEBUG("value is %d\n", value);
// // 	err = rk_event_ri_set_enabled(value);
// // 	if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED)
// // 		return -1;

// // 	return 0;
// // }

// // int ser_rk_event_ri_get_position_x(int fd) {
// // 	int err = 0;
// // 	int value;

// // 	err = rk_event_ri_get_position_x(&value);
// // 	LOG_DEBUG("value is %d\n", value);
// // 	if (sock_write(fd, &value, sizeof(value)) == SOCKERR_CLOSED)
// // 		return -1;
// // 	if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED)
// // 		return -1;

// // 	return 0;
// // }

// // int ser_rk_event_ri_set_position_x(int fd) {
// // 	int err = 0;
// // 	int value;

// // 	if (sock_read(fd, &value, sizeof(value)) == SOCKERR_CLOSED)
// // 		return -1;
// // 	LOG_DEBUG("value is %d\n", value);
// // 	err = rk_event_ri_set_position_x(value);
// // 	if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED)
// // 		return -1;

// // 	return 0;
// // }

// // int ser_rk_event_ri_get_position_y(int fd) {
// // 	int err = 0;
// // 	int value;

// // 	err = rk_event_ri_get_position_y(&value);
// // 	LOG_DEBUG("value is %d\n", value);
// // 	if (sock_write(fd, &value, sizeof(value)) == SOCKERR_CLOSED)
// // 		return -1;
// // 	if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED)
// // 		return -1;

// // 	return 0;
// // }

// // int ser_rk_event_ri_set_position_y(int fd) {
// // 	int err = 0;
// // 	int value;

// // 	if (sock_read(fd, &value, sizeof(value)) == SOCKERR_CLOSED)
// // 		return -1;
// // 	LOG_DEBUG("value is %d\n", value);
// // 	err = rk_event_ri_set_position_y(value);
// // 	if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED)
// // 		return -1;

// // 	return 0;
// // }

// // int ser_rk_event_ri_get_width(int fd) {
// // 	int err = 0;
// // 	int value;

// // 	err = rk_event_ri_get_width(&value);
// // 	LOG_DEBUG("value is %d\n", value);
// // 	if (sock_write(fd, &value, sizeof(value)) == SOCKERR_CLOSED)
// // 		return -1;
// // 	if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED)
// // 		return -1;

// // 	return 0;
// // }

// // int ser_rk_event_ri_set_width(int fd) {
// // 	int err = 0;
// // 	int value;

// // 	if (sock_read(fd, &value, sizeof(value)) == SOCKERR_CLOSED)
// // 		return -1;
// // 	LOG_DEBUG("value is %d\n", value);
// // 	err = rk_event_ri_set_width(value);
// // 	if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED)
// // 		return -1;

// // 	return 0;
// // }

// // int ser_rk_event_ri_get_height(int fd) {
// // 	int err = 0;
// // 	int value;

// // 	err = rk_event_ri_get_height(&value);
// // 	LOG_DEBUG("value is %d\n", value);
// // 	if (sock_write(fd, &value, sizeof(value)) == SOCKERR_CLOSED)
// // 		return -1;
// // 	if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED)
// // 		return -1;

// // 	return 0;
// // }

// // int ser_rk_event_ri_set_height(int fd) {
// // 	int err = 0;
// // 	int value;

// // 	if (sock_read(fd, &value, sizeof(value)) == SOCKERR_CLOSED)
// // 		return -1;
// // 	LOG_DEBUG("value is %d\n", value);
// // 	err = rk_event_ri_set_height(value);
// // 	if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED)
// // 		return -1;

// // 	return 0;
// // }

// // int ser_rk_event_ri_get_proportion(int fd) {
// // 	int err = 0;
// // 	int value;

// // 	err = rk_event_ri_get_proportion(&value);
// // 	LOG_DEBUG("value is %d\n", value);
// // 	if (sock_write(fd, &value, sizeof(value)) == SOCKERR_CLOSED)
// // 		return -1;
// // 	if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED)
// // 		return -1;

// // 	return 0;
// // }

// // int ser_rk_event_ri_set_proportion(int fd) {
// // 	int err = 0;
// // 	int value;

// // 	if (sock_read(fd, &value, sizeof(value)) == SOCKERR_CLOSED)
// // 		return -1;
// // 	LOG_DEBUG("value is %d\n", value);
// // 	err = rk_event_ri_set_proportion(value);
// // 	if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED)
// // 		return -1;

// // 	return 0;
// // }

// // int ser_rk_event_ri_get_sensitivity_level(int fd) {
// // 	int err = 0;
// // 	int value;

// // 	err = rk_event_ri_get_sensitivity_level(&value);
// // 	LOG_DEBUG("value is %d\n", value);
// // 	if (sock_write(fd, &value, sizeof(value)) == SOCKERR_CLOSED)
// // 		return -1;
// // 	if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED)
// // 		return -1;

// // 	return 0;
// // }

// // int ser_rk_event_ri_set_sensitivity_level(int fd) {
// // 	int err = 0;
// // 	int value;

// // 	if (sock_read(fd, &value, sizeof(value)) == SOCKERR_CLOSED)
// // 		return -1;
// // 	LOG_DEBUG("value is %d\n", value);
// // 	err = rk_event_ri_set_sensitivity_level(value);
// // 	if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED)
// // 		return -1;

// // 	return 0;
// // }

// // int ser_rk_event_ri_get_time_threshold(int fd) {
// // 	int err = 0;
// // 	int value;

// // 	err = rk_event_ri_get_time_threshold(&value);
// // 	LOG_DEBUG("value is %d\n", value);
// // 	if (sock_write(fd, &value, sizeof(value)) == SOCKERR_CLOSED)
// // 		return -1;
// // 	if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED)
// // 		return -1;

// // 	return 0;
// // }

// // int ser_rk_event_ri_set_time_threshold(int fd) {
// // 	int err = 0;
// // 	int value;

// // 	if (sock_read(fd, &value, sizeof(value)) == SOCKERR_CLOSED)
// // 		return -1;
// // 	LOG_DEBUG("value is %d\n", value);
// // 	err = rk_event_ri_set_time_threshold(value);
// // 	if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED)
// // 		return -1;

// // 	return 0;
// // }

// // system
// int ser_rk_system_capability_get_video(int fd) {
// 	int err = 0;
// 	int len;
// 	char value[4096];

// 	memset(value, '\0', 1); // set terminator
// 	err = rk_system_capability_get_video(value);
// 	len = strlen(value);
// 	LOG_DEBUG("len is %d, value is %s, addr is %p\n", len, value, value);
// 	if (sock_write(fd, &len, sizeof(len)) == SOCKERR_CLOSED)
// 		return -1;
// 	if (sock_write(fd, value, len) == SOCKERR_CLOSED)
// 		return -1;
// 	if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED)
// 		return -1;

// 	return 0;
// }

// int ser_rk_system_capability_get_image_adjustment(int fd) {
// 	int err = 0;
// 	int len;
// 	char value[4096];

// 	memset(value, '\0', 1); // set terminator
// 	err = rk_system_capability_get_image_adjustment(value);
// 	len = strlen(value);
// 	LOG_DEBUG("len is %d, value is %s, addr is %p\n", len, value, value);
// 	if (sock_write(fd, &len, sizeof(len)) == SOCKERR_CLOSED)
// 		return -1;
// 	if (sock_write(fd, value, len) == SOCKERR_CLOSED)
// 		return -1;
// 	if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED)
// 		return -1;

// 	return 0;
// }

// int ser_rk_system_capability_get_image_blc(int fd) {
// 	int err = 0;
// 	int len;
// 	char value[4096];

// 	memset(value, '\0', 1); // set terminator
// 	err = rk_system_capability_get_image_blc(value);
// 	len = strlen(value);
// 	LOG_DEBUG("len is %d, value is %s, addr is %p\n", len, value, value);
// 	if (sock_write(fd, &len, sizeof(len)) == SOCKERR_CLOSED)
// 		return -1;
// 	if (sock_write(fd, value, len) == SOCKERR_CLOSED)
// 		return -1;
// 	if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED)
// 		return -1;

// 	return 0;
// }

// int ser_rk_system_capability_get_image_enhancement(int fd) {
// 	int err = 0;
// 	int len;
// 	char value[4096];

// 	memset(value, '\0', 1); // set terminator
// 	err = rk_system_capability_get_image_enhancement(value);
// 	len = strlen(value);
// 	LOG_DEBUG("len is %d, value is %s, addr is %p\n", len, value, value);
// 	if (sock_write(fd, &len, sizeof(len)) == SOCKERR_CLOSED)
// 		return -1;
// 	if (sock_write(fd, value, len) == SOCKERR_CLOSED)
// 		return -1;
// 	if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED)
// 		return -1;

// 	return 0;
// }

// int ser_rk_system_capability_get_image_exposure(int fd) {
// 	int err = 0;
// 	int len;
// 	char value[4096];

// 	memset(value, '\0', 1); // set terminator
// 	err = rk_system_capability_get_image_exposure(value);
// 	len = strlen(value);
// 	LOG_DEBUG("len is %d, value is %s, addr is %p\n", len, value, value);
// 	if (sock_write(fd, &len, sizeof(len)) == SOCKERR_CLOSED)
// 		return -1;
// 	if (sock_write(fd, value, len) == SOCKERR_CLOSED)
// 		return -1;
// 	if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED)
// 		return -1;

// 	return 0;
// }

// int ser_rk_system_capability_get_image_night_to_day(int fd) {
// 	int err = 0;
// 	int len;
// 	char value[4096];

// 	memset(value, '\0', 1); // set terminator
// 	err = rk_system_capability_get_image_night_to_day(value);
// 	len = strlen(value);
// 	LOG_DEBUG("len is %d, value is %s, addr is %p\n", len, value, value);
// 	if (sock_write(fd, &len, sizeof(len)) == SOCKERR_CLOSED)
// 		return -1;
// 	if (sock_write(fd, value, len) == SOCKERR_CLOSED)
// 		return -1;
// 	if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED)
// 		return -1;

// 	return 0;
// }

// int ser_rk_system_capability_get_image_video_adjustment(int fd) {
// 	int err = 0;
// 	int len;
// 	char value[4096];

// 	memset(value, '\0', 1); // set terminator
// 	err = rk_system_capability_get_image_video_adjustment(value);
// 	len = strlen(value);
// 	LOG_DEBUG("len is %d, value is %s, addr is %p\n", len, value, value);
// 	if (sock_write(fd, &len, sizeof(len)) == SOCKERR_CLOSED)
// 		return -1;
// 	if (sock_write(fd, value, len) == SOCKERR_CLOSED)
// 		return -1;
// 	if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED)
// 		return -1;

// 	return 0;
// }

// int ser_rk_system_capability_get_image_white_blance(int fd) {
// 	int err = 0;
// 	int len;
// 	char value[4096];

// 	memset(value, '\0', 1); // set terminator
// 	err = rk_system_capability_get_image_white_blance(value);
// 	len = strlen(value);
// 	LOG_DEBUG("len is %d, value is %s, addr is %p\n", len, value, value);
// 	if (sock_write(fd, &len, sizeof(len)) == SOCKERR_CLOSED)
// 		return -1;
// 	if (sock_write(fd, value, len) == SOCKERR_CLOSED)
// 		return -1;
// 	if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED)
// 		return -1;

// 	return 0;
// }

// int ser_rk_system_get_deivce_name(int fd) {
// 	int err = 0;
// 	int len;
// 	const char *value;

// 	err = rk_system_get_deivce_name(&value);
// 	len = strlen(value);
// 	LOG_DEBUG("len is %d, value is %s, addr is %p\n", len, value, value);
// 	if (sock_write(fd, &len, sizeof(len)) == SOCKERR_CLOSED)
// 		return -1;
// 	if (sock_write(fd, value, len) == SOCKERR_CLOSED)
// 		return -1;
// 	if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED)
// 		return -1;

// 	return 0;
// }

// int ser_rk_system_get_telecontrol_id(int fd) {
// 	int err = 0;
// 	int len;
// 	const char *value;

// 	err = rk_system_get_telecontrol_id(&value);
// 	len = strlen(value);
// 	LOG_DEBUG("len is %d, value is %s, addr is %p\n", len, value, value);
// 	if (sock_write(fd, &len, sizeof(len)) == SOCKERR_CLOSED)
// 		return -1;
// 	if (sock_write(fd, value, len) == SOCKERR_CLOSED)
// 		return -1;
// 	if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED)
// 		return -1;

// 	return 0;
// }

// int ser_rk_system_get_model(int fd) {
// 	int err = 0;
// 	int len;
// 	const char *value;

// 	err = rk_system_get_model(&value);
// 	len = strlen(value);
// 	LOG_DEBUG("len is %d, value is %s, addr is %p\n", len, value, value);
// 	if (sock_write(fd, &len, sizeof(len)) == SOCKERR_CLOSED)
// 		return -1;
// 	if (sock_write(fd, value, len) == SOCKERR_CLOSED)
// 		return -1;
// 	if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED)
// 		return -1;

// 	return 0;
// }

// int ser_rk_system_get_serial_number(int fd) {
// 	int err = 0;
// 	int len;
// 	const char *value;

// 	err = rk_system_get_serial_number(&value);
// 	len = strlen(value);
// 	LOG_DEBUG("len is %d, value is %s, addr is %p\n", len, value, value);
// 	if (sock_write(fd, &len, sizeof(len)) == SOCKERR_CLOSED)
// 		return -1;
// 	if (sock_write(fd, value, len) == SOCKERR_CLOSED)
// 		return -1;
// 	if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED)
// 		return -1;

// 	return 0;
// }

// int ser_rk_system_get_firmware_version(int fd) {
// 	int err = 0;
// 	int len;
// 	const char *value;

// 	err = rk_system_get_firmware_version(&value);
// 	len = strlen(value);
// 	LOG_DEBUG("len is %d, value is %s, addr is %p\n", len, value, value);
// 	if (sock_write(fd, &len, sizeof(len)) == SOCKERR_CLOSED)
// 		return -1;
// 	if (sock_write(fd, value, len) == SOCKERR_CLOSED)
// 		return -1;
// 	if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED)
// 		return -1;

// 	return 0;
// }

// int ser_rk_system_get_encoder_version(int fd) {
// 	int err = 0;
// 	int len;
// 	const char *value;

// 	err = rk_system_get_encoder_version(&value);
// 	len = strlen(value);
// 	LOG_DEBUG("len is %d, value is %s, addr is %p\n", len, value, value);
// 	if (sock_write(fd, &len, sizeof(len)) == SOCKERR_CLOSED)
// 		return -1;
// 	if (sock_write(fd, value, len) == SOCKERR_CLOSED)
// 		return -1;
// 	if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED)
// 		return -1;

// 	return 0;
// }

// int ser_rk_system_get_web_version(int fd) {
// 	int err = 0;
// 	int len;
// 	const char *value;

// 	err = rk_system_get_web_version(&value);
// 	len = strlen(value);
// 	LOG_DEBUG("len is %d, value is %s, addr is %p\n", len, value, value);
// 	if (sock_write(fd, &len, sizeof(len)) == SOCKERR_CLOSED)
// 		return -1;
// 	if (sock_write(fd, value, len) == SOCKERR_CLOSED)
// 		return -1;
// 	if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED)
// 		return -1;

// 	return 0;
// }

// int ser_rk_system_get_plugin_version(int fd) {
// 	int err = 0;
// 	int len;
// 	const char *value;

// 	err = rk_system_get_plugin_version(&value);
// 	len = strlen(value);
// 	LOG_DEBUG("len is %d, value is %s, addr is %p\n", len, value, value);
// 	if (sock_write(fd, &len, sizeof(len)) == SOCKERR_CLOSED)
// 		return -1;
// 	if (sock_write(fd, value, len) == SOCKERR_CLOSED)
// 		return -1;
// 	if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED)
// 		return -1;

// 	return 0;
// }

// int ser_rk_system_get_channels_number(int fd) {
// 	int err = 0;
// 	int len;
// 	const char *value;

// 	err = rk_system_get_channels_number(&value);
// 	len = strlen(value);
// 	LOG_DEBUG("len is %d, value is %s, addr is %p\n", len, value, value);
// 	if (sock_write(fd, &len, sizeof(len)) == SOCKERR_CLOSED)
// 		return -1;
// 	if (sock_write(fd, value, len) == SOCKERR_CLOSED)
// 		return -1;
// 	if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED)
// 		return -1;

// 	return 0;
// }

// int ser_rk_system_get_hard_disks_number(int fd) {
// 	int err = 0;
// 	int len;
// 	const char *value;

// 	err = rk_system_get_hard_disks_number(&value);
// 	len = strlen(value);
// 	LOG_DEBUG("len is %d, value is %s, addr is %p\n", len, value, value);
// 	if (sock_write(fd, &len, sizeof(len)) == SOCKERR_CLOSED)
// 		return -1;
// 	if (sock_write(fd, value, len) == SOCKERR_CLOSED)
// 		return -1;
// 	if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED)
// 		return -1;

// 	return 0;
// }

// int ser_rk_system_get_alarm_inputs_number(int fd) {
// 	int err = 0;
// 	int len;
// 	const char *value;

// 	err = rk_system_get_alarm_inputs_number(&value);
// 	len = strlen(value);
// 	LOG_DEBUG("len is %d, value is %s, addr is %p\n", len, value, value);
// 	if (sock_write(fd, &len, sizeof(len)) == SOCKERR_CLOSED)
// 		return -1;
// 	if (sock_write(fd, value, len) == SOCKERR_CLOSED)
// 		return -1;
// 	if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED)
// 		return -1;

// 	return 0;
// }

// int ser_rk_system_get_alarm_outputs_number(int fd) {
// 	int err = 0;
// 	int len;
// 	const char *value;

// 	err = rk_system_get_alarm_outputs_number(&value);
// 	len = strlen(value);
// 	LOG_DEBUG("len is %d, value is %s, addr is %p\n", len, value, value);
// 	if (sock_write(fd, &len, sizeof(len)) == SOCKERR_CLOSED)
// 		return -1;
// 	if (sock_write(fd, value, len) == SOCKERR_CLOSED)
// 		return -1;
// 	if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED)
// 		return -1;

// 	return 0;
// }

// int ser_rk_system_get_firmware_version_info(int fd) {
// 	int err = 0;
// 	int len;
// 	const char *value;

// 	err = rk_system_get_firmware_version_info(&value);
// 	len = strlen(value);
// 	LOG_DEBUG("len is %d, value is %s, addr is %p\n", len, value, value);
// 	if (sock_write(fd, &len, sizeof(len)) == SOCKERR_CLOSED)
// 		return -1;
// 	if (sock_write(fd, value, len) == SOCKERR_CLOSED)
// 		return -1;
// 	if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED)
// 		return -1;

// 	return 0;
// }

// int ser_rk_system_get_manufacturer(int fd) {
// 	int err = 0;
// 	int len;
// 	const char *value;

// 	err = rk_system_get_manufacturer(&value);
// 	len = strlen(value);
// 	LOG_DEBUG("len is %d, value is %s, addr is %p\n", len, value, value);
// 	if (sock_write(fd, &len, sizeof(len)) == SOCKERR_CLOSED)
// 		return -1;
// 	if (sock_write(fd, value, len) == SOCKERR_CLOSED)
// 		return -1;
// 	if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED)
// 		return -1;

// 	return 0;
// }

// int ser_rk_system_get_hardware_id(int fd) {
// 	int err = 0;
// 	int len;
// 	const char *value;

// 	err = rk_system_get_hardware_id(&value);
// 	len = strlen(value);
// 	LOG_DEBUG("len is %d, value is %s, addr is %p\n", len, value, value);
// 	if (sock_write(fd, &len, sizeof(len)) == SOCKERR_CLOSED)
// 		return -1;
// 	if (sock_write(fd, value, len) == SOCKERR_CLOSED)
// 		return -1;
// 	if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED)
// 		return -1;

// 	return 0;
// }

// int ser_rk_system_set_deivce_name(int fd) {
// 	int ret = 0;
// 	int len;
// 	char *value = NULL;

// 	if (sock_read(fd, &len, sizeof(int)) == SOCKERR_CLOSED)
// 		return -1;
// 	LOG_DEBUG("len is %d\n", len);
// 	if (len) {
// 		value = (char *)malloc(len);
// 		if (sock_read(fd, value, len) == SOCKERR_CLOSED) {
// 			free(value);
// 			return -1;
// 		}
// 		LOG_DEBUG("value is %s\n", value);
// 		ret = rk_system_set_deivce_name(value);
// 		free(value);
// 		if (sock_write(fd, &ret, sizeof(int)) == SOCKERR_CLOSED)
// 			return -1;
// 	}

// 	return 0;
// }

// int ser_rk_system_set_telecontrol_id(int fd) {
// 	int ret = 0;
// 	int len;
// 	char *value = NULL;

// 	if (sock_read(fd, &len, sizeof(int)) == SOCKERR_CLOSED)
// 		return -1;
// 	if (len) {
// 		value = (char *)malloc(len);
// 		if (sock_read(fd, value, len) == SOCKERR_CLOSED) {
// 			free(value);
// 			return -1;
// 		}
// 		LOG_DEBUG("value is %s\n", value);
// 		ret = rk_system_set_telecontrol_id(value);
// 		free(value);
// 		if (sock_write(fd, &ret, sizeof(int)) == SOCKERR_CLOSED)
// 			return -1;
// 	}

// 	return 0;
// }

// int ser_rk_system_reboot(int fd) {
// 	int err = 0;

// 	LOG_DEBUG("begin\n");
// 	err = rk_system_reboot();
// 	if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED)
// 		return -1;

// 	return 0;
// }

// int ser_rk_system_factory_reset(int fd) {
// 	int err = 0;

// 	LOG_DEBUG("begin\n");
// 	err = rk_system_factory_reset();
// 	if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED)
// 		return -1;

// 	return 0;
// }

// int ser_rk_system_export_log(int fd) {
// 	int ret = 0;
// 	int len;
// 	char *value = NULL;

// 	if (sock_read(fd, &len, sizeof(int)) == SOCKERR_CLOSED)
// 		return -1;
// 	if (len) {
// 		value = (char *)malloc(len);
// 		if (sock_read(fd, value, len) == SOCKERR_CLOSED) {
// 			free(value);
// 			return -1;
// 		}
// 		LOG_DEBUG("value is %s\n", value);
// 		ret = rk_system_export_log(value);
// 		free(value);
// 		if (sock_write(fd, &ret, sizeof(int)) == SOCKERR_CLOSED)
// 			return -1;
// 	}

// 	return 0;
// }

// int ser_rk_system_export_db(int fd) {
// 	int ret = 0;
// 	int len;
// 	char *value = NULL;

// 	if (sock_read(fd, &len, sizeof(int)) == SOCKERR_CLOSED)
// 		return -1;
// 	if (len) {
// 		value = (char *)malloc(len);
// 		if (sock_read(fd, value, len) == SOCKERR_CLOSED) {
// 			free(value);
// 			return -1;
// 		}
// 		LOG_DEBUG("value is %s\n", value);
// 		ret = rk_system_export_db(value);
// 		free(value);
// 		if (sock_write(fd, &ret, sizeof(int)) == SOCKERR_CLOSED)
// 			return -1;
// 	}

// 	return 0;
// }

// int ser_rk_system_import_db(int fd) {
// 	int ret = 0;
// 	int len;
// 	char *value = NULL;

// 	if (sock_read(fd, &len, sizeof(int)) == SOCKERR_CLOSED)
// 		return -1;
// 	if (len) {
// 		value = (char *)malloc(len);
// 		if (sock_read(fd, value, len) == SOCKERR_CLOSED) {
// 			free(value);
// 			return -1;
// 		}
// 		LOG_DEBUG("value is %s\n", value);
// 		ret = rk_system_import_db(value);
// 		free(value);
// 		if (sock_write(fd, &ret, sizeof(int)) == SOCKERR_CLOSED)
// 			return -1;
// 	}

// 	return 0;
// }

// int ser_rk_system_upgrade(int fd) {
// 	int ret = 0;
// 	int len;
// 	char *value = NULL;

// 	if (sock_read(fd, &len, sizeof(int)) == SOCKERR_CLOSED)
// 		return -1;
// 	if (len) {
// 		value = (char *)malloc(len);
// 		if (sock_read(fd, value, len) == SOCKERR_CLOSED) {
// 			free(value);
// 			return -1;
// 		}
// 		LOG_DEBUG("value is %s\n", value);
// 		ret = rk_system_upgrade(value);
// 		free(value);
// 		if (sock_write(fd, &ret, sizeof(int)) == SOCKERR_CLOSED)
// 			return -1;
// 	}

// 	return 0;
// }
// int ser_rk_system_get_user_details(int fd) {
// 	int err = 0;
// 	int len, sql_query_len;
// 	char* sql_query = NULL;
// 	char value[5000];

// 	LOG_INFO("\n============comes in %s============\n",__func__);
	
// 	memset(value, '\0', 1); // set terminator

// 	if (sock_read(fd, &sql_query_len, sizeof(int)) == SOCKERR_CLOSED) { //receive lenght of sql query
// 		LOG_ERROR("\nCannot read sql query len\n");
// 		return -1;
// 	}

// 	sql_query = (char *)malloc(sql_query_len);
// 	if (sql_query == NULL) {
// 		LOG_ERROR("\nmalloc failed in %s\n", __func__);
// 		return -1;
// 	}

// 	if (sock_read(fd, sql_query, sql_query_len) == SOCKERR_CLOSED) { // receive the sql query after receivng the length
// 		free(sql_query);
// 		LOG_ERROR("\nCannot read sql_query\n");
// 		return -1;
// 	}
// 	sql_query[strlen(sql_query)] = '\0'; // add NULL terminator to the end of sequence
// 	// memset(sql_query + sql_query_len, '\0', 1);  // add NULL terminator to the end of sequence
// 	err = rk_system_get_user_details(value, sql_query); //run the query and store the results in value
// 	len = strlen(value);
// 	LOG_DEBUG("len is %d, value is %s, addr is %p\n", len, value, value);
// 	if (sock_write(fd, &len, sizeof(len)) == SOCKERR_CLOSED) { // send length of the value
// 		free(sql_query);
// 		return -1;
// 	}
// 	if (sock_write(fd, value, len) == SOCKERR_CLOSED) { //send the value itself
// 		free(sql_query);
// 		return -1;
// 	}
// 	if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED)
// 	{
// 		free(sql_query);
// 		return -1;
// 	}
// 	free(sql_query);
// 	return 0;
// }

// int ser_rk_system_get_user_num(int fd) {
// 	int err = 0;
// 	int value;

// 	err = rk_system_get_user_num(&value);
// 	LOG_DEBUG("value is %d\n", value);
// 	if (sock_write(fd, &value, sizeof(value)) == SOCKERR_CLOSED)
// 		return -1;
// 	if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED)
// 		return -1;

// 	return 0;
// }

// int ser_rk_system_check_user_exists(int fd) {
// 	int ret = 0;
// 	int  user_name_len, password_len, userlevel;
// 	bool need_password_userlevel;
// 	char *user_name = NULL;
// 	char password[70];

// 	if (sock_read(fd, &need_password_userlevel, sizeof(bool)) == SOCKERR_CLOSED)
// 	{
// 		// LOG_DEBUG(" ================================in function ser_rk_system_check_user_exists need_password_userlevel =%d---------\n",need_password_userlevel);
// 		return -1;
// 	}
// 	if (sock_read(fd, &user_name_len, sizeof(int)) == SOCKERR_CLOSED)
// 	{
// 		// LOG_DEBUG(" ================================in function ser_rk_system_check_user_exists username_len = %d---------\n",user_name_len);
// 		return -1;
// 	}
// 	user_name = (char *)malloc(user_name_len);
// 	if (user_name == NULL) {
// 		// LOG_DEBUG(" ================================in function ser_rk_system_check_user_exists username is NULLL---------\n");
// 		return -1;
// 	}
// 	if (sock_read(fd, user_name, user_name_len) == SOCKERR_CLOSED) {
// 		// LOG_DEBUG(" ================================in function ser_rk_system_check_user_exists uerror reading username---------\n");
// 		free(user_name);
// 		return -1;
// 	}

// 	ret = rk_system_check_user_exists(user_name, password,&userlevel,need_password_userlevel);
// 	LOG_DEBUG("user_level is %d, user_name is %s, password is %s, need_password_userlevel is %d \n", userlevel, user_name,
// 	          password,need_password_userlevel);
// 	if(need_password_userlevel)
// 	{
// 		password_len = strlen(password);
// 		if (sock_write(fd, &password_len, sizeof(password_len)) == SOCKERR_CLOSED)
// 			return -1;
// 		if (sock_write(fd, password, password_len) == SOCKERR_CLOSED)
// 			return -1;
// 		if (sock_write(fd, &userlevel, sizeof(userlevel)) == SOCKERR_CLOSED)
// 			return -1;
// 	}
// 	if (sock_write(fd, &ret, sizeof(int)) == SOCKERR_CLOSED)
// 		return -1;
// 	// LOG_DEBUG(" ================================in function ser_rk_system_check_user_exists end with success %d\n",ret);
// 	free(user_name);
// 	return ret;
// }
// int ser_rk_system_verify_security_ans(int fd) {
// 	int ret = 0;
// 	int  user_name_len, password_len,security_ans_len,security_que_len;
// 	char *user_name = NULL;
// 	char *password = NULL;
// 	char *security_ans = NULL;
// 	char *security_que = NULL;

	
// 	if (sock_read(fd, &user_name_len, sizeof(int)) == SOCKERR_CLOSED)
// 	{
// 		// LOG_DEBUG(" ================================in function ser_rk_system_check_user_exists username_len = %d---------\n",user_name_len);
// 		return -1;
// 	}
// 	user_name = (char *)malloc(user_name_len);
// 	if (user_name == NULL) {
// 		// LOG_DEBUG(" ================================in function ser_rk_system_check_user_exists username is NULLL---------\n");
// 		return -1;
// 	}
// 	if (sock_read(fd, user_name, user_name_len) == SOCKERR_CLOSED) {
// 		// LOG_DEBUG(" ================================in function ser_rk_system_check_user_exists uerror reading username---------\n");
// 		free(user_name);
// 		return -1;
// 	}

// 	if (sock_read(fd, &password_len, sizeof(int)) == SOCKERR_CLOSED)
// 	{
// 		// LOG_DEBUG(" ================================in function ser_rk_system_check_user_exists username_len = %d---------\n",user_name_len);
// 		return -1;
// 	}
// 	password = (char *)malloc(password_len);
// 	if (password == NULL) {
// 		//LOG_DEBUG(" after sock_read  password is MULL in function ser_rk_system_add_user\n");
// 		free(user_name);
// 		// LOG_DEBUG(" ================================in function ser_rk_system_check_user_exists username is NULLL---------\n");
// 		return -1;
// 	}
// 	if (sock_read(fd, password, password_len) == SOCKERR_CLOSED) {
// 		// LOG_DEBUG(" ================================in function ser_rk_system_check_user_exists uerror reading username---------\n");
// 		free(user_name);
// 		free(password);
// 		return -1;
// 	}

// 	if (sock_read(fd, &security_ans_len, sizeof(int)) == SOCKERR_CLOSED)
// 	{
// 		// LOG_DEBUG(" ================================in function ser_rk_system_check_user_exists username_len = %d---------\n",user_name_len);
// 		return -1;
// 	}
// 	security_ans = (char *)malloc(security_ans_len);
// 	if (security_ans == NULL) {
// 		free(user_name);
// 		free(password);
// 		// LOG_DEBUG(" ================================in function ser_rk_system_check_user_exists username is NULLL---------\n");
// 		return -1;
// 	}
// 	if (sock_read(fd, security_ans, security_ans_len) == SOCKERR_CLOSED) {
// 		// LOG_DEBUG(" ================================in function ser_rk_system_check_user_exists uerror reading username---------\n");
// 		free(user_name);
// 		free(password);
// 		free(security_ans);
// 		return -1;
// 	}
// 	if (sock_read(fd, &security_que_len, sizeof(int)) == SOCKERR_CLOSED)
// 	{
// 		// LOG_DEBUG(" ================================in function ser_rk_system_check_user_exists username_len = %d---------\n",user_name_len);
// 		return -1;
// 	}
// 	security_que = (char *)malloc(security_que_len);
// 	if (security_que == NULL) {
// 		free(user_name);
// 		free(password);
// 		free(security_ans);
// 		// LOG_DEBUG(" ================================in function ser_rk_system_check_user_exists username is NULLL---------\n");
// 		return -1;
// 	}
// 	if (sock_read(fd, security_que, security_que_len) == SOCKERR_CLOSED) {
// 		// LOG_DEBUG(" ================================in function ser_rk_system_check_user_exists uerror reading username---------\n");
// 		free(user_name);
// 		free(password);
// 		free(security_ans);
// 		free(security_que);
// 		return -1;
// 	}
// 	ret = rk_system_verify_security_ans(user_name, password,security_ans,security_que);
// 	LOG_DEBUG( "user_name is %s, password is %s, security_ans is %s, security_que is %s\n", user_name,
// 	          password,security_ans,security_que);
// 	if (sock_write(fd, &ret, sizeof(int)) == SOCKERR_CLOSED)
// 		return -1;
// 	LOG_DEBUG(" ================================in function ser_rk_system_check_user_exists end with success %d\n",ret);
// 	free(user_name);
// 	free(password);
// 	free(security_ans);
// 	free(security_que);
// 	return ret;
// }


// int ser_rk_system_manage_user_record(int fd) {
// 	int err = 0;
// 	int len, sql_query_len;
// 	char* sql_query = NULL;

// 	LOG_INFO("\n============comes in %s============\n",__func__);
	

// 	if (sock_read(fd, &sql_query_len, sizeof(int)) == SOCKERR_CLOSED) { //receive lenght of sql query
// 		LOG_ERROR("\nCannot read sql query len\n");
// 		return -1;
// 	}

// 	LOG_INFO("\n============query length %d============\n",sql_query_len);
// 	sql_query = (char *)malloc(sql_query_len);
// 	if (sql_query == NULL) {
// 		LOG_ERROR("\nmalloc failed in %s in allocating sql_query \n", __func__);
// 		return -1;
// 	}

// 	if (sock_read(fd, sql_query, sql_query_len) == SOCKERR_CLOSED) { // receive the sql query after receivng the length
// 		if (sql_query){
// 			free(sql_query);
// 		}
// 		LOG_ERROR("\nCannot read sql_query\n");
// 		return -1;
// 	}
// 	// memset(sql_query + sql_query_len, '\0', 1); // set terminator
// 	sql_query[strlen(sql_query)] = '\0'; // add NULL terminator to the end of sequence
	
// 	LOG_INFO("============sql query in %s\n",sql_query);
// 	err = rk_system_manage_user_record(sql_query); //run the query and store the results in value

// 	LOG_INFO("error code is %d\n", err);

// 	if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED)
// 	{
// 		if (sql_query){
// 			free(sql_query);
// 		}
// 		return -1;
// 	}
// 	if (sql_query){
// 			free(sql_query);
// 		}
// 	return 0;
// }
// int ser_rk_system_record_exists(int fd) {
// 	int err = 0;
// 	int len, sql_query_len;
// 	char* sql_query = NULL;

// 	LOG_INFO("\n============comes in %s============\n",__func__);
	

// 	if (sock_read(fd, &sql_query_len, sizeof(int)) == SOCKERR_CLOSED) { //receive lenght of sql query
// 		LOG_ERROR("\nCannot read sql query len\n");
// 		return -1;
// 	}
// 	printf("%s sql_query_len is %d\n",__func__,sql_query_len);
// 	LOG_INFO("\n============query length %d============\n",sql_query_len);
// 	sql_query = (char *)malloc(sql_query_len);
// 	if (sql_query == NULL) {
// 		LOG_ERROR("\nmalloc failed in %s in allocating sql_query \n", __func__);
// 		return -1;
// 	}

// 	if (sock_read(fd, sql_query, sql_query_len) == SOCKERR_CLOSED) { // receive the sql query after receivng the length
// 		if (sql_query){
// 			free(sql_query);
// 		}
// 		LOG_ERROR("\nCannot read sql_query\n");
// 		return -1;
// 	}
// 	// memset(sql_query + sql_query_len, '\0', 1); // set terminator
// 	sql_query[strlen(sql_query)] = '\0'; // add NULL terminator to the end of sequence
	
// 	LOG_INFO("============sql query in %s\n",sql_query);
// 	err = rk_system_record_exists(sql_query); //run the query and store the results in value

// 	LOG_INFO("error code is %d\n", err);

// 	if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED)
// 	{
// 		if (sql_query){
// 			free(sql_query);
// 		}
// 		return -1;
// 	}
// 	if (sql_query){
// 			free(sql_query);
// 		}
// 	return 0;
// }



// int ser_rk_analytics_find_records(int fd) {
// 	int err = 0;
// 	int len, sql_query_len;
// 	char* sql_query = NULL;
// 	char value[5000];

// 	printf("\n============comes in %s============\n",__func__);
	
// 	memset(value, '\0', 1); // set terminator
// 	if (sock_read(fd, &sql_query_len, sizeof(int)) == SOCKERR_CLOSED) { //receive lenght of sql query
// 		LOG_ERROR("\nCannot read sql query len\n");
// 		return -1;
// 	}
// 	printf("%s sql_query_len is %d\n",__func__,sql_query_len);
// 	sql_query = (char *)malloc(sql_query_len+1);
// 	if (sql_query == NULL) {
// 		LOG_ERROR("\nmalloc failed in %s\n", __func__);
// 		return -1;
// 	}
// 	memset(sql_query,0x00,sql_query_len+1);
// 	if (sock_read(fd, sql_query, sql_query_len) == SOCKERR_CLOSED) { // receive the sql query after receivng the length
// 		if (sql_query){
// 			free(sql_query);
// 		}
// 		LOG_ERROR("\nCannot read sql_query\n");
// 		return -1;
// 	}

// 	// memset(sql_query + sql_query_len, '\0', 1);
// 	sql_query[sql_query_len] = '\0'; // add NULL terminator to the end of sequence

// 	err = rk_analytics_find_records(value, sql_query); //run the query and store the results in value
// 	len = strlen(value);
// 	printf("%s len is %d, value is %s, addr is %p\n", __func__,len, value, value);
// 	if (sock_write(fd, &len, sizeof(len)) == SOCKERR_CLOSED) { // send length of the value
// 		if (sql_query){
// 			free(sql_query);
// 		}
// 		return -1;
// 	}
// 	if (sock_write(fd, value, len) == SOCKERR_CLOSED) { //send the value itself
// 		if (sql_query){
// 			free(sql_query);
// 		}
// 		return -1;
// 	}
// 	if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED)
// 	{
// 		if (sql_query){
// 			free(sql_query);
// 		}
// 		return -1;
// 	}
// 	if(sql_query) {
// 		free(sql_query);
// 	}
// 	return 0;
// }


// int ser_rk_analytics_get_rules(int fd) {
// 	int err = 0;
// 	int len, sql_query_len;
// 	char* sql_query = NULL;
// 	char value[5000];

// 	LOG_INFO("\n============comes in %s============\n",__func__);
	
// 	memset(value, '\0', 1); // set terminator

// 	if (sock_read(fd, &sql_query_len, sizeof(int)) == SOCKERR_CLOSED) { //receive lenght of sql query
// 		LOG_ERROR("\nCannot read sql query len\n");
// 		return -1;
// 	}
// 	printf("%s sql_query_len is %d\n",__func__,sql_query_len);
// 	sql_query = (char *)malloc(sql_query_len+1);
// 	if (sql_query == NULL) {
// 		LOG_ERROR("\nmalloc failed in %s\n", __func__);
// 		return -1;
// 	}

// 	if (sock_read(fd, sql_query, sql_query_len) == SOCKERR_CLOSED) { // receive the sql query after receivng the length
// 		free(sql_query);
// 		LOG_ERROR("\nCannot read sql_query\n");
// 		return -1;
// 	}
// 	sql_query[sql_query_len] = '\0'; // add NULL terminator to the end of sequence
// 	// memset(sql_query + sql_query_len, '\0', 1);  // add NULL terminator to the end of sequence
// 	err = rk_analytics_get_rules(value, sql_query); //run the query and store the results in value
// 	len = strlen(value);
// 	// LOG_DEBUG("len is %d, value is %s, addr is %p\n", len, value, value);
// 	if (sock_write(fd, &len, sizeof(len)) == SOCKERR_CLOSED) { // send length of the value
// 		free(sql_query);
// 		return -1;
// 	}
// 	if (sock_write(fd, value, len) == SOCKERR_CLOSED) { //send the value itself
// 		free(sql_query);
// 		return -1;
// 	}
// 	if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED)
// 	{
// 		free(sql_query);
// 		return -1;
// 	}
// 	free(sql_query);
// 	return 0;
// }




// int ser_rk_analytics_update_db(int fd) {
// 	int err = 0;
// 	int len, sql_query_len;
// 	char* sql_query = NULL;

// 	LOG_INFO("\n============comes in %s============\n",__func__);
// 	if (sock_read(fd, &sql_query_len, sizeof(int)) == SOCKERR_CLOSED) { //receive lenght of sql query
// 		LOG_ERROR("\nCannot read sql query len\n");
// 		return -1;
// 	}

// 	// printf("%s sql_query_len is %d\n",__func__,sql_query_len);

// 	// LOG_INFO("\n%s ============query length %d============\n",__func__,sql_query_len);
// 	sql_query = (char *)malloc(sql_query_len+1);
// 	if (sql_query == NULL) {
// 		LOG_ERROR("\nmalloc failed in %s in allocating sql_query \n", __func__);
// 		return -1;
// 	}
// 	memset(sql_query,0x00,sql_query_len+1);
// 	if (sock_read(fd, sql_query, sql_query_len) == SOCKERR_CLOSED) { // receive the sql query after receivng the length
// 		if (sql_query){
// 			free(sql_query);
// 		}
// 		LOG_ERROR("\nCannot read sql_query\n");
// 		return -1;
// 	}
// 	// memset(sql_query + sql_query_len, '\0', 1); // set terminator
// 	sql_query[sql_query_len] = '\0'; // add NULL terminator to the end of sequence
	
// 	// printf("%s ============sql query is %s\n",__func__,sql_query);
// 	err = rk_analytics_update_db(sql_query); //run the query and store the results in value
// 	// printf("error code is %d\n", err);

// 	if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED)
// 	{
// 		if (sql_query){
// 			free(sql_query);
// 		}
// 		return -1;
// 	}
// 	if (sql_query){
// 			free(sql_query);
// 		}
// 	return 0;
// }

// int ser_rk_count_db_records(int fd) {

// 	int err = 0;
// 	int len, sql_query_len;
// 	char* sql_query = NULL;
// 	int value = 0;

// 	// LOG_INFO("\n============comes in %s============\n",__func__);
	

// 	if (sock_read(fd, &sql_query_len, sizeof(int)) == SOCKERR_CLOSED) { //receive lenght of sql query
// 		LOG_ERROR("\nCannot read sql query len\n");
// 		return -1;
// 	}
// 	printf("%s sql_query_len is %d\n",__func__,sql_query_len);
// 	sql_query = (char *)malloc(sql_query_len+1);
// 	if (sql_query == NULL) {
// 		LOG_ERROR("\nmalloc failed in %s\n", __func__);
// 		return -1;
// 	}
// 	memset(sql_query,0x00,sql_query_len+1);
// 	if (sock_read(fd, sql_query, sql_query_len) == SOCKERR_CLOSED) { // receive the sql query after receivng the length
// 		if (sql_query){
// 			free(sql_query);
// 		}
// 		LOG_ERROR("\nCannot read sql_query\n");
// 		return -1;
// 	}
// 	// memset(sql_query + sql_query_len, '\0', 1);
// 	sql_query[sql_query_len] = '\0'; // add NULL terminator to the end of sequence
	
// 	err = rk_count_db_records(&value, sql_query); //run the query and store the results in value
	

// 	// LOG_DEBUG("value is %d, addr is %p\n", value, &value);
	
// 	if (sock_write(fd, &value, sizeof(value)) == SOCKERR_CLOSED) { // send length of the value
// 		if (sql_query){
// 			free(sql_query);
// 		}
// 		return -1;
// 	}

// 	if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED)
// 	{
// 		if (sql_query){
// 			free(sql_query);
// 		}
// 		return -1;
// 	}
// 	if (sql_query){
// 			free(sql_query);
// 		}
// 	return 0;

// }

// int ser_rk_update_analytics_module(int fd) {
// 	int err = 0;
// 	int param = 0;
// 	LOG_INFO("\n============comes in %s============\n",__func__);

// 	if (sock_read(fd, &param, sizeof(int)) == SOCKERR_CLOSED) { //receive lenght of sql query
// 		LOG_ERROR("\nCannot read sql query len\n");
// 		return -1;
// 	}
// 	if (param == 1) {
// 		err = rk_update_analytics_module(); //run the query and store the results in value
// 	}
// 	else {
// 		err = 1;
// 	}
	
// 	if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED) {
// 		return -1;
// 	}
// 	return 0;
// }


// int ser_rk_stop_analytics_module(int fd) {
// 	int err = 0;
// 	int param = 0;
// 	LOG_INFO("\n============comes in %s============\n",__func__);

// 	if (sock_read(fd, &param, sizeof(int)) == SOCKERR_CLOSED) { //receive lenght of sql query
// 		LOG_ERROR("\nCannot read sql query len\n");
// 		return -1;
// 	}
// 	if (param == 1) {
// 		err = rk_stop_analytics_module(); //run the query and store the results in value
// 	}
// 	else {
// 		err = 1;
// 	}
	
// 	if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED) {
// 		return -1;
// 	}
// 	return 0;
// }

// int ser_rk_start_analytics_module(int fd) {
// 	int err = 0;
// 	int param = 0;
// 	LOG_INFO("\n============comes in %s============\n",__func__);

// 	if (sock_read(fd, &param, sizeof(int)) == SOCKERR_CLOSED) { //receive lenght of sql query
// 		LOG_ERROR("\nCannot read sql query len\n");
// 		return -1;
// 	}
// 	if (param == 1) {
// 		err = rk_start_analytics_module(); //run the query and store the results in value
// 	}
// 	else {
// 		err = 1;
// 	}
	
// 	if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED) {
// 		return -1;
// 	}
// 	return 0;
// }


// int ser_rk_update_camera_tamper_module(int fd) {
// 	int err = 0;
// 	int param = 0;
// 	LOG_INFO("\n============comes in %s============\n",__func__);

// 	if (sock_read(fd, &param, sizeof(int)) == SOCKERR_CLOSED) { //receive lenght of sql query
// 		LOG_ERROR("\nCannot read sql query len\n");
// 		return -1;
// 	}
// 	if (param == 1) {
// 		err = rk_update_camera_tamper_module(); //run the query and store the results in value
// 	}
// 	else {
// 		err = 1;
// 	}
	
// 	if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED) {
// 		return -1;
// 	}
// 	return 0;
// }


// int ser_rk_start_camera_tamper_module(int fd) {
// 	int err = 0;
// 	int param = 0;
// 	LOG_INFO("\n============comes in %s============\n",__func__);

// 	if (sock_read(fd, &param, sizeof(int)) == SOCKERR_CLOSED) { //receive lenght of sql query
// 		LOG_ERROR("\nCannot read sql query len\n");
// 		return -1;
// 	}
// 	if (param == 1) {
// 		err = rk_start_camera_tamper_module(); //run the query and store the results in value
// 	}
// 	else {
// 		err = 1;
// 	}
	
// 	if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED) {
// 		return -1;
// 	}
// 	return 0;
// }


// int ser_rk_stop_camera_tamper_module(int fd) {
// 	int err = 0;
// 	int param = 0;
// 	LOG_INFO("\n============comes in %s============\n",__func__);

// 	if (sock_read(fd, &param, sizeof(int)) == SOCKERR_CLOSED) { //receive lenght of sql query
// 		LOG_ERROR("\nCannot read sql query len\n");
// 		return -1;
// 	}
// 	if (param == 1) {
// 		err = rk_stop_camera_tamper_module(); //run the query and store the results in value
// 	}
// 	else {
// 		err = 1;
// 	}
	
// 	if (sock_write(fd, &err, sizeof(int)) == SOCKERR_CLOSED) {
// 		return -1;
// 	}
// 	return 0;
// }


static const struct FunMap map[] = {
    // {(char *)"rk_isp_set", &ser_rk_isp_set},
    // {(char *)"rk_video_set", &ser_rk_video_set},
    // {(char *)"rk_audio_set", &ser_rk_audio_set},
    // // isp scenario
    // {(char *)"rk_isp_get_scenario", &ser_rk_isp_get_scenario},
    // {(char *)"rk_isp_set_scenario", &ser_rk_isp_set_scenario},
	// {(char *)"rk_isp_set_default", &ser_rk_isp_set_default},
    // // isp adjustment
    // {(char *)"rk_isp_get_contrast", &ser_rk_isp_get_contrast},
    // {(char *)"rk_isp_set_contrast", &ser_rk_isp_set_contrast},
    // {(char *)"rk_isp_get_brightness", &ser_rk_isp_get_brightness},
    // {(char *)"rk_isp_set_brightness", &ser_rk_isp_set_brightness},
    // {(char *)"rk_isp_get_saturation", &ser_rk_isp_get_saturation},
    // {(char *)"rk_isp_set_saturation", &ser_rk_isp_set_saturation},
    // {(char *)"rk_isp_get_sharpness", &ser_rk_isp_get_sharpness},
    // {(char *)"rk_isp_set_sharpness", &ser_rk_isp_set_sharpness},
    // {(char *)"rk_isp_get_hue", &ser_rk_isp_get_hue},
    // {(char *)"rk_isp_set_hue", &ser_rk_isp_set_hue},
    // // isp exposure
    // {(char *)"rk_isp_get_exposure_mode", &ser_rk_isp_get_exposure_mode},
    // {(char *)"rk_isp_set_exposure_mode", &ser_rk_isp_set_exposure_mode},
    // {(char *)"rk_isp_get_gain_mode", &ser_rk_isp_get_gain_mode},
    // {(char *)"rk_isp_set_gain_mode", &ser_rk_isp_set_gain_mode},
    // {(char *)"rk_isp_get_exposure_time", &ser_rk_isp_get_exposure_time},
    // {(char *)"rk_isp_set_exposure_time", &ser_rk_isp_set_exposure_time},
    // {(char *)"rk_isp_get_exposure_gain", &ser_rk_isp_get_exposure_gain},
    // {(char *)"rk_isp_set_exposure_gain", &ser_rk_isp_set_exposure_gain},
    // {(char *)"rk_isp_get_frame_rate", &ser_rk_isp_get_frame_rate},
    // {(char *)"rk_isp_set_frame_rate", &ser_rk_isp_set_frame_rate},
    // // isp night_to_day
    // {(char *)"rk_isp_get_night_to_day", &ser_rk_isp_get_night_to_day},
    // {(char *)"rk_isp_set_night_to_day", &ser_rk_isp_set_night_to_day},
    // {(char *)"rk_isp_get_fill_light_mode", &ser_rk_isp_get_fill_light_mode},
    // {(char *)"rk_isp_set_fill_light_mode", &ser_rk_isp_set_fill_light_mode},
    // {(char *)"rk_isp_get_light_brightness", &ser_rk_isp_get_light_brightness},
    // {(char *)"rk_isp_set_light_brightness", &ser_rk_isp_set_light_brightness},
    // {(char *)"rk_isp_get_night_to_day_filter_level", &ser_rk_isp_get_night_to_day_filter_level},
    // {(char *)"rk_isp_set_night_to_day_filter_level", &ser_rk_isp_set_night_to_day_filter_level},
    // {(char *)"rk_isp_get_night_to_day_filter_time", &ser_rk_isp_get_night_to_day_filter_time},
    // {(char *)"rk_isp_set_night_to_day_filter_time", &ser_rk_isp_set_night_to_day_filter_time},
	// {(char *)"rk_isp_set_IR_mode", &ser_rk_isp_set_IR_mode},
	// {(char *)"rk_isp_get_IR_mode", &ser_rk_isp_get_IR_mode},
	// {(char *)"rk_isp_get_IR_start_time", &ser_rk_isp_get_IR_start_time},
	// {(char *)"rk_isp_set_IR_start_time", &ser_rk_isp_set_IR_start_time},
	// {(char *)"rk_isp_get_IR_end_time", &ser_rk_isp_get_IR_end_time},
	// {(char *)"rk_isp_set_IR_end_time", &ser_rk_isp_set_IR_end_time},
	// {(char *)"rk_isp_restart_IR_monitoring_thread", &ser_rk_IR_monitoring_restart},
	// {(char *)"rk_isp_start_IR_monitoring_thread", &ser_rk_isp_start_IR_monitoring_thread},
	// {(char *)"rk_isp_stop_IR_monitoring_thread", &ser_rk_isp_stop_IR_monitoring_thread},
	// {(char *)"rk_isp_get_IR_brightness_mode", &ser_rk_isp_get_IR_brightness_mode},
	// {(char *)"rk_isp_set_IR_brightness_mode", &ser_rk_isp_set_IR_brightness_mode},


    // // isp blc
    // {(char *)"rk_isp_get_hdr", &ser_rk_isp_get_hdr},
    // {(char *)"rk_isp_set_hdr", &ser_rk_isp_set_hdr},
    // {(char *)"rk_isp_get_blc_region", &ser_rk_isp_get_blc_region},
    // {(char *)"rk_isp_set_blc_region", &ser_rk_isp_set_blc_region},
    // {(char *)"rk_isp_get_hlc", &ser_rk_isp_get_hlc},
    // {(char *)"rk_isp_set_hlc", &ser_rk_isp_set_hlc},
    // {(char *)"rk_isp_get_hdr_level", &ser_rk_isp_get_hdr_level},
    // {(char *)"rk_isp_set_hdr_level", &ser_rk_isp_set_hdr_level},
    // {(char *)"rk_isp_get_blc_strength", &ser_rk_isp_get_blc_strength},
    // {(char *)"rk_isp_set_blc_strength", &ser_rk_isp_set_blc_strength},
    // {(char *)"rk_isp_get_hlc_level", &ser_rk_isp_get_hlc_level},
    // {(char *)"rk_isp_set_hlc_level", &ser_rk_isp_set_hlc_level},
    // {(char *)"rk_isp_get_dark_boost_level", &ser_rk_isp_get_dark_boost_level},
    // {(char *)"rk_isp_set_dark_boost_level", &ser_rk_isp_set_dark_boost_level},
    // // isp white_blance
    // {(char *)"rk_isp_get_white_blance_style", &ser_rk_isp_get_white_blance_style},
    // {(char *)"rk_isp_set_white_blance_style", &ser_rk_isp_set_white_blance_style},
    // {(char *)"rk_isp_get_white_blance_red", &ser_rk_isp_get_white_blance_red},
    // {(char *)"rk_isp_set_white_blance_red", &ser_rk_isp_set_white_blance_red},
    // {(char *)"rk_isp_get_white_blance_green", &ser_rk_isp_get_white_blance_green},
    // {(char *)"rk_isp_set_white_blance_green", &ser_rk_isp_set_white_blance_green},
    // {(char *)"rk_isp_get_white_blance_blue", &ser_rk_isp_get_white_blance_blue},
    // {(char *)"rk_isp_set_white_blance_blue", &ser_rk_isp_set_white_blance_blue},
    // // isp enhancement
    // {(char *)"rk_isp_get_noise_reduce_mode", &ser_rk_isp_get_noise_reduce_mode},
    // {(char *)"rk_isp_set_noise_reduce_mode", &ser_rk_isp_set_noise_reduce_mode},
    // {(char *)"rk_isp_get_dehaze", &ser_rk_isp_get_dehaze},
    // {(char *)"rk_isp_set_dehaze", &ser_rk_isp_set_dehaze},
    // {(char *)"rk_isp_get_gray_scale_mode", &ser_rk_isp_get_gray_scale_mode},
    // {(char *)"rk_isp_set_gray_scale_mode", &ser_rk_isp_set_gray_scale_mode},
    // {(char *)"rk_isp_get_distortion_correction", &ser_rk_isp_get_distortion_correction},
    // {(char *)"rk_isp_set_distortion_correction", &ser_rk_isp_set_distortion_correction},
    // {(char *)"rk_isp_get_spatial_denoise_level", &ser_rk_isp_get_spatial_denoise_level},
    // {(char *)"rk_isp_set_spatial_denoise_level", &ser_rk_isp_set_spatial_denoise_level},
    // {(char *)"rk_isp_get_temporal_denoise_level", &ser_rk_isp_get_temporal_denoise_level},
    // {(char *)"rk_isp_set_temporal_denoise_level", &ser_rk_isp_set_temporal_denoise_level},
    // {(char *)"rk_isp_get_dehaze_level", &ser_rk_isp_get_dehaze_level},
    // {(char *)"rk_isp_set_dehaze_level", &ser_rk_isp_set_dehaze_level},
    // {(char *)"rk_isp_get_fec_level", &ser_rk_isp_get_fec_level},
    // {(char *)"rk_isp_set_fec_level", &ser_rk_isp_set_fec_level},
    // {(char *)"rk_isp_get_ldch_level", &ser_rk_isp_get_ldch_level},
    // {(char *)"rk_isp_set_ldch_level", &ser_rk_isp_set_ldch_level},
    // // isp video_adjustment
    // {(char *)"rk_isp_get_power_line_frequency_mode", &ser_rk_isp_get_power_line_frequency_mode},
    // {(char *)"rk_isp_set_power_line_frequency_mode", &ser_rk_isp_set_power_line_frequency_mode},
    // {(char *)"rk_isp_get_image_flip", &ser_rk_isp_get_image_flip},
    // {(char *)"rk_isp_set_image_flip", &ser_rk_isp_set_image_flip},
    // // isp auto focus
    // {(char *)"rk_isp_get_af_mode", &ser_rk_isp_get_af_mode},
    // {(char *)"rk_isp_set_af_mode", &ser_rk_isp_set_af_mode},
    // {(char *)"rk_isp_get_zoom_level", &ser_rk_isp_get_zoom_level},
    // {(char *)"rk_isp_get_focus_level", &ser_rk_isp_get_focus_level},
    // {(char *)"rk_isp_af_zoom_in", &ser_rk_isp_af_zoom_in},
    // {(char *)"rk_isp_af_zoom_out", &ser_rk_isp_af_zoom_out},
    // {(char *)"rk_isp_af_focus_in", &ser_rk_isp_af_focus_in},
    // {(char *)"rk_isp_af_focus_out", &ser_rk_isp_af_focus_out},
    // {(char *)"rk_isp_af_focus_once", &ser_rk_isp_af_focus_once},
    // // audio
    // {(char *)"rk_audio_restart", &ser_rk_audio_restart},
	// {(char *)"rkipc_audio_deinit", &ser_rkipc_audio_deinit},
    // {(char *)"rk_audio_get_bit_rate", &ser_rk_audio_get_bit_rate},
    // {(char *)"rk_audio_set_bit_rate", &ser_rk_audio_set_bit_rate},
    // {(char *)"rk_audio_get_sample_rate", &ser_rk_audio_get_sample_rate},
    // {(char *)"rk_audio_set_sample_rate", &ser_rk_audio_set_sample_rate},
    // {(char *)"rk_audio_get_volume", &ser_rk_audio_get_volume},
    // {(char *)"rk_audio_set_volume", &ser_rk_audio_set_volume},
    // {(char *)"rk_audio_get_enable_vqe", &ser_rk_audio_get_enable_vqe},
    // {(char *)"rk_audio_set_enable_vqe", &ser_rk_audio_set_enable_vqe},
    // {(char *)"rk_audio_get_encode_type", &ser_rk_audio_get_encode_type},
    // {(char *)"rk_audio_set_encode_type", &ser_rk_audio_set_encode_type},
    // // video
    // {(char *)"rk_video_restart", &ser_rk_video_restart},
    // {(char *)"rk_video_get_gop", &ser_rk_video_get_gop},
    // {(char *)"rk_video_set_gop", &ser_rk_video_set_gop},
    // {(char *)"rk_video_get_max_rate", &ser_rk_video_get_max_rate},
    // {(char *)"rk_video_set_max_rate", &ser_rk_video_set_max_rate},
    // {(char *)"rk_video_get_RC_mode", &ser_rk_video_get_RC_mode},
    // {(char *)"rk_video_set_RC_mode", &ser_rk_video_set_RC_mode},
    // {(char *)"rk_video_get_output_data_type", &ser_rk_video_get_output_data_type},
    // {(char *)"rk_video_set_output_data_type", &ser_rk_video_set_output_data_type},
    // {(char *)"rk_video_get_rc_quality", &ser_rk_video_get_rc_quality},
    // {(char *)"rk_video_set_rc_quality", &ser_rk_video_set_rc_quality},
    // // {(char *)"rk_video_get_smart", &ser_rk_video_get_smart},
    // // {(char *)"rk_video_set_smart", &ser_rk_video_set_smart},
    // // {(char *)"rk_video_get_gop_mode", &ser_rk_video_get_gop_mode},
    // // {(char *)"rk_video_set_gop_mode", &ser_rk_video_set_gop_mode},
    // {(char *)"rk_video_get_stream_type", &ser_rk_video_get_stream_type},
    // {(char *)"rk_video_set_stream_type", &ser_rk_video_set_stream_type},
    // // {(char *)"rk_video_get_h264_profile", &ser_rk_video_get_h264_profile},
    // // {(char *)"rk_video_set_h264_profile", &ser_rk_video_set_h264_profile},
    // {(char *)"rk_video_get_resolution", &ser_rk_video_get_resolution},
    // {(char *)"rk_video_set_resolution", &ser_rk_video_set_resolution},
    // {(char *)"rk_video_get_frame_rate", &ser_rk_video_get_frame_rate},
    // {(char *)"rk_video_set_frame_rate", &ser_rk_video_set_frame_rate},
    // // {(char *)"rk_video_get_frame_rate_in", &ser_rk_video_get_frame_rate_in},
    // // {(char *)"rk_video_set_frame_rate_in", &ser_rk_video_set_frame_rate_in},
    // {(char *)"rk_video_get_rotation", &ser_rk_video_get_rotation},
    // {(char *)"rk_video_set_rotation", &ser_rk_video_set_rotation},
    // // jpeg
    // // {(char *)"rk_video_get_enable_cycle_snapshot", &ser_rk_video_get_enable_cycle_snapshot},
    // // {(char *)"rk_video_set_enable_cycle_snapshot", &ser_rk_video_set_enable_cycle_snapshot},
    // // {(char *)"rk_video_get_image_quality", &ser_rk_video_get_image_quality},
    // // {(char *)"rk_video_set_image_quality", &ser_rk_video_set_image_quality},
    // // {(char *)"rk_video_get_snapshot_interval_ms", &ser_rk_video_get_snapshot_interval_ms},
    // // {(char *)"rk_video_set_snapshot_interval_ms", &ser_rk_video_set_snapshot_interval_ms},
    // // {(char *)"rk_video_get_jpeg_resolution", &ser_rk_video_get_jpeg_resolution},
    // // {(char *)"rk_video_set_jpeg_resolution", &ser_rk_video_set_jpeg_resolution},
    // // osd.common
    // {(char *)"rk_osd_get_is_presistent_text", &ser_rk_osd_get_is_presistent_text},
    // {(char *)"rk_osd_set_is_presistent_text", &ser_rk_osd_set_is_presistent_text},
    // {(char *)"rk_osd_get_font_size", &ser_rk_osd_get_font_size},
    // {(char *)"rk_osd_set_font_size", &ser_rk_osd_set_font_size},
    // {(char *)"rk_osd_get_boundary", &ser_rk_osd_get_boundary},
    // {(char *)"rk_osd_set_boundary", &ser_rk_osd_set_boundary},
    // {(char *)"rk_osd_get_normalized_screen_width", &ser_rk_osd_get_normalized_screen_width},
    // {(char *)"rk_osd_get_normalized_screen_height", &ser_rk_osd_get_normalized_screen_height},
    // {(char *)"rk_osd_get_attribute", &ser_rk_osd_get_attribute},
    // {(char *)"rk_osd_set_attribute", &ser_rk_osd_set_attribute},
    // {(char *)"rk_osd_get_font_color_mode", &ser_rk_osd_get_font_color_mode},
    // {(char *)"rk_osd_set_font_color_mode", &ser_rk_osd_set_font_color_mode},
    // {(char *)"rk_osd_get_font_color", &ser_rk_osd_get_font_color},
    // {(char *)"rk_osd_set_font_color", &ser_rk_osd_set_font_color},
    // {(char *)"rk_osd_get_alignment", &ser_rk_osd_get_alignment},
    // {(char *)"rk_osd_set_alignment", &ser_rk_osd_set_alignment},
    // {(char *)"rk_osd_get_font_path", &ser_rk_osd_get_font_path},
    // {(char *)"rk_osd_set_font_path", &ser_rk_osd_set_font_path},
    // // osd.x
    // {(char *)"rk_osd_get_enabled", &ser_rk_osd_get_enabled},
    // {(char *)"rk_osd_set_enabled", &ser_rk_osd_set_enabled},
    // {(char *)"rk_osd_get_position_x", &ser_rk_osd_get_position_x},
    // {(char *)"rk_osd_set_position_x", &ser_rk_osd_set_position_x},
    // {(char *)"rk_osd_get_position_y", &ser_rk_osd_get_position_y},
    // {(char *)"rk_osd_set_position_y", &ser_rk_osd_set_position_y},
    // {(char *)"rk_osd_get_height", &ser_rk_osd_get_height},
    // {(char *)"rk_osd_set_height", &ser_rk_osd_set_height},
    // {(char *)"rk_osd_get_width", &ser_rk_osd_get_width},
    // {(char *)"rk_osd_set_width", &ser_rk_osd_set_width},
    // {(char *)"rk_osd_get_display_week_enabled", &ser_rk_osd_get_display_week_enabled},
    // {(char *)"rk_osd_set_display_week_enabled", &ser_rk_osd_set_display_week_enabled},
    // {(char *)"rk_osd_get_date_style", &ser_rk_osd_get_date_style},
    // {(char *)"rk_osd_set_date_style", &ser_rk_osd_set_date_style},
    // {(char *)"rk_osd_get_time_style", &ser_rk_osd_get_time_style},
    // {(char *)"rk_osd_set_time_style", &ser_rk_osd_set_time_style},
    // {(char *)"rk_osd_get_type", &ser_rk_osd_get_type},
    // {(char *)"rk_osd_set_type", &ser_rk_osd_set_type},
    // {(char *)"rk_osd_get_display_text", &ser_rk_osd_get_display_text},
    // {(char *)"rk_osd_set_display_text", &ser_rk_osd_set_display_text},
    // {(char *)"rk_osd_get_image_path", &ser_rk_osd_get_image_path},
    // {(char *)"rk_osd_set_image_path", &ser_rk_osd_set_image_path},
    // {(char *)"rk_osd_get_style", &ser_rk_osd_get_style},
    // {(char *)"rk_osd_set_style", &ser_rk_osd_set_style},
    // {(char *)"rk_osd_restart", &ser_rk_osd_restart},
	// {(char *)"rk_osd_set_font_color_of_channelName", &ser_rk_osd_set_font_color_of_channelName},
	// {(char *)"rk_osd_set_font_color_of_dateTime", &ser_rk_osd_set_font_color_of_dateTime},
	// {(char *)"rk_osd_set_font_color_of_channelLocation", &ser_rk_osd_set_font_color_of_channelLocation},
	// {(char *)"rk_osd_get_font_color_of_channelName", &ser_rk_osd_get_font_color_of_channelName},
	// {(char *)"rk_osd_get_font_color_of_dateTime", &ser_rk_osd_get_font_color_of_dateTime},
	// {(char *)"rk_osd_get_font_color_of_channelLocation", &ser_rk_osd_get_font_color_of_channelLocation},
	// // roi.x
    // // {(char *)"rk_roi_get_stream_type", &ser_rk_roi_get_stream_type},
    // // {(char *)"rk_roi_set_stream_type", &ser_rk_roi_set_stream_type},
    // // {(char *)"rk_roi_get_name", &ser_rk_roi_get_name},
    // // {(char *)"rk_roi_set_name", &ser_rk_roi_set_name},
    // // {(char *)"rk_roi_get_id", &ser_rk_roi_get_id},
    // // {(char *)"rk_roi_set_id", &ser_rk_roi_set_id},
    // // {(char *)"rk_roi_get_enabled", &ser_rk_roi_get_enabled},
    // // {(char *)"rk_roi_set_enabled", &ser_rk_roi_set_enabled},
    // // {(char *)"rk_roi_get_position_x", &ser_rk_roi_get_position_x},
    // // {(char *)"rk_roi_set_position_x", &ser_rk_roi_set_position_x},
    // // {(char *)"rk_roi_get_position_y", &ser_rk_roi_get_position_y},
    // // {(char *)"rk_roi_set_position_y", &ser_rk_roi_set_position_y},
    // // {(char *)"rk_roi_get_height", &ser_rk_roi_get_height},
    // // {(char *)"rk_roi_set_height", &ser_rk_roi_set_height},
    // // {(char *)"rk_roi_get_width", &ser_rk_roi_get_width},
    // // {(char *)"rk_roi_set_width", &ser_rk_roi_set_width},
    // // {(char *)"rk_roi_get_quality_level", &ser_rk_roi_get_quality_level},
    // // {(char *)"rk_roi_set_quality_level", &ser_rk_roi_set_quality_level},
    // // {(char *)"rk_roi_set_all", &ser_rk_roi_set_all},
    // // // region_clip.x
    // // {(char *)"rk_region_clip_get_enabled", &ser_rk_region_clip_get_enabled},
    // // {(char *)"rk_region_clip_set_enabled", &ser_rk_region_clip_set_enabled},
    // // {(char *)"rk_region_clip_get_position_x", &ser_rk_region_clip_get_position_x},
    // // {(char *)"rk_region_clip_set_position_x", &ser_rk_region_clip_set_position_x},
    // // {(char *)"rk_region_clip_get_position_y", &ser_rk_region_clip_get_position_y},
    // // {(char *)"rk_region_clip_set_position_y", &ser_rk_region_clip_set_position_y},
    // // {(char *)"rk_region_clip_get_height", &ser_rk_region_clip_get_height},
    // // {(char *)"rk_region_clip_set_height", &ser_rk_region_clip_set_height},
    // // {(char *)"rk_region_clip_get_width", &ser_rk_region_clip_get_width},
    // // {(char *)"rk_region_clip_set_width", &ser_rk_region_clip_set_width},
    // // {(char *)"rk_region_clip_set_all", &ser_rk_region_clip_set_all},
    // network

	{(char *)"rk_network_get_dns_method", &ser_rk_network_get_dns_method},
	{(char *)"rk_network_set_dns_method", &ser_rk_network_set_dns_method},

    {(char *)"rk_network_ipv4_get", &ser_rk_network_ipv4_get},
    {(char *)"rk_network_ipv4_set", &ser_rk_network_ipv4_set},
    {(char *)"rk_network_dns_get", &ser_rk_network_dns_get},
    {(char *)"rk_network_dns_set", &ser_rk_network_dns_set},
    {(char *)"rk_network_get_mac", &ser_rk_network_get_mac},

	{(char *)"rk_network_get_http_port", &ser_rk_network_get_http_port},
	{(char *)"rk_network_set_http_port", &ser_rk_network_set_http_port},

	{(char *)"rk_network_get_https_port", &ser_rk_network_get_https_port},
	{(char *)"rk_network_set_https_port", &ser_rk_network_set_https_port},

	{(char *)"rk_network_get_rtsp_port", &ser_rk_network_get_rtsp_port},
	{(char *)"rk_network_set_rtsp_port", &ser_rk_network_set_rtsp_port},

	{(char *)"rk_network_get_onvif_port", &ser_rk_network_get_onvif_port},
	{(char *)"rk_network_set_onvif_port", &ser_rk_network_set_onvif_port},
	{(char *)"rk_network_smtp_get", &ser_rk_network_smtp_get},
	{(char *)"rk_network_smtp_set", &ser_rk_network_smtp_set},

	//  {(char *)"rk_network_ports_get", &ser_rk_network_ports_get},

	{(char *)"rk_network_get_method", &ser_rk_network_get_method},
	{(char *)"rk_network_set_method", &ser_rk_network_set_method},
	{(char *)"rk_network_set_ipv4_address", &ser_rk_network_set_ipv4_address},
	// {(char *)"rk_network_set_ipv4_netmask", &ser_rk_network_set_ipv4_netmask},
	// {(char *)"rk_network_set_ipv4_gateway", &ser_rk_network_set_ipv4_gateway},


    // {(char *)"rk_network_nicspeed_get", &ser_rk_network_nicspeed_get},
    // {(char *)"rk_network_nicspeed_set", &ser_rk_network_nicspeed_set},
    // {(char *)"rk_network_nicspeed_support_get", &ser_rk_network_nicspeed_support_get},
    // {(char *)"rk_wifi_power_get", &ser_rk_wifi_power_get},
    // {(char *)"rk_wifi_power_set", &ser_rk_wifi_power_set},
    // {(char *)"rk_wifi_scan_wifi", &ser_rk_wifi_scan_wifi},
    // {(char *)"rk_wifi_get_list", &ser_rk_wifi_get_list},
    // {(char *)"rk_wifi_connect_with_ssid", &ser_rk_wifi_connect_with_ssid},
    // {(char *)"rk_wifi_forget_with_ssid", &ser_rk_wifi_forget_with_ssid},
	//time
	{(char *)"rk_network_get_timezone", &ser_rk_network_get_timezone},
	{(char *)"rk_network_set_timezone", &ser_rk_network_set_timezone},
	{(char *)"rk_network_get_ntp_enable", &ser_rk_network_get_ntp_enable},
	{(char *)"rk_network_set_ntp_enable", &ser_rk_network_set_ntp_enable},
	{(char *)"rk_network_get_ntp_refresh_time_sec", &ser_rk_network_get_ntp_refresh_time_sec},
	{(char *)"rk_network_set_ntp_refresh_time_sec", &ser_rk_network_set_ntp_refresh_time_sec},
	{(char *)"rk_network_get_ntp_server", &ser_rk_network_get_ntp_server},
	{(char *)"rk_network_set_ntp_server", &ser_rk_network_set_ntp_server},
	{(char *)"rk_network_get_ntp_port", &ser_rk_network_get_ntp_port},
	{(char *)"rk_network_set_ntp_port", &ser_rk_network_set_ntp_port},
	{(char *)"rk_network_get_onvif_timesync_flag", &ser_rk_network_get_onvif_timesync_flag},
	{(char *)"rk_network_set_onvif_timesync_flag", &ser_rk_network_set_onvif_timesync_flag},
    // // storage
    // {(char *)"rk_storage_record_start", &ser_rk_storage_record_start},
    // {(char *)"rk_storage_record_stop", &ser_rk_storage_record_stop},
    // {(char *)"rk_storage_record_statue_get", &ser_rk_storage_record_statue_get},
	// {(char *)"rk_storage_get_schedule_plan", &ser_rk_storage_get_schedule_plan},
	// {(char *)"rk_storage_set_schedule_plan", &ser_rk_storage_set_schedule_plan},
	// {(char *)"rk_storage_get_recordings", &ser_rk_storage_get_recordings},
	// {(char *)"rk_storage_get_sdcard_size", &ser_rk_storage_get_sdcard_size},
	// {(char *)"rk_storage_get_recording_channel", &ser_rk_storage_get_recording_channel},
	// {(char *)"rk_storage_set_recording_channel", &ser_rk_storage_set_recording_channel},
	// {(char *)"rk_storage_get_recording_type", &ser_rk_storage_get_recording_type},
	// {(char *)"rk_storage_set_recording_type", ser_rk_storage_set_recording_type},
	// {(char *)"rk_storage_get_FIFO_status", &ser_rk_storage_get_FIFO_status},
	// {(char *)"rk_storage_set_FIFO_status", &ser_rk_storage_set_FIFO_status},
	// {(char *)"rk_storage_set_free_size_mb", &ser_rk_storage_set_free_size_mb},
	// {(char *)"rk_storage_format_sd_card", &ser_rk_storage_format_sd_card},
	//  // {(char *"rk_take_photo", &ser_rk_take_photo},
    // // event
    // // {(char *)"rk_event_ri_get_enabled", &ser_rk_event_ri_get_enabled},
    // // {(char *)"rk_event_ri_set_enabled", &ser_rk_event_ri_set_enabled},
    // // {(char *)"rk_event_ri_get_position_x", &ser_rk_event_ri_get_position_x},
    // // {(char *)"rk_event_ri_set_position_x", &ser_rk_event_ri_set_position_x},
    // // {(char *)"rk_event_ri_get_position_y", &ser_rk_event_ri_get_position_y},
    // // {(char *)"rk_event_ri_set_position_y", &ser_rk_event_ri_set_position_y},
    // // {(char *)"rk_event_ri_get_width", &ser_rk_event_ri_get_width},
    // // {(char *)"rk_event_ri_set_width", &ser_rk_event_ri_set_width},
    // // {(char *)"rk_event_ri_get_height", &ser_rk_event_ri_get_height},
    // // {(char *)"rk_event_ri_set_height", &ser_rk_event_ri_set_height},
    // // {(char *)"rk_event_ri_get_proportion", &ser_rk_event_ri_get_proportion},
    // // {(char *)"rk_event_ri_set_proportion", &ser_rk_event_ri_set_proportion},
    // // {(char *)"rk_event_ri_get_sensitivity_level", &ser_rk_event_ri_get_sensitivity_level},
    // // {(char *)"rk_event_ri_set_sensitivity_level", &ser_rk_event_ri_set_sensitivity_level},
    // // {(char *)"rk_event_ri_get_time_threshold", &ser_rk_event_ri_get_time_threshold},	
    // // {(char *)"rk_event_ri_set_time_threshold", &ser_rk_event_ri_set_time_threshold},
    // // system
    // {(char *)"rk_system_capability_get_video", &ser_rk_system_capability_get_video},
    // {(char *)"rk_system_capability_get_image_adjustment",
    //  &ser_rk_system_capability_get_image_adjustment},
    // {(char *)"rk_system_capability_get_image_blc", &ser_rk_system_capability_get_image_blc},
    // {(char *)"rk_system_capability_get_image_enhancement",
    //  &ser_rk_system_capability_get_image_enhancement},
    // {(char *)"rk_system_capability_get_image_exposure",
    //  &ser_rk_system_capability_get_image_exposure},
    // {(char *)"rk_system_capability_get_image_night_to_day",
    //  &ser_rk_system_capability_get_image_night_to_day},
    // {(char *)"rk_system_capability_get_image_video_adjustment",
    //  &ser_rk_system_capability_get_image_video_adjustment},
    // {(char *)"rk_system_capability_get_image_white_blance",
    //  &ser_rk_system_capability_get_image_white_blance},
    // {(char *)"rk_system_get_deivce_name", &ser_rk_system_get_deivce_name},
    // {(char *)"rk_system_get_telecontrol_id", &ser_rk_system_get_telecontrol_id},
    // {(char *)"rk_system_get_model", &ser_rk_system_get_model},
    // {(char *)"rk_system_get_serial_number", &ser_rk_system_get_serial_number},
    // {(char *)"rk_system_get_firmware_version", &ser_rk_system_get_firmware_version},
    // {(char *)"rk_system_get_encoder_version", &ser_rk_system_get_encoder_version},
    // {(char *)"rk_system_get_web_version", &ser_rk_system_get_web_version},
    // {(char *)"rk_system_get_plugin_version", &ser_rk_system_get_plugin_version},
    // {(char *)"rk_system_get_channels_number", &ser_rk_system_get_channels_number},
    // {(char *)"rk_system_get_hard_disks_number", &ser_rk_system_get_hard_disks_number},
    // {(char *)"rk_system_get_alarm_inputs_number", &ser_rk_system_get_alarm_inputs_number},
    // {(char *)"rk_system_get_alarm_outputs_number", &ser_rk_system_get_alarm_outputs_number},
    // {(char *)"rk_system_get_firmware_version_info", &ser_rk_system_get_firmware_version_info},
    // {(char *)"rk_system_get_manufacturer", &ser_rk_system_get_manufacturer},
    // {(char *)"rk_system_get_hardware_id", &ser_rk_system_get_hardware_id},
    // {(char *)"rk_system_set_deivce_name", &ser_rk_system_set_deivce_name},
    // {(char *)"rk_system_set_telecontrol_id", &ser_rk_system_set_telecontrol_id},
    // {(char *)"rk_system_reboot", &ser_rk_system_reboot},
    // {(char *)"rk_system_factory_reset", &ser_rk_system_factory_reset},
    // {(char *)"rk_system_export_log", &ser_rk_system_export_log},
    // {(char *)"rk_system_export_db", &ser_rk_system_export_db},
    // {(char *)"rk_system_import_db", &ser_rk_system_import_db},
    // {(char *)"rk_system_upgrade", &ser_rk_system_upgrade},
    // {(char *)"rk_system_get_user_details", &ser_rk_system_get_user_details},
	// {(char *)"rk_system_check_user_exists", &ser_rk_system_check_user_exists},
	// {(char *)"rk_system_verify_security_ans", &ser_rk_system_verify_security_ans},
	// {(char *)"rk_system_get_user_num", &ser_rk_system_get_user_num},
    // {(char *)"rk_system_manage_user_record", &ser_rk_system_manage_user_record},
	//  {(char *)"rk_system_record_exists", &ser_rk_system_record_exists},
	// {(char *)"rk_analytics_find_records",&ser_rk_analytics_find_records},
	// {(char *)"rk_count_db_records",&ser_rk_count_db_records},
	// {(char *)"rk_update_analytics_module",&ser_rk_update_analytics_module},
	// {(char *)"rk_start_analytics_module",&ser_rk_start_analytics_module},
	// {(char *)"rk_stop_analytics_module",&ser_rk_stop_analytics_module},
	// {(char *)"rk_update_camera_tamper_module",&ser_rk_update_camera_tamper_module},
	// {(char *)"rk_start_camera_tamper_module",&ser_rk_start_camera_tamper_module},
	// {(char *)"rk_stop_camera_tamper_module",&ser_rk_stop_camera_tamper_module},
	// {(char *)"rk_analytics_update_db",&ser_rk_analytics_update_db}
};

static void *rec_thread(void *arg) {
	int fd = (int)(intptr_t)arg;
	char *name = NULL;
	int len;
	int ret = 0;
	int i;
	int maplen = sizeof(map) / sizeof(struct FunMap);
	pthread_detach(pthread_self());

	if (sock_write(fd, &ret, sizeof(int)) == SOCKERR_CLOSED) {
		ret = -1;
		goto out;
	}
again:
	if (sock_read(fd, &len, sizeof(int)) == SOCKERR_CLOSED) {
		ret = -1;
		goto out;
	}
	if (len <= 0) {
		ret = -1;
		goto out;
	}
	name = (char *)malloc(len);
	if (sock_read(fd, name, len) == SOCKERR_CLOSED) {
		ret = -1;
		goto out;
	}

	for (i = 0; i < maplen; i++) {
		// printf("%s, %s\n", map[i].fun_name, name);
		if (!strcmp(map[i].fun_name, name)) {
			ret = map[i].fun(fd);
		}
	}
out:
	if (name)
		free(name);
	name = NULL;
	if (ret == 0) {
		sock_write(fd, &ret, sizeof(int));
		goto again;
	}

	close(fd);
	pthread_exit(NULL);

	return 0;
}

static void *rkipc_server_thread(void *arg) {
	int clifd;
	LOG_INFO("#Start %s thread, arg:%p\n", __func__, arg);
	prctl(PR_SET_NAME, "rkipc_server_thread", 0, 0, 0);
	pthread_detach(pthread_self());

	if ((listen_fd = serv_listen(CS_PATH)) < 0)
		LOG_ERROR("listen fail\n");

	while (RkIpcServerRun) {
		pthread_t thread_id;

		if ((clifd = serv_accept(listen_fd)) < 0) {
			LOG_ERROR("accept fail\n");
		}

		if (clifd >= 0)
			pthread_create(&thread_id, NULL, rec_thread, (void *)(intptr_t)clifd);
	}
	RkIpcServerTid = 0;
	pthread_exit(NULL);
	return 0;
}

void handle_pipe1(int sig) { LOG_INFO("%s sig = %d\n", __func__, sig); }

int rkipc_server_init(void) {
	struct sigaction action;
	action.sa_handler = handle_pipe1;
	sigemptyset(&action.sa_mask);
	action.sa_flags = 0;
	sigaction(SIGPIPE, &action, NULL);

	RkIpcServerRun = 1;
	if (RkIpcServerTid == 0)
		pthread_create(&RkIpcServerTid, NULL, rkipc_server_thread, NULL);
	return 0;
}

int rkipc_server_deinit(void) {
	RkIpcServerRun = 0;
	usleep(500 * 1000);
	if (RkIpcServerTid == 0)
		LOG_INFO("rkipc_server_deinit success\n");
	else
		LOG_ERROR("rkipc_server_deinit failed\n");

	return 0;
}
