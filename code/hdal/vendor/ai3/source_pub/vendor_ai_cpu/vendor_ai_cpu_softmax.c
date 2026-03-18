/**
	@brief Source file of cpu softmax.

	@file vendor_ai_cpu_softmax.c

	@ingroup vendor_ai_cpu_softmax

	@note Nothing.

	Copyright Novatek Microelectronics Corp. 2019.  All rights reserved.
*/

/*-----------------------------------------------------------------------------*/
/* Include Files                                                               */
/*-----------------------------------------------------------------------------*/
#include "hd_type.h"
//#include "vendor_ai_util.h"
//=============================================================
#define __CLASS__ 				"[ai][lib_cpu][softmax]"
#include "vendor_ai_debug.h"
//=============================================================

#include "vendor_ai_cpu_softmax.h"
#include "vendor_ai_cpu/vendor_ai_cpu_builtin.h"
#include "nvtnn/nvtnn_lib.h"
#include <string.h>

/*-----------------------------------------------------------------------------*/
/* Local Constant Definitions                                                  */
/*-----------------------------------------------------------------------------*/
#define POST_USE_CPU_STRUCT 0

/*-----------------------------------------------------------------------------*/
/* Local Types Declarations                                                    */
/*-----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------*/
/* Extern Function Prototype                                                   */
/*-----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------*/
/* Local Global Variables                                                      */
/*-----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------*/
/* Interface Functions                                                         */
/*-----------------------------------------------------------------------------*/

//static ER vendor_ais_softmax(NN_SOFTMAX_PARM *p_parm);
HD_RESULT vendor_ais_softmax_asym(NN_SOFTMAX_ASYM_PARM *p_parm, UINT32 net_id)
{
#if NN_USE_DSP
	uintptr_t in_va, out_va, in_trans_va, out_trans_va;

	in_va        = p_parm->in_addr;
	out_va       = p_parm->out_addr;
	in_trans_va  = p_parm->in_trans_addr;
	out_trans_va = p_parm->out_trans_addr;

	p_parm->in_addr        = vendor_ais_user_buff_va2pa(p_parm->in_addr       , net_id);
	p_parm->out_addr       = vendor_ais_user_buff_va2pa(p_parm->out_addr      , net_id);
	p_parm->in_trans_addr  = vendor_ais_user_buff_va2pa(p_parm->in_trans_addr , net_id);
	p_parm->out_trans_addr = vendor_ais_user_buff_va2pa(p_parm->out_trans_addr, net_id);
#endif

	nvtnn_softmax_asym_process(p_parm);

#if NN_USE_DSP
	p_parm->in_addr        = in_va;
	p_parm->out_addr       = out_va;
	p_parm->in_trans_addr  = in_trans_va;
	p_parm->out_trans_addr = out_trans_va;
#endif

	return HD_OK;
}

