/**
 * @file npu_inference.cpp
 * @brief NPU inference engine implementation using vendor AI3
 *
 * Wraps the vendor_ai3 API for model loading, inference, and
 * output retrieval. Used by AnalyticsEngine to run YOLO, SCRFD,
 * and other models on the CNN30 NPU.
 */
#include "ipcam/npu_inference.h"
#include <spdlog/spdlog.h>
#include <cstring>
#include <sys/stat.h>

#if VENDOR_AI3_ENABLED
extern "C" {
#include "hdal.h"
#include "hd_common.h"
#include "vendor_ai_cpu/vendor_ai_cpu.h"
}
#endif

namespace ipcam {
namespace ai {

NpuInference& NpuInference::Instance() {
    static NpuInference instance;
    return instance;
}

NpuInference::~NpuInference() {
    if (initialized_) {
        Shutdown();
    }
}

bool NpuInference::Init() {
    std::lock_guard<std::mutex> lock(mutex_);
    if (initialized_) return true;

#if VENDOR_AI3_ENABLED
    VENDOR_AI3_DEV_CFG dev_cfg;
    memset(&dev_cfg, 0, sizeof(dev_cfg));

    HD_RESULT ret = vendor_ai3_dev_init(&dev_cfg);
    if (ret != HD_OK) {
        spdlog::error("NpuInference: vendor_ai3_dev_init failed: {}", static_cast<int>(ret));
        return false;
    }

    VENDOR_AI3_VER ver;
    memset(&ver, 0, sizeof(ver));
    ret = vendor_ai3_dev_get(VENDOR_AI3_CFG_VER, &ver);
    if (ret == HD_OK) {
        spdlog::info("NpuInference: AI3 version lib={} kflow={} kdrv={}",
                     ver.vendor_ai_impl_version,
                     ver.kflow_ai_impl_version,
                     ver.kdrv_ai_impl_version);
    }

    initialized_ = true;
    spdlog::info("NpuInference: AI3 device initialized");
    return true;
#else
    spdlog::warn("NpuInference: VENDOR_AI3 not enabled at compile time");
    return false;
#endif
}

void NpuInference::Shutdown() {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!initialized_) return;

    // Unload all networks
    for (size_t i = 0; i < networks_.size(); i++) {
        if (networks_[i].loaded) {
            UnloadNetworkLocked(i);
        }
    }
    networks_.clear();

#if VENDOR_AI3_ENABLED
    vendor_ai3_dev_uninit();
#endif

