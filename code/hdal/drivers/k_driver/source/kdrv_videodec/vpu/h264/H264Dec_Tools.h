/*!
 *  \file       que_common.h
 *              A common queue tool, its entry-size is UINT32
 *
 *  \author     mobo <Mobo_Chuang@novatek.com.tw>
 *  \author     mobo <iroi_luo@novatek.com.tw>
 *  \date       2012/02/29 - mobo - added
 *  \date       2013/01/22 - iroi - modify
 *
 */

#ifndef _H264Dec_TOOLS_H_
#define _H264Dec_TOOLS_H_

#include "../../imvq.h"

///////////////////////que common//////////////////////////////
typedef enum que_ret {
    QUE_OK = 0,
    QUE_FAIL = 1
} que_ret;

typedef struct que_s {
    INT32   size;
    INT32   head;
    INT32   tail;
    UINT32 *pdata; //index of queue.
} que_s;

//  Initialize the queue with pre-allocated memory
//  \param[in]  obj         Queue object
//  \param[in]  size        data_array's size, the active storage size is size-1
//  \param[in]  data_array  pre-allocated memory

que_ret que_init(que_s *obj, INT32 size, UINT32 *data_array);

que_ret que_get(que_s *obj, UINT32 *v);
que_ret que_peek(que_s *obj, UINT32 *v);
que_ret que_put(que_s *obj, UINT32 v);
que_ret que_clear(que_s *obj);

BOOL que_is_empty(que_s *obj);
BOOL que_is_full(que_s *obj);

int que_count(que_s *obj);
////////////////////////////////////////////////////////////////

void _H264DecSwap(char *a, char *b, unsigned width);
void _H264DecShortsort(char *lo, char *hi, unsigned width, int (*comp)(const void *, const void *));
void _H264DecQsort(void *base, unsigned num, unsigned width, int (*comp)(const void *, const void *));
///////////////////////h264 memsh common////////////////////////
typedef enum memsh_align_t
{
    MEM_ALIGN_1  = 0,
    MEM_ALIGN_2  = 1,
    MEM_ALIGN_4  = 2,
    MEM_ALIGN_8  = 3,
    MEM_ALIGN_16 = 4,
    MEM_ALIGN_32 = 5,
    MEM_ALIGN_64 = 6,
    MEM_ALIGN_128 = 7,
} H246_MEM_ALIGNMENT;

typedef struct _ST_H264_MEM_POOL
{
    uintptr_t      	ulBufBase;		    //!< Virtual Base Address
    uintptr_t      	ulBufUsedPtr;
    unsigned int 	ulBufSize;		    //!< Size
    unsigned long  	ulPhy2VirOffset ;
    unsigned int 	ulUsedSize;
    unsigned int  	ulErrorCheck;
} ST_H264_MEM_POOL, *PST_H264_MEM_POOL;

uintptr_t __h264_VirToPhy(PST_H264_MEM_POOL memPool, uintptr_t virAddr);

uintptr_t __h264_PhyToVir(PST_H264_MEM_POOL memPool, uintptr_t phyAddr);

uintptr_t __h264_MemAlloc(PST_H264_MEM_POOL memPool, UINT32 allocSize, H246_MEM_ALIGNMENT align);

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// H.264 Decoder configuration
#if H26XD_SDK_MODE
#define H264_BSDMA_CMD_LEN     64
#else
#define H264_BSDMA_CMD_LEN     512  //in baseline profile, it may not enough for ASO, be careful.
#endif

#define H264_BSDMA_DATA_MAX    0x40000 //!< don't change it.
#define H264_BSDMA_CMD_SIZE    (1 + 2*H264_BSDMA_CMD_LEN)  //!< BSDMA of n entry = (1 + 2*n) words

#define EN_SW_PATCH_0717 1

#define H264DEC_MAX_CMDBUF_QUEUE_DEPTH          (1) //(4)  //hk : cannot queue > 1 pictures.

#define H264DEC_MAX_CMDBUF_QUEUE_DEPTH_PLUS_1   (H264DEC_MAX_CMDBUF_QUEUE_DEPTH+1)

#define TYPE_FRAME      0
#define TYPE_TOP_FIELD  1
#define TYPE_BTM_FIELD  2

#define MAX_PICINFO_NUM         32      //!< don't changed it
#define MAX_PICBUF_KEPT_NUM     16      //!< MAX_PICBUF_KEPT should not equal or larger than MAX_PICINFO_NUM
#define NO_PREV_CMDBUF_IDX      -1

#define H264_MAX_FRM_BUF           17

#ifdef CONFIG_NVT_IVOT_PLAT_NA51103
#define H264_MAX_MBCNT             (5120*4096/16/16)
#elif CONFIG_NVT_IVOT_PLAT_NA51102
#define H264_MAX_MBCNT             (8192*8192/16/16)
#else
#error "Please specify the plaform!"
#endif

#define H264_RPIC_REF_LIST_SIZE    27
#define H264_Min(a, b) (((a) > (b)) ? (b) : (a))

#define USE_LOCA_QSORT
#ifdef USE_LOCA_QSORT
#define H264CUTOFF 8
#else
#define _H264DecQsort qsort
#endif

#define H264COLORS      "\33[1;32;40m"
#define H264COLORE      "\33[0m"
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif  // _H264Dec_TOOLS_H_


