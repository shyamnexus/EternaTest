/*
 *  driver/mmc/nvt_ivot_mmc_ns02401.h
 *
 *  Copyright:	Novatek Inc.
 *
 */

#ifndef __NVT_IVOT_MMC_NS02401_H__
#define __NVT_IVOT_MMC_NS02401_H__

#include <asm/arch/ns02401_regs.h>
#include <asm/arch/gpio.h>
#include <linux/bitops.h>

/**************************
 *        Platform        *
 **************************/
/* SDIO controller number */
#define SDIO_HOST_ID_COUNT              (2)

/* SRCCLK */
#ifdef CONFIG_NVT_FPGA_EMULATION
#define SRCCLK_192MHZ                   (6000000)
#define SRCCLK_480MHZ                   (24000000)
#define SRCCLK_320MHZ                   (12000000)
#else
#define SRCCLK_192MHZ                   (192000000)
#define SRCCLK_480MHZ                   (480000000)
#define SRCCLK_320MHZ                   (320000000)
#endif

/* CLKSEL */
#define CG_SDIO_CLKSEL_REG              (IOADDR_CG_REG_BASE + 0x20)
#define CG_SDIO_CLKSEL_MASK             GENMASK(1, 0)
#define CG_SDIO_CLKSEL_SHIFT            (0)

#define CG_SDIO2_CLKSEL_REG             (IOADDR_CG_REG_BASE + 0x20)
#define CG_SDIO2_CLKSEL_MASK            GENMASK(3, 2)
#define CG_SDIO2_CLKSEL_SHIFT           (2)

#define _CLKSEL_192MHZ                  (0x0)
#define _CLKSEL_480MHZ                  (0x1)
#define _CLKSEL_320MHZ                  (0x2)

/* CLKDIV */
#define CG_SDIO_CLKDIV_REG              (IOADDR_CG_REG_BASE + 0x40)
#define CG_SDIO_CLKDIV_MASK             GENMASK(10, 0)
#define CG_SDIO_CLKDIV_SHIFT            (0)

#define CG_SDIO2_CLKDIV_REG             (IOADDR_CG_REG_BASE + 0x40)
#define CG_SDIO2_CLKDIV_MASK            GENMASK(26, 16)
#define CG_SDIO2_CLKDIV_SHIFT           (16)

/* CLKEN */
#define CG_SDIO_CLKEN_REG               (IOADDR_CG_REG_BASE + 0x74)
#define CG_SDIO_CLKEN_MASK              BIT(1)

#define CG_SDIO2_CLKEN_REG              (IOADDR_CG_REG_BASE + 0x74)
#define CG_SDIO2_CLKEN_MASK             BIT(2)

/* RSTN */
#define CG_SDIO_RSTN_REG                (IOADDR_CG_REG_BASE + 0x94)
#define CG_SDIO_RSTN_MASK               BIT(1)

#define CG_SDIO2_RSTN_REG               (IOADDR_CG_REG_BASE + 0x94)
#define CG_SDIO2_RSTN_MASK              BIT(2)

/* PINMUX */
#define PIN_SDIO_CFG_SDIO_1             0x1         ///< SDIO   (P_GPIO[12..13], P_GPIO[15..18])
#define PIN_SDIO_CFG_SDIO_2             0x2         ///< SDIO_2 (P_GPIO[19..20], P_GPIO[23..25])
#define PIN_SDIO_CFG_SDIO_3             0x4         ///< SDIO_3 (E_GPIO[24..29])

#define PIN_SDIO_CFG_SDIO2_1            0x10        ///< SDIO2 (C_GPIO[8..13])
#define PIN_SDIO_CFG_SDIO2_BUS_WIDTH    0x20        ///< SDIO2 bus width 8 bits (C_GPIO[14..17])
#define PIN_SDIO_CFG_SDIO2_DS           0x40        ///< SDIO2 Data Strobe (C_GPIO[6])

/* TOP */
#define TOP_SDIO_EN_REG                 (IOADDR_TOP_REG_BASE + 0x04)
#define TOP_SDIO_EN_MASK                BIT(4)
#define TOP_SDIO_EN_GPIO_REG            (IOADDR_TOP_REG_BASE + 0xA8)
#define TOP_SDIO_EN_GPIO_MASK           BIT(12) | BIT(13) | BIT(15) | BIT(16) | BIT(17) | BIT(18)

