/**
    @brief Header file of DLI (Deep Learning Interface) definition of vendor net flow sample.

    @file nn_dli.h

    @ingroup net_flow_sample

    @note Nothing.

    Copyright Novatek Microelectronics Corp. 2018.  All rights reserved.
*/
#ifndef _NN_DLI_H_
#define _NN_DLI_H_

/********************************************************************
    INCLUDE FILES
********************************************************************/
#include "kwrap/type.h"
#include "nn_net.h" // for NN_FMT
#include "nn_parm.h" // for NN_SF
#include "nn_verinfo.h" // for ENUM_DUMMY4WORD

/********************************************************************
    MACRO CONSTANT DEFINITIONS
********************************************************************/
#define NN_DLI_AXIS_NUM                 6
#define NN_DLI_AXIS_W                   0
#define NN_DLI_AXIS_H                   1
#define NN_DLI_AXIS_C                   2
#define NN_DLI_AXIS_N                   3
#define NN_DLI_AXIS_T                   4

#define NN_DLI_STRIDE_W                 0
#define NN_DLI_STRIDE_H                 1
#define NN_DLI_STRIDE_C                 2
#define NN_DLI_STRIDE_N                 3
#define NN_DLI_STRIDE_T                 4

#define NN_DLI_NORMALIZED_SHAPE_NUM     6

#define NN_DLI_TENSOR_INFO_HEADER_SIGN          MAKEFOURCC('T','I','H','1')
#define NN_DLI_QUANTIZATION_INFO_HEADER_SIGN    MAKEFOURCC('Q','I','H','1')

#define NN_DLI_ElEMENTWISE_PARM_SIGN            MAKEFOURCC('P','E','W','2')
#define NN_DLI_RESIZE_PARM_SIGN                 MAKEFOURCC('P','R','S','2')
#define NN_DLI_SOFTMAX_PARM_SIGN                MAKEFOURCC('P','S','M','2')
#define NN_DLI_LAYER_NORMALIZATION_PARM_SIGN    MAKEFOURCC('P','L','N','1')
#define NN_DLI_INSTANCE_NORMALIZATION_PARM_SIGN MAKEFOURCC('P','I','N','1')
#define NN_DLI_DECONVOLUTION_PARM_SIGN          MAKEFOURCC('P','D','V','1')
#define NN_DLI_CONV_PARM_SIGN                   MAKEFOURCC('P','C','O','2')
#define NN_DLI_POOLING_PARM_SIGN                MAKEFOURCC('P','P','O','1')
#define NN_DLI_MATMUL_PARM_SIGN                 MAKEFOURCC('P','M','M','1')
#define NN_DLI_PRIORBOX_PARM_SIGN               MAKEFOURCC('P','P','B','1')
#define NN_DLI_REVERSE_PARM_SIGN                MAKEFOURCC('P','R','V','1')
#define NN_DLI_PERMUTE_PARM_SIGN                MAKEFOURCC('P','P','M','1')
#define NN_DLI_NORM_PARM_SIGN                   MAKEFOURCC('P','N','M','1')
/********************************************************************
    TYPE DEFINITION
********************************************************************/

#ifndef UINTPTR
typedef uintptr_t                       UINTPTR;
#endif

/**
    Tensor information header.
*/
//@{
typedef struct _NN_DLI_TENSOR_INFO_HEADER {
	UINT32 size; // total size of TensorInfo (including header) in bytes.
	UINT32 sign; // signature = MAKEFOURCC('T','I','H','1')  = Tensor Info Header V1
	UINT32 nums; // Number of TensorInfo
	UINT32 attr; // Reserved. Default is 0.
} NN_DLI_TENSOR_INFO_HEADER;
//@}

