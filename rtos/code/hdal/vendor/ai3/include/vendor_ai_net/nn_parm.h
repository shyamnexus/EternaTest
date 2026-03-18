/**
    @brief Header file of NN engine definition of vendor net flow sample.

    @file nn_parm.h

    @ingroup net_flow_sample

    @note Nothing.

    Copyright Novatek Microelectronics Corp. 2018.  All rights reserved.
*/
#ifndef _NN_PARM_H_
#define _NN_PARM_H_

/********************************************************************
    INCLUDE FILES
********************************************************************/
#include "kwrap/type.h"
#include "nn_net.h"
#include "nn_verinfo.h"

/********************************************************************
    MACRO CONSTANT DEFINITIONS
********************************************************************/
#define NN_NET_PARM_VERSION             "1.02.003"

#define NN_PRIORBOX_SIZE_NUM            4
#define NN_PRIORBOX_ASPECT_RATIO_NUM    4
#define NN_PRIORBOX_VAR_NUM             4

#define NN_LSTM_PARM_NUM                4

#define NN_AXIS_NUM                     4


/********************************************************************
    TYPE DEFINITION
********************************************************************/

#ifndef UINTPTR
typedef uintptr_t                       UINTPTR;
#endif

/**
    Input/Output type.
*/
//@{
typedef enum {
	NN_AI_IO_INT8 = 0,
	NN_AI_IO_UINT8 = 1,
	NN_AI_IO_INT16 = 2,
	NN_AI_IO_UINT16 = 3,
	NN_AI_IO_FLOAT32,
	ENUM_DUMMY4WORD(NN_AI_IO_TYPE)
} NN_AI_IO_TYPE;
//@}

/**
	select pooling mode.
*/
//@{
typedef enum {
	NN_AI_POOL_LOCAL_MAX = 0,               ///< max local pooling
	NN_AI_POOL_LOCAL_AVG = 1,               ///< average local pooling
	NN_AI_POOL_GLOBAL_MAX = 2,              ///< max global pooling
	NN_AI_POOL_GLOBAL_AVG = 3,              ///< average global pooling
	ENUM_DUMMY4WORD(NN_AI_POOL_MODE)
} NN_AI_POOL_MODE;
//@}

/**
    select pooling calculate type.
*/
//@{
typedef enum {
	NN_AI_CEIL         = 0,            ///< ceil output calculation
	NN_AI_FLOOR        = 1,            ///< floor output calculation
	ENUM_DUMMY4WORD(NN_AI_POOL_CAL_TYPE)
} NN_AI_POOL_CAL_TYPE;
//@}

/**
    select avarage pooling divide type.
*/
//@{
typedef enum {
	NN_AI_NON_BOUNDARY         = 0,           ///< not consider non-valid boundary point, div_num = ker_w * ker_h
	NN_AI_CONSIDER_BOUNDARY    = 1,           ///< consider non-valid boundary point, div_num = (ker_w-isEndx) * (ker_h- isEndy)
	ENUM_DUMMY4WORD(NN_AI_POOL_AVE_DIV_TYPE)
} NN_AI_POOL_AVE_DIV_TYPE;
//@}

/**
	image padding kernel parameters configuration
*/
//@{
typedef struct _NN_AI_PAD_KERPARM {
	UINT8 top_pad_num;              ///< number of padding top pixels
	UINT8 bot_pad_num;              ///< number of padding bottom pixels
	UINT8 left_pad_num;             ///< number of padding left pixels
	UINT8 right_pad_num;            ///< number of padding right pixels
	INT32 pad_val;                  ///< padding value based on KDRV_AI_IO_TYPE
} NN_AI_PAD_KERPARM;
//@}


/**
    scale-shift parameters configuration
*/
//@{
typedef struct _NN_AI_SCLSFT_KERPARM {
	INT8 in_shift;
	INT8 out_shift;
	UINT32 in_scale;
	UINT32 out_scale;
} NN_AI_SCLSFT_KERPARM;
//@}

/**
	local pooling kernel parameters configuration
*/
//@{
typedef struct _NN_AI_LOCAL_POOL_KERPARM {
	UINT8 ker_w;                    ///< pooling kernel width
	UINT8 ker_h;                    ///< pooling kernel height
	UINT8 ker_stridex;              ///< pooling kernel stride x
	UINT8 ker_stridey;              ///< pooling kernel stride y
	NN_AI_PAD_KERPARM pad;        ///< padding parameters
	NN_AI_POOL_CAL_TYPE pool_cal_type;                ///< pooling calculate type
	NN_AI_POOL_AVE_DIV_TYPE pool_div_type;            ///< pooling divide type
} NN_AI_LOCAL_POOL_KERPARM;
//@}

/**
    Global pooling kernel parameters.
*/
//@{
typedef struct _NN_AI_GLOBAL_POOL_KERPARM {
	INT32 avg_mul;                  ///< multiplier of average pooling results
	INT8  avg_shf;                  ///< shift of average pooling results; right shift(>0), left shift(<0)
} NN_AI_GLOBAL_POOL_KERPARM;
//@}

/**
	pooling kernel parameters configuration
*/
//@{
typedef struct _NN_AI_POOL_KERPARM {
	NN_AI_POOL_MODE mode;             ///< select pooling mode
	NN_AI_LOCAL_POOL_KERPARM local;   ///< local pooling parameters
	NN_AI_GLOBAL_POOL_KERPARM global; ///< global pooling parameters
	INT8  pool_shf;                     ///< shift after pooling; right shift(>0), left shift(<0)
	NN_AI_SCLSFT_KERPARM sclshf;              ///< scale-shift
} NN_AI_POOL_KERPARM;
//@}

/**
    Size parameters.
*/
//@{
typedef struct _NN_AI_SIZE {
	UINT16 width;                           ///< width
	UINT16 height;                          ///< height
	UINT16 channel;                         ///< number of channels
} NN_AI_SIZE;
//@}

