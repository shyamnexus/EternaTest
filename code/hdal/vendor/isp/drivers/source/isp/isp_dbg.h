#ifndef _ISP_DBG_H_
#define _ISP_DBG_H_

#include "kflow_common/isp_if.h"

#if defined(__LINUX) && defined(__KERNEL__)
#define vk_pr_warn(fmt, args...)    printk(fmt, ## args);
#define vk_printk(fmt, args...)     printk(fmt, ## args);

extern unsigned int isp_debug_level;

#define ISP_DBG_FATAL     0
#define ISP_DBG_ERR       1
#define ISP_DBG_WRN       2
#define ISP_DBG_MSG       3
#define ISP_DBG_IND       4
#define ISP_DBG_FUNC      5

#define nvt_dbg_level(level, fmt, args...) \
do { \
	if (unlikely(ISP_DBG_##level <= isp_debug_level)) { \
		vk_pr_warn("%s:" fmt, __func__, ##args); \
	} \
} while (0)

#define DBG_ERR(fmt, args...)  nvt_dbg_level(ERR, fmt, ##args)
#define DBG_WRN(fmt, args...)  nvt_dbg_level(WRN, fmt, ##args)
#define DBG_MSG(fmt, args...)  nvt_dbg_level(MSG, fmt, ##args)
#define DBG_DUMP(fmt, args...) vk_pr_warn(fmt, ##args)

#elif defined(__FREERTOS)
#define THIS_DBGLVL         6  //NVT_DBG_MSG
#define __MODULE__          isp
#define __DBGLVL__          THIS_DBGLVL
#define __DBGFLT__          "*" // *=All, [mark]=CustomClass

#include "kwrap/debug.h"

#else
#error Not supported OS
#endif /* __LINUX */

#define ISP_DBG_NONE                   0x00000000
#define ISP_DBG_SYNC_AE_STATUS         0x00000001
#define ISP_DBG_SYNC_TOTAL_GAIN        0x00000002
#define ISP_DBG_SYNC_ISP_GAIN          0x00000004
#define ISP_DBG_SYNC_D_GAIN            0x00000008
#define ISP_DBG_SYNC_LV                0x00000010
#define ISP_DBG_SYNC_CA_LA_ENABLE      0x00000020
#define ISP_DBG_SYNC_EV_RATIO          0x00000040
#define ISP_DBG_SYNC_TM_RATIO          0x00000080
#define ISP_DBG_SYNC_C_GAIN            0x00000100
#define ISP_DBG_SYNC_FINAL_C_GAIN      0x00000200
#define ISP_DBG_SYNC_CT                0x00000400
#define ISP_DBG_SYNC_HBS_PARAM         0x00000800
#define ISP_DBG_SYNC_CA_TH             0x00001000
#define ISP_DBG_SYNC_CA_ROI            0x00002000
#define ISP_DBG_SYNC_LA_ROI            0x00004000
#define ISP_DBG_SYNC_VA_ROI            0x00008000
#define ISP_DBG_SENSOR_EXPT            0x00010000
#define ISP_DBG_SENSOR_GAIN            0x00020000
#define ISP_DBG_SENSOR_REG             0x00040000
#define ISP_DBG_SYNC_CAPTURE           0x00080000
#define ISP_DBG_SIE_CB                 0x00100000
#define ISP_DBG_IPP_CB                 0x00200000
#define ISP_DBG_ENC_CB                 0x00400000
#define ISP_DBG_VIE_CB                 0x00800000
#define ISP_DBG_SIE_SET                0x01000000
#define ISP_DBG_IPP_SET                0x02000000
#define ISP_DBG_ENC_SET                0x04000000
#define ISP_DBG_AIISP_SET              0x08000000
#define ISP_DBG_ERR_MSG                0x10000000
#define ISP_DBG_WRN_MSG                0x20000000

#define ISP_BYPASS_NONE_PARAM          0x00000000
#define ISP_BYPASS_SIE_ROI             0x00000001
#define ISP_BYPASS_SIE_PARAM           0x00000002
#define ISP_BYPASS_IFE_PARAM           0x00000004
#define ISP_BYPASS_PRE_PARAM           0x00000008
#define ISP_BYPASS_IPE_PARAM           0x00000010
#define ISP_BYPASS_IME_PARAM           0x00000020
#define ISP_BYPASS_ENC_PARAM           0x00000040
#define ISP_BYPASS_AIISP_PARAM         0x00000080
#define ISP_BYPASS_IFE_VIG_CENT        0x00000100
#define ISP_BYPASS_IFE_VA_WIN_SIZE     0x00000200
#define ISP_BYPASS_IPE_VA_WIN_SIZE     0x00000400
#define ISP_BYPASS_IME_VA_WIN_SIZE     0x00000800
#define ISP_BYPASS_IME_LCA_DBG_X_POS   0x00001000
#define ISP_BYPASS_IFE_FPN_INFO        0x00002000
#define ISP_BYPASS_SEN_REG             0x00010000
#define ISP_BYPASS_SEN_EXPT            0x00020000
#define ISP_BYPASS_SEN_GAIN            0x00040000
#define ISP_BYPASS_SEN_DIR             0x00080000
#define ISP_BYPASS_SEN_SLEEP           0x00100000
#define ISP_BYPASS_SEN_WAKEUP          0x00200000

#define ISP_IOC_NONE                   0x00000000
#define ISP_IOC_BYPASS_COMMON          0x00000001
#define ISP_IOC_BYPASS_VD              0x00000002
#define ISP_IOC_BYPASS_AE              0x00000010
#define ISP_IOC_BYPASS_AF              0x00000020
#define ISP_IOC_BYPASS_AWB             0x00000040
#define ISP_IOC_BYPASS_IQ              0x00000080
#define ISP_IOC_PRINT_COMMON           0x00010000
#define ISP_IOC_PRINT_VD               0x00020000
#define ISP_IOC_PRINT_AE               0x00100000
#define ISP_IOC_PRINT_AF               0x00200000
#define ISP_IOC_PRINT_AWB              0x00400000
#define ISP_IOC_PRINT_IQ               0x00800000

#define PRINT_ISP(type, fmt, args...) {if (type) DBG_DUMP(fmt, ## args); }
#define PRINT_ISP_ERR(type, fmt, args...) {if (type) DBG_ERR(fmt, ## args); if (isp_dbg_check_err_msg(type)) DBG_ERR(fmt, ## args); }
#define PRINT_ISP_WRN(type, fmt, args...) {if (type) DBG_WRN(fmt, ## args); if (isp_dbg_check_wrn_msg(type)) DBG_WRN(fmt, ## args); }
#if defined(__KERNEL__)
#define PRINT_ISP_INFO(sfile, fmt, args...) {seq_printf(sfile, fmt, ## args); }
#else
#define PRINT_ISP_INFO(sfile, fmt, args...) {DBG_DUMP(fmt, ## args); }
#endif

extern UINT32 isp_dbg_get_dbg_mode(ISP_ID id);
extern void isp_dbg_set_dbg_mode(ISP_ID id, UINT32 cmd);
extern UINT32 isp_dbg_get_bypass_eng(ISP_ID id);
extern void isp_dbg_set_bypass_eng(ISP_ID id, UINT32 cmd);
extern UINT32 isp_dbg_get_ioc_control(void);
extern void isp_dbg_set_ioc_control(UINT32 cmd);
extern BOOL isp_dbg_check_err_msg(BOOL show_dbg_msg);
extern void isp_dbg_clr_err_msg(void);
extern UINT32 isp_dbg_get_err_msg(void);
extern BOOL isp_dbg_check_wrn_msg(BOOL show_dbg_msg);
extern void isp_dbg_clr_wrn_msg(void);
extern UINT32 isp_dbg_get_wrn_msg(void);
extern CHAR *isp_dbg_get_ispt_item(UINT32 item);
extern CHAR *isp_dbg_get_aet_item(UINT32 item);
extern CHAR *isp_dbg_get_aft_item(UINT32 item);
extern CHAR *isp_dbg_get_awbt_item(UINT32 item);
extern CHAR *isp_dbg_get_iqt_item(UINT32 item);

#endif
