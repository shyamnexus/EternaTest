/**
    NVT timer function
    NVT timer driver
    @file       nvt-timer.c
    @ingroup
    @note
    Copyright   Novatek Microelectronics Corp. 2021.  All rights reserved.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License version 2 as
    published by the Free Software Foundation.
*/
#include <linux/interrupt.h>
#include <linux/clk.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/vmalloc.h>
#include <linux/slab.h>
#include <linux/io.h>
#include <linux/of_device.h>
#include <linux/soc/nvt/nvt-info.h>
#include <linux/soc/nvt/nvt-timer.h>
#include <plat/hrtimer-reg.h>

DEFINE_SEMAPHORE(sema_timer);
struct list_head timer_list_root;

static struct nvt_timer_t *nvt_get_timer_info(unsigned int index)
{
	struct nvt_timer_t *curr_info, *next_info;

	down(&sema_timer);
	list_for_each_entry_safe(curr_info, next_info, &timer_list_root, list) {
		if (curr_info->index == index) {
			up(&sema_timer);
			return curr_info;
		}
	}
	up(&sema_timer);

	return NULL;
}

int nvt_timer_get_count(unsigned int num, u64 *count)
{
	struct nvt_timer_t *nvt_timer = nvt_get_timer_info(num);

#if defined(CONFIG_NVT_IVOT_PLAT_NS02401)	/* Polling sync */

	u64 count_now;
	u64 val = 0, timeout = 0;

	if (nvt_timer == NULL) {
		pr_err("%s: Timer index(%d) not find !!!\n", __func__, num);
		return -EINVAL;
	}

	if (nvt_timer->in_use == false) {
		dev_err(nvt_timer->dev, "Timer(%d) is not in use !!!\n", num);
		return -EINVAL;
	}

	/* 1. Send apb cmd 1 to sync hrtimer counter values to hrtimer registers completely */
	writel(ENABLE, (nvt_timer->va + TMR_LATCH_APB));

	/* 2. Wait update finish & clear latch sts */
	val = readl(nvt_timer->va + TMR_LATCH_STS) & BIT(1);
	while (val != BIT(1)) {
		val = readl(nvt_timer->va + TMR_LATCH_STS) & BIT(1);
		timeout++;
		if(timeout == TMR_LATCH_STS_ERRCNT){
			pr_err("%s: Timer index(%d) counter sync not completely !!!\n", __func__, num);
			return -EINVAL;
		}
	}
	writel(TMR_LATCH_STS_CLEAN, (nvt_timer->va + TMR_LATCH_STS));

	/* 3. Read hrtimer registers */
	count_now = readl(nvt_timer->va + TMR_CNT_L);
	count_now += (u64)readl(nvt_timer->va + TMR_CNT_H) << 32;
	*count = count_now;
	return 0;

#else	/* Debounce */

	u64 count_prev, count_prev1, count_now;
	u64 diff[3], large_diff = TMR_DEBOUNCE_LARGE_DIFF;

	if (nvt_timer == NULL) {
		pr_err("%s: Timer index(%d) not find !!!\n", __func__, num);
		return -EINVAL;
	}

	if (nvt_timer->in_use == false) {
		dev_err(nvt_timer->dev, "Timer(%d) is not in use !!!\n", num);
		return -EINVAL;
	}

	count_prev = readl(nvt_timer->va + TMR_CNT_L);
	count_prev += (u64)readl(nvt_timer->va + TMR_CNT_H) << 32;
	count_prev1 = readl(nvt_timer->va + TMR_CNT_L);
	count_prev1 += (u64)readl(nvt_timer->va + TMR_CNT_H) << 32;
	count_now = readl(nvt_timer->va + TMR_CNT_L);
	count_now += (u64)readl(nvt_timer->va + TMR_CNT_H) << 32;

	/* Review if exists abnormal case */
	diff[0] = count_now - count_prev1;
	diff[1] = count_prev1 - count_prev;
	diff[2] = count_now - count_prev;

	/* large_diff = The debounce threshold, T(unit) = (1 sec)/(hrtimer clock) */
	if (diff[0] < large_diff) {
		*count = count_now;
		return 0;
	}

	if (diff[1] < large_diff) {
		*count = count_prev1;
		return 0;
	}

	if (diff[2] < (large_diff*2)) {
		*count = count_now;
		return 0;
	} else {
		pr_err("%s: Timer index(%d) counter abnormal !!!\n", __func__, num);
		return -EINVAL;
	}

#endif


}
EXPORT_SYMBOL(nvt_timer_get_count);

