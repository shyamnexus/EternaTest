/* SPDX-License-Identifier: GPL-2.0-only */

#ifndef NVT_XOR_H
#define NVT_XOR_H

#include <linux/types.h>
#include <linux/io.h>
#include <linux/dmaengine.h>
#include <linux/interrupt.h>

#define NVT_XOR_POOL_ALIGNMENT       64
#define NVT_XOR_POOL_SIZE            (NVT_XOR_SLOT_SIZE * 256)
#define NVT_XOR_SLOT_SIZE            64
#define NVT_XOR_THRESHOLD            1
#define NVT_XOR_MAX_CHANNELS         2

#define NVT_XOR_MIN_BYTE_COUNT       SZ_1
#define NVT_XOR_MAX_BYTE_COUNT       (SZ_16M - 1)

/* Values for the XOR_DESC */
#define XOR_DESC_DONE                BIT(0)
#define XOR_DESC_CHAIN_NULL          BIT(4)
#define XOR_DESC_ERR_TOUT            BIT(8)
#define XOR_DESC_ERR_RCBUS           BIT(9)
#define XOR_DESC_ERR_WCBUS           BIT(10)
#define XOR_DESC_STOPPED             BIT(11)

#define XOR_DESC_TAG_CHAIN_NULL      BIT(16)
#define XOR_DESC_EOD_INT_EN          BIT(31)

/* Values for the XOR_REG */
#define XOR_ACTIVATION(chan)         (chan->mmr_base + 0x60 + (chan->idx * 4))
#define XE_ENABLE                    BIT(0)
#define XE_STOP                      BIT(1)
#define XE_WDT_EN                    BIT(2)

#define XOR_NEXT_DESC(chan)          (chan->mmr_base + 0x100 + (chan->idx * 4))
#define XOR_NEXT_DESC_H(chan)        (chan->mmr_base + 0x120 + (chan->idx * 4))

#define XOR_CURR_DESC(chan)          (chan->mmr_base + 0x140 + (chan->idx * 4))
#define XOR_CURR_DESC_H(chan)        (chan->mmr_base + 0x160 + (chan->idx * 4))

#define XOR_CURR_BYTE_COUNT(chan)    (chan->mmr_base + 0x180 + (chan->idx * 4))

#define XOR_REG_CMD(chan)            (chan->mmr_base + 0x200 + (chan->idx * 4))
#define XOR_REG_DST(chan)            (chan->mmr_base + 0x260 + (chan->idx * 4))
#define XOR_REG_SRC0(chan)           (chan->mmr_base + 0x2A0 + (chan->idx * 4))
#define XOR_REG_SRC1(chan)           (chan->mmr_base + 0x2E0 + (chan->idx * 4))

#define XOR_INTR_MASK(chan)          (chan->mmr_base + 0x20 + (chan->idx * 4))
#define XOR_INTR_CAUSE(chan)         (chan->mmr_base + 0x40 + (chan->idx * 4))
#define XOR_INT_END_OF_CHAIN         BIT(1)
#define XOR_INT_END_OF_ONESHOT       BIT(3)
#define XOR_INT_ERR_TOUT             BIT(8)
#define XOR_INT_ERR_RCBUS            BIT(9)
#define XOR_INT_ERR_WCBUS            BIT(10)
#define XOR_INT_STOPPED              BIT(11)
#define XOR_INT_END_OF_DESC          BIT(14)

#define XOR_DSC_MODE                 (XOR_INT_ERR_TOUT | XOR_INT_ERR_RCBUS | XOR_INT_ERR_WCBUS)

#define XOR_INTR_ERRORS              (XOR_INT_ERR_TOUT | XOR_INT_ERR_RCBUS | XOR_INT_ERR_WCBUS)

#define XOR_INTR_MASK_VALUE          (XOR_INT_END_OF_DESC | XOR_INT_END_OF_CHAIN | XOR_INT_STOPPED | XOR_INT_END_OF_ONESHOT | XOR_INTR_ERRORS)

#define XOR_CONFIG(chan)             (chan->mmr_base + 0x04)

#define XOR_MODE                     BIT(26)
#define XOR_DMA_RC_HACK              BIT(28)

#define XOR_DSC_CFG                  (XOR_MODE | XOR_DMA_RC_HACK)

/**
 * struct nvt_xor_chan - internal representation of a XOR channel
 * @pending: allows batching of hardware operations
 * @lock: serializes enqueue/dequeue operations to the descriptors pool
 * @mmr_base: memory mapped register base
 * @idx: the index of the xor channel
 * @chain: device chain view of the descriptors
 * @free_slots: free slots usable by the channel
 * @allocated_slots: slots allocated by the driver
 * @completed_slots: slots completed by HW but still need to be acked
 * @device: parent device
 * @common: common dmaengine channel object members
 * @slots_allocated: records the actual size of the descriptor slot pool
 * @irq_tasklet: bottom half where nvt_xor_slot_cleanup runs
 * @op_in_desc: new mode of driver, each op is writen to descriptor.
 */
