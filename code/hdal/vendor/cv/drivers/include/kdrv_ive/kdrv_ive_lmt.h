/**
 * @file kdrv_ive_lmt.h
 * @brief parameter limitation of KDRV IVE
 * @author CVAI
 * @date in the year 2019
 */

#ifndef __IVE_DRV_LIMIT_H__
#define __IVE_DRV_LIMIT_H__

//=====================================================
// input / output limitation
#define IVE_WIDTH_MIN               1
#define IVE_WIDTH_MAX               16383

#define IVE_HEIGHT_MIN              1
#define IVE_HEIGHT_MAX              8191

#define IVE_OFSI_ALIGN              0x00000004
#define IVE_OFSO_ALIGN              0x00000004
#define IVE_IN_ADDR_ALIGN           0x00000004
#define IVE_OUT_ADDR_ALIGN          0x00000004

#define IVE_POSTPROC_WIDTH_MIN      16
#define IVE_POSTPROC_WIDTH_MAX      16383
#define IVE_POSTPROC_HEIGHT_MIN     16
#define IVE_POSTPROC_HEIGHT_MAX     8191
#define IVE_THRESH_WIDTH_MIN        64
#define IVE_THRESH_WIDTH_MAX        1920
#define IVE_THRESH_HEIGHT_MIN       64
#define IVE_THRESH_HEIGHT_MAX       1080
#define IVE_CANNY_WIDTH_MIN         16
#define IVE_CANNY_WIDTH_MAX         16383
#define IVE_CANNY_HEIGHT_MIN        16
#define IVE_CANNY_HEIGHT_MAX        8191
#define IVE_IMG_OP_WIDTH_MIN        64
#define IVE_IMG_OP_WIDTH_MAX        1920
#define IVE_IMG_OP_HEIGHT_MIN       64
#define IVE_IMG_OP_HEIGHT_MAX       1080
#define IVE_CSC_WIDTH_MIN         	16
#define IVE_CSC_WIDTH_MAX         	4096
#define IVE_CSC_HEIGHT_MIN        	16
#define IVE_CSC_HEIGHT_MAX        	8191
#define IVE_HISTO_WIDTH_MIN         64
#define IVE_HISTO_WIDTH_MAX         4096
#define IVE_HISTO_HEIGHT_MIN        64
#define IVE_HISTO_HEIGHT_MAX        2160
#define IVE_NCC_WIDTH_MIN        	32
#define IVE_NCC_WIDTH_MAX        	1920
#define IVE_NCC_HEIGHT_MIN       	32
#define IVE_NCC_HEIGHT_MAX       	1080
#define IVE_DMA_WIDTH_MIN      		32
#define IVE_DMA_WIDTH_MAX      		16383
#define IVE_DMA_HEIGHT_MIN     		1
#define IVE_DMA_HEIGHT_MAX     		8191
#define IVE_PLANAR_TO_PACK_WIDTH_MIN         32
#define IVE_PLANAR_TO_PACK_WIDTH_MAX         16383
#define IVE_PLANAR_TO_PACK_HEIGHT_MIN        32
#define IVE_PLANAR_TO_PACK_HEIGHT_MAX        8191


#define IVE_EDGE_COEFF_VMIN         -128
#define IVE_EDGE_COEFF_VMAX         127
#define IVE_THRESH_MODE_MIN         1
#define IVE_THRESH_MODE_MAX         11
#define IVE_THRESH_U8_MIN           0
#define IVE_THRESH_U8_MAX           255
#define IVE_THRESH_S8_MIN           -128
#define IVE_THRESH_S8_MAX           127

#define IVE_MORPHO_MASK_VMIN        0
#define IVE_MORPHO_MASK_VMAX        1
#define IVE_LBP_WIDTH_MIN      64
#define IVE_LBP_WIDTH_MAX      1920
#define IVE_LBP_HEIGHT_MIN     64
#define IVE_LBP_HEIGHT_MAX     1080
#define IVE_VA_WIDTH_MIN       32
#define IVE_VA_WIDTH_MAX       16383
#define IVE_VA_HEIGHT_MIN      32
#define IVE_VA_HEIGHT_MAX      8191
#define IVE_MAP_WIDTH_MIN      32
#define IVE_MAP_WIDTH_MAX      16383
#define IVE_MAP_HEIGHT_MIN     32
#define IVE_MAP_HEIGHT_MAX     8191
#define IVE_2D_LUT_WIDTH_MIN      1
#define IVE_2D_LUT_WIDTH_MAX      65
#define IVE_2D_LUT_HEIGHT_MIN     1
#define IVE_2D_LUT_HEIGHT_MAX     65


//=====================================================
// Para limitation
//#define IVE_GEN_FILT_MAX          15
#define IVE_EDGE_FILT_MAX           15
#define IVE_EDGE_FILT_MIN           -15
#define IVE_EDGE_THRES_MAX          255
#define IVE_EDGE_MAG_TH_MAX         128

#define IVE_DRV_ALIGN_DMA_LLC		(16)
#endif /* __IVE_DRV_LIMIT_H__ */
