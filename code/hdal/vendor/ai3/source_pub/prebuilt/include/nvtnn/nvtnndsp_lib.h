/**
    @brief Header file of NvtNN library definitions.

    @file nvtnndsp_lib.h

    @ingroup nvtnn

    @note Nothing.

    Copyright Novatek Microelectronics Corp. 2020.  All rights reserved.
*/
#ifndef _NVTNNDSP_LIB_H_
#define _NVTNNDSP_LIB_H_

/********************************************************************
    INCLUDE FILES
********************************************************************/
#include "hd_type.h"
#include "vendor_ai_net/nn_parm.h"
#include "nvtipc.h"

/********************************************************************
    TYPE DEFINITIONS
********************************************************************/
typedef NN_MODE             NVTNN_FUN;
typedef NN_SOFTMAX_PARM     NVTNN_SOFTMAX_PRMS;
typedef NN_PRELU_PARM       NVTNN_PRELU_PRMS;
typedef NN_PRIORBOX_PARM    NVTNN_PRIORBOX_PRMS;
typedef NN_DETOUT_PARM      NVTNN_DETOUT_PRMS;
typedef NN_LRN_PARM         NVTNN_LRN_PRMS;
typedef NN_FC_POST_PARM     NVTNN_FC_POST_PRMS;
typedef NN_POOL_PARM        NVTNN_POOL_PRMS;
typedef NN_LSTM_PARM        NVTNN_LSTM_PRMS;
typedef NN_PERMUTE_PARM     NVTNN_PERMUTE_PRMS;
typedef NN_REVERSE_PARM     NVTNN_REVERSE_PRMS;
typedef NN_NORM_PARM        NVTNN_NORM_PRMS;
typedef NN_ELTWISE_PARM     NVTNN_ELTWISE_PRMS;
typedef NN_FC_PARM          NVTNN_FC_PRMS;
typedef NN_DILATEDCONV_PARM NVTNN_DILATEDCONV_PRMS;
typedef NN_RESIZE_PARM		NVTNN_RESIZE_PRMS;
typedef NN_ELEMENTWISE_PARM   NVTNN_ELEMENTWISE_PRMS;
typedef NN_LAYER_NORMALIZATION_PARM NVTNN_LAYER_NORMALIZATION_PRMS;
typedef NN_INSTANCE_NORMALIZATION_PARM NVTNN_INSTANCE_NORMALIZATION_PRMS;
typedef NN_DECONVOLUTION_PARM    NVTNN_DECONVOLUTION_PRMS;
typedef NN_MATMUL_PARM    NVTNN_MATMUL_PRMS;

/**
    IPC destination core.
*/
typedef enum {
	NVTNN_SENDTO_DSP1       = NVTIPC_SENDTO_DSP1,   ///< sending the request to DSP1
	NVTNN_SENDTO_DSP2       = NVTIPC_SENDTO_DSP2,   ///< sending the request to DSP2
	ENUM_DUMMY4WORD(NVTNN_SENDTO)
} NVTNN_SENDTO;

/**
    Return status.
*/
typedef enum {
	NVTNN_STA_OK            = 0,                    ///< the status is OK
	NVTNN_STA_ERROR         = -1,                   ///< some error occurred
	NVTNN_STA_INVALID_FUN   = -2,                   ///< invalid function enum
	NVTNN_STA_INVALID_PARM  = -3,                   ///< invalid parameter value
	ENUM_DUMMY4WORD(NVTNN_STA)
} NVTNN_STA;

/**
    IPC init parameters.
*/
typedef struct {
	UINTPTR pa;                                     ///< IPC buffer physical address
	UINTPTR va;                                     ///< IPC buffer virtual address
	UINT32 size;                                    ///< IPC buffer size
} NVTNN_IPC_INIT;

