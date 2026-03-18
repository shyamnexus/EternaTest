#ifndef __DMA_UTIL_H__
#define __DMA_UTIL_H__
#include <stdbool.h>

typedef struct {
	unsigned int  dst_ddr_id;
	uintptr_t  dst_addr;
	unsigned int  src_ddr_id;
	uintptr_t  src_addr;
	unsigned int  size;    //must be 32 alignment
	unsigned int  pattern;
	bool  flush;
} dma_util_t;

/* IOCTLs */
#define DMA_UTIL_IOC_MAGIC  'h' //seed
#define DMA_UTIL_DO_DMA_MEMCPY      _IOW(DMA_UTIL_IOC_MAGIC, 1, dma_util_t)
#define DMA_UTIL_DO_DMA_MEMSET      _IOW(DMA_UTIL_IOC_MAGIC, 2, dma_util_t)

#endif /* __DMA_UTIL_H__ */
