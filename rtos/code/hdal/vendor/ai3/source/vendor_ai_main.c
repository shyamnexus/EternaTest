/**
	@brief Source file of vendor user-space net API.

	@file vendor_ai_main.c

	@ingroup vendor_ai_net

	@note Nothing.

	Copyright Novatek Microelectronics Corp. 2018.  All rights reserved.
*/

/*-----------------------------------------------------------------------------*/
/* Include Files                                                               */
/*-----------------------------------------------------------------------------*/
#include <stdio.h>
#include <string.h>
#include "hd_type.h"
#include "hd_common.h"

#include "vendor_common.h"
#include "vendor_ai_comm.h"
#include "vendor_ai_comm_flow.h"
#include "vendor_ai_internal.h"
#include "vendor_ai.h"
#include "vendor_ai_net/vendor_ai_net_flow.h"
#include "vendor_ai_net/vendor_ai_net_debug.h"
#include "vendor_ai_net/vendor_ai_net_cmd.h"
#include "vendor_ai_net/vendor_ai_net_gen.h"
#include "vendor_ai_net/vendor_ai_op.h"
#include "vendor_ai_net/nn_net.h"
#include "kwrap/platform.h"
#include "kwrap/semaphore.h"
#include "vendor_ai_version.h"
#include "kflow_ai_version.h"
#include "kdrv_ai_version.h"

#if defined(__LINUX)
#include <sys/ioctl.h>
#include <sys/time.h>
#endif

//=============================================================
#define __CLASS__ 				"[ai][lib][main]"
#include "vendor_ai_debug.h"
//=============================================================

/*-----------------------------------------------------------------------------*/
/* Extern Global Variables                                                     */
/*-----------------------------------------------------------------------------*/
extern UINT32 *g_is_job_debug_mode;
extern UINT32 g_ai_support_net_max;
extern UINT32 *g_is_aiisp_model;
extern UINT32 max_isp;
extern UINT32 g_real_chip_id;
UINT32 g_debug_mask = 0; //origin debug_value_all
UINT32 g_debug_value = 0;

/*-----------------------------------------------------------------------------*/
/* Extern Function Prototype                                                   */
/*-----------------------------------------------------------------------------*/
extern HD_RESULT _vendor_ai_net_mem_query_full_alloc_size(VENDOR_AIS_FLOW_MEM_PARM *p_model_buf, UINT32 *req_size);
extern HD_RESULT _vendor_ais_get_net_in_out_info_list(VENDOR_AIS_FLOW_MEM_PARM *p_mem, UINT32 *p_in_buf_cnt, UINT32 *p_out_buf_cnt, VENDOR_AI3_BUF_INFO *p_in_buf_info, VENDOR_AI3_BUF_INFO *p_out_buf_info);
extern HD_RESULT _vendor_ai_net_get_float_in_size(VENDOR_AIS_FLOW_MEM_PARM *p_mem, UINT32 *p_ret_size);
extern HD_RESULT _vendor_ai_net_get_float_out_size(VENDOR_AIS_FLOW_MEM_PARM *p_mem, UINT32 *p_ret_size);
extern HD_RESULT _vendor_ai_net_get_gblktile_model_float_out_size(VENDOR_AIS_FLOW_MEM_PARM *p_mem, UINT32 *p_ret_size);
extern HD_RESULT _vendor_ai_net_check_tcm_use(VENDOR_AIS_FLOW_MEM_PARM *p_model, BOOL *is_need_ubuf, UINT32 *need_size);

/*-----------------------------------------------------------------------------*/
/* Local Function Prototype                                                    */
/*-----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------*/
/* Local Global Variables                                                      */
/*-----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------*/
/* Interface Functions                                                         */
/*-----------------------------------------------------------------------------*/

HD_RESULT vendor_ai3_dev_init (VENDOR_AI3_DEV_CFG* p_dev_cfg)
{
	HD_RESULT ret = HD_OK;

	if (p_dev_cfg == NULL) {		DBG_ERR("check p_dev_cfg == NULL !!\r\n");		return HD_ERR_INV;	}

    // config timeline_all
    if(p_dev_cfg->mode & VENDOR_AI3_DEV_TIMELINE)
	{
		VENDOR_AI_PERF_TIMELINE timeline_info = {0};
        timeline_info.enable = 1;
        timeline_info.timeline_func = p_dev_cfg->info[VENDOR_AI3_DEV_TIMELINE_FUNC];
        timeline_info.enable = p_dev_cfg->info[VENDOR_AI3_DEV_TIMELINE_BUFSIZE];
		vendor_ai_cfg_set(VENDOR_AI_CFG_TIMELINE_ALL, &timeline_info);
	}

	// config share model mode 2
	{
		UINT32 share_mode = 2;
		vendor_ai_cfg_set(VENDOR_AI_CFG_SHAREMODEL_MODE, &share_mode);
	}

	// config multi_thread 1
	{
		UINT32 multi_thread = 1;
		vendor_ai_cfg_set(VENDOR_AI_CFG_MULTI_THREAD, &multi_thread);
	}

	ret = vendor_ai_init();
	return ret;
}

HD_RESULT vendor_ai3_dev_uninit (VOID)
{
	HD_RESULT ret = HD_OK;
	ret = vendor_ai_uninit();
	return ret;
}

// general config
HD_RESULT vendor_ai3_dev_set (VENDOR_AI3_CFG_ID cfg_id, void* p_param)
{
	HD_RESULT ret = HD_OK;
	INT32 id = (INT32)cfg_id; 
	if (p_param == NULL) {		DBG_ERR("check p_param == NULL !!\r\n");		return HD_ERR_INV;	}
  
	switch (id)
	{
		case VENDOR_AI3_CFG_VER:
		case VENDOR_AI3_CFG_MODEL_INFO:
			DBG_ERR("cfg_id(%d) is get only, set is NOT supported ...\r\n", (int)cfg_id);
			return HD_ERR_INV;
		case VENDOR_AI3_CFG_ISP_CB:
			{
				uintptr_t *isp_cb = (uintptr_t*) p_param ; 
				ret  = vendor_ais_set_isp_cb(isp_cb)  ;
				if (ret != HD_OK) return ret;
			}
			break;	
		case VENDOR_AI3_CFG_CORE_USAGE_LIMIT:
			{
				UINT32 *p_user = (UINT32 *) p_param;
				if (*p_user > 100){
						DBG_ERR("USAGE_LIMIT setting %u should be in the [0,100] %% \r\n", *p_user);
						return HD_ERR_DEV;
					}
				DBG_IND("set() - USAGE_LIMIT = %u%% \r\n", *p_user);
				ret = vendor_ais_set_usage_limit(*p_user) ; 
				if (ret != HD_OK) return ret;
			}
			break;	
		case VENDOR_AI3_CFG_NOTCHECK_INPUT_ALIGN:
			{
				UINT32 *p_user = (UINT32 *) p_param;
				if (*p_user != 0 && *p_user != 1){
						DBG_ERR("NOTCHECK_INPUT_ALIGN setting %u should be [0/1] \r\n", *p_user);
						return HD_ERR_DEV;
					}
				DBG_IND("NOTCHECK_INPUT_ALIGN = %u \r\n", *p_user);
				ret = vendor_ai_check_input_align_set(*p_user) ; 
				if (ret != HD_OK) return ret;
			}
			break;	
		case VENDOR_AI3_CFG_NOTCHECK_OUTPUT_ALIGN:
			{
				UINT32 *p_user = (UINT32 *) p_param;
				if (*p_user != 0 && *p_user != 1){
						DBG_ERR("NOTCHECK_OUTPUT_ALIGN setting %u should be [0/1] \r\n", *p_user);
						return HD_ERR_DEV;
					}
				DBG_IND("NOTCHECK_OUTPUT_ALIGN = %u \r\n", *p_user);
				ret = vendor_ai_check_output_align_set(*p_user) ; 
				if (ret != HD_OK) return ret;
			}
			break;	
		case 256:
		    {   
				//VENDOR_AI3_CFG_SW_RESET =  256,
				DBG_IND("set() - SW RESET\r\n");
				ret = vendor_ais_sw_reset();
				if (ret != HD_OK) return ret;
			}
			break;
		default:
			DBG_ERR("unsupport cfg_id(%d) ...\r\n", (int)cfg_id);
			return HD_ERR_INV;
	}
	return HD_OK;
}

HD_RESULT vendor_ai3_dev_get (VENDOR_AI3_CFG_ID cfg_id, void* p_param)
{
	HD_RESULT ret = HD_OK;

	if (p_param == NULL) {		DBG_ERR("check p_param == NULL !!\r\n");		return HD_ERR_INV;	}

	switch (cfg_id)
	{
		case VENDOR_AI3_CFG_VER: 
			{
				VENDOR_AI3_VER *p_user = (VENDOR_AI3_VER *)p_param;

				ret = vendor_ai_cfg_get(VENDOR_AI_CFG_LIB_IMPL_VERSION, p_user->vendor_ai_impl_version);
				if (ret != HD_OK) return ret;

				ret = vendor_ai_cfg_get(VENDOR_AI_CFG_KFLOW_IMPL_VERSION, p_user->kflow_ai_impl_version);
				if (ret != HD_OK) return ret;

				ret = vendor_ai_cfg_get(VENDOR_AI_CFG_KDRV_IMPL_VERSION,  p_user->kdrv_ai_impl_version);
				if (ret != HD_OK) return ret;
			}
			break;
		case VENDOR_AI3_CFG_VER2: 
			{
				VENDOR_AI3_VER2 *p_user = (VENDOR_AI3_VER2 *)p_param;

				ret = vendor_ai_cfg_get(VENDOR_AI_CFG_LIB_IMPL_VERSION, p_user->vendor_ai_impl_version);
				if (ret != HD_OK) return ret;

				ret = vendor_ai_cfg_get(VENDOR_AI_CFG_KFLOW_IMPL_VERSION, p_user->kflow_ai_impl_version);
				if (ret != HD_OK) return ret;

				ret = vendor_ai_cfg_get(VENDOR_AI_CFG_KFLOW_ISP_IMPL_VERSION, p_user->kflow_ai_isp_impl_version);
				if (ret != HD_OK) return ret;

				ret = vendor_ai_cfg_get(VENDOR_AI_CFG_KDRV_IMPL_VERSION,  p_user->kdrv_ai_impl_version);
				if (ret != HD_OK) return ret;
			}
			break;


		case VENDOR_AI3_CFG_MODEL_INFO:
			{
				VENDOR_AI3_MODEL_INFO *p_user = (VENDOR_AI3_MODEL_INFO *)p_param;
				VENDOR_AIS_FLOW_MAP_MEM_PARM mem_manager_tmp = {0};
				NN_GEN_NET_INFO net_info = {0};
				BOOL b_is_jmisp_model = FALSE;
				BOOL b_is_gblktile_model = FALSE;
				BOOL is_must_ubuf = FALSE;

				if (p_user->model_buf.pa == 0 || p_user->model_buf.va == 0 || p_user->model_buf.size == 0) {
					DBG_ERR("check model_buf fail !! model_buf (pa/va/size) = (0x%016lx, 0x%016lx, %d)\r\n", p_user->model_buf.pa, p_user->model_buf.va, (int)p_user->model_buf.size);
					return HD_ERR_INV;
				}

				// check if this is NN30 model & chip_id check
				if (HD_OK != vendor_ais_net_gen_chk_vers((VENDOR_AIS_FLOW_MEM_PARM *)&p_user->model_buf, 0)) {
					DBG_ERR("check NN30 nvt_model fail !!\r\n");
					return HD_ERR_INV;
				}

				// check if this is jmisp model
				{
					vendor_ais_get_net_info(&net_info, p_user->model_buf.va);
					b_is_jmisp_model = (net_info.p_head->jmisp_info_size > 0)? TRUE:FALSE;
					b_is_gblktile_model = (net_info.p_head->gblktile_info_size > 0)? TRUE:FALSE;
					
					// assign opmode
					p_user->mode = MODEL_TYPE_ATTR_GET(net_info.p_head->model_type, MODEL_TYPE_ATTR_OPMODE);
				
					if (TRUE == b_is_gblktile_model) {
						GBLKTILE_INFO_HEADER * gblktile_header = (GBLKTILE_INFO_HEADER *)net_info.p_gblktile_header_blk;
						p_user->type = AI3_GTILE_MODEL;
						p_user->config[MODEL_INFO_CORE_NUM] = gblktile_header->core_num;
						p_user->config[MODEL_INFO_UBUF_NUM] = gblktile_header->total_ubuf_size;
					}else if(TRUE == b_is_jmisp_model){
						AI_JMISP_INFO_HEADER * jmisp_header = (AI_JMISP_INFO_HEADER *)net_info.p_jmisp_header_blk;
						p_user->type = AI3_AIISP_MODEL;
						p_user->config[MODEL_INFO_CORE_NUM] = jmisp_header->core_num;
						p_user->config[MODEL_INFO_UBUF_NUM] = jmisp_header->total_ubuf_size;
					}else{
						p_user->type = AI3_COMMON_MODEL;
						p_user->config[MODEL_INFO_CORE_NUM] = 1;
						if (MODEL_TYPE_ATTR_GET(net_info.p_head->model_type, MODEL_TYPE_ATTR_USE_TILING) == 2) is_must_ubuf = TRUE;
					}					
				}

				//--- (1) get RONLY size ---
				vendor_ais_auto_alloc_mem((VENDOR_AIS_FLOW_MEM_PARM *)&p_user->model_buf, &mem_manager_tmp);  // we only need user_parm 
#if FLOW_USE_KERNEL
				p_user->proc_mem.buf[AI3_PROC_BUF_RONLYBUF].size  = ALIGN_CEIL_64(mem_manager_tmp.user_parm.size) * 2;  // user_parm + kerl_parm , EXCLUDE group buf ( we won't do group alg on sdk now )
#else 
				p_user->proc_mem.buf[AI3_PROC_BUF_RONLYBUF].size  = ALIGN_CEIL_64(mem_manager_tmp.user_parm.size);  // only user_parm, EXCLUDE group buf ( we won't do group alg on sdk now ) 
#endif


				//--- (2) get WORKBUF size ---
				// (2-1) get WORKBUF (iobuf size)
				//    => if use iobuf size on model header which doesn't count result buffer => zero-copy function will actually "illegal cross" iobuf size to analyze buffer offset, and will randomly cause system crash on any random place.
				//    => Modify here to check all fixed buffer (include result buffer) required size to avoid !!
				{
					UINT32 req_size = 0;
					UINT32 jmisp_info_buf_size = 0 ; 
					if ((p_user->ctrl & CTRL_BUF_FLOATOUT) && (FALSE == b_is_gblktile_model)) {
						//find the largest iomem offset for float out
						ret = _vendor_ai_net_get_float_out_size((VENDOR_AIS_FLOW_MEM_PARM *)&p_user->model_buf, &req_size); 
						if (ret != HD_OK) {
							DBG_ERR("query float out size failed ...\r\n");
							return ret;
						}
					}else {
						// use model header size
						req_size = ALIGN_CEIL_64(mem_manager_tmp.user_buff.size);
					}

					if(TRUE == b_is_jmisp_model){
						VENDOR_AIS_FLOW_MEM_PARM p_jmisp_info = {0} ;
						uintptr_t jmisp_info_offset = net_info.p_jmisp_info_blk - p_user->model_buf.va;
						p_jmisp_info.va = p_user->model_buf.va + jmisp_info_offset;
						p_jmisp_info.pa = p_user->model_buf.pa + jmisp_info_offset;
						p_jmisp_info.size = net_info.p_head->jmisp_info_size;
						// if it is frame mode, get workbuf_size from jmisp_info
						// else use model header size
						ret = vendor_ais_get_workbuf_size_from_jmisp_info(&p_jmisp_info, &jmisp_info_buf_size);
						if (ret != HD_OK) {
							DBG_ERR("query workbuf size from jmisp_info failed ...\r\n");
							return ret;
						}
					}

					if (TRUE == b_is_jmisp_model && jmisp_info_buf_size > 0 ){
						// if it is jmisp_model frame mode, get workbuf_size from jmisp_info
						// else use model header size
							req_size = ALIGN_CEIL_64(jmisp_info_buf_size);
							req_size += 64; //for dummy cmd buffer
					}else {
						req_size += 64; //for dummy cmd buffer
						if (req_size < mem_manager_tmp.user_buff.size) {
							DBG_ERR("req_size(%d) should >= model header iobuf size(%d), something wrong ... !!\r\n", req_size, mem_manager_tmp.user_buff.size);
							return HD_ERR_FAIL;
						}
					}
					
					p_user->proc_mem.buf[AI3_PROC_BUF_WORKBUF].size = req_size;
					DBG_IND("get() - IOBUF SIZE = %u \r\n", req_size);
				}

				// check if debug mode => re-write iobuf need size to FULL alloc size
				if (p_user->ctrl & CTRL_BUF_DEBUG) {
					UINT32 req_size = 0;
					ret = _vendor_ai_net_mem_query_full_alloc_size((VENDOR_AIS_FLOW_MEM_PARM *)&p_user->model_buf, &req_size);
					if (ret != HD_OK) {
						DBG_ERR("query full alloc size failed ...\r\n");
						return ret;
					}
					req_size += 64; //for dummy cmd buffer
					p_user->proc_mem.buf[AI3_PROC_BUF_WORKBUF].size = ALIGN_CEIL_64(req_size);  // debug mdoe, return FULL alloc size (bufopt = 0)
					DBG_IND("get() - IOBUF SIZE (rewrite to debug mode) = %u \r\n", req_size);
				}

				// (2-2) get WORKBUF (float_in size)
				if(p_user->ctrl & CTRL_BUF_FLOATIN){
					UINT32 req_size = 0;
					ret = _vendor_ai_net_get_float_in_size((VENDOR_AIS_FLOW_MEM_PARM *)&p_user->model_buf, &req_size);
					if (ret != HD_OK) {
						DBG_ERR("query float in size failed ...\r\n");
						return ret;
					}
					p_user->proc_mem.buf[AI3_PROC_BUF_WORKBUF].size += req_size;
					DBG_IND("get() - FLOAT_IN_BUF = %u \r\n", req_size);
				}

				// (2-2-1) gblktile_model get WORKBUF (float_out size)
				if(p_user->ctrl & CTRL_BUF_FLOATOUT && b_is_gblktile_model){
					UINT32 req_size = 0;
					ret = _vendor_ai_net_get_gblktile_model_float_out_size((VENDOR_AIS_FLOW_MEM_PARM *)&p_user->model_buf, &req_size);
					if (ret != HD_OK) {
						DBG_ERR("query gblktile_model_float_out_size failed ...\r\n");
						return ret;
					}
					p_user->proc_mem.buf[AI3_PROC_BUF_WORKBUF].size += req_size;
					DBG_IND("get() - gblktile_model FLOAT_OUT_BUF = %u \r\n", req_size);
				}

                // (2-3) get WORKBUF (float_out cmd size)
				if(p_user->ctrl & CTRL_BUF_FLOATIN || p_user->ctrl & CTRL_BUF_FLOATOUT){
					UINT32 req_size2 = 0;
					ret = vendor_ai3_op_get_cal_cmd_size(&req_size2);
					if (ret != HD_OK) {
						DBG_ERR("query float out cmd size failed ...\r\n");
						return ret;
					}
					p_user->proc_mem.buf[AI3_PROC_BUF_WORKBUF].size += req_size2;
					DBG_IND("get() - FLOAT_CMD_BUF = %u \r\n", req_size2);
				}

				// (2-4) get WORKBUF (fake ubuf)
				{
					// only normal model will use "fake ubuf" method
					if ((FALSE == b_is_jmisp_model) && (FALSE == b_is_gblktile_model)) {
						BOOL is_need_ubuf = FALSE;
						UINT32 ubuf_need_size = 0;
						ret = _vendor_ai_net_check_tcm_use((VENDOR_AIS_FLOW_MEM_PARM *)&p_user->model_buf, &is_need_ubuf, &ubuf_need_size);
						if (ret != HD_OK) {
							DBG_ERR("check tcm use failed ...\r\n");
							return ret;
						}
						if (TRUE == is_need_ubuf) {
							p_user->proc_mem.buf[AI3_PROC_BUF_WORKBUF].size += ubuf_need_size;
							DBG_IND("get() - FAKE_UBUF = %u \r\n", ubuf_need_size);
							if(TRUE == is_must_ubuf){
								p_user->config[MODEL_INFO_UBUF_NUM] = (ubuf_need_size/AI_SUPPORT_UBUF_BLOCK_SIZE) + (((ubuf_need_size % AI_SUPPORT_UBUF_BLOCK_SIZE) > 0U)?1U:0U) ;
								DBG_IND("get() - must use ubuf_num = %llu \r\n", p_user->config[MODEL_INFO_UBUF_NUM]);
							}
						}
					}
				}

			}
			break;

		case VENDOR_AI3_CFG_AI_CB: 
			{
				uintptr_t* isp_cb = (uintptr_t*) p_param ; 
				ret  = vendor_ais_get_isp_cb(isp_cb)  ;
				if (ret != HD_OK) return ret;
			}
			break;

		case VENDOR_AI3_CFG_IO_CNT:
			{
				VENDOR_AI3_IO_BUF_INFO *p_user = (VENDOR_AI3_IO_BUF_INFO *)p_param;
				UINT32 in_cnt=0, out_cnt=0;

				// check model valid
				if (p_user->model_buf.pa == 0 || p_user->model_buf.va == 0 || p_user->model_buf.size == 0) {
					DBG_ERR("check model_buf fail !! model_buf (pa/va/size) = (0x%016lx, 0x%016lx, %d)\r\n", p_user->model_buf.pa, p_user->model_buf.va, (int)p_user->model_buf.size);
					return HD_ERR_INV;
				}

				// check if this is NN30 model & chip_id check
				if (HD_OK != vendor_ais_net_gen_chk_vers((VENDOR_AIS_FLOW_MEM_PARM *)&p_user->model_buf, 0)) {
					DBG_ERR("check NN30 nvt_model fail !!\r\n");
					return HD_ERR_INV;
				}

				// query in/out buffer count & return to user
				ret = _vendor_ais_get_net_in_out_info_list((VENDOR_AIS_FLOW_MEM_PARM *)&p_user->model_buf, &in_cnt, &out_cnt, NULL, NULL);
				if (ret != HD_OK) {
					DBG_ERR("_vendor_ais_get_net_in_out_info_list() query in/out buffer count failed ... ret = %d\r\n", (int)ret);
					return ret;
				}
				p_user->in_buf_cnt  = in_cnt;
				p_user->out_buf_cnt = out_cnt;
			}
			break;

		case VENDOR_AI3_CFG_IO_INFO:
			{
				VENDOR_AI3_IO_BUF_INFO *p_user = (VENDOR_AI3_IO_BUF_INFO *)p_param;
				UINT32 in_cnt=0, out_cnt=0;

				// check pointer
				if (p_user->in_buf_info == NULL)  { DBG_ERR("check in_buf_info == NULL !!\r\n");  return HD_ERR_INV; }
				if (p_user->out_buf_info == NULL) { DBG_ERR("check out_buf_info == NULL !!\r\n"); return HD_ERR_INV; }

				// check model valid
				if (p_user->model_buf.pa == 0 || p_user->model_buf.va == 0 || p_user->model_buf.size == 0) {
					DBG_ERR("check model_buf fail !! model_buf (pa/va/size) = (0x%016lx, 0x%016lx, %d)\r\n", p_user->model_buf.pa, p_user->model_buf.va, (int)p_user->model_buf.size);
					return HD_ERR_INV;
				}

				// check if this is NN30 model & chip_id check
				if (HD_OK != vendor_ais_net_gen_chk_vers((VENDOR_AIS_FLOW_MEM_PARM *)&p_user->model_buf, 0)) {
					DBG_ERR("check NN30 nvt_model fail !!\r\n");
					return HD_ERR_INV;
				}

				// query in/out buffer count, check user struct in/out buffer count  ( K-customer may decide io_count by himself without call VENDOR_AI3_CFG_IO_CNT to query correct count. )
				ret = _vendor_ais_get_net_in_out_info_list((VENDOR_AIS_FLOW_MEM_PARM *)&p_user->model_buf, &in_cnt, &out_cnt, NULL, NULL);
				if (ret != HD_OK) {
					DBG_ERR("_vendor_ais_get_net_in_out_info_list() query in/out buffer count failed ... ret = %d\r\n", (int)ret);
					return ret;
				}

				// check user given in/out buffer count valid
				if (p_user->in_buf_cnt < in_cnt) {
					DBG_ERR("model input count = %d, but given in_buf_cnt = %d , it's NOT enough !!!!\r\n", in_cnt, p_user->in_buf_cnt);
					return HD_ERR_INV;
				}
				if (p_user->out_buf_cnt < out_cnt) {
					DBG_ERR("model output count = %d, but given out_buf_cnt = %d , it's NOT enough !!!!\r\n", out_cnt, p_user->out_buf_cnt);
					return HD_ERR_INV;
				}

				// fill in/out buffer info
				ret = _vendor_ais_get_net_in_out_info_list((VENDOR_AIS_FLOW_MEM_PARM *)&p_user->model_buf, &in_cnt, &out_cnt, p_user->in_buf_info, p_user->out_buf_info);
				if (ret != HD_OK) {
					DBG_ERR("_vendor_ais_get_net_in_out_info_list() query in/out buffer info failed ... ret = %d\r\n", (int)ret);
					return ret;
				}
			}
			break;

		case VENDOR_AI3_CFG_EOP_MAX:
			{
				VENDOR_AI3_EOP_CFG *p_user = (VENDOR_AI3_EOP_CFG*) p_param;
				switch(p_user->eop)
				{
					case FC_LL_MODE:
						{
							UINT32 buf_size = 0;
							UINT32 slice_num = 0; 
							UINT32 stripe_num = 0; 

							ret = _vendor_ai_get_cal_max_buf_size(&buf_size) ;
							if (ret != HD_OK) {
								DBG_ERR("_vendor_ai_get_cal_max_buf_size fail=%d\n", (int)ret);
								return ret;
							}

							stripe_num = (((p_user->input1.w +7) / 8)*32 * p_user->input1.h + 65535)/65536;
							if (stripe_num == 1 ){
								p_user->swap_out = 0 ; // No need to allocate temp buf 
							}else if(stripe_num % 2 == 1){
								p_user->swap_out = 1 ; // Output to temp buf 
							}else{
								p_user->swap_out = 2 ; // Output to out buf 
							}

							slice_num = (p_user->input2.h / 131000U) + (((p_user->input2.h % 131000U) > 0U)?1U:0U);
							p_user->buf.size = ALIGN_CEIL_64(sizeof(VENDOR_AI_OP_WORK)) + ALIGN_CEIL_64(buf_size * slice_num + sizeof(JOB_LIST_TRIG_CMD) + sizeof(JOB_LIST_NULL_CMD));
						}
						break;

					case PREPROC_YUV2RGB:
					case PREPROC_YUV2RGB_SCALE:
					case PREPROC_YUV2RGB_MEANSUB_PLANE:
					case PREPROC_YUV2RGB_MEANSUB_DC:
					case PREPROC_Y2Y_UV2UV:
					case PREPROC_RGB_SCALE:
					case PREPROC_Y8_SCALE:
						{
							p_user->buf.size = ALIGN_CEIL_64(sizeof(VENDOR_AI_OP_WORK)); 
						}
						break;
					
					case TOPN_SORT:
						{
							UINT32 buf_size = 0;
							UINT32 slice_num = 0;  

							ret = _vendor_ai_get_cal_max_buf_size(&buf_size) ;
							if (ret != HD_OK) {
								DBG_ERR("_vendor_ai_get_cal_max_buf_size fail=%d\n", (int)ret);
								return ret;
							}
		
							slice_num = (p_user->input1.h / 131000U) + (((p_user->input1.h % 131000U) > 0U)?1U:0U);
							p_user->buf.size = ALIGN_CEIL_64(sizeof(VENDOR_AI_OP_WORK)) + ALIGN_CEIL_64(buf_size * slice_num + sizeof(JOB_LIST_TRIG_CMD) + sizeof(JOB_LIST_NULL_CMD));
							// DBG_ERR("buf_size =%u slice_num =%u mem->buf.size %u\n", buf_size, slice_num, mem->buf.size);
						}
						break;

					default:
						DBG_ERR("unsupport eop(%d) ...\r\n", (int)p_user->eop);
						return HD_ERR_INV;

				}
			}
			break;
		case VENDOR_AI3_CFG_DEV_INFO: 
			{
				VENDOR_AI3_DEV_INFO* p_user = (VENDOR_AI3_DEV_INFO*) p_param ; 
				ret = vendor_ais_get_dtsi_info(p_user);
				if (ret != HD_OK) return ret;
			}
			break;
		default:
			DBG_ERR("unsupport cfg_id(%d) ...\r\n", (int)cfg_id);
			return HD_ERR_INV;
	}

	return ret;
}

