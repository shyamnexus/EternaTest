#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>
#include "isf_vdoout_proc.h"
#include "isf_vdoout_main.h"
#include "isf_vdoout_drv.h"
#include "../isf_vdoout_int.h"
#include "../isf_vdoout_api.h"
#include "../isf_vdoout_dbg.h"

//============================================================================
// Define
//============================================================================
#define MAX_CMD_LENGTH 30
#define MAX_ARG_NUM     6

//============================================================================
// Declaration
//============================================================================
typedef struct proc_cmd {
	char cmd[MAX_CMD_LENGTH];
	int (*execute)(ISF_VDOOUT_INFO*,unsigned char argc, char **argv);
} PROC_CMD, *PPROC_CMD;

//============================================================================
// Global variable
//============================================================================
static ISF_VDOOUT_DRV_INFO *p_drv_info_data;
static struct seq_file *g_seq_file;

//============================================================================
// Function define
//============================================================================
static int isf_vdoout_seq_printf(const char *fmtstr, ...)
{
	char    buf[512];
	int     len;

	va_list marker;

	va_start(marker, fmtstr);

	len = vsnprintf(buf, sizeof(buf), fmtstr, marker);
	va_end(marker);
    seq_printf(g_seq_file, buf);

    return 0;
}

//=============================================================================
// proc "info" file operation functions
//=============================================================================
extern void debug_log_cb(struct seq_file *sfile, UINT32 uid);
extern SEM_HANDLE ISF_VDOOUT_PROC_SEM_ID;
static int isf_vdoout_proc_info_show(struct seq_file *sfile, void *v)
{
	UINT32 dev;

	SEM_WAIT(ISF_VDOOUT_PROC_SEM_ID);
	g_seq_file = sfile;
	debug_log_cb(g_seq_file, 1); //show hdal version
    for(dev = 0; dev < 2; dev++) {
		UINT32 uid = ISF_UNIT_VDOOUT + dev;
		ISF_UNIT *p_unit = isf_unit_ptr(uid);

		if(p_unit == NULL){
			continue;
		}

		//dump bind, state and param settings of 1 device
		debug_log_cb(g_seq_file, uid);
		//dump work status of 1 device
		isf_vdoout_dump_status(isf_vdoout_seq_printf, p_unit);

	}

	SEM_SIGNAL(ISF_VDOOUT_PROC_SEM_ID);
	return 0;
}
static int isf_vdoout_proc_info_open(struct inode *inode, struct file *file)
{
	return single_open(file, isf_vdoout_proc_info_show, NULL);
}

static struct proc_ops proc_info_fops = {
	.proc_open    = isf_vdoout_proc_info_open,
	.proc_release = single_release,
	.proc_read    = seq_read,
	.proc_lseek   = seq_lseek,
};

//=============================================================================
// proc "Custom Command" file operation functions
//=============================================================================
#if 0
static PROC_CMD cmd_list[] = {
	// keyword          function name
	{ "debug",    	_isf_vdoout_api_debug },
	{ "trace",    	_isf_vdoout_api_trace },
	{ "probe",    	_isf_vdoout_api_probe },
	{ "perf",    	_isf_vdoout_api_perf },
	{ "save",    	_isf_vdoout_api_save },
};

#define NUM_OF_CMD (sizeof(cmd_list) / sizeof(PROC_CMD))
#endif
static int isf_vdoout_proc_cmd_show(struct seq_file *sfile, void *v)
{
	DBG_IND("\n");
	return 0;
}

static int isf_vdoout_proc_cmd_open(struct inode *inode, struct file *file)
{
	DBG_IND("\n");
	return single_open(file, isf_vdoout_proc_cmd_show, &p_drv_info_data->module_info);
}

static ssize_t isf_vdoout_proc_cmd_write(struct file *file, const char __user *buf,
		size_t size, loff_t *off)
{
	int len = size;
	int ret = -EINVAL;
	char cmd_line[MAX_CMD_LENGTH];
	char *cmdstr = cmd_line;
	const char delimiters[] = {' ', 0x0A, 0x0D, '\0'};
	char *argv[MAX_ARG_NUM] = {0};
	unsigned char ucargc = 0;

	// check command length
	if (len > (MAX_CMD_LENGTH - 1)) {
		nvt_dbg(ERR, "Command length is too long!\n");
		goto ERR_OUT;
	}

	// copy command string from user space
	if (copy_from_user(cmd_line, buf, len)) {
		goto ERR_OUT;
	}

	if (len) {
		cmd_line[len - 1] = '\0';
	} else {
		cmd_line[0] = 0;
	}

	DBG_IND("CMD:%s\n", cmd_line);

	argv[0] = 0;
	// parse command string
	for (ucargc = 1; ucargc < MAX_ARG_NUM; ucargc++) {
		argv[ucargc] = strsep(&cmdstr, delimiters);

		if (argv[ucargc] == NULL) {
			break;
		}
	}
	if (ucargc==0) {
		goto ERR_OUT;
	}

	// dispatch command handler
	ret = vdoout_cmd_execute(ucargc, argv);

	return size;

ERR_OUT:
	return -1;
}

