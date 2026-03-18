/*
 * Novatek PWM driver on NVT_FT platform.
 *
 * Copyright (C) 2020 Novatek MicroElectronics Corp.
 *
 * SPDX-License-Identifier: GPL-2.0+
 */

// #include <common.h>
#include <div64.h>
#include <pwm.h>
#include <asm/io.h>
#include <linux/libfdt.h>
#include "nvt_pwm.h"
#include <command.h>
#include <config.h>
#include <asm/nvt-common/nvt_common.h>

/* PWM Control Register */
#define PWM_CTRL_REG(id)     ((0x08 * id) + 0x00)
#define PWM_CYCLE_MASK       GENMASK(15, 0)
#define PWM_TYPE_BIT         BIT(16)

/* PWM Period Register */
#define PWM_PRD_REG(id)      ((0x08 * id) + 0x04)
#define PWM_RISE_MASK        GENMASK(7, 0)
#define PWM_FALL_MASK        GENMASK(15, 8)
#define PWM_PERIOD_MASK      GENMASK(23, 16)
#define PWM_INV_BIT          BIT(28)

/* PWM Expand Period Register */
#define PWM_PRD_HI_REG(id)   ((0x04 * id) + 0x230)
#define PWM_RISE_HI_MASK     GENMASK(7, 0)
#define PWM_FALL_HI_MASK     GENMASK(15, 8)
#define PWM_PERIOD_HI_MASK   GENMASK(23, 16)

/* PWM Enable Register */
#define PWM_EN_REG           0x100

/* PWM Disable Register */
#define PWM_DIS_REG          0x104

#define SRC_FREQ             120000000
#define PCLK_FREQ            250000

u32 pclk_freq = PCLK_FREQ;

static inline void nvt_pwm_write_reg(int reg, u32 val)
{
	writel(val, IOADDR_PWM_REG_BASE + reg);
}

static inline u32 nvt_pwm_read_reg(int reg)
{
	return readl(IOADDR_PWM_REG_BASE + reg);
}

int pwm_init(int pwm_id, int div, int invert)
{
	u32 prd_reg = 0x0;

	/* Enable clk */
	pwm_enable_clk(pwm_id);

	/* Enable pinmux depends on dts */
	do_nvt_pwm_set_pinmux(pwm_id);

	/* Disable pwm before inverting */
	nvt_pwm_write_reg(PWM_DIS_REG, (u32)(0x1 << pwm_id));

	/* Invert pwm */
	prd_reg = nvt_pwm_read_reg(PWM_PRD_REG(pwm_id));

	if (invert == 1) {
		prd_reg |= PWM_INV_BIT;
	} else {
		prd_reg &= ~PWM_INV_BIT;
	}

	nvt_pwm_write_reg(PWM_PRD_REG(pwm_id), prd_reg);

	return 0;
}

void nvt_pwm_set_freq(int pwm_id, u32 freq)
{
	u32 div;

	if (!freq || (freq > SRC_FREQ)) {
		DBG_ERR("pwm_id(%d) Invalid pwm frequency\r\n", pwm_id);
		return;
	}

	div = SRC_FREQ / freq;
	div = div - 1;
	pclk_freq = freq;

	printf("pclk_freq(%u) div(%u)\n", freq, div);

	nvt_pwm_set_div(pwm_id, div);
}

int pwm_config(int pwm_id, int duty_ns, int period_ns)
{
	u32 prd_reg = 0x0;
	u32 prd_hi_reg = 0x0;
	u64 period = 0, duty = 0;

	/* Set free run mode */
	nvt_pwm_write_reg(PWM_CTRL_REG(pwm_id), (u32)0x0);

	period = (u64)pclk_freq * (u64)period_ns;
	do_div(period, 1000000000);

	duty = (u64)pclk_freq * (u64)duty_ns;
	do_div(duty, 1000000000);

	prd_reg = nvt_pwm_read_reg(PWM_PRD_REG(pwm_id));

	prd_reg &= ~PWM_RISE_MASK;  /* Output is high since started */

	prd_reg &= ~PWM_FALL_MASK;
	prd_reg |= (PWM_FALL_MASK & (duty << 8));

	prd_reg &= ~PWM_PERIOD_MASK;
	prd_reg |= (PWM_PERIOD_MASK & (period << 16));

	nvt_pwm_write_reg(PWM_PRD_REG(pwm_id), prd_reg);

	if (pwm_id < 8) {
		prd_hi_reg &= ~PWM_RISE_HI_MASK;  /* Output is high since started */

		prd_hi_reg &= ~PWM_FALL_HI_MASK;
		prd_hi_reg |= (PWM_FALL_HI_MASK & ((duty >> 8) << 8));

		prd_hi_reg &= ~PWM_PERIOD_HI_MASK;
		prd_hi_reg |= (PWM_PERIOD_HI_MASK & ((period >> 8) << 16));

		nvt_pwm_write_reg(PWM_PRD_HI_REG(pwm_id), prd_hi_reg);
	}

	return 0;
}

int pwm_enable(int pwm_id)
{
	nvt_pwm_write_reg(PWM_EN_REG, (u32)(0x1 << pwm_id));

	//TODO: wait_for_completion
	return 0;
}

void pwm_disable(int pwm_id)
{
	nvt_pwm_write_reg(PWM_DIS_REG, (u32)(0x1 << pwm_id));

	//TODO: wait_for_completion
}

