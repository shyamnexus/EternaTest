#ifndef _SPI_REG_H
#define _SPI_REG_H

#include <rcw_macro.h>

#define SPI_GYRO_OP_INTERVAL_MAX        (0x1FFFFF)
#define SPI_GYRO_TRANSFER_INTERVAL_MAX  (0x1FFFFF)
#define SPI_GYRO_TRS_OFFSET_DELAY_MAX   (0x1FFFFF)

#define SPI_GYRO_TRANSFER_MIN           (1)
#define SPI_GYRO_TRANSFER_MAX           (96)  //96: pio backward compatible
#define SPI_GYRO_TRANSFER_MAX_DMA       (256) //256: dma
#define SPI_GYRO_TRANSFER_MAX_PIO       (96)  //96: pio

#define SPI_GYRO_TRSLEN_MIN             (1)
#define SPI_GYRO_TRSLEN_MAX             (4)

#define SPI_GYRO_OPLEN_MIN              (1)
#define SPI_GYRO_OPLEN_MAX              (8)

#define SPI_GYRO_FIFO_THRESHOLD         (50)    
#define SPI_GYRO_FIFO_DEPTH             (64)    // Max depth = 64 OP

/*
    SPI_LATCHDELAY_ENUM
*/
typedef enum {
	SPI_LATCHDELAY_NORMAL,              //< Normal latch
	SPI_LATCHDELAY_1T,                  //< Delay latch 1T
	SPI_LATCHDELAY_2T,                  //< Delay latch 2T
	ENUM_DUMMY4WORD(SPI_LATCHDELAY_ENUM)
} SPI_LATCHDELAY_ENUM;

/*
    SPI_PKT_LEN_ENUM
*/
typedef enum {
	SPI_PKT_LEN_ENUM_1BYTE,             //< 1 byte
	SPI_PKT_LEN_ENUM_2BYTES,            //< 2 byte
	ENUM_DUMMY4WORD(SPI_PKT_LEN_ENUM)
} SPI_PKT_LEN_ENUM;

/*
    SPI_PKT_CNT_ENUM
*/
typedef enum {
	SPI_PKT_CNT_ENUM_1PKT,              //< 1 packet
	SPI_PKT_CNT_ENUM_2PKT,              //< 2 packet
	SPI_PKT_CNT_ENUM_3PKT,              //< 3 packet (reserved)
	SPI_PKT_CNT_ENUM_4PKT,              //< 4 packet
	ENUM_DUMMY4WORD(SPI_PKT_CNT_ENUM)
} SPI_PKT_CNT_ENUM;

/*
    SPI_GYRO_MODE_ENUM
*/
typedef enum {
	SPI_GYRO_MODE_ENUM_NONE,            //< none
	SPI_GYRO_MODE_ENUM_SIETRIG,         //< SIE VD trigger mode
	SPI_GYRO_MODE_ENUM_ONESHOT,         //< f/w trigger one-shot mode
	SPI_GYRO_MODE_ENUM_FREERUN,         //< f/w trigger free run mode
	ENUM_DUMMY4WORD(SPI_GYRO_MODE_ENUM)
} SPI_GYRO_MODE_ENUM;

/*
    SPI_CS_ACT_LVL_ENUM
*/
typedef enum {
	SPI_CS_ACT_LVL_LOW,                 //< low active
	SPI_CS_ACT_LVL_HIGH,                //< high active
	ENUM_DUMMY4WORD(SPI_CS_ACT_LVL_ENUM)
} SPI_CS_ACT_LVL_ENUM;

#define SPI_CTRL_REG_OFS    0x00
REGDEF_BEGIN(SPI_CTRL_REG)            
REGDEF_BIT(spi_en, 1)                 /* S/W must to take care this bit when disable SPI */
REGDEF_BIT(spics_value, 1)            /* Force the value of SPICS ,0:Force to low, 1:Force to high*/
REGDEF_BIT(spi_dma_en, 1)             /* Start DMA mode */
REGDEF_BIT(spi_dma_dis, 1)            /* Abort DMA mode */
REGDEF_BIT(spi_rdsts_en, 1)           /* Start read flash status */
REGDEF_BIT(spi_rdsts_dis, 1)          /* Abort read flash status */
REGDEF_BIT(spi_gyro_en, 1)            /* Start GYRO mode */
REGDEF_BIT(spi_gyro_dis, 1)           /* Abort GYRO mode */
REGDEF_BIT(spi_gyro_update_even, 1)   /* Update GYRO settings on even-numbered VD */
REGDEF_BIT(spi_gyro_update_odd, 1)    /* Update GYRO settings on odd-numbered VD */
REGDEF_BIT(, 6)
REGDEF_BIT(spi_dma_sts, 1)            /* DMA mode status */
REGDEF_BIT(spi_rdsts_sts, 1)          /* Flash mode status */
REGDEF_BIT(spi_gyro_sts, 1)           /* Gyro mode status */
REGDEF_BIT(, 13)
REGDEF_END(SPI_CTRL_REG)               

