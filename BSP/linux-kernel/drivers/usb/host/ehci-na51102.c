/*
 * Klins Chen <klins_chen@novatek.com.tw>
 *
 */
#include <linux/clk.h>
#include <linux/spinlock.h>
#include <linux/interrupt.h>
#include <linux/of_address.h>
#include <plat/efuse_protected.h>
#include <scsi/scsi_host.h>
#include <linux/proc_fs.h>
#include <linux/kthread.h>
#include <linux/sched.h>
#include <linux/sysfs.h>
#include <uapi/linux/sched/types.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/writeback.h>
#include <linux/gpio.h>

#define DRV_VERSION "1.00.004"
#define loc_cpu(lock, flags) spin_lock_irqsave(lock, flags)
#define unl_cpu(lock, flags) spin_unlock_irqrestore(lock, flags)

DEFINE_SPINLOCK(task_remove);
extern int ehci_mask_connect;
int remove_thread = 0;
extern int ehci_mask_connect;

#define USB_SIMULATE_POWEROFF 0

#ifdef CONFIG_DMA_COHERENT
#define USBH_ENABLE_INIT  (USBH_ENABLE_CE \
						   | USB_MCFG_PFEN | USB_MCFG_RDCOMB \
						   | USB_MCFG_SSDEN | USB_MCFG_UCAM \
						   | USB_MCFG_EBMEN | USB_MCFG_EMEMEN)
#else
#define USBH_ENABLE_INIT  (USBH_ENABLE_CE \
						   | USB_MCFG_PFEN | USB_MCFG_RDCOMB \
						   | USB_MCFG_SSDEN \
						   | USB_MCFG_EBMEN | USB_MCFG_EMEMEN)
#endif

static void u3phy_setreg(struct ehci_hcd *ehci, u32 ofs, u32 val)
{
#ifndef CONFIG_NVT_FPGA_EMULATION
        writel(val, (ehci->u3phy_reg + ofs));
#endif
}

static u32 u3phy_getreg(struct ehci_hcd *ehci, u32 ofs)
{
#ifndef CONFIG_NVT_FPGA_EMULATION
        return readl(ehci->u3phy_reg + ofs);
#else
        return 0;
#endif
}

static void u2phy_init_setreg(struct ehci_hcd *ehci, u32 ofs, u32 val)
{
#ifndef CONFIG_NVT_FPGA_EMULATION
        writel(val, (ehci->u3_utmi_phy_reg + ofs));
#endif
}

#if 0 //no use in 51102
static u32 u2phy_init_getreg(struct ehci_hcd *ehci, u32 ofs)
{
#ifndef CONFIG_NVT_FPGA_EMULATION
        return readl(ehci->u3_utmi_phy_reg + ofs);
#else
        return 0;
#endif
}
#endif

static void u2phy_setreg(struct ehci_hcd *ehci, u32 ofs, u32 val)
{
#ifndef CONFIG_NVT_FPGA_EMULATION
	writel(val, (ehci->phy_reg + ofs));
#endif
}

static u32 u2phy_getreg(struct ehci_hcd *ehci, u32 ofs)
{
#ifndef CONFIG_NVT_FPGA_EMULATION
	return readl(ehci->phy_reg + ofs);
#else
	return 0;
#endif
}

static void nvt_remove_power_save(struct platform_device *dev)
{
#ifdef CONFIG_POWER_SAVE_OPTION
	struct usb_hcd  *hcd = platform_get_drvdata(dev);
	struct ehci_hcd *ehci = hcd_to_ehci(hcd);
	unsigned long       usbbase;
	unsigned long       tmpval;
	usbbase = (unsigned long)ehci->caps;

	//printk("enter power save +++\n");
	/* Host Reset */
	writel((readl((volatile unsigned long *)(usbbase + 0x10)) | 0x2), (volatile unsigned long *)(usbbase + 0x10));

	u2phy_setreg(ehci, 0xd4, 0xb4);
	mdelay(500);
	u2phy_init_setreg(ehci, 0xd4, 0xb4);
	mdelay(500);
	u2phy_setreg(ehci, 0xd0, 0xff);
	mdelay(500);
	u2phy_init_setreg(ehci, 0xd0, 0xff);
	mdelay(500);
	u2phy_setreg(ehci, 0x08, 0x14);
	mdelay(500);
	u2phy_init_setreg(ehci, 0x08, 0x14);
	mdelay(500);
	u2phy_setreg(ehci, 0x00, 0x8a);
	mdelay(500);
	u2phy_init_setreg(ehci, 0x00, 0x8a);
	mdelay(500);
	u2phy_setreg(ehci, 0x30, 0x35);
	mdelay(500);
	u2phy_init_setreg(ehci, 0x30, 0x35);
	mdelay(500);
	u2phy_setreg(ehci, 0x2c, 0xff);
	mdelay(500);
	u2phy_init_setreg(ehci, 0x2c, 0xff);
	mdelay(500);
	u2phy_setreg(ehci, 0x28, 0x20);
	mdelay(500);
	u2phy_init_setreg(ehci, 0x28, 0x20);
	mdelay(500);
	u2phy_setreg(ehci, 0x24, 0xb3);
	mdelay(500);
	u2phy_init_setreg(ehci, 0x24, 0xb3);
	mdelay(500);
	u2phy_setreg(ehci, 0x34, 0x90);
	mdelay(500);
	u2phy_init_setreg(ehci, 0x34, 0x90);
	mdelay(500);
	//printk("enter power save ---\n");

	/* clear U2 CLK reset flag */
        tmpval = readl(ehci->u3ctrl_reg + 0x28);
        if(tmpval&0x80) {
		//printk("ehci-na51102 : clear phy reset flag(u3 reg 0x28[7])\n");
		tmpval = tmpval & ~(0x80);
		writel(tmpval, ehci->u3ctrl_reg + 0x28);

		//printk("0x28 is 0x%x\n", readl(ehci->u3ctrl_reg + 0x28));
        } else {
		//printk("err\r\n");
        }
#endif
	return;
}

static ssize_t nvt_efuse_sysfs_show(struct device *dev,
									struct device_attribute *attr,
									char *buf)
{
	struct platform_device *pdev = to_platform_device(dev);
	struct usb_hcd *hcd = platform_get_drvdata(pdev);
	struct ehci_hcd *ehci = hcd_to_ehci(hcd);

	u32 data = ehci->store_efuse_data[0] | (ehci->store_efuse_data[1] << 8) | (ehci->store_efuse_data[2] << 16);
	return snprintf(buf, 32, "0x%x\n", data);
}

static DEVICE_ATTR(efuse, S_IWUSR | S_IRUGO,
				   nvt_efuse_sysfs_show, NULL);

