#ifndef __BUILTIN_IPP_DBG_INFO_H_
#define __BUILTIN_IPP_DBG_INFO_H_

#include "kdrv_ipp_builtin.h"

#define INPUT_PATH_ID_MAX (2)

/**
    FastBoot dbg information
 */
typedef struct{

	UINT32 *flow;         /*direct/d2d mode*/
	UINT32 input_w;
	UINT32 input_h;
	UINT32 sensor_cnt;   /* To exam drop frame from SIE to IME */
	UINT32 input_cnt;    /* To exam drop frame from SIE to IME */
	UINT32 output_cnt;   /* To exam drop frame from SIE to IME */
	UINT32 frc_drop_cnt[KDRV_IPP_BUILTIN_PATH_ID_MAX]; /* To exam frame rate control*/
	KDRV_IPP_BUILTIN_RATE_CTL *frc_info;               /* To exam frame rate control*/

	UINT32 input_frm_interval;    /* To exam SIE frame rate */
	UINT32 output_frm_interval;   /* To exam IPP frame rate control */

	KDRV_IPP_BUILTIN_IMG_INFO *output_info;       /* ipp output w/h */
	KDRV_IPP_BUILTIN_PRI_BUF *pri_buf_info;       /* wdr,3dnr,lca etc... buf addr info*/
	KDRV_IPP_BUILTIN_PHY_OUT_INFO *last_buf_info; /* ipp output buf addr info*/
	KDRV_IPP_BUILTIN_ISP_INFO *last_isp_info;     /* isp histogram etc. info*/

	UINT32 apply_fail_cnt;  /*Counter of register setting is not applied correctly next frame*/
	UINT32 intr_drop_cnt;
	UINT32 dtsi_chksum;
	UINT32 ch_cnt;
	UINT32 lock_cnt;
	UINT32 unlock_cnt;
	UINT32 _3dnr_lock_cnt;
	UINT32 _3dnr_unlock_cnt;
	UINT32 shdr_frm_num;

	UINT32 non_ring_release_idx[KDRV_IPP_BUILTIN_PATH_ID_MAX]; /* yuv buffer */
	UINT32 non_ring_release_cnt[KDRV_IPP_BUILTIN_PATH_ID_MAX];

} KDRV_IPP_BUILTIN_DBG_INFO;


#endif //__BUILTIN_IPP_DBG_INFO_H_