#define SPI_IO_REG_OFS      0x04
REGDEF_BEGIN(SPI_IO_REG)               
REGDEF_BIT(spi_gyro_cs_pol, 1)        /* CS polarity in Gyro mode */
REGDEF_BIT(, 1)
REGDEF_BIT(spi_cpha, 1)               /* SPI_CPHA Setting clock phase */
REGDEF_BIT(spi_cpol, 1)               /* SPI_CPOL Setting clock polarity. */
REGDEF_BIT(, 4)
REGDEF_BIT(spi_bus_width, 2)          /* SPI bus width */
REGDEF_BIT(, 2)
REGDEF_BIT(spi_io_out_en, 1)          /* 0:SPI IO pins to input , 1:SPI IO pins to output(must set to 1 in full duplex mode) */
REGDEF_BIT(spi_io_order, 1)           /* setup bit order of IO0 IO1 IO2 IO3 */
REGDEF_BIT(, 2)
REGDEF_BIT(spi_auto_io_out_en, 1)     /* Set data pin to output mode when data is required to transmit */
REGDEF_BIT(, 3)
REGDEF_BIT(spi_rdy_pol, 1)            /* Polarity of SPI_RDY */
REGDEF_BIT(, 11)
REGDEF_END(SPI_IO_REG)          

#define SPI_CONFIG_REG_OFS  0x08
REGDEF_BEGIN(SPI_CONFIG_REG)    
REGDEF_BIT(spi_pkt_lsb_mode, 1)             /* LSb mode of each packet */
REGDEF_BIT(, 1)
REGDEF_BIT(spi_pktlen, 1)                   /* spi_pktlen. 0: 1 byte, 1: 2 bytes */
REGDEF_BIT(, 1)
REGDEF_BIT(spi_pkt_cnt, 2)                  /* packet count. 0: 1 pkt, 1: 2 pkt, , 2: 3 pkt, 3: 4 pkt */
REGDEF_BIT(spi_pkt_burst_handshake_en, 1)   /* Enable handshake for each packet burst */
REGDEF_BIT(spi_dma_path_sel, 1)             /* 0: normal DMA, 1: gyro DMA */
REGDEF_BIT(spi_gyro_offset_dly_en, 1)       /* enable bit for 0x114 */
REGDEF_BIT(, 3)
REGDEF_BIT(spi_gyro_mode, 2)                /* 0: none, 1: sie auto trig, 2: one-shot , 3: free run */
REGDEF_BIT(, 2)
REGDEF_BIT(spi_pkt_burst_pre_cond, 1)       /* Packet burst pre-condition */
REGDEF_BIT(, 1)
REGDEF_BIT(spi_pkt_burst_post_cond, 1)      /* Packet burst post-condition */
REGDEF_BIT(, 13)
REGDEF_END(SPI_CONFIG_REG)                 

#define SPI_TIMING_REG_OFS  0x0C
REGDEF_BEGIN(SPI_TIMING_REG)       
REGDEF_BIT(spi_cs_dly, 8)           /* delay count from SPI_CS asserted to SPI_CLK toggle */
REGDEF_BIT(, 8)
REGDEF_BIT(spi_post_cond_dly, 13)   /* Packet burst post condition delay */
REGDEF_BIT(, 3)
REGDEF_END(SPI_TIMING_REG)          

#define SPI_FLASH_CTRL_REG_OFS  0x10
REGDEF_BEGIN(SPI_FLASH_CTRL_REG) 
REGDEF_BIT(spi_rdysts_bit, 3)       /* Flash ready bit position */
REGDEF_BIT(, 5)
REGDEF_BIT(spi_rdysts_val, 1)       /* Flash ready value */
REGDEF_BIT(, 7)
REGDEF_BIT(spi_rdsts_cmd, 8)        /* Command byte to get status of serial flash */
REGDEF_BIT(, 8)
REGDEF_END(SPI_FLASH_CTRL_REG)  

#define SPI_DLY_CHAIN_REG_OFS   0x14
REGDEF_BEGIN(SPI_DLY_CHAIN_REG) 
REGDEF_BIT(, 16)                      /* delay how many delay cell to sample data */
REGDEF_BIT(latch_clk_shift, 2)
REGDEF_BIT(, 2)
REGDEF_BIT(latch_clk_edge, 1)
REGDEF_BIT(, 11)
REGDEF_END(SPI_DLY_CHAIN_REG)   

