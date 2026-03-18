#if defined(__KERNEL__)
#include <linux/kernel.h>
#include <linux/module.h>
#include "linux/soc/nvt/rcw_macro.h"
#include "kdrv_type.h"
#else
#include <stdlib.h>
#include <stdio.h>
#include <compiler.h>
#endif

#include <libfdt.h>
#include <fdtfast.h>

#include "kwrap/flag.h"
#include "kwrap/task.h"
#include "kwrap/type.h"
#include "kwrap/debug.h"
#include "bridge.h"

// TODO: isp builtin start
#include "sie_eng.h"
#include "sie_eng_base.h"
// TODO: isp builtin end
#include "sie_init.h"
#include "kdrv_ipp_builtin.h"
#include "vdoenc_builtin.h"
#include "ae_param.h"
#include "ae_builtin.h"
#include "awb_param.h"
#include "awb_builtin.h"
#include "iq_builtin.h"
#include "isp_builtin_int.h"
#include "sensor_builtin.h"
#include "ae_builtin_nvt.h"
#include "awb_builtin_nvt.h"

//=============================================================================
// default dtsi
//=============================================================================
#define IQ_FRONT_NODE_PATH_1            "/fastboot/front_iq_0"
#define IQ_NODE_PATH_1                  "/fastboot/iq"
#define IQ_DPC_NODE_PATH_1              "NULL"
#define IQ_SHADING_NODE_PATH_1          "NULL"
#define IQ_FRONT_NODE_PATH_2            "/fastboot/front_iq_0"
#define IQ_NODE_PATH_2                  "/fastboot/iq"
#define IQ_DPC_NODE_PATH_2              "NULL"
#define IQ_SHADING_NODE_PATH_2          "NULL"

#define IQ_SHDR_FRONT_NODE_PATH_1       "/fastboot/front_iq_shdr_0"
#define IQ_SHDR_NODE_PATH_1             "/fastboot/iq_shdr"
#define IQ_SHDR_DPC_NODE_PATH_1         "NULL"
#define IQ_SHDR_SHADING_NODE_PATH_1     "NULL"
#define IQ_SHDR_FRONT_NODE_PATH_2       "/fastboot/front_iq_shdr_1"
#define IQ_SHDR_NODE_PATH_2             "/fastboot/iq_shdr_1"
#define IQ_SHDR_DPC_NODE_PATH_2         "NULL"
#define IQ_SHDR_SHADING_NODE_PATH_2     "NULL"

#define IQ_IR_FRONT_NODE_PATH_1         "/fastboot/front_iq_0"
#define IQ_IR_NODE_PATH_1               "/fastboot/iq"
#define IQ_IR_DPC_NODE_PATH_1           "NULL"
#define IQ_IR_SHADING_NODE_PATH_1       "NULL"
#define IQ_IR_FRONT_NODE_PATH_2         "/fastboot/front_iq_0"
#define IQ_IR_NODE_PATH_2               "/fastboot/iq"
#define IQ_IR_DPC_NODE_PATH_2           "NULL"
#define IQ_IR_SHADING_NODE_PATH_2       "NULL"

#define AE_NODE_PATH_1                  "/fastboot/ae"
#define AE_NODE_PATH_2                  "/fastboot/ae_1"

#define AE_SHDR_NODE_PATH_1             "/fastboot/ae_shdr"
#define AE_SHDR_NODE_PATH_2             "/fastboot/ae_shdr_1"

#define AE_IR_NODE_PATH_1               "/fastboot/ae_ir"
#define AE_IR_NODE_PATH_2               "/fastboot/ae_ir_1"

#define AWB_NODE_PATH_1                 "/fastboot/awb"
#define AWB_NODE_PATH_2                 "/fastboot/awb_1"

#define AWB_SHDR_NODE_PATH_1            "/fastboot/awb_shdr"
#define AWB_SHDR_NODE_PATH_2            "/fastboot/awb_shdr_1"

#define AWB_IR_NODE_PATH_1              "/fastboot/awb_ir"
#define AWB_IR_NODE_PATH_2              "/fastboot/awb_ir_1"

//=============================================================================
// misc.
//=============================================================================
#define AE_BUILTIN_INIT(id) \
{ \
	if (!ISP_BUILTIN_BYPASS_AE) { \
		if (_AE_BUILTIN_NVT) { \
			ae_builtin_init_nvt(id); \
		} else { \
			ae_builtin_init(id); \
		} \
	} \
}

#define AE_BUILTIN_UNINIT(id) \
{ \
	if (!ISP_BUILTIN_BYPASS_AE) { \
		if (_AE_BUILTIN_NVT) { \
			ae_builtin_uninit_nvt(id); \
		} \
	} \
}

#define AE_BUILTIN_TRIG(id, event) \
{ \
	if (!ISP_BUILTIN_BYPASS_AE) { \
		if (_AE_BUILTIN_NVT) { \
			ae_builtin_trig_nvt(id, event); \
		} else { \
			ae_builtin_trig(id, event); \
		} \
	} \
}

#define AWB_BUILTIN_INIT(id) \
{ \
	if (!ISP_BUILTIN_BYPASS_AWB) { \
		if (_AWB_BUILTIN_NVT) { \
			awb_builtin_init_nvt(id); \
		} else { \
			awb_builtin_init(id); \
		} \
	} \
}

#define AWB_BUILTIN_UNINIT(id) \
{ \
	if (!ISP_BUILTIN_BYPASS_AWB) { \
		if (_AWB_BUILTIN_NVT) { \
			awb_builtin_uninit_nvt(id); \
		} \
	} \
}

#define AWB_BUILTIN_TRIG(id, event) \
{ \
	if (!ISP_BUILTIN_BYPASS_AWB) { \
		if (_AWB_BUILTIN_NVT) { \
			awb_builtin_trig_nvt(id, event); \
		} else { \
			awb_builtin_trig(id, event); \
		} \
	} \
}

//=============================================================================
// global
//=============================================================================
void *isp_builtin_ca_rslt[ISP_BUILTIN_ID_MAX_NUM] = {NULL, NULL, NULL, NULL, NULL};
void *isp_builtin_la_rslt[ISP_BUILTIN_ID_MAX_NUM] = {NULL, NULL, NULL, NULL, NULL};
void *isp_builtin_histo_rslt[ISP_BUILTIN_ID_MAX_NUM] = {NULL, NULL, NULL, NULL, NULL};
#if (NVT_FAST_ISP_FLOW)
static ISP_BUILTIN_LA_RSLT *la_rslt = NULL;
static ISP_BUILTIN_CA_RSLT *ca_rslt = NULL;
static ISP_BUILTIN_HISTO_RSLT *histo_rslt = NULL;
#endif
void *isp_builtin_defog_stcs[ISP_BUILTIN_ID_MAX_NUM] = {NULL, NULL, NULL, NULL, NULL};
void *isp_builtin_ipe_subout[ISP_BUILTIN_ID_MAX_NUM] = {NULL, NULL, NULL, NULL, NULL};
static UINT32 ct[ISP_BUILTIN_ID_MAX_NUM] = {5500, 5500, 5500, 5500, 5500};
static UINT32 lv[ISP_BUILTIN_ID_MAX_NUM] = {8000000, 8000000, 8000000, 8000000, 8000000};
static UINT32 overexposure_offset[ISP_BUILTIN_ID_MAX_NUM] = {0};
static UINT32 compensation_ratio[ISP_BUILTIN_ID_MAX_NUM] = {100, 100, 100, 100, 100};
static UINT32 total_gain[ISP_BUILTIN_ID_MAX_NUM] = {100, 100, 100, 100, 100};
static UINT32 shdr_tm_ratio[ISP_BUILTIN_ID_MAX_NUM] = {16384, 16384, 16384, 16384, 16384};
static ISP_BUILTIN_SHDR_EV_RATIO shdr_ev_ratio[ISP_BUILTIN_ID_MAX_NUM] = {{{64, 1024}}, {{64, 1024}}, {{64, 1024}}, {{64, 1024}}, {{64, 1024}}};
static UINT32 shdr_smoothed_tm_ratio[ISP_BUILTIN_ID_MAX_NUM] = {1024, 1024, 1024, 1024, 1024};
static ISP_BUILTIN_AE_STATUS ae_status[ISP_BUILTIN_ID_MAX_NUM] = {ISP_BUILTIN_AE_STATUS_STABLE, ISP_BUILTIN_AE_STATUS_STABLE, ISP_BUILTIN_AE_STATUS_STABLE, ISP_BUILTIN_AE_STATUS_STABLE, ISP_BUILTIN_AE_STATUS_STABLE};
static ISP_BUILTIN_CGAIN cgain[ISP_BUILTIN_ID_MAX_NUM] = {{256, 256, 256}, {256, 256, 256}, {256, 256, 256}, {256, 256, 256}, {256, 256, 256}};
static ISP_BUILTIN_SENSOR_CTRL sensor_ctrl[ISP_BUILTIN_ID_MAX_NUM] = {{{10000, 625}, {1000, 1000}}, {{10000, 625}, {1000, 1000}}, {{10000, 625}, {1000, 1000}}, {{10000, 625}, {1000, 1000}}, {{10000, 625}, {1000, 1000}}};
static UINT32 dgain[ISP_BUILTIN_ID_MAX_NUM] = {128, 128, 128, 128, 128};
#if (NVT_FAST_ISP_FLOW)
static BOOL isp_builtin_task_open = FALSE;
static BOOL isp_builtin_direct_mode = TRUE;
static UINT32 isp_sensor_path_2 = 1;
static UINT32 shdr_map_path[ISP_BUILTIN_ID_MAX_NUM] = {0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF};
#endif
static UINT32 scene_chg_w[ISP_BUILTIN_ID_MAX_NUM] = {100, 100, 100, 100, 100};
static UINT32 sensor_name[ISP_BUILTIN_ID_MAX_NUM] = {0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF};  // -1: not available, 0: imx290, 1: f37, 2: os02k10, 3: os05a10, 4: f35, 5: gc4653, 6: f51
static UINT32 sensor_chgmode_fps[ISP_BUILTIN_ID_MAX_NUM] = {3000, 3000, 3000, 3000, 3000};
static UINT32 sensor_expt_max[ISP_BUILTIN_ID_MAX_NUM] = {30000, 30000, 30000, 30000, 30000};
static UINT32 sensor_i2c_id[ISP_BUILTIN_ID_MAX_NUM] = {0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF};
static UINT32 sensor_i2c_addr[ISP_BUILTIN_ID_MAX_NUM];
static BOOL sensor_valid[ISP_BUILTIN_ID_MAX_NUM];
static BOOL shdr_enable[ISP_BUILTIN_ID_MAX_NUM];
static UINT32 shdr_id_mask[ISP_BUILTIN_ID_MAX_NUM];
static UINT32 nr_lv[ISP_BUILTIN_ID_MAX_NUM] = {100, 100, 100, 100, 100};
static UINT32 _3dnr_lv[ISP_BUILTIN_ID_MAX_NUM] = {100, 100, 100, 100, 100};
static UINT32 sharpness_lv[ISP_BUILTIN_ID_MAX_NUM] = {100, 100, 100, 100, 100};
static UINT32 saturation_lv[ISP_BUILTIN_ID_MAX_NUM] = {100, 100, 100, 100, 100};
static UINT32 contrast_lv[ISP_BUILTIN_ID_MAX_NUM] = {100, 100, 100, 100, 100};
static UINT32 brightness_lv[ISP_BUILTIN_ID_MAX_NUM] = {100, 100, 100, 100, 100};
static UINT32 night_mode[ISP_BUILTIN_ID_MAX_NUM];
static UINT32 ae_stitch_mode[ISP_BUILTIN_ID_MAX_NUM];
static UINT32 awb_stitch_mode[ISP_BUILTIN_ID_MAX_NUM];
static ISP_BUILTIN_DTSI isp_builtin_dtsi[ISP_BUILTIN_DTSI_MAX] = {
	{ IQ_FRONT_NODE_PATH_1, IQ_NODE_PATH_1, IQ_DPC_NODE_PATH_1, IQ_SHADING_NODE_PATH_1, AE_NODE_PATH_1, AWB_NODE_PATH_1 },
	{ IQ_FRONT_NODE_PATH_2, IQ_NODE_PATH_2, IQ_DPC_NODE_PATH_2, IQ_SHADING_NODE_PATH_2, AE_NODE_PATH_2, AWB_NODE_PATH_2 }
};

