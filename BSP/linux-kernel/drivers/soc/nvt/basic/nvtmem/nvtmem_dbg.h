#ifndef __NVT_FRAMMAP_DBG_H__
#define __NVT_FRAMMAP_DBG_H__

#define NVT_FRAMMAP_DBG_IND                    0x00000001
#define NVT_FRAMMAP_DBG_WRN                    0x00000002
#define NVT_FRAMMAP_DBG_ERR                    0x00000004
#define NVT_FRAMMAP_DBG_MSG                    0x00000008

#define NVT_FRAMMAP_DEBUG

#ifdef NVT_FRAMMAP_DEBUG
extern unsigned int nvt_frammap_debug_level;
#define nvt_dbg(level, fmt, args...)                \
do {                                               \
	if (NVT_FRAMMAP_DBG_##level & nvt_frammap_debug_level)    \
	printk("%s:" fmt, __func__, ##args);   \
} while (0)
#else
#define nvt_dbg(level, fmt, args...)
#endif

#define DBG_ERR(fmt, args...) nvt_dbg(ERR, fmt, ##args)
#define DBG_WRN(fmt, args...) nvt_dbg(WRN, fmt, ##args)
#define DBG_IND(fmt, args...) nvt_dbg(IND, fmt, ##args)
#define DBG_MSG(fmt, args...) nvt_dbg(MSG, fmt, ##args)

#endif
