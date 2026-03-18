/*
    CAL module driver

    NT98690 CAL module driver.

    @file       cal_eng.c
    @ingroup    mIIPPCAL
    @note       None

    Copyright   Novatek Microelectronics Corp. 2019.  All rights reserved.
*/
#include "kwrap/util.h"
#include "cal_eng_int_reg.h"
#include "cal_eng.h"
#include "cal_eng_handle.h"
#include "cal_eng_int_platform.h"
#include "cal_eng_int_dbg.h"

static CAL_ENG_CTL eng_ctl;
#define CAL_ENG_ID_TO_CH(chip_id, eng_id) ((chip_id * eng_ctl.eng_num) + eng_id)

/**
    Get register buffer size
    Get register buffer size

    @param[in] eng_id    : CAL engine id param(CAL_ID Only)

    @return Register buffer size (unit: Byte), if error eng handle, return 0;
*/
UINT32 cal_eng_get_reg_base_buf_size(UINT32 eng_id)
{
	if (eng_id >= CAL_ID_MAX_NUM) {
		DBG_ERR("CAL engine ID error %d\r\n", eng_id);
		return 0;
	}
	return (CAL_ENG_REG_NUMS << 2);
}

/**
    Get flg buffer size
    Get flg buffer size

    @param[in] eng_id    : CAL engine id param(CAL_ID Only)

    @return Flg buffer size (unit: Byte), if error eng handle, return 0;
*/
UINT32 cal_eng_get_reg_flag_buf_size(UINT32 eng_id)
{
	if (eng_id >= CAL_ID_MAX_NUM) {
		DBG_ERR("CAL engine ID error %d\r\n", eng_id);
		return 0;
	}
	return (CAL_ENG_REG_NUMS);
}

/**
    Set buffer address for register & flg
    Set buffer address for register & flg

    @param[in]
        -@ p_eng            : CAL handle param
        -@ reg_base_addr    : Register buffer address
        -@ reg_flag_addr    : Flg buffer address

    @return Void
*/
VOID cal_eng_set_reg_buf(CAL_ENG_HANDLE *p_eng, uintptr_t reg_base_addr, uintptr_t reg_flag_addr)
{
    p_eng->p_cal_reg_st = (NT98690_CAL_REGISTER_STRUCT *)reg_base_addr;
	p_eng->p_cal_reg_chg_flag = (UINT8 *)reg_flag_addr;
}

/**
    CAL software reset
    CAL software reset

    @param[in] p_eng    : CAL engine handle param

    @return status
        - @ -1   : Fail
        - @  0   : Pass
*/
static INT32 cal_eng_int_soft_reset_hw_reg(CAL_ENG_HANDLE *p_eng)
{
	if (p_eng == NULL) {
		DBG_ERR("CAL handle null\r\n");
		return -1;
	}

    // NOTE: Do not retrieve register value, set CAL_SW_RST and write value back (it could re-start engine)
    CAL_ENG_SETREG(p_eng->reg_io_base + CAL_CONTROL_REGISTER_OFS, 0x1);
    CAL_ENG_SETREG(p_eng->reg_io_base + CAL_CONTROL_REGISTER_OFS, 0x0);

    return 0;
}

static INT32 cal_eng_int_dma_enable_hw_reg(CAL_ENG_HANDLE *p_eng, BOOL set_en)
{
    T_CAL_DMA_REGISTER ctl;

	if (p_eng == NULL) {
		DBG_ERR("CAL handle null\r\n");
		return -1;
	}

    ctl.reg = CAL_ENG_GETREG(p_eng->reg_io_base + CAL_DMA_REGISTER_OFS);
    ctl.bit.DMA_DISABLE = set_en ? 0 : 1;
    CAL_ENG_SETREG(p_eng->reg_io_base + CAL_DMA_REGISTER_OFS, ctl.reg);

    return 0;
}

/**
    Create CAL ctrl param (memory)
    Create CAL ctrl param (memory)

    @param[in] p_eng_ctl    : CAL engine ctrl param

    @return status
        - @ -1  : Fail
        - @  0  : Pass
*/
INT32 cal_eng_init(UINT32 chip_num, UINT32 eng_num)
{
	UINT32 total_ch;

	total_ch = (chip_num * eng_num);
	if (total_ch == 0) {
		DBG_ERR("CAL input parameter fail (%d)\r\n", total_ch);
		return -1;
	}

	if (eng_num > CAL_ID_MAX_NUM) {
		DBG_ERR("CAL engine num error (%d)\r\n", eng_num);
		return -1;
	}

	eng_ctl.p_eng = CAL_ENG_MALLOC(sizeof(CAL_ENG_HANDLE) * total_ch);
	if (eng_ctl.p_eng == NULL) {
		DBG_ERR("CAL allocate buf (%ld) failed\r\n", sizeof(CAL_ENG_HANDLE) * (int)total_ch);
		return -1;
	}

	eng_ctl.chip_num = chip_num;
	eng_ctl.eng_num = eng_num;
	eng_ctl.total_ch= total_ch;

	return 0;
}

