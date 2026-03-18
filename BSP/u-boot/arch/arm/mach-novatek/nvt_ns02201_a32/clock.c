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
#include <asm/armv7.h>
#include <asm/io.h>
#include <asm/arch/clock.h>
#include <asm/arch/IOAddress.h>
#include <asm/arch/hardware.h>
#include <asm/nvt-common/nvt_types.h>
#include <asm/nvt-common/nvt_common.h>
#include <asm/nvt-common/rcw_macro.h>
#include <asm/arch/efuse_protected.h>

#define SYSTEM_CLOCK_RATE_OFS           0x10
#define CPU_CLOCK_RATE_RATIO0_OFS       0x4820
#define CPU_CLOCK_RATE_RATIO1_OFS       0x4824
#define CPU_CLOCK_RATE_RATIO2_OFS       0x4828
#define DMA_CLOCK_RATE_RATIO0_OFS       (0x4000 + 0x4C0 + 0x20)
#define DMA_CLOCK_RATE_RATIO1_OFS       (0x4000 + 0x4C0 + 0x24)
#define DMA_CLOCK_RATE_RATIO2_OFS       (0x4000 + 0x4C0 + 0x28)

#define DMA2_CLOCK_RATE_RATIO0_OFS      (0x4000 + 0xC00 + 0x20)
#define DMA2_CLOCK_RATE_RATIO1_OFS      (0x4000 + 0xC00 + 0x24)
#define DMA2_CLOCK_RATE_RATIO2_OFS      (0x4000 + 0xC00 + 0x28)



#define PLL_CPU_NO                      16                              //No. of CPU's PLL = 8
#define PLL_CLKSEL_CPU                  (0)
#define PLL_CLKSEL_CPU_80               (0x00 << PLL_CLKSEL_CPU)        //< Select CPU clock 80MHz
#define PLL_CLKSEL_CPU_ARMPLL           (0x01 << PLL_CLKSEL_CPU)        //< Select CPU clock ARMPLL (for CPU)
#define PLL_CLKSEL_CPU_480              (0x02 << PLL_CLKSEL_CPU)        //< Select CPU clock 480MHz
#define PLL_CLKSEL_CPUMASK              (0x3)

#define PLL_CLKSEL_APBMASK              (0x1<<8)
#define PLL_CLKSEL_APB120               (0x0<<8)
#define PLL_CLKSEL_APB150               (0x1<<8)

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

#define CPU_NORMAL_FREQ_MAX             (1500000000)        //1.5GHz
#define CPU_OVER_CLKING_FREQ_MAX        (1600000000)        //1.6GHz

void core_timer_delay(u32 us);
u32 ca53_get_cycle_count(void);
void ca53_cycle_count_stop(void);
void ca53_cycle_count_start(BOOL do_reset, BOOL enable_divider);

inline void apll_set_data(u8 offset, u8 value)
{
#if 0
	writel(value, (APLL_BASE_ADDR + (offset * 4)));
#endif
}

inline u32 apll_get_data(u8 offset)
{
	return 0;
}

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

#if 0
inline void apll_enable(pll_page_t page)
{
	if (PLL_PAGE_0 == page) {
		writel(APLL_PAGE_0_EN, (APLL_PAGE_EN_ADDR));
	} else if (PLL_PAGE_B == page) {
		writel(APLL_PAGE_B_EN, (APLL_PAGE_EN_ADDR));
	} else {
		/* ignore */
	}
}
#endif

inline void mpll_set_data(u8 offset, u8 value)
{
#if 0
	writel(value, (MPLL_BASE_ADDR + (offset * 4)));
#endif
}

inline u32 mpll_get_data(u8 offset)
{
	return 0;
#if 0
	return readl((MPLL_BASE_ADDR + (offset * 4)));
#endif
}

#if 0
inline void mpll_enable(pll_page_t page)
{
	if (PLL_PAGE_0 == page) {
		writel(MPLL_PAGE_0_EN, (MPLL_PAGE_EN_ADDR));
	} else if (PLL_PAGE_B == page) {
		writel(MPLL_PAGE_B_EN, (MPLL_PAGE_EN_ADDR));
	} else {
		/* ignore */
	}
}
#endif

