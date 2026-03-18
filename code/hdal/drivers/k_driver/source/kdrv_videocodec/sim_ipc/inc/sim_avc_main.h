#ifndef _SIM_AVC_MAIN_H_
#define _SIM_AVC_MAIN_H_

#include "kwrap/type.h"

void sim_avc_enc_main(UINT32 width, UINT32 height, char *string, uintptr_t ddr_addr, UINT32 ddr_size);
void sim_avc_dec_main(UINT32 width, UINT32 height, char *string, uintptr_t ddr_addr, UINT32 ddr_size);

#endif // _SIM_AVC_MAIN_H_