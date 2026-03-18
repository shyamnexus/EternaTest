#ifndef __KDRV_IPP_BUILTIN_INT_H__
#define __KDRV_IPP_BUILTIN_INT_H__

#include "kwrap/spinlock.h"
#include "kwrap/list.h"
#include "kwrap/task.h"

#include "kwrap/cpu.h"
#include "kwrap/perf.h"
#include "kdrv_type.h"
#include "kdrv_ipp_builtin.h"
#include "kdrv_ipp_builtin_dbg.h"
#include "kdrv_ipp_builtin_drawstr.h"
#include "kwrap/flag.h"
#include "kdrv_ipp_builtin_dbg_info.h"

/**
	IFE Register
*/
#define REG_OFS_IFE_CTRL                  (0x0)
#define REG_OFS_IFE_FUNCTION_REG          (0x04)
#define REG_OFS_IFE_DRAM_SAILL            (0x14)
#define REG_OFS_IFE_DRAM_SAI0             (0x30)
#define REG_OFS_IFE_DRAM_SAI1             (0x38)
#define REG_OFS_IFE_DRAM_SINGLE_OUT_CTRL  (0x60)
#define REG_OFS_IFE_HISTOGRAM_REG         (0xAB8)
#define REG_OFS_IFE_WDR_CRTL_REG          (0x900)
#define REG_OFS_IFE_WDR_SUBIMG_SAI        (0x908)
#define REG_OFS_IFE_WDR_SUBIMG_SAO        (0x910)

#define REG_VAL_IFE_START               (0x2)
#define REG_VAL_IFE_LOAD_START          (0x4)
#define REG_VAL_IFE_LOAD_FRMSTART       (0x10)
#define REG_VAL_IFE_OUT1_SINGLE_EN      (0x2)
#define REG_VAL_IFE_MIRROR_EN           (0x40000000)
#define REG_VAL_IFE_LL_FIRE             (0x10000000)
#define REG_VAL_IFE_OUT_MODE            (0x80000000)
#define REG_VAL_IFE_WDR_EN              (0x01)
#define REG_VAL_IFE_WDR_SUBIMG_OUT_EN   (0x02)

#define REG_BIT_IFE_BAYER_FORMAT        (21)
#define REG_BIT_IFE_CFAPAT              (9)

#define REG_MASK_IFE_BAYER_FORMAT       (0x01)
#define REG_MASK_IFE_CFAPAT             (0x7)


/**
	IPE Register
*/
#define REG_OFS_IPE_CTRL                    (0x0)
#define REG_OFS_IPE_MODE_REG1               (0x08)
#define REG_OFS_IPE_DMA_REG0                (0x0C)
#define REG_OFS_IPE_DRAM_SAI_LL             (0x1C)
#define REG_OFS_IPE_DRAM_SAO_VA             (0x48)
#define REG_OFS_IPE_DMA_DEFOG_SUBIMG_IN_CH  (0x314)
#define REG_OFS_IPE_DMA_DEFOG_SUBIMG_OUT_CH (0x31C)

#define REG_VAL_IPE_START                (0x2)
#define REG_VAL_IPE_LODAD_START          (0x4)
#define REG_VAL_IPE_LOAD_FRMSTART        (0x10)
#define REG_VAL_IPE_DRAM_OUT3_SINGLE_EN  (0x8)
#define REG_VAL_IPE_DRAM_OUT2_SINGLE_EN  (0x4)
#define REG_VAL_IPE_DRAM_OUT_MODE        (0x80)
#define REG_VAL_DEFOG_SUBIMG_OUT_EN      (0x10)
#define REG_VAL_IPE_RGBGAMMA_EN          (0x02)
#define REG_VAL_IPE_YCURVE_EN            (0x04)
#define REG_VAL_IPE_LCE_EN               (0x80)
#define REG_VAL_IPE_LL_FIRE              (0x10000000)
/**
	IME Register
*/
#define KDRV_IPP_BUILTIN_PRI_MASK_REG_0       (0x4D0)
#define KDRV_IPP_BUILTIN_PRI_MASK_REG_1       (0x564)
#define KDRV_IPP_BUILTIN_PRI_MASK_REG_0_SET2  (0x518)
#define KDRV_IPP_BUILTIN_PRI_MASK_REG_0_SET4  (0x564)
#define KDRV_IPP_BUILTIN_PRI_MASK_REG_0_SET6  (0x5AC)

#define KDRV_IPP_BUILTIN_PRI_MASK_ALPHA_REG_0 (0x560)
#define KDRV_IPP_BUILTIN_PRI_MASK_ALPHA_REG_1 (0x5F4)
#define KDRV_IPP_BUILTIN_PRI_MASK_REG_OFS     (0x24)

