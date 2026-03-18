/**
    nvt-opt key manager
    This file will Enable and disable SRAM shutdown
    @file       nvt-otp.c
    @ingroup
    @note
    Copyright   Novatek Microelectronics Corp. 2018.  All rights reserved.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License version 2 as
    published by the Free Software Foundation.
*/

//#include <common.h>
#include "trace.h"
#include "io.h"
#include <mm/core_memprot.h>
#include <mm/core_mmu.h>
#include <efuse_protected.h>
#include <plat/io_address.h>
#include <kwrap/spinlock.h>
#include <pll_protected.h>

#ifndef CHKPNT
#define CHKPNT    EMSG("\033[37mCHK: %d, %s\033[0m\r\n", __LINE__, __func__)
#endif

#ifndef DBGD
#define DBGD(x)   EMSG("\033[0;35m%s=%d\033[0m\r\n", #x, x)
#endif

#ifndef DBGH
#define DBGH(x)   EMSG("\033[0;35m%s=0x%08X\033[0m\r\n", #x, x)
#endif

#ifndef DBG_DUMP
#define DBG_DUMP(fmtstr, args...) EMSG(fmtstr, ##args)
#endif

#ifndef DBG_ERR
#define DBG_ERR(fmtstr, args...)  EMSG("\033[0;31mERR:%s() \033[0m" fmtstr, __func__, ##args)
#endif

#ifndef DBG_WRN
#define DBG_WRN(fmtstr, args...)  EMSG("\033[0;33mWRN:%s() \033[0m" fmtstr, __func__, ##args)
#endif

#if 0
#define DBG_IND(fmtstr, args...) printf("%s(): " fmtstr, __func__, ##args)
#else
#ifndef DBG_IND
#define DBG_IND(fmtstr, args...)
#endif
#endif

static vaddr_t otp_reg_base;
static vaddr_t top_reg_base;
static vaddr_t gpio_reg_base;
static vaddr_t tmr_reg_base;

#define INREG32(x)              io_read32(x+otp_reg_base)
#define OUTREG32(x, y)          io_write32(x+otp_reg_base, y)

#define SETREG32(x, y)          OUTREG32((x), INREG32(x) | (y))
#define CLRREG32(x, y)          OUTREG32((x), INREG32(x) & ~(y))

#define TOP_OUTREG32(x, y)      io_write32(x+top_reg_base, y)
#define TOP_INREG32(x)          io_read32(x+top_reg_base)

#define TOP_SETREG32(x, y)      TOP_OUTREG32((x), TOP_INREG32(x) | (y))     ///< Set 32bits IO register
#define TOP_CLRREG32(x, y)      TOP_OUTREG32((x), TOP_INREG32(x) & ~(y))

#define GPIO_OUTREG32(x, y)     io_write32(x+gpio_reg_base, y)                 ///< Write 32bits IO register
#define GPIO_INREG32(x)         io_read32(x+gpio_reg_base)

#define GPIO_SETREG32(x, y)     GPIO_OUTREG32((x), GPIO_INREG32(x) | (y))   ///< Set 32bits IO register
#define GPIO_CLRREG32(x, y)     GPIO_OUTREG32((x), GPIO_INREG32(x) & ~(y))

#define TMR_OUTREG32(x, y)      io_write32(x+tmr_reg_base, y)                  ///< Write 32bits IO register
#define TMR_INREG32(x)          io_read32(x+tmr_reg_base)

#define TMR_SETREG32(x, y)      TMR_OUTREG32((x), TMR_INREG32(x) | (y))     ///< Set 32bits IO register
#define TMR_CLRREG32(x, y)      TMR_OUTREG32((x), TMR_INREG32(x) & ~(y))

#define HW_TIMER_TICK()          TMR_INREG32(0x108)
extern INT32    local_load_addr(UINT32 uiRowAddress, UINT32 base_addr);
extern INT32    local_store_addr(UINT32 uiRowAddress, UINT32 base_addr);
void poweron_efuse(void);
void powerdown_efuse(void);
void otp_udelay(UINT32 US);


#define OTP_48_TIMING               0x692618
#define OTP_60_TIMING               0x8B3026
#define OTP_80_TIMING               0xFF4032
#define OTP_120_TIMING              0xFF6050

#define RETRY_CNT   3
#define TIMING_1ST  0xFF5032
#define TIMING_2ND  0xFF6032
#define TIMING_3RD  0xFF7032


static UINT32 otp_timing[RETRY_CNT] = {
	TIMING_1ST,
	TIMING_2ND,
	TIMING_3RD,
};


