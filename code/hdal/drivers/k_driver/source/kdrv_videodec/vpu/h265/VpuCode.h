#ifndef __H265_VPU_CODE_H__
#define __H265_VPU_CODE_H__

#include "../vpu_comm.h"

typedef struct _ST_H265_VPU_CODEC_CONTEXT ST_H265_VPU_CODEC_CONTEXT,*PST_H265_VPU_CODEC_CONTEXT;

struct _ST_H265_VPU_CODEC_CONTEXT
{
	/* add by CW */
	unsigned int	yuv_width_thrd;	/* width threshold of sub YUV enable */
	DecLLJob *vpu_list_header;  // using by link list mode
	/*-------------------------------------------------*/

    /* Set buffer addr to video codec */
	ST_VPU_SET_BUFFER stSetBuff;        /* frame buffer, mbinfo buffer, bitstream buffer for decoder */

	void *priv;
};

#endif

