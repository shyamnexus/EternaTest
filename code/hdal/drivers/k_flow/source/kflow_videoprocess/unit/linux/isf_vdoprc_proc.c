#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>
#include "isf_vdoprc_proc.h"
#include "isf_vdoprc_main.h"
#include "../isf_vdoprc_api.h"
#include "../isf_vdoprc_dbg.h"
#include "../isf_vdoprc_int.h"
#include "kwrap/semaphore.h"


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
	int (*execute)(PMODULE_INFO p_drv, unsigned char argc, char **argv);
} PROC_CMD, *PPROC_CMD;

static struct seq_file *g_seq_file;

//============================================================================
// Global variable
//============================================================================
static ISF_VDOPRC_DRV_INFO *p_drv_info_data;

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

	seq_printf(g_seq_file, buf);

	return 0;
}

//=============================================================================
// proc "info" file operation functions
//=============================================================================
extern void debug_log_cb(struct seq_file *sfile, UINT32 uid);
static int isf_vdoprc_info_proc_show(struct seq_file *sfile, void *v)
{
	UINT32 dev;

	if (!g_vdoprc_init[0]) {
		return 0;
	}
	SEM_WAIT(ISF_VDOPRC_PROC_SEM_ID);
	g_seq_file = sfile;
	//dump info of all devices
	debug_log_cb(g_seq_file, 1); //show hdal version
	for(dev = 0; dev < VDOPRC_MAX_NUM; dev++) {
		UINT32 uid = ISF_UNIT_VDOPRC + dev;
		ISF_UNIT *p_unit = isf_unit_ptr(uid);
		//dump bind, state and param settings of 1 device
		debug_log_cb(g_seq_file, uid);
		//dump work status of 1 device
		isf_vdoprc_dump_status(isf_vdoprc_seq_printf, p_unit);
	}
	SEM_SIGNAL(ISF_VDOPRC_PROC_SEM_ID);
	return 0;
}

static int isf_vdoprc_info_proc_open(struct inode *inode, struct file *file)
{
	return single_open(file, isf_vdoprc_info_proc_show, NULL);
}

static struct proc_ops isf_vdoprc_info_proc_fops = {
	.proc_open   = isf_vdoprc_info_proc_open,
	.proc_release = single_release,
	.proc_read   = seq_read,
	.proc_lseek = seq_lseek,
};

#if 1
//=============================================================================
// proc "Custom Command" file operation functions
//=============================================================================
static int isf_vdoprc_proc_cmd_show(struct seq_file *sfile, void *v)
{
	DBG_IND("\n");
	return 0;
}

static int isf_vdoprc_proc_cmd_open(struct inode *inode, struct file *file)
{
	DBG_IND("\n");
	return single_open(file, isf_vdoprc_proc_cmd_show, &p_drv_info_data->module_info);
}

static ssize_t isf_vdoprc_proc_cmd_write(struct file *file, const char __user *buf,
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
	ret = vdoprc_cmd_execute(ucargc, argv);

	return size;

ERR_OUT:
	return -1;
}

static struct proc_ops isf_vdoprc_cmd_proc_fops = {
	.proc_open    = isf_vdoprc_proc_cmd_open,
	.proc_read    = seq_read,
	.proc_lseek  = seq_lseek,
	.proc_release = single_release,
	.proc_write   = isf_vdoprc_proc_cmd_write
};

//=============================================================================
// proc "help" file operation functions
//=============================================================================
static int isf_vdoprc_proc_help_show(struct seq_file *sfile, void *v)
{
	seq_printf(sfile, "=====================================================================\n");
	seq_printf(sfile, " Add message here\n");
	seq_printf(sfile, "=====================================================================\n");
	return 0;
}

static int isf_vdoprc_proc_help_open(struct inode *inode, struct file *file)
{
	return single_open(file, isf_vdoprc_proc_help_show, NULL);
}

