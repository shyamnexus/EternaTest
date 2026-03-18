/*
    LSU module driver

    NT98690 LSU module driver.

    @file       lsu_eng.c
    @inglsup    mIIPPLSU
    @note       None

    Copyright   Novatek Microelectronics Corp. 2019.  All rights reserved.
*/
#include "kwrap/util.h"
#include "lsu_eng_int_reg.h"
#include "lsu_eng.h"
#include "lsu_eng_handle.h"
#include "lsu_eng_int_platform.h"
#include "lsu_eng_int_dbg.h"

static LSU_ENG_CTL eng_ctl;
#define LSU_ENG_ID_TO_CH(chip_id, eng_id) ((chip_id * eng_ctl.eng_num) + eng_id)

/**
    Get register buffer size
    Get register buffer size

    @param[in] eng_id    : LSU engine id param(LSU_ID Only)

    @return Register buffer size (unit: Byte), if error eng handle, return 0;
*/
UINT32 lsu_eng_get_reg_base_buf_size(UINT32 eng_id)
{
	if (eng_id >= LSU_ID_MAX_NUM) {
		DBG_ERR("LSU engine ID error (%d)\r\n", eng_id);
		return 0;
	}
	return (LSU_ENG_REG_NUMS << 2);
}

/**
    Get flg buffer size
    Get flg buffer size

    @param[in] eng_id    : LSU engine id param(LSU_ID Only)

    @return Flg buffer size (unit: Byte), if error eng handle, return 0;
*/
UINT32 lsu_eng_get_reg_flag_buf_size(UINT32 eng_id)
{
	if (eng_id >= LSU_ID_MAX_NUM) {
		DBG_ERR("LSU engine ID error (%d)\r\n", eng_id);
		return 0;
	}
	return (LSU_ENG_REG_NUMS);
}

/**
    Set buffer address for register & flg
    Set buffer address for register & flg

    @param[in]
        -@ p_eng            : LSU handle param
        -@ reg_base_addr    : Register buffer address
        -@ reg_flag_addr    : Flg buffer address

    @return Void
*/
VOID lsu_eng_set_reg_buf(LSU_ENG_HANDLE *p_eng, uintptr_t reg_base_addr, uintptr_t reg_flag_addr)
{
    p_eng->p_lsu_reg_st = (NT98690_LSU_REGISTER_STRUCT *)reg_base_addr;
	p_eng->p_lsu_reg_chg_flag = (UINT8 *)reg_flag_addr;
}

/**
    LSU software reset
    LSU software reset

    @param[in] p_eng    : LSU engine handle param

    @return status
        - @ -1   : Fail
        - @  0   : Pass
*/
static INT32 lsu_eng_int_soft_reset_hw_reg(LSU_ENG_HANDLE *p_eng)
{
	if (p_eng == NULL) {
		DBG_ERR("LSU handle null\r\n");
		return -1;
	}

	// NOTE: Do not retrieve register value, set LSU_SW_RST and write value back (it could re-start engine)
    LSU_ENG_SETREG(p_eng->reg_io_base + LSU_CONTROL_REGISTER_OFS, 0x1);
    LSU_ENG_SETREG(p_eng->reg_io_base + LSU_CONTROL_REGISTER_OFS, 0x0);

    return 0;
}

static INT32 lsu_eng_int_dma_enable_hw_reg(LSU_ENG_HANDLE *p_eng, BOOL set_en)
{
    T_LSU_DMA_DISABLE_REGISTER_OFS ctl;

	if (p_eng == NULL) {
		DBG_ERR("LSU handle null\r\n");
		return -1;
	}

    ctl.reg = LSU_ENG_GETREG(p_eng->reg_io_base + LSU_DMA_DISABLE_REGISTER_OFS);
    ctl.bit.DMA_DISABLE = set_en ? 0 : 1;
    LSU_ENG_SETREG(p_eng->reg_io_base + LSU_DMA_DISABLE_REGISTER_OFS, ctl.reg);

    return 0;
}

