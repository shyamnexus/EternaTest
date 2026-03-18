/**
   Nvt tee system init
   To handle basic hw init in optee os
   @file       main_a64.c
   @ingroup
   @note
   Copyright   Novatek Microelectronics Corp. 2023.  All rights reserved.

   SPDX-License-Identifier: BSD-2-Clause.
*/
#include <stdlib.h>
#include <string.h>
#include <platform_config.h>
#include <initcall.h>
#include <console.h>
#include <drivers/gic.h>
#include <drivers/serial8250_uart.h>
#include <plat/nvt_tzpc.h>
#include <plat/nvt_tzasc.h>
#include <plat/io_address.h>
#include <efuse_protected.h>
#include <kernel/cache_helpers.h>
#include <kernel/interrupt.h>
#include <kernel/panic.h>
#include <kernel/tz_ssvce_pl310.h>
#include <kernel/boot.h>
#include <mm/core_memprot.h>
#include <platform_config.h>
#include <io.h>
#include <stdint.h>
#include <tee/entry_fast.h>
#include <tee/entry_std.h>
#include <kwrap/flag.h>
#include <crypto/crypto.h>
#include "trace.h"

static struct serial8250_uart_data console_data;
static void nvt_init_early(void);
static void nvt_init_late(void);

/* To map dram0 space */
register_phys_mem_pgdir(MEM_AREA_RAM_NSEC, DRAM0_BASE, DRAM0_SIZE);
/* To map peripheral IO space */
register_phys_mem_pgdir(MEM_AREA_IO_SEC, IOADDR_BASE, IOADDR_SIZE);

#ifdef CFG_GIC
static struct gic_data gic_data;

void main_init_gic(void)
{
	gic_init(&gic_data, GIC_BASE + GICC_OFFSET,
			   GIC_BASE + GICD_OFFSET);

	itr_init(&gic_data.chip);
	rtos_flag_init(10);
}

void main_secondary_init_gic(void)
{
	gic_cpu_init(&gic_data);
}

void itr_core_handler(void)
{
	gic_it_handle(&gic_data);
}
#endif
void console_init(void)
{
	serial8250_uart_init(&console_data, CONSOLE_UART_BASE,
						 CONSOLE_UART_CLK_IN_HZ, CONSOLE_BAUDRATE);
	register_serial_console(&console_data.chip);
}

void nvt_init_early(void)
{
#ifdef CFG_ARM32_core
#if (defined(_BSP_NA51103_) || defined(_BSP_NS02301_) || defined(_BSP_NA51055_))
	vaddr_t va_cpu_reg;

	va_cpu_reg = (vaddr_t)phys_to_virt(RELEASE_ADDR, MEM_AREA_IO_SEC, 0x1000);
	io_write32(va_cpu_reg, CFG_TEE_LOAD_ADDR + 0x180);
#endif

#if (defined(_BSP_NA51089_) || defined(_BSP_NA51055_))
	vaddr_t va_scu_reg;

	va_scu_reg = (vaddr_t)phys_to_virt(SCU_BASE, MEM_AREA_IO_SEC, 0x1000);

	/* SCU config */
	io_write32(va_scu_reg + SCU_INV_SEC, SCU_INV_CTRL_INIT);
	io_write32(va_scu_reg + SCU_SAC, SCU_SAC_CTRL_INIT);
	io_write32(va_scu_reg + SCU_NSAC, SCU_NSAC_CTRL_INIT);

	/* SCU enable */
	io_setbits32(va_scu_reg + SCU_CTRL, 0x1);
#endif
#endif

	/* To switch isolate peri to 0x2f000_0000 base */
	//va_cpu_reg = (vaddr_t)phys_to_virt(NVT_CPU_REG_BASE, MEM_AREA_IO_SEC, 0x1000);
	//val = io_read32(va_cpu_reg + NVT_ISOLATE_PERI_OFFSET);
	//io_write32(va_cpu_reg + NVT_ISOLATE_PERI_OFFSET, val & ~1);

#ifdef CFG_NVT_TZPC
	nvt_tzpc_init();
#endif

#ifdef CFG_NVT_TZASC
	nvt_tzasc_init();
#endif
}

void nvt_init_late(void)
{
#ifdef CFG_NVT_OPT
	otp_init();
	otp_showinfo();
#endif
#ifdef CFG_NVT_CRYPTO
	//crypto_test();
	//nvt_aes_test();
#endif
#ifdef CFG_NVT_TZASC
	nvt_tzasc_config();
#endif
	dcache_op_all(DCACHE_OP_CLEAN_INV);

}
#ifdef CFG_ARM32_core
static void wakeup_2nd_cpu_wfi(void)
{
	const size_t IRQ_SEC_SGI = 8;
	const uint8_t CPU1_MASK = (0x1 << 1);

	itr_raise_sgi(IRQ_SEC_SGI, CPU1_MASK);
	DMSG("wakeup_2nd_cpu_wfi\n");
}
#endif
void nvt_late_init_call(void)
{
#ifdef CFG_ARM32_core
	#if (CFG_TEE_CORE_NB_CORE >= 2)
		wakeup_2nd_cpu_wfi();
	#endif

	dcache_op_all(DCACHE_OP_CLEAN_INV);
#endif
}

static TEE_Result init_nvt_drivers(void)
{
	nvt_init_early();
	nvt_init_late();

	return TEE_SUCCESS;
}

service_init_late(init_nvt_drivers);