static struct proc_ops proc_cmd_fops = {
	.proc_open    = isf_vdoout_proc_cmd_open,
	.proc_read    = seq_read,
	.proc_lseek   = seq_lseek,
	.proc_release = single_release,
	.proc_write   = isf_vdoout_proc_cmd_write
};

//=============================================================================
// proc "help" file operation functions
//=============================================================================
static int isf_vdoout_proc_help_show(struct seq_file *sfile, void *v)
{
	seq_printf(sfile, "=====================================================================\n");
	seq_printf(sfile, " vdoout message \n");
	seq_printf(sfile, "=====================================================================\n");
	return 0;
}

static int isf_vdoout_proc_help_open(struct inode *inode, struct file *file)
{
	return single_open(file, isf_vdoout_proc_help_show, NULL);
}

static struct proc_ops proc_help_fops = {
	.proc_open   = isf_vdoout_proc_help_open,
	.proc_release = single_release,
	.proc_read   = seq_read,
	.proc_lseek = seq_lseek,
};

//=============================================================================
// proc "dbglevel" file operation functions
//=============================================================================
static int isf_vdoout_proc_dbglevel_show(struct seq_file *sfile, void *v)
{
	unsigned int dbg_lvl;

	dbg_lvl = get_hdal_flow_dbglevel(HDAL_FLOW_DBG_VOUT);
	seq_printf(sfile, "0: fatal, 1: err, 2: wrn, 3: msg, 4: ind, 5: func\r\n");
	seq_printf(sfile, "vout dbglevel:%d \r\n", dbg_lvl);
	return 0;
}

static int isf_vdoout_dbglevel_proc_open(struct inode *inode, struct file *file)
{
	return single_open(file, isf_vdoout_proc_dbglevel_show, NULL);
}

static ssize_t isf_vdoout_proc_dbglevel_write(struct file *file, const char __user *buffer, size_t count, loff_t *ppos)
{
	unsigned int flow_dbg_lvl;
	char ker_buffer[64] = {0};

	if (count > sizeof(ker_buffer)) {
		printk("count %zd > max %zd\n", count, sizeof(ker_buffer));
		return -EINVAL;
	}

	if (copy_from_user((void *)ker_buffer, buffer, count)) {
		return -EINVAL;
	}
	sscanf(ker_buffer, "%d", &flow_dbg_lvl);
	printk("vout dbglevel=%d\n", flow_dbg_lvl);
	set_hdal_flow_dbglevel(HDAL_FLOW_DBG_VOUT, flow_dbg_lvl);
	return count;
}

static struct proc_ops proc_dbglevel_fops = {
	.proc_open   = isf_vdoout_dbglevel_proc_open,
	.proc_release = single_release,
	.proc_read   = seq_read,
	.proc_write   = isf_vdoout_proc_dbglevel_write,
	.proc_lseek = seq_lseek,
};

//=============================================================================
// proc "kflow_vout/dbglevel" file operation functions
//=============================================================================
static int isf_vdoout_proc_kflow_dbglevel_show(struct seq_file *sfile, void *v)
{
	seq_printf(sfile, "0: fatal, 1: err, 2: wrn, 3: msg, 4: ind, 5: func\r\n");
	seq_printf(sfile, "kflow_vout dbglevel = %u \r\n", isf_vdoout_debug_level);
	return 0;
}

static int isf_vdoout_kflow_dbglevel_proc_open(struct inode *inode, struct file *file)
{
	return single_open(file, isf_vdoout_proc_kflow_dbglevel_show, NULL);
}

static ssize_t isf_vdoout_proc_kflow_dbglevel_write(struct file *file, const char __user *buffer, size_t count, loff_t *ppos)
{
	char ker_buffer[64] = {0};

	if (count > sizeof(ker_buffer)) {
		printk("count %zd > max %zd\n", count, sizeof(ker_buffer));
		return -EINVAL;
	}

	if (copy_from_user((void *)ker_buffer, buffer, count)) {
		return -EINVAL;
	}
	sscanf(ker_buffer, "%d", &isf_vdoout_debug_level);

	return count;
}

static struct proc_ops proc_kflow_dbglevel_fops = {
	.proc_open   = isf_vdoout_kflow_dbglevel_proc_open,
	.proc_release = single_release,
	.proc_read   = seq_read,
	.proc_write   = isf_vdoout_proc_kflow_dbglevel_write,
	.proc_lseek = seq_lseek,
};

//=============================================================================
// proc "kflow_vout/engine_spec" file operation functions
//=============================================================================
static int isf_vdoout_proc_kflow_engine_spec_show(struct seq_file *sfile, void *v)
{
	seq_printf(sfile, "streaming : yuv420/yuv422/yuv420_ycc\r\n");
	seq_printf(sfile, "framebuffer : argb1555/4444/8888 and palette 1/2/4\r\n");
	return 0;
}

static int isf_vdoout_kflow_engine_spec_proc_open(struct inode *inode, struct file *file)
{
	return single_open(file, isf_vdoout_proc_kflow_engine_spec_show, NULL);
}

