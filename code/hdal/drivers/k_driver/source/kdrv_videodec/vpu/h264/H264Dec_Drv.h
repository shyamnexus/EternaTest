#ifndef _DRV_H264DEC_H_
#define  _DRV_H264DEC_H_

// Common tools
#include "H264Dec_Tools.h"
#include "H264Dec_VLD.h"
// Picture buffer management
#include "H264Dec_Picbuf.h"
// Low-level driver
#include "H264Dec_LLD.h"
#include "H264Dec_Reg.h"
// VPU struct
#include "VpuCode.h"
#include "../vpu_comm.h"

/* first write hw spec */
#define H264D_FW_WIDTH_ALIGN		64	/* identical to H264D_FIRST_WIDTH_ALIGN in kdrv_videodec_lmt.h */
#define H264D_FW_HEIGHT_ALIGN		64 /* identical to H264D_FIRST_WIDTH_HEIGHT in kdrv_videodec_lmt.h */

#define PG_ALIGN(n) ((n + 0xFFF) & 0xFFFFF000) //PAGE_ALIGN// align n to a multiple of page size (4K)
#define H264_COL_SIZE(Y_sz) PG_ALIGN(((Y_sz)/16/16*20*4))

/* First Write HW alignment limitation. Identical to H264D_FIRST_WIDTH_ALIGN and H264D_FIRST_HEIGHT_ALIGN */
#define FW_Y_SIZE(x,y)			(ALIGN((x), H264D_FW_WIDTH_ALIGN) * ALIGN((y), H264D_FW_HEIGHT_ALIGN))

/* extwrite SCE buffer size */
#define EXTW_W_32(x)			ALIGN((x), 32)
#define EXTW_W_16(x)			((((x) + 15) >> 4) << 4)
#define EXTW_Y_SIZE(x,y)		((EXTW_W_16((EXTW_W_32(x) * 3) / 4)) * (y))

typedef enum DRV_H264DEC_t
{
    DRV_H264DEC_UNKNOW            = -999,
    DRV_H264DEC_NULL_HANDLE       = -987,
    DRV_H264DEC_ERR_NOT_EXPECTED  = -287,
    DRV_H264DEC_QUE_NOT_SYNC      = -187,
    DRV_H264DEC_ERR_GAPS_DROP	  = -8,
    DRV_H264DEC_FAIL              = -7,
    DRV_H264DEC_ERR_HW_STOPPED    = -4,
    DRV_H264DEC_ERR_BUF_ALLOCATE  = -3,
    DRV_H264DEC_NO_EMPTY_PICBUF   = -2,
    DRV_H264DEC_CMDBUF_EMPTY      = -1,
    DRV_H264DEC_OK                = 0,
    DRV_H264DEC_REF_ERR           = 1,
    DRV_H264DEC_DROP_NON_REF_B    = 2,
} DRV_H264DEC_STATUS;

typedef struct _SLICE_ENTRY
{
    uintptr_t  u32NalStartOffset;
    uintptr_t  u32NalEndOffset;
    UINT32  u32FirstMbInSlice;
}ST_SLICE_ENTRY;

/*!
 *  H.264 Decoder decoding context
 *  DRV_H264DEC uses a complete frame or field as "FRAME" unit
 */
