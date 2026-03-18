#if defined(__LINUX)
#include <linux/module.h>
#include <linux/string.h>
#include <linux/kernel.h>
#include <linux/moduleparam.h>
#include <linux/platform_device.h>
#include <linux/vmalloc.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/kdev_t.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/uaccess.h>
#include <linux/sched.h>
#include <linux/of_device.h>
#include <linux/kdev_t.h>
#include <kwrap/dev.h>
#include "comm/util/log.h"
#endif

#include <gximage/gfx_internal.h>
#include <gximage/gfx_open.h>
#include <gximage/gfx_dbg.h>
#include <gximage/gximage_internal.h>

///////////////////////////////////////////////////////////////////////////////
#define __MODULE__          gfx_open
#define __DBGLVL__          NVT_DBG_WRN
#include "kwrap/debug.h"
unsigned int gfx_open_debug_level = __DBGLVL__;
///////////////////////////////////////////////////////////////////////////////

#if defined(__LINUX)

void gfx_memset(void *buf, unsigned char val, int len)
{
	memset(buf, val, len);
}

void gfx_memcpy(void *buf, void *src, int len)
{
	memcpy(buf, src, len);
}

void* gfx_alloc(int size)
{
	return kmalloc(size, GFP_ATOMIC);
}

void gfx_free(void *buf)
{
	kfree(buf);
}

int gfx_copy_from_user(void *dst, void *src, int len)
{
	return copy_from_user(dst, (void __user *)src, len);
}

int gfx_copy_to_user(void *dst, void *src, int len)
{
	return copy_to_user((void __user *)dst, src, len);
}

int gfx_seq_printf(void *m, const char *fmtstr, ...)
{
	int     len = 0;

	va_list marker;
	va_start(marker, fmtstr);
#if LOG_BUFFER
	if(m){
		seq_printf(m, fmtstr, marker);
	}else{
		printm2(fmtstr, marker);
	}
#else
	seq_printf(m, fmtstr, marker);
#endif
	va_end(marker);

	return len;
}

int gfx_snprintf(char *buf, int size, const char *fmtstr, ...)
{
	int     len = 0;

	va_list marker;
	va_start(marker, fmtstr);
	len = vsnprintf(buf, size, fmtstr, marker);
	va_end(marker);

	return len;
}

static int proc_show(struct seq_file *m, void *v) {
	seq_printf(m, "nothing!\n");
	return 0;
}

static int proc_open(struct inode *inode, struct  file *file) {
	return single_open(file, proc_show, NULL);
}

static long proc_ioctl(struct file* f, unsigned int cmd, unsigned long arg)
{
	return nvt_gfx_ioctl(-1, cmd, (void*)arg);
}

static struct proc_dir_entry *api_proc_file_entry = NULL;
static const struct proc_ops proc_file_fops = {
	.proc_open             = proc_open,
	.proc_ioctl            = proc_ioctl,
	.proc_release          = single_release,
};

static int info_show(struct seq_file *m, void *v) {

	return gfx_info_show(m, v);
}

static int info_open(struct inode *inode, struct  file *file) {
	return single_open(file, info_show, NULL);
}

static const struct proc_ops info_fops = {
	.proc_open             = info_open,
	.proc_release          = single_release,
	.proc_read             = seq_read,
	.proc_lseek            = seq_lseek,
};

static int help_show(struct seq_file *m, void *v) {

	gfx_cmd_showhelp(m, v);

	return 0;
}

static int help_open(struct inode *inode, struct  file *file) {
	return single_open(file, help_show, NULL);
}

static const struct proc_ops help_fops = {
	.proc_open             = help_open,
	.proc_release          = single_release,
	.proc_read             = seq_read,
	.proc_lseek            = seq_lseek,
};

