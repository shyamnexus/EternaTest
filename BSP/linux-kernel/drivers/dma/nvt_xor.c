// SPDX-License-Identifier: GPL-2.0-only
/*
 * offload engine driver for the Novatek XOR engine
 */
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/dma-mapping.h>
#include <linux/spinlock.h>
#include <linux/interrupt.h>
#include <linux/of_device.h>
#include <linux/platform_device.h>
#include <linux/memory.h>
#include <linux/clk.h>
#include <linux/of.h>
#include <linux/of_irq.h>
#include <linux/irqdomain.h>
#include <linux/cpumask.h>
#include <linux/module.h>
#include <linux/platform_data/dma-nvt_xor.h>

#include "dmaengine.h"
#include "nvt_xor.h"

#define XOR_MODULE_VER  "1.0.5"

static int __xor_debug = 0;
module_param_named(xor_debug, __xor_debug, int, 0600);

static int __xor_desc_debug = 0;
module_param_named(xor_desc_debug, __xor_desc_debug, int, 0600);

static int __xor_isr_debug = 0;
module_param_named(xor_isr_debug, __xor_isr_debug, int, 0600);

static int __xor_flow_debug = 0;
module_param_named(xor_flow_debug, __xor_flow_debug, int, 0600);

#define xor_debug(fmt, ...) do { \
	if (__xor_debug) \
		printk(KERN_DEBUG "xor: %s: "fmt, __func__, ##__VA_ARGS__); \
} while (0)

#define xor_desc_debug(fmt, ...) do { \
	if (__xor_desc_debug) \
		printk(KERN_DEBUG "xor: %s: "fmt, __func__, ##__VA_ARGS__); \
} while (0)

#define xor_isr_debug(fmt, ...) do { \
	if (__xor_isr_debug) \
		printk(KERN_DEBUG "xor: %s: "fmt, __func__, ##__VA_ARGS__); \
} while (0)

#define xor_flow_debug(fmt, ...) do { \
	if (__xor_flow_debug) \
		pr_info("nvt_xor: <%s>\n"fmt, __func__, ##__VA_ARGS__); \
} while (0)

static void nvt_xor_issue_pending(struct dma_chan *chan);
static char nvt_chan_is_busy(struct nvt_xor_chan *nvt_chan);

#define to_nvt_xor_chan(chan)       \
	container_of(chan, struct nvt_xor_chan, dmachan)

#define to_nvt_xor_slot(tx)     \
	container_of(tx, struct nvt_xor_desc_slot, async_tx)

#define nvt_chan_to_devp(nvt_chan)           \
	((nvt_chan)->xordev->dmadev->dev)

static void nvt_desc_init(struct nvt_xor_desc_slot *desc,
						  dma_addr_t addr, u32 byte_count,
						  enum dma_ctrl_flags flags)
{
	struct nvt_xor_desc *hw_desc = desc->hw_desc;
	xor_flow_debug();

	hw_desc->phy_next_desc = 0;
	hw_desc->phy_high0_addr &= ~NEXT_DESC_HIGH_MASK;
	hw_desc->desc_command = XOR_DESC_TAG_CHAIN_NULL;
	/* Enable end-of-descriptor interrupts only for DMA_PREP_INTERRUPT */
	//hw_desc->desc_command |= (flags & DMA_PREP_INTERRUPT) ? XOR_DESC_EOD_INT_EN : 0;  // mark EOD for performance; let alloc_slot (or EOC) recycle slots
	hw_desc->phy_dest_addr = addr;
	hw_desc->phy_high0_addr &= ~DEST_ADDR_HIGH_MASK;
	hw_desc->phy_high0_addr |= ((addr >> 32) << DEST_ADDR_HIGH_SHIFT);
	hw_desc->byte_count = byte_count;
}

static void nvt_desc_set_next_desc(struct nvt_xor_desc_slot *desc,
								   dma_addr_t next_desc_addr)
{
	struct nvt_xor_desc *hw_desc = desc->hw_desc;
	xor_flow_debug();

	BUG_ON((hw_desc->phy_next_desc) && (hw_desc->phy_high0_addr & NEXT_DESC_HIGH_MASK));
	hw_desc->phy_next_desc = next_desc_addr;
	hw_desc->phy_high0_addr &= ~NEXT_DESC_HIGH_MASK;
	hw_desc->phy_high0_addr |= ((next_desc_addr >> 32) << NEXT_DESC_HIGH_SHIFT);
	if (hw_desc->desc_command & XOR_DESC_TAG_CHAIN_NULL) {
		hw_desc->desc_command &= ~XOR_DESC_TAG_CHAIN_NULL;
	}
}

static void nvt_desc_set_src_addr(struct nvt_xor_desc_slot *desc,
								  int index, dma_addr_t addr)
{
	struct nvt_xor_desc *hw_desc = desc->hw_desc;
	hw_desc->phy_src_addr[nvt_phy_src_idx(index)] = addr;
	xor_flow_debug();

	switch (index) {
	case 0:
		hw_desc->phy_high0_addr &= ~SRC0_ADDR_HIGH_MASK;
		hw_desc->phy_high0_addr |= ((addr >> 32) << SRC0_ADDR_HIGH_SHIFT);
		break;
	case 1:
		hw_desc->phy_high1_addr &= ~SRC1_ADDR_HIGH_MASK;
		hw_desc->phy_high1_addr |= ((addr >> 32) << SRC1_ADDR_HIGH_SHIFT);
		break;
	case 2:
		hw_desc->phy_high1_addr &= ~SRC2_ADDR_HIGH_MASK;
		hw_desc->phy_high1_addr |= ((addr >> 32) << SRC2_ADDR_HIGH_SHIFT);
		break;
	case 3:
		hw_desc->phy_high1_addr &= ~SRC3_ADDR_HIGH_MASK;
		hw_desc->phy_high1_addr |= ((addr >> 32) << SRC3_ADDR_HIGH_SHIFT);
		break;
	case 4:
		hw_desc->phy_high1_addr &= ~SRC4_ADDR_HIGH_MASK;
		hw_desc->phy_high1_addr |= ((addr >> 32) << SRC4_ADDR_HIGH_SHIFT);
		break;
	case 5:
		hw_desc->phy_high1_addr &= ~SRC5_ADDR_HIGH_MASK;
		hw_desc->phy_high1_addr |= ((addr >> 32) << SRC5_ADDR_HIGH_SHIFT);
		break;
	case 6:
		hw_desc->phy_high1_addr &= ~SRC6_ADDR_HIGH_MASK;
		hw_desc->phy_high1_addr |= ((addr >> 32) << SRC6_ADDR_HIGH_SHIFT);
		break;
	case 7:
		hw_desc->phy_high1_addr &= ~SRC7_ADDR_HIGH_MASK;
		hw_desc->phy_high1_addr |= ((addr >> 32) << SRC7_ADDR_HIGH_SHIFT);
		break;
	default:
		BUG();
		break;
	}
	hw_desc->desc_command |= (1 << index);
}

static u64 nvt_chan_get_current_desc(struct nvt_xor_chan *nvt_chan)
{
	u64 current_desc = (u64)readl_relaxed(XOR_CURR_DESC(nvt_chan));
	xor_flow_debug();

	current_desc |= ((u64)readl_relaxed(XOR_CURR_DESC_H(nvt_chan)) << 32);
	return current_desc;
}