static  VK_DEFINE_SPINLOCK(my_lock);
#define loc_cpu(flags) vk_spin_lock_irqsave(&my_lock, flags)
#define unl_cpu(flags) vk_spin_unlock_irqrestore(&my_lock, flags)
#define loc_multi_cores(flags)   loc_cpu(flags)
#define unl_multi_cores(flags)   unl_cpu(flags)

#define NOVATEK_GPIO_SAMPLE         0
//##############################################################################
//# poweron_efuse need porting by customer                                     #
//##############################################################################
void poweron_efuse(void)
{
	//Example P_GPIO_24
#if (NOVATEK_GPIO_SAMPLE == 1)
	GPIO_SETREG32(0x44, (0x1 << 24));
#endif
	otp_udelay(20 * 1000); //depend on customer
}

//##############################################################################
//# powerdown_efuse need porting by customer                                       #
//##############################################################################
void powerdown_efuse(void)
{
	//Example P_GPIO_24
#if (NOVATEK_GPIO_SAMPLE == 1)
	GPIO_SETREG32(0x64, (0x1 << 24));
#endif
	otp_udelay(20 * 1000); //depend on customer
}


void otp_udelay(UINT32 US)
{
	UINT32 start_time;
	start_time = HW_TIMER_TICK();
	while ((HW_TIMER_TICK() - start_time) < US);
}
/*
     otp_enable_reset

     Assert reset of OTP (OTP is now reset)

     @return void
*/
void otp_enable_reset(void)
{
}

/*
     otp_disable_reset

     Release reset of OTP (OTP is now available)

     @return void
*/
void otp_disable_reset(void)
{
}
static INT32 key_program_bit(UINT32 rol, UINT32 col)
{
	return local_store_addr(rol | (col << 5), otp_reg_base);
}


static INT32 key_write_data(UINT32 addr, UINT32 data)
{
	UINT32  ui_bits;
	UINT32  ui_data;

	if (addr < 12 && addr != 8) {
		DBG_ERR("key_write_data addr = 0x%08x error(need >= 12 or == 8)\r\n", (int)addr);
		return EFUSE_PARAM_ERR;
	}

	ui_data = data;
	ui_bits = 0;
	otp_disable_reset();
	SETREG32(KEY_MANAGER_CONTROL_ADDRESS, 0x1);
	while ((INREG32(KEY_MANAGER_STATUS_ADDRESS) & 0x1) != 0x1) {};
	while (ui_data) {
		ui_bits = __builtin_ctz(ui_data);
		ui_data &= ~(1 << ui_bits);
		if (key_program_bit(addr, ui_bits) != E_OK) {
			DBG_ERR("%s,eFuse program addr[%02x] bit[%2d] fail\r\n", __func__, (UINT32)addr, (int)ui_bits);
			return EFUSE_OPS_ERR;
		} else {
			otp_udelay(10);
		}
	}
	CLRREG32(KEY_MANAGER_CONTROL_ADDRESS, 0x1);
	while ((INREG32(KEY_MANAGER_STATUS_ADDRESS) & 0x1) == 0x1) {};
	otp_enable_reset();
	return EFUSE_SUCCESS;
}

static INT32 key_write_data_timing(UINT32 addr, UINT32 data, UINT32 timing)
{
	UINT32  ui_bits;
	UINT32  ui_data;

	if (addr < 12 && addr != 8) {
		DBG_ERR("key_write_data addr = 0x%08x error(need >= 12 or == 8)\r\n", (int)addr);
		return EFUSE_PARAM_ERR;
	}

	ui_data = data;
	ui_bits = 0;
	otp_disable_reset();
	OUTREG32(KEY_MANAGER_TIMING_CONFIG_ADDRESS, timing);
	DBG_DUMP("timing_reg= 0x%x\r\n", (int)INREG32(KEY_MANAGER_TIMING_CONFIG_ADDRESS));
	SETREG32(KEY_MANAGER_CONTROL_ADDRESS, 0x1);
	while ((INREG32(KEY_MANAGER_STATUS_ADDRESS) & 0x1) != 0x1) {};
	while (ui_data) {
		ui_bits = __builtin_ctz(ui_data);
		ui_data &= ~(1 << ui_bits);
		if (key_program_bit(addr, ui_bits) != E_OK) {
			DBG_ERR("%s,eFuse program addr[%02x] bit[%2d] fail\r\n", __func__, (UINT32)addr, (int)ui_bits);
			return EFUSE_OPS_ERR;
		} else {
			otp_udelay(10);
		}
	}
	CLRREG32(KEY_MANAGER_CONTROL_ADDRESS, 0x1);
	while ((INREG32(KEY_MANAGER_STATUS_ADDRESS) & 0x1) == 0x1) {};
	otp_enable_reset();
	return EFUSE_SUCCESS;
}