#define SPI_STATUS_REG_OFS  0x18
REGDEF_BEGIN(SPI_STATUS_REG)
REGDEF_BIT(spi_tdr_empty, 1)                   /* Transmit data register empty.  */
REGDEF_BIT(spi_rdr_full, 1)                    /* Receive data register full.  */
REGDEF_BIT(dma_tx_abort_sts, 1)                //538: dma_abort_sts
REGDEF_BIT(spi_dmaed, 1)                       /* 0: DMA not transfer/receive end , 1: DMA transfer/receive end */
REGDEF_BIT(spi_rdstsed, 1)                     /*0: SPI is reading status from serial flash or idle , 1: read SPI status done */
REGDEF_BIT(gyro_trs_rdy_sts, 1)
REGDEF_BIT(gyro_overrun_sts, 1)
REGDEF_BIT(all_gyrotrs_done_sts, 1)
REGDEF_BIT(gyro_seq_err_sts, 1)
REGDEF_BIT(gyro_trs_timeout_sts, 1)
REGDEF_BIT(gyro_update_done_even_sts, 1)
REGDEF_BIT(gyro_trs_abort_done_sts, 1)
REGDEF_BIT(gyro_dma_abort_done_sts, 1)
REGDEF_BIT(gyro_dma_addr_done_even_sts, 1)
REGDEF_BIT(gyro_dma_addr_done_odd_sts, 1)
REGDEF_BIT(gyro_update_done_odd_sts, 1)
REGDEF_BIT(dma_rx_abort_sts, 1)                //not used in 538
REGDEF_BIT(, 15)
REGDEF_END(SPI_STATUS_REG)     

#define SPI_INTEN_REG_OFS   0x1C
REGDEF_BEGIN(SPI_INTEN_REG)    
REGDEF_BIT(, 1)
REGDEF_BIT(spi_rdrf_int_en, 1)
REGDEF_BIT(spi_dma_tx_abort_int_en, 1)         //not used in 538
REGDEF_BIT(spi_dmaed_int_en, 1)                /* 0: DMA not transfer/receive end , 1: DMA transfer/receive end */
REGDEF_BIT(spi_rdstsed_int_en, 1)              /*0: SPI is reading status from serial flash or idle , 1: read SPI status done */
REGDEF_BIT(gyro_trs_rdy_int_en, 1)
REGDEF_BIT(gyro_overrun_int_en, 1)
REGDEF_BIT(all_gyrotrs_done_int_en, 1)
REGDEF_BIT(gyro_seq_err_int_en, 1)
REGDEF_BIT(gyro_trs_timeout_int_en, 1)
REGDEF_BIT(gyro_update_done_even_int_en, 1)
REGDEF_BIT(gyro_trs_abort_done_int_en, 1)
REGDEF_BIT(gyro_dma_abort_done_int_en, 1)
REGDEF_BIT(gyro_dma_addr_done_even_int_en, 1)
REGDEF_BIT(gyro_dma_addr_done_odd_int_en, 1)
REGDEF_BIT(gyro_update_done_odd_int_en, 1)
REGDEF_BIT(spi_dma_rx_abort_int_en, 1)           //not used in 538
REGDEF_BIT(, 15)
REGDEF_END(SPI_INTEN_REG)        

#define SPI_RDR_REG_OFS     0x20
REGDEF_BEGIN(SPI_RDR_REG)        
REGDEF_BIT(rdr, 32)             /* Receive data register */
REGDEF_END(SPI_RDR_REG)          

#define SPI_TDR_REG_OFS     0x24
REGDEF_BEGIN(SPI_TDR_REG)       
REGDEF_BIT(tdr, 32)             /* Transmit data register */
REGDEF_END(SPI_TDR_REG)         

//not used in 538
#define SPI_DMA_RX_STARTADDR_REG_OFS   0x28
REGDEF_BEGIN(SPI_DMA_RX_STARTADDR_REG)   
REGDEF_BIT(dma_rx_start_addr, 32) 
REGDEF_END(SPI_DMA_RX_STARTADDR_REG)    

#define SPI_DMA_CTRL_REG_OFS           0x30
REGDEF_BEGIN(SPI_DMA_CTRL_REG)   
REGDEF_BIT(dma_tx_en, 1)               //538  0=spi2dma, 1=dma2spi
REGDEF_BIT(dma_rx_en, 1)               //not used in 538
REGDEF_BIT(, 30)
REGDEF_END(SPI_DMA_CTRL_REG)    

#define SPI_DMA_BUFSIZE_REG_OFS 0x34
REGDEF_BEGIN(SPI_DMA_BUFSIZE_REG)    
REGDEF_BIT(spi_dma_bufsize, 24) /* DMA buffer size register in byte alignment */
REGDEF_BIT(, 8)
REGDEF_END(SPI_DMA_BUFSIZE_REG)  

