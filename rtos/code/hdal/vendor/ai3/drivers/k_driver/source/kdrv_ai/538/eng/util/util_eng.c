/*
    UTIL module driver

    NT98538 UTIL module driver.

    @file       util_eng.c
    @ingutilp    mIIPPUTIL
    @note       None

    Copyright   Novatek Microelectronics Corp. 2023.  All rights reserved.
*/
#include "kwrap/util.h"
#include "util_eng_int_reg.h"
#include "util_eng.h"
#include "util_eng_handle.h"
#include "util_eng_int_platform.h"
#include "util_eng_int_dbg.h"

static UTIL_ENG_CTL eng_ctl;
#define UTIL_ENG_ID_TO_CH(chip_id, eng_id) ((chip_id * eng_ctl.eng_num) + eng_id)

/**
    Get register buffer size
    Get register buffer size

    @param[in] eng_id    : UTIL engine id param(UTIL_ID Only)

    @return Register buffer size (unit: Byte), if error eng handle, return 0;
*/
UINT32 util_eng_get_reg_base_buf_size(UINT32 eng_id)
{
	if (eng_id >= UTIL_ID_MAX_NUM) {
		DBG_ERR("UTIL engine ID error (%d)\r\n", eng_id);
		return 0;
	}
	return (UTIL_ENG_REG_NUMS << 2);
}

/**
    Get flg buffer size
    Get flg buffer size

    @param[in] eng_id    : UTIL engine id param(UTIL_ID Only)

    @return Flg buffer size (unit: Byte), if error eng handle, return 0;
*/
UINT32 util_eng_get_reg_flag_buf_size(UINT32 eng_id)
{
	if (eng_id >= UTIL_ID_MAX_NUM) {
		DBG_ERR("UTIL engine ID error (%d)\r\n", eng_id);
		return 0;
	}
	return (UTIL_ENG_REG_NUMS);
}

/**
    Set buffer address for register & flg
    Set buffer address for register & flg

    @param[in]
        -@ p_eng            : UTIL handle param
        -@ reg_base_addr    : Register buffer address
        -@ reg_flag_addr    : Flg buffer address

    @return Void
*/
VOID util_eng_set_reg_buf(UTIL_ENG_HANDLE *p_eng, uintptr_t reg_base_addr, uintptr_t reg_flag_addr)
{
    p_eng->p_util_reg_st = (NT98538_UTIL_REGISTER_STRUCT *)reg_base_addr;
	p_eng->p_util_reg_chg_flag = (UINT8 *)reg_flag_addr;
}

/**
    UTIL software reset
    UTIL software reset

    @param[in] p_eng    : UTIL engine handle param

    @return status
        - @ -1   : Fail
        - @  0   : Pass
*/
static INT32 util_eng_int_soft_reset_hw_reg(UTIL_ENG_HANDLE *p_eng)
{

	if (p_eng == NULL) {
		DBG_ERR("UTIL handle parameter null\r\n");
		return -1;
	}

	UTIL_ENG_SETREG(p_eng->reg_io_base + UTIL_CONTROL_REGISTER_OFS, 0x1);
	UTIL_ENG_SETREG(p_eng->reg_io_base + UTIL_CONTROL_REGISTER_OFS, 0x0);

    return 0;
}

static INT32 util_eng_int_dma_enable_hw_reg(UTIL_ENG_HANDLE *p_eng, BOOL set_en)
{
    T_UTIL_DMA_CHANNEL_STATUS_REGISTER ctl;

	if (p_eng == NULL) {
		DBG_ERR("UTIL handle null\r\n");
		return -1;
	}

    ctl.reg = UTIL_ENG_GETREG(p_eng->reg_io_base + UTIL_DMA_CHANNEL_STATUS_REGISTER_OFS);
    ctl.bit.DMA_DISABLE = set_en ? 0 : 1;
    UTIL_ENG_SETREG(p_eng->reg_io_base + UTIL_DMA_CHANNEL_STATUS_REGISTER_OFS, ctl.reg);

    return 0;
}

