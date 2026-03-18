#include <stdio.h>
#include <string.h>

#include "kwrap/nvt_type.h"
#include "kwrap/cmdsys.h"
#include "kwrap/sxcmd.h"

#include "isp_dev.h"

#include "iq_alg.h"
#include "iqt_api.h"
#include "iq_dbg.h"
#include "iq_main.h"

static BOOL iq_sxcmd_dump_info(unsigned char argc, char **argv)
{
	iq_msg_show_info();

	return 0;
}

static BOOL iq_sxcmd_get_buffer_size(unsigned char argc, char **argv)
{
	iq_msg_show_buffer_size();

	return 0;
}

static BOOL iq_sxcmd_get_param(unsigned char argc, char **argv)
{
	IQ_ID id;
	UINT32 param_id;

	if (argc < 2) {
		DBG_DUMP("wrong argument:%d", argc);
		DBG_DUMP("please set (id, param_id)\r\n");
		return -1;
	}

	sscanf(argv[0], "%x", &id);
	sscanf(argv[1], "%d", (int *)&param_id);

	if (!iq_flow_get_id_valid(id) || (id >= IQ_ID_MAX_NUM)) {
		DBG_DUMP("invalid id (%d) \r\n", id);
		return 0;
	}

	iq_msg_show_param(id, param_id);

	return 0;
}

static BOOL iq_sxcmd_get_ui_param(unsigned char argc, char **argv)
{
	IQ_ID id;

	if (argc < 1) {
		DBG_DUMP("wrong argument:%d", argc);
		return -1;
	}

	sscanf(argv[0], "%x", &id);

	if (!iq_flow_get_id_valid(id) || (id >= IQ_ID_MAX_NUM)) {
		DBG_DUMP("invalid id (%d) \r\n", id);
		return 0;
	}

	iq_msg_show_ui(id);

	return 0;
}

static BOOL iq_sxcmd_get_cfg_data(unsigned char argc, char **argv)
{
	IQ_ID id;

	if (argc < 1) {
		DBG_DUMP("wrong argument:%d", argc);
		return -1;
	}

	sscanf(argv[0], "%x", &id);

	if (!iq_flow_get_id_valid(id) || (id >= IQ_ID_MAX_NUM)) {
		DBG_DUMP("invalid id (%d) \r\n", id);
		return 0;
	}

	iq_msg_show_cfg_path();

	return 0;
}

static BOOL iq_sxcmd_get_low_power_info(unsigned char argc, char **argv)
{
	iq_msg_show_low_power_info();

	return 0;
}

static BOOL iq_sxcmd_set_dbg(unsigned char argc, char **argv)
{
	IQ_ID id;
	UINT32 dbg_lv;

	if (argc < 2) {
		DBG_DUMP("wrong argument:%d \r\n", argc);
		DBG_DUMP("please set (id, dbg_lv) \r\n");
		return -1;
	}

	sscanf(argv[0], "%x", &id);
	sscanf(argv[1], "%x", (int *)&dbg_lv);

	DBG_DUMP("set iq(%d) dbg level(0x%x) \r\n", id, dbg_lv);
	iq_dbg_set_dbg_mode(id, dbg_lv);

	return 0;
}

static BOOL iq_sxcmd_set_ui_param(unsigned char argc, char **argv)
{
	IQ_ID id;
	IQ_UI_ITEM ui_type;
	UINT32 ui_value;

	if (argc < 3) {
		DBG_DUMP("wrong argument : %d \r\n", argc);
		DBG_DUMP("please set (id, ui_type, ui_value) \r\n");
		return -1;
	}
	sscanf(argv[0], "%x", &id);
	sscanf(argv[1], "%x", &ui_type);
	sscanf(argv[2], "%d", (int *)&ui_value);

	DBG_DUMP("iq_sxcmd_set_ui_param \r\n");
	DBG_DUMP("id : %d \r\n", id);
	DBG_DUMP("set ui type : %d, value : %d \r\n", ui_type, ui_value);
	iq_ui_set_info(id, ui_type, ui_value);

	return 0;
}

