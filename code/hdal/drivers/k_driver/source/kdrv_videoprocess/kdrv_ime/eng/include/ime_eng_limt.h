
#ifndef _IME_ENG_LIMIT_H_
#define _IME_ENG_LIMIT_H_

//=====================================================
// input limitation
#define IME_SSDRV_SRC_SIZE_H_MIN            4
#define IME_SSDRV_SRC_SIZE_H_MAX            65532
#define IME_SSDRV_SRC_SIZE_H_ALIGN          4

#define IME_SSDRV_SRC_SIZE_ALLDIR_H_MAX     4096   ///< for all direct mode

#define IME_SSDRV_SRC_SIZE_V_MIN            4
#define IME_SSDRV_SRC_SIZE_V_HMAX           65532
#define IME_SSDRV_SRC_SIZE_V_ALIGN          4

#define IME_SSDRV_SRC_BUF_LOFS_ALIGN        4
#define IME_SSDRV_SRC_BUF_ADDR_ALIGN        4

//=====================================================
// output limitation
#define IME_SSDRV_SCALE_SIZE_H_MIN          4
#define IME_SSDRV_SCALE_SIZE_H_MAX          65534
#define IME_SSDRV_SCALE_SIZE_H_ALIGN        2

#define IME_SSDRV_SCALE_SIZE_V_MIN          4
#define IME_SSDRV_SCALE_SIZE_V_MAX          65534
#define IME_SSDRV_SCALE_SIZE_V_ALIGN        2


//=====================================================
// yuv420 input scale limitation
#define IME_SSDRV_INPUT_SCALE_YUV420_SIZE_H_MIN    4
#define IME_SSDRV_INPUT_SCALE_YUV420_SIZE_H_MAX    4096
#define IME_SSDRV_INPUT_SCALE_YUV420_SIZE_H_ALIGN  4

#define IME_SSDRV_INPUT_SCALE_YUV420_SIZE_V_MIN    4
#define IME_SSDRV_INPUT_SCALE_YUV420_SIZE_V_MAX    65532
#define IME_SSDRV_INPUT_SCALE_YUV420_SIZE_V_ALIGN  4

//=====================================================
// yuv420 output scale limitation
#define IME_SSDRV_SCALE_YUV420_SIZE_H_MIN          4
#define IME_SSDRV_SCALE_YUV420_SIZE_H_MAX          65534
#define IME_SSDRV_SCALE_YUV420_SIZE_H_ALIGN        2

#define IME_SSDRV_SCALE_YUV420_SIZE_V_MIN          4
#define IME_SSDRV_SCALE_YUV420_SIZE_V_MAX          65534
#define IME_SSDRV_SCALE_YUV420_SIZE_V_ALIGN        2


//=====================================================
// yuv420 output crop limitation
#define IME_SSDRV_PRE_YUV420_CROP_POS_H_ALIGN       2
#define IME_SSDRV_PRE_YUV420_CROP_POS_V_ALIGN       2
#define IME_SSDRV_PRE_YUV420_CROP_W_ALIGN           2
#define IME_SSDRV_PRE_YUV420_CROP_H_ALIGN           2

#define IME_SSDRV_POST_YUV420_CROP_POS_H_ALIGN      2
#define IME_SSDRV_POST_YUV420_CROP_POS_V_ALIGN      2
#define IME_SSDRV_POST_YUV420_CROP_W_ALIGN          2
#define IME_SSDRV_POST_YUV420_CROP_H_ALIGN          2

#define IME_SSDRV_DST_YUV420_SIZE_H_MIN            4
#define IME_SSDRV_DST_YUV420_SIZE_H_MAX            65534
#define IME_SSDRV_DST_YUV420_SIZE_H_ALIGN          2            // for normal case
//#define IME_SSDRV_DST_YUV420_P1_WITH_3DNR_SIZE_H_ALIGN     0  // for 3dnr on and path1 is reference image

#define IME_SSDRV_DST_YUV420_SIZE_V_MIN            4
#define IME_SSDRV_DST_YUV420_SIZE_V_HMAX           65534
#define IME_SSDRV_DST_YUV420_SIZE_V_ALIGN          2            // for normal case
//#define IME_SSDRV_DST_YUV420_P1_WITH_3DNR_SIZE_V_ALIGN     0  // for 3dnr on and path1 is reference image

#define IME_SSDRV_DST_YUV420_BUF_LOFS_ALIGN        4
#define IME_SSDRV_DST_YUV420_BUF_ADDR_Y_ALIGN      1
#define IME_SSDRV_DST_YUV420_BUF_ADDR_UV_ALIGN     2

