#ifndef _CTL_VIE_DEBUG_INT_H_
#define _CTL_VIE_DEBUG_INT_H_
#include "ctl_vie_dbg.h"
#include "ctl_vie_utility_int.h"
/**
    ctl_vie_debug_int.h


    @file       ctl_vie_debug_int.h
    @ingroup    mISYSAlg
    @note       Nothing (or anything need to be mentioned).

    Copyright   Novatek Microelectronics Corp. 2011.  All rights reserved.
*/

/**
    time stamp log
*/
#define CTL_VIE_NAME_LEN                    32
#define CTL_VIE_DBG_TS_MAXNUM               (8 * CTL_VIE_MAX_SUPPORT_ID)
#define CTL_VIE_DBG_ISR_IOCTL_NUM           (8 * CTL_VIE_MAX_SUPPORT_ID)
#define CTL_VIE_DBG_ISP_CB_T_DBG_NUM        (4 * CTL_VIE_MAX_SUPPORT_ID)
#define CTL_VIE_SYSDBG_CNT_DFT              10 // per id

#if defined(CONFIG_NVT_SMALL_HDAL)
#define CTL_VIE_DBG_DUMP_IO_BUF (DISABLE)
#else
#define CTL_VIE_DBG_DUMP_IO_BUF (ENABLE)
#endif

struct val_str_map_info {
	unsigned int val;
	char *str;
};

typedef enum {
	CTL_VIE_DBG_MSG_OFF = 0,        //disable all debug msg
	CTL_VIE_DBG_MSG_CTL_INFO,       //dump kflow_vie parameters info
	CTL_VIE_DBG_MSG_PROC_TIME,      //dump kflow_vie processing time/vd period
	CTL_VIE_DBG_MSG_BUF_IO_LITE,    //dump kflow_vie buffer NEW/PUSH lite info
	CTL_VIE_DBG_MSG_BUF_IO_FULL,    //dump kflow_vie buffer NEW/PUSH full info
	CTL_VIE_DBG_MSG_PSR,            //enable Problem Steps Recorder
	CTL_VIE_DBG_MSG_ALL,            //enable all ctl vie debug msg
	CTL_VIE_DBG_MSG_ISR_IOCTL,      //set vie buffer new/push/lock/unlock infor, set 1 to enable record, set 2 to dump
	CTL_VIE_DBG_MSG_RING_BUF_FULL,  //set VIE2 ring buf height to scale_out_h on/off(full) (set before trigger start)
	CTL_VIE_DBG_MSG_REG_DBG_CB,     //
	CTL_VIE_DBG_MSG_FORCE_PATGEN,   //
	CTL_VIE_DBG_MSG_BUF_CHK,        //enable for protect vie buffer(flag check)
	CTL_VIE_DBG_MSG_BUF_DMA_WP,     //enable for dma write protect vie output buffer
	CTL_VIE_DBG_MSG_MAX
} CTL_VIE_DBG_MSG_TYPE;

const static struct val_str_map_info dbg_type_str_tab[CTL_VIE_DBG_MSG_MAX] = {
	{CTL_VIE_DBG_MSG_OFF,              "type: 0,  disable dbg msg"},
	{CTL_VIE_DBG_MSG_CTL_INFO,         "type: 1,  dump info"},
	{CTL_VIE_DBG_MSG_PROC_TIME,        "type: 2,  dump proc time"},
	{CTL_VIE_DBG_MSG_BUF_IO_LITE,      "type: 3,  dump buffer in/out lite status, [par1]: dump count"},
	{CTL_VIE_DBG_MSG_BUF_IO_FULL,      "type: 4,  dump buffer in/out full status, [par1]: dump count"},
	{CTL_VIE_DBG_MSG_PSR,              "type  5,  enable for psr msg, [par1]: 0:disable,1:enable"},
	{CTL_VIE_DBG_MSG_ALL,              "type: 6,  dump all debug info"},
	{CTL_VIE_DBG_MSG_ISR_IOCTL,        "type: 7,  record and dump inte proc ts, [par1]: 0:disable,1:enable record, 2:dump"},
	{CTL_VIE_DBG_MSG_RING_BUF_FULL,    "type: 8,  set before kflow open for enable/disable full ring buffer allocate, [par1]: 0:disable,1:enable(only valid for SHDR and VIE2)"},
	{CTL_VIE_DBG_MSG_REG_DBG_CB,       "type: 9,  reserved"},
	{CTL_VIE_DBG_MSG_FORCE_PATGEN,     "type: 10, reserved"},
	{CTL_VIE_DBG_MSG_BUF_CHK,          "type: 11, set for vie output buffer flag check, [par1]: 0:disable,1:enable"},
	{CTL_VIE_DBG_MSG_BUF_DMA_WP,       "type: 12, set for vie output ch0 buffer dma write protect, [par1]: 0: disable, 1: enable, [par2]: 0: DDR_ARB_1, 1: DDR_ARB_2, [par3]: 0: vproc will dis wp, 1: dis_wp_before_push"},
};

