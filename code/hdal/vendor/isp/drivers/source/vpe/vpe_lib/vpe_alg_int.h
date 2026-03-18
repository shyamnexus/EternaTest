#ifndef _VPE_ALG_INT_H_
#define _VPE_ALG_INT_H_

#include "vpe_alg.h"

//=============================================================================
// struct & definition
//=============================================================================
#define VPE_IDX_MAX_NUM          4

typedef struct _VPE_2DLUT_INT_PARAM {
	VPE_ISP_2DLUT_SZ lut_sz;
	ULONG lut_addr;
	UINT8 lut2d_precision;
} VPE_2DLUT_INT_PARAM;

typedef struct _VPE_PARAM_PTR {
	VPE_DCE_CTL_PARAM       *dce_ctl;
	VPE_SHARPEN_PARAM       *sharpen;
	VPE_2DLUT_INT_PARAM     *lut2d_int;
	VPE_2DLUT_EXPAND_PARAM  *lut2d_expand;
	VPE_DRT_PARAM           *drt;
	VPE_DCTG_CTRL           *dctg;
	VPE_FLIP_ROT_PARAM      *flip_rot;
} VPE_PARAM_PTR;

#endif

