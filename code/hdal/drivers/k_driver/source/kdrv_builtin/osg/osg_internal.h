#ifndef _OSG_INTERNAL_H_
#define _OSG_INTERNAL_H_

#include <kdrv_builtin/nvtmpp_init.h>
#include "osg_builtin.h"
#include "h26x_def.h"
#include "kdrv_videoenc/kdrv_videoenc.h"
#include "kdrv_videojpeg/kdrv_videoenc_jpeg.h"
#include "kdrv_gfx2d/kdrv_grph.h"
#include "kdrv_gfx2d/kdrv_grph_ctl.h"

typedef struct _BUILTIN_OSG_STAMP_BUF {
	OSG_BUF_TYPE           type;
	UINT32                 size;
	uintptr_t              p_addr[2];
	UINT32                 ddr_id;
	UINT8                  swap;
} BUILTIN_OSG_STAMP_BUF;

typedef struct _BUILTIN_OSG_STAMP {
	BUILTIN_OSG_STAMP_BUF  buf;
	OSG_STAMP_IMG          img;
	OSG_STAMP_ATTR         attr;
	UINT8                  valid;
} BUILTIN_OSG_STAMP;

typedef struct _BUILTIN_OSG {
	int                    num;
	BUILTIN_OSG_STAMP      stamp[BUILTIN_OSG_MAX_NUM];
} BUILTIN_OSG;

typedef struct _EXT_OSG {
	int                    num;
	BUILTIN_OSG_STAMP      stamp[EXT_OSG_MAX_NUM];
} EXT_OSG;

typedef struct _OSG_DST_BUFFER {
	int                    is_yuv420; //0: YUV422,1:YUV420
	int                    width[2];
	int                    loff[2];
	int                    height[2];
	uintptr_t              addr[2];
} OSG_DST_BUFFER;

typedef struct _OSG_GRPH_CB {
	ULONG              (*open)   (UINT32 chip , UINT32 eng);
	INT32              (*set)    (ULONG handle, enum kdrv_graph_param_id param_id, void *p_param);
	enum graph_drv_sts (*trigger)(ULONG handle, struct graph_drv_job_head *head);
	INT32              (*close)  (ULONG handle);
} OSG_GRPH_CB;

extern int fastboot_osg_setup_h26x_stamp(H26XENC_VAR *pVar, UINT32 path, VDOENC_BUILTIN_YUV_INFO *yuv);

extern int fastboot_osg_setup_jpeg_stamp(KDRV_VDOJPGE_PARAM *pVar, UINT32 path);

extern int fastboot_osg_demo_init(NVTMPP_FBOOT_MISC_CPOOL_S *pool);

#endif //_OSG_INTERNAL_H_
