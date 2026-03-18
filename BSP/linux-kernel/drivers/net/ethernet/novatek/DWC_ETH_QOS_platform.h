/*
 * Copyright (c) 2020, NovaTek Inc. All rights reserved.
 */

#ifndef __DWC_ETH_QOS_PLATFORM__
#define __DWC_ETH_QOS_PLATFORM__

#ifdef CONFIG_NVT_IVOT_PLAT_NA51102
/* ---------------------NA51102-------------------------- */

#define DRV_VERSION     "1.00.04"

#define DWC_ETH_QOS_SYSCLOCK        30000000 /* System clock is 30MHz */
#define DWC_ETH_QOS_SYSTIMEPERIOD   33 /* System time period is 33.33ns */

#ifdef CONFIG_NVT_FPGA_EMULATION
#define DWC_ETH_QOS_PTPSYSCLOCK     20000000ULL /* System clock is 20MHz */
#define DWC_ETH_QOS_PTPINPUTCLOCK   48000000ULL /* System clock is 48MHz */
#else
#define DWC_ETH_QOS_PTPSYSCLOCK     50000000ULL /* System clock is 50MHz */
#define DWC_ETH_QOS_PTPINPUTCLOCK   480000000ULL /* System clock is 480MHz */
#endif

/* Helper macro for handling coalesce parameters via ethtool */
/* Obtained by trial and error  */
#ifdef DWC_ETH_QOS_RT_LINUX
#define DWC_ETH_QOS_OPTIMAL_DMA_RIWT_USEC  200
#else
#define DWC_ETH_QOS_OPTIMAL_DMA_RIWT_USEC  790
#endif

/* Max delay before RX interrupt after a pkt is received Max
 * delay in usecs is 17467 for 33.33MHz device clock */
#define DWC_ETH_QOS_MAX_DMA_RIWT  0x7ff

/* Max no of pkts to be received before an RX interrupt */
#if (128 > CONFIG_RX_DESC)
#define DWC_ETH_QOS_RX_MAX_FRAMES CONFIG_RX_DESC
#else
#define DWC_ETH_QOS_RX_MAX_FRAMES (CONFIG_RX_DESC/2)
#endif

/* Max no of pkts to be received before an TX interrupt */
#if IS_ENABLED(CONFIG_MULTI_IRQ) || IS_ENABLED(CONFIG_NVT_FPGA_EMULATION)
#define DWC_ETH_QOS_TX_MAX_FRAMES 1
#else
#define DWC_ETH_QOS_TX_MAX_FRAMES (CONFIG_TX_DESC/8)
#endif

#define DMA_SBUS_AXI_PBL_MASK 0xFE
/* AXI Maximum Outstanding Request Limit*/
#define DMA_SBUS_RD_OSR 0x7
#define DMA_SBUS_WR_OSR 0x7

#define NVT_IRQ_MOD_TMR_100MS 0x16b4df4
#define NVT_IRQ_MOD_TMR_5MS 0x122a4c

#define CLK_EN_REG 0x70
#define ETH_CLK_EN BIT(27)
#define SRAM_DOWN_CTL_REG 0X1000
#define ETH_SRAM_SD BIT(14)

#elif defined(CONFIG_NVT_IVOT_PLAT_NS02201)
/* ---------------------NS02201-------------------------- */

#define DRV_VERSION     "1.00.28"

#define DWC_ETH_QOS_SYSCLOCK        30000000 /* System clock is 30MHz */
#define DWC_ETH_QOS_SYSTIMEPERIOD   33 /* System time period is 33.33ns */

#ifdef CONFIG_NVT_FPGA_EMULATION
#define DWC_ETH_QOS_PTPSYSCLOCK     20000000ULL /* System clock is 20MHz */
#define DWC_ETH_QOS_PTPINPUTCLOCK   48000000ULL /* System clock is 48MHz */
#else
#define DWC_ETH_QOS_PTPSYSCLOCK     50000000ULL /* System clock is 50MHz */
#define DWC_ETH_QOS_PTPINPUTCLOCK   480000000ULL /* System clock is 480MHz */
#endif

/* Helper macro for handling coalesce parameters via ethtool */
/* Obtained by trial and error  */
#ifdef DWC_ETH_QOS_RT_LINUX
#define DWC_ETH_QOS_OPTIMAL_DMA_RIWT_USEC  200
#else
#define DWC_ETH_QOS_OPTIMAL_DMA_RIWT_USEC  790
#endif

