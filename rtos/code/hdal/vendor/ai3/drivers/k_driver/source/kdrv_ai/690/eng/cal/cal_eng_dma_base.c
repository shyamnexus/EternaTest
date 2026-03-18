/*
    CAL dma module driver

    NT98690 CAL dma module driver.

    @file       cal_eng_dma_base.c
    @ingroup    mROU
    @note       None

    Copyright   Novatek Microelectronics Corp. 2019.  All rights reserved.
*/
#include "cal_eng_int_platform.h"
#include "cal_eng_dma_base.h"
#include "cal_eng_int_dbg.h"


INT32 cal_eng_set_dma_base_addr(CAL_ENG_HANDLE *p_eng, uintptr_t base_addr_msb, uintptr_t base_addr_lsb, UINT32 base_idx)
{
    T_CAL_LL_BASE_ADDR_REGISTER_0 base0;
    T_CAL_LL_BASE_ADDR_REGISTER_1 base0_msb;
    T_CAL_LL_BASE_ADDR_REGISTER_2 base1;
    T_CAL_LL_BASE_ADDR_REGISTER_3 base1_msb;
    T_CAL_LL_BASE_ADDR_REGISTER_4 base2;
    T_CAL_LL_BASE_ADDR_REGISTER_5 base2_msb;
    T_CAL_LL_BASE_ADDR_REGISTER_6 base3;
    T_CAL_LL_BASE_ADDR_REGISTER_7 base3_msb;
    #if 0
    T_CAL_LL_BASE_ADDR_REGISTER_8 base4;
    T_CAL_LL_BASE_ADDR_REGISTER_9 base4_msb;
    #endif

    if (p_eng == NULL) {
		return -1;
	}
    if (base_idx > 3) {
        DBG_ERR("CAL not support base address %d\n\r", base_idx);
        return -1;
    }

    switch(base_idx) {
    case 0:
        base0.reg = base_addr_lsb;
        base0_msb.reg = base_addr_msb;
        CAL_ENG_SETREG(p_eng->reg_io_base + CAL_LL_BASE_ADDR_REGISTER_0_OFS, base0.reg);
        CAL_ENG_SETREG(p_eng->reg_io_base + CAL_LL_BASE_ADDR_REGISTER_1_OFS, base0_msb.reg);
        break;
    case 1:
        base1.reg = base_addr_lsb;
        base1_msb.reg = base_addr_msb;
        CAL_ENG_SETREG(p_eng->reg_io_base + CAL_LL_BASE_ADDR_REGISTER_2_OFS, base1.reg);
        CAL_ENG_SETREG(p_eng->reg_io_base + CAL_LL_BASE_ADDR_REGISTER_3_OFS, base1_msb.reg);
        break;
    case 2:
        base2.reg = base_addr_lsb;
        base2_msb.reg = base_addr_msb;
        CAL_ENG_SETREG(p_eng->reg_io_base + CAL_LL_BASE_ADDR_REGISTER_4_OFS, base2.reg);
        CAL_ENG_SETREG(p_eng->reg_io_base + CAL_LL_BASE_ADDR_REGISTER_5_OFS, base2_msb.reg);
        break;
    case 3:
        base3.reg = base_addr_lsb;
        base3_msb.reg = base_addr_msb;
        CAL_ENG_SETREG(p_eng->reg_io_base + CAL_LL_BASE_ADDR_REGISTER_6_OFS, base3.reg);
        CAL_ENG_SETREG(p_eng->reg_io_base + CAL_LL_BASE_ADDR_REGISTER_7_OFS, base3_msb.reg);
        break;
    #if 0
    case 4:
        base4.reg = base_addr_lsb;
        base4_msb.reg = base_addr_msb;
        CAL_ENG_SETREG(p_eng->reg_io_base + CAL_LL_BASE_ADDR_REGISTER_8_OFS, base4.reg);
        CAL_ENG_SETREG(p_eng->reg_io_base + CAL_LL_BASE_ADDR_REGISTER_9_OFS, base4_msb.reg);
        break;
    #endif
    default:
        DBG_ERR("CAL not support base register %d\n\r", base_idx);
        return -1;
    }
    return 0;
}