static ssize_t nvt_runtime_sysfs_show(struct device *dev,
									  struct device_attribute *attr,
									  char *buf)
{
	struct platform_device *pdev = to_platform_device(dev);
	struct usb_hcd *hcd = platform_get_drvdata(pdev);
	struct ehci_hcd *ehci = hcd_to_ehci(hcd);
	u32 data = 0x0, reg_value = 0x0;
	u8 u2_trim_swctrl, u2_trim_sqsel, u2_trim_resint;

	reg_value = u2phy_getreg(ehci, 0x18);
	reg_value &= (0x7 << 1);
	u2_trim_swctrl = reg_value >> 1;

	reg_value = u2phy_getreg(ehci, 0x14);
	reg_value &= (0x7 << 0x2);
	u2_trim_sqsel = reg_value >> 0x2;

	reg_value = u2phy_getreg(ehci, 0x148);
	u2_trim_resint = reg_value & 0x1F;

	data = u2_trim_swctrl | (u2_trim_sqsel << 8) | (u2_trim_resint << 16);

	return snprintf(buf, 32, "0x%x\n", data);
}

static DEVICE_ATTR(runtime, S_IWUSR | S_IRUGO,
				   nvt_runtime_sysfs_show, NULL);

static struct attribute *nvt_efuse_sysfs_attributes[] = {
	&dev_attr_efuse.attr,
	&dev_attr_runtime.attr,
	NULL,
};

static const struct attribute_group nvt_efuse_sysfs_attr_group = {
	.attrs = nvt_efuse_sysfs_attributes,
};

static int nvt_efuse_sysfs_init(struct device *dev)
{
	return sysfs_create_group(&dev->kobj, &nvt_efuse_sysfs_attr_group);
}

static void nvt_efuse_sysfs_remove(struct device *dev)
{
	sysfs_remove_group(&dev->kobj, &nvt_efuse_sysfs_attr_group);
}

static int monitor_daemon_port(void *data)
{
	struct ehci_hcd *ehci = data;
	unsigned long usbbase = (unsigned long)ehci->caps;
	unsigned long tmpval;
	void __iomem *phy_reg;

	phy_reg = ehci->phy_reg;

	sched_set_fifo_low(current);
	current->flags |= PF_NOFREEZE;

	set_current_state(TASK_INTERRUPTIBLE);

	writel(0x80, phy_reg + 0xCC);

	while (!kthread_should_stop()) {
		tmpval = readl((void __iomem *)(usbbase + 0x80));
		tmpval = (tmpval >> 22) & 0x3;

		if ((remove_thread == 0) && (tmpval == 0x2)) {
			writel(0x80, phy_reg + 0xC8);
			readl(phy_reg + 0x80);
			writel(0x0, phy_reg + 0xC8);
		}

		schedule_timeout_interruptible(msecs_to_jiffies(1000));
	}

	set_current_state(TASK_RUNNING);

	return 0;
}

/*
#ifdef CONFIG_PM
static void nvtim_plat_resume(struct usb_hcd *hcd)
{
	struct ehci_hcd *ehci = hcd_to_ehci(hcd);
	int retval, retry = 0;

	ehci_writel(ehci, ehci->periodic_dma, &ehci->regs->frame_list);
	ehci_writel(ehci, (unsigned long)ehci->async->qh_dma, &ehci->regs->async_next);
	retry = 0;

	do {
		ehci_writel(ehci, INTR_MASK,
			&ehci->regs->intr_enable);

		retval = ehci_handshake(ehci, &ehci->regs->intr_enable,
			INTR_MASK, INTR_MASK, 250);
		retry++;
	} while (retval != 0);

	if (unlikely(retval != 0))
		ehci_err(ehci, "write fail!\n");
}
#endif
*/

static void nvtim_eof_init(struct ehci_hcd *ehci)
{
	unsigned long usbbase = (unsigned long)ehci->caps;
	unsigned long tmpval;

	/* Clear EOF1=3[3:2] EOF2[5:4]=0 */
	tmpval = readl((volatile unsigned long *)(usbbase + 0x40));
	tmpval &= ~(0x3 << 4);
	tmpval &= ~(0x3 << 2);
	tmpval |= (0x3 << 2);
	writel(tmpval, (volatile unsigned long *)(usbbase + 0x40));
}

