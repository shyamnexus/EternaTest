#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>
#include "ctl_vie_proc.h"
#include "ctl_vie_dbg.h"
#include "ctl_vie_main.h"
#include "ctl_vie_api.h"
#include "ctl_vie_debug_int.h"
//============================================================================
// Define
//============================================================================
#define MAX_CMD_LENGTH 100
#define MAX_ARG_NUM     30 //10

//============================================================================
// Declaration
//============================================================================
typedef struct proc_cmd {
	char cmd[MAX_CMD_LENGTH];
	int (*execute)(PMODULE_INFO pdrv, unsigned char argc, char **argv);
} PROC_CMD, *PPROC_CMD;

//============================================================================
// Global variable
//============================================================================
static PCTL_VIE_DRV_INFO pdrv_info_data;

//============================================================================
// Function define
//============================================================================

#if defined(__LINUX)
int nvt_ctl_vie_proc_buffer_to_int(const char __user *buffer, size_t count, int *dst, int dst_cnt)
{
#define PARAM_LENGTH 100

	int var_sum, n, ofs, hex_flag;
	char buf[PARAM_LENGTH], *scan_pos, *end_pos;

	if ((count > PARAM_LENGTH) || (count < 2)) {
		ctl_vie_dbg_err("cmd length(%zd) must <%d and >2\n", count, PARAM_LENGTH);
		return -1;
	}

	memset(buf, 0, PARAM_LENGTH);
	if (copy_from_user(buf, buffer, PARAM_LENGTH)) {
		ctl_vie_dbg_err("convert fail1\n");
		return -1;
	}

	scan_pos = &buf[0];
	end_pos = &buf[(count - 1)];
	ofs = 0;
	var_sum = 0;
	while (scan_pos < end_pos)
	{
		//skip space
		scan_pos += ofs;
		while (scan_pos < end_pos) {
			if (*scan_pos != ' ') {
				break;
			}
			scan_pos += 1;
		}

		if (scan_pos >= end_pos) {
			return var_sum;
		}


		hex_flag = 0;
		if ((int)(end_pos - scan_pos) > 2) {
			if (((*scan_pos == '0') && (*(scan_pos + 1) == 'x')) ||
				((*scan_pos == '0') && (*(scan_pos + 1) == 'X'))) {
				hex_flag = 1;
			}
		}

		if (hex_flag) {
			n = sscanf(scan_pos, "%x%n", &dst[var_sum], &ofs);
		} else {
			n = sscanf(scan_pos, "%d%n", &dst[var_sum], &ofs);
		}

		//EOF
		if (n == -1) {
			return var_sum;
		}

		//scan fail
		if (n == 0) {
			ctl_vie_dbg_err("convert fail2\n");
			return -1;
		}

		//dst buffer full
		var_sum += 1;
		if (var_sum >= dst_cnt) {
			break;
		}
	}
	return var_sum;
}
#endif

static int nvt_ctl_vie_seq_printf(const char *fmtstr, ...)
{
	va_list marker;

	va_start(marker, fmtstr);
	seq_vprintf(pdrv_info_data->info_seq_file, fmtstr, marker);
	va_end(marker);

	return 0;
}

#if 0
#endif
//=============================================================================
// proc "Custom Command" file operation functions
//=============================================================================
static int nvt_ctl_vie_proc_cmd_show(struct seq_file *sfile, void *v)
{
	return 0;
}

static int nvt_ctl_vie_proc_cmd_open(struct inode *inode, struct file *file)
{
	return single_open(file, nvt_ctl_vie_proc_cmd_show, &pdrv_info_data->module_info);
}

static ssize_t nvt_ctl_vie_proc_cmd(struct file *file, const char __user *buf, size_t size, loff_t *off)
{
	int len = size;
	char cmd_line[MAX_CMD_LENGTH];
	char *cmdstr = cmd_line;
	const char delimiters[] = {' ', 0x0A, 0x0D, '\0'};
	char *argv[MAX_ARG_NUM] = {0};
	unsigned char ucargc = 0;

	// check command length
	if (len > (MAX_CMD_LENGTH - 1)) {
		DBG_ERR("Command length is too long!\n");
		goto ERR_OUT;
	}

	// copy command string from user space
	if (copy_from_user(cmd_line, buf, len)) {
		goto ERR_OUT;
	}

	if (len == 0) {
		cmd_line[0] = '\0';
	} else {
		cmd_line[len - 1] = '\0';
	}

	DBG_IND("CMD:%s\n", cmd_line);

	argv[0] = "ctl_vie";
	// parse command string
	for (ucargc = 1; ucargc < MAX_ARG_NUM; ucargc++) {
		argv[ucargc] = strsep(&cmdstr, delimiters);

		if (argv[ucargc] == NULL) {
			break;
		}
	}
	if (ctl_vie_cmd_execute(ucargc, &argv[0]) == 0) {
		return size;
	}

ERR_OUT:
	return -1;
}

