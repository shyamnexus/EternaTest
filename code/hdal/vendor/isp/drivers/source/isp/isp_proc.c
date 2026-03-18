#include <linux/slab.h>
#include <linux/seq_file.h>

#include "isp_api_int.h"
#include "isp_dbg.h"
#include "isp_main.h"
#include "isp_msg.h"
#include "isp_proc.h"
#include "isp_uti.h"

//=============================================================================
// function declaration
//=============================================================================
INT32 isp_proc_init(ISP_DRV_INFO *pdrv_info);
void isp_proc_remove(ISP_DRV_INFO *pdrv_info);

#define ISP_PROC_MSG_BUFSIZE 2048

static ISP_PROC_MSG_BUF isp_proc_msg;
static ISP_PROC_R_ITEM isp_proc_r_item = ISP_PROC_R_ITEM_NONE;
static ISP_ID isp_proc_id;
static ISP_PROC_R_PARAM isp_proc_r_param;
struct proc_dir_entry *vendor_root = NULL;

//=============================================================================
// interanl functions
//=============================================================================
static inline INT32 isp_proc_alloc_msgbuf(void)
{
	isp_proc_msg.buf = kzalloc(ISP_PROC_MSG_BUFSIZE, GFP_KERNEL);

	if (isp_proc_msg.buf == NULL) {
		DBG_ERR("fail to allocate message buffer \n");
		return -ENOMEM;
	}

	isp_proc_msg.size = ISP_PROC_MSG_BUFSIZE;
	isp_proc_msg.count = 0;

	return 0;
}

static inline void isp_proc_free_msgbuf(void)
{
	if (isp_proc_msg.buf) {
		kfree(isp_proc_msg.buf);
		isp_proc_msg.buf = NULL;
	}
}

static inline void isp_proc_clean_msgbuf(void)
{
	isp_proc_msg.buf[0] = '\0';
	isp_proc_msg.count = 0;
}

static INT32 isp_proc_cmd_printf(const CHAR *f, ...)
{
	va_list args;
	INT32 len;

	if (isp_proc_msg.count < isp_proc_msg.size) {
		va_start(args, f);
		len = vsnprintf(isp_proc_msg.buf + isp_proc_msg.count, isp_proc_msg.size - isp_proc_msg.count, f, args);
		va_end(args);

		if (isp_proc_msg.count + len < isp_proc_msg.size) {
			isp_proc_msg.count += len;
			return 0;
		}
	}

	isp_proc_msg.count = isp_proc_msg.size;

	return -1;
}

static INT32 isp_proc_cmd_get_param(ISP_DRV_INFO *pdrv, INT32 argc, CHAR **argv)
{
	if (argc < 4) {
		isp_proc_cmd_printf("wrong argument:%d", argc);
		return -EINVAL;
	}

	isp_proc_r_item = ISP_PROC_R_ITEM_PARAM;
	isp_proc_id = simple_strtoul(argv[2], NULL, 0);
	isp_proc_r_param = simple_strtoul(argv[3], NULL, 0);

	return 0;
}

static inline ISP_DRV_INFO *isp_proc_get_drv_info(struct file *file)
{
	// get driver info from file.
	return (ISP_DRV_INFO *)((struct seq_file *)file->private_data)->private;
}

//=============================================================================
// proc "info" file operation functions
//=============================================================================
static INT32 isp_proc_info_show(struct seq_file *sfile, void *v)
{
	isp_msg_show_info(sfile);

	return 0;
}

static INT32 isp_proc_info_open(struct inode *inode, struct file *file)
{
	return single_open(file, isp_proc_info_show, PDE_DATA(inode));
}

static const struct proc_ops isp_proc_info_ops = {
	.proc_open    = isp_proc_info_open,
	.proc_read    = seq_read,
	.proc_lseek   = seq_lseek,
	.proc_release = single_release
};

static INT32 isp_proc_cmd_set_dbg(ISP_DRV_INFO *pdrv, INT32 argc, CHAR **argv)
{
	ISP_ID id;
	UINT32 cmd;

	if (argc < 4) {
		isp_proc_cmd_printf("wrong argument:%d", argc);
		return -EINVAL;
	}

	id = simple_strtoul(argv[2], NULL, 0);
	cmd = simple_strtoul(argv[3], NULL, 0);

	isp_proc_cmd_printf("set isp(%d) dbg level(0x%X) \n", (int)id, (unsigned int)cmd);
	isp_dbg_set_dbg_mode(id, cmd);

	return 0;
}

