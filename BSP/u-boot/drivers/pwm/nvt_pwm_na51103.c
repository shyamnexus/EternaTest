#include "nvt_pwm.h"

#define NUM_PWM                 3
#define PIN_PWM_CFG_PWM0      0x0001          ///< Enable PWM0 @P_GPIO24[PWM0]
#define PIN_PWM_CFG_PWM1      0x0010          ///< Enable PWM1 @P_GPIO25[PWM1]
#define PIN_PWM_CFG_PWM2      0x0100          ///< Enable PWM2 @P_GPIO19[PWM2]

void nvt_pwm_set_pinmux(int pwm_id, u32 pwm_pinmux)
{
	printf("pwm_id(%d) pwm_pinmux(0x%x)\n", pwm_id, pwm_pinmux);

	if (pwm_id == 0) {
		if (pwm_pinmux & PIN_PWM_CFG_PWM0) {
			*(u32 *)(IOADDR_TOP_REG_BASE + 0x18) &= ~(0x1 << (pwm_id * 2));
			*(u32 *)(IOADDR_TOP_REG_BASE + 0x18) |= (0x1 << (pwm_id * 2));
			*(u32 *)(IOADDR_TOP_REG_BASE + 0xA8) &= ~(0x1 << 24);
		} else {			
			DBG_ERR("!!Caution!!>>using pwm_id(%d) but pwm_pinmux(0x%x) not configured(check nvt-top.dtsi)\n", pwm_id, pwm_pinmux);
		}
	} else if (pwm_id == 1) {
		if (pwm_pinmux & PIN_PWM_CFG_PWM1) {
			*(u32 *)(IOADDR_TOP_REG_BASE + 0x18) &= ~(0x1 << (pwm_id * 2));
			*(u32 *)(IOADDR_TOP_REG_BASE + 0x18) |= (0x1 << (pwm_id * 2));
			*(u32 *)(IOADDR_TOP_REG_BASE + 0xA8) &= ~(0x1 << 25);
		} else {
			DBG_ERR("!!Caution!!>>using pwm_id(%d) but pwm_pinmux(0x%x) not configured(check nvt-top.dtsi)\n", pwm_id, pwm_pinmux);
		}
	} else if (pwm_id == 2) {
		if (pwm_pinmux & PIN_PWM_CFG_PWM2) {
			*(u32 *)(IOADDR_TOP_REG_BASE + 0x18) &= ~(0x1 << (pwm_id * 2));
			*(u32 *)(IOADDR_TOP_REG_BASE + 0x18) |= (0x1 << (pwm_id * 2));
			*(u32 *)(IOADDR_TOP_REG_BASE + 0xA8) &= ~(0x1 << 19);
		} else {
			DBG_ERR("!!Caution!!>>using pwm_id(%d) but pwm_pinmux(0x%x) not configured(check nvt-top.dtsi)\n", pwm_id, pwm_pinmux);
		}
	}
}

int do_nvt_pwm_set_pinmux(int pwm_id)
{
	char pwm[10] = {0};
	char path[20] = {0};
	ulong fdt_addr = nvt_readl((ulong)nvt_shminfo_boot_fdt_addr);
	int nodeoffset, len;
    u32 pwm_pinmux = 0;
	u32 *cell = NULL;

	if ((pwm_id < 4) || (pwm_id == 8) || (pwm_id == 9)) {
		sprintf(pwm, "pwm");
	} else {
		sprintf(pwm, "pwm2");
	}

    sprintf(path, "/top@%x/%s", IOADDR_TOP_REG_BASE, pwm);

	nodeoffset = fdt_path_offset((const void *)fdt_addr, path);
	if (nodeoffset < 0) {
		printf("%s(%d) nodeoffset < 0\n", __func__, __LINE__);
		return -1;
	}

	cell = (u32 *)fdt_getprop((const void *)fdt_addr, nodeoffset, "pinmux", &len);
	if (cell == NULL) {
		printf("%s(%d) cell = NULL\n", __func__, __LINE__);
		return -1;
	}

	if (len == 0) {
		printf("%s(%d) len = 0\n", __func__, __LINE__);
		return -1;
	}

	pwm_pinmux = __be32_to_cpu(cell[0]);
	printf("%s_pinmux = 0x%x\n", pwm, pwm_pinmux);
	nvt_pwm_set_pinmux(pwm_id, pwm_pinmux);

	return 0;
}

void nvt_pwm_set_div(int pwm_id, u32 div)
{
	if (pwm_id == 0) {
		*(u32 *)(IOADDR_CG_REG_BASE + 0x34) &= ~(0x3FFF);
		*(u32 *)(IOADDR_CG_REG_BASE + 0x34) |= (div & 0x3FFF);
	} else if (pwm_id == 1) {
		*(u32 *)(IOADDR_CG_REG_BASE + 0x34) &= ~(0x3FFF << 16);
		*(u32 *)(IOADDR_CG_REG_BASE + 0x34) |= ((div & 0x3FFF) << 16);
	} else if (pwm_id == 2) {
		*(u32 *)(IOADDR_CG_REG_BASE + 0x38) &= ~(0x3FFF);
		*(u32 *)(IOADDR_CG_REG_BASE + 0x38) |= (div & 0x3FFF);
	}
}

void pwm_enable_clk(int pwm_id)
{
	/* Enable clk */
	*(u32 *)(IOADDR_CG_REG_BASE + 0x7C) |= (0x1 << pwm_id);
	*(u32 *)(IOADDR_CG_REG_BASE + 0x9C) |= (0x1 << 9);
}

bool pwmid_is_valid(int pwm_id)
{
    if (pwm_id < 0 || pwm_id >= NUM_PWM) {
        return false;
    }

    return true;
}