INT32 cal_eng_release(VOID)
{
	UINT32 i;

	if (eng_ctl.p_eng) {
		for (i = 0; i < eng_ctl.total_ch; i ++) {
			cal_eng_platform_release_irq(&eng_ctl.p_eng[i]);
			cal_eng_platform_unprepare_clk(&eng_ctl.p_eng[i]);
		}
	}

	CAL_ENG_FREE(eng_ctl.p_eng);
	memset((VOID *)&eng_ctl, 0, sizeof(CAL_ENG_CTL));

	return 0;
}

/**
    Create CAL resource (flg/sem/preclk/irq)
    Create CAL resource (flg/sem/preclk/irq)

    @param[in] p_eng    : CAL engine handle param

    @return status
        - @ -1  :Fail
        - @  0  :Pass
*/
INT32 cal_eng_init_resource(CAL_ENG_HANDLE *p_eng)
{
	CAL_ENG_HANDLE *p_eng_hdl;
    CAL_ENG_HANDLE eng_hdl_back;

	p_eng_hdl = cal_eng_get_handle(p_eng->chip_id, p_eng->eng_id);
	if (p_eng_hdl == NULL) {
		DBG_ERR("CAL handle null!!\r\n");
        return -1;
	}
	if (p_eng->eng_id >= CAL_ID_MAX_NUM) 
    {
        DBG_ERR("CAL engine ID error (%d)\r\n", p_eng->eng_id);
        return -1;
    }

	eng_hdl_back.flg_id_cal = p_eng_hdl->flg_id_cal;
	*p_eng_hdl = *p_eng;
	p_eng_hdl->flg_id_cal = eng_hdl_back.flg_id_cal;

	cal_eng_platform_set_clk_rate(p_eng_hdl);
	cal_eng_platform_prepare_clk(p_eng_hdl);
	cal_eng_platform_request_irq(p_eng_hdl);

	return 0;
}

CAL_ENG_HANDLE* cal_eng_get_handle(UINT32 chip_id, UINT32 eng_id)
{
	UINT32 idx = chip_id * eng_ctl.eng_num + eng_id;

	if (eng_ctl.p_eng == NULL) {
		DBG_ERR("CAL handle null\r\n");
		return NULL;
	}

	if (idx < eng_ctl.total_ch) {
		return &eng_ctl.p_eng[idx];
	} else {
		DBG_ERR("CAL ID overflow (%d %d) > (%d %d)\r\n", chip_id, eng_id, eng_ctl.chip_num, eng_ctl.eng_num);
    	return NULL;
    }
}

VOID cal_eng_reg_isr_callback(CAL_ENG_HANDLE *p_eng, CAL_ISR_CB cb)
{
	if (p_eng == NULL) {
        DBG_ERR("CAL handle null\r\n");
		return;
	}
	p_eng->isr_cb = cb;
}

INT32 cal_eng_open(CAL_ENG_HANDLE *p_eng)
{
	if (p_eng == NULL) {
		DBG_ERR("CAL handle null\r\n");
        return -1;
	}
	if (p_eng->eng_id >= CAL_ID_MAX_NUM) {
		DBG_ERR("CAL engine ID error (%d)\r\n", p_eng->eng_id);
		return -1;
	}
	cal_eng_platform_create_resource(p_eng);

	// enable clock & sram 
	if (cal_eng_platform_enable_clk(p_eng)) {
		DBG_ERR("enable CAL clk fail\n");
		return -1;
	}
	cal_eng_platform_disable_sram_shutdown(p_eng);

	// clear interrupt enable & status 
    CAL_ENG_SETREG(p_eng->reg_io_base + CAL_INTERRUPT_ENABLE_REGISTER_OFS, 0);
	CAL_ENG_SETREG(p_eng->reg_io_base + CAL_INTERRUPT_STATUS_REGISTER_OFS, CAL_ENG_INTERRUPT_ALL);

	// software reset 
	cal_eng_int_soft_reset_hw_reg(p_eng);

	// set axi enable at here 
	cal_eng_dma_channel_enable_hw_reg(p_eng, TRUE);
	cal_eng_axi_channel_enable_hw_reg(p_eng, TRUE);

    // enable cycle counting
    cal_eng_cycle_enable_hw_reg(p_eng, TRUE);

	return 0;
}