static INT32 isp_proc_cmd_set_bypass(ISP_DRV_INFO *pdrv, INT32 argc, CHAR **argv)
{
	ISP_ID id;
	UINT32 cmd;

	if (argc < 4) {
		isp_proc_cmd_printf("wrong argument:%d", argc);
		return -EINVAL;
	}

	id = simple_strtoul(argv[2], NULL, 0);
	cmd = simple_strtoul(argv[3], NULL, 0);

	isp_proc_cmd_printf("set isp(%d) bypass eng(0x%X) \n", (int)id, (unsigned int)cmd);
	isp_dbg_set_bypass_eng(id, cmd);

	return 0;
}

static INT32 isp_proc_cmd_set_ioc_control(ISP_DRV_INFO *pdrv, INT32 argc, CHAR **argv)
{
	UINT32 cmd;

	if (argc < 3) {
		isp_proc_cmd_printf("wrong argument:%d", argc);
		return -EINVAL;
	}

	cmd = simple_strtoul(argv[2], NULL, 0);

	isp_proc_cmd_printf("set isp ioc control(0x%X) \n", (unsigned int)cmd);
	isp_dbg_set_ioc_control(cmd);

	return 0;
}

static INT32 isp_proc_cmd_set_sensor_dir(ISP_DRV_INFO *pdrv, INT32 argc, CHAR **argv)
{
	ISP_ID id;
	ISP_SENSOR_DIRECTION sensor_dir = {0};

	if (argc < 4) {
		isp_proc_cmd_printf("wrong argument:%d", argc);
		return -EINVAL;
	}

	id = simple_strtoul(argv[2], NULL, 0);
	sensor_dir.mirror = simple_strtoul(argv[3], NULL, 0);
	sensor_dir.flip = simple_strtoul(argv[4], NULL, 0);

	isp_api_set_direction(id, &sensor_dir);

	return 0;
}

static INT32 isp_proc_cmd_set_sensor_sleep(ISP_DRV_INFO *pdrv, INT32 argc, CHAR **argv)
{
	ISP_ID id;

	if (argc < 2) {
		isp_proc_cmd_printf("wrong argument:%d", argc);
		return -EINVAL;
	}

	id = simple_strtoul(argv[2], NULL, 0);

	isp_api_set_sensor_sleep(id);

	return 0;
}

static INT32 isp_proc_cmd_set_sensor_wakeup(ISP_DRV_INFO *pdrv, INT32 argc, CHAR **argv)
{
	ISP_ID id;

	if (argc < 2) {
		isp_proc_cmd_printf("wrong argument:%d", argc);
		return -EINVAL;
	}

	id = simple_strtoul(argv[2], NULL, 0);

	isp_api_set_sensor_wakeup(id);

	return 0;
}

static INT32 isp_proc_cmd_set_emu_enable(ISP_DRV_INFO *pdrv, INT32 argc, CHAR **argv)
{
	BOOL enable;

	if (argc < 2) {
		isp_proc_cmd_printf("wrong argument:%d", argc);
		return -EINVAL;
	}

	enable = simple_strtoul(argv[2], NULL, 0);

	isp_api_set_emu_enable(enable);

	printk("Emulator = %d \r\n", enable);

	return 0;
}

static INT32 isp_proc_cmd_set_ipp_indep(ISP_DRV_INFO *pdrv, INT32 argc, CHAR **argv)
{
	ISP_ID id;
	BOOL enable;

	if (argc < 2) {
		isp_proc_cmd_printf("wrong argument:%d", argc);
		return -EINVAL;
	}

	id = simple_strtoul(argv[2], NULL, 0);
	enable = simple_strtoul(argv[3], NULL, 0);

	isp_set_ipp_indep(id, enable);

	printk("isp_sync_ipp_indep[%d] = %d \r\n", id, enable);

	return 0;
}

