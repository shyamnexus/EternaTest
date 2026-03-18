/**
    Header file of SDP module driver

    This file is the header file of SDP module

    @file       SDP.h
    @ingroup    mIDrvIO_SDP
    @note       Nothing.

    Copyright   Novatek Microelectronics Corp. 2018.  All rights reserved.
*/

#ifndef _SDP_H
#define _SDP_H
#include <kwrap/nvt_type.h>
#include "../comm/driver.h"


/**
    @addtogroup mIDrvIO_SDP
*/
//@{

/**
    SDP ID
*/
typedef enum {
	SDP_ID_1,                   ///< SDP 1

	SDP_ID_COUNT,               ///< SDP ID count

	ENUM_DUMMY4WORD(SDP_ID)
} SDP_ID;

/* enum _SDP_EVENTS: enum for SDP events
 *
 */
typedef enum _SDP_EVENTS {
	SDP_EVENTS_FIFO_RDY =         0x01 << 0,     ///< SDP DMA fifo ready bit[0]
	SDP_EVENTS_FIFO_UNDERRUN =    0x01 << 1,     ///< SDP DMA FIFO underrun bit[1]
	SDP_EVENTS_DMA_EXHAUSTED =    0x01 << 2,     ///< SDP DMA exhausted bit[2]
	SDP_EVENTS_DMA_READ_END =     0x01 << 3,     ///< SDP DMA read end bit[3]  (SDP read dma to send data)

	SDP_EVENTS_MRX_FIFO_EMPTY =   0x01 << 4,     ///< SDP MRX FIFO empty bit[4]
	SDP_EVENTS_MTX_FIFO_FULL =    0x01 << 5,     ///< SDP MTX FIFO full bit[5]
	SDP_EVENTS_CMD_DATA_UNALIGN = 0x01 << 6,     ///< SDP Master cmd data non align length bit[6]
	SDP_EVENTS_FIFO_OVERRUN =     0x01 << 7,     ///< SDP DMA FIFO overrun bit[7]
	SDP_EVENTS_CMD_CONFLICT =     0x01 << 8,     ///< SDP cmd confict bit[8]
	SDP_EVENTS_DMA_WRITE_END =    0x01 << 9,     ///< SDP DMA write end bit[9] (SDP write dma to fetch data)
	SDP_EVENTS_CMD_UNKNOWN =      0x01 << 10,    ///< SDP Slave received a command that is not supported bit[10]

	ENUM_DUMMY4WORD(SDP_EVENTS)
} SDP_EVENTS;

/*
    SDP_TRANSFER_MODE

    @note For SDP_PARAM_ID_TRANSFER_MODE
*/
typedef enum {
	SDP_TRANSFER_MODE_CONT,     //< only support continously read a burst data
	SDP_TRANSFER_MODE_SLICE,    //< only support read slices data

	ENUM_DUMMY4WORD(SDP_TRANSFER_MODE)
} SDP_TRANSFER_MODE;

/*
    SDP_DMA_MODE

    @note For SDP_PARAM_ID_DMA_MODE
*/
typedef enum {
	SDP_DMA_MODE_READ,          //< only support data buffer to be returned to SPI master through command 03H (SDP -> SPI master)
	SDP_DMA_MODE_WRITE,         //< only support data buffer transferred from SPI master through command 02H (SDP <- SPI master)
	SDP_DMA_MODE_READ_WRITE,    //full duplex sdp
	ENUM_DUMMY4WORD(SDP_DMA_MODE)
} SDP_DMA_MODE;

/* enum _SDP_PARAM_ID: structure for set/get functions
 *
 */
typedef enum _SDP_PARAM_ID {
	SDP_PARAM_ID_EVENT_CALLBACK,    ///< [set/get] SDP event callback.
	SDP_PARAM_ID_TRANSFER_MODE,     ///< [set/get] SDP transfer mode by peer SPI master
	                                ///< can be SDP_TRANSFER_MODE_CONT or SDP_TRANSFER_MODE_SLICE
	SDP_PARAM_ID_DMA_MODE,          ///< [set/get] SDP dma mode by peer SPI master
	                                ///< can be SDP_DMA_MODE_READ or SDP_DMA_MODE_WRITE

	ENUM_DUMMY4WORD(SDP_PARAM_ID)
} SDP_PARAM_ID;

/*
 * SDP operation command
 *
 * Select which operation for SDP engine.
 *
 * @note Used for SDP_TRIGGER_PARAM.
 */
typedef enum _SDP_CMD {
	SDP_CMD_NOP,                        ///< NULL operation
	SDP_CMD_SET_MASTER_RX_PORT,         ///< Set master receive port (SDP -> SPI master)
	                                    ///< Set data word to be returned to SPI master through command 08H
	SDP_CMD_GET_MASTER_TX_PORT,         ///< Get master transmit port (SDP <- SPI master)
	                                    ///< Get data word transferred from SPI master through command 09H
	SDP_CMD_SET_DMA_BUF,                ///< Set DMA buffer (SDP -> SPI master)
	                                    ///< Set data buffer to be returned to SPI master through command 03H
	SDP_CMD_GET_DMA_BUF,                ///< Get DMA buffer (SDP <- SPI master)
	                                    ///< Get data buffer transferred from SPI master through command 02H
	SDP_CMD_ABORT_DMA,                  ///< Abort DMA Transfer
	                                    ///< Abort DMA to be returned to SPI master through command 03H

	SDP_CMD_COUNT,
	ENUM_DUMMY4WORD(SDP_CMD)
} SDP_CMD;


/* struct  _SDP_TRIGGER_PARAM: structure to describe a job
 *
 * @note For sdp_trigger()
 */
typedef struct _SDP_TRIGGER_PARAM {
	UINT32		ver;        	///< driver version (reserved)
	SDP_CMD		command;        ///< Command
	uintptr_t	*p_data;        ///< data buffer reference by command
	UINT32		size;           ///< size of p_data (unit: byte)
} SDP_TRIGGER_PARAM;



extern ER sdp_open(SDP_ID id);
extern ER sdp_close(SDP_ID id);
extern ER sdp_trigger(SDP_ID id, SDP_TRIGGER_PARAM *p_param, DRV_CB p_cb_func, void *p_user_data);
extern ER sdp_set(SDP_ID id, SDP_PARAM_ID param_id, VOID *p_param);

extern void sdp_platform_init(void);
extern void sdp_platform_uninit(void);

//@}

#endif