#ifndef _H264DEC_VLD_H_
#define _H264DEC_VLD_H_

#include "H264Dec_Tools.h"
#include "H264Dec_LLD.h"
#include "../../vps/h264/H264PS_DrvVLD.h"

typedef struct _ST_H264DEC_FrmStore
{
    UINT8 u8UsedForRef;
    BOOL  bIsLongTerm;
    BOOL  bIsExist;
    BOOL  bFld;
    BOOL  bTopLtBtm;
    BOOL  bFieldPic;

    // For Short-Term //
    UINT32 u32FrmNum;
    INT32  i32PicNum;
    INT32  i32TopPicNum;
    INT32  i32BtmPicNum;

    // For Long-Term //
    UINT32 u32LongTermFrmIdx;

    INT32  i32Poc;
    INT32  i32TopPoc;
    INT32  i32BtmPoc;

    UINT8  u8DPBIdx;
    UINT32 u32DecPicNum;

    UINT8  u8FSBIdx;
}ST_H264DEC_FrmStore, *PST_H264DEC_FrmStore;

typedef struct _ST_H264DEC_DecFrmBuf
{
    UINT8 u8MaxBufSize;
    UINT8 u8RefFrmNum;
    UINT8 u8ShortTermFrmNum;
    UINT8 u8LongTermFrmNum;
    UINT8 u8DecDPBFrmNum;

    ST_H264DEC_FrmStore stST_FSB[(H264_MAX_FRM_BUF-1)];
    ST_H264DEC_FrmStore stLT_FSB[(H264_MAX_FRM_BUF-1)];
    ST_H264DEC_FrmStore stFSB[H264_MAX_FRM_BUF];
}ST_H264DEC_DecFrmBuf;


typedef struct _H264DEC_OBJ
{
//    ST_H264DEC_NAL          stCurNalUnit;
    PST_H264DEC_SeqParSet  pstSPS; // +1 allocate for parsing tmp used
    PST_H264DEC_PicParSet  pstPPS; // +1 allocate for parsing tmp used
    ST_H264DEC_SeqParSet    *pActiveSPS;

    ST_H264DEC_SLICE        stCurSlice;    // maintain in VPU
    ST_H264DEC_SLICE        stPsSlice;     // from VPS.
#if !NOT_PARSING_H264_SEI
    ST_H264DEC_SEI          stSei;
#endif
    UINT32  u32DecPicNum;
    uintptr_t  u32HwBsAddr;
    UINT32  u32HwBsLen;

    UINT32  u32MaxNumRefFrm;
    UINT32  u32MaxFrmNum;
    UINT32  u32FrmWidth;
    UINT32  u32FrmHeight;
    UINT32  u32PreFrmWidth;
    UINT32  u32PreFrmHeight;
    BOOL    bResetAll;

    // Calculate Poc //
    INT32   i32PrePocMsb;
    INT32   i32PrePocLsb;
    INT32   i32PocMsb;
    UINT32  u32FrmNumOffset;
    UINT32  u32PreFrmNumOffset;
    UINT32  u32AbsFrmNum;
    INT32   i32ExpectedDeltaPerPocCycle;
    INT32   i32ExpectedPoc;
    UINT32  u32PocCycleCnt;
    UINT32  u32FrmNumInPocCycle;
    BOOL    bLastHasMmco5;
    INT32   i32Poc;
    INT32   i32TopPoc;
    INT32   i32BtmPoc;

    UINT8   u8FrmBufIdx;
    ST_H264DEC_FrmStore  *pCurFrm;
    ST_H264DEC_DecFrmBuf stDecFrmBuf;
    ST_H264DEC_FrmStore H264Ref_List0[32];
    ST_H264DEC_FrmStore H264Ref_List1[32];
    ST_H264DEC_FrmStore H264FS_List0[16];
    ST_H264DEC_FrmStore H264FS_List1[16];
    ST_H264DEC_FrmStore H264FS_ListLT[16];
    // TODO : change u8Qmat to ptr
    UINT8   u8Qmat[H264DEC_QMATSIZE];  //!< will be copied to SW/HW-buf
    UINT32  *pu32DEC_DPB_FRAME_BUF;    //!< pointed to SW/HW-buf. Store the frame number
    UINT32  u32RPIC_REF_LIST_BUF[H264_RPIC_REF_LIST_SIZE];   // TODO: 72568 need +3. not store the address.

    // Sei queue from VPS, put in vpu_mvc, get in collect_picinfo
#if !NOT_PARSING_H264_SEI
    UINT32          s32PicStructure;
    ST_H264DEC_SEI  stSeiQue[H264DEC_SEI_QUE_LENGTH];
    UINT32          u32SeiQueRIdx;  // R == W       ==> Empty, inital R = W = 0
    UINT32          u32SeiQueWIdx;  // W == R -1    ==> FULL
#endif

    // handle sequence info.
    UINT32  u32SpsChangedCounter;
    UINT32  u32PreSPSid;
    UINT32  u32PicCntInSps;
    UINT32  u32PicCntInIPic;
    // flag for scan header early terminated.
    BOOL    bScanHeader;
    BOOL    bFieldPicture;
}ST_H264DEC_OBJ, *PST_H264DEC_OBJ;