/**
    Shape parameters.
*/
//@{
typedef struct _NN_SHAPE {
	UINT16 width;                           ///< width
	UINT16 height;                          ///< height
	UINT16 channel;                         ///< number of channels
	UINT16 batch_num;                       ///< batch number
} NN_SHAPE;
//@}

/**
    Offset parameters.
*/
//@{
typedef struct _NN_AI_OFS {
	UINT32 line_ofs;                        ///< line offset for multi-stripe mode or image mode
	UINT32 channel_ofs;                     ///< channel offst for multi-stripe mode
	UINT32 batch_ofs;                       ///< batch offset for batch mode
} NN_AI_OFS;
//@}

/**
    Scale factor parameters.
*/
//@{
typedef struct _NN_SF {
	INT32 mul;                              ///< multiply
	INT8 shf;                               ///< shift; right shift(>0), left shift(<0)
} NN_SF;
//@}
/**
	Quantization parameters.
*/
//@{
typedef struct _NN_QUANT {
	FLOAT scale;		///< scale
	INT32 offset;		///< zero_point
} NN_QUANT;
//@}

typedef NN_AI_SIZE  NN_SIZE;
typedef NN_AI_OFS   NN_OFS;

/**
    CPU layer parameters.
*/
//@{
typedef struct _NN_CPU_PARM {
	UINTPTR addr_in;
	UINTPTR addr_out;
	UINT32 size_in;
	UINT32 size_out;
	UINT32 width;
	UINT32 height;
	UINT32 channel;
	UINT32 batch;
	UINT32 in_lineofs;
	UINT32 in_channelofs;
	UINT32 in_batchofs;
	UINT32 out_lineofs;
	UINT32 out_channelofs;
	UINT32 out_batchofs;
} NN_CPU_PARM;
//@}

/**
    DSP layer parameters.
*/
//@{
typedef struct _NN_DSP_PARM {
	UINTPTR addr_in;
	UINTPTR addr_out;
	UINT32 size_in;
	UINT32 size_out;
} NN_DSP_PARM;
//@}

/**
    Activation function.
*/
//@{
typedef enum {
        LOGISTIC,        // Logistic ( \f$ f(x) = \frac{1}{1 + e^{-x}} \f$ ) 
    TANH,            // Hyperbolic tangent ( \f$ f(x) = a \cdot tanh(b \cdot x) \f$ ) 
    RELU,            // Rectifier ( \f$ f(x) = max(0,x) \f$ ) 
    BOUNDED_RELU,    // Upper Bounded Rectifier ( \f$ f(x) = min(a, max(0,x)) \f$ ) 
    LU_BOUNDED_RELU, // Lower and Upper Bounded Rectifier ( \f$ f(x) = min(a, max(b,x)) \f$ ) 
    LEAKY_RELU,      // Leaky Rectifier ( \f$ f(x) = \begin{cases}  \alpha x & \quad \text{if } x \text{ < 0}\\  x & \quad \text{if } x \geq \text{ 0 } \end{cases} \f$ ) 
    SOFT_RELU,       // Soft Rectifier ( \f$ f(x)= log(1+e^x) \f$ ) 
    ELU,             // Exponential Linear Unit ( \f$ f(x) = \begin{cases}  \alpha (exp(x) - 1) & \quad \text{if } x \text{ < 0}\\  x & \quad \text{if } x \geq \text{ 0 } \end{cases} \f$ ) */
    ABS,             // Absolute ( \f$ f(x)= |x| \f$ ) 
    SQUARE,          // Square ( \f$ f(x)= x^2 \f$ )
    SQRT,            // Square root ( \f$ f(x) = \sqrt{x} \f$ )
    LINEAR,          // Linear ( \f$ f(x)= ax + b \f$ )
    IDENTITY,        // Identity ( \f$ f(x)= x \f$ ) 
    HARD_SWISH,      // Hard-swish ( \f$ f(x) = (x * relu6(x+3))/6 \f$ ) 
    ENUM_DUMMY4WORD(NN_ACTIVATION_FUNCTION)
}NN_ACTIVATION_FUNCTION;

typedef struct _NN_ACTIVATION_LAYER_INFO{

	NN_ACTIVATION_FUNCTION _act;
    float              _a;
    float              _b;
    BOOL               _enabled ;
}NN_ACTIVATION_LAYER_INFO;
//@}

/**
    FullyConnected layer post-processing parameters.
*/
//@{
typedef struct _NN_FC_POST_PARM {
	UINTPTR in_addr;                         ///< input address
	UINTPTR out_addr;                        ///< output address
	UINTPTR tmp_addr;                        ///< temporary buffer address
	UINTPTR bias_addr;                       ///< bias address
	NN_SHAPE shape;                         ///< input shape
	NN_OFS in_ofs;                          ///< input offset
	NN_OFS out_ofs;                         ///< output offset
	NN_FMT in_fmt;                          ///< input format
	NN_FMT out_fmt;                         ///< output format
	NN_SF isf;                              ///< input scale factor
	NN_SF osf;                              ///< output scale factor
	INT32 leaky_value;
	INT8 leaky_shift;
} NN_FC_POST_PARM;

//@}

/**
    Pooling layer parameters.
*/
//@{
typedef struct _NN_POOL_PARM {
	UINTPTR in_addr;                         ///< input address
	UINTPTR out_addr;                        ///< output address
	UINTPTR tmp_addr;                        ///< temporary buffer address
	NN_SHAPE shape;                         ///< input shape
	NN_OFS in_ofs;                          ///< input offset
	NN_OFS out_ofs;                         ///< output offset
	NN_FMT in_fmt;                          ///< input format
	NN_FMT out_fmt;                         ///< output format
	NN_SF isf;                              ///< input scale factor
	NN_SF osf;                              ///< output scale factor
	NN_AI_POOL_KERPARM pool;                ///< pooling parameters
} NN_POOL_PARM;
//@}

