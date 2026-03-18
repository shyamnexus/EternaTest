#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/uaccess.h>
#include <linux/delay.h>
#include <plat/nvt_jiffies.h>
#include <kwrap/nvt_type.h>
#include "timer_main.h"
#include "timer_dbg.h"
#include "comm/timer.h"


//============================================================================
// Define
//============================================================================
#define time_disance(a, b)  ((int)(b) - (int)(a))

#define MAX_CMD_LENGTH  30
#define MAX_ARG_NUM      6

//============================================================================
// Local variable
//============================================================================
static unsigned long __nvt_jiffies = 0, __nvt_jiffies_us = 0;  /* 1ms granularity and 1us granularity */
static u64 __nvt_jiffies_u64 = 0;                              /* 1ms granularity */
static unsigned int __nvt_jiffies_u32 = 0;                     /* 1ms granularity */
static unsigned long clk_khz = 1000, clk_mhz = 1;              /* default 1MHz */

DEFINE_SPINLOCK(spinlock);
static unsigned int refer_cnt = 0;
static unsigned int tmp, distance_ms = 0, distance_us = 0;

//============================================================================
// Export function
//============================================================================
/* get 1m jiffies */
unsigned long get_nvt_jiffies(void)
{
	static unsigned int keep_clk_val = 0;
	unsigned int new_clk_val;
	unsigned long flags;
	unsigned long value_us, value_ms;

	/* lock */
	spin_lock_irqsave(&spinlock, flags);
	new_clk_val = timer_get_current_count(timer_get_sys_timer_id());
	if (!keep_clk_val && !new_clk_val) {  /* not start yet */
		spin_unlock_irqrestore(&spinlock, flags);
		return 0;
	}

	tmp = time_disance(keep_clk_val, new_clk_val);
	distance_ms += tmp;
	distance_us += tmp;
	keep_clk_val = new_clk_val;

	if (distance_ms >= clk_khz) {
		value_ms = distance_ms / clk_khz;

		__nvt_jiffies += value_ms;
		__nvt_jiffies_u64 += value_ms;
		__nvt_jiffies_u32 += value_ms;
		distance_ms = distance_ms % clk_khz;
	}

	if (distance_us >= clk_mhz) {
		value_us = distance_us / clk_mhz;

		__nvt_jiffies_us += value_us;
		distance_us = distance_us % clk_mhz;
	}

	/* unlock */
	spin_unlock_irqrestore(&spinlock, flags);

	refer_cnt++;

	return __nvt_jiffies;
}
EXPORT_SYMBOL(get_nvt_jiffies);

/* get 1us jiffies */
unsigned long get_nvt_jiffies_us(void)
{
	get_nvt_jiffies();

	return __nvt_jiffies_us;
}
EXPORT_SYMBOL(get_nvt_jiffies_us);

/* get 1ms jiffies */
unsigned int get_nvt_jiffies_u32(void)
{
        get_nvt_jiffies();

        return __nvt_jiffies_u32;
}
EXPORT_SYMBOL(get_nvt_jiffies_u32);

/* get 1ms jiffies */
u64 get_nvt_jiffies_u64(void)
{
	get_nvt_jiffies();

	return __nvt_jiffies_u64;
}
EXPORT_SYMBOL(get_nvt_jiffies_u64);

//=============================================================================
// proc "Custom Command" file operation functions
//=============================================================================
static int proc_read_nvt_jiffies(struct seq_file *sfile, void *v)
{
	seq_printf(sfile, "nvt jiffies: 0x%x, HZ = %d \n", (u32)nvt_jiffies, (int)(clk_khz * 1000));
	seq_printf(sfile, "reference count: %d \n", refer_cnt);

	return 0;
}

static int nvt_jiffies_proc_cmd_open(struct inode *inode, struct file *file)
{
	return single_open(file, proc_read_nvt_jiffies, NULL);
}

static void nvt_jiffies_test(void)
{
	unsigned long jiffies_0 = 0, jiffies_1 = 0;
	unsigned long jiffies_us_0 = 0, jiffies_us_1 = 0;
	int delay_ms = 0;

	// delay 500 ms
	delay_ms = 500;
	jiffies_0 = get_nvt_jiffies();
	jiffies_us_0 = __nvt_jiffies_us;
	printk("Test jiffies_0: %d\n", (int)jiffies_0);
	mdelay(delay_ms);
	jiffies_1 = get_nvt_jiffies();
	jiffies_us_1 = __nvt_jiffies_us;
	printk("Test jiffies_1: %d\n", (int)jiffies_1);
	printk("jiffies_1 - jiffies_0: %d ms , expected spend time: %d ms \n", (int)(jiffies_1 - jiffies_0), (int)delay_ms);
	printk("jiffies_us_1 - jiffies_us_0: %d us , expected spend time: %d us \n", (int)(jiffies_us_1 - jiffies_us_0), (int)(delay_ms * 1000));

	// delay 5000 ms
	delay_ms = 5000;
	jiffies_0 = get_nvt_jiffies();
	jiffies_us_0 = __nvt_jiffies_us;
	printk("Test jiffies_0: %d\n", (int)jiffies_0);
	mdelay(delay_ms);
	jiffies_1 = get_nvt_jiffies();
	jiffies_us_1 = __nvt_jiffies_us;
	printk("Test jiffies_1: %d\n", (int)jiffies_1);
	printk("jiffies_1 - jiffies_0: %d ms , expected spend time: %d ms \n", (int)(jiffies_1 - jiffies_0), (int)delay_ms);
	printk("jiffies_us_1 - jiffies_us_0: %d us , expected spend time: %d us \n", (int)(jiffies_us_1 - jiffies_us_0), (int)(delay_ms * 1000));
}

