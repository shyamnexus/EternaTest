/*
    Header file for JPEG module

    This file is the header file that define the API for JPEG module.

    @file       jpeg.h
    @ingroup    mIDrvCodec_JPEG
    @note       Nothing.

    Copyright   Novatek Microelectronics Corp. 2012.  All rights reserved.
*/

#ifndef _JPEG_H
#define _JPEG_H

#include "kwrap/type.h"
#include "kdrv_videojpeg/kdrv_videodec_jpeg.h"
#include "kdrv_videojpeg/kdrv_videoenc_jpeg.h"
#include "kdrv_jpeg_queue.h"
#include "jpeg_emu.h"

#if defined(__FREERTOS)
#include "plat/interrupt.h" //irqreturn_t
#endif

#define SUPPORT_JPEGENC_SLICE		(1)
#define JPEGENC_FLUSH_HEADER		(1)
#define JPEGENC_BS_WORDALIGN		(0)
#define JPEG_DRAM2_SYS_PA			(0)		//default 0, need include sys.h

#define JPEG_KDRV_SET_RC			(1)
#define JPEG_SUPPORT_VBR_PRIORITY	(1)

#define JPEG_CLIP3(low, high, val)    ((val)>(high)?(high):((val)<(low)?(low):(val)))
#define JPEG_CLIP4(low, high, val)    (val > high ? high : ((int)val < (int)low ? low : val)) //unsigned compare

//Bold high intensity text
#define BHRED "\x1B[1;91m"
#define BHGRN "\x1B[1;92m"
#define BHYEL "\x1B[1;93m"
#define BHBLU "\x1B[1;94m"
#define BHMAG "\x1B[1;95m"
#define BHCYN "\x1B[1;96m"
#define COLOR_END	"\033[0m \n"

/**
    @addtogroup mIDrvCodec_JPEG
*/
//@{

/**
    JPEG block format

    @note For JPG_HW_YUV_FORMAT
*/
typedef enum {
	JPEG_BLK_Y_2X    = 0x0800,      ///< Y's block width is 16
	JPEG_BLK_Y_1X    = 0x0400,      ///< Y's block width is 8
	JPEG_BLK_Y_2Y    = 0x0200,      ///< Y's block height is 16
	JPEG_BLK_Y_1Y    = 0x0100,      ///< Y's block height is 8
	JPEG_BLK_U_2X    = 0x0080,      ///< U's block width is 16
	JPEG_BLK_U_1X    = 0x0040,      ///< U's block width is 8
	JPEG_BLK_U_2Y    = 0x0020,      ///< U's block height is 16
	JPEG_BLK_U_1Y    = 0x0010,      ///< U's block height is 8
	JPEG_BLK_V_2X    = 0x0008,      ///< V's block width is 16
	JPEG_BLK_V_1X    = 0x0004,      ///< V's block width is 8
	JPEG_BLK_V_2Y    = 0x0002,      ///< V's block height is 16
	JPEG_BLK_V_1Y    = 0x0001,      ///< V's block height is 8
	ENUM_DUMMY4WORD(JPEG_BLK_FORMAT)
} JPEG_BLK_FORMAT;

/**
    JPEG YUV format

    @note For jpeg_set_format()
*/
typedef enum {
	JPEG_YUV_FORMAT_100     = (JPEG_BLK_Y_1X | JPEG_BLK_Y_1Y),                                                                  ///< YUV 100 (Y plane only)
	JPEG_YUV_FORMAT_111     = (JPEG_BLK_Y_1X | JPEG_BLK_Y_1Y | JPEG_BLK_U_1X | JPEG_BLK_U_1Y | JPEG_BLK_V_1X | JPEG_BLK_V_1Y),  ///< YUV 111
	JPEG_YUV_FORMAT_211     = (JPEG_BLK_Y_2X | JPEG_BLK_Y_1Y | JPEG_BLK_U_1X | JPEG_BLK_U_1Y | JPEG_BLK_V_1X | JPEG_BLK_V_1Y),  ///< YUV 2h11 (4:2:2 in Exif)
	JPEG_YUV_FORMAT_211V    = (JPEG_BLK_Y_1X | JPEG_BLK_Y_2Y | JPEG_BLK_U_1X | JPEG_BLK_U_1Y | JPEG_BLK_V_1X | JPEG_BLK_V_1Y),  ///< YUV 2v11
	JPEG_YUV_FORMAT_222     = (JPEG_BLK_Y_2X | JPEG_BLK_Y_1Y | JPEG_BLK_U_2X | JPEG_BLK_U_1Y | JPEG_BLK_V_2X | JPEG_BLK_V_1Y),  ///< YUV 222h
	JPEG_YUV_FORMAT_222V    = (JPEG_BLK_Y_1X | JPEG_BLK_Y_2Y | JPEG_BLK_U_1X | JPEG_BLK_U_2Y | JPEG_BLK_V_1X | JPEG_BLK_V_2Y),  ///< YUV 111v
	JPEG_YUV_FORMAT_411     = (JPEG_BLK_Y_2X | JPEG_BLK_Y_2Y | JPEG_BLK_U_1X | JPEG_BLK_U_1Y | JPEG_BLK_V_1X | JPEG_BLK_V_1Y),  ///< YUV 411  (4:2:0 in Exif)
	JPEG_YUV_FORMAT_422     = (JPEG_BLK_Y_2X | JPEG_BLK_Y_2Y | JPEG_BLK_U_2X | JPEG_BLK_U_1Y | JPEG_BLK_V_2X | JPEG_BLK_V_1Y),  ///< YUV 422h
	JPEG_YUV_FORMAT_422V    = (JPEG_BLK_Y_2X | JPEG_BLK_Y_2Y | JPEG_BLK_U_1X | JPEG_BLK_U_2Y | JPEG_BLK_V_1X | JPEG_BLK_V_2Y),  ///< YUV 422v
	ENUM_DUMMY4WORD(JPEG_YUV_FORMAT)
} JPEG_YUV_FORMAT;


