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

#define PIN_PWMIII_CFG_PWM8_1              0x1            ///< PWM8_1  (P_GPIO[8])
#define PIN_PWMIII_CFG_PWM8_2              0x2            ///< PWM8_2  (C_GPIO[21])
#define PIN_PWMIII_CFG_PWM8_3              0x4            ///< PWM8_3  (HSI_GPIO[6])
#define PIN_PWMIII_CFG_PWM8_4              0x8            ///< PWM8_4  (P_GPIO[25])

#define PIN_PWMIII_CFG_PWM9_1              0x100          ///< PWM9_1  (P_GPIO[9])
#define PIN_PWMIII_CFG_PWM9_2              0x200          ///< PWM9_2  (C_GPIO[22])
#define PIN_PWMIII_CFG_PWM9_3              0x400          ///< PWM9_3  (HSI_GPIO[7])
#define PIN_PWMIII_CFG_PWM9_4              0x800          ///< PWM9_4  (P_GPIO[26])

#define PIN_PWMIII_CFG_PWM10_1             0x10000        ///< PWM10_1  (P_GPIO[10])
#define PIN_PWMIII_CFG_PWM10_2             0x20000        ///< PWM10_2  (C_GPIO[23])
#define PIN_PWMIII_CFG_PWM10_3             0x40000        ///< PWM10_3  (HSI_GPIO[8])
#define PIN_PWMIII_CFG_PWM10_4             0x80000        ///< PWM10_4  (P_GPIO[27])

#define PIN_PWMIII_CFG_PWM11_1             0x1000000      ///< PWM11_1  (P_GPIO[11])
#define PIN_PWMIII_CFG_PWM11_2             0x2000000      ///< PWM11_2  (C_GPIO[24])
#define PIN_PWMIII_CFG_PWM11_3             0x4000000      ///< PWM11_3  (HSI_GPIO[9])
#define PIN_PWMIII_CFG_PWM11_4             0x8000000      ///< PWM11_4  (P_GPIO[28])

