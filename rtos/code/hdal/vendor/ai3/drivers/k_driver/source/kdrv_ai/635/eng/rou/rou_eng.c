/*
    ROU module driver

    NT98635 ROU module driver.

    @file       rou_eng.c
    @ingroup    mIIPPROU
    @note       None

    Copyright   Novatek Microelectronics Corp. 2024.  All rights reserved.
*/
#include "kwrap/util.h"
#include "rou_eng_int_reg.h"
#include "rou_eng.h"
#include "rou_eng_handle.h"
#include "rou_eng_int_platform.h"
#include "rou_eng_int_dbg.h"

static ROU_ENG_CTL eng_ctl;
#define ROU_ENG_ID_TO_CH(chip_id, eng_id) ((chip_id * eng_ctl.eng_num) + eng_id)

/**
    Get register buffer size
    Get register buffer size

    @param[in] eng_id    : ROU engine id param(ROU_ID Only)

    @return Register buffer size (unit: Byte), if error eng handle, return 0;
*/
UINT32 rou_eng_get_reg_base_buf_size(UINT32 eng_id)
{
	if (eng_id >= ROU_ID_MAX_NUM) {
		DBG_ERR("ROU engine ID error (%d)\r\n", eng_id);
		return 0;
	}
	return (ROU_ENG_REG_NUMS << 2);
}

/**
    Get flg buffer size
    Get flg buffer size

    @param[in] eng_id    : ROU engine id param(ROU_ID Only)

    @return Flg buffer size (unit: Byte), if error eng handle, return 0;
*/
UINT32 rou_eng_get_reg_flag_buf_size(UINT32 eng_id)
{
	if (eng_id >= ROU_ID_MAX_NUM) {
		DBG_ERR("ROU engine ID error (%d)\r\n", eng_id);
		return 0;
	}
	return (ROU_ENG_REG_NUMS);
}

/**
    Set buffer address for register & flg
    Set buffer address for register & flg

    @param[in]
        -@ p_eng            : ROU handle param
        -@ reg_base_addr    : Register buffer address
        -@ reg_flag_addr    : Flg buffer address

    @return Void
*/
VOID rou_eng_set_reg_buf(ROU_ENG_HANDLE *p_eng, uintptr_t reg_base_addr, uintptr_t reg_flag_addr)
{
    p_eng->p_rou_reg_st = (NT98635_ROU_REGISTER_STRUCT *)reg_base_addr;
	p_eng->p_rou_reg_chg_flag = (UINT8 *)reg_flag_addr;
}

/**
    ROU software reset
    ROU software reset

    @param[in] p_eng    : ROU engine handle param

    @return status
        - @ -1   : Fail
        - @  0   : Pass
*/
static INT32 rou_eng_int_soft_reset_hw_reg(ROU_ENG_HANDLE *p_eng)
{
	if (p_eng == NULL) {
		DBG_ERR("ROU handle null\r\n");
		return -1;
	}

    // NOTE: Do not retrieve register value, set ROU_SW_RST and write value back (it could re-start engine)
    ROU_ENG_SETREG(p_eng->reg_io_base + ROU_CONTROL_REGISTER_OFS, 0x1);
    ROU_ENG_SETREG(p_eng->reg_io_base + ROU_CONTROL_REGISTER_OFS, 0x0);

    return 0;
}

static INT32 rou_eng_int_dma_enable_hw_reg(ROU_ENG_HANDLE *p_eng, BOOL set_en)
{
    T_ROU_MISC_REGISTER_1 ctl;

	if (p_eng == NULL) {
		DBG_ERR("ROU handle null\r\n");
		return -1;
	}

    ctl.reg = ROU_ENG_GETREG(p_eng->reg_io_base + ROU_MISC_REGISTER_1_OFS);
    ctl.bit.DMA_DIS = set_en ? 0 : 1;
    ROU_ENG_SETREG(p_eng->reg_io_base + ROU_MISC_REGISTER_1_OFS, ctl.reg);

    return 0;
}

