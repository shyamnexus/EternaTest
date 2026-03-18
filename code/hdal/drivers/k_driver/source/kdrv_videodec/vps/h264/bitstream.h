
#ifndef __BITSTREAM_H__
#define __BITSTREAM_H__

#include "../vps_comm.h"

typedef struct ST_VPU_OUTPUT_INFO
{
	UINT16 u16CropLeft;
	UINT16 u16CropRight;
	UINT16 u16CropTop;
	UINT16 u16CropBottom;
}ST_VPU_OUTPUT_INFO, *PST_VPU_OUTPUT_INFO;

/* solve conflicting issue in rtos */
#define init_parse_bitstream	avc_init_parse_bitstream
#define bit_length				avc_bit_length
#define get_bits				avc_get_bits

int avc_init_parse_bitstream(bstream *bs, uintptr_t ringBufStartAddr, uintptr_t ringBufEndAddr, uintptr_t startAddr, uintptr_t endAddr, unsigned int bs_len, uintptr_t (*emptyCallback)(bstream *));

int bs_next_buffer(bstream *bs);

void skip_bits(bstream * const bs, unsigned int bits);
unsigned int show_bits(bstream * const bs, unsigned int bits);
int avc_bit_length(bstream * const bs);
int bit_remainder(bstream * const bs);
unsigned int get_bits(bstream *const bs, unsigned int bits);

int update_total_res_bits(bstream * const bs, UINT32 start_byte_ofs);

/* seekToStartCode is used only for little endian target */
int seekToStartCode(bstream * const bs);
int byte_position(bstream * const bs);

static inline UINT32 get_uvlc(bstream *pBs)
{
    UINT32 peek32, leadingZeroCount;

    peek32 = show_bits(pBs, 32);

    if(peek32 >= 0xFFFFFFFE){
        skip_bits(pBs, 1);
        return 0;
    }

    leadingZeroCount = __builtin_clz(peek32);

    if (leadingZeroCount >= 32)
    {
        return get_bits(pBs, 32);
    }

    skip_bits(pBs, leadingZeroCount+1);

    if (leadingZeroCount != 0)
    {
        return (1 << leadingZeroCount) - 1 + get_bits(pBs, leadingZeroCount);
    }

    return 0;
}

static inline INT32 get_svlc(bstream *pBs)
{
    UINT32 peek32, leadingZeroCount;

    peek32 = show_bits(pBs, 32);

    if (peek32 == 0)
    {
        return get_bits(pBs, 32); //error syntax
    }

    leadingZeroCount = __builtin_clz(peek32);

    skip_bits(pBs, leadingZeroCount+1);

    if (leadingZeroCount != 0)
    {
		UINT32 val = (UINT32)((UINT32)1 << leadingZeroCount) + (UINT32) get_bits(pBs, leadingZeroCount);
		return	(INT32)((val & 1) ? -(val >> 1) : (val >> 1));
	}

    return 0;
}

#endif
