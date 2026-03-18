/**
    NVT PCIE cascade driver
    To handle NVT PCIE cascade driver: RC/EP connection and isr handling
    @file pcie-pcie-edma.c
    @ingroup
    @note
    Copyright Novatek Microelectronics Corp. 2021. All rights reserved.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License version 2 as
    published by the Free Software Foundation.
*/
#include <linux/module.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/gpio.h>
#include <linux/irq.h>
#include <linux/interrupt.h>
#include <linux/irqchip/chained_irq.h>
#include <linux/soc/nvt/nvt-io.h>
#include <linux/soc/nvt/nvt-pcie-lib.h>
#include <linux/msi.h>

#include "nvt_pcie_int.h"
#include "pcie_reg.h"
#include "nvt_pcie_edma.h"

#ifndef CHKPNT
#define CHKPNT    printk("\033[37mCHK: %d, %s\033[0m\r\n", __LINE__, __func__)
#endif

#ifndef DBGD
#define DBGD(x)   printk("\033[0;35m%s=%d\033[0m\r\n", #x, x)
#endif

#ifndef DBGH
#define DBGH(x)   printk("\033[0;35m%s=0x%08X\033[0m\r\n", #x, x)
#endif

#ifndef DBG_DUMP
#define DBG_DUMP(fmtstr, args...) printk(fmtstr, ##args)
#endif

#ifndef DBG_ERR
#define DBG_ERR(fmtstr, args...)  printk("\033[0;31mERR:%s() \033[0m" fmtstr, __func__, ##args)
#endif

#ifndef DBG_WRN
#define DBG_WRN(fmtstr, args...)  printk("\033[0;33mWRN:%s() \033[0m" fmtstr, __func__, ##args)
#endif

#if 0
#define DBG_IND(fmtstr, args...) printk("%s(): " fmtstr, __func__, ##args)
#else
#ifndef DBG_IND
#define DBG_IND(fmtstr, args...)
#endif
#endif


struct nvt_pcie_chip *nvtpcie_edma_chip_ptr = NULL;
static edmac_info_t edmac_info[EDMAC_ENGINE];
static int gPcie_id;


#if PCIE_EDMA_INT_EN
irqreturn_t nvt_pcie_drv_isr(int irq, void *devid)
{
	UINT32 edma_ch;
	UINT32 status, event;
	struct edmac_info_s *info = &edmac_info[gPcie_id];
	UINT32 match = 0;

	//printk("%s: \r\n", __func__);
	//PCIE_SETDBI(nvtpcie_edma_chip_ptr,DMA_WRITE_INT_CLEAR_OFF_OFS, 0x1);

	//status = PCIE_GETREG(nvtpcie_edma_chip_ptr, 0x20);
	//printk("%s: ISR Status = 0x%x\r\n", __func__,status);

	//status = PCIE_GETDBI(nvtpcie_edma_chip_ptr,DMA_READ_INT_MASK_OFF_OFS);
	//printk("%s: R Msk = 0x%x\r\n", __func__,status);

	/* DMA_READ_INT_STATUS_OFF, [7:0]RD_DONE_INT_STATUS, [23:16]RD_ABORT_INT_STATUS */
	//status = ioread32(edmac_vbase + 0xA0);
	status = PCIE_GETDBI(nvtpcie_edma_chip_ptr, DMA_READ_INT_STATUS_OFF_OFS);
	/* clear status */
	//iowrite32(status, edmac_vbase + 0xAC);
	PCIE_SETDBI(nvtpcie_edma_chip_ptr, DMA_READ_INT_CLEAR_OFF_OFS, status);
	//printk("%s: R Status = 0x%x\r\n", __func__,status);
	for (edma_ch = 0; edma_ch < EDMAC_CHAN; edma_ch ++) {
		event = 0;
		if (status & (0x1 << edma_ch)) {
			event = DMA_FINISH;
		}
		if ((status >> 16) & (0x1 << edma_ch)) {
			event = DMA_ABORT;
		}

		if (event) {
			info->read_chan[edma_ch].wait_event = event;
			info->read_chan[edma_ch].wakeup_jiffies = 0;//gm_jiffies;
			wake_up(&info->read_chan[edma_ch].wait_queue);
			match = 1;

			break;
		}
	}
	//printk("%s: test1\r\n", __func__);
	if (match) {
		return IRQ_HANDLED;
	}
	//printk("%s: test2\r\n", __func__);
	/* Write, DMA_WRITE_INT_STATUS_OFF */
	//status = ioread32(edmac_vbase + 0x4C);
	status = PCIE_GETDBI(nvtpcie_edma_chip_ptr, DMA_WRITE_INT_STATUS_OFF_OFS);
	/* clear status, DMA_WRITE_INT_CLEAR_OFF */
	//iowrite32(status, edmac_vbase + 0x58);
	PCIE_SETDBI(nvtpcie_edma_chip_ptr, DMA_WRITE_INT_CLEAR_OFF_OFS, status);
	//printk("%s: W Status = 0x%x\r\n", __func__,status);
	for (edma_ch = 0; edma_ch < EDMAC_CHAN; edma_ch ++) {
		event = 0;
		//printk("%s: edma_ch = 0x%x\r\n", __func__,edma_ch);
		if (status & (0x1 << edma_ch)) {
			event = DMA_FINISH;
		}
		if ((status >> 16) & (0x1 << edma_ch)) {
			event = DMA_ABORT;
		}

		if (event) {
			info->write_chan[edma_ch].wait_event = event;
			info->write_chan[edma_ch].wakeup_jiffies = 0;//gm_jiffies;
			wake_up(&info->write_chan[edma_ch].wait_queue);
			match = 1;
			break;
		}
	}

	return IRQ_HANDLED;
}
#endif