/**
    JPEG CODEC Mode

    @note For jpeg_add_queue()
*/
typedef enum {
	JPEG_CODEC_MODE_ENC,          ///< JPEG encode mode
	JPEG_CODEC_MODE_DEC,          ///< JPEG decode mode

	ENUM_DUMMY4WORD(JPEG_CODEC_MODE)
} JPEG_CODEC_MODE;

/**
    JPEG interrupt status

    @note For jpeg_get_status(), jpeg_set_enableint(), jpeg_set_disableint() and jpeg_waitdone().
*/
typedef enum {
	JPEG_INT_FRAMEEND   = 0x01,         ///< Frame end. Encoding/decoding a frame is done.
	///< @note  When bit-stream buffer end and frame end are both happened,
	///<        only bit-stream buffer end interrupt will be issued.
	///<        You have to set the bit-stream buffer again to issue the frame end interrupt.
	JPEG_INT_SLICEDONE  = 0x02,         ///< Slice Done. (Not support in NT96660)
	JPEG_INT_DECERR     = 0x04,         ///< Decode error. Encounter error in JPEG decode.
	JPEG_INT_BUFEND     = 0x08,         ///< Bit-stream buffer end. Bit-stream length exceed specified length.
	JPEG_INT_DECMP_ERR  = 0x10,			///< source decompress error
	JPEG_INT_ABORTDONE	= 0x20,			///< dma abort done
	JPEG_INT_TIMEOUT	= 0x40,			///< timeout interrupt
	JPEG_INT_ALL        = 0x7F,         ///< All interrupts of above

	ENUM_DUMMY4WORD(JPEG_INT)
} JPEG_INT;

/**
    JPEG Decode Scale Ratio

    @note For jpeg_set_scaleratio().
*/
typedef enum {
	JPEG_DECODE_RATIO_WIDTH_1_2,        ///< 1/2 of Width Only
	JPEG_DECODE_RATIO_BOTH_1_2,         ///< 1/2 of Width/Height
	JPEG_DECODE_RATIO_BOTH_1_4,         ///< 1/4 of Width/Height
	JPEG_DECODE_RATIO_BOTH_1_8,         ///< 1/8 of Width/Height

	ENUM_DUMMY4WORD(JPEG_DECODE_RATIO)
} JPEG_DECODE_RATIO;

/**
    JPEG DC Output Scaling ratio

    JPEG DC Output scaling ratio.
*/
typedef enum {
	JPEG_DC_OUT_RATIO_1_2 = 1,          ///< DC output scaled to 1/2
	JPEG_DC_OUT_RATIO_1_4,              ///< DC output scaled to 1/4
	JPEG_DC_OUT_RATIO_1_8,              ///< DC output scaled to 1/8

	ENUM_DUMMY4WORD(JPEG_DC_OUT_RATIO)
} JPEG_DC_OUT_RATIO;

/**
    JPEG DC Output Configuration structure

    @note For jpeg_set_dcout().
*/
typedef struct {
	UINT32              dc_enable;       ///< DC output enable
	///< - @b TRUE  : Enable DC output function
	///< - @b FALSE : Disable DC output function
	JPEG_DC_OUT_RATIO   dc_xratio;       ///< DC output width (horizontal) ratio
	JPEG_DC_OUT_RATIO   dc_yratio;       ///< DC output height (vertical) ratio
	uintptr_t           dc_yaddr;        ///< DC output Y DRAM address
	uintptr_t           dc_uaddr;        ///< DC output UV packed DRAM address
	uintptr_t           dc_vaddr;        ///< Obsoleted member
	UINT32              dc_ylineoffset;  ///< DC output Y line offset
	UINT32              dc_ulineoffset;  ///< DC output UV packed line offset
	UINT32              dc_vlineoffset;  ///< Obsoleted member
} JPEG_DC_OUT_CFG, *PJPEG_DC_OUT_CFG;


