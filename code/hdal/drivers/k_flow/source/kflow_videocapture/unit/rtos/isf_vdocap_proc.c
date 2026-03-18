#include "../isf_vdocap_api.h"
//#define __DBGLVL__          2 // 0=FATAL, 1=ERR, 2=WRN, 3=UNIT, 4=FUNC, 5=IND, 6=MSG, 7=VALUE, 8=USER
#include "../isf_vdocap_dbg.h"
#include "../isf_vdocap_int.h"
#include "kwrap/semaphore.h"
#include "comm/compiler.h"
#include "comm/libfdt.h"
#include <plat/rtosfdt.h>

//#ifdef DEBUG
unsigned int isf_vdocap_debug_level = THIS_DBGLVL;
module_param_named(isf_vdocap_debug_level, isf_vdocap_debug_level, int, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(isf_vdocap_debug_level, "vdocap debug level");
//#endif
//============================================================================
// Function define
//============================================================================
static int isf_vdocap_seq_printf(const char *fmtstr, ...)
{
	char    buf[512];
	int     len;

	va_list marker;

	va_start(marker, fmtstr);

	len = vsnprintf(buf, sizeof(buf), fmtstr, marker);
	va_end(marker);
	if (len > 0)
		DBG_DUMP(buf);
	return 0;
}

//=============================================================================
// proc "info" file operation functions
//=============================================================================
extern void debug_log_cb(UINT32 uid);
BOOL _isf_vdocap_api_info(unsigned char argc, char **pargv)
{
	UINT32 dev, vdocap_max_num;

	if (!g_vdocap_init[0]) {
		return 0;
	}
	if( nvt_get_chip_id() != CHIP_NA51055) {
		vdocap_max_num = VDOCAP_MAX_NUM;
	} else {
		vdocap_max_num = 3;
	}

	SEM_WAIT(ISF_VDOCAP_PROC_SEM_ID);
	//dump info of all devices
	debug_log_cb(1); //show hdal version
	for(dev = 0; dev < vdocap_max_num; dev++) {
		UINT32 uid = ISF_UNIT_VDOCAP + dev;
		ISF_UNIT *p_unit = isf_unit_ptr(uid);
		//dump bind, state and param settings of 1 device
		debug_log_cb(uid);
		//dump work status of 1 device
		isf_vdocap_dump_status(isf_vdocap_seq_printf, p_unit);
	}
	SEM_SIGNAL(ISF_VDOCAP_PROC_SEM_ID);
	return 1;
}

BOOL _isf_vdocap_api_drv(unsigned char argc, char **pargv)
{
#if defined(__LINUX)
	_vdocap_print_ctl_sie_info((VDOCAP_CONTEXT *)isf_vdocap0.refdata);
	_vdocap_print_ctl_sie_info((VDOCAP_CONTEXT *)isf_vdocap1.refdata);
	_vdocap_print_ctl_sie_info((VDOCAP_CONTEXT *)isf_vdocap2.refdata);
	_vdocap_print_ctl_sie_info((VDOCAP_CONTEXT *)isf_vdocap3.refdata);
	_vdocap_print_ctl_sie_info((VDOCAP_CONTEXT *)isf_vdocap4.refdata);
#if defined(_BSP_NA51000_)
	_vdocap_print_ctl_sie_info((VDOCAP_CONTEXT *)isf_vdocap5.refdata);
	_vdocap_print_ctl_sie_info((VDOCAP_CONTEXT *)isf_vdocap6.refdata);
	_vdocap_print_ctl_sie_info((VDOCAP_CONTEXT *)isf_vdocap7.refdata);
#endif
	_vdocap_print_ctl_sen_info((VDOCAP_CONTEXT *)isf_vdocap0.refdata);
	_vdocap_print_ctl_sen_info((VDOCAP_CONTEXT *)isf_vdocap1.refdata);
	_vdocap_print_ctl_sen_info((VDOCAP_CONTEXT *)isf_vdocap2.refdata);
	_vdocap_print_ctl_sen_info((VDOCAP_CONTEXT *)isf_vdocap3.refdata);
	_vdocap_print_ctl_sen_info((VDOCAP_CONTEXT *)isf_vdocap4.refdata);
#if defined(_BSP_NA51000_)
	_vdocap_print_ctl_sen_info((VDOCAP_CONTEXT *)isf_vdocap5.refdata);
	_vdocap_print_ctl_sen_info((VDOCAP_CONTEXT *)isf_vdocap6.refdata);
	_vdocap_print_ctl_sen_info((VDOCAP_CONTEXT *)isf_vdocap7.refdata);
#endif

#endif
	return 1;
}

BOOL _isf_vdocap_api_reg(unsigned char argc, char **pargv)
{
	VDOCAP_CONTEXT *p_ctx;

	p_ctx = (VDOCAP_CONTEXT *)isf_vdocap0.refdata;
	if (p_ctx && p_ctx->sie_hdl) {
		DBG_DUMP("[SIE1]\r\n");
		debug_dumpmem(0xF0C00000, 0x8B4);
	}
	p_ctx = (VDOCAP_CONTEXT *)isf_vdocap1.refdata;
	if (p_ctx && p_ctx->sie_hdl) {
		DBG_DUMP("[SIE2]\r\n");
		debug_dumpmem(0xF0D20000, 0x8B4);
	}
	p_ctx = (VDOCAP_CONTEXT *)isf_vdocap2.refdata;
	if (p_ctx && p_ctx->sie_hdl) {
		DBG_DUMP("[SIE3]\r\n");
		debug_dumpmem(0xF0D30000, 0x8B4);
	}
	if( nvt_get_chip_id() != CHIP_NA51055)
	{
		p_ctx = (VDOCAP_CONTEXT *)isf_vdocap3.refdata;
		if (p_ctx && p_ctx->sie_hdl) {
			DBG_DUMP("[SIE4]\r\n");
			debug_dumpmem(0xF0D40000, 0x8B4);
		}
		p_ctx = (VDOCAP_CONTEXT *)isf_vdocap4.refdata;
		if (p_ctx && p_ctx->sie_hdl) {
			DBG_DUMP("[SIE5]\r\n");
			debug_dumpmem(0xF0D50000, 0x8B4);
		}
	}
#if defined(_BSP_NA51000_)
	p_ctx = (VDOCAP_CONTEXT *)isf_vdocap5.refdata;
	if (p_ctx && p_ctx->sie_hdl) {
		DBG_DUMP("[SIE6]\r\n");
		debug_dumpmem(0xF0D60000, 0x8B4);
	}
	p_ctx = (VDOCAP_CONTEXT *)isf_vdocap6.refdata;
	if (p_ctx && p_ctx->sie_hdl) {
		DBG_DUMP("[SIE7]\r\n");
		debug_dumpmem(0xF0D70000, 0x8B4);
	}
	p_ctx = (VDOCAP_CONTEXT *)isf_vdocap7.refdata;
	if (p_ctx && p_ctx->sie_hdl) {
		DBG_DUMP("[SIE8]\r\n");
		debug_dumpmem(0xF0D80000, 0x8B4);
	}
#endif
	return 1;
}
BOOL _vdocap_parse_pinmux(CHAR *name, CTL_SEN_PINMUX *pinmux, INT32 max_num)
{
	unsigned int pinctrl[MAX_PINCTRL_ITEM] = {0};
	unsigned char *fdt_addr = (unsigned char *)fdt_get_base();
	int nodeoffset, len;
	INT32 i, cnt;
	const void *nodep;
	unsigned int *p_data;
	char node_name[64];

	snprintf(node_name, sizeof(node_name), "/top/%s", name);
	nodeoffset = fdt_path_offset((const void*)fdt_addr, node_name);
	if (nodeoffset >= 0) {
		nodep = fdt_getprop(fdt_addr, nodeoffset, "pinctrl", &len);
		if (len == 0 || nodep == NULL) {
		    DBG_ERR("Failed to read pinctrl\r\n");
		    return FALSE;
		}
		cnt = len / sizeof(unsigned int);
		if (cnt > MAX_PINCTRL_ITEM) {
			cnt = MAX_PINCTRL_ITEM;
			DBG_WRN("Pinmux only support max %d item\r\n", cnt/2);
		}
		p_data = (unsigned int *)nodep;
		for (i = 0; i < cnt; i++) {
		    pinctrl[i] = be32_to_cpu(p_data[i]);
		    //DBG_DUMP("pinctrl[%d] = 0x%X\r\n", i, pinctrl[i]);

		}
		for (i = 0; i < cnt/2; i++) {
			if (i >= max_num) {
				break;
			}
		    pinmux[i].func = pinctrl[i*2];
		    pinmux[i].cfg = pinctrl[i*2+1];
		    if (pinmux[i].func == PIN_FUNC_SENSORMISC) {
				// pinmux[i].cfg_mclk = pinmux[i].cfg & PIN_FUNC_MCLK_MASK;
		    }
		    if (i < (cnt/2 -1)) {
			    pinmux[i].pnext = &pinmux[i+1];
			}
		}
	} else {
		DBG_ERR("Failed to find %s in dtsi\r\n", name);
		return FALSE;
	}
	return TRUE;
}
