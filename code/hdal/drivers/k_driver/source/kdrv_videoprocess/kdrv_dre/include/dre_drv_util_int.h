#ifndef _DRE_DRV_UTIL_INT_H_
#define _DRE_DRV_UTIL_INT_H_

#include "dre_drv_platform_int.h"
#include "dre_drv_dbg_int.h"
#include "dre_drv_mm_int.h"
#include "dre_eng.h"
#include "kdrv_dre_ctl.h"

#define _DRE_VER_TOKEN_STR(a)  #a
#define DRE_VER_TOKEN_STR(a)  _DRE_VER_TOKEN_STR(a)

#define DRE_DRV_NAME "kdrv_dre"
#define DRE_VER_MAJOR 1
#define DRE_VER_MINOR 00
#define DRE_VER_PATCH 01
#define DRE_VER_EXT 00
#define DRE_DRV_MODULE_VERSION DRE_VER_TOKEN_STR(DRE_VER_MAJOR) "." DRE_VER_TOKEN_STR(DRE_VER_MINOR) "." DRE_VER_TOKEN_STR(DRE_VER_PATCH) "." DRE_VER_TOKEN_STR(DRE_VER_EXT)

#define PROC_DET_CMD ENABLE
#define PROC_PUT_JOB DISABLE
#define VIDEO_GRAPH_DUMP_REG_ALL ENABLE
#define KDRV_PUT_JOB DISABLE
#define KDRV_JOB_QUEUE_TYPE STATIC_MODE
#if defined(__LINUX)
#define VIDEO_GRAPH ENABLE
#else
#define VIDEO_GRAPH DISABLE
#endif

#ifdef CONFIG_NVT_PCIE_LIB
#define dre_drv_nvtpcie_downstream_active nvtpcie_downstream_active
#define dre_drv_nvtpcie_get_pcie_addr nvtpcie_get_pcie_addr
#define dre_drv_nvtpcie_get_ddrid nvtpcie_get_ddrid
#else
#define dre_drv_nvtpcie_downstream_active(i) (0)
#define dre_drv_nvtpcie_get_pcie_addr(loc_chipid, tar_ddrid, tar_pa) (tar_pa)
#define dre_drv_nvtpcie_get_ddrid(loc_chipid, loc_pa) (0)
#endif

enum DRE_DRV_UTI_ID {
	DRE_DRV_UTI_ID_1 = 0,
	DRE_DRV_UTI_ID_MAX,
};

extern u16 dre_drv_ctl_get_max_proc_num(void);
extern u16 dre_drv_module_get_max_job_q_num(void);
extern void dre_drv_eng_set_chip_num(u16 num);
extern int dre_drv_eng_get_chip_num(void);
extern int dre_drv_eng_get_eng_num(void);

#define DRE_DRV_ALIGN_ROUNDDOWN ALIGN_FLOOR
#define DRE_DRV_ALIGN_ROUNDUP ALIGN_CEIL
#define DRE_DRV_ALIGN_ROUND ALIGN_ROUND

#define DRE_IDX_TO_ENG_NUM(idx) (idx % dre_drv_eng_get_eng_num())
#define DRE_IDX_TO_CHIP_NUM(idx) ((idx / dre_drv_eng_get_eng_num()) % dre_drv_eng_get_chip_num())

#define DRE_CHIP_ENG_TO_IDX(chip_id, eng_id) ((chip_id * dre_drv_eng_get_eng_num()) + eng_id)

const char* dre_drv_uti_get_sts_str(unsigned int val);
unsigned long dre_drv_uti_get_ms(void);
unsigned long dre_drv_uti_get_us(void);
#define DRE_DRV_UTI_CHK_MSG(msg) (msg != 0 ? msg : "unknown")

unsigned int dre_drv_util_read_reg(void *vbase, unsigned long ofs);
void *dre_drv_util_malloc(size_t size);
void dre_drv_util_mfree(void *addr);
BOOL dre_drv_util_is_539a(void);

#if defined(__LINUX)
int dre_drv_uti_proc_buffer_to_int(const char __user *buffer, size_t count, int *dst, int dst_cnt);

#else
#define atomic_inc(x) __sync_add_and_fetch(x, 1)
#define atomic_dec(x) __sync_sub_and_fetch(x, 1)
#define atomic_read(x) __sync_add_and_fetch(x, 0)
#define atomic_set(x, v) __sync_bool_compare_and_swap(x, *x, v)
#define atomic_add(i, x) __sync_add_and_fetch(x, i)
#define atomic_sub(i, x) __sync_sub_and_fetch(x, i)
#endif

#endif //_DRE_DRV_UTIL_INT_H_
