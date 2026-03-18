#include "nvt_pwm.h"

#define NUM_PWM                 12
#define PIN_PWM_CFG_PWM0_1      0x01          ///< Enable PWM0 @P_GPIO0[PWM0_1]
#define PIN_PWM_CFG_PWM0_2      0x02          ///< Enable PWM0 @MC13[PWM0_2]
#define PIN_PWM_CFG_PWM0_3      0x04          ///< Enable PWM0 @S_GPIO1[PWM0_3]
#define PIN_PWM_CFG_PWM0_4      0x08          ///< Enable PWM0 @MC4[PWM0_4]
#define PIN_PWM_CFG_PWM0_5      0x10          ///< Enable PWM0 @DSI_GPIO0[PWM0_5]
#define PIN_PWM_CFG_PWM1_1      0x20          ///< Enable PWM1 @P_GPIO1[PWM1_1]
#define PIN_PWM_CFG_PWM1_2      0x40          ///< Enable PWM1 @MC14[PWM1_2]
#define PIN_PWM_CFG_PWM1_3      0x80          ///< Enable PWM1 @S_GPIO2[PWM1_3]
#define PIN_PWM_CFG_PWM1_4      0x100         ///< Enable PWM1 @MC5[PWM1_4]
#define PIN_PWM_CFG_PWM1_5      0x200         ///< Enable PWM1 @DSI_GPIO1[PWM1_5]
#define PIN_PWM_CFG_PWM2_1      0x400         ///< Enable PWM2 @P_GPIO2[PWM2_1]
#define PIN_PWM_CFG_PWM2_2      0x800         ///< Enable PWM2 @MC15[PWM2_2]
#define PIN_PWM_CFG_PWM2_3      0x1000        ///< Enable PWM2 @S_GPIO3[PWM2_3]
#define PIN_PWM_CFG_PWM2_4      0x2000        ///< Enable PWM2 @MC6[PWM2_4]
#define PIN_PWM_CFG_PWM2_5      0x4000        ///< Enable PWM2 @DSI_GPIO2[PWM2_5]
#define PIN_PWM_CFG_PWM3_1      0x8000        ///< Enable PWM3 @P_GPIO3[PWM3_1]
#define PIN_PWM_CFG_PWM3_2      0x10000       ///< Enable PWM3 @MC16[PWM3_2]
#define PIN_PWM_CFG_PWM3_3      0x20000       ///< Enable PWM3 @S_GPIO4[PWM3_3]
#define PIN_PWM_CFG_PWM3_4      0x40000       ///< Enable PWM3 @MC7[PWM3_4]
#define PIN_PWM_CFG_PWM3_5      0x80000       ///< Enable PWM3 @DSI_GPIO3[PWM3_5]
#define PIN_PWM_CFG_PWM8_1      0x100000      ///< Enable PWM8 @P_GPIO8[PWM8_1]
#define PIN_PWM_CFG_PWM8_2      0x200000      ///< Enable PWM8 @MC19[PWM8_2]
#define PIN_PWM_CFG_PWM8_3      0x400000      ///< Enable PWM8 @HSI_GPIO6[PWM8_3]
#define PIN_PWM_CFG_PWM8_4      0x800000      ///< Enable PWM8 @D_GPIO8[PWM8_4]
#define PIN_PWM_CFG_PWM8_5      0x1000000     ///< Enable PWM8 @DSI_GPIO8[PWM8_5]
#define PIN_PWM_CFG_PWM9_1      0x2000000     ///< Enable PWM9 @P_GPIO9[PWM9_1]
#define PIN_PWM_CFG_PWM9_2      0x4000000     ///< Enable PWM9 @MC20[PWM9_2]
#define PIN_PWM_CFG_PWM9_3      0x8000000     ///< Enable PWM9 @HSI_GPIO7[PWM9_3]
#define PIN_PWM_CFG_PWM9_4      0x10000000    ///< Enable PWM9 @D_GPIO9[PWM9_4]
#define PIN_PWM_CFG_PWM9_5      0x20000000    ///< Enable PWM9 @DSI_GPIO9[PWM9_5]

