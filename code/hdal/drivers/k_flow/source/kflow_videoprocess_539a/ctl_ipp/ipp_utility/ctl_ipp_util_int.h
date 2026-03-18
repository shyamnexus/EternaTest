/**
    Public header file for ipl utility

    This file is the header file that define the API and data type for ipl utility tool.

    @file       ipl_util.h
    @ingroup    mILibIPLCom
    @note       Nothing (or anything need to be mentioned).

    Copyright   Novatek Microelectronics Corp. 2009.  All rights reserved.
*/
#ifndef _IPL_UTIL_INT_H_
#define _IPL_UTIL_INT_H_

#include "plat/top.h"
#include "comm/timer.h"
#include "kwrap/type.h"
#include "kwrap/spinlock.h"
#include "kwrap/task.h"
#include "kwrap/util.h"
#include "kwrap/list.h"
#include "kflow_common/nvtmpp.h"
#include "ctl_ipp.h"
#include "ctl_ipp_util.h"
#include "ctl_ipp_isp_539a.h"
#if defined(__FREERTOS)
#include <string.h>
#endif

/**
	atomic operations
*/
#if defined(__LINUX)
/* use linux kernel atomic api */
#else
/* use gcc api */
typedef INT32 atomic_t;

#define ATOMIC_INIT(i) {(i)}
#define atomic_inc(x) __sync_add_and_fetch(x, 1)
#define atomic_dec(x) __sync_sub_and_fetch(x, 1)
#define atomic_read(x) __sync_add_and_fetch(x, 0)
#define atomic_set(x, v) __sync_bool_compare_and_swap(x, *x, v)
#define atomic_add(i, x) __sync_add_and_fetch(x, i)
#define atomic_xchg(x, v) __sync_val_compare_and_swap(x, *x, v)

#ifndef max
#define max(a, b) ((a) > (b) ? (a) : (b))
#endif

#ifndef min
#define min(a, b) ((a) > (b) ? (b) : (a))
#endif

#endif

/**
	vos list head type
*/
typedef struct vos_list_head CTL_IPP_LIST_HEAD;

/**
	page size of FLGPTN
*/
#define CTL_IPP_UTIL_FLAG_PAGE_SIZE (sizeof(FLGPTN) * 8)

/**
	offset of member in type
*/
#define CTL_IPP_UTIL_OFFSETOF(TYPE, MEMBER)  ((ULONG)&((TYPE *)0)->MEMBER)


/**
	default ratio for calculation
*/
#define CTL_IPP_RATIO_UNIT_DFT (1000)
#define CTL_IPP_LCA_H_MIN	(40)
#define CTL_IPP_LCA_V_MIN	(30)

/**
	ctl ipp buffer address utility
*/
#define CTL_IPP_BUF_ADDR_MAKE(__va, __pa) ({ CTL_IPP_BUF_ADDR __buf_addr = {(__va), (__pa)}; __buf_addr; })
#define CTL_IPP_BUF_ADDR_IS_ZERO(__addr) (((__addr).va == 0) || ((__addr).pa == 0))
#define CTL_IPP_BUF_ADDR_IS_EQUAL(__addr1, __addr2) (((__addr1).va == (__addr2).va) || ((__addr1).pa == (__addr2).pa))
#define CTL_IPP_BUF_ADDR_OFS(__addr, __ofs) ({ CTL_IPP_BUF_ADDR __buf_addr = {(__addr).va + (__ofs), (__addr).pa + (__ofs)}; __buf_addr; })

/**
	IPP Shared Ctrl Parameter
*/
#define CTL_IPP_STRP_NUM_MAX (8)	// ref to KDRV_IPP_MAX_STP_NUM
#define CTL_IPP_ENG_NUM 1			// ref to KDRV_IPP_TOTAL_NUM
#define CTL_IPP_DIR_INFO_NUM (CTL_IPP_ENG_NUM * CTL_IPP_HDR_MAX_FRAME_NUM * CTL_IPP_COMB_NUM_MAX)

#define CTL_IPP_DIR_CB_INFO_ID(eng_id, frm_id, comb_id) (						\
		(eng_id)																\
	+	(frm_id)	* CTL_IPP_ENG_NUM											\
	+	(comb_id)	* CTL_IPP_ENG_NUM*CTL_IPP_HDR_MAX_FRAME_NUM					\
)

typedef struct _CTL_IPP_BUF_INFO CTL_IPP_BUF_INFO;