#define TRIM_RESINT_UPPER 0x14
#define TRIM_RESINT_LOWER 0x1
static void nvtim_init_usbhc(struct ehci_hcd *ehci)
{
	unsigned long       usbbase;
	unsigned long       tmpval = 0;
	struct clk       *clk;

	usbbase = (unsigned long)ehci->caps;

	tmpval = readl(ehci->u3ctrl_reg + 0x28);
	if(tmpval&0x80) {
		//printk("ehci-na51102 : phy already reset by other driver\n");
	} else {
		//printk("ehci-na51102 re-active +++\n");
		clk = clk_get(NULL, "u2phy");
		if (!IS_ERR(clk)) {
            clk_prepare_enable(clk);
			clk_disable_unprepare(clk);
		} else {
			pr_err("%s: clk u2phy not found\n", __func__);
		}
		ehci_writel(ehci, INTR_MASK,
				&ehci->regs->intr_enable);
		//printk("ehci-na51102 re-active ---\n");
		tmpval |= 0x80;
		writel(tmpval, ehci->u3ctrl_reg + 0x28);
	}

	/* Set USB ID & VBUSI */
	tmpval = readl((volatile unsigned long *)(usbbase + 0x400));
	tmpval &= ~0xFF;
	tmpval |= 0x22;
	tmpval |= (0x1 << 20);
	tmpval &= ~(0x1 << 21);
	writel(tmpval, (volatile unsigned long *)(usbbase + 0x400));
	udelay(64);

	/* Clear DEVPHY_SUSPEND[31] */
	tmpval = readl((volatile unsigned long *)(usbbase + 0x1C8));
	tmpval &= ~(0x1 << 31);
	writel(tmpval, (volatile unsigned long *)(usbbase + 0x1C8));

	/* Clear HOSTPHY_SUSPEND[6] */
	tmpval = readl((volatile unsigned long *)(usbbase + 0x40));
	tmpval &= ~(0x1 << 6);
	writel(tmpval, (volatile unsigned long *)(usbbase + 0x40));
	udelay(64);

	/* Clear FORCE_FS[9] and handle HALF_SPEED[1] */
	tmpval = readl((volatile unsigned long *)(usbbase + 0x100));
	tmpval &= ~(0x1 << 9);
#ifdef CONFIG_NVT_FPGA_EMULATION
	tmpval |= (0x1 << 1);
#endif

	writel(tmpval, (volatile unsigned long *)(usbbase + 0x100));

	/* Clear EOF1=3[3:2] EOF2[5:4]=0 */
	tmpval = readl((volatile unsigned long *)(usbbase + 0x40));
	tmpval &= ~(0x3 << 4);
	tmpval |= (0x3 << 2);
	tmpval &= ~(0x3F << 16);
	tmpval |= (0x22 << 16);
	writel(tmpval, (volatile unsigned long *)(usbbase + 0x40));

#if USB_SIMULATE_POWEROFF
	printk("R0x40=0x%08X\r\n", readl((volatile unsigned long *)(usbbase + 0x40)));
#endif

	/* A_BUS_DROP[5] = 0 */
	tmpval = readl((volatile unsigned long *)(usbbase + 0x80));
	tmpval &= ~(0x1 << 5);
	tmpval &= ~(0x1 << 10);
	writel(tmpval, (volatile unsigned long *)(usbbase + 0x80));

	udelay(2);

	/* A_BUS_REQ[4] = 1 */
	tmpval = readl((volatile unsigned long *)(usbbase + 0x80));
	tmpval |= (0x1 << 4);
	writel(tmpval, (volatile unsigned long *)(usbbase + 0x80));

	/* RESP MODE open */
	tmpval = readl((volatile unsigned long *)(usbbase + 0x404));
	tmpval |= (0x1 << 30);
	writel(tmpval, (volatile unsigned long *)(usbbase + 0x404));

	/* AXI channel open */
	tmpval = readl((volatile unsigned long *)(usbbase + 0x404));
	tmpval &= ~(0x1 << 16);
	writel(tmpval, (volatile unsigned long *)(usbbase + 0x404));

#ifndef CONFIG_NVT_FPGA_EMULATION
	/* Configure PHY related settings below */
	{
		UINT16 data = 0;
		INT32 result = 0;
		u8 u2_trim_swctrl = 4, u2_trim_sqsel = 4, u3_trim_rint_sel = 8;

		result = efuse_readParamOps(EFUSE_USB_SATA_TRIM_DATA, &data);
		if (result == 0) {
			pr_info("[530] USB = 0x%04x\r\n", (int)data);
			if ((TRIM_RESINT_LOWER <= (data&0x1F)) && ((data&0x1F) <= TRIM_RESINT_UPPER)) {
				u3_trim_rint_sel = data & 0x1F;
				pr_info("[530] USB Trim data(12K) = 0x%04x\r\n", (int)u3_trim_rint_sel);
			} else {
				pr_err("[530] data is out of valid range\n");
			}
		} else {
			pr_err("[530] data = NULL\n");
		}

		//select PHY internal rsel value
		tmpval = u3phy_getreg(ehci, 0x684);
		tmpval &= ~(0x80);
		u3phy_setreg(ehci, 0x684, tmpval);

		//r45 trim setting
		if (ehci->u3phy_reg) {
			tmpval = u3phy_getreg(ehci, 0x680);
			if ((tmpval & 0xF) == 0) {
				u3phy_setreg(ehci, 0x680, 0x40 + u3_trim_rint_sel);
				u3phy_setreg(ehci, 0x68C, 0x60);
			}
		}

		//best setting
		u3phy_setreg(ehci, 0x158, 0x74);
		mdelay(5);
		u2phy_init_setreg(ehci, 0x24, 0xb0);
		mdelay(5);
		u2phy_setreg(ehci, 0x24, 0xb0);
		mdelay(5);
		u2phy_init_setreg(ehci, 0x48, 0x31);
		mdelay(5);
		u2phy_setreg(ehci, 0x48, 0x31);
		mdelay(5);

		//r45 cali
		u2phy_init_setreg(ehci, 0x144, 0x20);
		mdelay(5);
		u2phy_init_setreg(ehci, 0x144, 0x00);
		mdelay(5);

		ehci->store_efuse_data[0] = u2_trim_swctrl;
		ehci->store_efuse_data[1] = u2_trim_sqsel;
		ehci->store_efuse_data[2] = u3_trim_rint_sel;

		if ((ehci->tx_swing >= 0) && (ehci->tx_swing <= 7)) {
			u2_trim_swctrl = ehci->tx_swing;
		}

		if ((ehci->squelch >= 0) && (ehci->squelch <= 7)) {
			u2_trim_sqsel = ehci->squelch;
		}

		tmpval = u2phy_getreg(ehci, 0x18);
		tmpval &= ~(0x7 << 1);
		tmpval |= (u2_trim_swctrl << 0x1);
		u2phy_setreg(ehci, 0x18, tmpval);

		tmpval = u2phy_getreg(ehci, 0x14);
		tmpval &= ~(0x7 << 0x2);
		tmpval |= (u2_trim_sqsel << 0x2);
		u2phy_setreg(ehci, 0x14, tmpval);

		ehci->efuse_data[0] = u2_trim_swctrl;
		ehci->efuse_data[1] = u2_trim_sqsel;
		ehci->efuse_data[2] = u3_trim_rint_sel;
	}
#endif
	writel(8, (volatile unsigned long *)(usbbase + 0xC4));
}

static void nvtim_stop_usbhc(struct platform_device *dev)
{
	struct usb_hcd  *hcd = platform_get_drvdata(dev);
	struct ehci_hcd *ehci = hcd_to_ehci(hcd);
	unsigned long       usbbase;
	unsigned long       tmpval = 0;

	usbbase = (unsigned long)ehci->caps;

	/* Host Reset */
	writel((readl((volatile unsigned long *)(usbbase + 0x10)) | 0x2), (volatile unsigned long *)(usbbase + 0x10));

	/* A_BUS_REQ[4] = 0 */
	tmpval = readl((volatile unsigned long *)(usbbase + 0x80));
	tmpval &= ~(0x1 << 4);
	writel(tmpval, (volatile unsigned long *)(usbbase + 0x80));

	/* A_BUS_DROP[5] = 1 */
	tmpval = readl((volatile unsigned long *)(usbbase + 0x80));
	tmpval |= (0x1 << 5);
	writel(tmpval, (volatile unsigned long *)(usbbase + 0x80));

	/* Toggle reset of D-PHY and Controller to avoid unexpected situations */
	{
		struct clk *source_clk;

		/* Toggle D-PHY */
		source_clk = clk_get(&dev->dev, "u2phy");
		if (IS_ERR(source_clk)) {
			dev_err(&dev->dev, "%s: skip to get clock u2phy and go to toggle phyrstbase\n", __func__);
			tmpval = readl((volatile unsigned long *)(usbbase + 0x400));
			tmpval |= (0x1 << 17);
			writel(tmpval, (volatile unsigned long *)(usbbase + 0x400));

			tmpval &= ~(0x1 << 17);
			writel(tmpval, (volatile unsigned long *)(usbbase + 0x400));
		} else {
			clk_prepare_enable(source_clk);
			clk_disable_unprepare(source_clk);
			clk_put(source_clk);
		}

		/* Toggle Controller */
		source_clk = clk_get(&dev->dev, dev_name(&dev->dev));
		if (IS_ERR(source_clk)) {
			dev_err(&dev->dev, "%s: can't find clock %s\n", __func__, dev_name(&dev->dev));
			source_clk = NULL;
		} else {
			/* toggle reset */
			clk_disable_unprepare(source_clk);
			clk_prepare_enable(source_clk);

			/* toggle reset during the next initialization */
			clk_disable_unprepare(source_clk);

			clk_put(source_clk);
		}
	}

	if (ehci->power_gpio[0] > 0)
		gpio_direction_output(ehci->power_gpio[0], !ehci->power_gpio[1]);
}

