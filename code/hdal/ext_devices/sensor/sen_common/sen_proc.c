#include <linux/version.h>
//#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>
#include <linux/seq_file.h>
#include <kwrap/debug.h>
#include "sen_proc.h"

//============================================================================
// Define
//============================================================================
#define MAX_CMD_LENGTH  96
#define MAX_ARG_NUM     20
#define ALIGN_CEIL(value, base)   ALIGN_FLOOR((value) + ((base)-1), base)

//============================================================================
// Declaration
//============================================================================
typedef struct _SEN_DRV_INFO {
	// proc entries
	struct proc_dir_entry *pproc_module_root;
	struct proc_dir_entry *pproc_cmd_entry;;
	// private data
	char module_name[MAX_CMD_LENGTH];
	DUMP_CB dump_cb;
} SEN_DRV_INFO, *PSEN_DRV_INFO;

typedef struct _PROC_CMD {
	char cmd[MAX_CMD_LENGTH];
	int (*execute)(SEN_DRV_INFO *pdrv, unsigned char argc, char **argv);
} PROC_CMD, *PPROC_CMD;

//============================================================================
// Global variable
//============================================================================
static SEN_DRV_INFO drv_info = { 0 };

//============================================================================
// Function define
//============================================================================
int sen_cmd_dump(SEN_DRV_INFO *p_drv, unsigned char argc, char **argv)
{
	unsigned int sen_mode, fps, expt, gain, mirror, flip;
	if (drv_info.dump_cb && argc != 9) {
		sen_mode = simple_strtoul(argv[1], NULL, 0);
		fps = simple_strtoul(argv[2], NULL, 0);
		expt = simple_strtoul(argv[3], NULL, 0);
		gain = simple_strtoul(argv[4], NULL, 0);
		mirror = simple_strtoul(argv[5], NULL, 0);
		flip = simple_strtoul(argv[6], NULL, 0);
		drv_info.dump_cb(sen_mode, fps, expt, gain, mirror, flip, argv[7]);
	} else {
		nvt_dbg(ERR, "dump: command parameter number error (%d) \n", argc);
	}
	return 0;
}

//=============================================================================
// proc "Custom Command" file operation functions
//=============================================================================
static PROC_CMD cmd_list[] = {
	// keyword      function name
	{ "dump",       sen_cmd_dump},
};

#define NUM_OF_CMD (sizeof(cmd_list) / sizeof(PROC_CMD))

//=============================================================================
// proc "test1" file operation functions
//=============================================================================
static ssize_t sen_cmd_proc_write(struct file *file, const char __user *buf, size_t size, loff_t *off)
{
	int len = size;
	int ret = -EINVAL;
	char cmd_line[MAX_CMD_LENGTH];
	char *cmdstr = cmd_line;
	const char delimiters[] = {' ', 0x0A, 0x0D, '\0'};
	unsigned char argc = 0;
	char *argv[MAX_ARG_NUM] = {0};
	unsigned char loop;

	// check command length
	if (len > (MAX_CMD_LENGTH - 1)) {
		nvt_dbg(ERR, "Command length is too long \n");
		goto ERR_OUT;
	}
	if (len == 0) {
		nvt_dbg(ERR, "Command length is 0 \n");
		goto ERR_OUT;
	}

	// copy command string from user space
	if (copy_from_user(cmd_line, buf, len)) {
		goto ERR_OUT;
	}

	cmd_line[len - 1] = '\0';

	DBG_IND("CMD:%s\n", cmd_line);

	// parse command string
	for (argc = 0; argc < MAX_ARG_NUM; argc++) {
		argv[argc] = strsep(&cmdstr, delimiters);
		if (argv[argc] == NULL) {
			break;
		}
	}

	// dispatch command handler
	for (loop = 0 ; loop < NUM_OF_CMD; loop++) {
		if (strncmp(argv[0], cmd_list[loop].cmd, MAX_CMD_LENGTH) == 0) {
			ret = cmd_list[loop].execute(&drv_info, argc, &argv[0]);
			break;
		}
	}
	if (loop >= NUM_OF_CMD) {
		goto ERR_INVALID_CMD;
	}
	return size;

ERR_INVALID_CMD:
	nvt_dbg(ERR, "Invalid CMD \r\n");
ERR_OUT:
	return -1;
}

static int sen_cmd_proc_show(struct seq_file *sfile, void *v)
{
	seq_printf(sfile, "echo dump /tmp/nvt-fastboot-%s.dtsi > /proc/%s/cmd\n", drv_info.module_name, drv_info.module_name);
	return 0;
}

static int sen_cmd_proc_open(struct inode *inode, struct file *file)
{
	return single_open(file, sen_cmd_proc_show, PDE_DATA(inode));
}

// plug file-system


#if (LINUX_VERSION_CODE > KERNEL_VERSION(5, 10, 0))
static const struct proc_ops sen_cmd_proc_fops = {
	.proc_open    = sen_cmd_proc_open,
	.proc_write   = sen_cmd_proc_write,
	.proc_read    = seq_read,
	.proc_lseek   = seq_lseek,
	.proc_release = single_release,
};
#else
static const struct file_operations sen_cmd_proc_fops = {
	.owner   = THIS_MODULE,
	.open    = sen_cmd_proc_open,
	.read    = seq_read,
	.write   = sen_cmd_proc_write,
	.llseek  = seq_lseek,
	.release = single_release,
};
#endif

int sen_proc_init(char *module_name, DUMP_CB dump_cb)
{
	int ret = 0;
	SEN_DRV_INFO *pdrv_info = &drv_info;
	struct proc_dir_entry *pmodule_root = NULL;
	struct proc_dir_entry *pentry = NULL;

	pdrv_info->dump_cb = dump_cb;
	strncpy(pdrv_info->module_name, module_name, sizeof(pdrv_info->module_name) -1);

	pmodule_root = proc_mkdir(module_name, NULL);
	if (pmodule_root == NULL) {
		DBG_ERR("failed to create Module root \n");
		ret = -EINVAL;
		return ret;
	}
	pdrv_info->pproc_module_root = pmodule_root;

	// cmd
	pentry = proc_create("cmd", S_IRUGO | S_IXUGO, pmodule_root, &sen_cmd_proc_fops);
	if (pentry == NULL) {
		DBG_ERR("failed to create proc cmd \r\n");
		ret = -EINVAL;
		goto remove_root;
	}
	pdrv_info->pproc_cmd_entry = pentry;
	return ret;

remove_root:
	proc_remove(pdrv_info->pproc_module_root);
	return ret;
}

void sen_proc_exit(void)
{
	SEN_DRV_INFO *pdrv_info = &drv_info;
	proc_remove(pdrv_info->pproc_cmd_entry);
	if (pdrv_info->pproc_module_root) {
		proc_remove(pdrv_info->pproc_module_root);
	}
}


