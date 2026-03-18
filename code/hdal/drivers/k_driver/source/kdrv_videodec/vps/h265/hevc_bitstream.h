
#ifndef __H265_BITSTREAM_H__
#define __H265_BITSTREAM_H__

#include "../../imvq.h"
#include "../vps_comm.h"

int hevc_init_parse_bitstream(hevc_bstream *bs, uintptr_t ringBufStartAddr, uintptr_t ringBufEndAddr, unsigned int bs_len, uintptr_t (*emptyCallback)(hevc_bstream *));

void hevc_skip_bits(hevc_bstream * const bs, unsigned int bits);
void hevc_skip_bytes(hevc_bstream * const bs, int bytes);

unsigned int hevc_show_bits(hevc_bstream * const bs, unsigned int bits);
int hevc_bit_length(hevc_bstream * const bs);
int hevc_bit_remainder(hevc_bstream * const bs);

unsigned int hevc_get_bits(hevc_bstream *const bs, unsigned int bits);

/* hevc_seekToStartCode is used only for little endian target */
int hevc_seekToStartCode(hevc_bstream * const bs, bool first_seek_flag);

uintptr_t hevc_byte_position(hevc_bstream * const bs);	/* CW */

static inline unsigned int get_uvlc(hevc_bstream *pBs)
{
	unsigned int peek32, leadingZeroCount;

    peek32 = hevc_show_bits(pBs, 32);

    if(peek32 >= 0xFFFFFFFE){
        hevc_skip_bits(pBs, 1);
        return 0;
    }

    leadingZeroCount = __builtin_clz(peek32);

    if (leadingZeroCount >= 32)
    {
        return hevc_get_bits(pBs, 32);
    }

    hevc_skip_bits(pBs, leadingZeroCount+1);

    if (leadingZeroCount != 0)
    {
		return ((unsigned int)1 << leadingZeroCount) - 1 + hevc_get_bits(pBs, leadingZeroCount);
    }

    return 0;
}

static inline int get_svlc(hevc_bstream *pBs)
{
	unsigned int peek32, leadingZeroCount;

    peek32 = hevc_show_bits(pBs, 32);

    if (peek32 == 0)
    {
        return hevc_get_bits(pBs, 32); /* error syntax */
    }

    leadingZeroCount = __builtin_clz(peek32);

    hevc_skip_bits(pBs, leadingZeroCount+1);

    if (leadingZeroCount != 0)
    {
		unsigned int val = ((unsigned int)1 << leadingZeroCount) + (unsigned int)hevc_get_bits(pBs, leadingZeroCount);
        return  (int)((val & 1) ? -(val >> 1) : (val >> 1));
    }

    return 0;
}

#endif
