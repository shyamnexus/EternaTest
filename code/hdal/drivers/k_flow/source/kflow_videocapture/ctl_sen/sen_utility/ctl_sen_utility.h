/**
    @file       ctl_sen_utility.h

    @brief      Define sensor control function

    @note      Sensor Driver APIs

    Copyright   Novatek Microelectronics Corp. 2021.  All rights reserved.
*/
#ifndef _CTL_SEN_UTILITY_H_
#define _CTL_SEN_UTILITY_H_

#include "kwrap/mem.h"

#if defined(__FREERTOS) || defined(__ECOS) || defined(__UITRON)
#include <malloc.h>
#include <string.h>
#else
#include <linux/string.h>
#include <linux/of.h>
#include <linux/slab.h>
#endif

/* common covert api */
extern UINT32 sen_uint64_dividend(UINT64 dividend, UINT32 divisor);

/* mem info */
extern INT32 ctl_sen_util_os_malloc(CTL_SEN_VOS_MEM_INFO *vod_mem_info, UINT32 req_size);
extern INT32 ctl_sen_util_os_mfree(CTL_SEN_VOS_MEM_INFO *vod_mem_info);
extern void *ctl_sen_util_os_malloc_wrap(UINT32 want_size);
extern void ctl_sen_util_os_mfree_wrap(void *p_buf);

INT32 ctl_sen_conv_senmode(CTL_SEN_MAP_HDL *map_hdl, CTL_SEN_MODE senmode_in, CTL_SEN_MODE *senmode_sendrv);
#define conv_senmode(map_hdl, senmode_in, senmode_sendrv) ctl_sen_conv_senmode((CTL_SEN_MAP_HDL *)(map_hdl), (CTL_SEN_MODE)(senmode_in), (CTL_SEN_MODE *)(senmode_sendrv))
UINT32 ctl_sen_conv_senid(CTL_SEN_MAP_HDL *map_hdl);
#define conv_senid(map_hdl) ctl_sen_conv_senid((CTL_SEN_MAP_HDL *)map_hdl)

extern INT32 ctl_sen_util_setcfg_status(CTL_SEN_CFG cfg);
#define conv_setcfg_sts(cfg) ctl_sen_util_setcfg_status((CTL_SEN_CFG)cfg)
extern INT32 ctl_sen_util_getcfg_status(CTL_SEN_CFG cfg);
#define conv_getcfg_sts(cfg) ctl_sen_util_getcfg_status((CTL_SEN_CFG)cfg)

extern CHAR *ctl_sen_util_get_drvdev_str(CTL_SEN_DRVDEV drvdev);

extern BOOL ctl_sen_util_is_combo_sensor(CTL_SEN_MAP_HDL *map_hdl);
#define conv_is_combo(map_hdl) ctl_sen_util_is_combo_sensor((CTL_SEN_MAP_HDL *)map_hdl)

#endif // _CTL_SEN_UTILITY_H_
