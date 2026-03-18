/**
    Public header file for dal_ai_dla

    This file is the header file that defines the API and data types for vendor_ai_dla.

    @file       vendor_ai_dla.h
    @ingroup    vendor_ai_dla

    Copyright   Novatek Microelectronics Corp. 2018.    All rights reserved.
*/
#ifndef _VENDOR_AI_DLA_H_
#define _VENDOR_AI_DLA_H_

/********************************************************************
	INCLUDE FILES
********************************************************************/
#include "hd_type.h"
#include "kdrv_ai.h"
#include "ai_ioctl.h"
#include "vendor_ai_net/nn_model.h" // for old kdrv define/struct
#include "kflow_ai_net/kflow_ai_net.h"

/********************************************************************
	MACRO CONSTANT DEFINITIONS
********************************************************************/
#if 0
#define VENDOR_AI_FCD_VLC_EN              KDRV_AI_FCD_VLC_EN      		///< compression variable length encoding enable
#define VENDOR_AI_FCD_QUANTI_EN           KDRV_AI_FCD_QUANTI_EN      	///< compression quanti enable
#define VENDOR_AI_FCD_SPARSE_EN           KDRV_AI_FCD_SPARSE_EN         ///< compression sparse enable
#define VENDOR_AI_FCD_QUANTI_KMEANS_EN    KDRV_AI_FCD_QUANTI_KMEANS_EN  ///< compression quanti kmeans enable
#define VENDOR_AI_FCD_ALL_EN              KDRV_AI_FCD_ALL_EN      		///< compression all function enable

#define VENDOR_AI_NEURAL_DECONV_EN        KDRV_AI_NEURAL_DECONV_EN      ///< neural network deconvolution enable
#define VENDOR_AI_NEURAL_CONV_EN          KDRV_AI_NEURAL_CONV_EN      	///< neural network convolution enable
#define VENDOR_AI_NEURAL_NORM_EN          KDRV_AI_NEURAL_NORM_EN      	///< neural network normalized enable
#define VENDOR_AI_NEURAL_ELT_EN           KDRV_AI_NEURAL_ELT_EN      	///< neural network eltwise enable
#define VENDOR_AI_NEURAL_ACT_EN           KDRV_AI_NEURAL_ACT_EN      	///< neural network activation enable
#define VENDOR_AI_NEURAL_POOL_EN          KDRV_AI_NEURAL_POOL_EN      	///< neural network pooling enable
//#define VENDOR_AI_NEURAL_ALL_EN           KDRV_AI_NEURAL_ALL_EN      	///< neural network all function enable
#define VENDOR_AI_NEURAL_FUNC_CNT         KDRV_AI_NEURAL_FUNC_CNT       ///< number of network function

#define VENDOR_AI_FC_FULLY_EN             KDRV_AI_FC_ACT_EN      		///< fc fully connected enable
#define VENDOR_AI_FC_ACT_EN               KDRV_AI_FC_ACT_EN      		///< fc activation enable
//#define VENDOR_AI_FC_ALL_EN               KDRV_AI_FC_ALL_EN      		///< fc all function enable
#define VENDOR_AI_FC_FUNC_CNT             KDRV_AI_FC_FUNC_CNT           ///< number of fc function
#endif
/********************************************************************
	TYPE DEFINITION
********************************************************************/

#if 0
/**
    KDRV AI structure - trigger parameter
*/
typedef KDRV_AI_TRIGGER_PARAM VENDOR_AI_TRIGGER_PARAM;
#endif

/*
    AI trigger mode
*/
typedef KDRV_AI_TRIG_MODE VENDOR_AI_TRIG_MODE;

/**
    AI engine
*/
typedef KDRV_AI_ENG VENDOR_AI_ENG;

/**
    AI processing function
*/
typedef KDRV_AI_MODE VENDOR_AI_MODE;

#if 0
/**
    select input/output type
*/
typedef KDRV_AI_IO_TYPE VENDOR_AI_IO_TYPE;

/**
    source format for neural network processing
*/
typedef KDRV_AI_NEURAL_SRC_FMT VENDOR_AI_NEURAL_SRC_FMT;

/**
    convolution normalization mode
*/
typedef KDRV_AI_NORM_MODE VENDOR_AI_NORM_MODE;

