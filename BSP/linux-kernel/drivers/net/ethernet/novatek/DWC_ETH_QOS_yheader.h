/* =========================================================================
 * The Synopsys DWC ETHER QOS Software Driver and documentation (hereinafter
 * "Software") is an unsupported proprietary work of Synopsys, Inc. unless
 * otherwise expressly agreed to in writing between Synopsys and you.
 *
 * The Software IS NOT an item of Licensed Software or Licensed Product under
 * any End User Software License Agreement or Agreement for Licensed Product
 * with Synopsys or any supplement thereto.  Permission is hereby granted,
 * free of charge, to any person obtaining a copy of this software annotated
 * with this license and the Software, to deal in the Software without
 * restriction, including without limitation the rights to use, copy, modify,
 * merge, publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so, subject
 * to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THIS SOFTWARE IS BEING DISTRIBUTED BY SYNOPSYS SOLELY ON AN "AS IS" BASIS
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE HEREBY DISCLAIMED. IN NO EVENT SHALL SYNOPSYS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
 * DAMAGE.
 * ========================================================================= */

#ifndef __DWC_ETH_QOS__YHEADER__

#define __DWC_ETH_QOS__YHEADER__

/* OS Specific declarations and definitions */

#include <linux/slab.h>
#include <linux/mm.h>
#include <linux/uaccess.h>
#include <linux/string.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/timer.h>
#include <linux/sched.h>
#include <linux/highmem.h>
#include <linux/proc_fs.h>
#include <linux/in.h>
#include <linux/ctype.h>
#include <linux/version.h>
#include <linux/ptrace.h>
#include <linux/dma-mapping.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/ethtool.h>
#include <linux/skbuff.h>
#include <linux/delay.h>
#include <linux/spinlock.h>
#include <linux/crc32.h>
#include <linux/bitops.h>
#include <linux/mii.h>
#include <asm/processor.h>
#include <asm/dma.h>
#include <asm/page.h>
#include <asm/irq.h>
#include <linux/clk.h>
#include <net/checksum.h>
#include <linux/tcp.h>
#include <linux/ip.h>
#include <linux/ipv6.h>
#include <linux/semaphore.h>
#include <linux/errno.h>
#include <linux/interrupt.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/io.h>
#include <linux/ioport.h>
#include <linux/phy.h>
#include <linux/phylink.h>
#include <linux/mdio.h>
#include <linux/of_mdio.h>
#include <linux/clk-provider.h>
#if defined(CONFIG_VLAN_8021Q) || defined(CONFIG_VLAN_8021Q_MODULE)
#define DWC_ETH_QOS_ENABLE_VLAN_TAG
#endif
#include <linux/if_vlan.h>
/* for PPT */
#include <linux/ptp_clock.h>
#include <linux/net_tstamp.h>
#include <linux/ptp_clock_kernel.h>
#include <linux/clocksource.h>
#include <linux/gpio.h>
#include <plat/top.h>
#include <plat/pad.h>
#include <plat/nvt-gpio.h>
#include <plat/hardware.h>

#include <net/pkt_sched.h>
#include <net/pkt_cls.h>


#define DWC_ETH_QOS_CONFIG_TSO
#define DWC_ETH_QOS_CONFIG_UFO
//#define DWC_ETH_QOS_CONFIG_EEE
#if IS_ENABLED(CONFIG_TIMESTAMP)
#define DWC_ETH_QOS_CONFIG_TIMESTAMP
#endif
//#define CONFIG_DEBUGFS_OBJ

//#define DWC_ETH_QOS_CACHE_ACC

#include "DWC_ETH_QOS_debug_operation.h"
#include "DWC_ETH_QOS_platform.h"
#if IS_ENABLED(CONFIG_PCI)
#include <linux/soc/nvt/nvt-pcie-lib.h>
#endif

/* QOS Version Control Macros */
//#define DWC_ETH_QOS_VER_4_0  /* Default Configuration is for QOS version 4.1 and above */

/* Macro definitions*/

/*#include <asm-generic/errno.h>*/
//#define CONFIG_MDIO_INT_MODE

#if !IS_ENABLED(CONFIG_PCI)
#define CHIP_RC 0
#endif
#if IS_ENABLED(CONFIG_PREEMPT_RT_BASE)
#define DWC_ETH_QOS_RT_LINUX
#endif

#ifdef CONFIG_USAGECONTROL
//#define TIMER_CONTROL
#define DWC_ETH_QOS_ENABLE_IRQ_MOD
#endif

#ifdef DWC_ETH_QOS_ENABLE_IRQ_MOD
//#define DWC_ETH_QOS_ENABLE_IRQ_MOD_DEBUG
#define NVT_ETH_TX_IRQ_MOD_TMR_LIM NVT_IRQ_MOD_TMR_5MS
#define NVT_ETH_RX_IRQ_MOD_TMR_LIM NVT_IRQ_MOD_TMR_5MS
#endif

#ifdef TIMER_CONTROL
#define TIMER_LATENCY 100   /* ms */
#endif

#ifdef CONFIG_PGTEST_OBJ
#define DWC_ETH_QOS_CONFIG_PGTEST
#endif

#if IS_ENABLED(CONFIG_TIMESTAMP)
#define DWC_ETH_QOS_CONFIG_PTP
#endif

#ifdef CONFIG_DEBUGFS_OBJ
#define DWC_ETH_QOS_CONFIG_DEBUGFS
#endif

#ifdef DWC_ETH_QOS_CONFIG_PGTEST

#define DWC_ETH_QOS_DA_SA 12
#define DWC_ETH_QOS_TYPE 2
#define DWC_ETH_QOS_VLAN_TAG 4
#define DWC_ETH_QOS_ETH_HDR_AVB (DWC_ETH_QOS_DA_SA + \
								 DWC_ETH_QOS_TYPE + \
								 DWC_ETH_QOS_VLAN_TAG)

#define DWC_ETH_QOS_PG_FRAME_SIZE (pdata->dev->mtu + DWC_ETH_QOS_ETH_HDR_AVB)
#define DWC_ETH_QOS_AVTYPE 0x22f0

#endif /* end of DWC_ETH_QOS_CONFIG_PGTEST */

/* NOTE: Uncomment below line for TX and RX DESCRIPTOR DUMP in KERNEL LOG */
//#define DWC_ETH_QOS_ENABLE_TX_DESC_DUMP
//#define DWC_ETH_QOS_ENABLE_RX_DESC_DUMP

/* NOTE: Uncomment below line for TX and RX PACKET DUMP in KERNEL LOG */
//#define DWC_ETH_QOS_ENABLE_TX_PKT_DUMP
//#define DWC_ETH_QOS_ENABLE_RX_PKT_DUMP

/* Uncomment below macro definitions for testing corresponding IP features in driver */
//#define DWC_ETH_QOS_QUEUE_SELECT_ALGO
//#define DWC_ETH_QOS_CERTIFICATION_PKTBURSTCNT
//#define DWC_ETH_QOS_CERTIFICATION_PKTBURSTCNT_HALFDUPLEX
//#define DWC_ETH_QOS_TXPOLLING_MODE_ENABLE

#ifdef DWC_ETH_QOS_CONFIG_PTP
#undef DWC_ETH_QOS_TXPOLLING_MODE_ENABLE
#endif

/* Uncomment below macro to enable Double VLAN support. */
//#define DWC_ETH_QOS_ENABLE_DVLAN

/* Uncomment below macro to test EEE feature Tx path with
 * no EEE supported PHY card
 * */
//#define DWC_ETH_QOS_CUSTOMIZED_EEE_TEST

#ifdef DWC_ETH_QOS_CUSTOMIZED_EEE_TEST
#undef DWC_ETH_QOS_TXPOLLING_MODE_ENABLE
#endif


#ifdef DWC_ETH_QOS_CONFIG_PGTEST
#undef DWC_ETH_QOS_TXPOLLING_MODE_ENABLE
#endif

#ifdef DWC_ETH_QOS_CERTIFICATION_PKTBURSTCNT_HALFDUPLEX
#define DWC_ETH_QOS_CERTIFICATION_PKTBURSTCNT
#endif

#ifdef DWC_ETH_QOS_CERTIFICATION_PKTBURSTCNT
#undef DWC_ETH_QOS_TXPOLLING_MODE_ENABLE
#endif

/* NOTE: Uncomment below line for function trace log messages in KERNEL LOG */
//#define YDEBUG
//#define YDEBUG_PG
//#define YDEBUG_MDIO
//#define YDEBUG_PTP
//#define YDEBUG_FILTER
//#define YDEBUG_EEE

#define Y_TRUE 1
#define Y_FALSE 0
#define Y_SUCCESS 0
#define Y_FAILURE 1
#define Y_INV_WR 1
#define Y_INV_RD 2
#define Y_INV_ARG 3
#define Y_MAX_THRD_XEEDED 4

/* The following macros map error macros to POSIX errno values */
#define ERR_READ_TIMEOUT ETIME
#define ERR_WRITE_TIMEOUT ETIME
#define ERR_FIFO_READ_FAILURE EIO
#define ERR_FIFO_WRITE_FAILURE EIO
#define ERR_READ_OVRFLW ENOBUFS
#define ERR_READ_UNDRFLW ENODATA
#define ERR_WRITE_OVRFLW ENOBUFS
#define ERR_WRITE_UNDRFLW ENODATA

/* Helper macros for STANDARD VIRTUAL register handling */

#define GET_TX_ERROR_COUNTERS_PTR (&(pdata->tx_error_counters))

#define GET_RX_ERROR_COUNTERS_PTR (&(pdata->rx_error_counters))

#define GET_RX_PKT_FEATURES_PTR (&(pdata->rx_pkt_features))

#define GET_TX_PKT_FEATURES_PTR (&(pdata->tx_pkt_features))

#define MASK (0x1ULL << 0 | \
			  0x13c7ULL << 32)
#define MAC_MASK (0x10ULL << 0)
#ifdef CONFIG_NOVATEK_DWC_ETH_QOS_HIGHDMA
#define DWC_ETH_QOS_DMA_MASK 40
#else
#define DWC_ETH_QOS_DMA_MASK 32
#endif
#ifdef CONFIG_TX_DESC
#define TX_DESC_CNT CONFIG_TX_DESC
#else
#define TX_DESC_CNT 256
#endif
#ifdef CONFIG_RX_DESC
#define RX_DESC_CNT CONFIG_RX_DESC
#else
#define RX_DESC_CNT 256
#endif
#define SKB_DESC_CNT (256 * 3)
#define MIN_RX_DESC_CNT 16
#define TX_BUF_SIZE 1536
#define RX_BUF_SIZE 1568