INT32 cal_eng_close(CAL_ENG_HANDLE *p_eng)
{
	if (p_eng == NULL) {
        DBG_ERR("CAL handle null\r\n");
		return -1;
	}
	if (p_eng->eng_id >= CAL_ID_MAX_NUM) {
		DBG_ERR("CAL engine ID error (%d)\r\n", p_eng->eng_id);
		return -1;
	}
	cal_eng_platform_enable_sram_shutdown(p_eng);

	// disable clock 
	cal_eng_platform_disable_clk(p_eng);
	cal_eng_platform_release_resource(p_eng);

	return 0;
}

INT32 cal_eng_reset(CAL_ENG_HANDLE *p_eng)
{
    if(p_eng == NULL) {
        DBG_ERR("CAL handle null\r\n");
        return -1;
    }

    // DMA disable
    cal_eng_int_dma_enable_hw_reg(p_eng, FALSE);

    // wait DMA idle
    if(cal_eng_platform_dma_idle(p_eng)) {
        //DBG_ERR("CAL wait DMA idle fail\n\r");
	return -1;
    } else {
        // SW reset
        cal_eng_int_soft_reset_hw_reg(p_eng);

        // DMA enable
        cal_eng_int_dma_enable_hw_reg(p_eng, TRUE);

        // clear interrupt status
        cal_eng_clr_intr_status(p_eng, CAL_ENG_INTERRUPT_ALL);

	return 0;
    }
}

VOID cal_eng_dma_channel_enable_hw_reg(CAL_ENG_HANDLE *p_eng, BOOL set_en)
{
	T_CAL_DMA_REGISTER ctl;

	if (p_eng == NULL) {
        DBG_ERR("CAL handle null\r\n");
		return;
	}

	ctl.reg = CAL_ENG_GETREG(p_eng->reg_io_base + CAL_DMA_REGISTER_OFS);
    if (set_en) {
	    ctl.bit.DMA_DISABLE = 0;
    } else {
	    ctl.bit.DMA_DISABLE = 1;
	}
	CAL_ENG_SETREG(p_eng->reg_io_base + CAL_DMA_REGISTER_OFS, ctl.reg);
}

BOOL cal_eng_chk_dma_channel_idle_reg(CAL_ENG_HANDLE *p_eng)
{
	T_CAL_DMA_REGISTER ctl;

	if (p_eng == NULL) {
        DBG_ERR("CAL handle null\r\n");
		return FALSE;
	}

	ctl.reg = CAL_ENG_GETREG(p_eng->reg_io_base + CAL_DMA_REGISTER_OFS);
    if (ctl.bit.CAL_IDLE)
        return TRUE;
    else
        return FALSE;

    return FALSE;
}

VOID cal_eng_axi_channel_enable_hw_reg(CAL_ENG_HANDLE *p_eng, BOOL set_en)
{
	T_CAL_AXI_REGISTER_3 ctl;

	if (p_eng == NULL) {
        DBG_ERR("CAL handle null\r\n");
		return;
	}

	ctl.reg = CAL_ENG_GETREG(p_eng->reg_io_base + CAL_AXI_REGISTER_3_OFS);
    if (set_en) {
	    ctl.bit.AXI_BUS_DISABLE = 0;
    } else {
	    ctl.bit.AXI_BUS_DISABLE = 1;
	}
	CAL_ENG_SETREG(p_eng->reg_io_base + CAL_AXI_REGISTER_3_OFS, ctl.reg);
}

VOID cal_eng_trig_single_hw_reg(CAL_ENG_HANDLE *p_eng)
{
	T_CAL_CONTROL_REGISTER ctl;

	if (p_eng == NULL) {
        DBG_ERR("CAL handle null\r\n");
		return;
	}

	ctl.reg = CAL_ENG_GETREG(p_eng->reg_io_base + CAL_CONTROL_REGISTER_OFS);
	ctl.bit.CAL_START = 1;
	ctl.bit.JOB_START = 0;
	CAL_ENG_SETREG(p_eng->reg_io_base + CAL_CONTROL_REGISTER_OFS, ctl.reg);
}

