/**
    Public header file for Sensor driver

    This file is the header file that define the API and data type for Sensor driver.

    @file       ctl_sen.h
    @ingroup    mIDrvSensor
    @note       Nothing (or anything need to be mentioned).

    Copyright   Novatek Microelectronics Corp. 2009.  All rights reserved.
*/
#ifndef _CTL_SENSOR_H
#define _CTL_SENSOR_H


// common
#include "comm/sif_api.h"
#include "kwrap/flag.h"
#include "kwrap/semaphore.h"
#include "kwrap/type.h"
#include "kwrap/platform.h"
#include "kwrap/task.h"
#include "kwrap/list.h"
#include "kwrap/error_no.h"
#include "kwrap/stdio.h"
// kdrv
#include "kdrv_videocapture/kdrv_ssenif.h"
// kflow
/* for isp build error */
#if defined(__FREERTOS) || defined(__ECOS) || defined(__UITRON)
#else
#include <linux/init.h>
#include <linux/module.h>
#include <linux/random.h>
#include <linux/delay.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>
#include <linux/string.h>
#endif

#if defined(__FREERTOS)
#define CTL_SEN_READY DISABLE
#define CTL_SEN_KDRV_CG_READY ENABLE
#define CTL_SEN_KDRV_TOP_READY ENABLE
#define CTL_SEN_KDRV_SSENIF_READY DISABLE
#else
#define CTL_SEN_READY DISABLE
#define CTL_SEN_KDRV_CG_READY DISABLE
#define CTL_SEN_KDRV_TOP_READY ENABLE
#define CTL_SEN_KDRV_SSENIF_READY DISABLE
#endif

/*******************************************************************************/
/*                            Global ITEMs                                     */
/*******************************************************************************/
#define CTL_SEN_IGNORE  0xffffffff  ///< sensor ignore information
#define CTL_SEN_CMD_DELAY       0xffffffff // delay
#define CTL_SEN_CMD_SETVD       0xfffffffe // set vd period
#define CTL_SEN_RATIO(w, h) (((UINT32)(UINT16)(w) << 16) | (UINT32)(UINT16)(h))

#define CTL_SEN_SER_MAX_DATALANE 16		// max data lane for user (virtual). may combine multiple senphy as single serial controller
#define CTL_SEN_PHY_MAX_DATALANE 8		// max data lane for (each) senphy (physical)
#define CTL_SEN_LVDS_MAX_DATALANE 8
#define CTL_SEN_CSI_MAX_DATALANE 4
#define CTL_SEN_SLVSEC_MAX_DATALANE 8
#define CTL_SEN_MFRAME_MAX_NUM 4
#define CTL_SEN_MFRAME_SLVSEC_MAX_NUM 4
#define CTL_SEN_COMB_MAX_NUM 2
#define CTL_SEN_CMD2STR(a) (#a)
#define CTL_SEN_60M_HZ  60000000
#define CTL_SEN_120M_HZ 120000000
#define CTL_SEN_240M_HZ 240000000
#define CTL_SEN_320M_HZ 320000000
#define CTL_SEN_480M_HZ 480000000
#define CTL_SEN_TGE_TAG 0x01000000 // tge tag
#define CTL_SEN_VX1_TAG 0x02000000 // vx1 tag

/*
    error code define
*/
#define CTL_SEN_E_OK        (0) // normal completion
// sen ctrl flow (for output & dump)
#define CTL_SEN_E_STATE     (-1)    // state machine error
#define CTL_SEN_E_MAP_TBL   (-2)    // map table error, check init_cfg/reg_sendrv/unreg_sendrv
#define CTL_SEN_E_ID_OVFL   (-3)    // input id overflow
#define CTL_SEN_E_IN_PARAM  (-4)    // input parameter error
#define CTL_SEN_E_PINMUX    (-5)    // input pinmux error, check init_cfg pinmux
#define CTL_SEN_E_CLK       (-6)    // cfg clock error
#define CTL_SEN_E_SYS       (-7)    // system error
#define CTL_SEN_E_NS        (-8)    // feature not supported
#define CTL_SEN_E_IF        (-9)    // sensor interface error (LVDS/CSI/SLVS-EC/TGE)
#define CTL_SEN_E_CMDIF     (-10)   // sensor cmd interface error (VX1)
#define CTL_SEN_E_SENDRV    (-11)   // sensor driver error
#define CTL_SEN_E_NOMEM     (-12)   // memory allocate fail
#define CTL_SEN_E_IF_GLB    (-13)   // sensor interface glb error


/**
    chip_id
*/
typedef enum {
	CTL_SEN_ID_1 = 0,  ///< CTL of Sensor ID
	CTL_SEN_ID_2,      ///< CTL of Sensor ID
	CTL_SEN_ID_3,      ///< CTL of Sensor ID
	CTL_SEN_ID_4,      ///< CTL of Sensor ID
	CTL_SEN_ID_5,      ///< CTL of Sensor ID
	CTL_SEN_ID_6,      ///< CTL of Sensor ID
	CTL_SEN_ID_7,      ///< CTL of Sensor ID
	CTL_SEN_ID_8,      ///< CTL of Sensor ID
#if 0 // 567 remove
	CTL_SEN_ID_9,      ///< CTL of Sensor ID
	CTL_SEN_ID_10,      ///< CTL of Sensor ID
	CTL_SEN_ID_11,      ///< CTL of Sensor ID
	CTL_SEN_ID_12,      ///< CTL of Sensor ID
#endif

	CTL_SEN_ID_MAX,
	ENUM_DUMMY4WORD(CTL_SEN_ID)
} CTL_SEN_ID;


/*
    sensor mode frame_idx information (only for sendrv, not for user)
*/
#define CTL_SEN_MODE_OFS 0
#define CTL_SEN_MODE_EXT_OFS 8
#define CTL_SEN_MODE_FRMIDX_OFS 12

#define CTL_SEN_MODE_PWR (1 << CTL_SEN_MODE_EXT_OFS) // 0x00000100

//#define CTL_SEN_MODE_FRMIDX_0 (0 << CTL_SEN_MODE_FRMIDX_OFS) // 0x0000
#define CTL_SEN_MODE_FRMIDX_1 (1 << CTL_SEN_MODE_FRMIDX_OFS) // 0x1000
#define CTL_SEN_MODE_FRMIDX_2 (2 << CTL_SEN_MODE_FRMIDX_OFS) // 0x2000
#define CTL_SEN_MODE_FRMIDX_3 (3 << CTL_SEN_MODE_FRMIDX_OFS) // 0x3000
#define CTL_SEN_MODE_FRMIDX_4 (4 << CTL_SEN_MODE_FRMIDX_OFS) // 0x4000
#define CTL_SEN_MODE_FRMIDX_5 (5 << CTL_SEN_MODE_FRMIDX_OFS) // 0x5000
#define CTL_SEN_MODE_FRMIDX_6 (6 << CTL_SEN_MODE_FRMIDX_OFS) // 0x6000
#define CTL_SEN_MODE_FRMIDX_7 (7 << CTL_SEN_MODE_FRMIDX_OFS) // 0x7000

#define CTL_SEN_MODE_MASK_SENMODE(mode) ((mode & 0x000000ff) >> CTL_SEN_MODE_OFS)           // sensor mode
#define CTL_SEN_MODE_MASK_FRMIDX(mode)  ((mode & 0x0000f000) >> CTL_SEN_MODE_FRMIDX_OFS)    // multi-frame idx

/**
    Sensor mode
*/
typedef enum {
	CTL_SEN_MODE_1       = 0,    ///< Sensor mode 1
	CTL_SEN_MODE_2       = 1,    ///< Sensor mode 2
	CTL_SEN_MODE_3       = 2,    ///< Sensor mode 3
	CTL_SEN_MODE_4       = 3,    ///< Sensor mode 4
	CTL_SEN_MODE_5       = 4,    ///< Sensor mode 5
	CTL_SEN_MODE_6       = 5,    ///< Sensor mode 6
	CTL_SEN_MODE_7       = 6,    ///< Sensor mode 7
	CTL_SEN_MODE_8       = 7,    ///< Sensor mode 8
	CTL_SEN_MODE_9       = 8,    ///< Sensor mode 9
	CTL_SEN_MODE_10      = 9,    ///< Sensor mode 10
	CTL_SEN_MODE_11      = 10,   ///< Sensor mode 11
	CTL_SEN_MODE_12      = 11,   ///< Sensor mode 12
	CTL_SEN_MODE_13      = 12,   ///< Sensor mode 13
	CTL_SEN_MODE_14      = 13,   ///< Sensor mode 14
	CTL_SEN_MODE_15      = 14,   ///< Sensor mode 15
	CTL_SEN_MODE_NUM,
	CTL_SEN_MODE_UNKNOWN = 0xCC,    ///< Unknow Mode
	CTL_SEN_MODE_CUR     = 0xFF, ///< current sensor mode
	ENUM_DUMMY4WORD(CTL_SEN_MODE)
} CTL_SEN_MODE, *PCTL_SEN_MODE;

typedef struct {
	UINT32 addr;                      ///< address
	UINT32 data_len;                  ///< data length(bytes)
	UINT32 data[2];                   ///< data idx1(LSB) -> idx2(MSB)
} CTL_SEN_CMD;

/**
    sensor command interface
*/
typedef enum {
	CTL_SEN_CMDIF_TYPE_UNKNOWN = 0,
	CTL_SEN_CMDIF_TYPE_VX1     = 1, ///< Vx1, 510/520/560 N.S.
	CTL_SEN_CMDIF_TYPE_SIF     = 2, ///< serial
	CTL_SEN_CMDIF_TYPE_I2C     = 3, ///< I2C
	CTL_SEN_CMDIF_TYPE_IO      = 4, ///< IO
	CTL_SEN_CMDIF_TYPE_MAX_NUM,
	ENUM_DUMMY4WORD(CTL_SEN_CMDIF_TYPE)
} CTL_SEN_CMDIF_TYPE;

typedef enum {
	CTL_SEN_IF_TYPE_PARALLEL,   ///< parallel
	CTL_SEN_IF_TYPE_LVDS,       ///< LVDS
	CTL_SEN_IF_TYPE_MIPI,       ///< MIPI
	CTL_SEN_IF_TYPE_SLVSEC,     ///< SLVS-EC, only for 680/690. 510/520/560/538/567 N.S.
	CTL_SEN_IF_TYPE_DUMMY,
	CTL_SEN_IF_TYPE_SIEPATGEN,  ///< sie patten gen
	CTL_SEN_IF_TYPE_SPI,        ///< SPI
	CTL_SEN_IF_TYPE_TDIO,       ///< thermal data in/out
	CTL_SEN_IF_TYPE_MAX_NUM,
	ENUM_DUMMY4WORD(CTL_SEN_IF_TYPE)
} CTL_SEN_IF_TYPE;

typedef enum {
	CTL_SEN_DATA_FMT_RGB,
	CTL_SEN_DATA_FMT_RGBIR,
	CTL_SEN_DATA_FMT_RCCB,
	CTL_SEN_DATA_FMT_YUV,
	CTL_SEN_DATA_FMT_Y_ONLY,
	CTL_SEN_DATA_FMT_DVS,
	CTL_SEN_DATA_FMT_EVS,
	CTL_SEN_DATA_FMT_YUV420_LEGACY,
	CTL_SEN_DATA_FMT_THERMAL,

	CTL_SEN_DATA_FMT_MAX_NUM,
	ENUM_DUMMY4WORD(CTL_SEN_DATA_FMT)
} CTL_SEN_DATA_FMT;

typedef enum {
	CTL_SEN_PIXDEPTH_8BIT   = 8,    ///< Pixel depth is   8 bits per pixel.
	CTL_SEN_PIXDEPTH_10BIT  = 10,   ///< Pixel depth is  10 bits per pixel.
	CTL_SEN_PIXDEPTH_12BIT  = 12,   ///< Pixel depth is  12 bits per pixel.
	CTL_SEN_PIXDEPTH_14BIT  = 14,   ///< Pixel depth is  14 bits per pixel.
	CTL_SEN_PIXDEPTH_16BIT  = 16,   ///< Pixel depth is  16 bits per pixel.

	ENUM_DUMMY4WORD(CTL_SEN_PIXDEPTH)
} CTL_SEN_PIXDEPTH;

typedef enum {
	CTL_SEN_CLKLANE_1 = 1,
	CTL_SEN_CLKLANE_2 = 2,

	ENUM_DUMMY4WORD(CTL_SEN_CLKLANE)
} CTL_SEN_CLKLANE;

typedef enum {
	CTL_SEN_DATALANE_1 = 1,
	CTL_SEN_DATALANE_2 = 2,
	CTL_SEN_DATALANE_4 = 4,
	CTL_SEN_DATALANE_6 = 6, // 510/520/560 N.S.
	CTL_SEN_DATALANE_8 = 8, // 510/520/560 N.S.
	CTL_SEN_DATALANE_16 = 16, // 510/520/560/530/538/567 N.S.

	ENUM_DUMMY4WORD(CTL_SEN_DATALANE)
} CTL_SEN_DATALANE;

typedef enum {
	CTL_SEN_DVI_CCIR601 = 0,        ///< HW need VD,HD output pin
	CTL_SEN_DVI_CCIR656_EAV,        ///< recognize VD,HD by syncode. VIE auto calc active size
	CTL_SEN_DVI_CCIR656_ACT,        ///< VIE N.S. active size set by sensor driver.
	CTL_SEN_DVI_CCIR709,            ///< HW need VD,HD output pin
	CTL_SEN_DVI_CCIR601_1120,       ///< HW need VD,HD output pin
	CTL_SEN_DVI_CCIR656_1120_EAV,   ///< recognize VD,HD by syncode. VIE auto calc active size
	CTL_SEN_DVI_CCIR656_1120_ACT,   ///< VIE N.S. active size set by sensor driver.
	CTL_SEN_DVI_CCIR656_1120_EAV_DUAL_HEADER, ///< recognize VD,HD by syncode (with dual-header). VIE auto calc active size
	CTL_SEN_DVI_FMT_MAX_NUM,
	ENUM_DUMMY4WORD(CTL_SEN_DVI_FMT)
} CTL_SEN_DVI_FMT;

typedef enum {
	CTL_SEN_FMT_SEL_CCIR601 = 0,    ///< HW need VD,HD output pin
	CTL_SEN_FMT_SEL_CCIR656,        ///< recognize VD,HD by syncode
	CTL_SEN_FMT_SEL_CCIR709,        ///< HW need VD,HD output pin
	CTL_SEN_FMT_SEL_CCIR1120,
	CTL_SEN_FMT_SEL_MAX_NUM,
	ENUM_DUMMY4WORD(CTL_SEN_CCIR_FMT_SEL)
} CTL_SEN_CCIR_FMT_SEL;

typedef enum {
	CTL_SEN_DVI_DATA_MODE_SD = 0,   ///< 8 bits
	CTL_SEN_DVI_DATA_MODE_HD,       ///< 16 bits, HD inv set in init_obj dvi_msblsb_switch
	CTL_SEN_DVI_DATA_MODE_MAX_NUM,
	ENUM_DUMMY4WORD(CTL_SEN_DVI_DATA_MODE)
} CTL_SEN_DVI_DATA_MODE;

typedef struct {
	CTL_SEN_CCIR_FMT_SEL fmt;    ///< [in] ccir format
	BOOL    interlace;          ///< [in] TRUE:CTL_SEN_MODE_CCIR_INTERLACE, FALSE:CTL_SEN_MODE_CCIR
} CTL_SEN_MODESEL_CCIR;


/*

    sensor mode type - CTL_SEN_MODE_TYPE


   |---------------------------------------------------------------------------------------------------|
   |                                       frame 0 (bit 15.. 0)                                        |
   |---------------------------------------------------------------------------------------------------|
   |   bit 15..12   | bit 11 |      bit 10.. 8       | bit  7 |     bit  6.. 4     |    bit  3.. 0     |
   |---------------------------------------------------------------------------------------------------|
   |  reserve_frm0  |  resv  |  out_combine_sel_frm0 |  resv  | expt_gain_sel_frm0 | out_type_sel_frm0 |
   |---------------------------------------------------------------------------------------------------|

   |---------------------------------------------------------------------------------------------------|
   |                                       frame 1 (bit 31..16)                                        |
   |---------------------------------------------------------------------------------------------------|
   |   bit 31..28   | bit 27 |      bit 26..24       | bit 23 |     bit 22..20     |    bit 19..16     |
   |---------------------------------------------------------------------------------------------------|
   |  reserve_frm1  |  resv  |  out_combine_sel_frm1 |  resv  | expt_gain_sel_frm1 | out_type_sel_frm1 |
   |---------------------------------------------------------------------------------------------------|

   |---------------------------------------------------------------------------------------------------|
   |                                       frame 2 (bit 47..32)                                        |
   |---------------------------------------------------------------------------------------------------|
   |    bit 47..44  | bit 43 |      bit 42..40       | bit 39 |      bit 38..36    |    bit 35..32     |
   |---------------------------------------------------------------------------------------------------|
   |  reserve_frm2  |  resv  |  out_combine_sel_frm2 |  resv  | expt_gain_sel_frm2 | out_type_sel_frm2 |
   |---------------------------------------------------------------------------------------------------|

   |---------------------------------------------------------------------------------------------------|
   |                                       frame 3 (bit 63..48)                                        |
   |---------------------------------------------------------------------------------------------------|
   |    bit 63..60  | bit 59 |      bit 58..56       | bit 55 |      bit 54..52    |    bit 51..48     |
   |---------------------------------------------------------------------------------------------------|
   |  reserve_frm3  |  resv  |  out_combine_sel_frm3 |  resv  | expt_gain_sel_frm3 | out_type_sel_frm3 |
   |---------------------------------------------------------------------------------------------------|

*/

#define CTL_SEN_MODETYPE_FRM_OUTTYPE_OFS        0
#define CTL_SEN_MODETYPE_FRM_OUTTYPE_BITMAP     0x000FULL
#define CTL_SEN_MODETYPE_FRM_EXPTGAIN_OFS       4
#define CTL_SEN_MODETYPE_FRM_EXPTGAIN_BITMAP    0x0070ULL
#define CTL_SEN_MODETYPE_FRM_OUTCOMB_OFS        8
#define CTL_SEN_MODETYPE_FRM_OUTCOMB_BITMAP     0x0700ULL
#define CTL_SEN_MODETYPE_FRM_RESV_OFS           12
#define CTL_SEN_MODETYPE_FRM_RESV_BITMAP        0xF000ULL

#define CTL_SEN_MODETYPE_FRMIDX_OFS         	16
#define CTL_SEN_MODETYPE_FRMIDX_BITMAP      	0xF77FULL
#define CTL_SEN_MODETYPE_FRMIDX0_OFS        	0
#define CTL_SEN_MODETYPE_FRMIDX1_OFS        	16
#define CTL_SEN_MODETYPE_FRMIDX2_OFS        	32
#define CTL_SEN_MODETYPE_FRMIDX3_OFS        	48

/* out_type_sel, use CTL_SEN_ENTITY_MODETYPE_OUTTYPE seperation */
typedef enum {
	out_type_sel_unkn = 0,
	out_type_sel_raw,
	out_type_sel_ccir_progres,
	out_type_sel_ccir_intlace,
	out_type_sel_pdaf,
	out_type_sel_embed = out_type_sel_pdaf,
	out_type_sel_raw_pdaf,
	ENUM_DUMMY4WORD(CTL_SEN_MODETYPE_OUTTYPE_SEL)
} CTL_SEN_MODETYPE_OUTTYPE_SEL;
/* expt_gain_sel, use CTL_SEN_ENTITY_MODETYPE_EXPTGAIN seperation */
typedef enum {
	expt_gain_sel_unkn = 0,
	expt_gain_sel_expt,
	expt_gain_sel_hcg,
	expt_gain_sel_dcg = expt_gain_sel_hcg,
	expt_gain_sel_lcg,
	ENUM_DUMMY4WORD(CTL_SEN_MODETYPE_EXPTGAIN_SEL)
} CTL_SEN_MODETYPE_EXPTGAIN_SEL;
/* out_combine_sel, use CTL_SEN_ENTITY_MODETYPE_OUTCOMB seperation */
typedef enum {
	out_combine_sel_unkn = 0,
	out_combine_sel_linear,
	out_combine_sel_pwl,
	ENUM_DUMMY4WORD(CTL_SEN_MODETYPE_OUTCOMB_SEL)
} CTL_SEN_MODETYPE_OUTCOMB_SEL;

