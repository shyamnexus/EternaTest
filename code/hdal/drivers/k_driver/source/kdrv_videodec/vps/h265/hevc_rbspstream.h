#ifndef __H265_RBSPSTREAM_H__
#define __H265_RBSPSTREAM_H__

#include "hevc_bitstream.h"

typedef struct
{
	hevc_bstream	bitstream;

	/* bit-stream domain */
	uintptr_t	cur;
	uintptr_t	end;
	uintptr_t ring_start;
	uintptr_t ring_end;
	unsigned int	zero_count;
	int	shorten;

	/* rbsp domain */
	unsigned int	first_half_available;
	unsigned char	nal_rbsp[MAX_RBSP_SIZE] __attribute__((aligned(8)));
}hevc_rbspstream;

static inline hevc_bstream *hevc_cast2bitstream(hevc_rbspstream *rbsp)
{
    return &rbsp->bitstream;
}

int hevc_create_rbsp_from_bitstream(hevc_rbspstream *rbsp, hevc_bstream *bs, uintptr_t start_addr, uintptr_t end_addr);
#endif
