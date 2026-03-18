#ifndef __HEVC_TOOL_H__
#define __HEVC_TOOL_H__

#define FAST_LOG2(x) (sizeof(unsigned int)*8 - 1 - __builtin_clz((unsigned int)(x)))
#define ceil_log2(x) (((x) - (1 << FAST_LOG2(x))) ? FAST_LOG2(x) + 1 : FAST_LOG2(x))

#define MAX_LLC_INFO_Y_SIZE(width,height)      (ALIGN(ALIGN(width, 64)/64*16, 128) * (ALIGN(height, 64)/8))  //LLC4 ~= LLC8 * 4
#define MAX_LLC_INFO_UV_SIZE(width,height)   (MAX_LLC_INFO_Y_SIZE(width, height) / 2)

int fr2idx(ST_HEVC_CONTEXT *hevc, ST_BUFFER * frame);
ST_BUFFER* idx2fr(ST_HEVC_CONTEXT *hevc, int index);

int checkPictureDependency(ST_PIC_RESOURCE *pic, ST_SLICE *slice_in_next_pic);
int doBumpingProcess(ST_HEVC_CONTEXT *hevc, ST_PIC_RESOURCE *pic);
int processDecodedPictureBuffer(ST_HEVC_CONTEXT *hevc, ST_PIC_RESOURCE *pic);
int isBelongToThisPicture(ST_HEVC_CONTEXT *hevc, ST_PIC_RESOURCE *picRes, ST_SLICE *slice);
int insertSliceIntoPicture(ST_PIC_RESOURCE *picRes, ST_SLICE* slice);
unsigned int getNumRpsCurrTempList(ST_SLICE *slice);
int isSeekDone(ST_HEVC_CONTEXT *hevc, ST_SLICE *slice);
void checkCRA(ST_HEVC_CONTEXT *hevc, int nal_unit_type, int *pocCRA, unsigned char *prevRAPisBLA);
int isRandomAccessSkipPicture(ST_HEVC_CONTEXT *hevc, int nal_unit_type);
int isSkipPictureForBLA(ST_HEVC_CONTEXT *hevc, int nal_unit_type);
int isDecodable(ST_HEVC_CONTEXT *hevc, int nal_unit_type);
int isProgressive(ST_SPS *sps);
int resolution(ST_SPS *sps);
void init_frame_pool (PST_HEVC_CONTEXT hevc);

/* add by CW */
ST_BUFFER* alloc_frame_in_pool(PST_HEVC_CONTEXT hevc, unsigned int resolution, int buf_index);
void free_frame_in_pool(PST_HEVC_CONTEXT hevc, ST_BUFFER *buffer, int dummy);
void cleanup_all_buffer(PST_HEVC_CONTEXT hevc);

#endif

