#include <linux/version.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>
#include "../include/isf_audcap_proc.h"
#include "../include/isf_audcap_dbg.h"
#include "../include/isf_audcap_main.h"
#include "../include/isf_audcap_api.h"
#include "../isf_audcap/isf_audcap_int.h"

//#define __DBGLVL__          2 // 0=FATAL, 1=ERR, 2=WRN, 3=UNIT, 4=FUNC, 5=IND, 6=MSG, 7=VALUE, 8=USER

//============================================================================
// Define
//============================================================================
#define MAX_CMD_LENGTH 100
#define MAX_ARG_NUM     6

//============================================================================
// Declaration
//============================================================================
typedef struct proc_cmd {
	char cmd[MAX_CMD_LENGTH];
	int (*execute)(PISF_AUDCAP_INFO pdrv, unsigned char argc, char **argv);
} PROC_CMD, *PPROC_CMD;

//============================================================================
// Global variable
//============================================================================
static PISF_AUDCAP_DRV_INFO p_drv_info_data;
static struct seq_file *g_seq_file;

//============================================================================
// Function define
//============================================================================
static int isf_audcap_seq_printf(const char *fmtstr, ...)
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
// proc "Custom Command" file operation functions
//=============================================================================
static ssize_t isf_acap_cmd_proc_write(struct file *file, const char __user *buf, size_t size, loff_t *off)
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
	ret = audcap_cmd_execute(ucargc, argv);

	return size;

ERR_OUT:
	return -1;
}

static int isf_acap_cmd_proc_show(struct seq_file *sfile, void *v)
{
	return 0;
}

static int isf_acap_cmd_proc_open(struct inode *inode, struct file *file)
{
	return single_open(file, isf_acap_cmd_proc_show, &p_drv_info_data->module_info);
}

#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 6, 0)
static struct file_operations isf_acap_cmd_proc_fops = {
	.owner  = THIS_MODULE,
	.open   = isf_acap_cmd_proc_open,
	.read    = seq_read,
	.write   = isf_acap_cmd_proc_write,
	.llseek  = seq_lseek,
	.release = single_release,
};
#else
static struct proc_ops isf_acap_cmd_proc_fops = {
	.proc_open    = isf_acap_cmd_proc_open,
	.proc_read    = seq_read,
	.proc_write   = isf_acap_cmd_proc_write,
	.proc_lseek   = seq_lseek,
	.proc_release = single_release,
};
#endif

//=============================================================================
// proc "dbglevel" file operation functions
//=============================================================================
static int isf_acap_dbglevel_proc_show(struct seq_file *sfile, void *v)
{
	unsigned int dbg_lvl;

	dbg_lvl = get_hdal_flow_dbglevel(HDAL_FLOW_DBG_ACAP);
	seq_printf(sfile, "0: fatal, 1: err, 2: wrn, 3: msg, 4: ind, 5: func\r\n");
	seq_printf(sfile, "acap dbglevel:%d \r\n", dbg_lvl);
	return 0;
}

static int isf_acap_dbglevel_proc_open(struct inode *inode, struct file *file)
{
	return single_open(file, isf_acap_dbglevel_proc_show, NULL);
}

static ssize_t isf_acap_dbglevel_proc_write(struct file *file, const char __user *buffer,
		size_t count, loff_t *ppos)
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
	printk("acap dbglevel=%d\n", flow_dbg_lvl);
	set_hdal_flow_dbglevel(HDAL_FLOW_DBG_ACAP, flow_dbg_lvl);
	return count;
}

#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 6, 0)
static struct file_operations isf_acap_dbglevel_proc_fops = {
	.owner   = THIS_MODULE,
	.open    = isf_acap_dbglevel_proc_open,
	.release = single_release,
	.read    = seq_read,
	.write   = isf_acap_dbglevel_proc_write,
	.llseek  = seq_lseek,
};
#else
static struct proc_ops isf_acap_dbglevel_proc_fops = {
	.proc_open    = isf_acap_dbglevel_proc_open,
	.proc_release = single_release,
	.proc_read    = seq_read,
	.proc_write   = isf_acap_dbglevel_proc_write,
	.proc_lseek   = seq_lseek,
};
#endif

//=============================================================================
// proc "help" file operation functions
//=============================================================================
static int isf_acap_help_proc_show(struct seq_file *sfile, void *v)
{
	seq_printf(sfile, "1. 'cat /proc/hdal/acap/info' will show all the audiocap info\r\n");
	return 0;
}

static int isf_acap_help_proc_open(struct inode *inode, struct file *file)
{
	return single_open(file, isf_acap_help_proc_show, NULL);
}

#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 6, 0)
static struct file_operations isf_acap_help_proc_fops = {
	.owner  = THIS_MODULE,
	.open   = isf_acap_help_proc_open,
	.release = single_release,
	.read   = seq_read,
	.llseek = seq_lseek,
};
#else
static struct proc_ops isf_acap_help_proc_fops = {
	.proc_open    = isf_acap_help_proc_open,
	.proc_release = single_release,
	.proc_read    = seq_read,
	.proc_lseek   = seq_lseek,
};
#endif

