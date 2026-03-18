/**
    NVT PCIE cascade driver
    MSIX controller related API
    @file nvt_msix.c
    @ingroup
    @note
    Copyright Novatek Microelectronics Corp. 2021. All rights reserved.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License version 2 as
    published by the Free Software Foundation.
*/
#include <linux/module.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/gpio.h>
#include <linux/irq.h>
#include <linux/interrupt.h>
#include <linux/irqchip/chained_irq.h>
#include <linux/soc/nvt/nvt-io.h>
#include <linux/soc/nvt/nvt_type.h>
#include <linux/soc/nvt/nvt-pcie-lib.h>
#include <linux/msi.h>
#include <linux/dma-mapping.h>

#include "nvt_pcie_int.h"
#include "nvt-msix.h"
#include "msix_reg.h"

#define     MSIx_SETREG(chip, ofs, value)	writel((value), ((chip)->PCIE_REG_BASE[MEM_MSIX]+(ofs)))
#define     MSIx_GETREG(chip, ofs)			readl((chip)->PCIE_REG_BASE[MEM_MSIX]+(ofs))

#ifndef CHKPNT
#define CHKPNT    printk("\033[37mCHK: %d, %s\033[0m\r\n", __LINE__, __func__)
#endif

#ifndef DBGD
#define DBGD(x)   printk("\033[0;35m%s=%d\033[0m\r\n", #x, x)
#endif

#ifndef DBGH
#define DBGH(x)   printk("\033[0;35m%s=0x%08X\033[0m\r\n", #x, x)
#endif

#ifndef DBG_DUMP
#define DBG_DUMP(fmtstr, args...) printk(fmtstr, ##args)
#endif

#ifndef DBG_ERR
#define DBG_ERR(fmtstr, args...)  printk("\033[0;31mERR:%s() \033[0m" fmtstr, __func__, ##args)
#endif

#ifndef DBG_WRN
#define DBG_WRN(fmtstr, args...)  printk("\033[0;33mWRN:%s() \033[0m" fmtstr, __func__, ##args)
#endif

#if 0
#define DBG_IND(fmtstr, args...) printk("%s(): " fmtstr, __func__, ##args)
#else
#ifndef DBG_IND
#define DBG_IND(fmtstr, args...)
#endif
#endif

static 	UINT32	bMSIxOpened = FALSE;

#define DUMMY_WRITE_DATA	FALSE

#if(DUMMY_WRITE_DATA)
#define ALIGN64_UP(x)       ((((x) + 63) >> 6) << 6)
static void *dummy_write_data_area = NULL;		// store virtual address of dummy write data
static dma_addr_t dummy_write_phy_addr = 0;		// store physical address of dummy write data
static UINT32 dummy_write_data_size = ALIGN64_UP(INT_ID_MAX * 4);
#endif


