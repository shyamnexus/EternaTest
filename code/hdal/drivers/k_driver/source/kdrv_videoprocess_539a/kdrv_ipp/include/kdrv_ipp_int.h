/*
    IPP module driver

    NT98520 IPP internal header file.

    @file       kdrv_ipp_int.h
    @ingroup    mIIPP
    @note       Nothing

    Copyright   Novatek Microelectronics Corp. 2019.  All rights reserved.
*/

#ifndef _KDRV_IPP_INT_H_
#define _KDRV_IPP_INT_H_

#if defined(__LINUX)
#elif defined(__FREERTOS)
#include <string.h>
#endif
#include "kwrap/task.h"
#include "kwrap/type.h"
#include "kwrap/spinlock.h"
#include "kwrap/semaphore.h"
#include "kdrv_builtin/kdrv_ipp_builtin.h"
#include "kdrv_videoprocess/kdrv_ipp_539a.h"
#include "kdrv_ipp_int_dbg.h"
#include "kdrv_ipp_int_util.h"
#include "kdrv_ipp_builtin_dbg_info.h"
#include "kdrv_ipp_builtin.h"
#include "ime_eng_cal_dual_params.h"

#define KDRV_IPP_TOTAL_NODE_NUM   (1)
#define KDRV_IPP_ENG_MAX_BIND_NUM (KDRV_IPP_ENG_NUM)//538 modified
#define KDRV_IPP_JOB_HEAD_MAX_NUM (20)
#define KDRV_IPP_VPRC_BIND_MAX_NUM (128)

#define KDRV_IPP_VPROC_ENG_0 (0x00)
#define KDRV_IPP_VPROC_ENG_1 (0x01)

#define KDRV_IPP_JOB_ID_MAX ((1 << 16) - 1)
#define KDRV_IPP_TIMEOUT_MS (5000)

#define KDRV_IPP_NN_ISP_PATH_MAX (4)

#define KDRV_IPP_SW_BALANCE_TH (1000000000)

typedef enum {
	KDRV_IPP_CB_JOBEND = 0,
	KDRV_IPP_CB_JOBSTART = 0,
	KDRV_IPP_CB_ERR,
	KDRV_IPP_DO_NOT_CB,
	KDRV_IPP_CB_CHK_MAX
} KDRV_IPP_CB_CHK_RSLT;

typedef struct{
	UINT8 cnt;
	UINT8 drm_chk_fail_cnt;
	UINT8 used;
	UINT8 err_flg;
} KDRV_IPP_CB_COUNTER;

typedef struct {
	UINT8 is_direct_cnt;
	UINT8 is_direct;
	UINT16 done_eng_bit;
	UINT16 start_eng_bit;
	KDRV_IPP_STATUS status;
	UINT64 frm_st_timestamp;	/* keep last frame start timestamp */
} KDRV_IPP_DIR_INFO;


typedef struct{
 BOOL en;
 UINT32 proc_id;
 UINT32 isp_id;
 UINT32 nn_path_id;
}KDRV_IPP_NN_ISP_PATH_INFO;

typedef struct{
  UINT32 ipp_id;
  UINT32 width;
  UINT32 height;
  // NN_ISP_DRV_AI_PARAM drv_ai_param; // parameter for kdrv_ai, not ready
  KDRV_IPP_NN_ISP_PATH_INFO nn_path[KDRV_IPP_NN_ISP_PATH_MAX];
}KDRV_IPP_NN_ISP_JOB;

typedef struct {
	void *p_parent_job_head;
	KDRV_IPP_IO_CFG ipp_io_cfg;
	KDRV_PRE_IO_CFG pre_io_cfg;
	KDRV_IFE_IO_CFG ife_io_cfg;
	KDRV_DCE_IO_CFG dce_io_cfg;
	KDRV_IPE_IO_CFG ipe_io_cfg;
	KDRV_IME_IO_CFG ime_io_cfg;
	KDRV_PRE_IQ_CFG pre_iq_cfg;
	KDRV_IFE_IQ_CFG ife_iq_cfg;
	KDRV_DCE_IQ_CFG dce_iq_cfg;
	KDRV_IPE_IQ_CFG ipe_iq_cfg;
	KDRV_IME_IQ_CFG ime_iq_cfg;
	UINT32 cb_inte[KDRV_IPP_ENG_MAX];
	UINT16 wait_eng_mask;	/* frame end wait mask */
	UINT16 start_eng_mask;	/* frame start wait mask */
	UINT16 done_eng_bit;	/* frame end received bit */
	UINT16 start_eng_bit;	/* frame start received bit */
	UINT16 trig_eng_bit;	/* debug use */

	UINT32 status;
	ULONG owner;
	UINT32 isp_id;
	KDRV_IPP_LIST_HEAD pool_list;
	KDRV_IPP_LIST_HEAD cfg_list;
	KDRV_IPE_BUF_ADDR cfg_ycurve_addr;
	KDRV_IPE_BUF_ADDR cfg_gamma_addr;
	KDRV_IPE_BUF_ADDR cfg_3dcc_addr;
	KDRV_IPP_OP_MODE ipp_trig_status;

	KDRV_IPP_NN_ISP_JOB nn_isp_job;

	BOOL nn_isp_en;
	DUAL_GBL_PARAMS gbl_param;
	IME_ENG_REG_PARAMS left_param;
	IME_ENG_REG_PARAMS right_param;

	UINT32 nn_isp_w_idx;
	UINT32 nn_isp_r_idx;

} KDRV_IPP_JOB_CFG;

