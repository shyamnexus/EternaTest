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
#define __CLASS__ 				"[ai][kflow_isp][isp_flow]"
#include "kflow_ai_isp_debug.h"
//=============================================================
#include "kflow_isp/nn_isp.h"
#include "kflow_isp/kflow_isp_platform.h"
#include "kflow_isp/kflow_isp.h"
#include "kflow_isp/kflow_isp_change_res.h"
#include "kflow_ai_isp_build_instrs_sync_sdktool.h"
#include "kflow_isp/kflow_ai_isp_task.h"

#if (FLOW_AI_ISP == 1)

SEM_HANDLE g_ai_isp_lock;
SEM_HANDLE g_ai_isp_id ; 
SEM_HANDLE g_ai_isp_dgain_lock;
ID FLG_ID_JMISP = 0;
UINT32 MAX_ISP_NUM ;
UINT32 MAX_ISP_UB ;
UINT32 g_isp_trig_mode = 0 ;
INT32 *kflow_isp_sensor_map = 0 ;
UINT32 * kflow_isp_push_time = 0 ;
UINT32 * kflow_isp_first_proc = 0 ;
NN_ISP_ENABLE_3D * kflow_isp_enable_3d = 0 ;
UINT32 *kflow_isp_weight_loc_map = 0 ;  
NN_ISP_NN_INFO *kflow_isp_nn_info = 0;
NN_ISP_PATH_BUF_INFO *kflow_isp_ubuf_layout = 0;
KDRV_AI_JMISP_PATH_INFO *kflow_isp_drv_para =0;
NN_ISP_INPUT **g_ai_input_layer_map_table = 0;
uintptr_t **g_ai_input_addr_map_table = 0;
VENDOR_AIS_ISP_INPUT_INFO **g_ai_input_layer_info_table = 0; // 539A flow, (1)save input info when start (2) used when IPP callback to set input
KDRV_AI_JMISP_PATH_INFO  **g_ai_isp_p_drv = 0;
UINT32 *g_ai_input_layer_map_num = 0;
UINT32 *g_ai_input_layer_info_num = 0;
AI_TUNING_QUAN_PARAM **g_ai_isp_ori_ppu_quan_param = 0; // keep curr/sigma 's ppu quan param
UINT8 *g_ai_isp_pre_dgain = 0;
UINT32 *g_proc_trace = NULL;
KFLOW_AI_ISP_JOB* kflow_isp_job_statck = 0 ; 
VENDOR_AIS_ISP_PARM * g_ai_isp_parm = 0 ; 
AI_JMISP_INFO* g_ai_jmisp_info = 0 ; 
VENDOR_AIS_ISP_MEM_PARM *g_ai_isp_input_addr = 0; 
VENDOR_AIS_ISP_MEM_PARM *g_ai_isp_inputfeat_pingpong_seg0 = 0; 
VENDOR_AIS_ISP_MEM_PARM *g_ai_isp_inputfeat_pingpong_seg1 = 0; 
BOOL kflow_isp_init = 0 ;
UINT32	kflow_isp_init_count = 0;
INT32 kflow_isp_used_pool[4] = {-1, -1, -1, -1};
UINT32 kflow_isp_pool[4][5] = {0} ;
INT32 *kflow_isp_proc_pool_map = 0;
INT32 *kflow_isp_mode_map = 0;
UINT32* kflow_isp_proc_core_mask = 0;
UINT32* kflow_isp_proc_ub_mask = 0;
extern ER nvt_ai_comm_unlock(VOID) ; 
extern ER nvt_ai_comm_lock(VOID) ; 
extern ER nvt_ai_dump_ubuf_status(void) ; 
extern INT32 g_ai_net_ubuf_status[AI_SUPPORT_UBUF_MAX];
extern UINT32 kflow_isp_kcmd_proc_id ;
extern UINT32 kflow_isp_kcmd_trace ;


static CHAR msg_f1[] = _ANSI_W_"ai: \"proc[%d]\": %s";
static CHAR msg_g1[] = _ANSI_C_"ai: \"proc[%d]\": %s";
static CHAR msg_j1[] = _ANSI_Y_"ai: \"proc[%d]\": %s";
static CHAR msg_b1[] = _ANSI_M_"ai: \"proc[%d]\": %s";
// static CHAR msg_p1[] = _ANSI_G_"ai: \"proc[%d]\": %s";
// static CHAR msg_r1[] = _ANSI_R_"ai: \"proc[%d]\": %s";

#define JOBLL_DEBUG     0 

static char msg[512] = {0};
CHAR new_fmtstr[512] = {0};

void kflow_isp_net_trace(UINT32 proc_id, UINT32 class_bits, const char *fmtstr, ...)
{
   UINT32 trace;
//    if (proc_id == 0xffffffff) trace = g_all_trace;
//    else trace = g_proc_trace[proc_id];
   trace = g_proc_trace[proc_id];
   if (trace & class_bits) {
      va_list marker;

      if (class_bits & AIISP_INPUT)
         snprintf(new_fmtstr, 512, msg_f1, proc_id, fmtstr);
      if (class_bits & AIISP_FLOW)
         snprintf(new_fmtstr, 512, msg_g1, proc_id, fmtstr);
      if (class_bits & AIISP_PERF)
         snprintf(new_fmtstr, 512, msg_j1, proc_id, fmtstr);
      if (class_bits & AIISP_BUF)
         snprintf(new_fmtstr, 512, msg_b1, proc_id, fmtstr);
    //   if (class_bits & AI_PERF)
    //      snprintf(new_fmtstr, 512, msg_p1, proc_id, fmtstr);
    //   if (class_bits & AI_RES)
    //      snprintf(new_fmtstr, 512, msg_r1, proc_id, fmtstr);

      va_start(marker, fmtstr);
      vsnprintf((void *)msg, sizeof(msg)-1, new_fmtstr, marker);
      va_end(marker);

	  vk_printk(msg);
   }
}
ER kflow_isp_fix_jmisp_addr (uintptr_t jmisp_addr, uintptr_t parm_va_ofs, UINT32 size)
{
	UINT32 i = 0;
    UINT64 *temp_ptr = 0;

#if JOBLL_DEBUG
            DBG_DUMP(" kflow_isp_fix_jmisp_addr, jmisp_addr = %lx parm_va_ofs = %lx size = %u\n", (ULONG)jmisp_addr,  (ULONG)parm_va_ofs, size);
#endif	

    while(i < size) {
        temp_ptr = (UINT64 *)(jmisp_addr + i);
        if (NN_JOBLL_CMD_GET_MODE(*temp_ptr) == 0) {
#if JOBLL_DEBUG
			PATH_LIST_NULL_CMD* null_cmd = (PATH_LIST_NULL_CMD *)temp_ptr;

            DBG_DUMP("      (%4d)   cmd(NULL),  mode(0x%x)\n", 
            (int)i,  (UINT)null_cmd->mode);
#endif	
			i += 4;	
			//break;	//return E_OK;	//TODO:
          
		 } else if (NN_JOBLL_CMD_GET_MODE(*temp_ptr) == 1) {
#if JOBLL_DEBUG
            PATH_LIST_UPD_CMD *upd_cmd = (PATH_LIST_UPD_CMD *)temp_ptr;

            DBG_DUMP("      (%4d)   cmd(UPD) ,value(0x%08x), ofs(0x%03x), byte_en(0x%x), mode(0x%x)\n", 
            (int)i,  (UINT)upd_cmd->value, (UINT)upd_cmd->reg_ofs, (UINT)upd_cmd->byte_en, (UINT)upd_cmd->mode);
#endif		
            
            i += 8;	
        } else if (NN_JOBLL_CMD_GET_MODE(*temp_ptr) == 2) {
			PATH_LIST_JUMP_CMD* jump_cmd = (PATH_LIST_JUMP_CMD *)temp_ptr;
			jump_cmd->msb_addr += GET_MSB(parm_va_ofs); 
			jump_cmd->address += GET_LSB(parm_va_ofs); 
#if JOBLL_DEBUG
            DBG_DUMP("      (%4d)   cmd(JUMP) ,addr_type(0x%x), msb_addr(0x%04x), address(0x%08x), mode(0x%x)\n", 
            (int)i,  (UINT)jump_cmd->addr_type, (UINT)jump_cmd->msb_addr, (UINT)jump_cmd->address,  (UINT)jump_cmd->mode);
#endif	
            i += 8;
        } else if (NN_JOBLL_CMD_GET_MODE(*temp_ptr) == 4) {
			PATH_LIST_TRIG_CMD* trig_cmd = (PATH_LIST_TRIG_CMD *)temp_ptr;
			trig_cmd->msb_addr += GET_MSB(parm_va_ofs); 
			trig_cmd->address += GET_LSB(parm_va_ofs); 
#if JOBLL_DEBUG
            DBG_DUMP("      (%4d)   cmd(TRIGGER) ,addr_type(0x%x), func_idx(0x%03x), msb_addr(0x%04x), address(0x%08x), flag_id(0x%x), func_sel(0x%x), mode(0x%x)\n", 
            (int)i,  (UINT)trig_cmd->addr_type, (UINT)trig_cmd->func_idx, (UINT)trig_cmd->msb_addr, (UINT)trig_cmd->address, (UINT)trig_cmd->flag_id, (UINT)trig_cmd->func_sel, (UINT)trig_cmd->mode);
#endif	
            i += 12;
        } else if (NN_JOBLL_CMD_GET_MODE(*temp_ptr) == 5) {
#if JOBLL_DEBUG
			PATH_LIST_WAIT_CMD* wait_cmd = (PATH_LIST_WAIT_CMD *)temp_ptr;

            DBG_DUMP("      (%4d)   cmd(WAIT) ,flag_id(0x%x),  mode(0x%x)\n", 
            (int)i,  (UINT)wait_cmd->flag_id, (UINT)wait_cmd->mode);
#endif	
            i += 4;
        } else if (NN_JOBLL_CMD_GET_MODE(*temp_ptr) == 6) {
			PATH_LIST_CONDITION_JUMP_CMD* condjump_cmd = (PATH_LIST_CONDITION_JUMP_CMD *)temp_ptr;
			condjump_cmd->msb_addr += GET_MSB(parm_va_ofs); 
			condjump_cmd->address += GET_LSB(parm_va_ofs); 
#if JOBLL_DEBUG
            DBG_DUMP("      (%4d)   cmd(CONDJUMP) ,condi_val(0x%08x), addr_type(0x%x), byte_en(0x%x), msb_addr(0x%04x), address(0x%08x), reg_ofs(0x%03x), mode(0x%x)\n", 
            (int)i,  (UINT)condjump_cmd->condi_val, (UINT)condjump_cmd->addr_type, (UINT)condjump_cmd->byte_en, (UINT)condjump_cmd->msb_addr, (UINT)condjump_cmd->address, (UINT)condjump_cmd->reg_ofs, (UINT)condjump_cmd->mode);
#endif	
            i += 12;
        } else if (NN_JOBLL_CMD_GET_MODE(*temp_ptr) == 7) {
#if JOBLL_DEBUG
			PATH_LIST_MOV_CMD* mov_cmd = (PATH_LIST_MOV_CMD *)temp_ptr;

            DBG_DUMP("      (%4d)   cmd(MOV) ,dst_reg_ofs(0x%03x), src_reg_ofs(0x%03x), mode(0x%x)\n", 
            (int)i,  (UINT)mov_cmd->dst_reg_ofs, (UINT)mov_cmd->src_reg_ofs, (UINT)mov_cmd->mode);
#endif	
            i += 4;
        } else if (NN_JOBLL_CMD_GET_MODE(*temp_ptr) == 8) {
#if JOBLL_DEBUG
            PATH_LIST_LOGIC_WAIT_CMD* logicwait_cmd = (PATH_LIST_LOGIC_WAIT_CMD *)temp_ptr;

            DBG_DUMP("      (%4d)   cmd(LOGICWAIT) ,flag_status(0x%x), op(0x%x), mode(0x%x)\n", 
            (int)i,  (UINT)logicwait_cmd->flag_status, (UINT)logicwait_cmd->op, (UINT)logicwait_cmd->mode);
#endif
            i += 12;
        } else if (NN_JOBLL_CMD_GET_MODE(*temp_ptr) == 15) {
#if JOBLL_DEBUG
            PATH_LIST_ALU_CMD* alu_cmd = (PATH_LIST_ALU_CMD *)temp_ptr;

            DBG_DUMP("      (%4d)   cmd(ALU) ,value(0x%08x), reg_ofs(0x%03x), op(0x%x), mode(0x%x)\n", 
            (int)i,  (UINT)alu_cmd->value, (UINT)alu_cmd->reg_ofs, (UINT)alu_cmd->op, (UINT)alu_cmd->mode);
#endif			
            i += 8;
        } else {
			DBG_ERR("unknown job list cmd mode : %d\r\n", (int)NN_JOBLL_CMD_GET_MODE(*temp_ptr));
		}
    }

	// fix broken image
	vos_cpu_dcache_sync(jmisp_addr, size, VOS_DMA_TO_DEVICE);

	return E_OK;
}