/* Max delay before RX interrupt after a pkt is received Max
 * delay in usecs is 17467 for 33.33MHz device clock */
#define DWC_ETH_QOS_MAX_DMA_RIWT  0x7ff

/* Max no of pkts to be received before an RX interrupt */
#if (128 > CONFIG_RX_DESC)
#define DWC_ETH_QOS_RX_MAX_FRAMES CONFIG_RX_DESC
#else
#define DWC_ETH_QOS_RX_MAX_FRAMES (CONFIG_RX_DESC/2)
#endif

/* Max no of pkts to be received before an TX interrupt */
#if IS_ENABLED(CONFIG_MULTI_IRQ) || IS_ENABLED(CONFIG_NVT_FPGA_EMULATION)
#define DWC_ETH_QOS_TX_MAX_FRAMES 1
#else
#define DWC_ETH_QOS_TX_MAX_FRAMES (CONFIG_TX_DESC/8)
#endif

#define DMA_SBUS_AXI_PBL_MASK 0xFE
/* AXI Maximum Outstanding Request Limit*/
#define DMA_SBUS_RD_OSR 0x7
#define DMA_SBUS_WR_OSR 0x7

#define NVT_IRQ_MOD_TMR_100MS 0x16b4df4
#define NVT_IRQ_MOD_TMR_5MS 0x122a4c

#define CLK_EN_REG 0x70
#define ETH_CLK_EN BIT(27)
#define SRAM_DOWN_CTL_REG 0X1000
#define ETH_SRAM_SD BIT(27)

#elif defined(CONFIG_NVT_IVOT_PLAT_NA51055)
/* ---------------------NA51055-------------------------- */

#define DRV_VERSION     "1.00.04"

#define DWC_ETH_QOS_SYSCLOCK        62500000 /* System clock is 62.5MHz */
#define DWC_ETH_QOS_SYSTIMEPERIOD   16 /* System time period is 16ns */

#ifdef CONFIG_NVT_FPGA_EMULATION
#define DWC_ETH_QOS_PTPSYSCLOCK     20000000ULL /* System clock is 20MHz */
#define DWC_ETH_QOS_PTPINPUTCLOCK   48000000ULL /* System clock is 48MHz */
#else
#define DWC_ETH_QOS_PTPSYSCLOCK     50000000ULL /* System clock is 50MHz */
#define DWC_ETH_QOS_PTPINPUTCLOCK   480000000ULL /* System clock is 480MHz */
#endif

/* Helper macro for handling coalesce parameters via ethtool */
/* Obtained by trial and error  */
#ifdef DWC_ETH_QOS_RT_LINUX
#define DWC_ETH_QOS_OPTIMAL_DMA_RIWT_USEC  124
#else
#define DWC_ETH_QOS_OPTIMAL_DMA_RIWT_USEC  124
#endif

/* Max delay before RX interrupt after a pkt is received Max
 * delay in usecs is 17467 for 33.33MHz device clock */
#define DWC_ETH_QOS_MAX_DMA_RIWT  0xff

/* Max no of pkts to be received before an RX interrupt */
#if (128 > CONFIG_RX_DESC)
#define DWC_ETH_QOS_RX_MAX_FRAMES CONFIG_RX_DESC
#else
#define DWC_ETH_QOS_RX_MAX_FRAMES (CONFIG_RX_DESC/2)
#endif

/* Max no of pkts to be received before an TX interrupt */
#if IS_ENABLED(CONFIG_MULTI_IRQ) || IS_ENABLED(CONFIG_NVT_FPGA_EMULATION)
#define DWC_ETH_QOS_TX_MAX_FRAMES 1
#else
#define DWC_ETH_QOS_TX_MAX_FRAMES (CONFIG_TX_DESC/8)
#endif

#define DMA_SBUS_AXI_PBL_MASK 0xFE
/* AXI Maximum Outstanding Request Limit*/
#define DMA_SBUS_RD_OSR 0x3
#define DMA_SBUS_WR_OSR 0x3

#define NVT_IRQ_MOD_TMR_100MS 0x16b4df4
#define NVT_IRQ_MOD_TMR_5MS 0x122a4c