static void nvt_chan_set_next_descriptor(struct nvt_xor_chan *nvt_chan,
		dma_addr_t next_desc_addr)
{
	xor_flow_debug();

	writel_relaxed(next_desc_addr, XOR_NEXT_DESC(nvt_chan));
	writel_relaxed(next_desc_addr >> 32, XOR_NEXT_DESC_H(nvt_chan));
}

static void nvt_chan_unmask_interrupts(struct nvt_xor_chan *nvt_chan)
{
	u32 val = readl_relaxed(XOR_INTR_MASK(nvt_chan));
	xor_flow_debug();

	val |= XOR_INTR_MASK_VALUE;
	writel_relaxed(val, XOR_INTR_MASK(nvt_chan));
}

static u32 nvt_chan_get_intr_cause(struct nvt_xor_chan *nvt_chan)
{
	u32 intr_cause = readl_relaxed(XOR_INTR_CAUSE(nvt_chan));
	xor_flow_debug();

	return intr_cause;
}

static void nvt_chan_clear_eoc_cause(struct nvt_xor_chan *nvt_chan)
{
	u32 val;
	xor_flow_debug();

	val = XOR_INT_END_OF_DESC | XOR_INT_END_OF_CHAIN | XOR_INT_END_OF_ONESHOT | XOR_INT_STOPPED;
	xor_debug("chan(%d) val(0x%08x)\n", nvt_chan->idx, val);
	writel_relaxed(val, XOR_INTR_CAUSE(nvt_chan));
}

static void nvt_chan_clear_err_status(struct nvt_xor_chan *nvt_chan)
{
	u32 val = 0xFFFFFFFF;
	xor_flow_debug();

	writel_relaxed(val, XOR_INTR_CAUSE(nvt_chan));
}

static void nvt_chan_activate(struct nvt_xor_chan *nvt_chan)
{
	xor_flow_debug();
	xor_debug("chan(%d) en(%d)\n", nvt_chan->idx, nvt_chan_is_busy(nvt_chan));

	/* writel ensures all descriptors are flushed before activation */
	writel(XOR_DSC_CFG | readl(XOR_CONFIG(nvt_chan)), XOR_CONFIG(nvt_chan));
	writel(XE_ENABLE, XOR_ACTIVATION(nvt_chan));
}

static char nvt_chan_is_busy(struct nvt_xor_chan *nvt_chan)
{
	u32 status = readl_relaxed(XOR_ACTIVATION(nvt_chan));

	return (status & XE_ENABLE) ? 1 : 0;
}

/*
 * nvt_chan_start_new_chain - program the engine to operate on new
 * chain headed by sw_desc
 * Caller must hold &nvt_chan->lock while calling this function
 */
static void nvt_chan_start_new_chain(struct nvt_xor_chan *nvt_chan,
									 struct nvt_xor_desc_slot *sw_desc)
{
	xor_flow_debug();
	xor_debug("chan(%d) en(%d) [CURR(%x) NEXT(%x) CMD(%x) A(%x %x %x)] [desc(%pad) next(%x) cmd(%x) sts(%x) a(%x %x %x)]\n",
		nvt_chan->idx, nvt_chan_is_busy(nvt_chan),
		readl(XOR_CURR_DESC(nvt_chan)), readl(XOR_NEXT_DESC(nvt_chan)), readl(XOR_REG_CMD(nvt_chan)),
		readl(XOR_REG_DST(nvt_chan)), readl(XOR_REG_SRC0(nvt_chan)), readl(XOR_REG_SRC1(nvt_chan)),
		&sw_desc->async_tx.phys, sw_desc->hw_desc->phy_next_desc, sw_desc->hw_desc->desc_command, sw_desc->hw_desc->status,
		sw_desc->hw_desc->phy_dest_addr, sw_desc->hw_desc->phy_src_addr[0], sw_desc->hw_desc->phy_src_addr[1]);

	/* set the hardware chain */
	nvt_chan_set_next_descriptor(nvt_chan, sw_desc->async_tx.phys);

	nvt_chan->pending++;
	nvt_xor_issue_pending(&nvt_chan->dmachan);
}

static dma_cookie_t
nvt_desc_run_tx_complete_actions(struct nvt_xor_desc_slot *desc,
								 struct nvt_xor_chan *nvt_chan,
								 dma_cookie_t cookie)
{
	xor_flow_debug();
	BUG_ON(desc->async_tx.cookie < 0);

	if (desc->async_tx.cookie > 0) {
		cookie = desc->async_tx.cookie;

		dma_descriptor_unmap(&desc->async_tx);
		/* call the callback (must not sleep or submit new
		 * operations to this channel)
		 */
		dmaengine_desc_get_callback_invoke(&desc->async_tx, NULL);
	}

	/* run dependent operations */
	dma_run_dependencies(&desc->async_tx);

	return cookie;
}

static int
nvt_chan_clean_completed_slots(struct nvt_xor_chan *nvt_chan)
{
	struct nvt_xor_desc_slot *iter, *_iter;
	xor_flow_debug();

	list_for_each_entry_safe(iter, _iter, &nvt_chan->completed_slots,
							 node) {

		if (async_tx_test_ack(&iter->async_tx)) {
			list_move_tail(&iter->node, &nvt_chan->free_slots);
			xor_desc_debug("chan(%d) en(%d) [desc(%pad) a(%x %x %x) to free]\n", nvt_chan->idx, nvt_chan_is_busy(nvt_chan),
				&iter->async_tx.phys, iter->hw_desc->phy_dest_addr, iter->hw_desc->phy_src_addr[0], iter->hw_desc->phy_src_addr[1]);
			if (!list_empty(&iter->sg_tx_list)) {
				list_splice_tail_init(&iter->sg_tx_list,
									  &nvt_chan->free_slots);
			}
		}
	}
	return 0;
}

static int
nvt_desc_clean_slot(struct nvt_xor_desc_slot *desc,
					struct nvt_xor_chan *nvt_chan)
{
	xor_flow_debug();

	/* the client is allowed to attach dependent operations
	 * until 'ack' is set
	 */
	if (!async_tx_test_ack(&desc->async_tx)) {
		/* move this slot to the completed_slots */
		list_move_tail(&desc->node, &nvt_chan->completed_slots);
		xor_desc_debug("chan(%d) en(%d) flags(%d) [desc(%pad) a(%x %x %x) to completed]\n", nvt_chan->idx, nvt_chan_is_busy(nvt_chan), desc->async_tx.flags,
			&desc->async_tx.phys, desc->hw_desc->phy_dest_addr, desc->hw_desc->phy_src_addr[0], desc->hw_desc->phy_src_addr[1]);
		if (!list_empty(&desc->sg_tx_list)) {
			list_splice_tail_init(&desc->sg_tx_list,
								  &nvt_chan->completed_slots);
		}
	} else {
		list_move_tail(&desc->node, &nvt_chan->free_slots);
		xor_desc_debug("chan(%d) en(%d) flags(%d) [desc(%pad) a(%x %x %x) to free]\n", nvt_chan->idx, nvt_chan_is_busy(nvt_chan), desc->async_tx.flags,
			&desc->async_tx.phys, desc->hw_desc->phy_dest_addr, desc->hw_desc->phy_src_addr[0], desc->hw_desc->phy_src_addr[1]);
		if (!list_empty(&desc->sg_tx_list)) {
			list_splice_tail_init(&desc->sg_tx_list,
								  &nvt_chan->free_slots);
		}
	}

