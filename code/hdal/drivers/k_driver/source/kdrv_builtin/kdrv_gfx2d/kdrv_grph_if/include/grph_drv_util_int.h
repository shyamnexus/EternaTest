#ifndef _GRAPH_DRV_UTIL_INT_H_
#define _GRAPH_DRV_UTIL_INT_H_

#include "grph_drv_platform_int.h"
#include "grph_drv_dbg_int.h"
#include "grph_drv_mm_int.h"
#include "kdrv_grph_ctl.h"

#define _GRAPH_VER_TOKEN_STR(a)  #a
#define GRAPH_VER_TOKEN_STR(a)  _GRAPH_VER_TOKEN_STR(a)

#define GRAPH_DRV_NAME "kdrv_graph"
#define GRAPH_VER_MAJOR 1
#define GRAPH_VER_MINOR 00
#define GRAPH_VER_PATCH 002
#define GRAPH_VER_EXT 00
#define GRAPH_DRV_MODULE_VERSION GRAPH_VER_TOKEN_STR(GRAPH_VER_MAJOR) "." GRAPH_VER_TOKEN_STR(GRAPH_VER_MINOR) "." GRAPH_VER_TOKEN_STR(GRAPH_VER_PATCH) "." GRAPH_VER_TOKEN_STR(GRAPH_VER_EXT)

#define PROC_DET_CMD ENABLE
#define PROC_PUT_JOB DISABLE
#define VIDEO_GRAPH_DUMP_REG_ALL DISABLE
#define KDRV_INTERFACE ENABLE
#define KDRV_PUT_JOB DISABLE
#define KDRV_JOB_QUEUE_TYPE STATIC_DYNAMIC_MODE
#if defined(__LINUX)
#define VIDEO_GRAPH DISABLE
#else
#define VIDEO_GRAPH DISABLE
#endif

#ifdef CONFIG_NVT_PCIE_LIB
#define graph_drv_nvtpcie_downstream_active nvtpcie_downstream_active
#define graph_drv_nvtpcie_get_pcie_addr nvtpcie_get_pcie_addr
#define graph_drv_nvtpcie_get_ddrid nvtpcie_get_ddrid
#else
#define graph_drv_nvtpcie_downstream_active(i) (0)
#define graph_drv_nvtpcie_get_pcie_addr(loc_chipid, tar_ddrid, tar_pa) (tar_pa)
#define graph_drv_nvtpcie_get_ddrid(loc_chipid, loc_pa) (0)
#endif

extern u16 graph_drv_ctl_get_max_proc_num(void);
extern u16 graph_drv_module_get_max_job_q_num(void);
extern void graph_drv_eng_set_chip_num(u16 num);
extern int graph_drv_eng_get_chip_num(void);
extern int graph_drv_eng_get_eng_num(void);
extern u16 graph_drv_flow_clk_type;


#define GRAPH_DRV_ALIGN_ROUNDDOWN ALIGN_FLOOR
#define GRAPH_DRV_ALIGN_ROUNDUP ALIGN_CEIL
#define GRAPH_DRV_ALIGN_ROUND ALIGN_ROUND

#define GRAPH_IDX_TO_ENG_NUM(idx) (idx % graph_drv_eng_get_eng_num())
#define GRAPH_IDX_TO_CHIP_NUM(idx) ((idx / graph_drv_eng_get_eng_num()) % graph_drv_eng_get_chip_num())

#define GRAPH_CHIP_ENG_TO_IDX(chip_id, eng_id) ((chip_id * graph_drv_eng_get_eng_num()) + eng_id)

int graph_drv_uti_get_supt_eng(unsigned int cmd);
int graph_drv_uti_is_supt_req_fmt(GRPH_CMD cmd, GRPH_FORMAT fmt);
const char* graph_drv_uti_get_fmt_str(unsigned int val);
const char* graph_drv_uti_get_cmd_str(unsigned int val);
const char* graph_drv_uti_get_sts_str(unsigned int val);
const char* graph_drv_uti_get_img_id_str(unsigned int val);
const char* graph_drv_uti_get_io_id_str(unsigned int val);
const char* graph_drv_uti_get_pty_str(unsigned int val);
unsigned long graph_drv_uti_get_ms(void);
unsigned long graph_drv_uti_get_us(void);
#define GRAPH_DRV_UTI_CHK_MSG(msg) (msg != 0 ? msg : "unknown")

void *graph_drv_util_malloc(size_t size);
void graph_drv_util_mfree(void *addr);

#if defined(__LINUX)
int graph_drv_uti_proc_buffer_to_int(const char __user *buffer, size_t count, int *dst, int dst_cnt);

#else
#define atomic_inc(x) __sync_add_and_fetch(x, 1)
#define atomic_dec(x) __sync_sub_and_fetch(x, 1)
#define atomic_read(x) __sync_add_and_fetch(x, 0)
#define atomic_set(x, v) __sync_bool_compare_and_swap(x, *x, v)
#define atomic_add(i, x) __sync_add_and_fetch(x, i)
#define atomic_sub(i, x) __sync_sub_and_fetch(x, i)
#endif

#endif //_GRAPH_DRV_UTIL_INT_H_