/**
    Tensor Information.
*/
//@{
typedef struct _NN_DLI_TENSOR_INFO {
	UINT8 data_type; // NN_DLI_DATA_TYPE. Default is 0 (NN_DLI_DATA_TYPE_UNKNOWN).
	UINT8 format;    // Default is 0.
	UINT8 data_layout; // NN_DLI_DATA_LAYOUT. Default is 1 (NN_DLI_DATA_LAYOUT_NCHW).
	UINT8 trans_enable; // Default is 1.
	UINT32 num_channels; // Default is 1.
	UINT32 shape[NN_DLI_AXIS_NUM]; // NN_SHAPE. [width (0), height (1), channel (2), batch (3), time (4), (5)]. Default is 0 (Invalid).
	UINT32 strides[NN_DLI_AXIS_NUM]; // NN_OFS. [width (0), height (1), channel (2), batch (3), time (4), (5)]. Default is 0 (Invalid).
	UINT32 quant_info_nums; // Default is 0.
	UINTPTR quant_info_va; // Default is 0. Index at Quantization Info Table
	UINTPTR quant_info_pa; // Default is 0.
	UINTPTR data_va; // Default is 0.
	UINTPTR data_pa; // Default is 0.
	UINTPTR trans_va; // Default is 0.
	UINTPTR trans_pa; // Default is 0.
	UINTPTR obj_ptr; // Default is 0.
	UINTPTR f32_obj_ptr; // Default is 0.
} NN_DLI_TENSOR_INFO;
//@}

/**
    Quantization information header.
*/
//@{
typedef struct _NN_DLI_QUANTIZATION_INFO_HEADER {
	UINT32 size; // total size of QuantizationInfo (including header) in bytes.
	UINT32 sign; // signature = MAKEFOURCC('Q','I','H','1')  = Quantization Info Header V1
	UINT32 nums; // Number of QuantizationInfo
	UINT32 attr; // Reserved. Default is 0.
} NN_DLI_QUANTIZATION_INFO_HEADER;
//@}

/**
    Quantization Information.
*/
//@{
typedef struct _NN_DLI_QUANTIZATION_INFO {
	FLOAT scale; // Default is 1.0f.
	INT32 offset; // Default is 0. It's zero-point.
	NN_FMT fmt; // Reserved.
	NN_SF sf; // Reserved.
} NN_DLI_QUANTIZATION_INFO;
//@}

/**
    Data type.
*/
//@{
typedef enum {
	NN_DLI_DATA_TYPE_UNKNOWN = 0,
	NN_DLI_DATA_TYPE_U8 = 1,
	NN_DLI_DATA_TYPE_S8 = 2,
	NN_DLI_DATA_TYPE_QSYMM8 = 3,
	NN_DLI_DATA_TYPE_QASYMM8 = 4, // 8bit unsigned fixed point
	NN_DLI_DATA_TYPE_QASYMM8_SIGNED = 5, // 8bit signed fixed point
	NN_DLI_DATA_TYPE_QSYMM8_PER_CHANNEL = 6,
	NN_DLI_DATA_TYPE_U16 = 7,
	NN_DLI_DATA_TYPE_S16 = 8,
	NN_DLI_DATA_TYPE_QSYMM16 = 9, // 16bit signed fixed point
	NN_DLI_DATA_TYPE_QASYMM16 = 10, // 16bit unsigned fixed point
	NN_DLI_DATA_TYPE_U32 = 11,
	NN_DLI_DATA_TYPE_S32 = 12,
	NN_DLI_DATA_TYPE_U64 = 13,
	NN_DLI_DATA_TYPE_S64 = 14,
	NN_DLI_DATA_TYPE_BFLOAT16 = 15,
	NN_DLI_DATA_TYPE_F16 = 16,
	NN_DLI_DATA_TYPE_F32 = 17,
	NN_DLI_DATA_TYPE_F64 = 18,
	NN_DLI_DATA_TYPE_SIZET = 19,
	ENUM_DUMMY4WORD(NN_DLI_DATA_TYPE)
} NN_DLI_DATA_TYPE;
//@}

/**
    Data layout type.
*/
//@{
typedef enum {
	NN_DLI_DATA_LAYOUT_UNKNOWN = 0,
	NN_DLI_DATA_LAYOUT_NCHW = 1,
	NN_DLI_DATA_LAYOUT_NHWC = 2,
	NN_DLI_DATA_LAYOUT_NCDHW = 3,
	NN_DLI_DATA_LAYOUT_NDHWC = 4,
	ENUM_DUMMY4WORD(NN_DLI_DATA_LAYOUT)
} NN_DLI_DATA_LAYOUT;
//@}