/**    JPEG Bit stream DMA burst length    JPEG BS dma burst length.*/
typedef enum
{	JPEG_BS_DMA_BURST_32W,          ///< BS DMA burst 32W
    JPEG_BS_DMA_BURST_64W,          ///< BS DMA burst 64W
    ENUM_DUMMY4WORD(JPEG_BS_DMA_BURST)
} JPEG_BS_DMA_BURST;

/**
    JPEG Rotate mode

    @note For jpeg_set_rotate().
*/
typedef enum
{
    JPG_ROTATE_DISABLE = 0,             ///< Not rotate
    JPG_ROTATE_CCW      = 1,            ///< Rotate counter-clockwise
    JPG_ROTATE_CW     = 2,              ///< Rotate clockwise
    JPG_ROTATE_180     = 3,             ///< Rotate 180 degree clockwise

    ENUM_DUMMY4WORD(JPG_HW_ROTATE_MODE)
} JPG_HW_ROTATE_MODE;

/**
    JPEG Bit Rate Control Information

    @note For jpeg_get_brcinfo().
*/
typedef struct {
	UINT32 brcinfo1;                  ///< Rho Q/8
	UINT32 brcinfo2;                  ///< Rho Q/4
	UINT32 brcinfo3;                  ///< Rho Q/2
	UINT32 brcinfo4;                  ///< Rho Q
	UINT32 brcinfo5;                  ///< Rho 2Q
	UINT32 brcinfo6;                  ///< Rho 4Q
	UINT32 brcinfo7;                  ///< Rho 8Q
} JPEG_BRC_INFO, *PJPEG_BRC_INFO;

/**
    JPEG configuration identifier

    @note For jpeg_set_config()
*/
typedef enum {
	JPEG_CONFIG_ID_CHECK_DEC_ERR,           ///< Enable / Disable checking decode error
	///< (Default is DISABLED)
	///< Context can be:
	///< - @b TRUE  : JPEG driver will check decode error
	///< - @b FALSE : JPEG driver will NOT check decode error

	JPEG_CONFIG_ID_FREQ,                    ///< JPEG module clock (Unit: MHz), will be active on succeeding jpeg_open().
	///< Context can be one of the following:
	///< - @b 192   : 192 MHz (Default value)
	///< - @b 240   : 240 MHz
	///< - @b 250   : 250 MHz

	ENUM_DUMMY4WORD(JPEG_CONFIG_ID)
} JPEG_CONFIG_ID;


// typedef void (*JPEG_TRI_FUNC)(JPEG_CODEC_MODE codec_mode, void *p_param, KDRV_CALLBACK_FUNC *p_cb_func);
typedef int (*JPEG_TRI_FUNC)(JPEG_CODEC_MODE codec_mode, void *p_param, KDRV_CALLBACK_FUNC *p_cb_func, uintptr_t io_addr);

typedef struct _KDRV_JPEG_TRIG_INFO {
	KDRV_CALLBACK_FUNC     *cb;
	VOID                   *user_data;
	//UINT32                  flg_ptn;
	BOOL                    is_busy;
	KDRV_JPEG_QUEUE_INFO    *p_queue;
	JPEG_TRI_FUNC           tri_func;
	KDRV_VDOJPGE_PARAM      *jpeg_enc_param;
	KDRV_VDOJPGD_PARAM      *jpeg_dec_param;
} KDRV_JPEG_TRIG_INFO;

/**
    JPEG gray Parameter
*/
typedef struct {
	BOOL enable;
	BOOL color_to_gray;
	BOOL src_color_to_gray;
} JPEG_GRAY_CFG;

/**
    JPEG user data Parameter
*/
typedef struct {
	BOOL enable;
	uintptr_t data_addr;
	UINT32 data_length;
} JPEG_USER_DATA_CFG;

/**
    JPEG OSG Parameter
*/
#define MAX_JPEG_OSG_LAYER		1
#define MAX_JPEG_OSG_NUM		16 //538: 16, 567: 10
#define MAX_JPEG_OSG_PAL_NUM	16

typedef struct {
	UINT8	ucRgb2Yuv[3][3];
} JPEG_OSG_RGB_CFG;

typedef struct {
	UINT8 ucAlpha;
	UINT8 ucRed;
	UINT8 ucGreen;
	UINT8 ucBlue;
} JPEG_OSG_PAL_CFG;

typedef struct {
	UINT8  ucType;
	UINT16 usWidth;
	UINT16 usHeight;
	UINT16 usLofs;
	uintptr_t uiAddr;
	uintptr_t uiAddr_pa;
} JPEG_OSG_GRAP_CFG;

typedef struct {
	UINT8  ucMode;
	UINT16 usXStr;
	UINT16 usYStr;
	UINT8  ucBgAlpha;
	UINT8  ucFgAlpha;
	UINT8  ucMaskType;
	UINT8  ucMaskBdSize;
	UINT8  ucMaskY[2];
	UINT8  ucMaskCb;
	UINT8  ucMaskCr;
	UINT8  ucMosaicBlkSz;
} JPEG_OSG_DISP_CFG;

