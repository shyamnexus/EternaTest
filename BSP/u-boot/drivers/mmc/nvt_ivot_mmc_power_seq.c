
#include <asm/nvt-common/nvt_common.h>
#include <linux/delay.h>
#include <linux/bitops.h>
#include "nvt_ivot_mmc_power_seq.h"
#include "nvt_ivot_mmc_a64.h"

u32 SDIO_SUP[SDIO_MAX_EMB] = {
    0x1,
    SDIO1_2_SUP,
    SDIO1_3_SUP,
    SDIO2_1_SUP,
    SDIO2_2_SUP,
    SDIO3_1_SUP
};

// SDIO settting
#if (SDIO_HOST_ID_COUNT > 0)
// SDIO1_1 gpio/pad/setting
#ifdef PIN_SDIO_CFG_SDIO_1
// 0: EN_REG, 1: EN_MASK, 2: EN_GPIO_REG, 3: EN_GPIO_MASK, 4: 0:NO_BUS_WIDTH/ 1:BUS_WIDTH 
uintptr_t SDIO_1_PINMUX[] = {
    TOP_SDIO_EN_REG,
    TOP_SDIO_EN_MASK,
    TOP_SDIO_EN_GPIO_REG,
    TOP_SDIO_EN_GPIO_MASK,
};

#ifdef PIN_SDIO_CFG_SDIO_BUS_WIDTH
// 0: WIDTH_REG, 1: WIDTH_MASK, 2: WIDTH_GPIO_REG, 3: WIDTH_GPIO_MASK
uintptr_t SDIO_1_WIDTH[4] = {
    TOP_SDIO_WIDTH_REG,
    TOP_SDIO_WIDTH_MASK,
    TOP_SDIO_WIDTH_GPIO_REG,
    TOP_SDIO_WIDTH_GPIO_MASK
};
#else
uintptr_t SDIO_1_WIDTH[4] = {0};
#endif

u32 SDIO_1_GPIO[2 + SDIO_PIN_MAX] = {
    GPIO_SDIO_CLK,
	GPIO_SDIO_CMD,
	GPIO_SDIO_D0,
	GPIO_SDIO_D1,
	GPIO_SDIO_D2,
	GPIO_SDIO_D3,
#if SDIO_PIN_MAX > 4
    GPIO_SDIO_D4,
    GPIO_SDIO_D5,
    GPIO_SDIO_D6,
    GPIO_SDIO_D7,
#endif
};

uintptr_t SDIO_1_PAD_REG[2 + SDIO_PIN_MAX] = {
    PAD_PUPD_SDIO_CLK_REG,
	PAD_PUPD_SDIO_CMD_REG,
	PAD_PUPD_SDIO_D0_REG,
	PAD_PUPD_SDIO_D1_REG,
	PAD_PUPD_SDIO_D2_REG,
	PAD_PUPD_SDIO_D3_REG,
#if SDIO_PIN_MAX > 4
    PAD_PUPD_SDIO_D4_REG,
    PAD_PUPD_SDIO_D5_REG,
    PAD_PUPD_SDIO_D6_REG,
    PAD_PUPD_SDIO_D7_REG,
#endif
};

uintptr_t SDIO_1_PAD_MASK[2 + SDIO_PIN_MAX] = {
    PAD_PUPD_SDIO_CLK_MASK,
	PAD_PUPD_SDIO_CMD_MASK,
	PAD_PUPD_SDIO_D0_MASK,
	PAD_PUPD_SDIO_D1_MASK,
	PAD_PUPD_SDIO_D2_MASK,
	PAD_PUPD_SDIO_D3_MASK,
#if SDIO_PIN_MAX > 4
    PAD_PUPD_SDIO_D4_MASK,
    PAD_PUPD_SDIO_D5_MASK,
    PAD_PUPD_SDIO_D6_MASK,
    PAD_PUPD_SDIO_D7_MASK,
#endif
};

uintptr_t SDIO_1_PAD_SHIFT[2 + SDIO_PIN_MAX] = {
    PAD_PUPD_SDIO_CLK_SHIFT,
	PAD_PUPD_SDIO_CMD_SHIFT,
	PAD_PUPD_SDIO_D0_SHIFT,
	PAD_PUPD_SDIO_D1_SHIFT,
	PAD_PUPD_SDIO_D2_SHIFT,
	PAD_PUPD_SDIO_D3_SHIFT,
#if SDIO_PIN_MAX > 4
    PAD_PUPD_SDIO_D4_SHIFT,
    PAD_PUPD_SDIO_D5_SHIFT,
    PAD_PUPD_SDIO_D6_SHIFT,
    PAD_PUPD_SDIO_D7_SHIFT,
#endif
};
#else
// NULL array
uintptr_t SDIO_1_PINMUX[PIN_SDIO_BUS_WIDTH] = {0};
uintptr_t SDIO_1_WIDTH[PIN_SDIO_BUS_WIDTH] = {0};
u32 SDIO_1_GPIO[PIN_SDIO_BUS_WIDTH] = {0};
uintptr_t SDIO_1_PAD_REG[PIN_SDIO_BUS_WIDTH] = {0};
uintptr_t SDIO_1_PAD_MASK[PIN_SDIO_BUS_WIDTH] = {0};
uintptr_t SDIO_1_PAD_SHIFT[PIN_SDIO_BUS_WIDTH] = {0};
#endif

// SDIO1_2 gpio/pad/setting
#ifdef PIN_SDIO_CFG_SDIO_2
// 0: EN_REG, 1: EN_MASK, 2: EN_GPIO_REG, 3: EN_GPIO_MASK, 4: 0:NO_BUS_WIDTH/ 1:BUS_WIDTH  
uintptr_t SDIO_2_PINMUX[] = {
    TOP_SDIO_EN_REG,
    TOP_SDIO_2_EN_MASK,
    TOP_SDIO_2_EN_GPIO_REG,
    TOP_SDIO_2_EN_GPIO_MASK,
};

#ifdef PIN_SDIO_CFG_SDIO_BUS_WIDTH
// 0: WIDTH_REG, 1: WIDTH_MASK, 2: WIDTH_GPIO_REG, 3: WIDTH_GPIO_MASK
uintptr_t SDIO_2_WIDTH[4] = {
    TOP_SDIO_WIDTH_REG,
    TOP_SDIO_WIDTH_MASK,
    TOP_SDIO_2_WIDTH_GPIO_REG,
    TOP_SDIO_2_WIDTH_GPIO_MASK
};
#else
uintptr_t SDIO_2_WIDTH[4] = {0};
#endif

u32 SDIO_2_GPIO[2 + SDIO_PIN_MAX] = {
    GPIO_SDIO_2_CLK,
	GPIO_SDIO_2_CMD,
	GPIO_SDIO_2_D0,
	GPIO_SDIO_2_D1,
	GPIO_SDIO_2_D2,
	GPIO_SDIO_2_D3,
#if SDIO_PIN_MAX > 4
    GPIO_SDIO_2_D4,
    GPIO_SDIO_2_D5,
    GPIO_SDIO_2_D6,
    GPIO_SDIO_2_D7,
#endif
};

uintptr_t SDIO_2_PAD_REG[2 + SDIO_PIN_MAX] = {
    PAD_PUPD_SDIO_2_CLK_REG,
	PAD_PUPD_SDIO_2_CMD_REG,
	PAD_PUPD_SDIO_2_D0_REG,
	PAD_PUPD_SDIO_2_D1_REG,
	PAD_PUPD_SDIO_2_D2_REG,
	PAD_PUPD_SDIO_2_D3_REG,
#if SDIO_PIN_MAX > 4
    PAD_PUPD_SDIO_2_D4_REG,
    PAD_PUPD_SDIO_2_D5_REG,
    PAD_PUPD_SDIO_2_D6_REG,
    PAD_PUPD_SDIO_2_D7_REG,
#endif
};

