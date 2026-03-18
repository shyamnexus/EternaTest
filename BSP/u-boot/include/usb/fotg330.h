/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Faraday USB 3.0 OTG Controller
 *
 * (C) Copyright 2010 Faraday Technology
 * Dante Su <dantesu@faraday-tech.com>
 */

#ifndef _FOTG330_H
#define _FOTG330_H

#include <linux/kernel.h>

#define FUSB300_OFFSET_GCR		0x00
#define FUSB300_OFFSET_GTM		0x04
#define FUSB300_OFFSET_DAR		0x08
#define FUSB300_OFFSET_CSR		0x0C
#define FUSB300_OFFSET_CXPORT		0x10
#define FUSB300_OFFSET_EPSET0(n)	(0x20 + (n - 1) * 0x30)
#define FUSB300_OFFSET_EPSET1(n)	(0x24 + (n - 1) * 0x30)
#define FUSB300_OFFSET_EPSET2(n)	(0x28 + (n - 1) * 0x30)
#define FUSB300_OFFSET_EPFFR(n)		(0x2c + (n - 1) * 0x30)
#define FUSB300_OFFSET_EPSTRID(n)	(0x40 + (n - 1) * 0x30)
#define FUSB300_OFFSET_HSPTM		0x300
#define FUSB300_OFFSET_HSCR		0x304
#define FUSB300_OFFSET_SSCR0		0x308
#define FUSB300_OFFSET_SSCR1		0x30C
#define FUSB300_OFFSET_TT		0x310
#define FUSB300_OFFSET_DEVNOTF		0x314
#define FUSB300_OFFSET_DNC1		0x318
#define FUSB300_OFFSET_CS		0x31C
#define FUSB300_OFFSET_SOF		0x324
#define FUSB300_OFFSET_EFCS		0x328
#define FUSB300_OFFSET_IGR0		0x400
#define FUSB300_OFFSET_IGR1		0x404
#define FUSB300_OFFSET_IGR2		0x408
#define FUSB300_OFFSET_IGR3		0x40C
#define FUSB300_OFFSET_IGR4		0x410
#define FUSB300_OFFSET_IGR5		0x414
#define FUSB300_OFFSET_IGER0		0x420
#define FUSB300_OFFSET_IGER1		0x424
#define FUSB300_OFFSET_IGER2		0x428
#define FUSB300_OFFSET_IGER3		0x42C
#define FUSB300_OFFSET_IGER4		0x430
#define FUSB300_OFFSET_IGER5		0x434
#define FUSB300_OFFSET_DMAHMER		0x500
#define FUSB300_OFFSET_EPPRDRDY		0x504
#define FUSB300_OFFSET_DMAEPMR		0x508
#define FUSB300_OFFSET_DMAENR		0x50C
#define FUSB300_OFFSET_DMAAPR		0x510
#define FUSB300_OFFSET_AHBCR		0x514
#define FUSB300_OFFSET_EPPRD_W0(n)	(0x520 + (n - 1) * 0x10)
#define FUSB300_OFFSET_EPPRD_W1(n)	(0x524 + (n - 1) * 0x10)
#define FUSB300_OFFSET_EPPRD_W2(n)	(0x528 + (n - 1) * 0x10)
#define FUSB300_OFFSET_EPRD_PTR(n)	(0x52C + (n - 1) * 0x10)
#define FUSB300_OFFSET_BUFDBG_START	0x800
#define FUSB300_OFFSET_BUFDBG_END	0xBFC
#define FUSB300_OFFSET_EPPORT(n)	(0x1010 + (n - 1) * 0x10)




/*
 * *Interrupt Group 0 Register (offset = 400H)
 * */
