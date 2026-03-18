#if defined(__FREERTOS)
#include <string.h>
#else
#if defined(__aarch64__)
#include "linux/soc/nvt/nvt-io.h"
#else
#include "mach/nvt-io.h"
#endif
#endif
#include <plat/top.h>

#include "kwrap/type.h"

#if defined(__KERNEL__)
#include "isp_builtin.h"
#endif
#include "isp_api_int.h"
#include "isp_dbg.h"

#if defined(__FREERTOS)
#include "isp_dev_int.h"
#else
#include "isp_main.h"
#endif
#include "isp_mtr_int.h"
#include "isp_version.h"
#include "ispt_api_int.h"

typedef void (*ispt_fp)(ULONG addr);

//=============================================================================
// function declaration
//=============================================================================
static void ispt_api_get_version(ULONG addr);
static void ispt_api_get_size_tab(ULONG addr);
static void ispt_api_get_func(ULONG addr);
static void ispt_api_get_yuv(ULONG addr);
static void ispt_api_get_raw(ULONG addr);
static void ispt_api_get_frame(ULONG addr);
static void ispt_api_get_memory(ULONG addr);
static void ispt_api_get_sensor_info(ULONG addr);
static void ispt_api_get_sensor_reg(ULONG addr);
static void ispt_api_get_sensor_mode_info(ULONG addr);
static void ispt_api_get_3dnr_sta(ULONG addr);
static void ispt_api_get_md_sta(ULONG addr);
static void ispt_api_get_emc_new_buf(ULONG addr);
static void ispt_api_get_emu_run(ULONG addr);
static void ispt_api_get_bnr_sta(ULONG addr);
static void ispt_api_get_ca_roi(ULONG addr);
static void ispt_api_get_la_roi(ULONG addr);
static void ispt_api_get_va_indep_roi(ULONG addr);
static void ispt_api_get_ca_data(ULONG addr);
static void ispt_api_get_la_data(ULONG addr);
static void ispt_api_get_va_data(ULONG addr);
static void ispt_api_get_va_indep_data(ULONG addr);
static void ispt_api_get_wait_vd(ULONG addr);
static void ispt_api_get_sensor_expt(ULONG addr);
static void ispt_api_get_sensor_gain(ULONG addr);
static void ispt_api_get_d_gain(ULONG addr);
static void ispt_api_get_c_gain(ULONG addr);
static void ispt_api_get_total_gain(ULONG addr);
static void ispt_api_get_lv(ULONG addr);
static void ispt_api_get_ct(ULONG addr);
static void ispt_api_get_motor_iris(ULONG addr);
static void ispt_api_get_motor_focus(ULONG addr);
static void ispt_api_get_motor_zoom(ULONG addr);
static void ispt_api_get_motor_misc(ULONG addr);
static void ispt_api_get_sensor_direction(ULONG addr);
static void ispt_api_get_histo_data(ULONG addr);
static void ispt_api_get_ir_info(ULONG addr);
static void ispt_api_get_shdr_ev_ratio(ULONG addr);
static void ispt_api_get_ca_enable(ULONG addr);
static void ispt_api_get_la_enable(ULONG addr);
static void ispt_api_get_ife_va_indep_roi(ULONG addr);
static void ispt_api_get_ife_va_data(ULONG addr);
static void ispt_api_get_ife_va_indep_data(ULONG addr);
static void ispt_api_get_wait_frmend(ULONG addr);
static void ispt_api_get_wait_cfgstr(ULONG addr);
static void ispt_api_get_wait_procend(ULONG addr);
static void ispt_api_get_vprc_indep(ULONG addr);
static void ispt_api_get_sensor_mode_manual(ULONG addr);
static void ispt_api_get_low_power_lv(ULONG addr);

static void ispt_api_set_yuv(ULONG addr);
static void ispt_api_set_raw(ULONG addr);
static void ispt_api_set_frame(ULONG addr);
static void ispt_api_set_memory(ULONG addr);
static void ispt_api_set_sensor_reg(ULONG addr);
static void ispt_api_set_ca_roi(ULONG addr);
static void ispt_api_set_la_roi(ULONG addr);
static void ispt_api_set_va_indep_roi(ULONG addr);
static void ispt_api_set_sensor_expt(ULONG addr);
static void ispt_api_set_sensor_gain(ULONG addr);
static void ispt_api_set_d_gain(ULONG addr);
static void ispt_api_set_c_gain(ULONG addr);
static void ispt_api_set_total_gain(ULONG addr);
static void ispt_api_set_lv(ULONG addr);
static void ispt_api_set_ct(ULONG addr);
static void ispt_api_set_motor_iris(ULONG addr);
static void ispt_api_set_motor_focus(ULONG addr);
static void ispt_api_set_motor_zoom(ULONG addr);
static void ispt_api_set_motor_misc(ULONG addr);
static void ispt_api_set_sensor_direction(ULONG addr);
static void ispt_api_set_sensor_sleep(ULONG addr);
static void ispt_api_set_sensor_wakeup(ULONG addr);
static void ispt_api_set_shdr_ev_ratio(ULONG addr);
static void ispt_api_set_ca_enable(ULONG addr);
static void ispt_api_set_la_enable(ULONG addr);
static void ispt_api_set_ife_va_indep_roi(ULONG addr);
static void ispt_api_set_vprc_indep(ULONG addr);
static void ispt_api_set_sensor_mode_manual(ULONG addr);
static void ispt_api_set_low_power_lv(ULONG addr);

static void ispt_api_reserve(ULONG addr);

#define RESERVE_SIZE 0
static BOOL is_set_expt[ISP_ID_MAX_NUM];
static BOOL is_set_gain[ISP_ID_MAX_NUM];
static BOOL is_set_d_gain[ISP_ID_MAX_NUM];
static BOOL is_set_c_gain[ISP_ID_MAX_NUM];
static BOOL is_set_total_gain[ISP_ID_MAX_NUM];
static BOOL is_set_shdr_ev_ratio[ISP_ID_MAX_NUM];

