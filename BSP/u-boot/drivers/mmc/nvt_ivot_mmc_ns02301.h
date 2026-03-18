/*
 *  driver/mmc/nvt_ivot_mmc_ns02301.h
 *
 *  Copyright:	Novatek Inc.
 *
 */

#ifndef __NVT_IVOT_MMC_NS02301_H__
#define __NVT_IVOT_MMC_NS02301_H__

#include <asm/arch/ns02301_regs.h>
#include <asm/arch/gpio.h>
#include <linux/bitops.h>

#define SDIO_32BIT

/**************************
 *        Platform        *
 **************************/
/* SDIO controller number */
#define SDIO_HOST_ID_COUNT              (3)

/* SRCCLK */
#ifdef CONFIG_NVT_FPGA_EMULATION
#define SRCCLK_192MHZ                   (12000000)
#define SRCCLK_480MHZ                   (48000000)
#define SRCCLK_320MHZ                   (24000000)
#else
#define SRCCLK_192MHZ                   (192000000)
#define SRCCLK_480MHZ                   (480000000)
#define SRCCLK_320MHZ                   (320000000)
#endif

/* CLKSEL */
#define CG_SDIO_CLKSEL_REG              (IOADDR_CG_REG_BASE + 0x20)
#define CG_SDIO_CLKSEL_MASK             GENMASK(5, 4)
#define CG_SDIO_CLKSEL_SHIFT            (4)

#define CG_SDIO2_CLKSEL_REG             (IOADDR_CG_REG_BASE + 0x20)
#define CG_SDIO2_CLKSEL_MASK            GENMASK(9, 8)
#define CG_SDIO2_CLKSEL_SHIFT           (8)

#define CG_SDIO3_CLKSEL_REG             (IOADDR_CG_REG_BASE + 0x24)
#define CG_SDIO3_CLKSEL_MASK            GENMASK(1, 0)
#define CG_SDIO3_CLKSEL_SHIFT           (0)

#define _CLKSEL_480MHZ                  (0x0)
#define _CLKSEL_192MHZ                  (0x1)
#define _CLKSEL_320MHZ                  (0x2)

/* CLKDIV */
#define CG_SDIO_CLKDIV_REG              (IOADDR_CG_REG_BASE + 0x3C)
#define CG_SDIO_CLKDIV_MASK             GENMASK(10, 0)
#define CG_SDIO_CLKDIV_SHIFT            (0)

#define CG_SDIO2_CLKDIV_REG             (IOADDR_CG_REG_BASE + 0x3C)
#define CG_SDIO2_CLKDIV_MASK            GENMASK(26, 16)
#define CG_SDIO2_CLKDIV_SHIFT           (16)

#define CG_SDIO3_CLKDIV_REG             (IOADDR_CG_REG_BASE + 0x40)
#define CG_SDIO3_CLKDIV_MASK            GENMASK(10, 0)
#define CG_SDIO3_CLKDIV_SHIFT           (0)

/* CLKEN */
#define CG_SDIO_CLKEN_REG               (IOADDR_CG_REG_BASE + 0x74)
#define CG_SDIO_CLKEN_MASK              BIT(2)

#define CG_SDIO2_CLKEN_REG              (IOADDR_CG_REG_BASE + 0x74)
#define CG_SDIO2_CLKEN_MASK             BIT(3)

#define CG_SDIO3_CLKEN_REG              (IOADDR_CG_REG_BASE + 0x74)
#define CG_SDIO3_CLKEN_MASK             BIT(14)

/* RSTN */
#define CG_SDIO_RSTN_REG                (IOADDR_CG_REG_BASE + 0x84)
#define CG_SDIO_RSTN_MASK               BIT(2)

#define CG_SDIO2_RSTN_REG               (IOADDR_CG_REG_BASE + 0x84)
#define CG_SDIO2_RSTN_MASK              BIT(3)

// must remember this may influence spi
#define CG_SDIO3_RSTN_REG               (IOADDR_CG_REG_BASE + 0x84)
#define CG_SDIO3_RSTN_MASK              BIT(8)

/* PINMUX */
#define PIN_SDIO_CFG_SDIO_1             (0x1)

#define PIN_SDIO_CFG_SDIO2_1            (0x10)
#define PIN_SDIO_CFG_SDIO2_2            (0x20)