// 1-2
// #define TOP_SDIO_2_EN_REG            (IOADDR_TOP_REG_BASE + 0x04)
#define TOP_SDIO_2_EN_MASK              BIT(5)
#define TOP_SDIO_2_EN_GPIO_REG          (IOADDR_TOP_REG_BASE + 0xA8)
#define TOP_SDIO_2_EN_GPIO_MASK         BIT(19) | BIT(20) | BIT(22) | BIT(23) | BIT(24) | BIT(25)

// 1-3
// #define TOP_SDIO_2_EN_REG            (IOADDR_TOP_REG_BASE + 0x04)
#define TOP_SDIO_3_EN_MASK              BIT(4) | BIT(5)
#define TOP_SDIO_3_EN_GPIO_REG          (IOADDR_TOP_REG_BASE + 0xB0)
#define TOP_SDIO_3_EN_GPIO_MASK         BIT(24) | BIT(25) | BIT(26) | BIT(27) | BIT(28) | BIT(29)

#define TOP_SDIO2_EN_REG                (IOADDR_TOP_REG_BASE + 0x04)
#define TOP_SDIO2_EN_MASK               BIT(8)
#define TOP_SDIO2_EN_GPIO_REG           (IOADDR_TOP_REG_BASE + 0xA0)
#define TOP_SDIO2_EN_GPIO_MASK          BIT(8) | BIT(9) | BIT(10) | BIT(11) | BIT(12) | BIT(13)

#define TOP_SDIO2_WIDTH_REG             (IOADDR_TOP_REG_BASE + 0x04)
#define TOP_SDIO2_WIDTH_MASK            BIT(10)
#define TOP_SDIO2_WIDTH_GPIO_REG        (IOADDR_TOP_REG_BASE + 0xA0)
#define TOP_SDIO2_WIDTH_GPIO_MASK       BIT(14) | BIT(15) | BIT(16) | BIT(17)

/* PAD */
#define PAD_PUPD0_REG_OFS               (0x00)

#define PAD_REG_TO_BASE(reg)            (((reg)/(4))*(32))
#define	PAD_DS_CGPIO_BASE               PAD_REG_TO_BASE(0x104)       // 0x104~0x108

#define PAD_DS_CGPIO8                   (PAD_DS_CGPIO_BASE + 0)
#define PAD_DS_CGPIO9                   (PAD_DS_CGPIO_BASE + 4)
#define PAD_DS_CGPIO10                  (PAD_DS_CGPIO_BASE + 8)
#define PAD_DS_CGPIO11                  (PAD_DS_CGPIO_BASE + 12)
#define PAD_DS_CGPIO12                  (PAD_DS_CGPIO_BASE + 16)
#define PAD_DS_CGPIO13                  (PAD_DS_CGPIO_BASE + 20)
#define PAD_DS_CGPIO14                  (PAD_DS_CGPIO_BASE + 24)
#define PAD_DS_CGPIO15                  (PAD_DS_CGPIO_BASE + 28)

#define	PAD_DS_CGPIO_BASE2              PAD_REG_TO_BASE(0x108)       // 0x108
#define PAD_DS_CGPIO16                  (PAD_DS_CGPIO_BASE2 + 0)
#define PAD_DS_CGPIO17                  (PAD_DS_CGPIO_BASE2 + 4)

#define	PAD_DS_PGPIO_BASE               PAD_REG_TO_BASE(0x114)       // 0x114
#define PAD_DS_PGPIO12                  (PAD_DS_PGPIO_BASE + 16)
#define PAD_DS_PGPIO13                  (PAD_DS_PGPIO_BASE + 20)
#define PAD_DS_PGPIO14                  (PAD_DS_PGPIO_BASE + 24)
#define PAD_DS_PGPIO15                  (PAD_DS_PGPIO_BASE + 28)

#define	PAD_DS_PGPIO_BASE2              PAD_REG_TO_BASE(0x118)       // 0x118
#define PAD_DS_PGPIO16                  (PAD_DS_PGPIO_BASE2 + 0)
#define PAD_DS_PGPIO17                  (PAD_DS_PGPIO_BASE2 + 4)
#define PAD_DS_PGPIO18                  (PAD_DS_PGPIO_BASE2 + 8)
#define PAD_DS_PGPIO19                  (PAD_DS_PGPIO_BASE2 + 12)
#define PAD_DS_PGPIO20                  (PAD_DS_PGPIO_BASE2 + 16)
#define PAD_DS_PGPIO21                  (PAD_DS_PGPIO_BASE2 + 20)
#define PAD_DS_PGPIO22                  (PAD_DS_PGPIO_BASE2 + 24)
#define PAD_DS_PGPIO23                  (PAD_DS_PGPIO_BASE2 + 28)

