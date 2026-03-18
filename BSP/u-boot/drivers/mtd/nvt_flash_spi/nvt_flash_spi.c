/*
	@file       nvt_flash_spi.c
	@ingroup    mIStgNAND

	@brief      NAND low layer driver which will access to NAND controller

	@note       Nothing

	Copyright   Novatek Microelectronics Corp. 2011.  All rights reserved.

*/
#include "nvt_flash_spi_int.h"
#include "nvt_flash_spi_reg.h"
#include <cpu_func.h>

/*
	Configure nand host transfer register

	This function configure controller transfer related register

	@param[in]  transParam      Setup transfer configuration
	@param[in]  pageCount       Transfer page count
	@param[in]  length          Transfer length
	@param[in]  multiPageSelect multi page / multi spare / single page

	@return void
*/
void nand_host_setup_transfer(struct drv_nand_dev_info *info, \
				struct smc_setup_trans *transParam, \
				uint32_t pageCount, \
				uint32_t length, \
				uint32_t multiPageSelect)
{
	const struct nvt_nand_flash *f = info->flash_info;
	uint32_t value3 = 0;
	uint32_t value = 0;

	uint32_t value1 = 0;

	value3 = NAND_GETREG(info, NAND_INTMASK_REG_OFS);
	value3 &= ~NAND_COMP_INTREN;
	NAND_SETREG(info, NAND_INTMASK_REG_OFS, value3);

	if (pageCount > 0) {
		NAND_SETREG(info, NAND_PAGENUM_REG_OFS, pageCount);
		value = multiPageSelect;
	}

	//Even if not use FIFO, FIFO dir & PIO/DMA mode need to set
	NAND_SETREG(info, NAND_FIFO_CTRL_REG_OFS, (transParam->fifoDir | transParam->transMode));

	value1 = NAND_GETREG(info, NAND_CTRL0_REG_OFS);

	NAND_SETREG(info, NAND_CTRL0_REG_OFS, value);


	// Set column address
	NAND_SETREG(info, NAND_COLADDR_REG_OFS, transParam->colAddr);

	// Set row address
	NAND_SETREG(info, NAND_ROWADDR_REG_OFS, transParam->rowAddr);

	// Set data length
	NAND_SETREG(info, NAND_DATALEN_REG_OFS, length);

	if((value1 & _PROTECT1_EN) == _PROTECT1_EN)
		value |= _PROTECT1_EN;

	if((value1 & _PROTECT2_EN) == _PROTECT2_EN)
		value |= _PROTECT2_EN;

	value |= (transParam->type);

	if(transParam->fifoDir == _FIFO_READ)
		value |= _NAND_WP_EN;

	NAND_SETREG(info, NAND_CTRL0_REG_OFS, value);

	//config CS
	value3 = NAND_GETREG(info, NAND_SPI_CFG_REG_OFS);
	value3 &= ~(1<<3);
	value3 |= (transParam->uiCS<<3);
	NAND_SETREG(info, NAND_SPI_CFG_REG_OFS, value3);

	value3 = NAND_GETREG(info, NAND_CTRL_STS_REG_OFS);
	value3 |= NAND_COMP_STS;
	NAND_SETREG(info, NAND_CTRL_STS_REG_OFS, value3);

	// Set control0 register
	if ((transParam->type == _DATA_RD3) && (transParam->transMode == _PIO_MODE)) {
		NAND_DEBUG_MSG(" op cmd(0x16), read spare area only, do not use fifo_en\r\n");
	} else {
		if (length && ((length >= GET_NAND_SPARE_SIZE(f)*(1+info->flash_info->spare_size)) || \
						(length == 8) || (length == 4) || \
						multiPageSelect == _MULTI_SPARE)) {
			NAND_SETREG(info, NAND_FIFO_CTRL_REG_OFS, \
				(transParam->fifoDir | transParam->transMode));

			NAND_SETREG(info, NAND_FIFO_CTRL_REG_OFS, (_FIFO_EN | \
								transParam->fifoDir | \
								transParam->transMode));
		}
	}
	NAND_DEBUG_MSG("  NAND_FIFO_CTRL_REG: 0x%x\r\n", NAND_GETREG(info, NAND_FIFO_CTRL_REG_OFS));

	value3 = NAND_GETREG(info, NAND_INTMASK_REG_OFS);
	value3 |= NAND_COMP_INTREN;
	NAND_SETREG(info, NAND_INTMASK_REG_OFS, value3);

	//config NAND nor type
	nand_hostSetNandType(info, info->flash_info->config_nand_type);
}


/*
	Configure SM transfer command

	Command send to NAND

	@param[in]  command     nand command
	@param[in]  bTmOutEN    time out enable / disable

	@return void
*/
void nand_host_send_command(struct drv_nand_dev_info *info, uint32_t command,
							uint32_t bTmOutEN)
{
	union T_NAND_CTRL0_REG RegCtrl0;

	// Enable AXI ch
	nand_host_axi_ch_disable(info, 0);

	// Set command
	NAND_SETREG(info, NAND_COMMAND_REG_OFS, command);

	RegCtrl0.Reg = NAND_GETREG(info, NAND_CTRL0_REG_OFS);

	RegCtrl0.Bit.OPER_EN = 1;

	RegCtrl0.Bit.TIMEOUT_EN = bTmOutEN;

	NAND_SETREG(info, NAND_CTRL0_REG_OFS, RegCtrl0.Reg);
}


/*
	NAND controller host receive data routing

	After read operation related command issued, called this function to reveive data

	@param[in]  	buffer      buffer receive to
			length      receive buffer length
			transMode   PIO/DMA mode usage

	@return
		- @b E_OK           receive data success
		- @b E_SYS          error status
*/
int nand_host_receive_data(struct drv_nand_dev_info *info, uint8_t *buffer,
					uint32_t length, uint32_t transMode)
{
	uint32_t index = 0, tempBuffer = 0;

	if(transMode == _PIO_MODE) {
		while(length) {
			if(length >= SPINAND_FIFO_SIZE) {
				if(NAND_GETREG(info, NAND_FIFO_STS_REG_OFS) & _FIFO_FULL) {
					for (index = SPINAND_FIFO_SIZE; index > 0; index -= 4) {
						*((uint32_t *)buffer) = NAND_GETREG(info, NAND_DATAPORT_REG_OFS);
						length -= 4;
						buffer +=4;
					}
				}
			} else {
				if((NAND_GETREG(info, NAND_FIFO_STS_REG_OFS) & _FIFO_CNT_MASK) == ((length+3)/4)) {
					tempBuffer = NAND_GETREG(info, NAND_DATAPORT_REG_OFS);
					if(length < 4) {
						for(index = length; index > 0; index--) {
							*((uint8_t *)buffer) = tempBuffer & (0xFF) ;
							tempBuffer >>= 8;
							length -= 1;
							buffer += 1;
						}
					} else {
						*((uint32_t *)buffer) = tempBuffer;
						length -= 4;
						buffer += 4;
					}
				}
			}
		}
	}

	return nand_cmd_wait_complete(info);
}


/*
	NAND controller host transmit data routing

	After write operation related command issued, called this function to transmit data

	@param[in]  	buffer      buffer transmit from
			length      transmit buffer length
			transMode   PIO/DMA mode usage

	@return
		- @b E_OK           transmit data success
		- @b E_SYS          error status
*/
int nand_host_transmit_data(struct drv_nand_dev_info *info, uint8_t *buffer, uint32_t length, uint32_t transMode)
{
	uint32_t index = 0, tempBuffer = 0;
	uint8_t tempData = 0;
	BOOL not_aligned = 0;

	if(transMode == _PIO_MODE) {
		NAND_SETREG(info, NAND_FIFO_CTRL_REG_OFS, (_FIFO_EN | NAND_GETREG(info, NAND_FIFO_CTRL_REG_OFS)));

		while(length) {
			if(length >= SPINAND_FIFO_SIZE) {
				if(NAND_GETREG(info, NAND_FIFO_STS_REG_OFS) & _FIFO_EMPTY) {
					for (index = SPINAND_FIFO_SIZE; index > 0; index -= 4) {
						NAND_SETREG(info, NAND_DATAPORT_REG_OFS, *((uint32_t *)buffer));
						length -= 4;
						buffer += 4;
					}
				}
			} else {
				if((NAND_GETREG(info, NAND_FIFO_STS_REG_OFS) & _FIFO_EMPTY)) {
					while(length) {
						if(length < 4) {
							for(index = 0; index < length; index++) {
								tempData = (*((uint8_t *)buffer));
								tempBuffer |= (tempData << (8 * index));
								buffer += 1;
							}
							NAND_SETREG(info, NAND_DATAPORT_REG_OFS, tempBuffer);
							length = 0;
							not_aligned = 1;
						} else {
							NAND_SETREG(info, NAND_DATAPORT_REG_OFS, *((uint32_t *)buffer));
							length -= 4;
							buffer += 4;
						}
					}
				}
			}
		}

		if(not_aligned) {
			mdelay(1);
			tempBuffer = NAND_GETREG(info, NAND_FIFO_CTRL_REG_OFS);
        	tempBuffer &= ~(0x1);
			NAND_SETREG(info, NAND_FIFO_CTRL_REG_OFS, tempBuffer);
		}
	}

	return nand_cmd_wait_complete(info);
}


/*
	Check NAND read ready bit and make sure status register is ready

	@return E_OK : ready
		E_TMOUT: busy
*/
int nand_host_check_ready(struct drv_nand_dev_info *info)
{
#if (CONFIG_NVT_NAND_TYPE == 0)
	uint32_t i;

	i = 0;
	do {
		i++;
		//For slow card
		if (i > 0x48000) {
			printf("Status always busy\n\r");
			return -1;
		}
	} while ((NAND_GETREG(info, NAND_CTRL_STS_REG_OFS) & NAND_SM_BUSY) == 0);
#endif
	return E_OK;
}


/*
	Enable / Disable FIFO

	Enable and Disable FIFO of NAND controller

	@param[in]  uiEN     enable / disable

	@return void
*/
void nand_host_set_fifo_enable(struct drv_nand_dev_info *info, uint32_t uiEN)
{
	union T_NAND_FIFO_CTRL_REG RegFIFOCtrl;

	RegFIFOCtrl.Reg = NAND_GETREG(info, NAND_FIFO_CTRL_REG_OFS);

	RegFIFOCtrl.Bit.FIFO_EN = uiEN;

	NAND_SETREG(info, NAND_FIFO_CTRL_REG_OFS, RegFIFOCtrl.Reg);

	if(uiEN == FALSE)
		while((NAND_GETREG(info, NAND_FIFO_CTRL_REG_OFS) & _FIFO_EN) != 0);
}


