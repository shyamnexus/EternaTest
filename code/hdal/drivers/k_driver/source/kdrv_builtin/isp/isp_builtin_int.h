#ifndef _ISP_BUILTIN_INT_H_
#define _ISP_BUILTIN_INT_H_

#include "kwrap/type.h"
#if defined(__KERNEL__)
// TODO: isp builtin
#if 1
#include "linux/soc/nvt/rcw_macro.h"
#else
#include "mach/rcw_macro.h"
#endif
#endif

#include "isp_builtin.h"

//=============================================================================
// version
//=============================================================================
#define ISP_BUILTIN_VERSION 0x00100002

//=============================================================================
// debug message
//=============================================================================
#define ISP_BUILTIN_PRINT_VD_CNT             0  // NOTE: Set value to print VD trigger.
#define ISP_BUILTIN_PRINT_IPP_CNT            0  // NOTE: Set value to print IPP trigger.
#define ISP_BUILTIN_PRINT_ENC_CNT            0  // NOTE: Set value to print IPP trigger.
#define ISP_BUILTIN_PRINT_EXPT_CNT           0  // NOTE: Set value to print expt.
#define ISP_BUILTIN_PRINT_GAIN_CNT           0  // NOTE: Set value to print gain.
#define ISP_BUILTIN_PRINT_CGAIN_CNT          0  // NOTE: Set value to print cgain.
#define ISP_BUILTIN_PRINT_AE_MSG_CNT         0  // NOTE: Set value to AE message.
#define ISP_BUILTIN_PRINT_AWB_MSG_CNT        0  // NOTE: Set value to AWB message.
#define ISP_BUILTIN_PRINT_IQ_MSG_CNT         0  // NOTE: Set value to IQ message.
#define ISP_BUILTIN_PRINT_BUF_ENABLE         0

//=============================================================================
// flow control
//=============================================================================
#if defined(__KERNEL__)
#define _AE_BUILTIN_NVT                      1  // NOTE: Set 1 to use NVT AE
#define _AWB_BUILTIN_NVT                     1  // NOTE: Set 1 to use NVT AWB
#else
// NOTE: RTOS using normal 2A
#define _AE_BUILTIN_NVT                      0  // NOTE: Set 1 to use NVT AE
#define _AWB_BUILTIN_NVT                     0  // NOTE: Set 1 to use NVT AWB
#endif
#define ISP_BUILTIN_TRIG_2A_IQ               1  // NOTE: Set 1 to regist cb function.
#define ISP_BUILTIN_BYPASS_AE                0  // NOTE: Set 1 to bypass AE.
#define ISP_BUILTIN_BYPASS_AWB               0  // NOTE: Set 1 to bypass AWB.
#define ISP_BUILTIN_BYPASS_SENSOR            0
#define ISP_BUILTIN_RELOAD_DTSI              1
#define ISP_BUILTIN_MEASURE_DTSI             1
//=============================================================================
// code size control
//=============================================================================
//#if defined(CONFIG_NVT_FAST_ISP_FLOW)
#if defined(__KERNEL__) && !defined(CONFIG_NVT_AMP)
	#define NVT_FAST_ISP_FLOW  1
#else
	// RTOS
	#define NVT_FAST_ISP_FLOW  1
#endif

//=============================================================================
// struct & enum definition
//=============================================================================
#define FLGPTN_SIE_RESET      FLGPTN_BIT(1)
#define FLGPTN_PROC_DRAMEND   FLGPTN_BIT(2)
#define FLGPTN_PROC_VD        FLGPTN_BIT(3)
#define FLGPTN_SIE_RESET_2    FLGPTN_BIT(4)
#define FLGPTN_PROC_DRAMEND_2 FLGPTN_BIT(5)
#define FLGPTN_PROC_VD_2      FLGPTN_BIT(6)
#define FLGPTN_STOP           FLGPTN_BIT(7)
#define FLGPTN_IDLE_2         FLGPTN_BIT(30)
#define FLGPTN_IDLE           FLGPTN_BIT(31)

#if defined(__KERNEL__)
#define SET_FLG(x, y) set_flg((x), (y))
#define CLR_FLG(x, y) clr_flg((x), (y))
#define MALLOC(x) kmalloc((x), GFP_KERNEL)
#define FREE(x) kfree((x))
#else
#define SET_FLG(x, y) vos_flag_iset((x), (y))
#define CLR_FLG(x, y) vos_flag_iclr((x), (y))
#define MALLOC(x) malloc((x))
#define FREE(x) free((x))
#endif