/* modetype, describe 1 frame */
#define CTL_SEN_MODETYPE_FRM(out_type_sel, expt_gain_sel, out_combine_sel, resv) \
	(((out_type_sel 	<< CTL_SEN_MODETYPE_FRM_OUTTYPE_OFS) 	& 	CTL_SEN_MODETYPE_FRM_OUTTYPE_BITMAP) 	| \
	 ((expt_gain_sel 	<< CTL_SEN_MODETYPE_FRM_EXPTGAIN_OFS) 	& 	CTL_SEN_MODETYPE_FRM_EXPTGAIN_BITMAP) 	| \
	 ((out_combine_sel 	<< CTL_SEN_MODETYPE_FRM_OUTCOMB_OFS) 	& 	CTL_SEN_MODETYPE_FRM_OUTCOMB_BITMAP) 	| \
	 ((resv 			<< CTL_SEN_MODETYPE_FRM_RESV_OFS) 		& 	CTL_SEN_MODETYPE_FRM_RESV_BITMAP))

/* get modetype information */
#define CTL_SEN_ENTITY_MODETYPE_EXTRACT(modetype, frmidx)	(modetype >> (frmidx * CTL_SEN_MODETYPE_FRMIDX_OFS))
#define CTL_SEN_ENTITY_MODETYPE_FRM(modetype, frmidx) 		(CTL_SEN_ENTITY_MODETYPE_EXTRACT(modetype, frmidx) & CTL_SEN_MODETYPE_FRMIDX_BITMAP)
#define CTL_SEN_ENTITY_MODETYPE_OUTTYPE(modetype, frmidx)   ((CTL_SEN_ENTITY_MODETYPE_EXTRACT(modetype, frmidx) & CTL_SEN_MODETYPE_FRM_OUTTYPE_BITMAP) 	>> CTL_SEN_MODETYPE_FRM_OUTTYPE_OFS)
#define CTL_SEN_ENTITY_MODETYPE_EXPTGAIN(modetype, frmidx)  ((CTL_SEN_ENTITY_MODETYPE_EXTRACT(modetype, frmidx) & CTL_SEN_MODETYPE_FRM_EXPTGAIN_BITMAP) >> CTL_SEN_MODETYPE_FRM_EXPTGAIN_OFS)
#define CTL_SEN_ENTITY_MODETYPE_OUTCOMB(modetype, frmidx)   ((CTL_SEN_ENTITY_MODETYPE_EXTRACT(modetype, frmidx) & CTL_SEN_MODETYPE_FRM_OUTCOMB_BITMAP) 	>> CTL_SEN_MODETYPE_FRM_OUTCOMB_OFS)
#define CTL_SEN_ENTITY_MODETYPE_RESV(modetype, frmidx)      ((CTL_SEN_ENTITY_MODETYPE_EXTRACT(modetype, frmidx) & CTL_SEN_MODETYPE_FRM_RESV_BITMAP) 	>> CTL_SEN_MODETYPE_FRM_RESV_OFS)

typedef enum {
	CTL_SEN_MODE_TYPE_UNKNOWN 		= CTL_SEN_MODETYPE_FRM(out_type_sel_unkn		, expt_gain_sel_unkn		, out_combine_sel_unkn		, 0),

/* frame_num = 1 */
	/* linear raw */
	CTL_SEN_MODE_LINEAR 			= CTL_SEN_MODETYPE_FRM(out_type_sel_raw			, expt_gain_sel_expt		, out_combine_sel_linear	, 0),
	/* bulid in HDR */
	CTL_SEN_MODE_BUILTIN_HDR 		= CTL_SEN_MODETYPE_FRM(out_type_sel_raw			, expt_gain_sel_expt		, out_combine_sel_pwl		, 0),
	/* ccir progressive */
	CTL_SEN_MODE_CCIR 				= CTL_SEN_MODETYPE_FRM(out_type_sel_ccir_progres, expt_gain_sel_unkn		, out_combine_sel_unkn		, 0),
	/* ccir interlace */
	CTL_SEN_MODE_CCIR_INTERLACE 	= CTL_SEN_MODETYPE_FRM(out_type_sel_ccir_intlace, expt_gain_sel_unkn		, out_combine_sel_unkn		, 0),
	/* pdaf is embedded in the main frame	=>	530: get pdaf from raw plane 3.;	690: extract pdaf from duplicate sie frame.
	CTL_SEN_MODE_RAW_PDAF 			= CTL_SEN_MODETYPE_FRM(out_type_sel_raw_pdaf	, expt_gain_sel_expt		, out_combine_sel_linear	, 0), */
	/* pwl & dcg strategy
	CTL_SEN_MODE_BUILTIN_DCG_HDR 	= CTL_SEN_MODETYPE_FRM(out_type_sel_raw			, expt_gain_sel_dcg			, out_combine_sel_pwl		, 0), */
	/* combine linear raw */
	CTL_SEN_MODE_COMBO_LINEAR 		= CTL_SEN_MODETYPE_FRM(out_type_sel_raw			, expt_gain_sel_expt		, out_combine_sel_linear	, 1),
	/* combine ccir progressive */
	CTL_SEN_MODE_COMBO_CCIR 		= CTL_SEN_MODETYPE_FRM(out_type_sel_ccir_progres, expt_gain_sel_unkn		, out_combine_sel_unkn		, 1),
	/* thermal sensor
	CTL_SEN_MODE_THERMAL 			= CTL_SEN_MODETYPE_FRM(out_type_sel_raw			, expt_gain_sel_expt		, out_combine_sel_linear	, 2), */
	/* preroll linear raw */
	CTL_SEN_MODE_PR_LINEAR 			= CTL_SEN_MODETYPE_FRM(out_type_sel_raw			, expt_gain_sel_expt		, out_combine_sel_linear	, 3),

/* frame_num = 2 */
	/* stagger HDR */
	CTL_SEN_MODE_STAGGER_HDR 		= (CTL_SEN_MODETYPE_FRM(out_type_sel_raw		, expt_gain_sel_expt		, out_combine_sel_linear	, 0) |
									  (CTL_SEN_MODETYPE_FRM(out_type_sel_raw		, expt_gain_sel_expt		, out_combine_sel_linear	, 0) << CTL_SEN_MODETYPE_FRMIDX1_OFS)),
	/* combine stagger HDR */
	CTL_SEN_MODE_COMBO_STAGGER_HDR 	= (CTL_SEN_MODETYPE_FRM(out_type_sel_raw		, expt_gain_sel_expt		, out_combine_sel_linear	, 1) |
									  (CTL_SEN_MODETYPE_FRM(out_type_sel_raw		, expt_gain_sel_expt		, out_combine_sel_linear	, 1) << CTL_SEN_MODETYPE_FRMIDX1_OFS)),
	/* preroll stagger HDR */
	CTL_SEN_MODE_PR_STAGGER_HDR 	= (CTL_SEN_MODETYPE_FRM(out_type_sel_raw		, expt_gain_sel_expt		, out_combine_sel_linear	, 2) |
									  (CTL_SEN_MODETYPE_FRM(out_type_sel_raw		, expt_gain_sel_expt		, out_combine_sel_linear	, 2) << CTL_SEN_MODETYPE_FRMIDX1_OFS)),
	/* BME HDR */
	CTL_SEN_MODE_BME 			 	= (CTL_SEN_MODETYPE_FRM(out_type_sel_raw		, expt_gain_sel_expt		, out_combine_sel_linear	, 3) |
									  (CTL_SEN_MODETYPE_FRM(out_type_sel_raw		, expt_gain_sel_expt		, out_combine_sel_linear	, 3) << CTL_SEN_MODETYPE_FRMIDX1_OFS)),
	/* linear raw + pdaf embed path */
	CTL_SEN_MODE_PDAF 				= (CTL_SEN_MODETYPE_FRM(out_type_sel_raw		, expt_gain_sel_expt		, out_combine_sel_linear	, 0) |
									  (CTL_SEN_MODETYPE_FRM(out_type_sel_pdaf		, expt_gain_sel_unkn		, out_combine_sel_unkn		, 0) << CTL_SEN_MODETYPE_FRMIDX1_OFS)),
	/* 2-frame base HDR: raw + embed path
	CTL_SEN_MODE_2FRAME_BASE_HDR 	= (CTL_SEN_MODETYPE_FRM(out_type_sel_raw		, expt_gain_sel_expt		, out_combine_sel_linear	, 0) |
									  (CTL_SEN_MODETYPE_FRM(out_type_sel_embed		, expt_gain_sel_unkn		, out_combine_sel_unkn		, 0) << CTL_SEN_MODETYPE_FRMIDX1_OFS)), */
	/* 4-frame base HDR: raw + embed path
	CTL_SEN_MODE_4FRAME_BASE_HDR 	= (CTL_SEN_MODETYPE_FRM(out_type_sel_raw		, expt_gain_sel_expt		, out_combine_sel_linear	, 1) |
									  (CTL_SEN_MODETYPE_FRM(out_type_sel_embed		, expt_gain_sel_unkn		, out_combine_sel_unkn		, 1) << CTL_SEN_MODETYPE_FRMIDX1_OFS)), */
	/* path1: pwl, path2: linear
	CTL_SEN_MODE_BUILTIN_DCG_SHDR 	= (CTL_SEN_MODETYPE_FRM(out_type_sel_raw		, expt_gain_sel_dcg			, out_combine_sel_pwl		, 0) | 
									  (CTL_SEN_MODETYPE_FRM(out_type_sel_raw		, expt_gain_sel_expt		, out_combine_sel_linear	, 0) << CTL_SEN_MODETYPE_FRMIDX1_OFS)), */
	/* get pdaf from raw plane 3.
	CTL_SEN_MODE_STAGGER_PDAF 		= (CTL_SEN_MODETYPE_FRM(out_type_sel_raw_pdaf	, expt_gain_sel_expt		, out_combine_sel_linear	, 0) | 
									  (CTL_SEN_MODETYPE_FRM(out_type_sel_raw_pdaf	, expt_gain_sel_expt		, out_combine_sel_linear	, 0) << CTL_SEN_MODETYPE_FRMIDX1_OFS)), */
	/* dcg strategy
	CTL_SEN_MODE_DCG_HDR 			= (CTL_SEN_MODETYPE_FRM(out_type_sel_raw		, expt_gain_sel_dcg			, out_combine_sel_linear	, 0) | 
									  (CTL_SEN_MODETYPE_FRM(out_type_sel_raw		, expt_gain_sel_dcg			, out_combine_sel_linear	, 0) << CTL_SEN_MODETYPE_FRMIDX1_OFS)), */

/* frame_num = 3 */
	/* 3 frame stagger HDR
	CTL_SEN_MODE_STAGGER3_HDR 		= (CTL_SEN_MODETYPE_FRM(out_type_sel_raw		, expt_gain_sel_expt		, out_combine_sel_linear	, 0) | 
									  (CTL_SEN_MODETYPE_FRM(out_type_sel_raw		, expt_gain_sel_expt		, out_combine_sel_linear	, 0) << CTL_SEN_MODETYPE_FRMIDX1_OFS) | 
									  (CTL_SEN_MODETYPE_FRM(out_type_sel_raw		, expt_gain_sel_expt		, out_combine_sel_linear	, 0) << CTL_SEN_MODETYPE_FRMIDX2_OFS)), */
	/* path 1/2 dcg strategy
	CTL_SEN_MODE_DCG_SHDR 			= (CTL_SEN_MODETYPE_FRM(out_type_sel_raw		, expt_gain_sel_dcg			, out_combine_sel_linear	, 0) | 
									  (CTL_SEN_MODETYPE_FRM(out_type_sel_raw		, expt_gain_sel_dcg			, out_combine_sel_linear	, 0) << CTL_SEN_MODETYPE_FRMIDX1_OFS) | 
									  (CTL_SEN_MODETYPE_FRM(out_type_sel_raw		, expt_gain_sel_expt		, out_combine_sel_linear	, 0) << CTL_SEN_MODETYPE_FRMIDX2_OFS)), */

	CTL_SEN_MODE_MAX = 0x8000000000000000,
	ENUM_DUMMY4WORD(CTL_SEN_MODE_TYPE)
} CTL_SEN_MODE_TYPE;

typedef enum {
	CTL_SEN_MSG_TYPE_NONE       = 0x00000000,
	CTL_SEN_MSG_TYPE_OP         = 0x00000001,   ///< dump sensor operation
	CTL_SEN_MSG_TYPE_OP_SIMPLE  = 0x00000002,   ///< dump sensor operation, but skip dump set&get operation after power on, before power off

	ENUM_DUMMY4WORD(CTL_SEN_MSG_TYPE)
} CTL_SEN_MSG_TYPE;

// for CTL_SEN_DBG_SEL_DBG_MSG
#define CTL_SEN_DBG_MSG_PARAM(en, type) (((en & 0x1) << 31) | (type & 0x0FFFFFFF)) ///< en: BOOL, type: CTL_SEN_MSG_TYPE
#define CTL_SEN_DBG_MSG_EN(in) ((in >> 31) & 0x1)
#define CTL_SEN_DBG_MSG_TYPE(in) (in & 0x0FFFFFFF)

typedef enum {
	CTL_SEN_DBG_SEL_DUMP            = 0x00000001,   ///< dump get info (PCTL_SEN_OBJ()->get_cfg), param: dont care
	CTL_SEN_DBG_SEL_DUMP_EXT        = 0x00000002,   ///< dump ext info (PCTL_SEN_OBJ()->init_cfg & PCTL_SEN_OBJ()->get_cfg(CTL_SEN_CFGID_INIT_XX, value)), param: dont care
	CTL_SEN_DBG_SEL_DUMP_DRV        = 0x00000004,   ///< dump sensor driver get info (CTL_SEN_DRV_TAB()->get_cfg(id, CTL_SENDRV_CFGID, value)), param: dont care
	CTL_SEN_DBG_SEL_WAITVD          = 0x00000008,   ///< only support serial sensor (LVDS/CSI/SLVS-EC) & TGE & VX1, param: dont care
	CTL_SEN_DBG_SEL_WAITVD2         = 0x00000010,   ///< only support serial sensor (LVDS/CSI/SLVS-EC) HDR mode, param: dont care
	CTL_SEN_DBG_SEL_WAITFMD         = 0x00000020,   ///< only support serial sensor (LVDS/CSI/SLVS-EC), param: dont care
	CTL_SEN_DBG_SEL_WAITFMD2        = 0x00000040,   ///< only support serial sensor (LVDS/CSI/SLVS-EC) HDR mode, param: dont care
	CTL_SEN_DBG_SEL_DUMP_MAP_TBL    = 0x00000080,   ///< dump (ctl_sen_reg_sendrv/ctl_sen_unreg_sendrv & PCTL_SEN_OBJ()->init_cfg) mapping table, param: dont care
	CTL_SEN_DBG_SEL_DUMP_PROC_TIME  = 0x00000100,   ///< dump ctl sen process time, param: dont care
	CTL_SEN_DBG_SEL_DUMP_CTL        = 0x00000400,   ///< dump ctl_sen inner info
	CTL_SEN_DBG_SEL_WAITVD3         = 0x00000800,   ///< only support serial sensor (LVDS/CSI/SLVS-EC) & VX1 HDR mode, param: dont care
	CTL_SEN_DBG_SEL_WAITVD4         = 0x00001000,   ///< only support serial sensor (LVDS/CSI/SLVS-EC) & VX1 HDR mode, param: dont care
	CTL_SEN_DBG_SEL_WAITFMD3        = 0x00002000,   ///< only support serial sensor (LVDS/CSI/SLVS-EC) HDR mode, param: dont care
	CTL_SEN_DBG_SEL_WAITFMD4        = 0x00004000,   ///< only support serial sensor (LVDS/CSI/SLVS-EC) HDR mode, param: dont care

	CTL_SEN_DBG_SEL_WAIT_VD_TO_SIE  = 0x00040000,   ///< only support serial sensor (LVDS/CSI/SLVS-EC), param: output SIE idx (start from 0)
	CTL_SEN_DBG_SEL_WAIT_FMD_TO_SIE = 0x00080000,   ///< only support serial sensor (LVDS/CSI/SLVS-EC), param: output SIE idx (start from 0)
	CTL_SEN_DBG_SEL_WAIT_VD_TO_VIE  = 0x00100000,   ///< only support CSI sensor , param: output VIE idx (start from 0)
	CTL_SEN_DBG_SEL_WAIT_FMD_TO_VIE = 0x00200000,   ///< only support CSI sensor , param: output VIE idx (start from 0)

	CTL_SEN_DBG_SEL_WAITVD_TGE      = CTL_SEN_DBG_SEL_WAITVD | CTL_SEN_TGE_TAG,  ///< Valid for TGE (CTL_SEN_INIT_CFG_OBJ/cfg_obj.if_cfg.tge.tge_en = ENABLE), param: dont care
	ENUM_DUMMY4WORD(CTL_SEN_DBG_SEL)
} CTL_SEN_DBG_SEL;

/*******************************************************************************/
/*                            for sensor ctrl if                               */
/*******************************************************************************/
typedef enum {

	CTL_SEN_INTE_VD_TO_SIE0     = 0,   ///< Valid for CSI/LVDS
	CTL_SEN_INTE_VD_TO_SIE1     = 1,   ///< Valid for CSI/LVDS
	CTL_SEN_INTE_VD_TO_SIE2     = 2,   ///< Valid for CSI/LVDS
	CTL_SEN_INTE_VD_TO_SIE3     = 3,   ///< Valid for CSI/LVDS
	CTL_SEN_INTE_VD_TO_SIE4     = 4,   ///< Valid for CSI/LVDS
	CTL_SEN_INTE_VD_TO_SIE5     = 5,   ///< Valid for CSI/LVDS

	CTL_SEN_INTE_FMD_TO_SIE0    = 100,   ///< Valid for CSI/LVDS
	CTL_SEN_INTE_FMD_TO_SIE1    = 101,   ///< Valid for CSI/LVDS
	CTL_SEN_INTE_FMD_TO_SIE2    = 102,   ///< Valid for CSI/LVDS
	CTL_SEN_INTE_FMD_TO_SIE3    = 103,   ///< Valid for CSI/LVDS
	CTL_SEN_INTE_FMD_TO_SIE4    = 104,   ///< Valid for CSI/LVDS
	CTL_SEN_INTE_FMD_TO_SIE5    = 105,   ///< Valid for CSI/LVDS

	CTL_SEN_INTE_VD_TO_VIE0_CH0 = 200,   ///< Valid for CSI
	CTL_SEN_INTE_VD_TO_VIE0_CH1 = 201,   ///< Valid for CSI
	CTL_SEN_INTE_VD_TO_VIE0_CH2 = 202,   ///< Valid for CSI
	CTL_SEN_INTE_VD_TO_VIE0_CH3 = 203,   ///< Valid for CSI
	CTL_SEN_INTE_VD_TO_VIE1_CH0 = 204,   ///< Valid for CSI
	CTL_SEN_INTE_VD_TO_VIE1_CH1 = 205,   ///< Valid for CSI
	CTL_SEN_INTE_VD_TO_VIE1_CH2 = 206,   ///< Valid for CSI
	CTL_SEN_INTE_VD_TO_VIE1_CH3 = 207,   ///< Valid for CSI

	CTL_SEN_INTE_FMD_TO_VIE0_CH0 = 300,   ///< Valid for CSI
	CTL_SEN_INTE_FMD_TO_VIE0_CH1 = 301,   ///< Valid for CSI
	CTL_SEN_INTE_FMD_TO_VIE0_CH2 = 302,   ///< Valid for CSI
	CTL_SEN_INTE_FMD_TO_VIE0_CH3 = 303,   ///< Valid for CSI
	CTL_SEN_INTE_FMD_TO_VIE1_CH0 = 304,   ///< Valid for CSI
	CTL_SEN_INTE_FMD_TO_VIE1_CH1 = 305,   ///< Valid for CSI
	CTL_SEN_INTE_FMD_TO_VIE1_CH2 = 306,   ///< Valid for CSI
	CTL_SEN_INTE_FMD_TO_VIE1_CH3 = 307,   ///< Valid for CSI


	CTL_SEN_INTE_SSENIF_MAX,

	CTL_SEN_INTE_TGE_VD         = 10000,    ///< N.S.

	CTL_SEN_INTE_TGE_MAX,

	CTL_SEN_INTE_ABORT      = 0x80000000,   ///< Waiting event timeout abort.


	ENUM_DUMMY4WORD(CTL_SEN_INTE)
} CTL_SEN_INTE;


