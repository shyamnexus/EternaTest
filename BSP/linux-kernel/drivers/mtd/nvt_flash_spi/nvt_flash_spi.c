/*
    @file       nvt_flash_spi.c

    @brief      Low layer driver which will access to FLASH SPI controller

    @note       Nothing

    Copyright   Novatek Microelectronics Corp. 2016.  All rights reserved.

*/
#include "nvt_flash_spi_int.h"
#include "nvt_flash_spi_reg.h"
#include <linux/platform_device.h>

int nand_cmd_wait_complete(struct drv_nand_dev_info *info)
{
	u32 ret = 0;
	// Use interrupt to wait command.
	ret = wait_for_completion_timeout(&info->cmd_complete, 4*HZ);

	if (ret == 0) {
		ret = NAND_GETREG(info, NAND_CTRL_STS_REG_OFS);
		dev_err(&info->pdev->dev, "wait timeout with status 0x%x\n", ret);
		return -EINTR;
	}

	if ((info->nand_int_status & NAND_STS_FAIL_STS) && \
		!(info->nand_int_status & SPINAND_FEATURE_STATUS_ECC_UNCORR_ERR)) {
		dev_err(&info->pdev->dev, "nand status error\r\n");
		return -EINTR;
	}

	if(info->nand_int_status & NAND_TOUT_STS) {
		dev_err(&info->pdev->dev, "NAND cmd timeout\r\n");
		return -EIO;
	}
	return E_OK;
}