#define ISP_CA_W_WINNUM             32
#define ISP_CA_H_WINNUM             32
#define ISP_CA_MAX_WINNUM           ISP_CA_W_WINNUM*ISP_CA_H_WINNUM
#define ISP_LA_W_WINNUM             32
#define ISP_LA_H_WINNUM             32
#define ISP_LA_MAX_WINNUM           ISP_LA_W_WINNUM*ISP_LA_H_WINNUM
#define ISP_LA_HIST_NUM             128
#define ISP_HISTO_MAX_SIZE          128
#define ISP_BUILTIN_DTSI_MAX        2
#define ISP_DFG_AIRLIGHT_NUM        3
#define ISP_SUBOUT_W_WINNUM         32
#define ISP_SUBOUT_H_WINNUM         32
#define ISP_SUBOUT_MAX_SIZE         (ISP_SUBOUT_W_WINNUM * ISP_SUBOUT_H_WINNUM)
#define SUB_NODE_LENGTH             64

// bridge usage
#define SENSOR_PRESET_NAME   0x454E5053 //MAKEFOURCC('S', 'P', 'N', 'E');
#define SENSOR_CHGMODE_FPS   0x53464353 //MAKEFOURCC('S', 'C', 'F', 'S');
#define SENSOR_PRESET_EXPT   0x54455053 //MAKEFOURCC('S', 'P', 'E', 'T');
#define SENSOR_PRESET_GAIN   0x4E475053 //MAKEFOURCC('S', 'P', 'G', 'N');
#define SENSOR_EXPT_MAX      0x584D4553 //MAKEFOURCC('S', 'E', 'M', 'X');
#define SENSOR_I2C_ID        0x44494953 //MAKEFOURCC('S', 'I', 'I', 'D');
#define SENSOR_I2C_ADDR      0x52414953 //MAKEFOURCC('S', 'I', 'A', 'R');
#define SENSOR_CUR_MODE      0x4D524353 //MAKEFOURCC('S', 'C', 'R', 'M');
#define SENSOR_ROW_TIME      0x54575253 //MAKEFOURCC('S', 'R', 'W', 'T');
#define SENSOR_VD            0x31445653 //MAKEFOURCC('S', 'V', 'D', '1');
#define SENSOR_DFT_FPS       0x46544453 //MAKEFOURCC('S', 'D', 'T', 'F');
#define SENSOR_FRAME_NUM     0x4E454653 //MAKEFOURCC('S', 'F', 'E', 'N');
#define SENSOR_MAX_GAIN      0x47584D53 //MAKEFOURCC('S', 'M', 'X', 'G');
#define ISP_PATH             0x31485049 //MAKEFOURCC('I', 'P', 'H', '1');
#define ISP_D_GAIN           0x4E474449 //MAKEFOURCC('I', 'D', 'G', 'N');
#define ISP_R_GAIN           0x4E475249 //MAKEFOURCC('I', 'R', 'G', 'N');
#define ISP_G_GAIN           0x4E474749 //MAKEFOURCC('I', 'G', 'G', 'N');
#define ISP_B_GAIN           0x4E474249 //MAKEFOURCC('I', 'B', 'G', 'N');
#define ISP_SHDR_ENABLE      0x45455349 //MAKEFOURCC('I', 'S', 'E', 'E');
#define ISP_SHDR_PATH        0x48505348 //MAKEFOURCC('I', 'S', 'P', 'H');
#define ISP_SHDR_MASK        0x4B4D5349 //MAKEFOURCC('I', 'S', 'M', 'K');
#define ISP_NR_LV            0x4C524E49 //MAKEFOURCC('I', 'N', 'R', 'L');
#define ISP_3DNR_LV          0x4C523349 //MAKEFOURCC('I', '3', 'R', 'L');
#define ISP_SHARPNESS_LV     0x4C535349 //MAKEFOURCC('I', 'S', 'S', 'L');
#define ISP_SATURATION_LV    0x4C4E5349 //MAKEFOURCC('I', 'S', 'N', 'L');
#define ISP_CONTRAST_LV      0x4C544349 //MAKEFOURCC('I', 'C', 'T', 'L');
#define ISP_BRIGHTNESS_LV    0x4C534248 //MAKEFOURCC('I', 'B', 'S', 'L');
#define ISP_NIGHT_MODE       0x4D544E49 //MAKEFOURCC('I', 'N', 'T', 'M');

