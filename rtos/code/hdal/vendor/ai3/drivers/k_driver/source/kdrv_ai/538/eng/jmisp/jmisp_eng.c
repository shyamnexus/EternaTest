/*
    JMISP module driver

    NT98690 JMISP module driver.

    @file       jmisp_eng.c
    @ingjmispp    mIIPPJMISP
    @note       None

    Copyright   Novatek Microelectronics Corp. 2019.  All rights reserved.
*/
#include "kwrap/util.h"
#include "jmisp_eng_int_reg.h"
#include "jmisp_eng.h"
#include "jmisp_eng_handle.h"
#include "jmisp_eng_int_platform.h"
#include "jmisp_eng_int_dbg.h"

static JMISP_ENG_CTL eng_ctl;
#define JMISP_ENG_ID_TO_CH(chip_id, eng_id) ((chip_id * eng_ctl.eng_num) + eng_id)

/**
    Get register buffer size
    Get register buffer size

    @param[in] eng_id    :ive engine id param(IVE_ID Only)

    @return Register buffer size (unit: Byte), if error eng handle, return 0;
*/
UINT32 jmisp_eng_get_reg_base_buf_size(UINT32 eng_id)
{
	if (eng_id >= JMISP_ID_MAX_NUM) {
		DBG_ERR("error jmisp engine id\r\n");
		return 0;
	}
	return (JMISP_ENG_REG_NUMS << 2);
}

