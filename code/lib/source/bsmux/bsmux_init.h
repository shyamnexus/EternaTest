/**
    @brief 		Header file of bsmux library.

    @file 		bsmux_init.h

    @ingroup 	mBsMux

    @note		Nothing.

    Copyright Novatek Microelectronics Corp. 2019.  All rights reserved.
*/

#ifndef BSMUX_INT_H
#define BSMUX_INT_H

/*-----------------------------------------------------------------------------*/
/* Include Header Files                                                        */
/*-----------------------------------------------------------------------------*/
#include <string.h>
#include <kwrap/error_no.h>
#include <kwrap/perf.h>
#include <kwrap/type.h>
#include <kwrap/cmdsys.h>
#include <kwrap/sxcmd.h>
#include <kwrap/stdio.h>
#include <kwrap/util.h>
#include <kwrap/verinfo.h>
#include <hd_gfx.h>
#include "FileSysTsk.h"
#include <time.h>
#include <comm/hwclock.h>
#include "avfile/movieinterface_def.h"
#include "avfile/MOVLib.h"
#include "avfile/MediaWriteLib.h"
#include "avfile/AVFile_MakerMov.h"
#include "avfile/AVFile_MakerTS.h"
#include "Utility/avl.h"
#include "hd_bsmux_lib.h"

///////////////////////////////////////////////////////////////////////////////
#define __MODULE__          BSMUX
#define __DBGLVL__          2
#include "kwrap/debug.h"
extern unsigned int BSMUX_debug_level;
///////////////////////////////////////////////////////////////////////////////

// BACKWARD COMPATIBLE
#define ISF_UNIT_BSMUX                   0
#define ISF_MAX_BSMUX                    1
#define ISF_IN_BASE                      128
#define ISF_OUT_BASE                     0
#define ISF_BSMUX_IN_NUM                 32
#define ISF_BSMUX_OUT_NUM                16

#define MP_AUDENC_AAC_RAW_BLOCK          1024

#define NMEDIAREC_ALIGN_CEIL             1 //round up
#define NMEDIAREC_ALIGN_ROUND            2 //round off

#define BSMUXER_CALCSEC_UNKNOWN_FSSIZE   (UINT64)0xFFFFFFFF

#define BSMUXER_BSQ_ROLLBACK_SECNEW      300 //300 GOPs = 30fps (120sec) = 120fps (30sec)
#define BSMUXER_BSQ_MAX_SECENTRY         120 //60 fps +47 aac

#define BSMUXER_FEVENT_NORMAL            HD_BSMUX_FEVENT_NORMAL
#define BSMUXER_FEVENT_EMR               HD_BSMUX_FEVENT_EMR
#define BSMUXER_FEVENT_BSINCARD          HD_BSMUX_FEVENT_BSINCARD

#define BSMUXER_FOP_NONE                 HD_BSMUX_FOP_NONE
#define BSMUXER_FOP_CREATE               HD_BSMUX_FOP_CREATE
#define BSMUXER_FOP_CLOSE                HD_BSMUX_FOP_CLOSE
#define BSMUXER_FOP_CONT_WRITE           HD_BSMUX_FOP_CONT_WRITE
#define BSMUXER_FOP_SEEK_WRITE           HD_BSMUX_FOP_SEEK_WRITE
#define BSMUXER_FOP_FLUSH                HD_BSMUX_FOP_FLUSH
#define BSMUXER_FOP_DISCARD              HD_BSMUX_FOP_DISCARD
#define BSMUXER_FOP_READ                 HD_BSMUX_FOP_READ_WRITE
#define BSMUXER_FOP_SNAPSHOT             HD_BSMUX_FOP_SNAPSHOT

#define BSMUX_REC_AUDIOINFO              HD_BSMUX_AUDIOINFO
#define NMEDIA_REC_EMR_INFO              HD_BSMUX_TRIG_EMR
#define BSMUX_CALC_SEC                   HD_BSMUX_CALC_SEC
#define BSMUX_CALC_SEC_SETTING           HD_BSMUX_CALC_SEC_SETTING
#define BSMUX_REC_EXTINFO                HD_BSMUX_EXTINFO

#define BSMUX_OUT_DATA                   HD_BSMUX_OUT_DATA
#define BSMUXER_OUT_BUF                  HD_BSMUX_OUT_DATA

#define BSMUXER_RESULT_NORMAL            HD_BSMUX_ERRCODE_NONE
#define BSMUXER_RESULT_SLOWMEDIA         HD_BSMUX_ERRCODE_SLOWMEDIA
#define BSMUXER_RESULT_LOOPREC_FULL      HD_BSMUX_ERRCODE_LOOPREC_FULL
#define BSMUXER_RESULT_OVERTIME          HD_BSMUX_ERRCODE_OVERTIME
#define BSMUXER_RESULT_MAXSIZE           HD_BSMUX_ERRCODE_MAXSIZE
#define BSMUXER_RESULT_VANOTSYNC         HD_BSMUX_ERRCODE_VANOTSYNC
#define BSMUXER_RESULT_GOPMISMATCH       HD_BSMUX_ERRCODE_GOPMISMATCH
#define BSMUXER_RESULT_PROCDATAFAIL      HD_BSMUX_ERRCODE_PROCDATAFAIL
#define BSMUXER_RESULT                   HD_BSMUX_ERRCODE

#define BSMUXER_CBEVENT_PUTBSDONE        HD_BSMUX_CB_EVENT_PUTBSDONE
#define BSMUXER_CBEVENT_FOUTREADY        HD_BSMUX_CB_EVENT_FOUTREADY
#define BSMUXER_CBEVENT_CUT_COMPLETE     HD_BSMUX_CB_EVENT_CUT_COMPLETE
#define BSMUXER_CBEVENT_CLOSE_RESULT     HD_BSMUX_CB_EVENT_CLOSE_RESULT
#define BSMUXER_CBEVENT_COPYBSBUF        HD_BSMUX_CB_EVENT_COPYBSBUF
#define BSMUXER_CBEVENT_CUT_BEGIN        HD_BSMUX_CB_EVENT_CUT_BEGIN
#define BSMUXER_CBEVENT                  HD_BSMUX_CB_EVENT