	return 0;
}

/* This function must be called with the nvt_xor_chan spinlock held */
static void nvt_chan_slot_cleanup(struct nvt_xor_chan *nvt_chan)
{
	struct nvt_xor_desc_slot *iter, *_iter;
	dma_cookie_t cookie = 0;
	int busy = nvt_chan_is_busy(nvt_chan);
	u64 current_desc = nvt_chan_get_current_desc(nvt_chan);
	int current_cleaned = 0;
	struct nvt_xor_desc *hw_desc;
	u32 retry;
	xor_flow_debug();

	xor_desc_debug("chan(%d) en(%d) current_desc(%llx)\n", nvt_chan->idx, nvt_chan_is_busy(nvt_chan), current_desc);
	nvt_chan_clean_completed_slots(nvt_chan);

	/* free completed slots from the chain starting with
	 * the oldest descriptor
	 */

	list_for_each_entry_safe(iter, _iter, &nvt_chan->chain,
							 node) {
		retry = 5;

again:
		/* clean finished descriptors */
		hw_desc = iter->hw_desc;
		if (hw_desc->status & XOR_DESC_DONE) {
			cookie = nvt_desc_run_tx_complete_actions(iter, nvt_chan,
					 cookie);

			/* done processing desc, clean slot */
			nvt_desc_clean_slot(iter, nvt_chan);
			hw_desc->status &= ~XOR_DESC_DONE;

			/* break if we did cleaned the current */
			if (iter->async_tx.phys == current_desc) {
				current_cleaned = 1;
				break;
			}
		} else if (retry) {
			/* WA: if the register status is not synchronized to the descriptor, give it a few chances */
			retry--;
			goto again;
		} else {
			if (iter->async_tx.phys == current_desc) {
				current_cleaned = 0;
				break;
			}
		}
	}

	if ((busy == 0) && !list_empty(&nvt_chan->chain)) {
		if (current_cleaned) {
			/*
			 * current descriptor cleaned and removed, run
			 * from list head
			 */
			iter = list_entry(nvt_chan->chain.next,
							  struct nvt_xor_desc_slot,
							  node);
			xor_debug("(current_cleaned) <<<re-start>>>\n");
			nvt_chan_start_new_chain(nvt_chan, iter);
		} else {
			if (!list_is_last(&iter->node, &nvt_chan->chain)) {
				/*
				 * descriptors are still waiting after
				 * current, trigger them
				 */
				iter = list_entry(iter->node.next,
								  struct nvt_xor_desc_slot,
								  node);
				xor_debug("(!current_cleaned) <<<re-start>>>\n");
				nvt_chan_start_new_chain(nvt_chan, iter);
			} else {
				/*
				 * some descriptors are still waiting
				 * to be cleaned
				 */
				xor_debug("(!current_cleaned) <<<irq_tasklet>>>\n");
				tasklet_schedule(&nvt_chan->irq_tasklet);
			}
		}
	}

	if (cookie > 0) {
		nvt_chan->dmachan.completed_cookie = cookie;
	}
}

static void nvt_xor_tasklet(struct tasklet_struct *t)
{
	struct nvt_xor_chan *nvt_chan = from_tasklet(nvt_chan, t, irq_tasklet);
	xor_flow_debug();

	spin_lock(&nvt_chan->lock);
	nvt_chan_slot_cleanup(nvt_chan);
	spin_unlock(&nvt_chan->lock);
}

static struct nvt_xor_desc_slot *
nvt_chan_alloc_slot(struct nvt_xor_chan *nvt_chan)
{
	struct nvt_xor_desc_slot *iter;
	xor_flow_debug();

	spin_lock_bh(&nvt_chan->lock);

	if (!list_empty(&nvt_chan->free_slots)) {
		iter = list_first_entry(&nvt_chan->free_slots,
								struct nvt_xor_desc_slot,
								node);

		list_move_tail(&iter->node, &nvt_chan->allocated_slots);

		spin_unlock_bh(&nvt_chan->lock);

		/* pre-ack descriptor */
		async_tx_ack(&iter->async_tx);
		iter->async_tx.cookie = -EBUSY;

		return iter;

	}

	spin_unlock_bh(&nvt_chan->lock);

	/* try to free some slots if the allocation fails */
	tasklet_schedule(&nvt_chan->irq_tasklet);

	return NULL;
}

/************************ DMA engine API functions ****************************/
static dma_cookie_t
nvt_xor_tx_submit(struct dma_async_tx_descriptor *tx)
{
	struct nvt_xor_desc_slot *sw_desc = to_nvt_xor_slot(tx);
	struct nvt_xor_chan *nvt_chan = to_nvt_xor_chan(tx->chan);
	struct nvt_xor_desc_slot *old_chain_tail;
	dma_cookie_t cookie;
	int new_hw_chain = 1;
	xor_flow_debug();

	xor_debug("chan(%d) en(%d) [CURR(%x) NEXT(%x) CMD(%x) A(%x %x %x)] [desc(%pad) next(%x) cmd(%x) sts(%x) a(%x %x %x)]\n",
		nvt_chan->idx, nvt_chan_is_busy(nvt_chan),
		readl(XOR_CURR_DESC(nvt_chan)), readl(XOR_NEXT_DESC(nvt_chan)), readl(XOR_REG_CMD(nvt_chan)),
		readl(XOR_REG_DST(nvt_chan)), readl(XOR_REG_SRC0(nvt_chan)), readl(XOR_REG_SRC1(nvt_chan)),
		&sw_desc->async_tx.phys, sw_desc->hw_desc->phy_next_desc, sw_desc->hw_desc->desc_command, sw_desc->hw_desc->status,
		sw_desc->hw_desc->phy_dest_addr, sw_desc->hw_desc->phy_src_addr[0], sw_desc->hw_desc->phy_src_addr[1]);

	spin_lock_bh(&nvt_chan->lock);
	cookie = dma_cookie_assign(tx);

	if (list_empty(&nvt_chan->chain)) {
		list_move_tail(&sw_desc->node, &nvt_chan->chain);
	} else {
		new_hw_chain = 0;
		old_chain_tail = list_entry(nvt_chan->chain.prev,
									struct nvt_xor_desc_slot,
									node);
		list_move_tail(&sw_desc->node, &nvt_chan->chain);

		xor_debug("chan(%d) desc(%pad) append to last(%pad) <<<append>>>\n", nvt_chan->idx, &sw_desc->async_tx.phys, &old_chain_tail->async_tx.phys);

		/* fix up the hardware chain */
		nvt_desc_set_next_desc(old_chain_tail, sw_desc->async_tx.phys);

		dma_sync_single_for_device(nvt_chan_to_devp(nvt_chan), old_chain_tail->async_tx.phys, NVT_XOR_SLOT_SIZE, DMA_TO_DEVICE);  // flush

		/* if the channel is not busy */
		if (!nvt_chan_is_busy(nvt_chan)) {
			u64 current_desc = nvt_chan_get_current_desc(nvt_chan);
			/*
			 * and the curren desc is the end of the chain before
			 * the append, then we need to start the channel
			 */
			if (current_desc == old_chain_tail->async_tx.phys) {
				new_hw_chain = 1;    //((XOR_DESC_CHAIN_NULL == 1) && (XOR_DESC_TAG_CHAIN_NULL == 0)) todo?
				xor_debug("(!busy) current(%llx) == last(%pad) <<<re-start>>>\n", current_desc, &old_chain_tail->async_tx.phys);
			}
		}
	}

	if (new_hw_chain) {
		nvt_chan_start_new_chain(nvt_chan, sw_desc);
	}

	spin_unlock_bh(&nvt_chan->lock);

	return cookie;
}