#if 0
static BOOL iq_sxcmd_reload_dtsi(unsigned char argc, char **argv)
{
	IQT_DTSI_INFO dtsi_info = {0};

	if (argc < 2) {
		DBG_DUMP("wrong argument:%d \r\n", argc);
		DBG_DUMP("please set (id, dtsi_path) \r\n");
		return -1;
	}

	sscanf(argv[0], "%x", &dtsi_info.id);
	strncpy(dtsi_info.node_path, argv[1], IQ_DTSI_NAME_LENGTH-1);
	dtsi_info.node_path[IQ_DTSI_NAME_LENGTH-1] = '\0';
	strncpy(dtsi_info.file_path, "null", IQ_DTSI_NAME_LENGTH);
	dtsi_info.file_path[IQ_DTSI_NAME_LENGTH-1] = '\0';

	DBG_DUMP("set iq(%d) dtsi path (%s) \r\n", dtsi_info.id, dtsi_info.node_path);
	iqt_api_set_cmd(IQT_ITEM_RLD_DTSI, (UINT32)&dtsi_info);

	return 0;
}
#endif

static BOOL iq_sxcmd_reload_cfg(unsigned char argc, char **argv)
{
	IQT_CFG_INFO cfg_info = {0};

	if (argc < 2) {
		DBG_DUMP("wrong argument:%d \r\n", argc);
		DBG_DUMP("please set (id, cfg_path) \r\n");
		return -1;
	}

	sscanf(argv[0], "%x", &cfg_info.id);
	strncpy(cfg_info.path, argv[1], IQ_CFG_NAME_LENGTH-1);
	cfg_info.path[IQ_CFG_NAME_LENGTH-1] = '\0';

	DBG_DUMP("set iq(%d) cfg path (%s) \r\n", cfg_info.id, cfg_info.path);
	iqt_api_set_cmd(IQT_ITEM_RLD_CONFIG, (ULONG)&cfg_info);

	return 0;
}

static SXCMD_BEGIN(iq_cmd_tbl, "iq")
SXCMD_ITEM("info",              iq_sxcmd_dump_info,               "dump iq info")

SXCMD_ITEM("get_buffer_size",   iq_sxcmd_get_buffer_size,         "get iq buffer size")
SXCMD_ITEM("get_param",         iq_sxcmd_get_param,               "get iq param, param1 is iq_id(0~2), param2 is param_id(0~15)")
SXCMD_ITEM("get_ui_param",      iq_sxcmd_get_ui_param,            "get iq ui parameter, param1 is iq_id(0~2)")
SXCMD_ITEM("get_cfg_data",      iq_sxcmd_get_cfg_data,            "get iq cfg file name, param1 is iq_id(0~2)")
SXCMD_ITEM("low_power_info",    iq_sxcmd_get_low_power_info,      "get iq low power info, NT98539A only.")

SXCMD_ITEM("set_dbg",           iq_sxcmd_set_dbg,                 "set iq dbg level, param1 is iq_id(0~2), param2 is dbg_lv")
SXCMD_ITEM("set_ui_param",      iq_sxcmd_set_ui_param,            "set iq ui parameter, param1 is iq_id(0~2), param2 is ui_type(0~9), param3 is ui_value")
#if 0
SXCMD_ITEM("reload_dtsi",       iq_sxcmd_reload_dtsi,             "reload dtsi file, param1 is iq_id(0~2), param2 is is node path")
#endif
SXCMD_ITEM("reload_cfg",        iq_sxcmd_reload_cfg,              "reload cfg file, param1 is iq_id(0~2), param2 is is file path")
SXCMD_END()

