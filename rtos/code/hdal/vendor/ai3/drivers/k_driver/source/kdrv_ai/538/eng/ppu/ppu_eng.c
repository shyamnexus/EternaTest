/*
    PPU module driver

    NT98538 PPU module driver.

    @file       ppu_eng.c
    @ingroup    mIIPPPPU
    @note       None

    Copyright   Novatek Microelectronics Corp. 2023.  All rights reserved.
*/
#include "kwrap/util.h"
#include "ppu_eng_int_reg.h"
#include "ppu_eng.h"
#include "ppu_eng_handle.h"
#include "ppu_eng_int_platform.h"
#include "ppu_eng_int_dbg.h"

static PPU_ENG_CTL eng_ctl;
#define PPU_ENG_ID_TO_CH(chip_id, eng_id) ((chip_id * eng_ctl.eng_num) + eng_id)

/**
    Get register buffer size
    Get register buffer size

    @param[in] eng_id    : PPU engine id param(PPU_ID Only)

    @return Register buffer size (unit: Byte), if error eng handle, return 0;
*/
UINT32 ppu_eng_get_reg_base_buf_size(UINT32 eng_id)
{
	if (eng_id >= PPU_ID_MAX_NUM) {
		DBG_ERR("PPU engine ID error (%d)\r\n", eng_id);
		return 0;
	}
	return (PPU_ENG_REG_NUMS << 2);
}

/**
    Get flg buffer size
    Get flg buffer size

    @param[in] eng_id    : PPU engine id param(PPU_ID Only)

    @return Flg buffer size (unit: Byte), if error eng handle, return 0;
*/
UINT32 ppu_eng_get_reg_flag_buf_size(UINT32 eng_id)
{
	if (eng_id >= PPU_ID_MAX_NUM) {
		DBG_ERR("PPU engine ID error (%d)\r\n", eng_id);
		return 0;
	}
	return (PPU_ENG_REG_NUMS);
}

/**
    Set buffer address for register & flg
    Set buffer address for register & flg

    @param[in]
        -@ p_eng            : PPU handle param
        -@ reg_base_addr    : Register buffer address
        -@ reg_flag_addr    : Flg buffer address

    @return Void
*/
VOID ppu_eng_set_reg_buf(PPU_ENG_HANDLE *p_eng, uintptr_t reg_base_addr, uintptr_t reg_flag_addr)
{
    p_eng->p_ppu_reg_st = (NT98538_PPU_REGISTER_STRUCT *)reg_base_addr;
	p_eng->p_ppu_reg_chg_flag = (UINT8 *)reg_flag_addr;
}

/**
    PPU software reset
    PPU software reset

    @param[in] p_eng    : PPU engine handle param

    @return status
        - @ -1   : Fail
        - @  0   : Pass
*/
static INT32 ppu_eng_int_soft_reset_hw_reg(PPU_ENG_HANDLE *p_eng)
{
    if (p_eng == NULL) {
        DBG_ERR("PPU handle null\r\n");
        return -1;
    }

    // NOTE: Do not retrieve register value, set PPU_SW_RST and write value back (it could re-start engine)
    PPU_ENG_SETREG(p_eng->reg_io_base + PPU_CONTROL_REGISTER_OFS, 0x1);
    PPU_ENG_SETREG(p_eng->reg_io_base + PPU_CONTROL_REGISTER_OFS, 0x0);

    return 0;
}

/**
    Create PPU ctrl param (memory)
    Create PPU ctrl param (memory)

    @param[in] p_eng_ctl    :PPU engine ctrl param

    @return status
        - @ -1  : Fail
        - @  0  : Pass
*/
INT32 ppu_eng_init(UINT32 chip_num, UINT32 eng_num)
{
	UINT32 total_ch;

	total_ch = (chip_num * eng_num);
	if (total_ch == 0) {
		DBG_ERR("PPU input parameter fail (%d)\r\n", total_ch);
		return -1;
	}

	if (eng_num > PPU_ID_MAX_NUM) {
		DBG_ERR("PPU engine num error (%d)\r\n", eng_num);
		return -1;
	}

	eng_ctl.p_eng = PPU_ENG_MALLOC(sizeof(PPU_ENG_HANDLE) * total_ch);
	if (eng_ctl.p_eng == NULL) {
		DBG_ERR("alloc buf (%ld) failed\r\n", sizeof(PPU_ENG_HANDLE) * (int)total_ch);
		return -1;
	}

	eng_ctl.chip_num = chip_num;
	eng_ctl.eng_num = eng_num;
	eng_ctl.total_ch= total_ch;

	return 0;
}

