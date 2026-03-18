#ifndef __NVT_IVOT_MMCPLAT_H__
#define __NVT_IVOT_MMCPLAT_H__

#if (defined(CONFIG_TARGET_NA51055) || defined(CONFIG_TARGET_NA51055_A64))
#include "nvt_ivot_mmc_na51055.h"
#define DRIVING_CONVERT(PAD_CURRENT,TYPE)   ((TYPE) == 0 ? ((PAD_CURRENT) / 3) - 1 : ((PAD_CURRENT) / 4) - 1)
#elif (defined(CONFIG_TARGET_NA51089) || defined(CONFIG_TARGET_NA51089_A64))
#include "nvt_ivot_mmc_na51089.h"
#define DRIVING_CONVERT(PAD_CURRENT,TYPE)   ((TYPE) == 0 ? ((PAD_CURRENT) / 3) - 1 : ((PAD_CURRENT) / 4) - 1)
#elif (defined(CONFIG_TARGET_NA51090) || defined(CONFIG_TARGET_NA51090_A64))
#include "nvt_ivot_mmc_na51090.h"
#define DRIVING_CONVERT(PAD_CURRENT,TYPE)   ((TYPE) == 0 ? PAD_CURRENT : PAD_CURRENT)
#elif (defined(CONFIG_TARGET_NA51102) || defined(CONFIG_TARGET_NA51102_A64))
#include "nvt_ivot_mmc_na51102.h"
#define DRIVING_CONVERT(PAD_CURRENT,TYPE)   ((TYPE) == 0 ? PAD_CURRENT : PAD_CURRENT)
#elif (defined(CONFIG_TARGET_NA51103) || defined(CONFIG_TARGET_NA51103_A64))
#include "nvt_ivot_mmc_na51103.h"
#define DRIVING_CONVERT(PAD_CURRENT,TYPE)   ((TYPE) == 0 ? PAD_CURRENT : PAD_CURRENT)
#elif (defined(CONFIG_TARGET_NS02201) || defined(CONFIG_TARGET_NS02201_A64))
#include "nvt_ivot_mmc_ns02201.h"
#define DRIVING_CONVERT(PAD_CURRENT,TYPE)   ((TYPE) == 0 ? PAD_CURRENT : PAD_CURRENT)
#elif (defined(CONFIG_TARGET_NS02301) || defined(CONFIG_TARGET_NS02301_A64))
#include "nvt_ivot_mmc_ns02301.h"
#define DRIVING_CONVERT(PAD_CURRENT,TYPE)   ((TYPE) == 0 ? PAD_CURRENT : PAD_CURRENT)
#elif (defined(CONFIG_TARGET_NS02302) || defined(CONFIG_TARGET_NS02302_A64))
#include "nvt_ivot_mmc_ns02302.h"
#define DRIVING_CONVERT(PAD_CURRENT,TYPE)   ((TYPE) == 0 ? PAD_CURRENT : PAD_CURRENT)
#elif (defined(CONFIG_TARGET_NS02401) || defined(CONFIG_TARGET_NS02401_A64))
#include "nvt_ivot_mmc_ns02401.h"
#define DRIVING_CONVERT(PAD_CURRENT,TYPE)   ((TYPE) == 0 ? PAD_CURRENT : PAD_CURRENT)
#elif (defined(CONFIG_TARGET_NS02401_V0) || defined(CONFIG_TARGET_NS02401_V0_A64))
#include "nvt_ivot_mmc_ns02401_v0.h"
#define DRIVING_CONVERT(PAD_CURRENT,TYPE)   ((TYPE) == 0 ? PAD_CURRENT : PAD_CURRENT)
#endif

#endif
