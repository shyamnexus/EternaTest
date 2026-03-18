#ifndef __KDRV_IPP_INT_DBG_H_
#define __KDRV_IPP_INT_DBG_H_

#define __MODULE__	kdrv_ipp
#define __DBGLVL__	8 // 0=FATAL, 1=ERR, 2=WRN, 3=UNIT, 4=FUNC, 5=IND, 6=MSG, 7=VALUE, 8=USER
#include "kwrap/debug.h"
#include "kwrap/perf.h"
#include "kwrap/error_no.h"
/*For debug*/
#include "kdrv_ipp_dbg_log.h"

extern unsigned int kdrv_ipp_debug_level;

#define KDRV_IPP_FREQ_MSG_EN (1)

#if defined(__LINUX)
#include "comm/util/log.h"
#define ipp_pr_warn pr_warn
#define ipp_printm printm
extern void kdrv_ipp_proc_seq_printf(void *sfile, const char *fmt, ...);
#elif defined(__FREERTOS)
#include <kwrap/debug.h>
#include <stdarg.h>
#define ipp_pr_warn debug_msg_isr
#define kdrv_ipp_proc_seq_printf(sfile, fmt, args...) ipp_pr_warn(fmt, ##args)
#define ipp_printm(module, fmt, ...)
#endif
#define KDRV_IPP_LOG_TAG "VD"
#define nvt_ipp_dbg_log(fmt, args...) \
do { \
	ipp_pr_warn(fmt, ##args); \
	ipp_printm(KDRV_IPP_LOG_TAG, fmt, ##args); \
} while (0)
#define kdrv_ipp_uti_log kdrv_ipp_proc_seq_printf


#if (KDRV_IPP_FREQ_MSG_EN == 1)
#define KDRV_IPP_DBG_ERR_FREQ(cnt,args...) \
	do{ \
		static UINT16 i=0; \
		if(i == 0){ DBG_ERR(args); if(cnt!=0)i++; } \
		else if(i == cnt) i = 0; \
		else i++; \
	}while(0)
#else
#define KDRV_IPP_DBG_ERR_FREQ(cnt,args...) do{ DBG_ERR(args); }while(0)
#endif


typedef struct{
	UINT32  chip_id;
	UINT32  eng_id;
}KDRV_IPP_DBG_CMD_HDL;

typedef enum {
	KDRV_IPP_DBG_DTSI_CHECK_CHECKSUM = 0,
	KDRV_IPP_DBG_PARAM_ID_MAX
} KDRV_IPP_DBG_PARAM_ID;

typedef enum{
	KDRV_IPP_DBG_DTSI_CHK       = 0x00000001,
}KDRV_IPP_DBG_FUNC;

typedef struct{
	UINT32 ipe_ovlp;
	UINT32 ipe_alignment;

	UINT32 ime_ovlp;
	UINT32 ime_ovlp_msb;
	UINT32 ime_alignment;

	UINT32 ime_strp_h_l;
	UINT32 ime_strp_h_n;
	UINT32 ime_strp_h_m;
	UINT32 ime_strp_v_l;
	UINT32 ime_strp_v_n;
	UINT32 ime_strp_v_m;

	UINT32 dce_strp_rule;
	UINT32 drm_out_fail_cnt;
	UINT32 old_drm_out_fail_cnt;
	UINT32 dma_off_drm_out_fail_cnt;
	UINT32 skip_check_cnt;
}KDRV_IPP_DBG_INFO;

typedef struct{
	KDRV_IPP_DBG_FUNC ipp_dbg_fun;
}KDRV_IPP_DBG_CTL;

/* dump global information */
void kdrv_ipp_dbg_dump(void);

/* dump job information */
void kdrv_ipp_dbg_dump_cfg(char *name, void *p_data, int (*dump)(const char *fmt, ...));
void kdrv_ipp_dbg_dump_ll(void *p_data);
void kdrv_ipp_dbg_dump_job(void *p_data);
void kdrv_ipp_dbg_dump_register(void *p_data);
void kdrv_ipp_dbg_dump_fastboot(void *p_data);
UINT8 kdrv_ipp_dbg_job(UINT8 op, UINT8 val);
void kdrv_ipp_dbg_fastboot_pause_stream_en(BOOL en);
void kdrv_ipp_dbg_fastboot_bypass_algo_en(BOOL en);
void kdrv_ipp_dbg_fastboot_stamp_mode_en(BOOL en);
UINT32 kdrv_ipp_dbg_func_en(UINT32 id, UINT32 dbg_func_en, BOOL en);

/* dump timestamp */
void kdrv_ipp_dbg_dump_job_ts(void *p_data);
UINT8 kdrv_ipp_dbg_timestamp(UINT8 op, UINT8 val);

#endif	//__KDRV_IPP_DBG_INT_H_