static INT32 isp_proc_cmd_set_pwm_en(ISP_DRV_INFO *pdrv, INT32 argc, CHAR **argv)
{
	ER rt = E_OK;
	UINT32 pwm_id, period, duty_cycle, polarity;

	if (argc < 5) {
		isp_proc_cmd_printf("wrong argument:%d", argc);
		return -EINVAL;
	}

	pwm_id = simple_strtoul(argv[2], NULL, 0);
	period = simple_strtoul(argv[3], NULL, 0);
	duty_cycle = simple_strtoul(argv[4], NULL, 0);
	polarity = simple_strtoul(argv[5], NULL, 0);

	rt = isp_uti_enable_pwm(pwm_id, period, duty_cycle, polarity);

	printk("rt = %d, id = %d, period = %d, duty_cycle = %d, polarity = %d \r\n", rt, pwm_id, period, duty_cycle, polarity);

	return 0;
}

static INT32 isp_proc_cmd_set_pwm_param(ISP_DRV_INFO *pdrv, INT32 argc, CHAR **argv)
{
	ER rt = E_OK;
	UINT32 pwm_id, period, duty_cycle, polarity;

	if (argc < 5) {
		isp_proc_cmd_printf("wrong argument:%d", argc);
		return -EINVAL;
	}

	pwm_id = simple_strtoul(argv[2], NULL, 0);
	period = simple_strtoul(argv[3], NULL, 0);
	duty_cycle = simple_strtoul(argv[4], NULL, 0);
	polarity = simple_strtoul(argv[5], NULL, 0);

	rt = isp_uti_set_pwm(pwm_id, period, duty_cycle, polarity);

	printk("rt = %d, id = %d, period = %d, duty_cycle = %d, polarity = %d \r\n", rt, pwm_id, period, duty_cycle, polarity);

	return 0;
}

static INT32 isp_proc_cmd_set_pwm_dis(ISP_DRV_INFO *pdrv, INT32 argc, CHAR **argv)
{
	ER rt = E_OK;
	UINT32 pwm_id;

	if (argc < 2) {
		isp_proc_cmd_printf("wrong argument:%d", argc);
		return -EINVAL;
	}

	pwm_id = simple_strtoul(argv[2], NULL, 0);

	rt = isp_uti_disable_pwm(pwm_id);

	printk("rt = %d, id = %d \r\n", rt, pwm_id);

	return 0;
}

static INT32 isp_proc_cmd_set_low_power_lv(ISP_DRV_INFO *pdrv, INT32 argc, CHAR **argv)
{
	UINT32 lv;

	if (argc < 2) {
		isp_proc_cmd_printf("wrong argument:%d", argc);
		return -EINVAL;
	}

	lv = simple_strtoul(argv[2], NULL, 0);

	isp_api_set_low_power_lv(lv);

	return 0;
}

static ISP_PROC_CMD isp_proc_r_cmd_list[] = {
	// keyword              function name
	{ "get_param",          isp_proc_cmd_get_param,          "get isp param, param1 is isp_id, param2 is param_sel."},
};
#define NUM_OF_R_CMD (sizeof(isp_proc_r_cmd_list) / sizeof(ISP_PROC_CMD))

static ISP_PROC_CMD isp_proc_w_cmd_list[] = {
	// keyword              function name
	{ "dbg",                isp_proc_cmd_set_dbg,            "set isp dbg level, param1 is isp_id(15 means apply to each id), param2 is dbg_lv."},
	{ "bypass",             isp_proc_cmd_set_bypass,         "set isp bypass eng, param1 is isp_id(15 means apply to each id), param2 is bypass_eng."},
	{ "ioc",                isp_proc_cmd_set_ioc_control,    "set isp ioc control, param1 is ioc_control."},
	{ "dir",                isp_proc_cmd_set_sensor_dir,     "set sensor direction, param1 is isp_id, param2 is mirror, param3 is flip."},
	{ "sleep",              isp_proc_cmd_set_sensor_sleep,   "set sensor sleep, param1 is isp_id."},
	{ "wakeup",             isp_proc_cmd_set_sensor_wakeup,  "set sensor wakeup, param1 is isp_id."},
	{ "emu_en",             isp_proc_cmd_set_emu_enable,     "set emu enable, param1 is isp_id, param2 is enable."},
	{ "ipp_indep",          isp_proc_cmd_set_ipp_indep,      "set ipp_indep enable, param1 is isp_id, param2 is enable."},
	{ "pwm_en",             isp_proc_cmd_set_pwm_en,         "set pwm enable, param1 is pwm_id(0~11), param2 is period, param3 is duty_cycle, param4 is polarity."},
	{ "pwm_set",            isp_proc_cmd_set_pwm_param,      "set pwm param, param1 is pwm_id(0~11), param2 is period, param3 is duty_cycle, param4 is polarity."},
	{ "pwm_dis",            isp_proc_cmd_set_pwm_dis,        "set pwm disable, param1 is pwm_id(0~11)."},
	{ "low_power_lv",       isp_proc_cmd_set_low_power_lv,   "set low power level, param1 is lv(0~5), NT98539A only."},
};
#define NUM_OF_W_CMD (sizeof(isp_proc_w_cmd_list) / sizeof(ISP_PROC_CMD))

