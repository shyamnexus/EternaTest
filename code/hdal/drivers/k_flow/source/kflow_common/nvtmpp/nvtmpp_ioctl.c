#if defined(__LINUX)
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/platform_device.h>
#include <linux/fs.h>
#include <linux/vmalloc.h>
#include <linux/slab.h>
#include <linux/sched.h>
#include <linux/io.h>
#include <linux/of_device.h>
#include <linux/kdev_t.h>
#include <linux/clk.h>
#include <linux/dma-mapping.h>
#include <linux/uaccess.h>
#if defined (__aarch64__)
#include <linux/soc/nvt/fmem.h>
#else
#include <mach/fmem.h>
#endif
#endif

#include "kwrap/cpu.h"
#include "kflow_common/nvtmpp.h"
#include "kflow_common/nvtmpp_ioctl.h"
#include "nvtmpp_int.h"
#include "nvtmpp_init.h"
#include "nvtmpp_module.h"
#include "nvtmpp_blk.h"
#include "nvtmpp_pool.h"
#include "nvtmpp_debug.h"
#include "nvtmpp_heap.h"

#define FLUSH_ALL_THOLD_SIZE   0x80000


extern NVTMPP_VB_POOL_S *nvtmpp_vb_get_pool_by_mem_range_p(uintptr_t mem_start, uintptr_t mem_end);
extern NVTMPP_VB_POOL_S *nvtmpp_vb_get_pool_s(NVTMPP_VB_POOL pool);


