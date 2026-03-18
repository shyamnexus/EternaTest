/**
    @brief Source file of kflow_ai_isp.

    @file kflow_ai_isp.c

    @ingroup kflow_ai_isp

    @note Nothing.

    Copyright Novatek Microelectronics Corp. 2018.  All rights reserved.
*/

/*-----------------------------------------------------------------------------*/
/* Include Files                                                               */
/*-----------------------------------------------------------------------------*/

#include "kwrap/type.h"
#include "kwrap/error_no.h"
#include "kwrap/file.h"
#include "kwrap/platform.h"
#include "kwrap/semaphore.h"
#include "kwrap/cpu.h"

#include "ai_ioctl.h" //for AI_DRV_OPENCFG
#include "kdrv_ai.h"
#include "kflow_ai_net/kflow_ai_net_platform.h"
//=============================================================
#define __CLASS__ 				"[ai][kflow_isp][isp]"
#include "kflow_ai_isp_debug.h"
//=============================================================
#include "kflow_isp/kflow_isp_platform.h"
#include "kflow_isp/kflow_isp.h"
#include "kflow_isp/kflow_isp_change_res.h"
#include "kflow_ai_isp_build_instrs_sync_sdktool.h"
#include "kflow_isp/kflow_ai_isp_task.h"

#if (FLOW_AI_ISP == 1)
static KFLOW_AI_ISP_CB _kflow_ai_callback = 0;
extern ID FLG_ID_JMISP ;
extern UINT32 g_isp_trig_mode ; 
extern INT32 *kflow_isp_sensor_map;
extern UINT32 *kflow_isp_push_time ;
extern UINT32 *kflow_isp_first_proc;
extern NN_ISP_ENABLE_3D *kflow_isp_enable_3d;
extern UINT32 *kflow_isp_weight_loc_map ; 
extern UINT32 *kflow_isp_proc_core_mask;
extern INT32 kflow_isp_used_pool[4];
extern UINT32 kflow_isp_pool[4][5];
extern INT32 *kflow_isp_proc_pool_map;
extern NN_ISP_NN_INFO *kflow_isp_nn_info;
extern NN_ISP_PATH_BUF_INFO *kflow_isp_ubuf_layout;
extern KDRV_AI_JMISP_PATH_INFO *kflow_isp_drv_para;
extern NN_ISP_INPUT **g_ai_input_layer_map_table;
extern uintptr_t **g_ai_input_addr_map_table;
extern VENDOR_AIS_ISP_INPUT_INFO **g_ai_input_layer_info_table;
extern KDRV_AI_JMISP_PATH_INFO  **g_ai_isp_p_drv;
extern KFLOW_AI_ISP_JOB* kflow_isp_job_statck ; 
extern VENDOR_AIS_ISP_PARM * g_ai_isp_parm; 
extern AI_JMISP_INFO *g_ai_jmisp_info ; 
extern BOOL nvt_ai_check_proc_id(UINT32 proc_id) ; 
extern ER nvt_ai_lock_net(UINT32 net_id) ;
extern ER nvt_ai_unlock_net(UINT32 net_id) ;
extern SEM_HANDLE g_ai_isp_id ;
extern SEM_HANDLE g_ai_isp_dgain_lock;
extern SEM_HANDLE g_kflow_ai_isp_init_uninit_sem_id;
extern AI_TUNING_QUAN_PARAM **g_ai_isp_ori_ppu_quan_param;
extern UINT8 *g_ai_isp_pre_dgain;
extern int kflow_ai_isp_dump_jmisp_buf(UINT32 proc_id) ;
extern UINT32 *g_proc_trace ;
static VK_DEFINE_SPINLOCK(my_lock);
#define loc_cpu(flags)	vk_spin_lock_irqsave(&my_lock, flags)
#define unl_cpu(flags)	vk_spin_unlock_irqrestore(&my_lock, flags)  
#define JMISP_ENG_CLK  240
UINT32 jmisp_time = 0, job_cnt = 0, stack_size = 0  ;

#define JOBLL_DEBUG     0 
   


ER kflow_isp_set_id (NN_ISP_IN_PARAM* in_info)
{
	UINT32 id = in_info->isp_id ; //indicate which sensor to run
	// UINT32 prev_proc_id, i; 
	UINT32 proc_id = in_info->proc_id ;
	// AI_JMISP_INFO * jmisp_info = (AI_JMISP_INFO *)g_ai_isp_parm[proc_id].jmisp_info ;

	kflow_isp_net_trace(proc_id, AIISP_FLOW, "get() - isp_id(%u)\r\n", in_info->isp_id);
	SEM_WAIT(g_ai_isp_id);

	// if (kflow_isp_sensor_map[id] != -1 ){
	// 	prev_proc_id = kflow_isp_sensor_map[id] ;

	// 	if (g_ai_jmisp_info[prev_proc_id].iobuf[e_AI_JMISP_IOBUF_REF_IN0].va && 
	// 	   jmisp_info->iobuf[e_AI_JMISP_IOBUF_REF_IN0].pa != g_ai_jmisp_info[prev_proc_id].iobuf[e_AI_JMISP_IOBUF_REF_IN0].pa){

	// 		for (i= e_AI_JMISP_IOBUF_REF_IN0 ; i <= e_AI_JMISP_IOBUF_REF_OUT1; i++ ){
	// 			if(jmisp_info->iobuf[i].va && g_ai_jmisp_info[prev_proc_id].iobuf[i].va){
	// 				memcpy((void*)jmisp_info->iobuf[i].va, (void*)g_ai_jmisp_info[prev_proc_id].iobuf[i].va, g_ai_jmisp_info[prev_proc_id].iobuf[i].buffer_sz ) ;
    //                 vos_cpu_dcache_sync(jmisp_info->iobuf[i].va, jmisp_info->iobuf[i].buffer_sz, VOS_DMA_TO_DEVICE);
	// 			}
	// 		}
	// 		DBG_MSG(" Copy ref from proc[%u]\n", prev_proc_id);
	// 	}
	// }
	kflow_isp_sensor_map[id] = proc_id ;
	SEM_SIGNAL(g_ai_isp_id);

	return E_OK ;
}

ER kflow_isp_release_id (NN_ISP_IN_PARAM* in_info)
{
	UINT32 id = in_info->isp_id ; //indicate which sensor to run
	UINT32 proc_id = in_info->proc_id ;

	SEM_WAIT(g_ai_isp_id);

	if (kflow_isp_sensor_map[id] == proc_id){
		kflow_isp_sensor_map[id] = -1 ;
	}
	SEM_SIGNAL(g_ai_isp_id);
	return E_OK ;
	
}	

ER swap_ref_at_drv_para(UINT32 proc_id, const e_AI_JMISP_IOBUF_CONST iobuf_in, const e_AI_JMISP_IOBUF_CONST iobuf_out)
{
    uint64_t addr_offset;
    if ((iobuf_in < 0) || (iobuf_out < 0)) {
        return E_NOEXS;
    }
    if ((g_ai_jmisp_info[proc_id].iobuf[iobuf_in].buffer_sz > 0) && (g_ai_jmisp_info[proc_id].iobuf[iobuf_out].buffer_sz > 0)) {
        const e_AI_JMISP_DRV_IO_CONST drv_ref_in = convert_iobuf2drvio_type(&g_ai_jmisp_info[proc_id].iobuf[iobuf_in], iobuf_in);
        const e_AI_JMISP_DRV_IO_CONST drv_ref_out = convert_iobuf2drvio_type(&g_ai_jmisp_info[proc_id].iobuf[iobuf_out], iobuf_out);
        if ((drv_ref_in < 0) || (drv_ref_out < 0)) {
            return E_OBJ;
        }
        addr_offset = kflow_isp_drv_para[proc_id].frame[drv_ref_in].frame_addr ; 
        kflow_isp_drv_para[proc_id].frame[drv_ref_in].frame_addr = kflow_isp_drv_para[proc_id].frame[drv_ref_out].frame_addr;
        kflow_isp_drv_para[proc_id].frame[drv_ref_out].frame_addr = addr_offset ; 

        kflow_isp_net_trace(proc_id, AIISP_FLOW, "frame[%d].frame_addr  = %lx \n", drv_ref_in, (ULONG)kflow_isp_drv_para[proc_id].frame[drv_ref_in].frame_addr);
	    kflow_isp_net_trace(proc_id, AIISP_FLOW, "frame[%d].frame_addr  = %lx \n", drv_ref_out, (ULONG)kflow_isp_drv_para[proc_id].frame[drv_ref_out].frame_addr);
    }
    return E_OK;
}

ER kflow_isp_swap_ref(UINT32 proc_id)
{
	INT32 jmisp_id ; 
	INT32 pool_id = kflow_isp_proc_pool_map[proc_id] ; 

	kflow_isp_net_trace(proc_id, AIISP_FLOW, " =============== kflow_isp_swap_ref =============\n");

	if(pool_id < 0 ){
		if (kflow_isp_get_st_in_core_mask(kflow_isp_proc_core_mask[proc_id], &jmisp_id) != E_OK){
			DBG_ERR("proc_id(%u) is not using any core_mask !\r\n", proc_id);
			return E_SYS;
		}
	}else{
		// check whether pool is valid 
		if (pool_id > 3){
			DBG_ERR("pool_%u is illegal ! \n", pool_id);
			return E_CTX ; 
		}

		jmisp_id = kflow_isp_pool[pool_id][3] ; 
	}

	if(jmisp_id > 3 || jmisp_id < 0){
		DBG_ERR("jmisp_id(%d)is not valid...\r\n", jmisp_id);
		return E_ID ;
	}

    kflow_isp_net_trace(proc_id, AIISP_FLOW, "path_id = %u \n", kflow_isp_drv_para[proc_id].path_id);
    // Swap REF_IN, REF_OUT
    swap_ref_at_drv_para(proc_id, e_AI_JMISP_IOBUF_REF_IN0, e_AI_JMISP_IOBUF_REF_OUT0);
    swap_ref_at_drv_para(proc_id, e_AI_JMISP_IOBUF_REF_IN1, e_AI_JMISP_IOBUF_REF_OUT1);
	// set JMISP_PARAM
	kdrv_ai_set(KDRV_AI_PARAM_JMISP_PARAM, &kflow_isp_drv_para[proc_id]); 

#if (defined(_BSP_NS02302_))
	{	
		UINT32 i = 0 , skip;
		// run hw mode 
		if (g_ai_isp_parm[proc_id].core_num == 2){
			kflow_isp_drv_para[proc_id].wait_en = 1 ;
			for (i=0 ; i < 2 ; i++){
				kflow_isp_drv_para[proc_id].wait_en = 1 ;
				kflow_isp_drv_para[proc_id].path_id = i + jmisp_id ;
				skip = 0xA >> i;
				kflow_isp_drv_para[proc_id].blk_skip = skip ; 
				// set JMISP_PARAM
				kdrv_ai_set(KDRV_AI_PARAM_JMISP_PARAM, &kflow_isp_drv_para[proc_id]); 
			}
		}else if (g_ai_isp_parm[proc_id].core_num == 4){
			for (i=0 ; i < g_ai_isp_parm[proc_id].core_num ; i++){
				kflow_isp_drv_para[proc_id].wait_en = 1 ;
				kflow_isp_drv_para[proc_id].path_id = i ;
				skip = 0xf;
				skip = skip & ~ (1 << i) ; 
				kflow_isp_drv_para[proc_id].blk_skip = skip ; 
				// set JMISP_PARAM
				kdrv_ai_set(KDRV_AI_PARAM_JMISP_PARAM, &kflow_isp_drv_para[proc_id]); 
			}

		}
	}
#endif


	return E_OK;
}


