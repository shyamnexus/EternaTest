/**
    NVT nvt_dma device driver main file

    @file       nvt_dma_test_main.c
    @ingroup
    @note
    Copyright   Novatek Microelectronics Corp. 2024.  All rights reserved.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License version 2 as
    published by the Free Software Foundation.
*/

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/platform_device.h>
#include <linux/fs.h>
#include <linux/interrupt.h>
#include <linux/vmalloc.h>
#include <linux/mutex.h>
#include <linux/slab.h>
#include <linux/sched.h>
#include <linux/io.h>
#include <linux/of_device.h>
#include <linux/of_irq.h>
#include <linux/kdev_t.h>
#include <linux/clk.h>
#include <linux/mm.h>
#include <asm/signal.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/uaccess.h>
#include <linux/version.h>
#include <linux/dmaengine.h>
#include <linux/kthread.h>
#include <linux/freezer.h>

#define DEBUG               0

#define PROC_CMD_LENGTH     128
#define PROC_ARG_NUM        12
#define PROC_DIR_NAME       "hwcpy_dmatest"

#define DMA_TEST_VERSION    "0.0.1"
#define DMA_CHANNELS        4

static u32 gld_pat[PAGE_SIZE];

struct mutex lock_info;

struct nvt_dma_chan {
	struct list_head list;
	struct list_head tx_list;
	struct dma_chan *chan;
	struct task_struct *kthread;
	char chan_name[32];
	u64 src;
	u64 dst;
	u64 len;
	u64 curr_time;
	u64 after_time;
	u32 maxburst;
	u32 time;
	enum dma_transfer_direction dir;
	bool verify;
	bool linklist;
	int active_time;
};

struct nvt_dma {
	struct device *dev;
	struct list_head chan_list;
};

struct nvt_dma *nvt_dma_ptr = NULL;

typedef struct proc_cmd {
	char cmd[PROC_CMD_LENGTH];
	int (*pfunc)(int argc, char **argv);
} PROC_CMD;

static struct proc_dir_entry *gp_proc_root = NULL;

static void _proc_dump_queue(void)
{
	struct nvt_dma_chan *child, *_next;

	mutex_lock(&lock_info);
	if (list_empty(&nvt_dma_ptr->chan_list)) {
		mutex_unlock(&lock_info);
		return;
	}

	list_for_each_entry_safe(child, _next, &nvt_dma_ptr->chan_list, list) {
		pr_info("-------------------------\n");
		pr_info("channel name : %s\n", child->chan_name);
		pr_info("src address : 0x%llx\n", child->src);
		pr_info("dst address : 0x%llx\n", child->dst);
		pr_info("len : 0x%llx\n", child->len);
		pr_info("maxburst : 0x%x\n", child->maxburst);
		pr_info("time : %d\n", child->time);
		// pr_info("direction : %s\n", child->dir == DMA_DEV_TO_MEM ? "DMA_DEV_TO_MEM" : "DMA_MEM_TO_DEV");
		pr_info("direction : %s\n", "DMA_MEM_TO_MEM");
		pr_info("verify : %s\n", child->verify == true ? "true" : "false");
		pr_info("link list : %s\n", child->linklist == true ? "true" : "false");
	}
	mutex_unlock(&lock_info);
}

static int _proc_queue_show(struct seq_file *sfile, void *v)
{
	_proc_dump_queue();
	return 0;
}

static int _proc_queue_open(struct inode *inode, struct file *file)
{
	return single_open(file, _proc_queue_show, NULL);
}