HD_RESULT vendor_ai3_dev_perf_begin (VENDOR_AI3_PERF_ID perf_id)
{
	HD_RESULT ret = HD_OK;

	switch (perf_id)
	{
		case VENDOR_AI3_PERF_ID_TIME_UT:
			{
				ret = vendor_ai_cfg_set(VENDOR_AI_CFG_PERF_UT, NULL);
				if (HD_OK != ret) {
					DBG_ERR("(VENDOR_AI3_PERF_ID_TIME_UT) failed !! ret = %d\r\n", (int)ret);
					return ret;
				}
			}
			break;
        case VENDOR_AI3_PERF_ID_TIMELINE:
            {
                ret = vendor_ai_dla_perf_timeline(1);
				if (HD_OK != ret) {
					DBG_ERR("(VENDOR_AI3_PERF_ID_TIMELINE) failed !! ret = %d\r\n", (int)ret);
					return ret;
				}
            }
            break;
		default:
			DBG_ERR("unsupport perf_id(%d) ...\r\n", (int)perf_id);
			return HD_ERR_INV;
	}
	return ret;
}

HD_RESULT vendor_ai3_dev_perf_end (VENDOR_AI3_PERF_ID perf_id, void* p_param)
{
	HD_RESULT ret = HD_OK;

	switch (perf_id)
	{
		case VENDOR_AI3_PERF_ID_TIME_UT:
			{
				VENDOR_AI3_PERF_TIME_UT *p_user = (VENDOR_AI3_PERF_TIME_UT *)p_param;
				VENDOR_AI_PERF_UT perf_ut = {0};

                if (p_param == NULL) {		DBG_ERR("check p_param == NULL !!\r\n");		return HD_ERR_INV;	}
				ret = vendor_ai_cfg_get(VENDOR_AI_CFG_PERF_UT, &perf_ut);
				if (HD_OK != ret) {
					DBG_ERR("(VENDOR_AI3_PERF_ID_TIME_UT) failed !! ret = %d\r\n", (int)ret);
					return ret;
				}
				memcpy(p_user, &perf_ut, sizeof(VENDOR_AI_PERF_UT)); // simple copy, because api/user/kernel struct are actually the same
			}
			break;
        case VENDOR_AI3_PERF_ID_TIMELINE:
            {
                ret = vendor_ai_dla_perf_timeline(0);
				if (HD_OK != ret) {
					DBG_ERR("(VENDOR_AI3_PERF_ID_TIMELINE) failed !! ret = %d\r\n", (int)ret);
					return ret;
				}
            }
            break;
		default:
			DBG_ERR("unsupport perf_id(%d) ...\r\n", (int)perf_id);
			return HD_ERR_INV;
	}
	return ret;
}