#define REG_OFS_IME_CTRL                        (0x00)
#define REG_OFS_IME_FUNCTION_REG0               (0x04)
#define REG_OFS_IME_FUNCTION_REG1               (0x08)
#define REG_OFS_IME_DRAM_SINGLE_OUT_CTRL        (0x0C)
#define REG_OFS_IME_DRAM_LL_SAI                 (0x14)
#define REG_OFS_IME_INTERRUPT_EN                (0x18)
#define REG_OFS_IME_INPUT_IMAGE_SIZE            (0x20)
#define REG_OFS_IME_DRAM_END_OUTPUT_STATUS_REG0 (0x64)
#define REG_OFS_IME_DRAM_END_OUTPUT_STATUS_REG1 (0x68)
#define REG_OFS_IME_PM_Y_SAI                    (0x600)
#define REG_OFS_IME_LL_TAB_0                    (0x650)
#define REG_OFS_IME_LL_TAB_1                    (0x654)
#define REG_OFS_IME_LL_TAB_2                    (0x658)
#define REG_OFS_IME_LL_TAB_3                    (0x65C)
#define REG_OFS_IME_PM_PXL_DRAM_SAO             (0x360)
#define REG_OFS_IME_3DNR_REF_DRAM_Y_SAI         (0x908)
#define REG_OFS_IME_3DNR_REF_DRAM_Y_SAO         (0x918)
#define REG_OFS_IME_3DNR_REF_DRAM_UV_SAO        (0x91C)
#define REG_OFS_IME_3DNR_REF_DRAM_UV_SAI        (0x90C)
#define REG_OFS_IME_3DNR_MOT_DRAM_SAI           (0x924)
#define REG_OFS_IME_3DNR_MOT_DRAM_SAO           (0x92C)
#define REG_OFS_IME_3DNR_MV_DRAM_SAI            (0x93C)
#define REG_OFS_IME_3DNR_MV_DRAM_SAO            (0x944)
#define REG_OFS_IME_3DNR_FAST_CONVERGE_DRAM_SAI (0x950)
#define REG_OFS_IME_3DNR_FAST_CONVERGE_DRAM_SAO (0x958)
#define REG_OFS_IME_P0_Y_DRAM_SAO               (0xB8)
#define REG_OFS_IME_P0_U_DRAM_SAO               (0xBC)
#define REG_OFS_IME_P0_V_DRAM_SAO               (0xC0)
#define REG_OFS_IME_P1_Y_DRAM_SAO               (0x160)
#define REG_OFS_IME_P1_UV_DRAM_SAO              (0x164)
#define REG_OFS_IME_P2_Y_DRAM_SAO               (0x210)
#define REG_OFS_IME_P2_UV_DRAM_SAO              (0x214)

//#define REG_OFS_IME_VA_DRAM_SAO                 (0xA04) //538 removed

#define REG_OFS_IME_PM_SET0_LINE0_REG1          (0x4D4)
#define REG_OFS_IME_PM_SET0_LINE0_REG2          (0x4D8)
#define REG_OFS_IME_PM_SET1_LINE0_REG1          (0x4F8)
#define REG_OFS_IME_PM_SET1_LINE0_REG2          (0x4FC)
#define REG_OFS_IME_PM_SET2_LINE0_REG1          (0x51C)
#define REG_OFS_IME_PM_SET2_LINE0_REG2          (0x520)
#define REG_OFS_IME_PM_SET3_LINE0_REG1          (0x540)
#define REG_OFS_IME_PM_SET3_LINE0_REG2          (0x544)
#define REG_OFS_IME_PM_SET4_LINE0_REG1          (0x568)
#define REG_OFS_IME_PM_SET4_LINE0_REG2          (0x56C)
#define REG_OFS_IME_PM_SET5_LINE0_REG1          (0x58C)
#define REG_OFS_IME_PM_SET5_LINE0_REG2          (0x590)
#define REG_OFS_IME_PM_SET6_LINE0_REG1          (0x5B0)
#define REG_OFS_IME_PM_SET6_LINE0_REG2          (0x5B4)
#define REG_OFS_IME_PM_SET7_LINE0_REG1          (0x5D4)
#define REG_OFS_IME_PM_SET7_LINE0_REG2          (0x5D8)

#define REG_OFS_IME_PM_SET0_LINE1_REG1          (0x4DC)
#define REG_OFS_IME_PM_SET0_LINE1_REG2          (0x4E0)
#define REG_OFS_IME_PM_SET1_LINE1_REG1          (0x500)
#define REG_OFS_IME_PM_SET1_LINE1_REG2          (0x504)
#define REG_OFS_IME_PM_SET2_LINE1_REG1          (0x524)
#define REG_OFS_IME_PM_SET2_LINE1_REG2          (0x528)
#define REG_OFS_IME_PM_SET3_LINE1_REG1          (0x548)
#define REG_OFS_IME_PM_SET3_LINE1_REG2          (0x54C)
#define REG_OFS_IME_PM_SET4_LINE1_REG1          (0x570)
#define REG_OFS_IME_PM_SET4_LINE1_REG2          (0x574)
#define REG_OFS_IME_PM_SET5_LINE1_REG1          (0x594)
#define REG_OFS_IME_PM_SET5_LINE1_REG2          (0x598)
#define REG_OFS_IME_PM_SET6_LINE1_REG1          (0x5B8)
#define REG_OFS_IME_PM_SET6_LINE1_REG2          (0x5BC)
#define REG_OFS_IME_PM_SET7_LINE1_REG1          (0x5DC)
#define REG_OFS_IME_PM_SET7_LINE1_REG2          (0x5E0)

