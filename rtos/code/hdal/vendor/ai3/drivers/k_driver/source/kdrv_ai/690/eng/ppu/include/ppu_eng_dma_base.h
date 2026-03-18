#ifndef _PPU_ENG_DMA_BASE_H_
#define _PPU_ENG_DMA_BASE_H_

#ifdef __cplusplus
extern "C" {
#endif


#include "kwrap/type.h"
#include "ppu_eng_handle.h"


extern INT32 ppu_eng_set_dma_base_addr(PPU_ENG_HANDLE *p_eng, uintptr_t base_addr_msb, uintptr_t base_addr_lsb, UINT32 base_idx);


#ifdef __cplusplus
}
#endif


#endif //