void set_sys_mpll(unsigned long off, unsigned long val)
{
#if 0
	val <<= 17;
	val  /= 12;

	mpll_enable(PLL_PAGE_B);

	mpll_set_data((off + 0), ((val >> 0) & 0xff));
	mpll_set_data((off + 1), ((val >> 8) & 0xff));
	mpll_set_data((off + 2), ((val >> 16) & 0xff));
#endif
}

unsigned long get_sys_mpll(unsigned long off)
{
#if 0
	unsigned long val;

	mpll_enable(PLL_PAGE_B);

	val  = mpll_get_data(off);
	val |= (mpll_get_data((off + 1)) << 8);
	val |= (mpll_get_data((off + 2)) << 16);

	val *= 12;
	val += ((1UL << 17) - 1);
	val >>= 17;

	return val;
#endif
	return 0;
}

void set_cpu_clk(unsigned long freq)
{
	/* no implement */
}

unsigned long get_cpu_clk(void)
{
	unsigned int cpu_clk_sel = 0;
#ifndef CONFIG_NVT_FPGA_EMULATION
	unsigned int cpu_freq_ratio;
	unsigned int cpu_freq_ratio0, cpu_freq_ratio1, cpu_freq_ratio2;
#else
	return  nvt_cpu_get_freq_by_core_timer();
#endif
	cpu_clk_sel = readl(IOADDR_CG_REG_BASE + SYSTEM_CLOCK_RATE_OFS) & 0x3;

	if (cpu_clk_sel == 1) {
#ifdef CONFIG_NVT_FPGA_EMULATION
		return 24000;
#else
		cpu_freq_ratio0 = readl(IOADDR_CG_REG_BASE + \
								CPU_CLOCK_RATE_RATIO0_OFS);
		cpu_freq_ratio1 = readl(IOADDR_CG_REG_BASE + \
								CPU_CLOCK_RATE_RATIO1_OFS);
		cpu_freq_ratio2 = readl(IOADDR_CG_REG_BASE + \
								CPU_CLOCK_RATE_RATIO2_OFS);
		cpu_freq_ratio = cpu_freq_ratio0 | (cpu_freq_ratio1 << 8) | \
						 (cpu_freq_ratio2 << 16);

		cpu_freq_ratio = (cpu_freq_ratio << 5);

		return (12 * cpu_freq_ratio / 131072) * 1000;
#endif
	} else if (cpu_clk_sel == 2) {
		return 480000;
	} else {
		return 80000;
	}
}



#define read_PMCR() \
	({ \
		unsigned long cfg; \
		__asm__ __volatile__(\
							 "mrc p15, 0, %0, c9, c12, 0\n\t" \
							 : "=r"(cfg) \
							);\
		cfg;\
	})

#define write_PMCR(m)\
	({\
		__asm__ __volatile__("mcr p15, 0, %0, c9, c12, 0" : : "r" (m));\
	})

static BOOL cpu_count_open = FALSE;
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
	/* in general enable all counters (including cycle counter)*/
	int value = 1;

	if (cpu_count_open == TRUE) {
		printf("cpu count start already\r\n");
		return;
	}

	cpu_count_open = TRUE;

	if (do_reset) {
		value |= 2;     /* reset all counters to zero.*/
		value |= 4;     /* reset cycle counter to zero.*/
	}

	if (enable_divider) {
		value |= 8;    /* enable "by 64" divider for CCNT.*/
	}

	value |= 16;

	/* program the performance-counter control-register: */
	asm volatile("MCR p15, 0, %0, c9, c12, 0\t\n" :: "r"(value));

	/* enable all counters: */
	asm volatile("MCR p15, 0, %0, c9, c12, 1\t\n" :: "r"(0x8000000f));

	/* clear overflows: */
	asm volatile("MCR p15, 0, %0, c9, c12, 3\t\n" :: "r"(0x8000000f));
	return;
}

/**
    CA53 get CPU clock cycle count

    get CPU clock cycle count

    @param[out] type
    @return success or not
        - @b UINT64:   clock cycle of CPU
*/
u32 ca53_get_cycle_count(void)
{
	unsigned int value;
	__asm__ __volatile__("mrc p15, 0, %0, c9, c13, 0\n\t"
						 : "=r"(value)
						);
	return value;
}