/**
    Create ROU ctrl param (memory)
    Create ROU ctrl param (memory)

    @param[in] p_eng_ctl    :ROU engine ctrl param

    @return status
        - @ -1  : Fail
        - @  0  : Pass
*/
INT32 rou_eng_init(UINT32 chip_num, UINT32 eng_num)
{
	UINT32 total_ch;

	total_ch = (chip_num * eng_num);
	if (total_ch == 0) {
		DBG_ERR("ROU input parameter fail (%d)\r\n", total_ch);
		return -1;
	}

	if (eng_num > ROU_ID_MAX_NUM) {
		DBG_ERR("ROU engine num error (%d)\r\n", eng_num);
		return -1;
	}

	eng_ctl.p_eng = ROU_ENG_MALLOC(sizeof(ROU_ENG_HANDLE) * total_ch);
	if (eng_ctl.p_eng == NULL) {
		DBG_ERR("alloc buf (%ld) failed\r\n", sizeof(ROU_ENG_HANDLE) * (int)total_ch);
		return -1;
	}

	eng_ctl.chip_num = chip_num;
	eng_ctl.eng_num = eng_num;
	eng_ctl.total_ch= total_ch;

	return 0;
}

INT32 rou_eng_release(VOID)
{
	UINT32 i;

	if (eng_ctl.p_eng) {
		for (i = 0; i < eng_ctl.total_ch; i ++) {
			rou_eng_platform_release_irq(&eng_ctl.p_eng[i]);
			rou_eng_platform_unprepare_clk(&eng_ctl.p_eng[i]);
		}
	}
	ROU_ENG_FREE(eng_ctl.p_eng);
	memset((VOID *)&eng_ctl, 0, sizeof(ROU_ENG_CTL));

	return 0;
}

/**
    Create ROU resource (flg/sem/preclk/irq)
    Create ROU resource (flg/sem/preclk/irq)

    @param[in] p_eng    : ROU engine handle param

    @return status
        - @ -1  :Fail
        - @  0  :Pass
*/
INT32 rou_eng_init_resource(ROU_ENG_HANDLE *p_eng)
{
	ROU_ENG_HANDLE *p_eng_hdl;
    ROU_ENG_HANDLE eng_hdl_back;

	p_eng_hdl = rou_eng_get_handle(p_eng->chip_id, p_eng->eng_id);
	if (p_eng_hdl == NULL) {
		DBG_ERR("ROU handle null!!\r\n");
        return -1;
	}
	if (p_eng->eng_id >= ROU_ID_MAX_NUM) 
    {
        DBG_ERR("ROU engine ID error (%d)\r\n", p_eng->eng_id);
        return -1;
    }

	eng_hdl_back.flg_id_rou = p_eng_hdl->flg_id_rou;
	*p_eng_hdl = *p_eng;
	p_eng_hdl->flg_id_rou = eng_hdl_back.flg_id_rou;

	rou_eng_platform_set_clk_rate(p_eng_hdl);
	rou_eng_platform_prepare_clk(p_eng_hdl);
	rou_eng_platform_request_irq(p_eng_hdl);

	return 0;
}

ROU_ENG_HANDLE* rou_eng_get_handle(UINT32 chip_id, UINT32 eng_id)
{
	UINT32 idx = chip_id * eng_ctl.eng_num + eng_id;

	if (eng_ctl.p_eng == NULL) {
		DBG_ERR("ROU eng_ctl.p_eng == NULL\r\n");
		return NULL;
	}

	if (idx < eng_ctl.total_ch) {
		return &eng_ctl.p_eng[idx];
	} else {
		DBG_ERR("ROU ID overflow (%d x %d) > (%d x %d)\r\n", chip_id, eng_id, eng_ctl.chip_num, eng_ctl.eng_num);
    	return NULL;
    }
}

VOID rou_eng_reg_isr_callback(ROU_ENG_HANDLE *p_eng, ROU_ISR_CB cb)
{
	if (p_eng == NULL) {
        DBG_ERR("ROU handle null\r\n");
		return;
	}
	p_eng->isr_cb = cb;
}