static int _proc_setup(int argc, char **argv)
{
//	u64 RW, verify, linklist;
	u64 verify, linklist;
	struct nvt_dma_chan *nvtchan;

#if DEBUG
	{
		int i = 0;
		pr_info("Func: %s, Total argc = [%d]\n", __FUNCTION__, argc);
		for (i = 0; i < argc; i++) {
			pr_info("Func: %s, argv[%d]: %s\n", __FUNCTION__, i, argv[i]);
		}
	}
#endif

	if (argc != 9) {
		dev_err(nvt_dma_ptr->dev, "%s: Wrong input parameter!!!\n", __func__);
		return -EINVAL;
	}


	nvtchan = kzalloc(sizeof(struct nvt_dma_chan), GFP_KERNEL);
	if (!nvtchan) {
		dev_err(nvt_dma_ptr->dev, "%s: alloc failed!!!\n", __func__);
		return -ENOMEM;
	}

	snprintf(nvtchan->chan_name, 32, "%s", argv[0]);
	nvtchan->src = simple_strtol(argv[1], NULL, 16);
	nvtchan->dst = simple_strtol(argv[2], NULL, 16);
	nvtchan->len = simple_strtol(argv[3], NULL, 16);
	nvtchan->maxburst = simple_strtol(argv[4], NULL, 16);
	nvtchan->time = simple_strtol(argv[5], NULL, 16);
	/*
	RW = simple_strtol(argv[6], NULL, 16);
	if (RW)
	    nvtchan->dir = DMA_DEV_TO_MEM;
	else
	    nvtchan->dir = DMA_MEM_TO_DEV;
	*/
	nvtchan->dir = DMA_MEM_TO_MEM;
	verify = simple_strtol(argv[7], NULL, 16);
	nvtchan->verify = verify ? true : false;

	linklist = simple_strtol(argv[8], NULL, 16);
	nvtchan->linklist = linklist ? true : false;
	nvtchan->active_time = 0;

	// DMA usage steps 1. Allocate a DMA slave channel
	nvtchan->chan = dma_request_chan(nvt_dma_ptr->dev, nvtchan->chan_name);
	if (NULL == nvtchan->chan) {
		dev_err(nvt_dma_ptr->dev, "%s: %s is not exist!!!\n", __func__, nvtchan->chan_name);
		kfree(nvtchan);
		return PTR_ERR(nvtchan->chan);
	}

	list_add_tail(&nvtchan->list, &nvt_dma_ptr->chan_list);

	return 0;
}


static int setup_src(u64 src, u64 dst, u64 len)
{
	int ret = 0;
	u32 offset = 0;
	u64 remaining_size = len, copy_size;
	void *src_va, *dst_va;

#if DEBUG
	pr_info("Func: %s, Line: %d, src pa(0x%llx) dst pa(0x%llx) len(0x%llx) \n", __FUNCTION__, __LINE__, src, dst, len);
#endif

	src_va = ioremap(src, len);
	if (src_va == NULL) {
		pr_err("ioremap fail [src addr: 0x%llx size: 0x%llx]\n", src, len);
		ret = -ENOMEM;
		goto map_fail;
	}

	dst_va = ioremap(dst, len);
	if (src_va == NULL) {
		pr_err("ioremap fail [dst addr: 0x%llx size: 0x%llx]\n", dst, len);
		ret = -ENOMEM;
		goto map_fail;
	}

	while (remaining_size) {
		if (remaining_size > PAGE_SIZE) {
			copy_size = PAGE_SIZE;
		} else {
			copy_size = remaining_size;
		}

		memcpy((void *)(src_va + offset), (void *)gld_pat, copy_size);

		offset += copy_size;
		remaining_size -= copy_size;
	}

	memset((void *)dst_va, 0x01, len);

map_fail:
	if (src_va) {
		iounmap(src_va);
	}
	if (dst_va) {
		iounmap(dst_va);
	}
	return ret;
}

