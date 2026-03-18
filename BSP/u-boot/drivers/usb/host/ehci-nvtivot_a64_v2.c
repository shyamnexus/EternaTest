/*
 * Novatek IVOT USB HOST EHCI Controller
 *
 * Copyright (C) 2020 NOVATEK MICROELECTRONICS CORP
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <dm.h>
#include <fdtdec.h>
#include <malloc.h>
#include <usb.h>
#include <asm/io.h>
#include "ehci.h"
#include <asm/arch/IOAddress.h>
#include <asm/arch/hardware.h>
#include <asm/gpio.h>
#include <asm/nvt-common/nvt_types.h>
#include <asm/nvt-common/nvt_common.h>
#include <linux/libfdt.h>
#include <asm/arch/efuse_protected.h>
#include <linux/delay.h>
#include "ehci_nvtivot_reg_v2.h"

/* Declare global data pointer */
DECLARE_GLOBAL_DATA_PTR;
#define U2PHY_SETREG(ofs,value)         writel((value), (volatile void __iomem *)(IOADDR_USBPHY_REG_BASE + 0x1000 + ((ofs)<<2)))
#define U2PHY_GETREG(ofs)               readl((volatile void __iomem *)(IOADDR_USBPHY_REG_BASE + 0x1000 + ((ofs)<<2)))
#if defined(CONFIG_TARGET_NA51102_A64) || defined(CONFIG_TARGET_NS02201_A64) || defined(CONFIG_TARGET_NS02401_A64)
#define U3PHY_SETREG(ofs,value)         writel((value), (volatile void __iomem *)(IOADDR_USB3PHY_REG_BASE + 0x2000 + ((ofs)<<2)))
#define U3PHY_GETREG(ofs)               readl((volatile void __iomem *)(IOADDR_USB3PHY_REG_BASE + 0x2000 + ((ofs)<<2)))
#define U3_U2PHY_SETREG(ofs,value)      writel((value), (volatile void __iomem *)(IOADDR_USB3PHY_REG_BASE + 0x1000 + ((ofs)<<2)))
#define U3_U2PHY_GETREG(ofs)            readl((volatile void __iomem *)(IOADDR_USB3PHY_REG_BASE + 0x1000 + ((ofs)<<2)))
#endif
#define MAP_PHY(ofs)                    ((ofs)>>2)

#define TRIM_RESINT_UPPER 0x14
#define TRIM_RESINT_LOWER 0x1

#define DRV_VERSION                     "1.00.12"

static REGVALUE usb_getreg(unsigned long usbbase, unsigned long offset)
{
	return readl((volatile unsigned long *)(usbbase + offset));
}

static void usb_setreg(unsigned long usbbase,  unsigned long offset, unsigned long value)
{
	writel(value, (volatile unsigned long *)(usbbase + offset));
}

static void nvtim_set_gpio_power(int index)
{
	/* Enable it after dts parsing ready*/
	int nodeoffset;
	u32 *cell = NULL;
	char path[20] = {0};
	int gpio_pin, gpio_value, err;

	if (index) {
		//for two controller case
		#if defined(NVT_ECHI_2ND_CTRL)
#if defined(CONFIG_ARM64)
		sprintf(path,"/usb20host@%ld,%lx", (IOADDR_GLOBAL_BASE >> 32L), (IOADDR_USB2_REG_BASE & 0xFFFFFFFF));
#else
		sprintf(path,"/usb20host@%x",(IOADDR_USB2_REG_BASE & 0xFFFFFFFF));
#endif
		#endif

	} else {
#if defined(CONFIG_ARM64)
		sprintf(path,"/usb20host@%ld,%lx", (IOADDR_GLOBAL_BASE >> 32L), (IOADDR_USB_REG_BASE & 0xFFFFFFFF));
#else
		sprintf(path,"/usb20host@%x",(IOADDR_USB_REG_BASE & 0xFFFFFFFF));
#endif
	}

	nodeoffset = fdt_path_offset((const void*)nvt_fdt_buffer, path);

	cell = (u32*)fdt_getprop((const void*)nvt_fdt_buffer, nodeoffset, "power_gpio", NULL);

	if (cell > 0) {
		gpio_pin = __be32_to_cpu(cell[0]);
		gpio_value = __be32_to_cpu(cell[1]);

		err = gpio_request(gpio_pin, "vbus");
		if (err) {
			printf("#### failed to request vbus, %d\n", gpio_pin);
			return;
		}

		gpio_direction_output(gpio_pin, gpio_value);
		gpio_free(gpio_pin);
	} else {
		printf("no power gpio found\n");
	}
}

#ifdef CONFIG_DM_USB
struct uboot_ehci_nvt_priv_data{
	struct ehci_ctrl ctrl; /* Needed by EHCI */
	struct ehci_hccr *hccr;
	struct ehci_hcor *hcor;
};

static int ehci_usb_ofdata_to_platdata(struct udevice *dev)
{
	return 0;
}
#endif

static int nvt_get_port_speed(struct ehci_ctrl *ctrl, uint32_t reg)
{
	unsigned tmp = ehci_readl(((void *)ctrl->hcor) + 0x70);

	switch ((tmp >> 22) & 3) {
		case 0:
			return PORTSC_PSPD_FS;
		case 1:
			return PORTSC_PSPD_LS;
		case 2:
		default:
			return PORTSC_PSPD_HS;
	}
}

