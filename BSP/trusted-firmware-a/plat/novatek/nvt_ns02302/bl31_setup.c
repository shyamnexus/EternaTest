/*
 * Copyright (c) 2020, NovaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>

#include <libfdt.h>

#include <platform_def.h>

#include <arch.h>
#include <arch_helpers.h>
#include <common/debug.h>
#include <drivers/arm/gicv2.h>
#include <drivers/console.h>
#include <drivers/generic_delay_timer.h>
#include <drivers/ti/uart/uart_16550.h>
#include <lib/mmio.h>
#include <plat/common/platform.h>
#include <nvt_bin_info.h>
#include <novatek_def.h>
#include <novatek_private.h>

static entry_point_info_t bl32_image_ep_info;
static entry_point_info_t bl33_image_ep_info;
extern HEADINFO _nvt_headinfo;
static console_16550_t console;

static const gicv2_driver_data_t gic_data = {
	.gicd_base = GICD_BASE,
	.gicc_base = GICC_BASE,
};

/*
 * Try to find a DTB loaded in memory by previous stages.
 *
 * At the moment we implement a heuristic to find the DTB attached to U-Boot:
 * U-Boot appends its DTB to the end of the image. Assuming that BL33 is
 * U-Boot, try to find the size of the U-Boot image to learn the DTB address.
 * The generic ARMv8 U-Boot image contains the load address and its size
 * as u64 variables at the beginning of the image. There might be padding
 * or other headers before that data, so scan the first 2KB after the BL33
 * entry point to find the load address, which should be followed by the
 * size. Adding those together gives us the address of the DTB.
 */
static void *find_dtb(void)
{
	uint32_t *dtb_base;

	dtb_base = (void *)FDT_BASE;

	if (fdt_check_header(dtb_base) != 0) {
		NOTICE("FDT check fail.\n");
		return NULL;
	}
	return dtb_base;
}

void bl31_early_platform_setup2(u_register_t arg0, u_register_t arg1,
								u_register_t arg2, u_register_t arg3)
{
	/* Initialize the debug console as soon as possible */
	console_16550_register(UART0_BASE, UART0_CLK_IN_HZ,
						   UART0_BAUDRATE, &console);

#if USE_OPTEE >= 1
	/* Populate entry point information for BL32 */
	SET_PARAM_HEAD(&bl32_image_ep_info, PARAM_EP, VERSION_1, 0);
	SET_SECURITY_STATE(bl32_image_ep_info.h.attr, SECURE);
	bl32_image_ep_info.pc = mmio_read_64(BL32_BASE + 0x100);
	INFO("BL31: Have optee, jump to 0x%lx\n", bl32_image_ep_info.pc);
	bl32_image_ep_info.spsr = 0;
#endif

	/* Populate entry point information for BL33 */
	SET_PARAM_HEAD(&bl33_image_ep_info, PARAM_EP, VERSION_1, 0);
	/*
	 * Tell BL31 where the non-trusted software image
	 * is located and the entry state information
	 */
	bl33_image_ep_info.pc = plat_get_ns_image_entrypoint();
	bl33_image_ep_info.spsr = SPSR_64(MODE_EL2, MODE_SP_ELX,
									  DISABLE_ALL_EXCEPTIONS);
	SET_SECURITY_STATE(bl33_image_ep_info.h.attr, NON_SECURE);

}

void bl31_plat_arch_setup(void)
{
	plat_tmr_init();

	mmio_write_32(REG_RELEASE_ADDR, BL31_BASE);
	/* set TrustZone */
	mmio_write_64(TZPC_BASE, 0);
	mmio_write_64(TZPC_BASE + 0x8, 0);
	mmio_write_64(TZPC_BASE + 0x10, 0);
	mmio_write_64(TZPC_BASE + 0x18, 0);


	nova_setup_page_tables(BL31_BASE, (PLAT_HOLD_BASE - BL31_BASE),
						   BL_CODE_BASE, BL_CODE_END,
						   BL_RO_DATA_BASE, BL_RO_DATA_END
						  );
	enable_mmu_el3(0);
}