static ISPT_INFO ispt_info = { {
	//id                             size
	{ISPT_ITEM_VERSION,              sizeof(UINT32)                   },
	{ISPT_ITEM_SIZE_TAB,             sizeof(ISPT_INFO)                },
	{ISPT_ITEM_FUNC,                 sizeof(ISPT_FUNC)                },
	{ISPT_ITEM_YUV,                  sizeof(ISPT_YUV_INFO)            },
	{ISPT_ITEM_RAW,                  sizeof(ISPT_RAW_INFO)            },
	{ISPT_ITEM_FRAME,                sizeof(ISPT_MEMORY_INFO)         },
	{ISPT_ITEM_MEMORY,               sizeof(ISPT_MEMORY_INFO)         },
	{ISPT_ITEM_SENSOR_INFO,          sizeof(ISPT_SENSOR_INFO)         },
	{ISPT_ITEM_SENSOR_REG,           sizeof(ISPT_SENSOR_REG)          },
	{ISPT_ITEM_SENSOR_MODE_INFO,     sizeof(ISPT_SENSOR_MODE_INFO)    },
	{ISPT_ITEM_3DNR_STA,             sizeof(ISPT_3DNR_STA_INFO)       },
	{ISPT_ITEM_EMU_NEW_BUF,          sizeof(ISPT_EMU_NEW_BUF)         },
	{ISPT_ITEM_EMU_RUN,              sizeof(ISPT_EMU_RUN)             },
	{ISPT_ITEM_BNR_STA,              sizeof(ISPT_BNR_STA_INFO)        },
	{ISPT_ITEM_RESERVE_14,           RESERVE_SIZE                     },
	{ISPT_ITEM_RESERVE_15,           RESERVE_SIZE                     },
	{ISPT_ITEM_RESERVE_16,           RESERVE_SIZE                     },
	{ISPT_ITEM_RESERVE_17,           RESERVE_SIZE                     },
	{ISPT_ITEM_RESERVE_18,           RESERVE_SIZE                     },
	{ISPT_ITEM_RESERVE_19,           RESERVE_SIZE                     },
	{ISPT_ITEM_RESERVE_20,           RESERVE_SIZE                     },
	{ISPT_ITEM_RESERVE_21,           RESERVE_SIZE                     },
	{ISPT_ITEM_CA_ROI,               sizeof(ISPT_CA_ROI)              },
	{ISPT_ITEM_LA_ROI,               sizeof(ISPT_LA_ROI)              },
	{ISPT_ITEM_VA_INDEP_ROI,         sizeof(ISPT_VA_INDEP_ROI)        },
	{ISPT_ITEM_RESERVE_25,           RESERVE_SIZE                     },
	{ISPT_ITEM_RESERVE_26,           RESERVE_SIZE                     },
	{ISPT_ITEM_RESERVE_27,           RESERVE_SIZE                     },
	{ISPT_ITEM_CA_DATA,              sizeof(ISPT_CA_DATA)             },
	{ISPT_ITEM_LA_DATA,              sizeof(ISPT_LA_DATA)             },
	{ISPT_ITEM_VA_DATA,              sizeof(ISPT_IPE_VA_DATA)         },
	{ISPT_ITEM_VA_INDEP_DATA,        sizeof(ISPT_IPE_VA_INDEP_DATA)   },
	{ISPT_ITEM_WAIT_VD,              sizeof(ISPT_WAIT_VD)             },
	{ISPT_ITEM_SENSOR_EXPT,          sizeof(ISPT_SENSOR_EXPT)         },
	{ISPT_ITEM_SENSOR_GAIN,          sizeof(ISPT_SENSOR_GAIN)         },
	{ISPT_ITEM_D_GAIN,               sizeof(ISPT_D_GAIN)              },
	{ISPT_ITEM_C_GAIN,               sizeof(ISPT_C_GAIN)              },
	{ISPT_ITEM_TOTAL_GAIN,           sizeof(ISPT_TOTAL_GAIN)          },
	{ISPT_ITEM_LV,                   sizeof(ISPT_LV),                 },
	{ISPT_ITEM_CT,                   sizeof(ISPT_CT),                 },
	{ISPT_ITEM_MOTOR_IRIS,           sizeof(ISPT_MOTOR_IRIS)          },
	{ISPT_ITEM_MOTOR_FOCUS,          sizeof(ISPT_MOTOR_FOCUS)         },
	{ISPT_ITEM_MOTOR_ZOOM,           sizeof(ISPT_MOTOR_ZOOM)          },
	{ISPT_ITEM_MOTOR_MISC,           sizeof(ISPT_MOTOR_MISC)          },
	{ISPT_ITEM_SENSOR_DIRECTION,     sizeof(ISPT_SENSOR_DIRECTION)    },
	{ISPT_ITEM_SENSOR_SLEEP,         sizeof(UINT32),                  },
	{ISPT_ITEM_SENSOR_WAKEUP,        sizeof(UINT32),                  },
	{ISPT_ITEM_HISTO_DATA,           sizeof(ISPT_HISTO_DATA)          },
	{ISPT_ITEM_IR_INFO,              sizeof(ISPT_IR_INFO)             },
	{ISPT_ITEM_RESERVE_49,           RESERVE_SIZE                     },
	{ISPT_ITEM_MD_STA,               sizeof(ISPT_MD_STA_INFO)         },
	{ISPT_ITEM_SHDR_EV_RATIO,        sizeof(ISPT_SHDR_EV_RATIO)       },
	{ISPT_ITEM_CA_ENABLE,            sizeof(ISPT_CA_ENABLE)           },
	{ISPT_ITEM_LA_ENABLE,            sizeof(ISPT_LA_ENABLE)           },
	{ISPT_ITEM_IFE_VA_INDEP_ROI,     sizeof(ISPT_VA_INDEP_ROI)        },
	{ISPT_ITEM_IFE_VA_DATA,          sizeof(ISPT_IFE_VA_DATA)         },
	{ISPT_ITEM_IFE_VA_INDEP_DATA,    sizeof(ISPT_IFE_VA_INDEP_DATA)   },
	{ISPT_ITEM_RESERVE_57,           RESERVE_SIZE                     },
	{ISPT_ITEM_RESERVE_58,           RESERVE_SIZE                     },
	{ISPT_ITEM_WAIT_FRMEND,          sizeof(ISPT_WAIT_FRMEND)         },
	{ISPT_ITEM_WAIT_CFGSTR,          sizeof(ISPT_WAIT_CFGSTR)         },
	{ISPT_ITEM_WAIT_PROCEND,         sizeof(ISPT_WAIT_PROCEND)        },
	{ISPT_ITEM_RESERVE_62,           RESERVE_SIZE                     },
	{ISPT_ITEM_RESERVE_63,           RESERVE_SIZE                     },
	{ISPT_ITEM_VPRC_INDEP,           sizeof(ISPT_VPRC_INDEP)          },
	{ISPT_ITEM_SENSOR_MODE_MANUAL,   sizeof(ISPT_SENSOR_MODE_MANUAL)  },
	{ISPT_ITEM_LOW_POWER_LV,         sizeof(UINT32)                   },
	{ISPT_ITEM_RESERVE_67,           RESERVE_SIZE                     },
	{ISPT_ITEM_RESERVE_68,           RESERVE_SIZE                     },
	{ISPT_ITEM_RESERVE_69,           RESERVE_SIZE                     },
	{ISPT_ITEM_RESERVE_70,           RESERVE_SIZE                     },
	{ISPT_ITEM_RESERVE_71,           RESERVE_SIZE                     },
	{ISPT_ITEM_RESERVE_72,           RESERVE_SIZE                     },
	{ISPT_ITEM_RESERVE_73,           RESERVE_SIZE                     },
	{ISPT_ITEM_RESERVE_74,           RESERVE_SIZE                     },
	{ISPT_ITEM_RESERVE_75,           RESERVE_SIZE                     },
	{ISPT_ITEM_RESERVE_76,           RESERVE_SIZE                     },
	{ISPT_ITEM_RESERVE_77,           RESERVE_SIZE                     },
	{ISPT_ITEM_RESERVE_78,           RESERVE_SIZE                     },
	{ISPT_ITEM_RESERVE_79,           RESERVE_SIZE                     }
} };

static ispt_fp ispt_get_tab[ISPT_ITEM_MAX] = {
	ispt_api_get_version,
	ispt_api_get_size_tab,
	ispt_api_get_func,
	ispt_api_get_yuv,
	ispt_api_get_raw,
	ispt_api_get_frame,             // 5
	ispt_api_get_memory,
	ispt_api_get_sensor_info,
	ispt_api_get_sensor_reg,
	ispt_api_get_sensor_mode_info,
	ispt_api_get_3dnr_sta,          // 10
	ispt_api_get_emc_new_buf,
	ispt_api_get_emu_run,
	ispt_api_get_bnr_sta,
	ispt_api_reserve,
	ispt_api_reserve,               // 15
	ispt_api_reserve,
	ispt_api_reserve,
	ispt_api_reserve,
	ispt_api_reserve,
	ispt_api_reserve,               // 20
	ispt_api_reserve,
	ispt_api_get_ca_roi,
	ispt_api_get_la_roi,
	ispt_api_get_va_indep_roi,
	ispt_api_reserve,               // 25
	ispt_api_reserve,
	ispt_api_reserve,
	ispt_api_get_ca_data,
	ispt_api_get_la_data,
	ispt_api_get_va_data,           // 30
	ispt_api_get_va_indep_data,
	ispt_api_get_wait_vd,
	ispt_api_get_sensor_expt,
	ispt_api_get_sensor_gain,
	ispt_api_get_d_gain,            // 35
	ispt_api_get_c_gain,
	ispt_api_get_total_gain,
	ispt_api_get_lv,
	ispt_api_get_ct,
	ispt_api_get_motor_iris,        // 40
	ispt_api_get_motor_focus,
	ispt_api_get_motor_zoom,
	ispt_api_get_motor_misc,
	ispt_api_get_sensor_direction,
	ispt_api_reserve,               // 45
	ispt_api_reserve,
	ispt_api_get_histo_data,
	ispt_api_get_ir_info,
	ispt_api_reserve,
	ispt_api_get_md_sta,            // 50
	ispt_api_get_shdr_ev_ratio,
	ispt_api_get_ca_enable,
	ispt_api_get_la_enable,
	ispt_api_get_ife_va_indep_roi,
	ispt_api_get_ife_va_data,       // 55
	ispt_api_get_ife_va_indep_data,
	ispt_api_reserve,
	ispt_api_reserve,
	ispt_api_get_wait_frmend,
	ispt_api_get_wait_cfgstr,       // 60
	ispt_api_get_wait_procend,
	ispt_api_reserve,
	ispt_api_reserve,
	ispt_api_get_vprc_indep,
	ispt_api_get_sensor_mode_manual,// 65
	ispt_api_get_low_power_lv,
	ispt_api_reserve,
	ispt_api_reserve,
	ispt_api_reserve,
	ispt_api_reserve,               // 70
	ispt_api_reserve,
	ispt_api_reserve,
	ispt_api_reserve,
	ispt_api_reserve,
	ispt_api_reserve,               // 75
	ispt_api_reserve,
	ispt_api_reserve,
	ispt_api_reserve,
	ispt_api_reserve
};

