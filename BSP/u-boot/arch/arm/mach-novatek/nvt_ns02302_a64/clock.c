/**
    Clock info

    @file       clock.c
    @ingroup
    @note
    Copyright   Novatek Microelectronics Corp. 2019.  All rights reserved.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License version 2 as
    published by the Free Software Foundation.
*/

#include <common.h>
#include <command.h>
#include <asm/io.h>
#include <asm/arch/clock.h>
#include <asm/arch/IOAddress.h>
#include <asm/arch/hardware.h>
#include <asm/nvt-common/nvt_types.h>
#include <asm/nvt-common/nvt_common.h>
#include <asm/nvt-common/rcw_macro.h>
#include <asm/arch/efuse_protected.h>

#define SYSTEM_CLOCK_RATE_OFS           0x10
#define CPU_CLOCK_RATE_RATIO0_OFS       0x47e0
#define CPU_CLOCK_RATE_RATIO1_OFS       0x47e4
#define CPU_CLOCK_RATE_RATIO2_OFS       0x47e8

#define CPU_CLOCK_RATE_539A_RATIO0_OFS  0x46A0
#define CPU_CLOCK_RATE_539A_RATIO1_OFS  0x46A4
#define CPU_CLOCK_RATE_539A_RATIO2_OFS  0x46A8


#define DMA_CLOCK_RATE_RATIO0_OFS       (0x4000 + 0x400 + 0x20)
#define DMA_CLOCK_RATE_RATIO1_OFS       (0x4000 + 0x400 + 0x24)
#define DMA_CLOCK_RATE_RATIO2_OFS       (0x4000 + 0x400 + 0x28)



#define PLL_CPU_NO                      8                              //No. of CPU's PLL = 8
#define PLL_CLKSEL_CPU                  (0)
#define PLL_CLKSEL_CPU_80               (0x00 << PLL_CLKSEL_CPU)        //< Select CPU clock 80MHz
#define PLL_CLKSEL_CPU_ARMPLL           (0x01 << PLL_CLKSEL_CPU)        //< Select CPU clock ARMPLL (for CPU)
#define PLL_CLKSEL_CPU_480              (0x02 << PLL_CLKSEL_CPU)        //< Select CPU clock 480MHz
#define PLL_CLKSEL_CPUMASK              (0x3)

#define PLL_CLKSEL_APBMASK              (0x3<<8)
#define PLL_CLKSEL_APB48               (0x0<<8)
#define PLL_CLKSEL_APB60               (0x1<<8)
#define PLL_CLKSEL_APB80               (0x2<<8)
#define PLL_CLKSEL_APB120               (0x3<<8)

#ifdef CONFIG_NVT_FPGA_EMULATION
#define CALCULATE_CPU_FREQ_UNIT_US      100000
#define      timer_gettick()            readl(IOADDR_TIMER_REG_BASE + 0x108) * (10)
#define core_timer_gettick()            readl(IOADDR_SYSCNT_READ_BASE +0x00)
#else
#define CALCULATE_CPU_FREQ_UNIT_US      100000
#define      timer_gettick()            readl(IOADDR_TIMER_REG_BASE + 0x108)
#define core_timer_gettick()            readl(IOADDR_SYSCNT_READ_BASE +0x00)
#endif

#define INREG32(x)                      readl(x)
#define OUTREG32(x, y)                  writel(y, x)
#define SETREG32(x, y)                  OUTREG32((x), INREG32(x) | (y))
#define CLRREG32(x, y)                  OUTREG32((x), INREG32(x) & ~(y))

#define CPU_NORMAL_FREQ_MAX             (1100)        //1.1GHz
#define CPU_OVER_CLKING_FREQ_MAX        (1300)        //1.3GHz



void core_timer_delay(u32 us);
u64 ca53_get_cycle_count(void);
void ca53_cycle_count_stop(void);
void ca53_cycle_count_start(BOOL do_reset, BOOL enable_divider);

#ifdef CONFIG_NVT_FPGA_EMULATION
static int nvt_cpu_get_freq_by_core_timer(void)
{
	u32 freq;
	u32 time_1, time_2, temp, time_interval;

	ca53_cycle_count_start(TRUE, FALSE);
	time_1 = ca53_get_cycle_count();
	core_timer_delay(CALCULATE_CPU_FREQ_UNIT_US);
	time_2 = ca53_get_cycle_count();
	if (time_2 > time_1) {
		time_interval = (time_2 - time_1);
	} else {
		temp = 0xFFFFFFFF - time_1;
		time_interval = temp + time_2;
	}
	freq = (time_interval) / (CALCULATE_CPU_FREQ_UNIT_US);
	ca53_cycle_count_stop();
	return (freq * 1000);
}
#endif

