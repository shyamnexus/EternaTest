
#ifndef __VPS_COMM_H__
#define __VPS_COMM_H__

#include "../portab.h"

#define MAX_START_CODE_PRELOAD_SIZE (3)
#define MAX_RBSP_SIZE (256)
#define BS_ALIGN_SIZE  (sizeof(int))

#define NOT_PARSING_H264_SEI    (1)

#define swap32(a) ((((a) & 0xff000000L) >> 24) | (((a) & 0x00ff0000L) >>  8) |  (((a) & 0x0000ff00L) <<  8) | (((a) & 0x000000ffL) << 24))
#define swap64(a) ((swap32(a) << 32) | swap32(a >> 32))
#define BSWAP(a)    a = swap32(a)

#define likely(x) __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)

struct hevc_ringbstream;
struct ringbstream;

typedef struct hevc_ringbstream
{
	/* add by CW */
	UINT32	*seek;			/* seek to next word */
	UINT32	seek_offset;		/* fixed seek */
	UINT32	valid_size;		/* one frame of bitstream size */
	UINT32	used_size;		/* one frame of bitstream size */
	UINT32	rbsp_used_count;
	BOOL		record_previous_start_code_flag;
	/*-------------------------------------------------*/

	UINT32	bufl;
	UINT32	bufh;
	UINT32	offset;		/* bit unit */

	UINT32	wordAlignedOffset;
	uintptr_t	ringBufStartAddress;
	uintptr_t	ringBufEndAddress;
	UINT32	usedBufSize;
	UINT32	validBufSize;
	uintptr_t (*emptyCallback)(struct hevc_ringbstream *);

	INT32	overflow;
	INT32	bufIndex;
}hevc_bstream;

typedef struct ringbstream
{
	/* add by hk */
	UINT32  valid_size;   // one frame of bitstream size
	INT32  total_res_bits; // res size = valid_size*8 - consume bits

	UINT32 *read;   //point to next word
	UINT32 *write;
	UINT32  bufl;
	UINT32  bufh;
	UINT32	offset; //bit unit

	INT32  	underflow;
	UINT32  wordAlignedOffset;
	uintptr_t ringBufStartAddress;
	uintptr_t ringBufEndAddress;
	uintptr_t lastWrite[MAX_START_CODE_PRELOAD_SIZE]; //lastWrite[0] is the nearest update
	UINT32  usedBufSize;
	UINT32  validBufSize;
	INT32  bufIndex;
	uintptr_t bufRAddr[4];
	uintptr_t bufWAddr[4];
	UINT32 *updateRead;
	UINT32 *updateWrite;
	uintptr_t (*emptyCallback)(struct ringbstream *);
} bstream;

#endif