#define BSMUX_EVENT_CB                   HD_BSMUX_CALLBACK
#define BSMUXER_CBINFO                   HD_BSMUX_CBINFO
#define BSMUXER_OBJ                      HD_BSMUX_REG_CALLBACK

/**
    COMPATIBLE.
    @note only for internal use
*/
typedef unsigned int BSM_IO;
typedef uintptr_t    BSM_AD;
typedef size_t       BSM_SZ;

typedef struct _BSM_MEM {
	BSM_AD pa;
	BSM_AD va;
} BSM_MEM;

/**
    file buf.
    @note for push out to fileout
*/
typedef struct _BSMUXER_FILE_BUF {
	BSM_IO 				pathID;					   ///< keep path id
	UINT32              event;                     ///< BSMux event
	UINT32              fileop;                    ///< bitwise: open/close/conti_write or seek_write/flush/none(event only)/discard
	BSM_AD              addr;                      ///< write data address
	UINT64              size;                      ///< write data size
	UINT64              pos;                       ///< only valid if seek_write
	UINT32              type;                      ///< file type, MP4, TS, JPG, THM
} BSMUXER_FILE_BUF;


// INCLUDE PROTECTED
#include "bsmux_mdat.h"
#include "bsmux_ts.h"
#include "bsmux_cb.h"
#include "bsmux_ctrl.h"
#include "bsmux_id.h"
#include "bsmux_tsk.h"
#include "bsmux_util.h"

/*-----------------------------------------------------------------------------*/
/* Macro Function Definitions                                                  */
/*-----------------------------------------------------------------------------*/


/*-----------------------------------------------------------------------------*/
/* Macro Constant Definitions                                                  */
/*-----------------------------------------------------------------------------*/
#define BSMUX_MAX_PATH_NUM      16 //NMR_MAX_TOTALMDAT
#define BSMUX_MAX_CTRL_NUM      16 //NMR_MAX_TOTALMDAT
#define BSMUX_MAX_BSIN_NUM      64
#define BSMUX_MAX_META_NUM      10

#define BSMUX_BSQ_ROLLBACK_SEC  68 //68 GOPs = 30fps (30sec) = 120fps (7sec), from NMEDIAREC_BSQ_ROLLBACK_SECORI

//mdat header info
#define BSMUX_HDRMEM_MAX        4  //4 for the same path files going to card //NMR_MDAT_HDRMEM_MAX
#define BSMUX_HDRNUM_MAX        2

#define BSMUX_HDRMEM_NONE       0 //not hdr
#define BSMUX_HDRMEM_FRONT      1 //NMR_MDAT_HDRMEM_FRONT
#define BSMUX_HDRMEM_TEMP       2 //temp for copy front header to update filepos=0 //NMR_MDAT_HDRMEM_TEMP
#define BSMUX_HDRMEM_BACK       3 //NMR_MDAT_HDRMEM_BACK

#define BSMUX_BS2CARD_2M        0x200000 //NMEDIAREC_BS2CARD_STEP2M
#define BSMUX_BS2CARD_1M        0x100000 //NMEDIAREC_BS2CARD_STEP1M
#define BSMUX_BS2CARD_500K      0x80000 //NMEDIAREC_BS2CARD_STEP0_5M
#define BSMUX_BS2CARD_NORMAL    0 //not hdr
#define BSMUX_BS2CARD_FRONT     1 //NMR_MDAT_HDRMEM_FRONT
#define BSMUX_BS2CARD_TEMP      2 //temp for copy front header to update filepos=0 //NMR_MDAT_HDRMEM_TEMP
#define BSMUX_BS2CARD_BACK      3 //NMR_MDAT_HDRMEM_BACK
#define BSMUX_BS2CARD_LAST      4
#define BSMUX_BS2CARD_MOOV      5
#define BSMUX_BS2CARD_THUMB     6
#define BSMUX_BS2STRG_WRITE     7
#define BSMUX_BS2STRG_SYNC      8
#define BSMUX_BS2STRG_FLUSH     9
#define BSMUX_BS2STRG_READ      10

#define BSMUX_RESERVED_FILESIZE 0x10000 //NMR_RESERVED_FILESIZE,
#define BSMUX_HEADER_MIN        BSMUX_RESERVED_FILESIZE //NMRMDAT_HEADER_MIN
#define BSMUX_IDX1SIZE_NOMRAL   0x200000 // 2M for 30fps 60 min //NMRMDAT_IDX1SIZE_NOMRAL
#define BSMUX_GPS_MIN           0x4000  // 16 K //NMRMDAT_GPS_MIN
#define BSMUX_MAX_NIDX_BLK      0x2000//8K //NMEDIAREC_MAX_NIDX_BLK
#define BSMUX_VIDEO_DESC_SIZE   0x200   // 512 bytes //NMTMDAT_VIDEO_DESC_SIZE

#define BSMUX_ROLLBACKSEC_MIN   3
#define BSMUX_ROLLBACKSEC_MAX   30
#define BSMUX_KEEPSEC_MIN       5
#define BSMUX_KEEPSEC_MAX       60
#define BSMUX_RESVSEC_MIN       7
#define BSMUX_RESVSEC_MAX       30
#define BSMUX_PLAYVFR_15        15
#define BSMUX_PLAYVFR_30        30
#define BSMUX_NIDX_RESVSEC      4
#define BSMUX_LOCKNUMMIN        2 //one for our own, one for output
#define BSMUX_NORMALNUMMIN      3 //one for our own, one for padding reserved, one for output
#define BSMUX_DUR_US_MAX        6000000 //us (2s)
#define BSMUX_DUR_MS_MAX        6000    //ms (2s)

#define BSMUX_CTRL_IDX          0xF000
#define BSMUX_INVALID_IDX       0xFFFF

#define BSMUX_STEP_EMRREC      0x0001
#define BSMUX_STEP_EMRLOOP     0x0002

//TEST
#define BSMUX_TEST_GPS_ON       0
#define BSMUX_TEST_USER_ON      0
#define BSMUX_TEST_TS_FORMAT    0
#define BSMUX_TEST_CHK_FIRSTI   1
#define BSMUX_TEST_MOOV_RC      1

