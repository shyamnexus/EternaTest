/*
    Internal header file for SIE module.

    @file       sie_init_int.h
    @ingroup    mIIPPSIE

    Copyright   Novatek Microelectronics Corp. 2010.  All rights reserved.
*/

#ifndef _SIE_INIT_INT_H
#define _SIE_INIT_INT_H
#include "kwrap/error_no.h"
#include "kwrap/type.h"
#include "kwrap/debug.h"
#include "kdrv_type.h"
#include "sie_eng.h"
#include "sie_eng_int_platform.h"
#include "sie_eng_limit.h"
#include "sie_init.h"
#include "sie_init_main.h"

#if defined(__KERNEL__)
#define FDT_ERR_NOTFOUND 1
#else
#include <libfdt.h>
#endif

typedef enum _SIE_FB_DBG_LVL {
	SIE_FB_DBG_LVL_NONE = 0,
	SIE_FB_DBG_LVL_ERR,
	SIE_FB_DBG_LVL_WRN,
	SIE_FB_DBG_LVL_IND,
	SIE_FB_DBG_LVL_FUNC,
	SIE_FB_DBG_LVL_MAX,
	ENUM_DUMMY4WORD(SIE_FB_DBG_LVL)
} SIE_FB_DBG_LVL;

extern SIE_FB_DBG_LVL sie_fb_dbg_lvl;
#define sie_fb_dbg_err(fmt, args...)     { if (sie_fb_dbg_lvl >= SIE_FB_DBG_LVL_ERR) { DBG_ERR(fmt, ##args);  }}
#define sie_fb_dbg_wrn(fmt, args...)     { if (sie_fb_dbg_lvl >= SIE_FB_DBG_LVL_WRN) { DBG_WRN(fmt, ##args);  }}
#define sie_fb_dbg_ind(fmt, args...)     { if (sie_fb_dbg_lvl >= SIE_FB_DBG_LVL_IND) { DBG_DUMP("[sie]IND: "fmt, ##args); }}
#define sie_fb_dbg_func(fmt, args...)    { if (sie_fb_dbg_lvl >= SIE_FB_DBG_LVL_FUNC){ DBG_DUMP("[sie]FUNC: "fmt, ##args); }}

typedef enum  {
	SIE_INIT_NODE_CTRL,
	SIE_INIT_NODE_CLK,
	SIE_INIT_NODE_REG,
	SIE_INIT_NODE_NUM,
	ENUM_DUMMY4WORD(SIE_INIT_NODE)
} SIE_INIT_NODE;

typedef enum  {
	SIE_FB_OUT_DIRECT,
	SIE_FB_OUT_DRAM,
	ENUM_DUMMY4WORD(SIE_FB_OUT_MODE)
} SIE_FB_OUT_MODE;

typedef enum  {
	SIE_INIT_NODE_GLB_MSG,
	SIE_INIT_NODE_GLB_NUM,
	ENUM_DUMMY4WORD(SIE_INIT_NODE_GLB)
} SIE_INIT_NODE_GLB;

typedef struct {
	/* ctl param */
	SIE_ENG_HANDLE *ssdrv_hdl; // ssdrv handle
	BOOL sin_out_en[SIE_FB_OUT_CH_MAX];
	BOOL fb_streaming;					//streaming setting
 	USIZE out_size;
	UINT32 out_dest;					//0: direct mode, 1: dram mode
	UINT32 frame_count;
	SIE_BUILTIN_HEADER_INFO header_info;
	SIE_FB_BRIDGE_INFO bridge_info;
	UINT32 dupl_src_id;					//duplicate src id from dtsi
	UINT32 sen_out_dest;				//sensor output dest idx from dtsi
	UINT32 shdr_en;						//shdr function enable from dtsi
	UINT32 ref_load_id;
	UINT32 low_latency_en;				//low latency
	UINT32 one_buf_en;					//one buf mode
	UINT32 int_clk_src;
	BOOL   b_start;						//engine start flag
	BOOL   b_ref_load_id_vd;
	/* isr cb */
	SIE_FB_KDRV_ISR_FP kdrv_cb;
	BOOL fatboot_isr_proc;		// flag for run fastboot isr process
	BOOL kdrv_isr_proc;			// flag for run kdrv isr process
	BOOL fastboot_last_frame;	// flag for fastboot last frame, next vd int. sts will call kdrv isr
} SIE_BUILTIN_INFO;

/* dtsi parser */
int sie_init_plat_get_nodeoffset(CHAR *node, int *nodeoffset);
void sie_init_plat_get_nodeoffset_list(int nodeoffset, int *nodeoffset_list, int *nodeoffset_list2);
INT32 sie_init_plat_read_dtsi_array(int nodeoffset, CHAR *tag, UINT32 *buf, UINT32 num);

/* alloc & free buffer */
void *kdrv_sie_builtin_plat_malloc(UINT32 size);
void kdrv_sie_builtin_plat_free(void *p_buf);

void *kdrv_sie_builtin_ioremap(ULONG addr, UINT32 size);
SIE_BUILTIN_RESOURCE *sie_builtin_resource_get(UINT32 hdl_idx);

#endif// _SIE_INIT_INT_H
