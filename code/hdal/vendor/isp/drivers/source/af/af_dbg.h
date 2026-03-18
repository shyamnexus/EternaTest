#ifndef _AF_DBG_H_
#define _AF_DBG_H_

#if defined(__LINUX) && defined(__KERNEL__)
#include <linux/seq_file.h>

#define vk_pr_warn(fmt, args...)    printk(fmt, ## args);
#define vk_printk(fmt, args...)     printk(fmt, ## args);
#define PROFILE_TASK_BUSY()
#define PROFILE_TASK_IDLE()

extern unsigned int af_debug_level;

#elif defined(__FREERTOS)
#include <stdio.h>
#define vk_pr_warn printf
#define vk_printk printf

extern void profile_task_busy(void);
extern void profile_task_idle(void);
#define PROFILE_TASK_BUSY() profile_task_busy()
#define PROFILE_TASK_IDLE() profile_task_idle()

#ifndef af_debug_level
#define af_debug_level 4
#endif

#ifndef unlikely
#define unlikely(x) (x)
#endif

#else
#error Not supported OS
#endif /* __LINUX */

#define AF_DBG_FATAL     0
#define AF_DBG_ERR       1
#define AF_DBG_WRN       2
#define AF_DBG_MSG_      3
#define AF_DBG_IND       4
#define AF_DBG_FUNC      5
		
#define nvt_dbg_level(level, fmt, args...) \
		do { \
			if (unlikely(AF_DBG_##level <= af_debug_level)) { \
				vk_pr_warn("%s:" fmt, __func__, ##args); \
			} \
		} while (0)
		
#define DBG_ERR(fmt, args...)  nvt_dbg_level(ERR, fmt, ##args)
#define DBG_WRN(fmt, args...)  nvt_dbg_level(WRN, fmt, ##args)
#define DBG_MSG(fmt, args...)  nvt_dbg_level(MSG_, fmt, ##args)
#define DBG_DUMP(fmt, args...) vk_pr_warn(fmt, ##args)

#if defined(__KERNEL__)
#define PRINT_AF_INFO(sfile, fmt, args...) {seq_printf(sfile, fmt, ## args); }
#else
#define PRINT_AF_INFO(sfile, fmt, args...) {DBG_DUMP(fmt, ## args); }
#endif

#endif