static INT32 isp_proc_command_show(struct seq_file *sfile, void *v)
{
	if ((isp_proc_msg.buf == NULL) && (isp_proc_r_item == ISP_PROC_R_ITEM_NONE)) {
		return -EINVAL;
	}

	if (isp_proc_msg.buf > 0) {
		seq_printf(sfile, "%s\n", isp_proc_msg.buf);
		isp_proc_clean_msgbuf();
	}

	if (isp_proc_r_item == ISP_PROC_R_ITEM_PARAM) {
		switch (isp_proc_r_param) {
		case ISP_PROC_R_PARAM_CA:
			isp_msg_show_ca(sfile, isp_proc_id);
			break;

		case ISP_PROC_R_PARAM_CA_ACC_CNT:
			isp_msg_show_ca_acc_cnt(sfile, isp_proc_id);
			break;

		case ISP_PROC_R_PARAM_LA:
			isp_msg_show_la(sfile, isp_proc_id);
			break;

		case ISP_PROC_R_PARAM_VA:
			isp_msg_show_va(sfile, isp_proc_id);
			break;

		case ISP_PROC_R_PARAM_HISTO:
			isp_msg_show_histo(sfile, isp_proc_id);
			break;

		default:
			break;
		}
	}
	isp_proc_r_item = ISP_PROC_R_ITEM_NONE;
	return 0;
}

static INT32 isp_proc_command_open(struct inode *inode, struct file *file)
{
	//return single_open(file, isp_proc_command_show, PDE_DATA(inode));
	return single_open_size(file, isp_proc_command_show, PDE_DATA(inode), 4096 * sizeof(u32));
}

static ssize_t isp_proc_command_write(struct file *file, const CHAR __user *buf, size_t size, loff_t *off)
{
	INT32 len = size;
	INT32 ret = -EINVAL;
	ISP_DRV_INFO *pdrv_info = isp_proc_get_drv_info(file);
	ISP_DEV_INFO *pdev_info = &pdrv_info->dev_info;
	CHAR cmd_line[MAX_CMDLINE_LENGTH];
	CHAR *cmdstr = cmd_line;
	const CHAR delimiters[] = {' ', 0x0A, 0x0D, '\0'};
	CHAR *argv[MAX_CMD_ARGUMENTS] = {NULL};
	INT32 argc = 0;
	UINT32 i;

	isp_proc_clean_msgbuf();

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

			if (strncmp(argv[0], "r", 2) == 0) {
				for (i = 0; i < NUM_OF_R_CMD; i++) {
					if (strncmp(argv[1], isp_proc_r_cmd_list[i].cmd, MAX_CMD_LENGTH) == 0) {
						down(&pdev_info->proc_mutex);
						ret = isp_proc_r_cmd_list[i].execute(pdrv_info, argc, argv);
						up(&pdev_info->proc_mutex);
						break;
					}
				}

				if (i >= NUM_OF_R_CMD) {
					DBG_ERR("[ISP_ERR]: => ");
					for (i = 0; i < argc; i++) {
						DBG_ERR("%s ", argv[i]);
					}
					DBG_ERR("is not in r_cmd_list \n");
				}
			} else if (strncmp(argv[0], "w", 2) == 0) {
				for (i = 0; i < NUM_OF_W_CMD; i++) {
					if (strncmp(argv[1], isp_proc_w_cmd_list[i].cmd, MAX_CMD_LENGTH) == 0) {
						down(&pdev_info->proc_mutex);
						ret = isp_proc_w_cmd_list[i].execute(pdrv_info, argc, argv);
						up(&pdev_info->proc_mutex);
						break;
					}
				}

				if (i >= NUM_OF_W_CMD) {
					DBG_ERR("[ISP_ERR]: =>");
					for (i = 0; i < argc; i++) {
						DBG_ERR("%s ", argv[i]);
					}
					DBG_ERR("is not in w_cmd_list \n");
				}
			} else {
				DBG_ERR("[ISP_ERR]: =>");
				for (i = 0; i < argc; i++) {
					DBG_ERR("%s ", argv[i]);
				}
				DBG_ERR("is not legal command \n");
			}
		}
	}

	if (ret < 0) {
		DBG_ERR("[ISP_ERR]: fail to execute: ");
		for (i = 0; i < argc; i++) {
			DBG_ERR("%s ", argv[i]);
		}
		DBG_ERR("\n");
	}

	return size;
}

