/**
    NVT SRAM Contrl
    This file will Enable and disable SRAM shutdown
    @file       nvt-sramctl.c
    @ingroup
    @note
    Copyright   Novatek Microelectronics Corp. 2018.  All rights reserved.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License version 2 as
    published by the Free Software Foundation.
*/

#include <plat/nvt-sramctl.h>
#include <linux/spinlock.h>

static void __iomem *remap_base;
#define NVT_SRAM_SETREG(ofs, value)	writel((value), (remap_base+(ofs)))
#define NVT_SRAM_GETREG(ofs)		readl(remap_base+(ofs))

#ifdef CONFIG_NVT_IVOT_PLAT_NS02301
static void __iomem *remap_pr_base;
#define NVT_PR_SRAM_SETREG(ofs, value)     writel((value), (remap_pr_base+(ofs)))
#define NVT_PR_SRAM_GETREG(ofs)            readl(remap_pr_base+(ofs))
#endif

#ifdef __KERNEL__
static DEFINE_SPINLOCK(my_lock);
#define loc_cpu(flags)   spin_lock_irqsave(&my_lock, flags)
#define unl_cpu(flags)   spin_unlock_irqrestore(&my_lock, flags)
#endif


void nvt_disable_sram_shutdown(SRAM_SD id)
{
	unsigned long flags;
	unsigned long reg_data, reg_ofs;

	reg_ofs = (id >> 5) << 2;

	loc_cpu(flags);

	reg_data = NVT_SRAM_GETREG(reg_ofs);

	reg_data &= ~(1 << (id & 0x1F));

	NVT_SRAM_SETREG(reg_ofs, reg_data);

	unl_cpu(flags);
}

void nvt_enable_sram_shutdown(SRAM_SD id)
{
	unsigned long flags;
	unsigned long reg_data, reg_ofs;

	reg_ofs = (id >> 5) << 2;

	loc_cpu(flags);

	reg_data = NVT_SRAM_GETREG(reg_ofs);

	reg_data |= (1 << (id & 0x1F));

	NVT_SRAM_SETREG(reg_ofs, reg_data);

	unl_cpu(flags);
}

#if IS_ENABLED(CONFIG_NVT_IVOT_PLAT_NS02302)
void nvt_bypass_sram_shutdown(SRAM_SD id)
{
    int idx;

    for (idx = 0; idx < sram_shutdown_size; idx++) {
		if (nvt_sram_sd_tbl[idx].sram_shutdown_id == id) {
			nvt_sram_sd_tbl[idx].enable = FALSE;
		}
	}
}

void nvt_preset_sram_shutdown(void)
{
    struct device_node *sram_sd_np = of_find_node_by_name(NULL,"top");
    int sram_sd_bypass_size = -1;
    unsigned int *user_data = NULL;
    int rt, idx;
    // int rt, flg, idx, idx2;

    sram_sd_bypass_size = of_property_count_u32_elems(sram_sd_np, "sram_sd_bypass");
    if (sram_sd_bypass_size >= 0) {
		user_data = kzalloc(sizeof(unsigned int) * sram_sd_bypass_size, GFP_KERNEL);
		if (user_data && (rt = of_property_read_u32_array(sram_sd_np, "sram_sd_bypass", user_data, sram_sd_bypass_size)) == 0) {
            for (idx = 0; idx < sram_sd_bypass_size; idx++) {
                nvt_bypass_sram_shutdown(user_data[idx]);
            }
            kfree(user_data);
		}
	} else {
        return;
    }

    for (idx = 0; idx < sram_shutdown_size; idx++) {
		if (nvt_sram_sd_tbl[idx].enable) {
			nvt_enable_sram_shutdown(nvt_sram_sd_tbl[idx].sram_shutdown_id);
		}
	}
}
#endif


static int sram_count=0;

/**
    lock sram resource before use sram.

    @return int.          0,lock success,-1,lock fail
*/
int sram_trylock(void)
{
	unsigned long flags;

    //pr_err("sram_count %d\r\n",sram_count);
    loc_cpu(flags);
    if(!sram_count) {
        sram_count++;
        unl_cpu(flags);
        return 0;
    } else {
        pr_err("%s sram is lock %d \r\n",__FUNCTION__,sram_count);
        unl_cpu(flags);
        return -1;
    }
}