static ispt_fp ispt_set_tab[ISPT_ITEM_MAX] = {
	ispt_api_reserve,
	ispt_api_reserve,
	ispt_api_reserve,
	ispt_api_set_yuv,
	ispt_api_set_raw,
	ispt_api_set_frame,             // 5
	ispt_api_set_memory,
	ispt_api_reserve,
	ispt_api_set_sensor_reg,
	ispt_api_reserve,
	ispt_api_reserve,               // 10
	ispt_api_reserve,
	ispt_api_reserve,
	ispt_api_reserve,
	ispt_api_reserve,
	ispt_api_reserve,               // 15
	ispt_api_reserve,
	ispt_api_reserve,
	ispt_api_reserve,
	ispt_api_reserve,
	ispt_api_reserve,               // 20
	ispt_api_reserve,
	ispt_api_set_ca_roi,
	ispt_api_set_la_roi,
	ispt_api_set_va_indep_roi,
	ispt_api_reserve,               // 25
	ispt_api_reserve,
	ispt_api_reserve,
	ispt_api_reserve,
	ispt_api_reserve,
	ispt_api_reserve,               // 30
	ispt_api_reserve,
	ispt_api_reserve,
	ispt_api_set_sensor_expt,
	ispt_api_set_sensor_gain,
	ispt_api_set_d_gain,            // 35
	ispt_api_set_c_gain,
	ispt_api_set_total_gain,
	ispt_api_set_lv,
	ispt_api_set_ct,
	ispt_api_set_motor_iris,        // 40
	ispt_api_set_motor_focus,
	ispt_api_set_motor_zoom,
	ispt_api_set_motor_misc,
	ispt_api_set_sensor_direction,
	ispt_api_set_sensor_sleep,      // 45
	ispt_api_set_sensor_wakeup,
	ispt_api_reserve,
	ispt_api_reserve,
	ispt_api_reserve,
	ispt_api_reserve,               // 50
	ispt_api_set_shdr_ev_ratio,
	ispt_api_set_ca_enable,
	ispt_api_set_la_enable,
	ispt_api_set_ife_va_indep_roi,
	ispt_api_reserve,               // 55
	ispt_api_reserve,
	ispt_api_reserve,
	ispt_api_reserve,
	ispt_api_reserve,
	ispt_api_reserve,               // 60
	ispt_api_reserve,
	ispt_api_reserve,
	ispt_api_reserve,
	ispt_api_set_vprc_indep,
	ispt_api_set_sensor_mode_manual,// 65
	ispt_api_set_low_power_lv,
	ispt_api_reserve,
	ispt_api_reserve,
	ispt_api_reserve,
	ispt_api_reserve,               // 70
	ispt_api_reserve,
	ispt_api_reserve,
	ispt_api_reserve,
	ispt_api_reserve,
	ispt_api_reserve,               // 75
	ispt_api_reserve,
	ispt_api_reserve,
	ispt_api_reserve,
	ispt_api_reserve
};

extern UINT32 sie_crop_end_cnt[ISP_ID_MAX_NUM];

static void ispt_api_get_version(ULONG addr)
{
	UINT32 *data = (UINT32 *)addr;

	*data = isp_get_version();
}

static void ispt_api_get_size_tab(ULONG addr)
{
	memcpy((ISPT_INFO *)addr, &ispt_info, sizeof(ISPT_INFO));
}

static void ispt_api_get_func(ULONG addr)
{
	ISPT_FUNC *data = (ISPT_FUNC *)addr;

	if (data->id >= ISP_ID_MAX_NUM) {
		PRINT_ISP_WRN(isp_dbg_get_dbg_mode(data->id) & ISP_DBG_WRN_MSG, "isp id out of range (%d) \r\n", data->id);
		return;
	}
	isp_api_get_func(data->id, &data->func_info);
}

static void ispt_api_get_yuv(ULONG addr)
{
	ISPT_YUV_INFO *data = (ISPT_YUV_INFO *)addr;

	if (data->id >= ISP_ID_MAX_NUM) {
		PRINT_ISP_WRN(isp_dbg_get_dbg_mode(data->id) & ISP_DBG_WRN_MSG, "isp id out of range (%d) \r\n", data->id);
		return;
	}
	isp_api_get_yuv(data->id, &data->yuv_info);
}

static void ispt_api_get_raw(ULONG addr)
{
	ISPT_RAW_INFO *data = (ISPT_RAW_INFO *)addr;

	if (data->id >= SIE_ID_MAX_NUM) {
		PRINT_ISP_WRN(isp_dbg_get_dbg_mode(data->id) & ISP_DBG_WRN_MSG, "sie id out of range (%d) \r\n", data->id);
		data->raw_info.info_vaild = FALSE;
		return;
	}
	isp_api_get_raw(data->id, &data->raw_info, 0);
}

static void ispt_api_get_frame(ULONG addr)
{
	ISPT_MEMORY_INFO *data = (ISPT_MEMORY_INFO *)addr;

	memcpy((UINT32 *)&data->buf, (ULONG *)data->addr, data->size);
}

static void ispt_api_get_memory(ULONG addr)
{
	ISPT_MEMORY_INFO *data = (ISPT_MEMORY_INFO *)addr;
	ULONG base_addr;
	UINT32 i, read_value;

	if (data->addr == 0) {
		DBG_ERR("phyaddr = 0x%lx is not available ! \r\n", data->addr);
		return;
	}

	#if defined(__FREERTOS)
	base_addr = data->addr;

	for (i = 0; i < data->size; i+=4) {
		read_value = *(UINT32 volatile *)(base_addr+i);
		memcpy((UINT32 *)&data->buf[i], &read_value, sizeof(UINT32));
	}

	#else
	base_addr = (ULONG)ioremap(data->addr, data->size);
	if (base_addr == 0) {
		DBG_ERR("base_addr = 0x%lx is not available ! \r\n", base_addr);
		return;
	}

	for (i = 0; i < data->size; i+=4) {
		read_value = ioread32((void *)(base_addr+i));
		memcpy((UINT32 *)&data->buf[i], &read_value, sizeof(UINT32));
	}
	#endif
}

static void ispt_api_get_sensor_info(ULONG addr)
{
	ISPT_SENSOR_INFO *data = (ISPT_SENSOR_INFO *)addr;

	isp_api_get_sensor_info(&data->sensor_info);
}

static void ispt_api_get_sensor_reg(ULONG addr)
{
	ISPT_SENSOR_REG *data = (ISPT_SENSOR_REG *)addr;

	if (data->id >= ISP_ID_MAX_NUM) {
		PRINT_ISP_WRN(isp_dbg_get_dbg_mode(data->id) & ISP_DBG_WRN_MSG, "isp id out of range (%d) \r\n", data->id);
		return;
	}
	isp_api_get_sensor_reg(data->id, &data->addr, &data->data);
}

static void ispt_api_get_sensor_mode_info(ULONG addr)
{
	ISPT_SENSOR_MODE_INFO *data = (ISPT_SENSOR_MODE_INFO *)addr;

	if (data->id >= ISP_ID_MAX_NUM) {
		PRINT_ISP_WRN(isp_dbg_get_dbg_mode(data->id) & ISP_DBG_WRN_MSG, "isp id out of range (%d) \r\n", data->id);
		return;
	}
	isp_api_get_sensor_mode_info(data->id, &data->info);
}

static void ispt_api_get_bnr_sta(ULONG addr)
{
	if (nvt_get_chip_id() == CHIP_NS02402) {
		ISPT_BNR_STA_INFO *data = (ISPT_BNR_STA_INFO *)addr;
		
		if (data->id >= ISP_ID_MAX_NUM) {
			PRINT_ISP_WRN(isp_dbg_get_dbg_mode(data->id) & ISP_DBG_WRN_MSG, "isp id out of range (%d) \r\n", data->id);
			return;
		}
		isp_api_get_bnr_sta(data->id, &data->bnr_sta_info);
		return;
	} else {
		DBG_WRN("NT98538 not support BNR. \r\n");
		return;
	}
}

static void ispt_api_get_3dnr_sta(ULONG addr)
{
	ISPT_3DNR_STA_INFO *data = (ISPT_3DNR_STA_INFO *)addr;

	if (data->id >= ISP_ID_MAX_NUM) {
		PRINT_ISP_WRN(isp_dbg_get_dbg_mode(data->id) & ISP_DBG_WRN_MSG, "isp id out of range (%d) \r\n", data->id);
		return;
	}
	isp_api_get_3dnr_sta(data->id, &data->_3dnr_sta_info);
}

static void ispt_api_get_md_sta(ULONG addr)
{
	ISPT_MD_STA_INFO *data = (ISPT_MD_STA_INFO *)addr;

	if (data->id >= ISP_ID_MAX_NUM) {
		PRINT_ISP_WRN(isp_dbg_get_dbg_mode(data->id) & ISP_DBG_WRN_MSG, "isp id out of range (%d) \r\n", data->id);
		return;
	}
	isp_api_get_md_sta(data->id, &data->md_sta_info);
}

static void ispt_api_get_emc_new_buf(ULONG addr)
{
	ISPT_EMU_NEW_BUF *data = (ISPT_EMU_NEW_BUF *)addr;

	if (data->id >= SIE_ID_MAX_NUM) {
		PRINT_ISP_WRN(isp_dbg_get_dbg_mode(data->id) & ISP_DBG_WRN_MSG, "sie id out of range (%d) \r\n", data->id);
		return;
	}
	data->rt = isp_api_set_emu_new_buf(data->id, &data->buf_new);
}

