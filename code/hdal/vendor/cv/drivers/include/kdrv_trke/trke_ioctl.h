#ifndef __TRKE_IOCTL_CMD_H_
#define __TRKE_IOCTL_CMD_H_

#include "kwrap/ioctl.h"
#include <kwrap/nvt_type.h>

#define NEW_TRKE_IOCTL_IF 1

#if defined (__FREERTOS)
#define NVTMPP_OPEN(...) 0
#define NVTMPP_IOCTL nvtmpp_ioctl
#define NVTMPP_CLOSE(...)

#define NVTTRKE_OPEN(...) 0
#define NVTTRKE_IOCTL nvt_trke_ioctl
#define NVTTRKE_CLOSE(...)
#else
#define NVTMPP_OPEN  open
#define NVTMPP_IOCTL ioctl
#define NVTMPP_CLOSE close

#define NVTTRKE_OPEN  open
#define NVTTRKE_IOCTL ioctl
#define NVTTRKE_CLOSE close
#endif

/* job status */
enum trke_drv_sts {
    TRKE_DRV_STS_IDLE = 0,
    TRKE_DRV_STS_QUEUE,     // not process yet, in job list
    TRKE_DRV_STS_PROC,		// ready to process
    TRKE_DRV_STS_PROC_END,	// process done
    TRKE_DRV_STS_DONE,		// done by hw process
    TRKE_DRV_STS_FLUSH,     // stop by driver(not done)
    TRKE_DRV_STS_ERROR,		// error by driver(not done)
    TRKE_DRV_STS_OK,
	TRKE_DRV_STS_QFULL,
    TRKE_DRV_STS_MAX
};

#define TRKE_DRV_LAYER_MAX_NUM 6// 4
#define TRKE_DRV_STATUS_MAX_NUM 500
#define TRKE_DRV_STATUS_REG_CNT ((TRKE_DRV_STATUS_MAX_NUM + 31) / 32)

struct trke_drv_roi {
	UINT16 w;
	UINT16 h;
};

struct trke_drv_dma_img {
	//uintptr_t va;
	uintptr_t pa;
	UINT8 ddr_id;
	UINT16 lofs;		//lofs[2]: only valid at out_info
};

struct trke_drv_dma_data {
	//uintptr_t va;
	uintptr_t pa;
	UINT8 ddr_id;
};

struct trke_drv_ctl_param {
    UINT8 pyr_idx;
    UINT16 point_num;
    UINT16 max_search_range; //remove in 539A
    UINT16 iter_cnt;
    UINT8 is_initial_flow;
    UINT8 is_skip_point;
	UINT16 eps;
    UINT16 min_eig_val;
    BOOL  max_sr_disable;
    UINT8 patch_size; // 0:7x7, 1:15x15, 2: 21x21, 3: 31x31
    UINT16 pyr_search_range;
    UINT16 layer_search_range;
};

struct trke_drv_in_info {
	struct trke_drv_roi roi;
	struct trke_drv_dma_img prev_pyrmd;
	struct trke_drv_dma_img next_pyrmd;
    struct trke_drv_dma_data prev_points;
    struct trke_drv_dma_data initial_flow;
    //UINT8 is_initial;
};

struct trke_drv_out_info {
    struct trke_drv_dma_data next_points;
    struct trke_drv_dma_data err;
};

struct trke_layer_info {
    struct trke_drv_in_info in;
    struct trke_drv_out_info out;
    struct trke_drv_ctl_param ctl_param;
};

typedef struct _trke_param_cfg {
    UINT8 total_pyr_level;
    struct trke_layer_info layer[TRKE_DRV_LAYER_MAX_NUM];
    UINT32 init_status[TRKE_DRV_STATUS_REG_CNT];
	//uintptr_t output_status_pa;
	//int apply_next; //1: get result and set to next job
	//void* next_job_cfg;   //trke_param_cfg , if not have next job, set this to null
} trke_param_cfg;

typedef struct _trke_drv_process_cfg {
//----------set------------------------
	UINT32 chip;
	UINT32 eng;
	UINT32 id;
    BOOL instant; // 0: non-blocking 1: blocking
    //UINT16 num; //total job number
    UINT8 priority;
    //void *job_cfg;   //trke_param_cfg, if not have next job, set this to null
    trke_param_cfg job_cfg;   //trke_param_cfg, if not have next job, set this to null
//----------get------------------------
    unsigned int handle;
}trke_drv_process_cfg;