const struct ehci_ops nvt_ehci_ops = {
	.set_usb_mode		= NULL,
	.get_port_speed		= nvt_get_port_speed,
	.powerup_fixup		= NULL,
	.get_portsc_register	= NULL,
};

static void nvtim_init_usbhc(int index)
{
	unsigned long usbbase;
	unsigned long tmpval = 0;

	if (index) {
		//for two controller case
#if defined(NVT_ECHI_2ND_CTRL)
		usbbase = IOADDR_USB2_REG_BASE;

		printf("%s %d: controller 1\n", __FUNCTION__, __LINE__);
		/* Enable Reset */
		tmpval = readl((volatile unsigned long *)(IOADDR_CG_REG_BASE+CG_U2_1_RST));
		tmpval &= ~(0x1<<CG_U2_1_RST_BIT);
		writel(tmpval, (volatile unsigned long *)(IOADDR_CG_REG_BASE+CG_U2_1_RST));

		udelay(10);

		/* Disable Reset */
		tmpval = readl((volatile unsigned long *)(IOADDR_CG_REG_BASE+CG_U2_1_RST));
		tmpval |= 0x1<<CG_U2_1_RST_BIT;
		writel(tmpval, (volatile unsigned long *)(IOADDR_CG_REG_BASE+CG_U2_1_RST));

		/* Release sram shutdown */
		tmpval = readl((volatile unsigned long *)(IOADDR_TOP_REG_BASE+TOP_U2_1_SRAMSD));
		tmpval &= ~(0x1<<TOP_U2_1_SRAMSD_BIT);
		writel(tmpval, (volatile unsigned long *)(IOADDR_TOP_REG_BASE+TOP_U2_1_SRAMSD));

	#if defined(CG_U2_1_EN)
		tmpval = readl((volatile unsigned long *)(IOADDR_CG_REG_BASE+CG_U2_1_EN));
		tmpval |= (0x1<<CG_U2_1_EN_BIT);
		writel(tmpval, (volatile unsigned long *)(IOADDR_CG_REG_BASE+CG_U2_1_EN));
	#endif

	#if defined(CG_U2_1_PHYRST)
		// enable phy reset
		tmpval = readl((volatile unsigned long *)(IOADDR_CG_REG_BASE + CG_U2_1_PHYRST));
		tmpval &= ~(0x1 << CG_U2_1_PHYRST_BIT);
		writel(tmpval, (volatile unsigned long *)(IOADDR_CG_REG_BASE + CG_U2_1_PHYRST));

		udelay(10);

		// disable phy reset
		tmpval = readl((volatile unsigned long *)(IOADDR_CG_REG_BASE + CG_U2_1_PHYRST));
		tmpval |= (0x1 << CG_U2_1_PHYRST_BIT);
		writel(tmpval, (volatile unsigned long *)(IOADDR_CG_REG_BASE + CG_U2_1_PHYRST));
	#endif
#endif
	} else {
		usbbase = IOADDR_USB_REG_BASE;

		printf("%s %d: controller 0\n", __FUNCTION__, __LINE__);
		/* Enable Reset */
		tmpval = readl((volatile unsigned long *)(IOADDR_CG_REG_BASE+CG_U2_0_RST));
		tmpval &= ~(0x1<<CG_U2_0_RST_BIT);
		writel(tmpval, (volatile unsigned long *)(IOADDR_CG_REG_BASE+CG_U2_0_RST));

		udelay(10);

		/* Disable Reset */
		tmpval = readl((volatile unsigned long *)(IOADDR_CG_REG_BASE+CG_U2_0_RST));
		tmpval |= 0x1<<CG_U2_0_RST_BIT;
		writel(tmpval, (volatile unsigned long *)(IOADDR_CG_REG_BASE+CG_U2_0_RST));

		/* Release sram shutdown */
		tmpval = readl((volatile unsigned long *)(IOADDR_TOP_REG_BASE+TOP_U2_0_SRAMSD));
		tmpval &= ~(0x1<<TOP_U2_0_SRAMSD_BIT);
		writel(tmpval, (volatile unsigned long *)(IOADDR_TOP_REG_BASE+TOP_U2_0_SRAMSD));

		/* Enable clock and Reset PHY */
#if defined(CONFIG_TARGET_NA51102_A64)
		{
			unsigned long tmpval2 = 0;

			tmpval = readl((volatile unsigned long *)(IOADDR_STBC_CG_REG_BASE+CG_U2_0_EN));
			tmpval |= (0x1<<CG_U2_0_EN_BIT);
			writel(tmpval, (volatile unsigned long *)(IOADDR_STBC_CG_REG_BASE+CG_U2_0_EN));

			/* reset u2 phy */
			tmpval2 = readl((volatile unsigned long *)(IOADDR_USB3PHY_REG_BASE+0x28));
			if (tmpval2 & 0x80) {
				//do nothing, since xhci has do the phy reset
			} else {
				// enable phy reset
				tmpval = readl((volatile unsigned long *)(IOADDR_CG_REG_BASE+0x9C));
				tmpval &= ~(0x1<<7);
				writel(tmpval, (volatile unsigned long *)(IOADDR_CG_REG_BASE+0x9C));

				udelay(10);

				// disable phy reset
				tmpval = readl((volatile unsigned long *)(IOADDR_CG_REG_BASE+0x9C));
				tmpval |= (0x1<<7);
				writel(tmpval, (volatile unsigned long *)(IOADDR_CG_REG_BASE+0x9C));

				// mark the reset flag
				tmpval2 |= 0x80;
				writel(tmpval2, (volatile unsigned long *)(IOADDR_USB3PHY_REG_BASE+0x28));
			}
		}
#else
	#if defined(CG_U2_0_EN)
		tmpval = readl((volatile unsigned long *)(IOADDR_CG_REG_BASE+CG_U2_0_EN));
		tmpval |= (0x1<<CG_U2_0_EN_BIT);
		writel(tmpval, (volatile unsigned long *)(IOADDR_CG_REG_BASE+CG_U2_0_EN));
	#endif

	#if defined(CG_U2_0_PHYRST)
		// enable phy reset
		tmpval = readl((volatile unsigned long *)(IOADDR_CG_REG_BASE + CG_U2_0_PHYRST));
		tmpval &= ~(0x1 << CG_U2_0_PHYRST_BIT);
		writel(tmpval, (volatile unsigned long *)(IOADDR_CG_REG_BASE + CG_U2_0_PHYRST));

		udelay(10);

		// disable phy reset
		tmpval = readl((volatile unsigned long *)(IOADDR_CG_REG_BASE + CG_U2_0_PHYRST));
		tmpval |= (0x1 << CG_U2_0_PHYRST_BIT);
		writel(tmpval, (volatile unsigned long *)(IOADDR_CG_REG_BASE + CG_U2_0_PHYRST));
	#endif
#endif
	}

	mdelay(10);

	/* Set INT_POLARITY */
	union USB_GLOBALINTMASK_REG usbgintmskreg;
	usbgintmskreg.reg = usb_getreg(usbbase, USB_GLOBALINTMASK_REG_OFS);
	usbgintmskreg.bit.INT_POLARITY = 1;
	usb_setreg(usbbase, USB_GLOBALINTMASK_REG_OFS, usbgintmskreg.reg);

	/* Clear DEVPHY_SUSPEND[5] */
	if (nvt_get_chip_id() == CHIP_NA51055) {
		tmpval = readl((volatile unsigned long *)(usbbase+USB_DEVDMACTRL1_REG_OFS));
		tmpval &= ~(0x1<<5);
		writel(tmpval, (volatile unsigned long *)(usbbase+USB_DEVDMACTRL1_REG_OFS));
	} else {
		union USB_DEVDMACTRL1_REG usbctrl1reg;
		usbctrl1reg.reg = usb_getreg(usbbase, USB_DEVDMACTRL1_REG_OFS);
		usbctrl1reg.bit.DEVPHY_SUSPEND = 0;
		usb_setreg(usbbase, USB_DEVDMACTRL1_REG_OFS, usbctrl1reg.reg);
	}
	/* Set USB ID & VBUSI */
	union USB_PHYTOP_REG usbphytopreg;
	if (nvt_get_chip_id() == CHIP_NA51055) {
		usbphytopreg.reg = usb_getreg(usbbase, USB_PHYTOP_REG_OFS);
		usbphytopreg.bit.USB_VBUSI = 1;
                usbphytopreg.bit.USB_ID = 0;
	} else {
		usbphytopreg.reg = usb_getreg(usbbase, USB_PHYTOP_REG_OFS);
		usbphytopreg.reg &= ~0xFF;
		usbphytopreg.reg |= 0x22;
		usbphytopreg.bit.USB_VBUSI = 1;
		usbphytopreg.bit.USB_ID = 0;
	}
	usb_setreg(usbbase, USB_PHYTOP_REG_OFS, usbphytopreg.reg);

	/* Clear FORCE_FS[9] and handle HALF_SPEED[1] */
	union USB_DEVMAINCTRL_REG devmainctrlreg;
	devmainctrlreg.reg = usb_getreg(usbbase, USB_DEVMAINCTRL_REG_OFS);
	devmainctrlreg.bit.FORCE_FS = 0;
#ifdef CONFIG_NVT_FPGA_EMULATION
	devmainctrlreg.bit.HALF_SPEED = 1;
#endif
	usb_setreg(usbbase, USB_DEVMAINCTRL_REG_OFS, devmainctrlreg.reg);

	/* Clear HOSTPHY_SUSPEND[6] */
	usbphytopreg.reg = usb_getreg(usbbase, USB_PHYTOP_REG_OFS);
	usbphytopreg.bit.USB_VBUSI = 0;
	usb_setreg(usbbase, USB_PHYTOP_REG_OFS, usbphytopreg.reg);


	union USB_HCMISC_REG usbhcmiscreg;
	usbhcmiscreg.reg = usb_getreg(usbbase, USB_HCMISC_REG_OFS);
	usbhcmiscreg.bit.HostPhy_Suspend = 1;
	usb_setreg(usbbase, USB_HCMISC_REG_OFS, usbhcmiscreg.reg);

	mdelay(1);

	usbphytopreg.reg = usb_getreg(usbbase, USB_PHYTOP_REG_OFS);
	usbphytopreg.bit.USB_VBUSI = 1;
	usb_setreg(usbbase, USB_PHYTOP_REG_OFS, usbphytopreg.reg);

	if (nvt_get_chip_id() == CHIP_NA51055) {
		usbhcmiscreg.reg = usb_getreg(usbbase, USB_HCMISC_REG_OFS);
		tmpval &= ~(0x3F<<8);
		tmpval |=  (0x22<<8);
		usbhcmiscreg.bit.HostPhy_Suspend = 0;
		usb_setreg(usbbase, USB_HCMISC_REG_OFS, usbhcmiscreg.reg);
	} else {
		usbhcmiscreg.reg = usb_getreg(usbbase, USB_HCMISC_REG_OFS);
		usbhcmiscreg.reg &= ~(0x3F << 16);
		usbhcmiscreg.reg |=  (0x22 << 16);
		usbhcmiscreg.bit.HostPhy_Suspend = 0;
		usb_setreg(usbbase, USB_HCMISC_REG_OFS, usbhcmiscreg.reg);
	}

	mdelay(25);

	/* A_BUS_DROP[5] = 0 */
	union USB_OTGCTRLSTATUS_REG otgctrlstsreg;
	otgctrlstsreg.reg = usb_getreg(usbbase,USB_OTGCTRLSTATUS_REG_OFS);
	otgctrlstsreg.bit.A_BUS_DROP = 0;
	usb_setreg(usbbase, USB_OTGCTRLSTATUS_REG_OFS, otgctrlstsreg.reg);

	udelay(2);

	/* A_BUS_REQ[4] = 1 */
	otgctrlstsreg.reg = usb_getreg(usbbase, USB_OTGCTRLSTATUS_REG_OFS);
	otgctrlstsreg.bit.A_BUS_REQ = 1;
	usb_setreg(usbbase, USB_OTGCTRLSTATUS_REG_OFS, otgctrlstsreg.reg);

	/* Clear EOF1=3[3:2] EOF2[5:4]=0 */
	usbhcmiscreg.reg = usb_getreg(usbbase, USB_HCMISC_REG_OFS);
	usbhcmiscreg.bit.EOF2_Time = 0;
	usbhcmiscreg.bit.EOF1_Time = 0;
	usbhcmiscreg.bit.EOF1_Time = 0x3;
	usb_setreg(usbbase, USB_HCMISC_REG_OFS, usbhcmiscreg.reg);

	/* disable fifo empty interrupt */
	tmpval = readl((volatile unsigned long *)(usbbase+0x3AC));
	tmpval |= 0xFFFF;
	writel(tmpval, (volatile unsigned long *)(usbbase+0x3AC));

#if defined(CONFIG_TARGET_NS02201_A64) || defined(CONFIG_TARGET_NS02401_A64) || defined(CONFIG_TARGET_NA51102_A64)
	/* RESP MODE open */
	tmpval = readl((volatile unsigned long *)(usbbase+0x404));
	tmpval |= (0x1<<30);
	writel(tmpval, (volatile unsigned long *)(usbbase+0x404));

	/* AXI channel open */
	tmpval = readl((volatile unsigned long *)(usbbase+0x404));
	tmpval &= ~(0x1<<16);
	writel(tmpval, (volatile unsigned long *)(usbbase+0x404));
#endif

#ifndef CONFIG_NVT_FPGA_EMULATION
	/* Configure PHY related settings below */
#if defined(CONFIG_TARGET_NS02201_A64)
	{
		u8 u2_trim_swctrl = 6, u2_trim_sqsel = 4, u3_trim_rint_sel = 8;
		u32 trim;
		int code;
		bool is_found;

		// get trim value of usb column
		code = otp_key_manager(EFUSE_TRIM_DATA_USB3_1_USB2_DUAL_PHY_PRI);
		if(code == -33) {
			//!!!Please apply default value here!!!
			printf("Read EFUSE_TRIM_DATA_USB3_1_USB2_DUAL_PHY_PRI error, use default value\r\n");
		} else {
			// check if usb trim value is valid
			is_found = extract_trim_valid(code, (u32 *)&trim);
			if(is_found) {
				printf("[690] USB2 Trim data raw(0x2A) = 0x%04x\r\n", (int)trim);
				u3_trim_rint_sel = trim & 0x1F; //12K trim value

				//valid range:  1<= 12K trim value <= 20(0x14)
				if ((TRIM_RESINT_LOWER <= u3_trim_rint_sel) && (u3_trim_rint_sel <= TRIM_RESINT_UPPER)) {
					printf("[690] USB2 Trim data(12K) = 0x%04x\r\n", (int)u3_trim_rint_sel);
				} else {
					u3_trim_rint_sel = 8;
					printf("[690] USB2 Trim data(12K) is out of valid range: 0x%04x, use default value: 0x%04x\r\n", (int)(trim&0x1F), (int)u3_trim_rint_sel);
				}
			} else {
				//!!!Please apply default value here!!!
				pr_err("[690] USB2 Trim data raw(0x2A) = NULL\n");
				printf("is found [%d][ USB] = 0x%08x\r\n", is_found, (int)code);
			}
		}

		//select PHY internal rsel value
		tmpval = U3PHY_GETREG(0x1A1);
		tmpval &= ~(0x80);
		U3PHY_SETREG(0x1A1, tmpval);

		//r45 trim setting
		U3PHY_SETREG(0x1A0, 0x40 + u3_trim_rint_sel);
		U3PHY_SETREG(0x1A3, 0x60);

		//best setting
		U3PHY_SETREG(0x56, 0x74);
		U3_U2PHY_SETREG(0x9, 0xB0);
		U2PHY_SETREG(0x9, 0xB0);
		U3_U2PHY_SETREG(0x12, 0x31);
		U2PHY_SETREG(0x12, 0x31);
		U3_U2PHY_SETREG(0x6, 0x1C);
		U2PHY_SETREG(0x6, 0x1C);

		//r45 cali
		U3_U2PHY_SETREG(0x51, 0x20);
		mdelay(5);
		U3_U2PHY_SETREG(0x51, 0x00);
	}
#elif defined(CONFIG_TARGET_NA51102_A64)
	{
		u8 u2_trim_swctrl = 4, u2_trim_sqsel = 4, u3_trim_rint_sel = 8;
		u32 trim;
		int code;
		bool is_found;

		// get trim value of usb column
		code = otp_key_manager(EFUSE_USB_SATA_TRIM_DATA);
		if(code == -33) {
			//!!!Please apply default value here!!!
			printf("Read USB trim error, use default value\r\n");
		} else {
			// check if usb trim value is valid
			is_found = extract_trim_valid(code, (u32 *)&trim);
			if(is_found) {
				if ((TRIM_RESINT_LOWER <= (trim & 0x1F)) && ((trim & 0x1F) <= TRIM_RESINT_UPPER)) {
					u3_trim_rint_sel = trim & 0x1F;
					printf("is found [%d][USB] 12K_resistor Trim data = 0x%04x\r\n", is_found, (int)u3_trim_rint_sel);
				} else {
					printf("data is out of valid range\n");
				}
			} else {
				//!!!Please apply default value here!!!
				printf("is found [%d][ USB] = 0x%08x\r\n", is_found, (int)code);
			}
		}

		//select PHY internal rsel value
		tmpval = U3PHY_GETREG(0x1A1);
		tmpval &= ~(0x80);
		U3PHY_SETREG(0x1A1, tmpval);

		//r45 trim setting
		U3PHY_SETREG(0x1A0, 0x40 + u3_trim_rint_sel);
		U3PHY_SETREG(0x1A3, 0x60);

		//best setting
		U3PHY_SETREG(0x56, 0x74);
		U3_U2PHY_SETREG(0x9, 0xB0);
		U2PHY_SETREG(0x9, 0xB0);
		U3_U2PHY_SETREG(0x12, 0x31);
		U2PHY_SETREG(0x12, 0x31);

		//r45 cali
		U3_U2PHY_SETREG(0x51, 0x20);
		mdelay(5);
		U3_U2PHY_SETREG(0x51, 0x00);
	}
#elif defined(CONFIG_TARGET_NS02301) || defined(CONFIG_TARGET_NA51089) || defined(CONFIG_TARGET_NA51055)
	{
		u8 u2_trim_swctrl = 4, u2_trim_sqsel = 4, u2_trim_resint = 8;

		// Get trim value of usb column
		{
			int code;
			bool is_found;
			u32 trim;

			code = otp_key_manager(1);
			if (code == -33) {
				//!!!Please apply default value here!!!
				printf("%s: Read USB trim error, use default value\r\n", __func__);
			} else {
				// check if usb trim value is valid
				is_found = extract_trim_valid(code, (u32 *)&trim);
				if (is_found) {
					if ((nvt_get_chip_id() == CHIP_NA51089) || (nvt_get_chip_id() == CHIP_NA51055) || (nvt_get_chip_id() == CHIP_NA51084)) {
						printf("%s: [trim_rawdata(0x%04x)] [trim(0x%04x)]\r\n", __func__, (int)trim, (int)(trim >> 6));
						trim = (trim >> 6);
					}

					if ((TRIM_RESINT_LOWER <= (trim & 0x1F)) && ((trim & 0x1F) <= TRIM_RESINT_UPPER)) {
						u2_trim_resint = trim & 0x1F;
						printf("%s: [is_found(%d) code(%d)] [trim(0x%04x) 12K_resistor(0x%04x)]\r\n", __func__, code, is_found, (int)trim, (int)u2_trim_resint);
					} else {
						//!!!Please apply default value here!!!
						printf("%s: [is_found(%d) code(%d)] [trim(0x%04x)], trim out of range!!!\r\n", __func__, code, is_found, (int)trim);
					}
				} else {
					//!!!Please apply default value here!!!
					printf("%s: [is_found(%d) code(%d)] [trim(0x%04x)], trim not found!!!\r\n", __func__, code, is_found, (int)trim);
				}
			}
		}

		if (nvt_get_chip_id() == CHIP_NS02301) {
			// best setting
			U2PHY_SETREG(MAP_PHY(0x14), 0xd3);   // Port0 VCHSEL = 11
			U2PHY_SETREG(MAP_PHY(0x180), 0x01);  // sw datarpusel = 0
			U2PHY_SETREG(MAP_PHY(0x30), 0x20);   // Port0 datarpu1 = 1
			U2PHY_SETREG(MAP_PHY(0x2c), 0x20);   // sw set enable of reg_datarpu1 = 1
			U2PHY_SETREG(MAP_PHY(0x18c), 0x60);  // sw reg_tx_cs_pdn = 1
		}

		// r45 cali lock
		U2PHY_SETREG(MAP_PHY(0x144), 0x20);
		mdelay(5);
		U2PHY_SETREG(MAP_PHY(0x140), 0x30);

		// tx swing
		tmpval = U2PHY_GETREG(MAP_PHY(0x18));
		tmpval &= ~(0x7 << 1);
		tmpval |= (u2_trim_swctrl << 0x1);
		U2PHY_SETREG(MAP_PHY(0x18), tmpval);

		// squelch
		tmpval = U2PHY_GETREG(MAP_PHY(0x14));
		tmpval &= ~(0x7 << 0x2);
		tmpval |= (u2_trim_sqsel << 0x2);
		U2PHY_SETREG(MAP_PHY(0x14), tmpval);

		// res
		U2PHY_SETREG(MAP_PHY(0x148), 0x60 + u2_trim_resint);

		// r45 cali unlock
		U2PHY_SETREG(MAP_PHY(0x144), 0x00);
	}
#elif defined(CONFIG_TARGET_NA51103) || defined(CONFIG_TARGET_NA51103_A64)
	{
		u8 u2_trim_swctrl=4, u2_trim_sqsel=4, u2_trim_resint=8;
		u32 trim;
		int code;
		bool is_found;

		// get trim value of usb column
		code = otp_key_manager(OTP_USB_SET1_TRIM_FIELD);
		if(code == -33) {
			//!!!Please apply default value here!!!
			printf("Read USB trim error\r\n");
		} else {
			// check if usb trim value is valid
			is_found = extract_2_set_type_trim_valid(TRIM_USB_FIELD, code, (u32 *)&trim);
			if(is_found) {
				if ((TRIM_RESINT_LOWER <= trim) && (trim <= TRIM_RESINT_UPPER)) {
					u2_trim_resint = trim;
					printf("is found [%d][%d][USB] = 0x%04x\r\n", is_found, OTP_USB_SET1_TRIM_FIELD, (int)trim);
					printf("        => 12K_resistor[0x%04x]\r\n", (int)u2_trim_resint);
				} else {
					printf("data is out of valid range\n");
				}
			} else {
				//!!!Please apply default value here!!!
				printf("is found [%d][ USB] = 0x%08x\r\n", is_found, (int)code);
			}
		}

		if (index)
			usbbase = IOADDR_USB2PHY_REG_BASE;
		else
			usbbase = IOADDR_USBPHY_REG_BASE;
		writel(0x20, (volatile unsigned long *)(usbbase+USB_PHYCTRL51_REG_OFS));
		writel(0x30, (volatile unsigned long *)(usbbase+USB_PHYCTRL50_REG_OFS));
		writel(0x60 + u2_trim_resint, (volatile unsigned long *)(usbbase+USB_PHYCTRL52_REG_OFS));
		mdelay(5);
		writel(0x00, (volatile unsigned long *)(usbbase+USB_PHYCTRL51_REG_OFS));
		mdelay(5);

		tmpval = readl((volatile unsigned long *)(usbbase+USB_PHYCTRL06_REG_OFS));
		tmpval &= ~(0x7 << 1);
		tmpval |= (u2_trim_swctrl << 0x1);
		writel(tmpval, (volatile unsigned long *)(usbbase+USB_PHYCTRL06_REG_OFS));

		tmpval = readl((volatile unsigned long *)(usbbase+USB_PHYCTRL05_REG_OFS));
		tmpval &= ~(0x7 << 0x2);
		tmpval |= (u2_trim_sqsel << 0x2);
		writel(tmpval, (volatile unsigned long *)(usbbase+USB_PHYCTRL05_REG_OFS));


		if (index)
			usbbase = IOADDR_USB2_REG_BASE;
		else
			usbbase = IOADDR_USB_REG_BASE;

		tmpval = readl((volatile unsigned long *)(usbbase+USB_PHYTOP_REG_OFS));
		tmpval &= ~(0x1F << 8);
		tmpval |= (u2_trim_resint << 8);
		tmpval |= (0x1 << 15);
		writel(tmpval, (volatile unsigned long *)(usbbase+USB_PHYTOP_REG_OFS));
	}
#endif
#endif // #ifndef CONFIG_NVT_FPGA_EMULATION
}