/*
	Set NAND controller error correction SRAM access right(for CPU or NAND controller)

	Set register to switch error information SRAM access right to CPU or NAND controller

	@param[in]  bcpu_Acc
		- @b TRUE       switch error information sram area access right to CPU
		- @b FALSE      Switch error information sram area access right to NAND controller access

	@return
		- void
*/
void nand_host_set_cpu_access_err(struct drv_nand_dev_info *info, uint32_t bcpu_Acc)
{
	union T_NAND_SRAM_ACCESS_REG  RegSramAcc = {0x00000000};
	uint8_t i;

	RegSramAcc.Reg = NAND_GETREG(info, NAND_SRAM_ACCESS_REG_OFS);

	RegSramAcc.Bit.ERROR_ACC = bcpu_Acc;

	NAND_SETREG(info, NAND_SRAM_ACCESS_REG_OFS,RegSramAcc.Reg);

	//dummy read to delay 200ns for SRAM ready
	if(bcpu_Acc == TRUE) {
		for(i = 0; i < SRAMRDY_DELAY; i++)
			RegSramAcc.Reg = NAND_GETREG(info, NAND_SRAM_ACCESS_REG_OFS);
	}
}


/*
	Set NAND controller spare SRAM area access right(for CPU or NAND controller)

	Set register to switch spare area SRAM access right to CPU or NAND controller

	@param[in]  bcpu_Acc
		- @b TRUE       switch spare sram area access right to CPU
		- @b FALSE      switch spare sram area access right to NAND controller

	@return
		- void
*/
void nand_host_set_cpu_access_spare(struct drv_nand_dev_info *info,
							uint32_t bcpu_Acc)
{
	union T_NAND_SRAM_ACCESS_REG  RegSramAcc = {0x00000000};
	uint8_t i;

	RegSramAcc.Reg = NAND_GETREG(info, NAND_SRAM_ACCESS_REG_OFS);

	RegSramAcc.Bit.SPARE_ACC = bcpu_Acc;

	NAND_SETREG(info, NAND_SRAM_ACCESS_REG_OFS,RegSramAcc.Reg);

	//dummy read to delay 200ns for SRAM ready
	if(bcpu_Acc == TRUE) {
		for(i = 0; i < SRAMRDY_DELAY; i++)
			RegSramAcc.Reg = NAND_GETREG(info, NAND_SRAM_ACCESS_REG_OFS);
	}
}

/*
	NAND Reed Solomon ECC correction routing

	ECC correction flow by using reed solomon method,
	including correct error in spare area first 6 bytes

	@param[in]  Buf         Buffer to correct by RS correct routing
	@param[in]  section     section to be corrected

	@return
		- @b E_OK       success
		- @b E_CTX      uncorrect error
*/
int nand_host_correct_reedsolomon_ecc(struct drv_nand_dev_info *info,
					uint8_t * Buf, uint32_t uiSection)
{
	int32_t ret = 0;
	uint32_t i, j, shifts;
	uint32_t hrdata, rs_sts, rs_err, err_reg, err_addr, err_bit, mask_bit;

	nand_host_set_cpu_access_err(info, TRUE);

	rs_sts = NAND_GETREG(info, NAND_RSERR_STS0_REG_OFS);

	for(i = 0; i < uiSection; i++) {
		shifts = 4 * i;
		rs_err = (rs_sts >> shifts) & 0x7;
		if(rs_err == 0)
			continue;
		else if(rs_err == 5) {
			ret = E_CTX;
			continue;
		}

		for(j = 0; j < rs_err; j++) {

			err_reg = NAND_SEC0_EADDR0_REG_OFS + (16 * i) + (4 * j);

			hrdata = NAND_GETREG(info, (uintptr_t)err_reg);
			err_addr = hrdata & 0xFFFF;

			if(err_addr >= _512BYTE) {
				nand_host_set_cpu_access_spare(info, TRUE);
				err_addr -= _512BYTE;
				err_addr += (uint32_t)((CONFIG_SYS_NAND_BASE + NAND_SPARE00_REG_OFS) + (i * 16));
			}

			err_bit = (hrdata >> 16) & 0xFF;

			mask_bit = (err_addr & 0x03) * 8;

			if((hrdata & 0xFFFF) >= _512BYTE) {
				* (volatile uint32_t *)(uintptr_t) (err_addr & 0xFFFFFFFC) ^= (err_bit << mask_bit);
				nand_host_set_cpu_access_spare(info, FALSE);
			} else
				Buf[err_addr+ i*_512BYTE] ^= (err_bit);   // correct bit
		}
	}

	nand_host_set_cpu_access_err(info, FALSE);

	return ret;
}

/*
	NAND secondary ECC correction routing

	ECC correction flow by using secondary ECC method,

	@param[in]  Buf         Buffer to correct by RS correct routing
	@param[in]  section     section to be corrected

	@return
		- @b E_OK       success
		- @b E_CTX      uncorrect error
*/
int nand_host_correct_secondary_ecc(struct drv_nand_dev_info *info,
					uint32_t uiSection)
{
	int32_t ret = E_OK;
	uint32_t i, shifts;
	uint32_t hrdata, ham_sts, ham_err, err_reg, err_addr, err_bit;
	uint32_t mask_bit;

	nand_host_set_cpu_access_err(info, TRUE);

	ham_sts = NAND_GETREG(info, NAND_SECONDARY_ECC_STS_REG_OFS);

	for(i = 0; i < uiSection; i++) {   // 8 sections
		shifts = 2 * i;
		ham_err = (ham_sts >> shifts) & 0x3;
		if(ham_err == 0)
			continue;
		else if(ham_err == 2) {
			ret = E_CTX;
			continue;
		} else if(ham_err == 3) {
			ret = E_CTX;
			continue;
		}
		err_reg = NAND_SEC0_EADDR0_REG_OFS + (16 * i) + 4;

		hrdata = NAND_GETREG(info, (uintptr_t)err_reg);

		err_addr = hrdata & _ECC_SEC0_SECONDARY_ERRBYTE;
		err_bit = (hrdata & _ECC_SEC0_SECONDARY_ERRBIT)>>_ECC_SEC0_SECONDARY_BITSFT;

		if(err_addr >= _512BYTE) {
			nand_host_set_cpu_access_spare(info, TRUE);
			err_addr -= _512BYTE;
			err_addr += (uint32_t)(CONFIG_SYS_NAND_BASE + NAND_SPARE00_REG_OFS + i * 16);
			mask_bit = (err_addr & 0x03) * 8;

			* (volatile uint32_t *)(uintptr_t) (err_addr & 0xFFFFFFFC) ^= ((1<<err_bit) << mask_bit);

			nand_host_set_cpu_access_spare(info, FALSE);
		} else {
			printf("Secondary ECC should occurred > section size[0x%08x]\r\n", _512BYTE);
			ret = E_SYS;
			break;
		}
	}

	nand_host_set_cpu_access_err(info, FALSE);

	return ret;
}

/*
	NAND BCH ECC correction routing

	ECC correction flow by using BCH method
	including correct error in spare area first 12 bytes

	@param[in]  Buf         Buffer to correct by RS correct routing
	@param[in]  section     section to be corrected

	@return
		- @b E_OK       success
		- @b E_CTX      uncorrect error
*/
UINT32 gNANDSpareAlignSel = 1;
int nand_host_correct_bch_ecc(struct drv_nand_dev_info *info,
					uint8_t * Buf, uint32_t uiSection)
{
	int32_t ret = 0;
	uint32_t i, j, shifts;
	uint32_t hrdata, bch_sts, bch_err, err_reg, err_addr, err_bit, mask_bit;

	nand_host_set_cpu_access_err(info, TRUE);

	bch_sts = NAND_GETREG(info, NAND_BCHERR_STS0_REG_OFS);
	NAND_DEBUG_MSG("    BCH sts: 0x%08x \r\n", bch_sts);

	for(i = 0; i < uiSection; i++) {
		shifts = 4 * i;
		bch_err = (bch_sts >> shifts) & 0xf;
		if (bch_err == 0) {
			NAND_DEBUG_MSG("    no error\r\n");
			continue;
		} else if(bch_err == 9) {
			NAND_DEBUG_MSG("    uncorrect ecc error\r\n");
			ret = E_CTX;
			continue;
		}
		NAND_DEBUG_MSG("    %d-bit errors were detected \r\n", (int)bch_err);

		for(j = 0; j < bch_err; j++) {

			err_reg = NAND_SEC0_EADDR0_REG_OFS + (32 * i) + (4 * j);

			hrdata = NAND_GETREG(info, (uintptr_t)err_reg);

			err_addr = hrdata & 0xFFFF;
			err_bit = (hrdata >> 16) & 0xFF;
			err_addr -= _512BYTE*i;
			mask_bit = (err_addr & 0x03) * 8;	// u8 to u32

			if (err_addr >= _512BYTE) {
				// error bit in the spare area
				nand_host_set_cpu_access_spare(info, TRUE);

				err_addr += ((CONFIG_SYS_NAND_BASE + NAND_SPARE00_REG_OFS) - (0x200*(uiSection-i)));
				if (!gNANDSpareAlignSel && ((err_addr - i*0x10)&0xFF) > 0x2) {
					// no align, user area [0~2], ecc area[3~15]
					NAND_DEBUG_MSG("      sec[%d]BCH error bit[%03d][b%02x] (SpareEcc), skip\r\n", i, (err_addr&0x3FF), err_bit);
					nand_host_set_cpu_access_spare(info, FALSE);
					continue;
				} else if (gNANDSpareAlignSel){
					if (uiSection == 4) {
						// 2K page + align, Byte 0~11
						if ((err_addr&0xFF) > 0xB) {
							NAND_DEBUG_MSG("      sec[%d]BCH error bit[%03d][b%02x] (SpareEcc, B12~63), skip\r\n", i, err_addr, err_bit);
							nand_host_set_cpu_access_spare(info, FALSE);
							continue;
						}
					} else {
						// 4K page + align, Byte 0~23
						if ((err_addr&0xFF) > 0x17) {
							NAND_DEBUG_MSG("      sec[%d]BCH error bit[%03d][b%02x] (SpareEcc, B24~127), skip\r\n", i, err_addr, err_bit);
							nand_host_set_cpu_access_spare(info, FALSE);
							continue;
						}
					}
				}
				NAND_DEBUG_MSG("      sec[%d]BCH error bit[%03d][b%02x] (SpareUser)\r\n", i, err_addr, err_bit);

			} else {
				NAND_DEBUG_MSG("      sec[%d]BCH error bit[%03d][b%02x]\r\n", i, err_addr, err_bit);
			}

			if (err_addr >= _512BYTE) {
				* (volatile uint32_t *)(uintptr_t)(err_addr & 0xFFFFFFFC) ^= (err_bit << mask_bit);
				NAND_DEBUG_MSG("        verify spare[0x%08x]:0x%x\r\n", (err_addr & 0xFFFFFFFC), *(volatile uint32_t *)(uintptr_t)(err_addr & 0xFFFFFFFC));
				nand_host_set_cpu_access_spare(info, FALSE);
			} else {
				Buf[err_addr + i * _512BYTE] ^= (err_bit); // correct bit
			}
		}
	}

	nand_host_set_cpu_access_err(info, FALSE);

	return ret;
}