uintptr_t SDIO_2_PAD_MASK[2 + SDIO_PIN_MAX] = {
    PAD_PUPD_SDIO_2_CLK_MASK,
	PAD_PUPD_SDIO_2_CMD_MASK,
	PAD_PUPD_SDIO_2_D0_MASK,
	PAD_PUPD_SDIO_2_D1_MASK,
	PAD_PUPD_SDIO_2_D2_MASK,
	PAD_PUPD_SDIO_2_D3_MASK,
#if SDIO_PIN_MAX > 4
    PAD_PUPD_SDIO_2_D4_MASK,
    PAD_PUPD_SDIO_2_D5_MASK,
    PAD_PUPD_SDIO_2_D6_MASK,
    PAD_PUPD_SDIO_2_D7_MASK,
#endif
};

uintptr_t SDIO_2_PAD_SHIFT[2 + SDIO_PIN_MAX] = {
    PAD_PUPD_SDIO_2_CLK_SHIFT,
	PAD_PUPD_SDIO_2_CMD_SHIFT,
	PAD_PUPD_SDIO_2_D0_SHIFT,
	PAD_PUPD_SDIO_2_D1_SHIFT,
	PAD_PUPD_SDIO_2_D2_SHIFT,
	PAD_PUPD_SDIO_2_D3_SHIFT,
#if SDIO_PIN_MAX > 4
    PAD_PUPD_SDIO_2_D4_SHIFT,
    PAD_PUPD_SDIO_2_D5_SHIFT,
    PAD_PUPD_SDIO_2_D6_SHIFT,
    PAD_PUPD_SDIO_2_D7_SHIFT,
#endif
};
#else
// NULL array
uintptr_t SDIO_2_PINMUX[PIN_SDIO_BUS_WIDTH] = {0};
uintptr_t SDIO_2_WIDTH[PIN_SDIO_BUS_WIDTH] = {0};
u32 SDIO_2_GPIO[PIN_SDIO_BUS_WIDTH] = {0};
uintptr_t SDIO_2_PAD_REG[PIN_SDIO_BUS_WIDTH] = {0};
uintptr_t SDIO_2_PAD_MASK[PIN_SDIO_BUS_WIDTH] = {0};
uintptr_t SDIO_2_PAD_SHIFT[PIN_SDIO_BUS_WIDTH] = {0};
#endif

// SDIO1_3 gpio/pad/setting
#ifdef PIN_SDIO_CFG_SDIO_3
// 0: EN_REG, 1: EN_MASK, 2: EN_GPIO_REG, 3: EN_GPIO_MASK, 4: 0:NO_BUS_WIDTH/ 1:BUS_WIDTH 
uintptr_t SDIO_3_PINMUX[] = {
    TOP_SDIO_EN_REG,
    TOP_SDIO_3_EN_MASK,
    TOP_SDIO_3_EN_GPIO_REG,
    TOP_SDIO_3_EN_GPIO_MASK,
};

#ifdef PIN_SDIO_CFG_SDIO_BUS_WIDTH
// 0: WIDTH_REG, 1: WIDTH_MASK, 2: WIDTH_GPIO_REG, 3: WIDTH_GPIO_MASK
uintptr_t SDIO_3_WIDTH[4] = {
    TOP_SDIO_WIDTH_REG,
    TOP_SDIO_WIDTH_MASK,
    TOP_SDIO_3_WIDTH_GPIO_REG,
    TOP_SDIO_3_WIDTH_GPIO_MASK
};
#else
uintptr_t SDIO_3_WIDTH[4] = {0};
#endif

u32 SDIO_3_GPIO[2 + SDIO_PIN_MAX] = {
    GPIO_SDIO_3_CLK,
	GPIO_SDIO_3_CMD,
	GPIO_SDIO_3_D0,
	GPIO_SDIO_3_D1,
	GPIO_SDIO_3_D2,
	GPIO_SDIO_3_D3,
#if SDIO_PIN_MAX > 4
    GPIO_SDIO_3_D4,
    GPIO_SDIO_3_D5,
    GPIO_SDIO_3_D6,
    GPIO_SDIO_3_D7,
#endif
};

uintptr_t SDIO_3_PAD_REG[2 + SDIO_PIN_MAX] = {
    PAD_PUPD_SDIO_3_CLK_REG,
	PAD_PUPD_SDIO_3_CMD_REG,
	PAD_PUPD_SDIO_3_D0_REG,
	PAD_PUPD_SDIO_3_D1_REG,
	PAD_PUPD_SDIO_3_D2_REG,
	PAD_PUPD_SDIO_3_D3_REG,
#if SDIO_PIN_MAX > 4
    PAD_PUPD_SDIO_3_D4_REG,
    PAD_PUPD_SDIO_3_D5_REG,
    PAD_PUPD_SDIO_3_D6_REG,
    PAD_PUPD_SDIO_3_D7_REG,
#endif
};

uintptr_t SDIO_3_PAD_MASK[2 + SDIO_PIN_MAX] = {
    PAD_PUPD_SDIO_3_CLK_MASK,
	PAD_PUPD_SDIO_3_CMD_MASK,
	PAD_PUPD_SDIO_3_D0_MASK,
	PAD_PUPD_SDIO_3_D1_MASK,
	PAD_PUPD_SDIO_3_D2_MASK,
	PAD_PUPD_SDIO_3_D3_MASK,
#if SDIO_PIN_MAX > 4
    PAD_PUPD_SDIO_3_D4_MASK,
    PAD_PUPD_SDIO_3_D5_MASK,
    PAD_PUPD_SDIO_3_D6_MASK,
    PAD_PUPD_SDIO_3_D7_MASK,
#endif
};

uintptr_t SDIO_3_PAD_SHIFT[2 + SDIO_PIN_MAX] = {
    PAD_PUPD_SDIO_3_CLK_SHIFT,
	PAD_PUPD_SDIO_3_CMD_SHIFT,
	PAD_PUPD_SDIO_3_D0_SHIFT,
	PAD_PUPD_SDIO_3_D1_SHIFT,
	PAD_PUPD_SDIO_3_D2_SHIFT,
	PAD_PUPD_SDIO_3_D3_SHIFT,
#if SDIO_PIN_MAX > 4
    PAD_PUPD_SDIO_3_D4_SHIFT,
    PAD_PUPD_SDIO_3_D5_SHIFT,
    PAD_PUPD_SDIO_3_D6_SHIFT,
    PAD_PUPD_SDIO_3_D7_SHIFT,
#endif
};
#else
// NULL array
uintptr_t SDIO_3_PINMUX[PIN_SDIO_BUS_WIDTH] = {0};
uintptr_t SDIO_3_WIDTH[PIN_SDIO_BUS_WIDTH] = {0};
u32 SDIO_3_GPIO[PIN_SDIO_BUS_WIDTH] = {0};
uintptr_t SDIO_3_PAD_REG[PIN_SDIO_BUS_WIDTH] = {0};
uintptr_t SDIO_3_PAD_MASK[PIN_SDIO_BUS_WIDTH] = {0};
uintptr_t SDIO_3_PAD_SHIFT[PIN_SDIO_BUS_WIDTH] = {0};
#endif
#endif
// SDIO2 settting
#if (SDIO_HOST_ID_COUNT > 1)
// SDIO2_1 gpio/pad/setting
#ifdef PIN_SDIO_CFG_SDIO2_1
// 0: EN_REG, 1: EN_MASK, 2: EN_GPIO_REG, 3: EN_GPIO_MASK, 4: 0:NO_BUS_WIDTH/ 1:BUS_WIDTH  
uintptr_t SDIO2_1_PINMUX[] = {
    TOP_SDIO2_EN_REG,
    TOP_SDIO2_EN_MASK,
    TOP_SDIO2_EN_GPIO_REG,
    TOP_SDIO2_EN_GPIO_MASK,
};