typedef enum {
	CTL_IPP_IPP_NNISP_SLICE_MODE = 0,	// raw data processed by ipp <=> ai engine handshake
	CTL_IPP_IPP_NNISP_FRAME_MODE = 1,	// raw data processed by ai, then ipp do raw2yuv
	CTL_IPP_IPP_NNISP_MODE_MAX,
	ENUM_DUMMY4WORD(CTL_IPP_IPP_NNISP_MODE)
} CTL_IPP_IPP_NNISP_MODE;

typedef struct {
	/* stripe */
	CTL_IPP_STRP_RULE_SELECT stripe_rule;
	UINT32 stripe_num;

	/* nnisp */
	UINT8 nnisp_enable;				// use to check if any nn path is enable
	UINT8 nnisp_isp_update; 		// use to check if isp param is updated, which means ai param is from isp cb, instead of hdal vproc
	CTL_IPP_BUF_INFO *p_nnisp_buf;
	CTL_IPP_BUF_ADDR nnisp_in_addr;
	CTL_IPP_IPP_NNISP_MODE nnisp_mode;
	CTL_IPP_NN_MOTION_TYPE_PARAM nnisp_motion_type;
	CTL_IPP_ISP_INT_AIISP_PARAM nnisp_isp_param;
} CTL_IPP_IPP_CTRL;

/**
	PRE Ctrl Parameter
*/
typedef struct {
	/* input image info
		addr[0~(CTL_IPP_HDR_MAX_FRAME_NUM-1)] support most CTL_IPP_HDR_MAX_FRAME_NUM frame input
	*/
	VDO_PXLFMT in_fmt;
	VDO_PXLFMT in_fmt2;	// 538/567 for CFAPAT_2, decide by mirror enable
	CTL_IPP_BUF_ADDR in_addr[CTL_IPP_HDR_MAX_FRAME_NUM];
	UINT32 in_lofs[CTL_IPP_HDR_MAX_FRAME_NUM];
	USIZE in_size;
	CTL_IPP_IN_CROP_MODE in_crp_mode;
	URECT in_crp_window;
	UINT32 in_strp_size[CTL_IPP_STRP_NUM_MAX];
	UINT32 in_strp_ovlp;

	/* output image info */
	UINT32 out_lofs;
	VDO_PXLFMT out_fmt;

	/* subout buffer */
	CTL_IPP_BUF_INFO *p_subout_buf;

	/* raw decode */
	UINT8 decode_enable;
	UINT8 decode_ratio;
	UINT8 decode_aggresive;

	/* flip */
	CTL_IPP_FLIP_TYPE flip;

	/* direct hdr ring buffer line number */
	UINT32 ring_buf_line_num[CTL_IPP_HDR_MAX_FRAME_NUM-1];

	/* direct hdr check reference sie output one line to dram */
	UINT32 hdr_ref_chk_bit;

	/* shdr enable & fusion weight output enable */
	UINT8 shdr_enable;
	UINT8 shdr_out_enable;
	CTL_IPP_BUF_ADDR shdr_out_addr;
	CTL_IPP_BUF_ADDR shdr_out_addr_dbg;
	UINT32 shdr_out_lofs;

	/* va parameter */
	UINT8 va_enable;
	CTL_IPP_BUF_ADDR va_out_addr;
	CTL_IPP_BUF_ADDR va_out_addr_dbg; // dmawp extra buffer

	/* thermal mode */
	BOOL thermal_mode_en;

	/* BNR */
	UINT8 bnr_enable;
	VDO_PXLFMT bnr_in_ref_fmt;
	UINT8 bnr_in_ref_path;
	CTL_IPP_BUF_ADDR bnr_in_ref_addr;
	UINT32 bnr_in_ref_lofs;
	UINT8 bnr_in_dec_enable;
	CTL_IPP_BUF_ADDR bnr_in_mo_addr;
	UINT32 bnr_in_mo_lofs;
	CTL_IPP_BUF_ADDR bnr_in_sigma_addr;
	UINT32 bnr_in_sigma_lofs;

	CTL_IPP_BUF_ADDR bnr_out_mo_addr;
	CTL_IPP_BUF_ADDR bnr_out_mo_addr_dbg; // dmawp extra buffer
	UINT32 bnr_out_mo_lofs;
	CTL_IPP_BUF_ADDR bnr_out_sigma_addr;
	CTL_IPP_BUF_ADDR bnr_out_sigma_addr_dbg; // dmawp extra buffer
	UINT32 bnr_out_sigma_lofs;
	CTL_IPP_BUF_ADDR bnr_out_gamma_addr;
	CTL_IPP_BUF_ADDR bnr_out_gamma_addr_dbg; // dmawp extra buffer
	UINT32 bnr_out_gamma_lofs;

	CTL_IPP_ISP_BNR_STA bnr_sta_info;

	/* aided map */
	UINT8 aided_map_enable; // BNR gamma output to IFE/IPE motion
} CTL_IPP_PRE_CTRL;