static void nvtim_fuse_log(struct ehci_hcd *ehci)
{
#if 0
	unsigned long fusedata = 0;
	unsigned long   usbbase;

	usbbase = (unsigned long)ehci->caps;

	fusedata += ((readl((volatile unsigned long *)(usbbase + (0x1000 + (0x6 << 2)))) & 0xFF) << 8);
	fusedata += ((readl((volatile unsigned long *)(usbbase + (0x1000 + (0x5 << 2)))) & 0xFF) << 16);
	ehci->fuseData = fusedata;
	//printk("ehci->fuseData = 0x%08X\n",ehci->fuseData);
#endif
}

// static int nvtim_check_port_config(void)
// {
// struct device_node *node;

// node = of_find_node_by_path("/usb20@f0390000");
// if (node)
// return 1;
// else
// return 0;
// }

/* configure so an HC device and id are always provided */
/* always called with process context; sleeping is OK */

/**
 * nvtim_usb_ehci_probe - initialize nt96660-based HCDs
 * Context: !in_interrupt()
 *
 * Allocates basic resources for this USB host controller, and
 * then invokes the start() method for the HCD associated with it
 * through the hotplug entry's driver_data.
 *
 */
static int nvtim_usb_ehci_probe(const struct hc_driver *driver,
								struct usb_hcd **hcd_out, struct platform_device *dev)
{
	struct device_node *dn = dev->dev.of_node;
	struct usb_hcd *hcd;
	struct ehci_hcd *ehci;
	struct resource res_mem;
	int retval;
	int irq;
	struct clk *source_clk;
	u32 channel, get_tx_swing, get_squelch, patch_thread = 0x0;
	u32 get_ep_id, get_power_gpio[2];
	int tx_swing, squelch, ep_id, power_gpio[2];
	struct task_struct *p_mon;

#ifdef CONFIG_OF
	irq = platform_get_irq(dev, 0);
	if (irq < 0) {
		pr_debug("resource[1] is not IORESOURCE_IRQ");
		return -ENXIO;
	}

	if (of_property_read_u32(dev->dev.of_node, "channel", &channel)) {
		printk("channel not set");
	}

	if (!of_property_read_u32(dev->dev.of_node, "patch_disconnect", &patch_thread)) {
		printk("open patch_disconnect!");
	}

	if (of_property_read_u32(dev->dev.of_node, "tx_swing", &get_tx_swing)) {
		tx_swing = -1;
	} else {
		tx_swing = get_tx_swing;
	}

	if (of_property_read_u32(dev->dev.of_node, "squelch", &get_squelch)) {
		squelch = -1;
	} else {
		squelch = get_squelch;
	}

	if (of_property_read_u32(dev->dev.of_node, "ep_id", &get_ep_id)) {
		ep_id = -1;
	} else {
		ep_id = get_ep_id;
	}

	if (of_property_read_u32_array(dev->dev.of_node, "power_gpio", get_power_gpio, 2)) {
		power_gpio[0] = -1;
		power_gpio[1] = -1;
	} else {
		power_gpio[0] = get_power_gpio[0];
		power_gpio[1] = get_power_gpio[1];

		gpio_direction_output(power_gpio[0], !power_gpio[1]);
		mdelay(200);
	}
#else
	if (dev->resource[1].flags != IORESOURCE_IRQ) {
		pr_debug("resource[1] is not IORESOURCE_IRQ");
		retval = -ENOMEM;
		return retval;
	}
	irq = dev->resource[1].start;
#endif

	retval = of_address_to_resource(dn, 0, &res_mem);
	if (retval) {
		dev_err(&dev->dev, "no address resource provided for index 0");
		return retval;
	}

	hcd = usb_create_hcd(driver, &dev->dev, driver->description);
	if (!hcd) {
		return -ENOMEM;
	}

	hcd->product_desc = driver->product_desc;
	hcd->rsrc_start = res_mem.start;
	hcd->rsrc_len = resource_size(&res_mem);

	hcd->regs = devm_ioremap_resource(&dev->dev, &res_mem);
	if (!hcd->regs) {
		pr_debug("ioremap failed");
		retval = -ENOMEM;
		goto err_register;
	}

	/* This part is used in debug usage for using real usb addr. */
	/* hcd->regs =(void __iomem*) hcd->rsrc_start;
	 * if (!hcd->regs) {
	 *  pr_debug("ioremap failed");
	 *  retval = -ENOMEM;
	 *  goto err_register;
	 * }
	 */

	source_clk = clk_get(&dev->dev, dev_name(&dev->dev));
	if (IS_ERR(source_clk)) {
		dev_err(&dev->dev, "%s: can't find clock %s\n", __func__, dev_name(&dev->dev));
		source_clk = NULL;
	} else {
		clk_prepare_enable(source_clk);
		clk_put(source_clk);
	}

	ehci = hcd_to_ehci(hcd);
	ehci->caps = hcd->regs;
	ehci->regs = (void __iomem *)ehci->caps +
				 HC_LENGTH(ehci, readl((volatile unsigned long *)(&ehci->caps->hc_capbase)));

	ehci->sbrn = HCD_USB2;

	// coverity[side_effect_free]: spin_lock_init is kernel API.
	spin_lock_init(&ehci->host_lock);

#ifndef CONFIG_NVT_FPGA_EMULATION
	ehci->phy_reg = of_iomap(dn, 1);
	if (!ehci->phy_reg) {
		dev_err(&dev->dev, "no address resource provided for index 1");
		retval = -ENOMEM;
		goto err_map;
	} else {
		pr_info("get the phy addr: 0x%px\n", ehci->phy_reg);
	}

	ehci->u3phy_reg = of_iomap(dn, 2);
	if (!ehci->u3phy_reg) {
		dev_err(&dev->dev, "no address resource provided for index 2");
		retval = -ENOMEM;
		goto err_map;
	}  else {
		pr_info("get the u3phy addr: 0x%px\n", ehci->u3phy_reg);
	}

