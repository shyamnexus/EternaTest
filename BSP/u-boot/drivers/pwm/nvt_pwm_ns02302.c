#include "nvt_pwm.h"

#define NUM_PWM                 12

#define PIN_PWM_CFG_PWM0_1              0x1          ///< PWM0_1  (P_GPIO[0])
#define PIN_PWM_CFG_PWM0_2              0x2          ///< PWM0_2  (C_GPIO[15])
#define PIN_PWM_CFG_PWM0_3              0x4          ///< PWM0_3  (S_GPIO[1])
#define PIN_PWM_CFG_PWM0_4              0x8          ///< PWM0_4  (C_GPIO[4])
#define PIN_PWM_CFG_PWM0_5              0x10         ///< PWM0_5  (S_GPIO[9])

#define PIN_PWM_CFG_PWM1_1              0x100         ///< PWM1_1  (P_GPIO[1])
#define PIN_PWM_CFG_PWM1_2              0x200         ///< PWM1_2  (C_GPIO[16])
#define PIN_PWM_CFG_PWM1_3              0x400         ///< PWM1_3  (S_GPIO[2])
#define PIN_PWM_CFG_PWM1_4              0x800         ///< PWM1_4  (C_GPIO[5])
#define PIN_PWM_CFG_PWM1_5              0x1000        ///< PWM1_5  (S_GPIO[10])

#define PIN_PWM_CFG_PWM2_1              0x10000        ///< PWM2_1  (P_GPIO[2])
#define PIN_PWM_CFG_PWM2_2              0x20000        ///< PWM2_2  (C_GPIO[17])
#define PIN_PWM_CFG_PWM2_3              0x40000        ///< PWM2_3  (S_GPIO[3])
#define PIN_PWM_CFG_PWM2_4              0x80000        ///< PWM2_4  (C_GPIO[6])
#define PIN_PWM_CFG_PWM2_5              0x100000       ///< PWM2_5  (S_GPIO[11])

#define PIN_PWM_CFG_PWM3_1              0x1000000       ///< PWM3_1  (P_GPIO[3])
#define PIN_PWM_CFG_PWM3_2              0x2000000       ///< PWM3_2  (C_GPIO[18])
#define PIN_PWM_CFG_PWM3_3              0x4000000       ///< PWM3_3  (S_GPIO[4])
#define PIN_PWM_CFG_PWM3_4              0x8000000       ///< PWM3_4  (C_GPIO[7])
#define PIN_PWM_CFG_PWM3_5              0x10000000      ///< PWM3_5  (S_GPIO[12])

#define PIN_PWMII_CFG_PWM4_1              0x1          ///< PWM4_1  (P_GPIO[4])
#define PIN_PWMII_CFG_PWM4_2              0x2          ///< PWM4_2  (C_GPIO[13])
#define PIN_PWMII_CFG_PWM4_3              0x4          ///< PWM4_3  (S_GPIO[5])
#define PIN_PWMII_CFG_PWM4_4              0x8          ///< PWM4_4  (D_GPIO[3])
#define PIN_PWMII_CFG_PWM4_5              0x10         ///< PWM4_5  (S_GPIO[13])

#define PIN_PWMII_CFG_PWM5_1              0x100         ///< PWM5_1  (P_GPIO[5])
#define PIN_PWMII_CFG_PWM5_2              0x200         ///< PWM5_2  (C_GPIO[14])
#define PIN_PWMII_CFG_PWM5_3              0x400         ///< PWM5_3  (S_GPIO[6])
#define PIN_PWMII_CFG_PWM5_4              0x800         ///< PWM5_4  (D_GPIO[4])
#define PIN_PWMII_CFG_PWM5_5              0x1000        ///< PWM5_5  (S_GPIO[14])

#define PIN_PWMII_CFG_PWM6_1              0x10000        ///< PWM6_1  (P_GPIO[6])
#define PIN_PWMII_CFG_PWM6_2              0x20000        ///< PWM6_2  (C_GPIO[19])
#define PIN_PWMII_CFG_PWM6_3              0x40000        ///< PWM6_3  (S_GPIO[7])
#define PIN_PWMII_CFG_PWM6_4              0x80000        ///< PWM6_4  (D_GPIO[5])
#define PIN_PWMII_CFG_PWM6_5              0x100000       ///< PWM6_5  (S_GPIO[15])