/**
    Create LSU ctrl param (memory)
    Create LSU ctrl param (memory)

    @param[in] p_eng_ctl    :LSU engine ctrl param

    @return status
        - @ -1  : Fail
        - @  0  : Pass
*/
INT32 lsu_eng_init(UINT32 chip_num, UINT32 eng_num)
{
	UINT32 total_ch;

	total_ch = (chip_num * eng_num);
	if (total_ch == 0) {
		DBG_ERR("LSU input parameter fail (%d)\r\n", total_ch);
		return -1;
	}

	if (eng_num > LSU_ID_MAX_NUM) {
		DBG_ERR("LSU engine num error (%d)\r\n", eng_num);
		return -1;
	}

	eng_ctl.p_eng = LSU_ENG_MALLOC(sizeof(LSU_ENG_HANDLE) * total_ch);
	if (eng_ctl.p_eng == NULL) {
		DBG_ERR("alloc buf (%ld) failed\r\n", sizeof(LSU_ENG_HANDLE) * (int)total_ch);
		return -1;
	}

	eng_ctl.chip_num = chip_num;
	eng_ctl.eng_num = eng_num;
	eng_ctl.total_ch= total_ch;

	return 0;
}

INT32 lsu_eng_release(VOID)
{
	UINT32 i;

	if (eng_ctl.p_eng) {
		for (i = 0; i < eng_ctl.total_ch; i ++) {
			lsu_eng_platform_release_irq(&eng_ctl.p_eng[i]);
			lsu_eng_platform_unprepare_clk(&eng_ctl.p_eng[i]);
		}
	}

	LSU_ENG_FREE(eng_ctl.p_eng);
	memset((VOID *)&eng_ctl, 0, sizeof(LSU_ENG_CTL));

	return 0;
}

/**
    Create LSU resource (flg/sem/preclk/irq)
    Create LSU resource (flg/sem/preclk/irq)

    @param[in] p_eng    : LSU engine handle param

    @return status
        - @ -1  :Fail
        - @  0  :Pass
*/
INT32 lsu_eng_init_resource(LSU_ENG_HANDLE *p_eng)
{
	LSU_ENG_HANDLE *p_eng_hdl;
    LSU_ENG_HANDLE eng_hdl_back;

	p_eng_hdl = lsu_eng_get_handle(p_eng->chip_id, p_eng->eng_id);
	if (p_eng_hdl == NULL) {
		DBG_ERR("LSU handle null!!\r\n");
        return -1;
	}
	if (p_eng->eng_id >= LSU_ID_MAX_NUM) 
    {
        DBG_ERR("LSU engine ID error (%d)\r\n", p_eng->eng_id);
        return -1;
    }

	eng_hdl_back.flg_id_lsu = p_eng_hdl->flg_id_lsu;
	*p_eng_hdl = *p_eng;
	p_eng_hdl->flg_id_lsu = eng_hdl_back.flg_id_lsu;

	lsu_eng_platform_set_clk_rate(p_eng_hdl);
	lsu_eng_platform_prepare_clk(p_eng_hdl);
	lsu_eng_platform_request_irq(p_eng_hdl);

	return 0;
}

LSU_ENG_HANDLE* lsu_eng_get_handle(UINT32 chip_id, UINT32 eng_id)
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

VOID lsu_eng_reg_isr_callback(LSU_ENG_HANDLE *p_eng, LSU_ISR_CB cb)
{
	if (p_eng == NULL) {
		DBG_ERR("LSU handle null\r\n");
		return;
	}
	p_eng->isr_cb = cb;
}

INT32 lsu_eng_open(LSU_ENG_HANDLE *p_eng)
{
	if (p_eng == NULL) {
		DBG_ERR("LSU handle null\r\n");
        return -1;
	}
	if (p_eng->eng_id >= LSU_ID_MAX_NUM) {
		DBG_ERR("LSU engine ID error (%d)\r\n", p_eng->eng_id);
		return -1;
	}
	lsu_eng_platform_create_resource(p_eng);

	// enable clock & sram 
	if (lsu_eng_platform_enable_clk(p_eng)) {
		DBG_ERR("enable LSU clk fail\r\n");
		return -1;
	}
	lsu_eng_platform_disable_sram_shutdown(p_eng);

	// clear interrupt enable & status 
    LSU_ENG_SETREG(p_eng->reg_io_base + LSU_INTERRUPT_ENABLE_REGISTER_OFS, 0);
	LSU_ENG_SETREG(p_eng->reg_io_base + LSU_INTERRUPT_STATUS_REGISTER_OFS, LSU_ENG_INTERRUPT_ALL);

	// software reset 
	lsu_eng_int_soft_reset_hw_reg(p_eng);

	// set axi enable at here 
	lsu_eng_dma_channel_enable_hw_reg(p_eng, TRUE);
	lsu_eng_axi_channel_enable_hw_reg(p_eng, TRUE);

    // enable cycle counting
    lsu_eng_cycle_enable_hw_reg(p_eng, TRUE);

	return 0;
}

