

#ifndef _IME_ENG_COMM_H_
#define _IME_ENG_COMM_H_

#ifdef __cplusplus
extern "C" {
#endif



#include "kwrap/type.h"
#include "kwrap/semaphore.h"
#include "kdrv_type.h"

#include "ime_eng_int_reg.h"





typedef enum _IME_ENG_OPMODE_ {
	IME_ENG_OPMODE_D2D = 0, ///< Engine D2D
	IME_ENG_OPMODE_IFE2IME, ///< IFE -> DCE -> IPE -> IME
	IME_ENG_OPMODE_DCE2IME, ///< DCE -> IPE -> IME
	IME_ENG_OPMODE_SIE2IME, ///< SIE -> IFE -> DCE -> IPE -> IME
	IME_ENG_OPMODE_MAX,
	ENUM_DUMMY4WORD(IME_ENG_OPMODE)
} IME_ENG_OPMODE;


/**
    IME enum - output path selection
*/
typedef enum _IME_ENG_PATH_SEL_ {
	IME_PATH_IN = 0, ///< operation for input path

	IME_PATH1 = 1,   ///< operation for output path1
	IME_PATH2 = 2,   ///< operation for output path2
	IME_PATH3 = 3,   ///< operation for output path3
	IME_PATH4 = 4,   ///< operation for output path4
	ENUM_DUMMY4WORD(IME_ENG_PATH_SEL)
} IME_ENG_PATH_SEL;


typedef enum _IME_ENG_BUF_SEL_ {
	IME_ENG_BUF_SEL_Y = 0,  ///< buffer selection for Y channel
	IME_ENG_BUF_SEL_U = 1,  ///< buffer selection for U channel
	IME_ENG_BUF_SEL_V = 2,  ///< buffer selection for V channel
	ENUM_DUMMY4WORD(IME_ENG_BUF_SEL)
} IME_ENG_BUF_SEL;


/**
    ISD user coefficient control selection
*/
typedef enum _IME_ISD_UCOEF_CTRL_SEL_ {
	IME_ISD_WITHOUT_UCOEF = 0,   ///< without user coefficient control
	IME_ISD_WITH_UCOEF = 1,      ///< with user coefficient control
	ENUM_DUMMY4WORD(IME_ISD_UCOEF_CTRL_SEL)
} IME_ISD_UCOEF_CTRL_SEL;

/**
    IME enum - scale factor computation mode selection
*/
typedef enum _IME_SCALE_FACTOR_COEF_MODE {
	IME_SCALE_FACTOR_COEF_AUTO_MODE = 0,    ///< Auto Scale factor coefficeint computation mode
	IME_SCALE_FACTOR_COEF_USER_MODE = 1,    ///< User Scale factor coefficeint computation mode
	ENUM_DUMMY4WORD(IME_SCALE_FACTOR_COEF_MODE)
} IME_SCALE_FACTOR_COEF_MODE;



/**
    IME enum - scale type selection
*/
typedef enum _IME_SCALE_TYPE_SEL {
	IME_SCALE_DOWN = 0,  ///< scaling down enable
	IME_SCALE_UP = 1,    ///< scaling up enable
	ENUM_DUMMY4WORD(IME_SCALE_TYPE_SEL)
} IME_SCALE_TYPE_SEL;


/**
    IME structure - image size parameters for horizontal and vertical direction
*/
typedef struct _IME_SIZE_INFO {
	UINT32 size_h;                  ///< horizontal size
	UINT32 size_v;                  ///< vertical size
} IME_SIZE_INFO;




/**
    IME structure - scaling factors for horizontal and vertical direction
*/
typedef struct _IME_SCALE_FACTOR_INFO {

	IME_ISD_UCOEF_CTRL_SEL isd_coef_ctrl;           ///< user coefficient control mode

	IME_SCALE_TYPE_SEL    scale_h_ud;               ///< horizontal scale up/down selection
	UINT32                scale_h_dr;               ///< horizontal scale down rate
	UINT32                scale_h_ftr;              ///< horizontal scale factor
	UINT32                scale_h_ofs;              ///< horizontal scale initial offset
	UINT32                isd_scale_h_base_ftr;     ///< horizontal scale base
	UINT32                isd_h_coef_adj;           ///< horizontal coefficient adjustment
	UINT32                isd_scale_h_ftr[3];       ///< horizontal scale factor for integration method
	UINT32                isd_scale_h_coef_nums;    ///< coefficient numbers for horizontal direction


	IME_SCALE_TYPE_SEL    scale_v_ud;               ///< vertical scale up/down selection
	UINT32                scale_v_dr;               ///< vertical scale down rate
	UINT32                scale_v_ftr;              ///< vertical scale factor
	UINT32                isd_scale_v_base_ftr;     ///< horizontal scale base
	UINT32                isd_v_coef_adj;           ///< vertical coefficient adjustment
	UINT32                isd_scale_v_ftr[3];       ///< vertical scale factor for integration method
	UINT32                isd_scale_v_coef_nums;    ///< coefficient numbers for vertical direction

	INT32                 isd_scale_coefs[32];      ///< user coefficient of ISD


	//INT32                 isd_scale_h_coefs[17];      ///< user coefficient for horizontal direction
	//INT32                 isd_scale_h_coefs_all_sum;       ///< all user coefficient sum for horizontal direction
	//INT32                 isd_scale_h_coefs_half_sum;       ///< half user coefficient sum for horizontal direction

	//INT32                 isd_scale_v_coefs[17];      ///< user coefficient for vertical direction
	//INT32                 isd_scale_v_coefs_all_sum;       ///< all user coefficient sum for vertical direction
	//INT32                 isd_scale_v_coefs_half_sum;       ///< half user coefficient sum for vertical direction

	IME_SCALE_FACTOR_COEF_MODE CalScaleFactorMode;  ///< scale factor mode, recommended: IME_SCALE_FACTOR_COEF_AUTO_MODE
} IME_SCALE_FACTOR_INFO;

//------------------------------------------------------------------
typedef enum _IME_ENG_ID {
	IME_ID_0  = 0,
	IME_ID_1,
	IME_ID_MAX_NUM,
	ENUM_DUMMY4WORD(IME_ENG_ID)
} IME_ENG_ID;


typedef void (*IME_ISR_CB)(void *eng, UINT32 status, void *reserve);

typedef struct _IME_ENG_HANDLE_ {
	UINT32 chip_id;
	UINT32 eng_id;
	CHAR name[16];

	UINT32 clock_rate;
	void *pclk;         /* linux struct clk*, not used in freertos */
	volatile NT98538_IME_ENG_REG_STRUCT *p_ime_reg_st;
	volatile UINT8 *p_ime_reg_chg_flag;
	ULONG reg_io_base;
	UINT32 irq_id;

	BOOL dmach_dis;
	INT32 isr_msg_cnt;

	IME_ISR_CB isr_cb;
} IME_ENG_HANDLE;



#ifdef __cplusplus
}
#endif

#endif