/*******************************************************************************/
/*                            for dx camera                                    */
/*******************************************************************************/

/**
    Sensor sie mclk information
*/
typedef enum {
	CTL_SEN_SIEMCLK_SRC_DFT,    ///< id 0: PLL_CLK_SIEMCLK, id 1~x: PLL_CLK_SIEMCLK2
	CTL_SEN_SIEMCLK_SRC_MCLK,   ///< KDRV_SSENIF_SIEMCLK_ENABLE
	CTL_SEN_SIEMCLK_SRC_MCLK2,  ///< KDRV_SSENIF_SIEMCLK2_ENABLE
	CTL_SEN_SIEMCLK_SRC_MCLK3,  ///< KDRV_SSENIF_SIEMCLK3_ENABLE
	CTL_SEN_SIEMCLK_SRC_MCLK4,  ///< KDRV_SSENIF_SIEMCLK4_ENABLE
	CTL_SEN_SIEMCLK_SRC_MCLK5,  ///< KDRV_SSENIF_SIEMCLK5_ENABLE (538 N.S.)
	CTL_SEN_SIEMCLK_SRC_MCLK6,  ///< KDRV_SSENIF_SIEMCLK6_ENABLE (538 N.S.)
	CTL_SEN_SIEMCLK_SRC_MCLK7,  ///< KDRV_SSENIF_SIEMCLK7_ENABLE (538 N.S.)
	CTL_SEN_SIEMCLK_SRC_MCLK8,  ///< KDRV_SSENIF_SIEMCLK8_ENABLE (538 N.S.)

	CTL_SEN_SIEMCLK_SRC_SPCLK,  ///< KDRV_SSENIF_SPCLK_ENABLE
	CTL_SEN_SIEMCLK_SRC_SPCLK2, ///< KDRV_SSENIF_SPCLK2_ENABLE

	CTL_SEN_SIEMCLK_SRC_MAX,

	CTL_SEN_SIEMCLK_SRC_IGNORE = CTL_SEN_IGNORE, // extern MCLK source

	ENUM_DUMMY4WORD(CTL_SEN_SIEMCLK_SRC)
} CTL_SEN_SIEMCLK_SRC;

/**
    Sensor sie signal information
*/
typedef enum {
	CTL_SEN_ACTIVE_HIGH = 0,    ///< sensor active high, SIE non-inverse
	CTL_SEN_ACTIVE_LOW = 1,     ///< sensor active low, SIE inverse
	ENUM_DUMMY4WORD(CTL_SEN_ACTIVE_SEL)
} CTL_SEN_ACTIVE_SEL;

typedef enum {
	CTL_SEN_PHASE_RISING,  ///< rising edge latch/trigger
	CTL_SEN_PHASE_FALLING, ///< falling edge latch/trigger
	ENUM_DUMMY4WORD(CTL_SEN_PHASE_SEL)
} CTL_SEN_PHASE_SEL;

/**
    Sensor interface mapping
    CTL_SEN_ID (x) <-> interface (y) <-> CTL_SIE_ID (x) : SENSOR_(x)_MAP_IF_(interface)_(y)
    CTL_SEN_MAP_IF_DEFAULT:
    interface LVDSMIPI:
    SENSOR_1_MAP_IF_LVDSMIPI_1/SENSOR_2_MAP_IF_LVDSMIPI_2/SENSOR_3_MAP_IF_LVDSMIPI_3/SENSOR_4_MAP_IF_LVDSMIPI_4/
    SENSOR_5_MAP_IF_LVDSMIPI_5/SENSOR_6_MAP_IF_LVDSMIPI_6/SENSOR_7_MAP_IF_LVDSMIPI_7/SENSOR_8_MAP_IF_LVDSMIPI_8/
    interface SLVSEC:
    SENSOR_1_MAP_IF_SLVSEC_1

*/

typedef enum {
	CTL_SEN_I2C_W_ADDR_DFT = 0,
	CTL_SEN_I2C_W_ADDR_OPTION1 = 1,
	CTL_SEN_I2C_W_ADDR_OPTION2 = 2,
	CTL_SEN_I2C_W_ADDR_OPTION3 = 3,
	CTL_SEN_I2C_W_ADDR_OPTION4 = 4,
	CTL_SEN_I2C_W_ADDR_OPTION5 = 5,

	CTL_SEN_I2C_W_ADDR_SEL_MAX_NUM,
	ENUM_DUMMY4WORD(CTL_SEN_I2C_W_ADDR_SEL)
} CTL_SEN_I2C_W_ADDR_SEL;

typedef enum {
	CTL_SEN_I2C_CH_1,          ///< I2C channel 1
	CTL_SEN_I2C_CH_2,          ///< I2C channel 2
	CTL_SEN_I2C_CH_3,          ///< I2C channel 3
	CTL_SEN_I2C_CH_4,          ///< I2C channel 4
	CTL_SEN_I2C_CH_5,          ///< I2C channel 5

	CTL_SEN_I2C_CH_MAX_NUM,
	ENUM_DUMMY4WORD(CTL_SEN_I2C_CH)
} CTL_SEN_I2C_CH;

/**
    SIE 1/3 VD/HD signal source (only in parallel mode + slave sensor)
*/
typedef enum {
	CTL_SEN_TGE_SIE1_VD_SRC_CH1 = 0,              // use ch1 to be SIE1 VD source
	CTL_SEN_TGE_SIE1_VD_SRC_CH3,                  // use ch3 to be SIE1 VD source
	CTL_SEN_TGE_SIE3_VD_SRC_CH5,                  // use ch5 to be SIE3 VD source
	CTL_SEN_TGE_SIE3_VD_SRC_CH7,                  // use ch7 to be SIE3 VD source
	ENUM_DUMMY4WORD(CTL_SEN_TGE_SIE_VD_SRC)
} CTL_SEN_TGE_SIE_VD_SRC;

typedef struct {
	BOOL tge_en;    					// set TRUE for used tge, slave sensor (CTL_SEN_SIGNAL_SLAVE) must be enable
	BOOL swap;      					// after 510/520/560/530... N.S.
	CTL_SEN_TGE_SIE_VD_SRC sie_vd_src;  // after 530 N.S.
} CTL_SEN_IF_TGE;

/**
 * @brief ClockLane was not used after 690 sensor
 */
typedef enum {
	CTL_SEN_CLANE_SEL_CSI0_USE_C0,       ///< MIPI CSI0 uses HSI_CK0 as clock lane.
	CTL_SEN_CLANE_SEL_CSI0_USE_C2,       ///< MIPI CSI0 uses HSI_CK2 as clock lane.
	CTL_SEN_CLANE_SEL_CSI1_USE_C4,      ///< MIPI CSI1 uses HSI_CK4 as clock lane.
	CTL_SEN_CLANE_SEL_CSI1_USE_C6,      ///< MIPI CSI1 uses HSI_CK6 as clock lane.

	CTL_SEN_CLANE_SEL_CSI2_USE_C2,      ///< MIPI CSI2 uses HSI_CK2 as clock lane.
	CTL_SEN_CLANE_SEL_CSI3_USE_C6,      ///< MIPI CSI3 uses HSI_CK6 as clock lane.
	CTL_SEN_CLANE_SEL_CSI4_USE_C1,      ///< MIPI CSI4 uses HSI_CK1 as clock lane.
	CTL_SEN_CLANE_SEL_CSI5_USE_C3,      ///< MIPI CSI5 uses HSI_CK3 as clock lane.
	CTL_SEN_CLANE_SEL_CSI6_USE_C5,      ///< MIPI CSI6 uses HSI_CK5 as clock lane.
	CTL_SEN_CLANE_SEL_CSI7_USE_C7,      ///< MIPI CSI7 uses HSI_CK7 as clock lane.

	CTL_SEN_CLANE_SEL_LVDS0_USE_C0C4,    ///< LVDS0 uses HSI_CK0 as clock lane in one clock lane sensor.
	///< LVDS0 uses HSI_CK0+CK4 as clock lanes in two clock lanes sensor.(such as HiSPi 8 lanes)
	CTL_SEN_CLANE_SEL_LVDS0_USE_C2C6,    ///< LVDS0 uses HSI_CK2 as clock lane in one clock lane sensor.
	///< LVDS0 uses HSI_CK2+CK6 as clock lanes in two clock lanes sensor.(such as HiSPi 8 lanes)
	CTL_SEN_CLANE_SEL_LVDS1_USE_C4, ///< LVDS1 uses HSI_CK4 as clock lane.
	CTL_SEN_CLANE_SEL_LVDS1_USE_C6, ///< LVDS1 uses HSI_CK6 as clock lane.

	CTL_SEN_CLANE_SEL_LVDS2_USE_C2, ///< LVDS2 uses HSI_CK2 as clock lane.
	CTL_SEN_CLANE_SEL_LVDS3_USE_C6, ///< LVDS3 uses HSI_CK6 as clock lane.
	CTL_SEN_CLANE_SEL_LVDS4_USE_C1, ///< LVDS4 uses HSI_CK1 as clock lane.
	CTL_SEN_CLANE_SEL_LVDS5_USE_C3, ///< LVDS5 uses HSI_CK3 as clock lane.
	CTL_SEN_CLANE_SEL_LVDS6_USE_C5, ///< LVDS6 uses HSI_CK5 as clock lane.
	CTL_SEN_CLANE_SEL_LVDS7_USE_C7, ///< LVDS7 uses HSI_CK7 as clock lane.

	CTL_SEN_CLANE_SEL_CSI1_USE_C1,      ///< MIPI CSI1 uses HSI_CK1 as clock lane.
	CTL_SEN_CLANE_SEL_LVDS1_USE_C1,     ///< LVDS1 uses HSI_CK1 as clock lane in one clock lane sensor.


	CTL_SEN_CLANE_SEL_CSI1_USE_C0,		///< MIPI CSI1 uses HSI2_CK0 as clock lane.
	CTL_SEN_CLANE_SEL_CSI1_USE_C2,		///< MIPI CSI1 uses HSI2_CK2 as clock lane.
	CTL_SEN_CLANE_SEL_CSI1_USE_C3,		///< MIPI CSI1 uses HSI2_CK3 as clock lane.
	CTL_SEN_CLANE_SEL_CSI2_USE_C1,		///< MIPI CSI2 uses HSI2_CK1 as clock lane.
	CTL_SEN_CLANE_SEL_CSI3_USE_C2,		///< MIPI CSI3 uses HSI2_CK2 as clock lane.
	CTL_SEN_CLANE_SEL_CSI3_USE_C3,		///< MIPI CSI3 uses HSI2_CK3 as clock lane.
	CTL_SEN_CLANE_SEL_CSI4_USE_C3,		///< MIPI CSI4 uses HSI2_CK3 as clock lane.
	CTL_SEN_CLANE_SEL_LVDS0_USE_C0,		///< LVDS0 uses HSI1_CK0 as clock lane.
	CTL_SEN_CLANE_SEL_LVDS1_USE_C0,		///< LVDS1 uses HSI2_CK0 as clock lane.
	CTL_SEN_CLANE_SEL_LVDS1_USE_C2,		///< LVDS1 uses HSI2_CK2 as clock lane.
	CTL_SEN_CLANE_SEL_LVDS1_USE_C3,		///< LVDS1 uses HSI2_CK3 as clock lane.
	CTL_SEN_CLANE_SEL_LVDS2_USE_C1,		///< LVDS2 uses HSI2_CK1 as clock lane.
	CTL_SEN_CLANE_SEL_LVDS3_USE_C2,		///< LVDS3 uses HSI2_CK2 as clock lane.
	CTL_SEN_CLANE_SEL_LVDS3_USE_C3,		///< LVDS3 uses HSI2_CK3 as clock lane.
	CTL_SEN_CLANE_SEL_LVDS4_USE_C3,		///< LVDS4 uses HSI2_CK3 as clock lane.

} CTL_SEN_CLANE_SEL;


/*
    KDRV controller id(CSI/LVDS/TGE/SLVSEC) or ch(TGE) information
*/
#define CTL_SEN_DRVDEV_OFS_CSI      0
#define CTL_SEN_DRVDEV_OFS_LVDS     4
#define CTL_SEN_DRVDEV_OFS_TGE      8
#define CTL_SEN_DRVDEV_OFS_SLVSEC   12

#define CTL_SEN_DRVDEV_MASK_CSI(drvdev)         (drvdev & (0x0000000F << CTL_SEN_DRVDEV_OFS_CSI))
#define CTL_SEN_DRVDEV_MASK_LVDS(drvdev)        (drvdev & (0x0000000F << CTL_SEN_DRVDEV_OFS_LVDS))
#define CTL_SEN_DRVDEV_MASK_TGE(drvdev)         (drvdev & (0x0000000F << CTL_SEN_DRVDEV_OFS_TGE))
#define CTL_SEN_DRVDEV_MASK_SLVSEC(drvdev)      (drvdev & (0x0000000F << CTL_SEN_DRVDEV_OFS_SLVSEC))

// KDRV DEV IDX, start from 0
#define CTL_SEN_DRVDEV_IDX_CSI(drvdev)      ((CTL_SEN_DRVDEV_MASK_CSI(drvdev) - CTL_SEN_DRVDEV_CSI_BASE) >> CTL_SEN_DRVDEV_OFS_CSI) // KDRV_SSENIF_ENGINE_CSI0 ~ KDRV_SSENIF_ENGINE_CSIX
#define CTL_SEN_DRVDEV_IDX_LVDS(drvdev)     ((CTL_SEN_DRVDEV_MASK_LVDS(drvdev) - CTL_SEN_DRVDEV_LVDS_BASE) >> CTL_SEN_DRVDEV_OFS_LVDS) // KDRV_SSENIF_ENGINE_LVDS0 ~ KDRV_SSENIF_ENGINE_LVDSX
#define CTL_SEN_DRVDEV_IDX_TGE(drvdev)      ((CTL_SEN_DRVDEV_MASK_TGE(drvdev) - CTL_SEN_DRVDEV_TGE_BASE) >> CTL_SEN_DRVDEV_OFS_TGE) // KDRV_VDOCAP_TGE_ENGINE0/KDRV_TGE_VDHD_CH1 ~ KDRV_VDOCAP_TGE_ENGINE0/KDRV_TGE_VDHD_CH8
#define CTL_SEN_DRVDEV_IDX_SLVSEC(drvdev)   ((CTL_SEN_DRVDEV_MASK_SLVSEC(drvdev) - CTL_SEN_DRVDEV_SLVSEC_BASE) >> CTL_SEN_DRVDEV_OFS_SLVSEC) // KDRV_SSENIF_ENGINE_SLVSEC0 ~ KDRV_SSENIF_ENGINE_SLVSECX

typedef enum {
	CTL_SEN_DRVDEV_CSI_BASE    = 0x00000001,
	CTL_SEN_DRVDEV_CSI_0       = CTL_SEN_DRVDEV_CSI_BASE,   // select CSI controller 0
	CTL_SEN_DRVDEV_CSI_1       = 0x00000002,                // select CSI controller 1
	CTL_SEN_DRVDEV_CSI_2       = 0x00000003,                // select CSI controller 2
	CTL_SEN_DRVDEV_CSI_3       = 0x00000004,                // select CSI controller 3
	CTL_SEN_DRVDEV_CSI_4       = 0x00000005,                // select CSI controller 4
	CTL_SEN_DRVDEV_CSI_5       = 0x00000006,                // select CSI controller 5
	CTL_SEN_DRVDEV_CSI_6       = 0x00000007,                // select CSI controller 6
	CTL_SEN_DRVDEV_CSI_7       = 0x00000008,                // select CSI controller 7
	CTL_SEN_DRVDEV_CSI_MAX     = CTL_SEN_DRVDEV_CSI_7,      // must <= 0x0000000F

	CTL_SEN_DRVDEV_LVDS_BASE   = 0x00000010,
	CTL_SEN_DRVDEV_LVDS_0      = CTL_SEN_DRVDEV_LVDS_BASE,  // select LVDS controller 0
	CTL_SEN_DRVDEV_LVDS_1      = 0x00000020,                // select LVDS controller 1
	CTL_SEN_DRVDEV_LVDS_2      = 0x00000030,                // select LVDS controller 2
	CTL_SEN_DRVDEV_LVDS_3      = 0x00000040,                // select LVDS controller 3
	CTL_SEN_DRVDEV_LVDS_4      = 0x00000050,                // select LVDS controller 4
	CTL_SEN_DRVDEV_LVDS_5      = 0x00000060,                // select LVDS controller 5
	CTL_SEN_DRVDEV_LVDS_6      = 0x00000070,                // select LVDS controller 6
	CTL_SEN_DRVDEV_LVDS_7      = 0x00000080,                // select LVDS controller 7
	CTL_SEN_DRVDEV_LVDS_MAX    = CTL_SEN_DRVDEV_LVDS_7,     // must <= 0x000000F0

	CTL_SEN_DRVDEV_TGE_BASE    = 0x00000100,
	CTL_SEN_DRVDEV_TGE_0       = CTL_SEN_DRVDEV_TGE_BASE,   // select TGE VD0/HD0 generator
	CTL_SEN_DRVDEV_TGE_1       = 0x00000200,                // select TGE VD1/HD1 generator
	CTL_SEN_DRVDEV_TGE_2       = 0x00000300,                // select TGE VD2/HD2 generator
	CTL_SEN_DRVDEV_TGE_3       = 0x00000400,                // select TGE VD3/HD3 generator
	CTL_SEN_DRVDEV_TGE_4       = 0x00000500,                // select TGE VD4/HD4 generator
	CTL_SEN_DRVDEV_TGE_5       = 0x00000600,                // select TGE VD5/HD5 generator
	CTL_SEN_DRVDEV_TGE_6       = 0x00000700,                // select TGE VD6/HD6 generator
	CTL_SEN_DRVDEV_TGE_7       = 0x00000800,                // select TGE VD7/HD7 generator
	CTL_SEN_DRVDEV_TGE_MAX     = CTL_SEN_DRVDEV_TGE_7,      // must <= 0x00000F00

	CTL_SEN_DRVDEV_SLVSEC_BASE = 0x00001000,
	CTL_SEN_DRVDEV_SLVSEC_0    = CTL_SEN_DRVDEV_SLVSEC_BASE,// select SLVSEC controller 0
	CTL_SEN_DRVDEV_SLVSEC_1    = 0x00002000,                // select SLVSEC controller 1
	CTL_SEN_DRVDEV_SLVSEC_2    = 0x00003000,                // select SLVSEC controller 2
	CTL_SEN_DRVDEV_SLVSEC_3    = 0x00004000,                // select SLVSEC controller 3
	CTL_SEN_DRVDEV_SLVSEC_4    = 0x00005000,                // select SLVSEC controller 4
	CTL_SEN_DRVDEV_SLVSEC_5    = 0x00006000,                // select SLVSEC controller 5
	CTL_SEN_DRVDEV_SLVSEC_6    = 0x00007000,                // select SLVSEC controller 6
	CTL_SEN_DRVDEV_SLVSEC_7    = 0x00008000,                // select SLVSEC controller 7
	CTL_SEN_DRVDEV_SLVSEC_MAX  = CTL_SEN_DRVDEV_SLVSEC_7,   // must <= 0x0000F000

	CTL_SEN_DRVDEV_UNKNOWN     = 0xF0000000,                // reserved for default value

} CTL_SEN_DRVDEV;


/*

* sample A :

VD ..................................................
                ^                           ^
                |act_main y                 |
                v                           |
     -----------------------------------    |act_ext y
    |           ^                       |   |
    |           |act_main h (1)         |   |
    |           v                       |   v
     -----------------------------------
    |           ^                       |
    |           |act_ext h (1)          |
    |           v                       |
     -----------------------------------
    |           ^                       |
    |           |act_main h (2)         |
    |           v                       |
     -----------------------------------
    |           ^                       |
    |           |act_ext h (2)          |
    |           v                       |
     -----------------------------------
                .
                .
     -----------------------------------
    |           ^                       |
    |           |act_main h (loop_times)|
    |           v                       |
     -----------------------------------
    |           ^                       |
    |           |act_ext h (loop_times) |
    |           v                       |
     -----------------------------------


* sample B :

VD ..................................................
                ^                           ^
                |act_ext y                  |
                v                           |
     -----------------------------------    |act_main y
    |           ^                       |   |
    |           |act_ext h (1)          |   |
    |           v                       |   v
     -----------------------------------
    |           ^                       |
    |           |act_main h (1)         |
    |           v                       |
     -----------------------------------
    |           ^                       |
    |           |act_ext h (2)          |
    |           v                       |
     -----------------------------------
    |           ^                       |
    |           |act_main h (2)         |
    |           v                       |
     -----------------------------------
                .
                .
     -----------------------------------
    |           ^                       |
    |           |act_ext h (loop_times) |
    |           v                       |
     -----------------------------------
    |           ^                       |
    |           |act_main h (loop_times)|
    |           v                       |
     -----------------------------------

*/

