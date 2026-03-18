#include <linux/seq_file.h>

#include "isp_api.h"
#include "isp_dev.h"
#include "isp_uti.h"

#include "iq_alg.h"
#include "iqt_api.h"
#include "iq_dbg.h"
#include "iq_main.h"
#include "iq_proc.h"

//=============================================================================
// avoid GPL api
//=============================================================================
#define IQ_PROC_DUMP_TOTAL_SIZE 8000
#define MUTEX_ENABLE DISABLE

//=============================================================================
// global
//=============================================================================
#if MUTEX_ENABLE
static struct semaphore mutex;
#endif
static struct proc_dir_entry *proc_root;
static struct proc_dir_entry *proc_info;
static struct proc_dir_entry *proc_command;
static struct proc_dir_entry *proc_dbglevel;
static struct proc_dir_entry *proc_help;

static IQ_PROC_MSG_BUF iq_proc_cmd_msg;
static IQ_PROC_R_ITEM iq_proc_r_item = IQ_PROC_R_ITEM_NONE;
static IQ_ID iq_proc_id;
static UINT32 iq_proc_iso;

//=============================================================================
// routines
//=============================================================================
static inline ISP_MODULE *iq_proc_get_mudule_from_file(struct file *file)
{
	return (ISP_MODULE *)((struct seq_file *)file->private_data)->private;
}

//=============================================================================
// proc "info" file operation functions
//=============================================================================
static INT32 iq_proc_info_show(struct seq_file *sfile, void *v)
{
	iq_msg_show_info(sfile);

	return 0;
}

static INT32 iq_proc_info_open(struct inode *inode, struct file *file)
{
	return single_open(file, iq_proc_info_show, PDE_DATA(inode));
}

static const struct proc_ops iq_proc_info_ops = {
	.proc_open    = iq_proc_info_open,
	.proc_read    = seq_read,
	.proc_lseek   = seq_lseek,
	.proc_release = single_release,
};

//=============================================================================
// proc "cmd" file operation functions
//=============================================================================
static inline INT32 iq_proc_msg_buf_alloc(void)
{
	iq_proc_cmd_msg.buf = isp_uti_vmem_alloc(PROC_MSG_BUFSIZE);
	if (iq_proc_cmd_msg.buf == NULL) {
		DBG_ERR("fail to allocate IQ message buffer!! \r\n");
		return -E_SYS;
	}

	iq_proc_cmd_msg.size = PROC_MSG_BUFSIZE;
	iq_proc_cmd_msg.count = 0;
	return 0;
}

static inline void iq_proc_msg_buf_free(void)
{
	if (iq_proc_cmd_msg.buf) {
		isp_uti_vmem_free(iq_proc_cmd_msg.buf);
		iq_proc_cmd_msg.buf = NULL;
	}
}

static inline void iq_proc_msg_buf_clean(void)
{
	iq_proc_cmd_msg.buf[0] = '\0';
	iq_proc_cmd_msg.count = 0;
}

static INT32 iq_proc_cmd_printf(const s8 *f, ...)
{
	INT32 len;
	va_list args;

	if (iq_proc_cmd_msg.count < iq_proc_cmd_msg.size) {
		va_start(args, f);
		len = vsnprintf(iq_proc_cmd_msg.buf + iq_proc_cmd_msg.count, iq_proc_cmd_msg.size - iq_proc_cmd_msg.count, f, args);
		va_end(args);

		if ((iq_proc_cmd_msg.count + len) < iq_proc_cmd_msg.size) {
			iq_proc_cmd_msg.count += len;
			return 0;
		}
	}

	iq_proc_cmd_msg.count = iq_proc_cmd_msg.size;
	return -1;
}

static INT32 iq_proc_cmd_get_buffer_size(ISP_MODULE *pdrv, INT32 argc, CHAR **argv)
{
	iq_proc_r_item = IQ_PROC_R_ITEM_BUFFER;
	return 0;
}

static INT32 iq_proc_cmd_get_param(ISP_MODULE *pdrv, INT32 argc, CHAR **argv)
{
	if (argc < 4) {
		iq_proc_cmd_printf("wrong argument:%d", argc);
		return -EINVAL;
	}

	iq_proc_id = simple_strtoul(argv[2], NULL, 0);
	iq_proc_iso = simple_strtoul(argv[3], NULL, 0);

	if (iq_flow_get_id_valid(iq_proc_id) && (iq_proc_id < IQ_ID_MAX_NUM)) {
		iq_proc_r_item = IQ_PROC_R_ITEM_PARAM;
	} else {
		iq_proc_r_item = IQ_PROC_R_ITEM_NONE;
	}

	return 0;
}

static INT32 iq_proc_cmd_get_ui_param(ISP_MODULE *pdrv, INT32 argc, CHAR **argv)
{
	if (argc < 3) {
		iq_proc_cmd_printf("wrong argument:%d", argc);
		return -EINVAL;
	}

	iq_proc_id = simple_strtoul(argv[2], NULL, 0);

	if (iq_flow_get_id_valid(iq_proc_id) && (iq_proc_id < IQ_ID_MAX_NUM)) {
		iq_proc_r_item = IQ_PROC_R_ITEM_UI_PARAM;
	} else {
		iq_proc_r_item = IQ_PROC_R_ITEM_NONE;
	}

	return 0;
}

static INT32 iq_proc_cmd_get_nnsc_param(ISP_MODULE *pdrv, INT32 argc, CHAR **argv)
{
	if (argc < 3) {
		iq_proc_cmd_printf("wrong argument:%d", argc);
		return -EINVAL;
	}

	iq_proc_id = simple_strtoul(argv[2], NULL, 0);

	if (iq_flow_get_id_valid(iq_proc_id) && (iq_proc_id < IQ_ID_MAX_NUM)) {
		iq_proc_r_item = IQ_PROC_R_ITEM_NNSC_PARAM;
	} else {
		iq_proc_r_item = IQ_PROC_R_ITEM_NONE;
	}

	return 0;
}

static INT32 iq_proc_cmd_get_cfg_data(ISP_MODULE *pdrv, INT32 argc, CHAR **argv)
{
	if (argc < 2) {
		iq_proc_cmd_printf("wrong argument:%d", argc);
		return -EINVAL;
	}

	iq_proc_r_item = IQ_PROC_R_ITEM_CFG_DATA;

	return 0;
}

static INT32 iq_proc_cmd_get_low_power_info(ISP_MODULE *pdrv, INT32 argc, CHAR **argv)
{
	if (argc < 2) {
		iq_proc_cmd_printf("wrong argument:%d", argc);
		return -EINVAL;
	}

	iq_proc_r_item = IQ_PROC_R_ITEM_LOW_POWER_INFO;

	return 0;
}

static INT32 iq_proc_cmd_get_low_power_param(ISP_MODULE *pdrv, INT32 argc, CHAR **argv)
{
	if (argc < 2) {
		iq_proc_cmd_printf("wrong argument:%d", argc);
		return -EINVAL;
	}

	iq_proc_r_item = IQ_PROC_R_ITEM_LOW_POWER_PARAM;

	return 0;
}

static INT32 iq_proc_cmd_get_manual_info(ISP_MODULE *pdrv, INT32 argc, CHAR **argv)
{
	if (argc < 2) {
		iq_proc_cmd_printf("wrong argument:%d", argc);
		return -EINVAL;
	}

	iq_proc_r_item = IQ_PROC_R_ITEM_MANUAL_INFO;

	return 0;
}

static IQ_PROC_CMD iq_proc_cmd_r_list[] = {
	// keyword              function name
	{ "buffer_size",        iq_proc_cmd_get_buffer_size,     "get iq buffer size."},
	{ "param",              iq_proc_cmd_get_param,           "get iq param, param1 is iq_id, param2 is param_id(0~15)."},
	{ "ui_param",           iq_proc_cmd_get_ui_param,        "get iq ui param, param1 is iq_id."},
	{ "nnsc_param",         iq_proc_cmd_get_nnsc_param,      "get iq nnsc param, param1 is iq_id."},
	{ "cfg_data",           iq_proc_cmd_get_cfg_data,        "get iq cfg file info."},
	{ "low_power_info",     iq_proc_cmd_get_low_power_info,  "get iq low power info, NT98539A only."},
	{ "low_power_param",    iq_proc_cmd_get_low_power_param, "get iq low power param, NT98539A only."},
	{ "manual_info",        iq_proc_cmd_get_manual_info,     "get iq ob/dgain/cgain manual info."},
};
#define NUM_OF_R_CMD (sizeof(iq_proc_cmd_r_list) / sizeof(IQ_PROC_CMD))

static INT32 iq_proc_cmd_set_dbg(ISP_MODULE *pdrv, INT32 argc, CHAR **argv)
{
	IQ_ID id;
	u32 cmd;

	if (argc < 4) {
		iq_proc_cmd_printf("wrong argument:%d", argc);
		return -EINVAL;
	}

	id = simple_strtoul(argv[2], NULL, 0);
	cmd = simple_strtoul(argv[3], NULL, 0);

	iq_proc_cmd_printf("set iq(%d) dbg level(0x%x) \n", id, cmd);
	iq_dbg_set_dbg_mode(id, cmd);
	return 0;
}

static INT32 iq_proc_cmd_set_ui_param(ISP_MODULE *pdrv, INT32 argc, CHAR **argv)
{
	IQ_ID id;
	IQ_UI_ITEM ui_type;
	UINT32 ui_value;

	if (argc < 5) {
		iq_proc_cmd_printf("wrong argument:%d", argc);
		iq_proc_cmd_printf("please set (id, ui_type, ui_value) \r\n");
		return -EINVAL;
	}
	id = simple_strtoul(argv[2], NULL, 0);
	ui_type = simple_strtoul(argv[3], NULL, 0);
	ui_value = simple_strtoul(argv[4], NULL, 0);

	iq_proc_cmd_printf("iq_sxcmd_set_ui_param \r\n");
	iq_proc_cmd_printf("id : %d \r\n", id);
	iq_proc_cmd_printf("set ui type : %d, value : %d \r\n", ui_type, ui_value);
	iq_ui_set_info(id, ui_type, ui_value);

	return 0;
}

