#ifndef _ISE_DRV_UTIL_INT_H_
#define _ISE_DRV_UTIL_INT_H_

#include "ise_drv_platform_int.h"
#include "ise_drv_dbg_int.h"
#include "ise_drv_mm_int.h"
#include "ise_eng.h"
#include "kdrv_ise_ctl.h"

#define _ISE_VER_TOKEN_STR(a)  #a
#define ISE_VER_TOKEN_STR(a)  _ISE_VER_TOKEN_STR(a)

#define ISE_DRV_NAME "kdrv_ise"
#define ISE_VER_MAJOR 1
#define ISE_VER_MINOR 00
#define ISE_VER_PATCH 004
#define ISE_VER_EXT 00
#define ISE_DRV_MODULE_VERSION ISE_VER_TOKEN_STR(ISE_VER_MAJOR) "." ISE_VER_TOKEN_STR(ISE_VER_MINOR) "." ISE_VER_TOKEN_STR(ISE_VER_PATCH) "." ISE_VER_TOKEN_STR(ISE_VER_EXT)

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
#define ise_drv_nvtpcie_downstream_active nvtpcie_downstream_active
#define ise_drv_nvtpcie_get_pcie_addr nvtpcie_get_pcie_addr
#define ise_drv_nvtpcie_get_ddrid nvtpcie_get_ddrid
#else
#define ise_drv_nvtpcie_downstream_active(i) (0)
#define ise_drv_nvtpcie_get_pcie_addr(loc_chipid, tar_ddrid, tar_pa) (tar_pa)
#define ise_drv_nvtpcie_get_ddrid(loc_chipid, loc_pa) (0)
#endif

enum ISE_DRV_UTI_ID {
	ISE_DRV_UTI_ID_1 = 0,
	ISE_DRV_UTI_ID_2,
	ISE_DRV_UTI_ID_MAX,
};

extern u16 ise_drv_ctl_get_max_proc_num(void);
extern u16 ise_drv_module_get_max_job_q_num(void);
extern void ise_drv_eng_set_chip_num(u16 num);
extern int ise_drv_eng_get_chip_num(void);
extern int ise_drv_eng_get_eng_num(void);
extern u16 ise_drv_ctl_clk_type;

#define ISE_DRV_ALIGN_ROUNDDOWN ALIGN_FLOOR
#define ISE_DRV_ALIGN_ROUNDUP ALIGN_CEIL
#define ISE_DRV_ALIGN_ROUND ALIGN_ROUND

#define ISE_MTN_SIZE_PER_COL(img_h) (((img_h + 7) / 8) * 32)

#define ISE_IDX_TO_ENG_NUM(idx) (idx % ise_drv_eng_get_eng_num())
#define ISE_IDX_TO_CHIP_NUM(idx) ((idx / ise_drv_eng_get_eng_num()) % ise_drv_eng_get_chip_num())

#define ISE_CHIP_ENG_TO_IDX(chip_id, eng_id) ((chip_id * ise_drv_eng_get_eng_num()) + eng_id)

ISE_ENG_SCALE_METHOD ise_drv_uti_conv2_scale_method(enum ise_drv_scale_method_sel method);
ISE_ENG_IO_FMT ise_drv_uti_conv2_fmt(enum ise_drv_fmt fmt);
int ise_drv_uti_is_ycc_mode(enum ise_drv_fmt fmt);
int ise_drv_uti_get_ch_num(enum ise_drv_fmt fmt);
struct ise_drv_roi ise_drv_uti_get_min_lofs(struct ise_drv_roi roi, enum ise_drv_fmt fmt);

struct ise_drv_uti_buf_info {
	int size[2];
};
struct ise_drv_uti_buf_info ise_drv_uti_get_buf_size(struct ise_drv_roi roi, enum ise_drv_fmt fmt);

const char* ise_drv_uti_get_sts_str(unsigned int val);
const char* ise_drv_uti_get_fmt_str(unsigned int val);
const char* ise_drv_uti_get_scale_method_str(unsigned int val);
unsigned long ise_drv_uti_get_ms(void);
unsigned long ise_drv_uti_get_us(void);
#define ISE_DRV_UTI_CHK_MSG(msg) (msg != 0 ? msg : "unknown")

unsigned int ise_drv_util_read_reg(void *vbase, unsigned long ofs);
void *ise_drv_util_malloc(size_t size);
void ise_drv_util_mfree(void *addr);

#if defined(__LINUX)
int ise_drv_uti_proc_buffer_to_int(const char __user *buffer, size_t count, int *dst, int dst_cnt);

#else
#define atomic_inc(x) __sync_add_and_fetch(x, 1)
#define atomic_dec(x) __sync_sub_and_fetch(x, 1)
#define atomic_read(x) __sync_add_and_fetch(x, 0)
#define atomic_set(x, v) __sync_bool_compare_and_swap(x, *x, v)
#define atomic_add(i, x) __sync_add_and_fetch(x, i)
#define atomic_sub(i, x) __sync_sub_and_fetch(x, i)
#endif

#endif //_ISE_DRV_UTIL_INT_H_