ER kflow_isp_fix_ppu_addr (uintptr_t ppu_addr, uintptr_t parm_va_base, uintptr_t parm_pa_base, UINT32 size, UINT32 ppu_num) {

	
	UINT32 i = 0;
	const uintptr_t cache_addr = ppu_addr;

	if (ppu_addr == 0) {
		DBG_ERR("null ppu address\r\n");
		return -1;
	}

	for (i=0; i<ppu_num; ++i) {
		JMISPPPU_HEAD* p_head = (JMISPPPU_HEAD*)ppu_addr;
		UINT32 cmd_pos, per_ppu_sz;
		p_head->parm_addr += parm_va_base;
		per_ppu_sz = sizeof(JMISPPPU_HEAD) + p_head->parm_size;
		if (per_ppu_sz == 0) {
			DBG_ERR("ppu parm size is 0\r\n");
			continue;
		}

#if NNISP_DEBUG
		{
			UINT32 j = 0;
			DBG_DUMP("====== ppu debug start ======\n");
			DBG_DUMP("  p_head(0x%016lx)\n", (uintptr_t)p_head);
			DBG_DUMP("  parm_addr = 0x%016lx\n", p_head->parm_addr);
			DBG_DUMP("  parm_size = %u\n", p_head->parm_size);
			DBG_DUMP("  match_mctrl_idx = %u\n", p_head->match_mctrl_idx);
			for (j=0; j<JMISP_PPU_SUP_IOBUF_NUM; ++j) {
				DBG_DUMP("  match_iobuf_ID[%u] = %u\n", j, p_head->match_iobuf_ID[j]);
			}
			DBG_DUMP("====== ppu debug end ======\n");
		}
#endif

		cmd_pos = 0;
		while(cmd_pos < p_head->parm_size) {
			UINT64 *temp_ptr = (UINT64 *)(p_head->parm_addr + cmd_pos);
			if (NN_LL_CMD_GET_MODE(*temp_ptr) == 2) {
				LINK_LIST_JUMP_CMD* jump_cmd = (LINK_LIST_JUMP_CMD *)temp_ptr;
				jump_cmd->msb_addr += GET_MSB(parm_pa_base); 
				jump_cmd->address += GET_LSB(parm_pa_base); 
				cmd_pos += 8;
			} else if ((NN_LL_CMD_GET_MODE(*temp_ptr) == 0) || (NN_LL_CMD_GET_MODE(*temp_ptr) == 1) || (NN_LL_CMD_GET_MODE(*temp_ptr) == 3)) {
				cmd_pos += 8;
			} else {
				DBG_ERR("unknown linked list cmd mode : %d\r\n", (int)NN_LL_CMD_GET_MODE(*temp_ptr));
			}
		}

#if 0	//NNISP_DEBUG
		cmd_pos = 0;
		while(cmd_pos < p_head->parm_size) {
			UINT64 *temp_ptr = (UINT64 *)(p_head->parm_addr + cmd_pos);
			if (NN_LL_CMD_GET_MODE(*temp_ptr) == 0) {
				LINK_LIST_NULL_CMD* null_cmd = (LINK_LIST_NULL_CMD *)temp_ptr;
				DBG_DUMP("      (%4d)   cmd(NULL),  mode(0x%u), table_index(%d)\n",
				(int)i, (UINT)null_cmd->mode, (UINT)null_cmd->table_index);

			} else if (NN_LL_CMD_GET_MODE(*temp_ptr) == 1) {
				LINK_LIST_UPD_CMD *upd_cmd = (LINK_LIST_UPD_CMD *)temp_ptr;
				DBG_DUMP("      (%4d)   cmd(UPD) ,value(0x%u), ofs(0x%u), byte_en(0x%u), mode(0x%u)\n", 
				(int)i, (UINT)upd_cmd->value, (UINT)upd_cmd->reg_ofs, (UINT)upd_cmd->byte_en, (UINT)upd_cmd->mode);
			} else if (NN_LL_CMD_GET_MODE(*temp_ptr) == 2) {
				LINK_LIST_JUMP_CMD* jump_cmd = (LINK_LIST_JUMP_CMD *)temp_ptr;
				DBG_DUMP("      (%4d)   cmd(JUMP) ,addr_type(0x%u), msb_addr(0x%x), address(0x%x), mode(0x%u)\n", 
				(int)i, (UINT)jump_cmd->addr_type, (UINT)jump_cmd->msb_addr, (UINT)jump_cmd->address,  (UINT)jump_cmd->mode);
			} else if (NN_LL_CMD_GET_MODE(*temp_ptr) == 3) {
				LINK_LIST_NEXTLL_CMD* nextll_cmd = (LINK_LIST_NEXTLL_CMD *)temp_ptr;
				DBG_DUMP("      (%4d)   cmd(NEXTLL),  mode(0x%u), table_index(%d)\n", 
				(int)i, (UINT)nextll_cmd->mode, (UINT)nextll_cmd->table_index);
			} else {
				DBG_ERR("unknown linked list cmd mode : %d\r\n", (int)NN_LL_CMD_GET_MODE(*temp_ptr));
			}
			cmd_pos += 8;
		}
#endif

		ppu_addr += per_ppu_sz;
	}

	// fix broken image
	vos_cpu_dcache_sync(cache_addr, size, VOS_DMA_TO_DEVICE);

	return E_OK;
}
void kflow_isp_set_prioity(UINT32 job_priority)
{
	UINT32 i ;
	KDRV_AI_JOBM_PARAM jobm_parm = {0};

	jobm_parm.jl_priority = job_priority;

	for (i=0 ; i < MAX_ISP_NUM; i++){
		kdrv_ai_set(KDRV_AI_PARAM_JOBM_JLA_PARAM + i, &jobm_parm);
		DBG_MSG("-------------------kflow_isp_set_prioity jobm %u pri %u---------------------\n", i , job_priority);
	}
}

ER kflow_isp_get_isp_trig_mode(UINT32* g_isp_trig_mode)
{
	int res = 0 ;
	CHAR* path = "/ai-isp-cfg" ; 
	CHAR* name = "isp_trig_mode" ; 
	res = kflow_ai_isp_get_config_from_dtsi(path, name , g_isp_trig_mode, 1) ;
	if (res != E_OK ){
		DBG_WRN("can't find \"ai-isp-cfg\" at dtsi\r\n");
	}
		
	return  res ;
}

ER kflow_isp_get_max_isp_num(UINT32* ISP_NUM)
{
	int res = 0 ;
	CHAR* path = "/ai-maxpath-cfg" ; 
	CHAR* name = "ai_maxisp" ; 
	res = kflow_ai_isp_get_config_from_dtsi(path, name , ISP_NUM, 1) ;
	if (res != E_OK ){
		DBG_WRN("can't find \"ai_maxisp\" at dtsi\r\n");
	}
	if (*ISP_NUM == 0){
		DBG_WRN("find \"ai_maxisp = 0\" at dtsi\r\n");
		return E_NOSPT ;
	}
		
	return  res ;
}
ER kflow_isp_get_max_ubuf(UINT32* UB)
{
	ER ret = E_OK;
	CHAR* path = "/ai-maxpath-cfg";
	CHAR* name = "ai_maxubuf";
	if (E_OK !=  kflow_ai_isp_get_config_from_dtsi(path, name , UB, 1)) {
		*UB= 0;  // can't find "ai_maxubuf" at dtsi, means ai-isp do not need ubuf
		DBG_WRN("can't find \"ai_maxubuf\" at dtsi, use 0 instead\r\n");
	}
	return ret;
}
ER kflow_isp_get_pool(UINT32* pool, UINT32 i)
{
	ER ret = E_OK;
	CHAR* path = "/ai-pool-cfg";
	CHAR name[25] = {0};
    snprintf(name, 25, "pool_%u", i);
	if (E_OK !=  kflow_ai_isp_get_config_from_dtsi(path, name , pool, 5)) {
		 // can't find "pool_0" at dtsi, means ai-isp do not need ubuf
		DBG_WRN("can't find \"pool_%u\" at dtsi \r\n", i);
		ret = E_NOEXS ;
	}
	return ret;
}
ER kflow_isp_get_st_in_ub_mask(UINT32 proc_id, INT32 *available_idx)
{
#if defined(_BSP_NS02201_)
	UINT32 UBUF_MAX  = 4 ;
#elif defined(_BSP_NS02302_)
	UINT32 UBUF_MAX  = 3 ;
#elif defined(_BSP_NS02401_)
	UINT32 UBUF_MAX  = 3 ;
#endif
	INT32 i = 0, ch_mask = 0 ;

	if (kflow_isp_proc_ub_mask[proc_id]){
		for (i=0 ; i < UBUF_MAX ; i++){
			ch_mask = (1L << (i));
			if (kflow_isp_proc_ub_mask[proc_id] & ch_mask){
				*available_idx = i ;
				return E_OK;
			}
		}
	}
	return E_SYS;
}
ER kflow_isp_get_st_in_core_mask(UINT32 mask, INT32 *available_idx)
{
	INT32 i = 0, ch_mask = 0 ;

	if (mask){
		for (i=0 ; i < 4 ; i++){
			ch_mask = (1L << (i));
			if (mask & ch_mask){
				*available_idx = i ;
				return E_OK;
			}
		}
	}
	return E_SYS;

}
ER kflow_isp_get_ai_pool(void)
{
	UINT32 pool_val[5] = {0 ,0 ,0 ,0, 0}, i , j ; 
	INT32 UB[4] = {-1 ,-1 ,-1 ,-1};
	INT32 core[4] = {-1 ,-1 ,-1 ,-1};
	ER ret = E_NOEXS;

	kflow_isp_get_max_ubuf(&MAX_ISP_UB);

	for (i=0; i < 4; i++){
		memset(pool_val, 0, sizeof(UINT32) * 5);
		if (E_OK == kflow_isp_get_pool(pool_val, i)){
			for (j=0; j < 5; j++){
					kflow_isp_pool[i][j] = pool_val[j] ; 
					// DBG_DUMP("kflow_isp_get_pool_%u %u  \n", i, pool_val[j]) ; 
				}
				ret = E_OK ; 
				kflow_isp_used_pool[i] = 0 ;
		}	
	}	
	if (ret != E_OK){
		DBG_ERR("kflow_isp_get_ai_pool fail, can't find any ai_pool in dtsi\r\n");
		return ret ; 
	}

	// check dtsi setting 
	for (i=0; i < 4; i++){
		if (kflow_isp_used_pool[i] == 0 ){

			// check UB id & Core ch are valid
			if (kflow_isp_pool[i][0] > 0 ){
			// if UB is on or shared
				if ( kflow_isp_pool[i][1] >  kflow_isp_pool[i][2]){
					DBG_ERR("pool_%u [UB st_id](%u) > [UB end_id](%u) \r\n", i, kflow_isp_pool[i][1], kflow_isp_pool[i][2]);
					return E_PAR;
				}
				if (0 > kflow_isp_pool[i][1] || kflow_isp_pool[i][1] >= MAX_ISP_UB){
					DBG_ERR("pool_%u [UB st_id] %u shoule be in the interval [0, %u] \r\n", i, kflow_isp_pool[i][1], MAX_ISP_UB - 1);
					return E_PAR;
				}
				if (0 > kflow_isp_pool[i][2] || kflow_isp_pool[i][2] >= MAX_ISP_UB){
					DBG_ERR("pool_%u [UB end_id] %u shoule be in the interval [0, %u] \r\n", i, kflow_isp_pool[i][2], MAX_ISP_UB - 1);
					return E_PAR;
				}
			}
			if ( kflow_isp_pool[i][3] >  kflow_isp_pool[i][4]){
				DBG_ERR("pool_%u [Core st_ch](%u) > [Core end_ch](%u) \r\n", i, kflow_isp_pool[i][3], kflow_isp_pool[i][4]);
				return E_PAR;
			}

			if (0 > kflow_isp_pool[i][3] || kflow_isp_pool[i][3] >= MAX_ISP_NUM){
				DBG_ERR("pool_%u [Core st_ch] %u shoule be in the interval [0, %u] \r\n", i, kflow_isp_pool[i][3], MAX_ISP_NUM - 1);
				return E_PAR;
			}
			if (0 > kflow_isp_pool[i][4] || kflow_isp_pool[i][4] >= MAX_ISP_NUM){
				DBG_ERR("pool_%u [Core end_ch] %u shoule be in the interval [0, %u] \r\n", i, kflow_isp_pool[i][4], MAX_ISP_NUM - 1);
				return E_PAR ;
			}
			// check UB id has not been rearranged in 2 pool_id
			for (j = kflow_isp_pool[i][1]; j <= kflow_isp_pool[i][2] ; j++ ){
				if(kflow_isp_pool[i][0] == 0 ){
					// if UB is off, skip checking
					continue;
				}
				if (UB[j] == -1 ){
					UB[j] = i ;
				}else {
					DBG_ERR("pool_%u UB id[%u] has been rearranged in 2 pool_id \r\n",i, j);
					return E_PAR ;
				} 
						
				
			}
			// check Core ch has not been rearranged in 2 pool_id
			for (j = kflow_isp_pool[i][3]; j <= kflow_isp_pool[i][4] ; j++ ){
				if (core[j] == -1 ){
					core[j] = i ;
				}else {
					DBG_ERR("pool_%u Core ch[%u] has been rearranged in 2 pool_id \r\n", i, j);
					return E_PAR ;
				}
			}

		}
	}


	return ret ; 

}