#define CLK_EN_REG 0x74
#define ETH_CLK_EN BIT(29)
#define SRAM_DOWN_CTL_REG 0X1000
#define ETH_SRAM_SD BIT(18)

#elif defined(CONFIG_NVT_IVOT_PLAT_NA51089)
/* ---------------------NA51089-------------------------- */

#define DRV_VERSION     "1.00.02"

#define DWC_ETH_QOS_SYSCLOCK        15000000 /* System clock is 62.5MHz */
#define DWC_ETH_QOS_SYSTIMEPERIOD   67 /* System time period is 16ns */

#ifdef CONFIG_NVT_FPGA_EMULATION
#define DWC_ETH_QOS_PTPSYSCLOCK     20000000ULL /* System clock is 20MHz */
#define DWC_ETH_QOS_PTPINPUTCLOCK   48000000ULL /* System clock is 48MHz */
#else
#define DWC_ETH_QOS_PTPSYSCLOCK     50000000ULL /* System clock is 50MHz */
#define DWC_ETH_QOS_PTPINPUTCLOCK   480000000ULL /* System clock is 480MHz */
#endif

/* Helper macro for handling coalesce parameters via ethtool */
/* Obtained by trial and error  */
#ifdef DWC_ETH_QOS_RT_LINUX
#define DWC_ETH_QOS_OPTIMAL_DMA_RIWT_USEC  124
#else
#define DWC_ETH_QOS_OPTIMAL_DMA_RIWT_USEC  124
#endif

/* Max delay before RX interrupt after a pkt is received Max
 * delay in usecs is 17467 for 33.33MHz device clock */
#define DWC_ETH_QOS_MAX_DMA_RIWT  0xff

/* Max no of pkts to be received before an RX interrupt */
#if (128 > CONFIG_RX_DESC)
#define DWC_ETH_QOS_RX_MAX_FRAMES CONFIG_RX_DESC
#else
#define DWC_ETH_QOS_RX_MAX_FRAMES (CONFIG_RX_DESC/2)
#endif

/* Max no of pkts to be received before an TX interrupt */
#if IS_ENABLED(CONFIG_MULTI_IRQ) || IS_ENABLED(CONFIG_NVT_FPGA_EMULATION)
#define DWC_ETH_QOS_TX_MAX_FRAMES 1
#else
#define DWC_ETH_QOS_TX_MAX_FRAMES (CONFIG_TX_DESC/32)
#endif

#define DMA_SBUS_AXI_PBL_MASK 0xFE
/* AXI Maximum Outstanding Request Limit*/
#define DMA_SBUS_RD_OSR 0x3
#define DMA_SBUS_WR_OSR 0x3

#define NVT_IRQ_MOD_TMR_100MS 0x16b4df4
#define NVT_IRQ_MOD_TMR_5MS 0x122a4c

#define CLK_EN_REG 0x74
#define ETH_CLK_EN BIT(29)
#define SRAM_DOWN_CTL_REG 0X1000
#define ETH_SRAM_SD BIT(18)

#elif defined(CONFIG_NVT_IVOT_PLAT_NS02302)
/* ---------------------NS02302-------------------------- */

#define DRV_VERSION     "1.00.09"

#define DWC_ETH_QOS_SYSCLOCK        30000000 /* System clock is 30MHz */
#define DWC_ETH_QOS_SYSTIMEPERIOD   33 /* System time period is 33.33ns */

#ifdef CONFIG_NVT_FPGA_EMULATION
#define DWC_ETH_QOS_PTPSYSCLOCK     20000000ULL /* System clock is 20MHz */
#define DWC_ETH_QOS_PTPINPUTCLOCK   48000000ULL /* System clock is 48MHz */
#else
#define DWC_ETH_QOS_PTPSYSCLOCK     50000000ULL /* System clock is 50MHz */
#define DWC_ETH_QOS_PTPINPUTCLOCK   480000000ULL /* System clock is 480MHz */
#endif

/* Helper macro for handling coalesce parameters via ethtool */
/* Obtained by trial and error  */
#ifdef DWC_ETH_QOS_RT_LINUX
#define DWC_ETH_QOS_OPTIMAL_DMA_RIWT_USEC  200
#else
#define DWC_ETH_QOS_OPTIMAL_DMA_RIWT_USEC  1348
#endif

