#include <linux/version.h>
#include <linux/seq_file.h>
#include <linux/proc_fs.h>
#include <linux/slab.h>
#include <linux/export.h>
#include <asm/memory.h>
#include <asm/io.h>
#include <asm/uaccess.h>
#include <linux/ioctl.h>
#include <linux/miscdevice.h>
#include <linux/cdev.h>
#include <linux/of.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include "kdrv_audioio_int.h"
#include "kdrv_audioio_dbg.h"

extern unsigned int kdrv_audioio_debug_level;

uint dump_buf_param[6] = {0}; //[0]dev, [1]type, [2]ch
char dump_buf_path[256] = "/tmp";
char *dump_buf_ptr = 0;
uint dump_buf_size = 100000;
uint dump_buf_size_cur = 0;
uint dump_buf_write = 0;

#define AU_PROC_NAME "kdrv_audio"

/**
 * hold proc pointer
 */
static struct proc_dir_entry *entity_proc = NULL;
static struct proc_dir_entry *au_proc_dump_dts_d = NULL;
static struct proc_dir_entry *au_proc_dump_ch_cfg_d = NULL;
static struct proc_dir_entry *au_proc_kdrv_audioio_debug_level_d = NULL;
static struct proc_dir_entry *au_proc_dump_buf_d = NULL;

static int proc_read_kdrv_audioio_debug_level(struct seq_file *sfile, void *v)
{
	seq_printf(sfile, "0: fatal, 1: err, 2: wrn, 3: msg, 4: ind, 5: func\r\n");
	seq_printf(sfile, "kdrv_audio dbglevel:%d \r\n", kdrv_audioio_debug_level);

	return 0;
}

static int proc_read_dump_ch_cfg(struct seq_file *sfile, void *v)
{
	int i, dev = 0;

	for (i = 0; i < PLATFORM_MAX_SSP_CNT; i++) {
		seq_printf(sfile, "i2s idx        = %d\n", audio_i2s_num[i]);
		seq_printf(sfile, "------------------------\n");
		seq_printf(sfile, "audio enable   = %d\n", audio_enable[i]);
		if (!audio_enable[i]) {
			seq_printf(sfile, "\n");
			continue;
		}
		if (i != 4) { //not hdmi
			seq_printf(sfile, "in_dev         = %d\n", dev);
		}
		seq_printf(sfile, "out_dev        = %d\n", dev);
		seq_printf(sfile, "total channels = %d\n", audio_tdm_chan[i]);
		seq_printf(sfile, "stereo mode    = %d\n", audio_is_stereo[i]);
		seq_printf(sfile, "sample size    = %d\n", audio_sample_size[i]);
		seq_printf(sfile, "sample rate    = %d\n", audio_sample_rate[i]);
		seq_printf(sfile, "bit clock      = %d\n", audio_bit_clock[i]);
		seq_printf(sfile, "master mode    = %d\n", audio_i2s_master[i]);
		seq_printf(sfile, "data order type= %d\n", audio_i2s_dataorder_type[i]);
		seq_printf(sfile, "\n");
		dev++;
	}

	return 0;
}

static int proc_read_dump_dts(struct seq_file *sfile, void *v)
{
	struct device_node *node;
	int *value, *mclk_value;
	int i, j, para_len = 9, dtsi_num;
	const char var_name[9][20] = {
		"<  i2s_num  >,",
		"<   enable  >,",
		"<  channel  >,",
		"<data_stereo>,",
		"<sample_size>,",
		"<sample_rate>,",
		"< bit_clock >,",
		"< i2s_master>,",
		"< data_order>,",
	};

	value = (int *)kmalloc(sizeof(int) * para_len * PLATFORM_MAX_SSP_CNT, GFP_KERNEL);
	mclk_value = (int *)kmalloc(sizeof(int) * 2 * PLATFORM_MAX_CHIP_CNT, GFP_KERNEL);


	node = of_find_node_by_name(NULL, "audio_i2s_part");
	if (node == NULL) {
		printk("[audio_i2s_part] doesn't exist in audio dts! \n");
	} else {
		dtsi_num = of_property_count_u32_elems(node, "i2s") ;
		para_len = dtsi_num / PLATFORM_MAX_SSP_CNT;

		if (of_property_read_u32_array(node, "i2s", &value[0], dtsi_num) == 0) {

			seq_printf(sfile, "audio_i2s_part {\n");
			seq_printf(sfile, "               ");
			for (i = 0; i < para_len; i++) {
				seq_printf(sfile, "%s", var_name[i]);
			}
			seq_printf(sfile, "\r\n");
			seq_printf(sfile, "         i2s =\n");

			for (i = 0; i < PLATFORM_MAX_SSP_CNT; i++) {
				seq_printf(sfile, "               ");
				for (j = 0; j < para_len; j++) {
					seq_printf(sfile, "<%11d>,", value[para_len * i + j]);
				}
				seq_printf(sfile, "\r\n");
			}
			seq_printf(sfile, "};\n");
		} else {
			printk("[i2s] can't be found in the [audio_i2s_part] node, parameter parsing fail\n");
		}
	}

	para_len = 2;
	node = NULL;
	node = of_find_node_by_name(NULL, "audio_mclk_part");
	if (node == NULL) {
		printk("[audio_mclk_part] doesn't exist in audio dts! \n");
	} else {
		if (of_property_read_u32_array(node, "mclk", &mclk_value[0], 2 * PLATFORM_MAX_CHIP_CNT) == 0) {
			seq_printf(sfile, "audio_mclk_part {\n");
			seq_printf(sfile, "               <  i2s_num  >,<    mclk   >,\n");
			seq_printf(sfile, "        mclk =\n");
			for (i = 0; i < PLATFORM_MAX_SSP_CNT - 1; i++) { //hdmi no mclk
				seq_printf(sfile, "               ");
				for (j = 0; j < para_len; j++) {
					seq_printf(sfile, "<%11d>,", mclk_value[para_len * i + j]);
				}
				seq_printf(sfile, "\r\n");
			}
			seq_printf(sfile, "};\n");
		} else {
			printk("[mclk] can't be found in the [audio_mclk_part] node, parameter parsing fail\n");
		}
	}

	kfree(mclk_value);
	kfree(value);

	return 0;
}