/**
    Create UTIL ctrl param (memory)
    Create UTIL ctrl param (memory)

    @param[in] p_eng_ctl    :UTIL engine ctrl param

    @return status
        - @ -1  : Fail
        - @  0  : Pass
*/
INT32 util_eng_init(UINT32 chip_num, UINT32 eng_num)
{
	UINT32 total_ch;

	total_ch = (chip_num * eng_num);
	if (total_ch == 0) {
		DBG_ERR("UTIL input parameter fail (%d)\r\n", total_ch);
		return -1;
	}

	if (eng_num > UTIL_ID_MAX_NUM) {
		DBG_ERR("UTIL engine num error (%d)\r\n", eng_num);
		return -1;
	}

	eng_ctl.p_eng = UTIL_ENG_MALLOC(sizeof(UTIL_ENG_HANDLE) * total_ch);
	if (eng_ctl.p_eng == NULL) {
		DBG_ERR("alloc buf (%ld) failed\r\n", sizeof(UTIL_ENG_HANDLE) * (int)total_ch);
		return -1;
	}

	eng_ctl.chip_num = chip_num;
	eng_ctl.eng_num = eng_num;
	eng_ctl.total_ch= total_ch;

	return 0;
}

INT32 util_eng_release(VOID)
{
	UINT32 i;

	if (eng_ctl.p_eng) {
		for (i = 0; i < eng_ctl.total_ch; i ++) {
			util_eng_platform_release_irq(&eng_ctl.p_eng[i]);
			util_eng_platform_unprepare_clk(&eng_ctl.p_eng[i]);
		}
	}

	UTIL_ENG_FREE(eng_ctl.p_eng);
	memset((VOID *)&eng_ctl, 0, sizeof(UTIL_ENG_CTL));

	return 0;
}

/**
    Create UTIL resource (flg/sem/preclk/irq)
    Create UTIL resource (flg/sem/preclk/irq)

    @param[in] p_eng    : UTIL engine handle param

    @return status
        - @ -1  :Fail
        - @  0  :Pass
*/
INT32 util_eng_init_resource(UTIL_ENG_HANDLE *p_eng)
{
	UTIL_ENG_HANDLE *p_eng_hdl;
    UTIL_ENG_HANDLE eng_hdl_back;

	p_eng_hdl = util_eng_get_handle(p_eng->chip_id, p_eng->eng_id);
	if (p_eng_hdl == NULL) {
		DBG_ERR("UTIL handle null!!\r\n");
        return -1;
	}
	if (p_eng->eng_id >= UTIL_ID_MAX_NUM) 
    {
        DBG_ERR("UTIL engine ID error (%d)\r\n", p_eng->eng_id);
        return -1;
    }

	eng_hdl_back.flg_id_util = p_eng_hdl->flg_id_util;
	*p_eng_hdl = *p_eng;
	p_eng_hdl->flg_id_util = eng_hdl_back.flg_id_util;

	util_eng_platform_set_clk_rate(p_eng_hdl);
	util_eng_platform_prepare_clk(p_eng_hdl);
	util_eng_platform_request_irq(p_eng_hdl);

	return 0;
}

UTIL_ENG_HANDLE* util_eng_get_handle(UINT32 chip_id, UINT32 eng_id)
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

VOID util_eng_reg_isr_callback(UTIL_ENG_HANDLE *p_eng, UTIL_ISR_CB cb)
{
	if (p_eng == NULL) {
		return;
	}
	p_eng->isr_cb = cb;
}