struct nvt_xor_chan {
	int         pending;
	spinlock_t      lock; /* protects the descriptor slot pool */
	void __iomem        *mmr_base;
	unsigned int        idx;
	struct list_head    chain;
	struct list_head    free_slots;
	struct list_head    allocated_slots;
	struct list_head    completed_slots;
	dma_addr_t      dma_desc_pool;
	void            *dma_desc_pool_virt;
	size_t                  pool_size;
	struct dma_chan     dmachan;
	int         slots_allocated;
	struct tasklet_struct   irq_tasklet;
	char            dummy_src[NVT_XOR_MIN_BYTE_COUNT];
	char            dummy_dst[NVT_XOR_MIN_BYTE_COUNT];
	dma_addr_t      dummy_src_addr, dummy_dst_addr;
	u32                     saved_config_reg, saved_int_mask_reg;

	struct nvt_xor_device   *xordev;
};

struct nvt_xor_device {
	struct dma_device 	*dmadev;
	void __iomem 		*xor_base;
	struct clk 			*clk;
	int 				irq;
	struct nvt_xor_chan *channels[NVT_XOR_MAX_CHANNELS];
};

/**
 * struct nvt_xor_desc_slot - software descriptor
 * @node: node on the nvt_xor_chan lists
 * @hw_desc: virtual address of the hardware descriptor chain
 * @phys: hardware address of the hardware descriptor chain
 * @slot_used: slot in use or not
 * @idx: pool index
 * @tx_list: list of slots that make up a multi-descriptor transaction
 * @async_tx: support for the async_tx api
 */
struct nvt_xor_desc_slot {
	struct list_head                node;
	struct list_head                sg_tx_list;
	enum dma_transaction_type       type;
	struct nvt_xor_desc             *hw_desc;
	u16                             idx;
	struct dma_async_tx_descriptor  async_tx;
};

/*
 * This structure describes XOR descriptor size 64bytes. The
 * nvt_phy_src_idx() macro must be used when indexing the values of the
 * phy_src_addr[] array. This is due to the fact that the 'descriptor
 * swap' feature, used on big endian systems, swaps descriptors data
 * within blocks of 8 bytes. So two consecutive values of the
 * phy_src_addr[] array are actually swapped in big-endian, which
 * explains the different nvt_phy_src_idx() implementation.
 */
#if defined(__LITTLE_ENDIAN)
struct nvt_xor_desc {
	u32 status;     /* descriptor execution status */
	u32 desc_command;   /* type of operation to be carried out */
	u32 init_value;     /* initial value for memset operation */
	u32 phy_next_desc;  /* next descriptor address pointer */
	u32 byte_count;     /* size of src/dst blocks in bytes */
	u32 phy_dest_addr;  /* destination block address */
	u32 phy_high0_addr; /* high address for next_desc, dest_addr, src_addr[0] */
	u32 phy_src_addr[8];    /* source block addresses */
	u32 phy_high1_addr; /* high address for src_addr[1]~[7] */
};

/* Bit operations under phy_high0_addr */
#define NEXT_DESC_HIGH_MASK          GENMASK(3, 0)
#define DEST_ADDR_HIGH_MASK          GENMASK(7, 4)
#define SRC0_ADDR_HIGH_MASK          GENMASK(11, 8)
#define NEXT_DESC_HIGH_SHIFT         0
#define DEST_ADDR_HIGH_SHIFT         4
#define SRC0_ADDR_HIGH_SHIFT         8

/* Bit operations under phy_high1_addr */
#define SRC1_ADDR_HIGH_MASK          GENMASK(3, 0)
#define SRC2_ADDR_HIGH_MASK          GENMASK(7, 4)
#define SRC3_ADDR_HIGH_MASK          GENMASK(11, 8)
#define SRC4_ADDR_HIGH_MASK          GENMASK(15, 12)
#define SRC5_ADDR_HIGH_MASK          GENMASK(19, 16)
#define SRC6_ADDR_HIGH_MASK          GENMASK(23, 20)
#define SRC7_ADDR_HIGH_MASK          GENMASK(27, 24)
#define SRC1_ADDR_HIGH_SHIFT         0
#define SRC2_ADDR_HIGH_SHIFT         4
#define SRC3_ADDR_HIGH_SHIFT         8
#define SRC4_ADDR_HIGH_SHIFT         12
#define SRC5_ADDR_HIGH_SHIFT         16
#define SRC6_ADDR_HIGH_SHIFT         20
#define SRC7_ADDR_HIGH_SHIFT         24

#define nvt_phy_src_idx(src_idx) (src_idx)
#endif

#define to_nvt_sw_desc(addr_hw_desc)        \
	container_of(addr_hw_desc, struct nvt_xor_desc_slot, hw_desc)

#define nvt_hw_desc_slot_idx(hw_desc, idx)  \
	((void *)(((unsigned long)hw_desc) + ((idx) << 5)))

#endif