	ehci->u3_utmi_phy_reg = of_iomap(dn, 3);
	if (!ehci->u3_utmi_phy_reg) {
		dev_err(&dev->dev, "no address resource provided for index 3");
		retval = -ENOMEM;
		goto err_map;
	}  else {
		pr_info("get the u3_utmi_phy_reg addr: 0x%px\n", ehci->u3_utmi_phy_reg);
	}

	ehci->u3ctrl_reg = of_iomap(dn, 4);
	if (!ehci->u3ctrl_reg) {
		dev_err(&dev->dev, "no address resource provided for index 4");
		retval = -ENOMEM;
		goto err_map;
	}  else {
		pr_info("get the u3ctrl_reg addr: 0x%px\n", ehci->u3ctrl_reg);
	}
#endif

	/* cache this readonly data; minimize chip reads */
	ehci->hcs_params = ehci_readl(ehci, &ehci->caps->hcs_params);

	ehci->ep_id = ep_id;

	ehci->channel = channel;

	ehci->tx_swing = tx_swing;

	ehci->squelch = squelch;

	ehci->patch_thread = patch_thread;

	ehci->power_gpio[0] = power_gpio[0];

	ehci->power_gpio[1] = power_gpio[1];

	nvtim_fuse_log(ehci);
	nvtim_init_usbhc(ehci);

	if (patch_thread) {
		p_mon = kthread_run(monitor_daemon_port, ehci, "usb_monitor");
		if (IS_ERR(p_mon)) {
			pr_err("create monitor_daemon_port failed!\n");
			ehci->task_nvt_daemon = NULL;
		} else {
			ehci->task_nvt_daemon = p_mon;
		}
	}

	retval = nvt_efuse_sysfs_init(&dev->dev);
	if (retval < 0) {
		dev_err(&dev->dev, "create sysfs failed!\n");
	}

	if (power_gpio[0] > 0) {
		gpio_direction_output(power_gpio[0], power_gpio[1]);
		mdelay(200);
	}

	retval = usb_add_hcd(hcd, irq, IRQF_SHARED);
	if (retval == 0) {
		dev_info(&dev->dev, "ver %s\n", DRV_VERSION);
		platform_set_drvdata(dev, hcd);
		return retval;
	}

	nvtim_stop_usbhc(dev);

err_map:
	iounmap(ehci->regs);
	iounmap(ehci->phy_reg);
	iounmap(ehci->u3phy_reg);
	iounmap(ehci->u3_utmi_phy_reg);
err_register:
	release_mem_region(hcd->rsrc_start, hcd->rsrc_len);
	usb_put_hcd(hcd);

	return retval;
}

static void nvt_remove_thread(struct usb_hcd *hcd)
{
	struct ehci_hcd *ehci = hcd_to_ehci(hcd);
	struct task_struct *p_mon;

	p_mon = ehci->task_nvt_daemon;
	if (p_mon) {
		kthread_stop(p_mon);
		ehci->task_nvt_daemon = NULL;
	}
}

/* may be called without controller electrically present */
/* may be called with controller, bus, and devices active */
static void nvtim_usb_ehci_remove(struct usb_hcd *hcd,
								  struct platform_device *dev)
{
	unsigned long flags = 0x0;
	spin_lock_irqsave(&task_remove, flags);
	remove_thread = 1;
	spin_unlock_irqrestore(&task_remove, flags);

	nvt_remove_thread(hcd);

	nvt_efuse_sysfs_remove(&dev->dev);

	usb_remove_hcd(hcd);

	nvtim_stop_usbhc(dev);

	nvt_remove_power_save(dev);

	//release_mem_region(hcd->rsrc_start, hcd->rsrc_len);
	pr_debug("calling usb_put_hcd\n");

	usb_put_hcd(hcd);
}

static struct proc_dir_entry *nvt_ehci_proc_root    = NULL;
static struct proc_dir_entry *nvt_ehci_proc_mc   = NULL;

static int nvt_ehci_mc_show(struct seq_file *sfile, void *v)
{
	seq_printf(sfile, "ehci_mask_connect %d\n", ehci_mask_connect);
	return 0;
}

static int nvt_ehci_mc_open(struct inode *inode, struct file *file)
{
	return single_open(file, nvt_ehci_mc_show, PDE_DATA(inode));
}



static ssize_t nvt_ehci_mc_write(struct file *file, const char __user *buffer,
								 size_t count, loff_t *ppos)
{
	unsigned int value_in;
	char value_str[32] = {'\0'};

	if (copy_from_user(value_str, buffer, count)) {
		return -EFAULT;
	}

	value_str[count] = '\0';
	sscanf(value_str, "%x\n", &value_in);
	//printk("ehci_mask_connect %d\n", value_in);
	ehci_mask_connect = value_in;

	return count;
}

static struct proc_ops nvt_ehci_mc_ops = {
	.proc_open   = nvt_ehci_mc_open,
	.proc_write  = nvt_ehci_mc_write,
	.proc_read   = seq_read,
	.proc_lseek = seq_lseek,
	.proc_release = single_release,
};

int nvt_ehci_proc_init(void)
{
	const char name[16] = "nvt_ehci";
	struct proc_dir_entry *root;
	int ret = 0;
	static int init = 0;

	if (init) {
		return 0;
	}

	root = proc_mkdir(name, NULL);
	if (!root) {
		ret = -ENOMEM;
		goto end;
	}

	nvt_ehci_proc_root = root;

	nvt_ehci_proc_mc = proc_create("ehci_mask_connect", S_IRUGO | S_IXUGO, nvt_ehci_proc_root, &nvt_ehci_mc_ops);
	if (!nvt_ehci_proc_mc) {
		printk("create proc node 'nvt_sata_reset_trigger' failed!\n");
		ret = -EINVAL;
		goto err1;
	}

	init = 1;

	return ret;

err1:
	proc_remove(nvt_ehci_proc_root);

end:
	return ret;

}

void nvt_ehci_proc_exit(void)
{

	if (nvt_ehci_proc_root) {
		static int init = 0;

		if (init) {
			return;
		}

		proc_remove(nvt_ehci_proc_root);
		nvt_ehci_proc_root = NULL;
		init = 1;
	}
}