INT32 util_eng_open(UTIL_ENG_HANDLE *p_eng)
{
	if (p_eng == NULL) {
		DBG_ERR("UTIL handle null\r\n");
        return -1;
	}
	if (p_eng->eng_id >= UTIL_ID_MAX_NUM) {
		DBG_ERR("UTIL engine ID error (%d)\r\n", p_eng->eng_id);
		return -1;
	}
	util_eng_platform_create_resource(p_eng);

	// enable clock & sram 
	if (util_eng_platform_enable_clk(p_eng)) {
		DBG_ERR("enable UTIL clk fail\r\n");
		return -1;
	}
	util_eng_platform_disable_sram_shutdown(p_eng);

	// clear interrupt enable & status 
    UTIL_ENG_SETREG(p_eng->reg_io_base + UTIL_INTERRUPT_ENABLE_REGISTER_OFS, 0);
	UTIL_ENG_SETREG(p_eng->reg_io_base + UTIL_INTERRUPT_STATUS_REGISTER_OFS, UTIL_ENG_INTERRUPT_ALL);

	// software reset 
	util_eng_int_soft_reset_hw_reg(p_eng);

	// set axi enable at here 
	util_eng_dma_channel_enable_hw_reg(p_eng, TRUE);
	util_eng_axi_channel_enable_hw_reg(p_eng, TRUE);

    // enable cycle counting
    util_eng_cycle_enable_hw_reg(p_eng, TRUE);

	return 0;
}

INT32 util_eng_close(UTIL_ENG_HANDLE *p_eng)
{
	if (p_eng == NULL) {
		return -1;
	}
	if (p_eng->eng_id >= UTIL_ID_MAX_NUM) {
		DBG_ERR("UTIL engine ID error (%d)\r\n", p_eng->eng_id);
		return -1;
	}
	util_eng_platform_enable_sram_shutdown(p_eng);
	// disable clock 
	util_eng_platform_disable_clk(p_eng);
	util_eng_platform_release_resource(p_eng);

	return 0;
}

INT32 util_eng_reset(UTIL_ENG_HANDLE *p_eng)
{
    if(p_eng == NULL) {
        DBG_ERR("UTIL handle null\r\n");
        return -1;
    }

    // DMA disable
    util_eng_int_dma_enable_hw_reg(p_eng, FALSE);

    // wait DMA idle
    if(util_eng_platform_dma_idle(p_eng)) {
        //DBG_ERR("UTIL wait DMA idle fail\n\r");
	return -1;
    } else {
        // SW reset
        util_eng_int_soft_reset_hw_reg(p_eng);

        // DMA enable
        util_eng_int_dma_enable_hw_reg(p_eng, TRUE);

        // clear interrupt status
        util_eng_clr_intr_status(p_eng, UTIL_ENG_INTERRUPT_ALL);
	return 0;
    }
}

VOID util_eng_dma_channel_enable_hw_reg(UTIL_ENG_HANDLE *p_eng, BOOL set_en)
{
	T_UTIL_DMA_CHANNEL_STATUS_REGISTER ctl;

	if (p_eng == NULL) {
		return;
	}

	ctl.reg = UTIL_ENG_GETREG(p_eng->reg_io_base + UTIL_DMA_CHANNEL_STATUS_REGISTER_OFS);
    if (set_en) {
	    ctl.bit.DMA_DISABLE = 0;
    } else {
	    ctl.bit.DMA_DISABLE = 1;
	}
	UTIL_ENG_SETREG(p_eng->reg_io_base + UTIL_DMA_CHANNEL_STATUS_REGISTER_OFS, ctl.reg);
}

BOOL util_eng_chk_dma_channel_idle_reg(UTIL_ENG_HANDLE *p_eng)
{
	T_UTIL_DMA_CHANNEL_STATUS_REGISTER ctl;

	if (p_eng == NULL) {
		return FALSE;
	}

	ctl.reg = UTIL_ENG_GETREG(p_eng->reg_io_base + UTIL_DMA_CHANNEL_STATUS_REGISTER_OFS);
    if (ctl.bit.UTIL_ALL_DMA_IDLE)
        return TRUE;
    else
        return FALSE;

    return FALSE;
}

