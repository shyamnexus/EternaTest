/**
	@brief Header file of debug function.\n
	This file contains the debug function, and debug menu entry point.

	@file hd_debug.h

	@ingroup mhdal

	@note Nothing.

	Copyright Novatek Microelectronics Corp. 2018.  All rights reserved.
*/

#ifndef _NVTDIS_API_H_
#define _NVTDIS_API_H_

#include "hd_debug.h"
#include "hdal.h"

////////////////////////////////////
#define DIS_SAVE_MAX  0                ///< for debug


#define NVTDIS_MAX_IN_W  1280       ///< max dis input width
#define NVTDIS_MAX_IN_H  720        ///< max dis input high

typedef enum _NVTDIS_ID {
	NVTDIS_0 = 0,           ///< dis id
    NVTDIS_ID_MAX,
} NVTDIS_ID;

typedef enum _NVTDIS_LUT_SIZE_SEL{
	NVTDIS_LUT_9x9 = 9,             ///< dis output lookup table 9x9
	NVTDIS_LUT_2X2 = 2,             ///< dis output lookup table 2x2
	NVTDIS_LUT_65x65 = 65,           ///< dis output lookup table 65x65
	NVTDIS_LUT_129X129 = 129,         ///< dis output lookup table 129x129
	NVTDIS_LUT_257X257 = 257,         ///< dis output lookup table 257x257
	NVTDIS_LUT_MAX_NUM,
} NVTDIS_LUT_SIZE_SEL;


typedef enum _NVTDIS_COMPENSATE {
	NVTDIS_COMPENSATE_ON = 0,        ///< dis compensate on
	NVTDIS_COMPENSATE_OFF,           ///< dis compensate off
	NVTDIS_COMPENSATE_RESET,         ///< dis compensate reset
    NVTDIS_COMPENSATE_MAX,
} NVTDIS_COMPENSATE;

/* LUT Structure */
typedef struct _NVTDIS_2DLUT_S {
	uintptr_t u64Va ;
	UINT32 lut_sz;
} NVTDIS_2DLUT_S;

typedef struct _NVTDIS_INFO {
	NVTDIS_ID id;                   ///< dis id
	HD_DIM in_dim;                      ///< dis input frame size
	HD_DIM out_dim;                     ///< dis 2dlut apply frame size
	NVTDIS_LUT_SIZE_SEL lut2d_sel;  ///< 2dlut size
	NVTDIS_2DLUT_S  default_lut;           ///< set defalut lut for distortion correct,
	float           ratio;          ///< set dis crop ratio ,FOV(Field Of View¡^
} NVTDIS_INFO;

typedef struct _NVTDIS_OPEN_CFG {
	NVTDIS_ID id; 	          ///< dis id
    UINTPTR pa;                   ///< work buffer physical addr
    UINT32 buf_size;              ///< buffer length,get from NVTDIS_PARAM_WORKBUF
} NVTDIS_OPEN_CFG;

typedef struct _NVTDIS_LUT {
    UINTPTR buf_addr;             ///< lut buffer virtual addr
    UINT32 buf_size;              ///< buffer size
	HD_DIM dim;                   ///< lut dimension
} NVTDIS_LUT_BUF;

typedef struct _NVTDIS_LUT_TRANS {
	NVTDIS_LUT_BUF  in;	                ///< intput lut
	NVTDIS_LUT_BUF  out;	            ///< transfer lut
	NVTDIS_LUT_SIZE_SEL lut2d_sel;  ///< 2dlut size
} NVTDIS_LUT_TRANS;

typedef struct _NVTDIS_WORKBUF_Q {
	NVTDIS_ID id; 	          ///< dis id
    UINT32 buf_size;          ///< buffer length,should set NVTDIS_PARAM_INFO first
} NVTDIS_WORKBUF_Q;

typedef struct _NVTDIS_COMPENSATE_S {
	NVTDIS_ID id; 	            ///< dis id
    NVTDIS_COMPENSATE op;       ///< set operation,refer to NVTDIS_COMPENSATE
} NVTDIS_COMPENSATE_S;

