#include <stdio.h>
#include <string.h>

#include "kwrap/nvt_type.h"
#include "kwrap/cmdsys.h"
#include "kwrap/sxcmd.h"

#include "isp_api_int.h"
#include "isp_dbg.h"
#include "isp_msg.h"

static BOOL isp_sxcmd_dump_info(unsigned char argc, char **argv)
{
	isp_msg_show_info();

	return TRUE;
}

static BOOL isp_sxcmd_dump_ca(unsigned char argc, char **argv)
{
	ISP_ID id;

	if (argc < 1) {
		DBG_DUMP("wrong argument:%d \r\n", argc);
		return FALSE;
	}

	sscanf(argv[0], "%x", &id);

	isp_msg_show_ca(id);

	return TRUE;
}

static BOOL isp_sxcmd_dump_ca_acc_cnt(unsigned char argc, char **argv)
{
	ISP_ID id;

	if (argc < 1) {
		DBG_DUMP("wrong argument:%d \r\n", argc);
		return FALSE;
	}

	sscanf(argv[0], "%x", &id);

	isp_msg_show_ca_acc_cnt(id);

	return TRUE;
}

static BOOL isp_sxcmd_dump_la(unsigned char argc, char **argv)
{
	ISP_ID id;

	if (argc < 1) {
		DBG_DUMP("wrong argument:%d \r\n", argc);
		return FALSE;
	}

	sscanf(argv[0], "%x", &id);

	isp_msg_show_la(id);

	return TRUE;
}

static BOOL isp_sxcmd_dump_va(unsigned char argc, char **argv)
{
	ISP_ID id;

	if (argc < 1) {
		DBG_DUMP("wrong argument:%d \r\n", argc);
		return FALSE;
	}

	sscanf(argv[0], "%x", &id);

	isp_msg_show_va(id);

	return TRUE;
}

static BOOL isp_sxcmd_dump_histo(unsigned char argc, char **argv)
{
	ISP_ID id;

	if (argc < 1) {
		DBG_DUMP("wrong argument:%d \r\n", argc);
		return FALSE;
	}

	sscanf(argv[0], "%x", &id);

	isp_msg_show_histo(id);

	return TRUE;
}

static BOOL isp_sxcmd_set_dbg(unsigned char argc, char **argv)
{
	ISP_ID id;
	UINT32 cmd;

	if (argc < 2) {
		DBG_DUMP("wrong argument:%d \r\n", argc);
		return FALSE;
	}

	sscanf(argv[0], "%x", &id);
	sscanf(argv[1], "%x", (int *)&cmd);

	printf("set isp(%d) dbg level(0x%X) \n", (int)id, (unsigned int)cmd);
	isp_dbg_set_dbg_mode(id, cmd);

	return TRUE;
}

static BOOL isp_sxcmd_set_bypass(unsigned char argc, char **argv)
{
	ISP_ID id;
	UINT32 cmd;

	if (argc < 2) {
		DBG_DUMP("wrong argument:%d \r\n", argc);
		return FALSE;
	}

	sscanf(argv[0], "%x", &id);
	sscanf(argv[1], "%x", (int *)&cmd);

	printf("set isp(%d) bypass eng(0x%X) \n", (int)id, (unsigned int)cmd);
	isp_dbg_set_bypass_eng(id, cmd);

	return TRUE;
}

static BOOL isp_sxcmd_set_sensor_dir(unsigned char argc, char **argv)
{
	ISP_ID id;
	ISP_SENSOR_DIRECTION sensor_dir = {0};

	if (argc < 3) {
		DBG_DUMP("wrong argument:%d \r\n", argc);
		return FALSE;
	}

	sscanf(argv[0], "%x", &id);
	sscanf(argv[1], "%x", &sensor_dir.mirror);
	sscanf(argv[2], "%x", &sensor_dir.flip);

	isp_api_set_direction(id, &sensor_dir);

	return TRUE;
}

static BOOL isp_sxcmd_set_sensor_sleep(unsigned char argc, char **argv)
{
	ISP_ID id;

	if (argc < 1) {
		DBG_DUMP("wrong argument:%d \r\n", argc);
		return FALSE;
	}

	sscanf(argv[0], "%x", &id);

	isp_api_set_sensor_sleep(id);

	return TRUE;
}

static BOOL isp_sxcmd_set_sensor_wakeup(unsigned char argc, char **argv)
{
	ISP_ID id;

	if (argc < 1) {
		DBG_DUMP("wrong argument:%d \r\n", argc);
		return FALSE;
	}

	sscanf(argv[0], "%x", &id);

	isp_api_set_sensor_wakeup(id);

	return TRUE;
}