static void ispt_api_get_emu_run(ULONG addr)
{
	ISPT_EMU_RUN *data = (ISPT_EMU_RUN *)addr;

	if (data->id >= SIE_ID_MAX_NUM) {
		PRINT_ISP_WRN(isp_dbg_get_dbg_mode(data->id) & ISP_DBG_WRN_MSG, "sie id out of range (%d) \r\n", data->id);
		return;
	}
	data->rt = isp_api_set_emu_run(data->id, &data->buf_push);
}

static void ispt_api_get_ca_roi(ULONG addr)
{
	ISPT_CA_ROI *data = (ISPT_CA_ROI *)addr;

	if (data->id >= SIE_ID_MAX_NUM) {
		PRINT_ISP_WRN(isp_dbg_get_dbg_mode(data->id) & ISP_DBG_WRN_MSG, "sie id out of range (%d) \r\n", data->id);
		return;
	}
	isp_dev_get_sync_item(data->id, ISP_SYNC_SEL_CURRENT, ISP_SYNC_CA_ROI, &data->roi);
}

static void ispt_api_get_la_roi(ULONG addr)
{
	ISPT_LA_ROI *data = (ISPT_LA_ROI *)addr;

	if (data->id >= SIE_ID_MAX_NUM) {
		PRINT_ISP_WRN(isp_dbg_get_dbg_mode(data->id) & ISP_DBG_WRN_MSG, "sie id out of range (%d) \r\n", data->id);
		return;
	}
	isp_dev_get_sync_item(data->id, ISP_SYNC_SEL_CURRENT, ISP_SYNC_LA_ROI, &data->roi);
}

static void ispt_api_get_va_indep_roi(ULONG addr)
{
	ISPT_VA_INDEP_ROI *data = (ISPT_VA_INDEP_ROI *)addr;

	if (nvt_get_chip_id() == CHIP_NS02402) {
		PRINT_ISP_WRN(isp_dbg_get_dbg_mode(data->id) & ISP_DBG_WRN_MSG, "NT98539A not support ipe va indep roi. \r\n");
		return;
	}

	if (data->id >= ISP_ID_MAX_NUM) {
		PRINT_ISP_WRN(isp_dbg_get_dbg_mode(data->id) & ISP_DBG_WRN_MSG, "isp id out of range (%d) \r\n", data->id);
		return;
	}
	isp_dev_get_sync_item(data->id, ISP_SYNC_SEL_CURRENT, ISP_SYNC_VA_INDEP_ROI, &data->roi[0]);
}

static void ispt_api_get_ca_data(ULONG addr)
{
	ISPT_CA_DATA *data = (ISPT_CA_DATA *)addr;
	ISP_CA_RSLT *ca_rslt = isp_dev_get_ca(data->id);

	if (data->id >= SIE_ID_MAX_NUM) {
		PRINT_ISP_WRN(isp_dbg_get_dbg_mode(data->id) & ISP_DBG_WRN_MSG, "sie id out of range (%d) \r\n", data->id);
		return;
	}
	if (ca_rslt != NULL) {
		memcpy(&data->ca_rslt, ca_rslt, sizeof(ISP_CA_RSLT));
		data->frame_count = sie_crop_end_cnt[data->id];
	} else {
		data->frame_count = 0;
	}
}

static void ispt_api_get_la_data(ULONG addr)
{
	ISPT_LA_DATA *data = (ISPT_LA_DATA *)addr;
	ISP_LA_RSLT *la_rslt = isp_dev_get_la(data->id);

	if (data->id >= SIE_ID_MAX_NUM) {
		PRINT_ISP_WRN(isp_dbg_get_dbg_mode(data->id) & ISP_DBG_WRN_MSG, "sie id out of range (%d) \r\n", data->id);
		return;
	}
	if (la_rslt != NULL) {
		memcpy(&data->la_rslt, la_rslt, sizeof(ISP_LA_RSLT));
		data->frame_count = sie_crop_end_cnt[data->id];
	} else {
		data->frame_count = 0;
	}
}

static void ispt_api_get_va_data(ULONG addr)
{
	ISPT_IPE_VA_DATA *data = (ISPT_IPE_VA_DATA *)addr;
	ISP_IPE_VA_RSLT *va_rslt = isp_dev_get_ipe_va(data->id);

	if (nvt_get_chip_id() == CHIP_NS02402) {
		PRINT_ISP_WRN(isp_dbg_get_dbg_mode(data->id) & ISP_DBG_WRN_MSG, "NT98539A not support ipe va. \r\n");
		return;
	}

	if (data->id >= ISP_ID_MAX_NUM) {
		PRINT_ISP_WRN(isp_dbg_get_dbg_mode(data->id) & ISP_DBG_WRN_MSG, "isp id out of range (%d) \r\n", data->id);
		return;
	}
	if (va_rslt != NULL) {
		memcpy(&data->va_rslt, va_rslt, sizeof(ISP_IPE_VA_RSLT));
	}
}

static void ispt_api_get_va_indep_data(ULONG addr)
{
	ISPT_IPE_VA_INDEP_DATA *data = (ISPT_IPE_VA_INDEP_DATA *)addr;
	ISP_IPE_VA_INDEP_RSLT *va_rslt = isp_dev_get_ipe_va_indep(data->id);

	if (nvt_get_chip_id() == CHIP_NS02402) {
		PRINT_ISP_WRN(isp_dbg_get_dbg_mode(data->id) & ISP_DBG_WRN_MSG, "NT98539A not support ipe va indep. \r\n");
		return;
	}

	if (data->id >= ISP_ID_MAX_NUM) {
		PRINT_ISP_WRN(isp_dbg_get_dbg_mode(data->id) & ISP_DBG_WRN_MSG, "isp id out of range (%d) \r\n", data->id);
		return;
	}
	if (va_rslt != NULL) {
		memcpy(&data->va_indep_rslt, va_rslt, sizeof(ISP_IPE_VA_INDEP_RSLT));
	}
}

static void ispt_api_get_wait_vd(ULONG addr)
{
	ISPT_WAIT_VD *data = (ISPT_WAIT_VD *)addr;

	isp_dev_wait_vd(data->id, data->timeout, &data->frame_count);
}

static void ispt_api_get_sensor_expt(ULONG addr)
{
	ISPT_SENSOR_EXPT *data = (ISPT_SENSOR_EXPT *)addr;
	ISP_SENSOR_CTRL sensor_ctrl = {0};
	ISP_DEV_INFO *pdev_info = isp_get_dev_info();
	#if defined(__KERNEL__)
	ISP_BUILTIN_SENSOR_CTRL *built_sensor_ctrl = NULL;
	#endif

	if (pdev_info == NULL) {
		return;
	}

	if (data->id >= ISP_ID_MAX_NUM) {
		PRINT_ISP_WRN(isp_dbg_get_dbg_mode(data->id) & ISP_DBG_WRN_MSG, "isp id out of range (%d) \r\n", data->id);
		return;
	}

	if (isp_api_get_fastboot_valid() && (pdev_info->isp_ae == NULL) && !is_set_expt[data->id]) {
		#if defined(__KERNEL__)
		if (data->id < ISP_BUILTIN_ID_MAX_NUM) {
			built_sensor_ctrl = isp_builtin_get_sensor_expt(data->id);
			if (built_sensor_ctrl != NULL) {
				memcpy(&data->time, &built_sensor_ctrl->exp_time, sizeof(UINT32) * ISP_SEN_MFRAME_MAX_NUM);
			}
		} else {
			PRINT_ISP_WRN(isp_dbg_get_dbg_mode(data->id) & ISP_DBG_WRN_MSG, "fastboot does not support id (%d) \r\n", data->id);
		}
		#endif
	} else {
		isp_api_get_expt(data->id, &sensor_ctrl);
		memcpy(&data->time, &sensor_ctrl.exp_time, sizeof(UINT32) * ISP_SEN_MFRAME_MAX_NUM);
	}
}

static void ispt_api_get_sensor_gain(ULONG addr)
{
	ISPT_SENSOR_GAIN *data = (ISPT_SENSOR_GAIN *)addr;
	ISP_SENSOR_CTRL sensor_ctrl = {0};
	ISP_DEV_INFO *pdev_info = isp_get_dev_info();
	#if defined(__KERNEL__)
	ISP_BUILTIN_SENSOR_CTRL *built_sensor_ctrl = NULL;
	#endif

	if (pdev_info == NULL) {
		return;
	}

	if (data->id >= ISP_ID_MAX_NUM) {
		PRINT_ISP_WRN(isp_dbg_get_dbg_mode(data->id) & ISP_DBG_WRN_MSG, "isp id out of range (%d) \r\n", data->id);
		return;
	}

	if (isp_api_get_fastboot_valid() && (pdev_info->isp_ae == NULL) && !is_set_gain[data->id]) {
		#if defined(__KERNEL__)
		if (data->id < ISP_BUILTIN_ID_MAX_NUM) {
			built_sensor_ctrl = isp_builtin_get_sensor_gain(data->id);
			if (built_sensor_ctrl != NULL) {
				memcpy(&data->ratio, &built_sensor_ctrl->gain_ratio, sizeof(UINT32) * ISP_SEN_MFRAME_MAX_NUM);
			}
		} else {
			PRINT_ISP_WRN(isp_dbg_get_dbg_mode(data->id) & ISP_DBG_WRN_MSG, "fastboot does not support id (%d) \r\n", data->id);
		}
		#endif
	} else {
		isp_api_get_gain(data->id, &sensor_ctrl);
		memcpy(&data->ratio, &sensor_ctrl.gain_ratio, sizeof(UINT32) * ISP_SEN_MFRAME_MAX_NUM);
	}
}