typedef struct {
	CTL_SEN_MODE mode;
	URECT act_main;
	URECT act_ext;
	UINT32 loop_times;
} CTL_SEN_EXTRACT_DATA_PARAM;


/*
    external information
*/
#define senext_item_rev         1   // val_in type : N.S , val_out type : N.S.

typedef struct _CTL_SEN_EXT_INFO {
	UINT32 item;        // [in]  item (senext_item_xxxxx, ex : senext_item_rev)
	ULONG  val_in;      // [in]  value set to sendrv
	ULONG  val_out;     // [out] value get from sendrv
	struct _CTL_SEN_EXT_INFO *pnext;
} CTL_SEN_EXT_INFO;

typedef struct {
	CTL_SEN_MODE         mode;     // [in]
	CTL_SEN_EXT_INFO    *info;     // [in]
} CTL_SEN_EXT_PARAM;

/*******************************************************************************/
/*                            for sensor driver                                */
/*******************************************************************************/

typedef struct {
	CTL_SEN_MODE mode;                  ///< sensor mode
	UINT32 frame_rate;                  ///< sensor frame rate, fps * 100
} CTL_SENDRV_CHGMODE_OBJ;

/**
    sensor support property
*/
typedef enum {
	CTL_SEN_SUPPORT_PROPERTY_NONE     = 0x00000000,
	CTL_SEN_SUPPORT_PROPERTY_MIRROR   = 0x00000001,
	CTL_SEN_SUPPORT_PROPERTY_FLIP     = 0x00000002,
	CTL_SEN_SUPPORT_PROPERTY_CHGFPS   = 0x00000004,

	CTL_SEN_SUPPORT_PROPERTY_ALL      = 0x00000007,

	ENUM_DUMMY4WORD(CTL_SEN_SUPPORT_PROPERTY)
} CTL_SEN_SUPPORT_PROPERTY;

typedef enum {
	CTL_SEN_VENDOR_SONY,                                ///< SONY
	CTL_SEN_VENDOR_OMNIVISION,  ///< Omni-Vision
	CTL_SEN_VENDOR_ONSEMI,      ///< ON-Semi
	CTL_SEN_VENDOR_PANASONIC,   ///< Panasonic
	CTL_SEN_VENDOR_OTHERS,      ///< Sensor vendor not specified above.

	ENUM_DUMMY4WORD(CTL_SEN_VENDOR)
} CTL_SEN_VENDOR;

/**
    sensor signal type
*/
typedef enum {
	CTL_SEN_SIGNAL_MASTER = 0,       ///< sensor output HD and VD signal
	CTL_SEN_SIGNAL_SLAVE,            ///< dsp output HD and VD signal
	CTL_SEN_SIGNAL_MAX_NUM,
	ENUM_DUMMY4WORD(CTL_SEN_SIGNAL_TYPE)
} CTL_SEN_SIGNAL_TYPE;

typedef enum {
	CTL_SEN_PAD_SEL_A,
	CTL_SEN_PAD_SEL_B,
	CTL_SEN_PAD_SEL_AB,
	ENUM_DUMMY4WORD(CTL_SEN_PAD_SEL)
} CTL_SEN_PAD_SEL;

typedef enum {
	CTL_SEN_SMP_SINGLE_EDGE = 0,   // clock sampling with single edge of pxclk
	CTL_SEN_SMP_DUAL_EDGE,         // clock sampling with dual edge of pxclk
	ENUM_DUMMY4WORD(CTL_SEN_SMP_EDGE)
} CTL_SEN_SMP_EDGE;

typedef struct {
	CTL_SEN_ACTIVE_SEL vd_inv;
	CTL_SEN_ACTIVE_SEL hd_inv;
	CTL_SEN_PHASE_SEL vd_phase;
	CTL_SEN_PHASE_SEL hd_phase;
	CTL_SEN_PHASE_SEL data_phase;
} CTL_SEN_SIGNAL_INFO;

#define CTL_SEN_NAME_LEN 32
typedef struct {
	CHAR name[CTL_SEN_NAME_LEN];        ///< [out]
	CTL_SEN_VENDOR vendor;              ///< [out]
	UINT32 max_senmode;                 ///< [out]
	CTL_SEN_SUPPORT_PROPERTY property;  ///< [out] sensor support property
	UINT32 sync_timing;                 ///< [out] sync timing for Exposure time & gain(VD)
} CTL_SENDRV_GET_ATTR_BASIC_PARAM;

typedef struct {
	CTL_SEN_SIGNAL_TYPE type; // [out]
	CTL_SEN_SIGNAL_INFO info; // [out], only for parallel sensor and salve sensor (tge)
} CTL_SENDRV_GET_ATTR_SIGNAL_PARAM;

typedef struct {
	CTL_SEN_I2C_W_ADDR_SEL w_addr_sel; // Backward compatible
	UINT8 w_addr;
} CTL_SEN_I2C_WADDR;

typedef struct {
	CTL_SEN_I2C_WADDR w_addr_info[CTL_SEN_I2C_W_ADDR_SEL_MAX_NUM]; // Backward compatible
	CTL_SEN_I2C_WADDR cur_w_addr_info;
	CTL_SEN_I2C_CH ch;                 ///< I2C channel
	UINT32  s_clk;                   ///< I2C clock, set 0 for default
} CTL_SENDRV_I2C;

typedef struct {
	SIF_CH      channel;     ///< sif channel
	UINT32      bus_clk;     ///< unit:HZ, map to SIF_CONFIG_ID_BUSCLOCK
	UINT32      sen_d_s;
	UINT32      sen_h;      ///< map to sif hold time,ref:SIF_CONFIG_ID_SENH
} CTL_SENDRV_SIF;

typedef struct {
	UINT32 tx241_clane_speed;       ///< Sensor clock lane speed in bps, only tx_type == KDRV_SSENIFVX1_TXTYPE_THCV241 need to set
	UINT32 tx241_input_clk_freq;    ///< Assign 241 input oscillator frequency. Input value in Hertz(valid range is 10000000~40000000), only tx_type == KDRV_SSENIFVX1_TXTYPE_THCV241 need to set
	UINT32 tx241_real_mclk;         ///< Sensor real operation MCLK frequency provided , only tx_type == KDRV_SSENIFVX1_TXTYPE_THCV241 need to set
} CTL_SENDRV_VX1;

typedef struct {
	CTL_SEN_CMDIF_TYPE type;    ///< [out]
	union {
		CTL_SENDRV_I2C i2c;     ///< [out]
		CTL_SENDRV_SIF sif;     ///< [out]
	} info;
	CTL_SENDRV_VX1 vx1;         ///< [out], 510/520/560 N.S.
} CTL_SENDRV_GET_ATTR_CMDIF_PARAM;

#if 1  // not support, backward compatible
typedef struct {
} CTL_SEN_LVDS_CTRLPTN;
typedef struct {
	UINT32  sensor_pin_order;
	CTL_SEN_LVDS_CTRLPTN ctrl_ptn;
} CTL_SEN_LVDS_CTRLPTN_INFO;
typedef struct {
	UINT32 num;
	UINT32 code[7];
} CTL_SEN_LVDS_SYNC_CODE_PTN;
typedef struct {
	CTL_SEN_LVDS_CTRLPTN *(*fp_get_ctrl_ptn)(UINT32 idx, CTL_SEN_PIXDEPTH pixel_depth, CTL_SEN_MODE_TYPE mode_type);
} CTL_SENDRV_LVDS;
typedef struct {
	CTL_SEN_IF_TYPE type;
	union {
		CTL_SENDRV_LVDS lvds;
	} info;
} CTL_SENDRV_GET_ATTR_IF_PARAM;// not support, backward compatible
#endif

typedef struct {
	UINT32 mux_data_num;    ///< mux data number
} CTL_SEN_PARA_MUX_INFO;

#define ctl_sen_modesel_vendor_param_num 5
typedef struct {
	UINT32 frame_rate;                      ///< [in] fps * 100
	USIZE size;                             ///< [in]
	CTL_SEN_IF_TYPE if_type;                ///< [in]
	CTL_SEN_DATA_FMT data_fmt;              ///< [in] sensor output data format
	UINT32 frame_num;                       ///< [in]
	CTL_SEN_PIXDEPTH pixdepth;              ///< [in] sensor output pixel depth, set CTL_SEN_IGNORE for auto (sensor driver need to cover CTL_SEN_IGNORE)
	CTL_SEN_MODESEL_CCIR ccir;              ///< [in] only CTL_SEN_DATA_FMT_YUV, CTL_SEN_DATA_FMT_Y_ONLY need to set
	BOOL mux_singnal_en;                    ///< [in] only support CTL_SEN_IF_TYPE_PARALLEL
	CTL_SEN_PARA_MUX_INFO mux_signal_info;  ///< [in] only (mux_singnal_en == 1) need to set
	CTL_SEN_MODE_TYPE mode_type_sel;        ///< [in] if user input 0 or CTL_SEN_IGNORE (sendrv must ignore this parameters)
	CTL_SEN_DATALANE data_lane;             ///< [in] if user input 0 or CTL_SEN_IGNORE (sendrv must ignore this parameters)
	UINT32 vendor_param[ctl_sen_modesel_vendor_param_num]; ///< [in] please refer ctl_sen_ext.h
	CTL_SEN_SMP_EDGE smp_edge;				///< [in] sensor sample edge. only ccir sensor spt(sensor output to vie)
	CTL_SEN_MODE mode;                      ///< [out]
} CTL_SENDRV_GET_MODESEL_PARAM;

typedef struct {
	UINT32 frame_rate;                      ///< [in] fps * 100
	USIZE size;                             ///< [in]
	CTL_SEN_IF_TYPE if_type;                ///< [in]
	CTL_SEN_DATA_FMT data_fmt;              ///< [in] sensor output data format
	CTL_SEN_PIXDEPTH pixdepth;              ///< [in] sensor output pixel depth, set CTL_SEN_IGNORE for auto (sensor driver need to cover CTL_SEN_IGNORE)
	CTL_SEN_MODESEL_CCIR ccir;              ///< [in] only CTL_SEN_DATA_FMT_YUV, CTL_SEN_DATA_FMT_Y_ONLY need to set
	BOOL mux_singnal_en;                    ///< [in] only support CTL_SEN_IF_TYPE_PARALLEL
	CTL_SEN_PARA_MUX_INFO mux_signal_info;  ///< [in] only (mux_singnal_en == 1) need to set
	CTL_SEN_MODE_TYPE mode_type_sel;        ///< [in] if user input 0 or CTL_SEN_IGNORE (sendrv must ignore this parameters)
	CTL_SEN_DATALANE data_lane;             ///< [in] if user input 0 or CTL_SEN_IGNORE (sendrv must ignore this parameters)
	UINT32 vendor_param[ctl_sen_modesel_vendor_param_num]; ///< [in] please refer ctl_sen_ext.h
} CTL_SENDRV_MODESEL2_INFO;

typedef struct {
	UINT32                      frame_num;  ///< [in]
	CTL_SENDRV_MODESEL2_INFO    *info;      ///< [in] array number : frame_num
	CTL_SEN_MODE                mode;       ///< [out]
} CTL_SENDRV_GET_MODESEL2_PARAM;

typedef struct {
	UINT32 temp; // [out]
} CTL_SENDRV_GET_TEMP_PARAM;

typedef struct {
	UINT32 chg_fps;     ///< [out]chgmode frame rate (fps * 100)
	UINT32 cur_fps;     ///< [out]current frame rate (fps * 100)
} CTL_SENDRV_GET_FPS_PARAM;

/**
 * @brief 
 * - sendrv get `CTL_SENDRV_CFGID_GET_SPEED` param
 * 
 * - Sendrv code include default settings. User could set customize value through `ext_devices/sensor/configs`
 * 
 * - linux : `configs/cfg`; rtos : `configs/dtsi`
 * 
 */
typedef struct {
	CTL_SEN_MODE mode;  ///< [in] if input CTL_SEN_MODE_PWR, pls output power on information
	CTL_SEN_SIEMCLK_SRC mclk_src;   ///< [out] cannot change by sensor mode
	UINT32 mclk;        ///< [out] mclk frequency
	UINT32 pclk;        ///< [out] pclk frequency
	UINT32 data_rate;   ///< [out] data rate
} CTL_SENDRV_GET_SPEED_PARAM;

/**
    Sensor data start pixel or pattern
*/
typedef enum {
	// RGB start pixel
	CTL_SEN_STPIX_R = 0,            ///< start pixel R
	CTL_SEN_STPIX_GR = 1,           ///< start pixel GR
	CTL_SEN_STPIX_GB = 2,           ///< start pixel GB
	CTL_SEN_STPIX_B = 3,            ///< start pixel B

	// RGBIR data format
	CTL_SEN_STPIX_RGBIR_RGBG_GIGI = 100,  ///< start pixel RGBG/GIrGIr
	CTL_SEN_STPIX_RGBIR_GBGR_IGIG = 101,  ///< start pixel GBGR/IrGIrG
	CTL_SEN_STPIX_RGBIR_GIGI_BGRG = 102,  ///< start pixel GIrGIr/GBRG
	CTL_SEN_STPIX_RGBIR_IGIG_GRGB = 103,  ///< start pixel IrGIrG/GRGB
	CTL_SEN_STPIX_RGBIR_BGRG_GIGI = 104,  ///< start pixel BGRG/GIrGIr
	CTL_SEN_STPIX_RGBIR_GRGB_IGIG = 105,  ///< start pixel GRGB/IrGIrG
	CTL_SEN_STPIX_RGBIR_GIGI_RGBG = 106,  ///< start pixel GIrGIr/RGBG
	CTL_SEN_STPIX_RGBIR_IGIG_GBGR = 107,  ///< start pixel IrGIrG/GBGR

	// RCCB start pixel
	CTL_SEN_STPIX_RCCB_RC = 200,    ///< start pixel RC
	CTL_SEN_STPIX_RCCB_CR = 201,    ///< start pixel CR
	CTL_SEN_STPIX_RCCB_CB = 202,    ///< start pixel CB
	CTL_SEN_STPIX_RCCB_BC = 203,    ///< start pixel BC

	// Y only
	CTL_SEN_STPIX_Y_ONLY = 301,     ///< y pixel only

	// YUV start pixel
	CTL_SEN_STPIX_YUV_YUYV = 400,    ///< start pixel YUYV
	CTL_SEN_STPIX_YUV_YVYU = 401,    ///< start pixel YVYU
	CTL_SEN_STPIX_YUV_UYVY = 402,    ///< start pixel UYVY
	CTL_SEN_STPIX_YUV_VYUY = 403,    ///< start pixel VYUY

	// YUV420 Legacy
	CTL_SEN_STPIX_YUV420_LEGACY_YYU_YYV = 500,
	CTL_SEN_STPIX_YUV420_LEGACY_YUY_YVY = 501,
	CTL_SEN_STPIX_YUV420_LEGACY_UYY_VYY = 502,

	// None
	CTL_SEN_STPIX_NONE = 0xFFFF,

	CTL_SEN_STPIX_MAX_NUM,
	ENUM_DUMMY4WORD(CTL_SEN_STPIX)
} CTL_SEN_STPIX;

/**
    Sensor data format
*/
typedef enum {
	CTL_SEN_FMT_POGRESSIVE = 0,      ///< pogressive mode
	CTL_SEN_FMT_MAX_NUM,
	ENUM_DUMMY4WORD(CTL_SEN_FMT)
} CTL_SEN_FMT;


/**
    Sensor signal

    @note relative falling edge
*/
typedef struct {
	UINT32 hd_sync;     ///< hd sync
	UINT32 hd_period;   ///< hd period
	UINT32 vd_sync;     ///< vd sync
	UINT32 vd_period;   ///< vd period
} CTL_SEN_MODE_SIGNAL;


/**
    Sensor GAIN structure
*/
typedef struct {
	UINT32 min; ///< min gain (1X = 1 x 1000 = 1000)
	UINT32 max; ///< max gain (1X = 1 x 1000 = 1000)
} CTL_SEN_MODE_GAIN;

/**
    multi-frame information
*/
#define SEN_BIT_OFS_NONE (0)
#define SEN_BIT_OFS_0 (1<<0)
#define SEN_BIT_OFS_1 (1<<1)
#define SEN_BIT_OFS_2 (1<<2)
#define SEN_BIT_OFS_3 (1<<3)
#define SEN_BIT_OFS_4 (1<<4)
#define SEN_BIT_OFS_5 (1<<5)
#define SEN_BIT_OFS_6 (1<<6)
#define SEN_BIT_OFS_7 (1<<7)
#define SEN_BIT_OFS_8 (1<<8)
#define SEN_BIT_OFS_9 (1<<9)
#define SEN_BIT_OFS_10 (1<<10)
#define SEN_BIT_OFS_11 (1<<11)
#define SEN_BIT_OFS_12 (1<<12)
#define SEN_BIT_OFS_13 (1<<13)
#define SEN_BIT_OFS_14 (1<<14)
#define SEN_BIT_OFS_15 (1<<15)

typedef struct {
	CTL_SEN_MODE mode;              ///< [in]
	CTL_SEN_IF_TYPE if_type;        ///< [out]
	CTL_SEN_DATA_FMT data_fmt;      ///< [out]
	CTL_SEN_MODE_TYPE mode_type;    ///< [out]
	UINT32 dft_fps;                 ///< [out]sensor driver default (max) support frame rate (fps * 100)
	UINT32 frame_num;               ///< [out]
	CTL_SEN_STPIX stpix;            ///< [out]
	CTL_SEN_PIXDEPTH pixel_depth;   ///< [out]
	CTL_SEN_FMT fmt;                ///< [out]Sensor data type
	USIZE valid_size;                           ///< [out]the number of sample data, for serial interface
	URECT act_size[CTL_SEN_MFRAME_MAX_NUM];     ///< [out]the number of sample data, for sie act, must <= valid_size, [1]~[N] not valid in CTL_SEN_CFGID_GET_MFR_VER=1
	USIZE crp_size;                             ///< [out]the number of sample data, for sie crp (image data), must <= act_size
	CTL_SEN_MODE_SIGNAL signal_info;///< [out]
	UINT32 ratio_h_v;               ///< [out]sensor hv ratio (H:bit[31:16], V:bit[15:0])
	CTL_SEN_MODE_GAIN gain;         ///< [out]sensor gain X 1000
	UINT32 bining_ratio;            ///< [out]binning ratio X 100
} CTL_SENDRV_GET_MODE_BASIC_PARAM;

typedef enum {
	CTL_SEN_DATAIN_BIT_ORDER_LSB,  ///< Data Input Bit order is Least Significant Bit First.
	CTL_SEN_DATAIN_BIT_ORDER_MSB,  ///< Data Input Bit order is Most Significant Bit First.

	ENUM_DUMMY4WORD(CTL_SEN_DATAIN_BIT_ORDER)
} CTL_SEN_DATAIN_BIT_ORDER;

typedef enum {
	CTL_SEN_LVDS_VSYNC_GEN_MATCH_VD,        ///< LVDS/HiSPi Vsync Pulse Generate is compare the VD Sync code matched.
	///< This option is suited for the PANASONIC/Aptina LVDS sensor.
	CTL_SEN_LVDS_VSYNC_GEN_VD2HD,           ///< LVDS/HiSPi Vsync Pulse Generate is detecting SYNC_VD to SYNC_HD transition.
	///< This option is suited for the SONY/SHARP/Samsung LVDS sensor.
	CTL_SEN_LVDS_VSYNC_GEN_HD2VD,           ///< LVDS/HiSPi Vsync Pulse Generate is detecting SYNC_HD to SYNC_VD transition.
	///< This option is reserved for future usage.
	CTL_SEN_LVDS_VSYNC_GEN_MATCH_VD_BY_FSET,///< LVDS/HiSPi Vsync Pulse Generate is depending on the FSET bit changed in SYNC Code.
	///< After FSET bit changed, the first detected VD_SYNC Code would generate Vsync to SIE1.
	///< The first detected VD2_SYNC Code would generate Vsync to SIE2.
	///< This mode is used in the HDR mode for SONY Sensor.

	ENUM_DUMMY4WORD(CTL_SEN_LVDS_VSYNC_GEN)
} CTL_SEN_LVDS_VSYNC_GEN;