#define MIN_PACKET_SIZE 60

/*
#ifdef DWC_ETH_QOS_ENABLE_VLAN_TAG
#define MAX_PACKET_SIZE VLAN_ETH_FRAME_LEN
#else
#define MAX_PACKET_SIZE 1514
#endif
*/

/* RX header size for split header */
#define DWC_ETH_QOS_HDR_SIZE_64B   64   /* 64 bytes */
#define DWC_ETH_QOS_HDR_SIZE_128B  128  /* 128 bytes */
#define DWC_ETH_QOS_HDR_SIZE_256B  256  /* 256 bytes */
#define DWC_ETH_QOS_HDR_SIZE_512B  512  /* 512 bytes */
#define DWC_ETH_QOS_HDR_SIZE_1024B 1024 /* 1024 bytes */

#define DWC_ETH_QOS_MAX_HDR_SIZE DWC_ETH_QOS_HDR_SIZE_256B

#define MAX_MULTICAST_LIST 14
#define RX_DESC_DATA_LENGTH_LBIT 0
#define RX_DESC_DATA_LENGTH 0x7fff
#define DWC_ETH_QOS_TX_FLAGS_IP_PKT 0x00000001
#define DWC_ETH_QOS_TX_FLAGS_TCP_PKT 0x00000002

#define DEV_NAME "DWC_ETH_QOS"
#define DEV_ADDRESS 0xffffffff
#define DEV_REG_MMAP_SIZE 0x14e8

#define VENDOR_ID 0x16C3
#define DEVICE_ID_HAPS_6X 0x7101
#define DEVICE_ID_HAPS_DX 0x7102
#define COMPLETE_BAR 0

/* MII/GMII register offset */
#define DWC_ETH_QOS_AUTO_NEGO_NP    0x0007
#define DWC_ETH_QOS_PHY_CTL     0x0010
#define DWC_ETH_QOS_PHY_STS     0x0011

/* Default MTL queue operation mode values */
#define DWC_ETH_QOS_Q_DISABLED  0x0
#define DWC_ETH_QOS_Q_AVB           0x1
#define DWC_ETH_QOS_Q_DCB           0x2
#define DWC_ETH_QOS_Q_GENERIC   0x3

/* TSE_MODE*/
#define DWC_ETH_QOS_TSE_MODE_USO        0x0
#define DWC_ETH_QOS_TSE_MODE_UFO_WITH_CSUM  0x1
#define DWC_ETH_QOS_TSE_MODE_UFO_WITHOUT_CSUM   0x2

/* Driver PMT macros */
#define DWC_ETH_QOS_DRIVER_CONTEXT 1
#define DWC_ETH_QOS_IOCTL_CONTEXT 2
#define DWC_ETH_QOS_MAGIC_WAKEUP    (1 << 0)
#define DWC_ETH_QOS_REMOTE_WAKEUP   (1 << 1)
#define DWC_ETH_QOS_POWER_DOWN_TYPE(x)  \
	((x->power_down_type & DWC_ETH_QOS_MAGIC_WAKEUP) ? \
	 "Magic packet" : \
	 ((x->power_down_type & DWC_ETH_QOS_REMOTE_WAKEUP) ? \
	  "Remote wakeup packet" : \
	  "<error>"))

#define DWC_ETH_QOS_MAC_ADDR_LEN 6
#define DWC_ETH_QOS_ETH_FRAME_LEN (ETH_FRAME_LEN + ETH_FCS_LEN + VLAN_HLEN)

#define FIFO_SIZE_B(x) (x)
#define FIFO_SIZE_KB(x) (x*1024)

//#define DWC_ETH_QOS_MAX_DATA_PER_TX_BUF (1 << 13)     /* 8 KB Maximum data per buffer pointer(in Bytes) */
#define DWC_ETH_QOS_MAX_DATA_PER_TX_BUF (1 << 12)   /* for testing purpose: 4 KB Maximum data per buffer pointer(in Bytes) */
#define DWC_ETH_QOS_MAX_DATA_PER_TXD (DWC_ETH_QOS_MAX_DATA_PER_TX_BUF * 2)  /* Maxmimum data per descriptor(in Bytes) */

#define DWC_ETH_QOS_MAX_SUPPORTED_MTU 16380
#define DWC_ETH_QOS_MAX_GPSL 9000 /* Default maximum Gaint Packet Size Limit */
#define DWC_ETH_QOS_MIN_SUPPORTED_MTU (ETH_ZLEN + ETH_FCS_LEN + VLAN_HLEN)

#define DWC_ETH_QOS_RDESC3_OWN  0x80000000
#define DWC_ETH_QOS_RDESC3_FD       0x20000000
#define DWC_ETH_QOS_RDESC3_LD       0x10000000
#define DWC_ETH_QOS_RDESC3_RS2V 0x08000000
#define DWC_ETH_QOS_RDESC3_RS1V 0x04000000
#define DWC_ETH_QOS_RDESC3_RS0V 0x02000000
#define DWC_ETH_QOS_RDESC3_LT       0x00070000
#define DWC_ETH_QOS_RDESC3_ES       0x00008000
#define DWC_ETH_QOS_RDESC3_PL       0x00007FFF

#define DWC_ETH_QOS_RDESC2_HL   0x000003FF

#define DWC_ETH_QOS_RDESC1_PT       0x00000007 /* Payload type */
#define DWC_ETH_QOS_RDESC1_PT_TCP   0x00000002 /* Payload type = TCP */

/* Maximum size of pkt that is copied to a new buffer on receive */
#define DWC_ETH_QOS_COPYBREAK_DEFAULT 256

#define DWC_ETH_QOS_TX_QUEUE_CNT (pdata->tx_queue_cnt)
#define DWC_ETH_QOS_RX_QUEUE_CNT (pdata->rx_queue_cnt)
#define DWC_ETH_QOS_QUEUE_CNT min(DWC_ETH_QOS_TX_QUEUE_CNT, DWC_ETH_QOS_RX_QUEUE_CNT)

/* Helper macros for TX descriptor handling */

#define GET_TX_QUEUE_PTR(qInx) (&(pdata->tx_queue[(qInx)]))

#define GET_TX_DESC_PTR(qInx, dInx) (pdata->tx_queue[(qInx)].tx_desc_data.tx_desc_ptrs[(dInx)])

#define GET_TX_DESC_DMA_ADDR(qInx, dInx) (pdata->tx_queue[(qInx)].tx_desc_data.tx_desc_dma_addrs[(dInx)])

#define GET_TX_WRAPPER_DESC(qInx) (&(pdata->tx_queue[(qInx)].tx_desc_data))

#define GET_TX_BUF_PTR(qInx, dInx) (pdata->tx_queue[(qInx)].tx_desc_data.tx_buf_ptrs[(dInx)])

#define INCR_TX_DESC_INDEX(inx, offset) do {\
		(inx) += (offset);\
		if ((inx) >= TX_DESC_CNT)\
			(inx) = ((inx) - TX_DESC_CNT);\
	} while (0)

#define DECR_TX_DESC_INDEX(inx) do {\
		(inx)--;\
		if ((inx) < 0)\
			(inx) = (TX_DESC_CNT + (inx));\
	} while (0)

#define INCR_TX_LOCAL_INDEX(inx, offset)\
	(((inx) + (offset)) >= TX_DESC_CNT ?\
	 ((inx) + (offset) - TX_DESC_CNT) : ((inx) + (offset)))

#define GET_CURRENT_XFER_DESC_CNT(qInx) (pdata->tx_queue[(qInx)].tx_desc_data.packet_count)

#define GET_CURRENT_XFER_LAST_DESC_INDEX(qInx, start_index, offset)\
	(GET_CURRENT_XFER_DESC_CNT((qInx)) == 0) ? (TX_DESC_CNT - 1) :\
	((GET_CURRENT_XFER_DESC_CNT((qInx)) == 1) ? (INCR_TX_LOCAL_INDEX((start_index), (offset))) :\
	 INCR_TX_LOCAL_INDEX((start_index), (GET_CURRENT_XFER_DESC_CNT((qInx)) + (offset) - 1)))

#define GET_TX_TOT_LEN(buffer, start_index, packet_count, total_len) do {\
		int i, pkt_idx = (start_index);\
		for(i = 0; i < (packet_count); i++) {\
			(total_len) += ((buffer)[pkt_idx].len + (buffer)[pkt_idx].len2);\
			pkt_idx = INCR_TX_LOCAL_INDEX(pkt_idx, 1);\
		} \
	} while (0)

/* Helper macros for RX descriptor handling */

#define GET_RX_QUEUE_PTR(qInx) (&(pdata->rx_queue[(qInx)]))

#define GET_RX_DESC_PTR(qInx, dInx) (pdata->rx_queue[(qInx)].rx_desc_data.rx_desc_ptrs[(dInx)])

#define GET_RX_DESC_DMA_ADDR(qInx, dInx) (pdata->rx_queue[(qInx)].rx_desc_data.rx_desc_dma_addrs[(dInx)])

#define GET_RX_WRAPPER_DESC(qInx) (&(pdata->rx_queue[(qInx)].rx_desc_data))

#define GET_RX_BUF_PTR(qInx, dInx) (pdata->rx_queue[(qInx)].rx_desc_data.rx_buf_ptrs[(dInx)])

#define INCR_RX_DESC_INDEX(inx, offset) do {\
		(inx) += (offset);\
		if ((inx) >= RX_DESC_CNT)\
			(inx) = ((inx) - RX_DESC_CNT);\
	} while (0)

#define DECR_RX_DESC_INDEX(inx) do {\
		(inx)--;\
		if ((inx) < 0)\
			(inx) = (RX_DESC_CNT + (inx));\
	} while (0)

#define INCR_RX_LOCAL_INDEX(inx, offset)\
	(((inx) + (offset)) >= RX_DESC_CNT ?\
	 ((inx) + (offset) - RX_DESC_CNT) : ((inx) + (offset)))

#define GET_CURRENT_RCVD_DESC_CNT(qInx) (pdata->rx_queue[(qInx)].rx_desc_data.pkt_received)

#define GET_CURRENT_RCVD_LAST_DESC_INDEX(start_index, offset) (RX_DESC_CNT - 1)

#define GET_TX_DESC_IDX(qInx, desc) (((desc) - GET_TX_DESC_DMA_ADDR((qInx), 0))/(sizeof(struct s_TX_NORMAL_DESC)))