#define PIN_PWM_CFG2_PWM4_1     0x01          ///< Enable PWM4 @P_GPIO4[PWM4_1]
#define PIN_PWM_CFG2_PWM4_2     0x02          ///< Enable PWM4 @MC11[PWM4_2]
#define PIN_PWM_CFG2_PWM4_3     0x04          ///< Enable PWM4 @S_GPIO5[PWM4_3]
#define PIN_PWM_CFG2_PWM4_4     0x08          ///< Enable PWM4 @D_GPIO3[PWM4_4]
#define PIN_PWM_CFG2_PWM4_5     0x10          ///< Enable PWM4 @DSI_GPIO4[PWM4_5]
#define PIN_PWM_CFG2_PWM5_1     0x20          ///< Enable PWM5 @P_GPIO5[PWM5_1]
#define PIN_PWM_CFG2_PWM5_2     0x40          ///< Enable PWM5 @MC12[PWM5_2]
#define PIN_PWM_CFG2_PWM5_3     0x80          ///< Enable PWM5 @S_GPIO6[PWM5_3]
#define PIN_PWM_CFG2_PWM5_4     0x100         ///< Enable PWM5 @D_GPIO4[PWM5_4]
#define PIN_PWM_CFG2_PWM5_5     0x200         ///< Enable PWM5 @DSI_GPIO5[PWM5_5]
#define PIN_PWM_CFG2_PWM6_1     0x400         ///< Enable PWM6 @P_GPIO6[PWM6_1]
#define PIN_PWM_CFG2_PWM6_2     0x800         ///< Enable PWM6 @MC17[PWM6_2]
#define PIN_PWM_CFG2_PWM6_3     0x1000        ///< Enable PWM6 @S_GPIO7[PWM6_3]
#define PIN_PWM_CFG2_PWM6_4     0x2000        ///< Enable PWM6 @D_GPIO5[PWM6_4]
#define PIN_PWM_CFG2_PWM6_5     0x4000        ///< Enable PWM6 @DSI_GPIO6[PWM6_5]
#define PIN_PWM_CFG2_PWM7_1     0x8000        ///< Enable PWM7 @P_GPIO7[PWM7_1]
#define PIN_PWM_CFG2_PWM7_2     0x10000       ///< Enable PWM7 @MC18[PWM7_2]
#define PIN_PWM_CFG2_PWM7_3     0x20000       ///< Enable PWM7 @S_GPIO8[PWM7_3]
#define PIN_PWM_CFG2_PWM7_4     0x40000       ///< Enable PWM7 @D_GPIO6[PWM7_4]
#define PIN_PWM_CFG2_PWM7_5     0x80000       ///< Enable PWM7 @DSI_GPIO7[PWM7_5]
#define PIN_PWM_CFG2_PWM10_1    0x100000      ///< Enable PWM10 @P_GPIO10[PWM10_1]
#define PIN_PWM_CFG2_PWM10_2    0x200000      ///< Enable PWM10 @MC21[PWM10_2]
#define PIN_PWM_CFG2_PWM10_3    0x400000      ///< Enable PWM10 @HSI_GPIO8[PWM10_3]
#define PIN_PWM_CFG2_PWM10_4    0x800000      ///< Enable PWM10 @D_GPIO10[PWM10_4]
#define PIN_PWM_CFG2_PWM10_5    0x1000000     ///< Enable PWM10 @DSI_GPIO10[PWM10_5]
#define PIN_PWM_CFG2_PWM11_1    0x2000000     ///< Enable PWM11 @P_GPIO11[PWM11_1]
#define PIN_PWM_CFG2_PWM11_2    0x4000000     ///< Enable PWM11 @MC22[PWM11_2]
#define PIN_PWM_CFG2_PWM11_3    0x8000000     ///< Enable PWM11 @HSI_GPIO9[PWM11_3]
#define PIN_PWM_CFG2_PWM11_4    0x10000000    ///< Enable PWM11 @D_GPIO7[PWM11_4]
#define PIN_PWM_CFG2_PWM11_5    0x20000000    ///< Enable PWM11 @L_GPIO0[PWM11_5]