static INT32 iq_proc_cmd_set_nnsc_param(ISP_MODULE *pdrv, INT32 argc, CHAR **argv)
{
	IQ_ID id;
	IQ_NNSC_ITEM nnsc_type;
	UINT32 nnsc_value;

	if (argc < 5) {
		iq_proc_cmd_printf("wrong argument:%d", argc);
		iq_proc_cmd_printf("please set (id, nnsc_type, nnsc_value) \r\n");
		return -EINVAL;
	}
	id = simple_strtoul(argv[2], NULL, 0);
	nnsc_type = simple_strtoul(argv[3], NULL, 0);
	nnsc_value = simple_strtoul(argv[4], NULL, 0);

	iq_proc_cmd_printf("iq_sxcmd_set_nnsc_param \r\n");
	iq_proc_cmd_printf("id : %d \r\n", id);
	iq_proc_cmd_printf("set nnsc type : %d, value : %d \r\n", nnsc_type, nnsc_value);
	iq_nnsc_set_info(id, nnsc_type, nnsc_value);

	return 0;
}

static INT32 iq_proc_cmd_reload_cfg(ISP_MODULE *pdrv, INT32 argc, CHAR **argv)
{
	IQT_CFG_INFO cfg_info = {0};

	if (argc < 4) {
		iq_proc_cmd_printf("wrong argument:%d", argc);
		return -EINVAL;
	}

	cfg_info.id = simple_strtoul(argv[2], NULL, 0);
	strncpy(cfg_info.path, argv[3], IQ_CFG_NAME_LENGTH - 1);
	cfg_info.path[IQ_CFG_NAME_LENGTH - 1] = '\0';

	iqt_api_set_cmd(IQT_ITEM_RLD_CONFIG, (ULONG)&cfg_info);

	return 0;
}

static INT32 iq_proc_cmd_reload_dtsi(ISP_MODULE *pdrv, INT32 argc, CHAR **argv)
{
	IQT_DTSI_INFO dtsi_info = {0};

	if (argc < 4) {
		iq_proc_cmd_printf("wrong argument:%d", argc);
		return -EINVAL;
	}

	dtsi_info.id = simple_strtoul(argv[2], NULL, 0);
	strncpy(dtsi_info.node_path, argv[3], IQ_DTSI_NAME_LENGTH - 1);
	dtsi_info.node_path[IQ_DTSI_NAME_LENGTH - 1] = '\0';
	strncpy(dtsi_info.file_path, argv[4], IQ_DTSI_NAME_LENGTH - 1);
	dtsi_info.file_path[IQ_DTSI_NAME_LENGTH - 1] = '\0';

	iqt_api_set_cmd(IQT_ITEM_RLD_DTSI, (ULONG)&dtsi_info);

	return 0;
}

static INT32 iq_proc_cmd_set_test_mode(ISP_MODULE *pdrv, INT32 argc, CHAR **argv)
{
	u32 id;
	u32 en;
	u32 ob_mode;
	u32 dg_mode;
	u32 cg_mode;

	if (argc < 7) {
		iq_proc_cmd_printf("wrong argument:%d", argc);
		iq_proc_cmd_printf("param1 = id, range 0~%d \r\n", IQ_ID_MAX_NUM - 1);
		iq_proc_cmd_printf("param2 = en, range 0~1 \r\n");
		iq_proc_cmd_printf("param3 = ob_mode, range 0~2(        %d:SIE, %d:IFE_F, %d:IFE) \r\n", IQ_OB_SIE, IQ_OB_IFE_F, IQ_OB_IFE);
		iq_proc_cmd_printf("param4 = dg_mode, range 0~4(%d:OFF, %d:SIE, %d:IFE_F, %d:IFE, %d:IPE) \r\n", IQ_DG_OFF, IQ_DG_SIE, IQ_DG_IFE_F, IQ_DG_IFE, IQ_DG_IPE);
		iq_proc_cmd_printf("param5 = cg_mode, range 0~3(        %d:SIE, %d:IFE_F, %d:IFE, %d:IPE) \r\n", IQ_CG_SIE, IQ_CG_IFE_F, IQ_CG_IFE, IQ_CG_IPE);
		return -EINVAL;
	}

	id = simple_strtoul(argv[2], NULL, 0);
	en = simple_strtoul(argv[3], NULL, 0);
	ob_mode = simple_strtoul(argv[4], NULL, 0);
	dg_mode = simple_strtoul(argv[5], NULL, 0);
	cg_mode = simple_strtoul(argv[6], NULL, 0);

	test_mode_en[id] = en;
	test_mode_ob[id] = ob_mode;
	test_mode_dg[id] = dg_mode;
	test_mode_cg[id] = cg_mode;
	iq_proc_cmd_printf("set iq_info(%d) test_mode(%d) ob_mode(%d) dg_mode(%d) cg_mode(%d) \r\n", id, en, ob_mode, dg_mode, cg_mode);
	return 0;
}

static INT32 iq_proc_cmd_set_sie_ob_manual(ISP_MODULE *pdrv, INT32 argc, CHAR **argv)
{
	u32 id;
	u32 en;
	u32 value;

	if (argc < 5) {
		iq_proc_cmd_printf("wrong argument:%d", argc);
		iq_proc_cmd_printf("param1 = id, range 0~%d \r\n", IQ_ID_MAX_NUM - 1);
		iq_proc_cmd_printf("param2 = en, range 0~1 \r\n");
		iq_proc_cmd_printf("param3 = value, range 0~4095 \r\n");
		return -EINVAL;
	}

	id = simple_strtoul(argv[2], NULL, 0);
	en = simple_strtoul(argv[3], NULL, 0);
	value = simple_strtoul(argv[4], NULL, 0);

	iq_flow_ext_set_sie_ob_manual(id, en, value);

	iq_proc_cmd_printf("set iq_info(%d) en(%d) value(%d) \r\n", id, en, value);
	return 0;
}

static INT32 iq_proc_cmd_set_pre_f_ob_manual(ISP_MODULE *pdrv, INT32 argc, CHAR **argv)
{
	u32 id;
	u32 en;
	u32 value[IQ_OB_LEN];

	if (argc < 9) {
		iq_proc_cmd_printf("wrong argument:%d", argc);
		iq_proc_cmd_printf("param1 = id, range 0~%d \r\n", IQ_ID_MAX_NUM - 1);
		iq_proc_cmd_printf("param2 = en, range 0~1 \r\n");
		iq_proc_cmd_printf("param3~param7 = value, range 0~4095 \r\n");
		return -EINVAL;
	}

	id = simple_strtoul(argv[2], NULL, 0);
	en = simple_strtoul(argv[3], NULL, 0);
	value[0] = simple_strtoul(argv[4], NULL, 0);
	value[1] = simple_strtoul(argv[5], NULL, 0);
	value[2] = simple_strtoul(argv[6], NULL, 0);
	value[3] = simple_strtoul(argv[7], NULL, 0);
	value[4] = simple_strtoul(argv[8], NULL, 0);

	iq_flow_ext_set_pre_f_ob_manual(id, en, (u32 *)&value);

	iq_proc_cmd_printf("set iq_info(%d) en(%d) value(%d %d %d %d %d) \r\n", id, en, value[0], value[1], value[2], value[3], value[4]);
	return 0;
}

static INT32 iq_proc_cmd_set_pre_ob_manual(ISP_MODULE *pdrv, INT32 argc, CHAR **argv)
{
	u32 id;
	u32 en;
	u32 value[IQ_OB_LEN];

	if (argc < 9) {
		iq_proc_cmd_printf("wrong argument:%d", argc);
		iq_proc_cmd_printf("param1 = id, range 0~%d \r\n", IQ_ID_MAX_NUM - 1);
		iq_proc_cmd_printf("param2 = en, range 0~1 \r\n");
		iq_proc_cmd_printf("param3~param7 = value, range 0~4095 \r\n");
		return -EINVAL;
	}

	id = simple_strtoul(argv[2], NULL, 0);
	en = simple_strtoul(argv[3], NULL, 0);
	value[0] = simple_strtoul(argv[4], NULL, 0);
	value[1] = simple_strtoul(argv[5], NULL, 0);
	value[2] = simple_strtoul(argv[6], NULL, 0);
	value[3] = simple_strtoul(argv[7], NULL, 0);
	value[4] = simple_strtoul(argv[8], NULL, 0);

	iq_flow_ext_set_pre_ob_manual(id, en, (u32 *)&value);

	iq_proc_cmd_printf("set iq_info(%d) en(%d) value(%d %d %d %d %d) \r\n", id, en, value[0], value[1], value[2], value[3], value[4]);
	return 0;
}

static INT32 iq_proc_cmd_set_ife_f_ob_manual(ISP_MODULE *pdrv, INT32 argc, CHAR **argv)
{
	u32 id;
	u32 en;
	u32 value[IQ_OB_LEN];

	if (argc < 9) {
		iq_proc_cmd_printf("wrong argument:%d", argc);
		iq_proc_cmd_printf("param1 = id, range 0~%d \r\n", IQ_ID_MAX_NUM - 1);
		iq_proc_cmd_printf("param2 = en, range 0~1 \r\n");
		iq_proc_cmd_printf("param3~param7 = value, range 0~4095 \r\n");
		return -EINVAL;
	}

	id = simple_strtoul(argv[2], NULL, 0);
	en = simple_strtoul(argv[3], NULL, 0);
	value[0] = simple_strtoul(argv[4], NULL, 0);
	value[1] = simple_strtoul(argv[5], NULL, 0);
	value[2] = simple_strtoul(argv[6], NULL, 0);
	value[3] = simple_strtoul(argv[7], NULL, 0);
	value[4] = simple_strtoul(argv[8], NULL, 0);

	iq_flow_ext_set_ife_f_ob_manual(id, en, (u32 *)&value);

	iq_proc_cmd_printf("set iq_info(%d) en(%d) value(%d %d %d %d %d) \r\n", id, en, value[0], value[1], value[2], value[3], value[4]);
	return 0;
}