#define GET_RX_DESC_IDX(qInx, desc) (((desc) - GET_RX_DESC_DMA_ADDR((qInx), 0))/(sizeof(struct s_RX_NORMAL_DESC)))

/* Helper macros for handling receive error */
#define DWC_ETH_QOS_RX_LENGTH_ERR        0x00000001
#define DWC_ETH_QOS_RX_BUF_OVERFLOW_ERR  0x00000002
#define DWC_ETH_QOS_RX_CRC_ERR           0x00000004
#define DWC_ETH_QOS_RX_FRAME_ERR         0x00000008
#define DWC_ETH_QOS_RX_FIFO_OVERFLOW_ERR 0x00000010
#define DWC_ETH_QOS_RX_MISSED_PKT_ERR    0x00000020

#define DWC_ETH_QOS_RX_CHECKSUM_DONE 0x00000001
#define DWC_ETH_QOS_RX_VLAN_PKT      0x00000002

/* MAC Time stamp contorl reg bit fields */
#define MAC_TCR_TSENA         0x00000001 /* Enable timestamp */
#define MAC_TCR_TSCFUPDT      0x00000002 /* Enable Fine Timestamp Update */
#define MAC_TCR_TSENALL       0x00000100 /* Enable timestamping for all packets */
#define MAC_TCR_TSCTRLSSR     0x00000200 /* Enable Timestamp Digitla Contorl (1ns accuracy )*/
#define MAC_TCR_TSVER2ENA     0x00000400 /* Enable PTP packet processing for Version 2 Formate */
#define MAC_TCR_TSIPENA       0x00000800 /* Enable processing of PTP over Ethernet Packets */
#define MAC_TCR_TSIPV6ENA     0x00001000 /* Enable processing of PTP Packets sent over IPv6-UDP Packets */
#define MAC_TCR_TSIPV4ENA     0x00002000 /* Enable processing of PTP Packets sent over IPv4-UDP Packets */
#define MAC_TCR_TSEVENTENA    0x00004000 /* Enable Timestamp Snapshot for Event Messages */
#define MAC_TCR_TSMASTERENA   0x00008000 /* Enable snapshot for Message Relevent to Master */
#define MAC_TCR_SNAPTYPSEL_1  0x00010000 /* select PTP packets for taking snapshots */
#define MAC_TCR_SNAPTYPSEL_2  0x00020000
#define MAC_TCR_SNAPTYPSEL_3  0x00030000
#define MAC_TCR_TSENMACADDR   0x00040000 /* Enable MAC address for PTP Frame Filtering*/
#define MAC_TCR_AV8021ASMEN   0x10000000 /* Enable AV 802.1AS Mode */

/* PTP Offloading control register bits (MAC_PTO_control)*/
#define MAC_PTOCR_PTOEN       0x00000001 /* PTP offload Enable */
#define MAC_PTOCR_ASYNCEN     0x00000002 /* Automatic PTP Sync message enable */
#define MAC_PTOCR_APDREQEN    0x00000004 /* Automatic PTP Pdelay_Req message enable */


/* Hash Table Reg count */
#define DWC_ETH_QOS_HTR_CNT (256/32)

/* For handling VLAN filtering */
#define DWC_ETH_QOS_VLAN_PERFECT_FILTERING 0
#define DWC_ETH_QOS_VLAN_HASH_FILTERING 1

/* For handling differnet PHY interfaces */
#define DWC_ETH_QOS_GMII_MII    0x0
#define DWC_ETH_QOS_RGMII   0x1
#define DWC_ETH_QOS_SGMII   0x2
#define DWC_ETH_QOS_TBI     0x3
#define DWC_ETH_QOS_RMII    0x4
#define DWC_ETH_QOS_RTBI    0x5
#define DWC_ETH_QOS_SMII    0x6
#define DWC_ETH_QOS_RevMII  0x7

/* for EEE */
#define DWC_ETH_QOS_DEFAULT_LPI_LS_TIMER 0x3E8 /* 1000 in decimal */
#define DWC_ETH_QOS_DEFAULT_LPI_TWT_TIMER 0x0

#define DWC_ETH_QOS_DEFAULT_LPI_TIMER 1000 /* LPI Tx local expiration time in msec */
#define DWC_ETH_QOS_LPI_TIMER(x) (jiffies + msecs_to_jiffies(x))

/* Error and status macros defined below */

#define E_DMA_SR_TPS        6
#define E_DMA_SR_TBU        7
#define E_DMA_SR_RBU        8
#define E_DMA_SR_RPS        9
#define S_DMA_SR_RWT        2
#define E_DMA_SR_FBE       10
#define S_MAC_ISR_PMTIS     11

/* C data types typedefs */

//typedef unsigned short BOOL;  // conflict with nvt_type.h
typedef char CHAR;
typedef char *CHARP;
typedef double DOUBLE;
typedef double *DOUBLEP;
typedef float FLOAT;
typedef float *FLOATP;
typedef int INT;
typedef int *INTP;
typedef long LONG;
typedef long *LONGP;
typedef short SHORT;
typedef short *SHORTP;
typedef unsigned UINT;
typedef unsigned *UINTP;
typedef unsigned char UCHAR;
typedef unsigned char *UCHARP;
typedef unsigned long ULONG;
typedef unsigned long *ULONGP;
typedef unsigned long long ULONG_LONG;
typedef unsigned short USHORT;
typedef unsigned short *USHORTP;
typedef void VOID;
typedef void *VOIDP;

struct s_RX_CONTEXT_DESC {
	UINT RDES0;
	UINT RDES1;
	UINT RDES2;
	UINT RDES3;
};

typedef struct s_RX_CONTEXT_DESC t_RX_CONTEXT_DESC;

struct s_TX_CONTEXT_DESC {
	UINT TDES0;
	UINT TDES1;
	UINT TDES2;
	UINT TDES3;
};

typedef struct s_TX_CONTEXT_DESC t_TX_CONTEXT_DESC;

struct s_RX_NORMAL_DESC {
	UINT RDES0;
	UINT RDES1;
	UINT RDES2;
	UINT RDES3;
};

typedef struct s_RX_NORMAL_DESC t_RX_NORMAL_DESC;

struct s_TX_NORMAL_DESC {
	UINT TDES0;
	UINT TDES1;
	UINT TDES2;
	UINT TDES3;
};

typedef struct s_TX_NORMAL_DESC t_TX_NORMAL_DESC;

struct s_tx_error_counters {
	UINT tx_errors;
};

typedef struct s_tx_error_counters t_tx_error_counters;

struct s_rx_error_counters {
	UINT rx_errors;
};

typedef struct s_rx_error_counters t_rx_error_counters;

struct s_rx_pkt_features {
	UINT pkt_attributes;
	UINT vlan_tag;
};

typedef struct s_rx_pkt_features t_rx_pkt_features;

struct s_tx_pkt_features {
	UINT pkt_attributes;
	UINT vlan_tag;
	ULONG mss;
	ULONG hdr_len;
	ULONG pay_len;
	UCHAR ipcss;
	UCHAR ipcso;
	USHORT ipcse;
	UCHAR tucss;
	UCHAR tucso;
	USHORT tucse;
	UINT pkt_type;
	ULONG tcp_udp_hdr_len;
};

typedef struct s_tx_pkt_features t_tx_pkt_features;

typedef enum {
	eDWC_ETH_QOS_DMA_ISR_DC0IS,
	eDWC_ETH_QOS_DMA_SR0_TI,
	eDWC_ETH_QOS_DMA_SR0_TPS,
	eDWC_ETH_QOS_DMA_SR0_TBU,
	eDWC_ETH_QOS_DMA_SR0_RI,
	eDWC_ETH_QOS_DMA_SR0_RBU,
	eDWC_ETH_QOS_DMA_SR0_RPS,
	eDWC_ETH_QOS_DMA_SR0_FBE,
	eDWC_ETH_QOS_ALL
} e_DWC_ETH_QOS_int_id;

typedef enum {
	eDWC_ETH_QOS_256 = 0x0,
	eDWC_ETH_QOS_512 = 0x1,
	eDWC_ETH_QOS_1k = 0x3,
	eDWC_ETH_QOS_2k = 0x7,
	eDWC_ETH_QOS_4k = 0xf,
	eDWC_ETH_QOS_8k = 0x1f,
	eDWC_ETH_QOS_16k = 0x3f,
	eDWC_ETH_QOS_32k = 0x7f
} eDWC_ETH_QOS_mtl_fifo_size;

/* do forward declaration of private data structure */
struct DWC_ETH_QOS_prv_data;
struct DWC_ETH_QOS_tx_wrapper_descriptor;

struct hw_if_struct {

	INT(*tx_complete)(struct s_TX_NORMAL_DESC *);
	INT(*tx_window_error)(struct s_TX_NORMAL_DESC *);
	INT(*tx_aborted_error)(struct s_TX_NORMAL_DESC *);
	INT(*tx_carrier_lost_error)(struct s_TX_NORMAL_DESC *);
	INT(*tx_fifo_underrun)(struct s_TX_NORMAL_DESC *);
	INT(*tx_get_collision_count)(struct s_TX_NORMAL_DESC *);
	INT(*tx_handle_aborted_error)(struct s_TX_NORMAL_DESC *);
	INT(*tx_update_fifo_threshold)(struct s_TX_NORMAL_DESC *);
	/*tx threshold config */
	INT(*tx_config_threshold)(UINT);

	INT(*set_promiscuous_mode)(struct DWC_ETH_QOS_prv_data *);
	INT(*set_all_multicast_mode)(struct DWC_ETH_QOS_prv_data *);
	INT(*set_multicast_list_mode)(struct DWC_ETH_QOS_prv_data *);
	INT(*set_unicast_mode)(struct DWC_ETH_QOS_prv_data *);

	INT(*enable_rx_csum)(struct DWC_ETH_QOS_prv_data *);
	INT(*disable_rx_csum)(struct DWC_ETH_QOS_prv_data *);
	INT(*get_rx_csum_status)(struct DWC_ETH_QOS_prv_data *);