/**
    Interpolation policy.
*/
//@{
typedef enum {
	NN_DLI_INTERPOLATION_POLICY_NEAREST_NEIGHBOR = 0, // Output values are defined to match the source pixel whose center is nearest to the sample position.
	NN_DLI_INTERPOLATION_POLICY_BILINEAR         = 1, // Output values are defined by bilinear interpolation between the pixels.
	NN_DLI_INTERPOLATION_POLICY_AREA             = 2, // Output values are determined by averaging the source pixels whose areas fall under the area of the destination pixel, projected onto the source image.
	ENUM_DUMMY4WORD(NN_DLI_INTERPOLATION_POLICY)
} NN_DLI_INTERPOLATION_POLICY;
//@}

/**
    Border mode.
*/
//@{
typedef enum {
	NN_DLI_BORDER_MODE_UNDEFINED                 = 0, // Borders are left undefined.
	NN_DLI_BORDER_MODE_CONSTANT                  = 1, // Pixels outside the image are assumed to have a constant value.
	NN_DLI_BORDER_MODE_REPLICATE                 = 2, // Pixels outside the image are assumed to have the same value as the closest image pixel.
	ENUM_DUMMY4WORD(NN_DLI_BORDER_MODE)
} NN_DLI_BORDER_MODE;
//@}

/**
    Sampling policy.
*/
//@{
typedef enum {
	NN_DLI_SAMPLING_POLICY_CENTER                = 0, // Samples are taken at pixel center.
	NN_DLI_SAMPLING_POLICY_TOP_LEFT              = 1, // Samples are taken at pixel top left corner.
	ENUM_DUMMY4WORD(NN_DLI_SAMPLING_POLICY)
} NN_DLI_SAMPLING_POLICY;
//@}

/**
    Common layer parameters.
*/
//@{
typedef struct _NN_DLI_LAYER_PARM {
	UINT32 sign; // signature
	UINT32 parm_size; // parm size in bytes.
	UINT32 mode; // NN_MODE: NN_DLI_SQRT, NN_DLI_DIV, ....
	UINT32 attr; // Reserved. Default is 0.
	UINTPTR next_layer_va; // Default is 0.
	UINTPTR next_layer_pa; // Default is 0.
	UINTPTR obj_ptr; // Default is 0.
} NN_DLI_LAYER_PARM;
//@}

/**
    Elementwise layer parameters.
*/
//@{
typedef struct _NN_DLI_ElEMENTWISE_PARM {
	UINT32 sign; // signature = MAKEFOURCC('P','E','W','2')  = Parameter ElementWise V2
	UINT32 parm_size; // parm size in bytes.
	UINT32 mode; // NN_MODE: NN_DLI_SQRT, NN_DLI_DIV, ....
	UINT32 attr; // Reserved. Default is 0.
	UINTPTR next_layer_va; // Default is 0.
	UINTPTR next_layer_pa; // Default is 0.
	UINTPTR obj_ptr; // Default is 0.
	UINTPTR input1_info_va; // Index at Tensor Info Table
	UINTPTR input1_info_pa;
	UINTPTR input2_info_va; // Same as input1_info_va. If layer doesn't have input2, set it in default (0xFFFFFFFFFFFFFFFF = -1).
	UINTPTR input2_info_pa;
	UINTPTR output_info_va; // Same as input1_info_va.
	UINTPTR output_info_pa;
	UINTPTR temp_buf_va; // Op's temp buffer address
	UINTPTR temp_buf_pa;
} NN_DLI_ElEMENTWISE_PARM;
//@}

/**
    Resize layer parameters.
*/
//@{
typedef struct _NN_DLI_RESIZE_PARM {
	UINT32 sign; // signature = MAKEFOURCC('P','R','S','2')  = Parameter ReSize V2
	UINT32 parm_size; // parm size in bytes.
	UINT32 mode; // NN_MODE: NN_DLI_RESIZE
	UINT32 attr; // Reserved. Default is 0.
	UINTPTR next_layer_va; // Default is 0.
	UINTPTR next_layer_pa; // Default is 0.
	UINTPTR obj_ptr; // Default is 0.
	UINTPTR input_info_va; // Index at Tensor Info Table
	UINTPTR input_info_pa;
	UINTPTR output_info_va; // Same as input_info_va.
	UINTPTR output_info_pa;
	UINTPTR temp_buf_va; // Op's temp buffer address
	UINTPTR temp_buf_pa;
	UINT8 interpolation_policy; // NN_DLI_INTERPOLATION_POLICY. Default is 0 (NN_DLI_INTERPOLATION_POLICY_NEAREST_NEIGHBOR).
	UINT8 border_mode; // NN_DLI_BORDER_MODE. Default is 2 (NN_DLI_BORDER_MODE_REPLICATE).
	UINT8 sampling_policy; // NN_DLI_SAMPLING_POLICY. Default is 0 (NN_DLI_SAMPLING_POLICY_CENTER).
	UINT8 align_corners; // Bool. Default is 0 (FALSE).
	UINT32 __padding; // Reserved. Default is 0.
} NN_DLI_RESIZE_PARM;
//@}