    initialized_ = false;
    spdlog::info("NpuInference: shutdown complete");
}

int NpuInference::LoadNetwork(const std::string& name, const std::string& model_path) {
    std::lock_guard<std::mutex> lock(mutex_);

    if (!initialized_) {
        spdlog::error("NpuInference: not initialized, cannot load {}", name);
        return -1;
    }

#if VENDOR_AI3_ENABLED
    // Verify model file exists
    int32_t msz = FileSize(model_path);
    if (msz <= 0) {
        spdlog::error("NpuInference: model file not found: {}", model_path);
        return -1;
    }

    NpuNetwork net;
    net.name = name;
    net.model_path = model_path;

    // Allocate and load model buffer
    if (!AllocMem(net.model_buf, (name + "_mdl").c_str(), static_cast<uint32_t>(msz))) {
        spdlog::error("NpuInference: failed to allocate model buffer for {}", name);
        return -1;
    }

    if (!LoadFile(model_path, net.model_buf.va, static_cast<uint32_t>(msz))) {
        spdlog::error("NpuInference: failed to load model file {}", model_path);
        FreeMem(net.model_buf);
        return -1;
    }

    // Query memory requirements
    VENDOR_AI3_MODEL_INFO mi;
    memset(&mi, 0, sizeof(mi));
    mi.model_buf.pa   = net.model_buf.pa;
    mi.model_buf.va   = net.model_buf.va;
    mi.model_buf.size = net.model_buf.size;

    if (vendor_ai3_dev_get(VENDOR_AI3_CFG_MODEL_INFO, &mi) != HD_OK) {
        // Use defaults if query fails
        mi.proc_mem.buf[AI3_PROC_BUF_WORKBUF].size  = 20 * 1024 * 1024;
        mi.proc_mem.buf[AI3_PROC_BUF_RONLYBUF].size = 12 * 1024 * 1024;
        spdlog::warn("NpuInference: model info query failed for {}, using defaults", name);
    }

    // Allocate work and readonly buffers
    if (!AllocMem(net.work_buf, (name + "_wrk").c_str(),
                  mi.proc_mem.buf[AI3_PROC_BUF_WORKBUF].size)) {
        FreeMem(net.model_buf);
        return -1;
    }
    if (!AllocMem(net.ronly_buf, (name + "_ro").c_str(),
                  mi.proc_mem.buf[AI3_PROC_BUF_RONLYBUF].size)) {
        FreeMem(net.work_buf);
        FreeMem(net.model_buf);
        return -1;
    }

    // Open the network
    VENDOR_AI3_PROC_CFG pc;
    memset(&pc, 0, sizeof(pc));
    pc.model_buf.pa   = net.model_buf.pa;
    pc.model_buf.va   = net.model_buf.va;
    pc.model_buf.size = net.model_buf.size;
    pc.proc_mem.buf[AI3_PROC_BUF_WORKBUF].pa    = net.work_buf.pa;
    pc.proc_mem.buf[AI3_PROC_BUF_WORKBUF].va    = net.work_buf.va;
    pc.proc_mem.buf[AI3_PROC_BUF_WORKBUF].size  = net.work_buf.size;
    pc.proc_mem.buf[AI3_PROC_BUF_RONLYBUF].pa   = net.ronly_buf.pa;
    pc.proc_mem.buf[AI3_PROC_BUF_RONLYBUF].va   = net.ronly_buf.va;
    pc.proc_mem.buf[AI3_PROC_BUF_RONLYBUF].size = net.ronly_buf.size;
    pc.plugin[AI3_PLUGIN_CPU] = vendor_ai_cpu1_get_engine();

    HD_RESULT ret = vendor_ai3_net_open(&net.proc_id, &pc, &net.info);
    if (ret != HD_OK) {
        spdlog::error("NpuInference: net_open failed for {}: {}", name, static_cast<int>(ret));
        FreeMem(net.ronly_buf);
        FreeMem(net.work_buf);
        FreeMem(net.model_buf);
        return -1;
    }

    ret = vendor_ai3_net_start(net.proc_id);
    if (ret != HD_OK) {
        spdlog::error("NpuInference: net_start failed for {}: {}", name, static_cast<int>(ret));
        vendor_ai3_net_close(net.proc_id);
        FreeMem(net.ronly_buf);
        FreeMem(net.work_buf);
        FreeMem(net.model_buf);
        return -1;
    }

    // Allocate output buffers
    net.num_outputs = net.info.out_buf_cnt;
    net.out_bufs.resize(net.num_outputs);

    for (uint32_t i = 0; i < net.num_outputs; i++) {
        VENDOR_AI3_BUF ab;
        memset(&ab, 0, sizeof(ab));
        vendor_ai3_net_get(net.proc_id, static_cast<VENDOR_AI3_NET_PARAM_ID>(net.info.out_path_list[i]), &ab);

        spdlog::debug("NpuInference: {} out[{}] {}x{}x{} fmt=0x{:x} size={} scale={} zp={}",
                      name, i, ab.width, ab.height, ab.channel,
                      static_cast<uint32_t>(ab.fmt), ab.size, ab.scale_ratio, ab.zero_point);

        char buf_name[32];
        snprintf(buf_name, sizeof(buf_name), "%s_o%u", name.c_str(), i);
        if (!AllocMem(net.out_bufs[i], buf_name, ab.size)) {
            spdlog::error("NpuInference: failed to allocate output buffer {} for {}", i, name);
            // Cleanup already allocated outputs
            for (uint32_t j = 0; j < i; j++) FreeMem(net.out_bufs[j]);
            vendor_ai3_net_stop(net.proc_id);
            vendor_ai3_net_close(net.proc_id);
            FreeMem(net.ronly_buf);
            FreeMem(net.work_buf);
            FreeMem(net.model_buf);
            return -1;
        }

        ab.pa   = net.out_bufs[i].pa;
        ab.va   = net.out_bufs[i].va;
        ab.size = net.out_bufs[i].size;
        vendor_ai3_net_set(net.proc_id, static_cast<VENDOR_AI3_NET_PARAM_ID>(net.info.out_path_list[i]), &ab);
    }

    net.loaded = true;
    int net_id = static_cast<int>(networks_.size());
    networks_.push_back(std::move(net));

    spdlog::info("NpuInference: loaded {} ({}) -> id={}, outputs={}",
                 name, model_path, net_id, networks_[net_id].num_outputs);
    return net_id;
#else
    (void)name; (void)model_path;
    return -1;
#endif
}

void NpuInference::UnloadNetwork(int net_id) {
    std::lock_guard<std::mutex> lock(mutex_);
    UnloadNetworkLocked(net_id);
}

void NpuInference::UnloadNetworkLocked(int net_id) {
    if (net_id < 0 || net_id >= static_cast<int>(networks_.size())) return;
    auto& net = networks_[net_id];
    if (!net.loaded) return;

#if VENDOR_AI3_ENABLED
    if (net.proc_id) {
        vendor_ai3_net_stop(net.proc_id);
        vendor_ai3_net_close(net.proc_id);
        net.proc_id = 0;
    }
    for (auto& ob : net.out_bufs) FreeMem(ob);
    net.out_bufs.clear();
    FreeMem(net.ronly_buf);
    FreeMem(net.work_buf);
    FreeMem(net.model_buf);
#endif

    spdlog::info("NpuInference: unloaded {} (id={})", net.name, net_id);
    net.loaded = false;
}

bool NpuInference::Infer(int net_id, uintptr_t phy_addr, uintptr_t va,
                         uint32_t width, uint32_t height, uint32_t line_ofs) {
    std::lock_guard<std::mutex> lock(mutex_);

    if (net_id < 0 || net_id >= static_cast<int>(networks_.size())) return false;
    auto& net = networks_[net_id];
    if (!net.loaded) return false;

#if VENDOR_AI3_ENABLED
    uint32_t in_size = line_ofs * height * 3 / 2;

    // Set input buffer
    VENDOR_AI3_BUF ib;
    memset(&ib, 0, sizeof(ib));
    vendor_ai3_net_get(net.proc_id, static_cast<VENDOR_AI3_NET_PARAM_ID>(net.info.in_path_list[0]), &ib);
    ib.pa       = phy_addr;
    ib.va       = va;
    ib.width    = width;
    ib.height   = height;
    ib.line_ofs = line_ofs;
    ib.size     = in_size;
    vendor_ai3_net_set(net.proc_id, static_cast<VENDOR_AI3_NET_PARAM_ID>(net.info.in_path_list[0]), &ib);

    // Flush input
    if (va) {
        hd_common_mem_cache_sync(reinterpret_cast<void*>(va), in_size,
                                 HD_COMMON_MEM_DMA_TO_DEVICE);
    }

    // Re-set output buffers
    for (uint32_t i = 0; i < net.num_outputs; i++) {
        VENDOR_AI3_BUF ob;
        memset(&ob, 0, sizeof(ob));
        vendor_ai3_net_get(net.proc_id, static_cast<VENDOR_AI3_NET_PARAM_ID>(net.info.out_path_list[i]), &ob);
        ob.pa   = net.out_bufs[i].pa;
        ob.va   = net.out_bufs[i].va;
        ob.size = net.out_bufs[i].size;
        vendor_ai3_net_set(net.proc_id, static_cast<VENDOR_AI3_NET_PARAM_ID>(net.info.out_path_list[i]), &ob);
    }

    // Run inference
    HD_RESULT ret = vendor_ai3_net_proc(net.proc_id);
    if (ret != HD_OK) {
        spdlog::warn("NpuInference: inference failed for {}: {}", net.name, static_cast<int>(ret));
        return false;
    }

    // Invalidate output caches
    for (uint32_t i = 0; i < net.num_outputs; i++) {
        VENDOR_AI3_BUF ob;
        memset(&ob, 0, sizeof(ob));
        vendor_ai3_net_get(net.proc_id, static_cast<VENDOR_AI3_NET_PARAM_ID>(net.info.out_path_list[i]), &ob);
        if (ob.va) {
            hd_common_mem_cache_sync(reinterpret_cast<void*>(ob.va), ob.size,
                                     HD_COMMON_MEM_DMA_FROM_DEVICE);
        }
    }

    return true;
#else
    (void)phy_addr; (void)va; (void)width; (void)height; (void)line_ofs;
    return false;
#endif
}

bool NpuInference::GetOutput(int net_id, uint32_t idx, NpuOutputInfo& out) {
    std::lock_guard<std::mutex> lock(mutex_);

    if (net_id < 0 || net_id >= static_cast<int>(networks_.size())) return false;
    auto& net = networks_[net_id];
    if (!net.loaded || idx >= net.num_outputs) return false;

#if VENDOR_AI3_ENABLED
    VENDOR_AI3_BUF ob;
    memset(&ob, 0, sizeof(ob));
    HD_RESULT ret = vendor_ai3_net_get(net.proc_id, static_cast<VENDOR_AI3_NET_PARAM_ID>(net.info.out_path_list[idx]), &ob);
    if (ret != HD_OK) return false;

    out.va         = ob.va;
    out.size       = ob.size;
    out.fmt        = static_cast<uint32_t>(ob.fmt);
    out.scale      = ob.scale_ratio;
    out.zero_point = ob.zero_point;
    out.width      = ob.width;
    out.height     = ob.height;
    out.channel    = ob.channel;
    return true;
#else
    (void)idx; (void)out;
    return false;
#endif
}

uint32_t NpuInference::GetOutputCount(int net_id) const {
    std::lock_guard<std::mutex> lock(mutex_);
    if (net_id < 0 || net_id >= static_cast<int>(networks_.size())) return 0;
    return networks_[net_id].num_outputs;
}

bool NpuInference::IsLoaded(int net_id) const {
    std::lock_guard<std::mutex> lock(mutex_);
    if (net_id < 0 || net_id >= static_cast<int>(networks_.size())) return false;
    return networks_[net_id].loaded;
}

// ============================================================================
// Private helpers
// ============================================================================

#if VENDOR_AI3_ENABLED
bool NpuInference::AllocMem(NpuNetwork::MemBuf& buf, const char* name, uint32_t size) {
    UINTPTR pa = 0;
    void* va = nullptr;
    HD_RESULT ret = hd_common_mem_alloc(const_cast<CHAR*>(name), &pa, &va, size, DDR_ID0);
    if (ret != HD_OK || !va) {
        spdlog::error("NpuInference: mem alloc failed for {} ({}B): {}", name, size, static_cast<int>(ret));
        return false;
    }
    buf.pa   = pa;
    buf.va   = reinterpret_cast<uintptr_t>(va);
    buf.size = size;
    return true;
}

void NpuInference::FreeMem(NpuNetwork::MemBuf& buf) {
    if (buf.pa && buf.va) {
        hd_common_mem_free(buf.pa, reinterpret_cast<void*>(buf.va));
    }
    buf = {};
}

bool NpuInference::LoadFile(const std::string& path, uintptr_t va, uint32_t size) {
    FILE* f = fopen(path.c_str(), "rb");
    if (!f) return false;
    size_t rd = fread(reinterpret_cast<void*>(va), 1, size, f);
    fclose(f);
    if (rd != size) return false;
    hd_common_mem_cache_sync(reinterpret_cast<void*>(va), size,
                             HD_COMMON_MEM_DMA_TO_DEVICE);
    return true;
}

int32_t NpuInference::FileSize(const std::string& path) {
    struct stat st;
    if (stat(path.c_str(), &st) != 0) return -1;
    return static_cast<int32_t>(st.st_size);
}
#endif

} // namespace ai
} // namespace ipcam