inline void mpll_set_data(u8 offset, u8 value)
{

}

inline u32 mpll_get_data(u8 offset)
{
	return 0;
}

void set_cpu_clk(unsigned long freq)
{
	/* no implement */
}

unsigned long get_cpu_clk(void)
{
	unsigned int cpu_clk_sel = 0;
	static u32 chip_id;
#ifndef CONFIG_NVT_FPGA_EMULATION
	unsigned int cpu_freq_ratio;
	unsigned int cpu_freq_ratio0, cpu_freq_ratio1, cpu_freq_ratio2;
#else
	return  nvt_cpu_get_freq_by_core_timer();
#endif
	cpu_clk_sel = readl(IOADDR_CG_REG_BASE + SYSTEM_CLOCK_RATE_OFS) & 0x3;

	chip_id = nvt_get_chip_id();
	if (cpu_clk_sel == 1) {
#ifdef CONFIG_NVT_FPGA_EMULATION
		return 24000;
#else
		if(chip_id == CHIP_NS02402) {
			cpu_freq_ratio0 = readl(IOADDR_STBC_CG_REG_BASE + \
									CPU_CLOCK_RATE_539A_RATIO0_OFS);
			cpu_freq_ratio1 = readl(IOADDR_STBC_CG_REG_BASE + \
									CPU_CLOCK_RATE_539A_RATIO1_OFS);
			cpu_freq_ratio2 = readl(IOADDR_STBC_CG_REG_BASE + \
									CPU_CLOCK_RATE_539A_RATIO2_OFS);
		} else {
			cpu_freq_ratio0 = readl(IOADDR_STBC_CG_REG_BASE + \
									CPU_CLOCK_RATE_RATIO0_OFS);
			cpu_freq_ratio1 = readl(IOADDR_STBC_CG_REG_BASE + \
									CPU_CLOCK_RATE_RATIO1_OFS);
			cpu_freq_ratio2 = readl(IOADDR_STBC_CG_REG_BASE + \
									CPU_CLOCK_RATE_RATIO2_OFS);
		}
		cpu_freq_ratio = cpu_freq_ratio0 | (cpu_freq_ratio1 << 8) | \
						 (cpu_freq_ratio2 << 16);

		cpu_freq_ratio = (cpu_freq_ratio << 3);

		return (12 * cpu_freq_ratio / 131072) * 1000;
#endif
	} else if (cpu_clk_sel == 2) {
		return 480000;
	} else {
		return 80000;
	}
}


#define PERF_DEF_OPTS           (1 | 16)
#define PERF_OPT_RESET_CYCLES   (2 | 4)
#define PERF_OPT_DIV64          (8)
#define ARMV8_PMCR_MASK         0x3f
#define ARMV8_PMCR_E            (1 << 0) /* Enable all counters */
#define ARMV8_PMCR_P            (1 << 1) /* Reset all counters */
#define ARMV8_PMCR_C            (1 << 2) /* Cycle counter reset */
#define ARMV8_PMCR_D            (1 << 3) /* CCNT counts every 64th cpu cycle */
#define ARMV8_PMCR_X            (1 << 4) /* Export to ETM */
#define ARMV8_PMCR_DP           (1 << 5) /* Disable CCNT if non-invasive debug*/
#define ARMV8_PMCR_LC           (1 << 6) /* Cycle Counter 64bit overflow*/
#define ARMV8_PMCR_N_SHIFT      11       /* Number of counters supported */
#define ARMV8_PMCR_N_MASK       0x1f

#define ARMV8_PMUSERENR_EN_EL0  (1 << 0) /* EL0 access enable */
#define ARMV8_PMUSERENR_CR      (1 << 2) /* Cycle counter read enable */
#define ARMV8_PMUSERENR_ER      (1 << 3) /* Event counter read enable */

static inline u32 armv8pmu_pmcr_read(void)
{
	u64 val = 0;
	asm volatile("mrs %0, pmcr_el0" : "=r"(val));
	return (u32)val;
}
static inline void armv8pmu_pmcr_write(u32 val)
{
	val &= ARMV8_PMCR_MASK;
	isb();
	asm volatile("msr pmcr_el0, %0" : : "r"((u64)val));
}
static inline  long long armv8_read_CNTPCT_EL0(void)
{
	long long val;
	asm volatile("mrs %0, CNTVCT_EL0" : "=r"(val));
	return val;
}

