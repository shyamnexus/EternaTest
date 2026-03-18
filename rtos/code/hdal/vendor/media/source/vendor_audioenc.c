/**
	@brief Source file of vendor media audioenc.\n

	@file vendor_audioenc.c

	@ingroup mhdal

	@note Nothing.

	Copyright Novatek Microelectronics Corp. 2018.  All rights reserved.
*/
#if defined(__LINUX)
#include <sys/ioctl.h>
#endif
#include "kflow_common/isf_flow_def.h"
#include "kflow_common/isf_flow_ioctl.h"
#include "vendor_audioenc.h"
#include "kflow_audioenc/isf_audenc.h"
#include "kflow_videoenc/media_def.h"
#include "kflow_common/isf_flow_def.h"
#include "kflow_common/isf_flow_ioctl.h"
#include <stdarg.h>
#include "hd_logger_p.h"

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
#define HD_DEV_BASE HD_DAL_AUDIOENC_BASE
#define HD_DEV_MAX  HD_DAL_AUDIOENC_MAX
#define DEV_BASE        ISF_UNIT_AUDENC
#define DEV_COUNT       ISF_MAX_AUDENC
#define OUT_BASE        ISF_OUT_BASE
#define OUT_COUNT       2

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

/*-----------------------------------------------------------------------------*/
/* Local Function Prototype                                                    */
/*-----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------*/
/* Debug Variables & Functions                                                 */
/*-----------------------------------------------------------------------------*/
#define HD_AENC_DBG_FATAL     0
#define HD_AENC_DBG_ERR       1
#define HD_AENC_DBG_WRN       2
#define HD_AENC_DBG_MSG       3
#define HD_AENC_DBG_IND       4
#define HD_AENC_DBG_FUNC      5

#define HD_AENC_FLOW_ERR(fmt, args...) { if (hd_flow_dbg_lvl[HD_FLOW_DBG_AENC] && (*hd_flow_dbg_lvl[HD_FLOW_DBG_AENC]) >= HD_AENC_DBG_ERR) { hdal_flow_log_p(fmt, ##args);  }}
#define HD_AENC_FLOW_WRN(fmt, args...) { if (hd_flow_dbg_lvl[HD_FLOW_DBG_AENC] && (*hd_flow_dbg_lvl[HD_FLOW_DBG_AENC]) >= HD_AENC_DBG_WRN) { hdal_flow_log_p(fmt, ##args);  }}
#define HD_AENC_FLOW_MSG(fmt, args...) { if (hd_flow_dbg_lvl[HD_FLOW_DBG_AENC] && (*hd_flow_dbg_lvl[HD_FLOW_DBG_AENC]) >= HD_AENC_DBG_MSG) { hdal_flow_log_p(fmt, ##args);  }}
#define HD_AENC_FLOW_IND(fmt, args...) { if (hd_flow_dbg_lvl[HD_FLOW_DBG_AENC] && (*hd_flow_dbg_lvl[HD_FLOW_DBG_AENC]) >= HD_AENC_DBG_IND) { hdal_flow_log_p(fmt, ##args);  }}
#define HD_AENC_FLOW_FUNC(fmt, args...) { if (hd_flow_dbg_lvl[HD_FLOW_DBG_AENC] && (*hd_flow_dbg_lvl[HD_FLOW_DBG_AENC]) >= HD_AENC_DBG_FUNC) { hdal_flow_log_p(fmt, ##args);  }}

/*-----------------------------------------------------------------------------*/
/* Local Global Variables                                                      */
/*-----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------*/
/* Interface Functions                                                         */
/*-----------------------------------------------------------------------------*/
INT _vendor_audioenc_param_cvt_name(VENDOR_AUDIOENC_ITEM  id, CHAR *p_ret_string, INT max_str_len)
{
	switch (id) {
		case VENDOR_AUDIOENC_ITEM_BS_RESERVED_SIZE: snprintf(p_ret_string, max_str_len, "BS_RESERVED_SIZE");       break;
		case VENDOR_AUDIOENC_ITEM_AAC_VER:          snprintf(p_ret_string, max_str_len, "AAC_VER");     break;
		case VENDOR_AUDIOENC_ITEM_DATA_QUEUE_NUM:   snprintf(p_ret_string, max_str_len, "DATA_QUEUE_NUM");     break;
		default:
			snprintf(p_ret_string, max_str_len, "error");
			printf("unknown param_id(%d)\r\n", id);
			return (-1);
	}
	return 0;
}

