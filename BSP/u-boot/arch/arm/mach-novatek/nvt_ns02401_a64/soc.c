/**
	Provide basic CPU info

    @file       soc.c
    @ingroup
    @note
    Copyright   Novatek Microelectronics Corp. 2019.  All rights reserved.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License version 2 as
    published by the Free Software Foundation.
*/

#include <common.h>
#include <asm/io.h>
//#include <asm/arch/soc.h>
#include <asm/arch/clock.h>
#include <asm/nvt-common/nvt_ivot_optee.h>
//#include <asm/arch/gpio.h>
#include <asm/arch/ns02401_regs.h>
#include <asm/arch/IOAddress.h>
#include <asm/arch/hardware.h>
#include <linux/sizes.h>
#include <linux/compiler.h>
#include <ns16550.h>
#include <dm.h>
#include <asm/nvt-common/nvt_common.h>
#include <command.h>

#define RAM_SRC_SELECT_OFFSET 0x20

static char nvt_ddr_remap_help_text[] =
"\n"
"nvt_ddr_remap 2 - DDR1 base address on 4G bits\n"
"nvt_ddr_remap 3 - DDR1 base address on 8G bits\n"
"nvt_ddr_remap 4 - DDR1 base address on 16G bits\n"
"";

static int do_nvt_ddr_remap(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
	unsigned int reg;
	unsigned int val;

	if (argc < 2) {
		return -1;
	}
	val = (unsigned int)simple_strtoul(argv[1], NULL, 16);

	reg = readl(IOADDR_TOP_REG_BASE + RAM_SRC_SELECT_OFFSET);
	reg |= 1 << (val + 3);
	writel(reg, IOADDR_TOP_REG_BASE + RAM_SRC_SELECT_OFFSET);
	return 0;
}

U_BOOT_CMD(
	nvt_ddr_remap, 2, 1, do_nvt_ddr_remap,
	"Represent DRAMx select",
	nvt_ddr_remap_help_text
);

#ifdef CONFIG_NVT_IVOT_DDR_SLOW_DOWN_SUPPORT
extern int nvt_ddr_slow_down(void);
#endif

DECLARE_GLOBAL_DATA_PTR;

#if !CONFIG_IS_ENABLED(OF_CONTROL)
static struct ns16550_platdata ns16550_com1_pdata = {
	.base = CONFIG_SYS_NS16550_COM1,
	.reg_shift = 2,
	.clock = 48000000,
};

static struct ns16550_platdata ns16550_com2_pdata = {
	.base = CONFIG_SYS_NS16550_COM2,
	.reg_shift = 2,
	.clock = 48000000,
};

static struct ns16550_platdata ns16550_com3_pdata = {
	.base = CONFIG_SYS_NS16550_COM3,
	.reg_shift = 2,
	.clock = 48000000,
};

static struct ns16550_platdata ns16550_com4_pdata = {
	.base = CONFIG_SYS_NS16550_COM4,
	.reg_shift = 2,
	.clock = 48000000,
};

static struct ns16550_platdata ns16550_com5_pdata = {
	.base = CONFIG_SYS_NS16550_COM5,
	.reg_shift = 2,
	.clock = 48000000,
};

static struct ns16550_platdata ns16550_com6_pdata = {
	.base = CONFIG_SYS_NS16550_COM6,
	.reg_shift = 2,
	.clock = 48000000,
};

U_BOOT_DEVICE(ns16550_com1) = {
	"ns16550_serial", &ns16550_com1_pdata
#ifdef CONFIG_SYS_NS16550_COM2
	"ns16550_serial", &ns16550_com2_pdata
#endif
#ifdef CONFIG_SYS_NS16550_COM3
	"ns16550_serial", &ns16550_com3_pdata
#endif
#ifdef CONFIG_SYS_NS16550_COM4
	"ns16550_serial", &ns16550_com4_pdata
#endif
#ifdef CONFIG_SYS_NS16550_COM5
	"ns16550_serial", &ns16550_com5_pdata
#endif
#ifdef CONFIG_SYS_NS16550_COM6
	"ns16550_serial", &ns16550_com6_pdata
#endif
};
#endif


