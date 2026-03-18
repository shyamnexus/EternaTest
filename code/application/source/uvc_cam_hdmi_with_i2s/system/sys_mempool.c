#include <hdal.h>
#include "sys_mempool.h"
#include "vendor_common.h"

uintptr_t mempool_msdcnvt = 0;
uintptr_t mempool_msdcnvt_pa = 0;

void mempool_init(void)
{
	void                 *va;
	uintptr_t            pa;
	HD_RESULT            ret;

	if (mempool_msdcnvt == 0) {
		ret = vendor_common_mem_alloc_fixed_pool("msdcnvt", &pa, (void **)&va, POOL_SIZE_MSDCNVT, DDR_ID0);
		if (ret != HD_OK) {
			return;
		}
		mempool_msdcnvt = (uintptr_t)va;
		mempool_msdcnvt_pa = (uintptr_t)pa;
	}
}