//=====================================================
// yuv420 nvx2 input scale limitation
#define IME_SSDRV_INPUT_SCALE_YUV420_NVX2_SIZE_H_MIN    4
#define IME_SSDRV_INPUT_SCALE_YUV420_NVX2_SIZE_H_MAX    65532
#define IME_SSDRV_INPUT_SCALE_YUV420_NVX2_SIZE_H_ALIGN  4

#define IME_SSDRV_INPUT_SCALE_YUV420_NVX2_SIZE_V_MIN    4
#define IME_SSDRV_INPUT_SCALE_YUV420_NVX2_SIZE_V_MAX    65532
#define IME_SSDRV_INPUT_SCALE_YUV420_NVX2_SIZE_V_ALIGN  4

//=====================================================
// yuv420 nvx2 output scale limitation
#define IME_SSDRV_SCALE_YUV420_NVX2_SIZE_H_MIN          4
#define IME_SSDRV_SCALE_YUV420_NVX2_SIZE_H_MAX          65534
#define IME_SSDRV_SCALE_YUV420_NVX2_SIZE_H_ALIGN        2

#define IME_SSDRV_SCALE_YUV420_NVX2_SIZE_V_MIN          4
#define IME_SSDRV_SCALE_YUV420_NVX2_SIZE_V_MAX          65534
#define IME_SSDRV_SCALE_YUV420_NVX2_SIZE_V_ALIGN        2


//=====================================================
// yuv420 nvx2 output crop limitation
#define IME_SSDRV_PRE_CROP_YUV420_NVX2_POS_H_ALIGN      2
#define IME_SSDRV_PRE_CROP_YUV420_NVX2_POS_V_ALIGN      2
#define IME_SSDRV_PRE_CROP_YUV420_NVX2_W_ALIGN          2
#define IME_SSDRV_PRE_CROP_YUV420_NVX2_H_ALIGN          2

#define IME_SSDRV_POST_CROP_YUV420_NVX2_POS_H_ALIGN     2
#define IME_SSDRV_POST_CROP_YUV420_NVX2_POS_V_ALIGN     2
#define IME_SSDRV_POST_CROP_YUV420_NVX2_W_ALIGN         2
#define IME_SSDRV_POST_CROP_YUV420_NVX2_H_ALIGN         2

#define IME_SSDRV_DST_YUV420_NVX2_SIZE_H_MIN            4
#define IME_SSDRV_DST_YUV420_NVX2_SIZE_H_MAX            65532
#define IME_SSDRV_DST_YUV420_NVX2_SIZE_H_ALIGN          2           // for normal case
//#define IME_SSDRV_DST_YUV420_NVX2_P1_WITH_3DNR_SIZE_H_ALIGN     4 // for 3dnr on and path1 is reference image

#define IME_SSDRV_DST_YUV420_NVX2_SIZE_V_MIN            4
#define IME_SSDRV_DST_YUV420_NVX2_SIZE_V_HMAX           65532
#define IME_SSDRV_DST_YUV420_NVX2_SIZE_V_ALIGN          2           // for normal case
//#define IME_SSDRV_DST_YUV420_NVX2_P1_WITH_3DNR_SIZE_V_ALIGN     4 // for 3dnr on and path1 is reference image

#define IME_SSDRV_DST_YUV420_NVX2_BUF_LOFS_ALIGN        4
#define IME_SSDRV_DST_YUV420_NVX2_BUF_ADDR_Y_ALIGN      1
#define IME_SSDRV_DST_YUV420_NVX2_BUF_ADDR_UV_ALIGN     2

//=====================================================
// yuv420_mb4 input scale limitation
#define IME_SSDRV_INPUT_SCALE_YUV420_MB4_SIZE_H_MIN    0
#define IME_SSDRV_INPUT_SCALE_YUV420_MB4_SIZE_H_MAX    0
#define IME_SSDRV_INPUT_SCALE_YUV420_MB4_SIZE_H_ALIGN  0

#define IME_SSDRV_INPUT_SCALE_YUV420_MB4_SIZE_V_MIN    0
#define IME_SSDRV_INPUT_SCALE_YUV420_MB4_SIZE_V_MAX    0
#define IME_SSDRV_INPUT_SCALE_YUV420_MB4_SIZE_V_ALIGN  0

//=====================================================
// yuv420_mb4 output scale limitation
#define IME_SSDRV_SCALE_YUV420_MB4_SIZE_H_MIN          2
#define IME_SSDRV_SCALE_YUV420_MB4_SIZE_H_MAX          65532
#define IME_SSDRV_SCALE_YUV420_MB4_SIZE_H_ALIGN        2