static INT32 iq_proc_cmd_set_ife_ob_manual(ISP_MODULE *pdrv, INT32 argc, CHAR **argv)
{
	u32 id;
	u32 en;
	u32 value[IQ_OB_LEN];

	if (argc < 9) {
		iq_proc_cmd_printf("wrong argument:%d", argc);
		iq_proc_cmd_printf("param1 = id, range 0~%d \r\n", IQ_ID_MAX_NUM - 1);
		iq_proc_cmd_printf("param2 = en, range 0~1 \r\n");
		iq_proc_cmd_printf("param3~param7 = value, range 0~4095 \r\n");
		return -EINVAL;
	}

	id = simple_strtoul(argv[2], NULL, 0);
	en = simple_strtoul(argv[3], NULL, 0);
	value[0] = simple_strtoul(argv[4], NULL, 0);
	value[1] = simple_strtoul(argv[5], NULL, 0);
	value[2] = simple_strtoul(argv[6], NULL, 0);
	value[3] = simple_strtoul(argv[7], NULL, 0);
	value[4] = simple_strtoul(argv[8], NULL, 0);

	iq_flow_ext_set_ife_ob_manual(id, en, (u32 *)&value);

	iq_proc_cmd_printf("set iq_info(%d) en(%d) value(%d %d %d %d %d) \r\n", id, en, value[0], value[1], value[2], value[3], value[4]);
	return 0;
}

static INT32 iq_proc_cmd_set_ob_mode_manual(ISP_MODULE *pdrv, INT32 argc, CHAR **argv)
{
	u32 id;
	u32 en;
	u32 mode;

	if (argc < 5) {
		iq_proc_cmd_printf("wrong argument:%d", argc);
		iq_proc_cmd_printf("param1 = id, range 0~%d \r\n", IQ_ID_MAX_NUM - 1);
		iq_proc_cmd_printf("param2 = en, range 0~1 \r\n");
		iq_proc_cmd_printf("param3 = mode, range 0~4 \r\n");
		return -EINVAL;
	}

	id = simple_strtoul(argv[2], NULL, 0);
	en = simple_strtoul(argv[3], NULL, 0);
	mode = simple_strtoul(argv[4], NULL, 0);

	iq_flow_ext_set_ob_mode_manual(id, en, mode);

	iq_proc_cmd_printf("set iq_info(%d) en(%d) ob_mode(%d) \r\n", id, en, mode);
	return 0;
}

static INT32 iq_proc_cmd_set_sie_dg_manual(ISP_MODULE *pdrv, INT32 argc, CHAR **argv)
{
	u32 id;
	u32 en;
	u32 value;

	if (argc < 5) {
		iq_proc_cmd_printf("wrong argument:%d", argc);
		iq_proc_cmd_printf("param1 = id, range 0~%d \r\n", IQ_ID_MAX_NUM - 1);
		iq_proc_cmd_printf("param2 = en, range 0~1 \r\n");
		iq_proc_cmd_printf("param3 = value, range 0~32767 \r\n");
		return -EINVAL;
	}

	id = simple_strtoul(argv[2], NULL, 0);
	en = simple_strtoul(argv[3], NULL, 0);
	value = simple_strtoul(argv[4], NULL, 0);

	iq_flow_ext_set_sie_dg_manual(id, en, value);

	iq_proc_cmd_printf("set iq_info(%d) en(%d) value(%d) \r\n", id, en, value);
	return 0;
}

static INT32 iq_proc_cmd_set_pre_dg_manual(ISP_MODULE *pdrv, INT32 argc, CHAR **argv)
{
	u32 id;
	u32 en;
	u32 value;

	if (argc < 5) {
		iq_proc_cmd_printf("wrong argument:%d", argc);
		iq_proc_cmd_printf("param1 = id, range 0~%d \r\n", IQ_ID_MAX_NUM - 1);
		iq_proc_cmd_printf("param2 = en, range 0~1 \r\n");
		iq_proc_cmd_printf("param3 = value, range 0~32767 \r\n");
		return -EINVAL;
	}

	id = simple_strtoul(argv[2], NULL, 0);
	en = simple_strtoul(argv[3], NULL, 0);
	value = simple_strtoul(argv[4], NULL, 0);

	iq_flow_ext_set_pre_dg_manual(id, en, value);

	iq_proc_cmd_printf("set iq_info(%d) en(%d) value(%d) \r\n", id, en, value);
	return 0;
}

static INT32 iq_proc_cmd_set_ife_dg_manual(ISP_MODULE *pdrv, INT32 argc, CHAR **argv)
{
	u32 id;
	u32 en;
	u32 value;

	if (argc < 5) {
		iq_proc_cmd_printf("wrong argument:%d", argc);
		iq_proc_cmd_printf("param1 = id, range 0~%d \r\n", IQ_ID_MAX_NUM - 1);
		iq_proc_cmd_printf("param2 = en, range 0~1 \r\n");
		iq_proc_cmd_printf("param3 = value, range 0~32767 \r\n");
		return -EINVAL;
	}

	id = simple_strtoul(argv[2], NULL, 0);
	en = simple_strtoul(argv[3], NULL, 0);
	value = simple_strtoul(argv[4], NULL, 0);

	iq_flow_ext_set_ife_dg_manual(id, en, value);

	iq_proc_cmd_printf("set iq_info(%d) en(%d) value(%d) \r\n", id, en, value);
	return 0;
}

static INT32 iq_proc_cmd_set_dg_mode_manual(ISP_MODULE *pdrv, INT32 argc, CHAR **argv)
{
	u32 id;
	u32 en;
	u32 mode;

	if (argc < 5) {
		iq_proc_cmd_printf("wrong argument:%d", argc);
		iq_proc_cmd_printf("param1 = id, range 0~%d \r\n", IQ_ID_MAX_NUM - 1);
		iq_proc_cmd_printf("param2 = en, range 0~1 \r\n");
		iq_proc_cmd_printf("param3 = mode, range 0~5 \r\n");
		return -EINVAL;
	}

	id = simple_strtoul(argv[2], NULL, 0);
	en = simple_strtoul(argv[3], NULL, 0);
	mode = simple_strtoul(argv[4], NULL, 0);

	iq_flow_ext_set_dg_mode_manual(id, en, mode);

	iq_proc_cmd_printf("set iq_info(%d) en(%d) dg_mode(%d) \r\n", id, en, mode);
	return 0;
}

static INT32 iq_proc_cmd_set_sie_dg_max(ISP_MODULE *pdrv, INT32 argc, CHAR **argv)
{
	u32 id;
	u32 sie_dg_max;

	if (argc < 4) {
		iq_proc_cmd_printf("wrong argument:%d", argc);
		iq_proc_cmd_printf("param1 = id, range 0~%d \r\n", IQ_ID_MAX_NUM - 1);
		iq_proc_cmd_printf("param2 = sie_dg_max, range 0~32767 \r\n");
		return -EINVAL;
	}

	id = simple_strtoul(argv[2], NULL, 0);
	sie_dg_max = simple_strtoul(argv[3], NULL, 0);

	iq_flow_ext_set_sie_dg_max(id, sie_dg_max);

	iq_proc_cmd_printf("set iq_info(%d) sie_dg_max(%d) \r\n", id, sie_dg_max);
	return 0;
}

static INT32 iq_proc_cmd_set_cg_mode_manual(ISP_MODULE *pdrv, INT32 argc, CHAR **argv)
{
	u32 id;
	u32 en;
	u32 mode;

	if (argc < 5) {
		iq_proc_cmd_printf("wrong argument:%d", argc);
		iq_proc_cmd_printf("param1 = id, range 0~%d \r\n", IQ_ID_MAX_NUM - 1);
		iq_proc_cmd_printf("param2 = en, range 0~1 \r\n");
		iq_proc_cmd_printf("param3 = mode, range 0~4 \r\n");
		return -EINVAL;
	}

	id = simple_strtoul(argv[2], NULL, 0);
	en = simple_strtoul(argv[3], NULL, 0);
	mode = simple_strtoul(argv[4], NULL, 0);

	iq_flow_ext_set_cg_mode_manual(id, en, mode);

	iq_proc_cmd_printf("set iq_info(%d) en(%d) cg_mode(%d) \r\n", id, en, mode);
	return 0;
}

static INT32 iq_proc_cmd_set_dg_ai(ISP_MODULE *pdrv, INT32 argc, CHAR **argv)
{
	u32 id;
	u32 en;

	if (argc < 4) {
		iq_proc_cmd_printf("wrong argument:%d", argc);
		iq_proc_cmd_printf("param1 = id, range 0~%d \r\n", IQ_ID_MAX_NUM - 1);
		iq_proc_cmd_printf("param2 = en, range 0~1 \r\n");
		return -EINVAL;
	}

	id = simple_strtoul(argv[2], NULL, 0);
	en = simple_strtoul(argv[3], NULL, 0);

	iq_flow_ext_set_dg_ai(id, en);

	iq_proc_cmd_printf("set iq_info(%d) en(%d) \r\n", id, en);
	return 0;
}

static INT32 iq_proc_cmd_set_companding_en(ISP_MODULE *pdrv, INT32 argc, CHAR **argv)
{
	u32 id;
	u32 en;

	if (argc < 4) {
		iq_proc_cmd_printf("wrong argument:%d", argc);
		return -EINVAL;
	}

	id = simple_strtoul(argv[2], NULL, 0);
	en = simple_strtoul(argv[3], NULL, 0);

	companding_en[id] = en;
	iq_proc_cmd_printf("set iq_info(%d) companding_en = %d\r\n", id, en);
	return 0;
}

static INT32 iq_proc_cmd_set_nr_ratio(ISP_MODULE *pdrv, INT32 argc, CHAR **argv)
{
	IQ_ID id;
	u32 r_ratio;
	u32 b_ratio;

	if (argc < 5) {
		iq_proc_cmd_printf("wrong argument:%d", argc);
		return -EINVAL;
	}

	id = simple_strtoul(argv[2], NULL, 0);
	r_ratio = simple_strtoul(argv[3], NULL, 0);
	b_ratio = simple_strtoul(argv[4], NULL, 0);

	nr_r_ratio[id] = r_ratio;
	nr_b_ratio[id] = b_ratio;
	iq_proc_cmd_printf("set iq_info(%d) nr_r_ratio(%d) nr_b_ratio(%d)\n", id, r_ratio, b_ratio);
	return 0;
}

