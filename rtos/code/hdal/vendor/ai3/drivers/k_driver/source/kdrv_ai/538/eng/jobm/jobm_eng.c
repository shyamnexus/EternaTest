/*
    JOBM module driver

    NT98538 JOBM module driver.

    @file       jobm_eng.c
    @ingjobmp    mIIPPJOBM
    @note       None

    Copyright   Novatek Microelectronics Corp. 2019.  All rights reserved.
*/
#include "kwrap/util.h"
#include "jobm_eng_int_reg.h"
#include "jobm_eng.h"
#include "jobm_eng_handle.h"
#include "jobm_eng_int_platform.h"
#include "jobm_eng_int_dbg.h"

static JOBM_ENG_CTL eng_ctl;
#define JOBM_ENG_ID_TO_CH(chip_id, eng_id) ((chip_id * eng_ctl.eng_num) + eng_id)

/**
    Get register buffer size
    Get register buffer size

    @param[in] eng_id    :ive engine id param(IVE_ID Only)

    @return Register buffer size (unit: Byte), if error eng handle, return 0;
*/
UINT32 jobm_eng_get_reg_base_buf_size(UINT32 eng_id)
{
	if (eng_id >= JOBM_ID_MAX_NUM) {
		DBG_ERR("error jobm engine id\r\n");
		return 0;
	}
	return (JOBM_ENG_REG_NUMS << 2);
}