#define KEY_MANAGER_READ_PARAM()        otp_key_manager(8)
#define KEY_MANAGER_WRITE_PARAM(m)      key_write_data(8, m)


/*
     efuse_set_key_destination

     Destination of efuse get key field value

     @return IC revision of specific package revision
        - @b   NT9666X_VER_A    NT9666X version A
        - @b   UNKNOWN_DIE_VER  Unknown IC die version(system must halt)
*/
static ER set_key_destination(OTP_KEY_DESTINATION key_dst, SCE_KEY_SET_TO_OTP_FIELD key_field_set)
{
	if (key_dst >= OTP_KEY_MANAGER_CNT) {
		DBG_ERR("key_dst out of range %d\r\n", key_dst);
		return EFUSE_PARAM_ERR;
	}

	if (key_dst == OTP_KEY_MANAGER_CRYPTO &&  key_field_set >= EFUSE_CRYPTO_ENGINE_KEY_CNT) {
		DBG_ERR("Dest[Crypto]=>key_field_set out of range %d > %d\r\n", key_field_set, EFUSE_CRYPTO_ENGINE_KEY_CNT);
		return EFUSE_PARAM_ERR;
	}

	if ((key_dst == OTP_KEY_MANAGER_RSA || key_dst == OTP_KEY_MANAGER_HASH) &&  key_field_set >= EFUSE_TOTAL_KEY_SET_FIELD) {
		DBG_ERR("Dest[RSA] or [HASH] =>key_field_set out of range %d > %d\r\n", key_field_set, EFUSE_CRYPTO_ENGINE_KEY_CNT);
		return EFUSE_PARAM_ERR;
	}
	//DBG_WRN("key_dst = %d key_field_set = %d\r\n", (int)key_dst, key_field_set);
	OUTREG32(KEY_MANAGER_DESTINATION_ADDRESS, key_dst);
	OUTREG32(KEY_MANAGER_KEY_INDEX_ADDRESS, key_field_set);
	return E_OK;
}


void otp_init(void)
{
	UINT32	uiReg;
	UINT32	uiclk_sel;
	otp_reg_base = (vaddr_t)phys_to_virt(IOADDR_EFUSE_REG_BASE, MEM_AREA_IO_SEC, 0x100);
	top_reg_base = (vaddr_t)phys_to_virt(IOADDR_TOP_REG_BASE, MEM_AREA_IO_SEC, 0x200);
	gpio_reg_base = (vaddr_t)phys_to_virt(IOADDR_GPIO_REG_BASE, MEM_AREA_IO_SEC, 0x200);
	tmr_reg_base = (vaddr_t)phys_to_virt(IOADDR_TIMER_REG_BASE, MEM_AREA_IO_SEC, 0x200);

	DBG_DUMP("otp_reg_base= 0x%x\r\n", (int)otp_reg_base);
	pll_set_clock_rate(PLL_CLKSEL_APB, PLL_CLKSEL_APB_80);
	uiclk_sel = pll_get_clock_rate(PLL_CLKSEL_APB);

	if(uiclk_sel == PLL_CLKSEL_APB_48) {
		DBG_DUMP("48MHz\r\n");
		uiReg = OTP_48_TIMING;
		OUTREG32(KEY_MANAGER_TIMING_CONFIG_ADDRESS, uiReg);
	} else if(uiclk_sel == PLL_CLKSEL_APB_60) {
		DBG_DUMP("60MHz\r\n");
		uiReg = OTP_60_TIMING;
		OUTREG32(KEY_MANAGER_TIMING_CONFIG_ADDRESS, uiReg);
	} else if(uiclk_sel == PLL_CLKSEL_APB_80) {
		DBG_DUMP("80MHz\r\n");
		uiReg = OTP_80_TIMING;
		OUTREG32(KEY_MANAGER_TIMING_CONFIG_ADDRESS, uiReg);
	} else if(uiclk_sel == PLL_CLKSEL_APB_120) {
		DBG_DUMP("120MHz\r\n");
		uiReg = OTP_120_TIMING;
		OUTREG32(KEY_MANAGER_TIMING_CONFIG_ADDRESS, uiReg);
	}
	uiReg = INREG32(KEY_MANAGER_TIMING_CONFIG_ADDRESS);
	DBG_DUMP("otp_timing= 0x%x\r\n", (int)uiReg);

	//##########################################################################
	//Sample code
	//##########################################################################
#if (NOVATEK_GPIO_SAMPLE == 1)
	//PGPIO24 as GPIO
	TOP_SETREG32(0xA8, (0x1 << 24));

	//PGPIO24 as output PIN
	GPIO_SETREG32(0x24, (0x1 << 24));
#endif
	powerdown_efuse();

}