#define REG_OFS_IME_PM_SET0_LINE2_REG1          (0x4E4)
#define REG_OFS_IME_PM_SET0_LINE2_REG2          (0x4E8)
#define REG_OFS_IME_PM_SET1_LINE2_REG1          (0x508)
#define REG_OFS_IME_PM_SET1_LINE2_REG2          (0x50C)
#define REG_OFS_IME_PM_SET2_LINE2_REG1          (0x52C)
#define REG_OFS_IME_PM_SET2_LINE2_REG2          (0x530)
#define REG_OFS_IME_PM_SET3_LINE2_REG1          (0x550)
#define REG_OFS_IME_PM_SET3_LINE2_REG2          (0x554)
#define REG_OFS_IME_PM_SET4_LINE2_REG1          (0x578)
#define REG_OFS_IME_PM_SET4_LINE2_REG2          (0x57C)
#define REG_OFS_IME_PM_SET5_LINE2_REG1          (0x59C)
#define REG_OFS_IME_PM_SET5_LINE2_REG2          (0x5A0)
#define REG_OFS_IME_PM_SET6_LINE2_REG1          (0x5C0)
#define REG_OFS_IME_PM_SET6_LINE2_REG2          (0x5C4)
#define REG_OFS_IME_PM_SET7_LINE2_REG1          (0x5E4)
#define REG_OFS_IME_PM_SET7_LINE2_REG2          (0x5E8)

#define REG_OFS_IME_PM_SET0_LINE3_REG1          (0x4EC)
#define REG_OFS_IME_PM_SET0_LINE3_REG2          (0x4F0)
#define REG_OFS_IME_PM_SET1_LINE3_REG1          (0x510)
#define REG_OFS_IME_PM_SET1_LINE3_REG2          (0x514)
#define REG_OFS_IME_PM_SET2_LINE3_REG1          (0x534)
#define REG_OFS_IME_PM_SET2_LINE3_REG2          (0x538)
#define REG_OFS_IME_PM_SET3_LINE3_REG1          (0x558)
#define REG_OFS_IME_PM_SET3_LINE3_REG2          (0x55C)
#define REG_OFS_IME_PM_SET4_LINE3_REG1          (0x580)
#define REG_OFS_IME_PM_SET4_LINE3_REG2          (0x584)
#define REG_OFS_IME_PM_SET5_LINE3_REG1          (0x5A4)
#define REG_OFS_IME_PM_SET5_LINE3_REG2          (0x5A8)
#define REG_OFS_IME_PM_SET6_LINE3_REG1          (0x5C8)
#define REG_OFS_IME_PM_SET6_LINE3_REG2          (0x5CC)
#define REG_OFS_IME_PM_SET7_LINE3_REG1          (0x5EC)
#define REG_OFS_IME_PM_SET7_LINE3_REG2          (0x5F0)

#define REG_OFS_IME_PM_SET0_LINE4_REG1          (0x490)
#define REG_OFS_IME_PM_SET0_LINE4_REG2          (0x494)
#define REG_OFS_IME_PM_SET2_LINE4_REG1          (0x4A0)
#define REG_OFS_IME_PM_SET2_LINE4_REG2          (0x4A4)
#define REG_OFS_IME_PM_SET4_LINE4_REG1          (0x4B0)
#define REG_OFS_IME_PM_SET4_LINE4_REG2          (0x4B4)
#define REG_OFS_IME_PM_SET6_LINE4_REG1          (0x4C0)
#define REG_OFS_IME_PM_SET6_LINE4_REG2          (0x4C4)

#define REG_OFS_IME_PM_SET0_LINE5_REG1          (0x498)
#define REG_OFS_IME_PM_SET0_LINE5_REG2          (0x49c)
#define REG_OFS_IME_PM_SET2_LINE5_REG1          (0x4A8)
#define REG_OFS_IME_PM_SET2_LINE5_REG2          (0x4AC)
#define REG_OFS_IME_PM_SET4_LINE5_REG1          (0x4B8)
#define REG_OFS_IME_PM_SET4_LINE5_REG2          (0x4BC)
#define REG_OFS_IME_PM_SET6_LINE5_REG1          (0x4C8)
#define REG_OFS_IME_PM_SET6_LINE5_REG2          (0x4CC)