/*
	Reset NAND dll delay module

	Issue SW reset of NAND dll delay module

	@return void
*/
void nand_dll_reset(void)
{
	union T_NAND_DLL_PHASE_DLY_REG1 phy_dly;

	phy_dly.Reg = NAND_GETREG(NULL, NAND_DLL_PHASE_DLY_REG1_OFS);
	phy_dly.Bit.PHY_SW_RESET = 1;
	NAND_SETREG(NULL, NAND_DLL_PHASE_DLY_REG1_OFS, phy_dly.Reg);

	do {
		phy_dly.Reg = NAND_GETREG(NULL, NAND_DLL_PHASE_DLY_REG1_OFS);
	} while(phy_dly.Bit.PHY_SW_RESET == 1);
}

/*
	Set NAND PHY SRC Clock

	Set phy clock source

	@return void
*/
void nand_phy_config(struct drv_nand_dev_info *info)
{
	union T_NAND_DLL_PHASE_DLY_REG1 phy_dly;
	union T_NAND_DLL_PHASE_DLY_REG2 phy_dly2;
	union T_NAND_MODULE0_REG RegMdule0 = {0};
	u32 chip_id = nvt_get_chip_id();

	phy_dly.Reg = NAND_GETREG(NULL, NAND_DLL_PHASE_DLY_REG1_OFS);
	phy_dly2.Reg = NAND_GETREG(NULL, NAND_DLL_PHASE_DLY_REG2_OFS);
	RegMdule0.Reg = NAND_GETREG(NULL, NAND_MODULE0_REG_OFS);

	phy_dly.Bit.PHY_SAMPCLK_INV = 1;

	if ((chip_id == CHIP_NA51055) || (chip_id == CHIP_NA51084)) {
		if (info->ops_freq > 96000000) {
			if (RegMdule0.Bit.SPI_FLASH_TYPE) {
				phy_dly.Bit.PHY_SRC_CLK_SEL = 1;
				phy_dly2.Bit.INDLY_SEL = 0x3F;
			} else {
				phy_dly.Bit.PHY_SRC_CLK_SEL = 0;
				phy_dly2.Bit.INDLY_SEL = 0x20;
			}
		} else {
			phy_dly.Bit.PHY_SRC_CLK_SEL = 0;
			phy_dly2.Bit.INDLY_SEL = 0x0;
		}
	} else if (chip_id == CHIP_NA51089) {
		phy_dly.Bit.PHY_SRC_CLK_SEL = 0;
		phy_dly.Bit.PHY_PAD_CLK_SEL = 1;
		phy_dly2.Bit.INDLY_SEL = 0x5;
	} else if (chip_id == CHIP_NA51103) {
		phy_dly.Bit.PHY_SRC_CLK_SEL = 0;
		phy_dly.Bit.PHY_PAD_CLK_SEL = 1;
		phy_dly2.Bit.INDLY_SEL = 0x0;
	} else if ((chip_id == CHIP_NA51102) || (chip_id == CHIP_NS02201)) {
		phy_dly.Bit.PHY_SRC_CLK_SEL = 0;
		phy_dly.Bit.PHY_PAD_CLK_SEL = 1;
		if (info->ops_freq < 24000000) {
			phy_dly.Bit.PHY_SAMPCLK_INV = 0;
			phy_dly2.Bit.INDLY_SEL = 0x0;
		} else if (info->ops_freq <= 80000000) {
			phy_dly2.Bit.INDLY_SEL = 0x5;
		} else {	// ~ 120MHz
			phy_dly2.Bit.INDLY_SEL = 0x1E;
		}
	} else if ((chip_id == CHIP_NS02301) || (chip_id == CHIP_NS02302)) {
		phy_dly2.Bit.INDLY_SEL = 0x0;

		if (info->ops_freq <= 120000000) {
			phy_dly2.Bit.OUTDLY_SEL = 6;
		} else {
			phy_dly2.Bit.OUTDLY_SEL = 0;
		}
	} else if (chip_id == CHIP_NS02402) {
		/* When the frequency >= 120 MHz, indly needs to training. */
		phy_dly2.Bit.INDLY_SEL = 0x0;

		if (info->flash_info->dtr_en == 1) {
			if (info->ops_freq >= 48000000) {
				phy_dly2.Bit.OUTDLY_SEL = 0xF;
			} else {
				phy_dly2.Bit.OUTDLY_SEL = 0;
			}
		} else {
			if (info->ops_freq >= 48000000) {
				phy_dly2.Bit.OUTDLY_SEL = 0xA;
			} else {
				phy_dly2.Bit.OUTDLY_SEL = 0;
			}
		}
	} else {
		phy_dly.Bit.PHY_SRC_CLK_SEL = 0;
		phy_dly.Bit.PHY_PAD_CLK_SEL = 1;
		if (info->ops_freq >= 80000000) {
			phy_dly2.Bit.INDLY_SEL = 0x28;
		} else {
			phy_dly2.Bit.INDLY_SEL = 0x8;
		}
	}

	NAND_SETREG(NULL, NAND_DLL_PHASE_DLY_REG2_OFS, phy_dly2.Reg);
	NAND_SETREG(NULL, NAND_DLL_PHASE_DLY_REG1_OFS, phy_dly.Reg);
}

static void nvt_parse_frequency(u32 *freq, NAND_TYPE_SEL type)
{
	/* Enable it after dts parsing ready*/
	int nodeoffset;
	u32 *cell = NULL;

	if (type == NANDCTRL_SPI_NOR_TYPE) {
		nodeoffset = fdt_path_offset((const void*)nvt_fdt_buffer, "/nor");
	} else {
		nodeoffset = fdt_path_offset((const void *)nvt_fdt_buffer, "/nand");
	}

	cell = (u32 *)fdt_getprop((const void *)nvt_fdt_buffer, nodeoffset, "clock-frequency", NULL);

	if (cell > 0) {
		*freq = __be32_to_cpu(cell[0]);
	} else {
		*freq = 12000000;
	}
}