static void ispt_api_get_d_gain(ULONG addr)
{
	ISPT_D_GAIN *data = (ISPT_D_GAIN *)addr;
	ISP_DEV_INFO *pdev_info = isp_get_dev_info();

	if (pdev_info == NULL) {
		return;
	}

	if (data->id >= SIE_ID_MAX_NUM) {
		PRINT_ISP_WRN(isp_dbg_get_dbg_mode(data->id) & ISP_DBG_WRN_MSG, "sie id out of range (%d) \r\n", data->id);
		return;
	}

	if (isp_api_get_fastboot_valid() && (pdev_info->isp_ae == NULL) && !is_set_d_gain[data->id]) {
		#if defined(__KERNEL__)
		if (data->id < ISP_BUILTIN_ID_MAX_NUM) {
			data->gain = isp_builtin_get_dgain(data->id);
		} else {
			PRINT_ISP_WRN(isp_dbg_get_dbg_mode(data->id) & ISP_DBG_WRN_MSG, "fastboot does not support id (%d) \r\n", data->id);
		}
		#endif
	} else {
		isp_dev_get_sync_item(data->id, ISP_SYNC_SEL_IPP, ISP_SYNC_AE_DGAIN, &data->gain);
	}
}

static void ispt_api_get_c_gain(ULONG addr)
{
	ISPT_C_GAIN *data = (ISPT_C_GAIN *)addr;
	ISP_DEV_INFO *pdev_info = isp_get_dev_info();
	#if defined(__KERNEL__)
	ISP_BUILTIN_CGAIN *builtin_cgain = NULL;
	#endif

	if (pdev_info == NULL) {
		return;
	}

	if (data->id >= ISP_ID_MAX_NUM) {
		PRINT_ISP_WRN(isp_dbg_get_dbg_mode(data->id) & ISP_DBG_WRN_MSG, "isp id out of range (%d) \r\n", data->id);
		return;
	}

	if (isp_api_get_fastboot_valid() && (pdev_info->isp_awb == NULL) && !is_set_c_gain[data->id]) {
		#if defined(__KERNEL__)
		if (data->id < ISP_BUILTIN_ID_MAX_NUM) {
			builtin_cgain = isp_builtin_get_cgain(data->id);
			data->gain[0] = builtin_cgain->r;
			data->gain[1] = builtin_cgain->g;
			data->gain[2] = builtin_cgain->b;
		} else {
			PRINT_ISP_WRN(isp_dbg_get_dbg_mode(data->id) & ISP_DBG_WRN_MSG, "fastboot does not support id (%d) \r\n", data->id);
		}
		#endif
	} else {
		isp_dev_get_sync_item(data->id, ISP_SYNC_SEL_IPP, ISP_SYNC_AWB_CGAIN, &data->gain);
	}
}

static void ispt_api_get_total_gain(ULONG addr)
{
	ISPT_TOTAL_GAIN *data = (ISPT_TOTAL_GAIN *)addr;
	ISP_DEV_INFO *pdev_info = isp_get_dev_info();

	if (pdev_info == NULL) {
		return;
	}

	if (data->id >= ISP_ID_MAX_NUM) {
		PRINT_ISP_WRN(isp_dbg_get_dbg_mode(data->id) & ISP_DBG_WRN_MSG, "isp id out of range (%d) \r\n", data->id);
		return;
	}

	if (isp_api_get_fastboot_valid() && (pdev_info->isp_ae == NULL) && !is_set_total_gain[data->id]) {
		#if defined(__KERNEL__)
		if (data->id < ISP_BUILTIN_ID_MAX_NUM) {
			data->gain = isp_builtin_get_total_gain(data->id);
		} else {
			PRINT_ISP_WRN(isp_dbg_get_dbg_mode(data->id) & ISP_DBG_WRN_MSG, "fastboot does not support id (%d) \r\n", data->id);
		}
		#endif
	} else {
		isp_dev_get_sync_item(data->id, ISP_SYNC_SEL_IPP, ISP_SYNC_AE_TOTAL_GAIN, &data->gain);
	}
}

static void ispt_api_get_lv(ULONG addr)
{
	ISPT_LV *data = (ISPT_LV *)addr;

	if (data->id >= ISP_ID_MAX_NUM) {
		PRINT_ISP_WRN(isp_dbg_get_dbg_mode(data->id) & ISP_DBG_WRN_MSG, "isp id out of range (%d) \r\n", data->id);
		return;
	}
	isp_dev_get_sync_item(data->id, ISP_SYNC_SEL_IPP, ISP_SYNC_AE_LV, &data->lv);
}

static void ispt_api_get_ct(ULONG addr)
{
	ISPT_CT *data = (ISPT_CT *)addr;

	if (data->id >= ISP_ID_MAX_NUM) {
		PRINT_ISP_WRN(isp_dbg_get_dbg_mode(data->id) & ISP_DBG_WRN_MSG, "isp id out of range (%d) \r\n", data->id);
		return;
	}
	isp_dev_get_sync_item(data->id, ISP_SYNC_SEL_IPP, ISP_SYNC_AWB_CT, &data->ct);
}

static void ispt_api_get_motor_iris(ULONG addr)
{
	ISPT_MOTOR_IRIS *data = (ISPT_MOTOR_IRIS *)addr;

	isp_api_get_iris(data->cmd_type, &data->ctl_cmd);
}

static void ispt_api_get_motor_focus(ULONG addr)
{
	ISPT_MOTOR_FOCUS *data = (ISPT_MOTOR_FOCUS *)addr;

	isp_api_get_focus(data->cmd_type, &data->ctl_cmd);
}

static void ispt_api_get_motor_zoom(ULONG addr)
{
	ISPT_MOTOR_ZOOM *data = (ISPT_MOTOR_ZOOM *)addr;

	isp_api_get_zoom(data->cmd_type, &data->ctl_cmd);
}

static void ispt_api_get_motor_misc(ULONG addr)
{
	ISPT_MOTOR_MISC *data = (ISPT_MOTOR_MISC *)addr;

	isp_api_get_misc(data->cmd_type, &data->ctl_cmd);
}

static void ispt_api_get_sensor_direction(ULONG addr)
{
	ISPT_SENSOR_DIRECTION *data = (ISPT_SENSOR_DIRECTION *)addr;

	if (data->id >= ISP_ID_MAX_NUM) {
		PRINT_ISP_WRN(isp_dbg_get_dbg_mode(data->id) & ISP_DBG_WRN_MSG, "isp id out of range (%d) \r\n", data->id);
		return;
	}

	isp_api_get_direction(data->id, &data->direction);
}

static void ispt_api_get_histo_data(ULONG addr)
{
	ISPT_HISTO_DATA *data = (ISPT_HISTO_DATA *)addr;
	ISP_HISTO_RSLT *histo_rslt = isp_dev_get_histo(data->id);

	if (data->id >= ISP_ID_MAX_NUM) {
		PRINT_ISP_WRN(isp_dbg_get_dbg_mode(data->id) & ISP_DBG_WRN_MSG, "isp id out of range (%d) \r\n", data->id);
		return;
	}
	if (histo_rslt != NULL) {
		memcpy(&data->histo_rslt, histo_rslt, sizeof(ISP_HISTO_RSLT));
	}
}

static void ispt_api_get_ir_info(ULONG addr)
{
	ISPT_IR_INFO *data = (ISPT_IR_INFO *)addr;

	if (data->id >= SIE_ID_MAX_NUM) {
		PRINT_ISP_WRN(isp_dbg_get_dbg_mode(data->id) & ISP_DBG_WRN_MSG, "sie id out of range (%d) \r\n", data->id);
		return;
	}
	isp_api_get_ir_info(data->id, &data->info);
}