/* For system timer */
u64 hrtimer_get_target_count(void)
{
	u64 count;
	nvt_timer_get_count(0, &count);
	return count;
}
EXPORT_SYMBOL(hrtimer_get_target_count);

#ifdef NVT_EXT_CNT
void hrtimer_enable_extcnt(u32 en_mask)
{
	unsigned long flags;
	struct nvt_timer_t *nvt_timer = nvt_get_timer_info(0);

	if (nvt_timer == NULL) {
		pr_err("%s: Timer index(%d) not find !!!\n", __func__, 0);
		return;
	}

	if (nvt_timer->in_use == false) {
		dev_err(nvt_timer->dev, "Timer(%d) is not in use !!!\n", 0);
		return;
	}

	spin_lock_irqsave(&nvt_timer->lock, flags);

	writel(en_mask, nvt_timer->va + HRTIMER_EXTCNT_EN_REG);

	spin_unlock_irqrestore(&nvt_timer->lock, flags);
}
EXPORT_SYMBOL(hrtimer_enable_extcnt);

u32 hrtimer_get_extcnt_index(int ext_cnt_id)
{
	unsigned long flags;
	u32 buf_index = 0;
	struct nvt_timer_t *nvt_timer = nvt_get_timer_info(0);

	if (nvt_timer == NULL) {
		pr_err("%s: Timer index(%d) not find !!!\n", __func__, 0);
		return -EINVAL;
	}

	if (nvt_timer->in_use == false) {
		dev_err(nvt_timer->dev, "Timer(%d) is not in use !!!\n", 0);
		return -EINVAL;
	}

	spin_lock_irqsave(&nvt_timer->lock, flags);

	if (ext_cnt_id < 4) {
		buf_index = readl(nvt_timer->va + HRTIMER_EXTCNT_IDX_REG) & 0x1;
	} else {
		buf_index = (readl(nvt_timer->va + HRTIMER_EXTCNT_IDX_REG) & 0x2) >> 1;
	}

	spin_unlock_irqrestore(&nvt_timer->lock, flags);

	return buf_index;
}
EXPORT_SYMBOL(hrtimer_get_extcnt_index);

u64 hrtimertest_get_extcnt_count(int ext_cnt_id, int buf_index)
{
	unsigned long flags;
	u64 hrtimer_extcnt_lb;
	u64 hrtimer_extcnt_ub;
	u64 ext_hrtimer_cnt;
	struct nvt_timer_t *nvt_timer = nvt_get_timer_info(0);

	if (nvt_timer == NULL) {
		pr_err("%s: Timer index(%d) not find !!!\n", __func__, 0);
		return -EINVAL;
	}

	if (nvt_timer->in_use == false) {
		dev_err(nvt_timer->dev, "Timer(%d) is not in use !!!\n", 0);
		return -EINVAL;
	}

	if (buf_index > 1) {
		pr_err("%s: Wrong parameters, buf_index should be either 0 or 1\r\n", __func__);
	}

	spin_lock_irqsave(&nvt_timer->lock, flags);

	if (buf_index == 0) {
		hrtimer_extcnt_lb = readl(nvt_timer->va + HRT_EXT_CNT_LB + ext_cnt_id*CNT_SHIFT);
		hrtimer_extcnt_ub = readl(nvt_timer->va + HRT_EXT_CNT_UB + ext_cnt_id*CNT_SHIFT);
	} else {
		hrtimer_extcnt_lb = readl(nvt_timer->va + HRT_EXT_CNT_LB + ext_cnt_id*CNT_SHIFT + BUF1_SHIFT);
		hrtimer_extcnt_ub = readl(nvt_timer->va + HRT_EXT_CNT_UB + ext_cnt_id*CNT_SHIFT + BUF1_SHIFT);
	}

	spin_unlock_irqrestore(&nvt_timer->lock, flags);

	ext_hrtimer_cnt = (hrtimer_extcnt_ub<<(32)) + hrtimer_extcnt_lb;

#ifdef CONFIG_NVT_FPGA_EMULATION
	return ext_hrtimer_cnt*10;
#else
	return ext_hrtimer_cnt;
#endif
}
EXPORT_SYMBOL(hrtimertest_get_extcnt_count);