//int nvt_pcie_edma_init(void *pcie_dbi_vbase, int pcie_id, pcie_num_t my_role)
int nvt_pcie_edma_init(struct nvt_pcie_chip *chip_ptr, void *pcie_dbi_vbase, int pcie_id)
{
	edmac_info_t *info;
#if PCIE_EDMA_INT_EN
	int i;
#endif

	nvtpcie_edma_chip_ptr = chip_ptr;
	gPcie_id = pcie_id;

	memset(&edmac_info[pcie_id], 0x0,  sizeof(edmac_info_t));

	info = &edmac_info[pcie_id];
	info->edmac_vbase = nvtpcie_edma_chip_ptr->PCIE_REG_BASE[MEM_PCIE_DBI] + 0x380000;

	DBG_IND("nvt_pcie_edma_init start\r\n");

	sema_init(&info->edmac_sema, 1);
	sema_init(&info->read_chan_sema, EDMAC_CHAN);
	sema_init(&info->write_chan_sema, EDMAC_CHAN);

#if PCIE_EDMA_INT_EN
	DBG_IND("init_waitqueue_head\r\n");
	for (i = 0; i < EDMAC_CHAN; i ++) {
		init_waitqueue_head(&info->read_chan[i].wait_queue);
		init_waitqueue_head(&info->write_chan[i].wait_queue);
	}
#endif

	DBG_IND("nvt_pcie_edma_init end\r\n");
	return 0;
}

#if 0
static int get_ctrl_outcid(nvtpcie_chipid_t target_id)
{
	if (pcie_role == CHIP_RC) {
		if (target_id == CHIP_EP0) {
			return 0;    /* ctrl-0 */
		}

		/* ctrl-1 */
		return 1;
	}

	/* ctrl-1 */
	return 1;
}
#endif


/* allocate channel will decrease one semaphore. semaphore is returned in free chan.
 */
static int alloc_edmac_chan(int cid, int direction)
{
	struct edmac_info_s *info = &edmac_info[cid];
	int i, chan = -1;

	/* get available channel */
	if ((direction != EDMAC_READ) && (direction != EDMAC_WRITE)) {
		printk("%s, error direction:%d \n", __func__, direction);
		return -1;
	}

	/* get one channel semaphore first */
	(direction == EDMAC_READ) ? CHAN_READ_SEMA_LOCK(cid) : CHAN_WRITE_SEMA_LOCK(cid);

	/* lock database and get free one */
	ENGINE_SEMA_LOCK(cid);

	for (i = 0; i < EDMAC_CHAN; i ++) {
		if (direction == EDMAC_READ) {
			if (info->read_chan[i].busy == 1) {
				continue;
			}

			info->read_chan[i].busy = 1;
			info->read_chan[i].read_count ++;
			chan = i;
			break;
		} else {
			if (info->write_chan[i].busy == 1) {
				continue;
			}

			info->write_chan[i].busy = 1;
			info->write_chan[i].write_count ++;
			chan = i;
			break;
		}
	}
	ENGINE_SEMA_UNLOCK(cid);

	if (chan == -1) {
		panic("bug in dir:%d pcie dma channel management! \n", direction);
	}

	/* can't do CHAN_SEMA_UNLOCK. It is done in free channel */

	return chan;
}