VOID util_eng_axi_channel_enable_hw_reg(UTIL_ENG_HANDLE *p_eng, BOOL set_en)
{
	T_UTIL_AXI_REGISTER_2 ctl;

	if (p_eng == NULL) {
		return;
	}

	ctl.reg = UTIL_ENG_GETREG(p_eng->reg_io_base + UTIL_AXI_REGISTER_2_OFS);
    if (set_en) {
	    ctl.bit.AXI_BUS_DISABLE = 0;
    } else {
	    ctl.bit.AXI_BUS_DISABLE = 1;
	}
	UTIL_ENG_SETREG(p_eng->reg_io_base + UTIL_AXI_REGISTER_2_OFS, ctl.reg);
}

VOID util_eng_trig_single_hw_reg(UTIL_ENG_HANDLE *p_eng)
{
	T_UTIL_CONTROL_REGISTER ctl;

	if (p_eng == NULL) {
		return;
	}

	ctl.reg = UTIL_ENG_GETREG(p_eng->reg_io_base + UTIL_CONTROL_REGISTER_OFS);
	ctl.bit.UTIL_START = 1;
	ctl.bit.JOB0_START = 0;
	ctl.bit.JOB1_START = 0;
	UTIL_ENG_SETREG(p_eng->reg_io_base + UTIL_CONTROL_REGISTER_OFS, ctl.reg);
}

VOID util_eng_trig_ll_hw_reg(UTIL_ENG_HANDLE *p_eng, uintptr_t ll_addr_msb, uintptr_t ll_addr_lsb, UINT32 ll_idx)
{

	T_UTIL_CONTROL_REGISTER ctl;
    	T_UTIL_DRAMUB_JOB0_REGISTER_0 ll0_dma_lsb;
    	T_UTIL_DRAMUB_JOB0_REGISTER_1 ll0_dma_msb;

    	T_UTIL_DRAMUB_JOB1_REGISTER_0 ll1_dma_lsb;
    	T_UTIL_DRAMUB_JOB1_REGISTER_1 ll1_dma_msb;

	if (p_eng == NULL) {
		return;
	}
	if(ll_idx==0){
			ll0_dma_lsb.reg = ll_addr_lsb;
			ll0_dma_msb.reg = ll_addr_msb;

			UTIL_ENG_SETREG(p_eng->reg_io_base + UTIL_DRAMUB_JOB0_REGISTER_0_OFS, ll0_dma_lsb.reg);
		   	UTIL_ENG_SETREG(p_eng->reg_io_base + UTIL_DRAMUB_JOB0_REGISTER_1_OFS, ll0_dma_msb.reg);

			ctl.reg = UTIL_ENG_GETREG(p_eng->reg_io_base + UTIL_CONTROL_REGISTER_OFS);
			ctl.bit.UTIL_START = 0;
			ctl.bit.JOB0_START = 1;
			ctl.bit.JOB1_START = 0;
			UTIL_ENG_SETREG(p_eng->reg_io_base + UTIL_CONTROL_REGISTER_OFS, ctl.reg);
	}
	else if(ll_idx==1){
			ll1_dma_lsb.reg = ll_addr_lsb;
			ll1_dma_msb.reg = ll_addr_msb;

			UTIL_ENG_SETREG(p_eng->reg_io_base + UTIL_DRAMUB_JOB1_REGISTER_0_OFS, ll1_dma_lsb.reg);
		   	UTIL_ENG_SETREG(p_eng->reg_io_base + UTIL_DRAMUB_JOB1_REGISTER_1_OFS, ll1_dma_msb.reg);

			ctl.reg = UTIL_ENG_GETREG(p_eng->reg_io_base + UTIL_CONTROL_REGISTER_OFS);
			ctl.bit.UTIL_START = 0;
			ctl.bit.JOB0_START = 0;
			ctl.bit.JOB1_START = 1;
			UTIL_ENG_SETREG(p_eng->reg_io_base + UTIL_CONTROL_REGISTER_OFS, ctl.reg);
	}
	else{
			DBG_ERR("CAL: Wrong LL trigger option (%d)\n\r", ll_idx);
	}
}

