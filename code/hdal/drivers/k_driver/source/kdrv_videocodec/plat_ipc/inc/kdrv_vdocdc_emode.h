#ifndef __KDRV_VDOCDC_EMODE_H_
#define __KDRV_VDOCDC_EMODE_H_

#if defined(__LINUX)
#include "kdrv_vdocdc_ioctl.h"
#elif defined(__FREERTOS)

typedef struct _VENC_CH_INFO_{
	char kdrv_h26x_ver[16];
	int kdrv_builtin_ver;
	int emode_ver;
	int num;	
	int id[16];
} VENC_CH_INFO;
#endif
//#define KDRV_VDOCDC_EMODE_VERSION	(0x10000001)
#define KDRV_VDOCDC_EMODE_VERSION	(0x10000002) //update api of jnd
//#define KDRV_VDOCDC_EMODE_VERSION	(0x10000003) //update smart and fix evbr brt bug

int kdrv_vdocdc_emode_get_enc_info(VENC_CH_INFO *p_info);
int kdrv_vdocdc_emode_set_enc_id(unsigned int enc_id);
int kdrv_vdocdc_emode_read_cfg(char file_name[64]);
int kdrv_vdocdc_emode_write_cfg(char file_name[64]);


#endif	// __KDRV_VDOCDC_EMODE_H_