static void enable_cpu_counters(void)
{
	u64 val;
	/* Disable cycle counter overflow interrupt */
	asm volatile("msr pmintenset_el1, %0" : : "r"((u64)(0 << 31)));
	/* Enable cycle counter */
	asm volatile("msr pmcntenset_el0, %0" :: "r" BIT(31));
	/* Enable user-mode access to cycle counters. */
	asm volatile("msr pmuserenr_el0, %0" : : "r"(BIT(0) | BIT(2)));
	/* Clear cycle counter and start */
	asm volatile("mrs %0, pmcr_el0" : "=r"(val));
	val |= (BIT(0) | BIT(2));
	isb();
	asm volatile("msr pmcr_el0, %0" : : "r"(val));
	val = BIT(27);
	asm volatile("msr pmccfiltr_el0, %0" : : "r"(val));
}

static void disable_cpu_counters(void)
{
	/* Disable cycle counter */
	asm volatile("msr pmcntenset_el0, %0" :: "r"(0 << 31));
	/* Disable user-mode access to counters. */
	asm volatile("msr pmuserenr_el0, %0" : : "r"((u64)0));
}

static inline u64 arch_counter_get_cntpct(void)
{
	long long val;
	asm volatile("mrs %0, CNTVCT_EL0" : "=r"(val));
	return val;
}
//static BOOL cpu_count_open = FALSE;
void timer2_delay(u32 us)
{
	u32 start, end;
	start = timer_gettick();
	/*check timer count to target level*/
	while (1) {
		end = timer_gettick();
		if ((end - start) > us) {
			break;
		}
	}
}

void core_timer_delay(u32 us)
{
	u32     start, end;
	u32     tick;
	u32     total_tick;

	tick = (1000 / (CONFIG_SYS_HZ_CLOCK / 1000000));
	total_tick = us * 1000 / (u32)tick;
	start = core_timer_gettick();
	/*check timer count to target level*/
	while (1) {
		end = core_timer_gettick();
		if ((end - start) > (u32)total_tick) {
			break;
		}
	}
}

void ca53_cycle_count_start(BOOL do_reset, BOOL enable_divider)
{
	enable_cpu_counters();
	return;
}

/**
    CA53 get CPU clock cycle count

    get CPU clock cycle count

    @param[out] type
    @return success or not
        - @b UINT64:   clock cycle of CPU
*/
u64 ca53_get_cycle_count(void)
{
	u64 cval;
	isb();
	asm volatile("mrs %0, PMCCNTR_EL0" : "=r"(cval));
	return cval;
}

void ca53_cycle_count_stop(void)
{
	disable_cpu_counters();
	return;
}

int do_nvt_cpu_get_freq(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
	u64 time_1, time_2, temp, time_interval;
	u32 freq;
	u32 fpga = 0;
	static u32 chip_id;
	if (!strncmp(argv[1], "fpga", 4)) {
		fpga = 1;
	} else if (!strncmp(argv[1], "help", 4)) {
		return CMD_RET_USAGE;
	}
	ca53_cycle_count_start(TRUE, FALSE);
	time_1 = ca53_get_cycle_count();
	if (fpga) {
		core_timer_delay(CALCULATE_CPU_FREQ_UNIT_US);
	} else {
		timer2_delay(CALCULATE_CPU_FREQ_UNIT_US);
	}
	time_2 = ca53_get_cycle_count();
	if (time_2 > time_1) {
		time_interval = (time_2 - time_1);
	} else {
		temp = 0xFFFFFFFF - time_1;
		time_interval = temp + time_2;
	}
	freq = (time_interval) / (CALCULATE_CPU_FREQ_UNIT_US);
	ca53_cycle_count_stop();
	chip_id = nvt_get_chip_id();
	if(chip_id == CHIP_NS02402) {
		printf("CHIP[NS02402] =>");
	} else {
		printf("CHIP[NS02302] =>");
	}
	printf("CPU Freq %d MHz\n", freq);

	return 0;
}

#ifdef CONFIG_NVT_FPGA_EMULATION
U_BOOT_CMD(
	nvt_get_cpu_freq, 2,    1,  do_nvt_cpu_get_freq,
	"get cpu freq",
	"[Option] \n"
	"	          [fpga]\n"
	"	          [MHz]\n"
);
#else
U_BOOT_CMD(
	nvt_get_cpu_freq, 3,    1,  do_nvt_cpu_get_freq,
	"get cpu freq",
	"[MHz]\n"
);
#endif