static SXCMD_BEGIN(isp_cmd_tbl, "isp")
SXCMD_ITEM("info",              isp_sxcmd_dump_info,               "dump isp info")
SXCMD_ITEM("get_ca %d",         isp_sxcmd_dump_ca,                 "dump ca, param1 is isp_id")
SXCMD_ITEM("get_ca_acc_cnt %d", isp_sxcmd_dump_ca_acc_cnt,         "dump ca_acc_cnt, param1 is isp_id")
SXCMD_ITEM("get_la %d",         isp_sxcmd_dump_la,                 "dump la, param1 is isp_id")
SXCMD_ITEM("get_va %d",         isp_sxcmd_dump_va,                 "dump va, param1 is isp_id")
SXCMD_ITEM("get_histo %d",      isp_sxcmd_dump_histo,              "dump histo, param1 is isp_id")
SXCMD_ITEM("set_dbg %d %d",     isp_sxcmd_set_dbg,                 "set isp dbg level, param1 is isp_id(15 means apply to each id), param2 is dbg_lv.")
SXCMD_ITEM("set_bypass %d %d",  isp_sxcmd_set_bypass,              "set isp bypass eng, param1 is isp_id(15 means apply to each id), param2 is bypass_eng.")
SXCMD_ITEM("set_dir %d %d %d",  isp_sxcmd_set_sensor_dir,          "set sensor direction, param1 is isp_id, param2 is mirror, param3 is flip.")
SXCMD_ITEM("set_sleep %d",      isp_sxcmd_set_sensor_sleep,        "set sensor sleep, param1 is isp_id.")
SXCMD_ITEM("set_wakeup %d",     isp_sxcmd_set_sensor_wakeup,       "set sensor wakeup, param1 is isp_id.")
SXCMD_END()