typedef struct _trke_drv_query_cfg {
    unsigned int handle;
    BOOL instant; // 0: non-blocking 1: blocking

    /*   return value     */
    BOOL job_finish;
    UINT32 status[16];
}trke_drv_query_cfg;

//============================================================================
// IOCTL command
//============================================================================
#define TRKE_IOC_COMMON_TYPE 'M'
#if !NEW_TRKE_IOCTL_IF
#define TRKE_IOC_START					_VOS_IO(TRKE_IOC_COMMON_TYPE, 1)
#define TRKE_IOC_STOP					_VOS_IO(TRKE_IOC_COMMON_TYPE, 2)

#define TRKE_IOC_READ_REG				_VOS_IOWR(TRKE_IOC_COMMON_TYPE, 3, void*)
#define TRKE_IOC_WRITE_REG				_VOS_IOWR(TRKE_IOC_COMMON_TYPE, 4, void*)
#define TRKE_IOC_READ_REG_LIST			_VOS_IOWR(TRKE_IOC_COMMON_TYPE, 5, void*)
#define TRKE_IOC_WRITE_REG_LIST			_VOS_IOWR(TRKE_IOC_COMMON_TYPE, 6, void*)

#define TRKE_IOC_OPEN                      _VOS_IOWR(TRKE_IOC_COMMON_TYPE,  7, void*)
#define TRKE_IOC_CLOSE                     _VOS_IOWR(TRKE_IOC_COMMON_TYPE,  8, void*)
#define TRKE_IOC_OPENCFG					  _VOS_IOWR(TRKE_IOC_COMMON_TYPE,  9, void*)
#define TRKE_IOC_SET_IMG_INFO              _VOS_IOWR(TRKE_IOC_COMMON_TYPE, 10, void*)
#define TRKE_IOC_GET_IMG_INFO              _VOS_IOWR(TRKE_IOC_COMMON_TYPE, 11, void*)
#define TRKE_IOC_SET_IMG_DMA_IN            _VOS_IOWR(TRKE_IOC_COMMON_TYPE, 12, void*)
#define TRKE_IOC_GET_IMG_DMA_IN            _VOS_IOWR(TRKE_IOC_COMMON_TYPE, 13, void*)
#define TRKE_IOC_SET_IMG_DMA_OUT           _VOS_IOWR(TRKE_IOC_COMMON_TYPE, 14, void*)
#define TRKE_IOC_GET_IMG_DMA_OUT           _VOS_IOWR(TRKE_IOC_COMMON_TYPE, 15, void*)
#define TRKE_IOC_SET_STATUS_PARAM          _VOS_IOWR(TRKE_IOC_COMMON_TYPE, 16, void*)
#define TRKE_IOC_GET_STATUS_PARAM          _VOS_IOWR(TRKE_IOC_COMMON_TYPE, 17, void*)
#define TRKE_IOC_TRIGGER                   _VOS_IOWR(TRKE_IOC_COMMON_TYPE, 18, void*)
#define TRKE_IOC_SET_FUNCTION_PARAM        _VOS_IOWR(TRKE_IOC_COMMON_TYPE, 19, void*)
#define TRKE_IOC_GET_FUNCTION_PARAM        _VOS_IOWR(TRKE_IOC_COMMON_TYPE, 20, void*)
#define TRKE_IOC_GET_VERSION               _VOS_IOWR(TRKE_IOC_COMMON_TYPE, 21, void*)

#define TRKE_IOC_INIT                      _VOS_IOWR(TRKE_IOC_COMMON_TYPE, 22, void*)
#define TRKE_IOC_UNINIT                    _VOS_IOWR(TRKE_IOC_COMMON_TYPE, 23, void*)
#else
#define TRKE_IOC_GET_VERSION               _VOS_IOWR(TRKE_IOC_COMMON_TYPE, 56, void*)
#define TRKE_IOC_TRIGGER_JOB               _VOS_IOWR(TRKE_IOC_COMMON_TYPE, 60, trke_drv_process_cfg *)
#define TRKE_IOC_QUERY_JOB                 _VOS_IOWR(TRKE_IOC_COMMON_TYPE, 61, trke_drv_query_cfg *)
#endif
/* Add other command ID here*/
#if defined(__FREERTOS)
int nvt_trke_ioctl(int fd, unsigned int uiCmd, void *p_arg);
extern INT32 kdrv_trke_rtos_init(void);
extern INT32 kdrv_trke_rtos_uninit(void);
#endif


#endif