int serial_preinit(void)
{
#if ( CONFIG_UART2_PINMUX_CHANNEL_1 || \
	CONFIG_UART3_PINMUX_CHANNEL_1 || \
	CONFIG_UART4_PINMUX_CHANNEL_1 || CONFIG_UART4_PINMUX_CHANNEL_2 || \
	CONFIG_UART5_PINMUX_CHANNEL_1 || CONFIG_UART5_PINMUX_CHANNEL_2 || \
	CONFIG_UART6_PINMUX_CHANNEL_1 || CONFIG_UART6_PINMUX_CHANNEL_2 || \
	CONFIG_UART7_PINMUX_CHANNEL_1 || CONFIG_UART7_PINMUX_CHANNEL_2 || \
	CONFIG_UART8_PINMUX_CHANNEL_1 || CONFIG_UART8_PINMUX_CHANNEL_2 || \
	CONFIG_UART9_PINMUX_CHANNEL_1 || CONFIG_UART9_PINMUX_CHANNEL_2)
	u32 value;
#endif

#ifdef CONFIG_SYS_NS16550_COM2
	#ifdef CONFIG_UART2_PINMUX_CHANNEL_1
		value = readl(IOADDR_TOP_REG_BASE + 0xA8);
		writel(value & ~0xC0000000, IOADDR_TOP_REG_BASE + 0xA8);

		value = readl(IOADDR_TOP_REG_BASE + 0x34);
		value &= ~0xF0;
		writel(value | 0x10, IOADDR_TOP_REG_BASE + 0x34);

		value = readl(IOADDR_CG_REG_BASE + 0x78);
		writel(value | 0x200000, IOADDR_CG_REG_BASE + 0x78);
	#endif
#endif

#ifdef CONFIG_SYS_NS16550_COM3
	#ifdef CONFIG_UART3_PINMUX_CHANNEL_1
		value = readl(IOADDR_TOP_REG_BASE + 0xA8);
		writel(value & ~0x3000000, IOADDR_TOP_REG_BASE + 0xA8);

		value = readl(IOADDR_TOP_REG_BASE + 0x34);
		value &= ~0xF00;
		writel(value | 0x10, IOADDR_TOP_REG_BASE + 0x34);

		value = readl(IOADDR_CG_REG_BASE + 0x78);
		writel(value | 0x400000, IOADDR_CG_REG_BASE + 0x78);
	#endif
#endif

#ifdef CONFIG_SYS_NS16550_COM4
	#ifdef CONFIG_UART4_PINMUX_CHANNEL_1
		value = readl(IOADDR_TOP_REG_BASE + 0xA8);
		writel(value & ~0x300000, IOADDR_TOP_REG_BASE + 0xA8);

		value = readl(IOADDR_TOP_REG_BASE + 0x34);
		value &= ~0xF000;
		writel(value | 0x1000, IOADDR_TOP_REG_BASE + 0x34);

		value = readl(IOADDR_CG_REG_BASE + 0x78);
		writel(value | 0x800000, IOADDR_CG_REG_BASE + 0x78);
	#elif defined(CONFIG_UART4_PINMUX_CHANNEL_2)
		value = readl(IOADDR_TOP_REG_BASE + 0xE8);
		writel(value & ~0x03, IOADDR_TOP_REG_BASE + 0xE8);

		value = readl(IOADDR_TOP_REG_BASE + 0x34);
		value &= ~0xF000;
		writel(value | 0x2000, IOADDR_TOP_REG_BASE + 0x34);

		value = readl(IOADDR_CG_REG_BASE + 0x78);
		writel(value | 0x800000, IOADDR_CG_REG_BASE + 0x78);
	#endif
#endif