typedef struct {
	BOOL bEnable;
	BOOL bAlphaEn;
	UINT8 ucAlpha;
	UINT8 ucRed;
	UINT8 ucGreen;
	UINT8 ucBlue;
} JPEG_OSG_COLORKEY_CFG;

typedef struct {
	BOOL	bEnable;
	BOOL 	bRoiInvalid;
	JPEG_OSG_GRAP_CFG		stGrap;
	JPEG_OSG_DISP_CFG		stDisp;
	JPEG_OSG_COLORKEY_CFG	stKey;
} JPEG_OSG_WIN_CFG;

typedef struct {
	UINT32 overlap_type;
} JPEG_OSG_GLOBAL_CFG;

typedef struct {
	JPEG_OSG_WIN_CFG stOsgWin[MAX_JPEG_OSG_LAYER][MAX_JPEG_OSG_NUM];
	JPEG_OSG_PAL_CFG stOsgPal[MAX_JPEG_OSG_PAL_NUM];
	JPEG_OSG_RGB_CFG stOsgRgb;
	JPEG_OSG_GLOBAL_CFG stOsgGlobal;
} JPEG_OSG_CFG;

/**
    JPEG MASK Parameter
*/
#define MAX_JPEG_MASK_NUM		16
#define MAX_JPEG_MASK_PAL_NUM	16

typedef struct {
	UINT8 ucMosaicBlkW;	// keep latest set mosaic blk w //
	UINT8 ucMosaicBlkH;	// keep latest set mosaci blk h //

	UINT8  ucPalY[MAX_JPEG_MASK_PAL_NUM];
	UINT8  ucPalCb[MAX_JPEG_MASK_PAL_NUM];
	UINT8  ucPalCr[MAX_JPEG_MASK_PAL_NUM];
} JPEG_MASK_INIT_CFG;

typedef struct {
	UINT16 usPosX;
	UINT16 usPosY;
} JPEG_MASK_POS_CFG;

typedef struct {
	INT32 iCoeffa;
	INT32 iCoeffb;
	INT64 iCoeffc;

	UINT16 usX0;
	UINT16 usY0;
	UINT16 usX1;
	UINT16 usY1;

	UINT8  ucTh;
	UINT32 uiLineChkAdd;
} JPEG_MASK_LINE;

typedef struct {
	UINT8  ucConcave;
	UINT8  ucMinx;
	UINT8  ucMiny;
	UINT8  ucMaxx;
	UINT8  ucMaxy;
} JPEG_MASK_CONCAVE;

typedef struct {
	BOOL bEnable;

	UINT8  ucDid;
	UINT8  ucPalSel;
	UINT8  ucLineHitOpt;
	UINT16 usAlpha;
	UINT8  ucAlphaType;
	UINT8  ucComp[4];
	UINT8  ucHitWidth[4]; // when line hit opt is 2 or 3, hit width is 2; when line hit opt is 0, 1 or 4, hit width is 0; range: 0~15

	JPEG_MASK_POS_CFG stPos[4];

	JPEG_MASK_LINE     stLine[4];
	JPEG_MASK_CONCAVE  stConcave;
	UINT32  line_chk_th;

	UINT32	mask_hollow_type;
	UINT32	mask_hollow_size;
	UINT32	mask_width;
	UINT32	mask_height;
} JPEG_MASK_WIN_CFG;

typedef struct {
	BOOL bEnable;			// for total mask enable/disable //

	JPEG_MASK_INIT_CFG stInitCfg;
	JPEG_MASK_WIN_CFG  stWinCfg[MAX_JPEG_MASK_NUM];
} JPEG_MASK_CFG;

/**
    JPEG Rate Control Parameter
*/
#define FRAMERATE_BASE              1000
#define MAX_FRAME_RATE              120
#define DEFAULT_BRC_UPPER_BOUND     105//%
#define DEFAULT_BRC_LOWER_BOUND     95//%

typedef enum {
	VBR_CHANGE_FPS_STATE = 11,
	VBR_CHANGE_Q_STATE = 12,
	ENUM_DUMMY4WORD(JPG_RC_NEW_VBR_STATE),
} JPG_RC_NEW_VBR_STATE;

typedef struct {
	UINT32  width;                          ///< Image width
	UINT32  height;                         ///< Image height
	UINT32  ubound_byte;                     ///< Upper bound bit-stream size in byte
	UINT32  target_byte;                     ///< Target bit-stream size in byte
	UINT32  lbound_byte;                     ///< Lower bound bit-stream size in byte
} JPGBRC_PARAM, *PJPGBRC_PARAM;

typedef struct {
	UINT32 byte_record[MAX_FRAME_RATE];
	UINT32 push_idx;
	UINT32 pop_idx;
	UINT32 record_num;
	UINT32 cur_byterate;
	UINT32 measure_period;
	UINT32 br_frm_cnt;
} JPEG_BITRATE_RECORD;