#else /* !NVT_EXT_CNT */
inline void hrtimer_enable_extcnt(u32 en_mask) {}
inline u32 hrtimer_get_extcnt_index(int ext_cnt_id) { return -ENOTSUPP; }
inline u64 hrtimertest_get_extcnt_count(int ext_cnt_id, int buf_index) { return -ENOTSUPP; }
EXPORT_SYMBOL(hrtimer_enable_extcnt);
EXPORT_SYMBOL(hrtimer_get_extcnt_index);
EXPORT_SYMBOL(hrtimertest_get_extcnt_count);
#endif /* NVT_EXT_CNT */


int nvt_timer_reload(unsigned int num, u64 count)
{
	unsigned int val;
	unsigned long flags;
	struct nvt_timer_t *nvt_timer = nvt_get_timer_info(num);

	if (nvt_timer == NULL) {
		pr_err("%s: Timer index(%d) not find !!!\n", __func__, num);
		return -EINVAL;
	}

	if (nvt_timer->index == 0) {
		dev_err(nvt_timer->dev, "%s: Timer(0) is used for system timer!!!\n", __func__);
		return -EINVAL;
	}

	if (nvt_timer->in_use == false) {
		dev_err(nvt_timer->dev, "Timer(%d) is not in use !!!\n", num);
		return -EINVAL;
	}

	spin_lock_irqsave(&nvt_timer->lock, flags);

	/* Timer Stop */
	val = readl(nvt_timer->va + TMR_CTRL);
	val &= GENMASK(31, TMR_EN);
	writel(val, (nvt_timer->va + TMR_CTRL));

	/* Set Target Time */
	nvt_timer->target_count = count;
	val = (count) & GENMASK(31, 0);
	writel(val, (nvt_timer->va + TMR_TVAL_L));
	val = (count >> 32) & GENMASK(31, 0);
	writel(val, (nvt_timer->va + TMR_TVAL_H));

	/* Timer Start */
	val = readl(nvt_timer->va + TMR_CTRL);
	val |= ENABLE;
	writel(val, (nvt_timer->va + TMR_CTRL));

	spin_unlock_irqrestore(&nvt_timer->lock, flags);

	return 0;
}
EXPORT_SYMBOL(nvt_timer_reload);

int nvt_timer_stop(unsigned int num)
{
	unsigned int val;
	unsigned long flags;
	struct nvt_timer_t *nvt_timer = nvt_get_timer_info(num);

	if (nvt_timer == NULL) {
		pr_err("%s: Timer index(%d) not find !!!\n", __func__, num);
		return -EINVAL;
	}

	if (nvt_timer->index == 0) {
		dev_err(nvt_timer->dev, "%s: Timer(0) is used for system timer!!!\n", __func__);
		return -EINVAL;
	}

	if (nvt_timer->in_use == false) {
		dev_err(nvt_timer->dev, "Timer(%d) is not in use !!!\n", num);
		return -EINVAL;
	}

	spin_lock_irqsave(&nvt_timer->lock, flags);

	val = readl(nvt_timer->va + TMR_CTRL);
	val &= GENMASK(31, TMR_EN);
	writel(val, (nvt_timer->va + TMR_CTRL));

	spin_unlock_irqrestore(&nvt_timer->lock, flags);

	return 0;
}
EXPORT_SYMBOL(nvt_timer_stop);

int nvt_timer_start(unsigned int num)
{
	unsigned int val;
	unsigned long flags;
	struct nvt_timer_t *nvt_timer = nvt_get_timer_info(num);

	if (nvt_timer == NULL) {
		pr_err("%s: Timer index(%d) not find !!!\n", __func__, num);
		return -EINVAL;
	}

	if (nvt_timer->index == 0) {
		dev_err(nvt_timer->dev, "%s: Timer(0) is used for system timer!!!\n", __func__);
		return -EINVAL;
	}

	if (nvt_timer->in_use == false) {
		dev_err(nvt_timer->dev, "Timer(%d) is not in use !!!\n", num);
		return -EINVAL;
	}

	spin_lock_irqsave(&nvt_timer->lock, flags);

	val = readl(nvt_timer->va + TMR_CTRL);
	val |= ENABLE;
	writel(val, (nvt_timer->va + TMR_CTRL));

	spin_unlock_irqrestore(&nvt_timer->lock, flags);

	return 0;
}
EXPORT_SYMBOL(nvt_timer_start);