void bl31_platform_setup(void)
{
	void *fdt;
	flush_dcache_range(FDT_BASE, FDT_SIZE);

	fdt = find_dtb();
	if (fdt) {
		NOTICE("BL31: Found DTB\n");
	} else {
		NOTICE("BL31: No DTB found.\n");
	}

	generic_delay_timer_init();
#ifndef _NVT_FPGA_EMULATION_ON_
	nvt_tzasc_config();
#endif

	/* Configure the interrupt controller */
	gicv2_driver_init(&gic_data);
	gicv2_distif_init();
	gicv2_pcpu_distif_init();
	gicv2_cpuif_enable();

	INFO("BL31: Platform setup done\n");
}

static int bl31_get_core_count_from_dt(void *fdt)
{
	const char *device_type;
	int len, node, cores = 0;

	const char *cpu_path = "/cpus";

	node = fdt_path_offset(fdt, cpu_path);
	if (node < 0) {
		ERROR("Node %s not found in device tree\n", cpu_path);
		return 0;
	}

	fdt_for_each_subnode(node, fdt, node) {
		device_type = fdt_getprop(fdt, node, "device_type", &len);
		if (!device_type || len != sizeof("cpu") || strcmp(device_type, "cpu") != 0) {
			continue;
		}
		cores++;
	}

	return cores;
}

entry_point_info_t *bl31_plat_get_next_image_ep_info(uint32_t type)
{
	assert(sec_state_is_valid(type) != 0);

	if (_nvt_headinfo.BinCtrl) {
		int i;
		int num_core = bl31_get_core_count_from_dt(find_dtb());

		write_scr_el3(SCR_NS_BIT | SCR_RES1_BITS | SCR_HCE_BIT | SCR_SIF_BIT | SCR_RW_BIT);
		// retrieve such cpu[3] -> cpu[2] -> cpu[1] other than to cpu[0] for AMP
		INFO("fdt core num: %d\n", num_core);
		for (i = num_core-1; i >= 1; i--) {
			// PLAT_HOLD_BASE + i*sizeof(uint64_t) are filled from loader
			uint64_t addr = mmio_read_64(PLAT_HOLD_BASE + i*sizeof(uint64_t));
			// if address has prepared, let it go.
			if (addr) {
				INFO("psci_cpu_on core[%d] to 0x%lx\n", i, (uintptr_t)addr);
				psci_cpu_on(i, addr, 0);
			}
		}
		// after psci_cpu_on(), do cache flush for other cpu getting correct data
		flush_dcache_range(BL31_BASE, BL31_SIZE);
		// notify cpu[1] that cpu[0] is ready
		mmio_write_64(PLAT_CORE0_READY_BASE, 1);
		bl33_image_ep_info.pc = mmio_read_64(PLAT_HOLD_BASE);
		while (!bl33_image_ep_info.pc) {
			bl33_image_ep_info.pc = mmio_read_64(PLAT_HOLD_BASE);
		}
		bl33_image_ep_info.args.arg0 = mmio_read_64(PLAT_LINUX_FDT_BASE);
		NOTICE("Linux addr 0x%lx, FDT 0x%lx\n", bl33_image_ep_info.pc, (unsigned long)bl33_image_ep_info.args.arg0);
	}

	if (type == NON_SECURE) {
		return &bl33_image_ep_info;
	}

	if ((type == SECURE) && bl32_image_ep_info.pc) {
		return &bl32_image_ep_info;
	} else {
		NOTICE("not find OPTEE, if system not have OPTEEe, don't care OPTEE error or warning message\n");
	}
	return NULL;
}

int psci_hook_cpu_on(u_register_t target_cpu, uintptr_t entrypoint, u_register_t context_id, int ret)
{
	if (_nvt_headinfo.BinCtrl == 0) {
		// if not AMP, return directly
		return ret;
	}
	if (ret == PSCI_E_ALREADY_ON) {
		// write back to (PLAT_HOLD_BASE + target_cpu*sizeof(uint64_t))
		// for rtos to get linux secondary entry pointer
		uint64_t addr = PLAT_HOLD_BASE + target_cpu*sizeof(uint64_t);
		mmio_write_64(addr, entrypoint);
		flush_dcache_range(addr, sizeof(uint64_t));
		ret = 0;
	}
	return ret;
}