INT32 ppu_eng_release(VOID)
{
	UINT32 i;

	if (eng_ctl.p_eng) {
		for (i = 0; i < eng_ctl.total_ch; i ++) {
			ppu_eng_platform_release_irq(&eng_ctl.p_eng[i]);
			ppu_eng_platform_unprepare_clk(&eng_ctl.p_eng[i]);
		}
	}

	PPU_ENG_FREE(eng_ctl.p_eng);
	memset((VOID *)&eng_ctl, 0, sizeof(PPU_ENG_CTL));

	return 0;
}

/**
    Create PPU resource (flg/sem/preclk/irq)
    Create PPU resource (flg/sem/preclk/irq)

    @param[in] p_eng    : PPU engine handle param

    @return status
        - @ -1  :Fail
        - @  0  :Pass
*/
INT32 ppu_eng_init_resource(PPU_ENG_HANDLE *p_eng)
{
	PPU_ENG_HANDLE *p_eng_hdl;
    PPU_ENG_HANDLE eng_hdl_back;

	p_eng_hdl = ppu_eng_get_handle(p_eng->chip_id, p_eng->eng_id);
	if (p_eng_hdl == NULL) {
		DBG_ERR("PPU handle null!!\r\n");
        return -1;
	}
	if (p_eng->eng_id >= PPU_ID_MAX_NUM) 
    {
        DBG_ERR("PPU engine ID error (%d)\r\n", p_eng->eng_id);
        return -1;
    }
	eng_hdl_back.flg_id_ppu = p_eng_hdl->flg_id_ppu;
	*p_eng_hdl = *p_eng;
	p_eng_hdl->flg_id_ppu = eng_hdl_back.flg_id_ppu;

	ppu_eng_platform_set_clk_rate(p_eng_hdl);
	ppu_eng_platform_prepare_clk(p_eng_hdl);
	ppu_eng_platform_request_irq(p_eng_hdl);

	return 0;
}

PPU_ENG_HANDLE* ppu_eng_get_handle(UINT32 chip_id, UINT32 eng_id)
{
	UINT32 idx = chip_id * eng_id;

	if (eng_ctl.p_eng == NULL) {
		DBG_ERR("eng_ctl.p_eng == NULL\r\n");
		return NULL;
	}

	if (idx < eng_ctl.total_ch) {
		return &eng_ctl.p_eng[idx];
	} else {
		DBG_ERR("ID overflow (%d x %d) > (%d x %d)\r\n", chip_id, eng_id, eng_ctl.chip_num, eng_ctl.eng_num);
    	return NULL;
    }
}

VOID ppu_eng_reg_isr_callback(PPU_ENG_HANDLE *p_eng, PPU_ISR_CB cb)
{
	if (p_eng == NULL) {
        DBG_ERR("PPU handle null\r\n");
		return;
	}
	p_eng->isr_cb = cb;
}

INT32 ppu_eng_open(PPU_ENG_HANDLE *p_eng)
{
	if (p_eng == NULL) {
		DBG_ERR("PPU handle null\r\n");
        return -1;
	}
	if (p_eng->eng_id >= PPU_ID_MAX_NUM) {
		DBG_ERR("PPU engine ID error (%d)\r\n", p_eng->eng_id);
		return -1;
	}
	ppu_eng_platform_create_resource(p_eng);

	// enable clock & sram 
	if (ppu_eng_platform_enable_clk(p_eng)) {
		DBG_ERR("enable PPU clk fail\r\n");
		return -1;
	}
	ppu_eng_platform_disable_sram_shutdown(p_eng);

	// clear interrupt enable & status 
    PPU_ENG_SETREG(p_eng->reg_io_base + PPU_INTERRUPT_ENABLE_REGISTER_OFS, 0);
	PPU_ENG_SETREG(p_eng->reg_io_base + PPU_INTERRUPT_STATUS_REGISTER_OFS, PPU_ENG_INTERRUPT_ALL);

	// software reset 
	ppu_eng_int_soft_reset_hw_reg(p_eng);

	// set axi enable at here 
	ppu_eng_dma_channel_enable_hw_reg(p_eng, TRUE);
	ppu_eng_axi_channel_enable_hw_reg(p_eng, TRUE);

    // enable cycle counting
    ppu_eng_cycle_enable_hw_reg(p_eng, TRUE);

	return 0;
}