int nvt_timer_shutdown(unsigned int num)
{
	unsigned int val;
	unsigned long flags;
	struct nvt_timer_t *nvt_timer = nvt_get_timer_info(num);

	if (nvt_timer == NULL) {
		pr_err("%s: Timer index(%d) not find !!!\n", __func__, num);
		return -EINVAL;
	}

	if (nvt_timer->index == 0) {
		dev_err(nvt_timer->dev, "%s: Timer(0) is used for system timer!!!\n", __func__);
		return -EINVAL;
	}

	if (nvt_timer->in_use == false) {
		dev_err(nvt_timer->dev, "Timer(%d) is not in use !!!\n", num);
		return -EINVAL;
	}

	spin_lock_irqsave(&nvt_timer->lock, flags);

	/* Clear Count */
	nvt_timer->target_count = 0;
	writel(0, (nvt_timer->va + TMR_TVAL_L));
	writel(0, (nvt_timer->va + TMR_TVAL_H));

	/* Disable Interrupt */
	writel(DISABLE, (nvt_timer->va + TMR_INTE_CPU));
	nvt_timer->irqen = false;

	/* Clear Frequency */
	writel(0, (nvt_timer->va + TMR_CLK_DIV));

	/* Stop Timer */
	val = readl(nvt_timer->va + TMR_CTRL);
	val &= GENMASK(31, TMR_EN);
	writel(val, (nvt_timer->va + TMR_CTRL));

	spin_unlock_irqrestore(&nvt_timer->lock, flags);

	return 0;
}
EXPORT_SYMBOL(nvt_timer_shutdown);

int nvt_timer_setup(unsigned int num, bool freerun, u64 count, nvt_timer_callback_t callback)
{
	unsigned int val;
	unsigned long flags;
	struct nvt_timer_t *nvt_timer = nvt_get_timer_info(num);

	if (nvt_timer == NULL) {
		pr_err("%s: Timer index(%d) not find !!!\n", __func__, num);
		return -EINVAL;
	}

	if (nvt_timer->index == 0) {
		dev_err(nvt_timer->dev, "%s: Timer(0) is used for system timer!!!\n", __func__);
		return -EINVAL;
	}

	spin_lock_irqsave(&nvt_timer->lock, flags);

	nvt_timer->in_use = true;

	nvt_timer->callback = callback;

	/* Set Count */
	nvt_timer->target_count = count;
	val = (count) & GENMASK(31, 0);
	writel(val, (nvt_timer->va + TMR_TVAL_L));
	val = (count >> 32) & GENMASK(31, 0);
	writel(val, (nvt_timer->va + TMR_TVAL_H));

	/* Set Re-Load */
	writel(DISABLE, (nvt_timer->va + TMR_RLD));

	/* Set Interrupt */
	writel(ENABLE, (nvt_timer->va + TMR_INTE_CPU));
	nvt_timer->irqen = true;

	/* Set Frequency */
	writel(0, (nvt_timer->va + TMR_CLK_DIV));

	/* Start Timer */
	val = readl(nvt_timer->va + TMR_CTRL);
	val |= ENABLE;
	if (freerun)
		val |= TMR_FREE_RUN;
	else
		val &= TMR_ONE_SHOT;
	writel(val, (nvt_timer->va + TMR_CTRL));

	spin_unlock_irqrestore(&nvt_timer->lock, flags);

	return 0;
}
EXPORT_SYMBOL(nvt_timer_setup);

void nvt_timer_dump(void)
{
	int i = 0;
	u64 count;
	struct nvt_timer_t *nvt_timer = NULL;

	do {
		nvt_timer = nvt_get_timer_info(i);

		if (nvt_timer != NULL) {
			if (nvt_timer_get_count(i, &count) != 0) {
				count = 0;
			}
			dev_info(nvt_timer->dev, "Timer(%d) is %s, count = %lld\n", nvt_timer->index, nvt_timer->in_use ? "in use" : "not use", count);
			i++;
		}

	} while (nvt_timer);
}