THREAD_HANDLE isp_builtin_task_id;
ID isp_builtin_flag_id;
UINT32 trig_dram_num = 0, trig_dram_num2 = 0, trig_vd_num = 0, trig_vd_num2 = 0;
static BOOL sensor_bypass = FALSE, cgain_bypass = FALSE;

#define PRINT_NUM 100
#define PRINT_LEN 100
extern void fastboot_msg_en(int en);
extern CHAR ae_print_buf[PRINT_NUM][PRINT_LEN];

//=============================================================================
// function declaration
//=============================================================================
#if defined(__KERNEL__)
extern void nvt_bootts_add_ts(char *name);
#endif
#if (NVT_FAST_ISP_FLOW)
static UINT32 isp_builtin_get_src_id(UINT32 id);
#endif

//=============================================================================
// internal functions
//=============================================================================
#if (NVT_FAST_ISP_FLOW)
static void isp_builtin_set_ca_crop(UINT32 id, BOOL enable)
{
	iq_builtin_set_ca_crop(id, enable);
}

static void isp_builtin_cb_sie(UINT32 id, UINT32 status)
{
	static UINT32 shdr_frame_cnt_dram_end[ISP_BUILTIN_ID_MAX_NUM];
	static UINT32 shdr_frame_cnt_vd[ISP_BUILTIN_ID_MAX_NUM];
	static UINT32 src_id_mask_dram_end[ISP_BUILTIN_ID_MAX_NUM] = {0};
	static UINT32 src_id_mask_vd[ISP_BUILTIN_ID_MAX_NUM] = {0};
	static UINT32 src_id;

	#if (ISP_BUILTIN_TRIG_2A_IQ == 0)
	return;
	#endif

	src_id = isp_builtin_get_src_id(id);

	if (status == SIE_INT_ALL) {
		isp_builtin_direct_mode = kdrv_ipp_builtin_is_direct();

		#if (ISP_BUILTIN_PRINT_VD_CNT)
		DBG_DUMP("vd  rst id = %d, src id = %d, direct mode = %d \r\n", id, src_id, isp_builtin_direct_mode);
		#endif

		if (src_id == 0) {  // 1st sensor
			SET_FLG(isp_builtin_flag_id, FLGPTN_SIE_RESET);
		} else if (sensor_valid[src_id]) {  // 2nd sensor
			SET_FLG(isp_builtin_flag_id, FLGPTN_SIE_RESET_2);
		}
		return;
	}

	if (status & SIE_INT_DRAM_OUT1_END) {
		if (shdr_enable[src_id]) {
			shdr_frame_cnt_dram_end[src_id]++;
			src_id_mask_dram_end[src_id] |= 1 << id;

			if (shdr_frame_cnt_dram_end[src_id] == ISP_SEN_MFRAME_MAX_NUM) {
				if (src_id_mask_dram_end[src_id] == shdr_id_mask[src_id]) {
					if (src_id == 0) {
						SET_FLG(isp_builtin_flag_id, FLGPTN_PROC_DRAMEND);
					} else if (sensor_valid[src_id]) {
						SET_FLG(isp_builtin_flag_id, FLGPTN_PROC_DRAMEND_2);
					}
				} else {
					DBG_DUMP("warning: dram end trig, src id = %d, mask %d == %d ? \r\n", src_id, src_id_mask_dram_end[src_id], shdr_id_mask[src_id]);
				}
				shdr_frame_cnt_dram_end[src_id] = 0;
				src_id_mask_dram_end[src_id] = 0;
			}
		} else {
			if (src_id == 0) {
				SET_FLG(isp_builtin_flag_id, FLGPTN_PROC_DRAMEND);
			} else if (sensor_valid[src_id]) {
				SET_FLG(isp_builtin_flag_id, FLGPTN_PROC_DRAMEND_2);
			}
		}
	}

	if (status & SIE_INT_VD) {
		#if (ISP_BUILTIN_PRINT_VD_CNT)
		{
			static UINT32 print_cnt[ISP_BUILTIN_ID_MAX_NUM] = {0};

			print_cnt[id] ++;
			if (print_cnt[id] < ISP_BUILTIN_PRINT_VD_CNT) {
				DBG_DUMP(" vd cnt = %d, id = %d, src id = %d \r\n", print_cnt[id], id, src_id);
			}
		}
		#endif

		if (shdr_enable[src_id]) {
			shdr_frame_cnt_vd[src_id] ++;
			src_id_mask_vd[src_id] |= 1 << id;

			if (shdr_frame_cnt_vd[src_id] == ISP_SEN_MFRAME_MAX_NUM) {
				if (src_id_mask_vd[src_id] == shdr_id_mask[src_id]) {
					if (src_id == 0) {
						if (vos_flag_chk(isp_builtin_flag_id, FLGPTN_IDLE) == FLGPTN_IDLE) {
							SET_FLG(isp_builtin_flag_id, FLGPTN_PROC_VD);
						}
					} else if (sensor_valid[src_id]) {
						if (vos_flag_chk(isp_builtin_flag_id, FLGPTN_IDLE_2) == FLGPTN_IDLE_2) {
							SET_FLG(isp_builtin_flag_id, FLGPTN_PROC_VD_2);
						}
					}
				} else {
					DBG_DUMP("warning: vd trig, src id = %d, mask %d == %d ? \r\n", src_id, src_id_mask_vd[src_id], shdr_id_mask[src_id]);
				}
				shdr_frame_cnt_vd[src_id] = 0;
				src_id_mask_vd[src_id] = 0;
			}
		} else {
			if (src_id == 0) {
				if (vos_flag_chk(isp_builtin_flag_id, FLGPTN_IDLE) == FLGPTN_IDLE) {
					SET_FLG(isp_builtin_flag_id, FLGPTN_PROC_VD);
				}
			} else if (sensor_valid[src_id]) {
				if (vos_flag_chk(isp_builtin_flag_id, FLGPTN_IDLE_2) == FLGPTN_IDLE_2) {
					SET_FLG(isp_builtin_flag_id, FLGPTN_PROC_VD_2);
				}
			}
		}
	}
}

static void isp_builtin_cb_ipp(UINT32 id, UINT32 status)
{
	ISP_BUILTIN_HISTO_RSLT *histo_rslt = (ISP_BUILTIN_HISTO_RSLT *)isp_builtin_histo_rslt[id];

	#if (ISP_BUILTIN_TRIG_2A_IQ == 0)
	return;
	#endif

	if (status == KDRV_IPP_BUILTIN_ISP_EVENT_RESET) {
		isp_builtin_direct_mode = kdrv_ipp_builtin_is_direct();

		#if (ISP_BUILTIN_PRINT_IPP_CNT)
		DBG_DUMP("ipp rst id = %d, direct mode = %d \r\n", id, isp_builtin_direct_mode);
		#endif
		iq_builtin_trig(id, ISP_BUILTIN_IQ_TRIG_IPP_RESET);
	}

	if ((status == KDRV_IPP_BUILTIN_ISP_EVENT_TRIG) && (!isp_builtin_direct_mode)) {
		#if (ISP_BUILTIN_PRINT_IPP_CNT)
		{
			static UINT32 print_cnt[ISP_BUILTIN_ID_MAX_NUM] = {0};

			print_cnt[id] ++;
			if (print_cnt[id] < ISP_BUILTIN_PRINT_IPP_CNT) {
				DBG_DUMP("ipp cnt = %d, id = %d \r\n", print_cnt[id], id);
			}
		}
		#endif

		iq_builtin_trig(id, ISP_BUILTIN_IQ_TRIG_CGAIN);
		iq_builtin_trig(id, ISP_BUILTIN_IQ_TRIG_IPP);
	}

	if ((status == KDRV_IPP_BUILTIN_ISP_EVENT_FRM_ED) && (!isp_builtin_direct_mode)) {
		iq_builtin_get_histo(id, histo_rslt);
	}
}

