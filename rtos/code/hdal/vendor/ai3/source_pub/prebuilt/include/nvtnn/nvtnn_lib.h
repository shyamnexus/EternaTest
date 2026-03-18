/**
    @brief Header file of NvtNN library definitions.

    @file nvtnn_lib.h

    @ingroup nvtnn

    @note Nothing.

    Copyright Novatek Microelectronics Corp. 2020.  All rights reserved.
*/
#ifndef _NVTNN_LIB_H_
#define _NVTNN_LIB_H_

/********************************************************************
    INCLUDE FILES
********************************************************************/
#include "hd_type.h"
#include "vendor_ai_net/nn_parm.h"
#include "vendor_ai_net/nn_dli.h"

/********************************************************************
    TYPE DEFINITIONS
********************************************************************/
typedef NN_SOFTMAX_PARM         NVTNN_SOFTMAX_PRMS;
typedef NN_SOFTMAX_ASYM_PARM    NVTNN_SOFTMAX_ASYM_PRMS;
typedef NN_PRELU_PARM           NVTNN_PRELU_PRMS;
typedef NN_PRIORBOX_PARM        NVTNN_PRIORBOX_PRMS;
typedef NN_DETOUT_PARM          NVTNN_DETOUT_PRMS;
typedef NN_LRN_PARM             NVTNN_LRN_PRMS;
typedef NN_FC_POST_PARM         NVTNN_FC_POST_PRMS;
typedef NN_POOL_PARM            NVTNN_POOL_PRMS;
typedef NN_LSTM_PARM            NVTNN_LSTM_PRMS;
typedef NN_PERMUTE_PARM         NVTNN_PERMUTE_PRMS;
typedef NN_REVERSE_PARM         NVTNN_REVERSE_PRMS;
typedef NN_NORM_PARM            NVTNN_NORM_PRMS;
typedef NN_ELTWISE_PARM         NVTNN_ELTWISE_PRMS;
typedef NN_FC_PARM              NVTNN_FC_PRMS;
typedef NN_TOPK_PARM            NVTNN_TOPK_PRMS;
typedef NN_GATHER_PARM          NVTNN_GATHER_PRMS;
typedef NN_GATHERND_PARM        NVTNN_GATHERND_PARM;
typedef NN_GATHERELEMENT_PARM   NVTNN_GATHERELEMENT_PRMS;
typedef NN_ONEHOT_PARM          NVTNN_ONEHOT_PRMS;
typedef NN_GRIDSAMPLE_PARM      NVTNN_GRIDSAMPLE_PRMS;
typedef NN_CUMSUM_PARM          NVTNN_CUMSUM_PRMS;

typedef enum {
	RND_DOWN,
	RND_UP,
	RND_TOWARD_0,
	RND_AWAY_FROM_0,
	RND_HALF_DOWN,
	RND_HALF_UP,
	RND_HALF_TOWARD_0,
	RND_HALF_AWAY_FROM_0,
  	RND_HALF_TO_EVEN,
  	RND_HALF_TO_ODD,
  	RND_TO_NEAR_INT,
  	RND_RINT,
	ENUM_DUMMY4WORD(RND_MODE)
} RND_MODE;

