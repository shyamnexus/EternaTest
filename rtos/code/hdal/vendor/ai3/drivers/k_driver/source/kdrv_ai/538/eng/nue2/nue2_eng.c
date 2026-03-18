/*
    NUE2 module driver

    NT98538 NUE2 module driver.

    @file       nue2_eng.c
    @ingnue2p    mIIPPNUE2
    @note       None

    Copyright   Novatek Microelectronics Corp. 2023.  All rights reserved.
*/
#include "kwrap/util.h"
#include "nue2_eng_int_reg.h"
#include "nue2_eng.h"
#include "nue2_eng_handle.h"
#include "nue2_eng_int_platform.h"
#include "nue2_eng_int_dbg.h"

static NUE2_ENG_CTL eng_ctl;
#define NUE2_ENG_ID_TO_CH(chip_id, eng_id) ((chip_id * eng_ctl.eng_num) + eng_id)

/**
    Get register buffer size
    Get register buffer size

    @param[in] eng_id    :ive engine id param(IVE_ID Only)

    @return Register buffer size (unit: Byte), if error eng handle, return 0;
*/
UINT32 nue2_eng_get_reg_base_buf_size(UINT32 eng_id)
{
	if (eng_id >= NUE2_ID_MAX_NUM) {
		DBG_ERR("error nue2 engine id\r\n");
		return 0;
	}
	return (NUE2_ENG_REG_NUMS << 2);
}

/**
    Get flg buffer size
    Get flg buffer size

    @param[in] eng_id    :ive engine id param(IVE_ID Only)

    @return Flg buffer size (unit: Byte), if error eng handle, return 0;
*/
UINT32 nue2_eng_get_reg_flag_buf_size(UINT32 eng_id)
{
	if (eng_id >= NUE2_ID_MAX_NUM) {
		DBG_ERR("error nue2 engine id\r\n");
		return 0;
	}
	return (NUE2_ENG_REG_NUMS);
}

/**
    Set buffer address for register & flg
    Set buffer address for register & flg

    @param[in]
        -@ p_eng            :Ive handle param
        -@ reg_base_addr    :Register buffer address
        -@ reg_flag_addr    :Flg buffer address

    @return Void
*/
VOID nue2_eng_set_reg_buf(NUE2_ENG_HANDLE *p_eng, uintptr_t reg_base_addr, uintptr_t reg_flag_addr)
{
    p_eng->p_nue2_reg_st = (NT98538_NUE2_REGISTER_STRUCT *)reg_base_addr;
	p_eng->p_nue2_reg_chg_flag = (UINT8 *)reg_flag_addr;
}

/**
    nue2 software reset
    nue2 software reset

    @param[in] p_eng    :Ive engine handle param

    @return status
        - @ -1   : Fail
        - @  0   : Pass
*/
static INT32 nue2_eng_int_soft_reset_hw_reg(NUE2_ENG_HANDLE *p_eng)
{
	if (p_eng == NULL) {
		DBG_ERR("nue2 handle parameter null\r\n");
		return -1;
	}

	NUE2_ENG_SETREG(p_eng->reg_io_base + NUE2_CONTROL_REGISTER_OFS, 0x1);
	NUE2_ENG_SETREG(p_eng->reg_io_base + NUE2_CONTROL_REGISTER_OFS, 0x0);

    return 0;
}

static INT32 nue2_eng_int_dma_enable_hw_reg(NUE2_ENG_HANDLE *p_eng, BOOL set_en)
{
    T_NUE2_DMA_DISABLE_REGISTER0 ctl;

	if (p_eng == NULL) {
		DBG_ERR("NUE2 handle null\r\n");
		return -1;
	}

    ctl.reg = NUE2_ENG_GETREG(p_eng->reg_io_base + NUE2_DMA_DISABLE_REGISTER0_OFS);
    ctl.bit.DMA_DISABLE = set_en ? 0 : 1;
    NUE2_ENG_SETREG(p_eng->reg_io_base + NUE2_DMA_DISABLE_REGISTER0_OFS, ctl.reg);

    return 0;
}