#define PIN_SDIO_CFG_SDIO3_1            (0x100)
#define PIN_SDIO_CFG_SDIO3_BUS_WIDTH    (0x200)
#define PIN_SDIO_CFG_SDIO3_DS           (0x400)

/* TOP */
#define TOP_SDIO_EN_REG                 (IOADDR_TOP_REG_BASE + 0x04)
#define TOP_SDIO_EN_MASK                BIT(16)
#define TOP_SDIO_EN_GPIO_REG            (IOADDR_TOP_REG_BASE + 0xA0)
#define TOP_SDIO_EN_GPIO_MASK           BIT(13) | BIT(14) | BIT(15) | BIT(16) | BIT(17) | BIT(18)

// 2-1
#define TOP_SDIO2_EN_REG                (IOADDR_TOP_REG_BASE + 0x04)
#define TOP_SDIO2_EN_MASK               BIT(18)
#define TOP_SDIO2_EN_GPIO_REG           (IOADDR_TOP_REG_BASE + 0xA0)
#define TOP_SDIO2_EN_GPIO_MASK          BIT(19) | BIT(20) | BIT(21) | BIT(22) | BIT(23) | BIT(24)

// 2-2
// #define TOP_SDIO2_EN_REG             (IOADDR_TOP_REG_BASE + 0x04)
#define TOP_SDIO2_2_EN_MASK             BIT(19)
#define TOP_SDIO2_2_EN_GPIO_REG         (IOADDR_TOP_REG_BASE + 0xA8)
#define TOP_SDIO2_2_EN_GPIO_MASK        BIT(2) | BIT(3) | BIT(4) | BIT(5) | BIT(6) | BIT(7)

#define TOP_SDIO3_EN_REG                (IOADDR_TOP_REG_BASE + 0x04)
#define TOP_SDIO3_EN_MASK               BIT(20)
#define TOP_SDIO3_EN_GPIO_REG           (IOADDR_TOP_REG_BASE + 0xA0)
#define TOP_SDIO3_EN_GPIO_MASK          BIT(10) | BIT(11) | BIT(0) | BIT(1) | BIT(2) | BIT(3)

#define TOP_SDIO3_WIDTH_REG             (IOADDR_TOP_REG_BASE + 0x04)
#define TOP_SDIO3_WIDTH_MASK            BIT(23)
#define TOP_SDIO3_WIDTH_GPIO_REG        (IOADDR_TOP_REG_BASE + 0xA0)
#define TOP_SDIO3_WIDTH_GPIO_MASK       BIT(4) | BIT(5) | BIT(6) | BIT(7)

/* PAD */
#define PAD_PUPD0_REG_OFS               (0x00)

#define PAD_REG_TO_BASE(reg)            (((reg)/(4))*(32))
#define	PAD_DS_CGPIO_BASE               PAD_REG_TO_BASE(0x24)
#define	PAD_DS_CGPIO_BASE2              PAD_REG_TO_BASE(0x28)
#define	PAD_DS_CGPIO_BASE3              PAD_REG_TO_BASE(0x44)

#define PAD_DS_CGPIO0                   (PAD_DS_CGPIO_BASE + 0)
#define PAD_DS_CGPIO1                   (PAD_DS_CGPIO_BASE + 2)
#define PAD_DS_CGPIO2                   (PAD_DS_CGPIO_BASE + 4)
#define PAD_DS_CGPIO3                   (PAD_DS_CGPIO_BASE + 6)
#define PAD_DS_CGPIO4                   (PAD_DS_CGPIO_BASE + 8)
#define PAD_DS_CGPIO5                   (PAD_DS_CGPIO_BASE + 10)
#define PAD_DS_CGPIO6                   (PAD_DS_CGPIO_BASE + 12)
#define PAD_DS_CGPIO7                   (PAD_DS_CGPIO_BASE + 14)
#define PAD_DS_CGPIO8                   (PAD_DS_CGPIO_BASE + 16)
#define PAD_DS_CGPIO9                   (PAD_DS_CGPIO_BASE + 18)
#define PAD_DS_CGPIO10                  (PAD_DS_CGPIO_BASE + 20)
#define PAD_DS_CGPIO11                  (PAD_DS_CGPIO_BASE + 22)
#define PAD_DS_CGPIO12                  (PAD_DS_CGPIO_BASE + 24)