//----------------------------------- kflow_isp_cb -----------------------------------//
ER kflow_isp_get_max_strp(UINT32 proc_id, NN_ISP_NN_STRP* nn_strp)
{
	AI_JMISP_INFO* jmisp_info  ; 
	StripeInfo stripe ;
	BlockInfo block ;
	ER er = E_OK ;

	jmisp_info = (AI_JMISP_INFO*)g_ai_isp_parm[proc_id].jmisp_info ; 
	stripe = jmisp_info->iobuf[get_benchmark_iobuf_type()].stripeInfo ;
	block  = jmisp_info->iobuf[get_benchmark_iobuf_type()].blockInfo;
	nn_strp->max_stripe  = max(max(stripe.first_stripe_width, stripe.mid_stripe_width), stripe.last_stripe_width) ; 
	nn_strp->min_stripe = block.block_width ;     
	
	return er;
}
ER kflow_isp_update_jmisp_kdrv(KDRV_AI_JMISP_PATH_INFO* drv_para, NN_ISP_SET_RES_PARAM* in_info, AI_JMISP_INFO* jmisp_info)
{

	AI_JMISP_IOBUF_INFO* p_iobuf;
	ER er = E_OK ;
	uint32_t max_stripe_sz, slice_buf_sz;
	UINT32 proc_id = in_info->proc_id ; 
	INT32 pool_id = kflow_isp_proc_pool_map[proc_id], jmisp_id ; 
    const uint64_t slice_cnt_for_pingpong = 2;
	const uint64_t slice_cnt_for_ring = 3;
	const uint64_t lsb_mask = 0xffffffff;
    int32_t i;

	if(pool_id < 0 ){
		if (kflow_isp_get_st_in_core_mask(kflow_isp_proc_core_mask[proc_id], &jmisp_id) != E_OK){
			DBG_ERR("proc_id(%u) is not using any core_mask !\r\n", proc_id);
			return E_SYS;
		}
	}else{
		// check whether pool is valid 
		if (pool_id > 3){
			DBG_ERR("pool_%u is illegal ! \n", pool_id);
			return E_CTX ; 
		}

		jmisp_id = kflow_isp_pool[pool_id][3] ; 
	}

	if(jmisp_id > 3 || jmisp_id < 0){
		DBG_ERR("jmisp_id(%d)is not valid...\r\n", jmisp_id);
		return E_ID ;
	}

	drv_para->handshake = in_info->ipp_id * 4 +  in_info->path_id + 1; 
	drv_para->path_id = jmisp_id ;

    {
        for (i=0; i<=e_AI_IOBUF_NUM; ++i) {
            const e_AI_JMISP_IOBUF_CONST iobuf_type = (e_AI_JMISP_IOBUF_CONST)i;
            const e_AI_JMISP_DRV_IO_CONST drvio_type = convert_iobuf2drvio_type(&jmisp_info->iobuf[iobuf_type], iobuf_type);
            if (drvio_type == e_AI_JMISP_DRV_IO_UNKNOWN) {
                continue;
            }
            p_iobuf = &jmisp_info->iobuf[iobuf_type] ; 
            if (p_iobuf->buffer_sz <= 0) {
                continue;
            }
            if (is_pingpong_buf(p_iobuf, iobuf_type)) {
                if (drvio_type >= KDRV_AI_JMISP_MAX_PINGPONG_NUM) {
                    DBG_WRN("out of pingpong buffer boundary\r\n");
                    return E_MACV;
                }
                max_stripe_sz = max(max(p_iobuf->stripeInfo.first_stripe_width, p_iobuf->stripeInfo.mid_stripe_width), p_iobuf->stripeInfo.last_stripe_width);
                slice_buf_sz = p_iobuf->stripeInfo.output_slice_height * max_stripe_sz * p_iobuf->bitdepth >> 3;
                drv_para->pingpong[drvio_type].pingpong_addr0 = p_iobuf->pa ; 
                drv_para->pingpong[drvio_type].pingpong_addr1 = p_iobuf->pa + slice_buf_sz; 
                if ((slice_buf_sz*slice_cnt_for_pingpong) > p_iobuf->buffer_sz) {
                    DBG_ERR("ping pong buffer %d size not enough...\r\n", drvio_type);
                }
            } else if (is_ring_buf(p_iobuf, iobuf_type)) {
                if (drvio_type >= KDRV_AI_JMISP_MAX_FRAME_NUM) {
                    DBG_WRN("out of frame buffer boundary\r\n");
                    return E_MACV;
                }
                drv_para->frame[drvio_type].frame_addr = p_iobuf->pa ; 
                if (drv_para->frame[drvio_type].ring_en ) {
                    max_stripe_sz = max(max(p_iobuf->stripeInfo.first_stripe_width, p_iobuf->stripeInfo.mid_stripe_width), p_iobuf->stripeInfo.last_stripe_width);
                    slice_buf_sz = p_iobuf->stripeInfo.output_slice_height * max_stripe_sz * p_iobuf->bitdepth >> 3;
                    drv_para->frame[drvio_type].ring_start_addr = p_iobuf->pa ; 

                    if (g_ai_isp_parm[in_info->proc_id].aiisp_mode == 0){
                        // run hw mode => ring_buf cnt = 3  
                        drv_para->frame[drvio_type].ring_end_addr = p_iobuf->pa + slice_buf_sz * slice_cnt_for_ring ; 
                    }else {
                        // run sw mode => ring_buf cnt = slice cnt 
                        uint64_t slice_cnt = 0;
                        slice_cnt =  (in_info->height / p_iobuf->stripeInfo.output_slice_height ) + (((in_info->height / p_iobuf->stripeInfo.output_slice_height)>0) ? 1 :0) ; 
                        drv_para->frame[drvio_type].ring_end_addr = p_iobuf->pa + slice_buf_sz * slice_cnt;

                    }

                    if ((slice_buf_sz * slice_cnt_for_ring) > p_iobuf->buffer_sz) {
                        DBG_WRN("ring buffer %d size not enough...\r\n", drvio_type);
                    }
                }
            } else if (is_frame_buf(p_iobuf, iobuf_type)) {
                if (drvio_type >= KDRV_AI_JMISP_MAX_FRAME_NUM) {
                    DBG_WRN("out of frame buffer boundary\r\n");
                    return E_MACV;
                }
                drv_para->frame[drvio_type].frame_addr = p_iobuf->pa ;
            } else if (is_cust_pipe_buf(p_iobuf, iobuf_type)) {
                if (drvio_type >= KDRV_AI_JMISP_MAX_PIPEBUFF_NUM) {
                    DBG_WRN("out of pipe buffer boundary\r\n");
                    return E_MACV;
                }
                drv_para->ppu_pipebuf_lsbaddr[drvio_type] = (UINT32)(p_iobuf->pa & lsb_mask) ;
            }
        }
    }

    //TODO: why? del?
	kflow_isp_ubuf_layout[proc_id].ring_y_end_addr = drv_para->frame[e_AI_JMISP_DRV_IO_RING_BUF0].ring_end_addr;
	kflow_isp_ubuf_layout[proc_id].ring_uv_end_addr = drv_para->frame[e_AI_JMISP_DRV_IO_RING_BUF1].ring_end_addr; 

	kflow_isp_net_trace(in_info->proc_id, AIISP_BUF, "set() - set_jmisp_buf()");
    for (i=0; i<KDRV_AI_JMISP_MAX_PINGPONG_NUM; ++i) {
        kflow_isp_net_trace(in_info->proc_id, AIISP_BUF, "set() - pingpong[%d]_addr0        %lx \n", i, (ULONG)drv_para->pingpong[i].pingpong_addr0);
        kflow_isp_net_trace(in_info->proc_id, AIISP_BUF, "set() - pingpong[%d]_addr1        %lx \n", i, (ULONG)drv_para->pingpong[i].pingpong_addr1);
    }
    for (i=0; i<KDRV_AI_JMISP_MAX_FRAME_NUM; ++i) {
        kflow_isp_net_trace(in_info->proc_id, AIISP_BUF, "set() - frame[%d]_ring_start_addr %lx \n", i, (ULONG)drv_para->frame[i].ring_start_addr );
        kflow_isp_net_trace(in_info->proc_id, AIISP_BUF, "set() - frame[%d]_ring_end_addr   %lx \n", i, (ULONG)drv_para->frame[i].ring_end_addr);
        kflow_isp_net_trace(in_info->proc_id, AIISP_BUF, "set() - frame[%d]_ring_en         %u \n", i, drv_para->frame[i].ring_en );
        kflow_isp_net_trace(in_info->proc_id, AIISP_BUF, "set() - frame[%d]_addr            %lx \n", i, (ULONG)drv_para->frame[i].frame_addr);
    }
    for (i=0; i<KDRV_AI_JMISP_MAX_PIPEBUFF_NUM; ++i) {
        kflow_isp_net_trace(in_info->proc_id, AIISP_BUF, "set() - pipebuf[%d]_addr          %lx \n", i, (ULONG)drv_para->ppu_pipebuf_lsbaddr[i]);
    }
    {
        kflow_isp_net_trace(in_info->proc_id, AIISP_BUF, "set() - path_id                   %d \n", (int)drv_para->path_id);
        kflow_isp_net_trace(in_info->proc_id, AIISP_BUF, "set() - handshake                 %d \n", (int) drv_para->handshake);
    }
	return er;
}
ER kflow_isp_cal_nn_info (NN_ISP_NN_INFO* nn_info,  AI_JMISP_INFO* jmisp_info)
{
	StripeInfo stripe = jmisp_info->iobuf[get_benchmark_iobuf_type()].stripeInfo ; 
	BlockInfo block = jmisp_info->iobuf[get_benchmark_iobuf_type()].blockInfo ;
	nn_info->slice_max_out_height = stripe.output_slice_height ; 
	nn_info->slice_overlap = stripe.slice_overlap_height ; 
	nn_info->max_stripe = max(max(stripe.first_stripe_width, stripe.mid_stripe_width), stripe.last_stripe_width) ;
	nn_info->min_stripe = block.block_width ;

	return E_OK;
}

static INT32 kflow_jmisp_pla_isrcb(UINT32 cycle_eng, UINT32 cycle_ll, UINT32 cycle_dma, UINT32 intstatus, UINT32 eng, void *parm)
{
	DBG_MSG("kflow_jmisp_pla_isrcb-- cycle_eng=%u cycle_dma=%u \n", cycle_eng/JMISP_ENG_CLK, cycle_dma) ; 
	set_flg(FLG_ID_JMISP, FLG_ID_JMISP_A); //set JMISPA flag done

	return 0;	
}
static INT32 kflow_jmisp_plb_isrcb(UINT32 cycle_eng, UINT32 cycle_ll, UINT32 cycle_dma, UINT32 intstatus, UINT32 eng, void *parm)
{
	set_flg(FLG_ID_JMISP, FLG_ID_JMISP_B); //set JMISPB flag done

	return 0;	
}
static INT32 kflow_jmisp_plc_isrcb(UINT32 cycle_eng, UINT32 cycle_ll, UINT32 cycle_dma, UINT32 intstatus, UINT32 eng, void *parm)
{
	set_flg(FLG_ID_JMISP, FLG_ID_JMISP_C); //set JMISPC flag done

	return 0;	
}
static INT32 kflow_jmisp_pld_isrcb(UINT32 cycle_eng, UINT32 cycle_ll, UINT32 cycle_dma, UINT32 intstatus, UINT32 eng, void *parm)
{

	set_flg(FLG_ID_JMISP, FLG_ID_JMISP_D); //set JMISPD flag done

	return 0;	
}
ER kflow_isp_copy_weight (NN_ISP_SET_RES_PARAM* in_info)
{
	uintptr_t temp_va, temp_pa ; 
	VENDOR_AIS_ISP_MAP_MEM_PARM* p_mem_out = &g_ai_isp_parm[in_info->proc_id].p_info.parm;
	
	if (kflow_isp_weight_loc_map[in_info->proc_id]){
	
		temp_pa = p_mem_out->tcm_buff.pa  + p_mem_out->tcm_buff.size - 64 - ALIGN_CEIL_64(p_mem_out->user_model.size) ;
		temp_va = p_mem_out->tcm_buff.va  + p_mem_out->tcm_buff.size - 64 - ALIGN_CEIL_64(p_mem_out->user_model.size) ;
		memcpy((VOID *)temp_va, (VOID *)p_mem_out->user_model.va,  p_mem_out->user_model.size) ;
		vos_cpu_dcache_sync(temp_va, p_mem_out->user_model.size, VOS_DMA_TO_DEVICE); ///< cache clean - output to engine's input
	}

	return E_OK ; 
}
ER kflow_isp_set_callback_func (UINT32 proc_id)
{
	INT32 pool_id = kflow_isp_proc_pool_map[proc_id], jmisp_id ; 
    UINT32 i; 
	KDRV_AI_CBFUNC_PARAM cb_param = {0};

	if(pool_id < 0 ){
		if (kflow_isp_get_st_in_core_mask(kflow_isp_proc_core_mask[proc_id], &jmisp_id) != E_OK){
			DBG_ERR("proc_id(%u) is not using any core_mask !\r\n", proc_id);
			return E_SYS;
		}
	}else{
		// check whether pool is valid 
		if (pool_id > 3){
			DBG_ERR("pool_%u is illegal ! \n", pool_id);
			return E_CTX ; 
		}

		jmisp_id = kflow_isp_pool[pool_id][3] ; 
	}

	if(jmisp_id > 3 || jmisp_id < 0){
		DBG_ERR("jmisp_id(%d)is not valid...\r\n", jmisp_id);
		return E_ID ;
	}
	
	for (i = 0 ; i < g_ai_isp_parm[proc_id].core_num ; i++){
		switch (jmisp_id + i )
			{
				case 0:
					cb_param.eng       = KDRV_AI_ENG_JMISP_PLA;
					cb_param.isrcb_fp = kflow_jmisp_pla_isrcb;
					break;
				case 1:
					cb_param.eng       = KDRV_AI_ENG_JMISP_PLB; 
					cb_param.isrcb_fp = kflow_jmisp_plb_isrcb;
					break;
				case 2:
					cb_param.eng       = KDRV_AI_ENG_JMISP_PLC;
					cb_param.isrcb_fp = kflow_jmisp_plc_isrcb;
					break;
				case 3:
					cb_param.eng       = KDRV_AI_ENG_JMISP_PLD; 
					cb_param.isrcb_fp = kflow_jmisp_pld_isrcb;
					break;			
				
				default:
					DBG_ERR("unknown isp jmisp_id : %d\r\n", (int)jmisp_id);
					return E_NOSPT;
			}
	kdrv_ai_set(KDRV_AI_PARAM_CBFUNC, &cb_param);
	kflow_isp_net_trace(proc_id, AIISP_FLOW, "set() - set_callback_func() jmisp_ch(%d)  \n", jmisp_id + i);
	}
	return E_OK ; 
}

