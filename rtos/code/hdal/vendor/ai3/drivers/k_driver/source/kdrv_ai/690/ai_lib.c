/*#include <linux/module.h>
#include <linux/kernel.h>
#include <asm/io.h>
#include <linux/uaccess.h>*/
//#include "mach/fmem.h"
#include "kwrap/semaphore.h"
/*#include <linux/delay.h>
#include <linux/timer.h>
#include <linux/spinlock.h>
*/

#include "ai_lib.h"
#include "kdrv_ai.h"
#include "kdrv_ai_int.h"
#if defined(__FREERTOS)
#include "kwrap/debug.h"
#include "string.h"
#include "efuse_protected.h"
#else
#include "kdrv_ai_dbg.h"
#include <plat/efuse_protected.h>
#endif
#include "kwrap/flag.h"
#include "kdrv_ai_platform.h"
#include "kwrap/cpu.h"

#if defined(__FREERTOS)
#define AI_SHIFT_FOR_MSB(a)    0
#else
#define AI_SHIFT_FOR_MSB(a)    ((UINT32) (((uintptr_t) (a)) >> ((uintptr_t) (32))))
#endif

UINT32 kdrv_ai_is_init = 0;
UINT32 kdrv_ai_conv_clk_rate = 0;

INT32 kdrv_ai_int_clamp(INT32 prx, INT32 lb, INT32 ub);

ER kdrv_ai_chk_efuse_ai_eng(VOID)
{
	ER ret = E_OK;
	
	/*
	ret = kdrv_ai_chk_efuse_nue2();
	if (ret != E_OK) {
		return ret;
	}
*/
	return ret;
}

ER kdrv_ai_chk_efuse_ai_lib(VOID)
{
#if !defined(CONFIG_NVT_FPGA_EMULATION) && !defined(_NVT_FPGA_)
	if(efuse_check_available(NULL) != TRUE){
		DBG_ERR("objver mismatch in kdrv ai_lib\r\n");
		return E_CTX;
	}
#endif

	return E_OK;
}

INT32 kdrv_ai_int_clamp(INT32 prx, INT32 lb, INT32 ub)
{
	/* avoid integer overflow or underflow */
	if (prx < lb) {
		return (lb);
	} else if (prx > ub) {
		return (ub);
	} else {
		return (prx);
	}
}


ER kdrv_ai_conv_ll_trig(UINT8 conv_id, uintptr_t pa)
{
	ER er_return = E_OK;
	
	/*
	CNN_LL_PRM cnn_ll_prm = {0};
	
	if (pa.addr64 == 0) {
		DBG_ERR("input address is null\r\n");
		return E_NOMEM;
	}

	if (cnn_id == 0) {
		cnn_ll_prm.addrin_ll_base.addr64 = g_kdrv_ai_ll_base_addr[AI_ENG_CNN].addr64;
	} else {
		cnn_ll_prm.addrin_ll_base.addr64 = g_kdrv_ai_ll_base_addr[AI_ENG_CNN2].addr64;
	}
	cnn_ll_prm.addrin_ll = pa;
	er_return = cnn_ll_setmode(cnn_id, &cnn_ll_prm);
	
	if (er_return != E_OK) {
		DBG_ERR("set mode error : %08x\r\n", (int)er_return);
		return er_return;
	}

	if (g_ai_isr_trig == 0) {
		er_return = cnn_ll_start(cnn_id);
		if (er_return != E_OK) {
			DBG_ERR("start error : %08x\r\n", (int)er_return);
			return er_return;
		}
	} else {
		er_return = cnn_ll_isr_start(cnn_id);
		if (er_return != E_OK) {
			DBG_ERR("cnn_ll_isr_start error : %08x\r\n", (int)er_return);
			return er_return;
		}
	}
	*/
	return er_return;
}

ER kdrv_ai_conv_ll_done(BOOL conv_id, KDRV_AI_HANDLE *p_handle)
{
	ER er_return = E_OK;
/*
	cnn_wait_ll_frameend(cnn_id, FALSE);
	er_return = cnn_ll_pause(cnn_id);
*/
	return er_return;
}

ER kdrv_ai_nue2_ll_trig(uintptr_t pa)
{
	ER er_return = E_OK;
/*
	NUE2_LL_PRM kdrv_ll_parm;

	if (pa == 0) {
		DBG_ERR("input address is null\r\n");
		return E_NOMEM;
	}

	kdrv_ll_parm.ll_addr = pa;
	kdrv_ll_parm.ll_base_addr = g_kdrv_ai_ll_base_addr[AI_ENG_NUE2];
	er_return = nue2_ll_setmode(&kdrv_ll_parm);
	if (er_return != E_OK) {
		DBG_ERR("set mode error : %08x\r\n", (int)er_return);
		return er_return;
	}
	if (g_ai_isr_trig == 0) {
		er_return = nue2_ll_start();
		if (er_return != E_OK) {
			DBG_ERR("start error : %08x\r\n", (int)er_return);
			return er_return;
		}
	} else {
		er_return = nue2_ll_isr_start();
		if (er_return != E_OK) {
			DBG_ERR("nue2_ll_isr_start error : %08x\r\n", (int)er_return);
			return er_return;
		}

	}
*/
	return er_return;
}


ER kdrv_ai_nue2_ll_done(KDRV_AI_HANDLE *p_handle)
{
	ER er_return = E_OK;
/*
	nue2_wait_ll_frameend(FALSE);
	er_return = nue2_ll_pause();
*/
	return er_return;
}

VOID kdrv_ai_set_init_status(UINT32 status)
{
	kdrv_ai_is_init = status;
}

UINT32 kdrv_ai_get_init_status(VOID)
{
	return kdrv_ai_is_init;
}

VOID kdrv_ai_set_conv_clk_rate(UINT32 clk_rate)
{
	kdrv_ai_conv_clk_rate = clk_rate;
}

UINT32 kdrv_ai_get_conv_clk_rate(VOID)
{
	return kdrv_ai_conv_clk_rate;
}

#ifdef __KERNEL__
EXPORT_SYMBOL(kdrv_ai_conv_ll_trig);
EXPORT_SYMBOL(kdrv_ai_conv_ll_done);
EXPORT_SYMBOL(kdrv_ai_nue2_ll_trig);
EXPORT_SYMBOL(kdrv_ai_nue2_ll_done);
#endif