/**
	IFE Ctrl Parameter
*/
typedef enum {
	CTL_IPP_IFE_NNISP_0,
	CTL_IPP_IFE_NNISP_1,
	CTL_IPP_IFE_NNISP_MAX,
} CTL_IPP_IFE_NNISP_ID;

typedef struct {
	/* input image info
		addr[0~(CTL_IPP_HDR_MAX_FRAME_NUM-1)] support most CTL_IPP_HDR_MAX_FRAME_NUM frame input
	*/
	VDO_PXLFMT in_fmt;
	VDO_PXLFMT in_fmt2;	// 538/567 for CFAPAT_2, decide by mirror enable
	CTL_IPP_BUF_ADDR in_addr[CTL_IPP_HDR_MAX_FRAME_NUM];
	UINT32 in_lofs[CTL_IPP_HDR_MAX_FRAME_NUM];
	USIZE in_size;
	CTL_IPP_IN_CROP_MODE in_crp_mode;
	URECT in_crp_window;
	UINT32 in_strp_size[CTL_IPP_STRP_NUM_MAX];
	UINT32 in_strp_ovlp;

	/* output image info */
	UINT32 out_lofs;	// dont care. ife d2d only
	VDO_PXLFMT out_fmt;	// dont care. ife d2d only

	/* raw decode */
	UINT8 decode_enable;
	UINT8 decode_ratio;
	UINT8 decode_aggresive;

	/* flip */
	CTL_IPP_FLIP_TYPE flip;

	/* direct hdr ring buffer line number */
	UINT32 ring_buf_line_num[CTL_IPP_HDR_MAX_FRAME_NUM-1];

	/* direct hdr check reference sie output one line to dram */
	UINT32 hdr_ref_chk_bit;

	/* shdr enable */
	UINT8 shdr_enable;

	/* wdr enable */
	UINT8 wdr_enable;
	CTL_IPP_BUF_ADDR wdr_in_addr;
	CTL_IPP_BUF_ADDR wdr_out_addr;
	CTL_IPP_BUF_ADDR wdr_out_addr_dbg; // dmawp extra buffer

	/* va parameter */
	UINT8 va_enable;
	CTL_IPP_BUF_ADDR va_out_addr;
	CTL_IPP_BUF_ADDR va_out_addr_dbg; // dmawp extra buffer

	/* subisp */
	UINT8 subisp_enable;
	USIZE subisp_subsample;
	USIZE subisp_crop;
	VDO_PXLFMT subisp_fmt;

	/* nnisp */
	BOOL nnisp_update[CTL_IPP_IFE_NNISP_MAX]; // set when en/proc_id changed
	UINT32 nnisp_enable[CTL_IPP_IFE_NNISP_MAX];
	UINT32 nnisp_proc_id[CTL_IPP_IFE_NNISP_MAX];
	UINT32 nnisp_effect2proc[CTL_IPP_IFE_NNISP_MAX][CTL_IPP_AI_EFFECT_ID_MAX];

	/* combine mode */
	BOOL combine_mode_en;
	UINT32 strp_num;
	CTL_IPP_SIE_ID src_sel[CTL_IPP_COMB_NUM_MAX];
	UINT32 strp_size[CTL_IPP_COMB_NUM_MAX];
	UINT32 left_ovlp[CTL_IPP_COMB_NUM_MAX];
	UINT32 right_ovlp[CTL_IPP_COMB_NUM_MAX];

	/* two direct mode */
	CTL_IPP_SIE_ID two_dir_src_sel;
	BOOL two_dir_force_set;

	/* thermal mode */
	BOOL thermal_mode_en;

	/* motion parameter (aided map) */
	CTL_IPP_BUF_ADDR motion_in_addr;
	UINT32 motion_in_lofs;
	CTL_IPP_NN_ISP_MOTION_TYPE motion_type;
} CTL_IPP_IFE_CTRL;