INT32 lsu_eng_close(LSU_ENG_HANDLE *p_eng)
{
	if (p_eng == NULL) {
		DBG_ERR("LSU handle null\r\n");
		return -1;
	}
	if (p_eng->eng_id >= LSU_ID_MAX_NUM) {
		DBG_ERR("LSU engine ID error (%d)\r\n", p_eng->eng_id);
		return -1;
	}
	lsu_eng_platform_enable_sram_shutdown(p_eng);
	// disable clock 
	lsu_eng_platform_disable_clk(p_eng);
	lsu_eng_platform_release_resource(p_eng);

	return 0;
}

INT32 lsu_eng_reset(LSU_ENG_HANDLE *p_eng)
{
    if(p_eng == NULL) {
        DBG_ERR("LSU handle null\r\n");
        return -1;
    }

    // DMA disable
    lsu_eng_int_dma_enable_hw_reg(p_eng, FALSE);

    // wait DMA idle
    if(lsu_eng_platform_dma_idle(p_eng)) {
        //DBG_ERR("LSU wait DMA idle fail\n\r");
	return -1;
    } else {
        // SW reset
        lsu_eng_int_soft_reset_hw_reg(p_eng);

        // DMA enable
        lsu_eng_int_dma_enable_hw_reg(p_eng, TRUE);

        // clear interrupt status
        lsu_eng_clr_intr_status(p_eng, LSU_ENG_INTERRUPT_ALL);
    	return 0;
    }
}

VOID lsu_eng_dma_channel_enable_hw_reg(LSU_ENG_HANDLE *p_eng, BOOL set_en)
{
	T_LSU_DMA_DISABLE_REGISTER_OFS ctl;

	if (p_eng == NULL) {
		DBG_ERR("LSU handle null\r\n");
		return;
	}

	ctl.reg = LSU_ENG_GETREG(p_eng->reg_io_base + LSU_DMA_DISABLE_REGISTER_OFS);
    if (set_en) {
	    ctl.bit.DMA_DISABLE = 0;
    } else {
	    ctl.bit.DMA_DISABLE = 1;
	}
	LSU_ENG_SETREG(p_eng->reg_io_base + LSU_DMA_DISABLE_REGISTER_OFS, ctl.reg);
}

BOOL lsu_eng_chk_dma_channel_idle_reg(LSU_ENG_HANDLE *p_eng)
{
	T_LSU_DMA_DISABLE_REGISTER_OFS ctl;

	if (p_eng == NULL) {
		DBG_ERR("LSU handle null\r\n");
		return FALSE;
	}

	ctl.reg = LSU_ENG_GETREG(p_eng->reg_io_base + LSU_DMA_DISABLE_REGISTER_OFS);
    if (ctl.bit.DMA_IDLE)
        return TRUE;
    else
        return FALSE;

    return FALSE;
}

VOID lsu_eng_axi_channel_enable_hw_reg(LSU_ENG_HANDLE *p_eng, BOOL set_en)
{
	T_LSU_AXI_REGISTER_1_OFS ctl;

	if (p_eng == NULL) {
		DBG_ERR("LSU handle null\r\n");
		return;
	}

	ctl.reg = LSU_ENG_GETREG(p_eng->reg_io_base + LSU_AXI_REGISTER_1_OFS);
    if (set_en) {
	    ctl.bit.AXI_BUS_DISABLE = 0;
    } else {
	    ctl.bit.AXI_BUS_DISABLE = 1;
	}
	LSU_ENG_SETREG(p_eng->reg_io_base + LSU_AXI_REGISTER_1_OFS, ctl.reg);
}

VOID lsu_eng_trig_single_hw_reg(LSU_ENG_HANDLE *p_eng)
{
	T_LSU_CONTROL_REGISTER_OFS ctl;

	if (p_eng == NULL) {
		DBG_ERR("LSU handle null\r\n");
		return;
	}

	ctl.reg = LSU_ENG_GETREG(p_eng->reg_io_base + LSU_CONTROL_REGISTER_OFS);
	ctl.bit.LSU_START = 1;
	ctl.bit.JOB_START = 0;
	LSU_ENG_SETREG(p_eng->reg_io_base + LSU_CONTROL_REGISTER_OFS, ctl.reg);
}