static void nvt_pwm_masked_write(u32 hi, u32 lo, u32 val, u32 phy_reg)
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

	if (pwm_id == 0) {
		if (pwm_pinmux & PIN_PWM_CFG_PWM0_1) {
			nvt_pwm_masked_write(2, 0, 1, IOADDR_TOP_REG_BASE + 0x14);
			nvt_pwm_masked_write(0, 0, 0, IOADDR_TOP_REG_BASE + 0xA8);
		} else if (pwm_pinmux & PIN_PWM_CFG_PWM0_2) {
			nvt_pwm_masked_write(2, 0, 2, IOADDR_TOP_REG_BASE + 0x14);
			nvt_pwm_masked_write(15, 15, 0, IOADDR_TOP_REG_BASE + 0xA0);
		} else if (pwm_pinmux & PIN_PWM_CFG_PWM0_3) {
			nvt_pwm_masked_write(2, 0, 3, IOADDR_TOP_REG_BASE + 0x14);
			nvt_pwm_masked_write(3, 2, 2, IOADDR_TOP_REG_BASE + 0x100028);
		} else if (pwm_pinmux & PIN_PWM_CFG_PWM0_4) {
			nvt_pwm_masked_write(2, 0, 4, IOADDR_TOP_REG_BASE + 0x14);
			nvt_pwm_masked_write(4, 4, 0, IOADDR_TOP_REG_BASE + 0xA0);
		} else if (pwm_pinmux & PIN_PWM_CFG_PWM0_5) {
			nvt_pwm_masked_write(2, 0, 5, IOADDR_TOP_REG_BASE + 0x14);
			nvt_pwm_masked_write(19, 18, 2, IOADDR_TOP_REG_BASE + 0x100028);
		} else {
            DBG_ERR("!!Caution!!>>using pwm_id(%d) but pwm_pinmux(0x%x) not configured(check nvt-top.dtsi)\n", pwm_id, pwm_pinmux);
		}
	} else if (pwm_id == 1) {
		if (pwm_pinmux & PIN_PWM_CFG_PWM1_1) {
			nvt_pwm_masked_write(5, 3, 1, IOADDR_TOP_REG_BASE + 0x14);
			nvt_pwm_masked_write(1, 1, 0, IOADDR_TOP_REG_BASE + 0xA8);
		} else if (pwm_pinmux & PIN_PWM_CFG_PWM1_2) {
			nvt_pwm_masked_write(5, 3, 2, IOADDR_TOP_REG_BASE + 0x14);
			nvt_pwm_masked_write(16, 16, 0, IOADDR_TOP_REG_BASE + 0xA0);
		} else if (pwm_pinmux & PIN_PWM_CFG_PWM1_3) {
			nvt_pwm_masked_write(5, 3, 3, IOADDR_TOP_REG_BASE + 0x14);
			nvt_pwm_masked_write(5, 4, 2, IOADDR_TOP_REG_BASE + 0x100028);
		} else if (pwm_pinmux & PIN_PWM_CFG_PWM1_4) {
			nvt_pwm_masked_write(5, 3, 4, IOADDR_TOP_REG_BASE + 0x14);
			nvt_pwm_masked_write(5, 5, 0, IOADDR_TOP_REG_BASE + 0xA0);
		} else if (pwm_pinmux & PIN_PWM_CFG_PWM1_5) {
			nvt_pwm_masked_write(5, 3, 5, IOADDR_TOP_REG_BASE + 0x14);
			nvt_pwm_masked_write(21, 20, 2, IOADDR_TOP_REG_BASE + 0x100028);
		} else {
			DBG_ERR("!!Caution!!>>using pwm_id(%d) but pwm_pinmux(0x%x) not configured(check nvt-top.dtsi)\n", pwm_id, pwm_pinmux);
		}
	} else if (pwm_id == 2) {
		if (pwm_pinmux & PIN_PWM_CFG_PWM2_1) {
			nvt_pwm_masked_write(8, 6, 1, IOADDR_TOP_REG_BASE + 0x14);
			nvt_pwm_masked_write(2, 2, 0, IOADDR_TOP_REG_BASE + 0xA8);
		} else if (pwm_pinmux & PIN_PWM_CFG_PWM2_2) {
			nvt_pwm_masked_write(8, 6, 2, IOADDR_TOP_REG_BASE + 0x14);
			nvt_pwm_masked_write(17, 17, 0, IOADDR_TOP_REG_BASE + 0xA0);
		} else if (pwm_pinmux & PIN_PWM_CFG_PWM2_3) {
			nvt_pwm_masked_write(8, 6, 3, IOADDR_TOP_REG_BASE + 0x14);
			nvt_pwm_masked_write(7, 6, 2, IOADDR_TOP_REG_BASE + 0x100028);
		} else if (pwm_pinmux & PIN_PWM_CFG_PWM2_4) {
			nvt_pwm_masked_write(8, 6, 4, IOADDR_TOP_REG_BASE + 0x14);
			nvt_pwm_masked_write(6, 6, 0, IOADDR_TOP_REG_BASE + 0xA0);
		} else if (pwm_pinmux & PIN_PWM_CFG_PWM2_5) {
			nvt_pwm_masked_write(8, 6, 5, IOADDR_TOP_REG_BASE + 0x14);
			nvt_pwm_masked_write(23, 22, 2, IOADDR_TOP_REG_BASE + 0x100028);
		} else {
			DBG_ERR("!!Caution!!>>using pwm_id(%d) but pwm_pinmux(0x%x) not configured(check nvt-top.dtsi)\n", pwm_id, pwm_pinmux);
		}
	} else if (pwm_id == 3) {
		if (pwm_pinmux & PIN_PWM_CFG_PWM3_1) {
			nvt_pwm_masked_write(11, 9, 1, IOADDR_TOP_REG_BASE + 0x14);
			nvt_pwm_masked_write(3, 3, 0, IOADDR_TOP_REG_BASE + 0xA8);
		} else if (pwm_pinmux & PIN_PWM_CFG_PWM3_2) {
			nvt_pwm_masked_write(11, 9, 2, IOADDR_TOP_REG_BASE + 0x14);
			nvt_pwm_masked_write(18, 18, 0, IOADDR_TOP_REG_BASE + 0xA0);
		} else if (pwm_pinmux & PIN_PWM_CFG_PWM3_3) {
			nvt_pwm_masked_write(11, 9, 3, IOADDR_TOP_REG_BASE + 0x14);
			nvt_pwm_masked_write(9, 8, 2, IOADDR_TOP_REG_BASE + 0x100028);
		} else if (pwm_pinmux & PIN_PWM_CFG_PWM3_4) {
			nvt_pwm_masked_write(11, 9, 4, IOADDR_TOP_REG_BASE + 0x14);
			nvt_pwm_masked_write(7, 7, 0, IOADDR_TOP_REG_BASE + 0xA0);
		} else if (pwm_pinmux & PIN_PWM_CFG_PWM3_5) {
			nvt_pwm_masked_write(11, 9, 5, IOADDR_TOP_REG_BASE + 0x14);
			nvt_pwm_masked_write(25, 24, 2, IOADDR_TOP_REG_BASE + 0x100028);
		} else {
			DBG_ERR("!!Caution!!>>using pwm_id(%d) but pwm_pinmux(0x%x) not configured(check nvt-top.dtsi)\n", pwm_id, pwm_pinmux);
		}
	} else if (pwm_id == 4) {
		if (pwm_pinmux & PIN_PWMII_CFG_PWM4_1) {
			nvt_pwm_masked_write(14, 12, 1, IOADDR_TOP_REG_BASE + 0x14);
			nvt_pwm_masked_write(4, 4, 0, IOADDR_TOP_REG_BASE + 0xA8);
		} else if (pwm_pinmux & PIN_PWMII_CFG_PWM4_2) {
			nvt_pwm_masked_write(14, 12, 2, IOADDR_TOP_REG_BASE + 0x14);
			nvt_pwm_masked_write(13, 13, 0, IOADDR_TOP_REG_BASE + 0xA0);
		} else if (pwm_pinmux & PIN_PWMII_CFG_PWM4_3) {
			nvt_pwm_masked_write(14, 12, 3, IOADDR_TOP_REG_BASE + 0x14);
			nvt_pwm_masked_write(11, 10, 2, IOADDR_TOP_REG_BASE + 0x100028);
		} else if (pwm_pinmux & PIN_PWMII_CFG_PWM4_4) {
			nvt_pwm_masked_write(14, 12, 4, IOADDR_TOP_REG_BASE + 0x14);
			nvt_pwm_masked_write(3, 3, 0, IOADDR_TOP_REG_BASE + 0xD0);
		} else if (pwm_pinmux & PIN_PWMII_CFG_PWM4_5) {
			nvt_pwm_masked_write(14, 12, 5, IOADDR_TOP_REG_BASE + 0x14);
			nvt_pwm_masked_write(27, 26, 2, IOADDR_TOP_REG_BASE + 0x100028);
		} else {
			DBG_ERR("!!Caution!!>>using pwm_id(%d) but pwm_pinmux(0x%x) not configured(check nvt-top.dtsi)\n", pwm_id, pwm_pinmux);
		}
	} else if (pwm_id == 5) {
		if (pwm_pinmux & PIN_PWMII_CFG_PWM5_1) {
			nvt_pwm_masked_write(17, 15, 1, IOADDR_TOP_REG_BASE + 0x14);
			nvt_pwm_masked_write(5, 5, 0, IOADDR_TOP_REG_BASE + 0xA8);
		} else if (pwm_pinmux & PIN_PWMII_CFG_PWM5_2) {
			nvt_pwm_masked_write(17, 15, 2, IOADDR_TOP_REG_BASE + 0x14);
			nvt_pwm_masked_write(14, 14, 0, IOADDR_TOP_REG_BASE + 0xA0);
		} else if (pwm_pinmux & PIN_PWMII_CFG_PWM5_3) {
			nvt_pwm_masked_write(17, 15, 3, IOADDR_TOP_REG_BASE + 0x14);
			nvt_pwm_masked_write(13, 12, 2, IOADDR_TOP_REG_BASE + 0x100028);
		} else if (pwm_pinmux & PIN_PWMII_CFG_PWM5_4) {
			nvt_pwm_masked_write(17, 15, 4, IOADDR_TOP_REG_BASE + 0x14);
			nvt_pwm_masked_write(4, 4, 0, IOADDR_TOP_REG_BASE + 0xD0);
		} else if (pwm_pinmux & PIN_PWMII_CFG_PWM5_5) {
			nvt_pwm_masked_write(17, 15, 5, IOADDR_TOP_REG_BASE + 0x14);
			nvt_pwm_masked_write(29, 28, 2, IOADDR_TOP_REG_BASE + 0x100028);
		} else {
			DBG_ERR("!!Caution!!>>using pwm_id(%d) but pwm_pinmux(0x%x) not configured(check nvt-top.dtsi)\n", pwm_id, pwm_pinmux);
		}
	} else if (pwm_id == 6) {
		if (pwm_pinmux & PIN_PWMII_CFG_PWM6_1) {
			nvt_pwm_masked_write(20, 18, 1, IOADDR_TOP_REG_BASE + 0x14);
			nvt_pwm_masked_write(6, 6, 0, IOADDR_TOP_REG_BASE + 0xA8);
		} else if (pwm_pinmux & PIN_PWMII_CFG_PWM6_2) {
			nvt_pwm_masked_write(20, 18, 2, IOADDR_TOP_REG_BASE + 0x14);
			nvt_pwm_masked_write(19, 19, 0, IOADDR_TOP_REG_BASE + 0xA0);
		} else if (pwm_pinmux & PIN_PWMII_CFG_PWM6_3) {
			nvt_pwm_masked_write(20, 18, 3, IOADDR_TOP_REG_BASE + 0x14);
			nvt_pwm_masked_write(15, 14, 2, IOADDR_TOP_REG_BASE + 0x100028);
		} else if (pwm_pinmux & PIN_PWMII_CFG_PWM6_4) {
			nvt_pwm_masked_write(20, 18, 4, IOADDR_TOP_REG_BASE + 0x14);
			nvt_pwm_masked_write(5, 5, 0, IOADDR_TOP_REG_BASE + 0xD0);
		} else if (pwm_pinmux & PIN_PWMII_CFG_PWM6_5) {
			nvt_pwm_masked_write(20, 18, 5, IOADDR_TOP_REG_BASE + 0x14);
			nvt_pwm_masked_write(31, 30, 2, IOADDR_TOP_REG_BASE + 0x100028);
		} else {
			DBG_ERR("!!Caution!!>>using pwm_id(%d) but pwm_pinmux(0x%x) not configured(check nvt-top.dtsi)\n", pwm_id, pwm_pinmux);
		}
	} else if (pwm_id == 7) {
		if (pwm_pinmux & PIN_PWMII_CFG_PWM7_1) {
			nvt_pwm_masked_write(23, 21, 1, IOADDR_TOP_REG_BASE + 0x14);
			nvt_pwm_masked_write(7, 7, 0, IOADDR_TOP_REG_BASE + 0xA8);
		} else if (pwm_pinmux & PIN_PWMII_CFG_PWM7_2) {
			nvt_pwm_masked_write(23, 21, 2, IOADDR_TOP_REG_BASE + 0x14);
			nvt_pwm_masked_write(20, 20, 0, IOADDR_TOP_REG_BASE + 0xA0);
		} else if (pwm_pinmux & PIN_PWMII_CFG_PWM7_3) {
			nvt_pwm_masked_write(23, 21, 3, IOADDR_TOP_REG_BASE + 0x14);
			nvt_pwm_masked_write(8, 8, 0, IOADDR_TOP_REG_BASE + 0x100028);
		} else if (pwm_pinmux & PIN_PWMII_CFG_PWM7_4) {
			nvt_pwm_masked_write(23, 21, 4, IOADDR_TOP_REG_BASE + 0x14);
			nvt_pwm_masked_write(6, 6, 0, IOADDR_TOP_REG_BASE + 0xD0);
		} else if (pwm_pinmux & PIN_PWMII_CFG_PWM7_5) {
			nvt_pwm_masked_write(23, 21, 5, IOADDR_TOP_REG_BASE + 0x14);
			nvt_pwm_masked_write(1, 0, 2, IOADDR_TOP_REG_BASE + 0x10002C);
		} else {
			DBG_ERR("!!Caution!!>>using pwm_id(%d) but pwm_pinmux(0x%x) not configured(check nvt-top.dtsi)\n", pwm_id, pwm_pinmux);
		}
	} else if (pwm_id == 8) {
		if (pwm_pinmux & PIN_PWMIII_CFG_PWM8_1) {
            nvt_pwm_masked_write(26, 24, 1, IOADDR_TOP_REG_BASE + 0x14);
            nvt_pwm_masked_write(8, 8, 0, IOADDR_TOP_REG_BASE + 0xA8);
		} else if (pwm_pinmux & PIN_PWMIII_CFG_PWM8_2) {
            nvt_pwm_masked_write(26, 24, 2, IOADDR_TOP_REG_BASE + 0x14);
            nvt_pwm_masked_write(21, 21, 0, IOADDR_TOP_REG_BASE + 0xA0);
		} else if (pwm_pinmux & PIN_PWMIII_CFG_PWM8_3) {
			nvt_pwm_masked_write(26, 24, 3, IOADDR_TOP_REG_BASE + 0x14);
            nvt_pwm_masked_write(6, 6, 0, IOADDR_TOP_REG_BASE + 0xD8);
		} else if (pwm_pinmux & PIN_PWMIII_CFG_PWM8_4) {
			nvt_pwm_masked_write(26, 24, 4, IOADDR_TOP_REG_BASE + 0x14);
            nvt_pwm_masked_write(0, 0, 0, IOADDR_TOP_REG_BASE + 0x100030);
		} else {
			DBG_ERR("!!Caution!!>>using pwm_id(%d) but pwm_pinmux(0x%x) not configured(check nvt-top.dtsi)\n", pwm_id, pwm_pinmux);
		}
	} else if (pwm_id == 9) {
		if (pwm_pinmux & PIN_PWMIII_CFG_PWM9_1) {
            nvt_pwm_masked_write(29, 27, 1, IOADDR_TOP_REG_BASE + 0x14);
            nvt_pwm_masked_write(9, 9, 0, IOADDR_TOP_REG_BASE + 0xA8);
		} else if (pwm_pinmux & PIN_PWMIII_CFG_PWM9_2) {
            nvt_pwm_masked_write(29, 27, 2, IOADDR_TOP_REG_BASE + 0x14);
            nvt_pwm_masked_write(22, 22, 0, IOADDR_TOP_REG_BASE + 0xA0);
		} else if (pwm_pinmux & PIN_PWMIII_CFG_PWM9_3) {
            nvt_pwm_masked_write(29, 27, 3, IOADDR_TOP_REG_BASE + 0x14);
            nvt_pwm_masked_write(7, 7, 0, IOADDR_TOP_REG_BASE + 0xD8);
		} else if (pwm_pinmux & PIN_PWMIII_CFG_PWM9_4) {
			nvt_pwm_masked_write(29, 27, 4, IOADDR_TOP_REG_BASE + 0x14);
            nvt_pwm_masked_write(1, 1, 0, IOADDR_TOP_REG_BASE + 0x100030);
		} else {
			DBG_ERR("!!Caution!!>>using pwm_id(%d) but pwm_pinmux(0x%x) not configured(check nvt-top.dtsi)\n", pwm_id, pwm_pinmux);
		}
	} else if (pwm_id == 10) {
		if (pwm_pinmux & PIN_PWMIII_CFG_PWM10_1) {
			nvt_pwm_masked_write(2, 0, 1, IOADDR_TOP_REG_BASE + 0x18);
			nvt_pwm_masked_write(10, 10, 0, IOADDR_TOP_REG_BASE + 0xA8);
		} else if (pwm_pinmux & PIN_PWMIII_CFG_PWM10_2) {
			nvt_pwm_masked_write(2, 0, 2, IOADDR_TOP_REG_BASE + 0x18);
			nvt_pwm_masked_write(23, 23, 0, IOADDR_TOP_REG_BASE + 0xA0);
		} else if (pwm_pinmux & PIN_PWMIII_CFG_PWM10_3) {
			nvt_pwm_masked_write(2, 0, 3, IOADDR_TOP_REG_BASE + 0x18);
			nvt_pwm_masked_write(8, 8, 0, IOADDR_TOP_REG_BASE + 0xD8);
		} else if (pwm_pinmux & PIN_PWMIII_CFG_PWM10_4) {
			nvt_pwm_masked_write(2, 0, 4, IOADDR_TOP_REG_BASE + 0x18);
			nvt_pwm_masked_write(2, 2, 0, IOADDR_TOP_REG_BASE + 0x100030);
		} else {
			DBG_ERR("!!Caution!!>>using pwm_id(%d) but pwm_pinmux(0x%x) not configured(check nvt-top.dtsi)\n", pwm_id, pwm_pinmux);
		}
	} else if (pwm_id == 11) {
		if (pwm_pinmux & PIN_PWMIII_CFG_PWM11_1) {
			nvt_pwm_masked_write(5, 3, 1, IOADDR_TOP_REG_BASE + 0x18);
			nvt_pwm_masked_write(11, 11, 0, IOADDR_TOP_REG_BASE + 0xA8);
		} else if (pwm_pinmux & PIN_PWMIII_CFG_PWM11_2) {
			nvt_pwm_masked_write(5, 3, 2, IOADDR_TOP_REG_BASE + 0x18);
			nvt_pwm_masked_write(24, 24, 0, IOADDR_TOP_REG_BASE + 0xA0);
		} else if (pwm_pinmux & PIN_PWMIII_CFG_PWM11_3) {
			nvt_pwm_masked_write(5, 3, 3, IOADDR_TOP_REG_BASE + 0x18);
			nvt_pwm_masked_write(9, 9, 0, IOADDR_TOP_REG_BASE + 0xD8);
		} else if (pwm_pinmux & PIN_PWMIII_CFG_PWM11_4) {
			nvt_pwm_masked_write(5, 3, 4, IOADDR_TOP_REG_BASE + 0x18);
			nvt_pwm_masked_write(3, 3, 0, IOADDR_TOP_REG_BASE + 0x100030);
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

    sprintf(path, "/top@f0010000/%s", pwm);

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
		nvt_pwm_masked_write(13, 0, div, IOADDR_CG_REG_BASE + 0x50);
	} else if (pwm_id < 8) {
		nvt_pwm_masked_write(29, 16, div, IOADDR_CG_REG_BASE + 0x50);
	} else if (pwm_id == 8) {
		nvt_pwm_masked_write(13, 0, div, IOADDR_CG_REG_BASE + 0x54);
	} else if (pwm_id == 9) {
		nvt_pwm_masked_write(29, 16, div, IOADDR_CG_REG_BASE + 0x54);
	} else if (pwm_id == 10) {
		nvt_pwm_masked_write(13, 0, div, IOADDR_CG_REG_BASE + 0x58);
	} else if (pwm_id == 11) {
		nvt_pwm_masked_write(29, 16, div, IOADDR_CG_REG_BASE + 0x58);
	} else {
		printf("nvt_pwm_set_div: invalid pwm_id%d\r\n", pwm_id);
	}
}

void pwm_enable_clk(int pwm_id)
{
	/* Enable clk */
	if (pwm_id < 12) {
		nvt_pwm_masked_write(pwm_id, pwm_id, 1, IOADDR_CG_REG_BASE + 0x7C);
		nvt_pwm_masked_write(8, 8, 1, IOADDR_CG_REG_BASE + 0x88);
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