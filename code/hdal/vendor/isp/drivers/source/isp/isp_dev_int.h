#ifndef _ISP_DEV_INT_H_
#define _ISP_DEV_INT_H_

#include "isp_dev.h"

//=============================================================================
// struct & definition
//=============================================================================
#define IOCTL_BUF_SIZE          8192 + 3  // To match ISPT_MEMORY_INFO
#define SYNC_INFO_BUF           24
#define WAIT_VD_TIMOUT          1000    // ms

extern UINT32 isp_sync_id_sie[ISP_ID_MAX_NUM];
extern UINT32 isp_sync_id_ipp[ISP_ID_MAX_NUM];
extern UINT32 isp_sync_id_enc[ISP_ID_MAX_NUM];

typedef struct ISP_SYNC_INFO {
	ISP_AE_STATUS ae_status;
	UINT32 total_gain;                             // range : 100~3276800, sync from AE. 100 = 1x
	UINT32 isp_gain;                               // range : 100~3276800, sync from AE. 100 = 1x, for IQ parameter interpolation
	UINT32 dgain;                                  // range : 0~32767, sync from AE. 128 = 1x
	UINT32 lv;                                     // range : 0~20000000, sync from AE.
	UINT32 lv_base;                                // range : 1000000, sync from AE.
	UINT32 shdr_ev_ratio[ISP_SEN_MFRAME_MAX_NUM];  // range : 64~16384, sync from AE. shdr_ev_ratio[0]=64, shdr_ev_ratio[1]= 64~1024, shdr_ev_ratio[2]= 64~16384 , 64=1x
	UINT32 shdr_tm_ratio;                          // range : 0~16384, sync from AE. 0 = linear fcurve, 64 = 0EV, 128 = 1EV, 256 = 2EV, 512 = 3EV, 1024 = 4EV, ..., 16384 = 8EV
	UINT32 cgain[ISP_CGAIN_NUM];                   // range : 0~2047, sync from AWB. 256 = 1x
	UINT32 ct;                                     // range : 1000~12000, sync from AWB.
	UINT32 final_cgain[ISP_CGAIN_NUM];             // range : 0~2047, sync from IQ. 256 = 1x
	ISP_AE_HBS_PARAM shdr_hbs;                     // sync from AE
	BOOL ca_enable;
	BOOL la_enable;
} ISP_SYNC_INFO;

typedef struct _ISP_DEV_INFO {
	// algorithm module interfaces
	ISP_MODULE *isp_ae;
	ISP_MODULE *isp_awb;
	ISP_MODULE *isp_af;
	ISP_MODULE *isp_iq;
	ISP_MODULE *isp_ext;

	// statistics buffer
	ISP_CA_RSLT *ca_rslt[ISP_ID_MAX_NUM];
	ISP_LA_RSLT *la_rslt[ISP_ID_MAX_NUM];
	ISP_IPE_VA_RSLT *ipe_va_rslt[ISP_ID_MAX_NUM];
	ISP_IPE_VA_INDEP_RSLT *ipe_va_indep_rslt[ISP_ID_MAX_NUM];
	ISP_IFE_VA_RSLT *ife_va_rslt[ISP_ID_MAX_NUM];
	ISP_IFE_VA_INDEP_RSLT *ife_va_indep_rslt[ISP_ID_MAX_NUM];
	ISP_HISTO_RSLT *histo_rslt[ISP_ID_MAX_NUM];
	ISP_DEFOG_STCS defog_stcs[ISP_ID_MAX_NUM];
	ISP_HISTO_RSLT *histo_rslt_cap[ISP_ID_MAX_NUM];
	ISP_DEFOG_STCS defog_stcs_cap[ISP_ID_MAX_NUM];

	// ref image buffer
	ISP_IPE_SUBOUT_BUF *ipe_subout_buf[ISP_ID_MAX_NUM];
	ISP_IPE_SUBOUT_BUF *ipe_subout_buf_cap[ISP_ID_MAX_NUM];

	// capture usage
	BOOL is_capture[ISP_ID_MAX_NUM];
	UINT32 cap_id_mapping_table[ISP_ID_MAX_NUM];

	// mapping usage
	BOOL ipp_to_sen_id_map_en[ISP_ID_MAX_NUM];
	BOOL ipp_to_sie_id_map_en[ISP_ID_MAX_NUM];
	BOOL enc_to_sen_id_map_en[ISP_ID_MAX_NUM];
	BOOL enc_to_sie_id_map_en[ISP_ID_MAX_NUM];
	UINT32 ipp_to_sen_id_table[ISP_ID_MAX_NUM];
	UINT32 ipp_to_sie_id_table[ISP_ID_MAX_NUM];
	UINT32 enc_to_sen_id_table[ISP_ID_MAX_NUM];
	UINT32 enc_to_sie_id_table[ISP_ID_MAX_NUM];

	// sync item
	// NOTE: Start of frame cnt SyncInfo
	BOOL *p_sync_valid[ISP_ID_MAX_NUM];
	ISP_SYNC_INFO *p_sync_info[ISP_ID_MAX_NUM];
	// NOTE: end of frame cnt SyncInfo
	UINT32 capture_total_gain[ISP_ID_MAX_NUM];
	UINT32 capture_dgain[ISP_ID_MAX_NUM];
	UINT32 capture_cgain[ISP_ID_MAX_NUM][3];
	ISP_AE_INIT_INFO ae_init_info[ISP_ID_MAX_NUM];
	ISP_AWB_INIT_INFO awb_init_info[ISP_ID_MAX_NUM];
	ISP_IQ_INIT_INFO iq_init_info[ISP_ID_MAX_NUM];

	ISP_CA_TH_PARAM ca_th[ISP_ID_MAX_NUM];
	URECT ca_roi[ISP_ID_MAX_NUM];
	URECT la_roi[ISP_ID_MAX_NUM];
	URECT va_indep_roi[ISP_ID_MAX_NUM][ISP_INDEP_VA_WIN_NUM];
	URECT ife_va_indep_roi[ISP_ID_MAX_NUM][ISP_INDEP_VA_WIN_NUM];

	// motor control table
	CTL_MTR_DRV_TAB *mrt_drv_tab;

	#if defined(__KERNEL__)
	// synchronization mechanism
	wait_queue_head_t wait_vd[ISP_ID_MAX_NUM];
	wait_queue_head_t wait_frmend[ISP_ID_MAX_NUM];
	wait_queue_head_t wait_cfgstr[ISP_ID_MAX_NUM];
	wait_queue_head_t wait_procend[ISP_ID_MAX_NUM];

	// local variable
	ULONG ioctl_buf[IOCTL_BUF_SIZE];

	// synchronization mechanism
	struct semaphore api_mutex;
	struct semaphore ioc_mutex;
	struct semaphore proc_mutex;
	#endif
} ISP_DEV_INFO;

//=============================================================================
// extern functions
//=============================================================================
extern void isp_dev_reset_item_valid(ISP_ID id, UINT32 frame_cnt);
#if defined(__FREERTOS)
extern ISP_DEV_INFO *isp_get_dev_info(void);
#endif
extern INT32 isp_dev_construct(ISP_DEV_INFO *pdev_info);
extern void isp_dev_deconstruct(ISP_DEV_INFO *pdev_info);
extern void isp_dev_save_frame_count(UINT32 id);

#endif