typedef enum {
	CTL_VIE_DBG_ISP_OFF = 0,    //disable debug msg
	CTL_VIE_DBG_ISP_CB_SKIP,    //skip isp cb_fp
	CTL_VIE_DBG_ISP_MAX
} CTL_VIE_DBG_ISP_TYPE;

typedef enum {
	CTL_VIE_MANUAL_TYPE_BP3, 	// set bp3, type: UINT32, unit: line (start from vie active_y = 0)

	CTL_VIE_MANUAL_TYPE_MASK_SEL,

	CTL_VIE_MANUAL_TYPE_MASK0_WIN,
	CTL_VIE_MANUAL_TYPE_MASK1_WIN,
	CTL_VIE_MANUAL_TYPE_MASK2_WIN,
	CTL_VIE_MANUAL_TYPE_MASK3_WIN,

	CTL_VIE_MANUAL_TYPE_MASK0_COLOR,
	CTL_VIE_MANUAL_TYPE_MASK1_COLOR,
	CTL_VIE_MANUAL_TYPE_MASK2_COLOR,
	CTL_VIE_MANUAL_TYPE_MASK3_COLOR,

	CTL_VIE_MANUAL_TYPE_MAX
} CTL_VIE_MANUAL_TYPE;

typedef struct {
	UINT32 t;
	UINT64 fc;
	UINT32 evtcnt;
	CTL_VIE_DBG_TS_EVT evt;
} CTL_VIE_DBG_TS_INFO;

typedef struct {
	CHAR proc_name[CTL_VIE_NAME_LEN];
	CTL_VIE_ID id;
	CTL_VIE_PROC_TIME_ITEM item; // enter or exit
	UINT32 time_us; // clock, us
} CTL_VIE_PROC_TIME_INFO;

typedef struct {
	UINT32 vie_id;
	UINT32 isr_evt;
	UINT32 buf_ctl_type;
	UINT32 buf_id;
	ULONG buf_addr_va;	// virtual address
	ULONG buf_addr_pa;	// physical address
	ULONG ch0_addr_va;	// virtual address
	ULONG ch0_addr_pa;	// physical address
	USIZE  vie_out_size;
	UINT32 ch0_lofs;
	UINT64 frm_cnt;
	UINT32 ts;
} CTL_VIE_DBG_ISR_IOCTL;

/***************************************/
/*
dbg_msg_type map
|--reserved()--+--ddr_id(2)--+--dbg_type(20)--|
dbg_type: CTL_VIE_DBG_MSG_TYPE
ddr_id: vie out ch0 buffer ddr id
*/
/***************************************/
#define CTL_VIE_DBG_TYPE_OFS                    0
#define CTL_VIE_DBG_TYPE_MASK                   0xfffff
#define CTL_VIE_DBG_GET_DBG_TYPE(type)          (type & CTL_VIE_DBG_TYPE_MASK)

#define CTL_VIE_DBG_DDR_ID_OFS                  20
#define CTL_VIE_DBG_DDR_ID_MASK                 0x3
#define CTL_VIE_DBG_GET_DDR_ID(type)            (type>>CTL_VIE_DBG_DDR_ID_OFS) & CTL_VIE_DBG_DDR_ID_MASK

#define CTL_VIE_DBG_WP_RLS_OFS                  22
#define CTL_VIE_DBG_WP_RLS_MASK                 0x1
#define CTL_VIE_DBG_GET_WP_RLS_EN(type)         (type>>CTL_VIE_DBG_WP_RLS_OFS) & CTL_VIE_DBG_WP_RLS_MASK

#define CTL_VIE_DBG_SET_DBG_TYPE(cur_type, val, ofs, en) (en ? (*cur_type |= val<<ofs) : (*cur_type &= ~(val<<ofs)))
#define CTL_VIE_DBG_LOG_RATE_DFT 300

typedef struct {
	UINT64 fc;
	CTL_VIE_SYS_INFO *info;
	struct vos_list_head list;
} CTL_VIE_DBG_SYS_INFO;

typedef struct {
	UINT32 dbg_bufio_cnt;
	UINT32 dbg_ts_cnt;
	UINT32 dbg_isr_ioctl_cnt;
	UINT32 dbg_msg_type;
	CTL_VIE_DBG_TS_INFO dbg_ts_info[CTL_VIE_DBG_TS_MAXNUM];

	/* system information */
	BOOL                    sys_info_init;
	vk_spinlock_t           sys_info_list_lock;
	unsigned long           sys_info_list_flag;
	UINT32                  sys_info_num;
	struct vos_list_head    sys_info_list_root;
	CTL_VIE_DBG_SYS_INFO   *sys_info;

	UINT32 dbg_log_rate; // chk dram out dbg data

#if CTL_VIE_DBG_DUMP_IO_BUF
	CTL_VIE_DBG_ISR_IOCTL dbg_isr_ioctl[CTL_VIE_DBG_ISR_IOCTL_NUM];
#endif
} CTL_VIE_DBG_INFO;

