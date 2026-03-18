/**
	@brief Source file of vendor media videocapture.\n

	@file vendor_videocapture.c

	@ingroup mhdal

	@note Nothing.

	Copyright Novatek Microelectronics Corp. 2018.  All rights reserved.
*/
#if defined(__LINUX)
#include <sys/ioctl.h>
#endif
#include <string.h>
#include "hdal.h"
#define HD_MODULE_NAME VENDOR_VIDEOCAPTURE

#include "kflow_common/isf_flow_def.h"
#include "kflow_common/isf_flow_ioctl.h"
#include "vendor_videocapture.h"
#include "kflow_videocapture/isf_vdocap.h"
#define _FASTBOOT_2A_TO_DO_ 1
#if defined(__FREERTOS)
#if _FASTBOOT_2A_TO_DO_
#include <nvtmpp_init.h>
#include "sie_init.h"
#endif
#include "kflow_videocapture/ctl_sie.h"
#endif
#include "kflow_common/isf_flow_def.h"
#include "kflow_common/isf_flow_ioctl.h"
#if defined (__FREERTOS)
#include "isp_api.h"
#include "plat/top.h"
#endif
#include "hd_logger_p.h"
#if defined (__FREERTOS)
#define ISF_OPEN     isf_flow_open
#define ISF_IOCTL    isf_flow_ioctl
#define ISF_CLOSE    isf_flow_close
#endif
#if defined(__LINUX)
#define ISF_OPEN     open
#define ISF_IOCTL    ioctl
#define ISF_CLOSE    close
#endif
#include <kwrap/util.h>		//for sleep API

#define THIS_DBGLVL         2 // 0=FATAL, 1=ERR, 2=WRN, 3=UNIT, 4=FUNC, 5=IND, 6=MSG, 7=VALUE, 8=USER
///////////////////////////////////////////////////////////////////////////////
#define __MODULE__          VENDOR_VIDEOCAP
#define __DBGLVL__          THIS_DBGLVL
#define __DBGFLT__          "*" //*=All, [mark]=CustomClass
#include "kwrap/debug.h"

#define HD_VCAP_DBG_FATAL     0
#define HD_VCAP_DBG_ERR       1
#define HD_VCAP_DBG_WRN       2
#define HD_VCAP_DBG_MSG       3
#define HD_VCAP_DBG_IND       4
#define HD_VCAP_DBG_FUNC      5


#define HD_VCAP_FLOW_ERR(fmt, args...) { if (hd_flow_dbg_lvl[HD_FLOW_DBG_VCAP] && (*hd_flow_dbg_lvl[HD_FLOW_DBG_VCAP]) >= HD_VCAP_DBG_ERR) { hdal_flow_log_p(fmt, ##args);  }}
#define HD_VCAP_FLOW_WRN(fmt, args...) { if (hd_flow_dbg_lvl[HD_FLOW_DBG_VCAP] && (*hd_flow_dbg_lvl[HD_FLOW_DBG_VCAP]) >= HD_VCAP_DBG_WRN) { hdal_flow_log_p(fmt, ##args);  }}
#define HD_VCAP_FLOW_MSG(fmt, args...) { if (hd_flow_dbg_lvl[HD_FLOW_DBG_VCAP] && (*hd_flow_dbg_lvl[HD_FLOW_DBG_VCAP]) >= HD_VCAP_DBG_MSG) { hdal_flow_log_p(fmt, ##args);  }}
#define HD_VCAP_FLOW_IND(fmt, args...) { if (hd_flow_dbg_lvl[HD_FLOW_DBG_VCAP] && (*hd_flow_dbg_lvl[HD_FLOW_DBG_VCAP]) >= HD_VCAP_DBG_IND) { hdal_flow_log_p(fmt, ##args);  }}
#define HD_VCAP_FLOW_FUNC(fmt, args...) { if (hd_flow_dbg_lvl[HD_FLOW_DBG_VCAP] && (*hd_flow_dbg_lvl[HD_FLOW_DBG_VCAP]) >= HD_VCAP_DBG_FUNC) { hdal_flow_log_p(fmt, ##args);  }}

/*-----------------------------------------------------------------------------*/
/* Local Constant Definitions                                                  */
/*-----------------------------------------------------------------------------*/
#define DEV_BASE		ISF_UNIT_VDOCAP
#define DEV_COUNT		ISF_MAX_VDOCAP
#define IN_BASE		ISF_IN_BASE
#define IN_COUNT		1
#define OUT_BASE		ISF_OUT_BASE
#define OUT_COUNT 	1

#define HD_DEV_BASE	HD_DAL_VIDEOCAP_BASE
#define HD_DEV_MAX	HD_DAL_VIDEOCAP_MAX

#define _HD_CONVERT_SELF_ID(dev_id, rv) \
	do { \
		(rv) = HD_ERR_DEV;	\
		if((dev_id) == 0) { \
			(rv) = HD_ERR_UNIQUE; \
		} else if((dev_id) >= HD_DEV_BASE && (dev_id) <= HD_DEV_MAX) { \
			UINT32 id = (dev_id) - HD_DEV_BASE; \
			if(id < DEV_COUNT) { \
				(dev_id) = DEV_BASE + id; \
				(rv) = HD_OK; \
			} \
		} \
	} while(0)

#define _HD_CONVERT_OUT_ID(out_id, rv) \
	do { \
		(rv) = HD_ERR_IO; \
		if((out_id) == 0) { \
			(rv) = HD_ERR_UNIQUE; \
		} else if((out_id) >= HD_OUT_BASE && (out_id) <= HD_OUT_MAX) { \
			UINT32 id = (out_id) - HD_OUT_BASE; \
			if(id < OUT_COUNT) { \
				(out_id) = OUT_BASE + id; \
				(rv) = HD_OK; \
			} \
		} \
	} while(0)

#define CTL_SEN_MODESEL_MAX_NUM 10
/*-----------------------------------------------------------------------------*/
/* Local Types Declarations                                                    */
/*-----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------*/
/* Extern Global Variables                                                     */
/*-----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------*/
/* Extern Function Prototype                                                   */
/*-----------------------------------------------------------------------------*/
extern int _hd_common_get_fd(void);

/*-----------------------------------------------------------------------------*/
/* Local Function Prototype                                                    */
/*-----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------*/
/* Debug Variables & Functions                                                 */
/*-----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------*/
/* Local Global Variables                                                      */
/*-----------------------------------------------------------------------------*/
static BOOL set_if_cfg(VDOCAP_SEN_INIT_IF_CFG *p_if_cfg, HD_VIDEOCAP_SENSOR_DEVICE *p_sen)
{
	BOOL ret = TRUE;

	switch (p_sen->if_type) {
	case HD_COMMON_VIDEO_IN_MIPI_CSI:
		p_if_cfg->type = VDOCAP_SEN_IF_TYPE_MIPI;
		break;
	case HD_COMMON_VIDEO_IN_LVDS:
		p_if_cfg->type = VDOCAP_SEN_IF_TYPE_LVDS;
		break;
	case HD_COMMON_VIDEO_IN_SLVS_EC:
		p_if_cfg->type = VDOCAP_SEN_IF_TYPE_SLVSEC;
		break;
	case HD_COMMON_VIDEO_IN_P_RAW:
	case HD_COMMON_VIDEO_IN_P_AHD:
		p_if_cfg->type = VDOCAP_SEN_IF_TYPE_PARALLEL;
		break;
	case HD_COMMON_VIDEO_IN_MIPI_VX1:
		p_if_cfg->type = VDOCAP_SEN_IF_TYPE_MIPI;
		break;
	case HD_COMMON_VIDEO_IN_P_RAW_VX1:
		p_if_cfg->type = VDOCAP_SEN_IF_TYPE_PARALLEL;
		break;

	default:
		DBG_ERR("not support if_type = %d\r\n", p_sen->if_type);
		ret = FALSE;
		break;
	}

	p_if_cfg->tge.tge_en = p_sen->if_cfg.tge.tge_en;
	p_if_cfg->tge.swap = p_sen->if_cfg.tge.swap;
	p_if_cfg->tge.sie_vd_src = p_sen->if_cfg.tge.vcap_vd_src;
	p_if_cfg->tge.sie_sync_set = p_sen->if_cfg.tge.vcap_sync_set;

	return ret;
}
static BOOL set_cmd_if_cfg(VDOCAP_SEN_INIT_CMDIF_CFG *p_cmd_if_cfg, HD_VIDEOCAP_SENSOR_DEVICE *p_sen)
{
	BOOL ret = TRUE;

	p_cmd_if_cfg->vx1.en = p_sen->if_cfg.vx1.en;
	p_cmd_if_cfg->vx1.if_sel = p_sen->if_cfg.vx1.if_sel;
	p_cmd_if_cfg->vx1.ctl_sel = p_sen->if_cfg.vx1.ctl_sel;
	p_cmd_if_cfg->vx1.tx_type = p_sen->if_cfg.vx1.tx_type;

	return ret;
}
static BOOL set_pin_cfg(VDOCAP_SEN_INIT_PIN_CFG *p_pin_cfg, HD_VIDEOCAP_SENSOR_DEVICE *p_sen)
{
	BOOL ret = TRUE;

	p_pin_cfg->pinmux.sensor_pinmux =  p_sen->pin_cfg.pinmux.sensor_pinmux;
	p_pin_cfg->pinmux.cmd_if_pinmux = p_sen->pin_cfg.pinmux.cmd_if_pinmux;
	p_pin_cfg->pinmux.serial_if_pinmux = p_sen->pin_cfg.pinmux.serial_if_pinmux;
	p_pin_cfg->clk_lane_sel = p_sen->pin_cfg.clk_lane_sel;
	if (VDOCAP_SEN_SER_MAX_DATALANE >= HD_VIDEOCAP_SEN_SER_MAX_DATALANE) {
		memcpy(p_pin_cfg->sen_2_serial_pin_map, p_sen->pin_cfg.sen_2_serial_pin_map, sizeof(p_pin_cfg->sen_2_serial_pin_map));
	} else {
		DBG_ERR("sen_2_serial_pin_map len not match %d,%d\r\n", VDOCAP_SEN_SER_MAX_DATALANE, HD_VIDEOCAP_SEN_SER_MAX_DATALANE);
		ret = FALSE;
	}
	p_pin_cfg->ccir_msblsb_switch = p_sen->pin_cfg.ccir_msblsb_switch;
	//p_pin_cfg->pad_sel = p_sen->pin_cfg.pad_sel;
	//p_pin_cfg->smp_edge = p_sen->pin_cfg.smp_edge;
	p_pin_cfg->ccir_vd_hd_pin = p_sen->pin_cfg.ccir_vd_hd_pin;
	p_pin_cfg->vx1_tx241_cko_pin = p_sen->pin_cfg.vx1_tx241_cko_pin;
	p_pin_cfg->vx1_tx241_cfg_2lane_mode = p_sen->pin_cfg.vx1_tx241_cfg_2lane_mode;
	return ret;
}
#if defined (__FREERTOS)
#include "kflow_videocapture/ctl_sen.h"
#include <libfdt.h>
#include <compiler.h>
#include <rtosfdt.h>

