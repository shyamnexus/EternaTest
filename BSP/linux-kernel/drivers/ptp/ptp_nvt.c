/**
    NVT ptp function
    NVT ptp driver
    @file       ptp_nvt.c
    @ingroup
    @note
    Copyright   Novatek Microelectronics Corp. 2021.  All rights reserved.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License version 2 as
    published by the Free Software Foundation.
*/
#include <linux/err.h>
#include <linux/io.h>
#include <linux/module.h>
#include <linux/mod_devicetable.h>
#include <linux/platform_device.h>
#include <linux/ptp_clock_kernel.h>
#include <linux/types.h>
#include <linux/clocksource.h>
#include <linux/time.h>
#include <linux/timecounter.h>
#include <linux/delay.h>
#include <linux/clk.h>
#include <linux/irqreturn.h>
#include <linux/interrupt.h>
#include <linux/gpio.h>
#include <linux/workqueue.h>
#include <plat/hrtimer-reg.h>

/* ptp nvt priv structure */
struct ptp_nvt {
	void __iomem *regs;
	struct ptp_clock *ptp_clk;
	struct ptp_clock_info caps;
	struct device *dev;
	spinlock_t lock;

	struct cyclecounter cc;
	struct timecounter tc;
	struct clk *clk;
	u32 freq;
};

struct ptp_nvt *ptp_nvt;
static int ptp_cyclecounter_init(struct ptp_nvt *ptp);
static int ptp_nvt_enable(struct ptp_clock_info *ptp,
			  struct ptp_clock_request *rq, int on);

static s64 nvt_read_nco(void __iomem *regs)
{
	s64 ns;

	ns = readl(regs + TMR_CNT_L);
	ns += (u64)readl(regs + TMR_CNT_H) << 32;

	return ns;
}

static int ptp_nvt_adjfreq(struct ptp_clock_info *ptp, s32 ppb)
{
	unsigned long flags;
	struct ptp_nvt *ptp_nvt = container_of(ptp, struct ptp_nvt, caps);
	s8 freq_bit = (TMR_MAX_HZ / ppb) -1;
	u32 val;

	if (freq_bit < 0) {
		pr_err("%s: freq too fast should <= %d\n", __func__, TMR_MAX_HZ);
		return -EINVAL;
	}

	if (freq_bit > TMR_MAX_DIV_BIT) {
		pr_err("%s: freq too slow should >= %lld\n", __func__, TMR_MAX_HZ / (TMR_MAX_DIV_BIT + 1));
		return -EINVAL;
	}

	spin_lock_irqsave(&ptp_nvt->lock, flags);

	ptp_nvt->freq = ppb;

	/* Stop Timer */
	val = readl(ptp_nvt->regs + TMR_CTRL);
	val &= GENMASK(31, TMR_EN);
	writel(val, (ptp_nvt->regs + TMR_CTRL));

	/* Change Frequency */
	writel(freq_bit, (ptp_nvt->regs + TMR_CLK_DIV));

	/* Start Timer */
	val = readl(ptp_nvt->regs + TMR_CTRL);
	val |= ENABLE;
	writel(val, (ptp_nvt->regs + TMR_CTRL));

	spin_unlock_irqrestore(&ptp_nvt->lock, flags);

	return 0;
}

static int ptp_nvt_adjtime(struct ptp_clock_info *ptp, s64 delta)
{
	unsigned long flags;
	struct ptp_nvt *ptp_nvt = container_of(ptp, struct ptp_nvt, caps);

	spin_lock_irqsave(&ptp_nvt->lock, flags);

	timecounter_adjtime(&ptp_nvt->tc, delta);

	spin_unlock_irqrestore(&ptp_nvt->lock, flags);

	return 0;
}

static int ptp_nvt_gettime(struct ptp_clock_info *ptp, struct timespec64 *ts)
{
	struct ptp_nvt *ptp_nvt = container_of(ptp, struct ptp_nvt, caps);

	*ts = ns_to_timespec64(timecounter_read(&ptp_nvt->tc));
	return 0;
}

static int ptp_nvt_settime(struct ptp_clock_info *ptp,
			   const struct timespec64 *ts)
{
	unsigned long flags;
	struct ptp_nvt *ptp_nvt = container_of(ptp, struct ptp_nvt, caps);
	s64 ns, new_ns;

	spin_lock_irqsave(&ptp_nvt->lock, flags);

	new_ns = timespec64_to_ns(ts);
	ns = timecounter_read(&ptp_nvt->tc);
	new_ns -= ns;
	timecounter_adjtime(&ptp_nvt->tc, new_ns);

	spin_unlock_irqrestore(&ptp_nvt->lock, flags);

	return 0;
}

static int ptp_nvt_enable(struct ptp_clock_info *ptp,
			  struct ptp_clock_request *rq, int on)
{
	u32 val;
	unsigned long flags;
	s8 freq_bit = (TMR_MAX_HZ / ptp_nvt->freq) -1;

	if (freq_bit < 0) {
		pr_err("%s: freq too fast should <= %d\n", __func__, TMR_MAX_HZ);
		return -EINVAL;
	}

	if (freq_bit > TMR_MAX_DIV_BIT) {
		pr_err("%s: freq too slow should >= %lld\n", __func__, TMR_MAX_HZ / (TMR_MAX_DIV_BIT + 1));
		return -EINVAL;
	}