int do_nvt_cpu_freq(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
	u32 value = simple_strtoul(argv[1], NULL, 10);
	u32 uiReg;
	static u32 chip_id;
#ifdef CONFIG_NVT_FPGA_EMULATION
#else
	int code;
	code = otp_key_manager(4);
	if (code == 0) {
		printf("[InHs][CPU clk =@%dMHz]\n", value);
	} else {
		if(quary_secure_boot(SECUREBOOT_CPU_OVER_CLOCKING) == TRUE) {
			if (value > CPU_OVER_CLKING_FREQ_MAX) {
				value = CPU_OVER_CLKING_FREQ_MAX;
			}
		} else {
			if (value > CPU_NORMAL_FREQ_MAX) {
				value = CPU_NORMAL_FREQ_MAX;
				printf("force CPU clk @%dMHz\n", (CPU_NORMAL_FREQ_MAX));
			}
		}
	}
	chip_id = nvt_get_chip_id();
#endif
	switch (argc) {
	case 2:
		switch (value) {
		case 480:
			uiReg = readl(IOADDR_CG_REG_BASE + SYSTEM_CLOCK_RATE_OFS);
			if ((uiReg & PLL_CLKSEL_CPUMASK) == PLL_CLKSEL_CPU_480) {
				printf("Already 480MHz(48MHz@FPGA)\n");
				break;
			} else {
				uiReg = readl(IOADDR_CG_REG_BASE + SYSTEM_CLOCK_RATE_OFS);
				uiReg &= ~PLL_CLKSEL_CPUMASK;
				uiReg |= PLL_CLKSEL_CPU_480;
				writel(uiReg, (IOADDR_CG_REG_BASE + SYSTEM_CLOCK_RATE_OFS));
				writel(0x34, (IOADDR_STBC_CG_REG_BASE + 0xB0));
				while (!(readl(IOADDR_STBC_CG_REG_BASE + 0xB0) & (1 << 3)));
				writel(0x28, (IOADDR_STBC_CG_REG_BASE + 0xB0));
#ifdef CONFIG_NVT_FPGA_EMULATION
				printf("Set CPU clk 480MHz(48MHz@FPGA)\n");
#else
				printf("Set CPU clk 480MHz\n");
				uiReg = readl(IOADDR_CG_REG_BASE);
				if ((uiReg & (1 << PLL_CPU_NO)) == (1 << PLL_CPU_NO)) {
					printf("PLL%d = Enabled => Disable it\n", PLL_CPU_NO);
					uiReg &= ~(1 << PLL_CPU_NO);
					writel(uiReg, IOADDR_CG_REG_BASE); // B0
					uiReg = readl(IOADDR_CG_REG_BASE);
					printf("PLL%d = 0x%08x \n", PLL_CPU_NO, uiReg);
				}
#endif
			}
			break;
#ifdef CONFIG_NVT_FPGA_EMULATION
		case 80:
			uiReg = readl(IOADDR_CG_REG_BASE + SYSTEM_CLOCK_RATE_OFS);
			if ((uiReg & PLL_CLKSEL_CPUMASK) == PLL_CLKSEL_CPU_80) {
				printf("Already 80MHz(24MHz@FPGA)\n");
				break;
			} else {
				uiReg = readl(IOADDR_CG_REG_BASE + SYSTEM_CLOCK_RATE_OFS);
				uiReg &= ~PLL_CLKSEL_CPUMASK;
				uiReg |= PLL_CLKSEL_CPU_80;
				writel(uiReg, (IOADDR_CG_REG_BASE + SYSTEM_CLOCK_RATE_OFS));
				writel(0x34, (IOADDR_CG_REG_BASE + 0xB0));
				while (!(readl(IOADDR_CG_REG_BASE + 0xB0) & (1 << 3)));
				writel(0x28, (IOADDR_CG_REG_BASE + 0xB0));
				printf("Set CPU clk to 80MHz(24MHz@FPGA)\n");
			}
			break;

		case 1000:
		case 1200:
		case 1400:
		default:
			uiReg = readl(IOADDR_CG_REG_BASE + SYSTEM_CLOCK_RATE_OFS);
			if ((uiReg & PLL_CLKSEL_CPUMASK) == PLL_CLKSEL_CPU_ARMPLL) {
				printf("Already ARMPLL\n");
			} else {
				uiReg = readl(IOADDR_CG_REG_BASE + SYSTEM_CLOCK_RATE_OFS);
				uiReg &= ~PLL_CLKSEL_CPUMASK;
				uiReg |= PLL_CLKSEL_CPU_ARMPLL;
				writel(uiReg, (IOADDR_CG_REG_BASE + SYSTEM_CLOCK_RATE_OFS));
				writel(0x34, (IOADDR_CG_REG_BASE + 0xB0));
				while (!(readl(IOADDR_CG_REG_BASE + 0xB0) & (1 << 3)));
				writel(0x28, (IOADDR_CG_REG_BASE + 0xB0));
				printf("Set CPU clk APLLMHz\n");
			}

			break;
		}
#else
		case 1000:
			uiReg = readl(IOADDR_CG_REG_BASE + SYSTEM_CLOCK_RATE_OFS);
			if ((uiReg & PLL_CLKSEL_CPUMASK) != PLL_CLKSEL_CPU_ARMPLL) {
				uiReg = readl(IOADDR_STBC_CG_REG_BASE);
				if ((uiReg & (1 << PLL_CPU_NO)) != (1 << PLL_CPU_NO)) {
					printf("PLL8 = Disabled => Enable it\n");
					uiReg |= (1 << PLL_CPU_NO);
					writel(uiReg, IOADDR_STBC_CG_REG_BASE); // B0
					uiReg = readl(IOADDR_STBC_CG_REG_BASE);
					//Polling PLL8
					while (!(readl(IOADDR_STBC_CG_REG_BASE + 0x04) & (1 << 8)));
					printf("PLL8 enable done= 0x%08x 0x%08x\n", readl(IOADDR_STBC_CG_REG_BASE + 0x00), readl(IOADDR_STBC_CG_REG_BASE + 0x04));
				}
				uiReg = readl(IOADDR_CG_REG_BASE + SYSTEM_CLOCK_RATE_OFS);
				uiReg &= ~PLL_CLKSEL_CPUMASK;
				uiReg |= PLL_CLKSEL_CPU_ARMPLL;
				writel(uiReg, (IOADDR_CG_REG_BASE + SYSTEM_CLOCK_RATE_OFS));
				writel(0x34, (IOADDR_STBC_CG_REG_BASE + 0xB0));
				while (!(readl(IOADDR_STBC_CG_REG_BASE + 0xB0) & (1 << 3)));
				writel(0x28, (IOADDR_STBC_CG_REG_BASE + 0xB0));
			}
			//CPU 1000 MHz -> CPU-MPLL-125Mhz = 0x14D555
			if(chip_id == CHIP_NS02402) {
				writel(0x55, IOADDR_STBC_CG_REG_BASE + CPU_CLOCK_RATE_539A_RATIO0_OFS); // B0
				writel(0xD5, IOADDR_STBC_CG_REG_BASE + CPU_CLOCK_RATE_539A_RATIO1_OFS); // B1
				writel(0x14, IOADDR_STBC_CG_REG_BASE + CPU_CLOCK_RATE_539A_RATIO2_OFS); // B2
			} else {
				writel(0x55, IOADDR_STBC_CG_REG_BASE + CPU_CLOCK_RATE_RATIO0_OFS); // B0
				writel(0xD5, IOADDR_STBC_CG_REG_BASE + CPU_CLOCK_RATE_RATIO1_OFS); // B1
				writel(0x14, IOADDR_STBC_CG_REG_BASE + CPU_CLOCK_RATE_RATIO2_OFS); // B2
			}
			printf("Set CPU clk 1000MHz\n");
			break;
		case 1100:
			uiReg = readl(IOADDR_CG_REG_BASE + SYSTEM_CLOCK_RATE_OFS);
			if ((uiReg & PLL_CLKSEL_CPUMASK) != PLL_CLKSEL_CPU_ARMPLL) {
				uiReg = readl(IOADDR_STBC_CG_REG_BASE);
				if ((uiReg & (1 << PLL_CPU_NO)) != (1 << PLL_CPU_NO)) {
					printf("PLL8 = Disabled => Enable it\n");
					uiReg |= (1 << PLL_CPU_NO);
					writel(uiReg, IOADDR_STBC_CG_REG_BASE); // B0
					uiReg = readl(IOADDR_STBC_CG_REG_BASE);
					//Polling PLL8
					while (!(readl(IOADDR_STBC_CG_REG_BASE + 0x04) & (1 << 8)));
					printf("PLL8 enable done= 0x%08x 0x%08x\n", readl(IOADDR_STBC_CG_REG_BASE + 0x00), readl(IOADDR_STBC_CG_REG_BASE + 0x04));
				}
				uiReg = readl(IOADDR_CG_REG_BASE + SYSTEM_CLOCK_RATE_OFS);
				uiReg &= ~PLL_CLKSEL_CPUMASK;
				uiReg |= PLL_CLKSEL_CPU_ARMPLL;
				writel(uiReg, (IOADDR_CG_REG_BASE + SYSTEM_CLOCK_RATE_OFS));
				writel(0x34, (IOADDR_STBC_CG_REG_BASE + 0xB0));
				while (!(readl(IOADDR_STBC_CG_REG_BASE + 0xB0) & (1 << 3)));
				writel(0x28, (IOADDR_STBC_CG_REG_BASE + 0xB0));
			}
			//CPU 1100 MHz -> CPU-MPLL-125Mhz = 0x16eaaa
			if(chip_id == CHIP_NS02402) {
				writel(0xaa, IOADDR_STBC_CG_REG_BASE + CPU_CLOCK_RATE_539A_RATIO0_OFS); // B0
				writel(0xea, IOADDR_STBC_CG_REG_BASE + CPU_CLOCK_RATE_539A_RATIO1_OFS); // B1
				writel(0x16, IOADDR_STBC_CG_REG_BASE + CPU_CLOCK_RATE_539A_RATIO2_OFS); // B2
			} else {
				writel(0xaa, IOADDR_STBC_CG_REG_BASE + CPU_CLOCK_RATE_RATIO0_OFS); // B0
				writel(0xea, IOADDR_STBC_CG_REG_BASE + CPU_CLOCK_RATE_RATIO1_OFS); // B1
				writel(0x16, IOADDR_STBC_CG_REG_BASE + CPU_CLOCK_RATE_RATIO2_OFS); // B2
			}
			printf("Set CPU clk 1100MHz\n");
			break;
		case 1200:
			uiReg = readl(IOADDR_CG_REG_BASE + SYSTEM_CLOCK_RATE_OFS);
			if ((uiReg & PLL_CLKSEL_CPUMASK) != PLL_CLKSEL_CPU_ARMPLL) {
				uiReg = readl(IOADDR_STBC_CG_REG_BASE);
				if ((uiReg & (1 << PLL_CPU_NO)) != (1 << PLL_CPU_NO)) {
					printf("PLL8 = Disabled => Enable it\n");
					uiReg |= (1 << PLL_CPU_NO);
					writel(uiReg, IOADDR_STBC_CG_REG_BASE); // B0
					uiReg = readl(IOADDR_STBC_CG_REG_BASE);
					//Polling PLL8
					while (!(readl(IOADDR_STBC_CG_REG_BASE + 0x04) & (1 << 8)));
					printf("PLL8 enable done= 0x%08x 0x%08x\n", readl(IOADDR_STBC_CG_REG_BASE + 0x00), readl(IOADDR_STBC_CG_REG_BASE + 0x04));
				}
				uiReg = readl(IOADDR_CG_REG_BASE + SYSTEM_CLOCK_RATE_OFS);
				uiReg &= ~PLL_CLKSEL_CPUMASK;
				uiReg |= PLL_CLKSEL_CPU_ARMPLL;
				writel(uiReg, (IOADDR_CG_REG_BASE + SYSTEM_CLOCK_RATE_OFS));
				writel(0x34, (IOADDR_STBC_CG_REG_BASE + 0xB0));
				while (!(readl(IOADDR_STBC_CG_REG_BASE + 0xB0) & (1 << 3)));
				writel(0x28, (IOADDR_STBC_CG_REG_BASE + 0xB0));
			}
			//CPU 1200 MHz -> CPU-MPLL-125Mhz = 0x16eaaa
			if(chip_id == CHIP_NS02402) {
				writel(0x00, IOADDR_STBC_CG_REG_BASE + CPU_CLOCK_RATE_539A_RATIO0_OFS); // B0
				writel(0x00, IOADDR_STBC_CG_REG_BASE + CPU_CLOCK_RATE_539A_RATIO1_OFS); // B1
				writel(0x19, IOADDR_STBC_CG_REG_BASE + CPU_CLOCK_RATE_539A_RATIO2_OFS); // B2
			} else {
				writel(0x00, IOADDR_STBC_CG_REG_BASE + CPU_CLOCK_RATE_RATIO0_OFS); // B0
				writel(0x00, IOADDR_STBC_CG_REG_BASE + CPU_CLOCK_RATE_RATIO1_OFS); // B1
				writel(0x19, IOADDR_STBC_CG_REG_BASE + CPU_CLOCK_RATE_RATIO2_OFS); // B2
			}
			printf("Set CPU clk 1200MHz\n");
			break;
		case 1300:
			uiReg = readl(IOADDR_CG_REG_BASE + SYSTEM_CLOCK_RATE_OFS);
			if ((uiReg & PLL_CLKSEL_CPUMASK) != PLL_CLKSEL_CPU_ARMPLL) {
				uiReg = readl(IOADDR_STBC_CG_REG_BASE);
				if ((uiReg & (1 << PLL_CPU_NO)) != (1 << PLL_CPU_NO)) {
					printf("PLL8 = Disabled => Enable it\n");
					uiReg |= (1 << PLL_CPU_NO);
					writel(uiReg, IOADDR_STBC_CG_REG_BASE); // B0
					uiReg = readl(IOADDR_STBC_CG_REG_BASE);
					//Polling PLL8
					while (!(readl(IOADDR_STBC_CG_REG_BASE + 0x04) & (1 << 8)));
					printf("PLL8 enable done= 0x%08x 0x%08x\n", readl(IOADDR_STBC_CG_REG_BASE + 0x00), readl(IOADDR_STBC_CG_REG_BASE + 0x04));
				}
				uiReg = readl(IOADDR_CG_REG_BASE + SYSTEM_CLOCK_RATE_OFS);
				uiReg &= ~PLL_CLKSEL_CPUMASK;
				uiReg |= PLL_CLKSEL_CPU_ARMPLL;
				writel(uiReg, (IOADDR_CG_REG_BASE + SYSTEM_CLOCK_RATE_OFS));
				writel(0x34, (IOADDR_STBC_CG_REG_BASE + 0xB0));
				while (!(readl(IOADDR_STBC_CG_REG_BASE + 0xB0) & (1 << 3)));
				writel(0x28, (IOADDR_STBC_CG_REG_BASE + 0xB0));
			}
			//CPU 1300 MHz -> CPU-MPLL-162.5Mhz = 0x16eaaa
			if(chip_id == CHIP_NS02402) {
				writel(0x55, IOADDR_STBC_CG_REG_BASE + CPU_CLOCK_RATE_539A_RATIO0_OFS); // B0
				writel(0x15, IOADDR_STBC_CG_REG_BASE + CPU_CLOCK_RATE_539A_RATIO1_OFS); // B1
				writel(0x1b, IOADDR_STBC_CG_REG_BASE + CPU_CLOCK_RATE_539A_RATIO2_OFS); // B2
			} else {
				writel(0x55, IOADDR_STBC_CG_REG_BASE + CPU_CLOCK_RATE_RATIO0_OFS); // B0
				writel(0x15, IOADDR_STBC_CG_REG_BASE + CPU_CLOCK_RATE_RATIO1_OFS); // B1
				writel(0x1b, IOADDR_STBC_CG_REG_BASE + CPU_CLOCK_RATE_RATIO2_OFS); // B2
			}
			printf("Set CPU clk 1300MHz\n");
			break;

		default:
			printf("Not define this CPU Freq %d, use defualt value 1100!\n", value);
			uiReg = readl(IOADDR_CG_REG_BASE + SYSTEM_CLOCK_RATE_OFS);
			if ((uiReg & PLL_CLKSEL_CPUMASK) != PLL_CLKSEL_CPU_ARMPLL) {
				uiReg = readl(IOADDR_STBC_CG_REG_BASE);
				if ((uiReg & (1 << PLL_CPU_NO)) != (1 << PLL_CPU_NO)) {
					printf("PLL8 = Disabled => Enable it\n");
					uiReg |= (1 << PLL_CPU_NO);
					writel(uiReg, IOADDR_STBC_CG_REG_BASE); // B0
					uiReg = readl(IOADDR_STBC_CG_REG_BASE);
					//Polling PLL8
					while (!(readl(IOADDR_STBC_CG_REG_BASE + 0x04) & (1 << 8)));
					printf("PLL8 enable done= 0x%08x 0x%08x\n", readl(IOADDR_STBC_CG_REG_BASE + 0x00), readl(IOADDR_STBC_CG_REG_BASE + 0x04));
				}
				uiReg = readl(IOADDR_CG_REG_BASE + SYSTEM_CLOCK_RATE_OFS);
				uiReg &= ~PLL_CLKSEL_CPUMASK;
				uiReg |= PLL_CLKSEL_CPU_ARMPLL;
				writel(uiReg, (IOADDR_CG_REG_BASE + SYSTEM_CLOCK_RATE_OFS));
				writel(0x34, (IOADDR_STBC_CG_REG_BASE + 0xB0));
				while (!(readl(IOADDR_STBC_CG_REG_BASE + 0xB0) & (1 << 3)));
				writel(0x28, (IOADDR_STBC_CG_REG_BASE + 0xB0));
			}
			//CPU 1100 MHz -> CPU-MPLL-125Mhz = 0x16eaaa
			if(chip_id == CHIP_NS02402) {
				writel(0xaa, IOADDR_STBC_CG_REG_BASE + CPU_CLOCK_RATE_539A_RATIO0_OFS); // B0
				writel(0xea, IOADDR_STBC_CG_REG_BASE + CPU_CLOCK_RATE_539A_RATIO1_OFS); // B1
				writel(0x16, IOADDR_STBC_CG_REG_BASE + CPU_CLOCK_RATE_539A_RATIO2_OFS); // B2
			} else {
				writel(0xaa, IOADDR_STBC_CG_REG_BASE + CPU_CLOCK_RATE_RATIO0_OFS); // B0
				writel(0xea, IOADDR_STBC_CG_REG_BASE + CPU_CLOCK_RATE_RATIO1_OFS); // B1
				writel(0x16, IOADDR_STBC_CG_REG_BASE + CPU_CLOCK_RATE_RATIO2_OFS); // B2
			}
			printf("Set CPU clk 1100MHz\n");
			break;
		}
		break;
#endif
	default:
		printf("CPU:%d => type nvt_cpu_freq <freq>\n", (int)get_cpu_clk());
		return CMD_RET_USAGE;
	}
	return 0;
}


