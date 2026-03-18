/*!
 *  \file   h264dec_picbuf.h
 *          H264 picture buffer management
 *
 *  \author mobo <Mobo_Chuang@novatek.com.tw>
 *
 *  \date   2012/2/23 - mobo
 *
 *  \note   N/A
 */

#ifndef _PICBUF_H264DEC_H_
#define _PICBUF_H264DEC_H_

#include "H264Dec_VLD.h"
#include "H264Dec_Tools.h"

#define H264DEC_MAX_PICBUF_PIC_NUM     32  //!< don't change it

typedef struct ST_H264DEC_PICINFO {
    // Static info when dpb init_by_idx
    UINT32  u32UpperId;     //!< rec/col-buffer id/idx defined by upper layer
    UINT32  u32Stride;
    uintptr_t  u32RecYAddr;
    uintptr_t  u32RecUVAddr;
    uintptr_t  u32ColInfoAddr;
	uintptr_t  u32RecY2ndAddr;

	//add by hk: for debug
    UINT32  u32RecDmaSum;
    UINT32  u32ColDmaSum;
    UINT32  u32RecDmaXorSum;
    UINT32  u32ColDmaXorSum;

    // Dynamic info after Slice-level(SW) decoding
    UINT32  u32DecPicNum;   //!< frame num
    INT32   s32Poc;         //!< picture order count

    // Dynamic info after picture output
    BOOL    bIsOutput;      //!< set to TRUE if the frame(field) was decoded and selected for display
    BOOL    bIsDisplayed;   //!< set to TRUE if frame had been displayed(notified by display module through DecodeFrame API)
    int     s32Class;       //!< used to diffentiate the same POC of different time
    UINT32  u32NumOfErrMbs;
    UINT32  u32CheckSum[2];                         //checksum value.  [2] for top and bottom first decode put in [0], second put in [1]
    UINT32  u32PicType;
    UINT32  u32FieldType;

    //add for dynamically resolution change
	BOOL    bValid;
    uintptr_t  ulBufCfgPtr;
    UINT32  u32IndexOfVa;
    UINT32  u32BufSize;
    UINT32  u32PicSize;
    UINT32  u32ColSize;
    struct ST_H264DEC_PICINFO   *next;
}ST_H264DEC_PICINFO,*PST_H264DEC_PICINFO;

typedef struct _ST_H264DEC_PICBUF {

	/* add by hk */
	INT8     release_buf_list[32];
	INT8     release_buf_count;

    // picinfo list
    UINT32  u32PicBufCnt;
    UINT32  u32OutputCand;      //!< # of output candidates

    UINT32  u32Stride;
    UINT32  u32BufSize;
    UINT32  u32BufSizeY;
    UINT32  u32BufSizeUV;
    UINT32  u32BufSizeCol;
    UINT32  u32RefMask;       //!< ref-frame mask
    UINT32  u32CurClass;     //!< class#
    ST_H264DEC_PICINFO  *pstPicInfo;
    ST_H264DEC_PICINFO  *pstPicFreeList;
    ST_H264DEC_PICINFO  *pstPicOutputList;

} ST_H264DEC_PICBUF, *PST_H264DEC_PICBUF;

typedef enum _H264DEC_PICBUF_RET {
    H264DEC_PICBUF_OK = 0,
    H264DEC_PICBUF_ERROR,
    H264DEC_PICBUF_NO_OUTPUT_PIC,
    H264DEC_PICBUF_IDX_OUT_OF_RANGE,
    H264DEC_PICBUF_IDR_ERR,
    H264DEC_PICBUF_SLICE_ERR,
} H264DEC_PICBUF_RET;

void dump_picinfo(ST_H264DEC_PICBUF *obj, int idx);

/*!
 *  Initialize PICBUF object
 */
void h264dec_picbuf_init(PST_H264DEC_PICBUF obj);

// count how many buffer in use.
int h264dec_picbuf_pic_cnt(ST_H264DEC_PICBUF *obj);

