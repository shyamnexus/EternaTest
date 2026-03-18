/**
    @brief Header file of csi_tx module.\n
    This file contains the functions which is related to csi_tx in the chip.

    @file csi_tx.h

    @ingroup mhdal

    @note Nothing.

    Copyright Novatek Microelectronics Corp. 2022.  All rights reserved.
*/

#ifndef _CSI_TX_H_
#define _CSI_TX_H_

/********************************************************************
    INCLUDE FILES
********************************************************************/
#include "hdal.h"

/********************************************************************
    MACRO CONSTANT DEFINITIONS
********************************************************************/

/********************************************************************
    MACRO FUNCTION DEFINITIONS
********************************************************************/

/********************************************************************
    TYPE DEFINITION
********************************************************************/

typedef struct _CSI_TX_SEND {
	HD_VIDEO_FRAME    img;
	UINT32            vtotal;
	UINT32            vvalid_start;
	UINT32            vvalid_end;
	UINT32            hbp;
	UINT32            hfp;
	UINT32            hinv;
	UINT32            buffer_id;
	UINT32            pixpkt_ph_dt;
	UINT32            datalane_no; ///< 1 for lane#1. 2 for lane#2. 0/4 for lane#4
	UINT32            tlpx;
	UINT32            ths_prepare;
	UINT32            ths_zero;
	UINT32            ths_trail;
	UINT32            ths_exit;
	UINT32            tclk_prepare;
	UINT32            tclk_zero;
	UINT32            tclk_post;
	UINT32            tclk_pre;
	UINT32            tclk_trail;
	UINT32            mode;
	UINT32            int_en;
	UINT32            frame_rate;
	UINT32            pixpkt_ph_vc;
} CSI_TX_SEND;

/********************************************************************
    EXTERN VARIABLES & FUNCTION PROTOTYPES DECLARATIONS
********************************************************************/

HD_RESULT csi_tx_init(void);

HD_RESULT csi_tx_open(UINT32 clock_rate);

HD_RESULT csi_tx_send(CSI_TX_SEND *p_img);

HD_RESULT csi_tx_send_auto(CSI_TX_SEND *p_img);

HD_RESULT csi_tx_send_buf(uintptr_t buf1, uintptr_t buf2, UINTPTR blk);

HD_RESULT csi_tx_close(void);

HD_RESULT csi_tx_uninit(void);

#endif //_CSI_TX_H_

