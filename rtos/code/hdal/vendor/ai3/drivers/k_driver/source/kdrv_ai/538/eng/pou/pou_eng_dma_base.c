/*
    POU dma module driver

    NT98539A POU dma module driver.

    @file       pou_eng_dma_base.c
    @ingpoup    mIIPPPOU
    @note       None

    Copyright   Novatek Microelectronics Corp. 2023.  All rights reserved.
*/
#include "pou_eng_int_platform.h"
#include "pou_eng_dma_base.h"
#include "pou_eng_int_dbg.h"


INT32 pou_eng_set_dma_base_addr(POU_ENG_HANDLE *p_eng, uintptr_t base_addr_msb, uintptr_t base_addr_lsb, UINT32 base_idx, UINT32 ll_idx)
{
    T_POU_LL_BASE_ADDR_REGISTER_0_OFS base0;
    T_POU_LL_BASE_ADDR_REGISTER_1_OFS base0_msb;
    T_POU_LL_BASE_ADDR_REGISTER_2_OFS base1;
    T_POU_LL_BASE_ADDR_REGISTER_3_OFS base1_msb;
    T_POU_LL_BASE_ADDR_REGISTER_4_OFS base2;
    T_POU_LL_BASE_ADDR_REGISTER_5_OFS base2_msb;
    T_POU_LL_BASE_ADDR_REGISTER_6_OFS base3;
    T_POU_LL_BASE_ADDR_REGISTER_7_OFS base3_msb;
    T_POU_LL_BASE_ADDR_REGISTER_8_OFS base4;
    T_POU_LL_BASE_ADDR_REGISTER_9_OFS base4_msb;
	T_POU_LL_BASE_ADDR_REGISTER_10_OFS base5;
    T_POU_LL_BASE_ADDR_REGISTER_11_OFS base5_msb;
	T_POU_LL_BASE_ADDR_REGISTER_12_OFS base6;
    T_POU_LL_BASE_ADDR_REGISTER_13_OFS base6_msb;

    if (p_eng == NULL) {
		return -1;
	}
    if (base_idx > 6) {
        DBG_ERR("POU not support base%d\n\r", base_idx);
        return -1;
    }

    switch(base_idx) {
    case 0:
        base0.reg = base_addr_lsb;
        base0_msb.reg = base_addr_msb;
        POU_ENG_SETREG(p_eng->reg_io_base + POU_LL_BASE_ADDR_REGISTER_0_OFS, base0.reg);
        POU_ENG_SETREG(p_eng->reg_io_base + POU_LL_BASE_ADDR_REGISTER_1_OFS, base0_msb.reg);
        break;
    case 1:
        base1.reg = base_addr_lsb;
        base1_msb.reg = base_addr_msb;
        POU_ENG_SETREG(p_eng->reg_io_base + POU_LL_BASE_ADDR_REGISTER_2_OFS, base1.reg);
        POU_ENG_SETREG(p_eng->reg_io_base + POU_LL_BASE_ADDR_REGISTER_3_OFS, base1_msb.reg);
        break;
    case 2:
        base2.reg = base_addr_lsb;
        base2_msb.reg = base_addr_msb;
        POU_ENG_SETREG(p_eng->reg_io_base + POU_LL_BASE_ADDR_REGISTER_4_OFS, base2.reg);
        POU_ENG_SETREG(p_eng->reg_io_base + POU_LL_BASE_ADDR_REGISTER_5_OFS, base2_msb.reg);
        break;
    case 3:
        base3.reg = base_addr_lsb;
        base3_msb.reg = base_addr_msb;
        POU_ENG_SETREG(p_eng->reg_io_base + POU_LL_BASE_ADDR_REGISTER_6_OFS, base3.reg);
        POU_ENG_SETREG(p_eng->reg_io_base + POU_LL_BASE_ADDR_REGISTER_7_OFS, base3_msb.reg);
        break;
    case 4:
        base4.reg = base_addr_lsb;
        base4_msb.reg = base_addr_msb;
        POU_ENG_SETREG(p_eng->reg_io_base + POU_LL_BASE_ADDR_REGISTER_8_OFS, base4.reg);
        POU_ENG_SETREG(p_eng->reg_io_base + POU_LL_BASE_ADDR_REGISTER_9_OFS, base4_msb.reg);
        break;
    case 5:
        base5.reg = base_addr_lsb;
        base5_msb.reg = base_addr_msb;
        POU_ENG_SETREG(p_eng->reg_io_base + POU_LL_BASE_ADDR_REGISTER_10_OFS, base5.reg);
        POU_ENG_SETREG(p_eng->reg_io_base + POU_LL_BASE_ADDR_REGISTER_11_OFS, base5_msb.reg);
        break;
    case 6:
        base6.reg = base_addr_lsb;
        base6_msb.reg = base_addr_msb;
        POU_ENG_SETREG(p_eng->reg_io_base + POU_LL_BASE_ADDR_REGISTER_12_OFS, base6.reg);
        POU_ENG_SETREG(p_eng->reg_io_base + POU_LL_BASE_ADDR_REGISTER_13_OFS, base6_msb.reg);
        break;
    default:
        DBG_ERR("POU not support base%d\n\r", base_idx);
        return -1;
    }
    return 0;
}