/********************************************************************
    EXTERN VARIABLES & FUNCTION PROTOTYPES DECLARATIONS
********************************************************************/
#ifdef __cplusplus
extern "C" {
#endif


extern const CHAR *prebuilt_ai_get_lib_version(VOID);

/* Vision Layers */
extern HD_RESULT nvtnn_pool_process(NVTNN_POOL_PRMS *p_prms);

/* Recurrent Layers */
extern HD_RESULT nvtnn_lstm_process(NVTNN_LSTM_PRMS *p_prms);

extern HD_RESULT nvtnn_gather_process(NVTNN_GATHER_PRMS *p_prms);
extern HD_RESULT nvtnn_gather_test(UINTPTR buf);

/* Normalization Layers */
extern HD_RESULT nvtnn_lrn_process(NVTNN_LRN_PRMS *p_prms);
extern HD_RESULT nvtnn_norm_process(NVTNN_NORM_PRMS *p_prms);

/* Activation Layers */
extern HD_RESULT nvtnn_prelu_process(NVTNN_PRELU_PRMS *p_prms);

/* Utility Layers */
extern HD_RESULT nvtnn_softmax_process(NVTNN_SOFTMAX_PRMS *p_prms);
extern HD_RESULT nvtnn_softmax_asym_process(NVTNN_SOFTMAX_ASYM_PRMS *p_prms);
extern HD_RESULT nvtnn_dli_softmax_process(NN_DLI_SOFTMAX_PARM *p_prms);
extern HD_RESULT nvtnn_permute_process(NVTNN_PERMUTE_PRMS *p_prms);
extern HD_RESULT nvtnn_reverse_process(NVTNN_REVERSE_PRMS *p_prms);
extern HD_RESULT nvtnn_topk_process(NVTNN_TOPK_PRMS *p_prms);
extern HD_RESULT nvtnn_eltwise_process(NVTNN_ELTWISE_PRMS *p_prms);
extern HD_RESULT nvtnn_gathernd_process(NVTNN_GATHERND_PARM *p_prms);
extern HD_RESULT nvtnn_gatherelement_process(NVTNN_GATHERELEMENT_PRMS *p_prms);
extern HD_RESULT nvtnn_gatherelement_test(UINTPTR buf);
extern HD_RESULT nvtnn_onehot_process(NVTNN_ONEHOT_PRMS *p_prms);
extern HD_RESULT nvtnn_gridsample_process(NVTNN_GRIDSAMPLE_PRMS *p_prms);
extern HD_RESULT nvtnn_cumsum_process(NVTNN_CUMSUM_PRMS *p_prms);

/* Post-processing */
extern HD_RESULT nvtnn_priorbox_process(NVTNN_PRIORBOX_PRMS *p_prms);
extern HD_RESULT nvtnn_detout_process(NVTNN_DETOUT_PRMS *p_prms);
extern HD_RESULT nvtnn_fc_post_process(NVTNN_FC_POST_PRMS *p_prms);


/* Float to Fix */
extern VOID nvtnn_floattofix_c(FLOAT *p_in, VOID *p_out, UINT32 size, NN_FMT *p_fmt, NN_SF *p_sf);
extern VOID nvtnn_floattofix_neon(FLOAT *p_in, VOID *p_out, UINT32 size, NN_FMT *p_fmt, NN_SF *p_sf);
extern VOID nvtnn_floattofix_1d_neon(FLOAT *p_in, VOID *p_out, UINT32 size, NN_FMT *p_fmt, NN_SF *p_sf, INT32 zero_point);
extern VOID nvtnn_floattofix_nd_neon(FLOAT *p_in, VOID *p_out, UINT32 shape[NN_DLI_AXIS_NUM], UINT32 strides[NN_DLI_AXIS_NUM], NN_FMT *p_fmt, NN_SF *p_sf, INT32 zero_point);
extern VOID floattoint8_c(FLOAT *input_data, INT8 *output_data, INT16 frac_bits, INT32 size_data, FLOAT scale_ratio);
extern VOID floattouint8_c(FLOAT *input_data, UINT8 *output_data, INT16 frac_bits, INT32 size_data, FLOAT scale_ratio);
extern VOID floattoint16_c(FLOAT *input_data, INT16 *output_data, INT16 frac_bits, INT32 size_data, FLOAT scale_ratio);
extern VOID floattouint16_c(FLOAT *input_data, UINT16 *output_data, INT16 frac_bits, INT32 size_data, FLOAT scale_ratio);
extern VOID floattoint8_neon(FLOAT *input_data, INT8 *output_data, INT16 frac_bits, INT32 size_data, FLOAT scale_ratio);
extern VOID floattouint8_neon(FLOAT *input_data, UINT8 *output_data, INT16 frac_bits, INT32 size_data, FLOAT scale_ratio);
extern VOID floattoint16_neon(FLOAT *input_data, INT16 *output_data, INT16 frac_bits, INT32 size_data, FLOAT scale_ratio);
extern VOID floattouint16_neon(FLOAT *input_data, UINT16 *output_data, INT16 frac_bits, INT32 size_data, FLOAT scale_ratio);
extern VOID floattoint32_neon(FLOAT *input_data, INT32 *output_data, INT16 frac_bits, INT32 size_data, FLOAT scale_ratio);
extern VOID floattouint32_neon(FLOAT *input_data, UINT32 *output_data, INT16 frac_bits, INT32 size_data, FLOAT scale_ratio);
extern VOID floattoint8_1d_neon(FLOAT *input_data, INT8 *output_data, UINT32 size, INT16 frac_bits, FLOAT scale_ratio, INT32 zero_point);
extern VOID floattouint8_1d_neon(FLOAT *input_data, UINT8 *output_data, UINT32 size, INT16 frac_bits, FLOAT scale_ratio, INT32 zero_point);
extern VOID floattoint16_1d_neon(FLOAT *input_data, INT16 *output_data, UINT32 size, INT16 frac_bits, FLOAT scale_ratio, INT32 zero_point);
extern VOID floattouint16_1d_neon(FLOAT *input_data, UINT16 *output_data, UINT32 size, INT16 frac_bits, FLOAT scale_ratio, INT32 zero_point);
extern VOID floattoint8_nd_neon(FLOAT *input_data, INT8 *output_data, UINT32 shape[NN_DLI_AXIS_NUM], UINT32 strides[NN_DLI_AXIS_NUM], INT16 frac_bits, FLOAT scale_ratio, INT32 zero_point);
extern VOID floattouint8_nd_neon(FLOAT *input_data, UINT8 *output_data, UINT32 shape[NN_DLI_AXIS_NUM], UINT32 strides[NN_DLI_AXIS_NUM], INT16 frac_bits, FLOAT scale_ratio, INT32 zero_point);
extern VOID floattoint16_nd_neon(FLOAT *input_data, INT16 *output_data, UINT32 shape[NN_DLI_AXIS_NUM], UINT32 strides[NN_DLI_AXIS_NUM], INT16 frac_bits, FLOAT scale_ratio, INT32 zero_point);
extern VOID floattouint16_nd_neon(FLOAT *input_data, UINT16 *output_data, UINT32 shape[NN_DLI_AXIS_NUM], UINT32 strides[NN_DLI_AXIS_NUM], INT16 frac_bits, FLOAT scale_ratio, INT32 zero_point);

/* Fix to Float */
extern VOID nvtnn_fixtofloat_c(VOID *p_in, FLOAT *p_out, UINT32 size, NN_FMT *p_fmt, NN_SF *p_sf);
extern VOID nvtnn_fixtofloat_neon(VOID *p_in, FLOAT *p_out, UINT32 size, NN_FMT *p_fmt, NN_SF *p_sf);
extern VOID nvtnn_fixtofloat_1d_neon(VOID *p_in, FLOAT *p_out, UINT32 size, NN_FMT *p_fmt, NN_SF *p_sf, INT32 zero_point);
extern VOID nvtnn_fixtofloat_nd_neon(VOID *p_in, FLOAT *p_out, UINT32 shape[NN_DLI_AXIS_NUM], UINT32 strides[NN_DLI_AXIS_NUM], NN_FMT *p_fmt, NN_SF *p_sf, INT32 zero_point);
extern VOID int8tofloat_c(INT8 *input_data, FLOAT *output_data, INT16 frac_bits, INT32 size_data, FLOAT scale_ratio);
extern VOID uint8tofloat_c(UINT8 *input_data, FLOAT *output_data, INT16 frac_bits, INT32 size_data, FLOAT scale_ratio);
extern VOID int16tofloat_c(INT16 *input_data, FLOAT *output_data, INT16 frac_bits, INT32 size_data, FLOAT scale_ratio);
extern VOID uint16tofloat_c(UINT16 *input_data, FLOAT *output_data, INT16 frac_bits, INT32 size_data, FLOAT scale_ratio);
extern VOID int8tofloat_neon(INT8 *input_data, FLOAT *output_data, INT16 frac_bits, INT32 size_data, FLOAT scale_ratio);
extern VOID uint8tofloat_neon(UINT8 *input_data, FLOAT *output_data, INT16 frac_bits, INT32 size_data, FLOAT scale_ratio);
extern VOID int16tofloat_neon(INT16 *input_data, FLOAT *output_data, INT16 frac_bits, INT32 size_data, FLOAT scale_ratio);
extern VOID uint16tofloat_neon(UINT16 *input_data, FLOAT *output_data, INT16 frac_bits, INT32 size_data, FLOAT scale_ratio);
extern VOID int32tofloat_neon(INT32 *input_data, FLOAT *output_data, INT16 frac_bits, INT32 size_data, FLOAT scale_ratio);
extern VOID uint32tofloat_neon(UINT32 *input_data, FLOAT *output_data, INT16 frac_bits, INT32 size_data, FLOAT scale_ratio);
extern VOID int8tofloat_1d_neon(INT8 *input_data, FLOAT *output_data, UINT32 size, INT16 frac_bits, FLOAT scale_ratio, INT32 zero_point);
extern VOID uint8tofloat_1d_neon(UINT8 *input_data, FLOAT *output_data, UINT32 size, INT16 frac_bits, FLOAT scale_ratio, INT32 zero_point);
extern VOID int16tofloat_1d_neon(INT16 *input_data, FLOAT *output_data, UINT32 size, INT16 frac_bits, FLOAT scale_ratio, INT32 zero_point);
extern VOID uint16tofloat_1d_neon(UINT16 *input_data, FLOAT *output_data, UINT32 size, INT16 frac_bits, FLOAT scale_ratio, INT32 zero_point);
extern VOID int8tofloat_nd_neon(INT8 *input_data, FLOAT *output_data, UINT32 shape[NN_DLI_AXIS_NUM], UINT32 strides[NN_DLI_AXIS_NUM], INT16 frac_bits, FLOAT scale_ratio, INT32 zero_point);
extern VOID uint8tofloat_nd_neon(UINT8 *input_data, FLOAT *output_data, UINT32 shape[NN_DLI_AXIS_NUM], UINT32 strides[NN_DLI_AXIS_NUM], INT16 frac_bits, FLOAT scale_ratio, INT32 zero_point);
extern VOID int16tofloat_nd_neon(INT16 *input_data, FLOAT *output_data, UINT32 shape[NN_DLI_AXIS_NUM], UINT32 strides[NN_DLI_AXIS_NUM], INT16 frac_bits, FLOAT scale_ratio, INT32 zero_point);
extern VOID uint16tofloat_nd_neon(UINT16 *input_data, FLOAT *output_data, UINT32 shape[NN_DLI_AXIS_NUM], UINT32 strides[NN_DLI_AXIS_NUM], INT16 frac_bits, FLOAT scale_ratio, INT32 zero_point);
extern VOID nvtnn_convert_fmt_c(VOID *p_in, VOID *p_out, UINT32 size, NN_FMT *p_in_fmt, NN_FMT *p_out_fmt, NN_SF *p_in_sf, NN_SF *p_out_sf);

/* Permute Data */
extern VOID permute_data_out(INT8 *p_in, INT8 *p_out, UINT32 stride, UINT32 num);
extern VOID permute_data_in(INT8 *p_in, INT8 *p_out, UINT32 stride, UINT32 num);


#ifdef __cplusplus
}
#endif

#endif  /* _NVTNN_LIB_H_ */