static int nvtim_ehci_init(struct usb_hcd *hcd)
{
	struct ehci_hcd *ehci = hcd_to_ehci(hcd);
	unsigned long temp;
	int retval;
	unsigned long hcc_params;
	struct ehci_qh_hw   *hw;

	spin_lock_init(&ehci->lock);

	ehci->need_io_watchdog = 1;

	hrtimer_init(&ehci->hrtimer, CLOCK_MONOTONIC, HRTIMER_MODE_ABS);
	ehci->hrtimer.function = ehci_hrtimer_func;
	ehci->next_hrtimer_event = EHCI_HRTIMER_NO_EVENT;

	ehci->uframe_periodic_max = 100;

	/*
	 * hw default: 1K periodic list heads, one per frame.
	 * periodic_size can shrink by USBCMD update if hcc_params allows.
	 */
	ehci->periodic_size = DEFAULT_I_TDPS;

	INIT_LIST_HEAD(&ehci->async_unlink);
	INIT_LIST_HEAD(&ehci->async_idle);
	INIT_LIST_HEAD(&ehci->intr_unlink);
	INIT_LIST_HEAD(&ehci->intr_unlink_wait);
	INIT_LIST_HEAD(&ehci->intr_qh_list);
	INIT_LIST_HEAD(&ehci->cached_itd_list);
	INIT_LIST_HEAD(&ehci->cached_sitd_list);
	INIT_LIST_HEAD(&ehci->tt_list);

	if (dma_set_mask(hcd->self.controller, DMA_BIT_MASK(32)) < 0)
		printk("can't set DMA mask\n");

	retval = ehci_mem_init(ehci, GFP_KERNEL);
	if (retval < 0) {
		return retval;
	}

	/* controllers may cache some of the periodic schedule ... */
	hcc_params = ehci_readl(ehci, &ehci->caps->hcc_params);
	if (HCC_ISOC_CACHE(hcc_params)) { /* full frame cache */
		ehci->i_thresh = 8;
	} else {            /* N microframes cached */
		ehci->i_thresh = 2 + HCC_ISOC_THRES(hcc_params);
	}

	/*
	 * dedicate a qh for the async ring head, since we couldn't unlink
	 * a 'real' qh without stopping the async schedule [4.8].  use it
	 * as the 'reclamation list head' too.
	 * its dummy is used in hw_alt_next of many tds, to prevent the qh
	 * from automatically advancing to the next td after short reads.
	 */
	ehci->async->qh_next.qh = NULL;
	hw = ehci->async->hw;
	hw->hw_next = QH_NEXT(ehci, ehci->async->qh_dma);
	hw->hw_info1 = cpu_to_hc32(ehci, QH_HEAD);
	hw->hw_token = cpu_to_hc32(ehci, QTD_STS_HALT);
	hw->hw_qtd_next = EHCI_LIST_END(ehci);
	ehci->async->qh_state = QH_STATE_LINKED;
	hw->hw_alt_next = QTD_NEXT(ehci, ehci->async->dummy->qtd_dma);

	/* clear interrupt enables, set irq latency */
	if (log2_irq_thresh < 0 || log2_irq_thresh > 6) {
		log2_irq_thresh = 0;
	}
	temp = 1 << (16 + log2_irq_thresh);
	ehci->has_ppcd = 0;

	if (HCC_CANPARK(hcc_params)) {
		/* HW default park == 3, on hardware that supports it (like
		 * NVidia and ALI silicon), maximizes throughput on the async
		 * schedule by avoiding QH fetches between transfers.
		 *
		 * With fast usb storage devices and NForce2, "park" seems to
		 * make problems:  throughput reduction (!), data errors...
		 */
		if (park) {
			park = min_t(unsigned, park, 3);
			temp |= CMD_PARK;
			temp |= park << 8;
		}
	}

	if (HCC_PGM_FRAMELISTLEN(hcc_params)) {
		/* periodic schedule size can be smaller than default */
		temp &= ~(3 << 2);
		temp |= (EHCI_TUNE_FLS << 2);
		switch (EHCI_TUNE_FLS) {
		case 0:
			ehci->periodic_size = 1024;
			break;
		case 1:
			ehci->periodic_size = 512;
			break;
		case 2:
			ehci->periodic_size = 256;
			break;
		default:
			BUG();
		}
	}

	ehci->command = temp;
	hcd->has_tt = 1;
	hcd->self.sg_tablesize = 0;
	return 0;
}

static void nvtim_reset_controller(struct ehci_hcd *ehci, unsigned long usbbase)
{
	unsigned long tmpval = 0x0;

	usbbase = (unsigned long)ehci->caps;
	tmpval = readl((volatile unsigned long *)(usbbase + 0x400));
	tmpval |= (0x1 << 17);
	writel(tmpval, (volatile unsigned long *)(usbbase + 0x400));

	tmpval &= ~(0x1 << 17);
	writel(tmpval, (volatile unsigned long *)(usbbase + 0x400));

#if 0
	clk_enable(ehci->reset_clk);
	clk_disable(ehci->reset_clk);
#endif
	/* Set USB ID & VBUSI */
	tmpval = readl((volatile unsigned long *)(usbbase + 0x400));
	tmpval &= ~0xFF;
	tmpval |= 0x22;
	tmpval |= (0x1 << 20);
	tmpval &= ~(0x1 << 21);
	writel(tmpval, (volatile unsigned long *)(usbbase + 0x400));

	/* Clear DEVPHY_SUSPEND[31] */
	tmpval = readl((volatile unsigned long *)(usbbase + 0x1C8));
	tmpval &= ~(0x1 << 31);
	writel(tmpval, (volatile unsigned long *)(usbbase + 0x1C8));

	/* Clear HOSTPHY_SUSPEND[6] */
	tmpval = readl((volatile unsigned long *)(usbbase + 0x40));
	tmpval &= ~(0x1 << 6);
	writel(tmpval, (volatile unsigned long *)(usbbase + 0x40));

	/* Clear FORCE_FS[9] and handle HALF_SPEED[1] */
	tmpval = readl((volatile unsigned long *)(usbbase + 0x100));
	tmpval &= ~(0x1 << 9);
#ifdef CONFIG_FPGA_EMULATION
	tmpval |= (0x1 << 1);
#endif
	writel(tmpval, (volatile unsigned long *)(usbbase + 0x100));

	/* Clear EOF1=3[3:2] EOF2[5:4]=0 */
	tmpval = readl((volatile unsigned long *)(usbbase + 0x40));
	tmpval &= ~(0x3 << 4);
	tmpval |= (0x3 << 2);
	tmpval &= ~(0x3F << 16);
	tmpval |= (0x22 << 16);
	writel(tmpval, (volatile unsigned long *)(usbbase + 0x40));

#if USB_SIMULATE_POWEROFF
	printk("R0x40=0x%08X\r\n", readl((volatile unsigned long *)(usbbase + 0x40)));
#endif

	/* A_BUS_DROP[5] = 0 */
	tmpval = readl((volatile unsigned long *)(usbbase + 0x80));
	tmpval &= ~(0x1 << 5);
	writel(tmpval, (volatile unsigned long *)(usbbase + 0x80));

	/* A_BUS_REQ[4] = 1 */
	tmpval = readl((volatile unsigned long *)(usbbase + 0x80));
	tmpval |= (0x1 << 4);
	writel(tmpval, (volatile unsigned long *)(usbbase + 0x80));

	/* Configure PHY related settings below */
	{
		u8 u2_trim_swctrl, u2_trim_sqsel, u2_trim_resint;

		u2_trim_swctrl = ehci->efuse_data[0];
		u2_trim_sqsel = ehci->efuse_data[1];
		u2_trim_resint = ehci->efuse_data[2];

		if (ehci->channel == 0) {
			u2phy_setreg(ehci, 0x144, 0x20);
			u2phy_setreg(ehci, 0x140, 0x30);
			u2phy_setreg(ehci, 0x148, 0x60 + u2_trim_resint);
			mdelay(5);
			u2phy_setreg(ehci, 0x144, 0x00);
			mdelay(5);

			u2phy_setreg(ehci, 0x38, 0x04);
		}

		tmpval = u2phy_getreg(ehci, 0x18);
		tmpval &= ~(0x7 << 1);
		tmpval |= (u2_trim_swctrl << 0x1);
		u2phy_setreg(ehci, 0x18, tmpval);

		tmpval = u2phy_getreg(ehci, 0x14);
		tmpval &= ~(0x7 << 0x2);
		tmpval |= (u2_trim_sqsel << 0x2);
		u2phy_setreg(ehci, 0x14, tmpval);

		tmpval = readl((volatile unsigned long *)(usbbase + 0x400));
		tmpval &= ~(0x1F << 8);
		tmpval |= (u2_trim_resint << 8);
		tmpval |= (0x1 << 15);
		writel(tmpval, (volatile unsigned long *)(usbbase + 0x400));
	}
}