#define	PAD_DS_PGPIO_BASE3              PAD_REG_TO_BASE(0x118)       // 0x118
#define PAD_DS_PGPIO24                  (PAD_DS_PGPIO_BASE3 + 0)
#define PAD_DS_PGPIO25                  (PAD_DS_PGPIO_BASE3 + 4)

#define	PAD_DS_EGPIO_BASE               PAD_REG_TO_BASE(0x14C)       // 0x14C
#define PAD_DS_EGPIO24                  (PAD_DS_EGPIO_BASE + 0)
#define PAD_DS_EGPIO25                  (PAD_DS_EGPIO_BASE + 4)
#define PAD_DS_EGPIO26                  (PAD_DS_EGPIO_BASE + 8)
#define PAD_DS_EGPIO27                  (PAD_DS_EGPIO_BASE + 12)
#define PAD_DS_EGPIO28                  (PAD_DS_EGPIO_BASE + 16)
#define PAD_DS_EGPIO29                  (PAD_DS_EGPIO_BASE + 20)

#define PAD_DS_SDIO_CLK                 PAD_DS_PGPIO12
#define PAD_DS_SDIO_CMD                 PAD_DS_PGPIO13
#define PAD_DS_SDIO_D0                  PAD_DS_PGPIO15
#define PAD_DS_SDIO_D1                  PAD_DS_PGPIO16
#define PAD_DS_SDIO_D2                  PAD_DS_PGPIO17
#define PAD_DS_SDIO_D3                  PAD_DS_PGPIO18

#define PAD_DS_SDIO_2_CLK               PAD_DS_PGPIO19
#define PAD_DS_SDIO_2_CMD               PAD_DS_PGPIO20
#define PAD_DS_SDIO_2_D0                PAD_DS_PGPIO22
#define PAD_DS_SDIO_2_D1                PAD_DS_PGPIO23
#define PAD_DS_SDIO_2_D2                PAD_DS_PGPIO24
#define PAD_DS_SDIO_2_D3                PAD_DS_PGPIO25

#define PAD_DS_SDIO_3_CLK               PAD_DS_EGPIO24
#define PAD_DS_SDIO_3_CMD               PAD_DS_EGPIO25
#define PAD_DS_SDIO_3_D0                PAD_DS_EGPIO26
#define PAD_DS_SDIO_3_D1                PAD_DS_EGPIO27
#define PAD_DS_SDIO_3_D2                PAD_DS_EGPIO28
#define PAD_DS_SDIO_3_D3                PAD_DS_EGPIO29

#define PAD_DS_SDIO2_CLK                PAD_DS_CGPIO8
#define PAD_DS_SDIO2_CMD                PAD_DS_CGPIO9
#define PAD_DS_SDIO2_D0                 PAD_DS_CGPIO10
#define PAD_DS_SDIO2_D1                 PAD_DS_CGPIO11
#define PAD_DS_SDIO2_D2                 PAD_DS_CGPIO12
#define PAD_DS_SDIO2_D3                 PAD_DS_CGPIO13
#define PAD_DS_SDIO2_D4                 PAD_DS_CGPIO14
#define PAD_DS_SDIO2_D5                 PAD_DS_CGPIO15
#define PAD_DS_SDIO2_D6                 PAD_DS_CGPIO16
#define PAD_DS_SDIO2_D7                 PAD_DS_CGPIO17

/* Controlled as GPIO */
#define GPIO_SDIO_CLK                   P_GPIO(12)
#define GPIO_SDIO_CMD                   P_GPIO(13)
#define GPIO_SDIO_D0                    P_GPIO(15)
#define GPIO_SDIO_D1                    P_GPIO(16)
#define GPIO_SDIO_D2                    P_GPIO(17)
#define GPIO_SDIO_D3                    P_GPIO(18)

#define GPIO_SDIO_2_CLK                 P_GPIO(19)
#define GPIO_SDIO_2_CMD                 P_GPIO(20)
#define GPIO_SDIO_2_D0                  P_GPIO(22)
#define GPIO_SDIO_2_D1                  P_GPIO(23)
#define GPIO_SDIO_2_D2                  P_GPIO(24)
#define GPIO_SDIO_2_D3                  P_GPIO(25)

#define GPIO_SDIO_3_CLK                 E_GPIO(24)
#define GPIO_SDIO_3_CMD                 E_GPIO(25)
#define GPIO_SDIO_3_D0                  E_GPIO(26)
#define GPIO_SDIO_3_D1                  E_GPIO(27)
#define GPIO_SDIO_3_D2                  E_GPIO(28)
#define GPIO_SDIO_3_D3                  E_GPIO(29)

