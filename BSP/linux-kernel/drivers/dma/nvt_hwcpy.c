// SPDX-License-Identifier: GPL-2.0-only
/*
 * Driver for the Novatek HWCPY Controller
 *
 * Copyright (C) 2022 Novatek Corporation
 *
 */

#include <asm/barrier.h>
#include <linux/clk.h>
#include <linux/dmaengine.h>
#include <linux/dmapool.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/kernel.h>
#include <linux/list.h>
#include <linux/module.h>
#include <linux/of_dma.h>
#include <linux/of_platform.h>
#include <linux/platform_device.h>
#include <linux/dma-mapping.h>
#include <linux/delay.h>
#include <linux/proc_fs.h>
#include <linux/timekeeping.h>
//#include <linux/pm.h>

#include "dmaengine.h"

#define DRV_VERSION         "1.04.00"
#define NVT_DMA_MAX_CHAN    4

#define NVT_DMA_CTL     0x00    /* Operation Control Register */
#define     NVT_DMA_CH_TRIG BIT(0)  /* Channel Software Request Trigger */
#define     NVT_DMA_CFG_LL  BIT(1)  /* Link List */
#define NVT_DMA_CH0_OP  0x00    /* CH0 Operation Register */
#define NVT_DMA_CH1_OP  0x04    /* CH1 Operation Register */
#define NVT_DMA_CH2_OP  0x08    /* CH2 Operation Register */
#define NVT_DMA_CH3_OP  0x0C    /* CH3 Operation Register */

#define NVT_DMA_IE      0x14    /* Global Interrupt Enable Register */
#define     NVT_DMA_IE_EN   0xFFFF  /* Interrupt Enable */
#define     NVT_DMA_IE_DIS  0   /* Interrupt Disable */
#define NVT_DMA_STS     0x18    /* Global Interrupt Status Register */
#define NVT_DMA_GENERAL 0x1C    /* General Setting Register */

#define NVT_DMA_CFG     0x00    /* Operation Config Register */
#define     NVT_DMA_CFG_SET 0x0 /* Linear Set */
#define     NVT_DMA_CFG_CPY 0x2 /* Linear Copy */
#define NVT_DMA_SRC_ADDR    0x04    /* Source Address Register */
#define NVT_DMA_SRC_ADDR_H  0x08    /* Source Address MSB Register */
#define NVT_DMA_DEST_ADDR   0x14    /* Destination Address Register */
#define NVT_DMA_DEST_ADDR_H 0x18    /* Destination Address MSB Register */
#define NVT_DMA_LEN     0x30    /* Data Length Register */
#define NVT_DMA_CONT    0x34    /* Constant Value Register */

#define NVT_DMA_LL_CTRL 0x50    /* Link List Control Register */
#define NVT_DMA_LL_SRC_ADDR     0x54/* Link List Start Address Register */
#define NVT_DMA_LL_SRC_ADDR_H   0x58/* Link List Start Address MSB Register */

#define NVT_DMA_LL_TAB0 0x68    /* Link List Table Index Register 0 */
#define NVT_DMA_LL_TAB1 0x6C    /* Link List Table Index Register 1 */
#define NVT_DMA_LL_TAB2 0x70    /* Link List Table Index Register 2 */
#define NVT_DMA_LL_TAB3 0x74    /* Link List Table Index Register 3 */

/* Global Interrupt Enable/Status Register */
#define NVT_DMA_INT_CPYINT  0   /* HWCPY INT, Shift bit */
#define NVT_DMA_INT_LLDONE  4   /* LL Finish All Done, Shift bit */
#define NVT_DMA_INT_LLERR   8   /* LL ERROR, Shift bit */
#define NVT_DMA_INT_LLJOB   12  /* LL Reg Finish or Terminate, Shift bit */

#define NVT_DMA_LEN_MAX 0x3FFFFF0UL /* Maximum DATA Length */
#define NVT_DMA_MAX_BURST_LENGTH    (64 * 4)    /* 64 words */
#define NVT_DMA_MAX_DWIDTH  8   /* 64 bits */
#define NVT_DMA_MAX_LLD 64  /* link list cmd set */
#define NVT_DMA_MAX_CMD 16  /* link list max count */

#define NVT_DMA_DMA_BUSWIDTHS\
	(BIT(DMA_SLAVE_BUSWIDTH_1_BYTE) |\
	 BIT(DMA_SLAVE_BUSWIDTH_2_BYTES) |\
	 BIT(DMA_SLAVE_BUSWIDTH_4_BYTES) |\
	 BIT(DMA_SLAVE_BUSWIDTH_8_BYTES))

enum atc_status {
	NVT_DMA_CHAN_IS_ENABLE = 0,
	NVT_DMA_CHAN_IS_LL = 1,
};

/* ----- Channels ----- */
struct nvt_dma_chan {
	struct dma_chan chan;
	void __iomem    *ch_regs;
	u32             ch_id;      /* Channel ID */
	u32             cfg;        /* Channel Configuration Register */
	u32             irq_status;
	unsigned long           status;
	struct tasklet_struct   tasklet;
	struct dma_slave_config sconfig;

	spinlock_t          lock;

	struct list_head    xfers_list;
	struct list_head    free_descs_list;

	dma_addr_t dma_paddr;
	void *dma_vaddr;
#ifdef CONFIG_PM
	bool pm_state;
#endif
};


/* ----- Controller ----- */
struct nvt_dma {
	struct dma_device   dma;
	void __iomem        *base;
	int                 irq;
	spinlock_t          lock;
	struct clk          *clk;
	struct dma_pool     *nvt_dma_desc_pool;
	struct nvt_dma_chan chan[];
};


/* ----- Descriptors ----- */

/* Linked List Descriptor */
struct nvt_dma_lld {
	u32 mbr_nda;    /* Next Descriptor Member */
	u32 mbr_sa;     /* Source Address Member */
	u32 mbr_sa_h;   /* Source Address Member MSB */
	u32 mbr_da;     /* Destination Address Member */
	u32 mbr_da_h;   /* Destination Address Member MSB */
	u32 mbr_cfg;    /* Configuration Register */
	u32 mbr_len;    /* Data Length Register */
	u32 mbr_value;  /* Data Constant Value Register */
	u32 mbr_cmd;    /* Command Member */

	unsigned long mbr_ll_paddr;/* Link List phy addr */
	unsigned long mbr_ll_vaddr;/* Link List virt addr */
};

/* 64-bit alignment needed to update CNDA and CUBC registers in an atomic way. */
struct nvt_dma_desc {
	struct nvt_dma_lld      lld;
	enum dma_transfer_direction direction;
	struct dma_async_tx_descriptor  tx_dma_desc;
	struct list_head        desc_node;
	/* Following members are only used by the first descriptor */
	bool                active_xfer;
	unsigned int            xfer_size;
	struct list_head        descs_list;
	struct list_head        xfer_node;
} __aligned(sizeof(u64));

struct cyclic_data {
	u64 cmd;
	u64 src_addr;
	u64 src_addr_h;
	u64 dest_addr;
	u64 dest_addr_h;
	u64 length;
	u64 cont;
};

#define TIME_PERIOD  5000000 /* 5s */

static s64 recordTime[NVT_DMA_MAX_CHAN];	/* start record */
static s64 startTime[NVT_DMA_MAX_CHAN];	/* trig */
static s64 endTime[NVT_DMA_MAX_CHAN];		/* irq */
static s64 durationSum[NVT_DMA_MAX_CHAN] = {0};
static int utility[NVT_DMA_MAX_CHAN] = {0};
static struct proc_dir_entry *pmodule_root = NULL;
static struct proc_dir_entry *pentry = NULL;