static void nvtim_patch_usbhc2(struct usb_hcd *hcd)
{
	struct ehci_hcd *ehci = hcd_to_ehci(hcd);
	unsigned int    command = ehci_readl(ehci, &ehci->regs->command);
	int retval, retry = 0;
	unsigned long usbbase = 0x0;
	unsigned long flags = 0x0;

	pr_info("%s channel %d\n", __FUNCTION__, ehci->channel);

	if ((readl(&ehci->regs->port_status[0])) & PORT_CSC) {
		set_bit(1, &hcd->porcd_conn_chg);
	}

	if (ehci->power_gpio[0] > 0)
		gpio_direction_output(ehci->power_gpio[0], !ehci->power_gpio[1]);

	loc_cpu(&ehci->host_lock, flags);
	nvtim_reset_controller(ehci, usbbase);

	command |= CMD_RESET;
	ehci_writel(ehci, command, &ehci->regs->command);

	do {
		retval = ehci_handshake(ehci, &ehci->regs->command,
								CMD_RESET, 0, 250 * 1000);
		retry++;
	} while (retval && retry < 3);

	if (unlikely(retval != 0 && retry >= 3)) {
		ehci_err(ehci, "reset fail!\n");
	}

	command = ehci->command;

	ehci_writel(ehci, (command & ~((unsigned int)(CMD_RUN | CMD_PSE | CMD_ASE)))
				, &ehci->regs->command);
	ehci_writel(ehci, ehci->periodic_dma, &ehci->regs->frame_list);
	ehci_writel(ehci, (unsigned long)ehci->async->qh_dma, &ehci->regs->async_next);
	retry = 0;

	do {
		ehci_writel(ehci, INTR_MASK,
					&ehci->regs->intr_enable);
		retval = ehci_handshake(ehci, &ehci->regs->intr_enable,
								INTR_MASK, INTR_MASK, 250);
		retry++;
	} while (retval != 0);

	if (unlikely(retval != 0)) {
		ehci_err(ehci, "write fail!\n");
	}

	ehci->command &= ~((unsigned int)(CMD_PSE | CMD_ASE));
	set_bit(1, &hcd->porcd);

	nvtim_eof_init(ehci);
	unl_cpu(&ehci->host_lock, flags);

	if (ehci->power_gpio[0] > 0)
		gpio_direction_output(ehci->power_gpio[0], ehci->power_gpio[1]);
}

static void nvtim_patch_usbhc(struct usb_hcd *hcd)
{
	struct ehci_hcd *ehci = hcd_to_ehci(hcd);
	unsigned int    command = ehci_readl(ehci, &ehci->regs->command);
	int retval, retry = 0;
	unsigned long usbbase = 0x0;
	unsigned long flags = 0x0;

	pr_info("%s channel %d\n", __FUNCTION__, ehci->channel);

	if ((readl(&ehci->regs->port_status[0])) & PORT_CSC) {
		set_bit(1, &hcd->porcd_conn_chg);
	}

	if (ehci->power_gpio[0] > 0)
		gpio_direction_output(ehci->power_gpio[0], !ehci->power_gpio[1]);

	loc_cpu(&ehci->host_lock, flags);
	usbbase = (unsigned long)ehci->caps;
	retval = readl((volatile unsigned long *)(usbbase + 0x400));
	retval |= (0x1 << 17);
	writel(retval, (volatile unsigned long *)(usbbase + 0x400));

	retval &= ~(0x1 << 17);
	writel(retval, (volatile unsigned long *)(usbbase + 0x400));

	command |= CMD_RESET;
	ehci_writel(ehci, command, &ehci->regs->command);

	do {
		retval = ehci_handshake(ehci, &ehci->regs->command,
								CMD_RESET, 0, 250 * 1000);
		retry++;
	} while (retval && retry < 3);

	if (unlikely(retval != 0 && retry >= 3)) {
		ehci_err(ehci, "reset fail!\n");
	}

	command = ehci->command;

	ehci_writel(ehci, (command & ~((unsigned int)(CMD_RUN | CMD_PSE | CMD_ASE)))
				, &ehci->regs->command);
	ehci_writel(ehci, ehci->periodic_dma, &ehci->regs->frame_list);
	ehci_writel(ehci, (u32)ehci->async->qh_dma, &ehci->regs->async_next);
	retry = 0;

	do {
		ehci_writel(ehci, INTR_MASK,
					&ehci->regs->intr_enable);
		retval = ehci_handshake(ehci, &ehci->regs->intr_enable,
								INTR_MASK, INTR_MASK, 250);
		retry++;
	} while (retval != 0);

	if (unlikely(retval != 0)) {
		ehci_err(ehci, "write fail!\n");
	}

	ehci->command &= ~((unsigned int)(CMD_PSE | CMD_ASE));
	set_bit(1, &hcd->porcd);

	nvtim_eof_init(ehci);
	unl_cpu(&ehci->host_lock, flags);

	if (ehci->power_gpio[0] > 0)
		gpio_direction_output(ehci->power_gpio[0], ehci->power_gpio[1]);
}

