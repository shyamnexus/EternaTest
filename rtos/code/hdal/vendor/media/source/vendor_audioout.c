/**
	@brief Source file of vendor media audioout.\n

	@file vendor_audioout.c

	@ingroup mhdal

	@note Nothing.

	Copyright Novatek Microelectronics Corp. 2018.  All rights reserved.
*/
#if defined(__LINUX)
#include <sys/ioctl.h>
#endif
#include <stdarg.h>
#include "hd_logger_p.h"
#include "kflow_common/isf_flow_def.h"
#include "kflow_common/isf_flow_ioctl.h"
#include "vendor_audioout.h"
#include "kflow_audioout/isf_audout.h"
#include "kflow_common/isf_flow_def.h"
#include "kflow_common/isf_flow_ioctl.h"

#if defined (__UITRON) || defined(__ECOS)  || defined (__FREERTOS)
#define ISF_OPEN     isf_flow_open
#define ISF_IOCTL    isf_flow_ioctl
#define ISF_CLOSE    isf_flow_close

#define DBG_ERR(fmt, args...)
#define DBG_DUMP
#endif
#if defined(__LINUX)
#define ISF_OPEN     open
#define ISF_IOCTL    ioctl
#define ISF_CLOSE    close

#define DBG_ERR(fmt, args...) 	printf("%s: " fmt, __func__, ##args)
#define DBG_DUMP				printf
#endif

/*-----------------------------------------------------------------------------*/
/* Local Constant Definitions                                                  */
/*-----------------------------------------------------------------------------*/
#define HD_DEV_BASE HD_DAL_AUDIOOUT_BASE
#define HD_DEV_MAX  HD_DAL_AUDIOOUT_MAX
#define DEV_BASE        ISF_UNIT_AUDOUT
#define DEV_COUNT       ISF_MAX_AUDOUT
#define OUT_BASE        ISF_OUT_BASE
#define OUT_COUNT       1

#define _HD_CONVERT_SELF_ID(dev_id, rv) \
	do { \
		(rv) = HD_ERR_DEV;  \
		if((dev_id) == 0) { \
			(rv) = HD_ERR_UNIQUE; \
		} else if((dev_id) >= HD_DEV_BASE && (dev_id) <= HD_DEV_MAX) { \
			HD_DAL id = (dev_id) - HD_DEV_BASE; \
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
			HD_IO id = (out_id) - HD_OUT_BASE; \
			if(id < OUT_COUNT) { \
				(out_id) = OUT_BASE + id; \
				(rv) = HD_OK; \
			} \
		} \
	} while(0)

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
extern HD_RESULT _hd_audioout_kflow_param_set_by_vendor(HD_DAL self_id, HD_IO out_id, UINT param_id, UINT32 param);

/*-----------------------------------------------------------------------------*/
/* Local Function Prototype                                                    */
/*-----------------------------------------------------------------------------*/
INT _vendor_audioout_param_cvt_name(VENDOR_AUDIOOUT_ITEM  id, CHAR *p_ret_string, INT max_str_len)
{
	switch (id) {
		case VENDOR_AUDIOOUT_ITEM_EXT:              snprintf(p_ret_string, max_str_len, "EXT");             break;
		case VENDOR_AUDIOOUT_ITEM_AUDIO_FILTER_EN:  snprintf(p_ret_string, max_str_len, "FILTER");          break;
		case VENDOR_AUDIOOUT_ITEM_WAIT_PUSH:        snprintf(p_ret_string, max_str_len, "WAIT_PUSH");       break;
		case VENDOR_AUDIOOUT_ITEM_PRELOAD_DONE:     snprintf(p_ret_string, max_str_len, "PRELOAD_DONE");    break;
		case VENDOR_AUDIOOUT_ITEM_DONE_SIZE:        snprintf(p_ret_string, max_str_len, "DONE_SIZE");       break;
		case VENDOR_AUDIOOUT_ITEM_NEEDED_BUF:       snprintf(p_ret_string, max_str_len, "NEEDED");          break;
		case VENDOR_AUDIOOUT_ITEM_ALLOC_BUF:        snprintf(p_ret_string, max_str_len, "ALLOC");           break;
		case VENDOR_AUDIOOUT_ITEM_CHN_STATE:        snprintf(p_ret_string, max_str_len, "CHN_STATE");       break;
		case VENDOR_AUDIOOUT_ITEM_VOLUME:           snprintf(p_ret_string, max_str_len, "VOLUME");          break;
		case VENDOR_AUDIOOUT_ITEM_PREPWR_ENABLE:    snprintf(p_ret_string, max_str_len, "PREPWR");          break;
		case VENDOR_AUDIOOUT_ITEM_DMA_ABORT:        snprintf(p_ret_string, max_str_len, "ABORT");           break;
		default:
			snprintf(p_ret_string, max_str_len, "error");
			printf("unknown param_id(%d)\r\n", id);
			return (-1);
	}
	return 0;
}