typedef struct {
	// parameter
	BOOL enable;
	UINT32 quality;
	UINT32 vbr_mode;
	UINT32 min_quality;
	UINT32 max_quality;
	UINT32 target_rate;                ///< [w] bit rate
	//UINT32 frame_rate;                 ///< [w] frame rate (base 1000 ? )
	UINT32 frame_rate_base;				/// frame rate = frame base / frame incr
	UINT32 frame_rate_incr;

	// internal variable
	BOOL update;
	//UINT32 quality;
	UINT32 brcTargetSize;			/// target: one frame size
	//UINT32 brcCurrTargetRate;		/// vbr threshold

	//UINT32 encode_width;
	//UINT32 encode_height;
	UINT32 in_fmt;

	UINT32 brcStdQTableQuality;
	UINT32 brcQF;
	JPGBRC_PARAM brcParam;

	// VBR
	UINT32 vbrState;	// VBR_NORMAL, VBR_TO_CBR
	UINT32 vbrModechange;
	UINT32 vbrQuality;
	UINT32 vbrQ;

	//UINT32 totalFrameID;
	//UINT32 frameWindowSize;
	JPEG_BITRATE_RECORD brRecord;
	//UINT32 rec_frame_cnt;
	//UINT32 rec_byte_rate;
	
	UINT32 base_qp;

	UINT32 overflow_bitrate;
	// new vbr policy
	UINT32 vbr_priority;	// 0: origin, 1: quality priority, 2: frame rate priority
	UINT32 min_framerate;
	#if JPEG_SUPPORT_VBR_PRIORITY
	UINT32 update_period;
	UINT32 update_frame_cnt;
	UINT32 new_vbr_state;
	// for drop frame
	UINT32 src_framerate;
	UINT32 cur_framerate;
	UINT32 src_pos;		// origin frame pos
	UINT32 actual_pos;	// new frame rate pos
	UINT8 skip_frame;
	// bitrate measure
	UINT32 vbr_bitrate;
	UINT32 bitrate_upper;
	UINT32 bitrate_lower;
	// for update Q
	UINT32 min_fps_target_byte;
	UINT32 min_fps_target_upper;
	UINT32 min_fps_target_lower;
	#endif
} JPEG_RC_PARAM;

/**
    JPEG ROI Parameter
*/
#define MAX_JPEG_ROI_NUM	11
typedef struct {
	UINT32 roi_pos_x;
	UINT32 roi_pos_y;
	UINT32 roi_width;
	UINT32 roi_height;
	UINT32 roi_enable;
	UINT32 roi_threshold;
	UINT32 roi_threshold_ac1_ac2;
	UINT32 roi_sub_ac1_en;
} JPEG_ROI_PARAM;

typedef struct {
	JPEG_ROI_PARAM stRoiParam[MAX_JPEG_ROI_NUM];
} JPEG_ROI_CFG;

typedef struct {
	BOOL disable;		// disable padding when resolution is not 16x alignment
	UINT32 mode;		// reserved value
} JPEG_PAD_CFG;

/**
    JPEG Process Parameter
*/
typedef struct {
	// OSG
	JPEG_OSG_CFG stOsg;
	// MASK
	JPEG_MASK_CFG stMask;
	// rate control
	JPEG_RC_PARAM stRC;
	// gray
	JPEG_GRAY_CFG stGray;
	// rotation
	UINT32 uiRotation;
	// user data
	JPEG_USER_DATA_CFG stUserData;	
	// ROI
	JPEG_ROI_CFG stRoi;
	// PAD
	JPEG_PAD_CFG stPad;
} JPEG_PROCESS_CFG;

typedef struct {
       // Y out only
       BOOL bDecYOutOnly;
} JPEG_DEC_PROCESS_CFG;

typedef struct {
	uintptr_t	io_addr;
	UINT32		clock_freq;
	UINT32		id;
	UINT32		sram_shutdown;
	UINT32		m_reset;
	UINT32		w_reset;
	UINT32		clock_en;
} JPEG_ENGINE_CFG;

// -------------------------------------------------
// The API for the JPEG codec driver
// -------------------------------------------------
#if defined(__FREERTOS)
extern ER			jpeg_open(int chip, int engine);
extern ER			jpeg_close(int chip, int engine);
extern irqreturn_t	jpeg_isr(int irq, void *devid);
#else
extern ER			jpeg_open(int chip, int engine, uintptr_t io_addr);
extern ER			jpeg_close(int chip, int engine, uintptr_t io_addr);
extern UINT32		jpeg_isr(UINT32 entity_idx);
#endif
extern BOOL			jpeg_is_opened(void);
extern ID			jpeg_getlock_status(void);

extern ER			jpeg_set_config(JPEG_CONFIG_ID cfg_id, UINT32 cfg);
extern UINT32		jpeg_get_config(JPEG_CONFIG_ID cfg_id);