/**
    Softmax layer parameters.
*/
//@{
typedef struct _NN_DLI_SOFTMAX_PARM {
	UINT32 sign; // signature = MAKEFOURCC('P','S','M','2')  = Parameter SoftMax V2
	UINT32 parm_size; // parm size in bytes.
	UINT32 mode; // NN_MODE: NN_DLI_SQRT, NN_DLI_DIV, ....
	UINT32 attr; // Reserved. Default is 0.
	UINTPTR next_layer_va; // Default is 0.
	UINTPTR next_layer_pa; // Default is 0.
	UINTPTR obj_ptr; // Default is 0.
	UINTPTR input_info_va; // Index at Tensor Info Table
	UINTPTR input_info_pa;
	UINTPTR output_info_va; // Same as input_info_va.
	UINTPTR output_info_pa;
	UINTPTR temp_buf_va; // Op's temp buffer address
	UINTPTR temp_buf_pa;
	FLOAT beta;                         ///< Scaling factor for the exponent. Default: 1.0f.
	INT32 axis;                         ///< Axis to perform softmax. Default: 0.
} NN_DLI_SOFTMAX_PARM;
//@}

/**
    FullyConnection layer parameters.
*/
//@{
typedef struct _NN_DLI_FC_PARM {
	UINT32 sign; // signature = MAKEFOURCC('P','F','C','1')  = Parameter FC V1
	UINT32 parm_size; // parm size in bytes.
	UINT32 mode; // NN_MODE: NN_DLI_SQRT, NN_DLI_DIV, ....
	UINT32 attr; // Reserved. Default is 0.
	UINTPTR next_layer_va; // Default is 0.
	UINTPTR next_layer_pa; // Default is 0.
	UINTPTR obj_ptr; // Default is 0.
	UINTPTR input_info_va;
	UINTPTR input_info_pa;
	UINTPTR weight_info_va;
	UINTPTR weight_info_pa;
	UINTPTR bias_info_va;
	UINTPTR bias_info_pa;
	UINTPTR output_info_va;
	UINTPTR output_info_pa;
	UINTPTR temp_buf_va;
	UINTPTR temp_buf_pa;
	NN_ACTIVATION_LAYER_INFO activation_info; //Fused activation to apply after the matrix multiplication. 
	BOOL transpose_weights;            //Transpose weights if true
	BOOL are_weights_reshaped;         // Reshape the weights tensor if false
	BOOL retrain_internal_weights;     // Retrain internal reshaped weights.
	BOOL enable_fast_math;             // Enable fast math computation
	BOOL fp_mixed_precision;           // Use wider accumulators (32bit instead of 16 for FP16) to impprove accuracy
} NN_DLI_FC_PARM;
//@}

/**
    Normalize layer parameters.
*/
//@{
typedef struct _NN_DLI_NORM_PARM {
	UINT32 sign;                              ///< signature = MAKEFOURCC('P','N','M','1')  = Parameter norm V1
	UINT32 parm_size;                         ///< parm size in bytes.
	UINT32 mode;                              ///< NN_MODE: NN_DLI_NORM, ......
	UINT32 attr;                              ///< Reserved. Default is 0.
	UINTPTR next_layer_va;                    ///< Default is 0.
	UINTPTR next_layer_pa;                    ///< Default is 0.
	UINTPTR obj_ptr;                          ///< Default is 0.
	UINTPTR input_info_va;
	UINTPTR input_info_pa;
	UINTPTR output_info_va;
	UINTPTR output_info_pa;
	UINTPTR temp_buf_va;
	UINTPTR temp_buf_pa;
	UINTPTR scale_buf_va;
	UINTPTR scale_buf_pa;
	UINT32 across_spatial;                   ///< whether or not scale parameters are shared across channels
	UINT32 channel_shared;                   ///< epsilon for not dividing by zero while normalizing variance
	FLOAT eps;
} NN_DLI_NORM_PARM;
//@}