typedef struct _ST_H264DEC_CMD_BUF
{
    // keep cur cmd buf index
    UINT32          u32CmdBufIdx;
    // Use For Software Decode Header Only //
    UINT32          *pu32DecDPBFrm;      //!< Intermediate type between H264D_DecFrmBuf and puiRpicDPBFrmVa \ref H264DecModifyDPBFrmBuf
	UINT32          *pu32DecDPBFrm_msb;

    // Use For BIN HW Decode //
    UINT32    		*pu32BinCmdBuf;    	//!< Command buffer array of size (1+n*2)*32-bit
//#ifdef __aarch64__
    UINT32			*pu32BinCmdBufHi;   //!< Command buffer array of size (1+n*2)*32-bit
//#endif
    UINT8           *pu8Qmat;           //!< QMatrix array of size \ref H264D_QMATSIZE
    UINT32 		 	*pu32RpicRefList;    //!< Reference list size \ref H264D_RPIC_REF_LIST_SIZE
	UINT32          *pu32RpicRefList_msb;

    UINT32          *pu32RpicDPBFrm;
	UINT32          *pu32RpicDPBFrm_msb;

	/* LLC_INCLUDED */
	UINT32	        *pu32RpicLLCInfoList;
	UINT32	        *pu32RpicLLCInfoList_msb;

    // HW_CFG
    ST_H264DEC_HW_CFG    stHwCfg;     //!< HW config structure

    // For Verification
    UINT32          u32CheckSum; //!< Checksum of the current frame

    // Picture type, and Mark for output directly
    UINT32          u32FrameNum;          //!< used for check new frame.
    BOOL            bFieldPicFlag;      //!< used for check new frame.
    UINT32          u32FieldType;        //!< 0:Frame, 1: Top Field, 2: Bottom Field
    BOOL            bReusePrevFB;       //!< Set to TRUE only if current field is non-paired top-field, otherwise FALSE
    BOOL            bOutputFlag;        //!< indicate decoding completely(complete decoding frame or top+bot fields) and thus ready for display
    INT32           s32Poc;             //!< POC
    UINT32          u32DecPicNum;       //!< decoded picture number
    UINT8           u8DecDPBFrmNum;     //!< # of reference frames in DPB
    BOOL            bUsedForRef;        //!< used for ref flag
    BOOL            bIdrFlag;           //!< is IDR or not
    BOOL            bLastHasMmco5;      //!< Previous picture(decode order) has mmco5 or not
    BOOL            bResetAll;          //

    //VPU framebuf info
    UINT32          u32PicType;
    UINT32          u32ProgSeq;

    BOOL            bTopFieldFirst;
    BOOL            bRepeatFirstField;
    UINT32          u32DecPicWidth;
    UINT32          u32DecPicHeight;
    UINT32          u32Profile;
    UINT8           u8MaxNumRefFrm;
    UINT8           u8SliceType;
    UINT8           u8video_full_range_flag;
    BOOL            bIsFieldPicture;
    UINT32          u32RefMask;

    //deal with non-paired fields
    BOOL            bIsFirstField;

    // h264 uset data this data structure pointer will past to VA.
#if !NOT_PARSING_H264_SEI
    UINT32          u32TotalSeiDataSize;
    ST_H264DEC_SEI  stSeiData;
#endif
    ST_VPU_OUTPUT_INFO stCropInfo; //Get Cropping information for decoded picture from video decoder

    // h264 sequence conter
    UINT32          u32SequenceCounter;
    // used for 4:2:0->4:0:0(dynamically) need clear UV buffer.
    UINT32          u32ChromaIdc;

} ST_H264DEC_CMD_BUF, *PST_H264DEC_CMD_BUF;

