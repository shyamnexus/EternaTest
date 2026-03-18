/*-----------------------------------------------------------------------------*/
/* Include Header Files                                                        */
/*-----------------------------------------------------------------------------*/
#if defined(__LINUX)
#include <linux/module.h>
#include <linux/stat.h>
#if defined (__aarch64__)
#include <linux/soc/nvt/fmem.h>
#else
#include <mach/fmem.h>
#endif
#include <linux/uaccess.h>
#include <asm/cache.h>  // to get ARCH_DMA_MINALIGN
#elif defined(__FREERTOS)

#elif defined(__OPTEEOS)
#include <tee_api_types.h>
#include <tee/cache.h>
#include <mm/core_memprot.h>
#else
#error Not supported OS
#endif

#define __MODULE__    rtos_cpu
#define __DBGLVL__    8 // 0=FATAL, 1=ERR, 2=WRN, 3=UNIT, 4=FUNC, 5=IND, 6=MSG, 7=VALUE, 8=USER
#define __DBGFLT__    "*"
#include <kwrap/debug.h>
#include <kwrap/cpu.h>
#include "vos_ioctl.h"

/*-----------------------------------------------------------------------------*/
/* Local Types Declarations                                                    */
/*-----------------------------------------------------------------------------*/
#define RTOS_CPU_INITED_TAG     MAKEFOURCC('R', 'C', 'P', 'U') ///< a key value

#if 0 //0 (disable the check), 1 (enable the check)
#define VOS_CPU_IS_ALIGNED(val) VOS_IS_ALIGNED(val)
#else
#define VOS_CPU_IS_ALIGNED(val) 1
#endif
/*-----------------------------------------------------------------------------*/
/* Local Global Variables                                                      */
/*-----------------------------------------------------------------------------*/
unsigned int rtos_cpu_debug_level = NVT_DBG_WRN;

/*-----------------------------------------------------------------------------*/
/* Interface Functions                                                         */
/*-----------------------------------------------------------------------------*/
void rtos_cpu_init(void *param)
{
	if (NULL == param) {
		return;
	}
}

void rtos_cpu_exit(void)
{
}

VOS_ADDR vos_cpu_get_phy_addr(VOS_ADDR vaddr)
{
	paddr_t ret_addr;

	ret_addr = virt_to_phys((void *)vaddr);
	if (0 == ret_addr) {
		DBG_ERR("Get pa fail, vaddr 0x%X\r\n", (unsigned int)vaddr);
		return VOS_ADDR_INVALID;
	}

	return (VOS_ADDR)ret_addr;
}

void vos_cpu_dcache_sync(VOS_ADDR vaddr, UINT32 len, VOS_DMA_DIRECTION dir)
{
	enum utee_cache_operation op;

	if (dir <= VOS_DMA_NONE) {
		//check the alignment
		if (!VOS_CPU_IS_ALIGNED(vaddr) || !VOS_CPU_IS_ALIGNED(len)) {
			DBG_ERR("vaddr(0x%lX) or len(0x%X) not aligned %d\r\n", (ULONG)vaddr, len, VOS_ALIGN_BYTES);
			return;
		}
	}

	switch (dir) {
	case VOS_DMA_BIDIRECTIONAL:
	case VOS_DMA_BIDIRECTIONAL_NON_ALIGN:
		op = TEE_CACHEFLUSH;
		break;
	case VOS_DMA_TO_DEVICE:
	case VOS_DMA_TO_DEVICE_NON_ALIGN:
		op = TEE_CACHECLEAN;
		break;
	case VOS_DMA_FROM_DEVICE:
	case VOS_DMA_FROM_DEVICE_NON_ALIGN:
		op = TEE_CACHEINVALIDATE;
		break;
	default:
		DBG_ERR("Invalid dir %d\r\n", (unsigned int)dir);
		return;
	};

	if (TEE_SUCCESS != cache_operation(op, (void *)vaddr, (size_t)len)) {
		DBG_ERR("cache_operation failed, op %d vaddr %x len %d\r\n", (UINT)op, (UINT)vaddr, (UINT)len);
	}
}