int nand_pad_reset(struct drv_nand_dev_info *info, NAND_TYPE_SEL type)
{
	uint32_t reg, clk_div, freq = 0x0;
	u32 *cell = NULL;
	int len;
	u32 flash_pinmux = 0x03;
	u32 fdt_pinmux = 0x0;

	int nodeoffset;

	info->chip_id = nvt_get_chip_id();
	NAND_DBG_GLOBAL("dbg CHID: 0x%x\r\n", info->chip_id);

	/* Get fdt pinmux config */
	do {
		nodeoffset = fdt_path_offset((const void *)nvt_fdt_buffer, "/top/nand");
		if (nodeoffset < 0) {
			printf("%s(%d) nodeoffset < 0\n", __func__, __LINE__);
			printf("%s: path %s not found\n", __func__, "/top/nand");
			break;
		}

		cell = (u32 *)fdt_getprop((const void *)nvt_fdt_buffer, nodeoffset, "pinmux", &len);
		if (len == 0) {
			printf("%s(%d) len = 0\n", __func__, __LINE__);
			break;
		}
		if (cell > 0) {
			fdt_pinmux = __be32_to_cpu(cell[0]);
			printf("%s: spi flash pinmux 0x%x\r\n", __func__, fdt_pinmux);
		} else {
			printf("%s: get spi flash pinmux fail \r\n", __func__);
		}
	} while (0);

	NAND_DBG_GLOBAL("dbg nand base addr: 0x%p\r\n", (void *)CONFIG_SYS_NAND_BASE);
	NAND_DBG_GLOBAL("dbg cg base addr: 0x%p\r\n", (void *)IOADDR_CG_REG_BASE);

	/* Config NAND module CLKEN */
	reg = INW(IOADDR_CG_REG_BASE + 0x74);
	if ((info->chip_id == CHIP_NS02301) || (info->chip_id == CHIP_NS02302) || \
		(info->chip_id == CHIP_NS02401) || (info->chip_id == CHIP_NS02402) || \
		(info->chip_id == CHIP_NA51055) || (info->chip_id == CHIP_NA51084) || \
		(info->chip_id == CHIP_NA51089)) {
		reg |= (1);
	} else {
		reg |= (1 << 10);
	}
	NAND_DBG_GLOBAL("dbg clken: 0x%x\r\n", reg);
	OUTW(IOADDR_CG_REG_BASE + 0x74, reg);

	/* Config NAND module RSTN */
	if ((info->chip_id == CHIP_NA51055) || (info->chip_id == CHIP_NA51084) || \
		(info->chip_id == CHIP_NS02301) || (info->chip_id == CHIP_NA51089)) {
		reg = INW(IOADDR_CG_REG_BASE + 0x84);
		reg |= (1);
		NAND_DBG_GLOBAL("dbg rstn: 0x%x\r\n", reg);
		OUTW(IOADDR_CG_REG_BASE + 0x84, reg);
	} else if (info->chip_id == CHIP_NA51103) {
		reg = INW(IOADDR_CG_REG_BASE + 0x98);
		reg |= (0x2);
		OUTW(IOADDR_CG_REG_BASE + 0x98, reg);
	} else {
		reg = INW(IOADDR_CG_REG_BASE + 0x94);
		if ((info->chip_id == CHIP_NS02302) || (info->chip_id == CHIP_NS02401) || \
			(info->chip_id == CHIP_NS02402)) {
			reg |= (1);
		} else {
			reg |= (1 << 10);
		}
		NAND_DBG_GLOBAL("dbg rstn: 0x%x\r\n", reg);
		OUTW(IOADDR_CG_REG_BASE + 0x94, reg);
	}

	//NAND_SETREG(info, NAND_TIME0_REG_OFS, 0x06002222);
	//NAND_SETREG(info, NAND_TIME1_REG_OFS, 0x7f0f);

	/* Need use PinCtrl framework */
	reg = INW(IOADDR_TOP_REG_BASE + 0x4);
	if ((info->chip_id == CHIP_NS02302) || (info->chip_id == CHIP_NS02401) || \
		(info->chip_id == CHIP_NS02402)) {
		reg &= ~0x3;
		if ((fdt_pinmux&0x3) == 0x1) {          // PIN_NAND_CFG_NAND_1, Quad mode
			reg |= 0x1;
		} else if ((fdt_pinmux&0x3) == 0x2) {   // PIN_NAND_CFG_NAND_2, Octal mode
			reg |= 0x2;
		} else {
			printf("%s: nand fdt_pinmux 0x%x not support\r\n", __func__, fdt_pinmux);
		}
		if (fdt_pinmux & 0x4) { // PIN_NAND_CFG_NAND_CS1
			reg |= 0x4;
		}
		OUTW(IOADDR_TOP_REG_BASE + 0x4, reg);
		NAND_DBG_GLOBAL("dbg pinmux: 0x%x\r\n", reg);

		reg = INW(IOADDR_TOP_REG_BASE + 0xA0);
		if ((fdt_pinmux&0x3) == 0x1) {         // PIN_NAND_CFG_NAND_1, Quad mode
			if (info->chip_id == CHIP_NS02401)
				reg &= ~(0x0000003F);
			else
				reg &= ~(0x0000090F);
		} else if ((fdt_pinmux&0x3) == 0x2) {  // PIN_NAND_CFG_NAND_2, Octal mode
			if (info->chip_id == CHIP_NS02401)
				reg &= ~(0x00003C7F);
			else
				reg &= ~(0x00000DFF);
		}
		if (fdt_pinmux & 0x4) {                // PIN_NAND_CFG_NAND_CS1
			reg &= ~(0x200);                   // CS1
		}
		OUTW(IOADDR_TOP_REG_BASE + 0xA0, reg);
	} else if (info->chip_id == CHIP_NS02301) {
		reg &= ~0xC;
		if ((fdt_pinmux&0x3) == 0x1) {          // PIN_NAND_CFG_NAND_1, Quad mode
			reg |= 0x4;
		} else if ((fdt_pinmux&0x3) == 0x2) {   // PIN_NAND_CFG_NAND_2, Octal mode
			reg |= 0x8;
		} else {
			printf("%s: nand fdt_pinmux 0x%x not support\r\n", __func__, fdt_pinmux);
		}
		if (fdt_pinmux & 0x4) { // PIN_NAND_CFG_NAND_CS1
			reg |= 0x40;
		}
		OUTW(IOADDR_TOP_REG_BASE + 0x4, reg);
		NAND_DBG_GLOBAL("dbg pinmux: 0x%x\r\n", reg);

		reg = INW(IOADDR_TOP_REG_BASE + 0xA0);
		if ((fdt_pinmux&0x3) == 0x1) {         // PIN_NAND_CFG_NAND_1, Quad mode
			reg &= ~(0x0000140F);              // 4-bit
		} else if ((fdt_pinmux&0x3) == 0x2) {  // PIN_NAND_CFG_NAND_2, Octal mode
			reg &= ~(0x000015FF);              // 8-bit + DQS
		}
		if (fdt_pinmux & 0x4) {                // PIN_NAND_CFG_NAND_CS1
			reg &= ~(0x200);                   // CS1
		}
		OUTW(IOADDR_TOP_REG_BASE + 0xA0, reg);
	} else if (info->chip_id == CHIP_NA51103) {
		if (fdt_pinmux & 0x20) {
			flash_pinmux |= (1 << 2);
		}
		reg |= (flash_pinmux & 0xF);

		OUTW(IOADDR_TOP_REG_BASE + 0x4, reg);
		NAND_DBG_GLOBAL("dbg pinmux: 0x%x\r\n", reg);

		reg = INW(IOADDR_TOP_REG_BASE + 0xA0);
		reg &= ~(0x3F);
		if (flash_pinmux & (1 << 2)) {
			reg &= ~(1 << 6);
		}
		OUTW(IOADDR_TOP_REG_BASE + 0xA0, reg);
	} else if ((info->chip_id == CHIP_NA51055) || (info->chip_id == CHIP_NA51089) || \
				(info->chip_id == CHIP_NA51084)) {
		if (fdt_pinmux & 0x1) {
			flash_pinmux = (1 << 13);
		}

		reg |= flash_pinmux;
		OUTW(IOADDR_TOP_REG_BASE + 0x4, reg);
		NAND_DBG_GLOBAL("dbg pinmux: 0x%x\r\n", reg);

		if (flash_pinmux & (1 << 13)) {
			reg = INW(IOADDR_TOP_REG_BASE + 0xA0);
			reg &= ~(0x0000050F);
			OUTW(IOADDR_TOP_REG_BASE + 0xA0, reg);
			NAND_DBG_GLOBAL("dbg cgpio: 0x%x\r\n", reg);
		}
	} else {
		if (fdt_pinmux & 0x2) {                // PIN_NAND_CFG_NAND_CS1 = 0x2
			flash_pinmux |= (1 << 2);
		}
		reg |= flash_pinmux;

		OUTW(IOADDR_TOP_REG_BASE + 0x4, reg);
		NAND_DBG_GLOBAL("dbg pinmux: 0x%x\r\n", reg);

		reg = INW(IOADDR_TOP_REG_BASE + 0xA0);
		if (fdt_pinmux & 0x2) {                // PIN_NAND_CFG_NAND_CS1 = 0x2
			reg &= ~(0x0000070F);
		} else {
			reg &= ~(0x0000030F);
		}
		OUTW(IOADDR_TOP_REG_BASE + 0xA0, reg);
	}

	/* reset NAND Config NAND module configuration */

	/* Config clock div */
	nvt_parse_frequency(&freq, type);
	if (info->chip_id == CHIP_NA51090 || info->chip_id == CHIP_NA51103) {
		if (freq > 80000000) {
			printf("freq %d out of range, fix to 80MHz\n", freq);
			freq = 80000000;
		}
	}
	clk_div = NVT_FLASH_SOURCE_CLK / freq - 1;
	NAND_DBG_GLOBAL("dbg clksrc %d \n", NVT_FLASH_SOURCE_CLK);
	NAND_DBG_GLOBAL("dbg freq %d, div=0x%x \n", freq, clk_div);

	if (info->chip_id == CHIP_NA51090 || info->chip_id == CHIP_NA51102 || \
		info->chip_id == CHIP_NS02201 || info->chip_id == CHIP_NA51103) {
		reg = INW(IOADDR_CG_REG_BASE + 0x48);
		reg &= ~(0x3F0000);
		reg |= (clk_div << 16);
		OUTW(IOADDR_CG_REG_BASE + 0x48, reg);
	} else if ( (info->chip_id == CHIP_NA51055) || (info->chip_id == CHIP_NA51089) || \
				(info->chip_id == CHIP_NS02301) || (info->chip_id == CHIP_NS02302) || \
				(info->chip_id == CHIP_NS02402) || (info->chip_id == CHIP_NA51084)) {
		reg = INW(IOADDR_CG_REG_BASE + 0x40);
		NAND_DBG_GLOBAL("get reg: 0x%x \n", reg);
		reg &= ~(0x3F000);
		reg |= (clk_div << 12);
		OUTW(IOADDR_CG_REG_BASE + 0x40, reg);
		NAND_DBG_GLOBAL("dbg clkdiv: 0x%x \n", reg);
	} else if (info->chip_id == CHIP_NS02401) {
		reg = INW(IOADDR_CG_REG_BASE + 0x44);
		NAND_DBG_GLOBAL("get reg: 0x%x \n", reg);
		reg &= ~(0x3F0000);
		reg |= (clk_div << 16);
		OUTW(IOADDR_CG_REG_BASE + 0x44, reg);
		NAND_DBG_GLOBAL("dbg clkdiv: 0x%x \n", reg);
	} else {
		printf("clkdiv: skip (todo) \n");
	}

	/* Config driving */
	if ((info->chip_id == CHIP_NA51090)) {
		reg = INW(IOADDR_PAD_REG_BASE + 0x50);
		reg &= ~(0xFFFFFFF);

		if (freq <= 24000000) {
			reg |= 0x0111101;    // data,clk,cs = 6mA
		} else if (freq <= 60000000) {
			reg |= 0x0222202;    // data,clk = 9mA, cs = 6mA
		} else { // freq > 60MHz
			reg |= 0x0222203;    // data = 9mA, clk = 12mA, cs = 6mA
		}

		OUTW(IOADDR_PAD_REG_BASE + 0x50, reg);
	} else if (info->chip_id == CHIP_NA51103) {
		reg = INW(IOADDR_PAD_REG_BASE + 0x50);
		reg &= ~(0xFFFFFFF);

		if (freq <= 48000000) {
			reg |= 0x0000000;
		} else { // freq > 48MHz
			reg |= 0x0111101;    // clk&data = 8mA
		}

		OUTW(IOADDR_PAD_REG_BASE + 0x50, reg);

		OUTW(IOADDR_PAD_REG_BASE, 0x6AA9);

	} else if ((info->chip_id == CHIP_NA51102) || (info->chip_id == CHIP_NS02201)) {
		// D0 ~ D3
		reg = INW(IOADDR_PAD_REG_BASE + 0x100);
		reg &= ~(0xFFFF);
		if (freq < 24000000) {
			reg |= 0x4444;
		} else if (freq < 48000000) {
			reg |= 0x6666;
		} else { // freq >= 48MHz
			reg |= 0x8888;
		}
		OUTW(IOADDR_PAD_REG_BASE + 0x100, reg);

		// CLK, CS0&CS1
		reg = INW(IOADDR_PAD_REG_BASE + 0x104);
		reg &= ~(0xFFF);
		if (freq < 24000000) {
			reg |= 0x444;
		} else if (freq < 48000000) {
			reg |= 0x666;
		} else { // freq >= 48MHz
			reg |= 0x888;
		}
		OUTW(IOADDR_PAD_REG_BASE + 0x104, reg);
	} else if (info->chip_id == CHIP_NS02301) {
		reg = INW(IOADDR_PAD_REG_BASE + 0x24);

		// 3.3v SDR (TODO: 1.8v & DTR)
		reg &= ~(0x0333FFFF);

		if (freq < 48000000) {
			// CLK: 0x0, Data: 0x0, CS: 0x0
		} else if (freq < 96000000) {
			reg |= 0x1105555; // CLK: 0x1, Data: 0x1, CS: 0x1
		} else {
			reg |= 0x1205555; // CLK: 0x2, Data: 0x1, CS: 0x1
		}
		OUTW(IOADDR_PAD_REG_BASE + 0x24, reg);
	} else if (info->chip_id == CHIP_NS02302) {
		UINT32 reg1;

		reg = INW(IOADDR_PAD_REG_BASE + 0x100);		// Data 0~7
		reg1 = INW(IOADDR_PAD_REG_BASE + 0x104);	// clk, cs1, dqs, cs0

		// 3.3v SDR (TODO: 1.8v & DTR)
		reg &= ~(0xFFFFFFFF);
		reg1 &= ~(0x0000FFFF);

		if (freq < 48000000) {
			// CLK: 0x0, Data: 0x0, CS: 0x0
		} else if (freq < 96000000) {
			reg |= 0x11111111;	// Data: 0x1
			reg1 |= 0x1111;		// CLK: 0x1, CS: 0x1
		} else {
			reg |= 0x11111111;	// Data: 0x1
			reg1 |= 0x1112;		// CLK: 0x2, CS: 0x1
		}
		OUTW(IOADDR_PAD_REG_BASE + 0x100, reg);
		OUTW(IOADDR_PAD_REG_BASE + 0x104, reg1);
	} else if (info->chip_id == CHIP_NS02402) {
		UINT32 reg1;

		reg = INW(IOADDR_PAD_REG_BASE + 0x100);		// Data 7~0
		reg1 = INW(IOADDR_PAD_REG_BASE + 0x104);	// cs0, dqs, cs1, clk

		if ((fdt_pinmux&0x3) == 0x2) {
			reg &= ~(0xFFFFFFFF);	// 8-bit
			reg1 &= ~(0x00000F00);	// dqs
		} else {
			reg &= ~(0x0000FFFF);	// 4-bit
		}
		if (fdt_pinmux & 0x4) {
			reg1 &= ~(0x000000F0);	// cs1
		}
		reg1 &= ~(0x0000F00F);		// cs0, clk

		if (freq >= 120000000) {
			if (info->flash_info->dtr_en == 1) {
				// clk = 4, data = 3, cs = 2, DTR mode
				if ((fdt_pinmux&0x3) == 0x2) {
					reg |= 0x33333333;	// 8-bit
					reg1 |= 0x2304;		// cs0, dqs, na, clk
				} else {
					reg |= 0x00003333;	// 4-bit
					reg1 |= 0x2004;		// cs0, na, na, clk
				}
				if (fdt_pinmux & 0x4) {
					reg1 |= 0x0020;		// cs1
				}
			} else {
				// clk = 4, data = 2, cs = 2, SDR mode
				if ((fdt_pinmux&0x3) == 0x2) {
					reg |= 0x22222222;	// 8-bit
					reg1 |= 0x2204;		// cs0, dqs, na, clk
				} else {
					reg |= 0x00002222;	// 4-bit
					reg1 |= 0x2004;		// cs0, na, na, clk
				}
				if (fdt_pinmux & 0x4) {
					reg1 |= 0x0020;		// cs1
				}
			}
		} else if (freq >= 48000000) {
			// clk = 3, data = 2, cs = 2
			if ((fdt_pinmux&0x3) == 0x2) {
				reg |= 0x22222222;	// 8-bit
				reg1 |= 0x2203;		// cs0, dqs, na, clk
			} else {
				reg |= 0x00002222;	// 4-bit
				reg1 |= 0x2003;		// cs0, na, na, clk
			}
			if (fdt_pinmux & 0x4) {
				reg1 |= 0x0020;		// cs1
			}
		} else {
			// clk = 2, data = 1, cs = 1
			if ((fdt_pinmux&0x3) == 0x2) {
				reg |= 0x11111111;	// 8-bit
				reg1 |= 0x1102;		// cs0, dqs, na, clk
			} else {
				reg |= 0x00001111;	// 4-bit
				reg1 |= 0x1002;		// cs0, na, na, clk
			}
			if (fdt_pinmux & 0x4) {
				reg1 |= 0x0010;		// cs1
			}
		}

		NAND_DBG_GLOBAL("dbg pad driving: 0x100[0x%08x], 0x104[0x%08x] \n", reg, reg1);
		OUTW(IOADDR_PAD_REG_BASE + 0x100, reg);
		OUTW(IOADDR_PAD_REG_BASE + 0x104, reg1);
	} else if ((info->chip_id == CHIP_NA51055) || (info->chip_id == CHIP_NA51089) || \
				(info->chip_id == CHIP_NA51084)) {
		reg = INW(IOADDR_PAD_REG_BASE + 0x40);

		reg &= ~(0x3300FF);	// MC0-3, 8, 10
		reg |= 0x110055;	// fixed driving = 0x1
		OUTW(IOADDR_PAD_REG_BASE + 0x40, reg);
		NAND_DBG_GLOBAL("dbg pad driving: 0x%x \n", reg);
	} else {
		/* Release SRAM */
		printf("Release sram: skip (TODO)\r\n");
		//reg = INW(IOADDR_TOP_REG_BASE + 0x1000);
		//reg &= ~(0x20000);
		//OUTW(IOADDR_TOP_REG_BASE + 0x1000, reg);

		/* Config driving */
		printf("pad driving: skip (TODO)\r\n");
	}

	if (type == NANDCTRL_SPI_NAND_TYPE || type == NANDCTRL_SPI_NOR_TYPE) {
		nand_hostSetNandType(info, type);
	} else {
		printf("flash type[%d] no support \r\n", type);
		return -1;
	}

	nand_host_settiming2(info, 0x9F51);

	info->ops_freq = freq;

	return nand_cmd_reset(info);
}

