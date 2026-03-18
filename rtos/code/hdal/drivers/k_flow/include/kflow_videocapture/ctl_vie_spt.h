/**
    Public header file for VIE limit

    This file is the header file that define VIE limit.

    @file      ctl_vie_spt.h
    @ingroup    mILibVIECom
    @note      Nothing (or anything need to be mentioned).

    Copyright   Novatek Microelectronics Corp. 2020.  All rights reserved.
*/
#ifndef _CTL_VIE_SPT_H_
#define _CTL_VIE_SPT_H_
#include "kflow_videocapture/ctl_vie_utility.h"

/*
    output format support
*/
typedef enum {
	CTL_VIE_SPT_YUV_422_NOSPT   		= (1UL << CTL_VIE_YUV_422_NOSPT),
	CTL_VIE_SPT_YUV_422_SPT     		= (1UL << CTL_VIE_YUV_422_SPT),
	CTL_VIE_SPT_YUV_422_SPT_YCCENC     	= (1UL << CTL_VIE_YUV_422_SPT_YCCENC),
	CTL_VIE_SPT_YUV_420_SPT     		= (1UL << CTL_VIE_YUV_420_SPT),
	CTL_VIE_SPT_YUV_420_SPT_YCCENC     	= (1UL << CTL_VIE_YUV_420_SPT_YCCENC),
	ENUM_DUMMY4WORD(CTL_VIE_DATAFORMAT_SPT)
} CTL_VIE_DATAFORMAT_SPT;

/*
    function support
*/
typedef enum {
	CTL_VIE_FUNC_SPT_NONE          	= 0x00000000, // none
	CTL_VIE_FUNC_SPT_DVI			= 0x00000001, // dvi
	CTL_VIE_FUNC_SPT_FLIP_H        	= 0x00000002, // horizontal flip
	CTL_VIE_FUNC_SPT_FLIP_V        	= 0x00000004, // vertical flip
	CTL_VIE_FUNC_SPT_YCCENC			= 0x00000008, // ycc encode
	CTL_VIE_FUNC_SPT_SINGLE_OUT    	= 0x00000010, // single out
	CTL_VIE_FUNC_SPT_PATGEN			= 0x00000020, // patgen
} CTL_VIE_FUNC_SPT;

typedef enum {
	CTL_VIE_SPT_ITEM_DATAFORMAT,        // data_type: CTL_VIE_DATAFORMAT_SPT
	CTL_VIE_SPT_ITEM_DATAFORMAT_YCCENC, // data_type: CTL_VIE_DATAFORMAT_SPT
	CTL_VIE_SPT_ITEM_FUNC,              // data_type: CTL_VIE_FUNC_SPT
	CTL_VIE_SPT_ITEM_LIMIT,             // data_type: CTL_VIE_LIMIT
	ENUM_DUMMY4WORD(CTL_VIE_SPT_ITEM)
} CTL_VIE_SPT_ITEM;

/*
    ctl_vie support function or parameters

    input :
        CTL_VIE_ID id
        CTL_VIE_SPT_ITEM item
    output :
        void *spt
*/
void ctl_vie_spt(CTL_VIE_ID id, CTL_VIE_SPT_ITEM item, void *spt);

#endif //_CTL_VIE_SPT_H_