static INT32 iq_proc_cmd_set_nr_ir_ratio(ISP_MODULE *pdrv, INT32 argc, CHAR **argv)
{
	IQ_ID id;
	u32 ir_ratio;

	if (argc < 4) {
		iq_proc_cmd_printf("wrong argument:%d", argc);
		return -EINVAL;
	}

	id = simple_strtoul(argv[2], NULL, 0);
	ir_ratio = simple_strtoul(argv[3], NULL, 0);

	nr_ir_ratio[id] = ir_ratio;
	iq_proc_cmd_printf("set iq_info(%d) nr_ir_ratio(%d) \n", id, ir_ratio);
	return 0;
}

static INT32 iq_proc_cmd_set_nr_size(ISP_MODULE *pdrv, INT32 argc, CHAR **argv)
{
	IQ_ID id;
	u32 size;

	if (argc < 4) {
		iq_proc_cmd_printf("wrong argument:%d", argc);
		iq_proc_cmd_printf("param1 = id, range 0~%d", IQ_ID_MAX_NUM - 1);
		iq_proc_cmd_printf("param2 = en, range 0~1(0:5x5, 1:7x7)");
		return -EINVAL;
	}

	id = simple_strtoul(argv[2], NULL, 0);
	size = simple_strtoul(argv[3], NULL, 0);

	iq_flow_ext_set_nr_size(id, size);
	return 0;
}

static INT32 iq_proc_cmd_set_fcurve_en(ISP_MODULE *pdrv, INT32 argc, CHAR **argv)
{
	IQ_ID id;
	BOOL en;

	if (argc < 4) {
		iq_proc_cmd_printf("wrong argument:%d", argc);
		return -EINVAL;
	}

	id = simple_strtoul(argv[2], NULL, 0);
	en = simple_strtoul(argv[3], NULL, 0);

	fcurve_en[id] = en;
	iq_proc_cmd_printf("set iq_info(%d) fcurve_en = %d\r\n", id, en);
	return 0;
}

static INT32 iq_proc_cmd_set_cfa_en(ISP_MODULE *pdrv, INT32 argc, CHAR **argv)
{
	IQ_ID id;
	BOOL en;

	if (argc < 4) {
		iq_proc_cmd_printf("wrong argument:%d", argc);
		return -EINVAL;
	}

	id = simple_strtoul(argv[2], NULL, 0);
	en = simple_strtoul(argv[3], NULL, 0);

	cfa_en[id] = en;
	iq_proc_cmd_printf("set iq_info(%d) cfa_en = %d\r\n", id, en);
	return 0;
}

static INT32 iq_proc_cmd_set_irsub(ISP_MODULE *pdrv, INT32 argc, CHAR **argv)
{
	IQ_ID id;
	u32 lb;
	u32 th;
	u32 rng;

	if (argc < 6) {
		iq_proc_cmd_printf("wrong argument:%d", argc);
		return -EINVAL;
	}

	id = simple_strtoul(argv[2], NULL, 0);
	lb = simple_strtoul(argv[3], NULL, 0);
	th = simple_strtoul(argv[4], NULL, 0);
	rng = simple_strtoul(argv[5], NULL, 0);

	iq_flow_ext_set_irsub(id, lb, th, rng);
	return 0;
}

static INT32 iq_proc_cmd_set_pink_red(ISP_MODULE *pdrv, INT32 argc, CHAR **argv)
{
	IQ_ID id;
	BOOL en;
	u32 mode;
	u32 th1;
	u32 th2;

	if (argc < 7) {
		iq_proc_cmd_printf("wrong argument:%d", argc);
		return -EINVAL;
	}

	id = simple_strtoul(argv[2], NULL, 0);
	en = simple_strtoul(argv[3], NULL, 0);
	mode = simple_strtoul(argv[4], NULL, 0);
	th1 = simple_strtoul(argv[5], NULL, 0);
	th2 = simple_strtoul(argv[6], NULL, 0);

	iq_flow_ext_set_pink_red(id, en, mode, th1, th2);
	return 0;
}

static INT32 iq_proc_cmd_set_test_shdr(ISP_MODULE *pdrv, INT32 argc, CHAR **argv)
{
	u32 id;
	u32 en;
	u32 ev_ratio[ISP_SEN_MFRAME_MAX_NUM];
	u32 tm_ratio;

	if (argc < 8) {
		iq_proc_cmd_printf("wrong argument:%d", argc);
		return -EINVAL;
	}

	id = simple_strtoul(argv[2], NULL, 0);
	en = simple_strtoul(argv[3], NULL, 0);
	ev_ratio[0] = simple_strtoul(argv[4], NULL, 0);
	ev_ratio[1] = simple_strtoul(argv[5], NULL, 0);
	//ev_ratio[2] = simple_strtoul(argv[6], NULL, 0);
	tm_ratio = simple_strtoul(argv[7], NULL, 0);

	test_shdr_en[id] = en;
	test_ev_ratio[id][0] = ev_ratio[0];
	test_ev_ratio[id][1] = ev_ratio[1];
	//test_ev_ratio[id][2] = ev_ratio[2];
	test_tm_ratio[id] = tm_ratio;
	iq_proc_cmd_printf("set iq_info(%d) test_shdr(%d) ev_ratio(%d, %d, --) tm_ratio(%d) \r\n", id, en, ev_ratio[0], ev_ratio[1], tm_ratio);
	return 0;
}

static INT32 iq_proc_cmd_set_test_shdr_hbs(ISP_MODULE *pdrv, INT32 argc, CHAR **argv)
{
	u32 id;
	u32 en;
	u32 lum_th;
	u32 w_start;
	u32 w_slope;

	if (argc < 7) {
		iq_proc_cmd_printf("wrong argument:%d", argc);
		return -EINVAL;
	}

	id = simple_strtoul(argv[2], NULL, 0);
	en = simple_strtoul(argv[3], NULL, 0);
	lum_th = simple_strtoul(argv[4], NULL, 0);
	w_start = simple_strtoul(argv[5], NULL, 0);
	w_slope = simple_strtoul(argv[6], NULL, 0);

	test_shdr_hbs_en[id] = en;
	test_hbs[id].lum_th = lum_th;
	test_hbs[id].w_start = w_start;
	test_hbs[id].w_slope = w_slope;
	iq_proc_cmd_printf("set iq_info(%d) test_shdr_hbs_en(%d) test_hbs(%d, %d, %d) \r\n", id, en, lum_th, w_start, w_slope);
	return 0;
}

static INT32 iq_proc_cmd_set_wdr_subimg_lpf(ISP_MODULE *pdrv, INT32 argc, CHAR **argv)
{
	u32 id;
	u32 coef_0;
	u32 coef_1;
	u32 coef_2;

	if (argc < 6) {
		iq_proc_cmd_printf("wrong argument:%d", argc);
		return -EINVAL;
	}

	id = simple_strtoul(argv[2], NULL, 0);
	coef_0 = simple_strtoul(argv[3], NULL, 0);
	coef_1 = simple_strtoul(argv[4], NULL, 0);
	coef_2 = simple_strtoul(argv[5], NULL, 0);

	iq_flow_ext_set_wdr_subimg_lpf(id, coef_0, coef_1, coef_2);

	return 0;
}

static INT32 iq_proc_cmd_set_wdr_blend_w(ISP_MODULE *pdrv, INT32 argc, CHAR **argv)
{
	u32 id;
	u32 blend_w;

	if (argc < 4) {
		iq_proc_cmd_printf("wrong argument:%d", argc);
		return -EINVAL;
	}

	id = simple_strtoul(argv[2], NULL, 0);
	blend_w = simple_strtoul(argv[3], NULL, 0);

	iq_flow_ext_set_wdr_blend_w(id, blend_w);

	return 0;
}

static INT32 iq_proc_cmd_set_rgblpf(ISP_MODULE *pdrv, INT32 argc, CHAR **argv)
{
	IQ_ID id;
	BOOL en;
	u32 th0;
	u32 th1;

	if (argc < 6) {
		iq_proc_cmd_printf("wrong argument:%d", argc);
		return -EINVAL;
	}

	id = simple_strtoul(argv[2], NULL, 0);
	en = simple_strtoul(argv[3], NULL, 0);
	th0 = simple_strtoul(argv[4], NULL, 0);
	th1 = simple_strtoul(argv[5], NULL, 0);

	iq_flow_ext_set_rgblpf(id, en, th0, th1);

	return 0;
}

static INT32 iq_proc_cmd_set_ipe_subimg_size(ISP_MODULE *pdrv, INT32 argc, CHAR **argv)
{
	u32 id;
	u32 width;
	u32 height;

	if (argc < 5) {
		iq_proc_cmd_printf("wrong argument:%d", argc);
		return -EINVAL;
	}

	id = simple_strtoul(argv[2], NULL, 0);
	width = simple_strtoul(argv[3], NULL, 0);
	height = simple_strtoul(argv[4], NULL, 0);

	iq_flow_ext_set_ipe_subimg_size(id, width, height);

	return 0;
}

static INT32 iq_proc_cmd_set_edge_gamma_sel(ISP_MODULE *pdrv, INT32 argc, CHAR **argv)
{
	u32 id;
	u32 sel;

	if (argc < 4) {
		iq_proc_cmd_printf("wrong argument:%d", argc);
		return -EINVAL;
	}

	id = simple_strtoul(argv[2], NULL, 0);
	sel = simple_strtoul(argv[3], NULL, 0);

	iq_flow_ext_set_edge_gamma_sel(id, sel);

	return 0;
}

static INT32 iq_proc_cmd_set_edge_region_str(ISP_MODULE *pdrv, INT32 argc, CHAR **argv)
{
	u32 id;
	u32 enable;
	u32 enh_thin;
	u32 enh_robust;

	if (argc < 6) {
		iq_proc_cmd_printf("wrong argument:%d", argc);
		return -EINVAL;
	}

	id = simple_strtoul(argv[2], NULL, 0);
	enable = simple_strtoul(argv[3], NULL, 0);
	enh_thin = simple_strtoul(argv[4], NULL, 0);
	enh_robust= simple_strtoul(argv[5], NULL, 0);

	iq_flow_ext_set_edge_region_str(id, enable, enh_thin, enh_robust);

	return 0;
}

static INT32 iq_proc_cmd_set_edge_overshoot_w(ISP_MODULE *pdrv, INT32 argc, CHAR **argv)
{
	u32 id;
	u32 overshoot_w;
	u32 undershoot_w;

	if (argc < 5) {
		iq_proc_cmd_printf("wrong argument:%d", argc);
		return -EINVAL;
	}

	id = simple_strtoul(argv[2], NULL, 0);
	overshoot_w = simple_strtoul(argv[3], NULL, 0);
	undershoot_w = simple_strtoul(argv[4], NULL, 0);

	iq_flow_ext_set_edge_overshoot_w(id, overshoot_w, undershoot_w);

	return 0;
}