HD_RESULT vendor_ai3_net_open (UINT32* p_proc_id, VENDOR_AI3_PROC_CFG* p_proc_cfg, VENDOR_AI3_NET_INFO* p_net_info)
{
	HD_RESULT ret = HD_OK;
	UINT32 user_parm_size = 0, user_buff_size = 0;
	UINT32 proc_id = 0;
	VENDOR_AI_NET_INFO_PROC *p_proc = 0;
    UINT64 ts_start = 0;
    PERF_TIME_BEGIN(&ts_start);
	// check user param valid
	if (p_proc_id  == NULL) {		DBG_ERR("check p_proc_id == NULL !!\r\n");		return HD_ERR_INV;	}
	if (p_proc_cfg == NULL) {		DBG_ERR("check p_proc_cfg == NULL !!\r\n");		return HD_ERR_INV;	}
	if (p_net_info == NULL) {		DBG_ERR("check p_net_info == NULL !!\r\n");		return HD_ERR_INV;	}

	if (p_proc_cfg->model_buf.pa == 0 || p_proc_cfg->model_buf.va == 0 || p_proc_cfg->model_buf.size == 0) {
		DBG_ERR("check model_buf fail !! model_buf (pa/va/size) = (0x%016lx, 0x%016lx, %d)\r\n", p_proc_cfg->model_buf.pa, p_proc_cfg->model_buf.va, (int)p_proc_cfg->model_buf.size);
		return HD_ERR_INV;
	}
#if defined(_BSP_NS02201_) || defined(_BSP_NS02401_)
	if (p_proc_cfg->model_buf.pa % 64 != 0) {
		DBG_ERR("model_buf: pa(0x%016lx) is NOT 64x align !!\n", p_proc_cfg->model_buf.pa);
		return HD_ERR_INV;
	}
#else
	if (p_proc_cfg->model_buf.pa % 32 != 0) {
		DBG_ERR("model_buf: pa(0x%016lx) is NOT 32x align !!\n", p_proc_cfg->model_buf.pa);
		return HD_ERR_INV;
	}
#endif
	{
		UINT32 pool_id = p_proc_cfg->config[AI3_PROC_CFG_ISP_POOL_ID] & 0xf ; 
		VENDOR_AI3_CFG_BUF shared_workbuf = {0} ;
		
		if (p_proc_cfg->config[AI3_PROC_CFG_ISP_MODE] == AI3_ISP_MULTI_ISO_MODE ){
			// check whether pool_id is set 
			if ((p_proc_cfg->config[AI3_PROC_CFG_ISP_POOL_ID] & 0x80) == 0 ){
				DBG_ERR("pool_id is not set while AI3_PROC_CFG_ISP_MODE is on \r\n");
				goto failed;
			}
			ret = vendor_ais_get_shared_workbuf(pool_id, &shared_workbuf) ; 
			if (ret == HD_OK) {
				if( shared_workbuf.pa != p_proc_cfg->proc_mem.buf[AI3_PROC_BUF_WORKBUF].pa || 
				    shared_workbuf.va != p_proc_cfg->proc_mem.buf[AI3_PROC_BUF_WORKBUF].va || 
					shared_workbuf.size != p_proc_cfg->proc_mem.buf[AI3_PROC_BUF_WORKBUF].size){
				  DBG_ERR("AI3_PROC_BUF_WORKBUF is not the same as pool_%u's SHARED_WORKBUF (0x%016lx, 0x%016lx, %d) \r\n", pool_id, shared_workbuf.pa, shared_workbuf.va, (int)shared_workbuf.size);
				  goto failed;
				}
				
			}
		}
	}

	if (p_proc_cfg->proc_mem.buf[AI3_PROC_BUF_WORKBUF].pa == 0 || p_proc_cfg->proc_mem.buf[AI3_PROC_BUF_WORKBUF].va == 0 || p_proc_cfg->proc_mem.buf[AI3_PROC_BUF_WORKBUF].size == 0) {
		DBG_ERR("check AI3_PROC_BUF_WORKBUF fail !! AI3_PROC_BUF_WORKBUF (pa/va/size) = (0x%016lx, 0x%016lx, %d)\r\n", p_proc_cfg->proc_mem.buf[AI3_PROC_BUF_WORKBUF].pa, p_proc_cfg->proc_mem.buf[AI3_PROC_BUF_WORKBUF].va, (int)p_proc_cfg->proc_mem.buf[AI3_PROC_BUF_WORKBUF].size);
		return HD_ERR_INV;
	}
	if (p_proc_cfg->proc_mem.buf[AI3_PROC_BUF_RONLYBUF].pa == 0 || p_proc_cfg->proc_mem.buf[AI3_PROC_BUF_RONLYBUF].va == 0 || p_proc_cfg->proc_mem.buf[AI3_PROC_BUF_RONLYBUF].size == 0) {
		DBG_ERR("check AI3_PROC_BUF_RONLYBUF fail !! AI3_PROC_BUF_RONLYBUF (pa/va/size) = (0x%016lx, 0x%016lx, %d)\r\n", p_proc_cfg->proc_mem.buf[AI3_PROC_BUF_RONLYBUF].pa, p_proc_cfg->proc_mem.buf[AI3_PROC_BUF_RONLYBUF].va, (int)p_proc_cfg->proc_mem.buf[AI3_PROC_BUF_RONLYBUF].size);
		return HD_ERR_INV;
	}

	// check if this is NN30 model & chip_id check
	if (HD_OK != vendor_ais_net_gen_chk_vers((VENDOR_AIS_FLOW_MEM_PARM *)&p_proc_cfg->model_buf, 0)) {
		DBG_ERR("check NN30 nvt_model fail !!\r\n");
		return HD_ERR_INV;
	}

	// get user_parm & user_buff size
	{
		VENDOR_AIS_FLOW_MAP_MEM_PARM mem_manager_tmp = {0};
		vendor_ais_auto_alloc_mem((VENDOR_AIS_FLOW_MEM_PARM *)&p_proc_cfg->model_buf, &mem_manager_tmp);  // we only need user_parm 
		user_parm_size = ALIGN_CEIL_64(mem_manager_tmp.user_parm.size);
		user_buff_size = ALIGN_CEIL_64(mem_manager_tmp.user_buff.size);
	}

	// check if debug mode => re-write WORKBUF need size
	if (p_proc_cfg->ctrl & CTRL_BUF_DEBUG) {
		UINT32 req_size = 0;
		ret = _vendor_ai_net_mem_query_full_alloc_size((VENDOR_AIS_FLOW_MEM_PARM *)&p_proc_cfg->model_buf, &req_size);
		if (ret != HD_OK) {
			DBG_ERR("query full alloc size failed ...\r\n");
			return ret;
		}
		user_buff_size  = ALIGN_CEIL_64(req_size);  // debug mdoe, update user_buff_size need size = FULL alloc size (bufopt = 0)
	}

	// check user given size
	{
#if FLOW_USE_KERNEL
		if (p_proc_cfg->proc_mem.buf[AI3_PROC_BUF_RONLYBUF].size < (user_parm_size*2) ) {
			DBG_ERR("AI3_PROC_BUF_RONLYBUF size (%d) is too small, need (%d)\r\n", (int)p_proc_cfg->proc_mem.buf[AI3_PROC_BUF_RONLYBUF].size, (int)(user_parm_size*2));
			return HD_ERR_INV;
		}
#else 
		if (p_proc_cfg->proc_mem.buf[AI3_PROC_BUF_RONLYBUF].size < (user_parm_size) ) {
			DBG_ERR("AI3_PROC_BUF_RONLYBUF size (%d) is too small, need (%d)\r\n", (int)p_proc_cfg->proc_mem.buf[AI3_PROC_BUF_RONLYBUF].size, (int)(user_parm_size));
			return HD_ERR_INV;
		}
#endif
		if (p_proc_cfg->proc_mem.buf[AI3_PROC_BUF_WORKBUF].size  < (user_buff_size) ) {
			if (p_proc_cfg->config[AI3_PROC_CFG_ISP_MODE] == AI3_ISP_MULTI_ISO_MODE){
				DBG_ERR("SHARED_WORKBUF size (%d) is too small, need (%d)\r\n", (int)p_proc_cfg->proc_mem.buf[AI3_PROC_BUF_WORKBUF].size, (int)(user_buff_size));	
			}else {
				DBG_ERR("AI3_PROC_BUF_WORKBUF size (%d) is too small, need (%d)\r\n", (int)p_proc_cfg->proc_mem.buf[AI3_PROC_BUF_WORKBUF].size, (int)(user_buff_size));
			}
			return HD_ERR_INV;
		}
	}

    // check dyscale/dybatch model
    if (p_proc_cfg->ctrl & CTRL_BUF_DEBUG || p_proc_cfg->ctrl & CTRL_JOB_DUMPOUT) {
        NN_GEN_NET_INFO net_info = {0};
        UINT32 op_mode = 0, isDyScale = 0, isDyBatch = 0;
		ret = vendor_ais_get_net_info(&net_info, p_proc_cfg->model_buf.va);
		if (ret != HD_OK) {
			DBG_ERR("vendor_ais_get_net_info fail...\r\n");
			return ret;
		}
        isDyScale = MODEL_TYPE_ATTR_GET(net_info.p_head->model_type, MODEL_TYPE_ATTR_DYSCALE);
        isDyBatch = MODEL_TYPE_ATTR_GET(net_info.p_head->model_type, MODEL_TYPE_ATTR_DYBATCH);
        if(isDyScale || isDyBatch) {
            if (p_proc_cfg->ctrl & CTRL_BUF_DEBUG) {
                DBG_ERR("CTRL_BUF_DEBUG is not support in dyscale/dybatch model !!\r\n");
                return HD_ERR_NOT_SUPPORT;
            }
            if (p_proc_cfg->ctrl & CTRL_JOB_DUMPOUT) {
                op_mode = MODEL_TYPE_ATTR_GET(net_info.p_head->model_type, MODEL_TYPE_ATTR_OPMODE);
                if(op_mode != 0) {
                    DBG_DUMP("\n[ai][lib][comm] >>>>>>>>>>>>>>>>>>>>>>>>>>>  [ai][lib][comm] WARNNING:Intermediate layers of dyscale/dybatch model may give different results without using op-mode 0 ..  <<<<<<<<<<<<<<<<<<<<<<<<<<<<<\n\n");
                }
            }
        }
    }

	{
	// get available proc_id
	   
		ret = vendor_ai_get_id(&proc_id);
		if (ret != HD_OK) {
			DBG_ERR("get proc_id failed !! ret = %d\r\n", (int)ret);
			return ret;
		}
		
		*p_proc_id = proc_id;
		 p_proc = _vendor_ai_info(proc_id); 

	}

	// NOTE : after this line, if anything wrong ... should "goto failed" to release proc_id which is already got from vendor_ai_get_id()

	//===> RONLYBUF will be split into 2 part :  (1) original INITBUF + (2) original RONLYBUF   , PLEASE NOTE !! group() buffer is removed from INITBUF , because we won't do group() alg. at sdk now
	//===> WORKBUF  will be set to : (1) original WORKBUF

	// set buf opt : SDK will NOT support bufopt re-alloc, alwyas use bufopt = (-1) method  // TODO : maybe some debug mode, will set to bufopt = 0
	{
		VENDOR_AI_NET_CFG_BUF_OPT cfg_buf_opt = {0};
		if (p_proc_cfg->ctrl & CTRL_BUF_DEBUG) {
			cfg_buf_opt.method = VENDOR_AI_NET_BUF_OPT_FULL; // debug mode, use bufopt = 0
		} else {
			cfg_buf_opt.method = VENDOR_AI_NET_BUF_OPT_NONE; // normal case, use bufopt = (-1)  to use tool alloc iobuf size
		}
		cfg_buf_opt.ddr_id = DDR_ID0;
		ret = vendor_ai_net_set(proc_id, VENDOR_AI_NET_PARAM_CFG_BUF_OPT, &cfg_buf_opt);
		if (ret != HD_OK) {
			DBG_ERR("proc_id(%d) set VENDOR_AI_NET_PARAM_CFG_BUF_OPT fail, ret = %d\r\n", (int)proc_id, (int)ret);
			goto failed;
		}
	}
	// set buf_ctrl for float in & out
	{
		VENDOR_AI_NET_CFG_BUF_CTRL buf_ctrl = {0};
		if (p_proc_cfg->ctrl & CTRL_BUF_FLOATIN) {
			buf_ctrl.ctrl = buf_ctrl.ctrl | CTRL_BUF_FLOATIN ;  //  use float in 
		} 
		if (p_proc_cfg->ctrl & CTRL_BUF_FLOATOUT) {
			buf_ctrl.ctrl = buf_ctrl.ctrl | CTRL_BUF_FLOATOUT ;  // use float out
		} 
        if (p_proc_cfg->ctrl & CTRL_JOB_DUMPOUT) {
			buf_ctrl.ctrl = buf_ctrl.ctrl | CTRL_JOB_DUMPOUT ;  // for debug dump
		} 
		ret = vendor_ai_net_set(proc_id, VENDOR_AI_NET_PARAM_CFG_BUF_CTRL, &buf_ctrl);
		if (ret != HD_OK) {
			DBG_ERR("proc_id(%d) set VENDOR_AI_NET_CFG_BUF_CTRL fail, ret = %d\r\n", (int)proc_id, (int)ret);
			goto failed;
		}
	}

	// set job opt : SDK will NOT support jobopt group(), alwyas use jobopt = 0 method, wait_ms = (-1). If want graph method, tool should generate JOB LIST CMD + JOB LIST GRAPH_INFO.
	{
		VENDOR_AI_NET_CFG_JOB_OPT cfg_job_opt = {0};
		if (p_proc_cfg->ctrl & CTRL_JOB_DEBUG) {
			cfg_job_opt.method = VENDOR_AI_NET_JOB_OPT_LINEAR; // debug mode, use jobopt = 0
			cfg_job_opt.wait_ms = -1;                          // debug mode, use wait_ms = (-1)
            g_is_job_debug_mode[proc_id] = 1;
            vendor_ai_dla_set_job_debug_mode(proc_id);
			if (max_isp > 0)
				DBG_DUMP("\n[ai][lib][comm] >>>>>>>>>>>>>>>>>>>>>>>>>>>  WARNNING: proc_id(%d), debug mode is enabled dtsi ai_maxisp(%u), can't run with aiisp model simultaneously <<<<<<<<<<<<<<<<<<<<<<<<<<<<<\n\n", (int)proc_id, max_isp);

            cfg_job_opt.schd_parm = (VENDOR_AI_DLA_CORE(0)); //bind core 0 in debug mode
			DBG_DUMP("\n[ai][lib][comm] >>>>>>>>>>>>>>>>>>>>>>>>>>>  WARNNING: proc_id(%d), debug mode is enabled, force bind core 0!!  <<<<<<<<<<<<<<<<<<<<<<<<<<<<<\n\n", (int)proc_id);
        } else {
			cfg_job_opt.method = VENDOR_AI_NET_JOB_OPT_LINEAR; // normal case, use jobopt = 0
			cfg_job_opt.wait_ms = 0;                           // normal case, use wait_ms = 0
            cfg_job_opt.schd_parm = VENDOR_AI_CORE_MASK_DEFAULT; //FAIR dispatch to ALL core
		}
		ret = vendor_ai_net_set(proc_id, VENDOR_AI_NET_PARAM_CFG_JOB_OPT, &cfg_job_opt);
		if (ret != HD_OK) {
			DBG_ERR("proc_id(%d) set VENDOR_AI_NET_PARAM_CFG_JOB_OPT fail, ret = %d\r\n", (int)proc_id, (int)ret);
			goto failed;
		}
	}

	// set model
	ret = vendor_ai_net_set(proc_id, VENDOR_AI_NET_PARAM_CFG_MODEL, (VENDOR_AI_NET_CFG_MODEL*)&p_proc_cfg->model_buf);
	if (ret != HD_OK) {
		DBG_ERR("proc_id(%d) set model fail, ret = %d\r\n", (int)proc_id, (int)ret);
		goto failed;
	}

	// call this, just to update INITBUF req_size variable (for later error handle check)
	{
		VENDOR_AI_NET_CFG_INTLBUF buf = {0};
		ret = vendor_ai_net_get(proc_id, VENDOR_AI_NET_PARAM_CFG_INTLBUF, &buf);
		if (ret != HD_OK) {
			DBG_ERR("proc_id(%d) get VENDOR_AI_NET_PARAM_CFG_INTLBUF fail, ret = %d\r\n", (int)proc_id, (int)ret);
			goto failed;
		}
	}

	// set init buffer
	{
		VENDOR_AI_NET_CFG_INTLBUF buf = {0};

		buf.pa   = p_proc_cfg->proc_mem.buf[AI3_PROC_BUF_RONLYBUF].pa;
		buf.va   = p_proc_cfg->proc_mem.buf[AI3_PROC_BUF_RONLYBUF].va;
		buf.size = user_parm_size;
		ret = vendor_ai_net_set(proc_id, VENDOR_AI_NET_PARAM_CFG_INTLBUF, &buf);
		if (ret != HD_OK) {
			DBG_ERR("proc_id(%d) set VENDOR_AI_NET_PARAM_CFG_INTLBUF fail, ret = %d\r\n", (int)proc_id, (int)ret);
			goto failed;
		}
	}

	// set plugin engine
	{
		UINT32 i = 0;
		for (i = 0; i < VENDOR_AI_COUNT; i++) {
			VENDOR_AI_ENGINE_PLUGIN* p_cfg_engine = (VENDOR_AI_ENGINE_PLUGIN *)p_proc_cfg->plugin[i];
			UINT32 engine_id = i;

			if (p_cfg_engine && p_cfg_engine->proc_cb != 0) {
				switch (engine_id) {
				case AI3_PLUGIN_CPU:
					if (p_cfg_engine->eng != 1) {
						DBG_ERR("proc_id(%d) check user given plugin[AI3_PLUGIN_CPU] is NOT CPU engine !!\r\n", (int)proc_id);
						continue;
					}
					// call plugin engine open
					ret = p_cfg_engine->proc_cb(proc_id, VENDOR_AI_CTRL_LYR, VENDOR_AI_CTRL_NET_OPEN, 0, 0);
					if (ret != HD_OK) {
						DBG_ERR("proc_id(%d) proc_cb open fail...\r\n", (int)proc_id);
					}
					break;
				case AI3_PLUGIN_DSP:
					if (p_cfg_engine->eng != 2) {
						DBG_ERR("proc_id(%d) check user given plugin[AI3_PLUGIN_DSP] is NOT DSP engine !!\r\n", (int)proc_id);
						continue;
					}
					break;
				default:
					DBG_ERR("AI-open: plugin engine: not support type = %d!\r\n", (int)engine_id+1);
					continue;
				}

				ret = vendor_ai_net_set(proc_id, VENDOR_AI_NET_PARAM_PLUGIN_ENGINE, p_cfg_engine);
				if (ret != HD_OK) {
					DBG_ERR("AI-open: set plugin engine %d fail = %d\r\n", (int)engine_id+1, ret);
					goto failed;
				}
			}
		}
	}
	
	// if nnisp, gblktile model, set nnisp pool_id
	{
		NN_GEN_NET_INFO net_info = {0};
		AI_JMISP_INFO_HEADER * jmisp_header = NULL; 
		GBLKTILE_INFO_HEADER * gblktile_header = NULL;
		VENDOR_AI_NET_CFG_PROC *p_cfg = _vendor_ai_cfg(proc_id);

		ret = vendor_ais_get_net_info(&net_info, p_proc_cfg->model_buf.va);
		if (ret != HD_OK) {
			DBG_ERR("vendor_ais_get_net_info fail...\r\n");
			return ret;
		}
		// check whether ai-pool-cfg exist
		ret = vendor_ais_has_pool(&p_cfg->b_is_using_pool);

		if ((p_proc_cfg->config[AI3_PROC_CFG_ISP_POOL_ID] & 0x80) && p_proc_cfg->config[AI3_PROC_CFG_ISP_MODE]){
			 if (net_info.p_head->jmisp_info_size == 0){
				DBG_ERR("proc_id(%d) is not jmisp model, couldn't be set pool_id and AI3_PROC_CFG_ISP_MODE \r\n", (int)proc_id);
					goto failed;
			 }
		} 
		// if gblktile model
		if ((net_info.p_head->gblktile_info_size > 0)){
			g_is_aiisp_model[proc_id] = 1;
			gblktile_header = (GBLKTILE_INFO_HEADER *)net_info.p_gblktile_header_blk;
			if (p_proc_cfg->config[AI3_PROC_CFG_POOL_ID] & 0x80){
				vendor_ai_net_trace(proc_id, AI_FLOW, "set() - pool_id = %u\r\n", p_proc_cfg->config[AI3_PROC_CFG_POOL_ID] & 0xf);
				ret = vendor_ais_set_pool_id(proc_id, p_proc_cfg->config[AI3_PROC_CFG_POOL_ID] & 0xf, gblktile_header->total_ubuf_size, gblktile_header->core_num) ; 
				if (ret != HD_OK) {
					DBG_ERR("proc_id(%d) vendor_ais_set_pool_id fail\r\n", (int)proc_id);
					goto failed;
				}
			}else {
				
				if (p_cfg->b_is_using_pool){
					ret = HD_ERR_NO_CONFIG;
					DBG_ERR("This is gblktile model. Please set p_proc_cfg->config[AI3_PROC_CFG_POOL_ID] for proc_id(%d) \r\n", (int)proc_id);
					goto failed;
				}else {
					ret = vendor_ais_set_pool_id(proc_id, 0xff, gblktile_header->total_ubuf_size, gblktile_header->core_num) ; 
					if (ret != HD_OK) {
						DBG_ERR("proc_id(%d) vendor_ais_set_pool_id fail\r\n", (int)proc_id);
						goto failed;
					}
				}
				
			}

		}
		// if nnisp model
		if ((net_info.p_head->jmisp_info_size > 0)){
			g_is_aiisp_model[proc_id] = 1;
			jmisp_header = (AI_JMISP_INFO_HEADER *)net_info.p_jmisp_header_blk;
			if (p_proc_cfg->config[AI3_PROC_CFG_ISP_POOL_ID] & 0x80){
				vendor_ai_net_trace(proc_id, AI_FLOW, "set() - pool_id = %u\r\n", p_proc_cfg->config[AI3_PROC_CFG_ISP_POOL_ID] & 0xf);
				ret = vendor_ais_set_isp_pool_id(proc_id, p_proc_cfg->config[AI3_PROC_CFG_ISP_POOL_ID] & 0xf, jmisp_header->total_ubuf_size, jmisp_header->core_num, p_proc_cfg->config[AI3_PROC_CFG_ISP_MODE]) ; 
				if (ret != HD_OK) {
					DBG_ERR("proc_id(%d) vendor_ais_set_isp_pool_id fail\r\n", (int)proc_id);
					goto failed;
				}
			}else {

				if (p_cfg->b_is_using_pool){
					ret = HD_ERR_NO_CONFIG;
					DBG_ERR("This is aiisp model. Please set p_proc_cfg->config[AI3_PROC_CFG_POOL_ID] for proc_id(%d) \r\n", (int)proc_id);
					goto failed;
				}else {
					// pool_id 0xff => default using all aiisp resource max jobm & ub 
					ret = vendor_ais_set_isp_pool_id(proc_id, 0xff, jmisp_header->total_ubuf_size, jmisp_header->core_num, AI3_ISP_MULTI_DEV_MODE) ; 
					if (ret != HD_OK) {
						DBG_ERR("proc_id(%d) vendor_ais_set_pool_id fail\r\n", (int)proc_id);
						goto failed;
					}
				}
			}

		}

	}

	//======= call original open() ======
	ret = vendor_ai_net_open(proc_id);
	if (ret != HD_OK) {
		DBG_ERR("proc_id(%d) vendor_ai_net_open() fail\r\n", (int)proc_id);
		goto failed;
	}
	
	// set ronly buffer
	{
        VENDOR_AI_NET_CFG_RONLYBUF rbuf  = {0};
        rbuf.pa   = p_proc_cfg->proc_mem.buf[AI3_PROC_BUF_RONLYBUF].pa + user_parm_size;
        rbuf.va   = p_proc_cfg->proc_mem.buf[AI3_PROC_BUF_RONLYBUF].va + user_parm_size;
        rbuf.size = user_parm_size;
        ret = vendor_ai_net_set(proc_id, VENDOR_AI_NET_PARAM_CFG_RONLYBUF, &rbuf);
        if (ret != HD_OK) {
            DBG_ERR("proc_id(%d) set VENDOR_AI_NET_PARAM_CFG_RONLYBUF fail, ret = %d\r\n", (int)proc_id, (int)ret);
            goto failed;
        }
	}

	// set work buffer
	{
        VENDOR_AI_NET_CFG_WORKBUF wbuf = {0};
        wbuf.pa   = p_proc_cfg->proc_mem.buf[AI3_PROC_BUF_WORKBUF].pa;
        wbuf.va   = p_proc_cfg->proc_mem.buf[AI3_PROC_BUF_WORKBUF].va;
        wbuf.size = p_proc_cfg->proc_mem.buf[AI3_PROC_BUF_WORKBUF].size;
        ret = vendor_ai_net_set(proc_id, VENDOR_AI_NET_PARAM_CFG_WORKBUF, &wbuf);
        if (ret != HD_OK) {
            DBG_ERR("proc_id(%d) set VENDOR_AI_NET_PARAM_CFG_WORKBUF fail, ret = %d\r\n", (int)proc_id, (int)ret);
            goto failed;
        }
	}

	// query (TODO) & update net_info
	p_net_info->layer_cnt     = 0;  // TODO
	p_net_info->bind_cnt      = 0;
	p_net_info->buf_cnt       = 0;


	{
		VENDOR_AI_NET_INFO net_info = {0};
		ret = vendor_ai_net_get(proc_id, VENDOR_AI_NET_PARAM_INFO, &net_info);
		if (HD_OK != ret) {
			printf("proc_id(%u) get info fail !!\n", proc_id);
			goto failed;
		}
		p_proc = _vendor_ai_info(proc_id); 
		p_proc->net_info.in_buf_cnt    =  net_info.in_buf_cnt;
		p_proc->net_info.out_buf_cnt   =  net_info.out_buf_cnt;
		p_proc->mode				   =  p_proc_cfg->mode;
		
		/* get in out path list */
		p_proc->net_info.in_path_list = (UINT32 *)vendor_ai_malloc(sizeof(UINT32) * (p_proc->net_info.in_buf_cnt));
		ret = vendor_ai_net_get(proc_id, VENDOR_AI_NET_PARAM_IN_PATH_LIST, p_proc->net_info.in_path_list);
		if (HD_OK != ret) {
			printf("proc_id(%u) get VENDOR_AI_NET_PARAM_IN_PATH_LIST fail(%d) !!\n", proc_id, ret);
			goto failed;
		}

		if (p_proc->net_info.out_buf_cnt){
			// out_buf_cnt = 0 in nnisp model
			p_proc->net_info.out_path_list = (UINT32 *)vendor_ai_malloc(sizeof(UINT32) * (p_proc->net_info.out_buf_cnt));
			ret = vendor_ai_net_get(proc_id, VENDOR_AI_NET_PARAM_OUT_PATH_LIST, p_proc->net_info.out_path_list);
			if (HD_OK != ret) {
				printf("proc_id(%u) get VENDOR_AI_NET_PARAM_OUT_PATH_LIST fail(%d) !!\n", proc_id, ret);
				goto failed;
			}
		}
		
		memcpy(p_net_info, &p_proc->net_info, sizeof(VENDOR_AI3_NET_INFO) ) ;
	}
	
	// update not check setting
	{	
		p_proc = _vendor_ai_info(proc_id); 
		p_proc->not_check_input_align = _vendor_ai_common_info()->not_check_input_align ; 
		p_proc->not_check_output_align = _vendor_ai_common_info()->not_check_output_align ;
		vendor_ai_net_trace(proc_id, AI_FLOW, "set() - not_check_input_align(%u) not_check_output_align(%u)\r\n", p_proc->not_check_input_align, p_proc->not_check_output_align);
	}
	
	// handle debug ctrl
	{
		UINT32 debug_value = 0;

		// get current debug option (from echo cmd)
		ret = _vendor_ai_net_get_debug(proc_id, &debug_value);
		if (HD_OK != ret) {
			printf("proc_id(%u) _vendor_ai_net_get_debug fail(%d) !!\n", proc_id, ret);
			goto failed;
		}

		// backup original debug_mask, will be used to recover at close()
		p_proc->debug_mask = debug_value;
		p_proc->debug_value = 0;

		// check if DEBUG for dump out
		if (p_proc_cfg->ctrl & CTRL_JOB_DUMPOUT) {
			vendor_ai_cmd_set_iomem_debug(proc_id, "clear_iobuf", "on");
			debug_value |= KFLOW_AI_DBG_OBUF;
		}

		// check if DEBUG for command dump
		if (p_proc_cfg->ctrl & CTRL_JOB_DUMPCMD) {
			debug_value |= KFLOW_AI_DBG_CTX;
		}

		// check if DEBUG for perf time
		if (p_proc_cfg->ctrl & CTRL_JOB_PERFTIME) {
			debug_value |= KFLOW_AI_DBG_TIME;
		}

		// check if DEBUG for perf ut
		if (p_proc_cfg->ctrl & CTRL_JOB_PERFBW) {
			VENDOR_AI_NET_CFG_WORKBUF *p_workbuf = (VENDOR_AI_NET_CFG_WORKBUF *)&p_proc->workbuf;
			UINT32 ddr = (p_workbuf->pa > 0xffffffff) ? 1 : 0; // if pa > 4G, it is located at ddr2.
			debug_value |= KFLOW_AI_DBG_BW;
			debug_value |= (ddr << 24); //set DDR
		}

		//DBG_DUMP("set debug =%x\r\n", debug_value);
		// set final debug mask 
        if(p_proc->debug_mask != debug_value) {
		    _vendor_ai_net_set_debug(proc_id, debug_value);
        }
		p_proc->debug_value = debug_value; //record apply value
		
	}

    //call op open for float in, float out op
    {
        if (p_proc_cfg->ctrl & CTRL_BUF_FLOATIN || p_proc_cfg->ctrl & CTRL_BUF_FLOATOUT) {
            UINT32 tmp_proc_id;
            VENDOR_AI_OP_CFG_WORKBUF wbuf = {0};
            ret = vendor_ai_get_id(&tmp_proc_id);
            if (ret != HD_OK) {
                DBG_ERR("float_in/float_out: get tmp_proc_id failed !! ret = %d\r\n", (int)ret);
                goto failed;
            }
            p_proc->tmp_proc_id = tmp_proc_id;
            vendor_ais_lock_net(tmp_proc_id);

			if (p_proc_cfg->ctrl & CTRL_JOB_DEBUG) {
				// cfg_job_opt.method = VENDOR_AI_NET_JOB_OPT_LINEAR; // debug mode, use jobopt = 0
				// cfg_job_opt.wait_ms = -1;                          // debug mode, use wait_ms = (-1)
        		g_is_job_debug_mode[p_proc->tmp_proc_id] = 1;
            	vendor_ai_dla_set_job_debug_mode(p_proc->tmp_proc_id);
            	// cfg_job_opt.schd_parm = (VENDOR_AI_DLA_CORE(0) << max_isp); //bind core 0 in debug mode
				DBG_DUMP("\n[ai][lib][comm] >>>>>>>>>>>>>>>>>>>>>>>>>>>  WARNNING: proc_id(%d), debug mode is enabled, force bind core 0!!  <<<<<<<<<<<<<<<<<<<<<<<<<<<<<\n\n", (int)p_proc->tmp_proc_id);
        	}

            ret = vendor_ai_op_open2(tmp_proc_id) ;
            if (ret != HD_OK) {
                DBG_ERR("float_in/float_out: vendor_ai_op_open2 fail, ret = %d\n", (int)ret);
                vendor_ais_unlock_net(tmp_proc_id);
                vendor_ai_release_id(tmp_proc_id);
                goto failed;
            }

            wbuf.pa = p_proc->float_cmd_buf.pa;
            wbuf.va = p_proc->float_cmd_buf.va;
            wbuf.size = p_proc->float_cmd_buf.size;
            wbuf.op = VENDOR_AI_OP_FLOAT2FIXED;
            ret = vendor_ai_op_set(tmp_proc_id, VENDOR_AI_OP_PARAM_CFG_WORKBUF, &wbuf);
            if (ret != HD_OK) {
                DBG_ERR("float_in/float_out: vendor_ai_op_set fail, ret = %d\n", (int)ret);
                vendor_ai_op_close2(tmp_proc_id);
                vendor_ais_unlock_net(tmp_proc_id);
                vendor_ai_release_id(tmp_proc_id);
                goto failed;
            }
			
			{
				UINT32 debug_value = 0;
				VENDOR_AI_NET_INFO_PROC *tmp_p_proc = NULL;
				tmp_p_proc = _vendor_ai_info(p_proc->tmp_proc_id);

				// get current debug option (from echo cmd)
				ret = _vendor_ai_net_get_debug(p_proc->tmp_proc_id, &debug_value);
				if (HD_OK != ret) {
					printf("proc_id(%u) _vendor_ai_net_get_debug fail(%d) !!\n", proc_id, ret);
					vendor_ai_op_close2(tmp_proc_id);
					vendor_ais_unlock_net(tmp_proc_id);
					vendor_ai_release_id(tmp_proc_id);
					goto failed;
				}

				tmp_p_proc->debug_value = 0;
				tmp_p_proc->mode = p_proc_cfg->mode;

				// check if DEBUG for perf ut
				if ((p_proc_cfg->ctrl & CTRL_JOB_PERFTIME) && (p_proc_cfg->ctrl & CTRL_JOB_PERFBW)) {
					VENDOR_AI_NET_CFG_WORKBUF *p_workbuf = (VENDOR_AI_NET_CFG_WORKBUF *)&p_proc->workbuf;
					UINT32 ddr = (p_workbuf->pa > 0xffffffff) ? 1 : 0; // if pa > 4G, it is located at ddr2.
					debug_value |= KFLOW_AI_DBG_BW|KFLOW_AI_DBG_TIME;
					debug_value |= (ddr << 24); //set DDR
				}
                //check if DEBUG for timeline, force dump timeline_all
                if (p_proc->debug_value & KFLOW_AI_DBG_TIMELINE) {
                    p_proc->force_timeline_all = 1;
                    g_debug_value |= KFLOW_AI_DBG_TIMELINE_ALL;
                    _vendor_ai_net_set_debug(0xffffffff, g_debug_value); //set debug all
                    vendor_ai3_dev_perf_begin(VENDOR_AI3_PERF_ID_TIMELINE); //call vendor_ai3_dev_perf_begin to reset jobm clock
                    PERF_TIME_BEGIN(&ts_start); //start record open time from here
                }
				tmp_p_proc->debug_value = debug_value; //record apply value
			}
        }
    }

    PERF_TIME_END(proc_id, PERF_NET_OPEN, ts_start, 0);
	return HD_OK; // normal success return

failed:
	// release proc_id because open failed
	if (HD_OK != vendor_ai_release_id(proc_id)) {
		DBG_ERR("release proc_id(%d) failed !! ...\r\n", (int)*p_proc_id);
	}
	// NOTE : p_proc will be alloc at vendor_ai_net_open(), so if anything error before open() and goto failed => p_proc will be NULL
	if (p_proc) {
		if(p_proc->net_info.out_path_list)
			vendor_ai_free(p_proc->net_info.out_path_list, sizeof(UINT32) * (p_proc->net_info.out_buf_cnt)) ;
		if(p_proc->net_info.in_path_list)
			vendor_ai_free(p_proc->net_info.in_path_list, sizeof(UINT32) * (p_proc->net_info.in_buf_cnt)) ;
	}
	return ret;
}