ER kflow_isp_init_jmisp_net(void)
{
	UINT32	g_ai_net_max ; 
	UINT32  i=0;
	g_ai_net_max = kflow_ai_get_net_supported_num() ; 
	if (kflow_isp_get_max_isp_num(&MAX_ISP_NUM) != E_OK){
		// kflow_isp_init stay in uninit status 
		return E_OK;
	}	

	kflow_isp_init_count += 1;
    if (kflow_isp_init > 0) {
        return E_OK;
    }

	if(kflow_isp_init == 0) {

		SEM_CREATE(g_ai_isp_lock, 1);
		SEM_CREATE(g_ai_isp_id, 1);
		SEM_CREATE(g_ai_isp_dgain_lock, 1);
		OS_CONFIG_FLAG(FLG_ID_JMISP);
		clr_flg(FLG_ID_JMISP, FLG_ID_JMISP_ALL); //clear all flag

		kflow_isp_get_isp_trig_mode(&g_isp_trig_mode) ;

		memset(kflow_isp_used_pool, -1, sizeof(INT32) * 4);
		memset(kflow_isp_pool, 0, sizeof(INT32) * 4 * 5);

		kflow_isp_proc_pool_map  = (INT32 *)nvt_ai_mem_alloc(sizeof(INT32) * g_ai_net_max); // record pool_id of each proc_id
		if (kflow_isp_proc_pool_map == NULL) {
				DBG_ERR("kflow_isp_proc_pool_map alloc fail\r\n");
				return E_NOMEM;
		}
		memset(kflow_isp_proc_pool_map, -1, sizeof(INT32) * g_ai_net_max);

		kflow_isp_mode_map  = (INT32 *)nvt_ai_mem_alloc(sizeof(INT32) * g_ai_net_max); // record isp mode of each proc_id
		if (kflow_isp_mode_map == NULL) {
				DBG_ERR("kflow_isp_mode_map alloc fail\r\n");
				return E_NOMEM;
		}
		memset(kflow_isp_mode_map, -1, sizeof(INT32) * g_ai_net_max);

		kflow_isp_sensor_map  = (INT32 *)nvt_ai_mem_alloc(sizeof(INT32) * 64); // max sensor size 64
		if (kflow_isp_sensor_map == NULL) {
			DBG_ERR("kflow_isp_sensor_map alloc fail\r\n");
			return E_NOMEM;
		}
		memset(kflow_isp_sensor_map, -1, sizeof(INT32) * 64);

		kflow_isp_proc_core_mask = (UINT32 *)nvt_ai_mem_alloc(sizeof(UINT32) * g_ai_net_max); 
		if (kflow_isp_proc_core_mask == NULL) {
			DBG_ERR("kflow_isp_proc_core_mask alloc fail\r\n");
			return E_NOMEM;
		}
		memset(kflow_isp_proc_core_mask, 0x0, sizeof(UINT32) * g_ai_net_max);

		kflow_isp_proc_ub_mask = (UINT32 *)nvt_ai_mem_alloc(sizeof(UINT32) * g_ai_net_max); 
		if (kflow_isp_proc_ub_mask == NULL) {
			DBG_ERR("kflow_isp_proc_ub_mask alloc fail\r\n");
			return E_NOMEM;
		}
		memset(kflow_isp_proc_ub_mask, 0x0, sizeof(UINT32) * g_ai_net_max);

		kflow_isp_nn_info = (NN_ISP_NN_INFO *)nvt_ai_mem_alloc(sizeof(NN_ISP_NN_INFO) * g_ai_net_max);
		if (kflow_isp_nn_info == NULL) {
			DBG_ERR("kflow_isp_nn_info alloc fail\r\n");
			return E_NOMEM;
		}
		memset(kflow_isp_nn_info, 0x0, sizeof(NN_ISP_NN_INFO) * g_ai_net_max);

		kflow_isp_ubuf_layout = (NN_ISP_PATH_BUF_INFO *)nvt_ai_mem_alloc(sizeof(NN_ISP_PATH_BUF_INFO) * g_ai_net_max);
		if (kflow_isp_ubuf_layout == NULL) {
			DBG_ERR("kflow_isp_ubuf_layout alloc fail\r\n");
			return E_NOMEM;
		}
		memset(kflow_isp_ubuf_layout, 0x0, sizeof(NN_ISP_PATH_BUF_INFO) * g_ai_net_max);

		kflow_isp_drv_para = (KDRV_AI_JMISP_PATH_INFO *)nvt_ai_mem_alloc(sizeof(KDRV_AI_JMISP_PATH_INFO) * g_ai_net_max);
		if (kflow_isp_drv_para == NULL) {
			DBG_ERR("kflow_isp_drv_para alloc fail\r\n");
			return E_NOMEM;
		}
		memset(kflow_isp_drv_para, 0x0, sizeof(KDRV_AI_JMISP_PATH_INFO) * g_ai_net_max);

		kflow_isp_job_statck = (KFLOW_AI_ISP_JOB *)nvt_ai_mem_alloc(sizeof(KFLOW_AI_ISP_JOB) * KFLOW_AI_ISP_TSK_JOBSIZE);
		if (kflow_isp_job_statck == NULL) {
			DBG_ERR("kflow_isp_job_statck alloc fail\r\n");
			return E_NOMEM;
		}
		memset(kflow_isp_job_statck, 0x0, sizeof(KFLOW_AI_ISP_JOB) * KFLOW_AI_ISP_TSK_JOBSIZE);

		kflow_isp_push_time  = (UINT32 *)nvt_ai_mem_alloc(sizeof(UINT32) * g_ai_net_max); // record push time of each proc_id
		if (kflow_isp_push_time == NULL) {
				DBG_ERR("kflow_isp_push_time alloc fail\r\n");
				return E_NOMEM;
		}
		memset(kflow_isp_push_time, 0, sizeof(UINT32) * g_ai_net_max);

		kflow_isp_first_proc  = (UINT32 *)nvt_ai_mem_alloc(sizeof(UINT32) * g_ai_net_max); // record whether first frame has applied inputed parameters of each proc_id
		if (kflow_isp_first_proc == NULL) {
				DBG_ERR("kflow_isp_first_proc alloc fail\r\n");
				return E_NOMEM;
		}
		memset(kflow_isp_first_proc, 0, sizeof(UINT32) * g_ai_net_max);

		kflow_isp_enable_3d = (NN_ISP_ENABLE_3D *)nvt_ai_mem_alloc(sizeof(NN_ISP_ENABLE_3D) * g_ai_net_max); // record whether first frame has applied inputed parameters of each proc_id
		if (kflow_isp_enable_3d == NULL) {
				DBG_ERR("kflow_isp_enable_3d alloc fail\r\n");
				return E_NOMEM;
		}
		memset(kflow_isp_enable_3d, 0, sizeof(NN_ISP_ENABLE_3D) * g_ai_net_max);

		g_ai_input_layer_map_num = (UINT32 *)nvt_ai_mem_alloc(sizeof(UINT32) * g_ai_net_max);
		if (g_ai_input_layer_map_num == NULL) {
			DBG_ERR("g_ai_input_layer_map_num alloc fail\r\n");
			return E_NOMEM;
		}
		memset(g_ai_input_layer_map_num, 0x0, sizeof(UINT32) * g_ai_net_max);

		g_ai_input_layer_info_num = (UINT32 *)nvt_ai_mem_alloc(sizeof(UINT32) * g_ai_net_max);
		if (g_ai_input_layer_info_num == NULL) {
			DBG_ERR("g_ai_input_layer_info_num alloc fail\r\n");
			return E_NOMEM;
		}
		memset(g_ai_input_layer_info_num, 0x0, sizeof(UINT32) * g_ai_net_max);

		g_ai_isp_ori_ppu_quan_param = (AI_TUNING_QUAN_PARAM **)nvt_ai_mem_alloc(sizeof(AI_TUNING_QUAN_PARAM*) * g_ai_net_max);
		if (g_ai_isp_ori_ppu_quan_param == NULL) {
			DBG_ERR("g_ai_isp_ori_ppu_quan_param alloc fail\r\n");
			return E_NOMEM;
		}
		memset(g_ai_isp_ori_ppu_quan_param, 0x0, sizeof(AI_TUNING_QUAN_PARAM*) * g_ai_net_max);

		for (i=0; i<g_ai_net_max; i++) {
			g_ai_isp_ori_ppu_quan_param[i] = (AI_TUNING_QUAN_PARAM *)nvt_ai_mem_alloc(sizeof(AI_TUNING_QUAN_PARAM) * 2);  // 2 for curr & sigma
			if (g_ai_isp_ori_ppu_quan_param[i] == NULL) {
				DBG_ERR("g_ai_isp_ori_ppu_quan_param[%u] alloc fail\r\n", i);
				return E_NOMEM;
			}
			memset(g_ai_isp_ori_ppu_quan_param[i], 0x0, sizeof(AI_TUNING_QUAN_PARAM) * 2);  // 2 for curr & sigma
		}

		g_ai_isp_pre_dgain = (UINT8 *)nvt_ai_mem_alloc(sizeof(UINT8) * g_ai_net_max);
		if (g_ai_isp_pre_dgain == NULL) {
			DBG_ERR("g_ai_isp_pre_dgain alloc fail\r\n");
			return E_NOMEM;
		}
		memset(g_ai_isp_pre_dgain, 0x0, sizeof(UINT8) * g_ai_net_max);

		g_ai_input_layer_map_table = (NN_ISP_INPUT **)nvt_ai_mem_alloc(sizeof(NN_ISP_INPUT *) * g_ai_net_max);
		if (g_ai_input_layer_map_table == NULL) {
			DBG_ERR("g_ai_input_layer_map_table alloc fail\r\n");
			return E_NOMEM;
		}
		memset(g_ai_input_layer_map_table, 0x0, sizeof(NN_ISP_INPUT *) * g_ai_net_max);

		g_ai_input_addr_map_table = (uintptr_t **)nvt_ai_mem_alloc(sizeof(uintptr_t *) * g_ai_net_max);
		if (g_ai_input_addr_map_table == NULL) {
			DBG_ERR("g_ai_input_addr_map_table alloc fail\r\n");
			return E_NOMEM;
		}
		memset(g_ai_input_addr_map_table, 0x0, sizeof(uintptr_t *) * g_ai_net_max);

		g_ai_input_layer_info_table = (VENDOR_AIS_ISP_INPUT_INFO **)nvt_ai_mem_alloc(sizeof(VENDOR_AIS_ISP_INPUT_INFO *) * g_ai_net_max);
		if (g_ai_input_layer_info_table == NULL) {
			DBG_ERR("g_ai_input_layer_info_table alloc fail\r\n");
			return E_NOMEM;
		}
		memset(g_ai_input_layer_info_table, 0x0, sizeof(VENDOR_AIS_ISP_INPUT_INFO *) * g_ai_net_max);

		g_ai_isp_p_drv = (KDRV_AI_JMISP_PATH_INFO **)nvt_ai_mem_alloc(sizeof(KDRV_AI_JMISP_PATH_INFO*) * g_ai_net_max);
		if (g_ai_isp_p_drv == NULL) {
			DBG_ERR("g_ai_isp_p_drv alloc fail\r\n");
			return E_NOMEM;
		}
		memset(g_ai_isp_p_drv, 0x0, sizeof(KDRV_AI_JMISP_PATH_INFO*) * g_ai_net_max);

		g_ai_isp_input_addr = (VENDOR_AIS_ISP_MEM_PARM *)nvt_ai_mem_alloc(sizeof(VENDOR_AIS_ISP_MEM_PARM) * g_ai_net_max);
		if (g_ai_isp_input_addr == NULL) {
			DBG_ERR("g_ai_isp_input_addr alloc fail\r\n");
			return E_NOMEM;
		}
		memset(g_ai_isp_input_addr, 0x0, sizeof(VENDOR_AIS_ISP_MEM_PARM) * g_ai_net_max);

		g_ai_isp_inputfeat_pingpong_seg0 = (VENDOR_AIS_ISP_MEM_PARM *)nvt_ai_mem_alloc(sizeof(VENDOR_AIS_ISP_MEM_PARM) * g_ai_net_max);
		if (g_ai_isp_inputfeat_pingpong_seg0 == NULL) {
			DBG_ERR("g_ai_isp_inputfeat_pingpong_seg0 alloc fail\r\n");
			return E_NOMEM;
		}
		memset(g_ai_isp_inputfeat_pingpong_seg0, 0x0, sizeof(VENDOR_AIS_ISP_MEM_PARM) * g_ai_net_max);

		g_ai_isp_inputfeat_pingpong_seg1 = (VENDOR_AIS_ISP_MEM_PARM *)nvt_ai_mem_alloc(sizeof(VENDOR_AIS_ISP_MEM_PARM) * g_ai_net_max);
		if (g_ai_isp_inputfeat_pingpong_seg1 == NULL) {
			DBG_ERR("g_ai_isp_inputfeat_pingpong_seg1 alloc fail\r\n");
			return E_NOMEM;
		}
		memset(g_ai_isp_inputfeat_pingpong_seg1, 0x0, sizeof(VENDOR_AIS_ISP_MEM_PARM) * g_ai_net_max);

		g_ai_isp_parm = (VENDOR_AIS_ISP_PARM *)nvt_ai_mem_alloc(sizeof(VENDOR_AIS_ISP_PARM) * g_ai_net_max);
		if (g_ai_isp_parm == NULL) {
			DBG_ERR("g_ai_isp_parm alloc fail\r\n");
			return E_NOMEM;
		}
		memset(g_ai_isp_parm, 0x0, sizeof(VENDOR_AIS_ISP_PARM) * g_ai_net_max);

		g_ai_jmisp_info = (AI_JMISP_INFO *)nvt_ai_mem_alloc(sizeof(AI_JMISP_INFO) * g_ai_net_max);
		if (g_ai_jmisp_info == NULL) {
			DBG_ERR("g_ai_jmisp_info alloc fail\r\n");
			return E_NOMEM;
		}
		memset(g_ai_jmisp_info, 0x0, sizeof(AI_JMISP_INFO) * g_ai_net_max);

		g_proc_trace = (UINT32 *)nvt_ai_mem_alloc(sizeof(UINT32) * g_ai_net_max);
		if (g_proc_trace == NULL) {
			DBG_ERR("g_proc_trace alloc fail\r\n");
			return E_NOMEM;
		}
		memset(g_proc_trace, 0x0, sizeof(UINT32) * g_ai_net_max);

		kflow_isp_weight_loc_map = (UINT32 *)nvt_ai_mem_alloc(sizeof(UINT32) * g_ai_net_max);
		if (kflow_isp_weight_loc_map == NULL) {
			DBG_ERR("kflow_isp_weight_loc_map alloc fail\r\n");
			return E_NOMEM;
		}
		memset(kflow_isp_weight_loc_map, 0x0, sizeof(UINT32) * g_ai_net_max);

		kflow_ai_isp_tsk_open(); 
		kflow_isp_init = 1 ; 
	}
	return  E_OK ;
}
ER kflow_isp_uninit_jmisp_net(void)
{
	UINT32 i = 0 ; 
	UINT32	g_ai_net_max ; 
	g_ai_net_max = kflow_ai_get_net_supported_num() ; 

	if (kflow_isp_get_max_isp_num(&MAX_ISP_NUM) != E_OK){
		return E_OK;
	}
	if (kflow_isp_init_count > 1) {
        return E_OK;
    }
	if(kflow_isp_init == 1) {

		SEM_DESTROY(g_ai_isp_lock);
		SEM_DESTROY(g_ai_isp_id);
		SEM_DESTROY(g_ai_isp_dgain_lock);
		rel_flg(FLG_ID_JMISP);
		
		if (kflow_isp_proc_pool_map) {
				nvt_ai_mem_free(kflow_isp_proc_pool_map);
				kflow_isp_proc_pool_map = 0;
		}
		if (kflow_isp_mode_map) {
				nvt_ai_mem_free(kflow_isp_mode_map);
				kflow_isp_mode_map = 0;
		}
		if (kflow_isp_sensor_map) {
				nvt_ai_mem_free(kflow_isp_sensor_map);
				kflow_isp_sensor_map = 0;
		}
		if (kflow_isp_proc_core_mask) {
				nvt_ai_mem_free(kflow_isp_proc_core_mask);
				kflow_isp_proc_core_mask = 0;
		}
		if (kflow_isp_proc_ub_mask) {
				nvt_ai_mem_free(kflow_isp_proc_ub_mask);
				kflow_isp_proc_ub_mask = 0;
		}
		if (kflow_isp_nn_info) {
				nvt_ai_mem_free(kflow_isp_nn_info);
				kflow_isp_nn_info = 0;
		}
		if (kflow_isp_ubuf_layout) {
				nvt_ai_mem_free(kflow_isp_ubuf_layout);
				kflow_isp_ubuf_layout = 0;
		}
		if (kflow_isp_drv_para) {
				nvt_ai_mem_free(kflow_isp_drv_para);
				kflow_isp_drv_para = 0;
		}
		if (kflow_isp_job_statck) {
				nvt_ai_mem_free(kflow_isp_job_statck);
				kflow_isp_job_statck = 0;
		}

		if (kflow_isp_push_time) {
				nvt_ai_mem_free(kflow_isp_push_time);
				kflow_isp_push_time = 0;
		}
		if (kflow_isp_first_proc) {
				nvt_ai_mem_free(kflow_isp_first_proc);
				kflow_isp_first_proc = 0;
		}
		if (kflow_isp_enable_3d) {
				nvt_ai_mem_free(kflow_isp_enable_3d);
				kflow_isp_enable_3d = 0;
		}

		if (g_ai_input_layer_map_num) {
				nvt_ai_mem_free(g_ai_input_layer_map_num);
				g_ai_input_layer_map_num = 0;
		}

		if (g_ai_input_layer_info_num) {
				nvt_ai_mem_free(g_ai_input_layer_info_num);
				g_ai_input_layer_info_num = 0;
		}

		if (g_ai_isp_ori_ppu_quan_param) {
			for (i = 0; i < g_ai_net_max; i++) {
				if (g_ai_isp_ori_ppu_quan_param[i]) {
					nvt_ai_mem_free(g_ai_isp_ori_ppu_quan_param[i]);
					g_ai_isp_ori_ppu_quan_param[i] = 0;
				}
			}
			nvt_ai_mem_free(g_ai_isp_ori_ppu_quan_param);
			g_ai_isp_ori_ppu_quan_param = 0;
		}

		if (g_ai_isp_pre_dgain) {
				nvt_ai_mem_free(g_ai_isp_pre_dgain);
				g_ai_isp_pre_dgain = 0;
		}

		for (i = 0; i < g_ai_net_max; i++) {
			if (g_ai_input_layer_map_table[i]) {
				nvt_ai_mem_free(g_ai_input_layer_map_table[i]);
				g_ai_input_layer_map_table[i] = 0;
			}
		}
		if (g_ai_input_layer_map_table) {
				nvt_ai_mem_free(g_ai_input_layer_map_table);
				g_ai_input_layer_map_table = 0;
		}

		for (i = 0; i < g_ai_net_max; i++) {
			if (g_ai_input_addr_map_table[i]) {
				nvt_ai_mem_free(g_ai_input_addr_map_table[i]);
				g_ai_input_addr_map_table[i] = 0;
			}
		}

		if (g_ai_input_addr_map_table) {
				nvt_ai_mem_free(g_ai_input_addr_map_table);
				g_ai_input_addr_map_table = 0;
		}		

		if (g_ai_input_layer_info_table) {
			for (i = 0; i < g_ai_net_max; i++) {
				if (g_ai_input_layer_info_table[i]) {
					nvt_ai_mem_free(g_ai_input_layer_info_table[i]);
					g_ai_input_layer_info_table[i] = 0;
				}
			}
			nvt_ai_mem_free(g_ai_input_layer_info_table);
			g_ai_input_layer_info_table = 0;
		}

		for (i = 0; i < g_ai_net_max; i++) {
			if (g_ai_isp_p_drv[i]) {
				nvt_ai_mem_free(g_ai_isp_p_drv[i]);
				g_ai_isp_p_drv[i] = 0;
			}
		}
		if (g_ai_isp_p_drv) {
				nvt_ai_mem_free(g_ai_isp_p_drv);
				g_ai_isp_p_drv = 0;
		}

		if (g_ai_isp_input_addr) {
				nvt_ai_mem_free(g_ai_isp_input_addr);
				g_ai_isp_input_addr = 0;
		}
		if (g_ai_isp_inputfeat_pingpong_seg0) {
				nvt_ai_mem_free(g_ai_isp_inputfeat_pingpong_seg0);
				g_ai_isp_inputfeat_pingpong_seg0 = 0;
		}
		if (g_ai_isp_inputfeat_pingpong_seg1) {
				nvt_ai_mem_free(g_ai_isp_inputfeat_pingpong_seg1);
				g_ai_isp_inputfeat_pingpong_seg1 = 0;
		}
		if (g_ai_isp_parm) {
				nvt_ai_mem_free(g_ai_isp_parm);
				g_ai_isp_parm = 0;
		}
		if( g_ai_jmisp_info){
				nvt_ai_mem_free(g_ai_jmisp_info);
				g_ai_jmisp_info = 0;
		}
		if (g_proc_trace) {
				nvt_ai_mem_free(g_proc_trace);
				g_proc_trace = 0;
		}
		if (kflow_isp_weight_loc_map) {
				nvt_ai_mem_free(kflow_isp_weight_loc_map);
				kflow_isp_weight_loc_map = 0;
		}
		kflow_ai_isp_tsk_close(); 
		kflow_isp_init = 0 ; 
	}
	return  E_OK ;
}	