// TODO: isp builtin
#if defined(__KERNEL__)
static void isp_builtin_cb_enc(UINT32 id, UINT32 status)
{
	#if (ISP_BUILTIN_TRIG_2A_IQ == 0)
	return;
	#endif

	#if (ISP_BUILTIN_PRINT_ENC_CNT)
	{
		static UINT32 print_cnt[ISP_BUILTIN_ID_MAX_NUM] = {0};

		print_cnt[id] ++;
		if (print_cnt[id] < ISP_BUILTIN_PRINT_ENC_CNT) {
			DBG_DUMP("enc cnt = %d, id = %d \r\n", print_cnt[id], id);
		}
	}
	#endif

	if (status == BUILTIN_ISP_EVENT_ENC_SHARPEN) {
		iq_builtin_trig(id, ISP_BUILTIN_IQ_TRIG_ENC);
	}
}
#endif

THREAD_DECLARE(isp_builtin_tsk, arglist)
{
	static BOOL tag_2a = FALSE;
	UINT32 id;
	FLGPTN flag = (FLGPTN_IDLE | FLGPTN_IDLE_2);

	THREAD_ENTRY();

	while (1) {
		PROFILE_TASK_IDLE();
		wai_flg(&flag, isp_builtin_flag_id, FLGPTN_SIE_RESET | FLGPTN_PROC_DRAMEND | FLGPTN_PROC_VD |
											FLGPTN_SIE_RESET_2 | FLGPTN_PROC_DRAMEND_2 | FLGPTN_PROC_VD_2 |
											FLGPTN_STOP, TWF_ORW | TWF_CLR);

		PROFILE_TASK_BUSY();
		CLR_FLG(isp_builtin_flag_id, FLGPTN_IDLE);

		// NOTE: path 1
		if (flag & FLGPTN_SIE_RESET) {
			id = 0;
			trig_dram_num = 0;
			trig_vd_num = 0;
			AE_BUILTIN_TRIG(id, ISP_BUILTIN_AE_TRIG_RESET);
			AWB_BUILTIN_TRIG(id, ISP_BUILTIN_AWB_TRIG_RESET);
			iq_builtin_trig(id, ISP_BUILTIN_IQ_TRIG_SIE_RESET);
			if (isp_builtin_get_fastboot_version() == 1) {
				isp_builtin_set_ca_crop(id, ENABLE);
			}
		}

		if (flag & FLGPTN_PROC_DRAMEND) {
			id = 0;
			trig_dram_num ++;
			if ((trig_dram_num == 1) && (isp_builtin_get_fastboot_version() == 1)) { // first dramend
				AWB_BUILTIN_TRIG(id, ISP_BUILTIN_AWB_TRIG_PROC);
				if (isp_builtin_direct_mode) {
					iq_builtin_trig(id, ISP_BUILTIN_IQ_TRIG_CGAIN);
				}
				isp_builtin_set_ca_crop(id, DISABLE);
			}

			if (((trig_dram_num == 1) && isp_builtin_direct_mode) && (isp_builtin_get_fastboot_version() == 1)) {  // 1st ipp trigger move to dram end.
				iq_builtin_trig(id, ISP_BUILTIN_IQ_TRIG_IPP);
			}
		}

		if (flag & FLGPTN_PROC_VD) {
			id = 0;
			trig_vd_num ++;
			if (trig_vd_num > 1) {
				if ((trig_vd_num > 2) || (isp_builtin_get_fastboot_version() == 2)) {
					AWB_BUILTIN_TRIG(id, ISP_BUILTIN_AWB_TRIG_PROC);
					if (isp_builtin_direct_mode) {
						iq_builtin_trig(id, ISP_BUILTIN_IQ_TRIG_CGAIN);
					}
				}
				AE_BUILTIN_TRIG(id, ISP_BUILTIN_AE_TRIG_PROC);
			}

			iq_builtin_trig(id, ISP_BUILTIN_IQ_TRIG_SIE);
			if (((trig_vd_num > 1) && isp_builtin_direct_mode) || (isp_builtin_get_fastboot_version() == 2)) {  // 1st ipp trigger move to dram end.
				iq_builtin_trig(id, ISP_BUILTIN_IQ_TRIG_IPP);
			}

			if ((trig_vd_num == 2) && !tag_2a) {
				#if defined(__KERNEL__)
				nvt_bootts_add_ts("2A"); //end
				#endif
				tag_2a = TRUE;
			}
		}

		SET_FLG(isp_builtin_flag_id, FLGPTN_IDLE);
		CLR_FLG(isp_builtin_flag_id, FLGPTN_IDLE_2);

		// NOTE: path 2
		if (flag & FLGPTN_SIE_RESET_2) {
			id = isp_sensor_path_2;
			trig_dram_num2 = 0;
			trig_vd_num2 = 0;
			AE_BUILTIN_TRIG(id, ISP_BUILTIN_AE_TRIG_RESET);
			AWB_BUILTIN_TRIG(id, ISP_BUILTIN_AWB_TRIG_RESET);
			iq_builtin_trig(id, ISP_BUILTIN_IQ_TRIG_SIE_RESET);
			if (isp_builtin_get_fastboot_version() == 1) {
				isp_builtin_set_ca_crop(id, ENABLE);
			}
		}

		if (flag & FLGPTN_PROC_DRAMEND_2) {
			id = isp_sensor_path_2;
			trig_dram_num2 ++;
			if ((trig_dram_num2 == 1) && (isp_builtin_get_fastboot_version() == 1)) { // first dramend
				AWB_BUILTIN_TRIG(id, ISP_BUILTIN_AWB_TRIG_PROC);
				if (isp_builtin_direct_mode) {
					iq_builtin_trig(id, ISP_BUILTIN_IQ_TRIG_CGAIN);
				}
				isp_builtin_set_ca_crop(id, DISABLE);
			}

			if (((trig_dram_num2 == 1) && isp_builtin_direct_mode) && (isp_builtin_get_fastboot_version() == 1)) {  // 1st ipp trigger move to dram end.
				iq_builtin_trig(id, ISP_BUILTIN_IQ_TRIG_IPP);
			}
		}

		if (flag & FLGPTN_PROC_VD_2) {
			id = isp_sensor_path_2;
			trig_vd_num2 ++;
			if (trig_vd_num2 > 1) {
				if ((trig_vd_num2 > 2) || (isp_builtin_get_fastboot_version() == 2)) {
					AWB_BUILTIN_TRIG(id, ISP_BUILTIN_AWB_TRIG_PROC);
					if (isp_builtin_direct_mode) {
						iq_builtin_trig(id, ISP_BUILTIN_IQ_TRIG_CGAIN);
					}
				}
				AE_BUILTIN_TRIG(id, ISP_BUILTIN_AE_TRIG_PROC);
			}

			iq_builtin_trig(id, ISP_BUILTIN_IQ_TRIG_SIE);
			if (((trig_vd_num2 > 1) && isp_builtin_direct_mode) || (isp_builtin_get_fastboot_version() == 2)) {  // 1st ipp trigger move to dram end.
				iq_builtin_trig(id, ISP_BUILTIN_IQ_TRIG_IPP);
			}
		}

		SET_FLG(isp_builtin_flag_id, FLGPTN_IDLE_2);

		if (flag & FLGPTN_STOP) {
			DBG_DUMP("ISP_BUILTIN_STOP \n");
			break;
		}
	}

	#if defined(__KERNEL__)
	return 0;
	#endif
}

static UINT32 isp_builtin_get_src_id(UINT32 id)
{
	UINT32 src_id = 0;

	if ((id == 0) || (id == shdr_map_path[0])) {
		src_id = 0;
	} else if ((id == isp_sensor_path_2) || (id == shdr_map_path[isp_sensor_path_2])) {
		src_id = isp_sensor_path_2;
	}

	return src_id;
}
#endif

//=============================================================================
// external functions
//=============================================================================
ISP_BUILTIN_FUNC isp_builtin_get_func_en(UINT32 id)
{
	// TODO: fastboot2
	#if defined(__KERNEL__)
	ISP_BUILTIN_FUNC isp_func;
	KDRV_IPP_BUILTIN_IPP_INFO ipp_info ={0};

	ipp_info.isp_id = id;
	kdrv_ipp_builtin_get(NULL, KDRV_IPP_PARAM_BUILTIN_GET_IPP_INFO, &ipp_info);

	isp_func = ipp_info.func_en;

	return isp_func;
	#else
	return (ISP_BUILTIN_FUNC_3DNR | ISP_BUILTIN_FUNC_YUV_SUBOUT | ISP_BUILTIN_FUNC_WDR);
	#endif
}

