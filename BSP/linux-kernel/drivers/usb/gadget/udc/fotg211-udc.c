// SPDX-License-Identifier: GPL-2.0
/*
 * FOTG210 UDC Driver supports Bulk transfer so far
 *
 * Copyright (C) 2013 Faraday Technology Corporation
 *
 * Author : Yuan-Hsin Chen <yhchen@faraday-tech.com>
 */

#include <linux/dma-mapping.h>
#include <linux/err.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/usb/ch9.h>
#include <linux/usb/gadget.h>
#include <linux/of.h>
#include <linux/delay.h>
#include <plat/efuse_protected.h>
#include <linux/gpio.h>
#include <plat/nvt-gpio.h>
#include <plat/top.h>

#include "fotg211.h"
#include <linux/usb/video.h>
#include "fotg211-EP.h"
#include <linux/of_address.h>
#include <linux/usb/cdc.h>
#include <linux/clk.h>

#define DRIVER_DESC       "FOTG211 USB Device Controller Driver"
#define DRIVER_VERSION    "1.00.019"

static bool is_cdc_class = 0;
u32 vbus_gpio_no = D_GPIO(7);
static const char udc_name[] = "fotg211_udc";
//static const char * const fotg210_ep_name[] = {
//	"ep0", "ep1", "ep2", "ep3", "ep4"};

static const struct {
	const char *name;
	const struct usb_ep_caps caps;
} ep_info[] = {
#define EP_INFO(_name, _caps) \
	{ \
		.name = _name, \
		.caps = _caps, \
	}

	EP_INFO("ep0",
		USB_EP_CAPS(USB_EP_CAPS_TYPE_CONTROL, USB_EP_CAPS_DIR_ALL)),
	EP_INFO("ep1",
		USB_EP_CAPS(USB_EP_CAPS_TYPE_ALL, USB_EP_CAPS_DIR_ALL)),
	EP_INFO("ep2",
		USB_EP_CAPS(USB_EP_CAPS_TYPE_ALL, USB_EP_CAPS_DIR_ALL)),
	EP_INFO("ep3",
		USB_EP_CAPS(USB_EP_CAPS_TYPE_ALL, USB_EP_CAPS_DIR_ALL)),
	EP_INFO("ep4",
		USB_EP_CAPS(USB_EP_CAPS_TYPE_ALL, USB_EP_CAPS_DIR_ALL)),
	EP_INFO("ep5",
		USB_EP_CAPS(USB_EP_CAPS_TYPE_ALL, USB_EP_CAPS_DIR_ALL)),
	EP_INFO("ep6",
		USB_EP_CAPS(USB_EP_CAPS_TYPE_ALL, USB_EP_CAPS_DIR_ALL)),
	EP_INFO("ep7",
		USB_EP_CAPS(USB_EP_CAPS_TYPE_ALL, USB_EP_CAPS_DIR_ALL)),
	EP_INFO("ep8",
		USB_EP_CAPS(USB_EP_CAPS_TYPE_ALL, USB_EP_CAPS_DIR_ALL)),
	EP_INFO("ep9",
		USB_EP_CAPS(USB_EP_CAPS_TYPE_ALL, USB_EP_CAPS_DIR_ALL)),
	EP_INFO("ep10",
		USB_EP_CAPS(USB_EP_CAPS_TYPE_ALL, USB_EP_CAPS_DIR_ALL)),
	EP_INFO("ep11",
		USB_EP_CAPS(USB_EP_CAPS_TYPE_ALL, USB_EP_CAPS_DIR_ALL)),
	EP_INFO("ep12",
		USB_EP_CAPS(USB_EP_CAPS_TYPE_ALL, USB_EP_CAPS_DIR_ALL)),
	EP_INFO("ep13",
		USB_EP_CAPS(USB_EP_CAPS_TYPE_ALL, USB_EP_CAPS_DIR_ALL)),
	EP_INFO("ep14",
		USB_EP_CAPS(USB_EP_CAPS_TYPE_ALL, USB_EP_CAPS_DIR_ALL)),
	EP_INFO("ep15",
		USB_EP_CAPS(USB_EP_CAPS_TYPE_ALL, USB_EP_CAPS_DIR_ALL)),
#undef EP_INFO
};

