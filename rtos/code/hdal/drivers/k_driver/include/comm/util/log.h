/*
 *   @file   log.h
 *
 *   @brief  log header file.
 *
 *   Here defines some APIs for log purpose.
 *
 *   Copyright   Novatek Microelectronics Corp. 2018.  All rights reserved.
 */

#ifndef _LOG_H_
#define _LOG_H_
//#include <linux/ioctl.h>
//#include <linux/version.h>
//#include <linux/types.h>
#include <kwrap/nvt_type.h>
//#define MEM_LEAK_TRACE
#ifdef MEM_LEAK_TRACE
#ifndef _LOG_C_
#include "dbg_wrapper.h"
#endif
#endif

#define MAX_STRING_LEN  40

#define IOCTL_PRINTM                0x9969
#define IOCTL_PRINTM_WITH_PANIC     0x9970
#define IOCTL_SET_HDAL_VERSION      0x9971
#define IOCTL_SET_IMPL_VERSION      0x9972
#define IOCTL_GET_HDAL_DBGMODE      0x9973

#define FLOW_DISABLE_BIT       (0)
#define FLOW_ERR_BIT           (1)
#define FLOW_ALL_BIT           (2)


#define FLOW_CAP_BIT           (6)
#define FLOW_VPE_BIT           (7)
#define FLOW_VOUT_BIT          (8)
#define FLOW_DEC_BIT           (9)
#define FLOW_ENC_BIT           (10)
#define FLOW_AUDIO_BIT         (11)

#define FLOW_LV_BIT            (16)
#define FLOW_REC_BIT           (17)
#define FLOW_PB_BIT            (18)
#define FLOW_CLEARWIN_BIT      (19)
#define FLOW_NR_BIT            (20)
#define FLOW_MD_BIT            (21)
#define FLOW_CROP_BIT          (22)
#define FLOW_TRIGGER_BIT       (23)
#define FLOW_GFX_BIT           (24)
#define FLOW_OSG_BIT           (25)
#define FLOW_COMMON_BIT        (26)
#define FLOW_VENDOR_BIT        (27)


#define FLOW_DISABLE_FLAG      (0x1 << FLOW_DISABLE_BIT)
#define FLOW_ERR_FLAG          (0x1 << FLOW_ERR_BIT)
#define FLOW_ALL_FLAG          (0x1 << FLOW_ALL_BIT)

#define FLOW_CAP_FLAG          (0x1 << FLOW_CAP_BIT)
#define FLOW_VPE_FLAG          (0x1 << FLOW_VPE_BIT)
#define FLOW_VOUT_FLAG         (0x1 << FLOW_VOUT_BIT)
#define FLOW_DEC_FLAG          (0x1 << FLOW_DEC_BIT)
#define FLOW_ENC_FLAG          (0x1 << FLOW_ENC_BIT)
#define FLOW_AUDIO_FLAG        (0x1 << FLOW_AUDIO_BIT)

#define FLOW_LV_FLAG           (0x1 << FLOW_LV_BIT)
#define FLOW_REC_FLAG          (0x1 << FLOW_REC_BIT)
#define FLOW_PB_FLAG           (0x1 << FLOW_PB_BIT)
#define FLOW_CLEARWIN_FLAG     (0x1 << FLOW_CLEARWIN_BIT)
#define FLOW_NR_FLAG           (0x1 << FLOW_NR_BIT)
#define FLOW_MD_FLAG           (0x1 << FLOW_MD_BIT)
#define FLOW_CROP_FLAG         (0x1 << FLOW_CROP_BIT)
#define FLOW_TRIGGER_FLAG      (0x1 << FLOW_TRIGGER_BIT)
#define FLOW_GFX_FLAG          (0x1 << FLOW_GFX_BIT)
#define FLOW_OSG_FLAG          (0x1 << FLOW_OSG_BIT)
#define FLOW_COMMON_FLAG       (0x1 << FLOW_COMMON_BIT)
#define FLOW_VENDOR_FLAG       (0x1 << FLOW_VENDOR_BIT)


typedef enum _HDAL_FLOW_DBG_MODULE {
	HDAL_FLOW_DBG_COMM = 0,
	HDAL_FLOW_DBG_VCAP,
	HDAL_FLOW_DBG_VPRC,
	HDAL_FLOW_DBG_VOUT,
	HDAL_FLOW_DBG_VENC,
	HDAL_FLOW_DBG_VDEC,
	HDAL_FLOW_DBG_ACAP,
	HDAL_FLOW_DBG_AOUT,
	HDAL_FLOW_DBG_AENC,
	HDAL_FLOW_DBG_ADEC,
	HDAL_FLOW_DBG_GFX,
	HDAL_FLOW_DBG_MAX,
	E_HDAL_FLOW_DBG = 0x10000000
} HDAL_FLOW_DBG_MODULE;

typedef struct {
	unsigned int ddr_id;
	uintptr_t pa;
	unsigned int size;
	char filename[50];
} dump_info_t;

int register_panic_notifier(int (*func)(uintptr_t));
int register_printout_notifier(int (*func)(uintptr_t));
int register_master_print_notifier(int (*func)(uintptr_t));
int register_hdal_proc_notifier(int (*func)(uintptr_t));
int register_hdal_flow_notifier(int (*func)(uintptr_t));
int register_gmlib_flow_notifier(int (*func)(uintptr_t));
int damnit(char *module);
void printm(char *module, const char *fmt, ...);
int  printm2(const char *fmt, ...);
void master_print(const char *fmt, ...);
void dumpbuf_pa(dump_info_t *dump_info, unsigned int counts, char *path);
void dumpbuf_va(int ddr_id, uintptr_t va, unsigned int size, char *filename, char *path);
void register_version(char *);
int unregister_printout_notifier(int (*func)(uintptr_t));
int unregister_panic_notifier(int (*func)(uintptr_t));
int dumplog(char *module);
int get_cpu_state(void);
int set_hdal_flow_dbglevel(HDAL_FLOW_DBG_MODULE module, unsigned int module_dbg_lvl);
int get_hdal_flow_dbglevel(HDAL_FLOW_DBG_MODULE module);


#if defined(__FREERTOS)
int log_init(void *p_param)  __attribute__ ((section (".init.table.")));
int log_exit(void *p_param)  __attribute__ ((section (".exit.table.")));
void *log_mmap(void *p_addr, unsigned int len, int prot, int flags, int fd, unsigned int offset);
int log_munmap(void *p_addr, unsigned int len);
int log_ioctl(int fd, unsigned int cmd, void *p_arg);
#endif

#define VG_ASSERT(condition)\
	do{\
		if(unlikely(0 == (condition))){\
			printk("%s %d:assert error!\n", __FUNCTION__, __LINE__);\
			damnit("VG");\
		}\
	}while(0)


#endif