/**
	IPE Ctrl Parameter
*/

typedef struct {
	/* input image info
		addr[0]:	y channel address
		addr[1]:	uv channel address
		lofs[0]:	y channel lineoffset
		lofs[1]:	uv channel lineoffset
	*/
	VDO_PXLFMT in_fmt;
	UINT32 in_pix_ycc; // refer to VDO_PIX_YCC_MASK
	CTL_IPP_BUF_ADDR in_addr[2];
	UINT32 in_lofs[2];
	USIZE in_size;
	UINT32 in_strp_size[CTL_IPP_STRP_NUM_MAX];
	UINT32 in_strp_ovlp;

	/* output image info */
	UINT8 out_enable;
	VDO_PXLFMT out_fmt;
	CTL_IPP_BUF_ADDR out_addr[2];

	/* Eth parameter */
	CTL_IPP_ISP_ETH eth; // 538 N.S.

	/* lce parameter */
	BOOL defog_enable;
	CTL_IPP_BUF_ADDR defog_in_addr;
	CTL_IPP_BUF_ADDR defog_out_addr;
	CTL_IPP_BUF_ADDR defog_out_addr_dbg; // dmawp extra buffer
	CTL_IPP_BUF_ADDR lce_in_addr;	// 538 N.S.
	CTL_IPP_BUF_ADDR lce_out_addr;	// 538 N.S.
	CTL_IPP_BUF_ADDR lce_out_addr_dbg; // dmawp extra buffer

	/* defog hw airlight */
	BOOL airlight_enable; // record previous frame's defog enable to decide current airlight parameter
	UINT16 hw_airlight[CTL_IPP_ISP_DEFOG_AIRLIGHT_NUM];

	/* va parameter */
	UINT8 va_enable;
	CTL_IPP_BUF_ADDR va_out_addr;
	CTL_IPP_BUF_ADDR va_out_addr_dbg; // dmawp extra buffer

	/* motion parameter (aided map) */
	CTL_IPP_BUF_ADDR motion_in_addr;
	UINT32 motion_in_lofs;
	CTL_IPP_NN_ISP_MOTION_TYPE motion_type;
} CTL_IPP_IPE_CTRL;

/**
	IME Ctrl Parameter
*/
typedef enum {
	CTL_IPP_IME_NNISP_0,
	CTL_IPP_IME_NNISP_1,
	CTL_IPP_IME_NNISP_MAX,
} CTL_IPP_IME_NNISP_ID;

typedef struct {
	BOOL enable;
	UINT32 bgn_lofs;
	USIZE bgn_size;
	UPOINT region_ofs;
} CTL_IPP_IME_OUT_REGION;

typedef struct {
	UINT8 enable;
	UINT8 dma_enable;
	UINT8 flip_enable;
	VDO_PXLFMT fmt;
	CTL_IPP_BUF_ADDR addr[3];
	CTL_IPP_BUF_ADDR addr_dbg; // dmawp extra buffer
	UINT32 lofs[3];
	URECT pre_crp_window;		// user set crop win
	URECT pre_crp_window_adj;	// flow adjust crop win (depend on in scale size)
	USIZE scl_size;
	URECT post_crp_window;
	UINT8 one_buf_mode_enable;
	UINT8 md_enable;
	ULONG md_addr;
	UINT32 h_align;
	CTL_IPP_IME_OUT_REGION region;
} CTL_IPP_IME_OUT_IMG;

