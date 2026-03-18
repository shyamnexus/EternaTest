#include "nvt_pwm.h"

#define NUM_PWM                 13
#define	PIN_PWM_CFG_PWM0_1     0x1         ///< PWM0_1  (P_GPIO[0])
#define	PIN_PWM_CFG_PWM0_2     0x2         ///< PWM0_2  (DSI_GPIO[6])
#define	PIN_PWM_CFG_PWM1_1     0x10        ///< PWM1_1  (P_GPIO[1])
#define	PIN_PWM_CFG_PWM1_2     0x20        ///< PWM1_2  (DSI_GPIO[7])
#define	PIN_PWM_CFG_PWM2_1     0x100       ///< PWM2_1  (P_GPIO[2])
#define	PIN_PWM_CFG_PWM2_2     0x200       ///< PWM2_2  (DSI_GPIO[8])
#define	PIN_PWM_CFG_PWM3_1     0x1000      ///< PWM3_1  (P_GPIO[3])
#define	PIN_PWM_CFG_PWM3_2     0x2000      ///< PWM3_2  (DSI_GPIO[9])
#define	PIN_PWM_CFG_PWM4_1     0x10000     ///< PWM4_1  (P_GPIO[4])
#define	PIN_PWM_CFG_PWM4_2     0x20000     ///< PWM4_2  (D_GPIO[2])
#define	PIN_PWM_CFG_PWM5_1     0x100000    ///< PWM5_1  (P_GPIO[5])
#define	PIN_PWM_CFG_PWM5_2     0x200000    ///< PWM5_2  (D_GPIO[3])

#define PIN_PWMII_CFG_PWM6_1   0x1         ///< PWM6_1  (P_GPIO[6])
#define PIN_PWMII_CFG_PWM6_2   0x2         ///< PWM6_2  (D_GPIO[4])
#define PIN_PWMII_CFG_PWM7_1   0x10        ///< PWM7_1  (P_GPIO[7])
#define PIN_PWMII_CFG_PWM7_2   0x20        ///< PWM7_2  (D_GPIO[5])
#define PIN_PWMII_CFG_PWM8_1   0x100       ///< PWM8_1  (P_GPIO[8])
#define PIN_PWMII_CFG_PWM8_2   0x200       ///< PWM8_2  (C_GPIO[4])
#define PIN_PWMII_CFG_PWM8_3   0x400
#define PIN_PWMII_CFG_PWM9_1   0x1000      ///< PWM9_1  (P_GPIO[9])
#define PIN_PWMII_CFG_PWM9_2   0x2000      ///< PWM9_2  (C_GPIO[5])
#define PIN_PWMII_CFG_PWM9_3   0x4000
#define PIN_PWMII_CFG_PWM10_1  0x10000     ///< PWM10_1 (P_GPIO[10])
#define PIN_PWMII_CFG_PWM10_2  0x20000     ///< PWM10_2 (C_GPIO[6])
#define PIN_PWMII_CFG_PWM10_3  0x40000
#define PIN_PWMII_CFG_PWM11_1  0x100000    ///< PWM11_1 (P_GPIO[11])
#define PIN_PWMII_CFG_PWM11_2  0x200000    ///< PWM11_2 (C_GPIO[7])
#define PIN_PWMII_CFG_PWM11_3  0x400000

#define PIN_PWMII_CFG_PWM12_1  0x1000000