typedef struct {
	UINT32 chip_id;
	UINT32 eng_id;
	UINT32 job_id;
	KDRV_IPP_TRIG_PARAM trig_param;

	vk_spinlock_t lock;
	UINT8 job_num;
	UINT8 err_num;
	UINT8 done_num;
	UINT8 proc_num;
	UINT8 start_num;
	UINT8 split_num;
	UINT8 job_process_done;

	KDRV_IPP_LIST_HEAD job_cfg_root;/* list head of KDRV_IPP_JOB_CFG (cfg_list) */
	KDRV_IPP_LIST_HEAD ll_blk_root;	/* list head of KDRV_IPP_LL_BLK (grp_job_list) */
	KDRV_IPP_LIST_HEAD pool_list;	/* list for pool free/used list */
	KDRV_IPP_LIST_HEAD proc_list;	/* list for proc list */
	KDRV_IPP_LIST_HEAD cb_list;		/* list for cb list */
	KDRV_IPP_JOB_CFG *p_last_cfg;	/* last processing job cfg */

	KDRV_CALLBACK_FUNC cb_fp;

	UINT32 timestamp[KDRV_IPP_JOB_TS_MAX];
	UINT8 drm_output_done;
	KDRV_IPP_CB_COUNTER* p_cb_counter;
	KDRV_IPP_CB_COUNTER* p_fs_cb_counter;
	UINT32 cb_counter_id;
	KDRV_IPP_CB_STCS_OUTPUT* p_stcs_out_data;

	KDRV_IPP_CB_STCS_OUTPUT stcs_out_data;

	INT32 (*ipp_to_ai_cb)(KDRV_IPP_NN_ISP_OPERATION op, void* in, void* out);

	KDRV_IPP_NN_ISP_IPP_SLICE_JOB slice_job;

	#if(KDRV_AI_TEST)
	ULONG kdrv_ai_va;
	ULONG kdrv_ai_pa;
	ULONG ubuf_pa;
	#endif

	UINT8 skip_check;

} KDRV_IPP_JOB_HEAD;

typedef struct {
	VOS_TICK acc_start_tick;
	UINT32 acc_frm_cnt;
	UINT16 fps;
	UINT64 last_update_time;

	UINT64 acc_eng_time;
	UINT32 eng_usage;
	UINT32 trig_cnt;
} KDRV_IPP_DBG_PERF;
typedef struct {
	UINT32 chip_id;
	UINT32 eng_id;
	UINT8  bound;
	KDRV_IPP_BIND_MODE bind_mode;
} KDRV_IPP_ENGINE;


typedef struct {

	KDRV_IPP_BIND_MODE bind_mode;

}KDRV_IPP_KFLOW_HDL_INFO;

typedef struct{
	UINT32 node_id;
	KDRV_IPP_BIND_TYPE bind_type;
	UINT32 vporc_bind_num;  // number of vproc dinding to node
	UINT32 eng_bind_num;    // number of hw biding to node
	KDRV_IPP_ENGINE bound_eng[KDRV_IPP_ENG_MAX_BIND_NUM];

	KDRV_IPP_JOB_HEAD* p_cur_job;
	KDRV_IPP_JOB_HEAD* p_extend_job[KDRV_IPP_ENG_MAX_BIND_NUM];
	KDRV_IPP_KFLOW_HDL_INFO kflow_hdl_info[KDRV_IPP_VPRC_BIND_MAX_NUM];
	UINT32 cur_job_id;
	UINT32 cur_job_cnt;
	KDRV_IPP_CB_COUNTER cb_counter[KDRV_IPP_JOB_HEAD_MAX_NUM];
	KDRV_IPP_CB_COUNTER fs_cb_counter[KDRV_IPP_JOB_HEAD_MAX_NUM];
	UINT32 cb_counter_idx;

	vk_spinlock_t cb_counter_lock;
	vk_spinlock_t fs_cb_counter_lock;
	vk_spinlock_t bind_queue_flg_lock;

	UINT32 eng_0_open_cnt;
	UINT32 eng_1_open_cnt;

	BOOL bind_raw_all_flg;

}KDRV_IPP_NODE;