#define SENSOR_PRESET_NAME_2 0x324E5053 //MAKEFOURCC('S', 'P', 'N', '2');
#define SENSOR_CHGMODE_FPS_2 0x32464353 //MAKEFOURCC('S', 'C', 'F', '2');
#define SENSOR_PRESET_EXPT_2 0x32455053 //MAKEFOURCC('S', 'P', 'E', '2');
#define SENSOR_PRESET_GAIN_2 0x32475053 //MAKEFOURCC('S', 'P', 'G', '2');
#define SENSOR_EXPT_MAX_2    0x324D4553 //MAKEFOURCC('S', 'E', 'M', '2');
#define SENSOR_I2C_ID_2      0x32494953 //MAKEFOURCC('S', 'I', 'I', '2');
#define SENSOR_I2C_ADDR_2    0x32414953 //MAKEFOURCC('S', 'I', 'A', '2');
#define SENSOR_CUR_MODE_2    0x32524353 //MAKEFOURCC('S', 'C', 'R', '2');
#define SENSOR_ROW_TIME_2    0x32575253 //MAKEFOURCC('S', 'R', 'W', '2');
#define SENSOR_VD_2          0x32445653 //MAKEFOURCC('S', 'V', 'D', '2');
#define SENSOR_DFT_FPS_2     0x32544453 //MAKEFOURCC('S', 'D', 'T', '2')
#define SENSOR_FRAME_NUM_2   0x32454653 //MAKEFOURCC('S', 'F', 'E', '2');
#define SENSOR_MAX_GAIN_2    0x32584D53 //MAKEFOURCC('S', 'M', 'X', '2');
#define ISP_PATH_2           0x32485049 //MAKEFOURCC('I', 'P', 'H', '2');
#define ISP_D_GAIN_2         0x32474449 //MAKEFOURCC('I', 'D', 'G', '2');
#define ISP_R_GAIN_2         0x32475249 //MAKEFOURCC('I', 'R', 'G', '2');
#define ISP_G_GAIN_2         0x32474749 //MAKEFOURCC('I', 'G', 'G', '2');
#define ISP_B_GAIN_2         0x32474249 //MAKEFOURCC('I', 'B', 'G', '2');
#define ISP_SHDR_ENABLE_2    0x32455349 //MAKEFOURCC('I', 'S', 'E', '2');
#define ISP_SHDR_PATH_2      0x32505348 //MAKEFOURCC('I', 'S', 'P', '2');
#define ISP_SHDR_MASK_2      0x324D5349 //MAKEFOURCC('I', 'S', 'M', '2');
#define ISP_NR_LV_2          0x32524E49 //MAKEFOURCC('I', 'N', 'R', '2');
#define ISP_3DNR_LV_2        0x32523349 //MAKEFOURCC('I', '3', 'R', '2');
#define ISP_SHARPNESS_LV_2   0x32535349 //MAKEFOURCC('I', 'S', 'S', '2');
#define ISP_SATURATION_LV_2  0x324E5349 //MAKEFOURCC('I', 'S', 'N', '2');
#define ISP_CONTRAST_LV_2    0x32544349 //MAKEFOURCC('I', 'C', 'T', '2');
#define ISP_BRIGHTNESS_LV_2  0x32534248 //MAKEFOURCC('I', 'B', 'S', '2');
#define ISP_NIGHT_MODE_2     0x32544E49 //MAKEFOURCC('I', 'N', 'T', '2');

typedef enum _ISP_BUILTIN_AE_TRIG_MSG {
	ISP_BUILTIN_AE_TRIG_RESET,
	ISP_BUILTIN_AE_TRIG_PROC,
	ISP_BUILTIN_AE_TRIG_MSG_MAX,
	ENUM_DUMMY4WORD(ISP_BUILTIN_AE_TRIG_MSG)
} ISP_BUILTIN_AE_TRIG_MSG;

typedef enum _ISP_BUILTIN_AWB_TRIG_MSG {
	ISP_BUILTIN_AWB_TRIG_RESET,
	ISP_BUILTIN_AWB_TRIG_PROC,
	ISP_BUILTIN_AWB_TRIG_MSG_MAX,
	ENUM_DUMMY4WORD(ISP_BUILTIN_AWB_TRIG_MSG)
} ISP_BUILTIN_AWB_TRIG_MSG;

