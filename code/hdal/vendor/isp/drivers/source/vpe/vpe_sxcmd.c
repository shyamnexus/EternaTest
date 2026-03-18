#include <stdio.h>
#include <string.h>

#include "kwrap/nvt_type.h"
#include "kwrap/cmdsys.h"
#include "kwrap/sxcmd.h"

#include "vpe_alg_int.h"
#include "vpe_msg.h"
#include "vpe_dbg.h"
#include "vpe_version.h"

static VPE_ID vpe_proc_id;
static UINT32 vpe_proc_idx;

static BOOL vpe_sxcmd_dump_info(unsigned char argc, char **argv)
{
	vpe_msg_show_info();

	return 0;
}

static BOOL vpe_sxcmd_set_dbg(unsigned char argc, char **argv)
{
	VPE_ID id;
	UINT32 dbg_lv;

	if (argc < 2) {
		DBG_DUMP("wrong argument:%d \r\n", argc);
		return -1;
	}

	sscanf(argv[0], "%x", &id);
	sscanf(argv[1], "%x", (int *)&dbg_lv);

	DBG_DUMP("set vpe(%d) dbg level(0x%x) \r\n", id, dbg_lv);
	vpe_dbg_set_dbg_mode(id, dbg_lv);

	return 0;
}

static BOOL vpe_sxcmd_get_dce_ctrl(unsigned char argc, char **argv)
{
	if (argc < 1) {
		DBG_DUMP("wrong argument:%d \r\n", argc);
		return -1;
	}

	sscanf(argv[0], "%x", &vpe_proc_id);
	vpe_proc_idx = (vpe_proc_id >> 8) & 0xFF;
	vpe_proc_id = vpe_proc_id & 0xFF;

	vpe_msg_show_dce_ctrl(vpe_proc_id, vpe_proc_idx);

	return 0;
}

static BOOL vpe_sxcmd_get_sharpen(unsigned char argc, char **argv)
{
	if (argc < 1) {
		DBG_DUMP("wrong argument:%d \r\n", argc);
		return -1;
	}

	sscanf(argv[0], "%x", &vpe_proc_id);
	vpe_proc_idx = (vpe_proc_id >> 8) & 0xFF;
	vpe_proc_id = vpe_proc_id & 0xFF;

	vpe_msg_show_sharpen(vpe_proc_id, vpe_proc_idx);

	return 0;
}

static BOOL vpe_sxcmd_get_2dlut(unsigned char argc, char **argv)
{
	if (argc < 1) {
		DBG_DUMP("wrong argument:%d \r\n", argc);
		return -1;
	}

	sscanf(argv[0], "%x", &vpe_proc_id);
	vpe_proc_idx = (vpe_proc_id >> 8) & 0xFF;
	vpe_proc_id = vpe_proc_id & 0xFF;

	vpe_msg_show_2dlut(vpe_proc_id, vpe_proc_idx);

	return 0;
}

static BOOL vpe_sxcmd_get_2dlut_expand(unsigned char argc, char **argv)
{
	if (argc < 1) {
		DBG_DUMP("wrong argument:%d \r\n", argc);
		return -1;
	}

	sscanf(argv[0], "%x", &vpe_proc_id);
	vpe_proc_idx = (vpe_proc_id >> 8) & 0xFF;
	vpe_proc_id = vpe_proc_id & 0xFF;

	vpe_msg_show_2dlut_expand(vpe_proc_id, vpe_proc_idx);

	return 0;
}

static BOOL vpe_sxcmd_get_drt(unsigned char argc, char **argv)
{
	if (argc < 1) {
		DBG_DUMP("wrong argument:%d \r\n", argc);
		return -1;
	}

	sscanf(argv[0], "%x", &vpe_proc_id);
	vpe_proc_idx = (vpe_proc_id >> 8) & 0xFF;
	vpe_proc_id = vpe_proc_id & 0xFF;

	vpe_msg_show_drt(vpe_proc_id, vpe_proc_idx);

	return 0;
}

static BOOL vpe_sxcmd_get_dctg(unsigned char argc, char **argv)
{
	if (argc < 1) {
		DBG_DUMP("wrong argument:%d \r\n", argc);
		return -1;
	}

	sscanf(argv[0], "%x", &vpe_proc_id);
	vpe_proc_idx = (vpe_proc_id >> 8) & 0xFF;
	vpe_proc_id = vpe_proc_id & 0xFF;

	vpe_msg_show_dctg(vpe_proc_id, vpe_proc_idx);

	return 0;
}

static BOOL vpe_sxcmd_get_flip_rot(unsigned char argc, char **argv)
{
	if (argc < 1) {
		DBG_DUMP("wrong argument:%d \r\n", argc);
		return -1;
	}

	sscanf(argv[0], "%x", &vpe_proc_id);
	vpe_proc_idx = (vpe_proc_id >> 8) & 0xFF;
	vpe_proc_id = vpe_proc_id & 0xFF;

	vpe_msg_show_flip_rot(vpe_proc_id, vpe_proc_idx);

	return 0;
}