//=============================================================================
// proc "dbglevel" file operation functions
//=============================================================================
static int dbglevel_show(struct seq_file *sfile, void *v)
{
	unsigned int dbg_lvl;

	dbg_lvl = get_hdal_flow_dbglevel(HDAL_FLOW_DBG_GFX);
	seq_printf(sfile, "0: fatal, 1: err, 2: wrn, 3: msg, 4: ind, 5: func\r\n");
	seq_printf(sfile, "gfx dbglevel:%d \r\n", dbg_lvl);
	return 0;
}

static int dbglevel_open(struct inode *inode, struct file *file)
{
	return single_open(file, dbglevel_show, NULL);
}

static ssize_t dbglevel_write(struct file *file, const char __user *buffer, size_t count, loff_t *ppos)
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
	printk("gfx dbglevel=%d\n", flow_dbg_lvl);
	set_hdal_flow_dbglevel(HDAL_FLOW_DBG_GFX, flow_dbg_lvl);
	return count;
}

static struct proc_ops dbglevelp_fops = {
	.proc_open   = dbglevel_open,
	.proc_release = single_release,
	.proc_read   = seq_read,
	.proc_write   = dbglevel_write,
	.proc_lseek = seq_lseek,
};

static int kflow_dbglevel_show(struct seq_file *m, void *v)
{
	char          msg[512];
	extern unsigned int  gfx_debug_level;

	seq_printf(m, "0: fatal, 1: err, 2: wrn, 3: msg, 4: ind, 5: func\n");
	snprintf(msg, sizeof(msg), "kflow_gfx dbglevel = %u\n", gfx_debug_level);
	seq_printf(m, msg);

	return 0;
}

static int kflow_dbglevel_open(struct inode *inode, struct  file *file) {
	return single_open(file, kflow_dbglevel_show, NULL);
}

static ssize_t kflow_dbglevel_write(struct file *file, const char __user *data, size_t size, loff_t *off)
{
	int len = size;
	char cmd_line[256];
	extern unsigned int  gfx_debug_level;

	if (len == 0 || len > 256) {
		GFX_DBG_ERR("Command length(%d) is 0 or > 256!\n", len);
		return size;
	}

	if (copy_from_user(cmd_line, data, len)) {
		return size;
	}

	cmd_line[len - 1] = '\0';
	sscanf(cmd_line, "%d", &gfx_debug_level);

	return size;
}

static const struct proc_ops kflow_dbglevel_fops = {
	.proc_open    = kflow_dbglevel_open,
	.proc_release = single_release,
	.proc_read    = seq_read,
	.proc_write   = kflow_dbglevel_write,
	.proc_lseek  = seq_lseek,
};

static int engine_spec_show(struct seq_file *m, void *v)
{
	seq_printf(m, "support copy : yuv420/Y8/argb1555/argb4444/argb8888\n");
	seq_printf(m, "support rotate : yuv420/Y8/argb1555/argb4444/argb8888\n");
	seq_printf(m, "support scale : yuv420/Y8/argb1555/argb4444/argb8888\n");
	seq_printf(m, "support color transform : argb1555/4444/8888=>yuv420/yuv422, yuv420/yuv422=>yuyv, nvx2=>yuv420\n");

	seq_printf(m, "support draw vertical/horizontal line : yuv420/Y8/argb1555/argb4444/argb8888\n");
	seq_printf(m, "support draw slope line : yuv420\n");
	seq_printf(m, "support draw rectangle : yuv420/Y8/argb1555/argb4444/argb8888\n");

	return 0;
}

static int engine_spec_open(struct inode *inode, struct  file *file) {
	return single_open(file, engine_spec_show, NULL);
}

static const struct proc_ops engine_spec_fops = {
	.proc_open    = engine_spec_open,
	.proc_release = single_release,
	.proc_read    = seq_read,
	.proc_lseek  = seq_lseek,
};

static int kflow_default_flush_show(struct seq_file *m, void *v)
{
	if(gximg_get_force_no_flush()){
		seq_printf(m, "never flush\n");
	}else{
		seq_printf(m, "backward compatible flush\n");
	}

	return 0;
}

