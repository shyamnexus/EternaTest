#ifndef __H265_DEC_ENTITY_H__
#define __H265_DEC_ENTITY_H__

#include "../h26xdec_entity_comm.h"

/* decoder work fuction */
int H265Dec_SetBaseAddr(uintptr_t imvq_addr, int chip_idx);
void H265Dec_SetConfigSramBuf(DecoderParams *p_Dec);
void H265Dec_ReSetVpu(DecoderParams *p_Dec, struct buffer_info_t vpu_work_buffer);
void H265Dec_SetVpu(DecoderParams *p_Dec);
int H265Dec_SetCabacBuffer(DecoderParams *p_Dec, struct buffer_info_t cabac_buffer);
unsigned int H265Dec_CheckMemSize(void);
void H265Dec_Create(DecoderParams *p_Dec);
void H265Dec_SetEXWrite(struct h26xd_data_t *dec_data, int real_chn_idx);

void H265Dec_SetBitstreamBuf(DecoderParams *p_Dec, uintptr_t phy_addr, uintptr_t vir_addr, unsigned int bs_size, unsigned int total_buf_size, unsigned int max_ref_num);
int H265Dec_ScanFirstHeader(DecoderParams *p_Dec);
int H265Dec_SetRecBufferAddr(DecoderParams *p_Dec, H26X_DEC_FRAME_IOCTL *p_Frame);
int H265Dec_OneFrameStart(DecoderParams *p_Dec, H26X_DEC_FRAME_IOCTL *p_Frame);
void H265Dec_TriggerBin(DecoderParams *p_Dec);
int H265Dec_TriggerStart(DecoderParams *p_Dec, H26X_DEC_FRAME_IOCTL *p_Frame, int types);
int H265Dec_Sync(DecoderParams *p_Dec, H26X_DEC_FRAME_IOCTL *p_Frame);
int H265Dec_OutputAllPicture(DecoderParams *p_Dec, H26X_DEC_FRAME_IOCTL *p_Frame, int stop_flag);
int H265Dec_ReceiveIRQ(DecoderParams *p_Dec, int bypass_flag, int *h265_bin_st);
void H265Dec_TriggerHandler(DecoderParams *p_Dec);
unsigned int H265Dec_GetHwTimeout(DecoderParams *p_Dec);

#endif // _H265_DEC_ENTITY_H_
