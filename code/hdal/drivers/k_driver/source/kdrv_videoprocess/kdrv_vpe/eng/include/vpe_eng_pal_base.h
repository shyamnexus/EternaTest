#ifndef _VPE_ENG_PAL_BASE_H_
#define _VPE_ENG_PAL_BASE_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "kwrap/type.h"
#include "vpe_eng_handle.h"

typedef struct{
    UINT8 y;
    UINT8 cb;
    UINT8 cr;
} VPE_ENG_PAL_COLOR;

extern ER vpe_eng_set_palette_color_buf_reg(VPE_ENG_HANDLE *p_eng, UINT8 pal_idx, VPE_ENG_PAL_COLOR pal_color);

#ifdef __cplusplus
}
#endif


#endif //