INT32 rou_eng_open(ROU_ENG_HANDLE *p_eng)
{
	if (p_eng == NULL) {
		DBG_ERR("ROU handle null\r\n");
        return -1;
	}
	if (p_eng->eng_id >= ROU_ID_MAX_NUM) {
		DBG_ERR("ROU engine ID error (%d)\r\n", p_eng->eng_id);
		return -1;
	}
	rou_eng_platform_create_resource(p_eng);

	// enable clock & sram 
	if (rou_eng_platform_enable_clk(p_eng)) {
		DBG_ERR("enable ROU clk fail\r\n");
		return -1;
	}
	rou_eng_platform_disable_sram_shutdown(p_eng);

	// clear interrupt enable & status 
    ROU_ENG_SETREG(p_eng->reg_io_base + ROU_INTERRUPT_ENABLE_REGISTER_OFS, 0);
	ROU_ENG_SETREG(p_eng->reg_io_base + ROU_INTERRUPT_STATUS_REGISTER_OFS, ROU_ENG_INTERRUPT_ALL);

	// software reset 
	rou_eng_int_soft_reset_hw_reg(p_eng);

	// set axi enable at here 
	rou_eng_dma_channel_enable_hw_reg(p_eng, TRUE);
	rou_eng_axi_channel_enable_hw_reg(p_eng, TRUE);

    // enable cycle counting
    rou_eng_cycle_enable_hw_reg(p_eng, TRUE);

	return 0;
}

INT32 rou_eng_close(ROU_ENG_HANDLE *p_eng)
{
	if (p_eng == NULL) {
        DBG_ERR("ROU handle null\n\r");
		return -1;
	}
	if (p_eng->eng_id >= ROU_ID_MAX_NUM) {
		DBG_ERR("ROU engine ID error (%d)\r\n", p_eng->eng_id);
		return -1;
	}
	rou_eng_platform_enable_sram_shutdown(p_eng);
	// disable clock 
	rou_eng_platform_disable_clk(p_eng);
	rou_eng_platform_release_resource(p_eng);

	return 0;
}

INT32 rou_eng_reset(ROU_ENG_HANDLE *p_eng)
{
    if(p_eng == NULL) {
        DBG_ERR("ROU handle null\r\n");
        return -1;
    }

    // DMA disable
    rou_eng_int_dma_enable_hw_reg(p_eng, FALSE);

    // wait DMA idle
    if(rou_eng_platform_dma_idle(p_eng)) {
        //DBG_ERR("ROU wait DMA idle fail\n\r");
	    return -1;
    } else {
        // SW reset
        rou_eng_int_soft_reset_hw_reg(p_eng);

        // DMA enable
        rou_eng_int_dma_enable_hw_reg(p_eng, TRUE);

        // clear interrupt status
        rou_eng_clr_intr_status(p_eng, ROU_ENG_INTERRUPT_ALL);

	    return 0;
    }
}

VOID rou_eng_dma_channel_enable_hw_reg(ROU_ENG_HANDLE *p_eng, BOOL set_en)
{
	T_ROU_MISC_REGISTER_1 ctl;

	if (p_eng == NULL) {
        DBG_ERR("ROU handle null\r\n");
		return;
	}

	ctl.reg = ROU_ENG_GETREG(p_eng->reg_io_base + ROU_MISC_REGISTER_1_OFS);
    if (set_en) {
	    ctl.bit.DMA_DIS = 0;
    } else {
	    ctl.bit.DMA_DIS = 1;
	}
	ROU_ENG_SETREG(p_eng->reg_io_base + ROU_MISC_REGISTER_1_OFS, ctl.reg);
}

BOOL rou_eng_chk_dma_channel_idle_reg(ROU_ENG_HANDLE *p_eng)
{
	T_ROU_MISC_REGISTER_1 ctl;

	if (p_eng == NULL) {
        DBG_ERR("ROU handle null\r\n");
		return FALSE;
	}

	ctl.reg = ROU_ENG_GETREG(p_eng->reg_io_base + ROU_MISC_REGISTER_1_OFS);
    if (ctl.bit.DMA_IDLE)
        return TRUE;
    else
        return FALSE;

    return FALSE;
}