#ifdef PIN_SDIO_CFG_SDIO2_BUS_WIDTH
// 0: WIDTH_REG, 1: WIDTH_MASK, 2: WIDTH_GPIO_REG, 3: WIDTH_GPIO_MASK
uintptr_t SDIO2_1_WIDTH[4] = {
    TOP_SDIO2_WIDTH_REG,
    TOP_SDIO2_WIDTH_MASK,
    TOP_SDIO2_WIDTH_GPIO_REG,
    TOP_SDIO2_WIDTH_GPIO_MASK
};
#else
uintptr_t SDIO2_1_WIDTH[4] = {0};
#endif

u32 SDIO2_1_GPIO[2 + SDIO2_PIN_MAX] = {
    GPIO_SDIO2_CLK,
	GPIO_SDIO2_CMD,
	GPIO_SDIO2_D0,
	GPIO_SDIO2_D1,
	GPIO_SDIO2_D2,
	GPIO_SDIO2_D3,
#if SDIO2_PIN_MAX > 4
    GPIO_SDIO2_D4,
    GPIO_SDIO2_D5,
    GPIO_SDIO2_D6,
    GPIO_SDIO2_D7,
#endif
};

uintptr_t SDIO2_1_PAD_REG[2 + SDIO2_PIN_MAX] = {
    PAD_PUPD_SDIO2_CLK_REG,
	PAD_PUPD_SDIO2_CMD_REG,
	PAD_PUPD_SDIO2_D0_REG,
	PAD_PUPD_SDIO2_D1_REG,
	PAD_PUPD_SDIO2_D2_REG,
	PAD_PUPD_SDIO2_D3_REG,
#if SDIO2_PIN_MAX > 4
    PAD_PUPD_SDIO2_D4_REG,
    PAD_PUPD_SDIO2_D5_REG,
    PAD_PUPD_SDIO2_D6_REG,
    PAD_PUPD_SDIO2_D7_REG,
#endif
};

uintptr_t SDIO2_1_PAD_MASK[2 + SDIO2_PIN_MAX] = {
    PAD_PUPD_SDIO2_CLK_MASK,
	PAD_PUPD_SDIO2_CMD_MASK,
	PAD_PUPD_SDIO2_D0_MASK,
	PAD_PUPD_SDIO2_D1_MASK,
	PAD_PUPD_SDIO2_D2_MASK,
	PAD_PUPD_SDIO2_D3_MASK,
#if SDIO2_PIN_MAX > 4
    PAD_PUPD_SDIO2_D4_MASK,
    PAD_PUPD_SDIO2_D5_MASK,
    PAD_PUPD_SDIO2_D6_MASK,
    PAD_PUPD_SDIO2_D7_MASK,
#endif
};

uintptr_t SDIO2_1_PAD_SHIFT[2 + SDIO2_PIN_MAX] = {
    PAD_PUPD_SDIO2_CLK_SHIFT,
	PAD_PUPD_SDIO2_CMD_SHIFT,
	PAD_PUPD_SDIO2_D0_SHIFT,
	PAD_PUPD_SDIO2_D1_SHIFT,
	PAD_PUPD_SDIO2_D2_SHIFT,
	PAD_PUPD_SDIO2_D3_SHIFT,
#if SDIO2_PIN_MAX > 4
    PAD_PUPD_SDIO2_D4_SHIFT,
    PAD_PUPD_SDIO2_D5_SHIFT,
    PAD_PUPD_SDIO2_D6_SHIFT,
    PAD_PUPD_SDIO2_D7_SHIFT,
#endif
};
#else
// NULL array
uintptr_t SDIO2_1_PINMUX[PIN_SDIO_BUS_WIDTH] = {0};
uintptr_t SDIO2_1_WIDTH[PIN_SDIO_BUS_WIDTH] = {0};
u32 SDIO2_1_GPIO[PIN_SDIO_BUS_WIDTH] = {0};
uintptr_t SDIO2_1_PAD_REG[PIN_SDIO_BUS_WIDTH] = {0};
uintptr_t SDIO2_1_PAD_MASK[PIN_SDIO_BUS_WIDTH] = {0};
uintptr_t SDIO2_1_PAD_SHIFT[PIN_SDIO_BUS_WIDTH] = {0};
#endif

// SDIO2_2 gpio/pad/setting
#ifdef PIN_SDIO_CFG_SDIO2_2
// 0: EN_REG, 1: EN_MASK, 2: EN_GPIO_REG, 3: EN_GPIO_MASK, 4: 0:NO_BUS_WIDTH/ 1:BUS_WIDTH 
uintptr_t SDIO2_2_PINMUX[] = {
    TOP_SDIO2_EN_REG,
    TOP_SDIO2_2_EN_MASK,
    TOP_SDIO2_2_EN_GPIO_REG,
    TOP_SDIO2_2_EN_GPIO_MASK,
};

#ifdef PIN_SDIO_CFG_SDIO2_BUS_WIDTH
// 0: WIDTH_REG, 1: WIDTH_MASK, 2: WIDTH_GPIO_REG, 3: WIDTH_GPIO_MASK
uintptr_t SDIO2_2_WIDTH[4] = {
    TOP_SDIO2_WIDTH_REG,
    TOP_SDIO2_WIDTH_MASK,
    TOP_SDIO2_2_WIDTH_GPIO_REG,
    TOP_SDIO2_2_WIDTH_GPIO_MASK
};
#else
uintptr_t SDIO2_2_WIDTH[4] = {0};
#endif

u32 SDIO2_2_GPIO[2 + SDIO2_PIN_MAX] = {
    GPIO_SDIO2_2_CLK,
	GPIO_SDIO2_2_CMD,
	GPIO_SDIO2_2_D0,
	GPIO_SDIO2_2_D1,
	GPIO_SDIO2_2_D2,
	GPIO_SDIO2_2_D3,
#if SDIO2_PIN_MAX > 4
    GPIO_SDIO2_2_D4,
    GPIO_SDIO2_2_D5,
    GPIO_SDIO2_2_D6,
    GPIO_SDIO2_2_D7,
#endif
};

uintptr_t SDIO2_2_PAD_REG[2 + SDIO2_PIN_MAX] = {
    PAD_PUPD_SDIO2_2_CLK_REG,
	PAD_PUPD_SDIO2_2_CMD_REG,
	PAD_PUPD_SDIO2_2_D0_REG,
	PAD_PUPD_SDIO2_2_D1_REG,
	PAD_PUPD_SDIO2_2_D2_REG,
	PAD_PUPD_SDIO2_2_D3_REG,
#if SDIO2_PIN_MAX > 4
    PAD_PUPD_SDIO2_2_D4_REG,
    PAD_PUPD_SDIO2_2_D5_REG,
    PAD_PUPD_SDIO2_2_D6_REG,
    PAD_PUPD_SDIO2_2_D7_REG,
#endif
};

uintptr_t SDIO2_2_PAD_MASK[2 + SDIO2_PIN_MAX] = {
    PAD_PUPD_SDIO2_2_CLK_MASK,
	PAD_PUPD_SDIO2_2_CMD_MASK,
	PAD_PUPD_SDIO2_2_D0_MASK,
	PAD_PUPD_SDIO2_2_D1_MASK,
	PAD_PUPD_SDIO2_2_D2_MASK,
	PAD_PUPD_SDIO2_2_D3_MASK,
#if SDIO2_PIN_MAX > 4
    PAD_PUPD_SDIO2_2_D4_MASK,
    PAD_PUPD_SDIO2_2_D5_MASK,
    PAD_PUPD_SDIO2_2_D6_MASK,
    PAD_PUPD_SDIO2_2_D7_MASK,
#endif
};