#define REG_VAL_IME_START                       (0x2)
#define REG_VAL_IME_START_LOAD                  (0x4)
#define REG_VAL_IME_LOAD_FRMSTART               (0x10)
#define REG_VAL_IME_IME_P0_EN                   (0x4)
#define REG_VAL_IME_LCA_EN                      (0x400)
#define REG_VAL_IME_PM_PXL_SUBOUT_EN            (0x80000)
#define REG_VAL_IME_3DNR_EN                     (0x8000000)
#define REG_VAL_IME_3DNR_REF_OUT_EN             (0x40000000)
#define REG_VAL_IME_DRAM_OUT_MODE               (0x80000000)
#define REG_VAL_IME_PXL_SUBOUT_DRAM_OUT_SINGLE_EN  (0x01)
#define REG_VAL_IME_3DNR_REFOUT_DRAM_OUT_SINGLE_EN (0x02)
#define REG_VAL_IME_3DNR_MV_DRAM_OUT_SINGLE_EN     (0x10)
#define REG_VAL_IME_3DNR_MS_DRAM_OUT_SINGLE_EN     (0x04)
#define REG_VAL_IME_OUT_P0_DRAM_OUT_SINGLE_EN      (0x40)
#define REG_VAL_IME_3DNR_FC_DRAM_OUT_SINGLE_EN     (0x400)
//#define REG_VAL_IME_VA_DRAM_OUT_SINGLE_EN          (0x800) //538 removed
#define REG_VAL_IME_INTE_LL_END                    (0x01)
#define REG_VAL_IME_INTE_LL_JOB_END                (0x08)
#define REG_VAL_IME_INTE_FRM_END                   (0x80000000)
#define REG_VAL_IME_INTE_ALL                       (0xFFFFFFFF)
#define REG_VAL_IME_3DNR_INREF_FLIP_EN             (0x10000)
#define REG_VAL_IME_P0_FLIP_EN                     (0x1000)
#define REG_VAL_IME_3DNR_OUTREF_FLIP_EN            (0x20000)
#define REG_VAL_IME_LL_FIRE                        (0x10000000)
#define REG_VAL_IME_PM0_COMB_MODE_SEL_MASK         (0xc0000000)
#define REG_VAL_IME_PM0_COMB_MODE_SEL_DISABLE      (0x00000000)
#define REG_VAL_IME_PM0_COMB_MODE_SEL_HOLLOW       (0x40000000)
#define REG_VAL_IME_PM0_COMB_MODE_SEL_OCT          (0x80000000)
#define REG_VAL_IME_PM0_COMB_MODE_SEL_DEC          (0xc0000000)
#define REG_VAL_IME_PM0_SHAPE_MODE_SEL_MASK        (0x80000000)
#define REG_VAL_IME_PM0_SHAPE_MODE_QUAD            (0x00000000)
#define REG_VAL_IME_PM0_SHAPE_MODE_HEX             (0x80000000)

#define SSDRV_BUILTIN_IPP (0)

#define KDRV_IPP_RTOS_IPP_ENABLE (1)
#define KDRV_IPP_RTOS_YUV_PRE_ROLL_EN (0)
#define KDRV_IPP_BUILTIN_MSG_ENABLE (0)
#if KDRV_IPP_BUILTIN_MSG_ENABLE
#define KDRV_IPP_BUILTIN_DUMP(fmt, args...) DBG_DUMP(fmt, ##args)
#else
#define KDRV_IPP_BUILTIN_DUMP(fmt, args...)
#endif

#define KDRV_IPP_BUILTIN_ENG_BIT(eng) (1 << eng)


#define KDRV_IPP_BUILTIN_BUF_SIE 2

#define KDRV_IPP_BUILTIN_FMD_CB_NUM	(3)

#define RTOS_QUEUE_NUM_MAX (16)
#define IFE_LL_CMD_NUM (4+1)     // 1 (wdr in) + 1 (wdr out) + 1 (single out) + 1 (frame start load) + 1 (next ll)
#define DCE_LL_CMD_NUM (4+1)     // 1 (wdr in) + 1 (wdr out) + 1 (single out) + 1 (frame start load) + 1 (next ll)
#define IPE_LL_CMD_NUM (4+1)     // 1 (defog inr) + 1 (defog out) + 1 (single out) + 1 (frame start load) + 1 (next ll)
#define IME_LL_CMD_NUM (19+1)    // 9 (out addr) + 4 (sub-in) + 4 (sub-out) + 1(single out) + 1 (frame start load) + 1 (next ll)

#define KDRV_IPP_BUILTIN_LL_CMD_TYPE_NULL	(0)
#define KDRV_IPP_BUILTIN_LL_CMD_TYPE_NXTLL	(1)
#define KDRV_IPP_BUILTIN_LL_CMD_TYPE_NXTUPD	(2)
#define KDRV_IPP_BUILTIN_LL_CMD_TYPE_UPD	(4)