/*
	Configure nand host page size

	This function configure controller of SPI NAND page size

	@param[OUT]
	@return
		- @b_MULTI_PAGE
		- @b_SINGLE_PAGE

*/
UINT32 nand_hostGetMultipageSelect(void)
{
	union T_NAND_CTRL0_REG  RegCtrl = {0};

	RegCtrl.Reg = NAND_GETREG(info, NAND_CTRL0_REG_OFS);
	return (RegCtrl.Reg & 0x00030000);
}
/*
	set nand type

	set nand type

	@return
		- @b E_NOSPT    Not support
		- @b E_OK
*/
int nand_hostSetNandType(struct drv_nand_dev_info *info, NAND_TYPE_SEL uiNandType)
{
	union T_NAND_MODULE0_REG RegMdule0 = {0};
	int uRet = E_OK;

	RegMdule0.Reg = NAND_GETREG(info, NAND_MODULE0_REG_OFS);

	switch(uiNandType) {
	case NANDCTRL_ONFI_NAND_TYPE:
		RegMdule0.Bit.NAND_TYPE = 0;
		RegMdule0.Bit.SPI_FLASH_TYPE = 0;
	break;

	case NANDCTRL_SPI_NAND_TYPE:
		RegMdule0.Bit.NAND_TYPE = 1;
		RegMdule0.Bit.SPI_FLASH_TYPE = 0;
		//RegMdule0.Bit.COL_ADDR = NAND_1COL_ADDR;
		//RegMdule0.Bit.ROW_ADD = NAND_1ROW_ADDR;
	break;

	case NANDCTRL_SPI_NOR_TYPE:
		RegMdule0.Bit.NAND_TYPE = 1;
		RegMdule0.Bit.SPI_FLASH_TYPE = 1;
		//RegMdule0.Bit.COL_ADDR = NAND_1COL_ADDR;
		//RegMdule0.Bit.ROW_ADD = NAND_1ROW_ADDR;
	break;

	default:
		return E_NOSPT;
	break;
	}

	info->flash_info->config_nand_type = uiNandType;
	NAND_SETREG(info, NAND_MODULE0_REG_OFS, RegMdule0.Reg);

	return uRet;
}

/*
    Set nand IO type (SDR/DTR)

    @return
        - @b E_NOSPT    Not support
        - @b E_OK
*/
int nand_hostSetDTRType(DTR_TYPE_SEL uiDTRType)
{
	union T_NAND_MODULE0_REG RegMdule0 = {0};
	int uRet = E_OK;

	RegMdule0.Reg = NAND_GETREG(info, NAND_MODULE0_REG_OFS);
	switch (uiDTRType) {
	case NANDCTRL_SDR_TYPE:
		RegMdule0.Bit.SPI_NOR_FLASH_OP_MODE = 0;
		RegMdule0.Bit.SPI_WRITE_DDR_CLK = 0;
		break;

	case NANDCTRL_DTR_TYPE:
		RegMdule0.Bit.SPI_NOR_FLASH_OP_MODE = 1;
		RegMdule0.Bit.SPI_WRITE_DDR_CLK = 1;
		break;

	default:
		return E_NOSPT;
		break;
	}

	NAND_SETREG(info, NAND_MODULE0_REG_OFS, RegMdule0.Reg);

	return uRet;
}


/*
	Configure nand host row and column address cycle

	This function configure controller row & column cycle

	@param[in]  uiCS        chip select
	@param[in]  uiRow       row     cycle
	@param[in]  uiColumn    column  cycle

	@return
		- @b E_SYS      Status fail
		- @b E_OK       Operation success

*/
int nand_hostSetupAddressCycle(struct drv_nand_dev_info *info, NAND_ADDRESS_CYCLE_CNT uiRow, NAND_ADDRESS_CYCLE_CNT uiCol)
{
	union T_NAND_MODULE0_REG  RegMdule0 = {0};

	RegMdule0.Reg = NAND_GETREG(info, NAND_MODULE0_REG_OFS);

	if(uiCol > NAND_3_ADDRESS_CYCLE_CNT && uiCol != NAND_NOT_CONFIGED_ADDRESS_CYCLE_CNT)
		return E_SYS;

	if(uiCol != NAND_NOT_CONFIGED_ADDRESS_CYCLE_CNT)
		RegMdule0.Bit.COL_ADDR = uiCol;

	if(uiRow != NAND_NOT_CONFIGED_ADDRESS_CYCLE_CNT)
		RegMdule0.Bit.ROW_ADD = uiRow;

	NAND_SETREG(info, NAND_MODULE0_REG_OFS, RegMdule0.Reg);

	return E_OK;
}

/*
	Configure nand host page size

	This function configure controller of SPI NAND page size

	@param[in]  uiCS        chip select
	@param[in]  uiRow       row     cycle
	@param[in]  uiColumn    column  cycle

	@return
		- @b E_SYS      Status fail
		- @b E_OK       Operation success

*/
int nand_hostSetupPageSize(struct drv_nand_dev_info *info, NAND_PAGE_SIZE uiPageSize)
{
	union T_NAND_MODULE0_REG RegMdule0 = {0};

	RegMdule0.Reg = NAND_GETREG(info, NAND_MODULE0_REG_OFS);

	if(uiPageSize == NAND_PAGE_SIZE_512 || uiPageSize == NAND_PAGE_SIZE_2048 || uiPageSize == NAND_PAGE_SIZE_4096) {
		RegMdule0.Bit._PAGE_SIZE = uiPageSize;
		NAND_SETREG(info, NAND_MODULE0_REG_OFS, RegMdule0.Reg);
		return E_OK;
	} else
		return E_SYS;
}



/*
	NAND controller enable polling bit match operation

	Enable bit value compare function.
	After invoke spi_enBitMatch(), it will send uiCmd to SPI device
	and continously read data from SPI device.
	Once bit position specified by uiBitPosition of read data becomes bWaitValue,
	SPI module will stop checking and raise a interrupt.
	Caller of spi_enBitMatch() can use spi_waitBitMatch() to wait this checking complete.

	@param[in] uiStatusValue    The check value
	@param[in] uiStatusMask     Indicates which bit of status value you want to check. 1 for comparison and 0 for not comparison

	@return
		- @b E_OK: success
		- @b Else: fail
*/
int nand_hostSetupStatusCheckBit(struct drv_nand_dev_info *info, u8 uiStatusMask, u8 uiStatusValue)
{
	union T_NAND_STATUS_CHECK_REG RegStsChk = {0};

	RegStsChk.Bit.STATUS_VALUE = uiStatusValue;
	RegStsChk.Bit.STATUS_MASK = uiStatusMask;

	NAND_SETREG(info, NAND_STATUS_CHECK_REG_OFS, RegStsChk.Reg);
	return E_OK;
}

