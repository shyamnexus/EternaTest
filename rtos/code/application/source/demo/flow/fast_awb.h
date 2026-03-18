#ifndef _FAST_AWB_
#define _FAST_AWB_

//=============================================================================
// struct & enum definition
//=============================================================================
#define AWB_CA_W_WINNUM     32
#define AWB_CA_H_WINNUM     32
#define AWB_CA_MAX_WINNUM   (AWB_CA_W_WINNUM * AWB_CA_H_WINNUM)

//=============================================================================
// extern functions
//=============================================================================
extern void fast_awb_flow(HD_VIDEO_FRAME *video_frame, UINT32 *r_gain, UINT32 *g_gain, UINT32 *b_gain);

#endif