unsigned int outslice = 0;
module_param_named(outslice, outslice, int, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(outslice, "Fot class(CDC-NCM) out size is unknown, the use of short packet to terminate out");

static unsigned debug_on = 0;
module_param(debug_on, uint, S_IRUGO);
MODULE_PARM_DESC(debug_on, "Whether to enable debug message. Default = 0");

static unsigned plugout_delay = 50;
module_param(plugout_delay, uint, S_IRUGO);
MODULE_PARM_DESC(plugout_delay, "plugout delay. Default = 50");


static int do_test_packet = 0;

#define numsg		pr_info
#define itfnumsg	pr_info
#define devnumsg	pr_info
#define ep0numsg	pr_info
#define fifonumsg	pr_info

static int fotg210_cable_out_clear(struct fotg210_ep *ep)
{
	u32 value, iep_length;
	u32 dma_start;
	int length = 0;

	pr_info("cable_out\n");
	pr_info("ep is %d, fifo is %d\n", ep->epnum, gEPMap[ep->epnum - 1]);

	value = ioread32(ep->fotg210->reg + FOTG210_DMACPSR1);
	value |= DMACPSR1_DMA_ABORT;
	iowrite32(value, ep->fotg210->reg + FOTG210_DMACPSR1);

	if (ep->dir_in) {
		iep_length = ioread32(ep->fotg210->reg + FOTG210_INEPMPSR(ep->epnum));
		/* reset fifo */
		do {
			pr_info("IN case, remaing DMA len is 0x%x\n", ioread32(ep->fotg210->reg + FOTG210_DMA_RM_LEN));

			dma_start = ioread32(ep->fotg210->reg + FOTG210_DMACPSR1);

			if (ep->epnum) {
				value = ioread32(ep->fotg210->reg +
						FOTG210_FIBCR(gEPMap[ep->epnum - 1]));

				length = value&FIBCR_BCFX;

				pr_info("FIFO lenght = 0x%x, ep max length is 0x%x\n", length, iep_length);
				value = value|FIBCR_FFRST;
				iowrite32(value, ep->fotg210->reg +
						FOTG210_FIBCR(gEPMap[ep->epnum - 1]));
			} else {
				value = ioread32(ep->fotg210->reg + FOTG210_DCFESR);
				value |= DCFESR_CX_CLR;
				iowrite32(value, ep->fotg210->reg + FOTG210_DCFESR);
			}
		} while (dma_start & 0x1);
	} else {
		do {
			dma_start = ioread32(ep->fotg210->reg + FOTG210_DMACPSR1);
			pr_info("OUT case, wait dma_start clear\n");
		} while (dma_start & 0x1);
	}
	pr_info("cable_out pass\n");

	value = ioread32(ep->fotg210->reg + FOTG210_DMACPSR1);
	value &= ~DMACPSR1_DMA_ABORT;
	iowrite32(value, ep->fotg210->reg + FOTG210_DMACPSR1);

	return 1;
}

static void fotg210_disable_fifo_int(struct fotg210_ep *ep)
{
	u32 value;

	if (ep->dir_in)
	{
		value = ioread32(ep->fotg210->reg + FOTG210_DMISGR1);
		value |= DMISGR1_MF_IN_INT(gEPMap[ep->epnum - 1]);

		iowrite32(value, ep->fotg210->reg + FOTG210_DMISGR1);
	}
	else
	{
		if(gEPMap[ep->epnum - 1] < 8)
			value = ioread32(ep->fotg210->reg + FOTG210_DMISGR1);
		else
			value = ioread32(ep->fotg210->reg + FOTG210_DMISGR2);

		value |= DMISGR1_MF_OUTSPK_INT(gEPMap[ep->epnum - 1]);

		if(gEPMap[ep->epnum - 1] < 8)
			iowrite32(value, ep->fotg210->reg + FOTG210_DMISGR1);
		else
			iowrite32(value, ep->fotg210->reg + FOTG210_DMISGR2);
	}
}

static void fotg210_enable_fifo_int(struct fotg210_ep *ep)
{
	u32 value;

	if (ep->dir_in)
	{
		value = ioread32(ep->fotg210->reg + FOTG210_DMISGR1);
		value &= ~DMISGR1_MF_IN_INT(gEPMap[ep->epnum - 1]);
		iowrite32(value, ep->fotg210->reg + FOTG210_DMISGR1);
	}
	else
	{
		if(gEPMap[ep->epnum - 1] < 8)
			value = ioread32(ep->fotg210->reg + FOTG210_DMISGR1);
		else
			value = ioread32(ep->fotg210->reg + FOTG210_DMISGR2);

		value &= ~DMISGR1_MF_OUTSPK_INT(gEPMap[ep->epnum - 1]);

		if(gEPMap[ep->epnum - 1] < 8)
			iowrite32(value, ep->fotg210->reg + FOTG210_DMISGR1);
		else
			iowrite32(value, ep->fotg210->reg + FOTG210_DMISGR2);
	}
}

static void fotg210_set_cxdone(struct fotg210_udc *fotg210)
{
	u32 value = ioread32(fotg210->reg + FOTG210_DCFESR);

	value |= DCFESR_CX_DONE;
	iowrite32(value, fotg210->reg + FOTG210_DCFESR);
}

static void fotg210_done(struct fotg210_ep *ep, struct fotg210_request *req,
			int status)
{
	if (debug_on)
		numsg("fotg210_done\n");

	list_del_init(&req->queue);

	/* don't modify queue heads during completion callback */
	if (ep->fotg210->gadget.speed == USB_SPEED_UNKNOWN)
		req->req.status = -ESHUTDOWN;
	else
		req->req.status = status;

	if(req->req.complete != NULL) {
		spin_unlock(&ep->fotg210->lock);
		usb_gadget_giveback_request(&ep->ep, &req->req);
		spin_lock(&ep->fotg210->lock);
	}

	if (ep->epnum) {
		if (list_empty(&ep->queue))
			fotg210_disable_fifo_int(ep);
	} else {
		fotg210_set_cxdone(ep->fotg210);
	}
}

static void USB_setFIFOCfg(struct fotg210_ep *ep, USB_FIFO_NUM FIFOn, USB_EP_TYPE BLK_TYP, USB_EP_BLKNUM BLKNO, BOOL BLKSZ, USB_FIFO_DIR Dir)
{
#ifdef PRJ_FIFONUM_16
	struct fotg210_udc *fotg210 = ep->fotg210;
	u32 fifocfg = ioread32(fotg210->reg + FOTG210_FIFOCF_03);
	u32 fifocfg_dir = ioread32(fotg210->reg + FOTG210_FIFOMAP_03);

	u32 fifocfg2 = ioread32(fotg210->reg + FOTG210_FIFOCF_47);
	u32 fifocfg2_dir = ioread32(fotg210->reg + FOTG210_FIFOMAP_47);

	u32 fifocfg3 = ioread32(fotg210->reg + FOTG210_FIFOCF_811);
	u32 fifocfg3_dir = ioread32(fotg210->reg + FOTG210_FIFOMAP_811);

	u32 fifocfg4 = ioread32(fotg210->reg + FOTG210_FIFOCF_1215);
        u32 fifocfg4_dir = ioread32(fotg210->reg + FOTG210_FIFOMAP_1215);

	switch (FIFOn) {
	case USB_FIFO0: {
		if (!((BLKNO == BLKNUM_TRIPLE) && BLKSZ && (ep->type == USB_ENDPOINT_XFER_ISOC))) {
			fifocfg |= FIFOCF_TYPE_NVT(BLK_TYP, FIFOn);
			fifocfg |= FIFOCF_BLK_NO(BLKNO-1, FIFOn);
			fifocfg |= FIFOCF_BLKSZ_NVT(BLKSZ, FIFOn);
			fifocfg_dir |= FIFOMAP_DIR(Dir, FIFOn);
			fifocfg_dir &= ~FIFOMAP_EPNOMSK_NVT(FIFOn); // fifomap
			fifocfg_dir |= FIFOMAP_EPNO_NVT(ep->epnum, FIFOn);
			fifocfg |= FIFOCF_FIFO_EN_NVT(1, FIFOn);


			if ((BLKNO == BLKNUM_SINGLE) && (BLKSZ == 1)) {
				fifocfg |= FIFOCF_TYPE_NVT(BLK_TYP, FIFOn+1);
				fifocfg |= FIFOCF_BLK_NO(BLKNO-1, FIFOn+1);
				fifocfg |= FIFOCF_BLKSZ_NVT(BLKSZ, FIFOn+1);
				fifocfg |= FIFOCF_FIFO_EN_NVT(0, FIFOn+1);

			} else if (BLKNO == BLKNUM_DOUBLE) {
				fifocfg |= FIFOCF_TYPE_NVT(BLK_TYP, FIFOn+1);
				fifocfg |= FIFOCF_BLK_NO(BLKNO-1, FIFOn+1);
				fifocfg |= FIFOCF_BLKSZ_NVT(BLKSZ, FIFOn+1);
				fifocfg |= FIFOCF_FIFO_EN_NVT(0, FIFOn+1);

				if (BLKSZ == 1) {
					// Block size : 512~1024 bytes
					fifocfg |= FIFOCF_TYPE_NVT(BLK_TYP, FIFOn+2);
					fifocfg |= FIFOCF_BLK_NO(BLKNO-1, FIFOn+2);
					fifocfg |= FIFOCF_BLKSZ_NVT(BLKSZ, FIFOn+2);
					fifocfg |= FIFOCF_FIFO_EN_NVT(0, FIFOn+2);

					fifocfg |= FIFOCF_TYPE_NVT(BLK_TYP, FIFOn+3);
					fifocfg |= FIFOCF_BLK_NO(BLKNO-1, FIFOn+3);
					fifocfg |= FIFOCF_BLKSZ_NVT(BLKSZ, FIFOn+3);
					fifocfg |= FIFOCF_FIFO_EN_NVT(0, FIFOn+3);
				}

			} else if (BLKNO == BLKNUM_TRIPLE) {
				fifocfg |= FIFOCF_TYPE_NVT(BLK_TYP, FIFOn+1);
				fifocfg |= FIFOCF_BLK_NO(BLKNO-1, FIFOn+1);
				fifocfg |= FIFOCF_BLKSZ_NVT(BLKSZ, FIFOn+1);
				fifocfg |= FIFOCF_FIFO_EN_NVT(0, FIFOn+1);

				fifocfg |= FIFOCF_TYPE_NVT(BLK_TYP, FIFOn+2);
				fifocfg |= FIFOCF_BLK_NO(BLKNO-1, FIFOn+2);
				fifocfg |= FIFOCF_BLKSZ_NVT(BLKSZ, FIFOn+2);
				fifocfg |= FIFOCF_FIFO_EN_NVT(0, FIFOn+2);
			}
		} else {
			// Triplex1024: ping pong trigger.
			fifocfg |= FIFOCF_TYPE_NVT(BLK_TYP, FIFOn);
			fifocfg |= FIFOCF_BLK_NO(BLKNO-2, FIFOn);
			fifocfg |= FIFOCF_BLKSZ_NVT(BLKSZ, FIFOn);
			fifocfg_dir |= FIFOMAP_DIR(Dir, FIFOn);
			fifocfg_dir &= ~FIFOMAP_EPNOMSK_NVT(FIFOn); // fifomap
			fifocfg_dir |= FIFOMAP_EPNO_NVT(ep->epnum, FIFOn);
			fifocfg |= FIFOCF_FIFO_EN_NVT(1, FIFOn);
			fifocfg	|= FIFOCF_FIFO_PP_EN(1, FIFOn);

			fifocfg |= FIFOCF_TYPE_NVT(BLK_TYP, FIFOn+1);
			fifocfg |= FIFOCF_BLK_NO(BLKNO-2, FIFOn+1);
			fifocfg |= FIFOCF_BLKSZ_NVT(BLKSZ, FIFOn+1);
			fifocfg |= FIFOCF_FIFO_EN_NVT(0, FIFOn+1);
			fifocfg |= FIFOCF_FIFO_PP_EN(1, FIFOn+1);

			fifocfg |= FIFOCF_TYPE_NVT(BLK_TYP, FIFOn+2);
			fifocfg |= FIFOCF_BLK_NO(BLKNO-2, FIFOn+2);
			fifocfg |= FIFOCF_BLKSZ_NVT(BLKSZ, FIFOn+2);
			fifocfg |= FIFOCF_FIFO_EN_NVT(0, FIFOn+2);
			fifocfg |= FIFOCF_FIFO_PP_EN(1, FIFOn+2);

			fifocfg |= FIFOCF_TYPE_NVT(BLK_TYP, FIFOn+3);
			fifocfg |= FIFOCF_BLK_NO(BLKNO-2, FIFOn+3);
			fifocfg |= FIFOCF_BLKSZ_NVT(BLKSZ, FIFOn+3);
			fifocfg |= FIFOCF_FIFO_EN_NVT(0, FIFOn+3);
			fifocfg |= FIFOCF_FIFO_PP_EN(1, FIFOn+3);

			//fifo4~fifo7
			fifocfg2 |= FIFOCF_TYPE_NVT(BLK_TYP, FIFOn+4);
			fifocfg2 |= FIFOCF_BLK_NO(BLKNO-2, FIFOn+4);
			fifocfg2 |= FIFOCF_BLKSZ_NVT(BLKSZ, FIFOn+4);
			fifocfg2 |= FIFOCF_FIFO_EN_NVT(0, FIFOn+4);
			fifocfg2 |= FIFOCF_FIFO_PP_EN(1, FIFOn+4);


			fifocfg2 |= FIFOCF_TYPE_NVT(BLK_TYP, FIFOn+5);
			fifocfg2 |= FIFOCF_BLK_NO(BLKNO-2, FIFOn+5);
			fifocfg2 |= FIFOCF_BLKSZ_NVT(BLKSZ, FIFOn+5);
			fifocfg2 |= FIFOCF_FIFO_EN_NVT(0, FIFOn+5);
			fifocfg2 |= FIFOCF_FIFO_PP_EN(1, FIFOn+5);


			fifocfg2 |= FIFOCF_TYPE_NVT(BLK_TYP, FIFOn+6);
			fifocfg2 |= FIFOCF_BLK_NO(BLKNO-2, FIFOn+6);
			fifocfg2 |= FIFOCF_BLKSZ_NVT(BLKSZ, FIFOn+6);
			fifocfg2 |= FIFOCF_FIFO_EN_NVT(0, FIFOn+6);
			fifocfg2 |= FIFOCF_FIFO_PP_EN(1, FIFOn+6);


			fifocfg2 |= FIFOCF_TYPE_NVT(BLK_TYP, FIFOn+6);
			fifocfg2 |= FIFOCF_BLK_NO(BLKNO-2, FIFOn+6);
			fifocfg2 |= FIFOCF_BLKSZ_NVT(BLKSZ, FIFOn+6);
			fifocfg2 |= FIFOCF_FIFO_EN_NVT(0, FIFOn+6);
			fifocfg2 |= FIFOCF_FIFO_PP_EN(1, FIFOn+6);


			fifocfg2 |= FIFOCF_TYPE_NVT(BLK_TYP, FIFOn+7);
			fifocfg2 |= FIFOCF_BLK_NO(BLKNO-2, FIFOn+7);
			fifocfg2 |= FIFOCF_BLKSZ_NVT(BLKSZ, FIFOn+7);
			fifocfg2 |= FIFOCF_FIFO_EN_NVT(0, FIFOn+7);
			fifocfg2 |= FIFOCF_FIFO_PP_EN(1, FIFOn+7);

			// fifo8~fifo11
			fifocfg3 |= FIFOCF_TYPE_NVT(BLK_TYP, FIFOn+8);
			fifocfg3 |= FIFOCF_BLK_NO(BLKNO-2, FIFOn+8);
			fifocfg3 |= FIFOCF_BLKSZ_NVT(BLKSZ, FIFOn+8);
			fifocfg3 |= FIFOCF_FIFO_EN_NVT(0, FIFOn+8);
			fifocfg3 |= FIFOCF_FIFO_PP_EN(1, FIFOn+8);


			fifocfg3 |= FIFOCF_TYPE_NVT(BLK_TYP, FIFOn+9);
			fifocfg3 |= FIFOCF_BLK_NO(BLKNO-2, FIFOn+9);
			fifocfg3 |= FIFOCF_BLKSZ_NVT(BLKSZ, FIFOn+9);
			fifocfg3 |= FIFOCF_FIFO_EN_NVT(0, FIFOn+9);
			fifocfg3 |= FIFOCF_FIFO_PP_EN(1, FIFOn+9);


			fifocfg3 |= FIFOCF_TYPE_NVT(BLK_TYP, FIFOn+10);
			fifocfg3 |= FIFOCF_BLK_NO(BLKNO-2, FIFOn+10);
			fifocfg3 |= FIFOCF_BLKSZ_NVT(BLKSZ, FIFOn+10);
			fifocfg3 |= FIFOCF_FIFO_EN_NVT(0, FIFOn+10);
			fifocfg3 |= FIFOCF_FIFO_PP_EN(1, FIFOn+10);


			fifocfg3 |= FIFOCF_TYPE_NVT(BLK_TYP, FIFOn+11);
			fifocfg3 |= FIFOCF_BLK_NO(BLKNO-2, FIFOn+11);
			fifocfg3 |= FIFOCF_BLKSZ_NVT(BLKSZ, FIFOn+11);
			fifocfg3 |= FIFOCF_FIFO_EN_NVT(0, FIFOn+11);
			fifocfg3 |= FIFOCF_FIFO_PP_EN(1, FIFOn+11);
		}
	}
	break;

	case USB_FIFO1: {
			fifocfg |= FIFOCF_TYPE_NVT(BLK_TYP, FIFOn);
			fifocfg |= FIFOCF_BLK_NO(BLKNO-1, FIFOn);
			fifocfg |= FIFOCF_BLKSZ_NVT(BLKSZ, FIFOn);
			fifocfg_dir |= FIFOMAP_DIR(Dir, FIFOn);
			fifocfg_dir &= ~FIFOMAP_EPNOMSK_NVT(FIFOn); // fifomap
			fifocfg_dir |= FIFOMAP_EPNO_NVT(ep->epnum, FIFOn);
			fifocfg |= FIFOCF_FIFO_EN_NVT(1, FIFOn);

			if ((BLKNO == BLKNUM_SINGLE) && (BLKSZ == 1)) {
				fifocfg |= FIFOCF_TYPE_NVT(BLK_TYP, FIFOn+1);
				fifocfg |= FIFOCF_BLK_NO(BLKNO-1, FIFOn+1);
				fifocfg |= FIFOCF_BLKSZ_NVT(BLKSZ, FIFOn+1);
				fifocfg |= FIFOCF_FIFO_EN_NVT(0, FIFOn+1);

			} else if (BLKNO == BLKNUM_DOUBLE) {
				fifocfg |= FIFOCF_TYPE_NVT(BLK_TYP, FIFOn+1);
				fifocfg |= FIFOCF_BLK_NO(BLKNO-1, FIFOn+1);
				fifocfg |= FIFOCF_BLKSZ_NVT(BLKSZ, FIFOn+1);
				fifocfg |= FIFOCF_FIFO_EN_NVT(0, FIFOn+1);

				if (BLKSZ == 1) {
					fifocfg |= FIFOCF_TYPE_NVT(BLK_TYP, FIFOn+2);
					fifocfg |= FIFOCF_BLK_NO(BLKNO-1, FIFOn+2);
					fifocfg |= FIFOCF_BLKSZ_NVT(BLKSZ, FIFOn+2);
					fifocfg |= FIFOCF_FIFO_EN_NVT(0, FIFOn+2);

					//FIFO4~
					fifocfg2 |= FIFOCF_TYPE_NVT(BLK_TYP, FIFOn+3);
					fifocfg2 |= FIFOCF_BLK_NO(BLKNO-1, FIFOn+3);
					fifocfg2 |= FIFOCF_BLKSZ_NVT(BLKSZ, FIFOn+3);
					fifocfg2 |= FIFOCF_FIFO_EN_NVT(0, FIFOn+3);
				}
			} else if (BLKNO == BLKNUM_TRIPLE) {
				fifocfg |= FIFOCF_TYPE_NVT(BLK_TYP, FIFOn+1);
				fifocfg |= FIFOCF_BLK_NO(BLKNO-1, FIFOn+1);
				fifocfg |= FIFOCF_BLKSZ_NVT(BLKSZ, FIFOn+1);
				fifocfg |= FIFOCF_FIFO_EN_NVT(0, FIFOn+1);

				fifocfg |= FIFOCF_TYPE_NVT(BLK_TYP, FIFOn+2);
				fifocfg |= FIFOCF_BLK_NO(BLKNO-1, FIFOn+2);
				fifocfg |= FIFOCF_BLKSZ_NVT(BLKSZ, FIFOn+2);
				fifocfg |= FIFOCF_FIFO_EN_NVT(0, FIFOn+2);

				if (BLKSZ == 1) {
					//FIFO4~
					fifocfg2 |= FIFOCF_TYPE_NVT(BLK_TYP, FIFOn+3);
					fifocfg2 |= FIFOCF_BLK_NO(BLKNO-1, FIFOn+3);
					fifocfg2 |= FIFOCF_BLKSZ_NVT(BLKSZ, FIFOn+3);
					fifocfg2 |= FIFOCF_FIFO_EN_NVT(0, FIFOn+3);

					fifocfg2 |= FIFOCF_TYPE_NVT(BLK_TYP, FIFOn+4);
					fifocfg2 |= FIFOCF_BLK_NO(BLKNO-1, FIFOn+4);
					fifocfg2 |= FIFOCF_BLKSZ_NVT(BLKSZ, FIFOn+4);
					fifocfg2 |= FIFOCF_FIFO_EN_NVT(0, FIFOn+4);

					fifocfg2 |= FIFOCF_TYPE_NVT(BLK_TYP, FIFOn+5);
					fifocfg2 |= FIFOCF_BLK_NO(BLKNO-1, FIFOn+5);
					fifocfg2 |= FIFOCF_BLKSZ_NVT(BLKSZ, FIFOn+5);
					fifocfg2 |= FIFOCF_FIFO_EN_NVT(0, FIFOn+5);
				}
			}
		}
		break;

	case USB_FIFO2: {
			fifocfg |= FIFOCF_TYPE_NVT(BLK_TYP, FIFOn);
			fifocfg |= FIFOCF_BLK_NO(BLKNO-1, FIFOn);
			fifocfg |= FIFOCF_BLKSZ_NVT(BLKSZ, FIFOn);
			fifocfg_dir |= FIFOMAP_DIR(Dir, FIFOn);
			fifocfg_dir &= ~FIFOMAP_EPNOMSK_NVT(FIFOn); // fifomap
			fifocfg_dir |= FIFOMAP_EPNO_NVT(ep->epnum, FIFOn);
			fifocfg |= FIFOCF_FIFO_EN_NVT(1, FIFOn);

			if ((BLKNO == BLKNUM_SINGLE) && (BLKSZ == 1)) {
				fifocfg |= FIFOCF_TYPE_NVT(BLK_TYP, FIFOn+1);
				fifocfg |= FIFOCF_BLK_NO(BLKNO-1, FIFOn+1);
				fifocfg |= FIFOCF_BLKSZ_NVT(BLKSZ, FIFOn+1);
				fifocfg |= FIFOCF_FIFO_EN_NVT(0, FIFOn+1);
			} else if (BLKNO == BLKNUM_DOUBLE) {
				fifocfg |= FIFOCF_TYPE_NVT(BLK_TYP, FIFOn+1);
				fifocfg |= FIFOCF_BLK_NO(BLKNO-1, FIFOn+1);
				fifocfg |= FIFOCF_BLKSZ_NVT(BLKSZ, FIFOn+1);
				fifocfg |= FIFOCF_FIFO_EN_NVT(0, FIFOn+1);

				// Block size : 512~1024 bytes
				if (BLKSZ == 1) {
					//FIFO4~
					fifocfg2 |= FIFOCF_TYPE_NVT(BLK_TYP, FIFOn+2);
					fifocfg2 |= FIFOCF_BLK_NO(BLKNO-1, FIFOn+2);
					fifocfg2 |= FIFOCF_BLKSZ_NVT(BLKSZ, FIFOn+2);
					fifocfg2 |= FIFOCF_FIFO_EN_NVT(0, FIFOn+2);

					fifocfg2 |= FIFOCF_TYPE_NVT(BLK_TYP, FIFOn+3);
					fifocfg2 |= FIFOCF_BLK_NO(BLKNO-1, FIFOn+3);
					fifocfg2 |= FIFOCF_BLKSZ_NVT(BLKSZ, FIFOn+3);
					fifocfg2 |= FIFOCF_FIFO_EN_NVT(0, FIFOn+3);
				}
			} else if (BLKNO == BLKNUM_TRIPLE) {
                                fifocfg |= FIFOCF_TYPE_NVT(BLK_TYP, FIFOn+1);
                                fifocfg |= FIFOCF_BLK_NO(BLKNO-1, FIFOn+1);
                                fifocfg |= FIFOCF_BLKSZ_NVT(BLKSZ, FIFOn+1);
                                fifocfg |= FIFOCF_FIFO_EN_NVT(0, FIFOn+1);

				//FIFO4~
                                fifocfg2 |= FIFOCF_TYPE_NVT(BLK_TYP, FIFOn+2);
                                fifocfg2 |= FIFOCF_BLK_NO(BLKNO-1, FIFOn+2);
                                fifocfg2 |= FIFOCF_BLKSZ_NVT(BLKSZ, FIFOn+2);
                                fifocfg2 |= FIFOCF_FIFO_EN_NVT(0, FIFOn+2);

                                if (BLKSZ == 1) {
					fifocfg2 |= FIFOCF_TYPE_NVT(BLK_TYP, FIFOn+3);
					fifocfg2 |= FIFOCF_BLK_NO(BLKNO-1, FIFOn+3);
					fifocfg2 |= FIFOCF_BLKSZ_NVT(BLKSZ, FIFOn+3);
					fifocfg2 |= FIFOCF_FIFO_EN_NVT(0, FIFOn+3);

					fifocfg2 |= FIFOCF_TYPE_NVT(BLK_TYP, FIFOn+4);
					fifocfg2 |= FIFOCF_BLK_NO(BLKNO-1, FIFOn+4);
					fifocfg2 |= FIFOCF_BLKSZ_NVT(BLKSZ, FIFOn+4);
					fifocfg2 |= FIFOCF_FIFO_EN_NVT(0, FIFOn+4);

					fifocfg2 |= FIFOCF_TYPE_NVT(BLK_TYP, FIFOn+5);
					fifocfg2 |= FIFOCF_BLK_NO(BLKNO-1, FIFOn+5);
                                        fifocfg2 |= FIFOCF_BLKSZ_NVT(BLKSZ, FIFOn+5);
                                        fifocfg2 |= FIFOCF_FIFO_EN_NVT(0, FIFOn+5);
                                }

			}
		}
		break;

	case USB_FIFO3: {
			fifocfg |= FIFOCF_TYPE_NVT(BLK_TYP, FIFOn);
			fifocfg |= FIFOCF_BLK_NO(BLKNO-1, FIFOn);
			fifocfg |= FIFOCF_BLKSZ_NVT(BLKSZ, FIFOn);
			fifocfg_dir |= FIFOMAP_DIR(Dir, FIFOn);
			fifocfg_dir &= ~FIFOMAP_EPNOMSK_NVT(FIFOn); // fifomap
			fifocfg_dir |= FIFOMAP_EPNO_NVT(ep->epnum, FIFOn);
			fifocfg |= FIFOCF_FIFO_EN_NVT(1, FIFOn);

			if ((BLKNO == BLKNUM_SINGLE) && (BLKSZ == 1)) {
				//FIFO4~
				fifocfg2 |= FIFOCF_TYPE_NVT(BLK_TYP, FIFOn+1);
				fifocfg2 |= FIFOCF_BLK_NO(BLKNO-1, FIFOn+1);
				fifocfg2 |= FIFOCF_BLKSZ_NVT(BLKSZ, FIFOn+1);
				fifocfg2 |= FIFOCF_FIFO_EN_NVT(0, FIFOn+1);
			} else if (BLKNO == BLKNUM_DOUBLE) {
				//FIFO4~
				fifocfg2 |= FIFOCF_TYPE_NVT(BLK_TYP, FIFOn+1);
				fifocfg2 |= FIFOCF_BLK_NO(BLKNO-1, FIFOn+1);
				fifocfg2 |= FIFOCF_BLKSZ_NVT(BLKSZ, FIFOn+1);
				fifocfg2 |= FIFOCF_FIFO_EN_NVT(0, FIFOn+1);

				// Block size : 512~1024 bytes
				if (BLKSZ == 1) {
					fifocfg2 |= FIFOCF_TYPE_NVT(BLK_TYP, FIFOn+2);
					fifocfg2 |= FIFOCF_BLK_NO(BLKNO-1, FIFOn+2);
					fifocfg2 |= FIFOCF_BLKSZ_NVT(BLKSZ, FIFOn+2);
					fifocfg2 |= FIFOCF_FIFO_EN_NVT(0, FIFOn+2);

					fifocfg2 |= FIFOCF_TYPE_NVT(BLK_TYP, FIFOn+3);
					fifocfg2 |= FIFOCF_BLK_NO(BLKNO-1, FIFOn+3);
					fifocfg2 |= FIFOCF_BLKSZ_NVT(BLKSZ, FIFOn+3);
					fifocfg2 |= FIFOCF_FIFO_EN_NVT(0, FIFOn+3);
				}
			} else if (BLKNO == BLKNUM_TRIPLE) {
				//FIFO4~
				fifocfg2 |= FIFOCF_TYPE_NVT(BLK_TYP, FIFOn+1);
				fifocfg2 |= FIFOCF_BLK_NO(BLKNO-1, FIFOn+1);
				fifocfg2 |= FIFOCF_BLKSZ_NVT(BLKSZ, FIFOn+1);
				fifocfg2 |= FIFOCF_FIFO_EN_NVT(0, FIFOn+1);

				fifocfg2 |= FIFOCF_TYPE_NVT(BLK_TYP, FIFOn+2);
				fifocfg2 |= FIFOCF_BLK_NO(BLKNO-1, FIFOn+2);
				fifocfg2 |= FIFOCF_BLKSZ_NVT(BLKSZ, FIFOn+2);
				fifocfg2 |= FIFOCF_FIFO_EN_NVT(0, FIFOn+2);

				if (BLKSZ == 1) {
					fifocfg2 |= FIFOCF_TYPE_NVT(BLK_TYP, FIFOn+3);
					fifocfg2 |= FIFOCF_BLK_NO(BLKNO-1, FIFOn+3);
					fifocfg2 |= FIFOCF_BLKSZ_NVT(BLKSZ, FIFOn+3);
					fifocfg2 |= FIFOCF_FIFO_EN_NVT(0, FIFOn+3);

					fifocfg2 |= FIFOCF_TYPE_NVT(BLK_TYP, FIFOn+4);
					fifocfg2 |= FIFOCF_BLK_NO(BLKNO-1, FIFOn+4);
					fifocfg2 |= FIFOCF_BLKSZ_NVT(BLKSZ, FIFOn+4);
					fifocfg2 |= FIFOCF_FIFO_EN_NVT(0, FIFOn+4);

					//FIFO8~
					fifocfg3 |= FIFOCF_TYPE_NVT(BLK_TYP, FIFOn+5);
					fifocfg3 |= FIFOCF_BLK_NO(BLKNO-1, FIFOn+5);
					fifocfg3 |= FIFOCF_BLKSZ_NVT(BLKSZ, FIFOn+5);
					fifocfg3 |= FIFOCF_FIFO_EN_NVT(0, FIFOn+5);
				}
			}
	}
	break;

	case USB_FIFO4: {
			//FIFO4~
			fifocfg2 |= FIFOCF_TYPE_NVT(BLK_TYP, FIFOn);
			fifocfg2 |= FIFOCF_BLK_NO(BLKNO-1, FIFOn);
			fifocfg2 |= FIFOCF_BLKSZ_NVT(BLKSZ, FIFOn);
			fifocfg2_dir |= FIFOMAP_DIR(Dir, FIFOn);
			fifocfg2_dir &= ~FIFOMAP_EPNOMSK_NVT(FIFOn); // fifomap
			fifocfg2_dir |= FIFOMAP_EPNO_NVT(ep->epnum, FIFOn);
			fifocfg2 |= FIFOCF_FIFO_EN_NVT(1, FIFOn);

			if ((BLKNO == BLKNUM_SINGLE) && (BLKSZ == 1)) {
				fifocfg2 |= FIFOCF_TYPE_NVT(BLK_TYP, FIFOn+1);
				fifocfg2 |= FIFOCF_BLK_NO(BLKNO-1, FIFOn+1);
				fifocfg2 |= FIFOCF_BLKSZ_NVT(BLKSZ, FIFOn+1);
				fifocfg2 |= FIFOCF_FIFO_EN_NVT(0, FIFOn+1);
			} else if (BLKNO == BLKNUM_DOUBLE) {
				fifocfg2 |= FIFOCF_TYPE_NVT(BLK_TYP, FIFOn+1);
				fifocfg2 |= FIFOCF_BLK_NO(BLKNO-1, FIFOn+1);
				fifocfg2 |= FIFOCF_BLKSZ_NVT(BLKSZ, FIFOn+1);
				fifocfg2 |= FIFOCF_FIFO_EN_NVT(0, FIFOn+1);

				// Block size : 512~1024 bytes
				if (BLKSZ == 1) {
					fifocfg2 |= FIFOCF_TYPE_NVT(BLK_TYP, FIFOn+2);
					fifocfg2 |= FIFOCF_BLK_NO(BLKNO-1, FIFOn+2);
					fifocfg2 |= FIFOCF_BLKSZ_NVT(BLKSZ, FIFOn+2);
					fifocfg2 |= FIFOCF_FIFO_EN_NVT(0, FIFOn+2);

					fifocfg2 |= FIFOCF_TYPE_NVT(BLK_TYP, FIFOn+3);
					fifocfg2 |= FIFOCF_BLK_NO(BLKNO-1, FIFOn+3);
					fifocfg2 |= FIFOCF_BLKSZ_NVT(BLKSZ, FIFOn+3);
					fifocfg2 |= FIFOCF_FIFO_EN_NVT(0, FIFOn+3);
				}
			} else if (BLKNO == BLKNUM_TRIPLE) {
				fifocfg2 |= FIFOCF_TYPE_NVT(BLK_TYP, FIFOn+1);
				fifocfg2 |= FIFOCF_BLK_NO(BLKNO-1, FIFOn+1);
				fifocfg2 |= FIFOCF_BLKSZ_NVT(BLKSZ, FIFOn+1);
				fifocfg2 |= FIFOCF_FIFO_EN_NVT(0, FIFOn+1);

				fifocfg2 |= FIFOCF_TYPE_NVT(BLK_TYP, FIFOn+2);
				fifocfg2 |= FIFOCF_BLK_NO(BLKNO-1, FIFOn+2);
				fifocfg2 |= FIFOCF_BLKSZ_NVT(BLKSZ, FIFOn+2);
				fifocfg2 |= FIFOCF_FIFO_EN_NVT(0, FIFOn+2);

				if (BLKSZ == 1) {
					fifocfg2 |= FIFOCF_TYPE_NVT(BLK_TYP, FIFOn+3);
					fifocfg2 |= FIFOCF_BLK_NO(BLKNO-1, FIFOn+3);
					fifocfg2 |= FIFOCF_BLKSZ_NVT(BLKSZ, FIFOn+3);
					fifocfg2 |= FIFOCF_FIFO_EN_NVT(0, FIFOn+3);

					//FIFO8~
					fifocfg3 |= FIFOCF_TYPE_NVT(BLK_TYP, FIFOn+4);
					fifocfg3 |= FIFOCF_BLK_NO(BLKNO-1, FIFOn+4);
					fifocfg3 |= FIFOCF_BLKSZ_NVT(BLKSZ, FIFOn+4);
					fifocfg3 |= FIFOCF_FIFO_EN_NVT(0, FIFOn+4);

					fifocfg3 |= FIFOCF_TYPE_NVT(BLK_TYP, FIFOn+5);
					fifocfg3 |= FIFOCF_BLK_NO(BLKNO-1, FIFOn+5);
					fifocfg3 |= FIFOCF_BLKSZ_NVT(BLKSZ, FIFOn+5);
					fifocfg3 |= FIFOCF_FIFO_EN_NVT(0, FIFOn+5);
				}
			}
		}
		break;

	case USB_FIFO5: {
			fifocfg2 |= FIFOCF_TYPE_NVT(BLK_TYP, FIFOn);
			fifocfg2 |= FIFOCF_BLK_NO(BLKNO-1, FIFOn);
			fifocfg2 |= FIFOCF_BLKSZ_NVT(BLKSZ, FIFOn);
			fifocfg2_dir |= FIFOMAP_DIR(Dir, FIFOn);
			fifocfg2_dir &= ~FIFOMAP_EPNOMSK_NVT(FIFOn); // fifomap
			fifocfg2_dir |= FIFOMAP_EPNO_NVT(ep->epnum, FIFOn);
			fifocfg2 |= FIFOCF_FIFO_EN_NVT(1, FIFOn);

			if ((BLKNO == BLKNUM_SINGLE) && (BLKSZ == 1)) {
				fifocfg2 |= FIFOCF_TYPE_NVT(BLK_TYP, FIFOn+1);
				fifocfg2 |= FIFOCF_BLK_NO(BLKNO-1, FIFOn+1);
				fifocfg2 |= FIFOCF_BLKSZ_NVT(BLKSZ, FIFOn+1);
				fifocfg2 |= FIFOCF_FIFO_EN_NVT(0, FIFOn+1);
			} else if (BLKNO == BLKNUM_DOUBLE) {
				//FIFO4~
				fifocfg2 |= FIFOCF_TYPE_NVT(BLK_TYP, FIFOn+1);
				fifocfg2 |= FIFOCF_BLK_NO(BLKNO-1, FIFOn+1);
				fifocfg2 |= FIFOCF_BLKSZ_NVT(BLKSZ, FIFOn+1);
				fifocfg2 |= FIFOCF_FIFO_EN_NVT(0, FIFOn+1);

				// Block size : 512~1024 bytes
				if (BLKSZ == 1) {
					fifocfg2 |= FIFOCF_TYPE_NVT(BLK_TYP, FIFOn+2);
					fifocfg2 |= FIFOCF_BLK_NO(BLKNO-1, FIFOn+2);
					fifocfg2 |= FIFOCF_BLKSZ_NVT(BLKSZ, FIFOn+2);
					fifocfg2 |= FIFOCF_FIFO_EN_NVT(0, FIFOn+2);

					//FIFO8~
					fifocfg3 |= FIFOCF_TYPE_NVT(BLK_TYP, FIFOn+3);
					fifocfg3 |= FIFOCF_BLK_NO(BLKNO-1, FIFOn+3);
					fifocfg3 |= FIFOCF_BLKSZ_NVT(BLKSZ, FIFOn+3);
					fifocfg3 |= FIFOCF_FIFO_EN_NVT(0, FIFOn+3);
				}
			} else if (BLKNO == BLKNUM_TRIPLE) {
				fifocfg2 |= FIFOCF_TYPE_NVT(BLK_TYP, FIFOn+1);
				fifocfg2 |= FIFOCF_BLK_NO(BLKNO-1, FIFOn+1);
				fifocfg2 |= FIFOCF_BLKSZ_NVT(BLKSZ, FIFOn+1);
				fifocfg2 |= FIFOCF_FIFO_EN_NVT(0, FIFOn+1);

				fifocfg2 |= FIFOCF_TYPE_NVT(BLK_TYP, FIFOn+2);
				fifocfg2 |= FIFOCF_BLK_NO(BLKNO-1, FIFOn+2);
				fifocfg2 |= FIFOCF_BLKSZ_NVT(BLKSZ, FIFOn+2);
				fifocfg2 |= FIFOCF_FIFO_EN_NVT(0, FIFOn+2);

				if (BLKSZ == 1) {
					//FIFO8
					fifocfg3 |= FIFOCF_TYPE_NVT(BLK_TYP, FIFOn+3);
					fifocfg3 |= FIFOCF_BLK_NO(BLKNO-1, FIFOn+3);
					fifocfg3 |= FIFOCF_BLKSZ_NVT(BLKSZ, FIFOn+3);
					fifocfg3 |= FIFOCF_FIFO_EN_NVT(0, FIFOn+3);

					fifocfg3 |= FIFOCF_TYPE_NVT(BLK_TYP, FIFOn+4);
					fifocfg3 |= FIFOCF_BLK_NO(BLKNO-1, FIFOn+4);
					fifocfg3 |= FIFOCF_BLKSZ_NVT(BLKSZ, FIFOn+4);
					fifocfg3 |= FIFOCF_FIFO_EN_NVT(0, FIFOn+4);

					fifocfg3 |= FIFOCF_TYPE_NVT(BLK_TYP, FIFOn+5);
					fifocfg3 |= FIFOCF_BLK_NO(BLKNO-1, FIFOn+5);
					fifocfg3 |= FIFOCF_BLKSZ_NVT(BLKSZ, FIFOn+5);
					fifocfg3 |= FIFOCF_FIFO_EN_NVT(0, FIFOn+5);
				}
			}
		}
		break;

	case USB_FIFO6: {
			fifocfg2 |= FIFOCF_TYPE_NVT(BLK_TYP, FIFOn);
			fifocfg2 |= FIFOCF_BLK_NO(BLKNO-1, FIFOn);
			fifocfg2 |= FIFOCF_BLKSZ_NVT(BLKSZ, FIFOn);
			fifocfg2_dir |= FIFOMAP_DIR(Dir, FIFOn);
			fifocfg2_dir &= ~FIFOMAP_EPNOMSK_NVT(FIFOn); // fifomap
			fifocfg2_dir |= FIFOMAP_EPNO_NVT(ep->epnum, FIFOn);
			fifocfg2 |= FIFOCF_FIFO_EN_NVT(1, FIFOn);

			if ((BLKNO == BLKNUM_SINGLE) && (BLKSZ == 1)) {
				fifocfg2 |= FIFOCF_TYPE_NVT(BLK_TYP, FIFOn+1);
				fifocfg2 |= FIFOCF_BLK_NO(BLKNO-1, FIFOn+1);
				fifocfg2 |= FIFOCF_BLKSZ_NVT(BLKSZ, FIFOn+1);
				fifocfg2 |= FIFOCF_FIFO_EN_NVT(0, FIFOn+1);
			} else if (BLKNO == BLKNUM_DOUBLE) {
				fifocfg2 |= FIFOCF_TYPE_NVT(BLK_TYP, FIFOn+1);
				fifocfg2 |= FIFOCF_BLK_NO(BLKNO-1, FIFOn+1);
				fifocfg2 |= FIFOCF_BLKSZ_NVT(BLKSZ, FIFOn+1);
				fifocfg2 |= FIFOCF_FIFO_EN_NVT(0, FIFOn+1);

				// Block size : 512~1024 bytes
				if (BLKSZ == 1) {
					//FIFO8~
					fifocfg3 |= FIFOCF_TYPE_NVT(BLK_TYP, FIFOn+2);
					fifocfg3 |= FIFOCF_BLK_NO(BLKNO-1, FIFOn+2);
					fifocfg3 |= FIFOCF_BLKSZ_NVT(BLKSZ, FIFOn+2);
					fifocfg3 |= FIFOCF_FIFO_EN_NVT(0, FIFOn+2);

					fifocfg3 |= FIFOCF_TYPE_NVT(BLK_TYP, FIFOn+3);
					fifocfg3 |= FIFOCF_BLK_NO(BLKNO-1, FIFOn+3);
					fifocfg3 |= FIFOCF_BLKSZ_NVT(BLKSZ, FIFOn+3);
					fifocfg3 |= FIFOCF_FIFO_EN_NVT(0, FIFOn+3);
				}
			} else if (BLKNO == BLKNUM_TRIPLE) {
				fifocfg2 |= FIFOCF_TYPE_NVT(BLK_TYP, FIFOn+1);
				fifocfg2 |= FIFOCF_BLK_NO(BLKNO-1, FIFOn+1);
				fifocfg2 |= FIFOCF_BLKSZ_NVT(BLKSZ, FIFOn+1);
				fifocfg2 |= FIFOCF_FIFO_EN_NVT(0, FIFOn+1);

				//FIFO8~
				fifocfg3 |= FIFOCF_TYPE_NVT(BLK_TYP, FIFOn+2);
				fifocfg3 |= FIFOCF_BLK_NO(BLKNO-1, FIFOn+2);
				fifocfg3 |= FIFOCF_BLKSZ_NVT(BLKSZ, FIFOn+2);
				fifocfg3 |= FIFOCF_FIFO_EN_NVT(0, FIFOn+2);

				if (BLKSZ == 1) {
					fifocfg3 |= FIFOCF_TYPE_NVT(BLK_TYP, FIFOn+3);
					fifocfg3 |= FIFOCF_BLK_NO(BLKNO-1, FIFOn+3);
					fifocfg3 |= FIFOCF_BLKSZ_NVT(BLKSZ, FIFOn+3);
					fifocfg3 |= FIFOCF_FIFO_EN_NVT(0, FIFOn+3);

					fifocfg3 |= FIFOCF_TYPE_NVT(BLK_TYP, FIFOn+4);
					fifocfg3 |= FIFOCF_BLK_NO(BLKNO-1, FIFOn+4);
					fifocfg3 |= FIFOCF_BLKSZ_NVT(BLKSZ, FIFOn+4);
					fifocfg3 |= FIFOCF_FIFO_EN_NVT(0, FIFOn+4);

					fifocfg3 |= FIFOCF_TYPE_NVT(BLK_TYP, FIFOn+5);
					fifocfg3 |= FIFOCF_BLK_NO(BLKNO-1, FIFOn+5);
					fifocfg3 |= FIFOCF_BLKSZ_NVT(BLKSZ, FIFOn+5);
					fifocfg3 |= FIFOCF_FIFO_EN_NVT(0, FIFOn+5);
				}
			}
		}
		break;

	case USB_FIFO7: {
			fifocfg2 |= FIFOCF_TYPE_NVT(BLK_TYP, FIFOn);
			fifocfg2 |= FIFOCF_BLK_NO(BLKNO-1, FIFOn);
			fifocfg2 |= FIFOCF_BLKSZ_NVT(BLKSZ, FIFOn);
			fifocfg2_dir |= FIFOMAP_DIR(Dir, FIFOn);
			fifocfg2_dir &= ~FIFOMAP_EPNOMSK_NVT(FIFOn); // fifomap
			fifocfg2_dir |= FIFOMAP_EPNO_NVT(ep->epnum, FIFOn);
			fifocfg2 |= FIFOCF_FIFO_EN_NVT(1, FIFOn);

			if ((BLKNO == BLKNUM_SINGLE) && (BLKSZ == 1)) {
				//FIFO8~
				fifocfg3 |= FIFOCF_TYPE_NVT(BLK_TYP, FIFOn+1);
				fifocfg3 |= FIFOCF_BLK_NO(BLKNO-1, FIFOn+1);
				fifocfg3 |= FIFOCF_BLKSZ_NVT(BLKSZ, FIFOn+1);
				fifocfg3 |= FIFOCF_FIFO_EN_NVT(0, FIFOn+1);
			} else if (BLKNO == BLKNUM_DOUBLE) {
				//FIFO8~
				fifocfg3 |= FIFOCF_TYPE_NVT(BLK_TYP, FIFOn+1);
				fifocfg3 |= FIFOCF_BLK_NO(BLKNO-1, FIFOn+1);
				fifocfg3 |= FIFOCF_BLKSZ_NVT(BLKSZ, FIFOn+1);
				fifocfg3 |= FIFOCF_FIFO_EN_NVT(0, FIFOn+1);

				// Block size : 512~1024 bytes
				if (BLKSZ == 1) {
					fifocfg3 |= FIFOCF_TYPE_NVT(BLK_TYP, FIFOn+2);
					fifocfg3 |= FIFOCF_BLK_NO(BLKNO-1, FIFOn+2);
					fifocfg3 |= FIFOCF_BLKSZ_NVT(BLKSZ, FIFOn+2);
					fifocfg3 |= FIFOCF_FIFO_EN_NVT(0, FIFOn+2);

					fifocfg3 |= FIFOCF_TYPE_NVT(BLK_TYP, FIFOn+3);
					fifocfg3 |= FIFOCF_BLK_NO(BLKNO-1, FIFOn+3);
					fifocfg3 |= FIFOCF_BLKSZ_NVT(BLKSZ, FIFOn+3);
					fifocfg3 |= FIFOCF_FIFO_EN_NVT(0, FIFOn+3);
				}
			} else if (BLKNO == BLKNUM_TRIPLE) {
				//FIFO8~
				fifocfg3 |= FIFOCF_TYPE_NVT(BLK_TYP, FIFOn+1);
				fifocfg3 |= FIFOCF_BLK_NO(BLKNO-1, FIFOn+1);
				fifocfg3 |= FIFOCF_BLKSZ_NVT(BLKSZ, FIFOn+1);
				fifocfg3 |= FIFOCF_FIFO_EN_NVT(0, FIFOn+1);

				fifocfg3 |= FIFOCF_TYPE_NVT(BLK_TYP, FIFOn+2);
				fifocfg3 |= FIFOCF_BLK_NO(BLKNO-1, FIFOn+2);
				fifocfg3 |= FIFOCF_BLKSZ_NVT(BLKSZ, FIFOn+2);
				fifocfg3 |= FIFOCF_FIFO_EN_NVT(0, FIFOn+2);

				if (BLKSZ == 1) {
					fifocfg3 |= FIFOCF_TYPE_NVT(BLK_TYP, FIFOn+3);
					fifocfg3 |= FIFOCF_BLK_NO(BLKNO-1, FIFOn+3);
					fifocfg3 |= FIFOCF_BLKSZ_NVT(BLKSZ, FIFOn+3);
					fifocfg3 |= FIFOCF_FIFO_EN_NVT(0, FIFOn+3);

					fifocfg3 |= FIFOCF_TYPE_NVT(BLK_TYP, FIFOn+4);
					fifocfg3 |= FIFOCF_BLK_NO(BLKNO-1, FIFOn+4);
					fifocfg3 |= FIFOCF_BLKSZ_NVT(BLKSZ, FIFOn+4);
					fifocfg3 |= FIFOCF_FIFO_EN_NVT(0, FIFOn+4);

					//FIFO12~
					fifocfg4 |= FIFOCF_TYPE_NVT(BLK_TYP, FIFOn+5);
					fifocfg4 |= FIFOCF_BLK_NO(BLKNO-1, FIFOn+5);
					fifocfg4 |= FIFOCF_BLKSZ_NVT(BLKSZ, FIFOn+5);
					fifocfg4 |= FIFOCF_FIFO_EN_NVT(0, FIFOn+5);
				}
			}
		}
		break;

	case USB_FIFO8: {
			fifocfg3 |= FIFOCF_TYPE_NVT(BLK_TYP, FIFOn);
			fifocfg3 |= FIFOCF_BLK_NO(BLKNO-1, FIFOn);
			fifocfg3 |= FIFOCF_BLKSZ_NVT(BLKSZ, FIFOn);
			fifocfg3_dir |= FIFOMAP_DIR(Dir, FIFOn);
			fifocfg3_dir &= ~FIFOMAP_EPNOMSK_NVT(FIFOn); // fifomap
			fifocfg3_dir |= FIFOMAP_EPNO_NVT(ep->epnum, FIFOn);
			fifocfg3 |= FIFOCF_FIFO_EN_NVT(1, FIFOn);

			if ((BLKNO == BLKNUM_SINGLE) && (BLKSZ == 1)) {
				fifocfg3 |= FIFOCF_TYPE_NVT(BLK_TYP, FIFOn+1);
				fifocfg3 |= FIFOCF_BLK_NO(BLKNO-1, FIFOn+1);
				fifocfg3 |= FIFOCF_BLKSZ_NVT(BLKSZ, FIFOn+1);
				fifocfg3 |= FIFOCF_FIFO_EN_NVT(0, FIFOn+1);
			} else if (BLKNO == BLKNUM_DOUBLE) {
				fifocfg3 |= FIFOCF_TYPE_NVT(BLK_TYP, FIFOn+1);
				fifocfg3 |= FIFOCF_BLK_NO(BLKNO-1, FIFOn+1);
				fifocfg3 |= FIFOCF_BLKSZ_NVT(BLKSZ, FIFOn+1);
				fifocfg3 |= FIFOCF_FIFO_EN_NVT(0, FIFOn+1);

				// Block size : 512~1024 bytes
				if (BLKSZ == 1) {
					fifocfg3 |= FIFOCF_TYPE_NVT(BLK_TYP, FIFOn+2);
					fifocfg3 |= FIFOCF_BLK_NO(BLKNO-1, FIFOn+2);
					fifocfg3 |= FIFOCF_BLKSZ_NVT(BLKSZ, FIFOn+2);
					fifocfg3 |= FIFOCF_FIFO_EN_NVT(0, FIFOn+2);

					fifocfg3 |= FIFOCF_TYPE_NVT(BLK_TYP, FIFOn+3);
					fifocfg3 |= FIFOCF_BLK_NO(BLKNO-1, FIFOn+3);
					fifocfg3 |= FIFOCF_BLKSZ_NVT(BLKSZ, FIFOn+3);
					fifocfg3 |= FIFOCF_FIFO_EN_NVT(0, FIFOn+3);
				}
			} else if (BLKNO == BLKNUM_TRIPLE) {
				fifocfg3 |= FIFOCF_TYPE_NVT(BLK_TYP, FIFOn+1);
				fifocfg3 |= FIFOCF_BLK_NO(BLKNO-1, FIFOn+1);
				fifocfg3 |= FIFOCF_BLKSZ_NVT(BLKSZ, FIFOn+1);
				fifocfg3 |= FIFOCF_FIFO_EN_NVT(0, FIFOn+1);

				fifocfg3 |= FIFOCF_TYPE_NVT(BLK_TYP, FIFOn+2);
				fifocfg3 |= FIFOCF_BLK_NO(BLKNO-1, FIFOn+2);
				fifocfg3 |= FIFOCF_BLKSZ_NVT(BLKSZ, FIFOn+2);
				fifocfg3 |= FIFOCF_FIFO_EN_NVT(0, FIFOn+2);

				if (BLKSZ == 1) {
					fifocfg3 |= FIFOCF_TYPE_NVT(BLK_TYP, FIFOn+3);
					fifocfg3 |= FIFOCF_BLK_NO(BLKNO-1, FIFOn+3);
					fifocfg3 |= FIFOCF_BLKSZ_NVT(BLKSZ, FIFOn+3);
					fifocfg3 |= FIFOCF_FIFO_EN_NVT(0, FIFOn+3);

					//FIFO12~
					fifocfg4 |= FIFOCF_TYPE_NVT(BLK_TYP, FIFOn+4);
					fifocfg4 |= FIFOCF_BLK_NO(BLKNO-1, FIFOn+4);
					fifocfg4 |= FIFOCF_BLKSZ_NVT(BLKSZ, FIFOn+4);
					fifocfg4 |= FIFOCF_FIFO_EN_NVT(0, FIFOn+4);

					fifocfg4 |= FIFOCF_TYPE_NVT(BLK_TYP, FIFOn+5);
					fifocfg4 |= FIFOCF_BLK_NO(BLKNO-1, FIFOn+5);
					fifocfg4 |= FIFOCF_BLKSZ_NVT(BLKSZ, FIFOn+5);
					fifocfg4 |= FIFOCF_FIFO_EN_NVT(0, FIFOn+5);
				}
			}
		}
		break;


	case USB_FIFO9: {
			fifocfg3 |= FIFOCF_TYPE_NVT(BLK_TYP, FIFOn);
			fifocfg3 |= FIFOCF_BLK_NO(BLKNO-1, FIFOn);
			fifocfg3 |= FIFOCF_BLKSZ_NVT(BLKSZ, FIFOn);
			fifocfg3_dir |= FIFOMAP_DIR(Dir, FIFOn);
			fifocfg3_dir &= ~FIFOMAP_EPNOMSK_NVT(FIFOn); // fifomap
			fifocfg3_dir |= FIFOMAP_EPNO_NVT(ep->epnum, FIFOn);
			fifocfg3 |= FIFOCF_FIFO_EN_NVT(1, FIFOn);

			if ((BLKNO == BLKNUM_SINGLE) && (BLKSZ == 1)) {
				fifocfg3 |= FIFOCF_TYPE_NVT(BLK_TYP, FIFOn+1);
				fifocfg3 |= FIFOCF_BLK_NO(BLKNO-1, FIFOn+1);
				fifocfg3 |= FIFOCF_BLKSZ_NVT(BLKSZ, FIFOn+1);
				fifocfg3 |= FIFOCF_FIFO_EN_NVT(0, FIFOn+1);
			} else if (BLKNO == BLKNUM_DOUBLE) {
				fifocfg3 |= FIFOCF_TYPE_NVT(BLK_TYP, FIFOn+1);
				fifocfg3 |= FIFOCF_BLK_NO(BLKNO-1, FIFOn+1);
				fifocfg3 |= FIFOCF_BLKSZ_NVT(BLKSZ, FIFOn+1);
				fifocfg3 |= FIFOCF_FIFO_EN_NVT(0, FIFOn+1);

				// Block size : 512~1024 bytes
				if (BLKSZ == 1) {
					fifocfg3 |= FIFOCF_TYPE_NVT(BLK_TYP, FIFOn+2);
					fifocfg3 |= FIFOCF_BLK_NO(BLKNO-1, FIFOn+2);
					fifocfg3 |= FIFOCF_BLKSZ_NVT(BLKSZ, FIFOn+2);
					fifocfg3 |= FIFOCF_FIFO_EN_NVT(0, FIFOn+2);

					//FIFO12~
					fifocfg4 |= FIFOCF_TYPE_NVT(BLK_TYP, FIFOn+3);
					fifocfg4 |= FIFOCF_BLK_NO(BLKNO-1, FIFOn+3);
					fifocfg4 |= FIFOCF_BLKSZ_NVT(BLKSZ, FIFOn+3);
					fifocfg4 |= FIFOCF_FIFO_EN_NVT(0, FIFOn+3);
				}
			} else if (BLKNO == BLKNUM_TRIPLE) {
				fifocfg3 |= FIFOCF_TYPE_NVT(BLK_TYP, FIFOn+1);
				fifocfg3 |= FIFOCF_BLK_NO(BLKNO-1, FIFOn+1);
				fifocfg3 |= FIFOCF_BLKSZ_NVT(BLKSZ, FIFOn+1);
				fifocfg3 |= FIFOCF_FIFO_EN_NVT(0, FIFOn+1);

				fifocfg3 |= FIFOCF_TYPE_NVT(BLK_TYP, FIFOn+2);
				fifocfg3 |= FIFOCF_BLK_NO(BLKNO-1, FIFOn+2);
				fifocfg3 |= FIFOCF_BLKSZ_NVT(BLKSZ, FIFOn+2);
				fifocfg3 |= FIFOCF_FIFO_EN_NVT(0, FIFOn+2);

				if (BLKSZ == 1) {
					//FIFO12~
					fifocfg4 |= FIFOCF_TYPE_NVT(BLK_TYP, FIFOn+3);
					fifocfg4 |= FIFOCF_BLK_NO(BLKNO-1, FIFOn+3);
					fifocfg4 |= FIFOCF_BLKSZ_NVT(BLKSZ, FIFOn+3);
					fifocfg4 |= FIFOCF_FIFO_EN_NVT(0, FIFOn+3);

					fifocfg4 |= FIFOCF_TYPE_NVT(BLK_TYP, FIFOn+4);
					fifocfg4 |= FIFOCF_BLK_NO(BLKNO-1, FIFOn+4);
					fifocfg4 |= FIFOCF_BLKSZ_NVT(BLKSZ, FIFOn+4);
					fifocfg4 |= FIFOCF_FIFO_EN_NVT(0, FIFOn+4);

					fifocfg4 |= FIFOCF_TYPE_NVT(BLK_TYP, FIFOn+5);
					fifocfg4 |= FIFOCF_BLK_NO(BLKNO-1, FIFOn+5);
					fifocfg4 |= FIFOCF_BLKSZ_NVT(BLKSZ, FIFOn+5);
					fifocfg4 |= FIFOCF_FIFO_EN_NVT(0, FIFOn+5);
				}
			}
		}
		break;

	case USB_FIFO10: {
			fifocfg3 |= FIFOCF_TYPE_NVT(BLK_TYP, FIFOn);
			fifocfg3 |= FIFOCF_BLK_NO(BLKNO-1, FIFOn);
			fifocfg3 |= FIFOCF_BLKSZ_NVT(BLKSZ, FIFOn);
			fifocfg3_dir |= FIFOMAP_DIR(Dir, FIFOn);
			fifocfg3_dir &= ~FIFOMAP_EPNOMSK_NVT(FIFOn); // fifomap
			fifocfg3_dir |= FIFOMAP_EPNO_NVT(ep->epnum, FIFOn);
			fifocfg3 |= FIFOCF_FIFO_EN_NVT(1, FIFOn);

			if ((BLKNO == BLKNUM_SINGLE) && (BLKSZ == 1)) {
				fifocfg3 |= FIFOCF_TYPE_NVT(BLK_TYP, FIFOn+1);
				fifocfg3 |= FIFOCF_BLK_NO(BLKNO-1, FIFOn+1);
				fifocfg3 |= FIFOCF_BLKSZ_NVT(BLKSZ, FIFOn+1);
				fifocfg3 |= FIFOCF_FIFO_EN_NVT(0, FIFOn+1);
			} else if (BLKNO == BLKNUM_DOUBLE) {
				fifocfg3 |= FIFOCF_TYPE_NVT(BLK_TYP, FIFOn+1);
				fifocfg3 |= FIFOCF_BLK_NO(BLKNO-1, FIFOn+1);
				fifocfg3 |= FIFOCF_BLKSZ_NVT(BLKSZ, FIFOn+1);
				fifocfg3 |= FIFOCF_FIFO_EN_NVT(0, FIFOn+1);

				// Block size : 512~1024 bytes
				if (BLKSZ == 1) {
					//FIFO12~
					fifocfg4 |= FIFOCF_TYPE_NVT(BLK_TYP, FIFOn+2);
					fifocfg4 |= FIFOCF_BLK_NO(BLKNO-1, FIFOn+2);
					fifocfg4 |= FIFOCF_BLKSZ_NVT(BLKSZ, FIFOn+2);
					fifocfg4 |= FIFOCF_FIFO_EN_NVT(0, FIFOn+2);

					fifocfg4 |= FIFOCF_TYPE_NVT(BLK_TYP, FIFOn+3);
					fifocfg4 |= FIFOCF_BLK_NO(BLKNO-1, FIFOn+3);
					fifocfg4 |= FIFOCF_BLKSZ_NVT(BLKSZ, FIFOn+3);
					fifocfg4 |= FIFOCF_FIFO_EN_NVT(0, FIFOn+3);
				}
			} else if (BLKNO == BLKNUM_TRIPLE) {
				fifocfg3 |= FIFOCF_TYPE_NVT(BLK_TYP, FIFOn+1);
				fifocfg3 |= FIFOCF_BLK_NO(BLKNO-1, FIFOn+1);
				fifocfg3 |= FIFOCF_BLKSZ_NVT(BLKSZ, FIFOn+1);
				fifocfg3 |= FIFOCF_FIFO_EN_NVT(0, FIFOn+1);

				//FIFO12~
				fifocfg4 |= FIFOCF_TYPE_NVT(BLK_TYP, FIFOn+2);
				fifocfg4 |= FIFOCF_BLK_NO(BLKNO-1, FIFOn+2);
				fifocfg4 |= FIFOCF_BLKSZ_NVT(BLKSZ, FIFOn+2);
				fifocfg4 |= FIFOCF_FIFO_EN_NVT(0, FIFOn+2);

				if (BLKSZ == 1) {
					fifocfg4 |= FIFOCF_TYPE_NVT(BLK_TYP, FIFOn+3);
					fifocfg4 |= FIFOCF_BLK_NO(BLKNO-1, FIFOn+3);
					fifocfg4 |= FIFOCF_BLKSZ_NVT(BLKSZ, FIFOn+3);
					fifocfg4 |= FIFOCF_FIFO_EN_NVT(0, FIFOn+3);

					fifocfg4 |= FIFOCF_TYPE_NVT(BLK_TYP, FIFOn+4);
					fifocfg4 |= FIFOCF_BLK_NO(BLKNO-1, FIFOn+4);
					fifocfg4 |= FIFOCF_BLKSZ_NVT(BLKSZ, FIFOn+4);
					fifocfg4 |= FIFOCF_FIFO_EN_NVT(0, FIFOn+4);

					fifocfg4 |= FIFOCF_TYPE_NVT(BLK_TYP, FIFOn+5);
					fifocfg4 |= FIFOCF_BLK_NO(BLKNO-1, FIFOn+5);
					fifocfg4 |= FIFOCF_BLKSZ_NVT(BLKSZ, FIFOn+5);
					fifocfg4 |= FIFOCF_FIFO_EN_NVT(0, FIFOn+5);
				}
			}
		}
		break;

	case USB_FIFO11: {
			fifocfg3 |= FIFOCF_TYPE_NVT(BLK_TYP, FIFOn);
			fifocfg3 |= FIFOCF_BLK_NO(BLKNO-1, FIFOn);
			fifocfg3 |= FIFOCF_BLKSZ_NVT(BLKSZ, FIFOn);
			fifocfg3_dir |= FIFOMAP_DIR(Dir, FIFOn);
			fifocfg3_dir &= ~FIFOMAP_EPNOMSK_NVT(FIFOn); // fifomap
			fifocfg3_dir |= FIFOMAP_EPNO_NVT(ep->epnum, FIFOn);
			fifocfg3 |= FIFOCF_FIFO_EN_NVT(1, FIFOn);

			if ((BLKNO == BLKNUM_SINGLE) && (BLKSZ == 1)) {
				//FIFO12~
				fifocfg4 |= FIFOCF_TYPE_NVT(BLK_TYP, FIFOn+1);
				fifocfg4 |= FIFOCF_BLK_NO(BLKNO-1, FIFOn+1);
				fifocfg4 |= FIFOCF_BLKSZ_NVT(BLKSZ, FIFOn+1);
				fifocfg4 |= FIFOCF_FIFO_EN_NVT(0, FIFOn+1);
			} else if (BLKNO == BLKNUM_DOUBLE) {
				fifocfg4 |= FIFOCF_TYPE_NVT(BLK_TYP, FIFOn+1);
				fifocfg4 |= FIFOCF_BLK_NO(BLKNO-1, FIFOn+1);
				fifocfg4 |= FIFOCF_BLKSZ_NVT(BLKSZ, FIFOn+1);
				fifocfg4 |= FIFOCF_FIFO_EN_NVT(0, FIFOn+1);

				// Block size : 512~1024 bytes
				if (BLKSZ == 1) {
					fifocfg4 |= FIFOCF_TYPE_NVT(BLK_TYP, FIFOn+2);
					fifocfg4 |= FIFOCF_BLK_NO(BLKNO-1, FIFOn+2);
					fifocfg4 |= FIFOCF_BLKSZ_NVT(BLKSZ, FIFOn+2);
					fifocfg4 |= FIFOCF_FIFO_EN_NVT(0, FIFOn+2);

					fifocfg4 |= FIFOCF_TYPE_NVT(BLK_TYP, FIFOn+3);
					fifocfg4 |= FIFOCF_BLK_NO(BLKNO-1, FIFOn+3);
					fifocfg4 |= FIFOCF_BLKSZ_NVT(BLKSZ, FIFOn+3);
					fifocfg4 |= FIFOCF_FIFO_EN_NVT(0, FIFOn+3);
				}
			} else if (BLKNO == BLKNUM_TRIPLE) {
				//FIFO12~
				fifocfg4 |= FIFOCF_TYPE_NVT(BLK_TYP, FIFOn+1);
				fifocfg4 |= FIFOCF_BLK_NO(BLKNO-1, FIFOn+1);
				fifocfg4 |= FIFOCF_BLKSZ_NVT(BLKSZ, FIFOn+1);
				fifocfg4 |= FIFOCF_FIFO_EN_NVT(0, FIFOn+1);

				fifocfg4 |= FIFOCF_TYPE_NVT(BLK_TYP, FIFOn+2);
				fifocfg4 |= FIFOCF_BLK_NO(BLKNO-1, FIFOn+2);
				fifocfg4 |= FIFOCF_BLKSZ_NVT(BLKSZ, FIFOn+2);
				fifocfg4 |= FIFOCF_FIFO_EN_NVT(0, FIFOn+2);

				if (BLKSZ == 1) {
					pr_err("space not enough\n");
				}
			}
		}
		break;

	case USB_FIFO12: {
			fifocfg4 |= FIFOCF_TYPE_NVT(BLK_TYP, FIFOn);
			fifocfg4 |= FIFOCF_BLK_NO(BLKNO-1, FIFOn);
			fifocfg4 |= FIFOCF_BLKSZ_NVT(BLKSZ, FIFOn);
			fifocfg4_dir |= FIFOMAP_DIR(Dir, FIFOn);
			fifocfg4_dir &= ~FIFOMAP_EPNOMSK_NVT(FIFOn); // fifomap
			fifocfg4_dir |= FIFOMAP_EPNO_NVT(ep->epnum, FIFOn);
			fifocfg4 |= FIFOCF_FIFO_EN_NVT(1, FIFOn);

			if ((BLKNO == BLKNUM_SINGLE) && (BLKSZ == 1)) {
				fifocfg4 |= FIFOCF_TYPE_NVT(BLK_TYP, FIFOn+1);
				fifocfg4 |= FIFOCF_BLK_NO(BLKNO-1, FIFOn+1);
				fifocfg4 |= FIFOCF_BLKSZ_NVT(BLKSZ, FIFOn+1);
				fifocfg4 |= FIFOCF_FIFO_EN_NVT(0, FIFOn+1);
			} else if (BLKNO == BLKNUM_DOUBLE) {
				fifocfg4 |= FIFOCF_TYPE_NVT(BLK_TYP, FIFOn+1);
				fifocfg4 |= FIFOCF_BLK_NO(BLKNO-1, FIFOn+1);
				fifocfg4 |= FIFOCF_BLKSZ_NVT(BLKSZ, FIFOn+1);
				fifocfg4 |= FIFOCF_FIFO_EN_NVT(0, FIFOn+1);

				// Block size : 512~1024 bytes
				if (BLKSZ == 1) {
					fifocfg4 |= FIFOCF_TYPE_NVT(BLK_TYP, FIFOn+2);
					fifocfg4 |= FIFOCF_BLK_NO(BLKNO-1, FIFOn+2);
					fifocfg4 |= FIFOCF_BLKSZ_NVT(BLKSZ, FIFOn+2);
					fifocfg4 |= FIFOCF_FIFO_EN_NVT(0, FIFOn+2);

					fifocfg4 |= FIFOCF_TYPE_NVT(BLK_TYP, FIFOn+3);
					fifocfg4 |= FIFOCF_BLK_NO(BLKNO-1, FIFOn+3);
					fifocfg4 |= FIFOCF_BLKSZ_NVT(BLKSZ, FIFOn+3);
					fifocfg4 |= FIFOCF_FIFO_EN_NVT(0, FIFOn+3);
				}
			} else if (BLKNO == BLKNUM_TRIPLE) {
				fifocfg4 |= FIFOCF_TYPE_NVT(BLK_TYP, FIFOn+1);
				fifocfg4 |= FIFOCF_BLK_NO(BLKNO-1, FIFOn+1);
				fifocfg4 |= FIFOCF_BLKSZ_NVT(BLKSZ, FIFOn+1);
				fifocfg4 |= FIFOCF_FIFO_EN_NVT(0, FIFOn+1);

				fifocfg4 |= FIFOCF_TYPE_NVT(BLK_TYP, FIFOn+2);
				fifocfg4 |= FIFOCF_BLK_NO(BLKNO-1, FIFOn+2);
				fifocfg4 |= FIFOCF_BLKSZ_NVT(BLKSZ, FIFOn+2);
				fifocfg4 |= FIFOCF_FIFO_EN_NVT(0, FIFOn+2);

				if (BLKSZ == 1) {
					pr_err("space not enough\n");
				}
			}
		}
		break;

	case USB_FIFO13: {
			fifocfg4 |= FIFOCF_TYPE_NVT(BLK_TYP, FIFOn);
			fifocfg4 |= FIFOCF_BLK_NO(BLKNO-1, FIFOn);
			fifocfg4 |= FIFOCF_BLKSZ_NVT(BLKSZ, FIFOn);
			fifocfg4_dir |= FIFOMAP_DIR(Dir, FIFOn);
			fifocfg4_dir &= ~FIFOMAP_EPNOMSK_NVT(FIFOn); // fifomap
			fifocfg4_dir |= FIFOMAP_EPNO_NVT(ep->epnum, FIFOn);
			fifocfg4 |= FIFOCF_FIFO_EN_NVT(1, FIFOn);

			if ((BLKNO == BLKNUM_SINGLE) && (BLKSZ == 1)) {
				fifocfg4 |= FIFOCF_TYPE_NVT(BLK_TYP, FIFOn+1);
				fifocfg4 |= FIFOCF_BLK_NO(BLKNO-1, FIFOn+1);
				fifocfg4 |= FIFOCF_BLKSZ_NVT(BLKSZ, FIFOn+1);
				fifocfg4 |= FIFOCF_FIFO_EN_NVT(0, FIFOn+1);
			} else if (BLKNO == BLKNUM_DOUBLE) {
				fifocfg4 |= FIFOCF_TYPE_NVT(BLK_TYP, FIFOn+1);
				fifocfg4 |= FIFOCF_BLK_NO(BLKNO-1, FIFOn+1);
				fifocfg4 |= FIFOCF_BLKSZ_NVT(BLKSZ, FIFOn+1);
				fifocfg4 |= FIFOCF_FIFO_EN_NVT(0, FIFOn+1);

				// Block size : 512~1024 bytes
				if (BLKSZ == 1) {
					pr_err("space not enough\n");
				}
			} else if (BLKNO == BLKNUM_TRIPLE) {
				fifocfg4 |= FIFOCF_TYPE_NVT(BLK_TYP, FIFOn+1);
				fifocfg4 |= FIFOCF_BLK_NO(BLKNO-1, FIFOn+1);
				fifocfg4 |= FIFOCF_BLKSZ_NVT(BLKSZ, FIFOn+1);
				fifocfg4 |= FIFOCF_FIFO_EN_NVT(0, FIFOn+1);

				fifocfg4 |= FIFOCF_TYPE_NVT(BLK_TYP, FIFOn+2);
				fifocfg4 |= FIFOCF_BLK_NO(BLKNO-1, FIFOn+2);
				fifocfg4 |= FIFOCF_BLKSZ_NVT(BLKSZ, FIFOn+2);
				fifocfg4 |= FIFOCF_FIFO_EN_NVT(0, FIFOn+2);

				if (BLKSZ == 1) {
					pr_err("space not enough\n");
				}
			}
		}
		break;


	case USB_FIFO14: {
			fifocfg4 |= FIFOCF_TYPE_NVT(BLK_TYP, FIFOn);
			fifocfg4 |= FIFOCF_BLK_NO(BLKNO-1, FIFOn);
			fifocfg4 |= FIFOCF_BLKSZ_NVT(BLKSZ, FIFOn);
			fifocfg4_dir |= FIFOMAP_DIR(Dir, FIFOn);
			fifocfg4_dir &= ~FIFOMAP_EPNOMSK_NVT(FIFOn); // fifomap
			fifocfg4_dir |= FIFOMAP_EPNO_NVT(ep->epnum, FIFOn);
			fifocfg4 |= FIFOCF_FIFO_EN_NVT(1, FIFOn);

			if ((BLKNO == BLKNUM_SINGLE) && (BLKSZ == 1)) {
				fifocfg4 |= FIFOCF_TYPE_NVT(BLK_TYP, FIFOn+1);
				fifocfg4 |= FIFOCF_BLK_NO(BLKNO-1, FIFOn+1);
				fifocfg4 |= FIFOCF_BLKSZ_NVT(BLKSZ, FIFOn+1);
				fifocfg4 |= FIFOCF_FIFO_EN_NVT(0, FIFOn+1);
			} else if (BLKNO == BLKNUM_DOUBLE) {
				fifocfg4 |= FIFOCF_TYPE_NVT(BLK_TYP, FIFOn+1);
				fifocfg4 |= FIFOCF_BLK_NO(BLKNO-1, FIFOn+1);
				fifocfg4 |= FIFOCF_BLKSZ_NVT(BLKSZ, FIFOn+1);
				fifocfg4 |= FIFOCF_FIFO_EN_NVT(0, FIFOn+1);

				// Block size : 512~1024 bytes
				if (BLKSZ == 1) {
					pr_err("space not enough\n");
				}
			} else if (BLKNO == BLKNUM_TRIPLE) {
				pr_err("space not enough\n");
			}
		}
		break;

	case USB_FIFO15: {
			fifocfg4 |= FIFOCF_TYPE_NVT(BLK_TYP, FIFOn);
			fifocfg4 |= FIFOCF_BLK_NO(BLKNO-1, FIFOn);
			fifocfg4 |= FIFOCF_BLKSZ_NVT(BLKSZ, FIFOn);
			fifocfg4_dir |= FIFOMAP_DIR(Dir, FIFOn);
			fifocfg4_dir &= ~FIFOMAP_EPNOMSK_NVT(FIFOn); // fifomap
			fifocfg4_dir |= FIFOMAP_EPNO_NVT(ep->epnum, FIFOn);
                        fifocfg4 |= FIFOCF_FIFO_EN_NVT(1, FIFOn);

			if ((BLKNO > BLKNUM_SINGLE) || (BLKSZ == 1)) {
				pr_err("space not enough\n");
			}
		}
		break;


	default:
		break;

	}

	iowrite32(fifocfg, fotg210->reg + FOTG210_FIFOCF_03);
	iowrite32(fifocfg_dir, fotg210->reg + FOTG210_FIFOMAP_03);
	iowrite32(fifocfg2, fotg210->reg + FOTG210_FIFOCF_47);
	iowrite32(fifocfg2_dir, fotg210->reg + FOTG210_FIFOMAP_47);
	iowrite32(fifocfg3, fotg210->reg + FOTG210_FIFOCF_811);
	iowrite32(fifocfg3_dir, fotg210->reg + FOTG210_FIFOMAP_811);
	iowrite32(fifocfg4, fotg210->reg + FOTG210_FIFOCF_1215);
	iowrite32(fifocfg4_dir, fotg210->reg + FOTG210_FIFOMAP_1215);

	if (debug_on) {
		fifonumsg("%s: [1AC(%x) 1A8(%x)] [1DC(%x) 1D8(%x)] [1E4(%x) 1E0(%x)] [1EC(%x) 1E8(%x)]\n", __func__,
			fifocfg, fifocfg_dir, fifocfg2, fifocfg2_dir, fifocfg3, fifocfg3_dir, fifocfg4, fifocfg4_dir);
	}
#elif PRJ_FIFONUM_4
	struct fotg210_udc *fotg210 = ep->fotg210;
	u32 fifocfg = ioread32(fotg210->reg + FOTG210_FIFOCF_03);
	u32 fifocfg_dir = ioread32(fotg210->reg + FOTG210_FIFOMAP_03);

	switch (FIFOn) {
	case USB_FIFO0: {
			fifocfg |= FIFOCF_TYPE_NVT(BLK_TYP, FIFOn);
			fifocfg |= FIFOCF_BLK_NO(BLKNO-1, FIFOn);
			fifocfg |= FIFOCF_BLKSZ_NVT(BLKSZ, FIFOn);
			fifocfg_dir |= FIFOMAP_DIR(Dir, FIFOn);
			fifocfg_dir &= ~FIFOMAP_EPNOMSK_NVT(FIFOn); // fifomap
			fifocfg_dir |= FIFOMAP_EPNO_NVT(ep->epnum, FIFOn);
			fifocfg |= FIFOCF_FIFO_EN_NVT(1, FIFOn);

			if ((BLKNO == BLKNUM_SINGLE) && (BLKSZ == 1)) {
				fifocfg |= FIFOCF_TYPE_NVT(BLK_TYP, FIFOn+1);
				fifocfg |= FIFOCF_BLK_NO(BLKNO-1, FIFOn+1);
				fifocfg |= FIFOCF_BLKSZ_NVT(BLKSZ, FIFOn+1);
				fifocfg |= FIFOCF_FIFO_EN_NVT(0, FIFOn+1);

			} else if (BLKNO == BLKNUM_DOUBLE) {
				fifocfg |= FIFOCF_TYPE_NVT(BLK_TYP, FIFOn+1);
				fifocfg |= FIFOCF_BLK_NO(BLKNO-1, FIFOn+1);
				fifocfg |= FIFOCF_BLKSZ_NVT(BLKSZ, FIFOn+1);
				fifocfg |= FIFOCF_FIFO_EN_NVT(0, FIFOn+1);

				if (BLKSZ == 1) {
					// Block size : 513~1024 bytes
					fifocfg |= FIFOCF_TYPE_NVT(BLK_TYP, FIFOn+2);
					fifocfg |= FIFOCF_BLK_NO(BLKNO-1, FIFOn+2);
					fifocfg |= FIFOCF_BLKSZ_NVT(BLKSZ, FIFOn+2);
					fifocfg |= FIFOCF_FIFO_EN_NVT(0, FIFOn+2);

					fifocfg |= FIFOCF_TYPE_NVT(BLK_TYP, FIFOn+3);
					fifocfg |= FIFOCF_BLK_NO(BLKNO-1, FIFOn+3);
					fifocfg |= FIFOCF_BLKSZ_NVT(BLKSZ, FIFOn+3);
					fifocfg |= FIFOCF_FIFO_EN_NVT(0, FIFOn+3);
				}

			} else if (BLKNO == BLKNUM_TRIPLE) {
				fifocfg |= FIFOCF_TYPE_NVT(BLK_TYP, FIFOn+1);
				fifocfg |= FIFOCF_BLK_NO(BLKNO-1, FIFOn+1);
				fifocfg |= FIFOCF_BLKSZ_NVT(BLKSZ, FIFOn+1);
				fifocfg |= FIFOCF_FIFO_EN_NVT(0, FIFOn+1);

				fifocfg |= FIFOCF_TYPE_NVT(BLK_TYP, FIFOn+2);
				fifocfg |= FIFOCF_BLK_NO(BLKNO-1, FIFOn+2);
				fifocfg |= FIFOCF_BLKSZ_NVT(BLKSZ, FIFOn+2);
				fifocfg |= FIFOCF_FIFO_EN_NVT(0, FIFOn+2);

				// Block size : 513~1024 bytes. Exceed boundary.
				if (BLKSZ == 1) {
					pr_err("BLKNO %d, BLKSZ %d exceed resource\r\n", BLKNO, BLKSZ);
				}
			}
		}
		break;

	case USB_FIFO1: {
			fifocfg |= FIFOCF_TYPE_NVT(BLK_TYP, FIFOn);
			fifocfg |= FIFOCF_BLK_NO(BLKNO-1, FIFOn);
			fifocfg |= FIFOCF_BLKSZ_NVT(BLKSZ, FIFOn);
			fifocfg_dir |= FIFOMAP_DIR(Dir, FIFOn);
			fifocfg_dir &= ~FIFOMAP_EPNOMSK_NVT(FIFOn); // fifomap
			fifocfg_dir |= FIFOMAP_EPNO_NVT(ep->epnum, FIFOn);
			fifocfg |= FIFOCF_FIFO_EN_NVT(1, FIFOn);

			if ((BLKNO == BLKNUM_SINGLE) && (BLKSZ == 1)) {
				fifocfg |= FIFOCF_TYPE_NVT(BLK_TYP, FIFOn+1);
				fifocfg |= FIFOCF_BLK_NO(BLKNO-1, FIFOn+1);
				fifocfg |= FIFOCF_BLKSZ_NVT(BLKSZ, FIFOn+1);
				fifocfg |= FIFOCF_FIFO_EN_NVT(0, FIFOn+1);

			} else if (BLKNO == BLKNUM_DOUBLE) {
				fifocfg |= FIFOCF_TYPE_NVT(BLK_TYP, FIFOn+1);
				fifocfg |= FIFOCF_BLK_NO(BLKNO-1, FIFOn+1);
				fifocfg |= FIFOCF_BLKSZ_NVT(BLKSZ, FIFOn+1);
				fifocfg |= FIFOCF_FIFO_EN_NVT(0, FIFOn+1);

				// Block size : 513~1024 bytes. Exceed boundary.
				if (BLKSZ == 1) {
					pr_err("FIFOn %d, BLKNO %d, BLKSZ %d exceed resource\r\n", FIFOn, BLKNO, BLKSZ);
				}
			} else if (BLKNO == BLKNUM_TRIPLE) {
				fifocfg |= FIFOCF_TYPE_NVT(BLK_TYP, FIFOn+1);
				fifocfg |= FIFOCF_BLK_NO(BLKNO-1, FIFOn+1);
				fifocfg |= FIFOCF_BLKSZ_NVT(BLKSZ, FIFOn+1);
				fifocfg |= FIFOCF_FIFO_EN_NVT(0, FIFOn+1);

				fifocfg |= FIFOCF_TYPE_NVT(BLK_TYP, FIFOn+2);
				fifocfg |= FIFOCF_BLK_NO(BLKNO-1, FIFOn+2);
				fifocfg |= FIFOCF_BLKSZ_NVT(BLKSZ, FIFOn+2);
				fifocfg |= FIFOCF_FIFO_EN_NVT(0, FIFOn+2);

				// Block size : 513~1024 bytes. Exceed boundary.
				if (BLKSZ == 1) {
					pr_err("FIFOn %d, BLKNO %d, BLKSZ %d exceed resource\r\n", FIFOn, BLKNO, BLKSZ);
				}
			}
		}
		break;

	case USB_FIFO2: {
			fifocfg |= FIFOCF_TYPE_NVT(BLK_TYP, FIFOn);
			fifocfg |= FIFOCF_BLK_NO(BLKNO-1, FIFOn);
			fifocfg |= FIFOCF_BLKSZ_NVT(BLKSZ, FIFOn);
			fifocfg_dir |= FIFOMAP_DIR(Dir, FIFOn);
			fifocfg_dir &= ~FIFOMAP_EPNOMSK_NVT(FIFOn); // fifomap
			fifocfg_dir |= FIFOMAP_EPNO_NVT(ep->epnum, FIFOn);
			fifocfg |= FIFOCF_FIFO_EN_NVT(1, FIFOn);

			if ((BLKNO == BLKNUM_SINGLE) && (BLKSZ == 1)) {
				fifocfg |= FIFOCF_TYPE_NVT(BLK_TYP, FIFOn+1);
				fifocfg |= FIFOCF_BLK_NO(BLKNO-1, FIFOn+1);
				fifocfg |= FIFOCF_BLKSZ_NVT(BLKSZ, FIFOn+1);
				fifocfg |= FIFOCF_FIFO_EN_NVT(0, FIFOn+1);
			} else if (BLKNO == BLKNUM_DOUBLE) {
				fifocfg |= FIFOCF_TYPE_NVT(BLK_TYP, FIFOn+1);
				fifocfg |= FIFOCF_BLK_NO(BLKNO-1, FIFOn+1);
				fifocfg |= FIFOCF_BLKSZ_NVT(BLKSZ, FIFOn+1);
				fifocfg |= FIFOCF_FIFO_EN_NVT(0, FIFOn+1);

				// Block size : 513~1024 bytes
				if (BLKSZ == 1) {
					pr_err("FIFOn %d, BLKNO %d, BLKSZ %d exceed resource\r\n", FIFOn, BLKNO, BLKSZ);
				}
			} else if (BLKNO == BLKNUM_TRIPLE) {
				pr_err("FIFOn %d, BLKNO %d, BLKSZ %d exceed resource\r\n", FIFOn, BLKNO, BLKSZ);
			}
		}
		break;

	case USB_FIFO3: {
			fifocfg |= FIFOCF_TYPE_NVT(BLK_TYP, FIFOn);
			fifocfg |= FIFOCF_BLK_NO(BLKNO-1, FIFOn);
			fifocfg |= FIFOCF_BLKSZ_NVT(BLKSZ, FIFOn);
			fifocfg_dir |= FIFOMAP_DIR(Dir, FIFOn);
			fifocfg_dir &= ~FIFOMAP_EPNOMSK_NVT(FIFOn); // fifomap
			fifocfg_dir |= FIFOMAP_EPNO_NVT(ep->epnum, FIFOn);
			fifocfg |= FIFOCF_FIFO_EN_NVT(1, FIFOn);

			if ((BLKNO > BLKNUM_SINGLE) || (BLKSZ == 1)) {
				pr_err("FIFOn %d, BLKNO %d, BLKSZ %d exceed resource\r\n", FIFOn, BLKNO, BLKSZ);
			}
		}
		break;

	default:
		break;

	}

	iowrite32(fifocfg, fotg210->reg + FOTG210_FIFOCF_03);
	iowrite32(fifocfg_dir, fotg210->reg + FOTG210_FIFOMAP_03);
#else
	pr_err("Please add your project into PRJ_FIFONUM_X macro\n");
#endif
}
static void fotg210_fifo_ep_mapping(struct fotg210_ep *ep, u32 epnum,
                                u32 dir_in, USB_FIFO_NUM FIFOn)
{
	struct fotg210_udc *fotg210 = ep->fotg210;
	u32 val;

	/* Driver should map an ep to a fifo and then map the fifo
	 * to the ep. What a brain-damaged design!
	 */

	if (FIFOn <= USB_FIFO_NOT_USE) {
		pr_err("Please assign correct FIFO number!\n");
		return;
	}

	/* map a fifo to an ep */
	//1. Update registers
	if (epnum <= 4) {
		val = ioread32(fotg210->reg + FOTG210_EPMAP_14);
		val &= ~ EPMAP_FIFONOMSK_NVT(epnum, dir_in);
		val |= EPMAP_FIFONO_NVT(epnum, FIFOn, dir_in);
		iowrite32(val, fotg210->reg + FOTG210_EPMAP_14);
	} else if (epnum <= 8){
		val = ioread32(fotg210->reg + FOTG210_EPMAP_58);
		val &= ~ EPMAP_FIFONOMSK_NVT(epnum, dir_in);
		val |= EPMAP_FIFONO_NVT(epnum, FIFOn, dir_in);
		iowrite32(val, fotg210->reg + FOTG210_EPMAP_58);
	} else if (epnum <= 12){
		val = ioread32(fotg210->reg + FOTG210_EPMAP_912);
		val &= ~ EPMAP_FIFONOMSK_NVT(epnum, dir_in);
		val |= EPMAP_FIFONO_NVT(epnum, FIFOn, dir_in);
		iowrite32(val, fotg210->reg + FOTG210_EPMAP_912);
	} else {
		val = ioread32(fotg210->reg + FOTG210_EPMAP_1315);
		val &= ~ EPMAP_FIFONOMSK_NVT(epnum, dir_in);
		val |= EPMAP_FIFONO_NVT(epnum, FIFOn, dir_in);
		iowrite32(val, fotg210->reg + FOTG210_EPMAP_1315);
	}

	//2. update mapping table
	gEPMap[epnum-1] = FIFOn;


	/* map the ep to the fifo */
	// 1. update mapping table
	if (dir_in == USB_FIFO_IN) {
		//coverity[underrun-local]
		gFIFOInMap[FIFOn] = epnum;
	} else {
		//coverity[underrun-local]
		gFIFOOutMap[FIFOn] = epnum;
	}

	if (debug_on) {
		fifonumsg("%s: [epnum(%d) FIFOn(%d) dir_in(%d)] [gEPMap[%d](%d) gFIFOInMap[%d](%d) gFIFOOutMap[%d](%d)]\r\n", __func__,
			ep->epnum, FIFOn, dir_in, epnum-1, gEPMap[epnum-1], FIFOn, gFIFOInMap[FIFOn], FIFOn, gFIFOOutMap[FIFOn]);
	}

	// 2. set fifo config, and caculate the fifo no for next EP.
	USB_setFIFOCfg(ep, FIFOn, ep->type, gEPBlkNo[epnum-1], (gEPBlkSz[epnum-1] > 512), dir_in);
}

static void fotg210_set_mps(struct fotg210_ep *ep, u32 epnum, u32 mps,
				u32 dir_in)
{
	struct fotg210_udc *fotg210 = ep->fotg210;
	u32 val;
	u32 offset = dir_in ? FOTG210_INEPMPSR(epnum) :
				FOTG210_OUTEPMPSR(epnum);

	val = ioread32(fotg210->reg + offset);
	val |= INOUTEPMPSR_MPS(mps);
	iowrite32(val, fotg210->reg + offset);
}

#define CLEARMASK 0xFFFFFFFF
static int fotg210_config_ep(struct fotg210_ep *ep,
		     const struct usb_endpoint_descriptor *desc)
{
	struct fotg210_udc *fotg210 = ep->fotg210;
	int32_t FIFOn = 0;
	int i = 0;

	fotg210_set_mps(ep, ep->epnum, ep->ep.maxpacket, ep->dir_in); // set packet max size and HBM

	for (i = 0; i < ep->epnum-1; i++) {
		FIFOn += gEPBlkNo[i];

		if (gEPBlkSz[i] > 512)
			 FIFOn += gEPBlkNo[i];
	}

	if (debug_on) {
		fifonumsg("%s: epnum(%d) maxpacket(%d) dir_in(%d) FIFOn(%d)\r\n", __func__, ep->epnum, ep->ep.maxpacket, ep->dir_in, FIFOn);
	}

	fotg210_fifo_ep_mapping(ep, ep->epnum, ep->dir_in, FIFOn);
	fotg210->ep[ep->epnum] = ep;

	return 0;
}

static int fotg210_ep_enable(struct usb_ep *_ep,
			  const struct usb_endpoint_descriptor *desc)
{
	struct fotg210_ep *ep;

	if (debug_on)
		numsg("fotg210_ep_enable\n");

	ep = container_of(_ep, struct fotg210_ep, ep);

	ep->desc = desc;
	ep->epnum = usb_endpoint_num(desc);
	ep->type = usb_endpoint_type(desc);
	ep->dir_in = usb_endpoint_dir_in(desc);
	ep->ep.maxpacket = usb_endpoint_maxp(desc);

	if (debug_on) {
		itfnumsg("fotg210_ep_enable\n");
		itfnumsg("ep-desc len=0x%X type=0x%X epaddr=0x%X attr=0x%X MaxPkt=0x%X intval=0x%X\n",desc->bLength,desc->bDescriptorType
			,desc->bEndpointAddress,desc->bmAttributes,desc->wMaxPacketSize,desc->bInterval);
	}

	return fotg210_config_ep(ep, desc);
}

static void fotg210_reset_tseq(struct fotg210_udc *fotg210, u8 epnum)
{
	struct fotg210_ep *ep = fotg210->ep[epnum];
	u32 value;
	void __iomem *reg;

	reg = (ep->dir_in) ?
		fotg210->reg + FOTG210_INEPMPSR(epnum) :
		fotg210->reg + FOTG210_OUTEPMPSR(epnum);

	/* Note: Driver needs to set and clear INOUTEPMPSR_RESET_TSEQ
	 *	 bit. Controller wouldn't clear this bit. WTF!!!
	 */

	value = ioread32(reg);
	value |= INOUTEPMPSR_RESET_TSEQ;
	iowrite32(value, reg);

	value = ioread32(reg);
	value &= ~INOUTEPMPSR_RESET_TSEQ;
	iowrite32(value, reg);
}

static int fotg210_ep_release(struct fotg210_ep *ep)
{
	if (!ep->epnum)
		return 0;
	ep->epnum = 0;
	ep->stall = 0;
	ep->wedged = 0;

	fotg210_reset_tseq(ep->fotg210, ep->epnum);

	return 0;
}

static int fotg210_ep_disable(struct usb_ep *_ep)
{
	struct fotg210_ep *ep;
	struct fotg210_request *req;
	unsigned long flags;

	BUG_ON(!_ep);

	ep = container_of(_ep, struct fotg210_ep, ep);

	while (!list_empty(&ep->queue)) {
		req = list_entry(ep->queue.next,
			struct fotg210_request, queue);
		spin_lock_irqsave(&ep->fotg210->lock, flags);
		fotg210_done(ep, req, -ECONNRESET);
		spin_unlock_irqrestore(&ep->fotg210->lock, flags);
	}

	return fotg210_ep_release(ep);
}

static struct usb_request *fotg210_ep_alloc_request(struct usb_ep *_ep,
						gfp_t gfp_flags)
{
	struct fotg210_request *req;

	req = kzalloc(sizeof(struct fotg210_request), gfp_flags);
	if (!req)
		return NULL;

	INIT_LIST_HEAD(&req->queue);

	return &req->req;
}

static void fotg210_ep_free_request(struct usb_ep *_ep,
					struct usb_request *_req)
{
	struct fotg210_request *req;

	req = container_of(_req, struct fotg210_request, req);
	kfree(req);
}

#define BC_STUCK_CHK_CNT 300
static void fotg210_enable_dma(struct fotg210_ep *ep,
			      dma_addr_t d, u32 len)
{
	u32 value;
	struct fotg210_udc *fotg210 = ep->fotg210;
	bool cable_out_ret = 0;
	u32 pre_bc = 0;
	u32 cur_bc = 0;
	u32 count = 0;
	u32 pre_dma_remain = 0;
	u32 cur_dma_remain = 0;
	u32 fifox_empty = 0;

	if (debug_on)
		numsg("fotg210_enable_dma ep%d len=%d\n", ep->epnum, len);

	do {
		if (debug_on) {
			numsg("fotg210_enable_dma- wait dma not start\n");
		}
		value = ioread32(fotg210->reg + FOTG210_DMACPSR1);
	}while(value & DMACPSR1_DMA_START);

	/* check if fifo empty */
	if (is_cdc_class) {
		if ((ep->epnum)&&(ep->dir_in)) { //ep1~epn
			do {

				if(gEPMap[ep->epnum -1] < 8 )
				{
					value = ioread32(ep->fotg210->reg + FOTG210_DCFESR);
				}
				else
				{
					value = ioread32(ep->fotg210->reg + FOTG210_DICR);
				}
				fifox_empty = (u32)DCFESR_FIFO_EMPTY(gEPMap[ep->epnum -1]);

			} while(!(value&fifox_empty));
		} else { //ep0
			do {
				value = ioread32(ep->fotg210->reg + FOTG210_DCFESR);
			} while((!(value&DCFESR_CX_EMP)) && (value&DCFESR_CX_FNT_IN));
		}
	} else {
		if ((ep->epnum)&&(ep->dir_in)) {
			do { //ep1~epn
				udelay(plugout_delay);
				// check whether fifo stuck normally during DMA
				cur_bc = (ioread32(ep->fotg210->reg + FOTG210_FIBCR(gEPMap[ep->epnum -1])) & FIBCR_BCFX);
				pre_dma_remain = ioread32(ep->fotg210->reg + FOTG210_DMA_RM_LEN);
				if (debug_on) {
					numsg("[fifo reset]cur_bc is 0x%x, pre_bc is 0x%x\n", cur_bc, pre_bc);
					numsg("[fifo reset]cur_dma_remain is 0x%x, pre_dma_remain is 0x%x\n", cur_dma_remain, pre_dma_remain);
				}

				if ((cur_bc == pre_bc) && (cur_dma_remain == pre_dma_remain)) {
					count++;
					if (count == BC_STUCK_CHK_CNT) {
						numsg("[fifo reset] enter cable out recover, count is %d times\n", count);
						cable_out_ret = fotg210_cable_out_clear(ep);
						return;
					}
				} else {
					pre_bc = cur_bc;
					pre_dma_remain = cur_dma_remain;
					count = 0;
				}

				if(gEPMap[ep->epnum -1] < 8 )
				{
					value = ioread32(ep->fotg210->reg + FOTG210_DCFESR);
				}
				else
				{
					value = ioread32(ep->fotg210->reg + FOTG210_DICR);
				}
			} while (!(value & DCFESR_FIFO_EMPTY(gEPMap[ep->epnum -1])));
		} else { //ep0
			do {
				value = ioread32(ep->fotg210->reg + FOTG210_DCFESR);
			} while((!(value&DCFESR_CX_EMP)) && (value&DCFESR_CX_FNT_IN));
		}

	}

	/* set transfer length and direction */
	value = ioread32(fotg210->reg + FOTG210_DMACPSR1);
	value &= ~(DMACPSR1_DMA_LEN(0x7FFFFF) | DMACPSR1_DMA_TYPE(1));
	value |= DMACPSR1_DMA_LEN(len) | DMACPSR1_DMA_TYPE(ep->dir_in);
	iowrite32(value, fotg210->reg + FOTG210_DMACPSR1);

	/* set device DMA target FIFO number */
	value = ioread32(fotg210->reg + FOTG210_DMATFNR);
	if (ep->epnum) {
		value = DMATFNR_ACC_FN_NVT(gEPMap[ep->epnum -1]);
	} else {
		value = DMATFNR_ACC_CXF;
	}
	//value = DMATFNR_ACC_FN(ep->epnum - 1);
	iowrite32(value, fotg210->reg + FOTG210_DMATFNR);

	/* set DMA memory address */
	iowrite32(d, fotg210->reg + FOTG210_DMACPSR2);

	/* enable MDMA_EROR and MDMA_CMPLT interrupt */
	value = ioread32(fotg210->reg + FOTG210_DMISGR2);
	value &= ~(DMISGR2_MDMA_CMPLT | DMISGR2_MDMA_ERROR);
	iowrite32(value, fotg210->reg + FOTG210_DMISGR2);

	/* start DMA */
	value = ioread32(fotg210->reg + FOTG210_DMACPSR1);
	value |= DMACPSR1_DMA_START;
	iowrite32(value, fotg210->reg + FOTG210_DMACPSR1);
}

static void fotg210_disable_dma(struct fotg210_ep *ep)
{
	//iowrite32(DMATFNR_DISDMA, ep->fotg210->reg + FOTG210_DMATFNR);
}

static void fotg210_wait_dma_done(struct fotg210_ep *ep)
{
	u32 value;
	unsigned long timeout = 0;

	bool cable_out_ret = 0;
	u32 vbus_gpio = 0;
	u32 pre_bc = 0;
	u32 cur_bc = 0;
	u32 count = 0;
	u32 pre_dma_remain = 0;
	u32 cur_dma_remain = 0;

	timeout = jiffies + msecs_to_jiffies(1000);

	if (debug_on)
		numsg("fotg210_wait_dma_done\n");

	if (is_cdc_class || !ep->epnum) {
		do {
			if (debug_on) {
				numsg("cdc case check dma cmplt\n");
			}
			value = ioread32(ep->fotg210->reg + FOTG210_DISGR2);
			if ((value & DISGR2_USBRST_INT) ||
					(value & DISGR2_DMA_ERROR))
				goto dma_reset;

			if (time_after(jiffies, timeout)) {
				pr_err("%s timeout\n", __func__);
				goto dma_reset;
			}
			cpu_relax();

		} while (!(value & DISGR2_DMA_CMPLT));

		value |= DISGR2_DMA_CMPLT;
		iowrite32(value, ep->fotg210->reg + FOTG210_DISGR2);

	} else {
		do {
			udelay(plugout_delay);
			// check whether plugout cable during DMA operating
			vbus_gpio = gpio_get_value(vbus_gpio_no);
			if (!vbus_gpio) {
				if (debug_on) {
					numsg("[dma cmplt chk] vbus_gpio is %d\n", vbus_gpio);

				}
			}

			// check whether fifo stuck during DMA operating
			cur_bc = (ioread32(ep->fotg210->reg + FOTG210_FIBCR(gEPMap[ep->epnum -1])) & FIBCR_BCFX);
			cur_dma_remain = ioread32(ep->fotg210->reg + FOTG210_DMA_RM_LEN);
			if (debug_on) {
				numsg("[dma cmplt chk] cur_bc is 0x%x, pre_bc is 0x%x\n", cur_bc, pre_bc);
				numsg("[dma cmplt chk] cur_dma_remain is 0x%x, pre_dma_remain is 0x%x\n", cur_dma_remain, pre_dma_remain);
			}

			if ((cur_bc == pre_bc) && (cur_dma_remain == pre_dma_remain)) {
				count++;
				if (count == BC_STUCK_CHK_CNT) {
					numsg("[dma cmplt chk] enter cable out recover, count is %d times\n", count);
					cable_out_ret = fotg210_cable_out_clear(ep);
					value |= DISGR2_DMA_CMPLT;
					iowrite32(value, ep->fotg210->reg + FOTG210_DISGR2);
					return;
				}
			} else {
				pre_bc = cur_bc;
				pre_dma_remain = cur_dma_remain;
				count = 0;
			}

			value = ioread32(ep->fotg210->reg + FOTG210_DISGR2);
			if ((value & DISGR2_USBRST_INT) ||
					(value & DISGR2_DMA_ERROR))
				goto dma_reset;

			if (time_after(jiffies, timeout)) {
				pr_err("%s timeout\n", __func__);
				goto dma_reset;
			}
			cpu_relax();

		} while (!(value & DISGR2_DMA_CMPLT));

		value |= DISGR2_DMA_CMPLT;
		iowrite32(value, ep->fotg210->reg + FOTG210_DISGR2);
	}

	if (debug_on) {
		numsg("successful dma wait return\n");
	}
	return;

dma_reset:
	value = ioread32(ep->fotg210->reg + FOTG210_DMACPSR1);
	value |= DMACPSR1_DMA_ABORT;
	iowrite32(value, ep->fotg210->reg + FOTG210_DMACPSR1);

	/* reset fifo */
	if (ep->epnum) {
		//coverity[index_parm]
		value = ioread32(ep->fotg210->reg +
				FOTG210_FIBCR(gEPMap[ep->epnum -1]));
		value |= FIBCR_FFRST;
		iowrite32(value, ep->fotg210->reg +
				FOTG210_FIBCR(gEPMap[ep->epnum -1]));
	} else {
		value = ioread32(ep->fotg210->reg + FOTG210_DCFESR);
		value |= DCFESR_CX_CLR;
		iowrite32(value, ep->fotg210->reg + FOTG210_DCFESR);
	}
}

#if 0
static void ivot_usbdev_start_ep0_data(struct fotg210_ep *ep,
			struct fotg210_request *req)
{
	u32 *buffer;
	u32 value,length,i=0;
	s32	opsize;

	buffer = (u32 *)(req->req.buf + req->req.actual);

	if (req->req.length - req->req.actual > 64)  {
		length = 64;

		if (ep->dir_in) {
			value = ioread32(ep->fotg210->reg +
						FOTG210_DMISGR0);
			value &= ~DMISGR0_MCX_IN_INT;
			iowrite32(value, ep->fotg210->reg + FOTG210_DMISGR0);
		}

	} else {
		length = req->req.length - req->req.actual;

		if (ep->dir_in) {
			value = ioread32(ep->fotg210->reg +
						FOTG210_DMISGR0);
			value |= DMISGR0_MCX_IN_INT;
			iowrite32(value, ep->fotg210->reg + FOTG210_DMISGR0);
		}
	}

	value = ioread32(ep->fotg210->reg + FOTG210_DCFESR);
	if(value & DCFESR_CX_DATAPORT_EN)
		pr_err("DATAPORT EN ERROR!!!\n");

	if (ep->dir_in) {
		if (debug_on)
			ep0numsg("ivot_usbdev_start_ep0_data IN 0x%X  act=0x%X\n",req->req.length,req->req.actual);

		value = ioread32(ep->fotg210->reg + FOTG210_DCFESR);
		value &= ~DCFESR_CX_FNT_IN;
		value |= (length<<16);
		value |= DCFESR_CX_DATAPORT_EN;
		iowrite32(value, ep->fotg210->reg + FOTG210_DCFESR);


		opsize = length;
		while(opsize>0)
		{
			iowrite32(buffer[i++], ep->fotg210->reg + FOTG210_CXDATAPORT);
			opsize-=4;
		}
	} else {
		if (debug_on)
			ep0numsg("ivot_usbdev_start_ep0_data OUT 0x%X  act=0x%X\n",req->req.length,req->req.actual);

		value = ioread32(ep->fotg210->reg + FOTG210_DCFESR);
		value |= DCFESR_CX_DATAPORT_EN;
		iowrite32(value, ep->fotg210->reg + FOTG210_DCFESR);

		opsize = (ioread32(ep->fotg210->reg + FOTG210_DCFESR) & DCFESR_CX_FNT_OUT)>>24;
		while(!opsize) {
			msleep(1);
			opsize = (ioread32(ep->fotg210->reg + FOTG210_DCFESR) & DCFESR_CX_FNT_OUT)>>24;
		}

		while(opsize>0)
		{
			value = ioread32(ep->fotg210->reg + FOTG210_CXDATAPORT);

			if(opsize>=4) {
				buffer[i++] = value;
			} else if (opsize==3) {
				buffer[i] &= ~0xFFFFFF;
				value &= 0xFFFFFF;
				buffer[i] += value;
			} else if (opsize==2) {
				buffer[i] &= ~0xFFFF;
				value &= 0xFFFF;
				buffer[i] += value;
			} else if (opsize==1) {
				buffer[i] &= ~0xFF;
				value &= 0xFF;
				buffer[i] += value;
			}
			opsize-=4;
		}


	}

	value = ioread32(ep->fotg210->reg + FOTG210_DCFESR);
	value &= ~DCFESR_CX_DATAPORT_EN;
	iowrite32(value, ep->fotg210->reg + FOTG210_DCFESR);

	/* update actual transfer length */
	req->req.actual += length;

}
#endif

static void fotg210_start_dma(struct fotg210_ep *ep,
			struct fotg210_request *req)
{
	dma_addr_t d;
	u8 *buffer;
	u32 length,slice,szOp;
	struct fotg210_udc *fotg210 = ep->fotg210;

	if (debug_on)
		numsg("fotg210_start_dma\n");

	if (ep->epnum) {
		if (ep->dir_in) {
			buffer = req->req.buf + req->req.actual;
			length = req->req.length - req->req.actual;
		} else {
			buffer = req->req.buf + req->req.actual;
			length = ioread32(ep->fotg210->reg +
					FOTG210_FIBCR(gEPMap[ep->epnum - 1]));

			length &= FIBCR_BCFX;

			if(length < 512) {
				//short packet
			} else
				length = req->req.length - req->req.actual;

			if (outslice && (ep->type == USB_ENDPOINT_XFER_BULK)) {
				if(length > ep->ep.maxpacket)
					length = ep->ep.maxpacket;
			}
		}
	} else {
		buffer = req->req.buf + req->req.actual;
		length = req->req.length - req->req.actual;

		if(length > 64)
			length = 64;
	}

	szOp = 0;

	while(szOp<length) {

		//if((length - szOp) > 4096)
		//	slice = 4096;
		//else
			slice = (length - szOp);

		d = dma_map_single(fotg210->gadget.dev.parent, (u8 *)(buffer+szOp), slice,
				ep->dir_in ? DMA_TO_DEVICE : DMA_FROM_DEVICE);

		if (dma_mapping_error(fotg210->gadget.dev.parent, d)) {
			pr_err("dma_mapping_error\n");
			return;
		}

		dma_sync_single_for_device(fotg210->gadget.dev.parent, d, slice,
					   ep->dir_in ? DMA_TO_DEVICE :
						DMA_FROM_DEVICE);

		fotg210_enable_dma(ep, d, slice);

		/* check if dma is done */
		fotg210_wait_dma_done(ep);

		fotg210_disable_dma(ep);

		szOp+=slice;
		dma_unmap_single(fotg210->gadget.dev.parent, d, slice,
				ep->dir_in ? DMA_TO_DEVICE :
					DMA_FROM_DEVICE);
	}

	/* update actual transfer length */
	req->req.actual += length;
}

static void fotg210_ep0_queue(struct fotg210_ep *ep,
				struct fotg210_request *req)
{
	if (!req->req.length) {
		fotg210_done(ep, req, 0);
		return;
	}
	if (ep->dir_in) { /* if IN */
		if (req->req.length) {
			//ivot_usbdev_start_ep0_data(ep, req);
			fotg210_start_dma(ep, req);
		} else {
			pr_err("%s : req->req.length = 0x%x\n",
			       __func__, req->req.length);
		}
		if ((req->req.length == req->req.actual) ||
		    (req->req.actual < ep->ep.maxpacket))
			fotg210_done(ep, req, 0);
	} else { /* OUT */
		/* For set_feature(TEST_PACKET) */
		if (do_test_packet && req->req.length == 53) {
			ep->dir_in = 1;
			fotg210_start_dma(ep, req);
			do_test_packet = 0;
		} else {
			u32 value = ioread32(ep->fotg210->reg + FOTG210_DMISGR0);

			value &= ~DMISGR0_MCX_OUT_INT;
			iowrite32(value, ep->fotg210->reg + FOTG210_DMISGR0);
		}
	}
}

static int fotg210_ep_queue(struct usb_ep *_ep, struct usb_request *_req,
				gfp_t gfp_flags)
{
	struct fotg210_ep *ep;
	struct fotg210_request *req;
	unsigned long flags;
	int request = 0;

	ep = container_of(_ep, struct fotg210_ep, ep);
	req = container_of(_req, struct fotg210_request, req);

	if (ep->fotg210->gadget.speed == USB_SPEED_UNKNOWN)
		return -ESHUTDOWN;

	spin_lock_irqsave(&ep->fotg210->lock, flags);

	if (list_empty(&ep->queue))
		request = 1;

	list_add_tail(&req->queue, &ep->queue);

	req->req.actual = 0;
	req->req.status = -EINPROGRESS;

	if (!ep->epnum) /* ep0 */
		fotg210_ep0_queue(ep, req);
	else if (request && !ep->stall)
		fotg210_enable_fifo_int(ep);

	spin_unlock_irqrestore(&ep->fotg210->lock, flags);

	return 0;
}

static int fotg210_ep_dequeue(struct usb_ep *_ep, struct usb_request *_req)
{
	struct fotg210_ep *ep;
	struct fotg210_request *req;
	unsigned long flags;

	ep = container_of(_ep, struct fotg210_ep, ep);
	req = container_of(_req, struct fotg210_request, req);

	spin_lock_irqsave(&ep->fotg210->lock, flags);
	if (!list_empty(&ep->queue))
		fotg210_done(ep, req, -ECONNRESET);
	spin_unlock_irqrestore(&ep->fotg210->lock, flags);

	return 0;
}

static void fotg210_set_epnstall(struct fotg210_ep *ep)
{
	struct fotg210_udc *fotg210 = ep->fotg210;
	u32 value;
	void __iomem *reg;

	/* check if IN FIFO is empty before stall */
	if (ep->epnum) {
		if (ep->dir_in) {
			do {
				if (gEPMap[ep->epnum -1] < 8)
				{
					value = ioread32(ep->fotg210->reg + FOTG210_DCFESR);
				}
				else
				{
					value = ioread32(ep->fotg210->reg + FOTG210_DICR);
				}

				if (debug_on) {
					fifonumsg("%s: [epnum(%d) dir_in(%d)] [gEPMap[%d](%d)] [value(0x%x) DCFESR_FIFO_EMPTY(0x%x)]\n", __func__,
						ep->epnum, ep->dir_in, ep->epnum -1, gEPMap[ep->epnum -1], value, DCFESR_FIFO_EMPTY(gEPMap[ep->epnum - 1]));
				}
			} while (!(value & DCFESR_FIFO_EMPTY(gEPMap[ep->epnum - 1])));
		}
		reg = (ep->dir_in) ?
			fotg210->reg + FOTG210_INEPMPSR(ep->epnum) :
			fotg210->reg + FOTG210_OUTEPMPSR(ep->epnum);

		value = ioread32(reg);
		value |= INOUTEPMPSR_STL_EP;
		iowrite32(value, reg);
	} else {
		reg = fotg210->reg + FOTG210_DCFESR;
		value = ioread32(reg);
		value |= (DCFESR_CX_STL);
		iowrite32(value, reg);
		fotg210_set_cxdone(fotg210);
	}
}

static void fotg210_clear_epnstall(struct fotg210_ep *ep)
{
	struct fotg210_udc *fotg210 = ep->fotg210;
	u32 value;
	void __iomem *reg;

	reg = (ep->dir_in) ?
		fotg210->reg + FOTG210_INEPMPSR(ep->epnum) :
		fotg210->reg + FOTG210_OUTEPMPSR(ep->epnum);
	value = ioread32(reg);
	value &= ~INOUTEPMPSR_STL_EP;
	iowrite32(value, reg);
}

static int fotg210_set_halt_and_wedge(struct usb_ep *_ep, int value, int wedge, int bisneedlock)
{
	struct fotg210_ep *ep;
	struct fotg210_udc *fotg210;
	unsigned long flags;
	int ret = 0;

	ep = container_of(_ep, struct fotg210_ep, ep);

	fotg210 = ep->fotg210;

	if(bisneedlock)
		spin_lock_irqsave(&ep->fotg210->lock, flags);

	if (value) {
		fotg210_set_epnstall(ep);
		ep->stall = 1;
		if (wedge)
			ep->wedged = 1;
	} else {
		fotg210_reset_tseq(fotg210, ep->epnum);
		fotg210_clear_epnstall(ep);
		ep->stall = 0;
		ep->wedged = 0;
		if (!list_empty(&ep->queue))
			fotg210_enable_fifo_int(ep);
	}

	if(bisneedlock)
		spin_unlock_irqrestore(&ep->fotg210->lock, flags);

	return ret;
}

static int fotg210_ep_set_halt(struct usb_ep *_ep, int value)
{
	return fotg210_set_halt_and_wedge(_ep, value, 0, 1);
}

static int fotg210_ep_set_wedge(struct usb_ep *_ep)
{
	return fotg210_set_halt_and_wedge(_ep, 1, 1, 1);
}

static void fotg210_ep_fifo_flush(struct usb_ep *_ep)
{
}

static const struct usb_ep_ops fotg210_ep_ops = {
	.enable		= fotg210_ep_enable,
	.disable	= fotg210_ep_disable,

	.alloc_request	= fotg210_ep_alloc_request,
	.free_request	= fotg210_ep_free_request,

	.queue		= fotg210_ep_queue,
	.dequeue	= fotg210_ep_dequeue,

	.set_halt	= fotg210_ep_set_halt,
	.fifo_flush	= fotg210_ep_fifo_flush,
	.set_wedge	= fotg210_ep_set_wedge,
};

static void fotg210_set_tx0byte(struct fotg210_ep *ep)
{
	struct fotg210_udc *fotg210 = ep->fotg210;
	u32 val;
	u32 offset = (ep->dir_in) ? FOTG210_INEPMPSR(ep->epnum) :
				FOTG210_OUTEPMPSR(ep->epnum);
	bool cable_out_ret = 0;
	u32 vbus_gpio = 0;
	u32 pre_bc = 0;
	u32 cur_bc = 0;
	u32 count = 0;
	u32 pre_dma_remain = 0;
	u32 cur_dma_remain = 0;

	val = ioread32(fotg210->reg + offset);
	val |= INOUTEPMPSR_TX0BYTE_IEP;
	iowrite32(val, fotg210->reg + offset);

	do{
		udelay(plugout_delay);
		// check whether plugout cable during DMA
		vbus_gpio = gpio_get_value(vbus_gpio_no);
		if (!vbus_gpio) {
			if (debug_on) {
				numsg("[set_tx0byte]vbus_gpio is %d\n", vbus_gpio);
			}
		}

		// check whether fifo stuck normally during DMA
		cur_bc = (ioread32(ep->fotg210->reg + FOTG210_FIBCR(gEPMap[ep->epnum -1])) & FIBCR_BCFX);
		pre_dma_remain = ioread32(ep->fotg210->reg + FOTG210_DMA_RM_LEN);
		if (debug_on) {
			numsg("[set_tx0byte] cur_bc is 0x%x, pre_bc is 0x%x\n", cur_bc, pre_bc);
			numsg("[set_tx0byte] cur_dma_remain is 0x%x, pre_dma_remain is 0x%x\n", cur_dma_remain, pre_dma_remain);
		}

		if ((cur_bc == pre_bc) && (cur_dma_remain == pre_dma_remain)) {
			count++;
			if (count == BC_STUCK_CHK_CNT) {
				numsg("[set_tx0byte] enter cable out recover, count is %d times\n", count);
				cable_out_ret = fotg210_cable_out_clear(ep);
				return;
			}
		} else {
			pre_bc = cur_bc;
			pre_dma_remain = cur_dma_remain;
			count = 0;
		}

		val = ioread32(fotg210->reg + offset);
	}while(val & INOUTEPMPSR_TX0BYTE_IEP);

	return;
}

static void fotg210_clear_tx0byte(struct fotg210_udc *fotg210)
{
	u32 value = ioread32(fotg210->reg + FOTG210_TX0BYTE);

	value &= (TX0BYTE_EP1 | TX0BYTE_EP2 | TX0BYTE_EP3
		   | TX0BYTE_EP4 | TX0BYTE_EP5 | TX0BYTE_EP6 | TX0BYTE_EP7 | TX0BYTE_EP8
		   | TX0BYTE_EP9 | TX0BYTE_EP10 | TX0BYTE_EP11 | TX0BYTE_EP12 | TX0BYTE_EP13 | TX0BYTE_EP14 | TX0BYTE_EP15);
	iowrite32(value, fotg210->reg + FOTG210_TX0BYTE);
}

static void fotg210_clear_rx0byte(struct fotg210_udc *fotg210)
{
	u32 value = ioread32(fotg210->reg + FOTG210_RX0BYTE);

	value &= (RX0BYTE_EP1 | RX0BYTE_EP2 | RX0BYTE_EP3
		   | RX0BYTE_EP4 | RX0BYTE_EP5 | RX0BYTE_EP6 | RX0BYTE_EP7 | RX0BYTE_EP8
		   | RX0BYTE_EP9 | RX0BYTE_EP10 | RX0BYTE_EP11 | RX0BYTE_EP12 | RX0BYTE_EP13 | RX0BYTE_EP14 | RX0BYTE_EP15);
	iowrite32(value, fotg210->reg + FOTG210_RX0BYTE);
}

/* read 8-byte setup packet only */
static void fotg210_rdsetupp(struct fotg210_udc *fotg210,
		   u8 *buffer)
{
#if 1
	u32 *tmp32;
	tmp32 = (u32 *)buffer;
	*tmp32++ = ioread32(fotg210->reg + FOTG210_CXPORT);
	*tmp32   = ioread32(fotg210->reg + FOTG210_CXPORT);

	if (debug_on)
		ep0numsg("SETUP 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X\n"
			,buffer[0],buffer[1],buffer[2],buffer[3],buffer[4],buffer[5],buffer[6],buffer[7]);

#else
	int i = 0;
	u8 *tmp = buffer;
	u32 data;
	u32 length = 8;

	iowrite32(DMATFNR_ACC_CXF, fotg210->reg + FOTG210_DMATFNR);

	for (i = (length >> 2); i > 0; i--) {
		data = ioread32(fotg210->reg + FOTG210_CXPORT);
		*tmp = data & 0xFF;
		*(tmp + 1) = (data >> 8) & 0xFF;
		*(tmp + 2) = (data >> 16) & 0xFF;
		*(tmp + 3) = (data >> 24) & 0xFF;
		tmp = tmp + 4;
	}

	switch (length % 4) {
	case 1:
		data = ioread32(fotg210->reg + FOTG210_CXPORT);
		*tmp = data & 0xFF;
		break;
	case 2:
		data = ioread32(fotg210->reg + FOTG210_CXPORT);
		*tmp = data & 0xFF;
		*(tmp + 1) = (data >> 8) & 0xFF;
		break;
	case 3:
		data = ioread32(fotg210->reg + FOTG210_CXPORT);
		*tmp = data & 0xFF;
		*(tmp + 1) = (data >> 8) & 0xFF;
		*(tmp + 2) = (data >> 16) & 0xFF;
		break;
	default:
		break;
	}

	iowrite32(DMATFNR_DISDMA, fotg210->reg + FOTG210_DMATFNR);
#endif

}

static void fotg210_set_configuration(struct fotg210_udc *fotg210)
{
	u32 value = ioread32(fotg210->reg + FOTG210_DAR);

	value |= DAR_AFT_CONF;
	iowrite32(value, fotg210->reg + FOTG210_DAR);
}

static void fotg210_set_dev_addr(struct fotg210_udc *fotg210, u32 addr)
{
	u32 value = ioread32(fotg210->reg + FOTG210_DAR);

	value |= (addr & 0x7F);
	iowrite32(value, fotg210->reg + FOTG210_DAR);
}

static void fotg210_set_cxstall(struct fotg210_udc *fotg210)
{
	u32 value = ioread32(fotg210->reg + FOTG210_DCFESR);

	value |= DCFESR_CX_STL;
	iowrite32(value, fotg210->reg + FOTG210_DCFESR);
}

static void fotg210_request_error(struct fotg210_udc *fotg210)
{
	fotg210_set_cxstall(fotg210);
	pr_err("request error!!\n");
}

static void fotg210_set_address(struct fotg210_udc *fotg210,
				struct usb_ctrlrequest *ctrl)
{
	if (ctrl->wValue >= 0x0100) {
		fotg210_request_error(fotg210);
	} else {
		fotg210_set_dev_addr(fotg210, ctrl->wValue);
		fotg210_set_cxdone(fotg210);
	}
}

static void gen_test_packet(u8 *tst_packet)
{
	u8 *tp = tst_packet;

	int i;
	for (i = 0; i < 9; i++)/*JKJKJKJK x 9*/
		*tp++ = 0x00;

	for (i = 0; i < 8; i++) /* 8*AA */
		*tp++ = 0xAA;

	for (i = 0; i < 8; i++) /* 8*EE */
		*tp++ = 0xEE;

	*tp++ = 0xFE;

	for (i = 0; i < 11; i++) /* 11*FF */
		*tp++ = 0xFF;

	*tp++ = 0x7F;
	*tp++ = 0xBF;
	*tp++ = 0xDF;
	*tp++ = 0xEF;
	*tp++ = 0xF7;
	*tp++ = 0xFB;
	*tp++ = 0xFD;
	*tp++ = 0xFC;
	*tp++ = 0x7E;
	*tp++ = 0xBF;
	*tp++ = 0xDF;
	*tp++ = 0xEF;
	*tp++ = 0xF7;
	*tp++ = 0xFB;
	*tp++ = 0xFD;
	*tp++ = 0x7E;
}

static void fotg210_set_feature(struct fotg210_udc *fotg210,
				struct usb_ctrlrequest *ctrl)
{
	u16 w_index = le16_to_cpu(ctrl->wIndex);
	u16 w_value = le16_to_cpu(ctrl->wValue);
	u32 val;
	u8 tst_packet[53];

	switch (ctrl->bRequestType & USB_RECIP_MASK) {
	case USB_RECIP_DEVICE:
		switch(w_value) {
		case USB_DEVICE_TEST_MODE:
			switch (w_index >> 8) {
			case USB_TEST_J:
				pr_info("\n...TEST_J...\n");
				iowrite32(PHYTMSR_TST_JSTA,
					fotg210->reg + FOTG210_PHYTMSR);
				fotg210_set_cxdone(fotg210);
				break;
			case USB_TEST_K:
				pr_info("\n...TEST_K...\n");
				iowrite32(PHYTMSR_TST_KSTA,
					fotg210->reg + FOTG210_PHYTMSR);
				fotg210_set_cxdone(fotg210);
				break;
			case USB_TEST_SE0_NAK:
				pr_info("\n...TEST_SE0_NAK...\n");
				iowrite32(PHYTMSR_TST_SE0NAK,
					fotg210->reg + FOTG210_PHYTMSR);
				fotg210_set_cxdone(fotg210);
				break;
			case USB_TEST_PACKET:
				pr_info("\n...TEST_PACKET1...\n");
				iowrite32(PHYTMSR_TST_PKT,
					fotg210->reg + FOTG210_PHYTMSR);
				fotg210_set_cxdone(fotg210);

				gen_test_packet(tst_packet);

				fotg210->ep0_req->buf = tst_packet;
				fotg210->ep0_req->length = 53;
				do_test_packet = 1;
				pr_info("\n...TEST_PACKET2...\n");
				//spin_unlock(&fotg210->lock);
				fotg210_ep_queue(fotg210->gadget.ep0,
						fotg210->ep0_req, GFP_KERNEL);
				//spin_lock(&fotg210->lock);

				//Test Packet Done set
				val = ioread32(fotg210->reg + FOTG210_DCFESR);
				val |= DCFESR_TST_PKDONE;
				iowrite32(val, fotg210->reg + FOTG210_DCFESR);
				pr_info("\n...TEST_PACKET3...\n");
				break;
			case USB_TEST_FORCE_ENABLE:
				fotg210_set_cxdone(fotg210);
				break;
			default:
				fotg210_request_error(fotg210);
				break;
			}
			break;
		case USB_DEVICE_REMOTE_WAKEUP:
			fotg210->remote_wkp = 1;
			fallthrough;
		default:
			fotg210_set_cxdone(fotg210);
			break;
		}
		break;
	case USB_RECIP_INTERFACE:
		fotg210_set_cxdone(fotg210);
		break;
	case USB_RECIP_ENDPOINT: {
		u8 epnum;
		epnum = le16_to_cpu(ctrl->wIndex) & USB_ENDPOINT_NUMBER_MASK;
		if (epnum)
		{
			fotg210_set_epnstall(fotg210->ep[epnum]);
			fotg210->ep[epnum]->stall = 1;
		}
		else
			fotg210_set_cxstall(fotg210);
		fotg210_set_cxdone(fotg210);
		}
		break;
	default:
		fotg210_request_error(fotg210);
		break;
	}
}

static void fotg210_clear_feature(struct fotg210_udc *fotg210,
				struct usb_ctrlrequest *ctrl)
{
	struct fotg210_ep *ep =
		fotg210->ep[ctrl->wIndex & USB_ENDPOINT_NUMBER_MASK];

	switch (ctrl->bRequestType & USB_RECIP_MASK) {
	case USB_RECIP_DEVICE:
		if(ctrl->wValue == USB_DEVICE_REMOTE_WAKEUP) {
			fotg210->remote_wkp = 0;
		}
		fotg210_set_cxdone(fotg210);
		break;
	case USB_RECIP_INTERFACE:
		fotg210_set_cxdone(fotg210);
		break;
	case USB_RECIP_ENDPOINT:
		if (ctrl->wIndex & USB_ENDPOINT_NUMBER_MASK) {
			if (ep->wedged) {
				fotg210_set_cxdone(fotg210);
				break;
			}
			if (ep->stall)
				fotg210_set_halt_and_wedge(&ep->ep, 0, 0, 0);
		}
		fotg210_set_cxdone(fotg210);
		break;
	default:
		fotg210_request_error(fotg210);
		break;
	}
}

static int fotg210_is_epnstall(struct fotg210_ep *ep)
{
	struct fotg210_udc *fotg210 = ep->fotg210;
	u32 value;
	void __iomem *reg;

	reg = (ep->dir_in) ?
		fotg210->reg + FOTG210_INEPMPSR(ep->epnum) :
		fotg210->reg + FOTG210_OUTEPMPSR(ep->epnum);
	value = ioread32(reg);
	return value & INOUTEPMPSR_STL_EP ? 1 : 0;
}

static void fotg210_get_status(struct fotg210_udc *fotg210,
				struct usb_ctrlrequest *ctrl)
{
	u8 epnum;

	switch (ctrl->bRequestType & USB_RECIP_MASK) {
	case USB_RECIP_DEVICE:
		fotg210->ep0_data = 1 << USB_DEVICE_SELF_POWERED;
		fotg210->ep0_data |= (fotg210->remote_wkp << USB_DEVICE_REMOTE_WAKEUP);
		break;
	case USB_RECIP_INTERFACE:
		fotg210->ep0_data = 0;
		break;
	case USB_RECIP_ENDPOINT:
		epnum = ctrl->wIndex & USB_ENDPOINT_NUMBER_MASK;
		if (epnum)
			fotg210->ep0_data =
				fotg210_is_epnstall(fotg210->ep[epnum])
				<< USB_ENDPOINT_HALT;
		else
			fotg210_request_error(fotg210);
		break;

	default:
		fotg210_request_error(fotg210);
		return;		/* exit */
	}

	fotg210->ep0_req->buf = &fotg210->ep0_data;
	fotg210->ep0_req->length = 2;

	spin_unlock(&fotg210->lock);
	fotg210_ep_queue(fotg210->gadget.ep0, fotg210->ep0_req, GFP_ATOMIC);
	spin_lock(&fotg210->lock);
}

static int fotg210_setup_packet(struct fotg210_udc *fotg210,
				struct usb_ctrlrequest *ctrl)
{
	u8 *p = (u8 *)ctrl;
	u8 ret = 0;
	u32 reg;

	fotg210_rdsetupp(fotg210, p);

	fotg210->ep[0]->dir_in = ctrl->bRequestType & USB_DIR_IN;

	if (fotg210->gadget.speed == USB_SPEED_UNKNOWN) {
		u32 value = ioread32(fotg210->reg + FOTG210_DMCR);
		fotg210->gadget.speed = value & DMCR_HS_EN ?
				USB_SPEED_HIGH : USB_SPEED_FULL;
	}

	/* check request */
	if ((ctrl->bRequestType & USB_TYPE_MASK) == USB_TYPE_STANDARD) {
		switch (ctrl->bRequest) {
		case USB_REQ_GET_STATUS:
			fotg210_get_status(fotg210, ctrl);
			break;
		case USB_REQ_CLEAR_FEATURE:
			fotg210_clear_feature(fotg210, ctrl);
			break;
		case USB_REQ_SET_FEATURE:
			fotg210_set_feature(fotg210, ctrl);
			break;
		case USB_REQ_SET_ADDRESS:
			fotg210_set_address(fotg210, ctrl);
			break;
		case USB_REQ_SET_CONFIGURATION:
			fotg210_set_configuration(fotg210);
			ret = 1;
			break;
		default:
			ret = 1;
			break;
		}
	} else {
		if (debug_on)
			numsg("none standard request, ctrl->bRequestType is 0x%x, ctrl->bRequest is 0x%x\n",  ctrl->bRequestType, ctrl->bRequest);

		if ((ctrl->bRequestType & 0xFF) == USB_CDC_REQ_GET_LINE_CODING) {
			if (debug_on) {
				numsg("CDC case\n");
			}
			is_cdc_class = 1;
		}

		if ((ctrl->bRequest == UVC_GET_CUR) || (ctrl->bRequest == UVC_GET_MIN) || (ctrl->bRequest == UVC_GET_MAX) || (ctrl->bRequest == UVC_GET_RES)
				|| (ctrl->bRequest ==  UVC_GET_LEN) || (ctrl->bRequest ==  UVC_GET_DEF) || (ctrl->bRequest == UVC_GET_INFO)) {

			fotg210_set_cxdone(fotg210);
		}

		ret = 1;
	}

	if(fotg210->ep[0]->dir_in) {
		reg = ioread32(fotg210->reg + FOTG210_DMISGR0);
		reg &= ~DMISGR0_MCX_IN_INT;
		iowrite32(reg, fotg210->reg + FOTG210_DMISGR0);
	}

	return ret;
}

static void fotg210_ep0out(struct fotg210_udc *fotg210)
{
	struct fotg210_ep *ep = fotg210->ep[0];

	if (!list_empty(&ep->queue) && !ep->dir_in) {
		struct fotg210_request *req;

		req = list_first_entry(&ep->queue,
			struct fotg210_request, queue);

		if (req->req.length)
			//ivot_usbdev_start_ep0_data(ep, req);
			fotg210_start_dma(ep, req);

		if ((req->req.length - req->req.actual) < ep->ep.maxpacket)
			fotg210_done(ep, req, 0);
	} else {
		pr_err("%s : empty queue\n", __func__);
	}
}

static void fotg210_ep0in(struct fotg210_udc *fotg210)
{
	struct fotg210_ep *ep = fotg210->ep[0];
	u32 reg;

	if ((!list_empty(&ep->queue)) && (ep->dir_in)) {
		struct fotg210_request *req;

		req = list_entry(ep->queue.next,
				struct fotg210_request, queue);

		if (req->req.length)
			//ivot_usbdev_start_ep0_data(ep, req);
			fotg210_start_dma(ep, req);

		if (req->req.length == req->req.actual)
			fotg210_done(ep, req, 0);
	} else {
		reg = ioread32(fotg210->reg + FOTG210_DMISGR0);
		reg |= DMISGR0_MCX_IN_INT;
		iowrite32(reg, fotg210->reg + FOTG210_DMISGR0);
		fotg210_set_cxdone(fotg210);
	}
}

static void fotg210_clear_comabt_int(struct fotg210_udc *fotg210)
{
	u32 value = ioread32(fotg210->reg + FOTG210_DISGR0);

	value &= ~DISGR0_CX_COMABT_INT;
	iowrite32(value, fotg210->reg + FOTG210_DISGR0);
}

static void fotg210_in_fifo_handler(struct fotg210_ep *ep)
{
	struct fotg210_request *req = list_entry(ep->queue.next,
					struct fotg210_request, queue);

	if (req->req.length)
	{
		fotg210_start_dma(ep, req);
	}
	else
	{
		fotg210_set_tx0byte(ep);
	}
	if (req->req.length == req->req.actual)
		fotg210_done(ep, req, 0);
}

static void fotg210_out_fifo_handler(struct fotg210_ep *ep)
{
	struct fotg210_request *req = list_entry(ep->queue.next,
						 struct fotg210_request, queue);

	fotg210_start_dma(ep, req);

	/* finish out transfer */
	if (req->req.length == req->req.actual ||
	    (req->req.actual & (ep->ep.maxpacket-1))) {
		fotg210_done(ep, req, 0);
	}
}

static irqreturn_t fotg210_irq(int irq, void *_fotg210)
{
	struct fotg210_udc *fotg210 = _fotg210;
	u32 int_grp = ioread32(fotg210->reg + FOTG210_DIGR);
	u32 int_msk = ioread32(fotg210->reg + FOTG210_DMIGR);
	int EPn;
	u32 val;

	int_grp &= ~int_msk;

	spin_lock(&fotg210->lock);

	if (int_grp & DIGR_INT_G2) {
		void __iomem *reg = fotg210->reg + FOTG210_DISGR2;
		u32 int_grp2 = ioread32(reg);
		u32 int_msk2 = ioread32(fotg210->reg + FOTG210_DMISGR2);
		u32 value;

		int_grp2 &= ~int_msk2;

		if (int_grp2 & DISGR2_USBRST_INT) {
			iowrite32(DISGR2_USBRST_INT, reg);

			// reset releated settings when bus reset got
			iowrite32(0x00420000, fotg210->reg + FOTG210_DAR);

			iowrite32(0x00000200, fotg210->reg + 0x160);
			iowrite32(0x00000200, fotg210->reg + 0x164);
			iowrite32(0x00000200, fotg210->reg + 0x168);
			iowrite32(0x00000200, fotg210->reg + 0x16C);
			iowrite32(0x00000200, fotg210->reg + 0x170);
			iowrite32(0x00000200, fotg210->reg + 0x174);
			iowrite32(0x00000200, fotg210->reg + 0x178);
			iowrite32(0x00000200, fotg210->reg + 0x17C);
			iowrite32(0x00000200, fotg210->reg + 0x180);
			iowrite32(0x00000200, fotg210->reg + 0x184);
			iowrite32(0x00000200, fotg210->reg + 0x188);
			iowrite32(0x00000200, fotg210->reg + 0x18C);
			iowrite32(0x00000200, fotg210->reg + 0x190);
			iowrite32(0x00000200, fotg210->reg + 0x194);
			iowrite32(0x00000200, fotg210->reg + 0x198);
			iowrite32(0x00000200, fotg210->reg + 0x19C);

			iowrite32(0xFFFFFFFF, fotg210->reg + 0x1A0);
			iowrite32(0xFFFFFFFF, fotg210->reg + 0x1A4);
			iowrite32(0x0F0F0F0F, fotg210->reg + 0x1A8);
			iowrite32(0x00000000, fotg210->reg + 0x1AC);
#ifdef PRJ_FIFONUM_16
			iowrite32(0x0F0F0F0F, fotg210->reg + 0x1D8);
			iowrite32(0x00000000, fotg210->reg + 0x1DC);
#endif
			if (debug_on)
				devnumsg("fotg210 udc reset\n");
		}
		if (int_grp2 & DISGR2_SUSP_INT) {
			iowrite32(DISGR2_SUSP_INT, reg);

			if(fotg210->remote_wkp) {
				printk("!!go suspend\n");
				val = ioread32(fotg210->reg + FOTG210_DMCR);
				val |= DMCR_GOSUSP;
				iowrite32(val, fotg210->reg + FOTG210_DMCR);
			}
			if (debug_on)
				devnumsg("fotg210 udc suspend\n");
		}
		if (int_grp2 & DISGR2_RESM_INT) {
			iowrite32(DISGR2_RESM_INT, reg);
			if (debug_on)
				devnumsg("fotg210 udc resume\n");
		}
		if (int_grp2 & DISGR2_ISO_SEQ_ERR_INT) {
			iowrite32(DISGR2_ISO_SEQ_ERR_INT, reg);
			if (debug_on)
				devnumsg("fotg210 iso sequence error\n");
		}
		if (int_grp2 & DISGR2_ISO_SEQ_ABORT_INT) {
			iowrite32(DISGR2_ISO_SEQ_ABORT_INT, reg);
			if (debug_on)
				devnumsg("fotg210 iso sequence abort\n");
		}
		if (int_grp2 & DISGR2_TX0BYTE_INT) {
			iowrite32(DISGR2_TX0BYTE_INT, reg);
			fotg210_clear_tx0byte(fotg210);
			if (debug_on)
				numsg("fotg210 transferred 0 byte\n");
		}
		if (int_grp2 & DISGR2_RX0BYTE_INT) {
			iowrite32(DISGR2_RX0BYTE_INT, reg);
			fotg210_clear_rx0byte(fotg210);
			if (debug_on)
				numsg("fotg210 received 0 byte\n");
		}
		if (int_grp2 & DISGR2_DMA_ERROR) {
			value = ioread32(reg);
			value &= ~DISGR2_DMA_ERROR;
			iowrite32(value, reg);
			if (debug_on)
				numsg("fotg210 DISGR2_DMA_ERROR\n");
		}

		for (EPn = 0; EPn < (FOTG210_MAX_NUM_EP -1); EPn++) {
			if (gEPMap[EPn] == USB_FIFO_NOT_USE || gEPMap[EPn] < 8 )
				continue;

			if (((int_grp2&0xFFFF0000) & DISGR2_OUT_INT(gEPMap[EPn])) ||
					((int_grp2&0xFFFF0000) & DISGR2_SPK_INT(gEPMap[EPn])))
				fotg210_out_fifo_handler(fotg210->ep[EPn+1]);
		}
	}

	if (int_grp & DIGR_INT_G0) {
		void __iomem *reg = fotg210->reg + FOTG210_DISGR0;
		u32 int_grp0 = ioread32(reg);
		u32 int_msk0 = ioread32(fotg210->reg + FOTG210_DMISGR0);
		struct usb_ctrlrequest ctrl;

		int_grp0 &= ~int_msk0;
		if (debug_on)
			numsg("G0 0x%08X\n",int_grp0);

		/* the highest priority in this source register */
		if (int_grp0 & DISGR0_CX_COMABT_INT) {
			fotg210_clear_comabt_int(fotg210);
			pr_info("fotg210 CX command abort\n");
		}

		if (int_grp0 & DISGR0_CX_SETUP_INT) {
			if (fotg210_setup_packet(fotg210, &ctrl)) {
				spin_unlock(&fotg210->lock);
				if (fotg210->driver->setup(&fotg210->gadget,
							   &ctrl) < 0) {
					if (ctrl.bRequestType & USB_DIR_IN) {
						fotg210_set_cxstall(fotg210);
					} else {
						fotg210_set_cxstall(fotg210);
						fotg210_set_cxdone(fotg210);
					}
				}
				spin_lock(&fotg210->lock);
			}
		}
		if (int_grp0 & DISGR0_CX_COMEND_INT)
			pr_info("fotg210 cmd end\n");

		if (int_grp0 & DISGR0_CX_IN_INT)
			fotg210_ep0in(fotg210);

		if (int_grp0 & DISGR0_CX_OUT_INT) {
			val = ioread32(fotg210->reg + FOTG210_DMISGR0);
			val |= DMISGR0_MCX_OUT_INT;
			iowrite32(val, fotg210->reg + FOTG210_DMISGR0);
			fotg210_ep0out(fotg210);
		}

		if (int_grp0 & DISGR0_CX_COMFAIL_INT) {
			fotg210_set_cxstall(fotg210);
			pr_info("fotg210 ep0 fail\n");
		}
	}

	if (int_grp & DIGR_INT_G1) {
		void __iomem *reg = fotg210->reg + FOTG210_DISGR1;
		u32 int_grp1 = ioread32(reg);
		u32 int_msk1 = ioread32(fotg210->reg + FOTG210_DMISGR1);

		if (debug_on)
			numsg("G1 0x%08X\n",int_grp1);

		int_grp1 &= ~int_msk1;

		for (EPn = 0; EPn < (FOTG210_MAX_NUM_EP -1); EPn++) {
			if (gEPMap[EPn] == USB_FIFO_NOT_USE )
				continue;

			if (int_grp1 & DISGR1_IN_INT(gEPMap[EPn]))
				fotg210_in_fifo_handler(fotg210->ep[EPn+1]);

			if(gEPMap[EPn] < 8)
			{
				if ((int_grp1 & DISGR1_OUT_INT(gEPMap[EPn])) ||
					(int_grp1 & DISGR1_SPK_INT(gEPMap[EPn])))
					fotg210_out_fifo_handler(fotg210->ep[EPn+1]);
			}
		}
	}

	spin_unlock(&fotg210->lock);

	return IRQ_HANDLED;
}

#if 0
static irqreturn_t fotg210_isr(int irq, void *_fotg210)
{
	u32 value;
	struct fotg210_udc *fotg210 = _fotg210;

	spin_lock(&fotg210->lock);

	value = ioread32(fotg210->reg + FOTG210_DMCR);
	value &= ~DMCR_GLINT_EN;
	iowrite32(value, fotg210->reg + FOTG210_DMCR);

	spin_unlock(&fotg210->lock);

	return IRQ_WAKE_THREAD;
}

static irqreturn_t fotg210_ist(int irq, void *_fotg210)
{
	u32 value;
	struct fotg210_udc *fotg210 = (struct fotg210_udc *)_fotg210;

	fotg210_irq(0,(void *)_fotg210);

	value = ioread32(fotg210->reg + FOTG210_DMCR);
	value |= DMCR_GLINT_EN;
	iowrite32(value, fotg210->reg + FOTG210_DMCR);

	return IRQ_HANDLED;
}
#endif

static void fotg210_disable_unplug(struct fotg210_udc *fotg210)
{
	u32 reg = ioread32(fotg210->reg + FOTG210_PHYTMSR);
	u32 value;

	reg &= ~PHYTMSR_UNPLUG;
	iowrite32(reg, fotg210->reg + FOTG210_PHYTMSR);

	value = ioread32(fotg210->reg + FOTG210_DMCR);
	value |= DMCR_GLINT_EN;
	iowrite32(value, fotg210->reg + FOTG210_DMCR);
}

static void fotg210_enable_unplug(struct fotg210_udc *fotg210)
{
	u32 reg = ioread32(fotg210->reg + FOTG210_PHYTMSR);
	u32 value;

	value = ioread32(fotg210->reg + FOTG210_DMCR);
	value &= ~DMCR_GLINT_EN;
	iowrite32(value, fotg210->reg + FOTG210_DMCR);

	reg |= PHYTMSR_UNPLUG;
	iowrite32(reg, fotg210->reg + FOTG210_PHYTMSR);
}

static int fotg210_udc_start(struct usb_gadget *g,
		struct usb_gadget_driver *driver)
{
	struct fotg210_udc *fotg210 = gadget_to_fotg210(g);

	/* hook up the driver */
	driver->driver.bus = NULL;
	fotg210->driver = driver;

	/*move to pullup CallBack*/
#if 0
	fotg210_disable_unplug(fotg210);

	/* enable device global interrupt */
	value = ioread32(fotg210->reg + FOTG210_DMCR);
	value |= DMCR_GLINT_EN;
	iowrite32(value, fotg210->reg + FOTG210_DMCR);
#endif
	return 0;
}

#define TRIM_RESINT_UPPER 0x14
#define TRIM_RESINT_LOWER 0x1
static void fotg210_init(struct fotg210_udc *fotg210)
{
	u32 value;
#ifdef CONFIG_NVT_IVOT_PLAT_NA51102
	struct clk       *clk;
#endif
	if (fotg210->top_reg != NULL) {
		fotg210->phyrstbase = ((unsigned long)fotg210->top_reg + 0x00);
	} else {
		if (nvt_get_chip_id() == CHIP_NA51103) {
			fotg210->phyrstbase = ((unsigned long)fotg210->reg  + 0x1d8);
		} else {
			fotg210->phyrstbase = ((unsigned long)fotg210->reg  + 0x400);
		}
	}

#ifdef CONFIG_NVT_IVOT_PLAT_NA51102
	value = readl(fotg210->u3ctrl_reg + 0x28);
	if(value&0x80) {
		//printk("fotg210 : phy already reset by other driver\n");
	} else {
		//printk("fotg210 re-active +++\n");
		clk = clk_get(NULL, "u2phy");
		if (!IS_ERR(clk)) {
			clk_prepare_enable(clk);
			clk_disable_unprepare(clk);
		} else {
			pr_err("%s: clk u2phy not found\n", __func__);
		}

		//printk("fotg210 re-active ---\n");
		value |= 0x80;
		writel(value, fotg210->u3ctrl_reg + 0x28);
	}
#else
	/* Toggle reset of D-PHY to exit PD mode */
	{
		struct clk *source_clk;

		source_clk = clk_get(NULL, "u2phy");
		if (IS_ERR(source_clk)) {
			printk("%s: skip to get clock u2phy and go to toggle phyrstbase\n", __func__);
			value = readl((volatile unsigned long *)(fotg210->phyrstbase));
			value |= (0x1 << 17);
			writel(value, (volatile unsigned long *)(fotg210->phyrstbase));

			value &= ~(0x1 << 17);
			writel(value, (volatile unsigned long *)(fotg210->phyrstbase));
		} else {
			clk_prepare_enable(source_clk);
			clk_disable_unprepare(source_clk);
			clk_put(source_clk);
		}
	}
#endif

	iowrite32((ioread32(fotg210->reg + PHY_TOP_SET_REG))|(0x3<<20), fotg210->reg + PHY_TOP_SET_REG);

	/* make device exit suspend */
	value = ioread32(fotg210->reg + FOTG210_DMACPSR1);
	value &= ~DMACPSR1_DEVSUSPEND;
	iowrite32(value, fotg210->reg + FOTG210_DMACPSR1);

	udelay(200);

#ifndef CONFIG_NVT_FPGA_EMULATION
	/* Configure PHY related settings below */
	{
	#if defined(CONFIG_NVT_IVOT_PLAT_NS02201)
		u16 data=0;
		s32 result=0;
		u32 temp;
		u8 u2_trim_swctrl=6, u2_trim_sqsel=4, u3_trim_rint_sel=8;

		/* STEP1: read efuse */
		result = efuse_readParamOps(EFUSE_USB3_1_TRIM_DATA, &data);
		if (result == 0 /*EFUSE_SUCCESS*/) {
			pr_info("[690][gadget] USB2 Trim data raw(0x2A) = 0x%04x\r\n", (int)data);
			u3_trim_rint_sel = data & 0x1F; //12K trim value

			if ((TRIM_RESINT_LOWER <= u3_trim_rint_sel) && (u3_trim_rint_sel <= TRIM_RESINT_UPPER)) {
				pr_info("[690][gadget] USB2 Trim data(12K) = 0x%04x\r\n", (int)u3_trim_rint_sel);
			} else {
				u3_trim_rint_sel = 8;
				pr_err("[690][gadget] USB2 Trim data(12K) is out of valid range: 0x%04x, use default value: 0x%04x\r\n", (int)(data&0x1F), (int)u3_trim_rint_sel);
			}
		} else {
			pr_err("[690][gadget] USB2 Trim data raw(0x2A) = NULL\n");
		}

		/* STEP2: apply trim value and init phy */
		//select PHY internal rsel value
		temp = ioread32(fotg210->u3phy_reg + 0x684);
		temp &= ~(0x80);
		iowrite32(temp, (fotg210->u3phy_reg + 0x684));

		if (fotg210->u3phy_reg) {
			temp = ioread32(fotg210->u3phy_reg + 0x680);
			if ((temp & 0xF) == 0) {
				iowrite32(0x40 + u3_trim_rint_sel, (fotg210->u3phy_reg + 0x680));
				iowrite32(0x60, (fotg210->u3phy_reg + 0x68C));
			}
		}

		// best setting
		iowrite32(0x74, fotg210->u3phy_reg + 0x158);

		iowrite32(0xb0, fotg210->u3_utmi_phy_reg + (0x9<<2));
		mdelay(5);
		iowrite32(0xb0, fotg210->phy_reg + (0x9<<2));
		mdelay(5);
		iowrite32(0x31, fotg210->u3_utmi_phy_reg + (0x12<<2));
		mdelay(5);
		iowrite32(0x31, fotg210->phy_reg + (0x12<<2));
		mdelay(5);

		iowrite32(0x1c, fotg210->u3_utmi_phy_reg + (0x6<<2));
		mdelay(5);
		iowrite32(0x1c, fotg210->phy_reg + (0x6<<2));
		mdelay(5);

		// r45 cali
		iowrite32(0x20, fotg210->u3_utmi_phy_reg+(0x51<<2));
		mdelay(5);
		iowrite32(0x00, fotg210->u3_utmi_phy_reg+(0x51<<2));
		mdelay(5);

		// tx swing
		if ((fotg210->tx_swing >= 0) && (fotg210->tx_swing <= 7)) {
			u2_trim_swctrl = fotg210->tx_swing;
		}

		temp = ioread32(fotg210->phy_reg+(0x06<<2));
		temp &= ~(0x7<<1);
		temp |= (u2_trim_swctrl<<1);
		iowrite32(temp,(fotg210->phy_reg+(0x06<<2)));

		temp = ioread32((fotg210->phy_reg+(0x05<<2)));
		temp &= ~(0x7<<2);
		temp |= (u2_trim_sqsel<<2);
		iowrite32(temp,(fotg210->phy_reg+(0x05<<2)));
	#elif defined(CONFIG_NVT_IVOT_PLAT_NA51102)
		u16 data=0;
		s32 result=0;
		u32 temp;
		u8 u2_trim_swctrl=4, u2_trim_sqsel=4, u3_trim_rint_sel=8;

		/* STEP1: read efuse */
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

		/* STEP2: apply trim value and init phy */
		//select PHY internal rsel value
		temp = ioread32(fotg210->u3phy_reg + 0x684);
		temp &= ~(0x80);
		iowrite32(temp, (fotg210->u3phy_reg + 0x684));

		if (fotg210->u3phy_reg) {
			temp = ioread32(fotg210->u3phy_reg + 0x680);
			if ((temp & 0xF) == 0) {
				iowrite32(0x40 + u3_trim_rint_sel, (fotg210->u3phy_reg + 0x680));
				iowrite32(0x60, (fotg210->u3phy_reg + 0x68C));
			}
		}

		// best setting
		iowrite32(0x74, fotg210->u3phy_reg + 0x158);

		iowrite32(0xb0, fotg210->u3_utmi_phy_reg + (0x9<<2));
		mdelay(5);
		iowrite32(0xb0, fotg210->phy_reg + (0x9<<2));
		mdelay(5);
		iowrite32(0x31, fotg210->u3_utmi_phy_reg + (0x12<<2));
		mdelay(5);
		iowrite32(0x31, fotg210->phy_reg + (0x12<<2));
		mdelay(5);

		// r45 cali
		iowrite32(0x20, fotg210->u3_utmi_phy_reg+(0x51<<2));
		mdelay(5);
		iowrite32(0x00, fotg210->u3_utmi_phy_reg+(0x51<<2));
		mdelay(5);

		// tx swing
		if ((fotg210->tx_swing >= 0) && (fotg210->tx_swing <= 7)) {
			u2_trim_swctrl = fotg210->tx_swing;
		}

		temp = ioread32(fotg210->phy_reg+(0x06<<2));
		temp &= ~(0x7<<1);
		temp |= (u2_trim_swctrl<<1);
		iowrite32(temp,(fotg210->phy_reg+(0x06<<2)));

		temp = ioread32((fotg210->phy_reg+(0x05<<2)));
		temp &= ~(0x7<<2);
		temp |= (u2_trim_sqsel<<2);
		iowrite32(temp,(fotg210->phy_reg+(0x05<<2)));
	#elif defined(CONFIG_NVT_IVOT_PLAT_NA51089)
		u16 data=0;
		s32 result=0;
		u32 temp;
		u8 u2_trim_swctrl=4, u2_trim_sqsel=4, u2_trim_resint=8;

		/* STEP1: read efuse */
		result= efuse_readParamOps(EFUSE_USBC_TRIM_DATA, &data);

		if(result == 0) {
			u2_trim_swctrl = data&0x7;
			u2_trim_sqsel  = (data>>3)&0x7;
			u2_trim_resint = (data>>6)&0x1F;
		}

		/* STEP2: apply trim value and init phy */
		iowrite32(0x20, (fotg210->phy_reg+(0x51<<2)));
		iowrite32(0x30, (fotg210->phy_reg+(0x50<<2)));

		temp = ioread32(fotg210->phy_reg+(0x06<<2));
		temp &= ~(0x7<<1);
		temp |= (u2_trim_swctrl<<1);
		iowrite32(temp,(fotg210->phy_reg+(0x06<<2)));

		temp = ioread32((fotg210->phy_reg+(0x05<<2)));
		temp &= ~(0x7<<2);
		temp |= (u2_trim_sqsel<<2);
		iowrite32(temp,(fotg210->phy_reg+(0x05<<2)));

		iowrite32(0x60+u2_trim_resint, (fotg210->phy_reg+(0x52<<2)));
		iowrite32(0x00, (fotg210->phy_reg+(0x51<<2)));

		iowrite32(0x100+u2_trim_resint, fotg210->reg + 0x30C);

		fotg210->efuse_data[0] = u2_trim_swctrl;
		fotg210->efuse_data[1] = u2_trim_sqsel;
		fotg210->efuse_data[2] = u2_trim_resint;
	#elif defined(CONFIG_NVT_IVOT_PLAT_NS02301)
		u32 temp;
		u8 u2_trim_swctrl = 4, u2_trim_sqsel = 4, u2_trim_resint = 8;

		/* STEP1: read efuse */
		{
			u16 data = 0;
			s32 result = 0;

		#if IS_ENABLED(CONFIG_NVT_OTP_SUPPORT)
			result = efuse_readParamOps(EFUSE_USBC_TRIM_DATA, &data);
		#else
			pr_err("%s: CONFIG_NVT_OTP_SUPPORT is disabled, use default trim\n", __func__);
			result = -1;
		#endif
			if (result == 0) {
				u2_trim_resint = data & 0x1F;
			}
		}

		/* STEP2: apply trim value and init phy */
		// best setting
		iowrite32(0xd3, (fotg210->phy_reg + 0x14));

		// r45 cali lock
		iowrite32(0x20, (fotg210->phy_reg + 0x144));
		iowrite32(0x30, (fotg210->phy_reg + 0x140));

		// tx swing
		temp = ioread32(fotg210->phy_reg + 0x18);
		temp &= ~(0x7 << 1);
		temp |= (u2_trim_swctrl << 1);
		iowrite32(temp, (fotg210->phy_reg + 0x18));

		// squelch
		temp = ioread32((fotg210->phy_reg + 0x14));
		temp &= ~(0x7 << 2);
		temp |= (u2_trim_sqsel << 2);
		iowrite32(temp, (fotg210->phy_reg + 0x14));

		// res
		iowrite32(0x60 + u2_trim_resint, (fotg210->phy_reg + 0x148));

		// r45 cali unlock
		iowrite32(0x00, (fotg210->phy_reg+ 0x144));

		if (fotg210->top_reg != NULL) {
			temp = ioread32((fotg210->top_reg + 0x00));
			temp &= ~(0x1F << 8);
			temp |= (u2_trim_resint << 8);
			temp |= (0x1 << 15);
			iowrite32(temp, (fotg210->top_reg + 0x00));
		}

		fotg210->efuse_data[0] = u2_trim_swctrl;
		fotg210->efuse_data[1] = u2_trim_sqsel;
		fotg210->efuse_data[2] = u2_trim_resint;
	#elif defined(CONFIG_NVT_IVOT_PLAT_NA51103)
		u16 data=0;
		s32 result=0;

		u32 temp;
		u8 u2_trim_swctrl=4, u2_trim_sqsel=4, u2_trim_resint=8;

		#ifdef CONFIG_NVT_OTP_SUPPORT
		result= efuse_readParamOps(EFUSE_USBC_TRIM_DATA_1ST, &data);
		if(result == 0) {
			pr_info("[5-1] USB = 0x%04x\r\n", (int)data);
			if ((TRIM_RESINT_LOWER <= (data&0x1F)) && ((data&0x1F) <= TRIM_RESINT_UPPER)) {//check if trim_resint fall in valid range
				u2_trim_resint = data&0x1F;
				pr_info("[5-1]]USB Trim data(12K) = 0x%04x\r\n", (int)u2_trim_resint);
			} else {
				pr_err("[5-1] data is out of valid range\n");
			}
		} else {
			pr_err("[5-1] data = NULL\n");
		}
		#endif

		iowrite32(0x20, fotg210->phy_reg + (0x51<<2));
		iowrite32(0x30, fotg210->phy_reg + (0x50<<2));

		temp = ioread32(fotg210->phy_reg+(0x06<<2));
		temp &= ~(0x7<<1);
		temp |= (u2_trim_swctrl<<1);
		iowrite32(temp,(fotg210->phy_reg+(0x06<<2)));

		temp = ioread32((fotg210->phy_reg+(0x05<<2)));
		temp &= ~(0x7<<2);
		temp |= (u2_trim_sqsel<<2);
		iowrite32(temp,(fotg210->phy_reg+(0x05<<2)));

		iowrite32(0x60+u2_trim_resint, (fotg210->phy_reg+(0x52<<2)));
		iowrite32(0x00, fotg210->phy_reg+(0x51<<2));
		iowrite32(0x100+u2_trim_resint, fotg210->reg + 0x30C);

		temp = ioread32(fotg210->reg + 0x400);
		temp &= ~(0x1F << 8);
		temp |= (u2_trim_resint << 8);
		temp |= (0x1 << 15);
		iowrite32(temp, fotg210->reg + 0x400);

	#elif defined(CONFIG_NVT_IVOT_PLAT_NA51090)
		u16 data=0;
		s32 result=0;
		u32 temp;
		u8 u2_trim_swctrl=4, u2_trim_sqsel=4, u3_trim_rint_sel=8, u2_trim_resint=8;

		#ifdef CONFIG_NVT_OTP_SUPPORT
		/* STEP1: read efuse */
		result = efuse_readParamOps(EFUSE_USB2_TRIM_DATA, &data);
		if (result == 0) {
			u3_trim_rint_sel = data & 0x1F;
			u2_trim_resint   = (data >> 9) & 0x1F;
		}
		#endif

		/* STEP2: apply trim value and init phy */
		if ((fotg210->channel == 0) && (fotg210->u3phy_reg)) {
			temp = ioread32(fotg210->u3phy_reg + 0x680);
			if ((temp & 0xF) == 0) {
				iowrite32(0x40 + u3_trim_rint_sel, (fotg210->u3phy_reg + 0x680));
				iowrite32(0x60, (fotg210->u3phy_reg + 0x68C));
			}

			iowrite32(0x20, fotg210->u3_utmi_phy_reg + (0x51<<2));
			iowrite32(0x30, fotg210->u3_utmi_phy_reg + (0x50<<2));

			iowrite32(0x60+u2_trim_resint, (fotg210->u3_utmi_phy_reg+(0x52<<2)));
			iowrite32(0x00, fotg210->u3_utmi_phy_reg+(0x51<<2));
			mdelay(5);
			iowrite32(0x04, fotg210->u3_utmi_phy_reg+(0xE<<2));
			mdelay(5);

			temp = ioread32(fotg210->u3_utmi_phy_reg+(0x06<<2));
			temp &= ~(0x7<<1);
			temp |= (u2_trim_swctrl<<1);
			iowrite32(temp,(fotg210->u3_utmi_phy_reg+(0x06<<2)));

			temp = ioread32((fotg210->u3_utmi_phy_reg+(0x05<<2)));
			temp &= ~(0x7<<2);
			temp |= (u2_trim_sqsel<<2);
			iowrite32(temp,(fotg210->u3_utmi_phy_reg+(0x05<<2)));

			temp = ioread32(fotg210->reg + 0x400);
			temp &= ~(0x1F << 8);
			temp |= (u2_trim_resint << 8);
			temp |= (0x1 << 15);
			iowrite32(temp, fotg210->reg + 0x400);
		} else {
			iowrite32(0x20, fotg210->phy_reg + (0x51<<2));
			iowrite32(0x30, fotg210->phy_reg + (0x50<<2));

			iowrite32(0x60+u2_trim_resint, (fotg210->phy_reg+(0x52<<2)));
			iowrite32(0x00, fotg210->phy_reg+(0x51<<2));
			mdelay(5);
			iowrite32(0x04, fotg210->phy_reg+(0xE<<2));
			mdelay(5);

			temp = ioread32(fotg210->phy_reg+(0x06<<2));
			temp &= ~(0x7<<1);
			temp |= (u2_trim_swctrl<<1);
			iowrite32(temp,(fotg210->phy_reg+(0x06<<2)));

			temp = ioread32((fotg210->phy_reg+(0x05<<2)));
			temp &= ~(0x7<<2);
			temp |= (u2_trim_sqsel<<2);
			iowrite32(temp,(fotg210->phy_reg+(0x05<<2)));

			temp = ioread32(fotg210->reg + 0x400);
			temp &= ~(0x1F << 8);
			temp |= (u2_trim_resint << 8);
			temp |= (0x1 << 15);
			iowrite32(temp, fotg210->reg + 0x400);
		}

		fotg210->efuse_data[0] = u2_trim_swctrl;
		fotg210->efuse_data[1] = u2_trim_sqsel;
		fotg210->efuse_data[2] = u2_trim_resint;
	#endif
	}
#endif


	/* VBUS debounce time */
	iowrite32(ioread32(fotg210->reg + FOTG210_OTGCTRLSTS)|0x400, fotg210->reg + FOTG210_OTGCTRLSTS);


////////////////////////////////////////////////////////////////////////////////

	/* disable global interrupt and set int polarity to active high */
	iowrite32(GMIR_MHC_INT | GMIR_MOTG_INT | GMIR_INT_POLARITY,
		  fotg210->reg + FOTG210_GMIR);

	/* disable device global interrupt */
	value = ioread32(fotg210->reg + FOTG210_DMCR);
	value &= ~DMCR_GLINT_EN;
	#ifdef CONFIG_NVT_FPGA_EMULATION
	value |= DMCR_HALF_SPEED;
	#endif

	value |= DMCR_CAP_RMWAKUP;
	fotg210->remote_wkp = 0;

	iowrite32(value, fotg210->reg + FOTG210_DMCR);

	/* disable all fifo interrupt */
	iowrite32(~(u32)0, fotg210->reg + FOTG210_DMISGR1);

	/* disable cmd end */
	value = ioread32(fotg210->reg + FOTG210_DMISGR0);
	value |= DMISGR0_MCX_COMEND;
	iowrite32(value, fotg210->reg + FOTG210_DMISGR0);

	/* disable fifo empty interrupt */
	value = ioread32(fotg210->reg + VDMA_CMPLT_EMPTY_FX_INT_MASK);
	value |= 0xFFFF;
	iowrite32(value, fotg210->reg + VDMA_CMPLT_EMPTY_FX_INT_MASK);

	/* RESP MODE open */
	value = ioread32(fotg210->reg + AXI_MONI_REG);
	value |= BUF_RESP_MODE;
	iowrite32(value, fotg210->reg + AXI_MONI_REG);

	/* AXI channel open */
	value = ioread32(fotg210->reg + AXI_MONI_REG);
	value &= ~MONI_CH_DIS;
	iowrite32(value, fotg210->reg + AXI_MONI_REG);
}

static int fotg210_udc_stop(struct usb_gadget *g)
{
	struct fotg210_udc *fotg210 = gadget_to_fotg210(g);
	unsigned long	flags;

	spin_lock_irqsave(&fotg210->lock, flags);

	//fotg210_init(fotg210);
	fotg210->driver = NULL;

	spin_unlock_irqrestore(&fotg210->lock, flags);

	return 0;
}

static void fotg210_remove_power_save(struct platform_device *pdev)
{
	struct fotg210_udc *fotg210 = platform_get_drvdata(pdev);

	dev_info(&pdev->dev, "%s: Enter powerdown script\r\n", __func__);

	// IP provided to make PHY deeply sleeping
	iowrite32(0xB4, (fotg210->phy_reg + 0xD4));  // Port0 dmpulldown = 1, dppulldown = 0, suspendm = 1, termselect = 1, xcvrselect = 01, opmode = 00
	iowrite32(0xFF, (fotg210->phy_reg + 0xD0));  // Enable port0 sw reg 0xF060_10D4
	iowrite32(0x14, (fotg210->phy_reg + 0x08));  // Port0 pll_pdn = 0, pll_rstn = 0
	iowrite32(0x8A, (fotg210->phy_reg + 0x00));  // Port0 bias_pdn = 0, ldo_pdn = 0
	iowrite32(0x13, (fotg210->phy_reg + 0x30));  // Port0 datarpu1 = 0, datarpu2 = 1, spsel = 0, datarpusel = 0, datarpdp = 1, datarpdm = 1
	iowrite32(0xFF, (fotg210->phy_reg + 0x2C));  // Enable port0 sw reg 0x0030
	iowrite32(0x20, (fotg210->phy_reg + 0x28));  // Port0 usb_pwr_ok = 0
	iowrite32(0xB3, (fotg210->phy_reg + 0x24));  // Port0 u11_rx_pwd = 1
	iowrite32(0x90, (fotg210->phy_reg + 0x34));  // Port0 tx_pdn = 0
}

static int fotg210_udc_pullup(struct usb_gadget *g, int is_on)
{
	struct fotg210_udc *fotg210 = gadget_to_fotg210(g);

	if(is_on) {
		fotg210_disable_unplug(fotg210);
	} else {
		fotg210_enable_unplug(fotg210);
	}

	return 0;
}

static int fotg210_udc_wakeup(struct usb_gadget *g)
{
	struct fotg210_udc *fotg210 = gadget_to_fotg210(g);
	u32 value;

	if(fotg210->remote_wkp) {
		printk("signal remote_wkp\n");
		value = ioread32(fotg210->reg + FOTG210_DMACPSR1);
		value |= DMCR_RMWK;
		iowrite32(value, fotg210->reg + FOTG210_DMACPSR1);
	}

	return 0;
}


static const struct usb_gadget_ops fotg210_gadget_ops = {
	.udc_start		= fotg210_udc_start,
	.udc_stop		= fotg210_udc_stop,
	.pullup			= fotg210_udc_pullup,
	.wakeup			= fotg210_udc_wakeup,
};

static int fotg210_udc_remove(struct platform_device *pdev)
{
	struct fotg210_udc *fotg210 = platform_get_drvdata(pdev);
	int i;

	/* Toggle reset of D-PHY and Controller to avoid unexpected situations */
	{
		struct clk *source_clk;
		u32 value;

		/* Toggle D-PHY */
		source_clk = clk_get(&pdev->dev, "u2phy");
		if (IS_ERR(source_clk)) {
			dev_err(&pdev->dev, "%s: skip to get clock u2phy and go to toggle phyrstbase\n", __func__);
			value = readl((volatile unsigned long *)(fotg210->phyrstbase));
			value |= (0x1 << 17);
			writel(value, (volatile unsigned long *)(fotg210->phyrstbase));

			value &= ~(0x1 << 17);
			writel(value, (volatile unsigned long *)(fotg210->phyrstbase));
		} else {
			clk_prepare_enable(source_clk);
			clk_disable_unprepare(source_clk);
			clk_put(source_clk);
		}

		/* Toggle Controller */
		source_clk = clk_get(&pdev->dev, dev_name(&pdev->dev));
		if (IS_ERR(source_clk)) {
			dev_err(&pdev->dev, "%s: can't find clock %s\n", __func__, dev_name(&pdev->dev));
		} else {
			/* toggle reset during the next initialization */
			clk_disable_unprepare(source_clk);

			/* toggle reset */
			clk_prepare_enable(source_clk);
			clk_disable_unprepare(source_clk);
			clk_put(source_clk);
		}
	}

	usb_del_gadget_udc(&fotg210->gadget);
	iounmap(fotg210->reg);

	if (nvt_get_chip_id() == CHIP_NS02301) {
		fotg210_remove_power_save(pdev);
	}

#ifndef CONFIG_NVT_FPGA_EMULATION
	iounmap(fotg210->phy_reg);
	if (fotg210->top_reg)
		iounmap(fotg210->phy_reg);
	if (fotg210->u3phy_reg)
		iounmap(fotg210->u3phy_reg);
	if (fotg210->u3_utmi_phy_reg)
		iounmap(fotg210->u3_utmi_phy_reg);
	if (fotg210->u3ctrl_reg)
		iounmap(fotg210->u3ctrl_reg);
#endif

	free_irq(platform_get_irq(pdev, 0), fotg210);

	fotg210_ep_free_request(&fotg210->ep[0]->ep, fotg210->ep0_req);
	for (i = 0; i < FOTG210_MAX_NUM_EP; i++)
		kfree(fotg210->ep[i]);

	kfree(fotg210);

	return 0;
}

static int fotg210_udc_probe(struct platform_device *pdev)
{
#ifndef CONFIG_NVT_FPGA_EMULATION
	struct device_node *dn = pdev->dev.of_node;
#endif
	struct resource *res, *ires;
	struct fotg210_udc *fotg210 = NULL;
	struct fotg210_ep *_ep[FOTG210_MAX_NUM_EP];
	int ret = 0;
	int i;
	UINT32 EPi, FIFOi;
	int val, val2;
	UINT32 epnum = FOTG210_DEFAULT_NUM_EP;
	u32 channel = 0;

	u32 get_tx_swing;
	int tx_swing;
	unsigned int cdc_outslice = 0;
	int of_addr_index = 0;
	int use_top = 0;
	int u2_only = 0;

	if (debug_on)
		numsg("fotg210_udc_probe\r\n");

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!res) {
		pr_err("platform_get_resource error.\n");
		return -ENODEV;
	}

	ires = platform_get_resource(pdev, IORESOURCE_IRQ, 0);
	if (!ires) {
		pr_err("platform_get_resource IORESOURCE_IRQ error.\n");
		return -ENODEV;
	}

	if (of_property_read_u32(pdev->dev.of_node, "epnum", &epnum)) {
		pr_info("use default epnum\n");
	}

	ret = -ENOMEM;

	if (of_property_read_u32(pdev->dev.of_node, "channel", &channel)) {
		pr_info("channel not set. \n");
		channel = -1;
	}

	if (of_property_read_u32(pdev->dev.of_node, "cdc_outslice", &cdc_outslice)) {
		pr_info("outslice is set from module parameter.\n");
	} else {
		pr_info("outslice is set from dts.\n");
		outslice = cdc_outslice;
	}

	if (!of_property_read_u32(pdev->dev.of_node, "use_top", &use_top)) {
		printk("use top soc!\n");
	} else {
		if (nvt_get_chip_id() == CHIP_NS02301) {
			printk("use top soc! (default)\n");
			use_top = 1;
		} else {
			use_top = 0;
		}
	}

	if (!of_property_read_u32(pdev->dev.of_node, "u2_only", &u2_only)) {
		printk("u2 only soc!\n");
	} else {
		if (nvt_get_chip_id() == CHIP_NA51089 || nvt_get_chip_id() == CHIP_NS02301 || nvt_get_chip_id() == CHIP_NA51103) {
			printk("u2 only soc! (default)\n");
			u2_only = 1;
		} else
			u2_only = 0;
	}

	/* initialize udc */
	fotg210 = kzalloc(sizeof(struct fotg210_udc), GFP_KERNEL);
	if (fotg210 == NULL)
		goto err;

	for (i = 0; i < FOTG210_MAX_NUM_EP; i++) {
		_ep[i] = kzalloc(sizeof(struct fotg210_ep), GFP_KERNEL);
		if (_ep[i] == NULL)
			goto err_alloc;
		fotg210->ep[i] = _ep[i];
	}

	if (of_property_read_u32(pdev->dev.of_node, "tx_swing", &get_tx_swing)) {
		tx_swing = -1;
	} else {
		tx_swing = get_tx_swing;
	}

	fotg210->tx_swing = tx_swing;

	fotg210->reg = ioremap(res->start, resource_size(res));
	pr_info("res->start is 0x%px\n", &res->start);

	if (fotg210->reg == NULL) {
		pr_err("ioremap error.\n");
		goto err_alloc;
	}

#ifndef CONFIG_NVT_FPGA_EMULATION
	//need set r12 during u3 phy reg.
	fotg210->phy_reg = of_iomap(dn, ++of_addr_index);  // index is 1
	if (!fotg210->phy_reg) {
		pr_info("res[%d] phy_reg ioremap failed\n", of_addr_index);
		ret = -ENOMEM;
		goto err_map;
	} else {
		pr_info("res[%d] phy_reg(0x%px)\n", of_addr_index, fotg210->phy_reg);
	}

	if (use_top) {
		fotg210->top_reg = of_iomap(dn, ++of_addr_index);
		if (!fotg210->top_reg) {
			pr_info("res[%d] top_reg ioremap failed\n", of_addr_index);
			ret = -ENOMEM;
			goto err_map;
		} else {
			pr_info("res[%d] top_reg(0x%px)\n", of_addr_index, fotg210->top_reg);
		}
	} else {
		fotg210->top_reg = NULL;
	}

	if ((!u2_only) && (channel == 0)) {
		fotg210->u3phy_reg = of_iomap(dn, ++of_addr_index);
		if (!fotg210->u3phy_reg) {
			pr_info("res[%d] u3phy_reg ioremap failed\n", of_addr_index);
			ret = -ENOMEM;
			goto err_map;
		}  else {
			pr_info("res[%d] u3phy_reg(0x%px)\n", of_addr_index, fotg210->u3phy_reg);
		}

		fotg210->u3_utmi_phy_reg = of_iomap(dn, ++of_addr_index);
		if (!fotg210->u3_utmi_phy_reg) {
			pr_info("res[%d] u3_utmi_phy_reg ioremap failed\n", of_addr_index);
			ret = -ENOMEM;
			goto err_map;
		}  else {
			pr_info("res[%d] u3_utmi_phy_reg(0x%px)\n", of_addr_index, fotg210->u3_utmi_phy_reg);
		}

#ifdef CONFIG_NVT_IVOT_PLAT_NA51102
		fotg210->u3ctrl_reg = of_iomap(dn, ++of_addr_index);
		if (!fotg210->u3ctrl_reg) {
			pr_info("res[%d] u3ctrl_reg ioremap failed\n", of_addr_index);
			ret = -ENOMEM;
			goto err_map;
		}  else {
			pr_info("res[%d] u3ctrl_reg(0x%px)\n", of_addr_index, fotg210->u3ctrl_reg);
		}
#endif
	} else {
		fotg210->u3phy_reg = NULL;
		fotg210->u3_utmi_phy_reg = NULL;
		fotg210->u3ctrl_reg = NULL;
	}
#endif
	{
		struct clk *source_clk;

		source_clk = clk_get(&pdev->dev, dev_name(&pdev->dev));
		if (IS_ERR(source_clk)) {
			dev_err(&pdev->dev, "%s: can't find clock %s\n", __func__, dev_name(&pdev->dev));
			source_clk = NULL;
		} else {
			printk("clock %s get ok\n", dev_name(&pdev->dev));
			clk_prepare(source_clk);
			clk_enable(source_clk);
			clk_put(source_clk);
		}
	}

	spin_lock_init(&fotg210->lock);

	platform_set_drvdata(pdev, fotg210);

	fotg210->gadget.ops = &fotg210_gadget_ops;

	fotg210->gadget.max_speed = USB_SPEED_HIGH;
	fotg210->gadget.dev.parent = &pdev->dev;
	fotg210->gadget.dev.dma_mask = pdev->dev.dma_mask;
	fotg210->gadget.name = udc_name;

	INIT_LIST_HEAD(&fotg210->gadget.ep_list);

	for (i = 0; i < FOTG210_MAX_NUM_EP; i++) {
		struct fotg210_ep *ep = fotg210->ep[i];

		if (i) {
			INIT_LIST_HEAD(&fotg210->ep[i]->ep.ep_list);
			list_add_tail(&fotg210->ep[i]->ep.ep_list,
				      &fotg210->gadget.ep_list);
		}
		ep->fotg210 = fotg210;
		INIT_LIST_HEAD(&ep->queue);
		//ep->ep.name = fotg210_ep_name[i];
		ep->ep.name = ep_info[i].name;
		ep->ep.caps = ep_info[i].caps;
		ep->ep.ops = &fotg210_ep_ops;
		usb_ep_set_maxpacket_limit(&ep->ep, (unsigned short) ~0);

		if (i == 0) {
			ep->ep.caps.type_control = true;
		} else {
			ep->ep.caps.type_iso = true;
			ep->ep.caps.type_bulk = true;
			ep->ep.caps.type_int = true;
		}

		ep->ep.caps.dir_in = true;
		ep->ep.caps.dir_out = true;
	}
	usb_ep_set_maxpacket_limit(&fotg210->ep[0]->ep, 0x40);
	fotg210->gadget.ep0 = &fotg210->ep[0]->ep;
	INIT_LIST_HEAD(&fotg210->gadget.ep0->ep_list);

	fotg210->ep0_req = fotg210_ep_alloc_request(&fotg210->ep[0]->ep,
				GFP_KERNEL);
	if (fotg210->ep0_req == NULL)
		goto err_map;

	fotg210_init(fotg210);

	//fotg210_disable_unplug(fotg210);


#if 0
	/* Using tasklet for command processing */
	ret =  request_threaded_irq(ires->start, fotg210_isr, fotg210_ist,
			IRQF_SHARED, udc_name, fotg210);
#else
	ret = request_irq(ires->start, fotg210_irq, IRQF_SHARED,
			  udc_name, fotg210);
#endif
	if (ret < 0) {
		pr_err("request_irq error (%d)\n", ret);
		goto err_req;
	}

	ret = usb_add_gadget_udc(&pdev->dev, &fotg210->gadget);
	if (ret)
		goto err_add_udc;

	dev_info(&pdev->dev, "version %s\n", DRIVER_VERSION);

	//clear 0x1A8, 0x1AC
	val |= ioread32(fotg210->reg + 0x1A8);
	val2 |= ioread32(fotg210->reg + 0x1AC);

	val &= ~CLEARMASK;
	val2 &= ~CLEARMASK;

	iowrite32(val, (fotg210->reg+0x1A8));
	iowrite32(val2, (fotg210->reg+0x1AC));

	// clear all EP & FIFO setting of EP-FIFO mapping table. -1 because excluding the EP0.
	for (EPi = 0 ; EPi < (FOTG210_MAX_NUM_EP - 1) ; EPi++) {
		gEPMap[EPi] = USB_FIFO_NOT_USE;
	}

	for (FIFOi = 0 ; FIFOi < FOTG210_MAX_FIFO_NUM ; FIFOi++) {
		gFIFOInMap[FIFOi]  = USB_EP_NOT_USE;
		gFIFOOutMap[FIFOi] = USB_EP_NOT_USE;
	}

	if(epnum > FOTG210_DEFAULT_NUM_EP)
	{
		dev_info(&pdev->dev, "more than 8 EPs!, switch to 1 EP 1 Fifo\n");
		for (EPi = 0 ; EPi < (FOTG210_MAX_NUM_EP - 1) ; EPi++) {
			gEPBlkNo[EPi] = BLKNUM_SINGLE;
		}
	}

	return 0;

err_add_udc:
	free_irq(ires->start, fotg210);

err_req:
	fotg210_ep_free_request(&fotg210->ep[0]->ep, fotg210->ep0_req);

err_map:
	iounmap(fotg210->reg);
#ifndef CONFIG_NVT_FPGA_EMULATION
	iounmap(fotg210->phy_reg);
	if (fotg210->top_reg)
		iounmap(fotg210->phy_reg);
	if (fotg210->u3phy_reg)
		iounmap(fotg210->u3phy_reg);
	if (fotg210->u3_utmi_phy_reg)
		iounmap(fotg210->u3_utmi_phy_reg);
	if (fotg210->u3ctrl_reg)
		iounmap(fotg210->u3ctrl_reg);
#endif
err_alloc:
	for (i = 0; i < FOTG210_MAX_NUM_EP; i++)
		kfree(fotg210->ep[i]);
	kfree(fotg210);

err:
	return ret;
}