static irqreturn_t nvt_interrupt(int irq, void *dev_id)
{
	unsigned long flags;
	struct nvt_timer_t *nvt_timer = (struct nvt_timer_t *)dev_id;

	spin_lock_irqsave(&nvt_timer->lock, flags);

	writel(CLEAN, (nvt_timer->va + TMR_STS_CPU));

	spin_unlock_irqrestore(&nvt_timer->lock, flags);

	if (nvt_timer->callback != NULL) {
		nvt_timer->callback();
	}

	return IRQ_HANDLED;

}

void nvt_system_timer_init(struct nvt_timer_t *nvt_timer)
{
	unsigned int val;
	unsigned long flags;

	if (nvt_timer->in_use) {
		dev_info(nvt_timer->dev, "Timer(%d) is busy\n", nvt_timer->index);
		return;
	}
	spin_lock_irqsave(&nvt_timer->lock, flags);

	nvt_timer->in_use = true;

	nvt_timer->callback = NULL;

	/* Set Interrupt */
	writel(DISABLE, (nvt_timer->va + TMR_INTE_CPU));
	nvt_timer->irqen = false;

	/* Start Timer */
	val = readl(nvt_timer->va + TMR_CTRL);
	val |= ENABLE;
	val |= TMR_FREE_RUN;
	writel(val, (nvt_timer->va + TMR_CTRL));

	spin_unlock_irqrestore(&nvt_timer->lock, flags);

	return;
}

static int nvt_timer_probe(struct platform_device *pdev)
{
	int ret = 0;
	struct resource *res;
	int irq;
	struct nvt_timer_t *nvt_timer = NULL;

	nvt_timer = devm_kzalloc(&pdev->dev, sizeof(struct nvt_timer_t), GFP_KERNEL);
	if (nvt_timer == NULL) {
		dev_err(&pdev->dev, "memory alloc fail !!!\n");
		return -ENOMEM;
	}
	nvt_timer->dev = &pdev->dev;

	if (device_property_read_u32(&pdev->dev, "index", &nvt_timer->index) < 0) {
		dev_err(&pdev->dev, "index not find on device tree !!!\n");
		return -EINVAL;
	}

	platform_set_drvdata(pdev, nvt_timer);

	list_add_tail(&nvt_timer->list, &timer_list_root);

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (res == NULL) {
		dev_err(&pdev->dev, "no memory resource defined\n");
		return -ENODEV;
	}

	nvt_timer->va = devm_ioremap_resource(&pdev->dev, res);
	if (IS_ERR(nvt_timer->va)) {
		dev_err(&pdev->dev, "ioremap fail !!!\n");
		return PTR_ERR(nvt_timer->va);
	}

	irq = platform_get_irq(pdev, 0);
	if (irq < 0) {
		dev_err(&pdev->dev, "no irq resource: %d\n", irq);
		return irq;
	}

	ret = request_irq(irq, nvt_interrupt, 0, pdev->name, nvt_timer);
	if (ret) {
		dev_err(&pdev->dev, "request_irq fail\n");
		return ret;
	}

	nvt_timer->clk = devm_clk_get(&pdev->dev, NULL);
	if (IS_ERR(nvt_timer->clk)) {
		dev_err(&pdev->dev, "clk_get fail\n");
		return PTR_ERR(nvt_timer->clk);
	}

	ret = clk_prepare_enable(nvt_timer->clk);
	if (ret) {
		clk_disable_unprepare(nvt_timer->clk);
		dev_err(&pdev->dev, "clk_prepare_enable fail\n");
		return ret;
	}

	spin_lock_init(&nvt_timer->lock);

	if (nvt_timer->index == 0) {
		nvt_system_timer_init(nvt_timer);
	}

	dev_info(&pdev->dev, "nvt timer init success\n");

	return 0;
}

static int nvt_timer_remove(struct platform_device *pdev)
{
	struct nvt_timer_t *nvt_timer = platform_get_drvdata(pdev);

	nvt_timer->in_use = false;
	nvt_timer_proc_exit();
	clk_disable_unprepare(nvt_timer->clk);
	kfree(nvt_timer);
	return 0;
}