HD_RESULT vendor_ai3_net_close (UINT32 proc_id)
{
	HD_RESULT ret = HD_OK;
	VENDOR_AI_NET_INFO_PROC *p_proc = NULL;
	VENDOR_AI_NET_CFG_PROC *p_cfg = NULL;
    UINT64 ts_start = 0;
    PERF_TIME_BEGIN(&ts_start);

	ret = _vendor_ai_validate(proc_id);
	if (ret != HD_OK) {
		return ret;
	}

	p_proc =  _vendor_ai_info(proc_id);
	p_cfg  =  _vendor_ai_cfg(proc_id);

	if (p_proc == NULL) {
		DBG_ERR("proc_id(%d) p_proc = NULL ?\n", proc_id);
		return HD_ERR_FAIL;
	}
	if (p_cfg == NULL) {
		DBG_ERR("proc_id(%d) p_cfg = NULL ?\n", proc_id);
		return HD_ERR_FAIL;
	}

    p_cfg = _vendor_ai_cfg(proc_id);

    if (p_cfg == NULL) {
		DBG_ERR("proc_id(%d) p_cfg = NULL ?\n", proc_id);
		return HD_ERR_FAIL;
	}

    if ((p_cfg->buf_ctrl.ctrl & CTRL_BUF_FLOATIN) == CTRL_BUF_FLOATIN || (p_cfg->buf_ctrl.ctrl & CTRL_BUF_FLOATOUT) == CTRL_BUF_FLOATOUT) {
        
        ret = vendor_ai_op_close2(p_proc->tmp_proc_id);
		g_is_job_debug_mode[p_proc->tmp_proc_id] = 0;

        if (ret != HD_OK) {
            DBG_ERR("float_in/float_out: vendor_ai_op_close2, ret = %d\r\n", (int)ret);
            return ret;
        }

        vendor_ais_unlock_net(p_proc->tmp_proc_id);

        ret = vendor_ai_release_id(p_proc->tmp_proc_id);
        
        if (ret != HD_OK) {
            DBG_ERR("float_in/float_out: release tmp_proc_id(%d) failed, ret = %d\r\n", (int)p_proc->tmp_proc_id, (int)ret);
            return ret;
        }
    }

	if (p_proc->net_info.in_path_list) // free in & out path list
		vendor_ai_free(p_proc->net_info.in_path_list, sizeof(UINT32) * (p_proc->net_info.in_buf_cnt));
	if (p_proc->net_info.out_path_list)
		vendor_ai_free(p_proc->net_info.out_path_list, sizeof(UINT32) * (p_proc->net_info.out_buf_cnt));

    //force timeline_all case, call perf_end to dump timeline_all 
    if (p_proc->force_timeline_all) {
        PERF_TIME_END(proc_id, PERF_NET_CLOSE, ts_start, 0);
        vendor_ai3_dev_perf_end(VENDOR_AI3_PERF_ID_TIMELINE, 0);
        g_debug_value &= ~(KFLOW_AI_DBG_TIMELINE_ALL);
        _vendor_ai_net_set_debug(0xffffffff, g_debug_value); //restore debug_value_all
        p_proc->force_timeline_all = 0;
    }
    
	// recover original debug mask (set to original echo cmd debug)
	{
		//DBG_DUMP("restore debug =%x\r\n", p_proc->debug_mask);
        if(p_proc->debug_mask != p_proc->debug_value) {
		    _vendor_ai_net_set_debug(proc_id, p_proc->debug_mask);
        }
	}
    g_is_job_debug_mode[proc_id] = 0;
	g_is_aiisp_model[proc_id] = 0;

	// handle plugin engine
	{
		VENDOR_AI_ENGINE_PLUGIN *p_cfg_engine;
		UINT32 i = 0;
		for (i = 0; i < VENDOR_AI_COUNT; i++) {
			p_cfg_engine = (VENDOR_AI_ENGINE_PLUGIN *)&p_cfg->engine_plug[i];
			UINT32 engine_id = i;

			if (p_cfg_engine && p_cfg_engine->proc_cb != 0) {
				switch (engine_id) {
				case AI3_PLUGIN_CPU:
					// call plugin engine close
					ret = p_cfg_engine->proc_cb(proc_id, VENDOR_AI_CTRL_LYR, VENDOR_AI_CTRL_NET_CLOSE, 0, 0);
					if (ret != HD_OK) {
						DBG_ERR("proc_id(%d) proc_cb close fail...\r\n", (int)proc_id);
					}
					break;
				case AI3_PLUGIN_DSP:
					break;
				default:
					DBG_ERR("AI-open: plugin engine: not support type = %d!\r\n", (int)engine_id+1);
					continue;
				}
			}
		}
	}

	// NOTE : vendor_ai_net_close() will memset(0) to p_proc , if you want to use p_proc MUST before vendor_ai_net_close()

	ret = vendor_ai_net_close(proc_id);
	if (ret != HD_OK) {
		DBG_ERR("vendor_ai_net_close(%d) failed, ret = %d\r\n", (int)proc_id, (int)ret);
		return ret;
	}

    ret = vendor_ai_release_id(proc_id);
	if (ret != HD_OK) {
		DBG_ERR("release proc_id(%d) failed, ret = %d\r\n", (int)proc_id, (int)ret);
		return ret;
	}
	
    PERF_TIME_END(proc_id, PERF_NET_CLOSE, ts_start, 0);
	return HD_OK;
}

HD_RESULT vendor_ai3_net_start (UINT32 proc_id)
{
	HD_RESULT ret = HD_OK;
    VENDOR_AI_NET_INFO_PROC *p_proc = NULL;
    VENDOR_AI_NET_CFG_PROC *p_cfg = NULL;
    UINT64 ts_start = 0;
    PERF_TIME_BEGIN(&ts_start);

    p_proc =  _vendor_ai_info(proc_id);

	if (p_proc == NULL) {
		DBG_ERR("proc_id(%d) p_proc = NULL ?\n", proc_id);
		return HD_ERR_FAIL;
	}

    p_cfg = _vendor_ai_cfg(proc_id);

    if (p_cfg == NULL) {
		DBG_ERR("proc_id(%d) p_cfg = NULL ?\n", proc_id);
		return HD_ERR_FAIL;
	}

	ret = vendor_ai_net_start(proc_id);
	if (ret != HD_OK) {
		DBG_ERR("vendor_ai_net_start(%d) failed, ret = %d\r\n", (int)proc_id, (int)ret);
		return ret;
	}

    if ((p_cfg->buf_ctrl.ctrl & CTRL_BUF_FLOATIN) == CTRL_BUF_FLOATIN || (p_cfg->buf_ctrl.ctrl & CTRL_BUF_FLOATOUT) == CTRL_BUF_FLOATOUT){
        ret = vendor_ai_op_start(p_proc->tmp_proc_id) ;
        if (ret != HD_OK) {
            DBG_ERR("float_in/float_out:vendor_ai_op_start fail, ret = %d\n", (int)ret);
            vendor_ai_net_stop(proc_id);
            return ret;
        }
    }

    PERF_TIME_END(proc_id, PERF_NET_START, ts_start, 0);
	return ret;
}

HD_RESULT vendor_ai3_net_stop (UINT32 proc_id)
{
	HD_RESULT ret = HD_OK;
    VENDOR_AI_NET_INFO_PROC *p_proc = NULL;
    VENDOR_AI_NET_CFG_PROC *p_cfg = NULL;
    UINT64 ts_start = 0;
    PERF_TIME_BEGIN(&ts_start);

    p_proc =  _vendor_ai_info(proc_id);

	if (p_proc == NULL) {
		DBG_ERR("proc_id(%d) p_proc = NULL ?\n", proc_id);
		return HD_ERR_FAIL;
	}

    p_cfg = _vendor_ai_cfg(proc_id);

    if (p_cfg == NULL) {
		DBG_ERR("proc_id(%d) p_cfg = NULL ?\n", proc_id);
		return HD_ERR_FAIL;
	}

    if ((p_cfg->buf_ctrl.ctrl & CTRL_BUF_FLOATIN) == CTRL_BUF_FLOATIN || (p_cfg->buf_ctrl.ctrl & CTRL_BUF_FLOATOUT) == CTRL_BUF_FLOATOUT){
        ret = vendor_ai_op_stop(p_proc->tmp_proc_id);
        if (ret != HD_OK) {
            DBG_ERR("vendor_ai_op_stop fail=%d\n", ret);
            return ret;
        }
    }

	ret = vendor_ai_net_stop(proc_id);
	if (ret != HD_OK) {
		DBG_ERR("vendor_ai_net_stop(%d) failed, ret = %d\r\n", (int)proc_id, (int)ret);
		return ret;
	}

    PERF_TIME_END(proc_id, PERF_NET_STOP, ts_start, 0);
	return ret;
}

HD_RESULT vendor_ai3_net_proc (UINT32 proc_id)
{
	HD_RESULT ret = HD_OK;
    UINT64 ts_start = 0;
    PERF_TIME_BEGIN(&ts_start);
	ret = vendor_ai_net_proc(proc_id);
	if (ret != HD_OK) {
		DBG_ERR("vendor_ai_net_proc(%d) failed, ret = %d\r\n", (int)proc_id, (int)ret);
		return ret;
	}
    PERF_TIME_END(proc_id, PERF_NET_PROC, ts_start, 0);
	return ret;
}
HD_RESULT vendor_ai3_to_ai3buf (VENDOR_AI_BUF* ai_buf, VENDOR_AI3_BUF* ai3_buf)
{
	HD_RESULT ret = HD_OK;
	ai3_buf->sign = ai_buf->sign ;
	ai3_buf->chunk_size = ai_buf->chunk_size ;
	ai3_buf->pa = ai_buf->pa ;
	ai3_buf->va = ai_buf->va ;
	ai3_buf->size = ai_buf->size ;
	ai3_buf->fmt = ai_buf->fmt ;
	ai3_buf->width = ai_buf->width ;
	ai3_buf->height = ai_buf->height ;
	ai3_buf->channel = ai_buf->channel ;
	ai3_buf->batch_num = ai_buf->batch_num ;
	ai3_buf->time = ai_buf->time ;
	ai3_buf->reserve = ai_buf->reserve ;
	ai3_buf->scale_ratio = ai_buf->scale_ratio ;
	ai3_buf->line_ofs = ai_buf->line_ofs ;
	ai3_buf->channel_ofs = ai_buf->channel_ofs ;
	ai3_buf->batch_ofs = ai_buf->batch_ofs ;
	ai3_buf->time_ofs = ai_buf->time_ofs ;
	ai3_buf->zero_point = ai_buf->zero_point ;
	memcpy(ai3_buf->layout, ai_buf->layout, sizeof(ai_buf->layout));
	ai3_buf->name = ai_buf->name ;

	return ret;
}
HD_RESULT vendor_ai3_to_aibuf (VENDOR_AI3_BUF* ai3_buf, VENDOR_AI_BUF* ai_buf)
{
	HD_RESULT ret = HD_OK;
	
	ai_buf->sign = ai3_buf->sign ;
	ai_buf->chunk_size = ai3_buf->chunk_size ;
	ai_buf->pa = ai3_buf->pa ;
	ai_buf->va = ai3_buf->va ;
	ai_buf->size = ai3_buf->size ;
	ai_buf->fmt = ai3_buf->fmt ;
	ai_buf->width = ai3_buf->width ;
	ai_buf->height = ai3_buf->height ;
	ai_buf->channel = ai3_buf->channel ;
	ai_buf->batch_num = ai3_buf->batch_num ;
	ai_buf->time = ai3_buf->time ;
	ai_buf->reserve = ai3_buf->reserve ;
	ai_buf->scale_ratio = ai3_buf->scale_ratio ;
	ai_buf->line_ofs = ai3_buf->line_ofs ;
	ai_buf->channel_ofs = ai3_buf->channel_ofs ;
	ai_buf->batch_ofs = ai3_buf->batch_ofs ;
	ai_buf->time_ofs = ai3_buf->time_ofs ;
	ai_buf->zero_point = ai3_buf->zero_point ;
	memcpy(ai_buf->layout, ai3_buf->layout, sizeof(ai3_buf->layout));
	ai_buf->name = ai3_buf->name ;

	return ret;
}
HD_RESULT vendor_ai3_net_set (UINT32 proc_id, VENDOR_AI3_NET_PARAM_ID param_id, void* p_param)
{
    HD_RESULT ret = HD_OK;
    UINT64 ts_start = 0;
    PERF_TIME_BEGIN(&ts_start);
	if (p_param == NULL) {		DBG_ERR("check p_param == NULL !!\r\n");		return HD_ERR_INV;	}

	switch (param_id)
	{
		case VENDOR_AI3_NET_PARAM_SHAPE_DIM_IMM:	ret = vendor_ai_net_set(proc_id, VENDOR_AI_NET_PARAM_RES_DIM_IMM, p_param); break;
		case VENDOR_AI3_NET_PARAM_BATCH_N_IMM:		ret = vendor_ai_net_set(proc_id, VENDOR_AI_NET_PARAM_BATCH_N_IMM, p_param); break;
		case VENDOR_AI3_NET_PARAM_CUSTOM_INFO:		ret = vendor_ai_net_set(proc_id, VENDOR_AI_NET_PARAM_CUSTOM_INFO, p_param); break;
        case VENDOR_AI3_NET_PARAM_JOB_PRI:		    ret = vendor_ai_net_set(proc_id, VENDOR_AI_NET_PARAM_JOB_PRI, p_param); break;
		case VENDOR_AI3_NET_PARAM_CORE_MASK:		ret = vendor_ai_net_set(proc_id, VENDOR_AI_NET_PARAM_CORE_MASK, p_param); break;
		case VENDOR_AI3_NET_PARAM_UBUF_MASK:		ret = vendor_ai_net_set(proc_id, VENDOR_AI_NET_PARAM_UBUF_MASK, p_param); break;
		case VENDOR_AI3_NET_PARAM_CFG_WORKBUF_IMM:  ret = vendor_ai_net_set(proc_id, VENDOR_AI_NET_PARAM_CFG_WORKBUF_IMM, p_param); break;
		default:
			switch(VENDOR_AI_GET_PARAM_TYPE(param_id))
			{
				case VENDOR_AI_PARAM_TYPE_LAYER:
				case VENDOR_AI_PARAM_BUF_ID:
				case VENDOR_AI_PARAM_EXT_ID:
				case VENDOR_AI_PARAM_OUT_ID:
				case VENDOR_AI_PARAM_TYPE_IN:
				case VENDOR_AI_PARAM_TYPE_OUT:
				{
					VENDOR_AI_BUF ai_buf = {0};
					vendor_ai3_to_aibuf((VENDOR_AI3_BUF*)p_param, &ai_buf) ;
					ret = vendor_ai_net_set(proc_id, param_id, (void*)&ai_buf);  
				}
                break;
				default:
				{
					DBG_ERR("param_id(%u) not support !!\r\n", param_id);
					return HD_ERR_PARAM;
				}
				break;
			}
		break;
	}
    PERF_TIME_END(proc_id, PERF_NET_SET, ts_start, &param_id);
	return ret;
}

HD_RESULT vendor_ai3_net_get (UINT32 proc_id, VENDOR_AI3_NET_PARAM_ID param_id, void* p_param)
{
	HD_RESULT ret = HD_OK;
    UINT64 ts_start = 0;
    PERF_TIME_BEGIN(&ts_start);

	if (p_param == NULL) {		DBG_ERR("check p_param == NULL !!\r\n");		return HD_ERR_INV;	}

	switch (param_id)
	{
		case VENDOR_AI3_NET_PARAM_SHAPE_DIM_IMM:	ret = vendor_ai_net_get(proc_id, VENDOR_AI_NET_PARAM_RES_DIM_IMM, p_param); break;
		case VENDOR_AI3_NET_PARAM_BATCH_N_IMM:		ret = vendor_ai_net_get(proc_id, VENDOR_AI_NET_PARAM_BATCH_N_IMM, p_param); break;
		default:
			switch(VENDOR_AI_GET_PARAM_TYPE(param_id))
			{
				case VENDOR_AI_PARAM_TYPE_LAYER:
				case VENDOR_AI_PARAM_BUF_ID:
				case VENDOR_AI_PARAM_EXT_ID:
				case VENDOR_AI_PARAM_OUT_ID:
				case VENDOR_AI_PARAM_TYPE_IN:
				case VENDOR_AI_PARAM_TYPE_OUT:
				{
					VENDOR_AI_BUF ai_buf = {0};
					ret = vendor_ai_net_get(proc_id, param_id, (void*)&ai_buf);  
					vendor_ai3_to_ai3buf(&ai_buf,(VENDOR_AI3_BUF*)p_param) ;
				}
                break;
				default:
				{
					DBG_ERR("param_id(%u) not support !!\r\n", param_id);
					return HD_ERR_PARAM;
				}
				break;
			}
		break;
	}
    PERF_TIME_END(proc_id, PERF_NET_GET, ts_start, &param_id);
	return ret;
}

HD_RESULT vendor_ai3_op_eval_preprocess (VENDOR_AI3_OP_PREPROC_MEM* mem)
{
	mem->buf.size = ALIGN_CEIL_64(sizeof(VENDOR_AI_OP_WORK)) ; 
	return HD_OK;

}

