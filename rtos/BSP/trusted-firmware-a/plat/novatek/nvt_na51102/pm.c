/*
 * Copyright (c) 2020, NovaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>

#include <platform_def.h>

#include <arch_helpers.h>
#include <common/debug.h>
#include <drivers/arm/gicv2.h>
#include <drivers/delay_timer.h>
#include <lib/mmio.h>
#include <lib/psci/psci.h>
#include <plat/common/platform.h>

#include <novatek_def.h>
#include <novatek_private.h>

extern uint64_t nova_sec_entry_point;
extern void plat_cpu_wait_to_boot(void);
#ifdef LINUX_S3_SUPPORT
extern void plat_jump_to_sram(void);
extern void plat_s3_jump_to_sram(void);
extern void plat_mmap_s3_data_area(unsigned int addr);
extern int plat_get_power_mode(void);
#endif

#define RTC_PWBC_RESET 0  // 1: RTC-PWBC Reset 0: WDT Reset

#define mpidr_is_valid(mpidr) ( \
	MPIDR_AFFLVL3_VAL(mpidr) == 0 && \
	MPIDR_AFFLVL2_VAL(mpidr) == 0 && \
	MPIDR_AFFLVL1_VAL(mpidr) < PLATFORM_CLUSTER_COUNT && \
	MPIDR_AFFLVL0_VAL(mpidr) < PLATFORM_MAX_CPUS_PER_CLUSTER)

void nova_cpu_off(u_register_t mpidr)
{
}

static int nova_pwr_domain_on(u_register_t mpidr)
{
	unsigned int pos = plat_core_pos_by_mpidr(mpidr);
	uint64_t *hold_base = (uint64_t *)PLAT_HOLD_BASE;

	if (mpidr_is_valid(mpidr) == 0)
		return PSCI_E_INTERN_FAIL;

	assert(pos < PLATFORM_CORE_COUNT);

	hold_base[pos] = PLAT_HOLD_STATE_GO;

	/* Make sure that the write has completed */
	dsb();
	isb();

	sev();

	return PSCI_E_SUCCESS;
}

static void nova_pwr_domain_off(const psci_power_state_t *target_state)
{
	gicv2_cpuif_disable();
}

static void __dead2 nova_pwr_down_wfi(const psci_power_state_t *target_state)
{
#ifdef LINUX_S3_SUPPORT
	if (mmio_read_32(S3_MAGIC_ADDR)== S3_MAGIC_NUMBER) {
		plat_mmap_s3_data_area(mmio_read_32(S3_DATA_PTR_ADDR));
	}
#endif
	nova_cpu_off(read_mpidr());

	while (1)
		plat_cpu_wait_to_boot();
}

static void nova_pwr_domain_on_finish(const psci_power_state_t *target_state)
{

	gicv2_pcpu_distif_init();
	gicv2_cpuif_enable();
}

static void __dead2 nova_system_off(void)
{
#if (defined(_BSP_NA51102_)  || defined(_BSP_NS02201_)|| defined(_BSP_NS02302_) )
		// PWBC power off
		nvt_pwbc_power_off();
#else
		// not support rtc-pwbc reset
#endif
	wfi();
	panic();
}

