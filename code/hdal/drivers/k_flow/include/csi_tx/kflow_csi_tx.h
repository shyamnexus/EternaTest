/**
    @brief Header file of kflow csi tx module.\n
    This file contains the functions which is related to csi tx

    @file kflow_cis_tx.h

    @ingroup csi_tx

    @note Nothing.

    Copyright Novatek Microelectronics Corp. 2022.  All rights reserved.
*/

#ifndef _KFLOW_CSI_TX_H_
#define _KFLOW_CSI_TX_H_

#define CSI_TX_USER_VERSION 20220117

typedef enum {
	CSI_TX_USER_CMD_NULL,
	CSI_TX_USER_CMD_NULL1,
	CSI_TX_USER_CMD_NULL2,
	CSI_TX_USER_CMD_NULL3,
	CSI_TX_USER_CMD_INIT,
	CSI_TX_USER_CMD_OPEN,
	CSI_TX_USER_CMD_SEND,
	CSI_TX_USER_CMD_CLOSE,
	CSI_TX_USER_CMD_UNINIT,
	CSI_TX_USER_CMD_SEND_AUTO,
	CSI_TX_USER_CMD_SEND_BUF,
	CSI_TX_USER_CMD_MAX,

	ENUM_DUMMY4WORD(CSI_TX_USER_CMD)
} CSI_TX_USER_CMD;

typedef struct _CSI_TX_OPEN_K {
	UINT32       clock_rate;
} CSI_TX_OPEN_K;

typedef struct _CSI_TX_SEND_K {
	VDO_FRAME    img;
	uintptr_t    blk;
	UINT32       vtotal;
	UINT32       vvalid_start;
	UINT32       vvalid_end;
	UINT32       hbp;
	UINT32       hfp;
	UINT32       hinv;
	UINT32       buffer_id;
	UINT32       pixpkt_ph_dt;
	UINT32       datalane_no;
	UINT32       tlpx;
	UINT32       ths_prepare;
	UINT32       ths_zero;
	UINT32       ths_trail;
	UINT32       ths_exit;
	UINT32       tclk_prepare;
	UINT32       tclk_zero;
	UINT32       tclk_post;
	UINT32       tclk_pre;
	UINT32       tclk_trail;
	UINT32       mode;
	UINT32       int_en;
	UINT32       frame_rate;
	UINT32       pixpkt_ph_vc;
} CSI_TX_SEND_K;

typedef struct _CSI_TX_SEND_BUF_K {
	uintptr_t     y;
	uintptr_t     uv;
	uintptr_t     blk;
} CSI_TX_SEND_BUF_K;

typedef struct _CSI_TX_USER_DATA {
	UINT32                         version;
	CSI_TX_USER_CMD                cmd;
	union {
		CSI_TX_OPEN_K          open;
		CSI_TX_SEND_K          send;
		CSI_TX_SEND_BUF_K      send_buf;
	} data;
} CSI_TX_USER_DATA;

#if defined(__FREERTOS)
extern int nvt_csi_tx_init(void);
extern int nvt_csi_tx_ioctl(int f, unsigned int cmd, void *arg);
#endif
#endif //_KFLOW_CSI_TX_H_
