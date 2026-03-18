/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Novatek XOR platform device data definition file.
 */

#ifndef __DMA_NVT_XOR_H
#define __DMA_NVT_XOR_H

#include <linux/dmaengine.h>

#define NVT_XOR_NAME	"nvt_xor"

struct nvt_xor_channel_data {
	dma_cap_mask_t			cap_mask;
};

struct nvt_xor_platform_data {
	struct nvt_xor_channel_data    *channels;
};

#endif