ER kflow_isp_set_base_addr (UINT32 proc_id)
{
	ER er = E_OK ;
	KDRV_AI_BASE_ADDR_PARAM base_addr_parm;
    VENDOR_AIS_ISP_MAP_MEM_PARM* p_mem_out = &g_ai_isp_parm[proc_id].p_info.parm;
	uintptr_t parm_pa_ofs, model_pa_ofs, buff_pa_ofs, tcm_pa_ofs=0, tcm_va_ofs = 0;
	UINT32  each_ai_parm_size = 0, ch_mask, core_mask = 0 ; 
    INT32 pool_id = kflow_isp_proc_pool_map[proc_id] , i, jmisp_id = -4, st_id = -1; 

    if(pool_id < 0 ){
		if (kflow_isp_proc_core_mask[proc_id] == 0){
			DBG_ERR("proc_id(%u) is not using any core_mask !\r\n", proc_id);
			return E_SYS;
		}
		core_mask = kflow_isp_proc_core_mask[proc_id];
	}else{
		 // check whether pool is valid 
		if (pool_id > 3){
			DBG_ERR("pool_%u is illegal ! \n", pool_id);
			return E_CTX ; 
		}

		jmisp_id = kflow_isp_pool[pool_id][3] ; 

		if(jmisp_id > 3){
			DBG_ERR("jmisp_id is not valid...\r\n");
			return E_ID ;
		}
	}


    if (g_ai_isp_parm[proc_id].pre_in0_en){
        parm_pa_ofs =  g_ai_isp_parm[proc_id].p_info.parm.user_parm.pa + (g_ai_isp_parm[proc_id].p_ai_parm_list - g_ai_isp_parm[proc_id].p_info.parm.user_parm.va) ;
        each_ai_parm_size = g_ai_isp_parm[proc_id].each_ai_parm_size ; 
   }else {
        parm_pa_ofs =  ALIGN_CEIL_4(g_ai_isp_parm[proc_id].p_info.parm.user_parm.pa);
    }
	model_pa_ofs = ALIGN_CEIL_4(g_ai_isp_parm[proc_id].p_info.parm.user_model.pa);
	buff_pa_ofs = ALIGN_CEIL_4(g_ai_isp_parm[proc_id].p_info.parm.user_buff.pa);
#if (defined(_BSP_NS02201_) || defined(_BSP_NS02302_) || defined(_BSP_NS02401_))	
	tcm_pa_ofs = ALIGN_CEIL_4(g_ai_isp_parm[proc_id].p_info.parm.tcm_buff.pa);
    tcm_va_ofs = ALIGN_CEIL_4(g_ai_isp_parm[proc_id].p_info.parm.tcm_buff.va);
    if (kflow_isp_weight_loc_map[proc_id]){
		model_pa_ofs = p_mem_out->tcm_buff.pa  + p_mem_out->tcm_buff.size - 64 - ALIGN_CEIL_64(p_mem_out->user_model.size) ;
	}
#endif	
   if ((parm_pa_ofs == 0)
			|| (model_pa_ofs == 0)
			|| (buff_pa_ofs == 0)
			|| (tcm_pa_ofs == 0)
            || (tcm_va_ofs == 0)) {
		DBG_ERR("null memory...\r\n");
		return E_CTX;
	}
     

    kflow_isp_net_trace(proc_id, AIISP_FLOW, "set() - set_base_addr() \r\n");
    DBG_MSG(" each_ai_parm_size = %u each_dram_iobuf_size %u each_ubuf_budget %u \n", each_ai_parm_size, g_ai_isp_parm[proc_id].each_dram_iobuf_size, g_ai_isp_parm[proc_id].each_ubuf_budget);

	for (i = 0 ; i <  g_ai_isp_parm[proc_id].core_num ; i++){

		if(pool_id < 0 && core_mask){
			 if(kflow_isp_get_st_in_core_mask(core_mask, &st_id) != E_OK){
                DBG_ERR("kflow_isp_get_st_in_core_mask fail proc_id(%u) core_mask is needed !\r\n", proc_id);
                return E_PAR;
            }

			if(st_id > 3 || st_id < 0){
				DBG_ERR("st_id(%d)is not valid...\r\n", st_id);
				return E_ID ;
			}

			ch_mask = (1L << (st_id));
			kflow_isp_net_trace(proc_id, AIISP_FLOW, "set() - st_id(%u) core_mask(0x%02x) !", st_id, core_mask);
			core_mask &= (~ch_mask); // shift it to get the next jobm core
		}
		
		
        if( ((i + jmisp_id) == 0) || st_id == 0){
        
            base_addr_parm.eng = KDRV_AI_ENG_JMISP_PLA;  
            base_addr_parm.pa = (uintptr_t)(model_pa_ofs) ;
            base_addr_parm.base_id = 0; 
            kdrv_ai_set(KDRV_AI_PARAM_BASE_ADDR, &base_addr_parm);
            base_addr_parm.pa = (uintptr_t)(buff_pa_ofs) ; 
            base_addr_parm.base_id = 1; 
            kdrv_ai_set(KDRV_AI_PARAM_BASE_ADDR, &base_addr_parm);
            base_addr_parm.pa = (uintptr_t)(parm_pa_ofs) ; 
            base_addr_parm.base_id = 2; 
            kdrv_ai_set(KDRV_AI_PARAM_BASE_ADDR, &base_addr_parm);
            base_addr_parm.pa = (uintptr_t)(tcm_pa_ofs) ; 
            base_addr_parm.base_id = 3; 
            kdrv_ai_set(KDRV_AI_PARAM_BASE_ADDR, &base_addr_parm);
            kflow_isp_net_trace(proc_id, AIISP_FLOW, "set() - JMISP_PLA buff_pa_of %lx parm_pa_ofs %lx tcm_pa_ofs %lx \n",(ULONG)buff_pa_ofs, (ULONG)parm_pa_ofs, (ULONG)(tcm_pa_ofs) );
        }else if(((i + jmisp_id) == 1) || st_id == 1){

            base_addr_parm.eng = KDRV_AI_ENG_JMISP_PLB; 
            base_addr_parm.pa = (uintptr_t)(model_pa_ofs) ;
            base_addr_parm.base_id = 0; 
            kdrv_ai_set(KDRV_AI_PARAM_BASE_ADDR, &base_addr_parm); 
            base_addr_parm.pa = (uintptr_t)(buff_pa_ofs + i * g_ai_isp_parm[proc_id].each_dram_iobuf_size) ; 
            base_addr_parm.base_id = 1; 
            kdrv_ai_set(KDRV_AI_PARAM_BASE_ADDR, &base_addr_parm);
            base_addr_parm.pa = (uintptr_t)(parm_pa_ofs + i * each_ai_parm_size) ; 
            base_addr_parm.base_id = 2; 
            kdrv_ai_set(KDRV_AI_PARAM_BASE_ADDR, &base_addr_parm);
            base_addr_parm.pa = (uintptr_t)(tcm_pa_ofs + i * g_ai_isp_parm[proc_id].each_ubuf_budget ) ; 
            base_addr_parm.base_id = 3; 
            kdrv_ai_set(KDRV_AI_PARAM_BASE_ADDR, &base_addr_parm);
            kflow_isp_net_trace(proc_id, AIISP_FLOW, "set() - JMISP_PLB buff_pa_of %lx parm_pa_ofs %lx tcm_pa_ofs %lx \n",(ULONG)buff_pa_ofs + i * g_ai_isp_parm[proc_id].each_dram_iobuf_size,(ULONG)(parm_pa_ofs + i * each_ai_parm_size), (ULONG)(tcm_pa_ofs + i * g_ai_isp_parm[proc_id].each_ubuf_budget)) ;
        }else if(((i + jmisp_id) == 2) || st_id == 2){

            base_addr_parm.eng = KDRV_AI_ENG_JMISP_PLC;  
            base_addr_parm.pa = (uintptr_t)(model_pa_ofs) ;
            base_addr_parm.base_id = 0; 
            kdrv_ai_set(KDRV_AI_PARAM_BASE_ADDR, &base_addr_parm);
            base_addr_parm.pa = (uintptr_t)(buff_pa_ofs + i * g_ai_isp_parm[proc_id].each_dram_iobuf_size) ; 
            base_addr_parm.base_id = 1; 
            kdrv_ai_set(KDRV_AI_PARAM_BASE_ADDR, &base_addr_parm );
            base_addr_parm.pa = (uintptr_t)(parm_pa_ofs + i * each_ai_parm_size) ; 
            base_addr_parm.base_id = 2; 
            kdrv_ai_set(KDRV_AI_PARAM_BASE_ADDR, &base_addr_parm);
            base_addr_parm.pa = (uintptr_t)(tcm_pa_ofs + i * g_ai_isp_parm[proc_id].each_ubuf_budget) ; 
            base_addr_parm.base_id = 3; 
            kdrv_ai_set(KDRV_AI_PARAM_BASE_ADDR, &base_addr_parm);
            kflow_isp_net_trace(proc_id, AIISP_FLOW, "set() - JMISP_PLC buff_pa_of %lx parm_pa_ofs %lx tcm_pa_ofs %lx \n",(ULONG)buff_pa_ofs + i * g_ai_isp_parm[proc_id].each_dram_iobuf_size,(ULONG)(parm_pa_ofs + i * each_ai_parm_size) ,(ULONG)(tcm_pa_ofs + i * g_ai_isp_parm[proc_id].each_ubuf_budget)); 
        }else if(((i + jmisp_id) == 3) || st_id == 3){

            base_addr_parm.eng = KDRV_AI_ENG_JMISP_PLD; 
            base_addr_parm.pa = (uintptr_t)(model_pa_ofs) ;
            base_addr_parm.base_id = 0; 
            kdrv_ai_set(KDRV_AI_PARAM_BASE_ADDR, &base_addr_parm); 
            base_addr_parm.pa = (uintptr_t)(buff_pa_ofs + i * g_ai_isp_parm[proc_id].each_dram_iobuf_size) ; 
            base_addr_parm.base_id = 1; 
            kdrv_ai_set(KDRV_AI_PARAM_BASE_ADDR, &base_addr_parm);
            base_addr_parm.pa = (uintptr_t)(parm_pa_ofs + i * each_ai_parm_size) ; 
            base_addr_parm.base_id = 2; 
            kdrv_ai_set(KDRV_AI_PARAM_BASE_ADDR, &base_addr_parm);
            base_addr_parm.pa = (uintptr_t)(tcm_pa_ofs +  i * g_ai_isp_parm[proc_id].each_ubuf_budget) ; 
            base_addr_parm.base_id = 3; 
            kdrv_ai_set(KDRV_AI_PARAM_BASE_ADDR, &base_addr_parm);
            kflow_isp_net_trace(proc_id, AIISP_FLOW, "set() - JMISP_PLD buff_pa_of %lx parm_pa_ofs %lx tcm_pa_ofs %lx \n",(ULONG)buff_pa_ofs + i * g_ai_isp_parm[proc_id].each_dram_iobuf_size, (ULONG)(parm_pa_ofs + i * each_ai_parm_size) ,(ULONG)(tcm_pa_ofs +  i * g_ai_isp_parm[proc_id].each_ubuf_budget)  ) ;
        }else {
			if(jmisp_id >= 0)
				DBG_ERR("(jmisp_id %d + g_ai_isp_parm[%u].core_num %u) > 4 overflow !! jmisp only have 4 cores ! \n", jmisp_id, proc_id,  g_ai_isp_parm[proc_id].core_num ) ; 
			else 
				DBG_ERR("st_id(%u) core_mask(0x%02x) is wrong which doesn't map any jobm(4) !", st_id, kflow_isp_proc_core_mask[proc_id]);
			return E_CTX;
		}
        
    }  
   

    kflow_isp_net_trace(proc_id, AIISP_FLOW, "set() - weight_addr %lx dram_addr %lx, para_addr %lx, TCM_addr %lx \n",  (ULONG)model_pa_ofs, (ULONG)buff_pa_ofs, (ULONG)parm_pa_ofs, (ULONG)tcm_pa_ofs );

	return er;
}

