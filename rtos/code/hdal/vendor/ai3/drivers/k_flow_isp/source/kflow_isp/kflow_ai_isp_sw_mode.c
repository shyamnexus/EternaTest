/**
    @brief Source file of kflow_ai_net.

    @file kflow_ai_isp.c

    @ingroup kflow_ai_net

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
#define __CLASS__ 				"[ai][kflow_isp][sw_mode]"
#include "kflow_ai_isp_debug.h"
//=============================================================
#include "kflow_isp/kflow_isp_platform.h"
#include "kflow_isp/kflow_isp.h"
#include "kflow_isp/nn_isp.h"
#include "kflow_isp/kflow_isp_change_res.h"
#include "kflow_ai_isp_build_instrs_sync_sdktool.h"
#include "kflow_ai_net/kflow_ai_core.h"



#if (FLOW_AI_ISP == 1)
extern KDRV_AI_JMISP_PATH_INFO  **g_ai_isp_p_drv;
extern AI_JMISP_INFO *g_ai_jmisp_info ;
extern VENDOR_AIS_ISP_PARM * g_ai_isp_parm ;
extern UINT32  MAX_ISP_NUM  ;
extern UINT32 *kflow_isp_weight_loc_map ; 
extern INT32 kflow_isp_used_pool[4];
extern UINT32 kflow_isp_pool[4][5];
extern INT32 *kflow_isp_proc_pool_map;
extern UINT32 * kflow_isp_proc_core_mask;
JMISP_REG_INFO reg_info = {0};


ER kflow_isp_create_job(UINT32 proc_id, UINT32 num)
{
    KDRV_AI_JMISP_PATH_INFO* p_drv_array ; 
    KFLOW_AI_NET* p_net;
	KFLOW_AI_JOB* p_job;
    UINT32 i, time; 
    uintptr_t src = 0, dst = 0, null_pa_addr ; 
    time = _nvt_ai_get_counter();
    kflow_isp_net_trace(proc_id, AIISP_FLOW, "create_job(%u) - start \r\n", num);
    
    null_pa_addr = ispframemode_get_jmisp_null_pa_pos(g_ai_isp_parm[proc_id].p_pl_cmd_list, g_ai_isp_parm[proc_id].p_info.parm.user_parm.va, g_ai_isp_parm[proc_id].p_info.parm.user_parm.pa, g_ai_isp_parm[proc_id].jmisp_pl_size) ;
    p_drv_array = g_ai_isp_p_drv[proc_id] ; 

    p_net = kflow_ai_core_net(proc_id);
    if (p_net == NULL) {
        DBG_ERR("proc[%d], getting p_net err \r\n", proc_id);
        return E_CTX;
	}
    if (p_net->job) 
        kflow_ai_net_destory(p_net) ;
    kflow_ai_net_create(p_net, num , num , 0, 0);
    for (i = 0 ; i < num  ; i++){
        p_job = kflow_ai_net_add_job(p_net, i);
        if (p_job == NULL) {
			DBG_ERR("proc[%d] ADD_JOB, invalid job_id=%d?\r\n", proc_id, (int)i);
			return E_CTX ;
		}
        p_job->trig_src = 1 ; //NN_GEN_TRIG_LL_AI_DRV
        p_job->tot_trig_eng_times = 1 ; 
        p_job->engine_id = 7; //running this job by JOBM engine => KFLOW_AI_ENGINE_JOBM
        p_job->p_eng = kflow_ai_core_get_engine(p_job->engine_id);
        p_job->parm_addr = null_pa_addr ; 
        p_job->wait_ms =  0; 
        p_job->p_io_info = (void*)&p_drv_array[0];
        p_job->ts_exec_predict = 1 ; 
        p_job->engine_op |= 0x00008000 ;  //running this job by JOBM engine, and mark it with 0x00008000 for JMISP mode
    }

    kflow_ai_core_sum_job(p_net, &src, &dst);
    kflow_isp_net_trace(proc_id, AIISP_FLOW, "create_job() - end create_job time %u\r\n", _nvt_ai_get_counter() - time);
    return E_OK ; 
}
ER kflow_isp_push_job(KFLOW_AI_NET* p_net, UINT32 proc_id)
{
    KFLOW_AI_JOB* p_job , *dummy_begin;
    UINT32 i , core_mask = 0, usr_mask = kflow_isp_proc_core_mask[proc_id] ; 
    INT32 pool_id = kflow_isp_proc_pool_map[proc_id], st_id = -1 ; 
    ER er = E_OK ;
    UINT32 core_ch_st, core_ch_end; 

    if (pool_id < 0){
        for (i = 0 ; i <  g_ai_isp_parm[proc_id].core_num ; i++){
            if(kflow_isp_get_st_in_core_mask(usr_mask, &st_id) != E_OK){
                DBG_ERR("kflow_isp_get_st_in_core_mask fail proc_id(%u) core_mask is needed !\r\n", proc_id);
                return E_PAR;
            }
            if (st_id >= 0){
                core_mask |= (1L << (st_id)) ;
                usr_mask &= (~(1L << (st_id)));
            }else {
                DBG_ERR("kflow_isp_get_st_in_core_mask fail proc_id(%u) core_mask st_id(%d) is < 0 !\r\n", proc_id, st_id);
                return E_PAR;
            }
            
        }
    }else {
        core_ch_st = kflow_isp_pool[pool_id][3] ; 
        core_ch_end = kflow_isp_pool[pool_id][3] + g_ai_isp_parm[proc_id].core_num ; 
        for (i = core_ch_st  ; i < core_ch_end ; i ++){
            core_mask |= (1L << (i)) ;
        }
    }
    
    core_mask &= (0xff) ; //VENDOR_AI_DLA_CORE_ALL

    kflow_isp_net_trace(proc_id, AIISP_FLOW, "set() - core_mask 0x%02x\r\n", core_mask);
    dummy_begin = kflow_ai_net_job(p_net, 0);
     if (dummy_begin == NULL){
        return E_PAR;
    }
    er = kflow_ai_core_lock_job(p_net, dummy_begin) ;
    if (er){
		DBG_ERR("kflow_ai_core_lock_job fail...\r\n");
		return er;
	}

    for (i = 0 ; i < p_net->job_cnt ; i++){
        p_job = kflow_ai_net_job(p_net, i);
         if (p_job == NULL) {
            DBG_ERR("proc[%d], invalid job_id=%d?\r\n", proc_id, (int)i);
			return E_PAR ;
		}
        kflow_ai_core_push_job(p_net, p_job, core_mask, 0);
    }
    er = kflow_ai_core_unlock_job(p_net, p_job) ;
    if (er){
		DBG_ERR("kflow_ai_core_unlock_job fail...\r\n");
		return er;
	}

    return E_OK;
}
ER kflow_isp_pull_job(KFLOW_AI_NET* p_net)
{
    KFLOW_AI_JOB* pull_job;
	INT32 rv ; 
	pull_job = 0;

	rv = kflow_ai_core_pull_job(p_net, &pull_job);
    if (rv == 0) {
        // DBG_DUMP("proc[%d] PULL_JOB ok.\r\n", p_net->proc_id);
	}else if (rv == -2) {
		DBG_WRN("proc[%d] PULL_JOB get signal.\r\n", p_net->proc_id);
	}else if (rv == -3) {
		if (p_net->rv == -1) {
			DBG_ERR("proc[%d] PULL_JOB %u fail!\r\n", p_net->proc_id, pull_job->job_id);
		}
		if (p_net->rv == -2) {
			DBG_ERR("proc[%d] PULL_JOB %u timeout!\r\n", p_net->proc_id, pull_job->job_id);
		}
    }else {
        DBG_ERR("proc[%d] PULL_JOB %u  error!!!\r\n", p_net->proc_id, pull_job->job_id);
    }    

    return E_OK;
}

ER kflow_isp_get_slice_blk_num(JMISP_REG_INFO* p_reg_info, UINT32 push_time, UINT32* start_job, UINT32* end_job)
{
	UINT32 slice_y;
	if (p_reg_info == NULL) {
		return 0;
	}

    for (slice_y=0; slice_y < push_time; ++slice_y) {
        uint32_t blk_num = ispframemode_get_blk_num_in_a_slice(p_reg_info, slice_y);
        *start_job += blk_num;
    }
    *end_job = *start_job + ispframemode_get_blk_num_in_a_slice(p_reg_info, push_time);

	return E_OK;
}

ER kflow_isp_get_slice_substripe_num(JMISP_REG_INFO* p_reg_info, UINT32 push_time, UINT32* start_job, UINT32* end_job, UINT32 proc_id)
{
	UINT32 slice_y;
	if (p_reg_info == NULL) {
		return 0;
	}

    for (slice_y=0; slice_y < push_time; ++slice_y) {
        uint32_t blk_num = ispframemode_get_substripe_num_in_a_slice(p_reg_info, slice_y, g_ai_isp_parm[proc_id].core_num);
        *start_job += blk_num;
    }
    *end_job = *start_job + ispframemode_get_substripe_num_in_a_slice(p_reg_info, push_time, g_ai_isp_parm[proc_id].core_num);

	return E_OK;
}

ER kflow_isp_set_sw_mode_job (UINT32 proc_id)
{
    ER er = E_OK ;
    UINT32 num, job_num; 

    memset(&reg_info, 0x0, sizeof(JMISP_REG_INFO)) ;
    gen_jmisp_reg_para(&g_ai_jmisp_info[proc_id], &reg_info);

    if (g_ai_isp_parm[proc_id].pipe_mode.bit.sub_stripe_en) {
        num =  ispframemode_get_total_substripe_num (&reg_info, g_ai_isp_parm[proc_id].core_num);
        job_num = ispframemode_get_max_substripe_num_in_a_slice(&reg_info, g_ai_isp_parm[proc_id].core_num);
    } else {
        num =  ispframemode_get_total_blk_num (&reg_info);
        job_num = ispframemode_get_max_blk_num_in_a_slice(&reg_info);
    }
    if (g_ai_isp_p_drv[proc_id]) {
        nvt_ai_mem_free(g_ai_isp_p_drv[proc_id]);
        g_ai_isp_p_drv[proc_id] = 0;
    }
    g_ai_isp_p_drv[proc_id] = (KDRV_AI_JMISP_PATH_INFO *)nvt_ai_mem_alloc(sizeof(KDRV_AI_JMISP_PATH_INFO) * num);
	if (g_ai_isp_p_drv[proc_id] == NULL) {
		DBG_ERR("g_ai_isp_p_drv[proc_id] alloc fail\r\n");
		er = -1;
        return er;
	}
	memset(g_ai_isp_p_drv[proc_id], 0x0, sizeof(KDRV_AI_JMISP_PATH_INFO) * num);

    kflow_isp_create_job(proc_id, job_num);  
    return er;
}

ER kflow_isp_set_first_slice (UINT32 proc_id)
{
    UINT32 num = 0; 
    KDRV_AI_JMISP_PATH_INFO* p_drv_array; 
    ER er = E_OK ; 
	uintptr_t  temp_addr; 

    kflow_isp_net_trace(proc_id, AIISP_FLOW, "set() - set_first_slice \r\n" );

    kflow_isp_set_base_addr(proc_id) ; 

    if ((g_ai_jmisp_info[proc_id].iobuf[e_AI_JMISP_IOBUF_REF_IN0].buffer_sz > 0) && (g_ai_jmisp_info[proc_id].iobuf[e_AI_JMISP_IOBUF_REF_OUT0].buffer_sz > 0)) {
        temp_addr = g_ai_jmisp_info[proc_id].iobuf[e_AI_JMISP_IOBUF_REF_IN0].pa ; 
        g_ai_jmisp_info[proc_id].iobuf[e_AI_JMISP_IOBUF_REF_IN0].pa =  g_ai_jmisp_info[proc_id].iobuf[e_AI_JMISP_IOBUF_REF_OUT0].pa ; 
        g_ai_jmisp_info[proc_id].iobuf[e_AI_JMISP_IOBUF_REF_OUT0].pa = temp_addr ; 
    }

    if ((g_ai_jmisp_info[proc_id].iobuf[e_AI_JMISP_IOBUF_REF_IN1].buffer_sz > 0) && (g_ai_jmisp_info[proc_id].iobuf[e_AI_JMISP_IOBUF_REF_OUT1].buffer_sz > 0)) {
        temp_addr = g_ai_jmisp_info[proc_id].iobuf[e_AI_JMISP_IOBUF_REF_IN1].pa ; 
        g_ai_jmisp_info[proc_id].iobuf[e_AI_JMISP_IOBUF_REF_IN1].pa = g_ai_jmisp_info[proc_id].iobuf[e_AI_JMISP_IOBUF_REF_OUT1].pa ; 
        g_ai_jmisp_info[proc_id].iobuf[e_AI_JMISP_IOBUF_REF_OUT1].pa = temp_addr ;
    }

    memset(&reg_info, 0x0, sizeof(JMISP_REG_INFO)) ;
    gen_jmisp_reg_para(&g_ai_jmisp_info[proc_id], &reg_info);
    p_drv_array = g_ai_isp_p_drv[proc_id] ; 

    if (g_ai_isp_parm[proc_id].pipe_mode.bit.sub_stripe_en) {
        num =  ispframemode_get_total_substripe_num (&reg_info, g_ai_isp_parm[proc_id].core_num);
        er = ispframemode_create_substripe_array(&g_ai_jmisp_info[proc_id], p_drv_array, num, g_ai_isp_parm[proc_id].core_num);
        if (er){
            DBG_ERR("ispframemode_create_substripe_array fail...\r\n");
            return er;
        }
    } else {
        num =  ispframemode_get_total_blk_num (&reg_info) ;
        er = ispframemode_create_blkinfo_array(&g_ai_jmisp_info[proc_id], p_drv_array, num);
        if (er){
            DBG_ERR("ispframemode_create_blkinfo_array fail...\r\n");
            return er;
        }
    }
   
    // update pingpong out
    {
        int32_t i;
        for (i=0; i<=e_AI_IOBUF_NUM; ++i) {
            const e_AI_JMISP_IOBUF_CONST iobuf_type = (e_AI_JMISP_IOBUF_CONST)i;
            const e_AI_JMISP_DRV_IO_CONST drvio_type = convert_iobuf2drvio_type(&g_ai_jmisp_info[proc_id].iobuf[iobuf_type], iobuf_type);
            AI_JMISP_IOBUF_INFO iobuf = {0} ; 
            uint32_t max_stripe_sz, slice_buf_sz;
            if ((!is_pingpong_buf(&g_ai_jmisp_info[proc_id].iobuf[iobuf_type], iobuf_type))
                || (g_ai_jmisp_info[proc_id].iobuf[iobuf_type].buffer_sz <= 0)) {
                continue;
            }
            iobuf = g_ai_jmisp_info[proc_id].iobuf[iobuf_type] ; 
            max_stripe_sz = max(max(iobuf.stripeInfo.first_stripe_width, iobuf.stripeInfo.mid_stripe_width), iobuf.stripeInfo.last_stripe_width);
            slice_buf_sz = iobuf.stripeInfo.output_slice_height * max_stripe_sz * iobuf.bitdepth >> 3;
            if (g_ai_isp_parm[proc_id].pipe_mode.bit.sub_stripe_en) {
                er = ispframemode_upd_substripe_pingpong_addr(&reg_info, p_drv_array, num, drvio_type, iobuf.pa, iobuf.pa + slice_buf_sz, g_ai_isp_parm[proc_id].core_num);
                if (er){
                    DBG_ERR("ispframemode_upd_substripe_pingpong_addr fail...\r\n");
                    return er;
                }
            } else {
                er = ispframemode_upd_blk_pingpong_addr(&reg_info, p_drv_array, num, drvio_type, iobuf.pa, iobuf.pa + slice_buf_sz);
                if (er){
                    DBG_ERR("ispframemode_upd_blk_pingpong_addr fail...\r\n");
                    return er;
                }
            }
        }
    }

    // update ppu pipe buffer
    if (g_ai_isp_parm[proc_id].pipe_mode.bit.sub_stripe_en && (g_ai_isp_parm[proc_id].pipe_mode.bit.inppu_pipe_en || g_ai_isp_parm[proc_id].pipe_mode.bit.outppu_pipe_en)) {
        AI_JMISP_IOBUF_INFO* p_ppu_pipebuf0 = &g_ai_jmisp_info[proc_id].iobuf[e_AI_JMISP_IOBUF_PPU_PIPEBUF0];
        AI_JMISP_IOBUF_INFO* p_ppu_pipebuf1 = &g_ai_jmisp_info[proc_id].iobuf[e_AI_JMISP_IOBUF_PPU_PIPEBUF1];
        const uint32_t ppu_pipebuf_sz = p_ppu_pipebuf0->buffer_sz;
        er = ispframemode_upd_substripe_ppupipebuf_addr(&reg_info, p_drv_array, num, g_ai_isp_parm[proc_id].core_num, p_ppu_pipebuf0->pa, p_ppu_pipebuf1->pa, ppu_pipebuf_sz);
        if (er){
            DBG_ERR("ispframemode_upd_substripe_ppupipebuf_addr fail...\r\n");
            return er;
        }
    }
    return E_OK;
}
ER kflow_isp_push_slice (UINT32 proc_id, UINT32 push_time)
{
    UINT32  i, start_job = 0, end_job = 0, job_num; 
    KDRV_AI_JMISP_PATH_INFO* p_drv_array; 
    ER er = E_OK ; 
	uintptr_t   jmisp_pl_pa, null_pa_addr; 
    KFLOW_AI_NET* p_net;
	KFLOW_AI_JOB* p_job;
    UINT32 time = 0 ; 

    kflow_isp_net_trace(proc_id, AIISP_FLOW, "push_job() - slice(%u) start \r\n",push_time );

    if (g_ai_isp_parm[proc_id].jmisp_info == 0){
        DBG_ERR(" proc_id[%u] is not started  \n", proc_id);
        return -2;
    }
    time = _nvt_ai_get_counter();
    p_drv_array = g_ai_isp_p_drv[proc_id] ;
    
	memset(&reg_info, 0x0, sizeof(JMISP_REG_INFO)) ;
    gen_jmisp_reg_para(&g_ai_jmisp_info[proc_id], &reg_info);

    if (g_ai_isp_parm[proc_id].pipe_mode.bit.sub_stripe_en) {
        job_num = ispframemode_get_max_substripe_num_in_a_slice(&reg_info, g_ai_isp_parm[proc_id].core_num);
    } else {
        job_num = ispframemode_get_max_blk_num_in_a_slice(&reg_info);
    }

    jmisp_pl_pa = g_ai_isp_parm[proc_id].p_info.parm.user_parm.pa + (g_ai_isp_parm[proc_id].p_pl_cmd_list - g_ai_isp_parm[proc_id].p_info.parm.user_parm.va) ;
    null_pa_addr = ispframemode_get_jmisp_null_pa_pos(g_ai_isp_parm[proc_id].p_pl_cmd_list, g_ai_isp_parm[proc_id].p_info.parm.user_parm.va, g_ai_isp_parm[proc_id].p_info.parm.user_parm.pa, g_ai_isp_parm[proc_id].jmisp_pl_size) ;
    if (g_ai_isp_parm[proc_id].pipe_mode.bit.sub_stripe_en) {
        kflow_isp_get_slice_substripe_num(&reg_info, push_time, &start_job, &end_job, proc_id);
    } else {
        kflow_isp_get_slice_blk_num(&reg_info, push_time, &start_job, &end_job) ;
    }
   
    p_net = kflow_ai_core_net(proc_id);
    if (p_net == NULL){
        DBG_ERR("proc[%d], p_net is NULL...\r\n", proc_id);
        return E_NOEXS;
    }
    DBG_MSG("jmisp_pl_pa %lx \n", (ULONG)jmisp_pl_pa) ; 
    // DBG_DUMP(" push time %u start_job %u end_job %u \n",push_time,start_job, end_job) ;
    // DBG_DUMP(" push time %u  fram addr %lx in_addr_start %lx, in_addr_end %lx out_addr %lx \n", push_time,(ULONG)p_drv_array[start_job].frame[0].frame_addr, (ULONG)p_drv_array[start_job].frame[0].ring_start_addr,(ULONG)p_drv_array[start_job].frame[0].ring_end_addr, (ULONG)p_drv_array[start_job].pingpong[0].pingpong_addr0 ) ; 
   
     for (i = 0 ; i < job_num   ; i++){
        p_job = kflow_ai_net_job(p_net, i);
        if (p_job == NULL){
            DBG_ERR("proc[%d], invalid job_id=%d?\r\n", proc_id, (int)i);
            return E_NOEXS;
        }
        if (i + start_job < end_job){
            p_job->p_io_info = (void*)&p_drv_array[i + start_job];
            p_job->parm_addr = jmisp_pl_pa ;
        }else{
            p_job->p_io_info = (void*)&p_drv_array[0];
            p_job->parm_addr = null_pa_addr ; 
        }
    }
    // {
    //     CHAR file_name[128] = {0};
    //     UINT32 size = p_drv_array[start_job].frame[0].ring_end_addr -  p_drv_array[start_job].frame[0].ring_start_addr ; 
    //     uintptr_t pa = p_drv_array[start_job].frame[0].ring_start_addr ; 
    //     uintptr_t va = nvt_ai_pa2va_remap(pa, size) ; 
    //     snprintf(file_name, sizeof(file_name), "/mnt/sd/dram%u.bin", push_time);
    //     kflow_isp_write_file(file_name, (void *)va, size, 0);
    //     nvt_ai_pa2va_unmap(va, pa) ; 
    // }    
 

    kflow_isp_push_job(p_net, proc_id);
    
    DBG_MSG("push_time = %llu start_job = %u end_job = %u job_num %u\n",_nvt_ai_get_counter() - time, start_job, end_job, job_num);
    kflow_isp_pull_job(p_net) ; 
    kflow_isp_net_trace(proc_id, AIISP_PERF, "push_job() - slice(%u) end slice_job time %u \r\n",push_time,  _nvt_ai_get_counter() - time );
    kflow_isp_net_trace(proc_id, AIISP_FLOW, "push_job() - slice(%u) end \r\n",push_time );
    return er;
}
ER kflow_isp_push_frame (NN_ISP_FRAME *input, NN_ISP_FRAME *output)
{
    UINT32 num = 0, i; 
    KDRV_AI_JMISP_PATH_INFO* p_drv_array; 
    ER er = E_OK ; 
    UINT32 proc_id = input->proc_id ; 
	uintptr_t jmisp_pl_pa, temp_addr; 
    KFLOW_AI_NET* p_net;
	KFLOW_AI_JOB* p_job;
    // UINT32 time = 0 ; 
    // time = _nvt_ai_get_counter();
	
    
    kflow_isp_set_base_addr(proc_id) ;

    if ((g_ai_jmisp_info[proc_id].iobuf[e_AI_JMISP_IOBUF_REF_IN0].buffer_sz > 0) && (g_ai_jmisp_info[proc_id].iobuf[e_AI_JMISP_IOBUF_REF_OUT0].buffer_sz > 0)) {
        temp_addr = g_ai_jmisp_info[proc_id].iobuf[e_AI_JMISP_IOBUF_REF_IN0].pa ; 
        g_ai_jmisp_info[proc_id].iobuf[e_AI_JMISP_IOBUF_REF_IN0].pa =  g_ai_jmisp_info[proc_id].iobuf[e_AI_JMISP_IOBUF_REF_OUT0].pa ; 
        g_ai_jmisp_info[proc_id].iobuf[e_AI_JMISP_IOBUF_REF_OUT0].pa = temp_addr ; 
    }
    
    if ((g_ai_jmisp_info[proc_id].iobuf[e_AI_JMISP_IOBUF_REF_IN1].buffer_sz > 0) && (g_ai_jmisp_info[proc_id].iobuf[e_AI_JMISP_IOBUF_REF_OUT1].buffer_sz > 0)) {
        temp_addr = g_ai_jmisp_info[proc_id].iobuf[e_AI_JMISP_IOBUF_REF_IN1].pa ; 
        g_ai_jmisp_info[proc_id].iobuf[e_AI_JMISP_IOBUF_REF_IN1].pa = g_ai_jmisp_info[proc_id].iobuf[e_AI_JMISP_IOBUF_REF_OUT1].pa ; 
        g_ai_jmisp_info[proc_id].iobuf[e_AI_JMISP_IOBUF_REF_OUT1].pa = temp_addr ;
    }
    
    g_ai_jmisp_info[proc_id].iobuf[e_AI_JMISP_IOBUF_RING_IN0].pa = input->pa ; 
    g_ai_jmisp_info[proc_id].iobuf[e_AI_JMISP_IOBUF_PINGPONG_OUT0].pa = output->pa ; 

   
    memset(&reg_info, 0x0, sizeof(JMISP_REG_INFO)) ;
    gen_jmisp_reg_para(&g_ai_jmisp_info[proc_id], &reg_info);
  
    jmisp_pl_pa = g_ai_isp_parm[proc_id].p_info.parm.user_parm.pa + (g_ai_isp_parm[proc_id].p_pl_cmd_list - g_ai_isp_parm[proc_id].p_info.parm.user_parm.va) ;
    p_drv_array = g_ai_isp_p_drv[input->proc_id] ; 

    // DBG_MSG("jmisp_pl_pa %lx \n", (ULONG)jmisp_pl_pa) ; 
    
     if (g_ai_isp_parm[proc_id].pipe_mode.bit.sub_stripe_en) {
        num =  ispframemode_get_total_substripe_num (&reg_info, g_ai_isp_parm[proc_id].core_num);
        er = ispframemode_create_substripe_array(&g_ai_jmisp_info[proc_id], p_drv_array, num, g_ai_isp_parm[proc_id].core_num);
        if (er){
            DBG_ERR("ispframemode_create_substripe_array fail...\r\n");
            return er;
        }
    } else {
        num =  ispframemode_get_total_blk_num (&reg_info) ;
        er = ispframemode_create_blkinfo_array(&g_ai_jmisp_info[proc_id], p_drv_array, num);
        if (er){
            DBG_ERR("ispframemode_create_blkinfo_array fail...\r\n");
            return er;
        }
    }
   
    // update pingpong out
    {
        int32_t i;
        for (i=0; i<=e_AI_IOBUF_NUM; ++i) {
            const e_AI_JMISP_IOBUF_CONST iobuf_type = (e_AI_JMISP_IOBUF_CONST)i;
            const e_AI_JMISP_DRV_IO_CONST drvio_type = convert_iobuf2drvio_type(&g_ai_jmisp_info[proc_id].iobuf[iobuf_type], iobuf_type);
            AI_JMISP_IOBUF_INFO iobuf = {0} ; 
            uint32_t max_stripe_sz, slice_buf_sz;
            if ((!is_pingpong_buf(&g_ai_jmisp_info[proc_id].iobuf[iobuf_type], iobuf_type))
                || (g_ai_jmisp_info[proc_id].iobuf[iobuf_type].buffer_sz <= 0)) {
                continue;
            }
            iobuf = g_ai_jmisp_info[proc_id].iobuf[iobuf_type] ; 
            max_stripe_sz = max(max(iobuf.stripeInfo.first_stripe_width, iobuf.stripeInfo.mid_stripe_width), iobuf.stripeInfo.last_stripe_width);
            slice_buf_sz = iobuf.stripeInfo.output_slice_height * max_stripe_sz * iobuf.bitdepth >> 3;
            if (g_ai_isp_parm[proc_id].pipe_mode.bit.sub_stripe_en) {
                er = ispframemode_upd_substripe_pingpong_addr(&reg_info, p_drv_array, num, drvio_type, iobuf.pa, iobuf.pa + slice_buf_sz, g_ai_isp_parm[proc_id].core_num);
                if (er){
                    DBG_ERR("ispframemode_upd_substripe_pingpong_addr fail...\r\n");
                    return er;
                }
            } else {
                er = ispframemode_upd_blk_pingpong_addr(&reg_info, p_drv_array, num, drvio_type, iobuf.pa, iobuf.pa + slice_buf_sz);
                if (er){
                    DBG_ERR("ispframemode_upd_blk_pingpong_addr fail...\r\n");
                    return er;
                }
            }
        }
    }

    // update ppu pipe buffer
    if (g_ai_isp_parm[proc_id].pipe_mode.bit.sub_stripe_en && (g_ai_isp_parm[proc_id].pipe_mode.bit.inppu_pipe_en || g_ai_isp_parm[proc_id].pipe_mode.bit.outppu_pipe_en)) {
        AI_JMISP_IOBUF_INFO* p_ppu_pipebuf0 = &g_ai_jmisp_info[proc_id].iobuf[e_AI_JMISP_IOBUF_PPU_PIPEBUF0];
        AI_JMISP_IOBUF_INFO* p_ppu_pipebuf1 = &g_ai_jmisp_info[proc_id].iobuf[e_AI_JMISP_IOBUF_PPU_PIPEBUF1];
        const uint32_t ppu_pipebuf_sz = p_ppu_pipebuf0->buffer_sz;
        er = ispframemode_upd_substripe_ppupipebuf_addr(&reg_info, p_drv_array, num, g_ai_isp_parm[proc_id].core_num, p_ppu_pipebuf0->pa, p_ppu_pipebuf1->pa, ppu_pipebuf_sz);
        if (er){
            DBG_ERR("ispframemode_upd_substripe_ppupipebuf_addr fail...\r\n");
            return er;
        }
    }

    p_net = kflow_ai_core_net(proc_id);
    if (p_net == NULL){
        DBG_ERR("proc[%d], p_net is NULL...\r\n", proc_id);
        return E_NOEXS;
    }
 
    for (i = 0 ; i < num  ; i++){
        p_job = kflow_ai_net_job(p_net, i);
        if (p_job == NULL){
            DBG_ERR("proc[%d], invalid job_id=%d?\r\n", proc_id, (int)i);
            return E_NOEXS;
        }
        p_job->p_io_info = (void*)&p_drv_array[i];
        p_job->parm_addr = jmisp_pl_pa ;
    }
                    
    kflow_isp_push_job(p_net, proc_id);
    kflow_isp_pull_job(p_net) ; 

    

    return E_OK;
}
#endif