static void nvtim_remove_power_save(void)
{
	printf("ehci_nvt: Enter powerdown script\r\n");

	// IP provided to make PHY deeply sleeping
	U2PHY_SETREG(MAP_PHY(0xD4), 0xB4);   // Port0 dmpulldown = 1, dppulldown = 0, suspendm = 1, termselect = 1, xcvrselect = 01, opmode = 00
	U2PHY_SETREG(MAP_PHY(0xD0), 0xFF);   // Enable port0 sw reg 0xF060_10D4
	U2PHY_SETREG(MAP_PHY(0x08), 0x14);   // Port0 pll_pdn = 0, pll_rstn = 0
	U2PHY_SETREG(MAP_PHY(0x00), 0x8A);   // Port0 bias_pdn = 0, ldo_pdn = 0
	U2PHY_SETREG(MAP_PHY(0x30), 0x13);   // Port0 datarpu1 = 0, datarpu2 = 1, spsel = 0, datarpusel = 0, datarpdp = 1, datarpdm = 1
	U2PHY_SETREG(MAP_PHY(0x2C), 0xFF);   // Enable port0 sw reg 0x0030
	U2PHY_SETREG(MAP_PHY(0x28), 0x20);   // Port0 usb_pwr_ok = 0
	U2PHY_SETREG(MAP_PHY(0x24), 0xB3);   // Port0 u11_rx_pwd = 1
	U2PHY_SETREG(MAP_PHY(0x34), 0x90);   // Port0 tx_pdn = 0
}