#define LOG_INTERFACE_LENGTH 16
typedef struct DRV_H264DEC_CTX
{
    // Memory pool
    ST_H264_MEM_POOL    mWorkBuf;

    // LLD obj
    ST_H264DEC_LLD_OBJ  stLLDObj;       //!< Low-level driver object

    // Header decoding context
    ST_H264DEC_OBJ      stDecObj;

    // Picture buffer management
    ST_H264DEC_PICBUF   *pstPicBufObj;//!< picture buffer object, prepare the memory in H264Dec_Create() of avc_dec_entity.c, its size is only ST_H264DEC_PICBUF. And its member pstPicBufObj->pstPicInfo is also allocated int H264Dec_Create().

    // SW/HW CmdBuf Queue, and delayed queue (size = 1) for non-paired field checking
    ST_H264DEC_CMD_BUF  m_cmdbufs[LINK_LIST_NUM_JOBS];//ST_H264DEC_CMD_BUF  m_cmdbufs[H264DEC_MAX_CMDBUF_QUEUE_DEPTH];
	ST_H264DEC_CMD_BUF  m_cmdprebuf; //hk : to keep previous m_cmdbufs[0] in current picture
    que_s               m_cmdbuf_dec_que;       //!< Command buffer idx queue
    que_s               m_cmdbuf_free_que;  //!< Command buffer idx free-queue
    int                 preCmdBufIdx;       //!< Previous decoded cmdbuf idx, it is used for detecting consecutive fields of the same type. <0 implies empty entry
    int                 prePicInfoIdx;      //!< Previous allocated picinfo buffer idx, used for reconstruct bottom field slice

    // Private properties
    UINT32              u32Frm2ndStride; 	//! hk: extra wirte
    UINT32              u32FrmWidth;        //!< set by ScanHeader
    UINT32              u32FrmHeight;       //!< set by ScanHeader
    UINT32              u32MaxNumRefFrm;     //!< set by ScanHeader

    //VPU Motion info
    UINT32              u32Profile;         //!< set by ScanHeader
    UINT32              u32ProgSeq;         //!< set by ScanHeader??
#if !NOT_PARSING_H264_SEI
    INT32               s32PicStructure;    //!< set by scanHeader
#endif
    bool                bFieldPicture;      //!< set by scanHeader
    //VPU Common info
    UINT32              u32ChromaIdc;       //!< set by ScanHeader

    UINT16              u16CropLeft;        //!< set by ScanHeader
    UINT16              u16CropRight;       //!< set by ScanHeader
    UINT16              u16CropTop;         //!< set by ScanHeader
    UINT16              u16CropBottom;      //!< set by ScanHeader

    // TriggerHWDecode will use it
    int                 curCmdBufIdx;      //!< Current cmdbuf index used by decoder HW
    int                 curPicInfoIdx;     //!< Current picinfo index used by decoder HW

    int                 curDecPicId;       //!< Currently decoded picture ID
    ST_H264DEC_CMD_BUF* pstCurDecCmdBuf;   //!< ptr to current decoding command buffer context

    // State
    BOOL                bScanHeader;
    BOOL                bSeek;
    BOOL                bSeekAfterIdr;     // after seek I frame, B frame poc may reference error, so drop it, but IDR not.
    // BitStream Status
    BOOL                bStreamEnd;
    int                 preDecIdx;
    uintptr_t       	ulPicStartAddr;    //physical address report to va for search PTS.
    uintptr_t       	ulPicEndAddr;      //physical address report to va for update Read pointer.
    uintptr_t       	ulPicTmpAddr;		  //hk: for stress test random bsdma
    unsigned int       u32TotalBsSizeTmp;    //hk: for stress test random bsdma
#if !NOT_PARSING_H264_SEI
    ST_H264DEC_SEI      stDecSeiData;
#endif
    ST_SLICE_ENTRY      stSliceEntry[H264_BSDMA_CMD_LEN];
    UINT32              u32TotalBsSize;
    uintptr_t           u32BSDmaCmds[LINK_LIST_NUM_JOBS][H264_BSDMA_CMD_LEN*2+1];

    // variables used in VPU layer
    UINT32              scanHeaderFlag;                  //!< for header decoding
    UINT32              scanHeaderCnt;

    UINT32              bsRingBufferBegin;
    UINT32              bsRingBufferEnd;
    UINT32              bsRingBufferSize;

    BOOL                bIsBinInterrupted;

    UINT32              u32BinIntStatus;

    //tmperoal used, get info in ISR and set to picture in DecFinish
    UINT32              u32CheckSum;
    UINT32              u32ErrorMBs;

	/*------- add by CW ---------*/
    UINT32  *pu32DecQue;
    UINT32  *pu32FreeQue;
	int		chip_idx;
	int		chn_idx;

	unsigned int 	fw_disable;
	unsigned int 	fw_uv_swap;
    unsigned int	extw_state;
    unsigned int	extw_mode;
    unsigned int 	extw_uv_swap;
	unsigned int	extw_sce_en;
    unsigned int	extw_y_size;
    unsigned int	extw_stride;
	void *parent; //PST_VPU_CODEC_CONTEXT
} DRV_H264DEC_CTX;

/*!
 *  setup HW register Base Address
 *  \return     Driver context object(handle)
 */
void DRV_H264DEC_SetupRegisterBaseAddress(uintptr_t uiAddr, int chip_idx);

/*!
 *  Re-Initialize the decoding buffer address
 *  \param[in]  ctx             Driver context object(handle)
 */
void DRV_H264DEC_BufAddrReSet(DRV_H264DEC_CTX *ctx);

/*!
 *  Initialize the decoding context, and memsh managers
 *  \param[in]  ctx             Driver context object(handle)
 */
int DRV_H264DEC_Init(DRV_H264DEC_CTX *ctx);