VOID lsu_eng_trig_ll_hw_reg(LSU_ENG_HANDLE *p_eng, uintptr_t ll_addr_msb, uintptr_t ll_addr_lsb)
{
    T_LSU_LL_JOB_CHANNEL_REGISTER_0_OFS ll_dma_lsb;
    T_LSU_LL_JOB_CHANNEL_REGISTER_1_OFS ll_dma_msb;

	T_LSU_CONTROL_REGISTER_OFS ctl;

	if (p_eng == NULL) {
		DBG_ERR("LSU handle null\r\n");
		return;
	}
    
	ll_dma_lsb.reg = ll_addr_lsb;
    ll_dma_msb.reg = ll_addr_msb;

	LSU_ENG_SETREG(p_eng->reg_io_base + LSU_LL_JOB_CHANNEL_REGISTER_0_OFS, ll_dma_lsb.reg);
    LSU_ENG_SETREG(p_eng->reg_io_base + LSU_LL_JOB_CHANNEL_REGISTER_1_OFS, ll_dma_msb.reg);
	//LSU_ENG_SETREG(p_eng->reg_io_base + LSU_INTERRUPT_ENABLE_REGISTER_OFS, LSU_ENG_INTERRUPT_LL_ERR); //set ll_error enable befor trigger, and the int_en will update by ll_cmd

	ctl.reg = LSU_ENG_GETREG(p_eng->reg_io_base + LSU_CONTROL_REGISTER_OFS);
	ctl.bit.LSU_START = 0;
	ctl.bit.JOB_START = 1;
	LSU_ENG_SETREG(p_eng->reg_io_base + LSU_CONTROL_REGISTER_OFS, ctl.reg);
}

VOID lsu_eng_isr_hw_reg(LSU_ENG_HANDLE *p_eng)
{
	UINT32 status = 0, status_inte;
	UINT32 inte;
	FLGPTN flag = 0x0;
	
	if (p_eng == NULL) {
		DBG_ERR("LSU handle null\r\n");
		return ;
	}
	// get interrupt status & enable bit 
	status = LSU_ENG_GETREG(p_eng->reg_io_base + LSU_INTERRUPT_STATUS_REGISTER_OFS);
    //DBG_ERR("int sts = 0x%x\r\n", status);

	// clear interrupt status 
	inte = LSU_ENG_GETREG(p_eng->reg_io_base + LSU_INTERRUPT_ENABLE_REGISTER_OFS);
	status_inte = status & inte;

	if (status != 0) {
		flag = 0x0;
		if (status_inte & LSU_ENG_INTERRUPT_FRM_END) {
			flag |= FLGPTN_LSU_FRM_END;
		}
		if (status_inte & LSU_ENG_INTERRUPT_LL_END) {
			flag |= FLGPTN_LSU_LL_END;
		}
		if (status & LSU_ENG_INTERRUPT_LL_ERR) {
			DBG_ERR("%s: LSU linked-list error\r\n", __func__);
		}
		if (status & LSU_ENG_INTERRUPT_LEN_ERR) {
			DBG_ERR("%s: LSU ACD bitstream lens error\r\n", __func__);
		}
		if (status & LSU_ENG_INTERRUPT_LL_END) {
            LSU_ENG_SETREG(p_eng->reg_io_base + LSU_INTERRUPT_STATUS_REGISTER_OFS, LSU_ENG_INTERRUPT_FRM_END);
		}
		lsu_eng_platform_flg_set(p_eng, flag);
	}
	LSU_ENG_SETREG(p_eng->reg_io_base + LSU_INTERRUPT_STATUS_REGISTER_OFS, status_inte);

	// get interrupt status 
	if (p_eng->isr_cb != NULL) {
		p_eng->isr_cb(p_eng, status, NULL);
	}
}

/**
    Enable interrupt fuction of LSU
    Enable interrupt fuction of LSU

    @param[in]
        -@ p_eng   : LSU handle param
        -@int_en   : Which interrupt function

    @return Void
*/
INT32 lsu_eng_set_intrpt_en(LSU_ENG_HANDLE *p_eng, UINT32 int_en)
{
	T_LSU_INTERRUPT_ENABLE_REGISTER_OFS LocalReg;
	LocalReg.reg = int_en;
	
	if (p_eng == NULL) {
		DBG_ERR("LSU handle null\r\n");
		return -1;
	}
    if(p_eng->eng_id >=LSU_ID_MAX_NUM)
    {
		DBG_ERR("LSU engine ID error (%d)\r\n", p_eng->eng_id);
		return -1;
    }
	
	LSU_ENG_SETREG(p_eng->reg_io_base + LSU_INTERRUPT_ENABLE_REGISTER_OFS, LocalReg.reg);

	// must be used link list buffer
	p_eng->p_lsu_reg_st->LSU_Register_1.Word = (UINT32)int_en;
	p_eng->p_lsu_reg_chg_flag[1] = TRUE;
    return 0;
}

