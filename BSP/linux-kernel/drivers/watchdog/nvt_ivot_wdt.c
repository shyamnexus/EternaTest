/**
    NVT ivot watchdog driver
    @file nvt_ivot_wdt.c
    @ingroup
    @note
    Copyright Novatek Microelectronics Corp. 2021. All rights reserved.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License version 2 as
    published by the Free Software Foundation.
*/
#include <linux/err.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include <linux/types.h>
#include <linux/watchdog.h>
#include <linux/delay.h>
#include <linux/clk.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <plat/wdt_reg.h>


#define DRV_VERSION		"1.00.007"

/*The WDT range is from 1s to 178.9 s*/
#define NVT_WDT_DEFAULT_TIME 80 /*s*/
#define NVT_WDT_MAX_TIME 178     /*s*/

/*trigger sw timeout counter*/
#define WDT_SW_TIMEOUT	5

#if (! (defined(CONFIG_NVT_IVOT_PLAT_NA51055) || defined(CONFIG_NVT_IVOT_PLAT_NA51089)))
#define SUPPORT_PRETIMEOUT	(1)
#else
#define SUPPORT_PRETIMEOUT	(0)
#endif

static int wdt_time = NVT_WDT_DEFAULT_TIME;
module_param(wdt_time, int, 0);
MODULE_PARM_DESC(wdt_time, "wdt default time in sec");

static bool nowayout = WATCHDOG_NOWAYOUT;
module_param(nowayout, bool, 0);
MODULE_PARM_DESC(nowayout, "Watchdog cannot be stopped once started (default="
			__MODULE_STRING(WATCHDOG_NOWAYOUT) ")");

struct nvt_wdt_device {
	int			irq;
	int			int_pending;
	struct proc_dir_entry	*pproc_wdt;
	void __iomem            *base;
	struct clk              *clk;
	struct watchdog_device  wdd;
	spinlock_t              wdt_lock;
	bool			wdt_en_sts;
};

static unsigned long lock_flags;
#define WDT_GETREG(wdt_dev, ofs)            readl((void __iomem *)(wdt_dev->base + ofs))
#define WDT_SETREG(wdt_dev, ofs, value)     writel(value, (void __iomem *)(wdt_dev->base + ofs))

static struct nvt_wdt_device *global_nvt_wdt = NULL;

/*
 * Trigger Manual Reset.
 */
void nvt_trigger_manual_reset(void)
{
	union WDT_MANUAL_RST_REG manual_rst_reg;
	union WDT_CTRL_REG ctrl_reg;
	union WDT_STS_REG status_reg;
	int timeout;

	/*Enter critical section*/
	spin_lock_irqsave(&global_nvt_wdt->wdt_lock, lock_flags);

	/*Disable WDT First*/
	ctrl_reg.reg = WDT_GETREG(global_nvt_wdt, WDT_CTRL_REG_OFS);
	ctrl_reg.bit.enable = 0;
	ctrl_reg.bit.key_ctrl = WDT_KEY_VALUE;
	WDT_SETREG(global_nvt_wdt, WDT_CTRL_REG_OFS, ctrl_reg.reg);

	/*Wait for WDT is really disabled*/
	timeout = 0;
	while (timeout < WDT_SW_TIMEOUT) {
		status_reg.reg = WDT_GETREG(global_nvt_wdt, WDT_STS_REG_OFS);
		if (status_reg.bit.en_status == 0)
			break;
		udelay(1);
		timeout++;
	}
	if (timeout >= WDT_SW_TIMEOUT) {
		/*Leave critical section*/
		spin_unlock_irqrestore(&global_nvt_wdt->wdt_lock, lock_flags);
		printk("%s: wait WDT dis timeout\r\n", __func__);
		return;
	}

	/*Set external reset*/
	ctrl_reg.reg = WDT_GETREG(global_nvt_wdt, WDT_CTRL_REG_OFS);
	ctrl_reg.bit.mode       = WDT_MODE_RESET;
	ctrl_reg.bit.key_ctrl   = WDT_KEY_VALUE;
	ctrl_reg.bit.ext_reset  = 0x1;
	ctrl_reg.bit.msb = 0x1;
	WDT_SETREG(global_nvt_wdt, WDT_CTRL_REG_OFS, ctrl_reg.reg);

	/*wait 1T time before set WDT_EN*/
	udelay(80);

	/*Enable WDT*/
	ctrl_reg.reg = WDT_GETREG(global_nvt_wdt, WDT_CTRL_REG_OFS);
	ctrl_reg.bit.enable = 1;
	ctrl_reg.bit.key_ctrl = WDT_KEY_VALUE;
	WDT_SETREG(global_nvt_wdt, WDT_CTRL_REG_OFS, ctrl_reg.reg);

	/*Wait for WDT is really enabled*/
	timeout = 0;
	while (timeout < WDT_SW_TIMEOUT) {
		status_reg.reg = WDT_GETREG(global_nvt_wdt, WDT_STS_REG_OFS);
		if (status_reg.bit.en_status == 1)
			break;
		udelay(1);
		timeout++;
	}
	if (timeout >= WDT_SW_TIMEOUT) {
		/*Leave critical section*/
		spin_unlock_irqrestore(&global_nvt_wdt->wdt_lock, lock_flags);
		printk("%s: wait WDT en timeout\r\n", __func__);
		return;
	}

	manual_rst_reg.reg = WDT_GETREG(global_nvt_wdt, WDT_MANUAL_RST_REG_OFS);
	manual_rst_reg.bit.manual_rst = 0x1;
	WDT_SETREG(global_nvt_wdt, WDT_MANUAL_RST_REG_OFS, manual_rst_reg.reg);

	/*Leave critical section*/
	spin_unlock_irqrestore(&global_nvt_wdt->wdt_lock, lock_flags);
}