typedef struct {
	/* input image info */
	VDO_PXLFMT in_fmt;
	CTL_IPP_BUF_ADDR in_addr[3];
	UINT32 in_lofs[3];
	USIZE in_size;
	UINT32 in_strp_size[CTL_IPP_STRP_NUM_MAX];
	UINT32 in_strp_ovlp;
	BOOL in_scl_en;
	USIZE in_scl_size;

	/* output image info */
	CTL_IPP_IME_OUT_IMG out_img[CTL_IPP_OUT_PATH_ID_MAX];
	CTL_IPP_SCL_METHOD_SEL out_scl_method_sel;
	CTL_IPP_OUT_COLOR_SPACE out_color_space_sel;
	UINT8 all_out_disable; // true when all path is disable, used to decide if hdal prepare to do ipp stop

	/* lca input/output info */
	UINT8 lca_enable;
	USIZE lca_out_size;

	/* 3DNR input/output info */
	UINT8 tplnr_enable;
	UINT8 tplnr_out_ms_roi_enable;

	VDO_PXLFMT tplnr_in_ref_fmt;
	UINT8 tplnr_in_ref_path;
	USIZE tplnr_in_ref_size;
	CTL_IPP_BUF_ADDR tplnr_in_ref_addr[3];
	UINT32 tplnr_in_ref_lofs[3];
	UINT8 tplnr_in_ref_flip_enable;

	UINT8 tplnr_in_dec_enable;

	CTL_IPP_BUF_ADDR tplnr_in_mv_addr;
	CTL_IPP_BUF_ADDR tplnr_in_ms_addr;
	CTL_IPP_BUF_ADDR tplnr_in_fcvg_addr;

	CTL_IPP_BUF_ADDR tplnr_out_mv_addr;
	CTL_IPP_BUF_ADDR tplnr_out_mv_addr_dbg; // dmawp extra buffer
	CTL_IPP_BUF_ADDR tplnr_out_ms_addr;
	CTL_IPP_BUF_ADDR tplnr_out_ms_addr_dbg; // dmawp extra buffer
	CTL_IPP_BUF_ADDR tplnr_out_ms_roi_addr;
	CTL_IPP_BUF_ADDR tplnr_out_ms_roi_addr_dbg; // dmawp extra buffer
	CTL_IPP_BUF_ADDR tplnr_out_fcvg_addr;
	CTL_IPP_BUF_ADDR tplnr_out_fcvg_addr_dbg; // dmawp extra buffer

	UINT32 tplnr_out_strp_size[CTL_IPP_STRP_NUM_MAX];

	CTL_IPP_ISP_3DNR_STA tplnr_sta_info;
	UINT32 tplnr_motion_sum; // record 3dnr info for venc
	UINT32 tplnr_edge_sum;

	/* privacy mask input/output info */
	UINT8 pm_enable;
	UINT8 pm_pixel_enable;
	CTL_IPP_BUF_ADDR pm_in_addr;
	UINT32 pm_in_lofs;
	USIZE pm_in_size;
	CTL_IPP_BUF_ADDR pm_out_addr;
	CTL_IPP_BUF_ADDR pm_out_addr_dbg; // dmawp extra buffer
	UINT32 pm_out_lofs;
	USIZE pm_out_size;
	CTL_IPP_PM_PXL_BLK pm_pxl_blk;
	CTL_IPP_PM_PXL_SRC_SEL pxl_src_sel;

	/* low delay mode */
	UINT8 low_delay_enable;
	UINT8 low_delay_path;
	UINT32 low_delay_bp;
	UINT32 low_delay_src; // low delay source ipp for codec. 0: NA, 1: ipp0, 2: ipp1, 3: ipp0 + ipp1

	/* dram status */
	UINT8 dram_chk_bit;

	/* va parameter */
	UINT8 va_enable;
	CTL_IPP_BUF_ADDR va_out_addr;
	CTL_IPP_BUF_ADDR va_out_addr_dbg; // dmawp extra buffer

	/* nnisp */
	BOOL nnisp_update[CTL_IPP_IME_NNISP_MAX]; // set when en/proc_id changed
	UINT32 nnisp_enable[CTL_IPP_IME_NNISP_MAX];
	UINT32 nnisp_proc_id[CTL_IPP_IME_NNISP_MAX];
	UINT32 nnisp_effect2proc[CTL_IPP_IME_NNISP_MAX][CTL_IPP_AI_EFFECT_ID_MAX];
} CTL_IPP_IME_CTRL;


typedef struct {
	CTL_IPP_IPP_CTRL ipp_ctrl;
	CTL_IPP_PRE_CTRL pre_ctrl;
	CTL_IPP_IFE_CTRL ife_ctrl;
	CTL_IPP_IPE_CTRL ipe_ctrl;
	CTL_IPP_IME_CTRL ime_ctrl;
	UINT8 is_fastboot_addr;
} CTL_IPP_BASEINFO;


typedef struct {
	CHAR name[16];
	CTL_IPP_LIST_HEAD free_list_head;
	CTL_IPP_LIST_HEAD used_list_head;
	ULONG start_addr;
	UINT32 total_size;
	UINT32 blk_num;
	UINT32 blk_size;
	UINT32 cur_free_num;
	UINT32 max_used_num;
	vk_spinlock_t lock;
	ID flg_id;
} CTL_IPP_MEM_POOL;