ISP_BUILTIN_CA_RSLT *isp_builtin_get_ca(UINT32 id)
{
	#if (NVT_FAST_ISP_FLOW)
	ULONG addr1, addr2, addr3;
	SIE_ENG_HANDLE *p_sie_eng_handle;
	SIE_CA_RSLT_PARAMS sie_ca_rslt_params;
	SIE_CA_RSLT sie_ca_rslt;

	ca_rslt = (ISP_BUILTIN_CA_RSLT *)isp_builtin_ca_rslt[id];
	if (ca_rslt == NULL) {
		return NULL;
	}

	sie_ca_rslt.buf_r = ca_rslt->r;
	sie_ca_rslt.buf_g = ca_rslt->g;
	sie_ca_rslt.buf_b = ca_rslt->b;
	sie_ca_rslt.buf_ir = ca_rslt->ir;
	sie_ca_rslt.acc_cnt = ca_rslt->acc_cnt;

	// NOTE: addr2 is CA
	sie_fb_get_rdy_addr(id, &addr1, &addr2, &addr3);
	if (addr2 != 0x0) {
		p_sie_eng_handle = sie_eng_get_handle(KDRV_CHIP0, id);
		if (p_sie_eng_handle) {
			sie_eng_get_ca_win_buf_reg(p_sie_eng_handle, &sie_ca_rslt_params.ca_win);
			sie_eng_get_stcs_ob_buf_reg(p_sie_eng_handle, &sie_ca_rslt_params.stcs_ob);
			sie_eng_get_cgain_buf_reg(p_sie_eng_handle, &sie_ca_rslt_params.cgain);
			sie_eng_get_bayer_fmt_buf_reg(p_sie_eng_handle, &sie_ca_rslt_params.bayer_fmt);
			sie_ca_rslt_params.dgain = 1 << 8; // 1x: 256
			sie_eng_get_ca_rslt(p_sie_eng_handle, addr2, &sie_ca_rslt, &sie_ca_rslt_params);
			} else {
				DBG_DUMP("get sie handle fail (%d) \r\n", id);
			}
	} else {
		DBG_DUMP("CA addr 0x0 (%d) \r\n", id);
		return NULL;
	}

	return ca_rslt;
	#else
	return NULL;
	#endif
}

ISP_BUILTIN_LA_RSLT *isp_builtin_get_la(UINT32 id)
{
	#if (NVT_FAST_ISP_FLOW)
	ULONG addr1, addr2, addr3;
	SIE_ENG_HANDLE *p_sie_eng_handle;
	SIE_LA_RSLT sie_la_rslt;
	SIE_LA_WIN_PARAM sie_la_win_param;

	la_rslt = (ISP_BUILTIN_LA_RSLT *)isp_builtin_la_rslt[id];
	if (la_rslt == NULL) {
		return NULL;
	}

	sie_la_rslt.buf_la1 = la_rslt->lum_1;
	sie_la_rslt.buf_la2 = la_rslt->lum_2;

	// NOTE: addr3 is LA
	sie_fb_get_rdy_addr(id, &addr1, &addr2, &addr3);
	if (addr3 != 0x0) {
		p_sie_eng_handle = sie_eng_get_handle(KDRV_CHIP0, id);
		if (p_sie_eng_handle) {
			sie_eng_get_la_win_buf_reg(p_sie_eng_handle, &sie_la_win_param);
			sie_la_win_param.dgain = 1 << 8; // 1x: 256
			sie_eng_get_la_rslt(p_sie_eng_handle, addr3, &sie_la_rslt, &sie_la_win_param);
		} else {
			DBG_DUMP("get sie handle fail (%d) \r\n", id);
		}
	} else {
		DBG_DUMP("LA addr 0x0 (%d) \r\n", id);
		return NULL;
	}

	#if 0
	{
		UINT32 i, sum = 0, avg = 0;
		for (i = 0; i < 32*32; i++) {
			sum += la_rslt->lum_1[i];
		}
		avg = sum / (32*32);
		DBG_DUMP("LA = %d \r\n", avg);
	}
	#endif

	return la_rslt;
	#else
	return NULL;
	#endif
}

ISP_BUILTIN_HISTO_RSLT *isp_builtin_get_histo(UINT32 id)
{
	#if (NVT_FAST_ISP_FLOW)
	histo_rslt = (ISP_BUILTIN_HISTO_RSLT *)isp_builtin_histo_rslt[id];
	if (histo_rslt == NULL) {
		return NULL;
	}

	if (isp_builtin_direct_mode) {
		iq_builtin_get_histo(id, histo_rslt);
	}

	return histo_rslt;
	#else
	return NULL;
	#endif
}

ISP_BUILTIN_DEFOG_STCS *isp_builtin_get_defog_stcs(UINT32 id)
{
	#if (NVT_FAST_ISP_FLOW)
	ISP_BUILTIN_DEFOG_STCS *defog_stcs = (ISP_BUILTIN_DEFOG_STCS *)isp_builtin_defog_stcs[id];

	if (isp_builtin_direct_mode) {
		iq_builtin_get_airlight(id, defog_stcs);
	}

	return defog_stcs;
	#else
	return NULL;
	#endif
}

ISP_BUILTIN_IPE_SUBOUT_BUF *isp_builtin_get_ipe_subout(UINT32 id)
{
	#if (NVT_FAST_ISP_FLOW)
	ISP_BUILTIN_IPE_SUBOUT_BUF *ipe_subout = (ISP_BUILTIN_IPE_SUBOUT_BUF *)isp_builtin_ipe_subout[id];

	if (isp_builtin_direct_mode) {
		iq_builtin_get_ipe_subout(id, ipe_subout);
	}

	return ipe_subout;
	#else
	return NULL;
	#endif
}

UINT32 isp_builtin_get_i2c_id(UINT32 id)
{
	return sensor_i2c_id[id];
}

UINT32 isp_builtin_get_i2c_addr(UINT32 id)
{
	return sensor_i2c_addr[id];
}

BOOL isp_builtin_get_shdr_enable(UINT32 id)
{
	return shdr_enable[id];
}

UINT32 isp_builtin_get_shdr_id_mask(UINT32 id)
{
	return shdr_id_mask[id];
}

UINT32 isp_builtin_get_nr_lv(UINT32 id)
{
	return nr_lv[id];
}

#if defined(__FREERTOS)

void isp_builtin_ae_log(UINT32 id)
{
	#if (ISP_BUILTIN_PRINT_BUF_ENABLE)
	UINT32 i;
	fastboot_msg_en(TRUE);
	DBG_DUMP("Start of ae print \r\n");
	for (i = 0; i < 10; i++) {
		DBG_DUMP("%3d, %s \r\n", i, &ae_print_buf[i][0]);
	}
	DBG_DUMP("End of ae print \r\n");
	#else
	fastboot_msg_en(FALSE);
	#endif
}
#endif

UINT32 isp_builtin_get_3dnr_lv(UINT32 id)
{
	return _3dnr_lv[id];
}

UINT32 isp_builtin_get_sharpness_lv(UINT32 id)
{
	return sharpness_lv[id];
}

UINT32 isp_builtin_get_saturation_lv(UINT32 id)
{
	return saturation_lv[id];
}

UINT32 isp_builtin_get_contrast_lv(UINT32 id)
{
	return contrast_lv[id];
}

UINT32 isp_builtin_get_brightness_lv(UINT32 id)
{
	return brightness_lv[id];
}

UINT32 isp_builtin_get_night_mode(UINT32 id)
{
	return night_mode[id];
}

UINT32 isp_builtin_get_sensor_name(UINT32 id)
{
	//  0: imx290,   1: f37,  2: os02k10,  3: os05a10,  4: f35
	//  5: gc4653,   6: f51,  7: sc450ai,  8: os04c10,  9: os04e10
	// 10: nt99436
	return sensor_name[id];
}

UINT32 isp_builtin_get_ae_stitch_mode(UINT32 id)
{
	return ae_stitch_mode[id];
}

UINT32 isp_builtin_get_awb_stitch_mode(UINT32 id)
{
	return awb_stitch_mode[id];
}

UINT32 isp_builtin_get_chgmode_fps(UINT32 id)
{
	return sensor_chgmode_fps[id];
}

UINT32 isp_builtin_get_sensor_expt_max(UINT32 id)
{
	return sensor_expt_max[id];
}

UINT32 isp_builtin_get_total_gain(UINT32 id)
{
	return total_gain[id];
}

UINT32 isp_builtin_get_ct(UINT32 id)
{
	return ct[id];
}

UINT32 isp_builtin_get_lv(UINT32 id)
{
	return lv[id];
}

UINT32 isp_builtin_get_shdr_tm_ratio(UINT32 id)
{
	return shdr_tm_ratio[id];
}

ISP_BUILTIN_SHDR_EV_RATIO *isp_builtin_get_shdr_ev_ratio(UINT32 id)
{
	return &shdr_ev_ratio[id];
}

UINT32 isp_builtin_get_shdr_smoothed_tm_ratio(UINT32 id)
{
	return shdr_smoothed_tm_ratio[id];
}

UINT32 isp_builtin_get_scene_chg_w(UINT32 id)
{
	return scene_chg_w[id];
}

UINT32 isp_builtin_get_ae_status(UINT32 id)
{
	return ae_status[id];
}

ISP_BUILTIN_CGAIN *isp_builtin_get_cgain(UINT32 id)
{
	return &cgain[id];
}

UINT32 isp_builtin_get_dgain(UINT32 id)
{
	return dgain[id];
}

ISP_BUILTIN_SENSOR_CTRL *isp_builtin_get_sensor_gain(UINT32 id)
{
	return &sensor_ctrl[id];
}

ISP_BUILTIN_SENSOR_CTRL *isp_builtin_get_sensor_expt(UINT32 id)
{
	return &sensor_ctrl[id];
}

BOOL isp_builtin_get_sensor_valid(UINT32 id)
{
	return sensor_valid[id];
}

#if defined(__FREERTOS)
void isp_builtin_reg_i2c_cb(void *cb_fp)
{
	sensor_builtin_reg_i2c_cb(cb_fp);
}
#endif

UINT32 isp_builtin_get_sensor_row_time(UINT32 id)
{
	#if (NVT_FAST_ISP_FLOW)
	return sensor_builtin_get_row_time(id);
	#else
	return 0;
	#endif
}

UINT32 isp_builtin_get_sensor_min_expt_time(UINT32 id)
{
	#if (NVT_FAST_ISP_FLOW)
	return sensor_builtin_get_min_expt_time(id);
	#else
	return 0;
	#endif
}

UINT32 isp_builtin_get_overexposure_offset(UINT32 id)
{
	return overexposure_offset[id];
}

UINT32 isp_builtin_get_compensation_ratio(UINT32 id)
{
	return compensation_ratio[id];
}

UINT32 isp_builtin_get_isp_version(void) {

	return ISP_BUILTIN_VERSION;
}

UINT32 isp_builtin_get_ae_version(void) {
	#if (_AE_BUILTIN_NVT)
	return AE_BUILTIN_VERSION;
	#else
	return 0;
	#endif
}

