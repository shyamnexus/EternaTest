#ifndef _H265_HEADER_H_
#define _H265_HEADER_H_

#include "kwrap/type.h"

#include "h26x.h"
#include "h26x_def.h"
#include "h26x_common.h"

#include "h265_def.h"
#include "h265enc_api.h"

extern void h265Enc_encSeqHeader(H265ENC_CTX *pVdoCtx, H26XCOMN_CTX *pComnCtx);
extern INT32 h265Enc_encSliceHeader(H26XENC_VAR *pVar,BOOL bGetSeqHdrEn);
extern UINT32 h265Enc_encSuffixSEI(uintptr_t dst_addr, uintptr_t data_addr, UINT32 len);
#endif	// _H265_HEADER_H_

