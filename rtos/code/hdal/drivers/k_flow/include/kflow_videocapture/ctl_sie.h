/**
    SIE CTRL Layer

    @file       sie_ctrl.h
    @ingroup    mILibIPH
    @note

    Copyright   Novatek Microelectronics Corp. 2012.  All rights reserved.
*/
#ifndef _CTL_SIE_H_
#define _CTL_SIE_H_

#define KFLOW_SIE_DDR_READY ENABLE
#define KFLOW_SIE_READY DISABLE

#include "kwrap/type.h"
#include "ctl_sie_utility.h"
#include "ctl_sie_spt.h"

#define CTL_SIE_BUF_CB_IGNOR 0xffffffff

typedef enum {
	CTL_SIE_CBEVT_BUFIO			= 0 | CTL_SIE_EVENT_ISR_TAG,	//sts ref. to CTL_SIE_BUF_IO_CFG
	CTL_SIE_CBEVT_DIRECT		= 1 | CTL_SIE_EVENT_ISR_TAG,	//sie direct to ipp cb event
	CTL_SIE_CBEVT_ENG_SIE_ISR	= 2 | CTL_SIE_EVENT_ISR_TAG,	//sts ref. to CTL_SIE_INTE
	CTL_SIE_CBEVT_MAX = 3,
} CTL_SIE_CBEVT_TYPE;

/**
	SIE In/Out buffer config
*/
typedef enum _CTL_SIE_BUF_IO_CFG {
	CTL_SIE_BUF_IO_NEW		= 0x00000001,
	CTL_SIE_BUF_IO_PUSH		= 0x00000002,
	CTL_SIE_BUF_IO_LOCK		= 0x00000004,
	CTL_SIE_BUF_IO_UNLOCK	= 0x00000008,
	CTL_SIE_BUF_IO_ALL		= 0xffffffff,
	ENUM_DUMMY4WORD(CTL_SIE_BUF_IO_CFG)
} CTL_SIE_BUF_IO_CFG;

/**
	SIE direct cb info
*/

typedef enum {
	CTL_SIE_DUAL_DIRECT_UNKNOWN,
	CTL_SIE_DUAL_DIRECT_DISABLE,		///< single direct mode
	CTL_SIE_DUAL_DIRECT_INTERLACE,		///< output interlace mode for vcap0 and vcap1
										///<   sie1(o) | sie1(x) | sie1(o) | sie1(x) ...
										///<   sie2(x) | sie2(o) | sie2(x) | sie2(o) ...
	CTL_SIE_DUAL_DIRECT_PROGRESSIVE,	///< output progressive mode for vcap0 and vcap1
										///<    sie1   |         |  sie1   |         ...
										///<           |  sie2   |         |  sie2   ...
} CTL_SIE_DUAL_DIRECT_MODE;

// direct callback info
typedef struct {
	VDO_FRAME vdo_frm;			// hdal video frame info
	UINT32 sie_id;				// sie id
	UINT32 comm_buf_id;			// comm buf id
	ULONG comm_buf_addr;		// comm buf addr
	BOOL ife_chk_data;			// direct start only
								// 		* sie1 case : ife_chk_mode(560 hw only support 0, 530 hw support 0,1 default 1)
								// 		* sie2~x case :ife_chk_en
	UINT32 ring_buf_len;		// ring buffer length
	UINT32 sen_row_time;		// row time (usx10)

	// combine mode
	BOOL comb_en;				// combine mode enable
	UINT32 comb_idx_bit;		// indicate which sie will be combined together (ex. CTL_SIE_ID_IDX_1 | CTL_SIE_ID_IDX_4)
	UINT32 comb_num;			// combine mode stripe number
	UINT32 comb_order;			// combine mode stripe idx (0 ~ comb_num-1)
	UINT32 comb_width;			// combine mode stripe width (with overlap width)
	UINT32 comb_ovlp_width;		// combine mode overlap width (right overlap width)

	// dual direct mode
	CTL_SIE_DUAL_DIRECT_MODE dual_mode;	// dual direct sie mode
} CTL_SIE_DIRECT_CB_TO_IPP_INFO;

typedef struct {
	UINT32 sie_id;				// sie id
} CTL_SIE_DIRECT_CB_FROM_IPP_INFO;

/**
	SIE Direct CB Cfg
*/

typedef enum {
    CTL_SIE_DIRECT_IPP_STS_STOP,    /* hdal not set sie ready for ipp preparing */
    CTL_SIE_DIRECT_IPP_STS_READY,   /* hdal already set ipp ready for ipp preparing */
    CTL_SIE_DIRECT_IPP_STS_START,	/* ipp already prepared by hdal or ipp */
	CTL_SIE_DIRECT_IPP_STS_MAX,
} CTL_SIE_DIRECT_IPP_STS;