/*!
 *  Re-initialize the decoding context
 *  \param[in]  ctx     Driver context object(handle)
 */
void DRV_H264DEC_ReInit(DRV_H264DEC_CTX *ctx);

ProcessNalStatus DRV_H264DEC_ProcessSPS(DRV_H264DEC_CTX *ctx, UINT32 u32Id);
ProcessNalStatus DRV_H264DEC_ProcessNal(DRV_H264DEC_CTX *ctx);
/*!
 *  Trigger hardware to decode/reconstruct one frame/field
 *  \param[in]  ctx         Driver context object(handle)
 *  \return     0 if success, otherwise failed
 */
int DRV_H264DEC_TriggerBinDecode(DRV_H264DEC_CTX *ctx);

/*!
 *  This function should be called after PIC_DONE
 *  \param[in]  ctx     Driver context object(handle)
 *  \param[out] bs_release_point    BS buffer release point
 *  \return     0 if success, otherwise failed
 */
int DRV_H264DEC_FinishBinDecode(DRV_H264DEC_CTX *ctx);

/*!
 *  Allocate frame buffers / col-located information buffers (using memsh)
 *  \param[in]  ctx             Driver context object(handle)
 *  \param[in]  yuv_size        first write buffer size
 *  \param[in]  ex_yuv_size     extra write buffer size
 *  \param[in]  mbinfo_size     mbinfo buffer size
 */
int DRV_H264DEC_SetupPictureBuffer(DRV_H264DEC_CTX *ctx, UINT32 yuv_size, UINT32 ex_yuv_size, INT32 mbinfo_size);

/*!
 *  Set the frame stride of the decoder
 *  \param[in]  ctx             Driver context object(handle)
 *  \param[in]  frame_stride    Rowstride(or lineoffset), it should
 *                              be the multiple of 4
 */
void DRV_H264DEC_SetFrame2ndStride(DRV_H264DEC_CTX *ctx, UINT32 u32FrameStride);

/*!
 *  Get the frame stride of the decoder
 *  \param[in]  ctx             Driver context object(handle)
 *  \param[in]  decIndex        decoded index;
 *  \return                     number of error mbs.
 */
int DRV_H264DEC_GetErrorMbs(DRV_H264DEC_CTX *ctx);
/*!
 *  Get the frame stride of the decoder
 *  \param[in]  ctx             Driver context object(handle)
 *  \param[in]  decIndex        decoded index;
 *  \return                     checksum value.
 */
void DRV_H264DEC_GetCheckSum(DRV_H264DEC_CTX *ctx, UINT32 u32CheckSum[2]);

/*!
 *  Get an output frame and its info
 *  \param[in]  ctx             Driver context object(handle)
 *  \return     -1 if no frame to output, otherwise upper-layer id
 */
int DRV_H264DEC_GetOutputFrame(DRV_H264DEC_CTX *ctx);

/*!
 *  Get how many buffer in used.
 *  \param[in]  ctx             Driver context object(handle)
 *  \return     how many buffer in used.
 */
int DRV_H264DEC_GetFrameBufCnt(DRV_H264DEC_CTX *ctx);
/*!
 *  Get the decoded frame upper id
 *  \param[in]  ctx             Driver context object(handle)
 *  \return     -1 if no frame is decoded, otherwise upper-layer id
 */
int DRV_H264DEC_GetDecodedFrame(DRV_H264DEC_CTX *ctx);

/*!
 *  Set the frame upper_id as displayed
 *  \param[in]  ctx             Driver context object(handle)
 *  \param[in]  upper_id        Frame upper id (e.g. frame idx)
 */
void DRV_H264DEC_SetFrameDisplayed(DRV_H264DEC_CTX *ctx, int vaIndex);

/*!
 *  New bitstream is incoming, reset some internal states
 *  \param[in]  ctx             Driver context object(handle)
 *  \return     TRUE if the cmdbuf queue is full, otherwise FALSE
 */
BOOL DRV_H264DEC_IsAllCmdBufEmpty(DRV_H264DEC_CTX *ctx);
BOOL DRV_H264DEC_IsCmdBufFull(DRV_H264DEC_CTX *ctx);
UINT32 DRV_H264DEC_IsrBin(int chip_idx);
void DRV_H264DEC_SetBinDone(DRV_H264DEC_CTX *ctx);
UINT32 DRV_H264DEC_GetBinCycle(int chip_idx);

void DRV_H264DEC_ReinitObjBuffer(DRV_H264DEC_CTX *ctx);

#endif  // _DRV_H264DEC_H_