/* Max delay before RX interrupt after a pkt is received Max
 * delay in usecs is 17467 for 33.33MHz device clock */
#define DWC_ETH_QOS_MAX_DMA_RIWT  0xffff

/* Max no of pkts to be received before an RX interrupt */
#if (128 > CONFIG_RX_DESC)
#define DWC_ETH_QOS_RX_MAX_FRAMES CONFIG_RX_DESC
#else
#define DWC_ETH_QOS_RX_MAX_FRAMES (CONFIG_RX_DESC/2)
#endif

/* Max no of pkts to be received before an TX interrupt */
#if IS_ENABLED(CONFIG_MULTI_IRQ) || IS_ENABLED(CONFIG_NVT_FPGA_EMULATION)
#define DWC_ETH_QOS_TX_MAX_FRAMES 1
#else
#define DWC_ETH_QOS_TX_MAX_FRAMES (CONFIG_TX_DESC/8)
#endif

#define DMA_SBUS_AXI_PBL_MASK 0xFE
/* AXI Maximum Outstanding Request Limit*/
#define DMA_SBUS_RD_OSR 0x3
#define DMA_SBUS_WR_OSR 0x3

//XXX: Need to cabrate at real-chip
#define NVT_IRQ_MOD_TMR_100MS 0x122a4c
#define NVT_IRQ_MOD_TMR_5MS 0xe883

#define CLK_EN_REG 0x74
#define ETH_CLK_EN BIT(29)
#define SRAM_DOWN_CTL_REG 0X1000
#define ETH_SRAM_SD BIT(12)

#elif defined(CONFIG_NVT_IVOT_PLAT_NS02301)
/* ---------------------NS02301-------------------------- */

#define DRV_VERSION     "1.00.06"

#define DWC_ETH_QOS_SYSCLOCK        30000000 /* System clock is 30MHz */
#define DWC_ETH_QOS_SYSTIMEPERIOD   33 /* System time period is 33.33ns */

#ifdef CONFIG_NVT_FPGA_EMULATION
#define DWC_ETH_QOS_PTPSYSCLOCK     20000000ULL /* System clock is 20MHz */
#define DWC_ETH_QOS_PTPINPUTCLOCK   48000000ULL /* System clock is 48MHz */
#else
#define DWC_ETH_QOS_PTPSYSCLOCK     50000000ULL /* System clock is 50MHz */
#define DWC_ETH_QOS_PTPINPUTCLOCK   480000000ULL /* System clock is 480MHz */
#endif

/* Helper macro for handling coalesce parameters via ethtool */
/* Obtained by trial and error  */
#ifdef DWC_ETH_QOS_RT_LINUX
#define DWC_ETH_QOS_OPTIMAL_DMA_RIWT_USEC  200
#else
#define DWC_ETH_QOS_OPTIMAL_DMA_RIWT_USEC  400
#endif

/* Max delay before RX interrupt after a pkt is received Max
 * delay in usecs is 17467 for 33.33MHz device clock */
#define DWC_ETH_QOS_MAX_DMA_RIWT  0xff
/* AXI Maximum Outstanding Request Limit*/
#define DMA_SBUS_RD_OSR 0x3
#define DMA_SBUS_WR_OSR 0x3

/* Max no of pkts to be received before an RX interrupt */
#if (128 > CONFIG_RX_DESC)
#define DWC_ETH_QOS_RX_MAX_FRAMES CONFIG_RX_DESC
#else
#define DWC_ETH_QOS_RX_MAX_FRAMES (CONFIG_RX_DESC/2)
#endif

/* Max no of pkts to be received before an TX interrupt */
#if IS_ENABLED(CONFIG_MULTI_IRQ) || IS_ENABLED(CONFIG_NVT_FPGA_EMULATION)
#define DWC_ETH_QOS_TX_MAX_FRAMES 1
#else
#define DWC_ETH_QOS_TX_MAX_FRAMES (CONFIG_TX_DESC/8)
#endif

#define DMA_SBUS_AXI_PBL_MASK 0xFE

//XXX: Need to cabrate at real-chip
#define NVT_IRQ_MOD_TMR_100MS 0x122a4c
#define NVT_IRQ_MOD_TMR_5MS 0xe883

#define CLK_EN_REG 0x74
#define ETH_CLK_EN BIT(29)
#define SRAM_DOWN_CTL_REG 0X1000
#define ETH_SRAM_SD BIT(0)