static const struct hc_driver nvtivot_ehci_hc_driver = {
	.description =    hcd_name,
	.product_desc =  "usbhc-nvtivot",
	.hcd_priv_size =    sizeof(struct ehci_hcd),

	/*
	 * generic hardware linkage
	 */
	.irq =      ehci_irq,
	.flags =    HCD_MEMORY | HCD_USB2 | HCD_BH | HCD_DMA,

	/*
	 * basic lifecycle operations
	 */
	.reset =    nvtim_ehci_init,
	.start =    ehci_run,
	.stop =     ehci_stop,
	.shutdown = ehci_shutdown,

	/*
	 * managing i/o requests and associated device resources
	 */
	.urb_enqueue =    ehci_urb_enqueue,
	.urb_dequeue =    ehci_urb_dequeue,
	.endpoint_disable = ehci_endpoint_disable,
	.endpoint_reset =   ehci_endpoint_reset,

	/*
	 * scheduling support
	 */
	.get_frame_number = ehci_get_frame,

	/*
	 * root hub support
	 */
	.hub_status_data =  ehci_hub_status_data,
	.hub_control =    ehci_hub_control,
	.bus_suspend =    ehci_bus_suspend,
	.bus_resume =      ehci_bus_resume,
	.relinquish_port =  ehci_relinquish_port,
	.port_handed_over = ehci_port_handed_over,

	.clear_tt_buffer_complete = ehci_clear_tt_buffer_complete,
	.port_nc =        nvtim_patch_usbhc,
	.port_nc2 =       nvtim_patch_usbhc2,
};

static int nvtim_ehci_hcd_drv_probe(struct platform_device *pdev)
{
	struct usb_hcd *hcd = NULL;
	int ret;

	pr_debug("In nvtim_ehci_hcd_drv_probe\n");

	if (usb_disabled()) {
		return -ENODEV;
	}

	nvt_ehci_proc_init();

	ret = nvtim_usb_ehci_probe(&nvtivot_ehci_hc_driver, &hcd, pdev);
	return ret;
}

static int nvtim_ehci_hcd_drv_remove(struct platform_device *pdev)
{
	struct usb_hcd *hcd = platform_get_drvdata(pdev);
	pr_debug("In nvtim_ehci_hcd_drv_remove\n");
	nvtim_usb_ehci_remove(hcd, pdev);
	platform_set_drvdata(pdev, NULL);

	nvt_ehci_proc_exit();

	return 0;
}

#ifdef CONFIG_PM
static void nvtim_fuse_restore(struct ehci_hcd *ehci)
{
	unsigned long   phy_base;

	phy_base = (unsigned long)ehci->phy_reg;

	writel((ehci->fuseData >> 8) & 0xFF, (volatile unsigned long *)(phy_base + (0x6 << 2)));
	writel((ehci->fuseData >> 16) & 0xFF, (volatile unsigned long *)(phy_base + (0x5 << 2)));
	//printk("ehci->fuseData = 0x%08X\n",ehci->fuseData);
}

static void nvtim_suspend_usbhc(struct platform_device *dev)
{
	struct usb_hcd  *hcd = platform_get_drvdata(dev);
	struct ehci_hcd *ehci = hcd_to_ehci(hcd);
	unsigned long       usbbase;

	usbbase = (unsigned long)ehci->caps;

	/* Host PHY Suspend */
	writel((readl((volatile unsigned long *)(usbbase + 0x40)) | 0x40), (volatile unsigned long *)(usbbase + 0x40));
	udelay(1000);

}

static int nvtim_ehci_suspend(struct device *dev)
{
	struct usb_hcd *hcd = dev_get_drvdata(dev);
	bool do_wakeup = device_may_wakeup(dev);
	struct platform_device *pdev = to_platform_device(dev);
	int rc;
	int irq;

#ifdef CONFIG_OF
	irq = platform_get_irq(pdev, 0);
	if (irq < 0) {
		pr_debug("resource[1] is not IORESOURCE_IRQ");
		return -ENXIO;
	}
#else
	if (dev->resource[1].flags != IORESOURCE_IRQ) {
		pr_debug("resource[1] is not IORESOURCE_IRQ");
		retval = -ENOMEM;
		return retval;
	}
	irq = dev->resource[1].start;
#endif

	rc = ehci_suspend(hcd, do_wakeup);
	disable_irq(irq);

	//nvtim_stop_usbhc(pdev);
	nvtim_suspend_usbhc(pdev);

	return rc;
}


static int nvtim_ehci_resume(struct device *dev)
{
	struct usb_hcd *hcd = dev_get_drvdata(dev);
	struct platform_device *pdev = to_platform_device(dev);
	struct ehci_hcd *ehci = hcd_to_ehci(hcd);
	int irq;

#ifdef CONFIG_OF
	irq = platform_get_irq(pdev, 0);
	if (irq < 0) {
		pr_debug("resource[1] is not IORESOURCE_IRQ");
		return -ENXIO;
	}
#else
	if (dev->resource[1].flags != IORESOURCE_IRQ) {
		pr_debug("resource[1] is not IORESOURCE_IRQ");
		retval = -ENOMEM;
		return retval;
	}
	irq = dev->resource[1].start;
#endif

	nvtim_init_usbhc(ehci);
	nvtim_fuse_restore(ehci);
	//nvtim_plat_resume(hcd);

	ehci_resume(hcd, false);
	enable_irq(irq);
	return 0;
}
#else
static int nvtim_ehci_suspend(struct device *dev)
{
	ehci_err(hcd_to_ehci(dev_get_drvdata(dev)),
			 "+-%s[%d]:CONFIG_PM NOT DEFINED\r\n", __func__, __LINE__);
	return 0;
}
static int nvtim_ehci_resume(struct device *dev)
{
	ehci_err(hcd_to_ehci(dev_get_drvdata(dev)),
			 "+-%s[%d]:CONFIG_PM NOT DEFINED\r\n", __func__, __LINE__);
	return 0;
}
#endif

static const struct dev_pm_ops nvtivot_ehci_pm_ops = {
	.suspend = nvtim_ehci_suspend,
	.resume  = nvtim_ehci_resume,
};

#ifdef CONFIG_OF
static const struct of_device_id nvtivot_ehci_match[] = {
	{ .compatible = "nvt,ehci-nvtivot" },
	{},
};

MODULE_DEVICE_TABLE(of, nvtivot_ehci_match);
#endif

MODULE_ALIAS("ehci-nvtivot");
static struct platform_driver ehci_hcd_na51102_driver = {
	.probe = nvtim_ehci_hcd_drv_probe,
	.remove = nvtim_ehci_hcd_drv_remove,
	.driver = {
		.name = "ehci-nvtivot",
		.pm = &nvtivot_ehci_pm_ops,
#ifdef CONFIG_OF
		.of_match_table = nvtivot_ehci_match,
#endif
	}
};

MODULE_VERSION(DRV_VERSION);
