/*
    UTIL dma module driver

    NT98690 UTIL dma module driver.

    @file       util_eng_dma_base.c
    @ingutilp    mUTIL
    @note       None

    Copyright   Novatek Microelectronics Corp. 2023.  All rights reserved.
*/
#include "util_eng_int_platform.h"
#include "util_eng_dma_base.h"
#include "util_eng_int_dbg.h"


INT32 util_eng_set_dma_base_addr(UTIL_ENG_HANDLE *p_eng, uintptr_t base_addr_msb, uintptr_t base_addr_lsb, UINT32 base_idx, UINT32 ll_idx)
{
    UINT32 regofs_addr_lsb=0x0, regofs_addr_msb=0x0;

    if (p_eng == NULL) {
		return -1;
	}
    if (base_idx > 4) {
        DBG_ERR("UTIL not support base address %d\n\r", base_idx);
        return -1;
    }

    // register ofs
    switch(ll_idx) {
    case 0: // LL0
        switch(base_idx) {
        case 0:
            regofs_addr_lsb = UTIL_LL_BASE_ADDR_REGISTER_0_OFS;
            regofs_addr_msb = UTIL_LL_BASE_ADDR_REGISTER_1_OFS;
            break;
        case 1:
            regofs_addr_lsb = UTIL_LL_BASE_ADDR_REGISTER_2_OFS;
            regofs_addr_msb = UTIL_LL_BASE_ADDR_REGISTER_3_OFS;
            break;
        case 2:
            regofs_addr_lsb = UTIL_LL_BASE_ADDR_REGISTER_4_OFS;
            regofs_addr_msb = UTIL_LL_BASE_ADDR_REGISTER_5_OFS;
            break;
        case 3:
            regofs_addr_lsb = UTIL_LL_BASE_ADDR_REGISTER_6_OFS;
            regofs_addr_msb = UTIL_LL_BASE_ADDR_REGISTER_7_OFS;
            break;
        case 4:
            regofs_addr_lsb = UTIL_LL_BASE_ADDR_REGISTER_8_OFS;
            regofs_addr_msb = UTIL_LL_BASE_ADDR_REGISTER_9_OFS;
            break;
        default:
            DBG_ERR("UTIL not support base register %d\n\r", base_idx);
            return -1;
        }
        break;
    case 1: // LL1
        switch(base_idx) {
        case 0:
            regofs_addr_lsb = UTIL_LL_BASE_ADDR_REGISTER_10_OFS;
            regofs_addr_msb = UTIL_LL_BASE_ADDR_REGISTER_11_OFS;
            break;
        case 1:
            regofs_addr_lsb = UTIL_LL_BASE_ADDR_REGISTER_12_OFS;
            regofs_addr_msb = UTIL_LL_BASE_ADDR_REGISTER_13_OFS;
            break;
        case 2:
            regofs_addr_lsb = UTIL_LL_BASE_ADDR_REGISTER_14_OFS;
            regofs_addr_msb = UTIL_LL_BASE_ADDR_REGISTER_15_OFS;
            break;
        case 3:
            regofs_addr_lsb = UTIL_LL_BASE_ADDR_REGISTER_16_OFS;
            regofs_addr_msb = UTIL_LL_BASE_ADDR_REGISTER_17_OFS;
            break;
        case 4:
            regofs_addr_lsb = UTIL_LL_BASE_ADDR_REGISTER_18_OFS;
            regofs_addr_msb = UTIL_LL_BASE_ADDR_REGISTER_19_OFS;
            break;
        default:
            DBG_ERR("UTIL not support base register %d\n\r", base_idx);
            return -1;
        }
        break;
    default:
        DBG_ERR("UTIL wrong LL option (%d)\n\r", ll_idx);
        return -1;
    }

    // register setting
    UTIL_ENG_SETREG(p_eng->reg_io_base + regofs_addr_lsb, base_addr_lsb);
    UTIL_ENG_SETREG(p_eng->reg_io_base + regofs_addr_msb, base_addr_msb);
    return 0;
}

