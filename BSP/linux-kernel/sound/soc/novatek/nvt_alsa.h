/**
    NVT ALSA header file
    This file will provide NVT ALSA related structure & API
    @file       nvt_alsa.h
    @ingroup
    @note
    Copyright   Novatek Microelectronics Corp. 2023.  All rights reserved.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License version 2 as
    published by the Free Software Foundation.
*/

#ifndef _NVT_ALSA_H
#define _NVT_ALSA_H

#define NVT_DAI3_I2S    1

struct audio_substream_data {
	struct page *pg;
	unsigned int order;
	u16 num_of_pages;
	u16 direction;
	uint64_t size;
	void __iomem *dai_mmio;
};

#endif
