#ifndef _IVE_DRV_UTIL_INT_H_
#define _IVE_DRV_UTIL_INT_H_

#include "ive_drv_platform_int.h"
#include "ive_drv_dbg_int.h"
#include "ive_drv_mm_int.h"
#include "ive_eng.h"
#include "kdrv_ive_ctl.h"

#define _IVE_VER_TOKEN_STR(a)  #a
#define IVE_VER_TOKEN_STR(a)  _IVE_VER_TOKEN_STR(a)

#define IVE_DRV_NAME "kdrv_ive"
#define IVE_VER_MAJOR 1
#define IVE_VER_MINOR 01
#define IVE_VER_PATCH 00
#define IVE_VER_EXT 00
#define IVE_DRV_MODULE_VERSION IVE_VER_TOKEN_STR(IVE_VER_MAJOR) "." IVE_VER_TOKEN_STR(IVE_VER_MINOR) "." IVE_VER_TOKEN_STR(IVE_VER_PATCH) "." IVE_VER_TOKEN_STR(IVE_VER_EXT)

#define PROC_DET_CMD ENABLE
#define PROC_PUT_JOB DISABLE
#define VIDEO_GRAPH_DUMP_REG_ALL ENABLE
#define KDRV_INTERFACE ENABLE
#define KDRV_PUT_JOB DISABLE
#define KDRV_JOB_QUEUE_TYPE STATIC_MODE
#if defined(__LINUX)
#define VIDEO_GRAPH ENABLE
#else
#define VIDEO_GRAPH DISABLE
#endif

#ifdef CONFIG_NVT_PCIE_LIB
#define ive_drv_nvtpcie_downstream_active nvtpcie_downstream_active
#define ive_drv_nvtpcie_get_pcie_addr nvtpcie_get_pcie_addr
#define ive_drv_nvtpcie_get_ddrid nvtpcie_get_ddrid
#else
#define ive_drv_nvtpcie_downstream_active(i) (0)
#define ive_drv_nvtpcie_get_pcie_addr(loc_chipid, tar_ddrid, tar_pa) (tar_pa)
#define ive_drv_nvtpcie_get_ddrid(loc_chipid, loc_pa) (0)
#endif

enum IVE_DRV_UTI_ID {
	IVE_DRV_UTI_ID_1 = 0,
	IVE_DRV_UTI_ID_MAX,
};

extern u16 ive_drv_ctl_get_max_proc_num(void);
extern u16 ive_drv_module_get_max_job_q_num(void);
extern void ive_drv_eng_set_chip_num(u16 num);
extern int ive_drv_eng_get_chip_num(void);
extern int ive_drv_eng_get_eng_num(void);

#define IVE_DRV_ALIGN_ROUNDDOWN ALIGN_FLOOR
#define IVE_DRV_ALIGN_ROUNDUP ALIGN_CEIL
#define IVE_DRV_ALIGN_ROUND ALIGN_ROUND

#define IVE_MTN_SIZE_PER_COL(img_h) (((img_h + 7) / 8) * 32)

#define IVE_IDX_TO_ENG_NUM(idx) (idx % ive_drv_eng_get_eng_num())
#define IVE_IDX_TO_CHIP_NUM(idx) ((idx / ive_drv_eng_get_eng_num()) % ive_drv_eng_get_chip_num())

#define IVE_CHIP_ENG_TO_IDX(chip_id, eng_id) ((chip_id * ive_drv_eng_get_eng_num()) + eng_id)

const char* ive_drv_uti_get_sts_str(unsigned int val);
const char* ive_drv_uti_get_op_str(unsigned int val);
unsigned long ive_drv_uti_get_ms(void);
unsigned long ive_drv_uti_get_us(void);
#define IVE_DRV_UTI_CHK_MSG(msg) (msg != 0 ? msg : "unknown")

unsigned int ive_drv_util_read_reg(void *vbase, unsigned long ofs);
void *ive_drv_util_malloc(size_t size);
void ive_drv_util_mfree(void *addr);
BOOL ive_drv_util_is_539a(void);

#if defined(__LINUX)
int ive_drv_uti_proc_buffer_to_int(const char __user *buffer, size_t count, int *dst, int dst_cnt);
unsigned long ive_util_copy_from_user(void *to, const void __user *from, unsigned long n);
unsigned long ive_util_copy_to_user(void __user *to, const void *from, unsigned long n);
#else
#define atomic_inc(x) __sync_add_and_fetch(x, 1)
#define atomic_dec(x) __sync_sub_and_fetch(x, 1)
#define atomic_read(x) __sync_add_and_fetch(x, 0)
#define atomic_set(x, v) __sync_bool_compare_and_swap(x, *x, v)
#define atomic_add(i, x) __sync_add_and_fetch(x, i)
#define atomic_sub(i, x) __sync_sub_and_fetch(x, i)
unsigned long ive_util_copy_from_user(void *to, const void *from, unsigned long n);
unsigned long ive_util_copy_to_user(void *to, const void *from, unsigned long n);
#endif

#endif //_IVE_DRV_UTIL_INT_H_