uintptr_t SDIO2_2_PAD_SHIFT[2 + SDIO2_PIN_MAX] = {
    PAD_PUPD_SDIO2_2_CLK_SHIFT,
	PAD_PUPD_SDIO2_2_CMD_SHIFT,
	PAD_PUPD_SDIO2_2_D0_SHIFT,
	PAD_PUPD_SDIO2_2_D1_SHIFT,
	PAD_PUPD_SDIO2_2_D2_SHIFT,
	PAD_PUPD_SDIO2_2_D3_SHIFT,
#if SDIO2_PIN_MAX > 4
    PAD_PUPD_SDIO2_2_D4_SHIFT,
    PAD_PUPD_SDIO2_2_D5_SHIFT,
    PAD_PUPD_SDIO2_2_D6_SHIFT,
    PAD_PUPD_SDIO2_2_D7_SHIFT,
#endif
};
#else
// NULL array
uintptr_t SDIO2_2_PINMUX[PIN_SDIO_BUS_WIDTH] = {0};
uintptr_t SDIO2_2_WIDTH[PIN_SDIO_BUS_WIDTH] = {0};
u32 SDIO2_2_GPIO[PIN_SDIO_BUS_WIDTH] = {0};
uintptr_t SDIO2_2_PAD_REG[PIN_SDIO_BUS_WIDTH] = {0};
uintptr_t SDIO2_2_PAD_MASK[PIN_SDIO_BUS_WIDTH] = {0};
uintptr_t SDIO2_2_PAD_SHIFT[PIN_SDIO_BUS_WIDTH] = {0};
#endif
#endif
// SDIO3 settting
#if (SDIO_HOST_ID_COUNT > 2)
// SDIO3_1 gpio/pad/setting
#ifdef PIN_SDIO_CFG_SDIO3_1
// 0: EN_REG, 1: EN_MASK, 2: EN_GPIO_REG, 3: EN_GPIO_MASK, 4: 0:NO_BUS_WIDTH/ 1:BUS_WIDTH 
uintptr_t SDIO3_1_PINMUX[] = {
    TOP_SDIO3_EN_REG,
    TOP_SDIO3_EN_MASK,
    TOP_SDIO3_EN_GPIO_REG,
    TOP_SDIO3_EN_GPIO_MASK,
};

#ifdef PIN_SDIO_CFG_SDIO3_BUS_WIDTH
// 0: WIDTH_REG, 1: WIDTH_MASK, 2: WIDTH_GPIO_REG, 3: WIDTH_GPIO_MASK
uintptr_t SDIO3_1_WIDTH[4] = {
    TOP_SDIO3_WIDTH_REG,
    TOP_SDIO3_WIDTH_MASK,
    TOP_SDIO3_WIDTH_GPIO_REG,
    TOP_SDIO3_WIDTH_GPIO_MASK
};
#else
uintptr_t SDIO3_1_WIDTH[4] = {0};
#endif

u32 SDIO3_1_GPIO[2 + SDIO3_PIN_MAX] = {
    GPIO_SDIO3_CLK,
	GPIO_SDIO3_CMD,
	GPIO_SDIO3_D0,
	GPIO_SDIO3_D1,
	GPIO_SDIO3_D2,
	GPIO_SDIO3_D3,
#if SDIO3_PIN_MAX > 4
    GPIO_SDIO3_D4,
    GPIO_SDIO3_D5,
    GPIO_SDIO3_D6,
    GPIO_SDIO3_D7,
#endif
};

uintptr_t SDIO3_1_PAD_REG[2 + SDIO3_PIN_MAX] = {
    PAD_PUPD_SDIO3_CLK_REG,
	PAD_PUPD_SDIO3_CMD_REG,
	PAD_PUPD_SDIO3_D0_REG,
	PAD_PUPD_SDIO3_D1_REG,
	PAD_PUPD_SDIO3_D2_REG,
	PAD_PUPD_SDIO3_D3_REG,
#if SDIO3_PIN_MAX > 4
    PAD_PUPD_SDIO3_D4_REG,
    PAD_PUPD_SDIO3_D5_REG,
    PAD_PUPD_SDIO3_D6_REG,
    PAD_PUPD_SDIO3_D7_REG,
#endif
};

uintptr_t SDIO3_1_PAD_MASK[2 + SDIO3_PIN_MAX] = {
    PAD_PUPD_SDIO3_CLK_MASK,
	PAD_PUPD_SDIO3_CMD_MASK,
	PAD_PUPD_SDIO3_D0_MASK,
	PAD_PUPD_SDIO3_D1_MASK,
	PAD_PUPD_SDIO3_D2_MASK,
	PAD_PUPD_SDIO3_D3_MASK,
#if SDIO3_PIN_MAX > 4
    PAD_PUPD_SDIO3_D4_MASK,
    PAD_PUPD_SDIO3_D5_MASK,
    PAD_PUPD_SDIO3_D6_MASK,
    PAD_PUPD_SDIO3_D7_MASK,
#endif
};

uintptr_t SDIO3_1_PAD_SHIFT[2 + SDIO3_PIN_MAX] = {
    PAD_PUPD_SDIO3_CLK_SHIFT,
	PAD_PUPD_SDIO3_CMD_SHIFT,
	PAD_PUPD_SDIO3_D0_SHIFT,
	PAD_PUPD_SDIO3_D1_SHIFT,
	PAD_PUPD_SDIO3_D2_SHIFT,
	PAD_PUPD_SDIO3_D3_SHIFT,
#if SDIO3_PIN_MAX > 4
    PAD_PUPD_SDIO3_D4_SHIFT,
    PAD_PUPD_SDIO3_D5_SHIFT,
    PAD_PUPD_SDIO3_D6_SHIFT,
    PAD_PUPD_SDIO3_D7_SHIFT,
#endif
};
#else
// NULL array
uintptr_t SDIO3_1_PINMUX[PIN_SDIO_BUS_WIDTH] = {0};
uintptr_t SDIO3_1_WIDTH[PIN_SDIO_BUS_WIDTH] = {0};
u32 SDIO3_1_GPIO[PIN_SDIO_BUS_WIDTH] = {0};
uintptr_t SDIO3_1_PAD_REG[PIN_SDIO_BUS_WIDTH] = {0};
uintptr_t SDIO3_1_PAD_MASK[PIN_SDIO_BUS_WIDTH] = {0};
uintptr_t SDIO3_1_PAD_SHIFT[PIN_SDIO_BUS_WIDTH] = {0};
#endif
#endif

void nvt_mmc_pinmux_update(uintptr_t* pinmux_set, uintptr_t* width_set, u32 isWidth8, u32 value)
{
    volatile uintptr_t reg_val;

    if (value) {
        reg_val = readl(pinmux_set[0]);
        reg_val &= ~(pinmux_set[1]);
        reg_val |= (pinmux_set[1]);
        writel(reg_val, pinmux_set[0]);
        udelay(10);
        // 8 bits pinmux
        if (pinmux_set[4] && isWidth8) { 
            reg_val = readl(width_set[0]);
            reg_val &= ~(width_set[1]);
            reg_val |= (width_set[1]);
            writel(reg_val, width_set[0]);
            udelay(10);
        }
       
        reg_val = readl(pinmux_set[2]);
        reg_val &= ~(pinmux_set[3]);
        writel(reg_val, pinmux_set[2]);
        udelay(10);
        
        // 8 bits gpio
        if (pinmux_set[4] && isWidth8) { 
            reg_val = readl(width_set[2]);
            reg_val &= ~(width_set[3]);
            writel(reg_val, width_set[2]);
            udelay(10);
        }
    } else {
        // 0: EN_REG, 1: EN_MASK, 2: EN_GPIO_REG, 3: EN_GPIO_MASK, 4: 0:NO_BUS_WIDTH/ 1:BUS_WIDTH 
        reg_val = readl(pinmux_set[0]);
        reg_val &= ~(pinmux_set[1]);
        writel(reg_val, pinmux_set[0]);
        udelay(10);
        // 8 bits pinmux
        if (pinmux_set[4] && isWidth8) { 
            reg_val = readl(width_set[0]);
            reg_val &= ~(width_set[1]);
            writel(reg_val, width_set[0]);
            udelay(10);
        }

        reg_val = readl(pinmux_set[2]);
        reg_val &= ~(pinmux_set[3]);
        reg_val |= (pinmux_set[3]);
        writel(reg_val, pinmux_set[2]);
        udelay(10);
        
        // 8 bits gpio
        if (pinmux_set[4] && isWidth8) { 
            reg_val = readl(width_set[2]);
            reg_val &= ~(width_set[3]);
            reg_val |= (width_set[3]);
            writel(reg_val, width_set[2]);
            udelay(10);
        }
        
    }
}