HD_RESULT vendor_audioenc_set(HD_PATH_ID id, VENDOR_AUDIOENC_ITEM item, VOID *p_param)
{
	INT32 ret;
	int isf_fd;

	HD_DAL self_id = HD_GET_DEV(id);
	HD_IO out_id = HD_GET_OUT(id);
	ISF_FLOW_IOCTL_PARAM_ITEM cmd = {0};
	int r;

	if (p_param == NULL) {
		return HD_ERR_NULL_PTR;
	}

	{
		CHAR  param_name[20];
		_vendor_audioenc_param_cvt_name(item, param_name, 20);

		HD_AENC_FLOW_MSG("vendor_audioenc_set(%s):\n", param_name);
		HD_AENC_FLOW_MSG("    path_id(0x%x) ", id);
	}

	switch (item) {
		case VENDOR_AUDIOENC_ITEM_BS_RESERVED_SIZE: {
			VENDOR_AUDIOENC_BS_RESERVED_SIZE_CFG* p_user = (VENDOR_AUDIOENC_BS_RESERVED_SIZE_CFG*)p_param;
			UINT32 reserved_size = 0;

			isf_fd = _hd_common_get_fd();

			if (isf_fd <= 0) {
				return HD_ERR_UNINIT;
			}

			if(!(out_id >= HD_OUT_BASE && out_id <= HD_OUT_MAX)) {
				return HD_ERR_IO;
			}

			_HD_CONVERT_OUT_ID(out_id, ret);
			if(ret != HD_OK) {
				return ret;
			}

			_HD_CONVERT_SELF_ID(self_id, ret);
			if (ret != HD_OK) {
				return ret;
			}

			ret = HD_OK;

			HD_AENC_FLOW_MSG("reserved_size(%u)\n", reserved_size);

			reserved_size = p_user->reserved_size;

			cmd.dest = ISF_PORT(self_id, out_id);
			cmd.param = AUDENC_PARAM_BS_RESERVED_SIZE;
			cmd.value = reserved_size;
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

		#if (AUDENC_LIB_AAC == ENABLE)
		case VENDOR_AUDIOENC_ITEM_AAC_VER: {
			UINT32* p_user = (UINT32*)p_param;
			UINT32 aac_ver = 0;

			isf_fd = _hd_common_get_fd();

			if (isf_fd <= 0) {
				return HD_ERR_UNINIT;
			}

			if(!(out_id >= HD_OUT_BASE && out_id <= HD_OUT_MAX)) {
				return HD_ERR_IO;
			}

			_HD_CONVERT_OUT_ID(out_id, ret);
			if(ret != HD_OK) {
				return ret;
			}

			_HD_CONVERT_SELF_ID(self_id, ret);
			if (ret != HD_OK) {
				return ret;
			}

			ret = HD_OK;

			HD_AENC_FLOW_MSG("aac_ver(%u)\n", aac_ver);

			aac_ver = *p_user;

			cmd.dest = ISF_PORT(self_id, out_id);
			cmd.param = AUDENC_PARAM_AAC_VER;
			cmd.value = aac_ver;
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

		case VENDOR_AUDIOENC_ITEM_DATA_QUEUE_NUM: {
			UINT32 queue_num = *(UINT32*)p_param;

			isf_fd = _hd_common_get_fd();

			if (isf_fd <= 0) {
				return HD_ERR_UNINIT;
			}

			if(!(out_id >= HD_OUT_BASE && out_id <= HD_OUT_MAX)) {
				return HD_ERR_IO;
			}

			_HD_CONVERT_OUT_ID(out_id, ret);
			if(ret != HD_OK) {
				return ret;
			}

			_HD_CONVERT_SELF_ID(self_id, ret);
			if (ret != HD_OK) {
				return ret;
			}

			ret = HD_OK;

			HD_AENC_FLOW_MSG("queue_num(%u)\n", queue_num);

			cmd.dest = ISF_PORT(self_id, out_id);
			cmd.param = AUDENC_PARAM_DATA_QUEUE_NUM;
			cmd.value = queue_num;
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
			printf("vendor_audioenc_set not support item %d \n", (int)item);
			break;
	}

	return ret;
}

HD_RESULT vendor_audioenc_get(HD_PATH_ID id, VENDOR_AUDIOENC_ITEM item, VOID *p_param)
{
	return HD_OK;
}

