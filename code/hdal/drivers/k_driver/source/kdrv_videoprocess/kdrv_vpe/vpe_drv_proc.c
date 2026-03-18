/**
 * @file vpe_drv_proc.c
 *  vpe_drv proc interface
 *
 *
 */

#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/string.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <asm/uaccess.h>
#include <linux/uaccess.h>
#include "vpe_drv_dbg_int.h"
#include "vpe_drv_util_int.h"
#include "vpe_drv_ctl_int.h"
#include "vpe_drv_proc_int.h"
#include "vpe_drv_proc_api_int.h"

#define MAX_CMD_LENGTH 100
#define MAX_ARG_NUM     10

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

static struct proc_dir_entry *pproc_module_root;

static int vpe_drv_proc_cmd_show(struct seq_file *sfile, void *v)
{
    char *argv[1] = {"?"};
	vpe_drv_cmd_execute(1, &argv[0]);
    return 0;
}

static int vpe_drv_proc_cmd_open(struct inode *inode, struct file *file)
{
    return single_open(file, vpe_drv_proc_cmd_show, NULL);
}

static ssize_t vpe_drv_proc_cmd_write(struct file *file, const char __user *buf,
                                  size_t size, loff_t *off)
{
    int len = size;
    int ret;
    char cmd_line[MAX_CMD_LENGTH];
    char *cmdstr = cmd_line;
    const char delimiters[] = {' ', 0x0A, 0x0D, '\0'};
    char *argv[MAX_ARG_NUM] = {0};
    unsigned char ucargc = 0;

    // check command length
    if (!len || (len > (MAX_CMD_LENGTH - 1))) {
        vpe_drv_err("Command length is zero or too long!\n");
        goto ERR_OUT;
    }

    // copy command string from user space
    if (copy_from_user(cmd_line, buf, len))
        goto ERR_OUT;

    cmd_line[len - 1] = '\0';

	vpe_drv_info("CMD:%s\n", cmd_line);

    // parse command string
    for (ucargc = 0; ucargc < MAX_ARG_NUM; ucargc++) {
        argv[ucargc] = strsep(&cmdstr, delimiters);

        if (argv[ucargc] == NULL)
            break;
    }

    // dispatch command handler
    if (ucargc > 0) {
		ret = vpe_drv_cmd_execute(ucargc, &argv[0]);
		if (ret == 0) {
			goto ERR_INVALID_CMD;
		}
    } else {
		goto ERR_INVALID_CMD;
    }

    return size;

ERR_INVALID_CMD:
    vpe_drv_err("Invalid CMD \"%s\"\n", cmd_line);

ERR_OUT:
    return -1;
}

static struct proc_ops proc_cmd_fops = {
    .proc_open    = vpe_drv_proc_cmd_open,
    .proc_read    = seq_read,
    .proc_lseek   = seq_lseek,
    .proc_release = single_release,
    .proc_write   = vpe_drv_proc_cmd_write
};

#if 0
#endif

static int vpe_drv_proc_util_show(struct seq_file *sfile, void *v)
{
    vpe_drv_ctl_dump_util(sfile);
    return 0;
}

static int vpe_drv_proc_util_open(struct inode *inode, struct file *file)
{
    return single_open(file, vpe_drv_proc_util_show, NULL);
}

static struct proc_ops proc_util_fops = {
    .proc_open    = vpe_drv_proc_util_open,
    .proc_read    = seq_read,
    .proc_lseek   = seq_lseek,
    .proc_release = single_release,
};

#if 0
#endif

static ssize_t vpe_drv_proc_gating_write(struct file *file, const char __user *buffer, size_t count, loff_t *ppos)
{
#define DATA_SIZE 1
	int data[DATA_SIZE] = {0};

	if (vpe_drv_uti_proc_buffer_to_int(buffer, count, &data[0], DATA_SIZE) != DATA_SIZE) {
		vpe_drv_err("input parameter error\n");
		return -1;
	}

	if (data[0] <= 0) {
		vpe_drv_ctl_set_gating(0);
	} else {
		vpe_drv_ctl_set_gating(1);
	}

#undef DATA_SIZE
	return count;
}


static int vpe_drv_proc_gating_show(struct seq_file *sfile, void *v)
{
    seq_printf(sfile, "command: echo <1:enable, 0:disable>\n");
    vpe_drv_ctl_dump_gating(sfile);
    return 0;
}