void nvt_pad_set_pull_updown(uintptr_t sdio_pad_reg, uintptr_t sdio_pad_mask, uintptr_t sdio_pad_shift, uintptr_t value)
{
    volatile uintptr_t reg_val;

    reg_val = readl(sdio_pad_reg);
	reg_val &= ~(sdio_pad_mask);
	reg_val |= (value << sdio_pad_shift);
	writel(reg_val, sdio_pad_reg);
    udelay(10);
}

// gpio_pin: means pin, direction: means inout, value: set for output high/low, input don't care
void nvt_set_gpio_inout(u32 gpio_pin, u32 direction, u32 value)
{
    char str[10] = "sdio_";
    char numStr[10];
    
    sprintf(numStr, "%d", gpio_pin);
    strcat(str, numStr);

    /* Set as GPIO */
	gpio_request(gpio_pin, str);
    if (direction) {
        gpio_direction_output(gpio_pin, value);
    } else {
        gpio_direction_input(gpio_pin);
    }
}

void sdiohost_power_up(struct mmc_nvt_host *host)
{
	u8 i,j;
    uint8_t max_pin = SDIO_PIN_DATA4;

    // some customer do not want to active wifi in uboot. want to active in linux.
    if (host->is1v8wifi) {
        printk("host%x is 1v8 wifi, active in linux kernel\n", host->id);
#if defined (SDIO1_PAD_POWER_SUP) || defined (SDIO2_PAD_POWER_SUP) || defined (SDIO2_PAD_POWER_SUP)
        volatile uintptr_t reg_val = 0;
#endif
        // set pad power to 1v8 for linux kernel
#if defined (SDIO1_PAD_POWER_SUP)
        if (host->id == SDIO_HOST_ID_1) {
            reg_val = readl(SDIO1_PAD_POWER_REG);
            reg_val &= ~(SDIO1_PAD_POWER_MASK);
            writel(reg_val, SDIO1_PAD_POWER_REG);
            udelay(10);
        }
#endif
#if defined (SDIO2_PAD_POWER_SUP)
        if (host->id == SDIO_HOST_ID_2) {
            reg_val = readl(SDIO2_PAD_POWER_REG);
            reg_val &= ~(SDIO2_PAD_POWER_MASK);
            writel(reg_val, SDIO2_PAD_POWER_REG);
            udelay(10);
        }
#endif
        return;
    }

    // turn on power
    if (host->card_power[0]) {
        gpio_request(host->card_power[1], "sdio_power");
        gpio_direction_output(host->card_power[1], host->card_power[2]);
        /* SD spec rev9.10 section 6.4.1.4:
	     * Recommendation of Power ramp up:
	     * The ramp up time should be 0.1ms to 35ms for 2.7-3.6V power supply and host shall wait until VDD is stable.
	     * After 1ms VDD stable time, host provides at least 74 clocks before issuing the first command.
	     */
        if (host->card_power[3] > 0) {
            udelay(host->card_power[3] * 1000);
        } else {
            udelay(36 * 1000);
        }
    }

    for (i = 0; i < SDIO_MAX_EMB; i++) {
        if ((host->pinmux_value & SDIO_SUP[i]) && \
            (host->id == SDIO_HOST_ID_1)) {
                // SDIO1_1
                if (i == SDIO1_1_EMB) {
                    // check 4-bits/8-bits data
#ifdef PIN_SDIO_CFG_SDIO_BUS_WIDTH
                    if (host->enable_8bits) {
                        max_pin = SDIO_PIN_NUM;
                    } else {
                        max_pin = SDIO_PIN_DATA4;
                    }

                    // set CMD/DATA GPIO direction to input to avoid card identification in SPI mode
                    for (j = SDIO_PIN_CMD; j < max_pin; j++) {
			            nvt_set_gpio_inout(SDIO_1_GPIO[j], 0, 0);
		            }

                    // update pinmux config
                    if (max_pin == SDIO_PIN_DATA4)
                        nvt_mmc_pinmux_update(SDIO_1_PINMUX,SDIO_1_WIDTH,0,TRUE);
                    else
                        nvt_mmc_pinmux_update(SDIO_1_PINMUX,SDIO_1_WIDTH,1,TRUE);
#else
                    max_pin = SDIO_PIN_DATA4;
                    // set CMD/DATA GPIO direction to input to avoid card identification in SPI mode
                    for (j = SDIO_PIN_CMD; j < max_pin; j++) {
			            nvt_set_gpio_inout(SDIO_1_GPIO[j], 0, 0);
		            }

                    // update pinmux config
                    nvt_mmc_pinmux_update(SDIO_1_PINMUX,SDIO_1_WIDTH,0,TRUE);
#endif                             
                    /* SD spec rev9.10 section 6:
		             * Pull-up resistors protect the CMD and the DAT lines against bus floating before card identification.
		             */
                    // set PAD pull without CLK pin
                    for (j = SDIO_PIN_CMD; j < max_pin; j++) {
			            nvt_pad_set_pull_updown(SDIO_1_PAD_REG[j], SDIO_1_PAD_MASK[j], SDIO_1_PAD_SHIFT[j], PAD_PULLUP);
		            }
                    // set CLK PAD pullnone
                    nvt_pad_set_pull_updown(SDIO_1_PAD_REG[SDIO_PIN_CLK], SDIO_1_PAD_MASK[SDIO_PIN_CLK], SDIO_1_PAD_SHIFT[SDIO_PIN_CLK], PAD_PULLNONE);
                }
                // SDIO1_2 
                else if (i == SDIO1_2_EMB) {
                    // check 4-bits/8-bits data 
#ifdef PIN_SDIO_CFG_SDIO_BUS_WIDTH
                    if (host->enable_8bits) {
                        max_pin = SDIO_PIN_NUM;
                    } else {
                        max_pin = SDIO_PIN_DATA4;
                    }

                    // set CMD/DATA GPIO direction to input to avoid card identification in SPI mode
                    for (j = SDIO_PIN_CMD; j < max_pin; j++) {
			            nvt_set_gpio_inout(SDIO_2_GPIO[j], 0, 0);
		            }

                    // update pinmux config
                    if (max_pin == SDIO_PIN_DATA4)
                        nvt_mmc_pinmux_update(SDIO_2_PINMUX,SDIO_2_WIDTH,0,TRUE);
                    else
                        nvt_mmc_pinmux_update(SDIO_2_PINMUX,SDIO_2_WIDTH,1,TRUE);
#else
                    max_pin = SDIO_PIN_DATA4;

                    // set CMD/DATA GPIO direction to input to avoid card identification in SPI mode
                    for (j = SDIO_PIN_CMD; j < max_pin; j++) {
			            nvt_set_gpio_inout(SDIO_2_GPIO[j], 0, 0);
		            }

                    // update pinmux config
                    nvt_mmc_pinmux_update(SDIO_2_PINMUX,SDIO_2_WIDTH,0,TRUE);
#endif   
                    /* SD spec rev9.10 section 6:
		             * Pull-up resistors protect the CMD and the DAT lines against bus floating before card identification.
		             */
                    // set PAD pull without CLK pin
                    for (j = SDIO_PIN_CMD; j < max_pin; j++) {
			            nvt_pad_set_pull_updown(SDIO_2_PAD_REG[j], SDIO_2_PAD_MASK[j], SDIO_2_PAD_SHIFT[j], PAD_PULLUP);
		            }
                    // set CLK PAD pullnone
                    nvt_pad_set_pull_updown(SDIO_2_PAD_REG[SDIO_PIN_CLK], SDIO_2_PAD_MASK[SDIO_PIN_CLK], SDIO_2_PAD_SHIFT[SDIO_PIN_CLK], PAD_PULLNONE);
                } 
                // SDIO1_3
                else if (i == SDIO1_3_EMB) {
                    // check 4-bits/8-bits data 
#ifdef PIN_SDIO_CFG_SDIO_BUS_WIDTH
                    if (host->enable_8bits) {
                        max_pin = SDIO_PIN_NUM;
                    } else {
                        max_pin = SDIO_PIN_DATA4;
                    }

                    // set CMD/DATA GPIO direction to input to avoid card identification in SPI mode
                    for (j = SDIO_PIN_CMD; j < max_pin; j++) {
			            nvt_set_gpio_inout(SDIO_3_GPIO[j], 0, 0);
		            }

                    // update pinmux config
                    if (max_pin == SDIO_PIN_DATA4)
                        nvt_mmc_pinmux_update(SDIO_3_PINMUX,SDIO_3_WIDTH,0,TRUE);
                    else
                        nvt_mmc_pinmux_update(SDIO_3_PINMUX,SDIO_3_WIDTH,1,TRUE);
#else
                    max_pin = SDIO_PIN_DATA4;

                    // set CMD/DATA GPIO direction to input to avoid card identification in SPI mode
                    for (j = SDIO_PIN_CMD; j < max_pin; j++) {
			            nvt_set_gpio_inout(SDIO_3_GPIO[j], 0, 0);
		            }

                    // update pinmux config
                    nvt_mmc_pinmux_update(SDIO_3_PINMUX,SDIO_3_WIDTH,0,TRUE);
#endif      
                    /* SD spec rev9.10 section 6:
		             * Pull-up resistors protect the CMD and the DAT lines against bus floating before card identification.
		             */
                    // set PAD pull without CLK pin
                    for (j = SDIO_PIN_CMD; j < max_pin; j++) {
			            nvt_pad_set_pull_updown(SDIO_3_PAD_REG[j], SDIO_3_PAD_MASK[j], SDIO_3_PAD_SHIFT[j], PAD_PULLUP);
		            }
                    // set CLK PAD pullnone
                    nvt_pad_set_pull_updown(SDIO_3_PAD_REG[SDIO_PIN_CLK], SDIO_3_PAD_MASK[SDIO_PIN_CLK], SDIO_3_PAD_SHIFT[SDIO_PIN_CLK], PAD_PULLNONE);
                }
            }
#if (SDIO_HOST_ID_COUNT > 1)
        if ((host->pinmux_value & SDIO_SUP[i]) && \
            (host->id == SDIO_HOST_ID_2)) {
                // SDIO2_1
                if (i == SDIO2_1_EMB) {
                    // check 4-bits/8-bits data 
#ifdef PIN_SDIO_CFG_SDIO2_BUS_WIDTH
                    if (host->enable_8bits) {
                        max_pin = SDIO_PIN_NUM;
                    } else {
                        max_pin = SDIO_PIN_DATA4;
                    }

                    // set CMD/DATA GPIO direction to input to avoid card identification in SPI mode
                    for (j = SDIO_PIN_CMD; j < max_pin; j++) {
			            nvt_set_gpio_inout(SDIO2_1_GPIO[j], 0, 0);
		            }

                    // update pinmux config
                    if (max_pin == SDIO_PIN_DATA4)
                        nvt_mmc_pinmux_update(SDIO2_1_PINMUX,SDIO2_1_WIDTH,0,TRUE);
                    else
                        nvt_mmc_pinmux_update(SDIO2_1_PINMUX,SDIO2_1_WIDTH,1,TRUE);
#else
                    max_pin = SDIO_PIN_DATA4;
                    // set CMD/DATA GPIO direction to input to avoid card identification in SPI mode
                    for (j = SDIO_PIN_CMD; j < max_pin; j++) {
			            nvt_set_gpio_inout(SDIO2_1_GPIO[j], 0, 0);
		            }

                    // update pinmux config
                    nvt_mmc_pinmux_update(SDIO2_1_PINMUX,SDIO2_1_WIDTH,0,TRUE);
#endif 
                    /* SD spec rev9.10 section 6:
		             * Pull-up resistors protect the CMD and the DAT lines against bus floating before card identification.
		             */
                    // set PAD pull without CLK pin
                    for (j = SDIO_PIN_CMD; j < max_pin; j++) {
			            nvt_pad_set_pull_updown(SDIO2_1_PAD_REG[j], SDIO2_1_PAD_MASK[j], SDIO2_1_PAD_SHIFT[j], PAD_PULLUP);
		            }
                    // set CLK PAD pullnone
                    nvt_pad_set_pull_updown(SDIO2_1_PAD_REG[SDIO_PIN_CLK], SDIO2_1_PAD_MASK[SDIO_PIN_CLK], SDIO2_1_PAD_SHIFT[SDIO_PIN_CLK], PAD_PULLNONE);
                }
                // SDIO2_2
                else if (i == SDIO2_2_EMB) {
                    // check 4-bits/8-bits data 
#ifdef PIN_SDIO_CFG_SDIO2_BUS_WIDTH
                    if (host->enable_8bits) {
                        max_pin = SDIO_PIN_NUM;
                    } else {
                        max_pin = SDIO_PIN_DATA4;
                    }

                    // set CMD/DATA GPIO direction to input to avoid card identification in SPI mode
                    for (j = SDIO_PIN_CMD; j < max_pin; j++) {
			            nvt_set_gpio_inout(SDIO2_2_GPIO[j], 0, 0);
		            }

                    // update pinmux config
                    if (max_pin == SDIO_PIN_DATA4)
                        nvt_mmc_pinmux_update(SDIO2_2_PINMUX,SDIO2_2_WIDTH,0,TRUE);
                    else
                        nvt_mmc_pinmux_update(SDIO2_2_PINMUX,SDIO2_2_WIDTH,1,TRUE);
#else
                    max_pin = SDIO_PIN_DATA4;

                    // set CMD/DATA GPIO direction to input to avoid card identification in SPI mode
                    for (j = SDIO_PIN_CMD; j < max_pin; j++) {
			            nvt_set_gpio_inout(SDIO2_2_GPIO[j], 0, 0);
		            }

                    // update pinmux config
                    nvt_mmc_pinmux_update(SDIO2_2_PINMUX,SDIO2_2_WIDTH,0,TRUE);
#endif 
                    /* SD spec rev9.10 section 6:
		             * Pull-up resistors protect the CMD and the DAT lines against bus floating before card identification.
		             */
                    // set PAD pull without CLK pin
                    for (j = SDIO_PIN_CMD; j < max_pin; j++) {
			            nvt_pad_set_pull_updown(SDIO2_2_PAD_REG[j], SDIO2_2_PAD_MASK[j], SDIO2_2_PAD_SHIFT[j], PAD_PULLUP);
		            }
                    // set CLK PAD pullnone
                    nvt_pad_set_pull_updown(SDIO2_2_PAD_REG[SDIO_PIN_CLK], SDIO2_2_PAD_MASK[SDIO_PIN_CLK], SDIO2_2_PAD_SHIFT[SDIO_PIN_CLK], PAD_PULLNONE);
                }
                
            }
#endif
#if (SDIO_HOST_ID_COUNT > 2)
        if ((host->pinmux_value & SDIO_SUP[i]) && \
            (host->id == SDIO_HOST_ID_3)) {
                // SDIO3_1
                if (i == SDIO3_1_EMB) {
                    // check 4-bits/8-bits data 
#ifdef PIN_SDIO_CFG_SDIO3_BUS_WIDTH
                    if (host->enable_8bits) {
                        max_pin = SDIO_PIN_NUM;
                    } else {
                        max_pin = SDIO_PIN_DATA4;
                    }

                    // set CMD/DATA GPIO direction to input to avoid card identification in SPI mode
                    for (j = SDIO_PIN_CMD; j < max_pin; j++) {
			            nvt_set_gpio_inout(SDIO3_1_GPIO[j], 0, 0);
		            }

                    // update pinmux config
                    if (max_pin == SDIO_PIN_DATA4)
                        nvt_mmc_pinmux_update(SDIO3_1_PINMUX,SDIO3_1_WIDTH,0,TRUE);
                    else
                        nvt_mmc_pinmux_update(SDIO3_1_PINMUX,SDIO3_1_WIDTH,1,TRUE);
#else
                    max_pin = SDIO_PIN_DATA4;

                    // set CMD/DATA GPIO direction to input to avoid card identification in SPI mode
                    for (j = SDIO_PIN_CMD; j < max_pin; j++) {
			            nvt_set_gpio_inout(SDIO3_1_GPIO[j], 0, 0);
		            }

                    // update pinmux config
                    nvt_mmc_pinmux_update(SDIO3_1_PINMUX,SDIO3_1_WIDTH,0,TRUE);
#endif 
                    /* SD spec rev9.10 section 6:
		             * Pull-up resistors protect the CMD and the DAT lines against bus floating before card identification.
		             */
                    // set PAD pull without CLK pin
                    for (j = SDIO_PIN_CMD; j < max_pin; j++) {
			            nvt_pad_set_pull_updown(SDIO3_1_PAD_REG[j], SDIO3_1_PAD_MASK[j], SDIO3_1_PAD_SHIFT[j], PAD_PULLUP);
		            }
                    // set CLK PAD pullnone
                    nvt_pad_set_pull_updown(SDIO3_1_PAD_REG[SDIO_PIN_CLK], SDIO3_1_PAD_MASK[SDIO_PIN_CLK], SDIO3_1_PAD_SHIFT[SDIO_PIN_CLK], PAD_PULLNONE);
                }
            }
#endif
    }

    /* SD spec rev9.10 section 6.4.1.1:
	 * A device shall be ready to accept the first command within 1ms from detecting VDD min.
	 * Device may use up to 74 clocks for preparation before receiving the first command.
	 */
	udelay(1 * 1000);
}