#define MAX_PINCTRL_ITEM (MAX_PINMUX_ITEM*2)

#include "plat/pll.h"
#include "../source/include/pll_protected.h"

extern void pll_enable_clock(CG_EN Num);
extern void pll_set_clock_rate(PLL_CLKSEL clk_sel, UINT32 ui_value);

typedef struct {
	BOOL					clk_en;
	CTL_SIE_MCLK_SRC_SEL	mclk_src_sel;
	UINT32					clk_rate;
	CTL_SIE_MCLK_ID    		mclk_id_sel;
} CTL_SIE_MCLK_INFO;

static void _vdocap_pixfmt_to_senfmt(UINT32 pxlfmt, CTL_SEN_DATA_FMT *p_data_fmt, CTL_SEN_PIXDEPTH *p_pixdepth)
{
	if (pxlfmt == VDO_PXLFMT_YUV422) {
		*p_data_fmt = CTL_SEN_DATA_FMT_YUV;
		*p_pixdepth = CTL_SEN_IGNORE;

	} else {
		switch (pxlfmt & 0xFFFF0000) {
		case VDO_PXLFMT_RAW8:
		case VDO_PXLFMT_RAW8_SHDR2:
		case VDO_PXLFMT_RAW8_SHDR3:
		case VDO_PXLFMT_RAW8_SHDR4:
			*p_pixdepth = CTL_SEN_PIXDEPTH_8BIT;
			break;
		case VDO_PXLFMT_RAW10:
		case VDO_PXLFMT_RAW10_SHDR2:
		case VDO_PXLFMT_RAW10_SHDR3:
		case VDO_PXLFMT_RAW10_SHDR4:
			*p_pixdepth = CTL_SEN_PIXDEPTH_10BIT;
			break;
		case VDO_PXLFMT_RAW12:
		case VDO_PXLFMT_RAW12_SHDR2:
		case VDO_PXLFMT_RAW12_SHDR3:
		case VDO_PXLFMT_RAW12_SHDR4:
		case VDO_PXLFMT_NRX12:
		case VDO_PXLFMT_NRX12_SHDR2:
		case VDO_PXLFMT_NRX12_SHDR3:
		case VDO_PXLFMT_NRX12_SHDR4:
			*p_pixdepth = CTL_SEN_PIXDEPTH_12BIT;
			break;
		case VDO_PXLFMT_RAW16:
		case VDO_PXLFMT_RAW16_SHDR2:
		case VDO_PXLFMT_RAW16_SHDR3:
		case VDO_PXLFMT_RAW16_SHDR4:
			*p_pixdepth = CTL_SEN_PIXDEPTH_16BIT;
			break;
		default:
			*p_pixdepth = CTL_SEN_PIXDEPTH_8BIT;
			DBG_ERR("-VdoIN:incorrect pxlfmt(0x%X)!\r\n", pxlfmt);
			break;
		}
		switch (pxlfmt & 0x0000FFFF) {
		case VDO_PIX_RCCB:
			*p_data_fmt = CTL_SEN_DATA_FMT_RCCB;
			break;
		case VDO_PIX_RGBIR:
			*p_data_fmt = CTL_SEN_DATA_FMT_RGBIR;
			break;
		case VDO_PIX_Y:
			*p_data_fmt = CTL_SEN_DATA_FMT_Y_ONLY;
			break;
		default: //default VDO_PIX_RGB
			*p_data_fmt = CTL_SEN_DATA_FMT_RGB;
			break;
		}
	}
}
// #define PIN_FUNC_MCLK_MASK (PIN_SENSORMISC_CFG_SN_MCLK_1|PIN_SENSORMISC_CFG_SN2_MCLK_1|PIN_SENSORMISC_CFG_SN3_MCLK_1|PIN_SENSORMISC_CFG_SN3_MCLK_2|PIN_SENSORMISC_CFG_SN4_MCLK_1|PIN_SENSORMISC_CFG_SN4_MCLK_2|PIN_SENSORMISC_CFG_SN5_MCLK_1|PIN_SENSORMISC_CFG_SN5_MCLK_2)
#define PIN_FUNC_MCLK_MASK (PIN_SENSORMISC_CFG_SN_MCLK_1|PIN_SENSORMISC_CFG_SN2_MCLK_1|PIN_SENSORMISC_CFG_SN3_MCLK_1|PIN_SENSORMISC_CFG_SN4_MCLK_1|PIN_SENSORMISC_CFG_SN4_MCLK_2)
static void _vdocap_dump_pinmux(CHAR *name, CTL_SEN_PINMUX *pinmux, UINT32 max_num)
{
#if __DBGLVL__ >= NVT_DBG_MSG
	UINT32 i;
	CHAR *str_pin_func[] = {
		"PIN_FUNC_SDIO",      ///< SDIO. Configuration refers to PIN_SDIO_CFG.
		"PIN_FUNC_NAND",      ///< NAND. Configuration refers to PIN_NAND_CFG.
		"PIN_FUNC_ETH",       ///< ETH. Configuration refers to PINMUX_ETH_CFG
		"PIN_FUNC_I2C",       ///< I2C. Configuration refers to PIN_I2C_CFG.
		"PIN_FUNC_I2CII",     ///< I2CII. Configuration refers to PIN_I2CII_CFG.
		"PIN_FUNC_PWM",       ///< PWM. Configuration refers to PIN_PWM_CFG.
		"PIN_FUNC_PWMII",     ///< PWMII. Configuration refers to PIN_PWMII_CFG.
		"PIN_FUNC_CCNT",      ///< CCNT. Configuration refers to PIN_CCNT_CFG.
		"PIN_FUNC_SENSOR",    ///< sensor interface. Configuration refers to PIN_SENSOR_CFG.
		"PIN_FUNC_SENSOR2",   ///< sensor2 interface. Configuration refers to PIN_SENSOR2_CFG.
		"PIN_FUNC_SENSOR3",   ///< sensor3 interface. Configuration refers to PIN_SENSOR3_CFG.
		"PIN_FUNC_SENSORMISC",///< sensor misc interface. Configuration refers to PIN_SENSORMISC_CFG.
		"PIN_FUNC_SENSORSYNC",///< sensor sync interface. Configuration refers to PIN_SENSORSYNC_CFG.
		"PIN_FUNC_MIPI_LVDS", ///< MIPI/LVDS interface configuration. Configuration refers to PIN_MIPI_LVDS_CFG.
		"PIN_FUNC_AUDIO",     ///< AUDIO. Configuration refers to PIN_AUDIO_CFG.
		"PIN_FUNC_UART",      ///< UART. Configuration refers to PIN_UART_CFG.
		"PIN_FUNC_UARTII",    ///< UARTII. Configuration refers to PIN_UARTII_CFG.
		"PIN_FUNC_REMOTE",    ///< REMOTE. Configuration refers to PIN_REMOTE_CFG.
		"PIN_FUNC_SDP",       ///< SDP. Configuration refers to PIN_SDP_CFG.
		"PIN_FUNC_SPI",       ///< SPI. Configuration refers to PIN_SPI_CFG.
		"PIN_FUNC_SIF",       ///< SIF. Configuration refers to PIN_SIF_CFG.
		"PIN_FUNC_MISC",      ///< MISC. Configuration refers to PINMUX_MISC_CFG
		"PIN_FUNC_LCD",       ///< Software records LCD interface. Configuration refers to PINMUX_LCDINIT, PINMUX_DISPMUX_SEL.
		"PIN_FUNC_LCD2",      ///< Software records LCD2 interface. Configuration refers to PINMUX_LCDINIT, PINMUX_DISPMUX_SEL.
		"PIN_FUNC_TV",        ///< Software recores TV interface. Configuration refers to PINMUX_TV_HDMI_CFG.
		"PIN_FUNC_SEL_LCD",   ///< LCD interface. Configuration refers to PINMUX_LCD_SEL.
		"PIN_FUNC_SEL_LCD2",  ///< LCD2 interface. Configuration refers to PINMUX_LCD_SEL.
	};

	DBG_DUMP("######## pinmux for %s ########\r\n", name);
	for (i = 0; i < max_num; i++) {
		if (pinmux) {
			if (pinmux->func < (sizeof(str_pin_func)/sizeof(CHAR *))) {
				DBG_DUMP("%-20s, CFG = 0x%X \r\n", str_pin_func[pinmux->func], pinmux->cfg);
			} else {
				DBG_DUMP("FUNC = %2d, CFG = 0x%X \r\n", pinmux->func, pinmux->cfg);
			}
			if (pinmux->pnext) {
				pinmux = pinmux->pnext;
			} else {
				break;
			}
		}
	}
	DBG_DUMP("#######################################\r\n");
#endif
}
static void _vdocap_dump_modesel(UINT32 id, CTL_SEN_MODESEL_PARAM *p_data)
{
#if __DBGLVL__ >= NVT_DBG_MSG
	DBG_MSG("######## VDOCAP[%d]modesel  ########\r\n", id);
	DBG_MSG("fps = %d\r\n", p_data->fps);
	DBG_MSG("size.w = %d\r\n", p_data->size.w);
	DBG_MSG("    .h = %d\r\n", p_data->size.h);
	DBG_MSG("frame_num = %d\r\n", p_data->frame_num);
	if (p_data->adv) {
		UINT32 i;

		for (i = 0; i < CTL_SEN_MODESEL_MAX_NUM; i ++) {
			DBG_MSG("adv[%d].frm_idx = %d\r\n", i, p_data->adv[i].frm_idx);
			DBG_MSG("adv[%d].item = %d\r\n", i, p_data->adv[i].item);
			DBG_MSG("adv[%d].value = 0x%llX\r\n", i, p_data->adv[i].value);
			if (p_data->adv[i].pnext == NULL) {
				break;
			}
		}
	}
	DBG_MSG("mode = %d\r\n", p_data->mode);
	DBG_MSG("#######################################\r\n");
#endif
}
static BOOL _vdocap_parse_pinmux(CHAR *name, CTL_SEN_PINMUX *pinmux, INT32 max_num)
{
	unsigned int pinctrl[MAX_PINCTRL_ITEM] = {0};
	unsigned char *fdt_addr = (unsigned char *)fdt_get_base();
	int nodeoffset, len;
	INT32 i, cnt;
	const void *nodep;
	unsigned int *p_data;
	char node_name[64];

	snprintf(node_name, sizeof(node_name), "/top/%s", name);
	nodeoffset = fdt_path_offset((const void*)fdt_addr, node_name);
	if (nodeoffset >= 0) {
		nodep = fdt_getprop(fdt_addr, nodeoffset, "pinctrl", &len);
		if (len == 0 || nodep == NULL) {
		    DBG_ERR("Failed to read pinctrl\r\n");
		    return FALSE;
		}
		cnt = len / sizeof(unsigned int);
		if (cnt > MAX_PINCTRL_ITEM) {
			cnt = MAX_PINCTRL_ITEM;
			DBG_ERR("Pinmux only support max %d item\r\n", cnt/2);
		}
		p_data = (unsigned int *)nodep;
		for (i = 0; i < cnt; i++) {
		    pinctrl[i] = be32_to_cpu(p_data[i]);
		    //DBG_DUMP("pinctrl[%d] = 0x%X\r\n", i, pinctrl[i]);

		}
		for (i = 0; i < cnt/2; i++) {
			if (i >= max_num) {
				break;
			}
		    pinmux[i].func = pinctrl[i*2];
		    pinmux[i].cfg = pinctrl[i*2+1];
		    if (pinmux[i].func == PIN_FUNC_SENSORMISC) {
				pinmux[i].cfg_mclk = pinmux[i].cfg & PIN_FUNC_MCLK_MASK;
		    }
		    if (i < (cnt/2 -1)) {
			    pinmux[i].pnext = &pinmux[i+1];
			}
		}
	} else {
		DBG_ERR("Failed to find %s in dtsi\r\n", name);
		return FALSE;
	}
	return TRUE;
}
static BOOL ctl_sen_inited(UINT32 sen_id)
{
	return (ctl_sen_status(sen_id) & CTL_SEN_STATUS_MAP_INIT) ? TRUE : FALSE;
}