#elif defined(CONFIG_NVT_IVOT_PLAT_NA51090)
/* ---------------------NA51090-------------------------- */

#define DRV_VERSION     "1.00.02"

#define DWC_ETH_QOS_SYSCLOCK        30000000 /* System clock is 30MHz */
#define DWC_ETH_QOS_SYSTIMEPERIOD   33 /* System time period is 33.33ns */

#ifdef CONFIG_NVT_FPGA_EMULATION
#define DWC_ETH_QOS_PTPSYSCLOCK     20000000ULL /* System clock is 20MHz */
#define DWC_ETH_QOS_PTPINPUTCLOCK   48000000ULL /* System clock is 48MHz */
#else
#define DWC_ETH_QOS_PTPSYSCLOCK     50000000ULL /* System clock is 50MHz */
#define DWC_ETH_QOS_PTPINPUTCLOCK   480000000ULL /* System clock is 480MHz */
#endif

#ifdef CONFIG_NVT_PCIE_LIB
/* For sync 5.10 and 4.19.148 mac support pci code base */
#define CONFIG_PCI 1
#endif
/* Helper macro for handling coalesce parameters via ethtool */
/* Obtained by trial and error  */
#ifdef DWC_ETH_QOS_RT_LINUX
#define DWC_ETH_QOS_OPTIMAL_DMA_RIWT_USEC  200
#else
#define DWC_ETH_QOS_OPTIMAL_DMA_RIWT_USEC  790
#endif

/* Max delay before RX interrupt after a pkt is received Max
 * delay in usecs is 17467 for 33.33MHz device clock */
#define DWC_ETH_QOS_MAX_DMA_RIWT  0x7ff

/* Max no of pkts to be received before an RX interrupt */
#if (128 > CONFIG_RX_DESC)
#define DWC_ETH_QOS_RX_MAX_FRAMES CONFIG_RX_DESC
#else
#define DWC_ETH_QOS_RX_MAX_FRAMES (CONFIG_RX_DESC/2)
#endif

/* Max no of pkts to be received before an TX interrupt */
#if IS_ENABLED(CONFIG_MULTI_IRQ) || IS_ENABLED(CONFIG_NVT_FPGA_EMULATION)
#define DWC_ETH_QOS_TX_MAX_FRAMES 1
#else
#define DWC_ETH_QOS_TX_MAX_FRAMES (CONFIG_TX_DESC/8)
#endif

#define DMA_SBUS_AXI_PBL_MASK 0xFE
/* AXI Maximum Outstanding Request Limit*/
#define DMA_SBUS_RD_OSR 0x7
#define DMA_SBUS_WR_OSR 0x7

#define NVT_IRQ_MOD_TMR_100MS 0x16b4df4
#define NVT_IRQ_MOD_TMR_5MS 0x122a4c

#define CLK_EN_REG 0x70
#define ETH_CLK_EN BIT(27)
#define SRAM_DOWN_CTL_REG 0X1000
#define ETH_SRAM_SD BIT(9)

#elif defined(CONFIG_NVT_IVOT_PLAT_NA51103)
/* ---------------------NA51103-------------------------- */

#define DRV_VERSION     "1.00.01"

#define DWC_ETH_QOS_SYSCLOCK        30000000 /* System clock is 30MHz */
#define DWC_ETH_QOS_SYSTIMEPERIOD   33 /* System time period is 33.33ns */

#ifdef CONFIG_NVT_FPGA_EMULATION
#define DWC_ETH_QOS_PTPSYSCLOCK     20000000ULL /* System clock is 20MHz */
#define DWC_ETH_QOS_PTPINPUTCLOCK   48000000ULL /* System clock is 48MHz */
#else
#define DWC_ETH_QOS_PTPSYSCLOCK     50000000ULL /* System clock is 50MHz */
#define DWC_ETH_QOS_PTPINPUTCLOCK   480000000ULL /* System clock is 480MHz */
#endif

/* Helper macro for handling coalesce parameters via ethtool */
/* Obtained by trial and error  */
#ifdef DWC_ETH_QOS_RT_LINUX
#define DWC_ETH_QOS_OPTIMAL_DMA_RIWT_USEC  200
#else
#define DWC_ETH_QOS_OPTIMAL_DMA_RIWT_USEC  688
#endif