static INT32 iq_proc_cmd_set_edge_dir_th_shift(ISP_MODULE *pdrv, INT32 argc, CHAR **argv)
{
	u32 id;
	u32 th_shift;

	if (argc < 4) {
		iq_proc_cmd_printf("wrong argument:%d", argc);
		return -EINVAL;
	}

	id = simple_strtoul(argv[2], NULL, 0);
	th_shift = simple_strtoul(argv[3], NULL, 0);

	iq_flow_ext_set_edge_dir_th_shift(id, th_shift);

	return 0;
}

static INT32 iq_proc_cmd_set_defog_min_diff(ISP_MODULE *pdrv, INT32 argc, CHAR **argv)
{
	u32 id;
	u32 min_diff;

	if (argc < 4) {
		iq_proc_cmd_printf("wrong argument:%d", argc);
		return -EINVAL;
	}

	id = simple_strtoul(argv[2], NULL, 0);
	min_diff = simple_strtoul(argv[3], NULL, 0);

	iq_flow_ext_set_defog_min_diff(id, min_diff);

	return 0;
}

static INT32 iq_proc_cmd_set_dbcs_mode(ISP_MODULE *pdrv, INT32 argc, CHAR **argv)
{
	u32 id;
	u32 mode;

	if (argc < 4) {
		iq_proc_cmd_printf("wrong argument:%d", argc);
		iq_proc_cmd_printf("param1 = id, range 0~%d", IQ_ID_MAX_NUM - 1);
		iq_proc_cmd_printf("param2 = mode, range 0~2(0:Dark, 1:Bright, 2:Both)");
		return -EINVAL;
	}

	id = simple_strtoul(argv[2], NULL, 0);
	mode = simple_strtoul(argv[3], NULL, 0);

	iq_flow_ext_set_dbcs_mode(id, mode);

	return 0;
}

static INT32 iq_proc_cmd_set_dbcs_y_wt(ISP_MODULE *pdrv, INT32 argc, CHAR **argv)
{
	u32 id;
	u32 wt0;
	u32 wt1;
	u32 wt2;
	u32 wt3;
	u32 wt4;
	u32 wt5;
	u32 wt6;
	u32 wt7;
	u32 wt8;
	u32 wt9;
	u32 wt10;
	u32 wt11;
	u32 wt12;
	u32 wt13;
	u32 wt14;
	u32 wt15;

	if (argc < 19) {
		iq_proc_cmd_printf("wrong argument:%d", argc);
		return -EINVAL;
	}

	id = simple_strtoul(argv[2], NULL, 0);
	wt0 = simple_strtoul(argv[3], NULL, 0);
	wt1 = simple_strtoul(argv[4], NULL, 0);
	wt2 = simple_strtoul(argv[5], NULL, 0);
	wt3 = simple_strtoul(argv[6], NULL, 0);
	wt4 = simple_strtoul(argv[7], NULL, 0);
	wt5 = simple_strtoul(argv[8], NULL, 0);
	wt6 = simple_strtoul(argv[9], NULL, 0);
	wt7 = simple_strtoul(argv[10], NULL, 0);
	wt8 = simple_strtoul(argv[11], NULL, 0);
	wt9 = simple_strtoul(argv[12], NULL, 0);
	wt10 = simple_strtoul(argv[13], NULL, 0);
	wt11 = simple_strtoul(argv[14], NULL, 0);
	wt12 = simple_strtoul(argv[15], NULL, 0);
	wt13 = simple_strtoul(argv[16], NULL, 0);
	wt14 = simple_strtoul(argv[17], NULL, 0);
	wt15 = simple_strtoul(argv[18], NULL, 0);

	iq_flow_ext_set_dbcs_y_wt(id, wt0, wt1, wt2, wt3, wt4, wt5, wt6, wt7, wt8, wt9, wt10, wt11, wt12, wt13, wt14, wt15);

	return 0;
}

static INT32 iq_proc_cmd_set_dbcs_c_wt(ISP_MODULE *pdrv, INT32 argc, CHAR **argv)
{
	u32 id;
	u32 wt0;
	u32 wt1;
	u32 wt2;
	u32 wt3;
	u32 wt4;
	u32 wt5;
	u32 wt6;
	u32 wt7;
	u32 wt8;
	u32 wt9;
	u32 wt10;
	u32 wt11;
	u32 wt12;
	u32 wt13;
	u32 wt14;
	u32 wt15;

	if (argc < 19) {
		iq_proc_cmd_printf("wrong argument:%d", argc);
		return -EINVAL;
	}

	id = simple_strtoul(argv[2], NULL, 0);
	wt0 = simple_strtoul(argv[3], NULL, 0);
	wt1 = simple_strtoul(argv[4], NULL, 0);
	wt2 = simple_strtoul(argv[5], NULL, 0);
	wt3 = simple_strtoul(argv[6], NULL, 0);
	wt4 = simple_strtoul(argv[7], NULL, 0);
	wt5 = simple_strtoul(argv[8], NULL, 0);
	wt6 = simple_strtoul(argv[9], NULL, 0);
	wt7 = simple_strtoul(argv[10], NULL, 0);
	wt8 = simple_strtoul(argv[11], NULL, 0);
	wt9 = simple_strtoul(argv[12], NULL, 0);
	wt10 = simple_strtoul(argv[13], NULL, 0);
	wt11 = simple_strtoul(argv[14], NULL, 0);
	wt12 = simple_strtoul(argv[15], NULL, 0);
	wt13 = simple_strtoul(argv[16], NULL, 0);
	wt14 = simple_strtoul(argv[17], NULL, 0);
	wt15 = simple_strtoul(argv[18], NULL, 0);

	iq_flow_ext_set_dbcs_c_wt(id, wt0, wt1, wt2, wt3, wt4, wt5, wt6, wt7, wt8, wt9, wt10, wt11, wt12, wt13, wt14, wt15);

	return 0;
}

static INT32 iq_proc_cmd_set_residue_reset(ISP_MODULE *pdrv, INT32 argc, CHAR **argv)
{
	IQ_ID id;
	u32 reset_freq;

	if (argc < 4) {
		iq_proc_cmd_printf("wrong argument:%d", argc);
		return -EINVAL;
	}

	id = simple_strtoul(argv[2], NULL, 0);
	reset_freq = simple_strtoul(argv[3], NULL, 0);

	residue_reset_num[id] = reset_freq;
	iq_proc_cmd_printf("set iq(%d) reset(%d)\n", id, reset_freq);

	return 0;
}

static INT32 iq_proc_cmd_set_lca_location(ISP_MODULE *pdrv, INT32 argc, CHAR **argv)
{
	u32 id;
	u32 location;

	if (argc < 4) {
		iq_proc_cmd_printf("wrong argument:%d", argc);
		iq_proc_cmd_printf("param1 = id, range 0~%d \r\n", IQ_ID_MAX_NUM - 1);
		iq_proc_cmd_printf("param2 = location, range 0~1(0:Pre-3DNR, 1:Post-3DNR)\r\n");
		return -EINVAL;
	}

	id = simple_strtoul(argv[2], NULL, 0);
	location = simple_strtoul(argv[3], NULL, 0);

	iq_flow_ext_set_lca_location(id, location);

	return 0;
}

static INT32 iq_proc_cmd_set_lca_edge_ker_size(ISP_MODULE *pdrv, INT32 argc, CHAR **argv)
{
	u32 id;
	u32 ker_size;

	id = simple_strtoul(argv[2], NULL, 0);
	ker_size = simple_strtoul(argv[3], NULL, 0);

	iq_flow_ext_set_lca_edge_ker_size(id, ker_size);
	return 0;
}

static INT32 iq_proc_cmd_set_lca_y_out(ISP_MODULE *pdrv, INT32 argc, CHAR **argv)
{
	u32 id;
	u32 y_out;

	id = simple_strtoul(argv[2], NULL, 0);
	y_out = simple_strtoul(argv[3], NULL, 0);

	iq_flow_ext_set_lca_y_out(id, y_out);

	return 0;
}

static INT32 iq_proc_cmd_set_lca_c_out(ISP_MODULE *pdrv, INT32 argc, CHAR **argv)
{
	u32 id;
	u32 c_out;

	id = simple_strtoul(argv[2], NULL, 0);
	c_out = simple_strtoul(argv[3], NULL, 0);

	iq_flow_ext_set_lca_c_out(id, c_out);

	return 0;
}

static INT32 iq_proc_cmd_set_tmnr_ae_still_en(ISP_MODULE *pdrv, INT32 argc, CHAR **argv)
{
	u32 id;
	BOOL en;

	if (argc < 4) {
		iq_proc_cmd_printf("wrong argument:%d", argc);
		return -EINVAL;
	}

	id = simple_strtoul(argv[2], NULL, 0);
	en = simple_strtoul(argv[3], NULL, 0);

	tmnr_ae_still_en[id] = en;

	iq_proc_cmd_printf("set iq_info(%d) tmnr_ae_still_en = %d\r\n", id, en);

	return 0;
}

static INT32 iq_proc_cmd_set_post_sharpen_con_eng(ISP_MODULE *pdrv, INT32 argc, CHAR **argv)
{
	u32 id;
	u32 con_eng;

	if (argc < 4) {
		iq_proc_cmd_printf("wrong argument:%d", argc);
		return -EINVAL;
	}

	id = simple_strtoul(argv[2], NULL, 0);
	con_eng = simple_strtoul(argv[3], NULL, 0);

	iq_flow_ext_set_post_sharpen_con_eng(id, con_eng);

	return 0;
}

// NOTE: aiisp aided test
BOOL aiisp_aided_test_en = FALSE;
UINT32 aiisp_aided_test_clone_id = 0;
UINT32 aiisp_aided_test_src_id = 0;
static INT32 iq_proc_cmd_set_aiisp_aided_test(ISP_MODULE *pdrv, INT32 argc, CHAR **argv)
{
	if (argc < 4) {
		iq_proc_cmd_printf("wrong argument:%d", argc);
		return -EINVAL;
	}

	aiisp_aided_test_en = simple_strtoul(argv[2], NULL, 0);
	aiisp_aided_test_clone_id = simple_strtoul(argv[3], NULL, 0);
	aiisp_aided_test_src_id = simple_strtoul(argv[4], NULL, 0);

	return 0;
}

