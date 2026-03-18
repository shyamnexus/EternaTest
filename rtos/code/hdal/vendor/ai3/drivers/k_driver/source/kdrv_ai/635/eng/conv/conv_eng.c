/*
    CONV module drconvr

    NT98690 CONV module drconvr.

    @file       conv_eng.c
    @ingconvp    mIIPPCONV
    @note       None

    Copyright   Novatek Microelectronics Corp. 2019.  All rights reserved.
*/
#include "kwrap/util.h"
#include "conv_eng_int_reg.h"
#include "conv_eng.h"
#include "conv_eng_handle.h"
#include "conv_eng_int_platform.h"
#include "conv_eng_int_dbg.h"

static CONV_ENG_CTL eng_ctl;
#define CONV_ENG_ID_TO_CH(chip_id, eng_id) ((chip_id * eng_ctl.eng_num) + eng_id)

/**
    Get register buffer size
    Get register buffer size

    @param[in] eng_id    :conv engine id param(IVE_ID Only)

    @return Register buffer size (unit: Byte), if error eng handle, return 0;
*/
UINT32 conv_eng_get_reg_base_buf_size(UINT32 eng_id)
{
	if (eng_id >= CONV_ID_MAX_NUM) {
		DBG_ERR("error conv engine id\r\n");
		return 0;
	}
	return (CONV_ENG_REG_NUMS << 2);
}