VOID lsu_eng_clr_intr_status(LSU_ENG_HANDLE *p_eng, UINT32 uiIntrStatus)
{
	T_LSU_INTERRUPT_STATUS_REGISTER_OFS LocalReg;
	LocalReg.reg = uiIntrStatus;
	LSU_ENG_SETREG(p_eng->reg_io_base + LSU_INTERRUPT_STATUS_REGISTER_OFS, LocalReg.reg);
}

UINT32 lsu_eng_clr_flg(LSU_ENG_HANDLE *p_eng, FLGPTN flg)
{
	return lsu_eng_platform_flg_clear(p_eng, flg);
}

UINT32 lsu_eng_wait_flg(LSU_ENG_HANDLE *p_eng, PFLGPTN p_flgptn, FLGPTN flg)
{
	return lsu_eng_platform_flg_wait(p_eng, p_flgptn, flg);
}

VOID lsu_eng_wait_framend_ll(LSU_ENG_HANDLE *p_eng)
{
    FLGPTN uiflag;
    ER er_return = E_OK;

    er_return = lsu_eng_platform_flg_wait(p_eng, &uiflag, FLGPTN_LSU_LLEND);
    if(er_return != E_OK) {
        DBG_ERR("LSU LL waitdone error\n\r");
    }
}

UINT32 lsu_eng_wait_flg_timeout(LSU_ENG_HANDLE *p_eng, PFLGPTN p_flgptn, FLGPTN flg, int timeout_tick)
{
	return lsu_eng_platform_flg_wait_timeout(p_eng, p_flgptn, flg, timeout_tick);
}

BOOL lsu_ssdrv_proc_ver(VOID)
{
    DBG_DUMP("%s Version: %s %s %s\n", LSU_SSD_DRV_NAME, LSU_SSD_DRV_MODULE_VERSION, __DATE__, __TIME__);
    return TRUE;
}

VOID lsu_eng_cycle_enable_hw_reg(LSU_ENG_HANDLE *p_eng, BOOL set_en)
{
    T_LSU_DISIGN_DEBUG_REGISTER_OFS ctl;

	if (p_eng == NULL) {
		DBG_ERR("LSU handle null\r\n");
		return;
	}

	ctl.reg = LSU_ENG_GETREG(p_eng->reg_io_base + LSU_DISIGN_DEBUG_REGISTER_OFS);
    if (set_en) {
	    ctl.bit.CYCLE_COUNT_EN = 1;
    } else {
	    ctl.bit.CYCLE_COUNT_EN = 0;
	}
	LSU_ENG_SETREG(p_eng->reg_io_base + LSU_DISIGN_DEBUG_REGISTER_OFS, ctl.reg);
}

UINT32 lsu_eng_get_eng_cycle(LSU_ENG_HANDLE *p_eng)
{
    return LSU_ENG_GETREG(p_eng->reg_io_base + LSU_CYCLE_COUNT_REGISTER_0_OFS);
}

UINT32 lsu_eng_get_ll_cycle(LSU_ENG_HANDLE *p_eng)
{
    return LSU_ENG_GETREG(p_eng->reg_io_base + LSU_CYCLE_COUNT_REGISTER_1_OFS);
}

UINT32 lsu_eng_get_wait_cycle(LSU_ENG_HANDLE *p_eng)
{
    return LSU_ENG_GETREG(p_eng->reg_io_base + LSU_CYCLE_COUNT_REGISTER_2_OFS);
}

UINT32 lsu_eng_get_dram_bw(LSU_ENG_HANDLE *p_eng)
{
    return LSU_ENG_GETREG(p_eng->reg_io_base + LSU_RESERVED_REGISTER_11_OFS);
}

UINT32 lsu_eng_get_ub_bw(LSU_ENG_HANDLE *p_eng)
{
    return LSU_ENG_GETREG(p_eng->reg_io_base + LSU_RESERVED_REGISTER_12_OFS);
}

BOOL lsu_eng_get_dma_idle(LSU_ENG_HANDLE *p_eng)
{
    T_LSU_DMA_DISABLE_REGISTER_OFS ctl;

	if (p_eng == NULL) {
        DBG_ERR("LSU handle null\r\n");
		return FALSE;
	}

	ctl.reg = LSU_ENG_GETREG(p_eng->reg_io_base + LSU_DMA_DISABLE_REGISTER_OFS);
    
    return ctl.bit.DMA_IDLE;
}

UINT32 lsu_eng_get_clk_rate(LSU_ENG_HANDLE *p_eng)
{
	if (p_eng == NULL) {
		DBG_ERR("LSU handle null\r\n");
		return 0;
	}

	return lsu_eng_platform_get_clk_rate(p_eng);
}