typedef struct {
	CTL_SEN_MODE mode;                                      ///< [in]
	CTL_SEN_CLKLANE clk_lane;                               ///< [out] backward compatible, kdrv cover
	CTL_SEN_DATALANE data_lane;                             ///< [out]
	UINT32 output_pixel_order[CTL_SEN_LVDS_MAX_DATALANE];   ///< [out] output data pixel order, need to be filled by CTL_SEN_IGNORE
	UINT32 sel_frm_id[CTL_SEN_MFRAME_MAX_NUM];              ///< [out] HDR only
	UINT16 sel_bit_ofs;                                     ///< [out] N.S. backward compatible
	UINT32 fset_bit;                                        ///< [out] N.S. backward compatible
	CTL_SEN_DATAIN_BIT_ORDER data_in_order;                 ///< [out] N.S. backward compatible
	UINT32 rev;                                             ///< N.S. backward compatible
} CTL_SENDRV_GET_MODE_LVDS_PARAM;


#define CTL_SEN_MIPI_PIXEL_DATA 0xffff
#define CTL_SEN_MIPI_MAX_MANUAL 3
typedef enum {
	CTL_SEN_MIPI_MANUAL_NONE = 0,       ///< mipi auto.

	CTL_SEN_MIPI_MANUAL_8BIT = 8,       ///< mipi manual Pixel depth is  8 bits per pixel.
	CTL_SEN_MIPI_MANUAL_10BIT = 10,     ///< mipi manual Pixel depth is 10 bits per pixel.
	CTL_SEN_MIPI_MANUAL_12BIT = 12,     ///< mipi manual Pixel depth is 12 bits per pixel.
	CTL_SEN_MIPI_MANUAL_14BIT = 14,     ///< mipi manual Pixel depth is 14 bits per pixel.

	CTL_SEN_MIPI_MANUAL_YUV422 = 0x100, ///< mipi manual Pixel depth is YUV422.

	ENUM_DUMMY4WORD(CTL_SEN_MIPI_MANUAL_BIT)
} CTL_SEN_MIPI_MANUAL_BIT;

typedef struct {
	CTL_SEN_MIPI_MANUAL_BIT bit;    ///< manaul depack format (CSI_CONFIG_ID_MANUAL_FORMAT/..)
	UINT32 data_id;                 ///< receiving data id, range: 0x00~0xFF (CSI_CONFIG_ID_MANUAL_DATA_ID/..) or CTL_SEN_MIPI_PIXEL_DATA
} CTL_SEN_MIPI_MANUAL_INFO;

typedef struct {
	CTL_SEN_MODE mode;                                              ///< [in]
	CTL_SEN_CLKLANE clk_lane;                                       ///< [out] backward compatible, kdrv cover
	CTL_SEN_DATALANE data_lane;                                     ///< [out]
	CTL_SEN_MIPI_MANUAL_INFO manual_info[CTL_SEN_MIPI_MAX_MANUAL];  ///< [out]
	BOOL save_pwr;                                                  ///< [out]
	UINT32 sel_frm_id[CTL_SEN_MFRAME_MAX_NUM];                      ///< [out] multi-frame (frame_num > 1) only, ex : HDR
	///<       if (CTL_SEN_MIPI_MFR_OUTSEL == CTL_SEN_MIPI_MFR_OUTSEL_DT) : select data_type
	///<       else : select virtual_id
	UINT16 sel_bit_ofs;                                             ///< [out] backward compatible, kdrv cover
} CTL_SENDRV_GET_MODE_MIPI_PARAM;

typedef struct {
	CTL_SEN_MODE mode;              ///< [in]
	CTL_SEN_PARA_MUX_INFO mux_info; ///< [out], mux information
} CTL_SENDRV_GET_MODE_PARA_PARAM;

/**
    SLVSEC Operation Speed Selection

    The SLVSEC controller can only operate at two speeds: 4.608/2.304/1.152 Gbps.
    This is used at .set_slvsec_config(DAL_SSENIFSLVSEC_CFGID_SPEED) to
    tell the SLVSEC controller the sensor's output speed.
*/
typedef enum {
	CTL_SEN_SLVSEC_SPEED_4608,  ///< 4.608 Gbps
	CTL_SEN_SLVSEC_SPEED_2304,  ///< 2.304 Gbps
	CTL_SEN_SLVSEC_SPEED_1152,  ///< 1.152 Gbps

	ENUM_DUMMY4WORD(CTL_SEN_SLVSEC_SPEED)
} CTL_SEN_SLVSEC_SPEED;

typedef struct {
	CTL_SEN_MODE mode;                                              ///< [in]
	CTL_SEN_DATALANE data_lane;                                     ///< [out]
	CTL_SEN_SLVSEC_SPEED speed;                                     ///< [out]
	UINT32 output_pixel_order[CTL_SEN_SLVSEC_MAX_DATALANE];         ///< [out] output data pixel order, need to be filled by CTL_SEN_IGNORE
	UINT32 sel_frm_id[CTL_SEN_MFRAME_SLVSEC_MAX_NUM];               ///< [out] HDR only
} CTL_SENDRV_GET_MODE_SLVSEC_PARAM;

/**
    sensor dvi information
*/
typedef struct {
	CTL_SEN_MODE mode;                  ///< [in]
	CTL_SEN_DVI_FMT fmt;                ///< [out], sensor output fmt
	CTL_SEN_DVI_DATA_MODE data_mode;    ///< [out], sensor output data mode
} CTL_SENDRV_GET_MODE_DVI_PARAM;


/**
    sensor tge information
*/
typedef struct {
	CTL_SEN_MODE mode;             ///< [in]
	CTL_SEN_MODE_SIGNAL signal;    ///< [out], sensor need tge signal info
	UINT32 vd_frontblnk;		   ///< [out], sensor need additional tge signal: vd frontblank
} CTL_SENDRV_GET_MODE_TGE_PARAM;

typedef struct {
	INT32 probe_rst;               ///< [out] probe result, return 0 for check probe sensor OK
} CTL_SENDRV_GET_PROBE_SEN_PARAM;

#define CTL_SEN_PLUG_PARAM_NUM 5
typedef struct {
	USIZE size;         ///< [out], current plug size
	UINT32 fps;         ///< [out], current plug fps
	BOOL    interlace;          ///< [out] TRUE:CTL_SEN_MODE_CCIR_INTERLACE, FALSE:CTL_SEN_MODE_CCIR
	UINT32 param[CTL_SEN_PLUG_PARAM_NUM];    ///< [out], current plug info
} CTL_SENDRV_GET_PLUG_INFO_PARAM;

typedef struct {
	CTL_SEN_MODE mode;                  ///< [in]
	UINT32 clane_ctl_mode;              ///< [out] 1: force high speed , 0: normal.
} CTL_SENDRV_GET_MODE_MIPI_CLANE_CMETHOD;

typedef INT32(*CTL_SEN_CSI_EN_CB)(CTL_SEN_ID chip_id, BOOL en);
typedef struct {
	CTL_SEN_CSI_EN_CB csi_en_cb;    ///< [in] csi enable callback
} CTL_SENDRV_MIPI_EN_USER;

#define CTL_SENDRV_GET_MODE_MIPI_EN_USER CTL_SENDRV_MIPI_EN_USER

typedef enum {
	CTL_SEN_MANUAL_IADJ_SEL_OFF,
	CTL_SEN_MANUAL_IADJ_SEL_IADJ,
	CTL_SEN_MANUAL_IADJ_SEL_DATARATE,

	ENUM_DUMMY4WORD(CTL_SEN_MANUAL_IADJ_SEL)
} CTL_SEN_MANUAL_IADJ_SEL;

typedef struct {
	CTL_SEN_MODE mode;  ///< [in]
	UINT32 row_time;    ///< [out] spend time (us) x10 per row, must be a fixed value.
	UINT32 row_time_step;  ///< [out] for AE, based on row_time
} CTL_SENDRV_GET_MODE_ROWTIME_PARAM;

typedef struct {
	CTL_SEN_MODE mode;              ///< [in]
	UINT32 frame_rate;              ///< [in] fps * 100
	UINT32 num;                     ///< [in] diff_row num
	UINT32 *diff_row;               ///< [out] unit:row, frame1 & frame2 diff / frame2 & frame3 diff / frame3 & frame4 diff / ...
	UINT32 *diff_row_vd;            ///< [out] unit:row, v-sync diff frame1 & frame2 diff / frame2 & frame3 diff / frame3 & frame4 diff / ...
} CTL_SENDRV_GET_MFR_OUTPUT_TIMING_PARAM;

typedef struct {
	CTL_SEN_MODE mode;              ///< [in]
	CTL_SEN_MANUAL_IADJ_SEL sel;    ///< [out]
	union {
		UINT32 iadj;    ///< for CTL_SEN_MANUAL_IADJ_SEL_IADJ
		///< rx comparator spare current control signal
		///< (0: i_source=20uA, 1: i_source=40uA, 2: i_source=60uA, 3: i_source=80uA)
		UINT32 data_rate;   ///< for CTL_SEN_MANUAL_IADJ_SEL_DATARATE
		///< mipi data rate, unit: Mbps, kdrv will calculate the corresponding IADJ value
	} val; ///< [out]
} CTL_SENDRV_GET_MODE_MANUAL_IADJ;

#define CTL_SEN_HSDATAOUT_BASE_CLK CTL_SEN_60M_HZ
typedef struct {
	CTL_SEN_MODE mode;              ///< [in]
	UINT32 delay;                   ///< [out] high speed data output delay, base on CTL_SEN_60M_HZ
} CTL_SENDRV_GET_MODE_MIPI_HSDATAOUT_DLY;

typedef enum {
	CTL_SEN_FLIP_NONE   = 0x00000000,
	CTL_SEN_FLIP_H      = 0x00000001,
	CTL_SEN_FLIP_V      = 0x00000002,
	CTL_SEN_FLIP_H_V    = 0x00000003,

	ENUM_DUMMY4WORD(CTL_SEN_FLIP)
} CTL_SEN_FLIP;

typedef enum {
	CTL_SEN_STATUS_STANDBY = 1,
	CTL_SEN_STATUS_DMA_ABORT = 2,

	ENUM_DUMMY4WORD(CTL_SEN_STATUS)
} CTL_SEN_STATUS;

typedef struct {
	UINT32 chip_id;     ///< [in][out] chip_id(chip idx) for current sensor id
	UINT32 vin_id;      ///< [in][out] vin_id(port) for current sensor id (select ad input source)

	UINT32 reserved;
} CTL_SENDRV_AD_ID_MAP_PARAM;

typedef struct {
	UINT32 vin_id;      ///< [in] vin_id(ch) of image parameter (ad input source)
	UINT32 val;         ///< [in][out] set value / return value of image parameter

	UINT32 reserved;
} CTL_SENDRV_AD_IMAGE_PARAM;

typedef struct {
	UINT32 vc_id;		///< [in][out] vc_id (MIPI virtual channel) for cur sen_id
} CTL_SENDRV_MUX_VC_ID_PARAM;

typedef struct {
	CTL_SEN_MODE mode;              ///< [in]
	USIZE size;                     ///< [out] dvs sensor dram output size, unit : pixel
} CTL_SENDRV_DVS_INFO_PARAM;

typedef enum {
	CTL_SEN_VD_DLY_NONE,
	CTL_SEN_VD_DLY_AUTO,

	ENUM_DUMMY4WORD(CTL_SEN_VD_DLY)
} CTL_SEN_VD_DLY;

typedef struct {
	CTL_SEN_MODE mode;              ///< [in]
	UINT32 num;                     ///< [in] diff_dly num
	CTL_SEN_VD_DLY *dly;            ///< [out] vd delay frame1 / frame2 / frame3 / ...
} CTL_SENDRV_VD_DLY_PARAM;

typedef enum {
	CTL_SEN_MIPI_MFR_OUTSEL_SONYLI, // sony li
	CTL_SEN_MIPI_MFR_OUTSEL_VC,     // virtual channel
	CTL_SEN_MIPI_MFR_OUTSEL_DT,     // data type

	ENUM_DUMMY4WORD(CTL_SEN_MIPI_MFR_OUTSEL)
} CTL_SEN_MIPI_MFR_OUTSEL;

typedef struct {
	CTL_SEN_MODE mode;              ///< [in]
	CTL_SEN_MIPI_MFR_OUTSEL outsel; ///< [out] mipi multi-frame output select
} CTL_SENDRV_MIPI_MFR_OUTSEL_PARAM;

typedef struct {
	CTL_SEN_CCIR_FMT_SEL fmt;		///< [in] for 530 VIE set default CHx_FMT
	CTL_SEN_IF_TYPE if_type;		///< [in] for 530 VIE set default CHx_FMT
	CTL_SEN_SMP_EDGE smp_edge;		///< [in] for drv init
} CTL_SENDRV_OPEN_CFG_PARAM;

typedef struct {
	CTL_SEN_MODE mode;												///< [in]
	UINT32 combo_num;												///< [out] the number of combine frame
	USIZE valid_size[CTL_SEN_COMB_MAX_NUM];							///< [out] the number of sample data, for serial interface
	URECT act_size[CTL_SEN_COMB_MAX_NUM][CTL_SEN_MFRAME_MAX_NUM];	///< [out] the number of sample data, for sie act, must <= valid_size
	USIZE crp_size[CTL_SEN_COMB_MAX_NUM];							///< [out] the number of sample data (with overlap width), for sie crp (image data), must <= act_size
	UINT32 ovlp_width[CTL_SEN_COMB_MAX_NUM];						///< [out] the number of overlap width (right overlap width)
} CTL_SENDRV_GET_MODE_COMBO_PARAM;

typedef enum {
	CTL_SEN_THERMAL_TX_CFG_MODE_SD0 = 0,			///< output port 1-bit, SD0 => TX_CONFIG_DATA bit0~511
	CTL_SEN_THERMAL_TX_CFG_MODE_SD0_1,				///< output port 2-bit, SD0 => TX_CONFIG_DATA bit0~255, SD1 => TX_CONFIG_DATA bit256~511
	CTL_SEN_THERMAL_TX_CFG_MODE_SD0_2,				///< output port 3-bit, SD0 => TX_CONFIG_DATA bit0~127, SD1 => TX_CONFIG_DATA bit128~255, SD2 => TX_CONFIG_DATA bit256~383
	CTL_SEN_THERMAL_TX_CFG_MODE_SD0_3,				///< output port 7-bit, SD0 => TX_CONFIG_DATA bit0~127, SD1 => TX_CONFIG_DATA bit128~255, SD2 => TX_CONFIG_DATA bit256~383, SD3 => TX_CONFIG_DATA bit384~511
	CTL_SEN_THERMAL_TX_CFG_MODE_SD0_5_M1,			///< output port 6-bit, SD0 => TX_CONFIG_DATA bit0~95,  SD1 => TX_CONFIG_DATA bit96~191,  SD2 => TX_CONFIG_DATA bit192~287, SD3 => TX_CONFIG_DATA bit288~383, SD4 => TX_CONFIG_DATA bit384~479, SD5 => TX_CONFIG_DATA bit480~575, 538 not supported, 539A supported
    CTL_SEN_THERMAL_TX_CFG_MODE_SD0_M2,				///< output port 1-bit, SD0 => TX_CONFIG_DATA bit0~639, 538 not supported, 539A supported
    CTL_SEN_THERMAL_TX_CFG_MODE_SD0_5_M2,			///< output port 6-bit, SD0 => TX_CONFIG_DATA bit0~319, SD1 => TX_CONFIG_DATA bit320~383, SD2 => TX_CONFIG_DATA bit384~447, SD3 => TX_CONFIG_DATA bit448~511, SD4 => TX_CONFIG_DATA bit512~575, SD5 => TX_CONFIG_DATA bit576~639, 538 not supported, 539A supported
	CTL_SEN_THERMAL_TX_CFG_MODE_MAX,
} CTL_SEN_THERMAL_TX_CFG_MODE;

typedef enum {
	CTL_SEN_THERMAL_TX_LEVEL_LOW = 0,				///< unused data output level low
	CTL_SEN_THERMAL_TX_LEVEL_HIGH,					///< unused data output level high
	CTL_SEN_THERMAL_TX_LEVEL_MAX,
} CTL_SEN_THERMAL_TX_LEVEL_SEL;

typedef enum {
	CTL_SEN_THERMAL_TX_SYNC_CODE_MODE_SD0 = 0,		///< output port 1-bit
	CTL_SEN_THERMAL_TX_SYNC_CODE_MODE_SD0_1,		///< output port 2-bit
	CTL_SEN_THERMAL_TX_SYNC_CODE_MODE_SD0_2,		///< output port 3-bit
	CTL_SEN_THERMAL_TX_SYNC_CODE_MODE_SD0_6,		///< output port 7-bit
	CTL_SEN_THERMAL_TX_SYNC_CODE_MODE_MAX,
} CTL_SEN_THERMAL_TX_SYNC_CODE_MODE;

typedef enum {
    CTL_SEN_THERMAL_TX_FS_IDLE_MODE_HILO_END = 0,          ///< frame sync signal go to idle when tx high/low clock cycle end
    CTL_SEN_THERMAL_TX_FS_IDLE_MODE_DATA_END,              ///< frame sync signal go to idle when tx config/stream data end
    CTL_SEN_THERMAL_TX_FS_IDLE_MODE_NS,
	CTL_SEN_THERMAL_TX_FS_IDLE_MODE_MAX = CTL_SEN_THERMAL_TX_FS_IDLE_MODE_NS,
    ENUM_DUMMY4WORD(CTL_SEN_THERMAL_TX_FS_IDLE_MODE_SEL)
} CTL_SEN_THERMAL_TX_FS_IDLE_MODE_SEL;                     ///< 538 not supported, 539A supported

typedef enum {
    CTL_SEN_THERMAL_TX_FS_INV_NONE = 0,                    ///< frame sync signal invert disable
    CTL_SEN_THERMAL_TX_FS_INV_OUT,                         ///< frame sync signal invert output(high/low/delay/idle)
    CTL_SEN_THERMAL_TX_FS_INV_HILO,                        ///< frame sync signal invert high/low only, delay/idle keep low
    CTL_SEN_THERMAL_TX_FS_INV_HILO_OUT,                    ///< frame sync signal invert high/low + output
    CTL_SEN_THERMAL_TX_FS_INV_NS,
	CTL_SEN_THERMAL_TX_FS_INV_MAX = CTL_SEN_THERMAL_TX_FS_INV_NS,
    ENUM_DUMMY4WORD(CTL_SEN_THERMAL_TX_FS_INV_SEL)
} CTL_SEN_THERMAL_TX_FS_INV_SEL;                           ///< 538 not supported, 539A supported

typedef struct {
    UINT32  tx_dly_cnt;                             ///< clock count of delay for frame sync signal,      from 0 ~ 0xffff
    UINT32  tx_high_cnt;                            ///< clock count of level high for frame sync signal, from 1 ~ 0xffffffff
    UINT32  tx_low_cnt;                             ///< clock count of level low  for frame sync signal, from 1 ~ 0xffffffff
}  CTL_SEN_THERMAL_TX_FS_CLKCNT;                    ///< 538 not supported, 539A supported, SD6 as FS output pin when enable tsen_tx_fs

typedef enum {
	CTL_SEN_THERMAL_TX_OOC_MODE_SD0 = 0,			///< output port 1-bit
	CTL_SEN_THERMAL_TX_OOC_MODE_SD0_1_2PXL_M1,		///< output port 2-bit, 2-pixel mode-1, SD0=>Pixel(N,0), SD1=>Pixel(N,1)
	CTL_SEN_THERMAL_TX_OOC_MODE_SD0_1_2PXL_M2,		///< output port 2-bit, 2-pixel mode-2, SD0=>Pixel(N,1), SD1=>Pixel(N,0)
	CTL_SEN_THERMAL_TX_OOC_MODE_SD0_1,				///< output port 2-bit, 1-pixel mode
	CTL_SEN_THERMAL_TX_OOC_MODE_SD0_2,				///< output port 3-bit
	CTL_SEN_THERMAL_TX_OOC_MODE_SD0_6,				///< output port 7-bit
	CTL_SEN_THERMAL_TX_OOC_MODE_MAX,
} CTL_SEN_THERMAL_TX_OOC_MODE_SEL;

typedef enum {
	CTL_SEN_THERMAL_TX_OOC_BITDEPTH_6 = 0,
	CTL_SEN_THERMAL_TX_OOC_BITDEPTH_7,
	CTL_SEN_THERMAL_TX_OOC_BITDEPTH_8,
	CTL_SEN_THERMAL_TX_OOC_BITDEPTH_MAX,
} CTL_SEN_THERMAL_TX_OOC_BITDEPTH_SEL;