/**
    unlock sram resource after use sram.

    @return int.          0,unlock success,-1,unlock fail
*/
int sram_unlock(void)
{
	unsigned long flags;

    //pr_err("sram_count %d\r\n",sram_count);
    loc_cpu(flags);
    if(!sram_count){
        pr_err("%s alread unlock %d\r\n",__FUNCTION__,sram_count);
        unl_cpu(flags);
        return -1;
    } else {
        sram_count--;
        unl_cpu(flags);
        return 0;
    }
}

#ifdef CONFIG_NVT_IVOT_PLAT_NS02301
void nvt_disable_pr_sram_shutdown(PR_SRAM_SD id)
{
        unsigned long flags;
        unsigned long reg_data, reg_ofs;

        reg_ofs = (id >> 5) << 2;

        loc_cpu(flags);

        reg_data = NVT_PR_SRAM_GETREG(reg_ofs);

        reg_data &= ~(1 << (id & 0x1F));

        NVT_PR_SRAM_SETREG(reg_ofs, reg_data);

        unl_cpu(flags);
}

void nvt_enable_pr_sram_shutdown(PR_SRAM_SD id)
{
        unsigned long flags;
        unsigned long reg_data, reg_ofs;

        reg_ofs = (id >> 5) << 2;

        loc_cpu(flags);

        reg_data = NVT_PR_SRAM_GETREG(reg_ofs);

        reg_data |= (1 << (id & 0x1F));

        NVT_PR_SRAM_SETREG(reg_ofs, reg_data);

        unl_cpu(flags);
}
#endif

#ifdef CONFIG_NVT_IVOT_PLAT_NA51090
#define QOS_OFS 0x100
void nvt_set_qos(QOS_ID id, QOS_LEVEL level)
{
	unsigned long reg_data, reg_ofs;
	unsigned long      flags;
	unsigned long bit_ofs;

	reg_ofs = (id >> 5) << 2;
	bit_ofs = id % 32;
	level &= 0xF;

	loc_cpu(flags);

	reg_data = NVT_SRAM_GETREG(QOS_OFS + reg_ofs);
	reg_data &= ~((0xF) << bit_ofs);
	reg_data |= (level << bit_ofs);

	NVT_SRAM_SETREG(QOS_OFS + reg_ofs, reg_data);

	unl_cpu(flags);
}
EXPORT_SYMBOL(nvt_set_qos);
#endif

static int __init nvt_init_sram_membase(void)
{
	remap_base = ioremap(NVT_SRAM_BASE_PHYS, 0x1000);
	if (unlikely(remap_base == 0)) {
		pr_err("%s fails: ioremap fail\n", __FUNCTION__);
		return -1;
	}

	#ifdef CONFIG_NVT_IVOT_PLAT_NS02301
	remap_pr_base = ioremap(NVT_PR_SRAM_BASE_PHYS, 0x200);
	if (unlikely(remap_pr_base == 0)) {
		pr_err("%s fails: ioremap pre-roll fail\n", __FUNCTION__);
		return -1;
	}
	#endif

#if IS_ENABLED(CONFIG_NVT_IVOT_PLAT_NS02302)
    nvt_preset_sram_shutdown();
#endif

	pr_info("sramctl ver 0.0.1 \n");
	return 0;
}

core_initcall(nvt_init_sram_membase);
EXPORT_SYMBOL(nvt_enable_sram_shutdown);
EXPORT_SYMBOL(nvt_disable_sram_shutdown);
EXPORT_SYMBOL(sram_trylock);
EXPORT_SYMBOL(sram_unlock);
#ifdef CONFIG_NVT_IVOT_PLAT_NS02301
EXPORT_SYMBOL(nvt_disable_pr_sram_shutdown);
EXPORT_SYMBOL(nvt_enable_pr_sram_shutdown);
#endif