/*-----------------------------------------------------------------------------*/
/* Debug Variables & Functions                                                 */
/*-----------------------------------------------------------------------------*/
#define HD_AOUT_DBG_FATAL     0
#define HD_AOUT_DBG_ERR       1
#define HD_AOUT_DBG_WRN       2
#define HD_AOUT_DBG_MSG       3
#define HD_AOUT_DBG_IND       4
#define HD_AOUT_DBG_FUNC      5

#define HD_AOUT_FLOW_ERR(fmt, args...) { if (hd_flow_dbg_lvl[HD_FLOW_DBG_AOUT] && (*hd_flow_dbg_lvl[HD_FLOW_DBG_AOUT]) >= HD_AOUT_DBG_ERR) { hdal_flow_log_p(fmt, ##args);  }}
#define HD_AOUT_FLOW_WRN(fmt, args...) { if (hd_flow_dbg_lvl[HD_FLOW_DBG_AOUT] && (*hd_flow_dbg_lvl[HD_FLOW_DBG_AOUT]) >= HD_AOUT_DBG_WRN) { hdal_flow_log_p(fmt, ##args);  }}
#define HD_AOUT_FLOW_MSG(fmt, args...) { if (hd_flow_dbg_lvl[HD_FLOW_DBG_AOUT] && (*hd_flow_dbg_lvl[HD_FLOW_DBG_AOUT]) >= HD_AOUT_DBG_MSG) { hdal_flow_log_p(fmt, ##args);  }}
#define HD_AOUT_FLOW_IND(fmt, args...) { if (hd_flow_dbg_lvl[HD_FLOW_DBG_AOUT] && (*hd_flow_dbg_lvl[HD_FLOW_DBG_AOUT]) >= HD_AOUT_DBG_IND) { hdal_flow_log_p(fmt, ##args);  }}
#define HD_AOUT_FLOW_FUNC(fmt, args...) { if (hd_flow_dbg_lvl[HD_FLOW_DBG_AOUT] && (*hd_flow_dbg_lvl[HD_FLOW_DBG_AOUT]) >= HD_AOUT_DBG_FUNC) { hdal_flow_log_p(fmt, ##args);  }}

