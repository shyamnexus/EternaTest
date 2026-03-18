// SPDX-License-Identifier: GPL-2.0
/*
 * Fusb300 UDC (USB gadget)
 *
 * Copyright (C) 2010 Faraday Technology Corp.
 *
 * Author : Yuan-hsin Chen <yhchen@faraday-tech.com>
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
#include <linux/of_address.h>
#include <linux/delay.h>
#include <plat/efuse_protected.h>
#include <linux/clk.h>
#include <linux/gpio.h>


#if defined (CONFIG_NVT_IVOT_PLAT_NS02201) || defined (CONFIG_NVT_IVOT_PLAT_NS02302)
#include <linux/soc/nvt/nvt-clk-ext.h>
#endif

#if defined (CONFIG_NVT_IVOT_PLAT_NS02302)
#include <plat/top.h>
#endif

#include "fotg330_udc.h"

MODULE_DESCRIPTION("FUSB330  USB gadget driver");
MODULE_LICENSE("GPL");


#define DRIVER_VERSION	"1.00.039"
#define UVC_FIFOENTRY_NUM	10
#define UVC_ISO_MAX_PKTNUM	32

static const char udc_name[] = "fusb330_udc";
static const char * const fusb330_ep_name[] = {
	"ep0", "ep1", "ep2", "ep3", "ep4", "ep5", "ep6", "ep7", "ep8", "ep9",
	"ep10", "ep11", "ep12", "ep13", "ep14", "ep15"
};

#define USB_TRIM_INFO	0

struct fusb300 *gfusb330 = NULL;

static unsigned fifo_entry_num = FUSB300_FIFO_ENTRY_NUM;
static unsigned fotg330_dbg = 0;
static unsigned fotg330_irq_dbg = 0;
static unsigned pio_on = 0;
static unsigned hbw_ep = 0;
static unsigned uvc_enable = 0;
static unsigned u3_channel = 0;
static unsigned dma_int_mode = 0;
static u32 force_hs = 0;
static u32 force_rst = 1;

module_param(dma_int_mode, uint, S_IRUGO);
MODULE_PARM_DESC(dma_int_mode, "dma interrupt mode. Default = 0");

module_param(uvc_enable, uint, S_IRUGO);
MODULE_PARM_DESC(uvc_enable, "Whether enable uvc mode. Default = 0");
module_param(hbw_ep, uint, S_IRUGO);
MODULE_PARM_DESC(hbw_ep, "uvc mode high handwidth ep number. Default = 0");
module_param(pio_on, uint, S_IRUGO);
MODULE_PARM_DESC(pio_on, "Whether to use pio for out. Default = 0");
module_param(fotg330_dbg, uint, S_IRUGO);
MODULE_PARM_DESC(fotg330_dbg, "Whether to open dbg. Default = 0");
module_param(fotg330_irq_dbg, uint, S_IRUGO);
MODULE_PARM_DESC(fotg330_irq_dbg, "Whether to open irq dbg. Default = 0");

void nvt_usb3dev_u2phy_reset(void)
{
	struct clk *clk = NULL;

	//u2phy
	clk = clk_get(NULL, "u2phy");
	if (!IS_ERR(clk)) {
		pr_info("%s: u2phy reset\n", __func__);
		clk_prepare_enable(clk);
		clk_disable_unprepare(clk);
	} else
		pr_info("%s: clk u2phy not found\n", __func__);
}

void nvt_usb3dev_phy_reset(void)
{
	struct clk *clk = NULL;

	//u3phy
	{
		clk = clk_get(NULL, "u3phy");
		if (!IS_ERR(clk)) {
			pr_info("%s: u3phy reset\n", __func__);
			clk_prepare_enable(clk);
			clk_disable(clk);
			clk_enable(clk);
		} else
			pr_info("%s: u3phy not found\n", __func__);
	}

	//u3glue
	{
		clk = clk_get(NULL, "u3glue");
		if (!IS_ERR(clk)) {
			pr_info("%s: u3glue reset\n", __func__);
			clk_prepare_enable(clk);
			clk_disable(clk);
			clk_enable(clk);
		} else
			pr_info("%s: u3glue not found\n", __func__);
	}

	//u3hi
	{
		clk = clk_get(NULL, "u3hi");
		if (!IS_ERR(clk)) {
			pr_info("%s: u3hi reset\n", __func__);
			clk_prepare_enable(clk);
			clk_disable(clk);
			clk_enable(clk);
		} else
			pr_info("%s: u3hi not found\n", __func__);
	}
}

static void done(struct fusb300_ep *ep, struct fusb300_request *req,
		 int status);

static void fusb300_enable_bit(struct fusb300 *fusb300, u32 offset,
			       u32 value)
{
	u32 reg = ioread32(fusb300->reg + offset);

	reg |= value;
	iowrite32(reg, fusb300->reg + offset);
}

static void fusb300_disable_bit(struct fusb300 *fusb300, u32 offset,
				u32 value)
{
	u32 reg = ioread32(fusb300->reg + offset);

	reg &= ~value;
	iowrite32(reg, fusb300->reg + offset);
}


static void fusb300_ep_setting(struct fusb300_ep *ep,
			       struct fusb300_ep_info info)
{
	ep->epnum = info.epnum;
	ep->type = info.type;
}

static void usb3dev_clear_fifo(struct fusb300_ep *ep)
{
	u32 reg;

	reg = ioread32(ep->fusb300->reg + FUSB300_OFFSET_DMAEPMR);
	reg &= ~(1<<(ep->epnum));
	iowrite32(reg, ep->fusb300->reg + FUSB300_OFFSET_DMAEPMR);
	reg = ioread32(ep->fusb300->reg + FUSB300_OFFSET_EPFFR(ep->epnum));
	reg = reg|(FUSB300_FFR_RST);
	iowrite32(reg, ep->fusb300->reg + FUSB300_OFFSET_EPFFR(ep->epnum));
	reg = ioread32(ep->fusb300->reg + FUSB300_OFFSET_DMAEPMR);
	reg = reg|(1<<(ep->epnum));
	iowrite32(reg, ep->fusb300->reg + FUSB300_OFFSET_DMAEPMR);
}

static int fusb300_ep_release(struct fusb300_ep *ep)
{
	if (!ep->epnum)
		return 0;

	usb3dev_clear_fifo(ep);

	iowrite32(0, ep->fusb300->reg + FUSB300_OFFSET_EPPRD_W0(ep->epnum));
	iowrite32(0, ep->fusb300->reg + FUSB300_OFFSET_EPPRD_W1(ep->epnum));
	iowrite32(0x50, ep->fusb300->reg + FUSB300_OFFSET_EPSET1(ep->epnum));
	iowrite32(0, ep->fusb300->reg + FUSB300_OFFSET_EPSET2(ep->epnum));
	iowrite32(((1 << ep->epnum) << 16), ep->fusb300->reg + FUSB300_OFFSET_IGR5);

	ep->epnum = 0;
	ep->stall = 0;
	ep->wedged = 0;
	ep->type = 0;
	ep->fusb300->fifo_entry_num--;
	ep->fusb300->addrofs -= ((ep->ep.desc->wMaxPacketSize + (fifo_entry_num-1)) / fifo_entry_num * fifo_entry_num);
	return 0;
}

static void fusb300_set_fifo_entry(struct fusb300 *fusb300,
				   u32 ep)
{
	u32 val = ioread32(fusb300->reg + FUSB300_OFFSET_EPSET1(ep));

	val &= ~FUSB300_EPSET1_FIFOENTRY_MSK;
	val |= FUSB300_EPSET1_FIFOENTRY(fifo_entry_num);
	iowrite32(val, fusb300->reg + FUSB300_OFFSET_EPSET1(ep));
}

static void fusb300_set_start_entry(struct fusb300 *fusb300,
				    u8 ep)
{
	u32 reg = ioread32(fusb300->reg + FUSB300_OFFSET_EPSET1(ep));
	u32 start_entry = fusb300->fifo_entry_num * fifo_entry_num;

	reg &= ~FUSB300_EPSET1_START_ENTRY_MSK	;
	reg |= FUSB300_EPSET1_START_ENTRY(start_entry);
	iowrite32(reg, fusb300->reg + FUSB300_OFFSET_EPSET1(ep));
	if (fusb300->fifo_entry_num == FUSB300_MAX_FIFO_ENTRY) {
		fusb300->fifo_entry_num = 0;
		fusb300->addrofs = 0;
		pr_err("fifo entry is over the maximum number!\n");
	} else
		fusb300->fifo_entry_num++;
}

/* set fusb300_set_start_entry first before fusb300_set_epaddrofs */
static void fusb300_set_epaddrofs(struct fusb300 *fusb300,
				  struct fusb300_ep_info info)
{
	u32 reg = ioread32(fusb300->reg + FUSB300_OFFSET_EPSET2(info.epnum));

	reg &= ~FUSB300_EPSET2_ADDROFS_MSK;
	reg |= FUSB300_EPSET2_ADDROFS(fusb300->addrofs);
	iowrite32(reg, fusb300->reg + FUSB300_OFFSET_EPSET2(info.epnum));
	fusb300->addrofs += (info.maxpacket + (fifo_entry_num-1)) / fifo_entry_num * fifo_entry_num;
}

static void ep_fifo_setting(struct fusb300 *fusb300,
			    struct fusb300_ep_info info)
{
	fusb300_set_fifo_entry(fusb300, info.epnum);
	fusb300_set_start_entry(fusb300, info.epnum);
	fusb300_set_epaddrofs(fusb300, info);
}

static void fusb300_set_eptype(struct fusb300 *fusb300,
			       struct fusb300_ep_info info)
{
	u32 reg = ioread32(fusb300->reg + FUSB300_OFFSET_EPSET1(info.epnum));

	reg &= ~FUSB300_EPSET1_TYPE_MSK;
	reg |= FUSB300_EPSET1_TYPE(info.type);
	iowrite32(reg, fusb300->reg + FUSB300_OFFSET_EPSET1(info.epnum));
}

static void fusb300_set_epdir(struct fusb300 *fusb300,
			      struct fusb300_ep_info info)
{
	u32 reg;

	if (!info.dir_in)
		return;
	reg = ioread32(fusb300->reg + FUSB300_OFFSET_EPSET1(info.epnum));
	reg &= ~FUSB300_EPSET1_DIR_MSK;
	reg |= FUSB300_EPSET1_DIRIN;
	iowrite32(reg, fusb300->reg + FUSB300_OFFSET_EPSET1(info.epnum));
}

static void fusb300_set_ep_active(struct fusb300 *fusb300,
			  u8 ep)
{
	u32 reg = ioread32(fusb300->reg + FUSB300_OFFSET_EPSET1(ep));

	reg |= FUSB300_EPSET1_ACTEN;
	iowrite32(reg, fusb300->reg + FUSB300_OFFSET_EPSET1(ep));
}

static void fusb300_set_epmps(struct fusb300 *fusb300,
			      struct fusb300_ep_info info)
{
	u32 reg = ioread32(fusb300->reg + FUSB300_OFFSET_EPSET2(info.epnum));

	reg &= ~FUSB300_EPSET2_MPS_MSK;
	reg |= FUSB300_EPSET2_MPS(info.maxpacket);
	iowrite32(reg, fusb300->reg + FUSB300_OFFSET_EPSET2(info.epnum));
}

static void fusb300_set_interval(struct fusb300 *fusb300,
				 struct fusb300_ep_info info)
{
	u32 reg = ioread32(fusb300->reg + FUSB300_OFFSET_EPSET1(info.epnum));

	reg &= ~FUSB300_EPSET1_INTERVAL(0x7);
	reg |= FUSB300_EPSET1_INTERVAL(info.interval);
	iowrite32(reg, fusb300->reg + FUSB300_OFFSET_EPSET1(info.epnum));
}

static void fusb300_set_isoinpkt(struct fusb300 *fusb300,	 u8 epnum, u32 pktnum)
{
	u32 reg;

	if(pktnum>UVC_ISO_MAX_PKTNUM)
		pktnum = UVC_ISO_MAX_PKTNUM;

	reg = ioread32(fusb300->reg + FUSB300_OFFSET_EPSET1(epnum));

	reg &= ~FUSB300_EPSET1_ISOINPKT(0x3f);
	reg |= FUSB300_EPSET1_ISOINPKT(pktnum);
	iowrite32(reg, fusb300->reg + FUSB300_OFFSET_EPSET1(epnum));
}

static void fusb300_set_bwnum(struct fusb300 *fusb300,
			      struct fusb300_ep_info info)
{
	u32 reg = ioread32(fusb300->reg + FUSB300_OFFSET_EPSET1(info.epnum));

	reg &= ~FUSB300_EPSET1_BWNUM(0x3);
	reg |= FUSB300_EPSET1_BWNUM(info.bw_num);
	iowrite32(reg, fusb300->reg + FUSB300_OFFSET_EPSET1(info.epnum));
}

static void set_ep_reg(struct fusb300 *fusb300,
		      struct fusb300_ep_info info)
{
	fusb300_set_eptype(fusb300, info);
	fusb300_set_epdir(fusb300, info);
	fusb300_set_epmps(fusb300, info);

	if (info.interval)
		fusb300_set_interval(fusb300, info);

	if (info.bw_num)
		fusb300_set_bwnum(fusb300, info);

	fusb300_set_ep_active(fusb300, info.epnum);
}