static void ispt_api_get_shdr_ev_ratio(ULONG addr)
{
	ISPT_SHDR_EV_RATIO *data = (ISPT_SHDR_EV_RATIO *)addr;
	ISP_DEV_INFO *pdev_info = isp_get_dev_info();
	#if defined(__KERNEL__)
	ISP_BUILTIN_SHDR_EV_RATIO *builtin_shdr_ev_ratio = NULL;
	#endif

	if (pdev_info == NULL) {
		return;
	}

	if (data->id >= ISP_ID_MAX_NUM) {
		PRINT_ISP_WRN(isp_dbg_get_dbg_mode(data->id) & ISP_DBG_WRN_MSG, "isp id out of range (%d) \r\n", data->id);
		return;
	}

	if (isp_api_get_fastboot_valid() && (pdev_info->isp_ae == NULL) && !is_set_shdr_ev_ratio[data->id]) {
		#if defined(__KERNEL__)
		if (data->id < ISP_BUILTIN_ID_MAX_NUM) {
			builtin_shdr_ev_ratio = isp_builtin_get_shdr_ev_ratio(data->id);
			data->ratio[0] = builtin_shdr_ev_ratio->ratio[0];
			data->ratio[1] = builtin_shdr_ev_ratio->ratio[1];
		} else {
			PRINT_ISP_WRN(isp_dbg_get_dbg_mode(data->id) & ISP_DBG_WRN_MSG, "fastboot does not support id (%d) \r\n", data->id);
		}
		#endif
	} else {
		isp_dev_get_sync_item(data->id, ISP_SYNC_SEL_IPP, ISP_SYNC_AE_SHDR_EV_RATIO, &data->ratio);
	}
}

static void ispt_api_get_ca_enable(ULONG addr)
{
	ISPT_CA_ENABLE *data = (ISPT_CA_ENABLE *)addr;

	if (data->id >= SIE_ID_MAX_NUM) {
		PRINT_ISP_WRN(isp_dbg_get_dbg_mode(data->id) & ISP_DBG_WRN_MSG, "sie id out of range (%d) \r\n", data->id);
		return;
	}
	isp_dev_get_sync_item(data->id, ISP_SYNC_SEL_SIE, ISP_SYNC_AWB_CA_ENABLE, &data->enable);
}

static void ispt_api_get_la_enable(ULONG addr)
{
	ISPT_LA_ENABLE *data = (ISPT_LA_ENABLE *)addr;

	if (data->id >= SIE_ID_MAX_NUM) {
		PRINT_ISP_WRN(isp_dbg_get_dbg_mode(data->id) & ISP_DBG_WRN_MSG, "sie id out of range (%d) \r\n", data->id);
		return;
	}
	isp_dev_get_sync_item(data->id, ISP_SYNC_SEL_SIE, ISP_SYNC_AE_LA_ENABLE, &data->enable);
}

static void ispt_api_get_ife_va_indep_roi(ULONG addr)
{
	ISPT_VA_INDEP_ROI *data = (ISPT_VA_INDEP_ROI *)addr;

	if (data->id >= ISP_ID_MAX_NUM) {
		PRINT_ISP_WRN(isp_dbg_get_dbg_mode(data->id) & ISP_DBG_WRN_MSG, "isp id out of range (%d) \r\n", data->id);
		return;
	}
	isp_dev_get_sync_item(data->id, ISP_SYNC_SEL_CURRENT, ISP_SYNC_IFE_VA_INDEP_ROI, &data->roi[0]);
}

static void ispt_api_get_ife_va_data(ULONG addr)
{
	ISPT_IFE_VA_DATA *data = (ISPT_IFE_VA_DATA *)addr;
	ISP_IFE_VA_RSLT *va_rslt = isp_dev_get_ife_va(data->id);

	if (data->id >= ISP_ID_MAX_NUM) {
		PRINT_ISP_WRN(isp_dbg_get_dbg_mode(data->id) & ISP_DBG_WRN_MSG, "isp id out of range (%d) \r\n", data->id);
		return;
	}
	if (va_rslt != NULL) {
		memcpy(&data->va_rslt, va_rslt, sizeof(ISP_IFE_VA_RSLT));
	}
}

static void ispt_api_get_ife_va_indep_data(ULONG addr)
{
	ISPT_IFE_VA_INDEP_DATA *data = (ISPT_IFE_VA_INDEP_DATA *)addr;
	ISP_IFE_VA_INDEP_RSLT *va_rslt = isp_dev_get_ife_va_indep(data->id);

	if (data->id >= ISP_ID_MAX_NUM) {
		PRINT_ISP_WRN(isp_dbg_get_dbg_mode(data->id) & ISP_DBG_WRN_MSG, "isp id out of range (%d) \r\n", data->id);
		return;
	}
	if (va_rslt != NULL) {
		memcpy(&data->va_indep_rslt, va_rslt, sizeof(ISP_IFE_VA_INDEP_RSLT));
	}
}

static void ispt_api_get_wait_frmend(ULONG addr)
{
	ISPT_WAIT_FRMEND *data = (ISPT_WAIT_FRMEND *)addr;

	isp_dev_wait_frmend(data->id, data->timeout, &data->frame_count);
}

static void ispt_api_get_wait_cfgstr(ULONG addr)
{
	ISPT_WAIT_CFGSTR *data = (ISPT_WAIT_CFGSTR *)addr;

	isp_dev_wait_cfgstr(data->id, data->timeout, &data->frame_count);
}

static void ispt_api_get_wait_procend(ULONG addr)
{
	ISPT_WAIT_PROCEND *data = (ISPT_WAIT_PROCEND *)addr;

	isp_dev_wait_procend(data->id, data->timeout, &data->frame_count);
}

static void ispt_api_get_vprc_indep(ULONG addr)
{
	ISPT_VPRC_INDEP *data = (ISPT_VPRC_INDEP *)addr;

	if (data->id >= ISP_ID_MAX_NUM) {
		PRINT_ISP_WRN(isp_dbg_get_dbg_mode(data->id) & ISP_DBG_WRN_MSG, "isp id out of range (%d) \r\n", data->id);
		return;
	}
	data->enable = isp_get_ipp_indep(data->id);
}

static void ispt_api_get_sensor_mode_manual(ULONG addr)
{
	ISPT_SENSOR_MODE_MANUAL *data = (ISPT_SENSOR_MODE_MANUAL *)addr;

	if (data->id >= ISP_ID_MAX_NUM) {
		PRINT_ISP_WRN(isp_dbg_get_dbg_mode(data->id) & ISP_DBG_WRN_MSG, "isp id out of range (%d) \r\n", data->id);
		return;
	}
	isp_get_sensor_mode_manual(data->id, &data->manual);
}

static void ispt_api_get_low_power_lv(ULONG addr)
{
	UINT32 *data = (UINT32 *)addr;

	*data = isp_api_get_low_power_lv();
}

UINT32 ispt_api_get_item_size(ISPT_ITEM item)
{
	return ispt_info.size_tab[item][1];
}

ER ispt_api_get_cmd(ISPT_ITEM item, ULONG addr)
{
	if (ispt_get_tab[item] == NULL) {
		DBG_ERR("ispt_get_tab(%d) NULL \r\n", item);
		return E_SYS;
	}
	if (item >= ISPT_ITEM_MAX) {
		DBG_ERR("item(%d) out of range \r\n", item);
		return E_SYS;
	}

	ispt_get_tab[item](addr);
	return E_OK;
}

static void ispt_api_set_yuv(ULONG addr)
{
	UINT32 *data = (UINT32 *)addr;

	isp_api_set_yuv(*data);
}

static void ispt_api_set_raw(ULONG addr)
{
	UINT32 *data = (UINT32 *)addr;

	if (*data >= SIE_ID_MAX_NUM) {
		PRINT_ISP_WRN(isp_dbg_get_dbg_mode(*data) & ISP_DBG_WRN_MSG, "sie id out of range (%d) \r\n", *data);
		return;
	}

	isp_api_set_raw(*data);
}

static void ispt_api_set_frame(ULONG addr)
{
	ISPT_MEMORY_INFO *data = (ISPT_MEMORY_INFO *)addr;

	memcpy((UINT32 *)data->addr, (UINT32 *)&data->buf, data->size);
}

static void ispt_api_set_memory(ULONG addr)
{
	ISPT_MEMORY_INFO *data = (ISPT_MEMORY_INFO *)addr;
	#if defined(__FREERTOS)
	volatile ULONG base_addr;
	#else
	ULONG base_addr;
	#endif
	UINT32 i;

	if (data->addr == 0) {
		DBG_ERR("phyaddr = 0x%lx is not available ! \r\n", data->addr);
	}

	#if defined(__FREERTOS)
	base_addr = data->addr;

	for (i = 0; i < data->size; i+=4) {
		memcpy((UINT32 *)(base_addr+i), (UINT32 *)&data->buf[i], sizeof(UINT32));
	}

	#else
	base_addr = (ULONG)ioremap(data->addr, data->size);


	for (i = 0; i < data->size; i+=4) {
		memcpy((void *)(base_addr+i), (UINT32 *)&data->buf[i], sizeof(UINT32));
	}
	#endif
}

static void ispt_api_set_sensor_reg(ULONG addr)
{
	ISPT_SENSOR_REG *data = (ISPT_SENSOR_REG *)addr;

	if (data->id >= ISP_ID_MAX_NUM) {
		PRINT_ISP_WRN(isp_dbg_get_dbg_mode(data->id) & ISP_DBG_WRN_MSG, "isp id out of range (%d) \r\n", data->id);
		return;
	}
	isp_api_set_sensor_reg(data->id, &data->addr, &data->data);
}

