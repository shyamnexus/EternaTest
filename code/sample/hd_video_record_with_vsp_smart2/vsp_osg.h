#ifndef _VSP_OSG_H_
#define _VSP_OSG_H_
#include "hd_type.h"

#define OSG_TYPE_VIDEOPROC		0
#define OSG_TYPE_VIDEOENC		1
#define OSG_TYPE_VIDEOOUT		2


#define STREAM1_STAMP_W          ALIGN_CEIL(7824, 8)
#define STREAM1_STAMP_H          ALIGN_CEIL(3470, 4)
#define STREAM1_STAMP_PXLFMT	 HD_VIDEO_PXLFMT_ARGB4444
#define STREAM1_STAMP_X   	    (0)
#define STREAM1_STAMP_Y  		(0)
#define STREAM1_STAMP_C 		((0x1 << 15) | (0x1f << 10) | (0x0f << 5) | (0x0f)) //red

#define STREAM2_STAMP_W          ALIGN_CEIL(3840, 8)
#define STREAM2_STAMP_H          ALIGN_CEIL(1704, 4)
#define STREAM2_STAMP_PXLFMT	 HD_VIDEO_PXLFMT_ARGB4444
#define STREAM2_STAMP_X   	    (0)
#define STREAM2_STAMP_Y  		(0)
#define STREAM2_STAMP_C 		((0x1 << 15) | (0x1f << 10) | (0x0f << 5) | (0x0f)) //red

#define STREAM3_STAMP_W          ALIGN_CEIL(1920, 8)
#define STREAM3_STAMP_H          ALIGN_CEIL(852, 4)
#define STREAM3_STAMP_PXLFMT	 HD_VIDEO_PXLFMT_ARGB4444
#define STREAM3_STAMP_X   	    (0)
#define STREAM3_STAMP_Y  		(0)
#define STREAM3_STAMP_C 		((0x1 << 15) | (0x1f << 10) | (0x0f << 5) | (0x0f)) //red


#define OSG_DDR_ID		         DDR_ID0

typedef struct _VSP_OSG {
	// osg
	HD_PATH_ID vprc_stamp_path;
	HD_PATH_ID venc_stamp_path;
	HD_PATH_ID vout_stamp_path;

	HD_DIM stamp_dim;
	UINT32 stamp_fmt;
	UINTPTR         stamp_pa;
	HD_COMMON_MEM_VB_BLK stamp_blk;
	UINT32          stamp_size;
	UINTPTR         stamp_data_pa;
	UINTPTR         stamp_data_va;
	HD_COMMON_MEM_VB_BLK stamp_data_blk;

} VSP_OSG;



////////////////////////////

int osg_query_buf_size(UINT32 w, UINT32 h, HD_VIDEO_PXLFMT pxlfmt);

HD_RESULT osg_init(UINT32 id, VSP_OSG* osg, UINT32 w, UINT32 h, HD_VIDEO_PXLFMT pxlfmt, unsigned short c);
HD_RESULT osg_open(UINT32 id, VSP_OSG* osg, UINT32 type, UINT32 venc_in);
HD_RESULT osg_start(UINT32 id, VSP_OSG* osg, UINT32 type, UINT32 x, UINT32 y);
HD_RESULT osg_close(UINT32 id, VSP_OSG* osg, UINT32 type);
HD_RESULT osg_uninit(UINT32 id, VSP_OSG* osg);
HD_RESULT osg_update(UINT32 id, VSP_OSG* osg, char *pattern);
#endif
