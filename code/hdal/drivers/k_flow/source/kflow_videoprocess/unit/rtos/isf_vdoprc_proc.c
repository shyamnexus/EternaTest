#include "../isf_vdoprc_api.h"
#include "../isf_vdoprc_dbg.h"
#include "../isf_vdoprc_int.h"
#include "kwrap/semaphore.h"

//============================================================================
// Function define
//============================================================================
static int isf_vdoprc_seq_printf(const char *fmtstr, ...)
{
	char    buf[512];
	int     len;

	va_list marker;

	va_start(marker, fmtstr);

	len = vsnprintf(buf, sizeof(buf), fmtstr, marker);
	va_end(marker);
	if (len > 0)
		VPRC_DBG_DUMP(buf);
	return 0;
}

//=============================================================================
// proc "info" file operation functions
//=============================================================================
extern void debug_log_cb(UINT32 uid);
extern BOOL _isf_vdoprc_api_info(unsigned char argc, char **pargv)
{
	UINT32 dev;

	if (!g_vdoprc_init[0]) {
		return 0;
	}

	SEM_WAIT(ISF_VDOPRC_PROC_SEM_ID);
	//dump info of all devices
	debug_log_cb(1); //show hdal version
	for(dev = 0; dev < VDOPRC_MAX_NUM; dev++) {
		UINT32 uid = ISF_UNIT_VDOPRC + dev;
		ISF_UNIT *p_unit = isf_unit_ptr(uid);
		//dump bind, state and param settings of 1 device
		debug_log_cb(uid);
		//dump work status of 1 device
		isf_vdoprc_dump_status(isf_vdoprc_seq_printf, p_unit);
	}
	SEM_SIGNAL(ISF_VDOPRC_PROC_SEM_ID);
	return 1;
}


#include "kwrap/cmdsys.h"
#include "kwrap/debug.h"

//=============================================================================
// proc "flow" file operation functions
//=============================================================================

BOOL _isf_vdoprc_api_flow(unsigned char argc, char **pargv)
{
	if (!_isf_vdoprc_is_init()) {
		VPRC_DBG_ERR("[vdoprc] not init\r\n");
		return 1;
	}

	VPRC_DBG_DUMP("[kflow ctl_ipp]\r\n");
	nvt_cmdsys_runcmd("ctl_ipp dump_hdl_all");
	return 1;
}

//=============================================================================
// proc "drv" file operation functions
//=============================================================================

BOOL _isf_vdoprc_api_drv(unsigned char argc, char **pargv)
{
	if (!_isf_vdoprc_is_init()) {
		VPRC_DBG_ERR("[vdoprc] not init\r\n");
		return 1;
	}

	VPRC_DBG_DUMP("[kdrv ife]\r\n");
	nvt_cmdsys_runcmd("kdrv_ife info");
	VPRC_DBG_DUMP("[kdrv ife2]\r\n");
	nvt_cmdsys_runcmd("kdrv_ife2 info");
	VPRC_DBG_DUMP("[kdrv dce]\r\n");
	nvt_cmdsys_runcmd("kdrv_dce info");
	VPRC_DBG_DUMP("[kdrv ipe]\r\n");
	nvt_cmdsys_runcmd("kdrv_ipe info");
	VPRC_DBG_DUMP("[kdrv ime]\r\n");
	nvt_cmdsys_runcmd("kdrv_ime info");
	return 1;
}

//=============================================================================
// proc "reg" file operation functions
//=============================================================================
BOOL _isf_vdoprc_api_reg(unsigned char argc, char **pargv)
{
#if defined(_BSP_NA51000_)
	VPRC_DBG_DUMP("[RHE]\r\n");
	debug_dumpmem(0xf0ce0000, 0x900);
	VPRC_DBG_DUMP("[IFE]\r\n");
	debug_dumpmem(0xf0c70000, 0x200);
	VPRC_DBG_DUMP("[IFE2]\r\n");
	debug_dumpmem(0xf0d00000, 0x100);
	VPRC_DBG_DUMP("[DCE]\r\n");
	debug_dumpmem(0xf0c20000, 0x300);
	VPRC_DBG_DUMP("[IPE]\r\n");
	debug_dumpmem(0xf0c30000, 0x800);
	VPRC_DBG_DUMP("[IME]\r\n");
	debug_dumpmem(0xf0c40000, 0x1000);
#endif
	VPRC_DBG_DUMP("[IFE]\r\n");
	debug_dumpmem(0xf0c70000, 0x800);
	VPRC_DBG_DUMP("[IFE2]\r\n");
	debug_dumpmem(0xf0d00000, 0x100);
	VPRC_DBG_DUMP("[DCE]\r\n");
	debug_dumpmem(0xf0c20000, 0x700);
	VPRC_DBG_DUMP("[IPE]\r\n");
	debug_dumpmem(0xf0c30000, 0x900);
	VPRC_DBG_DUMP("[IME]\r\n");
	debug_dumpmem(0xf0c40000, 0xb00);
	return 1;
}


//=============================================================================
// proc "rate" file operation functions
//=============================================================================
BOOL _isf_vdoprc_api_rate2(unsigned char argc, char **pargv)
{

	VPRC_DBG_DUMP("[kflow ctl_ipp]\r\n");
	nvt_cmdsys_runcmd("ctl_ipp dump_t");
	return 1;
}