/* returns the number of allocated descriptors */
static int nvt_xor_alloc_chan_resources(struct dma_chan *chan)
{
	void *virt_desc;
	dma_addr_t dma_desc;
	int idx;
	struct nvt_xor_chan *nvt_chan = to_nvt_xor_chan(chan);
	struct nvt_xor_desc_slot *slot = NULL;
	int num_descs_in_pool = NVT_XOR_POOL_SIZE / NVT_XOR_SLOT_SIZE;
	xor_flow_debug();

	/* Allocate descriptor slots */
	idx = nvt_chan->slots_allocated;
	while (idx < num_descs_in_pool) {
		slot = kzalloc(sizeof(*slot), GFP_KERNEL);
		if (!slot) {
			dev_info(nvt_chan_to_devp(nvt_chan),
					 "channel only initialized %d descriptor slots",
					 idx);
			break;
		}
		virt_desc = nvt_chan->dma_desc_pool_virt;
		slot->hw_desc = virt_desc + idx * NVT_XOR_SLOT_SIZE;

		dma_async_tx_descriptor_init(&slot->async_tx, chan);
		slot->async_tx.tx_submit = nvt_xor_tx_submit;
		INIT_LIST_HEAD(&slot->node);
		INIT_LIST_HEAD(&slot->sg_tx_list);
		dma_desc = nvt_chan->dma_desc_pool;
		slot->async_tx.phys = dma_desc + idx * NVT_XOR_SLOT_SIZE;
		slot->idx = idx++;

		spin_lock_bh(&nvt_chan->lock);
		nvt_chan->slots_allocated = idx;
		list_add_tail(&slot->node, &nvt_chan->free_slots);
		spin_unlock_bh(&nvt_chan->lock);
	}

	xor_debug("chan(%d) allocated (%d) descriptor slots\n", nvt_chan->idx, nvt_chan->slots_allocated);

	return nvt_chan->slots_allocated ? : -ENOMEM;
}

static struct dma_async_tx_descriptor *
nvt_xor_prep_dma_xor(struct dma_chan *chan, dma_addr_t dest, dma_addr_t *src,
					 unsigned int src_cnt, size_t len, unsigned long flags)
{
	struct nvt_xor_chan *nvt_chan = to_nvt_xor_chan(chan);
	struct nvt_xor_desc_slot *sw_desc;
	xor_flow_debug();

	if (unlikely(len < NVT_XOR_MIN_BYTE_COUNT)) {
		return NULL;
	}

	BUG_ON(len > NVT_XOR_MAX_BYTE_COUNT);

	sw_desc = nvt_chan_alloc_slot(nvt_chan);
	if (sw_desc) {
		sw_desc->type = DMA_XOR;
		sw_desc->async_tx.flags = flags;
		nvt_desc_init(sw_desc, dest, len, flags);

		while (src_cnt--) {
			nvt_desc_set_src_addr(sw_desc, src_cnt, src[src_cnt]);
		}

		dma_sync_single_for_device(nvt_chan_to_devp(nvt_chan), sw_desc->async_tx.phys, NVT_XOR_SLOT_SIZE, DMA_TO_DEVICE);  // flush

		xor_debug("chan(%d) en(%d) src_cnt(%d) len(%zu) dest(%pad) flags(%ld) [CURR(%x) NEXT(%x) CMD(%x) A(%x %x %x)] [desc(%pad) next(%x) cmd(%x) sts(%x) a(%x %x %x)]\n",
			nvt_chan->idx, nvt_chan_is_busy(nvt_chan), src_cnt, len, &dest, flags,
			readl(XOR_CURR_DESC(nvt_chan)), readl(XOR_NEXT_DESC(nvt_chan)), readl(XOR_REG_CMD(nvt_chan)),
			readl(XOR_REG_DST(nvt_chan)), readl(XOR_REG_SRC0(nvt_chan)), readl(XOR_REG_SRC1(nvt_chan)),
			&sw_desc->async_tx.phys, sw_desc->hw_desc->phy_next_desc, sw_desc->hw_desc->desc_command, sw_desc->hw_desc->status,
			sw_desc->hw_desc->phy_dest_addr, sw_desc->hw_desc->phy_src_addr[0], sw_desc->hw_desc->phy_src_addr[1]);
	} else {
		xor_debug("chan(%d) en(%d) src_cnt(%d) len(%zu) dest(%pad) flags(%ld) [CURR(%x) NEXT(%x) CMD(%x) A(%x %x %x)] [null]\n",
			nvt_chan->idx, nvt_chan_is_busy(nvt_chan), src_cnt, len, &dest, flags,
			readl(XOR_CURR_DESC(nvt_chan)), readl(XOR_NEXT_DESC(nvt_chan)), readl(XOR_REG_CMD(nvt_chan)),
			readl(XOR_REG_DST(nvt_chan)), readl(XOR_REG_SRC0(nvt_chan)), readl(XOR_REG_SRC1(nvt_chan)));
	}

	return sw_desc ? &sw_desc->async_tx : NULL;
}

static struct dma_async_tx_descriptor *
nvt_xor_prep_dma_memcpy(struct dma_chan *chan, dma_addr_t dest, dma_addr_t src,
						size_t len, unsigned long flags)
{
	xor_flow_debug();

	/*
	 * A MEMCPY operation is identical to an XOR operation with only
	 * a single source address.
	 */
	return nvt_xor_prep_dma_xor(chan, dest, &src, 1, len, flags);
}

static struct dma_async_tx_descriptor *
nvt_xor_prep_dma_interrupt(struct dma_chan *chan, unsigned long flags)
{
	struct nvt_xor_chan *nvt_chan = to_nvt_xor_chan(chan);
	dma_addr_t src, dest;
	size_t len;
	xor_flow_debug();

	src = nvt_chan->dummy_src_addr;
	dest = nvt_chan->dummy_dst_addr;
	len = NVT_XOR_MIN_BYTE_COUNT;

	/*
	 * We implement the DMA_INTERRUPT operation as a minimum sized
	 * XOR operation with a single dummy source address.
	 */
	return nvt_xor_prep_dma_xor(chan, dest, &src, 1, len, flags);
}