typedef union {
	struct {
		UINT64 reg_val			: 32;	/* bits : 31_0 */
		UINT64 reg_ofs			: 12;	/* bits : 43_32 */
		UINT64 byte_en			: 4;	/* bits : 47_44 */
		UINT64					: 13;
		UINT64 cmd				: 3;	/* bits : 63_61 */
	} upd_bit;

	struct {
		UINT64 table_index		: 8;	/* bits : 7_0 */
		UINT64					: 53;
		UINT64 cmd				: 3;	/* bits : 63_61 */
	} null_bit;

	struct {
		UINT64 table_index		: 8;	/* bits : 7_0 */
		UINT64 next_ll_addr		: 32;	/* bits : 39_8 */
		UINT64					: 21;
		UINT64 cmd				: 3;	/* bits : 63_61 */
	} nxtll_bit;

	struct {
		UINT64					: 8;	/* bits : 7_0 */
		UINT64 next_upd_addr	: 32;	/* bits : 39_8 */
		UINT64					: 21;
		UINT64 cmd				: 3;	/* bits : 63_61 */
	} nxtupd_bit;

	UINT64 val;
} KDRV_IPP_BUILTIN_LL_CMD;

/* CTL_IPP_FLOW_TYPE */
typedef enum {
	KDRV_IPP_BUILTIN_FLOW_UNKNOWN = 0,
	KDRV_IPP_BUILTIN_FLOW_RAW,
	KDRV_IPP_BUILTIN_FLOW_DIRECT_RAW,
	KDRV_IPP_BUILTIN_FLOW_CCIR,
	KDRV_IPP_BUILTIN_FLOW_DIRECT_CCIR,
	KDRV_IPP_BUILTIN_FLOW_IME_D2D,
	KDRV_IPP_BUILTIN_FLOW_IPE_D2D,
	KDRV_IPP_BUILTIN_FLOW_VR360,
	KDRV_IPP_BUILTIN_FLOW_DCE_D2D,
	KDRV_IPP_BUILTIN_FLOW_CAPTURE_RAW,
	KDRV_IPP_BUILTIN_FLOW_CAPTURE_CCIR,
	KDRV_IPP_BUILTIN_FLOW_MAX
} KDRV_IPP_BUILTIN_FLOW_TYPE;

#define KDRV_IPP_BUILTIN_FRC_BASE (4096)

typedef struct{

	UINT8 set_bit;
	UINT8 expected_bit[2];
	UINT8 w_idx;
	UINT8 r_idx;

}KDRV_IPP_BUILTIN_APPLY_CHK_CTL;

typedef struct{
	KDRV_IPP_BUILTIN_LL_CMD ll_cmd[ IFE_LL_CMD_NUM * RTOS_QUEUE_NUM_MAX ];
	UINT32 cmd_idx;
}KDRV_IPP_BUILTIN_IFE_LL_CMD;

typedef struct{
	KDRV_IPP_BUILTIN_LL_CMD ll_cmd[ DCE_LL_CMD_NUM * RTOS_QUEUE_NUM_MAX ];
	UINT32 cmd_idx;
}KDRV_IPP_BUILTIN_DCE_LL_CMD;

typedef struct{
	KDRV_IPP_BUILTIN_LL_CMD ll_cmd[ IPE_LL_CMD_NUM * RTOS_QUEUE_NUM_MAX ];
	UINT32 cmd_idx;
}KDRV_IPP_BUILTIN_IPE_LL_CMD;

typedef struct{
	KDRV_IPP_BUILTIN_LL_CMD ll_cmd[ IME_LL_CMD_NUM * RTOS_QUEUE_NUM_MAX ];
	UINT32 cmd_idx;
}KDRV_IPP_BUILTIN_IME_LL_CMD;

typedef struct {
	const void *p_fdt;
	int       root_nodeoffset;
	int       child_nodeoffset;
} KDRV_IPP_BUILTIN_DTSI_NODE_INFO;

