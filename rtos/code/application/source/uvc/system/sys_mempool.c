#include <hdal.h>
#include <vendor_common.h>
#include "sys_mempool.h"

UINT32 mempool_storage_sdio = 0;
UINT32 mempool_filesys = 0;
UINT32 mempool_fdtapp = 0;
UINT32 mempool_pstore = 0;

void mempool_init(void)
{
#if 0
#if (_PACKAGE_SDCARD_)
	if (vendor_common_mem_alloc_fixed_pool("sdio", &pa, (void **)&va, POOL_SIZE_STORAGE_SDIO, DDR_ID0) == HD_OK) {
		mempool_storage_sdio = (UINT32)va;
	}
#endif
#if (_PACKAGE_FILESYS_)
	if (vendor_common_mem_alloc_fixed_pool("filesys", &pa, (void **)&va, POOL_SIZE_FILESYS, DDR_ID0) == HD_OK) {
		mempool_filesys = (UINT32)va;
	}
#endif
	if (vendor_common_mem_alloc_fixed_pool("fdapp", &pa, (void **)&va, POOL_SIZE_FDTAPP, DDR_ID0) == HD_OK) {
		mempool_fdtapp = (UINT32)va;
	}
	if (vendor_common_mem_alloc_fixed_pool("PStore", &pa, (void **)&va, POOL_SIZE_PS_BUFFER, DDR_ID0) == HD_OK) {
		mempool_pstore = (UINT32)va;
	}
#else
#if (_PACKAGE_SDCARD_)
	mempool_storage_sdio = (UINT32)malloc(POOL_SIZE_STORAGE_SDIO);
#endif
#if (_PACKAGE_FILESYS_)
	mempool_filesys = (UINT32)malloc(POOL_SIZE_FILESYS);
#endif
	mempool_fdtapp = (UINT32)malloc(POOL_SIZE_FDTAPP);
	mempool_pstore = (UINT32)malloc(POOL_SIZE_PS_BUFFER);
#endif

}

void mempool_init2(void)
{
#if (_PACKAGE_SDCARD_)
	mempool_storage_sdio = (UINT32)malloc(POOL_SIZE_STORAGE_SDIO);
#endif
#if (_PACKAGE_FILESYS_)
	mempool_filesys = (UINT32)malloc(POOL_SIZE_FILESYS);
#endif
	mempool_fdtapp = (UINT32)malloc(POOL_SIZE_FDTAPP);
	mempool_pstore = (UINT32)malloc(POOL_SIZE_PS_BUFFER);
}