extern UINT32		jpeg_get_status(uintptr_t io_addr);
extern UINT32		jpeg_get_activestatus(uintptr_t io_addr);
extern UINT32		jpeg_get_bssize(uintptr_t io_addr);
extern int			jpeg_set_swreset(uintptr_t io_addr);

extern int			jpeg_set_startencode(uintptr_t addr, UINT32 size, uintptr_t io_addr);
extern int			jpeg_set_startdecode(UINT32 bs_ori_size, uintptr_t io_addr);

extern void			jpeg_set_endencode(uintptr_t io_addr);
extern void			jpeg_set_enddecode(uintptr_t io_addr);

extern UINT32		jpeg_waitdone(uintptr_t io_addr);
extern BOOL			jpeg_waitdone_polling(void);

extern ER			jpeg_set_enableint(UINT32 interrupt, uintptr_t io_addr);
extern void			jpeg_set_disableint(UINT32 interrupt, uintptr_t io_addr);

// Store logical address
extern int			jpeg_set_addr_va(uintptr_t y_addr_va, uintptr_t uv_addr_va, uintptr_t bs_addr_va, uintptr_t sec_bs_addr_va, uintptr_t io_addr);

extern void			jpeg_set_imgstartaddr(uintptr_t yaddr, uintptr_t uaddr, uintptr_t vaddr, uintptr_t io_addr);
extern void			jpeg_set_imgstartaddr_oft(uintptr_t yaddr, uintptr_t uaddr, uintptr_t vaddr, INT32 y_offset, INT32 c_offset, uintptr_t io_addr);
extern void			jpeg_get_imgstartaddr(UINT32 *yaddr, UINT32 *uaddr, UINT32 *vaddr, uintptr_t io_addr);

extern ER			jpeg_set_imglineoffset(UINT32 YLOFS, UINT32 ULOFS, UINT32 VLOFS, uintptr_t io_addr);
extern UINT32		jpeg_get_imglineoffsety(uintptr_t io_addr);
extern UINT32		jpeg_get_imglineoffsetu(uintptr_t io_addr);
extern UINT32		jpeg_get_imglineoffsetv(void);

extern ER			jpeg_set_format(UINT32 imgwidth, UINT32 imgheight, JPEG_YUV_FORMAT fmt, uintptr_t io_addr);

// Bit-stream control
extern ER			jpeg_set_bsstartaddr(uintptr_t bs_addr, UINT32 buf_size, uintptr_t sec_bs_addr, UINT32 sec_buf_size, UINT32 decode_mode, uintptr_t io_addr);
extern uintptr_t	jpeg_get_bsstartaddr(uintptr_t io_addr);
extern uintptr_t	jpeg_get_bscurraddr(uintptr_t io_addr);
extern UINT32 		jpeg_get_oribssize(uintptr_t io_addr);		// YCT
extern void			jpeg_set_bsoutput(BOOL en, uintptr_t io_addr);

// Restart marker
extern ER			jpeg_set_restartinterval(UINT32 mcu_num, uintptr_t io_addr);
extern ER			jpeg_set_restartenable(BOOL en, uintptr_t io_addr);
extern UINT32		jpeg_set_restarteof(uintptr_t io_addr);
extern void			jpeg_set_restartpatch(BOOL en, uintptr_t io_addr);
extern ER			jpeg_set_eofenable(BOOL en, uintptr_t io_addr);

// Encode format transform (420 -> 422)
extern void			jpeg_set_fmt_transform(BOOL enable, uintptr_t io_addr);
// extern void     jpeg_set_fmttransdisable(void);

// Encode DC output
extern ER			jpeg_set_dcout(PJPEG_DC_OUT_CFG pdcout_cfg, uintptr_t io_addr);

// Decode Y out only
extern int jpeg_set_dec_y_out_cfg(UINT32 chn_id, BOOL bDecYOutOnly);
extern void jpeg_set_decode_y_out_only(BOOL enable, uintptr_t io_addr);

// Decode crop
extern ER			jpeg_set_crop(UINT32 sta_x, UINT32 sta_y, UINT32 width, UINT32 height, uintptr_t io_addr);
extern void			jpeg_set_cropdisable(uintptr_t io_addr);
extern void			jpeg_set_cropenable(uintptr_t io_addr);

// Decode scaling
extern void			jpeg_set_scaleenable(uintptr_t io_addr);
extern void			jpeg_set_scaledisable(uintptr_t io_addr);
extern ER			jpeg_set_scaleratio(JPEG_DECODE_RATIO scale_ratio, uintptr_t io_addr);

// Rotate
extern ER			jpeg_set_rotate(JPG_HW_ROTATE_MODE RotateMode, uintptr_t io_addr); //IMGDMA
extern ER			jpeg_set_newimgdma(BOOL en, uintptr_t io_addr); //BSDMA
extern ER			jpeg_set_burstlen(JPEG_BS_DMA_BURST bslen, uintptr_t io_addr);