#define FUSB300_IGR0_EP15_PRD_INT	(1 << 31)
#define FUSB300_IGR0_EP14_PRD_INT	(1 << 30)
#define FUSB300_IGR0_EP13_PRD_INT	(1 << 29)
#define FUSB300_IGR0_EP12_PRD_INT	(1 << 28)
#define FUSB300_IGR0_EP11_PRD_INT	(1 << 27)
#define FUSB300_IGR0_EP10_PRD_INT	(1 << 26)
#define FUSB300_IGR0_EP9_PRD_INT	(1 << 25)
#define FUSB300_IGR0_EP8_PRD_INT	(1 << 24)
#define FUSB300_IGR0_EP7_PRD_INT	(1 << 23)
#define FUSB300_IGR0_EP6_PRD_INT	(1 << 22)
#define FUSB300_IGR0_EP5_PRD_INT	(1 << 21)
#define FUSB300_IGR0_EP4_PRD_INT	(1 << 20)
#define FUSB300_IGR0_EP3_PRD_INT	(1 << 19)
#define FUSB300_IGR0_EP2_PRD_INT	(1 << 18)
#define FUSB300_IGR0_EP1_PRD_INT	(1 << 17)
#define FUSB300_IGR0_EPn_PRD_INT(n)	(1 << (n + 16))

#define FUSB300_IGR0_EP15_FIFO_INT	(1 << 15)
#define FUSB300_IGR0_EP14_FIFO_INT	(1 << 14)
#define FUSB300_IGR0_EP13_FIFO_INT	(1 << 13)
#define FUSB300_IGR0_EP12_FIFO_INT	(1 << 12)
#define FUSB300_IGR0_EP11_FIFO_INT	(1 << 11)
#define FUSB300_IGR0_EP10_FIFO_INT	(1 << 10)
#define FUSB300_IGR0_EP9_FIFO_INT	(1 << 9)
#define FUSB300_IGR0_EP8_FIFO_INT	(1 << 8)
#define FUSB300_IGR0_EP7_FIFO_INT	(1 << 7)
#define FUSB300_IGR0_EP6_FIFO_INT	(1 << 6)
#define FUSB300_IGR0_EP5_FIFO_INT	(1 << 5)
#define FUSB300_IGR0_EP4_FIFO_INT	(1 << 4)
#define FUSB300_IGR0_EP3_FIFO_INT	(1 << 3)
#define FUSB300_IGR0_EP2_FIFO_INT	(1 << 2)
#define FUSB300_IGR0_EP1_FIFO_INT	(1 << 1)
#define FUSB300_IGR0_EPn_FIFO_INT(n)	(1 << n)

/*
 * *Interrupt Group 1 Register (offset = 404H)
 * */
#define FUSB300_IGR1_INTGRP5		(1 << 31)
#define FUSB300_IGR1_VBUS_CHG_INT	(1 << 30)
#define FUSB300_IGR1_SYNF1_EMPTY_INT	(1 << 29)
#define FUSB300_IGR1_SYNF0_EMPTY_INT	(1 << 28)
#define FUSB300_IGR1_U3_EXIT_FAIL_INT	(1 << 27)
#define FUSB300_IGR1_U2_EXIT_FAIL_INT	(1 << 26)
#define FUSB300_IGR1_U1_EXIT_FAIL_INT	(1 << 25)
#define FUSB300_IGR1_U2_ENTRY_FAIL_INT	(1 << 24)
#define FUSB300_IGR1_U1_ENTRY_FAIL_INT	(1 << 23)
#define FUSB300_IGR1_U3_EXIT_INT	(1 << 22)
#define FUSB300_IGR1_U2_EXIT_INT	(1 << 21)
#define FUSB300_IGR1_U1_EXIT_INT	(1 << 20)
#define FUSB300_IGR1_U3_ENTRY_INT	(1 << 19)
#define FUSB300_IGR1_U2_ENTRY_INT	(1 << 18)
#define FUSB300_IGR1_U1_ENTRY_INT	(1 << 17)
#define FUSB300_IGR1_HOT_RST_INT	(1 << 16)
#define FUSB300_IGR1_WARM_RST_INT	(1 << 15)
#define FUSB300_IGR1_RESM_INT		(1 << 14)
#define FUSB300_IGR1_SUSP_INT		(1 << 13)
#define FUSB300_IGR1_HS_LPM_INT		(1 << 12)
#define FUSB300_IGR1_USBRST_INT		(1 << 11)
#define FUSB300_IGR1_DEV_MODE_CHG_INT	(1 << 9)
#define FUSB300_IGR1_CX_COMABT_INT	(1 << 8)
#define FUSB300_IGR1_CX_COMFAIL_INT	(1 << 7)
#define FUSB300_IGR1_CX_CMDEND_INT	(1 << 6)
#define FUSB300_IGR1_CX_OUT_INT		(1 << 5)
#define FUSB300_IGR1_CX_IN_INT		(1 << 4)
#define FUSB300_IGR1_CX_SETUP_INT	(1 << 3)
#define FUSB300_IGR1_INTGRP4		(1 << 2)
#define FUSB300_IGR1_INTGRP3		(1 << 1)
#define FUSB300_IGR1_INTGRP2		(1 << 0)

