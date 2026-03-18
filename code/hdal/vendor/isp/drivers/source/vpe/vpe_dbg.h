#ifndef _VPE_DBG_H_
#define _VPE_DBG_H_

#if defined(__LINUX) && defined(__KERNEL__)
#define vk_pr_warn(fmt, args...)    printk(fmt, ## args);
#define vk_printk(fmt, args...)     printk(fmt, ## args);

extern unsigned int vpe_debug_level;

#elif defined(__FREERTOS)
#include <stdio.h>
#define vk_pr_warn printf
#define vk_printk printf

#ifndef vpe_debug_level
#define vpe_debug_level 4
#endif

#ifndef unlikely
#define unlikely(x) (x)
#endif

#else
#error Not supported OS
#endif /* __LINUX */

#define VPE_DBG_FATAL     0
#define VPE_DBG_ERR       1
#define VPE_DBG_WRN       2
#define VPE_DBG_MSG       3
#define VPE_DBG_IND       4
#define VPE_DBG_FUNC      5

#define nvt_dbg_level(level, fmt, args...) \
do { \
	if (unlikely(VPE_DBG_##level <= vpe_debug_level)) { \
		vk_pr_warn("%s:" fmt, __func__, ##args); \
	} \
} while (0)

#define DBG_ERR(fmt, args...)  nvt_dbg_level(ERR, fmt, ##args)
#define DBG_WRN(fmt, args...)  nvt_dbg_level(WRN, fmt, ##args)
#define DBG_MSG(fmt, args...)  nvt_dbg_level(MSG, fmt, ##args)
#define DBG_DUMP(fmt, args...) vk_pr_warn(fmt, ##args)

#include "vpe_alg.h"

#define VPE_DBG_NONE             0x00000000
#define VPE_DBG_VPE_CB           0x00000001
#define VPE_DBG_VPE_UPDATE       0x00000004
#define VPE_DBG_ERR_MSG          0x10000000
#define VPE_DBG_WRN_MSG          0x20000000

#define VPE_IOC_NONE             0x00000000
#define VPE_IOC_BYPASS           0x00000001
#define VPE_IOC_PRINT            0x00010000

#define PRINT_VPE(type, fmt, args...) {if (type) DBG_DUMP(fmt, ## args); }
#define PRINT_VPE_ERR(type, fmt, args...) {if (type) DBG_ERR(fmt, ## args); if (vpe_dbg_check_err_msg(type)) DBG_ERR(fmt, ## args); }
#define PRINT_VPE_WRN(type, fmt, args...) {if (type) DBG_WRN(fmt, ## args); if (vpe_dbg_check_wrn_msg(type)) DBG_WRN(fmt, ## args); }
#if defined(__KERNEL__)
#define PRINT_VPE_INFO(sfile, fmt, args...) {seq_printf(sfile, fmt, ## args); }
#else
#define PRINT_VPE_INFO(sfile, fmt, args...) {DBG_DUMP(fmt, ## args); }
#endif

extern UINT32 vpe_dbg_get_dbg_mode(VPE_ID id);
extern void vpe_dbg_set_dbg_mode(VPE_ID id, UINT32 cmd);
extern UINT32 vpe_dbg_get_ioc_control(void);
extern void vpe_dbg_set_ioc_control(UINT32 cmd);
extern BOOL vpe_dbg_check_err_msg(BOOL show_dbg_msg);
extern void vpe_dbg_clr_err_msg(void);
extern UINT32 vpe_dbg_get_err_msg(void);
extern BOOL vpe_dbg_check_wrn_msg(BOOL show_dbg_msg);
extern void vpe_dbg_clr_wrn_msg(void);
extern UINT32 vpe_dbg_get_wrn_msg(void);
extern CHAR *vpe_dbg_get_vpet_item(UINT32 item);

#endif
