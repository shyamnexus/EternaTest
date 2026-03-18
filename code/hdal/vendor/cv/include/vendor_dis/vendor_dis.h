/**
	@brief Header file of debug function.\n
	This file contains the debug function, and debug menu entry point.

	@file hd_debug.h

	@ingroup mhdal

	@note Nothing.

	Copyright Novatek Microelectronics Corp. 2018.  All rights reserved.
*/

#ifndef _VENDOR_DIS_PROCESS_H_
#define _VENDOR_DIS_PROCESS_H_

#include "hd_debug.h"
#include "hdal.h"

////////////////////////////////////
#define DIS_SAVE_MAX  0                ///< for debug


#define VENDOR_DIS_MAX_IN_W  1280       ///< max dis input width
#define VENDOR_DIS_MAX_IN_H  720        ///< max dis input high

typedef enum _VENDOR_DIS_ID {
	VENDOR_DIS_0 = 0,           ///< dis id
    VENDOR_DIS_ID_MAX,
} VENDOR_DIS_ID;

typedef enum _VENDOR_DIS_LUT_SIZE_SEL{
	VENDOR_DIS_LUT_9x9 = 9,             ///< dis output lookup table 9x9
	VENDOR_DIS_LUT_2X2 = 2,             ///< dis output lookup table 2x2
	VENDOR_DIS_LUT_65x65 = 65,           ///< dis output lookup table 65x65
	VENDOR_DIS_LUT_129X129 = 129,         ///< dis output lookup table 129x129
	VENDOR_DIS_LUT_257X257 = 257,         ///< dis output lookup table 257x257
	VENDOR_DIS_LUT_MAX_NUM,
} VENDOR_DIS_LUT_SIZE_SEL;

typedef enum _VENDOR_DIS_COMPENSATE {
	VENDOR_DIS_ON = 0,        ///< dis compensate on
	VENDOR_DIS_OFF,           ///< dis compensate off
	VENDOR_DIS_RESET,         ///< dis compensate reset
	VENDOR_DIS_MAX,
} VENDOR_DIS_COMPENSATE;

/* LUT Structure */
typedef struct _VENDOR_DIS_2DLUT_S {
	uintptr_t u64Va ;
	UINT32 lut_sz;
} VENDOR_DIS_2DLUT_S;

typedef struct _VENDOR_DIS_INFO {
	VENDOR_DIS_ID id;                   ///< dis id
	HD_DIM in_dim;                      ///< dis input frame size
	HD_DIM out_dim;                     ///< dis 2dlut apply frame size
	VENDOR_DIS_LUT_SIZE_SEL lut2d_sel;  ///< 2dlut size
	VENDOR_DIS_2DLUT_S  default_lut;           ///< set defalut lut for distortion correct,
	float               ratio;          ///< set dis crop ratio ,FOV(Field Of View!^
} VENDOR_DIS_INFO;

typedef struct _VENDOR_DIS_OPEN_CFG {
	VENDOR_DIS_ID id; 	          ///< dis id
    UINTPTR pa;                   ///< work buffer physical addr
    UINT32 buf_size;              ///< buffer length,get from VENDOR_DIS_PARAM_WORKBUF
} VENDOR_DIS_OPEN_CFG;

typedef struct _VENDOR_DIS_LUT {
    UINTPTR buf_addr;             ///< lut buffer virtual addr
    UINT32 buf_size;              ///< buffer size
	HD_DIM dim;                   ///< lut dimension
} VENDOR_LUT_BUF;

typedef struct _VENDOR_DIS_LUT_TRANS {
	VENDOR_LUT_BUF  in;	                ///< intput lut
	VENDOR_LUT_BUF  out;	            ///< transfer lut
	VENDOR_DIS_LUT_SIZE_SEL lut2d_sel;  ///< 2dlut size
} VENDOR_DIS_LUT_TRANS;

typedef struct _VENDOR_DIS_WORKBUF_Q {
	VENDOR_DIS_ID id; 	          ///< dis id
    UINT32 buf_size;              ///< buffer length,should set VENDOR_DIS_PARAM_INFO first
} VENDOR_DIS_WORKBUF_Q;

typedef struct _VENDOR_DIS_COMPENSATE_S {
	VENDOR_DIS_ID id; 	            ///< dis id
    VENDOR_DIS_COMPENSATE op;       ///< set operation,refer to VENDOR_DIS_COMPENSATE
} VENDOR_DIS_COMPENSATE_S;

typedef struct _VENDOR_DIS_2DLUT_DISTORT_TABLE {
	VENDOR_DIS_2DLUT_S lut2d;
	float factor;
} VENDOR_DIS_2DLUT_DISTORT_TABLE;

typedef struct _VENDOR_DIS_2DLUT_DISTORT_INFO {
	VENDOR_DIS_2DLUT_DISTORT_TABLE* lut2d_table;
	UINT32 table_count;
	float min_factor;
	float max_factor;
	float ref_factor;
	VENDOR_DIS_2DLUT_S lut2d_undistor_addr;
} VENDOR_DIS_2DLUT_DISTORT_INFO;