#define PIN_PWMII_CFG_PWM7_1              0x1000000       ///< PWM7_1  (P_GPIO[7])
#define PIN_PWMII_CFG_PWM7_2              0x2000000       ///< PWM7_2  (C_GPIO[20])
#define PIN_PWMII_CFG_PWM7_3              0x4000000       ///< PWM7_3  (S_GPIO[8])
#define PIN_PWMII_CFG_PWM7_4              0x8000000       ///< PWM7_4  (D_GPIO[6])
#define PIN_PWMII_CFG_PWM7_5              0x10000000      ///< PWM7_5  (S_GPIO[16])

#define PIN_PWMIII_CFG_PWM8_1              0x1     ///< PWM8_1  (P_GPIO[8])
#define PIN_PWMIII_CFG_PWM8_2              0x2     ///< PWM8_2  (C_GPIO[20])
#define PIN_PWMIII_CFG_PWM8_4              0x8     ///< PWM8_4  (D_GPIO[8])
#define PIN_PWMIII_CFG_PWM8_5              0x10    ///< PWM8_5  (L_GPIO[8])

#define PIN_PWMIII_CFG_PWM9_1              0x100     ///< PWM9_1  (P_GPIO[9])
#define PIN_PWMIII_CFG_PWM9_2              0x200     ///< PWM9_2  (C_GPIO[21])
#define PIN_PWMIII_CFG_PWM9_4              0x800     ///< PWM9_4  (D_GPIO[9])
#define PIN_PWMIII_CFG_PWM9_5              0x1000    ///< PWM9_5  (L_GPIO[9])

#define PIN_PWMIII_CFG_PWM10_1              0x10000     ///< PWM10_1  (P_GPIO[10])
#define PIN_PWMIII_CFG_PWM10_2              0x20000     ///< PWM10_2  (C_GPIO[22])
#define PIN_PWMIII_CFG_PWM10_4              0x80000     ///< PWM10_4  (D_GPIO[10])
#define PIN_PWMIII_CFG_PWM10_5              0x100000    ///< PWM10_5  (L_GPIO[10])

#define PIN_PWMIII_CFG_PWM11_1              0x1000000     ///< PWM10_1  (P_GPIO[11])
#define PIN_PWMIII_CFG_PWM11_2              0x2000000     ///< PWM10_2  (C_GPIO[23])
#define PIN_PWMIII_CFG_PWM11_4              0x8000000     ///< PWM10_4  (D_GPIO[7])
#define PIN_PWMIII_CFG_PWM11_5              0x10000000    ///< PWM10_5  (L_GPIO[11])

static void nvt_pwm_masked_write(u32 hi, u32 lo, u32 val, void __iomem *phy_reg)
{
	u32 mask, tmp;

	mask = GENMASK(hi, lo);
	tmp = readl(phy_reg);

	tmp &= ~(mask);
	tmp |= ((val << lo) & mask);

	writel(tmp, phy_reg);
}