static struct proc_ops proc_kflow_engine_spec_fops = {
	.proc_open   = isf_vdoout_kflow_engine_spec_proc_open,
	.proc_release = single_release,
	.proc_read   = seq_read,
	.proc_lseek = seq_lseek,
};

int isf_vdoout_proc_init(ISF_VDOOUT_DRV_INFO *p_drv_info)
{
	int ret = 0;
	struct proc_dir_entry *p_module_root = NULL;
	struct proc_dir_entry *pentry = NULL;

	p_module_root = proc_mkdir("hdal/vout", NULL);
	if (p_module_root == NULL) {
		VDOOUT_DBG_ERR("failed to create module root\r\n");
		ret = -EINVAL;
		goto remove_root;
	}
	p_drv_info->p_proc_module_root = p_module_root;

	pentry = proc_create("cmd", S_IRUGO | S_IXUGO, p_module_root, &proc_cmd_fops);
	if (pentry == NULL) {
		nvt_dbg(ERR, "failed to create proc cmd!\n");
		ret = -EINVAL;
		goto remove_cmd;
	}
	p_drv_info->p_proc_cmd_entry = pentry;

	pentry = proc_create("help", S_IRUGO | S_IXUGO, p_module_root, &proc_help_fops);
	if (pentry == NULL) {
		nvt_dbg(ERR, "failed to create proc help!\n");
		ret = -EINVAL;
		goto remove_cmd;
	}
	p_drv_info->p_proc_help_entry = pentry;

	pentry = proc_create("info", S_IRUGO | S_IXUGO, p_module_root, &proc_info_fops);
	if (pentry == NULL) {
		nvt_dbg(ERR, "failed to create proc info!\n");
		ret = -EINVAL;
		goto remove_info;
	}
	p_drv_info->p_proc_info_entry = pentry;

	pentry = proc_create("dbglevel", S_IRUGO | S_IXUGO, p_module_root, &proc_dbglevel_fops);
	if (pentry == NULL) {
		nvt_dbg(ERR, "failed to create proc dbglevel!\n");
		ret = -EINVAL;
		goto remove_info;
	}
	p_drv_info->p_proc_dbglevel_entry = pentry;

	p_module_root = proc_mkdir("kflow_vout", NULL);
	if (p_module_root == NULL) {
		VDOOUT_DBG_ERR("failed to create /prock/kflow_vout\r\n");
		ret = -EINVAL;
		goto remove_root;
	}
	p_drv_info->p_proc_module_kflow = p_module_root;

	pentry = proc_create("dbglevel", S_IRUGO | S_IXUGO, p_module_root, &proc_kflow_dbglevel_fops);
	if (pentry == NULL) {
		nvt_dbg(ERR, "failed to create /proc/kflow_vout/dbglevel!\n");
		ret = -EINVAL;
		goto remove_cmd;
	}
	p_drv_info->p_proc_kflow_dbglevel_entry = pentry;

	pentry = proc_create("engine_spec", S_IRUGO | S_IXUGO, p_module_root, &proc_kflow_engine_spec_fops);
	if (pentry == NULL) {
		nvt_dbg(ERR, "failed to create /proc/kflow_vout/engine_spec!\n");
		ret = -EINVAL;
		goto remove_cmd;
	}
	p_drv_info->p_proc_kflow_engine_spec_entry = pentry;

	p_drv_info_data = p_drv_info;

	return ret;

remove_info:
	if (p_drv_info->p_proc_info_entry) {
		proc_remove(p_drv_info->p_proc_info_entry);
	}

remove_cmd:
	if (p_drv_info->p_proc_cmd_entry) {
		proc_remove(p_drv_info->p_proc_cmd_entry);
	}
	if (p_drv_info->p_proc_help_entry) {
		proc_remove(p_drv_info->p_proc_help_entry);
	}
remove_root:
	if (p_drv_info->p_proc_module_root) {
		proc_remove(p_drv_info->p_proc_module_root);
	}
	return ret;
}

int isf_vdoout_proc_remove(ISF_VDOOUT_DRV_INFO *p_drv_info)
{
	if (p_drv_info->p_proc_dbglevel_entry) {
		proc_remove(p_drv_info->p_proc_dbglevel_entry);
	}
	if (p_drv_info->p_proc_help_entry) {
		proc_remove(p_drv_info->p_proc_help_entry);
	}
	if (p_drv_info->p_proc_cmd_entry) {
		proc_remove(p_drv_info->p_proc_cmd_entry);
	}
	if (p_drv_info->p_proc_module_root) {
		proc_remove(p_drv_info->p_proc_module_root);
	}
	if (p_drv_info->p_proc_kflow_dbglevel_entry) {
		proc_remove(p_drv_info->p_proc_kflow_dbglevel_entry);
	}
	if (p_drv_info->p_proc_kflow_engine_spec_entry) {
		proc_remove(p_drv_info->p_proc_kflow_engine_spec_entry);
	}
	if (p_drv_info->p_proc_module_kflow) {
		proc_remove(p_drv_info->p_proc_module_kflow);
	}
	return 0;
}