static BOOL _vdocap_clock_lane_mapping(UINT32 value, CTL_SEN_CLANE_SEL *p_clk_lane_sel)
{
	BOOL ret = TRUE;

	//690 or later version doesn't need clock lane
	*p_clk_lane_sel = 0;
	return ret;
}
static CTL_SEN_DRVDEV _vdocap_serial_if_mapping(CTL_SEN_IF_TYPE if_cfg_type, UINT32 value)
{
	CTL_SEN_DRVDEV ret = 0;
	UINT32 id = (value >> 16)&0xF;

	if (if_cfg_type == CTL_SEN_IF_TYPE_LVDS) {
		switch (id) {
		case 0:
			ret = CTL_SEN_DRVDEV_LVDS_0;
			break;
		case 1:
			ret = CTL_SEN_DRVDEV_LVDS_1;
			break;
		case 2:
			ret = CTL_SEN_DRVDEV_LVDS_2;
			break;
		case 3:
			ret = CTL_SEN_DRVDEV_LVDS_3;
			break;
		case 4:
			ret = CTL_SEN_DRVDEV_LVDS_4;
			break;
		}
	} else if (if_cfg_type == CTL_SEN_IF_TYPE_MIPI) {
		switch (id) {
		case 0:
			ret = CTL_SEN_DRVDEV_CSI_0;
			break;
		case 1:
			ret = CTL_SEN_DRVDEV_CSI_1;
			break;
		case 2:
			ret = CTL_SEN_DRVDEV_CSI_2;
			break;
		case 3:
			ret = CTL_SEN_DRVDEV_CSI_3;
			break;
		case 4:
			ret = CTL_SEN_DRVDEV_CSI_4;
			break;
		}
	}
	DBG_IND("%d -> 0x%X\r\n", if_cfg_type, ret);
	return ret;
}
static INT32 ctl_sen_init_cfg(UINT32 id, HD_VIDEOCAP_SEN_CONFIG *p_sen_cfg)
{
	CTL_SEN_INIT_CFG_OBJ cfg_obj = {0};
	VDOCAP_SEN_INIT_CFG sen_init = {0};
	CTL_SEN_PINMUX pinmux[3] = {0};
	INT32 ret = HD_OK;
	CTL_SEN_INIT_MAP init_map = {0};
	CHAR pinmux_node_name[VDOCAP_SEN_NAME_LEN+2];
	CTL_SEN_IF_TYPE if_type;

	//------------- HDAL flow -----------------------------
	memcpy(sen_init.driver_name, p_sen_cfg->sen_dev.driver_name, VDOCAP_SEN_NAME_LEN);
	if (FALSE == set_if_cfg(&sen_init.sen_init_cfg.if_cfg, &p_sen_cfg->sen_dev))
		return HD_ERR_PARAM;
	if (FALSE == set_cmd_if_cfg(&sen_init.sen_init_cfg.cmd_if_cfg, &p_sen_cfg->sen_dev))
		return HD_ERR_PARAM;
	if (FALSE == set_pin_cfg(&sen_init.sen_init_cfg.pin_cfg, &p_sen_cfg->sen_dev))
		return HD_ERR_PARAM;
	memcpy(&sen_init.sen_init_option, &p_sen_cfg->sen_dev.option, sizeof(VDOCAP_SEN_INIT_OPTION));

	//-------------- isf_vdocap flow ----------------------------------------------------------
	//_dump_sen_init(&sen_init);
	snprintf(pinmux_node_name, sizeof(pinmux_node_name), "%s@%d", sen_init.driver_name, sen_init.sen_init_cfg.pin_cfg.pinmux.sensor_pinmux);
	if(FALSE == _vdocap_parse_pinmux(pinmux_node_name, pinmux, MAX_PINMUX_ITEM)) {
		return ISF_ERR_INVALID_VALUE;
	}
	_vdocap_dump_pinmux(pinmux_node_name, pinmux, MAX_PINMUX_ITEM);

	cfg_obj.pin_cfg.pinmux.func = pinmux[0].func;
	cfg_obj.pin_cfg.pinmux.cfg = pinmux[0].cfg;
	cfg_obj.pin_cfg.pinmux.pnext = pinmux[0].pnext;
	if_type = sen_init.sen_init_cfg.if_cfg.type;
	if (if_type == CTL_SEN_IF_TYPE_MIPI || if_type == CTL_SEN_IF_TYPE_LVDS) {
		if (FALSE == _vdocap_clock_lane_mapping(p_sen_cfg->sen_dev.pin_cfg.clk_lane_sel, &cfg_obj.pin_cfg.clk_lane_sel)) {
			DBG_ERR("clock lane error(0x%X)\r\n", p_sen_cfg->sen_dev.pin_cfg.clk_lane_sel);
			return ISF_ERR_INVALID_VALUE;
		}
	}
	DBG_MSG("ctl_sen[%d] clock_lane_mapping 0x%X -> %d\r\n", id, p_sen_cfg->sen_dev.pin_cfg.clk_lane_sel, cfg_obj.pin_cfg.clk_lane_sel);
	if (sizeof(cfg_obj.pin_cfg.sen_2_serial_pin_map) == sizeof(sen_init.sen_init_cfg.pin_cfg.sen_2_serial_pin_map)) {
		memcpy(cfg_obj.pin_cfg.sen_2_serial_pin_map, sen_init.sen_init_cfg.pin_cfg.sen_2_serial_pin_map, sizeof(cfg_obj.pin_cfg.sen_2_serial_pin_map));
	} else {
		DBG_ERR("VDOCAP_SEN_SER_MAX_DATALANE size not match!\r\n");
		return ISF_ERR_PROCESS_FAIL;
	}

	cfg_obj.pin_cfg.data_switch_mode = sen_init.sen_init_cfg.pin_cfg.ccir_msblsb_switch;
	//cfg_obj.pin_cfg.pad_sel = sen_init.sen_init_cfg.pin_cfg.pad_sel;
	//cfg_obj.pin_cfg.smp_edge = sen_init.sen_init_cfg.pin_cfg.smp_edge;

	cfg_obj.pin_cfg.ccir_vd_hd_pin = sen_init.sen_init_cfg.pin_cfg.ccir_vd_hd_pin;
	cfg_obj.pin_cfg.vx1_tx241_cko_pin = sen_init.sen_init_cfg.pin_cfg.vx1_tx241_cko_pin;
	cfg_obj.pin_cfg.vx1_tx241_cfg_2lane_mode = sen_init.sen_init_cfg.pin_cfg.vx1_tx241_cfg_2lane_mode;

	cfg_obj.if_cfg.type = if_type;
	cfg_obj.if_cfg.tge.tge_en = sen_init.sen_init_cfg.if_cfg.tge.tge_en;
	cfg_obj.if_cfg.tge.swap = sen_init.sen_init_cfg.if_cfg.tge.swap;
	cfg_obj.if_cfg.tge.sie_vd_src = sen_init.sen_init_cfg.if_cfg.tge.sie_vd_src;

	cfg_obj.cmd_if_cfg.vx1.en = sen_init.sen_init_cfg.cmd_if_cfg.vx1.en;
	cfg_obj.cmd_if_cfg.vx1.if_sel = sen_init.sen_init_cfg.cmd_if_cfg.vx1.if_sel;
//	cfg_obj.cmd_if_cfg.vx1.ctl_sel = sen_init.sen_init_cfg.cmd_if_cfg.vx1.ctl_sel;
	cfg_obj.cmd_if_cfg.vx1.tx_type = sen_init.sen_init_cfg.cmd_if_cfg.vx1.tx_type;
	cfg_obj.drvdev |= _vdocap_serial_if_mapping(cfg_obj.if_cfg.type, p_sen_cfg->sen_dev.pin_cfg.clk_lane_sel);

	init_map.chip_id = id;
	init_map.chip_cfg = &cfg_obj;
	init_map.chip_name = sen_init.driver_name;
	if (p_sen_cfg->shdr_map) {
		UINT32 map = p_sen_cfg->shdr_map & 0xFFFF;
		UINT32 i, temp;

		for (i = 0; i < VDOCAP_MAX_NUM; i++) {
			temp  = map >> i;
			if (temp & 0x1) {
				if (ctl_sen_inited(i)) {
					ctl_sen_uninit_map(i);
				}
				DBG_MSG("ctl_sen[%d] init_map chip_id=%d, chip_name=%s\r\n", i, init_map.chip_id, init_map.chip_name);
				ret = ctl_sen_init_map(i, &init_map);
			}
		}
	} else {
		DBG_MSG("ctl_sen[%d] init_map chip_id=%d, chip_name=%s\r\n", id, init_map.chip_id, init_map.chip_name);
		ret = ctl_sen_init_map(id, &init_map);
	}
	if(ret) {
		DBG_ERR("sen[%d] init_cfg failed(%d)!\r\n", id, ret);
		ret = ISF_ERR_INVALID_VALUE;
	}
	return ret;
}