static void calc_time(struct device *dev, int id)
{
	s64 period_ts;

	period_ts = endTime[id] - recordTime[id];
	durationSum[id] += (endTime[id] - startTime[id]);

	if (period_ts > TIME_PERIOD) {
		utility[id] = durationSum[id] * 100 / period_ts;
		dev_dbg(dev, "uti = %d, Sum = %lld, period = %lld\n", utility[id], durationSum[id], period_ts);
		durationSum[id] = 0;
	}
}

static inline void __iomem *nvt_dma_chan_reg_base(struct nvt_dma *nvtdmac, unsigned int chan_nb)
{
	return nvtdmac->base + ((chan_nb + 1) * 0x100);
}

#define nvt_dma_read(nvtdmac, reg) readl(nvtdmac->base + reg)
#define nvt_dma_write(nvtdmac, reg, value) \
	writel(value, nvtdmac->base + reg)

//#define nvt_dma_chan_read(nvtchan, reg) readl_relaxed((nvtchan)->ch_regs + (reg))
//#define nvt_dma_chan_write(nvtchan, reg, value) writel_relaxed((value), (nvtchan)->ch_regs + (reg))

static unsigned int nvt_dma_chan_read(struct nvt_dma_chan *nvtchan, unsigned int reg)
{
	u32 value;

	value = readl_relaxed((nvtchan)->ch_regs + (reg));
	//printk("read id %d, reg 0x%x, data 0x%x\n", nvtchan->ch_id, reg, value);
	return value;
}

static void nvt_dma_chan_write(struct nvt_dma_chan *nvtchan, unsigned int reg, unsigned int value)
{
	//printk("write id %d, reg 0x%x, value 0x%x\n", nvtchan->ch_id, reg, value);
	writel_relaxed((value), (nvtchan)->ch_regs + (reg));
}

static inline struct nvt_dma_chan *to_nvt_dma_chan(struct dma_chan *dchan)
{
	return container_of(dchan, struct nvt_dma_chan, chan);
}

static struct device *chan2dev(struct dma_chan *chan)
{
	return &chan->dev->device;
}

static inline struct nvt_dma *to_nvt_dma(struct dma_device *ddev)
{
	return container_of(ddev, struct nvt_dma, dma);
}

static inline struct nvt_dma_desc *txd_to_at_desc(struct dma_async_tx_descriptor *txd)
{
	return container_of(txd, struct nvt_dma_desc, tx_dma_desc);
}

static inline int nvt_dma_chan_is_linklist(struct nvt_dma_chan *nvtchan)
{
	return test_bit(NVT_DMA_CHAN_IS_LL, &nvtchan->status);
}

static unsigned int init_nr_desc_per_channel = 64;
module_param(init_nr_desc_per_channel, uint, 0644);
MODULE_PARM_DESC(init_nr_desc_per_channel,
				 "initial descriptors per channel (default: 64)");


static bool nvt_dma_chan_is_enabled(struct nvt_dma_chan *nvtchan)
{
	struct nvt_dma *nvtdmac = to_nvt_dma(nvtchan->chan.device);
	//dev_dbg(chan2dev(&nvtchan->chan),
	//   "enable channel %d(0x%08x)\n", nvtchan->ch_id, readl(nvtdmac->base + (nvtchan->ch_id << 2)));
	return readl(nvtdmac->base + (nvtchan->ch_id << 2)) & 0x3;
}

static void nvt_dma_off(struct nvt_dma *nvtdmac)
{
	unsigned int value;//i

	/* DMA disable */
	value = readl(nvtdmac->base + NVT_DMA_GENERAL);
	writel(value | BIT(1), nvtdmac->base + NVT_DMA_GENERAL);

	value = readl(nvtdmac->base + NVT_DMA_IE);
	writel(0, nvtdmac->base + NVT_DMA_IE);

	/* Wait that all chans are disabled. */
	cpu_relax();
}

/* Call with lock hold. */
static void nvt_dma_start_xfer(struct nvt_dma_chan *nvtchan,
							   struct nvt_dma_desc *first)
{
	struct nvt_dma  *nvtdmac = to_nvt_dma(nvtchan->chan.device);
	u32    reg;
	s64    period_ts;

	dev_dbg(chan2dev(&nvtchan->chan), "%s: desc 0x%p\n", __func__, first);

	/* Set transfer as active to not try to start it again. */
	first->active_xfer = true;

	if (nvt_dma_chan_is_linklist(nvtchan)) {
		dev_dbg(chan2dev(&nvtchan->chan), "%s: linklist mode\n", __func__);

		nvt_dma_chan_write(nvtchan, NVT_DMA_LL_SRC_ADDR, first->lld.mbr_ll_paddr & 0xFFFFFFFF);
		nvt_dma_chan_write(nvtchan, NVT_DMA_LL_SRC_ADDR_H, first->lld.mbr_ll_paddr >> 32);

		/* Fire */
		nvt_dma_write(nvtdmac, 4 * nvtchan->ch_id, NVT_DMA_CFG_LL);

	} else {

		nvt_dma_chan_write(nvtchan, NVT_DMA_SRC_ADDR, first->lld.mbr_sa);
		nvt_dma_chan_write(nvtchan, NVT_DMA_DEST_ADDR, first->lld.mbr_da);
		nvt_dma_chan_write(nvtchan, NVT_DMA_SRC_ADDR_H, first->lld.mbr_sa_h);
		nvt_dma_chan_write(nvtchan, NVT_DMA_DEST_ADDR_H, first->lld.mbr_da_h);
		nvt_dma_chan_write(nvtchan, NVT_DMA_LEN, first->lld.mbr_len);
		nvt_dma_chan_write(nvtchan, NVT_DMA_CONT, first->lld.mbr_value);
		reg = nvt_dma_chan_read(nvtchan, NVT_DMA_CFG);
		nvt_dma_chan_write(nvtchan, NVT_DMA_CFG, (reg & ~0x3) | first->lld.mbr_cfg);

		/* Trigger */
		nvt_dma_write(nvtdmac, 4 * nvtchan->ch_id, NVT_DMA_CH_TRIG);
	}

	period_ts = ktime_to_us(ktime_get());
	if (durationSum[nvtchan->ch_id] == 0)
			recordTime[nvtchan->ch_id] = period_ts;
	startTime[nvtchan->ch_id] = period_ts;

	dev_dbg(chan2dev(&nvtchan->chan),
			"%s: CFG=0x%08x LLAddr=0x%08x%08x\n",
			__func__, nvt_dma_chan_read(nvtchan, NVT_DMA_CFG),
			nvt_dma_chan_read(nvtchan, NVT_DMA_LL_SRC_ADDR_H),
			nvt_dma_chan_read(nvtchan, NVT_DMA_LL_SRC_ADDR));

}

static dma_cookie_t nvt_dma_tx_submit(struct dma_async_tx_descriptor *tx)
{
	struct nvt_dma_desc *desc = txd_to_at_desc(tx);
	struct nvt_dma_chan *nvtchan = to_nvt_dma_chan(tx->chan);
	dma_cookie_t        cookie;
	unsigned long       irqflags;

	spin_lock_irqsave(&nvtchan->lock, irqflags);
#ifdef CONFIG_PM
	if (nvtchan->pm_state == true) {
		dev_err(chan2dev(tx->chan), "cannot submit because system suspend\n");
		spin_unlock_irqrestore(&nvtchan->lock, irqflags);
		return -1;
	}
#endif
	cookie = dma_cookie_assign(tx);

	list_add_tail(&desc->xfer_node, &nvtchan->xfers_list);
	spin_unlock_irqrestore(&nvtchan->lock, irqflags);

	dev_dbg(chan2dev(tx->chan), "%s: nvtchan 0x%p, add desc 0x%p to xfers_list\n",
			__func__, nvtchan, desc);

	return cookie;
}