int isp_cmd_showhelp(int (*dump)(const char *fmt, ...))
{
	UINT32 cmd_num = SXCMD_NUM(isp_cmd_tbl);
	UINT32 loop = 1;

	dump("---------------------------------------------------------------------\r\n");
	dump("  %s\n", "isp");
	dump("---------------------------------------------------------------------\r\n");

	for (loop = 1 ; loop <= cmd_num ; loop++) {
		dump("%15s : %s\r\n", isp_cmd_tbl[loop].p_name, isp_cmd_tbl[loop].p_desc);
	}

	dump("---------------------------------------------------------------------\r\n");
	dump(" dbg_lv = \r\n");
	dump(" | 0x%8X = AE STATUS    | 0x%8X = TOTAL GAIN   | 0x%8X = ISP GAIN     | 0x%8X = D GAIN       | \r\n", ISP_DBG_SYNC_AE_STATUS, ISP_DBG_SYNC_TOTAL_GAIN, ISP_DBG_SYNC_ISP_GAIN, ISP_DBG_SYNC_D_GAIN);
	dump(" | 0x%8X = LV           | 0x%8X = CA_LA_ENABLE | 0x%8X = EV_RATIO     | 0x%8X = TM_RATIO     | \r\n", ISP_DBG_SYNC_LV, ISP_DBG_SYNC_CA_LA_ENABLE, ISP_DBG_SYNC_EV_RATIO, ISP_DBG_SYNC_TM_RATIO);
	dump(" | 0x%8X = C GAIN       | 0x%8X = FINAL C GAIN | 0x%8X = CT           | \r\n", ISP_DBG_SYNC_C_GAIN, ISP_DBG_SYNC_FINAL_C_GAIN, ISP_DBG_SYNC_CT);
	dump(" | 0x%8X = CA_TH        | 0x%8X = CA ROI       | 0x%8X = LA ROI       | 0x%8X = VA ROI       | \r\n", ISP_DBG_SYNC_CA_TH, ISP_DBG_SYNC_CA_ROI, ISP_DBG_SYNC_LA_ROI, ISP_DBG_SYNC_VA_ROI);
	dump(" | 0x%8X = SENSOR EXPT  | 0x%8X = SENSOR GAIN  | 0x%8X = SENSOR REG.  | 0x%8X = CAPTURE      | \r\n", ISP_DBG_SENSOR_EXPT, ISP_DBG_SENSOR_GAIN, ISP_DBG_SENSOR_REG, ISP_DBG_SYNC_CAPTURE);
	dump(" | 0x%8X = SIE CB       | 0x%8X = IPP CB       | 0x%8X = ENC CB       | \r\n", ISP_DBG_SIE_CB, ISP_DBG_IPP_CB, ISP_DBG_ENC_CB);
	dump(" | 0x%8X = SIE SET      | 0x%8X = IPP SET      | 0x%8X = ENC SET      | \r\n", ISP_DBG_SIE_SET, ISP_DBG_IPP_SET, ISP_DBG_ENC_SET);
	dump(" | 0x%8X = ERR MSG      | 0x%8X = WRN MSG      | \r\n", ISP_DBG_ERR_MSG, ISP_DBG_WRN_MSG);
	dump("---------------------------------------------------------------------\r\n");
	dump(" bypass_eng = \r\n");
	dump(" | 0x%8X = SIE_ROI      | 0x%8X = SIE_PARAM    | 0x%8X = IFE_PARAM    | 0x%8X = PRE_PARAM    | \r\n", ISP_BYPASS_SIE_ROI, ISP_BYPASS_SIE_PARAM, ISP_BYPASS_IFE_PARAM, ISP_BYPASS_PRE_PARAM);
	dump(" | 0x%8X = IPE_PARAM    | 0x%8X = IME_PARAM    | 0x%8X = ENC_PARAM    | \r\n", ISP_BYPASS_IPE_PARAM, ISP_BYPASS_IME_PARAM, ISP_BYPASS_ENC_PARAM);
	dump(" | 0x%8X = IFE_VIG_CENT | 0x%8X = IFE_VA_WIN_S | 0x%8X = IPE_VA_WIN_S | 0x%8X = IME_VA_WIN_S | \r\n", ISP_BYPASS_IFE_VIG_CENT, ISP_BYPASS_IFE_VA_WIN_SIZE, ISP_BYPASS_IPE_VA_WIN_SIZE, ISP_BYPASS_IME_VA_WIN_SIZE);
	dump(" | 0x%8X = IME_LCA_DBG  | \r\n", ISP_BYPASS_IME_LCA_DBG_X_POS);
	dump(" | 0x%8X = SEN_REG      | 0x%8X = SEN_EXPT     | 0x%8X = SEN_GAIN     | 0x%8X = SEN_DIR      | \r\n", ISP_BYPASS_SEN_REG, ISP_BYPASS_SEN_EXPT, ISP_BYPASS_SEN_GAIN, ISP_BYPASS_SEN_DIR);
	dump(" | 0x%8X = SEN_SLEEP    | 0x%8X = SEN_WAKEUP   | \r\n", ISP_BYPASS_SEN_SLEEP, ISP_BYPASS_SEN_WAKEUP);
	dump("---------------------------------------------------------------------\r\n");
	dump("Example: \r\n");
	dump("isp get_ca 0 \r\n");
	dump("isp get_la 0 \r\n");
	dump("isp get_va 0 \r\n");
	dump("isp set_dbg 15 0x00000000 > /proc/hdal/vendor/isp/cmd \r\n");
	dump("isp set_dbg 15 0x00030000 > /proc/hdal/vendor/isp/cmd \r\n");
	dump("isp set_dbg 15 0x00100000 > /proc/hdal/vendor/isp/cmd \r\n");
	dump("isp set_bypass 15 0x00000000 > /proc/hdal/vendor/isp/cmd \r\n");
	dump("isp set_bypass 15 0x00001F7F > /proc/hdal/vendor/isp/cmd \r\n");
	dump("isp set_dir 0 0 0 > /proc/hdal/vendor/isp/cmd    \r\n");

	return 0;
}

MAINFUNC_ENTRY(isp, argc, argv)
{
	UINT32 cmd_num = SXCMD_NUM(isp_cmd_tbl);
	UINT32 loop;
	int    ret;

	if (argc < 2) {
		return -1;
	}
	if (strncmp(argv[1], "?", 2) == 0) {
		isp_cmd_showhelp(vk_printk);
		return 0;
	}
	for (loop = 1 ; loop <= cmd_num ; loop++) {
		if (strncmp(argv[1], isp_cmd_tbl[loop].p_name, strlen(argv[1])) == 0) {
			ret = isp_cmd_tbl[loop].p_func(argc - 2, &argv[2]);
			return ret;
		}
	}
	if (loop > cmd_num) {
		DBG_ERR("Invalid CMD !!\r\n");
		isp_cmd_showhelp(vk_printk);
		return -1;
	}
	return 0;
}