void nvt_pwm_set_pinmux(int pwm_id, u32 pwm_pinmux)
{
	printf("pwm_id(%d) pwm_pinmux(0x%x)\n", pwm_id, pwm_pinmux);

	if (pwm_id == 0) {
		if (pwm_pinmux & PIN_PWM_CFG_PWM0_1) {
			*(u32 *)(IOADDR_TOP_REG_BASE + 0x18) &= ~(0x7 << (pwm_id * 4));
			*(u32 *)(IOADDR_TOP_REG_BASE + 0x18) |= (0x1 << (pwm_id * 4));
			*(u32 *)(IOADDR_TOP_REG_BASE + 0xA8) &= ~(0x1 << pwm_id);
		} else if (pwm_pinmux & PIN_PWM_CFG_PWM0_2) {
			*(u32 *)(IOADDR_TOP_REG_BASE + 0x18) &= ~(0x7 << (pwm_id * 4));
			*(u32 *)(IOADDR_TOP_REG_BASE + 0x18) |= (0x2 << (pwm_id * 4));
			*(u32 *)(IOADDR_TOP_REG_BASE + 0xD8) &= ~(0x1 << 6);
		} else {
			*(u32 *)(IOADDR_TOP_REG_BASE + 0x18) &= ~(0x7 << (pwm_id * 4));
            DBG_ERR("!!Caution!!>>using pwm_id(%d) but pwm_pinmux(0x%x) not configured(check nvt-top.dtsi)\n", pwm_id, pwm_pinmux);
		}
	} else if (pwm_id == 1) {
		if (pwm_pinmux & PIN_PWM_CFG_PWM1_1) {
			*(u32 *)(IOADDR_TOP_REG_BASE + 0x18) &= ~(0x7 << (pwm_id * 4));
			*(u32 *)(IOADDR_TOP_REG_BASE + 0x18) |= (0x1 << (pwm_id * 4));
			*(u32 *)(IOADDR_TOP_REG_BASE + 0xA8) &= ~(0x1 << pwm_id);
		} else if (pwm_pinmux & PIN_PWM_CFG_PWM1_2) {
			*(u32 *)(IOADDR_TOP_REG_BASE + 0x18) &= ~(0x7 << (pwm_id * 4));
			*(u32 *)(IOADDR_TOP_REG_BASE + 0x18) |= (0x2 << (pwm_id * 4));
			*(u32 *)(IOADDR_TOP_REG_BASE + 0xD8) &= ~(0x1 << 7);
		} else {
			*(u32 *)(IOADDR_TOP_REG_BASE + 0x18) &= ~(0x7 << (pwm_id * 4));
			DBG_ERR("!!Caution!!>>using pwm_id(%d) but pwm_pinmux(0x%x) not configured(check nvt-top.dtsi)\n", pwm_id, pwm_pinmux);
		}
	} else if (pwm_id == 2) {
		if (pwm_pinmux & PIN_PWM_CFG_PWM2_1) {
			*(u32 *)(IOADDR_TOP_REG_BASE + 0x18) &= ~(0x7 << (pwm_id * 4));
			*(u32 *)(IOADDR_TOP_REG_BASE + 0x18) |= (0x1 << (pwm_id * 4));
			*(u32 *)(IOADDR_TOP_REG_BASE + 0xA8) &= ~(0x1 << pwm_id);
		} else if (pwm_pinmux & PIN_PWM_CFG_PWM2_2) {
			*(u32 *)(IOADDR_TOP_REG_BASE + 0x18) &= ~(0x7 << (pwm_id * 4));
			*(u32 *)(IOADDR_TOP_REG_BASE + 0x18) |= (0x2 << (pwm_id * 4));
			*(u32 *)(IOADDR_TOP_REG_BASE + 0xD8) &= ~(0x1 << 8);
		} else {
			*(u32 *)(IOADDR_TOP_REG_BASE + 0x18) &= ~(0x7 << (pwm_id * 4));
			DBG_ERR("!!Caution!!>>using pwm_id(%d) but pwm_pinmux(0x%x) not configured(check nvt-top.dtsi)\n", pwm_id, pwm_pinmux);
		}
	} else if (pwm_id == 3) {
		if (pwm_pinmux & PIN_PWM_CFG_PWM3_1) {
			*(u32 *)(IOADDR_TOP_REG_BASE + 0x18) &= ~(0x7 << (pwm_id * 4));
			*(u32 *)(IOADDR_TOP_REG_BASE + 0x18) |= (0x1 << (pwm_id * 4));
			*(u32 *)(IOADDR_TOP_REG_BASE + 0xA8) &= ~(0x1 << pwm_id);
		} else if (pwm_pinmux & PIN_PWM_CFG_PWM3_2) {
			*(u32 *)(IOADDR_TOP_REG_BASE + 0x18) &= ~(0x7 << (pwm_id * 4));
			*(u32 *)(IOADDR_TOP_REG_BASE + 0x18) |= (0x2 << (pwm_id * 4));
			*(u32 *)(IOADDR_TOP_REG_BASE + 0xD8) &= ~(0x1 << 9);
		} else {
			*(u32 *)(IOADDR_TOP_REG_BASE + 0x18) &= ~(0x7 << (pwm_id * 4));
			DBG_ERR("!!Caution!!>>using pwm_id(%d) but pwm_pinmux(0x%x) not configured(check nvt-top.dtsi)\n", pwm_id, pwm_pinmux);
		}
	} else if (pwm_id == 4) {
		if (pwm_pinmux & PIN_PWM_CFG_PWM4_1) {
			*(u32 *)(IOADDR_TOP_REG_BASE + 0x18) &= ~(0x7 << (pwm_id * 4));
			*(u32 *)(IOADDR_TOP_REG_BASE + 0x18) |= (0x1 << (pwm_id * 4));
			*(u32 *)(IOADDR_TOP_REG_BASE + 0xA8) &= ~(0x1 << pwm_id);
		} else if (pwm_pinmux & PIN_PWM_CFG_PWM4_2) {
			*(u32 *)(IOADDR_TOP_REG_BASE + 0x18) &= ~(0x7 << (pwm_id * 4));
			*(u32 *)(IOADDR_TOP_REG_BASE + 0x18) |= (0x2 << (pwm_id * 4));
			*(u32 *)(IOADDR_TOP_REG_BASE + 0xB4) &= ~(0x1 << 2);
		} else {
			*(u32 *)(IOADDR_TOP_REG_BASE + 0x18) &= ~(0x7 << (pwm_id * 4));
			DBG_ERR("!!Caution!!>>using pwm_id(%d) but pwm_pinmux(0x%x) not configured(check nvt-top.dtsi)\n", pwm_id, pwm_pinmux);
		}
	} else if (pwm_id == 5) {
		if (pwm_pinmux & PIN_PWM_CFG_PWM5_1) {
			*(u32 *)(IOADDR_TOP_REG_BASE + 0x18) &= ~(0x7 << (pwm_id * 4));
			*(u32 *)(IOADDR_TOP_REG_BASE + 0x18) |= (0x1 << (pwm_id * 4));
			*(u32 *)(IOADDR_TOP_REG_BASE + 0xA8) &= ~(0x1 << pwm_id);
		} else if (pwm_pinmux & PIN_PWM_CFG_PWM5_2) {
			*(u32 *)(IOADDR_TOP_REG_BASE + 0x18) &= ~(0x7 << (pwm_id * 4));
			*(u32 *)(IOADDR_TOP_REG_BASE + 0x18) |= (0x2 << (pwm_id * 4));
			*(u32 *)(IOADDR_TOP_REG_BASE + 0xB4) &= ~(0x1 << 3);
		} else {
			*(u32 *)(IOADDR_TOP_REG_BASE + 0x18) &= ~(0x7 << (pwm_id * 4));
			DBG_ERR("!!Caution!!>>using pwm_id(%d) but pwm_pinmux(0x%x) not configured(check nvt-top.dtsi)\n", pwm_id, pwm_pinmux);
		}
	} else if (pwm_id == 6) {
		if (pwm_pinmux & PIN_PWMII_CFG_PWM6_1) {
			*(u32 *)(IOADDR_TOP_REG_BASE + 0x18) &= ~(0x7 << (pwm_id * 4));
			*(u32 *)(IOADDR_TOP_REG_BASE + 0x18) |= (0x1 << (pwm_id * 4));
			*(u32 *)(IOADDR_TOP_REG_BASE + 0xA8) &= ~(0x1 << pwm_id);
		} else if (pwm_pinmux & PIN_PWMII_CFG_PWM6_2) {
			*(u32 *)(IOADDR_TOP_REG_BASE + 0x18) &= ~(0x7 << (pwm_id * 4));
			*(u32 *)(IOADDR_TOP_REG_BASE + 0x18) |= (0x2 << (pwm_id * 4));
			*(u32 *)(IOADDR_TOP_REG_BASE + 0xB4) &= ~(0x1 << 4);
		} else {
			*(u32 *)(IOADDR_TOP_REG_BASE + 0x18) &= ~(0x7 << (pwm_id * 4));
			DBG_ERR("!!Caution!!>>using pwm_id(%d) but pwm_pinmux(0x%x) not configured(check nvt-top.dtsi)\n", pwm_id, pwm_pinmux);
		}
	} else if (pwm_id == 7) {
		if (pwm_pinmux & PIN_PWMII_CFG_PWM7_1) {
			*(u32 *)(IOADDR_TOP_REG_BASE + 0x18) &= ~(0x7 << (pwm_id * 4));
			*(u32 *)(IOADDR_TOP_REG_BASE + 0x18) |= (0x1 << (pwm_id * 4));
			*(u32 *)(IOADDR_TOP_REG_BASE + 0xA8) &= ~(0x1 << pwm_id);
		} else if (pwm_pinmux & PIN_PWMII_CFG_PWM7_2) {
			*(u32 *)(IOADDR_TOP_REG_BASE + 0x18) &= ~(0x7 << (pwm_id * 4));
			*(u32 *)(IOADDR_TOP_REG_BASE + 0x18) |= (0x2 << (pwm_id * 4));
			*(u32 *)(IOADDR_TOP_REG_BASE + 0xB4) &= ~(0x1 << 5);
		} else {
			*(u32 *)(IOADDR_TOP_REG_BASE + 0x18) &= ~(0x7 << (pwm_id * 4));
			DBG_ERR("!!Caution!!>>using pwm_id(%d) but pwm_pinmux(0x%x) not configured(check nvt-top.dtsi)\n", pwm_id, pwm_pinmux);
		}
	} else if (pwm_id == 8) {
		if (pwm_pinmux & PIN_PWMII_CFG_PWM8_1) {
			*(u32 *)(IOADDR_TOP_REG_BASE + 0x1C) &= ~(0xF << ((pwm_id - 8) * 4));
			*(u32 *)(IOADDR_TOP_REG_BASE + 0x1C) |= (0x1 << ((pwm_id - 8) * 4));
			*(u32 *)(IOADDR_TOP_REG_BASE + 0xA8) &= ~(0x1 << pwm_id);
		} else if (pwm_pinmux & PIN_PWMII_CFG_PWM8_2) {
			*(u32 *)(IOADDR_TOP_REG_BASE + 0x1C) &= ~(0xF << ((pwm_id - 8) * 4));
			*(u32 *)(IOADDR_TOP_REG_BASE + 0x1C) |= (0x2 << ((pwm_id - 8) * 4));
			*(u32 *)(IOADDR_TOP_REG_BASE + 0xA0) &= ~(0x1 << 4);
		} else if (pwm_pinmux & PIN_PWMII_CFG_PWM8_3) {
			*(u32 *)(IOADDR_TOP_REG_BASE + 0x1C) &= ~(0xF << ((pwm_id - 8) * 4));
			*(u32 *)(IOADDR_TOP_REG_BASE + 0x1C) |= (0x3 << ((pwm_id - 8) * 4));
			*(u32 *)(IOADDR_TOP_REG_BASE + 0xA0) &= ~(0x1 << 19);
		} else {
			*(u32 *)(IOADDR_TOP_REG_BASE + 0x18) &= ~(0xF << ((pwm_id - 8) * 4));
			DBG_ERR("!!Caution!!>>using pwm_id(%d) but pwm_pinmux(0x%x) not configured(check nvt-top.dtsi)\n", pwm_id, pwm_pinmux);
		}
	} else if (pwm_id == 9) {
		if (pwm_pinmux & PIN_PWMII_CFG_PWM9_1) {
			*(u32 *)(IOADDR_TOP_REG_BASE + 0x1C) &= ~(0xF << ((pwm_id - 8) * 4));
			*(u32 *)(IOADDR_TOP_REG_BASE + 0x1C) |= (0x1 << ((pwm_id - 8) * 4));
			*(u32 *)(IOADDR_TOP_REG_BASE + 0xA8) &= ~(0x1 << pwm_id);
		} else if (pwm_pinmux & PIN_PWMII_CFG_PWM9_2) {
			*(u32 *)(IOADDR_TOP_REG_BASE + 0x1C) &= ~(0xF << ((pwm_id - 8) * 4));
			*(u32 *)(IOADDR_TOP_REG_BASE + 0x1C) |= (0x2 << ((pwm_id - 8) * 4));
			*(u32 *)(IOADDR_TOP_REG_BASE + 0xA0) &= ~(0x1 << 5);
		} else if (pwm_pinmux & PIN_PWMII_CFG_PWM9_3) {
			*(u32 *)(IOADDR_TOP_REG_BASE + 0x1C) &= ~(0xF << ((pwm_id - 8) * 4));
			*(u32 *)(IOADDR_TOP_REG_BASE + 0x1C) |= (0x3 << ((pwm_id - 8) * 4));
			*(u32 *)(IOADDR_TOP_REG_BASE + 0xA0) &= ~(0x1 << 20);
		} else {
			*(u32 *)(IOADDR_TOP_REG_BASE + 0x18) &= ~(0xF << ((pwm_id - 8) * 4));
			DBG_ERR("!!Caution!!>>using pwm_id(%d) but pwm_pinmux(0x%x) not configured(check nvt-top.dtsi)\n", pwm_id, pwm_pinmux);
		}
	} else if (pwm_id == 10) {
		if (pwm_pinmux & PIN_PWMII_CFG_PWM10_1) {
			*(u32 *)(IOADDR_TOP_REG_BASE + 0x1C) &= ~(0xF << ((pwm_id - 8) * 4));
			*(u32 *)(IOADDR_TOP_REG_BASE + 0x1C) |= (0x1 << ((pwm_id - 8) * 4));
			*(u32 *)(IOADDR_TOP_REG_BASE + 0xA8) &= ~(0x1 << pwm_id);
		} else if (pwm_pinmux & PIN_PWMII_CFG_PWM10_2) {
			*(u32 *)(IOADDR_TOP_REG_BASE + 0x1C) &= ~(0xF << ((pwm_id - 8) * 4));
			*(u32 *)(IOADDR_TOP_REG_BASE + 0x1C) |= (0x2 << ((pwm_id - 8) * 4));
			*(u32 *)(IOADDR_TOP_REG_BASE + 0xA0) &= ~(0x1 << 6);
		} else if (pwm_pinmux & PIN_PWMII_CFG_PWM10_3) {
			*(u32 *)(IOADDR_TOP_REG_BASE + 0x1C) &= ~(0xF << ((pwm_id - 8) * 4));
			*(u32 *)(IOADDR_TOP_REG_BASE + 0x1C) |= (0x3 << ((pwm_id - 8) * 4));
			*(u32 *)(IOADDR_TOP_REG_BASE + 0xA0) &= ~(0x1 << 21);
		} else {
			*(u32 *)(IOADDR_TOP_REG_BASE + 0x18) &= ~(0xF << ((pwm_id - 8) * 4));
			DBG_ERR("!!Caution!!>>using pwm_id(%d) but pwm_pinmux(0x%x) not configured(check nvt-top.dtsi)\n", pwm_id, pwm_pinmux);
		}
	} else if (pwm_id == 11) {
		if (pwm_pinmux & PIN_PWMII_CFG_PWM11_1) {
			*(u32 *)(IOADDR_TOP_REG_BASE + 0x1C) &= ~(0xF << ((pwm_id - 8) * 4));
			*(u32 *)(IOADDR_TOP_REG_BASE + 0x1C) |= (0x1 << ((pwm_id - 8) * 4));
			*(u32 *)(IOADDR_TOP_REG_BASE + 0xA8) &= ~(0x1 << pwm_id);
		} else if (pwm_pinmux & PIN_PWMII_CFG_PWM11_2) {
			*(u32 *)(IOADDR_TOP_REG_BASE + 0x1C) &= ~(0xF << ((pwm_id - 8) * 4));
			*(u32 *)(IOADDR_TOP_REG_BASE + 0x1C) |= (0x2 << ((pwm_id - 8) * 4));
			*(u32 *)(IOADDR_TOP_REG_BASE + 0xA0) &= ~(0x1 << 7);
		} else if (pwm_pinmux & PIN_PWMII_CFG_PWM11_3) {
			*(u32 *)(IOADDR_TOP_REG_BASE + 0x1C) &= ~(0xF << ((pwm_id - 8) * 4));
			*(u32 *)(IOADDR_TOP_REG_BASE + 0x1C) |= (0x3 << ((pwm_id - 8) * 4));
			*(u32 *)(IOADDR_TOP_REG_BASE + 0xA0) &= ~(0x1 << 22);
		} else {
			*(u32 *)(IOADDR_TOP_REG_BASE + 0x18) &= ~(0xF << ((pwm_id - 8) * 4));
			DBG_ERR("!!Caution!!>>using pwm_id(%d) but pwm_pinmux(0x%x) not configured(check nvt-top.dtsi)\n", pwm_id, pwm_pinmux);
		}
	} else if (pwm_id == 12) {
		if (pwm_pinmux & PIN_PWMII_CFG_PWM12_1) {
			*(u32 *)(IOADDR_TOP_REG_BASE + 0x1C) &= ~(0xF << ((pwm_id - 8) * 4));
			*(u32 *)(IOADDR_TOP_REG_BASE + 0x1C) |= (0x1 << ((pwm_id - 8) * 4));
			*(u32 *)(IOADDR_TOP_REG_BASE + 0xAC) &= ~(0x1 << 10);
		} else {
			*(u32 *)(IOADDR_TOP_REG_BASE + 0x18) &= ~(0xF << ((pwm_id - 8) * 4));
			DBG_ERR("!!Caution!!>>using pwm_id(%d) but pwm_pinmux(0x%x) not configured(check nvt-top.dtsi)\n", pwm_id, pwm_pinmux);
		}
	}
}