static struct nvt_dma_desc *nvt_dma_alloc_desc(struct dma_chan *chan,
		gfp_t gfp_flags)
{
	struct nvt_dma_desc *desc;
	struct nvt_dma      *nvtdmac = to_nvt_dma(chan->device);
	dma_addr_t      phys;

	desc = dma_pool_zalloc(nvtdmac->nvt_dma_desc_pool, gfp_flags, &phys);
	if (desc) {
		INIT_LIST_HEAD(&desc->descs_list);
		dma_async_tx_descriptor_init(&desc->tx_dma_desc, chan);
		desc->tx_dma_desc.tx_submit = nvt_dma_tx_submit;
		desc->tx_dma_desc.phys = phys;
	}

	return desc;
}

static void nvt_dma_init_used_desc(struct nvt_dma_desc *desc)
{
	memset(&desc->lld, 0, sizeof(desc->lld));
	INIT_LIST_HEAD(&desc->descs_list);
	desc->direction = DMA_TRANS_NONE;
	desc->xfer_size = 0;
	desc->active_xfer = false;
}

/* Call must be protected by lock. */
static struct nvt_dma_desc *nvt_dma_get_desc(struct nvt_dma_chan *nvtchan)
{
	struct nvt_dma_desc *desc;

	if (list_empty(&nvtchan->free_descs_list)) {
		desc = nvt_dma_alloc_desc(&nvtchan->chan, GFP_NOWAIT);
	} else {
		desc = list_first_entry(&nvtchan->free_descs_list, struct nvt_dma_desc, desc_node);
		list_del(&desc->desc_node);
		nvt_dma_init_used_desc(desc);
	}

	return desc;
}

static void nvt_dma_queue_desc(struct dma_chan *chan, struct nvt_dma_desc *prev, struct nvt_dma_desc *desc)
{
	if (!prev || !desc) {
		return;
	}

	prev->lld.mbr_nda = desc->tx_dma_desc.phys;

	dev_dbg(chan2dev(chan), "%s: chain lld: prev=0x%p, mbr_nda=%pad\n",
			__func__, prev, &prev->lld.mbr_nda);
}

static struct dma_chan *nvt_dma_xlate(struct of_phandle_args *dma_spec, struct of_dma *of_dma)
{
	struct nvt_dma      *nvtdmac = of_dma->of_dma_data;
	struct dma_chan     *chan;
	struct device       *dev = nvtdmac->dma.dev;
	unsigned int ch_id;

	if (dma_spec->args_count != 1) {
		dev_err(dev, "dma phandler args: bad number of args\n");
		return NULL;
	}

	ch_id = dma_spec->args[0];
	chan = dma_get_slave_channel(&nvtdmac->chan[ch_id].chan);
	if (!chan) {
		dev_err(dev, "can't get a dma channel id(%d)\n", dma_spec->args[0]);
		return NULL;
	}

	return chan;
}

/*
 * Only check that maxburst and addr width values are supported by the
 * the controller but not that the configuration is good to perform the
 * transfer since we don't know the direction at this stage.
 */
static int nvt_dma_check_slave_config(struct dma_slave_config *sconfig)
{
	if ((sconfig->src_maxburst > NVT_DMA_MAX_BURST_LENGTH)
		|| (sconfig->dst_maxburst > NVT_DMA_MAX_BURST_LENGTH)) {
		return -EINVAL;
	}

	if ((sconfig->src_addr_width > NVT_DMA_MAX_DWIDTH)
		|| (sconfig->dst_addr_width > NVT_DMA_MAX_DWIDTH)) {
		return -EINVAL;
	}

	return 0;
}

static int nvt_dma_set_slave_config(struct dma_chan *chan, struct dma_slave_config *sconfig)
{
	struct nvt_dma_chan *nvtchan = to_nvt_dma_chan(chan);

	if (nvt_dma_check_slave_config(sconfig)) {
		dev_err(chan2dev(chan), "invalid slave configuration\n");
		return -EINVAL;
	}

	memcpy(&nvtchan->sconfig, sconfig, sizeof(nvtchan->sconfig));

	return 0;
}