static void nvt_xor_free_chan_resources(struct dma_chan *chan)
{
	struct nvt_xor_chan *nvt_chan = to_nvt_xor_chan(chan);
	struct nvt_xor_desc_slot *iter, *_iter;
	int in_use_descs = 0;
	xor_flow_debug();

	spin_lock_bh(&nvt_chan->lock);

	nvt_chan_slot_cleanup(nvt_chan);

	list_for_each_entry_safe(iter, _iter, &nvt_chan->chain,
							 node) {
		in_use_descs++;
		list_move_tail(&iter->node, &nvt_chan->free_slots);
	}
	list_for_each_entry_safe(iter, _iter, &nvt_chan->completed_slots,
							 node) {
		in_use_descs++;
		list_move_tail(&iter->node, &nvt_chan->free_slots);
	}
	list_for_each_entry_safe(iter, _iter, &nvt_chan->allocated_slots,
							 node) {
		in_use_descs++;
		list_move_tail(&iter->node, &nvt_chan->free_slots);
	}
	list_for_each_entry_safe_reverse(
		iter, _iter, &nvt_chan->free_slots, node) {
		list_del(&iter->node);
		kfree(iter);
		nvt_chan->slots_allocated--;
	}

	xor_debug("chan(%d) slots_allocated(%d)\n", nvt_chan->idx, nvt_chan->slots_allocated);
	spin_unlock_bh(&nvt_chan->lock);

	if (in_use_descs)
		dev_err(nvt_chan_to_devp(nvt_chan),
				"freeing (%d) in use descriptors!\n", in_use_descs);
}

/**
 * nvt_xor_status - poll the status of an XOR transaction
 * @chan: XOR channel handle
 * @cookie: XOR transaction identifier
 * @txstate: XOR transactions state holder (or NULL)
 */
static enum dma_status nvt_xor_status(struct dma_chan *chan,
									  dma_cookie_t cookie,
									  struct dma_tx_state *txstate)
{
	struct nvt_xor_chan *nvt_chan = to_nvt_xor_chan(chan);
	enum dma_status ret;
	xor_flow_debug();

	ret = dma_cookie_status(chan, cookie, txstate);
	if (ret == DMA_COMPLETE) {
		return ret;
	}

	spin_lock_bh(&nvt_chan->lock);
	nvt_chan_slot_cleanup(nvt_chan);
	spin_unlock_bh(&nvt_chan->lock);

	return dma_cookie_status(chan, cookie, txstate);
}

static void nvt_chan_dump_regs(struct nvt_xor_chan *nvt_chan)
{
	dev_err(nvt_chan_to_devp(nvt_chan), "channel_id     (%d)\n", nvt_chan->idx);

	dev_err(nvt_chan_to_devp(nvt_chan), "activation     (0x%08x)\n", readl_relaxed(XOR_ACTIVATION(nvt_chan)));

	dev_err(nvt_chan_to_devp(nvt_chan), "intr_cause     (0x%08x) , intr_mask      (0x%08x)\n", readl_relaxed(XOR_INTR_CAUSE(nvt_chan)), readl_relaxed(XOR_INTR_MASK(nvt_chan)));

	dev_err(nvt_chan_to_devp(nvt_chan), "next_desc      (0x%08x) , next_desc_h    (0x%08x)\n", readl_relaxed(XOR_NEXT_DESC(nvt_chan)), readl_relaxed(XOR_NEXT_DESC_H(nvt_chan)));

	dev_err(nvt_chan_to_devp(nvt_chan), "curr_desc      (0x%08x) , curr_desc_h    (0x%08x)\n", readl_relaxed(XOR_CURR_DESC(nvt_chan)), readl_relaxed(XOR_CURR_DESC_H(nvt_chan)));

	dev_err(nvt_chan_to_devp(nvt_chan), "curr_bytecount (0x%08x)\n", readl_relaxed(XOR_CURR_BYTE_COUNT(nvt_chan)));

	dev_err(nvt_chan_to_devp(nvt_chan), "curr_cmd       (0x%08x)\n", readl_relaxed(XOR_REG_CMD(nvt_chan)));
}

static void nvt_chan_dump_desc(struct nvt_xor_chan *nvt_chan)
{
	struct nvt_xor_desc_slot *iter, *_iter;
	struct nvt_xor_desc *hw_desc;

	dev_err(nvt_chan_to_devp(nvt_chan), "<chain>\n");
	list_for_each_entry_safe(iter, _iter, &nvt_chan->chain,
							 node) {
		hw_desc = iter->hw_desc;
		dev_err(nvt_chan_to_devp(nvt_chan), "[desc(%pad) next(%08x) cmd(%08x) sts(%08x) a(%08x %08x %08x)]\n", &iter->async_tx.phys,
			hw_desc->phy_next_desc, hw_desc->desc_command, hw_desc->status,
			hw_desc->phy_dest_addr, hw_desc->phy_src_addr[0], hw_desc->phy_src_addr[1]);
	}

	dev_err(nvt_chan_to_devp(nvt_chan), "<free_slots>\n");
	list_for_each_entry_safe(iter, _iter, &nvt_chan->free_slots,
							 node) {
		hw_desc = iter->hw_desc;
		dev_err(nvt_chan_to_devp(nvt_chan), "[desc(%pad) next(%08x) cmd(%08x) sts(%08x) a(%08x %08x %08x)]\n", &iter->async_tx.phys,
			hw_desc->phy_next_desc, hw_desc->desc_command, hw_desc->status,
			hw_desc->phy_dest_addr, hw_desc->phy_src_addr[0], hw_desc->phy_src_addr[1]);
	}

	dev_err(nvt_chan_to_devp(nvt_chan), "<allocated_slots>\n");
	list_for_each_entry_safe(iter, _iter, &nvt_chan->allocated_slots,
							 node) {
		hw_desc = iter->hw_desc;
		dev_err(nvt_chan_to_devp(nvt_chan), "[desc(%pad) next(%08x) cmd(%08x) sts(%08x) a(%08x %08x %08x)]\n", &iter->async_tx.phys,
			hw_desc->phy_next_desc, hw_desc->desc_command, hw_desc->status,
			hw_desc->phy_dest_addr, hw_desc->phy_src_addr[0], hw_desc->phy_src_addr[1]);
	}

	dev_err(nvt_chan_to_devp(nvt_chan), "<completed_slots>\n");
	list_for_each_entry_safe(iter, _iter, &nvt_chan->completed_slots,
							 node) {
		hw_desc = iter->hw_desc;
		dev_err(nvt_chan_to_devp(nvt_chan), "[desc(%pad) next(%08x) cmd(%08x) sts(%08x) a(%08x %08x %08x)]\n", &iter->async_tx.phys,
			hw_desc->phy_next_desc, hw_desc->desc_command, hw_desc->status,
			hw_desc->phy_dest_addr, hw_desc->phy_src_addr[0], hw_desc->phy_src_addr[1]);
	}
}

static void nvt_chan_err_interrupt_handler(struct nvt_xor_chan *nvt_chan,
		u32 intr_cause)
{
	xor_flow_debug();
	dev_err(nvt_chan_to_devp(nvt_chan), "error on chan(%d). intr cause(0x%08x)\n",
			nvt_chan->idx, intr_cause);

	nvt_chan_dump_regs(nvt_chan);
	nvt_chan_dump_desc(nvt_chan);
	WARN_ON(1);
}

