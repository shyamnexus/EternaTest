#ifndef __NVT_SCE_CFG_H
#define __NVT_SCE_CFG_H

#include "nvt_pll_cfg.h"


// SCE use STBC clk
#if defined(_BSP_NA51102_) || defined(_BSP_NS02201_)
#define SCE_USE_STBC_CLK
#endif

#if defined(_BSP_NS02301_) || defined(_BSP_NS02302_)
#define TRNG_USE_STBC_CLK
#endif

#if defined(_BSP_NS02302_)
typedef enum {
    TRNG_RO_CLK_DEF = RO_PHY_CLK,
} RO_CLK;
#elif defined(_BSP_NA51090_) || defined(_BSP_NA51103_)
typedef enum {
    TRNG_RO_CLK_DEF = TRNGRO_CLK,
} RO_CLK;
#else
typedef enum {
    TRNG_RO_CLK_DEF = TRNG_RO_CLK,
} RO_CLK;
#endif

#endif