#if 0
static INT32 iq_proc_cmd_set_wdr(ISP_MODULE *pdrv, INT32 argc, CHAR **argv)
{
	u32 dark_th;
	u32 dr_th;
	u32 dark_compensate;

	if (argc < 5) {
		iq_proc_cmd_printf("wrong argument:%d", argc);
		return -EINVAL;
	}

	dark_th = simple_strtoul(argv[2], NULL, 0);
	dr_th = simple_strtoul(argv[3], NULL, 0);
	dark_compensate = simple_strtoul(argv[4], NULL, 0);

	iq_auto_wdr_dark_th = dark_th;
	iq_auto_wdr_dr_th = dr_th;
	iq_auto_wdr_dark_compensate = dark_compensate;
	iq_proc_cmd_printf("set WDR iq_auto_wdr_dark_th(%d) iq_auto_wdr_dr_th(%d) iq_auto_wdr_dark_compensate(%d)\r\n", dark_th, dr_th, dark_compensate);
	return 0;
}

static INT32 iq_proc_cmd_set_airlight(ISP_MODULE *pdrv, INT32 argc, CHAR **argv)
{
	u32 airlight_th;
	u32 airlight_diff_max_ratio;

	if (argc < 4) {
		iq_proc_cmd_printf("wrong argument:%d", argc);
		return -EINVAL;
	}

	airlight_th = simple_strtoul(argv[2], NULL, 0);
	airlight_diff_max_ratio = simple_strtoul(argv[3], NULL, 0);

	iq_auto_defog_airlight_th = airlight_th;
	iq_auto_defog_airlight_diff_max_ratio = airlight_diff_max_ratio;
	iq_proc_cmd_printf("set AIRLIGHT iq_auto_defog_airlight_th(%d) iq_auto_defog_airlight_diff_max(%d)\r\n", airlight_th, airlight_diff_max_ratio);
	return 0;
}

static INT32 iq_proc_cmd_set_defog(ISP_MODULE *pdrv, INT32 argc, CHAR **argv)
{
	u32 cnt_ratio_th1;
	u32 cnt_ratio_th2;

	if (argc < 4) {
		iq_proc_cmd_printf("wrong argument:%d", argc);
		return -EINVAL;
	}

	cnt_ratio_th1 = simple_strtoul(argv[2], NULL, 0);
	cnt_ratio_th2 = simple_strtoul(argv[3], NULL, 0);

	iq_auto_defog_cnt_ratio_th1 = cnt_ratio_th1;
	iq_auto_defog_cnt_ratio_th2 = cnt_ratio_th2;
	iq_proc_cmd_printf("set AIRLIGHT iq_auto_defog_cnt_ratio_th1(%d) iq_auto_defog_cnt_ratio_th2(%d)\r\n", cnt_ratio_th1, cnt_ratio_th2);
	return 0;
}
#endif

static IQ_PROC_CMD iq_proc_cmd_w_list[] = {
	// keyword              function name
	{ "dbg",                iq_proc_cmd_set_dbg,                    "set iq dbg level, param1 is iq_id, param2 is dbg_lv."},
	{ "ui_param",           iq_proc_cmd_set_ui_param,               "set iq ui param, param1 is iq_id, param2 is ui_type, param3 is ui_value."},
	{ "nnsc_param",         iq_proc_cmd_set_nnsc_param,             "set iq nnsc param, param1 is iq_id, param2 is nnsc_type, param3 is nnsc_value."},
	{ "reload_cfg",         iq_proc_cmd_reload_cfg,                 "reload cfg file, param1 is iq_id, param2 is path of cfg file."},
	{ "reload_dtsi",        iq_proc_cmd_reload_dtsi,                "reload dtsi ile, param1 is iq_id, param2 is node path, param3 is file path."},
	// test proc item with id
	{ "mode",               iq_proc_cmd_set_test_mode,              "set iq test mode, param1 is iq_id, param2 is en, param3 is ob_mode, param4 is dg_mode, param5 is cg_mode."},
	{ "sie_ob_manual",      iq_proc_cmd_set_sie_ob_manual,          "set iq sie ob manual, param1 is iq_id, param2 is enable, param3 is ob_value."},
	{ "pref_ob_manual",     iq_proc_cmd_set_pre_f_ob_manual,        "set iq pref ob manual, param1 is iq_id, param2 is enable, param3~param7 is ob_value."},
	{ "pre_ob_manual",      iq_proc_cmd_set_pre_ob_manual,          "set iq pre ob manual, param1 is iq_id, param2 is enable, param3~param7 is ob_value."},
	{ "ifef_ob_manual",     iq_proc_cmd_set_ife_f_ob_manual,        "set iq ifef ob manual, param1 is iq_id, param2 is enable, param3~param7 is ob_value."},
	{ "ife_ob_manual",      iq_proc_cmd_set_ife_ob_manual,          "set iq ife ob manual, param1 is iq_id, param2 is enable, param3~param7 is ob_value."},
	{ "ob_mode_manual",     iq_proc_cmd_set_ob_mode_manual,         "set iq ob mode manual, param1 is iq_id, param2 is enable, param3 is mode."},
	{ "sie_dg_manual",      iq_proc_cmd_set_sie_dg_manual,          "set iq sie dg manual, param1 is iq_id, param2 is enable, param3 is dg_value."},
	{ "pre_dg_manual",      iq_proc_cmd_set_pre_dg_manual,          "set iq pre dg manual, param1 is iq_id, param2 is enable, param3 is dg_value."},
	{ "ife_dg_manual",      iq_proc_cmd_set_ife_dg_manual,          "set iq ife dg manual, param1 is iq_id, param2 is enable, param3 is dg_value."},
	{ "dg_mode_manual",     iq_proc_cmd_set_dg_mode_manual,         "set iq dg mode manual, param1 is iq_id, param2 is enable, param3 is mode."},
	{ "sie_dg_max",         iq_proc_cmd_set_sie_dg_max,             "set iq sie dg max, param1 is iq_id, param2 is dg max."},
	{ "cg_mode_manual",     iq_proc_cmd_set_cg_mode_manual,         "set iq cg mode manual, param1 is iq_id, param2 is enable, param3 is mode."},
	{ "dg_ai_en",           iq_proc_cmd_set_dg_ai,                  "set iq dg ai enable, param1 is iq_id, param2 is enable."},
	{ "companding",         iq_proc_cmd_set_companding_en,          "set iq companding en, param1 is iq_id, param2 is companding_en."},
	{ "nr",                 iq_proc_cmd_set_nr_ratio,               "set iq nr ratio, param1 is iq_id, param2 is r_ratio, param3 is b_ratio."},
	{ "nr_ir",              iq_proc_cmd_set_nr_ir_ratio,            "set iq nr_ir ratio, param1 is iq_id, param2 is ir_ratio."},
	{ "nr_size",            iq_proc_cmd_set_nr_size,                "set iq nr size, param1 is iq_id, param2 is size_type."},
	{ "fcurve_en",          iq_proc_cmd_set_fcurve_en,              "set iq fcurve en, param1 is iq_id, param2 is fcurve_en."},
	{ "cfa_en",             iq_proc_cmd_set_cfa_en,                 "set iq cfa en, param1 is iq_id, param2 is cfa_en."},
	{ "irsub",              iq_proc_cmd_set_irsub,                  "set iq irsub, param1 is iq_id, param2 is lb, param3 is th, param4 is rng."},
	{ "pink_red",           iq_proc_cmd_set_pink_red,               "set iq pink_red, param1 is iq_id, param2 is en, param3 is mode, param4 is th1, param5 is th2."},
	{ "shdr",               iq_proc_cmd_set_test_shdr,              "set iq test shdr, param1 is iq_id, param2 is en, param3~5 is ev_ratio, param6 is tm_ratio."},
	{ "shdr_hbs",           iq_proc_cmd_set_test_shdr_hbs,          "set iq test shdr hbs, param1 is iq_id, param2 is en, param3 is lum_th, param4 is w_start, param5 is w_slope."},
	{ "wdr_lpf",            iq_proc_cmd_set_wdr_subimg_lpf,         "set iq wdr_subimg_lpf, param1 is iq_id, param2 is coef_0, param3 is coef_1, param4 is coef_2."},
	{ "wdr_blend_w",        iq_proc_cmd_set_wdr_blend_w,            "set iq wdr_blend_w, param1 is iq_id, param2 is blend_w."},
	{ "rgblpf",             iq_proc_cmd_set_rgblpf,                 "set iq rgb lpf, param1 is iq_id, param2 is en, param3 is th0, param4 is th1."},
	{ "ipe_subimg",         iq_proc_cmd_set_ipe_subimg_size,        "set iq ipe_subimg_size, param1 is iq_id, param2 is width, param3 is height."},
	{ "edge_gamma_sel",     iq_proc_cmd_set_edge_gamma_sel,         "set iq edge_gamma_sel, param1 is iq_id, param2 is sel."},
	{ "edge_region_str",    iq_proc_cmd_set_edge_region_str,        "set iq edge region str, param1 is iq_id, param2 is enable, param3 is enh_thin, param4 is enh_robust."},
	{ "edge_overshoot_w",   iq_proc_cmd_set_edge_overshoot_w,       "set iq edge region str, param1 is iq_id, param2 is overshoot_w, param3 is undershoot_w."},
	{ "edge_dir_th_shift",  iq_proc_cmd_set_edge_dir_th_shift,      "set iq edge region str, param1 is iq_id, param2 is th_shift."},
	{ "defog_min_diff",     iq_proc_cmd_set_defog_min_diff,         "set iq defog_min_diff, param1 is iq_id, param2 is min_diff."},
	{ "dbcs_mode",          iq_proc_cmd_set_dbcs_mode,              "set iq dbcs mode, param1 is iq_id, param2 is mode."},
	{ "dbcs_y",             iq_proc_cmd_set_dbcs_y_wt,              "set iq dbcs y wt, param1 is iq_id, param2~17 is y_wt."},
	{ "dbcs_c",             iq_proc_cmd_set_dbcs_c_wt,              "set iq dbcs c wt, param1 is iq_id, param2~17 is c_wt."},
	{ "lca_location",       iq_proc_cmd_set_lca_location,           "set iq lca location, param1 is iq_id, param2 is location."},
	{ "edge_ker_size",      iq_proc_cmd_set_lca_edge_ker_size,      "set iq lca edge_ker_size, param1 is iq_id, param2 is edge_ker_size."},
	{ "lca_y_out",          iq_proc_cmd_set_lca_y_out,              "set iq lca y_out, param1 is iq_id, param2 is y_out."},
	{ "lca_c_out",          iq_proc_cmd_set_lca_c_out,              "set iq lca c_out, param1 is iq_id, param2 is c_out."},
	{ "tmnr_ae_en",         iq_proc_cmd_set_tmnr_ae_still_en,       "set iq tmnr ae_en, param1 is iq_id, param2 is tmnr_ae_still_en."},
	{ "tmnr_reset",         iq_proc_cmd_set_residue_reset,          "set iq tmnr residue reset freq, param1 is iq_id, param2 is reset_freq(0~60)."},
	{ "ps_con_eng",         iq_proc_cmd_set_post_sharpen_con_eng,   "set iq post_sharpen con_eng, param1 is iq_id, param2 is con_eng."},
	// test proc item with non-id
	{ "aiisp_aided_test",   iq_proc_cmd_set_aiisp_aided_test,       "set aiisp aided test, param1 is enable(0~1), param2 is clone id, param3 is src_id., NT98539A only."},
	#if 0
	{ "wdr",                iq_proc_cmd_set_wdr,                    "set iq wdr, param1 is dark_th, param2 is dr_th, param3 is dark_compensate"},
	{ "airlight",           iq_proc_cmd_set_airlight,               "set iq airlight, param1 is airlight_th, param2 is airlight_diff_max_ratio"},
	{ "defog",              iq_proc_cmd_set_defog,                  "set iq defog, param1 is cnt_ratio_th1, param2 is cnt_ratio_th2"},
	#endif
};
#define NUM_OF_W_CMD (sizeof(iq_proc_cmd_w_list) / sizeof(IQ_PROC_CMD))

