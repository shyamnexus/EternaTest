#ifndef _CTL_SIE_DEBUG_INT_H_
#define _CTL_SIE_DEBUG_INT_H_
#include "ctl_sie_dbg.h"
#include "ctl_sie_utility_int.h"
#include "kdrv_builtin/sie_init.h"
/**
    ctl_sie_debug_int.h


    @file       ctl_sie_debug_int.h
    @ingroup    mISYSAlg
    @note       Nothing (or anything need to be mentioned).

    Copyright   Novatek Microelectronics Corp. 2011.  All rights reserved.
*/

/**
    time stamp log
*/
#define CTL_SIE_NAME_LEN                    32
#define CTL_SIE_DBG_TS_MAXNUM               (12 * CTL_SIE_MAX_SUPPORT_ID)
#define CTL_SIE_DBG_ISR_IOCTL_NUM           (12 * CTL_SIE_MAX_SUPPORT_ID)
#define CTL_SIE_DBG_ISP_CB_T_DBG_NUM        (6 * CTL_SIE_MAX_SUPPORT_ID)
#define CTL_SIE_SYSDBG_CNT_DFT              10 // per id

#if defined(CONFIG_NVT_SMALL_HDAL)
#define CTL_SIE_DBG_DUMP_IO_BUF (DISABLE)
#else
#define CTL_SIE_DBG_DUMP_IO_BUF (ENABLE)
#endif

struct val_str_map_info {
	unsigned int val;
	char *str;
};

typedef enum {
	CTL_SIE_DBG_MSG_OFF = 0,        //disable all debug msg
	CTL_SIE_DBG_MSG_CTL_INFO,       //dump kflow_sie parameters info
	CTL_SIE_DBG_MSG_PROC_TIME,      //dump kflow_sie processing time/vd period
	CTL_SIE_DBG_MSG_BUF_IO_LITE,    //dump kflow_sie buffer NEW/PUSH lite info
	CTL_SIE_DBG_MSG_BUF_IO_FULL,    //dump kflow_sie buffer NEW/PUSH full info
	CTL_SIE_DBG_MSG_DIRECT_SKIP,    //enable for direct mode skip trigger ipp
	CTL_SIE_DBG_MSG_ALL,            //enable all ctl sie debug msg
	CTL_SIE_DBG_MSG_ISR_IOCTL,      //set sie buffer new/push/lock/unlock infor, set 1 to enable record, set 2 to dump
	CTL_SIE_DBG_MSG_RING_BUF_FULL,  //set SIE2 ring buf height to scale_out_h on/off(full) (set before trigger start)
	CTL_SIE_DBG_MSG_REG_DBG_CB,     //
	CTL_SIE_DBG_MSG_FORCE_PATGEN,   //
	CTL_SIE_DBG_MSG_BUF_CHK,        //enable for protect sie buffer(flag check)
	CTL_SIE_DBG_MSG_BUF_DMA_WP,     //enable for dma write protect sie output buffer
	CTL_SIE_DBG_MSG_STS_ISR_PROC,   //dump kflow_sie vd/bp3/cropend isr proc
	CTL_SIE_DBG_MSG_MAX
} CTL_SIE_DBG_MSG_TYPE;

