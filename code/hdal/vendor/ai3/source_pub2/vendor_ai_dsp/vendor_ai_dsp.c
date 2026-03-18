/**
	@brief Source file of dsp1 processing flow.

	@file vendor_ai_dsp.c

	@ingroup vendor_ai_dsp

	@note Nothing.

	Copyright Novatek Microelectronics Corp. 2019.  All rights reserved.
*/

/*-----------------------------------------------------------------------------*/
/* Include Files                                                               */
/*-----------------------------------------------------------------------------*/
#include "hd_type.h"
//#include "vendor_ai_util.h"
//=============================================================
#define __CLASS__ 				"[ai][lib_dsp][dsp]"
#include "vendor_ai_debug.h"
//=============================================================

#include "vendor_ai_net/nn_verinfo.h"
#include "vendor_ai_net/nn_net.h"
#include "vendor_ai_net/nn_parm.h"

#include "vendor_ai_plugin.h"
#include "vendor_ai_dsp/vendor_ai_dsp.h"  // NN_BNSCALE, NN_POOL, NN_FC_POST, NN_SOFTMAX
#include "vendor_ai_dsp_custnn.h"         // NN_CUSTOMER

#if NN_DLI
#include "vendor_ai_net/nn_dli.h"
#ifdef NN_DLI_DSP_ACL
#include "nn_dli_dsp/vendor_ai_dli_dsp.h"
#endif /* NN_DLI_DSP_ACL */
#endif /* NN_DLI */