typedef struct {
	const CHAR *name;
	CHAR node[64];
	KDRV_IPP_BUILTIN_DTSI_NODE_INFO root_node;

	UINT32 src_sie_id_bit;
	UINT32 flow;
	UINT32 queue_mode_en[KDRV_IPP_BUILTIN_PATH_ID_MAX];
	UINT32 queue_ring_mode_en;
	UINT32 queue_h264_mode_en;
	UINT32 queue_num[KDRV_IPP_BUILTIN_PATH_ID_MAX];
	UINT32 queue_in_idx[KDRV_IPP_BUILTIN_PATH_ID_MAX];
	UINT32 queue_out_idx[KDRV_IPP_BUILTIN_PATH_ID_MAX];
	UINT32 buf_num_in_que[KDRV_IPP_BUILTIN_PATH_ID_MAX];
	UINT32 yuv_frc_drop[KDRV_IPP_BUILTIN_PATH_ID_MAX];
	UINT32 yuv_str_cnt[KDRV_IPP_BUILTIN_PATH_ID_MAX]; //fb2.0
	UINT32 yuv_str_idx[KDRV_IPP_BUILTIN_PATH_ID_MAX]; //fb2.0
	UINT32 isp_id;

	UINT32 *p_dtsi_ife_buf;					/* buffer to read dtsi array */
	UINT32 *p_dtsi_ipe_buf;					/* buffer to read dtsi array */
	UINT32 *p_dtsi_ime_buf;					/* buffer to read dtsi array */

	//KDRV_IPP_BUILTIN_BLK vprc_blk[KDRV_IPP_BUILTIN_PATH_ID_MAX][2 + RTOS_QUEUE_NUM_MAX];
	KDRV_IPP_BUILTIN_BLK vprc_blk[KDRV_IPP_BUILTIN_PATH_ID_MAX][KDRV_IPP_BUILTIN_BUF_NUM_MAX];
	KDRV_IPP_BUILTIN_BLK vprc_shdr_blk[KDRV_IPP_BUILTIN_HDR_FRM_MAX-1];
	KDRV_IPP_BUILTIN_BLK vprc_ctrl;
	KDRV_IPP_BUILTIN_PRI_BUF pri_buf;
	KDRV_IPP_BUILTIN_FUNC func_en;
	KDRV_IPP_BUILTIN_DBG_FUNC dbg_func_en;
	KDRV_IPP_BUILTIN_PHY_OUT_INFO last_out_info;
	UINT32 _3dnr_ref_path;
	UINT32 fs_cnt;
	UINT32 fs_cnt_yuv_que;
	UINT32 fed_cnt;
	UINT32 job_end_cnt;
	VOS_TICK fs_timestamp;
	VOS_TICK pre_fs_timestamp;
	VOS_TICK switch_timestamp;
	UINT8 builtin_ipp_frm_st_start;
	UINT8 builtin_ipp_frm_ed_start;
	KDRV_IPP_BUILTIN_IMG_INFO path_info[KDRV_IPP_BUILTIN_PATH_ID_MAX];
	void *p_cur_job;
	UINT8 in_buf_release_cnt;
	UINT8 out_buf_release_cnt;
	UINT8 get_phy_out_cnt;

	KDRV_IPP_BUILTIN_RATE_CTL frc[KDRV_IPP_BUILTIN_PATH_ID_MAX];
	KDRV_IPP_BUILTIN_PRI_MASK mask[KDRV_IPP_BUILTIN_PRI_MASK_NUM];

	KDRV_IPP_BUILTIN_FMD_CB fmd_cb[KDRV_IPP_BUILTIN_FMD_CB_NUM];

	KDRV_IPP_BUILTIN_DBG_INFO dbg_info;
	/* for yuv func */
	ULONG _3dnr_lock_addr;
	UINT8 _3dnr_unluckable_flag;
	UINT32 lock_cnt;
	UINT8  ring_flag[KDRV_IPP_BUILTIN_PATH_ID_MAX];
	UINT8 input_disable_flag;
	KDRV_IPP_BUILTIN_APPLY_CHK_CTL ime_dram_end_ck;
	UINT8 get_isp_info_cnt;
	KDRV_IPP_BUILTIN_ISP_INFO isp_info;
	UINT32 hdr_frm_num; //number of HDR frame

	KDRV_IPP_BUILTIN_IFE_LL_CMD ife_ll;
	KDRV_IPP_BUILTIN_DCE_LL_CMD dce_ll;
	KDRV_IPP_BUILTIN_IPE_LL_CMD ipe_ll;
	KDRV_IPP_BUILTIN_IME_LL_CMD ime_ll;

	UINT32 mirror;
	UINT32 hdal_mirror;
	UINT32 flip[KDRV_IPP_BUILTIN_PATH_ID_MAX];
	UINT8  _3dnr_ref_flip;

	UINT32 output_queue_num;
	UINT32 ll_queue_num;
	UINT32 rtos_queue_num;
	UINT8 pre_roll_flg;

	UINT32 target_reg;
	UINT32 target_byte;
	UINT32 target_position;
	BOOL switch_start_flg;

	UINT32  stitch_en[KDRV_IPP_BUILTIN_PATH_ID_MAX];
	UINT32 stitch_buf_y_size[KDRV_IPP_BUILTIN_PATH_ID_MAX];
	UINT32 stitch_buf_uv_size[KDRV_IPP_BUILTIN_PATH_ID_MAX];
	UINT32 stitch_addr_y_ofs[KDRV_IPP_BUILTIN_PATH_ID_MAX];
	UINT32 stitch_addr_uv_ofs[KDRV_IPP_BUILTIN_PATH_ID_MAX];
	UINT32 stitch_img_width[KDRV_IPP_BUILTIN_PATH_ID_MAX];
	UINT32 stitch_img_height[KDRV_IPP_BUILTIN_PATH_ID_MAX];

	UINT8 frm_done;
	UINT8 stitch_frm_sync_flg;

	BOOL rtos_ipp_en;
	UINT32 rtos_frm_cnt;
	BOOL frm_st_flg;

	UINT32 sensor_fps;
	UINT32 frm_interval_us;

} KDRV_IPP_BUILTIN_HDL;

