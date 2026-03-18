/**
    NVT clocksource function
    NVT clocksource driver
    @file       timer-novatek.c
    @ingroup
    @note
    Copyright   Novatek Microelectronics Corp. 2021.  All rights reserved.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License version 2 as
    published by the Free Software Foundation.
*/
#include <linux/interrupt.h>
#include <linux/clockchips.h>
#include <linux/clocksource.h>
#include <linux/irqreturn.h>
#include <linux/sched_clock.h>
#include <linux/slab.h>
#include <linux/clk.h>
#include <linux/of_irq.h>
#include <linux/of_address.h>
#include "timer-of.h"
#include <plat/hrtimer-reg.h>

static struct timer_of to = {
	.flags = TIMER_OF_BASE | TIMER_OF_IRQ | TIMER_OF_CLOCK,

	.clkevt = {
		.rating = 100,
		.cpumask = cpu_possible_mask,
	},

	.of_irq = {
		.flags = IRQF_TIMER,
	},
};

#ifdef NVT_CLOCKSOURCE
static u64 notrace nvt_read_sched_clock(void)
{
	u64 ns = readl(timer_of_base(&to) + TMR_CNT_L);
	ns += (u64)readl(timer_of_base(&to) + TMR_CNT_H) << 32;
        return ns;
}

static u64 notrace nvt_read_mmio_clock(struct clocksource *c)
{
	u64 ns = readl(timer_of_base(&to) + TMR_CNT_L);
	ns +=(u64)readl(timer_of_base(&to) + TMR_CNT_H) << 32;
        return ns & c->mask;
}
#endif

static void nvt_clkevt_time_stop(struct timer_of *to)
{
	u32 val;

	val = readl(timer_of_base(to) + TMR_CTRL);
	val &= GENMASK(31, TMR_EN);
	writel(val, (timer_of_base(to) + TMR_CTRL));
}

static void nvt_clkevt_time_setup(struct timer_of *to,
				  unsigned long delay)
{
	u32 val;

	val = (delay) & GENMASK(31, 0);
	writel(val, (timer_of_base(to) + TMR_TVAL_L));
#ifdef CONFIG_64BIT
	val = (delay >> 32) & GENMASK(31, 0);
	writel(val, (timer_of_base(to) + TMR_TVAL_H));
#endif
}

static void nvt_clkevt_time_start(struct timer_of *to,
				  bool periodic)
{
	u32 val;

	val = readl(timer_of_base(to) + TMR_CTRL);
	val |= ENABLE;
	if (periodic) {
		val |= TMR_FREE_RUN;
	} else {
		val &= TMR_ONE_SHOT;
	}
	writel(val, (timer_of_base(to) + TMR_CTRL));
}

static int nvt_clkevt_shutdown(struct clock_event_device *clk)
{
	nvt_clkevt_time_stop(to_timer_of(clk));

	return 0;
}

static int nvt_clkevt_set_periodic(struct clock_event_device *clk)
{
	struct timer_of *to = to_timer_of(clk);

	nvt_clkevt_time_stop(to);
	nvt_clkevt_time_setup(to, to->of_clk.period);
	nvt_clkevt_time_start(to, true);
	return 0;
}

static int nvt_clkevt_set_oneshot(struct clock_event_device *clk)
{
	struct timer_of *to = to_timer_of(clk);

	nvt_clkevt_time_stop(to);
	nvt_clkevt_time_setup(to, to->of_clk.period);
	nvt_clkevt_time_start(to, false);

	return 0;
}

static int nvt_clkevt_next_event(unsigned long event,
				 struct clock_event_device *clk)
{
	struct timer_of *to = to_timer_of(clk);

	nvt_clkevt_time_stop(to);
	nvt_clkevt_time_setup(to, event);
	nvt_clkevt_time_start(to, false);

	return 0;
}

static irqreturn_t nvt_interrupt(int irq, void *dev_id)
{
	struct clock_event_device *clkevt = (struct clock_event_device *)dev_id;
	int ret = IRQ_HANDLED;

	writel(CLEAN, (timer_of_base(&to) + TMR_STS_CPU));
	if (clkevt->event_handler != NULL) {
		clkevt->event_handler(clkevt);
	}

	return ret;
}