static irqreturn_t nvt_xor_interrupt_handler(int irq, void *data)
{
	struct nvt_xor_device *xordev = data;
	struct nvt_xor_chan *nvt_chan;
	u32 intr_cause = 0;
	int i;
	xor_isr_debug();

	for (i = 0; i < NVT_XOR_MAX_CHANNELS; i++) {
		nvt_chan = xordev->channels[i];
		if (!nvt_chan) {
			continue;
		}

		intr_cause = nvt_chan_get_intr_cause(nvt_chan);
		xor_isr_debug("chan(%d) en(%d) intr cause(0x%x)\n", nvt_chan->idx, nvt_chan_is_busy(nvt_chan), intr_cause);

		if (intr_cause & XOR_INTR_ERRORS) {
			nvt_chan_err_interrupt_handler(nvt_chan, intr_cause);
		}

		tasklet_schedule(&nvt_chan->irq_tasklet);
		nvt_chan_clear_eoc_cause(nvt_chan);
	}

	return IRQ_HANDLED;
}

static void nvt_xor_issue_pending(struct dma_chan *chan)
{
	struct nvt_xor_chan *nvt_chan = to_nvt_xor_chan(chan);
	xor_flow_debug();

	if (nvt_chan->pending >= NVT_XOR_THRESHOLD) {
		nvt_chan->pending = 0;
		nvt_chan_activate(nvt_chan);
	}
}

/*
 * Perform a transaction to verify the HW works.
 */

static int nvt_chan_memcpy_self_test(struct nvt_xor_chan *nvt_chan)
{
	int i, ret;
	void *src, *dest;
	dma_addr_t src_dma, dest_dma;
	struct dma_chan *dma_chan;
	dma_cookie_t cookie;
	struct dma_async_tx_descriptor *tx;
	struct dmaengine_unmap_data *unmap;
	int err = 0;
	xor_flow_debug();

	src = kmalloc(PAGE_SIZE, GFP_KERNEL);
	if (!src) {
		return -ENOMEM;
	}

	dest = kzalloc(PAGE_SIZE, GFP_KERNEL);
	if (!dest) {
		kfree(src);
		return -ENOMEM;
	}

	/* Fill in src buffer */
	for (i = 0; i < PAGE_SIZE; i++) {
		((u8 *) src)[i] = (u8)i;
	}

	dma_chan = &nvt_chan->dmachan;
	if (nvt_xor_alloc_chan_resources(dma_chan) < 1) {
		err = -ENODEV;
		goto out;
	}

	unmap = dmaengine_get_unmap_data(dma_chan->device->dev, 2, GFP_KERNEL);
	if (!unmap) {
		err = -ENOMEM;
		goto free_resources;
	}

	src_dma = dma_map_page(dma_chan->device->dev, virt_to_page(src),
						   offset_in_page(src), PAGE_SIZE,
						   DMA_TO_DEVICE);
	unmap->addr[0] = src_dma;

	ret = dma_mapping_error(dma_chan->device->dev, src_dma);
	if (ret) {
		err = -ENOMEM;
		goto free_resources;
	}
	unmap->to_cnt = 1;

	dest_dma = dma_map_page(dma_chan->device->dev, virt_to_page(dest),
							offset_in_page(dest), PAGE_SIZE,
							DMA_FROM_DEVICE);
	unmap->addr[1] = dest_dma;

	ret = dma_mapping_error(dma_chan->device->dev, dest_dma);
	if (ret) {
		err = -ENOMEM;
		goto free_resources;
	}
	unmap->from_cnt = 1;
	unmap->len = PAGE_SIZE;

	tx = nvt_xor_prep_dma_memcpy(dma_chan, dest_dma, src_dma,
								 PAGE_SIZE, 0);
	if (!tx) {
		dev_err(dma_chan->device->dev,
				"Self-test cannot prepare operation, disabling\n");
		err = -ENODEV;
		goto free_resources;
	}

	cookie = nvt_xor_tx_submit(tx);
	if (dma_submit_error(cookie)) {
		dev_err(dma_chan->device->dev,
				"Self-test submit error, disabling\n");
		err = -ENODEV;
		goto free_resources;
	}

	nvt_xor_issue_pending(dma_chan);
	async_tx_ack(tx);
	msleep(1);
	if (nvt_xor_status(dma_chan, cookie, NULL) !=
		DMA_COMPLETE) {
		dev_err(dma_chan->device->dev,
				"Self-test copy timed out, disabling\n");
		err = -ENODEV;
		goto free_resources;
	}

	dma_sync_single_for_cpu(dma_chan->device->dev, dest_dma,
							PAGE_SIZE, DMA_FROM_DEVICE);
	if (memcmp(src, dest, PAGE_SIZE)) {
		dev_err(dma_chan->device->dev,
				"Self-test copy failed compare, disabling\n");
		err = -ENODEV;
		goto free_resources;
	}

free_resources:
	dmaengine_unmap_put(unmap);
	nvt_xor_free_chan_resources(dma_chan);
out:
	kfree(src);
	kfree(dest);
	return err;
}