//DEBUG
#define BSMUX_DEBUG_CBINFO      0
#define BSMUX_DEBUG_MEMBUF      0
#define BSMUX_DEBUG_BSQ         0
#define BSMUX_DEBUG_TSKINFO     0
#define BSMUX_DEBUG_ACTION      0
#define BSMUX_DEBUG_HEADER      0
#define BSMUX_DEBUG_BITRATE     0
#define BSMUX_DEBUG_MDAT        0
#define BSMUX_DEBUG_TS          0
#define BSMUX_DEBUG_MSG         0

typedef enum {
	BSMUXER_PARAM_MIN,
	//Basic Settings: THUMB
	BSMUXER_PARAM_THUMB_ON,     //thumb on/off <obsolete>
	BSMUXER_PARAM_THUMB_ADDR,   //thumb addr
	BSMUXER_PARAM_THUMB_SIZE,   //thumb size
	//Basic Settings: VID
	BSMUXER_PARAM_VID_WIDTH,    //vid wid
	BSMUXER_PARAM_VID_HEIGHT,   //vid height
	BSMUXER_PARAM_VID_VFR,      //vid frame rate
	BSMUXER_PARAM_VID_TBR,      //vid target bitrate
	BSMUXER_PARAM_VID_CODECTYPE,    //vid codectype
	BSMUXER_PARAM_VID_DESCADDR,     //vid desc addr
	BSMUXER_PARAM_VID_DESCSIZE,     //vid desc size
	BSMUXER_PARAM_VID_DAR,     //vid display aspect ratio, MP_VDOENC_DAR_DEFAULT
	BSMUXER_PARAM_VID_GOP,
	BSMUXER_PARAM_VID_NALUNUM,
	BSMUXER_PARAM_VID_VPSSIZE,
	BSMUXER_PARAM_VID_SPSSIZE,
	BSMUXER_PARAM_VID_PPSSIZE,
	BSMUXER_PARAM_VID_VAR_VFR,
	//Basic Settings: SUB VID
	BSMUXER_PARAM_VID_SUB_WIDTH,    //vid wid
	BSMUXER_PARAM_VID_SUB_HEIGHT,   //vid height
	BSMUXER_PARAM_VID_SUB_VFR,      //vid frame rate
	BSMUXER_PARAM_VID_SUB_TBR,      //vid target bitrate
	BSMUXER_PARAM_VID_SUB_CODECTYPE,    //vid codectype
	BSMUXER_PARAM_VID_SUB_DESCADDR,     //vid desc addr
	BSMUXER_PARAM_VID_SUB_DESCSIZE,     //vid desc size
	BSMUXER_PARAM_VID_SUB_DAR,     //vid display aspect ratio, MP_VDOENC_DAR_DEFAULT
	BSMUXER_PARAM_VID_SUB_GOP,
	BSMUXER_PARAM_VID_SUB_NALUNUM,
	BSMUXER_PARAM_VID_SUB_VPSSIZE,
	BSMUXER_PARAM_VID_SUB_SPSSIZE,
	BSMUXER_PARAM_VID_SUB_PPSSIZE,
	//Basic Settings: AUD
	BSMUXER_PARAM_AUD_CODECTYPE,//MOVAUDENC_AAC or others
	BSMUXER_PARAM_AUD_SR,       //audio sample rate
	BSMUXER_PARAM_AUD_CHS,      //audio channels
	BSMUXER_PARAM_AUD_ON,       //audio on/off
	BSMUXER_PARAM_AUD_EN_ADTS,    //audio BS ADTS shift bytes. default =0
	//Basic Settings: FILE
	BSMUXER_PARAM_FILE_EMRON,   //emr on
	BSMUXER_PARAM_FILE_EMRLOOP,   //emrloop on
	BSMUXER_PARAM_FILE_DDR_ID,  //ddr_id
	BSMUXER_PARAM_FILE_SEAMLESSSEC,     //seamlessSec
	BSMUXER_PARAM_FILE_ROLLBACKSEC,     //rollbacksec (default:1)
	BSMUXER_PARAM_FILE_KEEPSEC, //keepsec (only for EMR)
	BSMUXER_PARAM_FILE_FILETYPE,//filetype, MEDIA_FILEFORMAT_MP4 or others
	BSMUXER_PARAM_FILE_RECFORMAT,//recformat, MEDIAREC_AUD_VID_BOTH or others
    BSMUXER_PARAM_FILE_PLAYFRAMERATE, //default as 30
	BSMUXER_PARAM_FILE_BUFRESSEC,    //buffer reserved sec, default 5 (5~30)
	BSMUXER_PARAM_FILE_OVERLAP_ON,    //overlap on/off, default OFF
	BSMUXER_PARAM_FILE_PAUSE_ON,    // for emr pause
	BSMUXER_PARAM_FILE_PAUSE_ID,    // for emr pause
	BSMUXER_PARAM_FILE_PAUSE_CNT,    // for emr pause
	BSMUXER_PARAM_FILE_SEAMLESSSEC_MS,
	BSMUXER_PARAM_FILE_ROLLBACKSEC_MS,
	BSMUXER_PARAM_FILE_KEEPSEC_MS,
	BSMUXER_PARAM_FILE_BUFRESSEC_MS,
	//Basic Settings: HDR
	BSMUXER_PARAM_HDR_FRAMEBUF_ADDR,
	BSMUXER_PARAM_HDR_FRAMEBUF_SIZE,
	BSMUXER_PARAM_HDR_FRONTHDR_ADDR,
	BSMUXER_PARAM_HDR_FRONTHDR_SIZE,
	BSMUXER_PARAM_HDR_BACKHDR_ADDR,
	BSMUXER_PARAM_HDR_BACKHDR_SIZE,
	BSMUXER_PARAM_HDR_TEMPHDR_ADDR,
	BSMUXER_PARAM_HDR_TEMPHDR_SIEZ,
	BSMUXER_PARAM_HDR_TEMPHDR_1_ADDR,
	BSMUXER_PARAM_HDR_TEMPHDR_1_SIEZ,
	BSMUXER_PARAM_HDR_NIDX_ADDR,  //nidxaddr
	BSMUXER_PARAM_HDR_GSP_ADDR,   //gpsaddr
	//Basic Settings: GPS
	BSMUXER_PARAM_GPS_ON,     //gps on/off
	BSMUXER_PARAM_GPS_RATE,
	BSMUXER_PARAM_GPS_QUEUE,
	//Basic Settings: MEM
	BSMUXER_PARAM_MEM_ADDR,         // bsmux phy_addr
	BSMUXER_PARAM_MEM_VIRT,         // bsmux virt_addr
	BSMUXER_PARAM_MEM_SIZE,         // bamux buf_size
	BSMUXER_PARAM_MEM_END,
	BSMUXER_PARAM_MEM_WRTBLK,
	BSMUXER_PARAM_PRECALC_BUFFER,    //buffer to calculat min bsmuxer second
	//Basic Settings: NIDX
	BSMUXER_PARAM_NIDX_EN,
	BSMUXER_PARAM_NIDX_VFN,
	BSMUXER_PARAM_NIDX_SUB_VFN,
	BSMUXER_PARAM_NIDX_AFN,
	//Utility: USERDATA
	BSMUXER_PARAM_USERDATA_ON,
	BSMUXER_PARAM_USERDATA_ADDR,
	BSMUXER_PARAM_USERDATA_SIZE,
	//Utility: CUSTDATA
	BSMUXER_PARAM_CUSTDATA_ADDR,
	BSMUXER_PARAM_CUSTDATA_SIZE,
	BSMUXER_PARAM_CUSTDATA_TAG,
	//Utility: WRINFO
	BSMUXER_PARAM_WRINFO_FLUSH_FREQ,
	BSMUXER_PARAM_WRINFO_WRBLK_CNT,
	BSMUXER_PARAM_WRINFO_WRBLK_SIZE,
	BSMUXER_PARAM_WRINFO_CLOSE_FLAG,
	BSMUXER_PARAM_WRINFO_WRBLK_LOCKSEC,
	BSMUXER_PARAM_WRINFO_WRBLK_TIMEOUT,
	//Utility: EXTINFO
	BSMUXER_PARAM_EXTINFO_UNIT,
	BSMUXER_PARAM_EXTINFO_MAX_NUM,
	BSMUXER_PARAM_EXTINFO_ENABLE,
	//Utility: MOOVINFO
	BSMUXER_PARAM_FRONT_MOOV,
	BSMUXER_PARAM_MOOV_ADDR,
	BSMUXER_PARAM_MOOV_SIZE,
	BSMUXER_PARAM_MOOV_FREQ,
	BSMUXER_PARAM_MOOV_TUNE,
	//Utility: DROPINFO
	BSMUXER_PARAM_EN_DROP,
	BSMUXER_PARAM_VID_DROP,
	BSMUXER_PARAM_AUD_DROP,
	BSMUXER_PARAM_SUB_DROP,
	BSMUXER_PARAM_DAT_DROP,
	BSMUXER_PARAM_VID_SET,
	BSMUXER_PARAM_AUD_SET,
	BSMUXER_PARAM_SUB_SET,
	BSMUXER_PARAM_DAT_SET,
	BSMUXER_PARAM_FULL_SET,
	//Utility: UTC
	BSMUXER_PARAM_UTC_SIGN,  //1: negative | 0: positive
	BSMUXER_PARAM_UTC_ZONE,
	BSMUXER_PARAM_UTC_TIME,
	//Utility: ALIGN
	BSMUXER_PARAM_MUXALIGN,
	BSMUXER_PARAM_MUXMETHOD,
	//Utility: META
	BSMUXER_PARAM_META_ON,     //gps on/off
	BSMUXER_PARAM_META_NUM,
	BSMUXER_PARAM_META_DATA,
	//Others
	BSMUXER_PARAM_EN_POINTTHM,
	BSMUXER_PARAM_MAX_SIZE,
	BSMUXER_PARAM_EMR_NEXTSEC,   //second after emr finish to start next emr, default 3(3~10)
	BSMUXER_PARAM_FREASIZE,
	BSMUXER_PARAM_FREAENDSIZE,
	BSMUXER_PARAM_EN_FREABOX,
	BSMUXER_PARAM_EN_FASTPUT,
	BSMUXER_PARAM_DUR_US_MAX,
	BSMUXER_PARAM_BOXTAG_SIZE,
	BSMUXER_PARAM_PTS_RESET,
	//Utility: STRGBUF
	BSMUXER_PARAM_EN_STRGBUF,
	BSMUXER_PARAM_STRGBUF_ACT,
	BSMUXER_PARAM_STRGBUF_CUT,
	BSMUXER_PARAM_STRGBUF_HDR,
	BSMUXER_PARAM_STRGBUF_VID,
	BSMUXER_PARAM_STRGBUF_VID_NUM,
	BSMUXER_PARAM_STRGBUF_AUD_NUM,
	BSMUXER_PARAM_STRGBUF_CUR_POS,
	BSMUXER_PARAM_STRGBUF_MAX_NUM,
	BSMUXER_PARAM_STRGBUF_ALLOC_SIZE,
	BSMUXER_PARAM_STRGBUF_TOTAL_SIZE,
	BSMUXER_PARAM_STRGBUF_PUT_POS,
	BSMUXER_PARAM_STRGBUF_GET_POS,
	BSMUXER_PARAM_MAX,
	BSMUXER_EVENT_MIN,

	BSMUXER_EVENT_MAX,
	ENUM_DUMMY4WORD(BSMUXER_PARAM)
} BSMUXER_PARAM;