typedef enum {
    CTL_SIE_DIRECT_SIE_STS_STOP,    /* hdal not set sie ready for ipp preparing */
    CTL_SIE_DIRECT_SIE_STS_READY,   /* hdal already set sie ready for ipp preparing */
    CTL_SIE_DIRECT_SIE_STS_START,	/* sie already prepared by hdal or ipp */
	CTL_SIE_DIRECT_SIE_STS_MAX,
} CTL_SIE_DIRECT_SIE_STS;

typedef enum _CTL_SIE_DIRECT_CFG {
	CTL_SIE_DIRECT_TRIG_START		= 0x00000001,	//trigger ipp start
	CTL_SIE_DIRECT_TRIG_STOP		= 0x00000002,	//trigger ipp stop
	CTL_SIE_DIRECT_FRM_CFG_START	= 0x00000004,	//trigger ipp when bp3(center of crop height)
	CTL_SIE_DIRECT_PUSH_RDY_BUF		= 0x00000008,	//push ready buffer
	CTL_SIE_DIRECT_DROP_BUF			= 0x00000010,	//drop public buffer
	CTL_SIE_DIRECT_SKIP_CFG			= 0x00000020,	//skip cfg next frame
	CTL_SIE_DIRECT_GET_STS			= 0x00000040, 	// get ipp status. data_type: CTL_SIE_DIRECT_STS
	CTL_SIE_DIRECT_PREPARE			= 0x00000080,	// set ipp prepare, change status to CTL_IPP_DIRECT_SIE_STS_START. data_type: NULL
	CTL_SIE_DIRECT_UNPREPARE		= 0x00000100,	// set ipp unprepare, change status to CTL_IPP_DIRECT_SIE_STS_READY. data_type: NULL
	CTL_SIE_DIRECT_CFG_ALL			= 0xffffffff,
	ENUM_DUMMY4WORD(CTL_SIE_DIRECT_CFG)
} CTL_SIE_DIRECT_CFG;

typedef enum {
	CTL_SIE_DIRECT_IPP_GET_STS, 	/* get sie status. data_type: CTL_SIE_DIRECT_IPP_STS */
	CTL_SIE_DIRECT_IPP_PREPARE,		/* set sie prepare, change status to CTL_SIE_DIRECT_IPP_STS_START. data_type: NULL */
	CTL_SIE_DIRECT_IPP_UNPREPARE,	/* set sie unprepare, change status to CTL_SIE_DIRECT_IPP_STS_READY. data_type: NULL */
	CTL_SIE_DIRECT_IPP_MAX,
} CTL_SIE_DIRECT_IPP_EVENT;

typedef struct {
	CTL_SIE_CBEVT_TYPE cbevt;
	CTL_SIE_EVENT_FP fp;
	UINT32 sts;					//wait event status
} CTL_SIE_REG_CB_INFO;

#define GYRO_DATA_ITEM_MAX 6 //agyro_x,agyro_y,agyro_z,ags_x,ags_y,ags_z
typedef struct {
	UINT32 *data_num;				//output gyro data number
	UINT64 *t_crop_start;			//timestamp for crop_start
	UINT64 *t_crp_end;				//timestamp for crop_end
    UINT32 *time_stamp;				//timestamp for vd
	INT32 *agyro_x;
	INT32 *agyro_y;
	INT32 *agyro_z;
	INT32 *ags_x;
	INT32 *ags_y;
	INT32 *ags_z;
} CTL_SIE_EXT_GYRO_DATA;