#define NVT_XOR_NUM_SRC_TEST 4 /* must be <= 15 */
static int
nvt_chan_xor_self_test(struct nvt_xor_chan *nvt_chan)
{
	int i, src_idx, ret;
	struct page *dest;
	struct page *xor_srcs[NVT_XOR_NUM_SRC_TEST];
	dma_addr_t dma_srcs[NVT_XOR_NUM_SRC_TEST];
	dma_addr_t dest_dma;
	struct dma_async_tx_descriptor *tx;
	struct dmaengine_unmap_data *unmap;
	struct dma_chan *dma_chan;
	dma_cookie_t cookie;
	u8 cmp_byte = 0;
	u32 cmp_word;
	int err = 0;
	int src_count = NVT_XOR_NUM_SRC_TEST;
	xor_flow_debug();

	for (src_idx = 0; src_idx < src_count; src_idx++) {
		xor_srcs[src_idx] = alloc_page(GFP_KERNEL);
		if (!xor_srcs[src_idx]) {
			while (src_idx--) {
				__free_page(xor_srcs[src_idx]);
			}
			return -ENOMEM;
		}
	}

	dest = alloc_page(GFP_KERNEL);
	if (!dest) {
		while (src_idx--) {
			__free_page(xor_srcs[src_idx]);
		}
		return -ENOMEM;
	}

	/* Fill in src buffers */
	for (src_idx = 0; src_idx < src_count; src_idx++) {
		u8 *ptr = page_address(xor_srcs[src_idx]);
		for (i = 0; i < PAGE_SIZE; i++) {
			ptr[i] = (1 << src_idx);
		}
	}

	for (src_idx = 0; src_idx < src_count; src_idx++) {
		cmp_byte ^= (u8)(1 << src_idx);
	}

	cmp_word = (cmp_byte << 24) | (cmp_byte << 16) |
			   (cmp_byte << 8) | cmp_byte;

	memset(page_address(dest), 0, PAGE_SIZE);

	dma_chan = &nvt_chan->dmachan;
	if (nvt_xor_alloc_chan_resources(dma_chan) < 1) {
		err = -ENODEV;
		goto out;
	}

	unmap = dmaengine_get_unmap_data(dma_chan->device->dev, src_count + 1,
									 GFP_KERNEL);
	if (!unmap) {
		err = -ENOMEM;
		goto free_resources;
	}

	/* test xor */
	for (i = 0; i < src_count; i++) {
		unmap->addr[i] = dma_map_page(dma_chan->device->dev, xor_srcs[i],
									  0, PAGE_SIZE, DMA_TO_DEVICE);
		dma_srcs[i] = unmap->addr[i];
		ret = dma_mapping_error(dma_chan->device->dev, unmap->addr[i]);
		if (ret) {
			err = -ENOMEM;
			goto free_resources;
		}
		unmap->to_cnt++;
	}

	unmap->addr[src_count] = dma_map_page(dma_chan->device->dev, dest, 0, PAGE_SIZE,
										  DMA_FROM_DEVICE);
	dest_dma = unmap->addr[src_count];
	ret = dma_mapping_error(dma_chan->device->dev, unmap->addr[src_count]);
	if (ret) {
		err = -ENOMEM;
		goto free_resources;
	}
	unmap->from_cnt = 1;
	unmap->len = PAGE_SIZE;

	tx = nvt_xor_prep_dma_xor(dma_chan, dest_dma, dma_srcs,
							  src_count, PAGE_SIZE, 0);
	if (!tx) {
		dev_err(dma_chan->device->dev,
				"Self-test cannot prepare operation, disabling\n");
		err = -ENODEV;
		goto free_resources;
	}

	cookie = nvt_xor_tx_submit(tx);
	if (dma_submit_error(cookie)) {
		dev_err(dma_chan->device->dev,
				"Self-test submit error, disabling\n");
		err = -ENODEV;
		goto free_resources;
	}

	nvt_xor_issue_pending(dma_chan);
	async_tx_ack(tx);
	msleep(8);

	if (nvt_xor_status(dma_chan, cookie, NULL) !=
		DMA_COMPLETE) {
		dev_err(dma_chan->device->dev,
				"Self-test xor timed out, disabling\n");
		err = -ENODEV;
		goto free_resources;
	}

	dma_sync_single_for_cpu(dma_chan->device->dev, dest_dma,
							PAGE_SIZE, DMA_FROM_DEVICE);
	for (i = 0; i < (PAGE_SIZE / sizeof(u32)); i++) {
		u32 *ptr = page_address(dest);
		if (ptr[i] != cmp_word) {
			dev_err(dma_chan->device->dev,
					"Self-test xor failed compare, disabling. index %d, data %x, expected %x\n",
					i, ptr[i], cmp_word);
			err = -ENODEV;
			goto free_resources;
		}
	}

free_resources:
	dmaengine_unmap_put(unmap);
	nvt_xor_free_chan_resources(dma_chan);
out:
	src_idx = src_count;
	while (src_idx--) {
		__free_page(xor_srcs[src_idx]);
	}
	__free_page(dest);
	return err;
}

static int nvt_xor_channel_remove(struct nvt_xor_device *xordev,
								  struct platform_device *pdev,
								  struct nvt_xor_chan *nvt_chan)
{
	struct dma_chan *chan, *_chan;
	struct device *dev = &pdev->dev;
	xor_flow_debug();

	dma_async_device_unregister(xordev->dmadev);

	dma_free_coherent(dev, NVT_XOR_POOL_SIZE,
					  nvt_chan->dma_desc_pool_virt, nvt_chan->dma_desc_pool);
	dma_unmap_single(dev, nvt_chan->dummy_src_addr,
					 NVT_XOR_MIN_BYTE_COUNT, DMA_FROM_DEVICE);
	dma_unmap_single(dev, nvt_chan->dummy_dst_addr,
					 NVT_XOR_MIN_BYTE_COUNT, DMA_TO_DEVICE);

	list_for_each_entry_safe(chan, _chan, &xordev->dmadev->channels,
							 device_node) {
		list_del(&chan->device_node);
	}

	return 0;
}


/*
 * Since this XOR driver is basically used only for RAID5, we don't
 * need to care about synchronizing ->suspend with DMA activity,
 * because the DMA engine will naturally be quiet due to the block
 * devices being suspended.
 */
static int nvt_xor_suspend(struct platform_device *pdev, pm_message_t state)
{
	struct nvt_xor_device *xordev = platform_get_drvdata(pdev);
	int i;
	xor_flow_debug();

	for (i = 0; i < NVT_XOR_MAX_CHANNELS; i++) {
		struct nvt_xor_chan *nvt_chan = xordev->channels[i];

		if (!nvt_chan) {
			continue;
		}

		nvt_chan->saved_int_mask_reg =
			readl_relaxed(XOR_INTR_MASK(nvt_chan));
	}

	return 0;
}

static int nvt_xor_resume(struct platform_device *dev)
{
	struct nvt_xor_device *xordev = platform_get_drvdata(dev);
	int i;
	xor_flow_debug();

	for (i = 0; i < NVT_XOR_MAX_CHANNELS; i++) {
		struct nvt_xor_chan *nvt_chan = xordev->channels[i];

		if (!nvt_chan) {
			continue;
		}

		writel_relaxed(nvt_chan->saved_int_mask_reg,
					   XOR_INTR_MASK(nvt_chan));
	}

	return 0;
}

static const struct of_device_id nvt_xor_dt_ids[] = {
	{ .compatible = "nvt,nvt_xor" },
	{},
};

