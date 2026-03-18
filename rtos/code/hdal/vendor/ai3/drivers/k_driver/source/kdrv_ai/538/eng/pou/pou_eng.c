/*
    POU module driver

    NT98539A POU module driver.

    @file       pou_eng.c
    @ingpoup    mIIPPPOU
    @note       None

    Copyright   Novatek Microelectronics Corp. 2023.  All rights reserved.
*/
#include "kwrap/util.h"
#include "pou_eng_int_reg.h"
#include "pou_eng.h"
#include "pou_eng_handle.h"
#include "pou_eng_int_platform.h"
#include "pou_eng_int_dbg.h"

static POU_ENG_CTL eng_ctl;
#define POU_ENG_ID_TO_CH(chip_id, eng_id) ((chip_id * eng_ctl.eng_num) + eng_id)

/**
    Get register buffer size
    Get register buffer size

    @param[in] eng_id    : POU engine id param (POU_ID Only)

    @return Register buffer size (unit: Byte), if error eng handle, return 0;
*/
UINT32 pou_eng_get_reg_base_buf_size(UINT32 eng_id)
{
	if (eng_id >= POU_ID_MAX_NUM) {
		DBG_ERR("POU engine ID error (%d)\r\n", eng_id);
		return 0;
	}
	return (POU_ENG_REG_NUMS << 2);
}

/**
    Get flg buffer size
    Get flg buffer size

    @param[in] eng_id    : POU engine id param (POU_ID Only)

    @return Flg buffer size (unit: Byte), if error eng handle, return 0;
*/
UINT32 pou_eng_get_reg_flag_buf_size(UINT32 eng_id)
{
	if (eng_id >= POU_ID_MAX_NUM) {
		DBG_ERR("POU engine ID error (%d)\r\n", eng_id);
		return 0;
	}
	return (POU_ENG_REG_NUMS);
}

/**
    Set buffer address for register & flg
    Set buffer address for register & flg

    @param[in]
        -@ p_eng            : POU handle param
        -@ reg_base_addr    : Register buffer address
        -@ reg_flag_addr    : Flg buffer address

    @return Void
*/
VOID pou_eng_set_reg_buf(POU_ENG_HANDLE *p_eng, uintptr_t reg_base_addr, uintptr_t reg_flag_addr)
{
    p_eng->p_pou_reg_st = (NT98538_POU_REGISTER_STRUCT *)reg_base_addr;
	p_eng->p_pou_reg_chg_flag = (UINT8 *)reg_flag_addr;
}

/**
    POU software reset
    POU software reset

    @param[in] p_eng    : POU engine handle param

    @return status
        - @ -1   : Fail
        - @  0   : Pass
*/
static INT32 pou_eng_int_soft_reset_hw_reg(POU_ENG_HANDLE *p_eng)
{
	if (p_eng == NULL) {
		DBG_ERR("POU handle null\r\n");
		return -1;
	}

	// NOTE: Do not retrieve register value, set POU_SW_RST and write value back (it could re-start engine)
    POU_ENG_SETREG(p_eng->reg_io_base + POU_CONTROL_REGISTER_OFS, 0x1);
    POU_ENG_SETREG(p_eng->reg_io_base + POU_CONTROL_REGISTER_OFS, 0x0);

    return 0;
}

static INT32 pou_eng_int_dma_enable_hw_reg(POU_ENG_HANDLE *p_eng, BOOL set_en)
{
    T_POU_MISC_REGISTER_1_OFS ctl;

	if (p_eng == NULL) {
		DBG_ERR("POU handle null\r\n");
		return -1;
	}

    ctl.reg = POU_ENG_GETREG(p_eng->reg_io_base + POU_MISC_REGISTER_1_OFS);
    ctl.bit.DMA_DIS = set_en ? 0 : 1;
    POU_ENG_SETREG(p_eng->reg_io_base + POU_MISC_REGISTER_1_OFS, ctl.reg);

    return 0;
}

/**
    Create POU ctrl param (memory)
    Create POU ctrl param (memory)

    @param[in] p_eng_ctl    :POU engine ctrl param

    @return status
        - @ -1  : Fail
        - @  0  : Pass
*/
INT32 pou_eng_init(UINT32 chip_num, UINT32 eng_num)
{
	UINT32 total_ch;

	total_ch = (chip_num * eng_num);
	if (total_ch == 0) {
		DBG_ERR("POU input parameter fail (%d)\r\n", total_ch);
		return -1;
	}

	if (eng_num > POU_ID_MAX_NUM) {
		DBG_ERR("POU engine num error (%d)\r\n", eng_num);
		return -1;
	}

	eng_ctl.p_eng = POU_ENG_MALLOC(sizeof(POU_ENG_HANDLE) * total_ch);
	if (eng_ctl.p_eng == NULL) {
		DBG_ERR("alloc buf (%ld) failed\r\n", sizeof(POU_ENG_HANDLE) * (int)total_ch);
		return -1;
	}

	eng_ctl.chip_num = chip_num;
	eng_ctl.eng_num = eng_num;
	eng_ctl.total_ch= total_ch;

	return 0;
}