/* Max delay before RX interrupt after a pkt is received Max
 * delay in usecs is 17467 for 33.33MHz device clock */
#define DWC_ETH_QOS_MAX_DMA_RIWT  0x7ff

/* Max no of pkts to be received before an RX interrupt */
#if (128 > CONFIG_RX_DESC)
#define DWC_ETH_QOS_RX_MAX_FRAMES CONFIG_RX_DESC
#else
#define DWC_ETH_QOS_RX_MAX_FRAMES (CONFIG_RX_DESC/2)
#endif

/* Max no of pkts to be received before an TX interrupt */
#if IS_ENABLED(CONFIG_MULTI_IRQ) || IS_ENABLED(CONFIG_NVT_FPGA_EMULATION)
#define DWC_ETH_QOS_TX_MAX_FRAMES 1
#else
#define DWC_ETH_QOS_TX_MAX_FRAMES (CONFIG_TX_DESC/8)
#endif

#define DMA_SBUS_AXI_PBL_MASK 0xFE
/* AXI Maximum Outstanding Request Limit*/
#define DMA_SBUS_RD_OSR 0x7
#define DMA_SBUS_WR_OSR 0x7

#define NVT_IRQ_MOD_TMR_100MS 0x16b4df4
#define NVT_IRQ_MOD_TMR_5MS 0x122a4c

#define CLK_EN_REG 0x70
#define ETH_CLK_EN BIT(27)
#define SRAM_DOWN_CTL_REG 0X1000
#define ETH_SRAM_SD BIT(9)

#else
/* ---------------------default-------------------------- */

#define DRV_VERSION     "1.00.00"

#define DWC_ETH_QOS_SYSCLOCK        30000000 /* System clock is 30MHz */
#define DWC_ETH_QOS_SYSTIMEPERIOD   33 /* System time period is 33.33ns */

#ifdef CONFIG_NVT_FPGA_EMULATION
#define DWC_ETH_QOS_PTPSYSCLOCK     20000000ULL /* System clock is 20MHz */
#define DWC_ETH_QOS_PTPINPUTCLOCK   48000000ULL /* System clock is 48MHz */
#else
#define DWC_ETH_QOS_PTPSYSCLOCK     50000000ULL /* System clock is 50MHz */
#define DWC_ETH_QOS_PTPINPUTCLOCK   480000000ULL /* System clock is 480MHz */
#endif

/* Helper macro for handling coalesce parameters via ethtool */
/* Obtained by trial and error  */
#ifdef DWC_ETH_QOS_RT_LINUX
#define DWC_ETH_QOS_OPTIMAL_DMA_RIWT_USEC  200
#else
#define DWC_ETH_QOS_OPTIMAL_DMA_RIWT_USEC  790
#endif

/* Max delay before RX interrupt after a pkt is received Max
 * delay in usecs is 17467 for 33.33MHz device clock */
#define DWC_ETH_QOS_MAX_DMA_RIWT  0x7ff

/* Max no of pkts to be received before an RX interrupt */
#if (128 > CONFIG_RX_DESC)
#define DWC_ETH_QOS_RX_MAX_FRAMES CONFIG_RX_DESC

#else
#define DWC_ETH_QOS_RX_MAX_FRAMES (CONFIG_RX_DESC/2)
#endif

/* Max no of pkts to be received before an TX interrupt */
#if IS_ENABLED(CONFIG_MULTI_IRQ) || IS_ENABLED(CONFIG_NVT_FPGA_EMULATION)
#define DWC_ETH_QOS_TX_MAX_FRAMES 1
#else
#define DWC_ETH_QOS_TX_MAX_FRAMES (CONFIG_TX_DESC/8)
#endif
#define DMA_SBUS_AXI_PBL_MASK 0xFE
/* AXI Maximum Outstanding Request Limit*/
#define DMA_SBUS_RD_OSR 0x3
#define DMA_SBUS_WR_OSR 0x3

#define NVT_IRQ_MOD_TMR_100MS 0x16b4df4
#define NVT_IRQ_MOD_TMR_5MS 0x122a4c

#define CLK_EN_REG 0x74
#define ETH_CLK_EN BIT(29)
#define SRAM_DOWN_CTL_REG 0X1000
#define ETH_SRAM_SD BIT(12)

#endif
#endif