static int
__init nvt_setup(struct timer_of *to, u32 freq)
{
	u32 val;
	s8 freq_bit = (TMR_MAX_HZ / freq) -1;

	if (freq_bit < 0) {
		pr_err("%s: freq too fast should <= %d\n", __func__, TMR_MAX_HZ);
		return -EINVAL;
	}

	if (freq_bit > TMR_MAX_DIV_BIT) {
		pr_err("%s: freq too slow should >= %lld\n", __func__, TMR_MAX_HZ / (TMR_MAX_DIV_BIT + 1));
		return -EINVAL;
	}

	/* Set Max Count */
	val = (TMR_MAX_VALUE) & GENMASK(31, 0);
	writel(val, (timer_of_base(to) + TMR_TVAL_L));
	val = (TMR_MAX_VALUE >> 32) & GENMASK(31, 0);
	writel(val, (timer_of_base(to) + TMR_TVAL_H));

	/* Set Re-Load */
	writel(DISABLE, (timer_of_base(to) + TMR_RLD));

	/* Set Interrupt */
	writel(ENABLE, (timer_of_base(to) + TMR_INTE_CPU));

	/* Set Frequency */
	writel(freq_bit, (timer_of_base(to) + TMR_CLK_DIV));

	/* Start Timer */
	val = readl(timer_of_base(to) + TMR_CTRL);
	val |= ENABLE | TMR_FREE_RUN;
	writel(val, (timer_of_base(to) + TMR_CTRL));

	return 0;
}

static int __init nvt_common_init(struct device_node *node)
{
	int ret = 0;
	u32 *pfreq = 0;
	u32 freq;
	struct resource res;

	to.of_clk.clk = of_clk_get(node, 0);
	if (IS_ERR(to.of_clk.clk)) {
		pr_err("%s: clk_get fail\n", __func__);
		return PTR_ERR(to.of_clk.clk);
	}
	if (clk_prepare_enable(to.of_clk.clk) != 0) {
		pr_err("%s: enable clk fail\n", __func__);
		return PTR_ERR(to.of_clk.clk);
	}

	to.clkevt.features = CLOCK_EVT_FEAT_PERIODIC | CLOCK_EVT_FEAT_ONESHOT;
	to.clkevt.set_state_shutdown = nvt_clkevt_shutdown;
	to.clkevt.set_state_periodic = nvt_clkevt_set_periodic;
	to.clkevt.set_state_oneshot = nvt_clkevt_set_oneshot;
	to.clkevt.tick_resume = nvt_clkevt_shutdown;
	to.clkevt.set_next_event = nvt_clkevt_next_event;
	to.of_irq.handler = nvt_interrupt;

	pfreq = (u32 *)of_get_property(node, "clock-frequency", NULL);
	if (pfreq) {
		freq = __be32_to_cpu(*pfreq);
	} else {
		pr_err("%s: Get clock-frequency fail\n", __func__);
	}

	ret = of_address_to_resource(node, 0, &res);
	if (ret) {
		pr_err("%s: of_address_to_resource fail\n", __func__);
		goto resource_err;
	}

	if (!request_mem_region(res.start, resource_size(&res), node->full_name)) {
		pr_err("%s: request_mem_region fail\n", __func__);
		goto req_mem_err;
	}
	ret = timer_of_init(node, &to);
	if (ret)
		goto time_init_err;

	ret = nvt_setup(&to, freq);
	if (ret)
		goto time_setup_err;

	/* configure clock source */
#ifdef NVT_CLOCKSOURCE
	clocksource_mmio_init(NULL,
			      node->full_name, timer_of_rate(&to), freq, 64,
			      nvt_read_mmio_clock);

	sched_clock_register(nvt_read_sched_clock, 64, timer_of_rate(&to));
#endif
	/* configure clock event */
	clockevents_config_and_register(&to.clkevt, timer_of_rate(&to),
					1, TMR_MAX_VALUE);
	pr_info("%s: %s init done\n", __func__, node->full_name);
	return 0;

time_setup_err:
time_init_err:
	release_mem_region(res.start, (res.end - res.start + 1));
req_mem_err:
resource_err:
	clk_disable(to.of_clk.clk);
	clk_put(to.of_clk.clk);

	return ret;
}

static __init int nvt_timer_init(struct device_node *np)
{
	return nvt_common_init(np);
}
TIMER_OF_DECLARE(nvt_timer, "nvt,nvt_clk_src", nvt_timer_init);