//538: SPI_DMA_STARTADDR_REG_OFS
#define SPI_DMA_TX_STARTADDR_REG_OFS   0x38
REGDEF_BEGIN(SPI_DMA_TX_STARTADDR_REG) 
REGDEF_BIT(dma_tx_start_addr, 32)       
REGDEF_END(SPI_DMA_TX_STARTADDR_REG)


#define SPI_GYROSEN_CONFIG_REG_OFS      0x40
REGDEF_BEGIN(SPI_GYROSEN_CONFIG_REG)    /* SPI Gyro sensor Configuration Register */
REGDEF_BIT(trscnt, 7)                   /* NOT use: gyro transfer count per trigger */
REGDEF_BIT(, 1)
REGDEF_BIT(trslen_even, 2)               /* gyro transfer length: how many OPs */
REGDEF_BIT(, 2)
REGDEF_BIT(gyro_vdsrc, 4)                /* gyro VD source, SIE1~SIE5 VIE1~4 */
REGDEF_BIT(len_op0_even, 3)              /* OP0 byte length */
REGDEF_BIT(, 1)
REGDEF_BIT(len_op1_even, 3)              /* OP1 byte length */
REGDEF_BIT(, 1)
REGDEF_BIT(len_op2_even, 3)              /* OP2 byte length */
REGDEF_BIT(, 1)
REGDEF_BIT(len_op3_even, 3)              /* OP3 byte length */
REGDEF_BIT(, 1)
REGDEF_END(SPI_GYROSEN_CONFIG_REG)          /* SPI Gyro sensor Configuration Register end  */

#define SPI_GYROSEN_OP_INTERVAL_REG_OFS 0x44
REGDEF_BEGIN(SPI_GYROSEN_OP_INTERVAL_REG)   /* SPI Gyro sensor OP interval Register */
REGDEF_BIT(op_interval_even, 21)
REGDEF_BIT(, 11)
REGDEF_END(SPI_GYROSEN_OP_INTERVAL_REG)

#define SPI_GYROSEN_TRS_INTERVAL_REG_OFS    0x48
REGDEF_BEGIN(SPI_GYROSEN_TRS_INTERVAL_REG)  /* SPI Gyro sensor Transfer interval Register */
REGDEF_BIT(trs_interval_even, 21)
REGDEF_BIT(, 11)
REGDEF_END(SPI_GYROSEN_TRS_INTERVAL_REG)

#define SPI_GYROSEN_FIFO_STS_REG_OFS    0x4C
REGDEF_BEGIN(SPI_GYROSEN_FIFO_STS_REG)      /* SPI Gyro sensor Receive FIFO Status Register */
REGDEF_BIT(valid_entry, 8)
REGDEF_BIT(, 24)
REGDEF_END(SPI_GYROSEN_FIFO_STS_REG)

#define SPI_GYROSEN_TRS_STS_REG_OFS     0x50
REGDEF_BEGIN(SPI_GYROSEN_TRS_STS_REG)       /* SPI Gyro Transfer Status Register */
REGDEF_BIT(trsSts_0, 1)
REGDEF_BIT(trsSts_1, 1)
REGDEF_BIT(trsSts_2, 1)
REGDEF_BIT(trsSts_3, 1)
REGDEF_BIT(trsSts_4, 1)
REGDEF_BIT(trsSts_5, 1)
REGDEF_BIT(trsSts_6, 1)
REGDEF_BIT(trsSts_7, 1)
REGDEF_BIT(trsSts_8, 1)
REGDEF_BIT(trsSts_9, 1)
REGDEF_BIT(trsSts_10, 1)
REGDEF_BIT(trsSts_11, 1)
REGDEF_BIT(trsSts_12, 1)
REGDEF_BIT(trsSts_13, 1)
REGDEF_BIT(trsSts_14, 1)
REGDEF_BIT(trsSts_15, 1)
REGDEF_BIT(trsSts_16, 1)
REGDEF_BIT(trsSts_17, 1)
REGDEF_BIT(trsSts_18, 1)
REGDEF_BIT(trsSts_19, 1)
REGDEF_BIT(trsSts_20, 1)
REGDEF_BIT(trsSts_21, 1)
REGDEF_BIT(trsSts_22, 1)
REGDEF_BIT(trsSts_23, 1)
REGDEF_BIT(trsSts_24, 1)
REGDEF_BIT(trsSts_25, 1)
REGDEF_BIT(trsSts_26, 1)
REGDEF_BIT(trsSts_27, 1)
REGDEF_BIT(trsSts_28, 1)
REGDEF_BIT(trsSts_29, 1)
REGDEF_BIT(trsSts_30, 1)
REGDEF_BIT(trsSts_31, 1)
REGDEF_END(SPI_GYROSEN_TRS_STS_REG)         /* SPI Gyro Transfer Status Register */