ER kflow_isp_set_nn_info (NN_ISP_SET_RES_PARAM* in_info, NN_ISP_NN_INFO* nn_info)
{
	ER er = E_OK ;
	IPP_INFO ipp_info = {0};
	UINT32 proc_id = in_info->proc_id ;
	INT32 pool_id = kflow_isp_proc_pool_map[proc_id], jmisp_id ; 

	memset(&g_ai_jmisp_info[proc_id], 0, sizeof(AI_JMISP_INFO)) ; 

	if(pool_id < 0 ){
		if (kflow_isp_get_st_in_core_mask(kflow_isp_proc_core_mask[proc_id], &jmisp_id) != E_OK){
			DBG_ERR("proc_id(%u) is not using any core_mask !\r\n", proc_id);
			return E_SYS;
		}
	}else{
		// check whether pool is valid 
		if (pool_id > 3){
			DBG_ERR("pool_%u is illegal ! \n", pool_id);
			return E_CTX ; 
		}

		jmisp_id = kflow_isp_pool[pool_id][3] ; 
	}

	if(jmisp_id > 3 || jmisp_id < 0){
		DBG_ERR("jmisp_id(%d)is not valid...\r\n", jmisp_id);
		return E_ID ;
	}
	
// CHANGE_RES  
	ipp_info.frame_width = in_info->width;
	ipp_info.frame_height = in_info->height;
	ipp_info.stripe_overlap_width = in_info->stripe_overlap;
	ipp_info.stripe_num = in_info->stripe_num; 
	ipp_info.first_stripe_width = in_info->stripe_width[0];
	ipp_info.mid_stripe_width = 0;
	ipp_info.last_stripe_width = 0;

	if ( in_info->stripe_num > 1){
		ipp_info.last_stripe_width = in_info->stripe_width[in_info->stripe_num - 1]; 
	}
	if ( in_info->stripe_num > 2){
		ipp_info.mid_stripe_width = in_info->stripe_width[1] ;
	}
	// DBG_DUMP("stripe_overlap %u stripe_num %u first_stripe_width %u \n", ipp_info.stripe_overlap_width,ipp_info.stripe_num,ipp_info.first_stripe_width ) ; 
	er = convert_dyframe2jmisp_info(&ipp_info, (AI_JMISP_INFO*)g_ai_isp_parm[proc_id].jmisp_info, &g_ai_jmisp_info[proc_id]);
	if (er){
		DBG_ERR("convert_dyframe2jmisp_info fail...\r\n");
		return er;
	} 

#if NNISP_DEBUG
	DBG_DUMP("p_ppu_cmd ls va = %lx pa = %lx \r\n",(ULONG)g_ai_isp_parm[proc_id].p_ppu_cmd_list, (ULONG)g_ai_isp_parm[proc_id].p_info.user_parm.pa + (g_ai_isp_parm[proc_id].p_ppu_cmd_list - g_ai_isp_parm[proc_id].p_info.user_parm.va));
#endif
	er = parsing_ppu_instrs( &g_ai_jmisp_info[proc_id], (uintptr_t)g_ai_isp_parm[proc_id].p_ppu_cmd_list);
	if (er){
		DBG_ERR("parsing_ppu_instrs fail...\r\n");
		return er;
	}
	// flush in parsing_ppu_instrs to fix broken image
	//vos_cpu_dcache_sync(g_ai_isp_parm[proc_id].p_ppu_cmd_list, g_ai_isp_parm[proc_id].jmisp_ppu_size, VOS_DMA_TO_DEVICE);

	er = update_jmisp_kdrv_resolution( &g_ai_jmisp_info[proc_id], &kflow_isp_drv_para[proc_id]);
	if (er){
		DBG_ERR("update_jmisp_kdrv_resolution fail...\r\n");
		return er;
	} 

	er = kflow_isp_update_jmisp_kdrv(&kflow_isp_drv_para[proc_id], in_info, &g_ai_jmisp_info[proc_id]) ;
	if (er){
		DBG_ERR("kflow_isp_update_jmisp_kdrv fail...\r\n");
		return er;
	} 

	if (g_ai_isp_parm[proc_id].aiisp_mode == 0){
		// run hw mode 
		er = kflow_isp_set_callback_func(proc_id) ;
		if (er){
			DBG_ERR("proc_id %u kflow_isp_set_callback_func fail...\r\n", proc_id);
			return er;
		}
	}else{
		// run sw mode 
		// set job for sw_mode
		kflow_isp_set_sw_mode_job (proc_id) ; 
	}
					
// set_nn_info
	er = kflow_isp_cal_nn_info (nn_info,  &g_ai_jmisp_info[proc_id]) ;
	if (er){
		DBG_ERR("kflow_isp_cal_nn_info fail...\r\n");
		return er;
	} 

	nn_info->sw_handshake_en = g_ai_isp_parm[proc_id].aiisp_mode ; 
	kflow_isp_nn_info[proc_id] = *nn_info ; 
	kflow_isp_net_trace(proc_id, AIISP_FLOW, "set() - nn_info() slice_max_out_height(%u) slice_overlap(%u) max_stripe(%u) min_stripe(%u) sw_handshake_en(%u) \r\n", nn_info->slice_max_out_height, nn_info->slice_overlap, nn_info->max_stripe, nn_info->min_stripe, nn_info->sw_handshake_en );

	return er;
}
ER kflow_isp_get_nn_info (NN_ISP_IN_PARAM* in_info, NN_ISP_NN_INFO* nn_info)
{
	UINT32 proc_id = in_info->proc_id ;

	
	if (in_info == 0 || in_info == NULL ){
		DBG_ERR("in_info is null ...\r\n");
		return E_SYS ;
	}
	if (nn_info == 0 || nn_info == NULL ){
		DBG_ERR("nn_info is null ...\r\n");
		return E_SYS ;
	}  

	nn_info->slice_max_out_height = kflow_isp_nn_info[proc_id].slice_max_out_height; 
	nn_info->slice_overlap = kflow_isp_nn_info[proc_id].slice_overlap; 
	nn_info->max_stripe = kflow_isp_nn_info[proc_id].max_stripe;
	nn_info->min_stripe = kflow_isp_nn_info[proc_id].min_stripe;
	nn_info->sw_handshake_en =  kflow_isp_nn_info[proc_id].sw_handshake_en ; 

	return E_OK ; 
}	
ER kflow_isp_set_ubuf_layout (UINT32 proc_id)
{
	NN_ISP_PATH_BUF_INFO buf_info = {0} ;
	AI_JMISP_INFO* jmisp_info;
	ER er = E_OK ;

	jmisp_info = (AI_JMISP_INFO*) g_ai_isp_parm[proc_id].jmisp_info; 
	
	if(jmisp_info == NULL ){
		DBG_ERR("proc_id(%u) is not opened !\r\n", proc_id);
		return E_SYS;
	}

	buf_info.frame_y_addr = jmisp_info->iobuf[e_AI_JMISP_IOBUF_RING_IN0].pa;
	buf_info.frame_uv_addr = jmisp_info->iobuf[e_AI_JMISP_IOBUF_RING_IN1].pa;
	buf_info.frame_texture_addr = 0;
	buf_info.pingpong_y_addr = jmisp_info->iobuf[e_AI_JMISP_IOBUF_PINGPONG_OUT0].pa;
	buf_info.pingpong_uv_addr = jmisp_info->iobuf[e_AI_JMISP_IOBUF_PINGPONG_OUT1].pa;
	buf_info.pingpong_texture_addr = jmisp_info->iobuf[e_AI_JMISP_IOBUF_PINGPONG_OUT2].pa;
	buf_info.ring_y_start_addr = jmisp_info->iobuf[e_AI_JMISP_IOBUF_RING_IN0].pa;
	buf_info.ring_y_end_addr = jmisp_info->iobuf[e_AI_JMISP_IOBUF_RING_IN0].pa + jmisp_info->iobuf[e_AI_JMISP_IOBUF_RING_IN0].buffer_sz;
	buf_info.ring_uv_start_addr = jmisp_info->iobuf[e_AI_JMISP_IOBUF_RING_IN1].pa;
	buf_info.ring_uv_end_addr = jmisp_info->iobuf[e_AI_JMISP_IOBUF_RING_IN1].pa + jmisp_info->iobuf[e_AI_JMISP_IOBUF_RING_IN1].buffer_sz;
	kflow_isp_ubuf_layout[proc_id] = buf_info;
	kflow_isp_net_trace(proc_id, AIISP_BUF, "set() - set_ubuf_layout()");
    kflow_isp_net_trace(proc_id, AIISP_BUF, "set() - frame_y_addr       %lx \n", (ULONG)buf_info.frame_y_addr);
	kflow_isp_net_trace(proc_id, AIISP_BUF, "set() - frame_uv_addr      %lx \n", (ULONG)buf_info.frame_uv_addr);
	kflow_isp_net_trace(proc_id, AIISP_BUF, "set() - frame_texture_addr %lx \n", (ULONG)buf_info.frame_texture_addr);
	kflow_isp_net_trace(proc_id, AIISP_BUF, "set() - pingpong_y_addr    %lx \n", (ULONG)buf_info.pingpong_y_addr);
	kflow_isp_net_trace(proc_id, AIISP_BUF, "set() - pingpong_uv_addr   %lx \n", (ULONG)buf_info.pingpong_uv_addr);
	kflow_isp_net_trace(proc_id, AIISP_BUF, "set() - pingpong_texture_addr %lx \n", (ULONG)buf_info.pingpong_texture_addr);
	kflow_isp_net_trace(proc_id, AIISP_BUF, "set() - ring_y_start_addr  %lx \n", (ULONG)buf_info.ring_y_start_addr);
	kflow_isp_net_trace(proc_id, AIISP_BUF, "set() - ring_y_end_addr    %lx \n", (ULONG)buf_info.ring_y_end_addr);\
	kflow_isp_net_trace(proc_id, AIISP_BUF, "set() - ring_uv_start_addr %lx \n", (ULONG)buf_info.ring_uv_start_addr);
	kflow_isp_net_trace(proc_id, AIISP_BUF, "set() - ring_uv_end_addr   %lx \n", (ULONG)buf_info.ring_uv_end_addr);

	return er;

}	
ER kflow_isp_get_ubuf_layout (NN_ISP_IN_PARAM* in_info, NN_ISP_PATH_BUF_INFO* buf_info)
{
	ER er = E_OK ;
	UINT32 proc_id = in_info->proc_id ;

	
	if (in_info == 0 || in_info == NULL ){
		DBG_ERR("in_info is null ...\r\n");
		return E_SYS ;
	}
	if (buf_info == 0 || buf_info == NULL ){
		DBG_ERR("buf_info is null ...\r\n");
		return E_SYS ;
	}  
	
	memcpy(buf_info, &kflow_isp_ubuf_layout[proc_id], sizeof(NN_ISP_PATH_BUF_INFO));
	kflow_isp_net_trace(proc_id, AIISP_BUF, "get() - get_ubuf_layout()");
	kflow_isp_net_trace(proc_id, AIISP_BUF, "get() - frame_y_addr       %lx \n", (ULONG)buf_info-> frame_y_addr);
	kflow_isp_net_trace(proc_id, AIISP_BUF, "get() - frame_uv_addr      %lx \n", (ULONG)buf_info-> frame_uv_addr);
	kflow_isp_net_trace(proc_id, AIISP_BUF, "get() - frame_texture_addr %lx \n", (ULONG)buf_info-> frame_texture_addr);
	kflow_isp_net_trace(proc_id, AIISP_BUF, "get() - pingpong_y_addr    %lx \n", (ULONG)buf_info-> pingpong_y_addr);
	kflow_isp_net_trace(proc_id, AIISP_BUF, "get() - pingpong_uv_addr   %lx \n", (ULONG)buf_info-> pingpong_uv_addr);
	kflow_isp_net_trace(proc_id, AIISP_BUF, "get() - pingpong_texture_addr %lx \n", (ULONG)buf_info-> pingpong_texture_addr);
	kflow_isp_net_trace(proc_id, AIISP_BUF, "get() - ring_y_start_addr  %lx \n", (ULONG)buf_info-> ring_y_start_addr);
	kflow_isp_net_trace(proc_id, AIISP_BUF, "get() - ring_y_end_addr    %lx \n", (ULONG)buf_info-> ring_y_end_addr);\
	kflow_isp_net_trace(proc_id, AIISP_BUF, "get() - ring_uv_start_addr %lx \n", (ULONG)buf_info-> ring_uv_start_addr);
	kflow_isp_net_trace(proc_id, AIISP_BUF, "get() - ring_uv_end_addr   %lx \n", (ULONG)buf_info-> ring_uv_end_addr);

	return er;

}	
static void  kflow_isp_push_nn_job (void *p_param)
{
	ER er = E_OK ;
	uintptr_t jmisp_pl_pa = 0 ; 
	KFLOW_AI_ISP_JOB *isp_job = (KFLOW_AI_ISP_JOB *) p_param ; 
	UINT32 proc_id = isp_job->proc_id ;
	INT32 pool_id = kflow_isp_proc_pool_map[proc_id], jmisp_id ;
	FLGPTN jmisp_flag = 0;
	FLGPTN wait_flag = 0 ;
	NN_ISP_SLICE_DONE_PARAM slice_done = {0} ;

	unsigned long flags;

	if(pool_id < 0 ){
		if (kflow_isp_get_st_in_core_mask(kflow_isp_proc_core_mask[proc_id], &jmisp_id) != E_OK){
			DBG_ERR("proc_id(%u) is not using any core_mask !\r\n", proc_id);
			return ;
		}
	}else{
		// check whether pool is valid 
		if (pool_id > 3){
			DBG_ERR("pool_%u is illegal ! \n", pool_id);
			return  ; 
		}

		jmisp_id = kflow_isp_pool[pool_id][3] ; 
	}

	if(jmisp_id > 3 || jmisp_id < 0){
		DBG_ERR("jmisp_id(%d)is not valid...\r\n", jmisp_id);
		return ;
	}

	kflow_isp_net_trace(proc_id, AIISP_FLOW, "push_job() - jmisp_id(%d) ipp_id(%u) path_id(%u) \r\n", jmisp_id, isp_job->ipp_id, isp_job->path_id );
	er = nvt_ai_lock_net(proc_id) ; 
	if (er != E_OK) {
		DBG_ERR("nvt_ai_lock_net fail...\r\n");
		return ;
	}
	er = kflow_isp_assign_all_input_addr(proc_id);	
	if (er != E_OK) {
		DBG_WRN("kflow_isp_assign_all_input_addr fail...\r\n");
		nvt_ai_unlock_net(proc_id);
		return ;
	}

	er =  kflow_isp_swap_ref(proc_id);
	if (er != E_OK) {
		DBG_ERR("kflow_isp_swap_ref fail...\r\n");
		nvt_ai_unlock_net(proc_id);
		return ;
	}
	
	jmisp_pl_pa = g_ai_isp_parm[proc_id].p_info.parm.user_parm.pa + (g_ai_isp_parm[proc_id].p_pl_cmd_list - g_ai_isp_parm[proc_id].p_info.parm.user_parm.va) ;

	DBG_MSG("jmisp_pl_pa %lx \n", (ULONG)jmisp_pl_pa) ; 
	er = kflow_isp_set_base_addr(proc_id) ; 
	if (er){
		DBG_ERR("proc_id %u kflow_isp_set_base_addr fail...\r\n", proc_id);
		nvt_ai_unlock_net(proc_id);
		return ;
	}

	jmisp_time = _nvt_ai_get_counter();
	slice_done.frm_start_time = jmisp_time;
#if ( defined(_BSP_NS02302_))	
	{
		UINT32 i ; 
		for (i=0 ; i < g_ai_isp_parm[proc_id].core_num ; i++){
			kdrv_ai_trigger(KDRV_AI_ENG_JMISP_PLA + i + jmisp_id, jmisp_pl_pa);
			wait_flag = wait_flag | FLGPTN_BIT(i + jmisp_id) ; 
		}
	}
#else
	switch (jmisp_id)
	{
		case 0:
			kdrv_ai_trigger(KDRV_AI_ENG_JMISP_PLA, jmisp_pl_pa);
			wait_flag = FLG_ID_JMISP_A ; 
			break;
		case 1:
			kdrv_ai_trigger(KDRV_AI_ENG_JMISP_PLB, jmisp_pl_pa);
			wait_flag = FLG_ID_JMISP_B ; 
			break;
		case 2:
			kdrv_ai_trigger(KDRV_AI_ENG_JMISP_PLC, jmisp_pl_pa);
			wait_flag = FLG_ID_JMISP_C ; 
			break;
		case 3:
			kdrv_ai_trigger(KDRV_AI_ENG_JMISP_PLD, jmisp_pl_pa);
			wait_flag = FLG_ID_JMISP_D ; 
			break;			
		
		default:
			DBG_ERR("unknown isp nn_path_id : %d\r\n", (int)isp_job->path_id);
			break;
	}
#endif
	wai_flg(&jmisp_flag, FLG_ID_JMISP, wait_flag, TWF_ANDW | TWF_CLR);

	// restore the enable_3d val after first proc is done
	if(kflow_isp_first_proc[proc_id] == 1 && kflow_isp_enable_3d[proc_id].isp_input_addr.va){
		memcpy((VOID *)kflow_isp_enable_3d[proc_id].isp_input_addr.va, (VOID *)&(kflow_isp_enable_3d[proc_id].enable_3d_val), sizeof(UINT8));
		vos_cpu_dcache_sync((kflow_isp_enable_3d[proc_id].isp_input_addr.va), (UINT32)(sizeof(UINT8)), VOS_DMA_TO_DEVICE);
		kflow_isp_net_trace(proc_id, AIISP_FLOW, "After proc set() enable_3d back to %u\r\n", (UINT8)kflow_isp_enable_3d[proc_id].enable_3d_val);
	}

	slice_done.frm_end_time = _nvt_ai_get_counter();
	kflow_isp_net_trace(proc_id, AIISP_PERF, "push_job() - done ts_exec=%llu \r\n", _nvt_ai_get_counter() - jmisp_time);


	// if dump 
    if( g_proc_trace[proc_id] & AIISP_DUMP){
        kflow_ai_isp_dump_jmisp_buf(proc_id) ;
        g_proc_trace[proc_id] &= (~AIISP_DUMP); 
    }

	if(_kflow_ai_callback){
		slice_done.frm_ed_flg = TRUE;
		_kflow_ai_callback(NN_ISP_AI_TO_IPP_OP_SLICE_DONE, &slice_done, NULL) ; 
	}
	er = nvt_ai_unlock_net(proc_id);
	if (er){
		DBG_ERR("nvt_ai_unlock_net fail...\r\n");
		return ;
	}
	loc_cpu(flags);
	stack_size -- ; 
	unl_cpu(flags);
	return ; 
	
}	
ER kflow_isp_set_cb(uintptr_t fp)
{
	_kflow_ai_callback =(KFLOW_AI_ISP_CB) fp ; 

	DBG_MSG("_kflow_ai_callback = %lx \r\n", (ULONG)_kflow_ai_callback) ; 
	return E_OK;

}