/**
    BatchNormalization/Scale layer parameters.
    (obsoleted)
*/
//@{
typedef struct _NN_BNSCALE_PARM {
	UINT8 in_bitdepth;
	UINT8 in_sign;
	UINT8 out_bitdepth;
	UINT8 out_sign;
	UINTPTR in_addr;
	UINTPTR out_addr;
	UINTPTR mean_addr;
	UINTPTR alpha_addr;
	UINTPTR beta_addr;
	UINT32 width;
	UINT32 height;
	UINT32 channel;
	INT8 bn_shf_m;
	INT8 bn_shf_b;
	INT32 norm_scl;
	INT8 norm_shf;
	INT32 leaky_value;
	INT8 leaky_shift;
	INT8 relu_shift;
} NN_BNSCALE_PARM;
//@}

/**
    Softmax layer parameters.
*/
//@{
typedef struct _NN_SOFTMAX_PARM {
	UINTPTR in_addr;                         ///< input address
	UINTPTR out_addr;                        ///< output address
	UINTPTR tmp_addr;                        ///< temporary buffer address
	NN_SHAPE shape;                         ///< input/output shape
	NN_OFS in_ofs;                          ///< input offset
	NN_OFS out_ofs;                         ///< output offset
	NN_FMT in_fmt;                          ///< input format
	NN_FMT out_fmt;                         ///< output format
	NN_SF isf;                              ///< input scale factor
	NN_SF osf;                              ///< output scale factor
	INT8 axis;                              ///< axis to perform softmax
} NN_SOFTMAX_PARM;

/**
	Quantization Softmax layer parameters
*/
//@{
typedef struct _NN_SOFTMAX_ASYM_PARM {
	UINTPTR in_addr;						///< input address
	UINTPTR out_addr;						///< output address
	UINTPTR tmp_addr;						///< temp buffer address
	NN_SHAPE shape;							///< input/output shape
	NN_OFS in_ofs;							///< input offset
	NN_OFS out_ofs;							///< output offset
	NN_FMT in_fmt;                          ///< input format
	NN_FMT out_fmt;                         ///< output format
	NN_SF isf;                              ///< input scale factor
	NN_SF osf;                              ///< output scale factor
	NN_QUANT in_quant;						///< input quantization;
	NN_QUANT out_quant;						///< output quantization;
	INT8 axis;								///< axis to perform softmax
	UINTPTR in_trans_addr;
	UINTPTR out_trans_addr;
} NN_SOFTMAX_ASYM_PARM;
//@}

/**
    Fully Connected layer parameters.
*/
//@{
typedef struct _NN_FC_PARM {
	UINTPTR in_addr;                        ///< input address
	UINTPTR out_addr;                       ///< output address
	UINTPTR tmp_addr;                       ///< temporary buffer address
	UINTPTR weight_addr;                    ///< weight address
	UINTPTR bias_addr;                      ///< bias address
	NN_SHAPE in_shape;                      ///< input shape
	NN_SHAPE out_shape;                     ///< output shape
	NN_OFS in_ofs;                          ///< input offset
	NN_OFS out_ofs;                         ///< output offset
	NN_FMT in_fmt;                          ///< input format
	NN_FMT out_fmt;                         ///< output format
	NN_FMT weight_fmt;                      ///< weight format
	NN_FMT bias_fmt;                        ///< bias format
	NN_SF isf;                              ///< input scale factor
	NN_SF osf;                              ///< output scale factor
	BOOL transpose;            //Transpose weights if true
} NN_FC_PARM;
//@}

/**
    PReLU layer parameters.
*/
//@{
typedef struct _NN_PRELU_PARM {
	UINTPTR in_addr;
	UINTPTR out_addr;
	UINTPTR slope_addr;
	UINT32 width;
	UINT32 height;
	UINT32 channel;
	UINT32 batch_num;
	NN_FMT in_bit_fmt;
	NN_FMT out_bit_fmt;
	NN_FMT slope_bit_fmt;
	UINT8 channel_shared;
	INT32 out_scale;                    ///< output scale
	INT8 out_shift;                     ///< output shift; right shift(>0), left shift(<0)
} NN_PRELU_PARM;
//@}

/**
    LRN layer parameters.
*/
//@{
typedef struct _NN_LRN_PARM {
	UINTPTR in_addr;                         ///< input address
	UINTPTR out_addr;                        ///< output address
	UINTPTR tmp_addr;                        ///< temporary buffer address
	NN_SHAPE shape;                         ///< input/output shape
	NN_OFS in_ofs;                          ///< input offset
	NN_OFS out_ofs;                         ///< output offset
	NN_FMT in_fmt;                          ///< input format
	NN_FMT out_fmt;                         ///< output format
	NN_SF isf;                              ///< input scale factor
	NN_SF osf;                              ///< output scale factor
	UINT32 pre_pad;
	UINT32 size;
	UINT32 k;
	FLOAT  alpha;
	FLOAT  beta;
} NN_LRN_PARM;
//@}

/**
    PriorBox layer parameters.
*/
//@{
typedef struct _NN_PRIORBOX_PARM {
	UINTPTR in_addr;                         ///< input address
	UINTPTR out_addr;                        ///< output address
	UINTPTR tmp_addr;                        ///< temporary buffer address
	UINT32 in_width;
	UINT32 in_height;
	UINT32 img_width;
	UINT32 img_height;
	UINT32 out_channel_ofs;
	NN_FMT out_fmt;                         ///< output format
	NN_SF osf;                              ///< output scale factor
	FLOAT min_sizes[NN_PRIORBOX_SIZE_NUM];
	FLOAT max_sizes[NN_PRIORBOX_SIZE_NUM];
	UINT32 min_size_num;                    ///< number of max sizes
	UINT32 max_size_num;                    ///< number of min sizes
	FLOAT aspect_ratios[NN_PRIORBOX_ASPECT_RATIO_NUM];
	UINT32 aspect_ratio_num;
	UINT8 flip;
	UINT8 clip;
	FLOAT variances[NN_PRIORBOX_VAR_NUM];
	UINT32 variance_num;
	FLOAT offset;
} NN_PRIORBOX_PARM;
//@}