/**
    select eltwise mode
*/
typedef KDRV_AI_ELT_MODE VENDOR_AI_ELT_MODE;

/**
    select activation mode.
*/
typedef KDRV_AI_ACT_MODE VENDOR_AI_ACT_MODE;

/**
    select pooling mode.
*/
typedef KDRV_AI_POOL_MODE VENDOR_AI_POOL_MODE;

/**
    select roi pooling mode
*/
typedef KDRV_AI_ROIPOOL_MODE VENDOR_AI_ROIPOOL_MODE;

/**
    select svm kernel mode
*/
typedef KDRV_AI_SVMKER_MODE VENDOR_AI_SVMKER_MODE;

/**
    select fully connect source format
*/
typedef KDRV_AI_FC_SRC_FMT VENDOR_AI_FC_SRC_FMT;

/**
    select permute ordering mode
*/
typedef KDRV_AI_PERMUTE_MODE VENDOR_AI_PERMUTE_MODE;

/**
    app configuration
*/
typedef KDRV_AI_APP_HEAD VENDOR_AI_APP_HEAD;

#endif

/**
    linked list configuration
*/
typedef KDRV_AI_LL_HEAD VENDOR_AI_LL_HEAD;
#if 0
/**
    size configuration
*/
typedef KDRV_AI_SIZE VENDOR_AI_SIZE;

/**
    offset configuration
*/
typedef KDRV_AI_OFS VENDOR_AI_OFS;

/**
    image padding kernel parameters configuration
*/
typedef KDRV_AI_PAD_KERPARM  VENDOR_AI_PAD_KERPARM ;

/**
    compression kernel parameters configuration
*/
typedef KDRV_AI_FCD_KERPARM VENDOR_AI_FCD_KERPARM;

/**
    deconvolution kernel parameters configuration
*/
typedef KDRV_AI_DECONV_KERPARM VENDOR_AI_DECONV_KERPARM;

/**
    convolution kernel parameters configuration
*/
typedef KDRV_AI_CONV_KERPARM VENDOR_AI_CONV_KERPARM;

/**
    bn scale kernel parameters configuration
*/
typedef KDRV_AI_BN_SCALE_KERPARM VENDOR_AI_BN_SCALE_KERPARM;

/**
    (for NT96680)
    LRN kernel parameters configuration
*/
typedef KDRV_AI_LRN_KERPARM VENDOR_AI_LRN_KERPARM;

/**
    convolution normalization kernel parameters configuration
*/
typedef KDRV_AI_NORM_KERPARM VENDOR_AI_NORM_KERPARM;

/**
    eltwise kernel parameters configuration
*/
typedef KDRV_AI_ELT_KERPARM VENDOR_AI_ELT_KERPARM;

/**
    relu kernel parameters configuration
*/
typedef KDRV_AI_RELU_KERPARM VENDOR_AI_RELU_KERPARM;

/**
    activation kernel parameters configuration
*/
typedef KDRV_AI_ACT_KERPARM VENDOR_AI_ACT_KERPARM;

/**
    global pooling kernel parameters configuration
*/
typedef KDRV_AI_GLOBAL_POOL_KERPARM VENDOR_AI_GLOBAL_POOL_KERPARM;

/**
    local pooling kernel parameters configuration
*/
typedef KDRV_AI_LOCAL_POOL_KERPARM VENDOR_AI_LOCAL_POOL_KERPARM;

/**
    pooling kernel parameters configuration
*/
typedef KDRV_AI_POOL_KERPARM VENDOR_AI_POOL_KERPARM;

/**
    roi pooling kernel parameters configuration
*/
typedef KDRV_AI_ROIPOOL_KERPARM VENDOR_AI_ROIPOOL_KERPARM;

/**
    svm kernel parameters configuration
*/
typedef KDRV_AI_SVM_KERPARM VENDOR_AI_SVM_KERPARM;

/**
    fully connect kernel parameters configuration
*/
typedef KDRV_AI_FC_KERPARM VENDOR_AI_FC_KERPARM;

/**
    permute kernel parameters configuration
*/
typedef KDRV_AI_PERMUTE_KERPARM VENDOR_AI_PERMUTE_KERPARM;

