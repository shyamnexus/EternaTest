#ifndef _VPE_DRV_UTIL_INT_H_
#define _VPE_DRV_UTIL_INT_H_

#include "vpe_drv_platform_int.h"
#include "vpe_drv_dbg_int.h"
#include "vpe_drv_mm_int.h"
#include "vpe_eng.h"
#include "vpe_drv_ctl.h"

#define _VPE_VER_TOKEN_STR(a)  #a
#define VPE_VER_TOKEN_STR(a)  _VPE_VER_TOKEN_STR(a)

#define VPE_DRV_NAME "kdrv_vpe"
#define VPE_VER_MAJOR 1
#define VPE_VER_MINOR 01
#define VPE_VER_PATCH 006
#define VPE_VER_EXT 00
#define VPE_DRV_MODULE_VERSION VPE_VER_TOKEN_STR(VPE_VER_MAJOR) "." VPE_VER_TOKEN_STR(VPE_VER_MINOR) "." VPE_VER_TOKEN_STR(VPE_VER_PATCH) "." VPE_VER_TOKEN_STR(VPE_VER_EXT)

#define PROC_DET_CMD ENABLE
#define PROC_PUT_JOB DISABLE
#define VIDEO_GRAPH_DUMP_REG_ALL ENABLE
#define KDRV_PUT_JOB DISABLE
#if defined(__LINUX)
#define VIDEO_GRAPH ENABLE
#else
#define VIDEO_GRAPH DISABLE
#endif

#ifdef CONFIG_NVT_PCIE_LIB
#define vpe_drv_nvtpcie_downstream_active nvtpcie_downstream_active
#define vpe_drv_nvtpcie_get_pcie_addr nvtpcie_get_pcie_addr
#define vpe_drv_nvtpcie_get_ddrid nvtpcie_get_ddrid
#else
#define vpe_drv_nvtpcie_downstream_active(i) (0)
#define vpe_drv_nvtpcie_get_pcie_addr(loc_chipid, tar_ddrid, tar_pa) (tar_pa)
#define vpe_drv_nvtpcie_get_ddrid(loc_chipid, loc_pa) (0)
#endif

enum VPE_DRV_UTI_ID {
	VPE_DRV_UTI_ID_1 = 0,
	VPE_DRV_UTI_ID_MAX,
};

extern u16 vpe_drv_ctl_get_eng_col_mode(void);
extern u16 vpe_drv_ctl_get_max_proc_num(void);
extern u16 vpe_drv_module_get_max_job_q_num(void);
extern void vpe_drv_eng_set_chip_num(u16 num);
extern int vpe_drv_eng_get_chip_num(void);
extern int vpe_drv_eng_get_eng_num(void);
extern int vpe_drv_eng_get_eng_mask(void);
extern u16 vpe_drv_ctl_clk_type;

#define VPE_DRV_ALIGN_ROUNDDOWN ALIGN_FLOOR
#define VPE_DRV_ALIGN_ROUNDUP ALIGN_CEIL
#define VPE_DRV_ALIGN_ROUND ALIGN_ROUND

//y size
#define VPE_DRV_UTI_GET_420_Y_SZ(w,h)                 (VPE_DRV_ALIGN_ROUNDUP(w, 16) * VPE_DRV_ALIGN_ROUNDUP(h, 2))
#define VPE_DRV_UTI_GET_420YCC_Y_SZ(w,h)              (VPE_DRV_ALIGN_ROUNDUP(w, 32) * VPE_DRV_ALIGN_ROUNDUP(h, 2) * 3 / 4)

//y line offset
#define VPE_DRV_UTI_GET_422_Y_LOFS(w)               (VPE_DRV_ALIGN_ROUNDUP(w, 8) * 2)
#define VPE_DRV_UTI_GET_420_Y_LOFS(w)               (VPE_DRV_ALIGN_ROUNDUP(w, 16))
#define VPE_DRV_UTI_GET_420YCC_Y_LOFS(w)            (VPE_DRV_ALIGN_ROUNDUP((VPE_DRV_ALIGN_ROUNDUP(w, 32) * 3 / 4), 16))

//uv line offset
#define VPE_DRV_UTI_GET_420_UV_LOFS(w)               (VPE_DRV_ALIGN_ROUNDUP(w, 16))
#define VPE_DRV_UTI_GET_420YCC_UV_LOFS(w)            (VPE_DRV_ALIGN_ROUNDUP((VPE_DRV_ALIGN_ROUNDUP(w, 32) * 3 / 4), 16))

#define VPE_IDX_TO_ENG_NUM(idx) (idx % vpe_drv_eng_get_eng_num())
#define VPE_IDX_TO_CHIP_NUM(idx) ((idx / vpe_drv_eng_get_eng_num()) % vpe_drv_eng_get_chip_num())

#define VPE_CHIP_ENG_TO_IDX(chip_id, eng_id) ((chip_id * vpe_drv_eng_get_eng_num()) + eng_id)

const char* vpe_drv_uti_get_sts_str(unsigned int val);
const char* vpe_drv_uti_get_fmt_str(unsigned int val);
const char* vpe_drv_uti_get_lut2d_rot_str(unsigned int val);
int vpe_drv_uti_get_y_buf_sz(unsigned int w, unsigned int h, unsigned int type);
int vpe_drv_uti_get_y_line_offset(unsigned int w, unsigned int type);
int vpe_drv_uti_get_uv_line_offset(unsigned int w, unsigned int type);
unsigned long vpe_drv_uti_get_uv_addr(unsigned long addr, unsigned int w, unsigned int h, unsigned int type);

unsigned long vpe_drv_uti_get_ms(void);
unsigned long vpe_drv_uti_get_us(void);
#define VPE_DRV_UTI_CHK_MSG(msg) (msg != 0 ? msg : "unknown")

VPE_ENG_DRT vpe_drv_uti_conv2_drt(enum vpe_drv_drt drt);
VPE_ENG_SRC_FMT vpe_drv_uti_fmt_conv2_src_fmt(enum vpe_drv_fmt fmt);
VPE_ENG_DES_FMT vpe_drv_uti_fmt_conv2_dst_fmt(enum vpe_drv_fmt fmt);
unsigned int vpe_drv_util_read_reg(void *vbase, unsigned long ofs);
void *vpe_drv_util_malloc(size_t size);
void vpe_drv_util_mfree(void *addr);
unsigned int vpe_drv_util_cal_bit_nums(unsigned int val);

#if defined(__LINUX)
int vpe_drv_uti_proc_buffer_to_int(const char __user *buffer, size_t count, int *dst, int dst_cnt);

#else
#define atomic_inc(x) __sync_add_and_fetch(x, 1)
#define atomic_dec(x) __sync_sub_and_fetch(x, 1)
#define atomic_read(x) __sync_add_and_fetch(x, 0)
#define atomic_set(x, v) __sync_bool_compare_and_swap(x, *x, v)
#define atomic_add(i, x) __sync_add_and_fetch(x, i)
#define atomic_sub(i, x) __sync_sub_and_fetch(x, i)
#endif

#endif //_VPE_DRV_UTIL_INT_H_