H264DEC_PICBUF_RET h264dec_picbuf_picinfo_alloc(ST_H264DEC_PICBUF *obj, int *idx,va_buf_cfg *bufCfg, uintptr_t MbInfoBufAddr, uintptr_t FrameBufAddr2nd);
/*!
 *  Initialize a PICBUF picinfo entry
 *  \param[in]  obj         PICBUF object handle
 *  \param[in]  idx         pic_info index
 *  \param[in]  upper_id    rec/col-buffer id or idx defined by upper-layer
 *  \param[in]  yaddr       Y start address
 *  \param[in]  uvaddr      UV start address
 *  \param[in]  coladdr     Co-located information start address (EN_COL_BUF_ALLOC_DYN = 0),
 *                          and no-use (EN_COL_BUF_ALLOC_DYN = 1)
 */
H264DEC_PICBUF_RET h264dec_picbuf_picinfo_init(ST_H264DEC_PICBUF *obj, int idx, uintptr_t yaddr, uintptr_t uvaddr, uintptr_t coladdr);

/*!
 *  Set the slice-level(SW) information
 *  \param[in]  obj         PICBUF object handle
 *  \param[in]  idx         pic_info index
 *  \param[in]  dec_pic_num decoded picture number provided by DecodeSlice
 *  \param[in]  poc         picture order count
 *  \param[in]  pic_type    Picture Type PBI = {0, 1, 2}
 *  \param[in]  is_idr          is IDR pic?
 *  \param[in]  bLastHasMmco5   last has mmco=5?
 */
H264DEC_PICBUF_RET h264dec_picbuf_picinfo_set_slice(/*DRV_H264DEC_CTX *ctx,*/ ST_H264DEC_PICBUF *obj, int idx, UINT32 dec_pic_num, INT32 poc, UINT32 pic_type, BOOL bLastHasMmco5, BOOL isIdr );

/*!
 *  Output a picture from PICBUF
 *  \param[in]  obj             PICBUF object handle
 *  \param[in]  seekAfterIdr    after seek I frame, if that frame is not idr, then drop B picture before I frame output.
 *  \param[in]  bForceOutput    really flush all picture in picture buffer, it will occure in stream end or ES buffer reset.
 *  \param[out] out_upper_id    Output picture id or idx which is defined by upper-layer
 */
H264DEC_PICBUF_RET h264dec_picbuf_output_pic(/*DRV_H264DEC_CTX *ctx,*/ ST_H264DEC_PICBUF *obj, UINT32 *picId, BOOL seekAfterIdr, UINT32 u32ForceOutput);

/*!
 *  Flush all decoded pics
 *  \param[in]  obj             PICBUF object handle
 *  \param[in]  clear_disp_que  TRUE: Clear the display queue also
 */
H264DEC_PICBUF_RET h264dec_picbuf_flush(/*DRV_H264DEC_CTX *ctx,*/ ST_H264DEC_PICBUF *obj, BOOL clear_disp_que);

/*!
 *  Get an empty pictuer buffer
 *  \param[in]  obj             PICBUF object handle
 *
 *  \return     picinfo idx of empty buffer, -1 if not available
 */

int h264dec_picbuf_get_empty_pic(ST_H264DEC_PICBUF *obj);

/*!
 *  Find the picinfo idx by matching dec_pic_num
 *  \param[in]  obj             PICBUF object handle
 *  \param[in]  dec_pic_num     Decoded picture number
 *  \return     picinfo idx, -1 if not available
 */
int h264dec_picbuf_find_idx_by_dec_pic_num(ST_H264DEC_PICBUF *obj, UINT32 u32DecPicNum);

/*!
 *  Setup the refererence mask
 *  \param[in]  obj             PICBUF object handle
 *  \param[in]  mask            {mask[31], mask[30], ..., mask[1], mask[0]}
 */
void h264dec_picbuf_set_refmask(ST_H264DEC_PICBUF *obj, UINT32 mask);

/*!
 *  Set picture as displayed
 *  \param[in]  obj             PICBUF object handle
 */
void h264dec_picbuf_picinfo_set_displayed(ST_H264DEC_PICBUF *obj, int idx);

/*!
 *  Add picture buf node into output(display) list
 *  \param[in]  obj             PICBUF object handle
 */
int h264dec_picbuf_picinfo_register_node(ST_H264DEC_PICBUF *obj, int idx);


//void pic_testCnt(h264dec_picbuf_s *ctx);
#endif  // _H264DEC_PICBUF_H_

