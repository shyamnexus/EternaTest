/**
 * @file physical_memory.h
 * @brief Physical memory management for firmware staging
 * 
 * Wraps HDAL hd_common_mem_alloc for allocating physically contiguous
 * memory that persists across reboot or can be accessed by flash tools.
 */

#pragma once

#include "upgrade_types.h"
#include "compat.h"
#include <memory>

namespace ipcam::upgrade {

/**
 * @brief Physical memory allocator for firmware staging
 * 
 * Allocates physically contiguous memory using HDAL APIs.
 * This memory is required for:
 * - DMA transfers during flash operations
 * - Bootloader access after reboot
 */
class PhysicalMemory {
public:
    PhysicalMemory();
    ~PhysicalMemory();
    
    // Non-copyable, moveable
    PhysicalMemory(const PhysicalMemory&) = delete;
    PhysicalMemory& operator=(const PhysicalMemory&) = delete;
    PhysicalMemory(PhysicalMemory&&) noexcept;
    PhysicalMemory& operator=(PhysicalMemory&&) noexcept;
    
    /**
     * @brief Allocate physically contiguous memory
     * @param size Required size in bytes
     * @param name Optional name for debugging
     * @return Memory block on success, error on failure
     */
    Expected<PhysicalMemoryBlock, UpgradeError>
    allocate(size_t size, const std::string& name = "fw_upgrade");
    
    /**
     * @brief Release allocated memory
     */
    void release();
    
    /**
     * @brief Check if memory is allocated
     */
    bool isAllocated() const;
    
    /**
     * @brief Get allocated memory block info
     */
    const PhysicalMemoryBlock* memoryBlock() const;
    
    /**
     * @brief Write data to physical memory
     * @param offset Offset within allocated block
     * @param data Source data
     * @param size Size to write
     * @return True on success
     */
    bool write(size_t offset, const void* data, size_t size);
    
    /**
     * @brief Read data from physical memory
     * @param offset Offset within allocated block
     * @param data Destination buffer
     * @param size Size to read
     * @return True on success
     */
    bool read(size_t offset, void* data, size_t size) const;
    
    /**
     * @brief Flush cache to ensure data is in physical memory
     */
    void flushCache();
    
    /**
     * @brief Get available physical memory for firmware
     * Checks system memory configuration
     */
    static size_t availableMemory();
    
    /**
     * @brief Get the all-in-one memory region address
     * From memory configuration (BOARD_ALL_IN_ONE_ADDR)
     */
    static uint64_t getAllInOneAddress();
    
    /**
     * @brief Get the all-in-one memory region size
     * From memory configuration (BOARD_ALL_IN_ONE_SIZE)
     */
    static size_t getAllInOneSize();

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace ipcam::upgrade