const static CHAR   *msix_pcie_interrupt[INT_ID_SUDO_INT_MSIx - INT_GIC_SPI_START_ID + 1] = {
		"INT_ID_TIMER",
		"INT_ID_VPELITE",
		"INT_ID_DMACP",
		"INT_ID_VPE",
		"INT_ID_VCAP",
		"INT_ID_VCAP2",
		"INT_ID_DEI",
		"INT_ID_VENC",
		"INT_ID_VENC2",
		"INT_ID_VDEC",
		"INT_ID_OSG",
		"INT_ID_ETHERNET2",
		"INT_ID_ETHERNET",
		"INT_ID_SATA0",
		"INT_ID_SATA1",
		"INT_ID_DAI",
		"INT_ID_AGE",
		"INT_ID_JPEG",
		"INT_ID_LCD",
		"INT_ID_LCD2",
		"INT_ID_LCD3",
		"INT_ID_DEI2",
		"INT_ID_GPENC",
		"INT_ID_GPENC2",
		"INT_ID_GPIO",
		"INT_ID_REMOTE",
		"INT_ID_PWM",
		"INT_ID_USB",
		"INT_ID_USB3",
		"INT_ID_NAND",
		"INT_ID_SDIO",
		"INT_ID_PCIE",	//31

		"INT_ID_ETHERNET_TX",
		"INT_ID_MAU",
		"INT_ID_DAI2",
		"INT_ID_DAI3",
		"INT_ID_DAI4",
		"INT_ID_DAI5",
		"INT_ID_UART4",
		"INT_ID_ETHERNET_RX",
		"INT_ID_ETHERNET2_RX",
		"INT_ID_I2C",
		"INT_ID_I2C2",
		"INT_ID_UART",
		"INT_ID_UART2",
		"INT_ID_UART3",
		"INT_ID_HASH",
		"INT_ID_UART5",
		"INT_ID_RSA",
		"INT_ID_ETHERNET2_TX",
		"INT_ID_I2C4",
		"INT_ID_TVE100",
		"INT_ID_HDMI",
		"INT_ID_MAU2",
		"INT_ID_CSI",
		"INT_ID_CSI2",
		"INT_ID_RTC",
		"INT_ID_WDT",
		"INT_ID_CG",
		"INT_ID_CSI3",
		"INT_ID_I2C3",
		"INT_ID_SDIO2",
		"INT_ID_GPIO2",
		"INT_ID_GPIO3",	//63

		"INT_ID_CSI4",
		"INT_ID_CSI5",
		"INT_ID_CSI6",
		"INT_ID_CSI7",
		"INT_ID_CSI8",
		"INT_ID_SATA3",
		"INT_ID_HWCP",
		"INT_ID_IVE",
		"INT_ID_NUE",
		"INT_ID_NUE2",
		"INT_ID_SSCA",
		"INT_ID_CNN",
		"INT_ID_CNN2",
		"INT_ID_CNN3",
		"INT_ID_USB2",
		"INT_ID_SCE",
		"INT_ID_GPIO4",
		"INT_ID_CC",
		"INT_ID_CC1",
		"INT_ID_CC2",
		"INT_ID_CC3",
		"INT_ID_RSV0",
		"INT_ID_RSV1",
		"INT_ID_RSV2",
		"INT_ID_RSV3",
		"INT_ID_RSV4",
		"INT_ID_RSV5",
		"INT_ID_RSV6",
		"INT_ID_RSV7",
		"INT_ID_RSV8",
		"INT_ID_RSV9",
		"INT_ID_SUDO_INT_MSIx",	//95
	};

/**
    Open msix driver.

    This API will enable UART clock and interrupt and also
    initialize the driver internal variables of the UART module.
    The user shoule notice that the UART pinmux is opened at the PinmuxCfg.c before opening uart controller.

    @return
     - @b E_OK:     If success
     - @b E_ID:  Outside semaphore ID number range
     - @b E_NOEXS:  Semaphore does not yet exist
     - @b E_QOVR:   Semaphore's counter error, maximum counter < counter
*/
ER msix_open(struct nvt_pcie_chip *nvt_pcie_chip_ptr, MSIx_SEL_TYPE msix_type, UINT32 lsb, UINT32 msb)
{
//#if defined(_MSIX_ON_)
//	UINT32 group;
//	char str[20];
//#endif
	UINT32			intr;
	AXI_WRITE_ADDR  axi_write_addr;
#if(DUMMY_WRITE_DATA)
	AXI_WRITE_ADDR  axi_dummy_write_addr;
#endif

	if(bMSIxOpened)
		return E_OK;
//	request_irq(INT_ID_SUDO_INT_MSIx, msix_sudo_isr ,IRQF_TRIGGER_HIGH, "msix", 0);

//#if defined(_MSIX_ON_)
//	for(group = 0; group < MSIx_Group_CNT; group ++) {
//		request_irq(INT_ID_MSIx_0 + group, msix_group_isr, IRQF_TRIGGER_HIGH, "msixN", 0);
//	}
//#endif

	axi_write_addr.ui_axi_address_lsb = lsb;
	axi_write_addr.ui_axi_address_msb = msb;
	axi_write_addr.ui_msix_sel = msix_type;
	msix_set_config(nvt_pcie_chip_ptr, MSIx_CONFIG_ID_SET_AXI_WRITE_ADDRESS, (u64)&axi_write_addr);

//	msix_enable_irq(INT_ID_UART);

#if(DUMMY_WRITE_DATA)
	if (dummy_write_data_area == 0) {
		int i;

		dummy_write_data_area = nvt_pcie_chip_ptr->va_msix_dummy_addr;
		// Becasue pcie lib is init after pcie driver
		// It's mapping is still not built at this point
		// Translate RC address by ourself
		dummy_write_phy_addr = nvt_pcie_chip_ptr->rc.phys_map[ATU_MAP_MAU].addr + nvt_pcie_chip_ptr->dma_msix_dummy_addr;
		printk("%s: RC PA 0x%llx --> PCIE addr 0x%llx\r\n", __func__,
				nvt_pcie_chip_ptr->dma_msix_dummy_addr,
				dummy_write_phy_addr);
		for (i=0; i<dummy_write_data_size; i+=sizeof(int32_t)) {
			writel(0, dummy_write_data_area + i);
		}
		axi_dummy_write_addr.ui_axi_address_lsb = lower_32_bits(dummy_write_phy_addr);
		axi_dummy_write_addr.ui_axi_address_msb = upper_32_bits(dummy_write_phy_addr);
		axi_dummy_write_addr.ui_msix_sel = 1;
		msix_set_config(nvt_pcie_chip_ptr, MSIx_CONFIG_ID_SET_2ND_AXI_WRITE_ADDRESS, (u64)&axi_dummy_write_addr);
	}
#endif

	for(intr = (INT_ID_TIMER); intr < (INT_ID_RSV0); intr++) {
//DBG_DUMP("msix_enable_irq[%d]\r\n", intr);

//		msix_enable_irq(nvt_pcie_chip_ptr, intr);
	}
//DBG_DUMP("Done\r\n");

	bMSIxOpened = TRUE;
	return E_OK;
}