static struct proc_ops proc_cmd_fops = {
	.proc_open    	= nvt_ctl_vie_proc_cmd_open,
	.proc_read    	= seq_read,
	.proc_lseek  	= seq_lseek,
	.proc_release 	= single_release,
	.proc_write   	= nvt_ctl_vie_proc_cmd
};

#if 0
#endif
//=============================================================================
// proc "help" file operation functions
//=============================================================================
static int nvt_ctl_vie_proc_help_show(struct seq_file *sfile, void *v)
{
	seq_printf(sfile, "=============================cat====================================\n");
	seq_printf(sfile, "info: for dump kflow debug info\n");
	seq_printf(sfile, "buf: for dump kflow buf io info\n");
	seq_printf(sfile, "=============================Cmd====================================\n");
	seq_printf(sfile, "Press ( echo savemem addr w h > /proc/ctl_vie/cmd ) to dump memory to file sys.\n");
	seq_printf(sfile, "Press ( echo saveraw 0 > /proc/ctl_vie/cmd ) to dump id 0 raw image to file sys.\n");
	seq_printf(sfile, "Press ( echo dbglevel 1 > /proc/ctl_vie/cmd ) to enable kflow error msg, leveltype plz. refer. to CTL_VIE_DBG_LVL\n");
	seq_printf(sfile, "Press ( echo isp_cb_thr 0 1000 > /proc/ctl_vie/cmd ) to dump id 0 isp_cp_fp process time over than 1000 usec\n");
	seq_printf(sfile, "=============================dbgtype====================================\n");
	seq_printf(sfile, "Press ( echo dbgtype id type > /proc/ctl_vie/cmd ) to dump dbg info, id: 0~7, type: plz refer. to CTL_VIE_DBG_MSG_TYPE\n");
	seq_printf(sfile, "      type 0: disable\n");
	seq_printf(sfile, "      type 1: ctl vie info\n");
	seq_printf(sfile, "      type 2: process time\n");
	seq_printf(sfile, "      type 3: buffer io msg lite\n");
	seq_printf(sfile, "      type 4: buffer io msg full\n");
	seq_printf(sfile, "      type 5: problem step record\n");
	seq_printf(sfile, "      type 6: all dbg msg\n");
	seq_printf(sfile, "=====================================================================\n");
	return 0;
}

static int nvt_ctl_vie_proc_help_open(struct inode *inode, struct file *file)
{
	return single_open(file, nvt_ctl_vie_proc_help_show, NULL);
}

static struct proc_ops proc_help_fops = {
	.proc_open   	= nvt_ctl_vie_proc_help_open,
	.proc_release 	= single_release,
	.proc_read   	= seq_read,
	.proc_lseek 	= seq_lseek,
};

#if 0
#endif
//=============================================================================
// proc "info" file operation functions
//=============================================================================
static int nvt_ctl_vie_proc_info_show(struct seq_file *sfile, void *v)
{
	CTL_VIE_ID id = CTL_VIE_ID_1;

	pdrv_info_data->info_seq_file = sfile;
	ctl_vie_dbg_dump_info(nvt_ctl_vie_seq_printf);
	for (id = CTL_VIE_ID_1; id < CTL_VIE_MAX_SUPPORT_ID; id++) {
		if (ctl_vie_get_state_machine(id) == CTL_VIE_STS_RUN) {
			ctl_vie_dbg_dump_ts(id, nvt_ctl_vie_seq_printf);
			goto dump_ts_done;
		}
	}

	// no vie run
	for (id = CTL_VIE_ID_1; id < CTL_VIE_MAX_SUPPORT_ID; id++) {
		ctl_vie_dbg_dump_ts(id, nvt_ctl_vie_seq_printf);
	}

dump_ts_done:
	pdrv_info_data->info_seq_file = NULL;
	return 0;
}

static int nvt_ctl_vie_proc_info_open(struct inode *inode, struct file *file)
{
	return single_open(file, nvt_ctl_vie_proc_info_show, NULL);
}

static struct proc_ops proc_info_fops = {
	.proc_open   	= nvt_ctl_vie_proc_info_open,
	.proc_release 	= single_release,
	.proc_read   	= seq_read,
	.proc_lseek 	= seq_lseek,
};

//=============================================================================
// proc "dbglevel" file operation functions
//=============================================================================

static int nvt_ctl_vie_proc_dbg_lvl_show(struct seq_file *sfile, void *v)
{
	pdrv_info_data->info_seq_file = sfile;
	return 0;
}