/**
    Layer normalization layer parameters.
*/
//@{
typedef struct _NN_DLI_LAYER_NORMALIZATION_PARM {
	UINT32 sign; ///< signature = MAKEFOURCC('P','L','N','1')  = Parameter LN V1
	UINT32 parm_size; ///< parm size in bytes.
	UINT32 mode; ///< NN_MODE: NN_DLI_LAYER_NORMALIZATION, ....
	UINT32 attr; ///< Reserved. Default is 0.
	UINTPTR next_layer_va; ///< Default is 0.
	UINTPTR next_layer_pa; ///< Default is 0.
	UINTPTR obj_ptr; ///< Default is 0.
	UINTPTR input_info_va;
	UINTPTR input_info_pa;
	UINTPTR weight_info_va;
	UINTPTR weight_info_pa;
	UINTPTR bias_info_va;
	UINTPTR bias_info_pa;
	UINTPTR output_info_va;
	UINTPTR output_info_pa;
	UINTPTR temp_buf_va;
	UINTPTR temp_buf_pa;
	UINT32 elementwise_affine;							  ///< multiply gamma add beta elementwise_affine pytorch default 0
	UINT32 normalized_shape[NN_DLI_NORMALIZED_SHAPE_NUM]; ///< pytorch normalized_shape
	FLOAT  eps;											  ///< epsilon for not dividing by zero
} NN_DLI_LAYER_NORMALIZATION_PARM;
//@}
/**
    Instance Norm layer parameters.
*/
//@{
typedef struct _NN_DLI_INSTANCE_NORMALIZATION_PARM {
	UINT32 sign; ///< signature = MAKEFOURCC('P','I','N','1')  = Parameter IN V1
	UINT32 parm_size; ///< parm size in bytes.
	UINT32 mode; ///< NN_MODE: NN_DLI_INSTANCE_NORMALIZATION, ....
	UINT32 attr; ///< Reserved. Default is 0.
	UINTPTR next_layer_va; ///< Default is 0.
	UINTPTR next_layer_pa; ///< Default is 0.
	UINTPTR obj_ptr; ///< Default is 0.
	UINTPTR input_info_va;
	UINTPTR input_info_pa;
	UINTPTR mean_info_va;
	UINTPTR mean_info_pa;
	UINTPTR variance_info_va;
	UINTPTR variance_info_pa;
	UINTPTR gamma_info_va;
	UINTPTR gamma_info_pa;
	UINTPTR beta_info_va;
	UINTPTR beta_info_pa;
	UINTPTR output_info_va;
	UINTPTR output_info_pa;
	UINTPTR temp_buf_va;
	UINTPTR temp_buf_pa;
	UINT32  track_running_stats;             ///< save moving average mean and variance track_running_stats pytorch default false
	UINT32  affine;                          ///< this module has learnable affine parameters pytorch default False
	FLOAT   eps;                             ///< epsilon for not dividing by zero
} NN_DLI_INSTANCE_NORMALIZATION_PARM;
//@}

/**
    Deconvolution layer parameters.
*/
//@{
typedef struct _NN_DLI_DECONVOLUTION_PARM {
	UINT32 sign; ///< signature = MAKEFOURCC('P','D','V','1')  = Parameter Deconvolution V1
	UINT32 parm_size; ///< parm size in bytes.
	UINT32 mode; ///< NN_MODE: NN_DLI_DECONVOLUTION, ....
	UINT32 attr; ///< Reserved. Default is 0.
	UINTPTR next_layer_va; ///< Default is 0.
	UINTPTR next_layer_pa; ///< Default is 0.
	UINTPTR obj_ptr; ///< Default is 0.
	UINTPTR input_info_va;
	UINTPTR input_info_pa;
	UINTPTR weight_info_va;
	UINTPTR weight_info_pa;
	UINTPTR bias_info_va;
	UINTPTR bias_info_pa;
	UINTPTR output_info_va;
	UINTPTR output_info_pa;
	UINTPTR temp_buf_va;
	UINTPTR temp_buf_pa;
	NN_PAD_STRIDE_INFO       	conv_info;   ///< /**< Convolution info (Pads, strides,...) */
	NN_WEIGHTS_INFO			 	weights_info; 
	NN_SIZE2D				 	dilation;
	UINT32					 	num_groups;
	UINT32                      output_padding;
	BOOL                        bias; ///< pytorch param
} NN_DLI_DECONVOLUTION_PARM;
//@}