#ifdef CONFIG_PM
#if defined(CONFIG_NVT_IVOT_PLAT_NA51089) || defined(CONFIG_NVT_IVOT_PLAT_NS02301) || defined(CONFIG_NVT_IVOT_PLAT_NA51102) || defined(CONFIG_NVT_IVOT_PLAT_NA51103) || defined(CONFIG_NVT_IVOT_PLAT_NA51090)
static void fotg210_fifo_clr(struct fotg210_udc *fotg210)
{
	u32 value = 0;
	u32 dma_start;

	// dma abort & clr fifo
	value = ioread32(fotg210->reg + FOTG210_DMACPSR1);
	value |= DMACPSR1_DMA_ABORT;
	value |= DMACPSR1_CLR_FIFO_DMA_ABORT;
	iowrite32(value, fotg210->reg + FOTG210_DMACPSR1);

	do {
		dma_start = ioread32(fotg210->reg + FOTG210_DMACPSR1);
	} while (dma_start & 0x1);

	value = ioread32(fotg210->reg + FOTG210_DMACPSR1);
	value &= ~DMACPSR1_DMA_ABORT;
	iowrite32(value, fotg210->reg + FOTG210_DMACPSR1);
}

static void fotg210_restart_controller(struct fotg210_udc *fotg210)
{
	u32 value = 0;
	u32 temp;

	/* Toggle reset of D-PHY to exit PD mode */
	{
		struct clk *source_clk;

		source_clk = clk_get(NULL, "u2phy");
		if (IS_ERR(source_clk)) {
			printk("%s: skip to get clock u2phy and go to toggle phyrstbase\n", __func__);
			value = readl((volatile unsigned long *)(fotg210->phyrstbase));
			value |= (0x1 << 17);
			writel(value, (volatile unsigned long *)(fotg210->phyrstbase));

			value &= ~(0x1 << 17);
			writel(value, (volatile unsigned long *)(fotg210->phyrstbase));
		} else {
			clk_prepare_enable(source_clk);
			clk_disable_unprepare(source_clk);
			clk_put(source_clk);
		}
	}

	iowrite32((ioread32(fotg210->reg + PHY_TOP_SET_REG))|(0x3<<20), fotg210->reg + PHY_TOP_SET_REG);

	/* make device exit suspend */
	value = ioread32(fotg210->reg + FOTG210_DMACPSR1);
	value &= ~DMACPSR1_DEVSUSPEND;
	iowrite32(value, fotg210->reg + FOTG210_DMACPSR1);

	udelay(50);

	/* PHY re-init by store_efuse*/
	if (nvt_get_chip_id() == CHIP_NS02301) {
		// best setting
		iowrite32(0xd3, (fotg210->phy_reg + 0x14));
	}

	// r45 cali lock
	iowrite32(0x20, (fotg210->phy_reg+(0x51<<2)));
	iowrite32(0x30, (fotg210->phy_reg+(0x50<<2)));

	// tx swing
	temp = ioread32(fotg210->phy_reg+(0x06<<2));
	temp &= ~(0x7<<1);
	temp |= (fotg210->efuse_data[0]<<1);
	iowrite32(temp,(fotg210->phy_reg+(0x06<<2)));

	// squelch
	temp = ioread32((fotg210->phy_reg+(0x05<<2)));
	temp &= ~(0x7<<2);
	temp |= (fotg210->efuse_data[1]<<2);
	iowrite32(temp,(fotg210->phy_reg+(0x05<<2)));

	// res
	iowrite32(0x60+fotg210->efuse_data[2], (fotg210->phy_reg+(0x52<<2)));

	// r45 cali unlock
	iowrite32(0x00, (fotg210->phy_reg+(0x51<<2)));

	/* VBUS debounce time */
	iowrite32(ioread32(fotg210->reg + FOTG210_OTGCTRLSTS)|0x400, fotg210->reg + FOTG210_OTGCTRLSTS);

	/* disable global interrupt and set int polarity to active high */
	iowrite32(GMIR_MHC_INT | GMIR_MOTG_INT | GMIR_INT_POLARITY,
			fotg210->reg + FOTG210_GMIR);

	/* disable device global interrupt */
	value = ioread32(fotg210->reg + FOTG210_DMCR);
	value &= ~DMCR_GLINT_EN;
#ifdef CONFIG_NVT_FPGA_EMULATION
	value |= DMCR_HALF_SPEED;
#endif
	iowrite32(value, fotg210->reg + FOTG210_DMCR);

	/* disable all fifo interrupt */
	iowrite32(~(u32)0, fotg210->reg + FOTG210_DMISGR1);

	/* disable cmd end */
	value = ioread32(fotg210->reg + FOTG210_DMISGR0);
	value |= DMISGR0_MCX_COMEND;
	iowrite32(value, fotg210->reg + FOTG210_DMISGR0);

	#ifdef CONFIG_NVT_IVOT_PLAT_NA51102
	/* disable fifo empty interrupt */
	value = ioread32(fotg210->reg + VDMA_CMPLT_EMPTY_FX_INT_MASK);
	value |= 0xFFFF;
	iowrite32(value, fotg210->reg + VDMA_CMPLT_EMPTY_FX_INT_MASK);

	/* RESP MODE open */
	value = ioread32(fotg210->reg + AXI_MONI_REG);
	value |= BUF_RESP_MODE;
	iowrite32(value, fotg210->reg + AXI_MONI_REG);

	/* AXI channel open */
	value = ioread32(fotg210->reg + AXI_MONI_REG);
	value &= ~MONI_CH_DIS;
	iowrite32(value, fotg210->reg + AXI_MONI_REG);
	#endif
}