static struct dma_async_tx_descriptor *
nvt_dma_prep_dma_cyclic(struct dma_chan *chan, dma_addr_t buf_addr,
						size_t buf_len, size_t period_len,
						enum dma_transfer_direction direction,
						unsigned long flags)
{
	struct nvt_dma_chan *nvtchan = to_nvt_dma_chan(chan);
	struct nvt_dma      *nvtdmac = to_nvt_dma(nvtchan->chan.device);
	struct nvt_dma_desc *first = NULL, *prev = NULL, *desc = NULL;
	unsigned long       irqflags;
	u64 value, trans_value, tmp_vaddr, tmp_paddr;
	unsigned int        i, cmd_num, cmd_type, ch_offset, reg;

	dev_dbg(chan2dev(chan), "%s: buf_addr=%llx, buf_len=%zd, period_len=%zd, flags=0x%lx\n",
			__func__, buf_addr, buf_len, period_len, flags);

	if (unlikely(!buf_len)) {
		return NULL;
	}

	if (test_and_set_bit(NVT_DMA_CHAN_IS_LL, &nvtchan->status)) {
		dev_err(chan2dev(chan), "channel linklist currently used\n");
		return NULL;
	} else {
		/* set channel interrupt mode */
		spin_lock(&nvtdmac->lock);
		reg = readl(nvtdmac->base + NVT_DMA_IE);
		reg &= ~(0x1111 << nvtchan->ch_id);
		reg |= (0x110 << nvtchan->ch_id);
		nvt_dma_write(nvtdmac, NVT_DMA_IE, reg);
		spin_unlock(&nvtdmac->lock);
	}

	switch (direction) {
	case DMA_MEM_TO_DEV:
	case DMA_DEV_TO_MEM:
		dev_err(chan2dev(chan), "invalid DMA direction\n");
		return NULL;
		break;
	default:
		break;
	}

	spin_lock_irqsave(&nvtchan->lock, irqflags);
	desc = nvt_dma_get_desc(nvtchan);
	if (!desc) {
		dev_err(chan2dev(chan), "can't get descriptor\n");

		spin_unlock_irqrestore(&nvtchan->lock, irqflags);
		return NULL;
	}
	spin_unlock_irqrestore(&nvtchan->lock, irqflags);
	dev_dbg(chan2dev(chan),
			"%s: desc=0x%p, tx_dma_desc.phys=%pad, struct size %ld\n",
			__func__, desc, &desc->tx_dma_desc.phys, sizeof(struct cyclic_data));

	if (buf_len % (7 * 8)) {
		dev_err(chan2dev(chan), "buf len %d not multiples of 7\n", (unsigned int)buf_len);
		return NULL;
	}

	cmd_num = buf_len / (sizeof(struct cyclic_data));
	if (cmd_num > NVT_DMA_MAX_CMD) {
		dev_err(chan2dev(chan), "link list count %d too much, over %d\n", cmd_num, NVT_DMA_MAX_CMD);
		return NULL;
	}

	desc->lld.mbr_ll_paddr = (unsigned long)nvtchan->dma_paddr;
	desc->lld.mbr_ll_vaddr = (unsigned long)nvtchan->dma_vaddr;

	memcpy((void *)desc->lld.mbr_ll_vaddr, (void *)buf_addr, buf_len);
	dev_dbg(chan2dev(chan), "%s: done\n", __func__);

	tmp_paddr = desc->lld.mbr_ll_paddr;
	tmp_vaddr = desc->lld.mbr_ll_vaddr;

	ch_offset = (nvtchan->ch_id + 1) * 0x100;

	for (i = 0; i < cmd_num; i++) {

		tmp_paddr += sizeof(struct cyclic_data);

		value = *(u64 *)tmp_vaddr;
		cmd_type = value & 0xFFFFFFFF;

		trans_value = (BIT(31) | (0xF << 12) | (ch_offset + NVT_DMA_CFG)) << 32;
		trans_value |= value & 0xFFFFFFFF;
		*(u64 *)tmp_vaddr = trans_value;
		dev_dbg(chan2dev(chan), "mem%03d %08llx => %llx\n", i, value, trans_value);
		tmp_vaddr += 8;

		if (cmd_type == NVT_DMA_CFG_CPY) {

			value = *(u64 *)tmp_vaddr;
			trans_value = (BIT(31) | (0xF << 12) | (ch_offset + NVT_DMA_SRC_ADDR)) << 32;
			trans_value |= value & 0xFFFFFFFF;
			*(u64 *)tmp_vaddr = trans_value;
			dev_dbg(chan2dev(chan), "memcpy src %08llx => %llx\n", value, trans_value);
			tmp_vaddr += 8;

			value = *(u64 *)tmp_vaddr;
			trans_value = (BIT(31) | (0xF << 12) | (ch_offset + NVT_DMA_SRC_ADDR_H)) << 32;
			trans_value |= value & 0xFFFFFFFF;
			*(u64 *)tmp_vaddr = trans_value;
			dev_dbg(chan2dev(chan), "memcpy srh %08llx => %llx\n", value, trans_value);
			tmp_vaddr += 8;

			value = *(u64 *)tmp_vaddr;
			trans_value = (BIT(31) | (0xF << 12) | (ch_offset + NVT_DMA_DEST_ADDR)) << 32;
			trans_value |= value & 0xFFFFFFFF;
			*(u64 *)tmp_vaddr = trans_value;
			dev_dbg(chan2dev(chan), "memcpy dst %08llx => %llx\n", value, trans_value);
			tmp_vaddr += 8;

			value = *(u64 *)tmp_vaddr;
			trans_value = (BIT(31) | (0xF << 12) | (ch_offset + NVT_DMA_DEST_ADDR_H)) << 32;
			trans_value |= value & 0xFFFFFFFF;
			*(u64 *)tmp_vaddr = trans_value;
			dev_dbg(chan2dev(chan), "memcpy dsh %08llx => %llx\n", value, trans_value);
			tmp_vaddr += 8;

			value = *(u64 *)tmp_vaddr;
			trans_value = (BIT(31) | (0xF << 12) | (ch_offset + NVT_DMA_LEN)) << 32;
			trans_value |= value & 0xFFFFFFFF;
			*(u64 *)tmp_vaddr = trans_value;
			dev_dbg(chan2dev(chan), "memcpy len %08llx => %llx\n", value, trans_value);
			tmp_vaddr += 8;

			/* NEXT or NULL */
			value = *(u64 *)tmp_vaddr;
			if ((i + 1) < cmd_num) {
				trans_value = BIT(29) << 32;
				trans_value |= ((tmp_paddr << 8) | i);
				*(u64 *)tmp_vaddr = trans_value;
				dev_dbg(chan2dev(chan), "memcpy nxt %08llx\n", trans_value);
				tmp_vaddr += 8;
			} else {
				*(u64 *)tmp_vaddr = 0 | i;
				dev_dbg(chan2dev(chan), "memcpy NULL\n");
			}

		} else {
			/* length update this addr */
			value = *(u64 *)(tmp_vaddr + 32);
			trans_value = (BIT(31) | (0xF << 12) | (ch_offset + NVT_DMA_LEN)) << 32;
			trans_value |= value & 0xFFFFFFFF;
			*(u64 *)tmp_vaddr = trans_value;
			dev_dbg(chan2dev(chan), "memset len %08llx => %llx\n", value, trans_value);
			tmp_vaddr += 8;

			/* value update this addr */
			value = *(u64 *)(tmp_vaddr + 32);
			value = value | (value << 8) | (value << 16) | (value << 24);
			trans_value = (BIT(31) | (0xF << 12) | (ch_offset + NVT_DMA_CONT)) << 32;
			trans_value |= value & 0xFFFFFFFF;
			*(u64 *)tmp_vaddr = trans_value;
			dev_dbg(chan2dev(chan), "memset val %08llx => %llx\n", value, trans_value);
			tmp_vaddr += 8;

			value = *(u64 *)tmp_vaddr;
			trans_value = (BIT(31) | (0xF << 12) | (ch_offset + NVT_DMA_DEST_ADDR)) << 32;
			trans_value |= value & 0xFFFFFFFF;
			*(u64 *)tmp_vaddr = trans_value;
			dev_dbg(chan2dev(chan), "memset dst %08llx => %llx\n", value, trans_value);
			tmp_vaddr += 8;

			value = *(u64 *)tmp_vaddr;
			trans_value = (BIT(31) | (0xF << 12) | (ch_offset + NVT_DMA_DEST_ADDR_H)) << 32;
			trans_value |= value & 0xFFFFFFFF;
			*(u64 *)tmp_vaddr = trans_value;
			dev_dbg(chan2dev(chan), "memset dsh %08llx => %llx\n", value, trans_value);
			tmp_vaddr += 8;

			/* NEXT or NULL */
			value = *(u64 *)tmp_vaddr;
			if ((i + 1) < cmd_num) {
				trans_value = BIT(29) << 32;
				trans_value |= ((tmp_paddr << 8) | i);
				*(u64 *)tmp_vaddr = trans_value;
				dev_dbg(chan2dev(chan), "memset nxt %08llx\n", trans_value);
				tmp_vaddr += 16;
			} else {
				*(u64 *)tmp_vaddr = 0 | i;
				dev_dbg(chan2dev(chan), "memset NULL\n");
			}
		}

	}

	prev = desc;
	if (!first) {
		first = desc;
	}

	dev_dbg(chan2dev(chan), "%s: add desc 0x%p to descs_list 0x%p\n",
			__func__, desc, first);
	list_add_tail(&desc->desc_node, &first->descs_list);

	nvt_dma_queue_desc(chan, prev, first);
	first->tx_dma_desc.flags = flags;
	first->xfer_size = buf_len;
	first->direction = direction;

	return &first->tx_dma_desc;
}