static int kflow_default_flush_open(struct inode *inode, struct  file *file) {
	return single_open(file, kflow_default_flush_show, NULL);
}

static ssize_t kflow_default_flush_write(struct file *file, const char __user *data, size_t size, loff_t *off)
{
	int len = size;
	char cmd_line[256];
	int default_flush = 0;

	if (len == 0 || len > 256) {
		GFX_DBG_ERR("Command length(%d) is 0 or > 256!\n", len);
		return size;
	}

	if (copy_from_user(cmd_line, data, len)) {
		return size;
	}

	cmd_line[len - 1] = '\0';
	sscanf(cmd_line, "%d", &default_flush);

	if(default_flush){
		if(gximg_set_force_no_flush(0)){
			GFX_DBG_ERR("fail to enable flush\n");
		}
	}else{
		if(gximg_set_force_no_flush(1)){
			GFX_DBG_ERR("fail to disable flush\n");
		}
	}

	return size;
}

static const struct proc_ops kflow_default_flush_fops = {
	.proc_open    = kflow_default_flush_open,
	.proc_release = single_release,
	.proc_read    = seq_read,
	.proc_write   = kflow_default_flush_write,
	.proc_lseek  = seq_lseek,
};
//=============================================================================
// function declaration
//=============================================================================
static int nvt_gfx_probe(struct platform_device *pdev);
static int nvt_gfx_suspend(struct platform_device *pdev, pm_message_t state);
static int nvt_gfx_resume(struct platform_device *pdev);
static int nvt_gfx_remove(struct platform_device *pdev);
int __init nvt_gfx_module_init(void);
void __exit nvt_gfx_module_exit(void);

//=============================================================================
// platform driver
//=============================================================================

#define MODULE_MINOR_COUNT   1
#define MODULE_NAME          "nvt_gfx"

typedef struct _NVT_GFX_DRV_INFO {
        struct class *pmodule_class;
        struct device *p_device[MODULE_MINOR_COUNT];
        struct cdev cdev;
        dev_t dev_id;
} NVT_GFX_DRV_INFO, *PNVT_GFX_DRV_INFO;

static struct of_device_id gfx_match_table[] = {
	{   .compatible = "nvt,nvt_gfx"},
	{}
};

static int nvt_gfx_platform_open(struct inode *inode, struct file *file)
{
	return 0;
}

static int nvt_gfx_platform_release(struct inode *inode, struct file *file)
{
	return 0;
}

static long nvt_gfx_platform_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	return nvt_gfx_ioctl(-1, cmd, (void*)arg);
}

struct file_operations nvt_gfx_platform_fops = {
	.owner   = THIS_MODULE,
	.open    = nvt_gfx_platform_open,
	.release = nvt_gfx_platform_release,
	.unlocked_ioctl = nvt_gfx_platform_ioctl,
	.llseek  = no_llseek,
};