	INT(*read_phy_regs)(struct DWC_ETH_QOS_prv_data *, INT, INT, INT *);
	INT(*write_phy_regs)(struct DWC_ETH_QOS_prv_data *, INT, INT, INT);
	INT(*set_full_duplex)(struct DWC_ETH_QOS_prv_data *);
	INT(*set_half_duplex)(struct DWC_ETH_QOS_prv_data *);
	INT(*set_mii_speed_100)(struct DWC_ETH_QOS_prv_data *);
	INT(*set_mii_speed_10)(struct DWC_ETH_QOS_prv_data *);
	INT(*set_gmii_speed)(struct DWC_ETH_QOS_prv_data *);
	/* for PMT */
	INT(*start_dma_rx)(struct DWC_ETH_QOS_prv_data *, UINT);
	INT(*stop_dma_rx)(struct DWC_ETH_QOS_prv_data *, UINT);
	INT(*start_dma_tx)(struct DWC_ETH_QOS_prv_data *, UINT);
	INT(*stop_dma_tx)(struct DWC_ETH_QOS_prv_data *, UINT);
	INT(*start_mac_tx_rx)(struct DWC_ETH_QOS_prv_data *);
	INT(*stop_mac_tx_rx)(struct DWC_ETH_QOS_prv_data *);

	INT(*init)(struct DWC_ETH_QOS_prv_data *);
	INT(*exit)(struct DWC_ETH_QOS_prv_data *);
	INT(*enable_int)(e_DWC_ETH_QOS_int_id);
	INT(*disable_int)(e_DWC_ETH_QOS_int_id);
	void (*pre_xmit)(struct DWC_ETH_QOS_prv_data *, UINT qInx);
	void (*dev_read)(struct DWC_ETH_QOS_prv_data *, UINT qInx);
	void (*tx_desc_init)(struct DWC_ETH_QOS_prv_data *, UINT qInx);
	void (*rx_desc_init)(struct DWC_ETH_QOS_prv_data *, UINT qInx);
	void (*rx_desc_reset)(UINT, struct DWC_ETH_QOS_prv_data *,
						  UINT, UINT qInx);
	INT(*tx_desc_reset)(UINT, struct DWC_ETH_QOS_prv_data *, UINT qInx);
	/* last tx segmnet reports the tx status */
	INT(*get_tx_desc_ls)(struct s_TX_NORMAL_DESC *);
	INT(*get_tx_desc_ctxt)(struct s_TX_NORMAL_DESC *);
	void (*update_rx_tail_ptr)(struct DWC_ETH_QOS_prv_data *,
							   unsigned int qInx, dma_addr_t dma_addr);

	/* for FLOW ctrl */
	INT(*enable_rx_flow_ctrl)(struct DWC_ETH_QOS_prv_data *);
	INT(*disable_rx_flow_ctrl)(struct DWC_ETH_QOS_prv_data *);
	INT(*enable_tx_flow_ctrl)(struct DWC_ETH_QOS_prv_data *, UINT);
	INT(*disable_tx_flow_ctrl)(struct DWC_ETH_QOS_prv_data *, UINT);

	/* for PMT operations */
	INT(*enable_magic_pmt)(struct DWC_ETH_QOS_prv_data *);
	INT(*disable_magic_pmt)(struct DWC_ETH_QOS_prv_data *);
	INT(*enable_remote_pmt)(struct DWC_ETH_QOS_prv_data *);
	INT(*disable_remote_pmt)(struct DWC_ETH_QOS_prv_data *);
	INT(*configure_rwk_filter)(struct DWC_ETH_QOS_prv_data *, UINT *, UINT);

	/* for RX watchdog timer */
	INT(*config_rx_watchdog)(struct DWC_ETH_QOS_prv_data *, UINT, u32 riwt);

	/* for RX and TX threshold config */
	INT(*config_rx_threshold)(struct DWC_ETH_QOS_prv_data *, UINT ch_no, UINT val);
	INT(*config_tx_threshold)(struct DWC_ETH_QOS_prv_data *, UINT ch_no, UINT val);

	/* for RX and TX Store and Forward Mode config */
	INT(*config_rsf_mode)(struct DWC_ETH_QOS_prv_data *, UINT ch_no, UINT val);
	INT(*config_tsf_mode)(struct DWC_ETH_QOS_prv_data *, UINT ch_no, UINT val);

	/* for TX DMA Operate on Second Frame config */
	INT(*config_osf_mode)(struct DWC_ETH_QOS_prv_data *, UINT ch_no, UINT val);

	/* for INCR/INCRX config */
	INT(*config_incr_incrx_mode)(struct DWC_ETH_QOS_prv_data *, UINT val);
	/* for AXI PBL config */
	INT(*config_axi_pbl_val)(struct DWC_ETH_QOS_prv_data *, UINT val);
	/* for AXI WORL config */
	INT(*config_axi_worl_val)(struct DWC_ETH_QOS_prv_data *, UINT val);
	/* for AXI RORL config */
	INT(*config_axi_rorl_val)(struct DWC_ETH_QOS_prv_data *, UINT val);

	/* for RX and TX PBL config */
	INT(*config_rx_pbl_val)(struct DWC_ETH_QOS_prv_data *, UINT ch_no, UINT val);
	INT(*get_rx_pbl_val)(struct DWC_ETH_QOS_prv_data *, UINT ch_no);
	INT(*config_tx_pbl_val)(struct DWC_ETH_QOS_prv_data *, UINT ch_no, UINT val);
	INT(*get_tx_pbl_val)(struct DWC_ETH_QOS_prv_data *, UINT ch_no);
	INT(*config_pblx8)(struct DWC_ETH_QOS_prv_data *, UINT ch_no, UINT val);

	/* for TX vlan control */
	VOID(*enable_vlan_reg_control)(struct DWC_ETH_QOS_prv_data *,
								   struct DWC_ETH_QOS_tx_wrapper_descriptor *desc_data);
	VOID(*enable_vlan_desc_control)(struct DWC_ETH_QOS_prv_data *pdata);

	/* for rx vlan stripping */
//	 VOID(*config_rx_outer_vlan_stripping) (u32);
//	 VOID(*config_rx_inner_vlan_stripping) (u32);

	/* for sa(source address) insert/replace */
	VOID(*configure_mac_addr0_reg)(struct DWC_ETH_QOS_prv_data *, UCHAR *);
	VOID(*configure_mac_addr1_reg)(struct DWC_ETH_QOS_prv_data *, UCHAR *);
	VOID(*configure_sa_via_reg)(struct DWC_ETH_QOS_prv_data *, u32);

	/* for handling multi-queue */
	INT(*disable_dma_interrupt)(struct DWC_ETH_QOS_prv_data *, UINT);
	INT(*enable_dma_interrupt)(struct DWC_ETH_QOS_prv_data *, UINT);

	/* for handling tx multi-queue */
	INT(*disable_rx_dma_interrupt)(struct DWC_ETH_QOS_prv_data *, UINT);
	INT(*enable_rx_dma_interrupt)(struct DWC_ETH_QOS_prv_data *, UINT);

	/* for handling rx multi-queue */
	INT(*disable_tx_dma_interrupt)(struct DWC_ETH_QOS_prv_data *, UINT);
	INT(*enable_tx_dma_interrupt)(struct DWC_ETH_QOS_prv_data *, UINT);

	/* for handling MMC */
	INT(*disable_mmc_interrupts)(struct DWC_ETH_QOS_prv_data *);
	INT(*config_mmc_counters)(struct DWC_ETH_QOS_prv_data *);

	/* for handling split header */
	INT(*config_split_header_mode)(struct DWC_ETH_QOS_prv_data *, UINT qInx, USHORT sph_en);
	INT(*config_header_size)(struct DWC_ETH_QOS_prv_data *, USHORT header_size);

	/* for handling DCB and AVB */
	INT(*set_dcb_algorithm)(struct DWC_ETH_QOS_prv_data *, UCHAR dcb_algorithm);
	INT(*set_dcb_queue_weight)(struct DWC_ETH_QOS_prv_data *, UINT qInx, UINT q_weight);

	INT(*set_tx_queue_operating_mode)(struct DWC_ETH_QOS_prv_data *, UINT qInx, UINT q_mode);
	INT(*set_avb_algorithm)(struct DWC_ETH_QOS_prv_data *, UINT qInx, UCHAR avb_algorithm);
	INT(*config_credit_control)(struct DWC_ETH_QOS_prv_data *, UINT qInx, UINT cc);
	INT(*config_send_slope)(struct DWC_ETH_QOS_prv_data *, UINT qInx, UINT send_slope);
	INT(*config_idle_slope)(struct DWC_ETH_QOS_prv_data *, UINT qInx, UINT idle_slope);
	INT(*config_high_credit)(struct DWC_ETH_QOS_prv_data *, UINT qInx, UINT hi_credit);
	INT(*config_low_credit)(struct DWC_ETH_QOS_prv_data *, UINT qInx, UINT lo_credit);
	INT(*config_slot_num_check)(struct DWC_ETH_QOS_prv_data *, UINT qInx, UCHAR slot_check);
	INT(*config_advance_slot_num_check)(struct DWC_ETH_QOS_prv_data *,
										UINT qInx, UCHAR adv_slot_check);
#ifdef DWC_ETH_QOS_CONFIG_PGTEST
	void(*tx_desc_init_pg)(struct DWC_ETH_QOS_prv_data *, UINT qInx);
	void(*rx_desc_init_pg)(struct DWC_ETH_QOS_prv_data *, UINT qInx);
	INT(*set_ch_arb_weights)(struct DWC_ETH_QOS_prv_data *, UINT qInx, UCHAR weight);
	INT(*config_slot_interrupt)(struct DWC_ETH_QOS_prv_data *, UINT qInx, UCHAR config);
	INT(*set_slot_count)(struct DWC_ETH_QOS_prv_data *, UINT qInx, UCHAR slotCount);
	INT(*set_tx_rx_prio_policy)(struct DWC_ETH_QOS_prv_data *, UCHAR prio_policy);
	INT(*set_tx_rx_prio)(struct DWC_ETH_QOS_prv_data *, UCHAR prio);
	INT(*set_tx_rx_prio_ratio)(struct DWC_ETH_QOS_prv_data *, UCHAR prio_ratio);
	INT(*set_dma_tx_arb_algorithm)(struct DWC_ETH_QOS_prv_data *, UCHAR arb_algo);
	INT(*prepare_dev_pktgen)(struct DWC_ETH_QOS_prv_data *);
#endif /* end of DWC_ETH_QOS_CONFIG_PGTEST */