static void free_edmac_chan(int cid, int ch, int direction)
{
	struct edmac_info_s *info = &edmac_info[cid];

	/* get available channel */
	if ((direction != EDMAC_READ) && (direction != EDMAC_WRITE)) {
		printk("%s, error direction: %d \n", __func__, direction);
		return;
	}

	/* lock database and get free one */
	ENGINE_SEMA_LOCK(cid);

	if (direction == EDMAC_READ) {
		if (info->read_chan[ch].busy == 1) {
			info->read_chan[ch].busy = 0;
		} else {
			panic("bug in pcie read dma channel management!!! \n");
		}
	} else {
		if (info->write_chan[ch].busy == 1) {
			info->write_chan[ch].busy = 0;
		} else {
			panic("bug in pcie read dma channel management!!! \n");
		}
	}

	ENGINE_SEMA_UNLOCK(cid);

	/* release one semaphore */
	(direction == EDMAC_READ) ? CHAN_READ_SEMA_UNLOCK(cid) : CHAN_WRITE_SEMA_UNLOCK(cid);
}

/* Read Transfer On Read Channel#N. Initated by Local CPU(Non LL mode)
 * src_paddr or dst_paddr is through pcie address translation already and 16 bytes alignment.
 * len: bytes. len is 16 bytes alignment
 */