typedef enum _BSMUX_CTRL_INFO {
	BSMUX_CTRL_MIN,
	//bsq addr
	BSMUX_CTRL_NOWADDR,       //nowaddr: now address in copy buffer
	BSMUX_CTRL_LAST2CARD,     //last2card: last address in copy buffer to card
	BSMUX_CTRL_REAL2CARD,     //real2card: file size has in card => change to addr
	BSMUX_CTRL_END2CARD,      //fileend_addr
	//size related
	BSMUX_CTRL_TOTAL2CARD,    //total2card: total file size has been add to FS Queue
	//wrtie block
	BSMUX_CTRL_WRITEBLOCK,    //blksize: one blksize
	BSMUX_CTRL_WRITEOFFSET,   //blkoffset: cur offset
	BSMUX_CTRL_USEDBLOCK,     //blkusednum: used blk num
	//buf range
	BSMUX_CTRL_BUFADDR,       //buf addr (va)
	BSMUX_CTRL_BUFSIZE,       //buf size
	BSMUX_CTRL_BUFEND,
	BSMUX_CTRL_USEABLE,
	BSMUX_CTRL_FREESIZE,
	BSMUX_CTRL_USEDSIZE,

	BSMUX_CTRL_MAX,
	ENUM_DUMMY4WORD(BSMUX_CTRL_INFO)
} BSMUX_CTRL_INFO;

