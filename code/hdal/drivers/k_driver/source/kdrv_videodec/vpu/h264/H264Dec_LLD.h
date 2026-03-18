/*!
 *  \file       lld_H264Dec.h
 *              Low-level H264 Decoder driver for 78316/72568
 */

#ifndef _H264DEC_LLD_H_
#define _H264DEC_LLD_H_

#include "H264Dec_Tools.h"
#include "H264Dec_Reg.h"
#include "../../imvq.h"
#include "../vpu_comm.h"

typedef struct _ST_H264DEC_LLD_OBJ
{
    UINT32      u32VdecOpt;
    UINT32      u32HwTimeoutVal;

	DecLLJob *h264d_list_header;// using by link list mode
} ST_H264DEC_LLD_OBJ, *PST_H264DEC_LLD_OBJ;

typedef struct _H264D_HW_CFG
{
    // Bin/Ad share
    UINT32 PIC_SIZE;
    UINT32 SEQ_CFG;
    UINT32 PIC_CFG_I;
    UINT32 PIC_CFG_II;
    UINT32 PIC_TOP_POC;
    UINT32 PIC_BOTTOM_POC;

    // Bin
    UINT32          BIN_REC_LINE_OFFSET;
	UINT32          BIN_REC_2ND_LINE_OFFSET; //hk: extra write
    uintptr_t   	BIN_SCALING_MATRIX_VADDR;// virtual address
    uintptr_t   	BIN_RPIC_LIST_VADDR;// virtual address
    uintptr_t   	BIN_RPIC_DPB_VADDR;// virtual address
    uintptr_t   	BIN_BSDMA_CMD_VADDR;// virtual address
    uintptr_t   	BIN_BSDMA_CMD_VADDR_HI;// virtual address
    UINT32          BIN_DEC_MAX_LEN;         //!< Ad's output / Bin's input

    // err concealment settings
    UINT32 MAX_MVY_RANGE;

    // YUV Address
    uintptr_t       REC_Y_PADDR;	//hk: physical address
    uintptr_t       REC_UV_PADDR;	//hk: physical address
    uintptr_t       REC_COL_PADDR;	//hk: physical address

	uintptr_t       REC_Y_2ND_PADDR;	// physical address

    // RPIC_DPB_
    uintptr_t 		RpicDPBFrmAddr;
    UINT32 CHECKSUM;
    UINT32 mvcViewIndex;

	int chip_idx;
	unsigned int 	fw_disable;
	unsigned int	fw_uv_swap;
    unsigned int	extw_state;
    unsigned int	extw_mode;
    unsigned int 	extw_uv_swap;
	unsigned int	extw_sce_en;
    unsigned int	extw_y_size;
} ST_H264DEC_HW_CFG, *PST_H264DEC_HW_CFG;

//-----------------------------------------
// API Functions
//-----------------------------------------
void LLD_H264DEC_SetupRegisterBaseAddress(uintptr_t ulAddr, int chip_idx);
UINT32 LLD_H264DEC_Close(ST_H264DEC_LLD_OBJ *obj, int chip_idx);
void LLD_H264DEC_ResetAll(ST_H264DEC_LLD_OBJ *obj, int chip_idx);
void LLD_H264DEC_ResetBin(ST_H264DEC_LLD_OBJ *obj, int chip_idx);
BOOL LLD_H264DEC_TriggerBin(ST_H264DEC_LLD_OBJ *obj, ST_H264DEC_HW_CFG *pH264D_HW_CFG, ST_H264_MEM_POOL *pmWorkBuf);
void LLD_H264DEC_SetBinDone(int chip_idx);
UINT32 LLD_H264DEC_IsrBin(int chip_idx);
UINT32 LLD_H264DEC_GetBinCycle(int chip_idx);

#endif  //_H264DEC_LLD_H_