typedef struct{

	KDRV_IPP_BUILTIN_HDL *p_hdl;
	KDRV_IPP_BUILTIN_DRAW_BUF draw_buf_info;
	BOOL frm_skip_flag;

}KDRV_IPP_BUILTIN_DBG_DRAW_INFO;

typedef struct {
	/* todo: check sie push job struct */
	KDRV_IPP_BUILTIN_HDL *p_owner;
	ULONG blk[2];		/* blk_addr, used for release buffer */
	ULONG blk_2[2];		/* blk_addr, used for release buffer */
	ULONG va[3];		/* va[0]: raw/y, va[1]: raw/uv */
	ULONG va_2[3];		/* va[0]: raw/y, va[1]: raw/uv */
	ULONG pa[3];		/* pa[0]: raw/y, pa[1]: raw/uv */
	ULONG pa_2[3];		/* pa[0]: raw/y, pa[1]: raw/uv */
	UINT32 buf_ctrl;	/* refer to SIE_BUILTIN_HEADER_CTL_LOCK/SIE_BUILTIN_HEADER_CTL_PUSH */
	UINT32 timestamp;
	UINT32 count;
	struct vos_list_head pool_list;
	struct vos_list_head proc_list;
} KDRV_IPP_BUILTIN_JOB;
typedef struct {
	ULONG start_addr;
	UINT32 total_size;
	UINT32 blk_size;
	UINT8 blk_num;
	vk_spinlock_t lock;
	struct vos_list_head free_list_root;
	struct vos_list_head used_list_root;
} KDRV_IPP_BUILTIN_MEM_POOL;
typedef struct {
	void *reg_base[KDRV_IPP_BUILTIN_ENG_MAX];
	KDRV_IPP_BUILTIN_FMD_CB fmd_cb[KDRV_IPP_BUILTIN_FMD_CB_NUM];
	UINT32 dtsi_ver;	/* 0: old version for one direct mode, 1: new version for multi handle */
	UINT32 hdl_num;
	ULONG _3dnr_lock_addr; //tmp new method need to take care

	KDRV_IPP_BUILTIN_HDL *p_hdl;
	KDRV_IPP_BUILTIN_HDL *p_trig_hdl;	/* hdl that been trig start */
	UINT8 is_init;
	UINT8 is_fastboot;

	KDRV_IPP_BUILTIN_MEM_POOL job_pool;
	THREAD_HANDLE proc_tsk_id;
	THREAD_HANDLE ll_switch_tsk_id;
	ID proc_tsk_flg_id;
	ID linked_list_flg_id;
	vk_spinlock_t job_list_lock;
	vk_spinlock_t sie_cb_lock;
	struct vos_list_head job_list_root;
	UINT32 valid_src_id_bit;
	UINT8 ll_free_run_flg;
	UINT8 ll_triggered;
	UINT8 frm_close_flg;

	KDRV_IPP_BUILTIN_BLK stitch_buf[KDRV_IPP_BUILTIN_PATH_ID_MAX][2];

	KDRV_IPP_BUILTIN_HDL* p_frm_sync_master;
	KDRV_IPP_BUILTIN_HDL* p_frm_sync_slave;

	BOOL rtos_exit_flg;
	BOOL get_frm_idx_flg;

} KDRV_IPP_BUILTIN_CTL;

INT32 kdrv_ipp_builtin_plat_init_clk(void);
void* kdrv_ipp_builtin_plat_ioremap_nocache(ULONG addr, UINT32 size);
void kdrv_ipp_builtin_plat_iounmap(void *addr);
INT32 kdrv_ipp_builtin_get_node_offset(CHAR *path, UINT32* nodeoffset);
INT32 kdrv_ipp_builtin_get_root_node_info(CHAR *path, KDRV_IPP_BUILTIN_DTSI_NODE_INFO* root_node);
INT32 kdrv_ipp_builtin_set_child_nodeofsset(KDRV_IPP_BUILTIN_DTSI_NODE_INFO* root_node, CHAR *path);
INT32 kdrv_ipp_builtin_plat_read_dtsi_array(CHAR *node, CHAR *tag, UINT32 *buf, UINT32 num);
INT32 kdrv_ipp_builtin_plat_read_dtsi_array_ex(KDRV_IPP_BUILTIN_DTSI_NODE_INFO* root_node, CHAR *tag, UINT32 *buf, UINT32 num);
INT32 kdrv_ipp_builtin_plat_read_dtsi_string(CHAR *node, CHAR *tag, const CHAR **str);
void kdrv_ipp_builtin_plat_request_ime_irq(INT32 irq_id, CHAR *name, void *param);
void kdrv_ipp_builtin_plat_release_irq(INT32 irq_id, void *param);
void *kdrv_ipp_builtin_plat_malloc(UINT32 size);
void kdrv_ipp_builtin_plat_free(void *ptr);
void kdrv_ipp_builtin_setreg(uintptr_t ofs, UINT32 value);
UINT32 kdrv_ipp_builtin_getreg(uintptr_t ofs);
void kdrv_ipp_builtin_ife_shared_sram_clksel(void);
void kdrv_ipp_builtin_ipe_shared_sram_clksel(void);
void kdrv_ipp_builtin_ime_shared_sram_clksel(void);
void kdrv_ipp_builtin_cnn_dce_shared_sram_clksel(void);
UINT32 kdrv_ipp_builtin_dbg(void *p_hdl, UINT32 param_id, void *p_data);