VOID rou_eng_axi_channel_enable_hw_reg(ROU_ENG_HANDLE *p_eng, BOOL set_en)
{
	T_ROU_CHANNEL_REGISTER_0 ctl;

	if (p_eng == NULL) {
        DBG_ERR("ROU handle null\r\n");
		return;
	}

	ctl.reg = ROU_ENG_GETREG(p_eng->reg_io_base + ROU_CHANNEL_REGISTER_0_OFS);
    if (set_en) {
	    ctl.bit.AXI_DIS = 0;
    } else {
	    ctl.bit.AXI_DIS = 1;
	}
	ROU_ENG_SETREG(p_eng->reg_io_base + ROU_CHANNEL_REGISTER_0_OFS, ctl.reg);
}

VOID rou_eng_trig_single_hw_reg(ROU_ENG_HANDLE *p_eng)
{
	T_ROU_CONTROL_REGISTER ctl;

	if (p_eng == NULL) {
        DBG_ERR("ROU handle null\r\n");
		return;
	}

	ctl.reg = ROU_ENG_GETREG(p_eng->reg_io_base + ROU_CONTROL_REGISTER_OFS);
	ctl.bit.ROU_START = 1;
	ctl.bit.JOB0_START = 0;
    ctl.bit.JOB1_START = 0;
	ROU_ENG_SETREG(p_eng->reg_io_base + ROU_CONTROL_REGISTER_OFS, ctl.reg);
}

VOID rou_eng_trig_ll_hw_reg(ROU_ENG_HANDLE *p_eng, uintptr_t ll_addr_msb, uintptr_t ll_addr_lsb, UINT32 ll_idx)
{
    T_ROU_CONTROL_REGISTER ctl;
    T_ROU_DRAMUB_JOB_REGISTER_0 ll0_dma_lsb;
    T_ROU_DRAMUB_JOB_REGISTER_1 ll0_dma_msb;
    T_ROU_DRAMUB_JOB_REGISTER_2 ll1_dma_lsb;
	T_ROU_DRAMUB_JOB_REGISTER_3 ll1_dma_msb;

	if (p_eng == NULL) {
        DBG_ERR("ROU handle null\r\n");
		return;
	}
    
    switch(ll_idx) {
    case 0:
    	ll0_dma_lsb.reg = ll_addr_lsb;
        ll0_dma_msb.reg = ll_addr_msb;

    	ROU_ENG_SETREG(p_eng->reg_io_base + ROU_DRAMUB_JOB_REGISTER_0_OFS, ll0_dma_lsb.reg);
        ROU_ENG_SETREG(p_eng->reg_io_base + ROU_DRAMUB_JOB_REGISTER_1_OFS, ll0_dma_msb.reg);

    	ctl.reg = ROU_ENG_GETREG(p_eng->reg_io_base + ROU_CONTROL_REGISTER_OFS);
    	ctl.bit.ROU_START = 0;
    	ctl.bit.JOB0_START = 1;
        ctl.bit.JOB1_START = 0;
    	ROU_ENG_SETREG(p_eng->reg_io_base + ROU_CONTROL_REGISTER_OFS, ctl.reg);
        break;
    case 1:
        ll1_dma_lsb.reg = ll_addr_lsb;
        ll1_dma_msb.reg = ll_addr_msb;

        ROU_ENG_SETREG(p_eng->reg_io_base + ROU_DRAMUB_JOB_REGISTER_2_OFS, ll1_dma_lsb.reg);
        ROU_ENG_SETREG(p_eng->reg_io_base + ROU_DRAMUB_JOB_REGISTER_3_OFS, ll1_dma_msb.reg);

        ctl.reg = ROU_ENG_GETREG(p_eng->reg_io_base + ROU_CONTROL_REGISTER_OFS);
    	ctl.bit.ROU_START = 0;
    	ctl.bit.JOB0_START = 0;
        ctl.bit.JOB1_START = 1;
    	ROU_ENG_SETREG(p_eng->reg_io_base + ROU_CONTROL_REGISTER_OFS, ctl.reg);
        break;
    default:
        DBG_ERR("ROU: Wrong LL trigger option (%d)\n\r", ll_idx);
        break;
    }
}