static ssize_t nvt_jiffies_proc_cmd_write(struct file *file, const char __user *buf,	 size_t size, loff_t *off)
{
	int len = size;
	char cmd_line[MAX_CMD_LENGTH];
	char *cmdstr = cmd_line;
	const char delimiters[] = {' ', 0x0A, 0x0D, '\0'};
	char *argv[MAX_ARG_NUM] = {0};
	unsigned char ucargc = 0;

	// check command length
	if (len > (MAX_CMD_LENGTH - 1)) {
		pr_err("Command length is too long!\n");
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

	// parse command string
	for (ucargc = 0; ucargc < MAX_ARG_NUM; ucargc++) {
		argv[ucargc] = strsep(&cmdstr, delimiters);

		if (argv[ucargc] == NULL) {
			break;
		}
	}

	// dispatch command handler
	if (strncmp(argv[0], "t", 2) == 0) {
		nvt_jiffies_test();
	} else {
		goto ERR_OUT;
	}

	return size;

ERR_OUT:
	return -1;
}

static const struct proc_ops nvt_jiffies_proc_fops = {
	.proc_open		= nvt_jiffies_proc_cmd_open,
	.proc_read		= seq_read,
	.proc_lseek		= seq_lseek,
	.proc_release	= single_release,
	.proc_write		= nvt_jiffies_proc_cmd_write
};

//=============================================================================
// proc "help" file operation functions
//=============================================================================
static int nvt_jiffies_proc_help_show(struct seq_file *sfile, void *v)
{
	seq_printf(sfile, "=====================================================================\n");
	seq_printf(sfile, " Dump nvt_jiffies:\n");
	seq_printf(sfile, " cat nvt_jiffies_cmd\n");
	seq_printf(sfile, " Test nvt_jiffies accuracy:\n");
	seq_printf(sfile, " echo \"t\" > nvt_jiffies_cmd\n");
	seq_printf(sfile, "=====================================================================\n");

	return 0;
}

static int nvt_jiffies_proc_help_open(struct inode *inode, struct file *file)
{
	return single_open(file, nvt_jiffies_proc_help_show, NULL);
}

static struct proc_ops proc_help_fops = {
	.proc_open   = nvt_jiffies_proc_help_open,
	.proc_release = single_release,
	.proc_read   = seq_read,
	.proc_lseek = seq_lseek,
};

struct proc_dir_entry *pmodule_root = NULL;
struct proc_dir_entry *pentry_cmd = NULL;
struct proc_dir_entry *pentry_help = NULL;
int nvt_jiffies_proc_init()
{
	int ret = 0;

	pmodule_root = proc_mkdir("nvt_jiffies", NULL);
	if (pmodule_root == NULL) {
		nvt_dbg(ERR, "failed to create Module root\n");
		ret = -EINVAL;
		goto remove_root;
	}

	pentry_cmd = proc_create("nvt_jiffies_cmd", S_IRUGO | S_IXUGO, pmodule_root, &nvt_jiffies_proc_fops);
	if (pentry_cmd == NULL) {
		nvt_dbg(ERR, "failed to create proc cmd!\n");
		ret = -EINVAL;
		goto remove_cmd;
	}

	pentry_help = proc_create("help", S_IRUGO | S_IXUGO, pmodule_root, &proc_help_fops);
	if (pentry_help == NULL) {
		nvt_dbg(ERR, "failed to create proc help!\n");
		ret = -EINVAL;
		goto remove_help;
	}

	return ret;

remove_help:
	proc_remove(pentry_help);

remove_cmd:
	proc_remove(pentry_cmd);

remove_root:
	proc_remove(pmodule_root);
	return ret;
}

int nvt_jiffies_proc_remove()
{
	proc_remove(pentry_help);
	proc_remove(pentry_cmd);
	proc_remove(pmodule_root);
	return 0;
}

MODULE_AUTHOR("Novatek Microelectronics Corp.");
MODULE_LICENSE("GPL");
