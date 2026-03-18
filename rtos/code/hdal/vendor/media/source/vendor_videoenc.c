/**
	@brief Source file of vendor media videoenc.\n

	@file vendor_videoenc.c

	@ingroup mhdal

	@note Nothing.

	Copyright Novatek Microelectronics Corp. 2018.  All rights reserved.
*/
#include <string.h>
#if defined(__LINUX)
#include <sys/ioctl.h>
#endif
#include "kflow_common/isf_flow_def.h"
#include "kflow_common/isf_flow_ioctl.h"
#include "vendor_videoenc.h"
#include "kflow_videoenc/isf_vdoenc.h"
#include "videosprite/videosprite.h"
#include "hdal.h"
#include "vendor_type.h"

// INCLUDE_PROTECTED
#include "nmediarec_vdoenc.h"

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
#define HD_VIDEOENC_PATH(dev_id, in_id, out_id)	(((dev_id) << 16) | (((in_id) & 0x00ff) << 8)| ((out_id) & 0x00ff))

#define DEV_BASE        ISF_UNIT_VDOENC
#define DEV_COUNT       1 //ISF_MAX_VDOENC
#define IN_BASE         ISF_IN_BASE
#define IN_COUNT        16
#define OUT_BASE        ISF_OUT_BASE
#define OUT_COUNT       16

#define HD_DEV_BASE HD_DAL_VIDEOENC_BASE
#define HD_DEV_MAX  HD_DAL_VIDEOENC_MAX

#define KFLOW_VIDEOENC_PARAM_BEGIN     VDOENC_PARAM_START
#define KFLOW_VIDEOENC_PARAM_END       __VDOENC_PARAM_MAX__
#define KFLOW_VIDEOENC_PARAM_NUM      (KFLOW_VIDEOENC_PARAM_END - KFLOW_VIDEOENC_PARAM_BEGIN)