int nvt_pcie_edma_read(nvtpcie_chipid_t tid, phys_addr_t src_paddr, phys_addr_t dst_paddr, u32 len)
{
	T_DMA_READ_ENGINE_EN_OFF read_engine_en = {0};
	T_DMA_CH_CONTROL1_OFF_RDCH ctrl1;
	T_DMA_READ_DOORBELL_OFF doorbell = {0};
	T_DMA_TRANSFER_SIZE_OFF_RDCH tx_size;
	T_DMA_SAR_LOW_OFF_RDCH sar_low;
	T_DMA_SAR_HIGH_OFF_RDCH sar_high;
	T_DMA_DAR_LOW_OFF_RDCH dar_low;
	T_DMA_DAR_HIGH_OFF_RDCH dar_high;

	int ch = 0;
	UINT32 reg;
	int cid;
#if PCIE_EDMA_INT_EN
	int status;
	struct edmac_info_s *info;
#endif

	if (tid) {
		cid = 0;
	} else {
		cid = 1;
	}

#if PCIE_EDMA_INT_EN
	info = &edmac_info[cid];
#endif

	DBG_IND("nvtpcie_edma_read(TID = %d, CH =0x%d)\r\n",    tid, ch);
	ch = alloc_edmac_chan((int)cid, EDMAC_READ);

	//const SYS_PA pa = sys_va2pa(buf_addr);

	//if (ch >= PCIE_EDMA_CH_COUNT) {
	//  DBG_ERR("input ch %d invalid\r\n");
	//  return -1;
	//}

	// invalidate cache
	//dma_flushReadCache(buf_addr, len);

	// DMA Read Engine Enable
	read_engine_en.bit.DMA_READ_ENGINE = 1;
	PCIE_SETDBI(nvtpcie_edma_chip_ptr, DMA_READ_ENGINE_EN_OFF_OFS, read_engine_en.reg);

	// DMA Read Interrupt Mask

	// DMA Channel Control 1 register
	ctrl1.reg = PCIE_GETDBI(nvtpcie_edma_chip_ptr, DMA_CH_CONTROL1_OFF_RDCH_OFS(ch));
	ctrl1.bit.LIE = 1;
	PCIE_SETDBI(nvtpcie_edma_chip_ptr, DMA_CH_CONTROL1_OFF_RDCH_OFS(ch), ctrl1.reg);

	// DMA Transfer Size
	tx_size.bit.DMA_TRANSFER_SIZE = len;
	PCIE_SETDBI(nvtpcie_edma_chip_ptr, DMA_TRANSFER_SIZE_OFF_RDCH_OFS(ch), tx_size.reg);
	DBG_IND("len 0x%x\r\n", (int)len);
	DBG_IND("read len 0x%x\r\n", (int)PCIE_GETDBI(nvtpcie_edma_chip_ptr, DMA_TRANSFER_SIZE_OFF_RDCH_OFS(ch)));

	// DMA SAR Low/High (source: remote pcie address)
	sar_low.bit.SRC_ADDR_REG_LOW = src_paddr & 0xFFFFFFFF;
	PCIE_SETDBI(nvtpcie_edma_chip_ptr, DMA_SAR_LOW_OFF_RDCH_OFS(ch), sar_low.reg);
	sar_high.bit.SRC_ADDR_REG_HIGH = src_paddr >> 32;
	PCIE_SETDBI(nvtpcie_edma_chip_ptr, DMA_SAR_HIGH_OFF_RDCH_OFS(ch), sar_high.reg);
	DBG_IND("remote pcie addr 0x%llx\r\n", src_paddr);

	// DMA DAR Low/Hihg (destination: local system address)
	dar_low.bit.DST_ADDR_REG_LOW = dst_paddr & 0xFFFFFFFF;
	PCIE_SETDBI(nvtpcie_edma_chip_ptr, DMA_DAR_LOW_OFF_RDCH_OFS(ch), dar_low.reg);
	dar_high.bit.DST_ADDR_REG_HIGH = dst_paddr >> 32;
	PCIE_SETDBI(nvtpcie_edma_chip_ptr, DMA_DAR_HIGH_OFF_RDCH_OFS(ch), dar_high.reg);
	DBG_IND("local phy addr 0x%llx\r\n", dst_paddr);
#if PCIE_EDMA_INT_EN
	//DBG_IND("INT sts1 0x%x\r\n", (int)PCIE_GETDBI(nvtpcie_edma_chip_ptr, DMA_READ_INT_STATUS_OFF_OFS));
	//PCIE_SETDBI(nvtpcie_edma_chip_ptr, DMA_READ_INT_STATUS_OFF_OFS, 0x0);
	//DBG_IND("INT sts2 0x%x\r\n", (int)PCIE_GETDBI(nvtpcie_edma_chip_ptr, DMA_READ_INT_STATUS_OFF_OFS));
	PCIE_SETDBI(nvtpcie_edma_chip_ptr, DMA_READ_INT_MASK_OFF_OFS, 0x0);
	info->read_chan[ch].wait_jiffes = 0;//get_gm_jiffies;
	info->read_chan[ch].wakeup_jiffies = 0;
	info->read_chan[ch].wait_event = DMA_IN_PROCESS;
#endif

	// DMA Read Doorbell
	doorbell.bit.RD_DOORBELL_NUM = ch;
	PCIE_SETDBI(nvtpcie_edma_chip_ptr, DMA_READ_DOORBELL_OFF_OFS, doorbell.reg);

#if PCIE_EDMA_INT_EN
	//info = &edmac_info[cid];
//#if ENABLE_WAIT_FLG
//	pcie_platform_flg_wait(FLGPTN_EDMA_READ);
//#endif
	DBG_IND("wait TO \n");
	status = wait_event_timeout(info->read_chan[ch].wait_queue,
								info->read_chan[ch].wait_event != DMA_IN_PROCESS, 500000);
	DBG_IND("wait done \n");
	if ((status == 0) || (info->read_chan[ch].wait_event == DMA_ABORT)) {
		/* dump register for debugging */
		if (status == 0) {
			DBG_ERR("edma_read timeout!!! \n");
		} else {
			DBG_ERR("edma_read abort!!! \n");
		}
	}
#endif
	DBG_IND("free1 \n");
	free_edmac_chan((int)cid, ch, EDMAC_READ);
	DBG_IND("free 2\n");

#if !PCIE_EDMA_INT_EN
	while (1) {
		reg = PCIE_GETDBI(nvtpcie_edma_chip_ptr, DMA_READ_INT_STATUS_OFF_OFS);
		if (reg & (1 << ch)) {
			DBG_IND("READ done 0x%x\r\n", reg);
			PCIE_SETDBI(nvtpcie_edma_chip_ptr, DMA_READ_INT_CLEAR_OFF_OFS, reg);
			break;
		}
	}
#endif

	return 0;

}

int nvt_pcie_edma_read_ep(nvtpcie_chipid_t tid, phys_addr_t src_paddr, phys_addr_t dst_paddr, u32 len)
{
	DBG_ERR("nvt_pcie_edma_read_ep - not supported !\n");
	return 0;
}