static const struct proc_ops isp_proc_command_ops = {
	.proc_open    = isp_proc_command_open,
	.proc_read    = seq_read,
	.proc_write   = isp_proc_command_write,
	.proc_lseek   = seq_lseek,
	.proc_release = single_release,
};

//=============================================================================
// proc "dbglevel" file operation functions
//=============================================================================
unsigned int isp_debug_level = 3;

static INT32 isp_proc_dbglevel_show(struct seq_file *sfile, void *v)
{
	seq_printf(sfile, "0: fatal; 1: err; 2: wrn; 3: msg; 4: ind; 5: func \n");
	seq_printf(sfile, "isp_debug_level = %d \n", isp_debug_level);

	return 0;
}

static INT32 isp_proc_dbglevel_open(struct inode *inode, struct file *file)
{
	return single_open(file, isp_proc_dbglevel_show, PDE_DATA(inode));
}

static ssize_t isp_proc_dbglevel_write(struct file *file, const CHAR __user *buf, size_t size, loff_t *off)
{
	INT32 len = size;
	INT32 ret = -EINVAL;
	CHAR cmd_line[MAX_CMDLINE_LENGTH];
	CHAR *cmdstr = cmd_line;
	const CHAR delimiters[] = {' ', 0x0A, 0x0D, '\0'};
	CHAR *argv[MAX_CMD_ARGUMENTS] = {NULL};
	INT32 argc = 0;
	UINT32 i;

	isp_proc_clean_msgbuf();

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
				isp_proc_cmd_printf("wrong argument:%d", argc);
				return -EINVAL;
			}

			isp_debug_level = simple_strtoul(argv[0], NULL, 0);
			isp_proc_cmd_printf("set dbglevel %d \n", isp_debug_level);
		}
	}

	return size;
}

static const struct proc_ops isp_proc_dbglevel_ops = {
	.proc_open    = isp_proc_dbglevel_open,
	.proc_read    = seq_read,
	.proc_write   = isp_proc_dbglevel_write,
	.proc_lseek   = seq_lseek,
	.proc_release = single_release
};