/**
    Create nue2 ctrl param (memory)
    Create nue2 ctrl param (memory)

    @param[in] p_eng_ctl    :nue2 engine ctrl param

    @return status
        - @ -1  : Fail
        - @  0  : Pass
*/
INT32 nue2_eng_init(UINT32 chip_num, UINT32 eng_num)
{
	UINT32 total_ch;

	total_ch = (chip_num * eng_num);
	if (total_ch == 0) {
		DBG_ERR("nue2 input parameter fail %d\r\n", total_ch);
		return -1;
	}

	if (eng_num > NUE2_ID_MAX_NUM) {
		DBG_ERR("error ive engine num\r\n");
		return -1;
	}

	eng_ctl.p_eng = NUE2_ENG_MALLOC(sizeof(NUE2_ENG_HANDLE) * total_ch);
	if (eng_ctl.p_eng == NULL) {
		DBG_ERR("alloc buf(%ld) failed\r\n", sizeof(NUE2_ENG_HANDLE) * (int)total_ch);
		return -1;
	}

	eng_ctl.chip_num = chip_num;
	eng_ctl.eng_num = eng_num;
	eng_ctl.total_ch= total_ch;

	return 0;
}

INT32 nue2_eng_release(void)
{
	UINT32 i;

	if (eng_ctl.p_eng) {
		for (i = 0; i < eng_ctl.total_ch; i ++) {
			nue2_eng_platform_release_irq(&eng_ctl.p_eng[i]);
			nue2_eng_platform_unprepare_clk(&eng_ctl.p_eng[i]);
		}
	}

	NUE2_ENG_FREE(eng_ctl.p_eng);
	memset((void *)&eng_ctl, 0, sizeof(NUE2_ENG_CTL));

	return 0;
}

/**
    Create nue2 resource (flg/sem/preclk/irq)
    Create nue2 resource (flg/sem/preclk/irq)

    @param[in] p_eng    :nue2 engine handle param

    @return status
        - @ -1  :Fail
        - @  0  :Pass
*/
INT32 nue2_eng_init_resource(NUE2_ENG_HANDLE *p_eng)
{
	NUE2_ENG_HANDLE *p_eng_hdl;
    NUE2_ENG_HANDLE eng_hdl_back;

	p_eng_hdl = nue2_eng_get_handle(p_eng->chip_id, p_eng->eng_id);
	if (p_eng_hdl == NULL) {
		DBG_ERR("ive handle null !!\r\n");
        return -1;
	}
	if (p_eng->eng_id >= NUE2_ID_MAX_NUM) 
    {
        DBG_ERR("error ive engine id\r\n");
        return -1;
    }

	eng_hdl_back.flg_id_nue2 = p_eng_hdl->flg_id_nue2;
	*p_eng_hdl = *p_eng;
	p_eng_hdl->flg_id_nue2 = eng_hdl_back.flg_id_nue2;

	nue2_eng_platform_set_clk_rate(p_eng_hdl);
	nue2_eng_platform_prepare_clk(p_eng_hdl);
	nue2_eng_platform_request_irq(p_eng_hdl);

	return 0;
}

NUE2_ENG_HANDLE* nue2_eng_get_handle(UINT32 chip_id, UINT32 eng_id)
{
	UINT32 idx = chip_id * eng_id;

	if (eng_ctl.p_eng == NULL) {
		DBG_ERR("eng_ctl.p_eng == NULL\r\n");
		return NULL;
	}

	if (idx < eng_ctl.total_ch) {
		return &eng_ctl.p_eng[idx];
	} else {
		DBG_ERR("id overflow(%d %d) > (%d %d)\r\n", chip_id, eng_id, eng_ctl.chip_num, eng_ctl.eng_num);
    	return NULL;
    }
}

void nue2_eng_reg_isr_callback(NUE2_ENG_HANDLE *p_eng, NUE2_ISR_CB cb)
{
	if (p_eng == NULL) {
		return;
	}
	p_eng->isr_cb = cb;
}

INT32 nue2_eng_open(NUE2_ENG_HANDLE *p_eng)
{
	if (p_eng == NULL) {
		DBG_ERR(" nue2 handle null\r\n");
        return -1;
	}
	if (p_eng->eng_id >= NUE2_ID_MAX_NUM) {
		DBG_ERR("error ive engine id (%d)\r\n", p_eng->eng_id);
		return -1;
	}
	nue2_eng_platform_create_resource(p_eng);

	// enable clock & sram 
	if (nue2_eng_platform_enable_clk(p_eng)) {
		DBG_ERR("enable NUE2 clk fail\r\n");
		return -1;
	}
	nue2_eng_platform_disable_sram_shutdown(p_eng);

	// clear interrupt enable & status 
    NUE2_ENG_SETREG(p_eng->reg_io_base + NUE2_INTERRUPT_ENABLE_REGISTER_OFS, 0);
	NUE2_ENG_SETREG(p_eng->reg_io_base + NUE2_INTERRUPT_STATUS_REGISTER_OFS, NUE2_ENG_INTERRUPT_ALL);

	// software reset 
	nue2_eng_int_soft_reset_hw_reg(p_eng);

	// set axi enable at here 
	nue2_eng_dma_channel_enable_hw_reg(p_eng, TRUE);
	nue2_eng_axi_channel_enable_hw_reg(p_eng, TRUE);

	// enable cycle counting
    nue2_eng_cycle_enable_hw_reg(p_eng, TRUE);
	return 0;
}

