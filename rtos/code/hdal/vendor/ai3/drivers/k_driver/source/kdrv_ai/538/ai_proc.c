#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>
#include "kdrv_ai_dbg.h"
#include "ai_proc.h"
#include "ai_main.h"
#include "ai_api.h"

//============================================================================
// Define
//============================================================================
#define MAX_CMD_LENGTH  30
#define MAX_ARG_NUM     6

//============================================================================
// Declaration
//============================================================================
typedef struct proc_cmd {
	char cmd[MAX_CMD_LENGTH];
	int (*execute)(PAI_INFO pdrv, unsigned char argc, char **argv);
} PROC_CMD, *PPROC_CMD;

struct proc_dir_info_t {
	char *name;
	struct proc_dir_entry *entry;
};

struct proc_file_info_t {
	char *name;
	struct proc_dir_entry *entry;
	struct proc_ops *op;

	struct proc_dir_info_t *parent;
};

//============================================================================
// Global variable
//============================================================================
PAI_DRV_INFO pdrv_info_data;

//============================================================================
// Function define
//============================================================================
extern int nvt_ai_api_read_version(PAI_INFO pmodule_info, unsigned char argc, char **pargv);
extern int nvt_kdrv_ai_func_test(PAI_INFO pmodule_info, unsigned char argc, char **pargv);

//=============================================================================
// proc "Custom Command" file operation functions
//=============================================================================
static PROC_CMD cmd_read_list[] = {
	// keyword          function name
#if !defined(CONFIG_NVT_SMALL_HDAL)
	{ "reg",            nvt_ai_api_read_reg           }, // read reg
#endif
	{ "version",        nvt_ai_api_read_version       },  // read kdrv version
};

#define NUM_OF_READ_CMD (sizeof(cmd_read_list) / sizeof(PROC_CMD))

static PROC_CMD cmd_write_list[] = {
	// keyword          function name
#if !defined(CONFIG_NVT_SMALL_HDAL)
	{ "reg",            nvt_ai_api_write_reg            },
	{ "pattern",        nvt_ai_api_write_pattern        },
	{ "kdrv",           nvt_kdrv_ai_api_test        	},
#endif
};



#define NUM_OF_WRITE_CMD (sizeof(cmd_write_list) / sizeof(PROC_CMD))

