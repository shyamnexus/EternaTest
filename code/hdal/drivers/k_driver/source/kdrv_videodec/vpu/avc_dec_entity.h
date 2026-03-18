#ifndef __H264_DEC_ENTITY_H__
#define __H264_DEC_ENTITY_H__

#include "../h26xdec_entity_comm.h"

#define CHECK264_SPS     0x1
#define CHECK264_PPS     0x2
#define CHECK264_ISLICE  0x4
#define CHECK264_ALL    (CHECK264_SPS | CHECK264_PPS | CHECK264_ISLICE)

/* decoder work fuction */
unsigned int H264Dec_CheckMemSize(void);
void H264Dec_Create(DecoderParams *p_Dec);
int H264Dec_SetBaseAddr(uintptr_t imvq_addr, int chip_idx);
void H264Dec_ReSetVpu(DecoderParams *p_Dec, struct buffer_info_t vpu_work_buffer);
void H264Dec_SetVpu(DecoderParams *p_Dec);
void H264Dec_SetBitstreamBuf(DecoderParams *p_Dec, uintptr_t phy_addr, uintptr_t vir_addr, unsigned int bs_size, unsigned int total_buf_size, unsigned int max_ref_num);
int H264Dec_ScanFirstHeader(DecoderParams *p_Dec);
int H264Dec_SetRecBufferAddr(DecoderParams *p_Dec, H26X_DEC_FRAME_IOCTL *p_Frame);
int H264Dec_OneFrameStart(DecoderParams *p_Dec, H26X_DEC_FRAME_IOCTL *p_Frame);
int H264Dec_Isr(DecoderParams *p_Dec, int bypass_flag);
int H264Dec_GetOutPutInfo(DecoderParams *p_Dec, H26X_DEC_FRAME_IOCTL *p_Frame);
int H264Dec_CleanAll(DecoderParams *p_Dec, H26X_DEC_FRAME_IOCTL *p_Frame);
void H264Dec_ReinitDRV(DecoderParams *p_Dec);
void H264Dec_SetEXWrite(struct h26xd_data_t *dec_data, int real_chn_idx);
unsigned int H264Dec_GetHwTimeout(DecoderParams *p_Dec);

#endif // _H264_DEC_ENTITY_H_