UINT32 isp_builtin_get_awb_version(void) {
	#if (_AWB_BUILTIN_NVT)
	return AWB_BUILTIN_VERSION;
	#else
	return 0;
	#endif
}

UINT32 isp_builtin_get_iq_version(void) {

	return IQ_BUILTIN_VERSION;
}

UINT32 isp_builtin_get_sensor_version(void) {

	return SENSOR_BUILTIN_VERSION;
}

UINT32 isp_builtin_get_ae_msg_cnt(void) {

	return ISP_BUILTIN_PRINT_AE_MSG_CNT;
}

UINT32 isp_builtin_get_awb_msg_cnt(void) {

	return ISP_BUILTIN_PRINT_AWB_MSG_CNT;
}

void isp_builtin_get_ae_param(UINT32 id, ULONG *param_addr)
{
	#if defined(__KERNEL__)
	#if (NVT_FAST_ISP_FLOW && _AE_BUILTIN_NVT)
	ae_builtin_get_param(id, param_addr);
	#else
	*param_addr = 0;
	#endif
	#else
	*param_addr = 0;
	#endif
}

void isp_builtin_get_awb_param(UINT32 id, ULONG *param_addr)
{
	#if defined(__KERNEL__)
	#if (NVT_FAST_ISP_FLOW && _AWB_BUILTIN_NVT)
	awb_builtin_get_param(id, param_addr);
	#else
	*param_addr = 0;
	#endif
	#else
	*param_addr = 0;
	#endif
}

void isp_builtin_get_iq_param(UINT32 id, ULONG *param_addr)
{
	#if (NVT_FAST_ISP_FLOW)
	iq_builtin_get_param(id, param_addr);
	#else
	*param_addr = 0;
	#endif
}

ISP_BUILTIN_DTSI *isp_builtin_get_dtsi(UINT32 id)
{
	if ((id == 0) || (id >= ISP_BUILTIN_ID_MAX_NUM)) {
		return &isp_builtin_dtsi[0];
	} else {
		return &isp_builtin_dtsi[1];
	}
}

UINT32 isp_builtin_get_fastboot_version(void)
{
	unsigned char *p_fdt = NULL;
	int len;
	int nodeoffset;
	const void *nodep;
	static BOOL is_first = TRUE;
	static UINT32 fastboot_version = 1;

	if (is_first) {
		is_first = FALSE;
		p_fdt = (unsigned char *)fdtfast_get_base();
		if (p_fdt == NULL) {
			DBG_ERR("p_fdt is NULL.\n");
			return 1;
		}

		nodeoffset = fdt_path_offset(p_fdt, "/fastboot");
		if (nodeoffset < 0) {
			DBG_ERR("failed to offset for  %s = %d \n", "/fastboot", nodeoffset);
			return 1;
		}

		nodep = fdt_getprop(p_fdt, nodeoffset, "generation", &len);
		if (len == 0 || nodep == NULL) {
			DBG_DUMP("cannot find /fastboot/generation.\n");
			return 1;
		} else {
			unsigned int *p_data = (unsigned int *)nodep;
			fastboot_version = be32_to_cpu(p_data[0]);
			return fastboot_version;
		}
	} else {
		return fastboot_version;
	}
}

void isp_builtin_uninit_i2c(UINT32 id)
{
	sensor_i2c_id[id] = 0xFFFFFFFF;
	sensor_i2c_addr[id] = 0x0;

	#if (NVT_FAST_ISP_FLOW)
	#if defined(__KERNEL__)
	sensor_builtin_uninit_i2c(id);
	#endif
	#endif
}

INT32 isp_builtin_set_transfer_i2c(UINT32 id, void *msgs, INT32 num)
{
	#if (NVT_FAST_ISP_FLOW)
	return sensor_builtin_set_transfer_i2c(id, msgs, num);
	#else
	return -1;
	#endif
}

void isp_builtin_set_total_gain(UINT32 id, UINT32 value)
{
	total_gain[id] = value;
}

void isp_builtin_set_ct(UINT32 id, UINT32 value)
{
	ct[id] = value;
}

void isp_builtin_set_lv(UINT32 id, UINT32 value)
{
	lv[id] = value;
}

void isp_builtin_set_shdr_ev_ratio(UINT32 id, ISP_BUILTIN_SHDR_EV_RATIO *value)
{
	ISP_BUILTIN_SHDR_EV_RATIO *ratio_temp = (ISP_BUILTIN_SHDR_EV_RATIO *) value;

	shdr_ev_ratio[id].ratio[0] = ratio_temp->ratio[0];
	shdr_ev_ratio[id].ratio[1] = ratio_temp->ratio[1];
}

void isp_builtin_set_shdr_smoothed_tm_ratio(UINT32 id, UINT32 value)
{
	shdr_smoothed_tm_ratio[id] = value;
}

void isp_builtin_set_scene_chg_w(UINT32 id, UINT32 value)
{
	scene_chg_w[id] = value;
}

void isp_builtin_set_ae_status(UINT32 id, ISP_BUILTIN_AE_STATUS value)
{
	ae_status[id] = value;
}

void isp_builtin_set_cgain(UINT32 id, ISP_BUILTIN_CGAIN *value)
{
	ISP_BUILTIN_CGAIN *cgain_temp = (ISP_BUILTIN_CGAIN *) value;

	if (cgain_bypass) {
		return;
	}

	cgain[id].r = cgain_temp->r;
	cgain[id].g = cgain_temp->g;
	cgain[id].b = cgain_temp->b;

	#if (ISP_BUILTIN_PRINT_CGAIN_CNT)
	{
		static UINT32 print_cnt[ISP_BUILTIN_ID_MAX_NUM] = {0};
		UINT32 src_id = isp_builtin_get_src_id(id);

		print_cnt[src_id] ++;
		if (print_cnt[src_id] < ISP_BUILTIN_PRINT_CGAIN_CNT) {
			DBG_DUMP("cnt = %d, id = %d, src_id  = %d, cgain = %d %d %d \r\n", print_cnt[src_id], id, src_id, cgain[id].r, cgain[id].g, cgain[id].b);
		}
	}
	#endif
}

void isp_builtin_set_dgain(UINT32 id, UINT32 value)
{
	if (sensor_bypass) {
		return;
	}

	dgain[id] = value;
}

void isp_builtin_set_sensor_gain(UINT32 id, ISP_BUILTIN_SENSOR_CTRL *value)
{
	#if (NVT_FAST_ISP_FLOW)
	#if (!ISP_BUILTIN_BYPASS_SENSOR)
	ISP_BUILTIN_SENSOR_CTRL *sensor_ctrl_temp = (ISP_BUILTIN_SENSOR_CTRL *) value;

	if (sensor_bypass) {
		return;
	}

	sensor_ctrl[id].gain_ratio[0] = sensor_ctrl_temp->gain_ratio[0];
	sensor_ctrl[id].gain_ratio[1] = sensor_ctrl_temp->gain_ratio[1];
	#endif

	#if (ISP_BUILTIN_PRINT_GAIN_CNT)
	{
		static UINT32 print_cnt[ISP_BUILTIN_ID_MAX_NUM] = {0};
		UINT32 src_id = isp_builtin_get_src_id(id);

		print_cnt[src_id] ++;
		if (print_cnt[src_id] < ISP_BUILTIN_PRINT_GAIN_CNT) {
			DBG_DUMP("cnt = %d, id = %d, src_id  = %d, gain 0 = %d, gain 1 = %d, d gain = %d \r\n", print_cnt[src_id], id, src_id, sensor_ctrl[src_id].gain_ratio[0], sensor_ctrl[src_id].gain_ratio[1], dgain[src_id]);
		}
	}
	#endif

	sensor_builtin_set_gain(id, &sensor_ctrl[id]);
	#endif
}


void isp_builtin_set_sensor_expt(UINT32 id, ISP_BUILTIN_SENSOR_CTRL *value)
{
	#if (NVT_FAST_ISP_FLOW)
	#if (!ISP_BUILTIN_BYPASS_SENSOR)
	ISP_BUILTIN_SENSOR_CTRL *sensor_ctrl_temp = (ISP_BUILTIN_SENSOR_CTRL *) value;

	if (sensor_bypass) {
		return;
	}

	sensor_ctrl[id].exp_time[0] = sensor_ctrl_temp->exp_time[0];
	sensor_ctrl[id].exp_time[1] = sensor_ctrl_temp->exp_time[1];
	#endif

	#if (ISP_BUILTIN_PRINT_EXPT_CNT)
	{
		static UINT32 print_cnt[ISP_BUILTIN_ID_MAX_NUM] = {0};
		UINT32 src_id = isp_builtin_get_src_id(id);

		print_cnt[src_id] ++;
		if (print_cnt[src_id] < ISP_BUILTIN_PRINT_EXPT_CNT) {
			DBG_DUMP("cnt = %d, id = %d, src_id  = %d, expt 0 = %d, expt 1 = %d \r\n", print_cnt[src_id], id, src_id, sensor_ctrl[src_id].exp_time[0], sensor_ctrl[src_id].exp_time[1]);
		}
	}
	#endif

	sensor_builtin_set_expt(id, &sensor_ctrl[id]);
	#endif
}


void isp_builtin_set_overexposure_offset(UINT32 id, UINT32 value)
{
	overexposure_offset[id] = value;
}

void isp_builtin_set_sensor_bypass(BOOL bypass)
{
	sensor_bypass = bypass;
}

void isp_builtin_set_cgain_bypass(BOOL bypass)
{
	cgain_bypass = bypass;
}

void isp_builtin_set_compensation_ratio(UINT32 id, UINT32 value)
{
	compensation_ratio[id] = value;
}

void *isp_builtin_kmem_alloc(UINT32 mem_size)
{
	#if defined(__FREERTOS)
	return malloc(mem_size);
	#else
	return kzalloc(mem_size, GFP_KERNEL);
	#endif
}