#define IME_SSDRV_SCALE_YUV420_MB4_SIZE_V_MIN          2
#define IME_SSDRV_SCALE_YUV420_MB4_SIZE_V_MAX          65532
#define IME_SSDRV_SCALE_YUV420_MB4_SIZE_V_ALIGN        2

//=====================================================
// yuv420_mb4 output crop limitation
#define IME_SSDRV_PRE_CROP_YUV420_MB4_POS_H_ALIGN       2
#define IME_SSDRV_PRE_CROP_YUV420_MB4_POS_V_ALIGN       2
#define IME_SSDRV_PRE_CROP_YUV420_MB4_W_ALIGN           2
#define IME_SSDRV_PRE_CROP_YUV420_MB4_H_ALIGN           2

#define IME_SSDRV_POST_CROP_YUV420_MB4_POS_H_ALIGN     2
#define IME_SSDRV_POST_CROP_YUV420_MB4_POS_V_ALIGN     2
#define IME_SSDRV_POST_CROP_YUV420_MB4_W_ALIGN         2
#define IME_SSDRV_POST_CROP_YUV420_MB4_H_ALIGN         2

#define IME_SSDRV_DST_YUV420_MB4_SIZE_H_MIN            2
#define IME_SSDRV_DST_YUV420_MB4_SIZE_H_MAX            65534
#define IME_SSDRV_DST_YUV420_MB4_SIZE_H_ALIGN          2            // for normal case
//#define IME_SSDRV_DST_YUV420_MB4_P1_WITH_3DNR_SIZE_H_ALIGN     0  // for 3dnr on and path1 is reference image

#define IME_SSDRV_DST_YUV420_MB4_SIZE_V_MIN            2
#define IME_SSDRV_DST_YUV420_MB4_SIZE_V_HMAX           65534
#define IME_SSDRV_DST_YUV420_MB4_SIZE_V_ALIGN          2            // for normal case
//#define IME_SSDRV_DST_YUV420_MB4_P1_WITH_3DNR_SIZE_V_ALIGN     0  // for 3dnr on and path1 is reference image

#define IME_SSDRV_DST_YUV420_MB4_BUF_LOFS_ALIGN        4
#define IME_SSDRV_DST_YUV420_MB4_BUF_ADDR_Y_ALIGN      1
#define IME_SSDRV_DST_YUV420_MB4_BUF_ADDR_UV_ALIGN     2

//=====================================================
// yuv420_nvx5 input scale limitation
#define IME_SSDRV_INPUT_SCALE_YUV420_NVX5_SIZE_H_MIN    0
#define IME_SSDRV_INPUT_SCALE_YUV420_NVX5_SIZE_H_MAX    0
#define IME_SSDRV_INPUT_SCALE_YUV420_NVX5_SIZE_H_ALIGN  0

#define IME_SSDRV_INPUT_SCALE_YUV420_NVX5_SIZE_V_MIN    0
#define IME_SSDRV_INPUT_SCALE_YUV420_NVX5_SIZE_V_MAX    0
#define IME_SSDRV_INPUT_SCALE_YUV420_NVX5_SIZE_V_ALIGN  0

//=====================================================
// yuv420_nvx5 output scale limitation
#define IME_SSDRV_SCALE_YUV420_NVX5_SIZE_H_MIN          4
#define IME_SSDRV_SCALE_YUV420_NVX5_SIZE_H_MAX          65532
#define IME_SSDRV_SCALE_YUV420_NVX5_SIZE_H_ALIGN        4

#define IME_SSDRV_SCALE_YUV420_NVX5_SIZE_V_MIN          4
#define IME_SSDRV_SCALE_YUV420_NVX5_SIZE_V_MAX          65532
#define IME_SSDRV_SCALE_YUV420_NVX5_SIZE_V_ALIGN        4

//=====================================================
// yuv420_nvx5 output crop limitation
#define IME_SSDRV_PRE_CROP_YUV420_NVX5_POS_H_ALIGN      2
#define IME_SSDRV_PRE_CROP_YUV420_NVX5_POS_V_ALIGN      2
#define IME_SSDRV_PRE_CROP_YUV420_NVX5_W_ALIGN          2
#define IME_SSDRV_PRE_CROP_YUV420_NVX5_H_ALIGN          2

#define IME_SSDRV_POST_CROP_YUV420_NVX5_POS_H_ALIGN     2
#define IME_SSDRV_POST_CROP_YUV420_NVX5_POS_V_ALIGN     2
#define IME_SSDRV_POST_CROP_YUV420_NVX5_W_ALIGN         2
#define IME_SSDRV_POST_CROP_YUV420_NVX5_H_ALIGN         2