static void fotg210_stop_controller(struct fotg210_udc *fotg210)
{
	u32 value = 0;

#if 0
	//INTR_GRP0_MASK:0x134
	value = ioread32(fotg210->reg + FOTG210_DMISGR0);
	value |= 0x8;
	iowrite32(value, fotg210->reg + FOTG210_DMISGR0);

	//INTR_GRP1_MASK:0x138
	value = ioread32(fotg210->reg + FOTG210_DMISGR1);
	value |= 0xFFFFFFFF;
	iowrite32(value, fotg210->reg + FOTG210_DMISGR1);

	//INTR_GRP2_MASK:0x13C
	value = ioread32(fotg210->reg + FOTG210_DMISGR2);
	value |= 0xFFFF0000;
	iowrite32(value, fotg210->reg + FOTG210_DMISGR2);
#endif
	//DEV_MAIN_CTRL:0x100
	value = ioread32(fotg210->reg + FOTG210_DMCR);
	value &= ~DMCR_GLINT_EN;
	iowrite32(value, fotg210->reg + FOTG210_DMCR);

	//USB_TOP_REG:0x400
	value = ioread32(fotg210->reg + PHY_TOP_SET_REG);
	value &= ~(0x1<<20);
	iowrite32(value, fotg210->reg + PHY_TOP_SET_REG);

	//DEVSUSPEND:0x1C8
	value = ioread32(fotg210->reg + FOTG210_DMACPSR1);
	value |= DMACPSR1_DEVSUSPEND;
	iowrite32(value, fotg210->reg + FOTG210_DMACPSR1);

	if (debug_on) {
		// [0x130, 0x134, 0x138, 0x13C, 0x100, 0x400, 0x1C8]
		printk("USB_CTRL_STOP reg_dump: [0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x]\r\n",
				ioread32(fotg210->reg + DMIGR_MINT_G0), ioread32(fotg210->reg + FOTG210_DMISGR0),
				ioread32(fotg210->reg + FOTG210_DMISGR1), ioread32(fotg210->reg + FOTG210_DMISGR2),
				ioread32(fotg210->reg + FOTG210_DMCR), ioread32(fotg210->reg + PHY_TOP_SET_REG),
				ioread32(fotg210->reg + FOTG210_DMACPSR1));
	}
}