/**
    Get flg buffer size
    Get flg buffer size

    @param[in] eng_id    :conv engine id param(IVE_ID Only)

    @return Flg buffer size (unit: Byte), if error eng handle, return 0;
*/
UINT32 conv_eng_get_reg_flag_buf_size(UINT32 eng_id)
{
	if (eng_id >= CONV_ID_MAX_NUM) {
		DBG_ERR("error conv engine id\r\n");
		return 0;
	}
	return (CONV_ENG_REG_NUMS);
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
VOID conv_eng_set_reg_buf(CONV_ENG_HANDLE *p_eng, uintptr_t reg_base_addr, uintptr_t reg_flag_addr)
{
    p_eng->p_conv_reg_st = (NT98635_CONV_REGISTER_STRUCT *)reg_base_addr;
	p_eng->p_conv_reg_chg_flag = (UINT8 *)reg_flag_addr;
}

/**
    conv software reset
    conv software reset

    @param[in] p_eng    :Ive engine handle param

    @return status
        - @ -1   : Fail
        - @  0   : Pass
*/
static INT32 conv_eng_int_soft_reset_hw_reg(CONV_ENG_HANDLE *p_eng)
{
	if (p_eng == NULL) {
		DBG_ERR("conv handle parameter null\r\n");
		return -1;
	}

	CONV_ENG_SETREG(p_eng->reg_io_base + CONV_CONTROL_REGISTER_OFS, 0x1);
	CONV_ENG_SETREG(p_eng->reg_io_base + CONV_CONTROL_REGISTER_OFS, 0x0);

    return 0;
}

static INT32 conv_eng_int_dma_enable_hw_reg(CONV_ENG_HANDLE *p_eng, BOOL set_en)
{
    T_DMA_DISABLE_REGISTER0 ctl;

	if (p_eng == NULL) {
		DBG_ERR("CONV handle null\r\n");
		return -1;
	}

    ctl.reg = CONV_ENG_GETREG(p_eng->reg_io_base + DMA_DISABLE_REGISTER0_OFS);
    ctl.bit.DMA_DISABLE = set_en ? 0 : 1;
    CONV_ENG_SETREG(p_eng->reg_io_base + DMA_DISABLE_REGISTER0_OFS, ctl.reg);

    return 0;
}
/**
    Create conv ctrl param (memory)
    Create conv ctrl param (memory)

    @param[in] p_eng_ctl    :conv engine ctrl param

    @return status
        - @ -1  : Fail
        - @  0  : Pass
*/
INT32 conv_eng_init(UINT32 chip_num, UINT32 eng_num)
{
	UINT32 total_ch;

	total_ch = (chip_num * eng_num);
	if (total_ch == 0) {
		DBG_ERR("conv input parameter fail %d\r\n", total_ch);
		return -1;
	}

	if (eng_num > CONV_ID_MAX_NUM) {
		DBG_ERR("error conv engine num\r\n");
		return -1;
	}

	eng_ctl.p_eng = CONV_ENG_MALLOC(sizeof(CONV_ENG_HANDLE) * total_ch);
	if (eng_ctl.p_eng == NULL) {
		DBG_ERR("alloc buf(%ld) failed\r\n", sizeof(CONV_ENG_HANDLE) * (int)total_ch);
		return -1;
	}

	eng_ctl.chip_num = chip_num;
	eng_ctl.eng_num = eng_num;
	eng_ctl.total_ch= total_ch;

	return 0;
}

INT32 conv_eng_release(void)
{
	UINT32 i;

	if (eng_ctl.p_eng) {
		for (i = 0; i < eng_ctl.total_ch; i ++) {
			conv_eng_platform_release_irq(&eng_ctl.p_eng[i]);
			conv_eng_platform_unprepare_clk(&eng_ctl.p_eng[i]);
		}
	}

	CONV_ENG_FREE(eng_ctl.p_eng);
	memset((void *)&eng_ctl, 0, sizeof(CONV_ENG_CTL));

	return 0;
}

/**
    Create conv resource (flg/sem/preclk/irq)
    Create conv resource (flg/sem/preclk/irq)

    @param[in] p_eng    :conv engine handle param

    @return status
        - @ -1  :Fail
        - @  0  :Pass
*/
INT32 conv_eng_init_resource(CONV_ENG_HANDLE *p_eng)
{
	CONV_ENG_HANDLE *p_eng_hdl;
    CONV_ENG_HANDLE eng_hdl_back;

	p_eng_hdl = conv_eng_get_handle(p_eng->chip_id, p_eng->eng_id);
	if (p_eng_hdl == NULL) {
		DBG_ERR("conv handle null !!\r\n");
        return -1;
	}
	if (p_eng->eng_id >= CONV_ID_MAX_NUM) 
    {
        DBG_ERR("error conv engine id\r\n");
        return -1;
    }

	eng_hdl_back.flg_id_conv = p_eng_hdl->flg_id_conv;
	*p_eng_hdl = *p_eng;
	p_eng_hdl->flg_id_conv = eng_hdl_back.flg_id_conv;

	conv_eng_platform_set_clk_rate(p_eng_hdl);
	conv_eng_platform_prepare_clk(p_eng_hdl);
	conv_eng_platform_request_irq(p_eng_hdl);

	return 0;
}

CONV_ENG_HANDLE* conv_eng_get_handle(UINT32 chip_id, UINT32 eng_id)
{
	UINT32 idx = chip_id * eng_ctl.eng_num + eng_id;

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

void conv_eng_reg_isr_callback(CONV_ENG_HANDLE *p_eng, CONV_ISR_CB cb)
{
	if (p_eng == NULL) {
		return;
	}
	p_eng->isr_cb = cb;
}

INT32 conv_eng_open(CONV_ENG_HANDLE *p_eng)
{
	if (p_eng == NULL) {
		DBG_ERR(" conv handle null\r\n");
        return -1;
	}
	if (p_eng->eng_id >= CONV_ID_MAX_NUM) {
		DBG_ERR("error conv engine id (%d)\r\n", p_eng->eng_id);
		return -1;
	}
	conv_eng_platform_create_resource(p_eng);

	// enable clock & sram 
	if (conv_eng_platform_enable_clk(p_eng)) {
		DBG_ERR("enable CONV%d clk fail\r\n", p_eng->eng_id);
		return -1;
	}
	conv_eng_platform_disable_sram_shutdown(p_eng);

	// clear interrupt enable & status 
    CONV_ENG_SETREG(p_eng->reg_io_base + CONV_INTERRUPT_ENABLE_REGISTER_OFS, 0);
	CONV_ENG_SETREG(p_eng->reg_io_base + CONV_INTERRUPT_STATUS_REGISTER_OFS, CONV_ENG_INTERRUPT_ALL);

	// software reset 
	conv_eng_int_soft_reset_hw_reg(p_eng);

	// set axi enable at here 
	conv_eng_dma_channel_enable_hw_reg(p_eng, TRUE);
	conv_eng_axi_channel_enable_hw_reg(p_eng, TRUE);

	// enable cycle counting
    conv_eng_cycle_enable_hw_reg(p_eng, TRUE);
	
	return 0;
}

INT32 conv_eng_close(CONV_ENG_HANDLE *p_eng)
{
	if (p_eng == NULL) {
		return -1;
	}
	if (p_eng->eng_id >= CONV_ID_MAX_NUM) {
		DBG_ERR("error conv engine id\r\n");
		return -1;
	}
	conv_eng_platform_enable_sram_shutdown(p_eng);
	// disable clock 
	conv_eng_platform_disable_clk(p_eng);
	conv_eng_platform_release_resource(p_eng);

	return 0;
}

INT32 conv_eng_reset(CONV_ENG_HANDLE *p_eng)
{
    if(p_eng == NULL) {
        DBG_ERR("CONV handle null\r\n");
        return -1;
    }

    // DMA disable
    conv_eng_int_dma_enable_hw_reg(p_eng, FALSE);

    // wait DMA idle
    if(conv_eng_platform_dma_idle(p_eng)) {
        //DBG_ERR("CONV wait DMA idle fail\n\r");
		return -1;
    } else {
        // SW reset
        conv_eng_int_soft_reset_hw_reg(p_eng);
        // DMA enable
        conv_eng_int_dma_enable_hw_reg(p_eng, TRUE);
        // clear interrupt status
        conv_eng_clr_intr_status(p_eng, CONV_ENG_INTERRUPT_ALL);
		return 0;
    }
}

VOID conv_eng_axi_channel_enable_hw_reg(CONV_ENG_HANDLE *p_eng, BOOL set_en)
{
	T_AXI_REGISTER7 ctl;

	if (p_eng == NULL) {
		return;
	}

	ctl.reg = CONV_ENG_GETREG(p_eng->reg_io_base + AXI_REGISTER7_OFS);
    if (set_en)
	    ctl.bit.AXI_BUS_DISABLE = 0;
    else
	    ctl.bit.AXI_BUS_DISABLE = 1;
	CONV_ENG_SETREG(p_eng->reg_io_base + AXI_REGISTER7_OFS, ctl.reg);
}

VOID conv_eng_dma_channel_enable_hw_reg(CONV_ENG_HANDLE *p_eng, BOOL set_en)
{
	T_DMA_DISABLE_REGISTER0 ctl;

	if (p_eng == NULL) {
		return;
	}

	ctl.reg = CONV_ENG_GETREG(p_eng->reg_io_base + DMA_DISABLE_REGISTER0_OFS);
    if (set_en)
	    ctl.bit.DMA_DISABLE = 0;
    else
	    ctl.bit.DMA_DISABLE = 1;
	CONV_ENG_SETREG(p_eng->reg_io_base + DMA_DISABLE_REGISTER0_OFS, ctl.reg);
}

BOOL conv_eng_chk_dma_channel_idle_reg(CONV_ENG_HANDLE *p_eng)
{
	T_DMA_DISABLE_REGISTER0 ctl;

	if (p_eng == NULL) {
		return FALSE;
	}

	ctl.reg = CONV_ENG_GETREG(p_eng->reg_io_base + DMA_DISABLE_REGISTER0_OFS);
    if (ctl.bit.CONV_IDLE)
        return TRUE;
    else
        return FALSE;

    return FALSE;
}

void conv_eng_trig_single_hw_reg(CONV_ENG_HANDLE *p_eng)
{
	T_CONV_CONTROL_REGISTER ctl;

	if (p_eng == NULL) {
		return;
	}

	ctl.reg = CONV_ENG_GETREG(p_eng->reg_io_base + CONV_CONTROL_REGISTER_OFS);
	ctl.bit.CONV_START = 1;
	ctl.bit.JOB0_START = 0;
	ctl.bit.JOB1_START = 0;
	CONV_ENG_SETREG(p_eng->reg_io_base + CONV_CONTROL_REGISTER_OFS, ctl.reg);
}

void conv_eng_trig_ll_hw_reg(CONV_ENG_HANDLE *p_eng, uintptr_t ll_addr_msb, uintptr_t ll_addr_lsb, UINT32 ll_idx)
{
    T_DMA_TO_CONV_JOB_CHANNEL_REGISTER0 ll0_dma_lsb;
    T_DMA_TO_CONV_JOB_CHANNEL_REGISTER1 ll0_dma_msb;
    T_DMA_TO_CONV_JOB_CHANNEL_REGISTER2 ll1_dma_lsb;
    T_DMA_TO_CONV_JOB_CHANNEL_REGISTER3 ll1_dma_msb;
	T_CONV_CONTROL_REGISTER ctl;

	if (p_eng == NULL) {
		return;
	}
  
	if(ll_idx==1){
		ll1_dma_lsb.reg = ll_addr_lsb;
		ll1_dma_msb.reg = ll_addr_msb;
		CONV_ENG_SETREG(p_eng->reg_io_base + DMA_TO_CONV_JOB_CHANNEL_REGISTER2_OFS, ll1_dma_lsb.reg);
		CONV_ENG_SETREG(p_eng->reg_io_base + DMA_TO_CONV_JOB_CHANNEL_REGISTER3_OFS, ll1_dma_msb.reg);		
	}else{
		ll0_dma_lsb.reg = ll_addr_lsb;
		ll0_dma_msb.reg = ll_addr_msb;
		CONV_ENG_SETREG(p_eng->reg_io_base + DMA_TO_CONV_JOB_CHANNEL_REGISTER0_OFS, ll0_dma_lsb.reg);
		CONV_ENG_SETREG(p_eng->reg_io_base + DMA_TO_CONV_JOB_CHANNEL_REGISTER1_OFS, ll0_dma_msb.reg);
	}
	ctl.reg = CONV_ENG_GETREG(p_eng->reg_io_base + CONV_CONTROL_REGISTER_OFS);
	ctl.bit.CONV_START = 0;
	if(ll_idx==1)ctl.bit.JOB1_START = 1;
	else ctl.bit.JOB0_START = 1;
	CONV_ENG_SETREG(p_eng->reg_io_base + CONV_CONTROL_REGISTER_OFS, ctl.reg);
}

void conv_eng_isr_hw_reg(CONV_ENG_HANDLE *p_eng)
{
	UINT32 status = 0, status_inte;
	UINT32 status_ll0 = 0, status_ll1 = 0;
	UINT32 inte;
	FLGPTN flag = 0x0;

	if (p_eng == NULL) {
		DBG_ERR("parameter null\r\n");
		return ;
	}
	// get interrupt status & enable bit 
	status = CONV_ENG_GETREG(p_eng->reg_io_base + CONV_INTERRUPT_STATUS_REGISTER_OFS);
    //DBG_ERR("int sts = 0x%x\r\n", status);

	// clear interrupt status 
	inte = CONV_ENG_GETREG(p_eng->reg_io_base + CONV_INTERRUPT_ENABLE_REGISTER_OFS);
	status_inte = status & inte;

	if (status != 0) {
		flag = 0x0;
		if (status_inte & CONV_ENG_INTERRUPT_FRM_END) {
			flag |= FLGPTN_CONV_FRM_END;
		}
		if (status_inte & CONV_ENG_INTERRUPT_LL0_END) {
			flag |= FLGPTN_CONV_LL0_END;
			status_ll0 = 1;
		}
		if (status_inte & CONV_ENG_INTERRUPT_LL1_END) {
			flag |= FLGPTN_CONV_LL1_END;
			status_ll1 = 1;
		}
		if (status & CONV_ENG_INTERRUPT_LL0_ERR) {
			//flg |= FLGPTN_IME_LL_ERR;
			DBG_ERR("%s: linked-list0 error...\r\n", __func__);
		}
		if (status & CONV_ENG_INTERRUPT_LL1_ERR) {
			//flg |= FLGPTN_IME_LL_ERR;
			DBG_ERR("%s: linked-list1 error...\r\n", __func__);
		}
		if ((status & CONV_ENG_INTERRUPT_LL0_END)||(status & CONV_ENG_INTERRUPT_LL1_END)) {
            CONV_ENG_SETREG(p_eng->reg_io_base + CONV_INTERRUPT_STATUS_REGISTER_OFS, CONV_ENG_INTERRUPT_FRM_END);
		}
		conv_eng_platform_flg_set(p_eng, flag);
	}
	CONV_ENG_SETREG(p_eng->reg_io_base + CONV_INTERRUPT_STATUS_REGISTER_OFS, status_inte);

	// get interrupt status 
	if (p_eng->isr_cb != NULL) {
		if(status_ll0==1)p_eng->isr_cb(p_eng, status, NULL,0);
		if(status_ll1==1)p_eng->isr_cb(p_eng, status, NULL,1);
	}
}

/**
    Enable interrupt fuction of conv
    Enable interrupt fuction of conv

    @param[in]
        -@ p_eng   :Rou handle param
        -@int_en   :Which interrupt function

    @return Void
*/
INT32 conv_eng_set_intrpt_en(CONV_ENG_HANDLE *p_eng, UINT32 int_en)
{
	if (p_eng == NULL) {
		return -1;
	}
    if(p_eng->eng_id >=CONV_ID_MAX_NUM)
    {
		DBG_ERR("error conv engine id\r\n");
		return -1;
    }

	// must be used link list buffer
	p_eng->p_conv_reg_st->CONV_Register_1.Word = (UINT32)int_en;
	CONV_ENG_SETREG(p_eng->reg_io_base + CONV_INTERRUPT_ENABLE_REGISTER_OFS, int_en); 
	p_eng->p_conv_reg_chg_flag[1] = TRUE;
    return 0;
}

VOID conv_eng_clr_intr_status(CONV_ENG_HANDLE *p_eng, UINT32 uiIntrStatus)
{
	T_CONV_INTERRUPT_STATUS_REGISTER LocalReg;
	LocalReg.reg = uiIntrStatus;
	CONV_ENG_SETREG(p_eng->reg_io_base + CONV_INTERRUPT_STATUS_REGISTER_OFS, LocalReg.reg);
}

UINT32 conv_eng_clr_flg(CONV_ENG_HANDLE *p_eng, FLGPTN flg)
{
	return conv_eng_platform_flg_clear(p_eng, flg);
}
UINT32 conv_eng_wait_flg(CONV_ENG_HANDLE *p_eng, PFLGPTN p_flgptn, FLGPTN flg)
{
	return conv_eng_platform_flg_wait(p_eng, p_flgptn, flg);
}
UINT32 conv_eng_wait_flg_timeout(CONV_ENG_HANDLE *p_eng, PFLGPTN p_flgptn, FLGPTN flg, int timeout_tick)
{
	return conv_eng_platform_flg_wait_timeout(p_eng, p_flgptn, flg, timeout_tick);
}

void conv_eng_wait_framend_ll(CONV_ENG_HANDLE *p_eng, UINT32 ll_idx)
{
    FLGPTN uiflag;
    ER er_return = E_OK;
	if(ll_idx==1){
		er_return = conv_eng_platform_flg_wait(p_eng, &uiflag, FLGPTN_CONV_LL1_END);
	}else{
		er_return = conv_eng_platform_flg_wait(p_eng, &uiflag, FLGPTN_CONV_LL0_END);
	}
    if(er_return != E_OK) {
        DBG_ERR("CONV LL waitdone error\n\r");
    }
}

BOOL conv_ssdrv_proc_ver(void)
{
    DBG_DUMP("%s Version: %s %s %s\n", CONV_SSD_DRV_NAME, CONV_SSD_DRV_MODULE_VERSION, __DATE__, __TIME__);
    return TRUE;
}

VOID conv_eng_cycle_enable_hw_reg(CONV_ENG_HANDLE *p_eng, BOOL set_en)
{
    T_DESIGN_DEBUG_REGISTER0 ctl;

	if (p_eng == NULL) {
		return;
	}

	ctl.reg = CONV_ENG_GETREG(p_eng->reg_io_base + DESIGN_DEBUG_REGISTER0_OFS);
    if (set_en) {
	    ctl.bit.CYCLE_COUNT_EN = 1;
    } else {
	    ctl.bit.CYCLE_COUNT_EN = 0;
	}
	CONV_ENG_SETREG(p_eng->reg_io_base + DESIGN_DEBUG_REGISTER0_OFS, ctl.reg);
}

UINT32 conv_eng_get_eng_cycle(CONV_ENG_HANDLE *p_eng)
{
    return CONV_ENG_GETREG(p_eng->reg_io_base + CYCLE_COUNT_REGISTER1_OFS);
}

UINT32 conv_eng_get_ll_cycle(CONV_ENG_HANDLE *p_eng, UINT32 ll_idx)
{	
	if(ll_idx==1)
		return CONV_ENG_GETREG(p_eng->reg_io_base + CYCLE_COUNT_REGISTER0_OFS);
    else return CONV_ENG_GETREG(p_eng->reg_io_base + CYCLE_COUNT_REGISTER2_OFS);
}

UINT32 conv_eng_get_wait_cycle(CONV_ENG_HANDLE *p_eng)
{
    return CONV_ENG_GETREG(p_eng->reg_io_base + CYCLE_COUNT_REGISTER3_OFS);
}

UINT32 conv_eng_get_dram_bw(CONV_ENG_HANDLE *p_eng)
{
    return 0;//CONV_ENG_GETREG(p_eng->reg_io_base + RESERVED66_OFS);
}

UINT32 conv_eng_get_ub_bw(CONV_ENG_HANDLE *p_eng)
{
    return 0;//CONV_ENG_GETREG(p_eng->reg_io_base + RESERVED67_OFS);
}

BOOL conv_eng_get_dma_idle(CONV_ENG_HANDLE *p_eng)
{
    T_DMA_DISABLE_REGISTER0 ctl;

	if (p_eng == NULL) {
        DBG_ERR("CONV handle null\r\n");
		return FALSE;
	}

	ctl.reg = CONV_ENG_GETREG(p_eng->reg_io_base + DMA_DISABLE_REGISTER0_OFS);
    
    return ctl.bit.CONV_IDLE;
}

INT32 conv_eng_set_ll_setting(CONV_ENG_HANDLE *p_eng, CONV_LL_SETTING* ll_param)
{
	T_JOB_PRIORITY_REGISTER0 pri_mode;
	
	if (p_eng == NULL) {
		return -1;
	}
    if(p_eng->eng_id >=CONV_ID_MAX_NUM)
    {
		DBG_ERR("error conv engine id\r\n");
		return -1;
    }
	
	CONV_ENG_SETREG(p_eng->reg_io_base + LOCK_STATUS_REGISTER_OFS, ll_param->unlock_cycle);
	
	pri_mode.reg = CONV_ENG_GETREG(p_eng->reg_io_base + JOB_PRIORITY_REGISTER0_OFS);
	pri_mode.bit.JOB_PRIORITY_MODE = ll_param->priority_mode;
	CONV_ENG_SETREG(p_eng->reg_io_base + JOB_PRIORITY_REGISTER0_OFS, pri_mode.reg);
	
	CONV_ENG_SETREG(p_eng->reg_io_base + QOS_REGISTER0_OFS, ll_param->qos_tot_time);
	CONV_ENG_SETREG(p_eng->reg_io_base + QOS_REGISTER1_OFS, ll_param->qos_ocpy_time);
	CONV_ENG_SETREG(p_eng->reg_io_base + STATISTIC_QOS_REGISTER2_OFS, ll_param->sta_period_time);
    return 0;
}

UINT32 conv_eng_get_ll_sta_acc_cycle(CONV_ENG_HANDLE *p_eng, UINT32 ll_idx)
{	
	if(ll_idx==1)
		return CONV_ENG_GETREG(p_eng->reg_io_base + STATISTIC_QOS_REGISTER1_OFS);
    else return CONV_ENG_GETREG(p_eng->reg_io_base + STATISTIC_QOS_REGISTER0_OFS);
}

UINT32 conv_eng_get_out_chksum(CONV_ENG_HANDLE *p_eng)
{
	if (p_eng == NULL) {
        DBG_ERR("CONV handle null\r\n");
		return 0;
	}
    
	return CONV_ENG_GETREG(p_eng->reg_io_base + CHECKSUM_REGISTER8_OFS);
}

UINT32 conv_eng_get_qos_setting(CONV_ENG_HANDLE *p_eng)
{
    return CONV_ENG_GETREG(p_eng->reg_io_base + QOS_REGISTER0_OFS);
}

UINT32 conv_eng_get_clk_rate(CONV_ENG_HANDLE *p_eng)
{
	if (p_eng == NULL) {
		// print invalid message
		return 0;
	}
	return conv_eng_platform_get_clk_rate(p_eng);
}