#define SPI_GYROSEN_TRS_INTEN_REG_OFS        0x54
REGDEF_BEGIN(SPI_GYROSEN_TRS_INTEN_REG)      /* SPI Gyro Transfer Interrupt Enable Register */
REGDEF_BIT(trsInten_0, 1)
REGDEF_BIT(trsInten_1, 1)
REGDEF_BIT(trsInten_2, 1)
REGDEF_BIT(trsInten_3, 1)
REGDEF_BIT(trsInten_4, 1)
REGDEF_BIT(trsInten_5, 1)
REGDEF_BIT(trsInten_6, 1)
REGDEF_BIT(trsInten_7, 1)
REGDEF_BIT(trsInten_8, 1)
REGDEF_BIT(trsInten_9, 1)
REGDEF_BIT(trsInten_10, 1)
REGDEF_BIT(trsInten_11, 1)
REGDEF_BIT(trsInten_12, 1)
REGDEF_BIT(trsInten_13, 1)
REGDEF_BIT(trsInten_14, 1)
REGDEF_BIT(trsInten_15, 1)
REGDEF_BIT(trsInten_16, 1)
REGDEF_BIT(trsInten_17, 1)
REGDEF_BIT(trsInten_18, 1)
REGDEF_BIT(trsInten_19, 1)
REGDEF_BIT(trsInten_20, 1)
REGDEF_BIT(trsInten_21, 1)
REGDEF_BIT(trsInten_22, 1)
REGDEF_BIT(trsInten_23, 1)
REGDEF_BIT(trsInten_24, 1)
REGDEF_BIT(trsInten_25, 1)
REGDEF_BIT(trsInten_26, 1)
REGDEF_BIT(trsInten_27, 1)
REGDEF_BIT(trsInten_28, 1)
REGDEF_BIT(trsInten_29, 1)
REGDEF_BIT(trsInten_30, 1)
REGDEF_BIT(trsInten_31, 1)
REGDEF_END(SPI_GYROSEN_TRS_INTEN_REG)        /* SPI Gyro Transfer Interrupt Enable Register */

#define SPI_GYROSEN_COUNTER_REG_OFS         0x58
REGDEF_BEGIN(SPI_GYROSEN_COUNTER_REG)       /* SPI Gyro sensor Transfer and OP Counter Register */
REGDEF_BIT(transfer_counter, 8)
REGDEF_BIT(, 8)
REGDEF_BIT(op_counter, 2)
REGDEF_BIT(, 14)
REGDEF_END(SPI_GYROSEN_COUNTER_REG)

#define SPI_GYROSEN_RX_DATA_REG_OFS         0x5C
REGDEF_BEGIN(SPI_GYROSEN_RX_DATA_REG)       /* SPI Gyro sensor Receive Data Register */
REGDEF_BIT(rxdata, 32)
REGDEF_END(SPI_GYROSEN_RX_DATA_REG)

#define SPI_GYROSEN_TX_DATA1_REG_OFS        0x60
REGDEF_BEGIN(SPI_GYROSEN_TX_DATA1_REG)      /* SPI Gyro sensor Transfer Data Register 1 */
REGDEF_BIT(op0_lsb, 32)
REGDEF_END(SPI_GYROSEN_TX_DATA1_REG)

#define SPI_GYROSEN_TX_DATA2_REG_OFS        0x64
REGDEF_BEGIN(SPI_GYROSEN_TX_DATA2_REG)      /* SPI Gyro sensor Transfer Data Register 2 */
REGDEF_BIT(op0_msb, 32)
REGDEF_END(SPI_GYROSEN_TX_DATA2_REG)

/* 0x68, 0x6C: reserved */

#define SPI_GYROSEN_TX_DATA3_REG_OFS        0x70
REGDEF_BEGIN(SPI_GYROSEN_TX_DATA3_REG)      /* SPI Gyro sensor Transfer Data Register 3 */
REGDEF_BIT(op1_lsb, 32)
REGDEF_END(SPI_GYROSEN_TX_DATA3_REG)

#define SPI_GYROSEN_TX_DATA4_REG_OFS        0x74
REGDEF_BEGIN(SPI_GYROSEN_TX_DATA4_REG)      /* SPI Gyro sensor Transfer Data Register 4 */
REGDEF_BIT(op1_msb, 32)
REGDEF_END(SPI_GYROSEN_TX_DATA4_REG)

/* 0x78, 0x7C: reserved */

#define SPI_GYROSEN_TX_DATA5_REG_OFS        0x80
REGDEF_BEGIN(SPI_GYROSEN_TX_DATA5_REG)      /* SPI Gyro sensor Transfer Data Register 5 */
REGDEF_BIT(op2_lsb, 32)
REGDEF_END(SPI_GYROSEN_TX_DATA5_REG)