VOID cal_eng_trig_ll_hw_reg(CAL_ENG_HANDLE *p_eng, uintptr_t ll_addr_msb, uintptr_t ll_addr_lsb)
{
    T_CAL_DRAMUB_JOB_REGISTER_0 ll_dma_lsb;
    T_CAL_DRAMUB_JOB_REGISTER_1 ll_dma_msb;
	T_CAL_CONTROL_REGISTER ctl;

	if (p_eng == NULL) {
        DBG_ERR("CAL handle null\r\n");
		return;
	}
    
	ll_dma_lsb.reg = ll_addr_lsb;
    ll_dma_msb.reg = ll_addr_msb;

	CAL_ENG_SETREG(p_eng->reg_io_base + CAL_DRAMUB_JOB_REGISTER_0_OFS, ll_dma_lsb.reg);
    CAL_ENG_SETREG(p_eng->reg_io_base + CAL_DRAMUB_JOB_REGISTER_1_OFS, ll_dma_msb.reg);

	ctl.reg = CAL_ENG_GETREG(p_eng->reg_io_base + CAL_CONTROL_REGISTER_OFS);
	ctl.bit.CAL_START = 0;
	ctl.bit.JOB_START = 1;
	CAL_ENG_SETREG(p_eng->reg_io_base + CAL_CONTROL_REGISTER_OFS, ctl.reg);
}

VOID cal_eng_isr_hw_reg(CAL_ENG_HANDLE *p_eng)
{
	UINT32 status = 0, status_inte;
	UINT32 inte;
	FLGPTN flag = 0x0;
	
	if (p_eng == NULL) {
		DBG_ERR("CAL parameter null\r\n");
		return ;
	}
	// get interrupt status & enable bit 
	status = CAL_ENG_GETREG(p_eng->reg_io_base + CAL_INTERRUPT_STATUS_REGISTER_OFS);
    //DBG_ERR("int sts = 0x%x\r\n", status);

	// clear interrupt status 
	inte = CAL_ENG_GETREG(p_eng->reg_io_base + CAL_INTERRUPT_ENABLE_REGISTER_OFS);
	status_inte = status & inte;

	if (status != 0) {
		flag = 0x0;
		if (status_inte & CAL_ENG_INTERRUPT_FRM_END) {
			flag |= FLGPTN_CAL_FRM_END;
		}
		if (status_inte & CAL_ENG_INTERRUPT_LL_END) {
			flag |= FLGPTN_CAL_LL_END;
		}
		if (status & CAL_ENG_INTERRUPT_LL_ERR) {
			//flg |= FLGPTN_IME_LL_ERR;
			DBG_ERR("CAL linked-list error\r\n");
		}
		if (status & CAL_ENG_INTERRUPT_LL_END) {
            CAL_ENG_SETREG(p_eng->reg_io_base + CAL_INTERRUPT_STATUS_REGISTER_OFS, CAL_ENG_INTERRUPT_FRM_END);
		}
		cal_eng_platform_flg_set(p_eng, flag);
	}
	CAL_ENG_SETREG(p_eng->reg_io_base + CAL_INTERRUPT_STATUS_REGISTER_OFS, status_inte);

	// get interrupt status 
	if (p_eng->isr_cb != NULL) {
		p_eng->isr_cb(p_eng, status, NULL);
	}
}

/**
    Enable interrupt fuction of CAL
    Enable interrupt fuction of CAL

    @param[in]
        -@ p_eng   : CAL handle param
        -@int_en   : Which interrupt function

    @return Void
*/
INT32 cal_eng_set_intrpt_en(CAL_ENG_HANDLE *p_eng, UINT32 int_en)
{
	T_CAL_INTERRUPT_ENABLE_REGISTER LocalReg;
	LocalReg.reg = int_en;
	
	if (p_eng == NULL) {
        DBG_ERR("CAL handle null\r\n");
		return -1;
	}
    if(p_eng->eng_id >= CAL_ID_MAX_NUM)
    {
		DBG_ERR("CAL engine ID error %d\r\n", p_eng->eng_id);
		return -1;
    }
	
	CAL_ENG_SETREG(p_eng->reg_io_base + CAL_INTERRUPT_ENABLE_REGISTER_OFS, LocalReg.reg);
	// must be used link list buffer
	p_eng->p_cal_reg_st->CAL_Register_1.Word = (UINT32)int_en;
	p_eng->p_cal_reg_chg_flag[1] = TRUE;

    return 0;
}

