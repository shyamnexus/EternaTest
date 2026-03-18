/*
    LSU dma module driver

    NT98690 LSU dma module driver.

    @file       lsu_eng_dma_base.c
    @inglsup    mIIPPLSU
    @note       None

    Copyright   Novatek Microelectronics Corp. 2019.  All rights reserved.
*/
#include "lsu_eng_int_platform.h"
#include "lsu_eng_dma_base.h"
#include "lsu_eng_int_dbg.h"


INT32 lsu_eng_set_dma_base_addr(LSU_ENG_HANDLE *p_eng, uintptr_t base_addr_msb, uintptr_t base_addr_lsb, UINT32 base_idx)
{
    T_LSU_LL_BASE_ADDR_REGISTER_0_OFS base0;
    T_LSU_LL_BASE_ADDR_REGISTER_1_OFS base0_msb;
    T_LSU_LL_BASE_ADDR_REGISTER_2_OFS base1;
    T_LSU_LL_BASE_ADDR_REGISTER_3_OFS base1_msb;
    T_LSU_LL_BASE_ADDR_REGISTER_4_OFS base2;
    T_LSU_LL_BASE_ADDR_REGISTER_5_OFS base2_msb;
    T_LSU_LL_BASE_ADDR_REGISTER_6_OFS base3;
    T_LSU_LL_BASE_ADDR_REGISTER_7_OFS base3_msb;
    T_LSU_LL_BASE_ADDR_REGISTER_8_OFS base4;
    T_LSU_LL_BASE_ADDR_REGISTER_9_OFS base4_msb;

    if (p_eng == NULL) {
		return -1;
	}
    if (base_idx > 4) {
        DBG_ERR("LSU not support base%d\n\r", base_idx);
        return -1;
    }

    switch(base_idx) {
    case 0:
        base0.reg = base_addr_lsb;
        base0_msb.reg = base_addr_msb;
        LSU_ENG_SETREG(p_eng->reg_io_base + LSU_LL_BASE_ADDR_REGISTER_0_OFS, base0.reg);
        LSU_ENG_SETREG(p_eng->reg_io_base + LSU_LL_BASE_ADDR_REGISTER_1_OFS, base0_msb.reg);
        break;
    case 1:
        base1.reg = base_addr_lsb;
        base1_msb.reg = base_addr_msb;
        LSU_ENG_SETREG(p_eng->reg_io_base + LSU_LL_BASE_ADDR_REGISTER_2_OFS, base1.reg);
        LSU_ENG_SETREG(p_eng->reg_io_base + LSU_LL_BASE_ADDR_REGISTER_3_OFS, base1_msb.reg);
        break;
    case 2:
        base2.reg = base_addr_lsb;
        base2_msb.reg = base_addr_msb;
        LSU_ENG_SETREG(p_eng->reg_io_base + LSU_LL_BASE_ADDR_REGISTER_4_OFS, base2.reg);
        LSU_ENG_SETREG(p_eng->reg_io_base + LSU_LL_BASE_ADDR_REGISTER_5_OFS, base2_msb.reg);
        break;
    case 3:
        base3.reg = base_addr_lsb;
        base3_msb.reg = base_addr_msb;
        LSU_ENG_SETREG(p_eng->reg_io_base + LSU_LL_BASE_ADDR_REGISTER_6_OFS, base3.reg);
        LSU_ENG_SETREG(p_eng->reg_io_base + LSU_LL_BASE_ADDR_REGISTER_7_OFS, base3_msb.reg);
        break;
    case 4:
        base4.reg = base_addr_lsb;
        base4_msb.reg = base_addr_msb;
        LSU_ENG_SETREG(p_eng->reg_io_base + LSU_LL_BASE_ADDR_REGISTER_8_OFS, base4.reg);
        LSU_ENG_SETREG(p_eng->reg_io_base + LSU_LL_BASE_ADDR_REGISTER_9_OFS, base4_msb.reg);
        break;
    default:
        DBG_ERR("LSU not support base%d\n\r", base_idx);
        return -1;
    }
    return 0;
}