/**
    Get flg buffer size
    Get flg buffer size

    @param[in] eng_id    :ive engine id param(IVE_ID Only)

    @return Flg buffer size (unit: Byte), if error eng handle, return 0;
*/
UINT32 jobm_eng_get_reg_flag_buf_size(UINT32 eng_id)
{
	if (eng_id >= JOBM_ID_MAX_NUM) {
		DBG_ERR("error jobm engine id\r\n");
		return 0;
	}
	return (JOBM_ENG_REG_NUMS);
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
VOID jobm_eng_set_reg_buf(JOBM_ENG_HANDLE *p_eng, uintptr_t reg_base_addr, uintptr_t reg_flag_addr)
{
    p_eng->p_jobm_reg_st = (NT98538_JOBM_REGISTER_STRUCT *)reg_base_addr;
	p_eng->p_jobm_reg_chg_flag = (UINT8 *)reg_flag_addr;
}

/**
    jobm software reset
    jobm software reset

    @param[in] p_eng    :Ive engine handle param

    @return status
        - @ -1   : Fail
        - @  0   : Pass
*/
static INT32 jobm_eng_int_soft_reset_hw_reg(JOBM_ENG_HANDLE *p_eng)
{
	T_JOBM_CONTROL_REGISTER ctl;

	if (p_eng == NULL) {
		DBG_ERR("jobm handle parameter null\r\n");
		return -1;
	}

	ctl.reg = 0;
	ctl.bit.JOBM_SW_RST = 1;
	JOBM_ENG_SETREG(p_eng->reg_io_base + JOBM_CONTROL_REGISTER_OFS, ctl.reg);

	ctl.bit.JOBM_SW_RST = 0;
	JOBM_ENG_SETREG(p_eng->reg_io_base + JOBM_CONTROL_REGISTER_OFS, ctl.reg);

    return 0;
}

/**
    Create jobm ctrl param (memory)
    Create jobm ctrl param (memory)

    @param[in] p_eng_ctl    :jobm engine ctrl param

    @return status
        - @ -1  : Fail
        - @  0  : Pass
*/
INT32 jobm_eng_init(UINT32 chip_num, UINT32 eng_num)
{
	UINT32 total_ch;

	total_ch = (chip_num * eng_num);
	if (total_ch == 0) {
		DBG_ERR("jobm input parameter fail %d\r\n", total_ch);
		return -1;
	}

	if (eng_num > JOBM_ID_MAX_NUM) {
		DBG_ERR("error jobm engine num\r\n");
		return -1;
	}

	eng_ctl.p_eng = JOBM_ENG_MALLOC(sizeof(JOBM_ENG_HANDLE) * total_ch);
	if (eng_ctl.p_eng == NULL) {
		DBG_ERR("alloc buf(%ld) failed\r\n", sizeof(JOBM_ENG_HANDLE) * (int)total_ch);
		return -1;
	}

	eng_ctl.chip_num = chip_num;
	eng_ctl.eng_num = eng_num;
	eng_ctl.total_ch= total_ch;

	return 0;
}

INT32 jobm_eng_release(void)
{
	/*UINT32 i;
	
	if (eng_ctl.p_eng) {
		for (i = 0; i < eng_ctl.total_ch; i ++) {
			jobm_eng_platform_release_irq(&eng_ctl.p_eng[i]);
			jobm_eng_platform_unprepare_clk(&eng_ctl.p_eng[i]);
		}
	}*/

	JOBM_ENG_FREE(eng_ctl.p_eng);
	memset((void *)&eng_ctl, 0, sizeof(JOBM_ENG_CTL));

	return 0;
}

/**
    Create jobm resource (flg/sem/preclk/irq)
    Create jobm resource (flg/sem/preclk/irq)

    @param[in] p_eng    :jobm engine handle param

    @return status
        - @ -1  :Fail
        - @  0  :Pass
*/
INT32 jobm_eng_init_resource(JOBM_ENG_HANDLE *p_eng)
{
	JOBM_ENG_HANDLE *p_eng_hdl;
    JOBM_ENG_HANDLE eng_hdl_back;

	p_eng_hdl = jobm_eng_get_handle(p_eng->chip_id, p_eng->eng_id);
	if (p_eng_hdl == NULL) {
		DBG_ERR("ive handle null !!\r\n");
        return -1;
	}
	if (p_eng->eng_id >= JOBM_ID_MAX_NUM) 
    {
        DBG_ERR("error ive engine id\r\n");
        return -1;
    }

	eng_hdl_back.flg_id_jobm = p_eng_hdl->flg_id_jobm;
	*p_eng_hdl = *p_eng;
	p_eng_hdl->flg_id_jobm = eng_hdl_back.flg_id_jobm;

	//jobm_eng_platform_set_clk_rate(p_eng_hdl);
	//jobm_eng_platform_prepare_clk(p_eng_hdl);
	jobm_eng_platform_request_irq(p_eng_hdl);

	return 0;
}

JOBM_ENG_HANDLE* jobm_eng_get_handle(UINT32 chip_id, UINT32 eng_id)
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

void jobm_eng_reg_isr_callback(JOBM_ENG_HANDLE *p_eng, JOBM_ISR_CB cb)
{
	if (p_eng == NULL) {
		return;
	}
	p_eng->isr_cb = cb;
}

INT32 jobm_eng_open(JOBM_ENG_HANDLE *p_eng)
{
	if (p_eng == NULL) {
		DBG_ERR(" jobm handle null\r\n");
        return -1;
	}
	if (p_eng->eng_id >= JOBM_ID_MAX_NUM) {
		DBG_ERR("error ive engine id (%d)\r\n", p_eng->eng_id);
		return -1;
	}
	jobm_eng_platform_create_resource(p_eng);

	// clear interrupt enable & status 
    JOBM_ENG_SETREG(p_eng->reg_io_base + JOBM_INTERRUPT_ENABLE_REGISTER_OFS, 0);
	JOBM_ENG_SETREG(p_eng->reg_io_base + JOBM_INTERRUPT_STATUS_REGISTER_OFS, JOBM_ENG_INTERRUPT_ALL);

	// software reset 
	jobm_eng_int_soft_reset_hw_reg(p_eng);

	// set axi enable at here 
	jobm_eng_dma_channel_enable_hw_reg(p_eng, TRUE);
	jobm_eng_axi_channel_enable_hw_reg(p_eng, TRUE);
	
	// enable cycle count
	{
		T_MISC_REGISTER0 ctl;
		ctl.reg = JOBM_ENG_GETREG(p_eng->reg_io_base + MISC_REGISTER0_OFS); 
		ctl.bit.CYCLE_COUNT_ENABLE = 1;
		JOBM_ENG_SETREG(p_eng->reg_io_base + MISC_REGISTER0_OFS, ctl.reg);	
	}
	
	return 0;
}

INT32 jobm_eng_close(JOBM_ENG_HANDLE *p_eng)
{
	if (p_eng == NULL) {
		return -1;
	}
	if (p_eng->eng_id >= JOBM_ID_MAX_NUM) {
		DBG_ERR("error ive engine id\r\n");
		return -1;
	}

	jobm_eng_platform_release_resource(p_eng);

	return 0;
}

VOID jobm_eng_dma_channel_enable_hw_reg(JOBM_ENG_HANDLE *p_eng, BOOL set_en)
{
	T_MISC_REGISTER0 ctl;

	if (p_eng == NULL) {
		return;
	}

	ctl.reg = JOBM_ENG_GETREG(p_eng->reg_io_base + MISC_REGISTER0_OFS);
    if (set_en) {
	    ctl.bit.JLA_CH_DISABLE = 0;
		ctl.bit.JLB_CH_DISABLE = 0;
		ctl.bit.JLC_CH_DISABLE = 0;
		ctl.bit.JLD_CH_DISABLE = 0;
		ctl.bit.JLE_CH_DISABLE = 0;
    } else {
	    ctl.bit.JLA_CH_DISABLE = 1;
		ctl.bit.JLB_CH_DISABLE = 1;
		ctl.bit.JLC_CH_DISABLE = 1;
		ctl.bit.JLD_CH_DISABLE = 1;
		ctl.bit.JLE_CH_DISABLE = 1;
	}
	JOBM_ENG_SETREG(p_eng->reg_io_base + MISC_REGISTER0_OFS, ctl.reg);
}

BOOL jobm_eng_chk_dma_channel_idle_reg(JOBM_ENG_HANDLE *p_eng)
{
	T_MISC_REGISTER1 ctl;

	if (p_eng == NULL) {
		return FALSE;
	}

	ctl.reg = JOBM_ENG_GETREG(p_eng->reg_io_base + MISC_REGISTER1_OFS);
    if (ctl.bit.JLA_CH_IDLE & ctl.bit.JLB_CH_IDLE & ctl.bit.JLC_CH_IDLE & ctl.bit.JLD_CH_IDLE & ctl.bit.JLE_CH_IDLE)
        return TRUE;

    return FALSE;
}

VOID jobm_eng_axi_channel_enable_hw_reg(JOBM_ENG_HANDLE *p_eng, BOOL set_en)
{
	T_MISC_REGISTER0 ctl;

	if (p_eng == NULL) {
		return;
	}

	ctl.reg = JOBM_ENG_GETREG(p_eng->reg_io_base + MISC_REGISTER0_OFS);
    if (set_en) {
	    ctl.bit.AXI_BUS_DISABLE = 0;
    } else {
	    ctl.bit.AXI_BUS_DISABLE = 1;
	}
	JOBM_ENG_SETREG(p_eng->reg_io_base + MISC_REGISTER0_OFS, ctl.reg);
}

BOOL jobm_eng_chk_axi_channel_idle_reg(JOBM_ENG_HANDLE *p_eng)
{
	T_MISC_REGISTER1 ctl;

	if (p_eng == NULL) {
		return FALSE;
	}

	ctl.reg = JOBM_ENG_GETREG(p_eng->reg_io_base + MISC_REGISTER1_OFS);
    if (ctl.bit.AXI_BUS_IDLE)
        return TRUE;

    return FALSE;
}

void jobm_eng_trig_jl_hw_reg(JOBM_ENG_HANDLE *p_eng, uintptr_t jl_addr_msb, uintptr_t jl_addr_lsb, UINT32 jl_id)
{
    T_DMA_TO_JLA_CHANNEL_REGISTER0 jl_dma_lsb;
    T_DMA_TO_JLA_CHANNEL_REGISTER1 jl_dma_msb;
	T_JOBM_CONTROL_REGISTER ctl;

	if (p_eng == NULL) {
		return;
	}
    
	jl_dma_lsb.reg = jl_addr_lsb;
    jl_dma_msb.reg = jl_addr_msb;

	JOBM_ENG_SETREG(p_eng->reg_io_base + DMA_TO_JLA_CHANNEL_REGISTER0_OFS + 0x90*jl_id, jl_dma_lsb.reg);
    JOBM_ENG_SETREG(p_eng->reg_io_base + DMA_TO_JLA_CHANNEL_REGISTER1_OFS + 0x90*jl_id, jl_dma_msb.reg);

	ctl.reg = JOBM_ENG_GETREG(p_eng->reg_io_base + JOBM_CONTROL_REGISTER_OFS);
	if (jl_id == 0) {
		ctl.reg = ctl.reg & 0x1E0; // keep other jl terminate status
		ctl.bit.JLA_START = 1;
	} else if (jl_id == 1) {
		ctl.reg = ctl.reg & 0x1D0;
		ctl.bit.JLB_START = 1;
	} else if (jl_id == 2) {
		ctl.reg = ctl.reg & 0x1B0;
		ctl.bit.JLC_START = 1;
	} else if (jl_id == 3) {
		ctl.reg = ctl.reg & 0x170;
		ctl.bit.JLD_START = 1;
	} else if (jl_id == 4) {
		ctl.reg = ctl.reg & 0xF0;
		ctl.bit.JLE_START = 1;
	}
	
	JOBM_ENG_SETREG(p_eng->reg_io_base + JOBM_CONTROL_REGISTER_OFS, ctl.reg);
}

void jobm_eng_isr_hw_reg(JOBM_ENG_HANDLE *p_eng)
{
	UINT32 status = 0, status_inte;
	UINT32 inte;
	FLGPTN flag = 0x0;
	
	if (p_eng == NULL) {
		DBG_ERR("parameter null\r\n");
		return ;
	}
	// get interrupt status & enable bit 
	status = JOBM_ENG_GETREG(p_eng->reg_io_base + JOBM_INTERRUPT_STATUS_REGISTER_OFS);
    //DBG_ERR("int sts = 0x%x\r\n", status);

	// clear interrupt status 
	inte = JOBM_ENG_GETREG(p_eng->reg_io_base + JOBM_INTERRUPT_ENABLE_REGISTER_OFS);
	status_inte = status & inte;

	if (status != 0) {
		flag = 0x0;
		if (status_inte & JOBM_ENG_INTERRUPT_JLA_END) {
			flag |= FLGPTN_JOBM_JLA_END;
		}
		if (status_inte & JOBM_ENG_INTERRUPT_JLB_END) {
			flag |= FLGPTN_JOBM_JLB_END;
		}
		if (status_inte & JOBM_ENG_INTERRUPT_JLC_END) {
			flag |= FLGPTN_JOBM_JLC_END;
		}
		if (status_inte & JOBM_ENG_INTERRUPT_JLD_END) {
			flag |= FLGPTN_JOBM_JLD_END;
		}
		if (status_inte & JOBM_ENG_INTERRUPT_JLE_END) {
			flag |= FLGPTN_JOBM_JLE_END;
		}

		if (status & JOBM_ENG_INTERRUPT_JLA_ERR) {
			DBG_ERR("%s: JLA error..., please check command\r\n", __func__);
		}
		if (status & JOBM_ENG_INTERRUPT_JLB_ERR) {
			DBG_ERR("%s: JLB error..., please check command\r\n", __func__);
		}
		if (status & JOBM_ENG_INTERRUPT_JLC_ERR) {
			DBG_ERR("%s: JLC error..., please check command\r\n", __func__);
		}
		if (status & JOBM_ENG_INTERRUPT_JLD_ERR) {
			DBG_ERR("%s: JLD error..., please check command\r\n", __func__);
		}
		if (status & JOBM_ENG_INTERRUPT_JLE_ERR) {
			DBG_ERR("%s: JLE error..., please check command\r\n", __func__);
		}

		jobm_eng_platform_flg_set(p_eng, flag);
	}
	JOBM_ENG_SETREG(p_eng->reg_io_base + JOBM_INTERRUPT_STATUS_REGISTER_OFS, status_inte);

	// get interrupt status 
	if (p_eng->isr_cb != NULL) {
		p_eng->isr_cb(p_eng, status_inte, NULL);
	}
}

/**
    Enable interrupt fuction of jobm
    Enable interrupt fuction of jobm

    @param[in]
        -@ p_eng   :Rou handle param
        -@int_en   :Which interrupt function

    @return Void
*/
INT32 jobm_eng_set_intrpt_en(JOBM_ENG_HANDLE *p_eng, UINT32 int_en)
{
	T_JOBM_INTERRUPT_ENABLE_REGISTER LocalReg;
	
	if (p_eng == NULL) {
		return -1;
	}
    if(p_eng->eng_id >=JOBM_ID_MAX_NUM)
    {
		DBG_ERR("error jobm engine id\r\n");
		return -1;
    }
	
	LocalReg.reg = JOBM_ENG_GETREG(p_eng->reg_io_base + JOBM_INTERRUPT_ENABLE_REGISTER_OFS) | int_en;
	JOBM_ENG_SETREG(p_eng->reg_io_base + JOBM_INTERRUPT_STATUS_REGISTER_OFS, int_en);
	JOBM_ENG_SETREG(p_eng->reg_io_base + JOBM_INTERRUPT_ENABLE_REGISTER_OFS, LocalReg.reg);
	// must be used link list buffer
	p_eng->p_jobm_reg_st->JOBM_Register_1.Word = (UINT32)int_en;
	p_eng->p_jobm_reg_chg_flag[1] = TRUE;
    return 0;
}

INT32 jobm_eng_set_intrpt_dis(JOBM_ENG_HANDLE *p_eng, UINT32 int_dis)
{
	T_JOBM_INTERRUPT_ENABLE_REGISTER LocalReg;
	
	if (p_eng == NULL) {
		return -1;
	}
    if(p_eng->eng_id >=JOBM_ID_MAX_NUM)
    {
		DBG_ERR("error jobm engine id\r\n");
		return -1;
    }
	
	LocalReg.reg = JOBM_ENG_GETREG(p_eng->reg_io_base + JOBM_INTERRUPT_ENABLE_REGISTER_OFS) & (~int_dis);
	JOBM_ENG_SETREG(p_eng->reg_io_base + JOBM_INTERRUPT_ENABLE_REGISTER_OFS, LocalReg.reg);

    return 0;
}

VOID jobm_eng_clr_intr_status(JOBM_ENG_HANDLE *p_eng, UINT32 uiIntrStatus)
{
	T_JOBM_INTERRUPT_STATUS_REGISTER LocalReg;
	LocalReg.reg = uiIntrStatus;
	JOBM_ENG_SETREG(p_eng->reg_io_base + JOBM_INTERRUPT_STATUS_REGISTER_OFS, LocalReg.reg);
}

INT32 jobm_eng_set_single_dbg_en(JOBM_ENG_HANDLE *p_eng, UINT32 enable, UINT32 jl_id)
{
	T_JLA_DEBUG_MISC_REGISTER LocalReg;
	
	if (p_eng == NULL) {
		return -1;
	}
    if(p_eng->eng_id >= JOBM_ID_MAX_NUM)
    {
		DBG_ERR("error jobm engine id\r\n");
		return -1;
    }
	if (enable) {
		LocalReg.bit.JLA_DBG_CODE = 0x69;
		LocalReg.bit.JLA_DBG_STEP = 1;
	} else {
		LocalReg.bit.JLA_DBG_CODE = 0x0;
	}
	JOBM_ENG_SETREG(p_eng->reg_io_base + 0x90*jl_id + JLA_DEBUG_MISC_REGISTER_OFS, LocalReg.reg);
	
    return 0;
}

INT32 jobm_eng_run_single_dbg(JOBM_ENG_HANDLE *p_eng, UINT32 jl_id)
{
	T_JLA_DEBUG_MISC_REGISTER LocalReg;
	
	if (p_eng == NULL) {
		return -1;
	}
    if(p_eng->eng_id >= JOBM_ID_MAX_NUM)
    {
		DBG_ERR("error jobm engine id\r\n");
		return -1;
    }
	LocalReg.reg = JOBM_ENG_GETREG(p_eng->reg_io_base + 0x90*jl_id + JLA_DEBUG_MISC_REGISTER_OFS);
	//LocalReg = JOBM_ENG_GETREG(p_eng->reg_io_base + 0x90*jl_id + JLA_DEBUG_MISC_REGISTER_OFS);
	if (LocalReg.bit.JLA_DBG_CODE == 0x69) {
		LocalReg.bit.JLA_DBG_NEXT = 1;
		JOBM_ENG_SETREG(p_eng->reg_io_base + 0x90*jl_id + JLA_DEBUG_MISC_REGISTER_OFS, LocalReg.reg);
	} else {
		return -1;
	}
	
    return 0;
}

INT32 jobm_eng_set_arb_dbg_en(JOBM_ENG_HANDLE *p_eng, UINT32 enable_eng, UINT32 arb_id)
{
	T_JOBM_SUB_MODE_REGISTER2 LocalReg;
	
	if (p_eng == NULL) {
		return -1;
	}
    if(p_eng->eng_id >= JOBM_ID_MAX_NUM)
    {
		DBG_ERR("error jobm engine id\r\n");
		return -1;
    }
	
	LocalReg.reg = JOBM_ENG_GETREG(p_eng->reg_io_base + JOBM_SUB_MODE_REGISTER2_OFS);
	if (arb_id == 0) {
		LocalReg.bit.ARB_DBG_EN_CONV0 = (enable_eng >> 0) & 0x1;
		//LocalReg.bit.ARB_DBG_EN_CONV1 = (enable_eng >> 1) & 0x1;
		//LocalReg.bit.ARB_DBG_EN_CONV2 = (enable_eng >> 2) & 0x1;
		//LocalReg.bit.ARB_DBG_EN_CONV3 = (enable_eng >> 3) & 0x1;
		LocalReg.bit.ARB_DBG_EN_LSU   = (enable_eng >> 4) & 0x1;
		LocalReg.bit.ARB_DBG_EN_UTIL_ROU_CAL  = (((enable_eng >> 5) & 0x7) > 0) ? 1 : 0;
	} else {
		LocalReg.bit.ARB1_DBG_EN_CONV0 = (enable_eng >> 0) & 0x1;
		//LocalReg.bit.ARB_DBG_EN_CONV1 = (enable_eng >> 1) & 0x1;
		//LocalReg.bit.ARB_DBG_EN_CONV2 = (enable_eng >> 2) & 0x1;
		//LocalReg.bit.ARB_DBG_EN_CONV3 = (enable_eng >> 3) & 0x1;
		LocalReg.bit.ARB1_DBG_EN_LSU   = (enable_eng >> 4) & 0x1;
		LocalReg.bit.ARB1_DBG_EN_UTIL_ROU_CAL  = (((enable_eng >> 5) & 0x7) > 0) ? 1 : 0;
	}
	JOBM_ENG_SETREG(p_eng->reg_io_base + JOBM_SUB_MODE_REGISTER2_OFS, LocalReg.reg);
	
    return 0;
}

INT32 jobm_eng_run_arb_dbg(JOBM_ENG_HANDLE *p_eng, UINT32 enable_eng, UINT32 arb_id)
{
	T_JOBM_SUB_MODE_REGISTER2 dbg_en_reg;
	T_JOBM_SUB_MODE_REGISTER3 LocalReg;
	
	if (p_eng == NULL) {
		return -1;
	}
    if(p_eng->eng_id >= JOBM_ID_MAX_NUM)
    {
		DBG_ERR("error jobm engine id\r\n");
		return -1;
    }
	dbg_en_reg.reg = JOBM_ENG_GETREG(p_eng->reg_io_base + JOBM_SUB_MODE_REGISTER2_OFS);
	//dbg_en_reg = JOBM_ENG_GETREG(p_eng->reg_io_base + JOBM_SUB_MODE_REGISTER2_OFS);
	LocalReg.reg = 0;
	
	if (arb_id == 0) {
		if (dbg_en_reg.bit.ARB_DBG_EN_CONV0) {
			LocalReg.bit.ARB_DBG_NEXT_CONV0 = (enable_eng >> 0) & 0x1;
		}
		if (dbg_en_reg.bit.ARB_DBG_EN_LSU) {
			LocalReg.bit.ARB_DBG_NEXT_LSU   = (enable_eng >> 4) & 0x1;
		}
		if (dbg_en_reg.bit.ARB_DBG_EN_UTIL_ROU_CAL) {
			LocalReg.bit.ARB_DBG_NEXT_UTIL_ROU_CAL  = (((enable_eng >> 5) & 0x7) > 0) ? 1 : 0;
		}
	} else {
		if (dbg_en_reg.bit.ARB_DBG_EN_CONV0) {
			LocalReg.bit.ARB1_DBG_NEXT_CONV0 = (enable_eng >> 0) & 0x1;
		}
		if (dbg_en_reg.bit.ARB_DBG_EN_LSU) {
			LocalReg.bit.ARB1_DBG_NEXT_LSU   = (enable_eng >> 4) & 0x1;
		}
		if (dbg_en_reg.bit.ARB_DBG_EN_UTIL_ROU_CAL) {
			LocalReg.bit.ARB1_DBG_NEXT_UTIL_ROU_CAL  = (((enable_eng >> 5) & 0x7) > 0) ? 1 : 0;
		}
	}
	JOBM_ENG_SETREG(p_eng->reg_io_base + JOBM_SUB_MODE_REGISTER3_OFS, LocalReg.reg);
	
    return 0;
}

INT32 jobm_eng_set_priority(JOBM_ENG_HANDLE *p_eng, UINT32 jl_id, UINT32 no_dispatch_cnt, UINT32 priority)
{
	T_JLA_PRIORITY_REGISTER0 LocalReg;
	
	if (p_eng == NULL) {
		return -1;
	}
    if(p_eng->eng_id >= JOBM_ID_MAX_NUM)
    {
		DBG_ERR("error jobm engine id\r\n");
		return -1;
    }
	
	LocalReg.bit.JLA_NO_DISPATCH_CNT = no_dispatch_cnt & 0x7F;
	LocalReg.bit.JLA_PRIORITY		 = priority & 0x3;

	JOBM_ENG_SETREG(p_eng->reg_io_base + JLA_PRIORITY_REGISTER0_OFS + 0x90*jl_id, LocalReg.reg);
	
	return 0;
}

UINT32 jobm_eng_clr_flg(JOBM_ENG_HANDLE *p_eng, FLGPTN flg)
{
	return jobm_eng_platform_flg_clear(p_eng, flg);
}
UINT32 jobm_eng_wait_flg(JOBM_ENG_HANDLE *p_eng, PFLGPTN p_flgptn, FLGPTN flg)
{
	return jobm_eng_platform_flg_wait(p_eng, p_flgptn, flg);
}
UINT32 jobm_eng_wait_flg_timeout(JOBM_ENG_HANDLE *p_eng, PFLGPTN p_flgptn, FLGPTN flg, int timeout_tick)
{
	return jobm_eng_platform_flg_wait_timeout(p_eng, p_flgptn, flg, timeout_tick);
}

void jobm_eng_wait_framend(JOBM_ENG_HANDLE *p_eng, UINT32 jl_id)
{
    FLGPTN uiflag;
    ER er_return = E_OK;
	
	if (jl_id == 0) {
		er_return = jobm_eng_wait_flg_timeout(p_eng, &uiflag, FLGPTN_JOBM_JLA_END, vos_util_msec_to_tick(10000));
	} else if (jl_id == 1) {
		er_return = jobm_eng_wait_flg_timeout(p_eng, &uiflag, FLGPTN_JOBM_JLB_END, vos_util_msec_to_tick(10000));
	} else if (jl_id == 2) {
		er_return = jobm_eng_wait_flg_timeout(p_eng, &uiflag, FLGPTN_JOBM_JLC_END, vos_util_msec_to_tick(10000));
	} else if (jl_id == 3) {
		er_return = jobm_eng_wait_flg_timeout(p_eng, &uiflag, FLGPTN_JOBM_JLD_END, vos_util_msec_to_tick(10000));
	} else if (jl_id == 4) {
		er_return = jobm_eng_wait_flg_timeout(p_eng, &uiflag, FLGPTN_JOBM_JLE_END, vos_util_msec_to_tick(10000));
	}
    
    if(er_return != E_OK) {
        DBG_ERR("JOBM waitdone error\n\r");
    }
}

INT32 jobm_eng_reset(JOBM_ENG_HANDLE *p_eng)
{
    if(p_eng == NULL) {
        DBG_ERR("JOBM handle null\r\n");
        return -1;
    }

    // DMA disable
    jobm_eng_dma_channel_enable_hw_reg(p_eng, FALSE);
	
    // wait DMA idle
    if(jobm_eng_platform_dma_idle(p_eng)) {
		DBG_ERR("JOBM wait DMA idle fail\n\r");
		return -1;
    } else {
        // SW reset
        jobm_eng_int_soft_reset_hw_reg(p_eng);

        // DMA enable
        jobm_eng_dma_channel_enable_hw_reg(p_eng, TRUE);

        // clear interrupt status
        jobm_eng_clr_intr_status(p_eng, JOBM_ENG_INTERRUPT_ALL);
    }

    return 0;
}

UINT32 jobm_eng_get_joblist_cycle(JOBM_ENG_HANDLE *p_eng, UINT32 jl_id)
{
    if (p_eng == NULL) {
		DBG_ERR("JOBM handle null\r\n");
		return 0;
	}
	
	return JOBM_ENG_GETREG(p_eng->reg_io_base + JLA_CYCLE_COUNT_REGISTER0_OFS + jl_id*0x90);
}

UINT32 jobm_eng_get_wait_dma_cycle(JOBM_ENG_HANDLE *p_eng, UINT32 jl_id)
{
    if (p_eng == NULL) {
		DBG_ERR("JOBM handle null\r\n");
		return 0;
	}
	
	return JOBM_ENG_GETREG(p_eng->reg_io_base + JLA_CYCLE_COUNT_REGISTER1_OFS + jl_id*0x90);
}

UINT32 jobm_eng_get_wait_unit_cycle(JOBM_ENG_HANDLE *p_eng, UINT32 jl_id)
{
    if (p_eng == NULL) {
		DBG_ERR("JOBM handle null\r\n");
		return 0;
	}
	
	return JOBM_ENG_GETREG(p_eng->reg_io_base + JLA_CYCLE_COUNT_REGISTER2_OFS + jl_id*0x90);
}

UINT32 jobm_eng_get_dram_bw(JOBM_ENG_HANDLE *p_eng, UINT32 jl_id)
{
    if (p_eng == NULL) {
		DBG_ERR("JOBM handle null\r\n");
		return 0;
	}
	
	return JOBM_ENG_GETREG(p_eng->reg_io_base + 0xA0 + jl_id*0x10);
}

UINT32 jobm_eng_get_ub_bw(JOBM_ENG_HANDLE *p_eng, UINT32 jl_id)
{
    if (p_eng == NULL) {
		DBG_ERR("JOBM handle null\r\n");
		return 0;
	}
	
	return JOBM_ENG_GETREG(p_eng->reg_io_base + 0xA4 + jl_id*0x10);
}

UINT32 jobm_eng_set_cnt_clr(JOBM_ENG_HANDLE *p_eng, UINT32 clr_eng)
{
    T_BUSY_DEBUG_REGISTER0 LocalReg = {0};
	
	if (p_eng == NULL) {
		DBG_ERR("JOBM handle null\r\n");
		return 0;
	}
	
	LocalReg.bit.CONV0_BUSY_COUNT_CLR = (clr_eng >> 0) & 0x1;
	//LocalReg.bit.CONV1_BUSY_COUNT_CLR = 1;
	//LocalReg.bit.CONV2_BUSY_COUNT_CLR = 1;
	//LocalReg.bit.CONV3_BUSY_COUNT_CLR = 1;
	LocalReg.bit.LSU_BUSY_COUNT_CLR   = (clr_eng >> 4) & 0x1;
	LocalReg.bit.UTIL_ROU_CAL_BUSY_COUNT_CLR  = (((clr_eng >> 5) & 0x7) > 0) ? 1 : 0;
	//LocalReg.bit.ROU_BUSY_COUNT_CLR   = 1;
	//LocalReg.bit.CAL_BUSY_COUNT_CLR   = 1;
	
	JOBM_ENG_SETREG(p_eng->reg_io_base + BUSY_DEBUG_REGISTER0_OFS, LocalReg.reg);
	JOBM_ENG_SETREG(p_eng->reg_io_base + BUSY_DEBUG_REGISTER0_OFS + 0x300, LocalReg.reg);
	return 0;
}

UINT32 jobm_eng_get_conv0_busy_cnt(JOBM_ENG_HANDLE *p_eng, UINT32 arb_id)
{
	if (p_eng == NULL) {
		DBG_ERR("JOBM handle null\r\n");
		return 0;
	}
		
	return JOBM_ENG_GETREG(p_eng->reg_io_base + BUSY_DEBUG_REGISTER1_OFS + arb_id*0x300);
}

UINT32 jobm_eng_get_conv1_busy_cnt(JOBM_ENG_HANDLE *p_eng, UINT32 arb_id)
{
	if (p_eng == NULL) {
		DBG_ERR("JOBM handle null\r\n");
		return 0;
	}
	
	DBG_ERR("get conv1 busy cnt not support in current IC\r\n");
	return 0;
}

UINT32 jobm_eng_get_conv2_busy_cnt(JOBM_ENG_HANDLE *p_eng, UINT32 arb_id)
{
	if (p_eng == NULL) {
		DBG_ERR("JOBM handle null\r\n");
		return 0;
	}
		
	DBG_ERR("get conv2 busy cnt not support in current IC\r\n");
	return 0;
}

UINT32 jobm_eng_get_conv3_busy_cnt(JOBM_ENG_HANDLE *p_eng, UINT32 arb_id)
{
	if (p_eng == NULL) {
		DBG_ERR("JOBM handle null\r\n");
		return 0;
	}
		
	DBG_ERR("get conv3 busy cnt not support in current IC\r\n");
	return 0;
}

UINT32 jobm_eng_get_lsu_busy_cnt(JOBM_ENG_HANDLE *p_eng)
{
	if (p_eng == NULL) {
		DBG_ERR("JOBM handle null\r\n");
		return 0;
	}
		
	return JOBM_ENG_GETREG(p_eng->reg_io_base + BUSY_DEBUG_REGISTER5_OFS);
}

UINT32 jobm_eng_get_util_busy_cnt(JOBM_ENG_HANDLE *p_eng, UINT32 arb_id)
{
	if (p_eng == NULL) {
		DBG_ERR("JOBM handle null\r\n");
		return 0;
	}
		
	DBG_ERR("get util busy cnt not support in current IC\r\n");
	return 0;
}

UINT32 jobm_eng_get_rou_busy_cnt(JOBM_ENG_HANDLE *p_eng, UINT32 arb_id)
{
	if (p_eng == NULL) {
		DBG_ERR("JOBM handle null\r\n");
		return 0;
	}
		
	DBG_ERR("get util busy cnt not support in current IC\r\n");
	return 0;
}

UINT32 jobm_eng_get_cal_busy_cnt(JOBM_ENG_HANDLE *p_eng, UINT32 arb_id)
{
	if (p_eng == NULL) {
		DBG_ERR("JOBM handle null\r\n");
		return 0;
	}
		
	DBG_ERR("get util busy cnt not support in current IC\r\n");
	return 0;
}

UINT32 jobm_eng_get_util_rou_cal_busy_cnt(JOBM_ENG_HANDLE *p_eng, UINT32 arb_id)
{
	if (p_eng == NULL) {
		DBG_ERR("JOBM handle null\r\n");
		return 0;
	}
		
	return JOBM_ENG_GETREG(p_eng->reg_io_base + BUSY_DEBUG_REGISTER6_OFS + arb_id*0x300);
}

INT32 jobm_eng_set_unlock_idle_cycle(JOBM_ENG_HANDLE *p_eng, UINT32 cycle, UINT32 jl_id)
{
	T_JOBM_IDLE_STATUS_REGISTER0 LocalReg = {0};
	
	if (p_eng == NULL) {
		DBG_ERR("JOBM handle null\r\n");
		return -1;
	}
	
	LocalReg.bit.JLA_AUTO_UNLOCK_IDLE_CYCLE = cycle;
	JOBM_ENG_SETREG(p_eng->reg_io_base + JOBM_IDLE_STATUS_REGISTER0_OFS + jl_id*0x4, LocalReg.reg);
	
	return 0;
}

INT32 jobm_eng_set_preload(JOBM_ENG_HANDLE *p_eng, UINT32 enable)
{
	T_CONV0_ARBITER_REGISTER1 LocalReg = {0};
	
	if (p_eng == NULL) {
		DBG_ERR("JOBM handle null\r\n");
		return -1;
	}

	LocalReg.bit.CONV0_PRELOAD_JOB = enable;
	JOBM_ENG_SETREG(p_eng->reg_io_base + CONV0_ARBITER_REGISTER1_OFS, LocalReg.reg);
	JOBM_ENG_SETREG(p_eng->reg_io_base + LSU_ARBITER_REGISTER1_OFS, LocalReg.reg);
	JOBM_ENG_SETREG(p_eng->reg_io_base + UTIL_ROU_CAL_ARBITER_REGISTER1_OFS, LocalReg.reg);
	
	return 0;
}

INT32 jobm_eng_set_tcm_measure_enable(JOBM_ENG_HANDLE *p_eng, UINT32 enable)
{
	T_TCM_REGISTER0 LocalReg = {0};
	
	if (p_eng == NULL) {
		DBG_ERR("JOBM handle null\r\n");
		return -1;
	}
	
	LocalReg.bit.TCM_MEASURE_EN = (enable == 0)?0:0xF;
	JOBM_ENG_SETREG(p_eng->reg_io_base + TCM_REGISTER0_OFS, LocalReg.reg);
	
	return 0;
}

INT32 jobm_eng_set_tcm_measure_time(JOBM_ENG_HANDLE *p_eng, UINT32 cycle)
{
	T_TCM_REGISTER1 LocalReg = {0};
	
	if (p_eng == NULL) {
		DBG_ERR("JOBM handle null\r\n");
		return -1;
	}
	
	LocalReg.bit.TCM_MEASURE_TIME = cycle;
	JOBM_ENG_SETREG(p_eng->reg_io_base + TCM_REGISTER1_OFS, LocalReg.reg);
	
	return 0;
}

UINT32 jobm_eng_get_tcm0_write_count(JOBM_ENG_HANDLE *p_eng)
{
	if (p_eng == NULL) {
		DBG_ERR("JOBM handle null\r\n");
		return -1;
	}
	
	return JOBM_ENG_GETREG(p_eng->reg_io_base + TCM_REGISTER2_OFS);
}

UINT32 jobm_eng_get_tcm1_write_count(JOBM_ENG_HANDLE *p_eng)
{
	if (p_eng == NULL) {
		DBG_ERR("JOBM handle null\r\n");
		return -1;
	}
	
	return JOBM_ENG_GETREG(p_eng->reg_io_base + TCM_REGISTER3_OFS);
}

UINT32 jobm_eng_get_tcm2_write_count(JOBM_ENG_HANDLE *p_eng)
{
	if (p_eng == NULL) {
		DBG_ERR("JOBM handle null\r\n");
		return -1;
	}
	
	return JOBM_ENG_GETREG(p_eng->reg_io_base + TCM_REGISTER4_OFS);
}

UINT32 jobm_eng_get_tcm3_write_count(JOBM_ENG_HANDLE *p_eng)
{
	if (p_eng == NULL) {
		DBG_ERR("JOBM handle null\r\n");
		return -1;
	}
	
	return JOBM_ENG_GETREG(p_eng->reg_io_base + TCM_REGISTER5_OFS);
}

UINT32 jobm_eng_get_tcm0_read_count(JOBM_ENG_HANDLE *p_eng)
{
	if (p_eng == NULL) {
		DBG_ERR("JOBM handle null\r\n");
		return -1;
	}
	
	return JOBM_ENG_GETREG(p_eng->reg_io_base + TCM_REGISTER6_OFS);
}

UINT32 jobm_eng_get_tcm1_read_count(JOBM_ENG_HANDLE *p_eng)
{
	if (p_eng == NULL) {
		DBG_ERR("JOBM handle null\r\n");
		return -1;
	}
	
	return JOBM_ENG_GETREG(p_eng->reg_io_base + TCM_REGISTER7_OFS);
}

UINT32 jobm_eng_get_tcm2_read_count(JOBM_ENG_HANDLE *p_eng)
{
	if (p_eng == NULL) {
		DBG_ERR("JOBM handle null\r\n");
		return -1;
	}
	
	return JOBM_ENG_GETREG(p_eng->reg_io_base + TCM_REGISTER8_OFS);
}

UINT32 jobm_eng_get_tcm3_read_count(JOBM_ENG_HANDLE *p_eng)
{
	if (p_eng == NULL) {
		DBG_ERR("JOBM handle null\r\n");
		return -1;
	}
	
	return JOBM_ENG_GETREG(p_eng->reg_io_base + TCM_REGISTER9_OFS);
}

UINT32 jobm_eng_get_arb_status(JOBM_ENG_HANDLE *p_eng, UINT32 arb_id)
{
	UINT32 arb_status = 0;
	UINT32 nue_unit_sel = 0;
	
	if (p_eng == NULL) {
		DBG_ERR("JOBM handle null\r\n");
		return 0;
	}
	
	arb_status = (JOBM_ENG_GETREG(p_eng->reg_io_base + JOBM_SUB_MODE_REGISTER2_OFS) >> (16 + 8*arb_id)) & 0xFF;
	if ((arb_status >> 5) > 0) {
		nue_unit_sel = (JOBM_ENG_GETREG(p_eng->reg_io_base + UTIL_ROU_CAL_ARBITER_REGISTER0_OFS + arb_id*0x300) >> 20) & 0xF;
		if (nue_unit_sel == 4) {
			arb_status = (arb_status & 0x1F) | (1 << 5);
		} else if (nue_unit_sel == 3) {
			arb_status = (arb_status & 0x1F) | (1 << 6);
		} else if (nue_unit_sel == 2) {
			arb_status = (arb_status & 0x1F) | (1 << 7);
		}
	}
	
	return arb_status;
}

UINT32 jobm_eng_get_eng_arb_info(JOBM_ENG_HANDLE *p_eng, UINT32 eng, UINT32 *jl_idx, UINT32 *dispatch_sel, UINT32 *net_id, UINT64 *job_addr, UINT32 arb_id)
{
	T_CONV0_ARBITER_REGISTER0 LocalReg = {0};
	UINT64 addr = 0;
	UINT64 addr_msb = 0;
	
	if (p_eng == NULL) {
		DBG_ERR("JOBM handle null\r\n");
		return 0;
	}
	
	if (eng > 5) eng = 5; //UTIL/ROU/CAL
	LocalReg.reg  = JOBM_ENG_GETREG(p_eng->reg_io_base + 0x10*eng + CONV0_ARBITER_REGISTER0_OFS + arb_id*0x300);
	*jl_idx       = LocalReg.bit.CONV0_ARB_JOB_LIST_INDEX;
	*dispatch_sel = LocalReg.bit.CONV0_ARB_DISPATCH_SEL;
	*net_id       = LocalReg.bit.CONV0_ARB_NET_ID;
	addr          = JOBM_ENG_GETREG(p_eng->reg_io_base + 0x10*eng + CONV0_ARBITER_REGISTER2_OFS + arb_id*0x300);
	addr_msb      = JOBM_ENG_GETREG(p_eng->reg_io_base + 0x10*eng + CONV0_ARBITER_REGISTER3_OFS + arb_id*0x300);
	addr          = addr | (addr_msb << 32);	
	*job_addr     = addr;
	
	return 0;
}

UINT32 jobm_eng_get_eng_arb_cycle(JOBM_ENG_HANDLE *p_eng, UINT32 eng, UINT32 *eng_cycle, UINT32 arb_id)
{
	UINT32 i = 0;
	
	if (p_eng == NULL || eng_cycle == NULL) {
		DBG_ERR("JOBM handle null\r\n");
		return 0;
	}
	
	if (eng > 5) eng = 5; //UTIL/ROU/CAL
	for (i = 0; i < 8; i++) {
		if (eng == 0) {
			eng_cycle[i] = JOBM_ENG_GETREG(p_eng->reg_io_base + CONV_ARBITER_DEBUG_CYCLE_REGISTER0_OFS + i*0x4 + arb_id*0x300);
		} else if (eng == 4) {
			if (arb_id != 0) {
				DBG_ERR("LSU not support arb1\r\n");
				return 0;
			}
			eng_cycle[i] = JOBM_ENG_GETREG(p_eng->reg_io_base + LSU_ARBITER_DEBUG_CYCLE_REGISTER0_OFS + i*0x4 + arb_id*0x300);
		} else if (eng == 5) {
			eng_cycle[i] = JOBM_ENG_GETREG(p_eng->reg_io_base + UTIL_ROU_CAL_ARBITER_DEBUG_CYCLE_REGISTER0_OFS + i*0x4 + arb_id*0x300);
		} else {
			DBG_ERR("unknown engine\r\n");
			return 0;
		}
	}
	
	return 0;
}

INT32 jobm_eng_set_net_id(JOBM_ENG_HANDLE *p_eng, UINT32 jl_id, UINT32 net_id)
{
	T_JLA_DEBUG_REGISTER0 LocalReg;
	
	if (p_eng == NULL) {
		return -1;
	}
    if (p_eng->eng_id >= JOBM_ID_MAX_NUM)
    {
		DBG_ERR("error jobm engine id\r\n");
		return -1;
    }
	
	LocalReg.bit.JLA_NET_ID = net_id & 0xF;

	JOBM_ENG_SETREG(p_eng->reg_io_base + JLA_DEBUG_REGISTER0_OFS + 0x90*jl_id, LocalReg.reg);
	
	return 0;
}

BOOL jobm_ssdrv_proc_ver(void)
{
    DBG_DUMP("%s Version: %s %s %s\n", JOBM_SSD_DRV_NAME, JOBM_SSD_DRV_MODULE_VERSION, __DATE__, __TIME__);
    return TRUE;
}

UINT32 jobm_eng_get_clk_rate(JOBM_ENG_HANDLE *p_eng)
{
	if (p_eng == NULL) {
		DBG_ERR("JOBM handle null\r\n");
		return 0;
	}

	return 240000000;
}