// Detection Output ------------------------------------------------------------
/**
	PriorBox layer: code type.
*/
typedef enum {
	NN_PRIORBOX_CODE_CENTER = 0,        ///< caffe.PriorBoxParameter.CENTER_SIZE
	NN_PRIORBOX_CODE_CORNER = 1,        ///< caffe.PriorBoxParameter.CORNER
	NN_PRIORBOX_CODE_CORNER_SIZE = 2,	///< caffe.PriorBoxParameter.CORNER_SIZE
	ENUM_DUMMY4WORD(NN_PRIROBOX_CODE_TYPE)
} NN_PRIORBOX_CODE_TYPE;

/**
	DetectionOutput layer: input type.
*/
//@{
typedef enum {
	NN_DETOUT_IN_LOC = 0,               ///< location
	NN_DETOUT_IN_CONF = 1,              ///< confidence
	NN_DETOUT_IN_PRIOR = 2,             ///< prior box

	NN_DETOUT_IN_NUM,                   ///< number of input
	ENUM_DUMMY4WORD(NN_DETOUT_IN_TYPE)
} NN_DETOUT_IN_TYPE;
//@}

/**
    DetectionOutput layer parameters.
*/
//@{
typedef struct _NN_DETOUT_PARM {
	UINTPTR in_addr[NN_DETOUT_IN_NUM];       ///< input addresses
	UINTPTR out_addr;                        ///< output address
	UINTPTR tmp_addr;                        ///< temporary buffer address
	UINT16 batch_num;                       ///< batch number
	NN_OFS in_ofs[NN_DETOUT_IN_NUM];        ///< input offset
	NN_OFS out_ofs;                         ///< output offset
	NN_FMT in_fmt[NN_DETOUT_IN_NUM];        ///< input format
	NN_FMT out_fmt;                         ///< output format
	NN_SF isf[NN_DETOUT_IN_NUM];            ///< input scale factor
	NN_SF osf;                              ///< output scale factor
	UINT32 num_classes;                     ///< number of classes
	UINT32 num_priors;                      ///< number of prior boxes
	BOOL share_loc;                         ///< share location
	UINT32 bg_lbl_id;                       ///< background label id
	NN_PRIORBOX_CODE_TYPE code_type;        ///< code type
	BOOL var_enc_in_target;                 ///< variance encoded in_target
	UINT32 keep_top_k;                      ///< keep top k
	FLOAT conf_thresh;                      ///< confidence threshold
	FLOAT nms_thresh;                       ///< NMS threshold
	UINT32 top_k;                           ///< top k
	FLOAT eta;                              ///< eta
} NN_DETOUT_PARM;
//@}
// Detection Output end --------------------------------------------------------

/**
    LSTM layer parameters.
*/
//@{
typedef struct _NN_LSTM_PARM {
	UINTPTR in_addr0;                            ///< input-0 address
	UINTPTR in_addr1;                            ///< input-1 address
	UINTPTR out_addr;                            ///< output address
	UINTPTR tmp_addr;                            ///< temporary buffer address
	UINTPTR indicator_parm_addr;                 ///< indicator buffer address
	UINTPTR feat_parm_addr[NN_LSTM_PARM_NUM];    ///< feature parameter address
	UINTPTR static_parm_addr[NN_LSTM_PARM_NUM];  ///< static parameter address
	UINTPTR hidden_parm_addr[NN_LSTM_PARM_NUM];  ///< hidden parameter address
	UINTPTR bias_parm_addr[NN_LSTM_PARM_NUM];    ///< bias parameter address
	NN_SHAPE in_shape0;                         ///< input-0 shape
	NN_SHAPE in_shape1;                         ///< input-1 shape
	NN_OFS in_ofs0;                             ///< input-0 offset
	NN_OFS in_ofs1;                             ///< input-1 offset
	NN_OFS out_ofs;                             ///< output offset
	NN_FMT in_fmt0;                             ///< input-0 format
	NN_FMT in_fmt1;                             ///< input-1 format
	NN_FMT out_fmt;                             ///< output format
	NN_SF isf0;                                 ///< input-0 scale factor
	NN_SF isf1;                                 ///< input-1 scale factor
	NN_SF osf;                                  ///< output scale factor
	BOOL has_static_input;                      ///< Has static input, if enable, has 2 input
	UINT32 num_output;                          ///< feat number
}NN_LSTM_PARM;
//@}

typedef struct _NN_PERMUTE_ORDER {
	INT8 order[NN_AXIS_NUM];
}NN_PERMUTE_ORDER;

/**
    Permute layer parameters.
*/
//@{
typedef struct _NN_PERMUTE_PARM {
	UINTPTR in_addr;                         ///< input address
	UINTPTR out_addr;                        ///< output address
	UINTPTR tmp_addr;                        ///< temporary buffer address (unused)
	NN_SHAPE shape;                         ///< input/output shape
	NN_OFS in_ofs;                          ///< input offset
	NN_OFS out_ofs;                         ///< output offset
	NN_FMT in_fmt;                          ///< input format
	NN_FMT out_fmt;                         ///< output format
	NN_SF isf;                              ///< input scale factor
	NN_SF osf;                              ///< output scale factor
	INT8 order[NN_AXIS_NUM];                ///< permute order
} NN_PERMUTE_PARM;
//@}

/**
    Reverse layer parameters.
*/
//@{
typedef struct _NN_REVERSE_PARM {
	UINTPTR in_addr;                         ///< input address
	UINTPTR out_addr;                        ///< output address
	UINTPTR tmp_addr;                        ///< temporary buffer address (unused)
	NN_SHAPE shape;                         ///< input/output shape
	NN_OFS in_ofs;                          ///< input offset
	NN_OFS out_ofs;                         ///< output offset
	NN_FMT in_fmt;                          ///< input format
	NN_FMT out_fmt;                         ///< output format
	NN_SF isf;                              ///< input scale factor
	NN_SF osf;                              ///< output scale factor
	INT8 axis;                              ///< axis to perform the operation
} NN_REVERSE_PARM;
//@}