static HD_RESULT _fast_open_sensor(UINT32 id, VOID *p_param)
{
	VENDOR_VIDEOCAP_FAST_OPEN_SENSOR *p_user = (VENDOR_VIDEOCAP_FAST_OPEN_SENSOR *)p_param;
	INT32 ret;
	UINT32 frame_rate;
	UINT32 ctl_sen_buf_size;
	#if defined (__FREERTOS)
	ISP_SENSOR_PRESET_CTRL preset_ctrl;
	#endif
	static BOOL ctl_sen_inited = FALSE;
	CTL_SEN_MODESEL_PARAM modesel = {0};
	CTL_SEN_CHGMODE_INFO sen_chgmode_info = {0};
	CTL_SEN_MAP map = {0};
	CTL_SEN_MODESEL_PARAM_ADV modesel_adv[CTL_SEN_MODESEL_MAX_NUM]= {0};
	UINT32 dest, temp, i, frm_idx = 0;

	ctl_sen_buf_size = ctl_sen_buf_query(2);
	if (ctl_sen_inited == FALSE) {
		//set addr 0 to let ctl_set use internal kmalloc
		ret = ctl_sen_init(0, ctl_sen_buf_size);
		if (ret) {
			DBG_ERR("ctl sen init fail!(%d)\r\n", ret);
			return HD_ERR_INV;
		}
		ctl_sen_inited = TRUE;
	}

	//DBG_DUMP("id(%d) sen_mode=%d\r\n", (UINT32)id, (UINT32)p_user->sen_mode);
	ctl_sen_init_cfg(id, &p_user->sen_cfg);
	if (p_user->sen_cfg.shdr_map) {
		dest = p_user->sen_cfg.shdr_map & 0xFFFF;
	} else {
		dest = 1 << id;
	}
	DBG_MSG("dest=0x%X\r\n", dest);
	for (i = 0; i < VDOCAP_MAX_NUM; i++) {
		temp  = dest >> i;
		if ((temp & 0x1) == 0) {
			continue;
		}
		map.frm_idx = frm_idx;
		map.sen_mode = CTL_SEN_IGNORE;
		DBG_MSG("[%d]add map id%d=0x%X, sen_mode=0x%X\r\n", i, map.frm_idx, map.sen_mode, p_user->sen_mode);
		ret=  ctl_sen_add_map(i, &map);
		if (ret) {
			DBG_ERR("ctl_sen_add_map failed(%d)\r\n", ret);
			return HD_ERR_INV;
		}

		//sensor open
		ret = ctl_sen_open(i);
		if (ret != CTL_SEN_E_OK) {
			switch(ret) {
			case CTL_SEN_E_MAP_TBL:
				DBG_ERR("no sen driver\r\n");
				ret = HD_ERR_NOT_AVAIL;
				break;
			case CTL_SEN_E_SENDRV:
				DBG_ERR("sensor driver error\r\n");
				ret = HD_ERR_FAIL;//map to HD_ERR_FAIL
				break;
			default:
				DBG_ERR("sensor open fail(%d)\r\n", ret);
				ret = HD_ERR_INV;
				break;
			}
			return ret;
		}
		ctl_sen_pwr_ctrl(i, CTL_SEN_PWR_CTRL_TURN_ON);
		frm_idx++;
	}

	frm_idx = 0;
	for (i = 0; i < VDOCAP_MAX_NUM; i++) {
		temp  = dest >> i;
		if ((temp & 0x1) == 0) {
			continue;
		}

		#if defined (__FREERTOS)
		// NOTE: ISP_SENSOR_PRESET_AE: enable preset; ISP_SENSOR_PRESET_DEFAULT: disable preset
		if (p_user->ae_preset.enable) {
			preset_ctrl.mode = ISP_SENSOR_PRESET_AE;
			//preset_ctrl.mode = ISP_SENSOR_PRESET_DEFAULT;
			preset_ctrl.exp_time[0] = p_user->ae_preset.exp_time;
			preset_ctrl.gain_ratio[0] = p_user->ae_preset.gain_ratio;
			// for HDR
			preset_ctrl.exp_time[1] = (p_user->ae_preset.exp_time>>4);
			preset_ctrl.gain_ratio[1] = p_user->ae_preset.gain_ratio;
			ctl_sen_set(i, CTL_SEN_CFG_USER1, &preset_ctrl);
			DBG_DUMP("\r\n[video_capture] preset exp_time = %d, gain_ratio = %d\r\n\r\n", preset_ctrl.exp_time[0], preset_ctrl.gain_ratio[0]);
		}
		#endif

		frame_rate = 100*GET_HI_UINT16(p_user->frc)/GET_LO_UINT16(p_user->frc);
		if (VDOCAP_SEN_MODE_AUTO == p_user->sen_mode) {
			CTL_SEN_DATA_FMT sen_data_fmt;
			CTL_SEN_PIXDEPTH sen_pixdepth;
			UINT32 idx = 0;

			modesel.size.w = p_user->dim.w;
			modesel.size.h = p_user->dim.h;
			modesel.fps = frame_rate;
			modesel.frame_num = (UINT32)p_user->out_frame_num;
			_vdocap_pixfmt_to_senfmt(p_user->pxlfmt, &sen_data_fmt, &sen_pixdepth);
			modesel.adv = &modesel_adv[0];

			modesel_adv[idx].frm_idx = frm_idx;
			modesel_adv[idx].item = modesel_item_datafmt;
			modesel_adv[idx].value = sen_data_fmt;
			idx++;
			modesel_adv[idx-1].pnext = &modesel_adv[idx];
			modesel_adv[idx].frm_idx = frm_idx;
			modesel_adv[idx].item = modesel_item_pixdepth;
			modesel_adv[idx].value = sen_pixdepth;

			if (p_user->data_lane) {
				idx++;
				modesel_adv[idx-1].pnext = &modesel_adv[idx];
				modesel_adv[idx].frm_idx = frm_idx;
				modesel_adv[idx].item = modesel_item_datalane;
				modesel_adv[idx].value = p_user->data_lane;
			}

			idx++;
			modesel_adv[idx-1].pnext = &modesel_adv[idx];
			modesel_adv[idx].frm_idx = frm_idx;
			modesel_adv[idx].item = modesel_item_modetype;
			if (p_user->builtin_hdr) {
				modesel_adv[idx].value = CTL_SEN_MODE_BUILTIN_HDR;
			} else if (p_user->sen_cfg.shdr_map) {
				modesel_adv[idx].value = CTL_SEN_MODE_STAGGER_HDR;
			} else {
				modesel_adv[idx].value = CTL_SEN_MODE_LINEAR;
			}
			ret = ctl_sen_get(i, CTL_SEN_CFG_MODESEL, (void*)&modesel);
			_vdocap_dump_modesel(i, &modesel);
			if (ret) {
				DBG_ERR("sen modesel failed(%d)\r\n", ret);
				return HD_ERR_INV;
			}
		} else {
			modesel.mode = p_user->sen_mode;
		}

		map.frm_idx = frm_idx;
		map.sen_mode = modesel.mode;
		DBG_MSG("ctl_sen[%d] del_map\r\n", i);
		ctl_sen_del_map(i);
		DBG_MSG("ctl_sen[%d] add_map sen_mode = %d, frm_idx = %d\r\n", i, map.sen_mode, map.frm_idx);
		ret=  ctl_sen_add_map(i, &map);
		if (ret) {
			DBG_ERR("ctl_sen_add_map failed(%d)\r\n", ret);
			return HD_ERR_INV;
		}
		sen_chgmode_info.fps = frame_rate;
		sen_chgmode_info.output_dest = i;

		ret=  ctl_sen_chgmode(i, &sen_chgmode_info);

		if (ret) {
			DBG_ERR("ctl_sen_chgmode failed(%d)\r\n", ret);
			ret=  HD_ERR_INV;
		}
		frm_idx++;
	}//for (i = 0; i < VDOCAP_MAX_NUM; i++) {
	return ret;
}
#if _FASTBOOT_2A_TO_DO_
static HD_RESULT _fast_change_mode(UINT32 id, VOID *p_param)
{
	VENDOR_VIDEOCAP_FAST_OPEN_SENSOR *p_user = (VENDOR_VIDEOCAP_FAST_OPEN_SENSOR *)p_param;
	INT32 ret;
	UINT32 frame_rate;
	#if defined (__FREERTOS)
	ISP_SENSOR_PRESET_CTRL preset_ctrl;
	#endif
	CTL_SEN_MODESEL_PARAM modesel = {0};
	CTL_SEN_CHGMODE_INFO sen_chgmode_info = {0};
	CTL_SEN_MAP map = {0};
	CTL_SEN_MODESEL_PARAM_ADV modesel_adv[CTL_SEN_MODESEL_MAX_NUM]= {0};
	UINT32 dest, temp, i, frm_idx = 0;


	//DBG_DUMP("id(%d) sen_mode=%d\r\n", (UINT32)id, (UINT32)p_user->sen_mode);

	if (p_user->out_frame_num > 1) {
		sie_builtin_set_stop(id);
	}
	if (p_user->sen_cfg.shdr_map) {
		dest = p_user->sen_cfg.shdr_map & 0xFFFF;
	} else {
		dest = 1 << id;
	}
	DBG_MSG("_fast_change_mode dest=0x%X\r\n", dest);
	for (i = 0; i < VDOCAP_MAX_NUM; i++) {
		temp  = dest >> i;
		if ((temp & 0x1) == 0) {
			continue;
		}
		ctl_sen_pwr_ctrl(i, CTL_SEN_PWR_CTRL_TURN_OFF);
		ctl_sen_del_map(i);
		ctl_sen_close(i);

		map.frm_idx = frm_idx;
		map.sen_mode = CTL_SEN_IGNORE;
		DBG_MSG("[%d]add map id%d=0x%X, sen_mode=0x%X\r\n", i, map.frm_idx, map.sen_mode, p_user->sen_mode);
		ret=  ctl_sen_add_map(i, &map);
		if (ret) {
			DBG_ERR("ctl_sen_add_map failed(%d)\r\n", ret);
			return HD_ERR_INV;
		}
		//sensor open
		ret = ctl_sen_open(i);
		if (ret != CTL_SEN_E_OK) {
			switch(ret) {
			case CTL_SEN_E_MAP_TBL:
				DBG_ERR("no sen driver\r\n");
				ret = HD_ERR_NOT_AVAIL;
				break;
			case CTL_SEN_E_SENDRV:
				DBG_ERR("sensor driver error\r\n");
				ret = HD_ERR_FAIL;//map to HD_ERR_FAIL
				break;
			default:
				DBG_ERR("sensor open fail(%d)\r\n", ret);
				ret = HD_ERR_INV;
				break;
			}
			return ret;
		}
		ctl_sen_pwr_ctrl(i, CTL_SEN_PWR_CTRL_TURN_ON);
		frm_idx++;
	}

	frm_idx = 0;
	for (i = 0; i < VDOCAP_MAX_NUM; i++) {
		temp  = dest >> i;
		if ((temp & 0x1) == 0) {
			continue;
		}

		#if defined (__FREERTOS)
		// NOTE: ISP_SENSOR_PRESET_AE: enable preset; ISP_SENSOR_PRESET_DEFAULT: disable preset
		if (p_user->ae_preset.enable) {
			preset_ctrl.mode = ISP_SENSOR_PRESET_AE;
			//preset_ctrl.mode = ISP_SENSOR_PRESET_DEFAULT;
			preset_ctrl.exp_time[0] = p_user->ae_preset.exp_time;
			preset_ctrl.gain_ratio[0] = p_user->ae_preset.gain_ratio;
			// for HDR
			preset_ctrl.exp_time[1] = (p_user->ae_preset.exp_time>>4);
			preset_ctrl.gain_ratio[1] = p_user->ae_preset.gain_ratio;
			ctl_sen_set(i, CTL_SEN_CFG_USER1, &preset_ctrl);
			DBG_DUMP("\r\n[video_capture] preset exp_time = %d, gain_ratio = %d\r\n\r\n", preset_ctrl.exp_time[0], preset_ctrl.gain_ratio[0]);
		}
		#endif

		frame_rate = 100*GET_HI_UINT16(p_user->frc)/GET_LO_UINT16(p_user->frc);
		if (VDOCAP_SEN_MODE_AUTO == p_user->sen_mode) {
			CTL_SEN_DATA_FMT sen_data_fmt;
			CTL_SEN_PIXDEPTH sen_pixdepth;
			UINT32 idx = 0;

			modesel.size.w = p_user->dim.w;
			modesel.size.h = p_user->dim.h;
			modesel.fps = frame_rate;
			modesel.frame_num = (UINT32)p_user->out_frame_num;
			_vdocap_pixfmt_to_senfmt(p_user->pxlfmt, &sen_data_fmt, &sen_pixdepth);
			modesel.adv = &modesel_adv[0];

			modesel_adv[idx].frm_idx = frm_idx;
			modesel_adv[idx].item = modesel_item_datafmt;
			modesel_adv[idx].value = sen_data_fmt;
			idx++;
			modesel_adv[idx-1].pnext = &modesel_adv[idx];
			modesel_adv[idx].frm_idx = frm_idx;
			modesel_adv[idx].item = modesel_item_pixdepth;
			modesel_adv[idx].value = sen_pixdepth;

			if (p_user->data_lane) {
				idx++;
				modesel_adv[idx-1].pnext = &modesel_adv[idx];
				modesel_adv[idx].frm_idx = frm_idx;
				modesel_adv[idx].item = modesel_item_datalane;
				modesel_adv[idx].value = p_user->data_lane;
			}

			idx++;
			modesel_adv[idx-1].pnext = &modesel_adv[idx];
			modesel_adv[idx].frm_idx = frm_idx;
			modesel_adv[idx].item = modesel_item_modetype;
			if (p_user->builtin_hdr) {
				modesel_adv[idx].value = CTL_SEN_MODE_BUILTIN_HDR;
			} else if (p_user->sen_cfg.shdr_map) {
				modesel_adv[idx].value = CTL_SEN_MODE_STAGGER_HDR;
			} else {
				modesel_adv[idx].value = CTL_SEN_MODE_LINEAR;
			}
			ret = ctl_sen_get(i, CTL_SEN_CFG_MODESEL, (void*)&modesel);
			_vdocap_dump_modesel(i, &modesel);
			if (ret) {
				DBG_ERR("sen modesel failed(%d)\r\n", ret);
				return HD_ERR_INV;
			}
		} else {
			modesel.mode = p_user->sen_mode;
		}

		map.frm_idx = frm_idx;
		map.sen_mode = modesel.mode;
		DBG_MSG("ctl_sen[%d] del_map\r\n", i);
		ctl_sen_del_map(i);
		DBG_MSG("ctl_sen[%d] add_map sen_mode = %d, frm_idx = %d\r\n", i, map.sen_mode, map.frm_idx);
		ret=  ctl_sen_add_map(i, &map);
		if (ret) {
			DBG_ERR("ctl_sen_add_map failed(%d)\r\n", ret);
			return HD_ERR_INV;
		}
		sen_chgmode_info.fps = frame_rate;
		sen_chgmode_info.output_dest = i;

		ret=  ctl_sen_chgmode(i, &sen_chgmode_info);

		if (ret) {
			DBG_ERR("ctl_sen_chgmode failed(%d)\r\n", ret);
			ret=  HD_ERR_INV;
		}
		{
			SIE_BUILTIN_IOSIZE iosize = {0};
			CTL_SEN_GET_MODE_BASIC_PARAM mode_param = {0};

			mode_param.mode = CTL_SEN_MODE_CUR;
			if (ctl_sen_get(i, CTL_SEN_CFG_MODE_BASIC, (void *)(&mode_param))) {
				DBG_ERR("get sen[%d] mode failed!\r\n", i);
				return HD_ERR_INV;
			}
			iosize.act_win = mode_param.act_size[0];
			iosize.crp_win.w = mode_param.crp_size.w;
			iosize.crp_win.h = mode_param.crp_size.h;
			sie_fb_set_iosize(id, iosize);
		}
		frm_idx++;
	}//for (i = 0; i < VDOCAP_MAX_NUM; i++) {

	if (p_user->out_frame_num > 1) {
		sie_builtin_set_start(id);
	}

	return ret;
}
static HD_RESULT _fast_open_sie(UINT32 id_map)
{
	SIE_BUILTIN_INIT_INFO sie_init_info = {0};
	NVTMPP_FASTBOOT_MEM_S *p_fastboot_mem;
	SIE_BUILTIN_IOSIZE iosize = {0};
	CTL_SEN_GET_MODE_BASIC_PARAM mode_param = {0};
	UINT32 i;

	for (i = 0; i < VDOCAP_MAX_NUM; i++) {
		if ((id_map >> i) & 0x1) {
			mode_param.mode = CTL_SEN_MODE_CUR;
			if (ctl_sen_get(i, CTL_SEN_CFG_MODE_BASIC, (void *)(&mode_param))) {
				DBG_ERR("get sen[%d] mode failed!\r\n", i);
				return HD_ERR_NG;
			}
			iosize.act_win = mode_param.act_size[0];
			iosize.crp_win.w = mode_param.crp_size.w;
			iosize.crp_win.h = mode_param.crp_size.h;
			sie_fb_set_iosize(i, iosize);
		}
	}

	// get fastboot memory from dts
	if (nvtmpp_parse_fastboot_mem_dt() < 0) {
		return HD_ERR_NG;
	}
	p_fastboot_mem = nvtmpp_get_fastboot_mem();

	if (NULL == p_fastboot_mem) {
		DBG_ERR("nvtmpp_get_fastboot_mem failed!\r\n");
		return HD_ERR_NOBUF;
	}

	// init sie
	sie_init_info.ring_buf_blk_addr = p_fastboot_mem->pv_pools[FBOOT_POOL_VCAP_SHDR_BUF].addr;
	sie_init_info.ring_buf_blk_size = p_fastboot_mem->pv_pools[FBOOT_POOL_VCAP_SHDR_BUF].size;

	//DBG_DUMP("ring_buf_blk_addr/size=0x%X, 0x%X\r\n", sie_init_info.ring_buf_blk_addr, sie_init_info.ring_buf_blk_size);
	// to check id usage
	sie_init_info.sie_id_bit = id_map;
	sie_init_info.rtos_streaming_id_bit = id_map;
	sie_builtin_init(&sie_init_info);

	return HD_OK;
}
#endif
#endif
static INT vendor_videocap_param_cvt_name(VENDOR_VIDEOCAP_PARAM_ID  id, CHAR *p_ret_string, INT max_str_len)
{
	switch (id) {
		case VENDOR_VIDEOCAP_PARAM_MCLK_SRC_SYNC_SET: snprintf(p_ret_string, max_str_len, "MCLK_SRC_SYNC_SET");  break;
		case VENDOR_VIDEOCAP_PARAM_BUILTIN_HDR: snprintf(p_ret_string, max_str_len, "BUILTIN_HDR");  break;
		case VENDOR_VIDEOCAP_PARAM_DATA_LANE:     snprintf(p_ret_string, max_str_len, "DATA_LANE");   break;
		case VENDOR_VIDEOCAP_PARAM_CCIR_INFO:  snprintf(p_ret_string, max_str_len, "CCIR_INFO");   break;
		case VENDOR_VIDEOCAP_PARAM_GET_PLUG:  snprintf(p_ret_string, max_str_len, "GET_PLUG");   break;
		case VENDOR_VIDEOCAP_PARAM_GET_PLUG_INFO:  snprintf(p_ret_string, max_str_len, "GET_PLUG_INFO");   break;
		case VENDOR_VIDEOCAP_PARAM_SET_FPS:          snprintf(p_ret_string, max_str_len, "SET_FPS");        break;
		case VENDOR_VIDEOCAP_PARAM_AD_MAP:         snprintf(p_ret_string, max_str_len, "AD_MAP");       break;
		case VENDOR_VIDEOCAP_PARAM_AD_TYPE:        snprintf(p_ret_string, max_str_len, "AD_TYPE");      break;
		case VENDOR_VIDEOCAP_PARAM_PDAF_MAP:     snprintf(p_ret_string, max_str_len, "PDAF_MAP");      break;
		case VENDOR_VIDEOCAP_PARAM_DVS_INFO: snprintf(p_ret_string, max_str_len, "DVS_INFO");      break;
		case VENDOR_VIDEOCAP_PARAM_ABORT: snprintf(p_ret_string, max_str_len, "ABORT");      break;
		case VENDOR_VIDEOCAP_PARAM_SIE_MAP: snprintf(p_ret_string, max_str_len, "SIE_MAP");      break;
		case VENDOR_VIDEOCAP_PARAM_ENC_RATE: snprintf(p_ret_string, max_str_len, "ENC_RATE");      break;
		case VENDOR_VIDEOCAP_PARAM_AE_PRESET: snprintf(p_ret_string, max_str_len, "AE_PRESET");      break;
		case VENDOR_VIDEOCAP_PARAM_BP3_RATIO: snprintf(p_ret_string, max_str_len, "BP3_RATIO");      break;
		case VENDOR_VIDEOCAP_PARAM_QUEUE_FLUSH_SCHEME: snprintf(p_ret_string, max_str_len, "QUEUE_FLUSH_SCHEME");      break;
		case VENDOR_VIDEOCAP_PARAM_CSI_ERR_CNT: snprintf(p_ret_string, max_str_len, "CSI_ERR_CNT");      break;
		case VENDOR_VIDEOCAP_PARAM_SW_VD_SYNC: snprintf(p_ret_string, max_str_len, "SW_VD_SYNC");      break;
		case VENDOR_VIDEOCAP_PARAM_PROBE_SENSOR: snprintf(p_ret_string, max_str_len, "PROBE_SENSOR");      break;
		case VENDOR_VIDEOCAP_PARAM_MODE_TYPE: snprintf(p_ret_string, max_str_len, "MODE_TYPE");      break;
		case VENDOR_VIDEOCAP_PARAM_PDAF_TYPE: snprintf(p_ret_string, max_str_len, "PDAF_TYPE");      break;
		case VENDOR_VIDEOCAP_PARAM_RESET_FC: snprintf(p_ret_string, max_str_len, "RESET_FC");      break;
		case VENDOR_VIDEOCAP_PARAM_GYRO_INFO: snprintf(p_ret_string, max_str_len, "GYRO_INFO");      break;
		case VENDOR_VIDEOCAP_PARAM_USER_PINMUX: snprintf(p_ret_string, max_str_len, "USER_PINMUX");      break;
		case VENDOR_VIDEOCAP_PARAM_DEFAULT_CCIR_FMT: snprintf(p_ret_string, max_str_len, "DEFAULT_CCIR_FMT");      break;
		case VENDOR_VIDEOCAP_PARAM_SSENIF_FORCE_DIS: snprintf(p_ret_string, max_str_len, "SSENIF_FORCE_DIS");      break;
		case VENDOR_VIDEOCAP_PARAM_KEEP_PINMUX_SETTING: snprintf(p_ret_string, max_str_len, "KEEP_PINMUX_SETTING");      break;
		case VENDOR_VIDEOCAP_PARAM_PDAF_PXLFMT: snprintf(p_ret_string, max_str_len, "PDAF_PXLFMT");      break;
		case VENDOR_VIDEOCAP_PARAM_EVS_INFO: snprintf(p_ret_string, max_str_len, "EVS_INFO");      break;
		case VENDOR_VIDEOCAP_PARAM_ADVANCE_PUSH: snprintf(p_ret_string, max_str_len, "ADVANCE_PUSH");      break;
		case VENDOR_VIDEOCAP_PARAM_TGE_CTRL: snprintf(p_ret_string, max_str_len, "TGE_CTRL");      break;
		case VENDOR_VIDEOCAP_PARAM_TSEN_CFG: snprintf(p_ret_string, max_str_len, "TSEN_CFG");      break;
		case VENDOR_VIDEOCAP_PARAM_TSEN_OOC: snprintf(p_ret_string, max_str_len, "TSEN_OOC");      break;
		default:
			snprintf(p_ret_string, max_str_len, "error");
			DBG_WRN("unknown param_id(%d)\r\n", id);
			return (-1);
	}
	return 0;
}
/*-----------------------------------------------------------------------------*/
/* Interface Functions                                                         */
/*-----------------------------------------------------------------------------*/
HD_RESULT vendor_videocap_set(UINT32 id, VENDOR_VIDEOCAP_PARAM_ID param_id, VOID *p_param)
{
	#if !defined (__FREERTOS_LITE)
	HD_DAL self_id = HD_GET_DEV(id);
	//HD_IO out_id = HD_GET_OUT(id);
	HD_IO ctrl_id = HD_GET_CTRL(id);
	ISF_FLOW_IOCTL_PARAM_ITEM cmd = {0};
	int isf_fd;
	int r;
	#endif
	HD_RESULT rv = HD_ERR_NG;

	if (p_param == NULL) {
		return HD_ERR_NULL_PTR;
	}

	#if defined (__FREERTOS)
	if (param_id == VENDOR_VIDEOCAP_PARAM_FAST_OPEN_SENSOR) {
		return _fast_open_sensor(id, p_param);
	}
	#endif

	#if !defined (__FREERTOS_LITE)
	{
		CHAR  param_name[20];
		vendor_videocap_param_cvt_name(param_id, param_name, 20);

		HD_VCAP_FLOW_MSG("vendor_videocap_set(%s):\n", param_name);
		HD_VCAP_FLOW_IND("    id(0x%x) param(0x%X)\n", id, *(UINT32 *)p_param);
	}
	isf_fd = _hd_common_get_fd();
	if (isf_fd <= 0) {
		return HD_ERR_UNINIT;
	}
	_HD_CONVERT_SELF_ID(self_id, rv); 	if(rv != HD_OK) {	return rv;}
	rv = HD_OK;

	if(ctrl_id == HD_CTRL) {
		switch(param_id) {
		case VENDOR_VIDEOCAP_PARAM_AD_MAP: {
			UINT32 *p_user = (UINT32 *)p_param;
			cmd.dest = ISF_PORT(self_id, ISF_CTRL);
			cmd.param = VDOCAP_PARAM_AD_MAP;
			cmd.value = *p_user;
			cmd.size = 0;
			r = ISF_IOCTL(isf_fd, ISF_FLOW_CMD_SET_PARAM, &cmd);
			goto _VD_VC1;
		}
		case VENDOR_VIDEOCAP_PARAM_AD_TYPE: {
			UINT32 *p_user = (UINT32 *)p_param;
			cmd.dest = ISF_PORT(self_id, ISF_CTRL);
			cmd.param = VDOCAP_PARAM_AD_TYPE;
			cmd.value = *p_user;
			cmd.size = 0;
			r = ISF_IOCTL(isf_fd, ISF_FLOW_CMD_SET_PARAM, &cmd);
			goto _VD_VC1;
		}
		case VENDOR_VIDEOCAP_PARAM_MCLK_SRC_SYNC_SET: {
			UINT32 *p_user = (UINT32 *)p_param;
			cmd.dest = ISF_PORT(self_id, ISF_CTRL);
			cmd.param = VDOCAP_PARAM_MCLK_SRC_SYNC_SET;
			cmd.value = *p_user;
			cmd.size = 0;
			r = ISF_IOCTL(isf_fd, ISF_FLOW_CMD_SET_PARAM, &cmd);
			goto _VD_VC1;
		}
		case VENDOR_VIDEOCAP_PARAM_PDAF_MAP: {
			UINT32 *p_user = (UINT32 *)p_param;
			cmd.dest = ISF_PORT(self_id, ISF_CTRL);
			cmd.param = VDOCAP_PARAM_PDAF_MAP;
			cmd.value = *p_user;
			cmd.size = 0;
			r = ISF_IOCTL(isf_fd, ISF_FLOW_CMD_SET_PARAM, &cmd);
			goto _VD_VC1;
		}
		case VENDOR_VIDEOCAP_PARAM_SIE_MAP: {
			UINT32 *p_user = (UINT32 *)p_param;
			cmd.dest = ISF_PORT(self_id, ISF_CTRL);
			cmd.param = VDOCAP_PARAM_SIE_MAP;
			cmd.value = *p_user;
			cmd.size = 0;
			r = ISF_IOCTL(isf_fd, ISF_FLOW_CMD_SET_PARAM, &cmd);
			goto _VD_VC1;
		}
		case VENDOR_VIDEOCAP_PARAM_SET_FPS: {
			UINT32 *p_user = (UINT32 *)p_param;
			cmd.dest = ISF_PORT(self_id, ISF_CTRL);
			cmd.param = VDOCAP_PARAM_SET_FPS;
			cmd.value = *p_user;
			cmd.size = 0;
			r = ISF_IOCTL(isf_fd, ISF_FLOW_CMD_SET_PARAM, &cmd);
			goto _VD_VC1;
		}
		case VENDOR_VIDEOCAP_PARAM_ABORT: {
			//UINT32 *p_user = (UINT32 *)p_param;
			cmd.dest = ISF_PORT(self_id, ISF_CTRL);
			cmd.param = VDOCAP_PARAM_DMA_ABORT;
			cmd.value = 0;
			cmd.size = 0;
			r = ISF_IOCTL(isf_fd, ISF_FLOW_CMD_SET_PARAM, &cmd);
			goto _VD_VC1;
		}
		case VENDOR_VIDEOCAP_PARAM_SW_VD_SYNC: {
			UINT32 *p_user = (UINT32 *)p_param;
			cmd.dest = ISF_PORT(self_id, ISF_CTRL);
			cmd.param = VDOCAP_PARAM_SW_VD_SYNC;
			cmd.value = *p_user;
			cmd.size = 0;
			r = ISF_IOCTL(isf_fd, ISF_FLOW_CMD_SET_PARAM, &cmd);
			goto _VD_VC1;
		}
		case VENDOR_VIDEOCAP_PARAM_PROBE_SENSOR: {
			HD_VIDEOCAP_SENSOR_DEVICE* p_user = (HD_VIDEOCAP_SENSOR_DEVICE *)p_param;
			VDOCAP_SEN_INIT_CFG sen_init = {0};

			HD_VCAP_FLOW_MSG("    driver_name(%s) if_type(%d)\n", p_user->driver_name, p_user->if_type);
			HD_VCAP_FLOW_MSG("    sensor_pinmux(0x%x) clk_lane_sel(0x%X)\n",
																	p_user->pin_cfg.pinmux.sensor_pinmux,
																	p_user->pin_cfg.clk_lane_sel);
			HD_VCAP_FLOW_MSG("    sen_2_serial_pin_map[%d %d %d %d %d %d %d %d]\n",
															p_user->pin_cfg.sen_2_serial_pin_map[0],
															p_user->pin_cfg.sen_2_serial_pin_map[1],
															p_user->pin_cfg.sen_2_serial_pin_map[2],
															p_user->pin_cfg.sen_2_serial_pin_map[3],
															p_user->pin_cfg.sen_2_serial_pin_map[4],
															p_user->pin_cfg.sen_2_serial_pin_map[5],
															p_user->pin_cfg.sen_2_serial_pin_map[6],
															p_user->pin_cfg.sen_2_serial_pin_map[7]);
			memcpy(sen_init.driver_name, p_user->driver_name, VDOCAP_SEN_NAME_LEN);
			if (FALSE == set_if_cfg(&sen_init.sen_init_cfg.if_cfg, p_user))
				return HD_ERR_INV;
			if (FALSE == set_cmd_if_cfg(&sen_init.sen_init_cfg.cmd_if_cfg, p_user))
				return HD_ERR_INV;
			if (FALSE == set_pin_cfg(&sen_init.sen_init_cfg.pin_cfg, p_user))
				return HD_ERR_INV;
			memcpy(&sen_init.sen_init_option, &p_user->option, sizeof(VDOCAP_SEN_INIT_OPTION));

			cmd.dest = ISF_PORT(self_id, ISF_CTRL);
			cmd.param = VDOCAP_PARAM_PROBE_SENSOR;
			cmd.value = (ULONG)&sen_init;
			cmd.size = sizeof(VDOCAP_SEN_INIT_CFG);
			r = ISF_IOCTL(isf_fd, ISF_FLOW_CMD_SET_PARAM, &cmd);
			goto _VD_VC1;
		}
		case VENDOR_VIDEOCAP_PARAM_USER_PINMUX: {
			HD_VIDEOCAP_SEN_CONFIG_EXT *p_user = (HD_VIDEOCAP_SEN_CONFIG_EXT *)p_param;
			VDOCAP_USER_PINMUX user_pinmux = {0};
			UINT32 i;

			if (p_user->cfg_num > MAX_PINMUX_ITEM) {
				r = rv = HD_ERR_INV;
				DBG_ERR("Only support max %d items.\r\n", MAX_PINMUX_ITEM);
				return HD_ERR_INV;
			}

			if (NULL == p_user->p_sen_cfg_ext) {
				r = rv = HD_ERR_INV;
				DBG_ERR("p_sen_cfg_ext null!\r\n");
				return HD_ERR_INV;
			}

			user_pinmux.cfg_num = p_user->cfg_num;
			for (i = 0; i < p_user->cfg_num; i++) {
				user_pinmux.func[i] = p_user->p_sen_cfg_ext[i].func;
				user_pinmux.cfg[i] = p_user->p_sen_cfg_ext[i].value;
				HD_VCAP_FLOW_MSG("    [%d]func=0x%X, value=0x%X\n", user_pinmux.func[i], user_pinmux.cfg[i]);
			}

			cmd.dest = ISF_PORT(self_id, ISF_CTRL);
			cmd.param = VDOCAP_PARAM_USER_PINMUX;
			cmd.value = (ULONG)&user_pinmux;
			cmd.size = sizeof(VDOCAP_USER_PINMUX);
			r = ISF_IOCTL(isf_fd, ISF_FLOW_CMD_SET_PARAM, &cmd);
			goto _VD_VC1;
		}
		case VENDOR_VIDEOCAP_PARAM_DEFAULT_CCIR_FMT: {
			UINT32 *p_user = (UINT32 *)p_param;
			cmd.dest = ISF_PORT(self_id, ISF_CTRL);
			cmd.param = VDOCAP_PARAM_DEFAULT_CCIR_FMT;
			cmd.value = *p_user;
			cmd.size = 0;
			r = ISF_IOCTL(isf_fd, ISF_FLOW_CMD_SET_PARAM, &cmd);
			goto _VD_VC1;
		}
		default: rv = HD_ERR_PARAM; break;
		}
	} else {
		switch(param_id) {
		case VENDOR_VIDEOCAP_PARAM_BUILTIN_HDR: {
			BOOL *p_user = (BOOL *)p_param;
			cmd.dest = ISF_PORT(self_id, ISF_CTRL);
			cmd.param = VDOCAP_PARAM_BUILTIN_HDR;
			cmd.value = *p_user;
			cmd.size = 0;
			r = ISF_IOCTL(isf_fd, ISF_FLOW_CMD_SET_PARAM, &cmd);
			goto _VD_VC1;
		}
		case VENDOR_VIDEOCAP_PARAM_DATA_LANE: {
			UINT32 *p_user = (UINT32 *)p_param;

			cmd.dest = ISF_PORT(self_id, ISF_CTRL);
			cmd.param = VDOCAP_PARAM_DATA_LANE;
			cmd.value = *p_user;
			cmd.size = 0;
			r = ISF_IOCTL(isf_fd, ISF_FLOW_CMD_SET_PARAM, &cmd);
			goto _VD_VC1;
		}
		break;
		case VENDOR_VIDEOCAP_PARAM_ENC_RATE: {
			UINT32 *p_user = (UINT32 *)p_param;
			cmd.dest = ISF_PORT(self_id, ISF_CTRL);
			cmd.param = VDOCAP_PARAM_ENC_RATE;
			cmd.value = *p_user;
			cmd.size = 0;
			r = ISF_IOCTL(isf_fd, ISF_FLOW_CMD_SET_PARAM, &cmd);
			goto _VD_VC1;
		}
		case VENDOR_VIDEOCAP_PARAM_RESET_FC: {
			UINT32 *p_user = (UINT32 *)p_param;

			cmd.dest = ISF_PORT(self_id, ISF_CTRL);
			cmd.param = VDOCAP_PARAM_RESET_FC;
			cmd.value = *p_user;
			cmd.size = 0;
			r = ISF_IOCTL(isf_fd, ISF_FLOW_CMD_SET_PARAM, &cmd);
			goto _VD_VC1;
		}
		break;
		case VENDOR_VIDEOCAP_PARAM_CCIR_INFO: {
			VENDOR_VIDEOCAP_CCIR_INFO *p_user = (VENDOR_VIDEOCAP_CCIR_INFO *)p_param;
			VDOCAP_CCIR_INFO ccir_info;

			ccir_info.field_sel = (VDOCAP_SEN_FIELD_SEL)p_user->field_sel;
			ccir_info.fmt = (VDOCAP_SEN_CCIR_FMT_SEL)p_user->fmt;
			ccir_info.interlace = p_user->interlace;
			ccir_info.mux_data_index = p_user->mux_data_index;

			HD_VCAP_FLOW_MSG("    field_sel(%d) fmt(%d) interlace(%d) mux_data_index(%d)\n",
															ccir_info.field_sel,
															ccir_info.fmt,
															ccir_info.interlace,
															ccir_info.mux_data_index);
			cmd.dest = ISF_PORT(self_id, ISF_CTRL);
			cmd.param = VDOCAP_PARAM_CCIR_INFO;
			cmd.value = (ULONG)&ccir_info;
			cmd.size = sizeof(ccir_info);
			r = ISF_IOCTL(isf_fd, ISF_FLOW_CMD_SET_PARAM, &cmd);
			goto _VD_VC1;
		}
		case VENDOR_VIDEOCAP_PARAM_GYRO_INFO: {
			VENDOR_VIDEOCAP_GYRO_INFO *p_user = (VENDOR_VIDEOCAP_GYRO_INFO *)p_param;
			VDOCAP_GYRO_INFO gyro_info = {0};

			gyro_info.en = p_user->en;
			gyro_info.data_num = p_user->data_num;
			HD_VCAP_FLOW_MSG("    en(%d) data_num(%d)\n", gyro_info.en, gyro_info.data_num);
			cmd.dest = ISF_PORT(self_id, ISF_CTRL);
			cmd.param = VDOCAP_PARAM_GYRO_INFO;
			cmd.value = (ULONG)&gyro_info;
			cmd.size = sizeof(gyro_info);
			r = ISF_IOCTL(isf_fd, ISF_FLOW_CMD_SET_PARAM, &cmd);
			goto _VD_VC1;
		}
		case VENDOR_VIDEOCAP_PARAM_AE_PRESET: {
			VENDOR_VIDEOCAP_AE_PRESET *p_user = (VENDOR_VIDEOCAP_AE_PRESET *)p_param;
			VDOCAP_AE_PRESET ae_preset = {0};

			ae_preset.enable = p_user->enable;
			ae_preset.exp_time = p_user->exp_time;
			ae_preset.gain_ratio = p_user->gain_ratio;
			HD_VCAP_FLOW_MSG("    en(%d) exp_time(%d) gain_ratio(%d)\n", ae_preset.enable, ae_preset.exp_time, ae_preset.gain_ratio);
			cmd.dest = ISF_PORT(self_id, ISF_CTRL);
			cmd.param = VDOCAP_PARAM_AE_PRESET;
			cmd.value = (ULONG)&ae_preset;
			cmd.size = sizeof(ae_preset);
			r = ISF_IOCTL(isf_fd, ISF_FLOW_CMD_SET_PARAM, &cmd);
			goto _VD_VC1;
		}
		case VENDOR_VIDEOCAP_PARAM_DVS_INFO: {
			VENDOR_VIDEOCAP_DVS_INFO *p_user = (VENDOR_VIDEOCAP_DVS_INFO *)p_param;
			VDOCAP_DVS_INFO dvs_info = {0};

			dvs_info.enable = p_user->enable;
			dvs_info.positive = p_user->positive;
			dvs_info.negative = p_user->negative;
			dvs_info.nochange = p_user->nochange;
			HD_VCAP_FLOW_MSG("    en(%d) exp_time(%d) gain_ratio(%d)\n", dvs_info.enable, dvs_info.positive, dvs_info.negative, dvs_info.nochange);
			cmd.dest = ISF_PORT(self_id, ISF_CTRL);
			cmd.param = VDOCAP_PARAM_DVS_INFO;
			cmd.value = (ULONG)&dvs_info;
			cmd.size = sizeof(dvs_info);
			r = ISF_IOCTL(isf_fd, ISF_FLOW_CMD_SET_PARAM, &cmd);
			goto _VD_VC1;
		}
		case VENDOR_VIDEOCAP_PARAM_BP3_RATIO: {
			UINT32 *p_user = (UINT32 *)p_param;

			cmd.dest = ISF_PORT(self_id, ISF_CTRL);
			cmd.param = VDOCAP_PARAM_BP3_RATIO;
			cmd.value = *p_user;
			cmd.size = 0;
			r = ISF_IOCTL(isf_fd, ISF_FLOW_CMD_SET_PARAM, &cmd);
			goto _VD_VC1;
		}
		break;
		case VENDOR_VIDEOCAP_PARAM_QUEUE_FLUSH_SCHEME: {
			UINT32 *p_user = (UINT32 *)p_param;

			cmd.dest = ISF_PORT(self_id, ISF_CTRL);
			cmd.param = VDOCAP_PARAM_QUEUE_FLUSH_SCHEME;
			cmd.value = *p_user;
			cmd.size = 0;
			r = ISF_IOCTL(isf_fd, ISF_FLOW_CMD_SET_PARAM, &cmd);
			goto _VD_VC1;
		}
		break;
		case VENDOR_VIDEOCAP_PARAM_MODE_TYPE: {
			UINT32 mode_type = *(UINT32 *)p_param;

			cmd.dest = ISF_PORT(self_id, ISF_CTRL);
			cmd.param = VDOCAP_PARAM_MODE_TYPE;
			cmd.value =  mode_type;
			cmd.size = 0;
			r = ISF_IOCTL(isf_fd, ISF_FLOW_CMD_SET_PARAM, &cmd);
			goto _VD_VC1;
		}
		case VENDOR_VIDEOCAP_PARAM_PDAF_TYPE: {
			UINT32 pdaf_type = *(UINT32 *)p_param;

			cmd.dest = ISF_PORT(self_id, ISF_CTRL);
			cmd.param = VDOCAP_PARAM_PDAF_TYPE;
			cmd.value =  pdaf_type;
			cmd.size = 0;
			r = ISF_IOCTL(isf_fd, ISF_FLOW_CMD_SET_PARAM, &cmd);
			goto _VD_VC1;
		}
		case VENDOR_VIDEOCAP_PARAM_SSENIF_FORCE_DIS: {
			UINT32 force_disable = *(UINT32 *)p_param;

			cmd.dest = ISF_PORT(self_id, ISF_CTRL);
			cmd.param = VDOCAP_PARAM_SSENIF_FORCE_DIS;
			cmd.value =  force_disable;
			cmd.size = 0;
			r = ISF_IOCTL(isf_fd, ISF_FLOW_CMD_SET_PARAM, &cmd);
			goto _VD_VC1;
		}
		case VENDOR_VIDEOCAP_PARAM_KEEP_PINMUX_SETTING: {
			UINT32 keep_pinmux = *(UINT32 *)p_param;

			cmd.dest = ISF_PORT(self_id, ISF_CTRL);
			cmd.param = VDOCAP_PARAM_KEEP_PINMUX_SETTING;
			cmd.value =  keep_pinmux;
			cmd.size = 0;
			r = ISF_IOCTL(isf_fd, ISF_FLOW_CMD_SET_PARAM, &cmd);
			goto _VD_VC1;
		}
		case VENDOR_VIDEOCAP_PARAM_PDAF_PXLFMT: {
			UINT32 pdaf_type = *(UINT32 *)p_param;

			cmd.dest = ISF_PORT(self_id, ISF_CTRL);
			cmd.param = VDOCAP_PARAM_PDAF_PXLFMT;
			cmd.value =  pdaf_type;
			cmd.size = 0;
			r = ISF_IOCTL(isf_fd, ISF_FLOW_CMD_SET_PARAM, &cmd);
			goto _VD_VC1;
		}
		case VENDOR_VIDEOCAP_PARAM_EVS_INFO: {
			VENDOR_VIDEOCAP_EVS_INFO *p_user = (VENDOR_VIDEOCAP_EVS_INFO *)p_param;
			VDOCAP_EVS_INFO evs_info = {0};

			evs_info.max_out_pixel_cnt = p_user->max_out_pixel_cnt;
			evs_info.padding_val = p_user->padding_val;
			HD_VCAP_FLOW_MSG("    max_out_pixel_cnt(%d)\n", evs_info.max_out_pixel_cnt);
			HD_VCAP_FLOW_MSG("    padding_val(%d)\n", evs_info.padding_val);
			cmd.dest = ISF_PORT(self_id, ISF_CTRL);
			cmd.param = VDOCAP_PARAM_EVS_INFO;
			cmd.value = (ULONG)&evs_info;
			cmd.size = sizeof(evs_info);
			r = ISF_IOCTL(isf_fd, ISF_FLOW_CMD_SET_PARAM, &cmd);
			goto _VD_VC1;
		}
		case VENDOR_VIDEOCAP_PARAM_ADVANCE_PUSH: {
			UINT32 advance_push = *(UINT32 *)p_param;

			cmd.dest = ISF_PORT(self_id, ISF_CTRL);
			cmd.param = VDOCAP_PARAM_ADVANCE_PUSH;
			cmd.value =  advance_push;
			cmd.size = 0;
			r = ISF_IOCTL(isf_fd, ISF_FLOW_CMD_SET_PARAM, &cmd);
			goto _VD_VC1;
		}
		case VENDOR_VIDEOCAP_PARAM_TGE_CTRL: {
			UINT32 tge_ctrl = *(UINT32 *)p_param;

			cmd.dest = ISF_PORT(self_id, ISF_CTRL);
			cmd.param = VDOCAP_PARAM_TGE_CTRL;
			cmd.value =  tge_ctrl;
			cmd.size = 0;
			r = ISF_IOCTL(isf_fd, ISF_FLOW_CMD_SET_PARAM, &cmd);
			goto _VD_VC1;
		}
		case VENDOR_VIDEOCAP_PARAM_TSEN_CFG: {
			VENDOR_VCAP_TSEN_CFG *p_user = (VENDOR_VCAP_TSEN_CFG *)p_param;
			VDOCAP_TSEN_CFG tsen_cfg = {0};
			UINT32 i;

			tsen_cfg.len = p_user->len;
			memcpy(tsen_cfg.tx_data, p_user->tx_data, sizeof(tsen_cfg.tx_data));
			tsen_cfg.id = p_user->id;
			HD_VCAP_FLOW_MSG("    len(%d) id(%d)\n", tsen_cfg.len, tsen_cfg.id);
			for (i = 0; i < VDOCAP_TSEN_CFG_NUM; i++) {
				HD_VCAP_FLOW_MSG("    tx_data[%d] = 0x%X\n", i, tsen_cfg.tx_data[i]);
			}
			cmd.dest = ISF_PORT(self_id, ISF_CTRL);
			cmd.param = VDOCAP_PARAM_TSEN_CFG;
			cmd.value = (ULONG)&tsen_cfg;
			cmd.size = sizeof(tsen_cfg);
			r = ISF_IOCTL(isf_fd, ISF_FLOW_CMD_SET_PARAM, &cmd);
			goto _VD_VC1;
		}
		case VENDOR_VIDEOCAP_PARAM_TSEN_OOC: {
			VENDOR_VCAP_TSEN_OOC *p_user = (VENDOR_VCAP_TSEN_OOC *)p_param;
			VDOCAP_TSEN_OOC tsen_ooc = {0};

			tsen_ooc.buf_pa = p_user->buf_pa;
			tsen_ooc.pack_bus_sel = p_user->pack_bus_sel;
			tsen_ooc.lofs = p_user->lofs;
			tsen_ooc.id = p_user->id;
			HD_VCAP_FLOW_MSG("    buf_pa(0x%llX) pack_bus_sel(%d) lofs(%d) id(%d)\n", tsen_ooc.buf_pa, tsen_ooc.pack_bus_sel, tsen_ooc.lofs, tsen_ooc.id);

			cmd.dest = ISF_PORT(self_id, ISF_CTRL);
			cmd.param = VDOCAP_PARAM_TSEN_OOC;
			cmd.value = (ULONG)&tsen_ooc;
			cmd.size = sizeof(tsen_ooc);
			r = ISF_IOCTL(isf_fd, ISF_FLOW_CMD_SET_PARAM, &cmd);
			goto _VD_VC1;
		}
		default: rv = HD_ERR_PARAM; break;
		}
	}
	if(rv != HD_OK)
		return rv;

_VD_VC1:
	if (r == 0) {
		switch(cmd.rv) {
		case ISF_OK:
			rv = HD_OK;
			break;
		default:
			rv = HD_ERR_SYS;
			break;
		}
	} else {
		if(((int)cmd.rv <= ISF_ERR_BEGIN) && ((int)cmd.rv >= ISF_ERR_END)) {
			rv = cmd.rv; // ISF_ERR is exactly the same with HD_ERR
		} else {
			DBG_ERR("system fail, rv=%d\r\n", cmd.rv);
			rv = cmd.rv; // ISF_ERR is out of range of HD_ERR
		}
	}
	#endif
	return rv;
}

