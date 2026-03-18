#ifndef _AWB_DBG_H_
#define _AWB_DBG_H_

#include "awb_alg.h"

#if defined(__LINUX) && defined(__KERNEL__)
#include <linux/seq_file.h>

#define vk_pr_warn(fmt, args...)    printk(fmt, ## args);
#define vk_printk(fmt, args...)     printk(fmt, ## args);
#define PROFILE_TASK_BUSY()
#define PROFILE_TASK_IDLE()

extern unsigned int awb_debug_level;

#elif defined(__FREERTOS)
#include <stdio.h>
#define vk_pr_warn printf
#define vk_printk printf

extern void profile_task_busy(void);
extern void profile_task_idle(void);
#define PROFILE_TASK_BUSY() profile_task_busy()
#define PROFILE_TASK_IDLE() profile_task_idle()

#ifndef awb_debug_level
#define awb_debug_level 4
#endif

#ifndef unlikely
#define unlikely(x) (x)
#endif

#else
#error Not supported OS
#endif /* __LINUX */

#define AWB_DBG_FATAL     0
#define AWB_DBG_ERR       1
#define AWB_DBG_WRN       2
#define AWB_DBG_MSG       3
#define AWB_DBG_IND       4
#define AWB_DBG_FUNC      5
		
#define nvt_dbg_level(level, fmt, args...) \
		do { \
			if (unlikely(AWB_DBG_##level <= awb_debug_level)) { \
				vk_pr_warn("%s:" fmt, __func__, ##args); \
			} \
		} while (0)
		
#define DBG_ERR(fmt, args...)  nvt_dbg_level(ERR, fmt, ##args)
#define DBG_WRN(fmt, args...)  nvt_dbg_level(WRN, fmt, ##args)
#define DBG_MSG(fmt, args...)  nvt_dbg_level(MSG, fmt, ##args)
#define DBG_DUMP(fmt, args...) vk_pr_warn(fmt, ##args)

//=============================================================================
// define & struct
//=============================================================================
#define AWB_DBG_NONE             0x00
#define AWB_DBG_SYNC             0x01
#define AWB_DBG_CFG              0x02
#define AWB_DBG_UART             0x10
#define AWB_DBG_FLOW             0x20
#define AWB_DBG_ALG              0x40
#define AWB_DBG_CA               0x80

#if defined(__FREERTOS)
#define PRINT_AWB(type, fmt, args...)  {if (type) DBG_DUMP(fmt, ## args); }
#define PRINT_AWB_INFO(sfile, fmt, args...) {DBG_DUMP(fmt, ## args); }
#else
#define PRINT_AWB(type, fmt, args...)  {if (type) printk(fmt, ## args); }
#define PRINT_AWB_INFO(sfile, fmt, args...) {seq_printf(sfile, fmt, ## args); }
#endif

extern UINT32 awb_get_dbg_mode(AWB_ID id);
extern UINT32 awb_get_dbg_freq(AWB_ID id);
extern void awb_set_dbg_mode(AWB_ID id, UINT32 level, UINT32 freq);

#endif
