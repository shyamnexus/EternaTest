
#ifndef _VIE_LIMIT_H_
#define _VIE_LIMIT_H_

//=====================================================
// input limitation
#define VIE_MAX_ENG_NUM                2
#define VIE_MAX_CHANNEL                4

#define VIE1_MAX_OUTPUT_NUM            2 // out0/1 for each channel
#define VIE1_MAX_INPUT_NUM             0 
#define VIE2_MAX_OUTPUT_NUM            2 // out0/1 for each channel
#define VIE2_MAX_INPUT_NUM             0 

#define VIE_PATGEN_SRC_WIN_W_ALIGN     4
#define VIE_PATGEN_SRC_WIN_H_ALIGN     1
#define VIE_PATGEN_4KCH_SRC_WIN_MAX_W  8192  
#define VIE_PATGEN_SRC_WIN_MAX_W       4096
#define VIE_PATGEN_SRC_WIN_MAX_H       2160
#define VIE_PATGEN_SRC_WIN_MIN_W       16
#define VIE_PATGEN_SRC_WIN_MIN_H       16

#define VIE_CROP_WIN_W_ALIGN           4// if split function off
#define VIE_CROP_SPLIT_WIN_W_ALIGN     8// if split function on
#define VIE_CROP_WIN_H_ALIGN           1
#define VIE_CROP_YUV420_WIN_H_ALIGN    2
#define VIE_CROP_WIN_X_ALIGN           1
#define VIE_CROP_WIN_Y_ALIGN           1

#define VIE_4KCH_CRP_WIN_MAX_W         8192
#define VIE_CROP_WIN_MAX_W             4096
#define VIE_CROP_WIN_MAX_H             2160
#define VIE_CROP_WIN_MIN_W             16
#define VIE_CROP_WIN_MIN_H             16

#define VIE_OUT0_LINEOFFSET_ALIGN     4 // word align
#define VIE_OUT1_LINEOFFSET_ALIGN     4 // word align


typedef enum {
	VIE_SSDRV_FUNC_NONE          = 0x00000000, ///< none
	VIE_SSDRV_FUNC_PATGEN        = 0x00000001, ///< pattern gen
	VIE_SSDRV_FUNC_MASK          = 0x00000002, ///< mask0~3
	VIE_SSDRV_FUNC_DBG           = 0x00000004, ///< debug info
	VIE_SSDRV_FUNC_MIRROR        = 0x00000008, ///< mirror
	VIE_SSDRV_FUNC_FLIP          = 0x00000010, ///< fliP
	VIE_SSDRV_FUNC_YCC           = 0x00000020, ///< ycc
	VIE_SSDRV_FUNC_SINGLE_OUT    = 0x00000040, ///< single out
} VIE_SSDRV_FUNC_SUPPORT;



#define VIE1_SUPPORT_FUNC (VIE_SSDRV_FUNC_PATGEN|VIE_SSDRV_FUNC_MASK|VIE_SSDRV_FUNC_DBG|VIE_SSDRV_FUNC_MIRROR| \
						   VIE_SSDRV_FUNC_FLIP|VIE_SSDRV_FUNC_YCC|VIE_SSDRV_FUNC_SINGLE_OUT)

#define VIE2_SUPPORT_FUNC (VIE_SSDRV_FUNC_PATGEN|VIE_SSDRV_FUNC_MASK|VIE_SSDRV_FUNC_DBG|VIE_SSDRV_FUNC_MIRROR| \
						   VIE_SSDRV_FUNC_FLIP|VIE_SSDRV_FUNC_YCC|VIE_SSDRV_FUNC_SINGLE_OUT)

#endif