#define PAD_DS_CGPIO20                  (PAD_DS_CGPIO_BASE2 + 8)
#define PAD_DS_CGPIO21                  (PAD_DS_CGPIO_BASE2 + 10)
#define PAD_DS_CGPIO22                  (PAD_DS_CGPIO_BASE2 + 12)
#define PAD_DS_CGPIO23                  (PAD_DS_CGPIO_BASE2 + 14)
#define PAD_DS_CGPIO24                  (PAD_DS_CGPIO_BASE2 + 16)

#define PAD_DS_CGPIO13                  (PAD_DS_CGPIO_BASE3 + 0)
#define PAD_DS_CGPIO14                  (PAD_DS_CGPIO_BASE3 + 4)
#define PAD_DS_CGPIO15                  (PAD_DS_CGPIO_BASE3 + 8)
#define PAD_DS_CGPIO16                  (PAD_DS_CGPIO_BASE3 + 12)
#define PAD_DS_CGPIO17                  (PAD_DS_CGPIO_BASE3 + 16)
#define PAD_DS_CGPIO18                  (PAD_DS_CGPIO_BASE3 + 20)
#define PAD_DS_CGPIO19                  (PAD_DS_CGPIO_BASE3 + 24)

#define PAD_DS_SDIO_CLK                 PAD_DS_CGPIO13
#define PAD_DS_SDIO_CMD                 PAD_DS_CGPIO14
#define PAD_DS_SDIO_D0                  PAD_DS_CGPIO15
#define PAD_DS_SDIO_D1                  PAD_DS_CGPIO16
#define PAD_DS_SDIO_D2                  PAD_DS_CGPIO17
#define PAD_DS_SDIO_D3                  PAD_DS_CGPIO18

#define PAD_DS_SDIO2_CLK                PAD_DS_CGPIO19
#define PAD_DS_SDIO2_CMD                PAD_DS_CGPIO20
#define PAD_DS_SDIO2_D0                 PAD_DS_CGPIO21
#define PAD_DS_SDIO2_D1                 PAD_DS_CGPIO22
#define PAD_DS_SDIO2_D2                 PAD_DS_CGPIO23
#define PAD_DS_SDIO2_D3                 PAD_DS_CGPIO24

#define PAD_DS_SDIO3_CLK                PAD_DS_CGPIO10
#define PAD_DS_SDIO3_CMD                PAD_DS_CGPIO11
#define PAD_DS_SDIO3_D0                 PAD_DS_CGPIO0
#define PAD_DS_SDIO3_D1                 PAD_DS_CGPIO1
#define PAD_DS_SDIO3_D2                 PAD_DS_CGPIO2
#define PAD_DS_SDIO3_D3                 PAD_DS_CGPIO3
#define PAD_DS_SDIO3_D4                 PAD_DS_CGPIO4
#define PAD_DS_SDIO3_D5                 PAD_DS_CGPIO5
#define PAD_DS_SDIO3_D6                 PAD_DS_CGPIO6
#define PAD_DS_SDIO3_D7                 PAD_DS_CGPIO7

/* Controlled as GPIO */
#define GPIO_SDIO_CLK                   C_GPIO(13)
#define GPIO_SDIO_CMD                   C_GPIO(14)
#define GPIO_SDIO_D0                    C_GPIO(15)
#define GPIO_SDIO_D1                    C_GPIO(16)
#define GPIO_SDIO_D2                    C_GPIO(17)
#define GPIO_SDIO_D3                    C_GPIO(18)

#define GPIO_SDIO2_CLK                  C_GPIO(19)
#define GPIO_SDIO2_CMD                  C_GPIO(20)
#define GPIO_SDIO2_D0                   C_GPIO(21)
#define GPIO_SDIO2_D1                   C_GPIO(22)
#define GPIO_SDIO2_D2                   C_GPIO(23)
#define GPIO_SDIO2_D3                   C_GPIO(24)

#define GPIO_SDIO2_2_CLK                P_GPIO(7)
#define GPIO_SDIO2_2_CMD                P_GPIO(6)
#define GPIO_SDIO2_2_D0                 P_GPIO(2)
#define GPIO_SDIO2_2_D1                 P_GPIO(3)
#define GPIO_SDIO2_2_D2                 P_GPIO(4)
#define GPIO_SDIO2_2_D3                 P_GPIO(5)

