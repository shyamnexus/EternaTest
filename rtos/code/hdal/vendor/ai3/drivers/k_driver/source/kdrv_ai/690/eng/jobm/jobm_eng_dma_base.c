/*
    JOBM dma module driver

    NT98690 JOBM dma module driver.

    @file       jobm_eng_dma_base.c
    @ingjobmp    mJOBM
    @note       None

    Copyright   Novatek Microelectronics Corp. 2019.  All rights reserved.
*/
#include "jobm_eng_int_platform.h"
#include "jobm_eng_dma_base.h"
#include "jobm_eng_int_dbg.h"

INT32 jobm_eng_set_dma_base_addr(JOBM_ENG_HANDLE *p_eng, uintptr_t base_addr_msb, uintptr_t base_addr_lsb, UINT32 base_idx, UINT32 jl_id)
{
    T_JLA_BASE_ADDRESS_REGISTER0 base;
    T_JLA_BASE_ADDRESS_REGISTER1 base_msb;


    if (p_eng == NULL) {
		return -1;
	}
    if (base_idx > 4) {
        DBG_ERR("%s: not support base%d\n\r", __func__, base_idx);
        return -1;
    }
	
	base.reg     = base_addr_lsb;
    base_msb.reg = base_addr_msb;

    switch(base_idx) {
    case 0:
        JOBM_ENG_SETREG(p_eng->reg_io_base + JLA_BASE_ADDRESS_REGISTER0_OFS + 0x90*jl_id, base.reg);
        JOBM_ENG_SETREG(p_eng->reg_io_base + JLA_BASE_ADDRESS_REGISTER1_OFS + 0x90*jl_id, base_msb.reg);
        break;
    case 1:
        JOBM_ENG_SETREG(p_eng->reg_io_base + JLA_BASE_ADDRESS_REGISTER2_OFS + 0x90*jl_id, base.reg);
        JOBM_ENG_SETREG(p_eng->reg_io_base + JLA_BASE_ADDRESS_REGISTER3_OFS + 0x90*jl_id, base_msb.reg);
        break;
    case 2:
        JOBM_ENG_SETREG(p_eng->reg_io_base + JLA_BASE_ADDRESS_REGISTER4_OFS + 0x90*jl_id, base.reg);
        JOBM_ENG_SETREG(p_eng->reg_io_base + JLA_BASE_ADDRESS_REGISTER5_OFS + 0x90*jl_id, base_msb.reg);
        break;
    case 3:
        JOBM_ENG_SETREG(p_eng->reg_io_base + JLA_BASE_ADDRESS_REGISTER6_OFS + 0x90*jl_id, base.reg);
        JOBM_ENG_SETREG(p_eng->reg_io_base + JLA_BASE_ADDRESS_REGISTER7_OFS + 0x90*jl_id, base_msb.reg);
        break;
    case 4:
        JOBM_ENG_SETREG(p_eng->reg_io_base + JLA_BASE_ADDRESS_REGISTER8_OFS + 0x90*jl_id, base.reg);
        JOBM_ENG_SETREG(p_eng->reg_io_base + JLA_BASE_ADDRESS_REGISTER9_OFS + 0x90*jl_id, base_msb.reg);
        break;
    default:
        DBG_ERR("%s: not support base register %d\n\r", __func__, base_idx);
        return -1;
    }
    return 0;
}

#if 0
/**
    Set input source address
    Set input source address

    @param[in]
        -@uintptr_t src_addr    :input source address, should be multiple of 16

    @return void
*/
INT32 ive_eng_set_dma_in_addr_buf_reg(IVE_ENG_HANDLE *p_eng, UINT8 in_idx, uintptr_t in_addr)
{
	if (p_eng == NULL) {
		return -1;
	}

    if (in_idx == 0) {
    	p_eng->p_ive_reg_st->reg_5.bit.dram_in_saddr0 = in_addr & 0x0FFFFFFFF;
        p_eng->p_ive_reg_chg_flag[5] = TRUE;
    } else if (in_idx == 1) {
        p_eng->p_ive_reg_st->reg_7.bit.dram_in_saddr1 = (in_addr & 0x0FFFFFFFC) >> 2;
        p_eng->p_ive_reg_chg_flag[7] = TRUE;
    } else {

        return -1;
    }
    return 0;
}

INT32 ive_eng_set_dma_in_lnoft_buf_reg(IVE_ENG_HANDLE *p_eng, UINT8 in_idx, UINT32 ln_oft)
{
	if (p_eng == NULL) {
		return -1;
	}

    if (in_idx == 0) {
    	p_eng->p_ive_reg_st->reg_9.bit.dram_in_lofs0 = ln_oft & 0x0FFFFF;
        p_eng->p_ive_reg_chg_flag[9] = TRUE;
    } else if (in_idx == 1) {
        p_eng->p_ive_reg_st->reg_10.bit.dram_in_lofs1 = (ln_oft & 0x0FFFFC) >> 2;
        p_eng->p_ive_reg_chg_flag[10] = TRUE;
    } else {

        return -1;
    }
    return 0;
}