ER msix_set_config(struct nvt_pcie_chip *nvt_pcie_chip_ptr, MSIx_CONFIG_ID cfg_id, u64 ui_config)
{
	ER	result = E_OK;
	PAXI_WRITE_ADDR	p_axi_write_addr;
	T_AXI_CMD_ADDR_LSB	axi_cmd_lsb = {0};
	T_AXI_CMD_ADDR_MSB	axi_cmd_msb = {0};
	T_AXI_CMD_MAU_ADDR_LSB  axi_cmd_mau_lsb = {0};
	T_AXI_CMD_MAU_ADDR_MSB  axi_cmd_mau_msb = {0};
	switch (cfg_id) {

	case MSIx_CONFIG_ID_SET_AXI_WRITE_ADDRESS:
		p_axi_write_addr = (PAXI_WRITE_ADDR)ui_config;
		axi_cmd_lsb.bit.AXI_CMD_ADDR_LSB = p_axi_write_addr->ui_axi_address_lsb;
		axi_cmd_msb.bit.AXI_CMD_ADDR_MSB = p_axi_write_addr->ui_axi_address_msb;
		axi_cmd_msb.bit.MSI_SEL = p_axi_write_addr->ui_msix_sel;
//DBG_DUMP("MSIX address[0x%08x]\r\n", nvt_pcie_chip_ptr->PCIE_REG_BASE[MEM_MSIX]);
		nvt_pcie_chip_ptr->ep.write_msix_ctrl(&nvt_pcie_chip_ptr->ep, AXI_CMD_ADDR_LSB_OFS, axi_cmd_lsb.reg);
//DBG_DUMP("MSIX address2[0x%08x]\r\n", nvt_pcie_chip_ptr->PCIE_REG_BASE[MEM_MSIX]);

		nvt_pcie_chip_ptr->ep.write_msix_ctrl(&nvt_pcie_chip_ptr->ep, AXI_CMD_ADDR_MSB_OFS, axi_cmd_msb.reg);
		DBG_IND("LSB[0x%08x]MSB[0x%08x]\r\n",
			nvt_pcie_chip_ptr->ep.read_msix_ctrl(&nvt_pcie_chip_ptr->ep, AXI_CMD_ADDR_LSB_OFS),
			nvt_pcie_chip_ptr->ep.read_msix_ctrl(&nvt_pcie_chip_ptr->ep, AXI_CMD_ADDR_MSB_OFS));
//DBG_DUMP("MSIX address3[0x%08x]\r\n", nvt_pcie_chip_ptr->PCIE_REG_BASE[MEM_MSIX]);

	break;

	case MSIx_CONFIG_ID_SET_2ND_AXI_WRITE_ADDRESS:
		p_axi_write_addr = (PAXI_WRITE_ADDR)ui_config;
		axi_cmd_mau_lsb.bit.AXI_CMD_ADDR_LSB = p_axi_write_addr->ui_axi_address_lsb;
		axi_cmd_mau_msb.bit.AXI_CMD_ADDR_MSB = p_axi_write_addr->ui_axi_address_msb;
		axi_cmd_mau_msb.bit.MSI_MAU_SEL = p_axi_write_addr->ui_msix_sel;
		nvt_pcie_chip_ptr->ep.write_msix_ctrl(&nvt_pcie_chip_ptr->ep,
			AXI_CMD_MAU_ADDR_LSB_OFS, axi_cmd_mau_lsb.reg);
		nvt_pcie_chip_ptr->ep.write_msix_ctrl(&nvt_pcie_chip_ptr->ep,
			AXI_CMD_MAU_ADDR_MSB_OFS, axi_cmd_mau_msb.reg);
		nvt_pcie_chip_ptr->ep.write_msix_ctrl(&nvt_pcie_chip_ptr->ep,
			AXI_CMD_MAU_DATA_OFS, 0xFFFFFFFF);
		DBG_IND("LSB[0x%08x]MSB[0x%08x]\r\n",
			nvt_pcie_chip_ptr->ep.read_msix_ctrl(&nvt_pcie_chip_ptr->ep, AXI_CMD_MAU_ADDR_LSB_OFS),
			nvt_pcie_chip_ptr->ep.read_msix_ctrl(&nvt_pcie_chip_ptr->ep, AXI_CMD_MAU_ADDR_MSB_OFS));
	break;

	default:
		result = E_NOSPT;
	break;
	}
	return result;
}