int iq_cmd_showhelp(int (*dump)(const char *fmt, ...))
{
	UINT32 cmd_num = SXCMD_NUM(iq_cmd_tbl);
	UINT32 loop;

	DBG_DUMP("1. 'iq info' will show all the iq info \r\n");
	DBG_DUMP("2. 'iq xxx param1 param2 ...' can input command for some debug purpose \r\n");
	DBG_DUMP("The currently support input command are below: \r\n");

	DBG_DUMP("--------------------------------------------------------------------- \r\n");
	DBG_DUMP("  %s\n", "iq");
	DBG_DUMP("--------------------------------------------------------------------- \r\n");

	for (loop = 1 ; loop <= cmd_num ; loop++) {
		DBG_DUMP("%15s : %s \r\n", iq_cmd_tbl[loop].p_name, iq_cmd_tbl[loop].p_desc);
	}

	DBG_DUMP("--------------------------------------------------------------------- \r\n");
	DBG_DUMP(" func_en = \r\n");
	DBG_DUMP("   | 0x%8x = WDR    | 0x%8x = SHDR   | 0x%8x = DEFOG  | \r\n", ISP_FUNC_EN_WDR, ISP_FUNC_EN_SHDR, ISP_FUNC_EN_DEFOG);
	DBG_DUMP(" dbg_lv = \r\n");
	DBG_DUMP("   | 0x%8x = WRN    | 0x%8x = SYNC   | 0x%8x = CFG    | 0x%8x = DTS    | \r\n", IQ_DBG_WRN_MSG, IQ_DBG_SYNC, IQ_DBG_CFG, IQ_DBG_DTS);
	DBG_DUMP("   | 0x%8x = VIG    | 0x%8x = GAMMA  | 0x%8x = CCM    | 0x%8x = TABLE  | \r\n", IQ_DBG_VIG, IQ_DBG_GAMMA, IQ_DBG_CCM, IQ_DBG_TABLE);
	DBG_DUMP("   | 0x%8x = SIE    | 0x%8x = IFE    | 0x%8x = PRE    | 0x%8x = IPE    | \r\n", IQ_DBG_P_SIE, IQ_DBG_P_IFE, IQ_DBG_P_PRE, IQ_DBG_P_IPE);
	DBG_DUMP("   | 0x%8x = IME    | 0x%8x = ENC    |                                   \r\n", IQ_DBG_P_IME, IQ_DBG_P_ENC);
	DBG_DUMP("   | 0x%8x = WDR    | 0x%8x = DEFOG  | 0x%8x = SHDR   | 0x%8x = RGBIR  | \r\n", IQ_DBG_A_WDR, IQ_DBG_A_DEFOG, IQ_DBG_A_SHDR, IQ_DBG_A_RGBIR);
	DBG_DUMP("   | 0x%8x = ISO    | 0x%8x = EDGE   | \r\n", IQ_DBG_O_ISO, IQ_DBG_O_EDGE);
	DBG_DUMP("   | 0x%8x = CAP    | \r\n", IQ_DBG_CAPTURE);
	DBG_DUMP("   | 0x%8x = PERFOR | \r\n", IQ_DBG_PERFORMANCE);
	DBG_DUMP("--------------------------------------------------------------------- \r\n");
	DBG_DUMP(" ui_type = \r\n");
	DBG_DUMP("   | 0x%8x = NR     | 0x%8x = SHARP  | \r\n", IQ_UI_ITEM_NR_LV, IQ_UI_ITEM_SHARPNESS_LV);
	DBG_DUMP("   | 0x%8x = SAT    | 0x%8x = CONT   | 0x%8x = BRIGHT | \r\n", IQ_UI_ITEM_SATURATION_LV, IQ_UI_ITEM_CONTRAST_LV, IQ_UI_ITEM_BRIGHTNESS_LV);
	DBG_DUMP("   | 0x%8x = NIGHT  | 0x%8x = YCC    | 0x%8x = OPERA  | \r\n", IQ_UI_ITEM_NIGHT_MODE, IQ_UI_ITEM_YCC_FORMAT, IQ_UI_ITEM_OPERATION);
	DBG_DUMP("   | 0x%8x = EFFECT | 0x%8x = CCID   | 0x%8x = HUE    | \r\n", IQ_UI_ITEM_IMAGEEFFECT, IQ_UI_ITEM_CCID, IQ_UI_ITEM_HUE_SHIFT);
	DBG_DUMP("--------------------------------------------------------------------- \r\n");
	DBG_DUMP("Ex1: 'iq get_param 0 0' \r\n");
	DBG_DUMP("Ex2: 'iq get_ui_param 0' \r\n");
	DBG_DUMP("Ex3: 'iq set_dbg 0 0x00100' \r\n");
	DBG_DUMP("Ex4: 'iq reload_cfg 0 A:\\isp_imx290_0_uvc.cfg' \r\n");
	return 0;
}

MAINFUNC_ENTRY(iq, argc, argv)
{
	UINT32 cmd_num = SXCMD_NUM(iq_cmd_tbl);
	UINT32 loop;
	int    ret;

	if (argc < 2) {
		return -1;
	}
	if (strncmp(argv[1], "?", 2) == 0) {
		iq_cmd_showhelp(vk_printk);
		return 0;
	}
	for (loop = 1 ; loop <= cmd_num ; loop++) {
		if (strncmp(argv[1], iq_cmd_tbl[loop].p_name, strlen(argv[1])) == 0) {
			ret = iq_cmd_tbl[loop].p_func(argc - 2, &argv[2]);
			return ret;
		}
	}
	if (loop > cmd_num) {
		DBG_ERR("Invalid CMD !! \r\n");
		iq_cmd_showhelp(vk_printk);
		return -1;
	}
	return 0;
}