void UnmarkShortTermForRef(ST_H264DEC_OBJ *pH264DecObj,INT32 i32DifferenceOfPicNumsMinus1);
void UnmarkLongTermForRef(ST_H264DEC_OBJ *pH264DecObj,INT32 i32LongTermPicNum);
void UnmarkLongTermFieldForRefByFrmIdx(ST_H264DEC_OBJ *pH264DecObj,H264Dec_ePicStructure ePicStructure,INT32 i32PicNum_X,UINT32 u32LongTermFrmIdx,BOOL bMarkCurrent);
void AssignLongTermFrmIdx(ST_H264DEC_OBJ *pH264DecObj,INT32 i32DifferenceOfPicNumsMinus1,UINT32 u32LongTermFrmIdx);
void UpdateMaxFrmIdxInLongTerm(ST_H264DEC_OBJ *pH264DecObj,UINT32 u32MaxLongTermFrmIdxPlus1);
void ClearAllFrmInRefList(ST_H264DEC_OBJ *pH264DecObj);
void MarkCurFrmToLongTerm(ST_H264DEC_OBJ *pH264DecObj,UINT32 u32LongTermFrmIdx);
void H264DecUpdateLongTermRefFSB(ST_H264DEC_OBJ *pH264DecObj);
void AdaptiveMemoryManagement(ST_H264DEC_OBJ *pH264DecObj);

void H264DecFillFrmNumGap(ST_H264DEC_OBJ *pH264DecObj);
void H264DecModifyDPBFrmBuf(ST_H264DEC_OBJ *pH264DecObj);
void H264DecInitRefList(ST_H264DEC_OBJ *pH264DecObj);
void H264DecModifyRefListBuf(ST_H264DEC_OBJ *pH264DecObj);
void H264DecUpdateRefFrmBuf(ST_H264DEC_OBJ *pH264DecObj);
void H264DecUpdateRefFSB(ST_H264DEC_DecFrmBuf *pDecFrmBuf);


void H264DecCalculatePoc(ST_H264DEC_OBJ *pH264DecObj, ST_H264DEC_FrmStore *pCurFrm);
void H264DecHeaderInit(ST_H264DEC_OBJ *pH264DecObj);
BOOL H264DecHRD(bstream *pBitstream,ST_H264DEC_HRD *pH264D_Hrd);
BOOL H264DecSEI(ST_H264DEC_OBJ *pH264DecObj,  ST_H264DEC_CMD_BUF *pH264D_CMD_BUF);
ProcessNalStatus H264DecSlice(ST_H264DEC_OBJ *pH264DecObj, BOOL bSeek);
ProcessNalStatus H264DecHeader(ST_H264DEC_OBJ *pH264DecObj, ST_H264DEC_CMD_BUF *pH264D_CMD_BUF, UINT32 u32Seek);

#endif // _H264DEC_VLD_H_