void isp_builtin_kmem_free(void *mem_addr)
{
	#if defined(__FREERTOS)
	free(mem_addr);
	#else
	kfree(mem_addr);
	#endif
}

void *isp_builtin_vmem_alloc(UINT32 mem_size)
{
	#if defined(__FREERTOS)
	return malloc(mem_size);
	#else
	return vmalloc(mem_size);
	#endif
}

void isp_builtin_vmem_free(void *mem_addr)
{
	#if defined(__FREERTOS)
	free(mem_addr);
	#else
	vfree(mem_addr);
	#endif
}

#if (NVT_FAST_ISP_FLOW)
#if defined(__KERNEL__)
ER isp_builtin_init(void)
#else
ER isp_builtin_init(ISP_BUILTIN_INIT_PARAM *init_param)
#endif
{
	UINT32 id, total_gain;
	ISP_BUILTIN_CGAIN cgain_bridge[ISP_BUILTIN_ID_MAX_NUM] = {0};
	UINT32 sensor_name_1= 0xFFFFFFFF, sensor_name_2 = 0xFFFFFFFF;
	SENSOR_BUILTIN_DFT_PARAM param = {0};
	#if defined(__KERNEL__)
	UINT32 sensor_cur_mode = 0xCC;

	// get sensor expt/gain from rtos
	kdrv_bridge_get_tag(SENSOR_PRESET_NAME, &sensor_name_1);
	if (sensor_name_1 != 0xFFFFFFFF) {
		id = 0;

		sensor_name[id] = sensor_name_1;
		sensor_valid[id] = TRUE;
		kdrv_bridge_get_tag(SENSOR_CHGMODE_FPS, &sensor_chgmode_fps[id]);
		kdrv_bridge_get_tag(SENSOR_PRESET_EXPT, &sensor_ctrl[id].exp_time[0]);
		kdrv_bridge_get_tag(SENSOR_PRESET_GAIN, &sensor_ctrl[id].gain_ratio[0]);
		kdrv_bridge_get_tag(SENSOR_EXPT_MAX, &sensor_expt_max[id]);
		kdrv_bridge_get_tag(SENSOR_I2C_ID, &sensor_i2c_id[id]);
		kdrv_bridge_get_tag(SENSOR_I2C_ADDR, &sensor_i2c_addr[id]);
		kdrv_bridge_get_tag(SENSOR_CUR_MODE, &sensor_cur_mode);
		kdrv_bridge_get_tag(SENSOR_ROW_TIME, &param.row_time);
		kdrv_bridge_get_tag(SENSOR_VD, &param.vd);
		kdrv_bridge_get_tag(SENSOR_DFT_FPS, &param.dft_fps);
		kdrv_bridge_get_tag(SENSOR_FRAME_NUM, &param.frame_num);
		kdrv_bridge_get_tag(SENSOR_MAX_GAIN, &param.max_gain);
		kdrv_bridge_get_tag(ISP_D_GAIN, &dgain[id]);
		kdrv_bridge_get_tag(ISP_R_GAIN, &cgain_bridge[id].r);
		kdrv_bridge_get_tag(ISP_G_GAIN, &cgain_bridge[id].g);
		kdrv_bridge_get_tag(ISP_B_GAIN, &cgain_bridge[id].b);
		kdrv_bridge_get_tag(ISP_SHDR_ENABLE, &shdr_enable[id]);
		kdrv_bridge_get_tag(ISP_SHDR_PATH, &shdr_map_path[id]);
		kdrv_bridge_get_tag(ISP_SHDR_MASK, &shdr_id_mask[id]);
		kdrv_bridge_get_tag(ISP_NR_LV, &nr_lv[id]);
		kdrv_bridge_get_tag(ISP_3DNR_LV, &_3dnr_lv[id]);
		kdrv_bridge_get_tag(ISP_SHARPNESS_LV, &sharpness_lv[id]);
		kdrv_bridge_get_tag(ISP_SATURATION_LV, &saturation_lv[id]);
		kdrv_bridge_get_tag(ISP_CONTRAST_LV, &contrast_lv[id]);
		kdrv_bridge_get_tag(ISP_BRIGHTNESS_LV, &brightness_lv[id]);
		kdrv_bridge_get_tag(ISP_NIGHT_MODE, &night_mode[id]);

		sensor_builtin_set_dft_param(id, &param);

		DBG_DUMP("isp bridge 1/3, id %d, name 0x%X, fps %d, expt %d, gain %d, expt max %d, i2c 0x%X 0x%X, cur mode %d \r\n", id,
																											sensor_name[id],
																											sensor_chgmode_fps[id],
																											sensor_ctrl[id].exp_time[0],
																											sensor_ctrl[id].gain_ratio[0],
																											sensor_expt_max[id],
																											sensor_i2c_id[id],
																											sensor_i2c_addr[id],
																											sensor_cur_mode);
		DBG_DUMP("isp bridge 2/3, dgain %d, cgain %d %d %d,  shdr 0x%X, 0x%X, 0x%X \r\n", dgain[id],
																						cgain_bridge[id].r,
																						cgain_bridge[id].g,
																						cgain_bridge[id].b,
																						shdr_enable[id],
																						shdr_map_path[id],
																						shdr_id_mask[id]);

		DBG_DUMP("isp bridge 3/3, ui %d %d %d %d %d %d, night %d \r\n", nr_lv[id],
																		_3dnr_lv[id],
																		sharpness_lv[id],
																		saturation_lv[id],
																		contrast_lv[id] ,
																		brightness_lv[id],
																		night_mode[id]);
		sensor_builtin_print_info(id);
	}

	kdrv_bridge_get_tag(SENSOR_PRESET_NAME_2, &sensor_name_2);
	if (sensor_name_2 != 0xFFFFFFFF) {
		kdrv_bridge_get_tag(ISP_PATH_2, &isp_sensor_path_2);
		id = isp_sensor_path_2;

		sensor_name[id] = sensor_name_2;
		sensor_valid[id] = TRUE;
		kdrv_bridge_get_tag(SENSOR_CHGMODE_FPS_2, &sensor_chgmode_fps[id]);
		kdrv_bridge_get_tag(SENSOR_PRESET_EXPT_2, &sensor_ctrl[id].exp_time[0]);
		kdrv_bridge_get_tag(SENSOR_PRESET_GAIN_2, &sensor_ctrl[id].gain_ratio[0]);
		kdrv_bridge_get_tag(SENSOR_EXPT_MAX_2, &sensor_expt_max[id]);
		kdrv_bridge_get_tag(SENSOR_I2C_ID_2, &sensor_i2c_id[id]);
		kdrv_bridge_get_tag(SENSOR_I2C_ADDR_2, &sensor_i2c_addr[id]);
		kdrv_bridge_get_tag(SENSOR_CUR_MODE_2, &sensor_cur_mode);
		kdrv_bridge_get_tag(SENSOR_ROW_TIME_2, &param.row_time);
		kdrv_bridge_get_tag(SENSOR_VD_2, &param.vd);
		kdrv_bridge_get_tag(SENSOR_DFT_FPS_2, &param.dft_fps);
		kdrv_bridge_get_tag(SENSOR_FRAME_NUM_2, &param.frame_num);
		kdrv_bridge_get_tag(SENSOR_MAX_GAIN_2, &param.max_gain);
		kdrv_bridge_get_tag(ISP_D_GAIN_2, &dgain[id]);
		kdrv_bridge_get_tag(ISP_R_GAIN_2, &cgain_bridge[id].r);
		kdrv_bridge_get_tag(ISP_G_GAIN_2, &cgain_bridge[id].g);
		kdrv_bridge_get_tag(ISP_B_GAIN_2, &cgain_bridge[id].b);
		kdrv_bridge_get_tag(ISP_SHDR_ENABLE_2, &shdr_enable[id]);
		kdrv_bridge_get_tag(ISP_SHDR_PATH_2, &shdr_map_path[id]);
		kdrv_bridge_get_tag(ISP_SHDR_MASK_2, &shdr_id_mask[id]);
		kdrv_bridge_get_tag(ISP_NR_LV_2, &nr_lv[id]);
		kdrv_bridge_get_tag(ISP_3DNR_LV_2, &_3dnr_lv[id]);
		kdrv_bridge_get_tag(ISP_SHARPNESS_LV_2, &sharpness_lv[id]);
		kdrv_bridge_get_tag(ISP_SATURATION_LV_2, &saturation_lv[id]);
		kdrv_bridge_get_tag(ISP_CONTRAST_LV_2, &contrast_lv[id]);
		kdrv_bridge_get_tag(ISP_BRIGHTNESS_LV_2, &brightness_lv[id]);
		kdrv_bridge_get_tag(ISP_NIGHT_MODE_2, &night_mode[id]);

		sensor_builtin_set_dft_param(id, &param);

		DBG_DUMP("isp bridge 1/3, id %d, name 0x%X, fps %d, expt %d, gain %d, expt max %d, i2c 0x%X 0x%X, cur mode %d \r\n", id,
																											sensor_name[id],
																											sensor_chgmode_fps[id],
																											sensor_ctrl[id].exp_time[0],
																											sensor_ctrl[id].gain_ratio[0],
																											sensor_expt_max[id],
																											sensor_i2c_id[id],
																											sensor_i2c_addr[id],
																											sensor_cur_mode);
		DBG_DUMP("isp bridge 2/3, dgain %d, cgain %d %d %d,  shdr 0x%X, 0x%X, 0x%X \r\n", dgain[id],
																						cgain_bridge[id].r,
																						cgain_bridge[id].g,
																						cgain_bridge[id].b,
																						shdr_enable[id],
																						shdr_map_path[id],
																						shdr_id_mask[id]);

		DBG_DUMP("isp bridge 3/3, ui %d %d %d %d %d %d, night %d \r\n", nr_lv[id],
																		_3dnr_lv[id],
																		sharpness_lv[id],
																		saturation_lv[id],
																		contrast_lv[id] ,
																		brightness_lv[id],
																		night_mode[id]);
		sensor_builtin_print_info(id);
	}

	if (((sensor_i2c_id[0] == 0xFFFFFFFF) || (sensor_i2c_addr[0] == 0))  && ((sensor_i2c_id[1] == 0xFFFFFFFF) || (sensor_i2c_addr[1] == 0))) {
		DBG_DUMP("get sensor i2c bridge parameters fail, id = %d, addr = 0x%X, id2 = %d, addr2 = 0x%X \n", sensor_i2c_id[0], sensor_i2c_addr[0], sensor_i2c_id[1], sensor_i2c_addr[1]);
		return -E_SYS;
	}
	#else
	if (init_param == NULL) {
		DBG_DUMP("init_param NULL \r\n");
		return -E_SYS;
	}

	sensor_name_1 = init_param->sensor_preset_name[0];
	if (sensor_name_1 != 0xFFFFFFFF) {
		id = 0;

		sensor_name[id] = sensor_name_1;
		sensor_valid[id] = TRUE;
		sensor_chgmode_fps[id] =          init_param->sensor_chgmode_fps[0];
		sensor_ctrl[id].exp_time[0] =     init_param->sensor_preset_expt[0];
		sensor_ctrl[id].gain_ratio[0] =   init_param->sensor_preset_gain[0];
		sensor_expt_max[id] =             init_param->sensor_expt_max[0];
		sensor_i2c_id[id] =               init_param->sensor_i2c_id[0];
		sensor_i2c_id[id] =               init_param->sensor_i2c_id[0];
		sensor_i2c_addr[id] =             init_param->sensor_i2c_addr[0];
		param.row_time =                  init_param->sensor_row_time[0];
		param.vd =                        init_param->sensor_vd[0];
		param.dft_fps =                   init_param->sensor_dft_fps[0];
		param.frame_num =                 init_param->sensor_frame_num[0];
		param.max_gain =                  init_param->sensor_max_gain[0];
		dgain[id] =                       init_param->isp_d_gain[0];
		cgain_bridge[id].r =              init_param->isp_r_gain[0];
		cgain_bridge[id].g =              init_param->isp_g_gain[0];
		cgain_bridge[id].b =              init_param->isp_b_gain[0];
		shdr_enable[id] =                 init_param->isp_shdr_enable[0];
		shdr_map_path[id] =               init_param->isp_shdr_path[0];
		shdr_id_mask[id] =                init_param->isp_shdr_mask[0];
		nr_lv[id] =                       init_param->isp_nr_lv[0];
		_3dnr_lv[id] =                    init_param->isp_3dnr_lv[0];
		sharpness_lv[id] =                init_param->isp_sharpness_lv[0];
		saturation_lv[id] =               init_param->isp_ssaturation_lv[0];
		contrast_lv[id] =                 init_param->isp_contrast_lv[0];
		brightness_lv[id] =               init_param->isp_brightness_lv[0];
		night_mode[id] =                  init_param->isp_night_mode[0];

		sensor_builtin_set_dft_param(id, &param);

		DBG_DUMP("isp init 1/3, id %d, name 0x%X, fps %d, expt %d, gain %d, expt max %d, i2c 0x%X 0x%X, cur mode %d \r\n", id,
																											sensor_name[id],
																											sensor_chgmode_fps[id],
																											sensor_ctrl[id].exp_time[0],
																											sensor_ctrl[id].gain_ratio[0],
																											sensor_expt_max[id],
																											sensor_i2c_id[id],
																											sensor_i2c_addr[id],
																											init_param->sensor_cur_mode[0]);
		DBG_DUMP("isp init 2/3, dgain %d, cgain %d %d %d,  shdr 0x%X, 0x%X, 0x%X \r\n", dgain[id],
																						cgain_bridge[id].r,
																						cgain_bridge[id].g,
																						cgain_bridge[id].b,
																						shdr_enable[id],
																						shdr_map_path[id],
																						shdr_id_mask[id]);

		DBG_DUMP("isp init 3/3, ui %d %d %d %d %d %d, night %d \r\n", nr_lv[id],
																		_3dnr_lv[id],
																		sharpness_lv[id],
																		saturation_lv[id],
																		contrast_lv[id] ,
																		brightness_lv[id],
																		night_mode[id]);
		sensor_builtin_print_info(id);
	}

	sensor_name_2 = init_param->sensor_preset_name[1];
	if (sensor_name_2 != 0xFFFFFFFF) {
		isp_sensor_path_2 = init_param->isp_path[1];
		id = isp_sensor_path_2;

		sensor_name[id] = sensor_name_2;
		sensor_valid[id] = TRUE;
		sensor_chgmode_fps[id] =          init_param->sensor_chgmode_fps[1];
		sensor_ctrl[id].exp_time[0] =     init_param->sensor_preset_expt[1];
		sensor_ctrl[id].gain_ratio[0] =   init_param->sensor_preset_gain[1];
		sensor_expt_max[id] =             init_param->sensor_expt_max[1];
		sensor_i2c_id[id] =               init_param->sensor_i2c_id[1];
		sensor_i2c_addr[id] =             init_param->sensor_i2c_addr[1];
		param.row_time =                  init_param->sensor_row_time[1];
		param.vd =                        init_param->sensor_vd[1];
		param.dft_fps =                   init_param->sensor_dft_fps[1];
		param.frame_num =                 init_param->sensor_frame_num[1];
		param.max_gain =                  init_param->sensor_max_gain[1];
		dgain[id] =                       init_param->isp_d_gain[1];
		cgain_bridge[id].r =              init_param->isp_r_gain[1];
		cgain_bridge[id].g =              init_param->isp_g_gain[1];
		cgain_bridge[id].b =              init_param->isp_b_gain[1];
		shdr_enable[id] =                 init_param->isp_shdr_enable[1];
		shdr_map_path[id] =               init_param->isp_shdr_path[1];
		shdr_id_mask[id] =                init_param->isp_shdr_mask[1];
		nr_lv[id] =                       init_param->isp_nr_lv[1];
		_3dnr_lv[id] =                    init_param->isp_3dnr_lv[1];
		sharpness_lv[id] =                init_param->isp_sharpness_lv[1];
		saturation_lv[id] =               init_param->isp_ssaturation_lv[1];
		contrast_lv[id] =                 init_param->isp_contrast_lv[1];
		brightness_lv[id] =               init_param->isp_brightness_lv[1];
		night_mode[id] =                  init_param->isp_night_mode[1];

		sensor_builtin_set_dft_param(id, &param);

		DBG_DUMP("isp init 1/3, id %d, name 0x%X, fps %d, expt %d, gain %d, expt max %d, i2c 0x%X 0x%X, cur mode %d \r\n", id,
																											sensor_name[id],
																											sensor_chgmode_fps[id],
																											sensor_ctrl[id].exp_time[0],
																											sensor_ctrl[id].gain_ratio[0],
																											sensor_expt_max[id],
																											sensor_i2c_id[id],
																											sensor_i2c_addr[id],
																											init_param->sensor_cur_mode[1]);
		DBG_DUMP("isp init 2/3, dgain %d, cgain %d %d %d,  shdr 0x%X, 0x%X, 0x%X \r\n", dgain[id],
																						cgain_bridge[id].r,
																						cgain_bridge[id].g,
																						cgain_bridge[id].b,
																						shdr_enable[id],
																						shdr_map_path[id],
																						shdr_id_mask[id]);

		DBG_DUMP("isp init 3/3, ui %d %d %d %d %d %d, night %d \r\n", nr_lv[id],
																		_3dnr_lv[id],
																		sharpness_lv[id],
																		saturation_lv[id],
																		contrast_lv[id] ,
																		brightness_lv[id],
																		night_mode[id]);
		sensor_builtin_print_info(id);
	}

	if (((sensor_i2c_id[0] == 0xFFFFFFFF) || (sensor_i2c_addr[0] == 0))  && ((sensor_i2c_id[1] == 0xFFFFFFFF) || (sensor_i2c_addr[1] == 0))) {
		DBG_DUMP("get sensor i2c bridge parameters fail, id = %d, addr = 0x%X, id2 = %d, addr2 = 0x%X \n", sensor_i2c_id[0], sensor_i2c_addr[0], sensor_i2c_id[1], sensor_i2c_addr[1]);
		return -E_SYS;
	}
	#endif

	for (id = 0; id < ISP_BUILTIN_ID_MAX_NUM; id++) {
		if (((id == 0) && (sensor_valid[0] || shdr_enable[isp_builtin_get_src_id(0)])) || ((id == isp_sensor_path_2) && (sensor_valid[isp_sensor_path_2] || shdr_enable[isp_builtin_get_src_id(isp_sensor_path_2)]))) {
			isp_builtin_ca_rslt[id] = MALLOC(sizeof(ISP_BUILTIN_CA_RSLT)*ISP_SEN_MFRAME_MAX_NUM);
			if (isp_builtin_ca_rslt[id] == NULL) {
				DBG_DUMP("allocate isp_builtin_ca_rslt fail \n");
				return -E_SYS;
			}
			isp_builtin_la_rslt[id] = MALLOC(sizeof(ISP_BUILTIN_LA_RSLT)*ISP_SEN_MFRAME_MAX_NUM);
			if (isp_builtin_la_rslt[id] == NULL) {
				DBG_DUMP("allocate isp_builtin_la_rslt fail \n");
				return -E_SYS;
			}
			isp_builtin_histo_rslt[id] = MALLOC(sizeof(ISP_BUILTIN_HISTO_RSLT));
			if (isp_builtin_histo_rslt[id] == NULL) {
				DBG_DUMP("allocate isp_builtin_histo_rslt fail \n");
				return -E_SYS;
			}
		}
	}

	if (!isp_builtin_task_open) {
		// init task
		OS_CONFIG_FLAG(isp_builtin_flag_id);
		clr_flg(isp_builtin_flag_id, FLGPTN_BIT_ALL);
		THREAD_CREATE(isp_builtin_task_id, isp_builtin_tsk, NULL, "isp_builtin_tsk");
		if (isp_builtin_task_id == 0) {
			DBG_DUMP("ISP THREAD_CREATE fail (%ld) \r\n", (ULONG)isp_builtin_task_id);
			return -E_SYS;
		}

		THREAD_RESUME(isp_builtin_task_id);

		isp_builtin_task_open = TRUE;
	}

	// init i2c
	#if defined(__KERNEL__)
	for (id = 0; id < ISP_BUILTIN_ID_MAX_NUM; id++) {
		if (sensor_valid[id]) {
			sensor_builtin_init_i2c(id);
		}
	}
	#endif

	// modify dtsi if is in night/shdr mode
	for (id = 0; id < ISP_BUILTIN_DTSI_MAX; id++) {
		if (id == 0) {
			if (night_mode[id] == 1) {
				sprintf(isp_builtin_dtsi[id].iq_front_node_path, IQ_IR_FRONT_NODE_PATH_1);
				sprintf(isp_builtin_dtsi[id].iq_node_path, IQ_IR_NODE_PATH_1);
				sprintf(isp_builtin_dtsi[id].iq_dpc_node_path, IQ_IR_DPC_NODE_PATH_1);
				sprintf(isp_builtin_dtsi[id].iq_shading_node_path, IQ_IR_SHADING_NODE_PATH_1);
				sprintf(isp_builtin_dtsi[id].ae_node_path, AE_IR_NODE_PATH_1);
				sprintf(isp_builtin_dtsi[id].awb_node_path, AWB_IR_NODE_PATH_1);
			} else if (shdr_enable[id] == 1) {
				sprintf(isp_builtin_dtsi[id].iq_front_node_path, IQ_SHDR_FRONT_NODE_PATH_1);
				sprintf(isp_builtin_dtsi[id].iq_node_path, IQ_SHDR_NODE_PATH_1);
				sprintf(isp_builtin_dtsi[id].iq_dpc_node_path, IQ_SHDR_DPC_NODE_PATH_1);
				sprintf(isp_builtin_dtsi[id].iq_shading_node_path, IQ_SHDR_SHADING_NODE_PATH_1);
				sprintf(isp_builtin_dtsi[id].ae_node_path, AE_SHDR_NODE_PATH_1);
				sprintf(isp_builtin_dtsi[id].awb_node_path, AWB_SHDR_NODE_PATH_1);
			}
		} else if (id == 1) {
			if (night_mode[isp_sensor_path_2] == 1) {
				sprintf(isp_builtin_dtsi[id].iq_front_node_path, IQ_IR_FRONT_NODE_PATH_2);
				sprintf(isp_builtin_dtsi[id].iq_node_path, IQ_IR_NODE_PATH_2);
				sprintf(isp_builtin_dtsi[id].iq_dpc_node_path, IQ_IR_DPC_NODE_PATH_2);
				sprintf(isp_builtin_dtsi[id].iq_shading_node_path, IQ_IR_SHADING_NODE_PATH_2);
				sprintf(isp_builtin_dtsi[id].ae_node_path, AE_IR_NODE_PATH_2);
				sprintf(isp_builtin_dtsi[id].awb_node_path, AWB_IR_NODE_PATH_2);
			} else if (shdr_enable[isp_sensor_path_2] == 1) {
				sprintf(isp_builtin_dtsi[id].iq_front_node_path, IQ_SHDR_FRONT_NODE_PATH_2);
				sprintf(isp_builtin_dtsi[id].iq_node_path, IQ_SHDR_NODE_PATH_2);
				sprintf(isp_builtin_dtsi[id].iq_dpc_node_path, IQ_SHDR_DPC_NODE_PATH_2);
				sprintf(isp_builtin_dtsi[id].iq_shading_node_path, IQ_SHDR_SHADING_NODE_PATH_2);
				sprintf(isp_builtin_dtsi[id].ae_node_path, AE_SHDR_NODE_PATH_2);
				sprintf(isp_builtin_dtsi[id].awb_node_path, AWB_SHDR_NODE_PATH_2);
			}
		}
	}

	// init 2A/IQ
	#if defined(__KERNEL__)
	nvt_bootts_add_ts("2A"); //begin
	#endif
	for (id = 0; id < ISP_BUILTIN_ID_MAX_NUM; id++) {
		if (sensor_valid[id]) {
			// update total gain if not ISO100.
			total_gain = ((sensor_ctrl[id].gain_ratio[0] * dgain[id]) >> 7) / 10;
			isp_builtin_set_total_gain(id, total_gain);

			// update c gain if not zero value.
			if ((cgain_bridge[id].r != 0) && (cgain_bridge[id].g != 0) && (cgain_bridge[id].b != 0)) {
				isp_builtin_set_cgain(id, &cgain_bridge[id]);
			}

			AE_BUILTIN_INIT(id);
			AWB_BUILTIN_INIT(id);
			iq_builtin_init(id);
		}
	}

	// regist call-back fucntion
	sie_fb_reg_isr_Cb(&isp_builtin_cb_sie);
	kdrv_ipp_builtin_reg_isp_cb(&isp_builtin_cb_ipp);
	// TODO: isp builtin
	#if defined(__KERNEL__)
	vdoenc_builtin_evt_fp_reg(NULL, &isp_builtin_cb_enc);
	#endif

	DBG_DUMP("trig 2a/iq %d, ae lib %d, awb lib %d, bypass ae %d, bypass awb %d, bypass sen %d, reload dtsi %d \r\n", ISP_BUILTIN_TRIG_2A_IQ, _AE_BUILTIN_NVT, _AWB_BUILTIN_NVT, ISP_BUILTIN_BYPASS_AE, ISP_BUILTIN_BYPASS_AWB, ISP_BUILTIN_BYPASS_SENSOR, ISP_BUILTIN_RELOAD_DTSI);

	return E_OK;
}

