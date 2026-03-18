/**
	@brief Source file of cpu1 processing flow.

	@file vendor_ai_cpu.c

	@ingroup vendor_ai_cpu

	@note Nothing.

	Copyright Novatek Microelectronics Corp. 2019.  All rights reserved.
*/

/*-----------------------------------------------------------------------------*/
/* Include Files                                                               */
/*-----------------------------------------------------------------------------*/
#include "hd_type.h"
//#include "vendor_ai_util.h"
//=============================================================
#define __CLASS__ 				"[ai][lib_cpu][cpu]"
#include "vendor_ai_debug.h"
//=============================================================

#include "vendor_ai_net/nn_verinfo.h"
#include "vendor_ai_net/nn_net.h"
#include "vendor_ai_net/nn_parm.h"

#include "vendor_ai_plugin.h"
#include "vendor_ai_cpu/vendor_ai_cpu.h"  // NN_BNSCALE, NN_POOL, NN_FC_POST
#include "vendor_ai_cpu/vendor_ai_cpu_builtin.h" // NN_SOFTMAX
#include "vendor_ai_cpu_custnn.h"         // NN_CUSTOMER
#include "vendor_ai_cpu_softmax.h"        // NN_SOFTMAX
#include "vendor_ai_cpu_proposal.h"       // NN_PROPOSAL
#include "vendor_ai_cpu_prelu.h"          // NN_PRELU

#include "vendor_ai_cpu_version.h"	//VER
#include "nvtnn/nvtnn_lib.h"			//SUBVER

#include <string.h>

#if NN_DLI
#include "vendor_ai_net/nn_dli.h"
#ifdef NN_DLI_CPU_ACL
#include "nn_dli_cpu/vendor_ai_dli_cpu.h"
#endif /* NN_DLI_CPU_ACL */
#endif /* NN_DLI */


char lib_pub_ver[32] = "version="VENDOR_AI_CPU_IMPL_VERSION; //for grep version

#if (CUST_PROC_ALLOC_MEM==0)
#define NN_SUPPORT_NET_MAX  512    // max supported network number

extern uintptr_t next_cust_parm_addr[NN_SUPPORT_NET_MAX][CUST_MAX_OUT_NUM][CUST_MAX_MAP_NUM];
extern UINT32 next_cust_match_in_idx[NN_SUPPORT_NET_MAX][CUST_MAX_OUT_NUM][CUST_MAX_MAP_NUM];
#endif