static void ispt_api_set_ca_roi(ULONG addr)
{
	ISPT_CA_ROI *data = (ISPT_CA_ROI *)addr;

	if (data->id >= SIE_ID_MAX_NUM) {
		PRINT_ISP_WRN(isp_dbg_get_dbg_mode(data->id) & ISP_DBG_WRN_MSG, "sie id out of range (%d) \r\n", data->id);
		return;
	}
	isp_dev_set_sync_item(data->id, ISP_SYNC_SEL_CURRENT, ISP_SYNC_CA_ROI, &data->roi);
}

static void ispt_api_set_la_roi(ULONG addr)
{
	ISPT_LA_ROI *data = (ISPT_LA_ROI *)addr;

	if (data->id >= SIE_ID_MAX_NUM) {
		PRINT_ISP_WRN(isp_dbg_get_dbg_mode(data->id) & ISP_DBG_WRN_MSG, "sie id out of range (%d) \r\n", data->id);
		return;
	}
	isp_dev_set_sync_item(data->id, ISP_SYNC_SEL_CURRENT, ISP_SYNC_LA_ROI, &data->roi);
}

static void ispt_api_set_va_indep_roi(ULONG addr)
{
	ISPT_VA_INDEP_ROI *data = (ISPT_VA_INDEP_ROI *)addr;

	if (nvt_get_chip_id() == CHIP_NS02402) {
		PRINT_ISP_WRN(isp_dbg_get_dbg_mode(data->id) & ISP_DBG_WRN_MSG, "NT98539A not support ipe va indep roi. \r\n");
		return;
	}

	if (data->id >= ISP_ID_MAX_NUM) {
		PRINT_ISP_WRN(isp_dbg_get_dbg_mode(data->id) & ISP_DBG_WRN_MSG, "isp id out of range (%d) \r\n", data->id);
		return;
	}
	if (((data->roi[0].w == 0) || (data->roi[0].h == 0) || (data->roi[0].x + data->roi[0].w > 1000) || (data->roi[0].y + data->roi[0].h > 1000)) ||
		((data->roi[1].w == 0) || (data->roi[1].h == 0) || (data->roi[1].x + data->roi[1].w > 1000) || (data->roi[1].y + data->roi[1].h > 1000)) ||
		((data->roi[2].w == 0) || (data->roi[2].h == 0) || (data->roi[2].x + data->roi[2].w > 1000) || (data->roi[2].y + data->roi[2].h > 1000)) ||
		((data->roi[3].w == 0) || (data->roi[3].h == 0) || (data->roi[3].x + data->roi[3].w > 1000) || (data->roi[3].y + data->roi[3].h > 1000)) ||
		((data->roi[4].w == 0) || (data->roi[4].h == 0) || (data->roi[4].x + data->roi[4].w > 1000) || (data->roi[4].y + data->roi[4].h > 1000))) {
		PRINT_ISP_WRN(isp_dbg_get_dbg_mode(data->id) & ISP_DBG_WRN_MSG, "isp va_indep_roi out of range \r\n");
	}
	isp_dev_set_sync_item(data->id, ISP_SYNC_SEL_CURRENT, ISP_SYNC_VA_INDEP_ROI, &data->roi[0]);
}

static void ispt_api_set_sensor_expt(ULONG addr)
{
	ISPT_SENSOR_EXPT *data = (ISPT_SENSOR_EXPT *)addr;
	ISP_SENSOR_CTRL sensor_ctrl = {0};

	if (data->id >= ISP_ID_MAX_NUM) {
		PRINT_ISP_WRN(isp_dbg_get_dbg_mode(data->id) & ISP_DBG_WRN_MSG, "isp id out of range (%d) \r\n", data->id);
		return;
	}
	memcpy(&sensor_ctrl.exp_time, &data->time, sizeof(UINT32)*ISP_SEN_MFRAME_MAX_NUM);
	isp_api_set_expt(data->id, &sensor_ctrl);

	is_set_expt[data->id] = TRUE;
}

static void ispt_api_set_sensor_gain(ULONG addr)
{
	ISPT_SENSOR_GAIN *data = (ISPT_SENSOR_GAIN *)addr;
	ISP_SENSOR_CTRL sensor_ctrl = {0};

	if (data->id >= ISP_ID_MAX_NUM) {
		PRINT_ISP_WRN(isp_dbg_get_dbg_mode(data->id) & ISP_DBG_WRN_MSG, "isp id out of range (%d) \r\n", data->id);
		return;
	}
	memcpy(&sensor_ctrl.gain_ratio, &data->ratio, sizeof(UINT32)*ISP_SEN_MFRAME_MAX_NUM);
	isp_api_set_gain(data->id, &sensor_ctrl);

	is_set_gain[data->id] = TRUE;
}

static void ispt_api_set_d_gain(ULONG addr)
{
	ISPT_D_GAIN *data = (ISPT_D_GAIN *)addr;
	ISP_AE_INIT_INFO ae_init_info = {0};

	if (data->id >= SIE_ID_MAX_NUM) {
		PRINT_ISP_WRN(isp_dbg_get_dbg_mode(data->id) & ISP_DBG_WRN_MSG, "sie id out of range (%d) \r\n", data->id);
		return;
	}
	isp_dev_set_sync_item(data->id, ISP_SYNC_SEL_SIE, ISP_SYNC_AE_DGAIN, &data->gain);

	isp_dev_get_sync_item(data->id, ISP_SYNC_SEL_CURRENT, ISP_SYNC_AE_INIT, &ae_init_info);
	ae_init_info.d_gain= data->gain;
	isp_dev_set_sync_item(data->id, ISP_SYNC_SEL_CURRENT, ISP_SYNC_AE_INIT, &ae_init_info);

	isp_dev_set_sync_item(data->id, ISP_SYNC_SEL_CURRENT, ISP_SYNC_CAPTURE_DGAIN, &data->gain);

	is_set_d_gain[data->id] = TRUE;
}

static void ispt_api_set_c_gain(ULONG addr)
{
	ISPT_C_GAIN *data = (ISPT_C_GAIN *)addr;
	ISP_AWB_INIT_INFO awb_init_info = {0};

	if (data->id >= ISP_ID_MAX_NUM) {
		PRINT_ISP_WRN(isp_dbg_get_dbg_mode(data->id) & ISP_DBG_WRN_MSG, "isp id out of range (%d) \r\n", data->id);
		return;
	}
	isp_dev_set_sync_item(data->id, ISP_SYNC_SEL_SIE, ISP_SYNC_AWB_CGAIN, &data->gain);

	isp_dev_get_sync_item(data->id, ISP_SYNC_SEL_CURRENT, ISP_SYNC_AWB_INIT, &awb_init_info);
	awb_init_info.r_gain = data->gain[0];
	awb_init_info.g_gain = data->gain[1];
	awb_init_info.b_gain = data->gain[2];
	isp_dev_set_sync_item(data->id, ISP_SYNC_SEL_CURRENT, ISP_SYNC_AWB_INIT, &awb_init_info);

	isp_dev_set_sync_item(data->id, ISP_SYNC_SEL_CURRENT, ISP_SYNC_CAPTURE_CGAIN, &data->gain);

	is_set_c_gain[data->id] = TRUE;
}

static void ispt_api_set_total_gain(ULONG addr)
{
	ISPT_TOTAL_GAIN *data = (ISPT_TOTAL_GAIN *)addr;
	ISP_AE_INIT_INFO ae_init_info = {0};

	if (data->id >= ISP_ID_MAX_NUM) {
		PRINT_ISP_WRN(isp_dbg_get_dbg_mode(data->id) & ISP_DBG_WRN_MSG, "isp id out of range (%d) \r\n", data->id);
		return;
	}
	isp_dev_set_sync_item(data->id, ISP_SYNC_SEL_SIE, ISP_SYNC_AE_TOTAL_GAIN, &data->gain);

	isp_dev_get_sync_item(data->id, ISP_SYNC_SEL_CURRENT, ISP_SYNC_AE_INIT, &ae_init_info);
	ae_init_info.total_gain= data->gain;
	isp_dev_set_sync_item(data->id, ISP_SYNC_SEL_CURRENT, ISP_SYNC_AE_INIT, &ae_init_info);

	isp_dev_set_sync_item(data->id, ISP_SYNC_SEL_CURRENT, ISP_SYNC_CAPTURE_TOTAL_GAIN, &data->gain);

	is_set_total_gain[data->id] = TRUE;
}

static void ispt_api_set_lv(ULONG addr)
{
	ISPT_LV *data = (ISPT_LV *)addr;
	ISP_AE_INIT_INFO ae_init_info = {0};

	if (data->id >= ISP_ID_MAX_NUM) {
		PRINT_ISP_WRN(isp_dbg_get_dbg_mode(data->id) & ISP_DBG_WRN_MSG, "isp id out of range (%d) \r\n", data->id);
		return;
	}
	isp_dev_set_sync_item(data->id, ISP_SYNC_SEL_SIE, ISP_SYNC_AE_LV, &data->lv);

	isp_dev_get_sync_item(data->id, ISP_SYNC_SEL_CURRENT, ISP_SYNC_AE_INIT, &ae_init_info);
	ae_init_info.lv= data->lv;
	isp_dev_set_sync_item(data->id, ISP_SYNC_SEL_CURRENT, ISP_SYNC_AE_INIT, &ae_init_info);
}