typedef enum {
	CTL_SEN_THERMAL_RX_DECODE_MODE_DVP_8BIT = 0,	///< PCLK + VD + HD + D0~7
	CTL_SEN_THERMAL_RX_DECODE_MODE_DVP_14BIT,		///< PCLK + VD + HD + D0~13
	CTL_SEN_THERMAL_RX_DECODE_MODE_SYNC_CODE,		///< base on sync code setting
	CTL_SEN_THERMAL_RX_DECODE_MODE_TX_FS_LS,		///< base on OOC TX FS/LS loopback signal with FS/LS_Delay setting
	CTL_SEN_THERMAL_RX_DECODE_MODE_MAX,
} CTL_SEN_THERMAL_RX_DECODE_MODE;

typedef enum {
	CTL_SEN_THERMAL_RX_OUT_FMT_MSB = 0,				///< output data for raw 14bit from buffer[15:0] MSB => [15:2]
	CTL_SEN_THERMAL_RX_OUT_FMT_LSB,					///< output data for raw 14bit from buffer[15:0] LSB => [13:0]
	CTL_SEN_THERMAL_RX_OUT_FMT_MAX,
} CTL_SEN_THERMAL_RX_OUT_FMT;

#define CTL_SEN_THERMAL_CFG_NUM 20
#define CTL_SEN_THERMAL_LVL_NUM 7
#define CTL_SEN_THERMAL_FSCODE_NUM 8
#define CTL_SEN_THERMAL_LSCODE_NUM 8
#define CTL_SEN_THERMAL_BNK_NUM 3
typedef struct {
	CTL_SEN_MODE mode;

	CTL_SEN_THERMAL_TX_CFG_MODE tx_mode;		///< config data output port mode
	UINT16 tx_len;								///< from 4 ~ 512 clock cycle
	UINT32 tx_data[CTL_SEN_THERMAL_CFG_NUM];	///< config data
	CTL_SEN_THERMAL_TX_LEVEL_SEL tx_lvl[CTL_SEN_THERMAL_LVL_NUM];	///< SD#0~6 output level
	UINT32 tx_period;       					///< config data output period, clock cycle, 539A supported only, from 0 ~ 31 => means 1~32 clock cycle output 1 data
    UINT32 tx_blanking[2];  					///< blanking clock cycle for tx config,     539A supported only, | blanking[0] | tx_config | blanking[1] |, from 0 ~ 0xffff

	CTL_SEN_THERMAL_TX_SYNC_CODE_MODE sync_mode;///< sync_code data output port mode
	UINT16 fs_code_len;							///< FS code length, 4 ~ 32 clock cycle
	UINT16 ls_code_len;							///< LS code length, 4 ~ 32 clock cycle
	UINT32 fs_code[CTL_SEN_THERMAL_FSCODE_NUM];	///< FS code data
	UINT32 ls_code[CTL_SEN_THERMAL_LSCODE_NUM];	///< LS code data

	UINT16 fs_delay;	///< frame start delay cycle when rx_mode=CTL_SEN_THERMAL_RX_DECODE_MODE_TX_FS_LS
	UINT16 ls_delay;	///< line  start delay cycle when rx_mode=CTL_SEN_THERMAL_RX_DECODE_MODE_TX_FS_LS

	BOOL tx_combine_en;											//538 not supported, 539A supported

	/* FS output signal config*/
	BOOL tx_fs_en;												//538 not supported, 539A supported
	CTL_SEN_THERMAL_TX_FS_IDLE_MODE_SEL	fs_idle_mode_sel;		//538 not supported, 539A supported
	CTL_SEN_THERMAL_TX_FS_INV_SEL			fs_inv_sel;			//538 not supported, 539A supported
	CTL_SEN_THERMAL_TX_FS_CLKCNT			fs_clk_cnt;			//538 not supported, 539A supported

	CTL_SEN_THERMAL_TX_OOC_MODE_SEL ooc_mode;			///< OOC data output port mode
	CTL_SEN_THERMAL_TX_OOC_BITDEPTH_SEL ooc_bitdepth;	///< support on CTL_SEN_THERMAL_TX_OOC_MODE_SD0_1_2PXL_M1 and CTL_SEN_THERMAL_TX_OOC_MODE_SD0_1_2PXL_M2
	BOOL ooc_data_swap;									///< data swap, none(MSB->LSB), do-swap(LSB->MSB)
	UINT8 ooc_data_r_shift;								///< data right-shift 0 ~ 7
	UINT8 ooc_err_det_code;								///< OOC illegal code detection
	UINT8 ooc_err_rep_code;								///< OOC replace code for illegal data
	UINT16 ooc_width;									///< OOC frame width
	UINT16 ooc_height;									///< OOC frame height
	UINT8 ooc_dummy_top_cnt;							///< OOC frame dummy top    line count
	UINT8 ooc_dummy_bot_cnt;							///< OOC frame dummy bottom line count
	UINT8 ooc_dummy_val;
	UINT16 ooc_vblanking[CTL_SEN_THERMAL_BNK_NUM];		///< OOC frame vertical   blanking0,1,2, unit: clock cycle
	UINT16 ooc_hblanking[CTL_SEN_THERMAL_BNK_NUM];		///< OOC frame horizontal blanking0,1,2, unit: clock cycle

	CTL_SEN_THERMAL_RX_DECODE_MODE rx_mode;		///< receive frame data mode
	CTL_SEN_THERMAL_RX_OUT_FMT out_fmt;			///< receive frame data output format
	BOOL hi_byte_inv;							///< [15:8][7:0] => [8:15][7:0]
	BOOL lo_byte_inv;							///< [15:8][7:0] => [15:8][0:7]
	UINT8 hi_byte_r_shift;						///< [15:8][7:0] => 2bit shift, [15:10][0][0][7:0]
	UINT8 lo_byte_r_shift;						///< [15:8][7:0] => 2bit shift, [15 :8][7:2][0][0]
	BOOL two_byte_swap;							///< [15:8][7:0] => [7:0][15:8]
	BOOL two_byte_inv;							///< [15:0]      => [0:15]
	UINT8 two_byte_r_shift;						///< [15:0]      => 3bit shift, [15:3][0][0][0]
} CTL_SENDRV_GET_THERMAL_PARAM, CTL_SEN_GET_THERMAL_PARAM;

typedef INT32(*CTL_SEN_LVDS_EN_CB)(CTL_SEN_ID chip_id, BOOL en);
typedef struct {
	CTL_SEN_LVDS_EN_CB lvds_en_cb;    ///< [in] lvds enable callback
} CTL_SENDRV_LVDS_EN_USER;

typedef INT32(*CTL_SEN_SLVSEC_EN_CB)(CTL_SEN_ID chip_id, BOOL en);
typedef struct {
	CTL_SEN_SLVSEC_EN_CB slvsec_en_cb;    ///< [in] slvsec enable callback
} CTL_SENDRV_SLVSEC_EN_USER;

typedef INT32(*CTL_SEN_TGE_EN_CB)(CTL_SEN_ID chip_id, BOOL en);
typedef struct {
	CTL_SEN_TGE_EN_CB tge_en_cb;    ///< [in] tge enable callback
} CTL_SENDRV_TGE_EN_USER;

typedef enum {
	/* set only*/
	CTL_SENDRV_CFGID_SET_BASE = 0x00000000,
	CTL_SENDRV_CFGID_SET_EXPT,          ///< void (user define), sensor exposure setting
	CTL_SENDRV_CFGID_SET_GAIN,          ///< void (user define), sensor gain setting
	CTL_SENDRV_CFGID_SET_FPS,           ///< UINT32: fps * 100
	CTL_SENDRV_CFGID_SET_FPS_BASE,		///< CTL_SEN_SET_FPS_BASE_PARAM
	CTL_SENDRV_CFGID_SET_STATUS,        ///< CTL_SEN_STATUS
	CTL_SENDRV_CFGID_SET_OPEN_CFG,      ///< CTL_SENDRV_OPEN_CFG_PARAM

	/* get only */
	CTL_SENDRV_CFGID_GET_BASE = 0x01000000,
	CTL_SENDRV_CFGID_GET_EXPT,          ///< void (user define),
	CTL_SENDRV_CFGID_GET_GAIN,          ///< void (user define),
	CTL_SENDRV_CFGID_GET_ATTR_BASIC,    ///< CTL_SENDRV_GET_ATTR_BASIC_PARAM,
	CTL_SENDRV_CFGID_GET_ATTR_SIGNAL,   ///< CTL_SENDRV_GET_ATTR_SIGNAL_PARAM
	CTL_SENDRV_CFGID_GET_ATTR_CMDIF,    ///< CTL_SENDRV_GET_ATTR_CMDIF_PARAM,
	CTL_SENDRV_CFGID_GET_ATTR_IF,       ///< not support, backward compatible
	CTL_SENDRV_CFGID_GET_TEMP,          ///< CTL_SENDRV_GET_TEMP_PARAM,
	CTL_SENDRV_CFGID_GET_FPS,           ///< CTL_SENDRV_GET_FPS_PARAM,
	CTL_SENDRV_CFGID_GET_FPS_BASE,		///< CTL_SEN_GET_FPS_BASE_PARAM
	CTL_SENDRV_CFGID_GET_SPEED,         ///< CTL_SENDRV_GET_SPEED_PARAM,
	CTL_SENDRV_CFGID_GET_MODESEL,       ///< CTL_SENDRV_GET_MODESEL_PARAM,
	CTL_SENDRV_CFGID_GET_MODE_BASIC,    ///< CTL_SENDRV_GET_MODE_BASIC_PARAM,
	CTL_SENDRV_CFGID_GET_MODE_LVDS,     ///< CTL_SENDRV_GET_MODE_LVDS_PARAM,
	CTL_SENDRV_CFGID_GET_MODE_MIPI,     ///< CTL_SENDRV_GET_MODE_MIPI_PARAM,
	CTL_SENDRV_CFGID_GET_MODE_PARA,     ///< CTL_SENDRV_GET_MODE_PARA_PARAM,
	CTL_SENDRV_CFGID_GET_MODE_SLVSEC,   ///< CTL_SENDRV_GET_MODE_SLVSEC_PARAM, only for 680/690. 510/520/560 N.S.
	CTL_SENDRV_CFGID_GET_MODE_DVI,      ///< CTL_SENDRV_GET_MODE_DVI_PARAM,
	CTL_SENDRV_CFGID_GET_MODE_TGE,      ///< CTL_SENDRV_GET_MODE_TGE_PARAM,
	CTL_SENDRV_CFGID_GET_PLUG_INFO,     ///< CTL_SENDRV_GET_PLUG_INFO_PARAM
	CTL_SENDRV_CFGID_GET_PROBE_SEN,     ///< CTL_SENDRV_GET_PROBE_SEN_PARAM, need to get after sensor power on
	CTL_SENDRV_CFGID_GET_SENDRV_VER,    ///< UINT32, sensor driver version
	CTL_SENDRV_CFGID_GET_MODE_MIPI_CLANE_CMETHOD,   ///< CTL_SENDRV_GET_MODE_MIPI_CLANE_CMETHOD, clock lane control method
	CTL_SENDRV_CFGID_GET_MODE_MIPI_EN_USER, // backward compatible, CTL_SENDRV_GET_MODE_MIPI_EN_USER
	CTL_SENDRV_CFGID_MIPI_EN_USER = CTL_SENDRV_CFGID_GET_MODE_MIPI_EN_USER,          ///< set only, CTL_SENDRV_MIPI_EN_USER, if return OK, csi need enable by sendrv
	CTL_SENDRV_CFGID_GET_MODE_MIPI_MANUAL_IADJ,     ///< CTL_SENDRV_GET_MODE_MANUAL_IADJ, rx comparator spare current control signal
	CTL_SENDRV_CFGID_GET_MODE_ROWTIME,              ///< CTL_SENDRV_GET_MODE_ROWTIME_PARAM
	CTL_SENDRV_CFGID_GET_MFR_OUTPUT_TIMING,         ///< CTL_SENDRV_GET_MFR_OUTPUT_TIMING_PARAM, sensor output frame timing
	CTL_SENDRV_CFGID_GET_MODE_MIPI_HSDATAOUT_DLY,   ///< CTL_SENDRV_GET_MODE_MIPI_HSDATAOUT_DLY, sensor high speed data output delay
	CTL_SENDRV_CFGID_GET_DVS_INFO,                  ///< CTL_SENDRV_DVS_INFO_PARAM dvs sensor information
	CTL_SENDRV_CFGID_GET_VD_DLY,                    ///< CTL_SENDRV_VD_DLY_PARAM, multi-frame sensor mode only
	CTL_SENDRV_CFGID_GET_MFR_VER,                   ///< UINT32, multi-frame version, 0: CTL_SEN_MODE not support FRMIDX, 1 : CTL_SEN_MODE support FRMIDX
	CTL_SENDRV_CFGID_GET_MODESEL2,                  ///< CTL_SENDRV_GET_MODESEL2_PARAM,
	CTL_SENDRV_CFGID_GET_MODE_MIPI_MFR_OUTSEL,      ///< CTL_SENDRV_MIPI_MFR_OUTSEL_PARAM
	CTL_SENDRV_CFGID_GET_MODE_EXTRACT_DATA,      	///< CTL_SEN_EXTRACT_DATA_PARAM
	CTL_SENDRV_CFGID_GET_MODE_COMBO,                ///< CTL_SENDRV_GET_MODE_COMBO_PARAM, sensor combination setting
	CTL_SENDRV_CFGID_LVDS_EN_USER,					///< set only, CTL_SENDRV_LVDS_EN_USER, if return OK, lvds need enable by sendrv
	CTL_SENDRV_CFGID_SLVSEC_EN_USER,				///< set only, CTL_SENDRV_SLVSEC_EN_USER, if return OK, slvesc need enable by sendrv
	CTL_SENDRV_CFGID_TGE_EN_USER,					///< set only, CTL_SENDRV_TGE_EN_USER, if return OK, tge need enable by sendrv
	CTL_SENDRV_CFGID_GET_MODE_THERMAL,				///< CTL_SENDRV_GET_THERMAL_PARAM, thermal sensor information

	/* set & get */
	CTL_SENDRV_CFGID_BASE = 0x02000000,
	CTL_SENDRV_CFGID_FLIP_TYPE,         ///< CTL_SEN_FLIP
	CTL_SENDRV_CFGID_AD_ID_MAP,         ///< CTL_SENDRV_AD_ID_MAP_PARAM
	CTL_SENDRV_CFGID_AD_TYPE,           ///< CTL_SENDRV_AD_IMAGE_PARAM
	CTL_SENDRV_CFGID_AD_INIT,           ///< NULL
	CTL_SENDRV_CFGID_EXT,               ///< CTL_SEN_EXT_PARAM,
	CTL_SENDRV_CFGID_MUX_VC_ID,	        ///< CTL_SENDRV_MUX_VC_ID_PARAM, for mux sendrv to assign MIPI vc

	/* user define */
	CTL_SENDRV_CFGID_USER_BASE = 0x03000000,

	CTL_SENDRV_CFGID_USER_DEFINE1,      ///< user define 1 by sensor
	CTL_SENDRV_CFGID_AE_PRESET = CTL_SENDRV_CFGID_USER_DEFINE1,

	CTL_SENDRV_CFGID_USER_DEFINE2,      ///< user define 2 by sensor
	CTL_SENDRV_CFGID_MIN_EXPT = CTL_SENDRV_CFGID_USER_DEFINE2,

	CTL_SENDRV_CFGID_USER_DEFINE3,      ///< user define 3 by sensor
	CTL_SENDRV_CFGID_USER_DEFINE4,      ///< user define 4 by sensor
	CTL_SENDRV_CFGID_USER_DEFINE5,      ///< user define 5 by sensor
	CTL_SENDRV_CFGID_USER_DEFINE6,      ///< user define 6 by sensor
	CTL_SENDRV_CFGID_USER_DEFINE7,      ///< user define 7 by sensor
	CTL_SENDRV_CFGID_USER_DEFINE8,      ///< user define 8 by sensor
	CTL_SENDRV_CFGID_USER_DEFINE9,      ///< user define 9 by sensor
	CTL_SENDRV_CFGID_USER_DEFINE10,     ///< user define 10 by sensor

	ENUM_DUMMY4WORD(CTL_SENDRV_CFGID)
} CTL_SENDRV_CFGID;

typedef struct {
	ER(*open)(CTL_SEN_ID id);                                               ///< initial sensor flow
	ER(*close)(CTL_SEN_ID id);                                              ///< un-initial sensor flow
	ER(*sleep)(CTL_SEN_ID id);                                              ///< enter sensor sleep mode
	ER(*wakeup)(CTL_SEN_ID id);                                             ///< exit sensor sleep mode
	ER(*write)(CTL_SEN_ID id, CTL_SEN_CMD *cmd);                            ///< write command
	ER(*read)(CTL_SEN_ID id, CTL_SEN_CMD *cmd);                             ///< read command
	ER(*chgmode)(CTL_SEN_ID id, CTL_SENDRV_CHGMODE_OBJ chgmode_obj);        ///< change mode flow
	ER(*chgfps)(CTL_SEN_ID id, UINT32 frame_rate);                          ///< change fps flow, frame_rate: fps * 100
	ER(*set_cfg)(CTL_SEN_ID id, CTL_SENDRV_CFGID drv_cfg_id, void *data);   ///< set sensor information (if sensor driver not support feature, pls return E_NOSPT)
	ER(*get_cfg)(CTL_SEN_ID id, CTL_SENDRV_CFGID drv_cfg_id, void *data);   ///< get sensor information (if sensor driver not support feature, pls return E_NOSPT)
} CTL_SEN_DRV_TAB;

typedef enum {
	CTL_SEN_PWR_CTRL_TURN_ON = 0,
	CTL_SEN_PWR_CTRL_TURN_OFF,
	CTL_SEN_PWR_CTRL_SAVE_BEGIN,    // not support, backward compatible
	CTL_SEN_PWR_CTRL_SAVE_END,      // not support, backward compatible
} CTL_SEN_PWR_CTRL_FLAG;

typedef enum {
	CTL_SEN_CLK_SEL_SIEMCLK,
	CTL_SEN_CLK_SEL_SIEMCLK2,
	CTL_SEN_CLK_SEL_SIEMCLK3,
	CTL_SEN_CLK_SEL_SIEMCLK4, // (567 N.S.)
	CTL_SEN_CLK_SEL_SIEMCLK5, // (538 N.S.)
	CTL_SEN_CLK_SEL_SIEMCLK6, // (538 N.S.)
	CTL_SEN_CLK_SEL_SIEMCLK7, // (538 N.S.)
	CTL_SEN_CLK_SEL_SIEMCLK8, // (538 N.S.)

	CTL_SEN_CLK_SEL_SPCLK,
	CTL_SEN_CLK_SEL_SPCLK2,

	CTL_SEN_CLK_SEL_MAX,

	CTL_SEN_CLK_SEL_SIEMCLK_IGNORE = CTL_SEN_IGNORE,
	ENUM_DUMMY4WORD(CTL_SEN_CLK_SEL)
} CTL_SEN_CLK_SEL;

typedef BOOL (*CTL_SEN_PLUG_IN)(CTL_SEN_ID id);
typedef void (*CTL_SEN_CLK_CB)(CTL_SEN_CLK_SEL clk_sel, BOOL en);
typedef void (*CTL_SEN_PWR_CTRL)(CTL_SEN_ID id, CTL_SEN_PWR_CTRL_FLAG flag, CTL_SEN_CLK_CB clk_cb);

typedef struct {
	CTL_SEN_PWR_CTRL pwr_ctrl;
	CTL_SEN_PLUG_IN det_plug_in;
	CTL_SEN_DRV_TAB *drv_tab;
} CTL_SEN_REG_OBJ, *PCTL_SEN_REG_OBJ;

/**
    sensor chgmode information
    * sensor frame rate
      - input: user input/SEN_FPS_DFT
      - user input: frame rate * 100
      - user input: must be less than sensor driver default value
*/
typedef enum {
	CTL_SEN_MODESEL_AUTO = 0,   ///< auto select sensor mode by sensor driver
	CTL_SEN_MODESEL_MANUAL,     ///< user indicates sensor mode
} CTL_SEN_MODESEL;