/**
    msix_enable_irq

    msix enable irq (in tx side)

    @return
        - void
*/
void msix_enable_irq(struct nvt_pcie_chip *nvt_pcie_chip_ptr, int number)
{
	UINT32	num_except_spi;
	unsigned int bank;
	num_except_spi = number - INT_GIC_SPI_START_ID;


	bank = num_except_spi>>5;
	number   = number & 0x1f;
	number   = 1 << number;
	nvt_pcie_chip_ptr->ep.write_msix_ctrl(&nvt_pcie_chip_ptr->ep,
		MSIx_MSIx_INT_SET_EN0_OFS + (bank<<2), number);		/* MSIx_SYS_INT_SET_EN0_OFS */
}

/**
    msix_disable_irq

    msix disable irq (in tx side)

    @return
        - void
*/
void msix_disable_irq(struct nvt_pcie_chip *nvt_pcie_chip_ptr, int number)
{
	UINT32	num_except_spi;
	unsigned int bank;

	num_except_spi = number - INT_GIC_SPI_START_ID;
	bank =  num_except_spi>>5;
	number   = number & 0x1f;
	number   = 1 << number;
	nvt_pcie_chip_ptr->ep.write_msix_ctrl(&nvt_pcie_chip_ptr->ep,
		MSIx_MSIx_INT_SET_CLR0_OFS + (bank<<2), number);     /* MSIx_SYS_INT_SET_CLR0_OFS */
}

/**
    msix_clear_msix_tx_irq

    msix msix_clear_msix_tx_irq irq (in tx side)

    @return
        - void
*/
void msix_clear_msix_tx_irq_status(struct nvt_pcie_chip *nvt_pcie_chip_ptr, int number)
{
	unsigned int bank;
#if(DUMMY_WRITE_DATA)
	const UINT32 dummy_addr_ofs = (number << 2);

	writel(0, nvt_pcie_chip_ptr->va_msix_dummy_addr + dummy_addr_ofs);
#endif

	bank = number>>5;
	number   = number & 0x1f;
	number   = 1 << number;
	nvt_pcie_chip_ptr->ep.write_msix_ctrl(&nvt_pcie_chip_ptr->ep,
		MSIx_MSIx_INT_STS0_OFS + (bank<<2), number);     /* MSIx_SYS_INT_SET_CLR0_OFS */
}

#if 1 // (in rx side)
void msix_recv_cmd_set_en_irq(struct nvt_pcie_chip *nvt_pcie_chip_ptr, UINT32 group, UINT32 number)
{
	nvt_pcie_chip_ptr->rc.write_msix_ctrl(&nvt_pcie_chip_ptr->rc,
		MSIx_RCV_AXI_CMD_Group_0_BIT_0_31_SET_EN_OFS + (group<<2), number);		/* MSIx_SYS_INT_SET_EN0_OFS */
}