#define PREPROC_FMT_MASK 0x7fff0fff
HD_RESULT vendor_ai_check_preproc_format(VENDOR_AI3_BUF* src, VENDOR_AI3_BUF* dest, UINT32 scale_w, UINT32 scale_h){
	// YUV
	if (((src->fmt & PREPROC_FMT_MASK) == HD_VIDEO_PXLFMT_Y8) && (((src + 1)->fmt & PREPROC_FMT_MASK) == HD_VIDEO_PXLFMT_UV)){
		if ((src->width > 4096) || ((src + 1)->width > 4096)){
			DBG_ERR("Error! preproc (in_width1(%d), in_width2(%d)), the in_width must be <= (%d)\r\n", src->width, (src + 1)->width, 4096);
			return HD_ERR_LIMIT;
		}
		if ((src->height > 4096) || ((src + 1)->height > 4096)){
			DBG_ERR("Error! preproc (in_height1(%d), in_height2(%d)), the in_height must be <= (%d)\r\n", src->height, (src + 1)->height, 4096);
			return HD_ERR_LIMIT;
		}
		if ((src->width < 2) || ((src + 1)->width < 2)){
			DBG_ERR("Error! preproc (in_width1(%d), in_width2(%d)), the in_width must be >= (%d)\r\n", src->width, (src + 1)->width, 2);
			return HD_ERR_LIMIT;
		}
		if ((src->height < 2) || ((src + 1)->height < 2)){
			DBG_ERR("Error! preproc (in_height1(%d), in_height2(%d)), the in_height must be <= (%d)\r\n", src->height, (src + 1)->height, 2);
			return HD_ERR_LIMIT;
		}
	// Y only/UV packed
	}else if (((src->fmt & PREPROC_FMT_MASK) == HD_VIDEO_PXLFMT_Y8) || ((src->fmt & PREPROC_FMT_MASK) == HD_VIDEO_PXLFMT_UV)){
		if ((src->width > 4096) || (src->width < 2)){
			DBG_ERR("Error! preproc in_width1(%d), the in_width must be (%d) <= in_width1 <= (%d)\r\n", 2, src->width, 4096);
			return HD_ERR_LIMIT;
		}
		if ((src->height > 4096) || (src->height < 2)){
			DBG_ERR("Error! preproc in_height1(%d), the in_height must be (%d) <= in_height1 <= (%d)\r\n", 2, src->height, 4096);
			return HD_ERR_LIMIT;
		}
	// RGB
	}else{
		if ((src->width > 4096) || ((src + 1)->width > 4096) || ((src + 2)->width > 4096)){
			DBG_ERR("Error! preproc (in_width1(%d), in_width2(%d), in_width3(%d)), the in_width must be <= (%d)\r\n", src->width, (src + 1)->width, (src + 2)->width, 4096);
			return HD_ERR_LIMIT;
		}
		if ((src->height > 4096) || ((src + 1)->height > 4096) || ((src + 2)->height > 4096)){
			DBG_ERR("Error! preproc (in_height1(%d), in_height2(%d), in_height3(%d)), the in_height must be <= (%d)\r\n", src->height, (src + 1)->height, (src + 2)->height, 4096);
			return HD_ERR_LIMIT;
		}
		if ((src->width < 2) || ((src + 1)->width < 2) || ((src + 2)->width < 2)){
			DBG_ERR("Error! preproc (in_width1(%d), in_width2(%d), in_width3(%d)), the in_width must be >= (%d)\r\n", src->width, (src + 1)->width, (src + 2)->width, 2);
			return HD_ERR_LIMIT;
		}
		if ((src->height < 2) || ((src + 1)->height < 2) || ((src + 2)->height < 2)){
			DBG_ERR("Error! preproc (in_height1(%d), in_height2(%d), in_height3(%d)), the in_height must be <= (%d)\r\n", src->height, (src + 1)->height, (src + 2)->height, 2);
			return HD_ERR_LIMIT;
		}
	}

	//scale-up case
	if (scale_w && scale_h){
		// YUV
		if (((src->fmt & PREPROC_FMT_MASK) == HD_VIDEO_PXLFMT_Y8) && (((src + 1)->fmt & PREPROC_FMT_MASK) == HD_VIDEO_PXLFMT_UV)){
			if ((scale_w > src->width) && (src->width > 2048)){
				DBG_ERR("Error! scaling up case (in_width1(%d), scale_width1(%d)), the in_width must be <= (%d)\r\n", src->width, scale_w, 2048);
				return HD_ERR_LIMIT;
			}
			if ((scale_w > (src + 1)->width) && ((src + 1)->width > 2048)){
				DBG_ERR("Error! scaling up case (in_width2(%d), scale_width2(%d)), the in_width must be <= (%d)\r\n", (src + 1)->width, scale_w, 2048);
				return HD_ERR_LIMIT;
			}
			if ((scale_h > src->height) && (src->height > 2048)){
				DBG_ERR("Error! scaling up case (in_height1(%d), scale_height1(%d)), the in_height must be <= (%d)\r\n", src->height, scale_h, 2048);
				return HD_ERR_LIMIT;
			}
			if ((scale_h > (src + 1)->height) && ((src + 1)->height > 2048)){
				DBG_ERR("Error! scaling up case (in_height2(%d), scale_height2(%d)), the in_height must be <= (%d)\r\n", (src + 1)->height, scale_h, 2048);
				return HD_ERR_LIMIT;
			}
		// Y only/UV packed
		}else if (((src->fmt & PREPROC_FMT_MASK) == HD_VIDEO_PXLFMT_Y8) || ((src->fmt & PREPROC_FMT_MASK) == HD_VIDEO_PXLFMT_UV)){
			if ((scale_w > src->width) && (src->width > 2048)){
				DBG_ERR("Error! scaling up case (in_width1(%d), scale_width1(%d)), the in_width must be <= (%d)\r\n", src->width, scale_w, 2048);
				return HD_ERR_LIMIT;
			}
			if ((scale_h > src->height) && (src->height > 2048)){
				DBG_ERR("Error! scaling up case (in_height1(%d), scale_height1(%d)), the in_height must be <= (%d)\r\n", src->height, scale_h, 2048);
				return HD_ERR_LIMIT;
			}
		// RGB
		}else{
			if ((scale_w > src->width) && (src->width > 2048)){
				DBG_ERR("Error! scaling up case (in_width1(%d), scale_width1(%d)), the in_width must be <= (%d)\r\n", src->width, scale_w, 2048);
				return HD_ERR_LIMIT;
			}
			if ((scale_w > (src + 1)->width) && ((src + 1)->width > 2048)){
				DBG_ERR("Error! scaling up case (in_width2(%d), scale_width2(%d)), the in_width must be <= (%d)\r\n", (src + 1)->width, scale_w, 2048);
				return HD_ERR_LIMIT;
			}
			if ((scale_w > (src + 2)->width) && ((src + 2)->width > 2048)){
				DBG_ERR("Error! scaling up case (in_width3(%d), scale_width3(%d)), the in_width must be <= (%d)\r\n", (src + 2)->width, scale_w, 2048);
				return HD_ERR_LIMIT;
			}
			if ((scale_h > src->height) && (src->height > 2048)){
				DBG_ERR("Error! scaling up case (in_height1(%d), scale_height1(%d)), the in_height must be <= (%d)\r\n", src->height, scale_h, 2048);
				return HD_ERR_LIMIT;
			}
			if ((scale_h > (src + 1)->height) && ((src + 1)->height > 2048)){
				DBG_ERR("Error! scaling up case (in_height2(%d), scale_height2(%d)), the in_height must be <= (%d)\r\n", (src + 1)->height, scale_h, 2048);
				return HD_ERR_LIMIT;
			}
			if ((scale_h > (src + 2)->height) && ((src + 2)->height > 2048)){
				DBG_ERR("Error! scaling up case (in_height3(%d), scale_height3(%d)), the in_height must be <= (%d)\r\n", (src + 2)->height, scale_h, 2048);
				return HD_ERR_LIMIT;
			}
		}

		// scale check
		if ((scale_w > 4096) || (scale_h > 4096)){
			DBG_ERR("Error! scaling case (scale_width(%d), scale_height(%d)), the value must be <= (%d)\r\n", scale_w, scale_h, 4096);
			return HD_ERR_LIMIT;
		}
		if ((scale_w < 2) || (scale_h < 2)){
			DBG_ERR("Error! scaling case (scale_width(%d), scale_height(%d)), the value must be >= (%d)\r\n", scale_w, scale_h, 2);
			return HD_ERR_LIMIT;
		}
	}

	// YUV
	if (((dest->fmt & PREPROC_FMT_MASK) == HD_VIDEO_PXLFMT_Y8) && (((dest + 1)->fmt & PREPROC_FMT_MASK) == HD_VIDEO_PXLFMT_UV)){
		if ((dest->width > 4096) || ((dest + 1)->width > 4096)){
			DBG_ERR("Error! preproc (out_width1(%d), out_width2(%d)), the out_width must be <= (%d)\r\n", dest->width, (dest + 1)->width, 4096);
			return HD_ERR_LIMIT;
		}
		if ((dest->height > 4096) || ((dest + 1)->height > 4096)){
			DBG_ERR("Error! preproc (out_height1(%d), out_height2(%d)), the out_height must be <= (%d)\r\n", dest->height, (dest + 1)->height, 4096);
			return HD_ERR_LIMIT;
		}
		if ((dest->width < 2) || ((dest + 1)->width < 2)){
			DBG_ERR("Error! preproc (out_width1(%d), out_width2(%d)), the out_width must be >= (%d)\r\n", dest->width, (dest + 1)->width, 2);
			return HD_ERR_LIMIT;
		}
		if ((dest->height < 2) || ((dest + 1)->height < 2)){
			DBG_ERR("Error! preproc (out_height1(%d), out_height2(%d)), the out_height must be >= (%d)\r\n", dest->height, (dest + 1)->height, 2);
			return HD_ERR_LIMIT;
		}
	// Y only/UV packed
	}else if (((dest->fmt & PREPROC_FMT_MASK) == HD_VIDEO_PXLFMT_Y8) || ((dest->fmt & PREPROC_FMT_MASK) == HD_VIDEO_PXLFMT_UV)){
		if ((dest->width > 4096) || (dest->width < 2)){
			DBG_ERR("Error! preproc out_width1(%d), the out_width must be (%d) <= out_width1 <= (%d)\r\n", 2, dest->width, 4096);
			return HD_ERR_LIMIT;
		}
		if ((dest->height > 4096) || (dest->height < 2)){
			DBG_ERR("Error! preproc out_height1(%d), the out_height must be (%d) <= out_height1 <= (%d)\r\n", 2, dest->height, 4096);
			return HD_ERR_LIMIT;
		}
	// RGB
	}else{
		if ((dest->width > 4096) || ((dest + 1)->width > 4096) || ((dest + 2)->width > 4096)){
			DBG_ERR("Error! preproc (out_width1(%d), out_width2(%d), out_width3(%d)), the out_width must be <= (%d)\r\n", dest->width, (dest + 1)->width, (dest + 2)->width, 4096);
			return HD_ERR_LIMIT;
		}
		if ((dest->height > 4096) || ((dest + 1)->height > 4096) || ((dest + 2)->height > 4096)){
			DBG_ERR("Error! preproc (out_height1(%d), out_height2(%d), out_height3(%d)), the out_height must be <= (%d)\r\n", dest->height, (dest + 1)->height, (dest + 2)->height, 4096);
			return HD_ERR_LIMIT;
		}
		if ((dest->width < 2) || ((dest + 1)->width < 2) || ((dest + 2)->width < 2)){
			DBG_ERR("Error! preproc (out_width1(%d), out_width2(%d), out_width3(%d)), the out_width must be >= (%d)\r\n", dest->width, (dest + 1)->width, (dest + 2)->width, 2);
			return HD_ERR_LIMIT;
		}
		if ((dest->height < 2) || ((dest + 1)->height < 2) || ((dest + 2)->height < 2)){
			DBG_ERR("Error! preproc (out_height1(%d), out_height2(%d), out_height3(%d)), the out_height must be <= (%d)\r\n", dest->height, (dest + 1)->height, (dest + 2)->height, 2);
			return HD_ERR_LIMIT;
		}
	}
	
	return HD_OK;
}

HD_RESULT vendor_ai3_op_init_preprocess (UINT32* p_op_id, VENDOR_AI3_OP_PREPROC_CFG* cfg)
{
    HD_RESULT ret = HD_OK;
	UINT32 proc_id = 0;

	if (cfg->op_mem.buf.pa == 0 || cfg->op_mem.buf.va == 0 || cfg->op_mem.buf.size == 0) {
		DBG_ERR("check cfg->op_mem.buf fail !! cfg->op_mem.buf (pa/va/size) = (0x%016lx, 0x%016lx, %d)\r\n", cfg->op_mem.buf.pa, cfg->op_mem.buf.va, (int)cfg->op_mem.buf.size);
        goto exit;
	}
	if (p_op_id  == NULL) {		DBG_ERR("check p_op_id == NULL !!\r\n");		return HD_ERR_INV;	}

	// config share model mode 2
	{
		UINT32 share_mode = 2;
		vendor_ai_cfg_set(VENDOR_AI_CFG_SHAREMODEL_MODE, &share_mode);
	}
	// config multi_thread 1
	{
		UINT32 multi_thread = 1;
		vendor_ai_cfg_set(VENDOR_AI_CFG_MULTI_THREAD, &multi_thread);
	}

	ret = vendor_ai_init();
	if (ret != HD_OK) {
		DBG_ERR("vendor_ai_init fail=%d\n", (int)ret);
		goto exit;
	}
	
	ret = vendor_ai_get_id(&proc_id);
	if (ret != HD_OK) {
		DBG_ERR("get proc_id failed !! ret = %d\r\n", (int)ret);
		goto uninit;
	}		

	ret = vendor_ais_lock_net(proc_id);
    if (ret != HD_OK) {
		DBG_ERR("vendor_ais_lock_net failed !! ret = %d\r\n", (int)ret);
		goto release;
	}		

	{
		ret = vendor_ai_op_open(proc_id) ; 
		if (ret != HD_OK) {
			DBG_ERR("vendor_ai_op_open fail=%d\n", (int)ret);
			goto unlock;
		}

		VENDOR_AI_OP_CFG_WORKBUF wbuf = {0};
		wbuf.pa = cfg->op_mem.buf.pa;
		wbuf.va = cfg->op_mem.buf.va;
		wbuf.size = cfg->op_mem.buf.size;
		wbuf.op = VENDOR_AI_OP_PREPROC;
		ret = vendor_ai_op_set(proc_id, VENDOR_AI_OP_PARAM_CFG_WORKBUF, &wbuf);
		if (ret != HD_OK) {
			DBG_ERR("vendor_ai_op_set fail=%d\n", (int)ret);
			goto close;
		}
		ret = vendor_ai_op_start(proc_id) ;
		if (ret != HD_OK) {
			DBG_ERR("vendor_ai_op_start fail=%d\n", (int)ret);
			goto close;
		}
	}

    ret = vendor_ais_unlock_net(proc_id);
    if (ret != HD_OK) {
        DBG_ERR("vendor_ais_unlock_net fail=%d\n", (int)ret);
		goto stop;
    }

    *p_op_id = proc_id;
    return ret;

stop:
	ret = vendor_ai_op_stop(proc_id);
    if (ret != HD_OK) {
        DBG_ERR("vendor_ai_op_stop fail=%d\n", (int)ret);
    }
close:
	ret = vendor_ai_op_close(proc_id);
    if (ret != HD_OK) {
        DBG_ERR("vendor_ai_op_close fail=%d\n", (int)ret);
    }
unlock:
	ret = vendor_ais_unlock_net(proc_id);
    if (ret != HD_OK) {
        DBG_ERR("vendor_ais_unlock_net fail=%d\n", (int)ret);
    }
release:
	ret = vendor_ai_release_id(proc_id);
	if (ret != HD_OK) {
		DBG_ERR("release proc_id(%d) failed, ret = %d\r\n", (int)proc_id, (int)ret);
	}
uninit:
	ret = vendor_ai_uninit();
	if (ret != HD_OK) {
		DBG_ERR("vendor_ai_uninit fail=%d\n", (int)ret);
	}
exit:
	return HD_ERR_INV;
}

HD_RESULT vendor_ai3_op_do_preprocess (UINT32 op_id, VENDOR_AI3_BUF* src, VENDOR_AI3_BUF* dst, VENDOR_AI3_OP_PREPROC_PARAM* param)
{
    HD_RESULT ret = HD_OK;
    VENDOR_AI_OP_PREPROC_PARAM p_parm = {0};

    ret = vendor_ai_check_preproc_format(src, dst, param->scale_dim.w, param->scale_dim.h);
	if (ret != HD_OK) {
		DBG_ERR("vendor_ai_check_preproc_format fail=%d\n", (int)ret);
		return HD_ERR_LIMIT;
	}

	ret = vendor_ais_lock_net(op_id);
    if (ret != HD_OK) {
		DBG_ERR("vendor_ais_lock_net failed !! ret = %d\r\n", (int)ret);
		goto exit;
	}

    if (param->scale_dim.h && param->scale_dim.w) {
        p_parm.scale_dim.w = param->scale_dim.w;
        p_parm.scale_dim.h = param->scale_dim.h;
    }				    
        // plane mode
    if (param->p_out_sub.pa) {
        p_parm.p_out_sub.pa = param->p_out_sub.pa;
        p_parm.p_out_sub.va = param->p_out_sub.va;                       
        p_parm.p_out_sub.width = param->p_out_sub.width;
        p_parm.p_out_sub.height = param->p_out_sub.height;
        p_parm.p_out_sub.line_ofs = param->p_out_sub.line_ofs;
#if defined(_BSP_NS02302_)
		if(g_real_chip_id == NN_CHIP_CNN30_C){
			DBG_ERR("539A doesn't support in plane mode !!\r\n");
			goto exit;

		}
#endif
    }

    // dc mode
    if (param->out_sub_color[0] || param->out_sub_color[1] || param->out_sub_color[2]) {                 
        p_parm.out_sub_color[0] = param->out_sub_color[0];
        p_parm.out_sub_color[1] = param->out_sub_color[1];
        p_parm.out_sub_color[2] = param->out_sub_color[2];
    }

	{
		// AI_OP_PREPROC_UV2UV 
		if (dst[0].fmt == HD_VIDEO_PXLFMT_UV){
			ret = vendor_ai_op_proc(op_id, VENDOR_AI_OP_PREPROC, &p_parm, 1, src, 1, dst);
			if (ret != HD_OK) {
				DBG_ERR("vendor_ai_op_proc fail=%d\n", (int)ret);
				goto exit;
			}
		// AI_OP_PREPROC_Y2Y
		}else if (dst[0].fmt == HD_VIDEO_PXLFMT_Y8){
			ret = vendor_ai_op_proc(op_id, VENDOR_AI_OP_PREPROC, &p_parm, 1, src, 1, dst);
			if (ret != HD_OK) {
				DBG_ERR("vendor_ai_op_proc fail=%d\n", (int)ret);
				goto exit;
			}
			//AI_OP_PREPROC_UV2UV 
			if (dst[1].fmt == HD_VIDEO_PXLFMT_UV){
				p_parm.scale_dim.w = param->scale_dim.w / 2;
				p_parm.scale_dim.h = param->scale_dim.h / 2;
	
				ret = vendor_ai_op_proc(op_id, VENDOR_AI_OP_PREPROC, &p_parm, 1, src+1, 1, dst+1);
				if (ret != HD_OK) {
					DBG_ERR("vendor_ai_op_proc fail=%d\n", (int)ret);
					goto exit;
				}
			}
		// AI_OP_PREPROC_RGB2RGB
		}else if ((dst[0].fmt == HD_VIDEO_PXLFMT_R8) && (dst[1].fmt == HD_VIDEO_PXLFMT_G8) && (dst[2].fmt == HD_VIDEO_PXLFMT_B8) && (dst[0].fmt == src[0].fmt)){
			

			ret = vendor_ai_op_proc(op_id, VENDOR_AI_OP_PREPROC, &p_parm, 3, src, 3, dst);
			if (ret != HD_OK) {
				DBG_ERR("vendor_ai_op_proc fail=%d\n", (int)ret);
				goto exit;
			}
#if defined(_BSP_NS02302_)
			if(g_real_chip_id == NN_CHIP_CNN30_C){
				VENDOR_AI3_BUF src2 = {0}, dst2 = {0} ;
				memcpy(&dst2, dst+2, sizeof(VENDOR_AI3_BUF));
				memcpy(&src2, src+2, sizeof(VENDOR_AI3_BUF));
				src2.fmt = HD_VIDEO_PXLFMT_Y8;
				dst2.fmt = HD_VIDEO_PXLFMT_Y8;
				p_parm.out_sub_color[0] = param->out_sub_color[2];
				ret = vendor_ai_op_proc(op_id, VENDOR_AI_OP_PREPROC, &p_parm, 1, &src2, 1, &dst2);
				if (ret != HD_OK) {
					DBG_ERR("vendor_ai_op_proc fail=%d\n", (int)ret);
					goto exit;

				}
			}
#endif
		}else {
			ret = vendor_ai_op_proc(op_id, VENDOR_AI_OP_PREPROC, &p_parm, 2, src, 3, dst);
			if (ret != HD_OK) {
				DBG_ERR("vendor_ai_op_proc fail=%d\n", (int)ret);
				goto exit;
			}
		}
	}

	ret = vendor_ais_unlock_net(op_id);
    if (ret != HD_OK) {
        DBG_ERR("vendor_ais_unlock_net fail=%d\n", (int)ret);
		goto exit;
    }

    return ret;

exit:
	ret = vendor_ai_op_stop(op_id);
	if (ret != HD_OK) {
        DBG_ERR("vendor_ai_op_stop fail=%d\n", (int)ret);
    }

	ret = vendor_ai_op_close(op_id);
	if (ret != HD_OK) {
        DBG_ERR("vendor_ai_op_close fail=%d\n", (int)ret);
    }

	ret = vendor_ais_unlock_net(op_id);
	if (ret != HD_OK) {
        DBG_ERR("vendor_ais_unlock_net fail=%d\n", (int)ret);
    }

	ret = vendor_ai_release_id(op_id);
	if (ret != HD_OK) {
		DBG_ERR("release proc_id(%d) failed, ret = %d\r\n", (int)op_id, (int)ret);
	}

	ret = vendor_ai_uninit();
	if (ret != HD_OK) {
		DBG_ERR("vendor_ai_uninit fail=%d\n", (int)ret);
	}

    return HD_ERR_INV;
}

HD_RESULT vendor_ai3_op_uninit_preprocess (UINT32 op_id)
{
    HD_RESULT ret = HD_OK;

    ret = vendor_ais_lock_net(op_id);
    if (ret != HD_OK) {
		DBG_ERR("vendor_ais_lock_net failed !! ret = %d\r\n", (int)ret);
	}

    ret = vendor_ai_op_stop(op_id);
    if (ret != HD_OK) {
        DBG_ERR("vendor_ai_op_stop fail=%d\n", (int)ret);
    }

	ret = vendor_ai_op_close(op_id);
    if (ret != HD_OK) {
        DBG_ERR("vendor_ai_op_close fail=%d\n", (int)ret);
    }

	ret = vendor_ais_unlock_net(op_id);
    if (ret != HD_OK) {
        DBG_ERR("vendor_ais_unlock_net fail=%d\n", (int)ret);
    }

	ret = vendor_ai_release_id(op_id);
	if (ret != HD_OK) {
		DBG_ERR("release proc_id(%d) failed, ret = %d\r\n", (int)op_id, (int)ret);
	}
	
	ret = vendor_ai_uninit();
	if (ret != HD_OK) {
		DBG_ERR("vendor_ai_uninit fail=%d\n", (int)ret);
	}
	return ret;
}

