/*
    JOBM dma module driver

    NT98538 JOBM dma module driver.

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