static INT32 iq_proc_command_show(struct seq_file *sfile, void *v)
{
	if (iq_proc_cmd_msg.buf == NULL && iq_proc_r_item == IQ_PROC_R_ITEM_NONE) {
		return -EINVAL;
	}

	if (iq_proc_cmd_msg.buf > 0) {
		seq_printf(sfile, "%s\n", iq_proc_cmd_msg.buf);
		iq_proc_msg_buf_clean();
	}

	if (iq_proc_r_item == IQ_PROC_R_ITEM_BUFFER) {
		iq_msg_show_buffer_size(sfile);
	}

	if (iq_proc_r_item == IQ_PROC_R_ITEM_PARAM) {
		iq_msg_show_param(sfile, iq_proc_id, iq_proc_iso);
	}

	if (iq_proc_r_item == IQ_PROC_R_ITEM_UI_PARAM) {
		iq_msg_show_ui(sfile, iq_proc_id);
	}

	if (iq_proc_r_item == IQ_PROC_R_ITEM_NNSC_PARAM) {
		iq_msg_show_nnsc(sfile, iq_proc_id);
	}

	if (iq_proc_r_item == IQ_PROC_R_ITEM_CFG_DATA) {
		iq_msg_show_cfg_path(sfile);
	}

	if (iq_proc_r_item == IQ_PROC_R_ITEM_LOW_POWER_INFO) {
		iq_msg_show_low_power_info(sfile);
	}

	if (iq_proc_r_item == IQ_PROC_R_ITEM_LOW_POWER_PARAM) {
		iq_msg_show_low_power_param(sfile);
	}

	if (iq_proc_r_item == IQ_PROC_R_ITEM_MANUAL_INFO) {
		iq_msg_show_manual_info(sfile);
	}

	iq_proc_r_item = IQ_PROC_R_ITEM_NONE;

	return 0;
}

static INT32 iq_proc_command_open(struct inode *inode, struct file *file)
{
	//return single_open(file, iq_proc_command_show, PDE_DATA(inode));
	return single_open_size(file, iq_proc_command_show, PDE_DATA(inode), IQ_PROC_DUMP_TOTAL_SIZE * sizeof(u32));
}

static ssize_t iq_proc_command_write(struct file *file, const CHAR __user *buf, size_t size, loff_t *off)
{
	INT32 len = size;
	INT32 ret = -EINVAL;

	CHAR cmd_line[MAX_CMDLINE_LENGTH];
	CHAR *cmdstr = cmd_line;
	const CHAR delimiters[] = {' ', 0x0A, 0x0D, '\0'};
	CHAR *argv[MAX_CMD_ARGUMENTS] = {NULL};
	INT32 argc = 0;
	UINT32 i;
	ISP_MODULE *iq_module = iq_proc_get_mudule_from_file(file);

	// check command length
	if ((len <= 1) || (len > MAX_CMDLINE_LENGTH)) {
		DBG_ERR("command is too short or long!! \r\n");
	} else {
		// copy command string from user space
		if (copy_from_user(cmd_line, buf, len)) {
			;
		} else {
			cmd_line[len-1] = '\0';

			// parse command string
			for (i = 0; i < MAX_CMD_ARGUMENTS; i++) {
				argv[i] = strsep(&cmdstr, delimiters);
				if (argv[i] != NULL) {
					argc++;
				} else {
					break;
				}
			}

			// dispatch command handler
			ret = -EINVAL;

			if (strncmp(argv[0], "r", 2) == 0) {
				for (i = 0; i < NUM_OF_R_CMD; i++) {
					if (strncmp(argv[1], iq_proc_cmd_r_list[i].cmd, MAX_CMD_LENGTH) == 0) {
						#if MUTEX_ENABLE
						down(&mutex);
						#endif
						ret = iq_proc_cmd_r_list[i].execute(iq_module, argc, argv);
						#if MUTEX_ENABLE
						up(&mutex);
						#endif
						break;
					}
				}

				if (i >= NUM_OF_R_CMD) {
					DBG_ERR("[IQ_ERR]: => ");
					for (i = 0; i < argc; i++) {
						DBG_ERR("%s ", argv[i]);
					}
					DBG_ERR("is not in r_cmd_list!! \r\n");
				}
			} else if (strncmp(argv[0], "w", 2) == 0) {
				for (i = 0; i < NUM_OF_W_CMD; i++) {
					if (strncmp(argv[1], iq_proc_cmd_w_list[i].cmd, MAX_CMD_LENGTH) == 0) {
						#if MUTEX_ENABLE
						down(&mutex);
						#endif
						ret = iq_proc_cmd_w_list[i].execute(iq_module, argc, argv);
						#if MUTEX_ENABLE
						up(&mutex);
						#endif
						break;
					}
				}

				if (i >= NUM_OF_W_CMD) {
					DBG_ERR("[IQ_ERR]: =>");
					for (i = 0; i < argc; i++) {
						DBG_ERR("%s ", argv[i]);
					}
					DBG_ERR("is not in w_cmd_list!! \r\n");
				}
			} else {
				DBG_ERR("[IQ_ERR]: =>");
				for (i = 0; i < argc; i++) {
					DBG_ERR("%s ", argv[i]);
				}
				DBG_ERR("is not legal command!! \r\n");
			}
		}
	}

	if (ret < 0) {
		DBG_ERR("[IQ_ERR]: fail to execute: ");
		for (i = 0; i < argc; i++) {
			DBG_ERR("%s ", argv[i]);
		}
		DBG_ERR("\r\n");
	}

	return size;
}

static const struct proc_ops iq_proc_command_ops = {
	.proc_open    = iq_proc_command_open,
	.proc_read    = seq_read,
	.proc_write   = iq_proc_command_write,
	.proc_lseek   = seq_lseek,
	.proc_release = single_release,
};

//=============================================================================
// proc "dbglevel" file operation functions
//=============================================================================
unsigned int iq_debug_level = 3;

static INT32 iq_proc_dbglevel_show(struct seq_file *sfile, void *v)
{
	seq_printf(sfile, "0: fatal; 1: err; 2: wrn; 3: msg; 4: ind; 5: func \n");
	seq_printf(sfile, "iq_debug_level = %d \n", iq_debug_level);

	return 0;
}

static INT32 iq_proc_dbglevel_open(struct inode *inode, struct file *file)
{
	return single_open(file, iq_proc_dbglevel_show, PDE_DATA(inode));
}

static ssize_t iq_proc_dbglevel_write(struct file *file, const CHAR __user *buf, size_t size, loff_t *off)
{
	INT32 len = size;
	INT32 ret = -EINVAL;
	CHAR cmd_line[MAX_CMDLINE_LENGTH];
	CHAR *cmdstr = cmd_line;
	const CHAR delimiters[] = {' ', 0x0A, 0x0D, '\0'};
	CHAR *argv[MAX_CMD_ARGUMENTS] = {NULL};
	INT32 argc = 0;
	UINT32 i;

	iq_proc_msg_buf_clean();

	// check command length
	if ((len <= 1) || (len > MAX_CMDLINE_LENGTH)) {
		DBG_ERR("command is too short or long \n");
	} else {
		// copy command string from user space
		if (copy_from_user(cmd_line, buf, len)) {
			;
		} else {
			cmd_line[len-1] = '\0';

			// parse command string
			for (i = 0; i < MAX_CMD_ARGUMENTS; i++) {
				argv[i] = strsep(&cmdstr, delimiters);
				if (argv[i] != NULL) {
					argc++;
				} else {
					break;
				}
			}

			// dispatch command handler
			ret = -EINVAL;

			if (argc != 1) {
				iq_proc_cmd_printf("wrong argument:%d", argc);
				return -EINVAL;
			}

			iq_debug_level = simple_strtoul(argv[0], NULL, 0);
			iq_proc_cmd_printf("set dbglevel %d \n", iq_debug_level);
		}
	}

	return size;
}

static const struct proc_ops iq_proc_dbglevel_ops = {
	.proc_open    = iq_proc_dbglevel_open,
	.proc_read    = seq_read,
	.proc_write   = iq_proc_dbglevel_write,
	.proc_lseek   = seq_lseek,
	.proc_release = single_release
};

