/*
    Copyright   Novatek Microelectronics Corp. 2017.  All rights reserved.

    @file       isf_audcap_int.h

    @brief      isf_audcap internal header file

    @version    V1.00.000
    @author     Novatek FW Team
    @date       2018/06/04
*/
#ifndef _ISF_AUDCAP_INT_H
#define _ISF_AUDCAP_INT_H

#ifdef __KERNEL__
#include "kwrap/error_no.h"
#include "kwrap/type.h"
#include "kwrap/platform.h"
#include "kwrap/semaphore.h"
#include "kwrap/task.h"
#include <kwrap/spinlock.h>
#include "kwrap/util.h"
#include "kwrap/cpu.h"
#include "kflow_common/nvtmpp.h"
#include "kflow_common/isf_flow_def.h"
#include "kflow_common/isf_flow_core.h"
#include "kflow_audiocapture/isf_audcap.h"
#include "kflow_audiocapture/wavstudio_tsk.h"
#include "kflow_audiocapture/ctl_aud.h"
#include "kdrv_builtin/kdrv_builtin.h"
#include "kdrv_builtin/audcap_builtin.h"
#include "../include/isf_audcap_dbg.h"
#include <linux/string.h>

#define debug_msg 			vk_printk
#else
#include "kwrap/error_no.h"
#include "kwrap/type.h"
#include "kwrap/platform.h"
#include "kwrap/semaphore.h"
#include "kwrap/task.h"
#include <kwrap/spinlock.h>
#include "kwrap/util.h"
#include "kwrap/cpu.h"
#include "kflow_common/nvtmpp.h"
#include "kflow_common/isf_flow_def.h"
#include "kflow_common/isf_flow_core.h"
#include "kflow_audiocapture/isf_audcap.h"
#include "kflow_audiocapture/wavstudio_tsk.h"
#include "kflow_audiocapture/ctl_aud.h"
#include "../include/isf_audcap_dbg.h"
#include <string.h>

#define debug_msg 			vk_printk
#define msecs_to_jiffies(x)	0
#endif

#define ISF_AUDCAP_PULL_Q_MAX       60
#define ISF_AUDCAP_IN_NUM           1
#define ISF_AUDCAP_OUT_NUM          2
#define ISF_AUDCAP_BSDATA_BLK_MAX   150

extern UINT32 g_audcap_max_count;
#define DEV_MAX_COUNT  g_audcap_max_count
#define PATH_MAX_COUNT  2

typedef struct _AUDCAP_PULL_QUEUE {
	UINT32          Front;                  ///< Front pointer
	UINT32          Rear;                   ///< Rear pointer
	UINT32          bFull;                  ///< Full flag
	ISF_DATA*       Queue;
} AUDCAP_PULL_QUEUE, *PAUDCAP_PULL_QUEUE;

typedef struct _AUDCAP_MMAP_MEM_INFO {
	ULONG           addr_virtual;           ///< memory start addr (virtual)
	ULONG           addr_physical;          ///< memory start addr (physical)
	UINT32          size;                   ///< size
} AUDCAP_MMAP_MEM_INFO, *PAUDCAP_MMAP_MEM_INFO;

typedef enum {
	AUDCAP_AUD_CODEC_EMBEDDED,
	AUDCAP_AUD_CODEC_EXTERNAL,
	ENUM_DUMMY4WORD(AUDCAP_AUD_CODEC)
} AUDCAP_AUD_CODEC;

/**
    AudFilter object
*/
typedef struct _AUDCAP_AUDFILT_OBJ {
	BOOL(*open)(UINT32 , BOOL);
	BOOL(*close)(void);
	BOOL(*apply)(UINT32, UINT32, UINT32);
	void(*design)(void);
} AUDCAP_AUDFILT_OBJ, *PAUDCAP_AUDFILT_OBJ;

typedef struct {
	PWAVSTUD_INFO_SET pAudInfoSet;
	BOOL              bRelease;
} AUDCAP_MAX_MEM_INFO, *PAUDCAP_MAX_MEM_INFO;

typedef struct {
	UINT32           blk_id;
	ULONG            buf_addr;
	UINT32           occupy;
} AUDCAP_BUF_INFO;

typedef struct {
	UINT64           frame_time;
	UINT64           block_time;
} AUDCAP_TIMESTAMP;

typedef struct {
	UINT32           refCnt;
	ULONG            hData;
	MEM_RANGE        mem;
	UINT32           blk_id;
	void            *pnext_bsdata;
	AUDCAP_BUF_INFO  *p_buf_info;
	AUDCAP_BUF_INFO  *p_remain_buf_info;
} AUDCAP_BSDATA;