// Table access
extern ER			jpeg_set_hwqtable(UINT8 *pqtab_y, UINT8 *pqtab_uv, uintptr_t io_addr);
extern void			jpeg_enc_set_hufftable(UINT16 *phufftablumac, UINT16 *phufftablumdc, UINT16 *phufftabchrac, UINT16 *phufftabchrdc, uintptr_t io_addr);
extern void			jpeg_set_decode_hufftabhw(UINT8 *phuffdc0th, UINT8 *phuffdc1th, UINT8 *phuffac0th, UINT8 *phuffac1th, uintptr_t io_addr);

// Profiling
extern UINT32		jpeg_get_cyclecnt(uintptr_t io_addr);

// BRC control
extern void			jpeg_get_brcinfo(PJPEG_BRC_INFO p_brcinfo, uintptr_t io_addr);

extern UINT32		jpg_get_bs_offset(uintptr_t bs_addr, UINT32 header_size);

// extern ER			jpeg_trigger(JPEG_CODEC_MODE codec_mode, void *p_param, KDRV_CALLBACK_FUNC *p_cb_func);
extern int			jpeg_trigger(JPEG_CODEC_MODE codec_mode, void *p_param, KDRV_CALLBACK_FUNC *p_cb_func, uintptr_t io_addr);
extern int			jpeg_lookup_engine_emu(uintptr_t io_addr);
extern ER			jpeg_add_queue(UINT32 id, JPEG_CODEC_MODE codec_mode, void *p_param, KDRV_CALLBACK_FUNC *p_cb_func, VOID *p_user_data);
extern void			jpeg_set_property(KDRV_JPEG_INFO *p_jpeg_info);
extern KDRV_JPEG_TRIG_INFO *kdrv_jpeg_get_triginfo_by_coreid(void);

// set rotate //
extern int jpeg_set_rotate_cfg(UINT32 chn_id, UINT32 uiRotate);
extern ER jpeg_set_rotate_reg(UINT32 uiRotation, uintptr_t io_addr);

// set gray //
extern int jpeg_set_gray_cfg(UINT32 chn_id, JPEG_GRAY_CFG *pGray);
extern ER jpeg_set_gray_reg(JPEG_GRAY_CFG *pGray, uintptr_t io_addr);

// set user data
extern int jpeg_set_user_data_cfg(UINT32 chn_id, JPEG_USER_DATA_CFG *pData);

extern UINT32 jpeg_get_DCQvalue(uintptr_t io_addr);

// dma abort //
extern ER jpeg_set_dma_abort(uintptr_t io_addr);
extern UINT32 jpeg_get_dma_abort(uintptr_t io_addr);
extern UINT32 jpeg_get_dma_idle(uintptr_t io_addr);
extern UINT32 jpeg_get_dma_ack(uintptr_t io_addr);

// pad reg //
extern ER jpeg_set_pad_info(UINT32 width, UINT32 height, UINT32 rotation, BOOL disable, UINT32 mode, uintptr_t io_addr);

extern void jpeg_flush_bsbuf(uintptr_t addr, UINT32 size);
//extern void jpeg_frame_window_count(UINT32 id, UINT32 size, uintptr_t io_addr);

// ycc api //
extern ER jpeg_set_init_sde(uintptr_t io_addr);
extern ER jpeg_set_sde_enable(BOOL enable, uintptr_t io_addr);

// rate control kdriver api //
extern int jpeg_enc_set_rc(UINT32 chn_id, KDRV_VDOJPGE_RC *p_rc_param);
extern int jpeg_enc_get_rc(UINT32 chn_id, KDRV_VDOJPGE_RC *p_rc_param);

// osg kdriver api //
extern int jpeg_set_osg_win_cfg(UINT32 chn_id, UINT8 ucLayerIdx, UINT8 ucWinIdx, JPEG_OSG_WIN_CFG *pOsgWin);
extern int jpeg_set_osg_rgb_cfg(UINT32 chn_id, JPEG_OSG_RGB_CFG *pOsgRgb);
extern int jpeg_set_osg_pal_cfg(UINT32 chn_id, UINT8 ucPalIdx, JPEG_OSG_PAL_CFG *pOsgPal);
extern int jpeg_set_osg_global_cfg(UINT32 chn_id, JPEG_OSG_GLOBAL_CFG *pOsgGlobal);
extern int jpeg_get_osg_win_cfg(UINT32 chn_id, void *param);
extern int jpeg_get_osg_rgb_cfg(UINT32 chn_id, void *param);
extern int jpeg_get_osg_pal_cfg(UINT32 chn_id, void *param);
// extern int jpeg_get_osg_global_cfg(UINT32 chn_id, void *param);