#ifdef CONFIG_SYS_NS16550_COM5
	#ifdef CONFIG_UART5_PINMUX_CHANNEL_1
		value = readl(IOADDR_TOP_REG_BASE + 0xA8);
		writel(value & ~0x30000, IOADDR_TOP_REG_BASE + 0xA8);

		value = readl(IOADDR_TOP_REG_BASE + 0x34);
		value &= ~0xF0000;
		writel(value | 0x1000, IOADDR_TOP_REG_BASE + 0x34);

		value = readl(IOADDR_CG_REG_BASE + 0x78);
		writel(value | 0x1000000, IOADDR_CG_REG_BASE + 0x78);
	#elif defined(CONFIG_UART5_PINMUX_CHANNEL_2)
		value = readl(IOADDR_TOP_REG_BASE + 0xD8);
		writel(value & ~0x03, IOADDR_TOP_REG_BASE + 0xD8);

		value = readl(IOADDR_TOP_REG_BASE + 0x34);
		value &= ~0xF0000;
		writel(value | 0x20000, IOADDR_TOP_REG_BASE + 0x34);

		value = readl(IOADDR_CG_REG_BASE + 0x78);
		writel(value | 0x1000000, IOADDR_CG_REG_BASE + 0x78);
	#endif
#endif

#ifdef CONFIG_SYS_NS16550_COM6
	#ifdef CONFIG_UART6_PINMUX_CHANNEL_1
		value = readl(IOADDR_TOP_REG_BASE + 0xA8);
		writel(value & ~0x3000, IOADDR_TOP_REG_BASE + 0xA8);

		value = readl(IOADDR_TOP_REG_BASE + 0x34);
		value &= ~0xF00000;
		writel(value | 0x100000, IOADDR_TOP_REG_BASE + 0x34);

		value = readl(IOADDR_CG_REG_BASE + 0x7C);
		writel(value | 0x200000, IOADDR_CG_REG_BASE + 0x7C);
	#elif defined(CONFIG_UART6_PINMUX_CHANNEL_2)
		value = readl(IOADDR_TOP_REG_BASE + 0xD8);
		writel(value & ~0x30, IOADDR_TOP_REG_BASE + 0xD8);

		value = readl(IOADDR_TOP_REG_BASE + 0x34);
		value &= ~0xF00000;
		writel(value | 0x200000, IOADDR_TOP_REG_BASE + 0x34);

		value = readl(IOADDR_CG_REG_BASE + 0x7C);
		writel(value | 0x200000, IOADDR_CG_REG_BASE + 0x7C);
	#endif
#endif

#ifdef CONFIG_SYS_NS16550_COM7
	#ifdef CONFIG_UART7_PINMUX_CHANNEL_1
		value = readl(IOADDR_TOP_REG_BASE + 0xA8);
		writel(value & ~0x300, IOADDR_TOP_REG_BASE + 0xA8);

		value = readl(IOADDR_TOP_REG_BASE + 0x34);
		value &= ~0xF000000;
		writel(value | 0x1000000, IOADDR_TOP_REG_BASE + 0x34);

		value = readl(IOADDR_CG_REG_BASE + 0x7C);
		writel(value | 0x400000, IOADDR_CG_REG_BASE + 0x7C);
	#elif defined(CONFIG_UART7_PINMUX_CHANNEL_2)
		value = readl(IOADDR_TOP_REG_BASE + 0xA0);
		writel(value & ~0x30, IOADDR_TOP_REG_BASE + 0xA0);

		value = readl(IOADDR_TOP_REG_BASE + 0x34);
		value &= ~0xF000000;
		writel(value | 0x100000, IOADDR_TOP_REG_BASE + 0x34);

		value = readl(IOADDR_CG_REG_BASE + 0x7C);
		writel(value | 0x400000, IOADDR_CG_REG_BASE + 0x7C);
	#endif
#endif

#ifdef CONFIG_SYS_NS16550_COM8
	#ifdef CONFIG_UART8_PINMUX_CHANNEL_1
		value = readl(IOADDR_TOP_REG_BASE + 0xA8);
		writel(value & ~0x30, IOADDR_TOP_REG_BASE + 0xA8);

		value = readl(IOADDR_TOP_REG_BASE + 0x34);
		value &= ~0xF0000000;
		writel(value | 0x10000000, IOADDR_TOP_REG_BASE + 0x34);

		value = readl(IOADDR_CG_REG_BASE + 0x7C);
		writel(value | 0x800000, IOADDR_CG_REG_BASE + 0x7C);
	#elif defined(CONFIG_UART8_PINMUX_CHANNEL_2)
		value = readl(IOADDR_TOP_REG_BASE + 0xA0);
		writel(value & ~0x60000, IOADDR_TOP_REG_BASE + 0xA0);

		value = readl(IOADDR_TOP_REG_BASE + 0x34);
		value &= ~0xF0000000;
		writel(value | 0x20000000, IOADDR_TOP_REG_BASE + 0x34);

		value = readl(IOADDR_CG_REG_BASE + 0x7C);
		writel(value | 0x800000, IOADDR_CG_REG_BASE + 0x7C);
	#endif