static int nvt_gfx_probe(struct platform_device *p_dev)
{
	NVT_GFX_DRV_INFO *p_drv_info;
	int ret = 0;
	unsigned char ucloop;

	DBG_IND("%s\n", p_dev->name);

	p_drv_info = kzalloc(sizeof(NVT_GFX_DRV_INFO), GFP_KERNEL);
	if (!p_drv_info) {
		GFX_DBG_ERR("failed to allocate memory\n");
		return -ENOMEM;
	}

	//Dynamic to allocate Device ID
	if (vos_alloc_chrdev_region(&p_drv_info->dev_id, MODULE_MINOR_COUNT, MODULE_NAME)) {
		GFX_DBG_ERR("Can't get device ID\n");
		ret = -ENODEV;
		goto FAIL_FREE_REMAP;
	}

	DBG_IND("DevID Major:%d minor:%d\n" \
			, MAJOR(p_drv_info->dev_id), MINOR(p_drv_info->dev_id));

	/* Register character device for the volume */
	cdev_init(&p_drv_info->cdev, &nvt_gfx_platform_fops);
	p_drv_info->cdev.owner = THIS_MODULE;

	if (cdev_add(&p_drv_info->cdev, p_drv_info->dev_id, MODULE_MINOR_COUNT)) {
		GFX_DBG_ERR("Can't add cdev\n");
		ret = -ENODEV;
		goto FAIL_CDEV;
	}

	p_drv_info->pmodule_class = class_create(THIS_MODULE, MODULE_NAME);
	if (IS_ERR(p_drv_info->pmodule_class)) {
		GFX_DBG_ERR("failed in creating class.\n");
		ret = -ENODEV;
		goto FAIL_CDEV;
	}

	/* register your own device in sysfs, and this will cause udev to create corresponding device node */
	for (ucloop = 0 ; ucloop < (MODULE_MINOR_COUNT) ; ucloop++) {
		p_drv_info->p_device[ucloop] = device_create(p_drv_info->pmodule_class, NULL
									   , MKDEV(MAJOR(p_drv_info->dev_id), (ucloop + MINOR(p_drv_info->dev_id))), NULL
									   , MODULE_NAME"%d", ucloop);

		if (IS_ERR(p_drv_info->p_device[ucloop])) {
			GFX_DBG_ERR("failed in creating device%d.\n", ucloop);
			#if (MODULE_MINOR_COUNT > 1) //CID 131215 (#1 of 1): Logically dead code (DEADCODE)
			for (; ucloop > 0 ; ucloop--) {
				device_unregister(p_drv_info->p_device[ucloop - 1]);
			}
			#endif
			ret = -ENODEV;
			goto FAIL_CLASS;
		}
	}

	platform_set_drvdata(p_dev, p_drv_info);

	return ret;

FAIL_CLASS:
	class_destroy(p_drv_info->pmodule_class);

FAIL_CDEV:
	cdev_del(&p_drv_info->cdev);
	vos_unregister_chrdev_region(p_drv_info->dev_id, MODULE_MINOR_COUNT);

FAIL_FREE_REMAP:

	kfree(p_drv_info);
	//coverity[UNUSED_VALUE]
	p_drv_info = NULL;
	return ret;
}

static int nvt_gfx_remove(struct platform_device *p_dev)
{
	PNVT_GFX_DRV_INFO p_drv_info;
	unsigned char ucloop;

	DBG_IND("\n");

	p_drv_info = platform_get_drvdata(p_dev);

	for (ucloop = 0 ; ucloop < (MODULE_MINOR_COUNT) ; ucloop++) {
		device_unregister(p_drv_info->p_device[ucloop]);
	}

	class_destroy(p_drv_info->pmodule_class);
	cdev_del(&p_drv_info->cdev);
	vos_unregister_chrdev_region(p_drv_info->dev_id, MODULE_MINOR_COUNT);

	kfree(p_drv_info);
	//coverity[UNUSED_VALUE]
	p_drv_info = NULL;
	return 0;
}

static int nvt_gfx_suspend(struct platform_device *pdev, pm_message_t state)
{
	return 0;
}

static int nvt_gfx_resume(struct platform_device *pdev)
{
	return 0;
}

static struct platform_driver nvt_gfx_driver = {
	.driver = {
		.name   = "nvt_gfx",
		.owner  = THIS_MODULE,
		.of_match_table = gfx_match_table,
	},
	.probe      = nvt_gfx_probe,
	.remove     = nvt_gfx_remove,
	.suspend    = nvt_gfx_suspend,
	.resume     = nvt_gfx_resume
};

#if defined(_GROUP_KO_)
#undef __init
#undef __exit
#undef module_init
#undef module_exit
#define __init
#define __exit
#define module_init(x)
#define module_exit(x)
#ifndef MODULE // in built-in case
#undef MODULE_VERSION
#undef VOS_MODULE_VERSION
#define MODULE_VERSION(x)
#define VOS_MODULE_VERSION(a1, a2, a3, a4, a5)
#endif
#endif