//=============================================================================
// proc "info" file operation functions
//=============================================================================
extern void debug_log_cb(struct seq_file *sfile, UINT32 uid);

static int isf_acap_info_proc_show(struct seq_file *sfile, void *v)
{
	UINT32 uid = ISF_UNIT_AUDCAP;
	ISF_UNIT *p_unit = isf_unit_ptr(uid);

	if (!g_audcap_init[0]) {
		return 0;
	}

	SEM_WAIT(ISF_AUDCAP_PROC_SEM_ID);

	g_seq_file = sfile;
	debug_log_cb(g_seq_file, 1);
	debug_log_cb(g_seq_file, ISF_UNIT_AUDCAP);

	isf_audcap_dump_status(isf_audcap_seq_printf, p_unit);

	SEM_SIGNAL(ISF_AUDCAP_PROC_SEM_ID);

	return 0;
}

static int isf_acap_info_proc_open(struct inode *inode, struct file *file)
{
	return single_open(file, isf_acap_info_proc_show, NULL);
}

#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 6, 0)
static struct file_operations isf_acap_info_proc_fops = {
	.owner  = THIS_MODULE,
	.open   = isf_acap_info_proc_open,
	.release = single_release,
	.read   = seq_read,
	.llseek = seq_lseek,
};
#else
static struct proc_ops isf_acap_info_proc_fops = {
	.proc_open    = isf_acap_info_proc_open,
	.proc_release = single_release,
	.proc_read    = seq_read,
	.proc_lseek   = seq_lseek,
};
#endif

int isf_audcap_proc_init(PISF_AUDCAP_DRV_INFO p_drv_info)
{
	int ret = 0;
	struct proc_dir_entry *p_module_root = NULL;
	struct proc_dir_entry *pentry = NULL;

	p_module_root = proc_mkdir("hdal/acap", NULL);
	if (p_module_root == NULL) {
		DBG_ERR("failed to create Module root\n");
		ret = -EINVAL;
		goto remove_root;
	}
	p_drv_info->p_proc_module_root = p_module_root;


#if 1
	pentry = proc_create("info", S_IRUGO | S_IXUGO, p_module_root, &isf_acap_info_proc_fops);
	if (pentry == NULL) {
		DBG_ERR("failed to create proc status!\r\n");
		ret = -EINVAL;
		goto remove_root;
	}

	pentry = proc_create("cmd", S_IRUGO | S_IXUGO, p_module_root, &isf_acap_cmd_proc_fops);
	if (pentry == NULL) {
		DBG_ERR("failed to create proc cmd!\n");
		ret = -EINVAL;
		goto remove_info;
	}
	p_drv_info->p_proc_cmd_entry = pentry;

	pentry = proc_create("help", S_IRUGO | S_IXUGO, p_module_root, &isf_acap_help_proc_fops);
	if (pentry == NULL) {
		DBG_ERR("failed to create proc help!\n");
		ret = -EINVAL;
		goto remove_cmd;
	}
	p_drv_info->p_proc_help_entry = pentry;

	pentry = proc_create("dbglevel", S_IRUGO | S_IXUGO, p_module_root, &isf_acap_dbglevel_proc_fops);
	if (pentry == NULL) {
		DBG_ERR("failed to create dbglevel cmd!\n");
		ret = -EINVAL;
		goto remove_help;
	}
	p_drv_info->p_proc_dbglevel_entry = pentry;
#endif

	p_drv_info_data = p_drv_info;

	return ret;

remove_help:
	if (p_drv_info->p_proc_help_entry) {
		proc_remove(p_drv_info->p_proc_help_entry);
	}

remove_cmd:
	if (p_drv_info->p_proc_cmd_entry) {
		proc_remove(p_drv_info->p_proc_cmd_entry);
	}

remove_info:
	if (p_drv_info->p_proc_info_entry) {
		proc_remove(p_drv_info->p_proc_info_entry);
	}

remove_root:
	if (p_drv_info->p_proc_module_root) {
		proc_remove(p_drv_info->p_proc_module_root);
	}
	return ret;
}

int isf_audcap_proc_remove(PISF_AUDCAP_DRV_INFO p_drv_info)
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
	return 0;
}


//=============================================================================
// proc "engine_spec" file operation functions
//=============================================================================
static int kflow_audio_engine_spec_proc_show(struct seq_file *sfile, void *v)
{
	seq_printf(sfile, "kflow_audioio engine spec\n");
	seq_printf(sfile, "------------------------------------\n");
	seq_printf(sfile, " * In-Address alignment  : %d (byte)\n", AUDIO_IN_ADDR_ALIGN);
	seq_printf(sfile, " * Out-Address alignment : %d (byte)\n", AUDIO_OUT_ADDR_ALIGN);
	return 0;
}