/* Write Transfer On Write Channel#N. Initated by Local CPU(Non LL mode)
 * src_paddr or dst_paddr is through pcie address translation already and 16 bytes alignment.
 * len: bytes. len is 16 bytes alignment
 */
int nvt_pcie_edma_write(nvtpcie_chipid_t tid, phys_addr_t src_paddr, phys_addr_t dst_paddr, u32 len)
{
	UINT32 reg;
	T_DMA_WRITE_ENGINE_EN_OFF write_engine_en = {0};
	T_DMA_CH_CONTROL1_OFF_WRCH ctrl1;
	T_DMA_WRITE_DOORBELL_OFF doorbell = {0};
	T_DMA_TRANSFER_SIZE_OFF_WRCH tx_size;
#if 0
	T_DMA_WRITE_DONE_IMWR_LOW_OFF imwr_low;
	T_DMA_WRITE_DONE_IMWR_HIGH_OFF imwr_high;
#endif
	T_DMA_SAR_LOW_OFF_WRCH sar_low;
	T_DMA_SAR_HIGH_OFF_WRCH sar_high;
	T_DMA_DAR_LOW_OFF_WRCH dar_low;
	T_DMA_DAR_HIGH_OFF_WRCH dar_high;
#if PCIE_EDMA_INT_EN
	int status;
	struct edmac_info_s *info;
#endif
	int cid;

	int ch = 0;
	//const SYS_PA pa = sys_va2pa(buf_addr);

	//if (ch >= PCIE_EDMA_CH_COUNT) {
	//  DBG_ERR("input ch %d invalid\r\n");
	//  return -1;
	//}
	if (tid) {
		cid = 0;
	} else {
		cid = 1;
	}

#if PCIE_EDMA_INT_EN
	info = &edmac_info[cid];
#endif

	DBG_IND("nvtpcie_edma_write(TID = %d, CH =0x%d)\r\n",   tid, ch);
	ch = alloc_edmac_chan((int)cid, EDMAC_WRITE);

	// invalidate cache
	//dma_flushWriteCache(buf_addr, len);

	// DMA Write Engine Enable
	write_engine_en.bit.DMA_WRITE_ENGINE = 1;
	PCIE_SETDBI(nvtpcie_edma_chip_ptr, DMA_WRITE_ENGINE_EN_OFF_OFS, write_engine_en.reg);

#if 0
	// DMA Write Done IMWr Address Low and High
	imwr_low.bit.DMA_WRITE_DONE_LOW_REG;
	PCIE_SETDBI(DMA_WRITE_DONE_IMWR_LOW_OFF_OFS, imwr_low.reg);
	imwr_high.bit.DMA_WRITE_DONE_HIGH_REG;
	PCIE_SETDBI(DMA_WRITE_DONE_IMWR_HIGH_OFF_OFS, imwr_high.reg);
#endif
#if 0
	// DMA Write Channel IMWr Data
	reg = PCIE_GETDBI(nvtpcie_edma_chip_ptr, DMA_WRITE_CH01_IMWR_DATA_OFF_OFS + (ch / 2) * 4);
	if (ch & 0x01) {
		reg &= ~(0xFFFF << 16);
		reg |= ch << 16;
	} else {
		reg &= ~0xFFFF;
		reg |= ch;
	}
	PCIE_SETDBI(nvtpcie_edma_chip_ptr, DMA_WRITE_CH01_IMWR_DATA_OFF_OFS + (ch / 2) * 4, reg);
#endif
	// DMA Write Interrupt Mask

	// DMA Channel Control 1 register
	ctrl1.reg = PCIE_GETDBI(nvtpcie_edma_chip_ptr, DMA_CH_CONTROL1_OFF_WRCH_OFS(ch));
	ctrl1.bit.LIE = 1;
	PCIE_SETDBI(nvtpcie_edma_chip_ptr, DMA_CH_CONTROL1_OFF_WRCH_OFS(ch), ctrl1.reg);

	// DMA Transfer Size
	tx_size.bit.DMA_TRANSFER_SIZE = len;
	PCIE_SETDBI(nvtpcie_edma_chip_ptr, DMA_TRANSFER_SIZE_OFF_WRCH_OFS(ch), tx_size.reg);
	DBG_IND("len 0x%x\r\n", (int)len);
	DBG_IND("read len 0x%x\r\n", (int)PCIE_GETDBI(nvtpcie_edma_chip_ptr, DMA_TRANSFER_SIZE_OFF_WRCH_OFS(ch)));

	// DMA SAR Low/High (source: local sytem address)
	sar_low.bit.SRC_ADDR_REG_LOW = src_paddr & 0xFFFFFFFF;
	PCIE_SETDBI(nvtpcie_edma_chip_ptr, DMA_SAR_LOW_OFF_WRCH_OFS(ch), sar_low.reg);
	sar_high.bit.SRC_ADDR_REG_HIGH = src_paddr >> 32;
	PCIE_SETDBI(nvtpcie_edma_chip_ptr, DMA_SAR_HIGH_OFF_WRCH_OFS(ch), sar_high.reg);
	DBG_IND("local phy addr 0x%llx\r\n", src_paddr);

	// DMA DAR Low/Hihg (destination: remote pcie address)
	dar_low.bit.DST_ADDR_REG_LOW = dst_paddr & 0xFFFFFFFF;
	PCIE_SETDBI(nvtpcie_edma_chip_ptr, DMA_DAR_LOW_OFF_WRCH_OFS(ch), dar_low.reg);
	dar_high.bit.DST_ADDR_REG_HIGH = dst_paddr >> 32;
	PCIE_SETDBI(nvtpcie_edma_chip_ptr, DMA_DAR_HIGH_OFF_WRCH_OFS(ch), dar_high.reg);
	DBG_IND("remote pcie addr 0x%llx\r\n", dst_paddr);
#if PCIE_EDMA_INT_EN
	//PCIE_SETDBI(nvtpcie_edma_chip_ptr, DMA_WRITE_INT_STATUS_OFF_OFS, 0x0);
	PCIE_SETDBI(nvtpcie_edma_chip_ptr, DMA_WRITE_INT_MASK_OFF_OFS, 0x0);
#endif
	info->write_chan[ch].wait_jiffes = 0;//get_gm_jiffies;
	info->write_chan[ch].wakeup_jiffies = 0;
	info->write_chan[ch].wait_event = DMA_IN_PROCESS;

	// DMA Write Doorbell
	doorbell.bit.WR_DOORBELL_NUM = ch;
	PCIE_SETDBI(nvtpcie_edma_chip_ptr, DMA_WRITE_DOORBELL_OFF_OFS, doorbell.reg);

	//Interrupt Mask (0x54)
	//PCIE_SETDBI(nvtpcie_edma_chip_ptr,DMA_WRITE_INT_MASK_OFF_OFS, 1);

//#if ENABLE_WAIT_FLG
//	pcie_platform_flg_wait(FLGPTN_EDMA_WRITE);
//#endif

#if PCIE_EDMA_INT_EN
	//info = &edmac_info[cid];
	DBG_IND("wait TO \n");
	status = wait_event_timeout(info->write_chan[ch].wait_queue,
								info->write_chan[ch].wait_event != DMA_IN_PROCESS, 500000);
	DBG_IND("wait done \n");
	if ((status == 0) || (info->write_chan[ch].wait_event == DMA_ABORT)) {
		/* dump register for debugging */
		if (status == 0) {
			DBG_ERR("edma_write timeout!!! \n");
		} else {
			DBG_ERR("edma_write abort!!! \n");
		}
	}
#endif
	DBG_IND("free1 \n");
	free_edmac_chan((int)cid, ch, EDMAC_WRITE);
	DBG_IND("free2 \n");
#if !PCIE_EDMA_INT_EN
	while (1) {
		reg = PCIE_GETDBI(nvtpcie_edma_chip_ptr, DMA_WRITE_INT_STATUS_OFF_OFS);
		if (reg & (1 << ch)) {
			DBG_IND("write done 0x%x\r\n", reg);
			PCIE_SETDBI(nvtpcie_edma_chip_ptr, DMA_WRITE_INT_CLEAR_OFF_OFS, reg);
			break;
		}
	}
#endif

	return 0;
}

int nvt_pcie_edma_write_ep(nvtpcie_chipid_t tid, phys_addr_t src_paddr, phys_addr_t dst_paddr, u32 len)
{
	DBG_ERR("nvt_pcie_edma_write_ep - not supported !\n");
	return 0;
}