	spin_lock_irqsave(&ptp_nvt->lock, flags);
	if (on == ENABLE) {
		/* Set Max Count */
		val = (TMR_MAX_VALUE) & GENMASK(31, 0);
		writel(val, (ptp_nvt->regs + TMR_TVAL_L));
		val = (TMR_MAX_VALUE >> 32) & GENMASK(31, 0);
		writel(val, (ptp_nvt->regs + TMR_TVAL_H));

		/* Set Re-Load */
		writel(DISABLE, (ptp_nvt->regs + TMR_RLD));

		/* Set Frequency */
		writel(freq_bit, (ptp_nvt->regs + TMR_CLK_DIV));

		/* Start Timer */
		val = readl(ptp_nvt->regs + TMR_CTRL);
		val |= ENABLE | TMR_FREE_RUN;
		writel(val, (ptp_nvt->regs + TMR_CTRL));
	} else if (on == DISABLE) {
		/* Stop Timer */
		val = readl(ptp_nvt->regs + TMR_CTRL);
		val &= GENMASK(31, TMR_EN);
		writel(val, (ptp_nvt->regs + TMR_CTRL));
	} else {
		dev_err(ptp_nvt->dev, "%s: on/off should be set 1 or 0\n", __func__);
	}
	spin_unlock_irqrestore(&ptp_nvt->lock, flags);

	return 0;
}

static const struct ptp_clock_info ptp_nvt_caps = {
	.owner		= THIS_MODULE,
	.name		= "NVT PTP timer",
	.max_adj	= 50000000,
	.n_ext_ts	= 0,
	.n_pins		= 0,
	.pps		= 0,
	.adjfreq	= ptp_nvt_adjfreq,
	.adjtime	= ptp_nvt_adjtime,
	.gettime64	= ptp_nvt_gettime,
	.settime64	= ptp_nvt_settime,
	.enable		= ptp_nvt_enable,
};

static u64 ptp_cyclecounter_read(const struct cyclecounter *cc)
{
	return (u64)(nvt_read_nco(ptp_nvt->regs));
}

static int ptp_cyclecounter_init(struct ptp_nvt *ptp)
{
	u64 start_count;
	u32 mult, shift;

	start_count = (u64)ktime_to_ns(ktime_get_real());
	clocks_calc_mult_shift(&mult, &shift, ptp->freq, NSEC_PER_SEC, 60);
	ptp->cc.read = ptp_cyclecounter_read;
	ptp->cc.mask = CLOCKSOURCE_MASK(64);
	ptp->cc.mult = mult;
	ptp->cc.shift = shift;

	timecounter_init(&ptp->tc,
			 &ptp->cc, start_count);

	return 0;
}

static int ptp_nvt_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct resource *res;
	struct device_node *node = pdev->dev.of_node;
	u32 *pfreq;
	int ret = 0;

	ptp_nvt = devm_kzalloc(dev, sizeof(struct ptp_nvt), GFP_KERNEL);
	if (!ptp_nvt)
		goto kzalloc_err;

	ptp_nvt->dev = dev;
	ptp_nvt->caps = ptp_nvt_caps;

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	ptp_nvt->regs = devm_ioremap_resource(dev, res);
	if (IS_ERR(ptp_nvt->regs)) {
		dev_err(dev, "%s: io remap failed\n", __func__);
		goto remap_err;
	}

	spin_lock_init(&ptp_nvt->lock);

	ptp_nvt->clk = devm_clk_get(dev, NULL);
	if (IS_ERR(ptp_nvt->clk)) {
		dev_err(dev, "%s:%d clk_get fail\n", __func__, __LINE__);
		goto clk_err;
	}
	if (clk_prepare_enable(ptp_nvt->clk) != 0) {
		dev_err(dev, "%s:%d enable clk fail\n", __func__, __LINE__);
		goto clk_err;
	}
	ptp_nvt->freq = (u32)(clk_get_rate(ptp_nvt->clk));

	pfreq = (u32 *)of_get_property(node, "clock-frequency", NULL);
	if (pfreq != NULL) {
		ptp_nvt->freq = __be32_to_cpu(*pfreq);
	}

	ptp_nvt->ptp_clk = ptp_clock_register(&ptp_nvt->caps, &pdev->dev);

	ptp_cyclecounter_init(ptp_nvt);

	dev_info(dev, "%s: init done\n", __func__);
	return 0;
clk_err:
	iounmap(ptp_nvt->regs);
remap_err:
	kfree(ptp_nvt);
kzalloc_err:

	return ret;
}

static int ptp_nvt_remove(struct platform_device *pdev)
{
	if (ptp_nvt->regs)
		ptp_nvt_enable(&ptp_nvt->caps, 0, DISABLE);

	if (ptp_nvt->clk) {
		clk_disable(ptp_nvt->clk);
	}

	if (ptp_nvt->regs) {
		iounmap(ptp_nvt->regs);
	}

	if (ptp_nvt) {
		kfree(ptp_nvt);
	}
	return 0;
}

static const struct of_device_id ptp_nvt_of_match[] = {
	{ .compatible = "nvt,ptp-nvt", },
	{},
};

static struct platform_driver ptp_nvt_driver = {
	.driver = {
		.name = "ptp-nvt",
		.pm = NULL,
		.of_match_table = ptp_nvt_of_match,
	},
	.probe    = ptp_nvt_probe,
	.remove   = ptp_nvt_remove,
};
module_platform_driver(ptp_nvt_driver);

MODULE_AUTHOR("Novatek");
MODULE_VERSION("1.0.0");
MODULE_DESCRIPTION("Novatek PTP Clock driver");
MODULE_LICENSE("GPL v2");