static void stop_udc(struct fotg210_udc *fotg210)
{
	spin_lock(&fotg210->lock);

	/* FIFO clear */
	fotg210_fifo_clr(fotg210);

	/* Disconnect gadget driver */
	if (fotg210->driver) {
		fotg210_enable_unplug(fotg210);
		spin_unlock(&fotg210->lock);
		fotg210->driver->disconnect(&fotg210->gadget);
		spin_lock(&fotg210->lock);
	}

	spin_unlock(&fotg210->lock);
	//printk("Device disconnected\n");
}

/*
static void stop_phy(struct fotg210_udc *fotg210)
{
	#ifdef CONFIG_NVT_IVOT_PLAT_NA51102
	iowrite32(0x34, (fotg210->phy_reg+(0xD4)));
	iowrite32(0x34, (fotg210->u3_utmi_phy_reg+(0xD4)));

	iowrite32(0xff, (fotg210->phy_reg+(0xD0)));
	iowrite32(0xff, (fotg210->u3_utmi_phy_reg+(0xD0)));

	iowrite32(0x14, (fotg210->phy_reg+(0x08)));
	iowrite32(0x14, (fotg210->u3_utmi_phy_reg+(0x08)));

	iowrite32(0x8e, (fotg210->phy_reg+(0x00)));
	iowrite32(0x8e, (fotg210->u3_utmi_phy_reg+(0x00)));

	iowrite32(0x13, (fotg210->phy_reg+(0x30)));
	iowrite32(0x13, (fotg210->u3_utmi_phy_reg+(0x30)));

	iowrite32(0xFF, (fotg210->phy_reg+(0x2C)));
	iowrite32(0xFF, (fotg210->u3_utmi_phy_reg+(0x2C)));

	iowrite32(0x20, (fotg210->phy_reg+(0x28)));
	iowrite32(0x20, (fotg210->u3_utmi_phy_reg+(0x28)));

	iowrite32(0xb3, (fotg210->phy_reg+(0x24)));
	iowrite32(0xb3, (fotg210->u3_utmi_phy_reg+(0x24)));

	iowrite32(0x90, (fotg210->phy_reg+(0x34)));
	iowrite32(0x90, (fotg210->u3_utmi_phy_reg+(0x34)));

	#else
	iowrite32(0xFF, (fotg210->phy_reg+(0xD0)));
	iowrite32(0x34, (fotg210->phy_reg+(0xD4)));
	iowrite32(0x14, (fotg210->phy_reg+(0x08)));
	iowrite32(0x88, (fotg210->phy_reg+(0x00)));
	iowrite32(0x35, (fotg210->phy_reg+(0x30)));
	iowrite32(0xFF, (fotg210->phy_reg+(0x2C)));
	iowrite32(0x20, (fotg210->phy_reg+(0x28)));
	iowrite32(0x93, (fotg210->phy_reg+(0x24)));
	#endif
}
*/

