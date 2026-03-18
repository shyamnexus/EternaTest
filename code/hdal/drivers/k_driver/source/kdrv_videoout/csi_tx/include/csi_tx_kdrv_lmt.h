/**
 * @file csi_tx_kdrv_lmt.h
 * @brief parameter limitation of KDRV CSI_TX
 * @author ESW
 * @date in the year 2021
 */

#ifndef _KDRV_CSI_TX_LMT_H_
#define _KDRV_CSI_TX_LMT_H_

#include "comm/drv_lmt.h"
#include "kdrv_type.h"

#define KDRV_CSI_TX_MAX_WIDTH                                  (4096)
#define KDRV_CSI_TX_MAX_HIGH                                   (2048)

// buffer limitation
#define KDRV_CSI_TX_BUF_WIDTH_ALIGN			0x00000002					// unit: pixel
#define KDRV_CSI_TX_BUF_HIGH_ALIGN			0x00000002					// unit: line
#define KDRV_CSI_TX_BUF_ALIGN				DRV_LIMIT_ALIGN_WORD		// unit: byte
#define KDRV_CSI_TX_ADDR_ALIGN				DRV_LIMIT_ALIGN_BYTE