/*-----------------------------------------------------------------------------*/
/* Local Global Variables                                                      */
/*-----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------*/
/* Interface Functions                                                         */
/*-----------------------------------------------------------------------------*/
HD_RESULT vendor_audioout_set(HD_PATH_ID id, VENDOR_AUDIOOUT_ITEM item, VOID *p_param)
{
	HD_RESULT ret;
	int isf_fd;

	HD_DAL self_id = HD_GET_DEV(id);
	HD_IO ctrl_id = HD_GET_CTRL(id);
	HD_IO out_id = HD_GET_OUT(id);
	ISF_FLOW_IOCTL_PARAM_ITEM cmd = {0};
	int r;

	{
		CHAR  param_name[20];
		_vendor_audioout_param_cvt_name(item, param_name, 20);

		HD_AOUT_FLOW_MSG("vendor_audioout_set(%s):\n", param_name);
		HD_AOUT_FLOW_MSG("    path_id(0x%x) ", id);
	}

	if (p_param == NULL) {
		return HD_ERR_NULL_PTR;
	}

	switch (item) {
		case VENDOR_AUDIOOUT_ITEM_EXT: {
			AUDOUT_AUD_INIT_CFG init_config = {0};
			VENDOR_AUDIOOUT_INIT_CFG* p_user = (VENDOR_AUDIOOUT_INIT_CFG*)p_param;

			isf_fd = _hd_common_get_fd();

			if (isf_fd <= 0) {
				return HD_ERR_UNINIT;
			}

			_HD_CONVERT_SELF_ID(self_id, ret);
			if (ret != HD_OK) {
				return ret;
			}

			HD_AOUT_FLOW_MSG("bit_width(%u) bit_clk_ratio(%u) op_mode(%u) tdm_ch(%u)\n",
				p_user->aud_init_cfg.i2s_cfg.bit_width, p_user->aud_init_cfg.i2s_cfg.bit_clk_ratio,
				p_user->aud_init_cfg.i2s_cfg.op_mode, p_user->aud_init_cfg.i2s_cfg.tdm_ch);

			ret = HD_OK;

			if (ctrl_id != HD_CTRL) {
				ret = HD_ERR_IO;
				//printf("vendor_audioout_set invalid id %x\n", id);
				return ret;
			}

			if (p_user->driver_name[0] != 0) {
				snprintf(init_config.driver_name, VENDOR_AUDIOOUT_NAME_LEN-1, p_user->driver_name);
				init_config.aud_init_cfg.pin_cfg.pinmux.audio_pinmux  = p_user->aud_init_cfg.pin_cfg.pinmux.audio_pinmux;
				init_config.aud_init_cfg.pin_cfg.pinmux.cmd_if_pinmux = p_user->aud_init_cfg.pin_cfg.pinmux.cmd_if_pinmux;
				init_config.aud_init_cfg.i2s_cfg.bit_width     = p_user->aud_init_cfg.i2s_cfg.bit_width;
				init_config.aud_init_cfg.i2s_cfg.bit_clk_ratio = p_user->aud_init_cfg.i2s_cfg.bit_clk_ratio;
				init_config.aud_init_cfg.i2s_cfg.op_mode       = p_user->aud_init_cfg.i2s_cfg.op_mode;
				init_config.aud_init_cfg.i2s_cfg.tdm_ch        = p_user->aud_init_cfg.i2s_cfg.tdm_ch;
			} else {
				init_config.driver_name[0] = 0;
			}
			cmd.dest = ISF_PORT(self_id, ISF_CTRL);
			cmd.param = AUDOUT_PARAM_AUD_INIT_CFG;
			cmd.value = (ULONG)&init_config;
			cmd.size = sizeof(AUDOUT_AUD_INIT_CFG);
			r = ISF_IOCTL(isf_fd, ISF_FLOW_CMD_SET_PARAM, &cmd);

			if (r == 0) {
				switch (cmd.rv) {
				case ISF_OK:
					ret = HD_OK;
					break;
				default:
					ret = HD_ERR_SYS;
					break;
				}
			} else {
				if(((int)cmd.rv <= ISF_ERR_BEGIN) && ((int)cmd.rv >= ISF_ERR_END)) {
					ret= cmd.rv; // ISF_ERR is exactly the same with HD_ERR
				} else {
					DBG_ERR("system fail, rv=%d\r\n", (int)cmd.rv);
					ret = cmd.rv; // ISF_ERR is out of range of HD_ERR
				}
			}
			break;
		}
		#if (AUDOUT_LIB_SRC == ENABLE)
		case VENDOR_AUDIOOUT_ITEM_AUDIO_FILTER_EN: {
			UINT32 enable = *(UINT32*)p_param;

			isf_fd = _hd_common_get_fd();

			if (isf_fd <= 0) {
				return HD_ERR_UNINIT;
			}

			_HD_CONVERT_SELF_ID(self_id, ret);
			if (ret != HD_OK) {
				return ret;
			}

			HD_AOUT_FLOW_MSG("enable(%u)\n", enable);

			ret = HD_OK;

			if (ctrl_id != HD_CTRL) {
				ret = HD_ERR_IO;
				//printf("vendor_audioout_set invalid id %x\n", id);
				return ret;
			}

			cmd.dest = ISF_PORT(self_id, ISF_CTRL);
			cmd.param = AUDOUT_PARAM_AUD_LPF_EN;
			cmd.value = enable;
			cmd.size = 0;
			r = ISF_IOCTL(isf_fd, ISF_FLOW_CMD_SET_PARAM, &cmd);

			if (r == 0) {
				switch (cmd.rv) {
				case ISF_OK:
					ret = HD_OK;
					break;
				default:
					ret = HD_ERR_SYS;
					break;
				}
			} else {
				if(((int)cmd.rv <= ISF_ERR_BEGIN) && ((int)cmd.rv >= ISF_ERR_END)) {
					ret= cmd.rv; // ISF_ERR is exactly the same with HD_ERR
				} else {
					DBG_ERR("system fail, rv=%d\r\n", (int)cmd.rv);
					ret = cmd.rv; // ISF_ERR is out of range of HD_ERR
				}
			}
			break;
		}
		#endif
		case VENDOR_AUDIOOUT_ITEM_WAIT_PUSH: {
			UINT32 enable = *(UINT32*)p_param;

			isf_fd = _hd_common_get_fd();

			if (isf_fd <= 0) {
				return HD_ERR_UNINIT;
			}

			_HD_CONVERT_SELF_ID(self_id, ret);
			if (ret != HD_OK) {
				return ret;
			}

			HD_AOUT_FLOW_MSG("enable(%u)\n", enable);

			ret = HD_OK;

			if (ctrl_id != HD_CTRL) {
				ret = HD_ERR_IO;
				//printf("vendor_audioout_set invalid id %x\n", id);
				return ret;
			}

			cmd.dest = ISF_PORT(self_id, ISF_CTRL);
			cmd.param = AUDOUT_PARAM_WAIT_PUSH;
			cmd.value = enable;
			cmd.size = 0;
			r = ISF_IOCTL(isf_fd, ISF_FLOW_CMD_SET_PARAM, &cmd);

			if (r == 0) {
				switch (cmd.rv) {
				case ISF_OK:
					ret = HD_OK;
					break;
				default:
					ret = HD_ERR_SYS;
					break;
				}
			} else {
				if(((int)cmd.rv <= ISF_ERR_BEGIN) && ((int)cmd.rv >= ISF_ERR_END)) {
					ret= cmd.rv; // ISF_ERR is exactly the same with HD_ERR
				} else {
					DBG_ERR("system fail, rv=%d\r\n", (int)cmd.rv);
					ret = cmd.rv; // ISF_ERR is out of range of HD_ERR
				}
			}
			break;
		}
		case VENDOR_AUDIOOUT_ITEM_ALLOC_BUF: {
			AUDOUT_AUD_MEM mem = {0};
			VENDOR_AUDIOOUT_MEM* p_user = (VENDOR_AUDIOOUT_MEM*)p_param;

			isf_fd = _hd_common_get_fd();

			if (isf_fd <= 0) {
				return HD_ERR_UNINIT;
			}

			_HD_CONVERT_SELF_ID(self_id, ret);
			if (ret != HD_OK) {
				return ret;
			}

			HD_AOUT_FLOW_MSG("mem.pa(0x%lx) mem.va(0x%lx) mem.size(0x%lx)\n",
				(unsigned long)p_user->pa, (unsigned long)p_user->va, (unsigned long)p_user->size);

			ret = HD_OK;

			if (ctrl_id != HD_CTRL) {
				ret = HD_ERR_IO;
				//printf("vendor_audioout_set invalid id %x\n", id);
				return ret;
			}

			mem.pa = p_user->pa;
			mem.va = p_user->va;
			mem.size = p_user->size;

			cmd.dest = ISF_PORT(self_id, ISF_CTRL);
			cmd.param = AUDOUT_PARAM_ALLOC_BUFF;
			cmd.value = (ULONG)&mem;
			cmd.size = sizeof(AUDOUT_AUD_MEM);
			r = ISF_IOCTL(isf_fd, ISF_FLOW_CMD_SET_PARAM, &cmd);

			if (r == 0) {
				switch (cmd.rv) {
				case ISF_OK:
					ret = HD_OK;
					break;
				default:
					ret = HD_ERR_SYS;
					break;
				}
			} else {
				if(((int)cmd.rv <= ISF_ERR_BEGIN) && ((int)cmd.rv >= ISF_ERR_END)) {
					ret= cmd.rv; // ISF_ERR is exactly the same with HD_ERR
				} else {
					DBG_ERR("system fail, rv=%d\r\n", (int)cmd.rv);
					ret = cmd.rv; // ISF_ERR is out of range of HD_ERR
				}
			}
			break;
		}
		case VENDOR_AUDIOOUT_ITEM_VOLUME: {
			VENDOR_AUDIOOUT_VOLUME* vol = (VENDOR_AUDIOOUT_VOLUME*)p_param;

			isf_fd = _hd_common_get_fd();

			if (isf_fd <= 0) {
				return HD_ERR_UNINIT;
			}

			if(!(out_id >= HD_OUT_BASE && out_id <= HD_OUT_MAX)) {
				return HD_ERR_IO;
			}

			_HD_CONVERT_SELF_ID(self_id, ret);
			if (ret != HD_OK) {
				return ret;
			}

			_HD_CONVERT_OUT_ID(out_id, ret);
			if(ret != HD_OK) {
				return ret;
			}

			_hd_audioout_kflow_param_set_by_vendor(self_id, out_id, AUDOUT_PARAM_VOL_IMM, vol->volume);

			HD_AOUT_FLOW_MSG("vol(%u)\n", vol->volume);

			ret = HD_OK;

			cmd.dest = ISF_PORT(self_id, out_id);
			cmd.param = AUDOUT_PARAM_VOL_IMM;
			cmd.value = vol->volume;
			cmd.size = 0;
			r = ISF_IOCTL(isf_fd, ISF_FLOW_CMD_SET_PARAM, &cmd);

			if (r == 0) {
				switch (cmd.rv) {
				case ISF_OK:
					ret = HD_OK;
					break;
				default:
					ret = HD_ERR_SYS;
					break;
				}
			} else {
				if(((int)cmd.rv <= ISF_ERR_BEGIN) && ((int)cmd.rv >= ISF_ERR_END)) {
					ret= cmd.rv; // ISF_ERR is exactly the same with HD_ERR
				} else {
					DBG_ERR("system fail, rv=%d\r\n", (int)cmd.rv);
					ret = cmd.rv; // ISF_ERR is out of range of HD_ERR
				}
			}

			break;
		}
		case VENDOR_AUDIOOUT_ITEM_PREPWR_ENABLE: {
			INT32 enable = *(INT32*)p_param;

			isf_fd = _hd_common_get_fd();

			if (isf_fd <= 0) {
				return HD_ERR_UNINIT;
			}

			_HD_CONVERT_SELF_ID(self_id, ret);
			if (ret != HD_OK) {
				return ret;
			}

			HD_AOUT_FLOW_MSG("enable(%u)\n", enable);

			ret = HD_OK;

			if (ctrl_id != HD_CTRL) {
				ret = HD_ERR_IO;
				//printf("vendor_audiocapture_set invalid id %x\n", id);
				return ret;
			}

			cmd.dest = ISF_PORT(self_id, ISF_CTRL);
			cmd.param = AUDOUT_PARAM_AUD_PREPWR_EN;
			cmd.value = enable;
			cmd.size = 0;
			r = ISF_IOCTL(isf_fd, ISF_FLOW_CMD_SET_PARAM, &cmd);

			if (r == 0) {
				switch (cmd.rv) {
				case ISF_OK:
					ret = HD_OK;
					break;
				default:
					ret = HD_ERR_SYS;
					break;
				}
			} else {
				if(((int)cmd.rv <= ISF_ERR_BEGIN) && ((int)cmd.rv >= ISF_ERR_END)) {
					ret= cmd.rv; // ISF_ERR is exactly the same with HD_ERR
				} else {
					DBG_ERR("system fail, rv=%d\r\n", (int)cmd.rv);
					ret = cmd.rv; // ISF_ERR is out of range of HD_ERR
				}
			}
			break;
		}
		case VENDOR_AUDIOOUT_ITEM_DMA_ABORT: {
			UINT32 enable = *(UINT32*)p_param;

			isf_fd = _hd_common_get_fd();

			if (isf_fd <= 0) {
				return HD_ERR_UNINIT;
			}

			_HD_CONVERT_SELF_ID(self_id, ret);
			if (ret != HD_OK) {
				return ret;
			}

			HD_AOUT_FLOW_MSG("enable(%u)\n", enable);

			ret = HD_OK;

			if (ctrl_id != HD_CTRL) {
				ret = HD_ERR_IO;
				//printf("vendor_audiocapture_set invalid id %x\n", id);
				return ret;
			}

			cmd.dest = ISF_PORT(self_id, ISF_CTRL);
			cmd.param = AUDOUT_PARAM_DMA_ABORT;
			cmd.value = enable;
			cmd.size = 0;
			r = ISF_IOCTL(isf_fd, ISF_FLOW_CMD_SET_PARAM, &cmd);

			if (r == 0) {
				switch (cmd.rv) {
				case ISF_OK:
					ret = HD_OK;
					break;
				default:
					ret = HD_ERR_SYS;
					break;
				}
			} else {
				if(((int)cmd.rv <= ISF_ERR_BEGIN) && ((int)cmd.rv >= ISF_ERR_END)) {
					ret= cmd.rv; // ISF_ERR is exactly the same with HD_ERR
				} else {
					DBG_ERR("system fail, rv=%d\r\n", (int)cmd.rv);
					ret = cmd.rv; // ISF_ERR is out of range of HD_ERR
				}
			}
			break;
		}
		default:
			ret = HD_ERR_PARAM;
			//printf("vendor_audioout_set not support item %d \n", item);
			break;
	}

	return ret;
}