#define SPI_GYROSEN_TX_DATA6_REG_OFS        0x84
REGDEF_BEGIN(SPI_GYROSEN_TX_DATA6_REG)      /* SPI Gyro sensor Transfer Data Register 6 */
REGDEF_BIT(op2_msb, 32)
REGDEF_END(SPI_GYROSEN_TX_DATA6_REG)

/* 0x88, 0x8C: reserved */

#define SPI_GYROSEN_TX_DATA7_REG_OFS        0x90
REGDEF_BEGIN(SPI_GYROSEN_TX_DATA7_REG)      /* SPI Gyro sensor Transfer Data Register 7 */
REGDEF_BIT(op3_lsb, 32)
REGDEF_END(SPI_GYROSEN_TX_DATA7_REG)

#define SPI_GYROSEN_TX_DATA8_REG_OFS        0x94
REGDEF_BEGIN(SPI_GYROSEN_TX_DATA8_REG)      /* SPI Gyro sensor Transfer Data Register 8 */
REGDEF_BIT(op3_msb, 32)
REGDEF_END(SPI_GYROSEN_TX_DATA8_REG)

/* 0x98, 0x9C: reserved */

#define SPI_GYROSEN_EVEN_CFG_REG_OFS         0x100
REGDEF_BEGIN(SPI_GYROSEN_EVEN_CFG_REG)       /* SPI Gyro sensor Configuration Register 2 */
REGDEF_BIT(spi_gyro_trscnt_even, 8)
REGDEF_BIT(, 8)
REGDEF_BIT(spi_gyro_total_op_cnt_even, 11)
REGDEF_BIT(, 5)
REGDEF_END(SPI_GYROSEN_EVEN_CFG_REG)

#define SPI_GYROSEN_TRS_STS_REG2_OFS        0x104
REGDEF_BEGIN(SPI_GYROSEN_TRS_STS_REG2)      /* SPI Gyro Transfer Status Register 2 */
REGDEF_BIT(trsSts_32, 1)
REGDEF_BIT(trsSts_33, 1)
REGDEF_BIT(trsSts_34, 1)
REGDEF_BIT(trsSts_35, 1)
REGDEF_BIT(trsSts_36, 1)
REGDEF_BIT(trsSts_37, 1)
REGDEF_BIT(trsSts_38, 1)
REGDEF_BIT(trsSts_39, 1)
REGDEF_BIT(trsSts_40, 1)
REGDEF_BIT(trsSts_41, 1)
REGDEF_BIT(trsSts_42, 1)
REGDEF_BIT(trsSts_43, 1)
REGDEF_BIT(trsSts_44, 1)
REGDEF_BIT(trsSts_45, 1)
REGDEF_BIT(trsSts_46, 1)
REGDEF_BIT(trsSts_47, 1)
REGDEF_BIT(trsSts_48, 1)
REGDEF_BIT(trsSts_49, 1)
REGDEF_BIT(trsSts_50, 1)
REGDEF_BIT(trsSts_51, 1)
REGDEF_BIT(trsSts_52, 1)
REGDEF_BIT(trsSts_53, 1)
REGDEF_BIT(trsSts_54, 1)
REGDEF_BIT(trsSts_55, 1)
REGDEF_BIT(trsSts_56, 1)
REGDEF_BIT(trsSts_57, 1)
REGDEF_BIT(trsSts_58, 1)
REGDEF_BIT(trsSts_59, 1)
REGDEF_BIT(trsSts_60, 1)
REGDEF_BIT(trsSts_61, 1)
REGDEF_BIT(trsSts_62, 1)
REGDEF_BIT(trsSts_63, 1)
REGDEF_END(SPI_GYROSEN_TRS_STS_REG2)        /* SPI Gyro Transfer Status Register 2 */

#define SPI_GYROSEN_TRS_INTEN_REG2_OFS      0x108
REGDEF_BEGIN(SPI_GYROSEN_TRS_INTEN_REG2)    /* SPI Gyro Transfer Interrupt Enable Register 2 */
REGDEF_BIT(trsInten_32, 1)
REGDEF_BIT(trsInten_33, 1)
REGDEF_BIT(trsInten_34, 1)
REGDEF_BIT(trsInten_35, 1)
REGDEF_BIT(trsInten_36, 1)
REGDEF_BIT(trsInten_37, 1)
REGDEF_BIT(trsInten_38, 1)
REGDEF_BIT(trsInten_39, 1)
REGDEF_BIT(trsInten_40, 1)
REGDEF_BIT(trsInten_41, 1)
REGDEF_BIT(trsInten_42, 1)
REGDEF_BIT(trsInten_43, 1)
REGDEF_BIT(trsInten_44, 1)
REGDEF_BIT(trsInten_45, 1)
REGDEF_BIT(trsInten_46, 1)
REGDEF_BIT(trsInten_47, 1)
REGDEF_BIT(trsInten_48, 1)
REGDEF_BIT(trsInten_49, 1)
REGDEF_BIT(trsInten_50, 1)
REGDEF_BIT(trsInten_51, 1)
REGDEF_BIT(trsInten_52, 1)
REGDEF_BIT(trsInten_53, 1)
REGDEF_BIT(trsInten_54, 1)
REGDEF_BIT(trsInten_55, 1)
REGDEF_BIT(trsInten_56, 1)
REGDEF_BIT(trsInten_57, 1)
REGDEF_BIT(trsInten_58, 1)
REGDEF_BIT(trsInten_59, 1)
REGDEF_BIT(trsInten_60, 1)
REGDEF_BIT(trsInten_61, 1)
REGDEF_BIT(trsInten_62, 1)
REGDEF_BIT(trsInten_63, 1)
REGDEF_END(SPI_GYROSEN_TRS_INTEN_REG2)      /* SPI Gyro Transfer Interrupt Enable Register 2 */