/**
    Normalize layer parameters.
*/
//@{
typedef struct _NN_NORM_PARM {
	uintptr_t in_addr;                         ///< input address
	uintptr_t out_addr;                        ///< output address
	uintptr_t tmp_addr;                        ///< temporary buffer address
	uintptr_t scale_addr;                      ///< scale parameters address
	NN_SHAPE shape;                         ///< input/output shape
	NN_OFS in_ofs;                          ///< input offset
	NN_OFS out_ofs;                         ///< output offset
	NN_FMT in_fmt;                          ///< input format
	NN_FMT out_fmt;                         ///< output format
	NN_FMT scale_fmt;                       ///< scale parameters format
	NN_SF isf;                              ///< input scale factor
	NN_SF osf;                              ///< output scale factor
	UINT8 across_spatial;                   ///< normalize across channels
	UINT8 channel_shared;                   ///< whether or not scale parameters are shared across channels
	FLOAT eps;                              ///< epsilon for not dividing by zero while normalizing variance
} NN_NORM_PARM;
//@}

/**
    ELEMENTWISE layer: operation.
*/
//@{
typedef enum {
	NVTNN_DLI_ELEMENTWISE_OP_ADD = 0,                 ///< add
	NVTNN_DLI_ELEMENTWISE_OP_SUB = 1,                 ///< sub
	NVTNN_DLI_ELEMENTWISE_OP_DIV = 0x101,             ///< div
	NVTNN_DLI_ELEMENTWISE_OP_MIN = 3,                 ///< minimum
	NVTNN_DLI_ELEMENTWISE_OP_MAX = 4,                 ///< maximum
	NVTNN_DLI_ELEMENTWISE_OP_SQUARED_DIFF = 5,        ///< MSE mean square error
	NVTNN_DLI_ELEMENTWISE_OP_PROD = 6,                ///< product
	NVTNN_DLI_ELEMENTWISE_OP_SQRT = 0x100,            ///< sqrt
	NVTNN_DLI_ELEMENTWISE_OP_EXP  = 0x102,            ///< exp
	NVTNN_DLI_ELEMENTWISE_OP_POW  = 0x105,            ///< pow
	ENUM_DUMMY4WORD(NN_ELEMENTWISE_OP)
} NN_ELEMENTWISE_OP;
//@}

/**
    ELEMENTWISE layer parameters.
*/
//@{
typedef struct _NN_ELEMENTWISE_PARM {
	UINTPTR in_addr0;                       ///< input-0 address
	UINTPTR in_addr1;                       ///< input-1 address
	UINTPTR out_addr;                       ///< output address
	UINTPTR tmp_addr;                       ///< temporary buffer address
	NN_SHAPE shape;                         ///< input/output shape
	NN_OFS in_ofs0;                         ///< input-0 offset
	NN_OFS in_ofs1;                         ///< input-1 offset
	NN_OFS out_ofs;                         ///< output offset
	NN_FMT in_fmt0;                         ///< input-0 format
	NN_FMT in_fmt1;                         ///< input-1 format
	NN_FMT out_fmt;                         ///< output format
	NN_SF isf0;                             ///< input-0 scale factor
	NN_SF isf1;                             ///< input-1 scale factor
	NN_SF osf;                              ///< output scale factor
	NN_ELEMENTWISE_OP op;                   ///< operation
	// FLOAT coeff0;                        ///< coefficient-0 for SUM operation
	// FLOAT coeff1;                        ///< coefficient-1 for SUM operation
} NN_ELEMENTWISE_PARM;
//@}

typedef struct  _NN_LAYER_NORMALIZATION_PARM{
	UINTPTR in_addr;                        ///< input address
	UINTPTR out_addr;                       ///< output address
	UINTPTR tmp_addr;                       ///< temporary buffer address
	UINTPTR gamma_addr;                     ///< instance normalization gamma parameters address
	UINTPTR beta_addr;                      ///< instance normalization beta parameters address
	NN_SHAPE shape;                         ///< input/output shape
	NN_OFS in_ofs;                          ///< input offset
	NN_OFS out_ofs;                         ///< output offset
	NN_FMT in_fmt;                          ///< input format
	NN_FMT out_fmt;                         ///< output format
	NN_FMT gamma_fmt;                       ///< gamma format
	NN_FMT beta_fmt;                        ///< beta format
	NN_SF isf;                              ///< input scale factor
	NN_SF osf;                              ///< output scale factor
	UINT32 normalized_shape[6];             ///< pytorch normalized_shape
	UINT32 elementwise_affine;              ///< multiply gamma add beta elementwise_affine pytorch default 0
	FLOAT eps;                              ///< epsilon for not dividing by zero
} NN_LAYER_NORMALIZATION_PARM;