/**
    Get flg buffer size
    Get flg buffer size

    @param[in] eng_id    :ive engine id param(IVE_ID Only)

    @return Flg buffer size (unit: Byte), if error eng handle, return 0;
*/
UINT32 jmisp_eng_get_reg_flag_buf_size(UINT32 eng_id)
{
	if (eng_id >= JMISP_ID_MAX_NUM) {
		DBG_ERR("error jmisp engine id\r\n");
		return 0;
	}
	return (JMISP_ENG_REG_NUMS);
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
VOID jmisp_eng_set_reg_buf(JMISP_ENG_HANDLE *p_eng, uintptr_t reg_base_addr, uintptr_t reg_flag_addr)
{
    p_eng->p_jmisp_reg_st = (NT98538_JMISP_REGISTER_STRUCT *)reg_base_addr;
	p_eng->p_jmisp_reg_chg_flag = (UINT8 *)reg_flag_addr;
}

/**
    jmisp software reset
    jmisp software reset

    @param[in] p_eng    :Ive engine handle param

    @return status
        - @ -1   : Fail
        - @  0   : Pass
*/
static INT32 jmisp_eng_int_soft_reset_hw_reg(JMISP_ENG_HANDLE *p_eng)
{
	T_JMISP_CONTROL_REGISTER ctl;

	if (p_eng == NULL) {
		DBG_ERR("jmisp handle parameter null\r\n");
		return -1;
	}

	ctl.reg = 0;
	ctl.bit.JMISP_SW_RST = 1;
	JMISP_ENG_SETREG(p_eng->reg_io_base + JMISP_CONTROL_REGISTER_OFS, ctl.reg);

	ctl.bit.JMISP_SW_RST = 0;
	JMISP_ENG_SETREG(p_eng->reg_io_base + JMISP_CONTROL_REGISTER_OFS, ctl.reg);

    return 0;
}

/**
    Create jmisp ctrl param (memory)
    Create jmisp ctrl param (memory)

    @param[in] p_eng_ctl    :jmisp engine ctrl param

    @return status
        - @ -1  : Fail
        - @  0  : Pass
*/
INT32 jmisp_eng_init(UINT32 chip_num, UINT32 eng_num)
{
	UINT32 total_ch;

	total_ch = (chip_num * eng_num);
	if (total_ch == 0) {
		DBG_ERR("jmisp input parameter fail %d\r\n", total_ch);
		return -1;
	}

	if (eng_num > JMISP_ID_MAX_NUM) {
		DBG_ERR("error ive engine num\r\n");
		return -1;
	}

	eng_ctl.p_eng = JMISP_ENG_MALLOC(sizeof(JMISP_ENG_HANDLE) * total_ch);
	if (eng_ctl.p_eng == NULL) {
		DBG_ERR("alloc buf(%ld) failed\r\n", sizeof(JMISP_ENG_HANDLE) * (int)total_ch);
		return -1;
	}

	eng_ctl.chip_num = chip_num;
	eng_ctl.eng_num = eng_num;
	eng_ctl.total_ch= total_ch;

	return 0;
}

INT32 jmisp_eng_release(void)
{
	/*UINT32 i;
	
	if (eng_ctl.p_eng) {
		for (i = 0; i < eng_ctl.total_ch; i ++) {
			jmisp_eng_platform_release_irq(&eng_ctl.p_eng[i]);
			jmisp_eng_platform_unprepare_clk(&eng_ctl.p_eng[i]);
		}
	}*/

	JMISP_ENG_FREE(eng_ctl.p_eng);
	memset((void *)&eng_ctl, 0, sizeof(JMISP_ENG_CTL));

	return 0;
}

/**
    Create jmisp resource (flg/sem/preclk/irq)
    Create jmisp resource (flg/sem/preclk/irq)

    @param[in] p_eng    :jmisp engine handle param

    @return status
        - @ -1  :Fail
        - @  0  :Pass
*/
INT32 jmisp_eng_init_resource(JMISP_ENG_HANDLE *p_eng)
{
	JMISP_ENG_HANDLE *p_eng_hdl;
    JMISP_ENG_HANDLE eng_hdl_back;

	p_eng_hdl = jmisp_eng_get_handle(p_eng->chip_id, p_eng->eng_id);
	if (p_eng_hdl == NULL) {
		DBG_ERR("ive handle null !!\r\n");
        return -1;
	}
	if (p_eng->eng_id >= JMISP_ID_MAX_NUM) 
    {
        DBG_ERR("error ive engine id\r\n");
        return -1;
    }

	eng_hdl_back.flg_id_jmisp = p_eng_hdl->flg_id_jmisp;
	*p_eng_hdl = *p_eng;
	p_eng_hdl->flg_id_jmisp = eng_hdl_back.flg_id_jmisp;

	//jmisp_eng_platform_set_clk_rate(p_eng_hdl);
	//jmisp_eng_platform_prepare_clk(p_eng_hdl);
	jmisp_eng_platform_request_irq(p_eng_hdl);

	return 0;
}

JMISP_ENG_HANDLE* jmisp_eng_get_handle(UINT32 chip_id, UINT32 eng_id)
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

void jmisp_eng_reg_isr_callback(JMISP_ENG_HANDLE *p_eng, JMISP_ISR_CB cb)
{
	if (p_eng == NULL) {
		return;
	}
	p_eng->isr_cb = cb;
}

INT32 jmisp_eng_open(JMISP_ENG_HANDLE *p_eng)
{
	if (p_eng == NULL) {
		DBG_ERR(" jmisp handle null\r\n");
        return -1;
	}
	if (p_eng->eng_id >= JMISP_ID_MAX_NUM) {
		DBG_ERR("error ive engine id (%d)\r\n", p_eng->eng_id);
		return -1;
	}
	jmisp_eng_platform_create_resource(p_eng);

	// clear interrupt enable & status 
    JMISP_ENG_SETREG(p_eng->reg_io_base + JMISP_INTERRUPT_ENABLE_REGISTER_OFS, 0);
	JMISP_ENG_SETREG(p_eng->reg_io_base + JMISP_INTERRUPT_STATUS_REGISTER_OFS, JMISP_ENG_INTERRUPT_ALL);

	// software reset 
	jmisp_eng_int_soft_reset_hw_reg(p_eng);

	// set axi enable at here 
	jmisp_eng_dma_channel_enable_hw_reg(p_eng, TRUE);
	jmisp_eng_axi_channel_enable_hw_reg(p_eng, TRUE);
	
	// enable cycle count
	{
		T_JMISP_MISC_REGISTER0 ctl;
		ctl.reg = JMISP_ENG_GETREG(p_eng->reg_io_base + JMISP_MISC_REGISTER0_OFS); 
		ctl.bit.CYCLE_COUNT_ENABLE = 1;
		ctl.bit.CHECK_SUM_ENABLE   = 1;
		JMISP_ENG_SETREG(p_eng->reg_io_base + JMISP_MISC_REGISTER0_OFS, ctl.reg);	
	}

	return 0;
}

INT32 jmisp_eng_close(JMISP_ENG_HANDLE *p_eng)
{
	if (p_eng == NULL) {
		return -1;
	}
	if (p_eng->eng_id >= JMISP_ID_MAX_NUM) {
		DBG_ERR("error ive engine id\r\n");
		return -1;
	}

	jmisp_eng_platform_release_resource(p_eng);

	return 0;
}

VOID jmisp_eng_dma_channel_enable_hw_reg(JMISP_ENG_HANDLE *p_eng, BOOL set_en)
{
	T_JMISP_MISC_REGISTER0 ctl;

	if (p_eng == NULL) {
		return;
	}

	ctl.reg = JMISP_ENG_GETREG(p_eng->reg_io_base + JMISP_MISC_REGISTER0_OFS);
    if (set_en) {
	    ctl.bit.PL0_CH_DISABLE = 0;
		ctl.bit.PL1_CH_DISABLE = 0;
		ctl.bit.PL2_CH_DISABLE = 0;
		ctl.bit.PL3_CH_DISABLE = 0;
    } else {
	    ctl.bit.PL0_CH_DISABLE = 1;
		ctl.bit.PL1_CH_DISABLE = 1;
		ctl.bit.PL2_CH_DISABLE = 1;
		ctl.bit.PL3_CH_DISABLE = 1;
	}
	JMISP_ENG_SETREG(p_eng->reg_io_base + JMISP_MISC_REGISTER0_OFS, ctl.reg);
}

BOOL jmisp_eng_chk_dma_channel_idle_reg(JMISP_ENG_HANDLE *p_eng)
{
	T_JMISP_MISC_REGISTER1 ctl;

	if (p_eng == NULL) {
		return FALSE;
	}

	ctl.reg = JMISP_ENG_GETREG(p_eng->reg_io_base + JMISP_MISC_REGISTER1_OFS);
    if (ctl.bit.PL0_CH_IDLE & ctl.bit.PL1_CH_IDLE & ctl.bit.PL2_CH_IDLE & ctl.bit.PL3_CH_IDLE)
        return TRUE;

    return FALSE;
}

VOID jmisp_eng_axi_channel_enable_hw_reg(JMISP_ENG_HANDLE *p_eng, BOOL set_en)
{
	T_JMISP_MISC_REGISTER0 ctl;

	if (p_eng == NULL) {
		return;
	}

	ctl.reg = JMISP_ENG_GETREG(p_eng->reg_io_base + JMISP_MISC_REGISTER0_OFS);
    if (set_en) {
	    ctl.bit.AXI_BUS_DISABLE = 0;
    } else {
	    ctl.bit.AXI_BUS_DISABLE = 1;
	}
	JMISP_ENG_SETREG(p_eng->reg_io_base + JMISP_MISC_REGISTER0_OFS, ctl.reg);
}

BOOL jmisp_eng_chk_axi_channel_idle_reg(JMISP_ENG_HANDLE *p_eng)
{
	T_JMISP_MISC_REGISTER1 ctl;

	if (p_eng == NULL) {
		return FALSE;
	}

	ctl.reg = JMISP_ENG_GETREG(p_eng->reg_io_base + JMISP_MISC_REGISTER1_OFS);
    if (ctl.bit.AXI_BUS_IDLE)
        return TRUE;

    return FALSE;
}

void jmisp_eng_trig_pl_hw_reg(JMISP_ENG_HANDLE *p_eng, uintptr_t pl_addr_msb, uintptr_t pl_addr_lsb, UINT32 pl_id)
{
    T_PL0_DMA_CHANNEL_REGISTER0 pl_dma_lsb;
    T_PL0_DMA_CHANNEL_REGISTER1 pl_dma_msb;
	T_JMISP_CONTROL_REGISTER ctl;

	if (p_eng == NULL) {
		return;
	}
    
	pl_dma_lsb.reg = pl_addr_lsb;
    pl_dma_msb.reg = pl_addr_msb;

	JMISP_ENG_SETREG(p_eng->reg_io_base + PL0_DMA_CHANNEL_REGISTER0_OFS + 0x300*pl_id, pl_dma_lsb.reg);
    JMISP_ENG_SETREG(p_eng->reg_io_base + PL0_DMA_CHANNEL_REGISTER1_OFS + 0x300*pl_id, pl_dma_msb.reg);

	ctl.reg = JMISP_ENG_GETREG(p_eng->reg_io_base + JMISP_CONTROL_REGISTER_OFS);
	if (pl_id == 0) {
		ctl.reg = ctl.reg & 0xE0;
		if (ctl.bit.PL0_FRAME_START == 1) {
			DBG_ERR("PLA is busy, cannot trigger again!\n");
			return;
		} else {
			ctl.bit.PL0_FRAME_START = 1;
		}
	} else if (pl_id == 1) {
		ctl.reg = ctl.reg & 0xD0;
		if (ctl.bit.PL1_FRAME_START == 1) {
			DBG_ERR("PLB is busy, cannot trigger again!\n");
			return;
		} else {
			ctl.bit.PL1_FRAME_START = 1;
		}
	} else if (pl_id == 2) {
		ctl.reg = ctl.reg & 0xB0;
		if (ctl.bit.PL2_FRAME_START == 1) {
			DBG_ERR("PLC is busy, cannot trigger again!\n");
			return;
		} else {
			ctl.bit.PL2_FRAME_START = 1;
		}
	} else if (pl_id == 3) {
		ctl.reg = ctl.reg & 0x70;
		if (ctl.bit.PL3_FRAME_START == 1) {
			DBG_ERR("PLD is busy, cannot trigger again!\n");
			return;
		} else {
			ctl.bit.PL3_FRAME_START = 1;
		}
	} 
	
	JMISP_ENG_SETREG(p_eng->reg_io_base + JMISP_CONTROL_REGISTER_OFS, ctl.reg);
}

void jmisp_eng_isr_hw_reg(JMISP_ENG_HANDLE *p_eng)
{
	UINT32 status = 0, status_inte;
	UINT32 inte;
	FLGPTN flag = 0x0;
	
	if (p_eng == NULL) {
		DBG_ERR("parameter null\r\n");
		return ;
	}
	// get interrupt status & enable bit 
	status = JMISP_ENG_GETREG(p_eng->reg_io_base + JMISP_INTERRUPT_STATUS_REGISTER_OFS);
    //DBG_ERR("int sts = 0x%x\r\n", status);

	// clear interrupt status 
	inte = JMISP_ENG_GETREG(p_eng->reg_io_base + JMISP_INTERRUPT_ENABLE_REGISTER_OFS);
	status_inte = status & inte;

	if (status != 0) {
		flag = 0x0;
		if (status_inte & JMISP_ENG_INTERRUPT_PL0_END) {
			flag |= FLGPTN_JMISP_PL0_END;
		}
		if (status_inte & JMISP_ENG_INTERRUPT_PL1_END) {
			flag |= FLGPTN_JMISP_PL1_END;
		}
		if (status_inte & JMISP_ENG_INTERRUPT_PL2_END) {
			flag |= FLGPTN_JMISP_PL2_END;
		}
		if (status_inte & JMISP_ENG_INTERRUPT_PL3_END) {
			flag |= FLGPTN_JMISP_PL3_END;
		}


		if (status & JMISP_ENG_INTERRUPT_PL0_ERR) {
			DBG_ERR("%s: PL0 error...\r\n", __func__);
		}
		if (status & JMISP_ENG_INTERRUPT_PL1_ERR) {
			DBG_ERR("%s: PL1 error...\r\n", __func__);
		}
		if (status & JMISP_ENG_INTERRUPT_PL2_ERR) {
			DBG_ERR("%s: PL2 error...\r\n", __func__);
		}
		if (status & JMISP_ENG_INTERRUPT_PL3_ERR) {
			DBG_ERR("%s: PL3 error...\r\n", __func__);
		}

		jmisp_eng_platform_flg_set(p_eng, flag);
	}
	JMISP_ENG_SETREG(p_eng->reg_io_base + JMISP_INTERRUPT_STATUS_REGISTER_OFS, status_inte);

	// get interrupt status 
	if (p_eng->isr_cb != NULL) {
		p_eng->isr_cb(p_eng, status, NULL);
	}
}

/**
    Enable interrupt fuction of jmisp
    Enable interrupt fuction of jmisp

    @param[in]
        -@ p_eng   :Rou handle param
        -@int_en   :Which interrupt function

    @return Void
*/
INT32 jmisp_eng_set_intrpt_en(JMISP_ENG_HANDLE *p_eng, UINT32 int_en, UINT32 pl_id)
{
	T_JMISP_INTERRUPT_ENABLE_REGISTER LocalReg;
	LocalReg.reg = int_en;
	
	if (p_eng == NULL) {
		return -1;
	}
    if(p_eng->eng_id >=JMISP_ID_MAX_NUM)
    {
		DBG_ERR("error jmisp engine id\r\n");
		return -1;
    }
	
	// clear status for enabled bit
	JMISP_ENG_SETREG(p_eng->reg_io_base + JMISP_INTERRUPT_STATUS_REGISTER_OFS, 1 << pl_id);
	JMISP_ENG_SETREG(p_eng->reg_io_base + JMISP_INTERRUPT_ENABLE_REGISTER_OFS, LocalReg.reg);
	// must be used link list buffer
	p_eng->p_jmisp_reg_st->JMISP_Register_1.Word = (UINT32)int_en;
	p_eng->p_jmisp_reg_chg_flag[1] = TRUE;
    return 0;
}

VOID jmisp_eng_clr_intr_status(JMISP_ENG_HANDLE *p_eng, UINT32 uiIntrStatus)
{
	T_JMISP_INTERRUPT_STATUS_REGISTER LocalReg;
	LocalReg.reg = uiIntrStatus;
	JMISP_ENG_SETREG(p_eng->reg_io_base + JMISP_INTERRUPT_STATUS_REGISTER_OFS, LocalReg.reg);
}

INT32 jmisp_eng_set_single_dbg_en(JMISP_ENG_HANDLE *p_eng, UINT32 enable, UINT32 pl_id)
{
	T_PL0_DEBUG_MISC_REGISTER LocalReg;
	
	if (p_eng == NULL) {
		return -1;
	}
    if(p_eng->eng_id >= JMISP_ID_MAX_NUM)
    {
		DBG_ERR("error jmisp engine id\r\n");
		return -1;
    }
	if (enable) {
		LocalReg.bit.PL0_DBG_CODE = 0x69;
		LocalReg.bit.PL0_DBG_CMD_STEP = 1;
		LocalReg.bit.PL0_DBG_BLOCK_STEP = 1;
	} else {
		LocalReg.bit.PL0_DBG_CODE = 0x0;
	}
	JMISP_ENG_SETREG(p_eng->reg_io_base + 0x30*pl_id + PL0_DEBUG_MISC_REGISTER_OFS, LocalReg.reg);
	
    return 0;
}

INT32 jmisp_eng_run_single_dbg(JMISP_ENG_HANDLE *p_eng, UINT32 pl_id)
{
	T_PL0_DEBUG_MISC_REGISTER LocalReg;
	
	if (p_eng == NULL) {
		return -1;
	}
    if(p_eng->eng_id >= JMISP_ID_MAX_NUM)
    {
		DBG_ERR("error jmisp engine id\r\n");
		return -1;
    }
	LocalReg.reg = JMISP_ENG_GETREG(p_eng->reg_io_base + 0x30*pl_id + PL0_DEBUG_MISC_REGISTER_OFS);
	//LocalReg = JMISP_ENG_GETREG(p_eng->reg_io_base + 0x90*jl_id + JLA_DEBUG_MISC_REGISTER_OFS);
	if (LocalReg.bit.PL0_DBG_CODE == 0x69) {
		LocalReg.bit.PL0_DBG_NEXT = 1;
		JMISP_ENG_SETREG(p_eng->reg_io_base + 0x30*pl_id + PL0_DEBUG_MISC_REGISTER_OFS, LocalReg.reg);
	} else {
		return -1;
	}
	
    return 0;
}

INT32 jmisp_eng_set_arb_dbg_en(JMISP_ENG_HANDLE *p_eng, UINT32 enable_eng)
{
	T_JMISP_SUB_MODE_REGISTER2 LocalReg;
	
	if (p_eng == NULL) {
		return -1;
	}
    if(p_eng->eng_id >= JMISP_ID_MAX_NUM)
    {
		DBG_ERR("error jmisp engine id\r\n");
		return -1;
    }
	
	LocalReg.bit.ARB_DBG_EN_PPU   = (enable_eng >> 0) & 0x1;
	LocalReg.bit.ARB_DBG_EN_POU   = (enable_eng >> 1) & 0x1;
	
	JMISP_ENG_SETREG(p_eng->reg_io_base + JMISP_SUB_MODE_REGISTER2_OFS, LocalReg.reg);
	
    return 0;
}

INT32 jmisp_eng_run_arb_dbg(JMISP_ENG_HANDLE *p_eng, UINT32 enable_eng)
{
	T_JMISP_SUB_MODE_REGISTER2 dbg_en_reg;
	T_JMISP_SUB_MODE_REGISTER3 LocalReg;
	
	if (p_eng == NULL) {
		return -1;
	}
    if(p_eng->eng_id >= JMISP_ID_MAX_NUM)
    {
		DBG_ERR("error jmisp engine id\r\n");
		return -1;
    }
	dbg_en_reg.reg = JMISP_ENG_GETREG(p_eng->reg_io_base + JMISP_SUB_MODE_REGISTER2_OFS);
	//dbg_en_reg = JMISP_ENG_GETREG(p_eng->reg_io_base + JMISP_SUB_MODE_REGISTER2_OFS);
	LocalReg.reg = 0;
	if (dbg_en_reg.bit.ARB_DBG_EN_PPU) {
		LocalReg.bit.ARB_DBG_NEXT_PPU = (enable_eng >> 0) & 0x1;
	}	
	JMISP_ENG_SETREG(p_eng->reg_io_base + JMISP_SUB_MODE_REGISTER3_OFS, LocalReg.reg);
	
	if (dbg_en_reg.bit.ARB_DBG_EN_POU) {
		UINT32 reg_val = JMISP_ENG_GETREG(p_eng->reg_io_base + 0xe0);
		JMISP_ENG_SETREG(p_eng->reg_io_base + 0xe0, reg_val | 0x1);
	}
	
    return 0;
}

UINT32 jmisp_eng_clr_flg(JMISP_ENG_HANDLE *p_eng, FLGPTN flg)
{
	return jmisp_eng_platform_flg_clear(p_eng, flg);
}
UINT32 jmisp_eng_wait_flg(JMISP_ENG_HANDLE *p_eng, PFLGPTN p_flgptn, FLGPTN flg)
{
	return jmisp_eng_platform_flg_wait(p_eng, p_flgptn, flg);
}
UINT32 jmisp_eng_wait_flg_timeout(JMISP_ENG_HANDLE *p_eng, PFLGPTN p_flgptn, FLGPTN flg, int timeout_tick)
{
	return jmisp_eng_platform_flg_wait_timeout(p_eng, p_flgptn, flg, timeout_tick);
}

void jmisp_eng_wait_framend(JMISP_ENG_HANDLE *p_eng, UINT32 pl_id)
{
    FLGPTN uiflag;
    ER er_return = E_OK;
	
	if (pl_id == 0) {
		er_return = jmisp_eng_wait_flg(p_eng, &uiflag, FLGPTN_JMISP_PL0_END);
	} else if (pl_id == 1) {
		er_return = jmisp_eng_wait_flg(p_eng, &uiflag, FLGPTN_JMISP_PL1_END);
	} else if (pl_id == 2) {
		er_return = jmisp_eng_wait_flg(p_eng, &uiflag, FLGPTN_JMISP_PL2_END);
	} else if (pl_id == 3) {
		er_return = jmisp_eng_wait_flg(p_eng, &uiflag, FLGPTN_JMISP_PL3_END);
	} 
    
    if(er_return != E_OK) {
        DBG_ERR("JMISP waitdone error\n\r");
    }
}

INT32 jmisp_eng_set_path_info(JMISP_ENG_HANDLE *p_eng, JMISP_ENG_PATH_INFO* path_info)
{
	UINT32 path_id = 0;
	UINT32 i = 0;
	UINT32 reg_ofs = 0;
	UINT32 reg_val = 0;
	UINT32 ring_en_num = 0;
	UINT32 pl_mask = 0;
	
	if (p_eng == NULL) {
		return -1;
	}
	
	if (p_eng->eng_id >= JMISP_ID_MAX_NUM) {
		DBG_ERR("error jmisp engine id\r\n");
		return -1;
    }
	
	if (path_info == NULL) {
		DBG_ERR("error path_info\r\n");
		return -1;
	}
	
	if (path_info->path_id >= JMISP_ENG_MAX_PATH_NUM) {
		DBG_ERR("invalid path id\r\n");
		return -1;
	}
	
	path_id = path_info->path_id;
	
	// init ring enable
	reg_ofs = PL0_RING_BUFFER_CONTROL_REGISTER0_OFS + (0x300*path_id);
	JMISP_ENG_SETREG(p_eng->reg_io_base + reg_ofs, 0);
	for (i = 0; i < JMISP_ENG_MAX_FRAME_NUM; i++) {
		if (path_info->frame[i].frame_addr > 0) {
			//frame_addr
			reg_ofs = PL0_FRAME_BUFFER0_ADDRESS_REGISTER0_OFS + (0x8*i) + (0x300*path_id);
			reg_val = path_info->frame[i].frame_addr & 0xFFFFFFFF;
			JMISP_ENG_SETREG(p_eng->reg_io_base + reg_ofs, reg_val);
			reg_ofs = PL0_FRAME_BUFFER0_ADDRESS_REGISTER1_OFS + (0x8*i) + (0x300*path_id);
			reg_val = (path_info->frame[i].frame_addr >> 32) & 0xF;
			JMISP_ENG_SETREG(p_eng->reg_io_base + reg_ofs, reg_val);
			
			// ring en/ring addr
			if (path_info->frame[i].ring_en) {
				ring_en_num++;
				if (ring_en_num > 2) {
					DBG_ERR("path id %d setting invalid: too many ring enable, skip frame%d ring setting\r\n", path_id, i);
				} else {
					reg_ofs = PL0_RING_BUFFER_CONTROL_REGISTER0_OFS + (0x300*path_id);
					reg_val = JMISP_ENG_GETREG(p_eng->reg_io_base + reg_ofs);
					reg_val |= (1 << i);
					JMISP_ENG_SETREG(p_eng->reg_io_base + reg_ofs, reg_val);	
					
					reg_ofs = PL0_RING_BUFFER0_ADDRESS_REGISTER0_OFS + (0x10*(ring_en_num-1)) + (0x300*path_id);
					reg_val = path_info->frame[i].ring_start_addr & 0xFFFFFFFF;
					JMISP_ENG_SETREG(p_eng->reg_io_base + reg_ofs, reg_val);
					
					reg_ofs = PL0_RING_BUFFER0_ADDRESS_REGISTER1_OFS + (0x10*(ring_en_num-1)) + (0x300*path_id);
					reg_val = (path_info->frame[i].ring_start_addr >> 32) & 0xF;
					JMISP_ENG_SETREG(p_eng->reg_io_base + reg_ofs, reg_val);
					
					reg_ofs = PL0_RING_BUFFER0_ADDRESS_REGISTER2_OFS + (0x10*(ring_en_num-1)) + (0x300*path_id);
					reg_val = path_info->frame[i].ring_end_addr & 0xFFFFFFFF;
					JMISP_ENG_SETREG(p_eng->reg_io_base + reg_ofs, reg_val);
					
					reg_ofs = PL0_RING_BUFFER0_ADDRESS_REGISTER3_OFS + (0x10*(ring_en_num-1)) + (0x300*path_id);
					reg_val = (path_info->frame[i].ring_end_addr >> 32) & 0xF;
					JMISP_ENG_SETREG(p_eng->reg_io_base + reg_ofs, reg_val);
				}
			}
			
			//UINT32 first_stripe_x_ofs;  //0xD0 FRAMEBUF0_FIRST_STRIPE_X_OFS
			//UINT32 middle_stripe_x_ofs; //0xD0 FRAMEBUF0_MIDDLE_STRIPE_X_OFS	
			{
				T_PL0_FRAME_BUFFER0_REGISTER0 LocalReg = {0};
				LocalReg.bit.PL0_FRAMEBUF0_FIRST_STRIPE_X_OFS  = path_info->frame[i].first_stripe_x_ofs;
				LocalReg.bit.PL0_FRAMEBUF0_MIDDLE_STRIPE_X_OFS = path_info->frame[i].middle_stripe_x_ofs;
				reg_ofs = PL0_FRAME_BUFFER0_REGISTER0_OFS + (0x10*i)  + (0x300*path_id);
				JMISP_ENG_SETREG(p_eng->reg_io_base + reg_ofs, LocalReg.reg);
			}
			
			//UINT32 first_slice_y_ofs;   //0xD8 FRAMEBUF0_SLICE_Y_OFS0
			reg_ofs = PL0_FRAME_BUFFER0_REGISTER2_OFS + (0x10*i)  + (0x300*path_id);
			reg_val = path_info->frame[i].first_slice_y_ofs;
			JMISP_ENG_SETREG(p_eng->reg_io_base + reg_ofs, reg_val);
			
			//UINT32 middle_slice_y_ofs;  //0xDC FRAMEBUF0_SLICE_Y_OFS1	
			reg_ofs = PL0_FRAME_BUFFER0_REGISTER3_OFS + (0x10*i)  + (0x300*path_id);
			reg_val = path_info->frame[i].middle_slice_y_ofs;
			JMISP_ENG_SETREG(p_eng->reg_io_base + reg_ofs, reg_val);
			
			//JMISP_ENG_BLOCK_OFS_INFO blk_ofs_info;
			//UINT32 blk_x_first_ofs;  //0x138 SLICEBUF0_BLOCK_X_OFS0
			//UINT32 blk_x_middle_ofs; //0x138 SLICEBUF0_BLOCK_X_OFS1
			{
				T_PL0_SLICE_BUFFER0_REGISTER2 LocalReg = {0};
				LocalReg.bit.PL0_SLICEBUF0_BLOCK_X_OFS0 = path_info->frame[i].blk_ofs_info.blk_x_first_ofs;
				LocalReg.bit.PL0_SLICEBUF0_BLOCK_X_OFS1 = path_info->frame[i].blk_ofs_info.blk_x_middle_ofs;
				reg_ofs = PL0_SLICE_BUFFER0_REGISTER2_OFS + (0x10*i)  + (0x300*path_id);
				JMISP_ENG_SETREG(p_eng->reg_io_base + reg_ofs, LocalReg.reg);
			}
			//UINT32 blk_y_first_ofs;  //0x130 SLICEBUF0_BLOCK_Y_OFS0
			reg_ofs = PL0_SLICE_BUFFER0_REGISTER0_OFS + (0x10*i) + (0x300*path_id);
			reg_val = path_info->frame[i].blk_ofs_info.blk_y_first_ofs;
			JMISP_ENG_SETREG(p_eng->reg_io_base + reg_ofs, reg_val);
			
			//UINT32 blk_y_middle_ofs; //0x134 SLICEBUF0_BLOCK_Y_OFS1
			reg_ofs = PL0_SLICE_BUFFER0_REGISTER1_OFS + (0x10*i) + (0x300*path_id);
			reg_val = path_info->frame[i].blk_ofs_info.blk_y_middle_ofs;
			JMISP_ENG_SETREG(p_eng->reg_io_base + reg_ofs, reg_val);
		}
	}
	
	for (i = 0; i < JMISP_ENG_MAX_PINGPONG_NUM; i++) {
		if (path_info->pingpong[i].pingpong_addr0 > 0) {
			//pp_addr
			reg_ofs = PL0_PING_PONG_BUFFER_ADDRESS_REGISTER0_OFS + (0x10*i) + (0x300*path_id);
			reg_val = path_info->pingpong[i].pingpong_addr0 & 0xFFFFFFFF;
			JMISP_ENG_SETREG(p_eng->reg_io_base + reg_ofs, reg_val);
			reg_ofs = PL0_PING_PONG_BUFFER_ADDRESS_REGISTER1_OFS + (0x10*i) + (0x300*path_id);
			reg_val = (path_info->pingpong[i].pingpong_addr0 >> 32) & 0xF;
			JMISP_ENG_SETREG(p_eng->reg_io_base + reg_ofs, reg_val);
			reg_ofs = PL0_PING_PONG_BUFFER_ADDRESS_REGISTER2_OFS + (0x10*i) + (0x300*path_id);
			reg_val = path_info->pingpong[i].pingpong_addr1 & 0xFFFFFFFF;
			JMISP_ENG_SETREG(p_eng->reg_io_base + reg_ofs, reg_val);
			reg_ofs = PL0_PING_PONG_BUFFER_ADDRESS_REGISTER3_OFS + (0x10*i) + (0x300*path_id);
			reg_val = (path_info->pingpong[i].pingpong_addr1 >> 32) & 0xF;
			JMISP_ENG_SETREG(p_eng->reg_io_base + reg_ofs, reg_val);

			
			//JMISP_ENG_BLOCK_OFS_INFO blk_ofs_info;
			//UINT32 blk_x_first_ofs;  //0x138 SLICEBUF0_BLOCK_X_OFS0
			//UINT32 blk_x_middle_ofs; //0x138 SLICEBUF0_BLOCK_X_OFS1
			{
				T_PL0_SLICE_PINGPONG_BUFFER0_REGISTER2 LocalReg = {0};
				LocalReg.bit.PL0_PINGPONG0_BLOCK_X_OFS0 = path_info->pingpong[i].blk_ofs_info.blk_x_first_ofs;
				LocalReg.bit.PL0_PINGPONG0_BLOCK_X_OFS1 = path_info->pingpong[i].blk_ofs_info.blk_x_middle_ofs;
				reg_ofs = PL0_SLICE_PINGPONG_BUFFER0_REGISTER2_OFS + (0x10*i)  + (0x300*path_id);
				JMISP_ENG_SETREG(p_eng->reg_io_base + reg_ofs, LocalReg.reg);
			}
			//UINT32 blk_y_first_ofs;  //0x130 SLICEBUF0_BLOCK_Y_OFS0
			reg_ofs = PL0_SLICE_PINGPONG_BUFFER0_REGISTER0_OFS + (0x10*i) + (0x300*path_id);
			reg_val = path_info->pingpong[i].blk_ofs_info.blk_y_first_ofs;
			JMISP_ENG_SETREG(p_eng->reg_io_base + reg_ofs, reg_val);
			
			//UINT32 blk_y_middle_ofs; //0x134 SLICEBUF0_BLOCK_Y_OFS1
			reg_ofs = PL0_SLICE_PINGPONG_BUFFER0_REGISTER1_OFS + (0x10*i) + (0x300*path_id);
			reg_val = path_info->pingpong[i].blk_ofs_info.blk_y_middle_ofs;
			JMISP_ENG_SETREG(p_eng->reg_io_base + reg_ofs, reg_val);
		}
	}
	
	// handshake & blk_skip
	pl_mask = ~(0xFF << (8*path_id));
	reg_ofs = JMISP_SUB_MODE_REGISTER0_OFS;
	reg_val = JMISP_ENG_GETREG(p_eng->reg_io_base + reg_ofs) & pl_mask;
	reg_val |= (path_info->handshake << (8*path_id));
	reg_val |= (path_info->blk_skip << (8*path_id+4));
	JMISP_ENG_SETREG(p_eng->reg_io_base + reg_ofs, reg_val);
	
	// slice/stripe num
	{
		T_PL0_SUB_IMAGE_REGISTER0 LocalReg = {0};
		LocalReg.bit.PL0_FRAMEBUF_SLICE_Y_NUM = path_info->slice_num;
		LocalReg.bit.PL0_FRAMEBUF_STRIPE_X_NUM = path_info->stripe_num;
		reg_ofs = PL0_SUB_IMAGE_REGISTER0_OFS + (0x300*path_id);
		JMISP_ENG_SETREG(p_eng->reg_io_base + reg_ofs, LocalReg.reg);
	}
	
	// signal mode
	pl_mask = ~(0x1 << path_id);
	reg_ofs = JMISP_MAIN_MODE_REGISTER0_OFS;
	reg_val = JMISP_ENG_GETREG(p_eng->reg_io_base + reg_ofs) & pl_mask;
	reg_val |= (path_info->signal_mode_en << path_id);
	JMISP_ENG_SETREG(p_eng->reg_io_base + reg_ofs, reg_val);
	
	// blk_num_info
	{
		T_PL0_SUB_IMAGE_REGISTER2 LocalReg = {0};
		LocalReg.bit.PL0_SLICEBUF_BLOCK_Y_NUM = path_info->blk_num_info.blk_y_middle_num;
		LocalReg.bit.PL0_SLICEBUF_BLOCK_X_NUM = path_info->blk_num_info.blk_x_middle_num;
		reg_ofs = PL0_SUB_IMAGE_REGISTER2_OFS + (0x300*path_id);
		JMISP_ENG_SETREG(p_eng->reg_io_base + reg_ofs, LocalReg.reg);
	}
	
	{
		T_PL0_SUB_IMAGE_REGISTER3 LocalReg = {0};
		LocalReg.bit.PL0_FIRST_STRIPE_BLOCK_X_NUM = path_info->blk_num_info.blk_x_first_num;
		LocalReg.bit.PL0_LAST_STRIPE_BLOCK_X_NUM  = path_info->blk_num_info.blk_x_last_num;
		reg_ofs = PL0_SUB_IMAGE_REGISTER3_OFS + (0x300*path_id);
		JMISP_ENG_SETREG(p_eng->reg_io_base + reg_ofs, LocalReg.reg);
	}
	
	{
		T_PL0_SUB_IMAGE_REGISTER4 LocalReg = {0};
		LocalReg.bit.PL0_FIRST_SLICE_BLOCK_Y_NUM = path_info->blk_num_info.blk_y_first_num;
		LocalReg.bit.PL0_LAST_SLICE_BLOCK_Y_NUM  = path_info->blk_num_info.blk_y_last_num;
		reg_ofs = PL0_SUB_IMAGE_REGISTER4_OFS + (0x300*path_id);
		JMISP_ENG_SETREG(p_eng->reg_io_base + reg_ofs, LocalReg.reg);
	}
	
	// cust parameter (0x3c0~0x3c4)
	reg_ofs = PL0_CUST_VALUE_REGISTER0_OFS + (0x300*path_id);
	JMISP_ENG_SETREG(p_eng->reg_io_base + reg_ofs, path_info->cust[0]);
	reg_ofs = PL0_CUST_VALUE_REGISTER1_OFS + (0x300*path_id);
	JMISP_ENG_SETREG(p_eng->reg_io_base + reg_ofs, path_info->cust[1]);

	// ppu pipe buffer address parameter (0x3c8~0x3cc)
	reg_ofs = PL0_CUST_VALUE_REGISTER2_OFS + (0x300*path_id);
	JMISP_ENG_SETREG(p_eng->reg_io_base + reg_ofs, path_info->ppu_pipebuf_lsbaddr[0]);
	reg_ofs = PL0_CUST_VALUE_REGISTER3_OFS + (0x300*path_id);
	JMISP_ENG_SETREG(p_eng->reg_io_base + reg_ofs, path_info->ppu_pipebuf_lsbaddr[1]);

	// wait_en
	if (path_info->handshake > 0) {
		reg_ofs = JMISP_SUB_MODE_REGISTER1_OFS;
		reg_val = JMISP_ENG_GETREG(p_eng->reg_io_base + reg_ofs);
		pl_mask = (1 << (4*(path_info->handshake-1)+path_id));
		if (path_info->wait_en) {
			reg_val = reg_val | pl_mask;
		} else {
			reg_val = reg_val & (~pl_mask);
		}
		JMISP_ENG_SETREG(p_eng->reg_io_base + reg_ofs, reg_val);
	}
	
	return 0;
}

INT32 jmisp_eng_get_path_info(JMISP_ENG_HANDLE *p_eng, JMISP_ENG_PATH_INFO* path_info)
{
	UINT32 path_id = 0;
	UINT32 i = 0;
	UINT32 reg_ofs = 0;
	UINT32 reg_val = 0;
	UINT32 ring_en = 0;
	UINT32 ring_en_num = 0;
	
	if (p_eng == NULL) {
		return -1;
	}
	
	if (p_eng->eng_id >= JMISP_ID_MAX_NUM) {
		DBG_ERR("error jmisp engine id\r\n");
		return -1;
    }
	
	if (path_info == NULL) {
		DBG_ERR("error path_info\r\n");
		return -1;
	}
	
	if (path_info->path_id >= JMISP_ENG_MAX_PATH_NUM) {
		DBG_ERR("invalid path id\r\n");
		return -1;
	}
	
	path_id = path_info->path_id;
	
	// ring enable
	reg_ofs = PL0_RING_BUFFER_CONTROL_REGISTER0_OFS + (0x300*path_id);
	ring_en = JMISP_ENG_GETREG(p_eng->reg_io_base + reg_ofs);

	for (i = 0; i < JMISP_ENG_MAX_FRAME_NUM; i++) {
		reg_ofs = PL0_FRAME_BUFFER0_ADDRESS_REGISTER0_OFS + (0x8*i) + (0x300*path_id);
		reg_val = JMISP_ENG_GETREG(p_eng->reg_io_base + reg_ofs);
		path_info->frame[i].frame_addr = reg_val;
		reg_ofs = PL0_FRAME_BUFFER0_ADDRESS_REGISTER1_OFS + (0x8*i) + (0x300*path_id);
		reg_val = JMISP_ENG_GETREG(p_eng->reg_io_base + reg_ofs);
		path_info->frame[i].frame_addr = path_info->frame[i].frame_addr | ((UINT64)reg_val << 32);
		
		if ((ring_en >> i) & 0x1) {
			path_info->frame[i].ring_en = 1;
			
			reg_ofs = PL0_RING_BUFFER0_ADDRESS_REGISTER0_OFS + (0x10*ring_en_num) + (0x300*path_id);
			reg_val = JMISP_ENG_GETREG(p_eng->reg_io_base + reg_ofs);
			path_info->frame[i].ring_start_addr = reg_val;
			reg_ofs = PL0_RING_BUFFER0_ADDRESS_REGISTER1_OFS + (0x10*ring_en_num) + (0x300*path_id);
			reg_val = JMISP_ENG_GETREG(p_eng->reg_io_base + reg_ofs);
			path_info->frame[i].ring_start_addr = path_info->frame[i].ring_start_addr | ((UINT64)reg_val << 32);
			
			reg_ofs = PL0_RING_BUFFER0_ADDRESS_REGISTER2_OFS + (0x10*ring_en_num) + (0x300*path_id);
			reg_val = JMISP_ENG_GETREG(p_eng->reg_io_base + reg_ofs);
			path_info->frame[i].ring_end_addr = reg_val;
			reg_ofs = PL0_RING_BUFFER0_ADDRESS_REGISTER3_OFS + (0x10*ring_en_num) + (0x300*path_id);
			reg_val = JMISP_ENG_GETREG(p_eng->reg_io_base + reg_ofs);
			path_info->frame[i].ring_end_addr = path_info->frame[i].ring_end_addr | ((UINT64)reg_val << 32);
			
			ring_en_num++;
		} else {
			path_info->frame[i].ring_en = 0;
			path_info->frame[i].ring_start_addr = 0;
			path_info->frame[i].ring_end_addr   = 0;
		}
		
		//UINT32 first_stripe_x_ofs;  //0xD0 FRAMEBUF0_FIRST_STRIPE_X_OFS
		//UINT32 middle_stripe_x_ofs; //0xD0 FRAMEBUF0_MIDDLE_STRIPE_X_OFS
		{
			T_PL0_FRAME_BUFFER0_REGISTER0 LocalReg = {0};
			
			reg_ofs = PL0_FRAME_BUFFER0_REGISTER0_OFS + (0x10*i)  + (0x300*path_id);
			LocalReg.reg = JMISP_ENG_GETREG(p_eng->reg_io_base + reg_ofs);
			path_info->frame[i].first_stripe_x_ofs  = LocalReg.bit.PL0_FRAMEBUF0_FIRST_STRIPE_X_OFS;
			path_info->frame[i].middle_stripe_x_ofs = LocalReg.bit.PL0_FRAMEBUF0_MIDDLE_STRIPE_X_OFS;
		}
		
		//UINT32 first_slice_y_ofs;   //0xD8 FRAMEBUF0_SLICE_Y_OFS0
		reg_ofs = PL0_FRAME_BUFFER0_REGISTER2_OFS + (0x10*i)  + (0x300*path_id);
		reg_val = JMISP_ENG_GETREG(p_eng->reg_io_base + reg_ofs);
		path_info->frame[i].first_slice_y_ofs = reg_val;
		
		//UINT32 middle_slice_y_ofs;  //0xDC FRAMEBUF0_SLICE_Y_OFS1	
		reg_ofs = PL0_FRAME_BUFFER0_REGISTER3_OFS + (0x10*i)  + (0x300*path_id);
		reg_val = JMISP_ENG_GETREG(p_eng->reg_io_base + reg_ofs);
		path_info->frame[i].middle_slice_y_ofs = reg_val;
		
		//JMISP_ENG_BLOCK_OFS_INFO blk_ofs_info;
		//UINT32 blk_x_first_ofs;  //0x138 SLICEBUF0_BLOCK_X_OFS0
		//UINT32 blk_x_middle_ofs; //0x138 SLICEBUF0_BLOCK_X_OFS1
		{
			T_PL0_SLICE_BUFFER0_REGISTER2 LocalReg = {0};
			
			reg_ofs = PL0_SLICE_BUFFER0_REGISTER2_OFS + (0x10*i)  + (0x300*path_id);
			LocalReg.reg = JMISP_ENG_GETREG(p_eng->reg_io_base + reg_ofs);	
			path_info->frame[i].blk_ofs_info.blk_x_first_ofs  = LocalReg.bit.PL0_SLICEBUF0_BLOCK_X_OFS0;
			path_info->frame[i].blk_ofs_info.blk_x_middle_ofs = LocalReg.bit.PL0_SLICEBUF0_BLOCK_X_OFS1;
		}
		
		//UINT32 blk_y_first_ofs;  //0x130 SLICEBUF0_BLOCK_Y_OFS0
		reg_ofs = PL0_SLICE_BUFFER0_REGISTER0_OFS + (0x10*i) + (0x300*path_id);
		reg_val = JMISP_ENG_GETREG(p_eng->reg_io_base + reg_ofs);
		path_info->frame[i].blk_ofs_info.blk_y_first_ofs = reg_val;
		
		//UINT32 blk_y_middle_ofs; //0x134 SLICEBUF0_BLOCK_Y_OFS1
		reg_ofs = PL0_SLICE_BUFFER0_REGISTER1_OFS + (0x10*i) + (0x300*path_id);
		reg_val = JMISP_ENG_GETREG(p_eng->reg_io_base + reg_ofs);
		path_info->frame[i].blk_ofs_info.blk_y_middle_ofs = reg_val;
	}
	
	for (i = 0; i < JMISP_ENG_MAX_PINGPONG_NUM; i++) {
		//pp_addr
		reg_ofs = PL0_PING_PONG_BUFFER_ADDRESS_REGISTER0_OFS + (0x10*i) + (0x300*path_id);
		reg_val = JMISP_ENG_GETREG(p_eng->reg_io_base + reg_ofs);
		path_info->pingpong[i].pingpong_addr0 = reg_val;

		reg_ofs = PL0_PING_PONG_BUFFER_ADDRESS_REGISTER1_OFS + (0x10*i) + (0x300*path_id);
		reg_val = JMISP_ENG_GETREG(p_eng->reg_io_base + reg_ofs);
		path_info->pingpong[i].pingpong_addr0 = path_info->pingpong[i].pingpong_addr0 | ((UINT64)reg_val << 32);
		
		reg_ofs = PL0_PING_PONG_BUFFER_ADDRESS_REGISTER2_OFS + (0x10*i) + (0x300*path_id);
		reg_val = JMISP_ENG_GETREG(p_eng->reg_io_base + reg_ofs);
		path_info->pingpong[i].pingpong_addr1 = reg_val;

		reg_ofs = PL0_PING_PONG_BUFFER_ADDRESS_REGISTER3_OFS + (0x10*i) + (0x300*path_id);
		reg_val = JMISP_ENG_GETREG(p_eng->reg_io_base + reg_ofs);
		path_info->pingpong[i].pingpong_addr1 = path_info->pingpong[i].pingpong_addr1 | ((UINT64)reg_val << 32);
		
		//JMISP_ENG_BLOCK_OFS_INFO blk_ofs_info;
		//UINT32 blk_x_first_ofs;  //0x138 SLICEBUF0_BLOCK_X_OFS0
		//UINT32 blk_x_middle_ofs; //0x138 SLICEBUF0_BLOCK_X_OFS1
		{
			T_PL0_SLICE_PINGPONG_BUFFER0_REGISTER2 LocalReg = {0};
			reg_ofs = PL0_SLICE_PINGPONG_BUFFER0_REGISTER2_OFS + (0x10*i)  + (0x300*path_id);
			LocalReg.reg = JMISP_ENG_GETREG(p_eng->reg_io_base + reg_ofs);
			path_info->pingpong[i].blk_ofs_info.blk_x_first_ofs  = LocalReg.bit.PL0_PINGPONG0_BLOCK_X_OFS0;
			path_info->pingpong[i].blk_ofs_info.blk_x_middle_ofs = LocalReg.bit.PL0_PINGPONG0_BLOCK_X_OFS1;
		}
		
		
		//UINT32 blk_y_first_ofs;  //0x130 SLICEBUF0_BLOCK_Y_OFS0
		reg_ofs = PL0_SLICE_PINGPONG_BUFFER0_REGISTER0_OFS + (0x10*i) + (0x300*path_id);
		reg_val = JMISP_ENG_GETREG(p_eng->reg_io_base + reg_ofs);
		path_info->pingpong[i].blk_ofs_info.blk_y_first_ofs = reg_val;

		//UINT32 blk_y_middle_ofs; //0x134 SLICEBUF0_BLOCK_Y_OFS1
		reg_ofs = PL0_SLICE_PINGPONG_BUFFER0_REGISTER1_OFS + (0x10*i) + (0x300*path_id);
		reg_val = JMISP_ENG_GETREG(p_eng->reg_io_base + reg_ofs);
		path_info->pingpong[i].blk_ofs_info.blk_y_middle_ofs = reg_val;
	}
	
	// handshake & blk_skip
	reg_ofs = JMISP_SUB_MODE_REGISTER0_OFS;
	reg_val = JMISP_ENG_GETREG(p_eng->reg_io_base + reg_ofs);
	path_info->handshake = (reg_val >> (8*path_id)) & 0xF;
	path_info->blk_skip  = (reg_val >> (8*path_id+4)) & 0xF;
	
	// slice/stripe num
	{
		T_PL0_SUB_IMAGE_REGISTER0 LocalReg = {0};
		reg_ofs = PL0_SUB_IMAGE_REGISTER0_OFS + (0x300*path_id);
		LocalReg.reg = JMISP_ENG_GETREG(p_eng->reg_io_base + reg_ofs);
		path_info->slice_num  = (UINT32)LocalReg.bit.PL0_FRAMEBUF_SLICE_Y_NUM;
		path_info->stripe_num = (UINT32)LocalReg.bit.PL0_FRAMEBUF_STRIPE_X_NUM;
	}
	
	// signal mode
	reg_ofs = JMISP_MAIN_MODE_REGISTER0_OFS;
	reg_val = JMISP_ENG_GETREG(p_eng->reg_io_base + reg_ofs);
	path_info->signal_mode_en = (reg_val >> path_id) & 0x1;
	
	// blk_num_info
	{
		T_PL0_SUB_IMAGE_REGISTER2 LocalReg = {0};
		reg_ofs = PL0_SUB_IMAGE_REGISTER2_OFS + (0x300*path_id);
		LocalReg.reg = JMISP_ENG_GETREG(p_eng->reg_io_base + reg_ofs);
		path_info->blk_num_info.blk_y_middle_num = LocalReg.bit.PL0_SLICEBUF_BLOCK_Y_NUM;
		path_info->blk_num_info.blk_x_middle_num = LocalReg.bit.PL0_SLICEBUF_BLOCK_X_NUM;
	}
	
	{
		T_PL0_SUB_IMAGE_REGISTER3 LocalReg = {0};
		reg_ofs = PL0_SUB_IMAGE_REGISTER3_OFS + (0x300*path_id);
		LocalReg.reg = JMISP_ENG_GETREG(p_eng->reg_io_base + reg_ofs);
		path_info->blk_num_info.blk_x_first_num = LocalReg.bit.PL0_FIRST_STRIPE_BLOCK_X_NUM;
		path_info->blk_num_info.blk_x_last_num  = LocalReg.bit.PL0_LAST_STRIPE_BLOCK_X_NUM;
	}
	
	{
		T_PL0_SUB_IMAGE_REGISTER4 LocalReg = {0};
		reg_ofs = PL0_SUB_IMAGE_REGISTER4_OFS + (0x300*path_id);
		LocalReg.reg = JMISP_ENG_GETREG(p_eng->reg_io_base + reg_ofs);
		path_info->blk_num_info.blk_y_first_num = LocalReg.bit.PL0_FIRST_SLICE_BLOCK_Y_NUM;
		path_info->blk_num_info.blk_y_last_num  = LocalReg.bit.PL0_LAST_SLICE_BLOCK_Y_NUM;
	}
	
	// cust parameter (0x3c0~0x3c4)
	reg_ofs = PL0_CUST_VALUE_REGISTER0_OFS + (0x300*path_id);
	path_info->cust[0] = JMISP_ENG_GETREG(p_eng->reg_io_base + reg_ofs);
	reg_ofs = PL0_CUST_VALUE_REGISTER1_OFS + (0x300*path_id);
	path_info->cust[1] = JMISP_ENG_GETREG(p_eng->reg_io_base + reg_ofs);

	// ppu pipe buffer address parameter (0x3c8~0x3cc)
	reg_ofs = PL0_CUST_VALUE_REGISTER2_OFS + (0x300*path_id);
	path_info->ppu_pipebuf_lsbaddr[0] = JMISP_ENG_GETREG(p_eng->reg_io_base + reg_ofs);
	reg_ofs = PL0_CUST_VALUE_REGISTER3_OFS + (0x300*path_id);
	path_info->ppu_pipebuf_lsbaddr[1] = JMISP_ENG_GETREG(p_eng->reg_io_base + reg_ofs);
	
	// wait_en
	if (path_info->handshake > 0) {
		reg_ofs = JMISP_SUB_MODE_REGISTER1_OFS;
		reg_val = JMISP_ENG_GETREG(p_eng->reg_io_base + reg_ofs);
		path_info->wait_en = (reg_val >> (4*(path_info->handshake-1)+path_id)) & 0x1;
	} else {
		path_info->wait_en = 0;
	}
	
	return 0;
}

INT32 jmisp_eng_set_frame_addr(JMISP_ENG_HANDLE *p_eng, JMISP_ENG_FRAME_ADDR_INFO* frame_info)
{
	UINT32 path_id = 0;
	UINT32 i = 0;
	UINT32 reg_ofs = 0;
	UINT32 reg_val = 0;
	
	if (p_eng == NULL) {
		return -1;
	}
	
	if (p_eng->eng_id >= JMISP_ID_MAX_NUM) {
		DBG_ERR("error jmisp engine id\r\n");
		return -1;
    }
	
	if (frame_info == NULL) {
		DBG_ERR("error frame_info\r\n");
		return -1;
	}
	
	if (frame_info->path_id >= JMISP_ENG_MAX_PATH_NUM) {
		DBG_ERR("invalid path id\r\n");
		return -1;
	}
	
	path_id = frame_info->path_id;
	
	for (i = 0; i < JMISP_ENG_MAX_FRAME_NUM; i++) {
		//frame_addr
		reg_ofs = PL0_FRAME_BUFFER0_ADDRESS_REGISTER0_OFS + (0x8*i) + (0x300*path_id);
		reg_val = frame_info->frame_addr[i] & 0xFFFFFFFF;
		JMISP_ENG_SETREG(p_eng->reg_io_base + reg_ofs, reg_val);
		reg_ofs = PL0_FRAME_BUFFER0_ADDRESS_REGISTER1_OFS + (0x8*i) + (0x300*path_id);
		reg_val = (frame_info->frame_addr[i] >> 32) & 0xF;
		JMISP_ENG_SETREG(p_eng->reg_io_base + reg_ofs, reg_val);
	}
	
	return 0;
}

INT32 jmisp_eng_set_frm_upd_path_info(JMISP_ENG_HANDLE *p_eng, JMISP_ENG_FRM_UPD_PATH_INFO* path_info)
{
	UINT32 path_id = 0;
	UINT32 i = 0;
	UINT32 reg_ofs = 0;
	UINT32 reg_val = 0;
	UINT32 ring_en_num = 0;
	
	if (p_eng == NULL) {
		return -1;
	}
	
	if (p_eng->eng_id >= JMISP_ID_MAX_NUM) {
		DBG_ERR("error jmisp engine id\r\n");
		return -1;
    }
	
	if (path_info == NULL) {
		DBG_ERR("error path_info\r\n");
		return -1;
	}
	
	if (path_info->path_id >= JMISP_ENG_MAX_PATH_NUM) {
		DBG_ERR("invalid path id\r\n");
		return -1;
	}
	
	path_id = path_info->path_id;
	
	// init ring enable
	reg_ofs = PL0_RING_BUFFER_CONTROL_REGISTER0_OFS + (0x300*path_id);
	JMISP_ENG_SETREG(p_eng->reg_io_base + reg_ofs, 0);
	for (i = 0; i < JMISP_ENG_MAX_FRAME_NUM; i++) {
		if (path_info->frame[i].frame_addr > 0) {
			//frame_addr
			reg_ofs = PL0_FRAME_BUFFER0_ADDRESS_REGISTER0_OFS + (0x8*i) + (0x300*path_id);
			reg_val = path_info->frame[i].frame_addr & 0xFFFFFFFF;
			JMISP_ENG_SETREG(p_eng->reg_io_base + reg_ofs, reg_val);
			reg_ofs = PL0_FRAME_BUFFER0_ADDRESS_REGISTER1_OFS + (0x8*i) + (0x300*path_id);
			reg_val = (path_info->frame[i].frame_addr >> 32) & 0xF;
			JMISP_ENG_SETREG(p_eng->reg_io_base + reg_ofs, reg_val);
			
			// ring en/ring addr
			if (path_info->frame[i].ring_en) {
				ring_en_num++;
				if (ring_en_num > 2) {
					DBG_ERR("path id %d setting invalid: too many ring enable, skip frame%d ring setting\r\n", path_id, i);
				} else {
					reg_ofs = PL0_RING_BUFFER_CONTROL_REGISTER0_OFS + (0x300*path_id);
					reg_val = JMISP_ENG_GETREG(p_eng->reg_io_base + reg_ofs);
					reg_val |= (1 << i);
					JMISP_ENG_SETREG(p_eng->reg_io_base + reg_ofs, reg_val);	
					
					reg_ofs = PL0_RING_BUFFER0_ADDRESS_REGISTER0_OFS + (0x10*(ring_en_num-1)) + (0x300*path_id);
					reg_val = path_info->frame[i].ring_start_addr & 0xFFFFFFFF;
					JMISP_ENG_SETREG(p_eng->reg_io_base + reg_ofs, reg_val);
					
					reg_ofs = PL0_RING_BUFFER0_ADDRESS_REGISTER1_OFS + (0x10*(ring_en_num-1)) + (0x300*path_id);
					reg_val = (path_info->frame[i].ring_start_addr >> 32) & 0xF;
					JMISP_ENG_SETREG(p_eng->reg_io_base + reg_ofs, reg_val);
					
					reg_ofs = PL0_RING_BUFFER0_ADDRESS_REGISTER2_OFS + (0x10*(ring_en_num-1)) + (0x300*path_id);
					reg_val = path_info->frame[i].ring_end_addr & 0xFFFFFFFF;
					JMISP_ENG_SETREG(p_eng->reg_io_base + reg_ofs, reg_val);
					
					reg_ofs = PL0_RING_BUFFER0_ADDRESS_REGISTER3_OFS + (0x10*(ring_en_num-1)) + (0x300*path_id);
					reg_val = (path_info->frame[i].ring_end_addr >> 32) & 0xF;
					JMISP_ENG_SETREG(p_eng->reg_io_base + reg_ofs, reg_val);
				}
			}

			{
				T_PL0_SLICE_BUFFER0_REGISTER2 LocalReg = {0};
				LocalReg.bit.PL0_SLICEBUF0_BLOCK_X_OFS0 = path_info->frame[i].blk_ofs_info.blk_x_first_ofs;
				LocalReg.bit.PL0_SLICEBUF0_BLOCK_X_OFS1 = path_info->frame[i].blk_ofs_info.blk_x_middle_ofs;
				reg_ofs = PL0_SLICE_BUFFER0_REGISTER2_OFS + (0x10*i)  + (0x300*path_id);
				JMISP_ENG_SETREG(p_eng->reg_io_base + reg_ofs, LocalReg.reg);
			}
			//UINT32 blk_y_first_ofs;  //0x130 SLICEBUF0_BLOCK_Y_OFS0
			reg_ofs = PL0_SLICE_BUFFER0_REGISTER0_OFS + (0x10*i) + (0x300*path_id);
			reg_val = path_info->frame[i].blk_ofs_info.blk_y_first_ofs;
			JMISP_ENG_SETREG(p_eng->reg_io_base + reg_ofs, reg_val);
			
			//UINT32 blk_y_middle_ofs; //0x134 SLICEBUF0_BLOCK_Y_OFS1
			reg_ofs = PL0_SLICE_BUFFER0_REGISTER1_OFS + (0x10*i) + (0x300*path_id);
			reg_val = path_info->frame[i].blk_ofs_info.blk_y_middle_ofs;
			JMISP_ENG_SETREG(p_eng->reg_io_base + reg_ofs, reg_val);
		}
	}
	
	for (i = 0; i < JMISP_ENG_MAX_PINGPONG_NUM; i++) {
		if (path_info->pingpong[i].pingpong_addr0 > 0) {
			//pp_addr
			reg_ofs = PL0_PING_PONG_BUFFER_ADDRESS_REGISTER0_OFS + (0x10*i) + (0x300*path_id);
			reg_val = path_info->pingpong[i].pingpong_addr0 & 0xFFFFFFFF;
			JMISP_ENG_SETREG(p_eng->reg_io_base + reg_ofs, reg_val);
			reg_ofs = PL0_PING_PONG_BUFFER_ADDRESS_REGISTER1_OFS + (0x10*i) + (0x300*path_id);
			reg_val = (path_info->pingpong[i].pingpong_addr0 >> 32) & 0xF;
			JMISP_ENG_SETREG(p_eng->reg_io_base + reg_ofs, reg_val);
			reg_ofs = PL0_PING_PONG_BUFFER_ADDRESS_REGISTER2_OFS + (0x10*i) + (0x300*path_id);
			reg_val = path_info->pingpong[i].pingpong_addr1 & 0xFFFFFFFF;
			JMISP_ENG_SETREG(p_eng->reg_io_base + reg_ofs, reg_val);
			reg_ofs = PL0_PING_PONG_BUFFER_ADDRESS_REGISTER3_OFS + (0x10*i) + (0x300*path_id);
			reg_val = (path_info->pingpong[i].pingpong_addr1 >> 32) & 0xF;
			JMISP_ENG_SETREG(p_eng->reg_io_base + reg_ofs, reg_val);

			
			//JMISP_ENG_BLOCK_OFS_INFO blk_ofs_info;
			//UINT32 blk_x_first_ofs;  //0x138 SLICEBUF0_BLOCK_X_OFS0
			//UINT32 blk_x_middle_ofs; //0x138 SLICEBUF0_BLOCK_X_OFS1
			{
				T_PL0_SLICE_PINGPONG_BUFFER0_REGISTER2 LocalReg = {0};
				LocalReg.bit.PL0_PINGPONG0_BLOCK_X_OFS0 = path_info->pingpong[i].blk_ofs_info.blk_x_first_ofs;
				LocalReg.bit.PL0_PINGPONG0_BLOCK_X_OFS1 = path_info->pingpong[i].blk_ofs_info.blk_x_middle_ofs;
				reg_ofs = PL0_SLICE_PINGPONG_BUFFER0_REGISTER2_OFS + (0x10*i)  + (0x300*path_id);
				JMISP_ENG_SETREG(p_eng->reg_io_base + reg_ofs, LocalReg.reg);
			}
			//UINT32 blk_y_first_ofs;  //0x130 SLICEBUF0_BLOCK_Y_OFS0
			reg_ofs = PL0_SLICE_PINGPONG_BUFFER0_REGISTER0_OFS + (0x10*i) + (0x300*path_id);
			reg_val = path_info->pingpong[i].blk_ofs_info.blk_y_first_ofs;
			JMISP_ENG_SETREG(p_eng->reg_io_base + reg_ofs, reg_val);
			
			//UINT32 blk_y_middle_ofs; //0x134 SLICEBUF0_BLOCK_Y_OFS1
			reg_ofs = PL0_SLICE_PINGPONG_BUFFER0_REGISTER1_OFS + (0x10*i) + (0x300*path_id);
			reg_val = path_info->pingpong[i].blk_ofs_info.blk_y_middle_ofs;
			JMISP_ENG_SETREG(p_eng->reg_io_base + reg_ofs, reg_val);
		}
	}
		
	// cust parameter (0x3c0~0x3c4)
	reg_ofs = PL0_CUST_VALUE_REGISTER0_OFS + (0x300*path_id);
	JMISP_ENG_SETREG(p_eng->reg_io_base + reg_ofs, path_info->cust[0]);
	reg_ofs = PL0_CUST_VALUE_REGISTER1_OFS + (0x300*path_id);
	JMISP_ENG_SETREG(p_eng->reg_io_base + reg_ofs, path_info->cust[1]);
	
	// ppu pipe buffer address parameter (0x3c8~0x3cc)
	reg_ofs = PL0_CUST_VALUE_REGISTER2_OFS + (0x300*path_id);
	JMISP_ENG_SETREG(p_eng->reg_io_base + reg_ofs, path_info->ppu_pipebuf_lsbaddr[0]);
	reg_ofs = PL0_CUST_VALUE_REGISTER3_OFS + (0x300*path_id);
	JMISP_ENG_SETREG(p_eng->reg_io_base + reg_ofs, path_info->ppu_pipebuf_lsbaddr[1]);

	return 0;
}

INT32 jmisp_eng_get_frm_upd_path_info(JMISP_ENG_HANDLE *p_eng, JMISP_ENG_FRM_UPD_PATH_INFO* path_info)
{
	UINT32 path_id = 0;
	UINT32 i = 0;
	UINT32 reg_ofs = 0;
	UINT32 reg_val = 0;
	UINT32 ring_en = 0;
	UINT32 ring_en_num = 0;
	
	if (p_eng == NULL) {
		return -1;
	}
	
	if (p_eng->eng_id >= JMISP_ID_MAX_NUM) {
		DBG_ERR("error jmisp engine id\r\n");
		return -1;
    }
	
	if (path_info == NULL) {
		DBG_ERR("error path_info\r\n");
		return -1;
	}
	
	if (path_info->path_id >= JMISP_ENG_MAX_PATH_NUM) {
		DBG_ERR("invalid path id\r\n");
		return -1;
	}
	
	path_id = path_info->path_id;
	
	// ring enable
	reg_ofs = PL0_RING_BUFFER_CONTROL_REGISTER0_OFS + (0x300*path_id);
	ring_en = JMISP_ENG_GETREG(p_eng->reg_io_base + reg_ofs);

	for (i = 0; i < JMISP_ENG_MAX_FRAME_NUM; i++) {
		reg_ofs = PL0_FRAME_BUFFER0_ADDRESS_REGISTER0_OFS + (0x8*i) + (0x300*path_id);
		reg_val = JMISP_ENG_GETREG(p_eng->reg_io_base + reg_ofs);
		path_info->frame[i].frame_addr = reg_val;
		reg_ofs = PL0_FRAME_BUFFER0_ADDRESS_REGISTER1_OFS + (0x8*i) + (0x300*path_id);
		reg_val = JMISP_ENG_GETREG(p_eng->reg_io_base + reg_ofs);
		path_info->frame[i].frame_addr = path_info->frame[i].frame_addr | ((UINT64)reg_val << 32);
		
		if ((ring_en >> i) & 0x1) {
			path_info->frame[i].ring_en = 1;
			
			reg_ofs = PL0_RING_BUFFER0_ADDRESS_REGISTER0_OFS + (0x10*ring_en_num) + (0x300*path_id);
			reg_val = JMISP_ENG_GETREG(p_eng->reg_io_base + reg_ofs);
			path_info->frame[i].ring_start_addr = reg_val;
			reg_ofs = PL0_RING_BUFFER0_ADDRESS_REGISTER1_OFS + (0x10*ring_en_num) + (0x300*path_id);
			reg_val = JMISP_ENG_GETREG(p_eng->reg_io_base + reg_ofs);
			path_info->frame[i].ring_start_addr = path_info->frame[i].ring_start_addr | ((UINT64)reg_val << 32);
			
			reg_ofs = PL0_RING_BUFFER0_ADDRESS_REGISTER2_OFS + (0x10*ring_en_num) + (0x300*path_id);
			reg_val = JMISP_ENG_GETREG(p_eng->reg_io_base + reg_ofs);
			path_info->frame[i].ring_end_addr = reg_val;
			reg_ofs = PL0_RING_BUFFER0_ADDRESS_REGISTER3_OFS + (0x10*ring_en_num) + (0x300*path_id);
			reg_val = JMISP_ENG_GETREG(p_eng->reg_io_base + reg_ofs);
			path_info->frame[i].ring_end_addr = path_info->frame[i].ring_end_addr | ((UINT64)reg_val << 32);
			
			ring_en_num++;
		} else {
			path_info->frame[i].ring_en = 0;
			path_info->frame[i].ring_start_addr = 0;
			path_info->frame[i].ring_end_addr   = 0;
		}
		
		//JMISP_ENG_BLOCK_OFS_INFO blk_ofs_info;
		//UINT32 blk_x_first_ofs;  //0x138 SLICEBUF0_BLOCK_X_OFS0
		//UINT32 blk_x_middle_ofs; //0x138 SLICEBUF0_BLOCK_X_OFS1
		{
			T_PL0_SLICE_BUFFER0_REGISTER2 LocalReg = {0};
			
			reg_ofs = PL0_SLICE_BUFFER0_REGISTER2_OFS + (0x10*i)  + (0x300*path_id);
			LocalReg.reg = JMISP_ENG_GETREG(p_eng->reg_io_base + reg_ofs);	
			path_info->frame[i].blk_ofs_info.blk_x_first_ofs  = LocalReg.bit.PL0_SLICEBUF0_BLOCK_X_OFS0;
			path_info->frame[i].blk_ofs_info.blk_x_middle_ofs = LocalReg.bit.PL0_SLICEBUF0_BLOCK_X_OFS1;
		}
		
		//UINT32 blk_y_first_ofs;  //0x130 SLICEBUF0_BLOCK_Y_OFS0
		reg_ofs = PL0_SLICE_BUFFER0_REGISTER0_OFS + (0x10*i) + (0x300*path_id);
		reg_val = JMISP_ENG_GETREG(p_eng->reg_io_base + reg_ofs);
		path_info->frame[i].blk_ofs_info.blk_y_first_ofs = reg_val;
		
		//UINT32 blk_y_middle_ofs; //0x134 SLICEBUF0_BLOCK_Y_OFS1
		reg_ofs = PL0_SLICE_BUFFER0_REGISTER1_OFS + (0x10*i) + (0x300*path_id);
		reg_val = JMISP_ENG_GETREG(p_eng->reg_io_base + reg_ofs);
		path_info->frame[i].blk_ofs_info.blk_y_middle_ofs = reg_val;
	}
	
	for (i = 0; i < JMISP_ENG_MAX_PINGPONG_NUM; i++) {
		//pp_addr
		reg_ofs = PL0_PING_PONG_BUFFER_ADDRESS_REGISTER0_OFS + (0x10*i) + (0x300*path_id);
		reg_val = JMISP_ENG_GETREG(p_eng->reg_io_base + reg_ofs);
		path_info->pingpong[i].pingpong_addr0 = reg_val;

		reg_ofs = PL0_PING_PONG_BUFFER_ADDRESS_REGISTER1_OFS + (0x10*i) + (0x300*path_id);
		reg_val = JMISP_ENG_GETREG(p_eng->reg_io_base + reg_ofs);
		path_info->pingpong[i].pingpong_addr0 = path_info->pingpong[i].pingpong_addr0 | ((UINT64)reg_val << 32);
		
		reg_ofs = PL0_PING_PONG_BUFFER_ADDRESS_REGISTER2_OFS + (0x10*i) + (0x300*path_id);
		reg_val = JMISP_ENG_GETREG(p_eng->reg_io_base + reg_ofs);
		path_info->pingpong[i].pingpong_addr1 = reg_val;

		reg_ofs = PL0_PING_PONG_BUFFER_ADDRESS_REGISTER3_OFS + (0x10*i) + (0x300*path_id);
		reg_val = JMISP_ENG_GETREG(p_eng->reg_io_base + reg_ofs);
		path_info->pingpong[i].pingpong_addr1 = path_info->pingpong[i].pingpong_addr1 | ((UINT64)reg_val << 32);
		
		//JMISP_ENG_BLOCK_OFS_INFO blk_ofs_info;
		//UINT32 blk_x_first_ofs;  //0x138 SLICEBUF0_BLOCK_X_OFS0
		//UINT32 blk_x_middle_ofs; //0x138 SLICEBUF0_BLOCK_X_OFS1
		{
			T_PL0_SLICE_PINGPONG_BUFFER0_REGISTER2 LocalReg = {0};
			reg_ofs = PL0_SLICE_PINGPONG_BUFFER0_REGISTER2_OFS + (0x10*i)  + (0x300*path_id);
			LocalReg.reg = JMISP_ENG_GETREG(p_eng->reg_io_base + reg_ofs);
			path_info->pingpong[i].blk_ofs_info.blk_x_first_ofs  = LocalReg.bit.PL0_PINGPONG0_BLOCK_X_OFS0;
			path_info->pingpong[i].blk_ofs_info.blk_x_middle_ofs = LocalReg.bit.PL0_PINGPONG0_BLOCK_X_OFS1;
		}
		
		
		//UINT32 blk_y_first_ofs;  //0x130 SLICEBUF0_BLOCK_Y_OFS0
		reg_ofs = PL0_SLICE_PINGPONG_BUFFER0_REGISTER0_OFS + (0x10*i) + (0x300*path_id);
		reg_val = JMISP_ENG_GETREG(p_eng->reg_io_base + reg_ofs);
		path_info->pingpong[i].blk_ofs_info.blk_y_first_ofs = reg_val;

		//UINT32 blk_y_middle_ofs; //0x134 SLICEBUF0_BLOCK_Y_OFS1
		reg_ofs = PL0_SLICE_PINGPONG_BUFFER0_REGISTER1_OFS + (0x10*i) + (0x300*path_id);
		reg_val = JMISP_ENG_GETREG(p_eng->reg_io_base + reg_ofs);
		path_info->pingpong[i].blk_ofs_info.blk_y_middle_ofs = reg_val;
	}
	
	// cust parameter (0x3c0~0x3c4)
	reg_ofs = PL0_CUST_VALUE_REGISTER0_OFS + (0x300*path_id);
	path_info->cust[0] = JMISP_ENG_GETREG(p_eng->reg_io_base + reg_ofs);
	reg_ofs = PL0_CUST_VALUE_REGISTER1_OFS + (0x300*path_id);
	path_info->cust[1] = JMISP_ENG_GETREG(p_eng->reg_io_base + reg_ofs);
	
	// ppu pipe buffer address parameter (0x3c8~0x3cc)
	reg_ofs = PL0_CUST_VALUE_REGISTER2_OFS + (0x300*path_id);
	path_info->ppu_pipebuf_lsbaddr[0] = JMISP_ENG_GETREG(p_eng->reg_io_base + reg_ofs);
	reg_ofs = PL0_CUST_VALUE_REGISTER3_OFS + (0x300*path_id);
	path_info->ppu_pipebuf_lsbaddr[1] = JMISP_ENG_GETREG(p_eng->reg_io_base + reg_ofs);

	return 0;
}

INT32 jmisp_eng_run_cpu_slice(JMISP_ENG_HANDLE *p_eng, UINT32 path_id)
{
	UINT32 slice_en  = 0;
	UINT32 slice_int = 0;
	UINT32 cnt = 0;
	
	if (p_eng == NULL) {
		return -1;
	}
	
	if (path_id > 3) {
		return -1;
	}
	
	slice_en = 1 << (24 + path_id);
	slice_int = 1 << (8 + path_id);
	
	// clear interrupt status
	JMISP_ENG_SETREG(p_eng->reg_io_base + JMISP_INTERRUPT_STATUS_REGISTER_OFS, slice_int);
	
	// trigger slice
	JMISP_ENG_SETREG(p_eng->reg_io_base + JMISP_CONTROL_REGISTER_OFS, slice_en);
	
	if (0) {
		cnt = 0;
		while ((JMISP_ENG_GETREG(p_eng->reg_io_base + JMISP_INTERRUPT_STATUS_REGISTER_OFS) & slice_int) != slice_int) {
			cnt++;
			if (cnt > 10000) {
				DBG_ERR("waiting slice done fail... 0x%08X\n", JMISP_ENG_GETREG(p_eng->reg_io_base + JMISP_INTERRUPT_STATUS_REGISTER_OFS));
				return -1;
			}
		}
	}
	return 0;
}

INT32 jmisp_eng_reset(JMISP_ENG_HANDLE *p_eng)
{
    if(p_eng == NULL) {
        DBG_ERR("JMISP handle null\r\n");
        return -1;
    }

    // DMA disable
    jmisp_eng_dma_channel_enable_hw_reg(p_eng, FALSE);

    // wait DMA idle
    if(jmisp_eng_platform_dma_idle(p_eng)) {
        DBG_ERR("JMISP wait DMA idle fail\n\r");
		return -1;
    } else {
        // SW reset
        jmisp_eng_int_soft_reset_hw_reg(p_eng);

        // DMA enable
        jmisp_eng_dma_channel_enable_hw_reg(p_eng, TRUE);

        // clear interrupt status
        jmisp_eng_clr_intr_status(p_eng, JMISP_ENG_INTERRUPT_ALL);
    }

    return 0;
}

UINT32 jmisp_eng_get_joblist_cycle(JMISP_ENG_HANDLE *p_eng, UINT32 pl_id)
{
    if (p_eng == NULL) {
		DBG_ERR("JMISP handle null\r\n");
		return 0;
	}
	
	return JMISP_ENG_GETREG(p_eng->reg_io_base + PL0_CYCLE_COUNT_REGISTER0_OFS + pl_id*0x30);
}

UINT32 jmisp_eng_get_wait_dma_cycle(JMISP_ENG_HANDLE *p_eng, UINT32 pl_id)
{
    if (p_eng == NULL) {
		DBG_ERR("JMISP handle null\r\n");
		return 0;
	}
	
	return JMISP_ENG_GETREG(p_eng->reg_io_base + PL0_CYCLE_COUNT_REGISTER1_OFS + pl_id*0x30);
}

UINT32 jmisp_eng_get_wait_unit_cycle(JMISP_ENG_HANDLE *p_eng, UINT32 pl_id)
{
    if (p_eng == NULL) {
		DBG_ERR("JMISP handle null\r\n");
		return 0;
	}
	
	return JMISP_ENG_GETREG(p_eng->reg_io_base + PL0_CYCLE_COUNT_REGISTER2_OFS + pl_id*0x30);
}
/*
UINT32 jmisp_eng_get_dram_bw(JMISP_ENG_HANDLE *p_eng, UINT32 pl_id)
{
    if (p_eng == NULL) {
		DBG_ERR("JMISP handle null\r\n");
		return 0;
	}
	
	return JMISP_ENG_GETREG(p_eng->reg_io_base + 0xA0 + pl_id*0x10);
}

UINT32 jmisp_eng_get_ub_bw(JMISP_ENG_HANDLE *p_eng, UINT32 pl_id)
{
    if (p_eng == NULL) {
		DBG_ERR("JMISP handle null\r\n");
		return 0;
	}
	
	return JMISP_ENG_GETREG(p_eng->reg_io_base + 0xA4 + pl_id*0x10);
}
*/
UINT32 jmisp_eng_get_arb_status(JMISP_ENG_HANDLE *p_eng)
{
	if (p_eng == NULL) {
		DBG_ERR("JMISP handle null\r\n");
		return 0;
	}
		
	return (JMISP_ENG_GETREG(p_eng->reg_io_base + JMISP_SUB_MODE_REGISTER2_OFS) >> 16) & 0xFF;
}

UINT32 jmisp_eng_get_eng_arb_info(JMISP_ENG_HANDLE *p_eng, UINT32 eng, UINT32 *pl_idx, UINT32 *dispatch_sel, UINT32 *net_id, UINT64 *job_addr)
{
	UINT64 addr = 0;
	UINT64 addr_msb = 0;
	
	if (p_eng == NULL) {
		DBG_ERR("JMISP handle null\r\n");
		return 0;
	}
	
	if (eng == 9) {
        T_JMISP_PPU_ARBITER_REGISTER0 LocalReg = {0};
		LocalReg.reg  = JMISP_ENG_GETREG(p_eng->reg_io_base + JMISP_PPU_ARBITER_REGISTER0_OFS);
		*pl_idx       = LocalReg.bit.PPU_ARB_FUNC_INDEX;
		*dispatch_sel = LocalReg.bit.PPU_ARB_DISPATCH_SEL;
		*net_id       = LocalReg.bit.PPU_ARB_NET_ID;
		addr          = JMISP_ENG_GETREG(p_eng->reg_io_base + JMISP_PPU_ARBITER_REGISTER2_OFS);
		addr_msb      = JMISP_ENG_GETREG(p_eng->reg_io_base + JMISP_PPU_ARBITER_REGISTER3_OFS);
		addr          = addr | (addr_msb << 32);	
		*job_addr     = addr;
	} else if (eng == 27) {
        T_JMISP_POU_ARBITER_REGISTER0 LocalReg = {0};
		LocalReg.reg  = JMISP_ENG_GETREG(p_eng->reg_io_base + JMISP_POU_ARBITER_REGISTER0_OFS);
		*pl_idx       = LocalReg.bit.POU_ARB_FUNC_INDEX;
		*dispatch_sel = LocalReg.bit.POU_ARB_DISPATCH_SEL;
		*net_id       = LocalReg.bit.POU_ARB_NET_ID;
		addr          = JMISP_ENG_GETREG(p_eng->reg_io_base + JMISP_POU_ARBITER_REGISTER2_OFS);
		addr_msb      = JMISP_ENG_GETREG(p_eng->reg_io_base + JMISP_POU_ARBITER_REGISTER3_OFS);
		addr          = addr | (addr_msb << 32);	
		*job_addr     = addr;
	}
	
	return 0;
}

BOOL jmisp_ssdrv_proc_ver(void)
{
    DBG_DUMP("%s Version: %s %s %s\n", JMISP_SSD_DRV_NAME, JMISP_SSD_DRV_MODULE_VERSION, __DATE__, __TIME__);
    return TRUE;
}

UINT32 jmisp_eng_get_clk_rate(JMISP_ENG_HANDLE *p_eng)
{
	if (p_eng == NULL) {
		DBG_ERR("JMISP handle null\r\n");
		return 0;
	}

	return 240000000;
}