	/* for hw time stamping */
	INT(*config_hw_time_stamping)(struct DWC_ETH_QOS_prv_data *, UINT);
	INT(*config_sub_second_increment)(struct DWC_ETH_QOS_prv_data *, unsigned long ptp_clock);
	INT(*init_systime)(struct DWC_ETH_QOS_prv_data *, UINT, UINT);
	INT(*config_addend)(struct DWC_ETH_QOS_prv_data *, UINT);
	INT(*adjust_systime)(struct DWC_ETH_QOS_prv_data *, UINT, UINT, INT, bool);
	ULONG_LONG(*get_systime)(struct DWC_ETH_QOS_prv_data *);
	UINT(*get_tx_tstamp_status)(struct s_TX_NORMAL_DESC *txdesc);
	ULONG_LONG(*get_tx_tstamp)(struct s_TX_NORMAL_DESC *txdesc);
	UINT(*get_tx_tstamp_status_via_reg)(struct DWC_ETH_QOS_prv_data *);
	ULONG_LONG(*get_tx_tstamp_via_reg)(struct DWC_ETH_QOS_prv_data *);
	UINT(*rx_tstamp_available)(struct s_RX_NORMAL_DESC *rxdesc);
	UINT(*get_rx_tstamp_status)(struct s_RX_CONTEXT_DESC *rxdesc);
	ULONG_LONG(*get_rx_tstamp)(struct s_RX_CONTEXT_DESC *rxdesc);
	INT(*drop_tx_status_enabled)(struct DWC_ETH_QOS_prv_data *);

	/* for l2, l3 and l4 layer filtering */
	INT(*config_l2_da_perfect_inverse_match)(struct DWC_ETH_QOS_prv_data *,
			INT perfect_inverse_match);
	INT(*update_mac_addr32_127_low_high_reg)(struct DWC_ETH_QOS_prv_data *,
			INT idx, UCHAR addr[]);
	INT(*update_mac_addr1_31_low_high_reg)(struct DWC_ETH_QOS_prv_data *,
										   INT idx, UCHAR addr[]);
	INT(*update_hash_table_reg)(struct DWC_ETH_QOS_prv_data *,
								INT idx, UINT data);
	INT(*config_mac_pkt_filter_reg)(struct DWC_ETH_QOS_prv_data *,
									UCHAR, UCHAR, UCHAR, UCHAR, UCHAR);
	INT(*config_l3_l4_filter_enable)(struct DWC_ETH_QOS_prv_data *, INT);
	INT(*config_l3_filters)(struct DWC_ETH_QOS_prv_data *,
							INT filter_no, INT enb_dis, INT ipv4_ipv6_match,
							INT src_dst_addr_match, INT perfect_inverse_match);
	INT(*update_ip4_addr0)(struct DWC_ETH_QOS_prv_data *, INT filter_no, UCHAR addr[]);
	INT(*update_ip4_addr1)(struct DWC_ETH_QOS_prv_data *, INT filter_no, UCHAR addr[]);
	INT(*update_ip6_addr)(struct DWC_ETH_QOS_prv_data *, INT filter_no, USHORT addr[]);
	INT(*config_l4_filters)(struct DWC_ETH_QOS_prv_data *,
							INT filter_no, INT enb_dis,
							INT tcp_udp_match, INT src_dst_port_match,
							INT perfect_inverse_match);
	INT(*update_l4_sa_port_no)(struct DWC_ETH_QOS_prv_data *, INT filter_no, USHORT port_no);
	INT(*update_l4_da_port_no)(struct DWC_ETH_QOS_prv_data *, INT filter_no, USHORT port_no);

	/* for VLAN filtering */
	INT(*get_vlan_hash_table_reg)(struct DWC_ETH_QOS_prv_data *);
	INT(*update_vlan_hash_table_reg)(struct DWC_ETH_QOS_prv_data *, USHORT data);
	INT(*update_vlan_id)(struct DWC_ETH_QOS_prv_data *, USHORT vid);
	INT(*config_vlan_filtering)(struct DWC_ETH_QOS_prv_data *,
								INT filter_enb_dis,
								INT perfect_hash_filtering,
								INT perfect_inverse_match);
	INT(*config_mac_for_vlan_pkt)(struct DWC_ETH_QOS_prv_data *);
	UINT(*get_vlan_tag_comparison)(struct DWC_ETH_QOS_prv_data *);

	/* for differnet PHY interconnect */
	INT(*control_an)(struct DWC_ETH_QOS_prv_data *, bool enable, bool restart);
	INT(*get_an_adv_pause_param)(struct DWC_ETH_QOS_prv_data *);
	INT(*get_an_adv_duplex_param)(struct DWC_ETH_QOS_prv_data *);
	INT(*get_lp_an_adv_pause_param)(struct DWC_ETH_QOS_prv_data *);
	INT(*get_lp_an_adv_duplex_param)(struct DWC_ETH_QOS_prv_data *);

	/* for EEE */
	INT(*set_eee_mode)(struct DWC_ETH_QOS_prv_data *);
	INT(*reset_eee_mode)(struct DWC_ETH_QOS_prv_data *);
	INT(*set_eee_pls)(struct DWC_ETH_QOS_prv_data *, int phy_link);
	INT(*set_eee_timer)(struct DWC_ETH_QOS_prv_data *, int lpi_lst, int lpi_twt);
	u32(*get_lpi_status)(struct DWC_ETH_QOS_prv_data *);
	INT(*set_lpi_tx_automate)(struct DWC_ETH_QOS_prv_data *);

	/* for ARP */
	INT(*config_arp_offload)(struct DWC_ETH_QOS_prv_data *, int enb_dis);
	INT(*update_arp_offload_ip_addr)(struct DWC_ETH_QOS_prv_data *, UCHAR addr[]);

	/* for MAC loopback */
	INT(*config_mac_loopback_mode)(struct DWC_ETH_QOS_prv_data *, UINT);

	/* for MAC Double VLAN Processing config */
	INT(*config_tx_outer_vlan)(struct DWC_ETH_QOS_prv_data *, UINT op_type, UINT outer_vlt);
	INT(*config_tx_inner_vlan)(struct DWC_ETH_QOS_prv_data *, UINT op_type, UINT inner_vlt);
	INT(*config_svlan)(struct DWC_ETH_QOS_prv_data *, UINT);
	VOID(*config_dvlan)(struct DWC_ETH_QOS_prv_data *, bool enb_dis);
	VOID(*config_rx_outer_vlan_stripping)(struct DWC_ETH_QOS_prv_data *, u32);
	VOID(*config_rx_inner_vlan_stripping)(struct DWC_ETH_QOS_prv_data *, u32);

	/* for PFC */
	void (*config_pfc)(struct DWC_ETH_QOS_prv_data *, int enb_dis);

	/* for PTP offloading */
	VOID(*config_ptpoffload_engine)(struct DWC_ETH_QOS_prv_data *, UINT, UINT);

	VOID(*nvt_sys_init)(struct DWC_ETH_QOS_prv_data *);

#ifdef DWC_ETH_QOS_ENABLE_IRQ_MOD
	/* for tx/rx irq moderation */
	INT(*enable_tx_irq_mod)(struct DWC_ETH_QOS_prv_data *);
	INT(*enable_rx_irq_mod)(struct DWC_ETH_QOS_prv_data *);
	INT(*set_irq_mod_mode)(struct DWC_ETH_QOS_prv_data *);
	INT(*clear_tx_irq_mod_status)(struct DWC_ETH_QOS_prv_data *);
	INT(*get_tx_irq_mod_status)(struct DWC_ETH_QOS_prv_data *);
	INT(*clear_rx_irq_mod_status)(struct DWC_ETH_QOS_prv_data *);
	INT(*get_rx_irq_mod_status)(struct DWC_ETH_QOS_prv_data *);
	INT(*config_tx_irq_mod_timer_limit)(struct DWC_ETH_QOS_prv_data *, UINT);
	INT(*config_rx_irq_mod_timer_limit)(struct DWC_ETH_QOS_prv_data *, UINT);
#endif

};

/* wrapper buffer structure to hold transmit pkt details */
struct DWC_ETH_QOS_tx_buffer {
	dma_addr_t dma;     /* dma address of skb */
	struct sk_buff *skb;    /* virtual address of skb */
	unsigned short len; /* length of first skb */
	unsigned char buf1_mapped_as_page;

	dma_addr_t dma2; /* dam address of second skb */
	unsigned short len2; /* length of second skb */
	unsigned char buf2_mapped_as_page;

#ifdef DWC_ETH_QOS_CONFIG_PGTEST
	unsigned char slot_number;
#endif
};

struct DWC_ETH_QOS_tx_wrapper_descriptor {
	char *desc_name;    /* ID of descriptor */

	void *tx_desc_ptrs[TX_DESC_CNT];
	dma_addr_t tx_desc_dma_addrs[TX_DESC_CNT];

	struct DWC_ETH_QOS_tx_buffer *tx_buf_ptrs[TX_DESC_CNT];

	unsigned char contigous_mem;

	int cur_tx; /* always gives index of desc which has to
                be used for current xfer */
	int dirty_tx;   /* always gives index of desc which has to
                be checked for xfer complete */
	unsigned int free_desc_cnt; /* always gives total number of available
                    free desc count for driver */
	unsigned int tx_pkt_queued; /* always gives total number of packets
                    queued for transmission */
	unsigned int queue_stopped;
	int packet_count;

	UINT tx_threshold_val;  /* contain bit value for TX threshold */
	UINT tsf_on;        /* set to 1 if TSF is enabled else set to 0 */
	UINT osf_on;        /* set to 1 if OSF is enabled else set to 0 */
	UINT tx_pbl;

	/* for tx vlan delete/insert/replace */
	u32 tx_vlan_tag_via_reg;
	u32 tx_vlan_tag_ctrl;

	USHORT vlan_tag_id;
	UINT vlan_tag_present;

	/* for VLAN context descriptor operation */
	u32 context_setup;

	/* for TSO */
	u32 default_mss;
};

struct DWC_ETH_QOS_tx_queue {
	/* Tx descriptors */
	struct napi_struct napi;
	struct DWC_ETH_QOS_tx_wrapper_descriptor tx_desc_data;
	struct DWC_ETH_QOS_prv_data *pdata;
	int q_op_mode;
	unsigned int tse_mode;
	unsigned int qInx;
	unsigned int algorithm;
	unsigned int cc;
	unsigned int idle_slope;
	unsigned int send_slope;
	unsigned int hi_credit;
	unsigned int low_credit;
	INT tx_irq;
	INT tx_irq_flag;
	INT tx_qInx;
	spinlock_t tx_dma_lock;
};

/* wrapper buffer structure to hold received pkt details */
struct DWC_ETH_QOS_rx_buffer {
	dma_addr_t dma;     /* dma address of skb */
	struct sk_buff *skb;    /* virtual address of skb */
	unsigned short len; /* length of received packet */
	struct page *page;  /* page address */
	unsigned char mapped_as_page;
	bool good_pkt;      /* set to 1 if it is good packet else
                set to 0 */
	unsigned int inte;  /* set to non-zero if INTE is set for
                corresponding desc */