INT32 pou_eng_release(VOID)
{
	UINT32 i;

	if (eng_ctl.p_eng) {
		for (i = 0; i < eng_ctl.total_ch; i ++) {
			pou_eng_platform_release_irq(&eng_ctl.p_eng[i]);
			pou_eng_platform_unprepare_clk(&eng_ctl.p_eng[i]);
		}
	}

	POU_ENG_FREE(eng_ctl.p_eng);
	memset((VOID *)&eng_ctl, 0, sizeof(POU_ENG_CTL));

	return 0;
}

/**
    Create POU resource (flg/sem/preclk/irq)
    Create POU resource (flg/sem/preclk/irq)

    @param[in] p_eng    : POU engine handle param

    @return status
        - @ -1  :Fail
        - @  0  :Pass
*/
INT32 pou_eng_init_resource(POU_ENG_HANDLE *p_eng)
{
	POU_ENG_HANDLE *p_eng_hdl;
    POU_ENG_HANDLE eng_hdl_back;

	p_eng_hdl = pou_eng_get_handle(p_eng->chip_id, p_eng->eng_id);
	if (p_eng_hdl == NULL) {
		DBG_ERR("POU handle null!!\r\n");
        return -1;
	}
	if (p_eng->eng_id >= POU_ID_MAX_NUM) 
    {
        DBG_ERR("POU engine ID error (%d)\r\n", p_eng->eng_id);
        return -1;
    }

	eng_hdl_back.flg_id_pou = p_eng_hdl->flg_id_pou;
	*p_eng_hdl = *p_eng;
	p_eng_hdl->flg_id_pou = eng_hdl_back.flg_id_pou;

	pou_eng_platform_set_clk_rate(p_eng_hdl);
	pou_eng_platform_prepare_clk(p_eng_hdl);
	pou_eng_platform_request_irq(p_eng_hdl);

	return 0;
}

POU_ENG_HANDLE* pou_eng_get_handle(UINT32 chip_id, UINT32 eng_id)
{
	UINT32 idx = chip_id * eng_id;

	if (eng_ctl.p_eng == NULL) {
		DBG_ERR("eng_ctl.p_eng == NULL\r\n");
		return NULL;
	}

	if (idx < eng_ctl.total_ch) {
		return &eng_ctl.p_eng[idx];
	} else {
		DBG_ERR("ID overflow (%d %d) > (%d %d)\r\n", chip_id, eng_id, eng_ctl.chip_num, eng_ctl.eng_num);
    	return NULL;
    }
}

VOID pou_eng_reg_isr_callback(POU_ENG_HANDLE *p_eng, POU_ISR_CB cb)
{
	if (p_eng == NULL) {
		DBG_ERR("POU handle null\r\n");
		return;
	}
	p_eng->isr_cb = cb;
}

INT32 pou_eng_open(POU_ENG_HANDLE *p_eng)
{
	if (p_eng == NULL) {
		DBG_ERR("POU handle null\r\n");
        return -1;
	}
	if (p_eng->eng_id >= POU_ID_MAX_NUM) {
		DBG_ERR("POU engine ID error (%d)\r\n", p_eng->eng_id);
		return -1;
	}
	pou_eng_platform_create_resource(p_eng);

	// enable clock & sram 
	if (pou_eng_platform_enable_clk(p_eng)) {
		DBG_ERR("enable POU clk fail\r\n");
		return -1;
	}
	pou_eng_platform_disable_sram_shutdown(p_eng);

	// clear interrupt enable & status 
    POU_ENG_SETREG(p_eng->reg_io_base + POU_INTERRUPT_ENABLE_REGISTER_OFS, 0);
	POU_ENG_SETREG(p_eng->reg_io_base + POU_INTERRUPT_STATUS_REGISTER_OFS, POU_ENG_INTERRUPT_ALL);

	// software reset 
	pou_eng_int_soft_reset_hw_reg(p_eng);

	// set axi enable at here 
	pou_eng_dma_channel_enable_hw_reg(p_eng, TRUE);
	pou_eng_axi_channel_enable_hw_reg(p_eng, TRUE);

    // enable cycle counting
    pou_eng_cycle_enable_hw_reg(p_eng, TRUE);

	return 0;
}