INT32 ppu_eng_close(PPU_ENG_HANDLE *p_eng)
{
	if (p_eng == NULL) {
        DBG_ERR("PPU handle null\r\n");
		return -1;
	}
	if (p_eng->eng_id >= PPU_ID_MAX_NUM) {
		DBG_ERR("PPU engine ID error (%d)\r\n", p_eng->eng_id);
		return -1;
	}
	ppu_eng_platform_enable_sram_shutdown(p_eng);
	// disable clock 
	ppu_eng_platform_disable_clk(p_eng);
	ppu_eng_platform_release_resource(p_eng);

	return 0;
}

INT32 ppu_eng_reset(PPU_ENG_HANDLE *p_eng)
{
    if(p_eng == NULL) {
        DBG_ERR("PPU handle null\r\n");
        return -1;
    }

    // DMA disable
    ppu_eng_dma_channel_enable_hw_reg(p_eng, FALSE);

    // wait DMA idle
    if(ppu_eng_platform_dma_idle(p_eng)) {
        DBG_ERR("PPU wait DMA idle fail\n\r");
        return -1;
    } else {
        // SW reset
        ppu_eng_int_soft_reset_hw_reg(p_eng);

        // DMA enable
        ppu_eng_dma_channel_enable_hw_reg(p_eng, TRUE);

        // clear interrupt status
        ppu_eng_clr_intr_status(p_eng, PPU_ENG_INTERRUPT_ALL);

        return 0;
    }
}

VOID ppu_eng_dma_channel_enable_hw_reg(PPU_ENG_HANDLE *p_eng, BOOL set_en)
{
	T_PPU_MISC_REGISTER_1 ctl;

	if (p_eng == NULL) {
        DBG_ERR("PPU handle null\r\n");
		return;
	}

	ctl.reg = PPU_ENG_GETREG(p_eng->reg_io_base + PPU_MISC_REGISTER_1_OFS);
    if (set_en) {
	    ctl.bit.DMA_DIS = 0;
    } else {
	    ctl.bit.DMA_DIS = 1;
	}
	PPU_ENG_SETREG(p_eng->reg_io_base + PPU_MISC_REGISTER_1_OFS, ctl.reg);
}

BOOL ppu_eng_chk_dma_channel_idle_reg(PPU_ENG_HANDLE *p_eng)
{
	T_PPU_MISC_REGISTER_1 ctl;

	if (p_eng == NULL) {
        DBG_ERR("PPU handle null\r\n");
		return FALSE;
	}

	ctl.reg = PPU_ENG_GETREG(p_eng->reg_io_base + PPU_MISC_REGISTER_1_OFS);
    if (ctl.bit.DMA_IDLE)
        return TRUE;
    else
        return FALSE;

    return FALSE;
}

VOID ppu_eng_axi_channel_enable_hw_reg(PPU_ENG_HANDLE *p_eng, BOOL set_en)
{
	T_PPU_CHANNEL_REGISTER_0 ctl;

	if (p_eng == NULL) {
        DBG_ERR("PPU handle null\r\n");
		return;
	}

	ctl.reg = PPU_ENG_GETREG(p_eng->reg_io_base + PPU_CHANNEL_REGISTER_0_OFS);
    if (set_en) {
	    ctl.bit.AXI_DIS = 0;
    } else {
	    ctl.bit.AXI_DIS = 1;
	}
	PPU_ENG_SETREG(p_eng->reg_io_base + PPU_CHANNEL_REGISTER_0_OFS, ctl.reg);
}

VOID ppu_eng_trig_single_hw_reg(PPU_ENG_HANDLE *p_eng)
{
	T_PPU_CONTROL_REGISTER ctl;

	if (p_eng == NULL) {
        DBG_ERR("PPU handle null\r\n");
		return;
	}

	ctl.reg = PPU_ENG_GETREG(p_eng->reg_io_base + PPU_CONTROL_REGISTER_OFS);
	ctl.bit.PPU_START = 1;
	ctl.bit.JOB_START = 0;
	PPU_ENG_SETREG(p_eng->reg_io_base + PPU_CONTROL_REGISTER_OFS, ctl.reg);
}

