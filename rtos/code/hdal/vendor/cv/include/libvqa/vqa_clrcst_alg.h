/*
* Copyright  Novatek Microelectronics Corp. 2022.  All rights reserved.    
*----------------------------------------------------------------------
* Name: VQA CLRCST Library Header                                           
* Author: IVoT CVAI/PF
*/

#ifndef _VQA_CLRCST_ALG_H
#define _VQA_CLRCST_ALG_H

#include "hd_type.h"

#if defined(__LINUX)
#include <sys/mman.h>
#include <pthread.h>
#endif
#if defined(__FREERTOS)
#include <FreeRTOS_POSIX.h>
#include <FreeRTOS_POSIX/pthread.h>
#endif

#define MAX_CA_DATA_NUM 30
#define VQA_MAX_CH_NUM		1
#define VQA_CLRCST_INFO	0

#define CLRCST_HIS_SIZE (360+1)

#define CELL_W_WINNUM  32


typedef struct _CLRCST_RSLT {
        UINT16 r[CELL_W_WINNUM*CELL_W_WINNUM];
        UINT16 g[CELL_W_WINNUM*CELL_W_WINNUM];
        UINT16 b[CELL_W_WINNUM*CELL_W_WINNUM];
        UINT16 ir[CELL_W_WINNUM*CELL_W_WINNUM];
        UINT16 acc_cnt[CELL_W_WINNUM*CELL_W_WINNUM];
} CLRCST_RSLT;


typedef struct _CLRCST_DATA {
        UINT32 id;
        CLRCST_RSLT clrcst_rslt;
} CLRCST_DATA;

/**
    Color-Cast buffer parameter
    Structure of parameter of Color-Cast
*/

typedef struct _CLRCST_BUF_S {
        UINT32 ca_num;                          ///< ca buffer number
        CLRCST_DATA *clrcst_data;               ///< clrcst data buffer pointer (start pointer)
        UINT32 rd_cnt;                          ///< read count of ring buffer
        UINT32 w_cnt;                           ///< write count of ring buffer
        UINT32 is_full;                         ///< ring buffer full
        UINT32 is_empty;                        ///< ring buffer empty
} CLRCST_BUF_S;


/* Color-Cast state machine  */
typedef enum {
	CL_NORM              = 0,    ///< normal state
	CL_NORM_WARN         = 1,    ///< normal to warning state
	CL_ALRM              = 2,    ///< alarm state
	CL_ALRM_WARN         = 3,    ///< alarm to warning state
} CLRCST_STATE_M;

/* Color-Cast state machine AB buffer */
typedef enum {
	CL_INIT              = 0,    ///< init state
	CL_CLT_A             = 1,    ///< colect A
	CL_CLT_B             = 2,    ///< colect B
	CL_DET_A             = 3,    ///< detect A
	CL_DET_B             = 4,    ///< detect B
} CLRCST_STATE_1_M;

/* Color-Cast detection mode */
typedef enum {
	CL_M_H                     = 0,    ///< h onlyA
	CL_M_H_AND_S               = 1,    ///< h*s
} CLRCST_DET_MODE;

/* Color-Cast state machine*/
typedef enum {
	CL_BUF_A             = 0,    ///< buffer A
	CL_BUF_B             = 1,    ///< buffer B
} CLRCST_BUF_M;

/* Color-Cast hsv value
   Structure of hsv of Color-Cast */
typedef struct CLRCST_HSV {
	UINT32       *h;  ///< h difference of cell
	UINT32       *s;  ///< s difference of cell
	UINT32       *v;  ///< v difference of cell
} CLRCST_HSV;

/*  Color-Cast buffer parameter
    Structure of parameter of Color-Cast */
typedef struct CLRCST_BUF {
	UINT32 ca_num;				///< ca buffer number
	UINT32 rd_cnt;				///< read count of ring buffer
	UINT32 w_cnt;				///< write count of ring buffer
	UINT32 is_full;				///< ring buffer full
	UINT32 is_empty;			///< ring buffer empty
} CLRCST_BUF;

/*  Color-Cast buffer difference
    Structure of buffer difference of Color-Cast */