/* resv1 */
#define ctl_sie_vdofrm_resv1_directgetraw 1
#define ctl_sie_vdofrm_resv1_mask_direct(val, resv1) (((val << VDO_IPP_DIRECT_SHIFT) & VDO_IPP_DIRECT_MASK) | (resv1 & ~(VDO_IPP_DIRECT_MASK))) // bit 4..7
#define ctl_sie_chk_directgetraw(resv1) ((resv1 & VDO_IPP_DIRECT_MASK) == ((ctl_sie_vdofrm_resv1_directgetraw << VDO_IPP_DIRECT_SHIFT) & VDO_IPP_DIRECT_MASK)) ? TRUE : FALSE
/* reserved[2] */
#define ctl_sie_vdofrm_reserved_mask_slice_cnt(slice_cnt, reserved)		(((slice_cnt << 0) & 0xffff) | (reserved & ~(0xffff)))	// bit 0..15
/* reserved[2] */
#define ctl_sie_vdofrm_reserved_mask_oocid(ooc_id, reserved)			(((ooc_id << 0) & 0xff) 	| (reserved & ~(0xff)))		// bit 0..7
#define ctl_sie_vdofrm_reserved_mask_conid(con_id, reserved)			((((con_id << 8) & 0xff00) 	| (reserved & ~(0xff00))))	// bit 8..15
/* reserved[4] */
#define ctl_sie_vdofrm_reserved_mask_sieid(sieid, reserved)             (((sieid << 0) & 0xff) | (reserved & ~(0xff)))          // bit 0..7
#define ctl_sie_vdofrm_reserved_mask_ifechkdata(ifechkdata, reserved)   ((((ifechkdata << 8) & 0x100) | (reserved & ~(0x100))))	// bit 8
/* reserved[7] */
#define ctl_sie_vdofrm_reserved_mask_encoderate(encoderate, reserved)   	(((encoderate << 0) & 0x7f) | (reserved & ~(0x7f)))	// bit 0..6
#define ctl_sie_vdofrm_reserved_mask_encodeaggresmode(encoderate, reserved) (((encoderate << 7) & 0x80) | (reserved & ~(0x80)))	// bit 7
#define ctl_sie_vdofrm_reserved_mask_rowtime(rowtime, reserved)         ((((rowtime << 8) & 0xfff00) | (reserved & ~(0xfff00))))// bit 8..19

typedef struct {
	/* Member for CTL_SIE/VIE */
	UINT32 buf_id;			// for public buffer control
	ULONG  buf_addr;		// virtual address
	ULONG  buf_addr_pa;		// physical address (in 64bits, isr cannot va2pa)
	/**
	    vdo_frm reserved info
	    resv1 : refer type_vdo.h, ex : VDO_IPP_DIRECT_SHIFT/VDO_PIX_YCC_SHIFT
	    reserved[0]: for proc cmd out buf wp using (ipp_dis_wp_en << 31 | sie_wp_en << 30 | set_idx << 4 | ddr_id)
	    reserved[1]: gyro data virtual address, plz parsing to CTL_SIE_EXT_GYRO_DATA
	    reserved[2]: direct: public buf id,
					 dram: bit 0..15 : slice_cnt(valid when slice_mode_en = enable & slice_push_en = enable)
	    reserved[3]: direct: public buf addr
	    		     dram: thermal bit 0..7 : ooc_id, bit 8..15 : config_id
	    reserved[4]:
	        - bit 0..7 : ctl_sie_id (direct mode only)
	        - bit 8 : ife_chk_data (direct mode only, CTL_SIE_DIRECT_TRIG_START only)
	            * sie1 case : ife_chk_mode(before 560 hw only support 0, after 530 hw support 0,1 default 1 (CTL_SIE_IFE_CHK_MODE))
	            * sie2~x case :ife_chk_en
	    reserved[5]: direct: ring buffer length,
	    			 dram: dest crop construction of (x << 16 | y)
	    reserved[6]: dest crop construction of (w << 16 | h)
	    reserved[7]:
	        - bit 0..6 : encode rate
	        - bit 7 : aggressive mode en
	        - bit 8..19 : row time (usx10)
	        - bit 20..21: reserved for vprc cap_mode,vproc vendor will fill this value
	*/
	VDO_FRAME vdo_frm;
	/* Member for CTL_SIE */
	UINT64 gyro_vd_ts;		// vd timestamp for GYRO (VCAP Debug)
	UINT64 gyro_crpst_ts;	// crop_start timestamp for GYRO (VCAP Debug)
	UINT64 gyro_crpend_ts;	// crop_end timestamp for GYRO (VCAP Debug)
} CTL_SIE_HEADER_INFO;

void kflow_ctl_sie_init(void);
void kflow_ctl_sie_uninit(void);
UINT32 ctl_sie_buf_query(UINT32 num);
INT32 ctl_sie_init(ULONG buf_addr, UINT32 buf_size);
INT32 ctl_sie_uninit(void);
ULONG ctl_sie_open(void *open_cfg);	// CTL_SIE_OPEN_CFG
INT32 ctl_sie_close(ULONG hdl);
INT32 ctl_sie_set(ULONG hdl, CTL_SIE_ITEM item, void *data);
INT32 ctl_sie_get(ULONG hdl, CTL_SIE_ITEM item, void *data);
INT32 ctl_sie_suspend(ULONG hdl, void *data);
INT32 ctl_sie_resume(ULONG hdl, void *data);
void ctl_sie_spt(CTL_SIE_ID id, CTL_SIE_SPT_ITEM item, void *spt);
/* for direct mode using */
ULONG ctl_sie_get_dir_fp(void *data);

#endif //_CTL_SIE_H_
