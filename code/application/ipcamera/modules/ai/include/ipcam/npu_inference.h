/**
 * @file npu_inference.h
 * @brief NPU inference engine wrapper for vendor AI3
 *
 * Manages model loading, inference, and output retrieval using
 * the Novatek vendor_ai3 API. Supports multiple networks (YOLO,
 * SCRFD, OSNet, etc.) loaded concurrently on the CNN30 NPU.
 */
#ifndef IPCAM_NPU_INFERENCE_H_
#define IPCAM_NPU_INFERENCE_H_

#include <string>
#include <vector>
#include <cstdint>
#include <mutex>

#if VENDOR_AI3_ENABLED
extern "C" {
#include "hd_type.h"
#include "vendor_ai.h"
}
#endif

namespace ipcam {
namespace ai {

/// Information about a single output tensor from the NPU
struct NpuOutputInfo {
    uintptr_t va;           ///< Virtual address of output data
    uint32_t  size;         ///< Size in bytes
    uint32_t  fmt;          ///< Quantization format
    float     scale;        ///< Dequantization scale
    int32_t   zero_point;   ///< Dequantization zero point
    uint32_t  width;        ///< Tensor width
    uint32_t  height;       ///< Tensor height
    uint32_t  channel;      ///< Tensor channels
};

/// Represents a loaded neural network model
struct NpuNetwork {
    std::string name;
    std::string model_path;
    bool loaded = false;
    uint32_t num_outputs = 0;

#if VENDOR_AI3_ENABLED
    // Vendor AI3 handles
    UINT32 proc_id = 0;
    VENDOR_AI3_NET_INFO info = {};

    // Memory buffers (PA/VA/size)
    struct MemBuf {
        uintptr_t pa = 0;
        uintptr_t va = 0;
        uint32_t  size = 0;
    };
    MemBuf model_buf;
    MemBuf work_buf;
    MemBuf ronly_buf;
    std::vector<MemBuf> out_bufs;
#endif
};

/**
 * @brief NPU inference engine singleton
 *
 * Usage:
 *   auto& npu = NpuInference::Instance();
 *   npu.Init();
 *   int yolo_id = npu.LoadNetwork("yolo", "/mnt/app/models/yolo26n.bin");
 *   npu.Infer(yolo_id, frame_phy_addr, frame_va, width, height, line_ofs);
 *   auto outputs = npu.GetOutputs(yolo_id);
 *   // ... postprocess outputs ...
 *   npu.UnloadNetwork(yolo_id);
 *   npu.Shutdown();
 */
class NpuInference {
public:
    static NpuInference& Instance();

    /// Initialize the AI3 device. Call once at startup.
    bool Init();

    /// Shutdown the AI3 device. Unloads all networks.
    void Shutdown();

    /// Load a model file and prepare it for inference.
    /// @return network ID (index), or -1 on failure
    int LoadNetwork(const std::string& name, const std::string& model_path);

    /// Unload a previously loaded network
    void UnloadNetwork(int net_id);

    /// Run inference on a live video frame (from videoproc output)
    /// @param net_id  Network ID from LoadNetwork()
    /// @param phy_addr  Physical address of NV12 frame
    /// @param va        Virtual address (mmap'd)
    /// @param width     Frame width
    /// @param height    Frame height
    /// @param line_ofs  Line offset (stride) in bytes
    /// @return true on success
    bool Infer(int net_id, uintptr_t phy_addr, uintptr_t va,
               uint32_t width, uint32_t height, uint32_t line_ofs);

    /// Get output tensor info after inference
    /// @param net_id  Network ID
    /// @param idx     Output tensor index
    /// @param out     Output info struct to fill
    /// @return true on success
    bool GetOutput(int net_id, uint32_t idx, NpuOutputInfo& out);

    /// Get number of output tensors for a network
    uint32_t GetOutputCount(int net_id) const;

    /// Check if a network is loaded
    bool IsLoaded(int net_id) const;

    /// Check if the NPU device is initialized
    bool IsInitialized() const { return initialized_; }

private:
    NpuInference() = default;
    ~NpuInference();
    NpuInference(const NpuInference&) = delete;
    NpuInference& operator=(const NpuInference&) = delete;

    void UnloadNetworkLocked(int net_id);

    bool initialized_ = false;
    mutable std::mutex mutex_;
    std::vector<NpuNetwork> networks_;

#if VENDOR_AI3_ENABLED
    bool AllocMem(NpuNetwork::MemBuf& buf, const char* name, uint32_t size);
    void FreeMem(NpuNetwork::MemBuf& buf);
    bool LoadFile(const std::string& path, uintptr_t va, uint32_t size);
    int32_t FileSize(const std::string& path);
#endif
};

} // namespace ai
} // namespace ipcam

#endif // IPCAM_NPU_INFERENCE_H_