//=============================================================================
// proc "help" file operation functions
//=============================================================================
static INT32 isp_proc_help_show(struct seq_file *sfile, void *v)
{
	UINT32 loop;

	seq_printf(sfile, "1. 'cat /proc/hdal/vendor/isp/info' will show all the isp info\r\n");
	seq_printf(sfile, "2. 'echo r/w xxx > /proc/hdal/vendor/isp/cmd' can input command for some debug purpose\r\n");
	seq_printf(sfile, "The currently support input command are below:\r\n");

	seq_printf(sfile, "---------------------------------------------------------------------\r\n");
	seq_printf(sfile, "  %s\n", "isp");
	seq_printf(sfile, "---------------------------------------------------------------------\r\n");

	for (loop = 0 ; loop < NUM_OF_R_CMD ; loop++) {
		seq_printf(sfile, "r %15s : %s\r\n", isp_proc_r_cmd_list[loop].cmd, isp_proc_r_cmd_list[loop].text);
	}

	for (loop = 0 ; loop < NUM_OF_W_CMD ; loop++) {
		seq_printf(sfile, "w %15s : %s\r\n", isp_proc_w_cmd_list[loop].cmd, isp_proc_w_cmd_list[loop].text);
	}

	seq_printf(sfile, "---------------------------------------------------------------------\r\n");
	seq_printf(sfile, " param_sel = \r\n");
	seq_printf(sfile, " | 0x%8X = CA           | 0x%8X = CA_ACC_CNT   | 0x%8X = LA           | 0x%8X = VA           | \r\n", (unsigned int)ISP_PROC_R_PARAM_CA, (unsigned int)ISP_PROC_R_PARAM_CA_ACC_CNT, (unsigned int)ISP_PROC_R_PARAM_LA, (unsigned int)ISP_PROC_R_PARAM_VA);
	seq_printf(sfile, " | 0x%8X = HISTO        | \r\n", (unsigned int)ISP_PROC_R_PARAM_HISTO);
	seq_printf(sfile, "---------------------------------------------------------------------\r\n");
	seq_printf(sfile, " dbg_lv = \r\n");
	seq_printf(sfile, " | 0x%8X = AE STATUS    | 0x%8X = TOTAL GAIN   | 0x%8X = ISP GAIN     | 0x%8X = D GAIN       | \r\n", ISP_DBG_SYNC_AE_STATUS, ISP_DBG_SYNC_TOTAL_GAIN, ISP_DBG_SYNC_ISP_GAIN, ISP_DBG_SYNC_D_GAIN);
	seq_printf(sfile, " | 0x%8X = LV           | 0x%8X = CA_LA_ENABLE | 0x%8X = EV_RATIO     | 0x%8X = TM_RATIO     | \r\n", ISP_DBG_SYNC_LV, ISP_DBG_SYNC_CA_LA_ENABLE, ISP_DBG_SYNC_EV_RATIO, ISP_DBG_SYNC_TM_RATIO);
	seq_printf(sfile, " | 0x%8X = C GAIN       | 0x%8X = FINAL C GAIN | 0x%8X = CT           | \r\n", ISP_DBG_SYNC_C_GAIN, ISP_DBG_SYNC_FINAL_C_GAIN, ISP_DBG_SYNC_CT);
	seq_printf(sfile, " | 0x%8X = CA_TH        | 0x%8X = CA ROI       | 0x%8X = LA ROI       | 0x%8X = VA ROI       | \r\n", ISP_DBG_SYNC_CA_TH, ISP_DBG_SYNC_CA_ROI, ISP_DBG_SYNC_LA_ROI, ISP_DBG_SYNC_VA_ROI);
	seq_printf(sfile, " | 0x%8X = SENSOR EXPT  | 0x%8X = SENSOR GAIN  | 0x%8X = SENSOR REG.  | 0x%8X = CAPTURE      | \r\n", ISP_DBG_SENSOR_EXPT, ISP_DBG_SENSOR_GAIN, ISP_DBG_SENSOR_REG, ISP_DBG_SYNC_CAPTURE);
	seq_printf(sfile, " | 0x%8X = SIE CB       | 0x%8X = IPP CB       | 0x%8X = ENC CB       | \r\n", ISP_DBG_SIE_CB, ISP_DBG_IPP_CB, ISP_DBG_ENC_CB);
	seq_printf(sfile, " | 0x%8X = SIE SET      | 0x%8X = IPP SET      | 0x%8X = ENC SET      | 0x%8X = AIISP SET    | \r\n", ISP_DBG_SIE_SET, ISP_DBG_IPP_SET, ISP_DBG_ENC_SET, ISP_DBG_AIISP_SET);
	seq_printf(sfile, " | 0x%8X = ERR MSG      | 0x%8X = WRN MSG      | \r\n", ISP_DBG_ERR_MSG, ISP_DBG_WRN_MSG);
	seq_printf(sfile, "---------------------------------------------------------------------\r\n");
	seq_printf(sfile, " bypass_eng = \r\n");
	seq_printf(sfile, " | 0x%8X = SIE_ROI      | 0x%8X = SIE_PARAM    | 0x%8X = IFE_PARAM    | 0x%8X = PRE_PARAM    | \r\n", ISP_BYPASS_SIE_ROI, ISP_BYPASS_SIE_PARAM, ISP_BYPASS_IFE_PARAM, ISP_BYPASS_PRE_PARAM);
	seq_printf(sfile, " | 0x%8X = IPE_PARAM    | 0x%8X = IME_PARAM    | 0x%8X = ENC_PARAM    | 0x%8X = AIISP_PARAM  | \r\n", ISP_BYPASS_IPE_PARAM, ISP_BYPASS_IME_PARAM, ISP_BYPASS_ENC_PARAM, ISP_BYPASS_AIISP_PARAM);
	seq_printf(sfile, " | 0x%8X = IFE_VIG_CENT | 0x%8X = IFE_VA_WIN_S | 0x%8X = IPE_VA_WIN_S | 0x%8X = IME_VA_WIN_S | \r\n", ISP_BYPASS_IFE_VIG_CENT, ISP_BYPASS_IFE_VA_WIN_SIZE, ISP_BYPASS_IPE_VA_WIN_SIZE, ISP_BYPASS_IME_VA_WIN_SIZE);
	seq_printf(sfile, " | 0x%8X = IME_LCA_DBG  | \r\n", ISP_BYPASS_IME_LCA_DBG_X_POS);
	seq_printf(sfile, " | 0x%8X = SEN_REG      | 0x%8X = SEN_EXPT     | 0x%8X = SEN_GAIN     | 0x%8X = SEN_DIR      | \r\n", ISP_BYPASS_SEN_REG, ISP_BYPASS_SEN_EXPT, ISP_BYPASS_SEN_GAIN, ISP_BYPASS_SEN_DIR);
	seq_printf(sfile, " | 0x%8X = SEN_SLEEP    | 0x%8X = SEN_WAKEUP   | \r\n", ISP_BYPASS_SEN_SLEEP, ISP_BYPASS_SEN_WAKEUP);
	seq_printf(sfile, "---------------------------------------------------------------------\r\n");
	seq_printf(sfile, " ioc_control = \r\n");
	seq_printf(sfile, " | 0x%8X = bypass COMMON| 0x%8X = bypass VD    | \r\n", ISP_IOC_BYPASS_COMMON, ISP_IOC_BYPASS_VD);
	seq_printf(sfile, " | 0x%8X = bypass AE    | 0x%8X = bypass AF    | 0x%8X = bypass AWB   | 0x%8X = bypass IQ   | \r\n", ISP_IOC_BYPASS_AE, ISP_IOC_BYPASS_AF, ISP_IOC_BYPASS_AWB, ISP_IOC_BYPASS_IQ);
	seq_printf(sfile, " | 0x%8X = print COMMON | 0x%8X = print VD     | \r\n", ISP_IOC_PRINT_COMMON, ISP_IOC_PRINT_VD);
	seq_printf(sfile, " | 0x%8X = print AE     | 0x%8X = print AF     | 0x%8X = print AWB    | 0x%8X = print IQ    | \r\n", ISP_IOC_PRINT_AE, ISP_IOC_PRINT_AF, ISP_IOC_PRINT_AWB, ISP_IOC_PRINT_IQ);
	seq_printf(sfile, "---------------------------------------------------------------------\r\n");
	seq_printf(sfile, "Example: \r\n");
	seq_printf(sfile, "echo r get_param 0 2 > /proc/hdal/vendor/isp/cmd;cat /proc/hdal/vendor/isp/cmd \r\n");
	seq_printf(sfile, "echo w dbg 15 0x00000000 > /proc/hdal/vendor/isp/cmd \r\n");
	seq_printf(sfile, "echo w dbg 15 0x00030000 > /proc/hdal/vendor/isp/cmd \r\n");
	seq_printf(sfile, "echo w dbg 15 0x00100000 > /proc/hdal/vendor/isp/cmd \r\n");
	seq_printf(sfile, "echo w bypass 15 0x00000000 > /proc/hdal/vendor/isp/cmd \r\n");
	seq_printf(sfile, "echo w bypass 15 0x00001F7F > /proc/hdal/vendor/isp/cmd \r\n");
	seq_printf(sfile, "echo w ioc 0x00000000 > /proc/hdal/vendor/isp/cmd \r\n");
	seq_printf(sfile, "echo w ioc 0x00000001 > /proc/hdal/vendor/isp/cmd \r\n");
	seq_printf(sfile, "echo w ioc 0x00010000 > /proc/hdal/vendor/isp/cmd \r\n");
	seq_printf(sfile, "echo w dir 0 0 0 > /proc/hdal/vendor/isp/cmd \r\n");
	seq_printf(sfile, "echo w low_power_lv 0 > /proc/hdal/vendor/isp/cmd \r\n");

	return 0;
}