static int proc_read_dump_buf(struct seq_file *sfile, void *v)
{
	//regfile_ssp_t *ssp;
	//char *pchar, *pchar_start, *pchar_pa;
	//int i, j, k;
	uint dev, type, ch, mode;//, dev_idx, ch_idx;

	dev = dump_buf_param[0];
	type = dump_buf_param[1];
	ch = dump_buf_param[2];
	mode = dump_buf_param[3];

	//ssp = &regFile->ssp[0];

	seq_printf(sfile, "usage : echo i2s_idx type ch mode path buf_size buf_write > /proc/kdrv_audio/dump_buf\n");
	seq_printf(sfile, "  i2s_idx:\n");
	seq_printf(sfile, "    0: (i2s), 1: (i2s2), 2: (i2s3), 3: (i2s4), 4: (hdmi)\n");
	seq_printf(sfile, "  type:\n");
	seq_printf(sfile, "    0:rx, 1:tx, 2:Disable\n");
	seq_printf(sfile, "  ch:\n");
	seq_printf(sfile, "    0:ch0, 1:ch1, ..., 15:ch15\n");
	seq_printf(sfile, "  mode:\n");
	seq_printf(sfile, "    0:bin file, 1:uart\n");
	seq_printf(sfile, "  path:\n");
	seq_printf(sfile, "    path to save file\n");
	seq_printf(sfile, "  buf_size:\n");
	seq_printf(sfile, "    size of data buf (bytes)\n");
	seq_printf(sfile, "  buf_write:\n");
	seq_printf(sfile, "    0:set parameter, 1: only write buf to file, 2: reset write buf\n");
	seq_printf(sfile, "    when this value is not 0, other parameters is ignored\n\n");
	seq_printf(sfile, "  Ex:\n");
	seq_printf(sfile, "    To dump hdmi, tx, ch0, save bin file to /mnt/mtd, buf_size 100000\n");
	seq_printf(sfile, "    => echo 4 1 0 0 /mnt/mtd 100000 0 > /proc/kdrv_audio/dump_buf\n\n");
	seq_printf(sfile, "    To save file, set buf_write to 1\n");
	seq_printf(sfile, "    => echo 4 1 0 0 /mnt/mtd 100000 1 > /proc/kdrv_audio/dump_buf\n\n");
	seq_printf(sfile, "    To reset buf data, set buf_write to 2\n");
	seq_printf(sfile, "    => echo 4 1 0 0 /mnt/mtd 100000 2 > /proc/kdrv_audio/dump_buf\n\n");
	seq_printf(sfile, "current settings:\n");
	seq_printf(sfile, "    i2s_idx=%d, type=%d, ch=%d, mode=%d, path=%s, buf_size=%d, buf_write=%d\n", dev, type, ch, mode, dump_buf_path, dump_buf_size, dump_buf_write);

	return 0;
}

static ssize_t proc_write_kdrv_audioio_debug_level(struct file *file, const char __user *buffer, size_t count, loff_t *ppos)
{
	int len = count;
	unsigned char value[20] = {'\0'};
	uint new_value = 0;

	if (raw_copy_from_user(value, buffer, len)) {
		return EFAULT;
	}

	value[len] = '\0';
	sscanf(value, "%d\n", &new_value);

	kdrv_audioio_debug_level = new_value;
	printk("\n kdrv_audio debug_level = %d\n", kdrv_audioio_debug_level);
	return count;
}

