
#ifndef __VPU_COMM_H__
#define __VPU_COMM_H__

#include "../h26xdec_ll.h"

typedef enum EN_VPU_STATUS
{
	EN_VPU_STATUS_SUCCESS = 0,
	EN_VPU_STATUS_FAIL,
	EN_VPU_STATUS_VPS_FAIL,
	EN_VPU_STATUS_SPS_FAIL,
	EN_VPU_STATUS_PPS_FAIL,
	EN_VPU_STATUS_LOSS_VPS_SPS_PPS_FAIL,
	EN_VPU_STATUS_SLICE_FAIL,
	EN_VPU_STATUS_BS_EMPTY_FAIL,
	EN_VPU_STATUS_UNKOWN_NALU_TYPE_FAIL,
	EN_VPU_STATUS_TRIGGER_FAIL,
	EN_VPU_STATUS_HW_TIMEOUT_FAIL,
}EN_VPU_STATUS, *PEN_VPU_STATUS;

/* moved from VpuCode.h because of sharing in both 264/265 */
typedef struct ST_VPU_SET_BUFFER
{
	uintptr_t 		FrameBufAddr;		/* physical address */
	unsigned int 	FrameBufSize;
	uintptr_t 		MbInfoBufAddr;		/* physical address */
	unsigned int 	MbInfoBufSize;
	uintptr_t 		FrameBufAddr2nd;	/* physical address, 2nd output */
	unsigned int 	FrameBufSize2nd;
	uintptr_t 		BitStreamBufAddr;	/* physical address */
	unsigned int 	BitStreamBufSize;
	unsigned int 	RealBitStreamSize;
	unsigned long 	BitStreamPhy2VirOffset;

	/* ---------------------------------------------------------------- */
	/* add by CW */
	int BufIndex;          // frame buffer index
} ST_VPU_SET_BUFFER;

#endif /* __VPU_COMM_H__ */