const static struct val_str_map_info dbg_type_str_tab[CTL_SIE_DBG_MSG_MAX] = {
	{CTL_SIE_DBG_MSG_OFF,              "type: 0,  disable dbg msg"},
	{CTL_SIE_DBG_MSG_CTL_INFO,         "type: 1,  dump info"},
	{CTL_SIE_DBG_MSG_PROC_TIME,        "type: 2,  dump proc time"},
	{CTL_SIE_DBG_MSG_BUF_IO_LITE,      "type: 3,  dump buffer in/out lite status, [par1]: dump count"},
	{CTL_SIE_DBG_MSG_BUF_IO_FULL,      "type: 4,  dump buffer in/out full status, [par1]: dump count"},
	{CTL_SIE_DBG_MSG_DIRECT_SKIP,      "type  5,  enable for direct mode skip trigger ipp"},
	{CTL_SIE_DBG_MSG_ALL,              "type: 6,  dump all debug info"},
	{CTL_SIE_DBG_MSG_ISR_IOCTL,        "type: 7,  record and dump inte proc ts, [par1]: 0:disable,1:enable record, 2:dump"},
	{CTL_SIE_DBG_MSG_RING_BUF_FULL,    "type: 8,  set before kflow open for enable/disable full ring buffer allocate, [par1]: 0:disable,1:enable(only valid for SHDR and SIE2)"},
	{CTL_SIE_DBG_MSG_REG_DBG_CB,       "type: 9,  reserved"},
	{CTL_SIE_DBG_MSG_FORCE_PATGEN,     "type: 10, reserved"},
	{CTL_SIE_DBG_MSG_BUF_CHK,          "type: 11, set for sie output buffer flag check, [par1]: 0:disable,1:enable"},
	{CTL_SIE_DBG_MSG_BUF_DMA_WP,       "type: 12, set for sie output ch0 buffer dma write protect, [par1]: 0: disable, 1: enable, [par2]: 0: DDR_ARB_1, 1: DDR_ARB_2, [par3]: 0: vproc will dis wp, 1: dis_wp_before_push"},
	{CTL_SIE_DBG_MSG_STS_ISR_PROC,     "type: 13, dump kflow_sie vd/bp3/cropend isr proc time, [par1]: statistic count"},
};

typedef enum {
	CTL_SIE_DBG_ISP_OFF = 0,    //disable debug msg
	CTL_SIE_DBG_ISP_CB_SKIP,    //skip isp cb_fp
	CTL_SIE_DBG_ISP_MAX
} CTL_SIE_DBG_ISP_TYPE;

typedef enum {
	CTL_SIE_MANUAL_TYPE_VD_DLY, // set vd delay, type: CTL_SIE_VD_DLY

	CTL_SIE_MANUAL_TYPE_BP3,    // set bp3, type: UINT32, unit: line (start from sie active_y = 0)

	CTL_SIE_MANUAL_TYPE_MASK_SEL,

	CTL_SIE_MANUAL_TYPE_MASK0_WIN,
	CTL_SIE_MANUAL_TYPE_MASK1_WIN,
	CTL_SIE_MANUAL_TYPE_MASK2_WIN,
	CTL_SIE_MANUAL_TYPE_MASK3_WIN,

	CTL_SIE_MANUAL_TYPE_MASK0_COLOR,
	CTL_SIE_MANUAL_TYPE_MASK1_COLOR,
	CTL_SIE_MANUAL_TYPE_MASK2_COLOR,
	CTL_SIE_MANUAL_TYPE_MASK3_COLOR,

	CTL_SIE_MANUAL_TYPE_MAX,

} CTL_SIE_MANUAL_TYPE;

typedef struct {
	UINT32 t;
	UINT64 fc;
	UINT32 evtcnt;
	CTL_SIE_DBG_TS_EVT evt;
} CTL_SIE_DBG_TS_INFO;

typedef struct {
	CHAR proc_name[CTL_SIE_NAME_LEN];
	CTL_SIE_ID id;
	CTL_SIE_PROC_TIME_ITEM item; // enter or exit
	UINT32 time_us; // clock, us
} CTL_SIE_PROC_TIME_INFO;

typedef struct {
	UINT32 sie_id;
	UINT32 isr_evt;
	UINT32 buf_ctl_type;
	UINT32 buf_id;
	ULONG buf_addr_va;	// virtual address
	ULONG buf_addr_pa;	// physical address
	ULONG ch0_addr_va;	// virtual address
	ULONG ch0_addr_pa;	// physical address
	USIZE  sie_out_size;
	UINT32 ch0_lofs;
	UINT64 frm_cnt;
	UINT32 ts;
} CTL_SIE_DBG_ISR_IOCTL;