static int nvtim_fotg210_suspend(struct device *dev)
{
	struct platform_device *pdev = to_platform_device(dev);
	struct fotg210_udc *fotg210 = platform_get_drvdata(pdev);
	int irq;
	int rc;

	// STEP1: STOP UDC
	stop_udc(fotg210);

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

	disable_irq(irq);

	// STEP2: STOP controller
	fotg210_stop_controller(fotg210);

	printk("nvtim_fotg210_suspend done\r\n");

	// STEP3: STOP PHY
	//stop_phy(fotg210);

	rc = 0;

	return rc;
}

static int nvtim_fotg210_resume(struct device *dev)
{
	struct platform_device *pdev = to_platform_device(dev);
	struct fotg210_udc *fotg210 = platform_get_drvdata(pdev);
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
	fotg210_restart_controller(fotg210);
	//fotg210_init(fotg210);

	if (fotg210->driver) {
		fotg210_disable_unplug(fotg210);
	}

	enable_irq(irq);
	return 0;
}

#else
static int nvtim_fotg210_suspend(struct device *dev)
{
	return 0;
}

static int nvtim_fotg210_resume(struct device *dev)
{
	return 0;
}

#endif //PROJECT_CONFIG

#else
static int nvtim_fotg210_suspend(struct device *dev)
{
	return 0;
}

static int nvtim_fotg210_resume(struct device *dev)
{
	return 0;
}
#endif //CONFIG_PM

static const struct dev_pm_ops nvtivot_fotg210_pm_ops = {
	.suspend = nvtim_fotg210_suspend,
	.resume  = nvtim_fotg210_resume,
};

#ifdef CONFIG_OF
static const struct of_device_id of_fotg210_match[] = {
	{
		.compatible = "nvt,fotg200_udc"
	},

	{ },
};
MODULE_DEVICE_TABLE(of, of_fotg210_match);
#endif

static struct platform_driver fotg210_driver = {
	.driver		= {
		.name =	(char *)udc_name,
		.pm = &nvtivot_fotg210_pm_ops,
#ifdef CONFIG_OF
		.of_match_table = of_match_ptr(of_fotg210_match),
#endif
	},
	.probe		= fotg210_udc_probe,
	.remove		= fotg210_udc_remove,
};

module_platform_driver(fotg210_driver);

MODULE_AUTHOR("Klins Chen, klins_chen <klins_chen@novatek.com.tw>");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION(DRIVER_DESC);
MODULE_VERSION(DRIVER_VERSION);