void nvt_pwm_set_pinmux(int pwm_id, u32 pwm_pinmux)
{
	printf("pwm_id(%d) pwm_pinmux(0x%x)\n", pwm_id, pwm_pinmux);
	void __iomem *top_base = (void __iomem *)IOADDR_TOP_REG_BASE;

	if (pwm_id == 0) {
		if (pwm_pinmux & PIN_PWM_CFG_PWM0_1) {
			nvt_pwm_masked_write(3, 0, 1, top_base + 0x18);
			nvt_pwm_masked_write(0, 0, 0, top_base + 0xA8);
		} else if (pwm_pinmux & PIN_PWM_CFG_PWM0_2) {
			nvt_pwm_masked_write(3, 0, 2, top_base + 0x18);
			nvt_pwm_masked_write(14, 14, 0, top_base + 0xA0);
		} else if (pwm_pinmux & PIN_PWM_CFG_PWM0_3) {
			nvt_pwm_masked_write(3, 0, 3, top_base + 0x18);
			nvt_pwm_masked_write(1, 1, 0, top_base + 0xC0);
		} else if (pwm_pinmux & PIN_PWM_CFG_PWM0_4) {
			nvt_pwm_masked_write(3, 0, 4, top_base + 0x18);
			nvt_pwm_masked_write(4, 4, 0, top_base + 0xA0);
		} else if (pwm_pinmux & PIN_PWM_CFG_PWM0_5) {
			nvt_pwm_masked_write(3, 0, 5, top_base + 0x18);
			nvt_pwm_masked_write(0, 0, 0, top_base + 0xB8);
		} else {
            DBG_ERR("!!Caution!!>>using pwm_id(%d) but pwm_pinmux(0x%x) not configured(check nvt-top.dtsi)\n", pwm_id, pwm_pinmux);
		}
	} else if (pwm_id == 1) {
		if (pwm_pinmux & PIN_PWM_CFG_PWM1_1) {
			nvt_pwm_masked_write(7, 4, 1, top_base + 0x18);
			nvt_pwm_masked_write(1, 1, 0, top_base + 0xA8);
		} else if (pwm_pinmux & PIN_PWM_CFG_PWM1_2) {
			nvt_pwm_masked_write(7, 4, 2, top_base + 0x18);
			nvt_pwm_masked_write(15, 15, 0, top_base + 0xA0);
		} else if (pwm_pinmux & PIN_PWM_CFG_PWM1_3) {
			nvt_pwm_masked_write(7, 4, 3, top_base + 0x18);
			nvt_pwm_masked_write(2, 2, 0, top_base + 0xC0);
		} else if (pwm_pinmux & PIN_PWM_CFG_PWM1_4) {
			nvt_pwm_masked_write(7, 4, 4, top_base + 0x18);
			nvt_pwm_masked_write(5, 5, 0, top_base + 0xA0);
		} else if (pwm_pinmux & PIN_PWM_CFG_PWM1_5) {
			nvt_pwm_masked_write(7, 4, 5, top_base + 0x18);
			nvt_pwm_masked_write(1, 1, 0, top_base + 0xB8);
		} else {
			DBG_ERR("!!Caution!!>>using pwm_id(%d) but pwm_pinmux(0x%x) not configured(check nvt-top.dtsi)\n", pwm_id, pwm_pinmux);
		}
	} else if (pwm_id == 2) {
		if (pwm_pinmux & PIN_PWM_CFG_PWM2_1) {
			nvt_pwm_masked_write(11, 8, 1, top_base + 0x18);
			nvt_pwm_masked_write(2, 2, 0, top_base + 0xA8);
		} else if (pwm_pinmux & PIN_PWM_CFG_PWM2_2) {
			nvt_pwm_masked_write(11, 8, 2, top_base + 0x18);
			nvt_pwm_masked_write(16, 16, 0, top_base + 0xA0);
		} else if (pwm_pinmux & PIN_PWM_CFG_PWM2_3) {
			nvt_pwm_masked_write(11, 8, 3, top_base + 0x18);
			nvt_pwm_masked_write(3, 3, 0, top_base + 0xC0);
		} else if (pwm_pinmux & PIN_PWM_CFG_PWM2_4) {
			nvt_pwm_masked_write(11, 8, 4, top_base + 0x18);
			nvt_pwm_masked_write(6, 6, 0, top_base + 0xA0);
		} else if (pwm_pinmux & PIN_PWM_CFG_PWM2_5) {
			nvt_pwm_masked_write(11, 8, 5, top_base + 0x18);
			nvt_pwm_masked_write(2, 2, 0, top_base + 0xB8);
		} else {
			DBG_ERR("!!Caution!!>>using pwm_id(%d) but pwm_pinmux(0x%x) not configured(check nvt-top.dtsi)\n", pwm_id, pwm_pinmux);
		}
	} else if (pwm_id == 3) {
		if (pwm_pinmux & PIN_PWM_CFG_PWM3_1) {
			nvt_pwm_masked_write(15, 12, 1, top_base + 0x18);
			nvt_pwm_masked_write(3, 3, 0, top_base + 0xA8);
		} else if (pwm_pinmux & PIN_PWM_CFG_PWM3_2) {
			nvt_pwm_masked_write(15, 12, 2, top_base + 0x18);
			nvt_pwm_masked_write(17, 17, 0, top_base + 0xA0);
		} else if (pwm_pinmux & PIN_PWM_CFG_PWM3_3) {
			nvt_pwm_masked_write(15, 12, 3, top_base + 0x18);
			nvt_pwm_masked_write(4, 4, 0, top_base + 0xC0);
		} else if (pwm_pinmux & PIN_PWM_CFG_PWM3_4) {
			nvt_pwm_masked_write(15, 12, 4, top_base + 0x18);
			nvt_pwm_masked_write(7, 7, 0, top_base + 0xA0);
		} else if (pwm_pinmux & PIN_PWM_CFG_PWM3_5) {
			nvt_pwm_masked_write(15, 12, 5, top_base + 0x18);
			nvt_pwm_masked_write(3, 3, 0, top_base + 0xB8);
		} else {
			DBG_ERR("!!Caution!!>>using pwm_id(%d) but pwm_pinmux(0x%x) not configured(check nvt-top.dtsi)\n", pwm_id, pwm_pinmux);
		}
	} else if (pwm_id == 4) {
		if (pwm_pinmux & PIN_PWMII_CFG_PWM4_1) {
			nvt_pwm_masked_write(19, 16, 1, top_base + 0x18);
			nvt_pwm_masked_write(4, 4, 0, top_base + 0xA8);
		} else if (pwm_pinmux & PIN_PWMII_CFG_PWM4_2) {
			nvt_pwm_masked_write(19, 16, 2, top_base + 0x18);
			nvt_pwm_masked_write(12, 12, 0, top_base + 0xA0);
		} else if (pwm_pinmux & PIN_PWMII_CFG_PWM4_3) {
			nvt_pwm_masked_write(19, 16, 3, top_base + 0x18);
			nvt_pwm_masked_write(5, 5, 0, top_base + 0xC0);
		} else if (pwm_pinmux & PIN_PWMII_CFG_PWM4_4) {
			nvt_pwm_masked_write(19, 16, 4, top_base + 0x18);
			nvt_pwm_masked_write(3, 3, 0, top_base + 0xB4);
		} else if (pwm_pinmux & PIN_PWMII_CFG_PWM4_5) {
			nvt_pwm_masked_write(19, 16, 5, top_base + 0x18);
			nvt_pwm_masked_write(4, 4, 0, top_base + 0xB8);
		} else {
			DBG_ERR("!!Caution!!>>using pwm_id(%d) but pwm_pinmux(0x%x) not configured(check nvt-top.dtsi)\n", pwm_id, pwm_pinmux);
		}
	} else if (pwm_id == 5) {
		if (pwm_pinmux & PIN_PWMII_CFG_PWM5_1) {
			nvt_pwm_masked_write(23, 20, 1, top_base + 0x18);
			nvt_pwm_masked_write(5, 5, 0, top_base + 0xA8);
		} else if (pwm_pinmux & PIN_PWMII_CFG_PWM5_2) {
			nvt_pwm_masked_write(23, 20, 2, top_base + 0x18);
			nvt_pwm_masked_write(13, 13, 0, top_base + 0xA0);
		} else if (pwm_pinmux & PIN_PWMII_CFG_PWM5_3) {
			nvt_pwm_masked_write(23, 20, 3, top_base + 0x18);
			nvt_pwm_masked_write(6, 6, 0, top_base + 0xC0);
		} else if (pwm_pinmux & PIN_PWMII_CFG_PWM5_4) {
			nvt_pwm_masked_write(23, 20, 4, top_base + 0x18);
			nvt_pwm_masked_write(4, 4, 0, top_base + 0xB4);
		} else if (pwm_pinmux & PIN_PWMII_CFG_PWM5_5) {
			nvt_pwm_masked_write(23, 20, 5, top_base + 0x18);
			nvt_pwm_masked_write(5, 5, 0, top_base + 0xB8);
		} else {
			DBG_ERR("!!Caution!!>>using pwm_id(%d) but pwm_pinmux(0x%x) not configured(check nvt-top.dtsi)\n", pwm_id, pwm_pinmux);
		}
	} else if (pwm_id == 6) {
		if (pwm_pinmux & PIN_PWMII_CFG_PWM6_1) {
			nvt_pwm_masked_write(27, 24, 1, top_base + 0x18);
			nvt_pwm_masked_write(6, 6, 0, top_base + 0xA8);
		} else if (pwm_pinmux & PIN_PWMII_CFG_PWM6_2) {
			nvt_pwm_masked_write(27, 24, 2, top_base + 0x18);
			nvt_pwm_masked_write(18, 18, 0, top_base + 0xA0);
		} else if (pwm_pinmux & PIN_PWMII_CFG_PWM6_3) {
			nvt_pwm_masked_write(27, 24, 3, top_base + 0x18);
			nvt_pwm_masked_write(7, 7, 0, top_base + 0xC0);
		} else if (pwm_pinmux & PIN_PWMII_CFG_PWM6_4) {
			nvt_pwm_masked_write(27, 24, 4, top_base + 0x18);
			nvt_pwm_masked_write(5, 5, 0, top_base + 0xB4);
		} else if (pwm_pinmux & PIN_PWMII_CFG_PWM6_5) {
			nvt_pwm_masked_write(27, 24, 5, top_base + 0x18);
			nvt_pwm_masked_write(6, 6, 0, top_base + 0xB8);
		} else {
			DBG_ERR("!!Caution!!>>using pwm_id(%d) but pwm_pinmux(0x%x) not configured(check nvt-top.dtsi)\n", pwm_id, pwm_pinmux);
		}
	} else if (pwm_id == 7) {
		if (pwm_pinmux & PIN_PWMII_CFG_PWM7_1) {
			nvt_pwm_masked_write(31, 28, 1, top_base + 0x18);
			nvt_pwm_masked_write(7, 7, 0, top_base + 0xA8);
		} else if (pwm_pinmux & PIN_PWMII_CFG_PWM7_2) {
			nvt_pwm_masked_write(31, 28, 2, top_base + 0x18);
			nvt_pwm_masked_write(19, 19, 0, top_base + 0xA0);
		} else if (pwm_pinmux & PIN_PWMII_CFG_PWM7_3) {
			nvt_pwm_masked_write(31, 28, 3, top_base + 0x18);
			nvt_pwm_masked_write(8, 8, 0, top_base + 0xC0);
		} else if (pwm_pinmux & PIN_PWMII_CFG_PWM7_4) {
			nvt_pwm_masked_write(31, 28, 4, top_base + 0x18);
			nvt_pwm_masked_write(6, 6, 0, top_base + 0xB4);
		} else if (pwm_pinmux & PIN_PWMII_CFG_PWM7_5) {
			nvt_pwm_masked_write(31, 28, 5, top_base + 0x18);
			nvt_pwm_masked_write(7, 7, 0, top_base + 0xB8);
		} else {
			DBG_ERR("!!Caution!!>>using pwm_id(%d) but pwm_pinmux(0x%x) not configured(check nvt-top.dtsi)\n", pwm_id, pwm_pinmux);
		}
	} else if (pwm_id == 8) {
		if (pwm_pinmux & PIN_PWMIII_CFG_PWM8_1) {
            nvt_pwm_masked_write(3, 0, 1, top_base + 0x1C);
            nvt_pwm_masked_write(8, 8, 0, top_base + 0xA8);
		} else if (pwm_pinmux & PIN_PWMIII_CFG_PWM8_2) {
            nvt_pwm_masked_write(3, 0, 2, top_base + 0x1C);
            nvt_pwm_masked_write(20, 20, 0, top_base + 0xA0);
		} else if (pwm_pinmux & PIN_PWMIII_CFG_PWM8_4) {
			nvt_pwm_masked_write(3, 0, 4, top_base + 0x1C);
            nvt_pwm_masked_write(8, 8, 0, top_base + 0xB4);
		} else if (pwm_pinmux & PIN_PWMIII_CFG_PWM8_5) {
			nvt_pwm_masked_write(3, 0, 5, top_base + 0x1C);
            nvt_pwm_masked_write(8, 8, 0, top_base + 0xB8);
		} else {
			DBG_ERR("!!Caution!!>>using pwm_id(%d) but pwm_pinmux(0x%x) not configured(check nvt-top.dtsi)\n", pwm_id, pwm_pinmux);
		}
	} else if (pwm_id == 9) {
		if (pwm_pinmux & PIN_PWMIII_CFG_PWM9_1) {
            nvt_pwm_masked_write(7, 4, 1, top_base + 0x1C);
            nvt_pwm_masked_write(9, 9, 0, top_base + 0xA8);
		} else if (pwm_pinmux & PIN_PWMIII_CFG_PWM9_2) {
            nvt_pwm_masked_write(7, 4, 2, top_base + 0x1C);
            nvt_pwm_masked_write(21, 21, 0, top_base + 0xA0);
		} else if (pwm_pinmux & PIN_PWMIII_CFG_PWM9_4) {
            nvt_pwm_masked_write(7, 4, 4, top_base + 0x1C);
            nvt_pwm_masked_write(9, 9, 0, top_base + 0xB4);
		} else if (pwm_pinmux & PIN_PWMIII_CFG_PWM9_5) {
			nvt_pwm_masked_write(7, 4, 5, top_base + 0x1C);
            nvt_pwm_masked_write(9, 9, 0, top_base + 0xB8);
		} else {
			DBG_ERR("!!Caution!!>>using pwm_id(%d) but pwm_pinmux(0x%x) not configured(check nvt-top.dtsi)\n", pwm_id, pwm_pinmux);
		}
	} else if (pwm_id == 10) {
		if (pwm_pinmux & PIN_PWMIII_CFG_PWM10_1) {
			nvt_pwm_masked_write(11, 8, 1, top_base + 0x1C);
			nvt_pwm_masked_write(10, 10, 0, top_base + 0xA8);
		} else if (pwm_pinmux & PIN_PWMIII_CFG_PWM10_2) {
			nvt_pwm_masked_write(11, 8, 2, top_base + 0x1C);
			nvt_pwm_masked_write(22, 22, 0, top_base + 0xA0);
		} else if (pwm_pinmux & PIN_PWMIII_CFG_PWM10_4) {
			nvt_pwm_masked_write(11, 8, 4, top_base + 0x1C);
			nvt_pwm_masked_write(10, 10, 0, top_base + 0xB4);
		} else if (pwm_pinmux & PIN_PWMIII_CFG_PWM10_5) {
			nvt_pwm_masked_write(11, 8, 5, top_base + 0x1C);
			nvt_pwm_masked_write(10, 10, 0, top_base + 0xB8);
		} else {
			DBG_ERR("!!Caution!!>>using pwm_id(%d) but pwm_pinmux(0x%x) not configured(check nvt-top.dtsi)\n", pwm_id, pwm_pinmux);
		}
	} else if (pwm_id == 11) {
		if (pwm_pinmux & PIN_PWMIII_CFG_PWM11_1) {
			nvt_pwm_masked_write(15, 12, 1, top_base + 0x1C);
			nvt_pwm_masked_write(11, 11, 0, top_base + 0xA8);
		} else if (pwm_pinmux & PIN_PWMIII_CFG_PWM11_2) {
			nvt_pwm_masked_write(15, 12, 2, top_base + 0x1C);
			nvt_pwm_masked_write(23, 23, 0, top_base + 0xA0);
		} else if (pwm_pinmux & PIN_PWMIII_CFG_PWM11_4) {
			nvt_pwm_masked_write(15, 12, 4, top_base + 0x1C);
			nvt_pwm_masked_write(7, 7, 0, top_base + 0xB4);
		} else if (pwm_pinmux & PIN_PWMIII_CFG_PWM11_5) {
			nvt_pwm_masked_write(15, 12, 5, top_base + 0x1C);
			nvt_pwm_masked_write(11, 11, 0, top_base + 0xB8);
		} else {
			DBG_ERR("!!Caution!!>>using pwm_id(%d) but pwm_pinmux(0x%x) not configured(check nvt-top.dtsi)\n", pwm_id, pwm_pinmux);
		}
	} else {
		printf("nvt_pwm_set_div: invalid pwm_id%d\r\n", pwm_id);
	}
}