ER kflow_isp_alloc_map_table(UINT32 proc_id, UINT32 max_batch)
{	
	g_ai_input_layer_map_table[proc_id] = (NN_ISP_INPUT *)nvt_ai_mem_alloc(sizeof(NN_ISP_INPUT) * max_batch);
	if (g_ai_input_layer_map_table[proc_id] == NULL) {
		DBG_ERR("g_ai_input_layer_map_table[proc_id] alloc fail\r\n");
		return E_NOMEM;
	}
	memset(g_ai_input_layer_map_table[proc_id], 0x0, sizeof(NN_ISP_INPUT) * max_batch);

	g_ai_input_addr_map_table[proc_id] = (uintptr_t *)nvt_ai_mem_alloc(sizeof(uintptr_t) * max_batch);
	if (g_ai_input_addr_map_table[proc_id] == NULL) {
		DBG_ERR("g_ai_input_addr_map_table[proc_id] alloc fail\r\n");
		return E_NOMEM;
	}
	memset(g_ai_input_addr_map_table[proc_id], 0x0, sizeof(uintptr_t) * max_batch);

	g_ai_input_layer_info_table[proc_id] = (VENDOR_AIS_ISP_INPUT_INFO *)nvt_ai_mem_alloc(sizeof(VENDOR_AIS_ISP_INPUT_INFO) * max_batch);
	if (g_ai_input_layer_info_table[proc_id] == NULL) {
		DBG_ERR("g_ai_input_layer_info_table[proc_id] alloc fail\r\n");
		return E_NOMEM;
	}
	memset(g_ai_input_layer_info_table[proc_id], 0x0, sizeof(VENDOR_AIS_ISP_INPUT_INFO) * max_batch);

	g_ai_input_layer_map_num[proc_id] = 0 ; // reset input num
	g_ai_input_layer_info_num[proc_id] = 0 ; // reset input num
	return E_OK;
}
ER  kflow_isp_free_map_table(UINT32 proc_id)
{
	if (g_ai_input_layer_map_table[proc_id]) {
		nvt_ai_mem_free(g_ai_input_layer_map_table[proc_id]);
		g_ai_input_layer_map_table[proc_id] = 0;
	}	
	
	if (g_ai_input_addr_map_table[proc_id]) {
		nvt_ai_mem_free(g_ai_input_addr_map_table[proc_id]);
		g_ai_input_addr_map_table[proc_id] = 0;
	}	

	if (g_ai_input_layer_info_table[proc_id]) {
		nvt_ai_mem_free(g_ai_input_layer_info_table[proc_id]);
		g_ai_input_layer_info_table[proc_id] = 0;
	}

	if (g_ai_isp_p_drv[proc_id]) {
		nvt_ai_mem_free(g_ai_isp_p_drv[proc_id]);
		g_ai_isp_p_drv[proc_id] = 0;
	}
	g_ai_input_layer_map_num[proc_id] = 0 ; 
	g_ai_input_layer_info_num[proc_id] = 0 ;
	kflow_isp_weight_loc_map[proc_id] = 0 ;
	return E_OK;
}
ER kflow_isp_close_jmisp_net(UINT32 net_id)
{
	VENDOR_AIS_ISP_PARM isp_parm = {0} ;
	VENDOR_AIS_ISP_MAP_MEM_PARM p_mem ;

	if(kflow_isp_proc_pool_map[net_id] >= 0 && kflow_isp_used_pool[kflow_isp_proc_pool_map[net_id]] > 0){
		kflow_isp_used_pool[kflow_isp_proc_pool_map[net_id]] -= 1 ; // reset used_pool
	}

	kflow_isp_proc_pool_map[net_id] = -1 ; // reset pool_id
	kflow_isp_proc_core_mask[net_id] = 0 ; // reset core_mask
	kflow_isp_proc_ub_mask[net_id] = 0 ; // reset ub_mask
	kflow_isp_mode_map[net_id] = -1 ; // reset isp mode
	kflow_isp_free_map_table(net_id) ; 
	isp_parm = g_ai_isp_parm[net_id] ; 

	p_mem = isp_parm.p_info.parm;
	if ((p_mem.user_parm.va == 0 )
			|| (p_mem.user_parm.pa == 0 )
		) {
		DBG_WRN("null memory...\r\n");
		return E_CTX;
	}
	if (p_mem.user_parm.va)
		vos_cpu_dcache_sync(p_mem.user_parm.va, p_mem.user_parm.size, VOS_DMA_BIDIRECTIONAL);
	if (p_mem.user_buff.va)
		vos_cpu_dcache_sync(p_mem.user_buff.va, p_mem.user_buff.size, VOS_DMA_BIDIRECTIONAL);
	if (p_mem.tcm_buff.va)
		vos_cpu_dcache_sync(p_mem.tcm_buff.va, p_mem.tcm_buff.size, VOS_DMA_BIDIRECTIONAL);
	if (p_mem.user_model.va)
		vos_cpu_dcache_sync(p_mem.user_model.va, p_mem.user_model.size, VOS_DMA_BIDIRECTIONAL);


	if (p_mem.user_parm.va)
		nvt_ai_pa2va_unmap(p_mem.user_parm.va, p_mem.user_parm.pa);
	
	if (p_mem.user_buff.va)	
		nvt_ai_pa2va_unmap(p_mem.user_buff.va, p_mem.user_buff.pa);

	if (p_mem.tcm_buff.va)	
		nvt_ai_pa2va_unmap(p_mem.tcm_buff.va, p_mem.tcm_buff.pa);

	if (p_mem.user_model.va)	
		nvt_ai_pa2va_unmap(p_mem.user_model.va, p_mem.user_model.pa);	

	memset(&g_ai_isp_parm[net_id], 0x0, sizeof(VENDOR_AIS_ISP_PARM));
	// memset(&g_ai_jmisp_info[net_id], 0x0, sizeof(AI_JMISP_INFO));

	
	return E_OK;
}