/*
 * Watchdog interrupt service routine
 */
static irqreturn_t wdt_handler(int irq, void *data)
{
	union WDT_STS_REG status_reg;
	struct nvt_wdt_device *nvt_wdt = (struct nvt_wdt_device *)data;

	// write 1 clear interrupt status
	status_reg.reg = WDT_GETREG(nvt_wdt, WDT_STS_REG_OFS);
	WDT_SETREG(nvt_wdt, WDT_STS_REG_OFS, status_reg.reg);

	nvt_wdt->int_pending = 1;

	watchdog_notify_pretimeout(&nvt_wdt->wdd);

	return IRQ_HANDLED;
}

/*
 * Change the watchdog time interval.
 */
static int nvt_wdt_settimeout(struct nvt_wdt_device *nvt_wdt,
		int new_time, int pretimeout)
{
	uint32_t clk_value, clk_prevalue;
	union WDT_CTRL_REG ctrl_reg;
#if (SUPPORT_PRETIMEOUT == 1)
	union WDT_CTRL2_REG ctrl2_reg;
#endif

	if ((pretimeout != 0) && (new_time > pretimeout)) {
		new_time -= pretimeout;
	}

	clk_value = (new_time * WDT_SOURCE_CLOCK) >> 12;
	if (clk_value > WDT_MSB_MAX) {
		pr_err("Invalid value (%d)\n", new_time);
		nvt_wdt->wdd.timeout = wdt_time;
		nvt_wdt->wdd.pretimeout = 0;
		return -EINVAL;
	}
	clk_prevalue = (pretimeout * WDT_SOURCE_CLOCK) >> 12;

	/*Enter critical section*/
	spin_lock_irqsave(&nvt_wdt->wdt_lock, lock_flags);

	ctrl_reg.reg = WDT_GETREG(nvt_wdt, WDT_CTRL_REG_OFS);
	ctrl_reg.bit.msb = clk_value;
	ctrl_reg.bit.key_ctrl = WDT_KEY_VALUE;
	WDT_SETREG(nvt_wdt, WDT_CTRL_REG_OFS, ctrl_reg.reg);
#if (SUPPORT_PRETIMEOUT == 1)
	ctrl2_reg.reg = 0;
	ctrl2_reg.bit.msb2nd = clk_prevalue;
	ctrl2_reg.bit.key_ctrl2 = WDT_KEY_VALUE;
	WDT_SETREG(nvt_wdt, WDT_CTRL2_REG_OFS, ctrl2_reg.reg);
#endif

	/*Leave critical section*/
	spin_unlock_irqrestore(&nvt_wdt->wdt_lock, lock_flags);

	nvt_wdt->wdd.timeout = new_time;

	return 0;
}

/*
 * Disable the watchdog.
 */