void sdiohost_power_down(struct mmc_nvt_host *host)
{
	u8 i,j;
    uint8_t max_pin = SDIO_PIN_DATA4;

    if (host->is1v8wifi) {
        return;
    }

    for (i = 0; i < SDIO_MAX_EMB; i++) {
        if ((host->pinmux_value & SDIO_SUP[i]) && \
            (host->id == SDIO_HOST_ID_1)) {
                // SDIO1_1
                if (i == SDIO1_1_EMB) {
                    // check 4-bits/8-bits data 
#ifdef PIN_SDIO_CFG_SDIO_BUS_WIDTH
                    if (host->enable_8bits) {
                        max_pin = SDIO_PIN_NUM;
                    } else {
                        max_pin = SDIO_PIN_DATA4;
                    }

                    // update pinmux config
                    if (max_pin == SDIO_PIN_DATA4)
                        nvt_mmc_pinmux_update(SDIO_1_PINMUX,SDIO_1_WIDTH,0,FALSE);
                    else
                        nvt_mmc_pinmux_update(SDIO_1_PINMUX,SDIO_1_WIDTH,1,FALSE);
#else
                    max_pin = SDIO_PIN_DATA4;
                    // update pinmux config
                    nvt_mmc_pinmux_update(SDIO_1_PINMUX,SDIO_1_WIDTH,0,FALSE);
#endif   
                    // set CMD/DATA GPIO direction to input to avoid card identification in SPI mode
                    for (j = SDIO_PIN_CMD; j < max_pin; j++) {
			            nvt_set_gpio_inout(SDIO_1_GPIO[j], 0, 0);
		            }
                    // set PAD pull without CLK pin
                    for (j = SDIO_PIN_CMD; j < max_pin; j++) {
			            nvt_pad_set_pull_updown(SDIO_1_PAD_REG[j], SDIO_1_PAD_MASK[j], SDIO_1_PAD_SHIFT[j], PAD_PULLDOWN);
		            }
                    // set CLK PAD pullnone
                    nvt_pad_set_pull_updown(SDIO_1_PAD_REG[SDIO_PIN_CLK], SDIO_1_PAD_MASK[SDIO_PIN_CLK], \
                    SDIO_1_PAD_SHIFT[SDIO_PIN_CLK], PAD_PULLNONE);
                    // set CLK GPIO direction to output low to avoid current leakage
                    nvt_set_gpio_inout(SDIO_1_GPIO[SDIO_PIN_CLK], 1, 0);
                }
                // SDIO1_2 
                else if (i == SDIO1_2_EMB) {
                    // check 4-bits/8-bits data 
#ifdef PIN_SDIO_CFG_SDIO_BUS_WIDTH
                    if (host->enable_8bits) {
                        max_pin = SDIO_PIN_NUM;
                    } else {
                        max_pin = SDIO_PIN_DATA4;
                    }

                    // update pinmux config
                    if (max_pin == SDIO_PIN_DATA4)
                        nvt_mmc_pinmux_update(SDIO_2_PINMUX,SDIO_2_WIDTH,0,FALSE);
                    else
                        nvt_mmc_pinmux_update(SDIO_2_PINMUX,SDIO_2_WIDTH,1,FALSE);
#else
                    max_pin = SDIO_PIN_DATA4;
                    // update pinmux config
                    nvt_mmc_pinmux_update(SDIO_2_PINMUX,SDIO_2_WIDTH,0,FALSE);
#endif 
                    // set CMD/DATA GPIO direction to input to avoid card identification in SPI mode
                    for (j = SDIO_PIN_CMD; j < max_pin; j++) {
			            nvt_set_gpio_inout(SDIO_2_GPIO[j], 0, 0);
		            }
                    // set PAD pull without CLK pin
                    for (j = SDIO_PIN_CMD; j < max_pin; j++) {
			            nvt_pad_set_pull_updown(SDIO_2_PAD_REG[j], SDIO_2_PAD_MASK[j], SDIO_2_PAD_SHIFT[j], PAD_PULLDOWN);
		            }
                    // set CLK PAD pullnone
                    nvt_pad_set_pull_updown(SDIO_2_PAD_REG[SDIO_PIN_CLK], SDIO_2_PAD_MASK[SDIO_PIN_CLK], \
                    SDIO_2_PAD_SHIFT[SDIO_PIN_CLK], PAD_PULLNONE);
                    // set CLK GPIO direction to output low to avoid current leakage
                    nvt_set_gpio_inout(SDIO_2_GPIO[SDIO_PIN_CLK], 1, 0);
                } 
                // SDIO1_3
                else if (i == SDIO1_3_EMB) {
                    // check 4-bits/8-bits data 
#ifdef PIN_SDIO_CFG_SDIO_BUS_WIDTH
                    if (host->enable_8bits) {
                        max_pin = SDIO_PIN_NUM;
                    } else {
                        max_pin = SDIO_PIN_DATA4;
                    }

                    // update pinmux config
                    if (max_pin == SDIO_PIN_DATA4)
                        nvt_mmc_pinmux_update(SDIO_3_PINMUX,SDIO_3_WIDTH,0,FALSE);
                    else
                        nvt_mmc_pinmux_update(SDIO_3_PINMUX,SDIO_3_WIDTH,1,FALSE);
#else
                    max_pin = SDIO_PIN_DATA4;
                    // update pinmux config
                    nvt_mmc_pinmux_update(SDIO_3_PINMUX,SDIO_3_WIDTH,0,FALSE);
#endif   
                    // set CMD/DATA GPIO direction to input to avoid card identification in SPI mode
                    for (j = SDIO_PIN_CMD; j < max_pin; j++) {
			            nvt_set_gpio_inout(SDIO_3_GPIO[j], 0, 0);
		            }
                    // set PAD pull without CLK pin
                    for (j = SDIO_PIN_CMD; j < max_pin; j++) {
			            nvt_pad_set_pull_updown(SDIO_3_PAD_REG[j], SDIO_3_PAD_MASK[j], SDIO_3_PAD_SHIFT[j], PAD_PULLDOWN);
		            }
                    // set CLK PAD pullnone
                    nvt_pad_set_pull_updown(SDIO_3_PAD_REG[SDIO_PIN_CLK], SDIO_3_PAD_MASK[SDIO_PIN_CLK], \
                    SDIO_3_PAD_SHIFT[SDIO_PIN_CLK], PAD_PULLNONE);
                    // set CLK GPIO direction to output low to avoid current leakage
                    nvt_set_gpio_inout(SDIO_3_GPIO[SDIO_PIN_CLK], 1, 0);
                }
            }
#if (SDIO_HOST_ID_COUNT > 1)        
        if ((host->pinmux_value & SDIO_SUP[i]) && \
            (host->id == SDIO_HOST_ID_2)) {
                if (i == SDIO2_1_EMB) {
                    // check 4-bits/8-bits data 
#ifdef PIN_SDIO_CFG_SDIO2_BUS_WIDTH
                    if (host->enable_8bits) {
                        max_pin = SDIO_PIN_NUM;
                    } else {
                        max_pin = SDIO_PIN_DATA4;
                    }

                    // update pinmux config
                    if (max_pin == SDIO_PIN_DATA4)
                        nvt_mmc_pinmux_update(SDIO2_1_PINMUX,SDIO2_1_WIDTH,0,FALSE);
                    else
                        nvt_mmc_pinmux_update(SDIO2_1_PINMUX,SDIO2_1_WIDTH,1,FALSE);
#else
                    max_pin = SDIO_PIN_DATA4;
                    // update pinmux config
                    nvt_mmc_pinmux_update(SDIO2_1_PINMUX,SDIO2_1_WIDTH,0,FALSE);
#endif   
                    // set CMD/DATA GPIO direction to input to avoid card identification in SPI mode
                    for (j = SDIO_PIN_CMD; j < max_pin; j++) {
			            nvt_set_gpio_inout(SDIO2_1_GPIO[j], 0, 0);
		            }
                    // set PAD pull without CLK pin
                    for (j = SDIO_PIN_CMD; j < max_pin; j++) {
			            nvt_pad_set_pull_updown(SDIO2_1_PAD_REG[j], SDIO2_1_PAD_MASK[j], SDIO2_1_PAD_SHIFT[j], PAD_PULLDOWN);
		            }
                    // set CLK PAD pullnone
                    nvt_pad_set_pull_updown(SDIO2_1_PAD_REG[SDIO_PIN_CLK], SDIO2_1_PAD_MASK[SDIO_PIN_CLK], \
                    SDIO2_1_PAD_SHIFT[SDIO_PIN_CLK], PAD_PULLNONE);
                    // set CLK GPIO direction to output low to avoid current leakage
                    nvt_set_gpio_inout(SDIO2_1_GPIO[SDIO_PIN_CLK], 1, 0);
                }
                // SDIO2_2
                else if (i == SDIO2_2_EMB) {
                    // check 4-bits/8-bits data 
#ifdef PIN_SDIO_CFG_SDIO2_BUS_WIDTH
                    if (host->enable_8bits) {
                        max_pin = SDIO_PIN_NUM;
                    } else {
                        max_pin = SDIO_PIN_DATA4;
                    }

                    // update pinmux config
                    if (max_pin == SDIO_PIN_DATA4)
                        nvt_mmc_pinmux_update(SDIO2_2_PINMUX,SDIO2_2_WIDTH,0,FALSE);
                    else
                        nvt_mmc_pinmux_update(SDIO2_2_PINMUX,SDIO2_2_WIDTH,1,FALSE); 
#else
                    max_pin = SDIO_PIN_DATA4;
                    // update pinmux config
                    nvt_mmc_pinmux_update(SDIO2_2_PINMUX,SDIO2_2_WIDTH,0,FALSE);
#endif 
                    // set CMD/DATA GPIO direction to input to avoid card identification in SPI mode
                    for (j = SDIO_PIN_CMD; j < max_pin; j++) {
			            nvt_set_gpio_inout(SDIO2_2_GPIO[j], 0, 0);
		            }
                    // set PAD pull without CLK pin
                    for (j = SDIO_PIN_CMD; j < max_pin; j++) {
			            nvt_pad_set_pull_updown(SDIO2_2_PAD_REG[j], SDIO2_2_PAD_MASK[j], SDIO2_2_PAD_SHIFT[j], PAD_PULLDOWN);
		            }
                    // set CLK PAD pullnone
                    nvt_pad_set_pull_updown(SDIO2_2_PAD_REG[SDIO_PIN_CLK], SDIO2_2_PAD_MASK[SDIO_PIN_CLK], \
                    SDIO2_2_PAD_SHIFT[SDIO_PIN_CLK], PAD_PULLNONE);
                    // set CLK GPIO direction to output low to avoid current leakage
                    nvt_set_gpio_inout(SDIO2_2_GPIO[SDIO_PIN_CLK], 1, 0);
                }
            }
#endif
#if (SDIO_HOST_ID_COUNT > 2)
        if ((host->pinmux_value & SDIO_SUP[i]) && \
            (host->id == SDIO_HOST_ID_3)) {
                // SDIO3_1
                if (i == SDIO3_1_EMB) {
                    // check 4-bits/8-bits data 
#ifdef PIN_SDIO_CFG_SDIO3_BUS_WIDTH
                    if (host->enable_8bits) {
                        max_pin = SDIO_PIN_NUM;
                    } else {
                        max_pin = SDIO_PIN_DATA4;
                    }

                    // update pinmux config
                    if (max_pin == SDIO_PIN_DATA4)
                        nvt_mmc_pinmux_update(SDIO3_1_PINMUX,SDIO3_1_WIDTH,0,FALSE);
                    else
                        nvt_mmc_pinmux_update(SDIO3_1_PINMUX,SDIO3_1_WIDTH,1,FALSE);
#else
                    max_pin = SDIO_PIN_DATA4;
                    // update pinmux config
                    nvt_mmc_pinmux_update(SDIO3_1_PINMUX,SDIO3_1_WIDTH,0,FALSE);
#endif  
                    // set CMD/DATA GPIO direction to input to avoid card identification in SPI mode
                    for (j = SDIO_PIN_CMD; j < max_pin; j++) {
			            nvt_set_gpio_inout(SDIO3_1_GPIO[j], 0, 0);
		            }
                    // set PAD pull without CLK pin
                    for (j = SDIO_PIN_CMD; j < max_pin; j++) {
			            nvt_pad_set_pull_updown(SDIO3_1_PAD_REG[j], SDIO3_1_PAD_MASK[j], SDIO3_1_PAD_SHIFT[j], PAD_PULLDOWN);
		            }
                    // set CLK PAD pullnone
                    nvt_pad_set_pull_updown(SDIO3_1_PAD_REG[SDIO_PIN_CLK], SDIO3_1_PAD_MASK[SDIO_PIN_CLK], \
                    SDIO3_1_PAD_SHIFT[SDIO_PIN_CLK], PAD_PULLNONE);
                    // set CLK GPIO direction to output low to avoid current leakage
                    nvt_set_gpio_inout(SDIO3_1_GPIO[SDIO_PIN_CLK], 1, 0);
                }
            }
#endif
    }

    // turn off power
    if (host->card_power[0]) {
        gpio_request(host->card_power[1], "sdio_power");
        gpio_direction_output(host->card_power[1], !host->card_power[2]);
        if (host->card_power[3] > 0) {
            udelay(host->card_power[3] * 1000);
        }
    }

    /* SD spec rev9.10 section 6.4.1.1:
	 * A device shall be ready to accept the first command within 1ms from detecting VDD min.
	 * Device may use up to 74 clocks for preparation before receiving the first command.
	 */
	udelay(1 * 1000);
}

void sdiohost_power_cycle(struct mmc_nvt_host *host, uint32_t delay_ms)
{
	// turn off power
    if (host->card_power[0]) {
        gpio_request(host->card_power[1], "sdio_power");
        gpio_direction_output(host->card_power[1], !host->card_power[2]);
        if (host->card_power[3] > 0) {
             udelay(host->card_power[3] * 1000);
        }
    }

	sdiohost_power_down(host);
	udelay(delay_ms * 1000);
	sdiohost_power_up(host);
}