VOID rou_eng_isr_hw_reg(ROU_ENG_HANDLE *p_eng)
{
	UINT32 status = 0, status_inte;
	UINT32 status_ll0 = 0, status_ll1 = 0;
	UINT32 inte;
	FLGPTN flag = 0x0;
	
	if (p_eng == NULL) {
		DBG_ERR("ROU handle null\r\n");
		return ;
	}
	// get interrupt status & enable bit 
	status = ROU_ENG_GETREG(p_eng->reg_io_base + ROU_INTERRUPT_STATUS_REGISTER_OFS);
    //DBG_ERR("int sts = 0x%x\r\n", status);

	// clear interrupt status 
	inte = ROU_ENG_GETREG(p_eng->reg_io_base + ROU_INTERRUPT_ENABLE_REGISTER_OFS);
	status_inte = status & inte;

	if (status != 0) {
		flag = 0x0;
		if (status_inte & ROU_ENG_INTERRUPT_FRM_END) {
			flag |= FLGPTN_ROU_FRM_END;
		}
		if (status_inte & ROU_ENG_INTERRUPT_LL0_END) {
			flag |= FLGPTN_ROU_LL0_END;
			status_ll0 = 1;
		}
		if (status & ROU_ENG_INTERRUPT_LL0_ERR) {
			DBG_ERR("ROU linked-list 0 error\r\n");
		}
		if (status_inte & ROU_ENG_INTERRUPT_LL1_END) {
			flag |= FLGPTN_ROU_LL1_END;
			status_ll1 = 1;
		}
		if (status & ROU_ENG_INTERRUPT_LL1_ERR) {
			DBG_ERR("ROU linked-list 1 error\r\n");
		}
		if ((status & ROU_ENG_INTERRUPT_LL0_END) || (status & ROU_ENG_INTERRUPT_LL1_END)) {
            ROU_ENG_SETREG(p_eng->reg_io_base + ROU_INTERRUPT_STATUS_REGISTER_OFS, ROU_ENG_INTERRUPT_FRM_END);
		}
		rou_eng_platform_flg_set(p_eng, flag);
	}
	ROU_ENG_SETREG(p_eng->reg_io_base + ROU_INTERRUPT_STATUS_REGISTER_OFS, status_inte);

	// get interrupt status 
	if (p_eng->isr_cb != NULL) {
		if(status_ll0==1)p_eng->isr_cb(p_eng, status, NULL,0);
		if(status_ll1==1)p_eng->isr_cb(p_eng, status, NULL,1);
	}
}

/**
    Enable interrupt fuction of ROU
    Enable interrupt fuction of ROU

    @param[in]
        -@ p_eng   : ROU handle param
        -@int_en   : Which interrupt function

    @return Void
*/
INT32 rou_eng_set_intrpt_en(ROU_ENG_HANDLE *p_eng, UINT32 int_en)
{
	T_ROU_INTERRUPT_ENABLE_REGISTER LocalReg;
	LocalReg.reg = int_en;
	
	if (p_eng == NULL) {
        DBG_ERR("ROU handle null\r\n");
		return -1;
	}
    if(p_eng->eng_id >=ROU_ID_MAX_NUM)
    {
		DBG_ERR("ROU engine ID error (%d)\r\n", p_eng->eng_id);
		return -1;
    }
	
	ROU_ENG_SETREG(p_eng->reg_io_base + ROU_INTERRUPT_ENABLE_REGISTER_OFS, LocalReg.reg);

	// must be used link list buffer
	p_eng->p_rou_reg_st->ROU_Register_1.Word = (UINT32)int_en;
	p_eng->p_rou_reg_chg_flag[1] = TRUE;
    return 0;
}