ER isp_builtin_uninit(void)
{
	UINT32 id;

	if (isp_builtin_task_open) {
		// un-init task
		rel_flg(isp_builtin_flag_id);

		isp_builtin_task_open = FALSE;
	}

	for (id = 0; id < ISP_BUILTIN_ID_MAX_NUM; id++) {
		if (sensor_valid[id]) {
			sensor_builtin_init_i2c(id);
		}
	}

	for (id = 0; id < ISP_BUILTIN_ID_MAX_NUM; id++) {
		if (sensor_valid[id]) {
			sensor_builtin_uninit_i2c(id);

			if (isp_builtin_ca_rslt[id]) {
				FREE(isp_builtin_ca_rslt[id]);
				isp_builtin_ca_rslt[id] = NULL;
			}
			if (isp_builtin_la_rslt[id]) {
				FREE(isp_builtin_la_rslt[id]);
				isp_builtin_la_rslt[id] = NULL;
			}
			if (isp_builtin_histo_rslt[id]) {
				FREE(isp_builtin_histo_rslt[id]);
				isp_builtin_histo_rslt[id] = NULL;
			}
		}

		AE_BUILTIN_UNINIT(id);
		AWB_BUILTIN_UNINIT(id);
	}

	return E_OK;
}
#endif

#if defined(__KERNEL__)
EXPORT_SYMBOL(isp_builtin_get_i2c_id);
EXPORT_SYMBOL(isp_builtin_get_i2c_addr);
EXPORT_SYMBOL(isp_builtin_get_shdr_enable);
EXPORT_SYMBOL(isp_builtin_get_shdr_id_mask);
EXPORT_SYMBOL(isp_builtin_get_nr_lv);
EXPORT_SYMBOL(isp_builtin_get_3dnr_lv);
EXPORT_SYMBOL(isp_builtin_get_sharpness_lv);
EXPORT_SYMBOL(isp_builtin_get_saturation_lv);
EXPORT_SYMBOL(isp_builtin_get_contrast_lv);
EXPORT_SYMBOL(isp_builtin_get_brightness_lv);
EXPORT_SYMBOL(isp_builtin_get_night_mode);
EXPORT_SYMBOL(isp_builtin_get_ae_stitch_mode);
EXPORT_SYMBOL(isp_builtin_get_awb_stitch_mode);
EXPORT_SYMBOL(isp_builtin_get_chgmode_fps);
EXPORT_SYMBOL(isp_builtin_get_sensor_expt_max);
EXPORT_SYMBOL(isp_builtin_get_total_gain);
EXPORT_SYMBOL(isp_builtin_get_ct);
EXPORT_SYMBOL(isp_builtin_get_lv);
EXPORT_SYMBOL(isp_builtin_get_shdr_tm_ratio);
EXPORT_SYMBOL(isp_builtin_get_shdr_ev_ratio);
EXPORT_SYMBOL(isp_builtin_get_shdr_smoothed_tm_ratio);
EXPORT_SYMBOL(isp_builtin_get_overexposure_offset);
EXPORT_SYMBOL(isp_builtin_get_compensation_ratio);
EXPORT_SYMBOL(isp_builtin_get_cgain);
EXPORT_SYMBOL(isp_builtin_get_dgain);
EXPORT_SYMBOL(isp_builtin_get_sensor_gain);
EXPORT_SYMBOL(isp_builtin_get_sensor_expt);
EXPORT_SYMBOL(isp_builtin_get_sensor_valid);
EXPORT_SYMBOL(isp_builtin_get_isp_version);
EXPORT_SYMBOL(isp_builtin_get_ae_version);
EXPORT_SYMBOL(isp_builtin_get_awb_version);
EXPORT_SYMBOL(isp_builtin_get_sensor_version);
EXPORT_SYMBOL(isp_builtin_get_iq_version);
EXPORT_SYMBOL(isp_builtin_get_ae_param);
EXPORT_SYMBOL(isp_builtin_get_awb_param);
EXPORT_SYMBOL(isp_builtin_get_iq_param);
EXPORT_SYMBOL(isp_builtin_uninit_i2c);
EXPORT_SYMBOL(isp_builtin_set_transfer_i2c);
EXPORT_SYMBOL(isp_builtin_set_sensor_bypass);
EXPORT_SYMBOL(isp_builtin_set_cgain_bypass);
#endif