static void __dead2 nova_system_reset(void)
{
	unsigned int reg_value = 0;

	/* wdt clk reset */
#if defined(_BSP_NS02302_)
	reg_value = mmio_read_64(STBC_BASE + ATF_CG_RESET_OFS);

	if (!(reg_value & (1 << ATF_WDT_RST))) {
		reg_value |= (1 << ATF_WDT_RST);
		mmio_write_64(STBC_BASE + ATF_CG_RESET_OFS, reg_value);
	}
#else
	reg_value = mmio_read_64(CG_BASE + ATF_CG_RESET_OFS);

	if (!(reg_value & (1 << ATF_WDT_RST))) {
		reg_value |= (1 << ATF_WDT_RST);
		mmio_write_64(CG_BASE + ATF_CG_RESET_OFS, reg_value);
	}
#endif

	/* wdt clk enable */
#if (defined(_BSP_NA51102_) || defined(_BSP_NS02302_))
	reg_value = mmio_read_64(STBC_BASE + ATF_CG_ENABLE_OFS);

	if (!(reg_value & (1 << ATF_WDT_POS))) {
		reg_value |= (1 << ATF_WDT_POS);
		mmio_write_64(STBC_BASE + ATF_CG_ENABLE_OFS, reg_value);
	}
#else
	reg_value = mmio_read_64(CG_BASE + ATF_CG_ENABLE_OFS);

	if (!(reg_value & (1 << ATF_WDT_POS))) {
		reg_value |= (1 << ATF_WDT_POS);
		mmio_write_64(CG_BASE + ATF_CG_ENABLE_OFS, reg_value);
	}
#endif

	if( RTC_PWBC_RESET == 1 ) {
#if (defined(_BSP_NA51102_)  || defined(_BSP_NS02201_)|| defined(_BSP_NS02302_) )
		// rtc pwbc reset 
		nvt_pwbc_power_control(3);
#else
		// not support rtc-pwbc reset
#endif

	} else {
		/* wdt manual reset */
		mmio_write_64(WDT_BASE, 0x5a960112);
		mmio_write_64(WDT_BASE, 0x5a960113);
		mmio_write_64(WDT_BASE + MAN_RST_OFS, 1);
	}

	wfi();
	panic();
	
}

static int nova_validate_ns_entrypoint(uintptr_t ns_entrypoint)
{
	return PSCI_E_SUCCESS;
}

static void nova_pwr_domain_suspend(const psci_power_state_t *target_state)
{
	(void)console_flush();
	console_switch_state(0);

	//gicv2_cpuif_disable();
}

static void nova_pwr_domain_suspend_finish(const psci_power_state_t *target_state)
{
#ifdef LINUX_S3_SUPPORT
	plat_mmap_s3_data_area(mmio_read_32(S3_DATA_PTR_ADDR));
	if (plat_get_power_mode() == POWER_KEEP_MODE)
		plat_jump_to_sram();
	else
		plat_s3_jump_to_sram();
#endif
	gicv2_distif_init();
	gicv2_pcpu_distif_init();
	gicv2_cpuif_enable();
#if 0//def _BSP_NA51102_
	/* enable ICE */
	*(volatile unsigned int *)0x2F0010004 = 0x10011;
	*(volatile unsigned int *)0x2F00100b4 = 0x1ff;
#endif
}

static int nova_validate_power_state(unsigned int power_state, psci_power_state_t *req_state)
{
	req_state->pwr_domain_state[0] = PLAT_MAX_OFF_STATE;
	req_state->pwr_domain_state[1] = PLAT_MAX_RET_STATE;

	return PSCI_E_SUCCESS;
}

static void nova_get_sys_suspend_power_state(psci_power_state_t *req_state)
{
}

static const plat_psci_ops_t nova_psci_ops = {
	.pwr_domain_on			= nova_pwr_domain_on,
	.pwr_domain_off			= nova_pwr_domain_off,
	.pwr_domain_pwr_down_wfi	= nova_pwr_down_wfi,
	.pwr_domain_on_finish		= nova_pwr_domain_on_finish,
	.system_reset			= nova_system_reset,
	.system_off			= nova_system_off,
	.validate_ns_entrypoint		= nova_validate_ns_entrypoint,
	.pwr_domain_suspend         = nova_pwr_domain_suspend,
	.pwr_domain_suspend_finish  = nova_pwr_domain_suspend_finish,
	.validate_power_state       = nova_validate_power_state,
	.get_sys_suspend_power_state    = nova_get_sys_suspend_power_state,
};

int plat_setup_psci_ops(uintptr_t sec_entrypoint,
			const plat_psci_ops_t **psci_ops)
{

	nova_sec_entry_point = sec_entrypoint;
	flush_dcache_range((uint64_t)&nova_sec_entry_point, sizeof(uint64_t));

	/*
	* Initialize PSCI ops struct
	*/
	*psci_ops = &nova_psci_ops;
	return 0;
}