	dma_addr_t dma2;    /* dma address of second skb */
	struct page *page2; /* page address of second buffer */
	unsigned short len2;    /* length of received packet-second buffer */

	unsigned short rx_hdr_size; /* header buff size in case of split header */
};

struct DWC_ETH_QOS_rx_wrapper_descriptor {
	char *desc_name;    /* ID of descriptor */

	void *rx_desc_ptrs[RX_DESC_CNT];
	dma_addr_t rx_desc_dma_addrs[RX_DESC_CNT];

	struct DWC_ETH_QOS_rx_buffer *rx_buf_ptrs[RX_DESC_CNT];

	unsigned char contigous_mem;

	int cur_rx; /* always gives index of desc which needs to
                be checked for packet availabilty */
	int dirty_rx;
	unsigned int pkt_received;  /* always gives total number of packets
                    received from device in one RX interrupt */
	unsigned int skb_realloc_idx;
	unsigned int skb_realloc_threshold;

	/* for rx coalesce schem */
	int use_riwt;       /* set to 1 if RX watchdog timer should be used
                for RX interrupt mitigation */
	u32 rx_riwt;
	u32 rx_coal_frames; /* Max no of pkts to be received before
                an RX interrupt */

	UINT rx_threshold_val;  /* contain bit vlaue for RX threshold */
	UINT rsf_on;        /* set to 1 if RSF is enabled else set to 0 */
	UINT rx_pbl;

	struct sk_buff *skb_top;    /* points to first skb in the chain
                    in case of jumbo pkts */

	/* for rx vlan stripping */
	u32 rx_inner_vlan_strip;
	u32 rx_outer_vlan_strip;
};

struct DWC_ETH_QOS_rx_queue {
	/* Rx descriptors */
	struct DWC_ETH_QOS_rx_wrapper_descriptor rx_desc_data;
	struct napi_struct napi;
	struct DWC_ETH_QOS_prv_data *pdata;
	INT rx_irq;
	INT rx_irq_flag;
	INT qInx_rx ;
	spinlock_t rx_dma_lock;
};

struct desc_if_struct {
	INT(*alloc_queue_struct)(struct DWC_ETH_QOS_prv_data *);
	void(*free_queue_struct)(struct DWC_ETH_QOS_prv_data *);
	INT(*alloc_buff_and_desc)(struct DWC_ETH_QOS_prv_data *);
	void (*realloc_skb)(struct DWC_ETH_QOS_prv_data *, UINT);
	void (*unmap_rx_skb)(struct DWC_ETH_QOS_prv_data *,
						 struct DWC_ETH_QOS_rx_buffer *);
	void (*unmap_tx_skb)(struct DWC_ETH_QOS_prv_data *,
						 struct DWC_ETH_QOS_tx_buffer *);
	unsigned int (*map_tx_skb)(struct net_device *, struct sk_buff *);
	void (*tx_free_mem)(struct DWC_ETH_QOS_prv_data *);
	void (*rx_free_mem)(struct DWC_ETH_QOS_prv_data *);
	void (*wrapper_tx_desc_init)(struct DWC_ETH_QOS_prv_data *);
	void (*wrapper_tx_desc_init_single_q)(struct DWC_ETH_QOS_prv_data *,
										  UINT);
	void (*wrapper_rx_desc_init)(struct DWC_ETH_QOS_prv_data *);
	void (*wrapper_rx_desc_init_single_q)(struct DWC_ETH_QOS_prv_data *,
										  UINT);

	void (*rx_skb_free_mem)(struct DWC_ETH_QOS_prv_data *, UINT);
	void (*rx_skb_free_mem_single_q)(struct DWC_ETH_QOS_prv_data *, UINT);
	void (*tx_skb_free_mem)(struct DWC_ETH_QOS_prv_data *, UINT);
	void (*tx_skb_free_mem_single_q)(struct DWC_ETH_QOS_prv_data *, UINT);

	int (*handle_tso)(struct net_device *dev, struct sk_buff *skb);
};

/*
 * This structure contains different flags and each flags will indicates
 * different hardware features.
 */
struct DWC_ETH_QOS_hw_features {
	/* HW Feature Register0 */
	unsigned int mii_sel;   /* 10/100 Mbps support */
	unsigned int gmii_sel;  /* 1000 Mbps support */
	unsigned int hd_sel;    /* Half-duplex support */
	unsigned int pcs_sel;   /* PCS registers(TBI, SGMII or RTBI PHY
                interface) */
	unsigned int vlan_hash_en;  /* VLAN Hash filter selected */
	unsigned int sma_sel;   /* SMA(MDIO) Interface */
	unsigned int rwk_sel;   /* PMT remote wake-up packet */
	unsigned int mgk_sel;   /* PMT magic packet */
	unsigned int mmc_sel;   /* RMON module */
	unsigned int arp_offld_en;  /* ARP Offload features is selected */
	unsigned int ts_sel;    /* IEEE 1588-2008 Adavanced timestamp */
	unsigned int eee_sel;   /* Energy Efficient Ethernet is enabled */
	unsigned int tx_coe_sel;    /* Tx Checksum Offload is enabled */
	unsigned int rx_coe_sel;    /* Rx Checksum Offload is enabled */
	unsigned int mac_addr16_sel;    /* MAC Addresses 1-16 are selected */
	unsigned int mac_addr32_sel;    /* MAC Addresses 32-63 are selected */
	unsigned int mac_addr64_sel;    /* MAC Addresses 64-127 are selected */
	unsigned int tsstssel;  /* Timestamp System Time Source */
	unsigned int speed_sel; /* Speed Select */
	unsigned int sa_vlan_ins;   /* Source Address or VLAN Insertion */
	unsigned int act_phy_sel;   /* Active PHY Selected */

	/* HW Feature Register1 */
	unsigned int rx_fifo_size;  /* MTL Receive FIFO Size */
	unsigned int tx_fifo_size;  /* MTL Transmit FIFO Size */
	unsigned int adv_ts_hword;  /* Advance timestamping High Word
                    selected */
	unsigned int dcb_en;    /* DCB Feature Enable */
	unsigned int sph_en;    /* Split Header Feature Enable */
	unsigned int tso_en;    /* TCP Segmentation Offload Enable */
	unsigned int dma_debug_gen; /* DMA debug registers are enabled */
	unsigned int av_sel;    /* AV Feature Enabled */
	unsigned int lp_mode_en;    /* Low Power Mode Enabled */
	unsigned int hash_tbl_sz;   /* Hash Table Size */
	unsigned int l3l4_filter_num;   /* Total number of L3-L4 Filters */

	/* HW Feature Register2 */
	unsigned int rx_q_cnt;  /* Number of MTL Receive Queues */
	unsigned int tx_q_cnt;  /* Number of MTL Transmit Queues */
	unsigned int rx_ch_cnt; /* Number of DMA Receive Channels */
	unsigned int tx_ch_cnt; /* Number of DMA Transmit Channels */
	unsigned int pps_out_num;   /* Number of PPS outputs */
	unsigned int aux_snap_num;  /* Number of Auxillary snapshot
                    inputs */
};

/* structure to hold MMC values */
struct DWC_ETH_QOS_mmc_counters {
	/* MMC TX counters */
	unsigned long mmc_tx_octetcount_gb;
	unsigned long mmc_tx_framecount_gb;
	unsigned long mmc_tx_broadcastframe_g;
	unsigned long mmc_tx_multicastframe_g;
	unsigned long mmc_tx_64_octets_gb;
	unsigned long mmc_tx_65_to_127_octets_gb;
	unsigned long mmc_tx_128_to_255_octets_gb;
	unsigned long mmc_tx_256_to_511_octets_gb;
	unsigned long mmc_tx_512_to_1023_octets_gb;
	unsigned long mmc_tx_1024_to_max_octets_gb;
	unsigned long mmc_tx_unicast_gb;
	unsigned long mmc_tx_multicast_gb;
	unsigned long mmc_tx_broadcast_gb;
	unsigned long mmc_tx_underflow_error;
	unsigned long mmc_tx_singlecol_g;
	unsigned long mmc_tx_multicol_g;
	unsigned long mmc_tx_deferred;
	unsigned long mmc_tx_latecol;
	unsigned long mmc_tx_exesscol;
	unsigned long mmc_tx_carrier_error;
	unsigned long mmc_tx_octetcount_g;
	unsigned long mmc_tx_framecount_g;
	unsigned long mmc_tx_excessdef;
	unsigned long mmc_tx_pause_frame;
	unsigned long mmc_tx_vlan_frame_g;
	unsigned long mmc_tx_osize_frame_g;

	/* MMC RX counters */
	unsigned long mmc_rx_framecount_gb;
	unsigned long mmc_rx_octetcount_gb;
	unsigned long mmc_rx_octetcount_g;
	unsigned long mmc_rx_broadcastframe_g;
	unsigned long mmc_rx_multicastframe_g;
	unsigned long mmc_rx_crc_errror;
	unsigned long mmc_rx_align_error;
	unsigned long mmc_rx_run_error;
	unsigned long mmc_rx_jabber_error;
	unsigned long mmc_rx_undersize_g;
	unsigned long mmc_rx_oversize_g;
	unsigned long mmc_rx_64_octets_gb;
	unsigned long mmc_rx_65_to_127_octets_gb;
	unsigned long mmc_rx_128_to_255_octets_gb;
	unsigned long mmc_rx_256_to_511_octets_gb;
	unsigned long mmc_rx_512_to_1023_octets_gb;
	unsigned long mmc_rx_1024_to_max_octets_gb;
	unsigned long mmc_rx_unicast_g;
	unsigned long mmc_rx_length_error;
	unsigned long mmc_rx_outofrangetype;
	unsigned long mmc_rx_pause_frames;
	unsigned long mmc_rx_fifo_overflow;
	unsigned long mmc_rx_vlan_frames_gb;
	unsigned long mmc_rx_watchdog_error;
	unsigned long mmc_rx_receive_error;
	unsigned long mmc_rx_ctrl_frames_g;

	/* IPC */
	unsigned long mmc_rx_ipc_intr_mask;
	unsigned long mmc_rx_ipc_intr;

	/* IPv4 */
	unsigned long mmc_rx_ipv4_gd;
	unsigned long mmc_rx_ipv4_hderr;
	unsigned long mmc_rx_ipv4_nopay;
	unsigned long mmc_rx_ipv4_frag;
	unsigned long mmc_rx_ipv4_udsbl;