#define GPIO_SDIO2_CLK                  C_GPIO(8)
#define GPIO_SDIO2_CMD                  C_GPIO(9)
#define GPIO_SDIO2_D0                   C_GPIO(10)
#define GPIO_SDIO2_D1                   C_GPIO(11)
#define GPIO_SDIO2_D2                   C_GPIO(12)
#define GPIO_SDIO2_D3                   C_GPIO(13)
#define GPIO_SDIO2_D4                   C_GPIO(14)
#define GPIO_SDIO2_D5                   C_GPIO(15)
#define GPIO_SDIO2_D6                   C_GPIO(16)
#define GPIO_SDIO2_D7                   C_GPIO(17)

/* PAD pull */
#define PAD_PULLNONE                    (0x0)
#define PAD_PULLDOWN                    (0x1)
#define PAD_PULLUP                      (0x2)

#define PAD_PUPD_SDIO_CLK_REG           (IOADDR_PAD_REG_BASE + 0x08)
#define PAD_PUPD_SDIO_CLK_MASK          GENMASK(25, 24)
#define PAD_PUPD_SDIO_CLK_SHIFT         (24)
#define PAD_PUPD_SDIO_CMD_REG           (IOADDR_PAD_REG_BASE + 0x08)
#define PAD_PUPD_SDIO_CMD_MASK          GENMASK(27, 26)
#define PAD_PUPD_SDIO_CMD_SHIFT         (26)
#define PAD_PUPD_SDIO_D0_REG            (IOADDR_PAD_REG_BASE + 0x08)
#define PAD_PUPD_SDIO_D0_MASK           GENMASK(31, 30)
#define PAD_PUPD_SDIO_D0_SHIFT          (30)
#define PAD_PUPD_SDIO_D1_REG            (IOADDR_PAD_REG_BASE + 0x0C)
#define PAD_PUPD_SDIO_D1_MASK           GENMASK(1, 0)
#define PAD_PUPD_SDIO_D1_SHIFT          (0)
#define PAD_PUPD_SDIO_D2_REG            (IOADDR_PAD_REG_BASE + 0x0C)
#define PAD_PUPD_SDIO_D2_MASK           GENMASK(3, 2)
#define PAD_PUPD_SDIO_D2_SHIFT          (2)
#define PAD_PUPD_SDIO_D3_REG            (IOADDR_PAD_REG_BASE + 0x0C)
#define PAD_PUPD_SDIO_D3_MASK           GENMASK(5, 4)
#define PAD_PUPD_SDIO_D3_SHIFT          (4)

#define PAD_PUPD_SDIO_2_CLK_REG         (IOADDR_PAD_REG_BASE + 0x0C)
#define PAD_PUPD_SDIO_2_CLK_MASK        GENMASK(7, 6)
#define PAD_PUPD_SDIO_2_CLK_SHIFT       (6)
#define PAD_PUPD_SDIO_2_CMD_REG         (IOADDR_PAD_REG_BASE + 0x0C)
#define PAD_PUPD_SDIO_2_CMD_MASK        GENMASK(9, 8)
#define PAD_PUPD_SDIO_2_CMD_SHIFT       (8)
#define PAD_PUPD_SDIO_2_D0_REG          (IOADDR_PAD_REG_BASE + 0x0C)
#define PAD_PUPD_SDIO_2_D0_MASK         GENMASK(13, 12)
#define PAD_PUPD_SDIO_2_D0_SHIFT        (12)
#define PAD_PUPD_SDIO_2_D1_REG          (IOADDR_PAD_REG_BASE + 0x0C)
#define PAD_PUPD_SDIO_2_D1_MASK         GENMASK(15, 14)
#define PAD_PUPD_SDIO_2_D1_SHIFT        (14)
#define PAD_PUPD_SDIO_2_D2_REG          (IOADDR_PAD_REG_BASE + 0x0C)
#define PAD_PUPD_SDIO_2_D2_MASK         GENMASK(17, 16)
#define PAD_PUPD_SDIO_2_D2_SHIFT        (16)
#define PAD_PUPD_SDIO_2_D3_REG          (IOADDR_PAD_REG_BASE + 0x0C)
#define PAD_PUPD_SDIO_2_D3_MASK         GENMASK(19, 18)
#define PAD_PUPD_SDIO_2_D3_SHIFT        (18)