INT32 nue2_eng_close(NUE2_ENG_HANDLE *p_eng)
{
	if (p_eng == NULL) {
		return -1;
	}
	if (p_eng->eng_id >= NUE2_ID_MAX_NUM) {
		DBG_ERR("error ive engine id\r\n");
		return -1;
	}
	nue2_eng_platform_enable_sram_shutdown(p_eng);
	// disable clock 
	nue2_eng_platform_disable_clk(p_eng);
	nue2_eng_platform_release_resource(p_eng);

	return 0;
}

INT32 nue2_eng_reset(NUE2_ENG_HANDLE *p_eng)
{
    if(p_eng == NULL) {
        DBG_ERR("NUE2 handle null\r\n");
        return -1;
    }

    // DMA disable
    nue2_eng_int_dma_enable_hw_reg(p_eng, FALSE);

    // wait DMA idle
    if(nue2_eng_platform_dma_idle(p_eng)) {
        //DBG_ERR("NUE2 wait DMA idle fail\n\r");
		return -1;
    } else {
        // SW reset
        nue2_eng_int_soft_reset_hw_reg(p_eng);

        // DMA enable
        nue2_eng_int_dma_enable_hw_reg(p_eng, TRUE);

        // clear interrupt status
        nue2_eng_clr_intr_status(p_eng, NUE2_ENG_INTERRUPT_ALL);
		return 0;
    }
}

VOID nue2_eng_dma_channel_enable_hw_reg(NUE2_ENG_HANDLE *p_eng, BOOL set_en)
{
	T_NUE2_DMA_DISABLE_REGISTER0 ctl;

	if (p_eng == NULL) {
		return;
	}

	ctl.reg = NUE2_ENG_GETREG(p_eng->reg_io_base + NUE2_DMA_DISABLE_REGISTER0_OFS);
    if (set_en)
	    ctl.bit.DMA_DISABLE = 0;
    else
	    ctl.bit.DMA_DISABLE = 1;
	NUE2_ENG_SETREG(p_eng->reg_io_base + NUE2_DMA_DISABLE_REGISTER0_OFS, ctl.reg);
}

VOID nue2_eng_axi_channel_enable_hw_reg(NUE2_ENG_HANDLE *p_eng, BOOL set_en)
{
	T_NUE2_AXI_REGISTER0 ctl;

	if (p_eng == NULL) {
		return;
	}

	ctl.reg = NUE2_ENG_GETREG(p_eng->reg_io_base + NUE2_AXI_REGISTER0_OFS);
    if (set_en)
	    ctl.bit.AXI_DIS = 0;
    else
	    ctl.bit.AXI_DIS = 1;
	NUE2_ENG_SETREG(p_eng->reg_io_base + NUE2_AXI_REGISTER0_OFS, ctl.reg);
}

BOOL nue2_eng_chk_dma_channel_idle_reg(NUE2_ENG_HANDLE *p_eng)
{
	T_NUE2_DMA_DISABLE_REGISTER0 ctl;

	if (p_eng == NULL) {
		return FALSE;
	}

	ctl.reg = NUE2_ENG_GETREG(p_eng->reg_io_base + NUE2_DMA_DISABLE_REGISTER0_OFS);
    if (ctl.bit.NUE2_IDLE)
        return TRUE;
    else
        return FALSE;

    return FALSE;
}

void nue2_eng_trig_single_hw_reg(NUE2_ENG_HANDLE *p_eng)
{
	T_NUE2_CONTROL_REGISTER ctl;

	if (p_eng == NULL) {
		return;
	}

	ctl.reg = NUE2_ENG_GETREG(p_eng->reg_io_base + NUE2_CONTROL_REGISTER_OFS);
	ctl.bit.NUE2_START = 1;
	ctl.bit.LL_FIRE = 0;
	NUE2_ENG_SETREG(p_eng->reg_io_base + NUE2_CONTROL_REGISTER_OFS, ctl.reg);
}