static int do_pwm(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
	int id, inv = 0;
	u32 period_ns = 0x0, duty_ns = 0x0;
	u32 freq = 0x0;
	u32 pinmux = 0x0;

	if (!strncmp(argv[1], "config", 6)) {
		if (argc < 6) {
			return CMD_RET_USAGE;
		}

		id = simple_strtoul(argv[2], NULL, 10);

		inv = simple_strtoul(argv[3], NULL, 10);

		period_ns = simple_strtoul(argv[4], NULL, 10);

		duty_ns = simple_strtoul(argv[5], NULL, 10);

		if ((!pwmid_is_valid(id)) || (inv > 1) || (duty_ns > period_ns)) {
			return CMD_RET_USAGE;
		}

		pwm_init(id, 0, inv);

		nvt_pwm_set_freq(id, SRC_FREQ/4);
		pwm_config(id, duty_ns, period_ns);

		return 0;
	} else if (!strncmp(argv[1], "enable", 6)) {
		if (argc < 3) {
			return CMD_RET_USAGE;
		}

		id = simple_strtoul(argv[2], NULL, 10);
		if (!pwmid_is_valid(id)) {
			return CMD_RET_USAGE;
		}

		pwm_enable(id);

		return 0;
	} else if (!strncmp(argv[1], "disable", 7)) {
		if (argc < 3) {
			return CMD_RET_USAGE;
		}

		id = simple_strtoul(argv[2], NULL, 10);
		if (!pwmid_is_valid(id)) {
			return CMD_RET_USAGE;
		}

		pwm_disable(id);

		return 0;
	} else if (!strncmp(argv[1], "freq", 4)) {
		if (argc < 4) {
			return CMD_RET_USAGE;
		}

		id = simple_strtoul(argv[2], NULL, 10);
		if (!pwmid_is_valid(id)) {
			return CMD_RET_USAGE;
		}

		freq = simple_strtoul(argv[3], NULL, 10);

		nvt_pwm_set_freq(id, freq);

		return 0;
	} else if (!strncmp(argv[1], "pinmux", 6)) {
		if (argc < 4) {
			return CMD_RET_USAGE;
		}

		id = simple_strtoul(argv[2], NULL, 10);
		if (!pwmid_is_valid(id)) {
			return CMD_RET_USAGE;
		}

		pinmux = simple_strtoul(argv[3], NULL, 10);

		nvt_pwm_set_pinmux(id, pinmux);

		return 0;
	} else {
		return CMD_RET_USAGE;
	}
}

#if defined(CONFIG_TARGET_NA51090_A64) || defined(CONFIG_TARGET_NA51102_A64) || defined(CONFIG_TARGET_NA51103) || defined(CONFIG_TARGET_NS02201_A64) || defined(CONFIG_TARGET_NS02302_A64)
void pwm_set(int pwm_id, int level)
{
	u32 duty = 0;

	if (level <= 100) {
		pwm_init(pwm_id, 0, 0);

		duty = 84 * level;

		nvt_pwm_set_freq(pwm_id, SRC_FREQ/4);
		pwm_config(pwm_id, duty, 8400);

		pwm_enable(pwm_id);
	}
}

static int get_pwr_id(void)
{
	/* Enable it after dts parsing ready*/
	int nodeoffset;
	u32 *cell = NULL;
	char path[20] = {0};
	int pwm_id = -1;

	sprintf(path,"/pwr");

	nodeoffset = fdt_path_offset((const void*)nvt_fdt_buffer, path);

	cell = (u32*)fdt_getprop((const void*)nvt_fdt_buffer, nodeoffset, "pwm_id", NULL);

	if (cell == NULL) {
		printf("%s: get null pwr id\n", __func__);
	} else {
		pwm_id = __be32_to_cpu(cell[0]);
	}

	return pwm_id;
}

static int do_power_ctl(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{

	u32 level = 0x0;
	static u32 chip_id;
	int pwr_id = 0;
	int id = 0;

#ifdef CONFIG_TARGET_NA51090_A64
	pwr_id = 0;
#elif defined CONFIG_TARGET_NA51102_A64
	pwr_id = 9;
#elif defined CONFIG_TARGET_NA51103
	pwr_id = 1;
#elif defined CONFIG_TARGET_NS02201_A64
	pwr_id = 12;
#elif defined CONFIG_TARGET_NS02302_A64
	pwr_id = 10;
#endif

	chip_id = nvt_get_chip_id();
	if (chip_id == CHIP_NS02402) {
		int nodeoffset;
		char path[20] = {0};

		sprintf(path,"/pwr");
		nodeoffset = fdt_path_offset((const void*)nvt_fdt_buffer, path);
		if (nodeoffset < 0) {
			return 0;
		}
	}

	if (argc == 2) {
		level = simple_strtoul(argv[1], NULL, 10);

		id = get_pwr_id();

		if(!pwmid_is_valid(id)) {
			printf("use default pwm\r\n");
		} else {
			pwr_id = id;
		}

		printf("pwr idx: %d\r\n", pwr_id);
		pwm_set(pwr_id, level);

		return 0;
	}

	return CMD_RET_USAGE;
}

U_BOOT_CMD(
	power_ctl,  2,  0,   do_power_ctl,
	"nvt core power control",
	"[level]\n"
);
#endif

U_BOOT_CMD(
	pwm,  6,  0,   do_pwm,
	"nvt pwm operation",
	"config [id] [invert] [period_ns] [duty_ns]\n"
	"pwm enable [id]\n"
	"pwm disable [id]\n"
	"pwm freq [id] [freq(Hz)]\n"
	"Note:\n"
	"Default freq is 30MHz,\n"
	"therefore each unit of [period_ns] and [duty_ns] should be 34 ns,\n"
	"for channel 0 to 7 the maximum unit is 65535, so legal range of [period_ns] is 67 to 2184500 ns,\n"
	"for other channels the maximum unit is 255, so legal range of [period_ns] is 67 to 8500 ns.\n"
);