ER kflow_isp_reset_jmisp_net(void)
{
	UINT32 j, i;
	UINT32	g_ai_net_max ; 
	g_ai_net_max = kflow_ai_get_net_supported_num() ; 

	if (kflow_isp_init > 0 && kflow_isp_init_count != 0) {
			return E_OK;
	}
	
	if (kflow_isp_init > 0) {
		DBG_DUMP("kflow_isp - init: already init?\r\n");
		DBG_DUMP("kflow_isp - reset - begin\r\n");

		j = kflow_ai_isp_tsk_get_cnt();
		if (j > 0) {
			DBG_DUMP(" <0> drop isp task!\r\n");
			kflow_ai_isp_tsk_reset();
		}
		j = kflow_ai_isp_tsk_get_cnt();
		if (j > 0) {
			DBG_DUMP(" <1> kill isp task!\r\n");
			for (i=0; i < j; i++) {
				kflow_ai_isp_tsk_close();
			}
		}
		for (i=0; i<g_ai_net_max; i++) {
			if (g_ai_isp_parm[i].jmisp_info){
				DBG_DUMP(" <2> net[%d] force close\r\n", i);
				kflow_isp_close_jmisp_net(i) ; 
			}
		}
		DBG_DUMP(" <3> force uninit\r\n");
		kflow_isp_uninit_jmisp_net(); 
		
	}

	return  E_OK ;

}


ER kflow_isp_pars_jmisp_net (VENDOR_AIS_ISP_PARM *isp_parm)
{
	VENDOR_AIS_ISP_MAP_MEM_PARM p_mem ;
	VENDOR_AIS_ISP_MAP_MEM_PARM p_user_mem ;
	UINT32 net_id ; 
	uintptr_t parm_pa_ofs, parm_va_ofs, tcm_pa_ofs, tcm_va_ofs, buff_pa_ofs, buff_va_ofs ;
	UINT32 i = 0, ppu_num = 0;
	ER er = E_OK;
	AI_JMISP_INFO *jmisp_info ; 

	p_mem = isp_parm->p_info.parm;
	if (   (p_mem.user_parm.va == 0 )
		|| (p_mem.user_parm.pa == 0 )
		|| (p_mem.tcm_buff.pa == 0 )
		|| (p_mem.user_buff.pa == 0 )
		) {
		DBG_ERR("null memory...\r\n");
		return E_CTX;
	}
	if (isp_parm->jmisp_info == 0 ){
		DBG_ERR("jmisp_info is null !\r\n");
		return E_CTX;
	}else if (isp_parm->p_pl_cmd_list == 0){
		DBG_ERR("p_pl_cmd_list is null !\r\n");
		return E_CTX;
	}else if (isp_parm->p_ppu_cmd_list == 0){
		DBG_ERR("p_ppu_cmd_list is null !\r\n");
		return E_CTX;
	}else if (isp_parm->jmisp_info_size == 0 ){
		DBG_ERR("jmisp_info_size is null !\r\n");
		return E_CTX;
	}else if (isp_parm->jmisp_pl_size == 0){
		DBG_ERR("jmisp_pl_size is null !\r\n");
		return E_CTX;
	}else if (isp_parm->jmisp_ppu_size == 0){
		DBG_ERR("jmisp_ppu_size is 0 !\r\n");
		return E_CTX;
	}
  
	if (MAX_ISP_NUM == 0 ){
		DBG_ERR("It is a NN-ISP model, Set ai_maxisp (AIISP jobm setting) in nvt-mem-tbl.dtsi before running! \n"); 
		return E_SYS;  
	}
	if (isp_parm->p_info.net_id >= kflow_ai_get_net_supported_num()){
		DBG_ERR(" proc_id[%u] is out of range ! \r\n",isp_parm->p_info.net_id ) ; 
		return E_CTX;
	}
	net_id = isp_parm->p_info.net_id ;
	// set prioity 
	kflow_isp_set_prioity(1) ; 
	g_proc_trace[kflow_isp_kcmd_proc_id] = kflow_isp_kcmd_trace;
	// DBG_DUMP("p_mem.user_parm.va %lx \n", (ULONG)p_mem.user_parm.va) ; 
	// DBG_DUMP("isp_parm->p_pl_cmd_list %lx \n", (ULONG)isp_parm->p_pl_cmd_list) ; 
	// DBG_DUMP("isp_parm->p_ppu_cmd_list %lx \n", (ULONG)isp_parm->p_ppu_cmd_list) ; 
	p_user_mem.user_parm.va = (uintptr_t)nvt_ai_pa2va_remap(p_mem.user_parm.pa, p_mem.user_parm.size);
	p_user_mem.user_buff.va = (uintptr_t)nvt_ai_pa2va_remap(p_mem.user_buff.pa, p_mem.user_buff.size);
	p_user_mem.tcm_buff.va = (uintptr_t)nvt_ai_pa2va_remap(p_mem.tcm_buff.pa, p_mem.tcm_buff.size);
	p_user_mem.user_model.va = (uintptr_t)nvt_ai_pa2va_remap(p_mem.user_model.pa, p_mem.user_model.size);
	isp_parm->jmisp_info = isp_parm->jmisp_info - p_mem.user_parm.va + p_user_mem.user_parm.va; 
	isp_parm->p_pl_cmd_list = isp_parm->p_pl_cmd_list - p_mem.user_parm.va + p_user_mem.user_parm.va;
	isp_parm->p_ppu_cmd_list = isp_parm->p_ppu_cmd_list - p_mem.user_parm.va + p_user_mem.user_parm.va;
	isp_parm->p_ai_parm_list = isp_parm->p_ai_parm_list - p_mem.user_parm.va + p_user_mem.user_parm.va;
	isp_parm->p_info.parm.user_parm.va = p_user_mem.user_parm.va ;
	isp_parm->p_info.parm.user_buff.va = p_user_mem.user_buff.va ;
	isp_parm->p_info.parm.tcm_buff.va = p_user_mem.tcm_buff.va ;
	isp_parm->p_info.parm.user_model.va = p_user_mem.user_model.va ;
	jmisp_info = (AI_JMISP_INFO *)isp_parm->jmisp_info ; 

	parm_pa_ofs = ALIGN_CEIL_4(isp_parm->p_info.parm.user_parm.pa);
	parm_va_ofs = ALIGN_CEIL_4(isp_parm->p_info.parm.user_parm.va) ; 
	buff_pa_ofs = ALIGN_CEIL_4(isp_parm->p_info.parm.user_buff.pa);
	buff_va_ofs = ALIGN_CEIL_4(isp_parm->p_info.parm.user_buff.va) ; 
	
	// DBG_DUMP(" user_parm.va %lx  pa%lx \n", (ULONG)isp_parm->p_info.parm.user_parm.va, (ULONG)isp_parm->p_info.parm.user_parm.pa) ;
	// DBG_DUMP(" user_buff.va %lx  pa%lx \n", (ULONG)isp_parm->p_info.parm.user_buff.va, (ULONG)isp_parm->p_info.parm.user_buff.pa) ;
	// DBG_DUMP(" user_buff.va %lx  pa%lx \n", (ULONG)isp_parm->p_info.parm.user_buff.va, (ULONG)isp_parm->p_info.parm.user_buff.pa) ;
	// DBG_DUMP(" tcm_buff.va %lx  pa%lx \n", (ULONG)isp_parm->p_info.parm.tcm_buff.va, (ULONG)isp_parm->p_info.parm.tcm_buff.pa) ;
	
#if (defined(_BSP_NS02201_) || defined(_BSP_NS02302_) || defined(_BSP_NS02401_))	
	tcm_pa_ofs = ALIGN_CEIL_4(isp_parm->p_info.parm.tcm_buff.pa);
	tcm_va_ofs = ALIGN_CEIL_4(isp_parm->p_info.parm.tcm_buff.va);
#endif	

    DBG_MSG(" JMISP parm_pa_ofs = %lx, tcm_pa_ofs = %lx buff_pa_ofs = %lx model_pa_ofs = %lx \n", (ULONG)parm_pa_ofs, (ULONG)tcm_pa_ofs,(ULONG)buff_pa_ofs, (ULONG)isp_parm->p_info.parm.user_model.pa);


	for (i = 0 ; i < e_AI_IOBUF_NUM ; i ++){
		AI_JMISP_IOBUF_INFO* iobuf = &jmisp_info->iobuf[i] ; 

		if( (jmisp_info->iobuf[e_AI_JMISP_IOBUF_RING_IN0].stripeInfo.output_slice_height == 
	    	 jmisp_info->iobuf[e_AI_JMISP_IOBUF_RING_IN0].frameInfo.frame_height) && (i < e_AI_JMISP_IOBUF_REF_IN0) ){
			// skip initial RING/PINPONG_OUT in frame mode
			continue ;
		}

		if ( e_AI_JMISP_BASE_ADDR_1 == iobuf->addrType && iobuf->buffer_sz > 0 ){
			iobuf->pa = iobuf->pa + buff_pa_ofs;
			iobuf->va = iobuf->va + buff_va_ofs;
		}else if ( e_AI_JMISP_BASE_ADDR_3 == iobuf->addrType && iobuf->buffer_sz > 0 ){	
			iobuf->pa = iobuf->pa + tcm_pa_ofs ; 
			iobuf->va = iobuf->va + tcm_va_ofs ; 
		}else{
			continue ;
		}
		memset((void *)iobuf->va, 0x0, iobuf->buffer_sz) ; 
		vos_cpu_dcache_sync(iobuf->va, iobuf->buffer_sz, VOS_DMA_TO_DEVICE);

        DBG_MSG("After jmisp_info->iobuf[%u] va = %lx pa = %lx \n", i, (ULONG)jmisp_info->iobuf[i].va, (ULONG)jmisp_info->iobuf[i].pa);
		

	}
	memset(&kflow_isp_enable_3d[net_id], 0, sizeof(NN_ISP_ENABLE_3D)); // reset enable_3d

	g_ai_isp_inputfeat_pingpong_seg0[net_id].pa  = jmisp_info->iobuf[e_AI_JMISP_IOBUF_INPUTFEAT_PINGPONG_SEG0].pa ; 
	g_ai_isp_inputfeat_pingpong_seg0[net_id].va  = jmisp_info->iobuf[e_AI_JMISP_IOBUF_INPUTFEAT_PINGPONG_SEG0].va  ;
	g_ai_isp_inputfeat_pingpong_seg0[net_id].size  = jmisp_info->iobuf[e_AI_JMISP_IOBUF_INPUTFEAT_PINGPONG_SEG0].buffer_sz ;
	memset((void *)g_ai_isp_inputfeat_pingpong_seg0[net_id].va, 0x0, g_ai_isp_inputfeat_pingpong_seg0[net_id].size) ; 
	
	g_ai_isp_inputfeat_pingpong_seg1[net_id].pa  = jmisp_info->iobuf[e_AI_JMISP_IOBUF_INPUTFEAT_PINGPONG_SEG1].pa  ; 
	g_ai_isp_inputfeat_pingpong_seg1[net_id].va  = jmisp_info->iobuf[e_AI_JMISP_IOBUF_INPUTFEAT_PINGPONG_SEG1].va  ;
	g_ai_isp_inputfeat_pingpong_seg1[net_id].size  = jmisp_info->iobuf[e_AI_JMISP_IOBUF_INPUTFEAT_PINGPONG_SEG1].buffer_sz ;
	memset((void *)g_ai_isp_inputfeat_pingpong_seg1[net_id].va, 0x0, g_ai_isp_inputfeat_pingpong_seg1[net_id].size) ; 

	memcpy(&g_ai_isp_input_addr[net_id], &g_ai_isp_inputfeat_pingpong_seg0[net_id], sizeof(VENDOR_AIS_ISP_MEM_PARM)) ; 
	
	kflow_isp_fix_jmisp_addr(isp_parm->p_pl_cmd_list, parm_pa_ofs, isp_parm->jmisp_pl_size) ;

	ppu_num = get_ppu_num(jmisp_info);
	kflow_isp_fix_ppu_addr(isp_parm->p_ppu_cmd_list, parm_va_ofs, parm_pa_ofs, isp_parm->jmisp_ppu_size, ppu_num);
	
	g_ai_isp_parm[net_id] = *isp_parm ; 
	
	if (isp_parm->input_num > 0 ){
		er = kflow_isp_alloc_map_table(net_id, isp_parm->input_num); 
		if (er != E_OK) {
			DBG_ERR("kflow_isp_alloc_map_table fail...\r\n");
			return E_NOMEM;
		}
	}

	// get CURR ppu quan param
	if (jmisp_info->iobuf[e_AI_JMISP_IOBUF_RING_IN0].buffer_sz > 0) {
		g_ai_isp_ori_ppu_quan_param[net_id][0].mctrl_id = jmisp_info->iobuf[e_AI_JMISP_IOBUF_RING_IN0].match_ppu_id; // [0] for CURR
		kflow_isp_get_ppu_quan_out_scale_shift(net_id, &g_ai_isp_ori_ppu_quan_param[net_id][0]); // [0] for CURR
	}
	// get SIGMA ppu quan param
	if (jmisp_info->iobuf[e_AI_JMISP_IOBUF_SIGMA_IN].buffer_sz > 0) {
		g_ai_isp_ori_ppu_quan_param[net_id][1].mctrl_id = jmisp_info->iobuf[e_AI_JMISP_IOBUF_SIGMA_IN].match_ppu_id; // [1] for SIGMA
		kflow_isp_get_ppu_quan_out_scale_shift(net_id, &g_ai_isp_ori_ppu_quan_param[net_id][1]); // [1] for SIGMA
	}

	return er;
}

