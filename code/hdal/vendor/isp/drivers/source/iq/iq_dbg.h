#ifndef _IQ_DBG_H_
#define _IQ_DBG_H_

#include "iq_alg.h"
#include "iqt_api.h"

#if defined(__LINUX) && defined(__KERNEL__)
#include <linux/seq_file.h>

#define vk_pr_warn(fmt, args...)    printk(fmt, ## args);
#define vk_printk(fmt, args...)     printk(fmt, ## args);
#define PROFILE_TASK_BUSY()
#define PROFILE_TASK_IDLE()

extern unsigned int iq_debug_level;

#elif defined(__FREERTOS)
#include <stdio.h>
#define vk_pr_warn printf
#define vk_printk printf

extern void profile_task_busy(void);
extern void profile_task_idle(void);
#define PROFILE_TASK_BUSY() profile_task_busy()
#define PROFILE_TASK_IDLE() profile_task_idle()

#ifndef iq_debug_level
#define iq_debug_level 4
#endif

#ifndef unlikely
#define unlikely(x) (x)
#endif

#else
#error Not supported OS
#endif /* __LINUX */

#define IQ_DBG_FATAL     0
#define IQ_DBG_ERR       1
#define IQ_DBG_WRN       2
#define IQ_DBG_MSG       3
#define IQ_DBG_IND       4
#define IQ_DBG_FUNC      5
		
#define nvt_dbg_level(level, fmt, args...) \
		do { \
			if (unlikely(IQ_DBG_##level <= iq_debug_level)) { \
				vk_pr_warn("%s:" fmt, __func__, ##args); \
			} \
		} while (0)
		
#define DBG_ERR(fmt, args...)  nvt_dbg_level(ERR, fmt, ##args)
#define DBG_WRN(fmt, args...)  nvt_dbg_level(WRN, fmt, ##args)
#define DBG_MSG(fmt, args...)  nvt_dbg_level(MSG, fmt, ##args)
#define DBG_DUMP(fmt, args...) vk_pr_warn(fmt, ##args)

#define IQ_DBG_NONE             0x00000000
#define IQ_DBG_WRN_MSG          0x00000001
#define IQ_DBG_SYNC             0x00000002
#define IQ_DBG_CFG              0x00000004
#define IQ_DBG_DTS              0x00000008
#define IQ_DBG_VIG              0x00000010
#define IQ_DBG_GAMMA            0x00000020
#define IQ_DBG_CCM              0x00000040
#define IQ_DBG_TABLE            0x00000080
#define IQ_DBG_P_SIE            0x00000100
#define IQ_DBG_P_IFE            0x00000200
#define IQ_DBG_P_PRE            0x00000400
#define IQ_DBG_P_IPE            0x00000800
#define IQ_DBG_P_IME            0x00002000
#define IQ_DBG_P_ENC            0x00004000
#define IQ_DBG_P_AIISP          0x00008000
#define IQ_DBG_A_WDR            0x00010000
#define IQ_DBG_A_DEFOG          0x00020000
#define IQ_DBG_A_SHDR           0x00040000
#define IQ_DBG_A_RGBIR          0x00080000
#define IQ_DBG_O_ISO            0x00100000
#define IQ_DBG_O_EDGE           0x00200000
#define IQ_DBG_O_TMNR           0x00400000
#define IQ_DBG_O_ECS            0x00800000
#define IQ_DBG_CAPTURE          0x01000000
#define IQ_DBG_NNSC             0x02000000
#define IQ_DBG_FASTBOOT         0x04000000
#define IQ_DBG_SMOOTH           0x08000000
#define IQ_DBG_PERFORMANCE      0x10000000

#define PRINT_IQ(type, fmt, args...) {if (type) DBG_DUMP(fmt, ## args); }
#define PRINT_IQ_VAR(type, var)      {if (type) DBG_DUMP("%s = %ld\r\n", #var, (ULONG)var); }
#define PRINT_IQ_VAR_HEX(type, var)  {if (type) DBG_DUMP("%s = %lx\r\n", #var, (ULONG)var); }
#define PRINT_IQ_ARR(type, arr, len) {                                                                  \
		if (type) {                                                                                     \
			DBG_DUMP("%s = { ", #arr);                                                                  \
			if (sizeof(arr[0]) == sizeof(UINT32)) iq_print_arr_uint32((UINT32 *)arr, len, 10);          \
			else if (sizeof(arr[0]) == sizeof(UINT16)) iq_print_arr_uint16((UINT16 *)arr, len, 10);    \
			else if (sizeof(arr[0]) == sizeof(UINT8)) iq_print_arr_uint8((UINT8 *)arr, len, 10);       \
		};                                                                                              \
	}
#define PRINT_IQ_ARR_HEX(type, arr, len) {                                                              \
		if (type) {                                                                                     \
			DBG_DUMP("%s = { ", #arr);                                                                  \
			if (sizeof(arr[0]) == sizeof(UINT32)) iq_print_arr_uint32_hex((UINT32 *)arr, len, 10);     \
		};                                                                                              \
	}
#define PRINT_IQ_ARR_WIN(type, arr, len, width) {                                                       \
		if (type) {                                                                                     \
			DBG_DUMP("%s = { ", #arr);                                                                  \
			if (sizeof(arr[0]) == sizeof(UINT32)) iq_print_arr_uint32((UINT32 *)arr, len, width);      \
			else if (sizeof(arr[0]) == sizeof(UINT16)) iq_print_arr_uint16((UINT16 *)arr, len, width); \
			else if (sizeof(arr[0]) == sizeof(UINT8)) iq_print_arr_uint8((UINT8 *)arr, len, width);    \
		};                                                                                              \
	}
#define PRINT_IQ_WRN(type, fmt, args...) {if (type) DBG_WRN(fmt, ## args); if (iq_dbg_check_wrn_msg(type)) DBG_WRN(fmt, ## args); }
#if defined(__KERNEL__)
#define PRINT_IQ_INFO(sfile, fmt, args...) {seq_printf(sfile, fmt, ## args); }
#else
#define PRINT_IQ_INFO(sfile, fmt, args...) {DBG_DUMP(fmt, ## args); }
#endif

extern void iq_print_arr_uint32(UINT32 *arr, UINT32 len, UINT32 w_len);
extern void iq_print_arr_uint16(UINT16 *arr, UINT32 len, UINT32 w_len);
extern void iq_print_arr_uint8(UINT8 *arr, UINT32 len, UINT32 w_len);
extern void iq_print_arr_uint32_hex(UINT32 *arr, UINT32 len, UINT32 w_len);
extern UINT32 iq_dbg_get_dbg_mode(IQ_ID id);
extern void iq_dbg_set_dbg_mode(IQ_ID id, UINT32 cmd);
extern void iq_dbg_clr_dbg_mode(IQ_ID id, UINT32 cmd);
extern BOOL iq_dbg_check_wrn_msg(BOOL show_dbg_msg);
extern void iq_dbg_clr_wrn_msg(void);
extern UINT32 iq_dbg_get_wrn_msg(void);

#endif