/**

     ctl_ipp_util_yuvsize

     @note depend on Y width, height and format to calculate buffer size
     @param[in] fmt			image type
     @param[in] y_width		image width(pixel)
     @param[in] y_height	image height(pixel)
     @return
		- @b UINT32    image buffer size
*/
UINT32 ctl_ipp_util_yuvsize(VDO_PXLFMT fmt, UINT32 y_width, UINT32 y_height);

/**

	ctl_ipp_util_y2uvlof

	@note depend on Y lineoffset and format to calculate UV lineoffset
     @param[in] fmt			image type
     @param[in] y_lof		image lineoffset(byte)
     @param[in] info		ext info. nrx: encode ratio. set 0 for no encode. raw/yuv/other: lineofs. set 0 for defualt lineofs (depend on fmt)
     @return
		- @b UINT32    uv lineoffset
*/
UINT32 ctl_ipp_util_y2uvlof(VDO_PXLFMT fmt, UINT32 y_lof, UINT32 info);

/**

	ctl_ipp_util_y2uvwidth

	@note depend on Y width and format to calculate UV width
     @param[in] fmt			image type
     @param[in] y_w			image width(pixel)
     @return
		- @b UINT32    uv width
*/
UINT32 ctl_ipp_util_y2uvwidth(VDO_PXLFMT fmt, UINT32 y_w);

/**

	ctl_ipp_util_y2uvheight

	@note depend on Y height and format to calculate UV height
     @param[in] fmt			image type
     @param[in] y_h			image height(pixel)
     @return
		- @b UINT32    uv height
*/
UINT32 ctl_ipp_util_y2uvheight(VDO_PXLFMT fmt, UINT32 y_h);

/**

	ctl_ipp_util_ratio2value

	@note depend on ratio to calculate value
     @param[in] base		base value
     @param[in] ratio		ratio
     @param[in] unit		unit of ratio
     @param[in] align		alignment
     @return
		- @b UINT32    value calculate based on ratio, unit and alignment
						align((base * ratio / unit), x)
*/
UINT32 ctl_ipp_util_ratio2value(UINT32 base, UINT32 ratio, UINT32 unit, UINT32 align);

/**
	ctl_ipp_util_pm_subout_size

	@note calculate pm subout size base on pixelation block
*/
USIZE ctl_ipp_util_pm_subout_size(UINT32 w, UINT32 h, CTL_IPP_PM_PXL_BLK pixel_blk);

/**

	ctl_ipp_util_get_syst_timestamp

	@note get sys timestamp
     @return
		- @b UINT64    timestamp [63..32]sec, [31..0]usec
*/
UINT64 ctl_ipp_util_get_syst_timestamp(void);

/**

	ctl_ipp_util_get_syst_counter

	@note get sys counter
     @return
		- @b UINT32    [31..0]usec
*/
UINT32 ctl_ipp_util_get_syst_counter(void);

/**

	ctl_ipp_util_youtsize

	@note calculate sie yout size
	 @param[in] w		yout window number width
	 @param[in] h		yout window number height
     @return
		- @b UINT32    yout size(byte)
*/
UINT32 ctl_ipp_util_youtsize(UINT32 w, UINT32 h);

/**

	ctl_ipp_util_3dnr_ms_roi_size/width/height/lofs

	@note calculate ime 3dnr ms roi
	 @param[in] w		image width
	 @param[in] h		image height
     @return
		- @b UINT32    ime 3dnr ms roi size/width/height/lofs(byte)
*/
UINT32 ctl_ipp_util_3dnr_ms_roi_size(UINT32 w, UINT32 h);
UINT32 ctl_ipp_util_3dnr_ms_roi_width(UINT32 w);
UINT32 ctl_ipp_util_3dnr_ms_roi_height(UINT32 h);
UINT32 ctl_ipp_util_3dnr_ms_roi_lofs(UINT32 w);

UINT32 ctl_ipp_util_fusion_weight_size(UINT32 w, UINT32 h);
UINT32 ctl_ipp_util_fusion_weight_lofs(UINT32 w);
UINT32 ctl_ipp_util_bnr_sigma_size(UINT32 w, UINT32 h);
UINT32 ctl_ipp_util_bnr_sigma_lofs(UINT32 w);
UINT32 ctl_ipp_util_bnr_gamma_size(UINT32 w, UINT32 h);
UINT32 ctl_ipp_util_bnr_gamma_lofs(UINT32 w);