HD_RESULT vendor_audioout_get(HD_PATH_ID id, VENDOR_AUDIOOUT_ITEM item, VOID *p_param)
{
	HD_RESULT ret;
	int isf_fd;

	HD_DAL self_id = HD_GET_DEV(id);
	HD_IO ctrl_id = HD_GET_CTRL(id);
	HD_IO out_id = HD_GET_OUT(id);
	ISF_FLOW_IOCTL_PARAM_ITEM cmd = {0};
	int r;

	{
		CHAR  param_name[20];
		_vendor_audioout_param_cvt_name(item, param_name, 20);

		HD_AOUT_FLOW_MSG("vendor_audioout_get(%s):\n", param_name);
		HD_AOUT_FLOW_MSG("    path_id(0x%x) ", id);
	}

	if (p_param == NULL) {
		return HD_ERR_NULL_PTR;
	}

	switch (item) {
		case VENDOR_AUDIOOUT_ITEM_PRELOAD_DONE: {
			BOOL* p_user = (BOOL*)p_param;

			isf_fd = _hd_common_get_fd();

			if (isf_fd <= 0) {
				return HD_ERR_UNINIT;
			}

			_HD_CONVERT_SELF_ID(self_id, ret);
			if (ret != HD_OK) {
				return ret;
			}

			ret = HD_OK;

			if (ctrl_id != HD_CTRL) {
				ret = HD_ERR_IO;
				//printf("vendor_audioout_set invalid id %x\n", id);
				return ret;
			}

			cmd.dest = ISF_PORT(self_id, ISF_CTRL);
			cmd.param = AUDOUT_PARAM_PRELOAD_DONE;
			cmd.size = 0;

			r = ISF_IOCTL(isf_fd, ISF_FLOW_CMD_GET_PARAM, &cmd);

			if (r == 0) {
				switch(cmd.rv) {
				case ISF_OK:
					ret = HD_OK;
					*p_user = cmd.value;
					HD_AOUT_FLOW_MSG("value(%u)\n", cmd.value);
					break;
				default:
					ret = HD_ERR_SYS;
					break;
				}
			} else {
				if(((int)cmd.rv <= ISF_ERR_BEGIN) && ((int)cmd.rv >= ISF_ERR_END)) {
					ret = cmd.rv; // ISF_ERR is exactly the same with HD_ERR
				} else {
					DBG_ERR("system fail, rv=%d\r\n", (int)cmd.rv);
					ret = cmd.rv; // ISF_ERR is out of range of HD_ERR
				}
			}

			break;
		}
		case VENDOR_AUDIOOUT_ITEM_DONE_SIZE: {
			UINT32* p_user = (UINT32*)p_param;

			isf_fd = _hd_common_get_fd();

			if (isf_fd <= 0) {
				return HD_ERR_UNINIT;
			}

			_HD_CONVERT_SELF_ID(self_id, ret);
			if (ret != HD_OK) {
				return ret;
			}

			ret = HD_OK;

			if (ctrl_id != HD_CTRL) {
				ret = HD_ERR_IO;
				//printf("vendor_audioout_set invalid id %x\n", id);
				return ret;
			}

			cmd.dest = ISF_PORT(self_id, ISF_CTRL);
			cmd.param = AUDOUT_PARAM_DONE_SIZE;
			cmd.size = 0;

			r = ISF_IOCTL(isf_fd, ISF_FLOW_CMD_GET_PARAM, &cmd);

			if (r == 0) {
				switch(cmd.rv) {
				case ISF_OK:
					ret = HD_OK;
					*p_user = cmd.value;
					HD_AOUT_FLOW_MSG("value(%u)\n", cmd.value);
					break;
				default:
					ret = HD_ERR_SYS;
					break;
				}
			} else {
				if(((int)cmd.rv <= ISF_ERR_BEGIN) && ((int)cmd.rv >= ISF_ERR_END)) {
					ret = cmd.rv; // ISF_ERR is exactly the same with HD_ERR
				} else {
					DBG_ERR("system fail, rv=%d\r\n", (int)cmd.rv);
					ret = cmd.rv; // ISF_ERR is out of range of HD_ERR
				}
			}

			break;
		}
		case VENDOR_AUDIOOUT_ITEM_NEEDED_BUF: {
			UINT32* p_user = (UINT32*)p_param;

			isf_fd = _hd_common_get_fd();

			if (isf_fd <= 0) {
				return HD_ERR_UNINIT;
			}

			_HD_CONVERT_SELF_ID(self_id, ret);
			if (ret != HD_OK) {
				return ret;
			}

			ret = HD_OK;

			if (ctrl_id != HD_CTRL) {
				ret = HD_ERR_IO;
				//printf("vendor_audioout_set invalid id %x\n", id);
				return ret;
			}

			cmd.dest = ISF_PORT(self_id, ISF_CTRL);
			cmd.param = AUDOUT_PARAM_NEEDED_BUFF;
			cmd.size = 0;

			r = ISF_IOCTL(isf_fd, ISF_FLOW_CMD_GET_PARAM, &cmd);

			if (r == 0) {
				switch(cmd.rv) {
				case ISF_OK:
					ret = HD_OK;
					*p_user = cmd.value;
					HD_AOUT_FLOW_MSG("value(%u)\n", cmd.value);
					break;
				default:
					ret = HD_ERR_SYS;
					break;
				}
			} else {
				if(((int)cmd.rv <= ISF_ERR_BEGIN) && ((int)cmd.rv >= ISF_ERR_END)) {
					ret = cmd.rv; // ISF_ERR is exactly the same with HD_ERR
				} else {
					DBG_ERR("system fail, rv=%d\r\n", (int)cmd.rv);
					ret = cmd.rv; // ISF_ERR is out of range of HD_ERR
				}
			}

			break;
		}
		case VENDOR_AUDIOOUT_ITEM_CHN_STATE: {
			VENDOR_AUDIOOUT_CHN_STATE* p_user = (VENDOR_AUDIOOUT_CHN_STATE*)p_param;
			AUDOUT_CHN_STATE chn_state;

			isf_fd = _hd_common_get_fd();

			if (isf_fd <= 0) {
				return HD_ERR_UNINIT;
			}

			if(!(out_id >= HD_OUT_BASE && out_id <= HD_OUT_MAX)) {
				return HD_ERR_IO;
			}

			_HD_CONVERT_SELF_ID(self_id, ret);
			if (ret != HD_OK) {
				return ret;
			}

			_HD_CONVERT_OUT_ID(out_id, ret);
			if(ret != HD_OK) {
				return ret;
			}

			ret = HD_OK;

			cmd.dest = ISF_PORT(self_id, out_id);
			cmd.param = AUDOUT_PARAM_CHN_STATE;
			cmd.size = sizeof(AUDOUT_CHN_STATE);
			cmd.value = (ULONG)&chn_state;

			r = ISF_IOCTL(isf_fd, ISF_FLOW_CMD_GET_PARAM, &cmd);

			if (r == 0) {
				switch(cmd.rv) {
				case ISF_OK:
					ret = HD_OK;
					p_user->total_num = chn_state.total_num;
					p_user->busy_num = chn_state.busy_num;
					p_user->free_num = chn_state.free_num;
					HD_AOUT_FLOW_MSG("total(%u) busy(%u) free(%u)\n",
						p_user->total_num, p_user->busy_num, p_user->free_num);
					break;
				default:
					ret = HD_ERR_SYS;
					break;
				}
			} else {
				if(((int)cmd.rv <= ISF_ERR_BEGIN) && ((int)cmd.rv >= ISF_ERR_END)) {
					ret = cmd.rv; // ISF_ERR is exactly the same with HD_ERR
				} else {
					DBG_ERR("system fail, rv=%d\r\n", (int)cmd.rv);
					ret = cmd.rv; // ISF_ERR is out of range of HD_ERR
				}
			}

			break;
		}
		default:
			ret = HD_ERR_PARAM;
			//printf("vendor_audioout_set not support item %d \n", item);
			break;
	}

	return ret;
}