/********************************************************************
    EXTERN VARIABLES & FUNCTION PROTOTYPES DECLARATIONS
********************************************************************/
#ifdef __cplusplus
extern "C" {
#endif


extern const CHAR *prebuilt_aidsp_get_lib_version(VOID);

extern UINT32 nvtnndsp_get_buf_size(VOID);
extern HD_RESULT nvtnndsp_init(NVTNN_IPC_INIT *p_init, NVTNN_SENDTO dsp_core);
extern HD_RESULT nvtnndsp_uninit(NVTNN_SENDTO dsp_core);

extern HD_RESULT nvtnndsp_op_proc(NN_MODE mode, VOID *p_prms, NVTNN_SENDTO dsp_core);
extern HD_RESULT nvtnndsp_op_proc_nowait(NN_MODE mode, VOID *p_prms, NVTNN_SENDTO dsp_core);
extern HD_RESULT nvtnndsp_op_waitdone(NN_MODE mode, VOID *p_prms, NVTNN_SENDTO dsp_core);


/* Vision Layers */
extern HD_RESULT nvtnndsp_pool_process(NVTNN_POOL_PRMS *p_prms, NVTNN_SENDTO dsp_core);
extern HD_RESULT nvtnndsp_dilatedconv_process(NVTNN_DILATEDCONV_PRMS *p_prms, NVTNN_SENDTO dsp_core);

/* Recurrent Layers */
extern HD_RESULT nvtnndsp_lstm_process(NVTNN_LSTM_PRMS *p_prms, NVTNN_SENDTO dsp_core);

/* Normalization Layers */
extern HD_RESULT nvtnndsp_lrn_process(NVTNN_LRN_PRMS *p_prms, NVTNN_SENDTO dsp_core);
extern HD_RESULT nvtnndsp_norm_process(NVTNN_NORM_PRMS *p_prms, NVTNN_SENDTO dsp_core);

/* Activation Layers */
extern HD_RESULT nvtnndsp_prelu_process(NVTNN_PRELU_PRMS *p_prms, NVTNN_SENDTO dsp_core);

/* Utility Layers */
extern HD_RESULT nvtnndsp_softmax_process(NVTNN_SOFTMAX_PRMS *p_prms, NVTNN_SENDTO dsp_core);
extern HD_RESULT nvtnndsp_softmax_process_nowait(NVTNN_SOFTMAX_PRMS *p_prms, NVTNN_SENDTO dsp_core);
extern HD_RESULT nvtnndsp_softmax_waitdone(NVTNN_SOFTMAX_PRMS *p_prms, NVTNN_SENDTO dsp_core);
extern HD_RESULT nvtnndsp_permute_process(NVTNN_PERMUTE_PRMS *p_prms, NVTNN_SENDTO dsp_core);
extern HD_RESULT nvtnndsp_reverse_process(NVTNN_REVERSE_PRMS *p_prms, NVTNN_SENDTO dsp_core);
extern HD_RESULT nvtnndsp_eltwise_process(NVTNN_ELTWISE_PRMS *p_prms, NVTNN_SENDTO dsp_core);
extern HD_RESULT nvtnndsp_resize_process(NVTNN_RESIZE_PRMS *p_prms, NVTNN_SENDTO dsp_core);

/* Post-processing */
extern HD_RESULT nvtnndsp_priorbox_process(NVTNN_PRIORBOX_PRMS *p_prms, NVTNN_SENDTO dsp_core);
extern HD_RESULT nvtnndsp_detout_process(NVTNN_DETOUT_PRMS *p_prms, NVTNN_SENDTO dsp_core);
extern HD_RESULT nvtnndsp_fc_post_process(NVTNN_FC_POST_PRMS *p_prms, NVTNN_SENDTO dsp_core);


/* Float to Fix */
extern VOID nvtnndsp_floattofix(FLOAT *p_in, VOID *p_out, UINT32 size, NN_FMT *p_fmt, NN_SF *p_sf, NVTNN_SENDTO dsp_core);
extern VOID nvtnndsp_floattoint8(FLOAT *input_data, INT8 *output_data, INT16 frac_bits, INT32 size_data, FLOAT scale_ratio, NVTNN_SENDTO dsp_core);
extern VOID nvtnndsp_floattouint8(FLOAT *input_data, UINT8 *output_data, INT16 frac_bits, INT32 size_data, FLOAT scale_ratio, NVTNN_SENDTO dsp_core);
extern VOID nvtnndsp_floattoint16(FLOAT *input_data, INT16 *output_data, INT16 frac_bits, INT32 size_data, FLOAT scale_ratio, NVTNN_SENDTO dsp_core);
extern VOID nvtnndsp_floattouint16(FLOAT *input_data, UINT16 *output_data, INT16 frac_bits, INT32 size_data, FLOAT scale_ratio, NVTNN_SENDTO dsp_core);

/* Fix to Float */
extern VOID nvtnndsp_fixtofloat(VOID *p_in, FLOAT *p_out, UINT32 size, NN_FMT *p_fmt, NN_SF *p_sf);
extern VOID nvtnndsp_int8tofloat(INT8 *input_data, FLOAT *output_data, INT16 frac_bits, INT32 size_data, FLOAT scale_ratio, NVTNN_SENDTO dsp_core);
extern VOID nvtnndsp_uint8tofloat(UINT8 *input_data, FLOAT *output_data, INT16 frac_bits, INT32 size_data, FLOAT scale_ratio, NVTNN_SENDTO dsp_core);
extern VOID nvtnndsp_int16tofloat(INT16 *input_data, FLOAT *output_data, INT16 frac_bits, INT32 size_data, FLOAT scale_ratio, NVTNN_SENDTO dsp_core);
extern VOID nvtnndsp_uint16tofloat(UINT16 *input_data, FLOAT *output_data, INT16 frac_bits, INT32 size_data, FLOAT scale_ratio, NVTNN_SENDTO dsp_core);
extern VOID nvtnndsp_convert_fmt_c(VOID *p_in, VOID *p_out, UINT32 size, NN_FMT *p_in_fmt, NN_FMT *p_out_fmt, NN_SF *p_in_sf, NN_SF *p_out_sf);

#ifdef __cplusplus
}
#endif

#endif  /* _NVTNNDSP_LIB_H_ */