static SXCMD_BEGIN(vpe_cmd_tbl, "vpe")
SXCMD_ITEM("info",               vpe_sxcmd_dump_info,          "dump vpe info")
SXCMD_ITEM("set_dbg",            vpe_sxcmd_set_dbg,            "set vpe dbg level, param1 is vpe_id(0~15), param2 is dbg_lv")
SXCMD_ITEM("get_dce_ctl",        vpe_sxcmd_get_dce_ctrl,       "get vpe dce ctl param, param1 is vpe_id(0~15)")
SXCMD_ITEM("get_sharpen",        vpe_sxcmd_get_sharpen,        "get vpe sharpen param, param1 is vpe_id(0~15)")
SXCMD_ITEM("get_2dlut",          vpe_sxcmd_get_2dlut,          "get vpe 2dlut param, param1 is vpe_id(0~15)")
SXCMD_ITEM("get_2dlut_expand",   vpe_sxcmd_get_2dlut_expand,   "get vpe 2dlut_expand param, param1 is vpe_id(0~15)")
SXCMD_ITEM("get_drt",            vpe_sxcmd_get_drt,            "get vpe drt param, param1 is vpe_id(0~15)")
SXCMD_ITEM("get_dctg",           vpe_sxcmd_get_dctg,           "get vpe dctg param, param1 is vpe_id(0~15)")
SXCMD_ITEM("get_flip_rot",       vpe_sxcmd_get_flip_rot,       "get vpe flip_rot param, param1 is vpe_id(0~15)")
SXCMD_END()

int vpe_cmd_showhelp(int (*dump)(const char *fmt, ...))
{
	UINT32 cmd_num = SXCMD_NUM(vpe_cmd_tbl);
	UINT32 loop = 1;

	dump("---------------------------------------------------------------------\r\n");
	dump("  %s\n", "vpe");
	dump("---------------------------------------------------------------------\r\n");

	for (loop = 1 ; loop <= cmd_num ; loop++) {
		dump("%15s : %s\r\n", vpe_cmd_tbl[loop].p_name, vpe_cmd_tbl[loop].p_desc);
	}

	dump("\r\n");
	dump("-- Example of get/set parameter ------------------------------------------------\r\n");
	dump("vpe get_dce_ctl 0 \r\n");
	dump("vpe get_sharpen 0 \r\n");
	dump("vpe get_2dlut 0 \r\n");
	dump("vpe get_2dlut_expand 0 \r\n");
	dump("vpe get_drt 0 \r\n");
	dump("vpe get_dctg 0 \r\n");
	dump("vpe get_flip_rot 0 \r\n");
	dump("\r\n");
	dump("--Example of enable vpe cb message ---------------------------------------------\r\n");
	dump("vpe set_dbg 16 0x1 \r\n");
	dump("\r\n");
	dump("-- Example of enable vpe update message ----------------------------------------\r\n");
	dump("vpe set_dbg 16 0x4 \r\n");
	dump("\r\n");
	dump("-- Example of enable warning message -------------------------------------------\r\n");
	dump("vpe set_dbg 16 0x20000000 \r\n");
	dump("\r\n");
	dump("-- Example of disable message --------------------------------------------------\r\n");
	dump("vpe set_dbg 16 0x0 \r\n");
	dump("\r\n");
	dump("-- Example of enable ioc bypass ------------------------------------------------\r\n");
	dump("vpe set_ioc 0x1 \r\n");
	dump("\r\n");
	dump("-- Example of enable ioc print -------------------------------------------------\r\n");
	dump("vpe set_ioc 0x10000 \r\n");
	dump("\r\n");
	dump("-- Example of disable ioc control ----------------------------------------------\r\n");
	dump("vpe set_ioc 0x0 \r\n");

	return 0;
}

MAINFUNC_ENTRY(vpe, argc, argv)
{
	UINT32 cmd_num = SXCMD_NUM(vpe_cmd_tbl);
	UINT32 loop;
	int    ret;

	if (argc < 2) {
		return -1;
	}
	if (strncmp(argv[1], "?", 2) == 0) {
		vpe_cmd_showhelp(vk_printk);
		return 0;
	}
	for (loop = 1 ; loop <= cmd_num ; loop++) {
		if (strncmp(argv[1], vpe_cmd_tbl[loop].p_name, strlen(argv[1])) == 0) {
			ret = vpe_cmd_tbl[loop].p_func(argc - 2, &argv[2]);
			return ret;
		}
	}
	if (loop > cmd_num) {
		DBG_ERR("Invalid CMD !! \r\n");
		vpe_cmd_showhelp(vk_printk);
		return -1;
	}
	return 0;
}

