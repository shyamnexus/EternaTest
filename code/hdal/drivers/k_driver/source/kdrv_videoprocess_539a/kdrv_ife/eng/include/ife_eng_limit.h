
#ifndef _IFE_ENG_LIMIT_H_
#define _IFE_ENG_LIMIT_H_


//=====================================================
// ife input limitation
#define IFE_SRCBUF_W_MIN         (64)
#define IFE_SRCBUF_W_MAX         (8188)
#define IFE_SRCBUF_W_ALIGN       (4)

#define IFE_SRCBUF_H_MIN         (64)
#define IFE_SRCBUF_H_MAX         (8188)
#define IFE_SRCBUF_H_ALIGN       (2)

#define IFE_SRCBUF_LOFS_ALIGN    (4)
#define IFE_SRCBUF_ADDR_ALIGN    (4)

//=====================================================
// ife output limitation

#define IFE_DSTBUF_W_MIN         (64)
#define IFE_DSTBUF_W_MAX         (8188)
#define IFE_DSTBUF_W_ALIGN       (4)

#define IFE_DSTBUF_H_MIN         (64)
#define IFE_DSTBUF_H_MAX         (8188)
#define IFE_DSTBUF_H_ALIGN       (2)

#define IFE_DSTBUF_LOFS_ALIGN    (4)
#define IFE_DSTBUF_ADDR_ALIGN    (4)

//=====================================================
// ife crop limitation
#define IFE_CROP_W_MIN         (64)
#define IFE_CROP_W_MAX         (8188)
#define IFE_CROP_W_ALIGN       (4)

#define IFE_CROP_H_MIN         (64)
#define IFE_CROP_H_MAX         (8188)
#define IFE_CROP_H_ALIGN       (2)

//=====================================================
// linked-list
#define IFE_LLBUF_ADDR_ALIGN     4

//=====================================================
/*
//ife fusion
#define IFE_FU_BCN_SEL_MAX       (3)
#define IFE_FU_BCD_SEL_MAX       (3)
#define IFE_FU_DBG_MODE_MAX      (6)
#define IFE_FU_BCNL_RANGE_MIN    (16)
#define IFE_FU_BCNS_RANGE_MIN    (16)
#define IFE_FU_BCDL_RANGE_MIN    (16)
#define IFE_FU_BCDS_RANGE_MIN    (16)
#define IFE_FU_MC_LUT_DW_MAX     (16)
#define IFE_FU_MC_LUT_DWD_MAX    (16)
//ife fcurve
#define IFE_FC_YM_SEL_MAX        (2)
#define IFE_FC_YVWEIGHT_MAX      (8)
#define IFE_FCURVE_EV_FMT_MAX    (4)
//ife RDE
#define IFE_RDE_SEGBITNO_MAX     (2)
//ife
*/

#endif