/*
 * *Interrupt Enable Group 0 Register (offset = 420H)
 * */
#define FUSB300_IGER0_EEP15_PRD_INT    (1 << 31)
#define FUSB300_IGER0_EEP14_PRD_INT    (1 << 30)
#define FUSB300_IGER0_EEP13_PRD_INT    (1 << 29)
#define FUSB300_IGER0_EEP12_PRD_INT    (1 << 28)
#define FUSB300_IGER0_EEP11_PRD_INT    (1 << 27)
#define FUSB300_IGER0_EEP10_PRD_INT    (1 << 26)
#define FUSB300_IGER0_EEP9_PRD_INT     (1 << 25)
#define FUSB300_IGER0_EP8_PRD_INT      (1 << 24)
#define FUSB300_IGER0_EEP7_PRD_INT     (1 << 23)
#define FUSB300_IGER0_EEP6_PRD_INT     (1 << 22)
#define FUSB300_IGER0_EEP5_PRD_INT     (1 << 21)
#define FUSB300_IGER0_EEP4_PRD_INT     (1 << 20)
#define FUSB300_IGER0_EEP3_PRD_INT     (1 << 19)
#define FUSB300_IGER0_EEP2_PRD_INT     (1 << 18)
#define FUSB300_IGER0_EEP1_PRD_INT     (1 << 17)
#define FUSB300_IGER0_EEPn_PRD_INT(n)  (1 << (n + 16))

#define FUSB300_IGER0_EEP15_FIFO_INT   (1 << 15)
#define FUSB300_IGER0_EEP14_FIFO_INT   (1 << 14)
#define FUSB300_IGER0_EEP13_FIFO_INT   (1 << 13)
#define FUSB300_IGER0_EEP12_FIFO_INT   (1 << 12)
#define FUSB300_IGER0_EEP11_FIFO_INT   (1 << 11)
#define FUSB300_IGER0_EEP10_FIFO_INT   (1 << 10)
#define FUSB300_IGER0_EEP9_FIFO_INT    (1 << 9)
#define FUSB300_IGER0_EEP8_FIFO_INT    (1 << 8)
#define FUSB300_IGER0_EEP7_FIFO_INT    (1 << 7)
#define FUSB300_IGER0_EEP6_FIFO_INT    (1 << 6)
#define FUSB300_IGER0_EEP5_FIFO_INT    (1 << 5)
#define FUSB300_IGER0_EEP4_FIFO_INT    (1 << 4)
#define FUSB300_IGER0_EEP3_FIFO_INT    (1 << 3)
#define FUSB300_IGER0_EEP2_FIFO_INT    (1 << 2)
#define FUSB300_IGER0_EEP1_FIFO_INT    (1 << 1)
#define FUSB300_IGER0_EEPn_FIFO_INT(n) (1 << n)


/*
 * *Interrupt Enable Group 1 Register (offset = 424H)
 * */