INT32 pou_eng_close(POU_ENG_HANDLE *p_eng)
{
	if (p_eng == NULL) {
		DBG_ERR("POU handle null\r\n");
		return -1;
	}
	if (p_eng->eng_id >= POU_ID_MAX_NUM) {
		DBG_ERR("POU engine ID error (%d)\r\n", p_eng->eng_id);
		return -1;
	}
	pou_eng_platform_enable_sram_shutdown(p_eng);
	// disable clock 
	pou_eng_platform_disable_clk(p_eng);
	pou_eng_platform_release_resource(p_eng);

	return 0;
}

INT32 pou_eng_reset(POU_ENG_HANDLE *p_eng)
{
    if(p_eng == NULL) {
        DBG_ERR("POU handle null\r\n");
        return -1;
    }

    // DMA disable
    pou_eng_int_dma_enable_hw_reg(p_eng, FALSE);

    // wait DMA idle
    if(pou_eng_platform_dma_idle(p_eng)) {
        //DBG_ERR("POU wait DMA idle fail\n\r");
	return -1;
    } else {
        // SW reset
        pou_eng_int_soft_reset_hw_reg(p_eng);

        // DMA enable
        pou_eng_int_dma_enable_hw_reg(p_eng, TRUE);

        // clear interrupt status
        pou_eng_clr_intr_status(p_eng, POU_ENG_INTERRUPT_ALL);
    	return 0;
    }
}

VOID pou_eng_dma_channel_enable_hw_reg(POU_ENG_HANDLE *p_eng, BOOL set_en)
{
	T_POU_MISC_REGISTER_1_OFS ctl;

	if (p_eng == NULL) {
		DBG_ERR("POU handle null\r\n");
		return;
	}

	ctl.reg = POU_ENG_GETREG(p_eng->reg_io_base + POU_MISC_REGISTER_1_OFS);
    if (set_en) {
	    ctl.bit.DMA_DIS = 0;
    } else {
	    ctl.bit.DMA_DIS = 1;
	}
	POU_ENG_SETREG(p_eng->reg_io_base + POU_MISC_REGISTER_1_OFS, ctl.reg);
}

BOOL pou_eng_chk_dma_channel_idle_reg(POU_ENG_HANDLE *p_eng)
{
	T_POU_MISC_REGISTER_1_OFS ctl;

	if (p_eng == NULL) {
		DBG_ERR("POU handle null\r\n");
		return FALSE;
	}

	ctl.reg = POU_ENG_GETREG(p_eng->reg_io_base + POU_MISC_REGISTER_1_OFS);
    if (ctl.bit.DMA_IDLE)
        return TRUE;
    else
        return FALSE;

    return FALSE;
}

VOID pou_eng_axi_channel_enable_hw_reg(POU_ENG_HANDLE *p_eng, BOOL set_en)
{
	T_POU_CHANNEL_REGISTER_0_OFS ctl;

	if (p_eng == NULL) {
		DBG_ERR("POU handle null\r\n");
		return;
	}

	ctl.reg = POU_ENG_GETREG(p_eng->reg_io_base + POU_CHANNEL_REGISTER_0_OFS);
    if (set_en) {
	    ctl.bit.AXI_DIS = 0;
    } else {
	    ctl.bit.AXI_DIS = 1;
	}
	POU_ENG_SETREG(p_eng->reg_io_base + POU_CHANNEL_REGISTER_0_OFS, ctl.reg);
}

VOID pou_eng_trig_single_hw_reg(POU_ENG_HANDLE *p_eng)
{
	T_POU_CONTROL_REGISTER_OFS ctl;

	if (p_eng == NULL) {
		DBG_ERR("POU handle null\r\n");
		return;
	}

	ctl.reg = POU_ENG_GETREG(p_eng->reg_io_base + POU_CONTROL_REGISTER_OFS);
	ctl.bit.POU_START = 1;
	ctl.bit.JOB_START = 0;
	POU_ENG_SETREG(p_eng->reg_io_base + POU_CONTROL_REGISTER_OFS, ctl.reg);
}