#ifndef CONFIG_DM_USB
/*
 * Create the appropriate control structures to manage
 * a new EHCI host controller.
 */
int ehci_hcd_init(int index, enum usb_init_type init,
		struct ehci_hccr **hccr, struct ehci_hcor **hcor)
{
	nvtim_init_usbhc(index);

	if (index) {
		//two controller case
		#if defined(NVT_ECHI_2ND_CTRL)
		*hccr = (struct ehci_hccr *)IOADDR_USB2_REG_BASE;
		#endif
	} else {
		*hccr = (struct ehci_hccr *)IOADDR_USB_REG_BASE;
	}

	*hcor = (struct ehci_hcor *)((unsigned long)*hccr +
			HC_LENGTH(ehci_readl(&(*hccr)->cr_capbase)));

	nvtim_set_gpio_power(index);

	ehci_set_controller_priv(index, NULL, &nvt_ehci_ops);

	printf("nvtimusb 0x%lx %s\n", (unsigned long) *hccr, DRV_VERSION);

	return 0;
}

/*
 * Destroy the appropriate control structures corresponding
 * the the EHCI host controller.
 */
int ehci_hcd_stop(int index)
{
	u32 usbbase;
	u32 tmpval = 0;

	if (index) {
		//two controller case
		#if defined(NVT_ECHI_2ND_CTRL)
		usbbase = IOADDR_USB2_REG_BASE;
		#endif
	} else {
		usbbase = IOADDR_USB_REG_BASE;
	}

	/* Host Reset */
	union USB_MCR_REG usbmcrreg;
	usbmcrreg.reg = usb_getreg(usbbase, USB_MCR_REG_OFS);
	usbmcrreg.bit.HC_RESET = 1;
	usb_setreg(usbbase, USB_MCR_REG_OFS, usbmcrreg.reg);

	/* A_BUS_REQ[4] = 0 */
	union USB_OTGCTRLSTATUS_REG otgctrlstsreg;
	otgctrlstsreg.reg = usb_getreg(usbbase, USB_OTGCTRLSTATUS_REG_OFS);
	otgctrlstsreg.bit.A_BUS_REQ = 0;
	usb_setreg(usbbase, USB_OTGCTRLSTATUS_REG_OFS, otgctrlstsreg.reg);

	/* A_BUS_DROP[5] = 1 */
	otgctrlstsreg.reg = usb_getreg(usbbase, USB_OTGCTRLSTATUS_REG_OFS);
	otgctrlstsreg.bit.A_BUS_DROP = 1;
	usb_setreg(usbbase, USB_OTGCTRLSTATUS_REG_OFS, otgctrlstsreg.reg);

	if (index) {
		//two controller case
		#if defined(NVT_ECHI_2ND_CTRL)
		printf("%s %d: controller 1\n", __FUNCTION__, __LINE__);
		/* Enable Reset*/
		tmpval = readl((volatile unsigned long *)(IOADDR_CG_REG_BASE+CG_U2_1_RST));
		tmpval &= ~(0x1<<CG_U2_1_RST_BIT);
		writel(tmpval, (volatile unsigned long *)(IOADDR_CG_REG_BASE+CG_U2_1_RST));

		udelay(10);

		/* Disable Reset*/
		tmpval = readl((volatile unsigned long *)(IOADDR_CG_REG_BASE+CG_U2_1_RST));
		tmpval |= 0x1<<CG_U2_1_RST_BIT;
		writel(tmpval, (volatile unsigned long *)(IOADDR_CG_REG_BASE+CG_U2_1_RST));
		#endif
	} else {
		printf("%s %d: controller 0\n", __FUNCTION__, __LINE__);
		/* Enable Reset*/
		tmpval = readl((volatile unsigned long *)(IOADDR_CG_REG_BASE+CG_U2_0_RST));
		tmpval &= ~(0x1<<CG_U2_0_RST_BIT);
		writel(tmpval, (volatile unsigned long *)(IOADDR_CG_REG_BASE+CG_U2_0_RST));

		udelay(10);

		/* Disable Reset*/
		tmpval = readl((volatile unsigned long *)(IOADDR_CG_REG_BASE+CG_U2_0_RST));
		tmpval |= 0x1<<CG_U2_0_RST_BIT;
		writel(tmpval, (volatile unsigned long *)(IOADDR_CG_REG_BASE+CG_U2_0_RST));
	}

#if defined(CONFIG_TARGET_NA51102_A64)
	{
		unsigned long tmpval2 = 0;

		/* clear reset flag*/
		tmpval2 = readl((volatile unsigned long *)(IOADDR_USB3PHY_REG_BASE+0x28));
		if (tmpval2 & 0x80) {
			tmpval2 &= ~(0x80);
			writel(tmpval2, (volatile unsigned long *)(IOADDR_USB3PHY_REG_BASE+0x28));
		} else {
			//do nothing.
		}
	}
#endif

	if (nvt_get_chip_id() == CHIP_NS02301) {
		nvtim_remove_power_save();
	}

	return 0;
}