HD_RESULT vendor_ai3_eop_open(UINT32* p_op_id, UINT32 op_cmd, VENDOR_AI3_EOP_WORKBUF* buf)
{	
	HD_RESULT ret = HD_OK;
	UINT32 proc_id = 0;

	if (buf->pa == 0 || buf->va == 0 || buf->size == 0) {
		DBG_ERR("check buf fail !! buf (pa/va/size) = (0x%016lx, 0x%016lx, %d)\r\n", buf->pa, buf->va, (int)buf->size);
		return HD_ERR_INV;
	}
	if (p_op_id  == NULL) {		DBG_ERR("check p_op_id == NULL !!\r\n");		return HD_ERR_INV;	}
	if (buf == NULL) {		DBG_ERR("check buf == NULL !!\r\n");		return HD_ERR_INV;	}

	// config share model mode 2
	{
		UINT32 share_mode = 2;
		vendor_ai_cfg_set(VENDOR_AI_CFG_SHAREMODEL_MODE, &share_mode);
	}
	// config multi_thread 1
	{
		UINT32 multi_thread = 1;
		vendor_ai_cfg_set(VENDOR_AI_CFG_MULTI_THREAD, &multi_thread);
	}

	ret = vendor_ai_init();
	if (ret != HD_OK) {
		DBG_ERR("vendor_ai_init fail=%d\n", (int)ret);
		return ret;
	}
	
	ret = vendor_ai_get_id(&proc_id);
	if (ret != HD_OK) {
		DBG_ERR("get proc_id failed !! ret = %d\r\n", (int)ret);
		goto uninit;
	}

	*p_op_id = proc_id;
	ret = vendor_ais_lock_net(proc_id);
    if (ret != HD_OK) {
		DBG_ERR("vendor_ais_lock_net failed !! ret = %d\r\n", (int)ret);
		goto release;
	}

	ret = vendor_ai_op_open(proc_id) ; 
	if (ret != HD_OK) {
		DBG_ERR("vendor_ai_op_open fail=%d\n", (int)ret);
		goto unlock;
	}

	switch(op_cmd){
		case FC_LL_MODE:
        {
			VENDOR_AI_OP_CFG_WORKBUF wbuf = {0};
			wbuf.pa = buf->pa;
			wbuf.va = buf->va;
			wbuf.size = buf->size;
			wbuf.op = VENDOR_AI_OP_LIST;
			ret = vendor_ai_op_set(proc_id, VENDOR_AI_OP_PARAM_CFG_WORKBUF, &wbuf);
			if (ret != HD_OK) {
				DBG_ERR("vendor_ai_op_set fail=%d\n", (int)ret);
				goto exit;
			} 
	        break;
		}
		case PREPROC_YUV2RGB:
		case PREPROC_YUV2RGB_SCALE:
		case PREPROC_YUV2RGB_MEANSUB_PLANE:
		case PREPROC_YUV2RGB_MEANSUB_DC:
		case PREPROC_Y2Y_UV2UV:
		case PREPROC_RGB_SCALE:
		case PREPROC_Y8_SCALE:
		{
			VENDOR_AI_OP_CFG_WORKBUF wbuf = {0};
			wbuf.pa = buf->pa;
			wbuf.va = buf->va;
			wbuf.size = buf->size;
			wbuf.op = VENDOR_AI_OP_PREPROC;
			ret = vendor_ai_op_set(proc_id, VENDOR_AI_OP_PARAM_CFG_WORKBUF, &wbuf);
			if (ret != HD_OK) {
				DBG_ERR("vendor_ai_op_set fail=%d\n", (int)ret);
				goto exit;
			}
			break;
		}
		case TOPN_SORT:
		{
			VENDOR_AI_OP_CFG_WORKBUF wbuf = {0};
			wbuf.pa = buf->pa;
			wbuf.va = buf->va;
			wbuf.size = buf->size;
			wbuf.op = VENDOR_AI_OP_TOPN;
			ret = vendor_ai_op_set(proc_id, VENDOR_AI_OP_PARAM_CFG_WORKBUF, &wbuf);
			if (ret != HD_OK) {
				DBG_ERR("vendor_ai_op_set fail=%d\n", (int)ret);
				goto exit;
			}
			break;
		}
		default:
		{
			DBG_ERR("Unknown op_cmd\r\n");
			goto exit;
		}
	}

	ret = vendor_ais_unlock_net(proc_id);
	if (ret != HD_OK) {
		DBG_ERR("vendor_ais_unlock_net fail=%d\n", (int)ret);
		goto exit;
	}

	return ret;
	
exit:	
	ret = vendor_ai_op_close(proc_id);
	if (ret != HD_OK) {
		DBG_ERR("vendor_ai_op_close fail=%d\n", (int)ret);
	}
unlock:
	ret = vendor_ais_unlock_net(proc_id);
	if (ret != HD_OK) {
		DBG_ERR("vendor_ais_unlock_net fail=%d\n", (int)ret);
	}
release:
	ret = vendor_ai_release_id(proc_id);
	if (ret != HD_OK) {
		DBG_ERR("release proc_id(%d) failed, ret = %d\r\n", (int)proc_id, (int)ret);
	}
uninit:
	ret = vendor_ai_uninit();
	if (ret != HD_OK) {
		DBG_ERR("vendor_ai_uninit fail=%d\n", (int)ret);
	}

	return HD_ERR_INV;
}

HD_RESULT vendor_ai3_eop_start(UINT32 proc_id)
{
	HD_RESULT ret = HD_OK;

	ret = vendor_ais_lock_net(proc_id);
    if (ret != HD_OK) {
		DBG_ERR("vendor_ais_lock_net failed !! ret = %d\r\n", (int)ret);
		return HD_ERR_INV;
	}

	ret = vendor_ai_op_start(proc_id) ;
	if (ret != HD_OK) {
		DBG_ERR("vendor_ai_op_start fail=%d\n", (int)ret);
		goto exit;
	}

	ret = vendor_ais_unlock_net(proc_id);
	if (ret != HD_OK) {
		DBG_ERR("vendor_ais_unlock_net fail=%d\n", (int)ret);
		return HD_ERR_INV;
	}

	return ret;

exit:
	ret = vendor_ai_op_close(proc_id);
	if (ret != HD_OK) {
		DBG_ERR("vendor_ai_op_close fail=%d\n", (int)ret);
	}

	ret = vendor_ais_unlock_net(proc_id);
	if (ret != HD_OK) {
		DBG_ERR("vendor_ais_unlock_net fail=%d\n", (int)ret);
	}

	ret = vendor_ai_release_id(proc_id);
	if (ret != HD_OK) {
		DBG_ERR("release proc_id(%d) failed, ret = %d\r\n", (int)proc_id, (int)ret);
	}
	
	ret = vendor_ai_uninit();
	if (ret != HD_OK) {
		DBG_ERR("vendor_ai_uninit fail=%d\n", (int)ret);
	}

	return HD_ERR_INV;
}

HD_RESULT vendor_ai3_eop_proc(UINT32 proc_id, UINT32 op_cmd, VENDOR_AI3_BUF* src, VENDOR_AI3_BUF* dst, void* op_cfg, BOOL is_wait_done){
	HD_RESULT ret = HD_OK;

	ret = vendor_ais_lock_net(proc_id);
    if (ret != HD_OK) {
		DBG_ERR("vendor_ais_lock_net failed !! ret = %d\r\n", (int)ret);
		return HD_ERR_INV;
	}

	switch(op_cmd){
		case FC_LL_MODE:
        {
			ret = vendor_ai_op_proc(proc_id, VENDOR_AI_OP_LIST, NULL, 2, src, 2, dst);
			if (ret != HD_OK) {
				DBG_ERR("vendor_ai_op_proc fail=%d\n", (int)ret);
				goto exit;
			}
	        break;
		}

		case PREPROC_YUV2RGB:
		case PREPROC_YUV2RGB_SCALE:
		case PREPROC_YUV2RGB_MEANSUB_PLANE:
		case PREPROC_YUV2RGB_MEANSUB_DC:
		case PREPROC_Y2Y_UV2UV:
		case PREPROC_RGB_SCALE:
		case PREPROC_Y8_SCALE:
		{
			VENDOR_AI3_OP_PREPROC_PARAM* cfg = (VENDOR_AI3_OP_PREPROC_PARAM*) op_cfg;
			/*
			if (cfg->op_mem.buf.pa == 0 || cfg->op_mem.buf.va == 0 || cfg->op_mem.buf.size == 0) {
				DBG_ERR("check cfg->op_mem.buf fail !! cfg->op_mem.buf (pa/va/size) = (0x%016lx, 0x%016lx, %d)\r\n", cfg->op_mem.buf.pa, cfg->op_mem.buf.va, (int)cfg->op_mem.buf.size);
				return HD_ERR_INV;
			}
            */	
		    ret = vendor_ai_check_preproc_format(src, dst, cfg->scale_dim.w, cfg->scale_dim.h);
			if (ret != HD_OK) {
				DBG_ERR("vendor_ai_check_preproc_format fail=%d\n", (int)ret);
				goto exit;
			}

			{
				VENDOR_AI_OP_PREPROC_PARAM p_parm = {0};
				if (cfg->scale_dim.h && cfg->scale_dim.w) {
					p_parm.scale_dim.w = cfg->scale_dim.w;
					p_parm.scale_dim.h = cfg->scale_dim.h;
				}				

    		    // plane mode
    		    if (cfg->p_out_sub.pa) {
					p_parm.p_out_sub.pa = cfg->p_out_sub.pa;
					p_parm.p_out_sub.va = cfg->p_out_sub.va;                       
					p_parm.p_out_sub.width = cfg->p_out_sub.width;
					p_parm.p_out_sub.height = cfg->p_out_sub.height;
					p_parm.p_out_sub.line_ofs = cfg->p_out_sub.line_ofs;
#if defined(_BSP_NS02302_)
					if(g_real_chip_id == NN_CHIP_CNN30_C){
						DBG_ERR("539A doesn't support in plane mode !!\r\n");
						goto exit;

					}
#endif
    		    }

				// dc mode
				if (cfg->out_sub_color[0] || cfg->out_sub_color[1] || cfg->out_sub_color[2]) {                 
					p_parm.out_sub_color[0] = cfg->out_sub_color[0];
					p_parm.out_sub_color[1] = cfg->out_sub_color[1];
					p_parm.out_sub_color[2] = cfg->out_sub_color[2];
				}

				// AI_OP_PREPROC_UV2UV 
				if (dst[0].fmt == HD_VIDEO_PXLFMT_UV){
					ret = vendor_ai_op_proc(proc_id, VENDOR_AI_OP_PREPROC, &p_parm, 1, src, 1, dst);
					if (ret != HD_OK) {
						DBG_ERR("vendor_ai_op_proc fail=%d\n", (int)ret);
						goto exit;
					}
				// AI_OP_PREPROC_Y2Y
				}else if (dst[0].fmt == HD_VIDEO_PXLFMT_Y8){
					ret = vendor_ai_op_proc(proc_id, VENDOR_AI_OP_PREPROC, &p_parm, 1, src, 1, dst);
					if (ret != HD_OK) {
						DBG_ERR("vendor_ai_op_proc fail=%d\n", (int)ret);
						goto exit;
					}
					//AI_OP_PREPROC_UV2UV 
					if (dst[1].fmt == HD_VIDEO_PXLFMT_UV){
						p_parm.scale_dim.w = cfg->scale_dim.w / 2;
						p_parm.scale_dim.h = cfg->scale_dim.h / 2;

						ret = vendor_ai_op_proc(proc_id, VENDOR_AI_OP_PREPROC, &p_parm, 1, src+1, 1, dst+1);
						if (ret != HD_OK) {
							DBG_ERR("vendor_ai_op_proc fail=%d\n", (int)ret);
							goto exit;
						}
					}
				// AI_OP_PREPROC_RGB2RGB	
				}else if ((dst[0].fmt == HD_VIDEO_PXLFMT_R8) && (dst[1].fmt == HD_VIDEO_PXLFMT_G8) && (dst[2].fmt == HD_VIDEO_PXLFMT_B8) && (dst[0].fmt == src[0].fmt)){
					

					ret = vendor_ai_op_proc(proc_id, VENDOR_AI_OP_PREPROC, &p_parm, 3, src, 3, dst);
					if (ret != HD_OK) {
						DBG_ERR("vendor_ai_op_proc fail=%d\n", (int)ret);
						goto exit;
					}
#if defined(_BSP_NS02302_)
					if(g_real_chip_id == NN_CHIP_CNN30_C){
						VENDOR_AI3_BUF src2 = {0}, dst2 = {0} ;
						memcpy(&dst2, dst+2, sizeof(VENDOR_AI3_BUF));
						memcpy(&src2, src+2, sizeof(VENDOR_AI3_BUF));
						src2.fmt = HD_VIDEO_PXLFMT_Y8;
						dst2.fmt = HD_VIDEO_PXLFMT_Y8;
						p_parm.out_sub_color[0] = cfg->out_sub_color[2];
						ret = vendor_ai_op_proc(proc_id, VENDOR_AI_OP_PREPROC, &p_parm, 1, &src2, 1, &dst2);
						if (ret != HD_OK) {
							DBG_ERR("vendor_ai_op_proc fail=%d\n", (int)ret);
							goto exit;
						}
					}
#endif					
				// AI_OP_PREPROC_YUV2RGB
				}else {
					ret = vendor_ai_op_proc(proc_id, VENDOR_AI_OP_PREPROC, &p_parm, 2, src, 3, dst);
					if (ret != HD_OK) {
						DBG_ERR("vendor_ai_op_proc fail=%d\n", (int)ret);
						goto exit;
					}
				}
			}
			break;
		}

		case TOPN_SORT:
		{
			VENDOR_AI3_OP_TOPN_SORT_CFG* cfg = (VENDOR_AI3_OP_TOPN_SORT_CFG*) op_cfg;

			if (cfg->buf.pa == 0 || cfg->buf.va == 0 || cfg->buf.size == 0) {
				DBG_ERR("check cfg->buf fail !! cfg->buf (pa/va/size) = (0x%016lx, 0x%016lx, %d)\r\n", cfg->buf.pa, cfg->buf.va, (int)cfg->buf.size);
				goto exit;
			}

			{
				VENDOR_AI_OP_TOPNSORT_PARAM topn_param = {0};

				topn_param.sort_op = cfg->op;
				topn_param.sort_n  = cfg->n;

				ret = vendor_ai_op_proc(proc_id, VENDOR_AI_OP_TOPN, &topn_param, 1, src, 2, dst);
				if (ret != HD_OK) {
					DBG_ERR("vendor_ai_op_proc fail=%d\n", (int)ret);
					goto exit;
				}
			}
			break;
		}

		default:
		{
			DBG_ERR("Unknown op_cmd\r\n");
			goto exit;
		}
	}

	ret = vendor_ais_unlock_net(proc_id);
	if (ret != HD_OK) {
		DBG_ERR("vendor_ais_unlock_net fail=%d\n", (int)ret);
		return HD_ERR_INV;
	}

	return ret;

exit:
	ret = vendor_ai_op_stop(proc_id);
	if (ret != HD_OK) {
		DBG_ERR("vendor_ai_op_stop fail=%d\n", (int)ret);
	}

	ret = vendor_ai_op_close(proc_id);
	if (ret != HD_OK) {
		DBG_ERR("vendor_ai_op_close fail=%d\n", (int)ret);
	}

	ret = vendor_ais_unlock_net(proc_id);
	if (ret != HD_OK) {
		DBG_ERR("vendor_ais_unlock_net fail=%d\n", (int)ret);
	}

	ret = vendor_ai_release_id(proc_id);
	if (ret != HD_OK) {
		DBG_ERR("release proc_id(%d) failed, ret = %d\r\n", (int)proc_id, (int)ret);
	}
	
	ret = vendor_ai_uninit();
	if (ret != HD_OK) {
		DBG_ERR("vendor_ai_uninit fail=%d\n", (int)ret);
	}

	return HD_ERR_INV;
}

HD_RESULT vendor_ai3_eop_stop(UINT32 proc_id)
{
	HD_RESULT ret = HD_OK;

	ret = vendor_ais_lock_net(proc_id);
    if (ret != HD_OK) {
		DBG_ERR("vendor_ais_lock_net failed !! ret = %d\r\n", (int)ret);
		return HD_ERR_INV;
	}

	ret = vendor_ai_op_stop(proc_id);
	if (ret != HD_OK) {
		DBG_ERR("vendor_ai_op_stop fail=%d\n", (int)ret);
		goto exit;
	}

	ret = vendor_ais_unlock_net(proc_id);
	if (ret != HD_OK) {
		DBG_ERR("vendor_ais_unlock_net fail=%d\n", (int)ret);
		return HD_ERR_INV;
	}

	return ret;
exit:
	ret = vendor_ai_op_close(proc_id);
	if (ret != HD_OK) {
		DBG_ERR("vendor_ai_op_close fail=%d\n", (int)ret);
	}
	
	ret = vendor_ais_unlock_net(proc_id);
	if (ret != HD_OK) {
		DBG_ERR("vendor_ais_unlock_net fail=%d\n", (int)ret);
	}

	ret = vendor_ai_release_id(proc_id);
	if (ret != HD_OK) {
		DBG_ERR("release proc_id(%d) failed, ret = %d\r\n", (int)proc_id, (int)ret);
	}
	
	ret = vendor_ai_uninit();
	if (ret != HD_OK) {
		DBG_ERR("vendor_ai_uninit fail=%d\n", (int)ret);
	}

	return HD_ERR_INV;
}

HD_RESULT vendor_ai3_eop_close(UINT32 proc_id)
{
	HD_RESULT ret = HD_OK;

	ret = vendor_ais_lock_net(proc_id);
    if (ret != HD_OK) {
		DBG_ERR("vendor_ais_lock_net failed !! ret = %d\r\n", (int)ret);
	}
	
	ret = vendor_ai_op_close(proc_id);
	if (ret != HD_OK) {
		DBG_ERR("vendor_ai_op_close fail=%d\n", (int)ret);
	}

	ret = vendor_ais_unlock_net(proc_id);
	if (ret != HD_OK) {
		DBG_ERR("vendor_ais_unlock_net fail=%d\n", (int)ret);
	}

	ret = vendor_ai_release_id(proc_id);
 	if (ret != HD_OK) {
 		DBG_ERR("release proc_id(%d) failed, ret = %d\r\n", (int)proc_id, (int)ret);
	}

	ret = vendor_ai_uninit();
 	if (ret != HD_OK) {
		DBG_ERR("vendor_ai_uninit fail=%d\n", (int)ret);
 	}

	return ret;
}

extern HD_RESULT conv_fc_eval_work_buf(VENDOR_AI3_OP_FC_MEM* mem);
HD_RESULT vendor_ai3_op_eval_fullyconnect (VENDOR_AI3_OP_FC_MEM* mem)
{
	HD_RESULT ret = HD_OK;

	if (HD_VIDEO_PXLFMT_FMT(mem->out_fmt) == HD_VIDEO_PXLFMT_AI_SINT32){
		ret = conv_fc_eval_work_buf(mem);
		if (ret != HD_OK) {
			DBG_ERR("conv_fc_eval_work_buf fail=%d\n", (int)ret);
			return ret;
		}
	}
	else{
		UINT32 buf_size = 0;
		UINT32 slice_num = 0;
		UINT32 stripe_num = 0;

		ret = _vendor_ai_get_cal_max_buf_size(&buf_size);
		if (ret != HD_OK) {
			DBG_ERR("_vendor_ai_get_cal_max_buf_size fail=%d\n", (int)ret);
			return ret;
		}
		stripe_num = (((mem->input1.w +7) / 8)*32 * mem->input1.h + 65535)/65536;
		if (stripe_num == 1 ){
			mem->swap_out = 0; // No need to allocate temp buf 
		}else if(stripe_num % 2 == 1){
			mem->swap_out = 1; // Output to temp buf 
		}else{
			mem->swap_out = 2; // Output to out buf 
		}

		slice_num = (mem->input2.h / 131000U) + (((mem->input2.h % 131000U) > 0U)?1U:0U);
		mem->buf.size = ALIGN_CEIL_64(sizeof(VENDOR_AI_OP_WORK)) + ALIGN_CEIL_64(buf_size * slice_num + sizeof(JOB_LIST_TRIG_CMD) + sizeof(JOB_LIST_NULL_CMD)); 
		// DBG_ERR("buf_size =%u slice_num =%u mem->buf.size %u\n", buf_size, slice_num, mem->buf.size);
	}

	return HD_OK;
}