typedef struct _CLRCST_BUF_DIFF {
	UINT32       *d_x;  ///< r/h difference of cell
	UINT32       *d_y;  ///< g/s difference of cell
	UINT32       *d_z;  ///< b/v difference of cell
	
	UINT32      diff_th;   //< difference threshold
	UINT32      diff_th1;  //< difference threshold (white)
	UINT32      diff_area; //< the percent of area of diff
	UINT32      diff_pixl; //< number of area of diff
	UINT32      diff_pixl1;  //< number of area of diff (white:30~180)
	UINT32      diff_pixl2;  //< number of area of diff without mask
	UINT8       is_diff;  //< the color-cast detection was happened.
	UINT8       is_white; //< the pixel is approach to white color.
	UINT32      x_cnt;        //< count the number of r/h_value > 0

	UINT32      x_cnt_bypass; //< count the area that was not checkek.
	UINT32      y_cnt;        //< count the number of g/s_value > 0
	UINT32      z_cnt;        //< count the number of b/v_value > 0

	UINT32      det_mode;     //< detection mode

	UINT32	    hsi_max;
	UINT32      hsi_max_idx;
	UINT32      hsi_equal_area;
} CLRCST_BUF_DIFF;

typedef struct CLRCST_HSV_BUF_SET{
        CLRCST_HSV       hsv_data_tmp;               ///< tmp hsv buffer for merging
        CLRCST_HSV       hsv_data_merge_a;           ///< hsv buffer merged at A time.
        CLRCST_HSV       hsv_data_merge_b;           ///< hsv buffer merged at B time.
        CLRCST_HSV       hsv_data_merge_background;      ///< hsv buffer of background
	UINT32 is_empty;			///< ring buffer empty
} CLRCST_HSV_BUF_SET;

/* Color-Cast parameter Structure of parameter of Color-Cast */

typedef struct CLRCST_PARM_ALG{
        CLRCST_BUF_M    buf_mode;                   ///< the buffer type of ca buffer
        CLRCST_BUF_M    buf_mode_merge;             ///< the merged buffer type (A or B)

        UINT32          hsv_merge_background_cnt;       ///< the merge count of background

        UINT32          collect_xyz_time;           ///< the  maximum ca buffer for merging.
        UINT32          collect_ca_cnt; //< the merged count for ca buffer merged

        UINT8           is_first_a;
        UINT8           is_first_b;
        UINT8           is_first_flow;

        UINT32          background_cnt;     //< the time for background of hsv_data
        UINT32          background_update;  //< the time for background of hsv_data
        			        //(default: 30 = 1sec for back-ground)

        CLRCST_STATE_M  state_main; // < main state machine for Alarm (enable/disable)

        CLRCST_STATE_1_M state_flow;        ///< flow state machine for collection and retrieved.

	CLRCST_DET_MODE det_mode;           ///< 0:Hue only, 1:Hue+Satuation
	
	UINT32          warning_cnt;        ///< the happened time of warning
        UINT32          warning_max;        ///< the maximun warning count for alarm

	CLRCST_BUF_DIFF diff_buff_hsv;      ///< the parameter for handling diff-buff. (hsv)

	UINT32	clrcst_buf_va_addr;
        UINT32	clrcst_buf_pa_addr;

	UINT8 is_init;
	UINT8 is_uninit;

	UINT32 is_debug;
} CLRCST_PARM_ALG;

typedef struct _CLRCST_PARM {
	UINT8           is_init;                    ///< 1:initilization ok
	UINT8           is_uninit;                  ///< 1:uninit ok
        CLRCST_BUF_S	clrcst_buf;
	pthread_t    	collect_ca_thread_id;       ///< thread id for handling collecting ca buffer
	UINT32       	collect_xyz_time;           ///< the  maximum ca buffer for merging.
	UINT32       	collect_ca_cnt;             ///< the merged count for ca buffer merged

	UINT32          max_yuv_width;              ///< the max width of yuv420
	UINT32          max_yuv_height;             ///< the max height of yuv420

	UINT8 		    *r_img;                     ///< r buffer
	UINT8 		    *g_img;                     ///< g buffer
	UINT8 		    *b_img;                     ///< b buffer

	UINT32          *r_tmp;                     ///<
	UINT32          *g_tmp;                     ///<
	UINT32          *b_tmp;                     ///<

	UINT8           is_buf_ready;               ///<
	UINT32          sensor_ch;                  ///< hdr is 1, others is 0.
    UINT32 		    cl_src_type;

	UINT32 		    is_debug;
} CLRCST_PARM;

extern UINT32 clrcst_state_machine_main(CLRCST_PARM_ALG *cl_parm, CLRCST_PARM *cl_parm_lib, CLRCST_HSV_BUF_SET *cl_hsv_buf_set, UINTPTR yuv_va_addr, UINT32 yuv_width, UINT32 yuv_height);
extern VOID clrcst_rgb_to_hsv(CLRCST_HSV *hsv, CLRCST_DATA *rgb);
extern VOID clrcst_input_clrcst_data(UINTPTR yuv_va_addr, UINT32 yuv_width, UINT32 yuv_height);

#endif
