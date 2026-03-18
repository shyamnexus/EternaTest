#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>
#include <linux/bitfield.h>
#include <linux/delay.h>
#include <linux/phy.h>
#include <linux/netdevice.h>

#include <plat/hardware.h>

#include "DWC_ETH_QOS_yheader.h"
#include "DWC_ETH_QOS_yapphdr.h"


#define ETH_EPHY_BASE		(0xF0453800)
#define ACG_MSK		0x7
#define ACG_BIT		4
#define SWING_CONNT 7

char path[128];
static int PGA_TABLE[8] = {20, 34, 40, 45, 54, 67, 67, 67};

static int ephy_normalized_swing_cal(struct seq_file *sfile, void *v)
{
	struct DWC_ETH_QOS_prv_data *pdata = (struct DWC_ETH_QOS_prv_data *)sfile->private;

	UINT32 val, ACG;
	int PGA_GAIN, COUNT,  FFE_tap4;
	unsigned long Normalized_swing, Normalized_swing_avg, temp;
	unsigned long Normalized_swing_tmp[10];
	void __iomem *ptr_ephy;
	int i, j;
	ptr_ephy = ioremap(ETH_EPHY_BASE, 16384);
	COUNT = 0;
	Normalized_swing = 0;

	while(COUNT < SWING_CONNT)
	{
		DWC_ETH_QOS_restart_device(pdata);
		mdelay(250);
		val =readl(ptr_ephy + 0x98);
		ACG = (val >> ACG_BIT) & ACG_MSK;
		//printk("ACG val = %d, val = %d\r\n",ACG ,val);
		PGA_GAIN = PGA_TABLE[ACG];
		phy_write(pdata->phydev, MII_RESV1, 3);
		//mdelay(500);
		FFE_tap4 = phy_read(pdata->phydev, MII_LBRERROR);
		//printk("FFE_tap4 = %d", FFE_tap4);
		Normalized_swing_tmp[COUNT] = (unsigned long)(32768000/FFE_tap4)/(unsigned long)(PGA_GAIN);
		//printk("Normalized_swing_100 = %ld, count = %d\r\n", Normalized_swing_tmp[COUNT], COUNT);
		COUNT++;

	}

	for(i = 0; i < SWING_CONNT; ++i) {
		for(j = 0; j < i; ++j) {
			if(Normalized_swing_tmp[j] > Normalized_swing_tmp[i]) {
				temp = Normalized_swing_tmp[j];
				Normalized_swing_tmp[j] = Normalized_swing_tmp[i];
				Normalized_swing_tmp[i] = temp;
			}
		}
	}


	for(i=1; i < SWING_CONNT-1;i++){
		Normalized_swing = Normalized_swing + Normalized_swing_tmp[i];
		//printk("Normalized_swing %ld, Normalized_swing_tmp[%d] = %ld", Normalized_swing, i, Normalized_swing_tmp[i]);
	}

	Normalized_swing_avg = Normalized_swing/(SWING_CONNT-2);
	//printk("Normalized_swing_avg_100 = %ld \r\n", Normalized_swing_avg);

	if(Normalized_swing_avg > 2445) printk("Estimated Cable Length: < 30m \r\n");
	else if((Normalized_swing_avg <= 2445) && (Normalized_swing_avg > 2104))printk("Estimated Cable Length: 30m ~ 50m \r\n");
	else if((Normalized_swing_avg <= 2104) && (Normalized_swing_avg > 1831))printk("Estimated Cable Length: 50m ~ 60m \r\n");
	else if((Normalized_swing_avg <= 1831) && (Normalized_swing_avg > 1633))printk("Estimated Cable Length: 60m ~ 70m \r\n");
	else if((Normalized_swing_avg <= 1633) && (Normalized_swing_avg > 1455))printk("Estimated Cable Length: 70m ~ 80m \r\n");
	else if((Normalized_swing_avg <= 1455) && (Normalized_swing_avg > 1274))printk("Estimated Cable Length: 80m ~ 90m \r\n");
	else if((Normalized_swing_avg <= 1274) && (Normalized_swing_avg > 1088))printk("Estimated Cable Length: 90m ~ 100m \r\n");
	else if(Normalized_swing_avg < 1088) printk("Estimated Cable Length: > 100m \r\n");

	printk("ephy_normalized_swing_cal finish\r\n");
	iounmap(ptr_ephy);

	return 0;
}

static int proc_eth_swing_open(struct inode *inode, struct file *file)
{
	return single_open(file, ephy_normalized_swing_cal, PDE_DATA(inode));
}

#if (LINUX_VERSION_CODE > KERNEL_VERSION(5, 10, 0))
static struct proc_ops proc_eth_swing_fops = {
	.proc_open    = proc_eth_swing_open,
	.proc_read    = seq_read,
	.proc_lseek  = seq_lseek,
	.proc_release = single_release,
};

#else
static struct file_operations proc_eth_swing_fops = {
	.owner   = THIS_MODULE,
	.open    = proc_eth_swing_open,
	.read    = seq_read,
	.llseek  = seq_lseek,
	.release = single_release,
};
#endif

int nvt_eth_proc_init(struct DWC_ETH_QOS_prv_data *pdata, int idx)
{
	int ret = 0;
	struct proc_dir_entry *pmodule_root = NULL;
	struct proc_dir_entry *pentry = NULL;

	snprintf(path, sizeof(path), "nvt_eth%d", idx);

	pmodule_root = proc_mkdir(path, NULL);
		if (pmodule_root == NULL) {
		printk(KERN_ERR "failed to create eth Module root\n");
		ret = -EINVAL;
		goto remove_root;
	}

	pdata->pproc_module_root = pmodule_root;
	if(pdata->embd_phy == 1)
	{
		pentry = proc_create_data("eth_swing", S_IRUGO | S_IXUGO, pmodule_root, &proc_eth_swing_fops, (void*)pdata);
		if (pentry == NULL) {
			printk("error to proc_eth_swing proc\n");
			goto remove_cmd;
		}
	}
	pdata->pproc_eth_swing = pentry;

	return ret;

remove_cmd:
	proc_remove(pentry);

remove_root:
	proc_remove(pmodule_root);
	return ret;

}

int nvt_eth_proc_remove(struct DWC_ETH_QOS_prv_data *pdata)
{
	if(pdata->pproc_eth_swing)  proc_remove(pdata->pproc_eth_swing);
	if(pdata->pproc_module_root) 	proc_remove(pdata->pproc_module_root);

	return 0;
}
