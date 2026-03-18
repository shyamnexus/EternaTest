#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/platform_device.h>
#include <linux/fs.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/interrupt.h>
#include <linux/vmalloc.h>
#include <linux/slab.h>
#include <linux/sched.h>
#include <linux/io.h>
#include <linux/of_device.h>
#include <linux/kdev_t.h>
#include <linux/clk.h>
#include <linux/mm.h>
#include <linux/slab.h>
#include <asm/signal.h>

void *ptr = NULL;
struct proc_dir_entry *nvt_test_mod_dir_root = NULL;

static int nvt_data_breakpoint_test_show(struct seq_file *m, void *v)
{
	seq_printf(m, "%ld\n", *(unsigned long*)ptr);

	return 0;
}

static int nvt_data_breakpoint_test_open(struct inode *inode, struct file *file)
{
	return single_open(file, nvt_data_breakpoint_test_show, NULL);
}

static ssize_t nvt_data_breakpoint_test_write(struct file *file, const char __user *buffer,
								size_t count, loff_t *pos)
{
	char *buf = (char *) __get_free_page(GFP_USER);
	unsigned long num = 0;
	int res = 0;

	if (!buf)
		return -ENOMEM;

	res = -EFAULT;
	if (copy_from_user(buf, buffer, count))
			goto out;

	res = kstrtoul(buf, 10, &num);
	if (res < 0) {
		pr_info("Error parsing\n");
		goto out;
	}

	*(unsigned long*)ptr = num;
	res = count;
out:
	free_page((unsigned long)buf);
	return res;
}

static const struct proc_ops nvt_data_breakpoint_test_fops = {
	.proc_open = nvt_data_breakpoint_test_open,
	.proc_read = seq_read,
	.proc_write = nvt_data_breakpoint_test_write,
	.proc_release = single_release,
};

int __init test_main_init(void)
{
	struct proc_dir_entry *entry = NULL;

	ptr = kmalloc(SZ_1M, GFP_KERNEL);
	pr_info("Virtual address is %lx\n", (uintptr_t)ptr);

	nvt_test_mod_dir_root = proc_mkdir("nvt_test_mod", NULL);
	entry = proc_create("test", 0664, nvt_test_mod_dir_root, &nvt_data_breakpoint_test_fops);
	if (!entry)
		return -ENOMEM;
	return 0;
}

void __exit test_main_exit(void)
{
	kfree(ptr);
	proc_remove(nvt_test_mod_dir_root);
}

module_init(test_main_init);
module_exit(test_main_exit);

MODULE_AUTHOR("Novatek Corp.");
MODULE_DESCRIPTION("Test driver");
MODULE_LICENSE("GPL");