typedef struct _AUDCAP_CONTEXT_PORT {
	AUDCAP_BSDATA          *g_p_audcap_bsdata_link_head;
	AUDCAP_BSDATA          *g_p_audcap_bsdata_link_tail;
	BOOL                   aec_en;
	BOOL                   anr_en;
	MEM_RANGE              g_isf_audcap_pathmem;
	AUDIO_CH               isf_audout_ch;
	AUDIO_CH               isf_audout_ch_num;
	AUDIO_SR               isf_audcap_resample_max;
	AUDIO_SR               isf_audcap_resample;
	AUDIO_SR               isf_audcap_resample_update;
	BOOL                   isf_audcap_resample_dirty;
	UINT32                 isf_audcap_resample_frame;
	BOOL                   isf_audcap_resample_en;
	UINT32                 isf_audcap_started;
	UINT32                 isf_audcap_opened;
	MEM_RANGE              isf_audcap_output_mem;
	AUDCAP_AGC_CONFIG      isf_audcap_agc_cfg;
	KDRV_AUDIO_CAP_DEFSET  isf_audcap_defset;
	INT32                  isf_audcap_ng_thd;
	AUDCAP_PULL_QUEUE      isf_audcap_pull_que;
	AUDCAP_MMAP_MEM_INFO   isf_audcap_mmap_info;
	SEM_HANDLE             ISF_AUDCAP_SEM_ID;
	SEM_HANDLE             ISF_AUDCAP_PULLQ_SEM_ID;
	BOOL                   put_pullq;
	UINT32                 volume;
	BOOL                   isf_audcap_lb_en;
	AUDCAP_AEC_CONFIG      aec_config;
} AUDCAP_CONTEXT_PORT;

typedef struct _AUDOUT_CONTEXT_DEV {
	AUDCAP_CONTEXT_PORT port[PATH_MAX_COUNT];

	WAVSTUD_APPOBJ wso;
	WAVSTUD_INFO_SET wsis;
	WAVSTUD_INFO_SET wsis_max;

	ISF_DATA isf_audcap_memblk;
	ISF_DATA isf_audcap_bsmemblk[ISF_AUDCAP_IN_NUM];
	ISF_DATA isf_audcap_bufinfomemblk[ISF_AUDCAP_IN_NUM];
	ISF_DATA isf_audcap_aecmemblk;
	ISF_DATA isf_audcap_anrmemblk;
	ISF_DATA isf_audcap_srcmemblk;
	ISF_DATA isf_audcap_srcbufmemblk;
	ISF_DATA isf_audcap_bsdata;

	UINT32 g_isf_audcap_frame_sample;
	UINT32 g_isf_audcap_frame_block_sample;
	UINT32 g_isf_audcap_max_frame_sample;
	UINT32 g_isf_audcap_buf_cnt;

	AUDCAP_BSDATA           *g_p_audcap_bsdata_blk_pool;
	UINT32                 *g_p_audcap_bsdata_blk_maptlb;
	AUDCAP_BUF_INFO         *g_p_audcap_bufinfo_blk_pool;
	UINT32                 *g_p_audcap_bufinfo_blk_maptlb;

	BOOL                   audfilt_en;
	MEM_RANGE              g_isf_audcap_maxmem;
	BOOL                   g_isf_audcap_allocmem;
	MEM_RANGE              g_isf_audcap_aecmem;
	MEM_RANGE              g_isf_audcap_anrmem;
	MEM_RANGE              g_isf_audcap_srcmem;
	MEM_RANGE              g_isf_audcap_srcbufmem;
	AUDIO_CH               isf_audcap_mono_ch;

	int                    resample_handle_rec;
	MEM_RANGE              isf_audcap_last_left[ISF_AUDCAP_IN_NUM];
	AUDCAP_BUF_INFO         *p_isf_audcap_remain_info[ISF_AUDCAP_IN_NUM];
	AUDCAP_TIMESTAMP        isf_audcap_timestamp;
	UINT32                 isf_audcap_path_open_count;
	UINT32                 isf_audcap_path_start_count;
	BOOL                   isf_audcap_dual_mono;
	BOOL                   isf_audcap_aec_lb_swap;
	INT32                  isf_audcap_alc_en;
	UINT32                 isf_audcap_rec_src;
	INT32                  isf_audcap_prepwr_en;
	MEM_RANGE              isf_audcap_aec_last_left[ISF_AUDCAP_IN_NUM];
	INT32                  isf_audcap_tdm_ch;
	UINT32                 isf_audcap_gain_level;
	AUDCAP_AEC_CONFIG      aec_config_max;
	AUDCAP_ALC_CONFIG      isf_audcap_alc_cfg;
	BOOL                   mono_expand;
	UINT32                 g_volume;
	WAVSTUD_ACT            act;
	UINT32                 g_isf_audcap_audcodec;

	SEM_HANDLE ISF_AUDCAP_COMM_SEM_ID;

	UINT32                 g_audcap_bsdata_num;

} AUDCAP_CONTEXT_DEV;

