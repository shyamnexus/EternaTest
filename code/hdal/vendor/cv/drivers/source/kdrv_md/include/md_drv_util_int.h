#ifndef _MD_DRV_UTIL_INT_H_
#define _MD_DRV_UTIL_INT_H_

#include "md_drv_platform_int.h"
#include "md_drv_dbg_int.h"
#include "md_drv_mm_int.h"
#include "md_eng.h"
#include "kdrv_md_ctl.h"

#define _MD_VER_TOKEN_STR(a)  #a
#define MD_VER_TOKEN_STR(a)  _MD_VER_TOKEN_STR(a)

#define MD_DRV_NAME "kdrv_mdbc"
#define MD_VER_MAJOR 1
#define MD_VER_MINOR 00
#define MD_VER_PATCH 04
#define MD_VER_EXT 00
#define MD_DRV_MODULE_VERSION MD_VER_TOKEN_STR(MD_VER_MAJOR) "." MD_VER_TOKEN_STR(MD_VER_MINOR) "." MD_VER_TOKEN_STR(MD_VER_PATCH) "." MD_VER_TOKEN_STR(MD_VER_EXT)

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
#define md_drv_nvtpcie_downstream_active nvtpcie_downstream_active
#define md_drv_nvtpcie_get_pcie_addr nvtpcie_get_pcie_addr
#define md_drv_nvtpcie_get_ddrid nvtpcie_get_ddrid
#else
#define md_drv_nvtpcie_downstream_active(i) (0)
#define md_drv_nvtpcie_get_pcie_addr(loc_chipid, tar_ddrid, tar_pa) (tar_pa)
#define md_drv_nvtpcie_get_ddrid(loc_chipid, loc_pa) (0)
#endif

enum MD_DRV_UTI_ID {
	MD_DRV_UTI_ID_1 = 0,
	MD_DRV_UTI_ID_MAX,
};

extern u16 md_drv_ctl_get_max_proc_num(void);
extern u16 md_drv_module_get_max_job_q_num(void);
extern void md_drv_eng_set_chip_num(u16 num);
extern int md_drv_eng_get_chip_num(void);
extern int md_drv_eng_get_eng_num(void);
extern u16 md_drv_ctl_clk_type;

#define MD_DRV_ALIGN_ROUNDDOWN ALIGN_FLOOR
#define MD_DRV_ALIGN_ROUNDUP ALIGN_CEIL
#define MD_DRV_ALIGN_ROUND ALIGN_ROUND

#define MD_IDX_TO_ENG_NUM(idx) (idx % md_drv_eng_get_eng_num())
#define MD_IDX_TO_CHIP_NUM(idx) ((idx / md_drv_eng_get_eng_num()) % md_drv_eng_get_chip_num())

#define MD_CHIP_ENG_TO_IDX(chip_id, eng_id) ((chip_id * md_drv_eng_get_eng_num()) + eng_id)

const char* md_drv_uti_get_sts_str(unsigned int val);
unsigned long md_drv_uti_get_ms(void);
unsigned long md_drv_uti_get_us(void);
#define MD_DRV_UTI_CHK_MSG(msg) (msg != 0 ? msg : "unknown")

unsigned int md_drv_util_read_reg(void *vbase, unsigned long ofs);
void *md_drv_util_malloc(size_t size);
void md_drv_util_mfree(void *addr);
#if defined(__LINUX)
int md_drv_uti_proc_buffer_to_int(const char __user *buffer, size_t count, int *dst, int dst_cnt);
unsigned long md_util_copy_from_user(void *to, const void __user *from, unsigned long n);
unsigned long md_util_copy_to_user(void __user *to, const void *from, unsigned long n);
#else
#define atomic_inc(x) __sync_add_and_fetch(x, 1)
#define atomic_dec(x) __sync_sub_and_fetch(x, 1)
#define atomic_read(x) __sync_add_and_fetch(x, 0)
#define atomic_set(x, v) __sync_bool_compare_and_swap(x, *x, v)
#define atomic_add(i, x) __sync_add_and_fetch(x, i)
#define atomic_sub(i, x) __sync_sub_and_fetch(x, i)
unsigned long md_util_copy_from_user(void *to, const void *from, unsigned long n);
unsigned long md_util_copy_to_user(void *to, const void *from, unsigned long n);
#endif

#endif //_MD_DRV_UTIL_INT_H_