static struct proc_ops isf_vdoprc_help_proc_fops = {
	.proc_open   = isf_vdoprc_proc_help_open,
	.proc_release = single_release,
	.proc_read   = seq_read,
	.proc_lseek = seq_lseek,
};
#endif
//=============================================================================
// proc "dbglevel" file operation functions
//=============================================================================
static int isf_vdoprc_dbglevel_proc_show(struct seq_file *sfile, void *v)
{
	unsigned int dbg_lvl;

	dbg_lvl = get_hdal_flow_dbglevel(HDAL_FLOW_DBG_VPRC);
	seq_printf(sfile, "0: fatal, 1: err, 2: wrn, 3: msg, 4: ind, 5: func\r\n");
	seq_printf(sfile, "vprc dbglevel:%d \r\n", dbg_lvl);
	return 0;
}

static int isf_vdoprc_dbglevel_proc_open(struct inode *inode, struct file *file)
{
	return single_open(file, isf_vdoprc_dbglevel_proc_show, NULL);
}

static ssize_t isf_vdoprc_dbglevel_proc_write(struct file *file, const char __user *buffer, size_t count, loff_t *ppos)
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
	printk("vprc dbglevel=%d\n", flow_dbg_lvl);
	set_hdal_flow_dbglevel(HDAL_FLOW_DBG_VPRC, flow_dbg_lvl);
	return count;
}

static struct proc_ops isf_vdoprc_dbglevel_proc_fops = {
	.proc_open    = isf_vdoprc_dbglevel_proc_open,
	.proc_read    = seq_read,
	.proc_lseek  = seq_lseek,
	.proc_release = single_release,
	.proc_write   = isf_vdoprc_dbglevel_proc_write
};

int isf_vdoprc_proc_init(ISF_VDOPRC_DRV_INFO *p_drv_info)
{
	int ret = 0;
	//struct proc_dir_entry *p_hdal_root = NULL;
	struct proc_dir_entry *p_module_root = NULL;
	struct proc_dir_entry *p_entry = NULL;
	/*
	p_hdal_root = proc_mkdir("hdal", NULL);
	if (p_hdal_root == NULL) {
		VPRC_DBG_ERR("failed to create hdal root\r\n");
		ret = -EINVAL;
		return ret;
	}
	p_drv_info->p_proc_hdal_root = p_hdal_root;
	*/
	p_module_root = proc_mkdir("hdal/vprc", NULL);
	if (p_module_root == NULL) {
		VPRC_DBG_ERR("failed to create Module root\r\n");
		ret = -EINVAL;
		//goto remove_hdal;
		return ret;
	}
	p_drv_info->p_proc_module_root = p_module_root;
	p_entry = proc_create("info", S_IRUGO | S_IXUGO, p_module_root, &isf_vdoprc_info_proc_fops);
	if (p_entry == NULL) {
		VPRC_DBG_ERR("failed to create proc status!\r\n");
		ret = -EINVAL;
		goto remove_root;
	}
	p_drv_info->p_proc_info_entry = p_entry;
	p_entry = proc_create("cmd", S_IRUGO | S_IXUGO, p_module_root, &isf_vdoprc_cmd_proc_fops);
	if (p_entry == NULL) {
		VPRC_DBG_ERR("failed to create proc cmd!\r\n");
		ret = -EINVAL;
		goto remove_info;
	}
	p_drv_info->p_proc_cmd_entry = p_entry;
	p_entry = proc_create("help", S_IRUGO | S_IXUGO, p_module_root, &isf_vdoprc_help_proc_fops);
	if (p_entry == NULL) {
		VPRC_DBG_ERR("failed to create proc meminfo!\r\n");
		ret = -EINVAL;
		goto remove_cmd;
	}
	p_drv_info->p_proc_help_entry = p_entry;
	// dbglevel
	p_entry = proc_create("dbglevel", S_IRUGO | S_IXUGO, p_module_root, &isf_vdoprc_dbglevel_proc_fops);
	if (p_entry == NULL) {
		DBG_ERR("failed to create proc dbglevel!\r\n");
		ret = -EINVAL;
		goto remove_help;
	}
	p_drv_info->p_proc_dbglevel_entry = p_entry;
	p_drv_info_data = p_drv_info;
	return ret;

remove_help:
	proc_remove(p_drv_info->p_proc_help_entry);
remove_cmd:
	proc_remove(p_drv_info->p_proc_cmd_entry);
remove_info:
	proc_remove(p_drv_info->p_proc_info_entry);
remove_root:
	proc_remove(p_drv_info->p_proc_module_root);
//remove_hdal:
//	proc_remove(p_drv_info->p_proc_hdal_root);
	return ret;
}