static int nvt_wdt_stop(struct watchdog_device *wdd)
{
	struct nvt_wdt_device *nvt_wdt = watchdog_get_drvdata(wdd);
	union WDT_CTRL_REG ctrl_reg;
	union WDT_STS_REG status_reg;
	int timeout;

	/*Enter critical section*/
	spin_lock_irqsave(&nvt_wdt->wdt_lock, lock_flags);

	/*Enable WDT*/
	ctrl_reg.reg = WDT_GETREG(nvt_wdt, WDT_CTRL_REG_OFS);
	ctrl_reg.bit.enable = 0;
	ctrl_reg.bit.key_ctrl = WDT_KEY_VALUE;
	WDT_SETREG(nvt_wdt, WDT_CTRL_REG_OFS, ctrl_reg.reg);

	/*Wait for WDT is really enabled*/
	timeout = 0;
	while (timeout < WDT_SW_TIMEOUT) {
		status_reg.reg = WDT_GETREG(nvt_wdt, WDT_STS_REG_OFS);
		if (status_reg.bit.en_status == 0)
			break;
		udelay(1);
		timeout++;
	}
	if (timeout >= WDT_SW_TIMEOUT) {
		printk("%s: wait WDT dis timeout\r\n", __func__);
	}

	/*Leave critical section*/
	//coverity[double_unlock]: only unlock once
	spin_unlock_irqrestore(&nvt_wdt->wdt_lock, lock_flags);

	return 0;
}

/*
 * Enable and reset the watchdog.
 */
static int nvt_wdt_start(struct watchdog_device *wdd)
{
	struct nvt_wdt_device *nvt_wdt = watchdog_get_drvdata(wdd);
	union WDT_CTRL_REG ctrl_reg;
	union WDT_STS_REG status_reg;
	int timeout;
	int ret = 0;

	ret = nvt_wdt_settimeout(nvt_wdt, wdd->timeout, wdd->pretimeout);

	if (ret) {
		printk("wdt start fail\n");
		return -EINVAL;
	}

	/*Enter critical section*/
	spin_lock_irqsave(&nvt_wdt->wdt_lock, lock_flags);

	/*Enable WDT*/
	ctrl_reg.reg = WDT_GETREG(nvt_wdt, WDT_CTRL_REG_OFS);
#if (SUPPORT_PRETIMEOUT == 1)
	if (wdd->pretimeout)
		ctrl_reg.bit.mode       = WDT_MODE_INT2RESET;
	else
		ctrl_reg.bit.mode       = WDT_MODE_RESET;
#else
	ctrl_reg.bit.mode       = WDT_MODE_RESET;
#endif
	ctrl_reg.bit.enable = 1;
	ctrl_reg.bit.key_ctrl = WDT_KEY_VALUE;
	WDT_SETREG(nvt_wdt, WDT_CTRL_REG_OFS, ctrl_reg.reg);

	/*Wait for WDT is really enabled*/
	timeout = 0;
	while (timeout < WDT_SW_TIMEOUT) {
		status_reg.reg = WDT_GETREG(nvt_wdt, WDT_STS_REG_OFS);
		if (status_reg.bit.en_status == 1)
			break;
		udelay(1);
		timeout++;
	}
	if (timeout >= WDT_SW_TIMEOUT) {
		printk("%s: wait WDT en timeout\r\n", __func__);
	}

	nvt_wdt->int_pending = 0;

	/*Leave critical section*/
	//coverity[double_unlock]: only unlock once
	spin_unlock_irqrestore(&nvt_wdt->wdt_lock, lock_flags);

	return 0;
}

/*
 * Reload the watchdog timer.  (ie, pat the watchdog)
 */
static int nvt_wdt_reload(struct watchdog_device *wdd)
{
	struct nvt_wdt_device *nvt_wdt = watchdog_get_drvdata(wdd);
	union WDT_TRIG_REG trig_reg;
	int timeout;

	/*Wait for previous trigger done*/
	timeout = 0;
	while (timeout < WDT_SW_TIMEOUT) {
		trig_reg.reg = WDT_GETREG(nvt_wdt, WDT_TRIG_REG_OFS);
		if (trig_reg.bit.trigger == 0)
			break;
		udelay(1);
		timeout++;
	}
	if (timeout >= WDT_SW_TIMEOUT) {
		printk("%s: wait trigger timeout\r\n", __func__);
		return -ETXTBSY;
	}

	/*Trigger WDT*/
	trig_reg.bit.trigger = 1;
	WDT_SETREG(nvt_wdt, WDT_TRIG_REG_OFS, trig_reg.reg);

	nvt_wdt->int_pending = 0;

	return 0;
}


static unsigned int nvt_wdt_get_timeleft(struct watchdog_device *wdd)
{
	u64 timer_counter;
	struct nvt_wdt_device *nvt_wdt = watchdog_get_drvdata(wdd);
	union WDT_STS_REG status_reg;

	status_reg.reg = WDT_GETREG(nvt_wdt, WDT_STS_REG_OFS);
	timer_counter = (u64)status_reg.bit.cnt;
	do_div(timer_counter, WDT_SOURCE_CLOCK);

	return timer_counter;
}