VOID rou_eng_clr_intr_status(ROU_ENG_HANDLE *p_eng, UINT32 uiIntrStatus)
{
	T_ROU_INTERRUPT_STATUS_REGISTER LocalReg;
	LocalReg.reg = uiIntrStatus;
	ROU_ENG_SETREG(p_eng->reg_io_base + ROU_INTERRUPT_STATUS_REGISTER_OFS, LocalReg.reg);
}

UINT32 rou_eng_clr_flg(ROU_ENG_HANDLE *p_eng, FLGPTN flg)
{
	return rou_eng_platform_flg_clear(p_eng, flg);
}

UINT32 rou_eng_wait_flg(ROU_ENG_HANDLE *p_eng, PFLGPTN p_flgptn, FLGPTN flg)
{
	return rou_eng_platform_flg_wait(p_eng, p_flgptn, flg);
}

UINT32 rou_eng_wait_flg_timeout(ROU_ENG_HANDLE *p_eng, PFLGPTN p_flgptn, FLGPTN flg, int timeout_tick)
{
	return rou_eng_platform_flg_wait_timeout(p_eng, p_flgptn, flg, timeout_tick);
}

VOID rou_eng_wait_framend_ll(ROU_ENG_HANDLE *p_eng, UINT32 ll_idx)
{
    FLGPTN uiflag;
    ER er_return = E_OK;

    switch(ll_idx) {
    case 0:
        er_return = rou_eng_platform_flg_wait(p_eng, &uiflag, FLGPTN_ROU_LL0END);
        break;
    case 1:
        er_return = rou_eng_platform_flg_wait(p_eng, &uiflag, FLGPTN_ROU_LL1END);
        break;
    default:
        DBG_ERR("ROU wrong LL wait framend option (%d)\n\r", ll_idx);
        return;
    }
    if(er_return != E_OK) {
        DBG_ERR("ROU LL waitdone error\n\r");
    }
}

BOOL rou_ssdrv_proc_ver(void)
{
    DBG_DUMP("%s Version: %s %s %s\n", ROU_SSD_DRV_NAME, ROU_SSD_DRV_MODULE_VERSION, __DATE__, __TIME__);
    return TRUE;
}

VOID rou_eng_cycle_enable_hw_reg(ROU_ENG_HANDLE *p_eng, BOOL set_en)
{
    T_ROU_MISC_REGISTER_1 ctl;

	if (p_eng == NULL) {
        DBG_ERR("ROU handle null\r\n");
		return;
	}

	ctl.reg = ROU_ENG_GETREG(p_eng->reg_io_base + ROU_MISC_REGISTER_1_OFS);
    if (set_en) {
	    ctl.bit.CYCLE_EN = 1;
    } else {
	    ctl.bit.CYCLE_EN = 0;
	}
	ROU_ENG_SETREG(p_eng->reg_io_base + ROU_MISC_REGISTER_1_OFS, ctl.reg);
}

UINT32 rou_eng_get_eng_cycle(ROU_ENG_HANDLE *p_eng)
{
    return ROU_ENG_GETREG(p_eng->reg_io_base + ROU_CYCLE_COUNT_REGISTER_0_OFS);
}

UINT32 rou_eng_get_ll_cycle(ROU_ENG_HANDLE *p_eng, UINT32 ll_idx)
{
    switch(ll_idx) {
    case 0:
        return ROU_ENG_GETREG(p_eng->reg_io_base + ROU_CYCLE_COUNT_REGISTER_1_OFS);
    case 1:
        return ROU_ENG_GETREG(p_eng->reg_io_base + ROU_CYCLE_COUNT_REGISTER_3_OFS);
    default:
        DBG_ERR("ROU wrong get LL cycle option (%d)\n\r", ll_idx);
        return 0;
    }
}

UINT32 rou_eng_get_ll_sta_acc_cycle(ROU_ENG_HANDLE *p_eng, UINT32 ll_idx)
{
    switch(ll_idx) {
    case 0:
        return ROU_ENG_GETREG(p_eng->reg_io_base + ROU_STATISTIC_QOS_REGISTER_0_OFS);
    case 1:
        return ROU_ENG_GETREG(p_eng->reg_io_base + ROU_STATISTIC_QOS_REGISTER_1_OFS);
    default:
        DBG_ERR("ROU wrong get stastic LL acc cycle option (%d)\n\r", ll_idx);
        return 0;
    }
}