typedef struct _NVTDIS_DEV_ID {
	NVTDIS_ID id; 	            ///< dis id
    UINT32 device_id;           ///< set vprc device id
    UINT32 isp_id;              ///< set vprc isp id
} NVTDIS_DEV_ID;

typedef struct _NVTDIS_2DLUT_DISTORT_TABLE {
	NVTDIS_2DLUT_S lut2d;
	float factor;
} NVTDIS_2DLUT_DISTORT_TABLE;

typedef struct _NVTDIS_2DLUT_DISTORT_INFO {
	NVTDIS_ID id; 	             ///< dis id
    NVTDIS_2DLUT_DISTORT_TABLE* lut2d_table;
	UINT32 table_count;
	float min_factor;
	float max_factor;
	float ref_factor;
} NVTDIS_2DLUT_DISTORT_INFO;

//=========================================
#define LIB_PARAM   0x10000000
typedef enum _NVTDIS_PARAM_ITEM{
    ////// for vendor dis setting
	NVTDIS_PARAM_INFO,              ///< support get/set,set/get dis information for library
    NVTDIS_PARAM_LUT_TRANS,         ///< support get,transfer lut to different resolution
    NVTDIS_PARAM_WORKBUF,	        ///< support get,for query working buffer after set NVTDIS_PARAM_INFO and before nvtdis_open
	NVTDIS_PARAM_PERF,              ///< support set,for debug perf time
	NVTDIS_PARAM_SAVE_DIS,          ///< support set,for debug dump dis path frame depend on DIS_SAVE_MAX count
	NVTDIS_PARAM_SAVE_MAIN,         ///< support set,for debug dump main path frame depend on DIS_SAVE_MAX count
	NVTDIS_PARAM_COMPENSATE,        ///< support set,for control dis lut outupt,on: compensate lut,off: identity lut,reset:alg reset to calucaulat
	NVTDIS_PARAM_DIS_ENC_PATH,      ///< support set,for debug DIS streaming
	////// for nvdis lib setting
    NVTDIS_PARAM_VPEL_ID = LIB_PARAM,///< support set vpe-lite device id
    NVTDIS_PARAM_IME_ID,            ///< support set 3dnr device id
    NVTDIS_PARAM_UPDATE_DISTORT_LUT,///< support set,for update distortion lut
	ENUM_DUMMY4WORD(NVTDIS_PARAM_ID)
} NVTDIS_PARAM_ID;

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
        NVTDIS_OPEN_CFG dis_cfg = {0};

        dis_cfg.id = DIS_PATH;
        dis_cfg.pa = dis_work_pa;
        dis_cfg.va = dis_work_va;
        dis_cfg.buf_size = dis_work_size;
        ret = nvtdis_open(&dis_cfg);
	    if (ret != HD_OK) {
		    printf("nvtdis_open=%d\n", ret);
		    goto exit;
	    }
    }
    @endcode
*/
extern HD_RESULT nvtdis_open(NVTDIS_OPEN_CFG *p_open_cfg);

/**
    Close Dis.

    @param id: dis id.

    @return HD_OK for success, < 0 when some error happened, the error code can reference HD_RESULT.

    Example:
    @code
    {
        ret = nvtdis_close(DIS_PATH);
        if (HD_OK != ret) {
            DBG_ERR("close dis %d\r\n",ret);
        }
    }
    @endcode
*/
extern HD_RESULT nvtdis_close(NVTDIS_ID id);

/**
    start Dis.

    @param id: dis id.
    @param id: path_id,the dis reference path 720p for dis library input stream.ex:HD_VIDEOPROC_0_OUT_1


    @return HD_OK for success, < 0 when some error happened, the error code can reference HD_RESULT.

    Example:
    @code
    {
        ret = nvtdis_start(DIS_PATH,HD_VIDEOPROC_0_OUT_1);
        if (HD_OK != ret) {
            DBG_ERR("start dis %d\r\n",ret);
        }
    }
    @endcode
*/
extern HD_RESULT nvtdis_start(NVTDIS_ID id,HD_PATH_ID path_id);

