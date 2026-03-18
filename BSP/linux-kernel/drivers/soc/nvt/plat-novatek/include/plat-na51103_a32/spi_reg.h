
/*
	Register offset and bit definition for SPI module

	Register offset and bit definition for SPI module.

	@file       spi_reg.h
	@ingroup    
	@note

	Copyright   Novatek Microelectronics Corp. 2018.  All rights reserved
	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License version 2 as
	published by the Free Software Foundation.
*/
#ifndef _SPI_REG_H
#define _SPI_REG_H

#include <linux/soc/nvt/rcw_macro.h>

#define SPI_GYRO_OP_INTERVAL_MAX        (0x1FFFFF)
#define SPI_GYRO_TRANSFER_INTERVAL_MAX  (0x1FFFFF)

#define SPI_GYRO_TRANSFER_MIN           (1)
#define SPI_GYRO_TRANSFER_MAX           (32)

#define SPI_GYRO_TRSLEN_MIN             (1)
#define SPI_GYRO_TRSLEN_MAX             (4)

#define SPI_GYRO_OPLEN_MIN              (1)
#define SPI_GYRO_OPLEN_MAX              (8)

#define SPI_GYRO_FIFO_THRESHOLD         (24)
#define SPI_GYRO_FIFO_DEPTH             (32)    // Max depth = 32 OP

#ifndef ENUM_DUMMY4WORD
#define ENUM_DUMMY4WORD(m)
#endif

/*
	SPI_LATCHDELAY_ENUM
*/
typedef enum
{
	SPI_LATCHDELAY_NORMAL,              //< Normal latch
	SPI_LATCHDELAY_1T,                  //< Delay latch 1T
	SPI_LATCHDELAY_2T,                  //< Delay latch 2T

	ENUM_DUMMY4WORD(SPI_LATCHDELAY_ENUM)
} SPI_LATCHDELAY_ENUM;

/*
	SPI_PKT_LEN_ENUM
*/
typedef enum
{
	SPI_PKT_LEN_ENUM_1BYTE,             //< 1 byte
	SPI_PKT_LEN_ENUM_2BYTES,            //< 2 byte

	ENUM_DUMMY4WORD(SPI_PKT_LEN_ENUM)
} SPI_PKT_LEN_ENUM;

/*
	SPI_PKT_CNT_ENUM
*/
typedef enum
{
	SPI_PKT_CNT_ENUM_1PKT,              //< 1 packet
	SPI_PKT_CNT_ENUM_2PKT,              //< 2 packet
	SPI_PKT_CNT_ENUM_3PKT,              //< 3 packet (reserved)
	SPI_PKT_CNT_ENUM_4PKT,              //< 4 packet

	ENUM_DUMMY4WORD(SPI_PKT_CNT_ENUM)
} SPI_PKT_CNT_ENUM;

/*
	SPI_GYRO_MODE_ENUM
*/
typedef enum
{
	SPI_GYRO_MODE_ENUM_SIETRIG,         //< SIE VD trigger mode
	SPI_GYRO_MODE_ENUM_ONESHOT,         //< f/w trigger one-shot mode
	SPI_GYRO_MODE_ENUM_FREERUN,         //< f/w trigger free run mode

	ENUM_DUMMY4WORD(SPI_GYRO_MODE_ENUM)
} SPI_GYRO_MODE_ENUM;

/*
	SPI_CS_ACT_LVL_ENUM
*/
typedef enum
{
	SPI_CS_ACT_LVL_LOW,                 //< low active
	SPI_CS_ACT_LVL_HIGH,                //< high active

	ENUM_DUMMY4WORD(SPI_CS_ACT_LVL_ENUM)
} SPI_CS_ACT_LVL_ENUM;

#define SPI_CTRL_REG_OFS	0x00
union SPI_CTRL_REG {		     /* SPI Control Register */
	uint32_t reg;
	struct {
	unsigned int spi_en:1;		/* S/W must to take care this bit when disable SPI */
	unsigned int spics_value:1;	/* Force the value of SPICS ,0:Force to low, 1:Force to high*/
	unsigned int spi_dma_en:1;	/* Start DMA mode */
	unsigned int spi_dma_dis:1;	/* Abort DMA mode */
	unsigned int spi_rdsts_en:1;	/* Start read flash status */
	unsigned int spi_rdsts_dis:1;	/* Abort read flash status */
	unsigned int reserved0:1;	 
	unsigned int reserved1:1;	 
	unsigned int reserved2:1;	 
	unsigned int reserved3:7;
	unsigned int spi_dma_sts:1;	    /* DMA mode status */
	unsigned int spi_rdsts_sts:1;	/* Flash mode status */
	unsigned int reserved4:1;	 
	unsigned int reserved5:13;
	} bit;
};

#define SPI_IO_REG_OFS	0x04
union SPI_IO_REG {			/* SPI IO Register */
	uint32_t reg;
	struct {
	unsigned int reserved0:1;	 
	unsigned int reserved1:1;
	unsigned int spi_cpha:1;	/* SPI_CPHA Setting clock phase */
	unsigned int spi_cpol:1;	/* SPI_CPOL Setting clock polarity. */
	unsigned int reserved2:4;
	unsigned int spi_bus_width:2;	/* SPI bus width */
	unsigned int reserved3:2;
	unsigned int spi_io_out_en:1;	/* 0:SPI IO pins to input ,1:SPI IO pins to output */
	unsigned int spi_io_order:1;	/* setup bit order of IO0 IO1 IO2 IO3 */
	unsigned int reserved4:2;
	unsigned int spi_auto_io_out_en:1;	/* Set data pin to output mode when data is required to transmit */
	unsigned int reserved5:3;
	unsigned int spi_rdy_pol:1;	/* Polarity of SPI_RDY */
	unsigned int reserved6:11;
	} bit;
};