#if defined(__LINUX)
int ai_drv_uti_proc_buffer_to_int(const char __user *buffer, size_t count, int *dst, int dst_cnt)
{
#define PARAM_LENGTH 100

	int var_sum, n, ofs, hex_flag;
	char buf[PARAM_LENGTH], *scan_pos, *end_pos;

	if ((count > PARAM_LENGTH) || (count < 2)) {
		nvt_dbg(ERR, "cmd length(%zd) must <%d and >2\n", count, PARAM_LENGTH);
		return -1;
	}

	memset(buf, 0, PARAM_LENGTH);
	if (copy_from_user(buf, buffer, PARAM_LENGTH)) {
		nvt_dbg(ERR, "convert fail1\n");
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
			nvt_dbg(ERR, "convert fail2\n");
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

static int ai_drv_proc_register_cmd(struct proc_dir_entry *root, struct proc_dir_info_t *pdir, struct proc_file_info_t *pfile)
{
	int ret;
	struct proc_dir_info_t *dir_info;
	struct proc_file_info_t *file_info;

	dir_info = pdir;
	while(dir_info->name != NULL) {
		dir_info->entry = proc_mkdir_data(dir_info->name, S_IFDIR | S_IRUGO | S_IXUGO, root, NULL);

		if (dir_info->entry == NULL) {
			nvt_dbg(ERR, "proc_mkdir_data fail %s\n", dir_info->name);
		    ret = -EINVAL;
			goto dir_err;

		}
		dir_info += 1;
	}

	file_info = pfile;
	while(file_info->name != NULL) {

		if (file_info->parent == NULL) {
			file_info->entry = proc_create_data(file_info->name, S_IRUGO | S_IXUGO, root, file_info->op, NULL);
		} else {
			file_info->entry = proc_create_data(file_info->name, S_IRUGO | S_IXUGO, file_info->parent->entry, file_info->op, NULL);
		}

		if (file_info->entry == NULL) {
			nvt_dbg(ERR, "proc_create_data fail %s\n", file_info->name);
		    ret = -EINVAL;
			goto file_err;

		}
		file_info += 1;
	}
	return 0;

file_err:
	file_info = pfile;
	while(file_info->name != NULL) {
		if (file_info->entry) {
			proc_remove(file_info->entry);
		}
		file_info += 1;
	}

dir_err:
	dir_info = pdir;
	while(dir_info->name != NULL) {
		if (dir_info->entry) {
			proc_remove(dir_info->entry);
		}
		dir_info += 1;
	}
	return ret;
}

static int ai_drv_proc_unregister_cmd(struct proc_dir_info_t *pdir, struct proc_file_info_t *pfile)
{
	struct proc_dir_info_t *dir_info;
	struct proc_file_info_t *file_info;

	file_info = pfile;
	while(file_info->name != NULL) {
		if (file_info->entry) {
			proc_remove(file_info->entry);
		}
		file_info += 1;
	}

	dir_info = pdir;
	while(dir_info->name != NULL) {
		if (dir_info->entry) {
			proc_remove(dir_info->entry);
		}
		dir_info += 1;
	}

	return 0;
}


static int nvt_ai_proc_cmd_show(struct seq_file *sfile, void *v)
{
	if ((sfile == NULL) || (v == NULL)) {
		nvt_dbg(ERR, "invalid input\n");
		return -EINVAL;
	}

	return 0;
}

static int nvt_ai_proc_cmd_open(struct inode *inode, struct file *file)
{
	if ((inode == NULL) || (file == NULL)) {
		nvt_dbg(ERR, "invalid input\n");
		return -EINVAL;
	}

	return single_open(file, nvt_ai_proc_cmd_show, &pdrv_info_data->module_info);
}

static ssize_t nvt_ai_proc_cmd_write(struct file *file, const char __user *buf, size_t size, loff_t *off)
{
	int len = size;
	int ret = -EINVAL;
	char cmd_line[MAX_CMD_LENGTH];
	char *cmdstr = cmd_line;
	const char delimiters[] = {' ', 0x0A, 0x0D, '\0'};
	char *argv[MAX_ARG_NUM] = {0};
	unsigned char ucargc = 0;
	unsigned char loop;

	if ((file == NULL) || (buf == NULL) || (off == NULL)) {
		nvt_dbg(ERR, "invalid input\n");
		return -EINVAL;
	}

	// check command length
	if (len > (MAX_CMD_LENGTH - 1)) {
		nvt_dbg(ERR, "Command length is too long!\n");
		goto ERR_OUT;
	}
	if (len == 0) {
		nvt_dbg(ERR, "Command length is zero!\n");
		goto ERR_OUT;
	}

	// copy command string from user space
	if (copy_from_user(cmd_line, buf, len)) {
		goto ERR_OUT;
	}

	cmd_line[len - 1] = '\0';

	nvt_dbg(IND, "CMD:%s\n", cmd_line);

	// parse command string
	for (ucargc = 0; ucargc < MAX_ARG_NUM; ucargc++) {
		argv[ucargc] = strsep(&cmdstr, delimiters);

		if (argv[ucargc] == NULL) {
			break;
		}
	}

	// dispatch command handler
	if (strncmp(argv[0], "r", 2) == 0) {
		for (loop = 0 ; loop < NUM_OF_READ_CMD; loop++) {
			if (strncmp(argv[1], cmd_read_list[loop].cmd, MAX_CMD_LENGTH) == 0) {
				ret = cmd_read_list[loop].execute(&pdrv_info_data->module_info, ucargc - 2, &argv[2]);
				break;
			}
		}
		if (loop >= NUM_OF_READ_CMD) {
			goto ERR_INVALID_CMD;
		}
	} else if (strncmp(argv[0], "w", 2) == 0)  {

		for (loop = 0 ; loop < NUM_OF_WRITE_CMD ; loop++) {
			if (strncmp(argv[1], cmd_write_list[loop].cmd, MAX_CMD_LENGTH) == 0) {
				ret = cmd_write_list[loop].execute(&pdrv_info_data->module_info, ucargc - 2, &argv[2]);
				break;
			}
		}

		if (loop >= NUM_OF_WRITE_CMD) {
			goto ERR_INVALID_CMD;
		}

	} else {
		goto ERR_INVALID_CMD;
	}

	return size;

ERR_INVALID_CMD:
	nvt_dbg(ERR, "Invalid CMD \"%s\"\n", cmd_line);

ERR_OUT:
	return -1;
}

static struct proc_ops proc_cmd_fops = {
	.proc_open    = nvt_ai_proc_cmd_open,
	.proc_read    = seq_read,
	.proc_lseek   = seq_lseek,
	.proc_release = single_release,
	.proc_write   = nvt_ai_proc_cmd_write
};

//=============================================================================
// proc "help" file operation functions
//=============================================================================
static int nvt_ai_proc_help_show(struct seq_file *sfile, void *v)
{
	if ((sfile == NULL) || (v == NULL)) {
		nvt_dbg(ERR, "invalid input\n");
		return -EINVAL;
	}
	
	return 0;
}

static int nvt_ai_proc_help_open(struct inode *inode, struct file *file)
{
	if ((inode == NULL) || (file == NULL)) {
		nvt_dbg(ERR, "invalid input\n");
		return -EINVAL;
	}

	return single_open(file, nvt_ai_proc_help_show, NULL);
}

static struct proc_ops proc_help_fops = {
	.proc_open   = nvt_ai_proc_help_open,
	.proc_release = single_release,
	.proc_read   = seq_read,
	.proc_lseek = seq_lseek,
};

//=============================================================================
// proc "kdrv_ai/util" file operation functions
//=============================================================================
static int ai_drv_proc_util_show(struct seq_file *sfile, void *v)
{
    //ive_drv_ctl_dump_util(sfile);
    return 0;
}

static int ai_drv_proc_util_open(struct inode *inode, struct file *file)
{
    return single_open(file, ai_drv_proc_util_show, NULL);
}

static struct proc_ops proc_util_fops = {
    .proc_open    = ai_drv_proc_util_open,
    .proc_read    = seq_read,
    .proc_lseek   = seq_lseek,
    .proc_release = single_release,
};


//=============================================================================
// proc "kdrv_ai/gating_en" file operation functions
//=============================================================================
static int ai_gating_en = 1;
static ssize_t ai_drv_proc_gating_write(struct file *file, const char __user *buffer, size_t count, loff_t *ppos)
{
#define DATA_SIZE 1
	int data[DATA_SIZE] = {0};
	int i = 0;
	int flag = 0;
	
	if (ai_drv_uti_proc_buffer_to_int(buffer, count, &data[0], DATA_SIZE) != DATA_SIZE) {
		nvt_dbg(ERR, "input parameter error\n");
		return -1;
	}
	if (data[0] > 0) {
		flag = 1;
	}
	for (i = 0; i < MODULE_CLK_NUM; i++) {
		kdrv_ai_set_gating(flag, i);
	}
	ai_gating_en = flag;
#undef DATA_SIZE	
	return count;
}

static int ai_drv_proc_gating_show(struct seq_file *sfile, void *v)
{
    seq_printf(sfile, "command: echo <1:enable, 0:disable>\n");
	seq_printf(sfile, "gating_en = %d\n", ai_gating_en);
    return 0;
}

static int ai_drv_proc_gating_open(struct inode *inode, struct file *file)
{
    return single_open(file, ai_drv_proc_gating_show, NULL);
}

static struct proc_ops proc_gating_fops = {
    .proc_open    = ai_drv_proc_gating_open,
    .proc_read    = seq_read,
    .proc_lseek   = seq_lseek,
    .proc_release = single_release,
    .proc_write   = ai_drv_proc_gating_write
};

//=============================================================================
// proc "kdrv_ai/dbglevel" file operation functions
//=============================================================================
static ssize_t ai_drv_proc_dbg_lvl_write(struct file *file, const char __user *buffer, size_t count, loff_t *ppos)
{
#define DATA_SIZE 1
	int data[DATA_SIZE] = {0};

	if (ai_drv_uti_proc_buffer_to_int(buffer, count, &data[0], DATA_SIZE) != DATA_SIZE) {
		nvt_dbg(ERR, "input parameter error\n");
		return -1;
	}

	if (data[0] >= 0) {
		//kdrv_ive_debug_level = data[0];
	}

#undef DATA_SIZE
	return count;
}

static int ai_drv_proc_dbg_lvl_show(struct seq_file *sfile, void *v)
{
	//ive_drv_proc_dump_dbg_lvl(sfile);
    return 0;
}

static int ai_drv_proc_dbg_lvl_open(struct inode *inode, struct file *file)
{
    return single_open(file, ai_drv_proc_dbg_lvl_show, NULL);
}

static struct proc_ops proc_dbg_lvl_fops = {
    .proc_open    = ai_drv_proc_dbg_lvl_open,
    .proc_read    = seq_read,
    .proc_lseek   = seq_lseek,
    .proc_release = single_release,
    .proc_write   = ai_drv_proc_dbg_lvl_write
};
//=============================================================================
// proc "nue2/gating_en" file operation functions
//=============================================================================
/*static int proc_nue2_gating_show(struct seq_file *sfile, void *v)
{
    seq_printf(sfile, "kdrv_ai nue2 hw gating clock = %d\n", kdrv_ai_get_gating(1));
    return 0;
}
static int proc_nue2_gating_open(struct inode *inode, struct file *file)
{
    return single_open(file, proc_nue2_gating_show, NULL);
}
static ssize_t proc_nue2_gating_write(struct file *file, const char __user * buffer, size_t count, loff_t *ppos)
{
    char proc_buffer[64];
	int value = 0;
	
    if (count > sizeof(proc_buffer) - 1) {
        nvt_dbg(ERR, "input is too large\n");
        return -EINVAL;
    }
    if (copy_from_user(proc_buffer, buffer, count)) {
        nvt_dbg(ERR, "proc clk copy from user failed\n");
        return -EFAULT;
    }
    proc_buffer[count] = '\0';
    sscanf(proc_buffer, "%d", &value);

  	kdrv_ai_set_gating(value,1);
	
    return count;
}

static struct proc_ops nue2_gating_proc_ops = {
    .proc_open = proc_nue2_gating_open,
    .proc_read = seq_read,
    .proc_lseek = seq_lseek,
    .proc_release = single_release,
    .proc_write = proc_nue2_gating_write
};*/

static struct proc_file_info_t proc_file_info_tab[] = {
	{"cmd", NULL, &proc_cmd_fops, NULL},
	{"utilization", NULL, &proc_util_fops, NULL},
	{"gating_en", NULL, &proc_gating_fops, NULL},
	{"dbglevel", NULL, &proc_dbg_lvl_fops, NULL},
	{NULL, NULL, NULL, NULL}
};

static struct proc_dir_info_t proc_dir_info_tab[] = {
	{NULL, NULL}
};

int nvt_ai_proc_init(PAI_DRV_INFO pdrv_info)
{
	int ret = 0;
	struct proc_dir_entry *pmodule_root = NULL;
	//struct proc_dir_entry *pmodule_nue2 = NULL;
	//struct proc_dir_entry *pmodule_nue2_gating_proc = NULL;
	struct proc_dir_entry *pentry = NULL;

	if (pdrv_info == NULL) {
		nvt_dbg(ERR, "invalid input\n");
		return -EINVAL;
	}
	pmodule_root = proc_mkdir("kdrv_ai", NULL);
	if (pmodule_root == NULL) {
		nvt_dbg(ERR, "failed to create Module root\n");
		ret = -EINVAL;
		goto fail_init_proc;
	}
	pdrv_info->pproc_module_root = pmodule_root;
	
	if (ai_drv_proc_register_cmd(pdrv_info->pproc_module_root, proc_dir_info_tab, proc_file_info_tab) < 0) {
        ret = -EINVAL;
		goto fail_init_proc;
	}
	/*
	pmodule_nue2 = proc_mkdir("nue2", pmodule_root);
	if (pmodule_nue2 == NULL) {
		nvt_dbg(ERR, "failed to create kdrv_ai/nue2 proc\n");
		ret = -EINVAL;
		goto fail_init_proc;
	}
	pdrv_info->pproc_module_nue2 = pmodule_nue2;
	*/
	/*pentry = proc_create("cmd", S_IRUGO | S_IXUGO, pmodule_root, &proc_cmd_fops);
	if (pentry == NULL) {
		nvt_dbg(ERR, "failed to create proc cmd!\n");
		ret = -EINVAL;
		goto fail_init_proc;
	}
	pdrv_info->pproc_cmd_entry = pentry;*/

	pentry = proc_create("help", S_IRUGO | S_IXUGO, pmodule_root, &proc_help_fops);
	if (pentry == NULL) {
		nvt_dbg(ERR, "failed to create proc help!\n");
		ret = -EINVAL;
		goto fail_init_proc;
	}
	pdrv_info->pproc_help_entry = pentry;
	/*
	pmodule_nue2_gating_proc = proc_create("gating_en", S_IRUGO | S_IXUGO, pmodule_nue2, &nue2_gating_proc_ops);
	if (NULL == pmodule_nue2_gating_proc) {
		nvt_dbg(ERR, "Error to create kdrv_ai/nue2/gating_en proc\n");
		goto fail_init_proc;
	}
	pdrv_info->pproc_module_nue2_gating_proc = pmodule_nue2_gating_proc;
	*/
	pdrv_info_data = pdrv_info;

	return ret;

fail_init_proc:
	ai_drv_proc_unregister_cmd(proc_dir_info_tab, proc_file_info_tab);
	nvt_ai_proc_remove(pdrv_info);
	return ret;
}

int nvt_ai_proc_remove(PAI_DRV_INFO pdrv_info)
{
	if (pdrv_info == NULL) {
		nvt_dbg(ERR, "invalid input\n");
		return -EINVAL;
	}
	ai_drv_proc_unregister_cmd(proc_dir_info_tab, proc_file_info_tab);
	if(pdrv_info->pproc_help_entry)proc_remove(pdrv_info->pproc_help_entry);
	//if(pdrv_info->pproc_cmd_entry)proc_remove(pdrv_info->pproc_cmd_entry);
	if(pdrv_info->pproc_module_root)proc_remove(pdrv_info->pproc_module_root);
	if(pdrv_info->pproc_module_nue2)proc_remove(pdrv_info->pproc_module_nue2);
	if(pdrv_info->pproc_module_nue2_gating_proc)proc_remove(pdrv_info->pproc_module_nue2_gating_proc);
	return 0;
}