static void ispt_api_set_ct(ULONG addr)
{
	ISPT_CT *data = (ISPT_CT *)addr;
	ISP_AWB_INIT_INFO awb_init_info = {0};

	if (data->id >= ISP_ID_MAX_NUM) {
		PRINT_ISP_WRN(isp_dbg_get_dbg_mode(data->id) & ISP_DBG_WRN_MSG, "isp id out of range (%d) \r\n", data->id);
		return;
	}
	isp_dev_set_sync_item(data->id, ISP_SYNC_SEL_SIE, ISP_SYNC_AWB_CT, &data->ct);

	isp_dev_get_sync_item(data->id, ISP_SYNC_SEL_CURRENT, ISP_SYNC_AWB_INIT, &awb_init_info);
	awb_init_info.ct = data->ct;
	isp_dev_set_sync_item(data->id, ISP_SYNC_SEL_CURRENT, ISP_SYNC_AWB_INIT, &awb_init_info);
}

static void ispt_api_set_motor_iris(ULONG addr)
{
	ISPT_MOTOR_IRIS *data = (ISPT_MOTOR_IRIS *)addr;

	isp_api_set_iris(data->cmd_type, &data->ctl_cmd);
}

static void ispt_api_set_motor_focus(ULONG addr)
{
	ISPT_MOTOR_FOCUS *data = (ISPT_MOTOR_FOCUS *)addr;

	isp_api_set_focus(data->cmd_type, &data->ctl_cmd);
}

static void ispt_api_set_motor_zoom(ULONG addr)
{
	ISPT_MOTOR_ZOOM *data = (ISPT_MOTOR_ZOOM *)addr;

	isp_api_set_zoom(data->cmd_type, &data->ctl_cmd);
}

static void ispt_api_set_motor_misc(ULONG addr)
{
	ISPT_MOTOR_MISC *data = (ISPT_MOTOR_MISC *)addr;

	isp_api_set_misc(data->cmd_type, &data->ctl_cmd);
}

static void ispt_api_set_sensor_direction(ULONG addr)
{
	ISPT_SENSOR_DIRECTION *data = (ISPT_SENSOR_DIRECTION *)addr;

	if (data->id >= ISP_ID_MAX_NUM) {
		PRINT_ISP_WRN(isp_dbg_get_dbg_mode(data->id) & ISP_DBG_WRN_MSG, "isp id out of range (%d) \r\n", data->id);
		return;
	}

	isp_api_set_direction(data->id, &data->direction);
}

static void ispt_api_set_sensor_sleep(ULONG addr)
{
	UINT32 *data = (UINT32 *)addr;

	isp_api_set_sensor_sleep(*data);
}

static void ispt_api_set_sensor_wakeup(ULONG addr)
{
	UINT32 *data = (UINT32 *)addr;

	isp_api_set_sensor_wakeup(*data);
}

static void ispt_api_set_shdr_ev_ratio(ULONG addr)
{
	ISPT_SHDR_EV_RATIO *data = (ISPT_SHDR_EV_RATIO *)addr;
	ISP_AE_INIT_INFO ae_init_info = {0};

	if (data->id >= ISP_ID_MAX_NUM) {
		PRINT_ISP_WRN(isp_dbg_get_dbg_mode(data->id) & ISP_DBG_WRN_MSG, "isp id out of range (%d) \r\n", data->id);
		return;
	}
	isp_dev_set_sync_item(data->id, ISP_SYNC_SEL_SIE, ISP_SYNC_AE_SHDR_EV_RATIO, &data->ratio);

	isp_dev_get_sync_item(data->id, ISP_SYNC_SEL_CURRENT, ISP_SYNC_AE_INIT, &ae_init_info);
	ae_init_info.shdr_ev_ratio[0] = data->ratio[0];
	ae_init_info.shdr_ev_ratio[1] = data->ratio[1];
	isp_dev_set_sync_item(data->id, ISP_SYNC_SEL_CURRENT, ISP_SYNC_AE_INIT, &ae_init_info);

	is_set_shdr_ev_ratio[data->id] = TRUE;
}

static void ispt_api_set_ca_enable(ULONG addr)
{
	ISPT_CA_ENABLE *data = (ISPT_CA_ENABLE *)addr;

	if (data->id >= SIE_ID_MAX_NUM) {
		PRINT_ISP_WRN(isp_dbg_get_dbg_mode(data->id) & ISP_DBG_WRN_MSG, "sie id out of range (%d) \r\n", data->id);
		return;
	}
	isp_dev_set_sync_item(data->id, ISP_SYNC_SEL_SIE, ISP_SYNC_AWB_CA_ENABLE, &data->enable);
}

static void ispt_api_set_la_enable(ULONG addr)
{
	ISPT_LA_ENABLE *data = (ISPT_LA_ENABLE *)addr;

	if (data->id >= SIE_ID_MAX_NUM) {
		PRINT_ISP_WRN(isp_dbg_get_dbg_mode(data->id) & ISP_DBG_WRN_MSG, "sie id out of range (%d) \r\n", data->id);
		return;
	}
	isp_dev_set_sync_item(data->id, ISP_SYNC_SEL_SIE, ISP_SYNC_AE_LA_ENABLE, &data->enable);
}

static void ispt_api_set_ife_va_indep_roi(ULONG addr)
{
	ISPT_VA_INDEP_ROI *data = (ISPT_VA_INDEP_ROI *)addr;

	if (data->id >= ISP_ID_MAX_NUM) {
		PRINT_ISP_WRN(isp_dbg_get_dbg_mode(data->id) & ISP_DBG_WRN_MSG, "isp id out of range (%d) \r\n", data->id);
		return;
	}
	if (((data->roi[0].w == 0) || (data->roi[0].h == 0) || (data->roi[0].x + data->roi[0].w > 1000) || (data->roi[0].y + data->roi[0].h > 1000)) ||
		((data->roi[1].w == 0) || (data->roi[1].h == 0) || (data->roi[1].x + data->roi[1].w > 1000) || (data->roi[1].y + data->roi[1].h > 1000)) ||
		((data->roi[2].w == 0) || (data->roi[2].h == 0) || (data->roi[2].x + data->roi[2].w > 1000) || (data->roi[2].y + data->roi[2].h > 1000)) ||
		((data->roi[3].w == 0) || (data->roi[3].h == 0) || (data->roi[3].x + data->roi[3].w > 1000) || (data->roi[3].y + data->roi[3].h > 1000)) ||
		((data->roi[4].w == 0) || (data->roi[4].h == 0) || (data->roi[4].x + data->roi[4].w > 1000) || (data->roi[4].y + data->roi[4].h > 1000))) {
		PRINT_ISP_WRN(isp_dbg_get_dbg_mode(data->id) & ISP_DBG_WRN_MSG, "isp ife_va_indep_roi out of range \r\n");
	}
	isp_dev_set_sync_item(data->id, ISP_SYNC_SEL_CURRENT, ISP_SYNC_IFE_VA_INDEP_ROI, &data->roi[0]);
}

static void ispt_api_set_vprc_indep(ULONG addr)
{
	ISPT_VPRC_INDEP *data = (ISPT_VPRC_INDEP *)addr;

	if (data->id >= ISP_ID_MAX_NUM) {
		PRINT_ISP_WRN(isp_dbg_get_dbg_mode(data->id) & ISP_DBG_WRN_MSG, "isp id out of range (%d) \r\n", data->id);
		return;
	}
	isp_set_ipp_indep(data->id, data->enable);
}


static void ispt_api_set_sensor_mode_manual(ULONG addr)
{
	ISPT_SENSOR_MODE_MANUAL *data = (ISPT_SENSOR_MODE_MANUAL *)addr;

	if (data->id >= ISP_ID_MAX_NUM) {
		PRINT_ISP_WRN(isp_dbg_get_dbg_mode(data->id) & ISP_DBG_WRN_MSG, "isp id out of range (%d) \r\n", data->id);
		return;
	}
	isp_set_sensor_mode_manual(data->id, &data->manual);
}

static void ispt_api_set_low_power_lv(ULONG addr)
{
	UINT32 *data = (UINT32 *)addr;

	isp_api_set_low_power_lv(*data);
}

static void ispt_api_reserve(ULONG addr)
{
	return;
}

ER ispt_api_set_cmd(ISPT_ITEM item, ULONG addr)
{
	if (ispt_set_tab[item] == NULL) {
		DBG_ERR("ispt_set_tab(%d) NULL \r\n", item);
		return E_SYS;
	}
	if (item >= ISPT_ITEM_MAX) {
		DBG_ERR("item(%d) out of range \r\n", item);
		return E_SYS;
	}

	ispt_set_tab[item](addr);
	return E_OK;
}