	/* IPV6 */
	unsigned long mmc_rx_ipv6_gd_octets;
	unsigned long mmc_rx_ipv6_hderr_octets;
	unsigned long mmc_rx_ipv6_nopay_octets;

	/* Protocols */
	unsigned long mmc_rx_udp_gd;
	unsigned long mmc_rx_udp_err;
	unsigned long mmc_rx_tcp_gd;
	unsigned long mmc_rx_tcp_err;
	unsigned long mmc_rx_icmp_gd;
	unsigned long mmc_rx_icmp_err;

	/* IPv4 */
	unsigned long mmc_rx_ipv4_gd_octets;
	unsigned long mmc_rx_ipv4_hderr_octets;
	unsigned long mmc_rx_ipv4_nopay_octets;
	unsigned long mmc_rx_ipv4_frag_octets;
	unsigned long mmc_rx_ipv4_udsbl_octets;

	/* IPV6 */
	unsigned long mmc_rx_ipv6_gd;
	unsigned long mmc_rx_ipv6_hderr;
	unsigned long mmc_rx_ipv6_nopay;

	/* Protocols */
	unsigned long mmc_rx_udp_gd_octets;
	unsigned long mmc_rx_udp_err_octets;
	unsigned long mmc_rx_tcp_gd_octets;
	unsigned long mmc_rx_tcp_err_octets;
	unsigned long mmc_rx_icmp_gd_octets;
	unsigned long mmc_rx_icmp_err_octets;
};

struct DWC_ETH_QOS_extra_stats {
	unsigned long q_re_alloc_rx_buf_failed[8];

	/* Tx/Rx IRQ error info */
	unsigned long tx_process_stopped_irq_n[8];
	unsigned long rx_process_stopped_irq_n[8];
	unsigned long tx_buf_unavailable_irq_n[8];
	unsigned long rx_buf_unavailable_irq_n[8];
	unsigned long rx_watchdog_irq_n;
	unsigned long fatal_bus_error_irq_n;
	unsigned long pmt_irq_n;
	/* Tx/Rx IRQ Events */
	unsigned long tx_normal_irq_n[8];
	unsigned long rx_normal_irq_n[8];
	unsigned long napi_poll_n;
	unsigned long tx_clean_n[8];
	/* EEE */
	unsigned long tx_path_in_lpi_mode_irq_n;
	unsigned long tx_path_exit_lpi_mode_irq_n;
	unsigned long rx_path_in_lpi_mode_irq_n;
	unsigned long rx_path_exit_lpi_mode_irq_n;
	/* Tx/Rx frames */
	unsigned long tx_pkt_n;
	unsigned long rx_pkt_n;
	unsigned long tx_vlan_pkt_n;
	unsigned long rx_vlan_pkt_n;
	unsigned long tx_timestamp_captured_n;
	unsigned long rx_timestamp_captured_n;
	unsigned long tx_tso_pkt_n;
	unsigned long rx_split_hdr_pkt_n;

	/* Tx/Rx frames per channels/queues */
	unsigned long q_tx_pkt_n[8];
	unsigned long q_rx_pkt_n[8];
	unsigned long q_tx_stop_times_n[8];
};

struct DWC_ETH_QOS_prv_data {
	struct net_device *dev;
	struct platform_device *pdev;

	struct timer_list timer;
	spinlock_t rx_napi_lock;

	spinlock_t lock;
	spinlock_t tx_lock;
	spinlock_t pmt_lock;
	spinlock_t rx_int_lock;
	struct mutex ethtool_lock;
#ifdef CONFIG_MDIO_INT_MODE
	struct mutex mdio_idle;
	struct completion mdio_intr;
#endif
	UINT mem_start_addr;
	UINT mem_size;
	INT irq_number;
	struct hw_if_struct hw_if;
	struct desc_if_struct desc_if;

	struct s_tx_error_counters tx_error_counters;
	struct s_rx_error_counters rx_error_counters;
	struct s_rx_pkt_features rx_pkt_features;
	struct s_tx_pkt_features tx_pkt_features;

	/* TX Queue */
	struct DWC_ETH_QOS_tx_queue *tx_queue;
	UCHAR tx_queue_cnt;
	UINT tx_qInx;


	/* RX Queue */
	struct DWC_ETH_QOS_rx_queue *rx_queue;
	UCHAR rx_queue_cnt;
	UINT rx_qInx;


#ifdef DWC_ETH_QOS_CACHE_ACC
	int skb_assign_ptr;
	int skb_alloc_ptr;
	int clean_status; /* 0:ok, bit0=1:top fail, bit1=1:bottom fail */
	int skb_init;
	struct sk_buff *skb[SKB_DESC_CNT * 2];
#endif

	struct mii_bus *mii;
	struct phy_device *phydev;
	struct device_node *phy_node;
	int oldlink;
	int speed;
	int oldduplex;
	int phyaddr;
	int bus_id;
	u64 dev_state;
	u32 interface;

	int ep_id;

	/* saving state for Wake-on-LAN */
	int wolopts;

	/* Helper macros for handling FLOW control in HW */
#define DWC_ETH_QOS_FLOW_CTRL_OFF 0
#define DWC_ETH_QOS_FLOW_CTRL_RX  1
#define DWC_ETH_QOS_FLOW_CTRL_TX  2
#define DWC_ETH_QOS_FLOW_CTRL_TX_RX (DWC_ETH_QOS_FLOW_CTRL_TX |\
									 DWC_ETH_QOS_FLOW_CTRL_RX)

	unsigned int flow_ctrl;

	/* keeps track of previous programmed flow control options */
	unsigned int oldflow_ctrl;

	struct DWC_ETH_QOS_hw_features hw_feat;

	/* for sa(source address) insert/replace */
	u32 tx_sa_ctrl_via_desc;
	u32 tx_sa_ctrl_via_reg;
	unsigned char mac_addr[DWC_ETH_QOS_MAC_ADDR_LEN];

	/* keeps track of power mode for API based PMT control */
	u32 power_down;
	u32 power_down_type;

	/* AXI parameters */
	UINT incr_incrx;
	UINT axi_pbl;
	UINT axi_worl;
	UINT axi_rorl;

	/* for hanlding jumbo frames and split header feature on rx path */
	int (*clean_rx)(struct DWC_ETH_QOS_prv_data *pdata, int quota, UINT qInx);
	int (*alloc_rx_buf)(struct DWC_ETH_QOS_prv_data *pdata,
						struct DWC_ETH_QOS_rx_buffer *buffer, gfp_t gfp);
	unsigned int rx_buffer_len;

	/* variable frame burst size */
	UINT drop_tx_pktburstcnt;
	unsigned int mac_enable_count;  /* counter for enabling MAC transmit at
                    drop tx packet  */

	struct DWC_ETH_QOS_mmc_counters mmc;
	struct DWC_ETH_QOS_extra_stats xstats;

#ifdef DWC_ETH_QOS_CONFIG_PGTEST
	struct DWC_ETH_QOS_PGStruct *pg;
	struct timer_list pg_timer;
	INT prepare_pg_packet;
	INT run_test;
	INT max_counter;
#endif /* end of DWC_ETH_QOS_CONFIG_PGTEST */

	/* rx split header mode */
	unsigned char rx_split_hdr;

	/* for MAC loopback */
	unsigned int mac_loopback_mode;

	/* for hw time stamping */
	unsigned char hwts_tx_en;
	unsigned char hwts_rx_en;
	struct ptp_clock *ptp_clock;
	struct ptp_clock_info ptp_clock_ops;
	spinlock_t ptp_lock; /* protects registers */
	unsigned int default_addend;
	bool one_nsec_accuracy; /* set to 1 if one nano second accuracy
                   is enabled else set to zero */

	/* for filtering */
	int max_hash_table_size;
	int max_addr_reg_cnt;

	/* L3/L4 filtering */
	unsigned int l3_l4_filter;

	unsigned char vlan_hash_filtering;
	unsigned int l2_filtering_mode; /* 0 - if perfect and 1 - if hash filtering */

	/* For handling PCS(TBI/RTBI/SGMII) and RGMII/SMII interface */
	unsigned int pcs_link;
	unsigned int pcs_duplex;
	unsigned int pcs_speed;
	unsigned int pause;
	unsigned int duplex;
	unsigned int lp_pause;
	unsigned int lp_duplex;

	/* for handling EEE */
	struct timer_list eee_ctrl_timer;
	bool tx_path_in_lpi_mode;
	bool use_lpi_tx_automate;
	int eee_enabled;
	int eee_active;
	int tx_lpi_timer;

	/* arp offload enable/disable. */
	u32 arp_offload;

	/* set to 1 when ptp offload is enabled, else 0. */
	u32 ptp_offload;
	/* ptp offloading mode - ORDINARY_SLAVE, ORDINARY_MASTER,
	 * TRANSPARENT_SLAVE, TRANSPARENT_MASTER, PTOP_TRANSPERENT.
	 * */
	u32 ptp_offloading_mode;


	/* For configuring double VLAN via descriptor/reg */
	int inner_vlan_tag;
	int outer_vlan_tag;
	/* op_type will be
	 * 0/1/2/3 for none/delet/insert/replace respectively
	 * */
	int op_type;
	/* in_out will be
	 * 1/2/3 for outer/inner/both respectively.
	 * */
	int in_out;
	/* 0 for via registers and 1 for via descriptor */
	int via_reg_or_desc;



	/* this variable will hold vlan table value if vlan hash filtering
	 * is enabled else hold vlan id that is programmed in HW. Same is
	 * used to configure back into HW when device is reset during
	 * jumbo/split-header features.
	 * */
	UINT vlan_ht_or_id;

	/* Used when LRO is enabled,
	 * set to 1 if skb has TCP payload else set to 0
	 * */
	int tcp_pkt;

#if defined(CONFIG_NVT_IVOT_PLAT_NA51103)
	struct proc_dir_entry	*pproc_module_root;
	struct proc_dir_entry   *pproc_eth_swing;
#endif
	int embd_phy;

	struct work_struct restart_work;
	int qInx;

#ifdef DWC_ETH_QOS_ENABLE_IRQ_MOD
	unsigned long tx_irq_mod_lim;
	unsigned long rx_irq_mod_lim;
#endif
#ifdef DWC_ETH_QOS_CONFIG_DEBUGFS
	struct DWC_ETH_QOS_dbg_data dbg_data;