#ifdef CONFIG_PM
static int nvt_timer_suspend(struct device *ddev)
{
	unsigned int val = 0;
	struct nvt_timer_t *nvt_timer = dev_get_drvdata(ddev);

	if (!nvt_get_suspend_mode())
		return 0;

	if (nvt_timer == NULL)
		return 0;

	if (!nvt_timer->in_use)
		goto suspend_done;

	nvt_timer->freq = clk_get_rate(nvt_timer->clk);

	val = readl(nvt_timer->va + TMR_CTRL);
	if (val & TMR_FREE_RUN)
		nvt_timer->freerun = true;
	else
		nvt_timer->freerun = false;

	val = readl(nvt_timer->va + TMR_CTRL);
	val &= GENMASK(31, TMR_EN);
	/* Save timer counts */
	nvt_timer_get_count(nvt_timer->index, &nvt_timer->count);
	/* Stop timer */
	writel(val, (nvt_timer->va + TMR_CTRL));

suspend_done:
	clk_disable_unprepare(nvt_timer->clk);

	pr_info("%s: successfully\n", __func__);

	return 0;
}

static int nvt_timer_resume(struct device *ddev)
{
	unsigned int val = 0;
	struct nvt_timer_t *nvt_timer = dev_get_drvdata(ddev);

	if (!nvt_get_suspend_mode())
		return 0;

	if (nvt_timer == NULL)
		return 0;

	/* Set Interrupt */
	if (nvt_timer->irqen)
		writel(ENABLE, (nvt_timer->va + TMR_INTE_CPU));

	/* Set Frequency */
	writel(0, (nvt_timer->va + TMR_CLK_DIV));

	clk_set_rate(nvt_timer->clk, nvt_timer->freq);
	clk_prepare_enable(nvt_timer->clk);

	if (!nvt_timer->in_use)
		return 0;

	/* Set up timer*/
	if (nvt_timer->freerun) {
		val |= TMR_FREE_RUN;
	} else {
		val &= TMR_ONE_SHOT;
	}
	val |= TMR_INIT_COUNT_INIT;
	writel(val, nvt_timer->va + TMR_CTRL);

	/* Set up initial count */
	val = nvt_timer->count & GENMASK(31, 0);
	writel(val, nvt_timer->va + TMR_INIT_TVAL_L);
	val = (nvt_timer->count >> 32) & GENMASK(31, 0);
	writel(val, nvt_timer->va + TMR_INIT_TVAL_H);

	/* Set up target count */
	val = (nvt_timer->target_count) & GENMASK(31, 0);
	writel(val, (nvt_timer->va + TMR_TVAL_L));
	val = (nvt_timer->target_count >> 32) & GENMASK(31, 0);
	writel(val, (nvt_timer->va + TMR_TVAL_H));

	/* Start Timer */
	val = readl(nvt_timer->va + TMR_CTRL);
	val |= ENABLE;
	writel(val, (nvt_timer->va + TMR_CTRL));

	/* Disable init count */
	val = readl(nvt_timer->va + TMR_CTRL);
	val &= ~TMR_INIT_COUNT_INIT;
	writel(val, (nvt_timer->va + TMR_CTRL));

	pr_info("%s: successfully\n", __func__);

	return 0;
}

static const struct dev_pm_ops nvt_timer_pm_ops = {
	.suspend  = nvt_timer_suspend,
	.resume = nvt_timer_resume,
};
#endif

static struct of_device_id xxx_match_table[] = {
	{.compatible = "nvt,nvt_hwtimer"},
	{}
};

static struct platform_driver nvt_timer_driver = {
	.driver = {
		.name   = "nvt_hwtimer",
		.owner = THIS_MODULE,
		.of_match_table = xxx_match_table,
#ifdef CONFIG_PM
		.pm = &nvt_timer_pm_ops,
#endif
	},
	.probe      = nvt_timer_probe,
	.remove     = nvt_timer_remove,
};


int __init nvt_timer_init(void)
{
	int ret;

	INIT_LIST_HEAD(&timer_list_root);

	ret = nvt_timer_proc_init();
	if (ret) {
		pr_err("nvt_timer_proc_init fail\n");
		return ret;
	}

	return platform_driver_register(&nvt_timer_driver);
}

void __exit nvt_timer_exit(void)
{
	platform_driver_unregister(&nvt_timer_driver);
}

arch_initcall(nvt_timer_init);
module_exit(nvt_timer_exit);

MODULE_AUTHOR("Novatek Corp.");
MODULE_DESCRIPTION("nvt timer driver");
MODULE_VERSION("1.00.004");
MODULE_LICENSE("GPL");