VOID ppu_eng_trig_ll_hw_reg(PPU_ENG_HANDLE *p_eng, uintptr_t ll_addr_msb, uintptr_t ll_addr_lsb, UINT32 ll_idx)
{
    T_PPU_DRAMUB_JOB_REGISTER_0 ll_dma_lsb;
    T_PPU_DRAMUB_JOB_REGISTER_1 ll_dma_msb;
	T_PPU_CONTROL_REGISTER ctl;

	if (p_eng == NULL) {
        DBG_ERR("PPU handle null\r\n");
		return;
	}
    
	ll_dma_lsb.reg = ll_addr_lsb;
    ll_dma_msb.reg = ll_addr_msb;

	PPU_ENG_SETREG(p_eng->reg_io_base + PPU_DRAMUB_JOB_REGISTER_0_OFS, ll_dma_lsb.reg);
    PPU_ENG_SETREG(p_eng->reg_io_base + PPU_DRAMUB_JOB_REGISTER_1_OFS, ll_dma_msb.reg);

	ctl.reg = PPU_ENG_GETREG(p_eng->reg_io_base + PPU_CONTROL_REGISTER_OFS);
	ctl.bit.PPU_START = 0;
	ctl.bit.JOB_START = 1;
	PPU_ENG_SETREG(p_eng->reg_io_base + PPU_CONTROL_REGISTER_OFS, ctl.reg);
}

VOID ppu_eng_isr_hw_reg(PPU_ENG_HANDLE *p_eng)
{
	UINT32 status = 0, status_inte;
	UINT32 inte;
	FLGPTN flag = 0x0;
	
	if (p_eng == NULL) {
		DBG_ERR("PPU handle null\r\n");
		return ;
	}
	// get interrupt status & enable bit 
	status = PPU_ENG_GETREG(p_eng->reg_io_base + PPU_INTERRUPT_STATUS_REGISTER_OFS);
    //DBG_ERR("int sts = 0x%x\r\n", status);

	// clear interrupt status 
	inte = PPU_ENG_GETREG(p_eng->reg_io_base + PPU_INTERRUPT_ENABLE_REGISTER_OFS);
	status_inte = status & inte;

	if (status != 0) {
		flag = 0x0;
		if (status_inte & PPU_ENG_INTERRUPT_FRM_END) {
			flag |= FLGPTN_PPU_FRM_END;
		}
		if (status_inte & PPU_ENG_INTERRUPT_LL_END) {
			flag |= FLGPTN_PPU_LL_END;
		}
		if (status & PPU_ENG_INTERRUPT_LL_ERR) {
			DBG_ERR("%s: PPU linked-list error\r\n", __func__);
		}
		if (status & PPU_ENG_INTERRUPT_LL_END) {
            PPU_ENG_SETREG(p_eng->reg_io_base + PPU_INTERRUPT_STATUS_REGISTER_OFS, PPU_ENG_INTERRUPT_FRM_END);
		}
		ppu_eng_platform_flg_set(p_eng, flag);
	}
	PPU_ENG_SETREG(p_eng->reg_io_base + PPU_INTERRUPT_STATUS_REGISTER_OFS, status_inte);

	// get interrupt status 
	if (p_eng->isr_cb != NULL) {
		p_eng->isr_cb(p_eng, status, NULL, 0);
	}
}

/**
    Enable interrupt fuction of PPU
    Enable interrupt fuction of PPU

    @param[in]
        -@ p_eng   : PPU handle param
        -@int_en   : Which interrupt function

    @return Void
*/
INT32 ppu_eng_set_intrpt_en(PPU_ENG_HANDLE *p_eng, UINT32 int_en)
{
	T_PPU_INTERRUPT_ENABLE_REGISTER LocalReg;
	LocalReg.reg = int_en;
	
	if (p_eng == NULL) {
        DBG_ERR("PPU handle null\r\n");
		return -1;
	}
    if(p_eng->eng_id >=PPU_ID_MAX_NUM)
    {
		DBG_ERR("PPU engine ID error (%d)\r\n", p_eng->eng_id);
		return -1;
    }
	
	PPU_ENG_SETREG(p_eng->reg_io_base + PPU_INTERRUPT_ENABLE_REGISTER_OFS, LocalReg.reg);

	// must be used link list buffer
	p_eng->p_ppu_reg_st->PPU_Register_1.Word = (UINT32)int_en;
	p_eng->p_ppu_reg_chg_flag[1] = TRUE;
    return 0;
}