static HD_RESULT _vendor_ai_dsp1_proc(UINT32 proc_id, UINT32 layer_id, UINT32 mode, uintptr_t layer_param, uintptr_t parm_addr)
{
	HD_RESULT rv = HD_OK;
    UINT32 ctrl = mode & 0xff000000; //extract its cmd
    mode = (mode & 0x00ffffff);
	//printf("[_vendor_ai_dsp1_proc] callback !! proc_id = %u, layer_id = %u, ctrl = %08x, mode = %08x, layer_param = 0x%lx, parm_addr = 0x%lx\n", (int)proc_id, (int)layer_id, (int)ctrl, (int)mode, (ULONG)layer_param, (ULONG)parm_addr);

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
            return rv;
        }    
        if (ctrl == VENDOR_AI_CTRL_NET_START) {
            return rv;
        }    
        if (ctrl == VENDOR_AI_CTRL_NET_STOP) {
            return rv;
        }    
        if (ctrl == VENDOR_AI_CTRL_NET_CLOSE) {
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
		case NN_DLI_DILATEDCONV:
		case NN_DLI_DIV:
		case NN_DLI_EXP:
		case NN_DLI_SQRT:
		case NN_DLI_POW:
		case NN_DLI_SOFTMAX:
		case NN_DLI_DECONVOLUTION:
		case NN_DLI_DECONVOLUTION_DEPTHWISE:
		case NN_DLI_MATMUL:
		case NN_DLI_INSTANCE_NORMALIZATION:
		case NN_DLI_LAYER_NORMALIZATION:
		case NN_DLI_PRIORBOX:
		case NN_DLI_RESIZE:
		case NN_DLI_NORM:
#ifdef NN_DLI_DSP_ACL
			//printf("[_vendor_ai_dsp1_proc] proc_id = %u, layer_id = %u, start() !! mode = %u, parm_addr = 0x%lx\n",
			//	(int)proc_id, (int)layer_id, (int)mode, (ULONG)parm_addr);
			dli_dsp_init((NN_DLI_LAYER_PARM *)parm_addr);
#endif /* NN_DLI_DSP_ACL */
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
		case NN_DLI_DILATEDCONV:
		case NN_DLI_DIV:
		case NN_DLI_EXP:
		case NN_DLI_SQRT:
		case NN_DLI_POW:
		case NN_DLI_SOFTMAX:
		case NN_DLI_DECONVOLUTION:
		case NN_DLI_DECONVOLUTION_DEPTHWISE:
		case NN_DLI_MATMUL:
		case NN_DLI_INSTANCE_NORMALIZATION:
		case NN_DLI_LAYER_NORMALIZATION:
		case NN_DLI_PRIORBOX:
		case NN_DLI_RESIZE:
		case NN_DLI_NORM:
#ifdef NN_DLI_DSP_ACL
			//printf("[_vendor_ai_dsp1_proc] proc_id = %u, layer_id = %u, stop() !! mode = %u, parm_addr = 0x%lx\n",
			//	(int)proc_id, (int)layer_id, (int)mode, (ULONG)parm_addr);
			dli_dsp_uninit((NN_DLI_LAYER_PARM *)parm_addr);
#endif /* NN_DLI_DSP_ACL */
			break;
#endif /* NN_DLI */
        default:
            break;
        }
        return rv;
    }
	
    // layer control flow (proc_id, layer) - proc
	switch ((NN_MODE)mode) {
	case NN_CUSTOMER:
		{
            NN_GEN_MODE_CTRL *p_mctrl = (NN_GEN_MODE_CTRL *)layer_param;
            //NN_DATA_V30 *p_sai = (NN_DATA_V30*)(p_mctrl->iomem.imem_addr);
            NN_DATA_V30 *p_sao = (NN_DATA_V30*)(p_mctrl->iomem.omem_addr);
            NN_CUSTOM_PARM *p_head = (NN_CUSTOM_PARM *)parm_addr;
            uintptr_t layer_type_id = *(uintptr_t *)(p_head + 1);
            if ((layer_type_id & NN_LAYER_LIB_NVTNN) == NN_LAYER_LIB_NVTNN) {
                vendor_ai_dsp_nvtnn(parm_addr, proc_id);
            } else {
                vendor_ai_dsp_cust_set_tmp_buf(p_sao[2], parm_addr);
                vendor_ai_dsp_cust(parm_addr, proc_id);
            }
		}
		break;

	case NN_PROPOSAL:
	case NN_SOFTMAX:
	case NN_FC_POST:
	case NN_POOL:
	case NN_BNSCALE:
	case NN_PREPROC:
	case NN_PRELU:
	case NN_PRIORBOX:
#if NN_DLI
	case NN_DLI_DILATEDCONV:
	case NN_DLI_DIV:
	case NN_DLI_EXP:
	case NN_DLI_SQRT:
	case NN_DLI_POW:
	case NN_DLI_SOFTMAX:
	case NN_DLI_DECONVOLUTION:
	case NN_DLI_DECONVOLUTION_DEPTHWISE:
	case NN_DLI_MATMUL:
	case NN_DLI_INSTANCE_NORMALIZATION:
	case NN_DLI_LAYER_NORMALIZATION:
	case NN_DLI_PRIORBOX:
	case NN_DLI_RESIZE:
	case NN_DLI_NORM:
#ifdef NN_DLI_DSP_ACL
		//printf("[_vendor_ai_dsp1_proc] proc_id = %u, layer_id = %u, proc() !! mode = %u, parm_addr = 0x%lx\n",
		//	(int)proc_id, (int)layer_id, (int)mode, (ULONG)parm_addr);
		dli_dsp_proc((NN_DLI_LAYER_PARM *)parm_addr, NVTNN_SENDTO_DSP1);
#endif /* NN_DLI_DSP_ACL */
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
	case NN_RESHAPE:
	case NN_CORR:
	default:
		printf("_vendor_ai_dsp1_proc: err, Mode %u: not support\n", mode);
		return HD_ERR_NOT_SUPPORT;
	}
	return rv;
}


static VENDOR_AI_ENGINE_PLUGIN _vendor_ai_dsp1 = {
	.sign = MAKEFOURCC('A','E','N','G'),
	.eng = 2,  //dsp
	.ch = 0,
	.proc_cb = _vendor_ai_dsp1_proc
};

void* vendor_ai_dsp1_get_engine(void)
{
	return (void*)&_vendor_ai_dsp1;
}