static void  kflow_isp_push_slice_job(void *p_param )
{
	NN_ISP_SLICE_DONE_PARAM slice_done = {0} ; 
	UINT32 frame_height, slice_height, slice_num, time_1 = 0, time_2 = 0, time_3, time_4  ;
	ER er = E_OK ;
	KFLOW_AI_ISP_JOB *isp_job = (KFLOW_AI_ISP_JOB *) p_param ; 
	UINT32 proc_id = isp_job->proc_id ;
	unsigned long flags;
	if (p_param == NULL) {
		return;
	}
	slice_done.ipp_id = isp_job->ipp_id ; 
	slice_done.path_id = isp_job->path_id ;

	frame_height = g_ai_jmisp_info[proc_id].iobuf[get_benchmark_iobuf_type()].frameInfo.frame_height;
	slice_height = g_ai_jmisp_info[proc_id].iobuf[get_benchmark_iobuf_type()].stripeInfo.output_slice_height; 
	slice_num = (frame_height / slice_height) + (((frame_height % slice_height) > 0) ?1 : 0) ; 
	
	kflow_isp_net_trace(proc_id, AIISP_FLOW, "push_job() - slice(%u/%u) ipp_id(%u) path_id(%u) \r\n",isp_job->push_time % slice_num, slice_num, isp_job->ipp_id, isp_job->path_id );
	if (isp_job->push_time % slice_num == 0){
		er = nvt_ai_lock_net(proc_id) ; 
		if (er != E_OK) {
			DBG_ERR("nvt_ai_lock_net fail...\r\n");
			return ;
		}
		time_1 =  _nvt_ai_get_counter() ;
		er = kflow_isp_assign_all_input_addr(proc_id);	
		if (er != E_OK) {
			DBG_WRN("kflow_isp_assign_all_input_addr fail...\r\n");
			nvt_ai_unlock_net(proc_id);
			return ;
		}
		time_1 = _nvt_ai_get_counter() - time_1 ; 
		time_2 =  _nvt_ai_get_counter() ; 
		er = kflow_isp_set_first_slice(proc_id);	
		if (er != E_OK) {
			DBG_ERR("kflow_isp_set_first_slice fail...\r\n");
			nvt_ai_unlock_net(proc_id);
			return ;
		}
		time_2 = _nvt_ai_get_counter() - time_2 ; 	
	}
	time_3 =  _nvt_ai_get_counter() ; 
	er = kflow_isp_push_slice(proc_id, isp_job->push_time % slice_num); 
	if (er != E_OK){
		DBG_ERR("kflow_isp_push_slice fail...\r\n");
		nvt_ai_unlock_net(proc_id);
		return ;
	}
	time_3 = _nvt_ai_get_counter() - time_3 ; 
	time_4 = _nvt_ai_get_counter(); 
    if((isp_job->push_time % slice_num == slice_num -1) ){
		// restore the enable_3d val after first proc is done
		
		if(kflow_isp_first_proc[proc_id] == 1 && kflow_isp_enable_3d[proc_id].isp_input_addr.va){
			memcpy((VOID *)kflow_isp_enable_3d[proc_id].isp_input_addr.va, (VOID *)&(kflow_isp_enable_3d[proc_id].enable_3d_val), sizeof(UINT8));
			vos_cpu_dcache_sync((kflow_isp_enable_3d[proc_id].isp_input_addr.va), (UINT32)(sizeof(UINT8)), VOS_DMA_TO_DEVICE);
			kflow_isp_net_trace(proc_id, AIISP_FLOW, "After proc set() enable_3d back to %u\r\n", kflow_isp_enable_3d[proc_id].enable_3d_val);
		}
		// if dump 
		if (g_proc_trace[proc_id] & AIISP_DUMP){
			kflow_ai_isp_dump_jmisp_buf(proc_id) ;
			g_proc_trace[proc_id] &= (~AIISP_DUMP); 
		}
	}

	time_4 = _nvt_ai_get_counter() - time_4 ; 
	kflow_isp_net_trace(proc_id, AIISP_PERF, "push_job() - done slice(%u/%u)  input_update ts_exe(%u) first_slice ts_exe(%u) slice_job ts_exe(%u) post_process(%u)\r\n",isp_job->push_time % slice_num, slice_num, time_1, time_2, time_3,time_4);

	if(isp_job->push_time % slice_num == slice_num -1){
		slice_done.frm_ed_flg = TRUE ;
		slice_done.frm_start_time = isp_job->ts_exec ; // job ts_exec record the first slice job time
		slice_done.frm_end_time = _nvt_ai_get_counter() ;
		er = nvt_ai_unlock_net(proc_id);
		if (er){
			DBG_ERR("nvt_ai_unlock_net fail...\r\n");
			return ;
		}
	}

	if(_kflow_ai_callback){
		// if frame end
		_kflow_ai_callback(NN_ISP_AI_TO_IPP_OP_SLICE_DONE, &slice_done, NULL) ; 
		
		loc_cpu(flags);
		stack_size -- ; 
		unl_cpu(flags);
	}else {
		DBG_ERR("KFLOW_AI_ISP_CB is not set !\r\n");
	}

	return ;

}

ER kflow_isp_modify_gain(UINT32 proc_id, AI_JMISP_INFO *jmisp_info, UINT8 dgain)
{
	#define QUAN_DEBUG  0 // switch for debug

	#if QUAN_DEBUG
	#define DBG_QUAN_PARAM(str, p_param) DBG_DUMP("==> %s mctrl(%u) quan_out(dir,shift,scale)= [0](%u,%u,%u) [1](%u,%u,%u) [2](%u,%u,%u)\r\n", str, (p_param)->mctrl_id, (p_param)->quan_out[0].dir, (p_param)->quan_out[0].shift, (p_param)->quan_out[0].scale, (p_param)->quan_out[1].dir, (p_param)->quan_out[1].shift, (p_param)->quan_out[1].scale, (p_param)->quan_out[2].dir, (p_param)->quan_out[2].shift, (p_param)->quan_out[2].scale);
	#endif

	ER er = E_OK;

	// new dgain = previous dgain, don't have to update
	if (g_ai_isp_pre_dgain[proc_id] == dgain) {
		return E_OK;
	}

	// cal & set CURR ppu quan param
	if (jmisp_info->iobuf[e_AI_JMISP_IOBUF_RING_IN0].buffer_sz > 0) {
		AI_TUNING_QUAN_PARAM new_curr_param  = {0};
		AI_TUNING_QUAN_PARAM *p_ori_curr = &g_ai_isp_ori_ppu_quan_param[proc_id][0]; // [0] for curr
		AI_FLOAT_FORMAT mod_gain = {4, 4, dgain};

		er = kflow_isp_cal_gain(p_ori_curr, &new_curr_param, mod_gain);
		if (E_OK != er) { DBG_ERR("cal_gain(curr) failed...\r\n"); return er; }
		er = kflow_isp_set_ppu_quan_out_scale_shift(proc_id, &new_curr_param);
		if (E_OK != er) { DBG_ERR("set quan_out(curr) failed...\r\n"); return er; }
		#if QUAN_DEBUG
		{
			AI_TUNING_QUAN_PARAM after_set_param = {0};
			after_set_param.mctrl_id = p_ori_curr->mctrl_id;
			kflow_isp_get_ppu_quan_out_scale_shift(proc_id, &after_set_param);
			DBG_QUAN_PARAM("[ori][curr]", p_ori_curr);
			DBG_QUAN_PARAM("[new][curr]", &new_curr_param);
			DBG_QUAN_PARAM("[get][curr]", &after_set_param);
		}
		#endif
	}
	// cal & set SIGMA ppu quan param (note: sigma should be square of dgain, so call cal_gain 2 times.)
	if (jmisp_info->iobuf[e_AI_JMISP_IOBUF_SIGMA_IN].buffer_sz > 0) {
		AI_TUNING_QUAN_PARAM tmp_sigma_param  = {0};
		AI_TUNING_QUAN_PARAM new_sigma_param  = {0};
		AI_TUNING_QUAN_PARAM *p_ori_sigma = &g_ai_isp_ori_ppu_quan_param[proc_id][1]; // [0] for sigma
		AI_FLOAT_FORMAT mod_gain = {4, 4, dgain};

		er = kflow_isp_cal_gain(p_ori_sigma, &tmp_sigma_param, mod_gain);
		if (E_OK != er) { DBG_ERR("cal_gain(sigma) failed...\r\n"); return er; }
		er = kflow_isp_cal_gain(&tmp_sigma_param, &new_sigma_param, mod_gain); // call 2 times for dgain^2
		if (E_OK != er) { DBG_ERR("cal_gain (sigma) failed...\r\n"); return er; }
		er = kflow_isp_set_ppu_quan_out_scale_shift(proc_id, &new_sigma_param);
		if (E_OK != er) { DBG_ERR("set quan_out(sigma) failed...\r\n"); return er; }
		#if QUAN_DEBUG
		{
			AI_TUNING_QUAN_PARAM after_set_param = {0};
			after_set_param.mctrl_id = p_ori_sigma->mctrl_id;
			kflow_isp_get_ppu_quan_out_scale_shift(proc_id, &after_set_param);
			DBG_QUAN_PARAM("[ori][sigma]", p_ori_sigma);
			DBG_QUAN_PARAM("[new][sigma]", &new_sigma_param);
			DBG_QUAN_PARAM("[get][sigma]", &after_set_param);
		}
		#endif
	}

	g_ai_isp_pre_dgain[proc_id] = dgain;
	return E_OK;
}