/*
    Configure nand host transfer register

    This function configure controller transfer related register

    @param[in]  trans_param      Setup transfer configuration
    @param[in]  page_count       Transfer page count
    @param[in]  length          Transfer length
    @param[in]  multi_page_select multi page / multi spare / single page

    @return void
*/
void nand_host_setup_transfer(struct drv_nand_dev_info *info, \
				struct smc_setup_trans *trans_param, \
				uint32_t page_count, \
				uint32_t length, \
				uint32_t multi_page_select)
{
	const struct nvt_nand_flash *f = info->flash_info;
	uint32_t value3 = 0;
	uint32_t value = 0;
	uint32_t value1 = 0;

	//#NT#2012/09/05#Steven Wang -begin
	//#NT#0039432,Temp disable complete interrupt mask
	value3 = NAND_GETREG(info, NAND_INTMASK_REG_OFS);
	value3 &= ~NAND_COMP_INTREN;
	NAND_SETREG(info, NAND_INTMASK_REG_OFS, value3);
	//#NT#2012/09/05#Steven Wang -end

	if (page_count > 0)
	{
		NAND_SETREG(info, NAND_PAGENUM_REG_OFS, page_count);
		value = multi_page_select;
	}

#if !defined(CONFIG_NVT_IVOT_PLAT_NA51055) && !defined(CONFIG_NVT_IVOT_PLAT_NA51089)
	//Even if not use FIFO, FIFO dir & PIO/DMA mode need to set
	//if (page_count == 0 && (length == GET_NAND_SPARE_SIZE(f))) {
		NAND_SETREG(info, NAND_FIFO_CTRL_REG_OFS, (trans_param->fifo_dir | trans_param->trans_mode));
	//}
#endif

	//#NT#2012/09/05#Steven Wang -begin
	//#NT#0039432, Set single or multipage first
	value1 = NAND_GETREG(info, NAND_CTRL0_REG_OFS);
	NAND_SETREG(info, NAND_CTRL0_REG_OFS, value);
	//#NT#2012/09/05#Steven Wang -end

	// Set column address
	NAND_SETREG(info, NAND_COLADDR_REG_OFS, trans_param->col_addr);

	// Set row address
	NAND_SETREG(info, NAND_ROWADDR_REG_OFS, trans_param->row_addr);

	// Set data length
	NAND_SETREG(info, NAND_DATALEN_REG_OFS, length);

	//#NT#2010/02/09#Steven Wang -begin
	//#NT#Only for NT96460
	if((value1 & _PROTECT1_EN) == _PROTECT1_EN)
	{
		value |= _PROTECT1_EN;
	}
	if((value1 & _PROTECT2_EN) == _PROTECT2_EN)
	{
		value |= _PROTECT2_EN;
	}

	value |= trans_param->type;

	if(trans_param->fifo_dir == _FIFO_READ)
		value |= _NAND_WP_EN;

	NAND_SETREG(info, NAND_CTRL0_REG_OFS, value);

	//config CS
	value3 = NAND_GETREG(info, NAND_SPI_CFG_REG_OFS);
	value3 &= ~(1<<3);
	value3 |= (trans_param->cs <<3);
	NAND_SETREG(info, NAND_SPI_CFG_REG_OFS, value3);
	//NAND_DEBUG_MSG("info->flash_info cs = %d, trans_param->cs = %d, NAND_SPI_CFG_REG_OFS = 0x%08lx\r\n", info->flash_info->chip_sel, trans_param->cs, (unsigned long)NAND_GETREG(info, NAND_SPI_CFG_REG_OFS));

	//#NT#2012/09/05#Steven Wang -begin
	//#NT#1. Clear status
	//#NT#2. Re enable complete interrupt mask
	//#NT#0039432
	value3 = NAND_GETREG(info, NAND_CTRL_STS_REG_OFS);
	value3 |= NAND_COMP_STS;
	NAND_SETREG(info, NAND_CTRL_STS_REG_OFS, value3);
	//#NT#2012/09/05#Steven Wang -end

	// Set control0 register
#if defined(CONFIG_NVT_IVOT_PLAT_NA51055) || defined(CONFIG_NVT_IVOT_PLAT_NA51089)
	if (length && ((length > GET_NAND_SPARE_SIZE(f)) || \
					(length == 8) || (length == 4) || \
					(multi_page_select == _MULTI_SPARE))) {
#else
	if (length && ((length > GET_NAND_SPARE_SIZE(f)*(1+info->flash_info->spare_size)) || \
					(length == 8) || (length == 4) || (trans_param->type == _SPI_READ_N_BYTES) || \
					(multi_page_select == _MULTI_SPARE || trans_param->fifo_en == 1))) {
#endif
		NAND_SETREG(info, NAND_FIFO_CTRL_REG_OFS, \
			(trans_param->fifo_dir | trans_param->trans_mode));

		NAND_SETREG(info, NAND_FIFO_CTRL_REG_OFS, (_FIFO_EN | \
							trans_param->fifo_dir | \
							trans_param->trans_mode));
	}

/*
	if ((page_count == 0) && (length == 0x40)) {
		NAND_SETREG(info, NAND_FIFO_CTRL_REG_OFS, \
			(trans_param->fifo_dir | trans_param->trans_mode));

		NAND_GETREG(info, NAND_FIFO_CTRL_REG_OFS);

		NAND_SETREG(info, NAND_FIFO_CTRL_REG_OFS, (_FIFO_EN | \
							trans_param->fifo_dir | \
							trans_param->trans_mode));
	}
*/
	//#NT#0039432, re enable comp interrupt
	value3 = NAND_GETREG(info, NAND_INTMASK_REG_OFS);
	value3 |= NAND_COMP_INTREN;
	NAND_SETREG(info, NAND_INTMASK_REG_OFS, value3);
	//NAND_DEBUG_MSG("NAND_MODULE0_REG_OFS = 0x%08lx\r\n", (unsigned long)NAND_GETREG(info, NAND_MODULE0_REG_OFS));

	//config NAND nor type
	nand_host_set_nand_type(info, info->flash_info->config_nand_type);
}

/*
    Configure SM transfer command

    Command send to NAND

    @param[in]  command     nand command
    @param[in]  tmout_en    time out enable / disable

    @return void
*/
void nand_host_send_command(struct drv_nand_dev_info *info, uint32_t command,
							uint32_t tmout_en)
{
	union NAND_CTRL0_REG reg_ctrl0;

	// Set command
	NAND_SETREG(info, NAND_COMMAND_REG_OFS, command);
	reg_ctrl0.reg = NAND_GETREG(info, NAND_CTRL0_REG_OFS);
	reg_ctrl0.bit.oper_en = 1;
	reg_ctrl0.bit.timeout_en = tmout_en;

	NAND_SETREG(info, NAND_CTRL0_REG_OFS, reg_ctrl0.reg);
}


/*
    NAND controller host receive data routing

    After read operation related command issued, called this function to reveive data

    @param[in]  buffer      buffer receive to
                length      receive buffer length
                trans_mode   PIO/DMA mode usage

    @return
        - @b E_OK           receive data success
        - @b E_SYS          error status
*/
int nand_host_receive_data(struct drv_nand_dev_info *info, uint8_t *buffer,
					uint32_t length, uint32_t trans_mode)
{
	uint32_t tempBuffer;
	uint32_t index;
	const uint32_t fifo_size = FIFO_BURST_LEN;

	if(trans_mode == _PIO_MODE) {
		while(length) {
			if(length >= fifo_size) {
				if(NAND_GETREG(info, NAND_FIFO_STS_REG_OFS) & _FIFO_FULL) {
					for (index = fifo_size; index > 0; index -= 4) {
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

    @param[in]  buffer      buffer transmit from
                length      transmit buffer length
                trans_mode   PIO/DMA mode usage

    @return
        - @b E_OK           transmit data success
        - @b E_SYS          error status
*/
int nand_host_transmit_data(struct drv_nand_dev_info *info, uint8_t *buffer, uint32_t length, uint32_t trans_mode)
{
	int32_t index;
#if defined(CONFIG_NVT_IVOT_PLAT_NA51055) || defined(CONFIG_NVT_IVOT_PLAT_NA51089) || defined(CONFIG_NVT_IVOT_PLAT_NA51102)
	BOOL not_aligned = (length%4)?1:0;
	uint32_t temp, delay, sLen = length;
#endif

	if(trans_mode == _PIO_MODE) {
		length = ((length+3)/4) * 4;
		NAND_SETREG(info, NAND_FIFO_CTRL_REG_OFS, (_FIFO_EN | NAND_GETREG(info, NAND_FIFO_CTRL_REG_OFS)));

		while(length) {
			if(length < FIFO_BURST_LEN) {
				if(NAND_GETREG(info, NAND_FIFO_STS_REG_OFS) & _FIFO_EMPTY) {
					for (index = length; index > 0; index -= 4) {
						NAND_SETREG(info, NAND_DATAPORT_REG_OFS, *((uint32_t *)buffer));
						buffer += 4;
						length -= 4;
					}
				}
			} else {
				if(NAND_GETREG(info, NAND_FIFO_STS_REG_OFS) & _FIFO_EMPTY)
				{
					for (index = FIFO_BURST_LEN; index > 0; index -= 4) {
						NAND_SETREG(info, NAND_DATAPORT_REG_OFS, *((uint32_t *)buffer));
						buffer += 4;
						length -= 4;
					}
				}
			}
		}
#if defined(CONFIG_NVT_IVOT_PLAT_NA51055) || defined(CONFIG_NVT_IVOT_PLAT_NA51089) || defined(CONFIG_NVT_IVOT_PLAT_NA51102)
		if(not_aligned) {
			delay = (sLen*8+10)*100/(info->flash_freq/1000000);
			delay = (delay/100) + (delay%100?1:0);
			udelay(delay);

			temp = NAND_GETREG(info, NAND_FIFO_CTRL_REG_OFS);
			temp &= ~(0x1);
			NAND_SETREG(info, NAND_FIFO_CTRL_REG_OFS, temp);
		}
#endif
	}
	return nand_cmd_wait_complete(info);
}

/*
    Enable / Disable FIFO

    Enable and Disable FIFO of NAND controller

    @param[in]  en     enable / disable

    @return void
*/
void nand_host_set_fifo_enable(struct drv_nand_dev_info *info, uint32_t en)
{
	union NAND_FIFO_CTRL_REG reg_fifo_ctrl;

	reg_fifo_ctrl.reg = NAND_GETREG(info, NAND_FIFO_CTRL_REG_OFS);

	reg_fifo_ctrl.bit.fifo_en = en;

	NAND_SETREG(info, NAND_FIFO_CTRL_REG_OFS, reg_fifo_ctrl.reg);

	if(en == FALSE)
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
	union NAND_SRAM_ACCESS_REG  reg_sram_acc = {0x00000000};
	uint8_t i;

	reg_sram_acc.reg = NAND_GETREG(info, NAND_SRAM_ACCESS_REG_OFS);
	reg_sram_acc.bit.error_acc = bcpu_Acc;

	NAND_SETREG(info, NAND_SRAM_ACCESS_REG_OFS,reg_sram_acc.reg);

	//dummy read to delay 200ns for SRAM ready
	if(bcpu_Acc == TRUE) {
	for(i = 0; i < SRAMRDY_DELAY; i++)
		reg_sram_acc.reg = NAND_GETREG(info, NAND_SRAM_ACCESS_REG_OFS);
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
	union NAND_SRAM_ACCESS_REG  reg_sram_acc = {0x00000000};
	uint8_t i;

	reg_sram_acc.reg = NAND_GETREG(info, NAND_SRAM_ACCESS_REG_OFS);
	reg_sram_acc.bit.spare_acc = bcpu_Acc;

	NAND_SETREG(info, NAND_SRAM_ACCESS_REG_OFS,reg_sram_acc.reg);

	//dummy read to delay 200ns for SRAM ready
	if(bcpu_Acc == TRUE) {
		for(i = 0; i < SRAMRDY_DELAY; i++)
			reg_sram_acc.reg = NAND_GETREG(info, NAND_SRAM_ACCESS_REG_OFS);
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
					uint8_t * Buf, uint32_t section)
{
	int32_t ret = 0;
	uint32_t i, j, shifts;
	uint32_t hrdata, rs_sts, rs_err, err_reg, err_bit, mask_bit;
    unsigned long err_addr;

	nand_host_set_cpu_access_err(info, TRUE);

	rs_sts = NAND_GETREG(info, NAND_RSERR_STS0_REG_OFS);

	for(i = 0; i < section; i++) {
		shifts = 4 * i;
		rs_err = (rs_sts >> shifts) & 0x7;
		if(rs_err == 0) {
			continue;
		} else if(rs_err == 5) {
			ret = E_CTX;
			continue;
		}

		for(j = 0; j < rs_err; j++) {
			err_reg = NAND_SEC0_EADDR0_REG_OFS + (16 * i) + (4 * j);

			hrdata = NAND_GETREG(info, err_reg);
			err_addr = hrdata & 0xFFFF;

			if(err_addr >= _512BYTE)
			{
				nand_host_set_cpu_access_spare(info, TRUE);
				err_addr -= _512BYTE;
				err_addr += (uintptr_t)((info->mmio_base + NAND_SPARE00_REG_OFS) + (i * 16));
			}
			err_bit = (hrdata >> 16) & 0xFF;

			mask_bit = (err_addr & 0x03) * 8;

			if((hrdata & 0xFFFF) >= _512BYTE) {
				* (volatile uintptr_t *) (err_addr & 0xFFFFFFFC) ^= (err_bit << mask_bit);
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
					uint32_t section)
{
	int32_t ret = E_OK;
	uint32_t i, shifts;
	uint32_t hrdata, ham_sts, ham_err, err_reg, err_bit;
	uint32_t tmp_cs;
    unsigned long err_addr;
	union NAND_CTRL0_REG    nand_ctrl0;
	union NAND_MODULE0_REG  nand_module_cfg;

	uint32_t mask_bit;

	nand_host_set_cpu_access_err(info, TRUE);

	ham_sts = NAND_GETREG(info, NAND_SECONDARY_ECC_STS_REG_OFS);

	nand_ctrl0.reg = NAND_GETREG(info, NAND_CTRL0_REG_OFS);

	tmp_cs = nand_ctrl0.bit.chip_en;

	nand_module_cfg.reg = NAND_GETREG(info, tmp_cs * 4);

	for(i = 0; i < section; i++) {   // 8 sections
		shifts = 2 * i;
		ham_err = (ham_sts >> shifts) & 0x3;

		if(ham_err == 0) {
			continue;
		} else if(ham_err == 2) {
			ret = E_CTX;
			continue;
		} else if(ham_err == 3) {
			ret = E_CTX;
			continue;
		}
		err_reg = NAND_SEC0_EADDR0_REG_OFS + (16 * i) + 4;

		hrdata = NAND_GETREG(info, err_reg);

		err_addr = hrdata & _ECC_SEC0_SECONDARY_ERRBYTE;
		err_bit = (hrdata & _ECC_SEC0_SECONDARY_ERRBIT)>>_ECC_SEC0_SECONDARY_BITSFT;

		if(err_addr >= _512BYTE) {
			nand_host_set_cpu_access_spare(info, TRUE);
			err_addr -= _512BYTE;
			err_addr += (uintptr_t)(info->mmio_base + NAND_SPARE00_REG_OFS + i * 16);
			mask_bit = (err_addr & 0x03) * 8;
			* (volatile uintptr_t *) (err_addr & 0xFFFFFFFC) ^= ((1<<err_bit) << mask_bit);

			nand_host_set_cpu_access_spare(info, FALSE);
		}
		else {
			pr_info("Secondary ECC should occurred > section size[0x%08x]\r\n", _512BYTE);
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
	uint32_t hrdata, bch_sts, bch_err, err_reg, err_bit, mask_bit;
	unsigned long err_addr;

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
			pr_err("    uncorrect ecc error\r\n");
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

				err_addr += (((long unsigned int)info->mmio_base + NAND_SPARE00_REG_OFS) - (0x200*(uiSection-i)));
				if (!gNANDSpareAlignSel && ((err_addr - i*0x10)&0xFF) > 0x2) {
					// no align, user area [0~2], ecc area[3~15]
					NAND_DEBUG_MSG("      sec[%d]BCH error bit[%03ld][b%02x] (SpareEcc), skip\r\n", i, (err_addr&0x3FF), err_bit);
					nand_host_set_cpu_access_spare(info, FALSE);
					continue;
				} else if (gNANDSpareAlignSel){
					if (uiSection == 4) {
						// 2K page + align, Byte 0~11
						if ((err_addr&0xFF) > 0xB) {
							NAND_DEBUG_MSG("      sec[%d]BCH error bit[%03ld][b%02x] (SpareEcc, B12~63), skip\r\n", i, err_addr, err_bit);
							nand_host_set_cpu_access_spare(info, FALSE);
							continue;
						}
					} else {
						// 4K page + align, Byte 0~23
						if ((err_addr&0xFF) > 0x17) {
							NAND_DEBUG_MSG("      sec[%d]BCH error bit[%03ld][b%02x] (SpareEcc, B24~127), skip\r\n", i, err_addr, err_bit);
							nand_host_set_cpu_access_spare(info, FALSE);
							continue;
						}
					}
				}
				NAND_DEBUG_MSG("      sec[%d]BCH error bit[%03ld][b%02x] (SpareUser)\r\n", i, err_addr, err_bit);

			} else {
				NAND_DEBUG_MSG("      sec[%d]BCH error bit[%03ld][b%02x]\r\n", i, err_addr, err_bit);
			}

			if (err_addr >= _512BYTE) {
				* (volatile uintptr_t *)(err_addr & 0xFFFFFFFC) ^= (err_bit << mask_bit);
				NAND_DEBUG_MSG("        verify spare[0x%08lx]:0x%lx\r\n", (err_addr & 0xFFFFFFFC), *(volatile uintptr_t *)(err_addr & 0xFFFFFFFC));
				nand_host_set_cpu_access_spare(info, FALSE);
			} else {
				Buf[err_addr + i * _512BYTE] ^= (err_bit); // correct bit
			}
		}
	}

	nand_host_set_cpu_access_err(info, FALSE);

	return ret;
}

int nand_host_set_nand_type(struct drv_nand_dev_info *info, NAND_TYPE_SEL nand_type)
{
	union NAND_MODULE0_REG  reg_module0 = {0};
	int ret = E_OK;

	reg_module0.reg = NAND_GETREG(info, NAND_MODULE0_REG_OFS);
	switch(nand_type)
	{
	case NANDCTRL_ONFI_NAND_TYPE:
	reg_module0.bit.nand_type = 0;
	reg_module0.bit.spi_flash_type = 0;
	break;

	case NANDCTRL_SPI_NAND_TYPE:
	reg_module0.bit.nand_type = 1;
	reg_module0.bit.spi_flash_type = 0;
	//reg_module0.bit.col_addr = NAND_1COL_ADDR;
	//reg_module0.bit.row_addr = NAND_1ROW_ADDR;
	break;

	case NANDCTRL_SPI_NOR_TYPE:
	reg_module0.bit.nand_type = 1;
	reg_module0.bit.spi_flash_type = 1;
	//reg_module0.bit.col_addr = NAND_1COL_ADDR;
	//reg_module0.bit.row_addr = NAND_1ROW_ADDR;
	break;

	default:
	return E_NOSPT;
	break;
	}

	info->flash_info->config_nand_type = nand_type;
	NAND_SETREG(info, NAND_MODULE0_REG_OFS, reg_module0.reg);

	return ret;
}

int nand_host_set_dtr_type(struct drv_nand_dev_info *info, DTR_TYPE_SEL dtr_en)
{
	union NAND_MODULE0_REG  reg_module0 = {0};
	int ret = E_OK;

	reg_module0.reg = NAND_GETREG(info, NAND_MODULE0_REG_OFS);
	switch (dtr_en) {
	case NANDCTRL_SDR_TYPE:
		reg_module0.bit.spi_nor_flash_op_mode = 0;
		reg_module0.bit.spi_write_ddr_clk = 0;
		break;

	case NANDCTRL_DTR_TYPE:
		reg_module0.bit.spi_nor_flash_op_mode = 1;
		reg_module0.bit.spi_write_ddr_clk = 1;
		break;

	default:
		return E_NOSPT;
		break;
	}

	NAND_SETREG(info, NAND_MODULE0_REG_OFS, reg_module0.reg);

	return ret;
}

void nand_dll_reset(struct drv_nand_dev_info *info)
{
	union NAND_DLL_PHASE_DLY_REG1 reg_dll_ctrl1;

	reg_dll_ctrl1.reg = NAND_GETREG(info, NAND_DLL_PHASE_DLY_REG1_OFS);
	reg_dll_ctrl1.bit.phy_sw_reset = 1;
	NAND_SETREG(info, NAND_DLL_PHASE_DLY_REG1_OFS, reg_dll_ctrl1.reg);

	do {
		reg_dll_ctrl1.reg = NAND_GETREG(info, NAND_DLL_PHASE_DLY_REG1_OFS);
	} while (reg_dll_ctrl1.bit.phy_sw_reset == 1);

	// Enable AXI ch
	nand_host_axi_ch_disable(info, 0);

}

void nand_phy_config(struct drv_nand_dev_info *info)
{
	union NAND_DLL_PHASE_DLY_REG1 reg_phy_dly;
	union NAND_DLL_PHASE_DLY_REG2 reg_phy_dly2;
	union NAND_MODULE0_REG reg_module0;

	reg_phy_dly.reg = NAND_GETREG(info, NAND_DLL_PHASE_DLY_REG1_OFS);
	reg_phy_dly2.reg = NAND_GETREG(info, NAND_DLL_PHASE_DLY_REG2_OFS);
	reg_module0.reg = NAND_GETREG(info, NAND_MODULE0_REG_OFS);

	reg_phy_dly.bit.phy_sampclk_inv = 1;

#if defined(CONFIG_NVT_IVOT_PLAT_NA51103) || defined(CONFIG_NVT_IVOT_PLAT_NS02301) || defined(CONFIG_NVT_IVOT_PLAT_NS02302)
	reg_phy_dly.bit.phy_src_clk_sel = 0;
	reg_phy_dly.bit.phy_pad_clk_sel = 1;
	reg_phy_dly2.bit.indly_sel = 0x0;
#elif defined(CONFIG_NVT_IVOT_PLAT_NA51102) || defined(CONFIG_NVT_IVOT_PLAT_NS02201)
	reg_phy_dly.bit.phy_src_clk_sel = 0;
	reg_phy_dly.bit.phy_pad_clk_sel = 1;
	if (info->flash_freq < 24000000) {
		reg_phy_dly.bit.phy_sampclk_inv = 0;
		reg_phy_dly2.bit.indly_sel = 0x0;
	} else if (info->flash_freq <= 80000000){
		reg_phy_dly2.bit.indly_sel = 0x5;
	} else {	// ~ 120MHz
		reg_phy_dly2.bit.indly_sel = 0x1E;
	}

#else
	if (0) { //(nvt_get_chip_id() == CHIP_NA51055) {
		if (info->flash_freq > 96000000) {
			if (reg_module0.bit.spi_flash_type) {
				reg_phy_dly.bit.phy_src_clk_sel = 1;
				reg_phy_dly2.bit.indly_sel = 0x3F;
			} else {
				reg_phy_dly.bit.phy_src_clk_sel = 0;
				reg_phy_dly2.bit.indly_sel = 0x20;
			}
		} else {
			reg_phy_dly.bit.phy_src_clk_sel = 0;
			reg_phy_dly2.bit.indly_sel = 0x0;
		}
	} else {
		reg_phy_dly.bit.phy_src_clk_sel = 0;
		reg_phy_dly.bit.phy_pad_clk_sel = 1;
		reg_phy_dly2.bit.indly_sel = 0x8;
		pr_warn("nvt nand phy config: todo \n");
	}
#endif

	if (info->flash_info->indly) {
		pr_info("apply dts indly: %d \n", info->flash_info->indly);
		reg_phy_dly2.bit.indly_sel = info->flash_info->indly;
	}

#if defined(CONFIG_NVT_IVOT_PLAT_NS02301) || defined(CONFIG_NVT_IVOT_PLAT_NS02302)
	if (info->flash_freq < 48000000) {
		// in low speed, clear data dly phase sel
		reg_phy_dly.bit.data_dly_phase_sel = 0;
	}
#endif

	NAND_SETREG(info, NAND_DLL_PHASE_DLY_REG2_OFS, reg_phy_dly2.reg);
	NAND_SETREG(info, NAND_DLL_PHASE_DLY_REG1_OFS, reg_phy_dly.reg);
}

int nand_host_setup_page_size(struct drv_nand_dev_info *info, NAND_PAGE_SIZE page_size)
{
	union NAND_MODULE0_REG  reg_module0 = {0};

	reg_module0.reg = NAND_GETREG(info, NAND_MODULE0_REG_OFS);

	if(page_size == NAND_PAGE_SIZE_512 || page_size == NAND_PAGE_SIZE_2048 || page_size == NAND_PAGE_SIZE_4096)
	{
		reg_module0.bit.page_size = page_size;
		NAND_SETREG(info, NAND_MODULE0_REG_OFS, reg_module0.reg);
		return E_OK;
	}
	else
		return E_SYS;
}


/*
    Configure nand host row and column address cycle

    This function configure controller row & column cycle

    @param[in]  uiCS		chip select
    @param[in]  row 		row     cycle
    @param[in]  uiColumn	column  cycle

    @return
        - @b E_SYS      Status fail
        - @b E_OK       Operation success

*/
int nand_host_setup_address_cycle(struct drv_nand_dev_info *info,
		NAND_ADDRESS_CYCLE_CNT row, NAND_ADDRESS_CYCLE_CNT col)
{
	union NAND_MODULE0_REG  reg_module0 = {0};

	reg_module0.reg = NAND_GETREG(info, NAND_MODULE0_REG_OFS);

	if(col > NAND_3_ADDRESS_CYCLE_CNT && col != NAND_NOT_CONFIGED_ADDRESS_CYCLE_CNT)
		return E_SYS;

	if(col != NAND_NOT_CONFIGED_ADDRESS_CYCLE_CNT)
		reg_module0.bit.col_addr = col;

	if(row != NAND_NOT_CONFIGED_ADDRESS_CYCLE_CNT)
		reg_module0.bit.row_addr = row;

	NAND_SETREG(info, NAND_MODULE0_REG_OFS, reg_module0.reg);

	return E_OK;
}

/*
    NAND controller enable polling bit match operation

    Enable bit value compare function.
    After invoke spi_enBitMatch(), it will send uiCmd to SPI device
    and continously read data from SPI device.
    Once bit position specified by uiBitPosition of read data becomes bWaitValue,
    SPI module will stop checking and raise a interrupt.
    Caller of spi_enBitMatch() can use spi_waitBitMatch() to wait this checking complete.

    @param[in] status_value    The check value
    @param[in] status_mask     Indicates which bit of status value you want to check. 1 for comparison and 0 for not comparison

    @return
        - @b E_OK: success
        - @b Else: fail
*/
int nand_host_setup_status_check_bit(struct drv_nand_dev_info *info, u8 status_mask, u8 status_value)
{
	union NAND_STATUS_CHECK_REG reg_status_check = {0};

	reg_status_check.bit.status_value = status_value;
	reg_status_check.bit.status_mask = status_mask;

	NAND_SETREG(info, NAND_STATUS_CHECK_REG_OFS, reg_status_check.reg);
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
int nand_host_set_spi_io(struct drv_nand_dev_info *info,
		NAND_SPI_CS_POLARITY cs_pol, NAND_SPI_BUS_WIDTH bus_width,
			NAND_SPI_IO_ORDER io_order)
{
	union NAND_SPI_CFG_REG  reg_cfg = {0};

	reg_cfg.reg = NAND_GETREG(info, NAND_SPI_CFG_REG_OFS);

	reg_cfg.bit.spi_cs_pol    = cs_pol;
	reg_cfg.bit.spi_bs_width  = bus_width;
	reg_cfg.bit.spi_io_order  = io_order;
	reg_cfg.bit.spi_pull_wphld  = 0x1;

	NAND_SETREG(info, NAND_SPI_CFG_REG_OFS, reg_cfg.reg);
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
uint32_t nand_host_get_multi_page_select(struct drv_nand_dev_info *info)
{
	union NAND_CTRL0_REG  reg_ctrl = {0};

	reg_ctrl.reg = NAND_GETREG(info, NAND_CTRL0_REG_OFS);
	return (reg_ctrl.reg & 0x00030000);
}

/*
    Configure nand host chip select manual mode

    Configure nand host chip select manual mode

     @param[in] level    CS (chip select) level
            - NAND_SPI_CS_LOW    : CS set low
            - NAND_SPI_CS_HIGH   : CS set high

*/
int nand_host_set_cs_active(struct drv_nand_dev_info *info, \
				NAND_SPI_CS_LEVEL level)
{
	union NAND_SPI_CFG_REG  cfg_reg = {0};

	cfg_reg.reg = NAND_GETREG(info, NAND_SPI_CFG_REG_OFS);
	cfg_reg.bit.spi_nand_cs = level;

	NAND_SETREG(info, NAND_SPI_CFG_REG_OFS, cfg_reg.reg);
	return E_OK;
}

/*
    Configure nand host chip select manual mode

    Configure nand host chip select manual mode

     @param[in] mode    manual mode or auto mode
            - NAND_SPI_CS_AUTO_MODE     : manual mode(CS configure by user)
            - NAND_SPI_CS_MANUAL_MODE   : auto mode(CS configure by controller)

*/
int nand_host_config_cs_opmode(struct drv_nand_dev_info *info, \
				NAND_SPI_CS_OP_MODE mode)
{
	union NAND_SPI_CFG_REG  cfg_reg = {0};

	cfg_reg.reg = NAND_GETREG(info, NAND_SPI_CFG_REG_OFS);
	cfg_reg.bit.spi_operation_mode = mode;

	NAND_SETREG(info, NAND_SPI_CFG_REG_OFS, cfg_reg.reg);
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
	union NAND_TIME2_REG  time_reg = {0};

	time_reg.reg = time;

	if (time_reg.bit.tshch < 0x4) {
		pr_warn("SPI NAND tshch need >= 0x4, force config as 0x4\n");
		time_reg.bit.tshch = 0x4;
	}

	NAND_SETREG(info, NAND_TIME2_REG_OFS, time_reg.reg);
}


/*
    Configure nand host spare size

    @param[in]  bSpareSizeSel
    			- @b NAND_SPI_SPARE_16_BYTE : section = 512 + 16 byte
    			- @b NAND_SPI_SPARE_32_BYTE : section = 512 + 32 byte

*/
void nand_host_set_spare_size_sel(struct drv_nand_dev_info *info, NAND_SPI_SPARE_SIZE_SEL bSpareSizeSel)
{
	union NAND_MODULE0_REG  cfgReg = {0};

	cfgReg.reg = NAND_GETREG(info, NAND_MODULE0_REG_OFS);

	cfgReg.bit.spare_32_sel = bSpareSizeSel;
	NAND_SETREG(info, NAND_MODULE0_REG_OFS, cfgReg.reg);
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

    @return
        - @b E_OK: sucess

*/
void nand_host_set_spare_align_sel(struct drv_nand_dev_info *info, NAND_SPI_SPARE_ALIGN_SEL bSpareAlignSel)
{
	union NAND_MODULE0_REG  cfgReg = {0};

	cfgReg.reg = NAND_GETREG(info, NAND_MODULE0_REG_OFS);

	cfgReg.bit.spare_align_sel = bSpareAlignSel;
	NAND_SETREG(info, NAND_MODULE0_REG_OFS, cfgReg.reg);
}

/*
    Configure nand host BCH ecc protect spare user area or not

    @param[in]  bSpareEccFree
    			- @b NAND_SPI_SPARE_ECC_FREE_DISABLE : spare user area is protected
    			- @b NAND_SPI_SPARE_ECC_FREE_ENABLE  : spare user area is not protected

    @return
        - @b E_OK: sucess

*/
void nand_host_set_spare_eccfree(struct drv_nand_dev_info *info, NAND_SPI_SPARE_ECC_FREE bSpareEccFree)
{
	union NAND_ECC_FREE_REG  cfgReg = {0};

	cfgReg.reg = NAND_GETREG(info, NAND_ECC_FREE_REG_OFS);

	cfgReg.bit.ecc_free = bSpareEccFree;
	NAND_SETREG(info, NAND_ECC_FREE_REG_OFS, cfgReg.reg);
}

/*
    Enable / Disable AXI CH

    Enable and Disable AXI channel of NAND controller

    @param[in]  uiEN     0: enable, 1: disable

    @return void
*/
void nand_host_axi_ch_disable(struct drv_nand_dev_info *info, BOOL uiEN)
{
	union NAND_AXI_REG0 axiReg = {0};

	axiReg.reg = NAND_GETREG(info, NAND_AXI_REG0_OFS);

	axiReg.bit.ch_disable = uiEN;

	NAND_SETREG(info, NAND_AXI_REG0_OFS, axiReg.reg);

	if (uiEN == FALSE) {
		while ((NAND_GETREG(info, NAND_AXI_REG0_OFS) & _FIFO_EN) != 0);
	}
}