UINT32 rou_eng_get_clk_rate(ROU_ENG_HANDLE *p_eng)
{
    if(p_eng==NULL) {
        DBG_ERR("ROU handle null\r\n");
        return 0;
    }
    return rou_eng_platform_get_clk_rate(p_eng);
}

UINT32 rou_eng_get_wait_cycle(ROU_ENG_HANDLE *p_eng)
{
    return ROU_ENG_GETREG(p_eng->reg_io_base + ROU_CYCLE_COUNT_REGISTER_2_OFS);
}

UINT32 rou_eng_get_dram_bw(ROU_ENG_HANDLE *p_eng)
{
    return ROU_ENG_GETREG(p_eng->reg_io_base + ROU_RESERVED_OFS_25);
}

UINT32 rou_eng_get_ub_bw(ROU_ENG_HANDLE *p_eng)
{
    return ROU_ENG_GETREG(p_eng->reg_io_base + ROU_RESERVED_OFS_26);
}

BOOL rou_eng_get_dma_idle(ROU_ENG_HANDLE *p_eng)
{
    T_ROU_MISC_REGISTER_1 ctl;

	if (p_eng == NULL) {
        DBG_ERR("ROU handle null\r\n");
		return FALSE;
	}

	ctl.reg = ROU_ENG_GETREG(p_eng->reg_io_base + ROU_MISC_REGISTER_1_OFS);
    
    return ctl.bit.DMA_IDLE;
}

INT32 rou_eng_set_ll_setting(ROU_ENG_HANDLE *p_eng, ROU_LL_SETTING *ll_param)
{
    T_ROU_JOB_PRIORITY_REGISTER_0 ctl_priority;
    T_ROU_QOS_REGISTER_0 ctl_qos0;
    T_ROU_QOS_REGISTER_1 ctl_qos1;
    T_ROU_STATISTIC_QOS_REGISTER_2 ctl_sta_qos2;

    if (p_eng == NULL) {
        DBG_ERR("ROU handle null\r\n");
		return FALSE;
	}

    ctl_priority.bit.JOB_PRIORITY_MODE = ll_param->priority_mode;
    ctl_qos0.bit.QOS_TOTAL_TIME = ll_param->qos_tot_time;
    ctl_qos1.bit.QOS_LOW_OCCUPY_TIME = ll_param->qos_ocpy_time;
    ctl_sta_qos2.bit.QOS_PERIOD_TIME = ll_param->sta_period_time;

    ROU_ENG_SETREG(p_eng->reg_io_base + ROU_JOB_PRIORITY_REGISTER_0_OFS, ctl_priority.reg);
    ROU_ENG_SETREG(p_eng->reg_io_base + ROU_QOS_REGISTER_0_OFS, ctl_qos0.reg);
    ROU_ENG_SETREG(p_eng->reg_io_base + ROU_QOS_REGISTER_1_OFS, ctl_qos1.reg);
    ROU_ENG_SETREG(p_eng->reg_io_base + ROU_STATISTIC_QOS_REGISTER_2_OFS, ctl_sta_qos2.reg);

    return TRUE;
}

UINT32 rou_eng_get_qos_setting(ROU_ENG_HANDLE *p_eng)
{
    return ROU_ENG_GETREG(p_eng->reg_io_base + ROU_QOS_REGISTER_0_OFS);
}

UINT32 rou_eng_get_busy_count(ROU_ENG_HANDLE *p_eng, UINT32 ll_idx)
{
    if(p_eng->eng_id == ROU_ID) {
        DBG_ERR("ROU does not support busy count, please uses CAL busy count instead\n\r");
    } else if(p_eng->eng_id == ROU2_ID) {
        DBG_ERR("ROU2 does not support busy count\n\r");
    }
    return 0;
}

VOID rou_eng_clr_busy_count(ROU_ENG_HANDLE *p_eng, UINT32 ll_idx)
{
    DBG_ERR("ROU not support busy count\n\r");
}