static struct dma_async_tx_descriptor *
nvt_dma_prep_dma_memcpy(struct dma_chan *chan, dma_addr_t dest, dma_addr_t src,
						size_t len, unsigned long flags)
{
	struct nvt_dma_chan *nvtchan = to_nvt_dma_chan(chan);
	struct nvt_dma      *nvtdmac = to_nvt_dma(nvtchan->chan.device);
	struct nvt_dma_desc *first = NULL, *prev = NULL;
	size_t          remaining_size = len, xfer_size = 0;
	dma_addr_t      src_addr = src, dst_addr = dest;
	unsigned long   irqflags;
	unsigned int    reg;

	dev_dbg(chan2dev(chan), "%s: src=%pad, dest=%pad, len=0x%lx, flags=0x%lx\n",
			__func__, &src, &dest, len, flags);

	if (unlikely(!len)) {
		return NULL;
	}

	/* set channel interrupt mode */
	spin_lock(&nvtdmac->lock);
	reg = readl(nvtdmac->base + NVT_DMA_IE);
	reg &= ~(0x1111 << nvtchan->ch_id);
	reg |= (0x1 << nvtchan->ch_id);
	nvt_dma_write(nvtdmac, NVT_DMA_IE, reg);
	spin_unlock(&nvtdmac->lock);

	/* Prepare descriptors. */
	while (remaining_size) {
		struct nvt_dma_desc *desc = NULL;

		dev_dbg(chan2dev(chan), "%s: remaining_size=%zu\n", __func__, remaining_size);

		spin_lock_irqsave(&nvtchan->lock, irqflags);
		desc = nvt_dma_get_desc(nvtchan);
		//coverity[double_unlock]: only unlock once
		spin_unlock_irqrestore(&nvtchan->lock, irqflags);
		if (!desc) {
			dev_err(chan2dev(chan), "can't get descriptor\n");
			if (first) {
				list_splice_init(&first->descs_list, &nvtchan->free_descs_list);
			}
			return NULL;
		}

		/* Update src and dest addresses. */
		src_addr += xfer_size;
		dst_addr += xfer_size;

		if (remaining_size >= NVT_DMA_LEN_MAX) {
			xfer_size = NVT_DMA_LEN_MAX;
		} else {
			xfer_size = remaining_size;
		}

		dev_dbg(chan2dev(chan), "%s: xfer_size=%zu\n", __func__, xfer_size);

		remaining_size -= xfer_size;

		desc->lld.mbr_sa = src_addr & 0xFFFFFFFF;
		desc->lld.mbr_da = dst_addr & 0xFFFFFFFF;

#ifdef CONFIG_ARCH_DMA_ADDR_T_64BIT
		desc->lld.mbr_sa_h = src_addr >> 32;
		desc->lld.mbr_da_h = dst_addr >> 32;
#else
		desc->lld.mbr_sa_h = 0;
		desc->lld.mbr_da_h = 0;
#endif
		desc->lld.mbr_len = len;
		desc->lld.mbr_cfg = NVT_DMA_CFG_CPY;
		desc->lld.mbr_cmd = 0;

		dev_dbg(chan2dev(chan),
				"%s: cpy: sa=0x%08x%08x, da=0x%08x%08x, len=0x%08x, cfg=0x%08x\n",
				__func__, desc->lld.mbr_sa_h, desc->lld.mbr_sa, desc->lld.mbr_da_h, desc->lld.mbr_da,
				desc->lld.mbr_len, desc->lld.mbr_cfg);

		/* Chain lld. */
		if (prev) {
			nvt_dma_queue_desc(chan, prev, desc);
		}

		prev = desc;
		if (!first) {
			first = desc;
		}

		dev_dbg(chan2dev(chan), "%s: add desc 0x%p to descs_list 0x%p\n",
				__func__, desc, first);
		list_add_tail(&desc->desc_node, &first->descs_list);
	}

	first->tx_dma_desc.flags = flags;
	first->xfer_size = len;

	return &first->tx_dma_desc;
}

static struct nvt_dma_desc *nvt_dma_memset_create_desc(struct dma_chan *chan,
		struct nvt_dma_chan *nvtchan,
		dma_addr_t dst_addr,
		size_t len,
		int value)
{
	struct nvt_dma_desc *desc;
	unsigned long       flags;
	u32                 constant;

	if (len >= NVT_DMA_LEN_MAX) {
		dev_err(chan2dev(chan),
				"%s: Transfer too large, aborting...\n",
				__func__);
		return NULL;
	}

	spin_lock_irqsave(&nvtchan->lock, flags);
	desc = nvt_dma_get_desc(nvtchan);
	spin_unlock_irqrestore(&nvtchan->lock, flags);
	if (!desc) {
		dev_err(chan2dev(chan), "can't get descriptor\n");
		return NULL;
	}

	desc->lld.mbr_da = dst_addr & 0xFFFFFFFF;
#ifdef CONFIG_ARCH_DMA_ADDR_T_64BIT
	desc->lld.mbr_da_h = dst_addr >> 32;
#else
	desc->lld.mbr_da_h = 0;
#endif
	constant = value | (value << 8) | (value << 16) | (value << 24);

	desc->lld.mbr_value = constant;
	desc->lld.mbr_len = len;
	desc->lld.mbr_cfg = NVT_DMA_CFG_SET;
	desc->lld.mbr_cmd = 0;

	dev_dbg(chan2dev(chan),
			"%s: lld: da=0x%08x%08x, value=0x%08x, len=0x%08x, cfg=0x%08x\n",
			__func__, desc->lld.mbr_da_h, desc->lld.mbr_da, desc->lld.mbr_value, desc->lld.mbr_len,
			desc->lld.mbr_cfg);

	return desc;
}

static struct dma_async_tx_descriptor *
nvt_dma_prep_dma_memset(struct dma_chan *chan, dma_addr_t dest, int value, size_t len, unsigned long flags)
{
	struct nvt_dma_chan *nvtchan = to_nvt_dma_chan(chan);
	struct nvt_dma      *nvtdmac = to_nvt_dma(nvtchan->chan.device);
	struct nvt_dma_desc *desc;
	unsigned int    reg;

	dev_dbg(chan2dev(chan), "%s: dest=%pad, len=0x%lx, pattern=0x%x, flags=0x%lx\n",
			__func__, &dest, len, value, flags);

	if (unlikely(!len)) {
		return NULL;
	}

	/* set channel interrupt mode */
	spin_lock(&nvtdmac->lock);
	reg = readl(nvtdmac->base + NVT_DMA_IE);
	reg &= ~(0x1111 << nvtchan->ch_id);
	reg |= (0x1 << nvtchan->ch_id);
	nvt_dma_write(nvtdmac, NVT_DMA_IE, reg);
	spin_unlock(&nvtdmac->lock);

	desc = nvt_dma_memset_create_desc(chan, nvtchan, dest, len, value);
	if (!desc) {
		return NULL;
	}
	list_add_tail(&desc->desc_node, &desc->descs_list);

	desc->tx_dma_desc.cookie = -EBUSY;
	desc->tx_dma_desc.flags = flags;
	desc->xfer_size = len;

	return &desc->tx_dma_desc;
}

static enum dma_status
nvt_dma_tx_status(struct dma_chan *chan, dma_cookie_t cookie,
				  struct dma_tx_state *txstate) {
	return dma_cookie_status(chan, cookie, txstate);
}

/* Call must be protected by lock. */
static void nvt_dma_remove_xfer(struct nvt_dma_chan *nvtchan, struct nvt_dma_desc *desc)
{
	dev_dbg(chan2dev(&nvtchan->chan), "%s: desc 0x%p\n", __func__, desc);

	/*
	 * Remove the transfer from the transfer list then move the transfer
	 * descriptors into the free descriptors list.
	 */
	list_del(&desc->xfer_node);
	list_splice_init(&desc->descs_list, &nvtchan->free_descs_list);
}