#define IME_SSDRV_DST_YUV420_NVX5_SIZE_H_MIN            4
#define IME_SSDRV_DST_YUV420_NVX5_SIZE_H_MAX            65532
#define IME_SSDRV_DST_YUV420_NVX5_SIZE_H_ALIGN          2           // for normal case
//#define IME_SSDRV_DST_YUV420_NVX5_P1_WITH_3DNR_SIZE_H_ALIGN     0 // for 3dnr on and path1 is reference image

#define IME_SSDRV_DST_YUV420_NVX5_SIZE_V_MIN            4
#define IME_SSDRV_DST_YUV420_NVX5_SIZE_V_HMAX           65532
#define IME_SSDRV_DST_YUV420_NVX5_SIZE_V_ALIGN          2           // for normal case
//#define IME_SSDRV_DST_YUV420_NVX5_P1_WITH_3DNR_SIZE_V_ALIGN     0 // for 3dnr on and path1 is reference image

#define IME_SSDRV_DST_YUV420_NVX5_BUF_LOFS_ALIGN        4
#define IME_SSDRV_DST_YUV420_NVX5_BUF_ADDR_Y_ALIGN      1
#define IME_SSDRV_DST_YUV420_NVX5_BUF_ADDR_UV_ALIGN     2

//=====================================================
// yuv422 input scale limitation
#define IME_SSDRV_INPUT_SCALE_YUV422_SIZE_H_MIN    0
#define IME_SSDRV_INPUT_SCALE_YUV422_SIZE_H_MAX    0
#define IME_SSDRV_INPUT_SCALE_YUV422_SIZE_H_ALIGN  0

#define IME_SSDRV_INPUT_SCALE_YUV422_SIZE_V_MIN    0
#define IME_SSDRV_INPUT_SCALE_YUV422_SIZE_V_MAX    0
#define IME_SSDRV_INPUT_SCALE_YUV422_SIZE_V_ALIGN  0

//=====================================================
// yuv422 output scale limitation
#define IME_SSDRV_SCALE_YUV422_SIZE_H_MIN          4
#define IME_SSDRV_SCALE_YUV422_SIZE_H_MAX          65534
#define IME_SSDRV_SCALE_YUV422_SIZE_H_ALIGN        2

#define IME_SSDRV_SCALE_YUV422_SIZE_V_MIN          4
#define IME_SSDRV_SCALE_YUV422_SIZE_V_MAX          65534
#define IME_SSDRV_SCALE_YUV422_SIZE_V_ALIGN        2

//=====================================================
// yuv422 output crop limitation
#define IME_SSDRV_PRE_CROP_YUV422_POS_H_ALIGN       2
#define IME_SSDRV_PRE_CROP_YUV422_POS_V_ALIGN       2
#define IME_SSDRV_PRE_CROP_YUV422_W_ALIGN           2
#define IME_SSDRV_PRE_CROP_YUV422_H_ALIGN           2

#define IME_SSDRV_POST_CROP_YUV422_POS_H_ALIGN     2
#define IME_SSDRV_POST_CROP_YUV422_POS_V_ALIGN     2
#define IME_SSDRV_POST_CROP_YUV422_W_ALIGN         2
#define IME_SSDRV_POST_CROP_YUV422_H_ALIGN         2

#define IME_SSDRV_DST_YUV422_SIZE_H_MIN            2
#define IME_SSDRV_DST_YUV422_SIZE_H_MAX            65534
#define IME_SSDRV_DST_YUV422_SIZE_H_ALIGN          2            // for normal case
//#define IME_SSDRV_DST_YUV422_P1_WITH_3DNR_SIZE_H_ALIGN     0  // for 3dnr on and path1 is reference image

#define IME_SSDRV_DST_YUV422_SIZE_V_MIN            2
#define IME_SSDRV_DST_YUV422_SIZE_V_HMAX           65534
#define IME_SSDRV_DST_YUV422_SIZE_V_ALIGN          2            // for normal case
//#define IME_SSDRV_DST_YUV422_P1_WITH_3DNR_SIZE_V_ALIGN     0  // for 3dnr on and path1 is reference image

#define IME_SSDRV_DST_YUV422_BUF_LOFS_ALIGN        4
#define IME_SSDRV_DST_YUV422_BUF_ADDR_Y_ALIGN      1
#define IME_SSDRV_DST_YUV422_BUF_ADDR_UV_ALIGN     2

//=====================================================
// common
#define IME_SSDRV_CROP_POS_H_ALIGN          2
#define IME_SSDRV_CROP_POS_V_ALIGN          2


