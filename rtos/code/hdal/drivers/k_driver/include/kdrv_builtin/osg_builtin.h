#ifndef _OSG_BUILTIN_H_
#define _OSG_BUILTIN_H_

#include "kwrap/type.h"
#include "vdoenc_builtin.h"
#include "kdrv_gfx2d/kdrv_grph.h"
#include "kdrv_gfx2d/kdrv_grph_ctl.h"

#define BUILTIN_OSG_MAX_NUM 10
#define EXT_OSG_MAX_NUM     16

typedef enum _OSG_BUF_TYPE {
	OSG_BUF_TYPE_NULL,
	OSG_BUF_TYPE_SINGLE,
	OSG_BUF_TYPE_PING_PONG,
	OSG_BUF_TYPE_MAX,
	ENUM_DUMMY4WORD(OSG_BUF_TYPE)
} OSG_BUF_TYPE;

typedef enum _OSG_PXLFMT {
	OSG_PXLFMT_ARGB1555 = 0x21101555,
	OSG_PXLFMT_ARGB4444 = 0x21104444,
	OSG_PXLFMT_ARGB8888 = 0x21108888,
	ENUM_DUMMY4WORD(VDO_PXLFMT)
} OSG_PXLFMT;

typedef struct _OSG_STAMP_BUF {
	OSG_BUF_TYPE   type;
	UINT32         size;
	uintptr_t      p_addr;
	UINT32         ddr_id;
} OSG_STAMP_BUF;

typedef struct _OSG_STAMP_IMG {
	OSG_PXLFMT     fmt;
	ISIZE          dim;
} OSG_STAMP_IMG;

typedef struct _OSG_STAMP_ATTR {
	UINT32         alpha;
	IPOINT         position;
	UINT32         layer;
	UINT32         region;
} OSG_STAMP_ATTR;

typedef struct _OSG_STAMP {
	OSG_STAMP_BUF  buf;
	OSG_STAMP_IMG  img;
	OSG_STAMP_ATTR attr;
} OSG_STAMP;

typedef struct _OSG {
	int            num;
	OSG_STAMP      *stamp;
} OSG;

extern int vds_set_early_osg(UINT32 path, OSG *osg);

extern int vds_update_early_osg(UINT32 path, int idx, uintptr_t p_addr);

extern int vds_set_early_ext_osg(UINT32 path, OSG *osg);

extern int vds_update_early_ext_osg(UINT32 path, int idx, uintptr_t p_addr);

extern int osg_module_init(void);

extern int fastboot_osg_register_grph_cb(ULONG              (*open)   (UINT32 chip , UINT32 eng),
                                         INT32              (*set)    (ULONG handle, enum kdrv_graph_param_id param_id, void *p_param),
										 enum graph_drv_sts (*trigger)(ULONG handle, struct graph_drv_job_head *head),
										 INT32              (*close)  (ULONG handle));

#endif //_OSG_BUILTIN_H_
