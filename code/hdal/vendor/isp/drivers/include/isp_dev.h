#ifndef _ISP_DEV_H_
#define _ISP_DEV_H_

#include "kwrap/type.h"
#include "isp_api.h"
#include "isp_mtr.h"

//=============================================================================
// struct & definition
//=============================================================================
#define ISP_MAX_NAME_SIZE       16

typedef enum _ISP_SYNC_SEL {
	ISP_SYNC_SEL_SIE,       ///< Sync Info from SIE EVENT
	ISP_SYNC_SEL_IPP,       ///< Sync Info from IPP EVENT
	ISP_SYNC_SEL_ENC,       ///< Sync Info from ENC EVENT
	ISP_SYNC_SEL_CURRENT,   ///< Sync Info from CURRENT
	ISP_SYNC_SEL_MAX,
	ENUM_DUMMY4WORD(ISP_SYNC_SEL_MAX)
} ISP_SYNC_SEL;

typedef enum _ISP_SYNC_ITEM {
	// NOTE: Start of frame cnt SyncInfo
	ISP_SYNC_AE_STATUS           = 0,///< Sync from AE
	ISP_SYNC_AE_TOTAL_GAIN,          ///< Sync from AE
	ISP_SYNC_AE_DGAIN,               ///< Sync from AE
	ISP_SYNC_AE_LV,                  ///< Sync from AE
	ISP_SYNC_AE_LV_BASE,             ///< Sync from AE
	ISP_SYNC_AE_SHDR_EV_RATIO    = 5,///< Sync from AE
	ISP_SYNC_AE_SHDR_TM_RATIO,       ///< Sync from AE
	ISP_SYNC_AE_LA_ENABLE,           ///< Sync from AE
	ISP_SYNC_AWB_CGAIN,              ///< Sync from AWB
	ISP_SYNC_AWB_CT,                 ///< Sync from AWB
	ISP_SYNC_AWB_CA_ENABLE      = 10,///< Sync from AWB
	ISP_SYNC_IQ_FINAL_CGAIN,         ///< Sync from IQ
	ISP_SYNC_AE_SHDR_HBS,            ///< Sync from AE
	ISP_SYNC_FRAME_CNT_MAX      = 20,
	// NOTE: end of frame cnt SyncInfo
	ISP_SYNC_CAPTURE_TOTAL_GAIN,     ///< Sync from AE
	ISP_SYNC_CAPTURE_DGAIN,          ///< Sync from AE
	ISP_SYNC_CAPTURE_CGAIN,          ///< Sync from AWB
	ISP_SYNC_AE_INIT,                ///< Sync from AE or buildin AE
	ISP_SYNC_AWB_INIT           = 25,///< Sync from AWB or buildin AWB
	ISP_SYNC_IQ_INIT,                ///< Sync from buildin ISP or buildin IQ
	ISP_SYNC_CA_TH,                  ///< Sync from AWB
	ISP_SYNC_LA_ROI,                 ///< Sync from AE
	ISP_SYNC_CA_ROI,                 ///< Sync from AWB
	ISP_SYNC_VA_INDEP_ROI       = 30,///< Sync from AF
	ISP_SYNC_IFE_VA_INDEP_ROI,       ///< Sync from AF
	ISP_SYNC_ITEM_MAX,
	ENUM_DUMMY4WORD(ISP_SYNC_ITEM_MAX)
} ISP_SYNC_ITEM;

typedef struct _ISP_MODULE {
	CHAR     name[ISP_MAX_NAME_SIZE];        // module name
	void     *private;                       // private date
	INT32    (*fn_init)      (UINT32 id);
	INT32    (*fn_uninit)    (UINT32 id);
	void     (*fn_trig)      (UINT32 id, void *arg);
	INT32    (*fn_pause)     (UINT32 id);
	INT32    (*fn_resume)    (UINT32 id);
	INT32    (*fn_ioctl)     (INT32 cmd, ULONG arg, ULONG *buf_addr);
} ISP_MODULE;

//=============================================================================
// extern functions
//=============================================================================
extern ER isp_dev_get_sync_item(ISP_ID id, ISP_SYNC_SEL sel, ISP_SYNC_ITEM item, void *data);
extern void isp_dev_set_sync_item(ISP_ID id, ISP_SYNC_SEL sel, ISP_SYNC_ITEM item, void *data);
extern ISP_CA_RSLT *isp_dev_get_ca(ISP_ID id);
extern ISP_LA_RSLT *isp_dev_get_la(ISP_ID id);
extern ISP_IPE_VA_RSLT *isp_dev_get_ipe_va(ISP_ID id);
extern ISP_IPE_VA_INDEP_RSLT *isp_dev_get_ipe_va_indep(ISP_ID id);
extern ISP_IFE_VA_RSLT *isp_dev_get_ife_va(ISP_ID id);
extern ISP_IFE_VA_INDEP_RSLT *isp_dev_get_ife_va_indep(ISP_ID id);
extern ISP_HISTO_RSLT *isp_dev_get_histo(ISP_ID id);
extern ISP_DEFOG_STCS *isp_dev_get_defog_stcs(ISP_ID id);
extern ISP_IPE_SUBOUT_BUF *isp_dev_get_ipe_subout_buf(ISP_ID id);
extern CTL_MTR_DRV_TAB *isp_dev_get_mtr_drv_tab(void);
extern void isp_dev_reg_ae_module(ISP_MODULE *ae_module);
extern void isp_dev_reg_af_module(ISP_MODULE *af_module);
extern void isp_dev_reg_awb_module(ISP_MODULE *awb_module);
extern void isp_dev_reg_iq_module(ISP_MODULE *iq_module);
extern void isp_dev_reg_motor_driver(CTL_MTR_DRV_TAB *mrt_drv_tab);
extern void isp_dev_wait_vd(ISP_ID id, UINT32 timeout, UINT32 *frame_count);
extern void isp_dev_wait_frmend(ISP_ID id, UINT32 timeout, UINT32 *frame_count);
extern void isp_dev_wait_cfgstr(ISP_ID id, UINT32 timeout, UINT32 *frame_count);
extern void isp_dev_wait_procend(ISP_ID id, UINT32 timeout, UINT32 *frame_count);

#endif

