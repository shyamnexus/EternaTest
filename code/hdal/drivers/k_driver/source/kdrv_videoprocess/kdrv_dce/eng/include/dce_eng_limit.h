/**
 * @file DCE_eng_limt.h
 * @brief parameter limitation of DCE
 * @author ISP
 * @date in the year 2021
 */

#ifndef _DCE_ENG_LIMIT_H_
#define _DCE_ENG_LIMIT_H_

//=====================================================
// input limitation
#define DCE_INPUT_W_MIN         (4)
#define DCE_INPUT_W_MAX         (8188)
#define DCE_INPUT_W_ALIGN       (4)

#define DCE_INPUT_H_MIN         (2)
#define DCE_INPUT_H_MAX         (8190)
#define DCE_INPUT_H_ALIGN       (2)

#define DCE_INPUT_LOFS_ALIGN    (DRV_LIMIT_ALIGN_WORD)
#define DCE_INPUT_ADDR_ALIGN    (DRV_LIMIT_ALIGN_WORD)

//=====================================================
// (cropping) output limitation
#define DCE_OUTPUT_W_MIN         (4)
#define DCE_OUTPUT_W_MAX         (8188)
#define DCE_OUTPUT_W_ALIGN       (4)

#define DCE_OUTPUT_H_MIN         (2)
#define DCE_OUTPUT_H_MAX         (8190)
#define DCE_OUTPUT_H_ALIGN       (2)

#define DCE_OUTPUT_CROP_X_ALIGN	 (2)

#define DCE_OUTPUT_LOFS_ALIGN    (DRV_LIMIT_ALIGN_WORD)
#define DCE_OUTPUT_ADDR_ALIGN    (DRV_LIMIT_ALIGN_WORD)

//=====================================================
// CFA Sub image limitation
#define DCE_CFA_SUBIMG_W_MIN          (8)
#define DCE_CFA_SUBIMG_W_MAX          (DCE_INPUT_W_MAX)
#define DCE_CFA_SUBIMG_W_ALIGN        (2)

#define DCE_CFA_SUBIMG_H_MIN          (8)
#define DCE_CFA_SUBIMG_H_MAX          (DCE_INPUT_H_MAX)
#define DCE_CFA_SUBIMG_H_ALIGN        (1)

#define DCE_CFA_SUBIMG_LOFS_ALIGN     (DRV_LIMIT_ALIGN_WORD)
#define DCE_CFA_SUBIMG_ADDR_ALIGN     (DRV_LIMIT_ALIGN_WORD)


//=====================================================
// WDR Sub image limitation
#define DCE_WDR_SUBIMG_W_MIN          (8)
#define DCE_WDR_SUBIMG_W_MAX          (48)
#define DCE_WDR_SUBIMG_W_ALIGN        (1)

#define DCE_WDR_SUBIMG_H_MIN          (8)
#define DCE_WDR_SUBIMG_H_MAX          (48)
#define DCE_WDR_SUBIMG_H_ALIGN        (1)

#define DCE_WDR_SUBIMG_LOFS_ALIGN     (DRV_LIMIT_ALIGN_WORD)
#define DCE_WDR_SUBIMG_ADDR_ALIGN     (DRV_LIMIT_ALIGN_WORD)

//=====================================================
// Other limitation
#define DCE_SST_MAX_WIDTH     (4096)

#endif