/*
	Configure nand host page size

	This function configure controller of SPI NAND page size

	@param[OUT]
	@return
		- @b_MULTI_PAGE
		- @b_SINGLE_PAGE

*/
ER nand_hostSetSPIIORegister(struct drv_nand_dev_info *info, NAND_SPI_CS_POLARITY uiCSPol, NAND_SPI_BUS_WIDTH uiBusWidth, NAND_SPI_IO_ORDER uiIOOrder)
{
	union T_NAND_SPI_CFG_REG  RegIO = {0};

	RegIO.Reg = NAND_GETREG(info, NAND_SPI_CFG_REG_OFS);

	RegIO.Bit.SPI_CS_POL    = uiCSPol;
	RegIO.Bit.SPI_BS_WIDTH  = uiBusWidth;
	RegIO.Bit.SPI_IO_ORDER  = uiIOOrder;
	RegIO.Bit.SPI_PULL_WPHLD  = 0x1;

	NAND_SETREG(info, NAND_SPI_CFG_REG_OFS, RegIO.Reg);
	return E_OK;
}

/*
	Configure nand host chip select manual mode

	Configure nand host chip select manual mode

	@param[in] bModeSel    manual mode or auto mode
		- NAND_SPI_CS_AUTO_MODE     : manual mode(CS configure by user)
		- NAND_SPI_CS_MANUAL_MODE   : auto mode(CS configure by controller)

*/
ER nand_hostConfigChipSelOperationMode(struct drv_nand_dev_info *info, NAND_SPI_CS_OP_MODE bModeSel)
{
	union T_NAND_SPI_CFG_REG  cfgReg = {0};

	cfgReg.Reg = NAND_GETREG(info, NAND_SPI_CFG_REG_OFS);
	cfgReg.Bit.SPI_OPERATION_MODE   = bModeSel;

	NAND_SETREG(info, NAND_SPI_CFG_REG_OFS, cfgReg.Reg);
	return E_OK;
}

/*
	Configure nand host chip select manual mode

	Configure nand host chip select manual mode

	@param[in] bCSLevel    CS (chip select) level
		- NAND_SPI_CS_LOW    : CS set low
		- NAND_SPI_CS_HIGH   : CS set high

*/
ER nand_hostSetCSActive(struct drv_nand_dev_info *info, NAND_SPI_CS_LEVEL bCSLevel)
{
	union T_NAND_SPI_CFG_REG  cfgReg = {0};

	cfgReg.Reg = NAND_GETREG(info, NAND_SPI_CFG_REG_OFS);
	cfgReg.Bit.SPI_NAND_CS   = bCSLevel;

	NAND_SETREG(info, NAND_SPI_CFG_REG_OFS, cfgReg.Reg);
	return E_OK;
}



/*
	Configure nand host timing 2 configuration

	Configure nand host timing 2 configuration

	@param[in] bCSLevel    CS (chip select) level
		- NAND_SPI_CS_LOW    : CS set low
		- NAND_SPI_CS_HIGH   : CS set high

*/
void nand_host_settiming2(struct drv_nand_dev_info *info, u32 time)
{
	union T_NAND_TIME2_REG  time_reg = {0};

	time_reg.Reg = time;

	if ((nvt_get_chip_id() == CHIP_NA51103) && time_reg.Bit.TSLCH < 3) {
		printf("331 SPI NAND TSLCH need >= 0x3, force config as 0x3\n");
		time_reg.Bit.TSLCH = 0x3;
	}

	if (time_reg.Bit.TSHCH < 0x4) {
		printf("SPI NAND TSHCH need >= 0x4, force config as 0x4\n");
		time_reg.Bit.TSHCH = 0x4;
	}

	NAND_SETREG(info, NAND_TIME2_REG_OFS, time_reg.Reg);
}

/*
    Configure nand host spare size

    @param[in]  bSpareSizeSel
    			- @b NAND_SPI_SPARE_16_BYTE : section = 512 + 16 byte
    			- @b NAND_SPI_SPARE_32_BYTE : section = 512 + 32 byte

*/
void nand_host_set_spare_size_sel(struct drv_nand_dev_info *info, NAND_SPI_SPARE_SIZE_SEL bSpareSizeSel)
{
	union T_NAND_MODULE0_REG  cfgReg = {0};

	cfgReg.Reg = NAND_GETREG(info, NAND_MODULE0_REG_OFS);

	//gNANDSpareSizeSel = bSpareSizeSel;

	cfgReg.Bit.SPARE_32_SEL = bSpareSizeSel;
	NAND_SETREG(info, NAND_MODULE0_REG_OFS, cfgReg.Reg);
}

/*
    Configure nand host Spare Align

    @param[in]  bSpareAlignSel
				- @b NAND_SPI_SPARE_ALIGN_SECTION: 3-byte user data and  13-byte ecc code per section
					|      section-0      |      section-1      | ...
					|  user  |  ecc code  |  user  |  ecc code  | ...
					| 3-byte |  13-byte   | 3-byte |  13-byte   | ...

				- @b NAND_SPI_SPARE_ALIGN_PAGE: 12-byte user data and  52-byte ecc code per page
					|             in 2K page spare              |
					|      user      |         ecc code         |
					|    12-byte     |          52-byte         |

					|             in 4K page spare              |
					|      user      |         ecc code         |
					|    24-byte     |         108-byte         |

*/
void nand_host_set_spare_align_sel(struct drv_nand_dev_info *info, NAND_SPI_SPARE_ALIGN_SEL bSpareAlignSel)
{
	union T_NAND_MODULE0_REG  cfgReg = {0};

	cfgReg.Reg = NAND_GETREG(info, NAND_MODULE0_REG_OFS);

	//gNANDSpareAlignSel = SpareAlignSel;

	cfgReg.Bit.SPARE_ALIGN_SEL = bSpareAlignSel;
	NAND_SETREG(info, NAND_MODULE0_REG_OFS, cfgReg.Reg);
}

/*
    Configure nand host BCH ecc protect spare user area or not

    @param[in]  bSpareEccFree
    			- @b NAND_SPI_SPARE_ECC_FREE_DISABLE : spare user area is protected
    			- @b NAND_SPI_SPARE_ECC_FREE_ENABLE  : spare user area is not protected

*/
void nand_host_set_spare_eccfree(struct drv_nand_dev_info *info, NAND_SPI_SPARE_ECC_FREE bSpareEccFree)
{
	union T_NAND_ECC_FREE_REG  cfgReg = {0};

	cfgReg.Reg = NAND_GETREG(info, NAND_ECC_FREE_REG_OFS);

	//gNANDSpareSizeSel = bSpareSizeSel;

	cfgReg.Bit.ECC_FREE = bSpareEccFree;
	NAND_SETREG(info, NAND_ECC_FREE_REG_OFS, cfgReg.Reg);
}

/*
    Enable / Disable AXI CH

    Enable and Disable AXI channel of NAND controller

    @param[in]  uiEN     0: enable, 1: disable

    @return void
*/
void nand_host_axi_ch_disable(struct drv_nand_dev_info *info, BOOL uiEN)
{
	union T_NAND_AXI_REG0 axiReg = {0};

	axiReg.Reg = NAND_GETREG(info, NAND_AXI_REG0_OFS);

	axiReg.Bit.AXI_CH_DISABLE = uiEN;

	NAND_SETREG(info, NAND_AXI_REG0_OFS, axiReg.Reg);

	if (uiEN == FALSE) {
		while ((NAND_GETREG(info, NAND_AXI_REG0_OFS) & _FIFO_EN) != 0);
	}
}

void nand_host_set_clkdiv(struct drv_nand_dev_info *info, u32 div)
{
	UINT32 reg;

	if (info->chip_id == CHIP_NS02201 || \
		info->chip_id == CHIP_NA51103) {
		reg = INW(IOADDR_CG_REG_BASE + 0x48);
		reg &= ~(0x3F0000);
		reg |= (div << 16);
		OUTW(IOADDR_CG_REG_BASE + 0x48, reg);
	} else if ((info->chip_id == CHIP_NS02301) || \
				(info->chip_id == CHIP_NS02302) || \
				(info->chip_id == CHIP_NS02402)) {
		reg = INW(IOADDR_CG_REG_BASE + 0x40);
		reg &= ~(0x3F000);
		reg |= (div << 12);
		OUTW(IOADDR_CG_REG_BASE + 0x40, reg);
	} else {
		printf("%s: chip id[0x%x] not support \n", __func__, info->chip_id);
	}
}

