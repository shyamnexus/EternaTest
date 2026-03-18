#ifndef __KDRV_IFE_DBG_H_
#define __KDRV_IFE_DBG_H_

#define __MODULE__	kdrv_ife
#define __DBGLVL__	8 // 0=FATAL, 1=ERR, 2=WRN, 3=UNIT, 4=FUNC, 5=IND, 6=MSG, 7=VALUE, 8=USER
#include "kwrap/debug.h"
#include "kwrap/perf.h"
#include "comm/hwclock.h"

extern unsigned int kdrv_ife_debug_level;

#define IFE_STATUS_RECORD_NUM (100)


typedef enum {
	KDRV_IFE_DBG_IFE_LINEBUF_STATUS_REC_EN = 0,
	KDRV_IFE_DBG_IFE_FUSION_DBG_EN         = 1,
	KDRV_IFE_DBG_SET_SSDRV_DBG_LV          = 2,
	KDRVO_IFE_DBB_SET_HDR_REF_CH           = 3,
	KDRV_IFE_DBG_PARAM_ID_MAX
} KDRV_IFE_DBG_PARAM_ID;

typedef enum{
	KDRV_IFE_DBG_STATUS_REC       = 0x00000001,
	KDRV_IFE_DBG_SET_HDR_OUT_MODE = 0x00000002,
	KDRV_IFE_DBG_SET_FUSION_REF   = 0x00000004,
}KDRV_IFE_DBG_FUNC;

typedef enum{
	KDRV_IFEDBG_HDR_REF_SIE1   = 0x00000001, //reference channel: sie1
	KDRV_IFE_DBG_HDR_REF_SIE2   = 0x00000002, //reference channel: sie2
	//KDRV_IFE_DBG_HDR_REF_SIE3   = 0x00000004, //reference channel: sie3
	KDRV_IFE_DBG_HDR_REF_UNKOWN = 0x00000008, //reference channel: unknown
}KDRV_IFE_DBG_HDR_REF_CHK;

typedef struct{
	UINT32  chip_id;
	UINT32  eng_id;
}KDRV_IFE_DBG_CMD_HDL;

typedef struct{

	BOOL    rec_en;
	INT32   rec_num;
	BOOL    bypass_isr_cb;

}KDRV_IFE_DBG_REC_CTL_PARAM;

typedef struct{

	BOOL    fusion_dbg_en;
	BOOL    fusion_set_fu_ref_en;
	UINT32   out_mode;

}KDRV_IFE_DBG_FUSION_OUT_CTL_PARAM;

typedef struct{

	BOOL fusion_set_fu_ref_en;
	KDRV_IFE_DBG_HDR_REF_CHK  hdr_ref_chk;

}KDRV_IFE_DBG_SET_FU_REF_PARAM;

typedef struct{
	KDRV_IFE_DBG_FUNC ife_dbg_fun;
	BOOL ife_bypass_isr_cb;
	UINT32 ife_fusion_out_mode;
	KDRV_IFE_DBG_HDR_REF_CHK hdr_ref_chk;
}KDRV_IFE_DBG_CTL;

#endif

