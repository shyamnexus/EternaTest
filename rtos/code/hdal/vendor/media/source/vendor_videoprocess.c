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
#define HD_MODULE_NAME VENDOR_VIDEOPROCESS

#include "kflow_common/isf_flow_def.h"
#include "kflow_common/isf_flow_ioctl.h"
#include "kflow_videoprocess/isf_vdoprc.h"
#include "vendor_videoprocess.h"
#include <stdarg.h>
#include "hd_logger_p.h"

/*-----------------------------------------------------------------------------*/
/* Debug Variables & Functions                                                 */
/*-----------------------------------------------------------------------------*/
#define HD_VPRC_DBG_FATAL     0
#define HD_VPRC_DBG_ERR       1
#define HD_VPRC_DBG_WRN       2
#define HD_VPRC_DBG_MSG       3
#define HD_VPRC_DBG_IND       4
#define HD_VPRC_DBG_FUNC      5

#define HD_VPRC_FLOW_ERR(fmt, args...) { if (hd_flow_dbg_lvl[HD_FLOW_DBG_VPRC] && (*hd_flow_dbg_lvl[HD_FLOW_DBG_VPRC]) >= HD_VPRC_DBG_ERR) { hdal_flow_log_p(fmt, ##args);  }}
#define HD_VPRC_FLOW_WRN(fmt, args...) { if (hd_flow_dbg_lvl[HD_FLOW_DBG_VPRC] && (*hd_flow_dbg_lvl[HD_FLOW_DBG_VPRC]) >= HD_VPRC_DBG_WRN) { hdal_flow_log_p(fmt, ##args);  }}
#define HD_VPRC_FLOW_MSG(fmt, args...) { if (hd_flow_dbg_lvl[HD_FLOW_DBG_VPRC] && (*hd_flow_dbg_lvl[HD_FLOW_DBG_VPRC]) >= HD_VPRC_DBG_MSG) { hdal_flow_log_p(fmt, ##args);  }}
#define HD_VPRC_FLOW_IND(fmt, args...) { if (hd_flow_dbg_lvl[HD_FLOW_DBG_VPRC] && (*hd_flow_dbg_lvl[HD_FLOW_DBG_VPRC]) >= HD_VPRC_DBG_IND) { hdal_flow_log_p(fmt, ##args);  }}
#define HD_VPRC_FLOW_FUNC(fmt, args...) { if (hd_flow_dbg_lvl[HD_FLOW_DBG_VPRC] && (*hd_flow_dbg_lvl[HD_FLOW_DBG_VPRC]) >= HD_VPRC_DBG_FUNC) { hdal_flow_log_p(fmt, ##args);  }}


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
#define DBG_ERR(fmt, args...) 	printf("%s: " fmt, __func__, ##args)
#define DBG_DUMP				printf
#define CHKPNT                  printf("\033[37mCHK: %d, %s\033[0m\r\n", __LINE__, __func__)


/*-----------------------------------------------------------------------------*/
/* Local Constant Definitions                                                  */
/*-----------------------------------------------------------------------------*/
#define DEV_BASE		ISF_UNIT_VDOPRC
#define DEV_COUNT		ISF_MAX_VDOPRC
#define IN_BASE		    ISF_IN_BASE
#define IN_COUNT		1
#define OUT_BASE		ISF_OUT_BASE
#define OUT_COUNT 	    16
#define OUT_PHY_COUNT    5


#define HD_DEV_BASE	HD_DAL_VIDEOPROC_BASE
#define HD_DEV_MAX	HD_DAL_VIDEOPROC_MAX

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

#define _HD_CONVERT_IN_ID(in_id, rv) \
	do { \
		(rv) = HD_ERR_IO; \
		if((in_id) == 0) { \
			(rv) = HD_ERR_UNIQUE; \
		} else if((in_id) >= HD_IN_BASE && (in_id) <= HD_IN_MAX) { \
			UINT32 id = (in_id) - HD_IN_BASE; \
			if(id < IN_COUNT) { \
				(in_id) = IN_BASE + id; \
				(rv) = HD_OK; \
			} \
		} \
	} while(0)