typedef struct {
	UINT32 frame_rate;                      ///< fps * 100
	USIZE size;                             ///< sensor output size (pixel)
	UINT32 frame_num;                       ///< sensor output frame number (linear mode: 1, hdr mode: 2~CTL_SEN_MFRAME_MAX_NUM)
	CTL_SEN_DATA_FMT data_fmt;              ///< sensor output data format
	CTL_SEN_PIXDEPTH pixdepth;              ///< sensor output pixel depth, set CTL_SEN_IGNORE for auto (sensor driver need to cover CTL_SEN_IGNORE)
	CTL_SEN_MODESEL_CCIR ccir;              ///< only CTL_SEN_DATA_FMT_YUV, CTL_SEN_DATA_FMT_Y_ONLY need to set
	BOOL mux_singnal_en;                    ///< only support CTL_SEN_IF_TYPE_PARALLEL
	CTL_SEN_PARA_MUX_INFO mux_signal_info;  ///< only (mux_singnal_en == 1) need to set
	CTL_SEN_MODE_TYPE mode_type_sel;        ///< if set 0 or CTL_SEN_IGNORE (sendrv will ignore this parameters)
	CTL_SEN_DATALANE data_lane;             ///< if set 0 or CTL_SEN_IGNORE (sendrv will ignore this parameters)
} CTL_SEN_MODESEL_AUTO_INFO;

typedef struct {
	UINT32 frame_rate;  // fps * 100
	CTL_SEN_MODE sen_mode;
} CTL_SEN_MODESEL_MANUAL_INFO;


typedef enum {
	CTL_SEN_VX1_IF_SEL_0,   ///< vx1 interface 0, support parallel sensor linear mode and mipi sensor linear mode
	CTL_SEN_VX1_IF_SEL_1,   ///< vx1 interface 1, support mipi sensor linear mode and hdr mode
	ENUM_DUMMY4WORD(CTL_SEN_VX1_IF_SEL)
} CTL_SEN_VX1_IF_SEL;

typedef enum {
	CTL_SEN_VX1_CTL_0,   ///< vx1 controller 0, mapping to KDRV_SSENIF_ENGINE_VX1_0, NT96680 only support SIE 1~5
	CTL_SEN_VX1_CTL_1,   ///< vx1 controller 1, mapping to KDRV_SSENIF_ENGINE_VX1_1, NT96680 only support SIE 4,6
	ENUM_DUMMY4WORD(CTL_SEN_VX1_CTL_SEL)
} CTL_SEN_VX1_CTL_SEL;

/*
    mapping to KDRV_SSENIFVX1_TXTYPE_THCV235/KDRV_SSENIFVX1_TXTYPE_THCV231/KDRV_SSENIFVX1_TXTYPE_THCV241
*/
typedef enum {
	CTL_SEN_VX1_TXTYPE_THCV235, ///< Vx1 Transmitter IC is THCV235.
	///< This only support Vx1 One-lane none-HDR parallel sensor.
	CTL_SEN_VX1_TXTYPE_THCV231, ///< Vx1 Transmitter IC is THCV231.
	///< This only support Vx1 One-lane none-HDR parallel sensor.

	CTL_SEN_VX1_TXTYPE_THCV241, ///< Vx1 Transmitter IC is THCV241. This is MIPI CSI input interface Vx1 Tx.
	///< Which can support 1 or 2 lanes Vx1 interface. HDR MIPI Sensor such as SONY_LI or OV/ON-SEMI is also supported.
	///< The maximum MIPI Sensor support spec is 4 lanes mipi with 1Gbps per lane by using Vx1 2 lanes.
	///< The maximum MIPI Sensor support spec is 4 lanes mipi with 500Mbps per lane by using Vx1 1 lanes.
	ENUM_DUMMY4WORD(CTL_SEN_VX1_TXTYPE)
} CTL_SEN_VX1_TXTYPE;

typedef struct {
	CTL_SEN_VX1_IF_SEL  if_sel;     ///< interface select
	CTL_SEN_VX1_TXTYPE tx_type;     ///< tx module select
} CTL_SEN_VX1_INFO;

typedef struct {
	CTL_SEN_CMDIF_TYPE type;///< [out]
	CTL_SEN_VX1_INFO vx1;   ///< [out], 510/520/560 N.S.
} CTL_SEN_GET_CMDIF_PARAM;

typedef struct {
	CTL_SEN_PARA_MUX_INFO mux_info;
} CTL_SEN_PARALLEL_INFO;

typedef struct {
	CTL_SEN_MODE mode;      ///< [in]
	CTL_SEN_IF_TYPE type;   ///< [out]
	union {
		CTL_SEN_PARALLEL_INFO parallel;
	} info; // [out]
} CTL_SEN_GET_IF_PARAM;


typedef enum {
	CTL_SEN_DATA_SWITCH_MODE_NONE 	= 0x00UL,
	CTL_SEN_DATA_SWITCH_MODE_A 		= 0x01UL, // HI_LO_BYTE_SWAP	, input = [15:8][7:0], the swap result = [7:0][15:8]
	CTL_SEN_DATA_SWITCH_MODE_B 		= 0x02UL, // LO_BYTE_INV		, input = [15:8][7:0], the swap result = [15:8][0:7]
	CTL_SEN_DATA_SWITCH_MODE_C 		= 0x04UL, // HI_BYTE_INV		, input = [15:8][7:0], the swap result = [8:15][7:0]
	CTL_SEN_DATA_SWITCH_MODE_D 		= 0x08UL, // LO_4BIT_SWAP		, input = [15:12][11:8][7:4][3:0], the swap result = [15:12][11:8][3:0][7:4]
	CTL_SEN_DATA_SWITCH_MODE_E 		= 0x10UL, // HI_4BIT_SWAP		, input = [15:12][11:8][7:4][3:0], the swap result = [11:8][15:12][7:4][3:0]
	ENUM_DUMMY4WORD(CTL_SEN_DATA_SWITCH_MODE)
} CTL_SEN_DATA_SWITCH_MODE;

typedef struct {
	CTL_SEN_MODE mode;                  // [in]
	CTL_SEN_DVI_FMT fmt;                // [out], sensor output fmt
	CTL_SEN_DVI_DATA_MODE data_mode;    // [out], sensor output data mode
	CTL_SEN_DATA_SWITCH_MODE data_switch_mode; // [out], sensor output data switch mode
} CTL_SEN_GET_DVI_PARAM;

typedef struct {
	UINT32 dft_fps;     ///< [out] sensor driver default (max) support frame rate (fps * 100)
	UINT32 chg_fps;     ///< [out] chgmode frame rate (fps * 100)
	UINT32 cur_fps;     ///< [out] current frame rate (fps * 100)
} CTL_SEN_GET_FPS_PARAM;

typedef struct {
	UINT32 fps;     	///< [in] current frame rate (fps * base)
	UINT32 base;		///< [in] current frame base
} CTL_SEN_SET_FPS_BASE_PARAM;

typedef struct {
	UINT32 dft_fps;     ///< [out] sensor driver default (max) support frame rate (fps * base)
	UINT32 chg_fps;     ///< [out] chgmode frame rate (fps * base)
	UINT32 cur_fps;     ///< [out] current frame rate (fps * base)
	UINT32 base;		///< [out] current frame base
} CTL_SEN_GET_FPS_BASE_PARAM;

/*
    538 hw surpport list :

                480     PLL4    PLL5    PLL6    PLL12
        MCLK1   O       x       O       O       O
        MCLK2   O       x       O       O       O
        MCLK3   O       x       O       O       O
        MCLK4   O       x       O       O       O
        MCLK5   x       x       x       x       x
        MCLK6   x       x       x       x       x
        MCLK7   x       x       x       x       x
        MCLK8   x       x       x       x       x
        SPCLK1  O       O       O       o       x
        SPCLK2  O       O       O       o       x

*/
typedef enum {
	CTL_SEN_CLKSRC_480,         ///< clock source as 480MHz
	CTL_SEN_CLKSRC_320,         ///< clock source as 320MHz
	CTL_SEN_CLKSRC_PLL4,        ///< clock source as PLL4
	CTL_SEN_CLKSRC_PLL5,        ///< clock source as PLL5
	CTL_SEN_CLKSRC_PLL6,        ///< clock source as PLL6
	CTL_SEN_CLKSRC_PLL11,       ///< clock source as PLL11
	CTL_SEN_CLKSRC_PLL12,       ///< clock source as PLL12
	CTL_SEN_CLKSRC_PLL20,       ///< clock source as PLL20
	CTL_SEN_CLKSRC_PLL23,       ///< clock source as PLL23
	CTL_SEN_CLKSRC_MAX,
	ENUM_DUMMY4WORD(CTL_SEN_CLKSRC_SEL)
} CTL_SEN_CLKSRC_SEL;

typedef struct {
	CHAR name[CTL_SEN_NAME_LEN];        ///< [out]
	CTL_SEN_VENDOR vendor;              ///< [out]
	UINT32 max_senmode;                 ///< [out]
	CTL_SEN_CMDIF_TYPE cmdif_type;      ///< [out]
	CTL_SEN_IF_TYPE if_type;            ///< [out]
	CTL_SEN_DRVDEV drvdev;              ///< [out] KDRV controller id(CSI/LVDS/TGE) or ch(TGE) information
	CTL_SEN_SUPPORT_PROPERTY property;  ///< [out] sensor support property
	UINT32 sync_timing;                 ///< [out] sync timing for Exposure time & gain(VD)
	CTL_SEN_SIGNAL_TYPE signal_type;    ///< [out]
	CTL_SEN_SIGNAL_INFO signal_info;    ///< [out]
	CTL_SEN_PAD_SEL pad_sel;	// only ccir sensor spt(sensor output to vie)
	CTL_SEN_SMP_EDGE smp_edge;	// only ccir sensor spt(sensor output to vie)
} CTL_SEN_GET_ATTR_PARAM;

typedef struct {
	CTL_SEN_MODE mode;  ///< [in]
	CTL_SEN_VD_DLY dly; ///< [out] vd delay
} CTL_SEN_VD_DLY_PARAM;

/* item define */
#define modesel_item_size_w          0   // value type : UINT32                  		, default : CTL_SEN_MODESEL_PARAM.size.w
#define modesel_item_size_h          1   // value type : UINT32                  		, default : CTL_SEN_MODESEL_PARAM.size.h
#define modesel_item_datafmt         2   // value type : CTL_SEN_DATA_FMT        		, default : CTL_SEN_DATA_FMT_RGB
#define modesel_item_pixdepth        3   // value type : CTL_SEN_PIXDEPTH        		, default : CTL_SEN_IGNORE
#define modesel_item_ccir_fmt        4   // value type : CTL_SEN_CCIR_FMT_SEL    		, default : CTL_SEN_FMT_SEL_CCIR656
#define modesel_item_ccir_interlace  5   // value type : BOOL                    		, default : FALSE
#define modesel_item_mux_datanum     6   // value type : UINT32                  		, default : 1       (only support CTL_SEN_IF_TYPE_PARALLEL)
#define modesel_item_mux_en          7   // value type : BOOL                    		, default : FALSE   (only support CTL_SEN_IF_TYPE_PARALLEL)
#define modesel_item_modetype        8   // value type : CTL_SEN_MODE_TYPE       		, default : CTL_SEN_IGNORE
#define modesel_item_datalane        9   // value type : CTL_SEN_DATALANE        		, default : CTL_SEN_IGNORE
#define modesel_item_vendor_param_0 10   // value type : UINT32 (refer ctl_sen_ext.h)	, default : CTL_SEN_IGNORE
#define modesel_item_vendor_param_1 11   // value type : UINT32 (refer ctl_sen_ext.h)   , default : CTL_SEN_IGNORE
#define modesel_item_vendor_param_2 12   // value type : UINT32 (refer ctl_sen_ext.h)   , default : CTL_SEN_IGNORE
#define modesel_item_vendor_param_3 13   // value type : UINT32 (refer ctl_sen_ext.h)   , default : CTL_SEN_IGNORE
#define modesel_item_vendor_param_4 14   // value type : UINT32 (refer ctl_sen_ext.h)   , default : CTL_SEN_IGNORE

typedef struct _CTL_SEN_MODESEL_PARAM_ADV {
	UINT32 frm_idx; ///< [in] specify frm_idx
	UINT32 item;    ///< [in] specify frm_idx set item (modesel_item_xxxxx, ex : modesel_item_size_w)
	UINT64 value;   ///< [in] specify frm_idx set value
	struct _CTL_SEN_MODESEL_PARAM_ADV *pnext;
} CTL_SEN_MODESEL_PARAM_ADV;

typedef struct {
	UINT32 fps;                     ///< [in] curr sen_id fps * 100
	USIZE size;                     ///< [in] curr sen_id size
	UINT32 frame_num;               ///< [in] curr sen_id frame number
	CTL_SEN_MODESEL_PARAM_ADV *adv; ///< [in]
	CTL_SEN_MODE mode;              ///< [out]
} CTL_SEN_MODESEL_PARAM;

typedef struct {
	CTL_SEN_MODE mode;      ///< [in]
	UINT32 frame_rate;      ///< [in] fps * 100
	UINT32 diff_row;        ///< [out] unit:row, diff row, which compare with previous frm_idx
	UINT32 diff_row_vd;     ///< [out] unit:row, diff row, which compare with previous frm_idx (v-sync)
} CTL_SEN_MFR_TIMING_PARAM;

typedef struct {
	CTL_SEN_MODE mode;              ///< [in]
	CTL_SEN_DATA_FMT data_fmt;      ///< [out]
	CTL_SEN_MODE_TYPE mode_type;    ///< [out]
	CTL_SEN_IF_TYPE if_type;        ///< [out]
	UINT32 dft_fps;                 ///< [out]sensor driver default (max) support frame rate (fps * 100)
	UINT32 frame_num;               ///< [out]
	CTL_SEN_STPIX stpix;            ///< [out]
	CTL_SEN_PIXDEPTH pixel_depth;   ///< [out]
	CTL_SEN_FMT fmt;                ///< [out] Sensor data type
	USIZE valid_size;                           ///< [out]the number of sample data, for serial interface
	URECT act_size[CTL_SEN_MFRAME_MAX_NUM];     ///< [out]the number of sample data, for sie act, must <= valid_size, [1]~[N] not valid in CTL_SEN_CFGID_GET_MFR_VER=1
	USIZE crp_size;                             ///< [out]the number of sample data, for sie crp (image data), must <= act_size
	CTL_SEN_MODE_SIGNAL signal_info;///< [out]
	UINT32 ratio_h_v;               ///< [out]sensor hv ratio (H:bit[31:16], V:bit[15:0])
	CTL_SEN_MODE_GAIN gain;         ///< [out] sensor gain X 1000
	UINT32 bining_ratio;            ///< [out] binning ratio X 100
	UINT32 row_time;                ///< [out] spend time (us) X10 per row, must be a fixed value.
	UINT32 row_time_step;           ///< [out] for AE, based on row_time
	UINT32 data_rate;               ///< [out] sie or tge clk freq must larger than data_rate
	CTL_SEN_SIEMCLK_SRC mclk_src;	///< [out] sen mode mclk src
	union {
		CTL_SEN_PARALLEL_INFO parallel;
	} if_info; // [out]
} CTL_SEN_GET_MODE_BASIC_PARAM;

typedef struct {
	INT32 probe_rst;               ///< [out] probe result, return 0 for check probe sensor OK
} CTL_SEN_GET_PROBE_SEN_PARAM;

typedef struct {
	USIZE size;         ///< [out], current plug size
	UINT32 fps;         ///< [out], current plug fps
	BOOL    interlace;          ///< [out] TRUE:CTL_SEN_MODE_CCIR_INTERLACE, FALSE:CTL_SEN_MODE_CCIR
	UINT32 param[5];    ///< [out], current plug info
} CTL_SEN_GET_PLUG_INFO_PARAM;

typedef struct {
	UINT32 pin;         ///< [SET-in][GET-in], NT96680 : pin=0 valid for THCV241, pin=1 valid for THCV241, pin=2 valid for THCV241
	///<    , pin=3 valid for THCV241/THCV231/THCV235, pin=4 valid for THCV231/THCV235
	BOOL value;         ///< [SET-in][GET-out], 0 for low, 1 for high
} CTL_SEN_CTL_VX1_GPIO_PARAM;

#define CTL_SEN_AD_VCAP_ID_MAX 8

typedef struct {
//	UINT32 chip_id;     ///< [in][out] chip_id(chip idx) for current sensor id
	UINT32 vin_id;      ///< [in][out] vin_id(ch) for current sensor id

	UINT32 reserved;
} CTL_SEN_AD_ID_MAP_PARAM;

/*
	Muxer (serdes) sendrv
	DESCRIPTION:
		[1] Using convention sendrv for each sensor.
		[2] Each sensor are registered under a muxer engine with muxer_id
		[3] The engine use the same MIPI IF to send through signal toward SIE.
		[4] Each sensor use unique virtual channel to split frame to different SIE.
	SUPPORT: 690
	PARAM: checkout CTL_SEN_MUX_SEN_PARAM
	FUNCTION:
		[1] VCAP: 	CTL_SEN_CFG_MUX_VC_ID, CTL_SEN_CFG_MUXER_VC_BIT
		[2] SENDRV: CTL_SENDRV_CFGID_MUX_VC_ID
*/
#define CTL_SEN_MUXER_ID_MAX 2
#define CTL_SEN_MUX_VC_ID_MAX 4

typedef struct {
	BOOL muxer_en; 		// [in] muxer enable
	UINT32 muxer_id; 	// [in][out] muxer id of the muxer device
	UINT32 vc_id; 		// [in][out] Assigned MIPI virtual channel id for cur sen_id
} CTL_SEN_MUX_SEN_PARAM;

typedef struct {
	UINT32 vin_id;      ///< [in] vin_id(ch) of image parameter
	UINT32 val;         ///< [in][out] set value / return value of image parameter

	UINT32 reserved;
} CTL_SEN_AD_IMAGE_PARAM;

typedef struct {
	UINT32 vc_id;		///< [in][out] vc_id (MIPI virtual channel id) for cur sen_id
} CTL_SEN_MUX_VC_ID_PARAM;

typedef struct {
	UINT32 vc_bit;	///< [out] muxer vc bitmap
} CTL_SEN_MUXER_VC_BIT_PARAM;

typedef struct {
	CTL_SEN_MODE mode;              ///< [in]
	USIZE size;                     ///< [out] dvs sensor dram output size, unit : pixel
} CTL_SEN_DVS_INFO_PARAM;

typedef struct {
	CTL_SEN_IF_TYPE type;
	CTL_SEN_IF_TGE tge;          ///< tge information
	UINT32 mclksrc_sync;    // invalid parameter, backward compatible, pls used PIN_SENSOR2_CFG_MCLK_SRC
} CTL_SEN_INIT_IF_CFG;

typedef struct {
	BOOL en;
	CTL_SEN_VX1_IF_SEL  if_sel;     ///< interface select
	CTL_SEN_VX1_TXTYPE tx_type;     ///< tx module select
} CTL_SEN_CMDIF_VX1;

typedef struct {
	CTL_SEN_CMDIF_VX1 vx1;          ///< invalid parameter, backward compatible
} CTL_SEN_INIT_CMDIF_CFG;

#define CTL_SEN_PINMUX_MAX_NUM 10
typedef struct _CTL_SEN_PINMUX {
	UINT32  func;           ///< need mapping to top.h define (PIN_FUNC)
	UINT32  cfg;            ///< need mapping to top.h define (PIN_SENSOR_CFG/PIN_SENSOR2_CFG/.../PIN_SENSOR8_CFG/PIN_MIPI_LVDS_CFG/PIN_I2C_CFG/PIN_SIF_CFG/...)
	///< 1. GPIO->FUNCTION :
	///<    (1) PIN_I2C_CFG : after sendrv power on
	///<    (2) PIN_SENSORx_CFG(except cfg_mclk)/PIN_MIPI_LVDS_CFG/PIN_SIF_CFG : sensor_open
	///< 2. FUNCTION->GPIO :
	///<    (1) PIN_I2C_CFG : before sendrv power off
	UINT32  cfg_mclk;       ///< need mapping to top.h define (PIN_SENSOR_CFG/PIN_SENSOR2_CFG/.../PIN_SENSOR8_CFG)
	///< 1. GPIO->FUNCTION :
	///<    (1) before MCLK_ENABLE
	///< 2. FUNCTION->GPIO :
	///<    (1) after MCLK_DISABLE
	struct _CTL_SEN_PINMUX *pnext;
} CTL_SEN_PINMUX;