static int check_result(u64 src, u64 dst, u64 len)
{
	int ret = 0;
	u32 offset = 0;
	u64 remaining_size = len, copy_size;
	void *src_va, *dst_va;

	src_va = ioremap(src, len);
	if (src_va == NULL) {
		pr_err("ioremap fail [src addr: 0x%llx size: 0x%llx]\n", src, len);
		ret = -ENOMEM;
		goto map_fail;
	}

	dst_va = ioremap(dst, len);
	if (src_va == NULL) {
		pr_err("ioremap fail [dst addr: 0x%llx size: 0x%llx]\n", dst, len);
		ret = -ENOMEM;
		goto map_fail;
	}

	while (remaining_size) {
		if (remaining_size > PAGE_SIZE) {
			copy_size = PAGE_SIZE;
		} else {
			copy_size = remaining_size;
		}

		if (memcmp((void *)(dst_va + offset), (void *)(src_va + offset), copy_size)) {
			pr_err("!!!! data is not match[src addr: 0x%llx, dst addr: 0x%llx, size : 0x%llx]\n", src, dst, len);
			ret = -EIO;
			goto map_fail;
		}

		offset += copy_size;
		remaining_size -= copy_size;
	}

#if DEBUG
	pr_info("\nFunc: %s, Line: %d,*** data compare pass *** src pa(0x%llx) dst pa(0x%llx) len(0x%llx) \n\n",
			__FUNCTION__, __LINE__, src, dst, len);
#endif

map_fail:
	if (src_va) {
		iounmap(src_va);
	}
	if (dst_va) {
		iounmap(dst_va);
	}
	return ret;
}

static int nvt_linklist_thread(void *param)
{
	int ret = 0, i = 0;
	struct nvt_dma_chan *nvtchan = param;
	smp_rmb();
	while (nvtchan->active_time > 0) {
		schedule_timeout_interruptible(msecs_to_jiffies(1));
		i++;
		if (i > 1000) {
			break;
		}
	}
	nvtchan->after_time = ktime_get_real_ns();

	if (i > 1000) {
		dev_err(nvt_dma_ptr->dev, "%s: Timeout !!!\n", nvtchan->chan_name);
		ret = -ETIMEDOUT;
	}

	if (nvtchan->verify) {
		if (check_result(nvtchan->src, nvtchan->dst, nvtchan->len)) {
			ret = -EINVAL;
		}
	}

	pr_info("[%s] : Bandwidth: %llu MB/s \n", nvtchan->chan_name, nvtchan->time * 1000 * nvtchan->len / (nvtchan->after_time - nvtchan->curr_time));

	dma_release_channel(nvtchan->chan);
	list_del(&nvtchan->list);
	kfree(nvtchan);
	return ret;
}

static void dmatest_callback(void *arg)
{
	struct nvt_dma_chan *nvtchan = arg;
	nvtchan->active_time--;
	if (nvtchan->active_time == 0) {
		pr_info("[%s] : [src addr: 0x%llx, dst addr: 0x%llx, size : 0x%llx] : PASS\n", nvtchan->chan_name, nvtchan->src, nvtchan->dst, nvtchan->len);
	}
}