/**
    Instance normalization layer parameters.
*/
typedef struct _NN_INSTANCE_NORMALIZATION_PARM {
	UINTPTR in_addr;                        ///< input address
	UINTPTR out_addr;                       ///< output address
	UINTPTR tmp_addr;                       ///< temporary buffer address
	UINTPTR mean_addr;                      ///< instance normalization moving_average_mean parameters address
	UINTPTR variance_addr;                  ///< instance normalization moving_average_variance parameters address
	UINTPTR gamma_addr;                     ///< instance normalization gamma parameters address
	UINTPTR beta_addr;                      ///< instance normalization beta parameters address
	NN_SHAPE shape;                         ///< input/output shape
	NN_OFS in_ofs;                          ///< input offset
	NN_OFS out_ofs;                         ///< output offset
	NN_FMT in_fmt;                          ///< input format
	NN_FMT out_fmt;                         ///< output format
	NN_FMT mean_fmt;                        ///< mean format
	NN_FMT variance_fmt;                    ///< variance format
	NN_FMT gamma_fmt;                       ///< gamma format
	NN_FMT beta_fmt;                        ///< beta format
	NN_SF isf;                              ///< input scale factor
	NN_SF osf;                              ///< output scale factor
	UINT32 track_running_stats;             ///< save moving average mean and variance track_running_stats pytorch default false
	UINT32 affine;                          ///< this module has learnable affine parameters pytorch default False
	FLOAT eps;                              ///< epsilon for not dividing by zero
} NN_INSTANCE_NORMALIZATION_PARM;
/**
    Deconvolution layer parameters.
*/
//@{
typedef struct _NN_DECONVOLUTION_PARM {
	UINTPTR in_addr;                        ///< input address
	UINTPTR out_addr;                       ///< output address
	UINTPTR tmp_addr;                       ///< temporary buffer address
	UINTPTR weight_addr;                    ///< weight parameters address
	UINTPTR bias_addr;                      ///< bias parameters address
	NN_SHAPE inshape;                       ///< input  shape
	NN_SHAPE outshape;                      ///< output shape
	NN_SHAPE wtshape;                       ///< weight shape
	NN_OFS in_ofs;                          ///< input  offset
	NN_OFS out_ofs;                         ///< output offset
	NN_OFS weight_ofs;                      ///< weight offset
	NN_FMT in_fmt;                          ///< input  format
	NN_FMT out_fmt;                         ///< output format
	NN_FMT weight_fmt;                      ///< weight format
	NN_FMT bias_fmt;                        ///< bias   format
	NN_SF isf;                              ///< input  scale factor
	NN_SF osf;                              ///< output scale factor
	NN_SF wsf;                              ///< weight scale factor
	NN_SF bsf;                              ///< bias   scale factor
	UINT32 kernel_w;                        ///< kernel_w
	UINT32 kernel_h;                        ///< kernel_h
	UINT32 stride_w;                        ///< stride_w
	UINT32 stride_h;                        ///< stride_h
	UINT32 dilation_w;                      ///< dilation_w
	UINT32 dilation_h;                      ///< dilation_h
	UINT32 pad_w;                           ///< pad_w
	UINT32 pad_h;                           ///< pad_h
	UINT32 group;                           ///< group
	UINT32 num_output;                      ///< num_output/output channel
	UINT32 output_padding;                  ///< output padding
	UINT32 bias;                            ///< bias term
} NN_DECONVOLUTION_PARM;
//@}

/**
    Matrix multiplication layer parameters.
*/
//@{
typedef struct _NN_MATMUL_PARM {
	UINTPTR in_addr0;                       ///< input-0 address
	UINTPTR in_addr1;                       ///< input-1 address
	UINTPTR out_addr;                       ///< output address
	UINTPTR tmp_addr;                       ///< temporary buffer address
	NN_SHAPE in_shape0;                     ///< input-0 shape
	NN_SHAPE in_shape1;                     ///< input-1 shape
	NN_OFS in_ofs0;                         ///< input-0 offset
	NN_OFS in_ofs1;                         ///< input-1 offset
	NN_OFS out_ofs;                         ///< output offset
	NN_FMT in_fmt0;                         ///< input-0 format
	NN_FMT in_fmt1;                         ///< input-1 format
	NN_FMT out_fmt;                         ///< output format
	NN_SF isf0;                             ///< input-0 scale factor
	NN_SF isf1;                             ///< input-1 scale factor
	NN_SF osf;                              ///< output scale factor
} NN_MATMUL_PARM;
//@}

typedef enum {
	NN_ELTWISE_OP_PROD = 0,                 ///< product
	NN_ELTWISE_OP_SUM  = 1,                 ///< summation
	NN_ELTWISE_OP_MAX  = 2,                 ///< maximum
	ENUM_DUMMY4WORD(NN_ELTWISE_OP)
} NN_ELTWISE_OP;
//@}

/**
    Eltwise layer parameters.
*/
//@{
typedef struct _NN_ELTWISE_PARM {
	UINTPTR in_addr0;                       ///< input-0 address
	UINTPTR in_addr1;                       ///< input-1 address
	UINTPTR out_addr;                       ///< output address
	UINTPTR tmp_addr;                       ///< temporary buffer address
	NN_SHAPE shape;                         ///< input/output shape
	NN_OFS in_ofs0;                         ///< input-0 offset
	NN_OFS in_ofs1;                         ///< input-1 offset
	NN_OFS out_ofs;                         ///< output offset
	NN_FMT in_fmt0;                         ///< input-0 format
	NN_FMT in_fmt1;                         ///< input-1 format
	NN_FMT out_fmt;                         ///< output format
	NN_SF isf0;                             ///< input-0 scale factor
	NN_SF isf1;                             ///< input-1 scale factor
	NN_SF osf;                              ///< output scale factor
	NN_ELTWISE_OP op;                       ///< operation
	FLOAT coeff0;                           ///< coefficient-0 for SUM operation
	FLOAT coeff1;                           ///< coefficient-1 for SUM operation
} NN_ELTWISE_PARM;
//@}

/**
    Convolution parameters.
*/
//@{
typedef enum _NN_DIMENSIONROUNDINGTYPE
{
    FLOOR, /**< Floor rounding */
    CEIL   /**< Ceil rounding */
}NN_DIMENSIONROUNDINGTYPE;

typedef struct _NN_PAD_STRIDE_INFO{
	UINT32 stride[2];
    UINT32 pad_left;
    UINT32 pad_top;
    UINT32 pad_right;
    UINT32 pad_bottom;
	NN_DIMENSIONROUNDINGTYPE round_type;
}NN_PAD_STRIDE_INFO;

typedef struct _NN_SIZE2D{
    UINT32 width;
    UINT32 height;
}NN_SIZE2D;

typedef enum{
	NN_DLI_POOLING_OP_MAX = 0, ///< Max Pooling
	NN_DLI_POOLING_OP_AVE = 1, ///< Average Pooling
	NN_DLI_POOLING_OP_L2  = 2, ///< L2 Pooing
	ENUM_DUMMY4WORD(NN_POOLING_TYPE)
}NN_POOLING_TYPE;

typedef struct _NN_POOLING_LAYER_INFO{
	NN_POOLING_TYPE     pool_type;
	NN_SIZE2D           pool_size;
	NN_PAD_STRIDE_INFO  pad_stride_info;     ///< (Optional) Padding and stride informaiton PadStrideInfo
	BOOL                exclude_padding;     ///< (Optional) Default = false, Strategy when accounting padding in calculations. True will exclude padding while false will not (Used in AVG/L2 pooling to determine the pooling area).
	BOOL                is_global_pooing;    ///< (Optional) Default = false
	BOOL                fp_mixed_precision;  ///< (Optional) Use wider accumulators (32 bit instead of 16 for FP16) to improve accuracy.
}NN_POOLING_LAYER_INFO;