/**
    reorganize kernel parameters configuration
*/
typedef KDRV_AI_REORG_KERPARM VENDOR_AI_REORG_KERPARM;

/**
    Neural Network Processing parameters
    de-convolution -> convolution -> normalization -> eltwise -> activation -> pooling
*/
typedef KDRV_AI_NEURAL_PARM VENDOR_AI_NEURAL_PARM;

/**
    roi pooling parameters configuration
*/
typedef KDRV_AI_ROIPOOL_PARM VENDOR_AI_ROIPOOL_PARM;

/**
    svm parameters configuration
*/
typedef KDRV_AI_SVM_PARM VENDOR_AI_SVM_PARM;

/**
    fully connect parameters configuration
*/
typedef KDRV_AI_FC_PARM VENDOR_AI_FC_PARM;

/**
    MAU parameters configuration
*/
typedef KDRV_AI_MAU_PARM VENDOR_AI_MAU_PARM;

/**
    permute parameters configuration
*/
typedef KDRV_AI_PERMUTE_PARM VENDOR_AI_PERMUTE_PARM;

/**
    reorganize parameters configuration
*/
typedef KDRV_AI_REORG_PARM VENDOR_AI_REORG_PARM;

/**
    anchor transform parameters configuration
*/
typedef KDRV_AI_ANCHOR_PARM VENDOR_AI_ANCHOR_PARM;

/**
    softmax parameters configuration
*/
typedef KDRV_AI_SOFTMAX_PARM VENDOR_SOFTMAX_PARM;

/**
    preprocessing parameters configuration
*/
typedef KDRV_AI_PREPROC_PARM VENDOR_PREPROC_PARM;

/**
    KDRV AI structure - ai open object
*/
typedef KDRV_AI_OPENCFG VENDOR_AI_OPENCFG;

/**
    KDRV AI structure
    AI app structure
*/
typedef KDRV_AI_APP_INFO VENDOR_AI_APP_INFO;

/**
    KDRV AI structure
    AI linked list structure
*/
typedef KDRV_AI_LL_INFO VENDOR_AI_LL_INFO;

typedef KDRV_AI_FC_INFO VENDOR_AI_FC_INFO;

#endif

/**
    vendor AI engine id
*/
typedef enum {
	VENDOR_AI_ENGINE_UNKNOWN = -1,
	VENDOR_AI_ENGINE_CONV    = 0, //DLA1
	VENDOR_AI_ENGINE_ROU     = 1, //DLA2
	VENDOR_AI_ENGINE_NUE2    = 2, //DLA3
	VENDOR_AI_ENGINE_LSU     = 3,
	VENDOR_AI_ENGINE_UTIL	 = 4,
	VENDOR_AI_ENGINE_CAL	 = 5,
	VENDOR_AI_ENGINE_PPU	 = 6,
	VENDOR_AI_ENGINE_JOBM	 = 7,
    VENDOR_AI_ENGINE_JMISP	 = 8,
    VENDOR_AI_ENGINE_DLA11   = 10,
	VENDOR_AI_ENGINE_DLA12   = 11,
    VENDOR_AI_ENGINE_DLA13   = 12,
	VENDOR_AI_ENGINE_CPU     = 13,
	VENDOR_AI_ENGINE_DSP     = 14,
    VENDOR_AI_ENGINE_HTE     = 15,
	VENDOR_AI_ENGINE_MAX_ID,
	ENUM_DUMMY4WORD(VENDOR_AI_ENGINE_ID)
} VENDOR_AI_ENGINE_ID;

#define CORE_MAX   32

typedef struct _VENDOR_AI_ENGINE_CORE_UT {
    CHAR   name[8];  ///< name
    UINT32 time;     ///< time
    UINT32 util;     ///< utility
} VENDOR_AI_ENGINE_CORE_UT;

typedef struct _VENDOR_AI_ENGINE_PERF_UT {
    UINT32 core_count;
    VENDOR_AI_ENGINE_CORE_UT core[CORE_MAX];
} VENDOR_AI_ENGINE_PERF_UT;

