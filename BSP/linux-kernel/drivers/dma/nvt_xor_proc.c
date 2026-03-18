#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/delay.h>

#include <linux/dma-mapping.h>
#include <linux/dmaengine.h>

typedef struct proc_xor {
	struct proc_dir_entry *pproc_module_root;
	struct proc_dir_entry *pproc_memcpy_entry;
} proc_xor_t;
proc_xor_t proc_xor;

// for nvt_xor memcpy test
static int nvt_xor_proc_memcpy_show(struct seq_file *sfile, void *v)
{
    struct dma_device       *dev;
    struct dma_chan         *nvt_xor_ch;
    struct dma_async_tx_descriptor *tx;
    dma_cap_mask_t		    mask;
    dma_cookie_t            cookie;
    dma_addr_t              src_dma, dest_dma;
    enum dma_status         status;
    void                    *src, *dest;
    int                     i = 0;
    
    dma_cap_zero(mask);
	dma_cap_set(DMA_XOR, mask);

    src = kmalloc(PAGE_SIZE, GFP_KERNEL);
	if (!src) {
		return -ENOMEM;
	}
    /* Fill in src buffer */
	for (i = 0; i < PAGE_SIZE; i++) {
		((u8 *) src)[i] = (u8)i;
	}

	dest = kzalloc(PAGE_SIZE, GFP_KERNEL);
	if (!dest) {
		kfree(src);
		return -ENOMEM;
	}

	/* Get a channel */
	nvt_xor_ch = dma_request_channel(mask, NULL, NULL);
    if (!nvt_xor_ch) {
		pr_err("cannot get DMA channel.\n");
        return 0;
	}

    dev = nvt_xor_ch->device;
    src_dma = dma_map_page(nvt_xor_ch->device->dev, virt_to_page(src),
						   offset_in_page(src), PAGE_SIZE,
						   DMA_TO_DEVICE);

    dest_dma = dma_map_page(nvt_xor_ch->device->dev, virt_to_page(dest),
							offset_in_page(dest), PAGE_SIZE,
							DMA_FROM_DEVICE);

    tx = dev->device_prep_dma_memcpy(nvt_xor_ch,dest_dma, src_dma, PAGE_SIZE, 0);
    cookie = dmaengine_submit(tx);
    dma_async_issue_pending(nvt_xor_ch);
    async_tx_ack(tx);
	msleep(1);
    status = dma_sync_wait(nvt_xor_ch, cookie);
	if (DMA_COMPLETE != status) {
		pr_err("dma_sync_wait failed, status %d\r\n", status);
		return 0;
	}
    dma_sync_single_for_cpu(nvt_xor_ch->device->dev, dest_dma,
							PAGE_SIZE, DMA_FROM_DEVICE);
	if (memcmp(src, dest, PAGE_SIZE)) {
		pr_err("Self-test copy failed compare, disabling\n");
	}

	seq_printf(sfile, "compare successful\n");

	dma_release_channel(nvt_xor_ch);

	return 0;
}

static int nvt_xor_proc_memcpy_open(struct inode *inode, struct file *file)
{
	return single_open(file, nvt_xor_proc_memcpy_show, NULL);
}

static struct proc_ops proc_memcpy_test_fops = {
	.proc_open    = nvt_xor_proc_memcpy_open,
	.proc_release = single_release,
	.proc_read    = seq_read,
	.proc_lseek   = seq_lseek,
};

int nvt_xor_proc_init(void)
{
	int ret = 0;
	struct proc_dir_entry *pmodule_root = NULL;
	struct proc_dir_entry *pentry = NULL;

    pmodule_root = proc_mkdir("nvt_info/nvt_xor", NULL);
	if (pmodule_root == NULL) {
		pr_err("failed to create Module root\n");
		ret = -EINVAL;
		goto remove_proc;
	}
	proc_xor.pproc_module_root = pmodule_root;

    pentry = proc_create("memcpy_test", S_IRUGO | S_IXUGO, pmodule_root, &proc_memcpy_test_fops);
	if (pentry == NULL) {
		pr_err("failed to create proc memcpy!\n");
		ret = -EINVAL;
		goto remove_proc;
	}
	proc_xor.pproc_memcpy_entry = pentry;

	

remove_proc:
	return ret;
}

late_initcall(nvt_xor_proc_init);