typedef struct _NN_WEIGHTS_INFO{
	BOOL are_reshaped;
	UINT32 kernel_width;
	UINT32 kernel_height;
	UINT32 num_kernels;
	BOOL retain_internal_weights;
}NN_WEIGHTS_INFO;

typedef struct _NN_DILATEDCONV_PARM {
	UINTPTR in_addr;                         ///< input address
	UINTPTR out_addr;                         ///< output address
	UINTPTR tmp_addr;                         ///< temporary buffer address
	UINTPTR weight_addr;
	UINTPTR bias_addr;
	NN_SHAPE shape;                         ///< input shape
	NN_OFS in_ofs;                         ///< input offset
	NN_OFS out_ofs;                         ///< output offset
	NN_FMT in_fmt;                          ///< input format
	NN_FMT out_fmt;                         ///< output format
	NN_FMT weight_fmt;                      ///< weight format
	NN_FMT bias_fmt;                         //////< bias format
	NN_SF isf;                              ///< input scale factor
	NN_SF osf;                              ///< output scale factor
	UINT8 dilation_rate;
	UINT8 kernel_size;
	UINT8 stride_x;
	UINT8 stride_y;
	NN_AI_PAD_KERPARM pad_info;
	UINT32 kernel_num;
	UINT32 num_groups;
	NN_PAD_STRIDE_INFO       pad_stride_info;        /**< Convolution info (Pads, strides,...) */
    UINT32        	         depth_multiplier;       /**< Multiplier to apply to input's depth to retrieve the output depth. Defaults to 1 */
    NN_ACTIVATION_LAYER_INFO act_info;               /**< Fused activation to apply after convolution. */
    NN_SIZE2D              	 dilation; 

} NN_DILATEDCONV_PARM;
//@}

/**
    TopK layer parameters.
*/
//@{
typedef struct _NN_TOPK_PARM {
	UINTPTR in_addr;
	UINTPTR out_addr_val;
	UINTPTR out_addr_idx;
	UINTPTR tmp_addr;
	NN_OFS in_ofs;
	NN_OFS out_ofs_val;
	NN_OFS out_ofs_idx;
	NN_FMT in_fmt;
	NN_FMT out_fmt_val;
	NN_FMT out_fmt_idx;
	NN_SF in_sf;
	NN_SF out_sf_val;
	NN_SF out_sf_idx;
	NN_SHAPE shape;
	INT8 dim_num;
	INT8 axis;
	INT8 largest;
	INT8 sorted;
	INT32 k;
} NN_TOPK_PARM;
//@}


/**
    GridSample layer: interpolation mode.
*/
//@{
typedef enum {
	NN_GS_BILINEAR          = 0,            ///< bilinear
	NN_GS_NEAREST           = 1,            ///< nearest
	NN_GS_BICUBIC           = 2,            ///< bicubic
	ENUM_DUMMY4WORD(NN_GRIDSAMPLE_MODE)
} NN_GRIDSAMPLE_MODE;
//@}

/**
    GridSample layer: padding mode.
*/
//@{
typedef enum {
	NN_GS_ZEROS             = 0,            ///< zeros
	NN_GS_BORDER            = 1,            ///< border
	NN_GS_REFLECTION        = 2,            ///< reflection
	ENUM_DUMMY4WORD(NN_GRIDSAMPLE_PADDING)
} NN_GRIDSAMPLE_PADDING;
//@}

/**
    GridSample layer parameters.
*/
//@{
typedef struct _NN_GRIDSAMPLE_PARM {
	UINTPTR in_addr;                        ///< input address
	UINTPTR out_addr;                       ///< output address
	UINTPTR tmp_addr;                       ///< temporary buffer address
	UINTPTR grid_addr;                      ///< grid address
	NN_SHAPE in_shape;                      ///< input shape
	NN_SHAPE out_shape;                     ///< output shape
	NN_SHAPE grid_shape;                    ///< grid shape
	NN_OFS in_ofs;                          ///< input offset
	NN_OFS out_ofs;                         ///< output offset
	NN_OFS grid_ofs;                        ///< grid offset
	NN_FMT in_fmt;                          ///< input format
	NN_FMT out_fmt;                         ///< output format
	NN_FMT grid_fmt;                        ///< grid format
	NN_SF isf;                              ///< input scale factor
	NN_SF osf;                              ///< output scale factor
	NN_SF gsf;                              ///< grid scale factor
	NN_QUANT in_quant;                      ///< input quant
	NN_QUANT out_quant;                     ///< output quant
	NN_QUANT grid_quant;                    ///< grid quant
	BOOL align_corners;                     ///< 0: align corner points; 1: align center points
	NN_GRIDSAMPLE_MODE mode;                ///< interpolation mode
	NN_GRIDSAMPLE_PADDING padding;          ///< padding mode
	UINT32 layer_num;                       ///< number of grid sample layers to concat output
	UINT32 layer_idx;                       ///< grid sample layer index
} NN_GRIDSAMPLE_PARM;
//@}

/**
    Resize layer parameters.
*/
//@{
typedef struct _NN_RESIZE_PARM {

	UINTPTR in_addr;
	UINTPTR out_addr;
	UINTPTR tmp_addr;
	NN_SHAPE shape;
	NN_OFS in_ofs;
	NN_OFS out_ofs;
	NN_FMT in_fmt;
	NN_FMT out_fmt;

	UINT32 dst_rows;
	UINT32 dst_cols;
    INT32 tmp_step;
    INT32 filter_size;
	// ACL
	UINT8 interpolation_policy; ///< NN_DLI_INTERPOLATION_POLICY. Default is 0 (NN_DLI_INTERPOLATION_POLICY_NEAREST_NEIGHBOR).
	UINT8 border_mode; ///< NN_DLI_BORDER_MODE. Default is 2 (NN_DLI_BORDER_MODE_REPLICATE).
	UINT8 sampling_policy; ///< NN_DLI_SAMPLING_POLICY. Default is 0 (NN_DLI_SAMPLING_POLICY_CENTER).
	UINT8 align_corners; ///< Bool. Default is 0 (FALSE).
	UINT32 __padding; ///< Reserved. Default is 0.

} NN_RESIZE_PARM;
//@}