typedef struct {
	CTL_SEN_PINMUX pinmux;          // set gpio for function
	CTL_SEN_CLANE_SEL clk_lane_sel; // only LVDS and MIPI need to set, 510/520 N.S.
	UINT32 sen_2_serial_pin_map[CTL_SEN_SER_MAX_DATALANE];    	// Serial (lvds/csi/slvsec) data pin 2 hw map
	CTL_SEN_DATA_SWITCH_MODE data_switch_mode; 					// ccir parallel sensor only
	BOOL ccir_vd_hd_pin;            // invalid parameter, backward compatible
	BOOL vx1_tx241_cko_pin;         // invalid parameter, backward compatible
	BOOL vx1_tx241_cfg_2lane_mode;  // invalid parameter, backward compatible
	CTL_SEN_PAD_SEL pad_sel;	// only ccir sensor spt(sensor output to vie)
	CTL_SEN_SMP_EDGE smp_edge;	// only ccir sensor spt(sensor output to vie)
} CTL_SEN_INIT_PIN_CFG;

typedef struct {
	CTL_SEN_AD_ID_MAP_PARAM ad_id_map; 	// for ad sendrv
	CTL_SEN_CCIR_FMT_SEL fmt;			// for 530 VIE set default CHx_FMT
	CTL_SEN_MUX_SEN_PARAM mux_sen;		// for mux(serdes) sendrv
} CTL_SEN_INIT_SENDRV_CFG;

typedef struct {
	CTL_SEN_INIT_PIN_CFG pin_cfg;
	CTL_SEN_INIT_IF_CFG if_cfg;
	CTL_SEN_INIT_CMDIF_CFG cmd_if_cfg;
	CTL_SEN_INIT_SENDRV_CFG sendrv_cfg;
	CTL_SEN_DRVDEV drvdev;
	CTL_SEN_DRVDEV drvdev2;	// used in combo mode, need multiple controllers to receive single image data
} CTL_SEN_INIT_CFG_OBJ;

typedef struct {
	CHAR                   *chip_name;
	UINT32                  chip_id;  // sensor physical id
	CTL_SEN_INIT_CFG_OBJ   *chip_cfg;
} CTL_SEN_INIT_MAP;

typedef struct {
	CTL_SEN_MODE    sen_mode;
	UINT32          frm_idx;
} CTL_SEN_MAP;

typedef struct {
	UINT32  fps;            // fps * 100
	UINT32  output_dest;    // ctl_sen_output_sie_x or ctl_sen_output_vie_x, ex ctl_sen_output_sie_0
} CTL_SEN_CHGMODE_INFO;

typedef struct {
	CTL_SEN_MODE mode;												///< [in]
	UINT32 combo_num;												///< [out] the number of combine frame
	USIZE valid_size[CTL_SEN_COMB_MAX_NUM];							///< [out] the number of sample data, for serial interface
	URECT act_size[CTL_SEN_COMB_MAX_NUM][CTL_SEN_MFRAME_MAX_NUM];	///< [out] the number of sample data, for sie act, must <= valid_size
	USIZE crp_size[CTL_SEN_COMB_MAX_NUM];							///< [out] the number of sample data (with overlap width), for sie crp (image data), must <= act_size
	UINT32 ovlp_width[CTL_SEN_COMB_MAX_NUM];						///< [out] the number of overlap width (right overlap width)
} CTL_SEN_GET_MODE_COMBO_PARAM;

#define CTL_SEN_LVDS_SYNCCODE_LEN_MAX 7
#define CTL_SEN_LVDS_CW_FRM_NUM 4
typedef struct {
	BOOL enable;										// 1: sync info from user setting. 0: sync info from sensor driver
	UINT32 sync_word_len;								// sync word length
	UINT32 sync_word[CTL_SEN_LVDS_SYNCCODE_LEN_MAX]; 	// sync word value
	UINT32 cw_hd;										// ctrl word for hsync
	UINT32 cw_vd[CTL_SEN_LVDS_CW_FRM_NUM];				// ctrl word for vsync 1~n
	UINT32 cw_le;										// ctrl word for line end
	UINT32 cw_fe[CTL_SEN_LVDS_CW_FRM_NUM];				// ctrl word for frame end 1~n
	CTL_SEN_LVDS_VSYNC_GEN vd_gen_method;				// Vsync Pulse Generate method
	BOOL vd_gen_with_hd;								// Set Vsync generation together with Hsync function enable/disable
	CTL_SEN_DATAIN_BIT_ORDER data_in_order;				// Data Input Bit order
} CTL_SEN_LVDS_SYNC_MANUAL_INFO;

#define CTL_SEN_MAX_REG_SENDRV 8

/** 
	CTL_SEN_NUM_SEN_ID should be synchronize with:
  	- isf_vdocap.h		VDOCAP_MAX_NUM
  	- ctl_sen_clk.c		SIEMCLK_SOURCE
  	- Proposal/CIS_RX	sensor data lanes					
*/
#define CTL_SEN_MIN_SEN_ID 0
#define CTL_SEN_MAX_SEN_ID 9
#define CTL_SEN_NUM_SEN_ID (CTL_SEN_MAX_SEN_ID - CTL_SEN_MIN_SEN_ID + 1)


#define ctl_sen_output_sie_base 0
#define ctl_sen_output_sie_0 ctl_sen_output_sie_base
#define ctl_sen_output_sie_1 1
#define ctl_sen_output_sie_2 2
#define ctl_sen_output_sie_3 3
#define ctl_sen_output_sie_4 4
#define ctl_sen_output_sie_5 5
#define ctl_sen_output_sie_6 6
#define ctl_sen_output_sie_7 7
#define ctl_sen_output_sie_8 8
#define ctl_sen_output_sie_9 9
#define ctl_sen_output_sie_10 10
#define ctl_sen_output_sie_11 11
#define ctl_sen_output_sie_12 12
#define ctl_sen_output_sie_13 13
#define ctl_sen_output_sie_14 14
#define ctl_sen_output_sie_15 15

/* vie engine 0, video ch 0 ~ 3 */
#define ctl_sen_output_vie_base 100
#define ctl_sen_output_vie_0 ctl_sen_output_vie_base
#define ctl_sen_output_vie_1 101
#define ctl_sen_output_vie_2 102
#define ctl_sen_output_vie_3 103

/* vie engine 1, video ch 0 ~ 3 */
#define ctl_sen_output_vie_4 104
#define ctl_sen_output_vie_5 105
#define ctl_sen_output_vie_6 106
#define ctl_sen_output_vie_7 107


/*
    for ctl_sen_status

    get / set item , user must check power on or not
*/

#define CTL_SEN_STATUS_IDLE         0x0
#define CTL_SEN_STATUS_MAP_INIT     0x1
#define CTL_SEN_STATUS_MAP_ADD      0x2
#define CTL_SEN_STATUS_OPEN         0x4
#define CTL_SEN_STATUS_PWR_ON       0x8
#define CTL_SEN_STATUS_CHGMODE      0x10
#define CTL_SEN_STATUS_SLEEP        0x20
#define CTL_SEN_STATUS_SET          0x40
#define CTL_SEN_STATUS_GET          0x80
#define CTL_SEN_STATUS_SET_REG      0x100
#define CTL_SEN_STATUS_GET_REG      0x200
#define CTL_SEN_STATUS_WRIT_REG     0x400
#define CTL_SEN_STATUS_READ_REG     0x800
#define CTL_SEN_STATUS_WAIT_STS     0x1000

/*
    for CTL_SEN_CFG_CHGMODE_COND
*/
#define CTL_SEN_CHGMODE_COND_FORCE_SENDRV 1 // force sendrv chgmode flow

/*
    for CTL_SEN_CFG_PINMUX_COND
*/
#define CTL_SEN_PINMUX_COND_UPD_SETTING   0x0 // update pinmux setting if off
#define CTL_SEN_PINMUX_COND_KEEP_SETTING  0x1 // keep pinmux setting if off

typedef enum {
	CTL_SEN_CFG_EXPT,           ///< [set],     data_type: void (user define), sensor exposure setting
	CTL_SEN_CFG_GAIN,           ///< [set],     data_type: void (user define), sensor gain setting
	CTL_SEN_CFG_ATTR,           ///<     [get], data_type: CTL_SEN_GET_ATTR_PARAM
	CTL_SEN_CFG_MODESEL,        ///<     [get], data_type: CTL_SEN_MODESEL_PARAM
	CTL_SEN_CFG_MODE_BASIC,     ///<     [get], data_type: CTL_SEN_GET_MODE_BASIC_PARAM
	CTL_SEN_CFG_FPS,            ///< [set/get], data_type: set[UINT32 (fps*100)], get[CTL_SEN_GET_FPS_PARAM]
	CTL_SEN_CFG_FPS_BASE,		///< [set/get], data_type: set[CTL_SEN_SET_FPS_BASE_PARAM], get[CTL_SEN_GET_FPS_BASE_PARAM]
	CTL_SEN_CFG_FLIP,           ///< [set/get], data_type: CTL_SEN_FLIP
	CTL_SEN_CFG_STATUS,         ///< [set],     data_type: CTL_SEN_STATUS, sensor status
	CTL_SEN_CFG_MFR_TIMING,     ///<     [get], data_type: CTL_SEN_MFR_TIMING_PARAM, sensor output frame timing
	CTL_SEN_CFG_VD_DLY,         ///<     [get], data_type: CTL_SEN_VD_DLY_PARAM, multi-frame sensor mode only
	CTL_SEN_CFG_AD_ID_MAP,      ///< [set/get], data_type: CTL_SEN_AD_ID_MAP_PARAM
	CTL_SEN_CFG_AD_TYPE,        ///< [set/get], data_type: CTL_SEN_AD_IMAGE_PARAM
	CTL_SEN_CFG_MUX_VC_ID,   	///< [set/get], data_type: CTL_SEN_MUX_VC_ID_PARAM
	CTL_SEN_CFG_MUXER_VC_BIT,	///<     [get], data_type: CTL_SEN_MUXER_VC_BIT_PARAM, for muxer sendrv control vc bit
	CTL_SEN_CFG_DVS_INFO,       ///<     [get], data_type: CTL_SEN_DVS_INFO_PARAM, dvs sensor information
	CTL_SEN_CFG_DVI_INFO,       ///<     [get], data_type: CTL_SEN_GET_DVI_PARAM
	CTL_SEN_CFG_PLUG,           ///<     [get], data_type: BOOL
	CTL_SEN_CFG_PLUG_INFO,      ///<     [get], data_type: CTL_SEN_GET_PLUG_INFO_PARAM
	CTL_SEN_CFG_PROBE_SEN,      ///<     [get], data_type: CTL_SEN_GET_PROBE_SEN_PARAM, need to get after sensor power on

	CTL_SEN_CFG_USER1,          ///< [set/get], data_type: define user
	CTL_SEN_CFG_AE_PRESET = CTL_SEN_CFG_USER1,

	CTL_SEN_CFG_USER2,          ///< [set/get], data_type: define user
	CTL_SEN_CFG_MIN_EXPT = CTL_SEN_CFG_USER2,

	CTL_SEN_CFG_USER3,          ///< [set/get], data_type: define user
	CTL_SEN_CFG_USER4,          ///< [set/get], data_type: define user
	CTL_SEN_CFG_USER5,          ///< [set/get], data_type: define user
	CTL_SEN_CFG_USER6,          ///< [set/get], data_type: define user
	CTL_SEN_CFG_USER7,          ///< [set/get], data_type: define user
	CTL_SEN_CFG_USER8,          ///< [set/get], data_type: define user
	CTL_SEN_CFG_USER9,          ///< [set/get], data_type: define user
	CTL_SEN_CFG_USER10,         ///< [set/get], data_type: define user

	CTL_SEN_CFG_SSENIF_VDCNT,   ///<     [get], data_type: UINT32, ssenif vd count
	CTL_SEN_CFG_SSENIF_TIMEOUT, ///< [set/get], data_type: UINT32, ms, default: 1000ms, only CTL_SEN_IF_TYPE_LVDS, CTL_SEN_IF_TYPE_MIPI support
	CTL_SEN_CFG_SSENIF_ERRCNT,  ///<     [get], data_type: UINT32, ssenif err count
	CTL_SEN_CFG_SSENIF_GATING, 	///< [set/get], data_type: BOOL, Set channel pixel clock auto-gating, only CTL_SEN_IF_TYPE_MIPI support

	CTL_SEN_CFG_LVDS_SYNC_MANUAL,	///< [set], data_type: CTL_SEN_LVDS_SYNC_MANUAL_INFO, lvds manually set sync info

	CTL_SEN_CFG_TGE_VD_PERIOD,  ///< [set/get], data_type: UINT32
	CTL_SEN_CFG_TGE_HD_PERIOD,  ///< [set/get], data_type: UINT32
	CTL_SEN_CFG_TGE_VD_SYNC,    ///< [set/get], data_type: UINT32
	CTL_SEN_CFG_TGE_HD_SYNC,    ///< [set/get], data_type: UINT32
	CTL_SEN_CFG_TGE_RST,		///< [set],     data_type: UINT32, toggle tge reset

	CTL_SEN_CFG_EXT,            ///< [set/get], data_type: CTL_SEN_EXT_PARAM,
	CTL_SEN_CFG_CHGMODE_COND,  	///< [set/get], data_type: UINT32, CTL_SEN_CHGMODE_COND_XXX (ex : CTL_SEN_CHGMODE_COND_FORCE_SENDRV)
	CTL_SEN_CFG_EXTRACT_DATA,   ///<     [get], data_type: CTL_SEN_EXTRACT_DATA_PARAM
	CTL_SEN_CFG_SSENIF_FORCE_DIS,	///< [set], data_type: UINT32, TRUE/FALSE, force engine clear status when stop

	CTL_SEN_CFG_PINMUX_COND,    ///< [set/get], data_type: UINT32, CTL_SEN_PINMUX_COND_XXX (ex : CTL_SEN_PINMUX_COND_KEEP_SETTING)
	CTL_SEN_CFG_MODE_COMBO,		///<     [get], data_type: CTL_SEN_GET_MODE_COMBO_PARAM, sensor combination setting
	CTL_SEN_CFG_THERMAL_INFO,	///<     [get], data_type: CTL_SEN_GET_THERMAL_PARAM, thermal sensor information

	CTL_SEN_CFG_MAX,

	ENUM_DUMMY4WORD(CTL_SEN_CFG)
} CTL_SEN_CFG;

#define CTL_SEN_CFG_SET_WITH_REG (FLGPTN_BIT(CTL_SEN_CFG_EXPT) | FLGPTN_BIT(CTL_SEN_CFG_GAIN) | FLGPTN_BIT(CTL_SEN_CFG_FPS) | FLGPTN_BIT(CTL_SEN_CFG_FLIP))
#define CTL_SEN_CFG_GET_WITH_REG (FLGPTN_BIT(CTL_SEN_CFG_EXPT) | FLGPTN_BIT(CTL_SEN_CFG_GAIN) | FLGPTN_BIT(CTL_SEN_CFG_FPS) | FLGPTN_BIT(CTL_SEN_CFG_FLIP) | FLGPTN_BIT(CTL_SEN_CFG_PLUG))

/* backward compatible */
#define CTL_SEN_CFGID CTL_SEN_CFG
#define CTL_SEN_CFGID_SET_EXPT          CTL_SEN_CFG_EXPT
#define CTL_SEN_CFGID_SET_GAIN          CTL_SEN_CFG_GAIN
#define CTL_SEN_CFGID_SET_FPS           CTL_SEN_CFG_FPS
#define CTL_SEN_CFGID_SET_STATUS        CTL_SEN_CFG_STATUS
#define CTL_SEN_CFGID_GET_EXPT          CTL_SEN_CFG_EXPT
#define CTL_SEN_CFGID_GET_GAIN          CTL_SEN_CFG_GAIN
#define CTL_SEN_CFGID_GET_ATTR          CTL_SEN_CFG_ATTR
#define CTL_SEN_CFGID_GET_FPS           CTL_SEN_CFG_FPS
#define CTL_SEN_CFGID_GET_MODE_BASIC    CTL_SEN_CFG_MODE_BASIC
#define CTL_SEN_CFGID_FLIP_TYPE         CTL_SEN_CFG_FLIP
#define CTL_SEN_CFGID_USER_DEFINE1      CTL_SEN_CFG_USER1
#define CTL_SEN_CFGID_USER_DEFINE2      CTL_SEN_CFG_USER2
#define CTL_SEN_CFGID_USER_DEFINE3      CTL_SEN_CFG_USER3
#define CTL_SEN_CFGID_USER_DEFINE4      CTL_SEN_CFG_USER4
#define CTL_SEN_CFGID_USER_DEFINE5      CTL_SEN_CFG_USER5
#if 1 // TODO
#define CTL_SEN_CFGID_TGE_VD_PERIOD     CTL_SEN_CFG_TGE_VD_PERIOD
#define CTL_SEN_CFGID_TGE_HD_PERIOD     CTL_SEN_CFG_TGE_HD_PERIOD
#define CTL_SEN_CFGID_TGE_VD_SYNC       CTL_SEN_CFG_TGE_VD_SYNC
#define CTL_SEN_CFGID_TGE_HD_SYNC       CTL_SEN_CFG_TGE_HD_SYNC
#endif
/* backward compatible end */


int kflow_ctl_sen_init(void);
int kflow_ctl_sen_uninit(void);

void ctl_sen_install_id(void);
void ctl_sen_uninstall_id(void);

UINT32 ctl_sen_buf_query(UINT32 num);
INT32 ctl_sen_init_buf(ULONG buf_addr, UINT32 buf_size);
INT32 ctl_sen_uninit_buf(void);

/* backward compatible */
#define ctl_sen_init(buf_addr, buf_size) ctl_sen_init_buf((ULONG)buf_addr, (UINT32)buf_size)
#define ctl_sen_uninit() ctl_sen_uninit_buf()

/*
    sendrv register and unregister
    max register sendrv number : CTL_SEN_MAX_REG_SENDRV
*/
INT32 ctl_sen_reg_sendrv(CHAR *name, CTL_SEN_REG_OBJ *reg_obj);    ///< name length: CTL_SEN_NAME_LEN
INT32 ctl_sen_unreg_sendrv(CHAR *name);                            ///< name length: CTL_SEN_NAME_LEN


/*

    init_map -> (add_map) <-> open ->

    -----------------------------
    |   power_on / power_off    |
    |   chgmode                 |
    |   set / get               |
    |   add_map / del_map       |
    |   sleep / wakeup          |
    -----------------------------

    close <-> (del_map) -> uninit_map

    sen_id range : CTL_SEN_MIN_SEN_ID ~ CTL_SEN_MAX_SEN_ID
*/
INT32 ctl_sen_init_map(UINT32 sen_id, CTL_SEN_INIT_MAP *init_map);
INT32 ctl_sen_uninit_map(UINT32 sen_id);

INT32 ctl_sen_add_map(UINT32 sen_id, CTL_SEN_MAP *map);
INT32 ctl_sen_del_map(UINT32 sen_id);

INT32 ctl_sen_senid_info(UINT32 sen_id, UINT32 *chip_id, CTL_SEN_MAP *map, CTL_SEN_CHGMODE_INFO *chgmode_info);
UINT32 ctl_sen_get_senid(UINT32 chip_id, UINT32 frm_idx, CHAR *chip_name);

INT32 ctl_sen_open(UINT32 sen_id);
INT32 ctl_sen_close(UINT32 sen_id);

INT32 ctl_sen_sleep(UINT32 sen_id);
INT32 ctl_sen_wakeup(UINT32 sen_id);

UINT32 ctl_sen_status(UINT32 sen_id);   // return CTL_SEN_STATUS_XXX (ex : CTL_SEN_STATUS_IDEL)

INT32 ctl_sen_pwr_ctrl(UINT32 sen_id, CTL_SEN_PWR_CTRL_FLAG flag);

INT32 ctl_sen_chgmode(UINT32 sen_id, CTL_SEN_CHGMODE_INFO *info);

INT32 ctl_sen_set(UINT32 sen_id, CTL_SEN_CFG cfg, void *param);
INT32 ctl_sen_get(UINT32 sen_id, CTL_SEN_CFG cfg, void *param);

INT32 ctl_sen_write_reg(UINT32 sen_id, CTL_SEN_CMD *cmd);
INT32 ctl_sen_read_reg(UINT32 sen_id, CTL_SEN_CMD *cmd);

CTL_SEN_INTE ctl_sen_wait_intruupt(UINT32 sen_id, CTL_SEN_INTE waited_flag);

#endif