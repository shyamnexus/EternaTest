#ifndef _KFLOW_AI_ISP_DEBUG_H_
#define _KFLOW_AI_ISP_DEBUG_H_

//check linux version, see NA51107-275
#if defined(__LINUX) && defined(__KERNEL__)
#include <linux/version.h>
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 15, 0))
#define AI_LINUX_OWN_STDARG 1
#else
#define AI_LINUX_OWN_STDARG 0
#endif
#else //not Linux kernel
#define AI_LINUX_OWN_STDARG 0
#endif

#if AI_LINUX_OWN_STDARG
#include <linux/stdarg.h>
#else
#include <stdarg.h>
#endif

#include <kwrap/type.h>

#if defined(__LINUX) && defined(__KERNEL__)
#include <linux/printk.h>
#define vk_pr_warn pr_warn
#define vk_printk printk
#define vk_print_isr printk
#define vos_dump_stack dump_stack

#elif defined(__FREERTOS)
#define vk_pr_warn debug_msg_isr
#define vk_printk debug_msg_isr
#define vk_print_isr debug_msg_isr

#ifndef unlikely
#define unlikely(x) (x)
#endif

#ifndef KERN_EMERG
#define KERN_EMERG ""
#endif

#else
#include <stdio.h>
#define vk_pr_warn printf
#define vk_printk printf
#define vk_print_isr printf

#ifndef unlikely
#define unlikely(x) (x)
#endif

#endif

#define NVT_DBG_FATAL     0
#define NVT_DBG_ERR       1
#define NVT_DBG_WRN       2
#define NVT_DBG_UNIT      3
#define NVT_DBG_FUNC      4
#define NVT_DBG_IND       5
#define NVT_DBG_MSG       6
#define NVT_DBG_VALUE     7
#define NVT_DBG_USER      8

extern unsigned int kflow_ai_isp_debug_level;

#define nvt_dbg(level, fmt, args...) do { \
	if (unlikely(NVT_DBG_##level <= kflow_ai_isp_debug_level)) { \
		vk_pr_warn(fmt, ##args); \
	} \
} while (0)

#define DBG_FATAL(fmt, args...)         nvt_dbg(FATAL, __CLASS__" FATAL:%s() " fmt, __func__, ##args)
#define DBG_ERR(fmt, args...)           nvt_dbg(ERR,   __CLASS__" ERR:%s() "   fmt, __func__, ##args)
#define DBG_WRN(fmt, args...)           nvt_dbg(WRN,   __CLASS__" WRN:%s() "   fmt, __func__, ##args)
#define DBG_UNIT(fmt, args...)          nvt_dbg(UNIT,  __CLASS__" UNIT:%s() "  fmt, __func__, ##args)
#define DBG_FUNC_BEGIN(fmt, args...)    nvt_dbg(FUNC,  __CLASS__" %s():begin " fmt, __func__, ##args)
#define DBG_FUNC(fmt, args...)          nvt_dbg(FUNC,  __CLASS__" %s(): "      fmt, __func__, ##args)
#define DBG_FUNC_END(fmt, args...)      nvt_dbg(FUNC,  __CLASS__" %s():end "   fmt, __func__, ##args)
#define DBG_IND(fmt, args...)           nvt_dbg(IND,   __CLASS__" %s() "       fmt, __func__, ##args)
#define DBG_MSG(fmt, args...)           nvt_dbg(MSG,   __CLASS__" " fmt, ##args)
#define DBG_VALUE(fmt, args...)         nvt_dbg(VALUE, __CLASS__" " fmt, ##args)
#define DBG_USER(fmt, args...)          nvt_dbg(USER,  __CLASS__" " fmt, ##args)
#define DBG_DUMP(fmt, args...)          vk_pr_warn(fmt, ##args)

#define _ANSI_BN_    "\x1B[1;30m"   //~N -- for Bright-Gray string      "30"
#define _ANSI_BR_    "\x1B[1;31m"   //~R -- for Bright-Red string       "31"
#define _ANSI_BG_    "\x1B[1;32m"   //~G -- for Bright-Green string     "32"
#define _ANSI_BY_    "\x1B[1;33m"   //~Y -- for Bright-Yellow string    "33"
#define _ANSI_BB_    "\x1B[1;34m"   //~B -- for Bright-Blue string      "34"
#define _ANSI_BM_    "\x1B[1;35m"   //~M -- for Bright-Magenta string   "35"
#define _ANSI_BC_    "\x1B[1;36m"   //~C -- for Bright-Cyan string      "36"
#define _ANSI_BW_    "\x1B[1;37m"   //~W -- for Bright-White string     "37"
#define _ANSI_N_     "\x1B[0;30m"   //^N -- for Gray (Normal) string    "30"
#define _ANSI_R_     "\x1B[0;31m"   //^R -- for Red string              "31"
#define _ANSI_G_     "\x1B[0;32m"   //^G -- for Green string            "32"
#define _ANSI_Y_     "\x1B[0;33m"   //^Y -- for Yellow string           "33"
#define _ANSI_B_     "\x1B[0;34m"   //^B -- for Blue string             "34"
#define _ANSI_M_     "\x1B[0;35m"   //^M -- for Magenta string          "35"
#define _ANSI_C_     "\x1B[0;36m"   //^C -- for Cyan string             "36"
#define _ANSI_W_     "\x1B[0;37m"   //^W -- for White string            "37"
#define _ANSI_0_     "\x1B[0m"   //(Reset)


#define DBG_OUT_PATH  "/mnt/sd/ai_dumpbuf"

#define CHKPNT    vk_pr_warn("AI:CHK: %d, %s\r\n", __LINE__, __func__)
#define DBGD(x)   vk_pr_warn("AI:DBG: %s=%d\r\n", #x, x)
#define DBGH(x)   vk_pr_warn("AI:DBG: %s=0x%08X\r\n", #x, x)

#define AIISP_INPUT	0x00000001  // nnisp input
#define AIISP_FLOW	0x00000010
#define AIISP_BUF	0x00000100
#define AIISP_PERF	0x00001000

#define AIISP_DUMP	0x10000000	//performance
#define AI_RES	0x20000000	//resource

#if defined(__LINUX) && defined(__KERNEL__)
extern void kflow_msg_out(const char *fmtstr, ...);
#define DBG_DUMP2(fmt, args...)          kflow_msg_out(fmt, ##args)
#endif

#define PROF                DISABLE
#if PROF
	static struct timeval tstart, tend;
	#define PROF_START()    do_gettimeofday(&tstart);
	#define PROF_END(msg)   do_gettimeofday(&tend);     \
			DBG_DUMP("%s time (us): %lu\r\n", msg,        \
					(tend.tv_sec - tstart.tv_sec) * 1000000 + (tend.tv_usec - tstart.tv_usec));
#else
	#define PROF_START()
	#define PROF_END(msg)
#endif


#define ASSERT(x)                                                       \
	do {    if (x) break;                                               \
		vk_printk(KERN_EMERG "### ASSERTION FAILED %s: %s: %d: %s\n",      \
			   __FILE__, __func__, __LINE__, #x); vos_dump_stack(); vos_debug_halt();  \
	} while (0)


#if defined(__FREERTOS)
int debug_msg_isr(const char *fmtstr, ...);
#endif

void debug_dumpmem(ULONG addr, ULONG length); ///< dump memory region to console output
extern void kflow_isp_net_trace(UINT32 proc_id, UINT32 class_bits, const char *fmtstr, ...) ; 

#endif //_KFLOW_AI_DEBUG_H_