void nvt_pwm_set_pinmux(int pwm_id, u32 pwm_pinmux)
{
	printf("pwm_id(%d) pwm_pinmux(0x%x)\n", pwm_id, pwm_pinmux);

	if (pwm_id == 0) {
		if (pwm_pinmux & PIN_PWM_CFG_PWM0_1) {
			*(u32 *)(IOADDR_TOP_REG_BASE + 0x1C) &= ~(0x7 << (pwm_id * 3));
			*(u32 *)(IOADDR_TOP_REG_BASE + 0x1C) |= (0x1 << (pwm_id * 3));
			*(u32 *)(IOADDR_TOP_REG_BASE + 0xA8) &= ~(0x1 << pwm_id);
		} else if (pwm_pinmux & PIN_PWM_CFG_PWM0_2) {
			*(u32 *)(IOADDR_TOP_REG_BASE + 0x1C) &= ~(0x7 << (pwm_id * 3));
			*(u32 *)(IOADDR_TOP_REG_BASE + 0x1C) |= (0x2 << (pwm_id * 3));
			*(u32 *)(IOADDR_TOP_REG_BASE + 0xA0) &= ~(0x1 << 13);
		} else if (pwm_pinmux & PIN_PWM_CFG_PWM0_3) {
			*(u32 *)(IOADDR_TOP_REG_BASE + 0x1C) &= ~(0x7 << (pwm_id * 3));
			*(u32 *)(IOADDR_TOP_REG_BASE + 0x1C) |= (0x3 << (pwm_id * 3));
			*(u32 *)(IOADDR_TOP_REG_BASE + 0xB0) &= ~(0x1 << 1);
		} else if (pwm_pinmux & PIN_PWM_CFG_PWM0_4) {
			*(u32 *)(IOADDR_TOP_REG_BASE + 0x1C) &= ~(0x7 << (pwm_id * 3));
			*(u32 *)(IOADDR_TOP_REG_BASE + 0x1C) |= (0x4 << (pwm_id * 3));
			*(u32 *)(IOADDR_TOP_REG_BASE + 0xA0) &= ~(0x1 << 4);
		} else if (pwm_pinmux & PIN_PWM_CFG_PWM0_5) {
			*(u32 *)(IOADDR_TOP_REG_BASE + 0x1C) &= ~(0x7 << (pwm_id * 3));
			*(u32 *)(IOADDR_TOP_REG_BASE + 0x1C) |= (0x5 << (pwm_id * 3));
			*(u32 *)(IOADDR_TOP_REG_BASE + 0xB8) &= ~(0x1 << 0);
		} else {
			DBG_ERR("!!Caution!!>>using pwm_id(%d) but pwm_pinmux(0x%x) not configured(check nvt-top.dtsi)\n", pwm_id, pwm_pinmux);
		}
	} else if (pwm_id == 1) {
		if (pwm_pinmux & PIN_PWM_CFG_PWM1_1) {
			*(u32 *)(IOADDR_TOP_REG_BASE + 0x1C) &= ~(0x7 << (pwm_id * 3));
			*(u32 *)(IOADDR_TOP_REG_BASE + 0x1C) |= (0x1 << (pwm_id * 3));
			*(u32 *)(IOADDR_TOP_REG_BASE + 0xA8) &= ~(0x1 << pwm_id);
		} else if (pwm_pinmux & PIN_PWM_CFG_PWM1_2) {
			*(u32 *)(IOADDR_TOP_REG_BASE + 0x1C) &= ~(0x7 << (pwm_id * 3));
			*(u32 *)(IOADDR_TOP_REG_BASE + 0x1C) |= (0x2 << (pwm_id * 3));
			*(u32 *)(IOADDR_TOP_REG_BASE + 0xA0) &= ~(0x1 << 14);
		} else if (pwm_pinmux & PIN_PWM_CFG_PWM1_3) {
			*(u32 *)(IOADDR_TOP_REG_BASE + 0x1C) &= ~(0x7 << (pwm_id * 3));
			*(u32 *)(IOADDR_TOP_REG_BASE + 0x1C) |= (0x3 << (pwm_id * 3));
			*(u32 *)(IOADDR_TOP_REG_BASE + 0xB0) &= ~(0x1 << 2);
		} else if (pwm_pinmux & PIN_PWM_CFG_PWM1_4) {
			*(u32 *)(IOADDR_TOP_REG_BASE + 0x1C) &= ~(0x7 << (pwm_id * 3));
			*(u32 *)(IOADDR_TOP_REG_BASE + 0x1C) |= (0x4 << (pwm_id * 3));
			*(u32 *)(IOADDR_TOP_REG_BASE + 0xA0) &= ~(0x1 << 5);
		} else if (pwm_pinmux & PIN_PWM_CFG_PWM1_5) {
			*(u32 *)(IOADDR_TOP_REG_BASE + 0x1C) &= ~(0x7 << (pwm_id * 3));
			*(u32 *)(IOADDR_TOP_REG_BASE + 0x1C) |= (0x5 << (pwm_id * 3));
			*(u32 *)(IOADDR_TOP_REG_BASE + 0xB8) &= ~(0x1 << 1);
		} else {
			DBG_ERR("!!Caution!!>>using pwm_id(%d) but pwm_pinmux(0x%x) not configured(check nvt-top.dtsi)\n", pwm_id, pwm_pinmux);
		}
	} else if (pwm_id == 2) {
		if (pwm_pinmux & PIN_PWM_CFG_PWM2_1) {
			*(u32 *)(IOADDR_TOP_REG_BASE + 0x1C) &= ~(0x7 << (pwm_id * 3));
			*(u32 *)(IOADDR_TOP_REG_BASE + 0x1C) |= (0x1 << (pwm_id * 3));
			*(u32 *)(IOADDR_TOP_REG_BASE + 0xA8) &= ~(0x1 << pwm_id);
		} else if (pwm_pinmux & PIN_PWM_CFG_PWM2_2) {
			*(u32 *)(IOADDR_TOP_REG_BASE + 0x1C) &= ~(0x7 << (pwm_id * 3));
			*(u32 *)(IOADDR_TOP_REG_BASE + 0x1C) |= (0x2 << (pwm_id * 3));
			*(u32 *)(IOADDR_TOP_REG_BASE + 0xA0) &= ~(0x1 << 15);
		} else if (pwm_pinmux & PIN_PWM_CFG_PWM2_3) {
			*(u32 *)(IOADDR_TOP_REG_BASE + 0x1C) &= ~(0x7 << (pwm_id * 3));
			*(u32 *)(IOADDR_TOP_REG_BASE + 0x1C) |= (0x3 << (pwm_id * 3));
			*(u32 *)(IOADDR_TOP_REG_BASE + 0xB0) &= ~(0x1 << 3);
		} else if (pwm_pinmux & PIN_PWM_CFG_PWM2_4) {
			*(u32 *)(IOADDR_TOP_REG_BASE + 0x1C) &= ~(0x7 << (pwm_id * 3));
			*(u32 *)(IOADDR_TOP_REG_BASE + 0x1C) |= (0x4 << (pwm_id * 3));
			*(u32 *)(IOADDR_TOP_REG_BASE + 0xA0) &= ~(0x1 << 6);
		} else if (pwm_pinmux & PIN_PWM_CFG_PWM2_5) {
			*(u32 *)(IOADDR_TOP_REG_BASE + 0x1C) &= ~(0x7 << (pwm_id * 3));
			*(u32 *)(IOADDR_TOP_REG_BASE + 0x1C) |= (0x5 << (pwm_id * 3));
			*(u32 *)(IOADDR_TOP_REG_BASE + 0xB8) &= ~(0x1 << 2);
		} else {
			DBG_ERR("!!Caution!!>>using pwm_id(%d) but pwm_pinmux(0x%x) not configured(check nvt-top.dtsi)\n", pwm_id, pwm_pinmux);
		}
	} else if (pwm_id == 3) {
		if (pwm_pinmux & PIN_PWM_CFG_PWM3_1) {
			*(u32 *)(IOADDR_TOP_REG_BASE + 0x1C) &= ~(0x7 << (pwm_id * 3));
			*(u32 *)(IOADDR_TOP_REG_BASE + 0x1C) |= (0x1 << (pwm_id * 3));
			*(u32 *)(IOADDR_TOP_REG_BASE + 0xA8) &= ~(0x1 << pwm_id);
		} else if (pwm_pinmux & PIN_PWM_CFG_PWM3_2) {
			*(u32 *)(IOADDR_TOP_REG_BASE + 0x1C) &= ~(0x7 << (pwm_id * 3));
			*(u32 *)(IOADDR_TOP_REG_BASE + 0x1C) |= (0x2 << (pwm_id * 3));
			*(u32 *)(IOADDR_TOP_REG_BASE + 0xA0) &= ~(0x1 << 16);
		} else if (pwm_pinmux & PIN_PWM_CFG_PWM3_3) {
			*(u32 *)(IOADDR_TOP_REG_BASE + 0x1C) &= ~(0x7 << (pwm_id * 3));
			*(u32 *)(IOADDR_TOP_REG_BASE + 0x1C) |= (0x3 << (pwm_id * 3));
			*(u32 *)(IOADDR_TOP_REG_BASE + 0xB0) &= ~(0x1 << 4);
		} else if (pwm_pinmux & PIN_PWM_CFG_PWM3_4) {
			*(u32 *)(IOADDR_TOP_REG_BASE + 0x1C) &= ~(0x7 << (pwm_id * 3));
			*(u32 *)(IOADDR_TOP_REG_BASE + 0x1C) |= (0x4 << (pwm_id * 3));
			*(u32 *)(IOADDR_TOP_REG_BASE + 0xA0) &= ~(0x1 << 7);
		} else if (pwm_pinmux & PIN_PWM_CFG_PWM3_5) {
			*(u32 *)(IOADDR_TOP_REG_BASE + 0x1C) &= ~(0x7 << (pwm_id * 3));
			*(u32 *)(IOADDR_TOP_REG_BASE + 0x1C) |= (0x5 << (pwm_id * 3));
			*(u32 *)(IOADDR_TOP_REG_BASE + 0xB8) &= ~(0x1 << 3);
		} else {
			DBG_ERR("!!Caution!!>>using pwm_id(%d) but pwm_pinmux(0x%x) not configured(check nvt-top.dtsi)\n", pwm_id, pwm_pinmux);
		}
	} else if (pwm_id == 4) {
		if (pwm_pinmux & PIN_PWM_CFG2_PWM4_1) {
			*(u32 *)(IOADDR_TOP_REG_BASE + 0x1C) &= ~(0x7 << (pwm_id * 3));
			*(u32 *)(IOADDR_TOP_REG_BASE + 0x1C) |= (0x1 << (pwm_id * 3));
			*(u32 *)(IOADDR_TOP_REG_BASE + 0xA8) &= ~(0x1 << pwm_id);
		} else if (pwm_pinmux & PIN_PWM_CFG2_PWM4_2) {
			*(u32 *)(IOADDR_TOP_REG_BASE + 0x1C) &= ~(0x7 << (pwm_id * 3));
			*(u32 *)(IOADDR_TOP_REG_BASE + 0x1C) |= (0x2 << (pwm_id * 3));
			*(u32 *)(IOADDR_TOP_REG_BASE + 0xA0) &= ~(0x1 << 11);
		} else if (pwm_pinmux & PIN_PWM_CFG2_PWM4_3) {
			*(u32 *)(IOADDR_TOP_REG_BASE + 0x1C) &= ~(0x7 << (pwm_id * 3));
			*(u32 *)(IOADDR_TOP_REG_BASE + 0x1C) |= (0x3 << (pwm_id * 3));
			*(u32 *)(IOADDR_TOP_REG_BASE + 0xB0) &= ~(0x1 << 5);
		} else if (pwm_pinmux & PIN_PWM_CFG2_PWM4_4) {
			*(u32 *)(IOADDR_TOP_REG_BASE + 0x1C) &= ~(0x7 << (pwm_id * 3));
			*(u32 *)(IOADDR_TOP_REG_BASE + 0x1C) |= (0x4 << (pwm_id * 3));
			*(u32 *)(IOADDR_TOP_REG_BASE + 0xD0) &= ~(0x1 << 3);
		} else if (pwm_pinmux & PIN_PWM_CFG2_PWM4_5) {
			*(u32 *)(IOADDR_TOP_REG_BASE + 0x1C) &= ~(0x7 << (pwm_id * 3));
			*(u32 *)(IOADDR_TOP_REG_BASE + 0x1C) |= (0x5 << (pwm_id * 3));
			*(u32 *)(IOADDR_TOP_REG_BASE + 0xB8) &= ~(0x1 << 4);
		} else {
			DBG_ERR("!!Caution!!>>using pwm_id(%d) but pwm_pinmux(0x%x) not configured(check nvt-top.dtsi)\n", pwm_id, pwm_pinmux);
		}
	} else if (pwm_id == 5) {
		if (pwm_pinmux & PIN_PWM_CFG2_PWM5_1) {
			*(u32 *)(IOADDR_TOP_REG_BASE + 0x1C) &= ~(0x7 << (pwm_id * 3));
			*(u32 *)(IOADDR_TOP_REG_BASE + 0x1C) |= (0x1 << (pwm_id * 3));
			*(u32 *)(IOADDR_TOP_REG_BASE + 0xA8) &= ~(0x1 << pwm_id);
		} else if (pwm_pinmux & PIN_PWM_CFG2_PWM5_2) {
			*(u32 *)(IOADDR_TOP_REG_BASE + 0x1C) &= ~(0x7 << (pwm_id * 3));
			*(u32 *)(IOADDR_TOP_REG_BASE + 0x1C) |= (0x2 << (pwm_id * 3));
			*(u32 *)(IOADDR_TOP_REG_BASE + 0xA0) &= ~(0x1 << 12);
		} else if (pwm_pinmux & PIN_PWM_CFG2_PWM5_3) {
			*(u32 *)(IOADDR_TOP_REG_BASE + 0x1C) &= ~(0x7 << (pwm_id * 3));
			*(u32 *)(IOADDR_TOP_REG_BASE + 0x1C) |= (0x3 << (pwm_id * 3));
			*(u32 *)(IOADDR_TOP_REG_BASE + 0xB0) &= ~(0x1 << 6);
		} else if (pwm_pinmux & PIN_PWM_CFG2_PWM5_4) {
			*(u32 *)(IOADDR_TOP_REG_BASE + 0x1C) &= ~(0x7 << (pwm_id * 3));
			*(u32 *)(IOADDR_TOP_REG_BASE + 0x1C) |= (0x4 << (pwm_id * 3));
			*(u32 *)(IOADDR_TOP_REG_BASE + 0xD0) &= ~(0x1 << 4);
		} else if (pwm_pinmux & PIN_PWM_CFG2_PWM5_5) {
			*(u32 *)(IOADDR_TOP_REG_BASE + 0x1C) &= ~(0x7 << (pwm_id * 3));
			*(u32 *)(IOADDR_TOP_REG_BASE + 0x1C) |= (0x5 << (pwm_id * 3));
			*(u32 *)(IOADDR_TOP_REG_BASE + 0xB8) &= ~(0x1 << 5);
		} else {
			DBG_ERR("!!Caution!!>>using pwm_id(%d) but pwm_pinmux(0x%x) not configured(check nvt-top.dtsi)\n", pwm_id, pwm_pinmux);
		}
	} else if (pwm_id == 6) {
		if (pwm_pinmux & PIN_PWM_CFG2_PWM6_1) {
			*(u32 *)(IOADDR_TOP_REG_BASE + 0x1C) &= ~(0x7 << (pwm_id * 3));
			*(u32 *)(IOADDR_TOP_REG_BASE + 0x1C) |= (0x1 << (pwm_id * 3));
			*(u32 *)(IOADDR_TOP_REG_BASE + 0xA8) &= ~(0x1 << pwm_id);
		} else if (pwm_pinmux & PIN_PWM_CFG2_PWM6_2) {
			*(u32 *)(IOADDR_TOP_REG_BASE + 0x1C) &= ~(0x7 << (pwm_id * 3));
			*(u32 *)(IOADDR_TOP_REG_BASE + 0x1C) |= (0x2 << (pwm_id * 3));
			*(u32 *)(IOADDR_TOP_REG_BASE + 0xA0) &= ~(0x1 << 17);
		} else if (pwm_pinmux & PIN_PWM_CFG2_PWM6_3) {
			*(u32 *)(IOADDR_TOP_REG_BASE + 0x1C) &= ~(0x7 << (pwm_id * 3));
			*(u32 *)(IOADDR_TOP_REG_BASE + 0x1C) |= (0x3 << (pwm_id * 3));
			*(u32 *)(IOADDR_TOP_REG_BASE + 0xB0) &= ~(0x1 << 7);
		} else if (pwm_pinmux & PIN_PWM_CFG2_PWM6_4) {
			*(u32 *)(IOADDR_TOP_REG_BASE + 0x1C) &= ~(0x7 << (pwm_id * 3));
			*(u32 *)(IOADDR_TOP_REG_BASE + 0x1C) |= (0x4 << (pwm_id * 3));
			*(u32 *)(IOADDR_TOP_REG_BASE + 0xD0) &= ~(0x1 << 5);
		} else if (pwm_pinmux & PIN_PWM_CFG2_PWM6_5) {
			*(u32 *)(IOADDR_TOP_REG_BASE + 0x1C) &= ~(0x7 << (pwm_id * 3));
			*(u32 *)(IOADDR_TOP_REG_BASE + 0x1C) |= (0x5 << (pwm_id * 3));
			*(u32 *)(IOADDR_TOP_REG_BASE + 0xB8) &= ~(0x1 << 6);
		} else {
			DBG_ERR("!!Caution!!>>using pwm_id(%d) but pwm_pinmux(0x%x) not configured(check nvt-top.dtsi)\n", pwm_id, pwm_pinmux);
		}
	} else if (pwm_id == 7) {
		if (pwm_pinmux & PIN_PWM_CFG2_PWM7_1) {
			*(u32 *)(IOADDR_TOP_REG_BASE + 0x1C) &= ~(0x7 << (pwm_id * 3));
			*(u32 *)(IOADDR_TOP_REG_BASE + 0x1C) |= (0x1 << (pwm_id * 3));
			*(u32 *)(IOADDR_TOP_REG_BASE + 0xA8) &= ~(0x1 << pwm_id);
		} else if (pwm_pinmux & PIN_PWM_CFG2_PWM7_2) {
			*(u32 *)(IOADDR_TOP_REG_BASE + 0x1C) &= ~(0x7 << (pwm_id * 3));
			*(u32 *)(IOADDR_TOP_REG_BASE + 0x1C) |= (0x2 << (pwm_id * 3));
			*(u32 *)(IOADDR_TOP_REG_BASE + 0xA0) &= ~(0x1 << 18);
		} else if (pwm_pinmux & PIN_PWM_CFG2_PWM7_3) {
			*(u32 *)(IOADDR_TOP_REG_BASE + 0x1C) &= ~(0x7 << (pwm_id * 3));
			*(u32 *)(IOADDR_TOP_REG_BASE + 0x1C) |= (0x3 << (pwm_id * 3));
			*(u32 *)(IOADDR_TOP_REG_BASE + 0xB0) &= ~(0x1 << 8);
		} else if (pwm_pinmux & PIN_PWM_CFG2_PWM7_4) {
			*(u32 *)(IOADDR_TOP_REG_BASE + 0x1C) &= ~(0x7 << (pwm_id * 3));
			*(u32 *)(IOADDR_TOP_REG_BASE + 0x1C) |= (0x4 << (pwm_id * 3));
			*(u32 *)(IOADDR_TOP_REG_BASE + 0xD0) &= ~(0x1 << 6);
		} else if (pwm_pinmux & PIN_PWM_CFG2_PWM7_5) {
			*(u32 *)(IOADDR_TOP_REG_BASE + 0x1C) &= ~(0x7 << (pwm_id * 3));
			*(u32 *)(IOADDR_TOP_REG_BASE + 0x1C) |= (0x5 << (pwm_id * 3));
			*(u32 *)(IOADDR_TOP_REG_BASE + 0xB8) &= ~(0x1 << 7);
		} else {
			DBG_ERR("!!Caution!!>>using pwm_id(%d) but pwm_pinmux(0x%x) not configured(check nvt-top.dtsi)\n", pwm_id, pwm_pinmux);
		}
	} else if (pwm_id == 8) {
		if (pwm_pinmux & PIN_PWM_CFG_PWM8_1) {
			*(u32 *)(IOADDR_TOP_REG_BASE + 0x18) &= ~(0x7 << ((pwm_id - 8) * 3 + 16));
			*(u32 *)(IOADDR_TOP_REG_BASE + 0x18) |= (0x1 << ((pwm_id - 8) * 3 + 16));
			*(u32 *)(IOADDR_TOP_REG_BASE + 0xA8) &= ~(0x1 << pwm_id);
		} else if (pwm_pinmux & PIN_PWM_CFG_PWM8_2) {
			*(u32 *)(IOADDR_TOP_REG_BASE + 0x18) &= ~(0x7 << ((pwm_id - 8) * 3 + 16));
			*(u32 *)(IOADDR_TOP_REG_BASE + 0x18) |= (0x2 << ((pwm_id - 8) * 3 + 16));
			*(u32 *)(IOADDR_TOP_REG_BASE + 0xA0) &= ~(0x1 << 19);
		} else if (pwm_pinmux & PIN_PWM_CFG_PWM8_3) {
			*(u32 *)(IOADDR_TOP_REG_BASE + 0x18) &= ~(0x7 << ((pwm_id - 8) * 3 + 16));
			*(u32 *)(IOADDR_TOP_REG_BASE + 0x18) |= (0x3 << ((pwm_id - 8) * 3 + 16));
			*(u32 *)(IOADDR_TOP_REG_BASE + 0xD8) &= ~(0x1 << 6);
		} else if (pwm_pinmux & PIN_PWM_CFG_PWM8_4) {
			*(u32 *)(IOADDR_TOP_REG_BASE + 0x18) &= ~(0x7 << ((pwm_id - 8) * 3 + 16));
			*(u32 *)(IOADDR_TOP_REG_BASE + 0x18) |= (0x4 << ((pwm_id - 8) * 3 + 16));
			*(u32 *)(IOADDR_TOP_REG_BASE + 0xD0) &= ~(0x1 << 8);
		} else if (pwm_pinmux & PIN_PWM_CFG_PWM8_5) {
			*(u32 *)(IOADDR_TOP_REG_BASE + 0x18) &= ~(0x7 << ((pwm_id - 8) * 3 + 16));
			*(u32 *)(IOADDR_TOP_REG_BASE + 0x18) |= (0x5 << ((pwm_id - 8) * 3 + 16));
			*(u32 *)(IOADDR_TOP_REG_BASE + 0xB8) &= ~(0x1 << 8);
		} else {
			DBG_ERR("!!Caution!!>>using pwm_id(%d) but pwm_pinmux(0x%x) not configured(check nvt-top.dtsi)\n", pwm_id, pwm_pinmux);
		}
	} else if (pwm_id == 9) {
		if (pwm_pinmux & PIN_PWM_CFG_PWM9_1) {
			*(u32 *)(IOADDR_TOP_REG_BASE + 0x18) &= ~(0x7 << ((pwm_id - 8) * 3 + 16));
			*(u32 *)(IOADDR_TOP_REG_BASE + 0x18) |= (0x1 << ((pwm_id - 8) * 3 + 16));
			*(u32 *)(IOADDR_TOP_REG_BASE + 0xA8) &= ~(0x1 << pwm_id);
		} else if (pwm_pinmux & PIN_PWM_CFG_PWM9_2) {
			*(u32 *)(IOADDR_TOP_REG_BASE + 0x18) &= ~(0x7 << ((pwm_id - 8) * 3 + 16));
			*(u32 *)(IOADDR_TOP_REG_BASE + 0x18) |= (0x2 << ((pwm_id - 8) * 3 + 16));
			*(u32 *)(IOADDR_TOP_REG_BASE + 0xA0) &= ~(0x1 << 20);
		} else if (pwm_pinmux & PIN_PWM_CFG_PWM9_3) {
			*(u32 *)(IOADDR_TOP_REG_BASE + 0x18) &= ~(0x7 << ((pwm_id - 8) * 3 + 16));
			*(u32 *)(IOADDR_TOP_REG_BASE + 0x18) |= (0x3 << ((pwm_id - 8) * 3 + 16));
			*(u32 *)(IOADDR_TOP_REG_BASE + 0xD8) &= ~(0x1 << 7);
		} else if (pwm_pinmux & PIN_PWM_CFG_PWM9_4) {
			*(u32 *)(IOADDR_TOP_REG_BASE + 0x18) &= ~(0x7 << ((pwm_id - 8) * 3 + 16));
			*(u32 *)(IOADDR_TOP_REG_BASE + 0x18) |= (0x4 << ((pwm_id - 8) * 3 + 16));
			*(u32 *)(IOADDR_TOP_REG_BASE + 0xD0) &= ~(0x1 << 9);
		} else if (pwm_pinmux & PIN_PWM_CFG_PWM9_5) {
			*(u32 *)(IOADDR_TOP_REG_BASE + 0x18) &= ~(0x7 << ((pwm_id - 8) * 3 + 16));
			*(u32 *)(IOADDR_TOP_REG_BASE + 0x18) |= (0x5 << ((pwm_id - 8) * 3 + 16));
			*(u32 *)(IOADDR_TOP_REG_BASE + 0xB8) &= ~(0x1 << 9);
		} else {
			DBG_ERR("!!Caution!!>>using pwm_id(%d) but pwm_pinmux(0x%x) not configured(check nvt-top.dtsi)\n", pwm_id, pwm_pinmux);
		}
	} else if (pwm_id == 10) {
		if (pwm_pinmux & PIN_PWM_CFG2_PWM10_1) {
			*(u32 *)(IOADDR_TOP_REG_BASE + 0x18) &= ~(0x7 << ((pwm_id - 8) * 3 + 16));
			*(u32 *)(IOADDR_TOP_REG_BASE + 0x18) |= (0x1 << ((pwm_id - 8) * 3 + 16));
			*(u32 *)(IOADDR_TOP_REG_BASE + 0xA8) &= ~(0x1 << pwm_id);
		} else if (pwm_pinmux & PIN_PWM_CFG2_PWM10_2) {
			*(u32 *)(IOADDR_TOP_REG_BASE + 0x18) &= ~(0x7 << ((pwm_id - 8) * 3 + 16));
			*(u32 *)(IOADDR_TOP_REG_BASE + 0x18) |= (0x2 << ((pwm_id - 8) * 3 + 16));
			*(u32 *)(IOADDR_TOP_REG_BASE + 0xA0) &= ~(0x1 << 21);
		} else if (pwm_pinmux & PIN_PWM_CFG2_PWM10_3) {
			*(u32 *)(IOADDR_TOP_REG_BASE + 0x18) &= ~(0x7 << ((pwm_id - 8) * 3 + 16));
			*(u32 *)(IOADDR_TOP_REG_BASE + 0x18) |= (0x3 << ((pwm_id - 8) * 3 + 16));
			*(u32 *)(IOADDR_TOP_REG_BASE + 0xD8) &= ~(0x1 << 8);
		} else if (pwm_pinmux & PIN_PWM_CFG2_PWM10_4) {
			*(u32 *)(IOADDR_TOP_REG_BASE + 0x18) &= ~(0x7 << ((pwm_id - 8) * 3 + 16));
			*(u32 *)(IOADDR_TOP_REG_BASE + 0x18) |= (0x4 << ((pwm_id - 8) * 3 + 16));
			*(u32 *)(IOADDR_TOP_REG_BASE + 0xD0) &= ~(0x1 << 10);
		} else if (pwm_pinmux & PIN_PWM_CFG2_PWM10_5) {
			*(u32 *)(IOADDR_TOP_REG_BASE + 0x18) &= ~(0x7 << ((pwm_id - 8) * 3 + 16));
			*(u32 *)(IOADDR_TOP_REG_BASE + 0x18) |= (0x5 << ((pwm_id - 8) * 3 + 16));
			*(u32 *)(IOADDR_TOP_REG_BASE + 0xB8) &= ~(0x1 << 10);
		} else {
			DBG_ERR("!!Caution!!>>using pwm_id(%d) but pwm_pinmux(0x%x) not configured(check nvt-top.dtsi)\n", pwm_id, pwm_pinmux);
		}
	} else if (pwm_id == 11) {
		if (pwm_pinmux & PIN_PWM_CFG2_PWM11_1) {
			*(u32 *)(IOADDR_TOP_REG_BASE + 0x18) &= ~(0x7 << ((pwm_id - 8) * 3 + 16));
			*(u32 *)(IOADDR_TOP_REG_BASE + 0x18) |= (0x1 << ((pwm_id - 8) * 3 + 16));
			*(u32 *)(IOADDR_TOP_REG_BASE + 0xA8) &= ~(0x1 << pwm_id);
		} else if (pwm_pinmux & PIN_PWM_CFG2_PWM11_2) {
			*(u32 *)(IOADDR_TOP_REG_BASE + 0x18) &= ~(0x7 << ((pwm_id - 8) * 3 + 16));
			*(u32 *)(IOADDR_TOP_REG_BASE + 0x18) |= (0x2 << ((pwm_id - 8) * 3 + 16));
			*(u32 *)(IOADDR_TOP_REG_BASE + 0xA0) &= ~(0x1 << 22);
		} else if (pwm_pinmux & PIN_PWM_CFG2_PWM11_3) {
			*(u32 *)(IOADDR_TOP_REG_BASE + 0x18) &= ~(0x7 << ((pwm_id - 8) * 3 + 16));
			*(u32 *)(IOADDR_TOP_REG_BASE + 0x18) |= (0x3 << ((pwm_id - 8) * 3 + 16));
			*(u32 *)(IOADDR_TOP_REG_BASE + 0xD8) &= ~(0x1 << 9);
		} else if (pwm_pinmux & PIN_PWM_CFG2_PWM11_4) {
			*(u32 *)(IOADDR_TOP_REG_BASE + 0x18) &= ~(0x7 << ((pwm_id - 8) * 3 + 16));
			*(u32 *)(IOADDR_TOP_REG_BASE + 0x18) |= (0x4 << ((pwm_id - 8) * 3 + 16));
			*(u32 *)(IOADDR_TOP_REG_BASE + 0xD0) &= ~(0x1 << 7);
		} else if (pwm_pinmux & PIN_PWM_CFG2_PWM11_5) {
			*(u32 *)(IOADDR_TOP_REG_BASE + 0x18) &= ~(0x7 << ((pwm_id - 8) * 3 + 16));
			*(u32 *)(IOADDR_TOP_REG_BASE + 0x18) |= (0x5 << ((pwm_id - 8) * 3 + 16));
			*(u32 *)(IOADDR_TOP_REG_BASE + 0xB8) &= ~(0x1 << 0);
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
	if (pwm_id < 4) {
		*(u32 *)(IOADDR_CG_REG_BASE + 0x50) &= ~(0x3FFF);
		*(u32 *)(IOADDR_CG_REG_BASE + 0x50) |= (div & 0x3FFF);
	} else if (pwm_id < 8) {
		*(u32 *)(IOADDR_CG_REG_BASE + 0x50) &= ~(0x3FFF << 16);
		*(u32 *)(IOADDR_CG_REG_BASE + 0x50) |= ((div << 16) & (0x3FFF << 16));
	} else if (pwm_id == 8) {
		*(u32 *)(IOADDR_CG_REG_BASE + 0x54) &= ~(0x3FFF);
		*(u32 *)(IOADDR_CG_REG_BASE + 0x54) |= (div & 0x3FFF);
	} else if (pwm_id == 9) {
		*(u32 *)(IOADDR_CG_REG_BASE + 0x54) &= ~(0x3FFF << 16);
		*(u32 *)(IOADDR_CG_REG_BASE + 0x54) |= ((div << 16) & (0x3FFF << 16));
	} else if (pwm_id == 10) {
		*(u32 *)(IOADDR_CG_REG_BASE + 0x58) &= ~(0x3FFF);
		*(u32 *)(IOADDR_CG_REG_BASE + 0x58) |= (div & 0x3FFF);
	} else if (pwm_id == 11) {
		*(u32 *)(IOADDR_CG_REG_BASE + 0x58) &= ~(0x3FFF << 16);
		*(u32 *)(IOADDR_CG_REG_BASE + 0x58) |= ((div << 16) & (0x3FFF << 16));
	}
}

void pwm_enable_clk(int pwm_id)
{
	/* Enable clk */
	*(u32 *)(IOADDR_CG_REG_BASE + 0x7C) |= (0x1 << pwm_id);
	*(u32 *)(IOADDR_CG_REG_BASE + 0x88) |= (0x1 << 8);
}

bool pwmid_is_valid(int pwm_id)
{
    if (pwm_id < 0 || pwm_id >= NUM_PWM) {
        return false;
    }

    return true;
}