/**
    Convolution parameters.
*/
//@{
typedef struct _NN_DLI_CONV_PARM {
	UINT32 sign; // signature = MAKEFOURCC('P','C','O','2')  = Parameter conv V2
	UINT32 parm_size; // parm size in bytes.
	UINT32 mode; // NN_MODE: NN_DLI_SQRT, NN_DLI_DIV, ....
	UINT32 attr; // Reserved. Default is 0.
	UINTPTR next_layer_va; // Default is 0.
	UINTPTR next_layer_pa; // Default is 0.
	UINTPTR obj_ptr; // Default is 0.
    UINTPTR input_info_va; // Index at Tensor Info Table
    UINTPTR input_info_pa;
    UINTPTR weight_info_va; // Index at Tensor Info Table
    UINTPTR weight_info_pa;
    UINTPTR bias_info_va; // Index at Tensor Info Table
    UINTPTR bias_info_pa;
    UINTPTR output_info_va; // Same as input_info_va.
    UINTPTR output_info_pa;
    UINTPTR temp_buf_va; // Op's temp buffer address
    UINTPTR temp_buf_pa;
	NN_PAD_STRIDE_INFO       	conv_info; ///< acl param
	NN_WEIGHTS_INFO			 	weights_info; ///< acl param
	NN_SIZE2D				 	dilation; ///< acl param
	NN_ACTIVATION_LAYER_INFO 	act_info; ///< acl param
    BOOL					 	enable_fast_math; ///< acl param
	UINT32					 	num_groups; ///< acl param
	BOOL                        bias; ///< pytorch param
} NN_DLI_CONV_PARM;
//@}

/**
    Matrix multiplication layer parameters.
*/
//@{
typedef struct _NN_DLI_MATMUL_PARM {
	UINT32 sign;                              ///< signature = MAKEFOURCC('P','M','M','1')  = Parameter matmul V1
	UINT32 parm_size;                         ///< parm size in bytes.
	UINT32 mode;                              ///< NN_MODE: NN_DLI_MATMUL, ....
	UINT32 attr;                              ///< Reserved. Default is 0.
	UINTPTR next_layer_va;                    ///< Default is 0.
	UINTPTR next_layer_pa;                    ///< Default is 0.
	UINTPTR obj_ptr;                          ///< Default is 0.
	UINTPTR input1_info_va;
	UINTPTR input1_info_pa;
	UINTPTR input2_info_va;
	UINTPTR input2_info_pa;
	UINTPTR output_info_va;
	UINTPTR output_info_pa;
	UINTPTR temp_buf_va;
	UINTPTR temp_buf_pa;
} NN_DLI_MATMUL_PARM;
//@}

/**
    priorbox layer parameters.
*/
//@{
typedef struct _NN_DLI_PRIORBOX_PARM {
	UINT32 sign;                              ///< signature = MAKEFOURCC('P','P','B','1')  = Parameter priorbox V1
	UINT32 parm_size;                         ///< parm size in bytes.
	UINT32 mode;                              ///< NN_MODE: NN_DLI_PRIORBOX, ....
	UINT32 attr;                              ///< Reserved. Default is 0.
	UINTPTR next_layer_va;                    ///< Default is 0.
	UINTPTR next_layer_pa;                    ///< Default is 0.
	UINTPTR obj_ptr;                          ///< Default is 0.
	UINTPTR output_info_va;                   ///< output address
	UINTPTR output_info_pa;
	UINTPTR temp_buf_va;					  ///< temporary buffer address
	UINTPTR temp_buf_pa;
	UINT32 in_width;
	UINT32 in_height;
	UINT32 img_width;
	UINT32 img_height;
	FLOAT min_sizes[NN_PRIORBOX_SIZE_NUM];
	FLOAT max_sizes[NN_PRIORBOX_SIZE_NUM];
	UINT32 min_size_num;                      ///< number of max sizes
	UINT32 max_size_num;                      ///< number of min sizes
	FLOAT aspect_ratios[NN_PRIORBOX_ASPECT_RATIO_NUM];
	UINT32 aspect_ratio_num;
	UINT8 flip;
	UINT8 clip;
	FLOAT variances[NN_PRIORBOX_VAR_NUM];
	UINT32 variance_num;
	FLOAT offset;
} NN_DLI_PRIORBOX_PARM;