int isf_vdoprc_proc_remove(ISF_VDOPRC_DRV_INFO *p_drv_info)
{
	if (p_drv_info_data) {
		proc_remove(p_drv_info->p_proc_dbglevel_entry);
		proc_remove(p_drv_info->p_proc_help_entry);
		proc_remove(p_drv_info->p_proc_cmd_entry);
		proc_remove(p_drv_info->p_proc_info_entry);
		proc_remove(p_drv_info->p_proc_module_root);
//		proc_remove(p_drv_info->p_proc_hdal_root);
		p_drv_info_data = NULL;
	}
	return 0;
}


///////////////////////////////////////////////////////


//=============================================================================
// proc "flow" file operation functions
//=============================================================================

#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/kmod.h>

BOOL _isf_vdoprc_api_flow(unsigned char argc, char **pargv)
{
#if defined(_BSP_NA51000_)
	char * envp[] = { "HOME=/", "TERM=linux", "PATH=/sbin:/usr/sbin:/bin:/usr/bin", NULL };
	char * argv[] = { "/bin/ash", "-c", "echo r dump_hdl_all > /proc/kflow_ctl_ipp/cmd", NULL };
	if (!_isf_vdoprc_is_init()) {
		VPRC_DBG_ERR("[vdoprc] not init\r\n");
		return 1;
	}
	VPRC_DBG_DUMP("[kflow ctl_ipp]\r\n");
	call_usermodehelper(argv[0], argv, envp, UMH_WAIT_EXEC);
#endif
#if defined(_BSP_NA51055_) || defined(_BSP_NA51089_) || defined(_BSP_NA51102_) || defined(_BSP_NS02302_)
	char * envp[] = { "HOME=/", "TERM=linux", "PATH=/sbin:/usr/sbin:/bin:/usr/bin", NULL };
	char * argv[] = { "/bin/ash", "-c", "echo r dump_hdl_all > /proc/kflow_ctl_ipp/cmd", NULL };
	if (!_isf_vdoprc_is_init()) {
		VPRC_DBG_ERR("[vdoprc] not init\r\n");
		return 1;
	}
	VPRC_DBG_DUMP("[kflow ctl_ipp]\r\n");
	call_usermodehelper(argv[0], argv, envp, UMH_WAIT_EXEC);
#endif
	return 1;
}

//=============================================================================
// proc "drv" file operation functions
//=============================================================================