#define SPI_GYROSEN_TRS_STS_REG3_OFS        0x10C
REGDEF_BEGIN(SPI_GYROSEN_TRS_STS_REG3)      /* SPI Gyro Transfer Status Register 3 */
REGDEF_BIT(trsSts_64, 1)
REGDEF_BIT(trsSts_65, 1)
REGDEF_BIT(trsSts_66, 1)
REGDEF_BIT(trsSts_67, 1)
REGDEF_BIT(trsSts_68, 1)
REGDEF_BIT(trsSts_69, 1)
REGDEF_BIT(trsSts_70, 1)
REGDEF_BIT(trsSts_71, 1)
REGDEF_BIT(trsSts_72, 1)
REGDEF_BIT(trsSts_73, 1)
REGDEF_BIT(trsSts_74, 1)
REGDEF_BIT(trsSts_75, 1)
REGDEF_BIT(trsSts_76, 1)
REGDEF_BIT(trsSts_77, 1)
REGDEF_BIT(trsSts_78, 1)
REGDEF_BIT(trsSts_79, 1)
REGDEF_BIT(trsSts_80, 1)
REGDEF_BIT(trsSts_81, 1)
REGDEF_BIT(trsSts_82, 1)
REGDEF_BIT(trsSts_83, 1)
REGDEF_BIT(trsSts_84, 1)
REGDEF_BIT(trsSts_85, 1)
REGDEF_BIT(trsSts_86, 1)
REGDEF_BIT(trsSts_87, 1)
REGDEF_BIT(trsSts_88, 1)
REGDEF_BIT(trsSts_89, 1)
REGDEF_BIT(trsSts_90, 1)
REGDEF_BIT(trsSts_91, 1)
REGDEF_BIT(trsSts_92, 1)
REGDEF_BIT(trsSts_93, 1)
REGDEF_BIT(trsSts_94, 1)
REGDEF_BIT(trsSts_95, 1)
REGDEF_END(SPI_GYROSEN_TRS_STS_REG3)        /* SPI Gyro Transfer Status Register 3 */

#define SPI_GYROSEN_TRS_INTEN_REG3_OFS      0x110
REGDEF_BEGIN(SPI_GYROSEN_TRS_INTEN_REG3)    /* SPI Gyro Transfer Interrupt Enable Register 3 */
REGDEF_BIT(trsInten_64, 1)
REGDEF_BIT(trsInten_65, 1)
REGDEF_BIT(trsInten_66, 1)
REGDEF_BIT(trsInten_67, 1)
REGDEF_BIT(trsInten_68, 1)
REGDEF_BIT(trsInten_69, 1)
REGDEF_BIT(trsInten_70, 1)
REGDEF_BIT(trsInten_71, 1)
REGDEF_BIT(trsInten_72, 1)
REGDEF_BIT(trsInten_73, 1)
REGDEF_BIT(trsInten_74, 1)
REGDEF_BIT(trsInten_75, 1)
REGDEF_BIT(trsInten_76, 1)
REGDEF_BIT(trsInten_77, 1)
REGDEF_BIT(trsInten_78, 1)
REGDEF_BIT(trsInten_79, 1)
REGDEF_BIT(trsInten_80, 1)
REGDEF_BIT(trsInten_81, 1)
REGDEF_BIT(trsInten_82, 1)
REGDEF_BIT(trsInten_83, 1)
REGDEF_BIT(trsInten_84, 1)
REGDEF_BIT(trsInten_85, 1)
REGDEF_BIT(trsInten_86, 1)
REGDEF_BIT(trsInten_87, 1)
REGDEF_BIT(trsInten_88, 1)
REGDEF_BIT(trsInten_89, 1)
REGDEF_BIT(trsInten_90, 1)
REGDEF_BIT(trsInten_91, 1)
REGDEF_BIT(trsInten_92, 1)
REGDEF_BIT(trsInten_93, 1)
REGDEF_BIT(trsInten_94, 1)
REGDEF_BIT(trsInten_95, 1)
REGDEF_END(SPI_GYROSEN_TRS_INTEN_REG3)      /* SPI Gyro Transfer Interrupt Enable Register 3 */

