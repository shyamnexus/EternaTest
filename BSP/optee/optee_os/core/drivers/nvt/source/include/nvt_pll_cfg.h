#ifndef __NVT_PLL_CFG_H__
#define __NVT_PLL_CFG_H__

#if defined(__FREERTOS) || defined(__OPTEEOS)
#if defined(_BSP_NA51055_)
#include "pll_reg/na51055_pll_int.h"
#include "pll_reg/na51055_pll_reg.h"
#elif defined(_BSP_NA51089_)
#include "pll_reg/na51089_pll_int.h"
#include "pll_reg/na51089_pll_reg.h"
#elif defined(_BSP_NA51090_)
#include "pll_reg/na51090_pll_int.h"
#include "pll_reg/na51090_pll_reg.h"
#elif defined(_BSP_NA51102_)
#include "pll_reg/na51102_pll_int.h"
#include "pll_reg/na51102_pll_reg.h"
#elif defined(_BSP_NA51103_)
#include "pll_reg/na51103_pll_int.h"
#include "pll_reg/na51103_pll_reg.h"
#elif defined(_BSP_NS02201_)
#include "pll_reg/ns02201_pll_int.h"
#include "pll_reg/ns02201_pll_reg.h"
#elif defined(_BSP_NS02301_)
#include "pll_reg/ns02301_pll_int.h"
#include "pll_reg/ns02301_pll_reg.h"
#elif defined(_BSP_NS02302_)
#include "pll_reg/ns02302_pll_int.h"
#include "pll_reg/ns02302_pll_reg.h"
#elif defined(_BSP_NS02401_)
#include "pll_reg/ns02401_pll_int.h"
#include "pll_reg/ns02401_pll_reg.h"
#endif

#endif
#endif