typedef enum _ISP_BUILTIN_IQ_TRIG_MSG {
	ISP_BUILTIN_IQ_TRIG_SIE_RESET,
	ISP_BUILTIN_IQ_TRIG_IPP_RESET,
	ISP_BUILTIN_IQ_TRIG_SIE,
	ISP_BUILTIN_IQ_TRIG_IPP,
	ISP_BUILTIN_IQ_TRIG_CGAIN,
	ISP_BUILTIN_IQ_TRIG_ENC,
	ISP_BUILTIN_IQ_TRIG_MSG_MAX,
	ENUM_DUMMY4WORD(ISP_BUILTIN_IQ_TRIG_MSG)
} ISP_BUILTIN_IQ_TRIG_MSG;

typedef enum _ISP_BUILTIN_AE_STATUS {
	ISP_BUILTIN_AE_STATUS_STABLE     = 0,
	ISP_BUILTIN_AE_STATUS_FINE       = 1,
	ISP_BUILTIN_AE_STATUS_COARSE     = 2,
	ENUM_DUMMY4WORD(ISP_BUILTIN_AE_STATUS)
} ISP_BUILTIN_AE_STATUS;

typedef enum _ISP_BUILTIN_FUNC {
	ISP_BUILTIN_FUNC_NONE              =   0x00000000,
	ISP_BUILTIN_FUNC_WDR               =   0x00000001,
	ISP_BUILTIN_FUNC_SHDR              =   0x00000002,
	ISP_BUILTIN_FUNC_DEFOG             =   0x00000004,
	ISP_BUILTIN_FUNC_3DNR              =   0x00000008,
	ISP_BUILTIN_FUNC_DATASTAMP         =   0x00000010,
	ISP_BUILTIN_FUNC_PRIMASK           =   0x00000020,
	ISP_BUILTIN_FUNC_PM_PIXELIZTION    =   0x00000040,
	ISP_BUILTIN_FUNC_YUV_SUBOUT        =   0x00000080,
	ISP_BUILTIN_FUNC_VA_SUBOUT         =   0x00000100,
	ISP_BUILTIN_FUNC_3DNR_STA          =   0x00000200,
	ISP_BUILTIN_FUNC_GDC               =   0x00000400,
} ISP_BUILTIN_FUNC;

/**
	type for ISP_CA_RST
	ca result, array size should be ca window num_x * num_y
	user need to prepare memory for these pointer
	CA Output R/G/B/Cnt/IRth/Rth/Gth/Bth @16bit for each window
	each array size should be window num_x * num_y * 2(16bit)
*/
typedef struct _ISP_BUILTIN_CA_RSLT {
	UINT16 r[ISP_CA_MAX_WINNUM];
	UINT16 g[ISP_CA_MAX_WINNUM];
	UINT16 b[ISP_CA_MAX_WINNUM];
	UINT16 ir[ISP_CA_MAX_WINNUM];
	UINT16 acc_cnt[ISP_CA_MAX_WINNUM];
} ISP_BUILTIN_CA_RSLT;

/**
	type for ISP_LA_RST
	la result, array size should be la window num_x * num_y
	histogram array size = 64
	user need to prepare memory for these pointer
	p_buf_lum_1 buffer size: la_win_w * la_win_h * 2
	p_buf_lum_2 buffer size: la_win_w * la_win_h * 2
	p_buf_histogram buffer size: 64 * 2
*/
typedef struct _ISP_BUILTIN_LA_RSLT {
	UINT16 lum_1[ISP_LA_MAX_WINNUM];    // pre gamma result
	UINT16 lum_2[ISP_LA_MAX_WINNUM];    // post-gamma result
} ISP_BUILTIN_LA_RSLT;

/**
	type for KDRV_DCE_PARAM_IPL_HIST_RSLT
*/
typedef struct _ISP_BUILTIN_HISTO_RSLT {
	UINT16 hist_stcs_pre_wdr[ISP_HISTO_MAX_SIZE];           ///< histogram statistics
	UINT16 hist_stcs_post_wdr[ISP_HISTO_MAX_SIZE];          ///< histogram statistics
} ISP_BUILTIN_HISTO_RSLT;