/**
    otp_write_key

    Write specific key into specific key set (0~3)

    @Note: key set 0 is for secure boot use

    @param[in] key_set_index   key set (0~3)
    @param[in] ucKey           key (16bytes)
    @return Description of data returned.
        - @b E_OK:   Success
        - @b E_SYS:  Fail
*/
INT32 otp_write_key(EFUSE_OTP_KEY_SET_FIELD key_set_index, UINT8 *uc_key)
{
	INT32   result = EFUSE_SUCCESS;
	UINT8    *u32_key = (UINT8 *)(uc_key);
	UINT32  data[4];
	UINT32  data_read_back[4];
	UINT32  data_2nd_write[4];
	UINT32  key_field_start_index = 16;
	UINT32  index_cnt;
	UINT32  enable_secure_number = 32;
	UINT32  key_field_index;
	UINT32  uiRetryCnt = RETRY_CNT;
	UINT32  uiRetryIndex = 0;
	UINT32  uiNeedRetry = FALSE;
	switch (key_set_index) {
	//Note: >>>1st Key set is dedicate for secure boot usage<<<
	case EFUSE_OTP_1ST_KEY_SET_FIELD:
		key_field_start_index = 16;
		if (is_1st_key_programmed() == 1) {
			result = EFUSE_FREEZE_ERR;
		} else {
			enable_secure_number = SECUREBOOT_1ST_KEY_SET_PROGRAMMED;
		}
		break;

	case EFUSE_OTP_2ND_KEY_SET_FIELD:
		key_field_start_index = 20;
		if (is_2nd_key_programmed() == 1) {
			result = EFUSE_FREEZE_ERR;
		} else {
			enable_secure_number = SECUREBOOT_2ND_KEY_SET_PROGRAMMED;
		}
		break;

	case EFUSE_OTP_3RD_KEY_SET_FIELD:
		key_field_start_index = 24;
		if (is_3rd_key_programmed() == 1) {
			result = EFUSE_FREEZE_ERR;
		} else {
			enable_secure_number = SECUREBOOT_3RD_KEY_SET_PROGRAMMED;
		}
		break;

	case EFUSE_OTP_4TH_KEY_SET_FIELD:
		key_field_start_index = 28;
		if (is_4th_key_programmed() == 1) {
			result = EFUSE_FREEZE_ERR;
		} else {
			enable_secure_number = SECUREBOOT_4TH_KEY_SET_PROGRAMMED;
		}
		break;

	case EFUSE_OTP_5TH_KEY_SET_FIELD:
		key_field_start_index = 12;
		if (is_5th_key_programmed() == 1) {
			result = EFUSE_FREEZE_ERR;
		} else {
			enable_secure_number = SECUREBOOT_5TH_KEY_SET_PROGRAMMED;
		}
		break;

	default:
		DBG_ERR("Unknow key set[%d] => should be 0~4\r\n", (int)key_set_index + 1);
		result = EFUSE_OPS_ERR;
		break;
	}

	if (result != EFUSE_SUCCESS) {
		if (result == EFUSE_FREEZE_ERR) {
			DBG_ERR("key set[%d], key field not enpty\r\n", (int)key_set_index);
		}
		return result;
	}
	/*
	    //Need porting
	    data[0] = *(UINT32 *)(u32_key + 0);
	    data[1] = *(UINT32 *)(u32_key + 4);
	    data[2] = *(UINT32 *)(u32_key + 8);
	    data[3] = *(UINT32 *)(u32_key + 12);
	*/
	data[0] =  u32_key[0] | (u32_key[1] << 8) | (u32_key[2] << 16) | (u32_key[3] << 24);
	data[1] =  u32_key[4] | (u32_key[5] << 8) | (u32_key[6] << 16) | (u32_key[7] << 24);
	data[2] =  u32_key[8] | (u32_key[9] << 8) | (u32_key[10] << 16) | (u32_key[11] << 24);
	data[3] = u32_key[12] | (u32_key[13] << 8) | (u32_key[14] << 16) | (u32_key[15] << 24);

	uiNeedRetry = FALSE;
	for (uiRetryIndex = 0; uiRetryIndex < uiRetryCnt; uiRetryIndex++) {
		poweron_efuse();

		for (index_cnt = 0; index_cnt < EFUSE_OTP_KEY_FIELD_CNT; index_cnt++) {
			key_field_index = key_field_start_index + index_cnt;
			if (uiNeedRetry) {
				result = key_write_data_timing(key_field_index, data_2nd_write[index_cnt], otp_timing[uiRetryIndex - 1]);
				if (result < 0) {
					DBG_DUMP("[%d]set key => write addr[%2d][0x%08x](org) 2nd data[0x%08x] fail\r\n", (int)(key_set_index + 1), (int)(key_field_index), (UINT32)data[index_cnt], data_2nd_write[index_cnt]);
					break;
				}
			} else {
				result = key_write_data(key_field_index, data[index_cnt]);
				if (result < 0) {
					//DBG_ERR("[%d]set key => write addr[%2d][0x%08lx] fail\r\n", (int)(((key_field_start_index - 16) / 4) + 1), (int)(key_field_start_index + index_cnt), (unsigned long)data[index_cnt]);
					DBG_DUMP("[%d]set key => write addr[%2d][0x%08x] fail\r\n", (int)(key_set_index + 1), (int)(key_field_index), (UINT32)data[index_cnt]);
					break;
				}
			}
		}
		powerdown_efuse();
		//Read back check
		uiNeedRetry = FALSE;
		for (index_cnt = 0; index_cnt < EFUSE_OTP_KEY_FIELD_CNT; index_cnt++) {
			key_field_index = key_field_start_index + index_cnt;
			data_read_back[index_cnt] = otp_key_manager(key_field_index);
			data_2nd_write[index_cnt] = data_read_back[index_cnt] ^ data[index_cnt];
			if (data_2nd_write[index_cnt]) {
				DBG_ERR("[%d]set key => write addr[%2d][0x%08x] != [0x%08x] fail 2nd(0x%08x)\r\n", (int)(key_set_index + 1), (int)(key_field_index), (UINT32)data[index_cnt], (UINT32)data_read_back[index_cnt], (UINT32)data_2nd_write[index_cnt]);
				uiNeedRetry = TRUE;
				result = EFUSE_CONTENT_ERR;
			}
		}

		if (uiNeedRetry == FALSE) {
			result = EFUSE_SUCCESS;
			break;
		}
	}

	if(result == EFUSE_SUCCESS) {
		enable_secure_boot(enable_secure_number);
	}
	return result;
}

