#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/uaccess.h>
#include <linux/platform_device.h>
#include <linux/kernel.h>
#include <linux/soc/nvt/nvt-timer.h>

#define PROC_CMD_LENGTH 64
#define PROC_ARG_NUM     6

static struct proc_dir_entry *pg_proc_root = NULL;
static struct proc_dir_entry *pg_proc_cmd = NULL;
static struct proc_dir_entry *pg_proc_help = NULL;

// definition -----------
typedef struct proc_cmd {
	char cmd[PROC_CMD_LENGTH];
	int (*pfunc)(int argc, char **argv);
} PROC_CMD;

int proc_test_callback(void)
{
	pr_info("Get Timer IRQ!!!\n");
	return 0;
}

int proc_func_start(int argc, char **argv)
{
	int ret;

	if (argc != 2) {
		pr_err("%s: Wrong input parameter !!!\n", __func__);
		return -EINVAL;
	}

	ret = nvt_timer_start(simple_strtol(argv[1], NULL, 10));
	return ret;
}

int proc_func_stop(int argc, char **argv)
{
	int ret;

	if (argc != 2) {
		pr_err("%s: Wrong input parameter !!!\n", __func__);
		return -EINVAL;
	}

	ret = nvt_timer_stop(simple_strtol(argv[1], NULL, 10));
	return ret;
}

int proc_func_setup(int argc, char **argv)
{
	int ret;

	if (argc != 4) {
		pr_err("%s: Wrong input parameter !!!\n", __func__);
		return -EINVAL;
	}

	ret = nvt_timer_setup(simple_strtol(argv[1], NULL, 10), simple_strtol(argv[2], NULL, 10), simple_strtol(argv[3], NULL, 10), &proc_test_callback);
	return ret;
}

int proc_func_shutdown(int argc, char **argv)
{
	int ret;

	if (argc != 2) {
		pr_err("%s: Wrong input parameter !!!\n", __func__);
		return -EINVAL;
	}

	ret = nvt_timer_shutdown(simple_strtol(argv[1], NULL, 10));
	return ret;
}

int proc_func_reload(int argc, char **argv)
{
	int ret;

	if (argc != 3) {
		pr_err("%s: Wrong input parameter !!!\n", __func__);
		return -EINVAL;
	}

	ret = nvt_timer_reload(simple_strtol(argv[1], NULL, 10), simple_strtol(argv[2], NULL, 10));
	return ret;
}

int proc_func_get_count(int argc, char **argv)
{
	int ret;
	u64 count;

	if (argc != 2) {
		pr_err("%s: Wrong input parameter !!!\n", __func__);
		return -EINVAL;
	}

	ret = nvt_timer_get_count(simple_strtol(argv[1], NULL, 10), &count);
	pr_info("%s: Get Timer count = %lld\n", __func__, count);

	return ret;
}

int proc_func_timer_dump(int argc, char **argv)
{
	if (argc != 1) {
		pr_err("%s: Wrong input parameter !!!\n", __func__);
		return -EINVAL;
	}

	nvt_timer_dump();
	return 0;
}

static PROC_CMD g_proc_cmd_list[] = {
	{"start", proc_func_start},
	{"stop", proc_func_stop},
	{"setup", proc_func_setup},
	{"shutdown", proc_func_shutdown},
	{"reload", proc_func_reload},
	{"get_count", proc_func_get_count},
	{"dump", proc_func_timer_dump},
};

static int proc_show_cmd(struct seq_file *sfile, void *v)
{
	unsigned int cmd_idx;

	pr_info("Usage: echo xxx > cmd\n");
	for (cmd_idx = 0 ; cmd_idx < ARRAY_SIZE(g_proc_cmd_list); cmd_idx++) {
		pr_info("%s\n", g_proc_cmd_list[cmd_idx].cmd);
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
		pr_err("Invalid cmd size %ld\n", (unsigned long)size);
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
	pr_err("Invalid CMD [%s]\n", cmd_line);

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
			==== Setup Timer ==== \n \
			echo setup [timer num] [oneshot(0)/freerun(1)] [count] > /proc/nvt-hwtimer/cmd \n \
			count = time * HZ[%d] \n \
			==== Start Timer ==== \n \
			echo start [timer num] > /proc/nvt-hwtimer/cmd\n \
			==== Reload Timer ==== \n \
			echo reload [timer num] > /proc/nvt-hwtimer/cmd\n \
			==== Get Timer Count ==== \n \
			echo get_count [timer num] > /proc/nvt-hwtimer/cmd\n \
			==== Stop Timer ==== \n \
			echo stop [timer num] > /proc/nvt-hwtimer/cmd\n \
			==== Shutdown Timer ==== \n \
			echo shutdown [timer num] > /proc/nvt-hwtimer/cmd\n", 0);
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

int nvt_timer_proc_init()
{
	pg_proc_root = proc_mkdir("nvt-hwtimer", NULL);
	if (NULL == pg_proc_root) {
		pr_err("proc_mkdir\n");
		goto proc_init_err;
	}

	pg_proc_cmd = proc_create("cmd", S_IRUGO | S_IXUGO, pg_proc_root, &proc_fops_cmd);
	if (NULL == pg_proc_cmd) {
		pr_err("proc_create cmd\n");
		goto proc_init_err;
	}

	pg_proc_help = proc_create("help", S_IRUGO | S_IXUGO, pg_proc_root, &proc_fops_help);
	if (NULL == pg_proc_help) {
		pr_err("proc_create help\n");
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

void nvt_timer_proc_exit(void)
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
