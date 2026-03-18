#include "ai_lib.h"
#include "ai_ioctl.h"
#include <kwrap/verinfo.h> 
#include "kdrv_ai_int.h"
#include "interrupt.h"

#include <string.h>
#include "kwrap/semaphore.h"
#include "kwrap/debug.h"
#include "kwrap/flag.h"

#include "conv_eng.h"
#include "nue2_eng.h"
#include "ppu_eng.h"
#include "pou_eng.h"
#include "lsu_eng.h"
#include "jobm_eng.h"
#include "jmisp_eng.h"
#include "util_eng.h"
#include "rou_eng.h"
#include "cal_eng.h"

#if defined(__FREERTOS)
#include "hdal.h"
#include "hd_debug.h"
#include <malloc.h>
#include "kdrv_ai.h"

#define GET_32BitsValue(addr)           (*(volatile UINT32*)((uintptr_t)addr))
#define SET_32BitsValue(addr, value)    (*(volatile UINT32*)((uintptr_t)addr) = (UINT32)(value))
uintptr_t ai_eng_io_addr[9] = {0};
struct clk* ai_clk[8];

UINT8 * kdrv_ai_vmalloc(UINT32 v_size)
{
    char *v_buff;
    v_buff = (char *) malloc(v_size);
    return (UINT8 *) v_buff;
}

VOID kdrv_ai_vfree(UINT8 *v_buff)
{
    if (v_buff) {
        free((char *)v_buff);
    }
    return;
}

irqreturn_t ai_rtos_isr(int irq, void *devid)
{
	if (irq == INT_ID_CNN) {
		CONV_ENG_HANDLE* p_handle = conv_eng_get_handle(0, 0);
		conv_eng_isr_hw_reg(p_handle);
	} else if (irq == INT_ID_NUE2) {
		NUE2_ENG_HANDLE* p_handle = nue2_eng_get_handle(0, 0);
		nue2_eng_isr_hw_reg(p_handle);
	} else if (irq == INT_ID_PPU) {
		PPU_ENG_HANDLE* p_handle = ppu_eng_get_handle(0, 0);
		ppu_eng_isr_hw_reg(p_handle);
	} else if (irq == INT_ID_LSU) {
		LSU_ENG_HANDLE* p_handle = lsu_eng_get_handle(0, 0);
		lsu_eng_isr_hw_reg(p_handle);
	} else if (irq == INT_ID_JM) {
		JOBM_ENG_HANDLE* p_handle = jobm_eng_get_handle(0, 0);
		jobm_eng_isr_hw_reg(p_handle);
	} else if (irq == INT_ID_JMISP) {
		JMISP_ENG_HANDLE* p_handle = jmisp_eng_get_handle(0, 0);
		jmisp_eng_isr_hw_reg(p_handle);
	} else if (irq == INT_ID_UTL) {
		UTIL_ENG_HANDLE* p_handle = util_eng_get_handle(0, 0);
		util_eng_isr_hw_reg(p_handle);
	} else if (irq == INT_ID_ROU) {
		ROU_ENG_HANDLE* p_handle = rou_eng_get_handle(0, 0);
		rou_eng_isr_hw_reg(p_handle);
	} else if (irq == INT_ID_CAL) {
		CAL_ENG_HANDLE* p_handle = cal_eng_get_handle(0, 0);
		cal_eng_isr_hw_reg(p_handle);
	} else if (irq == INT_ID_POU) {
		POU_ENG_HANDLE* p_handle = pou_eng_get_handle(0, 0);
		pou_eng_isr_hw_reg(p_handle);
	}
	
	return IRQ_HANDLED;
}

void nvt_ai_drv_init_rtos(void)
{
	ai_eng_io_addr[0] = 0x2f0800000; // CONV
	ai_eng_io_addr[1] = 0x2f0ec0000; // NUE2
	ai_eng_io_addr[2] = 0x2f0760000; // PPU
	ai_eng_io_addr[3] = 0x2f0750000; // LSU
	ai_eng_io_addr[4] = 0x2f0700000; // JOBM
	ai_eng_io_addr[5] = 0x2f0710000; // JMISP
	if (nvt_get_chip_id() == CHIP_NS02302) {
	ai_eng_io_addr[6] = 0x2f0720000; // UTIL
	} else if (nvt_get_chip_id() == CHIP_NS02402) {
	ai_eng_io_addr[6] = 0x2f0770000; // POU
	}
	ai_eng_io_addr[7] = 0x2f0730000; // ROU
	ai_eng_io_addr[8] = 0x2f0740000; // CAL
	
	request_irq(INT_ID_CNN, ai_rtos_isr, IRQF_TRIGGER_HIGH, "cnn", 0);
	request_irq(INT_ID_NUE2, ai_rtos_isr, IRQF_TRIGGER_HIGH, "nue2", 0);
	request_irq(INT_ID_PPU, ai_rtos_isr, IRQF_TRIGGER_HIGH, "ppu", 0);
	request_irq(INT_ID_LSU, ai_rtos_isr, IRQF_TRIGGER_HIGH, "lsu", 0);
	request_irq(INT_ID_JM, ai_rtos_isr, IRQF_TRIGGER_HIGH, "jm", 0);
	request_irq(INT_ID_JMISP, ai_rtos_isr, IRQF_TRIGGER_HIGH, "jmisp", 0);
	if (nvt_get_chip_id() == CHIP_NS02302) {
	request_irq(INT_ID_UTL, ai_rtos_isr, IRQF_TRIGGER_HIGH, "util", 0);
	} else if (nvt_get_chip_id() == CHIP_NS02402) {
	request_irq(INT_ID_POU, ai_rtos_isr, IRQF_TRIGGER_HIGH, "pou", 0);
	}
	request_irq(INT_ID_ROU, ai_rtos_isr, IRQF_TRIGGER_HIGH, "rou", 0);
	request_irq(INT_ID_CAL, ai_rtos_isr, IRQF_TRIGGER_HIGH, "cal", 0);
	
	// set TCM4k reg bit
	SET_32BitsValue(0x2f07f0040, (GET_32BitsValue(0x2f07f0040) |0x10));
}

void nvt_ai_drv_uninit_rtos(void)
{
	free_irq(INT_ID_CNN, 0);
	free_irq(INT_ID_NUE2, 0);
	free_irq(INT_ID_PPU, 0);
	free_irq(INT_ID_LSU, 0);
	free_irq(INT_ID_JM, 0);
	free_irq(INT_ID_JMISP, 0);
	if (nvt_get_chip_id() == CHIP_NS02302) {
	free_irq(INT_ID_UTL, 0);
	} else if (nvt_get_chip_id() == CHIP_NS02402) {
	free_irq(INT_ID_POU, 0);
	}
	free_irq(INT_ID_ROU, 0);
	free_irq(INT_ID_CAL, 0);
}

int nvt_ai_ioctl(int fd, unsigned int uiCmd, void *p_arg)
{
	int iRet = 0;
	nvt_dbg(IND, "cmd:%x\n", uiCmd);

	switch (uiCmd) {
	/* Add other operations here */
	// engine init
	default:
		nvt_dbg(ERR, "unknown ai ioctl cmd: %08x\r\n", uiCmd);
		break;
	}

	return iRet;
}

#endif