//=============================================================================
// proc "help" file operation functions
//=============================================================================
static INT32 iq_proc_help_show(struct seq_file *sfile, void *v)
{
	UINT32 loop;

	seq_printf(sfile, "1. 'cat /proc/hdal/vendor/iq/info' will show all the iq info \r\n");
	seq_printf(sfile, "2. 'echo r/w xxx > /proc/hdal/vendor/iq/cmd' can input command for some debug purpose \r\n");
	seq_printf(sfile, "The currently support input command are below: \r\n");

	seq_printf(sfile, "--------------------------------------------------------------------- \r\n");
	seq_printf(sfile, "  %s\n", "iq");
	seq_printf(sfile, "--------------------------------------------------------------------- \r\n");

	for (loop = 0 ; loop < NUM_OF_R_CMD ; loop++) {
		seq_printf(sfile, "r %15s : %s\r\n", iq_proc_cmd_r_list[loop].cmd, iq_proc_cmd_r_list[loop].text);
	}

	for (loop = 0 ; loop < NUM_OF_W_CMD ; loop++) {
		seq_printf(sfile, "w %15s : %s\r\n", iq_proc_cmd_w_list[loop].cmd, iq_proc_cmd_w_list[loop].text);
	}

	seq_printf(sfile, "------------------------------------------------------------------------ \r\n");
	seq_printf(sfile, " func_en = \r\n");
	seq_printf(sfile, "   | 0x%8x = WDR    | 0x%8x = SHDR   | 0x%8x = DEFOG  |                  \r\n", ISP_FUNC_EN_WDR, ISP_FUNC_EN_SHDR, ISP_FUNC_EN_DEFOG);
	seq_printf(sfile, " dbg_lv = \r\n");
	seq_printf(sfile, "   | 0x%8x = WRN    | 0x%8x = SYNC   | 0x%8x = CFG    | 0x%8x = DTS    | \r\n", IQ_DBG_WRN_MSG, IQ_DBG_SYNC, IQ_DBG_CFG, IQ_DBG_DTS);
	seq_printf(sfile, "   | 0x%8x = VIG    | 0x%8x = GAMMA  | 0x%8x = CCM    | 0x%8x = TABLE  | \r\n", IQ_DBG_VIG, IQ_DBG_GAMMA, IQ_DBG_CCM, IQ_DBG_TABLE);
	seq_printf(sfile, "   | 0x%8x = SIE    | 0x%8x = IFE    | 0x%8x = PRE    | 0x%8x = IPE    | \r\n", IQ_DBG_P_SIE, IQ_DBG_P_IFE, IQ_DBG_P_PRE, IQ_DBG_P_IPE);
	seq_printf(sfile, "   | 0x%8x = IME    | 0x%8x = ENC    | 0x%8x = AIISP  |                  \r\n", IQ_DBG_P_IME, IQ_DBG_P_ENC, IQ_DBG_P_AIISP);
	seq_printf(sfile, "   | 0x%8x = WDR    | 0x%8x = DEFOG  | 0x%8x = SHDR   | 0x%8x = RGBIR  | \r\n", IQ_DBG_A_WDR, IQ_DBG_A_DEFOG, IQ_DBG_A_SHDR, IQ_DBG_A_RGBIR);
	seq_printf(sfile, "   | 0x%8x = ISO    | 0x%8x = EDGE   | 0x%8x = TMNR   | 0x%8x = ECS    | \r\n", IQ_DBG_O_ISO, IQ_DBG_O_EDGE, IQ_DBG_O_TMNR, IQ_DBG_O_ECS);
	seq_printf(sfile, "   | 0x%8x = CAP    | 0x%8x = NNSC   | 0x%8x = FAST   | 0x%8x = SMOOTH | \r\n", IQ_DBG_CAPTURE, IQ_DBG_NNSC, IQ_DBG_FASTBOOT, IQ_DBG_SMOOTH);
	seq_printf(sfile, "   | 0x%8x = PERFOR |                                                    \r\n", IQ_DBG_PERFORMANCE);
	seq_printf(sfile, "------------------------------------------------------------------------ \r\n");
	seq_printf(sfile, " ui_type = \r\n");
	seq_printf(sfile, "   | 0x%8x = NR     | 0x%8x = 3DNR   | 0x%8x = SHARP  |                  \r\n", IQ_UI_ITEM_NR_LV ,IQ_UI_ITEM_3DNR_LV, IQ_UI_ITEM_SHARPNESS_LV);
	seq_printf(sfile, "   | 0x%8x = SAT    | 0x%8x = CONT   | 0x%8x = BRIGHT |                  \r\n", IQ_UI_ITEM_SATURATION_LV, IQ_UI_ITEM_CONTRAST_LV, IQ_UI_ITEM_BRIGHTNESS_LV);
	seq_printf(sfile, "   | 0x%8x = NIGHT  | 0x%8x = YCC    | 0x%8x = OPERA  |                  \r\n", IQ_UI_ITEM_NIGHT_MODE, IQ_UI_ITEM_YCC_FORMAT, IQ_UI_ITEM_OPERATION);
	seq_printf(sfile, "   | 0x%8x = EFFECT | 0x%8x = CCID   | 0x%8x = HUE    |                  \r\n", IQ_UI_ITEM_IMAGEEFFECT, IQ_UI_ITEM_CCID, IQ_UI_ITEM_HUE_SHIFT);
	seq_printf(sfile, "   | 0x%8x = TON_LV | 0x%8x = GMA_LV |                                   \r\n", IQ_UI_ITEM_TONE_LV, IQ_UI_ITEM_GAMMA_LV);
	seq_printf(sfile, "------------------------------------------------------------------------ \r\n");
	seq_printf(sfile, " nnsc_type = \r\n");
	seq_printf(sfile, "   | 0x%8x = DARK   | 0x%8x = CONT   | 0x%8x = GREEN  | 0x%8x = SKIN   | \r\n", IQ_NNSC_ITEM_DARK_ENH_RATIO, IQ_NNSC_ITEM_CONTRAST_ENH_RATIO, IQ_NNSC_ITEM_GREEN_ENH_RATIO, IQ_NNSC_ITEM_SKIN_ENH_RATIO);
	seq_printf(sfile, "------------------------------------------------------------------------ \r\n");
	seq_printf(sfile, "Example: \r\n");
	seq_printf(sfile, "echo r param 0 0 > /proc/hdal/vendor/iq/cmd;cat /proc/hdal/vendor/iq/cmd \r\n");
	seq_printf(sfile, "echo r ui_param 0 > /proc/hdal/vendor/iq/cmd;cat /proc/hdal/vendor/iq/cmd \r\n");
	seq_printf(sfile, "echo r low_power_info > /proc/hdal/vendor/iq/cmd;cat /proc/hdal/vendor/iq/cmd \r\n");
	seq_printf(sfile, "echo r manual_info > /proc/hdal/vendor/iq/cmd;cat /proc/hdal/vendor/iq/cmd \r\n");
	seq_printf(sfile, "echo w dbg 0 0x100 > /proc/hdal/vendor/iq/cmd \r\n");
	seq_printf(sfile, "echo w reload_cfg 0 /mnt/app/isp/isp_os04a10_0.cfg > /proc/hdal/vendor/iq/cmd \r\n");
	return 0;
}

static INT32 iq_proc_help_open(struct inode *inode, struct file *file)
{
	return single_open(file, iq_proc_help_show, PDE_DATA(inode));
}

static const struct proc_ops iq_proc_help_ops = {
	.proc_open    = iq_proc_help_open,
	.proc_read    = seq_read,
	.proc_lseek   = seq_lseek,
	.proc_release = single_release,
};

//=============================================================================
// extern functions
//=============================================================================

INT32 iq_proc_create(void)
{
	INT32 ret = 0;
	struct proc_dir_entry *root = NULL, *pentry = NULL;

	// initialize synchronization mechanism
	#if MUTEX_ENABLE
	sema_init(&mutex, 1);
	#endif

	root = proc_mkdir("hdal/vendor/iq", NULL);

	if (root == NULL) {
		DBG_ERR("fail to create IQ proc root!! \r\n");
		iq_proc_remove();
		return -EINVAL;
	}
	proc_root = root;

	// create "info" entry
	pentry = proc_create_data("info", S_IRUGO | S_IXUGO, root, &iq_proc_info_ops, NULL);
	if (pentry == NULL) {
		DBG_ERR("fail to create proc info!! \r\n");
		iq_proc_remove();
		return -EINVAL;
	}
	proc_info = pentry;

	// create "command" entry
	pentry = proc_create_data("cmd", S_IRUGO | S_IXUGO, root, &iq_proc_command_ops, NULL);
	if (pentry == NULL) {
		DBG_ERR("fail to create proc cmd!! \r\n");
		iq_proc_remove();
		return -EINVAL;
	}
	proc_command = pentry;

	// create "dbglevel" entry
	pentry = proc_create_data("dbglevel", S_IRUGO | S_IXUGO, root, &iq_proc_dbglevel_ops, NULL);
	if (pentry == NULL) {
		DBG_ERR("fail to create proc dbglevel \n");
		iq_proc_remove();
		return -EINVAL;
	}
	proc_dbglevel = pentry;

	// create "help" entry
	pentry = proc_create_data("help", S_IRUGO | S_IXUGO, root, &iq_proc_help_ops, NULL);
	if (pentry == NULL) {
		DBG_ERR("fail to create proc help!! \r\n");
		iq_proc_remove();
		return -EINVAL;
	}
	proc_help = pentry;

	// allocate memory for massage buffer
	ret = iq_proc_msg_buf_alloc();
	if (ret < 0) {
		iq_proc_remove();
	}

	return ret;
}

void iq_proc_remove(void)
{
	if (proc_root == NULL) {
		return;
	}

	// remove "info"
	if (proc_info) {
		proc_remove(proc_info);
	}
	// remove "cmd"
	if (proc_info) {
		proc_remove(proc_command);
	}
	// remove "dbglevel"
	if (proc_dbglevel) {
		proc_remove(proc_dbglevel);
	}
	// remove "help"
	if (proc_help) {
		proc_remove(proc_help);
	}

	// remove root entry
	proc_remove(proc_root);

	// free message buffer
	iq_proc_msg_buf_free();
}