#endif
	/* for pm suspend, resume */
	UINT control_reg;			// 0x3004
	UINT phy_led_signal_reg;		// 0x300c
	UINT io_control_reg;			// 0x3014
	UINT sbd_int_mask_reg;			// 0x3028
	UINT axi_addr_manual_mode_reg;		// 0x302c
	UINT dma_control_reg;			// 0x3040
	UINT monitor_channel_status_reg;	// 0x3044
	UINT interrupt_moderation_control_reg;	// 0x3048
	UINT tx_irq_moderation_timer_limit_reg;	// 0x304c
	UINT rx_irq_moderation_timer_limit_reg;	// 0x3050

	char *ext_phy_clk_name;
	char *ptp_clk_name;
	int spclk_en;
};

typedef enum {
	eSAVE,
	eRESTORE
} e_int_state;

/* Variable prototypes*/
//extern void __iomem *ETH_QOS_REG_BASE;

/* Function prototypes*/

void DWC_ETH_QOS_init_function_ptrs_dev(struct hw_if_struct *);
void DWC_ETH_QOS_init_function_ptrs_desc(struct desc_if_struct *);
struct net_device_ops *DWC_ETH_QOS_get_netdev_ops(void);
struct ethtool_ops *DWC_ETH_QOS_get_ethtool_ops(void);
int DWC_ETH_QOS_poll_mq(struct napi_struct *, int);
int DWC_ETH_QOS_rx_poll_mq(struct napi_struct *, int);
int DWC_ETH_QOS_tx_poll_mq(struct napi_struct *, int);

//void DWC_ETH_QOS_get_pdata(struct DWC_ETH_QOS_prv_data *pdata);

/* Debugfs related functions. */
int create_debug_files(struct DWC_ETH_QOS_prv_data *pdata);
void remove_debug_files(struct DWC_ETH_QOS_prv_data *pdata);

int DWC_ETH_QOS_mdio_register(struct net_device *dev, int chipid);
void DWC_ETH_QOS_mdio_unregister(struct net_device *dev);
INT DWC_ETH_QOS_mdio_read_direct(struct DWC_ETH_QOS_prv_data *pdata,
								 int phyaddr, int phyreg, int *phydata);
INT DWC_ETH_QOS_mdio_write_direct(struct DWC_ETH_QOS_prv_data *pdata,
								  int phyaddr, int phyreg, int phydata);
void dbgpr_regs(void);
void dump_phy_registers(struct DWC_ETH_QOS_prv_data *);
void dump_tx_desc(struct DWC_ETH_QOS_prv_data *pdata, int first_desc_idx,
				  int last_desc_idx, int flag, UINT qInx);
void dump_rx_desc(UINT, struct s_RX_NORMAL_DESC *desc, int cur_rx);
void print_pkt(struct sk_buff *skb, int len, bool tx_rx, int desc_idx);
void DWC_ETH_QOS_get_all_hw_features(struct DWC_ETH_QOS_prv_data *pdata);
void DWC_ETH_QOS_print_all_hw_features(struct DWC_ETH_QOS_prv_data *pdata);
void DWC_ETH_QOS_configure_flow_ctrl(struct DWC_ETH_QOS_prv_data *pdata);
INT DWC_ETH_QOS_powerup(struct net_device *, UINT);
INT DWC_ETH_QOS_powerdown(struct net_device *, UINT, UINT);
u32 DWC_ETH_QOS_usec2riwt(u32 usec, struct DWC_ETH_QOS_prv_data *pdata);
void DWC_ETH_QOS_init_rx_coalesce(struct DWC_ETH_QOS_prv_data *pdata);
void DWC_ETH_QOS_enable_all_ch_dma_interrpt(struct DWC_ETH_QOS_prv_data *pdata);
void DWC_ETH_QOS_disable_all_ch_dma_interrpt(struct DWC_ETH_QOS_prv_data *pdata);

void DWC_ETH_QOS_enable_rx_ch_dma_interrpt(struct DWC_ETH_QOS_prv_data *pdata, UINT qInx);
void DWC_ETH_QOS_disable_rx_ch_dma_interrpt(struct DWC_ETH_QOS_prv_data *pdata, UINT qInx);
void DWC_ETH_QOS_enable_tx_ch_dma_interrpt(struct DWC_ETH_QOS_prv_data *pdata, UINT qInx);
void DWC_ETH_QOS_disable_tx_ch_dma_interrpt(struct DWC_ETH_QOS_prv_data *pdata, UINT qInx);
#if defined(CONFIG_NVT_IVOT_PLAT_NA51103) || defined(CONFIG_NVT_IVOT_PLAT_NS02302) || defined(CONFIG_NVT_IVOT_PLAT_NA51102) || defined(CONFIG_NVT_IVOT_PLAT_NA51055)
int nvt_eth_env_probe(struct platform_device *pdev, void __iomem **p_base, int *embd_en);
#if defined(CONFIG_NVT_IVOT_PLAT_NA51103)
int DWC_ETH_QOS_restart_device(struct DWC_ETH_QOS_prv_data *pdata);
int nvt_eth_proc_init(struct DWC_ETH_QOS_prv_data *pdata, int idx);
int nvt_eth_proc_remove(struct DWC_ETH_QOS_prv_data *pdata);
#endif
#else
int nvt_eth_env_probe(struct platform_device *pdev, void __iomem **p_base);
#endif
void DWC_ETH_QOS_update_rx_errors(struct net_device *, unsigned int);
void DWC_ETH_QOS_stop_all_ch_tx_dma(struct DWC_ETH_QOS_prv_data *pdata);
UCHAR get_tx_queue_count(struct DWC_ETH_QOS_prv_data *pdata);
UCHAR get_rx_queue_count(struct DWC_ETH_QOS_prv_data *pdata);
UCHAR probe_get_tx_queue_count(unsigned long base);
UCHAR probe_get_rx_queue_count(unsigned long base);
void DWC_ETH_QOS_mmc_read(struct DWC_ETH_QOS_prv_data *pdata, struct DWC_ETH_QOS_mmc_counters *mmc);
UINT DWC_ETH_QOS_get_total_desc_cnt(struct DWC_ETH_QOS_prv_data *pdata,
									struct sk_buff *skb, UINT qInx);

int DWC_ETH_QOS_ptp_init(struct DWC_ETH_QOS_prv_data *pdata);
void DWC_ETH_QOS_ptp_remove(struct DWC_ETH_QOS_prv_data *pdata);
phy_interface_t DWC_ETH_QOS_get_phy_interface(struct DWC_ETH_QOS_prv_data *pdata);
#ifdef DWC_ETH_QOS_CONFIG_EEE
bool DWC_ETH_QOS_eee_init(struct DWC_ETH_QOS_prv_data *pdata);
void DWC_ETH_QOS_handle_eee_interrupt(struct DWC_ETH_QOS_prv_data *pdata);
void DWC_ETH_QOS_disable_eee_mode(struct DWC_ETH_QOS_prv_data *pdata);
void DWC_ETH_QOS_enable_eee_mode(struct DWC_ETH_QOS_prv_data *pdata);
#endif

int DWC_ETH_QOS_drv_resume(struct net_device *dev);
int DWC_ETH_QOS_drv_suspend(struct net_device *dev);
#ifdef CONFIG_PM
void nvt_eth_get_clk_name(struct DWC_ETH_QOS_prv_data *pdata, int idx_mac);
#endif
#ifdef DWC_ETH_QOS_CONFIG_PGTEST
irqreturn_t DWC_ETH_QOS_ISR_SW_DWC_ETH_QOS_pg(int irq, void *device_id);
void DWC_ETH_QOS_default_confs(struct DWC_ETH_QOS_prv_data *pdata);
int DWC_ETH_QOS_handle_pg_ioctl(struct DWC_ETH_QOS_prv_data *pdata, void *ptr);
int DWC_ETH_QOS_alloc_pg(struct DWC_ETH_QOS_prv_data *pdata);
void DWC_ETH_QOS_free_pg(struct DWC_ETH_QOS_prv_data *pdata);
int DWC_ETH_QOS_alloc_rx_buf_pg(struct DWC_ETH_QOS_prv_data *pdata,
								struct DWC_ETH_QOS_rx_buffer *buffer,
								gfp_t gfp);
int DWC_ETH_QOS_alloc_tx_buf_pg(struct DWC_ETH_QOS_prv_data *pdata,
								struct DWC_ETH_QOS_tx_buffer *buffer,
								gfp_t gfp);
#endif /* end of DWC_ETH_QOS_CONFIG_PGTEST */

#if IS_ENABLED(CONFIG_PCI)
nvtpcie_chipid_t nvt_eth_dts_chipid(struct device_node *np);
int nvt_eth_ep_pinmux_capture(PIN_GROUP_CONFIG *pinmux_config, struct device_node *np);
#endif
#ifdef TIMER_CONTROL
void DWC_ETH_QOS_I_monitor_func(struct timer_list *arg);
#endif

#ifdef DWC_ETH_QOS_ENABLE_IRQ_MOD
int nvt_irq_mod_sysfs_init(struct net_device *ndev);
void nvt_irq_mod_sysfs_remove(struct net_device *ndev);
#endif

int nvt_eth_sysfs_init(struct net_device *ndev);
void nvt_eth_sysfs_remove(struct net_device *ndev);

/* For debug prints*/
#define DRV_NAME "DWC_ETH_QOS_drv.c"

#ifdef YDEBUG
#define DBGPR(x...) printk(KERN_ALERT x)
#define DBGPR_REGS() dbgpr_regs()
#define DBGPHY_REGS(x...) dump_phy_registers(x)
#else
#define DBGPR(x...) do { } while (0)
#define DBGPR_REGS() do { } while (0)
#define DBGPHY_REGS(x...) do { } while (0)
#endif

#ifdef YDEBUG_PG
#define DBGPR_PG(x...) printk(KERN_ALERT x)
#else
#define DBGPR_PG(x...) do {} while (0)
#endif

#ifdef YDEBUG_MDIO
#define DBGPR_MDIO(x...) printk(KERN_ALERT x)
#else
#define DBGPR_MDIO(x...) do {} while (0)
#endif

#ifdef YDEBUG_PTP
#define DBGPR_PTP(x...) printk(KERN_ALERT x)
#else
#define DBGPR_PTP(x...) do {} while (0)
#endif

#ifdef YDEBUG_FILTER
#define DBGPR_FILTER(x...) printk(KERN_ALERT x)
#else
#define DBGPR_FILTER(x...) do {} while (0)
#endif

#ifdef YDEBUG_EEE
#define DBGPR_EEE(x...) printk(KERN_ALERT x)
#else
#define DBGPR_EEE(x...) do {} while (0)
#endif

#endif