/**
    otp_set_key_destination

    Durung encrypt or decrypt, configure specific key set as AES key(0~3)

    @Note: key set 0 is for secure boot use

	@param[in]	key_dst			Crypto engone / RSA / Hash engine
    @param[in] 	key_word_ofs   	No. of key word offset index(total 20 words->640 bits => 5 sets of keys)
    @param[in]  key_word_cnt	word count of key(AES128 = 4 / AES256 = 8)
    @return Description of data returned.
        - @b E_OK:                      Success
        - @b EFUSE_OPS_ERR:             Operation error
        - @b EFUSE_UNKNOW_PARAM_ERR:    Param error
*/
INT32 otp_set_key_destination(OTP_KEY_DESTINATION key_dst, UINT32 key_word_ofs, UINT32 key_word_cnt)
{
	INT32   result = EFUSE_SUCCESS;
	UINT32  key_field_start_index = 16;
	UINT32	key_word_curr;
	UINT32  key_field_index;

	//DBG_WRN("trigger_efuse_key\r\n");

	if (key_dst >= OTP_KEY_MANAGER_CNT) {
		DBG_ERR("Unknow key_dst[%d] => should be 0(Normal)/1(Crypto)/2(RSA)/3(HASH)\r\n", (int)key_dst);
		return EFUSE_UNKNOW_PARAM_ERR;
	}

	if (key_word_cnt != 4 && key_word_cnt != 8) {
		DBG_ERR("Unknow key_word_cnt[%d] => should be 4(AES128) or 8(AES256)\r\n", (int)key_word_cnt);
		return EFUSE_UNKNOW_PARAM_ERR;
	}

	if ((int)key_word_ofs + (key_word_cnt - 1) > 19) {
		DBG_ERR("Key field key_word_ofs[%d] ~ [%d](exceed 19)\r\n", (int)key_word_ofs, (int)(key_word_ofs + key_word_cnt - 1));
		return EFUSE_UNKNOW_PARAM_ERR;
	}


	if (key_dst == OTP_KEY_MANAGER_NONE) {
		DBG_WRN("Caution : Normal operation of key manager => not send key to any secure engine(Crypte/RSA/HASH)\r\n");
	}

	switch (key_word_ofs) {
	//Note: >>>1st Key set is dedicate for secure boot usage<<<
	case 0:
	case 4:
	case 8:
	case 12:
		key_field_start_index = 16 + key_word_ofs;
		break;

	case 16:
		key_field_start_index = 12;
		break;


	default:
		DBG_ERR("Start key word ofs not multiple of 4 [%d] => should be 0/4/8/12/16\r\n", (int)key_word_ofs);
		result = EFUSE_OPS_ERR;
		break;
	}


	//efuse_open(EFUSE_AUTO_MODE, EFUSE_READ, EFUSE_REPAIR_DISABLE, EFUSE_ARRAY_FUNCTION_SEL);

	for (key_word_curr = 0; key_word_curr < key_word_cnt; key_word_curr++) {
		otp_disable_reset();
		set_key_destination(key_dst, key_word_curr);
		key_field_index = key_field_start_index + key_word_curr;
		if (key_field_start_index + key_word_curr < 32) {
			otp_key_manager(key_field_index);
		} else {
			otp_key_manager(key_field_index - 20);
		}
	}
	//efuse_close();
	return result;
}