/**
    stop Dis.

    @param id: dis id.

    @return HD_OK for success, < 0 when some error happened, the error code can reference HD_RESULT.

    Example:
    @code
    {
        ret = nvtdis_stop(DIS_PATH);

        if (HD_OK != ret) {
            DBG_ERR("stop dis %d\r\n",ret);
        }
    }
    @endcode
*/
extern HD_RESULT nvtdis_stop(NVTDIS_ID id);

/**
    set Dis parameter.

    @param id: param_id.refer to NVTDIS_PARAM_ID
    @param p_param: the config paramter structure, refer to NVTDIS_PARAM_ID introduction

    @return HD_OK for success, < 0 when some error happened, the error code can reference HD_RESULT.

    Example:
    @code
    {
        NVTDIS_INFO dis_info = {0};

        dis_info.id = DIS_PATH;
        dis_info.in_dim.w = DIS_VDO_SIZE_W;
        dis_info.in_dim.h = DIS_VDO_SIZE_H;
        dis_info.out_dim.w = VDO_SIZE_W;
        dis_info.out_dim.h = VDO_SIZE_H;
        dis_info.lut2d_sel = DIS_2DLUT_SZ_SEL;

        ret = nvtdis_set(NVTDIS_PARAM_INFO,&dis_info);
    	if (ret != HD_OK) {
    		printf("NVTDIS_PARAM_INFO fail=%d\n", ret);
    		goto exit;
    	}
    }
    @endcode
*/
extern HD_RESULT nvtdis_set(NVTDIS_PARAM_ID param_id, void *p_param);


/**
    get Dis parameter.

    @param id: param_id.refer to NVTDIS_PARAM_ID
    @param p_param: the config paramter structure, refer to NVTDIS_PARAM_ID introduction

    @return HD_OK for success, < 0 when some error happened, the error code can reference HD_RESULT.

    Example:
    @code
    {
        NVTDIS_LUT_TRANS trans= {0};

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

        ret =nvtdis_get(NVTDIS_PARAM_LUT_TRANS,&trans);

        if (HD_OK != ret) {
            DBG_ERR("get dis %d\r\n",ret);
        }
    }
    @endcode
*/
extern HD_RESULT nvtdis_get(NVTDIS_PARAM_ID param_id, void *p_param);

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
        		ret = nvtdis_pull_out_buf(p_stream0->proc0_path, &video_frame, -1,DIS_PATH, &dis_queue_valid); // -1 = blocking mode
        		if (ret != HD_OK) {
        			//if (ret != HD_ERR_UNDERRUN)
        			printf("nvtdis_pull_out_buf error=%d !!\r\n\r\n", ret);
            		goto skip3;
        		}
                // push to vpe
                ret = nvtdis_release_out_buf(p_stream0->proc0_path, &video_frame, dis_queue_valid);
        		if (ret != HD_OK) {
        			printf("proc_release error=%d !!\r\n\r\n", ret);
        		}
        	}
        }
    }
    @endcode
*/

extern HD_RESULT nvtdis_pull_out_buf(HD_PATH_ID path_id, HD_VIDEO_FRAME* p_video_frame_out, INT32 wait_ms,NVTDIS_ID dis_id, BOOL *p_dis_queue_valid);

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
        		ret = nvtdis_pull_out_buf(p_stream0->proc0_path, &video_frame, -1,DIS_PATH, &dis_queue_valid); // -1 = blocking mode
        		if (ret != HD_OK) {
        			//if (ret != HD_ERR_UNDERRUN)
        			printf("nvtdis_pull_out_buf error=%d !!\r\n\r\n", ret);
            		goto skip3;
        		}
                // push to vpe
                ret = nvtdis_release_out_buf(p_stream0->proc0_path, &video_frame, dis_queue_valid);
        		if (ret != HD_OK) {
        			printf("proc_release error=%d !!\r\n\r\n", ret);
        		}
        	}
        }
    }
    @endcode
*/
extern HD_RESULT nvtdis_release_out_buf(HD_PATH_ID path_id, HD_VIDEO_FRAME* p_video_frame, BOOL release_dis_queue);

#endif