int nand_host_create_edge_detect_pattern(struct drv_nand_dev_info *info, NAND_TYPE_SEL nandType, u8 *wBuf)
{
	union T_NAND_DLL_PHASE_DLY_REG1 phy_dly;
	UINT32 clk_div;
	UINT32 pageSize = info->flash_info->page_size;
	u32 pattern_addr;
	int ret = E_OK;
	u8 *rBuf;

	if (nandType == NANDCTRL_SPI_NOR_TYPE) {
		struct spi_flash *flash = malloc(sizeof(struct spi_flash));

		if (!flash) {
			printf("alloc spi_flash failed!\n");
			return -ENOMEM;
		}

		if (info->autok_addr && \
			((info->autok_addr & 0xFF) == 0) && \
			(info->flash_info->device_size>info->autok_addr)) {
			pattern_addr = info->autok_addr;
		} else {
			pattern_addr = (info->flash_info->device_size-0x1000); // default: last sector
		}

		NAND_AUTOK_MSG(info, "create tuning pattern addr: 0x%x\r\n", pattern_addr);

		if (info->flash_info->device_size > SPI_FLASH_16MB_BOUN) {
			flash->erase_opcode = FLASH_CMD_SECTOR_ERASE_4BYTE;
		} else {
			flash->erase_opcode = FLASH_CMD_SECTOR_ERASE;
		}
		NAND_AUTOK_DBG(info, "Erase cmd: 0x%x\r\n", flash->erase_opcode);

		// Write pattern to flash, switch to low speed mode
		clk_div = 39;	// src480/div(39+1)=12MHz
		nand_host_set_clkdiv(info, clk_div);
		NAND_AUTOK_DBG(info, "create tuning pattern, switch to 12MHz\r\n");

		// dll config
		phy_dly.Reg = NAND_GETREG(NULL, NAND_DLL_PHASE_DLY_REG1_OFS);
		phy_dly.Bit.PHY_SAMPCLK_INV = 0;
		phy_dly.Bit.PHY_SRC_CLK_SEL = 1;
		NAND_SETREG(NULL, NAND_DLL_PHASE_DLY_REG1_OFS, phy_dly.Reg);
		nand_dll_reset();

		// erase
		spinor_erase_sector(info, flash, pattern_addr);	// start row_addr: todo

		// write page
		spinor_program_operation(info, pattern_addr, pageSize, wBuf);

		// read back & verify pattern
		rBuf = info->data_buff;
		NAND_AUTOK_DBG(info, "rBuf: %p\r\n", rBuf);

		flush_dcache_range((unsigned long)rBuf, (unsigned long)((unsigned long)rBuf + pageSize));

		if (spinor_read_operation(info, pattern_addr, pageSize, rBuf)) {	// row addr = 0 (todo)
			printf("read pattern fail\r\n");
			ret = E_SYS;
		} else {
			if (memcmp((void *)wBuf, (void *)rBuf, pageSize) != 0) {
				ret = E_PAR;
			} else {
				NAND_AUTOK_DBG(info, "create tuning pattern pass\r\n");
			}
		}

		free(flash);

	} else if (nandType == NANDCTRL_SPI_NAND_TYPE) {
		if (info->autok_addr && \
			((info->autok_addr & (pageSize-1)) == 0) && \
			((info->flash_info->device_size*1024*1024) > info->autok_addr)) {
			pattern_addr = info->autok_addr;
		} else {
			pattern_addr = (info->flash_info->device_size*8 - 1) * 64 * pageSize; // default: last block
		}

		NAND_AUTOK_MSG(info, "create tuning pattern addr: 0x%x\r\n", pattern_addr);

		// Write pattern to flash, switch to low speed mode
		clk_div = 39;
		nand_host_set_clkdiv(info, clk_div);
		NAND_AUTOK_DBG(info, "create tuning pattern, switch to 12MHz\r\n");

		// dll config
		phy_dly.Reg = NAND_GETREG(NULL, NAND_DLL_PHASE_DLY_REG1_OFS);
		phy_dly.Bit.PHY_SAMPCLK_INV = 0;
		phy_dly.Bit.PHY_SRC_CLK_SEL = 1;
		NAND_SETREG(NULL, NAND_DLL_PHASE_DLY_REG1_OFS, phy_dly.Reg);
		nand_dll_reset();

		nand_cmd_erase_block(info, (pattern_addr/pageSize));
		info->buf_count = pageSize;
		nand_cmd_write_operation_single(info, wBuf, pattern_addr, 0);

		// read back & verify pattern
		rBuf = info->data_buff;
		nand_cmd_read_operation(info, rBuf, pattern_addr, 1);
		NAND_AUTOK_DBG(info, "rBuf: %p\r\n", rBuf);

		if (memcmp((void *)wBuf, (void *)rBuf, pageSize) != 0) {
			NAND_AUTOK_DBG(info, "create tuning pattern fail\r\n");
			ret = E_PAR;
		} else {
			NAND_AUTOK_DBG(info, "create tuning pattern pass\r\n");
		}

	} else {
		printf("flash type(%d) not support\r\n", nandType);
		ret = E_ID;
	}

	// Reset to real speed
	NAND_AUTOK_DBG(info, "reset to real speed %d Hz\r\n", info->ops_freq);
	clk_div = NVT_FLASH_SOURCE_CLK/info->ops_freq - 1;
	//clk_div = 192000000/info->ops_freq - 1;	// TODO: clksrc chk

	nand_host_set_clkdiv(info, clk_div);

	// dll reset
	nand_phy_config(info);
	nand_dll_reset();

	return ret;
}

void nand_host_indly_scan(struct drv_nand_dev_info *info, NAND_TYPE_SEL nandType, u8 *wBuf)
{
	union T_NAND_DLL_PHASE_DLY_REG1 phy_dly;
	union T_NAND_DLL_PHASE_DLY_REG2 phy_dly2;
	UINT32 pageSize, i;
	UINT32 PsZoneLen, PsZoneStart = 0, PsZoneEnd = 0;
	BOOL PsZoneStartFlg = 0, PsZoneEndFlg = 0, errFlg = 0;
	u8 *rBuf;
	u32 addr;

	pageSize = info->flash_info->page_size;

	if (nandType == NANDCTRL_SPI_NOR_TYPE) {
		if (info->autok_addr && \
			((info->autok_addr & 0xFF) == 0) && \
			(info->flash_info->device_size>info->autok_addr)) {
			addr = info->autok_addr;
		} else {
			addr = (info->flash_info->device_size-0x1000); // default: last sector
		}
	} else if (nandType == NANDCTRL_SPI_NAND_TYPE) {
		if (info->autok_addr && \
			((info->autok_addr & (pageSize-1)) == 0) && \
			((info->flash_info->device_size*1024*1024) > info->autok_addr)) {
			addr = info->autok_addr;
		} else {
			addr = (info->flash_info->device_size*8 - 1) * 64 * pageSize; // default: last block
		}
	} else {
		printf("%s err: nandType[%d] not support\r\n", __func__, nandType);
		return;
	}

	printf("autok tuning pattern addr: 0x%x\r\n", addr);

	rBuf = info->data_buff;
	NAND_AUTOK_DBG(info, "rBuf: %p\r\n", rBuf);

	phy_dly.Reg = NAND_GETREG(NULL, NAND_DLL_PHASE_DLY_REG1_OFS);
	phy_dly2.Reg = NAND_GETREG(NULL, NAND_DLL_PHASE_DLY_REG2_OFS);

	phy_dly.Bit.PHY_SAMPCLK_INV = 1;
	phy_dly.Bit.PHY_SRC_CLK_SEL = 0;
	phy_dly.Bit.PHY_PAD_CLK_SEL = 1;
	phy_dly2.Bit.INDLY_SEL = 0x0;
	NAND_AUTOK_MSG(info, "SAMPCLK_INV: 1, SRC_CLK: 0, PAD_CLK_SEL: 1\r\n");

	NAND_SETREG(NULL, NAND_DLL_PHASE_DLY_REG1_OFS, phy_dly.Reg);
	NAND_SETREG(NULL, NAND_DLL_PHASE_DLY_REG2_OFS, phy_dly2.Reg);
	nand_dll_reset();

	if (info->flash_info->dtr_read == NANDCTRL_DTR_TYPE) {
		nand_hostSetDTRType(NANDCTRL_DTR_TYPE);
	}

	NAND_AUTOK_MSG(info, "[NAND] Pass Zone scan: \r\n");

	// dump indly ruler
	if (info->autok_param & NAND_AUTOK_DUMP_LOG) {
		for (i = 0; i < NVT_INDLY_STEP; i++) {
			if (i%10 == 0) {
				NAND_AUTOK_MSG(info, "%1X", (i/10));
			} else {
				NAND_AUTOK_MSG(info, " ");
			}
		}
		NAND_AUTOK_MSG(info, "\r\n");
		for (i = 0; i < NVT_INDLY_STEP; i++) {
			NAND_AUTOK_MSG(info, "%1d", (i%10));
		}
		NAND_AUTOK_MSG(info, " (ruler)\r\n");
	}

	for (i = 0; i < NVT_INDLY_STEP; i++) {
		memset((void*)rBuf, 0x0, pageSize);	// clear rx buf

		phy_dly2.Reg = NAND_GETREG(NULL, NAND_DLL_PHASE_DLY_REG2_OFS);
		phy_dly2.Bit.INDLY_SEL = i;
		NAND_SETREG(NULL, NAND_DLL_PHASE_DLY_REG2_OFS, phy_dly2.Reg);
		nand_dll_reset();

		if (nandType == NANDCTRL_SPI_NOR_TYPE) {
			flush_dcache_range((unsigned long)rBuf, (unsigned long)((unsigned long)rBuf + pageSize));
			if (spinor_read_operation(info, addr, pageSize, rBuf)) {
				printf("read pattern fail\r\n");
				break;
			}
		} else if (nandType == NANDCTRL_SPI_NAND_TYPE) {
			if (nand_cmd_read_operation(info, rBuf, addr, 1)) {
				printf("read pattern fail\r\n");
				break;
			}
		}

		if (memcmp((void *)wBuf, (void *)rBuf, pageSize) != 0) {
			NAND_AUTOK_MSG(info, "x");
			if (PsZoneStartFlg & !PsZoneEndFlg) {
				PsZoneEnd = i-1;
				PsZoneEndFlg = 1;
			}
		} else {
			NAND_AUTOK_MSG(info, "o");
			if (!PsZoneStartFlg) {
				PsZoneStart = i;
				PsZoneStartFlg = 1;
			}
			if (PsZoneStartFlg && PsZoneEndFlg) {
				errFlg = 1;
			}
			if (PsZoneStartFlg && i == (NVT_INDLY_STEP-1)) {
				PsZoneEnd = i;
				PsZoneEndFlg = 1;
			}
		}
	}
	NAND_AUTOK_MSG(info, " (result)\r\n");

	if (errFlg) {
		printf("WRN: find multiple pass zone\r\n");
	}

	if (!PsZoneStartFlg) {
		printf("ERR: search pass zone fail\r\n");
		nand_phy_config(info);
		nand_dll_reset();
	} else if ((PsZoneStart > NVT_INDLY_STEP) || (PsZoneEnd > NVT_INDLY_STEP)) {
		printf("ERR: abnormal scan result, Start[%d] End[%d] \r\n", PsZoneStart, PsZoneEnd);
		nand_phy_config(info);
		nand_dll_reset();
	} else {
		PsZoneLen = PsZoneEnd - PsZoneStart;
		printf("[NAND]   ===> PassZone: Start[%d] End[%d] len[%d]\r\n", PsZoneStart, PsZoneEnd, PsZoneLen);
		if (PsZoneLen < 20) {
			printf("WRN: PassZonelen[%d] too short \r\n", PsZoneLen);
		}

		if (info->indly) {
			printf("[NAND]   ===> Dtsi indly check[%d]: ", info->indly);
			if ((info->indly > (PsZoneStart+10)) && \
				((info->indly + 10) < PsZoneEnd) && \
				(PsZoneLen>20)) {
				printf("(safe) \r\n");
			} else if ((info->indly > PsZoneStart) && \
						(info->indly < PsZoneEnd)) {
				printf("(danger) \r\n");
			} else {
				printf("(out of range) \r\n");
			}
		}

		printf("[NAND]   ===> Autok best indly: %d \r\n", PsZoneStart+(PsZoneLen/2));
		// Apply scan result
		if (info->autok_param & NAND_AUTOK_APPLY) {
			info->indly = PsZoneStart+(PsZoneLen/2);
			printf("[NAND]   ===> Apply Autok best indly\r\n");
		} else {
			printf("phy dll: reset to driver default setting\r\n");
			nand_phy_config(info);
			nand_dll_reset();
		}

		if (info->indly) {
			phy_dly2.Reg = NAND_GETREG(NULL, NAND_DLL_PHASE_DLY_REG2_OFS);
			phy_dly2.Bit.INDLY_SEL = info->indly;
			NAND_SETREG(NULL, NAND_DLL_PHASE_DLY_REG2_OFS, phy_dly2.Reg);
			nand_dll_reset();
		}
	}

	if (info->flash_info->dtr_read == NANDCTRL_DTR_TYPE) {
		// reset to SDR mode
		nand_hostSetDTRType(NANDCTRL_SDR_TYPE);
	}

}