static void nvt_dma_advance_work(struct nvt_dma_chan *nvtchan)
{
	struct nvt_dma_desc *desc;

	/*
	 * If channel is enabled, do nothing, advance_work will be triggered
	 * after the interruption.
	 */
	if (!nvt_dma_chan_is_enabled(nvtchan) && !list_empty(&nvtchan->xfers_list)) {
		desc = list_first_entry(&nvtchan->xfers_list,
								struct nvt_dma_desc,
								xfer_node);
		dev_dbg(chan2dev(&nvtchan->chan), "%s: desc 0x%p\n", __func__, desc);
		if (!desc->active_xfer) {
			nvt_dma_start_xfer(nvtchan, desc);
		} else {
			dev_err(chan2dev(&nvtchan->chan), "%s: not ready\n", __func__);
		}
	}
}

/* Schedule completion tasklet */
static void nvt_dma_tasklet(struct tasklet_struct *t)
{
	struct nvt_dma_chan *nvtchan = from_tasklet(nvtchan, t, tasklet);
	struct nvt_dma_desc *desc;
	struct dma_async_tx_descriptor  *txd;

	dev_dbg(chan2dev(&nvtchan->chan), "%s: status=0x%08x\n",
			__func__, nvtchan->irq_status);

	spin_lock_irq(&nvtchan->lock);
	desc = list_first_entry(&nvtchan->xfers_list,
							struct nvt_dma_desc,
							xfer_node);
	dev_dbg(chan2dev(&nvtchan->chan), "%s: desc 0x%p\n", __func__, desc);
	if (!desc->active_xfer) {
		dev_err(chan2dev(&nvtchan->chan), "Xfer not active: exiting");
		spin_unlock_irq(&nvtchan->lock);
		return;
	}

	txd = &desc->tx_dma_desc;

	desc->active_xfer = false;
	dma_cookie_complete(txd);

	if (txd->flags & DMA_PREP_INTERRUPT) {
		dmaengine_desc_get_callback_invoke(txd, NULL);
	}

	dma_run_dependencies(txd);
	nvt_dma_remove_xfer(nvtchan, desc);

	nvt_dma_advance_work(nvtchan);
	spin_unlock_irq(&nvtchan->lock);
}

static irqreturn_t nvt_dma_interrupt(int irq, void *dev_id)
{
	struct nvt_dma      *nvtdmac = (struct nvt_dma *)dev_id;
	struct nvt_dma_chan *nvtchan;
	u32         status, pending;
	u32         chan_status;
	int         i, ret = IRQ_NONE;

	//printk("<I>");

	do {
		status = nvt_dma_read(nvtdmac, NVT_DMA_STS);
		pending = status;
		//printk("<0x%x>", status);

		dev_dbg(nvtdmac->dma.dev,
				"%s: status=0x%08x, pending=0x%08x, chancnt=%d\n",
				__func__, status, pending, nvtdmac->dma.chancnt);

		if (!pending) {
			break;
		}

		/* We have to find which channel has generated the interrupt. */
		for (i = 0; i < NVT_DMA_MAX_CHAN; i++) {

			nvtchan = &nvtdmac->chan[i];
			if (nvt_dma_chan_is_linklist(nvtchan)) {
				if (!((0x110 << i) & pending)) {
					continue;
				}
			} else {
				if (!((0x1 << i) & pending)) {
					continue;
				}
			}

			chan_status = nvt_dma_read(nvtdmac, NVT_DMA_STS);

			if ((0x1 << (NVT_DMA_INT_LLERR + i)) & chan_status) {
				printk("channel %d Link List command is invalid\n", i);
			}

			nvtchan->irq_status = chan_status;
			dev_dbg(nvtdmac->dma.dev,
					"%s: chan%d: status=0x%x\n",
					__func__, i, chan_status);

			/* Write 1 clear */
			if (nvt_dma_chan_is_linklist(nvtchan)) {
				nvt_dma_write(nvtdmac, NVT_DMA_STS, 0x1111 << nvtchan->ch_id);

				clear_bit(NVT_DMA_CHAN_IS_LL, &nvtchan->status);
			} else {
				nvt_dma_write(nvtdmac, NVT_DMA_STS, 1 << nvtchan->ch_id);
			}
			endTime[nvtchan->ch_id] = ktime_to_us(ktime_get());
			calc_time(nvtdmac->dma.dev, nvtchan->ch_id);

			tasklet_schedule(&nvtchan->tasklet);
			ret = IRQ_HANDLED;
		}

	} while (pending);

	dev_dbg(nvtdmac->dma.dev, "%s finish\n", __func__);

	return ret;
}

static void nvt_dma_issue_pending(struct dma_chan *chan)
{
	struct nvt_dma_chan *nvtchan = to_nvt_dma_chan(chan);
	unsigned long flags;

	dev_dbg(chan2dev(&nvtchan->chan), "%s\n", __func__);

	spin_lock_irqsave(&nvtchan->lock, flags);
	nvt_dma_advance_work(nvtchan);
	spin_unlock_irqrestore(&nvtchan->lock, flags);

	return;
}

static int nvt_dma_device_config(struct dma_chan *chan, struct dma_slave_config *config)
{
	struct nvt_dma_chan *nvtchan = to_nvt_dma_chan(chan);
	int ret;
	unsigned long       flags;

	dev_dbg(chan2dev(chan), "%s\n", __func__);

	spin_lock_irqsave(&nvtchan->lock, flags);
	ret = nvt_dma_set_slave_config(chan, config);
	spin_unlock_irqrestore(&nvtchan->lock, flags);

	return ret;
}

static int nvt_dma_device_terminate_all(struct dma_chan *chan)
{
	struct nvt_dma_desc *desc, *_desc;
	struct nvt_dma_chan *nvtchan = to_nvt_dma_chan(chan);
	unsigned long       flags;

	dev_dbg(chan2dev(chan), "%s\n", __func__);

	spin_lock_irqsave(&nvtchan->lock, flags);

	/* Cancel all pending transfers. */
	list_for_each_entry_safe(desc, _desc, &nvtchan->xfers_list, xfer_node)
	nvt_dma_remove_xfer(nvtchan, desc);

	spin_unlock_irqrestore(&nvtchan->lock, flags);

	return 0;
}

static int nvt_dma_alloc_chan_resources(struct dma_chan *chan)
{
	struct nvt_dma_chan *nvtchan = to_nvt_dma_chan(chan);
	struct nvt_dma_desc *desc;
	int         i;

	dev_dbg(chan2dev(chan), "%s: alloc ch %d descriptor\n", __func__, nvtchan->ch_id);

	if (nvt_dma_chan_is_enabled(nvtchan)) {
		dev_err(chan2dev(chan),
				"can't allocate channel resources (channel enabled)\n");
		return -EIO;
	}

	if (test_and_set_bit(NVT_DMA_CHAN_IS_ENABLE, &nvtchan->status)) {
		dev_err(chan2dev(chan),
				"can't set channel enable\n");
		return -EIO;
	}

	if (!list_empty(&nvtchan->free_descs_list)) {
		dev_err(chan2dev(chan),
				"can't allocate channel resources (channel not free from a previous use)\n");
		return -EIO;
	}

	for (i = 0; i < init_nr_desc_per_channel; i++) {
		desc = nvt_dma_alloc_desc(chan, GFP_KERNEL);
		if (!desc) {
			dev_warn(chan2dev(chan),
					 "only %d descriptors have been allocated\n", i);
			break;
		}
		list_add_tail(&desc->desc_node, &nvtchan->free_descs_list);
	}

	dma_cookie_init(chan);

	dev_dbg(chan2dev(chan), "%s: allocated %d descriptors\n", __func__, i);

	return i;
}