ER kflow_isp_set_input (VENDOR_AIS_ISP_INPUT_INFO *p_isp_input_info)
{ 
	UINT32 input_num ;
	NN_ISP_INPUT isp_input = {0} ; 
	UINT32 net_id = p_isp_input_info->net_id;
	NN_DATA_V30 *p_imem = p_isp_input_info->imem;

	if (p_imem[0].pa == 0 ){
		DBG_ERR("input pa is null !\r\n");
		return E_CTX;
	}else if (p_isp_input_info->parm_addr == 0){
		DBG_ERR("mctrl parm_addr is null !\r\n");
		return E_CTX;
	}

	SEM_WAIT(g_ai_isp_lock);
	input_num = g_ai_isp_parm[net_id].input_num ; 

	if(g_ai_input_layer_map_num[net_id] >= input_num){
		// set input num > model input, should be reset 
		g_ai_input_layer_map_num[net_id] = 0 ;
		if (g_ai_input_addr_map_table[net_id][0] == g_ai_isp_inputfeat_pingpong_seg0[net_id].pa)
			memcpy(&g_ai_isp_input_addr[net_id], &g_ai_isp_inputfeat_pingpong_seg0[net_id], sizeof(VENDOR_AIS_ISP_MEM_PARM)) ; 
		else
			memcpy(&g_ai_isp_input_addr[net_id], &g_ai_isp_inputfeat_pingpong_seg1[net_id], sizeof(VENDOR_AIS_ISP_MEM_PARM)) ; 
	}
	isp_input.parm_addr   = p_isp_input_info->parm_addr;
	isp_input.in_buff_ofs = p_isp_input_info->in_buff_ofs;
	isp_input.eng_type    = p_isp_input_info->eng_type;
	isp_input.mctrl_id    = (UINT32)p_isp_input_info->mctrl_id;
	isp_input.port_id     = (UINT32)p_isp_input_info->port_id;

	g_ai_input_layer_map_table[net_id][g_ai_input_layer_map_num[net_id]] = isp_input;
	g_ai_input_addr_map_table[net_id][g_ai_input_layer_map_num[net_id]] = g_ai_isp_input_addr[net_id].pa ;
#if NNISP_DEBUG
	DBG_DUMP("g_ai_input_addr_map_table[%u][%u] = %lx \n", net_id, g_ai_input_layer_map_num[net_id], g_ai_isp_input_addr[net_id].pa) ;
#endif	
	g_ai_input_layer_map_num[net_id]++;
	p_imem[0].va = nvt_ai_pa2va_remap_wo_sync(p_imem[0].pa, p_imem[0].size);
	
	if(p_imem[1].va  && p_imem[1].pa){
		// if it is design-in model, layer name should be passed
		p_imem[1].va = nvt_ai_pa2va_remap_wo_sync(p_imem[1].pa, p_imem[1].size);
	}

	{
		UINT32 val ;
		kflow_isp_net_trace(net_id, AIISP_INPUT, "set() - isp input pa(%lx) va(%lx) size(%u) \r\n", p_imem[0].pa, p_imem[0].va, p_imem[0].size );
		val = (p_imem[0].size == 1)? *(UINT8*) p_imem[0].va: \
		      (p_imem[0].size == 2)? *(UINT16*)p_imem[0].va: \
		      (p_imem[0].size == 4)? *(UINT32*)p_imem[0].va:0;

		if (p_imem[1].va){
			CHAR *name = (CHAR *)p_imem[1].va;
			kflow_isp_net_trace(net_id, AIISP_INPUT, "set() - val = %u name %s\r\n", val, name);
		}else {
			kflow_isp_net_trace(net_id, AIISP_INPUT, "set() - val = %u \r\n",  val);
		}
	}

	if(p_imem[1].va  && p_imem[1].pa){
		// if it is design-in model
		// if layer name == enable_3d, record the enable_3d of the first_proc & second proc 
		                                                                     // when switch iso, don't need set input, directly apply the values of last round (second proc)
		if ((kflow_isp_first_proc[net_id]==0 || kflow_isp_first_proc[net_id]==2) && strcmp((CHAR *)p_imem[1].va, "enable_3d")==0) {
			// record the enable_3d value and effect parameter addr in inputfeat_pingpong for second proc restore
			kflow_isp_enable_3d[net_id].enable_3d_val = *((UINT8*)p_imem[0].va) ;
			kflow_isp_enable_3d[net_id].isp_input_addr.pa = g_ai_isp_input_addr[net_id].pa ;
			kflow_isp_enable_3d[net_id].isp_input_addr.va = g_ai_isp_input_addr[net_id].va ;
			kflow_isp_enable_3d[net_id].isp_input_addr.size = sizeof(UINT8) ;
			// DBG_DUMP("proc-id(%u) set() - kflow_isp_first_proc[net_id] %u isp_input_addr.pa %lx to 0 \r\n",net_id, kflow_isp_first_proc[net_id],(ULONG)kflow_isp_enable_3d[net_id].isp_input_addr.pa );
			kflow_isp_net_trace(net_id, AIISP_INPUT, "set() - store enable_3d(%u) value\r\n", *((UINT8*)p_imem[0].va));
			kflow_isp_net_trace(net_id, AIISP_INPUT, "set() - first frame force reset enable_3d to val = 0\r\n");
		}
	}

	memset((VOID *)g_ai_isp_input_addr[net_id].va, 0, ALIGN_CEIL_4(p_imem[0].size)); // memset buffer range = 0 (so later we can print value as UINT32, even if it's UINT16/UINT8)
	memcpy((VOID *)g_ai_isp_input_addr[net_id].va, (VOID *)p_imem[0].va, p_imem[0].size); // only copy available range, keep high-byte-addr as 0

	g_ai_isp_input_addr[net_id].va += ALIGN_CEIL_4(p_imem[0].size); 
	g_ai_isp_input_addr[net_id].pa += ALIGN_CEIL_4(p_imem[0].size); 
	if(g_ai_isp_input_addr[net_id].pa > (g_ai_input_addr_map_table[net_id][0] + g_ai_isp_input_addr[net_id].size)){
		DBG_ERR("kflow_isp_set_input fail ! %u inputs is more than isp model inputs' num...\r\n", g_ai_input_layer_map_num[net_id] );
		SEM_SIGNAL(g_ai_isp_lock);
		return E_OK;
	}

	nvt_ai_pa2va_unmap(p_imem[0].va, p_imem[0].pa);
	if(p_imem[1].va  && p_imem[1].pa){
		nvt_ai_pa2va_unmap(p_imem[1].va, p_imem[1].pa);
	}
	SEM_SIGNAL(g_ai_isp_lock);

	return E_OK;
}

ER kflow_isp_set_input_info (VENDOR_AIS_ISP_INPUT_INFO *p_isp_input_info)
{
	UINT32 net_id = p_isp_input_info->net_id;
	UINT32 input_num = g_ai_isp_parm[net_id].input_num;
#if 0
	DBG_DUMP("========  kflow_isp_set_input_info  ========\r\n");
	DBG_DUMP("mctrl_id     = %d\r\n", (int)p_isp_input_info->mctrl_id);
	DBG_DUMP("port_id      = %d\r\n", (int)p_isp_input_info->port_id);
	DBG_DUMP("eng_type     = %d\r\n", (int)p_isp_input_info->eng_type);
	DBG_DUMP("ref_frame_id = %d\r\n", (int)p_isp_input_info->ref_frame_id);
	DBG_DUMP("parm_addr    = 0x%016lx\r\n", p_isp_input_info->parm_addr);
	DBG_DUMP("in_buff_ofs  = %d\r\n", (int)p_isp_input_info->in_buff_ofs);
	DBG_DUMP("net_id       = %d\r\n", (int)p_isp_input_info->net_id);
	DBG_DUMP("imem[0].size = %d\r\n", (int)p_isp_input_info->imem[0].size);
#endif
	if(g_ai_input_layer_info_num[net_id] >= input_num){
		// set input num > model input, should be reset
		g_ai_input_layer_info_num[net_id] = 0 ;
	}
	memcpy(&g_ai_input_layer_info_table[net_id][g_ai_input_layer_info_num[net_id]], p_isp_input_info, sizeof(VENDOR_AIS_ISP_INPUT_INFO));
	g_ai_input_layer_info_num[net_id]++;
	return E_OK;
}

ER kflow_isp_assign_input_addr(NN_ISP_INPUT* isp_input, uintptr_t input_addr, uintptr_t parm_va_ofs)
{
	uintptr_t  sai_addr = 0;
	CONV_LL_PARM_SMALL *p_parm ; 
	
	p_parm = (CONV_LL_PARM_SMALL *)(isp_input->parm_addr + parm_va_ofs);
	sai_addr = input_addr + isp_input->in_buff_ofs  ;
#if NNISP_DEBUG					
	DBG_DUMP("CONV input address(%#lx) \r\n", sai_addr);
#endif					
	p_parm->input.bit.addr     = GET_LSB(sai_addr);
	p_parm->input_msb.bit.addr = GET_MSB(sai_addr);
#if defined(_BSP_NS02401_)
	if ((p_parm->input_msb.bit.addr >= 0x100)) {
		DBG_ERR("CONV input address(%#lx) is over 32+8 bits.\r\n", sai_addr);
		return E_MACV;
	}
#else
	if ((p_parm->input_msb.bit.addr >= 0x10)) {
		DBG_ERR("CONV input address(%#lx) is over 32+4 bits.\r\n", sai_addr);
		return E_MACV;
	}
#endif
	vos_cpu_dcache_sync((uintptr_t)p_parm, sizeof(CONV_LL_PARM_SMALL), VOS_DMA_TO_DEVICE);

	return E_OK;
}

ER kflow_isp_assign_input_addr_pou(NN_ISP_INPUT* isp_input, uintptr_t input_addr, AI_JMISP_INFO* p_info, uintptr_t ppu_addr, UINT32 net_id) {
	ER er = E_OK;

	UINT8 _get_u8_value_for_pa(uintptr_t pa) {
		UINT8 value=0;
		uintptr_t tmp_va = nvt_ai_pa2va_remap(pa, sizeof(UINT8));
		value = *(UINT8*)tmp_va;
		nvt_ai_pa2va_unmap(tmp_va, pa);
		return value;
	}

	// TARGET => user will set input for POU for two times(but it's one-time-trigger POU), with the same mctrl's port[3] and port[4]
	//           we have to concat both port[3](MOTION_STR) and port[4](STILL_STR) setting => then call update funciton at one time
	//           but we can't know if user set port[3] or port[4] first, it can be any order. So we have to handle here ...

	//=== if this is port[4] => skip and return ===
	if (isp_input->port_id == 4) return E_OK;

	//=== if this is port[3] => search all input for (same mctrl)'s port[4] , concat those 2 setting , call update ===
	if (isp_input->port_id == 3) {
		UINT32 input_num = g_ai_isp_parm[net_id].input_num ;
		NN_ISP_INPUT *tmp_isp_input=NULL;
		AI_TUNING_PARA upd_pou_tuning_param = {0};
		UINT32 i=0;
		uintptr_t param_pa=0;
		UINT8 motion_str=0, still_str=0;

		// port[3] for motion_str
		motion_str = _get_u8_value_for_pa(input_addr);

		// search all input, find (same mctrl)'s port[4] for still_str
		for(i = 0 ; i < input_num; i++){
			tmp_isp_input = &g_ai_input_layer_map_table[net_id][i];
			if ((tmp_isp_input->mctrl_id == isp_input->mctrl_id) && (tmp_isp_input->port_id == 4)) {
				param_pa = g_ai_input_addr_map_table[net_id][i];
				still_str = _get_u8_value_for_pa(param_pa);

				// concat setting
				upd_pou_tuning_param.tuning_para = (int32_t)((((UINT32)still_str) << 16) | motion_str);
				upd_pou_tuning_param.mctrl_id    = (int32_t)isp_input->mctrl_id;
#if NNISP_DEBUG
				DBG_DUMP("POU update for mctrl(%u) still_str(%u) motion_str(%u) tuning_param(0x%08x)\r\n", isp_input->mctrl_id, still_str, motion_str, (unsigned int)upd_pou_tuning_param.tuning_para);
#endif
				// update POU reg
				er = change_pou_inputfeat_seg(p_info, ppu_addr, &upd_pou_tuning_param);
				if (E_OK != er) {
					DBG_ERR("change_pou_inputfeat_seg failed ...\r\n");
				}
				return er;
			}
		}

		DBG_ERR("fail to find mctrl(%u)'s port[4] input .... !!\r\n", isp_input->mctrl_id);
		return E_SYS;
	}

	DBG_ERR("POU update for mctrl(%u) port(%u) is illegal, port should be 3 or 4 !!\r\n", isp_input->mctrl_id, isp_input->port_id);
	return E_SYS;
}