#define FUSB300_IGER1_EINT_GRP5		(1 << 31)
#define FUSB300_IGER1_VBUS_CHG_INT	(1 << 30)
#define FUSB300_IGER1_SYNF1_EMPTY_INT	(1 << 29)
#define FUSB300_IGER1_SYNF0_EMPTY_INT	(1 << 28)
#define FUSB300_IGER1_U3_EXIT_FAIL_INT	(1 << 27)
#define FUSB300_IGER1_U2_EXIT_FAIL_INT	(1 << 26)
#define FUSB300_IGER1_U1_EXIT_FAIL_INT	(1 << 25)
#define FUSB300_IGER1_U2_ENTRY_FAIL_INT	(1 << 24)
#define FUSB300_IGER1_U1_ENTRY_FAIL_INT	(1 << 23)
#define FUSB300_IGER1_U3_EXIT_INT	(1 << 22)
#define FUSB300_IGER1_U2_EXIT_INT	(1 << 21)
#define FUSB300_IGER1_U1_EXIT_INT	(1 << 20)
#define FUSB300_IGER1_U3_ENTRY_INT	(1 << 19)
#define FUSB300_IGER1_U2_ENTRY_INT	(1 << 18)
#define FUSB300_IGER1_U1_ENTRY_INT	(1 << 17)
#define FUSB300_IGER1_HOT_RST_INT	(1 << 16)
#define FUSB300_IGER1_WARM_RST_INT	(1 << 15)
#define FUSB300_IGER1_RESM_INT		(1 << 14)
#define FUSB300_IGER1_SUSP_INT		(1 << 13)
#define FUSB300_IGER1_LPM_INT		(1 << 12)
#define FUSB300_IGER1_HS_RST_INT	(1 << 11)
#define FUSB300_IGER1_EDEV_MODE_CHG_INT	(1 << 9)
#define FUSB300_IGER1_CX_COMABT_INT	(1 << 8)
#define FUSB300_IGER1_CX_COMFAIL_INT	(1 << 7)
#define FUSB300_IGER1_CX_CMDEND_INT	(1 << 6)
#define FUSB300_IGER1_CX_OUT_INT	(1 << 5)
#define FUSB300_IGER1_CX_IN_INT		(1 << 4)
#define FUSB300_IGER1_CX_SETUP_INT	(1 << 3)
#define FUSB300_IGER1_INTGRP4		(1 << 2)
#define FUSB300_IGER1_INTGRP3		(1 << 1)
#define FUSB300_IGER1_INTGRP2		(1 << 0)


/*
 * * Device Address Register (offset = 008H)
 * */
#define FUSB300_DAR_SETCONFG	(1 << 7)
#define FUSB300_DAR_DRVADDR(x)	(x & 0x7F)
#define FUSB300_DAR_DRVADDR_MSK	0x7F


/*
 * *Control Transfer Configuration and Status Register
 * (CX_Config_Status, offset = 00CH)
 * */
#define FUSB300_CSR_LEN(x)	((x & 0xFFFF) << 8)
#define FUSB300_CSR_LEN_MSK	(0xFFFF << 8)
#define FUSB300_CSR_EMP		(1 << 4)
#define FUSB300_CSR_FUL		(1 << 3)
#define FUSB300_CSR_CLR		(1 << 2)
#define FUSB300_CSR_STL		(1 << 1)
#define FUSB300_CSR_DONE		(1 << 0)


/*
 * * EPn Setting 0 (EPn_SET0, offset = 020H+(n-1)*30H, n=1~15 )
 * */
#define FUSB300_EPSET0_STL_CLR		(1 << 3)
#define FUSB300_EPSET0_CLRSEQNUM	(1 << 2)
#define FUSB300_EPSET0_STL		(1 << 0)

/*
 * * EPn Setting 1 (EPn_SET1, offset = 024H+(n-1)*30H, n=1~15)
 * */
#define FUSB300_EPSET1_START_ENTRY(x)  ((x & 0xFF) << 24)
#define FUSB300_EPSET1_START_ENTRY_MSK (0xFF << 24)
#define FUSB300_EPSET1_FIFOENTRY(x)    ((x & 0x1F) << 12)
#define FUSB300_EPSET1_FIFOENTRY_MSK   (0x1f << 12)
#define FUSB300_EPSET1_INTERVAL(x)     ((x & 0x7) << 6)
#define FUSB300_EPSET1_ISOINPKT(x)     ((x & 0x3f) << 6)
#define FUSB300_EPSET1_BWNUM(x)                ((x & 0x3) << 4)
#define FUSB300_EPSET1_TYPEISO         (1 << 2)
#define FUSB300_EPSET1_TYPEBLK         (2 << 2)
#define FUSB300_EPSET1_TYPEINT         (3 << 2)
#define FUSB300_EPSET1_TYPE(x)         ((x & 0x3) << 2)
#define FUSB300_EPSET1_TYPE_MSK                (0x3 << 2)
#define FUSB300_EPSET1_DIROUT          (0 << 1)
#define FUSB300_EPSET1_DIRIN           (1 << 1)
#define FUSB300_EPSET1_DIR(x)          ((x & 0x1) << 1)
#define FUSB300_EPSET1_DIRIN           (1 << 1)
#define FUSB300_EPSET1_DIR_MSK         ((0x1) << 1)
#define FUSB300_EPSET1_ACTDIS          0
#define FUSB300_EPSET1_ACTEN           1