U_BOOT_CMD(
	nvt_cpu_freq, 2,    1,  do_nvt_cpu_freq,
	"change cpu freq",
	"[MHz]\n"
#ifdef CONFIG_NVT_FPGA_EMULATION
	"nvt_cpu_freq 480(change to 48MHz@FPGA)\n"
	"nvt_cpu_freq  80(change to 24MHz@FPGA)\n"
	"nvt_cpu_freq !=480 && != 80(will change to ARMPLL@FPGA, check excel)\n"
#else
	"nvt_cpu_freq 1000\n"
	"nvt_cpu_freq 1100\n"
	"nvt_cpu_freq 1300(if 539A)\n"
#endif
);

int do_nvt_apb_freq(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
	u32 uiReg;
	u32 value = simple_strtoul(argv[1], NULL, 10);

	switch (argc) {
	case 2:
		uiReg = readl(IOADDR_STBC_CG_REG_BASE + SYSTEM_CLOCK_RATE_OFS);
		uiReg &= ~(0x300);
		switch (value) {
		case 48:
			uiReg |= PLL_CLKSEL_APB48;
			writel(uiReg, IOADDR_STBC_CG_REG_BASE + SYSTEM_CLOCK_RATE_OFS);
			break;

		case 60:
			uiReg |= PLL_CLKSEL_APB60;
			writel(uiReg, IOADDR_STBC_CG_REG_BASE + SYSTEM_CLOCK_RATE_OFS);
			break;
		case 80:
			uiReg |= PLL_CLKSEL_APB80;
			writel(uiReg, IOADDR_STBC_CG_REG_BASE + SYSTEM_CLOCK_RATE_OFS);
			break;
		case 120:
			uiReg |= PLL_CLKSEL_APB120;
			writel(uiReg, IOADDR_STBC_CG_REG_BASE + SYSTEM_CLOCK_RATE_OFS);
			break;

		default:
			printf("APB:%d => type nvt_apb_freq <freq>\n", (int)value);
			break;
			return CMD_RET_USAGE;
		}
		break;
	default:
		printf("APB:%d => type nvt_apb_freq <freq>\n", value);
		return CMD_RET_USAGE;
	}
	return 0;
}
U_BOOT_CMD(
	nvt_apb_freq, 2,    1,  do_nvt_apb_freq,
	"change apb freq",
	"[MHz]\n"
	"nvt_apb_freq 48\n"
	"nvt_apb_freq 60\n"
	"nvt_apb_freq 80\n"
	"nvt_apb_freq 120\n"
);


