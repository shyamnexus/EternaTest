/**
 * @file hdal_pipeline_init.cpp
 * @brief HDAL Pipeline Initialization (ISP, Memory, Modules)
 */

#include "hdal_pipeline_common.h"

namespace ipcam {
namespace platform {

// ============================================================================
// Vendor ISP Initialization (from pq_video_rtsp.c)
// ============================================================================

bool HdalPipeline::InitVendorIsp() {
#if HDAL_PIPELINE_ENABLED
    spdlog::info("Initializing vendor ISP...");
    
    HD_RESULT ret = vendor_isp_init();
    if (ret != HD_OK) {
        spdlog::error("vendor_isp_init failed: {}", static_cast<int>(ret));
        return false;
    }
    
    const char* ver = vendor_isp_get_ver();
    spdlog::info("Vendor ISP initialized, version: {}", ver ? ver : "unknown");
    
    return true;
#else
    spdlog::warn("HDAL_PIPELINE_ENABLED not defined, skipping vendor ISP init");
    return true;
#endif
}

bool HdalPipeline::LoadIspTuningConfig() {
#if HDAL_PIPELINE_ENABLED
    spdlog::info("Loading ISP tuning config: {}/{}.cfg", 
                 config_.isp_tuning.config_path, 
                 config_.isp_tuning.config_name);
    
    // Build config file path
    char cfg_path[256];
    snprintf(cfg_path, sizeof(cfg_path), "%s/%s.cfg",
             config_.isp_tuning.config_path.c_str(),
             config_.isp_tuning.config_name.c_str());
    
    // Load AE config
    AET_CFG_INFO ae_cfg;
    memset(&ae_cfg, 0, sizeof(ae_cfg));
    ae_cfg.id = static_cast<AE_ID>(config_.sensor.vcap_id);
    strncpy(ae_cfg.path, cfg_path, sizeof(ae_cfg.path) - 1);
    
    HD_RESULT ret = vendor_isp_set_ae(AET_ITEM_RLD_CONFIG, &ae_cfg);
    if (ret != HD_OK) {
        spdlog::warn("Failed to load AE config: {}", static_cast<int>(ret));
    }
    
    // Load AWB config
    AWBT_CFG_INFO awb_cfg;
    memset(&awb_cfg, 0, sizeof(awb_cfg));
    awb_cfg.id = static_cast<AWB_ID>(config_.sensor.vcap_id);
    strncpy(awb_cfg.path, cfg_path, sizeof(awb_cfg.path) - 1);
    
    ret = vendor_isp_set_awb(AWBT_ITEM_RLD_CONFIG, &awb_cfg);
    if (ret != HD_OK) {
        spdlog::warn("Failed to load AWB config: {}", static_cast<int>(ret));
    }
    
    // Load IQ config
    IQT_CFG_INFO iq_cfg;
    memset(&iq_cfg, 0, sizeof(iq_cfg));
    iq_cfg.id = static_cast<IQ_ID>(config_.sensor.vcap_id);
    strncpy(iq_cfg.path, cfg_path, sizeof(iq_cfg.path) - 1);
    
    ret = vendor_isp_set_iq(IQT_ITEM_RLD_CONFIG, &iq_cfg);
    if (ret != HD_OK) {
        spdlog::warn("Failed to load IQ config: {}", static_cast<int>(ret));
    }
    
    // Set initial color temperature
    ISPT_CT ct;
    memset(&ct, 0, sizeof(ct));
    ct.id = static_cast<UINT32>(config_.sensor.vcap_id);
    ct.ct = config_.isp_tuning.color_temperature;
    vendor_isp_set_common(ISPT_ITEM_CT, &ct);
    
    // Get color gain from color temperature and apply
    AWBT_CT_TO_CGAIN ct_to_cgain;
    memset(&ct_to_cgain, 0, sizeof(ct_to_cgain));
    ct_to_cgain.id = static_cast<AWB_ID>(config_.sensor.vcap_id);
    ct_to_cgain.ct_to_cgain.ct = config_.isp_tuning.color_temperature;
    
    ret = vendor_isp_get_awb(AWBT_ITEM_CT_TO_CGAIN, &ct_to_cgain);
    if (ret == HD_OK) {
        ISPT_C_GAIN c_gain;
        memset(&c_gain, 0, sizeof(c_gain));
        c_gain.id = static_cast<UINT32>(config_.sensor.vcap_id);
        c_gain.gain[0] = ct_to_cgain.ct_to_cgain.r_gain;
        c_gain.gain[1] = ct_to_cgain.ct_to_cgain.g_gain;
        c_gain.gain[2] = ct_to_cgain.ct_to_cgain.b_gain;
        vendor_isp_set_common(ISPT_ITEM_C_GAIN, &c_gain);
    }
    
    spdlog::info("ISP tuning config loaded successfully");
    return true;
#else
    return true;
#endif
}

void HdalPipeline::UninitVendorIsp() {
#if HDAL_PIPELINE_ENABLED
    spdlog::info("Uninitializing vendor ISP...");
    vendor_isp_uninit();
#endif
}

// ============================================================================
// HDAL Common Initialization
// ============================================================================

bool HdalPipeline::InitHdalCommon() {
#if HDAL_PIPELINE_ENABLED
    spdlog::info("Initializing HDAL common...");
    
    // Kill any stale nvtrtspd_ipc processes from previous crashed instances
    // This must be done BEFORE hd_common_init to avoid "client is still alive" error
    int ret_sys = system("pkill -9 nvtrtspd_ipc 2>/dev/null");
    (void)ret_sys;  // Ignore - process may not exist
    
    // Try to force cleanup any lingering HDAL state from previous crashed instances
    // This helps recover from "client is still alive" errors
    HD_RESULT ret = hd_common_init(0);
    if (ret != HD_OK) {
        spdlog::warn("hd_common_init failed ({}), attempting cleanup and retry...", static_cast<int>(ret));
        
        // Try to uninit in case previous instance left state behind
        hd_common_uninit();
        
        // Small delay to let kernel driver reset
        usleep(100000);  // 100ms
        
        // Retry initialization
        ret = hd_common_init(0);
        if (ret != HD_OK) {
            spdlog::error("hd_common_init failed after cleanup: {}", static_cast<int>(ret));
            return false;
        }
    }
    
    spdlog::info("HDAL common initialized");
    return true;
#else
    return true;
#endif
}

void HdalPipeline::UninitHdalCommon() {
#if HDAL_PIPELINE_ENABLED
    spdlog::info("Uninitializing HDAL common...");
    hd_common_uninit();
#endif
}

// ============================================================================
// Memory Pool Initialization (from pq_video_rtsp.c)
// ============================================================================

bool HdalPipeline::InitMemoryPools() {
#if HDAL_PIPELINE_ENABLED
    spdlog::info("Initializing HDAL memory pools...");
    
    HD_COMMON_MEM_INIT_CONFIG mem_cfg;
    memset(&mem_cfg, 0, sizeof(mem_cfg));
    int pool_num = 0;
    
    // Get main stream dimensions for buffer calculation
    int main_w = config_.sensor.crop.enabled ? config_.sensor.crop.width : config_.sensor.native_width;
    int main_h = config_.sensor.crop.enabled ? config_.sensor.crop.height : config_.sensor.native_height;
    
    // Pool 0: RAW buffers (capture)
    mem_cfg.pool_info[pool_num].type = HD_COMMON_MEM_COMMON_POOL;
    mem_cfg.pool_info[pool_num].blk_size = CalculateRawBufferSize(main_w, main_h, config_.sensor.raw_format);
    mem_cfg.pool_info[pool_num].blk_cnt = config_.memory.raw_buffer_count;
    mem_cfg.pool_info[pool_num].ddr_id = static_cast<HD_COMMON_MEM_DDR_ID>(config_.memory.ddr_id);
    spdlog::debug("Pool {}: RAW buffers, size={}, count={}", 
                  pool_num, mem_cfg.pool_info[pool_num].blk_size, mem_cfg.pool_info[pool_num].blk_cnt);
    pool_num++;
    
    // Pool 1: YUV buffers (main stream / IPP internal paths)
    // IMPORTANT: Always allocate at full sensor crop resolution (not current stream resolution).
    // The IPP pipeline (3DNR reference path, IME scaling paths) processes internally at the
    // full sensor output size (e.g. 2944x1664) and allocates YUV buffers from the common pool.
    // If stream 0 is configured smaller (e.g. 1280x960) the blocks must still be large enough
    // for the IPP, otherwise ctl_ipp_buf_alloc() fails with "ime adj fail".
    mem_cfg.pool_info[pool_num].type = HD_COMMON_MEM_COMMON_POOL;
    mem_cfg.pool_info[pool_num].blk_size = CalculateYuvBufferSize(main_w, main_h);
    mem_cfg.pool_info[pool_num].blk_cnt = config_.memory.yuv_buffer_count;
    mem_cfg.pool_info[pool_num].ddr_id = static_cast<HD_COMMON_MEM_DDR_ID>(config_.memory.ddr_id);
    spdlog::debug("Pool {}: Main YUV (sensor-sized), size={}, count={}", 
                  pool_num, mem_cfg.pool_info[pool_num].blk_size, mem_cfg.pool_info[pool_num].blk_cnt);
    pool_num++;
    
    // Pool 2: YUV buffers (sub stream)
    if (config_.video_streams[1].enabled) {
        mem_cfg.pool_info[pool_num].type = HD_COMMON_MEM_COMMON_POOL;
        mem_cfg.pool_info[pool_num].blk_size = CalculateYuvBufferSize(
            config_.video_streams[1].width, config_.video_streams[1].height);
        mem_cfg.pool_info[pool_num].blk_cnt = 3;
        mem_cfg.pool_info[pool_num].ddr_id = static_cast<HD_COMMON_MEM_DDR_ID>(config_.memory.ddr_id);
        spdlog::debug("Pool {}: Sub YUV, size={}, count={}", 
                      pool_num, mem_cfg.pool_info[pool_num].blk_size, mem_cfg.pool_info[pool_num].blk_cnt);
        pool_num++;
    }
    
    // Pool 3: YUV buffers (third stream)
    if (config_.video_streams[2].enabled) {
        mem_cfg.pool_info[pool_num].type = HD_COMMON_MEM_COMMON_POOL;
        mem_cfg.pool_info[pool_num].blk_size = CalculateYuvBufferSize(
            config_.video_streams[2].width, config_.video_streams[2].height);
        mem_cfg.pool_info[pool_num].blk_cnt = 3;
        mem_cfg.pool_info[pool_num].ddr_id = static_cast<HD_COMMON_MEM_DDR_ID>(config_.memory.ddr_id);
        spdlog::debug("Pool {}: Third YUV, size={}, count={}", 
                      pool_num, mem_cfg.pool_info[pool_num].blk_size, mem_cfg.pool_info[pool_num].blk_cnt);
        pool_num++;
    }
    
    // Pool 4: YUV buffers (fourth stream)
    if (config_.video_streams[3].enabled) {
        mem_cfg.pool_info[pool_num].type = HD_COMMON_MEM_COMMON_POOL;
        mem_cfg.pool_info[pool_num].blk_size = CalculateYuvBufferSize(
            config_.video_streams[3].width, config_.video_streams[3].height);
        mem_cfg.pool_info[pool_num].blk_cnt = 3;
        mem_cfg.pool_info[pool_num].ddr_id = static_cast<HD_COMMON_MEM_DDR_ID>(config_.memory.ddr_id);
        spdlog::debug("Pool {}: Fourth YUV, size={}, count={}", 
                      pool_num, mem_cfg.pool_info[pool_num].blk_size, mem_cfg.pool_info[pool_num].blk_cnt);
        pool_num++;
    }
    
    // Pool 5: Audio buffers
    if (config_.audio.enabled) {
        mem_cfg.pool_info[pool_num].type = HD_COMMON_MEM_COMMON_POOL;
        mem_cfg.pool_info[pool_num].blk_size = config_.memory.audio_buffer_size;
        mem_cfg.pool_info[pool_num].blk_cnt = 1;
        mem_cfg.pool_info[pool_num].ddr_id = static_cast<HD_COMMON_MEM_DDR_ID>(config_.memory.ddr_id);
        spdlog::debug("Pool {}: Audio, size={}, count={}", 
                      pool_num, mem_cfg.pool_info[pool_num].blk_size, mem_cfg.pool_info[pool_num].blk_cnt);
        pool_num++;
    }
    
    // Pool 6: OSG buffers for OSD stamp overlay (per-stream, ping-pong)
    // Stream 0 (2944x200 @ 16bpp * 2) needs ~2.25MB. Use 3MB blocks for full-width support.
    mem_cfg.pool_info[pool_num].type = HD_COMMON_MEM_OSG_POOL;
    mem_cfg.pool_info[pool_num].blk_size = 3072 * 1024; // 3MB per block for full-width 2944px
    mem_cfg.pool_info[pool_num].blk_cnt = 4;            // 4 blocks (enough for 3 streams + spare)
    mem_cfg.pool_info[pool_num].ddr_id = static_cast<HD_COMMON_MEM_DDR_ID>(config_.memory.ddr_id);
    spdlog::debug("Pool {}: OSG (OSD stamps), size={}, count={}", 
                  pool_num, mem_cfg.pool_info[pool_num].blk_size, mem_cfg.pool_info[pool_num].blk_cnt);
    pool_num++;
    
    // Pool 7: Snapshot JPEG buffers (for CaptureSnapshot)
    // Max size: 1920x1080 YUV420 / 5 (JPEG compression) = ~1.5MB, round up to 2MB for safety
    mem_cfg.pool_info[pool_num].type = HD_COMMON_MEM_USER_POOL_BEGIN;  // User pool for snapshot allocation
    mem_cfg.pool_info[pool_num].blk_size = 2 * 1024 * 1024; // 2MB per block
    mem_cfg.pool_info[pool_num].blk_cnt = 2;                 // 2 blocks (support concurrent snapshots)
    mem_cfg.pool_info[pool_num].ddr_id = static_cast<HD_COMMON_MEM_DDR_ID>(config_.memory.ddr_id);
    spdlog::debug("Pool {}: Snapshot JPEG, size={}, count={}", 
                  pool_num, mem_cfg.pool_info[pool_num].blk_size, mem_cfg.pool_info[pool_num].blk_cnt);
    pool_num++;
    
    HD_RESULT ret = hd_common_mem_init(&mem_cfg);
    if (ret != HD_OK) {
        spdlog::error("hd_common_mem_init failed: {}", static_cast<int>(ret));
        return false;
    }
    
    spdlog::info("HDAL memory pools initialized ({} pools)", pool_num);
    return true;
#else
    return true;
#endif
}

void HdalPipeline::UninitMemory() {
#if HDAL_PIPELINE_ENABLED
    spdlog::info("Uninitializing HDAL memory...");
    hd_common_mem_uninit();
#endif
}

// ============================================================================
// Module Initialization
// ============================================================================

bool HdalPipeline::InitModules() {
#if HDAL_PIPELINE_ENABLED
    spdlog::info("Initializing HDAL modules...");
    
    HD_RESULT ret;
    
    ret = hd_videocap_init();
    if (ret != HD_OK) {
        spdlog::error("hd_videocap_init failed: {}", static_cast<int>(ret));
        return false;
    }
    
    ret = hd_videoproc_init();
    if (ret != HD_OK) {
        spdlog::error("hd_videoproc_init failed: {}", static_cast<int>(ret));
        return false;
    }
    
    ret = hd_videoenc_init();
    if (ret != HD_OK) {
        spdlog::error("hd_videoenc_init failed: {}", static_cast<int>(ret));
        return false;
    }
    
    if (config_.audio.enabled) {
        ret = hd_audiocap_init();
        if (ret != HD_OK) {
            spdlog::error("hd_audiocap_init failed: {}", static_cast<int>(ret));
            return false;
        }
        
        ret = hd_audioenc_init();
        if (ret != HD_OK) {
            spdlog::error("hd_audioenc_init failed: {}", static_cast<int>(ret));
            return false;
        }
    }
    
    spdlog::info("HDAL modules initialized");
    return true;
#else
    return true;
#endif
}

void HdalPipeline::UninitModules() {
#if HDAL_PIPELINE_ENABLED
    spdlog::info("Uninitializing HDAL modules...");
    
    if (config_.audio.enabled) {
        hd_audioenc_uninit();
        hd_audiocap_uninit();
    }
    
    hd_videoenc_uninit();
    hd_videoproc_uninit();
    hd_videocap_uninit();
#endif
}

} // namespace platform
} // namespace ipcam