ER kflow_isp_assign_all_input_addr (UINT32 net_id)
{
	UINT32 input_num, i, j; 
	NN_ISP_INPUT isp_input ;
	ER er = E_OK;
	uintptr_t addr  ;
	UINT8* enable_3d ;

	kflow_isp_net_trace(net_id, AIISP_INPUT, "set() - assign all isp inputs \r\n" );

	if (g_ai_isp_parm[net_id].jmisp_info == 0){
		DBG_WRN(" proc_id[%u] is not started  \n", net_id);
		return -2;
	}

	input_num = g_ai_isp_parm[net_id].input_num ; 
	//model don't have input => no need to update input addr 
	if (input_num == 0 ){
		DBG_WRN("model don't have input ! \n");
		return E_OK;
	}

	SEM_WAIT(g_ai_isp_lock);

	if(g_ai_input_addr_map_table[net_id][input_num -1] == 0){
		DBG_ERR("Please set isp model input ! \r\n");
		SEM_SIGNAL(g_ai_isp_lock);
		return -1 ; 
	}
	if(g_ai_input_layer_map_num[net_id] > input_num){
		g_ai_input_layer_map_num[net_id] = 0; // reset input num for next input setting 
		DBG_ERR("kflow_isp_set_input fail ! inputs is more than isp model inputs' num %u...\r\n", input_num);
		SEM_SIGNAL(g_ai_isp_lock);
		return -1;
	}
	
	// If set input num != input_num => no need to update, usr is still setting 
	if(g_ai_input_layer_map_num[net_id] < input_num ){
		if (g_ai_input_layer_map_num[net_id] >  0)
			DBG_WRN("Have set %u input, please continue ! \n", g_ai_input_layer_map_num[net_id]);
		// g_ai_input_layer_map_num[net_id]== 0 => using old values, print vlaues
		goto exit ;
	}

	if (kflow_isp_first_proc[net_id] == 0){
		//  force to set enable_3d = 0 in first proc 
		if(kflow_isp_enable_3d[net_id].isp_input_addr.va){
			// if it is design-in model
			enable_3d = (UINT8*)kflow_isp_enable_3d[net_id].isp_input_addr.va;
			*enable_3d = 0 ;
			// DBG_DUMP("proc-id(%u) set() - isp_input_addr.pa %lx to 0 \r\n",net_id, (ULONG)kflow_isp_enable_3d[net_id].isp_input_addr.pa );
		}
	}

	// If (set input num == input_num) && (model has input) => update input addr 
	// update POU reg first (MUST before CONV)
	for(i = 0 ; i < input_num; i++){
		// NOTE : update POU need full input information, g_ai_input_layer_map_num[net_id] should be full number here for POU to search.
		addr = g_ai_input_addr_map_table[net_id][i] ;
		isp_input = g_ai_input_layer_map_table[net_id][i];
		if (isp_input.eng_type == NN_GEN_ENG_POU) {
			er = kflow_isp_assign_input_addr_pou(&isp_input, addr, &g_ai_jmisp_info[net_id], (uintptr_t)g_ai_isp_parm[net_id].p_ppu_cmd_list, net_id);
			if (er != E_OK) {
				DBG_ERR("kflow_isp_assign_input_addr(POU) fail...\r\n");
				SEM_SIGNAL(g_ai_isp_lock);
				return er;
			}
		}
	}

	// update CONV reg
	for(i = 0 ; i < input_num; i++){
		g_ai_input_layer_map_num[net_id] -- ; // this will minus g_ai_input_layer_map_num, so CONV MUST update after POU
		addr = g_ai_input_addr_map_table[net_id][g_ai_input_layer_map_num[net_id]] ;
		isp_input = g_ai_input_layer_map_table[net_id][g_ai_input_layer_map_num[net_id]];
#if NNISP_DEBUG		
		DBG_DUMP("Set isp input input %u = %lx\n", i, (ULONG)addr);
#endif
		if (isp_input.eng_type == NN_GEN_ENG_CONV) {
			if(g_ai_isp_parm[net_id].pre_in0_en){
				// if pre_in0_en => need to update all llcmd addr
				for (j=0; j < g_ai_isp_parm[net_id].core_num; j++){
					er = kflow_isp_assign_input_addr(&isp_input, addr, g_ai_isp_parm[net_id].p_ai_parm_list + j * g_ai_isp_parm[net_id].each_ai_parm_size);
					if (er != E_OK) {
						DBG_ERR("kflow_isp_assign_input_addr fail...\r\n");
						SEM_SIGNAL(g_ai_isp_lock);
						return er;
					}
				}
			}else {
				er = kflow_isp_assign_input_addr(&isp_input, addr, g_ai_isp_parm[net_id].p_info.parm.user_parm.va);
				if (er != E_OK) {
					DBG_ERR("kflow_isp_assign_input_addr fail...\r\n");
					SEM_SIGNAL(g_ai_isp_lock);
					return er;
				}
			}
		}
	}

	if (g_ai_input_addr_map_table[net_id][0] == g_ai_isp_inputfeat_pingpong_seg0[net_id].pa){
		vos_cpu_dcache_sync((g_ai_isp_inputfeat_pingpong_seg0[net_id].va), (UINT32)(g_ai_isp_inputfeat_pingpong_seg0[net_id].size), VOS_DMA_TO_DEVICE);
		memcpy(&g_ai_isp_input_addr[net_id], &g_ai_isp_inputfeat_pingpong_seg1[net_id], sizeof(VENDOR_AIS_ISP_MEM_PARM)) ; 
	}else{
		vos_cpu_dcache_sync((g_ai_isp_inputfeat_pingpong_seg1[net_id].va), (UINT32)(g_ai_isp_inputfeat_pingpong_seg1[net_id].size), VOS_DMA_TO_DEVICE);
		memcpy(&g_ai_isp_input_addr[net_id], &g_ai_isp_inputfeat_pingpong_seg0[net_id], sizeof(VENDOR_AIS_ISP_MEM_PARM)) ; 
	}
exit: 
	if(kflow_isp_first_proc[net_id] < 2 )
		kflow_isp_first_proc[net_id] ++ ; // 0: first proc inputed parameters hasn't been asigned 
										  // 1: first proc inputed parameters has been asigned 
										  // 2: second proc inputed  has been asigned 
	// DBG_DUMP("------------kflow_isp_first_proc[%u] %u-------------------\n ",net_id, kflow_isp_first_proc[net_id]) ;
	kflow_isp_net_trace(net_id, AIISP_FLOW, "set() - kflow_isp_first_proc[%u] %u \r\n", net_id, kflow_isp_first_proc[net_id] );

	{
		uint32_t j=0;
		UINT32* p_val; // inputfeat will ALIGN_CEIL_4 , so cast UINT32*
		if (g_ai_input_addr_map_table[net_id][0] == g_ai_isp_inputfeat_pingpong_seg0[net_id].pa){
			kflow_isp_net_trace(net_id, AIISP_FLOW, "set() - INPUTFEAT_PINGPONG_SEG0 pa(%lx) \r\n",(ULONG)g_ai_isp_inputfeat_pingpong_seg0[net_id].pa );
			for (j=0; j<input_num; ++j) {
				p_val = (UINT32*)g_ai_isp_inputfeat_pingpong_seg0[net_id].va;
				kflow_isp_net_trace(net_id, AIISP_FLOW, "set() - val[%u] = %u \r\n", j, p_val[j]);
			}
		} else {
			kflow_isp_net_trace(net_id, AIISP_FLOW, "set() - INPUTFEAT_PINGPONG_SEG1 pa(%lx) \r\n",(ULONG)g_ai_isp_inputfeat_pingpong_seg1[net_id].pa );
			for (j=0; j<input_num; ++j) {
				p_val = (UINT32*)g_ai_isp_inputfeat_pingpong_seg1[net_id].va;
				kflow_isp_net_trace(net_id, AIISP_FLOW, "set() - val[%u] = %u \r\n", j, p_val[j]);
			}
		}
	}

	SEM_SIGNAL(g_ai_isp_lock);
	return E_OK;
}

ER kflow_isp_get_ppu_quan_out_scale_shift(UINT32 proc_id, AI_TUNING_QUAN_PARAM *p_ppu_quan_param)
{
	return get_ppu_quan_param((AI_JMISP_INFO *)g_ai_isp_parm[proc_id].jmisp_info, (uintptr_t)g_ai_isp_parm[proc_id].p_ppu_cmd_list, p_ppu_quan_param);
}

ER kflow_isp_set_ppu_quan_out_scale_shift(UINT32 proc_id, AI_TUNING_QUAN_PARAM *p_ppu_quan_param)
{
	return set_ppu_quan_param((AI_JMISP_INFO *)g_ai_isp_parm[proc_id].jmisp_info, (uintptr_t)g_ai_isp_parm[proc_id].p_ppu_cmd_list, p_ppu_quan_param);
}

ER kflow_isp_cal_gain(const AI_TUNING_QUAN_PARAM* i_para, AI_TUNING_QUAN_PARAM* o_para, const AI_FLOAT_FORMAT gain)
{
	o_para->mctrl_id = i_para->mctrl_id;      // update mctrl_id
	return update_gain(i_para, o_para, gain); // update quan_out[3]
}


#endif