#else
static void nvtim_init_usbhc_dm(int index, struct ehci_hccr **hccr, struct ehci_hcor **hcor)
{
	nvtim_init_usbhc(index);

	if (index == 0)
		*hccr = (struct ehci_hccr *)IOADDR_USB_REG_BASE;
	else {
		//two controller case
		#if defined(NVT_ECHI_2ND_CTRL)
		*hccr = (struct ehci_hccr *)IOADDR_USB2_REG_BASE;
		#endif
	}

	*hcor = (struct ehci_hcor *)((unsigned long)*hccr +
			HC_LENGTH(ehci_readl(&(*hccr)->cr_capbase)));
}

static int ehci_usb_probe(struct udevice *dev)
{
	struct uboot_ehci_nvt_priv_data *ctx = dev_get_priv(dev);
	unsigned long base_addr = dev_read_addr(dev);
	int index;

#if defined(CONFIG_TARGET_NA51055)
        #ifdef CONFIG_USB_HOST_NVTIVOT_525
        /* 51055: NT98525*/
        #else
        /* 51055: NT98528*/
        base_addr |= 0xF000000;
        #endif
#endif
	printf("0x%lx ver %s\n", base_addr, DRV_VERSION);

	if ((unsigned int)base_addr == (unsigned int)IOADDR_USB_REG_BASE)
		index = 0;
	else
		index = 1;

	nvtim_init_usbhc_dm(index, &ctx->hccr , &ctx->hcor);

	nvtim_set_gpio_power(index);

	return ehci_register(dev, ctx->hccr, ctx->hcor, &nvt_ehci_ops, 0, USB_INIT_HOST);
}