void msix_recv_cmd_clr_en_irq(struct nvt_pcie_chip *nvt_pcie_chip_ptr, UINT32 group, UINT32 number)
{
	nvt_pcie_chip_ptr->rc.write_msix_ctrl(&nvt_pcie_chip_ptr->rc,
		MSIx_RCV_AXI_CMD_Group_0_BIT_0_31_CLR_EN_OFS + (group<<2), number);		/* MSIx_SYS_INT_SET_EN0_OFS */
}

void msix_recv_cmd_set_pending_irq(struct nvt_pcie_chip *nvt_pcie_chip_ptr, UINT32 group, UINT32 number)
{
	nvt_pcie_chip_ptr->rc.write_msix_ctrl(&nvt_pcie_chip_ptr->rc,
		MSIx_RCV_AXI_CMD_Group_0_BIT_0_31_SET_PENDING_OFS + (group<<2), number);		/* MSIx_SYS_INT_SET_EN0_OFS */
}

UINT32 msix_recv_cmd_get_pending_irq(struct nvt_pcie_chip *nvt_pcie_chip_ptr, UINT32 group)
{
	return nvt_pcie_chip_ptr->rc.read_msix_ctrl(&nvt_pcie_chip_ptr->rc,
		MSIx_RCV_AXI_CMD_Group_0_BIT_0_31_SET_PENDING_OFS + (group<<2));		/* MSIx_SYS_INT_SET_EN0_OFS */
}

UINT32 msix_recv_cmd_chk_pending_dummy_data(struct nvt_pcie_chip *nvt_pcie_chip_ptr, UINT32 number)
{
#if(DUMMY_WRITE_DATA)
	UINT32	dummy_data;
	const UINT32 dummy_addr_ofs = (number << 2);

	dummy_data = readl(nvt_pcie_chip_ptr->va_msix_dummy_addr + dummy_addr_ofs);

	printk("MSIx_TX -> PCIe -> MXIx RX receive isr[%2d]\r\n", number);
	if (dummy_data != 0) {
		printk("MSIx_TX2 addr[0x%08llx] = 0x%08x(dummy_data[0x%08x]) => Success)\r\n",
			nvt_pcie_chip_ptr->dma_msix_dummy_addr, dummy_data,
			nvt_pcie_chip_ptr->ep.read_msix_ctrl(&nvt_pcie_chip_ptr->ep, AXI_CMD_MAU_DATA_OFS));
	} else {
		printk("MSIx_TX2 addr[0x%08llx] = 0x%08x(dummy_data[0x%08x]) => Fail)\r\n",
			nvt_pcie_chip_ptr->dma_msix_dummy_addr, dummy_data,
			nvt_pcie_chip_ptr->ep.read_msix_ctrl(&nvt_pcie_chip_ptr->ep, AXI_CMD_MAU_DATA_OFS));
	}
//	printk("%s: hw irq %d, data 0x%x (base pa 0x%llx)\r\n", __func__,
//		number, dummy_data, nvt_pcie_chip_ptr->dma_msix_dummy_addr);
	return (dummy_data != 0);
#else
	return 1;
#endif
}

UINT32 msix_recv_cmd_get_enabled_irq(struct nvt_pcie_chip *nvt_pcie_chip_ptr, UINT32 group)
{
	return nvt_pcie_chip_ptr->rc.read_msix_ctrl(&nvt_pcie_chip_ptr->rc,
		MSIx_RCV_AXI_CMD_Group_0_BIT_0_31_SET_EN_OFS + (group<<2));		/* MSIx_SYS_INT_SET_EN0_OFS */
}

void msix_recv_cmd_clr_pending_irq(struct nvt_pcie_chip *nvt_pcie_chip_ptr, UINT32 group, UINT32 number)
{
	nvt_pcie_chip_ptr->rc.write_msix_ctrl(&nvt_pcie_chip_ptr->rc,
		MSIx_RCV_AXI_CMD_Group_0_BIT_0_31_CLR_PENDING_OFS + (group<<2), number);		/* MSIx_SYS_INT_SET_EN0_OFS */
}

#endif