static void nvt_dma_free_chan_resources(struct dma_chan *chan)
{
	struct nvt_dma_chan *nvtchan = to_nvt_dma_chan(chan);
	struct nvt_dma      *nvtdmac = to_nvt_dma(chan->device);
	struct nvt_dma_desc *desc, *_desc;

	dev_dbg(chan2dev(chan), "%s: freeing ch %d descriptor\n", __func__, nvtchan->ch_id);

	list_for_each_entry_safe(desc, _desc, &nvtchan->free_descs_list, desc_node) {
		//dev_dbg(chan2dev(chan), "%s: freeing descriptor %p\n", __func__, desc);
		list_del(&desc->desc_node);
		dma_pool_free(nvtdmac->nvt_dma_desc_pool, desc, desc->tx_dma_desc.phys);
	}
	clear_bit(NVT_DMA_CHAN_IS_ENABLE, &nvtchan->status);

	if (test_bit(NVT_DMA_CHAN_IS_LL, &nvtchan->status)) {
		clear_bit(NVT_DMA_CHAN_IS_LL, &nvtchan->status);
	}

	return;
}

#ifdef CONFIG_PM
static int nvt_dma_prepare(struct device *dev)
{
	struct nvt_dma      *nvtdmac = dev_get_drvdata(dev);
	struct dma_chan     *chan, *_chan;
	struct nvt_dma_chan *nvtchan;
	int i = 0;

	for (i = 0; i < NVT_DMA_MAX_CHAN; i++) {
		nvtchan = &nvtdmac->chan[i];
		nvtchan->pm_state = true;
	}

	list_for_each_entry_safe(chan, _chan, &nvtdmac->dma.channels, device_node) {
		struct nvt_dma_chan *nvtchan = to_nvt_dma_chan(chan);

		/* Wait for transfer completion */
		if (nvt_dma_chan_is_enabled(nvtchan)) {
			return -EAGAIN;
		}
	}
	return 0;
}
#else
#   define nvt_dma_prepare NULL
#endif

#ifdef CONFIG_PM_SLEEP
static int nvt_dma_suspend(struct device *dev)
{
	struct nvt_dma      *nvtdmac = dev_get_drvdata(dev);

	clk_disable_unprepare(nvtdmac->clk);

	return 0;
}

static int nvt_dma_resume(struct device *dev)
{
	struct nvt_dma      *nvtdmac = dev_get_drvdata(dev);
	struct nvt_dma_chan *nvtchan;
	unsigned int value;
	int ret, i = 0;

	ret = clk_prepare_enable(nvtdmac->clk);
	if (ret) {
		dev_err(dev, "can't prepare or enable clock\n");
		return ret;
	}

	/* Disable all chans and interrupts. */
	nvt_dma_off(nvtdmac);

	/* Clear pending interrupts. */
	nvt_dma_write(nvtdmac, NVT_DMA_STS, NVT_DMA_IE_EN);

	/* DMA enable */
	value = readl(nvtdmac->base + NVT_DMA_GENERAL);
	value &= ~BIT(1);
	writel(value, nvtdmac->base + NVT_DMA_GENERAL);

	for (i = 0; i < NVT_DMA_MAX_CHAN; i++) {
		nvtchan = &nvtdmac->chan[i];
		nvtchan->pm_state = false;
	}

	return 0;
}
#endif /* CONFIG_PM_SLEEP */

static int nvt_dma_probe(struct platform_device *pdev)
{
	struct nvt_dma  *nvtdmac;
	int     irq, size, i, ret;
	void __iomem    *base;
	unsigned int value;

	irq = platform_get_irq(pdev, 0);
	if (irq < 0) {
		dev_err(&pdev->dev, "get irq fail\n");
		return irq;
	}

	base = devm_platform_ioremap_resource(pdev, 0);
	if (IS_ERR(base)) {
		dev_err(&pdev->dev, "ioremap resource fail\n");
		return PTR_ERR(base);
	}

	size = sizeof(*nvtdmac);
	size += NVT_DMA_MAX_CHAN * sizeof(struct nvt_dma_chan);
	nvtdmac = devm_kzalloc(&pdev->dev, size, GFP_KERNEL);
	if (!nvtdmac) {
		dev_err(&pdev->dev, "can't allocate nvt_dma structure\n");
		return -ENOMEM;
	}

	nvtdmac->base = base;
	nvtdmac->irq = irq;

	/* Disable all chans and interrupts. */
	nvt_dma_off(nvtdmac);

	/* Clear pending interrupts. */
	nvt_dma_write(nvtdmac, NVT_DMA_STS, NVT_DMA_IE_EN);

	/* Do not use dev res to prevent races with tasklet */
	ret = request_irq(nvtdmac->irq, nvt_dma_interrupt, 0, "nvt_dma", nvtdmac);
	if (ret) {
		dev_err(&pdev->dev, "can't request irq\n");
		return ret;
	}

#ifdef CONFIG_OF
	dev_info(&pdev->dev, "%s, clk_name = %s, DRV_VERSION = %s\n", __func__, dev_name(&pdev->dev), DRV_VERSION);
	nvtdmac->clk = clk_get(&pdev->dev, dev_name(&pdev->dev));
	if (IS_ERR(nvtdmac->clk)) {
		dev_err(&pdev->dev, "can't find clock %s\n", dev_name(&pdev->dev));
		goto err_free_irq;
	} else {
		ret = clk_prepare_enable(nvtdmac->clk);
		if (ret) {
			dev_err(&pdev->dev, "can't prepare or enable clock\n");
			goto err_free_irq;
		}
	}
#endif

	nvtdmac->nvt_dma_desc_pool =
		dmam_pool_create(dev_name(&pdev->dev), &pdev->dev,
						 sizeof(struct nvt_dma_desc), 4, 0);
	if (!nvtdmac->nvt_dma_desc_pool) {
		dev_err(&pdev->dev, "no memory for descriptors dma pool\n");
		ret = -ENOMEM;
		goto err_clk_disable;
	}

	dma_cap_set(DMA_CYCLIC, nvtdmac->dma.cap_mask);
	dma_cap_set(DMA_MEMCPY, nvtdmac->dma.cap_mask);
	dma_cap_set(DMA_MEMSET, nvtdmac->dma.cap_mask);

	nvtdmac->dma.dev                = &pdev->dev;
	nvtdmac->dma.device_alloc_chan_resources    = nvt_dma_alloc_chan_resources;
	nvtdmac->dma.device_free_chan_resources     = nvt_dma_free_chan_resources;
	nvtdmac->dma.device_tx_status           = nvt_dma_tx_status;
	nvtdmac->dma.device_issue_pending       = nvt_dma_issue_pending;
	nvtdmac->dma.device_prep_dma_cyclic     = nvt_dma_prep_dma_cyclic;
	nvtdmac->dma.device_prep_dma_memcpy     = nvt_dma_prep_dma_memcpy;
	nvtdmac->dma.device_prep_dma_memset     = nvt_dma_prep_dma_memset;
	nvtdmac->dma.device_config          = nvt_dma_device_config;
	nvtdmac->dma.device_terminate_all       = nvt_dma_device_terminate_all;
	nvtdmac->dma.src_addr_widths = NVT_DMA_DMA_BUSWIDTHS;
	nvtdmac->dma.dst_addr_widths = NVT_DMA_DMA_BUSWIDTHS;
	nvtdmac->dma.directions = BIT(DMA_MEM_TO_MEM);
	nvtdmac->dma.residue_granularity = DMA_RESIDUE_GRANULARITY_BURST;

	pdev->dev.coherent_dma_mask = DMA_BIT_MASK(36);

	/* Init channels. */
	INIT_LIST_HEAD(&nvtdmac->dma.channels);
	for (i = 0; i < NVT_DMA_MAX_CHAN; i++) {
		struct nvt_dma_chan *nvtchan = &nvtdmac->chan[i];

		nvtchan->chan.device = &nvtdmac->dma;
		list_add_tail(&nvtchan->chan.device_node,
					  &nvtdmac->dma.channels);

		nvtchan->ch_regs = nvt_dma_chan_reg_base(nvtdmac, i);
		nvtchan->ch_id = i;
#ifdef CONFIG_PM
		nvtchan->pm_state = false;
#endif
		nvtchan->dma_vaddr = dma_alloc_coherent(&pdev->dev, NVT_DMA_MAX_CMD * sizeof(struct cyclic_data), &nvtchan->dma_paddr, GFP_KERNEL | GFP_DMA32);
		if (!nvtchan->dma_vaddr) {
			dev_err(&pdev->dev, "dma alloc fail\n");
			goto err_pool_destroy;
		}
		// coverity[side_effect_free]
		spin_lock_init(&nvtchan->lock);
		INIT_LIST_HEAD(&nvtchan->xfers_list);
		INIT_LIST_HEAD(&nvtchan->free_descs_list);
		tasklet_setup(&nvtchan->tasklet, nvt_dma_tasklet);
	}
	// coverity[side_effect_free]
	spin_lock_init(&nvtdmac->lock);

	/* DMA enable */
	value = readl(nvtdmac->base + NVT_DMA_GENERAL);
	value &= ~BIT(1);
	writel(value, nvtdmac->base + NVT_DMA_GENERAL);

	platform_set_drvdata(pdev, nvtdmac);

	ret = dma_async_device_register(&nvtdmac->dma);
	if (ret) {
		dev_err(&pdev->dev, "fail to register DMA engine device\n");
		goto err_free_coherent;
	}

	ret = of_dma_controller_register(pdev->dev.of_node,
									 nvt_dma_xlate, nvtdmac);
	if (ret) {
		dev_err(&pdev->dev, "could not register of dma controller\n");
		goto err_dma_unregister;
	}

	dev_info(&pdev->dev, "%d channels, mapped at 0x%p\n",
			 NVT_DMA_MAX_CHAN, nvtdmac->base);

	return 0;

err_dma_unregister:
	dma_async_device_unregister(&nvtdmac->dma);
err_free_coherent:
	for (i = 0; i < NVT_DMA_MAX_CHAN; i++) {
		struct nvt_dma_chan *nvtchan = &nvtdmac->chan[i];

		dma_free_coherent(&pdev->dev, NVT_DMA_MAX_CMD * sizeof(struct cyclic_data), nvtchan->dma_vaddr, nvtchan->dma_paddr);
	}
err_pool_destroy:
	dmam_pool_destroy(nvtdmac->nvt_dma_desc_pool);
err_clk_disable:
	clk_disable_unprepare(nvtdmac->clk);
	clk_put(nvtdmac->clk);
err_free_irq:
	free_irq(nvtdmac->irq, nvtdmac);
	return ret;
}