#define IME_SSDRV_DST_SIZE_H_MIN            4
#define IME_SSDRV_DST_SIZE_H_MAX            65532
#define IME_SSDRV_DST_SIZE_H_ALIGN          2      // for normal case
#define IME_SSDRV_DST_P1_WITH_3DNR_SIZE_H_ALIGN     4  // for 3dnr on and path1 is reference image

#define IME_SSDRV_DST_SIZE_V_MIN            4
#define IME_SSDRV_DST_SIZE_V_HMAX           65532
#define IME_SSDRV_DST_SIZE_V_ALIGN          2      // for normal case
#define IME_SSDRV_DST_P1_WITH_3DNR_SIZE_V_ALIGN     4  // for 3dnr on and path1 is reference image

#define IME_SSDRV_DST_BUF_LOFS_ALIGN        4
#define IME_SSDRV_DST_BUF_ADDR_Y_ALIGN      1
#define IME_SSDRV_DST_BUF_ADDR_UV_ALIGN     2

//=====================================================
// scaling limitation
#define IME_SSDRV_SCALE_UP_RATE_MAX     2048        // (in - 1) * 65536 / (out - 1)
#define IME_SSDRV_SCALE_DOWN_RATE_MAX   1047920     // (in - 1) * 65536 / (out - 1)


//=====================================================
// integration
#define IME_SSDRV_SCALE_INTG_STP_OUT_SIZE_H_MAX     2048     ///< each stripe output max. size when ISD


//=====================================================
// linked-list
#define IME_SSDRV_LL_BUF_ADDR_ALIGN     4

//=====================================================
// lca limitation
#define IME_SSDRV_LCA_SIZE_H_MIN        40
#define IME_SSDRV_LCA_SIZE_H_WMAX       65532
#define IME_SSDRV_LCA_SIZE_H_WALIGN     1

#define IME_SSDRV_LCA_SIZE_V_MIN        30
#define IME_SSDRV_LCA_SIZE_V_HMAX       65532
#define IME_SSDRV_LCA_SIZE_V_HALIGN     1

#define IME_SSDRV_LCA_BUF_LOFS_ALIGN    4
#define IME_SSDRV_LCA_BUF_ADDR_ALIGN    4


//=====================================================
// OSD input limitation
// 530 removed
#if 0
#define IME_SSDRV_OSD_SIZE_H_MIN        8
#define IME_SSDRV_OSD_SIZE_H_MAX        8192
#define IME_SSDRV_OSD_SIZE_H_ALIGN      2

#define IME_SSDRV_OSD_SIZE_V_MIN        8
#define IME_SSDRV_OSD_SIZE_V_HMAX       8192
#define IME_SSDRV_OSD_SIZE_V_ALIGN      2

#define IME_SSDRV_OSD_BUF_LOFS_ALIGN    4
#define IME_SSDRV_OSD_BUF_ADDR_ALIGN    4

typedef enum {
	IME_SSDRV_OSD_SET0 = 0x00000001,
	IME_SSDRV_OSD_SET1 = 0x00000002,
	IME_SSDRV_OSD_SET2 = 0x00000004,
	IME_SSDRV_OSD_SET3 = 0x00000008,
} IME_SSDRV_OSD_SET;

#define IME_SSDRV_OSD_D2D_SUPPORT       (IME_SSDRV_OSD_SET0 | IME_SSDRV_OSD_SET3);
#define IME_SSDRV_OSD_DIRECT_SUPPORT    (IME_SSDRV_OSD_SET0 | IME_SSDRV_OSD_SET1 | IME_SSDRV_OSD_SET2 | IME_SSDRV_OSD_SET3);
#endif

//=====================================================
// Privacy-mask pixelation input limitation
#define IME_SSDRV_PM_PXL_SIZE_H_MIN         2
#define IME_SSDRV_PM_PXL_SIZE_H_MAX         2048
#define IME_SSDRV_PM_PXL_SIZE_H_ALIGN       2

#define IME_SSDRV_PM_PXL_SIZE_V_MIN         2
#define IME_SSDRV_PM_PXL_SIZE_V_MAX         2048
#define IME_SSDRV_PM_PXL_SIZE_V_ALIGN       1

#define IME_SSDRV_PM_PXL_BUF_LOFS_ALIGN     4
#define IME_SSDRV_PM_PXL_BUF_ADDR_ALIGN     4

//=====================================================
// 3DNR
#define IME_SSDRV_3DNR_BUF_LOFS_ALIGN       4
#define IME_SSDRV_3DNR_BUF_ADDR_ALIGN       4


//extern INT32 ime_eng_chk_limitation(UINT32 reg_base_addr, UINT32 reg_flag_addr);



#endif