static int linklist_run(struct nvt_dma_chan *nvtchan)
{
	dma_cookie_t cookie;
	char name[32];
	struct dma_async_tx_descriptor *desc;
	int i, ret = 0;

	for (i = 0; i < nvtchan->time; i ++) {

		// DMA usage steps 3. Get a descriptor for transaction
		desc = dmaengine_prep_dma_memcpy(nvtchan->chan, nvtchan->dst, nvtchan->src, nvtchan->len, DMA_PREP_INTERRUPT);
		if (!desc) {
			dev_err(nvt_dma_ptr->dev, "%s: dmaengine_prep_dma_memcpy failed\n", nvtchan->chan_name);
			ret = -EINVAL;
			goto fail_out;
		}

		desc->callback = dmatest_callback;
		desc->callback_param = nvtchan;
		nvtchan->active_time++;

		// DMA usage steps 4. Submit the transaction
		cookie = dmaengine_submit(desc);
		if (dma_submit_error(cookie)) {
			dev_err(nvt_dma_ptr->dev, "%s: dmaengine_submit failed\r\n", nvtchan->chan_name);
			ret = cookie;
			goto fail_out;
		}
	}

	nvtchan->curr_time = ktime_get_real_ns();
	// DMA usage steps 5. Trigger DMA
	dma_async_issue_pending(nvtchan->chan);

	snprintf(name, 32, "%s-linklist", nvtchan->chan_name);
	smp_wmb();
	kthread_run(nvt_linklist_thread, nvtchan, name);

fail_out:
	return ret;
}
static int normal_run(struct nvt_dma_chan *nvtchan)
{
	enum dma_status status;
	dma_cookie_t cookie;
	struct dma_async_tx_descriptor *desc;
	int i, ret = 0;

#if DEBUG
	pr_info("Func: %s, Line: %d, nvtchan->time(%d) chan_name(%s) \n",
			__FUNCTION__, __LINE__, nvtchan->time, nvtchan->chan_name);
#endif

	for (i = 0; i < nvtchan->time; i ++) {
		// DMA usage steps 3. Get a descriptor for transaction
		desc = dmaengine_prep_dma_memcpy(nvtchan->chan, nvtchan->dst, nvtchan->src, nvtchan->len, DMA_PREP_INTERRUPT);
		if (!desc) {
			dev_err(nvt_dma_ptr->dev, "%s: dmaengine_prep_dma_memcpy failed\n", nvtchan->chan_name);
			ret = -EINVAL;
			goto fail_out;
		}

		// DMA usage steps 4. Submit the transaction
		cookie = dmaengine_submit(desc);
		if (dma_submit_error(cookie)) {
			dev_err(nvt_dma_ptr->dev, "%s: dmaengine_submit failed\r\n", nvtchan->chan_name);
			ret = cookie;
			goto fail_out;
		}

		nvtchan->curr_time = ktime_get_real_ns();
		// DMA usage steps 5. Trigger DMA, Wait DMA done
		status = dma_sync_wait(nvtchan->chan, cookie);
		if (DMA_COMPLETE != status) {
			dev_err(nvt_dma_ptr->dev, "%s: dma_sync_wait failed, status %d\r\n", nvtchan->chan_name, status);
			ret = status;
			goto fail_out;
		}
		nvtchan->after_time = ktime_get_real_ns();

		if (nvtchan->verify) {
			if (check_result(nvtchan->src, nvtchan->dst, nvtchan->len)) {
				ret = -ENOMEM;
				goto fail_out;
			}
		}
	}

fail_out:
	pr_info("[%s] : [src addr: 0x%llx, dst addr: 0x%llx, size : 0x%llx] : PASS[%d] FAIL[%d]\n", nvtchan->chan_name, nvtchan->src, nvtchan->dst, nvtchan->len, i, nvtchan->time - i);
	pr_info("[%s] : Bandwidth: %llu MB/s \n", nvtchan->chan_name, 1000 * nvtchan->len / (nvtchan->after_time - nvtchan->curr_time));
	return ret;
}

static int nvt_dma_thread(void *param)
{
	int ret = 0;
	struct dma_slave_config config;
	struct nvt_dma_chan *nvtchan = param;

	set_freezable();

	smp_rmb();

	config.src_addr_width = 1;  // DMA_SLAVE_BUSWIDTH_1_BYTE
	config.dst_addr_width = 1;  // DMA_SLAVE_BUSWIDTH_1_BYTE
	config.src_maxburst = nvtchan->maxburst;
	config.dst_maxburst = nvtchan->maxburst;
	config.direction = nvtchan->dir;

#if DEBUG
	pr_info("Func: %s, Line: %d, nvtchan->verify(%d) src_maxburst(%d) dst_maxburst(%d) direction(%d) \n",
			__FUNCTION__, __LINE__, nvtchan->verify, config.src_maxburst, config.dst_maxburst, config.direction);
#endif

	if (nvtchan->verify) {
		if (setup_src(nvtchan->src, nvtchan->dst, nvtchan->len)) {
			dev_err(nvt_dma_ptr->dev, "%s: setup src failed\n", nvtchan->chan_name);
			ret = -EINVAL;
			goto thread_out;
		}
	}

	// DMA usage steps 2. Set slave and controller specific parameters
	ret = dmaengine_slave_config(nvtchan->chan, &config);
	if (ret) {
		dev_err(nvt_dma_ptr->dev, "%s: config failed\n", nvtchan->chan_name);
		goto thread_out;
	}

	if (nvtchan->linklist) {
		ret = linklist_run(nvtchan);
		return ret;
	} else {
		ret = normal_run(nvtchan);
	}

thread_out:
	dma_release_channel(nvtchan->chan);
	list_del(&nvtchan->list);
	kfree(nvtchan);
	return ret;
}