VOID util_eng_isr_hw_reg(UTIL_ENG_HANDLE *p_eng)
{
	UINT32 status = 0, status_inte;
	UINT32 status_ll0 = 0, status_ll1 = 0;
	UINT32 inte;
	FLGPTN flag = 0x0;
	
	if (p_eng == NULL) {
		DBG_ERR("UTIL parameter null\r\n");
		return ;
	}
	// get interrupt status & enable bit 
	status = UTIL_ENG_GETREG(p_eng->reg_io_base + UTIL_INTERRUPT_STATUS_REGISTER_OFS);
    //DBG_ERR("int sts = 0x%x\r\n", status);

	// clear interrupt status 
	inte = UTIL_ENG_GETREG(p_eng->reg_io_base + UTIL_INTERRUPT_ENABLE_REGISTER_OFS);
	status_inte = status & inte;

	if (status != 0) {
		flag = 0x0;
		if (status_inte & UTIL_ENG_INTERRUPT_FRM_END) {
			flag |= FLGPTN_UTIL_FRM_END;
		}
		if (status_inte & UTIL_ENG_INTERRUPT_LL0_END) {
			flag |= FLGPTN_UTIL_LL0_END;
			status_ll0 = 1;
		}
		if (status & UTIL_ENG_INTERRUPT_LL0_ERR) {
			DBG_ERR("%s: UTIL linked-list 0 error\r\n", __func__);
		}
		if (status_inte & UTIL_ENG_INTERRUPT_LL1_END) {
			flag |= FLGPTN_UTIL_LL1_END;
			status_ll1 = 1;
		}
		if (status & UTIL_ENG_INTERRUPT_LL1_ERR) {
			DBG_ERR("%s: UTIL linked-list error\r\n", __func__);
		}
		if ((status_inte & UTIL_ENG_INTERRUPT_LL0_END) || (status & UTIL_ENG_INTERRUPT_LL1_END)) {
            		UTIL_ENG_SETREG(p_eng->reg_io_base + UTIL_INTERRUPT_STATUS_REGISTER_OFS, UTIL_ENG_INTERRUPT_FRM_END);
		}
		util_eng_platform_flg_set(p_eng, flag);
	}
	UTIL_ENG_SETREG(p_eng->reg_io_base + UTIL_INTERRUPT_STATUS_REGISTER_OFS, status_inte);

	// get interrupt status 
	if (p_eng->isr_cb != NULL) {
		if(status_ll0==1)p_eng->isr_cb(p_eng, status, NULL,0);
		if(status_ll1==1)p_eng->isr_cb(p_eng, status, NULL,1);
	}
}

/**
    Enable interrupt fuction of UTIL
    Enable interrupt fuction of UTIL

    @param[in]
        -@ p_eng   : UTIL handle param
        -@int_en   : Which interrupt function

    @return Void
*/
INT32 util_eng_set_intrpt_en(UTIL_ENG_HANDLE *p_eng, UINT32 int_en)
{
	T_UTIL_INTERRUPT_ENABLE_REGISTER LocalReg;
	LocalReg.reg = int_en;
	
	if (p_eng == NULL) {
		return -1;
	}
    if(p_eng->eng_id >=UTIL_ID_MAX_NUM)
    {
		DBG_ERR("UTIL engine ID error (%d)\r\n", p_eng->eng_id);
		return -1;
    }
	
	UTIL_ENG_SETREG(p_eng->reg_io_base + UTIL_INTERRUPT_ENABLE_REGISTER_OFS, LocalReg.reg);

	// must be used link list buffer
	p_eng->p_util_reg_st->UTIL_Register_1.Word = (UINT32)int_en;
	p_eng->p_util_reg_chg_flag[1] = TRUE;
    return 0;
}