#endif

#ifdef CONFIG_SYS_NS16550_COM9
	#ifdef CONFIG_UART9_PINMUX_CHANNEL_1
		value = readl(IOADDR_TOP_REG_BASE + 0xA8);
		writel(value & ~0x03, IOADDR_TOP_REG_BASE + 0xA8);

		value = readl(IOADDR_TOP_REG_BASE + 0x38);
		value &= ~0x0F;
		writel(value | 0x01, IOADDR_TOP_REG_BASE + 0x38);

		value = readl(IOADDR_CG_REG_BASE + 0x7C);
		writel(value | 0x1000000, IOADDR_CG_REG_BASE + 0x7C);
	#elif defined(CONFIG_UART9_PINMUX_CHANNEL_2)
		value = readl(IOADDR_TOP_REG_BASE + 0xB4);
		writel(value & ~0x60, IOADDR_TOP_REG_BASE + 0xB4);

		value = readl(IOADDR_TOP_REG_BASE + 0x38);
		value &= ~0x0F;
		writel(value | 0x02, IOADDR_TOP_REG_BASE + 0x38);

		value = readl(IOADDR_CG_REG_BASE + 0x7C);
		writel(value | 0x1000000, IOADDR_CG_REG_BASE + 0x7C);
	#endif
#endif

	return 0;
}

void enable_caches(void)
{
	dcache_enable();
	icache_enable();
}


#ifdef CONFIG_ARCH_MISC_INIT
#ifdef CONFIG_NOVATEK_PAD
extern void nvt_pad_init(void);
#endif
int arch_misc_init(void)
{
#ifdef CONFIG_NOVATEK_PAD
	nvt_pad_init();
#endif
	return 0;
}
#endif

/*
 * Print board information
 */
int checkboard(void)
{
	return 0;
}

/*
 * Print CPU information
 */
#ifdef CONFIG_DISPLAY_CPUINFO
int print_cpuinfo(void)
{
	printf("CPU:   Novatek %s @ %lu MHz\n", _CHIP_NAME_, get_cpu_clk()/1000);

	return 0;
}
#endif

void s_init(void)
{
	//u32 val;

	/* Do we want to combine ARM_UART_B and STBC_UART_1 on EVB board?
	 * NO! So we need set fc040250[bit0] to "0"
	 */
	// val = *(volatile unsigned int *)0xfc040250;
	// if ((val & 0x01) == 0x01) {
		// val &= ~0x01;
		// writel(0x0, 0xfc040250);
	// }

	gd->cpu_clk = get_cpu_clk() * 1000;
}

void cpu_cache_initialization(void)
{
	return;
}

u32 nvt_get_chip_id(void)
{
	unsigned int chip_id;

	chip_id = (readl(IOADDR_TOP_REG_BASE + TOP_VERSION_REG_OFS) >> 16) & 0xFFFF;

	return chip_id;
}

//
// ROM code version number
//
#define TOP_ROMVER_A            0x00000102
#define TOP_ROMVER_B            0x00000103
#define TOP_GETROMVER()         readl(0x2F0007FFC)
u32 nvt_get_chip_ver(void)
{
	unsigned int chip_ver;

	chip_ver = TOP_GETROMVER();
    if(chip_ver == TOP_ROMVER_A) {
        return CHIPVER_A;
    } else if(chip_ver == TOP_ROMVER_B) {
        return CHIPVER_B;
    } else {
		printf("force to Ver B\n");
        return CHIPVER_B;
    }

	return chip_ver;
}

