/**
    csi tx operation module.

    This module include the csi tx function

    @file       csi_tx_open.h
    @ingroup    imgtrans

    Copyright   Novatek Microelectronics Corp. 2022.  All rights reserved.
*/
#ifndef _CSI_TX_OPEN_H_
#define _CSI_TX_OPEN_H_

#ifdef __cplusplus
extern "C" {
#endif

extern void* csi_tx_alloc(int size);

extern void  csi_tx_free(void* buf);

extern void  csi_tx_memset(void *buf, unsigned char val, int len);

extern void  csi_tx_memcpy(void *dst, void *src, int len);

extern int   csi_tx_copy_from_user(void *dst, void *src, int len);

extern int   csi_tx_copy_to_user(void *dst, void *src, int len);

#ifdef __cplusplus
} //extern "C"
#endif

#endif //_CSI_TX_OPEN_H_
