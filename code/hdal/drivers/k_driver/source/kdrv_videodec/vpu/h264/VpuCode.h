#ifndef __H264_VPU_CODE_H__
#define __H264_VPU_CODE_H__

#include "../vpu_comm.h"

typedef struct ST_VPU_FRAME_INFO
{
    UINT8  video_full_range_flag;
    UINT8  bit_depth_luma_minus8;
    UINT8  bit_depth_chroma_minus8;
    UINT32 u32NumOfErrMbs;                         //The number of error blocks
    UINT32 u32CheckSum[2];                         //checksum value.  [0]:top field or frame checkSum, [1]:bottom field checkSum.
    UINT32 u32SeuqenceCounter;                     //The counter show how many sequence appear.
    UINT32 u32ProgSeq;                             //Source sequence's scan type is Progressive or Interlaced
    bool bFieldPicture;                         //The source picture type is frame or field
    bool bTopFieldFirst;
    bool bRepeatFirstField;
    UINT32 u32DecPicWidth;
    UINT32 u32DecPicHeight;
    INT32 s32PairedId;                            // >= 0 means that second field id used when TOP/BOT fields are in individual frame buffer.
    UINT32 u32Profile;

	UINT16 u16CropLeft;
	UINT16 u16CropRight;
	UINT16 u16CropTop;
	UINT16 u16CropBottom;
}ST_VPU_FRAME,*PST_VPU_FRAME;

typedef struct _ST_H264_VPU_CODEC_CONTEXT ST_H264_VPU_CODEC_CONTEXT,*PST_H264_VPU_CODEC_CONTEXT;

/* corresponding to _ST_VPU_CODEC_CONTEXT of fpga verification code */
struct _ST_H264_VPU_CODEC_CONTEXT
{
	/* add by CW */
	UINT32    u32DispClr;                 //write 1 to bit N means frame N has been displayed can be freed for decoding
	INT32     s32FrameDecoded;                //Flag the frame whether had been decoded
	INT8     release_buf_list[32];
	INT8     release_buf_count;
	INT8     display_buf_idx;
	DecLLJob  *vpu_list_header;  // using by link list mode
	INT32     chn_idx;
	INT32     chip_idx;

    void *priv;	//DRV_H264DEC_CTX
	
    //Set parameter to video codec
	ST_VPU_SET_BUFFER stSetBuff;        //Configure bistream buffer, frame buffer, working buffer for decoder.

    //Get information from video codec
    ST_VPU_FRAME stFrameBufInfo[32];    //32 frames are the max supported numbers for BODA
};

#endif