void nue2_eng_trig_ll_hw_reg(NUE2_ENG_HANDLE *p_eng, uintptr_t ll_addr_msb, uintptr_t ll_addr_lsb, UINT32 ll_idx)
{
    T_DMA_TO_NUE2_REGISTER3 ll_dma_lsb;
    T_DMA_TO_NUE2_MSB_REGISTER3 ll_dma_msb;
	T_NUE2_CONTROL_REGISTER ctl;

	if (p_eng == NULL) {
		return;
	}
    
	ll_dma_lsb.reg = ll_addr_lsb;
    ll_dma_msb.reg = ll_addr_msb;

	NUE2_ENG_SETREG(p_eng->reg_io_base + DMA_TO_NUE2_REGISTER3_OFS, ll_dma_lsb.reg);
    NUE2_ENG_SETREG(p_eng->reg_io_base + DMA_TO_NUE2_MSB_REGISTER3_OFS, ll_dma_msb.reg);
	//NUE2_ENG_SETREG(p_eng->reg_io_base + NUE2_INTERRUPT_ENABLE_REGISTER_OFS, NUE2_ENG_INTERRUPT_LL_ERR); //set ll_error enable befor trigger, and the int_en will update by ll_cmd

	ctl.reg = NUE2_ENG_GETREG(p_eng->reg_io_base + NUE2_CONTROL_REGISTER_OFS);
	ctl.bit.NUE2_START = 0;
	ctl.bit.LL_FIRE = 1;
	NUE2_ENG_SETREG(p_eng->reg_io_base + NUE2_CONTROL_REGISTER_OFS, ctl.reg);
}

void nue2_eng_isr_hw_reg(NUE2_ENG_HANDLE *p_eng)
{
	UINT32 status = 0, status_inte;
	UINT32 inte;
	FLGPTN flag = 0x0;

	if (p_eng == NULL) {
		DBG_ERR("parameter null\r\n");
		return ;
	}
	// get interrupt status & enable bit 
	status = NUE2_ENG_GETREG(p_eng->reg_io_base + NUE2_INTERRUPT_STATUS_REGISTER_OFS);
    //DBG_ERR("int sts = 0x%x\r\n", status);

	// clear interrupt status 
	inte = NUE2_ENG_GETREG(p_eng->reg_io_base + NUE2_INTERRUPT_ENABLE_REGISTER_OFS);
	status_inte = status & inte;

	if (status != 0) {
		flag = 0x0;
		if (status_inte & NUE2_ENG_INTERRUPT_FRM_END) {
			flag |= FLGPTN_NUE2_FRM_END;
		}
		if (status_inte & NUE2_ENG_INTERRUPT_LL_END) {
			flag |= FLGPTN_NUE2_LL_END;
		}
		if (status & NUE2_ENG_INTERRUPT_LL_ERR) {
			//flg |= FLGPTN_IME_LL_ERR;
			DBG_ERR("%s: linked-list error...\r\n", __func__);
		}
		if (status & NUE2_ENG_INTERRUPT_LL_END) {
            NUE2_ENG_SETREG(p_eng->reg_io_base + NUE2_INTERRUPT_STATUS_REGISTER_OFS, NUE2_ENG_INTERRUPT_FRM_END);
		}
		nue2_eng_platform_flg_set(p_eng, flag);
	}
	NUE2_ENG_SETREG(p_eng->reg_io_base + NUE2_INTERRUPT_STATUS_REGISTER_OFS, status_inte);

	// get interrupt status 
	if (p_eng->isr_cb != NULL) {
		p_eng->isr_cb(p_eng, status, NULL, 0);
	}
}

/**
    Enable interrupt fuction of nue2
    Enable interrupt fuction of nue2

    @param[in]
        -@ p_eng   :Rou handle param
        -@int_en   :Which interrupt function

    @return Void
*/
INT32 nue2_eng_set_intrpt_en(NUE2_ENG_HANDLE *p_eng, UINT32 int_en)
{
	if (p_eng == NULL) {
		return -1;
	}
    if(p_eng->eng_id >=NUE2_ID_MAX_NUM)
    {
		DBG_ERR("error ive engine id\r\n");
		return -1;
    }

	// must be used link list buffer
	p_eng->p_nue2_reg_st->NUE2_Register_15.Word = (UINT32)int_en;
	p_eng->p_nue2_reg_chg_flag[1] = TRUE;
	NUE2_ENG_SETREG(p_eng->reg_io_base + NUE2_INTERRUPT_ENABLE_REGISTER_OFS, int_en);
    return 0;
}