typedef struct _AUDCAP_CTX_MEM {
	ISF_DATA        ctx_memblk;
	ULONG           ctx_addr;
	UINT32          isf_audcap_size;
	UINT32          wavstudio_size;
	UINT32          pull_queue_max;
	UINT32          pull_queue_size;
} AUDCAP_CTX_MEM;

typedef struct _AUDCAP_CONTEXT_COMMON {
	// CONTEXT
	AUDCAP_CTX_MEM         ctx_mem;   // alloc context memory for isf_audenc + nmedia
} AUDCAP_CONTEXT_COMMON;

typedef struct _AUDCAP_CONTEXT {
	AUDCAP_CONTEXT_COMMON comm;
	AUDCAP_CONTEXT_DEV *dev;
} AUDCAP_CONTEXT;

#define DEV_NUM     1
#define DEV_ID_0    0
extern ISF_UNIT *g_audcap_list[DEV_NUM]; //list of all audiocap device
#define DEV_UNIT(did)	g_audcap_list[(did)]

extern UINT32 g_audcap_init[ISF_FLOW_MAX];

extern void isf_audcap_install_id(void);
extern void isf_audcap_uninstall_id(void);
//extern SEM_HANDLE ISF_AUDCAP_SEM_ID[ISF_AUDCAP_OUT_NUM];
//extern SEM_HANDLE ISF_AUDCAP_PULLQ_SEM_ID[ISF_AUDCAP_OUT_NUM];
extern SEM_HANDLE ISF_AUDCAP_PROC_SEM_ID;
extern ISF_DATA_CLASS _isf_audcap_base[DEV_NUM];

//debug
extern void isf_audcap_dump_status(int (*dump)(const char *fmt, ...), ISF_UNIT *p_thisunit);
extern void isf_audcap_get_audinfo(PWAVSTUD_INFO_SET info);
extern UINT32 isf_audcap_get_vol(UINT32 path);
extern UINT32 isf_audcap_get_recsrc(void);
extern ISF_RV _isf_audcap_outputport_pullbuf(UINT32 devID, UINT32 oport, ISF_DATA *p_data, struct _ISF_UNIT *p_destunit, INT32 wait_ms);
extern ISF_RV _isf_audcap_getportstruct(UINT32 devID, ISF_UNIT *p_thisunit, UINT32 nport, UINT32 param, ULONG *p_struct, UINT32 size);
extern ISF_RV _isf_audcap_setportstruct(UINT32 devID, struct _ISF_UNIT *p_thisunit, UINT32 nport, UINT32 param, ULONG* p_struct, UINT32 size);
extern ULONG _isf_audcap_getportparam(UINT32 devID, struct _ISF_UNIT *p_thisunit, UINT32 nport, UINT32 param);
extern ISF_RV _isf_audcap_setportparam(UINT32 devID, struct _ISF_UNIT *p_thisunit, UINT32 nport, UINT32 param, ULONG value);
extern ISF_RV _isf_audcap_bindouput(struct _ISF_UNIT *p_thisunit, UINT32 oport, struct _ISF_UNIT *p_destunit, UINT32 iport);
extern ISF_RV _isf_audcap_updateport(UINT32 devID, struct _ISF_UNIT *p_thisunit, UINT32 oport, ISF_PORT_CMD cmd);
extern ISF_RV _isf_audcap_do_command(UINT32 cmd, UINT32 p0, UINT32 p1, UINT32 p2);
extern int isf_audcap0_chk_ver(void);
extern int isf_audcap_chk_ver(void);
extern BOOL _isf_audcap_sxcmd_otrace(CHAR *strCmd);
extern BOOL _isf_audcap_sxcmd_ctrace(CHAR *strCmd);
extern BOOL _isf_audcap_sxcmd_dumpque(UINT32 dev, int (*dump)(const char *fmt, ...));
extern BOOL _isf_audcap_sxcmd_dumpbuf(UINT32 dev);
extern BOOL _isf_audcap_sxcmd_dump_que_num(UINT32 dev, int (*dump)(const char *fmt, ...));
#endif