void msix_proc_clear_pending(struct nvt_pcie_chip *nvt_pcie_chip_ptr)
{
	int i;
#if(DUMMY_WRITE_DATA)
	UINT32	dummy_addr_ofs;
#endif

	// clear RX
	nvt_pcie_chip_ptr->rc.write_msix_ctrl(&nvt_pcie_chip_ptr->rc,
		MSIx_RCV_AXI_CMD_Group_0_BIT_0_31_CLR_PENDING_OFS + 0x00, 0xFFFFFFFF);
	nvt_pcie_chip_ptr->rc.write_msix_ctrl(&nvt_pcie_chip_ptr->rc,
		MSIx_RCV_AXI_CMD_Group_0_BIT_0_31_CLR_PENDING_OFS + 0x04, 0xFFFFFFFF);
	nvt_pcie_chip_ptr->rc.write_msix_ctrl(&nvt_pcie_chip_ptr->rc,
		MSIx_RCV_AXI_CMD_Group_0_BIT_0_31_CLR_PENDING_OFS + 0x08, 0xFFFFFFFF);

	// clear TX
	for (i=0; i<INT_ID_MAX; i++) {
		msix_clear_msix_tx_irq_status(nvt_pcie_chip_ptr, i);

#if(DUMMY_WRITE_DATA)
		dummy_addr_ofs = (i << 2);
		if (dummy_write_data_area != NULL) {
			writel(0, dummy_write_data_area+dummy_addr_ofs);
		}
#endif
	}
}

void msix_proc_list_pending(struct seq_file *seq, struct nvt_pcie_chip *nvt_pcie_chip_ptr)
{
	UINT32	translate_int_id= 0;
	UINT32	intID_Enable;
	UINT32	intID_Pending;
	UINT32	uiBits;
	UINT32	i;
#if(DUMMY_WRITE_DATA)
	UINT32	dummy_addr_ofs;
	UINT32	dummy_data;
#endif

	for(i = 0; i < 4; i++) {
		intID_Enable = nvt_pcie_chip_ptr->rc.read_msix_ctrl(&nvt_pcie_chip_ptr->rc,
			MSIx_RCV_AXI_CMD_Group_0_BIT_0_31_SET_EN_OFS + (i << 2));
		intID_Pending = nvt_pcie_chip_ptr->rc.read_msix_ctrl(&nvt_pcie_chip_ptr->rc,
			MSIx_RCV_AXI_CMD_Group_0_BIT_0_31_SET_PENDING_OFS  + (i << 2));
//		printk("%s: %d: enable 0x%x, pending 0x%x\r\n", __func__, i, intID_Enable, intID_Pending);
//		intID_Pending &= intID_Enable;
		while (intID_Pending) {
			uiBits = __builtin_ctz(intID_Pending);
			translate_int_id = i * 32 + uiBits;
			msix_clear_msix_tx_irq_status(nvt_pcie_chip_ptr, translate_int_id);
			nvt_pcie_chip_ptr->rc.write_msix_ctrl(&nvt_pcie_chip_ptr->rc,
				MSIx_RCV_AXI_CMD_Group_0_BIT_0_31_CLR_PENDING_OFS + i * 4, (1<<uiBits));
			seq_printf(seq, "MSIx_TX -> PCIe -> MSIx latch ID[%2d] => Module [%s]\r\n", translate_int_id, msix_pcie_interrupt[translate_int_id]);
#if(DUMMY_WRITE_DATA)
			dummy_addr_ofs = (translate_int_id << 2);
			dummy_data = 0;
			if (dummy_write_data_area != NULL) {
				dummy_data = readl(dummy_write_data_area+dummy_addr_ofs);
			}
			if (dummy_data != 0) {
				seq_printf(seq, "MSIx_TX2 addr[0x%px] = 0x%08x(dummy_data[0x%08x]) => Success)\r\n", dummy_write_data_area+dummy_addr_ofs, *(UINT32 *)(dummy_write_data_area+dummy_addr_ofs), dummy_data);
			} else {
				seq_printf(seq, "MSIx_TX2 addr[0x%px] = 0x%08x(dummy_data[0x%08x]) => Fail)\r\n", dummy_write_data_area+dummy_addr_ofs, *(UINT32 *)(dummy_write_data_area+dummy_addr_ofs), dummy_data);
			}
			if (dummy_write_data_area != NULL) {
				writel(0, dummy_write_data_area+dummy_addr_ofs);
			}
#endif
			intID_Pending &= ~(1 << uiBits);
		}
	}
}