VOID ppu_eng_clr_intr_status(PPU_ENG_HANDLE *p_eng, UINT32 uiIntrStatus)
{
	T_PPU_INTERRUPT_STATUS_REGISTER LocalReg;
	LocalReg.reg = uiIntrStatus;
	PPU_ENG_SETREG(p_eng->reg_io_base + PPU_INTERRUPT_STATUS_REGISTER_OFS, LocalReg.reg);
}

UINT32 ppu_eng_clr_flg(PPU_ENG_HANDLE *p_eng, FLGPTN flg)
{
	return ppu_eng_platform_flg_clear(p_eng, flg);
}

UINT32 ppu_eng_wait_flg(PPU_ENG_HANDLE *p_eng, PFLGPTN p_flgptn, FLGPTN flg)
{
	return ppu_eng_platform_flg_wait(p_eng, p_flgptn, flg);
}

UINT32 ppu_eng_wait_flg_timeout(PPU_ENG_HANDLE *p_eng, PFLGPTN p_flgptn, FLGPTN flg, int timeout_tick)
{
	return ppu_eng_platform_flg_wait_timeout(p_eng, p_flgptn, flg, timeout_tick);
}

VOID ppu_eng_wait_framend_ll(PPU_ENG_HANDLE *p_eng, UINT32 ll_idx)
{
    FLGPTN uiflag;
    ER er_return = E_OK;

    er_return = ppu_eng_platform_flg_wait(p_eng, &uiflag, FLGPTN_PPU_LLEND);
    if(er_return != E_OK) {
        DBG_ERR("PPU LL waitdone error\n\r");
    }
}

BOOL ppu_ssdrv_proc_ver(VOID)
{
    DBG_DUMP("%s Version: %s %s %s\n", PPU_SSD_DRV_NAME, PPU_SSD_DRV_MODULE_VERSION, __DATE__, __TIME__);
    return TRUE;
}

VOID ppu_eng_cycle_enable_hw_reg(PPU_ENG_HANDLE *p_eng, BOOL set_en)
{
    T_PPU_MISC_REGISTER_1 ctl;

	if (p_eng == NULL) {
        DBG_ERR("PPU handle null\r\n");
		return;
	}

	ctl.reg = PPU_ENG_GETREG(p_eng->reg_io_base + PPU_MISC_REGISTER_1_OFS);
    if (set_en) {
	    ctl.bit.CYCLE_EN = 1;
    } else {
	    ctl.bit.CYCLE_EN = 0;
	}
	PPU_ENG_SETREG(p_eng->reg_io_base + PPU_MISC_REGISTER_1_OFS, ctl.reg);
}

UINT32 ppu_eng_get_eng_cycle(PPU_ENG_HANDLE *p_eng)
{
    return PPU_ENG_GETREG(p_eng->reg_io_base + PPU_CYCLE_COUNT_REGISTER_0_OFS);
}

UINT32 ppu_eng_get_ll_cycle(PPU_ENG_HANDLE *p_eng, UINT32 ll_idx)
{
    return PPU_ENG_GETREG(p_eng->reg_io_base + PPU_CYCLE_COUNT_REGISTER_1_OFS);
}

UINT32 ppu_eng_get_wait_cycle(PPU_ENG_HANDLE *p_eng)
{
    return PPU_ENG_GETREG(p_eng->reg_io_base + PPU_CYCLE_COUNT_REGISTER_2_OFS);
}

UINT32 ppu_eng_get_dram_bw(PPU_ENG_HANDLE *p_eng)
{
    return PPU_ENG_GETREG(p_eng->reg_io_base + PPU_RESERVED_OFS_25);
}

UINT32 ppu_eng_get_ub_bw(PPU_ENG_HANDLE *p_eng)
{
    return PPU_ENG_GETREG(p_eng->reg_io_base + PPU_RESERVED_OFS_26);
}

BOOL ppu_eng_get_dma_idle(PPU_ENG_HANDLE *p_eng)
{
    T_PPU_MISC_REGISTER_1 ctl;

	if (p_eng == NULL) {
        DBG_ERR("PPU handle null\r\n");
		return FALSE;
	}

	ctl.reg = PPU_ENG_GETREG(p_eng->reg_io_base + PPU_MISC_REGISTER_1_OFS);
    
    return ctl.bit.DMA_IDLE;
}

UINT32 ppu_eng_get_clk_rate(PPU_ENG_HANDLE *p_eng)
{
	if (p_eng == NULL) {
		DBG_ERR("PPU handle null\r\n");
		return 0;
	}

	return ppu_eng_platform_get_clk_rate(p_eng);
}