/*-----------------------------------------------------------------------------*/
/* Local Macros Declarations                                                   */
/*-----------------------------------------------------------------------------*/
#define _HD_CONVERT_SELF_ID(dev_id, rv) \
	do { \
		(rv) = HD_ERR_DEV;  \
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
extern HD_RESULT _hd_videoenc_kflow_param_set_by_vendor(UINT32 self_id, UINT32 out_id, UINT32 kflow_videoenc_param, ULONG param);
extern HD_RESULT _hd_get_struct_param(HD_DAL self_id, HD_IO out_id, UINT32 param, VOID *p_value, UINT32 size);

/*-----------------------------------------------------------------------------*/
/* Local Function Prototype                                                    */
/*-----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------*/
/* Debug Variables & Functions                                                 */
/*-----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------*/
/* Local Global Variables                                                      */
/*-----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------*/
/* Interface Functions                                                         */
/*-----------------------------------------------------------------------------*/
static HD_RESULT set_osg_qp(HD_PATH_ID path_id, VENDOR_VIDEOENC_OSG_QP *p_param)
{
	_VDS_QP qp = { 0 };

	if(!p_param){
		DBG_ERR("vendor_videoenc_set_osg_qp() : p_param is NULL\n");
		return HD_ERR_NULL_PTR;
	}

	qp.magic1        = 0xABCDEFAB;
	qp.magic2        = 1;
	qp.data.lpm_mode = p_param->lpm_mode;
	qp.data.tnr_mode = p_param->tnr_mode;
	qp.data.fro_mode = p_param->fro_mode;

	return hd_videoenc_set(path_id, HD_VIDEOENC_PARAM_IN_STAMP_ATTR, &qp);
}

static HD_RESULT set_osg_color_invert(HD_PATH_ID path_id, VENDOR_VIDEOENC_OSG_COLOR_INVERT *p_param)
{
	_VDS_COLOR_INVERT invert = { 0 };

	if(!p_param){
		DBG_ERR("vendor_videoenc_set_osg_color_invert() : p_param is NULL\n");
		return HD_ERR_NULL_PTR;
	}

	invert.magic1               = 0xABCDEFAB;
	invert.magic2               = 2;
	invert.data.blk_num         = p_param->blk_num;
	invert.data.org_color_level = p_param->org_color_level;
	invert.data.inv_color_level = p_param->inv_color_level;
	invert.data.nor_diff_th     = p_param->nor_diff_th;
	invert.data.inv_diff_th     = p_param->inv_diff_th;
	invert.data.sta_only_mode   = p_param->sta_only_mode;
	invert.data.full_eval_mode  = p_param->full_eval_mode;
	invert.data.eval_lum_targ   = p_param->eval_lum_targ;

	return hd_videoenc_set(path_id, HD_VIDEOENC_PARAM_IN_STAMP_ATTR, &invert);
}

static HD_RESULT set_osg_overlap(HD_PATH_ID path_id, int *p_param)
{
	_VDS_OVERLAP overlap = { 0 };

	if(!p_param){
		DBG_ERR("vendor_videoenc_set_osg_overlap() : p_param is NULL\n");
		return HD_ERR_NULL_PTR;
	}

	overlap.magic1               = 0xABCDEFAB;
	overlap.magic2               = 3;
	overlap.data.type            = *p_param;

	return hd_videoenc_set(path_id, HD_VIDEOENC_PARAM_IN_STAMP_ATTR, &overlap);
}

typedef struct _OSG_STAMP_ATTR {
	UINT32            magic1;
	UINT32            magic2;
	HD_OSG_STAMP_ATTR attr;
	UINT32            bg_alpha;
} OSG_STAMP_ATTR;

static HD_RESULT set_encoder_stamp_attr(HD_PATH_ID path_id, VENDOR_VIDEOENC_OSG_STAMP_ATTR *p_param)
{
	OSG_STAMP_ATTR attr = { 0 };

	if(!p_param){
		DBG_ERR("set_encoder_stamp_attr() : p_param is NULL\n");
		return HD_ERR_NULL_PTR;
	}

	attr.magic1   = 0xABCDEFAB;
	attr.magic2   = 4;
	attr.bg_alpha = p_param->bg_alpha;

	memcpy(&attr.attr, &p_param->attr, sizeof(HD_OSG_STAMP_ATTR));

	return hd_videoenc_set(path_id, HD_VIDEOENC_PARAM_IN_STAMP_ATTR, &attr);
}

static HD_RESULT set_encoder_mask(HD_PATH_ID path_id, VENDOR_VIDEOENC_OSG_MASK *p_param)
{
	_VDS_ENC_MASK mask = { 0 };

	if(!p_param){
		DBG_ERR("set_encoder_mask() : p_param is NULL\n");
		return HD_ERR_NULL_PTR;
	}

	mask.magic1          = 0xABCDEFAB;
	mask.magic2          = 1;
	mask.data.color      = p_param->color;
	mask.data.alpha      = p_param->alpha;
	mask.data.x          = p_param->position.x;
	mask.data.y          = p_param->position.y;
	mask.data.w          = p_param->dim.w;
	mask.data.h          = p_param->dim.h;
	mask.data.thickness  = p_param->thickness;
	mask.data.layer      = p_param->layer;
	mask.data.region     = p_param->region;

	if(p_param->type == HD_OSG_MASK_TYPE_SOLID || p_param->type == HD_OSG_MASK_TYPE_NULL)
		mask.data.type = VDS_MASK_TYPE_SOLID;
	else if(p_param->type == HD_OSG_MASK_TYPE_HOLLOW)
		mask.data.type = VDS_MASK_TYPE_HOLLOW;
	else{
		printf("set_encoder_mask() : type(%d) is not supported\n", p_param->type);
		return HD_ERR_NOT_SUPPORT;
	}

	return hd_videoenc_set(path_id, HD_VIDEOENC_PARAM_IN_MASK_ATTR, &mask);
}

static HD_RESULT set_encoder_mosaic(HD_PATH_ID path_id, VENDOR_VIDEOENC_OSG_MOSAIC *p_param)
{
	_VDS_ENC_MOSAIC mosaic = { 0 };

	if(!p_param){
		DBG_ERR("set_encoder_mosaic() : p_param is NULL\n");
		return HD_ERR_NULL_PTR;
	}

	mosaic.magic1              = 0xABCDEFAB;
	mosaic.magic2              = 1;
	mosaic.data.x              = p_param->position.x;
	mosaic.data.y              = p_param->position.y;
	mosaic.data.w              = p_param->dim.w;
	mosaic.data.h              = p_param->dim.h;
	mosaic.data.layer          = p_param->layer;
	mosaic.data.region         = p_param->region;
	mosaic.data.mosaic_blk_w   = p_param->mosaic_blk_w;
	mosaic.data.mosaic_blk_h   = p_param->mosaic_blk_h;

	return hd_videoenc_set(path_id, HD_VIDEOENC_PARAM_IN_MOSAIC_ATTR, &mosaic);
}

static HD_RESULT set_encoder_mosaic_size(HD_PATH_ID path_id, HD_OSG_MOSAIC_ATTR *p_param)
{
	_VDS_MOSAIC_BLK_SIZE mosaic = { 0 };

	if(!p_param){
		DBG_ERR("set_encoder_mosaic_size() : p_param is NULL\n");
		return HD_ERR_NULL_PTR;
	}

	mosaic.magic1              = 0xABCDEFAB;
	mosaic.magic2              = 2;
	mosaic.data.mosaic_blk_w   = p_param->mosaic_blk_w;
	mosaic.data.mosaic_blk_h   = p_param->mosaic_blk_h;

	return hd_videoenc_set(path_id, HD_VIDEOENC_PARAM_IN_MOSAIC_ATTR, &mosaic);
}

static HD_RESULT set_encoder_qp_mosaic(HD_PATH_ID path_id, int *p_param)
{
	_VDS_QP_MOSAIC mosaic = { 0 };

	if(!p_param){
		DBG_ERR("set_encoder_qp_mosaic() : p_param is NULL\n");
		return HD_ERR_NULL_PTR;
	}

	mosaic.magic1              = 0xABCDEFAB;
	mosaic.magic2              = 3;
	mosaic.data.qp_mosaic      = *p_param;

	return hd_videoenc_set(path_id, HD_VIDEOENC_PARAM_IN_MOSAIC_ATTR, &mosaic);
}

HD_RESULT vendor_videoenc_set(HD_PATH_ID path_id, VENDOR_VIDEOENC_PARAM_ID id, VOID *p_param)
{
	HD_DAL self_id = HD_GET_DEV(path_id);
	//HD_IO    in_id = HD_GET_IN(path_id);
	HD_IO   out_id = HD_GET_OUT(path_id);
	//HD_IO ctrl_id  = HD_GET_CTRL(path_id);
	HD_RESULT rv = HD_ERR_NG;
	int r=0;
	ISF_FLOW_IOCTL_PARAM_ITEM cmd = {0};

	int isf_fd = _hd_common_get_fd();

	if (isf_fd <= 0) {
		return HD_ERR_UNINIT;
	}

	if (p_param == NULL) {
		return HD_ERR_NULL_PTR;
	}

	_HD_CONVERT_SELF_ID(self_id, rv);   if (rv != HD_OK) { return rv; }
	rv = HD_OK;

	switch (id) {
		case VENDOR_VIDEOENC_PARAM_OUT_BS_RESERVED_SIZE: {
			VENDOR_VIDEOENC_BS_RESERVED_SIZE_CFG* p_user = (VENDOR_VIDEOENC_BS_RESERVED_SIZE_CFG*)p_param;
			UINT32 reserved_size = p_user->reserved_size;

			if(!(out_id >= HD_OUT_BASE && out_id <= HD_OUT_MAX)) { return HD_ERR_IO; }
			_HD_CONVERT_OUT_ID(out_id, rv);     if (rv != HD_OK) { return rv; }

			cmd.dest = ISF_PORT(self_id, out_id);
			cmd.param = VDOENC_PARAM_BS_RESERVED_SIZE;
			cmd.value = reserved_size;
			cmd.size = 0;
			r = ISF_IOCTL(isf_fd, ISF_FLOW_CMD_SET_PARAM, &cmd);
			goto _VD_VE1;
		}
		break;

		case VENDOR_VIDEOENC_PARAM_OUT_TIMELAPSE_TIME: {
			VENDOR_VIDEOENC_TIMELAPSE_TIME_CFG* p_user = (VENDOR_VIDEOENC_TIMELAPSE_TIME_CFG*)p_param;
			UINT32 timelapse_time = p_user->timelapse_time;

			if(!(out_id >= HD_OUT_BASE && out_id <= HD_OUT_MAX)) { return HD_ERR_IO; }
			_HD_CONVERT_OUT_ID(out_id, rv);     if (rv != HD_OK) { return rv; }

			cmd.dest = ISF_PORT(self_id, out_id);
			cmd.param = VDOENC_PARAM_TIMELAPSE_TIME;
			cmd.value = timelapse_time;
			cmd.size = 0;
			r = ISF_IOCTL(isf_fd, ISF_FLOW_CMD_SET_PARAM, &cmd);
			goto _VD_VE1;
		}
		break;

		case VENDOR_VIDEOENC_PARAM_OUT_JPG_RC: {
			VENDOR_VIDEOENC_JPG_RC_CFG* p_user = (VENDOR_VIDEOENC_JPG_RC_CFG*)p_param;
			UINT32 vbr_mode_en  = p_user->vbr_mode_en;
			UINT32 jpg_rc_min_q = p_user->min_quality;
			UINT32 jpg_rc_max_q = p_user->max_quality;
			UINT32 vbr_priority = p_user->vbr_priority;
			UINT32 min_framerate = p_user->min_framerate;

			if (vbr_mode_en > 1) {
				printf("invalid vbr_mode_en (%lu), should be 0~1\n", vbr_mode_en);
				return HD_ERR_PARAM;
			}

			if (jpg_rc_min_q > 100) {
				printf("invalid jpg_rc_min_q (%u), should be 0~100\n", jpg_rc_min_q);
				return HD_ERR_PARAM;
			}

			if (jpg_rc_max_q > 100) {
				printf("invalid jpg_rc_max_q (%u), should be 0~100\n", jpg_rc_max_q);
				return HD_ERR_PARAM;
			}

			if (jpg_rc_min_q > jpg_rc_max_q) {
				printf("invalid jpg_rc_min_q (%u) >  jpg_rc_max_q (%u)\n", jpg_rc_min_q, jpg_rc_max_q);
				return HD_ERR_PARAM;
			}

			if (vbr_priority > 2) {
				printf("invalid vbr_priority (%u), should be 0~2\n", vbr_priority);
				return HD_ERR_PARAM;
			}

			if(!(out_id >= HD_OUT_BASE && out_id <= HD_OUT_MAX)) { return HD_ERR_IO; }
			_HD_CONVERT_OUT_ID(out_id, rv);     if (rv != HD_OK) { return rv; }

			_hd_videoenc_kflow_param_set_by_vendor(self_id, out_id, VDOENC_PARAM_JPG_VBR_MODE,       (ULONG)vbr_mode_en);
			_hd_videoenc_kflow_param_set_by_vendor(self_id, out_id, VDOENC_PARAM_JPG_RC_MIN_QUALITY, (ULONG)jpg_rc_min_q);
			_hd_videoenc_kflow_param_set_by_vendor(self_id, out_id, VDOENC_PARAM_JPG_RC_MAX_QUALITY, (ULONG)jpg_rc_max_q);
			_hd_videoenc_kflow_param_set_by_vendor(self_id, out_id, VDOENC_PARAM_JPG_RC_VBR_PRIORITY, (ULONG)vbr_priority);
			_hd_videoenc_kflow_param_set_by_vendor(self_id, out_id, VDOENC_PARAM_JPG_RC_MIN_FRAME_RATE, (ULONG)min_framerate);

			return HD_OK;
		}
		break;

		case VENDOR_VIDEOENC_PARAM_OUT_JPG_YUV_TRANS: {
			VENDOR_VIDEOENC_JPG_YUV_TRANS_CFG* p_user = (VENDOR_VIDEOENC_JPG_YUV_TRANS_CFG*)p_param;
			UINT32 jpg_yuv_trans_en = p_user->jpg_yuv_trans_en;

			if (jpg_yuv_trans_en > 1) {
				printf("invalid jpg_yuv_trans_en (%lu), should be 0~1\n", jpg_yuv_trans_en);
				return HD_ERR_PARAM;
			}

			if(!(out_id >= HD_OUT_BASE && out_id <= HD_OUT_MAX)) { return HD_ERR_IO; }
			_HD_CONVERT_OUT_ID(out_id, rv);     if (rv != HD_OK) { return rv; }

			return _hd_videoenc_kflow_param_set_by_vendor(self_id, out_id, VDOENC_PARAM_JPG_YUV_TRAN_ENABLE, (ULONG)jpg_yuv_trans_en);
		}
		break;

		case VENDOR_VIDEOENC_PARAM_OUT_TRIG_SNAPSHOT: {
			VENDOR_VIDEOENC_H26XENC_TRIG_SNAPSHOT* p_user_vendor = (VENDOR_VIDEOENC_H26XENC_TRIG_SNAPSHOT *)p_param;
			HD_H26XENC_TRIG_SNAPSHOT* p_user_hd = (HD_H26XENC_TRIG_SNAPSHOT *)p_param;  // VENDOR_VIDEOENC_H26XENC_TRIG_SNAPSHOT cast to HD_H26XENC_TRIG_SNAPSHOT

			rv = hd_videoenc_set(path_id, HD_VIDEOENC_PARAM_OUT_TRIG_SNAPSHOT, p_user_hd);
			if (rv != HD_OK) { return rv; }

			//get result (bs size + timestamp)
			if(!(out_id >= HD_OUT_BASE && out_id <= HD_OUT_MAX)) { return HD_ERR_IO; }
			_HD_CONVERT_OUT_ID(out_id, rv);     if (rv != HD_OK) { return rv; }

			{
				VDOENC_SNAP_INFO snap_info = {0};
				cmd.dest = ISF_PORT(self_id, out_id);
				cmd.param = VDOENC_PARAM_SNAPSHOT_PURE_LINUX;
				cmd.value = (ULONG)&snap_info;
				cmd.size = sizeof(VDOENC_SNAP_INFO);

				r = ISF_IOCTL(isf_fd, ISF_FLOW_CMD_GET_PARAM, &cmd);
				if (r != 0) {
					p_user_vendor->size = 0;
					p_user_vendor->timestamp = 0;
					goto _VD_VE1;
				}
				p_user_vendor->size      = snap_info.size;
				p_user_vendor->timestamp = snap_info.timestamp;
			}
		}
		break;

		case VENDOR_VIDEOENC_PARAM_OUT_RDO: {
			VENDOR_VIDEOENC_RDO_CFG* p_user = (VENDOR_VIDEOENC_RDO_CFG*)p_param;

			if (p_user->rdo_codec > 1) {
				printf("invalid rdo rdo_codec (%u), should be 0~1\n", p_user->rdo_codec);
				return HD_ERR_PARAM;
			}
			if (p_user->rdo_codec == VENDOR_VIDEOENC_CODEC_264) {
				if (p_user->rdo_param.rdo_264.avc_intra_4x4_cost_bias > 31) {
					printf("invalid rdo avc_intra_4x4_cost_bias (%u), should be 0~31\n", p_user->rdo_param.rdo_264.avc_intra_4x4_cost_bias);
					return HD_ERR_PARAM;
				}

				if (p_user->rdo_param.rdo_264.avc_intra_8x8_cost_bias > 31) {
					printf("invalid rdo avc_intra_8x8_cost_bias (%u), should be 0~31\n", p_user->rdo_param.rdo_264.avc_intra_8x8_cost_bias);
					return HD_ERR_PARAM;
				}

				if (p_user->rdo_param.rdo_264.avc_intra_16x16_cost_bias > 31) {
					printf("invalid rdo avc_intra_16x16_cost_bias (%u), should be 0~31\n", p_user->rdo_param.rdo_264.avc_intra_16x16_cost_bias);
					return HD_ERR_PARAM;
				}

				if (p_user->rdo_param.rdo_264.avc_inter_tu4_cost_bias > 31) {
					printf("invalid rdo avc_inter_tu4_cost_bias (%u), should be 0~31\n", p_user->rdo_param.rdo_264.avc_inter_tu4_cost_bias);
					return HD_ERR_PARAM;
				}

				if (p_user->rdo_param.rdo_264.avc_inter_tu8_cost_bias > 31) {
					printf("invalid rdo avc_inter_tu8_cost_bias (%u), should be 0~31\n", p_user->rdo_param.rdo_264.avc_inter_tu8_cost_bias);
					return HD_ERR_PARAM;
				}

				if (p_user->rdo_param.rdo_264.avc_inter_skip_cost_bias > 31) {
					printf("invalid rdo avc_inter_skip_cost_bias (%u), should be 0~31\n", p_user->rdo_param.rdo_264.avc_inter_skip_cost_bias);
					return HD_ERR_PARAM;
				}
			} else {
				if (p_user->rdo_param.rdo_265.hevc_intra_32x32_cost_bias > 15) {
					printf("invalid rdo hevc_intra_32x32_cost_bias (%u), should be 0~15\n", p_user->rdo_param.rdo_265.hevc_intra_32x32_cost_bias);
					return HD_ERR_PARAM;
				}

				if (p_user->rdo_param.rdo_265.hevc_intra_16x16_cost_bias > 15) {
					printf("invalid rdo hevc_intra_16x16_cost_bias (%u), should be 0~15\n", p_user->rdo_param.rdo_265.hevc_intra_16x16_cost_bias);
					return HD_ERR_PARAM;
				}

				if (p_user->rdo_param.rdo_265.hevc_intra_8x8_cost_bias > 15) {
					printf("invalid rdo hevc_intra_8x8_cost_bias (%u), should be 0~15\n", p_user->rdo_param.rdo_265.hevc_intra_8x8_cost_bias);
					return HD_ERR_PARAM;
				}

				if (p_user->rdo_param.rdo_265.hevc_inter_skip_cost_bias < -16 || p_user->rdo_param.rdo_265.hevc_inter_skip_cost_bias > 15) {
					printf("invalid rdo hevc_inter_skip_cost_bias (%d), should be -16~15\n", p_user->rdo_param.rdo_265.hevc_inter_skip_cost_bias);
					return HD_ERR_PARAM;
				}

				if (p_user->rdo_param.rdo_265.hevc_inter_merge_cost_bias < -16 || p_user->rdo_param.rdo_265.hevc_inter_merge_cost_bias > 15) {
					printf("invalid rdo hevc_inter_merge_cost_bias (%d), should be -16~15\n", p_user->rdo_param.rdo_265.hevc_inter_merge_cost_bias);
					return HD_ERR_PARAM;
				}

				if (p_user->rdo_param.rdo_265.hevc_inter_64x64_cost_bias > 31) {
					printf("invalid rdo hevc_inter_64x64_cost_bias (%u), should be 0~31\n", p_user->rdo_param.rdo_265.hevc_inter_64x64_cost_bias);
					return HD_ERR_PARAM;
				}

				if (p_user->rdo_param.rdo_265.hevc_inter_64x32_32x64_cost_bias > 31) {
					printf("invalid rdo hevc_inter_64x32_32x64_cost_bias (%u), should be 0~31\n", p_user->rdo_param.rdo_265.hevc_inter_64x32_32x64_cost_bias);
					return HD_ERR_PARAM;
				}

				if (p_user->rdo_param.rdo_265.hevc_inter_32x32_cost_bias > 31) {
					printf("invalid rdo hevc_inter_32x32_cost_bias (%u), should be 0~31\n", p_user->rdo_param.rdo_265.hevc_inter_32x32_cost_bias);
					return HD_ERR_PARAM;
				}

				if (p_user->rdo_param.rdo_265.hevc_inter_32x16_16x32_cost_bias > 31) {
					printf("invalid rdo hevc_inter_32x16_16x32_cost_bias (%u), should be 0~31\n", p_user->rdo_param.rdo_265.hevc_inter_32x16_16x32_cost_bias);
					return HD_ERR_PARAM;
				}

				if (p_user->rdo_param.rdo_265.hevc_inter_16x16_cost_bias > 31) {
					printf("invalid rdo hevc_inter_16x16_cost_bias (%u), should be 0~31\n", p_user->rdo_param.rdo_265.hevc_inter_16x16_cost_bias);
					return HD_ERR_PARAM;
				}
			}

			if(!(out_id >= HD_OUT_BASE && out_id <= HD_OUT_MAX)) { return HD_ERR_IO; }
			_HD_CONVERT_OUT_ID(out_id, rv);     if (rv != HD_OK) { return rv; }

			return _hd_videoenc_kflow_param_set_by_vendor(self_id, out_id, VDOENC_PARAM_RDO, (ULONG)p_param);
		}
		break;

		case VENDOR_VIDEOENC_PARAM_OUT_JND: {
			VENDOR_VIDEOENC_JND_CFG* p_user = (VENDOR_VIDEOENC_JND_CFG*)p_param;

			if (p_user->enable > 1) {
				printf("invalid jnd enable (%u), should be 0~1\n", p_user->enable);
				return HD_ERR_PARAM;
			}

			if (p_user->str > 15) {
				printf("invalid jnd str (%u), should be 0~15\n", p_user->str);
				return HD_ERR_PARAM;
			}

			if (p_user->level > 15) {
				printf("invalid jnd level (%u), should be 0~15\n", p_user->level);
				return HD_ERR_PARAM;
			}

			if (p_user->threshold > 255) {
				printf("invalid jnd threshold (%u), should be 0~255\n", p_user->threshold);
				return HD_ERR_PARAM;
			}

			if (p_user->c_str > 15) {
				printf("invalid jnd c_str (%u), should be 0~15\n", p_user->c_str);
				return HD_ERR_PARAM;
			}

			if (p_user->t_str > 15) {
				printf("invalid jnd t_str (%u), should be 0~15\n", p_user->t_str);
				return HD_ERR_PARAM;
			}

			if(!(out_id >= HD_OUT_BASE && out_id <= HD_OUT_MAX)) { return HD_ERR_IO; }
			_HD_CONVERT_OUT_ID(out_id, rv);     if (rv != HD_OK) { return rv; }

			return _hd_videoenc_kflow_param_set_by_vendor(self_id, out_id, VDOENC_PARAM_JND, (ULONG)p_param);
		}
		break;

		case VENDOR_VIDEOENC_PARAM_OUT_H26X_VBR_POLICY: {
			VENDOR_VIDEOENC_H26X_VBR_POLICY_CFG* p_user = (VENDOR_VIDEOENC_H26X_VBR_POLICY_CFG*)p_param;
			UINT32 h26x_vbr_policy = p_user->h26x_vbr_policy;

			if (h26x_vbr_policy > 1) {
				printf("invalid h26x_vbr_policy (%lu), should be 0~1\n", h26x_vbr_policy);
				return HD_ERR_PARAM;
			}

			if(!(out_id >= HD_OUT_BASE && out_id <= HD_OUT_MAX)) { return HD_ERR_IO; }
			_HD_CONVERT_OUT_ID(out_id, rv);     if (rv != HD_OK) { return rv; }

			return _hd_videoenc_kflow_param_set_by_vendor(self_id, out_id, VDOENC_PARAM_H26X_VBR_POLICY, (ULONG)h26x_vbr_policy);
		}
		break;

		case VENDOR_VIDEOENC_PARAM_OUT_H26X_RC_GOP: {
			VENDOR_VIDEOENC_H26X_RC_GOP_CFG* p_user = (VENDOR_VIDEOENC_H26X_RC_GOP_CFG*)p_param;
			UINT32 h26x_rc_gop = p_user->h26x_rc_gop;
			if (h26x_rc_gop > 4096) {
				printf("invalid h26x_rc_gop (%lu), should be 0~4096\n", h26x_rc_gop);
				return HD_ERR_PARAM;
			}

			if(!(out_id >= HD_OUT_BASE && out_id <= HD_OUT_MAX)) { return HD_ERR_IO; }
			_HD_CONVERT_OUT_ID(out_id, rv);     if (rv != HD_OK) { return rv; }

			return _hd_videoenc_kflow_param_set_by_vendor(self_id, out_id, VDOENC_PARAM_H26X_RC_GOPNUM, (ULONG)h26x_rc_gop);
		}
		break;

		case VENDOR_VIDEOENC_PARAM_OUT_H26X_ENC_GOP: {
			VENDOR_VIDEOENC_H26X_ENC_GOP_CFG* p_user = (VENDOR_VIDEOENC_H26X_ENC_GOP_CFG*)p_param;
			UINT32 h26x_enc_gop = p_user->h26x_enc_gop;
			if (h26x_enc_gop > 4096) {
				printf("invalid h26x_enc_gop (%lu), should be 0~4096\n", h26x_enc_gop);
				return HD_ERR_PARAM;
			}

			if(!(out_id >= HD_OUT_BASE && out_id <= HD_OUT_MAX)) { return HD_ERR_IO; }
			_HD_CONVERT_OUT_ID(out_id, rv);     if (rv != HD_OK) { return rv; }

			return _hd_videoenc_kflow_param_set_by_vendor(self_id, out_id, VDOENC_PARAM_GOPNUM_ONSTART, (ULONG)h26x_enc_gop);
		}
		break;

		case VENDOR_VIDEOENC_PARAM_OUT_MIN_RATIO: {
			VENDOR_VIDEOENC_MIN_RATIO_CFG* p_user = (VENDOR_VIDEOENC_MIN_RATIO_CFG*)p_param;
			UINT32 min_i_ratio = p_user->min_i_ratio;
			UINT32 min_p_ratio = p_user->min_p_ratio;

			if(!(out_id >= HD_OUT_BASE && out_id <= HD_OUT_MAX)) { return HD_ERR_IO; }
			_HD_CONVERT_OUT_ID(out_id, rv);     if (rv != HD_OK) { return rv; }

			_hd_videoenc_kflow_param_set_by_vendor(self_id, out_id, VDOENC_PARAM_MIN_I_RATIO, (ULONG)min_i_ratio);
			_hd_videoenc_kflow_param_set_by_vendor(self_id, out_id, VDOENC_PARAM_MIN_P_RATIO, (ULONG)min_p_ratio);

			return HD_OK;
		}
		break;

		case VENDOR_VIDEOENC_PARAM_OUT_COLMV: {
			VENDOR_VIDEOENC_H26X_ENC_COLMV* p_user = (VENDOR_VIDEOENC_H26X_ENC_COLMV*)p_param;
			UINT32 h26x_colmv_en = p_user->h26x_colmv_en;

			if(!(out_id >= HD_OUT_BASE && out_id <= HD_OUT_MAX)) { return HD_ERR_IO; }
			_HD_CONVERT_OUT_ID(out_id, rv);     if (rv != HD_OK) { return rv; }

			_hd_videoenc_kflow_param_set_by_vendor(self_id, out_id, VDOENC_PARAM_COLMV_ENABLE, (ULONG)h26x_colmv_en);

			return HD_OK;
		}
		break;

		case VENDOR_VIDEOENC_PARAM_OUT_COMM_RECFRM: {
			VENDOR_VIDEOENC_H26X_COMM_RECFRM* p_user = (VENDOR_VIDEOENC_H26X_COMM_RECFRM*)p_param;
			UINT32 h26x_comm_recfrm_en = p_user->enable;
			UINT32 h26x_comm_base_recfrm_en = p_user->h26x_comm_base_recfrm_en;
			UINT32 h26x_comm_svc_recfrm_en = p_user->h26x_comm_svc_recfrm_en;
			UINT32 h26x_comm_ltr_recfrm_en = p_user->h26x_comm_ltr_recfrm_en;

			if(!(out_id >= HD_OUT_BASE && out_id <= HD_OUT_MAX)) { return HD_ERR_IO; }
			_HD_CONVERT_OUT_ID(out_id, rv);     if (rv != HD_OK) { return rv; }

			_hd_videoenc_kflow_param_set_by_vendor(self_id, out_id, VDOENC_PARAM_COMM_RECFRM_ENABLE, (ULONG)h26x_comm_recfrm_en);
			_hd_videoenc_kflow_param_set_by_vendor(self_id, out_id, VDOENC_PARAM_COMM_BASE_RECFRM, (ULONG)h26x_comm_base_recfrm_en);
			_hd_videoenc_kflow_param_set_by_vendor(self_id, out_id, VDOENC_PARAM_COMM_SVC_RECFRM, (ULONG)h26x_comm_svc_recfrm_en);
			_hd_videoenc_kflow_param_set_by_vendor(self_id, out_id, VDOENC_PARAM_COMM_LTR_RECFRM, (ULONG)h26x_comm_ltr_recfrm_en);

			return HD_OK;
		}
		break;

		case VENDOR_VIDEOENC_PARAM_OUT_FIT_WORK_MEMORY: {
			VENDOR_VIDEOENC_FIT_WORK_MEMORY* p_user = (VENDOR_VIDEOENC_FIT_WORK_MEMORY*)p_param;
			UINT32 b_fit_work_memory = p_user->b_enable;

			if(!(out_id >= HD_OUT_BASE && out_id <= HD_OUT_MAX)) { return HD_ERR_IO; }
			_HD_CONVERT_OUT_ID(out_id, rv);     if (rv != HD_OK) { return rv; }

			return _hd_videoenc_kflow_param_set_by_vendor(self_id, out_id, VDOENC_PARAM_MAXMEM_FIT_WORK_MEMORY, (ULONG)b_fit_work_memory);
		}
		break;

		case VENDOR_VIDEOENC_PARAM_OUT_LONG_START_CODE: {
			VENDOR_VIDEOENC_LONG_START_CODE* p_user = (VENDOR_VIDEOENC_LONG_START_CODE*)p_param;
			UINT32 long_start_code_en = p_user->long_start_code_en;

			if(!(out_id >= HD_OUT_BASE && out_id <= HD_OUT_MAX)) { return HD_ERR_IO; }
			_HD_CONVERT_OUT_ID(out_id, rv);     if (rv != HD_OK) { return rv; }

			_hd_videoenc_kflow_param_set_by_vendor(self_id, out_id, VDOENC_PARAM_LONG_START_CODE, (ULONG)long_start_code_en);

			return HD_OK;
		}
		break;

		case VENDOR_VIDEOENC_PARAM_OUT_H26X_SVC_WEIGHT_MODE: {
			VENDOR_VIDEOENC_H26X_SVC_WEIGHT_MODE* p_user = (VENDOR_VIDEOENC_H26X_SVC_WEIGHT_MODE*)p_param;
			UINT32 h26x_svc_weight_mode = p_user->h26x_svc_weight_mode;

			if (h26x_svc_weight_mode > 1) {
				printf("invalid h26x_svc_weight_mode (%lu), should be 0~1\n", h26x_svc_weight_mode);
				return HD_ERR_PARAM;
			}

			if(!(out_id >= HD_OUT_BASE && out_id <= HD_OUT_MAX)) { return HD_ERR_IO; }
			_HD_CONVERT_OUT_ID(out_id, rv);     if (rv != HD_OK) { return rv; }

			return _hd_videoenc_kflow_param_set_by_vendor(self_id, out_id, VDOENC_PARAM_H26X_SVC_WEIGHT_MODE, (ULONG)h26x_svc_weight_mode);
		}
		break;

		case VENDOR_VIDEOENC_PARAM_OUT_SKIP_BS_QUICK_ROLLBACK: {
			VENDOR_VIDEOENC_SKIP_BS_QUICK_ROLLBACK_CFG* p_user = (VENDOR_VIDEOENC_SKIP_BS_QUICK_ROLLBACK_CFG*)p_param;
			UINT32 b_skip_bs_quick_rollback = p_user->enable;

			if(!(out_id >= HD_OUT_BASE && out_id <= HD_OUT_MAX)) { return HD_ERR_IO; }
			_HD_CONVERT_OUT_ID(out_id, rv);     if (rv != HD_OK) { return rv; }

			return _hd_videoenc_kflow_param_set_by_vendor(self_id, out_id, VDOENC_PARAM_SKIP_BS_QUICK_ROLLBACK, (ULONG)b_skip_bs_quick_rollback);
		}
		break;

		case VENDOR_VIDEOENC_PARAM_OUT_QUALITY_BASE: {
			VENDOR_VIDEOENC_QUALITY_BASE_MODE* p_user = (VENDOR_VIDEOENC_QUALITY_BASE_MODE*)p_param;
			UINT32 quality_base_en = p_user->quality_base_en;
			if(!(out_id >= HD_OUT_BASE && out_id <= HD_OUT_MAX)) { return HD_ERR_IO; }
			_HD_CONVERT_OUT_ID(out_id, rv); 	if (rv != HD_OK) { return rv; }
			_hd_videoenc_kflow_param_set_by_vendor(self_id, out_id, VDOENC_PARAM_QUALITY_BASE_MODE_ENABLE, (ULONG)quality_base_en);
			return HD_OK;
		}
		break;

		case VENDOR_VIDEOENC_PARAM_OUT_DMA_ABORT: {
			VENDOR_VIDEOENC_DMA_ABORT* p_user = (VENDOR_VIDEOENC_DMA_ABORT*)p_param;
			UINT32 dma_abort = p_user->dma_abort_en;

			if(!(out_id >= HD_OUT_BASE && out_id <= HD_OUT_MAX)) { return HD_ERR_IO; }
			_HD_CONVERT_OUT_ID(out_id, rv); 	if (rv != HD_OK) { return rv; }

			cmd.dest = ISF_PORT(self_id, out_id);
			cmd.param = VDOENC_PARAM_DMA_ABORT;
			cmd.value = dma_abort;
			cmd.size = 0;
			r = ISF_IOCTL(isf_fd, ISF_FLOW_CMD_SET_PARAM, &cmd);
			goto _VD_VE1;
		}
		break;

		case VENDOR_VIDEOENC_PARAM_OUT_PADDING_MODE: {
			VENDOR_VIDEOENC_PADDING_MODE_CFG* p_user = (VENDOR_VIDEOENC_PADDING_MODE_CFG*)p_param;
			UINT32 padding_mode = p_user->mode;

			if (padding_mode > 1) {
				printf("invalid padding_mode (%lu), should be 0~1\n", padding_mode);
				return HD_ERR_PARAM;
			}

			if(!(out_id >= HD_OUT_BASE && out_id <= HD_OUT_MAX)) { return HD_ERR_IO; }
			_HD_CONVERT_OUT_ID(out_id, rv);     if (rv != HD_OK) { return rv; }

			return _hd_videoenc_kflow_param_set_by_vendor(self_id, out_id, VDOENC_PARAM_HW_PADDING_MODE, (ULONG)padding_mode);
		}
		break;

		case VENDOR_VIDEOENC_PARAM_OUT_BS_QUEUE_CNT: {
			VENDOR_VIDEOENC_BS_QUEUE_CNT_CFG* p_user = (VENDOR_VIDEOENC_BS_QUEUE_CNT_CFG*)p_param;
			UINT32 bs_queue_cnt = p_user->queue_cnt;

			if(!(out_id >= HD_OUT_BASE && out_id <= HD_OUT_MAX)) { return HD_ERR_IO; }
			_HD_CONVERT_OUT_ID(out_id, rv);     if (rv != HD_OK) { return rv; }

			cmd.dest = ISF_PORT(self_id, out_id);
			cmd.param = VDOENC_PARAM_BS_QUEUE_CNT;
			cmd.value = bs_queue_cnt;
			cmd.size = 0;
			r = ISF_IOCTL(isf_fd, ISF_FLOW_CMD_SET_PARAM, &cmd);
			goto _VD_VE1;
		}
		break;

		case VENDOR_VIDEOENC_PARAM_OUT_COMM_SRCOUT: {
			VENDOR_VIDEOENC_COMM_SRCOUT* p_user = (VENDOR_VIDEOENC_COMM_SRCOUT*)p_param;
			UINT32 b_comm_srcout_en = p_user->b_enable;
			UINT32 b_comm_srcout_no_jpg = p_user->b_no_jpeg_enc;
			UINTPTR comm_srcout_addr = p_user->addr;
			UINT32 comm_srcout_size = p_user->size;

			if(!(out_id >= HD_OUT_BASE && out_id <= HD_OUT_MAX)) { return HD_ERR_IO; }
			_HD_CONVERT_OUT_ID(out_id, rv);     if (rv != HD_OK) { return rv; }

			_hd_videoenc_kflow_param_set_by_vendor(self_id, out_id, VDOENC_PARAM_COMM_SRCOUT_ENABLE, (ULONG)b_comm_srcout_en);
			_hd_videoenc_kflow_param_set_by_vendor(self_id, out_id, VDOENC_PARAM_COMM_SRCOUT_NO_JPG_ENC, (ULONG)b_comm_srcout_no_jpg);
			_hd_videoenc_kflow_param_set_by_vendor(self_id, out_id, VDOENC_PARAM_COMM_SRCOUT_ADDR, (ULONG)comm_srcout_addr);
			_hd_videoenc_kflow_param_set_by_vendor(self_id, out_id, VDOENC_PARAM_COMM_SRCOUT_SIZE, (ULONG)comm_srcout_size);

			return HD_OK;
		}
		break;

		case VENDOR_VIDEOENC_PARAM_OUT_TIMER_TRIG_COMP: {
			VENDOR_VIDEOENC_TIMER_TRIG_COMP* p_user = (VENDOR_VIDEOENC_TIMER_TRIG_COMP*)p_param;
			UINT32 b_timer_trig_comp_en = p_user->b_enable;

			if(!(out_id >= HD_OUT_BASE && out_id <= HD_OUT_MAX)) { return HD_ERR_IO; }
			_HD_CONVERT_OUT_ID(out_id, rv);     if (rv != HD_OK) { return rv; }

			_hd_videoenc_kflow_param_set_by_vendor(self_id, out_id, VDOENC_PARAM_TIMER_TRIG_COMP_ENABLE, (ULONG)b_timer_trig_comp_en);

			return HD_OK;
		}
		break;

		case VENDOR_VIDEOENC_PARAM_OUT_REQ_TARGET_I: {
			VENDOR_VIDEOENC_REQ_TARGET_I* p_user = (VENDOR_VIDEOENC_REQ_TARGET_I *)p_param;
			NMI_VDOENC_REQ_TARGET_I_INFO req_i = {0};

			if(!(out_id >= HD_OUT_BASE && out_id <= HD_OUT_MAX)) { return HD_ERR_IO; }
			_HD_CONVERT_OUT_ID(out_id, rv);     if (rv != HD_OK) { return rv; }

			req_i.enable = p_user->enable;
			req_i.target_timestamp = p_user->target_timestamp;

			cmd.dest = ISF_PORT(self_id, out_id);
			cmd.param = VDOENC_PARAM_REQ_TARGET_I;
			cmd.value = (ULONG)&req_i;
			cmd.size = sizeof(NMI_VDOENC_REQ_TARGET_I_INFO);
			r = ISF_IOCTL(isf_fd, ISF_FLOW_CMD_SET_PARAM, &cmd);
			goto _VD_VE1;
		}
		break;

		case VENDOR_VIDEOENC_PARAM_OUT_BR_TOLERANCE: {
			VENDOR_VIDEOENC_BR_TOLERANCE* p_user = (VENDOR_VIDEOENC_BR_TOLERANCE*)p_param;
			UINT32 br_tolerance = p_user->br_tolerance;

			if (br_tolerance < 0 || br_tolerance > 6) {
				printf("invalid br_tolerance (%lu), should be 0~6\n", br_tolerance);
				return HD_ERR_PARAM;
			}

			if(!(out_id >= HD_OUT_BASE && out_id <= HD_OUT_MAX)) { return HD_ERR_IO; }
			_HD_CONVERT_OUT_ID(out_id, rv);     if (rv != HD_OK) { return rv; }

			return _hd_videoenc_kflow_param_set_by_vendor(self_id, out_id, VDOENC_PARAM_BR_TOLERANCE, br_tolerance);
		}
		break;

		case VENDOR_VIDEOENC_PARAM_OUT_JPG_COLOR_TO_GRAY: {
			if(!(out_id >= HD_OUT_BASE && out_id <= HD_OUT_MAX)) { return HD_ERR_IO; }
			_HD_CONVERT_OUT_ID(out_id, rv); 	if (rv != HD_OK) { return rv; }

			return _hd_videoenc_kflow_param_set_by_vendor(self_id, out_id, VDOENC_PARAM_JPG_COLOR_TO_GRAY, (ULONG)p_param);
		}
		break;

		case VENDOR_VIDEOENC_PARAM_OUT_H26X_ENC_GDR: {
			VENDOR_VIDEOENC_H26X_ENC_GDR* p_user_vendor = (VENDOR_VIDEOENC_H26X_ENC_GDR *)p_param;
			HD_H26XENC_GDR* p_user_hd = (HD_H26XENC_GDR *)p_param;	// VENDOR_VIDEOENC_H26X_ENC_GDR cast to HD_H26XENC_GDR

			if (p_user_vendor->enable> 1) {
				printf("invalid h26x_enc_gdr enable (%u), should be 0~1\n", p_user_vendor->enable);
				return HD_ERR_PARAM;
			}

			if (p_user_vendor->period > 0xFFFFFFFF) {
				printf("invalid h26x_enc_gdr period (%u), should be 0~0xFFFFFFFF\n", p_user_vendor->period);
				return HD_ERR_PARAM;
			}

			if (p_user_vendor->enable_gdr_i_frm > 1) {
				printf("invalid h26x_enc_gdr enable_gdr_i_frm (%u), should be 0~1\n", p_user_vendor->enable_gdr_i_frm);
				return HD_ERR_PARAM;
			}

			if (p_user_vendor->enable_gdr_qp > 1) {
				printf("invalid h26x_enc_gdr enable_gdr_qp (%u), should be 0~1\n", p_user_vendor->enable_gdr_qp);
				return HD_ERR_PARAM;
			}

			if (p_user_vendor->gdr_qp > 51) {
				printf("invalid h26x_enc_gdr gdr_qp (%u), should be 0~51\n", p_user_vendor->gdr_qp);
				return HD_ERR_PARAM;
			}

			rv = hd_videoenc_set(path_id, HD_VIDEOENC_PARAM_OUT_ENC_GDR, p_user_hd);
			if (rv != HD_OK) { return rv; }

			if(!(out_id >= HD_OUT_BASE && out_id <= HD_OUT_MAX)) { return HD_ERR_IO; }
			_HD_CONVERT_OUT_ID(out_id, rv); 	if (rv != HD_OK) { return rv; }

			return _hd_videoenc_kflow_param_set_by_vendor(self_id, out_id, VDOENC_PARAM_GDR, (ULONG)p_user_vendor);
		}
		break;

		case VENDOR_VIDEOENC_PARAM_OUT_TILE_SPLIT_MODE: {
			VENDOR_VIDEOENC_TILE_SPLIT_MODE_CFG* p_user = (VENDOR_VIDEOENC_TILE_SPLIT_MODE_CFG*)p_param;
			UINT32 tile_split_mode = p_user->mode;

			if (tile_split_mode >= VENDOR_VIDEOENC_TILE_SPLIT_MODE_MAX) {
				printf("invalid tile_split_mode (%lu)\n", tile_split_mode);
				return HD_ERR_PARAM;
			}

			if(!(out_id >= HD_OUT_BASE && out_id <= HD_OUT_MAX)) { return HD_ERR_IO; }
			_HD_CONVERT_OUT_ID(out_id, rv); 	if (rv != HD_OK) { return rv; }

			return _hd_videoenc_kflow_param_set_by_vendor(self_id, out_id, VDOENC_PARAM_TILE_SPLIT_MODE, (ULONG)tile_split_mode);;
		}
		break;

		case VENDOR_VIDEOENC_PARAM_OUT_H26X_LOW_POWER: {
			VENDOR_VIDEOENC_H26X_LOW_POWER_CFG *p_user = (VENDOR_VIDEOENC_H26X_LOW_POWER_CFG *)p_param;
			NMI_VDOENC_LOW_POWER lpm = {0};

			if(!(out_id >= HD_OUT_BASE && out_id <= HD_OUT_MAX)) { return HD_ERR_IO; }
			_HD_CONVERT_OUT_ID(out_id, rv);     if (rv != HD_OK) { return rv; }

			lpm.enable = p_user->b_enable;
			lpm.mode = p_user->mode;
			return _hd_videoenc_kflow_param_set_by_vendor(self_id, out_id, VDOENC_PARAM_H26X_LOW_POWER, (ULONG)(&lpm));
		}
		break;

		case VENDOR_VIDEOENC_PARAM_OUT_JPG_UVC_SLICE_ENC: {
			VENDOR_VIDEOENC_JPG_UVC_SLICE_ENC_CFG* p_user = (VENDOR_VIDEOENC_JPG_UVC_SLICE_ENC_CFG*)p_param;
			UINT32 jpg_uvc_slice_enc = p_user->enable;
			if(!(out_id >= HD_OUT_BASE && out_id <= HD_OUT_MAX)) { return HD_ERR_IO; }
			_HD_CONVERT_OUT_ID(out_id, rv); 	if (rv != HD_OK) { return rv; }

			cmd.dest = ISF_PORT(self_id, out_id);
			cmd.param = VDOENC_PARAM_JPG_UVC_SLICE_ENC;
			cmd.value = jpg_uvc_slice_enc;
			cmd.size = 0;
			r = ISF_IOCTL(isf_fd, ISF_FLOW_CMD_SET_PARAM, &cmd);
			goto _VD_VE1;
		}
		break;

		case VENDOR_VIDEOENC_PARAM_META_ALLC: {
			VENDOR_META_ALLOC* p_user_vendor = (VENDOR_META_ALLOC*)p_param;
			NMI_VDOENC_META_ALLOC_INFO* p_meta_info = (NMI_VDOENC_META_ALLOC_INFO*)p_param;

			if (p_user_vendor == NULL) {
				printf("invalid p_user_vendor null\n");
				return HD_ERR_PARAM;
			}

			if (sizeof(p_user_vendor) != sizeof(p_meta_info)) {
				printf("invalid p_meta_info size (%lu), should be (%lu)\n", sizeof(NMI_VDOENC_META_ALLOC_INFO), sizeof(VENDOR_META_ALLOC));
				return HD_ERR_PARAM;
			}

			if(!(out_id >= HD_OUT_BASE && out_id <= HD_OUT_MAX)) { return HD_ERR_IO; }
			_HD_CONVERT_OUT_ID(out_id, rv); 	if (rv != HD_OK) { return rv; }

			return _hd_videoenc_kflow_param_set_by_vendor(self_id, out_id, VDOENC_PARAM_META_ALLOC, (ULONG)p_user_vendor);
		}
		break;

		case VENDOR_VIDEOENC_PARAM_OUT_H26X_MAQ_DIFF: {
			VENDOR_VIDEOENC_H26X_MAQ_DIFF* p_user = (VENDOR_VIDEOENC_H26X_MAQ_DIFF*)p_param;
			UINT32 maq_diff_en = p_user->b_enable;
			UINT32 str		   = p_user->str;
			UINT32 start_idx   = p_user->start_idx;
			UINT32 end_idx	   = p_user->end_idx;

			if (p_user->b_enable > 1) {
				printf("invalid maq_diff_en (%lu), should be 0~1\n", p_user->b_enable);
				return HD_ERR_PARAM;
			}

			if (p_user->str < -15 || p_user->str > 15) {
				printf("invalid maq_diff str (%ld), should be -15~15\n", p_user->str);
				return HD_ERR_PARAM;
			}

			if (p_user->start_idx > 10) {
				printf("invalid maq_diff start_idx (%lu), should be 0~10\n", p_user->start_idx);
				return HD_ERR_PARAM;
			}

			if (p_user->end_idx > 10) {
				printf("invalid maq_diff end_idx (%lu), should be 0~10\n", p_user->end_idx);
				return HD_ERR_PARAM;
			}

			if (p_user->start_idx >= p_user->end_idx) {
				printf("invalid maq_diff start_idx (%lu), should be less than end_idx (%lu)\n", p_user->start_idx, p_user->end_idx);
				return HD_ERR_PARAM;
			}

			if(!(out_id >= HD_OUT_BASE && out_id <= HD_OUT_MAX)) { return HD_ERR_IO; }
			_HD_CONVERT_OUT_ID(out_id, rv); 	if (rv != HD_OK) { return rv; }

			_hd_videoenc_kflow_param_set_by_vendor(self_id, out_id, VDOENC_PARAM_H26X_MAQ_DIFF_ENABLE, maq_diff_en);
			_hd_videoenc_kflow_param_set_by_vendor(self_id, out_id, VDOENC_PARAM_H26X_MAQ_DIFF_STR,    str);
			_hd_videoenc_kflow_param_set_by_vendor(self_id, out_id, VDOENC_PARAM_H26X_MAQ_DIFF_START_IDX, start_idx);
			_hd_videoenc_kflow_param_set_by_vendor(self_id, out_id, VDOENC_PARAM_H26X_MAQ_DIFF_END_IDX,   end_idx);

			return HD_OK;
		}
		break;

		case VENDOR_VIDEOENC_PARAM_OUT_USER_TILE_CTRL: {
			if(!(out_id >= HD_OUT_BASE && out_id <= HD_OUT_MAX)) { return HD_ERR_IO; }
			_HD_CONVERT_OUT_ID(out_id, rv);     if (rv != HD_OK) { return rv; }

			return _hd_videoenc_kflow_param_set_by_vendor(self_id, out_id, VDOENC_PARAM_USER_TILE_CTRL, (ULONG)p_param);
		}
		break;

		case VENDOR_VIDEOENC_PARAM_OUT_USER_DATA: {
			if(!(out_id >= HD_OUT_BASE && out_id <= HD_OUT_MAX)) { return HD_ERR_IO; }
			_HD_CONVERT_OUT_ID(out_id, rv);     if (rv != HD_OK) { return rv; }

			return _hd_videoenc_kflow_param_set_by_vendor(self_id, out_id, VDOENC_PARAM_USER_DATA, (ULONG)p_param);
		}
		break;

		case VENDOR_VIDEOENC_PARAM_OUT_BG_RDO: {
			VENDOR_VIDEOENC_BG_RDO_CFG* p_user = (VENDOR_VIDEOENC_BG_RDO_CFG*)p_param;

			if (p_user->b_enable > 1) {
				printf("invalid rdo bg b_enable (%lu), should be 0~1\n", p_user->b_enable);
				return HD_ERR_PARAM;
			}

			if (p_user->avc_bg_skip_bias > 31) {
				printf("invalid rdo bg avc_bg_skip_bias (%lu), should be 0~31\n", p_user->avc_bg_skip_bias);
				return HD_ERR_PARAM;
			}

			if (p_user->hevc_bg_skip_bias < -16 || p_user->hevc_bg_skip_bias > 15) {
				printf("invalid rdo bg hevc_bg_skip_bias (%ld), should be -16~15\n", p_user->hevc_bg_skip_bias);
				return HD_ERR_PARAM;
			}

			if (p_user->hevc_bg_merge_bias < -16 || p_user->hevc_bg_merge_bias > 15) {
				printf("invalid rdo bg hevc_bg_merge_bias (%ld), should be -16~15\n", p_user->hevc_bg_merge_bias);
				return HD_ERR_PARAM;
			}

			if (p_user->bg_bias_shift > 3) {
				printf("invalid rdo bg bg_bias_shift (%lu), should be 0~3\n", p_user->bg_bias_shift);
				return HD_ERR_PARAM;
			}

			if (p_user->mode > 1) {
				printf("invalid rdo bg mode (%lu), should be 0~1\n", p_user->mode);
				return HD_ERR_PARAM;
			}

			if(!(out_id >= HD_OUT_BASE && out_id <= HD_OUT_MAX)) { return HD_ERR_IO; }
			_HD_CONVERT_OUT_ID(out_id, rv);     if (rv != HD_OK) { return rv; }

			return _hd_videoenc_kflow_param_set_by_vendor(self_id, out_id, VDOENC_PARAM_BG_RDO, (ULONG)p_param);
		}
		break;

		case VENDOR_VIDEOENC_PARAM_OUT_H26X_SLICE_SPLIT: {
			VENDOR_VIDEOENC_H26X_SLICE_SPLIT_CFG* p_user_vendor = (VENDOR_VIDEOENC_H26X_SLICE_SPLIT_CFG *)p_param;
			HD_H26XENC_SLICE_SPLIT* p_user_hd = (HD_H26XENC_SLICE_SPLIT *)p_param;	// VENDOR_VIDEOENC_H26X_SLICE_SPLIT_CFG cast to HD_H26XENC_SLICE_SPLIT
			BOOL b_slice_interrupt = p_user_vendor->enable_slice_bs;

			if (p_user_vendor->enable > 1) {
				printf("invalid slice split enable (%lu), should be 0~1\n", p_user_vendor->enable);
				return HD_ERR_PARAM;
			}

			if (p_user_vendor->slice_mode > 1) {
				printf("invalid slice split slice_mode (%lu), should be 0~1\n", p_user_vendor->slice_mode);
				return HD_ERR_PARAM;
			}

			if (p_user_vendor->slice_mode == 1) {
				if (p_user_vendor->slice_i_idx > 3) {
					printf("invalid slice split slice_i_idx (%lu), should be 0~3\n", p_user_vendor->slice_i_idx);
					return HD_ERR_PARAM;
				}
			}

			rv = hd_videoenc_set(path_id, HD_VIDEOENC_PARAM_OUT_SLICE_SPLIT, p_user_hd);
			if (rv != HD_OK) { return rv; }

			if(!(out_id >= HD_OUT_BASE && out_id <= HD_OUT_MAX)) { return HD_ERR_IO; }
			_HD_CONVERT_OUT_ID(out_id, rv); 	if (rv != HD_OK) { return rv; }

			_hd_videoenc_kflow_param_set_by_vendor(self_id, out_id, VDOENC_PARAM_SLICE_SPLIT, (ULONG)p_user_vendor);
			_hd_videoenc_kflow_param_set_by_vendor(self_id, out_id, VDOENC_PARAM_SLICE_INTERRUPT, (ULONG)b_slice_interrupt);
		}
		break;

		case VENDOR_VIDEOENC_PARAM_OUT_WATERMARK: {
			VENDOR_VIDEOENC_WATERMARK *p_user = (VENDOR_VIDEOENC_WATERMARK *)p_param;
			NMI_VDOENC_WATERMARK watermark_info = {0};
			UINT32 i = 0;

			if(!(out_id >= HD_OUT_BASE && out_id <= HD_OUT_MAX)) { return HD_ERR_IO; }
			_HD_CONVERT_OUT_ID(out_id, rv);     if (rv != HD_OK) { return rv; }

			for (i = 0; i < NMI_VDOENC_WATERMARK_COUNT; i++) {
				watermark_info.enable[i]   = p_user->watermark[i].enable;
				watermark_info.addr[i]     = p_user->watermark[i].addr;
				watermark_info.len[i]      = p_user->watermark[i].len;
				watermark_info.uv[i]       = p_user->watermark[i].uv;
				watermark_info.str[i]      = p_user->watermark[i].str;
				watermark_info.left_top[i] = p_user->watermark[i].left_top;
			}
			return _hd_videoenc_kflow_param_set_by_vendor(self_id, out_id, VDOENC_PARAM_WATERMARK, (ULONG)(&watermark_info));
		}
		break;

		case VENDOR_VIDEOENC_PARAM_OUT_EMBEDDED_MD: {
			VENDOR_VIDEOENC_EMBEDDED_MD_CFG* p_user = (VENDOR_VIDEOENC_EMBEDDED_MD_CFG*)p_param;

			if (p_user->b_md_enable > 1) {
				printf("invalid embedded md b_md_enable (%lu), should be 0~1\n", p_user->b_md_enable);
				return HD_ERR_PARAM;
			}

			if (p_user->md_delta_qp < -16 || p_user->md_delta_qp > 15) {
				printf("invalid embedded md md_delta_qp (%ld), should be -16~15\n", p_user->md_delta_qp);
				return HD_ERR_PARAM;
			}

			if (p_user->md_user_offset < -16 || p_user->md_user_offset > 15) {
				printf("invalid embedded md md_user_offset (%ld), should be -16~15\n", p_user->md_user_offset);
				return HD_ERR_PARAM;
			}

			if (p_user->md_boundary_thr > 1000) {
				printf("invalid embedded md md_boundary_thr (%lu), should be 0~1000\n", p_user->md_boundary_thr);
				return HD_ERR_PARAM;
			}

			if (p_user->b_motion_filter_enable > 1) {
				printf("invalid embedded md b_motion_filter_enable (%lu), should be 0~1\n", p_user->b_motion_filter_enable);
				return HD_ERR_PARAM;
			}

			if (p_user->motion_filter_thr > 7) {
				printf("invalid embedded md motion_filter_thr (%lu), should be 0~7\n", p_user->motion_filter_thr);
				return HD_ERR_PARAM;
			}

			if (p_user->motion_filter_delta_qp < -16 || p_user->motion_filter_delta_qp > 15) {
				printf("invalid embedded md motion_filter_delta_qp (%ld), should be -16~15\n", p_user->motion_filter_delta_qp);
				return HD_ERR_PARAM;
			}
	
			if(!(out_id >= HD_OUT_BASE && out_id <= HD_OUT_MAX)) { return HD_ERR_IO; }
			_HD_CONVERT_OUT_ID(out_id, rv);     if (rv != HD_OK) { return rv; }

			return _hd_videoenc_kflow_param_set_by_vendor(self_id, out_id, VDOENC_PARAM_EMBEDDED_MD, (ULONG)p_param);
		}
		break;

		case VENDOR_VIDEOENC_PARAM_SMART_ROI: {
			VENDOR_VIDEOENC_SMART_ROI* p_user = (VENDOR_VIDEOENC_SMART_ROI*)p_param;
			UINT32 i = 0;

			if (p_user->enable > 1) {
				printf("invalid smart roi enable (%lu), should be 0~1\n", p_user->enable);
				return HD_ERR_PARAM;
			}

			for (i = 0; i < VENDOR_VIDEOENC_SMART_ROI_CLASS_MAX; i++) {
				if (p_user->fg_str[i] > 255) {
					printf("invalid smart roi fg_str[VENDOR_VIDEOENC_SMART_ROI_CLASS%lu] (%lu), should be 255\n", i, p_user->fg_str[i]);
					return HD_ERR_PARAM;
				}
			}

			if (p_user->mode > 1 && p_user->mode < 0xFF) {//0xFF for AI demo
				printf("invalid smart roi mode (%lu), should be 0~1\n", p_user->mode);
				return HD_ERR_PARAM;
			}

			if(!(out_id >= HD_OUT_BASE && out_id <= HD_OUT_MAX)) { return HD_ERR_IO; }
			_HD_CONVERT_OUT_ID(out_id, rv);     if (rv != HD_OK) { return rv; }

			return _hd_videoenc_kflow_param_set_by_vendor(self_id, out_id, VDOENC_PARAM_SMART_ROI, (ULONG)p_param);
		}
		break;

		case VENDOR_VIDEOENC_PARAM_SMART_BBOX: {
			VENDOR_VIDEOENC_SMART_BBOX* p_user = (VENDOR_VIDEOENC_SMART_BBOX*)p_param;
			NMI_VDOENC_SMART_BBOX smart_bbox_info = {0};
			UINT32 i = 0;

			if (p_user->bbox_num > VENDOR_VIDEOENC_SMART_BBOX_MAX_NUM) {
				printf("invalid smart bbox_num enable (%lu), should be 0~%lu\n", p_user->bbox_num, VENDOR_VIDEOENC_SMART_BBOX_MAX_NUM);
				return HD_ERR_PARAM;
			}

			smart_bbox_info.base_width = p_user->base_resolution.w;
			smart_bbox_info.base_height = p_user->base_resolution.h;
			smart_bbox_info.bbox_num = p_user->bbox_num;
			for (i = 0; i < smart_bbox_info.bbox_num; i++) {
				smart_bbox_info.bbox[i].position[0].x = p_user->bbox[i].positions[0].x;
				smart_bbox_info.bbox[i].position[0].y = p_user->bbox[i].positions[0].y;
				smart_bbox_info.bbox[i].position[1].x = p_user->bbox[i].positions[1].x;
				smart_bbox_info.bbox[i].position[1].y = p_user->bbox[i].positions[1].y;
				smart_bbox_info.bbox[i].class_id = p_user->bbox[i].class_id;
			}
			smart_bbox_info.timestamp = p_user->timestamp;

			if(!(out_id >= HD_OUT_BASE && out_id <= HD_OUT_MAX)) { return HD_ERR_IO; }
			_HD_CONVERT_OUT_ID(out_id, rv);     if (rv != HD_OK) { return rv; }

			cmd.dest = ISF_PORT(self_id, out_id);
			cmd.param = VDOENC_PARAM_SMART_BBOX;
			cmd.value = (ULONG)&smart_bbox_info;
			cmd.size = sizeof(NMI_VDOENC_SMART_BBOX);
			r = ISF_IOCTL(isf_fd, ISF_FLOW_CMD_SET_PARAM, &cmd);
		}
		break;

		case VENDOR_VIDEOENC_PARAM_DYNAMIC_FR: {
			VENDOR_VIDEOENC_DYNAMIC_FR_CFG* p_user = (VENDOR_VIDEOENC_DYNAMIC_FR_CFG*)p_param;
			NMI_VDOENC_DYNAMIC_FR dynamic_fr_info = {0};

			if (p_user->enable > 1) {
				printf("invalid dynamic fr enable (%lu), should be 0~1\n", p_user->enable);
				return HD_ERR_PARAM;
			}

			if (p_user->motion_sensitivity > 15) {
				printf("invalid dynamic fr motion_sensitivity (%lu), should be 0~15\n", p_user->motion_sensitivity);
				return HD_ERR_PARAM;
			}

			dynamic_fr_info.enable             = p_user->enable;
			dynamic_fr_info.min_fps            = p_user->min_fps;
			dynamic_fr_info.motion_sensitivity = p_user->motion_sensitivity;

			if(!(out_id >= HD_OUT_BASE && out_id <= HD_OUT_MAX)) { return HD_ERR_IO; }
			_HD_CONVERT_OUT_ID(out_id, rv);     if (rv != HD_OK) { return rv; }

			cmd.dest = ISF_PORT(self_id, out_id);
			cmd.param = VDOENC_PARAM_DYNAMIC_FR;
			cmd.value = (ULONG)&dynamic_fr_info;
			cmd.size = sizeof(NMI_VDOENC_DYNAMIC_FR);
			r = ISF_IOCTL(isf_fd, ISF_FLOW_CMD_SET_PARAM, &cmd);
		}
		break;

		case VENDOR_VIDEOENC_PARAM_DYNAMIC_GOP: {
			VENDOR_VIDEOENC_DYNAMIC_GOP_CFG* p_user = (VENDOR_VIDEOENC_DYNAMIC_GOP_CFG*)p_param;
			NMI_VDOENC_DYNAMIC_GOP dynamic_gop_info = {0};

			if (p_user->enable > 1) {
				printf("invalid dynamic gop enable (%lu), should be 0~1\n", p_user->enable);
				return HD_ERR_PARAM;
			}

			if (p_user->max_gop > 4096) {
				printf("invalid dynamic gop max_gop (%lu), should be 0~4096\n", p_user->max_gop);
				return HD_ERR_PARAM;
			}

			if (p_user->motion_sensitivity > 15) {
				printf("invalid dynamic gop motion_sensitivity (%lu), should be 0~15\n", p_user->motion_sensitivity);
				return HD_ERR_PARAM;
			}

			dynamic_gop_info.enable             = p_user->enable;
			dynamic_gop_info.max_gop            = p_user->max_gop;
			dynamic_gop_info.motion_sensitivity = p_user->motion_sensitivity;

			if(!(out_id >= HD_OUT_BASE && out_id <= HD_OUT_MAX)) { return HD_ERR_IO; }
			_HD_CONVERT_OUT_ID(out_id, rv);     if (rv != HD_OK) { return rv; }

			cmd.dest = ISF_PORT(self_id, out_id);
			cmd.param = VDOENC_PARAM_DYNAMIC_GOP;
			cmd.value = (ULONG)&dynamic_gop_info;
			cmd.size = sizeof(NMI_VDOENC_DYNAMIC_GOP);
			r = ISF_IOCTL(isf_fd, ISF_FLOW_CMD_SET_PARAM, &cmd);
		}
		break;

		case VENDOR_VIDEOENC_PARAM_OUT_SEI_SIGNED_VDO: {
			VENDOR_VIDEOENC_SEI_SIGNED_VDO_CFG *p_user = (VENDOR_VIDEOENC_SEI_SIGNED_VDO_CFG *)p_param;
			NMI_VDOENC_SEI_SIGNED_VDO_CFG  sei_signed_vdo_info = {0};
			UINT32 i = 0;

			if (p_user->enable > 1) {
				printf("invalid sei_signed_vdo_en (%lu), should be 0~1\n", p_user->enable);
				return HD_ERR_PARAM;
			}

			if(!(out_id >= HD_OUT_BASE && out_id <= HD_OUT_MAX)) { return HD_ERR_IO; }
			_HD_CONVERT_OUT_ID(out_id, rv);     if (rv != HD_OK) { return rv; }

			sei_signed_vdo_info.enable = p_user->enable;
			for (i = 0; i < NMI_VDOENC_SEI_SIGNED_VDO_ID_MAX; i++) {
				sei_signed_vdo_info.machine_id[i]   = p_user->machine_id[i];
			}
			return _hd_videoenc_kflow_param_set_by_vendor(self_id, out_id, VDOENC_PARAM_SEI_SIGNED_VDO, (ULONG)(&sei_signed_vdo_info));
		}
		break;

		case VENDOR_VIDEOENC_PARAM_OUT_H26X_PRIVACY: {
			VENDOR_VIDEOENC_H26X_PRIVACY_CFG* p_user = (VENDOR_VIDEOENC_H26X_PRIVACY_CFG*)p_param;
			UINT32 b_h26x_privacy = p_user->enable;

			if (b_h26x_privacy > 1) {
				printf("invalid b_h26x_privacy (%lu), should be 0~1\n", b_h26x_privacy);
				return HD_ERR_PARAM;
			}

			if(!(out_id >= HD_OUT_BASE && out_id <= HD_OUT_MAX)) { return HD_ERR_IO; }
			_HD_CONVERT_OUT_ID(out_id, rv); 	if (rv != HD_OK) { return rv; }

			return _hd_videoenc_kflow_param_set_by_vendor(self_id, out_id, VDOENC_PARAM_H26X_PRIVACY, b_h26x_privacy);
		}
		break;

		case VENDOR_VIDEOENC_PARAM_IN_STAMP_QP:
			return set_osg_qp(path_id, p_param);

		case VENDOR_VIDEOENC_PARAM_IN_STAMP_COLOR_INVERT:
			return set_osg_color_invert(path_id, p_param);

		case VENDOR_VIDEOENC_PARAM_IN_STAMP_OVERLAP:
			return set_osg_overlap(path_id, p_param);

		case VENDOR_VIDEOENC_PARAM_IN_STAMP_ATTR:
			return set_encoder_stamp_attr(path_id, p_param);

		case VENDOR_VIDEOENC_PARAM_IN_MASK_ATTR:
			return set_encoder_mask(path_id, p_param);

		case VENDOR_VIDEOENC_PARAM_IN_MOSAIC_ATTR:
			return set_encoder_mosaic(path_id, p_param);

		case VENDOR_VIDEOENC_PARAM_IN_MOSAIC_SIZE:
			return set_encoder_mosaic_size(path_id, p_param);

		case VENDOR_VIDEOENC_PARAM_IN_QP_MOSAIC:
			return set_encoder_qp_mosaic(path_id, (int*)p_param);

		default:
			rv = HD_ERR_PARAM;
			printf("vendor_videoenc_set not support item %d \n", id);
			return rv;
	}

_VD_VE1:
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

HD_RESULT vendor_videoenc_get(HD_PATH_ID path_id, VENDOR_VIDEOENC_PARAM_ID id, VOID *p_param)
{
	HD_DAL self_id = HD_GET_DEV(path_id);
	HD_IO out_id = HD_GET_OUT(path_id);
	HD_RESULT rv = HD_ERR_NG;

	_HD_CONVERT_SELF_ID(self_id, rv); if(rv != HD_OK) {	return rv;}
	_HD_CONVERT_OUT_ID(out_id, rv); if(rv != HD_OK) {	return rv;}

	switch (id) {
		case VENDOR_VIDEOENC_PARAM_OUT_H26X_DESC: {
			VENDOR_VIDEOENC_H26X_DESC_CFG *p_user = (VENDOR_VIDEOENC_H26X_DESC_CFG*)p_param;
			VDOENC_H26X_DESC_INFO desc_info = {0};

			rv = _hd_get_struct_param(self_id, out_id, VDOENC_PARAM_H26X_DESC, (VOID*)&desc_info, sizeof(VDOENC_H26X_DESC_INFO));

			if (rv != HD_OK) { return rv; }

			p_user->vps_paddr = desc_info.vps_paddr;
			p_user->vps_size  = desc_info.vps_size;
			p_user->sps_paddr = desc_info.sps_paddr;
			p_user->sps_size  = desc_info.sps_size;
			p_user->pps_paddr = desc_info.pps_paddr;
			p_user->pps_size  = desc_info.pps_size;
		}
		break;

		default:
			return rv;
	}
	return rv;
}