static int _proc_run(int argc, char **argv)
{
	struct nvt_dma_chan *child, *_next;

	if (list_empty(&nvt_dma_ptr->chan_list)) {
		mutex_unlock(&lock_info);
		return 0;
	}

	list_for_each_entry_safe(child, _next, &nvt_dma_ptr->chan_list, list) {
		smp_wmb();
		child->kthread = kthread_run(nvt_dma_thread, child, child->chan_name);
		if (IS_ERR(child->kthread)) {
			pr_err("Create thread[%s] failed !!\n", child->chan_name);
			mutex_unlock(&lock_info);
			return -EINVAL;
		}
	}

	list_del_init(&nvt_dma_ptr->chan_list);
	return 0;
}

static int _proc_clean(int argc, char **argv)
{
	struct nvt_dma_chan *child, *_next;

	if (list_empty(&nvt_dma_ptr->chan_list)) {
		return 0;
	}

	list_for_each_entry_safe(child, _next, &nvt_dma_ptr->chan_list, list) {
		dma_release_channel(child->chan);
		list_del(&child->list);
		kfree(child);
	}
	list_del_init(&nvt_dma_ptr->chan_list);
	return 0;
}

static PROC_CMD g_proc_cmd_list[] = {
	{"setup", _proc_setup},
	{"run", _proc_run},
	{"clean", _proc_clean},
};

static void _proc_cmd_usage(void)
{
	unsigned int cmd_idx;

	pr_info("Usage: echo xxx > cmd\n");
	pr_info("---------------------\n");
	for (cmd_idx = 0 ; cmd_idx < ARRAY_SIZE(g_proc_cmd_list); cmd_idx++) {
		pr_info("%s\n", g_proc_cmd_list[cmd_idx].cmd);
		if (strcmp("setup", g_proc_cmd_list[cmd_idx].cmd) == 0) {
			pr_info("example : echo %s [dmas_name] [src address] [dst address] [size] [maxburst size] [times] [Write(0x0)] [skip verify(0x0)/verify(0x1) result] [normal mode(0x0)/link list mode(0x1)]> cmd\n", g_proc_cmd_list[cmd_idx].cmd);
			pr_info("echo %s hwcp0 0x30200000 0x800200000 0x200000 0x1 0x1 0x0 0x1 0x0> cmd\n", g_proc_cmd_list[cmd_idx].cmd);
		}
	}
}

static int _proc_cmd_show(struct seq_file *sfile, void *v)
{
	_proc_cmd_usage();
	return 0;
}

static int _proc_cmd_open(struct inode *inode, struct file *file)
{
	return single_open(file, _proc_cmd_show, NULL);
}

static ssize_t _proc_cmd_write(struct file *file, const char __user *buf,
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

	smp_rmb();
	mutex_lock(&lock_info);
	// copy command string from user space
	if (copy_from_user(cmd_line, buf, size)) {
		mutex_unlock(&lock_info);
		goto ERR_OUT;
	}

	cmd_line[size - 1] = '\0';

	pr_info("PROC_CMD: [%s], size %ld\n", cmd_line, (unsigned long)size);

	// parse command string
	for (argc = 0; argc < PROC_ARG_NUM; argc++) {
		argv[argc] = strsep(&cmdstr, delimiters);

		if (argv[argc] == NULL) {
			break;
		}
	}

	// dispatch command handler
	for (cmd_idx = 0 ; cmd_idx < ARRAY_SIZE(g_proc_cmd_list); cmd_idx++) {
		if (strncmp(argv[0], g_proc_cmd_list[cmd_idx].cmd, PROC_CMD_LENGTH) == 0) {
			ret = g_proc_cmd_list[cmd_idx].pfunc(argc - 1, &argv[1]);
			break;
		}
	}
	mutex_unlock(&lock_info);

	if (cmd_idx >= ARRAY_SIZE(g_proc_cmd_list)) {
		goto ERR_INVALID_CMD;
	}

	return size;

ERR_INVALID_CMD:
	pr_err("Invalid CMD [%s]\n", cmd_line);
	_proc_cmd_usage();

ERR_OUT:
	return -EINVAL;
}