typedef struct {
	UINT32 chip_id;
	UINT32 eng_id;

	UINT32 open_cnt;

	SEM_HANDLE sem;

	vk_spinlock_t lock;
	UINT32 cur_job_id;
	UINT32 job_in_cnt;
	UINT32 job_out_cnt;
	UINT8 is_dma_abort;
	UINT8 trig_op;
	KDRV_IPP_DIR_INFO direct_info;
	KDRV_IPP_JOB_HEAD *p_cur_job;	/* job in configuration, wait for trigger to add to list */
	KDRV_IPP_JOB_CFG *p_fired_cfg;	/* engine processing cfg for cpu mode */
	KDRV_IPP_JOB_CFG *p_ll_fired_cfg;  /* engine processing cfg for ll mode */
	KDRV_IPP_LL_BLK *p_fired_ll;	/* engine processing ll for ll mode */
	KDRV_IPP_JOB_CFG *p_dbg_cfg;    /* cfg for dbg cmd*/
	ID eng_sts_flg_id;

	THREAD_HANDLE proc_tsk_id;
	THREAD_HANDLE nn_isp_tsk_id;
	ID proc_tsk_flg_id;
	ID nn_isp_tsk_flg_id;
	vk_spinlock_t job_list_lock;
	KDRV_IPP_LIST_HEAD job_list_root;

	THREAD_HANDLE cb_tsk_id;
	ID cb_tsk_flg_id;
	vk_spinlock_t cb_list_lock;
	KDRV_IPP_LIST_HEAD cb_list_root;

	ID pool_flg_id;

	vk_spinlock_t ll_wait_list_lock;
	KDRV_IPP_LIST_HEAD ll_wait_list_root;

	vk_spinlock_t nn_isp_fw_handshake_lock;

	KDRV_IPP_DBG_PERF dbg_perf_info;
	KDRV_IPP_DBG_INFO ipp_dbg_info;
	KDRV_IPP_DBG_CTL ipp_dbg_ctl;


	KDRV_IPP_NODE* bund_node;
	UINT32 sw_balance_acc;

	UINT32 job_in_cb_queue;

	BOOL nn_isp_p0_first_slice_ready;
	BOOL nn_isp_p1_first_slice_ready;
	BOOL nn_isp_p2_first_slice_ready;
	BOOL nn_isp_p3_first_slice_ready;
	KDRV_IPP_DTSI_INFO *dtsi_info;
	BOOL node_changed;

	UINT32 pre_expect_intr;
	UINT32 ife_expect_intr;
	UINT32 ipe_expect_intr;
	UINT32 ime_expect_intr;

} KDRV_IPP_HANDLE;

typedef struct {
	KDRV_IPP_HANDLE* triggerd_hdl;
	UINT8 w_idx;
	UINT8 r_idx;
} KDRV_IPP_ENG_HW_INFO;

typedef struct {
	CHAR name[16];
	KDRV_IPP_BUF_ADDR start_addr;
	ULONG total_size;
	ULONG blk_size;
	UINT8 blk_num;
	vk_spinlock_t lock;
	UINT8 cur_free_num;
	UINT8 max_used_num;
	KDRV_IPP_LIST_HEAD blk_free_list_root; /* free block list head of buffer struct(depend on pool) */
	KDRV_IPP_LIST_HEAD blk_used_list_root; /* used block list head of buffer struct(depend on pool) */
} KDRV_IPP_MEM_POOL;

