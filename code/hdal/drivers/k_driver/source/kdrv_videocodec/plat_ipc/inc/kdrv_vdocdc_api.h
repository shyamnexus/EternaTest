#ifndef _KDRV_VDOCDC_API_H_
#define _KDRV_VDOCDC_API_H_

#include "kwrap/type.h"

int kdrv_vdocdc_api_wt_sim(unsigned char argc, char **argv);
int kdrv_vdocdc_api_wt_dbg(unsigned char argc, char **argv);
int kdrv_vdocdc_api_wt_int(unsigned char argc, char **argv);
unsigned int kdrv_venc_get_dbg_level(void);
void kdrv_venc_set_dbg_level(unsigned int level);

extern int h264Enc_getH264NonTileI(void);
extern int h264Enc_setH264NonTileI(int value);

extern int h26xEnc_getCustQPPriority(void);
extern int h26xEnc_setCustQPPriority(int value);

#endif // _KDRV_VDOCDC_API_H_