int do_nvt_pwm_set_pinmux(int pwm_id)
{
	char pwm[10] = {0};
	char path[20] = {0};
	int nodeoffset, len;
    u32 pwm_pinmux = 0;
	u32 *cell = NULL;

	if (pwm_id < 6) {
		sprintf(pwm, "pwm");
	} else {
		sprintf(pwm, "pwmII");
	}

    sprintf(path, "/top@2,f0010000/%s", pwm);

	nodeoffset = fdt_path_offset((const void *)nvt_fdt_buffer, path);
	if (nodeoffset < 0) {
		printf("%s(%d) nodeoffset < 0\n", __func__, __LINE__);
		return -1;
	}

	cell = (u32 *)fdt_getprop((const void *)nvt_fdt_buffer, nodeoffset, "pinmux", &len);
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
	if (pwm_id < 4) {
		*(u32 *)(IOADDR_CG_REG_BASE + 0x30) &= ~(0x3FFF);
		*(u32 *)(IOADDR_CG_REG_BASE + 0x30) |= (div & 0x3FFF);
	} else if (pwm_id < 8) {
		*(u32 *)(IOADDR_CG_REG_BASE + 0x30) &= ~(0x3FFF << 16);
		*(u32 *)(IOADDR_CG_REG_BASE + 0x30) |= ((div << 16) & (0x3FFF << 16));
	} else if (pwm_id == 8) {
		*(u32 *)(IOADDR_CG_REG_BASE + 0x34) &= ~(0x3FFF);
		*(u32 *)(IOADDR_CG_REG_BASE + 0x34) |= (div & 0x3FFF);
	} else if (pwm_id == 9) {
		*(u32 *)(IOADDR_CG_REG_BASE + 0x34) &= ~(0x3FFF << 16);
		*(u32 *)(IOADDR_CG_REG_BASE + 0x34) |= ((div << 16) & (0x3FFF << 16));
	} else if (pwm_id == 10) {
		*(u32 *)(IOADDR_CG_REG_BASE + 0x38) &= ~(0x3FFF);
		*(u32 *)(IOADDR_CG_REG_BASE + 0x38) |= (div & 0x3FFF);
	} else if (pwm_id == 11) {
		*(u32 *)(IOADDR_CG_REG_BASE + 0x38) &= ~(0x3FFF << 16);
		*(u32 *)(IOADDR_CG_REG_BASE + 0x38) |= ((div << 16) & (0x3FFF << 16));
	} else if (pwm_id == 12) {
		*(u32 *)(IOADDR_CG_REG_BASE + 0x324) &= ~(0x3FFF << 16);
		*(u32 *)(IOADDR_CG_REG_BASE + 0x324) |= ((div << 16) & (0x3FFF << 16));
	}
}

void pwm_enable_clk(int pwm_id)
{
	/* Enable clk */
	if (pwm_id < 12) {
		*(u32 *)(IOADDR_CG_REG_BASE + 0x84) |= (0x1 << pwm_id);
		*(u32 *)(IOADDR_CG_REG_BASE + 0xa4) |= (0x1 << 0);
	} else if(pwm_id == 12) {
		*(u32 *)(IOADDR_CG_REG_BASE + 0x8C) |= (0x1 << 22);
		*(u32 *)(IOADDR_CG_REG_BASE + 0xa4) |= (0x1 << 0);
	}
}

bool pwmid_is_valid(int pwm_id)
{
    if (pwm_id < 0 || pwm_id >= NUM_PWM) {
        return false;
    }

    return true;
}