static int ehci_usb_remove(struct udevice *dev)
{
	int ret;
	unsigned long usbbase;
	unsigned long tmpval = 0;
	unsigned long base_addr = dev_read_addr(dev);

	usbbase = base_addr;

	/* Host Reset */
	union USB_MCR_REG usbmcrreg;
	usbmcrreg.reg = usb_getreg(usbbase, USB_MCR_REG_OFS);
	usbmcrreg.bit.HC_RESET = 1;
	usb_setreg(usbbase, USB_MCR_REG_OFS, usbmcrreg.reg);

	/* A_BUS_REQ[4] = 0 */
	union USB_OTGCTRLSTATUS_REG otgctrlstsreg;
	otgctrlstsreg.reg = usb_getreg(usbbase, USB_OTGCTRLSTATUS_REG_OFS);
	otgctrlstsreg.bit.A_BUS_REQ = 0;
	usb_setreg(usbbase, USB_OTGCTRLSTATUS_REG_OFS, otgctrlstsreg.reg);

	/* A_BUS_DROP[5] = 1 */
	otgctrlstsreg.reg = usb_getreg(usbbase, USB_OTGCTRLSTATUS_REG_OFS);
	otgctrlstsreg.bit.A_BUS_DROP = 1;
	usb_setreg(usbbase, USB_OTGCTRLSTATUS_REG_OFS, otgctrlstsreg.reg);

	if (usbbase == IOADDR_USB_REG_BASE) {
		printf("%s %d: controller 0\n", __FUNCTION__, __LINE__);
		/* Enable Reset*/
		tmpval = readl((volatile unsigned long *)(IOADDR_CG_REG_BASE+CG_U2_0_RST));
		tmpval &= ~(0x1<<CG_U2_0_RST_BIT);
		writel(tmpval, (volatile unsigned long *)(IOADDR_CG_REG_BASE+CG_U2_0_RST));

		udelay(10);

		/* Disable Reset*/
		tmpval = readl((volatile unsigned long *)(IOADDR_CG_REG_BASE+CG_U2_0_RST));
		tmpval |= 0x1<<CG_U2_0_RST_BIT;
		writel(tmpval, (volatile unsigned long *)(IOADDR_CG_REG_BASE+CG_U2_0_RST));
	} else {
		//two controller case
		#if defined(NVT_ECHI_2ND_CTRL)
		printf("%s %d: controller 1\n", __FUNCTION__, __LINE__);
		/* Enable Reset*/
		tmpval = readl((volatile unsigned long *)(IOADDR_CG_REG_BASE+CG_U2_1_RST));
		tmpval &= ~(0x1<<CG_U2_1_RST_BIT);
		writel(tmpval, (volatile unsigned long *)(IOADDR_CG_REG_BASE+CG_U2_1_RST));

		udelay(10);

		/* Disable Reset*/
		tmpval = readl((volatile unsigned long *)(IOADDR_CG_REG_BASE+CG_U2_1_RST));
		tmpval |= 0x1<<CG_U2_1_RST_BIT;
		writel(tmpval, (volatile unsigned long *)(IOADDR_CG_REG_BASE+CG_U2_1_RST));
		#endif
	}

#if defined(CONFIG_TARGET_NA51102_A64)
	{
		unsigned long tmpval2 = 0;

		/* clear reset flag*/
		tmpval2 = readl((volatile unsigned long *)(IOADDR_USB3PHY_REG_BASE+0x28));
		if (tmpval2 & 0x80) {
			tmpval2 &= ~(0x80);
			writel(tmpval2, (volatile unsigned long *)(IOADDR_USB3PHY_REG_BASE+0x28));
		} else {
			//do nothing.
		}
	}
#endif

	if (nvt_get_chip_id() == CHIP_NS02301) {
		nvtim_remove_power_save();
	}

	ret = ehci_deregister(dev);
	if (ret)
		return ret;

	return 0;
}

static const struct udevice_id ehci_usb_ids[] = {
	{ .compatible = "nvt,ehci-nvtivot" },
	{ }
};

U_BOOT_DRIVER(usb_ehci) = {
	.name	= "ehci_nvt",
	.id	= UCLASS_USB,
	.of_match = ehci_usb_ids,
	.of_to_plat = ehci_usb_ofdata_to_platdata,
	.probe = ehci_usb_probe,
	.remove = ehci_usb_remove,
	.ops	= &ehci_usb_ops,
	.priv_auto = sizeof(struct uboot_ehci_nvt_priv_data),
//	.platdata_auto_alloc_size = sizeof(struct exynos_ehci_platdata),
	.flags	= DM_FLAG_ALLOC_PRIV_DMA,
};
#endif