ER kflow_isp_cb_exe (NN_ISP_OPERATION op, void* in, void* out)
{

	ER er = E_OK ;
	
	switch (op) {
		case NN_ISP_OP_QUERY_AI_STATUS:
			{
				NN_ISP_IN_PARAM* in_info = (NN_ISP_IN_PARAM *) in ; 
				if (in  == NULL) { DBG_ERR("op(NN_ISP_OP_QUERY_AI_STATUS),  in = null?\r\n"); return E_SYS; }

				if (g_ai_isp_parm == 0){
					DBG_WRN(" AI is not init yet  \n");
					return -2;
				}
				if ( !nvt_ai_check_proc_id(in_info->proc_id)){
					DBG_ERR(" proc_id[%u] is not opened  \n", in_info->proc_id);
					return -1;
				}
				er = nvt_ai_lock_net(in_info->proc_id) ; 
				if (er != E_OK) {
					DBG_ERR("nvt_ai_lock_net fail...\r\n");
					return er;
				}
				if (g_ai_isp_parm[in_info->proc_id].jmisp_info == 0){
					DBG_WRN(" proc_id[%u] is not started  \n", in_info->proc_id);
					nvt_ai_unlock_net(in_info->proc_id);
					return -2;
				}
				er = nvt_ai_unlock_net(in_info->proc_id);
				if (er){
					DBG_ERR("nvt_ai_unlock_net fail...\r\n");
					return er;
				}
			}
			break;

		case NN_ISP_OP_GET_NN_INFO:
			{
				NN_ISP_IN_PARAM* in_info = (NN_ISP_IN_PARAM *) in ; 
				NN_ISP_NN_INFO* nn_info = (NN_ISP_NN_INFO *) out ; 
				if (in  == NULL) { DBG_ERR("op(NN_ISP_OP_GET_NN_INFO),  in = null?\r\n"); return E_SYS; }
				if (out == NULL) { DBG_ERR("op(NN_ISP_OP_GET_NN_INFO), out = null?\r\n"); return E_SYS; }

				if (g_ai_isp_parm == 0){
					DBG_WRN(" AI is not init yet  \n");
					return -2;
				}
				er = nvt_ai_lock_net(in_info->proc_id) ; 
				if (er != E_OK) {
					DBG_ERR("nvt_ai_lock_net fail...\r\n");
					return er;
				}
				if (g_ai_isp_parm[in_info->proc_id].jmisp_info == 0){
					DBG_WRN(" proc_id[%u] is not started  \n", in_info->proc_id);
					nvt_ai_unlock_net(in_info->proc_id);
					return -2;
				}
				kflow_isp_net_trace(in_info->proc_id, AIISP_FLOW, "get() - get_nn_info() ipp_id(%u) path_id(%u) \r\n", in_info->ipp_id, in_info->path_id);

				er = kflow_isp_get_nn_info(in_info, nn_info) ;
				if (er){
					DBG_ERR("kflow_isp_get_nn_info fail...\r\n");
					nvt_ai_unlock_net(in_info->proc_id);
					return er;
				}
				kflow_isp_net_trace(in_info->proc_id, AIISP_FLOW, "get() - get_nn_info() slice_max_out_height(%u) slice_overlap(%u) max_stripe(%u) min_stripe(%u) sw_handshake_en(%u) \r\n", nn_info->slice_max_out_height, nn_info->slice_overlap, nn_info->max_stripe, nn_info->min_stripe, nn_info->sw_handshake_en );
				er = nvt_ai_unlock_net(in_info->proc_id);
				if (er){
					DBG_ERR("nvt_ai_unlock_net fail...\r\n");
					return er;
				}
			}
			break;
		case NN_ISP_OP_GET_UBUF_LAYOUT:
			{	
				NN_ISP_IN_PARAM* in_info = (NN_ISP_IN_PARAM *) in ; 
				NN_ISP_PATH_BUF_INFO* buf_info = (NN_ISP_PATH_BUF_INFO *) out ; 
				if (in  == NULL) { DBG_ERR("op(NN_ISP_OP_GET_UBUF_LAYOUT),  in = null?\r\n"); return E_SYS; }
				if (out == NULL) { DBG_ERR("op(NN_ISP_OP_GET_UBUF_LAYOUT), out = null?\r\n"); return E_SYS; }

				if (g_ai_isp_parm == 0){
					DBG_WRN(" AI is not init yet  \n");
					return -2;
				}
				er = nvt_ai_lock_net(in_info->proc_id) ; 
				if (er != E_OK) {
					DBG_ERR("nvt_ai_lock_net fail...\r\n");
					return er;
				}
				if (g_ai_isp_parm[in_info->proc_id].jmisp_info == 0){
					DBG_WRN(" proc_id[%u] is not started  \n", in_info->proc_id);
					nvt_ai_unlock_net(in_info->proc_id);
					return -2;
				}
				kflow_isp_net_trace(in_info->proc_id, AIISP_FLOW, "get() - get_ubuf_layout() ipp_id(%u) path_id(%u) \r\n", in_info->ipp_id, in_info->path_id);
				
				er = kflow_isp_get_ubuf_layout (in_info,  buf_info);
				if (er){
					DBG_ERR("kflow_isp_get_ubuf_layout fail...\r\n");
					nvt_ai_unlock_net(in_info->proc_id);
					return er;
				}
				er = nvt_ai_unlock_net(in_info->proc_id);
				if (er){
					DBG_ERR("nvt_ai_unlock_net fail...\r\n");
					return er;
				}

			}
			break;
		case NN_ISP_OP_GET_MAX_STRP:
			{
				NN_ISP_IN_PARAM* in_info = (NN_ISP_IN_PARAM *) in ; 
				NN_ISP_NN_STRP*  nn_strp = (NN_ISP_NN_STRP *) out ; 
				if (in  == NULL) { DBG_ERR("op(NN_ISP_OP_GET_MAX_STRP),  in = null?\r\n"); return E_SYS; }
				if (out == NULL) { DBG_ERR("op(NN_ISP_OP_GET_MAX_STRP), out = null?\r\n"); return E_SYS; }

				if (g_ai_isp_parm == 0){
					DBG_WRN(" AI is not init yet  \n");
					return -2;
				}
				er = nvt_ai_lock_net(in_info->proc_id) ; 
				if (er != E_OK) {
					DBG_ERR("nvt_ai_lock_net fail...\r\n");
					return er;
				}
				if (g_ai_isp_parm[in_info->proc_id].jmisp_info == 0){
					DBG_WRN(" proc_id[%u] is not started  \n", in_info->proc_id);
					nvt_ai_unlock_net(in_info->proc_id);
					return -2;
				}

				er = kflow_isp_set_id(in_info) ;
				if (er){
					DBG_ERR("kflow_isp_set_id fail...\r\n");
					nvt_ai_unlock_net(in_info->proc_id);
					return er;
				}
				
				er = kflow_isp_get_max_strp(in_info->proc_id, nn_strp);
				if (er){
					DBG_ERR("kflow_isp_get_max_strp fail...\r\n");
					nvt_ai_unlock_net(in_info->proc_id);
					return er;
				}
				kflow_isp_net_trace(in_info->proc_id, AIISP_FLOW, "get() - get_max_strp() ipp_id(%u) path_id(%u) max_stripe(%u) min_stripe(%u)\r\n", in_info->ipp_id, in_info->path_id, nn_strp->max_stripe, nn_strp->min_stripe);
				er = kflow_isp_set_ubuf_layout (in_info->proc_id);
				if (er){
					DBG_ERR("kflow_isp_set_ubuf_layout fail...\r\n");
					nvt_ai_unlock_net(in_info->proc_id);
					return er;
				}
				er = nvt_ai_unlock_net(in_info->proc_id);
				if (er){
					DBG_ERR("nvt_ai_unlock_net fail...\r\n");
					return er;
				}

			}
			break;
		case NN_ISP_OP_CAL_SLICE:
			{ 
				NN_ISP_SET_RES_PARAM* in_info = (NN_ISP_SET_RES_PARAM *) in ; 
				NN_ISP_NN_INFO* nn_info = (NN_ISP_NN_INFO *) out ; 
				unsigned long flags;
				if (in  == NULL) { DBG_ERR("op(NN_ISP_OP_CAL_SLICE),  in = null?\r\n"); return E_SYS; }
				if (out == NULL) { DBG_ERR("op(NN_ISP_OP_CAL_SLICE), out = null?\r\n"); return E_SYS; }

				if (g_ai_isp_parm == 0){
					DBG_WRN(" AI is not init yet  \n");
					return -2;
				}
				er = nvt_ai_lock_net(in_info->proc_id) ; 
				if (er != E_OK) {
					DBG_ERR("nvt_ai_lock_net fail...\r\n");
					return er;
				}
				if (g_ai_isp_parm[in_info->proc_id].jmisp_info == 0){
					DBG_WRN(" proc_id[%u] is not started  \n", in_info->proc_id);
					nvt_ai_unlock_net(in_info->proc_id);
					return -2;
				}
				kflow_isp_net_trace(in_info->proc_id, AIISP_FLOW, "cal_slice() - ipp_id(%u) path_id(%u) width(%u) height(%u)\r\n", in_info->ipp_id, in_info->path_id, in_info->width, in_info->height);

				kflow_isp_copy_weight(in_info);

				er = kflow_isp_set_nn_info(in_info, nn_info) ;
				if (er){
					DBG_ERR("kflow_isp_set_nn_info fail...\r\n");
					nvt_ai_unlock_net(in_info->proc_id);
					return er;
				}

				kflow_isp_push_time[in_info->proc_id] = 0 ;
				kflow_isp_first_proc[in_info->proc_id] = 0 ; // reset in IPP START (for iso change or res change)
				kflow_isp_net_trace(in_info->proc_id, AIISP_FLOW, "reset() - kflow_isp_first_proc[%u] %u \r\n", in_info->proc_id, kflow_isp_first_proc[in_info->proc_id] );
				er = nvt_ai_unlock_net(in_info->proc_id);
				if (er){
					DBG_ERR("nvt_ai_unlock_net fail...\r\n");
					return er;
				}
				loc_cpu(flags);
				job_cnt = 0 ; 
				stack_size = 0  ; 
				unl_cpu(flags);
			}
			break;
		case NN_ISP_OP_CHANGE_RES:
			{
				// NN_ISP_IN_PARAM* in_info = (NN_ISP_IN_PARAM *) in ; 
				// NN_ISP_NN_INFO* nn_info = (NN_ISP_NN_INFO *) out ; 
				//if (in  == NULL) { DBG_ERR("op(NN_ISP_OP_CHANGE_RES),  in = null?\r\n"); return E_SYS; }
				//if (out == NULL) { DBG_ERR("op(NN_ISP_OP_CHANGE_RES), out = null?\r\n"); return E_SYS; }

				DBG_MSG("--------------NN_ISP_OP_CHANGE_RES -----------------\n");
				// DBG_DUMP("width = %u, height = %u, proc_id = %u,  ipp_id=%u, path_id=%u \n",in_info->width, in_info->height, in_info->proc_id, in_info->ipp_id, in_info->path_id);


			}
			break; 	
		case NN_ISP_OP_FRAME_MODE:
			{
				UINT32* FRAME_MODE = (UINT32 *) out ; 
				NN_ISP_FRAME_MODE_INFO* in_info = (NN_ISP_FRAME_MODE_INFO *) in ;
				AI_JMISP_INFO * jmisp_info = NULL ; 
				if (out == NULL) { DBG_ERR("op(NN_ISP_OP_FRAME_MODE), out = null?\r\n"); return E_SYS; }
				if (in == NULL){
					*FRAME_MODE = g_isp_trig_mode ; 
					DBG_MSG("--------------NN_ISP_OP_FRAME_MODE : g_isp_trig_mode %u -----------------\n", g_isp_trig_mode);
				}else{
					if (g_ai_isp_parm == 0){
						DBG_WRN(" AI is not init yet  \n");
						return -2;
					}
					er = nvt_ai_lock_net(in_info->proc_id) ; 
					if (er != E_OK) {
						DBG_ERR("nvt_ai_lock_net fail...\r\n");
						return er;
					}
					jmisp_info = (AI_JMISP_INFO *)g_ai_isp_parm[in_info->proc_id].jmisp_info ;
					if (jmisp_info == NULL){
						DBG_WRN(" proc_id[%u] is not started  \n", in_info->proc_id);
						nvt_ai_unlock_net(in_info->proc_id);
						return -2;
					}
					if( jmisp_info->iobuf[e_AI_JMISP_IOBUF_RING_IN0].stripeInfo.output_slice_height == 
	    				jmisp_info->iobuf[e_AI_JMISP_IOBUF_RING_IN0].frameInfo.frame_height){
						*FRAME_MODE = 1 ;
					}else {
						*FRAME_MODE = 0 ;
					}
					kflow_isp_net_trace(in_info->proc_id, AIISP_FLOW, " get() - FRAME_MODE(%u)\r\n", *FRAME_MODE);
					er = nvt_ai_unlock_net(in_info->proc_id);
					if (er){
						DBG_ERR("nvt_ai_unlock_net fail...\r\n");
						return er;
					}
				}
				

			}
			break; 	

		case NN_ISP_OP_UPDATE:
			{
				NN_ISP_IN_PARAM* in_info = (NN_ISP_IN_PARAM *) in ; 
				AI_JMISP_INFO * jmisp_info = NULL;
				UINT32 i = 0 ; 
				if (in  == NULL) { DBG_ERR("op(NN_ISP_OP_UPDATE),  in = null?\r\n"); return E_SYS; }

				if (g_ai_isp_parm == 0){
					DBG_WRN(" AI is not init yet  \n");
					return -2;
				}
				er = nvt_ai_lock_net(in_info->proc_id) ; 
				if (er != E_OK) {
					DBG_ERR("nvt_ai_lock_net fail...\r\n");
					return er;
				}
				jmisp_info = (AI_JMISP_INFO *)g_ai_isp_parm[in_info->proc_id].jmisp_info;
				if (jmisp_info == NULL){
					DBG_WRN(" proc_id[%u] is not started  \n", in_info->proc_id);
					nvt_ai_unlock_net(in_info->proc_id);
					return -2;
				}
				if (in_info->en == false){
					kflow_isp_net_trace(in_info->proc_id, AIISP_FLOW, "close isp() - release isp_id(%d)\r\n", in_info->isp_id);
					kflow_isp_release_id(in_info) ; 
					kflow_isp_first_proc[in_info->proc_id] = 0 ;
					for (i= e_AI_JMISP_IOBUF_REF_IN0 ; i <= e_AI_JMISP_IOBUF_REF_OUT1; i++ ){
						if(jmisp_info->iobuf[i].va){
							memset((void*)jmisp_info->iobuf[i].va, 0x0, jmisp_info->iobuf[i].buffer_sz ) ; 
							vos_cpu_dcache_sync(jmisp_info->iobuf[i].va, jmisp_info->iobuf[i].buffer_sz, VOS_DMA_TO_DEVICE);
						}
					}
				}
				er = nvt_ai_unlock_net(in_info->proc_id);
				if (er){
					DBG_ERR("nvt_ai_unlock_net fail...\r\n");
					return er;
				}
			}
			break;		 		 	

		case NN_ISP_OP_SET_FUSION_WEIGHT:
			{
				NN_ISP_FRAME *input = (NN_ISP_FRAME *) in ;
				//NN_ISP_FRAME *output = (NN_ISP_FRAME *) out ;
				AI_JMISP_INFO *jmisp_info = NULL;
				if (in  == NULL) { DBG_ERR("op(NN_ISP_OP_SET_FUSION_WEIGHT),  in = null?\r\n"); return E_SYS; }
				//if (out == NULL) { DBG_ERR("op(NN_ISP_OP_SET_FUSION_WEIGHT), out = null?\r\n"); return E_SYS; }

				if (g_ai_jmisp_info == 0){
					DBG_WRN(" AI is not init yet  \n");
					return -2;
				}
				er = nvt_ai_lock_net(input->proc_id) ; 
				if (er != E_OK) {
					DBG_ERR("nvt_ai_lock_net fail...\r\n");
					return er;
				}
				jmisp_info = (AI_JMISP_INFO *)&g_ai_jmisp_info[input->proc_id];
				if (jmisp_info == NULL){
					DBG_WRN(" proc_id[%u] is not started  \n", input->proc_id);
					nvt_ai_unlock_net(input->proc_id);
					return -2;
				}
				DBG_MSG("--------------NN_ISP_OP_SET_FUSION_WEIGHT :proc_id=%u, input width/height/line_ofs/fmt = (%u/%u/%u/0x%08x), pa/va = (0x%016lx , 0x%016lx) -----------------\n",input->proc_id, input->width, input->height, input->line_ofs, input->fmt, input->pa, input->va);

				if (jmisp_info->iobuf[e_AI_JMISP_IOBUF_FUSION_WEIGHT_IN].buffer_sz > 0) {
					if (jmisp_info->iobuf[e_AI_JMISP_IOBUF_REF_IN0].buffer_sz > 0) {
						DBG_ERR("This aiisp model (REF_IN0.buffer_sz > 0) ... should NOT set FUSION_WEIGHT, something wrong !!\r\n");
						nvt_ai_unlock_net(input->proc_id);
						return E_NOSPT;
					}
					jmisp_info->iobuf[e_AI_JMISP_IOBUF_FUSION_WEIGHT_IN].pa = input->pa;
					jmisp_info->iobuf[e_AI_JMISP_IOBUF_FUSION_WEIGHT_IN].va = input->va;
				}
				er = nvt_ai_unlock_net(input->proc_id);
				if (er){
					DBG_ERR("nvt_ai_unlock_net fail...\r\n");
					return er;
				}
			}
			break;

		case NN_ISP_OP_SET_SIGMA:
			{
				NN_ISP_FRAME *input = (NN_ISP_FRAME *) in ;
				//NN_ISP_FRAME *output = (NN_ISP_FRAME *) out ;
				AI_JMISP_INFO *jmisp_info = NULL;
				if (in  == NULL) { DBG_ERR("op(NN_ISP_OP_SET_SIGMA),  in = null?\r\n"); return E_SYS; }
				//if (out == NULL) { DBG_ERR("op(NN_ISP_OP_SET_SIGMA), out = null?\r\n"); return E_SYS; }

				if (g_ai_jmisp_info == 0){
					DBG_WRN(" AI is not init yet  \n");
					return -2;
				}
				er = nvt_ai_lock_net(input->proc_id) ; 
				if (er != E_OK) {
					DBG_ERR("nvt_ai_lock_net fail...\r\n");
					return er;
				}
				jmisp_info = (AI_JMISP_INFO *)&g_ai_jmisp_info[input->proc_id];
				if (jmisp_info == NULL){
					DBG_WRN(" proc_id[%u] is not started  \n", input->proc_id);
					nvt_ai_unlock_net(input->proc_id);
					return -2;
				}
				DBG_MSG("--------------NN_ISP_OP_SET_SIGMA :proc_id=%u, input width/height/line_ofs/fmt = (%u/%u/%u/0x%08x), pa/va = (0x%016lx , 0x%016lx) -----------------\n",input->proc_id, input->width, input->height, input->line_ofs, input->fmt, input->pa, input->va);

				if (jmisp_info->iobuf[e_AI_JMISP_IOBUF_SIGMA_IN].buffer_sz > 0) {
					if (jmisp_info->iobuf[e_AI_JMISP_IOBUF_REF_IN1].buffer_sz > 0) {
						DBG_ERR("This aiisp model (REF_IN1.buffer_sz > 0) ... should NOT set SIGMA, something wrong !!\r\n");
						nvt_ai_unlock_net(input->proc_id);
						return E_NOSPT;
					}
					jmisp_info->iobuf[e_AI_JMISP_IOBUF_SIGMA_IN].pa = input->pa;
					jmisp_info->iobuf[e_AI_JMISP_IOBUF_SIGMA_IN].va = input->va;
				}
				er = nvt_ai_unlock_net(input->proc_id);
				if (er){
					DBG_ERR("nvt_ai_unlock_net fail...\r\n");
					return er;
				}
			}
			break;

		case NN_ISP_OP_SET_GAMMA:
			{
				NN_ISP_FRAME *input = (NN_ISP_FRAME *) in ;
				NN_ISP_FRAME *output = (NN_ISP_FRAME *) out ;
				AI_JMISP_INFO *jmisp_info = NULL;
				if (in  == NULL) { DBG_ERR("op(NN_ISP_OP_SET_GAMMA),  in = null?\r\n"); return E_SYS; }

				if (g_ai_jmisp_info == 0){
					DBG_WRN(" AI is not init yet  \n");
					return -2;
				}
				er = nvt_ai_lock_net(input->proc_id) ; 
				if (er != E_OK) {
					DBG_ERR("nvt_ai_lock_net fail...\r\n");
					return er;
				}
				jmisp_info = (AI_JMISP_INFO *)&g_ai_jmisp_info[input->proc_id];
				if (jmisp_info == NULL){
					DBG_WRN(" proc_id[%u] is not started  \n", input->proc_id);
					nvt_ai_unlock_net(input->proc_id);
					return -2;
				}
				DBG_MSG("--------------NN_ISP_OP_SET_GAMMA(in) :proc_id=%u, input width/height/line_ofs/fmt = (%u/%u/%u/0x%08x), pa/va = (0x%016lx , 0x%016lx) -----------------\n",input->proc_id, input->width, input->height, input->line_ofs, input->fmt, input->pa, input->va);

				if (jmisp_info->iobuf[e_AI_JMISP_IOBUF_GAMMA_IN].buffer_sz > 0) {
					if (jmisp_info->iobuf[e_AI_JMISP_IOBUF_REF_OUT0].buffer_sz > 0) {
						DBG_ERR("This aiisp model (REF_OUT0.buffer_sz > 0) ... should NOT set GAMMA_IN, something wrong !!\r\n");
						nvt_ai_unlock_net(input->proc_id);
						return E_NOSPT;
					}
					jmisp_info->iobuf[e_AI_JMISP_IOBUF_GAMMA_IN].pa = input->pa;
					jmisp_info->iobuf[e_AI_JMISP_IOBUF_GAMMA_IN].va = input->va;
				}
				// if this is ai-aided model => set output->pa to jmisp_info->iobuf[GAMMA_OUT].pa
				if (jmisp_info->iobuf[e_AI_JMISP_IOBUF_GAMMA_OUT].buffer_sz > 0) {
					if (jmisp_info->iobuf[e_AI_JMISP_IOBUF_REF_OUT1].buffer_sz > 0) {
						DBG_ERR("This aiisp model (REF_OUT1.buffer_sz > 0) ... should NOT set GAMMA_OUT, something wrong !!\r\n");
						nvt_ai_unlock_net(input->proc_id);
						return E_NOSPT;
					}
					if (out == NULL) { DBG_ERR("op(NN_ISP_OP_SET_GAMMA), out = null?\r\n"); return E_SYS; }
					DBG_MSG("--------------NN_ISP_OP_SET_GAMMA(out) :proc_id=%u, output width/height/line_ofs/fmt = (%u/%u/%u/0x%08x), pa/va = (0x%016lx , 0x%016lx) -----------------\n",input->proc_id, output->width, output->height, output->line_ofs, output->fmt, output->pa, output->va);
					if (output == NULL) {
						DBG_ERR("This is ai-aided model, but IPP set GAMMA_OUT is NULL !!\r\n");
						nvt_ai_unlock_net(input->proc_id);
						return E_SYS;
					}
					if (output->pa == 0) {
						DBG_ERR("This is ai-aided model, but IPP set GAMMA_OUT pa = (0x%016lx) is invalid !!\r\n", output->pa);
						nvt_ai_unlock_net(input->proc_id);
						return E_SYS;
					}
					jmisp_info->iobuf[e_AI_JMISP_IOBUF_GAMMA_OUT].pa = output->pa;
					jmisp_info->iobuf[e_AI_JMISP_IOBUF_GAMMA_OUT].va = output->va;
				}
				er = nvt_ai_unlock_net(input->proc_id);
				if (er){
					DBG_ERR("nvt_ai_unlock_net fail...\r\n");
					return er;
				}
			}
			break;

		case NN_ISP_OP_GET_MOTION_TYPE:
			{
				NN_ISP_MOTION_TYPE_INFO *output = (NN_ISP_MOTION_TYPE_INFO *) out ;
				AI_JMISP_INFO *jmisp_info = NULL;
				UINT32 ratio=0;
				if (out == NULL) { DBG_ERR("op(NN_ISP_OP_GET_MOTION_TYPE), out = null?\r\n"); return E_SYS; }

				if (g_ai_isp_parm == 0){
					DBG_WRN(" AI is not init yet  \n");
					return -2;
				}
				er = nvt_ai_lock_net(output->proc_id) ; 
				if (er != E_OK) {
					DBG_ERR("nvt_ai_lock_net fail...\r\n");
					return er;
				}
				jmisp_info = (AI_JMISP_INFO *)g_ai_isp_parm[output->proc_id].jmisp_info;
				if (jmisp_info == NULL){
					DBG_ERR(" proc_id[%u] is not started  \n", output->proc_id);
					nvt_ai_unlock_net(output->proc_id);
					return -2;
				}
				DBG_MSG("--------------NN_ISP_OP_GET_MOTION_TYPE :proc_id=%u-----------------\n", output->proc_id);
				output->en = (jmisp_info->iobuf[e_AI_JMISP_IOBUF_GAMMA_OUT].buffer_sz > 0)? TRUE:FALSE;
				output->motion_type = 0;
				if (output->en) {
					ratio = (jmisp_info->iobuf[e_AI_JMISP_IOBUF_RING_IN0].frameInfo.frame_width  / jmisp_info->iobuf[e_AI_JMISP_IOBUF_GAMMA_OUT].frameInfo.frame_width) * \
					        (jmisp_info->iobuf[e_AI_JMISP_IOBUF_RING_IN0].frameInfo.frame_height / jmisp_info->iobuf[e_AI_JMISP_IOBUF_GAMMA_OUT].frameInfo.frame_height);
					switch (ratio) {
						case 64: output->motion_type = NN_ISP_MOTION_TYPE_64; break;
						case 16: output->motion_type = NN_ISP_MOTION_TYPE_16; break;
						case  4: output->motion_type = NN_ISP_MOTION_TYPE_4;  break;
						default:
							DBG_ERR("calculate motion_type failed!! model RING_IN0 w/h = (%d/%d) GAMMA_OUT0 w/h = (%d/%d), ratio = %d can't match any motion_type !!\r\n", jmisp_info->iobuf[e_AI_JMISP_IOBUF_RING_IN0].frameInfo.frame_width, jmisp_info->iobuf[e_AI_JMISP_IOBUF_RING_IN0].frameInfo.frame_height, jmisp_info->iobuf[e_AI_JMISP_IOBUF_GAMMA_OUT].frameInfo.frame_width, jmisp_info->iobuf[e_AI_JMISP_IOBUF_GAMMA_OUT].frameInfo.frame_height, ratio);
							nvt_ai_unlock_net(output->proc_id);
							return E_NOSPT;
					}
				}
				er = nvt_ai_unlock_net(output->proc_id);
				if (er){
					DBG_ERR("nvt_ai_unlock_net fail...\r\n");
					return er;
				}
				DBG_MSG("--------------NN_ISP_OP_GET_MOTION_TYPE :proc_id=%u, en(%d) motion_type(%d) <= GAMMA_OUT buffer_size(%d), model RING_IN0 w/h = (%d/%d) GAMMA_OUT0 w/h = (%d/%d)-----------------\n", output->proc_id, output->en, output->motion_type, jmisp_info->iobuf[e_AI_JMISP_IOBUF_GAMMA_OUT].buffer_sz, jmisp_info->iobuf[e_AI_JMISP_IOBUF_RING_IN0].frameInfo.frame_width, jmisp_info->iobuf[e_AI_JMISP_IOBUF_RING_IN0].frameInfo.frame_height, jmisp_info->iobuf[e_AI_JMISP_IOBUF_GAMMA_OUT].frameInfo.frame_width, jmisp_info->iobuf[e_AI_JMISP_IOBUF_GAMMA_OUT].frameInfo.frame_height );
			}
			break;

		case NN_ISP_OP_SET_ISP_PARAM:
			{
				NN_ISP_ISP_PARAM *input = (NN_ISP_ISP_PARAM *) in ;
				UINT32 input_num=0, i=0;
				VENDOR_AIS_ISP_INPUT_INFO *p_isp_input_info = 0;
				if (in  == NULL) { DBG_ERR("op(NN_ISP_OP_SET_ISP_PARAM),  in = null?\r\n"); return E_SYS; }

				DBG_MSG("--------------NN_ISP_OP_SET_ISP_PARAM : proc_id(%d) param_num(%d)-----------------\n", input->proc_id, (int)input->param_num);
				if (g_ai_input_layer_info_table == NULL || g_ai_isp_parm == NULL) {
					DBG_ERR("AI is not init yet, can't update ISP_PARAM !!\r\n");
					return E_SYS;
				}
				er = nvt_ai_lock_net(input->proc_id) ; 
				if (er != E_OK) {
					DBG_ERR("nvt_ai_lock_net fail...\r\n");
					return er;
				}
				input_num = g_ai_isp_parm[input->proc_id].input_num;
				if (input->param_num != input_num) {
					DBG_ERR("IPP param_num(%d) != model input_num(%d)\r\n", input->param_num, input_num);
					nvt_ai_unlock_net(input->proc_id);
					return E_SYS;
				}
				if (input_num == 0) {
					DBG_WRN("This model input_num=0, should NOT set isp param, skip set...\r\n");
					nvt_ai_unlock_net(input->proc_id);
					return E_OK;
				}
				if (g_ai_input_layer_info_table[input->proc_id][0].imem[0].size == 0) {  // check if first input param's size had been updated at start.
					DBG_ERR("AI is not start yet, can't update ISP_PARAM !!\r\n");
					nvt_ai_unlock_net(input->proc_id);
					return E_SYS;
				}
				if (g_ai_isp_parm[input->proc_id].jmisp_info == 0){
					DBG_WRN(" proc_id[%u] is not started  \n", input->proc_id);
					nvt_ai_unlock_net(input->proc_id);
					return -2;
				}

				for (i=0 ; i<input_num ; i++) {
					// get already saved input_info
					p_isp_input_info = &g_ai_input_layer_info_table[input->proc_id][i];

					// update addr from IPP
					p_isp_input_info->imem[0].pa = input->param_phyaddr[i];  //imem[0] is input.  imem[1] is name.

					if (p_isp_input_info->imem[0].size != input->param_size[i]) {
						DBG_ERR("check isp param[%d]'s size(%d) is not equal to model info size(%d)\r\n", (int)i, input->param_size[i], p_isp_input_info->imem[0].size);
						nvt_ai_unlock_net(input->proc_id);
						return E_SYS;
					}
					DBG_MSG("--------------NN_ISP_OP_SET_ISP_PARAM : proc_id(%d) param[%d] ipp_pa(0x%016lx) , AI=> mctrl(%d) port(%d) in_buff_ofs(%d) eng(%d) param_addr(0x%016lx)-----------------\n", input->proc_id, (int)i, input->param_phyaddr[i], (int)p_isp_input_info->mctrl_id, (int)p_isp_input_info->port_id, (int)p_isp_input_info->in_buff_ofs, (int)p_isp_input_info->eng_type, p_isp_input_info->parm_addr);

					// set input
					kflow_isp_set_input(p_isp_input_info);
				}
				er = nvt_ai_unlock_net(input->proc_id);
				if (er){
					DBG_ERR("nvt_ai_unlock_net fail...\r\n");
					return er;
				}
			}
			break;

		case NN_ISP_OP_CHK_PARAM_VALID:
			{
				NN_IPP_CHK_PARAM *p_ipp_chk_param = (NN_IPP_CHK_PARAM *) in;
				NN_AI_CHK_PARAM *p_ai_chk_param = (NN_AI_CHK_PARAM *)out;
				if (in  == NULL) { DBG_ERR("op(NN_ISP_OP_CHK_PARAM_VALID),  in = null?\r\n"); return E_SYS; }
				if (out == NULL) { DBG_ERR("op(NN_ISP_OP_CHK_PARAM_VALID), out = null?\r\n"); return E_SYS; }

				DBG_MSG("--------------NN_ISP_OP_CHK_PARAM_VALID : proc_id(%d) ipp_ver(%d)/sdk_ipp_max_ver(%d) ai_ver(%d)-----------------\r\n", p_ipp_chk_param->proc_id, p_ipp_chk_param->version, NN_CHK_IPP_VER, NN_CHK_AI_VER);

				if (p_ipp_chk_param->version > NN_CHK_IPP_VER) {
					DBG_WRN("IPP version(%u) > sdk support max(%u), newer element is NOT checked ...!\r\n", p_ipp_chk_param->version, NN_CHK_IPP_VER);
				}
				if (g_ai_isp_parm == 0){
					DBG_WRN(" AI is not init yet  \n");
					return -2;
				}
				er = nvt_ai_lock_net(p_ipp_chk_param->proc_id) ; 
				if (er != E_OK) {
					DBG_ERR("nvt_ai_lock_net fail...\r\n");
					return er;
				}
				//--- check IPP param ---
				if (p_ipp_chk_param->version >= 1) {
					// nothing to check for version 1
					// if any value is NOT expected, return E_SYS;
				}

				//--- fill AI param (for IPP to check later, let IPP know aisdk's AI_CHK_PARAM version) ---
				p_ai_chk_param->version = NN_CHK_AI_VER;
				//version 1 (fmt, but only fill bpp)
				{
					AI_JMISP_INFO *jmisp_info = (AI_JMISP_INFO *)g_ai_isp_parm[p_ipp_chk_param->proc_id].jmisp_info;
					if (jmisp_info == 0){
						DBG_WRN(" proc_id[%u] is not started  \n", p_ipp_chk_param->proc_id);
						nvt_ai_unlock_net(p_ipp_chk_param->proc_id);
						return -2;
					}
					p_ai_chk_param->fmt = ((jmisp_info->iobuf[e_AI_JMISP_IOBUF_RING_IN0].bitdepth) << 16);
					DBG_MSG("--------------NN_ISP_OP_CHK_PARAM_VALID : proc_id(%d) fmt(0x%08x)-----------------\r\n", p_ipp_chk_param->proc_id, p_ai_chk_param->fmt);
				}
				er = nvt_ai_unlock_net(p_ipp_chk_param->proc_id);
				if (er){
					DBG_ERR("nvt_ai_unlock_net fail...\r\n");
					return er;
				}
			}
			break;

		case NN_ISP_OP_PER_FRAME_PARAM:
			{
				NN_IPP_PER_FRAME_PARAM *p_in_param = (NN_IPP_PER_FRAME_PARAM *)in;
				AI_JMISP_INFO *jmisp_info = NULL;
				UINT8 dgain=0;
				if (in  == NULL) { DBG_ERR("op(NN_ISP_OP_PER_FRAME_PARAM),  in = null?\r\n"); return E_SYS; }
				if (p_in_param->data_va == 0) { DBG_ERR("op(NN_ISP_OP_PER_FRAME_PARAM),  in->data_va = 0 ?\r\n"); return E_SYS; }

				DBG_MSG("--------------NN_ISP_OP_PER_FRAME_PARAM : proc_id(%u) version(%u)-----------------\r\n", p_in_param->proc_id, p_in_param->version);
				if (g_ai_isp_parm == 0){
					DBG_WRN(" AI is not init yet  \n");
					return -2;
				}
				er = nvt_ai_lock_net(p_in_param->proc_id) ; 
				if (er != E_OK) {
					DBG_ERR("nvt_ai_lock_net fail...\r\n");
					return er;
				}
				if (p_in_param->version == 3) {
					NN_ISP_PER_FRAME_PARAM_DATA_V3 *p_data = (NN_ISP_PER_FRAME_PARAM_DATA_V3 *)p_in_param->data_va;
					if (p_in_param->data_size != sizeof(NN_ISP_PER_FRAME_PARAM_DATA_V3)) {
						DBG_ERR("op(NN_ISP_OP_PER_FRAME_PARAM), check ipp data_size(%u) != (%u)\r\n", p_in_param->data_size, (UINT32)sizeof(NN_ISP_PER_FRAME_PARAM_DATA_V3));
						nvt_ai_unlock_net(p_in_param->proc_id);
						return E_SYS;
					}
					DBG_MSG("--------------NN_ISP_OP_PER_FRAME_PARAM : V3 param => dgain(%u.%u)-----------------\r\n", (p_data->dgain >> 4), (p_data->dgain & 0xf)*625);
					dgain = p_data->dgain;
				} else {
					DBG_ERR("op(NN_ISP_OP_PER_FRAME_PARAM),  unsupported param version(%u)\r\n", p_in_param->version);
					nvt_ai_unlock_net(p_in_param->proc_id);
					return E_SYS;
				}

				jmisp_info = (AI_JMISP_INFO *)g_ai_isp_parm[p_in_param->proc_id].jmisp_info;
				if (jmisp_info == 0){
					DBG_WRN(" proc_id[%u] is not started  \n", p_in_param->proc_id);
					nvt_ai_unlock_net(p_in_param->proc_id);
					return -2;
				}
				// dgain
				if (p_in_param->version >= 3) {
					#if 1
					// check dgain valid
					if (dgain != 16 && dgain != 32 && dgain != 64) {
						DBG_ERR("dgain(0x%02x) value should be 1x/2x/4x !!\r\n", dgain);
						nvt_ai_unlock_net(p_in_param->proc_id);
						return E_SYS;
					}
					#endif
					SEM_WAIT(g_ai_isp_dgain_lock);
					er = kflow_isp_modify_gain(p_in_param->proc_id, jmisp_info, dgain);
					if (er){
						DBG_ERR("op(NN_ISP_OP_PER_FRAME_PARAM), dgain update fail...\r\n");
						SEM_SIGNAL(g_ai_isp_dgain_lock);
						nvt_ai_unlock_net(p_in_param->proc_id);
						return er;
					}
					SEM_SIGNAL(g_ai_isp_dgain_lock);
				}
				er = nvt_ai_unlock_net(p_in_param->proc_id);
				if (er){
					DBG_ERR("nvt_ai_unlock_net fail...\r\n");
					return er;
				}

			}
			break;

		case NN_ISP_OP_PUSH_NN_JOB:
			{
				NN_ISP_IPP_NN_JOB *job = (NN_ISP_IPP_NN_JOB *) in ; 
				KFLOW_AI_ISP_JOB *p_job ;
				unsigned long flags;
				UINT32 i ;
				if (in  == NULL) { DBG_ERR("op(NN_ISP_OP_PUSH_NN_JOB),  in = null?\r\n"); return E_SYS; }
				if (g_ai_jmisp_info == 0){
					DBG_WRN(" AI is not init yet  \n");
					return -2;
				}
				DBG_MSG("--------------NN_ISP_OP_PUSH_NN_JOB : ipp_id=%u, width =%u  height =%u -----------------\n",job->ipp_id, job->width, job->height);

				for (i=0; i < NN_ISP_NN_PATH_MAX; i++){
					if(job->nn_path[i].en){
						loc_cpu(flags);
						if (stack_size > 4 * 3 ){
						// job max trigger 4 proc_id * ipp frame buff 3 frame 
							unl_cpu(flags);
							kflow_isp_push_time[job->nn_path[i].proc_id] ++ ;
							DBG_WRN("NN_ISP_OP_PUSH_NN_JOB stack_size (%d) drop slice !\r\n", (int)stack_size);
							return E_OK;
						}
						job_cnt = job_cnt % KFLOW_AI_ISP_TSK_JOBSIZE ; 
						p_job = &(kflow_isp_job_statck[job_cnt]) ;
						p_job->proc_id = job->nn_path[i].proc_id ; 
						p_job->path_id = job->nn_path[i].nn_path_id ; 
						p_job->ipp_id = job->ipp_id ; 
						p_job->push_time = kflow_isp_push_time[job->nn_path[i].proc_id] ; 
						p_job->ts_exec = _nvt_ai_get_counter() ; 
						p_job->deferred.list.prev = 0;
						p_job->deferred.list.next = 0;
						p_job->deferred.p_param = 0;
						p_job->deferred.p_exec = 0;
						kflow_ai_isp_tsk_init_func(&(p_job->deferred), kflow_isp_push_nn_job, p_job);
						stack_size ++ ;
						job_cnt ++ ; 
						// if overflow 
						if (kflow_isp_push_time[job->nn_path[i].proc_id] + 1 <= kflow_isp_push_time[job->nn_path[i].proc_id]){
							kflow_isp_push_time[job->nn_path[i].proc_id] = 0 ;
						}
						kflow_isp_push_time[job->nn_path[i].proc_id] ++ ; 
						unl_cpu(flags);
						kflow_ai_isp_tsk_put_func(&(p_job->deferred));
					}
				}

			}
			break;
		
		case NN_ISP_OP_PUSH_SLICE_JOB:
			{
				NN_ISP_IPP_SLICE_JOB *input = (NN_ISP_IPP_SLICE_JOB *) in ;
				KFLOW_AI_ISP_JOB *p_job ;
				UINT32 frame_height = 0 , slice_height = 0, slice_num = 0 ;
				UINT32 frm_start_time = 0 ; 
				unsigned long flags;
				if (in  == NULL) { DBG_ERR("op(NN_ISP_OP_PUSH_SLICE_JOB),  in = null?\r\n"); return E_SYS; }
				if (g_ai_jmisp_info == 0){
					DBG_WRN(" AI is not init yet  \n");
					return -2;
				}
				DBG_MSG("--------------NN_ISP_OP_PUSH_SLICE_JOB :proc_id=%u, ipp_id=%u, path_id=%u input width =%u input height =%u -----------------\n",input->proc_id, input->ipp_id, input->path_id, input->width, input->height);
				loc_cpu(flags);
				if (g_ai_isp_parm[input->proc_id].jmisp_info == 0){
					DBG_WRN(" proc_id[%u] is not started  \n", input->proc_id);
					unl_cpu(flags);
					return -2;
				}
				frame_height = g_ai_jmisp_info[input->proc_id].iobuf[get_benchmark_iobuf_type()].frameInfo.frame_height;
				slice_height = g_ai_jmisp_info[input->proc_id].iobuf[get_benchmark_iobuf_type()].stripeInfo.output_slice_height; 
				slice_num = (frame_height / slice_height) + (((frame_height % slice_height) > 0) ?1 : 0) ; 
				
			
				job_cnt = job_cnt % KFLOW_AI_ISP_TSK_JOBSIZE ; 

				if (stack_size > slice_num * 3 ){
					// ipp frame buff = 3 frame 
					unl_cpu(flags);
					kflow_isp_push_time[input->proc_id] ++ ;
					DBG_WRN("NN_ISP_OP_PUSH_SLICE_JOB stack_size (%d) drop slice !\r\n", (int)stack_size);
					return E_OK;
				}
				if (kflow_isp_push_time[input->proc_id] % slice_num == 0){
					frm_start_time = _nvt_ai_get_counter();
				} 
				
				p_job = &(kflow_isp_job_statck[job_cnt]) ;
				p_job->proc_id = input->proc_id ; 
				p_job->path_id = input->path_id ; 
				p_job->ipp_id = input->ipp_id ; 
				p_job->push_time = kflow_isp_push_time[input->proc_id] ; 
				p_job->ts_exec = frm_start_time ;  // job ts_exec record the first slice job time => frm_start_time 
				p_job->deferred.list.prev = 0;
				p_job->deferred.list.next = 0;
				p_job->deferred.p_param = 0;
				p_job->deferred.p_exec = 0;
				kflow_ai_isp_tsk_init_func(&(p_job->deferred), kflow_isp_push_slice_job, p_job);
				stack_size ++ ;
				job_cnt ++ ; 
				// if overflow 
				if (kflow_isp_push_time[input->proc_id] + 1 <= kflow_isp_push_time[input->proc_id]){
					kflow_isp_push_time[input->proc_id] = kflow_isp_push_time[input->proc_id] % slice_num ;
				}
				kflow_isp_push_time[input->proc_id] ++ ; 
				unl_cpu(flags);
				kflow_ai_isp_tsk_put_func(&(p_job->deferred));
			}

			break;
		case NN_ISP_OP_PUSH_FRAME:
			{
				NN_ISP_FRAME *input = (NN_ISP_FRAME *) in ;
				NN_ISP_FRAME *output = (NN_ISP_FRAME *) out ;
				UINT32 time = 0 ; 
				if (in  == NULL) { DBG_ERR("op(NN_ISP_OP_PUSH_FRAME),  in = null?\r\n"); return E_SYS; }
				if (out == NULL) { DBG_ERR("op(NN_ISP_OP_PUSH_FRAME), out = null?\r\n"); return E_SYS; }
				if (g_ai_jmisp_info == 0){
					DBG_WRN(" AI is not init yet  \n");
					return -2;
				}
				kflow_isp_net_trace(input->proc_id, AIISP_FLOW, "push_frame() - start \r\n" );
   			 	time = _nvt_ai_get_counter();
				er = nvt_ai_lock_net(input->proc_id) ; 
				if (er != E_OK) {
					DBG_ERR("nvt_ai_lock_net fail...\r\n");
					return er;
				}
				er = kflow_isp_assign_all_input_addr(input->proc_id);	
				if (er != E_OK) {
					DBG_WRN("kflow_isp_assign_all_input_addr fail...\r\n");
					nvt_ai_unlock_net(input->proc_id);
					return er;
				}

				er = kflow_isp_push_frame(input, output); 
				if (er){
					DBG_ERR("kflow_isp_push_frame fail...\r\n");
					nvt_ai_unlock_net(input->proc_id);
					return er;
				}


				// restore the enable_3d val after first proc is done
				if(kflow_isp_first_proc[input->proc_id] == 1 && kflow_isp_enable_3d[input->proc_id].isp_input_addr.va){
					memcpy((VOID *)kflow_isp_enable_3d[input->proc_id].isp_input_addr.va, (VOID *)&(kflow_isp_enable_3d[input->proc_id].enable_3d_val), sizeof(UINT8));
					vos_cpu_dcache_sync((kflow_isp_enable_3d[input->proc_id].isp_input_addr.va), (UINT32)(sizeof(UINT8)), VOS_DMA_TO_DEVICE);
					kflow_isp_net_trace(input->proc_id, AIISP_FLOW, "After proc set() enable_3dpa(%#lx) back to %u  \r\n", kflow_isp_enable_3d[input->proc_id].isp_input_addr.pa, kflow_isp_enable_3d[input->proc_id].enable_3d_val);
				}
				
				// if dump 
				output->proc_time =  _nvt_ai_get_counter() - time ;

				if( g_proc_trace[input->proc_id] & AIISP_DUMP){
					kflow_ai_isp_dump_jmisp_buf(input->proc_id) ;
					g_proc_trace[input->proc_id] &= (~AIISP_DUMP); 
				}
				kflow_isp_net_trace(input->proc_id, AIISP_PERF, "push_frame() - push_frame time %u \r\n",  _nvt_ai_get_counter() - time );
    			kflow_isp_net_trace(input->proc_id, AIISP_FLOW, "push_frame() - end \r\n" );
				er = nvt_ai_unlock_net(input->proc_id);
				if (er){
					DBG_ERR("nvt_ai_unlock_net fail...\r\n");
					return er;
				}
			}
			break;	
		default:
			DBG_ERR("unsupport NN_ISP_OPERATION(%d) ...\r\n", (int)op);
			return -1;


	}

	return E_OK;

}

ER kflow_isp_cb (NN_ISP_OPERATION op, void* in, void* out)
{
	ER er = E_OK;

	if (g_ai_isp_parm == 0 || g_ai_jmisp_info == 0 || g_ai_input_layer_info_table == 0){
		DBG_WRN(" AI is not init yet  \n");
		return -2;
	}

	SEM_WAIT(g_kflow_ai_isp_init_uninit_sem_id);
	er = kflow_isp_cb_exe(op, in, out);
	SEM_SIGNAL(g_kflow_ai_isp_init_uninit_sem_id);

	return er;
}

#endif