INT32 ive_eng_set_dma_in_enable_reg(IVE_ENG_HANDLE *p_eng, UINT8 in_idx, UINT8 enable)
{
	if (p_eng == NULL) {
		return -1;
	}

    if (in_idx == 0) {
        if (enable)
    	    p_eng->p_ive_reg_st->reg_64.bit.in0_ch_en = 1;
        else
    	    p_eng->p_ive_reg_st->reg_64.bit.in0_ch_en = 0;
    } else if (in_idx == 1) {
        if (enable)
    	    p_eng->p_ive_reg_st->reg_64.bit.in1_ch_en = 1;
        else
    	    p_eng->p_ive_reg_st->reg_64.bit.in1_ch_en = 0;
    } else {

        return -1;
    }
    
    p_eng->p_ive_reg_st->reg_64.bit.in0_lock_dis = 1;
    p_eng->p_ive_reg_st->reg_64.bit.in1_lock_dis = 1;
    p_eng->p_ive_reg_st->reg_64.bit.out0_lock_dis = 1;
    p_eng->p_ive_reg_st->reg_64.bit.out1_lock_dis = 1;
    p_eng->p_ive_reg_st->reg_64.bit.out1_lock_dis = 1;
    p_eng->p_ive_reg_st->reg_64.bit.write_ch_outstand_num = 0xf;
    p_eng->p_ive_reg_st->reg_64.bit.read_ch_outstand_num = 0xf;
    
    p_eng->p_ive_reg_chg_flag[64] = TRUE;
    return 0;
}

/**
    Set output channel DMA address
    Set output channel DMA address

    @param[in]
        -@UINT32 addr       :output channel DMA address, should be multiple of 16

    @return void
*/
INT32 ive_eng_set_dma_out_addr_buf_reg(IVE_ENG_HANDLE *p_eng, UINT8 out_idx, uintptr_t out_addr)
{

	if (p_eng == NULL) {
		return -1;
	}

    if (out_idx == 0) {
        p_eng->p_ive_reg_st->reg_11.bit.dram_out_saddr0 = out_addr & 0x0FFFFFFFF;
        p_eng->p_ive_reg_chg_flag[11] = TRUE;
    } else if (out_idx == 1) {
        p_eng->p_ive_reg_st->reg_13.bit.dram_out_saddr1 = (out_addr & 0x0FFFFFFFC) >> 2;
        p_eng->p_ive_reg_chg_flag[13] = TRUE;
    } else if (out_idx == 2) {
        p_eng->p_ive_reg_st->reg_15.bit.dram_out_saddr2 = (out_addr & 0x0FFFFFFFC) >> 2;
        p_eng->p_ive_reg_chg_flag[15] = TRUE;
    } else {

        return -1;
    }
    return 0;
}

INT32 ive_eng_set_dma_out_lnoft_buf_reg(IVE_ENG_HANDLE *p_eng, UINT8 out_idx, UINT32 ln_oft)
{

	if (p_eng == NULL) {
		return -1;
	}

    if (out_idx == 0) {
        p_eng->p_ive_reg_st->reg_17.bit.dram_out_lofs0 = ln_oft & 0x0FFFFFFFF;
        p_eng->p_ive_reg_chg_flag[17] = TRUE;
    } else if (out_idx == 1) {
        p_eng->p_ive_reg_st->reg_18.bit.dram_out_lofs1 = (ln_oft & 0x0FFFFFFFC) >> 2;
        p_eng->p_ive_reg_chg_flag[18] = TRUE;
    } else if (out_idx == 2) {
        p_eng->p_ive_reg_st->reg_19.bit.dram_out_lofs2 = (ln_oft & 0x0FFFFFFFC) >> 2;
        p_eng->p_ive_reg_chg_flag[19] = TRUE;
    } else {

        return -1;
    }
    return 0;
}

INT32 ive_eng_set_dma_out_enable_reg(IVE_ENG_HANDLE *p_eng, UINT8 in_idx, UINT8 enable)
{
	if (p_eng == NULL) {
		return -1;
	}

    if (in_idx == 0) {
        if (enable)
    	    p_eng->p_ive_reg_st->reg_64.bit.out0_ch_en = 1;
        else
    	    p_eng->p_ive_reg_st->reg_64.bit.out0_ch_en = 0;
    } else if (in_idx == 1) {
        if (enable)
    	    p_eng->p_ive_reg_st->reg_64.bit.out1_ch_en = 1;
        else
    	    p_eng->p_ive_reg_st->reg_64.bit.out1_ch_en = 0;
    } else if (in_idx == 2) {
        if (enable)
            p_eng->p_ive_reg_st->reg_64.bit.out2_ch_en = 1;
        else
            p_eng->p_ive_reg_st->reg_64.bit.out2_ch_en = 0;
    } else {

        return -1;
    }

    p_eng->p_ive_reg_st->reg_64.bit.in0_lock_dis = 1;
    p_eng->p_ive_reg_st->reg_64.bit.in1_lock_dis = 1;
    p_eng->p_ive_reg_st->reg_64.bit.out0_lock_dis = 1;
    p_eng->p_ive_reg_st->reg_64.bit.out1_lock_dis = 1;
    p_eng->p_ive_reg_st->reg_64.bit.out1_lock_dis = 1;
    p_eng->p_ive_reg_st->reg_64.bit.write_ch_outstand_num = 0xf;
    p_eng->p_ive_reg_st->reg_64.bit.read_ch_outstand_num = 0xf;

    p_eng->p_ive_reg_chg_flag[64] = TRUE;

    return 0;
}
#endif