typedef enum _BSMUX_FILE_INFO {
	BSMUX_FILEINFO_MIN,

	BSMUX_FILEINFO_TOTALVF,

	BSMUX_FILEINFO_TOTALAF,
	BSMUX_FILEINFO_NOWSEC,
	BSMUX_FILEINFO_ADDVF,
	BSMUX_FILEINFO_COPYPOS,

	BSMUX_FILEINFO_TOTALGPS,
	BSMUX_FILEINFO_TOTAL,
	BSMUX_FILEINFO_FRONTPUT,
	BSMUX_FILEINFO_EXTNUM,
	BSMUX_FILEINFO_EXTSEC,

	BSMUX_FILEINFO_BLKPUT,
	BSMUX_FILEINFO_SUBVF,
	BSMUX_FILEINFO_THUMBPUT,
	BSMUX_FILEINFO_EXTNSEC,
	BSMUX_FILEINFO_TOTALVF_DROP,

	BSMUX_FILEINFO_TOTALAF_DROP,
	BSMUX_FILEINFO_SUBVF_DROP,
	BSMUX_FILEINFO_CUTBLK,

	BSMUX_FILEINFO_PUT_VF,
	BSMUX_FILEINFO_PUT_AF,
	BSMUX_FILEINFO_PUT_SUBVF,

	BSMUX_FILEINFO_ROLLSEC,
	BSMUX_FILEINFO_VAR_RATE,

	BSMUX_FILEINFO_TOTALMETA,
	BSMUX_FILEINFO_TOTALMETA2,

	BSMUX_FILEINFO_MAX,
	ENUM_DUMMY4WORD(BSMUX_FILE_INFO)
} BSMUX_FILE_INFO;

/*-----------------------------------------------------------------------------*/
/* Type Definitions                                                            */
/*-----------------------------------------------------------------------------*/

//            [WAITING HIT] -- hit --> [HIT TO RUN]
//                ^                        |
//                |                        v
//  [IDLE]  -- resume                --> [RUN]
//          <-- [PUTALL] <-- suspend --
//

/**
    type: bsmux tsk status.
*/
typedef enum _BSMUX_STATUS {
	BSMUX_STATUS_MIN = 0,
	BSMUX_STATUS_IDLE,
	BSMUX_STATUS_RUN,
	//>> BSMUX_ACTION_MAKE_HEADER
	//>> BSMUX_ACTION_SAVE_ENTRY
	//>> BSMUX_ACTION_CUTFILE
	BSMUX_STATUS_SUSPEND,
	//>> BSMUX_ACTION_UPDATE_HEADER
	//>> BSMUX_ACTION_OVERLAP_1SEC
	BSMUX_STATUS_RESUME,
	//>> BSMUX_ACTION_WAITING_HIT
	//>> BSMUX_ACTION_WAITING_RUN
	BSMUX_STATUS_MAX,
	ENUM_DUMMY4WORD(BSMUX_STATUS)
} BSMUX_STATUS;

/**
    type: bsmux tsk action.
*/
typedef enum _BSMUX_ACTION {
	BSMUX_ACTION_NONE			= 0x00000000,
	//common
	BSMUX_ACTION_WAITING_HIT	= 0x00000001,
	BSMUX_ACTION_WAITING_RUN	= 0x00000002,
	BSMUX_ACTION_CUTFILE		= 0x00000004,
	BSMUX_ACTION_OVERLAP_1SEC	= 0x00000008,
	BSMUX_ACTION_WAITING_IDLE	= 0x00000010,
	BSMUX_ACTION_ADDLAST		= 0x00000020,
	BSMUX_ACTION_PUT_FRONT		= 0x00000040,
	BSMUX_ACTION_PUT_LAST		= 0x00000080,
	//mdat
	BSMUX_ACTION_MAKE_HEADER	= 0x00000100,
	BSMUX_ACTION_UPDATE_HEADER	= 0x00000200,
	BSMUX_ACTION_SAVE_ENTRY		= 0x00000400,
	BSMUX_ACTION_PAD_NIDX		= 0x00000800,
	BSMUX_ACTION_MAKE_MOOV      = 0x00001000,
	//ts
	BSMUX_ACTION_MAKE_PES		= 0x00010000,
	BSMUX_ACTION_MAKE_PAT		= 0x00020000,
	BSMUX_ACTION_MAKE_PMT		= 0x00040000,
	BSMUX_ACTION_MAKE_PCR		= 0x00080000,
	//2v1a
	BSMUX_ACTION_REORDER_1SEC	= 0x01000000,
	ENUM_DUMMY4WORD(BSMUX_ACTION)
} BSMUX_ACTION;

/**
    input data type.
    @note modified from NMEDIA_REC_BSQ_TYPE
*/
typedef enum _BSMUX_TYPE {
	BSMUX_TYPE_VIDEO     = 0x0001,
	BSMUX_TYPE_AUDIO     = 0x0002,
	BSMUX_TYPE_THUMB     = 0x0004,
	BSMUX_TYPE_RAWEN     = 0x0008,
	BSMUX_TYPE_GPSIN     = 0x0010,
	BSMUX_TYPE_USERT     = 0x0020,
	BSMUX_TYPE_USRCB     = 0x0040,
	BSMUX_TYPE_NIDXT     = 0x0100,
	BSMUX_TYPE_SUBVD     = 0x0200,
	BSMUX_TYPE_MTAIN     = 0x0400,
	ENUM_DUMMY4WORD(BSMUX_TYPE)
} BSMUX_TYPE;