static int nvt_wdt_set_timeout(struct watchdog_device *wdd, unsigned int timeout)
{
	wdd->timeout = timeout;

	nvt_wdt_stop(wdd);

	nvt_wdt_start(wdd);

	return 0;
}

#if (SUPPORT_PRETIMEOUT == 1)
static int nvt_wdt_set_pretimeout(struct watchdog_device *wdd, unsigned int timeout)
{
	wdd->pretimeout = timeout;
	nvt_wdt_stop(wdd);
	nvt_wdt_start(wdd);

	return 0;
}
#endif

/* ......................................................................... */
static int wdt_proc_show(struct seq_file *seq, void *v)
{
	struct nvt_wdt_device *nvt_wdt = seq->private;
	seq_printf(seq, "Interrupt Pending %d\r\n", nvt_wdt->int_pending);
	return 0;
}

static int wdt_proc_open(struct inode *inode, struct file *file)
{
        return single_open(file, wdt_proc_show, PDE_DATA(inode));
}

static struct proc_ops proc_wdt_fops = {
        .proc_open    = wdt_proc_open,
        .proc_read    = seq_read,
        .proc_lseek  = seq_lseek,
        .proc_release = single_release,
};

/* ......................................................................... */

static const struct watchdog_info nvt_wdt_info = {
	.identity = "Novatek IVOT Watchdog",
	.options = WDIOF_SETTIMEOUT |
#if (SUPPORT_PRETIMEOUT == 1)
			WDIOF_PRETIMEOUT |
#endif
			WDIOF_KEEPALIVEPING |
			WDIOF_MAGICCLOSE,
};

static const struct watchdog_ops nvt_wdt_ops = {
	.owner		= THIS_MODULE,
	.start		= nvt_wdt_start,
	.stop		= nvt_wdt_stop,
	.ping		= nvt_wdt_reload,
	.get_timeleft	= nvt_wdt_get_timeleft,
	.set_timeout	= nvt_wdt_set_timeout,
#if (SUPPORT_PRETIMEOUT == 1)
	.set_pretimeout	= nvt_wdt_set_pretimeout,
#endif
};

static void nvt_wdt_init(struct nvt_wdt_device *nvt_wdt)
{
	union WDT_CTRL_REG ctrl_reg;

	/*Enter critical section*/
	spin_lock_irqsave(&nvt_wdt->wdt_lock, lock_flags);

	/*Set the WDT as reset mode*/
	ctrl_reg.reg = 0;
	ctrl_reg.bit.mode       = WDT_MODE_RESET;
	ctrl_reg.bit.key_ctrl   = WDT_KEY_VALUE;
	ctrl_reg.bit.rst_num0_en = 1;
#ifdef CONFIG_NOVATEK_WATCHDOG_EXTERNAL_RESET
	ctrl_reg.bit.ext_reset  = 0x1;
#endif
	WDT_SETREG(nvt_wdt, WDT_CTRL_REG_OFS, ctrl_reg.reg);

	nvt_wdt->int_pending = 0;

	/*Leave critical section*/
	spin_unlock_irqrestore(&nvt_wdt->wdt_lock, lock_flags);
}

