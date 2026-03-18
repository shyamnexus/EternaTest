/**
	@brief Source file of vendor media audioenc.\n

	@file vendor_audiodec.c

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
#include "vendor_audiodec.h"
#include "kflow_audiodec/isf_auddec.h"
#include "kflow_videodec/media_def.h"
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
#define HD_DEV_BASE HD_DAL_AUDIODEC_BASE
#define HD_DEV_MAX  HD_DAL_AUDIODEC_MAX
#define DEV_BASE        ISF_UNIT_AUDDEC
#define DEV_COUNT       ISF_MAX_AUDDEC
#define OUT_BASE        ISF_OUT_BASE
#define OUT_COUNT       2
#define IN_BASE         ISF_IN_BASE
#define IN_COUNT        2

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

#define _HD_CONVERT_IN_ID(in_id, rv) \
	do { \
		(rv) = HD_ERR_IO; \
		if((in_id) == 0) { \
			(rv) = HD_ERR_UNIQUE; \
		} else if((in_id) >= HD_IN_BASE && (in_id) <= HD_IN_MAX) { \
			HD_IO id = (in_id) - HD_IN_BASE; \
			if(id < IN_COUNT) { \
				(in_id) = IN_BASE + id; \
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
INT _vendor_audiodec_param_cvt_name(VENDOR_AUDIODEC_ITEM  id, CHAR *p_ret_string, INT max_str_len)
{
	switch (id) {
		case VENDOR_AUDIODEC_ITEM_CODEC_HEADER:     snprintf(p_ret_string, max_str_len, "CODEC_HEADER");       break;
		case VENDOR_AUDIODEC_ITEM_RAW_BLOCK_SIZE:   snprintf(p_ret_string, max_str_len, "RAW_BLOCK_SIZE");     break;
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
#define HD_ADEC_DBG_FATAL     0
#define HD_ADEC_DBG_ERR       1
#define HD_ADEC_DBG_WRN       2
#define HD_ADEC_DBG_MSG       3
#define HD_ADEC_DBG_IND       4
#define HD_ADEC_DBG_FUNC      5

#define HD_ADEC_FLOW_ERR(fmt, args...) { if (hd_flow_dbg_lvl[HD_FLOW_DBG_ADEC] && (*hd_flow_dbg_lvl[HD_FLOW_DBG_ADEC]) >= HD_ADEC_DBG_ERR) { hdal_flow_log_p(fmt, ##args);  }}
#define HD_ADEC_FLOW_WRN(fmt, args...) { if (hd_flow_dbg_lvl[HD_FLOW_DBG_ADEC] && (*hd_flow_dbg_lvl[HD_FLOW_DBG_ADEC]) >= HD_ADEC_DBG_WRN) { hdal_flow_log_p(fmt, ##args);  }}
#define HD_ADEC_FLOW_MSG(fmt, args...) { if (hd_flow_dbg_lvl[HD_FLOW_DBG_ADEC] && (*hd_flow_dbg_lvl[HD_FLOW_DBG_ADEC]) >= HD_ADEC_DBG_MSG) { hdal_flow_log_p(fmt, ##args);  }}
#define HD_ADEC_FLOW_IND(fmt, args...) { if (hd_flow_dbg_lvl[HD_FLOW_DBG_ADEC] && (*hd_flow_dbg_lvl[HD_FLOW_DBG_ADEC]) >= HD_ADEC_DBG_IND) { hdal_flow_log_p(fmt, ##args);  }}
#define HD_ADEC_FLOW_FUNC(fmt, args...) { if (hd_flow_dbg_lvl[HD_FLOW_DBG_ADEC] && (*hd_flow_dbg_lvl[HD_FLOW_DBG_ADEC]) >= HD_ADEC_DBG_FUNC) { hdal_flow_log_p(fmt, ##args);  }}

/*-----------------------------------------------------------------------------*/
/* Local Global Variables                                                      */
/*-----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------*/
/* Interface Functions                                                         */
/*-----------------------------------------------------------------------------*/
HD_RESULT vendor_audiodec_set(HD_PATH_ID id, VENDOR_AUDIODEC_ITEM item, VOID *p_param)
{
	HD_RESULT ret;
	int isf_fd;

	HD_DAL self_id = HD_GET_DEV(id);
	//HD_IO out_id = HD_GET_OUT(id);
	HD_IO in_id = HD_GET_IN(id);
	ISF_FLOW_IOCTL_PARAM_ITEM cmd = {0};
	int r;

	{
		CHAR  param_name[20];
		_vendor_audiodec_param_cvt_name(item, param_name, 20);

		HD_ADEC_FLOW_MSG("vendor_audiodec_set(%s):\n", param_name);
		HD_ADEC_FLOW_MSG("    path_id(0x%x) ", id);
	}

	if (p_param == NULL) {
		return HD_ERR_NULL_PTR;
	}

	switch (item) {
		#if (AUDDEC_LIB_AAC == ENABLE)
		case VENDOR_AUDIODEC_ITEM_CODEC_HEADER: {
			UINT32 codec_header = *(UINT32*)p_param;

			isf_fd = _hd_common_get_fd();

			if (isf_fd <= 0) {
				return HD_ERR_UNINIT;
			}

			if(!(in_id >= HD_IN_BASE && in_id <= HD_IN_MAX)) {
				return HD_ERR_IO;
			}

			_HD_CONVERT_IN_ID(in_id, ret);
			if(ret != HD_OK) {
				return ret;
			}

			_HD_CONVERT_SELF_ID(self_id, ret);
			if (ret != HD_OK) {
				return ret;
			}

			HD_ADEC_FLOW_MSG("codec_header(%u)\n", codec_header);

			ret = HD_OK;

			cmd.dest = ISF_PORT(self_id, in_id);
			cmd.param = AUDDEC_PARAM_ADTS_EN;
			cmd.value = codec_header;
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

		case VENDOR_AUDIODEC_ITEM_RAW_BLOCK_SIZE: {
			UINT32 block_size = *(UINT32*)p_param;

			isf_fd = _hd_common_get_fd();

			if (isf_fd <= 0) {
				return HD_ERR_UNINIT;
			}

			if(!(in_id >= HD_IN_BASE && in_id <= HD_IN_MAX)) {
				return HD_ERR_IO;
			}

			_HD_CONVERT_IN_ID(in_id, ret);
			if(ret != HD_OK) {
				return ret;
			}

			_HD_CONVERT_SELF_ID(self_id, ret);
			if (ret != HD_OK) {
				return ret;
			}

			HD_ADEC_FLOW_MSG("block_size(%u)\n", block_size);

			ret = HD_OK;

			cmd.dest = ISF_PORT(self_id, in_id);
			cmd.param = AUDDEC_PARAM_RAW_BLOCK_SIZE;
			cmd.value = block_size;
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

HD_RESULT vendor_audiodec_get(HD_PATH_ID id, VENDOR_AUDIODEC_ITEM item, VOID *p_param)
{
	return HD_OK;
}