/***************************************/
/*
dbg_msg_type map
|--reserved()--+--ddr_id(2)--+--dbg_type(20)--|
dbg_type: CTL_SIE_DBG_MSG_TYPE
ddr_id: sie out ch0 buffer ddr id
*/
/***************************************/
#define CTL_SIE_DBG_TYPE_OFS                    0
#define CTL_SIE_DBG_TYPE_MASK                   0xfffff
#define CTL_SIE_DBG_GET_DBG_TYPE(type)          (type & CTL_SIE_DBG_TYPE_MASK)

#define CTL_SIE_DBG_DDR_ID_OFS                  20
#define CTL_SIE_DBG_DDR_ID_MASK                 0x3
#define CTL_SIE_DBG_GET_DDR_ID(type)            (type>>CTL_SIE_DBG_DDR_ID_OFS) & CTL_SIE_DBG_DDR_ID_MASK

#define CTL_SIE_DBG_WP_RLS_OFS                  22
#define CTL_SIE_DBG_WP_RLS_MASK                 0x1
#define CTL_SIE_DBG_GET_WP_RLS_EN(type)         (type>>CTL_SIE_DBG_WP_RLS_OFS) & CTL_SIE_DBG_WP_RLS_MASK

#define CTL_SIE_DBG_SET_DBG_TYPE(cur_type, val, ofs, en) (en ? (*cur_type |= val<<ofs) : (*cur_type &= ~(val<<ofs)))
#define CTL_SIE_DBG_LOG_RATE_DFT 300

typedef struct {
	UINT64 fc;
	CTL_SIE_SYS_INFO *info;
	struct vos_list_head list;
} CTL_SIE_DBG_SYS_INFO;

typedef struct {
	CTL_SIE_DBG_TS_INFO dbg_ts_info[CTL_SIE_DBG_TS_MAXNUM];

	/* system information */
	BOOL                    sys_info_init;
	vk_spinlock_t           sys_info_list_lock;
	unsigned long           sys_info_list_flag;
	UINT32                  sys_info_num;
	struct vos_list_head    sys_info_list_root;
	CTL_SIE_DBG_SYS_INFO   *sys_info;

	UINT32 dbg_log_rate; // chk dram out dbg data

#if CTL_SIE_DBG_DUMP_IO_BUF
	CTL_SIE_DBG_ISR_IOCTL dbg_isr_ioctl[CTL_SIE_DBG_ISR_IOCTL_NUM];
#endif
} CTL_SIE_DBG_INFO;

typedef struct {
	UINT32 dbg_bufio_cnt;
	UINT32 dbg_ts_cnt;
	UINT32 dbg_isr_ioctl_cnt;
	UINT32 dbg_msg_type;
	UINT32 dbg_stsisr_cnt;		// cnt for ctl_sie_dbg_stat_isr_proc
} CTL_SIE_DBG_CTRL_INFO;

typedef struct {
	ISP_ID ispid;
	ISP_EVENT evt;
	UINT64 raw_fc;
	UINT32 ts_start;
	UINT32 ts_end;
} CTL_SIE_DBG_ISP_CB_T_LOG;

typedef struct {
	/* vd delay */
	BOOL vd_dly_manual;         // manual set vd delay
	CTL_SIE_VD_DLY vd_dly;      // vd delay set by user (proc)

	/* bp3 */
	BOOL bp3_manual;            // manual set bp3
	UINT32 bp3;                 // bp3 set by user (proc), units:line (start from sie active y = 0)

	/* mask */
	UINT32 mask_manual;          // manual set mask (0 : disable, 1: manual set window info, 2 : four corners, 3 : scroll horizontally)
	CTL_SIE_MASK mask;           // mask set by user (proc)

} CTL_SIE_DBG_OBJ;

/**
    get
*/
void ctl_sie_dbg_get_buf_wp_info(CTL_SIE_ID id, UINT32 *ddr_id, UINT32 *dis_wp);
BOOL ctl_sie_dbg_chk_msg_type(CTL_SIE_ID id, UINT32 chk_type);
CTL_SIE_DBG_OBJ *ctl_sie_dbg_get_dbg_obj(CTL_SIE_ID id);
UINT32 ctl_sie_dbg_get_dbg_log_rate(CTL_SIE_ID id);