static HD_RESULT _vendor_ai_cpu1_proc(UINT32 proc_id, UINT32 layer_id, UINT32 mode, uintptr_t layer_param, uintptr_t parm_addr)
{
	HD_RESULT rv = HD_OK;
    UINT32 ctrl = mode & 0xff000000; //extract its cmd
    mode = (mode & 0x00ffffff);
	//printf("[_vendor_ai_cpu1_proc] callback !! proc_id = %u, layer_id = %u, ctrl = %08x, mode = %08x, layer_param = 0x%lx, parm_addr = 0x%lx\n", (int)proc_id, (int)layer_id, (int)ctrl, (int)mode, (ULONG)layer_param, (ULONG)parm_addr);

    // dev control flow (proc_id = CTRL, layer_id = CTRL)
	if ((proc_id == VENDOR_AI_CTRL_NET) && (layer_id == VENDOR_AI_CTRL_LYR)) {
        if (ctrl == VENDOR_AI_CTRL_ENG_INIT) {
            return rv;
        }
        if (ctrl == VENDOR_AI_CTRL_ENG_UNINIT) {
            return rv;
        }
        return rv;
    }

    // net control flow (proc_id = any, layer_id = CTRL)
	if (layer_id == VENDOR_AI_CTRL_LYR) {
        if (ctrl == VENDOR_AI_CTRL_NET_OPEN) {
#if (CUST_PROC_ALLOC_MEM==1)
			// dynamic alloc custnn context for this proc_id
			uintptr_t* next_cust_parm_addr = (uintptr_t *)malloc(sizeof(uintptr_t) * CUST_MAX_OUT_NUM * CUST_MAX_MAP_NUM);
			UINT32* next_cust_match_in_idx = (UINT32*)malloc(sizeof(UINT32) * CUST_MAX_OUT_NUM * CUST_MAX_MAP_NUM);
			if ((next_cust_parm_addr == NULL) || (next_cust_match_in_idx == NULL)) {
				DBG_ERR("proc_id(%d) failed to alloc usr context ...\r\n", proc_id);
				if (next_cust_parm_addr)    { free(next_cust_parm_addr); }
				if (next_cust_match_in_idx) { free(next_cust_match_in_idx); }
				return HD_ERR_NOMEM;
			}
			memset(next_cust_parm_addr, 0, sizeof(uintptr_t) * CUST_MAX_OUT_NUM * CUST_MAX_MAP_NUM);
			memset(next_cust_match_in_idx, 0, sizeof(UINT32) * CUST_MAX_OUT_NUM * CUST_MAX_MAP_NUM);
			rv = _vendor_ai_net_set_usr_context(proc_id, next_cust_parm_addr, next_cust_match_in_idx);
			if (HD_OK != rv) {
				DBG_ERR("proc_id(%d) failed to set usr context ...\r\n", proc_id);
				return rv;
			}
#endif
            return rv;
        }
        if (ctrl == VENDOR_AI_CTRL_NET_START) {
            return rv;
        }
        if (ctrl == VENDOR_AI_CTRL_NET_STOP) {
            return rv;
        }
        if (ctrl == VENDOR_AI_CTRL_NET_CLOSE) {
#if (CUST_PROC_ALLOC_MEM==1)
			// dynamic free custnn context for this proc_id
			uintptr_t* next_cust_parm_addr = NULL;
			UINT32* next_cust_match_in_idx = NULL;
			rv = _vendor_ai_net_get_usr_context(proc_id, &next_cust_parm_addr, &next_cust_match_in_idx);
			if (HD_OK != rv) {
				DBG_ERR("proc_id(%d) failed to get usr context ...\r\n", proc_id);
				return rv;
			}
			if (next_cust_parm_addr)    { free(next_cust_parm_addr); }
			if (next_cust_match_in_idx) { free(next_cust_match_in_idx); }
#endif
            return rv;
        }
        return rv;
    }

    // layer control flow (proc_id, layer) - start
    if (ctrl == VENDOR_AI_CTRL_LYR_START) {
        switch ((NN_MODE)mode) {
        case NN_SOFTMAX:
            break;
        case NN_POOL:
            break;
#if NN_DLI
		case NN_DLI_SQRT:
		case NN_DLI_EXP:
		case NN_DLI_DIV:
		case NN_DLI_RESIZE:
		case NN_DLI_LOG:
		case NN_DLI_POW:
		case NN_DLI_SIN:
		case NN_DLI_EQUAL:
		case NN_DLI_GREATER:
		case NN_DLI_LESS:
		case NN_DLI_FLOOR:
		case NN_DLI_ROUND:
		case NN_DLI_SOFTMAX:
		case NN_DLI_POOLING:
		case NN_DLI_REVERSE:
		case NN_DLI_FC_POST:
		case NN_DLI_PERMUTE:
#ifdef NN_DLI_CPU_ACL
			//printf("[_vendor_ai_cpu1_proc] proc_id = %u, layer_id = %u, start() !! mode = %u, parm_addr = 0x%lx\n",
			//	(int)proc_id, (int)layer_id, (int)mode, (ULONG)parm_addr);
			dli_cpu_init((NN_DLI_LAYER_PARM *)parm_addr);
#endif /* NN_DLI_CPU_ACL */
			break;
#endif /* NN_DLI */
        default:
            break;
        }
        return rv;
    }

    // layer control flow (proc_id, layer) - stop
    if (ctrl == VENDOR_AI_CTRL_LYR_STOP) {
        switch ((NN_MODE)mode) {
        case NN_SOFTMAX:
            break;
        case NN_POOL:
            break;
#if NN_DLI
		case NN_DLI_SQRT:
		case NN_DLI_EXP:
		case NN_DLI_DIV:
		case NN_DLI_RESIZE:
		case NN_DLI_LOG:
		case NN_DLI_POW:
		case NN_DLI_SIN:
		case NN_DLI_EQUAL:
		case NN_DLI_GREATER:
		case NN_DLI_LESS:
		case NN_DLI_FLOOR:
		case NN_DLI_ROUND:
		case NN_DLI_SOFTMAX:
		case NN_DLI_POOLING:
		case NN_DLI_REVERSE:
		case NN_DLI_FC_POST:
		case NN_DLI_PERMUTE:
#ifdef NN_DLI_CPU_ACL
			//printf("[_vendor_ai_cpu1_proc] proc_id = %u, layer_id = %u, stop() !! mode = %u, parm_addr = 0x%lx\n",
			//	(int)proc_id, (int)layer_id, (int)mode, (ULONG)parm_addr);
			dli_cpu_uninit((NN_DLI_LAYER_PARM *)parm_addr);
#endif /* NN_DLI_CPU_ACL */
			break;
#endif /* NN_DLI */
        default:
            break;
        }
        return rv;
    }

    // layer control flow (proc_id, layer) - proc
	switch ((NN_MODE)mode) {

	case NN_PROPOSAL:
		rv = vendor_ais_proposal(parm_addr);
		break;


	case NN_CUSTOMER:
		{
			VOID* usr_info = _vendor_ai_net_get_usr_info(proc_id);
#if (CUST_PROC_ALLOC_MEM==1)
			uintptr_t* next_cust_parm_addr = NULL;
			UINT32* next_cust_match_in_idx = NULL;
			rv = _vendor_ai_net_get_usr_context(proc_id, &next_cust_parm_addr, &next_cust_match_in_idx);
			if (HD_OK != rv) {
				DBG_ERR("proc_id(%d) failed to get usr context ...\r\n", proc_id);
				return rv;
			}
#endif
			vendor_ai_cust_set_next_info(CUST_MAX_OUT_NUM, CUST_MAX_MAP_NUM, (uintptr_t*)next_cust_parm_addr, (UINT32*)next_cust_match_in_idx);
			vendor_ai_cpu_cust_set_next_parm_addr(proc_id, layer_id); // will call vendor_ai_cust_set_next_process_mctrl() to set addr
			vendor_ai_cpu_cust(parm_addr, proc_id, layer_id, usr_info);
			// reset next info
			for (int j = 0; j < CUST_MAX_OUT_NUM; j++) {
				for (int i = 0; i < CUST_MAX_MAP_NUM; i++) {
					vendor_ai_cust_set_next_process_mctrl(0, 0, j, i, proc_id);
				}
			}
		}
		break;

	case NN_SOFTMAX:
		rv = vendor_ais_softmax_asym((NN_SOFTMAX_ASYM_PARM *)parm_addr, proc_id);
		break;
	case NN_GATHER:
		rv = nvtnn_gather_process((NVTNN_GATHER_PRMS *)parm_addr);
		break;
	case NN_GATHER_ND:
		rv = nvtnn_gathernd_process((NN_GATHERND_PARM *)parm_addr);
		break;
	case NN_GATHER_ELEMENT:
		rv = nvtnn_gatherelement_process((NN_GATHERELEMENT_PARM *)parm_addr);
		break;
	case NN_ONEHOT:
		rv = nvtnn_onehot_process((NN_ONEHOT_PARM *)parm_addr);
		break;
	case NN_TOPK:
		rv = nvtnn_topk_process((NN_TOPK_PARM *)parm_addr);
		break;
	case NN_CPU_GRIDSAMPLE:
		rv = nvtnn_gridsample_process((NN_GRIDSAMPLE_PARM *)parm_addr);
		break;
	case NN_CUMSUM:
		rv = nvtnn_cumsum_process((NN_CUMSUM_PARM *)parm_addr);
		break;
	case NN_CPU_LRN:
		rv = nvtnn_lrn_process((NN_LRN_PARM *)parm_addr);
		break;
	case NN_POOL:
        rv = nvtnn_pool_process((NN_POOL_PARM *)parm_addr);
		break;

	case NN_BNSCALE:
		//rv = vendor_ai_bnscale_process((NN_BNSCALE_PARM *)parm_addr);
		//break;

	//--- [ not support ] ---
	case NN_PREPROC:
		/*
		p_pre_pram = (NN_PRE_PARM *)(p_mctrl[process_index].addr);
		pre_buff.src.va 		= p_io_mem[layer_index].SAI[0].address;
		if (process_index == 0) {
			pre_buff.src.pa		= in_img.phy_addr[0];
		} else {
			pre_buff.src.pa		= vendor_ais_user_buff_va2pa(pre_buff.src.va, net_id);
		}
		pre_buff.src.size 		= p_io_mem[layer_index].SAI[0].size;
		pre_buff.dst.va 		= p_io_mem[layer_index].SAO[0].address;
		pre_buff.dst.pa			= vendor_ais_user_buff_va2pa(pre_buff.dst.va, net_id);
		pre_buff.dst.size 		= p_io_mem[layer_index].SAO[0].size;
		pre_buff.interm.va 		= p_io_mem[layer_index].SAI[2].address;
		pre_buff.interm.pa		= vendor_ais_user_buff_va2pa(pre_buff.interm.va, net_id);
		pre_buff.interm.size 	= p_io_mem[layer_index].SAI[2].size;
		pre_buff.mean.va 		= p_io_mem[layer_index].SAI[1].address;
		pre_buff.mean.pa		= vendor_ais_user_model_va2pa(pre_buff.mean.va, net_id);
		pre_buff.mean.size 		= p_io_mem[layer_index].SAI[1].size;
		vendor_ais_preproc(p_pre_pram, pre_buff, net_id);
		break;
		*/
	case NN_PRELU:
		vendor_ais_prelu((NN_PRELU_PARM *)parm_addr, proc_id);
		break;
    case NN_RESHAPE:
		nvtnn_permute_process((NN_PERMUTE_PARM *)parm_addr);
		break;
	case NN_REVERSE:
		nvtnn_reverse_process((NN_REVERSE_PARM *)parm_addr);
		break;
	case NN_LSTM:
		nvtnn_lstm_process((NN_LSTM_PARM *)parm_addr);
		break;
	case NN_NORM:
		nvtnn_norm_process((NN_NORM_PARM *)parm_addr);
		break;
#if NN_DLI
	case NN_DLI_SQRT:
	case NN_DLI_EXP:
	case NN_DLI_DIV:
	case NN_DLI_RESIZE:
	case NN_DLI_LOG:
	case NN_DLI_POW:
	case NN_DLI_SIN:
	case NN_DLI_EQUAL:
	case NN_DLI_GREATER:
	case NN_DLI_LESS:
	case NN_DLI_FLOOR:
	case NN_DLI_ROUND:
	case NN_DLI_SOFTMAX:
	case NN_DLI_POOLING:
	case NN_DLI_REVERSE:
	case NN_DLI_FC_POST:
	case NN_DLI_PERMUTE:
#ifdef NN_DLI_CPU_ACL
		//printf("[_vendor_ai_cpu1_proc] proc_id = %u, layer_id = %u, proc() !! mode = %u, parm_addr = 0x%lx\n",
		//	(int)proc_id, (int)layer_id, (int)mode, (ULONG)parm_addr);
		dli_cpu_proc((NN_DLI_LAYER_PARM *)parm_addr);
#endif /* NN_DLI_CPU_ACL */
		break;
#endif /* NN_DLI */
	case NN_CONV:
	case NN_DECONV:
	case NN_MATMUL:
	case NN_SVM:
	case NN_FC:
	case NN_ROIPOOLING:
	case NN_ELTWISE:
	case NN_REORGANIZATION:
	case NN_CORR:
	default:
		printf("[ai][pub][cpu] ERR: _vendor_ai_cpu1_proc: proc_id[%u], layer_id[%u], mode[%u]: UKNOWN op mode? please check your model.bin\n", (int)proc_id, (int)layer_id, (int)mode);
		return HD_ERR_NOT_SUPPORT;
	}
	return rv;
}

