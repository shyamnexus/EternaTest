/**
 * @file physical_memory.cpp
 * @brief Physical/DMA memory management for firmware staging
 * 
 * Uses HDAL hd_common_mem_alloc API to allocate from managed memory pools,
 * avoiding conflicts with NVTMPP video buffer regions.
 */

#include "ipcam/physical_memory.h"
#include <spdlog/spdlog.h>

// HDAL headers for memory allocation
extern "C" {
#include <hd_common.h>
}

#include <cstring>

namespace ipcam::upgrade {

// Default firmware staging size (128MB max for most platforms)
constexpr size_t DEFAULT_FIRMWARE_SIZE = 128 * 1024 * 1024;

// =============================================================================
// Implementation
// =============================================================================

struct PhysicalMemory::Impl {
    PhysicalMemoryBlock block{};
    UINTPTR physAddr = 0;
    void* virtAddr = nullptr;
    std::string allocName;
    
    void cleanup() {
        if (block.allocated && physAddr != 0 && virtAddr != nullptr) {
            HD_RESULT ret = hd_common_mem_free(physAddr, virtAddr);
            if (ret != HD_OK) {
                spdlog::warn("PhysicalMemory: hd_common_mem_free failed: {}", static_cast<int>(ret));
            }
        }
        physAddr = 0;
        virtAddr = nullptr;
        block = {};
    }
};

// =============================================================================
// PhysicalMemory
// =============================================================================

PhysicalMemory::PhysicalMemory()
    : impl_(std::make_unique<Impl>())
{
}

PhysicalMemory::~PhysicalMemory()
{
    release();
}

PhysicalMemory::PhysicalMemory(PhysicalMemory&& other) noexcept
    : impl_(std::move(other.impl_))
{
    other.impl_ = std::make_unique<Impl>();
}

PhysicalMemory& PhysicalMemory::operator=(PhysicalMemory&& other) noexcept
{
    if (this != &other) {
        release();
        impl_ = std::move(other.impl_);
        other.impl_ = std::make_unique<Impl>();
    }
    return *this;
}

Expected<PhysicalMemoryBlock, UpgradeError>
PhysicalMemory::allocate(size_t size, const std::string& name)
{
    if (impl_->block.allocated) {
        spdlog::warn("PhysicalMemory::allocate: Already allocated, releasing first");
        release();
    }
    
    impl_->allocName = name;
    
    // Use HDAL hd_common_mem_alloc which allocates from managed memory pools
    // This avoids conflicts with NVTMPP video buffer regions
    HD_RESULT ret = hd_common_mem_alloc(
        const_cast<char*>(name.c_str()),
        &impl_->physAddr,
        &impl_->virtAddr,
        static_cast<UINT32>(size),
        DDR_ID0  // Use first DDR
    );
    
    if (ret != HD_OK) {
        spdlog::error("PhysicalMemory: hd_common_mem_alloc failed: {} (size={})", 
                      static_cast<int>(ret), size);
        return Expected<PhysicalMemoryBlock, UpgradeError>(
            Expected<PhysicalMemoryBlock, UpgradeError>::UnexpectedTag{},
            UpgradeError::MemoryAllocationFailed);
    }
    
    impl_->block.physicalAddress = static_cast<uint64_t>(impl_->physAddr);
    impl_->block.virtualAddress = impl_->virtAddr;
    impl_->block.size = size;
    impl_->block.allocated = true;
    
    spdlog::info("PhysicalMemory: Allocated {} bytes '{}' at phys=0x{:08X}, virt={:p}",
                 size, name, impl_->physAddr, impl_->virtAddr);
    
    return impl_->block;
}

void PhysicalMemory::release()
{
    if (impl_) {
        impl_->cleanup();
    }
}

bool PhysicalMemory::isAllocated() const
{
    return impl_ && impl_->block.allocated;
}

const PhysicalMemoryBlock* PhysicalMemory::memoryBlock() const
{
    if (impl_ && impl_->block.allocated) {
        return &impl_->block;
    }
    return nullptr;
}

bool PhysicalMemory::write(size_t offset, const void* data, size_t size)
{
    if (!isAllocated()) {
        spdlog::error("PhysicalMemory::write: Not allocated");
        return false;
    }
    
    if (offset + size > impl_->block.size) {
        spdlog::error("PhysicalMemory::write: Out of bounds ({} + {} > {})",
                      offset, size, impl_->block.size);
        return false;
    }
    
    uint8_t* dest = static_cast<uint8_t*>(impl_->block.virtualAddress) + offset;
    std::memcpy(dest, data, size);
    return true;
}

bool PhysicalMemory::read(size_t offset, void* data, size_t size) const
{
    if (!isAllocated()) {
        spdlog::error("PhysicalMemory::read: Not allocated");
        return false;
    }
    
    if (offset + size > impl_->block.size) {
        spdlog::error("PhysicalMemory::read: Out of bounds");
        return false;
    }
    
    const uint8_t* src = static_cast<const uint8_t*>(impl_->block.virtualAddress) + offset;
    std::memcpy(data, src, size);
    return true;
}

void PhysicalMemory::flushCache()
{
    if (isAllocated() && impl_->virtAddr) {
        // HDAL memory is typically non-cached for DMA operations,
        // but we can use __builtin_aarch64_dc for ARM64 cache flush if needed
        // For now, rely on HDAL's memory being uncached
        __sync_synchronize();  // Memory barrier
    }
}

size_t PhysicalMemory::availableMemory()
{
    return getAllInOneSize();
}

uint64_t PhysicalMemory::getAllInOneAddress()
{
    // Query HDAL for available memory info
    HD_COMMON_MEM_POOL_INFO mem_info;
    mem_info.type = HD_COMMON_MEM_COMMON_POOL;
    mem_info.ddr_id = DDR_ID0;
    
    if (hd_common_mem_get(HD_COMMON_MEM_PARAM_POOL_CONFIG, &mem_info) == HD_OK) {
        return static_cast<uint64_t>(mem_info.start_addr);
    }
    
    return 0;
}

size_t PhysicalMemory::getAllInOneSize()
{
    // Query HDAL for available memory info
    HD_COMMON_MEM_POOL_INFO mem_info;
    mem_info.type = HD_COMMON_MEM_COMMON_POOL;
    mem_info.ddr_id = DDR_ID0;
    
    if (hd_common_mem_get(HD_COMMON_MEM_PARAM_POOL_CONFIG, &mem_info) == HD_OK) {
        return static_cast<size_t>(mem_info.blk_size * mem_info.blk_cnt);
    }
    
    return DEFAULT_FIRMWARE_SIZE;
}

} // namespace ipcam::upgrade