/**
    otp_set_key_engine_access_right

    Once otp_set_key_engine_access_right set, this key set field will not transfer key to destination engine(RSA/HASH/Crypto)

    @Note: key set 0 is for secure boot use

    @param[in] key_set_index   key set (0~4)
    @return Description of data returned.
        - @b E_OK:   Success
*/
INT32 otp_set_key_engine_access_right(EFUSE_OTP_KEY_SET_FIELD key_set_index)
{
	UINT32	key_set_engine_access_right_index;
	switch (key_set_index) {
	case EFUSE_OTP_1ST_KEY_SET_FIELD:
		key_set_engine_access_right_index = OTP_1ST_KEY_BIT_START;
		break;

	case EFUSE_OTP_2ND_KEY_SET_FIELD:
		key_set_engine_access_right_index = OTP_2ND_KEY_BIT_START;
		break;

	case EFUSE_OTP_3RD_KEY_SET_FIELD:
		key_set_engine_access_right_index = OTP_3RD_KEY_BIT_START;
		break;

	case EFUSE_OTP_4TH_KEY_SET_FIELD:
		key_set_engine_access_right_index = OTP_4TH_KEY_BIT_START;
		break;

	case EFUSE_OTP_5TH_KEY_SET_FIELD:
		key_set_engine_access_right_index = OTP_5TH_KEY_BIT_START;
		break;

	default:
		DBG_ERR("Unknow key set[%d] => should be 0~4\r\n", (int)key_set_index);
		return EFUSE_OPS_ERR;
	}
	otp_disable_reset();
	OUTREG32(KEY_MANAGER_KEY_ENGINE_RIGHT_ADDRESS, key_set_engine_access_right_index);
	otp_enable_reset();
	return EFUSE_SUCCESS;
}

//Bit[0] & Bit[5] == 1
/**
    quary_secure_boot

    Quary now is what kind of secure boot type

    @Note: key set 0 is for secure boot use

    @param[in] scu_status   status
    @return Description of data returned.
        - @b  TRUE:   enabled
        - @b FALSE:   disabled
*/
BOOL quary_secure_boot(SECUREBOOT_STATUS scu_status)
{
	UINT32  sec;
	BOOL    result = FALSE;

	sec = KEY_MANAGER_READ_PARAM();

	switch (scu_status) {
	case SECUREBOOT_SECURE_EN:
		if ((sec & (OTP_HW_SECURE_EN | OTP_FW_SECURE_EN)) == (OTP_HW_SECURE_EN | OTP_FW_SECURE_EN)) {
			result = TRUE;
		}
		break;

	case SECUREBOOT_DATA_AREA_ENCRYPT:
		if ((sec & OTP_DATA_ENCRYPT_EN) == OTP_DATA_ENCRYPT_EN) {
			result = TRUE;
		}
		break;

	case SECUREBOOT_SIGN_RSA:
		if ((sec & OTP_SIGNATURE_RSA) == OTP_SIGNATURE_RSA) {
			result = TRUE;
		}
		break;

	case SECUREBOOT_SIGN_RSA_CHK:
		if ((sec & OTP_SIGNATURE_RSA_CHK_EN) == OTP_SIGNATURE_RSA_CHK_EN) {
			result = TRUE;
		}
		break;

	case SECUREBOOT_JTAG_DISABLE_EN:
		if ((sec & OTP_JTAG_DISABLE_EN) == OTP_JTAG_DISABLE_EN) {
			result = TRUE;
		}
		break;

	case SECUREBOOT_1ST_KEY_SET_PROGRAMMED:
		if ((sec & OTP_1ST_KEY_PROGRAMMED) == OTP_1ST_KEY_PROGRAMMED) {
			result = TRUE;
		}
		break;

	case SECUREBOOT_2ND_KEY_SET_PROGRAMMED:
		if ((sec & OTP_2ND_KEY_PROGRAMMED) == OTP_2ND_KEY_PROGRAMMED) {
			result = TRUE;
		}
		break;

	case SECUREBOOT_3RD_KEY_SET_PROGRAMMED:
		if ((sec & OTP_3RD_KEY_PROGRAMMED) == OTP_3RD_KEY_PROGRAMMED) {
			result = TRUE;
		}
		break;

	case SECUREBOOT_4TH_KEY_SET_PROGRAMMED:
		if ((sec & OTP_4TH_KEY_PROGRAMMED) == OTP_4TH_KEY_PROGRAMMED) {
			result = TRUE;
		}
		break;

	case SECUREBOOT_5TH_KEY_SET_PROGRAMMED:
		if ((sec & OTP_5TH_KEY_PROGRAMMED) == (UINT32)OTP_5TH_KEY_PROGRAMMED) {
			result = TRUE;
		}
		break;
    case SECUREBOOT_1ST_KEY_SET_READ_LOCK:
		if ((sec & OTP_1ST_KEY_READ_LOCK) == OTP_1ST_KEY_READ_LOCK) {
			result = TRUE;
		}
		break;

	case SECUREBOOT_2ND_KEY_SET_READ_LOCK:
		if ((sec & OTP_2ND_KEY_READ_LOCK) == OTP_2ND_KEY_READ_LOCK) {
			result = TRUE;
		}
		break;

	case SECUREBOOT_3RD_KEY_SET_READ_LOCK:
		if ((sec & OTP_3RD_KEY_READ_LOCK) == OTP_3RD_KEY_READ_LOCK) {
			result = TRUE;
		}
		break;

	case SECUREBOOT_4TH_KEY_SET_READ_LOCK:
		if ((sec & OTP_4TH_KEY_READ_LOCK) == OTP_4TH_KEY_READ_LOCK) {
			result = TRUE;
		}
		break;

	case SECUREBOOT_5TH_KEY_SET_READ_LOCK:
		if ((sec & OTP_5TH_KEY_READ_LOCK) == OTP_5TH_KEY_READ_LOCK) {
			result = TRUE;
		}
		break;

	default:
		break;
	}
	return result;
}

