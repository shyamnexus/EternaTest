#ifndef __DEBUG_H__
#define __DEBUG_H__

extern int h26xd_dbglevel;
extern int h26xd_dbg_mode;

#define NVT_DBG_ERR     0
#define NVT_DBG_WRN     1
#define NVT_DBG_IND		2
#define NVT_DBG_MSG    	3

#define H26XD_DRV_LOG_TAG "DE"

extern void printm(char *module, const char *fmt, ...);
extern int damnit(char *fmt);

/* if dbg_level is lower than NVT_DBG_WRN, it will print out on console */
#define nvt_dbg_level(level, fmt, args...) \
do { \
	if (unlikely(NVT_DBG_##level <= h26xd_dbglevel)) { \
		if (NVT_DBG_##level <= NVT_DBG_WRN) { \
			pr_warn("[DE]:%s:" fmt, __func__, ##args); \
		} else if (h26xd_dbg_mode == 1) { \
			printm(H26XD_DRV_LOG_TAG, fmt, ##args); \
		} \
	} \
} while (0)

#define h26xd_err(fmt, args...) nvt_dbg_level(ERR, fmt, ##args)
#define h26xd_warn(fmt, args...) nvt_dbg_level(WRN, fmt, ##args)
#define h26xd_info(fmt, args...) nvt_dbg_level(IND, fmt, ##args)
#define h26xd_dbg(fmt, args...) nvt_dbg_level(MSG, fmt, ##args)

#endif /* __DEBUG_H__ */