#define _HD_CONVERT_OSG_ID(osg_id) \
	do { \
		if((osg_id) == 0) { \
			(osg_id) = 0; \
		} if((osg_id) >= HD_STAMP_BASE && (osg_id) <= HD_MASK_EX_MAX) { \
			if((osg_id) >= HD_STAMP_BASE && (osg_id) <= HD_STAMP_MAX) { \
				HD_IO id = (osg_id) - HD_STAMP_BASE; \
				(osg_id) = 0x00010000 | id; \
			} else if((osg_id) >= HD_STAMP_EX_BASE && (osg_id) <= HD_STAMP_EX_MAX) { \
				HD_IO id = (osg_id) - HD_STAMP_EX_BASE; \
				(osg_id) = 0x00020000 | id; \
			} else if((osg_id) >= HD_MASK_BASE && (osg_id) <= HD_MASK_MAX) { \
				HD_IO id = (osg_id) - HD_MASK_BASE; \
				(osg_id) = 0x00040000 | id; \
			} else if((osg_id) >= HD_MASK_EX_BASE && (osg_id) <= HD_MASK_EX_MAX) { \
				HD_IO id = (osg_id) - HD_MASK_EX_BASE; \
				(osg_id) = 0x00080000 | id; \
			} else { \
				(osg_id) = 0; \
			} \
		} else { \
			(osg_id) = 0; \
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
extern HD_RESULT _hd_osg_set(UINT32 unit_id, UINT32 port_id, UINT32 osg_id, UINT32 param_id, VOID* p_param);
extern HD_RESULT _hd_videoproc_kflow_param_set_by_vendor(HD_DAL self_id, HD_IO in_id, HD_IO out_id, UINT32 param_id, ULONG param);
extern HD_RESULT _hd_videoproc_kflow_param_set_need_update(HD_DAL self_id, HD_IO in_id, HD_IO out_id, UINT32 param_id, ULONG param);

/*-----------------------------------------------------------------------------*/
/* Local Function Prototype                                                    */
/*-----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------*/
/* Debug Variables & Functions                                                 */
/*-----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------*/
/* Local Global Variables                                                      */
/*-----------------------------------------------------------------------------*/

static INT _vendor_videoproc_param_cvt_name(VENDOR_VIDEOPROC_PARAM_ID  id, CHAR *p_ret_string, INT max_str_len)
{
	switch (id) {
        case VENDOR_VIDEOPROC_PARAM_HEIGHT_ALIGN:           snprintf(p_ret_string, max_str_len, "HEIGHT_ALIGN");           break;
        case VENDOR_VIDEOPROC_PARAM_IN_DEPTH:	  	        snprintf(p_ret_string, max_str_len, "IN_DEPT");           break;
        case VENDOR_VIDEOPROC_PARAM_DMA_ABORT:		        snprintf(p_ret_string, max_str_len, "DMA_ABORT");           break;
        case VENDOR_VIDEOPROC_PARAM_SLICE_MODE:		        snprintf(p_ret_string, max_str_len, "SLICE_MODE");           break;
        case VENDOR_VIDEOPROC_PARAM_USER_CROP_TRIG:         snprintf(p_ret_string, max_str_len, "USER_CROP_TRIG");           break;
        case VENDOR_VIDEOPROC_CFG_DIS_SCALERATIO:	        snprintf(p_ret_string, max_str_len, "DIS_SCALERATIO");           break;
        case VENDOR_VIDEOPROC_CFG_DIS_SUBSAMPLE:		    snprintf(p_ret_string, max_str_len, "DIS_SUBSAMPLE");           break;
        case VENDOR_VIDEOPROC_PARAM_STRIP:		            snprintf(p_ret_string, max_str_len, "STRIP");           break;
        case VENDOR_VIDEOPROC_PARAM_OUT_ONEBUF_MAX:         snprintf(p_ret_string, max_str_len, "OUT_ONEBUF_MAX");           break;
        case VENDOR_VIDEOPROC_PARAM_LINEOFFSET_ALIGN:       snprintf(p_ret_string, max_str_len, "LINEOFFSET_ALIGN");           break;
        case VENDOR_VIDEOPROC_PARAM_VSP_CFG:      	        snprintf(p_ret_string, max_str_len, "VSP_CFG");           break;
        case VENDOR_VIDEOPROC_PARAM_DRE_CFG:                snprintf(p_ret_string, max_str_len, "DRE_CFG");           break;
        case VENDOR_VIDEOPROC_PARAM_EIS_FUNC:               snprintf(p_ret_string, max_str_len, "EIS_FUNC");           break;
        case VENDOR_VIDEOPROC_PARAM_VPE_CLEAR_WIN:          snprintf(p_ret_string, max_str_len, "VPE_CLEAR_WIN");           break;
        case VENDOR_VIDEOPROC_PARAM_ISP_AI_START:           snprintf(p_ret_string, max_str_len, "ISP_AI_START");           break;
        case VENDOR_VIDEOPROC_PARAM_ISP_AI_STOP:            snprintf(p_ret_string, max_str_len, "ISP_AI_STOP");           break;
		case VENDOR_VIDEOPROC_PARAM_AI_CB:                  snprintf(p_ret_string, max_str_len, "AI_CB");           break;
		case VENDOR_VIDEOPROC_PARAM_ISP_CB:                 snprintf(p_ret_string, max_str_len, "ISP_CB");           break;
		case VENDOR_VIDEOPROC_PARAM_VPE_MASK:               snprintf(p_ret_string, max_str_len, "VPE_MASK");           break;
		case VENDOR_VIDEOPROC_PARAM_VPE_MOSAIC:             snprintf(p_ret_string, max_str_len, "VPE_MOSAIC");           break;
		case VENDOR_VIDEOPROC_PARAM_FISHEYE_MASK:           snprintf(p_ret_string, max_str_len, "FISHEYE_MASK");           break;
		case VENDOR_VIDEOPROC_PARAM_VPROC_POLYGON_MASK:     snprintf(p_ret_string, max_str_len, "VPROC_POLYGON_MASK");           break;
		case VENDOR_VIDEOPROC_PARAM_VPE_POLYGON_MASK:       snprintf(p_ret_string, max_str_len, "VPE_POLYGON_MASK");           break;
		case VENDOR_VIDEOPROC_PARAM_VPE_SPLIT_INFO:         snprintf(p_ret_string, max_str_len, "VPE_SPLIT_INFO");           break;
		case VENDOR_VIDEOPROC_PARAM_THERMAL_INFO:           snprintf(p_ret_string, max_str_len, "THERMAL_INFO");           break;
		case VENDOR_VIDEOPROC_PARAM_LCA_FUNC:               snprintf(p_ret_string, max_str_len, "LCA_FUNC");           break;
		case VENDOR_VIDEOPROC_PARAM_ISP_AI_EFFECT:          snprintf(p_ret_string, max_str_len, "ISP_AI_EFFECT");           break;
		case VENDOR_VIDEOPROC_PARAM_ISP_AI_POSTPROC:        snprintf(p_ret_string, max_str_len, "ISP_AI_POSTPROC");           break;
		default:
			snprintf(p_ret_string, max_str_len, "error");
			return (-1);
	}
	return 0;
}

ULONG VDOPRC_PARAM_AI_CB_param;
/*-----------------------------------------------------------------------------*/
/* Interface Functions                                                         */
/*-----------------------------------------------------------------------------*/
HD_RESULT _vendor_videoproc_set(UINT32 path_id, VENDOR_VIDEOPROC_PARAM_ID param_id, VOID *p_param)
{
	HD_DAL self_id = HD_GET_DEV(path_id);
	HD_IO    in_id = HD_GET_IN(path_id);
	HD_IO   out_id = HD_GET_OUT(path_id);
	HD_IO  ctrl_id = HD_GET_CTRL(path_id);
	HD_RESULT rv = HD_ERR_NG;
	int r = 0;
	ISF_FLOW_IOCTL_PARAM_ITEM cmd = {0};
	int isf_fd = _hd_common_get_fd();

    {
		CHAR  param_name[20];
		_vendor_videoproc_param_cvt_name(param_id, param_name, 20);

		HD_VPRC_FLOW_MSG("vendor_videoproc_set(%s):\n", param_name);
		HD_VPRC_FLOW_MSG("    path_id(0x%x) ", path_id);
	}

	if (isf_fd <= 0) {
		return HD_ERR_UNINIT;
	}
	if (p_param == NULL) {
		return HD_ERR_NULL_PTR;
	}
	_HD_CONVERT_SELF_ID(self_id, rv);   if (rv != HD_OK) { return rv; }

	{
		HD_IO osg_in_id = in_id, osg_out_id = out_id;
		UINT32 unit_id = 0, port_id = 0, osg_id = 0;
		_HD_CONVERT_OSG_ID(osg_in_id);
		_HD_CONVERT_OSG_ID(osg_out_id);


		if(osg_in_id) {
			_HD_CONVERT_OUT_ID(out_id, rv);	if(rv != HD_OK) {	return rv;}
			unit_id = self_id;
			port_id = out_id;
			osg_id = osg_in_id;
		} else if(osg_out_id) {
			_HD_CONVERT_IN_ID(in_id, rv);	if(rv != HD_OK) {	return rv;}
			unit_id = self_id;
			port_id = in_id;
			osg_id = osg_out_id;
		}

		if(param_id == VENDOR_VIDEOPROC_PARAM_VPE_MASK){
			extern int _hd_osg_set_vpe_mask(UINT32 unit_id, UINT32 port_id, UINT32 osg_id, VENDOR_VIDEOPROC_VPE_MASK* p_attr);
			return _hd_osg_set_vpe_mask(unit_id, port_id, osg_id, (VENDOR_VIDEOPROC_VPE_MASK*)p_param);
		}else if(param_id == VENDOR_VIDEOPROC_PARAM_VPE_MOSAIC){
			extern int _hd_osg_set_vpe_mosaic(UINT32 unit_id, UINT32 port_id, UINT32 osg_id, VENDOR_VIDEOPROC_VPE_MOSAIC* p_attr);
			return _hd_osg_set_vpe_mosaic(unit_id, port_id, osg_id, (VENDOR_VIDEOPROC_VPE_MOSAIC*)p_param);
		}else if(param_id == VENDOR_VIDEOPROC_PARAM_FISHEYE_MASK){
			extern int _hd_osg_set_ime_fisheye(UINT32 unit_id, UINT32 port_id, UINT32 osg_id, VENDOR_VIDEOPROC_FISHEYE_MASK* p_attr);
			unit_id = self_id;
			return _hd_osg_set_ime_fisheye(unit_id, port_id, osg_id, (VENDOR_VIDEOPROC_FISHEYE_MASK*)p_param);
		}else if(param_id == VENDOR_VIDEOPROC_PARAM_VPROC_POLYGON_MASK){
			extern int _hd_osg_set_vproc_polygon_mask(UINT32 unit_id, UINT32 port_id, UINT32 osg_id, VENDOR_VIDEOPROC_VPE_MASK* p_attr);
			return _hd_osg_set_vproc_polygon_mask(unit_id, port_id, osg_id, (VENDOR_VIDEOPROC_VPE_MASK*)p_param);
		}else if(param_id == VENDOR_VIDEOPROC_PARAM_VPE_POLYGON_MASK){
			extern int _hd_osg_set_vpe_polygon_mask(UINT32 unit_id, UINT32 port_id, UINT32 osg_id, VENDOR_VIDEOPROC_VPE_MASK* p_attr);
			return _hd_osg_set_vpe_polygon_mask(unit_id, port_id, osg_id, (VENDOR_VIDEOPROC_VPE_MASK*)p_param);
		}
	}

	rv = HD_OK;
	if(ctrl_id == HD_CTRL) {
		switch(param_id) {
			case VENDOR_VIDEOPROC_PARAM_DMA_ABORT: {
				UINT32 *p_user = (UINT32 *)p_param;
				HD_VPRC_FLOW_MSG("abort(%d) \n", *p_user);

				cmd.dest = ISF_PORT(self_id, ISF_CTRL);
				cmd.param = VDOPRC_PARAM_DMA_ABORT;
				cmd.value = *p_user;
				cmd.size = 0;
				r = ISF_IOCTL(isf_fd, ISF_FLOW_CMD_SET_PARAM, &cmd);
				goto _VD_VPRC1;
			}
			break;
			case VENDOR_VIDEOPROC_PARAM_IN_DEPTH: {
				UINT32 *p_user = (UINT32 *)p_param;
				HD_VPRC_FLOW_MSG("depth(%d) \n", *p_user);

				cmd.dest = ISF_PORT(self_id, ISF_IN(0));
				cmd.param = VDOPRC_PARAM_IN_DEPTH;
				cmd.value = *p_user;
				cmd.size = 0;
				r = ISF_IOCTL(isf_fd, ISF_FLOW_CMD_SET_PARAM, &cmd);
				goto _VD_VPRC1;
			}
			break;
			case VENDOR_VIDEOPROC_PARAM_SLICE_MODE: {
				VENDOR_VIDEOPROC_SLICE_MODE *p_user = (VENDOR_VIDEOPROC_SLICE_MODE *)p_param;
				HD_VPRC_FLOW_MSG("cnt(%d) \n", p_user->cnt);

				if (sizeof(VENDOR_VIDEOPROC_SLICE_MODE) != sizeof(VDOPRC_SLICE_MODE_INFO)) {
					printf("struct size mismatch\r\n");
					rv = HD_ERR_PARAM;
					return rv;
				}
				cmd.dest = ISF_PORT(self_id, ISF_CTRL);
				cmd.param = VDOPRC_PARAM_SLICE_MODE;
				cmd.value = (ULONG)p_user;
				cmd.size = sizeof(VDOPRC_SLICE_MODE_INFO);
				r = ISF_IOCTL(isf_fd, ISF_FLOW_CMD_SET_PARAM, &cmd);
				goto _VD_VPRC1;
			}
			break;
			case VENDOR_VIDEOPROC_CFG_DIS_SCALERATIO: {
				UINT32 *p_user = (UINT32 *)p_param;
				HD_VPRC_FLOW_MSG("ratio(%d) \n", *p_user);

				cmd.dest = ISF_PORT(self_id, ISF_IN(0));
				cmd.param = VDOPRC_PARAM_DIS_SCALERATIO;
				cmd.value = *p_user;
				cmd.size = 0;
				r = ISF_IOCTL(isf_fd, ISF_FLOW_CMD_SET_PARAM, &cmd);
				goto _VD_VPRC1;
			}
			break;
			case VENDOR_VIDEOPROC_CFG_DIS_SUBSAMPLE: {
				UINT32 *p_user = (UINT32 *)p_param;
				HD_VPRC_FLOW_MSG("subsample(%d) \n", *p_user);

				cmd.dest = ISF_PORT(self_id, ISF_IN(0));
				cmd.param = VDOPRC_PARAM_DIS_SUBSAMPLE;
				cmd.value = *p_user;
				cmd.size = 0;
				r = ISF_IOCTL(isf_fd, ISF_FLOW_CMD_SET_PARAM, &cmd);
				goto _VD_VPRC1;
			}
			break;
			case VENDOR_VIDEOPROC_PARAM_STRIP: {
				UINT32 *p_user = (UINT32 *)p_param;
				HD_VPRC_FLOW_MSG("strip(%d) \n", *p_user);

				cmd.dest = ISF_PORT(self_id, ISF_CTRL);
				cmd.param = VDOPRC_PARAM_STRIP;
				cmd.value = *p_user;
				cmd.size = 0;

				r = ISF_IOCTL(isf_fd, ISF_FLOW_CMD_SET_PARAM, &cmd);
				goto _VD_VPRC1;
			}
			break;
			case VENDOR_VIDEOPROC_PARAM_VSP_CFG: {
				VENDOR_VIDEOPROC_VSP_CFG* p_user = (VENDOR_VIDEOPROC_VSP_CFG*)p_param;
				HD_VPRC_FLOW_MSG("frm_num(%d) mode(%d) level(%d) size(%d,%d)\n", p_user->blend_frm_num,p_user->dre_proc_mode,p_user->dre_quality_level,p_user->vpe_out_size.h,p_user->vpe_out_size.h);

				cmd.dest = ISF_PORT(self_id, ISF_CTRL);
				cmd.param = VDOPRC_PARAM_VSP_CFG;
				cmd.value = (ULONG)p_user;
				cmd.size = sizeof(VENDOR_VIDEOPROC_VSP_CFG);
				r = ISF_IOCTL(isf_fd, ISF_FLOW_CMD_SET_PARAM, &cmd);
				goto _VD_VPRC1;
			}
			break;
			case VENDOR_VIDEOPROC_PARAM_EIS_FUNC: {
				UINT32 *p_user = (UINT32 *)p_param;
				HD_VPRC_FLOW_MSG("func(%d) \n", *p_user);

				cmd.dest = ISF_PORT(self_id, ISF_CTRL);
				cmd.param = VDOPRC_PARAM_EIS_FUNC;
				cmd.value = *p_user;
				cmd.size = 0;

				r = ISF_IOCTL(isf_fd, ISF_FLOW_CMD_SET_PARAM, &cmd);
				goto _VD_VPRC1;
			}
			break;
			case VENDOR_VIDEOPROC_PARAM_AI_CB: {
				ULONG *p_user = (ULONG *)p_param;
				HD_VPRC_FLOW_MSG("ai_cb(0x%lx) \n", p_param);

				_hd_videoproc_kflow_param_set_by_vendor(self_id, in_id, out_id, VDOPRC_PARAM_AI_CB, (ULONG)p_param);
				_hd_videoproc_kflow_param_set_need_update(self_id, in_id, out_id, VDOPRC_PARAM_AI_CB, TRUE);

				VDOPRC_PARAM_AI_CB_param = *(ULONG *)p_param;

				cmd.dest = ISF_PORT(self_id, ISF_CTRL);
				cmd.param = VDOPRC_PARAM_AI_CB;
				cmd.value = (ULONG)p_user;
				cmd.size = sizeof(ULONG);

				r = ISF_IOCTL(isf_fd, ISF_FLOW_CMD_SET_PARAM, &cmd);
				goto _VD_VPRC1;
			}
			break;
			case VENDOR_VIDEOPROC_PARAM_ISP_AI_START:
			case VENDOR_VIDEOPROC_PARAM_ISP_AI_STOP: {
				VENDOR_VIDEOPROC_ISP_AI* p_user = (VENDOR_VIDEOPROC_ISP_AI*)p_param;
                if(!p_user) {
                    r = HD_ERR_NULL_PTR;
                    goto _VD_VPRC1;
                } else {
				    HD_VPRC_FLOW_MSG("(%d,%d) \n", p_user->path_id,p_user->proc_id);
                }
				cmd.dest = ISF_PORT(self_id, ISF_CTRL);

                if(param_id ==VENDOR_VIDEOPROC_PARAM_ISP_AI_START) {
				    cmd.param = VDOPRC_PARAM_ISP_AI_START;
                } else {
				    cmd.param = VDOPRC_PARAM_ISP_AI_STOP;
                }
				cmd.value = (ULONG)p_user;
				cmd.size = sizeof(VENDOR_VIDEOPROC_ISP_AI);

				r = ISF_IOCTL(isf_fd, ISF_FLOW_CMD_SET_PARAM, &cmd);
				goto _VD_VPRC1;
			}
			break;
			case VENDOR_VIDEOPROC_PARAM_THERMAL_INFO: {
				VENDOR_VIDEOPROC_THERMAL_INFO *p_user = (VENDOR_VIDEOPROC_THERMAL_INFO *)p_param;
				HD_VPRC_FLOW_MSG("strip(%d) \n", *p_user);

				cmd.dest = ISF_PORT(self_id, ISF_CTRL);
				cmd.param = VDOPRC_PARAM_THERMAL_INFO;
				cmd.value = (ULONG)p_user->enable;
				cmd.size = 0;

				r = ISF_IOCTL(isf_fd, ISF_FLOW_CMD_SET_PARAM, &cmd);
				goto _VD_VPRC1;
			}
			break;
			case VENDOR_VIDEOPROC_PARAM_LCA_FUNC: {
				VENDOR_VIDEOPROC_LCA_FUNC *p_user = (VENDOR_VIDEOPROC_LCA_FUNC *)p_param;
				HD_VPRC_FLOW_MSG("lca(%d) \n", p_user->enable);

				cmd.dest = ISF_PORT(self_id, ISF_CTRL);
				cmd.param = VDOPRC_PARAM_LCA_FUNC;
				cmd.value = (ULONG)p_user->enable;
				cmd.size = 0;

				r = ISF_IOCTL(isf_fd, ISF_FLOW_CMD_SET_PARAM, &cmd);
				goto _VD_VPRC1;
			}
			break;
			case VENDOR_VIDEOPROC_PARAM_ISP_AI_EFFECT: {
				VENDOR_VIDEOPROC_ISP_AI_EFFECT *p_user = (VENDOR_VIDEOPROC_ISP_AI_EFFECT *)p_param;
				if (!p_user) {
					r = HD_ERR_NULL_PTR;
					goto _VD_VPRC1;
				} else {
					INT idx;
					HD_VPRC_FLOW_MSG("(%d:", p_user->path_id);
					for (idx = 0; idx < VENDOR_VIDEOPROC_ISP_AI_EFFECT_MAX; idx++) {
						HD_VPRC_FLOW_MSG("-%d", p_user->proc_id[idx]);
					}
					HD_VPRC_FLOW_MSG(")\n");
				}

				_hd_videoproc_kflow_param_set_by_vendor(self_id, in_id, out_id, VDOPRC_PARAM_ISP_AI_EFFECT, (ULONG)p_param);
				_hd_videoproc_kflow_param_set_need_update(self_id, in_id, out_id, VDOPRC_PARAM_ISP_AI_EFFECT, TRUE);

				cmd.dest = ISF_PORT(self_id, ISF_CTRL);
				cmd.param = VDOPRC_PARAM_ISP_AI_EFFECT;
				cmd.value = (ULONG)p_user;
				cmd.size = sizeof(VENDOR_VIDEOPROC_ISP_AI_EFFECT);

				r = ISF_IOCTL(isf_fd, ISF_FLOW_CMD_SET_PARAM, &cmd);
				goto _VD_VPRC1;
			}
			break;
			case VENDOR_VIDEOPROC_PARAM_ISP_AI_POSTPROC: {
				VENDOR_VIDEOPROC_ISP_AI_POSTPROC* p_user = (VENDOR_VIDEOPROC_ISP_AI_POSTPROC*)p_param;
				HD_VPRC_FLOW_MSG("isp_ai_postproc(%d) \n", p_user->enable);

				cmd.dest = ISF_PORT(self_id, ISF_CTRL);
				cmd.param = VDOPRC_PARAM_ISP_AI_POSTPROC;
				cmd.value = (ULONG)p_user;
				cmd.size = sizeof(VENDOR_VIDEOPROC_ISP_AI_POSTPROC);

				r = ISF_IOCTL(isf_fd, ISF_FLOW_CMD_SET_PARAM, &cmd);
				goto _VD_VPRC1;
			}
			break;
			default:
				rv = HD_ERR_PARAM;
				printf("vendor_videoproc_set not support item %d \n", param_id);
				return rv;
			break;
		}
	} else {
		switch (param_id) {
			case VENDOR_VIDEOPROC_PARAM_HEIGHT_ALIGN: {
				UINT32 *p_user = (UINT32 *)p_param;
				HD_VPRC_FLOW_MSG("h_align(%d) \n", *p_user);

				if(!(out_id >= HD_OUT_BASE && out_id <= HD_OUT_MAX)) {return HD_ERR_IO;}
				_HD_CONVERT_OUT_ID(out_id, rv);   if (rv != HD_OK) { return rv; }

				cmd.dest = ISF_PORT(self_id, out_id);
				cmd.param = VDOPRC_PARAM_HEIGHT_ALIGN;
				cmd.value = *p_user;
				cmd.size = 0;
				r = ISF_IOCTL(isf_fd, ISF_FLOW_CMD_SET_PARAM, &cmd);
	 			goto _VD_VPRC1;
			}
			break;
			case VENDOR_VIDEOPROC_PARAM_USER_CROP_TRIG: {
				UINT32 *p_user = (UINT32 *)p_param;
				HD_VPRC_FLOW_MSG("tigger(%d)\n", *p_user);

				if(!(out_id >= HD_OUT_BASE && out_id <= HD_OUT_MAX)) {return HD_ERR_IO;}
				_HD_CONVERT_OUT_ID(out_id, rv);   if (rv != HD_OK) { return rv; }

				cmd.dest = ISF_PORT(self_id, out_id);
				cmd.param = VDOPRC_PARAM_TRIG_USER_CROP;
				cmd.value = *p_user;
				cmd.size = 0;
				r = ISF_IOCTL(isf_fd, ISF_FLOW_CMD_SET_PARAM, &cmd);
	 			goto _VD_VPRC1;
			}
			break;
			case VENDOR_VIDEOPROC_PARAM_OUT_ONEBUF_MAX: {
				UINT32 *p_user = (UINT32 *)p_param;
				HD_VPRC_FLOW_MSG("max(0x%08x) \n", *p_user);

				if(!(out_id >= HD_OUT_BASE && out_id <= HD_OUT_MAX)) {return HD_ERR_IO;}
				_HD_CONVERT_OUT_ID(out_id, rv);   if (rv != HD_OK) { return rv; }

				cmd.dest = ISF_PORT(self_id, out_id);
				cmd.param = VDOPRC_PARAM_OUT_ONEBUF_MAX;
				cmd.value = *p_user;
				cmd.size = 0;
				r = ISF_IOCTL(isf_fd, ISF_FLOW_CMD_SET_PARAM, &cmd);
	 			goto _VD_VPRC1;
			}
			break;
			case VENDOR_VIDEOPROC_PARAM_LINEOFFSET_ALIGN: {
				UINT32 *p_user = (UINT32 *)p_param;
				HD_VPRC_FLOW_MSG("loff(%d) \n", *p_user);

				if(!(out_id >= HD_OUT_BASE && out_id <= HD_OUT_MAX)) {return HD_ERR_IO;}
				_HD_CONVERT_OUT_ID(out_id, rv);   if (rv != HD_OK) { return rv; }

                if((*p_user)%2) {
                    DBG_ERR("should multiple 2\r\n");
                    return HD_ERR_PARAM;
                }
				cmd.dest = ISF_PORT(self_id, out_id);
				cmd.param = VDOPRC_PARAM_LOFF_ALIGN;
				cmd.value = *p_user;
				cmd.size = 0;
				r = ISF_IOCTL(isf_fd, ISF_FLOW_CMD_SET_PARAM, &cmd);
	 			goto _VD_VPRC1;
			}
			break;
			case VENDOR_VIDEOPROC_PARAM_DRE_CFG: {
				VENDOR_VIDEOPROC_DRE_CFG* p_user = (VENDOR_VIDEOPROC_DRE_CFG*)p_param;
				HD_VPRC_FLOW_MSG("mode(%d) level(%d)\n", p_user->dre_proc_mode,p_user->dre_quality_level);

				if(!(out_id >= HD_OUT_BASE && out_id <= HD_OUT_MAX)) {return HD_ERR_IO;}
				_HD_CONVERT_OUT_ID(out_id, rv);   if (rv != HD_OK) { return rv; }

				cmd.dest = ISF_PORT(self_id, out_id);
				cmd.param = VDOPRC_PARAM_DRE_CFG;
				cmd.value = (ULONG)p_user;
				cmd.size = sizeof(VENDOR_VIDEOPROC_DRE_CFG);
				r = ISF_IOCTL(isf_fd, ISF_FLOW_CMD_SET_PARAM, &cmd);
	 			goto _VD_VPRC1;
			}
			break;
			case VENDOR_VIDEOPROC_PARAM_VPE_CLEAR_WIN: {
				VENDOR_VIDEOPROC_VPE_CLEAR_WIN *p_user = (VENDOR_VIDEOPROC_VPE_CLEAR_WIN *)p_param;
				HD_VPRC_FLOW_MSG("color(0x%08x) rect(%d,%d,%d,%d)\n", p_user->color,p_user->rect.x,p_user->rect.y,p_user->rect.w,p_user->rect.h);

				if(!(out_id >= HD_OUT_BASE && out_id <= HD_OUT_MAX)) {return HD_ERR_IO;}
				_HD_CONVERT_OUT_ID(out_id, rv);   if (rv != HD_OK) { return rv; }

				cmd.dest = ISF_PORT(self_id, out_id);
				cmd.param = VDOPRC_PARAM_VPE_CLEAR_WIN;
				cmd.value = (ULONG)p_user;
				cmd.size = sizeof(VENDOR_VIDEOPROC_VPE_CLEAR_WIN);
				r = ISF_IOCTL(isf_fd, ISF_FLOW_CMD_SET_PARAM, &cmd);
				goto _VD_VPRC1;
			}
			break;
            case VENDOR_VIDEOPROC_PARAM_FUNC_CONFIG: {
				HD_VIDEOPROC_FUNC_CONFIG* p_user = (HD_VIDEOPROC_FUNC_CONFIG*)p_param;

				if(!(out_id >= HD_OUT_BASE && out_id <= HD_OUT_MAX)) {return HD_ERR_IO;}
				_HD_CONVERT_OUT_ID(out_id, rv);   if (rv != HD_OK) { return rv; }

				cmd.dest = ISF_PORT(self_id, out_id);
				cmd.param = VDOPRC_PARAM_OUT_VNDCFG_FUNC;
				cmd.value = p_user->out_func;
				cmd.size = 0;
				r = ISF_IOCTL(isf_fd, ISF_FLOW_CMD_SET_PARAM, &cmd);
	 			goto _VD_VPRC1;
			}
			break;
			case VENDOR_VIDEOPROC_PARAM_VPE_SPLIT_INFO: {
				VENDOR_VIDEOPROC_VPE_SPLIT_INFO *p_user = (VENDOR_VIDEOPROC_VPE_SPLIT_INFO *)p_param;
				VDOPRC_SPLIT_INFO split_info = {0};
				ISF_VDO_MAX max = {0};
				ISF_VDO_WIN win = {0};

				HD_VPRC_FLOW_MSG("op(%d) split_point(%d) depth(%d) bg(%d,%d) dst_pos(%d,%d)\n", p_user->op, p_user->split_point, p_user->depth, p_user->bg.w, p_user->bg.h, p_user->dst_pos.x, p_user->dst_pos.y);

				if(!(out_id >= HD_OUT_BASE && out_id <= HD_OUT_MAX)) {return HD_ERR_IO;}
				_HD_CONVERT_OUT_ID(out_id, rv);   if (rv != HD_OK) { return rv; }

				cmd.dest = ISF_PORT(self_id, out_id);

				max.max_frame = p_user->depth;
				cmd.param = ISF_UNIT_PARAM_VDOMAX;
				cmd.value = (ULONG)&max;
				cmd.size = sizeof(ISF_VDO_MAX);
				r = ISF_IOCTL(isf_fd, ISF_FLOW_CMD_SET_PARAM, &cmd);
				if (r != 0) goto _VD_VPRC1;

				win.window.x = p_user->dst_pos.x;
				win.window.y = p_user->dst_pos.y;
				win.window.w = 0;
				win.window.h = 0;
				win.imgaspect.w = p_user->bg.w;
				win.imgaspect.h = p_user->bg.h;

				cmd.param = VDOPRC_PARAM_OUT_REGION;
				cmd.value = (ULONG)&win;
				cmd.size = sizeof(ISF_VDO_WIN);
				r = ISF_IOCTL(isf_fd, ISF_FLOW_CMD_SET_PARAM, &cmd);
				if (r != 0) goto _VD_VPRC1;

				split_info.op = p_user->op;
				split_info.split_point = p_user->split_point;
				cmd.param = VDOPRC_PARAM_SPLIT_INFO;
				cmd.value = (ULONG)&split_info;
				cmd.size = sizeof(ISF_VDO_WIN);
				r = ISF_IOCTL(isf_fd, ISF_FLOW_CMD_SET_PARAM, &cmd);
				if (r != 0) goto _VD_VPRC1;

			}
			break;

			default:
				rv = HD_ERR_PARAM;
				printf("vendor_videoproc_set not support item %d \n", param_id);
				return rv;
			break;
		}
	}

_VD_VPRC1:
	if (r == 0) {
		switch(cmd.rv) {
		case ISF_OK:  rv = HD_OK; break;
		default: rv = HD_ERR_SYS; break;
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

HD_RESULT _vendor_videoproc_get(UINT32 path_id, VENDOR_VIDEOPROC_PARAM_ID param_id, VOID *p_param)
{
	HD_DAL self_id = HD_GET_DEV(path_id);
	//HD_IO    in_id = HD_GET_IN(path_id);
	//HD_IO   out_id = HD_GET_OUT(path_id);
	HD_IO  ctrl_id = HD_GET_CTRL(path_id);
	HD_RESULT rv = HD_ERR_NG;
	int r = 0;
	ISF_FLOW_IOCTL_PARAM_ITEM cmd = {0};
	int isf_fd = _hd_common_get_fd();

	{
		CHAR  param_name[20];
		_vendor_videoproc_param_cvt_name(param_id, param_name, 20);

		HD_VPRC_FLOW_MSG("vendor_videoproc_get(%s):\n", param_name);
		HD_VPRC_FLOW_MSG("    path_id(0x%x) ", path_id);
	}

	if (isf_fd <= 0) {
		return HD_ERR_UNINIT;
	}

	if (p_param == NULL) {
		return HD_ERR_NULL_PTR;
	}

	_HD_CONVERT_SELF_ID(self_id, rv);   if (rv != HD_OK) { return rv; }

	rv = HD_OK;
	if(ctrl_id == HD_CTRL) {
		switch(param_id) {
			case VENDOR_VIDEOPROC_PARAM_ISP_CB: {
				ULONG *p_user = (ULONG *)p_param;
				ULONG isp_cb;


				HD_VPRC_FLOW_MSG("ai_cb(0x%lx) \n", p_param);

				cmd.dest = ISF_PORT(self_id, ISF_CTRL);
				cmd.param = VDOPRC_PARAM_ISP_CB;
				cmd.size = sizeof(ULONG);
				cmd.value = (ULONG)&isp_cb;

				r = ISF_IOCTL(isf_fd, ISF_FLOW_CMD_GET_PARAM, &cmd);
					if (r == 0 && cmd.rv == ISF_OK) {
					*p_user = isp_cb;
				}

				goto _VD_VPRC1;
			}
			break;
			default:
				rv = HD_ERR_PARAM;
				printf("vendor_videoproc_set not support item %d \n", param_id);
				return rv;
			break;
		}
	} else {
		switch (param_id) {
			default:
				rv = HD_ERR_PARAM;
				printf("vendor_videoproc_set not support item %d \n", param_id);
				return rv;
			break;
		}
	}

_VD_VPRC1:
	if (r == 0) {
		switch(cmd.rv) {
		case ISF_OK:  rv = HD_OK; break;
		default: rv = HD_ERR_SYS; break;
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

extern int _hd_videoproc_enable_pipe_bnr(void);
extern int _hd_videoproc_is_flow_bnr(HD_DAL dev_id);
extern HD_DAL _hd_videoproc_get_dev_id(HD_DAL self_id);
#define HD_VIDEOPROC_PATH(dev_id, in_id, out_id)	(((dev_id) << 16) | (((in_id) & 0x00ff) << 8)| ((out_id) & 0x00ff))
#define HD_VPRC_DID(dev_id) (dev_id - ISF_UNIT_VDOPRC)
#define DEV_INVALID             0xFFFFFFFF

HD_RESULT _vendor_videoproc_set_flow_bnr(UINT32 path_id, VENDOR_VIDEOPROC_PARAM_ID param_id, VOID *p_param)
{
	HD_DAL self_id = HD_GET_DEV(path_id);
	HD_IO   out_id = HD_GET_OUT(path_id);
	HD_IO  ctrl_id = HD_GET_CTRL(path_id);
	HD_RESULT rv = HD_ERR_NG;

	HD_DAL dev_id;

	_HD_CONVERT_SELF_ID(self_id, rv);   if (rv != HD_OK) { return rv; }

	dev_id = _hd_videoproc_get_dev_id(self_id);
	if (dev_id == DEV_INVALID) {
		DBG_ERR("self_id(0x%x) dev invalid\r\n", (unsigned int)self_id);
		rv = HD_ERR_NOT_OPEN;
		goto hd_exit;
	}

	rv = HD_OK;
	if(ctrl_id == HD_CTRL) {
		switch(param_id) {
			case VENDOR_VIDEOPROC_PARAM_DMA_ABORT:
			case VENDOR_VIDEOPROC_PARAM_IN_DEPTH:
			case VENDOR_VIDEOPROC_PARAM_STRIP:
			case VENDOR_VIDEOPROC_PARAM_THERMAL_INFO:
			case VENDOR_VIDEOPROC_PARAM_LCA_FUNC:
			{
				rv = _vendor_videoproc_set(HD_VIDEOPROC_CTRL(HD_VPRC_DID(self_id)), param_id, p_param);
				rv = _vendor_videoproc_set(HD_VIDEOPROC_CTRL(HD_VPRC_DID(dev_id)), param_id, p_param);
				goto hd_exit;
			}
			break;
			case VENDOR_VIDEOPROC_PARAM_SLICE_MODE:
			{
				rv = _vendor_videoproc_set(HD_VIDEOPROC_CTRL(HD_VPRC_DID(dev_id)), param_id, p_param);
				goto hd_exit;
			}
			break;
			case VENDOR_VIDEOPROC_PARAM_AI_CB:
			{
				rv = _vendor_videoproc_set(HD_VIDEOPROC_CTRL(HD_VPRC_DID(self_id)), param_id, p_param);
				rv = _vendor_videoproc_set(HD_VIDEOPROC_CTRL(HD_VPRC_DID(dev_id)), param_id, p_param);
				goto hd_exit;
			}
			break;
			case VENDOR_VIDEOPROC_PARAM_ISP_AI_START:
			case VENDOR_VIDEOPROC_PARAM_ISP_AI_STOP:
			{
				rv = _vendor_videoproc_set(HD_VIDEOPROC_CTRL(HD_VPRC_DID(dev_id)), VENDOR_VIDEOPROC_PARAM_AI_CB, &VDOPRC_PARAM_AI_CB_param);
				rv = _vendor_videoproc_set(HD_VIDEOPROC_CTRL(HD_VPRC_DID(dev_id)), param_id, p_param);
				rv = _vendor_videoproc_set(HD_VIDEOPROC_CTRL(HD_VPRC_DID(self_id)), param_id, p_param);
				goto hd_exit;
			}
			break;
			case VENDOR_VIDEOPROC_PARAM_ISP_AI_EFFECT: {
				rv = _vendor_videoproc_set(HD_VIDEOPROC_CTRL(HD_VPRC_DID(dev_id)), VENDOR_VIDEOPROC_PARAM_AI_CB, &VDOPRC_PARAM_AI_CB_param);
				if (rv != HD_OK) {
					DBG_DUMP("VENDOR_VIDEOPROC_PARAM_AI_CB failed\r\n");
				}
				rv = _vendor_videoproc_set(HD_VIDEOPROC_CTRL(HD_VPRC_DID(dev_id)), param_id, p_param);
				goto hd_exit;
			}
			break;
			default:
			break;
		}
	} else {
		switch (param_id) {
			case VENDOR_VIDEOPROC_PARAM_HEIGHT_ALIGN:
			case VENDOR_VIDEOPROC_PARAM_OUT_ONEBUF_MAX:
			case VENDOR_VIDEOPROC_PARAM_LINEOFFSET_ALIGN:
			case VENDOR_VIDEOPROC_PARAM_FUNC_CONFIG:
			{
				if(!(out_id >= HD_OUT_BASE && out_id <= HD_OUT_MAX)) {return HD_ERR_IO;}
				_HD_CONVERT_OUT_ID(out_id, rv);   if (rv != HD_OK) { return rv; }
				rv = _vendor_videoproc_set(HD_VIDEOPROC_PATH(HD_DAL_VIDEOPROC(HD_VPRC_DID(dev_id)), HD_IN(0), HD_OUT(out_id)), param_id, p_param);
			}
			break;
			default:
			break;
		}
	}

hd_exit:
	return rv;
}

HD_RESULT vendor_videoproc_set(UINT32 path_id, VENDOR_VIDEOPROC_PARAM_ID param_id, VOID *p_param)
{
	//--- check if PIPE info enable ---
	if (_hd_videoproc_enable_pipe_bnr()) {
		//--- check if PIPE info had been set ---
		if (_hd_videoproc_is_flow_bnr(HD_GET_DEV(path_id))) {
			return _vendor_videoproc_set_flow_bnr(path_id, param_id, p_param);
		}
	}
	return _vendor_videoproc_set(path_id, param_id, p_param);
}

HD_RESULT vendor_videoproc_get(UINT32 path_id, VENDOR_VIDEOPROC_PARAM_ID param_id, VOID *p_param)
{
	return _vendor_videoproc_get(path_id, param_id, p_param);
}