static int nvt_dma_remove(struct platform_device *pdev)
{
	struct nvt_dma  *nvtdmac = (struct nvt_dma *)platform_get_drvdata(pdev);
	int     i;

	if (nvtdmac->clk) {
		clk_disable_unprepare(nvtdmac->clk);
	}

	clk_put(nvtdmac->clk);

	nvt_dma_off(nvtdmac);
	of_dma_controller_free(pdev->dev.of_node);
	dmam_pool_destroy(nvtdmac->nvt_dma_desc_pool);
	dma_async_device_unregister(&nvtdmac->dma);

	free_irq(nvtdmac->irq, nvtdmac);

	for (i = 0; i < nvtdmac->dma.chancnt; i++) {
		struct nvt_dma_chan *nvtchan = &nvtdmac->chan[i];

		tasklet_kill(&nvtchan->tasklet);
		nvt_dma_free_chan_resources(&nvtchan->chan);
		dma_free_coherent(&pdev->dev, NVT_DMA_MAX_CMD * sizeof(struct cyclic_data), nvtchan->dma_vaddr, nvtchan->dma_paddr);
	}

	return 0;
}

#ifdef CONFIG_PM_SLEEP
static const struct dev_pm_ops nvt_dma_dev_pm_ops = {
	.prepare    = nvt_dma_prepare,
	SET_LATE_SYSTEM_SLEEP_PM_OPS(nvt_dma_suspend, nvt_dma_resume)
};
#endif

static int nvt_dma_proc_utilization_show(struct seq_file *sfile, void *v)
{
	int i = 0;
	int sum = 0;

	seq_printf(sfile, "======== HWCOPY =============\n");
	seq_printf(sfile, "chip eng usage    fps\n");
	for (i = 0; i < NVT_DMA_MAX_CHAN; i++)
		sum += utility[i];

	seq_printf(sfile, "%4d %3d %5d %6d\n", 0, 0, sum/NVT_DMA_MAX_CHAN, 0);

	for (i = 0; i < NVT_DMA_MAX_CHAN; i++) {
		utility[i] = 0;
		durationSum[i] = 0;
	}

	return 0;
}

static int nvt_dma_proc_utilization_open(struct inode *inode, struct file *file)
{
	return single_open(file, nvt_dma_proc_utilization_show, NULL);
}

static struct proc_ops proc_utilization_fops = {
	.proc_open    = nvt_dma_proc_utilization_open,
	.proc_release = single_release,
	.proc_read    = seq_read,
	.proc_lseek   = seq_lseek,
};

static int nvt_dma_proc_init(void)
{
	int ret = 0;

	pmodule_root = proc_mkdir("kdrv_hwcopy", NULL);
	if (pmodule_root == NULL) {
		pr_err("failed to create kdrv_hwcopy\n");
		ret = -EINVAL;
		goto remove_proc;
	}

	pentry = proc_create("utilization", S_IRUGO | S_IXUGO, pmodule_root, &proc_utilization_fops);
	if (pentry == NULL) {
		pr_err("failed to create proc hwcopy utilization!\n");
		ret = -EINVAL;
		goto remove_proc;
	}

remove_proc:
	return ret;
}

static const struct of_device_id nvt_dma_dt_ids[] = {
	{ .compatible = "nvt,nvt_hwcopy" },
	{},
};
MODULE_DEVICE_TABLE(of, nvt_dma_dt_ids);

static struct platform_driver nvt_dma_driver = {
	.probe      = nvt_dma_probe,
	.remove     = nvt_dma_remove,
	.driver = {
		.name       = "nvt_dma",
		.of_match_table = of_match_ptr(nvt_dma_dt_ids),
#ifdef CONFIG_PM_SLEEP
		.pm     = &nvt_dma_dev_pm_ops,
#endif
	}
};

static int __init nvt_dma_init(void)
{
	nvt_dma_proc_init();
	return platform_driver_register(&nvt_dma_driver);
}
subsys_initcall(nvt_dma_init);

static void __exit nvt_dma_exit(void)
{
	platform_driver_unregister(&nvt_dma_driver);
	proc_remove(pentry);
	proc_remove(pmodule_root);
}
module_exit(nvt_dma_exit);

MODULE_DESCRIPTION("Novatek DMA Controller driver");
MODULE_VERSION(DRV_VERSION);
MODULE_LICENSE("GPL");