static INT32 isp_proc_help_open(struct inode *inode, struct file *file)
{
	return single_open(file, isp_proc_help_show, PDE_DATA(inode));
}

static const struct proc_ops isp_proc_help_ops = {
	.proc_open    = isp_proc_help_open,
	.proc_read    = seq_read,
	.proc_lseek   = seq_lseek,
	.proc_release = single_release
};

//=============================================================================
// extern functions
//=============================================================================
INT32 isp_proc_init(ISP_DRV_INFO *pdrv_info)
{
	INT32 ret = 0;
	struct proc_dir_entry *root = NULL;
	struct proc_dir_entry *pentry = NULL;

	// create root entry
	vendor_root = proc_mkdir("hdal/vendor", NULL);
	if (vendor_root == NULL) {
		DBG_ERR("failed to create hdal/vendor root\r\n");
		ret = -EINVAL;
		return ret;
	}

	// create root entry
	root = proc_mkdir("hdal/vendor/isp", NULL);
	if (root == NULL) {
		DBG_ERR("fail to create proc root \n");
		return -EINVAL;
	}
	pdrv_info->proc_root = root;

	// create "info" entry
	pentry = proc_create_data("info", S_IRUGO | S_IXUGO, root, &isp_proc_info_ops, (void *)pdrv_info);
	if (pentry == NULL) {
		DBG_ERR("fail to create proc info \n");
		isp_proc_remove(pdrv_info);
		return -EINVAL;
	}
	pdrv_info->proc_info = pentry;

	// create "command" entry
	pentry = proc_create_data("cmd", S_IRUGO | S_IXUGO, root, &isp_proc_command_ops, (void *)pdrv_info);
	if (pentry == NULL) {
		DBG_ERR("fail to create proc command \n");
		isp_proc_remove(pdrv_info);
		return -EINVAL;
	}
	pdrv_info->proc_command = pentry;

	// create "dbglevel" entry
	pentry = proc_create_data("dbglevel", S_IRUGO | S_IXUGO, root, &isp_proc_dbglevel_ops, (void *)pdrv_info);
	if (pentry == NULL) {
		DBG_ERR("fail to create proc dbglevel \n");
		isp_proc_remove(pdrv_info);
		return -EINVAL;
	}
	pdrv_info->proc_dbglevel = pentry;

	// create "help" entry
	pentry = proc_create_data("help", S_IRUGO | S_IXUGO, root, &isp_proc_help_ops, (void *)pdrv_info);
	if (pentry == NULL) {
		DBG_ERR("fail to create proc help \n");
		isp_proc_remove(pdrv_info);
		return -EINVAL;
	}
	pdrv_info->proc_help = pentry;

	// allocate memory for massage buffer
	ret = isp_proc_alloc_msgbuf();
	if (ret < 0) {
		isp_proc_remove(pdrv_info);
	}

	return ret;
}

void isp_proc_remove(ISP_DRV_INFO *pdrv_info)
{
	if (pdrv_info->proc_root == NULL) {
		return;
	}

	// remove "info"
	if (pdrv_info->proc_info) {
		proc_remove(pdrv_info->proc_info);
	}

	// remove "command"
	if (pdrv_info->proc_command) {
		proc_remove(pdrv_info->proc_command);
	}

	// remove "dbglevel"
	if (pdrv_info->proc_dbglevel) {
		proc_remove(pdrv_info->proc_dbglevel);
	}

	// remove "help"
	if (pdrv_info->proc_help) {
		proc_remove(pdrv_info->proc_help);
	}

	// remove root entry
	proc_remove(pdrv_info->proc_root);

	// remove vendor root entry
	proc_remove(vendor_root);

	// free message buffer
	isp_proc_free_msgbuf();
}