/**

	ctl_ipp_util_pxlfmt_by_flip

	@note adjust pixel start by flip
	 @param[in] pixfmt	start pixel format
	 @param[in] flip	flip function
     @return
		- @b UINT32    pixfmt(byte)
*/
UINT32 ctl_ipp_info_pxlfmt_by_flip(VDO_PXLFMT pixfmt, CTL_IPP_FLIP_TYPE flip);

/**

	ctl_ipp_util_pxlfmt_by_crop

	@note adjust pixel start by crop position
	 @param[in] pixfmt	start pixel format
	 @param[in] crp_x   crop start x position
	 @param[in] crp_y   crop start y position
     @return
		- @b VDO_PXLFMT    pixfmt(byte)
*/
VDO_PXLFMT ctl_ipp_info_pxlfmt_by_crop(VDO_PXLFMT pixfmt, UINT32 crp_x, UINT32 crp_y);

/**

	ctl_ipp_util_youtsize

	@note calculate sie yout size
	 @param[in] w				image width
	 @param[in] h				iamge height
	 @param[in] out_bit_sel		0 --> output 2 bit, 1 --> output 8 bit
	 @param[in] subsample_en	subsample enable, if true w/2, h/2
     @return
		- @b UINT32    eth size(byte)
*/
UINT32 ctl_ipp_util_ethsize(UINT32 w, UINT32 h, BOOL out_bit_sel, BOOL subsample_en);

/**

     ctl_ipp_util_bufsize

     @note depend on width, height and format to calculate buffer size
     @param[in] fmt			image type
     @param[in] w			image width(pixel)
     @param[in] h			image height(pixel)
     @param[in] info		ext info. nrx: encode ratio. set 0 for no encode. raw/yuv/other: lineofs. set 0 for defualt lineofs (depend on fmt)
     @return
		- @b UINT32    buffer size
*/
UINT32 ctl_ipp_util_bufsize(VDO_PXLFMT fmt, UINT32 w, UINT32 h, UINT32 info);

/**

	ctl_ipp_util_os_malloc_wrap

	@note allocate buffer for differenct platform
	 @param[in] want_size		size to allocated
     @return
		- @b void*    buffer
*/
void *ctl_ipp_util_os_malloc_wrap(UINT32 want_size);

/**

	ctl_ipp_util_os_mfree_wrap

	@note free buffer for differenct platform
	 @param[in] p_buf		buffer to release
*/
void ctl_ipp_util_os_mfree_wrap(void *p_buf);

/**

	ctl_ipp_util_os_ioremap_nocache_wrap

	@note non-cacheable ioremap for differenct platform
	 @param[in] offset		physical address
	 @param[in] size		remap size
     @return
		- @b void*			virtual address
*/
void *ctl_ipp_util_os_ioremap_nocache_wrap(phys_addr_t offset, size_t size);

/**

	ctl_ipp_util_os_iounmap_wrap

	@note iounmap for differenct platform
	 @param[in] addr		virtual address from ctl_ipp_util_os_ioremap_nocache_wrap
*/
void ctl_ipp_util_os_iounmap_wrap(void *addr);

/**

	ctl_ipp_util_flow_str

	@note return flow string name
	 @param[in] flow		ipp flow type enum
*/
CHAR *ctl_ipp_util_flow_str(CTL_IPP_FLOW_TYPE flow);
CHAR *ctl_ipp_util_io_type_str(CTL_IPP_BUF_IO_CFG io_type);
CHAR *ctl_ipp_util_buf_type_str(CTL_IPP_BUF_TYPE buf_type);
CHAR *ctl_ipp_util_hdal_func_str(CTL_IPP_FUNC func);

/* get direct mode maxima support width */
INT32 ctl_ipp_util_get_max_stripe(void);
INT32 ctl_ipp_util_get_max_stripe_dual(void);
USIZE ctl_ipp_util_get_max_size(void);
CTL_IPP_FUNC ctl_ipp_util_get_max_spt_func(UINT32 low_pwr_lvl);

/* init and get chip_id, prevent nvt_get_chip_id call malloc in direct mode(ISR) */
void ctl_ipp_util_init_chip_id(void);
UINT32 ctl_ipp_util_get_chip_id(void);


#endif // _IPL_UTIL_INT_H_