HD_RESULT vendor_ai3_op_init_fullyconnect (UINT32* p_op_id, VENDOR_AI3_OP_FC_CFG* cfg)
{	
	HD_RESULT ret = HD_OK;
	UINT32 proc_id = 0;

	if (cfg->op_mem.buf.pa == 0 || cfg->op_mem.buf.va == 0 || cfg->op_mem.buf.size == 0) {
		DBG_ERR("check buf fail !! buf (pa/va/size) = (0x%016lx, 0x%016lx, %d)\r\n", cfg->op_mem.buf.pa, cfg->op_mem.buf.va, (int)cfg->op_mem.buf.size);
		return HD_ERR_INV;
	}
	if (p_op_id  == NULL) {		DBG_ERR("check p_op_id == NULL !!\r\n");		return HD_ERR_INV;	}

	// config share model mode 2
	{
		UINT32 share_mode = 2;
		vendor_ai_cfg_set(VENDOR_AI_CFG_SHAREMODEL_MODE, &share_mode);
	}
	// config multi_thread 1
	{
		UINT32 multi_thread = 1;
		vendor_ai_cfg_set(VENDOR_AI_CFG_MULTI_THREAD, &multi_thread);
	}

	ret = vendor_ai_init();
	if (ret != HD_OK) {
		DBG_ERR("vendor_ai_init fail=%d\n", (int)ret);
		goto exit;
	}
	
	ret = vendor_ai_get_id(&proc_id);
	if (ret != HD_OK) {
		DBG_ERR("get proc_id failed !! ret = %d\r\n", (int)ret);
		goto uninit;
	}
	*p_op_id = proc_id;

	ret = vendor_ais_lock_net(proc_id);
	if (ret != HD_OK) {
		DBG_ERR("vendor_ais_lock_net failed !! ret = %d\r\n", (int)ret);
		goto release;
	}

	{
		ret = vendor_ai_op_open(proc_id) ; 
		if (ret != HD_OK) {
			DBG_ERR("vendor_ai_op_open fail=%d\n", (int)ret);
			goto unlock;
		}

		VENDOR_AI_OP_CFG_WORKBUF wbuf = {0};
		wbuf.pa = cfg->op_mem.buf.pa;
		wbuf.va = cfg->op_mem.buf.va;
		wbuf.size = cfg->op_mem.buf.size;
		wbuf.op = VENDOR_AI_OP_LIST;
		ret = vendor_ai_op_set(proc_id, VENDOR_AI_OP_PARAM_CFG_WORKBUF, &wbuf);
		if (ret != HD_OK) {
			DBG_ERR("vendor_ai_op_set fail=%d\n", (int)ret);
			goto close;
		} 

		ret = vendor_ai_op_start(proc_id) ;
		if (ret != HD_OK) {
			DBG_ERR("vendor_ai_op_start fail=%d\n", (int)ret);
			goto close;
		}
	}

	ret = vendor_ais_unlock_net(proc_id);
    if (ret != HD_OK) {
        DBG_ERR("vendor_ais_unlock_net fail=%d\n", (int)ret);
		goto stop;
    }

	return ret;
	
stop:
	ret = vendor_ai_op_stop(proc_id);
	if (ret != HD_OK) {
        DBG_ERR("vendor_ai_op_stop fail=%d\n", (int)ret);
    }
close:	
	ret = vendor_ai_op_close(proc_id);
	if (ret != HD_OK) {
        DBG_ERR("vendor_ai_op_close fail=%d\n", (int)ret);
    }
unlock:
	ret = vendor_ais_unlock_net(proc_id);
	if (ret != HD_OK) {
        DBG_ERR("vendor_ais_unlock_net fail=%d\n", (int)ret);
    }
release:
	ret = vendor_ai_release_id(proc_id);
	if (ret != HD_OK) {
		DBG_ERR("release proc_id(%d) failed, ret = %d\r\n", (int)proc_id, (int)ret);
	}
uninit:
	ret = vendor_ai_uninit();
	if (ret != HD_OK) {
		DBG_ERR("vendor_ai_uninit fail=%d\n", (int)ret);
	}
exit:
	return HD_ERR_INV;
}

extern HD_RESULT vendor_ai_fc_proc(UINT32 proc_id, VENDOR_AI_OP aop, void* p_param, UINT32 in_cnt, VENDOR_AI3_BUF* p_in_buf, UINT32 out_cnt, VENDOR_AI3_BUF* p_out_buf);
HD_RESULT vendor_ai3_op_do_fullyconnect (UINT32 op_id, VENDOR_AI3_BUF* src1, VENDOR_AI3_BUF* src2, VENDOR_AI3_BUF* dst1)
{
	HD_RESULT ret = HD_OK;
	VENDOR_AI3_BUF src[2] = {0};

	memcpy(src, src1, sizeof(VENDOR_AI3_BUF));
	memcpy(src + 1, src2, sizeof(VENDOR_AI3_BUF));

	ret = vendor_ais_lock_net(op_id);
	if (ret != HD_OK) {
		DBG_ERR("vendor_ais_lock_net failed !! ret = %d\r\n", (int)ret);
		goto exit;
	}

	if ((HD_VIDEO_PXLFMT_FMT(src1->fmt) == HD_VIDEO_PXLFMT_AI_SINT8) && (HD_VIDEO_PXLFMT_FMT(src2->fmt) == HD_VIDEO_PXLFMT_AI_SINT8) && (HD_VIDEO_PXLFMT_FMT(dst1->fmt) == HD_VIDEO_PXLFMT_AI_SINT32))
	{
#if (defined(_BSP_NS02302_) || defined(_BSP_NS02201_) || defined(_BSP_NS02401_))
		// DBG_DUMP("run CONV\r\n");
		ret = vendor_ai_fc_proc(op_id, VENDOR_AI_OP_LIST, NULL, 2, src, 2, dst1);
		if (ret != HD_OK) {
			DBG_ERR("vendor_ai_op_proc fail=%d\n", (int)ret);
			goto exit;
		}
#else
		DBG_ERR("FC does not support input in S8 S8 format and output in S32 format\r\n");
		goto exit;
#endif
	}
	
	else{
		// DBG_DUMP("run CAL\r\n");
		ret = vendor_ai_op_proc(op_id, VENDOR_AI_OP_LIST, NULL, 2, src, 2, dst1);
		if (ret != HD_OK) {
			DBG_ERR("vendor_ai_op_proc fail=%d\n", (int)ret);
			goto exit;
		}
	}

	ret = vendor_ais_unlock_net(op_id);
	if (ret != HD_OK) {
        DBG_ERR("vendor_ais_unlock_net fail=%d\n", (int)ret);
		return HD_ERR_INV;
    }

	return ret;	

exit:
	ret = vendor_ai_op_stop(op_id);
	if (ret != HD_OK) {
        DBG_ERR("vendor_ai_op_stop fail=%d\n", (int)ret);
    }

	ret = vendor_ai_op_close(op_id);
	if (ret != HD_OK) {
        DBG_ERR("vendor_ai_op_close fail=%d\n", (int)ret);
    }

	ret = vendor_ais_unlock_net(op_id);
	if (ret != HD_OK) {
        DBG_ERR("vendor_ais_unlock_net fail=%d\n", (int)ret);
    }

	ret = vendor_ai_release_id(op_id);
	if (ret != HD_OK) {
		DBG_ERR("release proc_id(%d) failed, ret = %d\r\n", (int)op_id, (int)ret);
	}

	ret = vendor_ai_uninit();
	if (ret != HD_OK) {
		DBG_ERR("vendor_ai_uninit fail=%d\n", (int)ret);
	}

	return ret;
}

HD_RESULT vendor_ai3_op_uninit_fullyconnect (UINT32 op_id)
{
	HD_RESULT ret = HD_OK;

	ret = vendor_ais_lock_net(op_id);
    if (ret != HD_OK) {
		DBG_ERR("vendor_ais_lock_net failed !! ret = %d\r\n", (int)ret);
	}

	ret = vendor_ai_op_stop(op_id);
    if (ret != HD_OK) {
        DBG_ERR("vendor_ai_op_stop fail=%d\n", (int)ret);
    }

	ret = vendor_ai_op_close(op_id);
    if (ret != HD_OK) {
        DBG_ERR("vendor_ai_op_close fail=%d\n", (int)ret);
    }
	
	ret = vendor_ais_unlock_net(op_id);
    if (ret != HD_OK) {
        DBG_ERR("vendor_ais_unlock_net fail=%d\n", (int)ret);
    }

	ret = vendor_ai_release_id(op_id);
	if (ret != HD_OK) {
		DBG_ERR("release proc_id(%d) failed, ret = %d\r\n", (int)op_id, (int)ret);
	}

	ret = vendor_ai_uninit();
	if (ret != HD_OK) {
		DBG_ERR("vendor_ai_uninit fail=%d\n", (int)ret);
	}

	return ret;
}

HD_RESULT vendor_ai3_op_eval_topnsort(VENDOR_AI3_OP_TOPNSORT_MEM* mem)
{
	HD_RESULT ret = HD_OK;
	UINT32 buf_size = 0;
	UINT32 slice_num = 0;  

	ret = _vendor_ai_get_cal_max_buf_size(&buf_size) ;
	if (ret != HD_OK) {
		DBG_ERR("_vendor_ai_get_cal_max_buf_size fail=%d\n", (int)ret);
		return ret;
	}
		
	slice_num = (mem->input1.h / 131000U) + (((mem->input1.h % 131000U) > 0U)?1U:0U);
	mem->buf.size = ALIGN_CEIL_64(sizeof(VENDOR_AI_OP_WORK)) + ALIGN_CEIL_64(buf_size * slice_num + sizeof(JOB_LIST_TRIG_CMD) + sizeof(JOB_LIST_NULL_CMD)) ; 
	// DBG_ERR("buf_size =%u slice_num =%u mem->buf.size %u\n", buf_size, slice_num, mem->buf.size);
	return HD_OK;
}

HD_RESULT vendor_ai3_op_init_topnsort (UINT32* p_op_id, VENDOR_AI3_OP_TOPNSORT_CFG* cfg){
	HD_RESULT ret = HD_OK;
	UINT32 proc_id = 0;

	if (cfg->buf.pa == 0 || cfg->buf.va == 0 || cfg->buf.size == 0) {
		DBG_ERR("check cfg->buf fail !! cfg->buf (pa/va/size) = (0x%016lx, 0x%016lx, %d)\r\n", cfg->buf.pa, cfg->buf.va, (int)cfg->buf.size);
		return HD_ERR_INV;
	}
	// config share model mode 2
	{
		UINT32 share_mode = 2;
		vendor_ai_cfg_set(VENDOR_AI_CFG_SHAREMODEL_MODE, &share_mode);
	}
	// config multi_thread 1
	{
		UINT32 multi_thread = 1;
		vendor_ai_cfg_set(VENDOR_AI_CFG_MULTI_THREAD, &multi_thread);
	}

	ret = vendor_ai_init();
	if (ret != HD_OK) {
		DBG_ERR("vendor_ai_init fail=%d\n", (int)ret);
		goto exit;
	}

	ret = vendor_ai_get_id(&proc_id);
	if (ret != HD_OK) {
		DBG_ERR("get proc_id failed !! ret = %d\r\n", (int)ret);
		goto uninit;
	}
	*p_op_id = proc_id;

	ret = vendor_ais_lock_net(proc_id);
	if (ret != HD_OK) {
		DBG_ERR("vendor_ais_lock_net fail=%d\n", (int)ret);
		goto release;
	}

	{
		ret = vendor_ai_op_open(proc_id) ; 
		if (ret != HD_OK) {
			DBG_ERR("vendor_ai_op_open fail=%d\n", (int)ret);
			goto unlock;
		}

		VENDOR_AI_OP_CFG_WORKBUF wbuf = {0};
		wbuf.pa = cfg->buf.pa;
		wbuf.va = cfg->buf.va;
		wbuf.size = cfg->buf.size;
		wbuf.op = VENDOR_AI_OP_TOPN;
		ret = vendor_ai_op_set(proc_id, VENDOR_AI_OP_PARAM_CFG_WORKBUF, &wbuf);
		if (ret != HD_OK) {
			DBG_ERR("vendor_ai_op_set fail=%d\n", (int)ret);
			goto close;
		}

		ret = vendor_ai_op_start(proc_id) ;
		if (ret != HD_OK) {
			DBG_ERR("vendor_ai_op_start fail=%d\n", (int)ret);
			goto close;
		}
	}

	ret = vendor_ais_unlock_net(proc_id);
	if (ret != HD_OK) {
   		DBG_ERR("vendor_ais_unlock_net fail=%d\n", (int)ret);
		goto stop;
	}

	return ret;
	
stop:
	ret = vendor_ai_op_stop(proc_id);
	if (ret != HD_OK) {
        DBG_ERR("vendor_ai_op_stop fail=%d\n", (int)ret);
    }
close:	
	ret = vendor_ai_op_close(proc_id);
	if (ret != HD_OK) {
        DBG_ERR("vendor_ai_op_close fail=%d\n", (int)ret);
    }
unlock:
	ret = vendor_ais_unlock_net(proc_id);
	if (ret != HD_OK) {
        DBG_ERR("vendor_ais_unlock_net fail=%d\n", (int)ret);
    }
release:
	ret = vendor_ai_release_id(proc_id);
	if (ret != HD_OK) {
		DBG_ERR("release proc_id(%d) failed, ret = %d\r\n", (int)proc_id, (int)ret);
	}
uninit:
	ret = vendor_ai_uninit();
	if (ret != HD_OK) {
		DBG_ERR("vendor_ai_uninit fail=%d\n", (int)ret);
	}
exit:
	return HD_ERR_INV;
}

HD_RESULT vendor_ai3_op_do_topnsort (UINT32 op_id, VENDOR_AI3_BUF* src, VENDOR_AI3_BUF* dst_value, VENDOR_AI3_BUF* dst_idx, VENDOR_AI3_TOPNSORT_OP op, UINT32 n)
{
	HD_RESULT ret = HD_OK;
	
	ret = vendor_ais_lock_net(op_id);
	if (ret != HD_OK) {
		DBG_ERR("vendor_ais_lock_net fail=%d\n", (int)ret);
		goto exit;
	}
	
	{
		VENDOR_AI_OP_TOPNSORT_PARAM topnsort_param = {0};
		VENDOR_AI3_BUF dst[2] = {0};

		topnsort_param.sort_op = op;
		topnsort_param.sort_n  = n;
		memcpy(dst, dst_value, sizeof(VENDOR_AI3_BUF));
		memcpy(dst + 1, dst_idx, sizeof(VENDOR_AI3_BUF));

		ret = vendor_ai_op_proc(op_id, VENDOR_AI_OP_TOPN, &topnsort_param, 1, src, 2, dst);
		if (ret != HD_OK) {
			DBG_ERR("vendor_ai_op_proc fail=%d\n", (int)ret);
			goto exit;
		}
	}

	ret = vendor_ais_unlock_net(op_id);
	if (ret != HD_OK) {
		DBG_ERR("vendor_ais_unlock_net fail=%d\n", (int)ret);
		return HD_ERR_INV;
	}

	return ret;	

exit:
	ret = vendor_ai_op_stop(op_id);
	if (ret != HD_OK) {
        DBG_ERR("vendor_ai_op_stop fail=%d\n", (int)ret);
    }

	ret = vendor_ai_op_close(op_id);
	if (ret != HD_OK) {
        DBG_ERR("vendor_ai_op_close fail=%d\n", (int)ret);
    }

	ret = vendor_ais_unlock_net(op_id);
	if (ret != HD_OK) {
        DBG_ERR("vendor_ais_unlock_net fail=%d\n", (int)ret);
    }

	ret = vendor_ai_release_id(op_id);
	if (ret != HD_OK) {
		DBG_ERR("release proc_id(%d) failed, ret = %d\r\n", (int)op_id, (int)ret);
	}

	ret = vendor_ai_uninit();
	if (ret != HD_OK) {
		DBG_ERR("vendor_ai_uninit fail=%d\n", (int)ret);
	}

	return ret;
}

extern HD_RESULT vendor_ai3_op_uninit_topnsort (UINT32 op_id){
	HD_RESULT ret = HD_OK;

	ret = vendor_ais_lock_net(op_id);
    if (ret != HD_OK) {
		DBG_ERR("vendor_ais_lock_net failed !! ret = %d\r\n", (int)ret);
	}

	ret = vendor_ai_op_stop(op_id);
    if (ret != HD_OK) {
        DBG_ERR("vendor_ai_op_stop fail=%d\n", (int)ret);
    }

	ret = vendor_ai_op_close(op_id);
    if (ret != HD_OK) {
        DBG_ERR("vendor_ai_op_close fail=%d\n", (int)ret);
    }
	
	ret = vendor_ais_unlock_net(op_id);
    if (ret != HD_OK) {
        DBG_ERR("vendor_ais_unlock_net fail=%d\n", (int)ret);
    }

	ret = vendor_ai_release_id(op_id);
	if (ret != HD_OK) {
		DBG_ERR("release proc_id(%d) failed, ret = %d\r\n", (int)op_id, (int)ret);
	}

	ret = vendor_ai_uninit();
	if (ret != HD_OK) {
		DBG_ERR("vendor_ai_uninit fail=%d\n", (int)ret);
	}

	return ret;
}

HD_RESULT vendor_ai3_op_eval_matrix (VENDOR_AI3_OP_MATRIX_MEM* mem)
{
	HD_RESULT ret = HD_OK;
	UINT32 buf_size = 0;
	UINT32 slice_num = 0;

	ret = _vendor_ai_get_cal_max_buf_size(&buf_size);
	if (ret != HD_OK) {
		DBG_ERR("_vendor_ai_get_cal_max_buf_size fail=%d\n", (int)ret);
		return ret;
	}

	slice_num = (mem->input2.h / 131000U) + (((mem->input2.h % 131000U) > 0U)?1U:0U);
	mem->buf.size = ALIGN_CEIL_64(sizeof(VENDOR_AI_OP_WORK)) + ALIGN_CEIL_64(buf_size * slice_num + sizeof(JOB_LIST_TRIG_CMD) + sizeof(JOB_LIST_NULL_CMD)); 
	// DBG_ERR("buf_size =%u slice_num =%u mem->buf.size %u\n", buf_size, slice_num, mem->buf.size);

	return HD_OK;
}

HD_RESULT vendor_ai3_op_init_matrix (UINT32* p_op_id, VENDOR_AI3_OP_MATRIX_CFG* cfg)
{	
	HD_RESULT ret = HD_OK;
	UINT32 proc_id = 0;

	if (cfg->op_mem.buf.pa == 0 || cfg->op_mem.buf.va == 0 || cfg->op_mem.buf.size == 0) {
		DBG_ERR("check buf fail !! buf (pa/va/size) = (0x%016lx, 0x%016lx, %d)\r\n", cfg->op_mem.buf.pa, cfg->op_mem.buf.va, (int)cfg->op_mem.buf.size);
		return HD_ERR_INV;
	}
	if (p_op_id  == NULL) {		DBG_ERR("check p_op_id == NULL !!\r\n");		return HD_ERR_INV;	}

	// config share model mode 2
	{
		UINT32 share_mode = 2;
		vendor_ai_cfg_set(VENDOR_AI_CFG_SHAREMODEL_MODE, &share_mode);
	}
	// config multi_thread 1
	{
		UINT32 multi_thread = 1;
		vendor_ai_cfg_set(VENDOR_AI_CFG_MULTI_THREAD, &multi_thread);
	}

	ret = vendor_ai_init();
	if (ret != HD_OK) {
		DBG_ERR("vendor_ai_init fail=%d\n", (int)ret);
		goto exit;
	}
	
	ret = vendor_ai_get_id(&proc_id);
	if (ret != HD_OK) {
		DBG_ERR("get proc_id failed !! ret = %d\r\n", (int)ret);
		goto uninit;
	}
	*p_op_id = proc_id;

	ret = vendor_ais_lock_net(proc_id);
	if (ret != HD_OK) {
		DBG_ERR("vendor_ais_lock_net failed !! ret = %d\r\n", (int)ret);
		goto release;
	}

	{
		ret = vendor_ai_op_open(proc_id) ; 
		if (ret != HD_OK) {
			DBG_ERR("vendor_ai_op_open fail=%d\n", (int)ret);
			goto unlock;
		}

		VENDOR_AI_OP_CFG_WORKBUF wbuf = {0};
		wbuf.pa = cfg->op_mem.buf.pa;
		wbuf.va = cfg->op_mem.buf.va;
		wbuf.size = cfg->op_mem.buf.size;
		wbuf.op = VENDOR_AI_OP_MATRIX;
		ret = vendor_ai_op_set(proc_id, VENDOR_AI_OP_PARAM_CFG_WORKBUF, &wbuf);
		if (ret != HD_OK) {
			DBG_ERR("vendor_ai_op_set fail=%d\n", (int)ret);
			goto close;
		} 

		ret = vendor_ai_op_start(proc_id) ;
		if (ret != HD_OK) {
			DBG_ERR("vendor_ai_op_start fail=%d\n", (int)ret);
			goto close;
		}
	}

	ret = vendor_ais_unlock_net(proc_id);
    if (ret != HD_OK) {
        DBG_ERR("vendor_ais_unlock_net fail=%d\n", (int)ret);
		goto stop;
    }

	return ret;
	
stop:
	ret = vendor_ai_op_stop(proc_id);
	if (ret != HD_OK) {
        DBG_ERR("vendor_ai_op_stop fail=%d\n", (int)ret);
    }
close:	
	ret = vendor_ai_op_close(proc_id);
	if (ret != HD_OK) {
        DBG_ERR("vendor_ai_op_close fail=%d\n", (int)ret);
    }
unlock:
	ret = vendor_ais_unlock_net(proc_id);
	if (ret != HD_OK) {
        DBG_ERR("vendor_ais_unlock_net fail=%d\n", (int)ret);
    }
release:
	ret = vendor_ai_release_id(proc_id);
	if (ret != HD_OK) {
		DBG_ERR("release proc_id(%d) failed, ret = %d\r\n", (int)proc_id, (int)ret);
	}
uninit:
	ret = vendor_ai_uninit();
	if (ret != HD_OK) {
		DBG_ERR("vendor_ai_uninit fail=%d\n", (int)ret);
	}
exit:
	return HD_ERR_INV;
}