ER kflow_isp_set_weight_loc(VENDOR_AIS_ISP_WEIGHT_LOC * loc)
{
	UINT32 proc_id = loc->proc_id ;
	INT32 pool_id = kflow_isp_proc_pool_map[proc_id] ; 

	if(pool_id < 0 ){
		if (kflow_isp_proc_core_mask[proc_id]){
			// using core_mask => force put weight in dram
			return E_OK;
		}
		
		DBG_ERR("proc_id(%u) is not binding with any pool !\r\n", proc_id);
		return E_CTX;
	}else{
		// check whether pool is valid 
		if (pool_id > 3){
			DBG_ERR("pool_%u is illegal ! \n", pool_id);
			return E_CTX ; 
		}
	}
	kflow_isp_weight_loc_map[proc_id] = loc->weight_loc ; 

	if(kflow_isp_pool[pool_id][0] == 2){
		// if UB is shared, weight couldn't move to UB 
		kflow_isp_weight_loc_map[proc_id] = 0 ;
	}

	kflow_isp_net_trace(proc_id, AIISP_BUF, "kflow_isp_weight_loc_map[%u] = %u  \n", proc_id, kflow_isp_weight_loc_map[proc_id]);
	return E_OK;
}
ER kflow_isp_set_default_pool_info(VENDOR_AIS_ISP_POOL_INFO * pool_info)
{
	UINT32 i ;
	UINT32 proc_id = pool_info->proc_id ;
	UINT32 core_mask = 0, ub_mask = 0; 

	kflow_isp_get_max_ubuf(&MAX_ISP_UB);

	// check whether UB is enough
	if (pool_info->need_ubuf_size > 0  && pool_info->need_ubuf_size > MAX_ISP_UB){
		DBG_ERR("ai_maxubuf(%u) is not enough for proc_id(%u) UB(%u)! \n", MAX_ISP_UB, proc_id, pool_info->need_ubuf_size);
		return E_CTX ; 
	}
	// check whether cores is enough
	if (pool_info->need_core_num > MAX_ISP_NUM){
		DBG_ERR("ai_maxisp(%u) is not enough for proc_id(%u) core num(%u)! \n", MAX_ISP_NUM, proc_id, pool_info->need_core_num);
		return E_CTX ; 
	}
	
	for (i = 0  ; i < pool_info->need_core_num ; i ++){
        core_mask |= (1L << (i)) ;
    }
	for (i = 0  ; i < pool_info->need_ubuf_size ; i ++){
        ub_mask |= (1L << (i)) ;
    }
	kflow_isp_proc_core_mask[proc_id] = core_mask ;
	kflow_isp_proc_ub_mask[proc_id] = ub_mask ;
	kflow_isp_net_trace(proc_id, AIISP_FLOW, "set() - default_pool_info core_mask(0x%02x) ub_mask(0x%02x)\r\n", core_mask, ub_mask);
	return E_OK;
}
ER kflow_isp_set_core_mask(VENDOR_AIS_ISP_MASK* mask_info)
{
	UINT32 proc_id = mask_info->proc_id, i;
	UINT32 core_mask = mask_info->mask; 
	for (i = 0  ; i < MAX_ISP_NUM ; i ++){
        core_mask >>= 1 ;
    }
	if (core_mask){
		DBG_ERR("proc_id(%u) AIISP set() - core_mask(0x%02x) is exceed the ai_maxisp(%u) boundary ! \n", proc_id, mask_info->mask, MAX_ISP_NUM);
		return E_SYS;
	}
	kflow_isp_proc_core_mask[proc_id] =  mask_info->mask ;
	kflow_isp_net_trace(proc_id, AIISP_FLOW, "set() - core_mask(0x%02x)\r\n", kflow_isp_proc_core_mask[proc_id]);
	return E_OK;
}
ER kflow_isp_set_ub_mask(VENDOR_AIS_ISP_MASK* mask_info)
{
	UINT32 proc_id = mask_info->proc_id, i ;
	UINT32 ub_mask = mask_info->mask; 
	for (i = 0  ; i < MAX_ISP_UB ; i ++){
        ub_mask >>= 1 ;
    }
	if (ub_mask){
		DBG_ERR("proc_id(%u) AIISP set() - ub_mask(0x%02x) is exceed the ai_maxubuf(%u) boundary ! \n", proc_id, mask_info->mask, MAX_ISP_UB);
		return E_SYS;
	}
	kflow_isp_proc_ub_mask[proc_id] = mask_info->mask ;
	kflow_isp_net_trace(proc_id, AIISP_FLOW, "set() - ub_mask(0x%02x)\r\n", kflow_isp_proc_ub_mask[proc_id]);
	return E_OK;
}
ER kflow_isp_set_pool_info(VENDOR_AIS_ISP_POOL_INFO * pool_info)
{
	UINT32 i ;
	UINT32 pool_id = pool_info->pool_id;
	UINT32 proc_id = pool_info->proc_id ;
	// check whether ai_maxisp dtsi is valid 
	if (kflow_isp_get_max_isp_num(&MAX_ISP_NUM) != E_OK){
		DBG_ERR("can't find \"ai_maxisp\" at dtsi or ai_maxisp is 0\r\n");
		return E_CTX;
	}	

	if(kflow_isp_init == 0) {
		DBG_ERR("kflow_isp.ko is not init fail \r\n");
		return E_CTX; 
	}
	// check whether pool dtsi is valid 
	if (kflow_isp_get_ai_pool() != E_OK){
			DBG_ERR("kflow_isp_get_ai_pool fail \r\n");
			return E_CTX; 
	}
	// check whether pool is valid 
	if (kflow_isp_used_pool[pool_id] < 0){
		DBG_ERR("pool_%u is not defined in dtsi ! \n", pool_id);
		return E_CTX ; 
	}

	// check whether pool is valid 
	if (pool_id > 3){
		DBG_ERR("pool_%u is illegal ! \n", pool_id);
		return E_CTX ; 
	}

	// check pool UB status 
	if (pool_info->need_ubuf_size > 0 ){
		if(kflow_isp_pool[pool_id][0] == 0){
			DBG_ERR("proc_id(%u) needs UB(%u), but pool_%u UB status is off ! \n", proc_id, pool_info->need_ubuf_size, pool_id);
			return E_CTX ;
		}
		if(kflow_isp_pool[pool_id][0] == 3){
			DBG_ERR("proc_id(%u) needs UB(%u), but pool_%u UB status is 3 (for gblktile) ! \n", proc_id, pool_info->need_ubuf_size, pool_id);
			return E_CTX ;
		}
	}
	// check whether pool UB is enough
	if (pool_info->need_ubuf_size > 0  && pool_info->need_ubuf_size > (kflow_isp_pool[pool_id][2] - kflow_isp_pool[pool_id][1] + 1)){
		DBG_ERR("pool_%u [UB st_id(%u)] [UB end_id(%u)] is not enough for proc_id(%u) UB(%u)! \n", pool_id, kflow_isp_pool[pool_id][1], kflow_isp_pool[pool_id][2], proc_id, pool_info->need_ubuf_size);
		return E_CTX ; 
	}
	// check whether pool cores is enough
	if (pool_info->need_core_num > (kflow_isp_pool[pool_id][4] - kflow_isp_pool[pool_id][3] + 1)){
		DBG_ERR("pool_%u [Core st_ch(%u)] [Core end_ch(%u)] is not enough for proc_id(%u) core num(%u)! \n", pool_id, kflow_isp_pool[pool_id][3], kflow_isp_pool[pool_id][4], proc_id, pool_info->need_core_num);
		return E_CTX ; 
	}
	if(pool_info->IsShare_workbuf != 0 && kflow_isp_pool[pool_id][0] != 2){
		DBG_ERR("In order to apply isp_mode %u, pool_%u [[UB status](%u)] should be 2:shared ! \n", pool_info->IsShare_workbuf, pool_id, kflow_isp_pool[pool_id][0]);
		return E_CTX ; 
	}
	// check whether pool is used 
	if (kflow_isp_used_pool[pool_id] > 0 && pool_info->IsShare_workbuf == 0){
		for (i=0 ; i < kflow_ai_get_net_supported_num(); i++){
			if (kflow_isp_proc_pool_map[i] == pool_id){
				DBG_ERR("pool_%u is used by proc_id(%u) ! \n", pool_id, i) ;
			}
		}
		return E_CTX ; 
	}
	kflow_isp_mode_map[proc_id] = pool_info->IsShare_workbuf ;
	kflow_isp_proc_pool_map[proc_id] = pool_id ;
	kflow_isp_used_pool[pool_id] += 1 ;
	kflow_isp_net_trace(proc_id, AIISP_FLOW, "set() - pool_id = %u \r\n", pool_id );
	return E_OK;
}

ER kflow_isp_get_proc_id_by_pool(VENDOR_AIS_ISP_POOL_INFO* pool_info)
{
	UINT32 i ;
	INT32 pool_id = pool_info->pool_id;

	
	if(pool_id < 0 || pool_id > 3){
		DBG_ERR("pool_id %d is illegal !\r\n", pool_id);
		return E_SYS;
	}
	// check whether pool dtsi is valid 
	if (kflow_isp_get_ai_pool() != E_OK){
			DBG_ERR("kflow_isp_get_ai_pool fail \r\n");
			return E_CTX; 
	}
	// check whether pool is valid
	if (kflow_isp_used_pool[pool_id] < 0){
		DBG_ERR("pool_%u is not defined in dtsi ! \n", pool_id);
		return E_CTX ; 
	}

	for (i=0 ; i < kflow_ai_get_net_supported_num(); i++){
		if (kflow_isp_proc_pool_map[i] == pool_id){
			pool_info->proc_id = i ; 
			DBG_MSG("proc_id(%u) => pool_%d\n", i, pool_id) ; 
			return E_OK;
		}
	}
	
	DBG_MSG("pool_id(%d) is not binded with any proc_id !?\r\n", pool_id);
	return E_SYS;
}


ER kflow_isp_get_ubuf(UINT32 proc_id, INT32 *available_idx, UINT32 need_size)
{
	INT32 pool_id = kflow_isp_proc_pool_map[proc_id] ; 

	*available_idx = (-1); // init as NOT FOUND

	if (need_size == 0) {
		DBG_ERR("need_size = 0 !?\r\n");
		return E_SYS;
	}
	if(pool_id < 0 ){
		if (kflow_isp_get_st_in_ub_mask(proc_id, available_idx) == E_OK){
			return E_OK;
		}else {
			DBG_ERR("proc_id(%u) is not binding with any pool !\r\n", proc_id);
		}
		return E_SYS;
	}
	// check whether pool is valid 
	if (pool_id > 3){
		DBG_ERR("pool_%u is illegal ! \n", pool_id);
		return E_CTX ; 
	}

	if (MAX_ISP_UB > AI_SUPPORT_UBUF_MAX) {
		DBG_ERR("dtsi \"ai_maxubuf\" => isp need max ubuf(%d) must < AI_SUPPORT_UBUF_MAX(%d)\r\n", MAX_ISP_UB, AI_SUPPORT_UBUF_MAX);
		return E_SYS;
	}
	if (need_size > MAX_ISP_UB) {
		DBG_ERR("dtsi \"ai_maxubuf\" => ai_maxubuf (%u) < isp need ubuf(%u)\r\n", MAX_ISP_UB, need_size);
		nvt_ai_dump_ubuf_status();
		return E_SYS;
	}
	if(kflow_isp_pool[pool_id][0] == 0){
		DBG_ERR("proc_id(%u) needs UB(%u), but pool_%u UB status is off ! \n", proc_id, need_size, pool_id);
		return E_SYS;
	}
	// check whether pool UB is enough
	if (need_size > (kflow_isp_pool[pool_id][2] - kflow_isp_pool[pool_id][1] + 1)){
		DBG_ERR("pool_%u [UB st_id(%u)] [UB end_id(%u)] is not enough for proc_id(%u) UB(%u)! \n", pool_id, kflow_isp_pool[pool_id][0], kflow_isp_pool[pool_id][1], proc_id, need_size);
		return E_SYS ; 
	}
	*available_idx = kflow_isp_pool[pool_id][1] ; 
	
	return E_OK;
}

ER kflow_isp_free_ubuf(UINT32 proc_id, INT32 idx, UINT32 size)
{
	
	// After latest dtsi setting, using the statis ubuf idx 
 	/*
	UINT32 i=0;
	UINT32 MAX_ISP_UB = 0;
	kflow_isp_get_max_ubuf(&MAX_ISP_UB);
	if (MAX_ISP_UB > AI_SUPPORT_UBUF_MAX) {
		DBG_ERR("dtsi \"ai_maxubuf\" => isp need max ubuf(%d) must < AI_SUPPORT_UBUF_MAX(%d)\r\n", MAX_ISP_UB, AI_SUPPORT_UBUF_MAX);
		return E_SYS;
	}

	if (idx >= MAX_ISP_UB) {
		DBG_ERR("proc_id(%d) want to free isp ubuf idx (%d), but this is normal case range, isp should be (0)~(%d) ...\r\n", (int)proc_id, (int)idx, (int)(MAX_ISP_UB-1));
		return E_SYS;
	}

	if (idx + size > MAX_ISP_UB) {
		DBG_ERR("proc_id(%d) want to free isp ubuf idx (%d)~(%d), exceed MAX_ISP_UB(%d) ...\r\n", (int)proc_id, (int)idx, (int)(idx+size-1), (int)MAX_ISP_UB);
		return E_SYS;
	}

	nvt_ai_comm_lock();

	// check if this free request is correct
	for (i=idx ; i < idx+size ; i++) {
		if (g_ai_net_ubuf_status[i] != proc_id) {
			DBG_ERR("proc_id(%d) want to free isp ubuf idx (%d)~(%d), but status[%d]=(%d) is NOT given proc_id(%d) ....\r\n", (int)proc_id, (int)idx, (int)(idx+size-1), (int)i, (int)g_ai_net_ubuf_status[i], (int)proc_id);
			nvt_ai_dump_ubuf_status();

			nvt_ai_comm_unlock();
			return E_SYS;
		}
	}

	// free idx
	for (i=idx ; i < idx+size ; i++) {
		g_ai_net_ubuf_status[i] = NN_UBUF_STATUS_FREE;
	}

	nvt_ai_comm_unlock();
	*/
	return E_OK;
}
ER kflow_isp_get_jobm_core(UINT32 proc_id, INT32 *available_idx)
{
	INT32 pool_id = kflow_isp_proc_pool_map[proc_id] ; 

	*available_idx = (-1); // init as NOT FOUND

	
	if(pool_id < 0 ){
		if (kflow_isp_get_st_in_core_mask(kflow_isp_proc_core_mask[proc_id], available_idx) == E_OK){
			return E_OK;
		}else {
			DBG_ERR("proc_id(%u) is not binding with any pool !\r\n", proc_id);
		}
		return E_SYS;
	}
	// check whether pool is valid 
	if (pool_id > 3){
		DBG_ERR("pool_%u is illegal ! \n", pool_id);
		return E_CTX ; 
	}

	
	if(kflow_isp_pool[pool_id][0] == 3){
		DBG_ERR("proc_id(%u) apply pool_%u but UB status is 3 (3: gblktile) ! \n", proc_id, pool_id);
		return E_SYS;
	}
	
	*available_idx = kflow_isp_pool[pool_id][3] ; 
	
	return E_OK;
}
ER kflow_ai_isp_get_workbuf_size_from_jmisp_info(VENDOR_AIS_ISP_JMISP_INFO_PARM* jmisp_info_param)
{
    UINT32 i = 0 ;
	UINT32 max_size = 0, cur_addr = 0;
	AI_JMISP_INFO * jmisp_info = (AI_JMISP_INFO *)jmisp_info_param->jmisp_info.va ; 

	// DBG_DUMP(" jmisp_info.va %lx pa%lx size(%u)\n", (ULONG)jmisp_info_param->jmisp_info.va, (ULONG)jmisp_info_param->jmisp_info.pa, jmisp_info_param->jmisp_info.size) ;

	if (jmisp_info->iobuf[e_AI_JMISP_IOBUF_RING_IN0].stripeInfo.output_slice_height != jmisp_info->iobuf[e_AI_JMISP_IOBUF_RING_IN0].frameInfo.frame_height){
		// if not frame mode, get buf_size from header ;
		jmisp_info_param->workbuf_size = 0 ; 
		return E_OK;
	}

	for (i = e_AI_JMISP_IOBUF_REF_IN0 ; i < e_AI_IOBUF_NUM ; i ++){
		AI_JMISP_IOBUF_INFO* iobuf = &jmisp_info->iobuf[i] ; 
		if ( e_AI_JMISP_BASE_ADDR_1 == iobuf->addrType && iobuf->buffer_sz > 0 ){
			cur_addr = iobuf->pa + iobuf->buffer_sz;
			max_size = (max_size < cur_addr)? cur_addr:max_size;
		}else{
			continue ;
		}
	}
    jmisp_info_param->workbuf_size = max_size ;
	DBG_MSG("Frame Mode get jmisp_info_param->workbuf_size(%u) \n", jmisp_info_param->workbuf_size);
		
	return E_OK;
}