/**
    set_secure_boot

    Quary now is what kind of secure boot type

    @Note: key set 0 is for secure boot use

    @param[in] scu_status   status
    @return Description of data returned.
        - @b  TRUE:   enabled
        - @b FALSE:   something wrong(already configured)
*/
BOOL enable_secure_boot(SECUREBOOT_STATUS scu_status)
{
	UINT32  sec;
	BOOL    result = FALSE;
	sec = KEY_MANAGER_READ_PARAM();

	poweron_efuse();

	switch (scu_status) {
	case SECUREBOOT_SECURE_EN:
		if ((sec & (OTP_HW_SECURE_EN | OTP_FW_SECURE_EN)) != 0x0) {
			DBG_ERR("Secure already enabled ...");
		} else {
			result = TRUE;
			KEY_MANAGER_WRITE_PARAM((OTP_HW_SECURE_EN | OTP_FW_SECURE_EN));
		}
		break;

	case SECUREBOOT_DATA_AREA_ENCRYPT:
		if ((sec & OTP_DATA_ENCRYPT_EN) != 0x0) {
			DBG_ERR("Data area encrypted bit already set ...");
		} else {
			result = TRUE;
			KEY_MANAGER_WRITE_PARAM(OTP_DATA_ENCRYPT_EN);
		}
		break;

	case SECUREBOOT_SIGN_RSA:
		if ((sec & OTP_SIGNATURE_RSA) != 0x0) {
			DBG_ERR("Signature use RSA bit already set ...");
		} else {
			result = TRUE;
			KEY_MANAGER_WRITE_PARAM(OTP_SIGNATURE_RSA);
		}
		break;

	case SECUREBOOT_SIGN_RSA_CHK:

		if ((sec & OTP_SIGNATURE_RSA_CHK_EN) != 0x0) {
			DBG_ERR("Signature use RSA and process hash checksum bit already set ...\r\n");
		} else {
			result = TRUE;
			KEY_MANAGER_WRITE_PARAM(OTP_SIGNATURE_RSA_CHK_EN);
		}
		break;

	case SECUREBOOT_JTAG_DISABLE_EN:
		KEY_MANAGER_WRITE_PARAM(OTP_JTAG_DISABLE_EN);
		result = TRUE;
		break;

	case SECUREBOOT_1ST_KEY_SET_PROGRAMMED:
		KEY_MANAGER_WRITE_PARAM(OTP_1ST_KEY_PROGRAMMED);
		result = TRUE;
		break;

	case SECUREBOOT_2ND_KEY_SET_PROGRAMMED:
		KEY_MANAGER_WRITE_PARAM(OTP_2ND_KEY_PROGRAMMED);
		result = TRUE;
		break;

	case SECUREBOOT_3RD_KEY_SET_PROGRAMMED:
		KEY_MANAGER_WRITE_PARAM(OTP_3RD_KEY_PROGRAMMED);
		result = TRUE;
		break;

	case SECUREBOOT_4TH_KEY_SET_PROGRAMMED:
		KEY_MANAGER_WRITE_PARAM(OTP_4TH_KEY_PROGRAMMED);
		result = TRUE;
		break;

	case SECUREBOOT_5TH_KEY_SET_PROGRAMMED:
		KEY_MANAGER_WRITE_PARAM(OTP_5TH_KEY_PROGRAMMED);
		result = TRUE;
		break;

	case SECUREBOOT_1ST_KEY_SET_READ_LOCK:
		KEY_MANAGER_WRITE_PARAM(OTP_1ST_KEY_READ_LOCK);
		result = TRUE;
		break;

	case SECUREBOOT_4TH_KEY_SET_READ_LOCK:
		KEY_MANAGER_WRITE_PARAM(OTP_4TH_KEY_READ_LOCK);
		result = TRUE;
		break;

	case SECUREBOOT_5TH_KEY_SET_READ_LOCK:
		KEY_MANAGER_WRITE_PARAM(OTP_5TH_KEY_READ_LOCK);
		result = TRUE;
		break;
	case SECUREBOOT_2ND_KEY_SET_READ_LOCK:
	case SECUREBOOT_3RD_KEY_SET_READ_LOCK:
		DBG_ERR("2ND & 3RD key set can not configured as read lock(RSA checksum)");
		result = FALSE;
		break;
	default:
		result = FALSE;
		break;
	}

	powerdown_efuse();
	return result;
}