/**
	Gather layer parameters.
*/
//@{
typedef struct _NN_GATHER_PARM{
	UINTPTR in_addr;						///< input address
	UINTPTR out_addr;						///< output address
	UINTPTR ind_addr;						///< index address
	UINTPTR tmp_addr;						///< temporary buffer address
	NN_SHAPE in_shape;						///< input shape
	NN_SHAPE ind_shape;					///< index shape
	NN_SHAPE out_shape;					///< output shape
	NN_OFS in_ofs;							///< input offset
	NN_OFS out_ofs;							///< output offset
	NN_FMT in_fmt;							///< input format
	NN_FMT ind_fmt;							///< indice format
	NN_FMT out_fmt;							///< output format
	NN_SF isf;								///< input scale factor
	NN_SF osf;								///< output scale factor
	INT8 axis;								///< input dim
} NN_GATHER_PARM;
//@}

/**
	GatherND layer parameters.
*/
//@{
typedef struct  _NN_GATHERND_PARM{
	UINTPTR in_addr;                        ///< input address
	UINTPTR out_addr;                       ///< output address
	UINTPTR indices_addr;                   ///< indices buffer address
	UINTPTR tmp_addr;                       // 32/64 diff
	NN_SHAPE in_shape;                      ///< input/output shape
	NN_SHAPE out_shape;                     ///< input/output shape
	NN_SHAPE indices_shape;                 ///< input/output shape
	NN_OFS in_ofs;                          ///< input offset
	NN_OFS out_ofs;                         ///< output offset
	NN_FMT in_fmt;                          ///< input format
	NN_FMT ind_fmt;						///< indice format
	NN_FMT out_fmt;                         ///< output format
	NN_SF isf;                              ///< input scale factor
	NN_SF osf;                              ///< output scale factor
	INT32 batch_dims;                       ///< onnx normalized_shape
} NN_GATHERND_PARM;
//@}

/**
	GatherElement layer parameters.
*/
//@{
typedef struct _NN_GATHERELEMENT_PARM{
	UINTPTR in_addr;                      ///< input address
	UINTPTR out_addr;                     ///< output address
	UINTPTR indices_addr;                 ///< indices address
    UINTPTR tmp_addr;                     /// tmp_buffer
	NN_SHAPE in_shape;                    ///< input shape
	NN_SHAPE out_shape;                   ///< output shape
	NN_SHAPE indices_shape;               ///< indices shape
	NN_OFS in_ofs;                        ///< input offset
    NN_OFS index_ofs;                     ///< indice offset
	NN_OFS out_ofs;                       ///< output offset
	NN_FMT in_fmt;                        ///< input format
    NN_FMT ind_fmt;                       ///< indice format
	NN_FMT out_fmt;                       ///< output format
	NN_SF isf;                            ///< input scale factor
	NN_SF osf;                            ///< output scale factor
	INT8 axis;                            ///< dim
} NN_GATHERELEMENT_PARM;
//@}

/**
	OneHot layer parameters.
*/
//@{
typedef struct _NN_ONEHOT_PARM{
	UINTPTR in_addr;                      ///< input address
	UINTPTR out_addr;                     ///< output address
	UINTPTR value_addr;                   ///< value address
	NN_SHAPE in_shape;                    ///< input shape
	NN_SHAPE out_shape;                   ///< output shape
	NN_OFS in_ofs;                        ///< input offset
	NN_OFS out_ofs;                       ///< output offset
	NN_FMT in_fmt;                        ///< input format
	NN_FMT out_fmt;                       ///< output format
	NN_FMT value_fmt;                     ///< value format
	NN_SF isf;                            ///< input scale factor
	NN_SF osf;                            ///< output scale factor
	NN_SF vsf;                            ///< value scale factor
	INT32 in_dim_num;                     ///< in_dim_num
	INT32 axis;                           ///< axis
	INT32 depth;                          ///< depth
} NN_ONEHOT_PARM;
//@}

/**
    CumSum layer parameters.
*/
//@{
typedef struct _NN_CUMSUM_PARM {
	UINTPTR in_addr;                        ///< input address
	UINTPTR out_addr;                       ///< output address
	UINTPTR tmp_addr;                       ///< temporary buffer address
	NN_SHAPE in_shape;                      ///< input shape
	NN_SHAPE out_shape;                     ///< output shape
	NN_OFS in_ofs;                          ///< input offset
	NN_OFS out_ofs;                         ///< output offset
	NN_FMT in_fmt;                          ///< input format
	NN_FMT out_fmt;                         ///< output format
	NN_SF isf;                              ///< input scale factor
	NN_SF osf;                              ///< output scale factor 
	INT32 axis;	
	INT32 exclusive;	
	INT32 reverse;
} NN_CUMSUM_PARM;
//@}

// Custom ----------------------------------------------------------------------
/**
    Custom layer parameters.
*/
//@{
typedef struct _NN_CUSTOM_PARM {
	UINT32 input_num;
	UINT32 output_num;
	UINT32 model_num;
	UINTPTR temp_buf_addr;
	UINT32 temp_buf_size;
	UINT32 parm_size;
	/*
	NN_DATA* input;    // size = sizeof(NN_DATA)*input_num
	NN_DATA* output; // size = sizeof(NN_DATA)*output_num
	NN_DATA* model;  // size = sizeof(NN_DATA)*model_num
	NN_CUSTOM_DIM* input_dim;  // size = sizeof(NN_CUSTOM_DIM)*input_num
	NN_CUSTOM_DIM* output_dim; // size = sizeof(NN_CUSTOM_DIM)*output_num
	...
	custom parameters
	*/
} NN_CUSTOM_PARM;
//@}

#endif  /* _NN_PARM_H_ */