#define SPI_CONFIG_REG_OFS	0x08
union SPI_CONFIG_REG {   		/* SPI Configuration Register */
	uint32_t reg;
	struct {
	unsigned int spi_pkt_lsb_mode:1;/* LSb mode of each packet */
	unsigned int reserved0:1;
	unsigned int spi_pktlen:1;	/* spi_pktlen. 0: 1 byte, 1: 2 bytes */
	unsigned int reserved1:1;
	unsigned int spi_pkt_cnt:2;	/* packet count. 0: 1 pkt, 1: 2 pkt, , 2: 3 pkt, 3: 4 pkt */
	unsigned int spi_pkt_burst_handshake_en:1;	/* Enable handshake for each packet burst */
	unsigned int reserved2:5;
	unsigned int reserved3:2;	 
	unsigned int reserved4:2;
	unsigned int spi_pkt_burst_pre_cond:1;	/* Packet burst pre-condition */
	unsigned int reserved5:1;
	unsigned int spi_pkt_burst_post_cond:1;	/* Packet burst post-condition */
	unsigned int reserved6:13;
	} bit;
};

#define SPI_TIMING_REG_OFS	0x0C
union SPI_TIMING_REG {		/* SPI Timing Register */
	uint32_t reg;
	struct {
	unsigned int reserved0:8;	 
	unsigned int reserved1:8;
	unsigned int spi_post_cond_dly:13;	/* Packet burst post condition delay */
	unsigned int reserved2:3;
	} bit;
};

#define SPI_FLASH_CTRL_REG_OFS	0x10
union SPI_FLASH_CTRL_REG {		/* SPI Flash Control Register */
	uint32_t reg;
	struct {
	unsigned int spi_rdysts_bit:3;	/* Flash ready bit position */
	unsigned int reserved0:5;
	unsigned int spi_rdysts_val:1;	/* Flash ready value */
	unsigned int reserved1:7;
	unsigned int spi_rdsts_cmd:8;	/* Command byte to get status of serial flash */
	unsigned int reserved2:8;
	} bit;
};

#define SPI_DLY_CHAIN_REG_OFS	0x14
union SPI_DLY_CHAIN_REG {		/* SPI Delay Chain Register */
	uint32_t reg;
	struct {
	unsigned int reserved0:16;
	unsigned int latch_clk_shift:2;
	unsigned int reserved1:2;
	unsigned int latch_clk_edge:1;
	unsigned int reserved2:11;
	} bit;
};

#define SPI_STATUS_REG_OFS	0x18
union SPI_STATUS_REG {			/* SPI Status Register */
	uint32_t reg;
	struct {
	unsigned int spi_tdr_empty:1;	/* Transmit data register empty. */
	unsigned int spi_rdr_full:1;	/* Receive data register full. */
	unsigned int dma_abort_sts:1;
	unsigned int spi_dmaed:1;	/* 0: DMA not transfer/receive end , 1: DMA transfer/receive end */
	unsigned int spi_rdstsed:1;	/*0: SPI is reading status from serial flash or idle , 1: read SPI status done */
	unsigned int reserved0:1;
	unsigned int reserved1:1;
	unsigned int reserved2:1;
	unsigned int reserved3:1;
	unsigned int reserved4:1;
	unsigned int reserved5:22;
	} bit;
};

#define SPI_INTEN_REG_OFS	0x1C
union SPI_INTEN_REG {			/* SPI Status Register */
	uint32_t reg;
	struct {
	unsigned int reserved0:1;
	unsigned int spi_rdr_full:1;	/* Receive data register full.  */
	unsigned int reserved1:1;
	unsigned int spi_dmaed_en:1;	/* 0: DMA not transfer/receive end , 1: DMA transfer/receive end */
	unsigned int spi_rdstsed_en:1;	/* 0: SPI is reading status from serial flash or idle , 1: read SPI status done */
	unsigned int reserved2:1;
	unsigned int reserved3:1;
	unsigned int reserved4:1;
	unsigned int reserved5:1;
	unsigned int reserved6:1;
	unsigned int reserved7:22;
	} bit;
};

#define SPI_RDR_REG_OFS	0x20
union SPI_RDR_REG {		/* SPI Receive Data Register */
	uint32_t reg;
	struct {
	unsigned int rdr:32;	/* Receive data register */
	} bit;
};

#define SPI_TDR_REG_OFS	0x24
union SPI_TDR_REG {		/* SPI Transmit Data Register */
	uint32_t reg;
	struct {
	unsigned int tdr:32;	/* Transmit data register */
	} bit;
};

/* 0x28, 0x2C: reserved */

#define SPI_DMA_CTRL_REG_OFS	0x30
union SPI_DMA_CTRL_REG {	/* SPI DMA Control Register */
	uint32_t reg;
	struct {
	unsigned int dma_dir:1;	/* 0: SPI2DMA, 1: DMA2SPI */
	unsigned int reserved0:31;
	} bit;
};

#define SPI_DMA_BUFSIZE_REG_OFS	0x34
union SPI_DMA_BUFSIZE_REG {			/* SPI DMA Control Register */
	uint32_t reg;
	struct {
	unsigned int spi_dma_bufsize:24;	/* DMA buffer size register in byte alignment */
	unsigned int reserved0:8;
	} bit;
};

#define SPI_DMA_STARTADDR_REG_OFS	0x38
union SPI_DMA_STARTADDR_REG {			/* SPI DMA Control Register */
	uint32_t reg;
	struct {
	unsigned int spi_dma_start_addr:32;	/* DMA DRAM starting address */
	} bit;
};

/* 0x3C: reserved */

#endif