/**
    input data.
    @note modified from BSMUXER_MDATDATA
    @PUSH IN: VIDEO / AUDIO / THUMB / RAW / GPS / USER
*/
typedef struct _BSMUXER_DATA {
	UINT32 type;                    ///< signature type
	BSM_AD bSMemAddr;               ///< physical address of encoded data
	BSM_SZ bSSize;                  ///< size of encoded data
	UINT32 isKey;                   ///< if video frame type I-frame
	BSM_AD bSVirAddr;               ///< virtual address of encoded data
	UINT32 bufid;                   ///< memory block
	BSM_AD naluaddr;
	BSM_SZ nalusize;
	UINT64 bSTimeStamp;
	BSM_AD naluVirAddr;
	BSM_SZ naluVPSSize;
	BSM_SZ naluSPSSize;
	BSM_SZ naluPPSSize;
	UINT32 naluNum;        //tile
	BSM_AD naluOftMemAddr; //tile
	BSM_AD naluOftVirAddr; //tile
	VOID * user_data;
	UINT32 meta_data_num;
	BSM_AD meta_data_mem_addr;
	BSM_AD meta_data_vir_addr;
} BSMUXER_DATA;

/**
    struct: bsmux tsk ctrl obj.
    @note modified from NMEDIA_REC_MDAT_OBJ, NMEDIA_TS_OBJ
*/
typedef struct _BSMUX_TSK_CTRL_OBJ {
	BSMUX_STATUS Status;
	BSMUX_ACTION Action;
} BSMUX_TSK_CTRL_OBJ;

/**
    struct: bsmux mem buf.
    @note modified from NMEDIASAVEQ_MEMBUF, NMEDIA_TS_BUF, NMEDIA_TS_FILE_INFO
*/
typedef struct _BSMUX_MEM_BUF {
	//mem
	BSM_AD  addr;
	BSM_SZ  size;
	BSM_AD  end;
	BSM_AD  max;
	//bsq addr
	BSM_AD  nowaddr;
	BSM_AD  last2card;
	BSM_AD  real2card;
	BSM_AD  end2card;
	//size related
	BSM_SZ  total2card;  //total file size has been add to FS Queue && NMEDIA_TS_FILE_INFO
	BSM_SZ  allowmaxsize;   //size in FS queue but not in card
	//wrtie block
	BSM_SZ  blksize;     //one blksize
	BSM_AD  blkoffset;   //cur offset
	UINT32  blkusednum;
	//buf usage
	BSM_SZ  free_size;
	BSM_SZ  used_size;
} BSMUX_MEM_BUF;

/**
    struct: bsmux saveq bs info.
    @note modified from NMEDIA_REC_BSQ_INFO
*/
//#define BSMUX_SAVEQ_BS_INFO_SIZE 12
typedef struct _BSMUX_SAVEQ_BS_INFO {
	UINT32 uiType;                  ///< signature type
	BSM_AD uiBSMemAddr;             ///< physical address of encoded data
	BSM_SZ uiBSSize;                ///< size of encoded data
	UINT32 uiIsKey;                 ///< if video frame type I-frame
	BSM_AD uiBSVirAddr;             ///< virtual address of encoded data
	UINT32 uibufid;                 ///< memory block (if meta as memory block index)
	BSM_IO uiPortID;                ///< output port id
	UINT32 uiCountSametype;
	UINT64 uiTimeStamp;
	BSM_AD uiTOftMemAddr;
	BSM_AD uiTOftVirAddr;
} BSMUX_SAVEQ_BS_INFO;
//STATIC_ASSERT(sizeof(BSMUX_SAVEQ_BS_INFO) / sizeof(UINT32) == BSMUX_SAVEQ_BS_INFO_SIZE);

/**
    struct: bsmux saveq file info.
    @note modified from NMEDIABSQ_FILEINFO, NMEDIA_TS_FILE_INFO
*/
typedef struct _BSMUX_SAVEQ_FILE_INFO {
	UINT32 vidCount;        //vid count for entry (has added)
	UINT32 copyTotalCount;  //total count for entry (has copyed)
	UINT32 vidCopyCount;    //vid count for entry (has copyed, for moov) && NMEDIA_TS_FILE_INFO
	UINT32 audCopyCount;    //aud count for entry (has copyed, for moov) && NMEDIA_TS_FILE_INFO
	BSM_SZ copyPos;         //copy ok, file position
	UINT32 gpsCopyCount;    //gps count for entry (has copyed, for moov)
	UINT32 vidSec;          //rec sec, count from vidFrameRate
	UINT32 headerbyte;      //hdr byte per second
	UINT32 vidCopySec;      //rec copy sec (has copyed, for moov)
	UINT32 headerok;        //headerok
	UINT32 thumbok;         //thumbok
	UINT32 extNum;
	UINT32 extSec;
	UINT32 blkPut;
	UINT32 subvidCopyCount;
	UINT32 extNextSec;
	UINT32 vidDropCount;
	UINT32 audDropCount;
	UINT32 subvidDropCount;
	UINT32 cut_blk_count;
	UINT32 vid_put_count;
	UINT32 aud_put_count;
	UINT32 sub_put_count;
	UINT32 rollack_sec;
	UINT32 variable_rate;
	UINT32 meta_copy_count[BSMUX_MAX_META_NUM];
} BSMUX_SAVEQ_FILE_INFO;

/**
    video info (project setting).
*/
typedef struct _BSMUX_REC_VIDEOINFO {
	UINT32 vidcodec;                  ///< vidoe codec type (using HD_BSMUX_VIDCODEC)
	UINT32 vfr;                       ///< vidoe frame rate (round)(playback)
	UINT32 width;                     ///< video width
	UINT32 height;                    ///< video height
	BSM_AD descAddr;                  ///<
	BSM_SZ descSize;                  ///<
	UINT32 tbr;                       ///< target bitrate Bytes/sec
	UINT32 DAR;                       ///< MP_VDOENC_DAR_DEFAULT or others
	UINT32 gop;                       ///< gop
	UINT32 naluNum;
	BSM_SZ vpsSize;
	BSM_SZ spsSize;
	BSM_SZ ppsSize;
	UINT32 var_vfr;
} BSMUX_REC_VIDEOINFO;