static int write_buf_to_file(void)
{
	char *pchar;
	struct file *filp = NULL;
	char log_path[64] = "/tmp/au_log.bin";
	int ret = 0;

#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 14, 0))
	mm_segment_t fs;
#endif
	unsigned long long offset = 0;

#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 14, 0))
	fs = get_fs();
	set_fs(KERNEL_DS);
#endif
	sprintf(log_path, "%s/au_log_%s_i2s_idx%d_ch%02d.bin", dump_buf_path, (dump_buf_param[1] == 1) ? "out" : "in", dump_buf_param[0], dump_buf_param[2]);
	filp = filp_open(log_path, O_WRONLY | O_CREAT, 0777);
	if (IS_ERR(filp)) {
		printk("Error to open %s\n", log_path);
		goto returnit;
	}
	offset = 0;

	pchar = dump_buf_ptr;
	//sprintf(write_buf, "%s : dev=%d, ch=%d, size=%d, addr_va=%lx, timestamp=%lu\n\n", dir ? "input" : "output", hdl, handle, size, (long unsigned int)addr_va, get_nvt_jiffies());
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 14, 0))
	ret = vfs_write(filp, (unsigned char *)pchar, dump_buf_size, &offset);
#else
	ret = kernel_write(filp, (unsigned char *)pchar, dump_buf_size, &offset);
#endif
	filp_close(filp, NULL);
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 14, 0))
	set_fs(fs);
#endif

returnit:

	return 0;
}

static ssize_t proc_write_dump_buf(struct file *file, const char __user *buffer, size_t count, loff_t *ppos)
{
	int len = count;
	unsigned char value[60] = {'\0'};

	if (raw_copy_from_user(value, buffer, len)) {
		return 0;
	}

	value[len] = '\0';
	sscanf(value, "%x %x %x %x %s  %d %d\n", &dump_buf_param[0], &dump_buf_param[1], &dump_buf_param[2], &dump_buf_param[3], dump_buf_path, &dump_buf_size, &dump_buf_write);

	if (dump_buf_write == 1) {
		if (dump_buf_ptr) {
			printk("\nwrite buf to file\n");
			write_buf_to_file();
		} else {
			printk("\nno buf, please set dump_buf first\n");
		}

		dump_buf_write = 0;
	} else if (dump_buf_write == 2) { // clear buf
		printk("\nreset dump buf\n");
		dump_buf_size_cur = 0;
		dump_buf_write = 0;
	} else {
		printk("\ni2s_idx = %d, type = %d, ch = %d, mode = %d, path=%s, buf_size=%d, buf_write=%d\n", dump_buf_param[0], dump_buf_param[1], dump_buf_param[2], dump_buf_param[3], dump_buf_path, dump_buf_size, dump_buf_write);

		if (dump_buf_ptr) {
			kfree(dump_buf_ptr);
			dump_buf_ptr = NULL;
		}
		dump_buf_ptr = kzalloc(dump_buf_size, GFP_KERNEL);
		if (!dump_buf_ptr) {
			printk("dump_buf:alloc data buf failed\n");
		}
	}
	return count;
}



static int proc_open_kdrv_audioio_debug_level(struct inode *inode, struct file *file)
{
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3,16,0))
	return single_open(file, proc_read_kdrv_audioio_debug_level, PDE_DATA(inode));
#else
	return single_open(file, proc_read_kdrv_audioio_debug_level, PDE(inode)->data);
#endif
}

static int proc_open_dump_ch_cfg(struct inode *inode, struct file *file)
{
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3,16,0))
	return single_open(file, proc_read_dump_ch_cfg, PDE_DATA(inode));
#else
	return single_open(file, proc_read_dump_ch_cfg, PDE(inode)->data);
#endif
}

static int proc_open_dump_dts(struct inode *inode, struct file *file)
{
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3,16,0))
	return single_open(file, proc_read_dump_dts, PDE_DATA(inode));
#else
	return single_open(file, proc_read_dump_dts, PDE(inode)->data);
#endif
}

static int proc_open_dump_buf(struct inode *inode, struct file *file)
{
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3,16,0))
	return single_open(file, proc_read_dump_buf, PDE_DATA(inode));
#else
	return single_open(file, proc_read_dump_buf, PDE(inode)->data);
#endif
}

static struct proc_ops au_proc_kdrv_audioio_debug_level_ops = {
	.proc_open  = proc_open_kdrv_audioio_debug_level,
	.proc_write = proc_write_kdrv_audioio_debug_level,
	.proc_read   = seq_read,
	.proc_lseek = seq_lseek,
	.proc_release = single_release,
};