static int nvt_wdt_probe(struct platform_device *pdev)
{
	int res;
	int irq;
	struct proc_dir_entry *pentry = NULL;
	struct device *dev = &pdev->dev;
	struct resource *memres = NULL;
	struct nvt_wdt_device *nvt_wdt;

	nvt_wdt = devm_kzalloc(dev, sizeof(*nvt_wdt), GFP_KERNEL);
	if (!nvt_wdt)
		return -ENOMEM;

	global_nvt_wdt = nvt_wdt;

	irq = platform_get_irq(pdev, 0);
        if (irq < 0) {
                dev_err(&pdev->dev, "No IRQ resource!\n");
		return irq;
        }

	res = devm_request_irq(&pdev->dev, irq, wdt_handler, 0, pdev->name,
                        nvt_wdt);
        if (res < 0) {
                dev_err(&pdev->dev, "Failed to request IRQ: #%d: %d\n", irq, res);
                return res;
        }
	nvt_wdt->irq = irq;

	nvt_wdt->clk = devm_clk_get(dev, dev_name(dev));
	if (WARN_ON(IS_ERR(nvt_wdt->clk))) {
		return PTR_ERR(nvt_wdt->clk);
	}

	clk_prepare_enable(nvt_wdt->clk);

	platform_set_drvdata(pdev, nvt_wdt);

	memres = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	nvt_wdt->base = devm_ioremap_resource(dev, memres);
	if (IS_ERR(nvt_wdt->base)) {
		return PTR_ERR(nvt_wdt->base);
	}

	nvt_wdt->wdd.info = &nvt_wdt_info;
	nvt_wdt->wdd.ops = &nvt_wdt_ops;
	nvt_wdt->wdd.min_timeout = 1;
	nvt_wdt->wdd.max_timeout = NVT_WDT_MAX_TIME;
	nvt_wdt->wdd.timeout = wdt_time;
	nvt_wdt->wdd.parent = &pdev->dev;

	// coverity[side_effect_free]: spin_lock_init is kernel API.
	spin_lock_init(&nvt_wdt->wdt_lock);

	nvt_wdt_init(nvt_wdt);

	watchdog_init_timeout(&nvt_wdt->wdd, wdt_time, dev);
	watchdog_set_drvdata(&nvt_wdt->wdd, nvt_wdt);
	watchdog_set_nowayout(&nvt_wdt->wdd, nowayout);

	res = watchdog_register_device(&nvt_wdt->wdd);
	if (res < 0) {
		dev_err(dev, "cannot register watchdog device\n");
	}

	pentry = proc_create_data("nvt_wdt", S_IRUGO | S_IXUGO, NULL, &proc_wdt_fops, nvt_wdt);
        if (pentry == NULL) {
                dev_err(&pdev->dev, "failed to create wdt proc!\n");
                res = -EINVAL;
                goto out;
        }
        nvt_wdt->pproc_wdt = pentry;

	return res;

out:
	watchdog_unregister_device(&nvt_wdt->wdd);

	return res;
}

static int nvt_wdt_remove(struct platform_device *pdev)
{
	struct nvt_wdt_device *nvt_wdt = platform_get_drvdata(pdev);

	if (nvt_wdt->pproc_wdt) {
                proc_remove(nvt_wdt->pproc_wdt);
        }
	watchdog_unregister_device(&nvt_wdt->wdd);
	clk_disable_unprepare(nvt_wdt->clk);

	return 0;
}

static void nvt_wdt_shutdown(struct platform_device *pdev)
{
	struct nvt_wdt_device *nvt_wdt = platform_get_drvdata(pdev);

	if (watchdog_active(&nvt_wdt->wdd))
		nvt_wdt_stop(&nvt_wdt->wdd);
}

#ifdef CONFIG_PM

static int nvt_wdt_suspend(struct platform_device *pdev,
			      pm_message_t message)
{
	struct nvt_wdt_device *nvt_wdt = platform_get_drvdata(pdev);

	if (WDT_GETREG(global_nvt_wdt, WDT_CTRL_REG_OFS) & 0x1) {
		nvt_wdt->wdt_en_sts = 1;
		nvt_wdt_stop(&nvt_wdt->wdd);
	} else {
		nvt_wdt->wdt_en_sts = 0;
	}

	return 0;
}

static int nvt_wdt_resume(struct platform_device *pdev)
{
	struct nvt_wdt_device *nvt_wdt = platform_get_drvdata(pdev);

	if (nvt_wdt->wdt_en_sts) {
		nvt_wdt_start(&nvt_wdt->wdd);
	}

	return 0;
}

#else
#define nvt_wdt_suspend NULL
#define nvt_wdt_resume	NULL
#endif

#ifdef CONFIG_OF
static const struct of_device_id nvt_wdt_match[] = {
	{ .compatible = "nvt,nvt_wdt" },
	{},
};

MODULE_DEVICE_TABLE(of, nvt_wdt_match);
#endif

static struct platform_driver nvt_wdt_driver = {
	.probe		= nvt_wdt_probe,
	.remove		= nvt_wdt_remove,
	.shutdown	= nvt_wdt_shutdown,
	.suspend	= nvt_wdt_suspend,
	.resume		= nvt_wdt_resume,
	.driver		= {
		.name	= "nvt_wdt",
		.owner	= THIS_MODULE,
#ifdef CONFIG_OF
		.of_match_table = nvt_wdt_match,
#endif
	},
};

module_platform_driver(nvt_wdt_driver);

MODULE_AUTHOR("Howard Chang @ Novatek");
MODULE_DESCRIPTION("Watchdog driver for Novatek");
MODULE_LICENSE("GPL");
MODULE_VERSION(DRV_VERSION);
MODULE_ALIAS("platform:nvt_wdt");