static ssize_t nvt_ctl_vie_proc_dbg_lvl_write(struct file *file, const char __user *buffer, size_t count, loff_t *ppos)
{
#define DATA_SIZE 1
	int data[DATA_SIZE] = {0};

	if (nvt_ctl_vie_proc_buffer_to_int(buffer, count, &data[0], DATA_SIZE) != DATA_SIZE) {
		ctl_vie_dbg_err("input parameter error\n");
		return -1;
	}
	ctl_vie_dbg_dump("0: fatal, 1: err, 2: wrn, 3: msg, 4: ind, 5: func\n");
	ctl_vie_dbg_dump("kflow_vie dbglevel = %d\n", data[0]);
	ctl_vie_set_dbg_lvl(data[0]);

#undef DATA_SIZE
	return count;
}

static int nvt_ctl_vie_proc_dbg_lvl_open(struct inode *inode, struct file *file)
{
	return single_open(file, nvt_ctl_vie_proc_dbg_lvl_show, NULL);
}

static struct proc_ops proc_dbg_lvl_fops = {
	.proc_open   	= nvt_ctl_vie_proc_dbg_lvl_open,
	.proc_release 	= single_release,
	.proc_read   	= seq_read,
	.proc_lseek 	= seq_lseek,
    .proc_write   	= nvt_ctl_vie_proc_dbg_lvl_write,
};

#if 0
#endif
//=============================================================================
// proc "buf" file operation functions
//=============================================================================
static int nvt_ctl_vie_proc_buf_show(struct seq_file *sfile, void *v)
{
	UINT32 i;

	for (i = 0; i < CTL_VIE_MAX_SUPPORT_ID; i++) {
		if (ctl_vie_get_state_machine(i) != CTL_VIE_STS_RUN) {
			continue;
		}
		ctl_vie_dbg_set_msg_type(i, CTL_VIE_DBG_MSG_BUF_IO_FULL, 10, 0, 0);
	}

	return 0;
}

static int nvt_ctl_vie_proc_buf_open(struct inode *inode, struct file *file)
{
	return single_open(file, nvt_ctl_vie_proc_buf_show, NULL);
}

static struct proc_ops proc_buf_fops = {
	.proc_open   = nvt_ctl_vie_proc_buf_open,
	.proc_release = single_release,
	.proc_read   = seq_read,
	.proc_lseek = seq_lseek,
};

int nvt_ctl_vie_proc_init(PCTL_VIE_DRV_INFO pdrv_info)
{
	int ret = 0;
	struct proc_dir_entry *pmodule_root = NULL;
	struct proc_dir_entry *pentry = NULL;

	pmodule_root = proc_mkdir(MODULE_NAME, NULL);
	if (pmodule_root == NULL) {
		DBG_ERR("failed to create Module root\n");
		ret = -EINVAL;
		goto remove_root;
	}
	pdrv_info->pproc_module_root = pmodule_root;


	pentry = proc_create("cmd", S_IRUGO | S_IXUGO, pmodule_root, &proc_cmd_fops);
	if (pentry == NULL) {
		DBG_ERR("failed to create proc cmd!\n");
		ret = -EINVAL;
		goto remove_cmd;
	}
	pdrv_info->pproc_cmd_entry = pentry;

	pentry = proc_create("help", S_IRUGO | S_IXUGO, pmodule_root, &proc_help_fops);
	if (pentry == NULL) {
		DBG_ERR("failed to create proc help!\n");
		ret = -EINVAL;
		goto remove_cmd;
	}
	pdrv_info->pproc_help_entry = pentry;

	pentry = proc_create("info", S_IRUGO | S_IXUGO, pmodule_root, &proc_info_fops);
	if (pentry == NULL) {
		DBG_ERR("failed to create proc info!\n");
		ret = -EINVAL;
		goto remove_cmd;
	}
	pdrv_info->pproc_info_entry = pentry;

	pentry = proc_create("buf", S_IRUGO | S_IXUGO, pmodule_root, &proc_buf_fops);
	if (pentry == NULL) {
		DBG_ERR("failed to create proc buf!\n");
		ret = -EINVAL;
		goto remove_cmd;
	}
	pdrv_info->pproc_buf_entry = pentry;

	pentry = proc_create("dbglevel", S_IRUGO | S_IXUGO, pmodule_root, &proc_dbg_lvl_fops);
	if (pentry == NULL) {
		DBG_ERR("failed to create proc info!\n");
		ret = -EINVAL;
		goto remove_cmd;
	}
	pdrv_info->pproc_dbglevel_entry = pentry;

	pdrv_info_data = pdrv_info;

	return ret;

remove_cmd:
	proc_remove(pdrv_info->pproc_cmd_entry);

remove_root:
	proc_remove(pdrv_info->pproc_module_root);
	return ret;
}

int nvt_ctl_vie_proc_remove(PCTL_VIE_DRV_INFO pdrv_info)
{
	proc_remove(pdrv_info->pproc_help_entry);
	proc_remove(pdrv_info->pproc_cmd_entry);
	proc_remove(pdrv_info->pproc_info_entry);
	proc_remove(pdrv_info->pproc_buf_entry);
	proc_remove(pdrv_info->pproc_dbglevel_entry);
	proc_remove(pdrv_info->pproc_module_root);
	return 0;
}