#define PAD_PUPD_SDIO_3_CLK_REG         (IOADDR_PAD_REG_BASE + 0x24)
#define PAD_PUPD_SDIO_3_CLK_MASK        GENMASK(17, 16)
#define PAD_PUPD_SDIO_3_CLK_SHIFT       (16)
#define PAD_PUPD_SDIO_3_CMD_REG         (IOADDR_PAD_REG_BASE + 0x24)
#define PAD_PUPD_SDIO_3_CMD_MASK        GENMASK(19, 18)
#define PAD_PUPD_SDIO_3_CMD_SHIFT       (18)
#define PAD_PUPD_SDIO_3_D0_REG          (IOADDR_PAD_REG_BASE + 0x24)
#define PAD_PUPD_SDIO_3_D0_MASK         GENMASK(21, 20)
#define PAD_PUPD_SDIO_3_D0_SHIFT        (20)
#define PAD_PUPD_SDIO_3_D1_REG          (IOADDR_PAD_REG_BASE + 0x24)
#define PAD_PUPD_SDIO_3_D1_MASK         GENMASK(23, 22)
#define PAD_PUPD_SDIO_3_D1_SHIFT        (22)
#define PAD_PUPD_SDIO_3_D2_REG          (IOADDR_PAD_REG_BASE + 0x24)
#define PAD_PUPD_SDIO_3_D2_MASK         GENMASK(25, 24)
#define PAD_PUPD_SDIO_3_D2_SHIFT        (24)
#define PAD_PUPD_SDIO_3_D3_REG          (IOADDR_PAD_REG_BASE + 0x24)
#define PAD_PUPD_SDIO_3_D3_MASK         GENMASK(27, 26)
#define PAD_PUPD_SDIO_3_D3_SHIFT        (26)

#define PAD_PUPD_SDIO2_CLK_REG          (IOADDR_PAD_REG_BASE + 0x00)
#define PAD_PUPD_SDIO2_CLK_MASK         GENMASK(17, 16)
#define PAD_PUPD_SDIO2_CLK_SHIFT        (16)
#define PAD_PUPD_SDIO2_CMD_REG          (IOADDR_PAD_REG_BASE + 0x00)
#define PAD_PUPD_SDIO2_CMD_MASK         GENMASK(19, 18)
#define PAD_PUPD_SDIO2_CMD_SHIFT        (18)
#define PAD_PUPD_SDIO2_D0_REG           (IOADDR_PAD_REG_BASE + 0x00)
#define PAD_PUPD_SDIO2_D0_MASK          GENMASK(21, 20)
#define PAD_PUPD_SDIO2_D0_SHIFT         (20)
#define PAD_PUPD_SDIO2_D1_REG           (IOADDR_PAD_REG_BASE + 0x00)
#define PAD_PUPD_SDIO2_D1_MASK          GENMASK(23, 22)
#define PAD_PUPD_SDIO2_D1_SHIFT         (22)
#define PAD_PUPD_SDIO2_D2_REG           (IOADDR_PAD_REG_BASE + 0x00)
#define PAD_PUPD_SDIO2_D2_MASK          GENMASK(25, 24)
#define PAD_PUPD_SDIO2_D2_SHIFT         (24)
#define PAD_PUPD_SDIO2_D3_REG           (IOADDR_PAD_REG_BASE + 0x00)
#define PAD_PUPD_SDIO2_D3_MASK          GENMASK(27, 26)
#define PAD_PUPD_SDIO2_D3_SHIFT         (26)
#define PAD_PUPD_SDIO2_D4_REG           (IOADDR_PAD_REG_BASE + 0x00)
#define PAD_PUPD_SDIO2_D4_MASK          GENMASK(29, 28)
#define PAD_PUPD_SDIO2_D4_SHIFT         (28)
#define PAD_PUPD_SDIO2_D5_REG           (IOADDR_PAD_REG_BASE + 0x00)
#define PAD_PUPD_SDIO2_D5_MASK          GENMASK(31, 30)
#define PAD_PUPD_SDIO2_D5_SHIFT         (30)
#define PAD_PUPD_SDIO2_D6_REG           (IOADDR_PAD_REG_BASE + 0x04)
#define PAD_PUPD_SDIO2_D6_MASK          GENMASK(1, 0)
#define PAD_PUPD_SDIO2_D6_SHIFT         (0)
#define PAD_PUPD_SDIO2_D7_REG           (IOADDR_PAD_REG_BASE + 0x04)
#define PAD_PUPD_SDIO2_D7_MASK          GENMASK(3, 2)
#define PAD_PUPD_SDIO2_D7_SHIFT         (2)

#endif