// for debug dump vpe output frame depend on DIS_SAVE_MAX count
UINT32 _dis_queue_vpe(HD_VIDEO_FRAME *p_video_frame, UINT32 count, HD_VIDEO_FRAME *p_queue);
UINT32 _dis_save_all_queued_vpe(HD_VIDEO_FRAME *p_queue, HD_PATH_ID path_id);
//=========================================

typedef enum _VENDOR_DIS_PARAM_ITEM{
	VENDOR_DIS_PARAM_INFO,              ///< support get/set,set/get dis information for library
    VENDOR_DIS_PARAM_LUT_TRANS,         ///< support get,transfer lut to different resolution
    VENDOR_DIS_PARAM_WORKBUF,	        ///< support get,for query working buffer after set VENDOR_DIS_PARAM_INFO and before vendor_dis_open
	VENDOR_DIS_PARAM_PERF,              ///< support set,for debug perf time
	VENDOR_DIS_PARAM_SAVE_DIS,          ///< support set,for debug dump dis path frame depend on DIS_SAVE_MAX count
	VENDOR_DIS_PARAM_SAVE_MAIN,         ///< support set,for debug dump main path frame depend on DIS_SAVE_MAX count
	VENDOR_DIS_PARAM_COMPENSATE,        ///< support set,for control dis lut outupt,on: compensate lut,off: identity lut,reset:alg reset to calucaulat
	VENDOR_DIS_PARAM_DIS_ENC_PATH,      ///< support set,for debug DIS streaming
	VENDOR_DIS_PARAM_UPDATE_DISTORT_LUT,///< support get,for update distortion lut
	ENUM_DUMMY4WORD(VENDOR_DIS_PARAM_ID)
} VENDOR_DIS_PARAM_ID;

/********************************************************************
	EXTERN VARIABLES & FUNCTION PROTOTYPES DECLARATIONS
********************************************************************/
/**
    Open Dis.

    @param p_open_cfg: cfg parameter for  open.

    @return HD_OK for success, < 0 when some error happened, the error code can reference HD_RESULT.

    Example:
    @code
    {
        VENDOR_DIS_OPEN_CFG dis_cfg = {0};

        dis_cfg.id = DIS_PATH;
        dis_cfg.pa = dis_work_pa;
        dis_cfg.va = dis_work_va;
        dis_cfg.buf_size = dis_work_size;
        ret = vendor_dis_open(&dis_cfg);
	    if (ret != HD_OK) {
		    printf("vendor_dis_open=%d\n", ret);
		    goto exit;
	    }
    }
    @endcode
*/
extern HD_RESULT vendor_dis_open(VENDOR_DIS_OPEN_CFG *p_open_cfg);

/**
    Close Dis.

    @param id: dis id.

    @return HD_OK for success, < 0 when some error happened, the error code can reference HD_RESULT.

    Example:
    @code
    {
        ret = vendor_dis_close(DIS_PATH);
        if (HD_OK != ret) {
            DBG_ERR("close dis %d\r\n",ret);
        }
    }
    @endcode
*/
extern HD_RESULT vendor_dis_close(VENDOR_DIS_ID id);

/**
    start Dis.

    @param id: dis id.
    @param id: path_id,the dis reference path 720p.the path should open start/stop close by user,if need distortion, user should corret by themselves
               vendor_dis_start only pull refernce path frame to dis lib

    @return HD_OK for success, < 0 when some error happened, the error code can reference HD_RESULT.

    Example:
    @code
    {
        ret = vendor_dis_start(DIS_PATH,HD_VIDEOPROC_0_OUT_1);
        if (HD_OK != ret) {
            DBG_ERR("start dis %d\r\n",ret);
        }
    }
    @endcode
*/
extern HD_RESULT vendor_dis_start(VENDOR_DIS_ID id,HD_PATH_ID path_id);

/**
    stop Dis.

    @param id: dis id.

    @return HD_OK for success, < 0 when some error happened, the error code can reference HD_RESULT.

    Example:
    @code
    {
        ret = vendor_dis_stop(DIS_PATH);

        if (HD_OK != ret) {
            DBG_ERR("stop dis %d\r\n",ret);
        }
    }
    @endcode
*/
extern HD_RESULT vendor_dis_stop(VENDOR_DIS_ID id);

/**
    set Dis parameter.

    @param id: param_id.refer to VENDOR_DIS_PARAM_ID
    @param p_param: the config paramter structure, refer to VENDOR_DIS_PARAM_ID introduction

    @return HD_OK for success, < 0 when some error happened, the error code can reference HD_RESULT.

    Example:
    @code
    {
        VENDOR_DIS_INFO dis_info = {0};

        dis_info.id = DIS_PATH;
        dis_info.in_dim.w = DIS_VDO_SIZE_W;
        dis_info.in_dim.h = DIS_VDO_SIZE_H;
        dis_info.out_dim.w = VDO_SIZE_W;
        dis_info.out_dim.h = VDO_SIZE_H;
        dis_info.lut2d_sel = DIS_2DLUT_SZ_SEL;

        ret = vendor_dis_set(VENDOR_DIS_PARAM_INFO,&dis_info);
    	if (ret != HD_OK) {
    		printf("VENDOR_DIS_PARAM_INFO fail=%d\n", ret);
    		goto exit;
    	}
    }
    @endcode
*/
extern HD_RESULT vendor_dis_set(VENDOR_DIS_PARAM_ID param_id, void *p_param);