#define GPIO_SDIO3_CLK                  C_GPIO(10)
#define GPIO_SDIO3_CMD                  C_GPIO(11)
#define GPIO_SDIO3_D0                   C_GPIO(0)
#define GPIO_SDIO3_D1                   C_GPIO(1)
#define GPIO_SDIO3_D2                   C_GPIO(2)
#define GPIO_SDIO3_D3                   C_GPIO(3)
#define GPIO_SDIO3_D4                   C_GPIO(4)
#define GPIO_SDIO3_D5                   C_GPIO(5)
#define GPIO_SDIO3_D6                   C_GPIO(6)
#define GPIO_SDIO3_D7                   C_GPIO(7)

/* PAD pull */
#define PAD_PULLNONE                    (0x0)
#define PAD_PULLDOWN                    (0x1)
#define PAD_PULLUP                      (0x2)

#define PAD_PUPD_SDIO_CLK_REG           (IOADDR_PAD_REG_BASE + 0x00)
#define PAD_PUPD_SDIO_CLK_MASK          GENMASK(23, 22)
#define PAD_PUPD_SDIO_CLK_SHIFT         (22)
#define PAD_PUPD_SDIO_CMD_REG           (IOADDR_PAD_REG_BASE + 0x00)
#define PAD_PUPD_SDIO_CMD_MASK          GENMASK(25, 24)
#define PAD_PUPD_SDIO_CMD_SHIFT         (24)
#define PAD_PUPD_SDIO_D0_REG            (IOADDR_PAD_REG_BASE + 0x00)
#define PAD_PUPD_SDIO_D0_MASK           GENMASK(27, 26)
#define PAD_PUPD_SDIO_D0_SHIFT          (26)
#define PAD_PUPD_SDIO_D1_REG            (IOADDR_PAD_REG_BASE + 0x00)
#define PAD_PUPD_SDIO_D1_MASK           GENMASK(29, 28)
#define PAD_PUPD_SDIO_D1_SHIFT          (28)
#define PAD_PUPD_SDIO_D2_REG            (IOADDR_PAD_REG_BASE + 0x00)
#define PAD_PUPD_SDIO_D2_MASK           GENMASK(31, 30)
#define PAD_PUPD_SDIO_D2_SHIFT          (30)
#define PAD_PUPD_SDIO_D3_REG            (IOADDR_PAD_REG_BASE + 0x04)
#define PAD_PUPD_SDIO_D3_MASK           GENMASK(1, 0)
#define PAD_PUPD_SDIO_D3_SHIFT          (0)

#define PAD_PUPD_SDIO2_CLK_REG          (IOADDR_PAD_REG_BASE + 0x04)
#define PAD_PUPD_SDIO2_CLK_MASK         GENMASK(3, 2)
#define PAD_PUPD_SDIO2_CLK_SHIFT        (2)
#define PAD_PUPD_SDIO2_CMD_REG          (IOADDR_PAD_REG_BASE + 0x04)
#define PAD_PUPD_SDIO2_CMD_MASK         GENMASK(5, 4)
#define PAD_PUPD_SDIO2_CMD_SHIFT        (4)
#define PAD_PUPD_SDIO2_D0_REG           (IOADDR_PAD_REG_BASE + 0x04)
#define PAD_PUPD_SDIO2_D0_MASK          GENMASK(7, 6)
#define PAD_PUPD_SDIO2_D0_SHIFT         (6)
#define PAD_PUPD_SDIO2_D1_REG           (IOADDR_PAD_REG_BASE + 0x04)
#define PAD_PUPD_SDIO2_D1_MASK          GENMASK(9, 8)
#define PAD_PUPD_SDIO2_D1_SHIFT         (8)
#define PAD_PUPD_SDIO2_D2_REG           (IOADDR_PAD_REG_BASE + 0x04)
#define PAD_PUPD_SDIO2_D2_MASK          GENMASK(11, 10)
#define PAD_PUPD_SDIO2_D2_SHIFT         (10)
#define PAD_PUPD_SDIO2_D3_REG           (IOADDR_PAD_REG_BASE + 0x04)
#define PAD_PUPD_SDIO2_D3_MASK          GENMASK(13, 12)
#define PAD_PUPD_SDIO2_D3_SHIFT         (12)