HD_RESULT vendor_ai3_op_do_matrix (UINT32 op_id, VENDOR_AI3_BUF* src1, VENDOR_AI3_BUF* src2, VENDOR_AI3_BUF* dst1, VENDOR_AI3_MATRIX_OP mtx_op)
{
	HD_RESULT ret = HD_OK;
	VENDOR_AI3_BUF src[2] = {0};

	memcpy(src, src1, sizeof(VENDOR_AI3_BUF));
	memcpy(src + 1, src2, sizeof(VENDOR_AI3_BUF));

	ret = vendor_ais_lock_net(op_id);
	if (ret != HD_OK) {
		DBG_ERR("vendor_ais_lock_net failed !! ret = %d\r\n", (int)ret);
		goto exit;
	}
	
	if (!((mtx_op == VENDOR_AI3_MATRIX_MUL)||(mtx_op == VENDOR_AI3_MATRIX_ADD)||(mtx_op == VENDOR_AI3_MATRIX_SUB))){
		DBG_ERR("mtx_op selection failed. mtx_op only supports 0: MUL, 1: ADD, 2: SUB. Current mtx_op is: %u\n", (int)mtx_op);
		goto exit;
	}

	ret = vendor_ai_op_proc(op_id, VENDOR_AI_OP_MATRIX, &mtx_op, 2, src, 1, dst1);
	if (ret != HD_OK) {
		DBG_ERR("vendor_ai_op_proc fail=%d\n", (int)ret);
		goto exit;
	}

	ret = vendor_ais_unlock_net(op_id);
	if (ret != HD_OK) {
        DBG_ERR("vendor_ais_unlock_net fail=%d\n", (int)ret);
		return HD_ERR_INV;
    }

	return ret;	

exit:
	ret = vendor_ai_op_stop(op_id);
	if (ret != HD_OK) {
        DBG_ERR("vendor_ai_op_stop fail=%d\n", (int)ret);
    }

	ret = vendor_ai_op_close(op_id);
	if (ret != HD_OK) {
        DBG_ERR("vendor_ai_op_close fail=%d\n", (int)ret);
    }

	ret = vendor_ais_unlock_net(op_id);
	if (ret != HD_OK) {
        DBG_ERR("vendor_ais_unlock_net fail=%d\n", (int)ret);
    }

	ret = vendor_ai_release_id(op_id);
	if (ret != HD_OK) {
		DBG_ERR("release proc_id(%d) failed, ret = %d\r\n", (int)op_id, (int)ret);
	}

	ret = vendor_ai_uninit();
	if (ret != HD_OK) {
		DBG_ERR("vendor_ai_uninit fail=%d\n", (int)ret);
	}

	return ret;
}

HD_RESULT vendor_ai3_op_uninit_matrix (UINT32 op_id)
{
	HD_RESULT ret = HD_OK;

	ret = vendor_ais_lock_net(op_id);
    if (ret != HD_OK) {
		DBG_ERR("vendor_ais_lock_net failed !! ret = %d\r\n", (int)ret);
	}

	ret = vendor_ai_op_stop(op_id);
    if (ret != HD_OK) {
        DBG_ERR("vendor_ai_op_stop fail=%d\n", (int)ret);
    }

	ret = vendor_ai_op_close(op_id);
    if (ret != HD_OK) {
        DBG_ERR("vendor_ai_op_close fail=%d\n", (int)ret);
    }
	
	ret = vendor_ais_unlock_net(op_id);
    if (ret != HD_OK) {
        DBG_ERR("vendor_ais_unlock_net fail=%d\n", (int)ret);
    }

	ret = vendor_ai_release_id(op_id);
	if (ret != HD_OK) {
		DBG_ERR("release proc_id(%d) failed, ret = %d\r\n", (int)op_id, (int)ret);
	}

	ret = vendor_ai_uninit();
	if (ret != HD_OK) {
		DBG_ERR("vendor_ai_uninit fail=%d\n", (int)ret);
	}

	return ret;
}

HD_RESULT vendor_ai3_op_eval_postprocess (VENDOR_AI3_OP_POSTPROC_MEM* mem)
{
	HD_RESULT ret = HD_OK;
	UINT32 buf_size = 0;
	UINT32 slice_num = 0;  

	ret = _vendor_ai_get_ppu_max_buf_size(&buf_size) ;
	if (ret != HD_OK) {
		printf("_vendor_ai_get_ppu_max_buf_size fail=%d\n", ret);
		return ret;
	}
		
	slice_num = 16;
	
	mem->buf.size = ALIGN_CEIL_64(sizeof(VENDOR_AI_OP_WORK)) + ALIGN_CEIL_64(buf_size * slice_num + sizeof(PATH_LIST_TRIG_CMD) + sizeof(PATH_LIST_NULL_CMD)) ; 
	return HD_OK;

}
HD_RESULT vendor_ai_check_postproc_format(VENDOR_AI3_BUF* src, VENDOR_AI3_BUF* dest){
	if (src->fmt == HD_VIDEO_PXLFMT_RAW12){
		if (dest->fmt != HD_VIDEO_PXLFMT_RAW16){
			DBG_ERR("Error! The input format HD_VIDEO_PXLFMT_RAW12 needs to be matched with the output format HD_VIDEO_PXLFMT_RAW16\r\n");
			return HD_ERR_LIMIT;
		}
		if ((src->width != dest->width) || (src->height != dest->height)){
			DBG_ERR("Error! postproc (in_width(%d), in_height(%d)) (out_width(%d), out_height(%d)), The input and output width and height must be equal\r\n", src->width, src->height, dest->width, dest->height);
			return HD_ERR_LIMIT;
		}
		if (((src->width * 3 / 2) > src->line_ofs) || ((dest->width * 2) > dest->line_ofs)){
			DBG_ERR("Error! postproc (in_width(%d), in_line_ofs(%llu)) (out_width(%d), out_line_ofs(%llu)), The line offset must be set in byte units\r\n", src->width, src->line_ofs, dest->width, dest->line_ofs);
			return HD_ERR_LIMIT;
		}
		
		if (((src->width % 2) != 0) || ((src->height % 2) != 0)){
			DBG_ERR("Error! postproc (in_width(%d), in_height(%d)), must be divisible by 2 if PPU_IN_FMT == Bayer12\r\n", src->width,  src->height);
			return HD_ERR_LIMIT;
		}
		if ((src->width > 4096) || (src->width < 2)){
			DBG_ERR("Error! postproc (in_width(%d)), the in_width must be <= (%d) and >= (%d)\r\n", src->width, 4096, 2);
			return HD_ERR_LIMIT;
		}
		if ((src->height > 4096) || (src->height < 2)){
			DBG_ERR("Error! postproc (in_height(%d)), the in_height must be <= (%d) and >= (%d)\r\n", src->height, 4096, 2);
			return HD_ERR_LIMIT;
		}
		if ((dest->width > 4096) || (dest->width < 2)){
			DBG_ERR("Error! postproc (out_width(%d)), the out_width must be <= (%d) and >= (%d)\r\n", dest->width, 4096, 2);
			return HD_ERR_LIMIT;
		}
		if ((dest->height > 4096) || (dest->height < 2)){
			DBG_ERR("Error! postproc (out_height(%d)), the out_height must be <= (%d) and >= (%d)\r\n", dest->height, 4096, 2);
			return HD_ERR_LIMIT;
		}
	}else if (src->fmt == HD_VIDEO_PXLFMT_RAW16){
		if (dest->fmt != HD_VIDEO_PXLFMT_RAW12){
			DBG_ERR("Error! The input format HD_VIDEO_PXLFMT_RAW16 needs to be matched with the output format HD_VIDEO_PXLFMT_RAW12\r\n");
			return HD_ERR_LIMIT;
		}
		if ((src->width != dest->width) || (src->height != dest->height)){
			DBG_ERR("Error! postproc (in_width(%d), in_height(%d)) (out_width(%d), out_height(%d)), The input and output width and height must be equal\r\n", src->width, src->height, dest->width, dest->height);
			return HD_ERR_LIMIT;
		}
		if (((src->width * 2) > src->line_ofs) || ((dest->width * 3 / 2) > dest->line_ofs)){
			DBG_ERR("Error! postproc (in_width(%d), in_line_ofs(%llu)) (out_width(%d), out_line_ofs(%llu)), The line offset must be set in byte units\r\n", src->width, src->line_ofs, dest->width, dest->line_ofs);
			return HD_ERR_LIMIT;
		}

		if (((src->width % 2) != 0) || ((src->height % 2) != 0)){
			DBG_ERR("Error! postproc (in_width(%d), in_height(%d)), must be divisible by 2 if PPU_IN_FMT == Bayer16\r\n", src->width,  src->height);
			return HD_ERR_LIMIT;
		}
		if ((src->width > 4096) || (src->width < 2)){
			DBG_ERR("Error! postproc (in_width(%d)), the in_width must be <= (%d) and >= (%d)\r\n", src->width, 4096, 2);
			return HD_ERR_LIMIT;
		}
		if ((src->height > 4096) || (src->height < 2)){
			DBG_ERR("Error! postproc (in_height(%d)), the in_height must be <= (%d) and >= (%d)\r\n", src->height, 4096, 2);
			return HD_ERR_LIMIT;
		}
		if ((dest->width > 4096) || (dest->width < 2)){
			DBG_ERR("Error! postproc (out_width(%d)), the out_width must be <= (%d) and >= (%d)\r\n", dest->width, 4096, 2);
			return HD_ERR_LIMIT;
		}
		if ((dest->height > 4096) || (dest->height < 2)){
			DBG_ERR("Error! postproc (out_height(%d)), the out_height must be <= (%d) and >= (%d)\r\n", dest->height, 4096, 2);
			return HD_ERR_LIMIT;
		}
	}else{
		if (!((src->fmt == HD_VIDEO_PXLFMT_R8) && ((src+1)->fmt == HD_VIDEO_PXLFMT_G8) && ((src+2)->fmt == HD_VIDEO_PXLFMT_B8))){
			if (!((src->fmt == HD_VIDEO_PXLFMT_B8) && ((src+1)->fmt == HD_VIDEO_PXLFMT_G8) && ((src+2)->fmt == HD_VIDEO_PXLFMT_R8))){
				DBG_ERR("Error! The input format only support HD_VIDEO_PXLFMT_RAW16, HD_VIDEO_PXLFMT_RAW12 and HD_VIDEO_PXLFMT_R8/G8/B8\r\n");
				return HD_ERR_LIMIT;
			}
		}
		if (!((dest->fmt == HD_VIDEO_PXLFMT_Y8) && ((dest+1)->fmt == HD_VIDEO_PXLFMT_UV))){
			DBG_ERR("Error! The output format only support HD_VIDEO_PXLFMT_RAW16, HD_VIDEO_PXLFMT_RAW12 and HD_VIDEO_PXLFMT_Y8/UV\r\n");
			return HD_ERR_LIMIT;
		}

		if ((src->width != dest->width) || (src->height != dest->height)){
			DBG_ERR("Error! postproc (in1_width(%d), in1_height(%d)) (out1_width(%d), out1_height(%d)), The input and output width and height must be equal\r\n", src->width, src->height, dest->width, dest->height);
			return HD_ERR_LIMIT;
		}
		if (((src+1)->width != (dest+1)->width) || ((src+1)->height != (dest+1)->height)){
			DBG_ERR("Error! postproc (in2_width(%d), in2_height(%d)) (out2_width(%d), out2_height(%d)), The input and output width and height must be equal\r\n", (src+1)->width, (src+1)->height, (dest+1)->width, (dest+1)->height);
			return HD_ERR_LIMIT;
		}
		if (((src+2)->width != (dest+1)->width) || ((src+2)->height != (dest+1)->height)){
			DBG_ERR("Error! postproc (in3_width(%d), in3_height(%d)) (out2_width(%d), out2_height(%d)), The input and output width and height must be equal\r\n", (src+2)->width, (src+2)->height, (dest+1)->width, (dest+1)->height);
			return HD_ERR_LIMIT;
		}

		// src check
		if ((src->width > 4096) || ((src+1)->width > 4096) || ((src+2)->width > 4096)){
			DBG_ERR("Error! postproc (in_width1(%d), in_width2(%d), in_width3(%d)), the in_width must be <= (%d)\r\n", src->width, (src+1)->width, (src+2)->width, 4096);
			return HD_ERR_LIMIT;
		}
		if ((src->width < 2) || ((src+1)->width < 2) || ((src+2)->width < 2)){
			DBG_ERR("Error! postproc (in_width1(%d), in_width2(%d), in_width3(%d)), the in_width must be >= (%d)\r\n", src->width, (src+1)->width, (src+2)->width, 2);
			return HD_ERR_LIMIT;
		}
		if ((src->height > 4096) || ((src+1)->height > 4096) || ((src+2)->height > 4096)){
			DBG_ERR("Error! postproc (in_height1(%d), in_height2(%d), in_height3(%d)), the in_height must be <= (%d)\r\n", src->height, (src+1)->height, (src+2)->height, 4096);
			return HD_ERR_LIMIT;
		}
		if ((src->height < 2) || ((src+1)->height < 2) || ((src+2)->height < 2)){
			DBG_ERR("Error! postproc (in_height1(%d), in_height2(%d), in_height3(%d)), the in_height must be <= (%d)\r\n", src->height, (src+1)->height, (src+2)->height, 2);
			return HD_ERR_LIMIT;
		}

		// dset check
		if ((dest->width > 4096) || ((dest+1)->width > 4096)){
			DBG_ERR("Error! postproc (out_width1(%d), out_width2(%d)), the out_width must be <= (%d)\r\n", dest->width, (dest+1)->width, 4096);
			return HD_ERR_LIMIT;
		}
		if ((dest->width < 2) || ((dest+1)->width < 2)){
			DBG_ERR("Error! postproc (out_width1(%d), out_width2(%d)), the out_width must be >= (%d)\r\n", dest->width, (dest+1)->width, 2);
			return HD_ERR_LIMIT;
		}
		if ((dest->height > 4096) || ((dest+1)->height > 4096)){
			DBG_ERR("Error! postproc (out_height1(%d), out_height2(%d)), the out_height must be <= (%d)\r\n", dest->height, (dest+1)->height, 4096);
			return HD_ERR_LIMIT;
		}
		if ((dest->height < 2) || ((dest+1)->height < 2)){
			DBG_ERR("Error! postproc (out_height1(%d), out_height2(%d)), the out_height must be <= (%d)\r\n", dest->height, (dest+1)->height, 2);
			return HD_ERR_LIMIT;
		}
	}

	return HD_OK;
}

HD_RESULT vendor_ai3_op_init_postprocess (UINT32* p_op_id, VENDOR_AI3_OP_POSTPROC_CFG* cfg)
{
    HD_RESULT ret = HD_OK;
	UINT32 proc_id = 0;

	if (cfg->op_mem.buf.pa == 0 || cfg->op_mem.buf.va == 0 || cfg->op_mem.buf.size == 0) {
		DBG_ERR("check cfg->op_mem.buf fail !! cfg->op_mem.buf (pa/va/size) = (0x%016lx, 0x%016lx, %d)\r\n", cfg->op_mem.buf.pa, cfg->op_mem.buf.va, (int)cfg->op_mem.buf.size);
        goto exit;
	}
	if (p_op_id  == NULL) {		DBG_ERR("check p_op_id == NULL !!\r\n");		return HD_ERR_INV;	}

	// config share model mode 2
	{
		UINT32 share_mode = 2;
		vendor_ai_cfg_set(VENDOR_AI_CFG_SHAREMODEL_MODE, &share_mode);
	}
	// config multi_thread 1
	{
		UINT32 multi_thread = 1;
		vendor_ai_cfg_set(VENDOR_AI_CFG_MULTI_THREAD, &multi_thread);
	}

	ret = vendor_ai_init();
	if (ret != HD_OK) {
		DBG_ERR("vendor_ai_init fail=%d\n", (int)ret);
		goto exit;
	}
	
	ret = vendor_ai_get_id(&proc_id);
	if (ret != HD_OK) {
		DBG_ERR("get proc_id failed !! ret = %d\r\n", (int)ret);
		goto uninit;
	}		

	ret = vendor_ais_lock_net(proc_id);
    if (ret != HD_OK) {
		DBG_ERR("vendor_ais_lock_net failed !! ret = %d\r\n", (int)ret);
		goto release;
	}		

	{
		ret = vendor_ai_op_open(proc_id) ; 
		if (ret != HD_OK) {
			DBG_ERR("vendor_ai_op_open fail=%d\n", (int)ret);
			goto unlock;
		}

		VENDOR_AI_OP_CFG_WORKBUF wbuf = {0};
		wbuf.pa = cfg->op_mem.buf.pa;
		wbuf.va = cfg->op_mem.buf.va;
		wbuf.size = cfg->op_mem.buf.size;
		wbuf.op = VENDOR_AI_OP_POSTPROC;
		ret = vendor_ai_op_set(proc_id, VENDOR_AI_OP_PARAM_CFG_WORKBUF, &wbuf);
		if (ret != HD_OK) {
			DBG_ERR("vendor_ai_op_set fail=%d\n", (int)ret);
			goto close;
		}
		ret = vendor_ai_op_start(proc_id) ;
		if (ret != HD_OK) {
			DBG_ERR("vendor_ai_op_start fail=%d\n", (int)ret);
			goto close;
		}
	}

    ret = vendor_ais_unlock_net(proc_id);
    if (ret != HD_OK) {
        DBG_ERR("vendor_ais_unlock_net fail=%d\n", (int)ret);
		goto stop;
    }

    *p_op_id = proc_id;
    return ret;

stop:
	ret = vendor_ai_op_stop(proc_id);
    if (ret != HD_OK) {
        DBG_ERR("vendor_ai_op_stop fail=%d\n", (int)ret);
    }
close:
	ret = vendor_ai_op_close(proc_id);
    if (ret != HD_OK) {
        DBG_ERR("vendor_ai_op_close fail=%d\n", (int)ret);
    }
unlock:
	ret = vendor_ais_unlock_net(proc_id);
    if (ret != HD_OK) {
        DBG_ERR("vendor_ais_unlock_net fail=%d\n", (int)ret);
    }
release:
	ret = vendor_ai_release_id(proc_id);
	if (ret != HD_OK) {
		DBG_ERR("release proc_id(%d) failed, ret = %d\r\n", (int)proc_id, (int)ret);
	}
uninit:
	ret = vendor_ai_uninit();
	if (ret != HD_OK) {
		DBG_ERR("vendor_ai_uninit fail=%d\n", (int)ret);
	}
exit:
	return HD_ERR_INV;
}

HD_RESULT vendor_ai3_op_do_postprocess (UINT32 op_id, VENDOR_AI3_BUF* src, VENDOR_AI3_BUF* dst, VENDOR_AI3_OP_POSTPROC_PARAM* param)
{
    HD_RESULT ret = HD_OK;
    VENDOR_AI3_OP_POSTPROC_PARAM p_parm = {0};

    ret = vendor_ai_check_postproc_format(src, dst);
	if (ret != HD_OK) {
		DBG_ERR("vendor_ai_check_postproc_format fail=%d\n", (int)ret);
		return HD_ERR_LIMIT;
	}

	ret = vendor_ais_lock_net(op_id);
    if (ret != HD_OK) {
		DBG_ERR("vendor_ais_lock_net failed !! ret = %d\r\n", (int)ret);
		goto exit;
	}

	{
		// AI_OP_POSTPROC_BAYER16TPBAYER12
		if (dst[0].fmt == HD_VIDEO_PXLFMT_RAW12){
			ret = vendor_ai_op_proc(op_id, VENDOR_AI_OP_POSTPROC, &p_parm, 1, src, 1, dst);
			if (ret != HD_OK) {
				DBG_ERR("vendor_ai_op_proc fail=%d\n", (int)ret);
				goto exit;
			}
		// AI_OP_POSTPROC_BAYER12TPBAYER12
		}else if (dst[0].fmt == HD_VIDEO_PXLFMT_RAW16){
			ret = vendor_ai_op_proc(op_id, VENDOR_AI_OP_POSTPROC, &p_parm, 1, src, 1, dst);
			if (ret != HD_OK) {
				DBG_ERR("vendor_ai_op_proc fail=%d\n", (int)ret);
				goto exit;
			}
		}else{
			ret = vendor_ai_op_proc(op_id, VENDOR_AI_OP_POSTPROC, &p_parm, 3, src, 2, dst);
			if (ret != HD_OK) {
				DBG_ERR("vendor_ai_op_proc fail=%d\n", (int)ret);
				goto exit;
			}
		}
	}

	ret = vendor_ais_unlock_net(op_id);
    if (ret != HD_OK) {
        DBG_ERR("vendor_ais_unlock_net fail=%d\n", (int)ret);
		goto exit;
    }

    return ret;

exit:
	ret = vendor_ai_op_stop(op_id);
	if (ret != HD_OK) {
        DBG_ERR("vendor_ai_op_stop fail=%d\n", (int)ret);
    }

	ret = vendor_ai_op_close(op_id);
	if (ret != HD_OK) {
        DBG_ERR("vendor_ai_op_close fail=%d\n", (int)ret);
    }

	ret = vendor_ais_unlock_net(op_id);
	if (ret != HD_OK) {
        DBG_ERR("vendor_ais_unlock_net fail=%d\n", (int)ret);
    }

	ret = vendor_ai_release_id(op_id);
	if (ret != HD_OK) {
		DBG_ERR("release proc_id(%d) failed, ret = %d\r\n", (int)op_id, (int)ret);
	}

	ret = vendor_ai_uninit();
	if (ret != HD_OK) {
		DBG_ERR("vendor_ai_uninit fail=%d\n", (int)ret);
	}

    return HD_ERR_INV;
}

HD_RESULT vendor_ai3_op_uninit_postprocess (UINT32 op_id)
{
    HD_RESULT ret = HD_OK;

    ret = vendor_ais_lock_net(op_id);
    if (ret != HD_OK) {
		DBG_ERR("vendor_ais_lock_net failed !! ret = %d\r\n", (int)ret);
	}

    ret = vendor_ai_op_stop(op_id);
    if (ret != HD_OK) {
        DBG_ERR("vendor_ai_op_stop fail=%d\n", (int)ret);
    }

	ret = vendor_ai_op_close(op_id);
    if (ret != HD_OK) {
        DBG_ERR("vendor_ai_op_close fail=%d\n", (int)ret);
    }

	ret = vendor_ais_unlock_net(op_id);
    if (ret != HD_OK) {
        DBG_ERR("vendor_ais_unlock_net fail=%d\n", (int)ret);
    }

	ret = vendor_ai_release_id(op_id);
	if (ret != HD_OK) {
		DBG_ERR("release proc_id(%d) failed, ret = %d\r\n", (int)op_id, (int)ret);
	}
	
	ret = vendor_ai_uninit();
	if (ret != HD_OK) {
		DBG_ERR("vendor_ai_uninit fail=%d\n", (int)ret);
	}
	return ret;
}