#ifdef __KERNEL__
#include <linux/slab.h>
#include <linux/buffer_head.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>

#define PROC_SW_DEC				"kdrv_sw_dec"
#define PROC_MAX_BUFFER			64

//=========================================
//  local variable
//=========================================
static struct proc_dir_entry *proc_root_entry = NULL;
static struct proc_dir_entry *proc_stuff_zeros_entry = NULL;


//=========================================
//  extern variable
//=========================================


//=====================================
//  proc stuff_zeros function
//=====================================
int g_stuff_zeros = 0;
void nvt_jpg_set_stuff_zeros(int enable)
{
	g_stuff_zeros = enable;
}
static int proc_stuff_zeros_show(struct seq_file *sfile, void *v)
{
	seq_printf(sfile, "JPEG sw decoder stuff_zeros enable = %d \n", g_stuff_zeros);
    return 0;
}
static int proc_stuff_zeros_open(struct inode *inode, struct file *file)
{
	return single_open(file, proc_stuff_zeros_show, NULL);
}
static ssize_t proc_stuff_zeros_write(struct file *file, const char __user * buffer, size_t count, loff_t *ppos)
{
	int enable;
	char proc_buffer[PROC_MAX_BUFFER];

	if (count > sizeof(proc_buffer) - 1) {
		printk("input is too large\n");
		return -EINVAL;
	}
	if (copy_from_user(proc_buffer, buffer, count)) {
		printk("proc dbg copy from user failed\n");
		return -EFAULT;
	}
	proc_buffer[count] = '\0';

	sscanf(proc_buffer, "%d", &enable);

	nvt_jpg_set_stuff_zeros(enable);

    return count;
}
static struct proc_ops proc_stuff_zeros_fops = {
    .proc_open = proc_stuff_zeros_open,
    .proc_read = seq_read,
    .proc_lseek = seq_lseek,
    .proc_release = single_release,
    .proc_write = proc_stuff_zeros_write
};


//=====================================
//  proc main function
//=====================================
int nvt_sw_dec_proc_clear(void)
{
	if (proc_stuff_zeros_entry)
		proc_remove(proc_stuff_zeros_entry);
	if (proc_root_entry)
		proc_remove(proc_root_entry);

	return 0;
}
int nvt_sw_dec_proc_init(void)
{
	if ((proc_root_entry = proc_mkdir(PROC_SW_DEC, NULL)) == NULL) {
		printk("failed to create JPEG sw decoder root, error !! \n");
		goto fail_proc;
	}
	if ((proc_stuff_zeros_entry = proc_create("stuff_zeros", S_IRUGO | S_IXUGO, proc_root_entry, &proc_stuff_zeros_fops)) == NULL) {
		printk("failed to create proc stuff_zeros, error !! \n");
		goto fail_proc;
	}

	return 0;

fail_proc:
	nvt_sw_dec_proc_clear();
	return -1;
}


#endif //__KERNEL__