typedef struct {
	ISP_ID ispid;
	ISP_EVENT evt;
	UINT64 raw_fc;
	UINT32 ts_start;
	UINT32 ts_end;
} CTL_VIE_DBG_ISP_CB_T_LOG;

typedef struct {
	/* bp3 */
	BOOL bp3_manual; 			// manual set bp3
	UINT32 bp3;					// bp3 set by user (proc), units:line (start from vie active y = 0)

	/* mask */
	UINT32 mask_manual;          // manual set mask (0 : disable, 1: manual set window info, 2 : four corners, 3 : scroll horizontally)
	CTL_VIE_MASK mask;           // mask set by user (proc)

} CTL_VIE_DBG_OBJ;

/**
    get
*/
void ctl_vie_dbg_get_buf_wp_info(CTL_VIE_ID id, UINT32 *ddr_id, UINT32 *dis_wp);
BOOL ctl_vie_dbg_chk_msg_type(CTL_VIE_ID id, UINT32 chk_type);
CTL_VIE_DBG_OBJ *ctl_vie_dbg_get_dbg_obj(CTL_VIE_ID id);
UINT32 ctl_vie_dbg_get_dbg_log_rate(CTL_VIE_ID id);

/**
    set
*/
UINT32 ctl_vie_dbg_query_buf(void);
void ctl_vie_dbg_set_buf(UINT32 id, ULONG buf_addr);
void ctl_vie_dbg_set_msg_type(CTL_VIE_ID id, CTL_VIE_DBG_MSG_TYPE type, UINT32 par1, UINT32 par2, UINT32 par3);
void ctl_vie_dbg_set_ts(CTL_VIE_ID id, CTL_VIE_DBG_TS_EVT evt, UINT64 vie_fc);
void ctl_vie_dbg_set_isr_ioctl(CTL_VIE_ID id, UINT32 status, UINT32 buf_ctl_type, CTL_VIE_HEAD_IDX head_idx);
void ctl_vie_dbg_set_frm_ctrl_info(CTL_VIE_ID id, CTL_VIE_FRM_CTL_INFO *frm_ctrl);
void ctl_vie_dbg_set_isp_cb_t_log(ISP_ID id, ISP_EVENT evt, UINT64 fc, UINT32 ts_start, UINT32 ts_end);
void ctl_vie_dbg_isp_set_msg_type(ISP_ID id, CTL_VIE_DBG_ISP_TYPE type, UINT32 par1, UINT32 par2);
void ctl_vie_dbg_set_proc_t(CTL_VIE_ID id, CHAR *proc_name, CTL_VIE_PROC_TIME_ITEM item);
void ctl_vie_dbg_reset_ts(CTL_VIE_ID id);
void ctl_vie_dbg_set_ccir_header_chk(CTL_VIE_ID id, UINT32 chk_header, UINT32 interval_ms, UINT32 chk_times);
void ctl_vie_dbg_manual(CTL_VIE_ID id, CTL_VIE_MANUAL_TYPE type, BOOL en, void *param);
void ctl_vie_dbg_upd_sysdbg_info(CTL_VIE_ID id, UINT32 evt, UINT64 vie_fc);
void ctl_vie_dbg_set_sysdbg_en(CTL_VIE_ID id, BOOL en, UINT32 num);
void ctl_vie_dbg_set_dbg_log_rate(CTL_VIE_ID id, UINT32 dbg_log_rate, BOOL proc);

/**
    dump info
*/
void ctl_vie_dbg_dump_info(int (*dump)(const char *fmt, ...));
void ctl_vie_dbg_dump_ts(CTL_VIE_ID id, int (*dump)(const char *fmt, ...));
void ctl_vie_dbg_dump_buf_io(CTL_VIE_ID id, CTL_VIE_BUF_IO_CFG buf_io, UINT32 total_size, ULONG header_addr);
void ctl_vie_dbg_dump_isr_ioctl(int (*dump)(const char *fmt, ...));
void ctl_vie_dbg_isp_cb_t_dump(int (*dump)(const char *fmt, ...));
void ctl_vie_dbg_isp_dump(int (*dump)(const char *fmt, ...));
void ctl_vie_dbg_dump_proc_time(int (*dump)(const char *fmt, ...));
void ctl_vie_dbg_savefile(CHAR *f_name, ULONG addr, UINT32 size);
void ctl_vie_dbg_dump_sysdbg(CTL_VIE_ID id, int (*dump)(const char *fmt, ...));
int ctl_vie_panic_printout_handler(uintptr_t data);

void ctl_vie_dbg_init(CTL_VIE_ID id);
void ctl_vie_dbg_uninit(CTL_VIE_ID id);

#endif