int nand_host_1t_edge_detect(struct drv_nand_dev_info *info, NAND_TYPE_SEL nandType, u8 *rBuf)
{
	UINT32 timeOutCnt = 0, tmp, i, result, passZoneLen;
	UINT32 pageSize = info->flash_info->page_size;
	u32 addr;

	if (nandType == NANDCTRL_SPI_NOR_TYPE) {
		if (info->autok_addr && \
			((info->autok_addr & 0xFF) == 0) && \
			(info->flash_info->device_size>info->autok_addr)) {
			addr = info->autok_addr;
		} else {
			addr = (info->flash_info->device_size-0x1000); // default: last sector
		}
	} else {
		printf("NAND type: TODO \r\n");
		return E_PAR;
	}

	// clear indly
	NAND_SETREG(NULL, NAND_DLL_PHASE_DLY_REG2_OFS, NAND_GETREG(NULL, NAND_DLL_PHASE_DLY_REG2_OFS) & 0x00FFFFFF);

	if (info->ops_freq < 80000000) {
		printf("err: auto tuning 1t detect freq(%d) need >= 80MHz \r\n", info->ops_freq);
		return E_PAR;
	}

	// Step1: Auto 1T detect
	if (info->flash_info->dqs_en) {  // Set CAL_12T_SEL = 1 (1/2T)
		NAND_SETREG(NULL, NAND_DAT_PHASE_DLY_DLL_REG_OFS, \
			NAND_GETREG(NULL, NAND_DAT_PHASE_DLY_DLL_REG_OFS) | 0x2);
	}

	// Set CAL_EN
	NAND_SETREG(NULL, NAND_DAT_PHASE_DLY_DLL_REG_OFS, \
			NAND_GETREG(NULL, NAND_DAT_PHASE_DLY_DLL_REG_OFS) | 0x1);

	// dll reset
	nand_dll_reset();

	// Set clk pin is freerun
	tmp = NAND_GETREG(NULL, NAND_DLL_PHASE_DLY_REG1_OFS);
	NAND_SETREG(NULL, NAND_DLL_PHASE_DLY_REG1_OFS, tmp | (1<<30));

	while(timeOutCnt < 0x10000) {
		if ((NAND_GETREG(NULL, NAND_DAT_PHASE_DLY_DLL_REG_OFS)&0x1) == 0x0)
			break;
		timeOutCnt++;
	}

	// Reset PHY_DLL_CLK_ON
	tmp = NAND_GETREG(NULL, NAND_DLL_PHASE_DLY_REG1_OFS) & ~(1<<30);
	NAND_SETREG(NULL, NAND_DLL_PHASE_DLY_REG1_OFS, tmp);

	if (timeOutCnt >= 0x10000) {
		printf("scan 1t timeout");
		return E_PAR;
	}

	tmp = NAND_GETREG(NULL, NAND_DAT_PHASE_DLY_DLL_REG_OFS);
	// check scan result
	if (((tmp &0x1) == 0) && (tmp&0x10)) {
		printf("CAL scan done \r\n");

		// clear CAL_END & CAL_ERR
		if (tmp&0x20) {
			printf("CAL ERR \r\n");
		}
		NAND_SETREG(NULL, NAND_DAT_PHASE_DLY_DLL_REG_OFS, tmp);

		tmp = (tmp>>16)&0xFF;
		printf("CAL result = %d , dqs = %d\r\n", tmp, info->flash_info->dqs_en);

		// Set DATA_DLY_SEL
		i = NAND_GETREG(NULL, NAND_DAT_PHASE_DLY_DLL_REG_OFS)&~(0xFF<<24);
		NAND_SETREG(NULL, NAND_DAT_PHASE_DLY_DLL_REG_OFS, i|(tmp<<24));
		nand_dll_reset();

	} else {
		printf("CAL scan fail\r\n");
		return E_PAR;
	}

	// 2. Edge detect
	if (info->flash_info->dqs_en) {
		printf("Set DQS En\r\n");
		NAND_SETREG(NULL, NAND_DLL_PHASE_DLY_REG1_OFS, \
			NAND_GETREG(NULL, NAND_DLL_PHASE_DLY_REG1_OFS)|0x1<<21);
	}

	NAND_SETREG(NULL, NAND_DLL_PHASE_DLY_REG1_OFS, \
			NAND_GETREG(NULL, NAND_DLL_PHASE_DLY_REG1_OFS)|0x2);
	udelay(1);
	NAND_SETREG(NULL, NAND_DLL_PHASE_DLY_REG1_OFS, \
			NAND_GETREG(NULL, NAND_DLL_PHASE_DLY_REG1_OFS)&~0x2);

	nand_dll_reset();

	// read tuning pattern
	flush_dcache_range((unsigned long)rBuf, (unsigned long)((unsigned long)rBuf + pageSize));
	if (spinor_read_operation(info, addr, pageSize, rBuf)) {
		printf("read pattern fail\r\n");
	}

	tmp = NAND_GETREG(NULL, NAND_DLL_PHASE_DLY_REG0_OFS);
	printf("edge detect result: 0x%x \r\n", tmp);

	if (info->flash_info->dtr_read) {
		// DDR mode
		result = (tmp&0xFFF) | ((tmp>>16)&0xFFF);

		if (result) {
			// 567 1T detect: check scan result
			printf("DTR Result: 0x%x \r\n", result);
			passZoneLen = 0;

			if (info->flash_info->dqs_en) {
				for (i = 0; i < 11; i++) {
					if (result & (0x1<<i))
						break;
					passZoneLen++;
				}
				if (passZoneLen == 0) {
					printf("Edge detect fail, passZoneLen = 0\r\n");
					return E_SYS;
				} else {
					if (passZoneLen < 3) {
						printf("WRN: passZoneLen too short, len = %d \r\n", passZoneLen);
					}
					info->bestPhase = (passZoneLen-1)/2;
					printf("DQS PassZone: Phase 0 ~ %d \r\n", passZoneLen-1);
					printf("BestPhase = %d \r\n", info->bestPhase);
				}
			} else { // internel clk
				UINT32 passZoneStart = 0, eg1 = 0;

				for (i = 10; i > 0; i--) {
					if (passZoneStart == 0) {
						if (!eg1 && ((result & (0x1<<i)) == 0)) {
							continue;
						} else if (eg1 == 0) {
							eg1 = i;
							continue;
						} else {
							if (result & (0x1<<i)) {
								continue;
							}
						}
						passZoneStart = i;
						passZoneLen++;
					} else {
						if (result & (0x1<<i))
							break;
						passZoneLen++;
					}
				}
				printf("CLK PassZone: Phase %d ~ %d , len = %d\r\n", (passZoneStart - passZoneLen+1), (passZoneStart), passZoneLen);
				info->bestPhase = passZoneStart - (passZoneLen/2);
				printf("BestPhase = %d \r\n", info->bestPhase);

			}
			// apply bestPhase
			tmp = NAND_GETREG(NULL, NAND_DLL_PHASE_DLY_REG1_OFS)&~0xF00;
			NAND_SETREG(NULL, NAND_DLL_PHASE_DLY_REG1_OFS, tmp|(info->bestPhase<<8));
			nand_dll_reset();
		} else {
			printf("err: no detect edge \r\n");
			return E_SYS;
		}
	} else {
		printf("SDR mode: TBD\r\n");
		return E_SYS;
	}

	// reset to internel clk
	if (info->flash_info->dqs_en) {
		NAND_SETREG(NULL, NAND_DLL_PHASE_DLY_REG1_OFS, \
			NAND_GETREG(NULL, NAND_DLL_PHASE_DLY_REG1_OFS)& ~(0x1<<21));
	}
	return E_OK;
}

int nand_host_dll_auto_tuning(struct drv_nand_dev_info *info, NAND_TYPE_SEL nandType)
{
	UINT32 param = info->autok_param;
	UINT32 pageSize, bitMode;
	u8 *wBuf;
	int ret = E_OK;

	printf("Start Auto tuning, param: 0x%x\r\n", param);

	if (nandType == NANDCTRL_SPI_NAND_TYPE) {
		pageSize = info->flash_info->page_size;
		bitMode = (info->flash_info->spi_nand_status.bQuadProgram)?SPI_NOR_QUAD_READ:SPI_NOR_NORMAL_READ;
	} else if (nandType == NANDCTRL_SPI_NOR_TYPE) {
		pageSize = info->flash_info->page_size;
		//bitMode = info->flash_info->nor_quad_support;
		bitMode = info->flash_info->nor_read_mode;
	} else {
		printf("nandType %d not support\r\n", nandType);
		return E_PAR;
	}

	wBuf = malloc(pageSize);
	if (!wBuf) {
		printf("alloc write buf failed!\n");
		return -ENOMEM;
	}

	printf("Pattern size: 0x%x\r\n", pageSize);
	NAND_AUTOK_DBG(info, "wBuf: %p\r\n", wBuf);
	NAND_AUTOK_MSG(info, "bitMode: %d-bit \r\n", (1<<bitMode));

	if (bitMode == SPI_NOR_OCTAL_READ) {
		for(uint32_t i = 0; i < (pageSize/4);i++)
			OUTW((wBuf+4*i), 0x00FF00FF);
	} else if (bitMode == SPI_NOR_QUAD_READ) {
		memset((void*)wBuf, 0x0F, pageSize);	// 4-bit mode
	} else {
		memset((void*)wBuf, 0x55, pageSize);	// 1-bit mode
	}

	if (param & NAND_AUTOK_PATTERN_EN) {
		ret = nand_host_create_edge_detect_pattern(info, nandType, wBuf);
		if (ret != E_OK) {
			printf("create tuning pattern fail (%d)\r\n", ret);
		}
	}

	if (param & NAND_AUTOK_MODE1) {
		nand_host_indly_scan(info, nandType, wBuf);
	//} else if (param & NAND_AUTOK_MODE2) {
	//	nand_host_dll_edge_scan(info, nandType);
	} else if ((param & NAND_AUTOK_MODE3) && (info->chip_id == CHIP_NS02301 || info->chip_id == CHIP_NS02302 || info->chip_id == CHIP_NS02402)) {
		ret = nand_host_1t_edge_detect(info, nandType, wBuf);
	} else {
		printf("Auto tuning mode not support \r\n");
		ret = E_PAR;
	}

	free(wBuf);

	return ret;
}