void ca53_cycle_count_stop(void)
{
	u32 pmcr_reg;

	if (cpu_count_open == FALSE) {
		printf("cpu count not start yet\r\n");
		return;
	}

	cpu_count_open = FALSE;
	pmcr_reg = read_PMCR();
	pmcr_reg &= ~(0x1 << 0); /*E*/
	pmcr_reg &= ~(0x1 << 5); /*DP increase each clock cycle*/

	write_PMCR(pmcr_reg);
	return;
}



int do_nvt_cpu_get_freq(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
	u32 fpga = 0;
	u32 /*test, */freq;
	u32 time_1, time_2, temp, time_interval;

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
	printf("CHIP[NS02201] =>");
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
				printf("force CPU clk @%dMHz\n", (CPU_NORMAL_FREQ_MAX/1000000));
			}
		}
	}
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
				writel(0x34, (IOADDR_CG_REG_BASE + 0xB0));
				while (!(readl(IOADDR_CG_REG_BASE + 0xB0) & (1 << 3)));
				writel(0x28, (IOADDR_CG_REG_BASE + 0xB0));
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
#else
		case 1000:
			uiReg = readl(IOADDR_CG_REG_BASE + SYSTEM_CLOCK_RATE_OFS);
			if ((uiReg & PLL_CLKSEL_CPUMASK) != PLL_CLKSEL_CPU_ARMPLL) {
				uiReg = readl(IOADDR_CG_REG_BASE);
				if ((uiReg & (1 << PLL_CPU_NO)) != (1 << PLL_CPU_NO)) {
					printf("PLL%d = Disabled => Enable it\n", PLL_CPU_NO);
					uiReg |= (1 << PLL_CPU_NO);
					writel(uiReg, IOADDR_CG_REG_BASE); // B0
					uiReg = readl(IOADDR_CG_REG_BASE);
					//Polling PLL16
					while (!(readl(IOADDR_CG_REG_BASE + 0x04) & (1 << PLL_CPU_NO)));
					printf("PLL%d enable done= 0x%08x 0x%08x\n", PLL_CPU_NO, readl(IOADDR_CG_REG_BASE + 0x00), readl(IOADDR_CG_REG_BASE + 0x04));
				}
				uiReg = readl(IOADDR_CG_REG_BASE + SYSTEM_CLOCK_RATE_OFS);
				uiReg &= ~PLL_CLKSEL_CPUMASK;
				uiReg |= PLL_CLKSEL_CPU_ARMPLL;
				writel(uiReg, (IOADDR_CG_REG_BASE + SYSTEM_CLOCK_RATE_OFS));
				writel(0x34, (IOADDR_CG_REG_BASE + 0xB0));
				while (!(readl(IOADDR_CG_REG_BASE + 0xB0) & (1 << 3)));
				writel(0x28, (IOADDR_CG_REG_BASE + 0xB0));
			}
			//CPU 1000 MHz -> CPU-MPLL-125Mhz = 0x053555
			writel(0x55, IOADDR_CG_REG_BASE + CPU_CLOCK_RATE_RATIO0_OFS); // B0
			writel(0x35, IOADDR_CG_REG_BASE + CPU_CLOCK_RATE_RATIO1_OFS); // B1
			writel(0x05, IOADDR_CG_REG_BASE + CPU_CLOCK_RATE_RATIO2_OFS); // B2
			if ((INREG32(IOADDR_CG_REG_BASE + 0x8000) & 0x1) != 0x1) {
				timer2_delay(300);
				//resetb_armpll --> enable PLL & BG LDO ON
				SETREG32(IOADDR_CG_REG_BASE + 0x8000 + 0x10, 0x3);
				timer2_delay(500);
				//select ti ARMPLL
				SETREG32(IOADDR_CG_REG_BASE + 0x8000, 0x1);
			}
			printf("Set CPU clk 1000MHz\n");
			break;
		case 1200:
			uiReg = readl(IOADDR_CG_REG_BASE + SYSTEM_CLOCK_RATE_OFS);
			if ((uiReg & PLL_CLKSEL_CPUMASK) != PLL_CLKSEL_CPU_ARMPLL) {
				uiReg = readl(IOADDR_CG_REG_BASE);
				if ((uiReg & (1 << PLL_CPU_NO)) != (1 << PLL_CPU_NO)) {
					printf("PLL%d = Disabled => Enable it\n", PLL_CPU_NO);
					uiReg |= (1 << PLL_CPU_NO);
					writel(uiReg, IOADDR_CG_REG_BASE); // B0
					uiReg = readl(IOADDR_CG_REG_BASE);
					//Polling PLL16
					while (!(readl(IOADDR_CG_REG_BASE + 0x04) & (1 << PLL_CPU_NO)));
					printf("PLL%d enable done= 0x%08x 0x%08x\n", PLL_CPU_NO, readl(IOADDR_CG_REG_BASE + 0x00), readl(IOADDR_CG_REG_BASE + 0x04));
				}
				uiReg = readl(IOADDR_CG_REG_BASE + SYSTEM_CLOCK_RATE_OFS);
				uiReg &= ~PLL_CLKSEL_CPUMASK;
				uiReg |= PLL_CLKSEL_CPU_ARMPLL;
				writel(uiReg, (IOADDR_CG_REG_BASE + SYSTEM_CLOCK_RATE_OFS));
				writel(0x34, (IOADDR_CG_REG_BASE + 0xB0));
				while (!(readl(IOADDR_CG_REG_BASE + 0xB0) & (1 << 3)));
				writel(0x28, (IOADDR_CG_REG_BASE + 0xB0));
			}
			//CPU 1200 MHz -> CPU-MPLL-150Mhz = 0x064000
			writel(0x00, IOADDR_CG_REG_BASE + CPU_CLOCK_RATE_RATIO0_OFS); // B0
			writel(0x40, IOADDR_CG_REG_BASE + CPU_CLOCK_RATE_RATIO1_OFS); // B1
			writel(0x06, IOADDR_CG_REG_BASE + CPU_CLOCK_RATE_RATIO2_OFS); // B2
			if ((INREG32(IOADDR_CG_REG_BASE + 0x8000) & 0x1) != 0x1) {
				timer2_delay(300);
				//resetb_armpll --> enable PLL & BG LDO ON
				SETREG32(IOADDR_CG_REG_BASE + 0x8000 + 0x10, 0x3);
				timer2_delay(500);
				//select ti ARMPLL
				SETREG32(IOADDR_CG_REG_BASE + 0x8000, 0x1);
			}
			printf("Set CPU clk 1200MHz\n");
			break;

		case 1400:
			uiReg = readl(IOADDR_CG_REG_BASE + SYSTEM_CLOCK_RATE_OFS);
			if ((uiReg & PLL_CLKSEL_CPUMASK) != PLL_CLKSEL_CPU_ARMPLL) {
				uiReg = readl(IOADDR_CG_REG_BASE);
				if ((uiReg & (1 << PLL_CPU_NO)) != (1 << PLL_CPU_NO)) {
					printf("PLL%d = Disabled => Enable it\n", PLL_CPU_NO);
					uiReg |= (1 << PLL_CPU_NO);
					writel(uiReg, IOADDR_CG_REG_BASE); // B0
					uiReg = readl(IOADDR_CG_REG_BASE);
					//Polling PLL16
					while (!(readl(IOADDR_CG_REG_BASE + 0x04) & (1 << PLL_CPU_NO)));
					printf("PLL%d enable done= 0x%08x 0x%08x\n", PLL_CPU_NO, readl(IOADDR_CG_REG_BASE + 0x00), readl(IOADDR_CG_REG_BASE + 0x04));
				}
				uiReg = readl(IOADDR_CG_REG_BASE + SYSTEM_CLOCK_RATE_OFS);
				uiReg &= ~PLL_CLKSEL_CPUMASK;
				uiReg |= PLL_CLKSEL_CPU_ARMPLL;
				writel(uiReg, (IOADDR_CG_REG_BASE + SYSTEM_CLOCK_RATE_OFS));
				writel(0x34, (IOADDR_CG_REG_BASE + 0xB0));
				while (!(readl(IOADDR_CG_REG_BASE + 0xB0) & (1 << 3)));
				writel(0x28, (IOADDR_CG_REG_BASE + 0xB0));
			}
			//CPU 1400 MHz -> CPU-MPLL-150Mhz = 0x074AAA
			writel(0xAA, IOADDR_CG_REG_BASE + CPU_CLOCK_RATE_RATIO0_OFS); // B0
			writel(0x4A, IOADDR_CG_REG_BASE + CPU_CLOCK_RATE_RATIO1_OFS); // B1
			writel(0x07, IOADDR_CG_REG_BASE + CPU_CLOCK_RATE_RATIO2_OFS); // B2
			if ((INREG32(IOADDR_CG_REG_BASE + 0x8000) & 0x1) != 0x1) {
				timer2_delay(300);
				//resetb_armpll --> enable PLL & BG LDO ON
				SETREG32(IOADDR_CG_REG_BASE + 0x8000 + 0x10, 0x3);
				timer2_delay(500);
				//select ti ARMPLL
				SETREG32(IOADDR_CG_REG_BASE + 0x8000, 0x1);
			}
			printf("Set CPU clk 1400MHz\n");
			break;

		case 1500:
			uiReg = readl(IOADDR_CG_REG_BASE + SYSTEM_CLOCK_RATE_OFS);
			if ((uiReg & PLL_CLKSEL_CPUMASK) != PLL_CLKSEL_CPU_ARMPLL) {
				uiReg = readl(IOADDR_CG_REG_BASE);
				if ((uiReg & (1 << PLL_CPU_NO)) != (1 << PLL_CPU_NO)) {
					printf("PLL%d = Disabled => Enable it\n", PLL_CPU_NO);
					uiReg |= (1 << PLL_CPU_NO);
					writel(uiReg, IOADDR_CG_REG_BASE); // B0
					uiReg = readl(IOADDR_CG_REG_BASE);
					//Polling PLL16
					while (!(readl(IOADDR_CG_REG_BASE + 0x04) & (1 << PLL_CPU_NO)));
					printf("PLL%d enable done= 0x%08x 0x%08x\n", PLL_CPU_NO, readl(IOADDR_CG_REG_BASE + 0x00), readl(IOADDR_CG_REG_BASE + 0x04));
				}
				uiReg = readl(IOADDR_CG_REG_BASE + SYSTEM_CLOCK_RATE_OFS);
				uiReg &= ~PLL_CLKSEL_CPUMASK;
				uiReg |= PLL_CLKSEL_CPU_ARMPLL;
				writel(uiReg, (IOADDR_CG_REG_BASE + SYSTEM_CLOCK_RATE_OFS));
				writel(0x34, (IOADDR_CG_REG_BASE + 0xB0));
				while (!(readl(IOADDR_CG_REG_BASE + 0xB0) & (1 << 3)));
				writel(0x28, (IOADDR_CG_REG_BASE + 0xB0));
			}
			//CPU 1500 MHz -> CPU-MPLL-150Mhz = 0x074AAA
			writel(0x00, IOADDR_CG_REG_BASE + CPU_CLOCK_RATE_RATIO0_OFS); // B0
			writel(0xD0, IOADDR_CG_REG_BASE + CPU_CLOCK_RATE_RATIO1_OFS); // B1
			writel(0x07, IOADDR_CG_REG_BASE + CPU_CLOCK_RATE_RATIO2_OFS); // B2
			if ((INREG32(IOADDR_CG_REG_BASE + 0x8000) & 0x1) != 0x1) {
				timer2_delay(300);
				//resetb_armpll --> enable PLL & BG LDO ON
				SETREG32(IOADDR_CG_REG_BASE + 0x8000 + 0x10, 0x3);
				timer2_delay(500);
				//select ti ARMPLL
				SETREG32(IOADDR_CG_REG_BASE + 0x8000, 0x1);
			}
			printf("Set CPU clk 1500MHz\n");
			break;

		case 1600:
			uiReg = readl(IOADDR_CG_REG_BASE + SYSTEM_CLOCK_RATE_OFS);
			if ((uiReg & PLL_CLKSEL_CPUMASK) != PLL_CLKSEL_CPU_ARMPLL) {
				uiReg = readl(IOADDR_CG_REG_BASE);
				if ((uiReg & (1 << PLL_CPU_NO)) != (1 << PLL_CPU_NO)) {
					printf("PLL%d = Disabled => Enable it\n", PLL_CPU_NO);
					uiReg |= (1 << PLL_CPU_NO);
					writel(uiReg, IOADDR_CG_REG_BASE); // B0
					uiReg = readl(IOADDR_CG_REG_BASE);
					//Polling PLL16
					while (!(readl(IOADDR_CG_REG_BASE + 0x04) & (1 << PLL_CPU_NO)));
					printf("PLL%d enable done= 0x%08x 0x%08x\n", PLL_CPU_NO, readl(IOADDR_CG_REG_BASE + 0x00), readl(IOADDR_CG_REG_BASE + 0x04));
				}
				uiReg = readl(IOADDR_CG_REG_BASE + SYSTEM_CLOCK_RATE_OFS);
				uiReg &= ~PLL_CLKSEL_CPUMASK;
				uiReg |= PLL_CLKSEL_CPU_ARMPLL;
				writel(uiReg, (IOADDR_CG_REG_BASE + SYSTEM_CLOCK_RATE_OFS));
				writel(0x34, (IOADDR_CG_REG_BASE + 0xB0));
				while (!(readl(IOADDR_CG_REG_BASE + 0xB0) & (1 << 3)));
				writel(0x28, (IOADDR_CG_REG_BASE + 0xB0));
			}
			//CPU 1600 MHz -> CPU-MPLL-150Mhz = 0x085555
			writel(0x55, IOADDR_CG_REG_BASE + CPU_CLOCK_RATE_RATIO0_OFS); // B0
			writel(0x55, IOADDR_CG_REG_BASE + CPU_CLOCK_RATE_RATIO1_OFS); // B1
			writel(0x08, IOADDR_CG_REG_BASE + CPU_CLOCK_RATE_RATIO2_OFS); // B2
			if ((INREG32(IOADDR_CG_REG_BASE + 0x8000) & 0x1) != 0x1) {
				timer2_delay(300);
				//resetb_armpll --> enable PLL & BG LDO ON
				SETREG32(IOADDR_CG_REG_BASE + 0x8000 + 0x10, 0x3);
				timer2_delay(500);
				//select ti ARMPLL
				SETREG32(IOADDR_CG_REG_BASE + 0x8000, 0x1);
			}
			printf("Set CPU clk 1600MHz\n");
			break;

		case 470:
			uiReg = readl(IOADDR_CG_REG_BASE + SYSTEM_CLOCK_RATE_OFS);
			if ((uiReg & PLL_CLKSEL_CPUMASK) != PLL_CLKSEL_CPU_ARMPLL) {
				uiReg = readl(IOADDR_CG_REG_BASE);
				if ((uiReg & (1 << PLL_CPU_NO)) != (1 << PLL_CPU_NO)) {
					printf("PLL%d = Disabled => Enable it\n", PLL_CPU_NO);
					uiReg |= (1 << PLL_CPU_NO);
					writel(uiReg, IOADDR_CG_REG_BASE); // B0
					uiReg = readl(IOADDR_CG_REG_BASE);
					//Polling PLL16
					while (!(readl(IOADDR_CG_REG_BASE + 0x04) & (1 << PLL_CPU_NO)));
					printf("PLL%d enable done= 0x%08x 0x%08x\n", PLL_CPU_NO, readl(IOADDR_CG_REG_BASE + 0x00), readl(IOADDR_CG_REG_BASE + 0x04));
				}
				uiReg = readl(IOADDR_CG_REG_BASE + SYSTEM_CLOCK_RATE_OFS);
				uiReg &= ~PLL_CLKSEL_CPUMASK;
				uiReg |= PLL_CLKSEL_CPU_ARMPLL;
				writel(uiReg, (IOADDR_CG_REG_BASE + SYSTEM_CLOCK_RATE_OFS));
				writel(0x34, (IOADDR_CG_REG_BASE + 0xB0));
				while (!(readl(IOADDR_CG_REG_BASE + 0xB0) & (1 << 3)));
				writel(0x28, (IOADDR_CG_REG_BASE + 0xB0));
			}
			//CPU 960 MHz -> CPU-MPLL-960Mhz = 0x0000A0
			//Prepare use normal PLL
			//0xF0028000 bit[0] = 0 //switch to 1x mode
			CLRREG32(IOADDR_CG_REG_BASE + 0x8000 + 0x00, 0x1);
			CLRREG32(IOADDR_CG_REG_BASE + 0x8000 + 0x10, 0x3);
			writel(0x55, IOADDR_CG_REG_BASE + CPU_CLOCK_RATE_RATIO0_OFS); // B0
			writel(0x55, IOADDR_CG_REG_BASE + CPU_CLOCK_RATE_RATIO1_OFS); // B1
			writel(0x4E, IOADDR_CG_REG_BASE + CPU_CLOCK_RATE_RATIO2_OFS); // B2
			printf("Set CPU clk 470MHz\n");
			break;

		default:
			printf("Not define this CPU Freq %d, use defualt value 1200!\n", value);
			uiReg = readl(IOADDR_CG_REG_BASE + SYSTEM_CLOCK_RATE_OFS);
			if ((uiReg & PLL_CLKSEL_CPUMASK) != PLL_CLKSEL_CPU_ARMPLL) {
				uiReg = readl(IOADDR_CG_REG_BASE);
				if ((uiReg & (1 << PLL_CPU_NO)) != (1 << PLL_CPU_NO)) {
					printf("PLL%d = Disabled => Enable it\n", PLL_CPU_NO);
					uiReg |= (1 << PLL_CPU_NO);
					writel(uiReg, IOADDR_CG_REG_BASE); // B0
					uiReg = readl(IOADDR_CG_REG_BASE);
					//Polling PLL16
					while (!(readl(IOADDR_CG_REG_BASE + 0x04) & (1 << PLL_CPU_NO)));
					printf("PLL%d enable done= 0x%08x 0x%08x\n", PLL_CPU_NO, readl(IOADDR_CG_REG_BASE + 0x00), readl(IOADDR_CG_REG_BASE + 0x04));
				}
				uiReg = readl(IOADDR_CG_REG_BASE + SYSTEM_CLOCK_RATE_OFS);
				uiReg &= ~PLL_CLKSEL_CPUMASK;
				uiReg |= PLL_CLKSEL_CPU_ARMPLL;
				writel(uiReg, (IOADDR_CG_REG_BASE + SYSTEM_CLOCK_RATE_OFS));
				writel(0x34, (IOADDR_CG_REG_BASE + 0xB0));
				while (!(readl(IOADDR_CG_REG_BASE + 0xB0) & (1 << 3)));
				writel(0x28, (IOADDR_CG_REG_BASE + 0xB0));
			}
			printf("Not define this CPU Freq %d, use defualt value 1000!\n", value);
			//CPU 1200 MHz -> CPU-MPLL-125Mhz = 0x14D555
			writel(0x00, IOADDR_CG_REG_BASE + CPU_CLOCK_RATE_RATIO0_OFS); // B0
			writel(0x40, IOADDR_CG_REG_BASE + CPU_CLOCK_RATE_RATIO1_OFS); // B1
			writel(0x06, IOADDR_CG_REG_BASE + CPU_CLOCK_RATE_RATIO2_OFS); // B2
			if ((INREG32(IOADDR_CG_REG_BASE + 0x8000) & 0x1) != 0x1) {
				timer2_delay(300);
				//resetb_armpll --> enable PLL & BG LDO ON
				SETREG32(IOADDR_CG_REG_BASE + 0x8000 + 0x10, 0x3);
				timer2_delay(500);
				//select ti ARMPLL
				SETREG32(IOADDR_CG_REG_BASE + 0x8000, 0x1);
			}
			printf("Set CPU clk 1200MHz\n");
			break;