// osg register setting //
extern ER jpeg_set_osg_reg(JPEG_OSG_CFG *p_osg, UINT32 uiImgW, UINT32 uiImgH, uintptr_t io_addr);
extern ER jpeg_set_osg_rgb_reg(JPEG_OSG_CFG *p_osg, uintptr_t io_addr);
extern ER jpeg_set_osg_pal_reg(JPEG_OSG_CFG *p_osg, uintptr_t io_addr);
extern ER jpeg_clean_osg(JPEG_OSG_CFG *p_osg);
extern BOOL jpeg_check_osg_enable(JPEG_OSG_CFG *p_osg);

// mask kdriver api //
extern int jpeg_set_mask_init_cfg(UINT32 chn_id, JPEG_MASK_INIT_CFG *pMaskInitCfg);
extern int jpeg_set_mask_win_cfg(UINT32 chn_id, UINT8 ucMaskId, JPEG_MASK_WIN_CFG *pMaskWinCfg);
extern ER jpeg_set_mask_line_cfg(JPEG_MASK_WIN_CFG *pMaskWin);

// roi kdriver api //
extern int jpeg_set_roi_cfg(UINT32 chn_id, UINT8 roi_idx, JPEG_ROI_PARAM *p_roi_param);
extern ER jpeg_set_roi_reg(JPEG_ROI_CFG *p_roi_cfg, uintptr_t io_addr);
extern ER jpeg_clear_roi_reg(JPEG_ROI_CFG *p_roi_cfg);
// roi emulation function //
extern ER jpeg_set_roi_reg_emu(UINT32 roi_reg[ROI_REGION][ROI_LEVEL], uintptr_t io_addr);
extern ER jpeg_clear_roi_reg_emu(uintptr_t io_addr);

// pad kdrvier api //
extern int jpeg_set_pad_cfg(UINT32 chn_id, JPEG_PAD_CFG *p_pad);

// mask register setting //
extern ER jpeg_set_mask_reg(JPEG_MASK_CFG *pMask, uintptr_t io_addr);
extern ER jpeg_clear_mask(JPEG_MASK_CFG *pMask);
// ctg emulation function //
extern ER jpeg_set_ctg_reg_emu(UINT32 ctg_src_en, UINT32 ctg_out_en, uintptr_t io_addr);
// mask emulation function //
extern ER jpeg_set_mask_reg_emu(UINT32 mask_reg[417], uintptr_t io_addr);
extern ER jpeg_set_mask_disable_emu(uintptr_t io_addr);
// osg emulation function //
extern ER jpeg_set_osg_win_reg_emu(UINT32 osg_graph_reg[OSG_LAYER][OSG_REGION][9], UINT32 osg_chroma, uintptr_t io_addr);
extern ER jpeg_set_osg_cst_reg_emu(UINT32 *osg_cst_reg, uintptr_t io_addr);
extern ER jpeg_set_osg_pal_reg_emu(UINT32 *osg_pal, uintptr_t io_addr);
extern ER jpeg_set_osg_win_disable_emu(uintptr_t io_addr);
// ycc emulation function //
extern ER jpeg_set_sde_reg_emu(UINT32 *ycc_reg, uintptr_t io_addr);
// pad emulation function //
extern ER jpeg_set_pad_reg_emu(UINT32 pad_right, UINT32 pad_bottom, uintptr_t io_addr);
extern ER jpeg_set_dec_pad_reg_emu(UINT32 pad_right, UINT32 pad_bottom, uintptr_t io_addr);
// timeout th //
extern ER emu_jpeg_set_timeout_th(UINT32 timeout_threshold, uintptr_t io_addr);
// dummy write //
extern ER jpeg_set_dummy_write(UINT32 write_num, uintptr_t dma_addr, uintptr_t io_addr);
// engine config //
extern void jpeg_set_engine_cfg(JPEG_ENGINE_CFG *pEngine);
//@}

extern ER jpeg_set_uv_swap(BOOL uv_swap, uintptr_t io_addr);
extern void jpeg_dump_reg(uintptr_t io_addr);
extern void jpeg_dump_osg_reg(uintptr_t io_addr);
extern void jpeg_dump_ycc_reg(uintptr_t io_addr);

// kdriver API
extern ER jpeg_putjob(JPEG_CODEC_MODE codec_mode, unsigned int kdrv_id, void *p_param, void *user_data, KDRV_CALLBACK_FUNC *p_cb_func);
extern ER jpeg_stopjob(JPEG_CODEC_MODE codec_mode, unsigned int kdrv_id);
extern void jpeg_enc_set_cb(KDRV_CALLBACK_FUNC *p_cb_func);
extern KDRV_CALLBACK_FUNC *jpeg_enc_get_cb(void);
extern void jpeg_dec_set_cb(KDRV_CALLBACK_FUNC *p_cb_func);
extern KDRV_CALLBACK_FUNC *jpeg_dec_get_cb(void);

/* return value:
 *	1: get frame end interrupt.
 *	0: not get frame end interrupt.
 */
extern int jpeg_check_frame_end_isr(uintptr_t io_addr);

#endif