/*
 * *EPn Setting 2 (EPn_SET2, offset = 028H+(n-1)*30H, n=1~15)
 * */
#define FUSB300_EPSET2_ADDROFS(x)      ((x & 0x7FFF) << 16)
#define FUSB300_EPSET2_ADDROFS_MSK     (0x7fff << 16)
#define FUSB300_EPSET2_MPS(x)          (x & 0x7FF)
#define FUSB300_EPSET2_MPS_MSK         0x7FF

/*
 * * EPn FIFO Register (offset = 2cH+(n-1)*30H)
 * */
#define FUSB300_FFR_RST                (1 << 31)
#define FUSB300_FF_FUL         (1 << 30)
#define FUSB300_FF_EMPTY       (1 << 29)
#define FUSB300_TX0BYTE                (1 << 28)
#define FUSB300_FFR_BYCNT      0x1FFFFF
/* EP PRD Ready (EP_PRD_RDY, offset = 504H) */

#define FUSB300_EPPRDR_EP15_PRD_RDY            (1 << 15)
#define FUSB300_EPPRDR_EP14_PRD_RDY            (1 << 14)
#define FUSB300_EPPRDR_EP13_PRD_RDY            (1 << 13)
#define FUSB300_EPPRDR_EP12_PRD_RDY            (1 << 12)
#define FUSB300_EPPRDR_EP11_PRD_RDY            (1 << 11)
#define FUSB300_EPPRDR_EP10_PRD_RDY            (1 << 10)
#define FUSB300_EPPRDR_EP9_PRD_RDY             (1 << 9)
#define FUSB300_EPPRDR_EP8_PRD_RDY             (1 << 8)
#define FUSB300_EPPRDR_EP7_PRD_RDY             (1 << 7)
#define FUSB300_EPPRDR_EP6_PRD_RDY             (1 << 6)
#define FUSB300_EPPRDR_EP5_PRD_RDY             (1 << 5)
#define FUSB300_EPPRDR_EP4_PRD_RDY             (1 << 4)
#define FUSB300_EPPRDR_EP3_PRD_RDY             (1 << 3)
#define FUSB300_EPPRDR_EP2_PRD_RDY             (1 << 2)
#define FUSB300_EPPRDR_EP1_PRD_RDY             (1 << 1)
#define FUSB300_EPPRDR_EP_PRD_RDY(n)           (1 << n)


/* WORD 0 Data Structure of PRD Table */
#define FUSB300_EPPRD0_M                       (1 << 30)
#define FUSB300_EPPRD0_O                       (1 << 29)
/* The finished prd */
#define FUSB300_EPPRD0_F                       (1 << 28)
#define FUSB300_EPPRD0_I                       (1 << 27)
#define FUSB300_EPPRD0_A                       (1 << 26)
/* To decide HW point to first prd at next time */
#define FUSB300_EPPRD0_L                       (1 << 25)
#define FUSB300_EPPRD0_H                       (1 << 24)
#define FUSB300_EPPRD0_BTC(n)                  (n & 0xFFFFFF)


#define FUSB330_MAX_NUM_EP		8

#define FUSB300_FIFO_ENTRY_NUM         4
#define FUSB300_MAX_FIFO_ENTRY         32

#define SS_CTL_MAX_PACKET_SIZE         0x200
#define SS_BULK_MAX_PACKET_SIZE                0x400
#define SS_INT_MAX_PACKET_SIZE         0x400
#define SS_ISO_MAX_PACKET_SIZE         0x400

#define HS_BULK_MAX_PACKET_SIZE                0x200
#define HS_CTL_MAX_PACKET_SIZE         0x40
#define HS_INT_MAX_PACKET_SIZE         0x400
#define HS_ISO_MAX_PACKET_SIZE         0x400

#define FUSB300_GCR_DEVEN_FS		0x3
#define FUSB300_GCR_DEVEN_HS		0x2
#define FUSB300_GCR_DEVEN_SS		0x1
#define FUSB300_GCR_DEVDIS		0x0
#define FUSB300_GCR_DEVEN_MSK		0x3



struct fusb300_ep_info {
       u8      epnum;
       u8      type;
       u8      interval;
       u8      dir_in;
       u16     maxpacket;
       u16     addrofs;
       u16     bw_num;
};

#endif