#endif
		}
		break;
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
	"nvt_cpu_freq  470(MPLL)\n"
	"nvt_cpu_freq  480(Fix480)\n"
	"nvt_cpu_freq 1000\n"
	"nvt_cpu_freq 1200(default)\n"
	"nvt_cpu_freq 1400\n"
	"nvt_cpu_freq 1500\n"
	"nvt_cpu_freq 1600\n"
#endif
);

int do_nvt_apb_freq(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
	u32 uiReg;
	u32	uiReg2;
	u32 value = simple_strtoul(argv[1], NULL, 10);

	switch (argc) {
	case 2:
		uiReg = readl(IOADDR_CG_REG_BASE + SYSTEM_CLOCK_RATE_OFS);
		uiReg &= ~(0x100);
		switch (value) {
		case 150:
			uiReg2 = readl(IOADDR_CG_REG_BASE + 0x0);
			if((uiReg2 | (1<<4)) != (1<<4)) {
				uiReg2 |= (1<<4);
				printf("PLL4 = Disabled => Enable it\n");
				writel(uiReg2, IOADDR_CG_REG_BASE + 0x0);
				while((readl(IOADDR_CG_REG_BASE + 0x4) & (1<<4)) != (1<<4)){};
			}
			uiReg |= PLL_CLKSEL_APB150;
			writel(uiReg, IOADDR_CG_REG_BASE + SYSTEM_CLOCK_RATE_OFS);
			break;

		case 120:
			uiReg |= PLL_CLKSEL_APB120;
			writel(uiReg, IOADDR_CG_REG_BASE + SYSTEM_CLOCK_RATE_OFS);
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
	"nvt_apb_freq 120\n"
	"nvt_apb_freq 150\n"
);


int do_nvt_ddr_freq(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
	unsigned int freq_ratio;
	unsigned int freq_ratio0, freq_ratio1, freq_ratio2;

	//0xF0020000 + (0x4000 + 0x400 + 0x20) = 0xF0024400 + 0x20
	freq_ratio0 = readl(IOADDR_CG_REG_BASE + \
						DMA_CLOCK_RATE_RATIO0_OFS);

	//0xF0020000 + (0x4000 + 0x400 + 0x20) = 0xF0024400 + 0x24
	freq_ratio1 = readl(IOADDR_CG_REG_BASE + \
						DMA_CLOCK_RATE_RATIO1_OFS);

	//0xF0020000 + (0x4000 + 0x400 + 0x20) = 0xF0024400 + 0x28
	freq_ratio2 = readl(IOADDR_CG_REG_BASE + \
						DMA_CLOCK_RATE_RATIO2_OFS);
	freq_ratio = freq_ratio0 | (freq_ratio1 << 8) | \
				 (freq_ratio2 << 16);

	freq_ratio = (freq_ratio << 3);
	printf("DMA clock rate = %d => [%d]MHz\r\n", (12 * freq_ratio / 131072), ((12 * freq_ratio / 131072) >> 1));


	if (readl(IOADDR_CG_REG_BASE + 0x70) & 0x800) {
		freq_ratio0 = readl(IOADDR_CG_REG_BASE + \
							DMA2_CLOCK_RATE_RATIO0_OFS);

		//0xF0020000 + (0x4000 + 0x400 + 0x20) = 0xF0024400 + 0x24
		freq_ratio1 = readl(IOADDR_CG_REG_BASE + \
							DMA2_CLOCK_RATE_RATIO1_OFS);

		//0xF0020000 + (0x4000 + 0x400 + 0x20) = 0xF0024400 + 0x28
		freq_ratio2 = readl(IOADDR_CG_REG_BASE + \
							DMA2_CLOCK_RATE_RATIO2_OFS);
		freq_ratio = freq_ratio0 | (freq_ratio1 << 8) | \
					 (freq_ratio2 << 16);

		freq_ratio = (freq_ratio << 3);
		printf("DMA2 clock rate = %d => [%d]MHz\r\n", (12 * freq_ratio / 131072), ((12 * freq_ratio / 131072) >> 1));
	}
	return 0;
}

U_BOOT_CMD(
	nvt_get_ddr_freq, 2,      1,     do_nvt_ddr_freq,
	"get ddr freq/type\n",
	"\n"
);
