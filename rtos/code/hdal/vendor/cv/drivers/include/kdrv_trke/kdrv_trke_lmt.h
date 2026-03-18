/**
 * @file kdrv_trke_lmt.h
 * @brief parameter limitation of KDRV TRKE
 * @author CVAI
 * @date in the year 2019
 */

#ifndef __TRKE_DRV_LIMIT_H__
#define __TRKE_DRV_LIMIT_H__

//=====================================================
// input / output limitation
#define TRKE_WIDTH_MIN               1
#define TRKE_WIDTH_MAX               16383

#define TRKE_HEIGHT_MIN              1
#define TRKE_HEIGHT_MAX              8191

#define TRKE_OFSI_ALIGN              0x00000004
#define TRKE_OFSO_ALIGN              0x00000004
#define TRKE_IN_ADDR_ALIGN           0x00000004
#define TRKE_OUT_ADDR_ALIGN          0x00000004

#define TRKE_POSTPROC_WIDTH_MIN      16
#define TRKE_POSTPROC_WIDTH_MAX      16383
#define TRKE_POSTPROC_HEIGHT_MIN     16
#define TRKE_POSTPROC_HEIGHT_MAX     8191
#define TRKE_THRESH_WIDTH_MIN        64
#define TRKE_THRESH_WIDTH_MAX        1920
#define TRKE_THRESH_HEIGHT_MIN       64
#define TRKE_THRESH_HEIGHT_MAX       1080
#define TRKE_CANNY_WIDTH_MIN         16
#define TRKE_CANNY_WIDTH_MAX         16383
#define TRKE_CANNY_HEIGHT_MIN        16
#define TRKE_CANNY_HEIGHT_MAX        8191
#define TRKE_IMG_OP_WIDTH_MIN        64
#define TRKE_IMG_OP_WIDTH_MAX        1920
#define TRKE_IMG_OP_HEIGHT_MIN       64
#define TRKE_IMG_OP_HEIGHT_MAX       1080
#define TRKE_CSC_WIDTH_MIN         	16
#define TRKE_CSC_WIDTH_MAX         	4096
#define TRKE_CSC_HEIGHT_MIN        	16
#define TRKE_CSC_HEIGHT_MAX        	8191
#define TRKE_HISTO_WIDTH_MIN         64
#define TRKE_HISTO_WIDTH_MAX         4096
#define TRKE_HISTO_HEIGHT_MIN        64
#define TRKE_HISTO_HEIGHT_MAX        2160
#define TRKE_NCC_WIDTH_MIN        	32
#define TRKE_NCC_WIDTH_MAX        	1920
#define TRKE_NCC_HEIGHT_MIN       	32
#define TRKE_NCC_HEIGHT_MAX       	1080
#define TRKE_DMA_WIDTH_MIN      		32
#define TRKE_DMA_WIDTH_MAX      		16383
#define TRKE_DMA_HEIGHT_MIN     		1
#define TRKE_DMA_HEIGHT_MAX     		8191

#define TRKE_EDGE_COEFF_VMIN         -15
#define TRKE_EDGE_COEFF_VMAX         15
#define TRKE_THRESH_MODE_MIN         1
#define TRKE_THRESH_MODE_MAX         11
#define TRKE_MORPHO_MASK_VMIN        0
#define TRKE_MORPHO_MASK_VMAX        1

//=====================================================
// Para limitation
//#define TRKE_GEN_FILT_MAX          15
#define TRKE_EDGE_FILT_MAX           15
#define TRKE_EDGE_FILT_MIN           -15
#define TRKE_EDGE_THRES_MAX          255
#define TRKE_EDGE_MAG_TH_MAX         128

#define TRKE_DRV_ALIGN_DMA_LLC		(16)
#endif /* __TRKE_DRV_LIMIT_H__ */