#if defined(__LINUX)
void vos_cpu_dcache_sync_vb(VOS_ADDR vaddr, UINT32 len, VOS_DMA_DIRECTION dir)
{
	VOS_DMA_DIRECTION real_dir;

	if (dir <= VOS_DMA_NONE) {
		//check the alignment
		if (!VOS_CPU_IS_ALIGNED(vaddr) || !VOS_CPU_IS_ALIGNED(len)) {
			DBG_ERR("vaddr(0x%lX) or len(0x%X) not aligned %d\r\n", (ULONG)vaddr, len, VOS_ALIGN_BYTES);
			return;
		}
		real_dir = dir;
	} else if (dir <= VOS_DMA_FROM_DEVICE_NON_ALIGN) {
		//skip the alignment check
		real_dir = dir - VOS_DMA_ALIGN_DIFF;
	} else {
		DBG_ERR("Invalid dir %d\r\n", dir);
		return;
	}

	fmem_dcache_sync_vb((void *)vaddr, len, (enum dma_data_direction)real_dir);
}

int vos_cpu_dcache_sync_by_cpu(VOS_ADDR vaddr, UINT32 len, VOS_DMA_DIRECTION dir, UINT cache_op_cpu_id)
{
#if 0
	unsigned long fmem_ret;

	fmem_ret = fmem_dcache_sync_by_cpu((void *)vaddr, len, (enum dma_data_direction)dir, cache_op_cpu_id);
	if (fmem_ret != 0) {
		DBG_ERR("fmem_dcache_sync_by_cpu failed, ret %ld, cpu id %d\r\n", fmem_ret, cache_op_cpu_id);
		return -1;
	}

	return 0;
#else
	DBG_ERR("not supported\r\n");
	return -1;
#endif
}

int _IOFUNC_CPU_IOCMD_DCACHE_SYNC(unsigned long arg)
{
	VOS_CPU_IOARG ioarg = {0};

	if (copy_from_user((void *)&ioarg, (void *)arg, sizeof(VOS_CPU_IOARG))) {
		DBG_ERR("copy_from_user failed\n");
		return -EFAULT;
	}

	if (ioarg.is_vb) {
		vos_cpu_dcache_sync_vb((VOS_ADDR)ioarg.vaddr, (UINT32)ioarg.len, (VOS_DMA_DIRECTION)ioarg.dir);
	} else {
		vos_cpu_dcache_sync((VOS_ADDR)ioarg.vaddr, (UINT32)ioarg.len, (VOS_DMA_DIRECTION)ioarg.dir);
	}

	return 0;
}

int _IOFUNC_CPU_IOCMD_GET_PHY_ADDR(unsigned long arg)
{
	VOS_CPU_IOARG_VA_PA ioarg = {0};

	if (copy_from_user((void *)&ioarg, (void *)arg, sizeof(VOS_CPU_IOARG_VA_PA))) {
		DBG_ERR("copy_from_user failed\n");
		return -EFAULT;
	}

	//return to users
	ioarg.paddr = (unsigned long)vos_cpu_get_phy_addr((VOS_ADDR)ioarg.vaddr);
	if (copy_to_user((void *)arg, (void *)&ioarg, sizeof(VOS_CPU_IOARG_VA_PA))) {
		return -EFAULT;
	}

	return 0;
}

int _IOFUNC_CPU_IOCMD_SYNC_CPU(unsigned long arg)
{
	VOS_CPU_IOARG_SYNC_CPU ioarg = {0};

	if (copy_from_user((void *)&ioarg, (void *)arg, sizeof(VOS_CPU_IOARG_SYNC_CPU))) {
		DBG_ERR("copy_from_user failed\n");
		return -EFAULT;
	}

	if (0 != vos_cpu_dcache_sync_by_cpu((VOS_ADDR)ioarg.vaddr, (UINT32)ioarg.len, (VOS_DMA_DIRECTION)ioarg.dir, (UINT)ioarg.cpu_id)) {
		return -EPERM;
	}

	return 0;
}

/*-----------------------------------------------------------------------------*/
/* Kernel Mode Definiton                                                       */
/*-----------------------------------------------------------------------------*/
EXPORT_SYMBOL(vos_cpu_get_phy_addr);
EXPORT_SYMBOL(vos_cpu_dcache_sync);
EXPORT_SYMBOL(vos_cpu_dcache_sync_vb);
EXPORT_SYMBOL(vos_cpu_dcache_sync_by_cpu);
#endif //defined(__LINUX)