/**    Pooling parameters.*/
//@{
typedef struct _NN_DLI_POOLING_PARM {
	UINT32 sign;                             ///< signature = MAKEFOURCC('P','P','O','1')  = Parameter Pooling V1
	UINT32 parm_size;                        ///< parm size in bytes.
	UINT32 mode;                             ///< NN_MODE: NN_DLI_SQRT, NN_DLI_DIV, ....
	UINT32 attr;                             ///< Reserved. Default is 0.
	UINTPTR next_layer_va;                   ///< Default is 0.
	UINTPTR next_layer_pa;                   ///< Default is 0.
	UINTPTR obj_ptr;                         ///< Default is 0.
	UINTPTR input_info_va;                   ///< Index at Tensor Info Table
	UINTPTR input_info_pa;
	UINTPTR indices_info_va;                 ///< (optional)the indices of the maximal values
	UINTPTR indices_info_pa;
	UINTPTR output_info_va;                  ///< Same as input_info_va.
	UINTPTR output_info_pa;
	UINTPTR temp_buf_va;                     ///< Op's temp buffer address
	UINTPTR temp_buf_pa;
	NN_POOLING_LAYER_INFO pool_info;         ///< acl parameter
	NN_SIZE2D             dilation;          ///< (pytorch optional) MaxPool parameter, the stride between elements within a sliding window, must be > 0
	BOOL                  celi_mode;         ///< (pytorch optional) MaxPool & AvgPool parameter,  when True, will use ceil instead of floor to compute the output shape
	BOOL                  count_inclue_pad;  ///< (pytorch optional) AvgPool Parameter, will include the zero-padding in the averaging calculation
	BOOL                  divisor_override ; ///< (pytorch optional) AvgPool Parameter,  it will be used as divisor, otherwise size of the pooling region will be used
} NN_DLI_POOLING_PARM;
//@}

/**
	Reverse layer parameters.
*/
//@{
typedef struct _NN_DLI_REVERSE_PARM {
	UINT32 sign;				      // signature = MAKEFOURCC('P','R','V','1')  = Parameter reverse V1
	UINT32 parm_size;			    // parm size in bytes.
	UINT32 mode;				      // NN_MODE: NN_DLI_SQRT, NN_DLI_DIV, ....
	UINT32 attr;				      // Reserved. Default is 0.
	UINTPTR next_layer_va;		// Default is 0.
	UINTPTR next_layer_pa;		// Default is 0.
	UINTPTR obj_ptr;			    // Default is 0.
	UINTPTR input_info_va;		// Index at Tensor Info Table
	UINTPTR input_info_pa;
	UINTPTR output_info_va;	  // Same as input_info_va.
	UINTPTR output_info_pa;
	UINTPTR temp_buf_va;		  // Op's temp buffer address
	UINTPTR temp_buf_pa;
	INT32 axis;					      // layer's axis
} NN_DLI_REVERSE_PARM;
//@}

/**
	Pemrute layer parameters.
*/
//@{
typedef struct _NN_DLI_PERMUTE_PARM {
	UINT32 sign;                              ///< signature = MAKEFOURCC('P','P','M','1')  = Parameter permute V1
	UINT32 parm_size;                         ///< parm size in bytes.
	UINT32 mode;                              ///< NN_MODE: NN_DLI_SQRT, NN_DLI_DIV, ....
	UINT32 attr;                              ///< Reserved. Default is 0.
	UINTPTR next_layer_va;                    ///< Default is 0.
	UINTPTR next_layer_pa;                    ///< Default is 0.
	UINTPTR obj_ptr;                          ///< Default is 0.
	UINTPTR input_info_va;                    ///< Index at Tensor Info Table
	UINTPTR input_info_pa;
	UINTPTR output_info_va;                   ///< Same as input_info_va.
	UINTPTR output_info_pa;
	UINTPTR temp_buf_va;                      ///< Op's temp buffer address
	UINTPTR temp_buf_pa;
	NN_PERMUTE_ORDER perm;
}NN_DLI_PERMUTE_PARM;
//@}
#endif  /* _NN_DLI_H_ */
