#ifndef _VEMD_API_H_
#define _VEMD_API_H_

#include "kwrap/type.h"
#include "kdrv_videoenc.h"
#include "h26xenc_int.h"
#include "h264enc_api.h"
#include "h265enc_api.h"




typedef struct _vemd_func_t_{
	CODEC_TYPE eCodecType;
	
	H26XEncSliceSplit	stSliceSplit;
	H26XEncGdr			stGdr;
	H26XEncRoi			stRoi;
	H26XEncRowRc		stRowRc;
	H26XEncAq			stAq;
	H26XEncLpm			stLpm;
	h26XEncRnd			stRnd;
	H26XEncMotAq		stMAq;
	H26XEncJnd			stJnd;
	H26XEncQpRelatedCfg	stRqp;	
	H26XEncVar		    stVar;	
	H26XEncResSup		stResSup;
#if 0//H26X_SUPPORT_SMART_ROI
			H26XEncSmartRoi  stSmartRoi;
#endif
#if 0//H26X_SUPPORT_DYNAMIC_FR
			H26XEncDynFR		stDynFR;
#endif
#if 0//H26X_SUPPORT_DYNAMIC_GOP
			H26XEncDynGop		stDynGop;
#endif
	union {
		H264EncFroCfg st264;
		H265EncFroCfg st265;
	} stFro;

	union {
		H264EncRdo st264;
		H265EncRdo st265;
	} stRdo;

	H26XEncRCParam stRcParam;
} vemd_func_t;

typedef struct _vemd_info_t_{		
	BOOL enable;			
	
	int enc_id;
	int slice_en;
	int gdr_en;
	int roi_en;
	int rrc_en;	
	int aq_en;
	int lpm_en;
	int rnd_en;
	int maq_en;
	int jnd_en;
	int rqp_en;	
	int var_en;
	int fro_en;
	int rdo_en;

	int kdrv_aq_en;
	int kdrv_rc_en;
	int kdrv_rrc_en;
	int kdrv_maq_diff_en;
	int kdrv_jnd_en;
	int kdrv_bg_rdo_en;
	int kdrv_rdo_en;
#if 0//H26X_SUPPORT_SMART_ROI
	int kdrv_smart_roi_en;
#endif
#if 0//H26X_SUPPORT_DYNAMIC_FR
	int kdrv_dfr_en;
#endif
#if 0//H26X_SUPPORT_DYNAMIC_GOP
	int kdrv_dgop_en;
#endif
	vemd_func_t func;

	KDRV_VDOENC_AQ      kdrv_aq_cfg;
	KDRV_VDOENC_ROW_RC  kdrv_rrc_cfg;
	KDRV_VDOENC_MAQDIFF kdrv_maqdiff_cfg;
	KDRV_VDOENC_JND     kdrv_jnd_cfg;
	KDRV_VDOENC_BG_RDO  kdrv_bg_rdo_cfg;
	KDRV_VDOENC_RDO     kdrv_rdo_cfg;
#if 0//H26X_SUPPORT_SMART_ROI
			KDRV_VDOENC_SMART_ROI kdrv_smart_cfg;
#endif
#if 0//H26X_SUPPORT_DYNAMIC_FR
			KDRV_VDOENC_DYNAMIC_FR kdrv_dfr;
#endif
#if 0//H26X_SUPPORT_DYNAMIC_GOP
			KDRV_VDOENC_DYNAMIC_GOP kdrv_dgop;
#endif
} vemd_info_t;

extern vemd_info_t g_vemd_info;

int vemd_get_func_info(vemd_func_t *pstfunc);


#endif // _VEMD_API_H_
