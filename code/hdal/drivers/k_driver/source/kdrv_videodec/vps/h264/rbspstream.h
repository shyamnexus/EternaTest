#ifndef __RBSPSTREAM_H__
#define __RBSPSTREAM_H__

#include "bitstream.h"

#define MAX_RBSP_SIZE (256)

typedef struct
{
    bstream bitstream;

    /* bit-stream domain */
    uintptr_t cur;
    uintptr_t end;
    uintptr_t ring_start;
    uintptr_t ring_end;
    UINT32 zero_count;
    INT32 shorten;

    /* rbsp domain */
    UINT32 first_half_available;
    UINT8  nal_rbsp[MAX_RBSP_SIZE] __attribute__((aligned(8)));
}rbspstream;

extern int create_rbsp_from_bitstream(rbspstream *rbsp, bstream *bs, uintptr_t start_offset, uintptr_t end_offset);

static inline bstream *cast2bitstream(rbspstream *rbsp)
{
    return &rbsp->bitstream;
}
#endif