VOID pou_eng_trig_ll_hw_reg(POU_ENG_HANDLE *p_eng, uintptr_t ll_addr_msb, uintptr_t ll_addr_lsb, UINT32 ll_idx)
{
    T_POU_LL_JOB_CHANNEL_REGISTER_0_OFS ll_dma_lsb;
    T_POU_LL_JOB_CHANNEL_REGISTER_1_OFS ll_dma_msb;

	T_POU_CONTROL_REGISTER_OFS ctl;

	if (p_eng == NULL) {
		DBG_ERR("POU handle null\r\n");
		return;
	}
    
	ll_dma_lsb.reg = ll_addr_lsb;
    ll_dma_msb.reg = ll_addr_msb;

	POU_ENG_SETREG(p_eng->reg_io_base + POU_LL_JOB_CHANNEL_REGISTER_0_OFS, ll_dma_lsb.reg);
    POU_ENG_SETREG(p_eng->reg_io_base + POU_LL_JOB_CHANNEL_REGISTER_1_OFS, ll_dma_msb.reg);
	//POU_ENG_SETREG(p_eng->reg_io_base + POU_INTERRUPT_ENABLE_REGISTER_OFS, POU_ENG_INTERRUPT_LL_ERR); //set ll_error enable befor trigger, and the int_en will update by ll_cmd

	ctl.reg = POU_ENG_GETREG(p_eng->reg_io_base + POU_CONTROL_REGISTER_OFS);
	ctl.bit.POU_START = 0;
	ctl.bit.JOB_START = 1;
	POU_ENG_SETREG(p_eng->reg_io_base + POU_CONTROL_REGISTER_OFS, ctl.reg);
}

VOID pou_eng_isr_hw_reg(POU_ENG_HANDLE *p_eng)
{
	UINT32 status = 0, status_inte;
	UINT32 inte;
	FLGPTN flag = 0x0;
	
	if (p_eng == NULL) {
		DBG_ERR("POU handle null\r\n");
		return ;
	}
	// get interrupt status & enable bit 
	status = POU_ENG_GETREG(p_eng->reg_io_base + POU_INTERRUPT_STATUS_REGISTER_OFS);
    //DBG_ERR("int sts = 0x%x\r\n", status);

	// clear interrupt status 
	inte = POU_ENG_GETREG(p_eng->reg_io_base + POU_INTERRUPT_ENABLE_REGISTER_OFS);
	status_inte = status & inte;

	if (status != 0) {
		flag = 0x0;
		if (status_inte & POU_ENG_INTERRUPT_FRM_END) {
			flag |= FLGPTN_POU_FRM_END;
		}
		if (status_inte & POU_ENG_INTERRUPT_LL_END) {
			flag |= FLGPTN_POU_LL_END;
		}
		if (status & POU_ENG_INTERRUPT_LL_ERR) {
			DBG_ERR("%s: POU linked-list error\r\n", __func__);
		}
		if (status & POU_ENG_INTERRUPT_LL_END) {
            POU_ENG_SETREG(p_eng->reg_io_base + POU_INTERRUPT_STATUS_REGISTER_OFS, POU_ENG_INTERRUPT_FRM_END);
		}
		pou_eng_platform_flg_set(p_eng, flag);
	}
	POU_ENG_SETREG(p_eng->reg_io_base + POU_INTERRUPT_STATUS_REGISTER_OFS, status_inte);

	// get interrupt status 
	if (p_eng->isr_cb != NULL) {
		p_eng->isr_cb(p_eng, status, NULL, 0);
	}
}

/**
    Enable interrupt fuction of POU
    Enable interrupt fuction of POU

    @param[in]
        -@ p_eng   : POU handle param
        -@int_en   : Which interrupt function

    @return Void
*/
INT32 pou_eng_set_intrpt_en(POU_ENG_HANDLE *p_eng, UINT32 int_en)
{
	T_POU_INTERRUPT_ENABLE_REGISTER_OFS LocalReg;
	LocalReg.reg = int_en;
	
	if (p_eng == NULL) {
		DBG_ERR("POU handle null\r\n");
		return -1;
	}
    if(p_eng->eng_id >= POU_ID_MAX_NUM)
    {
		DBG_ERR("POU engine ID error (%d)\r\n", p_eng->eng_id);
		return -1;
    }
	
	POU_ENG_SETREG(p_eng->reg_io_base + POU_INTERRUPT_ENABLE_REGISTER_OFS, LocalReg.reg);

	// must be used link list buffer
	p_eng->p_pou_reg_st->POU_Register_1.Word = (UINT32)int_en;
	p_eng->p_pou_reg_chg_flag[1] = TRUE;
    return 0;
}

VOID pou_eng_clr_intr_status(POU_ENG_HANDLE *p_eng, UINT32 uiIntrStatus)
{
	T_POU_INTERRUPT_STATUS_REGISTER_OFS LocalReg;
	LocalReg.reg = uiIntrStatus;
	POU_ENG_SETREG(p_eng->reg_io_base + POU_INTERRUPT_STATUS_REGISTER_OFS, LocalReg.reg);
}