static struct proc_ops _proc_cmd_fops = {
	.proc_open    = _proc_cmd_open,
	.proc_read    = seq_read,
	.proc_write   = _proc_cmd_write,
	.proc_lseek  = seq_lseek,
	.proc_release = single_release,
};

static struct proc_ops _proc_queue_fops = {
	.proc_open    = _proc_queue_open,
	.proc_read    = seq_read,
	.proc_release = single_release,
};

int nvt_dma_test_proc_init(void)
{
	gp_proc_root = proc_mkdir(PROC_DIR_NAME, NULL);
	if (NULL == gp_proc_root) {
		pr_err("proc_mkdir %s failed\n", PROC_DIR_NAME);
		goto proc_init_err;
	}

	if (NULL == proc_create("cmd", S_IRUGO | S_IXUGO, gp_proc_root, &_proc_cmd_fops)) {
		pr_err("proc_create cmd failed\n");
		goto proc_init_err;
	}

	if (NULL == proc_create("queue", S_IRUGO | S_IXUGO, gp_proc_root, &_proc_queue_fops)) {
		pr_err("proc_create queue failed\n");
		goto proc_init_err;
	}

	return 0;

proc_init_err:
	if (gp_proc_root) {
		proc_remove(gp_proc_root);
		gp_proc_root = NULL;
	}

	return -1;
}

void nvt_dma_test_proc_exit(void)
{
	if (gp_proc_root) {
		proc_remove(gp_proc_root);
		gp_proc_root = NULL;
	}
}

static int nvt_dma_test_probe(struct platform_device *pdev)
{
	int i;

	dev_info(&pdev->dev, "Func: %s, Line: %d, init start\n", __FUNCTION__, __LINE__);

	nvt_dma_ptr = devm_kzalloc(&pdev->dev, sizeof(struct nvt_dma), GFP_KERNEL);
	if (!nvt_dma_ptr) {
		pr_err("Alloc memory for nvt_dma failed\n");
		return -ENOMEM;
	}

	nvt_dma_ptr->dev = &pdev->dev;
	INIT_LIST_HEAD(&nvt_dma_ptr->chan_list);

	for (i = 0; i < PAGE_SIZE / 4; i += 4) {
		memset((void *)(gld_pat + i), 0x22, 1);
		memset((void *)(gld_pat + i + 1), 0x11, 1);
		memset((void *)(gld_pat + i + 2), 0xbb, 1);
		memset((void *)(gld_pat + i + 3), 0xaa, 1);
	}

	if (0 > nvt_dma_test_proc_init()) {
		pr_err("%s: proc_init failed\n", __func__);
		return -ENOMEM;
	}


	// coverity[side_effect_free]
	mutex_init(&lock_info);

	dev_info(&pdev->dev, "Func: %s, Line: %d, init success\n", __FUNCTION__, __LINE__);

	return 0;
}

static int nvt_dma_test_remove(struct platform_device *pdev)
{
	return 0;
}

static const struct of_device_id nvt_dma_test_dt_match[] = {
	{.compatible = "nvt,nvt_hwcpy_test"},
	{ },
};
MODULE_DEVICE_TABLE(of, nvt_dma_test_dt_match);

static struct platform_driver nvt_dma_test_driver = {
	.probe = nvt_dma_test_probe,
	.remove = nvt_dma_test_remove,
	.driver = {
		.name = "nvt_hwcpy_test",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(nvt_dma_test_dt_match),
	},
};

int __init nvt_dma_test_init(void)
{
	int ret;

	ret = platform_driver_register(&nvt_dma_test_driver);
	if (ret < 0) {
		pr_err("%s: init failed\n", __func__);
		return ret;
	}
	return 0;
}

void __exit nvt_dma_test_exit(void)
{
	nvt_dma_test_proc_exit();
}

module_init(nvt_dma_test_init);
module_exit(nvt_dma_test_exit);

MODULE_AUTHOR("Novatek Corp.");
MODULE_DESCRIPTION("DMA test driver");
MODULE_LICENSE("GPL");
MODULE_VERSION(DMA_TEST_VERSION);