/**
    get Dis parameter.

    @param id: param_id.refer to VENDOR_DIS_PARAM_ID
    @param p_param: the config paramter structure, refer to VENDOR_DIS_PARAM_ID introduction

    @return HD_OK for success, < 0 when some error happened, the error code can reference HD_RESULT.

    Example:
    @code
    {
        VENDOR_DIS_LUT_TRANS trans= {0};

            //transfer distort_lut to 720
        trans.in.buf_addr  = (UINTPTR)&distort_lut[0];
        trans.in.buf_size  = sizeof(distort_lut);
        trans.in.dim.w = VDO_SIZE_W;
        trans.in.dim.h = VDO_SIZE_H;
        trans.out.buf_addr  = (UINTPTR)&distort_lut_720[0];;
        trans.out.buf_size  = sizeof(distort_lut_720);
        trans.out.dim.w = DIS_VDO_SIZE_W;
        trans.out.dim.h = DIS_VDO_SIZE_H;
        trans.lut2d_sel  = DIS_2DLUT_SZ_SEL;

        ret =vendor_dis_get(VENDOR_DIS_PARAM_LUT_TRANS,&trans);

        if (HD_OK != ret) {
            DBG_ERR("get dis %d\r\n",ret);
        }
    }
    @endcode
*/
extern HD_RESULT vendor_dis_get(VENDOR_DIS_PARAM_ID param_id, void *p_param);

/**
    pull vprc frame with dis lookup table and push to vpe to corret.

    @param id: path_id.the main path id for push to vpe
    @param p_video_frame_out: get frame
    @param dis_queue_valid: return valid lut,for release buffer

    @return HD_OK for success, < 0 when some error happened, the error code can reference HD_RESULT.

    Example:
    @code
    {
        static void *process_thread(void *arg)
        {
        	VIDEO_DIS_RECORD* p_stream0 = (VIDEO_DIS_RECORD *)arg;
        	VIDEO_DIS_RECORD* p_stream1 = p_stream0+1;


        	//--------- pull data test ---------
        	while (p_stream0->prc_exit == 0) {
        		//printf("proc_pull ....\r\n");
        		ret = vendor_dis_pull_out_buf(p_stream0->proc0_path, &video_frame, -1,DIS_PATH, &dis_queue_valid); // -1 = blocking mode
        		if (ret != HD_OK) {
        			//if (ret != HD_ERR_UNDERRUN)
        			printf("vendor_dis_pull_out_buf error=%d !!\r\n\r\n", ret);
            		goto skip3;
        		}
                // push to vpe
                ret = vendor_dis_release_out_buf(p_stream0->proc0_path, &video_frame, dis_queue_valid);
        		if (ret != HD_OK) {
        			printf("proc_release error=%d !!\r\n\r\n", ret);
        		}
        	}
        }
    }
    @endcode
*/

extern HD_RESULT vendor_dis_pull_out_buf(HD_PATH_ID path_id, HD_VIDEO_FRAME* p_video_frame_out, INT32 wait_ms,VENDOR_DIS_ID dis_id, BOOL *p_dis_queue_valid);

/**
    release vprc frame with dis lookup table .

    @param id: path_id.the main path id
    @param p_video_frame: the frame which need to release
    @param dis_queue_valid: valid lut,get from pull for release buffer

    @return HD_OK for success, < 0 when some error happened, the error code can reference HD_RESULT.

    Example:
    @code
    {
        static void *process_thread(void *arg)
        {
        	VIDEO_DIS_RECORD* p_stream0 = (VIDEO_DIS_RECORD *)arg;
        	VIDEO_DIS_RECORD* p_stream1 = p_stream0+1;


        	//--------- pull data test ---------
        	while (p_stream0->prc_exit == 0) {
        		//printf("proc_pull ....\r\n");
        		ret = vendor_dis_pull_out_buf(p_stream0->proc0_path, &video_frame, -1,DIS_PATH, &dis_queue_valid); // -1 = blocking mode
        		if (ret != HD_OK) {
        			//if (ret != HD_ERR_UNDERRUN)
        			printf("vendor_dis_pull_out_buf error=%d !!\r\n\r\n", ret);
            		goto skip3;
        		}
                // push to vpe
                ret = vendor_dis_release_out_buf(p_stream0->proc0_path, &video_frame, dis_queue_valid);
        		if (ret != HD_OK) {
        			printf("proc_release error=%d !!\r\n\r\n", ret);
        		}
        	}
        }
    }
    @endcode
*/
extern HD_RESULT vendor_dis_release_out_buf(HD_PATH_ID path_id, HD_VIDEO_FRAME* p_video_frame, BOOL release_dis_queue);


#endif