UINT32 pou_eng_clr_flg(POU_ENG_HANDLE *p_eng, FLGPTN flg)
{
	return pou_eng_platform_flg_clear(p_eng, flg);
}

UINT32 pou_eng_wait_flg(POU_ENG_HANDLE *p_eng, PFLGPTN p_flgptn, FLGPTN flg)
{
	return pou_eng_platform_flg_wait(p_eng, p_flgptn, flg);
}

VOID pou_eng_wait_framend_ll(POU_ENG_HANDLE *p_eng, UINT32 ll_idx)
{
    FLGPTN uiflag;
    ER er_return = E_OK;

    er_return = pou_eng_platform_flg_wait(p_eng, &uiflag, FLGPTN_POU_LLEND);
    if(er_return != E_OK) {
        DBG_ERR("POU LL waitdone error\n\r");
    }
}

UINT32 pou_eng_wait_flg_timeout(POU_ENG_HANDLE *p_eng, PFLGPTN p_flgptn, FLGPTN flg, int timeout_tick)
{
	return pou_eng_platform_flg_wait_timeout(p_eng, p_flgptn, flg, timeout_tick);
}

BOOL pou_ssdrv_proc_ver(VOID)
{
    DBG_DUMP("%s Version: %s %s %s\n", POU_SSD_DRV_NAME, POU_SSD_DRV_MODULE_VERSION, __DATE__, __TIME__);
    return TRUE;
}

VOID pou_eng_cycle_enable_hw_reg(POU_ENG_HANDLE *p_eng, BOOL set_en)
{
    T_POU_MISC_REGISTER_1_OFS ctl;

	if (p_eng == NULL) {
		DBG_ERR("POU handle null\r\n");
		return;
	}

	ctl.reg = POU_ENG_GETREG(p_eng->reg_io_base + POU_MISC_REGISTER_1_OFS);
    if (set_en) {
	    ctl.bit.CYCLE_EN = 1;
    } else {
	    ctl.bit.CYCLE_EN = 0;
	}
	POU_ENG_SETREG(p_eng->reg_io_base + POU_MISC_REGISTER_1_OFS, ctl.reg);
}

UINT32 pou_eng_get_eng_cycle(POU_ENG_HANDLE *p_eng)
{
	if (p_eng == NULL) {
		DBG_ERR("POU handle null\r\n");
		return 0;
	}

    return POU_ENG_GETREG(p_eng->reg_io_base + POU_CYCLE_COUNT_REGISTER_0_OFS);
}

UINT32 pou_eng_get_ll_cycle(POU_ENG_HANDLE *p_eng, UINT32 ll_idx)
{
	if (p_eng == NULL) {
		DBG_ERR("POU handle null\r\n");
		return 0;
	}

    return POU_ENG_GETREG(p_eng->reg_io_base + POU_CYCLE_COUNT_REGISTER_1_OFS);
}

UINT32 pou_eng_get_wait_cycle(POU_ENG_HANDLE *p_eng)
{
	if (p_eng == NULL) {
		DBG_ERR("POU handle null\r\n");
		return 0;
	}

    return POU_ENG_GETREG(p_eng->reg_io_base + POU_CYCLE_COUNT_REGISTER_2_OFS);
}

UINT32 pou_eng_get_dram_bw(POU_ENG_HANDLE *p_eng)
{
	if (p_eng == NULL) {
		DBG_ERR("POU handle null\r\n");
		return 0;
	}

    return POU_ENG_GETREG(p_eng->reg_io_base + POU_RESERVED_REGISTER_0_OFS);
}

UINT32 pou_eng_get_ub_bw(POU_ENG_HANDLE *p_eng)
{
	if (p_eng == NULL) {
		DBG_ERR("POU handle null\r\n");
		return 0;
	}

    return POU_ENG_GETREG(p_eng->reg_io_base + POU_RESERVED_REGISTER_1_OFS);
}

BOOL pou_eng_get_dma_idle(POU_ENG_HANDLE *p_eng)
{
    T_POU_MISC_REGISTER_1_OFS ctl;

	if (p_eng == NULL) {
        DBG_ERR("POU handle null\r\n");
		return FALSE;
	}

	ctl.reg = POU_ENG_GETREG(p_eng->reg_io_base + POU_MISC_REGISTER_1_OFS);
    
    return ctl.bit.DMA_IDLE;
}

UINT32 pou_eng_get_clk_rate(POU_ENG_HANDLE *p_eng)
{
	if (p_eng == NULL) {
		DBG_ERR("POU handle null\r\n");
		return 0;
	}

	return pou_eng_platform_get_clk_rate(p_eng);
}