static HD_RESULT _vendor_ai_cpu1_get(UINT32 proc_id, UINT32 layer_id, UINT32 mode, uintptr_t layer_param, uintptr_t parm_addr, UINT32 cmd, uintptr_t* buf_addr, UINT32* buf_size)
{
	HD_RESULT rv = HD_ERR_NOT_SUPPORT;
	//printf("[_vendor_ai_cpu1_proc] callback !! mode = %lu, eng = %lu, parm_addr = 0x%08x, proc_id = %lu\n", mode, eng, (UINT)parm_addr, proc_id);

    // dev get param (proc_id = CTRL, layer_id = CTRL)
	if ((proc_id == VENDOR_AI_CTRL_NET) && (layer_id == VENDOR_AI_CTRL_LYR)) {
        switch (cmd) {
        case VENDOR_AI_PLUGIN_VER:
            memcpy((CHAR*)parm_addr, VENDOR_AI_CPU_IMPL_VERSION, sizeof(VENDOR_AI_CPU_IMPL_VERSION));
            rv = HD_OK;
            break;
        case VENDOR_AI_PLUGIN_SUBVER:
            memcpy((CHAR*)parm_addr, prebuilt_ai_get_lib_version(), layer_param);
            rv = HD_OK;
            break;
#if NN_DLI
#ifdef NN_DLI_CPU_ACL
		case VENDOR_AI_PLUGIN_SIGN:
			rv = HD_OK;
			break;
#endif /* NN_DLI_CPU_ACL */
#endif /* NN_DLI */
        default:
            break;
        }
        return rv;
    }

    // net get param (proc_id = any, layer_id = CTRL)
	if (layer_id == VENDOR_AI_CTRL_LYR) {
        switch (cmd) {
        //case VENDOR_AI_PLUGIN_NET_P1:
        //    rv = HD_OK;
        //    break;
        default:
            break;
        }
        return rv;
    }

    // layer get param (proc_id = any, layer = any)
	switch ((NN_MODE)mode) {
	default:
		break;
	}
	return rv;
}

