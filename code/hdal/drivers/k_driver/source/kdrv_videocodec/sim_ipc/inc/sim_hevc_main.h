#ifndef _SIM_HEVC_MAIN_H_
#define _SIM_HEVC_MAIN_H_

#include "kwrap/type.h"

void sim_hevc_enc_main(UINT32 width, UINT32 height, char *string, uintptr_t ddr_addr, UINT32 ddr_size);
//void sim_hevc_dec_main(UINT32 width, UINT32 height, char *string);
#endif // _SIM_HEVC_MAIN_H_