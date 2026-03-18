/**
    csi tx module.

    This module include the csi tx function

    @file       csi_tx_internal.h
    @ingroup    csi_tx

    Copyright   Novatek Microelectronics Corp. 2022.  All rights reserved.
*/
#ifndef _CSI_TX_INTERNAL_H_
#define _CSI_TX_INTERNAL_H_

#ifdef __cplusplus
extern "C" {
#endif

extern int  nvt_csi_tx_init(void);

extern void nvt_csi_tx_exit(void);

extern int  nvt_csi_tx_ioctl(int f, unsigned int cmd, void *arg);

#ifdef __cplusplus
} //extern "C"
#endif

#endif //_CSI_TX_INTERNAL_H_