#define PAD_PUPD_SDIO2_2_CLK_REG        (IOADDR_PAD_REG_BASE + 0x08)
#define PAD_PUPD_SDIO2_2_CLK_MASK       GENMASK(15, 14)
#define PAD_PUPD_SDIO2_2_CLK_SHIFT      (14)
#define PAD_PUPD_SDIO2_2_CMD_REG        (IOADDR_PAD_REG_BASE + 0x08)
#define PAD_PUPD_SDIO2_2_CMD_MASK       GENMASK(17, 16)
#define PAD_PUPD_SDIO2_2_CMD_SHIFT      (16)
#define PAD_PUPD_SDIO2_2_D0_REG         (IOADDR_PAD_REG_BASE + 0x08)
#define PAD_PUPD_SDIO2_2_D0_MASK        GENMASK(5, 4)
#define PAD_PUPD_SDIO2_2_D0_SHIFT       (4)
#define PAD_PUPD_SDIO2_2_D1_REG         (IOADDR_PAD_REG_BASE + 0x08)
#define PAD_PUPD_SDIO2_2_D1_MASK        GENMASK(7, 6)
#define PAD_PUPD_SDIO2_2_D1_SHIFT       (6)
#define PAD_PUPD_SDIO2_2_D2_REG         (IOADDR_PAD_REG_BASE + 0x08)
#define PAD_PUPD_SDIO2_2_D2_MASK        GENMASK(9, 8)
#define PAD_PUPD_SDIO2_2_D2_SHIFT       (8)
#define PAD_PUPD_SDIO2_2_D3_REG         (IOADDR_PAD_REG_BASE + 0x08)
#define PAD_PUPD_SDIO2_2_D3_MASK        GENMASK(11, 10)
#define PAD_PUPD_SDIO2_2_D3_SHIFT       (10)

#define PAD_PUPD_SDIO3_CLK_REG          (IOADDR_PAD_REG_BASE + 0x00)
#define PAD_PUPD_SDIO3_CLK_MASK         GENMASK(21, 20)
#define PAD_PUPD_SDIO3_CLK_SHIFT        (20)
#define PAD_PUPD_SDIO3_CMD_REG          (IOADDR_PAD_REG_BASE + 0x00)
#define PAD_PUPD_SDIO3_CMD_MASK         GENMASK(23, 22)
#define PAD_PUPD_SDIO3_CMD_SHIFT        (22)
#define PAD_PUPD_SDIO3_D0_REG           (IOADDR_PAD_REG_BASE + 0x00)
#define PAD_PUPD_SDIO3_D0_MASK          GENMASK(1, 0)
#define PAD_PUPD_SDIO3_D0_SHIFT         (0)
#define PAD_PUPD_SDIO3_D1_REG           (IOADDR_PAD_REG_BASE + 0x00)
#define PAD_PUPD_SDIO3_D1_MASK          GENMASK(3, 2)
#define PAD_PUPD_SDIO3_D1_SHIFT         (2)
#define PAD_PUPD_SDIO3_D2_REG           (IOADDR_PAD_REG_BASE + 0x00)
#define PAD_PUPD_SDIO3_D2_MASK          GENMASK(5, 4)
#define PAD_PUPD_SDIO3_D2_SHIFT         (4)
#define PAD_PUPD_SDIO3_D3_REG           (IOADDR_PAD_REG_BASE + 0x00)
#define PAD_PUPD_SDIO3_D3_MASK          GENMASK(7, 6)
#define PAD_PUPD_SDIO3_D3_SHIFT         (6)
#define PAD_PUPD_SDIO3_D4_REG           (IOADDR_PAD_REG_BASE + 0x00)
#define PAD_PUPD_SDIO3_D4_MASK          GENMASK(9, 8)
#define PAD_PUPD_SDIO3_D4_SHIFT         (8)
#define PAD_PUPD_SDIO3_D5_REG           (IOADDR_PAD_REG_BASE + 0x00)
#define PAD_PUPD_SDIO3_D5_MASK          GENMASK(11, 10)
#define PAD_PUPD_SDIO3_D5_SHIFT         (10)
#define PAD_PUPD_SDIO3_D6_REG           (IOADDR_PAD_REG_BASE + 0x04)
#define PAD_PUPD_SDIO3_D6_MASK          GENMASK(13, 12)
#define PAD_PUPD_SDIO3_D6_SHIFT         (12)
#define PAD_PUPD_SDIO3_D7_REG           (IOADDR_PAD_REG_BASE + 0x00)
#define PAD_PUPD_SDIO3_D7_MASK          GENMASK(15, 14)
#define PAD_PUPD_SDIO3_D7_SHIFT         (14)

#endif