VOID cal_eng_clr_intr_status(CAL_ENG_HANDLE *p_eng, UINT32 uiIntrStatus)
{
	T_CAL_INTERRUPT_STATUS_REGISTER LocalReg;
	LocalReg.reg = uiIntrStatus;
	CAL_ENG_SETREG(p_eng->reg_io_base + CAL_INTERRUPT_STATUS_REGISTER_OFS, LocalReg.reg);
}

UINT32 cal_eng_clr_flg(CAL_ENG_HANDLE *p_eng, FLGPTN flg)
{
	return cal_eng_platform_flg_clear(p_eng, flg);
}

UINT32 cal_eng_wait_flg(CAL_ENG_HANDLE *p_eng, PFLGPTN p_flgptn, FLGPTN flg)
{
	return cal_eng_platform_flg_wait(p_eng, p_flgptn, flg);
}

UINT32 cal_eng_wait_flg_timeout(CAL_ENG_HANDLE *p_eng, PFLGPTN p_flgptn, FLGPTN flg, int timeout_tick)
{
	return cal_eng_platform_flg_wait_timeout(p_eng, p_flgptn, flg, timeout_tick);
}

VOID cal_eng_wait_framend_ll(CAL_ENG_HANDLE *p_eng)
{
    FLGPTN uiflag;
    ER er_return = E_OK;

    er_return = cal_eng_platform_flg_wait(p_eng, &uiflag, FLGPTN_CAL_LLEND);
    if(er_return != E_OK) {
        DBG_ERR("CAL LL waitdone error\n\r");
    }
}

BOOL cal_ssdrv_proc_ver(void)
{
    DBG_DUMP("%s Version: %s %s %s\n", CAL_SSD_DRV_NAME, CAL_SSD_DRV_MODULE_VERSION, __DATE__, __TIME__);
    return TRUE;
}

VOID cal_eng_cycle_enable_hw_reg(CAL_ENG_HANDLE *p_eng, BOOL set_en)
{
    T_CAL_DEBUG_REGISTER_0 ctl;

	if (p_eng == NULL) {
        DBG_ERR("CAL handle null\r\n");
		return;
	}

	ctl.reg = CAL_ENG_GETREG(p_eng->reg_io_base + CAL_DEBUG_REGISTER_0_OFS);
    if (set_en) {
	    ctl.bit.CYCLE_COUNT_EN = 1;
    } else {
	    ctl.bit.CYCLE_COUNT_EN = 0;
	}
	CAL_ENG_SETREG(p_eng->reg_io_base + CAL_DEBUG_REGISTER_0_OFS, ctl.reg);
}

UINT32 cal_eng_get_eng_cycle(CAL_ENG_HANDLE *p_eng)
{
    return CAL_ENG_GETREG(p_eng->reg_io_base + CAL_CYCLE_COUNT_REGISTER_0_OFS);
}

UINT32 cal_eng_get_ll_cycle(CAL_ENG_HANDLE *p_eng)
{
    return CAL_ENG_GETREG(p_eng->reg_io_base + CAL_CYCLE_COUNT_REGISTER_1_OFS);
}

UINT32 cal_eng_get_clk_rate(CAL_ENG_HANDLE *p_eng)
{
    if(p_eng==NULL) {
        DBG_ERR("CAL handle null\r\n");
        return 0;
    }
    return cal_eng_platform_get_clk_rate(p_eng);
}

UINT32 cal_eng_get_wait_cycle(CAL_ENG_HANDLE *p_eng)
{
    return CAL_ENG_GETREG(p_eng->reg_io_base + CAL_CYCLE_COUNT_REGISTER_2_OFS);
}

UINT32 cal_eng_get_dram_bw(CAL_ENG_HANDLE *p_eng)
{
    return CAL_ENG_GETREG(p_eng->reg_io_base + CAL_RESERVED_OFS_38);
}

UINT32 cal_eng_get_ub_bw(CAL_ENG_HANDLE *p_eng)
{
    return CAL_ENG_GETREG(p_eng->reg_io_base + CAL_RESERVED_OFS_39);
}

BOOL cal_eng_get_dma_idle(CAL_ENG_HANDLE *p_eng)
{
    T_CAL_DMA_REGISTER ctl;

	if (p_eng == NULL) {
        DBG_ERR("CAL handle null\r\n");
		return FALSE;
	}

	ctl.reg = CAL_ENG_GETREG(p_eng->reg_io_base + CAL_DMA_REGISTER_OFS);
    
    return ctl.bit.CAL_IDLE;
}