static struct proc_ops au_proc_dump_dts_ops = {
	.proc_open  = proc_open_dump_dts,
	.proc_read   = seq_read,
	.proc_lseek = seq_lseek,
	.proc_release = single_release,
};


static struct proc_ops au_proc_dump_ch_cfg_ops = {
	.proc_open  = proc_open_dump_ch_cfg,
	.proc_read   = seq_read,
	.proc_lseek = seq_lseek,
	.proc_release = single_release,
};

static struct proc_ops au_proc_dump_buf_ops = {
	.proc_open  = proc_open_dump_buf,
	.proc_write = proc_write_dump_buf,
	.proc_read  = seq_read,
	.proc_lseek = seq_lseek,
	.proc_release = single_release,
};


int au_proc_init(void)
{
	int ret = 0;

	DBG_FUNC("\n");

	dump_buf_param[1] = 2; //disable dump_buf
	/* create proc */
	entity_proc = proc_mkdir(AU_PROC_NAME, NULL);
	if (entity_proc == NULL) {
		printk("Error to create driver proc, please insert audio driver first.\n");
		ret = -EFAULT;
		goto err0;
	}
#if (LINUX_VERSION_CODE <= KERNEL_VERSION(2,6,30))
	entity_proc->owner = THIS_MODULE;
#endif

	au_proc_dump_dts_d = proc_create("dump_dts", S_IRUGO | S_IXUGO, entity_proc, &au_proc_dump_dts_ops);
	if (au_proc_dump_dts_d == NULL) {
		printk("%s fails: create proc node 'dump_dts' failed!\n", __func__);
		ret = -EINVAL;
		goto err11;
	}
#if (LINUX_VERSION_CODE <= KERNEL_VERSION(2,6,30))
	au_proc_dump_dts_d->owner = THIS_MODULE;
#endif

	au_proc_dump_ch_cfg_d = proc_create("dump_ch_cfg", S_IRUGO | S_IXUGO, entity_proc, &au_proc_dump_ch_cfg_ops);
	if (au_proc_dump_ch_cfg_d == NULL) {
		printk("%s fails: create proc node 'dump_ch_cfg' failed!\n", __func__);
		ret = -EINVAL;
		goto err12;
	}
#if (LINUX_VERSION_CODE <= KERNEL_VERSION(2,6,30))
	au_proc_dump_ch_cfg_d->owner = THIS_MODULE;
#endif

	au_proc_dump_buf_d = proc_create("dump_buf", S_IRUGO | S_IXUGO, entity_proc, &au_proc_dump_buf_ops);
	if (au_proc_dump_buf_d == NULL) {
		printk("%s fails: create proc node 'dump_buf' failed!\n", __func__);
		ret = -EINVAL;
		goto err13;
	}
#if (LINUX_VERSION_CODE <= KERNEL_VERSION(2,6,30))
	au_proc_dump_buf_d->owner = THIS_MODULE;
#endif

	au_proc_kdrv_audioio_debug_level_d = proc_create("dbglevel", S_IRUGO | S_IXUGO, entity_proc, &au_proc_kdrv_audioio_debug_level_ops);
	if (au_proc_kdrv_audioio_debug_level_d == NULL) {
		printk("%s fails: create proc node 'kdrv_audioio_debug_level' failed!\n", __func__);
		ret = -EINVAL;
		goto err14;
	}
#if (LINUX_VERSION_CODE <= KERNEL_VERSION(2,6,30))
	au_proc_kdrv_audioio_debug_level_d->owner = THIS_MODULE;
#endif

	return ret;
err14:
	remove_proc_entry("kdrv_audioio_debug_level", entity_proc);
err13:
	remove_proc_entry("dump_buf", entity_proc);
err12:
	remove_proc_entry("dump_ch_cfg", entity_proc);
err11:
	remove_proc_entry("dump_dts", entity_proc);
err0:
	remove_proc_entry(AU_PROC_NAME, NULL);
	return ret;
}

void au_proc_remove(void)
{
	DBG_FUNC("\n");
	if (au_proc_kdrv_audioio_debug_level_d) {
		remove_proc_entry("dbglevel", entity_proc);
	}

	if (au_proc_dump_buf_d) {
		remove_proc_entry("dump_buf", entity_proc);
	}

	if (au_proc_dump_ch_cfg_d) {
		remove_proc_entry("dump_ch_cfg", entity_proc);
	}

	if (au_proc_dump_dts_d) {
		remove_proc_entry("dump_dts", entity_proc);
	}

	if (entity_proc) {
		remove_proc_entry(AU_PROC_NAME, NULL);
	}
}