typedef struct {
	UINT32 chip_num;
	UINT32 eng_num;
	UINT32 total_ch;

	KDRV_IPP_HANDLE p_hdl[KDRV_IPP_CHIP_NUM][KDRV_IPP_ENG_NUM][KDRV_IPP_BIND_QUEUE_NUM];
	UINT8 start_from_fastboot;

	KDRV_IPP_MEM_POOL job_head_pool;
	KDRV_IPP_MEM_POOL job_cfg_pool;
	KDRV_IPP_MEM_POOL ll_blk_pool;
	UINT32 ll_eng_cmd_num[KDRV_IPP_ENG_MAX];
	ID pool_flg_id;

	KDRV_IPP_ENGINE eng_bind[KDRV_IPP_CHIP_NUM][KDRV_IPP_ENG_NUM];
	UINT32 available_node_num;

	KDRV_IPP_ENG_HW_INFO pre_hw_info[KDRV_IPP_CHIP_NUM][KDRV_IPP_ENG_NUM];
	KDRV_IPP_ENG_HW_INFO ife_hw_info[KDRV_IPP_CHIP_NUM][KDRV_IPP_ENG_NUM];
	KDRV_IPP_ENG_HW_INFO ipe_hw_info[KDRV_IPP_CHIP_NUM][KDRV_IPP_ENG_NUM];
	KDRV_IPP_ENG_HW_INFO ime_hw_info[KDRV_IPP_CHIP_NUM][KDRV_IPP_ENG_NUM];

	vk_spinlock_t pre_hdl_queue_lock;
	vk_spinlock_t ife_hdl_queue_lock;
	vk_spinlock_t ipe_hdl_queue_lock;
	vk_spinlock_t ime_hdl_queue_lock;

	UINT8 eng_0_open_cnt;
	UINT8 eng_1_open_cnt;

} KDRV_IPP_CTL;

typedef struct {
	INT32(*set_fp)(KDRV_IPP_NODE *p_node, UINT32 flow_hdl_id, void *p_data);
	UINT8 set_chk_data;
	UINT8 set_chk_cur_job;

	INT32(*get_fp)(KDRV_IPP_NODE *p_node, UINT32 flow_hdl_id, void *p_data);
	UINT8 get_chk_data;
	UINT8 get_chk_cur_job;

	CHAR *msg;
} KDRV_IPP_FUNC_ITEM;

INT32 kdrv_ipp_int_stripe_preprocess(KDRV_IPP_HANDLE *p_hdl, KDRV_IPP_JOB_CFG *p_cfg);
INT32 kdrv_ipp_job_process_cpu(KDRV_IPP_HANDLE *p_hdl, KDRV_IPP_JOB_HEAD* p_job, KDRV_IPP_JOB_CFG *p_cfg, KDRV_IPP_TRIG_PARAM *p_trig_param);
INT32 kdrv_ipp_job_process_ll(KDRV_IPP_HANDLE *p_hdl, KDRV_IPP_JOB_HEAD* p_job, KDRV_IPP_TRIG_PARAM *p_trig_param, KDRV_IPP_JOB_CFG *p_cfg, KDRV_IPP_LL_BLK *p_ll_blk, UINT32 *eng_cmd_num, UINT32 stage);
INT32 kdrv_ipp_job_trig_ll(KDRV_IPP_HANDLE *p_hdl, KDRV_IPP_LL_BLK *p_ll_blk);
INT32 kdrv_ipp_job_stop_eng_direct(KDRV_IPP_HANDLE *p_hdl, void *reserve);
UINT32 kdrv_ipp_dtsi_checksum_cal(KDRV_IPP_JOB_CFG* p_ipp_cfg);
KDRV_DEV_ENGINE kdrv_ipp_get_pre_eng_id(UINT32 id);
KDRV_DEV_ENGINE kdrv_ipp_get_ife_eng_id(UINT32 id);
//KDRV_DEV_ENGINE kdrv_ipp_get_dce_eng_id(UINT32 id); // 690 removed
KDRV_DEV_ENGINE kdrv_ipp_get_ipe_eng_id(UINT32 id);
KDRV_DEV_ENGINE kdrv_ipp_get_ime_eng_id(UINT32 id);
INT32 kdrv_ipp_get_nn_isp_max_strp(KDRV_IPP_HANDLE *p_hdl, KDRV_IPP_JOB_HEAD *p_job);
INT32 kdrv_ipp_nn_isp_cal_slice(KDRV_IPP_HANDLE *p_hdl, KDRV_IPP_JOB_HEAD *p_job);
INT32 kdrv_ipp_set_nn_isp_info(KDRV_IPP_HANDLE *p_hdl, KDRV_IPP_JOB_HEAD *p_job);
INT32 kdrv_ipp_update_nn_isp_info(KDRV_IPP_HANDLE *p_hdl, KDRV_IPP_JOB_HEAD *p_job);
BOOL kdrv_ipp_check_split(KDRV_IPP_NODE* p_node, KDRV_IPP_JOB_CFG* p_cfg);
void *kdrv_ipp_int_plat_malloc(UINT32 size);
void kdrv_ipp_int_plat_free(void *ptr);

#endif //_KDRV_IPP_INT_H_