/**
    file info.
    @note modified from MDAT_FILEINFO
*/
typedef struct _BSMUX_REC_FILEINFO {
	BSMUX_REC_VIDEOINFO vid;
	BSMUX_REC_VIDEOINFO sub_vid;
	BSMUX_REC_AUDIOINFO aud;
	UINT32 emron;
	UINT32 emrloop;
	UINT32 strgid;
	UINT32 ddr_id;
	UINT32 seamlessSec;
	UINT32 rollbacksec;	//video rollback sec
	UINT32 keepsec;		//emr keep sec
	UINT32 endtype;		//MOVREC_ENDTYPE_CUTOVERLAP
	UINT32 filetype;	//MEDIA_FILEFORMAT_MP4
	UINT32 recformat;	//MEDIAREC_AUD_VID_BOTH as default
	UINT32 playvfr;  	//30 as default, only works in MEDIAREC_TIMELAPSE/MEDIAREC_GOLFSHOT
	UINT32 revsec;  // buffer reserved sec [gNMR_bsm_revsec]
	UINT32 overlop_on; //overlap on/off [gNMR_rollbackOverlap]
	UINT32 pause_on;   // for trig emr pause
	UINT32 pause_id;   // for trig emr pause
	UINT32 pause_cnt;   // for trig emr pause
	UINT32 seamlessSec_ms;
	UINT32 rollbacksec_ms;
	UINT32 keepsec_ms;
	UINT32 revsec_ms;
} BSMUX_REC_FILEINFO;

/**
    gps info.
    @note modified from MDAT_GPSINFO
*/
typedef struct _BSMUX_REC_GPSINFO {
	UINT32 gpson;
	BSM_AD gpsdataadr;
	BSM_SZ gpsdatasize;
	UINT32 gps_rate;
	UINT32 gps_queue;
} BSMUX_REC_GPSINFO;

/**
    thumb info.
    @note modified from MDAT_THUMBINFO
*/
typedef struct _BSMUX_REC_THUMBINFO {
	UINT32 thumbon; //<obsolete>
	BSM_AD thumbadr;
	BSM_SZ thumbsize;
} BSMUX_REC_THUMBINFO;

/**
    mem info.
    @note modified from MDAT_MEMINFO & BSMUXER_MEM_RANGE
*/
typedef struct _BSMUX_REC_MEMINFO {
	BSM_AD addr;
	BSM_AD phy_addr;
	BSM_AD virt_addr;
	BSM_SZ size;
	BSM_AD end;
	BSM_SZ calc_buf; // [SET PRECALC_BUFFER] [GET ALLOC_SIZE]
} BSMUX_REC_MEMINFO;

/**
    header info.
    @note modified from MDAT_HDRINFO
*/
typedef struct _BSMUX_REC_HDRINFO {
	BSM_AD  frontheader;
	BSM_AD  backheader;
	BSM_SZ  backsize;
	BSM_AD  framebuf;
	BSM_SZ  framesize;
	BSM_AD  nidxaddr;
	BSM_AD  gpsaddr;//gps data in mdat
	BSM_AD  gpsbuffer;//gps entry
	BSM_SZ  gpsbufsize;
	BSM_AD  gpstagaddr;//gps atom in moov
	BSM_SZ  gpstagsize;
	BSM_AD  frontheader_2;
	BSM_AD  frontheader_1;
} BSMUX_REC_HDRINFO;

/**
    nidx info.
    @note modified from MDAT_NIDXINFO
*/
typedef struct {
	UINT32 nidx_en;
	UINT32 nidxok_vfn;
	UINT32 nidxok_sub_vfn;
	UINT32 nidxok_afn;
} BSMUX_REC_NIDXINFO;

/**
    userdata info.
    @note modified from NMR_MDATFILE_INFO
*/
typedef struct _BSMUX_REC_USERINFO {
	UINT32 on;
	BSM_AD addr;
	BSM_SZ size;
} BSMUX_REC_USERINFO;

/**
    wrinfo.
*/
typedef struct _BSMUX_REC_WRINFO {
	UINT32 flush_freq;
	UINT32 wrblk_count;
	BSM_SZ wrblk_size;
	UINT32 close_flag;
	UINT32 wrblk_locksec;
	UINT32 wrblk_timeout;
} BSMUX_REC_WRINFO;

/**
    moovinfo.
*/
typedef struct _BSMUX_REC_MOOVINFO {
	UINT32 fmoov_on;
	BSM_AD moov_addr;
	BSM_SZ moov_size;
	UINT32 moov_freq;
	UINT32 moov_tune;
} BSMUX_REC_MOOVINFO;

/**
    dropinfo.
*/
typedef struct _BSMUX_REC_DROPINFO {
	UINT32 enable;
	UINT32 vidcount;
	UINT32 audcount;
	UINT32 subcount;
	UINT32 datcount;
	UINT32 vid_drop_set;
	UINT32 aud_drop_set;
	UINT32 sub_drop_set;
	UINT32 dat_drop_set;
	UINT32 full_drop_set;
} BSMUX_REC_DROPINFO;

/**
    zone-utcinfo.
*/
typedef struct _BSMUX_REC_ZONEINFO {
	UINT32 utc_sign;
	UINT32 utc_zone;
} BSMUX_REC_ZONEINFO;

/**
    meta info.
*/
typedef struct _BSMUX_REC_METADATA {
	UINT32 meta_sign;
	UINT32 meta_rate;
	UINT32 meta_queue;
	UINT32 meta_index; //0 ~ (BSMUX_MAX_META_NUM-1)
} BSMUX_REC_METADATA;
typedef struct _BSMUX_REC_METAINFO {
	UINT32 meta_on;
	UINT32 meta_num;
	BSMUX_REC_METADATA meta_data[BSMUX_MAX_META_NUM];
} BSMUX_REC_METAINFO;

/**
    storage-buffer-info.
*/
typedef struct _BSMUX_STRGBUF_INFO {
	UINT32 en;                  //function enable
	// status
	UINT32 active;              //function active or not
	UINT32 cut;                 //need to cut and use bs buffer
	UINT32 is_hdr;              //header made
	UINT32 vid_ok;              //video data ready
	// frame entry
	UINT32 vid_entry_num;       //video entry num in card : as vidCopyCount
	UINT32 aud_entry_num;       //audio entry num in card : ad audCopyCount
	BSM_SZ cur_entry_pos;       //video entry pos in card : as copyPos
	UINT32 max_entry_num;       //max entry num => sec * vfr
	// card space
	BSM_SZ alloc_size;          //fallocate space size => (sec + resv) * tbr
	BSM_SZ total_size;          //total entry size (alignment) : as total2card
	BSM_SZ put_pos;             //last pos to put bs (if use, need move to blk align)
	BSM_SZ get_pos;             //first pos to get bs
} BSMUX_STRGBUF_INFO;