static int vpe_drv_proc_gating_open(struct inode *inode, struct file *file)
{
    return single_open(file, vpe_drv_proc_gating_show, NULL);
}

static struct proc_ops proc_gating_fops = {
    .proc_open    = vpe_drv_proc_gating_open,
    .proc_read    = seq_read,
    .proc_lseek   = seq_lseek,
    .proc_release = single_release,
    .proc_write   = vpe_drv_proc_gating_write
};

#if 0
#endif
static ssize_t vpe_drv_proc_dbg_lvl_write(struct file *file, const char __user *buffer, size_t count, loff_t *ppos)
{
#define DATA_SIZE 1
	int data[DATA_SIZE] = {0};

	if (vpe_drv_uti_proc_buffer_to_int(buffer, count, &data[0], DATA_SIZE) != DATA_SIZE) {
		vpe_drv_err("input parameter error\n");
		return -1;
	}

	if (data[0] >= 0) {
		kdrv_vpe_debug_level = data[0];
	}

#undef DATA_SIZE
	return count;
}

static int vpe_drv_proc_dbg_lvl_show(struct seq_file *sfile, void *v)
{
	vpe_drv_proc_dump_dbg_lvl(sfile);
    return 0;
}

static int vpe_drv_proc_dbg_lvl_open(struct inode *inode, struct file *file)
{
    return single_open(file, vpe_drv_proc_dbg_lvl_show, NULL);
}

static struct proc_ops proc_dbg_lvl_fops = {
    .proc_open    = vpe_drv_proc_dbg_lvl_open,
    .proc_read    = seq_read,
    .proc_lseek   = seq_lseek,
    .proc_release = single_release,
    .proc_write   = vpe_drv_proc_dbg_lvl_write
};

#if 0
#endif

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

static int vpe_drv_proc_register_cmd(struct proc_dir_entry *root, struct proc_dir_info_t *pdir, struct proc_file_info_t *pfile)
{
	int ret;
	struct proc_dir_info_t *dir_info;
	struct proc_file_info_t *file_info;

	dir_info = pdir;
	while(dir_info->name != NULL) {
		dir_info->entry = proc_mkdir_data(dir_info->name, S_IFDIR | S_IRUGO | S_IXUGO, root, NULL);

		if (dir_info->entry == NULL) {
			vpe_drv_err("proc_mkdir_data fail %s\n", dir_info->name);
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
			vpe_drv_err("proc_create_data fail %s\n", file_info->name);
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

static int vpe_drv_proc_unregister_cmd(struct proc_dir_info_t *pdir, struct proc_file_info_t *pfile)
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


int vpe_drv_proc_register(void)
{
    int ret = 0;
    struct proc_dir_entry *pmodule_root = NULL;

	/* register root */
	pmodule_root = proc_mkdir(VPE_DRV_PROC_PATH, NULL);
	if(pmodule_root == NULL) {
		vpe_drv_err("failed to create Module root\n");
        ret = -EINVAL;
		return ret;
	}
	pproc_module_root = pmodule_root;

	if (vpe_drv_proc_register_cmd(pproc_module_root, proc_dir_info_tab, proc_file_info_tab) < 0) {
        ret = -EINVAL;
		goto err;
	}
	return ret;

err:
    vpe_drv_err("failed to create proc cmd!\n");
	vpe_drv_proc_unregister();
	return ret;
}

void vpe_drv_proc_unregister(void)
{
	vpe_drv_proc_unregister_cmd(proc_dir_info_tab, proc_file_info_tab);

	if (pproc_module_root) {
		proc_remove(pproc_module_root);
		pproc_module_root = 0;
	}
}

void vpe_drv_proc_seq_printf(void *sfile, const char *fmt, ...)
{
#define MAX_MSG_SIZE 256
	va_list args;
	char msg[MAX_MSG_SIZE];

	if (sfile == 0) {
		va_start(args, fmt);
		vsnprintf(msg, MAX_MSG_SIZE, fmt, args);
		va_end(args);
		nvt_vpe_dbg_log("%s", &msg[0]);
	} else {
		va_start(args, fmt);
		seq_vprintf(sfile, fmt, args);
		va_end(args);
	}
}