/*-----------------------------------------------------------------------------*/
/* Include Header Files                                                        */
/*-----------------------------------------------------------------------------*/
#include <linux/module.h>
#include <linux/version.h>

#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 19, 148))
#include <mach/fmem.h>
#else
#include <linux/soc/nvt/fmem.h>
#endif

#define __MODULE__    rtos_mem
#define __DBGLVL__    8 // 0=FATAL, 1=ERR, 2=WRN, 3=UNIT, 4=FUNC, 5=IND, 6=MSG, 7=VALUE, 8=USER
#define __DBGFLT__    "*"
#include <kwrap/debug.h>
#include <kwrap/mem.h>
#include "vos_ioctl.h"

/*-----------------------------------------------------------------------------*/
/* Local Types Declarations                                                    */
/*-----------------------------------------------------------------------------*/
#define RTOS_MEM_INITED_TAG       MAKEFOURCC('R', 'M', 'E', 'M') ///< a key value
#define VOS_MEM_CMA_INDEX         0

#if defined(__LINUX)

#elif defined(__FREERTOS)

#else
#error Not supported OS
#endif


/*-----------------------------------------------------------------------------*/
/* Local Global Variables                                                      */
/*-----------------------------------------------------------------------------*/
unsigned int rtos_mem_debug_level = NVT_DBG_WRN;

module_param_named(rtos_mem_debug_level, rtos_mem_debug_level, int, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(rtos_mem_debug_level, "Debug message level");

/*-----------------------------------------------------------------------------*/
/* Interface Functions                                                         */
/*-----------------------------------------------------------------------------*/
void rtos_mem_init(void *param)
{
	STATIC_ASSERT(sizeof(struct vos_mem_info_t) == sizeof(struct nvt_fmem_mem_info_t));
	STATIC_ASSERT(sizeof_field(struct vos_mem_info_t, type) == sizeof_field(struct nvt_fmem_mem_info_t, type));
	STATIC_ASSERT(sizeof_field(struct vos_mem_info_t, reserved1) == sizeof_field(struct nvt_fmem_mem_info_t, dev));
	STATIC_ASSERT(sizeof_field(struct vos_mem_info_t, paddr) == sizeof_field(struct nvt_fmem_mem_info_t, paddr));
	STATIC_ASSERT(sizeof_field(struct vos_mem_info_t, reserved2) == sizeof_field(struct nvt_fmem_mem_info_t, pages));
	STATIC_ASSERT(sizeof_field(struct vos_mem_info_t, reserved3) == sizeof_field(struct nvt_fmem_mem_info_t, page_count));
	STATIC_ASSERT(sizeof_field(struct vos_mem_info_t, vaddr) == sizeof_field(struct nvt_fmem_mem_info_t, vaddr));
	STATIC_ASSERT(sizeof_field(struct vos_mem_info_t, size) == sizeof_field(struct nvt_fmem_mem_info_t, size));
	STATIC_ASSERT((VOS_MEM_TYPE_CACHE - NVT_FMEM_ALLOC_CACHE) == 0);
	STATIC_ASSERT((VOS_MEM_TYPE_NONCACHE - NVT_FMEM_ALLOC_NONCACHE) == 0);
	STATIC_ASSERT((VOS_MEM_TYPE_BUFFER - NVT_FMEM_ALLOC_BUFFER) == 0);
}

void rtos_mem_exit(void)
{
}

int vos_mem_init_info(struct vos_mem_info_t *p_info, VOS_MEM_TYPE type, UINT32 size, void *dev)
{
	if (NULL == p_info) {
		DBG_ERR("p_info is NULL\r\n");
		return -1;
	}

	if (0 != nvt_fmem_mem_info_init((struct nvt_fmem_mem_info_t *)p_info, (nvt_fmem_alloc_type_t)type, size, dev)) {
		DBG_ERR("nvt_fmem_mem_info_init failed\r\n");
		return -1;
	}

	return 0;
}

VOS_MEM_HDL vos_mem_alloc_cma(struct vos_mem_info_t *p_info, unsigned int index)
{
	void *p_alloc;

	if (NULL == p_info) {
		DBG_ERR("p_info is NULL\r\n");
		return NULL;
	}

	if (0 == p_info->size) {
		DBG_ERR("size is 0\r\n");
		return NULL;
	}

	p_alloc = fmem_alloc_from_cma((struct nvt_fmem_mem_info_t *)p_info, index);
	if (NULL == p_alloc) {
		DBG_ERR("fmem_alloc_from_cma[%d] %ld bytes failed\r\n", index, (ULONG)p_info->size);
		return NULL;
	}

	return (VOS_MEM_HDL)p_alloc;
}

int vos_mem_release_cma(VOS_MEM_HDL p_hdl, unsigned int index)
{
	int ret;

	if (NULL == p_hdl) {
		DBG_ERR("p_hdl is NULL\r\n");
		return -1;
	}

	ret = fmem_release_from_cma((void *)p_hdl, index);
	if (0 != ret) {
		DBG_ERR("fmem_release_from_cma[%d] failed, ret %d\r\n", index, ret);
	}

	return ret;
}

VOS_MEM_HDL vos_mem_alloc_contiguous(struct vos_mem_info_t *p_info)
{
	void *p_alloc;

	if (NULL == p_info) {
		DBG_ERR("p_info is NULL\r\n");
		return NULL;
	}

	if (0 == p_info->size) {
		DBG_ERR("size is 0\r\n");
		return NULL;
	}

	if (fmem_cma_is_valid(VOS_MEM_CMA_INDEX)) {
		p_alloc = fmem_alloc_from_cma((struct nvt_fmem_mem_info_t *)p_info, VOS_MEM_CMA_INDEX);
		if (NULL == p_alloc) {
			DBG_ERR("fmem_alloc_from_cma(%d) %ld bytes failed\r\n", VOS_MEM_CMA_INDEX, (ULONG)p_info->size);
			return NULL;
		}
	} else {
		p_alloc = fmem_alloc_from_ker((struct nvt_fmem_mem_info_t *)p_info);
		if (NULL == p_alloc) {
			DBG_ERR("fmem_alloc_from_ker %ld bytes failed\r\n", (ULONG)p_info->size);
			return NULL;
		}
	}

	return (VOS_MEM_HDL)p_alloc;
}

int vos_mem_release_contiguous(VOS_MEM_HDL p_hdl)
{
	int ret;

	if (NULL == p_hdl) {
		DBG_ERR("p_hdl is NULL\r\n");
		return -1;
	}

	if (fmem_cma_is_valid(VOS_MEM_CMA_INDEX)) {
		ret = fmem_release_from_cma((void *)p_hdl, VOS_MEM_CMA_INDEX);
		if (0 != ret) {
			DBG_ERR("fmem_release_from_cma failed, ret %d\r\n", ret);
		}
	} else {
		ret = fmem_release_from_ker((void *)p_hdl);
		if (0 != ret) {
			DBG_ERR("fmem_release_from_cma failed, ret %d\r\n", ret);
		}
	}

	return ret;
}

EXPORT_SYMBOL(vos_mem_init_info);
EXPORT_SYMBOL(vos_mem_alloc_cma);
EXPORT_SYMBOL(vos_mem_release_cma);
EXPORT_SYMBOL(vos_mem_alloc_contiguous);
EXPORT_SYMBOL(vos_mem_release_contiguous);