static int nvt_xor_probe(struct platform_device *pdev)
{
	struct nvt_xor_device *xordev;
	struct nvt_xor_chan *nvt_chan;
	dma_cap_mask_t 	cap_mask;
	void __iomem    *base;
	int irq;
	int i, ret;
	xor_flow_debug();

	dev_notice(&pdev->dev, "Novatek shared XOR driver\n");

	xordev = devm_kzalloc(&pdev->dev, sizeof(*xordev), GFP_KERNEL);
	if (!xordev) {
		dev_err(&pdev->dev, "xordev kzalloc failed\n");
		return -ENOMEM;
	}

	xordev->dmadev = devm_kzalloc(&pdev->dev, sizeof(xordev->dmadev), GFP_KERNEL);
	if (!xordev) {
		dev_err(&pdev->dev, "dma_device kzalloc failed\n");
		return -ENOMEM;
	}

	irq = platform_get_irq(pdev, 0);
	if (irq < 0) {
		dev_err(&pdev->dev, "get irq fail\n");
		return irq;
	}
	xordev->irq = irq;

	base = devm_platform_ioremap_resource(pdev, 0);
	if (IS_ERR(base)) {
		dev_err(&pdev->dev, "ioremap resource fail\n");
		return PTR_ERR(base);
	}
	xordev->xor_base = base;

	ret = request_irq(xordev->irq, nvt_xor_interrupt_handler, 0, dev_name(&pdev->dev), xordev);
	if (ret) {
		dev_err(&pdev->dev, "can't request irq\n");
		goto err_free_irq;
	}

#ifdef CONFIG_OF
	dev_info(&pdev->dev, "%s, clk_name = %s\n", __func__, dev_name(&pdev->dev));
	xordev->clk = clk_get(&pdev->dev, dev_name(&pdev->dev));
	if (IS_ERR(xordev->clk)) {
		dev_err(&pdev->dev, "can't find clock %s\n", dev_name(&pdev->dev));
		goto err_free_irq;
	} else {
		ret = clk_prepare_enable(xordev->clk);
		if (ret) {
			dev_err(&pdev->dev, "can't prepare or enable clock\n");
			goto err_free_irq;
		}
	}
#endif

	dma_cap_zero(cap_mask);
	dma_cap_set(DMA_XOR, cap_mask);
	dma_cap_set(DMA_MEMCPY, cap_mask);
	dma_cap_set(DMA_INTERRUPT, cap_mask);
	xordev->dmadev->cap_mask = cap_mask;

	xordev->dmadev->dev = &pdev->dev;
	xordev->dmadev->device_alloc_chan_resources = nvt_xor_alloc_chan_resources;
	xordev->dmadev->device_free_chan_resources = nvt_xor_free_chan_resources;
	xordev->dmadev->device_tx_status = nvt_xor_status;
	xordev->dmadev->device_issue_pending = nvt_xor_issue_pending;
	xordev->dmadev->device_prep_dma_interrupt = nvt_xor_prep_dma_interrupt;
	xordev->dmadev->device_prep_dma_memcpy = nvt_xor_prep_dma_memcpy;
	xordev->dmadev->max_xor = 8;
	xordev->dmadev->device_prep_dma_xor = nvt_xor_prep_dma_xor;

	ret = dma_set_mask(&pdev->dev, DMA_BIT_MASK(36));
	if (ret != 0) {
		dev_err(&pdev->dev, "set dma mask fail\n");
		return -ENXIO;
	}

	pdev->dev.coherent_dma_mask = DMA_BIT_MASK(36);

	/* Init channels. */
	INIT_LIST_HEAD(&xordev->dmadev->channels);
	for (i = 0; i < NVT_XOR_MAX_CHANNELS; i++) {
		struct nvt_xor_chan *xorchan;
		struct dma_device *dma_dev;

		xorchan = devm_kzalloc(&pdev->dev, sizeof(*xorchan), GFP_KERNEL);
		if (!xorchan) {
			dev_err(&pdev->dev, "chan kzalloc failed\n");
			return -ENOMEM;
		}

		dma_dev = xordev->dmadev;
		xorchan->dmachan.device = dma_dev;
		xorchan->xordev = xordev;

		/*
	 	 * These source and destination dummy buffers are used to implement
	     * a DMA_INTERRUPT operation as a minimum-sized XOR operation.
	     * Hence, we only need to map the buffers at initialization-time.
	     */
		xorchan->dummy_src_addr = dma_map_single(dma_dev->dev,
							xorchan->dummy_src, NVT_XOR_MIN_BYTE_COUNT, DMA_FROM_DEVICE);
		xorchan->dummy_dst_addr = dma_map_single(dma_dev->dev,
							xorchan->dummy_dst, NVT_XOR_MIN_BYTE_COUNT, DMA_TO_DEVICE);

		/* allocate coherent memory for hardware descriptors
	 	 * note: writecombine gives slightly better performance, but
	 	 * requires that we explicitly flush the writes
	 	 */
		xorchan->dma_desc_pool_virt =
			dma_alloc_wc(&pdev->dev, NVT_XOR_POOL_SIZE, &xorchan->dma_desc_pool, GFP_KERNEL);
		if (!xorchan->dma_desc_pool_virt) {
			dev_err(&pdev->dev, "dma_desc_pool alloc failed\n");
			goto err_channel_add;
		}
		if (!IS_ALIGNED(xorchan->dma_desc_pool, NVT_XOR_POOL_ALIGNMENT)) {
			dev_err(&pdev->dev, "dma_desc_pool(%llx) is not %d byte alignment\n", xorchan->dma_desc_pool, NVT_XOR_POOL_ALIGNMENT);
			goto err_free_dma;
		}

		xorchan->mmr_base = xordev->xor_base;
		xorchan->idx = i;

		/* clear errors before enabling interrupts */
		nvt_chan_clear_err_status(xorchan);
		nvt_chan_unmask_interrupts(xorchan);
		tasklet_setup(&xorchan->irq_tasklet, nvt_xor_tasklet);

		spin_lock_init(&xorchan->lock);
		INIT_LIST_HEAD(&xorchan->chain);
		INIT_LIST_HEAD(&xorchan->completed_slots);
		INIT_LIST_HEAD(&xorchan->free_slots);
		INIT_LIST_HEAD(&xorchan->allocated_slots);
		dma_cookie_init(&xorchan->dmachan);
		list_add_tail(&xorchan->dmachan.device_node,
					  &xordev->dmadev->channels);

		dev_info(&pdev->dev, "chan%d( %s%s%s) desc_pool(0x%llx) desc_num(%d)\n",
			 i,
			 dma_has_cap(DMA_XOR, dma_dev->cap_mask) ? "xor " : "",
			 dma_has_cap(DMA_MEMCPY, dma_dev->cap_mask) ? "cpy " : "",
			 dma_has_cap(DMA_INTERRUPT, dma_dev->cap_mask) ? "intr " : "",
			 xorchan->dma_desc_pool, (NVT_XOR_POOL_SIZE / NVT_XOR_SLOT_SIZE));

		xordev->channels[i] = xorchan;
	}

	platform_set_drvdata(pdev, xordev);
	ret = dma_async_device_register(xordev->dmadev);
	if (ret) {
		dev_err(&pdev->dev, "dma_async_device_register failed\n");
		goto err_free_dma;
	}

	for (i = 0; i < NVT_XOR_MAX_CHANNELS; i++) {
		nvt_chan = xordev->channels[i];
		if (dma_has_cap(DMA_MEMCPY, xordev->dmadev->cap_mask)) {
			ret = nvt_chan_memcpy_self_test(nvt_chan);
			if (ret) {
				dev_err(&pdev->dev, "memcpy self test failed returned %d\n", ret);
				goto err_free_dma;
			}
		}

		if (dma_has_cap(DMA_XOR, xordev->dmadev->cap_mask)) {
			ret = nvt_chan_xor_self_test(nvt_chan);
			if (ret) {
				dev_err(&pdev->dev, "xor self test failed returned %d\n", ret);
				goto err_free_dma;
			}
		}

	}

	return 0;


err_free_dma:
	dma_free_coherent(&pdev->dev, NVT_XOR_POOL_SIZE,
					  nvt_chan->dma_desc_pool_virt, nvt_chan->dma_desc_pool);
err_free_irq:
	free_irq(xordev->irq, xordev);

err_channel_add:
	for (i = 0; i < NVT_XOR_MAX_CHANNELS; i++)
		if (xordev->channels[i]) {
			nvt_xor_channel_remove(xordev, pdev, xordev->channels[i]);
		}

	if (!IS_ERR(xordev->clk)) {
		clk_disable_unprepare(xordev->clk);
		clk_put(xordev->clk);
	}

	return ret;
}

static struct platform_driver nvt_xor_driver = {
	.probe      = nvt_xor_probe,
	.suspend        = nvt_xor_suspend,
	.resume         = nvt_xor_resume,
	.driver     = {
		.name           = NVT_XOR_NAME,
		.of_match_table = nvt_xor_dt_ids,
	},
};

builtin_platform_driver(nvt_xor_driver);

MODULE_DESCRIPTION("DMA engine driver for Novatek's XOR engine");
MODULE_VERSION(XOR_MODULE_VER);
MODULE_LICENSE("GPL");