#define KDRV_IPP_BUILTIN_TSK_PRIORITY (3)
#define KDRV_IPP_BUILTIN_QUE_DEPTH (8)

#define KDRV_IPP_BUILTIN_TSK_PAUSE			FLGPTN_BIT(0)
#define KDRV_IPP_BUILTIN_TSK_PAUSE_END		FLGPTN_BIT(1)
#define KDRV_IPP_BUILTIN_TSK_RESUME			FLGPTN_BIT(2)
#define KDRV_IPP_BUILTIN_TSK_RESUME_END		FLGPTN_BIT(3)
#define KDRV_IPP_BUILTIN_TSK_TRIGGER		FLGPTN_BIT(4)
#define KDRV_IPP_BUILTIN_TSK_EXIT			FLGPTN_BIT(5)
#define KDRV_IPP_BUILTIN_TSK_EXIT_END		FLGPTN_BIT(6)
#define KDRV_IPP_BUILTIN_TSK_CHK			FLGPTN_BIT(7)
#define KDRV_IPP_BUILTIN_TSK_JOBDONE		FLGPTN_BIT(8)
#define KDRV_IPP_BUILTIN_TSK_FASTBOOT_DONE	FLGPTN_BIT(9)
#define KDRV_IPP_BUILTIN_TSK_RTOS_EXIT_OK	FLGPTN_BIT(10)
#define KDRV_IPP_BUILTIN_TSK_RTOS_FRMEND    FLGPTN_BIT(11)
/**
	BUILTIN IME
*/

#define NVT_IME_BUILTIN_CHIP_COUNT		(1)
#define NVT_IME_BUILTIN_MINOR_COUNT		(1)
#define NVT_IME_BUILTIN_TOTAL_CH_COUNT	(NVT_IME_BUILTIN_CHIP_COUNT * NVT_IME_BUILTIN_MINOR_COUNT)
#define NVT_IME_BUILTIN_NAME            "builtin_ime"

/* chip_id, eng_id to idx mapping */
#define IME_BUILTIN_CHIP_IDX(chip)				(chip - KDRV_CHIP0)
#define IME_BUILTIN_ENG_IDX(eng)				(eng - KDRV_VIDEOPROCS_IME_ENGINE0)
#define IME_BUILTIN_CONV2_HDL_IDX(chip, eng)	((IME_BUILTIN_CHIP_IDX(chip) * NVT_IME_BUILTIN_MINOR_COUNT) + IME_BUILTIN_ENG_IDX(eng))
#define IME_BUILTIN_IDX_CHIP(idx)				((idx / NVT_IME_BUILTIN_MINOR_COUNT) + KDRV_CHIP0)
#define IME_BUILTIN_IDX_ENG(idx)				((idx % NVT_IME_BUILTIN_MINOR_COUNT) + KDRV_VIDEOPROCS_IME_ENGINE0)

/* interrupt status */
#define IME_BUILTIN_INTS_FB_FRM_END          0x10000000  ///< fastboot last frame-end
#define IME_BUILTIN_INTS_FRM_START           0x20000000
#define IME_BUILTIN_INTS_FRM_END             0x80000000  ///< frame-end status
#define IME_BUILTIN_INTS_LL_END              0x00000001  ///
#define IME_BUILTIN_INTS_LL_ERR              0x00000002  ///
#define IME_BUILTIN_INTS_JOB_END             0x00000008  ///

typedef struct {
	UINT32 dev_id;
	CHAR name[16];

	NVT_IME_BUILTIN_RESOURCE resource;
	IME_BUILTIN_ISR_CB fastboot_cb;
	IME_BUILTIN_ISR_CB hdal_cb;
	UINT8 switch_to_hdal;
} IME_BUILTIN_HANDLE;

typedef struct {
	UINT32 chip_num;
	UINT32 eng_num;
	UINT32 total_ch;
	IME_BUILTIN_HANDLE *p_hdl;
	UINT32 *reg_data;
} IME_BUILTIN_CTL;

void ime_builtin_init(UINT32 chip_num, UINT32 eng_num);
void ime_builtin_init_resource(NVT_IME_BUILTIN_RESOURCE *p_resource, UINT32 idx);
void ime_builtin_exit(void);
void ime_builtin_reg_fastboot_cb(UINT32 dev_id, IME_BUILTIN_ISR_CB cb);
void ime_builtin_unreg_fastboot_cb(UINT32 dev_id);
void ime_builtin_request_irq(void);

void ime_builtin_get_path_info(KDRV_IPP_BUILTIN_PATH_ID pid, KDRV_IPP_BUILTIN_IMG_INFO *info);

#endif
