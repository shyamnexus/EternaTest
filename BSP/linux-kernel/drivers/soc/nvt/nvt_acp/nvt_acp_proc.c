#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/uaccess.h>
#include <linux/platform_device.h>
#include <linux/kernel.h>

#define PROC_CMD_LENGTH 64
#define PROC_ARG_NUM     6

struct device *acp_dev;
extern void nvtacp_dump_stack(void);

static struct proc_dir_entry *pg_proc_root = NULL;
static struct proc_dir_entry *pg_proc_cmd = NULL;
static struct proc_dir_entry *pg_proc_help = NULL;

// definition -----------
typedef struct proc_cmd {
	char cmd[PROC_CMD_LENGTH];
	int (*pfunc)(int argc, char **argv);
} PROC_CMD;

int proc_func_acp_dump(int argc, char **argv)
{
	if (argc != 1) {
		dev_err(acp_dev, "Wrong input parameter !!!\n");
		return -EINVAL;
	}

	nvtacp_dump_stack();

	return 0;
}

static PROC_CMD g_proc_cmd_list[] = {
	{"dump", proc_func_acp_dump},
};

static int proc_show_cmd(struct seq_file *sfile, void *v)
{
	unsigned int cmd_idx;

	dev_err(acp_dev, "echo xxx > cmd\n");
	for (cmd_idx = 0 ; cmd_idx < ARRAY_SIZE(g_proc_cmd_list); cmd_idx++) {
		dev_err(acp_dev, "%s\n", g_proc_cmd_list[cmd_idx].cmd);
	}
	return 0;
}

static int proc_open_cmd(struct inode *inode, struct file *file)
{
	return single_open(file, proc_show_cmd, NULL);
}

static ssize_t proc_write_cmd(struct file *file, const char __user *buf,
			      size_t size, loff_t *off)
{
	char cmd_line[PROC_CMD_LENGTH];
	const char delimiters[] = {' ', '\r', '\n', '\0'};
	char *argv[PROC_ARG_NUM] = {0};
	unsigned char argc = 0;

	char *cmdstr = cmd_line;
	unsigned int cmd_idx;
	int ret = -EINVAL;

	// Note: In fact, size will never be smaller than 1 in proc cmd
	//       to prevent coverity warning, we still check the size < 1
	// e.g. cmd [abcde], size = 6
	// e.g. cmd [], size = 1
	if (size > PROC_CMD_LENGTH || size < 1) {
		dev_err(acp_dev, "Invalid cmd size %ld\n", (unsigned long)size);
		goto ERR_OUT;
	}

	// copy command string from user space
	if (copy_from_user(cmd_line, buf, size))
		goto ERR_OUT;

	cmd_line[size - 1] = '\0';

	pr_debug("PROC_CMD: [%s], size %ld\n", cmd_line, (unsigned long)size);

	// parse command string
	for (argc = 0; argc < PROC_ARG_NUM; argc++) {
		argv[argc] = strsep(&cmdstr, delimiters);

		if (argv[argc] == NULL)
			break;
	}

	// dispatch command handler
	for (cmd_idx = 0 ; cmd_idx < ARRAY_SIZE(g_proc_cmd_list); cmd_idx++) {
		if (strncmp(argv[0], g_proc_cmd_list[cmd_idx].cmd, strlen(g_proc_cmd_list[cmd_idx].cmd)) == 0) {
			ret = g_proc_cmd_list[cmd_idx].pfunc(argc, argv);
			break;
		}
	}
	if (cmd_idx >= ARRAY_SIZE(g_proc_cmd_list))
		goto ERR_INVALID_CMD;

	return size;

ERR_INVALID_CMD:
	dev_err(acp_dev, "Invalid CMD [%s]\n", cmd_line);

ERR_OUT:
	return -EINVAL;
}

static const struct proc_ops proc_fops_cmd = {
	.proc_open    = proc_open_cmd,
	.proc_read    = seq_read,
	.proc_write   = proc_write_cmd,
	.proc_lseek   = seq_lseek,
	.proc_release = single_release,
};

static int proc_show_help(struct seq_file *sfile, void *v)
{
	seq_printf(sfile,"\
			==== Dump ACP history ==== \n \
			echo dump > /proc/nvt_acp/cmd\n");
	return 0;
}

static int proc_open_help(struct inode *inode, struct file *file)
{
	return single_open(file, proc_show_help, NULL);
}

static const struct proc_ops proc_fops_help = {
	.proc_open    = proc_open_help,
	.proc_read    = seq_read,
	.proc_lseek   = seq_lseek,
	.proc_release = single_release,
};

int nvtacp_proc_init(struct device *dev)
{
	acp_dev = dev;

	pg_proc_root = proc_mkdir("nvt_acp", NULL);
	if (NULL == pg_proc_root) {
		dev_err(acp_dev, "proc_mkdir\n");
		goto proc_init_err;
	}

	pg_proc_cmd = proc_create("cmd", S_IRUGO | S_IXUGO, pg_proc_root, &proc_fops_cmd);
	if (NULL == pg_proc_cmd) {
		dev_err(acp_dev, "proc_create cmd\n");
		goto proc_init_err;
	}

	pg_proc_help = proc_create("help", S_IRUGO | S_IXUGO, pg_proc_root, &proc_fops_help);
	if (NULL == pg_proc_help) {
		dev_err(acp_dev, "proc_create help\n");
		goto proc_init_err;
	}

	return 0;

proc_init_err:
	if (pg_proc_help) {
		proc_remove(pg_proc_help);
		pg_proc_help = NULL;
	}

	if (pg_proc_cmd) {
		proc_remove(pg_proc_cmd);
		pg_proc_cmd = NULL;
	}

	if (pg_proc_root) {
		proc_remove(pg_proc_root);
		pg_proc_root = NULL;
	}

	return -1;
}

void nvtacp_proc_exit(void)
{
	if (pg_proc_help) {
		proc_remove(pg_proc_help);
		pg_proc_help = NULL;
	}

	if (pg_proc_cmd) {
		proc_remove(pg_proc_cmd);
		pg_proc_cmd = NULL;
	}

	if (pg_proc_root) {
		proc_remove(pg_proc_root);
		pg_proc_root = NULL;
	}
}