HD_RESULT vendor_videocap_set_lite(UINT32 id, VENDOR_VIDEOCAP_PARAM_ID param_id, VOID *p_param)
{
	HD_RESULT rv = HD_ERR_NG;

	if (p_param == NULL) {
		return HD_ERR_NULL_PTR;
	}

	#if defined (__FREERTOS) && _FASTBOOT_2A_TO_DO_
	if (param_id == VENDOR_VIDEOCAP_PARAM_FAST_OPEN_SENSOR) {
		return _fast_open_sensor(id, p_param);
	}
	if (param_id == VENDOR_VIDEOCAP_PARAM_FAST_CHANGE_MODE) {
		return _fast_change_mode(id, p_param);
	}
	if (param_id == VENDOR_VIDEOCAP_PARAM_FAST_START && (*(UINT32 *)p_param)) {
		return _fast_open_sie(id);
	}
	#else
	DBG_ERR("Not Ready!\r\n");
	#endif

	return rv;
}

#if !defined (__FREERTOS_LITE)
HD_RESULT vendor_videocap_get(UINT32 id, VENDOR_VIDEOCAP_PARAM_ID param_id, VOID *p_param)
{
	HD_DAL self_id = HD_GET_DEV(id);
	//HD_IO out_id = HD_GET_OUT(id);
	HD_IO ctrl_id = HD_GET_CTRL(id);
	ISF_FLOW_IOCTL_PARAM_ITEM cmd = {0};
	int isf_fd;
	int r;
	HD_RESULT rv = HD_ERR_NG;

	if (p_param == NULL) {
		return HD_ERR_NULL_PTR;
	}

	isf_fd = _hd_common_get_fd();
	if (isf_fd <= 0) {
		return HD_ERR_UNINIT;
	}
	_HD_CONVERT_SELF_ID(self_id, rv); 	if(rv != HD_OK) {	return rv;}

	{
		CHAR  param_name[20];
		vendor_videocap_param_cvt_name(param_id, param_name, 20);

		HD_VCAP_FLOW_IND("vendor_videocap_get(%s):\n", param_name);
		HD_VCAP_FLOW_IND("    id(0x%x) param(0x%X)\n", id, *(UINT32 *)p_param);
	}
	rv = HD_OK;

	if(ctrl_id == HD_CTRL) {
		switch(param_id) {
		case VENDOR_VIDEOCAP_PARAM_GET_PLUG: {
			UINT32 *p_user = (UINT32 *)p_param;
			cmd.dest = ISF_PORT(self_id, ISF_CTRL);
			cmd.param = VDOCAP_PARAM_GET_PLUG;
			cmd.size = 0;
			r = ISF_IOCTL(isf_fd, ISF_FLOW_CMD_GET_PARAM, &cmd);
			if (r == 0 && cmd.rv == ISF_OK) {
				*p_user = cmd.value;
			}
			HD_VCAP_FLOW_IND("    value(%d)\n", cmd.value);
			goto _VD_VC2;
		}
		case VENDOR_VIDEOCAP_PARAM_GET_PLUG_INFO: {
			VENDOR_VIDEOCAP_GET_PLUG_INFO* p_user = (VENDOR_VIDEOCAP_GET_PLUG_INFO *)p_param;
			//VENDOR_VIDEOCAP_GET_PLUG_INFO get_plug_info = {0};
			cmd.dest = ISF_PORT(self_id, ISF_CTRL);
			cmd.param = VDOCAP_PARAM_GET_PLUG_INFO;
			//cmd.value = (UINT32)&get_plug_info;
			cmd.value = (ULONG)p_user;
			cmd.size = sizeof(VENDOR_VIDEOCAP_GET_PLUG_INFO);
			r = ISF_IOCTL(isf_fd, ISF_FLOW_CMD_GET_PARAM, &cmd);
			HD_VCAP_FLOW_IND("    size(%dx%d) fps(%d) interlace(%d)\n", p_user->size.w, p_user->size.h, p_user->fps, p_user->interlace);
			goto _VD_VC2;
		}
		case VENDOR_VIDEOCAP_PARAM_CSI_ERR_CNT: {
			UINT32 *p_user = (UINT32 *)p_param;
			cmd.dest = ISF_PORT(self_id, ISF_CTRL);
			cmd.param = VDOCAP_PARAM_CSI_ERR_CNT;
			cmd.size = 0;
			r = ISF_IOCTL(isf_fd, ISF_FLOW_CMD_GET_PARAM, &cmd);
			if (r == 0 && cmd.rv == ISF_OK) {
				*p_user = cmd.value;
			}
			HD_VCAP_FLOW_IND("    value(%d)\n", cmd.value);
			goto _VD_VC2;
		}
		default: rv = HD_ERR_PARAM; break;
		}
	} else {
		switch(param_id) {
		default: rv = HD_ERR_PARAM; break;
		}
	}
	if(rv != HD_OK)
		return rv;

_VD_VC2:
	if (r == 0) {
		switch(cmd.rv) {
		case ISF_OK:
			rv = HD_OK;
			break;
		default:
			rv = HD_ERR_SYS;
			break;
		}
	} else {
		if(((int)cmd.rv <= ISF_ERR_BEGIN) && ((int)cmd.rv >= ISF_ERR_END)) {
			rv = cmd.rv; // ISF_ERR is exactly the same with HD_ERR
		} else {
			DBG_ERR("system fail, rv=%d\r\n", cmd.rv);
			rv = cmd.rv; // ISF_ERR is out of range of HD_ERR
		}
	}
	return rv;
}
#endif