#if defined(__LINUX)
int nvtmpp_user_va_cacheable(uintptr_t user_va)
{
    struct vm_area_struct *vma = NULL;
    pgprot_t nc_prot = {0}, wb_prot = {0};

    if (current && current->mm) {
        vma = find_vma(current->mm, user_va);
        if (vma) {
            //DBG_DUMP("user_va = 0x%x, vm_start = 0x%x, vm_end = 0x%x, vm_page_prot = 0x%x\r\n",
            //    (int)user_va, (int)vma->vm_start, (int)vma->vm_end, vma->vm_page_prot);
            nc_prot = pgprot_noncached(vma->vm_page_prot);
            wb_prot = pgprot_writecombine(vma->vm_page_prot);
            if (memcmp(&nc_prot, &vma->vm_page_prot, sizeof(pgprot_t)) == 0) {
                // DBG_IND("This is non-cache\r\n");
                return EFAULT;
            } else if (memcmp(&wb_prot, &vma->vm_page_prot, sizeof(pgprot_t)) == 0) {
                // DBG_IND("This is non-cache\r\n");
                return EFAULT;
            } else {
                // DBG_IND("This is cache\r\n");
                return 0;
            }
        } else {
            // DBG_ERR("find vma_fail for user_va 0x%lx\r\n", user_va);
            return ENXIO;
        }
    } else {
        // DBG_ERR("This is not user task\r\n");
        return ESPIPE;
    }
}
long nvtmpp_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	int __user *argp = (int __user *)arg;

	DBG_IND("cmd = 0x%x\r\n", cmd);
	switch (cmd) {

	case NVTMPP_IOC_VB_CONF_SET: {
			NVTMPP_IOC_VB_CONF_S msg;

			memset((void *)&msg, 0x00, sizeof(msg));
			if (unlikely(copy_from_user(&msg, argp, sizeof(msg)))) {
				return -EFAULT;
			}
			msg.rtn = nvtmpp_vb_set_conf((NVTMPP_VB_CONF_S *)&msg);
			if (unlikely(copy_to_user(argp, &msg, sizeof(msg)))) {
				return -EFAULT;
			}
		}
		break;
	case NVTMPP_IOC_VB_CONF_GET: {
			NVTMPP_IOC_VB_CONF_S msg;

			memset((void *)&msg, 0x00, sizeof(msg));
			msg.rtn = nvtmpp_vb_get_conf((NVTMPP_VB_CONF_S *)&msg);
			if (unlikely(copy_to_user(argp, &msg, sizeof(msg)))) {
				return -EFAULT;
			}
		}
		break;
	case NVTMPP_IOC_VB_INIT: {
			NVTMPP_IOC_VB_INIT_S msg;

			memset((void *)&msg, 0x00, sizeof(msg));
			msg.rtn = nvtmpp_vb_init();
			if (unlikely(copy_to_user(argp, &msg, sizeof(msg)))) {
				return -EFAULT;
			}
		}
		break;
	case NVTMPP_IOC_VB_EXIT: {
			NVTMPP_IOC_VB_INIT_S msg;

			memset((void *)&msg, 0x00, sizeof(msg));
			msg.rtn = nvtmpp_vb_exit();
			if (unlikely(copy_to_user(argp, &msg, sizeof(msg)))) {
				return -EFAULT;
			}
		}
		break;
	case NVTMPP_IOC_VB_RELAYOUT: {
			NVTMPP_IOC_VB_INIT_S msg;

			memset((void *)&msg, 0x00, sizeof(msg));
			msg.rtn = nvtmpp_vb_relayout();
			if (unlikely(copy_to_user(argp, &msg, sizeof(msg)))) {
				return -EFAULT;
			}
		}
		break;
	case NVTMPP_IOC_VB_CREATE_POOL: {
			NVTMPP_IOC_VB_CREATE_POOL_S msg;

			memset((void *)&msg, 0x00, sizeof(msg));
			if (unlikely(copy_from_user(&msg, argp, sizeof(msg)))) {
				return -EFAULT;
			}
			msg.rtn = nvtmpp_vb_create_pool(msg.pool_name, msg.blk_size, msg.blk_cnt, msg.ddr);
			if (unlikely(copy_to_user(argp, &msg, sizeof(msg)))) {
				return -EFAULT;
			}
		}
		break;
	case NVTMPP_IOC_VB_CREATE_FIXED_POOL: {
			NVTMPP_IOC_VB_CREATE_FIXPOOL_S msg;
			NVTMPP_VB_POOL_S              *p_pool;
			NVTMPP_VB_POOL                 pool;
			NVTMPP_VB_BLK                  blk;

			memset((void *)&msg, 0x00, sizeof(msg));
			if (unlikely(copy_from_user(&msg, argp, sizeof(msg)))) {
				return -EFAULT;
			}
			pool = nvtmpp_sys_create_fixed_pool(msg.pool_name, msg.blk_size, msg.blk_cnt, msg.ddr);
			if (NVTMPP_VB_INVALID_POOL != pool) {
				p_pool = nvtmpp_vb_get_pool_s(pool);
				if (!p_pool) {
					msg.rtn = 0;
				} else {
					blk = nvtmpp_vb_get_free_block_from_pool(p_pool, msg.blk_size);
					msg.rtn = nvtmpp_vb_blk2pa(blk);
				}
			} else {
				msg.rtn = 0;
			}
			if (unlikely(copy_to_user(argp, &msg, sizeof(msg)))) {
				return -EFAULT;
			}
		}
		break;
	case NVTMPP_IOC_VB_DESTROY_POOL: {
			NVTMPP_IOC_VB_DESTROY_POOL_S msg;

			memset((void *)&msg, 0x00, sizeof(msg));
			if (unlikely(copy_from_user(&msg, argp, sizeof(msg)))) {
				return -EFAULT;
			}
			msg.rtn = nvtmpp_vb_destroy_pool(msg.pool);
			if (unlikely(copy_to_user(argp, &msg, sizeof(msg)))) {
				return -EFAULT;
			}
		}
		break;
	case NVTMPP_IOC_VB_GET_BLK: {
			NVTMPP_IOC_VB_GET_BLK_S msg;

			memset((void *)&msg, 0x00, sizeof(msg));
			if (unlikely(copy_from_user(&msg, argp, sizeof(msg)))) {
				return -EFAULT;
			}
			msg.rtn = nvtmpp_vb_get_block(USER_MODULE, msg.pool, msg.blk_size, msg.ddr);
			if (unlikely(copy_to_user(argp, &msg, sizeof(msg)))) {
				return -EFAULT;
			}
		}
		break;
	case NVTMPP_IOC_VB_REL_BLK: {
			NVTMPP_IOC_VB_REL_BLK_S msg;

			memset((void *)&msg, 0x00, sizeof(msg));
			if (unlikely(copy_from_user(&msg, argp, sizeof(msg)))) {
				return -EFAULT;
			}
			msg.rtn = nvtmpp_vb_unlock_block(USER_MODULE, msg.blk);
			if (unlikely(copy_to_user(argp, &msg, sizeof(msg)))) {
				return -EFAULT;
			}
		}
		break;
	case NVTMPP_IOC_VB_GET_BLK_PA: {
			NVTMPP_IOC_VB_GET_BLK_PA_S msg;

			memset((void *)&msg, 0x00, sizeof(msg));
			if (unlikely(copy_from_user(&msg, argp, sizeof(msg)))) {
				return -EFAULT;
			}
			msg.rtn = nvtmpp_vb_blk2pa(msg.blk);
			if (unlikely(copy_to_user(argp, &msg, sizeof(msg)))) {
				return -EFAULT;
			}
		}
		break;
	case NVTMPP_IOC_VB_CACHE_SYNC: {
			NVTMPP_IOC_VB_CACHE_SYNC_S msg = {0};
			UINT32                     dma_dir;
			uintptr_t                  pa;

			if (unlikely(copy_from_user(&msg, argp, sizeof(msg)))) {
				return -EFAULT;
			}
			pa = fmem_lookup_pa((ULONG)msg.virt_addr);
			if (pa == -1UL ) {
				DBG_ERR("Err va = 0x%lx\r\n", (ULONG)msg.virt_addr);
				msg.rtn = NVTMPP_ER_PARM;
			} else {
				msg.rtn = NVTMPP_ER_OK;
				dma_dir = msg.dma_dir;
				#if defined (__aarch64__)
				if (dma_dir == VOS_DMA_FROM_DEVICE) {
					dma_dir = VOS_DMA_BIDIRECTIONAL;
				}
				#endif
				vos_cpu_dcache_sync((VOS_ADDR)msg.virt_addr, msg.size, dma_dir);
			}
			if (unlikely(copy_to_user(argp, &msg, sizeof(msg)))) {
				return -EFAULT;
			}
		}
		break;
	case NVTMPP_IOC_VB_CACHE_SYNC_BY_CPU: {
			NVTMPP_IOC_VB_CACHE_SYNC_BY_CPU_S msg = {0};
			UINT32                     dma_dir;
			uintptr_t                  pa;

			if (unlikely(copy_from_user(&msg, argp, sizeof(msg)))) {
				return -EFAULT;
			}
			pa = fmem_lookup_pa((ULONG)msg.virt_addr);
			if (pa == -1UL ) {
				DBG_ERR("Err va = 0x%lx\r\n", (ULONG)msg.virt_addr);
				msg.rtn = NVTMPP_ER_PARM;
			} else {
				msg.rtn = NVTMPP_ER_OK;
				dma_dir = msg.dma_dir;
				#if defined (__aarch64__)
				if (dma_dir == VOS_DMA_FROM_DEVICE) {
					dma_dir = VOS_DMA_BIDIRECTIONAL;
				}
				#endif
				//
				#if 0
				if (msg.cpu_count <= 1 && msg.size >= FLUSH_ALL_THOLD_SIZE ) {
					//vos_cpu_dcache_sync_by_cpu((VOS_ADDR)msg.virt_addr, msg.size, dma_dir, msg.cpu_id);
					vos_cpu_dcache_syncall_by_cpu(msg.cpu_id);
				} else {
					vos_cpu_dcache_sync((VOS_ADDR)msg.virt_addr, msg.size, dma_dir);
				}
				#else
				vos_cpu_dcache_sync((VOS_ADDR)msg.virt_addr, msg.size, dma_dir);
				#endif
			}
			if (unlikely(copy_to_user(argp, &msg, sizeof(msg)))) {
				return -EFAULT;
			}
		}
		break;
	case NVTMPP_IOC_VB_PA_TO_POOL: {
			NVTMPP_IOC_VB_PA_TO_POOL_S  msg = {0};
			NVTMPP_VB_POOL_S           *p_pool;

			if (unlikely(copy_from_user(&msg, argp, sizeof(msg)))) {
				return -EFAULT;
			}
			// assume minimum blk size
			p_pool = nvtmpp_vb_get_pool_by_mem_range_p(msg.blk_pa, msg.blk_pa + sizeof(NVTMPP_VB_BLK_S));
			if (p_pool == NULL) {
				msg.pool = NVTMPP_VB_INVALID_POOL;
				msg.size = 0;
			} else {
				msg.pool = p_pool->pool;
				msg.size = p_pool->blk_size;
			}
			if (unlikely(copy_to_user(argp, &msg, sizeof(msg)))) {
				return -EFAULT;
			}
		}
		break;

	case NVTMPP_IOC_VB_PA_TO_VA: {
			NVTMPP_IOC_VB_PA_TO_VA_S    msg = {0};
			uintptr_t                   va;

			if (unlikely(copy_from_user(&msg, argp, sizeof(msg)))) {
				return -EFAULT;
			}
			va = nvtmpp_sys_pa2va(msg.pa);
			if (va == 0) {
				return -EFAULT;
			}
			msg.va = va;
			if (unlikely(copy_to_user(argp, &msg, sizeof(msg)))) {
				return -EFAULT;
			}
		}
		break;
	case NVTMPP_IOC_VB_VA_TO_PA: {
			NVTMPP_IOC_VB_VA_TO_PA_S    msg = {0};
			uintptr_t                   pa;

			if (unlikely(copy_from_user(&msg, argp, sizeof(msg)))) {
				return -EFAULT;
			}
			pa = nvtmpp_sys_va2pa(msg.va);
			if (pa == 0) {
				return -EFAULT;
			}
			msg.pa = pa;
			if (unlikely(copy_to_user(argp, &msg, sizeof(msg)))) {
				return -EFAULT;
			}
		}
		break;
	case NVTMPP_IOC_VB_GET_USER_VA_INFO: {
			NVTMPP_IOC_VB_GET_USER_VA_INFO_S    msg = {0};
			uintptr_t                           pa;

			if (unlikely(copy_from_user(&msg, argp, sizeof(msg)))) {
				return -EFAULT;
			}
			pa = fmem_lookup_pa((ULONG)msg.va);
			msg.rtn = NVTMPP_ER_OK;
			if (pa == -1UL ) {
				DBG_ERR("Err va = 0x%lx\r\n", (ULONG)msg.va);
				msg.rtn = NVTMPP_ER_PARM;
			}
			msg.pa = pa;
			if (nvtmpp_user_va_cacheable((uintptr_t)msg.va) == 0) {
				msg.cached = 1;
			} else {
				msg.cached = 0;
			}
			if (unlikely(copy_to_user(argp, &msg, sizeof(msg)))) {
				return -EFAULT;
			}
		}
		break;

	case NVTMPP_IOC_VB_GET_BRIDGE_MEM: {
			MEM_RANGE mem_range = {0};
			NVTMPP_GET_SYSMEM_REGION_S msg = {0};

			if (unlikely(copy_from_user(&msg, argp, sizeof(msg)))) {
				return -EFAULT;
			}

			if (FALSE == nvtmpp_vb_get_bridge_mem(&mem_range)) {
				msg.rtn = NVTMPP_ER_POOL_UNEXIST;
			} else {
				msg.phys_addr = mem_range.addr;
				msg.size = mem_range.size;
				msg.rtn = NVTMPP_ER_OK;
			}

			if (unlikely(copy_to_user(argp, &msg, sizeof(msg)))) {
				return -EFAULT;
			}
		}
		break;
	case NVTMPP_IOC_VB_GET_MAX_FREE_BLK_SZ: {
			NVTMPP_IOC_VB_GET_MAX_FREE_S        msg = {0};

			if (unlikely(copy_from_user(&msg, argp, sizeof(msg)))) {
				return -EFAULT;
			}
			msg.size = nvtmpp_vb_get_max_free_size(msg.ddr);
			if (unlikely(copy_to_user(argp, &msg, sizeof(msg)))) {
				return -EFAULT;
			}
		}
		break;
	case NVTMPP_IOC_VB_GET_COMM_POOL_RANGE: {
			MEM_RANGE                    mem_range = {0};
			NVTMPP_GET_COMM_POOL_RANGE_S msg = {0};

			if (unlikely(copy_from_user(&msg, argp, sizeof(msg)))) {
				return -EFAULT;
			}
			if (FALSE == nvtmpp_vb_get_comm_pool_range(msg.ddr, &mem_range)) {
				msg.rtn = NVTMPP_ER_POOL_UNEXIST;
			} else {
				msg.phys_addr = mem_range.addr;
				msg.size = mem_range.size;
				msg.rtn = NVTMPP_ER_OK;
			}
			if (unlikely(copy_to_user(argp, &msg, sizeof(msg)))) {
				return -EFAULT;
			}
		}
		break;

	case NVTMPP_IOC_VB_GET_FREE_SZ: {
			NVTMPP_IOC_VB_GET_FREE_S        msg = {0};

			if (unlikely(copy_from_user(&msg, argp, sizeof(msg)))) {
				return -EFAULT;
			}
			msg.size = nvtmpp_heap_get_free_size(msg.ddr);
			if (unlikely(copy_to_user(argp, &msg, sizeof(msg)))) {
				return -EFAULT;
			}
		}
		break;
	case NVTMPP_IOC_VB_GET_HDAL_BASE: {
			NVTMPP_IOC_GET_HDAL_BASE_S        msg = {0};
			NVTMPP_MMZ_INFO_S  *p_mmz;
			UINT32             i;

			if (unlikely(copy_from_user(&msg, argp, sizeof(msg)))) {
				return -EFAULT;
			}
			p_mmz = nvtmpp_get_mmz();
			for (i = 0; i < NVTMPP_DDR_MAX; i++) {
				msg.ddr_id[i] = i;
				msg.base[i] = p_mmz->ddr_mem[i].pa;
				msg.size[i] = p_mmz->ddr_mem[i].size;
			}
			if (unlikely(copy_to_user(argp, &msg, sizeof(msg)))) {
				return -EFAULT;
			}
		}
		break;
	case NVTMPP_IOC_VB_LOCK_BLK: {
			NVTMPP_IOC_VB_LOCK_BLK_S msg;

			memset((void *)&msg, 0x00, sizeof(msg));
			if (unlikely(copy_from_user(&msg, argp, sizeof(msg)))) {
				return -EFAULT;
			}
			msg.rtn = nvtmpp_vb_lock_block(USER_MODULE, msg.blk);
			if (unlikely(copy_to_user(argp, &msg, sizeof(msg)))) {
				return -EFAULT;
			}
		}
		break;

	default:
		DBG_ERR("unknown cmd 0x%x\r\n", cmd);
		return -EINVAL;
	}
	return 0;
}
#else
int nvtmpp_ioctl (int fd, unsigned int cmd, void *p_arg)
{
	DBG_IND("cmd = 0x%x\r\n", cmd);
	switch (cmd) {
	case NVTMPP_IOC_VB_CONF_SET: {
			NVTMPP_IOC_VB_CONF_S *p_msg = (NVTMPP_IOC_VB_CONF_S *)p_arg;

			p_msg->rtn = nvtmpp_vb_set_conf((NVTMPP_VB_CONF_S *)p_msg);
		}
		break;
	case NVTMPP_IOC_VB_CONF_GET: {
			NVTMPP_IOC_VB_CONF_S *p_msg = (NVTMPP_IOC_VB_CONF_S *)p_arg;

			p_msg->rtn = nvtmpp_vb_get_conf((NVTMPP_VB_CONF_S *)p_msg);
		}
		break;
	case NVTMPP_IOC_VB_INIT: {
			NVTMPP_IOC_VB_INIT_S *p_msg = (NVTMPP_IOC_VB_INIT_S *)p_arg;

			p_msg->rtn = nvtmpp_vb_init();
		}
		break;
	case NVTMPP_IOC_VB_EXIT: {
			NVTMPP_IOC_VB_INIT_S *p_msg = (NVTMPP_IOC_VB_INIT_S *)p_arg;

			p_msg->rtn = nvtmpp_vb_exit();
		}
		break;
	case NVTMPP_IOC_VB_RELAYOUT: {
			NVTMPP_IOC_VB_INIT_S *p_msg = (NVTMPP_IOC_VB_INIT_S *)p_arg;

			p_msg->rtn = nvtmpp_vb_relayout();
		}
		break;
	case NVTMPP_IOC_VB_CREATE_POOL: {
			NVTMPP_IOC_VB_CREATE_POOL_S *p_msg = (NVTMPP_IOC_VB_CREATE_POOL_S *)p_arg;

			p_msg->rtn = nvtmpp_vb_create_pool(p_msg->pool_name, p_msg->blk_size, p_msg->blk_cnt, p_msg->ddr);
		}
		break;
	case NVTMPP_IOC_VB_CREATE_FIXED_POOL: {
			NVTMPP_IOC_VB_CREATE_FIXPOOL_S *p_msg = (NVTMPP_IOC_VB_CREATE_FIXPOOL_S *)p_arg;
			NVTMPP_VB_POOL_S               *p_pool;
			NVTMPP_VB_POOL                  pool;
			NVTMPP_VB_BLK                   blk;

			pool = nvtmpp_sys_create_fixed_pool(p_msg->pool_name, p_msg->blk_size, p_msg->blk_cnt, p_msg->ddr);
			p_pool = nvtmpp_vb_get_pool_s(pool);
			blk = nvtmpp_vb_get_free_block_from_pool(p_pool, p_msg->blk_size);
			p_msg->rtn = nvtmpp_vb_blk2pa(blk);
		}
		break;
	case NVTMPP_IOC_VB_DESTROY_POOL: {
			NVTMPP_IOC_VB_DESTROY_POOL_S *p_msg = (NVTMPP_IOC_VB_DESTROY_POOL_S *)p_arg;

			p_msg->rtn = nvtmpp_vb_destroy_pool(p_msg->pool);
		}
		break;
	case NVTMPP_IOC_VB_GET_BLK: {
			NVTMPP_IOC_VB_GET_BLK_S *p_msg = (NVTMPP_IOC_VB_GET_BLK_S *)p_arg;

			p_msg->rtn = nvtmpp_vb_get_block(USER_MODULE, p_msg->pool, p_msg->blk_size, p_msg->ddr);
		}
		break;
	case NVTMPP_IOC_VB_REL_BLK: {
			NVTMPP_IOC_VB_REL_BLK_S *p_msg = (NVTMPP_IOC_VB_REL_BLK_S *)p_arg;

			p_msg->rtn = nvtmpp_vb_unlock_block(USER_MODULE, p_msg->blk);
		}
		break;
	case NVTMPP_IOC_VB_GET_BLK_PA: {
			NVTMPP_IOC_VB_GET_BLK_PA_S *p_msg = (NVTMPP_IOC_VB_GET_BLK_PA_S *)p_arg;

			p_msg->rtn = nvtmpp_vb_blk2pa(p_msg->blk);
		}
		break;
	case NVTMPP_IOC_VB_CACHE_SYNC: {
			NVTMPP_IOC_VB_CACHE_SYNC_S *p_msg = (NVTMPP_IOC_VB_CACHE_SYNC_S *)p_arg;

			vos_cpu_dcache_sync((VOS_ADDR)p_msg->virt_addr, p_msg->size, p_msg->dma_dir);
		}
		break;
	case NVTMPP_IOC_VB_PA_TO_POOL: {
			NVTMPP_IOC_VB_PA_TO_POOL_S  *p_msg = (NVTMPP_IOC_VB_PA_TO_POOL_S *)p_arg;
			NVTMPP_VB_POOL_S           *p_pool;
			uintptr_t                   va;

			// assume minimum blk size
			va = nvtmpp_sys_pa2va(p_msg->blk_pa);
			if (va == 0) {
				DBG_ERR("Err pa = 0x%lx\r\n", (ULONG)p_msg->blk_pa);
				return -1;
			}
			p_pool = nvtmpp_vb_get_pool_by_mem_range_p(va, va + sizeof(NVTMPP_VB_BLK_S));
			if (p_pool == NULL) {
				p_msg->pool = NVTMPP_VB_INVALID_POOL;
				p_msg->size = 0;
			} else {
				p_msg->pool = p_pool->pool;
				p_msg->size = p_pool->blk_size;
			}
		}
		break;

	case NVTMPP_IOC_VB_PA_TO_VA: {
			NVTMPP_IOC_VB_PA_TO_VA_S    *p_msg = (NVTMPP_IOC_VB_PA_TO_VA_S *)p_arg;
			uintptr_t                   va;

			va = nvtmpp_sys_pa2va(p_msg->pa);
			if (va == 0) {
				DBG_ERR("Err pa = 0x%lx\r\n", (ULONG)p_msg->pa);
				return -1;
			}
			p_msg->va = va;
		}
		break;
	case NVTMPP_IOC_VB_VA_TO_PA: {
			NVTMPP_IOC_VB_VA_TO_PA_S    *p_msg = (NVTMPP_IOC_VB_VA_TO_PA_S *)p_arg;
			uintptr_t                   pa;

			pa = nvtmpp_sys_pa2va(p_msg->va);
			if (pa == 0) {
				DBG_ERR("Err va = 0x%lx\r\n", (ULONG)p_msg->va);
				return -1;
			}
			p_msg->pa = pa;
		}
		break;
	case NVTMPP_IOC_VB_GET_USER_VA_INFO: {
			NVTMPP_IOC_VB_GET_USER_VA_INFO_S    *p_msg = (NVTMPP_IOC_VB_GET_USER_VA_INFO_S *)p_arg;
			uintptr_t                            pa;

			pa = vos_cpu_get_phy_addr((VOS_ADDR)p_msg->va);
			if (pa == VOS_ADDR_INVALID) {
				DBG_ERR("Err va = 0x%lx\r\n", (ULONG)p_msg->va);
				p_msg->rtn = NVTMPP_ER_PARM;
			} else {
				p_msg->rtn = NVTMPP_ER_OK;
			}
			p_msg->pa = pa;
			p_msg->cached = 1;
		}
		break;
	case NVTMPP_IOC_VB_GET_BRIDGE_MEM: {
			MEM_RANGE mem_range = {0};
			NVTMPP_GET_SYSMEM_REGION_S *p_msg = (NVTMPP_GET_SYSMEM_REGION_S *)p_arg;

			if (FALSE == nvtmpp_vb_get_bridge_mem(&mem_range)) {
				p_msg->rtn = NVTMPP_ER_POOL_UNEXIST;
			} else {
				p_msg->phys_addr = mem_range.addr;
				p_msg->size = mem_range.size;
				p_msg->rtn = NVTMPP_ER_OK;
			}
		}
		break;
	case NVTMPP_IOC_VB_GET_MAX_FREE_BLK_SZ: {
			NVTMPP_IOC_VB_GET_MAX_FREE_S       *p_msg = (NVTMPP_IOC_VB_GET_MAX_FREE_S *)p_arg;

			p_msg->size = nvtmpp_vb_get_max_free_size((unsigned int)p_msg->ddr);
		}
		break;
	case NVTMPP_IOC_VB_GET_COMM_POOL_RANGE: {
			MEM_RANGE                           mem_range = {0};
			NVTMPP_GET_COMM_POOL_RANGE_S       *p_msg = (NVTMPP_GET_COMM_POOL_RANGE_S *)p_arg;

			if (FALSE == nvtmpp_vb_get_comm_pool_range(p_msg->ddr, &mem_range)) {
				p_msg->rtn = NVTMPP_ER_POOL_UNEXIST;
			} else {
				p_msg->phys_addr = mem_range.addr;
				p_msg->size = mem_range.size;
				p_msg->rtn = NVTMPP_ER_OK;
			}
		}
		break;
	case NVTMPP_IOC_VB_GET_FREE_SZ: {
			NVTMPP_IOC_VB_GET_FREE_S       *p_msg = (NVTMPP_IOC_VB_GET_FREE_S *)p_arg;

			p_msg->size = nvtmpp_heap_get_free_size((unsigned int)p_msg->ddr);
		}
		break;
	case NVTMPP_IOC_VB_LOCK_BLK: {
			NVTMPP_IOC_VB_LOCK_BLK_S *p_msg = (NVTMPP_IOC_VB_LOCK_BLK_S *)p_arg;

			p_msg->rtn = nvtmpp_vb_lock_block(USER_MODULE, p_msg->blk);
		}
		break;
	default:
		DBG_ERR("unknown cmd 0x%x\r\n", cmd);
		return -1;
	}
	return 0;
}

#endif