#ifdef __cplusplus
extern "C" {
#endif
extern HD_RESULT vendor_ai_dla_getinfo(VENDOR_AI_ENGINE_ID engine_id);
extern HD_RESULT vendor_ai_dla_reset (void);
extern HD_RESULT vendor_ai_dla_init (void);
extern HD_RESULT vendor_ai_dla_uninit (void);
extern HD_RESULT vendor_ai_dla_open_path(UINT32 proc_id);
extern HD_RESULT vendor_ai_dla_close_path(UINT32 proc_id);
extern HD_RESULT vendor_ai_dla_perf_ut(UINT32 enable, VENDOR_AI_ENGINE_PERF_UT* p_perf_ut);

extern HD_RESULT vendor_ai_dla_create_joblist (UINT32 proc_id, UINT32 max_job_cnt, UINT32 job_cnt, UINT32 bind_cnt, UINT32 ddr_id);
extern HD_RESULT vendor_ai_dla_destory_joblist (UINT32 proc_id);
extern HD_RESULT vendor_ai_dla_create_sub_joblist (UINT32 proc_id, UINT32 sub_job_cnt);
extern HD_RESULT vendor_ai_dla_destory_sub_joblist (UINT32 proc_id);
extern HD_RESULT vendor_ai_dla_dump_joblist (UINT32 proc_id, UINT32 info);

extern HD_RESULT vendor_ai_dla_clear_job (UINT32 proc_id, UINT32 job_id);
extern HD_RESULT vendor_ai_dla_clear_job3 (UINT32 proc_id, UINT32 job_id);
//extern HD_RESULT vendor_ai_dla_set_job (UINT32 proc_id, UINT32 job_id, UINT32 engine_id, UINT32 engine_op, void* p_op_info, void* p_io_info, INT32 wait_ms);
extern HD_RESULT vendor_ai_dla_set_job (UINT32 proc_id, UINT32 job_id, UINT32 engine_id, UINT32 engine_op, UINT32 schd_parm, void* p_op_info, void* p_io_info, INT32 wait_ms);
extern HD_RESULT vendor_ai_dla_set_sub_job (UINT32 proc_id, UINT32 job_id, uintptr_t parm_addr);
extern HD_RESULT vendor_ai_dla_bind_job (UINT32 proc_id, UINT32 job_id, UINT32 next_job_id);
extern HD_RESULT vendor_ai_dla_sum_job (UINT32 proc_id, uintptr_t* src_count, uintptr_t* dest_count);
extern HD_RESULT vendor_ai_dla_push_job (UINT32 proc_id, UINT32 job_id, UINT32 schd_parm, UINT32 job_priority);
extern HD_RESULT vendor_ai_dla_pull_job (UINT32 proc_id, UINT32* job_id);
extern HD_RESULT vendor_ai_dla_lock_job (UINT32 proc_id, UINT32 job_id);
extern HD_RESULT vendor_ai_dla_unlock_job (UINT32 proc_id, UINT32 job_id);

//extern HD_RESULT vendor_ai_dla_set_job2 (UINT32 proc_id, UINT32 job_id, UINT32 engine_id, UINT32 engine_op, void* p_op_info, void* p_io_info, INT32 wait_ms);
extern HD_RESULT vendor_ai_dla_set_job2 (UINT32 proc_id, UINT32 job_id, UINT32 engine_id, UINT32 engine_op, UINT32 schd_parm, void* p_op_info, void* p_io_info, INT32 wait_ms);
extern HD_RESULT vendor_ai_dla_set_job3 (UINT32 proc_id, UINT32 job_id, UINT32 engine_id, UINT32 engine_op, UINT32 schd_parm, void* p_op_info, void* p_io_info, INT32 wait_ms);

extern HD_RESULT vendor_ai_dla_set_job_debug_mode (UINT32 proc_id);

extern HD_RESULT vendor_ai_dla_perf_timeline(UINT32 enable);
extern HD_RESULT vendor_ai_dla_set_timeline_job(VENDOR_AIS_FLOW_TIMELINE_JOB_INFO *timeline_job_info);
extern HD_RESULT vendor_ai_dla_update_timeline_info(UINT32 timeline_func, UINT32 timeline_buf_size);
#ifdef __cplusplus
}
#endif

#endif //_VENDOR_AI_DLA_H_
