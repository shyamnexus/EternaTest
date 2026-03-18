/**
 * @file hdal_pipeline_common.h
 * @brief Common definitions and includes for HDAL pipeline modules
 */

#ifndef HDAL_PIPELINE_COMMON_H
#define HDAL_PIPELINE_COMMON_H

#include "ipcam/hdal_pipeline.h"
#include "ipcam/config.h"
#include <spdlog/spdlog.h>
#include <mutex>
#include <cstring>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>

#if HDAL_PIPELINE_ENABLED
extern "C" {
#include "hdal.h"
#include "hd_debug.h"
#include "vendor_isp.h"
#include "vendor_videocapture.h"
#include "vendor_videoprocess.h"
}
#endif

namespace ipcam {
namespace platform {

// Shared mutex for pipeline operations
extern std::mutex g_pipeline_mutex;

// ============================================================================
// Buffer Size Calculation Helpers (from pq_video_rtsp.c)
// ============================================================================

#if HDAL_PIPELINE_ENABLED
// Debug info buffer
constexpr int DBGINFO_BUFSIZE = 0x200;

// RAW buffer size calculation
inline int VDO_RAW_BUFSIZE(int w, int h, int bpp) {
    return ((((w) * (bpp) / 8 + 3) / 4) * 4) * (h);
}

// YUV420 buffer size
inline int VDO_YUV_BUFSIZE(int w, int h) {
    return ((((w) * 12 / 8 + 3) / 4) * 4) * (h);  // YUV420 = 12 bpp
}

// CA buffer for AWB
inline int VDO_CA_BUF_SIZE(int win_w, int win_h) {
    return (((win_w * win_h * 8) * 2 + 3) / 4) * 4;
}

// LA buffer for AE
inline int VDO_LA_BUF_SIZE(int win_w, int win_h) {
    return (((win_w * win_h * 2) * 2 + 3) / 4) * 4;
}

// VA buffer for AF
inline int VDO_VA_BUF_SIZE(int win_w, int win_h) {
    return (((win_w * win_h * 4) + 3) / 4) * 4;
}

// ISP window numbers - use macros from isp_api.h if available
#ifndef ISP_CA_W_WINNUM
constexpr int HDAL_ISP_CA_W_WINNUM = 32;
constexpr int HDAL_ISP_CA_H_WINNUM = 32;
constexpr int HDAL_ISP_LA_W_WINNUM = 32;
constexpr int HDAL_ISP_LA_H_WINNUM = 32;
constexpr int HDAL_ISP_VA_W_WINNUM = 16;
constexpr int HDAL_ISP_VA_H_WINNUM = 16;
#else
// Use the SDK-defined macros
constexpr int HDAL_ISP_CA_W_WINNUM = ISP_CA_W_WINNUM;
constexpr int HDAL_ISP_CA_H_WINNUM = ISP_CA_H_WINNUM;
constexpr int HDAL_ISP_LA_W_WINNUM = ISP_LA_W_WINNUM;
constexpr int HDAL_ISP_LA_H_WINNUM = ISP_LA_H_WINNUM;
constexpr int HDAL_ISP_VA_W_WINNUM = ISP_VA_W_WINNUM;
constexpr int HDAL_ISP_VA_H_WINNUM = ISP_VA_H_WINNUM;
#endif
#endif

} // namespace platform
} // namespace ipcam

#endif // HDAL_PIPELINE_COMMON_H