/*
    otp read data

    Read specific eFuse addressing(32bits/per time)
     ^
     |-- (row address)
    @param[in] rowAddress       row address

    @return read half-word efuse data
        - @b  Positive: Valid data
        - @b    E_SYS : Invalid data
*/
UINT32 otp_key_manager(UINT32 rowAddress)
{
	INT32  uiData;
	unsigned long lock = 0;

	loc_multi_cores(lock);
	otp_disable_reset();
	uiData = local_load_addr(rowAddress, otp_reg_base);
	otp_enable_reset();
	unl_multi_cores(lock);
	//DBG_WRN("rowAddress = %d uiData = 0x%08x\r\n", (int)rowAddress, (int)uiData);

	return uiData;
}

#ifndef CYGBLD_ATTRIB_SECTION
#define CYGBLD_ATTRIB_SECTION(__sect__) __attribute__((section (__sect__)))
#endif

#define __string(_x) #_x
#define __xstring(_x) __string(_x)
#define OTP_MAJOR            00
#define OTP_MINOR            002
#define OTP_REVISION_STR     "1.00.003"

#ifndef VERSION_INFO_ID4
#define VERSION_INFO_ID4(_name, _major, _minor, _bugfix, _ext) \
	CYGBLD_ATTRIB_SECTION(".moduleTest.version." __xstring(_major) "_" __xstring(_minor) "_" __xstring(_bugfix) "." __xstring(_name))
#endif
static CHAR    OTPVersion[] CYGBLD_ATTRIB_SECTION(".otp.version." __xstring(1) "." __xstring(OTP_MAJOR)"."__xstring(OTP_MINOR)) = "NT9833X_OTP#"OTP_REVISION_STR;
void otp_showinfo(void)
{
//	UINT32 i;

	DBG_DUMP("=>[quary] ver[%s]\r\n", OTPVersion);
	DBG_DUMP("             is_secure_enable()=%d\r\n", is_secure_enable());
	DBG_DUMP("       is_data_area_encrypted()=%d\r\n", is_data_area_encrypted());
	DBG_DUMP("             is_signature_rsa()=%d\r\n", is_signature_rsa());
	DBG_DUMP("is_signature_rsa_chsum_enable()=%d\r\n", is_signature_rsa_chsum_enable());
	DBG_DUMP("           is_JTAG_DISABLE_en()=%d\r\n", is_JTAG_DISABLE_en());
	DBG_DUMP("        is_1st_key_programmed()=%d\r\n", is_1st_key_programmed());
	DBG_DUMP("        is_2nd_key_programmed()=%d\r\n", is_2nd_key_programmed());
	DBG_DUMP("        is_3rd_key_programmed()=%d\r\n", is_3rd_key_programmed());
	DBG_DUMP("        is_4th_key_programmed()=%d\r\n", is_4th_key_programmed());
	DBG_DUMP("        is_5th_key_programmed()=%d\r\n", is_5th_key_programmed());


}