int do_nvt_pwm_set_pinmux(int pwm_id)
{
	char pwm[10] = {0};
	char path[20] = {0};
	int nodeoffset, len;
    u32 pwm_pinmux = 0;
	u32 *cell = NULL;

	if (pwm_id < 4) {
		sprintf(pwm, "pwm");
	} else if (pwm_id < 8) {
		sprintf(pwm, "pwmII");
	} else {
		sprintf(pwm, "pwmIII");
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
	void __iomem *cg_base = (void __iomem *)IOADDR_CG_REG_BASE;

	if (pwm_id < 4) {
		nvt_pwm_masked_write(13, 0, div, cg_base + 0x50);
	} else if (pwm_id < 8) {
		nvt_pwm_masked_write(29, 16, div, cg_base + 0x50);
	} else if (pwm_id == 8) {
		nvt_pwm_masked_write(13, 0, div, cg_base + 0x54);
	} else if (pwm_id == 9) {
		nvt_pwm_masked_write(29, 16, div, cg_base + 0x54);
	} else if (pwm_id == 10) {
		nvt_pwm_masked_write(13, 0, div, cg_base + 0x58);
	} else if (pwm_id == 11) {
		nvt_pwm_masked_write(29, 16, div, cg_base + 0x58);
	} else {
		printf("nvt_pwm_set_div: invalid pwm_id%d\r\n", pwm_id);
	}
}

void pwm_enable_clk(int pwm_id)
{
	/* Enable clk */
	if (pwm_id < 12) {
		void __iomem *cg_base = (void __iomem *)IOADDR_CG_REG_BASE;
		nvt_pwm_masked_write(pwm_id, pwm_id, 1, cg_base + 0x7C);
		nvt_pwm_masked_write(8, 8, 1, cg_base + 0x98);
	} else {
		printf("pwm_enable_clk: invalid pwm_id%d\r\n", pwm_id);
	}
}

bool pwmid_is_valid(int pwm_id)
{
    if (pwm_id < 0 || pwm_id >= NUM_PWM) {
        return false;
    }

    return true;
}