VOID nue2_eng_clr_intr_status(NUE2_ENG_HANDLE *p_eng, UINT32 uiIntrStatus)
{
	T_NUE2_INTERRUPT_STATUS_REGISTER LocalReg;
	LocalReg.reg = uiIntrStatus;
	NUE2_ENG_SETREG(p_eng->reg_io_base + NUE2_INTERRUPT_STATUS_REGISTER_OFS, LocalReg.reg);
}

UINT32 nue2_eng_clr_flg(NUE2_ENG_HANDLE *p_eng, FLGPTN flg)
{
	return nue2_eng_platform_flg_clear(p_eng, flg);
}
UINT32 nue2_eng_wait_flg(NUE2_ENG_HANDLE *p_eng, PFLGPTN p_flgptn, FLGPTN flg)
{
	return nue2_eng_platform_flg_wait(p_eng, p_flgptn, flg);
}
UINT32 nue2_eng_wait_flg_timeout(NUE2_ENG_HANDLE *p_eng, PFLGPTN p_flgptn, FLGPTN flg, int timeout_tick)
{
	return nue2_eng_platform_flg_wait_timeout(p_eng, p_flgptn, flg, timeout_tick);
}

void nue2_eng_wait_framend_ll(NUE2_ENG_HANDLE *p_eng, UINT32 ll_idx)
{
    FLGPTN uiflag;
    ER er_return = E_OK;

    er_return = nue2_eng_platform_flg_wait(p_eng, &uiflag, FLGPTN_NUE2_LL_END);
    if(er_return != E_OK) {
        DBG_ERR("NUE2 LL waitdone error\n\r");
    }
}

BOOL nue2_ssdrv_proc_ver(void)
{
    DBG_DUMP("%s Version: %s %s %s\n", NUE2_SSD_DRV_NAME, NUE2_SSD_DRV_MODULE_VERSION, __DATE__, __TIME__);
    return TRUE;
}

VOID nue2_eng_cycle_enable_hw_reg(NUE2_ENG_HANDLE *p_eng, BOOL set_en)
{
    T_NUE2_DEBUG_DESIGN_REGISTER ctl;

	if (p_eng == NULL) {
		return;
	}

	ctl.reg = NUE2_ENG_GETREG(p_eng->reg_io_base + NUE2_DEBUG_DESIGN_REGISTER_OFS);
    if (set_en) {
	    ctl.bit.CYCLE_COUNT_EN = 1;
    } else {
	    ctl.bit.CYCLE_COUNT_EN = 0;
	}
	NUE2_ENG_SETREG(p_eng->reg_io_base + NUE2_DEBUG_DESIGN_REGISTER_OFS, ctl.reg);
}

UINT32 nue2_eng_get_eng_cycle(NUE2_ENG_HANDLE *p_eng)
{
    return NUE2_ENG_GETREG(p_eng->reg_io_base + NUE2_CYCLE_COUNT_REGISTER0_OFS);
}

UINT32 nue2_eng_get_ll_cycle(NUE2_ENG_HANDLE *p_eng, UINT32 ll_idx)
{
    return NUE2_ENG_GETREG(p_eng->reg_io_base + NUE2_CYCLE_COUNT_REGISTER1_OFS);
}

UINT32 nue2_eng_get_wait_cycle(NUE2_ENG_HANDLE *p_eng)
{
    return NUE2_ENG_GETREG(p_eng->reg_io_base + NUE2_CYCLE_COUNT_REGISTER2_OFS);
}

UINT32 nue2_eng_get_dram_bw(NUE2_ENG_HANDLE *p_eng)
{
    return NUE2_ENG_GETREG(p_eng->reg_io_base + RESERVED_REGISTER10_OFS);
}

UINT32 nue2_eng_get_ub_bw(NUE2_ENG_HANDLE *p_eng)
{
    return NUE2_ENG_GETREG(p_eng->reg_io_base + RESERVED_REGISTER11_OFS);
}

BOOL nue2_eng_get_dma_idle(NUE2_ENG_HANDLE *p_eng)
{
    T_NUE2_DMA_DISABLE_REGISTER0 ctl;

	if (p_eng == NULL) {
        DBG_ERR("NUE2 handle null\r\n");
		return FALSE;
	}

	ctl.reg = NUE2_ENG_GETREG(p_eng->reg_io_base + NUE2_DMA_DISABLE_REGISTER0_OFS);
    
    return ctl.bit.NUE2_IDLE;
}

UINT32 nue2_eng_get_clk_rate(NUE2_ENG_HANDLE *p_eng)
{
	if (p_eng == NULL) {
		DBG_ERR("NUE2 handle null\r\n");
		return 0;
	}

	return nue2_eng_platform_get_clk_rate(p_eng);
}