static int config_ep(struct fusb300_ep *ep,
		     const struct usb_endpoint_descriptor *desc)
{
	struct fusb300 *fusb300 = ep->fusb300;
	struct fusb300_ep_info info;
	//u32 reg;
	//u32 i;

	//pr_info("config_ep ======================= start\n");
	ep->ep.desc = desc;

	info.interval = 0;
	info.addrofs = 0;
	info.bw_num = 0;

	info.type = desc->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK;
	info.dir_in = (desc->bEndpointAddress & USB_ENDPOINT_DIR_MASK) ? 1 : 0;
	info.maxpacket = usb_endpoint_maxp(desc);
	info.epnum = desc->bEndpointAddress & USB_ENDPOINT_NUMBER_MASK;

	if ((info.type == USB_ENDPOINT_XFER_INT) ||
	   (info.type == USB_ENDPOINT_XFER_ISOC)) {
		info.interval = desc->bInterval;
		if (info.type == USB_ENDPOINT_XFER_ISOC)
			info.bw_num = usb_endpoint_maxp_mult(desc);
	}

	ep_fifo_setting(fusb300, info);

	set_ep_reg(fusb300, info);

	fusb300_ep_setting(ep, info);

	if(fotg330_dbg){
		if(info.type == USB_ENDPOINT_XFER_INT)
			pr_info("--[INT]--\n");
		else if(info.type == USB_ENDPOINT_XFER_ISOC)
			pr_info("--[ISO]--\n");
		else if(info.type == USB_ENDPOINT_XFER_CONTROL)
			pr_info("--[CTRL]--\n");
		else if(info.type == USB_ENDPOINT_XFER_BULK)
			pr_info("--[BULKn");

		pr_info("info.epnum = [%d]--\n", info.epnum);
		pr_info("maxburst = [%d]--\n", ep->ep.maxburst);
		pr_info("mult = [%d]--\n", ep->ep.mult);
	}

	if((info.type == USB_ENDPOINT_XFER_ISOC)&&(ep->fusb300->gadget.speed == USB_SPEED_SUPER)){
		fusb300_set_isoinpkt(fusb300, info.epnum, ep->ep.mult*ep->ep.maxburst);
	}

	fusb300->ep[info.epnum] = ep;
	//pr_info("config_ep ======================= end\n");
#if 0
	pr_info(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> \n");
	for(i=1;i<=5;i++)
	{
		pr_info("========[EP%d]===========  \n",i);
		reg = ioread32(fusb300->reg + FUSB300_OFFSET_EPSET0(i));
		pr_info("EP[%d] EPSET0 = 0x%x\n",i,reg);
		reg = ioread32(fusb300->reg + FUSB300_OFFSET_EPSET1(i));
		pr_info("EP[%d] EPSET1 = 0x%x\n",i,reg);
		reg = ioread32(fusb300->reg + FUSB300_OFFSET_EPSET2(i));
		pr_info("EP[%d] EPSET2 = 0x%x\n",i,reg);
		reg = ioread32(fusb300->reg + FUSB300_OFFSET_EPFFR(i));
		pr_info("EP[%d] EPFFR = 0x%x\n",i,reg);
	}
	pr_info(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> \n");
#endif
	return 0;
}

static int fusb330_enable(struct usb_ep *_ep,
			  const struct usb_endpoint_descriptor *desc)
{
	struct fusb300_ep *ep;

	ep = container_of(_ep, struct fusb300_ep, ep);

	if (ep->fusb300->reenum) {
		ep->fusb300->fifo_entry_num = 0;
		ep->fusb300->addrofs = 0;
		ep->fusb300->reenum = 0;
	}

	if(hbw_ep)
	{
		fifo_entry_num = UVC_FIFOENTRY_NUM;
		pr_info("fifo_entry_num = [%d]--\n", fifo_entry_num);
	}

	ep->disable = 0;

	return config_ep(ep, desc);
}

static int fusb330_disable(struct usb_ep *_ep)
{
	struct fusb300_ep *ep;
	struct fusb300_request *req;
	unsigned long flags;
	u32 reg;

	ep = container_of(_ep, struct fusb300_ep, ep);

	BUG_ON(!ep);

	if(ep->epnum) {
		ep->disable = 1;
	}

	while (!list_empty(&ep->queue)) {
		req = list_entry(ep->queue.next, struct fusb300_request, queue);
		spin_lock_irqsave(&ep->fusb300->lock, flags);
		if(dma_int_mode && ep->dma_ongoing) {
			usb3dev_clear_fifo(ep);
			reg = ioread32(ep->fusb300->reg + FUSB300_OFFSET_IGER0);
			reg &= ~FUSB300_IGER0_EEPn_PRD_INT(ep->epnum);
			iowrite32(reg, ep->fusb300->reg + FUSB300_OFFSET_IGER0);

			usb_gadget_unmap_request(&ep->fusb300->gadget,
				&req->req, ep->ep.desc->bEndpointAddress& USB_DIR_IN);

			ep->dma_ongoing = 0;
		}
		done(ep, req, -ECONNRESET);
		spin_unlock_irqrestore(&ep->fusb300->lock, flags);
	}

	return fusb300_ep_release(ep);
}


static struct usb_request *fusb330_alloc_request(struct usb_ep *_ep,
						gfp_t gfp_flags)
{
	struct fusb300_request *req;

	req = kzalloc(sizeof(struct fusb300_request), gfp_flags);
	if (!req)
		return NULL;
	INIT_LIST_HEAD(&req->queue);

	return &req->req;
}

static void fusb330_free_request(struct usb_ep *_ep, struct usb_request *_req)
{
	struct fusb300_request *req;

	req = container_of(_req, struct fusb300_request, req);
	kfree(req);
}

static int enable_fifo_int(struct fusb300_ep *ep)
{
	struct fusb300 *fusb300 = ep->fusb300;

	if (ep->epnum) {
		fusb300_enable_bit(fusb300, FUSB300_OFFSET_IGER0,
			FUSB300_IGER0_EEPn_FIFO_INT(ep->epnum));
	} else {
		pr_err("can't enable_fifo_int ep0\n");
		return -EINVAL;
	}

	return 0;
}

static int disable_fifo_int(struct fusb300_ep *ep)
{
	struct fusb300 *fusb300 = ep->fusb300;

	if (ep->epnum) {
		fusb300_disable_bit(fusb300, FUSB300_OFFSET_IGER0,
			FUSB300_IGER0_EEPn_FIFO_INT(ep->epnum));
	} else {
		pr_err("can't disable_fifo_int ep0\n");
		return -EINVAL;
	}

	return 0;
}

static void fusb300_set_cxlen(struct fusb300 *fusb300, u32 length)
{
	u32 reg;

	reg = ioread32(fusb300->reg + FUSB300_OFFSET_CSR);
	reg &= ~FUSB300_CSR_LEN_MSK;
	reg |= FUSB300_CSR_LEN(length);
	iowrite32(reg, fusb300->reg + FUSB300_OFFSET_CSR);
}

/* write data to cx fifo */
static void fusb300_wrcxf(struct fusb300_ep *ep,
		   struct fusb300_request *req)
{
	int i = 0;
	u8 *tmp;
	u32 data;
	struct fusb300 *fusb300 = ep->fusb300;
	u32 length = req->req.length - req->req.actual;

	tmp = req->req.buf + req->req.actual;

	iowrite32(0x0F0F0002, (fusb300->u3ctrl_reg+0x14));

	if (length > SS_CTL_MAX_PACKET_SIZE) {
		fusb300_set_cxlen(fusb300, SS_CTL_MAX_PACKET_SIZE);
		for (i = (SS_CTL_MAX_PACKET_SIZE >> 2); i > 0; i--) {
			data = *tmp | *(tmp + 1) << 8 | *(tmp + 2) << 16 |
				*(tmp + 3) << 24;
			iowrite32(data, fusb300->reg + FUSB300_OFFSET_CXPORT);
			tmp += 4;
		}
		req->req.actual += SS_CTL_MAX_PACKET_SIZE;
	} else { /* length is less than max packet size */
		//pr_info(" LEN   0x%x\n", length);
		fusb300_set_cxlen(fusb300, length);
		for (i = length >> 2; i > 0; i--) {
			data = *tmp | *(tmp + 1) << 8 | *(tmp + 2) << 16 |
				*(tmp + 3) << 24;
			//pr_info("    0x%x\n", data);
			iowrite32(data, fusb300->reg + FUSB300_OFFSET_CXPORT);
			tmp = tmp + 4;
		}
		switch (length % 4) {
		case 1:
			data = *tmp;
			//pr_info("1    0x%x\n", data);
			iowrite32(data, fusb300->reg + FUSB300_OFFSET_CXPORT);
			break;
		case 2:
			data = *tmp | *(tmp + 1) << 8;
			//pr_info("2    0x%x\n", data);
			iowrite32(data, fusb300->reg + FUSB300_OFFSET_CXPORT);
			break;
		case 3:
			data = *tmp | *(tmp + 1) << 8 | *(tmp + 2) << 16;
			//pr_info("3    0x%x\n", data);
			iowrite32(data, fusb300->reg + FUSB300_OFFSET_CXPORT);
			break;
		default:
			break;
		}
		req->req.actual += length;
	}
	//iowrite32(0x0F0F0003, (fusb300->u3ctrl_reg+0x14));
}


static void fusb300_set_cxstall(struct fusb300 *fusb300)
{
	fusb300_enable_bit(fusb300, FUSB300_OFFSET_CSR,
			   FUSB300_CSR_STL);
}

static void fusb300_set_cxdone(struct fusb300 *fusb300)
{
	fusb300_enable_bit(fusb300, FUSB300_OFFSET_CSR,
			   FUSB300_CSR_DONE);
}

static void fusb300_set_epnstall(struct fusb300 *fusb300, u8 ep)
{
	//pr_info("fusb300_set_epnstall[%d]  fusb300=0x%lx  reg=0x%lX ++\n",ep, (unsigned long)fusb300, (unsigned long)fusb300->reg);
	if (ep)
		fusb300_enable_bit(fusb300, FUSB300_OFFSET_EPSET0(ep), FUSB300_EPSET0_STL);
	 else
		fusb300_set_cxstall(fusb300);
	//pr_info("fusb300_set_epnstall[%d]--\n", ep);
}

static void fusb300_clear_epnstall(struct fusb300 *fusb300, u8 ep)
{
	u32 reg = ioread32(fusb300->reg + FUSB300_OFFSET_EPSET0(ep));

	if (reg & FUSB300_EPSET0_STL) {
		printk(KERN_DEBUG "EP%d stall... Clear!!\n", ep);
		reg |= FUSB300_EPSET0_STL_CLR;
		iowrite32(reg, fusb300->reg + FUSB300_OFFSET_EPSET0(ep));
	}
}

static void ep0_queue(struct fusb300_ep *ep, struct fusb300_request *req)
{
	if (ep->fusb300->ep0_dir) { /* if IN */
		if (req->req.length) {
			fusb300_wrcxf(ep, req);
		} else
			pr_info("%s : req->req.length = 0x%x\n",
				__func__, req->req.length);
		if ((req->req.length == req->req.actual) ||
		    (req->req.actual < ep->ep.maxpacket))
			done(ep, req, 0);
	} else { /* OUT */
		if (!req->req.length)
			done(ep, req, 0);
		else
			fusb300_enable_bit(ep->fusb300, FUSB300_OFFSET_IGER1,
				FUSB300_IGER1_CX_OUT_INT);
	}
}

static int fusb330_queue(struct usb_ep *_ep, struct usb_request *_req,
			 gfp_t gfp_flags)
{
	struct fusb300_ep *ep;
	struct fusb300_request *req;
	unsigned long flags;
	int request  = 0;

	ep = container_of(_ep, struct fusb300_ep, ep);
	req = container_of(_req, struct fusb300_request, req);

	if (ep->fusb300->gadget.speed == USB_SPEED_UNKNOWN)
		return -ESHUTDOWN;

	if(fotg330_dbg)
		pr_info("fusb330_queue\n");

	spin_lock_irqsave(&ep->fusb300->lock, flags);

	if (list_empty(&ep->queue))
		request = 1;

	list_add_tail(&req->queue, &ep->queue);

	req->req.actual = 0;
	req->req.status = -EINPROGRESS;

	if (ep->ep.desc == NULL) /* ep0 */
		ep0_queue(ep, req);
	else if (request && !ep->stall)
		enable_fifo_int(ep);

	spin_unlock_irqrestore(&ep->fusb300->lock, flags);

	return 0;
}

static int fusb330_dequeue(struct usb_ep *_ep, struct usb_request *_req)
{
	struct fusb300_ep *ep;
	struct fusb300_request *req;
	unsigned long flags;
	u32 reg;

	if(fotg330_dbg)
		pr_info("fusb330_dequeue\n");

	ep = container_of(_ep, struct fusb300_ep, ep);
	req = container_of(_req, struct fusb300_request, req);

	spin_lock_irqsave(&ep->fusb300->lock, flags);
	if (!list_empty(&ep->queue)) {
		if(dma_int_mode && ep->dma_ongoing) {
			usb3dev_clear_fifo(ep);
			reg = ioread32(ep->fusb300->reg + FUSB300_OFFSET_IGER0);
			reg &= ~FUSB300_IGER0_EEPn_PRD_INT(ep->epnum);
			iowrite32(reg, ep->fusb300->reg + FUSB300_OFFSET_IGER0);
			iowrite32(FUSB300_IGR0_EPn_PRD_INT(ep->epnum), ep->fusb300->reg + FUSB300_OFFSET_IGR0);
			usb_gadget_unmap_request(&ep->fusb300->gadget, &req->req, ep->ep.desc->bEndpointAddress& USB_DIR_IN);
			pr_info("fusb330_dequeue ongoing dma\n");
			ep->dma_ongoing = 0;
		}
		done(ep, req, -ECONNRESET);
	}

	spin_unlock_irqrestore(&ep->fusb300->lock, flags);

	return 0;
}

static int fusb300_set_halt_and_wedge(struct usb_ep *_ep, int value, int wedge)
{
	struct fusb300_ep *ep;
	struct fusb300 *fusb300;
	unsigned long flags;
	int ret = 0;

	ep = container_of(_ep, struct fusb300_ep, ep);

	fusb300 = ep->fusb300;

	spin_lock_irqsave(&ep->fusb300->lock, flags);

	if (!list_empty(&ep->queue)) {
		ret = -EAGAIN;
		goto out;
	}

	if (ep->fusb300->gadget.speed == USB_SPEED_UNKNOWN) {
		pr_info("no need to set halt_and_wedge\n");
		goto out;
	}

	if (value) {
		fusb300_set_epnstall(fusb300, ep->epnum);
		ep->stall = 1;
		if (wedge)
			ep->wedged = 1;
	} else {
		fusb300_clear_epnstall(fusb300, ep->epnum);
		ep->stall = 0;
		ep->wedged = 0;
	}

out:
	spin_unlock_irqrestore(&ep->fusb300->lock, flags);
	return ret;
}

static int fusb330_set_halt(struct usb_ep *_ep, int value)
{
	return fusb300_set_halt_and_wedge(_ep, value, 0);
}

static int fusb330_set_wedge(struct usb_ep *_ep)
{
	return fusb300_set_halt_and_wedge(_ep, 1, 1);
}

static void fusb330_fifo_flush(struct usb_ep *_ep)
{
}

static const struct usb_ep_ops fusb330_ep_ops = {
	.enable		= fusb330_enable,
	.disable	= fusb330_disable,

	.alloc_request	= fusb330_alloc_request,
	.free_request	= fusb330_free_request,

	.queue		= fusb330_queue,
	.dequeue	= fusb330_dequeue,

	.set_halt	= fusb330_set_halt,
	.fifo_flush	= fusb330_fifo_flush,
	.set_wedge	= fusb330_set_wedge,
};

/*****************************************************************************/
static void fusb300_clear_int(struct fusb300 *fusb300, u32 offset,
		       u32 value)
{
	iowrite32(value, fusb300->reg + offset);
}

static void fusb300_reset(void)
{
}

static void fusb300_soft_reset(struct fusb300 *fusb300)
{
	u32 reg = ioread32(fusb300->reg + FUSB300_OFFSET_GCR);
	reg |= 0x100;
	iowrite32(reg, fusb300->reg + FUSB300_OFFSET_GCR);
}
static void fusb300_enable_uvc(struct fusb300 *fusb300, u8 ep)
{
	u32 reg = ioread32(fusb300->u3ctrl_reg+0x3C);

	reg |= (0x10000<<ep);

	iowrite32(reg, (fusb300->u3ctrl_reg+0x3C));
}

static void fusb300_disable_uvc(struct fusb300 *fusb300, u8 ep)
{
	u32 reg = ioread32(fusb300->u3ctrl_reg+0x3C);

	reg &= ~(0x10000<<ep);

	iowrite32(reg, (fusb300->u3ctrl_reg+0x3C));
}


static bool  fusb300_chkEPFull(struct fusb300_ep *ep)
{
	u32 reg;
	if(ep->epnum> 0) {
		reg = ioread32(ep->fusb300->reg + FUSB300_OFFSET_EPFFR(ep->epnum));
		if(reg & (FUSB300_FF_FUL))
			return 1;
		else
			return 0;
	} else {
		return 0;
	}
}


static bool  fusb300_chkEPEmpty(struct fusb300_ep *ep)
{
	u32 reg;
	if(ep->epnum> 0) {
		reg = ioread32(ep->fusb300->reg + FUSB300_OFFSET_EPFFR(ep->epnum));
		if(reg & (FUSB300_FF_EMPTY))
			return 1;
		else
			return 0;
	} else {
		return 0;
	}
}

/* read data from cx fifo */
static u32 fusb330_rdcxf(struct fusb300 *fusb300,
		   u8 *buffer, u32 length)
{
	int i = 0;
	u8 *tmp;
	u32 data;

	//pr_info("fusb330_rdcxf len =%d\n",length);
	data = ioread32(fusb300->reg + FUSB300_OFFSET_CSR);
	data = (data >>8)&0xFFFF;

	if(length > data) {
		if(fotg330_dbg)
			pr_info("fusb330_rdcxf len reset from 0x%x to 0x%x\n",length, data);
		length = data;
	}

	iowrite32(0x0F0F0002, (fusb300->u3ctrl_reg+0x14));
	tmp = buffer;

	for (i = (length >> 2); i > 0; i--) {
		data = ioread32(fusb300->reg + FUSB300_OFFSET_CXPORT);
		//pr_info("    0x%08x\n", data);
		*tmp = data & 0xFF;
		*(tmp + 1) = (data >> 8) & 0xFF;
		*(tmp + 2) = (data >> 16) & 0xFF;
		*(tmp + 3) = (data >> 24) & 0xFF;
		tmp = tmp + 4;
	}

	switch (length % 4) {
	case 1:
		data = ioread32(fusb300->reg + FUSB300_OFFSET_CXPORT);
			//pr_info("    0x%x\n", data);
		*tmp = data & 0xFF;
		break;
	case 2:
		data = ioread32(fusb300->reg + FUSB300_OFFSET_CXPORT);
			//pr_info("    0x%x\n", data);
		*tmp = data & 0xFF;
		*(tmp + 1) = (data >> 8) & 0xFF;
		break;
	case 3:
		data = ioread32(fusb300->reg + FUSB300_OFFSET_CXPORT);
			//pr_info("    0x%x\n", data);
		*tmp = data & 0xFF;
		*(tmp + 1) = (data >> 8) & 0xFF;
		*(tmp + 2) = (data >> 16) & 0xFF;
		break;
	default:
		break;
	}
	//iowrite32(0x0F0F0003, (fusb300->u3ctrl_reg+0x14));
	return length;
}

static void fusb330_rdfifo(struct fusb300_ep *ep,
			  struct fusb300_request *req,
			  u32 length)
{
	int i = 0;
	u8 *tmp;
	u32 data, reg;
	struct fusb300 *fusb300 = ep->fusb300;

	tmp = req->req.buf + req->req.actual;
	req->req.actual += length;

	if (req->req.actual > req->req.length)
		pr_info("req->req.actual > req->req.length\n");

	for (i = (length >> 2); i > 0; i--) {
		data = ioread32(fusb300->reg +
			FUSB300_OFFSET_EPPORT(ep->epnum));
		*tmp = data & 0xFF;
		*(tmp + 1) = (data >> 8) & 0xFF;
		*(tmp + 2) = (data >> 16) & 0xFF;
		*(tmp + 3) = (data >> 24) & 0xFF;
		tmp = tmp + 4;
	}

	switch (length % 4) {
	case 1:
		data = ioread32(fusb300->reg +
			FUSB300_OFFSET_EPPORT(ep->epnum));
		*tmp = data & 0xFF;
		break;
	case 2:
		data = ioread32(fusb300->reg +
			FUSB300_OFFSET_EPPORT(ep->epnum));
		*tmp = data & 0xFF;
		*(tmp + 1) = (data >> 8) & 0xFF;
		break;
	case 3:
		data = ioread32(fusb300->reg +
			FUSB300_OFFSET_EPPORT(ep->epnum));
		*tmp = data & 0xFF;
		*(tmp + 1) = (data >> 8) & 0xFF;
		*(tmp + 2) = (data >> 16) & 0xFF;
		break;
	default:
		break;
	}

	do {
		reg = ioread32(fusb300->reg + FUSB300_OFFSET_IGR1);
		reg &= FUSB300_IGR1_SYNF0_EMPTY_INT;
		if (i)
			pr_info("sync fifo is not empty!\n");
		i++;
	} while (!reg);
}

static u8 fusb300_get_epnstall(struct fusb300 *fusb300, u8 ep)
{
	u8 value;
	u32 reg = ioread32(fusb300->reg + FUSB300_OFFSET_EPSET0(ep));

	value = reg & FUSB300_EPSET0_STL;

	return value;
}

static u8 fusb300_get_cxstall(struct fusb300 *fusb300)
{
	u8 value;
	u32 reg = ioread32(fusb300->reg + FUSB300_OFFSET_CSR);

	value = (reg & FUSB300_CSR_STL) >> 1;

	return value;
}

static void request_error(struct fusb300 *fusb300)
{
	fusb300_set_cxstall(fusb300);
	pr_info("request error!!\n");
}

static void get_status(struct fusb300 *fusb300, struct usb_ctrlrequest *ctrl)
__releases(fusb300->lock)
__acquires(fusb300->lock)
{
	u8 ep;
	u16 status = 0;
	u16 w_index = ctrl->wIndex;

	switch (ctrl->bRequestType & USB_RECIP_MASK) {
	case USB_RECIP_DEVICE:
		status = 1 << USB_DEVICE_SELF_POWERED;
		status|= (fusb300->remote_wkp << USB_DEVICE_REMOTE_WAKEUP);
		break;
	case USB_RECIP_INTERFACE:
		status = 0;
		break;
	case USB_RECIP_ENDPOINT:
		ep = w_index & USB_ENDPOINT_NUMBER_MASK;
		if (ep) {
			if (fusb300_get_epnstall(fusb300, ep))
				status = 1 << USB_ENDPOINT_HALT;
		} else {
			if (fusb300_get_cxstall(fusb300))
				status = 0;
		}
		break;

	default:
		request_error(fusb300);
		return;		/* exit */
	}

	fusb300->ep0_data = cpu_to_le16(status);
	fusb300->ep0_req->buf = &fusb300->ep0_data;
	fusb300->ep0_req->length = 2;

	spin_unlock(&fusb300->lock);
	fusb330_queue(fusb300->gadget.ep0, fusb300->ep0_req, GFP_KERNEL);
	spin_lock(&fusb300->lock);
}

static void set_feature(struct fusb300 *fusb300, struct usb_ctrlrequest *ctrl)
{
	u8 ep;

	switch (ctrl->bRequestType & USB_RECIP_MASK) {
	case USB_RECIP_DEVICE:
		if(le16_to_cpu(ctrl->wValue) == USB_DEVICE_REMOTE_WAKEUP) {
			fusb300->remote_wkp = 1;
		}
		fusb300_set_cxdone(fusb300);
		break;
	case USB_RECIP_INTERFACE:
		fusb300_set_cxdone(fusb300);
		break;
	case USB_RECIP_ENDPOINT: {
		u16 w_index = le16_to_cpu(ctrl->wIndex);

		ep = w_index & USB_ENDPOINT_NUMBER_MASK;
		if (ep) {
			fusb300_set_epnstall(fusb300, ep);
			fusb300->ep[ep]->stall = 1;
		}
		else
			fusb300_set_cxstall(fusb300);
		fusb300_set_cxdone(fusb300);
		}
		break;
	default:
		request_error(fusb300);
		break;
	}
}

static void fusb300_clear_seqnum(struct fusb300 *fusb300, u8 ep)
{
	fusb300_enable_bit(fusb300, FUSB300_OFFSET_EPSET0(ep),
			    FUSB300_EPSET0_CLRSEQNUM);
}

static int clear_feature(struct fusb300 *fusb300, struct usb_ctrlrequest *ctrl)
{
	int ret = 0;
	struct fusb300_ep *ep =
		fusb300->ep[ctrl->wIndex & USB_ENDPOINT_NUMBER_MASK];

	switch (ctrl->bRequestType & USB_RECIP_MASK) {
	case USB_RECIP_DEVICE:
		if(le16_to_cpu(ctrl->wValue) == USB_DEVICE_REMOTE_WAKEUP) {
			fusb300->remote_wkp = 0;
		}
		fusb300_set_cxdone(fusb300);
		break;
	case USB_RECIP_INTERFACE:
		fusb300_set_cxdone(fusb300);
		break;
	case USB_RECIP_ENDPOINT:
		if (ctrl->wIndex & USB_ENDPOINT_NUMBER_MASK) {
			if (ep->wedged) {
				fusb300_set_cxdone(fusb300);
				break;
			}
			if (ep->stall) {
				ep->stall = 0;
				fusb300_clear_seqnum(fusb300, ep->epnum);
				fusb300_clear_epnstall(fusb300, ep->epnum);
				if (!list_empty(&ep->queue))
					enable_fifo_int(ep);
			}

			if(uvc_enable == 2 && ep->type == USB_ENDPOINT_XFER_BULK) {
				pr_info("bulk uvc stop stream!\n");
				fusb300_clear_seqnum(fusb300, ctrl->wIndex & USB_ENDPOINT_NUMBER_MASK);
				if(dma_int_mode) {
					usb3dev_clear_fifo(ep);
					disable_fifo_int(ep);
				}
				ret = 1;
			}
		}
		fusb300_set_cxdone(fusb300);
		break;
	default:
		request_error(fusb300);
		break;
	}
	return ret;
}

static void fusb330_set_dev_addr(struct fusb300 *fusb300, u16 addr)
{
	u32 reg = ioread32(fusb300->reg + FUSB300_OFFSET_DAR);

	reg &= ~FUSB300_DAR_DRVADDR_MSK;
	reg |= FUSB300_DAR_DRVADDR(addr);

	iowrite32(reg, fusb300->reg + FUSB300_OFFSET_DAR);
}

static void set_address(struct fusb300 *fusb300, struct usb_ctrlrequest *ctrl)
{
	if (ctrl->wValue >= 0x0100)
		request_error(fusb300);
	else {
		fusb330_set_dev_addr(fusb300, ctrl->wValue);
		fusb300_set_cxdone(fusb300);
	}
}

#define UVC_COPY_DESCRIPTORS(mem, src) \
	do { \
		const struct usb_descriptor_header * const *__src; \
		for (__src = src; *__src; ++__src) { \
			memcpy(mem, *__src, (*__src)->bLength); \
			mem += (*__src)->bLength; \
		} \
	} while (0)

static int setup_packet(struct fusb300 *fusb300, struct usb_ctrlrequest *ctrl)
{
	u8 *p = (u8 *)ctrl;
	u8 ret = 0;
	u8 i = 0;
	u32 reg;

	fusb330_rdcxf(fusb300, p, 8);

	fusb300->ep0_dir = ctrl->bRequestType & USB_DIR_IN;
	fusb300->ep0_length = ctrl->wLength;

	if(fotg330_dbg)
		pr_info("ctrl->bRequest =  [0x%x]\n",ctrl->bRequest);
	/* check request */
	if ((ctrl->bRequestType & USB_TYPE_MASK) == USB_TYPE_STANDARD) {
		//pr_info( "[STD]ctrl->bRequest =  [0x%x]\n",ctrl->bRequest);
		switch (ctrl->bRequest) {
		case USB_REQ_GET_STATUS:
			get_status(fusb300, ctrl);
			break;
		case USB_REQ_CLEAR_FEATURE:
			if(clear_feature(fusb300, ctrl)) {
				ret = 1;
			}
			break;
		case USB_REQ_SET_FEATURE:
			set_feature(fusb300, ctrl);
			break;
		case USB_REQ_SET_ADDRESS:
			set_address(fusb300, ctrl);
			break;
		case USB_REQ_SET_CONFIGURATION:
			fusb300_enable_bit(fusb300, FUSB300_OFFSET_DAR,
					   FUSB300_DAR_SETCONFG);
			/* clear sequence number */
			for (i = 1; i <= FUSB330_MAX_NUM_EP; i++)
				fusb300_clear_seqnum(fusb300, i);
			fusb300->reenum = 1;
			ret = 1;
			break;
		default:
			ret = 1;
			break;
		}
	} else{
		//printk(KERN_INFO "[NOSTD]ctrl->bRequest =  [0x%x]\n",ctrl->bRequest);
		ret = 1;
	}

	if(fusb300->ep0_dir) {
		reg = ioread32(fusb300->reg + FUSB300_OFFSET_IGER1);
		reg |= FUSB300_IGER1_CX_IN_INT;
		iowrite32(reg, fusb300->reg + FUSB300_OFFSET_IGER1);
	}

	return ret;
}

static void done(struct fusb300_ep *ep, struct fusb300_request *req,
		 int status)
{
	list_del_init(&req->queue);

	/* don't modify queue heads during completion callback */
	if (ep->fusb300->gadget.speed == USB_SPEED_UNKNOWN)
		req->req.status = -ESHUTDOWN;
	else
		req->req.status = status;

	if(req->req.complete != NULL) {
		spin_unlock(&ep->fusb300->lock);
		usb_gadget_giveback_request(&ep->ep, &req->req);
		spin_lock(&ep->fusb300->lock);
	}

	if (ep->epnum) {
		disable_fifo_int(ep);
		if (!list_empty(&ep->queue) && !ep->disable)
			enable_fifo_int(ep);
	} else
		fusb300_set_cxdone(ep->fusb300);
}

static void fusb300_fill_idma_prdtbl(struct fusb300_ep *ep, dma_addr_t d,
		u32 len, unsigned extra_zero)
{
	u32 value;
	u32 reg;
	u32 fifo_time;

	/* wait SW owner */
	do {
		reg = ioread32(ep->fusb300->reg +
			FUSB300_OFFSET_EPPRD_W0(ep->epnum));
		reg &= FUSB300_EPPRD0_H;
	} while (reg);

	/*wait tx0b done to prevent racing*/
	if(ep->ep.desc->bEndpointAddress&USB_DIR_IN) {
		do {
			reg = ioread32(ep->fusb300->reg +
				FUSB300_OFFSET_EPFFR(ep->epnum));
			reg &= FUSB300_TX0BYTE;
		} while (reg);
	}

	iowrite32(d, ep->fusb300->reg + FUSB300_OFFSET_EPPRD_W1(ep->epnum));

	value = FUSB300_EPPRD0_BTC(len) | FUSB300_EPPRD0_H |
		FUSB300_EPPRD0_F | FUSB300_EPPRD0_L | FUSB300_EPPRD0_I |(extra_zero?FUSB300_EPPRD0_O:0);
	iowrite32(value, ep->fusb300->reg + FUSB300_OFFSET_EPPRD_W0(ep->epnum));

	iowrite32(0x0, ep->fusb300->reg + FUSB300_OFFSET_EPPRD_W2(ep->epnum));

	if((uvc_enable)&&(hbw_ep==ep->epnum)&&(ep->type == USB_ENDPOINT_XFER_ISOC))
	{
		while(1){
			if(fusb300_chkEPEmpty(ep))
			{
				//printf("EP fifo full\n");
				break;
			}
		}

		// Set EP stall
		fusb300_enable_uvc(ep->fusb300, ep->epnum);
	}

	fusb300_enable_bit(ep->fusb300, FUSB300_OFFSET_EPPRDRDY,
		FUSB300_EPPRDR_EP_PRD_RDY(ep->epnum));

	//pr_info("Len = 0x%x\r\n",len);
	if((uvc_enable)&&(hbw_ep==ep->epnum)&&(ep->type == USB_ENDPOINT_XFER_ISOC))
	{
		fifo_time = (ep->fusb300->gadget.speed == USB_SPEED_SUPER)? 2 : 9;
		if(len >= (1024*fifo_entry_num)) {
			while(1){
				if(fusb300_chkEPFull(ep))
				{
					//printf("EP fifo full\n");
					break;
				}
			}
		}else {
			udelay(fifo_time*fifo_entry_num);
		}
		// Release EP stall
		fusb300_disable_uvc(ep->fusb300, ep->epnum);
	}
}

static void fusb300_wait_idma_finished(struct fusb300_ep *ep)
{
	u32 reg;
	unsigned long timeout = jiffies + msecs_to_jiffies(1000);

	do {
		reg = ioread32(ep->fusb300->reg + FUSB300_OFFSET_IGR1);
		if ((reg & FUSB300_IGR1_VBUS_CHG_INT) ||
		    (reg & FUSB300_IGR1_WARM_RST_INT) ||
		    (reg & FUSB300_IGR1_HOT_RST_INT) ||
		    (reg & FUSB300_IGR1_USBRST_INT)
		)
			goto IDMA_RESET;

		if (time_after(jiffies, timeout)) {
			printk("%s timeout\n", __func__);
			usb3dev_clear_fifo(ep);
			goto IDMA_RESET;
		}

		reg = ioread32(ep->fusb300->reg + FUSB300_OFFSET_IGR0);
		reg &= FUSB300_IGR0_EPn_PRD_INT(ep->epnum);
		cpu_relax();
	} while (!reg);

	fusb300_clear_int(ep->fusb300, FUSB300_OFFSET_IGR0,
		FUSB300_IGR0_EPn_PRD_INT(ep->epnum));
	return;

IDMA_RESET:
	reg = ioread32(ep->fusb300->reg + FUSB300_OFFSET_IGER0);
	reg &= ~FUSB300_IGER0_EEPn_PRD_INT(ep->epnum);
	iowrite32(reg, ep->fusb300->reg + FUSB300_OFFSET_IGER0);
}


static void fusb300_idma_finished(struct fusb300_ep *ep, int status)
{
	u32 reg;
	struct fusb300_request *req = list_entry(ep->queue.next,
					struct fusb300_request, queue);

	ep->dma_ongoing = 0;

	fusb300_clear_int(ep->fusb300, FUSB300_OFFSET_IGR0,
		FUSB300_IGR0_EPn_PRD_INT(ep->epnum));

	usb_gadget_unmap_request(&ep->fusb300->gadget,
			&req->req, ep->ep.desc->bEndpointAddress& USB_DIR_IN);

	if(ep->ep.desc->bEndpointAddress & USB_ENDPOINT_DIR_MASK)
		req->req.actual += req->req.length;
	else {
		reg = ioread32(ep->fusb300->reg + FUSB300_OFFSET_EPPRD_W0(ep->epnum));
		req->req.actual += (req->req.length - (reg&FUSB300_FFR_BYCNT));
	}

	done(ep, req, status);
	return;
}


static void fusb300_fill_idma_prdtbl_sg(struct fusb300_ep *ep, dma_addr_t d,
		u32 len)
{
	u32 reg;

	/* wait SW owner */
	do {
		reg = ioread32(ep->fusb300->reg +
			FUSB300_OFFSET_EPPRD_W0(ep->epnum));
		reg &= FUSB300_EPPRD0_H;
	} while (reg);

	iowrite32(d, ep->fusb300->reg + FUSB300_OFFSET_EPPRD_W1(ep->epnum));
	iowrite32(len, ep->fusb300->reg + FUSB300_OFFSET_EPPRD_W0(ep->epnum));

	/*Need to shift 8(size of sg_prd) due to the reason that the first address is setting on iprd*/
	iowrite32(ep->prd_pool_dma + 8, ep->fusb300->reg + FUSB300_OFFSET_EPPRD_W2(ep->epnum));

	fusb300_enable_bit(ep->fusb300, FUSB300_OFFSET_EPPRDRDY,
		FUSB300_EPPRDR_EP_PRD_RDY(ep->epnum));
}


static void fusb300_set_idma(struct fusb300_ep *ep,
			struct fusb300_request *req)
{
	int ret;
	int extra_zero = 0;
	int dma_mapped = 0;
	int i = 0;

	if(req->req.num_mapped_sgs || req->req.dma_mapped) {
		dma_mapped = 1;
	}

	if(!dma_mapped) {
		ret = usb_gadget_map_request(&ep->fusb300->gadget,
				&req->req, ep->ep.desc->bEndpointAddress& USB_DIR_IN);
		if (ret)
			return;
	} else {
		/*if already mapped, no need to map again*/
		printk("no need to map!\n");
	}

	if(dma_int_mode) {
		disable_fifo_int(ep);
		ep->dma_ongoing = 1;
		ep->dma_timeout_jiffies = jiffies + msecs_to_jiffies(5000);
	}

	if(req->req.dma_mapped) {
		/*linear dma situation*/
		fusb300_enable_bit(ep->fusb300, FUSB300_OFFSET_IGER0,
			FUSB300_IGER0_EEPn_PRD_INT(ep->epnum));

		if(ep->ep.desc->bEndpointAddress& USB_DIR_IN) {
			if (ep->epnum && req->req.zero &&
				!usb_endpoint_xfer_isoc(ep->ep.desc) &&
				!(req->req.length%usb_endpoint_maxp(ep->ep.desc)) &&
				uvc_enable == 2 ) {
				extra_zero = 1;
			}
			fusb300_fill_idma_prdtbl(ep, req->req.dma, req->req.length, extra_zero);
		} else {
			fusb300_fill_idma_prdtbl(ep, req->req.dma + req->req.actual, req->req.length, extra_zero);
		}
	} else if(req->req.num_mapped_sgs) {
		/*SGDMA situation*/
		struct scatterlist *sg = req->req.sg;
		struct scatterlist *s;

		if(req->req.num_mapped_sgs > FOTG330_PRD_NUM) {
			pr_err("%s : fotg330 only support 16 sg entry, now you mapping %u..abort!\n", __func__,
				req->req.num_mapped_sgs);

			usb_gadget_unmap_request(&ep->fusb300->gadget,
				&req->req, ep->ep.desc->bEndpointAddress& USB_DIR_IN);

			if(dma_int_mode)
				done(ep, req, -ENOMEM);
			return;
		}

		fusb300_enable_bit(ep->fusb300, FUSB300_OFFSET_IGER0,
			FUSB300_IGER0_EEPn_PRD_INT(ep->epnum));

		for_each_sg(sg, s, req->req.num_mapped_sgs, i) {
			ep->prd_pool[i].prd_word0 = FUSB300_EPPRD0_BTC(sg_dma_len(s)) | FUSB300_EPPRD0_H;
			ep->prd_pool[i].prd_word1 = sg_dma_address(s);
		}
		/*Interrupt Only set in last prd*/
		ep->prd_pool[req->req.num_mapped_sgs-1].prd_word0 |= (FUSB300_EPPRD0_F | FUSB300_EPPRD0_L | FUSB300_EPPRD0_I);

		fusb300_fill_idma_prdtbl_sg(ep, ep->prd_pool[0].prd_word1, ep->prd_pool[0].prd_word0);
	}

	if(!dma_int_mode) {
		/* check idma is done */
		fusb300_wait_idma_finished(ep);

		usb_gadget_unmap_request(&ep->fusb300->gadget,
			&req->req, ep->ep.desc->bEndpointAddress& USB_DIR_IN);
	}
}

static void usb3dev_set_tx0byte(struct fusb300_ep *ep)
{
	u32 reg = ioread32(ep->fusb300->reg + FUSB300_OFFSET_EPFFR(ep->epnum));

	reg = reg|(FUSB300_TX0BYTE);
	iowrite32(reg, ep->fusb300->reg + FUSB300_OFFSET_EPFFR(ep->epnum));
}

static void in_ep_fifo_handler(struct fusb300_ep *ep)
{
	struct fusb300_request *req = list_entry(ep->queue.next,
					struct fusb300_request, queue);

	if (req->req.length) {
		fusb300_set_idma(ep, req);

		if(!dma_int_mode) {
			req->req.actual += req->req.length;
			done(ep, req, 0);
		}
	} else {
		//pr_info("TX0B ep->epnum [0x%x] \r\n",ep->epnum);
		usb3dev_set_tx0byte(ep);
		done(ep, req, 0);
	}
}

static void out_ep_fifo_handler(struct fusb300_ep *ep)
{
	struct fusb300 *fusb300 = ep->fusb300;
	struct fusb300_request *req = list_entry(ep->queue.next,
						 struct fusb300_request, queue);

	u32 reg = ioread32(fusb300->reg + FUSB300_OFFSET_EPFFR(ep->epnum));
	u32 length = reg & FUSB300_FFR_BYCNT;

	if(pio_on) {
		fusb330_rdfifo(ep, req, length);

		if ((req->req.length == req->req.actual) || (length < ep->ep.maxpacket))
			done(ep, req, 0);
	} else {
		fusb300_set_idma(ep, req);

		if(!dma_int_mode) {
			reg = ioread32(ep->fusb300->reg + FUSB300_OFFSET_EPPRD_W0(ep->epnum));
			req->req.actual += (req->req.length - (reg&FUSB300_FFR_BYCNT));
			done(ep, req, 0);
		}
	}
}

static void check_device_mode(struct fusb300 *fusb300)
{
	u32 reg = ioread32(fusb300->reg + FUSB300_OFFSET_GCR);
	static u32 dev_mode = FUSB300_GCR_DEVDIS;

	switch (reg & FUSB300_GCR_DEVEN_MSK) {
	case FUSB300_GCR_DEVEN_SS:
		fusb300->gadget.speed = USB_SPEED_SUPER;
		//printk(KERN_INFO "dev_mode = SS\n");
		break;
	case FUSB300_GCR_DEVEN_HS:
		fusb300->gadget.speed = USB_SPEED_HIGH;

		if(dev_mode == FUSB300_GCR_DEVDIS && force_rst ) {
			printk("reset again\n");
			force_rst = 0;
			fusb300_soft_reset(fusb300);
		}

		//printk(KERN_INFO "dev_mode = HS\n");
		break;
	case FUSB300_GCR_DEVEN_FS:
		fusb300->gadget.speed = USB_SPEED_FULL;
		//printk(KERN_INFO "dev_mode = FS\n");
		break;
	default:
		fusb300->gadget.speed = USB_SPEED_UNKNOWN;
		//printk(KERN_INFO "dev_mode = UNKNOWN\n");
		break;
	}
	//printk(KERN_INFO "dev_mode = %d\n", (reg & FUSB300_GCR_DEVEN_MSK));
}


static void fusb330_ep0out(struct fusb300 *fusb300)
{
	struct fusb300_ep *ep = fusb300->ep[0];
	u32 reg;

	if (!list_empty(&ep->queue)) {
		struct fusb300_request *req;

		req = list_first_entry(&ep->queue,
			struct fusb300_request, queue);
		if (req->req.length)
			req->req.actual = fusb330_rdcxf(ep->fusb300, req->req.buf,
				req->req.length);

		//printk(KERN_INFO "fusb330_rdcxf len Before done =%d\n",req->req.length);
		done(ep, req, 0);
		//printk(KERN_INFO "fusb330_rdcxf len After done =%d\n",req->req.length);
		reg = ioread32(fusb300->reg + FUSB300_OFFSET_IGER1);
		reg &= ~FUSB300_IGER1_CX_OUT_INT;
		//printk(KERN_INFO "reg =%d\n",req);
		iowrite32(reg, fusb300->reg + FUSB300_OFFSET_IGER1);
	} else
		pr_err("%s : empty queue\n", __func__);

	//printk(KERN_INFO "fusb330_ep0out done\n");
}

static void fusb330_ep0in(struct fusb300 *fusb300)
{
	struct fusb300_request *req;
	struct fusb300_ep *ep = fusb300->ep[0];
	u32 reg;

	if ((!list_empty(&ep->queue)) && (fusb300->ep0_dir)) {
		req = list_entry(ep->queue.next,
				struct fusb300_request, queue);
		if (req->req.length)
			fusb300_wrcxf(ep, req);
		if ((req->req.length - req->req.actual) < ep->ep.maxpacket)
			done(ep, req, 0);
	} else {
		fusb300_set_cxdone(fusb300);
		reg = ioread32(fusb300->reg + FUSB300_OFFSET_IGER1);
		reg &= ~FUSB300_IGER1_CX_IN_INT;
		iowrite32(reg, fusb300->reg + FUSB300_OFFSET_IGER1);
	}
}

static void fusb330_grp2_handler(void)
{
}

static void fusb330_grp3_handler(void)
{
}

static void fusb330_grp4_handler(void)
{
}

static void fusb330_grp5_handler(void)
{
}

static irqreturn_t fusb330_irq(int irq, void *_fusb300)
{
	struct fusb300 *fusb300 = _fusb300;
	u32 int_grp1 = ioread32(fusb300->reg + FUSB300_OFFSET_IGR1);
	u32 int_grp1_en = ioread32(fusb300->reg + FUSB300_OFFSET_IGER1);
	u32 int_grp0 = ioread32(fusb300->reg + FUSB300_OFFSET_IGR0);
	u32 int_grp0_en = ioread32(fusb300->reg + FUSB300_OFFSET_IGER0);
	struct usb_ctrlrequest ctrl;
	u8 in;
	u32 reg;
	int i;

	spin_lock(&fusb300->lock);

	//pr_info("fusb330_irq [0x%x] [0x%x]\r\n",int_grp0,int_grp1);

	int_grp1 &= int_grp1_en;
	int_grp0 &= int_grp0_en;
	iowrite32(int_grp0, fusb300->reg + FUSB300_OFFSET_IGR0);
	iowrite32(int_grp1, fusb300->reg + FUSB300_OFFSET_IGR1);

	//pr_info("fusb330_irq en [0x%x] [0x%x]\r\n",int_grp0,int_grp1);

	if (int_grp1 & FUSB300_IGR1_WARM_RST_INT) {
		//fusb300_clear_int(fusb300, FUSB300_OFFSET_IGR1,
		//		  FUSB300_IGR1_WARM_RST_INT);
		if(fotg330_irq_dbg)
			printk(KERN_INFO"fusb330_warmreset\n");
		fusb300_reset();
	}

	if (int_grp1 & FUSB300_IGR1_HOT_RST_INT) {
		//fusb300_clear_int(fusb300, FUSB300_OFFSET_IGR1,
		//		  FUSB300_IGR1_HOT_RST_INT);
		if(fotg330_irq_dbg)
			printk(KERN_INFO"fusb330_hotreset\n");
		fusb300_reset();
	}

	if (int_grp1 & FUSB300_IGR1_USBRST_INT) {
		//fusb300_clear_int(fusb300, FUSB300_OFFSET_IGR1,
		//		  FUSB300_IGR1_USBRST_INT);
		if(fotg330_irq_dbg)
			printk(KERN_INFO"fusb330_reset\n");
		fusb300_reset();

		if(fusb300->gadget.speed == USB_SPEED_HIGH && uvc_enable == 2) {
			printk("force disconnect function fot highspeed uvc\n");
			fusb300->driver->disconnect(&fusb300->gadget);
			fusb300_soft_reset(fusb300);
		}
	}
	/* COMABT_INT has a highest priority */

	if (int_grp1 & FUSB300_IGR1_CX_COMABT_INT) {
		//fusb300_clear_int(fusb300, FUSB300_OFFSET_IGR1,
		//		  FUSB300_IGR1_CX_COMABT_INT);
		if(fotg330_irq_dbg)
			printk(KERN_INFO"fusb330_ep0abt\n");
	}

	if (int_grp1 & FUSB300_IGR1_VBUS_CHG_INT) {
		//fusb300_clear_int(fusb300, FUSB300_OFFSET_IGR1,
		//		  FUSB300_IGR1_VBUS_CHG_INT);
		if(fotg330_irq_dbg)
			printk(KERN_INFO"fusb330_vbus_change\n");
	}

	if (int_grp1 & FUSB300_IGR1_U3_EXIT_FAIL_INT) {
		//fusb300_clear_int(fusb300, FUSB300_OFFSET_IGR1,
		//		  FUSB300_IGR1_U3_EXIT_FAIL_INT);
	}

	if (int_grp1 & FUSB300_IGR1_U2_EXIT_FAIL_INT) {
		//fusb300_clear_int(fusb300, FUSB300_OFFSET_IGR1,
		//		  FUSB300_IGR1_U2_EXIT_FAIL_INT);
	}

	if (int_grp1 & FUSB300_IGR1_U1_EXIT_FAIL_INT) {
		//fusb300_clear_int(fusb300, FUSB300_OFFSET_IGR1,
		//		  FUSB300_IGR1_U1_EXIT_FAIL_INT);
	}

	if (int_grp1 & FUSB300_IGR1_U2_ENTRY_FAIL_INT) {
		//fusb300_clear_int(fusb300, FUSB300_OFFSET_IGR1,
		//		  FUSB300_IGR1_U2_ENTRY_FAIL_INT);
	}

	if (int_grp1 & FUSB300_IGR1_U1_ENTRY_FAIL_INT) {
		//fusb300_clear_int(fusb300, FUSB300_OFFSET_IGR1,
		//		  FUSB300_IGR1_U1_ENTRY_FAIL_INT);
	}

	if (int_grp1 & FUSB300_IGR1_U3_EXIT_INT) {
		//fusb300_clear_int(fusb300, FUSB300_OFFSET_IGR1,
		//		  FUSB300_IGR1_U3_EXIT_INT);
		printk(KERN_INFO "FUSB330_IGR1_U3_EXIT_INT\n");
	}

	if (int_grp1 & FUSB300_IGR1_U2_EXIT_INT) {
		//fusb300_clear_int(fusb300, FUSB300_OFFSET_IGR1,
		//		  FUSB300_IGR1_U2_EXIT_INT);
		printk(KERN_INFO "FUSB330_IGR1_U2_EXIT_INT\n");
	}

	if (int_grp1 & FUSB300_IGR1_U1_EXIT_INT) {
		//fusb300_clear_int(fusb300, FUSB300_OFFSET_IGR1,
		//		  FUSB300_IGR1_U1_EXIT_INT);
		printk(KERN_INFO "FUSB330_IGR1_U1_EXIT_INT\n");
	}

	if (int_grp1 & FUSB300_IGR1_U3_ENTRY_INT) {
		//fusb300_clear_int(fusb300, FUSB300_OFFSET_IGR1,
		//		  FUSB300_IGR1_U3_ENTRY_INT);
		printk(KERN_INFO "FUSB330_IGR1_U3_ENTRY_INT\n");
		fusb300_enable_bit(fusb300, FUSB300_OFFSET_SSCR1,
				   FUSB300_SSCR1_GO_U3_DONE);
	}

	if (int_grp1 & FUSB300_IGR1_U2_ENTRY_INT) {
		//fusb300_clear_int(fusb300, FUSB300_OFFSET_IGR1,
		//		  FUSB300_IGR1_U2_ENTRY_INT);
		if(fotg330_irq_dbg)
			printk(KERN_INFO "FUSB330_IGR1_U2_ENTRY_INT\n");
	}

	if (int_grp1 & FUSB300_IGR1_U1_ENTRY_INT) {
		//fusb300_clear_int(fusb300, FUSB300_OFFSET_IGR1,
		//		  FUSB300_IGR1_U1_ENTRY_INT);
		if(fotg330_irq_dbg)
			printk(KERN_INFO "FUSB330_IGR1_U1_ENTRY_INT\n");
	}

	if (int_grp1 & FUSB300_IGR1_RESM_INT) {
		//fusb300_clear_int(fusb300, FUSB300_OFFSET_IGR1,
		//		  FUSB300_IGR1_RESM_INT);
		if(fotg330_irq_dbg)
			printk(KERN_INFO "fusb330_resume\n");
	}

	if (int_grp1 & FUSB300_IGR1_SUSP_INT) {
		//fusb300_clear_int(fusb300, FUSB300_OFFSET_IGR1,
		//		  FUSB300_IGR1_SUSP_INT);
		//usb_gadget_set_state(&fusb300->gadget, USB_STATE_NOTATTACHED);
		if(fusb300->remote_wkp) {
			printk("go suspend!!\n");
			reg = ioread32(fusb300->reg + FUSB300_OFFSET_HSCR);
			reg |= FUSB300_HSCR_HS_GOSUSP;
			iowrite32(reg, fusb300->reg + FUSB300_OFFSET_HSCR);
		}
		if(fotg330_irq_dbg)
			printk(KERN_INFO "fusb330_suspend\n");
	}

	if (int_grp1 & FUSB300_IGR1_HS_LPM_INT) {
		//fusb300_clear_int(fusb300, FUSB300_OFFSET_IGR1,
		//		  FUSB300_IGR1_HS_LPM_INT);
		if(fotg330_irq_dbg)
			printk(KERN_INFO "fusb330_HS_LPM_INT\n");
	}

	if (int_grp1 & FUSB300_IGR1_DEV_MODE_CHG_INT) {
		//fusb300_clear_int(fusb300, FUSB300_OFFSET_IGR1,
		//		  FUSB300_IGR1_DEV_MODE_CHG_INT);
		check_device_mode(fusb300);
	}

	if (int_grp1 & FUSB300_IGR1_CX_COMFAIL_INT) {
		fusb300_set_cxstall(fusb300);
		printk(KERN_INFO "fusb330_ep0fail\n");
	}

	if (int_grp1 & FUSB300_IGR1_CX_SETUP_INT) {
		if(fotg330_irq_dbg)
			printk(KERN_INFO "fusb330_ep0setup\n");
		if (setup_packet(fusb300, &ctrl)) {
			spin_unlock(&fusb300->lock);
			if (fusb300->driver->setup(&fusb300->gadget, &ctrl) < 0)
				fusb300_set_cxstall(fusb300);
			spin_lock(&fusb300->lock);
		}
	}

	if (int_grp1 & FUSB300_IGR1_CX_CMDEND_INT)
	{
		if(fotg330_irq_dbg)
			printk(KERN_INFO "fusb330_cmdend\n");
	}


	if (int_grp1 & FUSB300_IGR1_CX_OUT_INT) {
		if(fotg330_irq_dbg)
			printk(KERN_INFO "fusb330_cxout\n");
		fusb330_ep0out(fusb300);
	}

	if (int_grp1 & FUSB300_IGR1_CX_IN_INT) {
		if(fotg330_irq_dbg)
			printk(KERN_INFO "fusb330_cxin\n");
		fusb330_ep0in(fusb300);
	}

	if (int_grp1 & FUSB300_IGR1_INTGRP5)
		fusb330_grp5_handler();

	if (int_grp1 & FUSB300_IGR1_INTGRP4)
		fusb330_grp4_handler();

	if (int_grp1 & FUSB300_IGR1_INTGRP3)
		fusb330_grp3_handler();

	if (int_grp1 & FUSB300_IGR1_INTGRP2)
		fusb330_grp2_handler();

	if (int_grp0) {
		for (i = 1; i <= FUSB330_MAX_NUM_EP; i++) {
			if (int_grp0 & FUSB300_IGR0_EPn_FIFO_INT(i)) {
				reg = ioread32(fusb300->reg +
					FUSB300_OFFSET_EPSET1(i));
				in = (reg & FUSB300_EPSET1_DIRIN) ? 1 : 0;
				if (in)
					in_ep_fifo_handler(fusb300->ep[i]);
				else
					out_ep_fifo_handler(fusb300->ep[i]);
			}

			if(dma_int_mode) {
				if (int_grp0 & FUSB300_IGR0_EPn_PRD_INT(i)) {
					fusb300_idma_finished(fusb300->ep[i], 0);
				}
			}
		}
	}

	spin_unlock(&fusb300->lock);

	return IRQ_HANDLED;
}

static void fusb330_set_u2_timeout(struct fusb300 *fusb300,
				   u32 time)
{
	u32 reg;

	reg = ioread32(fusb300->reg + FUSB300_OFFSET_TT);
	reg &= ~0xff;
	reg |= FUSB300_SSCR2_U2TIMEOUT(time);

	iowrite32(reg, fusb300->reg + FUSB300_OFFSET_TT);
}

static void fusb330_set_u1_timeout(struct fusb300 *fusb300,
				   u32 time)
{
	u32 reg;

	reg = ioread32(fusb300->reg + FUSB300_OFFSET_TT);
	reg &= ~(0xff << 8);
	reg |= FUSB300_SSCR2_U1TIMEOUT(time);

	iowrite32(reg, fusb300->reg + FUSB300_OFFSET_TT);
}


#define USB3PHY_SETREG(ofs,value)   iowrite32(value, gfusb330->u3phy_reg + ((ofs)<<2))
#define USB3PHY_GETREG(ofs)        ioread32(gfusb330->u3phy_reg +((ofs)<<2))
#define USB3U2PHY_SETREG(ofs,value) iowrite32(value, gfusb330->u3_utmi_phy_reg + ((ofs)<<2))
#define USB3U2PHY_GETREG(ofs)       ioread32(gfusb330->u3_utmi_phy_reg +((ofs)<<2))

#define U3PIPE_PHY_BASE 0xF0282000
#define U3UTMI_PHY_BASE 0xF0281000
static ssize_t nvt_hwdump_sysfs_show(struct device *dev,
		struct device_attribute *attr,
		char *buf)
{
	printk("=======================================\n");
	printk("============ USB2 trim dump ============\n");
	printk("========================================\n");
	printk("[TRIM][U2] 0x%x = (0x%x)\n", (U3UTMI_PHY_BASE + 0x06*4), USB3U2PHY_GETREG(0x06));
	printk("[TRIM][U2] 0x%x = (0x%x)\n", (U3UTMI_PHY_BASE + 0x05*4), USB3U2PHY_GETREG(0x05));
	printk("[TRIM][U2] 0x%x = (0x%x)\n", (U3UTMI_PHY_BASE + 0x3B*4), USB3U2PHY_GETREG(0x3B));


	printk("=======================================\n");
	printk("============ USB3 trim dump ============\n");
	printk("========================================\n");
	printk("[TRIM][U3] 0x%x = (0x%x)\n", (U3PIPE_PHY_BASE + 0x102*4), USB3PHY_GETREG(0x102));
	printk("[TRIM][U3] 0x%x = (0x%x)\n", (U3PIPE_PHY_BASE + 0x115*4), USB3PHY_GETREG(0x115));
	printk("[TRIM][U3] 0x%x = (0x%x)\n", (U3PIPE_PHY_BASE + 0x1A3*4), USB3PHY_GETREG(0x1A3));
	printk("[TRIM][U3] 0x%x = (0x%x)\n", (U3PIPE_PHY_BASE + 0x1A1*4), USB3PHY_GETREG(0x1A1));
	printk("[TRIM][U3] 0x%x = (0x%x)\n", (U3PIPE_PHY_BASE + 0x1A0*4), USB3PHY_GETREG(0x1A0));
	printk("[TRIM][U3] 0x%x = (0x%x)\n", (U3PIPE_PHY_BASE + 0x30*4), USB3PHY_GETREG(0x30));
	printk("[TRIM][U3] 0x%x = (0x%x)\n", (U3PIPE_PHY_BASE + 0x10*4), USB3PHY_GETREG(0x10));


	printk("============================================\n");
	printk("============ USB2 best setting  ============\n");
	printk("============================================\n");
	printk("[BEST_S][U2] 0x%x = (0x%x)\n", (U3PIPE_PHY_BASE + 0x56*4), USB3PHY_GETREG(0x56));
	printk("[BEST_S][U2] 0x%x = (0x%x)\n", (U3UTMI_PHY_BASE + 0x12*4), USB3U2PHY_GETREG(0x12));


	printk("============================================\n");
	printk("============ USB3 best setting  ============\n");
	printk("============================================\n");
	printk("[BEST_S][U3] 0x%x = (0x%x)\n", (U3PIPE_PHY_BASE + 0x1BF*4), USB3PHY_GETREG(0x1BF));
	printk("[BEST_S][U3] 0x%x = (0x%x)\n", (U3PIPE_PHY_BASE + 0x12*4), USB3PHY_GETREG(0x12));
	printk("[BEST_S][U3] 0x%x = (0x%x)\n", (U3PIPE_PHY_BASE + 0x14*4), USB3PHY_GETREG(0x14));
	printk("[BEST_S][U3] 0x%x = (0x%x)\n", (U3PIPE_PHY_BASE + 0x34*4), USB3PHY_GETREG(0x34));
	printk("[BEST_S][U3] 0x%x = (0x%x)\n", (U3PIPE_PHY_BASE + 0x114*4), USB3PHY_GETREG(0x114));
	printk("[BEST_S][U3] 0x%x = (0x%x)\n", (U3PIPE_PHY_BASE + 0x152*4), USB3PHY_GETREG(0x152));
	printk("[BEST_S][U3] 0x%x = (0x%x)\n", (U3PIPE_PHY_BASE + 0x153*4), USB3PHY_GETREG(0x153));
	printk("[BEST_S][U3] 0x%x = (0x%x)\n", (U3PIPE_PHY_BASE + 0x1B0*4), USB3PHY_GETREG(0x1B0));
	printk("[BEST_S][U3] 0x%x = (0x%x)\n", (U3PIPE_PHY_BASE + 0x1B1*4), USB3PHY_GETREG(0x1B1));
	printk("[BEST_S][U3] 0x%x = (0x%x)\n", (U3PIPE_PHY_BASE + 0x1B2*4), USB3PHY_GETREG(0x1B2));
	printk("[BEST_S][U3] 0x%x = (0x%x)\n", (U3PIPE_PHY_BASE + 0x135*4), USB3PHY_GETREG(0x135));
	printk("[BEST_S][U3] 0x%x = (0x%x)\n", (U3PIPE_PHY_BASE + 0x12A*4), USB3PHY_GETREG(0x12A));
	printk("[BEST_S][U3] 0x%x = (0x%x)\n", (U3PIPE_PHY_BASE + 0x105*4), USB3PHY_GETREG(0x105));
	printk("[BEST_S][U3] 0x%x = (0x%x)\n", (U3PIPE_PHY_BASE + 0x27B*4), USB3PHY_GETREG(0x27B));
	printk("[BEST_S][U3] 0x%x = (0x%x)\n", (U3PIPE_PHY_BASE + 0x141*4), USB3PHY_GETREG(0x141));
	printk("[BEST_S][U3] 0x%x = (0x%x)\n", (U3PIPE_PHY_BASE + 0x140*4), USB3PHY_GETREG(0x140));
	printk("[BEST_S][U3] 0x%x = (0x%x)\n", (U3PIPE_PHY_BASE + 0x56*4), USB3PHY_GETREG(0x56));
	printk("[BEST_S][U3] 0x%x = (0x%x)\n", (U3PIPE_PHY_BASE + 0x11*4), USB3PHY_GETREG(0x11));
	printk("[BEST_S][U3] 0x%x = (0x%x)\n", (U3PIPE_PHY_BASE + 0x31*4), USB3PHY_GETREG(0x31));
	printk("[BEST_S][U3] 0x%x = (0x%x)\n", (U3PIPE_PHY_BASE + 0x13*4), USB3PHY_GETREG(0x13));


        return 0;
}

static DEVICE_ATTR(hwdump, S_IWUSR | S_IRUGO,
                nvt_hwdump_sysfs_show, NULL);

static struct attribute *nvt_hwdump_sysfs_attributes[] = {
        &dev_attr_hwdump.attr,
        NULL,
};

static const struct attribute_group nvt_hwdump_sysfs_attr_group = {
        .attrs = nvt_hwdump_sysfs_attributes,
};

static int nvt_hwdump_sysfs_init(struct device *dev)
{
        return sysfs_create_group(&dev->kobj, &nvt_hwdump_sysfs_attr_group);
}

static void nvt_hwdump_sysfs_remove(struct device *dev)
{
        sysfs_remove_group(&dev->kobj, &nvt_hwdump_sysfs_attr_group);
}

#ifndef CONFIG_NVT_FPGA_EMULATION
#if defined(CONFIG_NVT_IVOT_PLAT_NA51102)
static void usb3_u2u3phyinit(void)
{
	//UINT8 u3_trim_rint_sel=8,u3_trim_swctrl=4, u3_trim_sqsel=4,u3_trim_icdr=0xB;
	u8 USB_SATA_Internal_12K_resistor=8;
	u32 temp;
	u8 USB_SATA_TX_TRIM = 0x6;
	u8 USB_SATA_RX_TRIM = 0xC;
	u8 u2_trim_swctrl = 4;
	//u8 u3_trim_swctrl;

	if(fotg330_dbg)
		pr_info("usb3_u2u3phyinit\r\n");

	// Bank 0
	USB3PHY_SETREG(0xFF, 0x00);

	temp = USB3PHY_GETREG(0x97);
	temp |= 0x3;
	USB3PHY_SETREG(0x97 , temp);

#if 1//USB3_CONFIG_TRIM
	{
		s32       trim;
		s32       addr;
		u16      value;

		trim = efuse_readParamOps(EFUSE_USB_SATA_TRIM_DATA, &value);
		addr = EFUSE_USB_SATA_TRIM_DATA;

		if (trim == 0) {
			//12K resistor 1<= x <= 20(0x14)
			//usb3dbg_evt(("  USB(SATA) Trim data read success addr[0x%02x] = 0x%04x\r\n", (int)addr, (int)value));
			//usb3dbg_evt(("  12K_resistor[4..0] valid from 1(0x1) ~ 20(0x14)\r\n"));
			USB_SATA_Internal_12K_resistor = value & 0x1F;
			USB_SATA_TX_TRIM = ((value >> 5)&0xF);
			USB_SATA_RX_TRIM = ((value >> 9)&0xF);

			if (USB_SATA_Internal_12K_resistor > 0x14) {
				pr_err("12K_resistor Trim data error [0x%02x]=0x%04x > 20(0x14)\r\n", (int)addr, (int)USB_SATA_Internal_12K_resistor);
				USB_SATA_Internal_12K_resistor=8;
			} else if (USB_SATA_Internal_12K_resistor < 0x1) {
				pr_err("12K_resistor Trim data error [0x%02x]=0x%04x < 1(0x1)\r\n", (int)addr, (int)USB_SATA_Internal_12K_resistor);
				USB_SATA_Internal_12K_resistor=8;
			} else {
				pr_info("  *12K_resistor Trim data range success 0x1 <= [0x%04x] <= 0x14\r\n", (int)USB_SATA_Internal_12K_resistor);
			}

			//TX term bit[8..5] 0x2 <= x <= 0xE
			if (USB_SATA_TX_TRIM > 0xE) {
				pr_err("USB(SATA) TX Trim data error [0x%02x]=0x%04x > 0xE\r\n", (int)addr, (int)USB_SATA_TX_TRIM);
				USB_SATA_TX_TRIM = 0x6;
			} else if (USB_SATA_TX_TRIM < 0x2) {
				pr_err("USB(SATA) TX Trim data error [0x%02x]=0x%04x < 0x2\r\n", (int)addr, (int)USB_SATA_TX_TRIM);
				USB_SATA_TX_TRIM = 0x6;
			} else {
				pr_info("  *USB(SATA) TX Trim data range success 0x2 <= [0x%04x] <= 0xE\r\n", (int)USB_SATA_TX_TRIM);
			}
			//RX term bit[12..9] 0x8 <= x <= 0xE
			if (USB_SATA_RX_TRIM > 0xE) {
				pr_err("USB(SATA) RX Trim data error [0x%02x]=0x%04x > 0xE\r\n", (int)addr, (int)USB_SATA_RX_TRIM);
				USB_SATA_RX_TRIM = 0xC;
			} else if (USB_SATA_RX_TRIM < 0x8) {
				pr_err("USB(SATA) RX Trim data error [0x%02x]=0x%04x < 0x8\r\n", (int)addr, (int)USB_SATA_RX_TRIM);
				USB_SATA_RX_TRIM = 0xC;
			} else {
				pr_info("  *USB(SATA) RX Trim data range success 0x8 <= [0x%04x] <= 0xE\r\n", (int)USB_SATA_RX_TRIM);
			}
		} else {
			pr_err("efuse trim : addr[%02x] = %d\r\n", (int)addr, (int)trim);
			//return;
		}

		// UINT16 data=0;
		// INT32 result;

		// result= efuse_readParamOps(EFUSE_USBC_TRIM_DATA, &data);
		// if(result >= 0) {
			// u3_trim_swctrl = data&0x7;
			// u3_trim_sqsel  = (data>>3)&0x7;
			// u3_trim_rint_sel = (data>>6)&0x1F;
		// }

		//usb3_validateTrim();

		temp = USB3PHY_GETREG(0x1A1);
		temp &= ~(0x80);
		USB3PHY_SETREG(0x1A1 , temp);
             //pr_info(" temp = [0x%04x]\r\n", (int)temp);
	      //pr_info(" res = [0x%04x]\r\n", (int)USB_SATA_Internal_12K_resistor);
		USB3PHY_SETREG(0x1A0, 0x40+USB_SATA_Internal_12K_resistor);
		USB3PHY_SETREG(0x1A3, 0x60);

		//======================================================
		//(3) USB20 swing & sqsel trim bits removed in INT98530
		//======================================================
		//temp = USB3U2PHY_GETREG(0x06);
		//temp &= ~(0x7<<1);
		//temp |= (u3_trim_swctrl<<1);
		//USB3U2PHY_SETREG(0x06, temp);

		//temp = USB3U2PHY_GETREG(0x05);
		//temp &= ~(0x7<<2);
		//temp |= (u3_trim_sqsel<<2);
		//USB3U2PHY_SETREG(0x05, temp);
	}
#else
	temp = USB3PHY_GETREG(0x1A1);
	temp &= ~(0x80);
	USB3U2PHY_SETREG(0x1A1 , temp);
	USB3PHY_SETREG(0x1A0, 0x40+USB_SATA_Internal_12K_resistor);
	USB3PHY_SETREG(0x1A3, 0x60);
#endif
	//========================================================
	//add additonal setting for 0.85V
	//========================================================
	// Bank 2
	USB3PHY_SETREG(0xFF, 0x02);
	// bit[5:4]  = LDO_VLDO_SEL_DCO[1:0] ; 00 = 0.85v
	USB3PHY_SETREG(0x2A, 0x80);
	// Bank 0
	USB3PHY_SETREG(0xFF, 0x00);
	// bit[1:0]  = LDO_SEL_18[1:0] ; 00 = 0.85v
	USB3PHY_SETREG(0x64, 0x44);
	// bit[1:0] = TXPLL_CP[1:0] = 01
	USB3PHY_SETREG(0x65, 0X99);

	// Bank 0
	USB3PHY_SETREG(0xFF, 0x00);
	// bit[5:4] = V_VCHSEL[1:0] = 11
	USB3PHY_SETREG(0x56, 0X74);
	// TX_AMP_CTL=2, TX_DEC_EM_CTL=8
	USB3PHY_SETREG(0x14, 0X8A);
	// bit[5:4] = TX_AMP_CTL_LFPS[1:0]
	//USB3PHY_SETREG(0x12, 0XE8);
	USB3PHY_SETREG(0x12, 0XF8);
	// bit3 : TX_DE_EN manual mode
	// bit[7:4]: TX_DEC_EM_CTL manual mode
	USB3PHY_SETREG(0x34, 0Xfc);

	//======================================================
	//(4) TX termination trim bits								[NEW]
	//======================================================
	// 0xF05F2040[7:4] = tx_term_trim_val[3:0]
	// Bank 0
	USB3PHY_SETREG(0xFF, 0x00);

	temp = USB3PHY_GETREG(0x10);
	temp &= ~(0xF0);
	temp |= (USB_SATA_TX_TRIM<<4);
	USB3PHY_SETREG(0x10 , temp);

	//======================================================
	//(5) RX termination trim bits								[NEW]
	//======================================================
	// Bank 2
	USB3PHY_SETREG(0xFF, 0x02);

	temp = USB3PHY_GETREG(0x0);
	temp &= ~(0xF);
	temp |= (USB_SATA_RX_TRIM);
	USB3PHY_SETREG(0x00 , temp);

	// Bank 0
	USB3PHY_SETREG(0xFF, 0x00);

	//======================================================
	//(6) T12 USB30 APHY setting								[NEW]
	//======================================================
	USB3PHY_SETREG(0xFF ,0x00);
	USB3PHY_SETREG(0x149,0xFF);
	USB3PHY_SETREG(0x145,0xFF);
	USB3PHY_SETREG(0xFF ,0x01);
	USB3PHY_SETREG(0x168,0xEF);
	USB3PHY_SETREG(0x108,0x02);
	USB3PHY_SETREG(0x1A0,0x5F);
	USB3PHY_SETREG(0x1A2,0xF4);
	USB3PHY_SETREG(0x15E,0xFF);
	USB3PHY_SETREG(0x22 ,0x00);
	USB3PHY_SETREG(0x16C,0x00);
	USB3PHY_SETREG(0x170,0x00);
	USB3PHY_SETREG(0x174,0xB9);
	USB3PHY_SETREG(0x23 ,0x13);
	USB3PHY_SETREG(0x1A5,0x2C);
	USB3PHY_SETREG(0x5A ,0x08);
	USB3PHY_SETREG(0xB9 ,0xC0);
	USB3PHY_SETREG(0xFF ,0x02);
	USB3PHY_SETREG(0x28 ,0xF7);
	USB3PHY_SETREG(0x3  ,0x21);
	USB3PHY_SETREG(0x2  ,0xEC);
	USB3PHY_SETREG(0x1E ,0x10);
	USB3PHY_SETREG(0x33 ,0xA0);
	USB3PHY_SETREG(0xA  ,0x0F);
	USB3PHY_SETREG(0xB  ,0x3B);
	USB3PHY_SETREG(0x1F ,0x7C);
	//USB3PHY_SETREG(0x2A ,0x90);
	USB3PHY_SETREG(0xFF ,0x00);

	/* asd_mode=1 (toggle rate) */
	USB3PHY_SETREG(0x198, 0x04);
	/* RX_ICTRL's offset = 0 */
	//USB3PHY_SETREG(0x1BF, 0x40);
	/* TX_AMP_CTL=2, TX_DEC_EM_CTL=8 */
	USB3PHY_SETREG(0x014, 0x8a);
	/* TX_LFPS_AMP_CTL = 1 */
	USB3PHY_SETREG(0x034, 0xfc);//default 0xfc
#if 0
	if ((tx_swing >= 0) && (tx_swing <= 3)) {
		u3_trim_swctrl = tx_swing;
		pr_info("u3_trim_swctrl = 0x%x\n",u3_trim_swctrl);

		temp = USB3PHY_GETREG(0x14);
		temp &= ~(0x3);
		temp |= (u3_trim_swctrl);
		pr_info("temp = 0x%x\n",temp);
		USB3PHY_SETREG(0x14 , temp);
	}
#endif
	/* PHY Power Mode Change ready reponse time. (3 is 1ms.)(4 is 1.3ms.) */
	//USB3PHY_SETREG(0x114, 0x0B);
	USB3PHY_SETREG(0x114, 0x04);
	//USB3PHY_SETREG(0x152, 0x2E);
	//USB3PHY_SETREG(0x153, 0x01);
	//USB3PHY_SETREG(0x1B0, 0xC0);
	//USB3PHY_SETREG(0x1B1, 0x91);
	//USB3PHY_SETREG(0x1B2, 0x00);
	//USB3PHY_SETREG(0x135, 0x88);
	//USB3PHY_SETREG(0x12A, 0x50);
	//USB3PHY_SETREG(0x1F0, 0x80);
	//USB3PHY_SETREG(0x1F5, 0x01|(u3_trim_icdr<<4));//0xB1
	//USB3PHY_SETREG(0x105, 0x01);
	//disconnect level to 0x3
	USB3PHY_SETREG(0x056, 0x74);
	udelay(2);

	//USB3PHY_SETREG(0x102, 0xFF);
	USB3PHY_SETREG(0x102, 0xF0);
	udelay(10);
	USB3PHY_SETREG(0x102, 0x00);
	udelay(300);

	/* disable_power_saving */
	//Checking this one
	USB3U2PHY_SETREG(0xE, 0x04);

	udelay(50);

	//USB3PHY_SETREG(0x103, 0x01);
	//udelay(100);

	//R45 Calibration
	USB3U2PHY_SETREG(0x51, 0x20);
	udelay(2);
	USB3U2PHY_SETREG(0x51, 0x00);
	udelay(2);

	if(1) {
		temp = USB3PHY_GETREG(0x011);
		temp |= 0x1;
		USB3PHY_SETREG(0x011, temp);

		temp = USB3PHY_GETREG(0x031);
		temp |= 0x1;
		USB3PHY_SETREG(0x031, temp);
	}
	USB3PHY_SETREG(0x029, 0x1);

	//Port2 ctrl_ls = 11
	USB3U2PHY_SETREG(0x9, 0xb0);
	//Port2 dout inverter
	USB3U2PHY_SETREG(0x12, 0x31);

	//if ((u2_tx_swing >= 0) && (u2_tx_swing <= 7)) {
	//	u2_trim_swctrl = u2_tx_swing;
	//}

	//Set u2 phy swing
	temp = USB3U2PHY_GETREG(0x6);
	temp &= ~(0x7 << 1);
	temp |= (u2_trim_swctrl << 0x1);
	USB3U2PHY_SETREG(0x6, temp);
}
#elif defined(CONFIG_NVT_IVOT_PLAT_NA51090)
static void usb3_u2u3phyinit(void)
{
	UINT8 u3_trim_rint_sel=8,u3_trim_swctrl=4, u3_trim_sqsel=4,u3_trim_icdr=0xB;
	UINT32 temp;
	u16 data=0;
	s32 result=0;

	if(fotg330_dbg)
		pr_info("usb3_u2u3phyinit\r\n");

	USB3PHY_SETREG(0x0D, 0x01);
	//*(UINT32*) 0xF05F2680 = 0x40;
	USB3PHY_SETREG(0x1A0, 0x40);
	{
		// UINT16 data=0;
		// INT32 result;

		// result= efuse_readParamOps(EFUSE_USBC_TRIM_DATA, &data);
		// if(result >= 0) {
			// u3_trim_swctrl = data&0x7;
			// u3_trim_sqsel  = (data>>3)&0x7;
			// u3_trim_rint_sel = (data>>6)&0x1F;
		// }
		result = efuse_readParamOps(EFUSE_USB2_TRIM_DATA, &data);
		if (result == 0) {
			u3_trim_rint_sel = data & 0x1F;
			u3_trim_icdr     = (data>>5)&0xF;
		}

		//usb3_validateTrim();
		USB3PHY_SETREG(0x1A0, 0x40+u3_trim_rint_sel);
		USB3PHY_SETREG(0x1A3, 0x60);

		temp = USB3U2PHY_GETREG(0x06);
		temp &= ~(0x7<<1);
		temp |= (u3_trim_swctrl<<1);
		USB3U2PHY_SETREG(0x06, temp);

		temp = USB3U2PHY_GETREG(0x05);
		temp &= ~(0x7<<2);
		temp |= (u3_trim_sqsel<<2);
		USB3U2PHY_SETREG(0x05, temp);
	}

	/* asd_mode=1 (toggle rate) */
	USB3PHY_SETREG(0x198, 0x04);
	/* RX_ICTRL's offset = 0 */
	USB3PHY_SETREG(0x1BF, 0x40);
	/* TX_AMP_CTL=2, TX_DEC_EM_CTL=8 */
	USB3PHY_SETREG(0x014, 0x8a);
	/* TX_LFPS_AMP_CTL = 1 */
	USB3PHY_SETREG(0x034, 0xfc);//default 0xfc
	/* PHY Power Mode Change ready reponse time. (3 is 1ms.)(4 is 1.3ms.) */

	USB3PHY_SETREG(0x114, 0x0B);
	USB3PHY_SETREG(0x152, 0x2E);
	USB3PHY_SETREG(0x153, 0x01);
	USB3PHY_SETREG(0x1B0, 0xC0);
	USB3PHY_SETREG(0x1B1, 0x91);
	USB3PHY_SETREG(0x1B2, 0x00);
	USB3PHY_SETREG(0x135, 0x88);
	USB3PHY_SETREG(0x12A, 0x50);
	USB3PHY_SETREG(0x1F0, 0x80);
	USB3PHY_SETREG(0x1F5, 0x01|(u3_trim_icdr<<4));//0xB1
	USB3PHY_SETREG(0x105, 0x01);
	USB3PHY_SETREG(0x056, 0x74);//disconnect level to 0x3
	udelay(2);

	USB3PHY_SETREG(0x102, 0x20);
	udelay(10);
	USB3PHY_SETREG(0x102, 0x00);
	udelay(300);

	/* disable_power_saving */
	USB3U2PHY_SETREG(0xE, 0x04);

	udelay(50);

	USB3PHY_SETREG(0x103, 0x01);
	udelay(100);

	USB3U2PHY_SETREG(0x51, 0x20);
	udelay(2);
	USB3U2PHY_SETREG(0x51, 0x00);
	udelay(2);

	if(1) {
		temp = USB3PHY_GETREG(0x011);
		temp |= 0x1;
		USB3PHY_SETREG(0x011, temp);

		temp = USB3PHY_GETREG(0x031);
		temp |= 0x1;
		USB3PHY_SETREG(0x031, temp);
	}
	USB3PHY_SETREG(0x029, 0x1);
}
#elif defined(CONFIG_NVT_IVOT_PLAT_NS02201)
static void usb3_u2u3phyinit(void)
{
	//UINT8 u3_trim_rint_sel=8,u3_trim_swctrl=4, u3_trim_sqsel=4,u3_trim_icdr=0xB;
	u8 USB3_Internal_12K_resistor=8;
	u8 USB3_2_12K_resistor=8;
	u32 temp;
	u8 USB_TX_TRIM = 0x6;
	u8 USB_RX_TRIM = 0xC;
	u8 u2_trim_swctrl = 6;
	//u8 u3_trim_swctrl;

	if(fotg330_dbg)
		pr_info("690 usb3_u2u3phyinit\r\n");

	// Bank 0
	USB3PHY_SETREG(0xFF, 0x00);

	temp = USB3PHY_GETREG(0x97);
	temp |= 0x3;
	USB3PHY_SETREG(0x97 , temp);

#if 1//USB3_CONFIG_TRIM
	{
		s32       trim;
		s32       addr;
		u16      value;

		if(u3_channel == 1) {
			trim = efuse_readParamOps(EFUSE_USB3_2_SATA2_TRIM_DATA, &value);
			addr = 0x05;
		} else {
			trim = efuse_readParamOps(EFUSE_USB3_1_TRIM_DATA, &value);
			addr = 0x2A;
		}

		if (trim == 0) {
			//12K resistor 1<= x <= 20(0x14)
			//usb3dbg_evt(("  USB(SATA) Trim data read success addr[0x%02x] = 0x%04x\r\n", (int)addr, (int)value));
			//usb3dbg_evt(("  12K_resistor[4..0] valid from 1(0x1) ~ 20(0x14)\r\n"));
			USB3_Internal_12K_resistor = value & 0x1F;
			USB_TX_TRIM = ((value >> 5)&0xF);
			USB_RX_TRIM = ((value >> 9)&0xF);

			if (USB3_Internal_12K_resistor > 0x14) {
				pr_err("12K_resistor Trim data error [0x%02x]=0x%04x > 20(0x14)\r\n", (int)addr, (int)USB3_Internal_12K_resistor);
				USB3_Internal_12K_resistor=8;
			} else if (USB3_Internal_12K_resistor < 0x1) {
				pr_err("12K_resistor Trim data error [0x%02x]=0x%04x < 1(0x1)\r\n", (int)addr, (int)USB3_Internal_12K_resistor);
				USB3_Internal_12K_resistor=8;
			} else {
#if USB_TRIM_INFO
				pr_info("  *12K_resistor Trim data range success 0x1 <= [0x%04x] <= 0x14\r\n", (int)USB3_Internal_12K_resistor);
#endif
			}

			//TX term bit[8..5] 0x2 <= x <= 0xE
			if (USB_TX_TRIM > 0xE) {
				pr_err("USB(SATA) TX Trim data error [0x%02x]=0x%04x > 0xE\r\n", (int)addr, (int)USB_TX_TRIM);
				USB_TX_TRIM = 0x6;
			} else if (USB_TX_TRIM < 0x2) {
				pr_err("USB(SATA) TX Trim data error [0x%02x]=0x%04x < 0x2\r\n", (int)addr, (int)USB_TX_TRIM);
				USB_TX_TRIM = 0x6;
			} else {
#if USB_TRIM_INFO
				pr_info("  *USB(SATA) TX Trim data range success 0x2 <= [0x%04x] <= 0xE\r\n", (int)USB_TX_TRIM);
#endif
			}
			//RX term bit[12..9] 0x8 <= x <= 0xE
			if (USB_RX_TRIM > 0xE) {
				pr_err("USB(SATA) RX Trim data error [0x%02x]=0x%04x > 0xE\r\n", (int)addr, (int)USB_RX_TRIM);
				USB_RX_TRIM = 0xC;
			} else if (USB_RX_TRIM < 0x8) {
				pr_err("USB(SATA) RX Trim data error [0x%02x]=0x%04x < 0x8\r\n", (int)addr, (int)USB_RX_TRIM);
				USB_RX_TRIM = 0xC;
			} else {
#if USB_TRIM_INFO
				pr_info("  *USB(SATA) RX Trim data range success 0x8 <= [0x%04x] <= 0xE\r\n", (int)USB_RX_TRIM);
#endif
			}
		} else {
			//pr_err("efuse trim : addr[%02x] = %d\r\n", (int)addr, (int)trim);
			pr_err("[0x%x] USB param default\r\n", (int)addr);
			//return;
		}
		//(1) select PHY internal rsel value 0x2684 bit 7
		temp = USB3PHY_GETREG(0x1A1);
		temp &= ~(0x80);
		USB3PHY_SETREG(0x1A1 , temp);
		//(2) R12K trim setting  if 0x2680[3..0] == 0 setting
		//temp = USB3PHY_GETREG(0x1A0);
		//if ((temp & 0xF) == 0) {
			USB3PHY_SETREG(0x1A0, 0x40+USB3_Internal_12K_resistor);
			USB3PHY_SETREG(0x1A3, 0x60);
		//}
		//======================================================
		//(3) USB20 swing & sqsel trim bits removed in INT98530
		//======================================================
		//temp = USB3U2PHY_GETREG(0x06);
		//temp &= ~(0x7<<1);
		//temp |= (u3_trim_swctrl<<1);
		//USB3U2PHY_SETREG(0x06, temp);

		//temp = USB3U2PHY_GETREG(0x05);
		//temp &= ~(0x7<<2);
		//temp |= (u3_trim_sqsel<<2);
		//USB3U2PHY_SETREG(0x05, temp);

		if(u3_channel == 1){
			trim = efuse_readParamOps(EFUSE_USB_RINT_TRIM_DATA, &value);
			addr = 0x39;

			if (trim == 0) {
				//12K resistor 1<= x <= 20(0x14)
#if USB_TRIM_INFO
				pr_err("  USB3 single portTrim data read success addr[0x%02x] = 0x%04x\r\n", (int)addr, (int)value);
				pr_err("  12K_resistor[4..0] valid from 1(0x1) ~ 20(0x14)\r\n");
#endif
				USB3_2_12K_resistor = value & 0x1F;

				if (USB3_2_12K_resistor > 0x14) {
					pr_err("12K_resistor Trim data error [0x%02x]=0x%04x > 20(0x14)\r\n", (int)addr, (int)USB3_2_12K_resistor);
					USB3_2_12K_resistor=8;
				} else if (USB3_2_12K_resistor < 0x1) {
					pr_err("12K_resistor Trim data error [0x%02x]=0x%04x < 1(0x1)\r\n", (int)addr, (int)USB3_2_12K_resistor);
					USB3_2_12K_resistor=8;
				} else {
#if USB_TRIM_INFO
					pr_err("  *12K_resistor Trim data range success 0x1 <= [0x%04x] <= 0x14\r\n", (int)USB3_2_12K_resistor);
#endif
				}
			} else {
				//pr_err("efuse trim failed : addr[%02x] error %d\r\n", (int)addr, (int)trim);
				pr_err("USB param default : addr[%02x] \r\n", (int)addr);
			}
#if 0
			USB3U2PHY_SETREG(0x00 , 0x88);
			USB3U2PHY_SETREG(0x04 , 0x08);
			USB3U2PHY_SETREG(0x37 , 0x6e);
			USB3U2PHY_SETREG(0x35 , 0x35);
			USB3U2PHY_SETREG(0x34 , 0xff);
			USB3U2PHY_SETREG(0x52 , 0x60+USB3_2_12K_resistor);
			USB3U2PHY_SETREG(0x50 , 0x13);
			udelay(60);
			USB3U2PHY_SETREG(0x50 , 0x17);
			udelay(60);
			temp = USB3U2PHY_GETREG(0x52);
			pr_err("12K_resistor Trim data [0xF0B11148]=0x%04x\r\n", (int)temp);
			//Rintcal enable (set 1 to enable , set 0 to reset)
			USB3U2PHY_SETREG(0x50 , 0x14);
			udelay(30);
#else
			USB3U2PHY_SETREG(0x51 , 0x20);
			USB3U2PHY_SETREG(0x50 , 0x30);
			USB3U2PHY_SETREG(0x52 , 0x60+USB3_2_12K_resistor);
			USB3U2PHY_SETREG(0x51 , 0x00);
#endif
			//Port0 VLDOSEL = 00
			USB3U2PHY_SETREG(0x00 , 0x00);
			//Port0 PLL_RG_LEVEL = 0
			USB3U2PHY_SETREG(0x54 , 0x00);
		}
	}
#else
	temp = USB3PHY_GETREG(0x1A1);
	temp &= ~(0x80);
	USB3U2PHY_SETREG(0x1A1 , temp);

		temp = USB3PHY_GETREG(0x1A0);
		if ((temp & 0xF) == 0) {
			USB3PHY_SETREG(0x1A0, 0x40+USB3_Internal_12K_resistor);
			USB3PHY_SETREG(0x1A3, 0x60);
		}
#endif
	//======================================================
	//(3) USB20 swing & sqsel trim bits removed in INT98530
	//======================================================
	//========================================================
	//add additonal setting for 0.85V
	//========================================================
	// Bank 2
	USB3PHY_SETREG(0xFF, 0x02);
	// bit[5:4]  = LDO_VLDO_SEL_DCO[1:0] ; 00 = 0.85v
	USB3PHY_SETREG(0x2A, 0x80);
	// Bank 0
	USB3PHY_SETREG(0xFF, 0x00);
	// bit[1:0]  = LDO_SEL_18[1:0] ; 00 = 0.85v
	USB3PHY_SETREG(0x64, 0x44);
	// bit[1:0] = TXPLL_CP[1:0] = 01
	USB3PHY_SETREG(0x65, 0X99);

	//========================================================
	//swing
	//========================================================
	// Bank 0
	USB3PHY_SETREG(0xFF, 0x00);
	// bit[5:4] = V_VCHSEL[1:0] = 11
	USB3PHY_SETREG(0x56, 0X74);
	// TX_AMP_CTL=2, TX_DEC_EM_CTL=8
	USB3PHY_SETREG(0x14, 0X8A);
	// bit[5:4] = TX_AMP_CTL_LFPS[1:0]
	//USB3PHY_SETREG(0x12, 0XE8);
	USB3PHY_SETREG(0x12, 0XE8);
	// bit3 : TX_DE_EN manual mode
	// bit[7:4]: TX_DEC_EM_CTL manual mode
	USB3PHY_SETREG(0x34, 0Xfc);

	//======================================================
	//(4) TX termination trim bits								[NEW]
	//======================================================
	// 0xF05F2040[7:4] = tx_term_trim_val[3:0]
	// Bank 0
	USB3PHY_SETREG(0xFF, 0x00);

	temp = USB3PHY_GETREG(0x10);
	temp &= ~(0xF0);
	temp |= (USB_TX_TRIM<<4);
	USB3PHY_SETREG(0x10 , temp);

	//======================================================
	//(5) RX termination trim bits								[NEW]
	//======================================================
	// Bank 2
	USB3PHY_SETREG(0xFF, 0x02);

	temp = USB3PHY_GETREG(0x0);
	temp &= ~(0xF);
	temp |= (USB_RX_TRIM);
	USB3PHY_SETREG(0x00 , temp);

	// Bank 0
	USB3PHY_SETREG(0xFF, 0x00);

	//======================================================
	//(6) T12 USB30 APHY setting								[NEW]
	//======================================================
	USB3PHY_SETREG(0xFF ,0x00);
	USB3PHY_SETREG(0x149,0xFF);
	USB3PHY_SETREG(0x145,0xFF);
	USB3PHY_SETREG(0xFF ,0x01);
	USB3PHY_SETREG(0x168,0xEF);
	USB3PHY_SETREG(0x108,0x02);
	USB3PHY_SETREG(0x1A0,0x5F);
	USB3PHY_SETREG(0x1A2,0xF4);
	USB3PHY_SETREG(0x15E,0xFF);
	USB3PHY_SETREG(0x22 ,0x00);
	USB3PHY_SETREG(0x16C,0x00);
	USB3PHY_SETREG(0x170,0x00);
	USB3PHY_SETREG(0x174,0xB9);
	USB3PHY_SETREG(0x23 ,0x13);
	USB3PHY_SETREG(0x1A5,0x2C);
	USB3PHY_SETREG(0x5A ,0x08);
	USB3PHY_SETREG(0xB9 ,0xC0);
	USB3PHY_SETREG(0xFF ,0x02);
	USB3PHY_SETREG(0x28 ,0xF7);
	USB3PHY_SETREG(0x3  ,0x21);
	USB3PHY_SETREG(0x2  ,0xEC);
	USB3PHY_SETREG(0x1E ,0x10);
	USB3PHY_SETREG(0x33 ,0xA0);
	USB3PHY_SETREG(0xA  ,0x0F);
	USB3PHY_SETREG(0xB  ,0x3B);
	USB3PHY_SETREG(0x1F ,0x7C);
	//USB3PHY_SETREG(0x2A ,0x90);
	USB3PHY_SETREG(0xFF ,0x00);

	/* asd_mode=1 (toggle rate) */
	USB3PHY_SETREG(0x198, 0x04);
	/* RX_ICTRL's offset = 0 */
	//USB3PHY_SETREG(0x1BF, 0x40);
	/* TX_AMP_CTL=2, TX_DEC_EM_CTL=8 */
	USB3PHY_SETREG(0x014, 0x8a);
	/* TX_LFPS_AMP_CTL = 1 */
	USB3PHY_SETREG(0x034, 0xfc);//default 0xfc
#if 0
	if ((tx_swing >= 0) && (tx_swing <= 3)) {
		u3_trim_swctrl = tx_swing;
		pr_info("u3_trim_swctrl = 0x%x\n",u3_trim_swctrl);

		temp = USB3PHY_GETREG(0x14);
		temp &= ~(0x3);
		temp |= (u3_trim_swctrl);
		pr_info("temp = 0x%x\n",temp);
		USB3PHY_SETREG(0x14 , temp);
	}
#endif
	/* PHY Power Mode Change ready reponse time. (3 is 1ms.)(4 is 1.3ms.) */
	//////////////////////////////////// suspend <-> resume begine
	//USB3PHY_SETREG(0x114, 0x0B);
	USB3PHY_SETREG(0x114, 0x04);
	// //bit[3:0] = u30p_ele2_dly_time[3:0]
	//   1 = 128us , 4 = 512us, default = 0x8
	USB3PHY_SETREG(0x27b, 0x51);

	//bit3 u30p_rxanaif_all_skp_recalib_en
	//0xf05f2504[0] = 1
	temp = USB3PHY_GETREG(0x141);
	temp |= 0x1;
	USB3PHY_SETREG(0x141, temp);

	//phystatus_tmr_en (timer enable)
	//(0xf05f2504[4] )
	//phystatus_period[2:0] , 1 = 1us
	//(0xf05f2504[7:5] )
	temp = USB3PHY_GETREG(0x141);
	temp |= 0x50;
	USB3PHY_SETREG(0x141, temp);

	//bit7 : u30p_rxanaif_ss_ready_mode=1
	temp = USB3PHY_GETREG(0x140);
	temp |= 0x80;
	USB3PHY_SETREG(0x140, temp);
	//////////////////////////////////// suspend <-> resume end
	//USB3PHY_SETREG(0x152, 0x2E);
	//USB3PHY_SETREG(0x153, 0x01);
	//USB3PHY_SETREG(0x1B0, 0xC0);
	//USB3PHY_SETREG(0x1B1, 0x91);
	//USB3PHY_SETREG(0x1B2, 0x00);
	//USB3PHY_SETREG(0x135, 0x88);
	//USB3PHY_SETREG(0x12A, 0x50);
	//USB3PHY_SETREG(0x1F0, 0x80);
	//USB3PHY_SETREG(0x1F5, 0x01|(u3_trim_icdr<<4));//0xB1
	//USB3PHY_SETREG(0x105, 0x01);
	//disconnect level to 0x3
	USB3PHY_SETREG(0x056, 0x74);
	udelay(2);

	//USB3PHY_SETREG(0x102, 0xFF);
	USB3PHY_SETREG(0x102, 0xF0);
	udelay(10);
	USB3PHY_SETREG(0x102, 0x00);
	udelay(300);

	/* disable_power_saving */
	//Checking this one
	//USB3U2PHY_SETREG(0xE, 0x04);

	//udelay(50);

	//USB3PHY_SETREG(0x103, 0x01);
	//udelay(100);

	//R45 Calibration
	USB3U2PHY_SETREG(0x51, 0x20);
	udelay(2);
	USB3U2PHY_SETREG(0x51, 0x00);
	udelay(2);

	if(1) {
		temp = USB3PHY_GETREG(0x011);
		temp |= 0x1;
		USB3PHY_SETREG(0x011, temp);

		temp = USB3PHY_GETREG(0x031);
		temp |= 0x1;
		USB3PHY_SETREG(0x031, temp);
	}
	USB3PHY_SETREG(0x029, 0x1);

	//======================================================
	// U2 best setting												[NEW]
	//======================================================
	//U3 vchsel = 11
	USB3PHY_SETREG(0x56, 0x74);
	if(u3_channel == 1) {
		USB3U2PHY_SETREG(0x5, 0xd3);
	}
	//Port2 ctrl_ls = 11
	USB3U2PHY_SETREG(0x9, 0xb0);
	//Port2 dout inverter
	USB3U2PHY_SETREG(0x12, 0x31);
	// tx swing  & de-emphasis swing  (690 add)
	USB3U2PHY_SETREG(0x6, 0x1c);

	//if ((u2_tx_swing >= 0) && (u2_tx_swing <= 7)) {
	//	u2_trim_swctrl = u2_tx_swing;
	//}

	//Set u2 phy swing
	temp = USB3U2PHY_GETREG(0x6);
	temp &= ~(0x7 << 1);
	temp |= (u2_trim_swctrl << 0x1);
	USB3U2PHY_SETREG(0x6, temp);
}
#elif defined(CONFIG_NVT_IVOT_PLAT_NS02302)
static void usb3_u2u3phyinit(void)
{
	u8 USB_SATA_Internal_12K_resistor = 8;
	u8 tx_term_trim_val = 6;
	u32 temp;
	u8 u3_trim_swctrl = 4;
	u8 u3_trim_sqsel = 4;

	if(fotg330_dbg)
		pr_info("usb3_u2u3phyinit\r\n");

	//======================================================
	// Get USB30 trim
	//======================================================
	#if 1
        {
		u16 data=0;
		u32 trim;
		u32 addr;

		trim = efuse_readParamOps(EFUSE_USBC_TRIM_DATA, &data);
		//trim = 1; // Be used while efuse api not ready
		addr = EFUSE_USBC_TRIM_DATA;

		if(trim == 0) {
			//12K resistor 1<= x <= 20(0x14)
			pr_info("  USB Trim data = 0x%04x\r\n", (int)data);
			USB_SATA_Internal_12K_resistor = data & 0x1F;
			tx_term_trim_val = ((data >> 5) & 0xF);

			//12K
			if (USB_SATA_Internal_12K_resistor > 0x14) {
				pr_info("12K_resistor Trim data error 0x%04x > 20(0x14)\r\n", (int)USB_SATA_Internal_12K_resistor);
				USB_SATA_Internal_12K_resistor = 8;
			} else if (USB_SATA_Internal_12K_resistor < 0x1) {
				pr_info("12K_resistor Trim data error 0x%04x < 1(0x1)\r\n", (int)USB_SATA_Internal_12K_resistor);
				USB_SATA_Internal_12K_resistor = 8;
			} else {
				pr_info("  *12K_resistor Trim data range success 0x1 <= [0x%04x] <= 0x14\r\n", (int)USB_SATA_Internal_12K_resistor);
			}

			//TX term bit[8..5] 0x2 <= x <= 0xE
			if (tx_term_trim_val > 0xE) {
				pr_info("USB(SATA) TX Trim data error 0x%04x > 0xE\r\n", (int)tx_term_trim_val);
				tx_term_trim_val = 6;
			} else if (tx_term_trim_val < 0x2) {
				pr_info("USB(SATA) TX Trim data error 0x%04x < 0x2\r\n", (int)tx_term_trim_val);
				tx_term_trim_val = 6;
			} else {
				pr_info("  *USB(SATA) TX Trim data range success 0x2 <= [0x%04x] <= 0xE\r\n", (int)tx_term_trim_val);
			}
			//RX term use auto calibration on 538
		}else {
			pr_err("[0x%x] data = NULL\n", addr);
		}
	}
	#endif //USB3_CONFIG_TRIM END

	//======================================================
	// USB30 12K & TX trim bits
	//======================================================

	temp = USB3PHY_GETREG(0x102);
	pr_info("U3 internal block reset(0x2408): 0x%d\n", temp);

	temp = USB3PHY_GETREG(0x115);
	pr_info("U3 RX/TX termination(0x2454): 0x%d\n", temp);

	// step1: 12K trim set
	USB3PHY_SETREG(0x1A3, 0x60);

	temp = USB3PHY_GETREG(0x1A1);
	temp &= ~(0x80);
	USB3PHY_SETREG(0x1A1 , temp);

	USB3PHY_SETREG(0x1A0, 0x40 + USB_SATA_Internal_12K_resistor);

	// step2: tx trim
	//bit[7:4] :TX_ZTX_CTL force mode
	USB3PHY_SETREG(0x30, 0xF0);
	//0x40[7:4] = tx_term_trim_val[3:0]
	temp = USB3PHY_GETREG(0x10);
	temp &= ~(0xF0);
	temp |= (tx_term_trim_val<<4);
	USB3PHY_SETREG(0x10 , temp);

	//======================================================
	// USB20 swing & sqsel trim bits
	//======================================================
	temp = USB3U2PHY_GETREG(0x06);
	temp &= ~(0x7<<1);
	temp |= (u3_trim_swctrl<<1);
	USB3U2PHY_SETREG(0x06, temp);

	temp = USB3U2PHY_GETREG(0x05);
	temp &= ~(0x7<<2);
	temp |= (u3_trim_sqsel<<2);
	USB3U2PHY_SETREG(0x05, temp);

	//R45 Calibration
	USB3U2PHY_SETREG(0x51, 0x20);
	udelay(2);
	USB3U2PHY_SETREG(0x51, 0x00);
	udelay(2);

	//======================================================
	// USB20 Best setting on NT98538
	//======================================================
	//device mode
	USB3PHY_SETREG(0x56, 0x74);
	USB3U2PHY_SETREG(0x12, 0x31); //RX clk inverse

	//======================================================
	// USB30 Best setting on NT98538
	//======================================================
	USB3PHY_SETREG(0x1BF, 0x40); //EOC RX_ICTRL's offset = 0
	USB3PHY_SETREG(0x12, 0x28);  //bit[5:4] : TX_AMP_CTL_LFPS[1:0]
	USB3PHY_SETREG(0x14, 0x8A);  //Follow 336 for TX CTS
	USB3PHY_SETREG(0x34, 0xFC);
	USB3PHY_SETREG(0x114, 0x04); //PHY_PWR_MODE_CHG_RDY_RSP = 1.3ms
	USB3PHY_SETREG(0x152, 0x2E); //LB of VBS TUNE FREQ CNT
	USB3PHY_SETREG(0x153, 0x01); //UB of VBS TUNE FREQ CNT
	USB3PHY_SETREG(0x1B0, 0xC0); //EOC 2-data sampler
	USB3PHY_SETREG(0x1B1, 0x91); //LB of EOC THR
	USB3PHY_SETREG(0x1B2, 0x00); //UB of EOC THR
	USB3PHY_SETREG(0x135, 0x88); //SET VGA = FORCE MODE
	USB3PHY_SETREG(0x12A, 0x50); //SET VGA = 5
	USB3PHY_SETREG(0x105, 0x0); //Remove RX_ZRX OFS
	USB3PHY_SETREG(0x27B, 0x51);
	USB3PHY_SETREG(0x141, 0x51);
	USB3PHY_SETREG(0x140, 0x80);
	USB3PHY_SETREG(0x56, 0x74);  //bit[5:4] = BIAS_VCH_SEL[1:0] = 2'b11
	USB3PHY_SETREG(0x11, 0x11);  //bit[0] TX_PDB
	USB3PHY_SETREG(0x31, 0x1);   //bit[0] SM_TX_PDB
	USB3PHY_SETREG(0x13, 0x14);  //bit[2] TX_DET1_EN force value = 1
	USB3PHY_SETREG(0x102, 0x20); //ANAIF_PC_RST = 1
	udelay(1);
	USB3PHY_SETREG(0x102, 0x00); //ANAIF_PC_RST = 0
}
#endif
#endif //CONFIG_NVT_FPGA_EMULATION

static void init_phy(struct fusb300 *fusb300)
{
	u32 val = 0;

	// U3 Phy reset
	nvt_usb3dev_phy_reset();

	// U2 Phy reset
	// For combo-u2phy case(ex. 690/530/635), we need prevent reset u2phy clk 2nd time.
	// So we use 0x28[7] to check u2phy status
	// 0: u2 phy not reset
	// 1: u2 phy has been reset
	val = ioread32(fusb300->u3ctrl_reg + FUSB300_TOP_DBG_SEL_REG);
	if((val & (0x1 << 7)) == 0x00)
	{
		nvt_usb3dev_u2phy_reset();
		iowrite32((0x1 << 7), (fusb300->u3ctrl_reg + FUSB300_TOP_DBG_SEL_REG));
	}

	// PHY best setting
	#ifndef CONFIG_NVT_FPGA_EMULATION
	usb3_u2u3phyinit();
	#endif

}

static void init_controller(struct fusb300 *fusb300)
{
	u32 reg;
	u32 mask = 0;
	u32 val = 0;
	struct clk* pll14_clk = clk_get(NULL, "pll14");

	if(fotg330_dbg)
		pr_err("%s start\n", __FUNCTION__);

	/* STEP3: controller setting(PLL on, u3_clken, ssc..etc) */
	#if defined(CONFIG_NVT_IVOT_PLAT_NS02201)
	reg = ioread32(fusb300->u3cg_reg + 0x70);

	if (!IS_ERR(pll14_clk)) {
		nvt_spread_spectrum(pll14_clk, SSC_MULFAC, 1, SSC_PERIOD);
		//pr_err("%s: PLL14 set SSC\n", __func__);
	} else {
		pr_err("%s: PLL14 clk not found\n", __func__);
	}

	if(u3_channel == 1) {
		reg |= 0x80;
	} else {
		reg |= 0x40;
	}
	iowrite32(reg, fusb300->u3cg_reg + 0x70);
	#elif defined(CONFIG_NVT_IVOT_PLAT_NS02302)
	if (!IS_ERR(pll14_clk)) {
		nvt_spread_spectrum(pll14_clk, SSC_MULFAC, 1, SSC_PERIOD);
		pr_info("%s: PLL14 set SSC\n", __func__);
	} else {
		pr_info("%s: PLL14 clk not found\n", __func__);
	}
	#endif


	/* enable high-speed LPM */
	mask = val = FUSB300_HSCR_HS_LPM_PERMIT;
	reg = ioread32(fusb300->reg + FUSB300_OFFSET_HSCR);
	reg &= ~mask;
	reg &= ~(BESL_MASK << FUSB300_HSCR_HS_LPM_BESL_PERMIT_MIN);
	reg |= val;
	reg |= FUSB300_HSCR_CAP_RMWKUP;
	reg |= (BESL_MIN_DEFAULT << FUSB300_HSCR_HS_LPM_BESL_PERMIT_MIN);
	iowrite32(reg, fusb300->reg + FUSB300_OFFSET_HSCR);


	fusb300->remote_wkp = 0;

	/*set u1 u2 timmer*/
	fusb330_set_u2_timeout(fusb300, 0);
	fusb330_set_u1_timeout(fusb300, 0);

	/* disable all grp1 interrupt */
	iowrite32(0, fusb300->reg + FUSB300_OFFSET_IGER1);

	if(fotg330_dbg)
		pr_err("%s done\n", __FUNCTION__);
}

static void init_nvt_u3top(struct fusb300 *fusb300)
{
	u32 val = 0;

	val = ioread32(fusb300->u3ctrl_reg + FUSB300_TOP_MISC_2_REG);
	val &= ~(0x1 << FUSB300_TOP_BUF_CH_DIS);
	iowrite32(val, fusb300->u3ctrl_reg + FUSB300_TOP_MISC_2_REG);

	val = ioread32(fusb300->u3ctrl_reg + FUSB300_TOP_MISC_1_REG);
	val &= ~(0x1 << FUSB300_TOP_UTMI_PHY_SUSPEND);

	if(fusb300->get_power_gpio < ARCH_NR_GPIOS) {
		pr_info("usb vbusi polling mode, VBUSVALID not set yet!!\n");
		val |= (0x1 << FUSB300_TOP_USB_ID); // dev mode
	} else {
		val |= (0x1 << FUSB300_TOP_USB_ID); // dev mode
		val |= (0x1 << FUSB300_TOP_VBUS_VALID); // VBUSVALID
	}

	if (force_hs)
		val |= (0x1 << FUSB300_TOP_FORCE_HS);
	else
		val &= ~(0x1 << FUSB300_TOP_RXTERM_EN_MSK);

	val &= ~((0x7 << FUSB300_TOP_U3PHY_RST_CTL0) | (0x7 << FUSB300_TOP_U2PHY_RST_CTL0) | (0x1 << FUSB300_TOP_ELEC_BUF_MODE));
	val |= ((0x4 << FUSB300_TOP_U3PHY_RST_CTL0) | (0x4 << FUSB300_TOP_U2PHY_RST_CTL0));

	iowrite32(val, (fusb300->u3ctrl_reg + FUSB300_TOP_MISC_1_REG));

	if(fotg330_dbg)
		pr_info("0x14 setting = 0x%8x\n",val);

}

/*------------------------------------------------------------------------*/
static int fusb330_udc_start(struct usb_gadget *g,
		struct usb_gadget_driver *driver)
{
	struct fusb300 *fusb300 = to_fusb300(g);

	if(fotg330_dbg)
		pr_info("fusb330_udc_start\n");

	/* hook up the driver */
	driver->driver.bus = NULL;
	fusb300->driver = driver;

	//iowrite32(0x200, fusb300->reg + 0x328);

	/* enable all grp1 interrupt */
	//iowrite32(0xcfffff9f, fusb300->reg + FUSB300_OFFSET_IGER1);
	return 0;
}

static void fusb330_disable_unplug(struct fusb300 *fusb300)
{
	iowrite32(0x200, fusb300->reg + 0x328);
	/* enable all grp1 interrupt */
	iowrite32(0xcfffff9f, fusb300->reg + FUSB300_OFFSET_IGER1);
}

static void fusb330_enable_unplug(struct fusb300 *fusb300)
{
	iowrite32(0, fusb300->reg + FUSB300_OFFSET_IGER1);
	iowrite32(0x3FF, fusb300->reg + 0x328);
}


static int fusb330_udc_stop(struct usb_gadget *g)
{
	struct fusb300 *fusb300 = to_fusb300(g);

	if(fotg330_dbg)
		pr_info("fusb330_udc_stop\n");

	init_phy(fusb300);
	init_controller(fusb300);
	iowrite32(0x3FF, fusb300->reg + FUSB300_OFFSET_EFCS);
	fusb300->driver = NULL;
	return 0;
}
/*--------------------------------------------------------------------------*/

static int fusb330_udc_pullup(struct usb_gadget *_gadget, int is_active)
{
	struct fusb300 *fusb300 = to_fusb300(_gadget);

	if(is_active) {
		fusb330_disable_unplug(fusb300);
	} else {
		fusb330_enable_unplug(fusb300);
	}

	return 0;
}

static int fusb330_udc_wakeup(struct usb_gadget *g)
{
	struct fusb300 *fusb300 = to_fusb300(g);
	u32 value;

	if(fusb300->remote_wkp) {
		printk("signal remote_wkp\n");
		value = ioread32(fusb300->reg + FUSB300_OFFSET_HSCR);
		value |= FUSB300_HSCR_HS_GORMWKU;
		iowrite32(value, fusb300->reg + FUSB300_OFFSET_HSCR);
	}

	return 0;
}

static const struct usb_gadget_ops fusb330_gadget_ops = {
	.pullup		= fusb330_udc_pullup,
	.udc_start	= fusb330_udc_start,
	.udc_stop	= fusb330_udc_stop,
	.wakeup		= fusb330_udc_wakeup,
};

static int fusb330_remove(struct platform_device *pdev)
{
	struct fusb300 *fusb300 = platform_get_drvdata(pdev);
	int i;

	if(fusb300->gadget.sg_supported) {
		for (i = 1; i <= FUSB330_MAX_NUM_EP ; i++) {
			struct fusb300_ep *ep = fusb300->ep[i];
			/*free prd for sg_dma */
			dma_free_coherent(fusb300->gadget.dev.parent,sizeof(struct sg_prd) * FOTG330_PRD_NUM,
				ep->prd_pool, ep->prd_pool_dma);
			ep->prd_pool = NULL;
			ep->prd_pool_dma = 0;
		}
	}

	if( fusb300->get_power_gpio < ARCH_NR_GPIOS) {
		kthread_stop(fusb300->polling_thread_task);
		fusb300->polling_thread_task = NULL;
		fusb300->get_power_gpio = ARCH_NR_GPIOS;
	}

	if (fusb300->fsm_polling_thread_task) {
		/* disconnect thread */
		kthread_stop(fusb300->fsm_polling_thread_task);
		fusb300->fsm_polling_thread_task = NULL;
	}

	usb_del_gadget_udc(&fusb300->gadget);
	iounmap(fusb300->reg);
	free_irq(platform_get_irq(pdev, 0), fusb300);

	nvt_hwdump_sysfs_remove(&pdev->dev);

	fusb330_free_request(&fusb300->ep[0]->ep, fusb300->ep0_req);
	for (i = 0; i <= FUSB330_MAX_NUM_EP; i++)
		kfree(fusb300->ep[i]);
	kfree(fusb300);

	return 0;
}

#define FSM_SS_DIS 0
#define FSM_SS_U0 0xE
int disconnet_polling_thread(void *_fusb330)
{
	struct fusb300 *fusb300 = _fusb330;
	u8 connected = 0;
	u32 ss_fsm, pre_ss_fsm = 0;
	u32 vbus;

	sched_set_fifo(current);
	current->flags |= PF_NOFREEZE;

	set_current_state(TASK_INTERRUPTIBLE);

	while (!kthread_should_stop()) {
		if(fusb300->driver) {
			ss_fsm = (ioread32(fusb300->u3ctrl_reg + 0x1C) >> 16) & 0x1F;

			if ((ss_fsm == FSM_SS_DIS) && (pre_ss_fsm != FSM_SS_DIS) && (connected == 1)) {
				printk("disconnect event happen!\n");
				connected = 0;

				vbus = ioread32(fusb300->u3ctrl_reg + 0x10);
				vbus &= 0xFFFFFFFE;
				iowrite32(vbus, (fusb300->u3ctrl_reg+0x10));
				msleep(50);
				vbus |= 1;
				iowrite32(vbus, (fusb300->u3ctrl_reg+0x10));
				fusb300->driver->disconnect(&fusb300->gadget);
				force_rst = 1;

			} else if ((ss_fsm == FSM_SS_U0) && (connected == 0)) {
				printk("connect event happen!\n");
				connected = 1;
			} else {
				//
			}

			pre_ss_fsm = ss_fsm;

		} else {
			ss_fsm = 0;
			connected = 0;
		}
		schedule_timeout_interruptible(msecs_to_jiffies(100));
	}

	set_current_state(TASK_RUNNING);

	return 0;
}

int vbus_polling_thread(void *_fusb330)
{
	struct fusb300 *fusb300 = _fusb330;
	u32 plug_counter = 0;
	u32 unplug_counter = 0;
	u32 val;
	u8 connected = 0;

	sched_set_fifo(current);
	current->flags |= PF_NOFREEZE;

	set_current_state(TASK_INTERRUPTIBLE);

	while (!kthread_should_stop()) {
		if(fusb300->driver) {
			if(gpio_get_value(fusb300->get_power_gpio)) {
				unplug_counter++;
				plug_counter = 0;
			} else {
				plug_counter++;
				unplug_counter = 0;
			}

			if(plug_counter == 3 && connected == 0) {
				printk("connect!!..%u\n", gpio_get_value(fusb300->get_power_gpio));
				val = ioread32(fusb300->u3ctrl_reg + 0x10);
				val |= 0x1;
				iowrite32(val, (fusb300->u3ctrl_reg+0x10));
				connected = 1;
			}

			if(unplug_counter == 3 && connected == 1) {
				printk("disconnect!!..%u\n", gpio_get_value(fusb300->get_power_gpio));
				val = ioread32(fusb300->u3ctrl_reg + 0x10);
				val &= 0xFFFFFFFE;
				iowrite32(val, (fusb300->u3ctrl_reg+0x10));
				fusb300->driver->disconnect(&fusb300->gadget);
				connected = 0;
			}
		}else {
			unplug_counter = 0;
			plug_counter = 0;
			connected = 0;
		}
		schedule_timeout_interruptible(msecs_to_jiffies(100));
	}

	set_current_state(TASK_RUNNING);

	return 0;
}

#if 0
int dma_monitor_thread(void *_fusb330)
{
	struct fusb300 *fusb300 = _fusb330;
	int i;
	unsigned long flags;

	sched_set_fifo(current);
	current->flags |= PF_NOFREEZE;

	set_current_state(TASK_INTERRUPTIBLE);

	while (!kthread_should_stop()) {
		if(fusb300->driver) {
			for(i = 0; i < FUSB330_MAX_NUM_EP; i++) {
				if( !fusb300->ep[i]->disable &&
					fusb300->ep[i]->type == USB_ENDPOINT_XFER_BULK &&
					fusb300->ep[i]->ep.desc->bEndpointAddress& USB_DIR_IN &&
					fusb300->ep[i]->dma_ongoing) {
					if (time_after(jiffies, fusb300->ep[i]->dma_timeout_jiffies)) {
						printk("timeout!!\n");
						spin_lock_irqsave(&fusb300->ep[i]->fusb300->lock, flags);
						usb3dev_clear_fifo(fusb300->ep[i]);
						fusb300_idma_finished(fusb300->ep[i], -ECONNRESET);
						spin_unlock_irqrestore(&fusb300->ep[i]->fusb300->lock, flags);
					}
				}
			}
		}
		schedule_timeout_interruptible(msecs_to_jiffies(500));
	}

	set_current_state(TASK_RUNNING);

	return 0;
}
#endif

static int fusb330_probe(struct platform_device *pdev)
{
	struct device_node *dn = pdev->dev.of_node;
	struct resource *res, *ires;
	void __iomem *reg = NULL;
	struct fusb300 *fusb300 = NULL;
	struct fusb300_ep *_ep[FUSB330_MAX_NUM_EP + 1];
	int ret = 0;
	int i;
	//u32 force_hs = 0;

	if(fotg330_dbg)
		pr_info("fusb330_probe.\n");

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!res) {
		ret = -ENODEV;
		pr_err("platform_get_resource error.\n");
		goto clean_up;
	}

	ires = platform_get_resource(pdev, IORESOURCE_IRQ, 0);
	if (!ires) {
		ret = -ENODEV;
		dev_err(&pdev->dev,
			"platform_get_resource IORESOURCE_IRQ error.\n");
		goto clean_up;
	}

	reg = ioremap(res->start, resource_size(res));
	if (reg == NULL) {
		ret = -ENOMEM;
		pr_err("ioremap error.\n");
		goto clean_up;
	}

	if(fotg330_dbg)
		pr_info("res->start = 0x%llx\n", res->start);


	if(of_property_read_u32(pdev->dev.of_node, "force-hs", &force_hs)) {
	} else {
		if(force_hs)
			pr_info("[FROM DTSI] force_hs = 0x%x\n", force_hs);
	}

	if (of_property_read_u32(pdev->dev.of_node, "channel", &u3_channel)) {
		pr_info("channel not set. \n");
	} else {
		if(u3_channel)
			pr_info("[FROM DTSI] u3_channel = 0x%x\n", u3_channel);
	}

	/* initialize udc */
	fusb300 = kzalloc(sizeof(struct fusb300), GFP_KERNEL);
	if (fusb300 == NULL) {
		ret = -ENOMEM;
		goto clean_up;
	}

	if (of_property_read_u32(pdev->dev.of_node, "vbus_gpio", &fusb300->get_power_gpio)) {
		fusb300->get_power_gpio = ARCH_NR_GPIOS;
	} else {
		gpio_direction_input(fusb300->get_power_gpio);
	}

	if (of_property_read_u32(pdev->dev.of_node, "dis_det", &fusb300->disconnect_det)) {
		pr_info("disconnect_det thread not open!!!. \n");
	} else {
		pr_info("disconnect_det open. \n");
	}

	if (of_property_read_u32(pdev->dev.of_node, "sg_support", &i)) {
		fusb300->gadget.sg_supported = 0;
	} else {
		pr_info("sg_support=%u\n", i);
		fusb300->gadget.sg_supported = i;
	}

	gfusb330 = fusb300;

	for (i = 0; i <= FUSB330_MAX_NUM_EP; i++) {
		_ep[i] = kzalloc(sizeof(struct fusb300_ep), GFP_KERNEL);
		if (_ep[i] == NULL) {
			ret = -ENOMEM;
			goto clean_up;
		}
		fusb300->ep[i] = _ep[i];
	}

	spin_lock_init(&fusb300->lock);

	platform_set_drvdata(pdev, fusb300);

	fusb300->gadget.ops = &fusb330_gadget_ops;

	fusb300->gadget.max_speed = USB_SPEED_SUPER;
	fusb300->gadget.name = udc_name;
	fusb300->reg = reg;

	ret = request_irq(ires->start, fusb330_irq, IRQF_SHARED,
			  udc_name, fusb300);
	if (ret < 0) {
		pr_err("request_irq error (%d)\n", ret);
		goto clean_up;
	}

	if(fotg330_dbg)
		pr_info("IRQ ires->start: 0x%lld\n", ires->start);


	fusb300->u3ctrl_reg = of_iomap(dn, 1);

	if (!fusb300->u3ctrl_reg) {
		pr_info("u3ctrl_reg  ioremap failed");
		ret = -ENOMEM;
		goto clean_map_ctrl;
	} else {
		//pr_info("get the u3ctrl_reg addr: 0x%px\n", fusb300->u3ctrl_reg);
	}

	fusb300->u3_utmi_phy_reg = of_iomap(dn, 2);
	if (!fusb300->u3_utmi_phy_reg) {
		pr_info("u3_utmi_phy_reg ioremap failed");
		ret = -ENOMEM;
		goto clean_map_utmi_phy;
	}  else {
		//pr_info("get the u3 utmi addr: 0x%px\n", fusb300->u3_utmi_phy_reg);
	}

	fusb300->u3phy_reg = of_iomap(dn, 3);
	if (!fusb300->u3phy_reg) {
		pr_info("u3phy_reg ioremap failed");
		ret = -ENOMEM;
		goto clean_map_phy;
	}  else {
		//pr_info("get the u3phy_reg addr: 0x%px\n", fusb300->u3phy_reg);
	}
#if defined(CONFIG_NVT_IVOT_PLAT_NS02201) || defined(CONFIG_NVT_IVOT_PLAT_NS02302)|| defined(CONFIG_NVT_IVOT_PLAT_NA51090)
	fusb300->u3cg_reg = of_iomap(dn, 4);
	if (!fusb300->u3cg_reg) {
		pr_info("u3cg_reg ioremap failed");
		ret = -ENOMEM;
		goto clean_map_cg;
	}  else {
		pr_info("get the u3cg_reg addr: 0x%px\n", fusb300->u3phy_reg);
	}
#endif

	INIT_LIST_HEAD(&fusb300->gadget.ep_list);

	for (i = 0; i <= FUSB330_MAX_NUM_EP ; i++) {
		struct fusb300_ep *ep = fusb300->ep[i];

		if (i != 0) {
			INIT_LIST_HEAD(&fusb300->ep[i]->ep.ep_list);
			list_add_tail(&fusb300->ep[i]->ep.ep_list,
				     &fusb300->gadget.ep_list);
		}
		ep->fusb300 = fusb300;
		INIT_LIST_HEAD(&ep->queue);
		ep->ep.name = fusb330_ep_name[i];
		ep->ep.ops = &fusb330_ep_ops;
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
	usb_ep_set_maxpacket_limit(&fusb300->ep[0]->ep, HS_CTL_MAX_PACKET_SIZE);
	fusb300->ep[0]->epnum = 0;
	fusb300->gadget.ep0 = &fusb300->ep[0]->ep;
	INIT_LIST_HEAD(&fusb300->gadget.ep0->ep_list);

	fusb300->ep0_req = fusb330_alloc_request(&fusb300->ep[0]->ep,
				GFP_KERNEL);
	if (fusb300->ep0_req == NULL) {
		ret = -ENOMEM;
		goto clean_up3;
	}

	/**************************************************************/
	/************************* HW initial start *******************/
	/**************************************************************/
	//step1: PHY reset & PHY init best setting
	init_phy(fusb300);

	//step2: Controller init
	{
		struct clk       *clk;

		clk = clk_get(&pdev->dev, dev_name(&pdev->dev));
		if (!IS_ERR(clk)) {
			pr_info("%s:  clk_en & reset\n", dev_name(&pdev->dev));
			clk_prepare_enable(clk);
			clk_disable_unprepare(clk);
			clk_prepare_enable(clk);
		} else
			pr_err("%s not found\n", dev_name(&pdev->dev));

	}

	init_controller(fusb300);

	//step3: U3 top init
	init_nvt_u3top(fusb300);

	/**************************************************************/
	/************************* HW initial end *********************/
	/**************************************************************/

	ret = usb_add_gadget_udc(&pdev->dev, &fusb300->gadget);
	if (ret)
		goto err_add_udc;

	if(fusb300->get_power_gpio < ARCH_NR_GPIOS) {
		fusb300->polling_thread_task = kthread_create(vbus_polling_thread, fusb300, "vbus-polling-thread");
		wake_up_process(fusb300->polling_thread_task);
	}

	if (fusb300->disconnect_det) {
		pr_info("fsm_polling_thread_task open\n");
		fusb300->fsm_polling_thread_task =
			kthread_create(disconnet_polling_thread, fusb300, "disconnect-process");
		wake_up_process(fusb300->fsm_polling_thread_task);
	}

	ret = nvt_hwdump_sysfs_init(&pdev->dev);
	if (ret < 0) {
		dev_err(&pdev->dev, "create sysfs failed!\n");
	} else {
		pr_info("nvt_hwdump_sysfs_init pass\n");
	}

#if 0
	//prepare for dma monitor
	if(dma_int_mode) {
		pr_info("dma_monitor open\n");
		fusb300->dma_monitor =
			kthread_create(dma_monitor_thread, fusb300, "dma_monitor");
		wake_up_process(fusb300->dma_monitor);
	}
#endif

	if(fusb300->gadget.sg_supported) {
		for (i = 1; i <= FUSB330_MAX_NUM_EP ; i++) {
			struct fusb300_ep *ep = fusb300->ep[i];

			/*allocate prd for sg_dma */
			ep->prd_pool = dma_alloc_coherent(fusb300->gadget.dev.parent,
				sizeof(struct sg_prd) * FOTG330_PRD_NUM,
				&ep->prd_pool_dma, GFP_KERNEL);

			if (!ep->prd_pool) {
				pr_info("allocate SG_DMA pool fail!, force to disable sg_supported\n");
				fusb300->gadget.sg_supported = 0;
				break;
			}
		}
	}

	dev_info(&pdev->dev, "version %s\n", DRIVER_VERSION);

	return 0;

err_add_udc:
	fusb330_free_request(&fusb300->ep[0]->ep, fusb300->ep0_req);

clean_up3:
	free_irq(ires->start, fusb300);
#if defined(CONFIG_NVT_IVOT_PLAT_NS02201) || defined(CONFIG_NVT_IVOT_PLAT_NS02302)|| defined(CONFIG_NVT_IVOT_PLAT_NA51090)
clean_map_cg:
	if (fusb300->u3cg_reg)
		iounmap(fusb300->u3cg_reg);
#endif
clean_map_phy:
	if (fusb300->u3phy_reg)
		iounmap(fusb300->u3phy_reg);
clean_map_utmi_phy:
	if (fusb300->u3_utmi_phy_reg)
		iounmap(fusb300->u3_utmi_phy_reg);
clean_map_ctrl:
	if (fusb300->u3ctrl_reg)
		iounmap(fusb300->u3ctrl_reg);

clean_up:
	if (fusb300) {
		if (fusb300->ep0_req)
			fusb330_free_request(&fusb300->ep[0]->ep,
				fusb300->ep0_req);
		for (i = 0; i <= FUSB330_MAX_NUM_EP; i++)
			kfree(fusb300->ep[i]);
		kfree(fusb300);
	}
	if (reg)
		iounmap(reg);


	return ret;
}



#ifdef CONFIG_PM
#if defined(CONFIG_NVT_IVOT_PLAT_NA51102) || defined(CONFIG_NVT_IVOT_PLAT_NS02302) || defined(CONFIG_NVT_IVOT_PLAT_NS02201)
static void fotg330_fifo_clr(struct fusb300 *fusb300)
{
	u32 i;
	u32 reg;

	for(i=1;i<=FUSB330_MAX_NUM_EP;i++)
	{
		//printk(KERN_INFO "========[EP%d]===========  \n",i);
		reg = ioread32(fusb300->reg + FUSB300_OFFSET_EPFFR(i));
		reg = reg|(FUSB300_FFR_RST);
		iowrite32(reg, fusb300->reg + FUSB300_OFFSET_EPFFR(i));
	}
}

static void fotg330_abort(struct fusb300 *fusb300)
{
	//u32 reg;

	//printk("fotg330_abort \r\n");
	//To do
	//reg = ioread32(fusb300->reg + FUSB300_OFFSET_DMAEPMR);
	//reg &= ~(1<<EPn);
	//iowrite32(reg, fusb300->reg + FUSB300_OFFSET_DMAEPMR);

	fotg330_fifo_clr(fusb300);

	//reg = ioread32(fusb300->reg + FUSB300_OFFSET_DMAEPMR);
	//reg  |= (1<<EPn);;
	//iowrite32(reg, fusb300->reg + FUSB300_OFFSET_DMAEPMR);

	//printk("fotg330_abort done!\r\n");

}

static void stop_udc(struct fusb300 *fusb300)
{
	spin_lock(&fusb300->lock);
	//printk("stop_udc \r\n");

	/* FIFO clear */
	fotg330_abort(fusb300);

	/* Disconnect gadget driver */
	if (fusb300->driver) {
		fusb330_enable_unplug(fusb300);
		spin_unlock(&fusb300->lock);
		fusb300->driver->disconnect(&fusb300->gadget);
		spin_lock(&fusb300->lock);
	}

	spin_unlock(&fusb300->lock);
	//printk("stop_udc done \r\n");
}


static void fotg330_stop_controller(struct fusb300 *fusb300)
{
	//To do
}

static void fotg330_restart_controller(struct fusb300 *fusb300)
{
	if(fotg330_dbg)
		printk("fotg330_restart_controller !!\r\n");

	init_phy(fusb300);
	init_controller(fusb300);
	init_nvt_u3top(fusb300);

	if(fotg330_dbg)
		printk("fotg330_restart_controller done !!\r\n");
}

static int nvt_fotg330_suspend(struct device *dev)
{
	struct platform_device *pdev = to_platform_device(dev);
	struct fusb300 *fusb300 = platform_get_drvdata(pdev);
	int irq;

	// STEP1: STOP UDC fotg330
	stop_udc(fusb300);

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
	fotg330_stop_controller(fusb300);

	printk("nvt_fotg330_suspend done\r\n");
	return 0;
}

static int nvt_fotg330_resume(struct device *dev)
{
	struct platform_device *pdev = to_platform_device(dev);
	struct fusb300 *fusb300 = platform_get_drvdata(pdev);
	int irq;

	//printk("nvt_fotg330_resume \r\n");

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

	fotg330_restart_controller(fusb300);

	if (fusb300->driver) {
		printk("fusb330_disable_unplug !!\r\n");
		fusb330_disable_unplug(fusb300);
	}

	enable_irq(irq);

	//printk("nvt_fotg330_resume done\r\n");
	return 0;
}
#else
static int nvt_fotg330_suspend(struct device *dev)
{
	return 0;
}

static int nvt_fotg330_resume(struct device *dev)
{
	return 0;
}

#endif

#else
static int nvt_fotg330_suspend(struct device *dev)
{
	return 0;
}

static int nvt_fotg330_resume(struct device *dev)
{
	return 0;
}

#endif

static const struct dev_pm_ops nvt_fotg330_pm_ops = {
        .suspend = nvt_fotg330_suspend,
        .resume  = nvt_fotg330_resume,
};



#ifdef CONFIG_OF
static const struct of_device_id of_fotg330_match[] = {
	{
		.compatible = "nvt,fotg330_udc"
	},

	{ },
};
MODULE_DEVICE_TABLE(of, of_fotg330_match);
#endif

static struct platform_driver fusb330_driver = {
	.probe = fusb330_probe,
	.remove =	fusb330_remove,
	.driver		= {
		.name =	(char *) udc_name,
#ifdef CONFIG_PM
		.pm = &nvt_fotg330_pm_ops,
#endif

#ifdef CONFIG_OF
		.of_match_table = of_match_ptr(of_fotg330_match),
#endif
	},
};


module_platform_driver_probe(fusb330_driver, fusb330_probe);