#define SPI_GYROSEN_EVEN_OFF_DLY_REG_OFS         0x114
REGDEF_BEGIN(SPI_GYROSEN_EVEN_OFF_DLY_REG)   /* SPI Gyro sensor Even Offset Delay Register */
REGDEF_BIT(spi_gyro_even_offset_dly, 21)
REGDEF_BIT(, 11)
REGDEF_END(SPI_GYROSEN_EVEN_OFF_DLY_REG)

#define SPI_GYROSEN_ODD_OFF_DLY_REG_OFS          0x118
REGDEF_BEGIN(SPI_GYROSEN_ODD_OFF_DLY_REG)    /* SPI Gyro sensor Odd Offset Delay Register */
REGDEF_BIT(spi_gyro_odd_offset_dly, 21)
REGDEF_BIT(, 11)
REGDEF_END(SPI_GYROSEN_ODD_OFF_DLY_REG)

/* 0x11C: reserved */

#define SPI_GYROSEN_GYRO_DMA_EVEN_LOW_REG_OFS      0x120
REGDEF_BEGIN(SPI_GYROSEN_GYRO_DMA_EVEN_LOW_REG)        /* SPI Gyro DMA Low Start Address 0 Register */
REGDEF_BIT(spi_gyro_dma_start_even_low, 32)            /* Gyro DMA DRAM low starting address 0 */
REGDEF_END(SPI_GYROSEN_GYRO_DMA_EVEN_LOW_REG)

#define SPI_GYROSEN_GYRO_DMA_ODD_LOW_REG_OFS       0x128
REGDEF_BEGIN(SPI_GYROSEN_GYRO_DMA_ODD_LOW_REG)     /* SPI Gyro DMA Low Start Address 1 Register */
REGDEF_BIT(spi_gyro_dma_start_odd_low, 32)             /* Gyro DMA DRAM low starting address 1 */
REGDEF_END(SPI_GYROSEN_GYRO_DMA_ODD_LOW_REG)

#define SPI_GYROSEN_ODD_CONFIG_REG_OFS              0x130
REGDEF_BEGIN(SPI_GYROSEN_ODD_CONFIG_REG)            /* SPI Gyro sensor Configuration Register */
REGDEF_BIT(, 8)
REGDEF_BIT(trslen_odd, 2)                           /* gyro transfer length: how many OPs */
REGDEF_BIT(, 6)
REGDEF_BIT(len_op0_odd, 3)                          /* OP0 byte length */
REGDEF_BIT(, 1)
REGDEF_BIT(len_op1_odd, 3)                          /* OP1 byte length */
REGDEF_BIT(, 1)
REGDEF_BIT(len_op2_odd, 3)                          /* OP2 byte length */
REGDEF_BIT(, 1)
REGDEF_BIT(len_op3_odd, 3)                          /* OP3 byte length */
REGDEF_BIT(, 1)
REGDEF_END(SPI_GYROSEN_ODD_CONFIG_REG)              /* SPI Gyro sensor Configuration Register end  */

#define SPI_GYROSEN_ODD_OP_INTERVAL_REG_OFS         0x134
REGDEF_BEGIN(SPI_GYROSEN_ODD_OP_INTERVAL_REG)       /* SPI Gyro sensor Odd OP interval Register */
REGDEF_BIT(op_interval_odd, 21)
REGDEF_BIT(, 11)
REGDEF_END(SPI_GYROSEN_ODD_OP_INTERVAL_REG)

#define SPI_GYROSEN_ODD_TRS_INTERVAL_REG_OFS        0x138
REGDEF_BEGIN(SPI_GYROSEN_ODD_TRS_INTERVAL_REG)      /* SPI Gyro sensor Odd Transfer interval Register */
REGDEF_BIT(trs_interval_odd, 21)
REGDEF_BIT(, 11)
REGDEF_END(SPI_GYROSEN_ODD_TRS_INTERVAL_REG)

#define SPI_GYROSEN_CONFIG_ODD_REG_OFS             0x13C
REGDEF_BEGIN(SPI_GYROSEN_CONFIG_ODD_REG)           /* SPI Gyro sensor Configuration Register 2 */
REGDEF_BIT(spi_gyro_trscnt_odd, 8)
REGDEF_BIT(, 8)
REGDEF_BIT(spi_gyro_total_op_cnt_odd, 11)
REGDEF_BIT(, 5)
REGDEF_END(SPI_GYROSEN_CONFIG_ODD_REG)
#endif