static int kflow_audio_engine_spec_proc_open(struct inode *inode, struct file *file)
{
	return single_open(file, kflow_audio_engine_spec_proc_show, NULL);
}

#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 6, 0)
static struct file_operations kflow_audio_engine_spec_proc_fops = {
	.owner   = THIS_MODULE,
	.open    = kflow_audio_engine_spec_proc_open,
	.release = single_release,
	.read    = seq_read,
	.llseek  = seq_lseek,
};
#else
static struct proc_ops kflow_audio_engine_spec_proc_fops = {
	.proc_open    = kflow_audio_engine_spec_proc_open,
	.proc_release = single_release,
	.proc_read    = seq_read,
	.proc_lseek   = seq_lseek,
};
#endif

//=============================================================================
// proc "dbglevel" file operation functions
//=============================================================================
static int kflow_audio_dbglevel_proc_show(struct seq_file *sfile, void *v)
{
	unsigned int dbg_lvl;

	dbg_lvl = get_hdal_flow_dbglevel(HDAL_FLOW_DBG_ACAP);
	seq_printf(sfile, "0: fatal, 1: err, 2: wrn, 3: msg, 4: ind, 5: func\r\n");
	seq_printf(sfile, "kflow_audio dbglevel:%d \r\n", dbg_lvl);
	return 0;
}

static int kflow_audio_dbglevel_proc_open(struct inode *inode, struct file *file)
{
	return single_open(file, kflow_audio_dbglevel_proc_show, NULL);
}

static ssize_t kflow_audio_dbglevel_proc_write(struct file *file, const char __user *buffer,
		size_t count, loff_t *ppos)
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
	printk("kflow_audio dbglevel=%d\n", flow_dbg_lvl);
	set_hdal_flow_dbglevel(HDAL_FLOW_DBG_ACAP, flow_dbg_lvl);
	return count;
}

#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 6, 0)
static struct file_operations kflow_audio_dbglevel_proc_fops = {
	.owner   = THIS_MODULE,
	.open    = kflow_audio_dbglevel_proc_open,
	.release = single_release,
	.read    = seq_read,
	.write   = kflow_audio_dbglevel_proc_write,
	.llseek  = seq_lseek,
};
#else
static struct proc_ops kflow_audio_dbglevel_proc_fops = {
	.proc_open    = kflow_audio_dbglevel_proc_open,
	.proc_release = single_release,
	.proc_read    = seq_read,
	.proc_write   = kflow_audio_dbglevel_proc_write,
	.proc_lseek   = seq_lseek,
};
#endif

int kflow_audio_proc_init(PISF_AUDCAP_DRV_INFO p_drv_info)
{
	int ret = 0;
	struct proc_dir_entry *p_module_root = NULL;
	struct proc_dir_entry *pentry = NULL;

	p_module_root = proc_mkdir("kflow_audio", NULL);
	if (p_module_root == NULL) {
		DBG_ERR("failed to create Module root\n");
		ret = -EINVAL;
		goto remove_root;
	}
	p_drv_info->p_proc_kflow_module_root = p_module_root;


#if 1
	pentry = proc_create("engine_spec", S_IRUGO | S_IXUGO, p_module_root, &kflow_audio_engine_spec_proc_fops);
	if (pentry == NULL) {
		DBG_ERR("failed to create engine_spec cmd!\n");
		ret = -EINVAL;
		goto remove_root;
	}
	p_drv_info->p_proc_kflow_engine_spec_entry = pentry;

	pentry = proc_create("dbglevel", S_IRUGO | S_IXUGO, p_module_root, &kflow_audio_dbglevel_proc_fops);
	if (pentry == NULL) {
		DBG_ERR("failed to create dbglevel cmd!\n");
		ret = -EINVAL;
		goto remove_engine_spec;
	}
	p_drv_info->p_proc_kflow_dbglevel_entry = pentry;
#endif


	return ret;

remove_engine_spec:
	if (p_drv_info->p_proc_kflow_engine_spec_entry) {
		proc_remove(p_drv_info->p_proc_kflow_engine_spec_entry);
	}

remove_root:
	if (p_drv_info->p_proc_kflow_module_root) {
		proc_remove(p_drv_info->p_proc_kflow_module_root);
	}
	return ret;
}

int kflow_audio_proc_remove(PISF_AUDCAP_DRV_INFO p_drv_info)
{
	if (p_drv_info->p_proc_kflow_engine_spec_entry) {
		proc_remove(p_drv_info->p_proc_kflow_engine_spec_entry);
	}
	if (p_drv_info->p_proc_kflow_dbglevel_entry) {
		proc_remove(p_drv_info->p_proc_kflow_dbglevel_entry);
	}
	if (p_drv_info->p_proc_kflow_module_root) {
		proc_remove(p_drv_info->p_proc_kflow_module_root);
	}
	return 0;
}