BOOL _isf_vdoprc_api_drv(unsigned char argc, char **pargv)
{
#if defined(_BSP_NA51000_)
	char * envp[] = { "HOME=/", "TERM=linux", "PATH=/sbin:/usr/sbin:/bin:/usr/bin", NULL };
	char * argv1[] = { "/bin/ash", "-c", "echo r dump > /proc/kdrv_rhe/cmd", NULL };
	char * argv2[] = { "/bin/ash", "-c", "echo r dump > /proc/kdrv_ife/cmd", NULL };
	char * argv3[] = { "/bin/ash", "-c", "echo r dump > /proc/kdrv_ife2/cmd", NULL };
	char * argv4[] = { "/bin/ash", "-c", "echo r dump > /proc/kdrv_dce/cmd", NULL };
	char * argv5[] = { "/bin/ash", "-c", "echo r dump > /proc/kdrv_ipe/cmd", NULL };
	char * argv6[] = { "/bin/ash", "-c", "echo r dump > /proc/kdrv_ime/cmd", NULL };
	if (!_isf_vdoprc_is_init()) {
		VPRC_DBG_ERR("[vdoprc] not init\r\n");
		return 1;
	}
	VPRC_DBG_DUMP("[kdrv rhe]\r\n");
	call_usermodehelper(argv1[0], argv1, envp, UMH_WAIT_EXEC);
	VPRC_DBG_DUMP("[kdrv ife]\r\n");
	call_usermodehelper(argv2[0], argv2, envp, UMH_WAIT_EXEC);
	VPRC_DBG_DUMP("[kdrv ife2]\r\n");
	call_usermodehelper(argv3[0], argv3, envp, UMH_WAIT_EXEC);
	VPRC_DBG_DUMP("[kdrv dce]\r\n");
	call_usermodehelper(argv4[0], argv4, envp, UMH_WAIT_EXEC);
	VPRC_DBG_DUMP("[kdrv ipe]\r\n");
	call_usermodehelper(argv5[0], argv5, envp, UMH_WAIT_EXEC);
	VPRC_DBG_DUMP("[kdrv ime]\r\n");
	call_usermodehelper(argv6[0], argv6, envp, UMH_WAIT_EXEC);
#endif
#if defined(_BSP_NA51055_) || defined(_BSP_NA51089_) || defined(_BSP_NA51102_) || defined(_BSP_NS02302_)
	char * envp[] = { "HOME=/", "TERM=linux", "PATH=/sbin:/usr/sbin:/bin:/usr/bin", NULL };
	char * argv2[] = { "/bin/ash", "-c", "echo r dump > /proc/kdrv_ife/cmd", NULL };
	char * argv3[] = { "/bin/ash", "-c", "echo r dump > /proc/kdrv_ife2/cmd", NULL };
	char * argv4[] = { "/bin/ash", "-c", "echo r dump > /proc/kdrv_dce/cmd", NULL };
	char * argv5[] = { "/bin/ash", "-c", "echo r dump > /proc/kdrv_ipe/cmd", NULL };
	char * argv6[] = { "/bin/ash", "-c", "echo r dump > /proc/kdrv_ime/cmd", NULL };
	if (!_isf_vdoprc_is_init()) {
		VPRC_DBG_ERR("[vdoprc] not init\r\n");
		return 1;
	}
	VPRC_DBG_DUMP("[kdrv ife]\r\n");
	call_usermodehelper(argv2[0], argv2, envp, UMH_WAIT_EXEC);
	VPRC_DBG_DUMP("[kdrv ife2]\r\n");
	call_usermodehelper(argv3[0], argv3, envp, UMH_WAIT_EXEC);
	VPRC_DBG_DUMP("[kdrv dce]\r\n");
	call_usermodehelper(argv4[0], argv4, envp, UMH_WAIT_EXEC);
	VPRC_DBG_DUMP("[kdrv ipe]\r\n");
	call_usermodehelper(argv5[0], argv5, envp, UMH_WAIT_EXEC);
	VPRC_DBG_DUMP("[kdrv ime]\r\n");
	call_usermodehelper(argv6[0], argv6, envp, UMH_WAIT_EXEC);
#endif
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
#if defined(_BSP_NA51055_) || defined(_BSP_NA51089_) || defined(_BSP_NA51102_) || defined(_BSP_NS02302_)
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
#endif
	return 1;
}

//=============================================================================
// proc "rate" file operation functions
//=============================================================================
BOOL _isf_vdoprc_api_rate2(unsigned char argc, char **pargv)
{
#if defined(_BSP_NA51000_)
	{
	char * envp[] = { "HOME=/", "TERM=linux", "PATH=/sbin:/usr/sbin:/bin:/usr/bin", NULL };
	char * argv[] = { "/bin/ash", "-c", "echo r dump_t > /proc/kflow_ctl_ipp/cmd", NULL };
	VPRC_DBG_DUMP("[kflow ctl_ipp]\r\n");
	call_usermodehelper(argv[0], argv, envp, UMH_WAIT_EXEC);
	}
#endif
#if defined(_BSP_NA51055_) || defined(_BSP_NA51089_) || defined(_BSP_NA51102_) || defined(_BSP_NS02302_)
	{
	char * envp[] = { "HOME=/", "TERM=linux", "PATH=/sbin:/usr/sbin:/bin:/usr/bin", NULL };
	char * argv[] = { "/bin/ash", "-c", "echo r dump_t > /proc/kflow_ctl_ipp/cmd", NULL };
	VPRC_DBG_DUMP("[kflow ctl_ipp]\r\n");
	call_usermodehelper(argv[0], argv, envp, UMH_WAIT_EXEC);
	}
#endif
	return 1;
}