VOID util_eng_clr_intr_status(UTIL_ENG_HANDLE *p_eng, UINT32 uiIntrStatus)
{
	T_UTIL_INTERRUPT_STATUS_REGISTER LocalReg;
	LocalReg.reg = uiIntrStatus;
	UTIL_ENG_SETREG(p_eng->reg_io_base + UTIL_INTERRUPT_STATUS_REGISTER_OFS, LocalReg.reg);
}

UINT32 util_eng_clr_flg(UTIL_ENG_HANDLE *p_eng, FLGPTN flg)
{
	return util_eng_platform_flg_clear(p_eng, flg);
}

UINT32 util_eng_wait_flg(UTIL_ENG_HANDLE *p_eng, PFLGPTN p_flgptn, FLGPTN flg)
{
	return util_eng_platform_flg_wait(p_eng, p_flgptn, flg);
}

VOID util_eng_wait_framend_ll(UTIL_ENG_HANDLE *p_eng, UINT32 ll_idx)
{
    FLGPTN uiflag;
    ER er_return = E_OK;
    switch(ll_idx) {
        case 0:
            er_return = util_eng_platform_flg_wait(p_eng, &uiflag, FLGPTN_UTIL_LL0END);
            break;
        case 1:
            er_return = util_eng_platform_flg_wait(p_eng, &uiflag, FLGPTN_UTIL_LL1END);
            break;
        default:
            DBG_ERR("UTIL wrong LL wait framend option (%d)\n\r", ll_idx);
            return;
    }
    if(er_return != E_OK) {
        DBG_ERR("UTIL LL waitdone error\n\r");
    }
}

UINT32 util_eng_wait_flg_timeout(UTIL_ENG_HANDLE *p_eng, PFLGPTN p_flgptn, FLGPTN flg, int timeout_tick)
{
	return util_eng_platform_flg_wait_timeout(p_eng, p_flgptn, flg, timeout_tick);
}

BOOL util_ssdrv_proc_ver(VOID)
{
    DBG_DUMP("%s Version: %s %s %s\n", UTIL_SSD_DRV_NAME, UTIL_SSD_DRV_MODULE_VERSION, __DATE__, __TIME__);
    return TRUE;
}

VOID util_eng_cycle_enable_hw_reg(UTIL_ENG_HANDLE *p_eng, BOOL set_en)
{
    T_UTIL_MISC_REGISTER_0 ctl;

	if (p_eng == NULL) {
		return;
	}

	ctl.reg = UTIL_ENG_GETREG(p_eng->reg_io_base + UTIL_MISC_REGISTER_0_OFS);
    if (set_en) {
	    ctl.bit.CYCLE_EN = 1;
	    ctl.bit.CYCLE_DMA_WAIT_EN = 1;
    } else {
	    ctl.bit.CYCLE_EN = 0;
	    ctl.bit.CYCLE_DMA_WAIT_EN = 0;
	}
	UTIL_ENG_SETREG(p_eng->reg_io_base + UTIL_MISC_REGISTER_0_OFS, ctl.reg);
}

UINT32 util_eng_get_eng_cycle(UTIL_ENG_HANDLE *p_eng)
{
    return UTIL_ENG_GETREG(p_eng->reg_io_base + UTIL_CYCLE_COUNT_REGISTER_0_OFS);
}

UINT32 util_eng_get_ll_cycle(UTIL_ENG_HANDLE *p_eng, UINT32 ll_idx)
{
    if(ll_idx==0) {
    	return UTIL_ENG_GETREG(p_eng->reg_io_base + UTIL_CYCLE_COUNT_REGISTER_1_OFS);
    }
    else{
    	return UTIL_ENG_GETREG(p_eng->reg_io_base + UTIL_CYCLE_COUNT_REGISTER_3_OFS);
    }
}