/**
    record setting info.
    @note modified from MDAT_ONEINFO
*/
typedef struct _BSMUX_REC_INFO {
	BSMUX_REC_FILEINFO  file;
	BSMUX_REC_GPSINFO   gps;
	BSMUX_REC_THUMBINFO thumb;
	BSMUX_REC_HDRINFO   hdr;
	BSMUX_REC_MEMINFO   mem;
	BSMUX_REC_NIDXINFO  nidxinfo;
	BSMUX_REC_USERINFO  userdata;
	CUSTOMDATA          custdata;
	BSMUX_REC_WRINFO    wrinfo;
	BSMUX_REC_EXTINFO   extinfo;
	BSMUX_REC_MOOVINFO  moov;
	BSMUX_REC_DROPINFO  drop;
	BSMUX_REC_ZONEINFO  zone;
	BSMUX_REC_METAINFO  meta;
	BSM_SZ              freasize;//out freasize
	BSM_SZ              freaendsize;//out freasize
	UINT32              start;//starting
	UINT32              freabox_en;
	UINT32              fastput_en;
	UINT32              drop_en;
	UINT32              dur_us_max;
	UINT32              mux_align;
	UINT32              mux_method;
	UINT32              boxtag_size;
	UINT32              pts_reset;
	BSM_SZ              padding_size;	////fast put flow ver3
	BSMUX_STRGBUF_INFO  strgbuf_info;
} BSMUX_REC_INFO;

/**
    header mem info.
    @note modified from NMR_MDAT_HDRMEM_INFO
    @note for make header use (mdat)
*/
typedef struct _BSMUX_HDRMEM_INFO {
	UINT32 hdrtype;
	BSM_AD hdraddr;
	BSM_SZ hdrsize;
	UINT32 hdrused;
} BSMUX_HDRMEM_INFO;

/**
	bs timestamp sync info
	@not for v/a entry sync
*/
typedef struct _BSMUX_SYNC_INFO {
	UINT64 BsTimeStamp;
	UINT64 BsDuration;
	UINT32 FrmNum;
	UINT16 TimeSyncAlm;
	UINT16 FrmSyncAlm;
	UINT16 Tolerance;
} BSMUX_SYNC_INFO;

/*-----------------------------------------------------------------------------*/
/* Type Definitions                                                            */
/*-----------------------------------------------------------------------------*/
//common (16)
typedef ER BSMUX_DBG(BOOL value);
typedef ER BSMUX_TSKOBJ_INIT(BSM_IO id, VOID *p_action);
typedef ER BSMUX_ENG_OPEN(VOID);
typedef ER BSMUX_ENG_CLOSE(VOID);
typedef BSM_SZ BSMUX_ENG_COPY(VOID *p_dest, VOID *p_src, BSM_SZ size, UINT32 method);
typedef ER BSMUX_MEM_GETSIZE(BSM_IO id, VOID *p_size);
typedef ER BSMUX_MEM_SETSIZE(BSM_IO id, BSM_AD addr, VOID *p_size);
typedef ER BSMUX_CLEAN(BSM_IO id);
typedef ER BSMUX_UPDATE_VIDINFO(BSM_IO id, BSM_AD p1, VOID *p_bsq);
typedef ER BSMUX_RELEASE_BUF(VOID *p_buf);
typedef ER BSMUX_ADD_GPS(BSM_IO id, VOID *p_bsq);
typedef ER BSMUX_ADD_THUMB(BSM_IO id, VOID *p_bsq);
typedef ER BSMUX_ADD_LAST(BSM_IO id);
typedef ER BSMUX_PUT_LAST(BSM_IO id);
typedef ER BSMUX_ADD_META(UINT32 id, void *p_bsq);
typedef BOOL BSMUX_CHECK_BUFUSE(BSM_IO id, VOID *p_bsq);
//sepcific (7)
typedef ER BSMUX_SAVE_ENTRY(BSM_IO id, VOID *p_bsq);
typedef ER BSMUX_NIDX_PAD(BSM_IO id);
typedef ER BSMUX_MAKE_HEADER(BSM_IO id, VOID *p_maker);
typedef ER BSMUX_UPDATE_HEADER(BSM_IO id, VOID *p_maker);
typedef ER BSMUX_MAKE_PES(BSM_IO id, VOID *p_bsq);
typedef ER BSMUX_MAKE_PAT(BSM_IO id, BSM_AD addr);
typedef ER BSMUX_MAKE_MOOV(BSM_IO id, VOID *p_maker, UINT32 minus1sec);

typedef struct _BSMUX_OPS {
	UINT32                Type;
	//common (16)
	BSMUX_DBG             *Dbg;
	BSMUX_TSKOBJ_INIT     *TskObjInit;
	BSMUX_ENG_OPEN        *EngOpen;
	BSMUX_ENG_CLOSE       *EngClose;
	BSMUX_ENG_COPY        *EngCpy;
	BSMUX_MEM_GETSIZE     *MemGetSize;
	BSMUX_MEM_SETSIZE     *MemSetSize;
	BSMUX_CLEAN           *Clean;
	BSMUX_UPDATE_VIDINFO  *UpdateVidInfo;
	BSMUX_RELEASE_BUF     *ReleaseBuf;
	BSMUX_ADD_GPS         *AddGPS;
	BSMUX_ADD_THUMB       *AddThumb;
	BSMUX_ADD_LAST        *AddLast;
	BSMUX_PUT_LAST        *PutLast;
	BSMUX_ADD_META        *AddMeta;
	BSMUX_CHECK_BUFUSE    *CheckBufuse;
	//sepcific (7)
	BSMUX_SAVE_ENTRY      *SaveEntry;
	BSMUX_NIDX_PAD        *NidxPad;
	BSMUX_MAKE_HEADER     *MakeHeader;
	BSMUX_UPDATE_HEADER   *UpdateHeader;
	BSMUX_MAKE_PES        *MakePES;
	BSMUX_MAKE_PAT        *MakePAT;
	BSMUX_MAKE_MOOV       *MakeMoov;
} BSMUX_OPS;

#endif //BSMUX_INT_H
