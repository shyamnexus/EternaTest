#pragma once

#include <stdio.h>
#define COLOR_NONE "\e[0m"
#define COLOR_NORMAL "\e[1m"
#define COLOR_RED "\e[31m"
#define COLOR_GREEN "\e[32m"
#define COLOR_YELLOW "\e[33m"

#define LOG(format, ...) printf("[%s: %d] " format "\r\n", __FUNCTION__,__LINE__, ##__VA_ARGS__)
#define LOGE(format, ...) printf(COLOR_RED    "[ERROR  ]" COLOR_NONE "[%s: %d] " format "\r\n", __FUNCTION__,__LINE__, ##__VA_ARGS__)
#define LOGW(format, ...) printf(COLOR_YELLOW "[WARNING]" COLOR_NONE "[%s: %d] " format "\r\n", __FUNCTION__,__LINE__, ##__VA_ARGS__)
#define LOGI(format, ...) printf(COLOR_GREEN  "[INFO   ]" COLOR_NONE "[%s: %d] " format "\r\n", __FUNCTION__,__LINE__, ##__VA_ARGS__)
#define FUNC_IN() LOG("+")
#define FUNC_OUT() LOG("-")