int do_nvt_ddr_freq(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
	unsigned int freq_ratio;
	unsigned int freq_ratio0, freq_ratio1, freq_ratio2;

	//0xF0020000 + (0x4000 + 0x400 + 0x20) = 0xF0024400 + 0x20
	freq_ratio0 = readl(IOADDR_STBC_CG_REG_BASE + \
						DMA_CLOCK_RATE_RATIO0_OFS);

	//0xF0020000 + (0x4000 + 0x400 + 0x20) = 0xF0024400 + 0x24
	freq_ratio1 = readl(IOADDR_STBC_CG_REG_BASE + \
						DMA_CLOCK_RATE_RATIO1_OFS);

	//0xF0020000 + (0x4000 + 0x400 + 0x20) = 0xF0024400 + 0x28
	freq_ratio2 = readl(IOADDR_STBC_CG_REG_BASE + \
						DMA_CLOCK_RATE_RATIO2_OFS);
	freq_ratio = freq_ratio0 | (freq_ratio1 << 8) | \
				 (freq_ratio2 << 16);

	freq_ratio = (freq_ratio << 3);
	printf("DMA data rate = %d => [%d]MHz\r\n", (12 * freq_ratio / 131072), ((12 * freq_ratio / 131072) >> 1));


	return 0;
}

U_BOOT_CMD(
	nvt_get_ddr_freq, 2,      1,     do_nvt_ddr_freq,
	"get ddr freq/type\n",
	"\n"
);

int prepare_power_trim(int mode)
{
    unsigned int	remap_data;
    char cmd[50];
	static u32 chip_id;

	chip_id = nvt_get_chip_id();
	if (chip_id != CHIP_NS02402) {
		return 0;
	}



	//Fixed @ 0.95V
	remap_data = 19;

	printf("remap_data %d\n", remap_data);
	sprintf(cmd, "power_ctl %d", remap_data);
	run_command(cmd, 0);

    return 0;
}