UINT32 util_eng_get_ll_sta_acc_cycle(UTIL_ENG_HANDLE *p_eng, UINT32 ll_idx)
{
    switch(ll_idx) {
    case 0:
        return UTIL_ENG_GETREG(p_eng->reg_io_base + UTIL_STATISTIC_QOS_REGISTER_0_OFS);
    case 1:
        return UTIL_ENG_GETREG(p_eng->reg_io_base + UTIL_STATISTIC_QOS_REGISTER_1_OFS);
    default:
        DBG_ERR("UTIL wrong get stastic LL acc cycle option (%d)\n\r", ll_idx);
        return 0;
    }
}

UINT32 util_eng_get_wait_cycle(UTIL_ENG_HANDLE *p_eng)
{
    return UTIL_ENG_GETREG(p_eng->reg_io_base + UTIL_CYCLE_COUNT_REGISTER_2_OFS);
}

UINT32 util_eng_get_dram_bw(UTIL_ENG_HANDLE *p_eng)
{
    return UTIL_ENG_GETREG(p_eng->reg_io_base + UTIL_RESERVED_REGISTER_89_OFS);
}

UINT32 util_eng_get_ub_bw(UTIL_ENG_HANDLE *p_eng)
{
    return UTIL_ENG_GETREG(p_eng->reg_io_base + UTIL_RESERVED_REGISTER_90_OFS);
}

BOOL util_eng_get_dma_idle(UTIL_ENG_HANDLE *p_eng)
{
    T_UTIL_DMA_CHANNEL_STATUS_REGISTER ctl;

	if (p_eng == NULL) {
        DBG_ERR("UTIL handle null\r\n");
		return FALSE;
	}

	ctl.reg = UTIL_ENG_GETREG(p_eng->reg_io_base + UTIL_DMA_CHANNEL_STATUS_REGISTER_OFS);
    
    return ctl.bit.UTIL_ALL_DMA_IDLE;
}

INT32 util_eng_set_ll_setting(UTIL_ENG_HANDLE *p_eng, UTIL_LL_SETTING *ll_param)
{
    T_UTIL_JOB_PRIORITY_REGISTER_0 ctl_priority;
    T_UTIL_QOS_REGISTER_0 ctl_qos0;
    T_UTIL_QOS_REGISTER_1 ctl_qos1;
    T_UTIL_STATISTIC_QOS_REGISTER_2 ctl_sta_qos2;

    if (p_eng == NULL) {
        DBG_ERR("UTIL handle null\r\n");
		return FALSE;
	}

    ctl_priority.bit.JOB_PRIORITY_MODE = ll_param->priority_mode;
    ctl_qos0.bit.QOS_TOTAL_TIME = ll_param->qos_tot_time;
    ctl_qos1.bit.QOS_LOW_OCCUPY_TIME = ll_param->qos_ocpy_time;
    ctl_sta_qos2.bit.QOS_PERIOD_TIME = ll_param->sta_period_time;

    UTIL_ENG_SETREG(p_eng->reg_io_base + UTIL_JOB_PRIORITY_REGISTER_0_OFS, ctl_priority.reg);
    UTIL_ENG_SETREG(p_eng->reg_io_base + UTIL_QOS_REGISTER_0_OFS, ctl_qos0.reg);
    UTIL_ENG_SETREG(p_eng->reg_io_base + UTIL_QOS_REGISTER_1_OFS, ctl_qos1.reg);
    UTIL_ENG_SETREG(p_eng->reg_io_base + UTIL_STATISTIC_QOS_REGISTER_2_OFS, ctl_sta_qos2.reg);
	
    return TRUE;
}

UINT32 util_eng_get_qos_setting(UTIL_ENG_HANDLE *p_eng)
{
    return UTIL_ENG_GETREG(p_eng->reg_io_base + UTIL_QOS_REGISTER_0_OFS);
}

UINT32 util_eng_get_clk_rate(UTIL_ENG_HANDLE *p_eng)
{
	if (p_eng == NULL) {
		DBG_ERR("UTIL handle null\r\n");
		return 0;
	}

	return util_eng_platform_get_clk_rate(p_eng);
}