/**
    set
*/
UINT32 ctl_sie_dbg_query_buf(void);
void ctl_sie_dbg_set_buf(UINT32 id, ULONG buf_addr);
void ctl_sie_dbg_set_msg_type(CTL_SIE_ID id, CTL_SIE_DBG_MSG_TYPE type, UINT32 par1, UINT32 par2, UINT32 par3);
void ctl_sie_dbg_set_ts(CTL_SIE_ID id, CTL_SIE_DBG_TS_EVT evt, UINT64 sie_fc);
void ctl_sie_dbg_set_isr_ioctl(CTL_SIE_ID id, UINT32 status, UINT32 buf_ctl_type, CTL_SIE_HEAD_IDX head_idx);
void ctl_sie_dbg_set_frm_ctrl_info(CTL_SIE_ID id, CTL_SIE_FRM_CTL_INFO *frm_ctrl);
void ctl_sie_dbg_set_isp_cb_t_log(ISP_ID id, ISP_EVENT evt, UINT64 fc, UINT32 ts_start, UINT32 ts_end);
void ctl_sie_dbg_isp_set_msg_type(ISP_ID id, CTL_SIE_DBG_ISP_TYPE type, UINT32 par1, UINT32 par2);
void ctl_sie_dbg_set_proc_t(CTL_SIE_ID id, CHAR *proc_name, CTL_SIE_PROC_TIME_ITEM item);
void ctl_sie_dbg_reset_ts(CTL_SIE_ID id);
void ctl_sie_dbg_manual(CTL_SIE_ID id, CTL_SIE_MANUAL_TYPE type, UINT32 sel, void *param);
void ctl_sie_dbg_upd_sysdbg_info(CTL_SIE_ID id, UINT64 sie_fc, CTL_SIE_SYS_INFO *info_out);
void ctl_sie_dbg_set_sysdbg_en(CTL_SIE_ID id, BOOL en, UINT32 num);
void ctl_sie_dbg_set_dbg_log_rate(CTL_SIE_ID id, UINT32 dbg_log_rate, BOOL proc);
void ctl_sie_dbg_set_clk(UINT32 id, UINT32 clk_rate);

/**
    dump info
*/
void ctl_sie_dbg_dump_info(int (*dump)(const char *fmt, ...));
void ctl_sie_dbg_dump_ts(CTL_SIE_ID id, int (*dump)(const char *fmt, ...));
void ctl_sie_dbg_dump_buf_io(CTL_SIE_ID id, CTL_SIE_BUF_IO_CFG buf_io, UINT32 total_size, ULONG header_addr);
void ctl_sie_dbg_stat_isr_proc(CTL_SIE_ID id, CTL_SIE_DBG_STS_ISR_TYPE isr_flg, UINT32 start_ts);
void ctl_sie_dbg_dump_isr_ioctl(int (*dump)(const char *fmt, ...));
void ctl_sie_dbg_isp_cb_t_dump(int (*dump)(const char *fmt, ...));
void ctl_sie_dbg_isp_dump(int (*dump)(const char *fmt, ...));
void ctl_sie_dbg_dump_proc_time(int (*dump)(const char *fmt, ...));
void ctl_sie_dbg_dump_fb_buf_info(VOS_FILE fd);
void ctl_sie_dbg_savefile(CHAR *f_name, ULONG addr, UINT32 size);
void ctl_sie_dbg_dump_sysdbg(CTL_SIE_ID id, int (*dump)(const char *fmt, ...));
int ctl_sie_panic_printout_handler(uintptr_t data);

void ctl_sie_dbg_init(CTL_SIE_ID id);
void ctl_sie_dbg_uninit(CTL_SIE_ID id);

#endif //_IPP_DEBUG_INT_H_