/**
	type for KDRV_IPE_PARAM_IPL_DEFOG_STCS_RSLT
*/
typedef struct _ISP_BUILTIN_DEFOG_STCS {
	UINT16 dfg_airlight[ISP_DFG_AIRLIGHT_NUM];         ///< the defog statistics
} ISP_BUILTIN_DEFOG_STCS;

typedef struct _ISP_BUILTIN_IPE_SUBOUT_BUF {
	UINT16 min[ISP_SUBOUT_MAX_SIZE];     ///< Legal range : 0~1023, the min value of block in subout image
	UINT16 avg[ISP_SUBOUT_MAX_SIZE];     ///< Legal range : 0~1023, the avg value of block in subout image
} ISP_BUILTIN_IPE_SUBOUT_BUF;

typedef struct _ISP_BUILTIN_DTSI {
	CHAR iq_front_node_path[SUB_NODE_LENGTH];
	CHAR iq_node_path[SUB_NODE_LENGTH];
	CHAR iq_dpc_node_path[SUB_NODE_LENGTH];
	CHAR iq_shading_node_path[SUB_NODE_LENGTH];
	CHAR ae_node_path[SUB_NODE_LENGTH];
	CHAR awb_node_path[SUB_NODE_LENGTH];
} ISP_BUILTIN_DTSI;

//=============================================================================
// extern functions
//=============================================================================
#if (ISP_BUILTIN_MEASURE_DTSI)
extern void nvt_bootts_add_ts(char *name);
#endif
extern ISP_BUILTIN_FUNC isp_builtin_get_func_en(UINT32 id);
extern ISP_BUILTIN_CA_RSLT *isp_builtin_get_ca(UINT32 id);
extern ISP_BUILTIN_LA_RSLT *isp_builtin_get_la(UINT32 id);
extern ISP_BUILTIN_HISTO_RSLT *isp_builtin_get_histo(UINT32 id);
extern ISP_BUILTIN_DEFOG_STCS *isp_builtin_get_defog_stcs(UINT32 id);
extern ISP_BUILTIN_IPE_SUBOUT_BUF *isp_builtin_get_ipe_subout(UINT32 id);
extern UINT32 isp_builtin_get_scene_chg_w(UINT32 id);
extern UINT32 isp_builtin_get_ae_status(UINT32 id);
extern UINT32 isp_builtin_get_sensor_row_time(UINT32 id);
extern UINT32 isp_builtin_get_ae_msg_cnt(void);
extern UINT32 isp_builtin_get_awb_msg_cnt(void);
extern UINT32 isp_builtin_get_sensor_mode_type(UINT32 id);
extern UINT32 isp_builtin_get_fastboot_version(void);
extern ISP_BUILTIN_DTSI *isp_builtin_get_dtsi(UINT32 id);
extern void isp_builtin_set_total_gain(UINT32 id, UINT32 value);
extern void isp_builtin_set_ct(UINT32 id, UINT32 value);
extern void isp_builtin_set_lv(UINT32 id, UINT32 value);
extern void isp_builtin_set_shdr_ev_ratio(UINT32 id, ISP_BUILTIN_SHDR_EV_RATIO *value);
extern void isp_builtin_set_shdr_smoothed_tm_ratio(UINT32 id, UINT32 value);
extern void isp_builtin_set_overexposure_offset(UINT32 id, UINT32 value);
extern void isp_builtin_set_compensation_ratio(UINT32 id, UINT32 value);
extern void isp_builtin_set_scene_chg_w(UINT32 id, UINT32 value);
extern void isp_builtin_set_ae_status(UINT32 id, ISP_BUILTIN_AE_STATUS value);
extern void isp_builtin_set_dgain(UINT32 id, UINT32 value);
extern void isp_builtin_set_sensor_gain(UINT32 id, ISP_BUILTIN_SENSOR_CTRL *value);
extern void isp_builtin_set_sensor_expt(UINT32 id, ISP_BUILTIN_SENSOR_CTRL *value);
extern void *isp_builtin_kmem_alloc(UINT32 mem_size);
extern void isp_builtin_kmem_free(void *mem_addr);
extern void *isp_builtin_vmem_alloc(UINT32 mem_size);
extern void isp_builtin_vmem_free(void *mem_addr);
#if defined(__KERNEL__)
extern ER isp_builtin_init(void);
#endif
extern ER isp_builtin_uninit(void);
#endif