int __init nvt_gfx_module_init(void)
{
	int                      ret;
	struct proc_dir_entry    *proc_file_entry = NULL;
	struct proc_dir_entry    *proc_parent     = NULL;

	if(nvt_gfx_init()){
		GFX_DBG_ERR("nvt_gfx_init() fail\n");
		return -1;
	}

	ret = platform_driver_register(&nvt_gfx_driver);

	api_proc_file_entry = proc_create("nvt_gfx", 0, NULL, &proc_file_fops);
	if(api_proc_file_entry == NULL){
		GFX_DBG_ERR("fail to create /proc/nvt_gfx\n");
		return -ENOMEM;
	}

	proc_parent = proc_mkdir("hdal/gfx",NULL);
	if(!proc_parent){
		GFX_DBG_ERR("fail to create /proc/gfx\r\n");
		return -ENOMEM;
	}

	proc_file_entry = proc_create("info", 0, proc_parent, &info_fops);
	if(proc_file_entry == NULL){
		GFX_DBG_ERR("fail to create /proc/hdal/info\r\n");
		return -ENOMEM;
	}

	proc_file_entry = proc_create("help", 0, proc_parent, &help_fops);
	if(proc_file_entry == NULL){
		GFX_DBG_ERR("fail to create /proc/hdal/help\r\n");
		return -ENOMEM;
	}

	proc_file_entry = proc_create("dbglevel", 0, proc_parent, &dbglevelp_fops);
	if(proc_file_entry == NULL){
		GFX_DBG_ERR("fail to create /proc/hdal/dbglevel\r\n");
		return -ENOMEM;
	}

	proc_parent = proc_mkdir("kflow_gfx",NULL);
	if(!proc_parent){
		GFX_DBG_ERR("fail to create /proc/kflow_gfx\r\n");
		return -ENOMEM;
	}

	proc_file_entry = proc_create("dbglevel", 0, proc_parent, &kflow_dbglevel_fops);
	if(proc_file_entry == NULL){
		GFX_DBG_ERR("fail to create /proc/kflow_gfx/dbglevel\r\n");
		return -ENOMEM;
	}

	proc_file_entry = proc_create("engine_spec", 0, proc_parent, &engine_spec_fops);
	if(proc_file_entry == NULL){
		GFX_DBG_ERR("fail to create /proc/kflow_gfx/engine_spec\r\n");
		return -ENOMEM;
	}

	proc_file_entry = proc_create("default_flush", 0, proc_parent, &kflow_default_flush_fops);
	if(proc_file_entry == NULL){
		GFX_DBG_ERR("fail to create /proc/kflow_gfx/default_flush\r\n");
		return -ENOMEM;
	}

	return 0;
}

void __exit nvt_gfx_module_exit(void)
{
	nvt_gfx_exit();

	if(api_proc_file_entry){
		remove_proc_entry("nvt_gfx", NULL);
		remove_proc_entry("hdal/gfx/info", NULL);
		remove_proc_entry("hdal/gfx/help", NULL);
		remove_proc_entry("hdal/gfx/dbglevel", NULL);
		remove_proc_entry("hdal/gfx", NULL);
		remove_proc_entry("kflow_gfx/dbglevel", NULL);
		remove_proc_entry("kflow_gfx/engine_spec", NULL);
		remove_proc_entry("kflow_gfx/default_flush", NULL);
		remove_proc_entry("kflow_gfx", NULL);
		proc_remove(api_proc_file_entry);
		api_proc_file_entry = NULL;
	}

	platform_driver_unregister(&nvt_gfx_driver);
}

module_init(nvt_gfx_module_init);
module_exit(nvt_gfx_module_exit);

MODULE_AUTHOR("Novatek Corp.");
MODULE_DESCRIPTION("gfx driver");
MODULE_VERSION("1.05.000");
MODULE_LICENSE("GPL");
#endif
