/*
    CONV dma module drconvr

    NT98690 CONV dma module drconvr.

    @file       conv_eng_dma_base.c
    @ingroup    mCONV
    @note       None

    Copyright   Novatek Microelectronics Corp. 2019.  All rights reserved.
*/

#include "conv_eng_int_platform.h"
#include "conv_eng_dma_base.h"
#include "nue2_eng_int_dbg.h"

INT32 conv_eng_set_dma_base_addr(CONV_ENG_HANDLE *p_eng, uintptr_t base_addr_msb, uintptr_t base_addr_lsb, UINT32 base_idx, UINT32 ll_idx)
{
	if(ll_idx==1){
		T_LL_BASE_ADDRESS_REGISTER10 base0;
		T_LL_BASE_ADDRESS_REGISTER11 base0_msb;
		T_LL_BASE_ADDRESS_REGISTER12 base1;
		T_LL_BASE_ADDRESS_REGISTER13 base1_msb;
		T_LL_BASE_ADDRESS_REGISTER14 base2;
		T_LL_BASE_ADDRESS_REGISTER15 base2_msb;
		T_LL_BASE_ADDRESS_REGISTER16 base3;
		T_LL_BASE_ADDRESS_REGISTER17 base3_msb;
		T_LL_BASE_ADDRESS_REGISTER18 base4;
		T_LL_BASE_ADDRESS_REGISTER19 base4_msb;

		if (p_eng == NULL) {
			DBG_ERR("in set dma: handle is NULL?\n");
			return -1;
		}
		//printk("0x%lX\n", p_eng->reg_io_base);
		switch(base_idx) {
		case 0:
			base0.reg = base_addr_lsb;
			base0_msb.reg = base_addr_msb;
			CONV_ENG_SETREG(p_eng->reg_io_base + LL_BASE_ADDRESS_REGISTER10_OFS, base0.reg);
			CONV_ENG_SETREG(p_eng->reg_io_base + LL_BASE_ADDRESS_REGISTER11_OFS, base0_msb.reg);
			break;
		case 1:
			base1.reg = base_addr_lsb;
			base1_msb.reg = base_addr_msb;
			CONV_ENG_SETREG(p_eng->reg_io_base + LL_BASE_ADDRESS_REGISTER12_OFS, base1.reg);
			CONV_ENG_SETREG(p_eng->reg_io_base + LL_BASE_ADDRESS_REGISTER13_OFS, base1_msb.reg);
			break;
		case 2:
			base2.reg = base_addr_lsb;
			base2_msb.reg = base_addr_msb;
			CONV_ENG_SETREG(p_eng->reg_io_base + LL_BASE_ADDRESS_REGISTER14_OFS, base2.reg);
			CONV_ENG_SETREG(p_eng->reg_io_base + LL_BASE_ADDRESS_REGISTER15_OFS, base2_msb.reg);
			break;
		case 3:
			base3.reg = base_addr_lsb;
			base3_msb.reg = base_addr_msb;
			CONV_ENG_SETREG(p_eng->reg_io_base + LL_BASE_ADDRESS_REGISTER16_OFS, base3.reg);
			CONV_ENG_SETREG(p_eng->reg_io_base + LL_BASE_ADDRESS_REGISTER17_OFS, base3_msb.reg);
			break;
		case 4:
			base4.reg = base_addr_lsb;
			base4_msb.reg = base_addr_msb;
			CONV_ENG_SETREG(p_eng->reg_io_base + LL_BASE_ADDRESS_REGISTER18_OFS, base4.reg);
			CONV_ENG_SETREG(p_eng->reg_io_base + LL_BASE_ADDRESS_REGISTER19_OFS, base4_msb.reg);
			break;
		default:
			DBG_WRN("%s: not support base register %d\n\r", __func__, base_idx);
			return -1;
		}		
	}else{
		T_LL_BASE_ADDRESS_REGISTER0 base0;
		T_LL_BASE_ADDRESS_REGISTER1 base0_msb;
		T_LL_BASE_ADDRESS_REGISTER2 base1;
		T_LL_BASE_ADDRESS_REGISTER3 base1_msb;
		T_LL_BASE_ADDRESS_REGISTER4 base2;
		T_LL_BASE_ADDRESS_REGISTER5 base2_msb;
		T_LL_BASE_ADDRESS_REGISTER6 base3;
		T_LL_BASE_ADDRESS_REGISTER7 base3_msb;
		T_LL_BASE_ADDRESS_REGISTER8 base4;
		T_LL_BASE_ADDRESS_REGISTER9 base4_msb;

		if (p_eng == NULL) {
			DBG_ERR("in set dma: handle is NULL?\n");
			return -1;
		}
		//printk("0x%lX\n", p_eng->reg_io_base);
		switch(base_idx) {
		case 0:
			base0.reg = base_addr_lsb;
			base0_msb.reg = base_addr_msb;
			CONV_ENG_SETREG(p_eng->reg_io_base + LL_BASE_ADDRESS_REGISTER0_OFS, base0.reg);
			CONV_ENG_SETREG(p_eng->reg_io_base + LL_BASE_ADDRESS_REGISTER1_OFS, base0_msb.reg);
			break;
		case 1:
			base1.reg = base_addr_lsb;
			base1_msb.reg = base_addr_msb;
			CONV_ENG_SETREG(p_eng->reg_io_base + LL_BASE_ADDRESS_REGISTER2_OFS, base1.reg);
			CONV_ENG_SETREG(p_eng->reg_io_base + LL_BASE_ADDRESS_REGISTER3_OFS, base1_msb.reg);
			break;
		case 2:
			base2.reg = base_addr_lsb;
			base2_msb.reg = base_addr_msb;
			CONV_ENG_SETREG(p_eng->reg_io_base + LL_BASE_ADDRESS_REGISTER4_OFS, base2.reg);
			CONV_ENG_SETREG(p_eng->reg_io_base + LL_BASE_ADDRESS_REGISTER5_OFS, base2_msb.reg);
			break;
		case 3:
			base3.reg = base_addr_lsb;
			base3_msb.reg = base_addr_msb;
			CONV_ENG_SETREG(p_eng->reg_io_base + LL_BASE_ADDRESS_REGISTER6_OFS, base3.reg);
			CONV_ENG_SETREG(p_eng->reg_io_base + LL_BASE_ADDRESS_REGISTER7_OFS, base3_msb.reg);
			break;
		case 4:
			base4.reg = base_addr_lsb;
			base4_msb.reg = base_addr_msb;
			CONV_ENG_SETREG(p_eng->reg_io_base + LL_BASE_ADDRESS_REGISTER8_OFS, base4.reg);
			CONV_ENG_SETREG(p_eng->reg_io_base + LL_BASE_ADDRESS_REGISTER9_OFS, base4_msb.reg);
			break;
		default:
			DBG_WRN("%s: not support base register %d\n\r", __func__, base_idx);
			return -1;
		}
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
INT32 conv_eng_set_dma_in_addr_buf_reg(CONV_ENG_HANDLE *p_eng, UINT8 in_idx, uintptr_t in_addr)
{
	if (p_eng == NULL) {
		return -1;
	}

    if (in_idx == 0) {
    	p_eng->p_conv_reg_st->reg_5.bit.dram_in_saddr0 = in_addr & 0x0FFFFFFFF;
        p_eng->p_conv_reg_chg_flag[5] = TRUE;
    } else if (in_idx == 1) {
        p_eng->p_conv_reg_st->reg_7.bit.dram_in_saddr1 = (in_addr & 0x0FFFFFFFC) >> 2;
        p_eng->p_conv_reg_chg_flag[7] = TRUE;
    } else {

        return -1;
    }
    return 0;
}

INT32 conv_eng_set_dma_in_lnoft_buf_reg(CONV_ENG_HANDLE *p_eng, UINT8 in_idx, UINT32 ln_oft)
{
	if (p_eng == NULL) {
		return -1;
	}

    if (in_idx == 0) {
    	p_eng->p_conv_reg_st->reg_9.bit.dram_in_lofs0 = ln_oft & 0x0FFFFF;
        p_eng->p_conv_reg_chg_flag[9] = TRUE;
    } else if (in_idx == 1) {
        p_eng->p_conv_reg_st->reg_10.bit.dram_in_lofs1 = (ln_oft & 0x0FFFFC) >> 2;
        p_eng->p_conv_reg_chg_flag[10] = TRUE;
    } else {

        return -1;
    }
    return 0;
}

INT32 conv_eng_set_dma_in_enable_reg(CONV_ENG_HANDLE *p_eng, UINT8 in_idx, UINT8 enable)
{
	if (p_eng == NULL) {
		return -1;
	}

    if (in_idx == 0) {
        if (enable)
    	    p_eng->p_conv_reg_st->reg_64.bit.in0_ch_en = 1;
        else
    	    p_eng->p_conv_reg_st->reg_64.bit.in0_ch_en = 0;
    } else if (in_idx == 1) {
        if (enable)
    	    p_eng->p_conv_reg_st->reg_64.bit.in1_ch_en = 1;
        else
    	    p_eng->p_conv_reg_st->reg_64.bit.in1_ch_en = 0;
    } else {

        return -1;
    }
    
    p_eng->p_conv_reg_st->reg_64.bit.in0_lock_dis = 1;
    p_eng->p_conv_reg_st->reg_64.bit.in1_lock_dis = 1;
    p_eng->p_conv_reg_st->reg_64.bit.out0_lock_dis = 1;
    p_eng->p_conv_reg_st->reg_64.bit.out1_lock_dis = 1;
    p_eng->p_conv_reg_st->reg_64.bit.out1_lock_dis = 1;
    p_eng->p_conv_reg_st->reg_64.bit.write_ch_outstand_num = 0xf;
    p_eng->p_conv_reg_st->reg_64.bit.read_ch_outstand_num = 0xf;
    
    p_eng->p_conv_reg_chg_flag[64] = TRUE;
    return 0;
}




/**
    Set output channel DMA address
    Set output channel DMA address

    @param[in]
        -@UINT32 addr       :output channel DMA address, should be multiple of 16

    @return void
*/
INT32 conv_eng_set_dma_out_addr_buf_reg(CONV_ENG_HANDLE *p_eng, UINT8 out_idx, uintptr_t out_addr)
{

	if (p_eng == NULL) {
		return -1;
	}

    if (out_idx == 0) {
        p_eng->p_conv_reg_st->reg_11.bit.dram_out_saddr0 = out_addr & 0x0FFFFFFFF;
        p_eng->p_conv_reg_chg_flag[11] = TRUE;
    } else if (out_idx == 1) {
        p_eng->p_conv_reg_st->reg_13.bit.dram_out_saddr1 = (out_addr & 0x0FFFFFFFC) >> 2;
        p_eng->p_conv_reg_chg_flag[13] = TRUE;
    } else if (out_idx == 2) {
        p_eng->p_conv_reg_st->reg_15.bit.dram_out_saddr2 = (out_addr & 0x0FFFFFFFC) >> 2;
        p_eng->p_conv_reg_chg_flag[15] = TRUE;
    } else {

        return -1;
    }
    return 0;
}

INT32 conv_eng_set_dma_out_lnoft_buf_reg(CONV_ENG_HANDLE *p_eng, UINT8 out_idx, UINT32 ln_oft)
{

	if (p_eng == NULL) {
		return -1;
	}

    if (out_idx == 0) {
        p_eng->p_conv_reg_st->reg_17.bit.dram_out_lofs0 = ln_oft & 0x0FFFFFFFF;
        p_eng->p_conv_reg_chg_flag[17] = TRUE;
    } else if (out_idx == 1) {
        p_eng->p_conv_reg_st->reg_18.bit.dram_out_lofs1 = (ln_oft & 0x0FFFFFFFC) >> 2;
        p_eng->p_conv_reg_chg_flag[18] = TRUE;
    } else if (out_idx == 2) {
        p_eng->p_conv_reg_st->reg_19.bit.dram_out_lofs2 = (ln_oft & 0x0FFFFFFFC) >> 2;
        p_eng->p_conv_reg_chg_flag[19] = TRUE;
    } else {

        return -1;
    }
    return 0;
}

INT32 conv_eng_set_dma_out_enable_reg(CONV_ENG_HANDLE *p_eng, UINT8 in_idx, UINT8 enable)
{
	if (p_eng == NULL) {
		return -1;
	}

    if (in_idx == 0) {
        if (enable)
    	    p_eng->p_conv_reg_st->reg_64.bit.out0_ch_en = 1;
        else
    	    p_eng->p_conv_reg_st->reg_64.bit.out0_ch_en = 0;
    } else if (in_idx == 1) {
        if (enable)
    	    p_eng->p_conv_reg_st->reg_64.bit.out1_ch_en = 1;
        else
    	    p_eng->p_conv_reg_st->reg_64.bit.out1_ch_en = 0;
    } else if (in_idx == 2) {
        if (enable)
            p_eng->p_conv_reg_st->reg_64.bit.out2_ch_en = 1;
        else
            p_eng->p_conv_reg_st->reg_64.bit.out2_ch_en = 0;
    } else {

        return -1;
    }

    p_eng->p_conv_reg_st->reg_64.bit.in0_lock_dis = 1;
    p_eng->p_conv_reg_st->reg_64.bit.in1_lock_dis = 1;
    p_eng->p_conv_reg_st->reg_64.bit.out0_lock_dis = 1;
    p_eng->p_conv_reg_st->reg_64.bit.out1_lock_dis = 1;
    p_eng->p_conv_reg_st->reg_64.bit.out1_lock_dis = 1;
    p_eng->p_conv_reg_st->reg_64.bit.write_ch_outstand_num = 0xf;
    p_eng->p_conv_reg_st->reg_64.bit.read_ch_outstand_num = 0xf;

    p_eng->p_conv_reg_chg_flag[64] = TRUE;

    return 0;
}

#endif