static HD_RESULT _vendor_ai_cpu1_set(UINT32 proc_id, UINT32 layer_id, UINT32 mode, uintptr_t layer_param, uintptr_t parm_addr, UINT32 cmd, uintptr_t buf_addr, UINT32 buf_size)
{
	HD_RESULT rv = HD_ERR_NOT_SUPPORT;
	//printf("[_vendor_ai_cpu1_proc] callback !! mode = %lu, eng = %lu, parm_addr = 0x%08x, proc_id = %lu\n", mode, eng, (UINT)parm_addr, proc_id);

    // dev set param (proc_id = CTRL, layer_id = CTRL)
	if ((proc_id == VENDOR_AI_CTRL_NET) && (layer_id == VENDOR_AI_CTRL_LYR)) {
        switch (cmd) {
        case VENDOR_AI_PLUGIN_VER:
            //memcpy((CHAR*)parm_addr, VENDOR_AI_CPU_IMPL_VERSION, layer_param);
            rv = HD_OK;
            break;
        case VENDOR_AI_PLUGIN_SUBVER:
            //memcpy((CHAR*)parm_addr, prebuilt_ai_get_lib_version(), layer_param);
            rv = HD_OK;
            break;
        default:
            break;
        }
        return rv;
    }

    // net set param (proc_id = any, layer_id = CTRL)
	if (layer_id == VENDOR_AI_CTRL_LYR) {
        switch (cmd) {
        //case VENDOR_AI_PLUGIN_NET_P1:
        //    rv = HD_OK;
        //    break;
        default:
            break;
        }
        return rv;
    }

    // layer set param (proc_id = any, layer = any)
	switch ((NN_MODE)mode) {
        default:
            break;
	}
	return rv;
}

static VENDOR_AI_ENGINE_PLUGIN _vendor_ai_cpu1 = {
	.sign = MAKEFOURCC('A','E','N','G'),
	.eng = 1, //cpu
	.ch = 0,
	.proc_cb = _vendor_ai_cpu1_proc,
	.get_cb = _vendor_ai_cpu1_get,
	.set_cb = _vendor_ai_cpu1_set
};

void* vendor_ai_cpu1_get_engine(void)
{
	return (void*)&_vendor_ai_cpu1;
}

