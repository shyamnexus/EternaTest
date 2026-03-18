#ifndef _IVE_ENG_INT_REGISTER_H_
#define _IVE_ENG_INT_REGISTER_H_

#ifdef __cplusplus
extern "C" {
#endif

#if defined (__LINUX)
/*
#if defined(__aarch64__)
#include "linux/soc/nvt/rcw_macro.h"
#else
#include "rcw_macro.h"
#endif
*/
#include <linux/types.h>
#include <linux/soc/nvt/nvt-io.h>
#include "plat/top.h"
#include "kwrap/type.h"
#include "kwrap/nvt_type.h"

//=========================================================================
#elif defined (__FREERTOS)


#include "rcw_macro.h"
#include "top.h"
#include "kwrap/type.h"

#include "kwrap/nvt_type.h"

#else


#endif

#define REGVALUE                uint32_t
#define UBITFIELD		unsigned int 	/* Unsigned bit field */
#define BITFIELD 		signed int	/* Signed bit field */

#define ASSERT_CONCAT_(a, b)    a##b
#define ASSERT_CONCAT(a, b)     ASSERT_CONCAT_(a, b)

#if defined(__COUNTER__)
#define STATIC_ASSERT(expr) \
enum { ASSERT_CONCAT(FAILED_STATIC_ASSERT_, __COUNTER__) = 1/(expr) }
#else
#define STATIC_ASSERT(expr) \
enum { ASSERT_CONCAT(FAILED_STATIC_ASSERT_, __LINE__) = 1/(expr) }
#endif

//#define OUTW(addr, value)    	nvt_writel(value, addr)
//#define INW(addr)           	nvt_readl(addr)

#define REGDEF_BEGIN(name)      \
typedef union                   \
{                               \
    REGVALUE    reg;            \
    struct                      \
    {

#define REGDEF_BIT(field, bits) \
    UBITFIELD   field : bits;

#define REGDEF_END(name)        \
    } bit;                      \
} T_##name;                     \
STATIC_ASSERT(sizeof(T_##name) == sizeof(REGVALUE));


/*
    IVE_RST    :    [0x0, 0x1],          bits : 0
    IVE_START  :    [0x0, 0x1],          bits : 1
    LL_FIRE    :    [0x0, 0x1],          bits : 28
*/
#define GLOBAL_CONTROL_REGISTER_OFS 0x0000
REGDEF_BEGIN(GLOBAL_CONTROL_REGISTER)
REGDEF_BIT(IVE_RST,          1)
REGDEF_BIT(IVE_START,        1)
REGDEF_BIT(     ,           26)
REGDEF_BIT(LL_FIRE,          1)
REGDEF_BIT(         ,        3)
REGDEF_END(GLOBAL_CONTROL_REGISTER)


/*
    INTE_FRM_END                 :    [0x0, 0x1],           bits : 0
    INTE_LL_END                  :    [0x0, 0x1],           bits : 8
    INTE_LL_ERROR                :    [0x0, 0x1],           bits : 9
    INTE_LL_JOB_END              :    [0x0, 0x1],           bits : 10
    NTE_CCL_LABEL_OVERFLOW       :    [0x0, 0x1],           bits : 16
    INTE_CCL_PIXEL_NUM_OVERFLOW  :    [0x0, 0x1],           bits : 17
*/
#define GLOBAL_INTERRUPT_REGISTER0_OFS 0x0004
REGDEF_BEGIN(GLOBAL_INTERRUPT_REGISTER0)
REGDEF_BIT(INTE_FRM_END,                   1)
REGDEF_BIT(,                               7)
REGDEF_BIT(INTE_LL_END,                    1)
REGDEF_BIT(INTE_LL_ERROR,                  1)
REGDEF_BIT(INTE_LL_JOB_END,                1)
REGDEF_BIT(,                               5)
REGDEF_BIT(INTE_CCL_LABEL_OVERFLOW,        1)
REGDEF_BIT(INTE_CCL_PIXEL_NUM_OVERFLOW,    1)
REGDEF_BIT(,                              14)
REGDEF_END(GLOBAL_INTERRUPT_REGISTER0)


/*
    INTS_FRM_END                 :    [0x0, 0x1],            bits : 0
    INTS_LL_END                  :    [0x0, 0x1],            bits : 8
    INTS_LL_ERROR                :    [0x0, 0x1],            bits : 9
    INTS_LL_JOB_END              :    [0x0, 0x1],            bits : 10
    INTS_CCL_LABEL_OVERFLOW      :    [0x0, 0x1],            bits : 16
    INTS_CCL_PIXEL_NUM_OVERFLOW  :    [0x0, 0x1],            bits : 17
*/
#define GLOBAL_INTERRUPT_REGISTER1_OFS 0x0008
REGDEF_BEGIN(GLOBAL_INTERRUPT_REGISTER1)
REGDEF_BIT(INTS_FRM_END,                   1)
REGDEF_BIT(,                               7)
REGDEF_BIT(INTS_LL_END,                    1)
REGDEF_BIT(INTS_LL_ERROR,                  1)
REGDEF_BIT(INTS_LL_JOB_END,                1)
REGDEF_BIT(,                               5)
REGDEF_BIT(INTS_CCL_LABEL_OVERFLOW,        1)
REGDEF_BIT(INTS_CCL_PIXEL_NUM_OVERFLOW,    1)
REGDEF_BIT(,                              14)
REGDEF_END(GLOBAL_INTERRUPT_REGISTER1)


/*
    LL_TIRMINATE                 :    [0x0, 0x1],            bits : 0
*/
#define LL_TERMINATE_REGISTER_OFS 0x000c
REGDEF_BEGIN(LL_TERMINATE_REGISTER)
REGDEF_BIT(LL_TREMINATE,        1)
REGDEF_BIT(,        31)
REGDEF_END(LL_TERMINATE_REGISTER)


/*
    IVE_MODE                :    [0x0, 0xFF],           bits : 7_0
    GEN_FILT_EN             :    [0x0, 0x1],            bits : 8
    ORST_FILT_EN            :    [0x0, 0x1],            bits : 9
    EDGE_FILT_EN            :    [0x0, 0x1],            bits : 10
    NON_MAX_SUP_EN          :    [0x0, 0x1],            bits : 11
	THRES_EN                :    [0x0, 0x1],            bits : 12
	POSTPROC_EN             :    [0x0, 0x1],            bits : 13
	INTEGRAL_EN             :    [0x0, 0x1],            bits : 14
	MAP_EN                  :    [0x0, 0x1],            bits : 15
    CANNY_OUT_SEL           :    [0x0, 0xF],            bits : 19_16
*/
#define FUNCTION_ENABLE_REGISTER_OFS 0x0010
REGDEF_BEGIN(FUNCTION_ENABLE_REGISTER)
REGDEF_BIT(IVE_MODE,                    8)
REGDEF_BIT(GEN_FILT_EN,                 1)
REGDEF_BIT(ORST_FILT_EN,                1)
REGDEF_BIT(EDGE_FILT_EN,                1)
REGDEF_BIT(NON_MAX_SUP_EN,              1)
REGDEF_BIT(THRES_EN,                    1)
REGDEF_BIT(POSTPROC_EN,                 1)
REGDEF_BIT(INTEGRAL_EN,                 1)
REGDEF_BIT(MAP_EN,                      1)
REGDEF_BIT(CANNY_OUT_SEL,               4)
REGDEF_BIT(,                           12)
REGDEF_END(FUNCTION_ENABLE_REGISTER)


/*
    DRAM_IN_SADDR0:    [0x0, 0xffffffff],         bits : 31_0
*/
#define DRAM_IN_REGISTER0_OFS 0x0014
REGDEF_BEGIN(DRAM_IN_REGISTER0)
REGDEF_BIT(DRAM_IN_SADDR0,        32)
REGDEF_END(DRAM_IN_REGISTER0)

/*
    DRAM_IN_SADDR0_MSB:    [0x0, 0xf],            bits : 3_0
*/
#define DRAM_IN_REGISTER1_OFS 0x0018
REGDEF_BEGIN(DRAM_IN_REGISTER1)
REGDEF_BIT(DRAM_IN_SADDR0_MSB,     4)
REGDEF_BIT(,                      28)
REGDEF_END(DRAM_IN_REGISTER1)

/*
    DRAM_IN_SADDR1:    [0x0, 0xfffffffc],         bits : 31_2
*/
#define DRAM_IN_REGISTER2_OFS 0x001C
REGDEF_BEGIN(DRAM_IN_REGISTER2)
REGDEF_BIT(,                      2)
REGDEF_BIT(DRAM_IN_SADDR1,        30)
REGDEF_END(DRAM_IN_REGISTER2)

/*
    DRAM_IN_SADDR1_MSB:    [0x0, 0xf],            bits : 3_0
*/
#define DRAM_IN_REGISTER3_OFS 0x0020
REGDEF_BEGIN(DRAM_IN_REGISTER3)
REGDEF_BIT(DRAM_IN_SADDR1_MSB,     4)
REGDEF_BIT(,                      28)
REGDEF_END(DRAM_IN_REGISTER3)


/*
    DRAM_IN_LOFST0:    [0x0, 0xfffff],            bits : 19_0
*/
#define DRAM_IN_LOFST0_REGISTER_OFS 0x0024
REGDEF_BEGIN(DRAM_IN_LOFST0_REGISTER)
REGDEF_BIT(DRAM_IN_LOFST0,        20)
REGDEF_BIT(,                      12)
REGDEF_END(DRAM_IN_LOFST0_REGISTER)


/*
    DRAM_IN_LOFST1:    [0x0, 0xffffc],            bits : 19_2
*/
#define DRAM_IN_LOFST1_REGISTER_OFS 0x0028
REGDEF_BEGIN(DRAM_IN_LOFST1_REGISTER)
REGDEF_BIT(,                       2)
REGDEF_BIT(DRAM_IN_LOFST1,        18)
REGDEF_BIT(,                      12)
REGDEF_END(DRAM_IN_LOFST1_REGISTER)

/*
    DRAM_OUT_SADDR0:    [0x0, 0xffffffff],         bits : 31_0
*/
#define DRAM_OUT_REGISTER0_OFS 0x002C
REGDEF_BEGIN(DRAM_OUT_REGISTER0)
REGDEF_BIT(DRAM_OUT_SADDR0,        32)
REGDEF_END(DRAM_OUT_REGISTER0)

/*
    DRAM_OUT_SADDR0_MSB:    [0x0, 0xf],            bits : 3_0
*/
#define DRAM_OUT_REGISTER1_OFS 0x0030
REGDEF_BEGIN(DRAM_OUT_REGISTER1)
REGDEF_BIT(DRAM_OUT_SADDR0_MSB,    4)
REGDEF_BIT(,                      28)
REGDEF_END(DRAM_OUT_REGISTER1)

/*
    DRAM_OUT_SADDR1:    [0x0, 0xffffffffc],        bits : 31_2
*/
#define DRAM_OUT_REGISTER2_OFS 0x0034
REGDEF_BEGIN(DRAM_OUT_REGISTER2)
REGDEF_BIT(,                      2)
REGDEF_BIT(DRAM_IN_SADDR2,        30)
REGDEF_END(DRAM_OUT_REGISTER2)

/*
    DRAM_OUT_SADDR1_MSB:    [0x0, 0xf],            bits : 3_0
*/
#define DRAM_OUT_REGISTER3_OFS 0x0038
REGDEF_BEGIN(DRAM_OUT_REGISTER3)
REGDEF_BIT(DRAM_IN_SADDR2_MSB,    4)
REGDEF_BIT(,                      28)
REGDEF_END(DRAM_OUT_REGISTER3)

/*
    DRAM_OUT_SADDR2:    [0x0, 0xffffffffc],        bits : 31_2
*/
#define DRAM_OUT_REGISTER4_OFS 0x003C
REGDEF_BEGIN(DRAM_OUT_REGISTER4)
REGDEF_BIT(,                        32)
REGDEF_END(DRAM_OUT_REGISTER4)

/*
    DRAM_OUT_SADDR2_MSB:    [0x0, 0xf],            bits : 3_0
*/
#define DRAM_OUT_REGISTER5_OFS 0x0040
REGDEF_BEGIN(DRAM_OUT_REGISTER5)
REGDEF_BIT(,                      32)
REGDEF_END(DRAM_OUT_REGISTER5)

/*
    DRAM_OUT_LOFST0       :    [0x0, 0xfffff],           bits : 19_0
*/
#define DRAM_OUT_LOFST0_REGISTER_OFS 0x0044
REGDEF_BEGIN(DRAM_OUT_LOFST0_REGISTER)
REGDEF_BIT(DRAM_OUT_LOFST0,        20)
REGDEF_BIT(,                      12)
REGDEF_END(DRAM_OUT_LOFST0_REGISTER)

/*
    DRAM_OUT_LOFST1       :    [0x0, 0xffffc],           bits : 19_2
*/
#define DRAM_OUT_LOFST1_REGISTER_OFS 0x0048
REGDEF_BEGIN(DRAM_OUT_LOFST1_REGISTER)
REGDEF_BIT(DRAM_OUT_LOFST2,        20)
REGDEF_BIT(,                      12)
REGDEF_END(DRAM_OUT_LOFST1_REGISTER)

/*
    DRAM_OUT_LOFST2       :    [0x0, 0xffffc],           bits : 19_2
*/
#define DRAM_OUT_LOFST2_REGISTER_OFS 0x004C
REGDEF_BEGIN(DRAM_OUT_LOFST2_REGISTER)
REGDEF_BIT(,                       32)
REGDEF_END(DRAM_OUT_LOFST2_REGISTER)

/*
    DRAM_IN_LLC          :    [0x0, 0xffffffff],         bits : 31_0
*/
#define DRAM_LLC_REGISTER0_OFS        0x0050
REGDEF_BEGIN(DRAM_LLC_REGISTER0)
REGDEF_BIT(DRAM_IN_LLC,          32)
REGDEF_END(DRAM_LLC_REGISTER0)

/*
    DRAM_IN_LLC_MSB      :    [0x0, 0xf],                bits : 3_0
*/
#define DRAM_LLC_REGISTER1_OFS        0x0054
REGDEF_BEGIN(DRAM_LLC_REGISTER1)
REGDEF_BIT(DRAM_IN_LLC_MSB,          4)
REGDEF_BIT(,                        28)
REGDEF_END(DRAM_LLC_REGISTER1)

/*
    IMG_WIDTH:    [0x0, 0x3fff],           bits : 13_0
    IMG_HEIGHT :  [0x0, 0x1fff],           bits : 28_16
*/
#define INPUT_IMG_SIZE_REGISTER_OFS 0x0058
REGDEF_BEGIN(INPUT_IMG_SIZE_REGISTER)
REGDEF_BIT(IMG_WIDTH,        14)
REGDEF_BIT(,        2)
REGDEF_BIT(IMG_HEIGHT,       13)
REGDEF_BIT(,        3)
REGDEF_END(INPUT_IMG_SIZE_REGISTER)

/*

*/
#define GENERAL_FILTER_REGISTER0_OFS 0x005C
REGDEF_BEGIN(GENERAL_FILTER_REGISTER0)
REGDEF_BIT(GEN_FILT_COEFF0,        8)
REGDEF_BIT(GEN_FILT_COEFF1,        8)
REGDEF_BIT(GEN_FILT_COEFF2,        8)
REGDEF_BIT(GEN_FILT_COEFF3,        8)
REGDEF_END(GENERAL_FILTER_REGISTER0)

/*

*/
#define GENERAL_FILTER_REGISTER1_OFS 0x0060
REGDEF_BEGIN(GENERAL_FILTER_REGISTER1)
REGDEF_BIT(GEN_FILT_COEFF4,        8)
REGDEF_BIT(GEN_FILT_COEFF5,        8)
REGDEF_BIT(GEN_FILT_COEFF6,        8)
REGDEF_BIT(GEN_FILT_COEFF7,        8)
REGDEF_END(GENERAL_FILTER_REGISTER1)

/*

*/
#define GENERAL_FILTER_REGISTER2_OFS 0x0064
REGDEF_BEGIN(GENERAL_FILTER_REGISTER2)
REGDEF_BIT(GEN_FILT_COEFF8,        8)
REGDEF_BIT(GEN_FILT_COEFF9,        8)
REGDEF_BIT(GEN_IN_FMT,             2)
REGDEF_BIT(,                       6)
REGDEF_BIT(GEN_IN_SYB_RATIO,       2)
REGDEF_BIT(,                       6)
REGDEF_END(GENERAL_FILTER_REGISTER2)

/*

*/
#define ORST_FILTER_REGISTER0_OFS   0x0068
REGDEF_BEGIN(ORST_FILTER_REGISTER0)
REGDEF_BIT(ORST_FILT_MODE,        2)
REGDEF_BIT(,                     30)
REGDEF_END(ORST_FILTER_REGISTER0)

/*

*/
#define EDGE_FILTER_REGISTER0_OFS   0x006C
REGDEF_BEGIN(EDGE_FILTER_REGISTER0)
REGDEF_BIT(EDGE_MODE,            2)
REGDEF_BIT(,                     2)
REGDEF_BIT(EDGE_ANGLE_SLP_FACT,  8)
REGDEF_BIT(EDGE_KERNEL_MODE,     1)
REGDEF_BIT(,                     3)
REGDEF_BIT(EDGE_SHIFT_BIT,       5)
REGDEF_BIT(,                     3)
REGDEF_BIT(EDGE_MAG_TH,          8)
REGDEF_END(EDGE_FILTER_REGISTER0)

/*

*/
#define EDGE_FILTER0_COEFF_REGISTER0_OFS 0x0070
REGDEF_BEGIN(EDGE_FILTER0_COEFF_REGISTER0)
REGDEF_BIT(EDGE_FILT0_COEFF0,        8)
REGDEF_BIT(EDGE_FILT0_COEFF1,        8)
REGDEF_BIT(EDGE_FILT0_COEFF2,        8)
REGDEF_BIT(EDGE_FILT0_COEFF3,        8)
REGDEF_END(EDGE_FILTER0_COEFF_REGISTER0)

/*

*/
#define EDGE_FILTER0_COEFF_REGISTER1_OFS 0x0074
REGDEF_BEGIN(EDGE_FILTER0_COEFF_REGISTER1)
REGDEF_BIT(EDGE_FILT0_COEFF4,        8)
REGDEF_BIT(EDGE_FILT0_COEFF5,        8)
REGDEF_BIT(EDGE_FILT0_COEFF6,        8)
REGDEF_BIT(EDGE_FILT0_COEFF7,        8)
REGDEF_END(EDGE_FILTER0_COEFF_REGISTER1)

/*

*/
#define EDGE_FILTER0_COEFF_REGISTER2_OFS 0x0078
REGDEF_BEGIN(EDGE_FILTER0_COEFF_REGISTER2)
REGDEF_BIT(EDGE_FILT0_COEFF8,        8)
REGDEF_BIT(EDGE_FILT0_COEFF9,        8)
REGDEF_BIT(EDGE_FILT0_COEFF10,       8)
REGDEF_BIT(EDGE_FILT0_COEFF11,       8)
REGDEF_END(EDGE_FILTER0_COEFF_REGISTER2)

/*

*/
#define EDGE_FILTER0_COEFF_REGISTER3_OFS 0x007C
REGDEF_BEGIN(EDGE_FILTER0_COEFF_REGISTER3)
REGDEF_BIT(EDGE_FILT0_COEFF12,       8)
REGDEF_BIT(EDGE_FILT0_COEFF13,       8)
REGDEF_BIT(EDGE_FILT0_COEFF14,       8)
REGDEF_BIT(EDGE_FILT0_COEFF15,       8)
REGDEF_END(EDGE_FILTER0_COEFF_REGISTER3)

/*

*/
#define EDGE_FILTER0_COEFF_REGISTER4_OFS 0x0080
REGDEF_BEGIN(EDGE_FILTER0_COEFF_REGISTER4)
REGDEF_BIT(EDGE_FILT0_COEFF16,       8)
REGDEF_BIT(EDGE_FILT0_COEFF17,       8)
REGDEF_BIT(EDGE_FILT0_COEFF18,       8)
REGDEF_BIT(EDGE_FILT0_COEFF19,       8)
REGDEF_END(EDGE_FILTER0_COEFF_REGISTER4)

/*

*/
#define EDGE_FILTER0_COEFF_REGISTER5_OFS 0x0084
REGDEF_BEGIN(EDGE_FILTER0_COEFF_REGISTER5)
REGDEF_BIT(EDGE_FILT0_COEFF20,       8)
REGDEF_BIT(EDGE_FILT0_COEFF21,       8)
REGDEF_BIT(EDGE_FILT0_COEFF22,       8)
REGDEF_BIT(EDGE_FILT0_COEFF23,       8)
REGDEF_END(EDGE_FILTER0_COEFF_REGISTER5)

/*

*/
#define EDGE_FILTER0_COEFF_REGISTER6_OFS 0x0088
REGDEF_BEGIN(EDGE_FILTER0_COEFF_REGISTER6)
REGDEF_BIT(EDGE_FILT0_COEFF24,       8)
REGDEF_BIT(,                        24)
REGDEF_END(EDGE_FILTER0_COEFF_REGISTER6)

/*

*/
#define EDGE_FILTER1_COEFF_REGISTER0_OFS 0x008C
REGDEF_BEGIN(EDGE_FILTER1_COEFF_REGISTER0)
REGDEF_BIT(EDGE_FILT1_COEFF0,        8)
REGDEF_BIT(EDGE_FILT1_COEFF1,        8)
REGDEF_BIT(EDGE_FILT1_COEFF2,        8)
REGDEF_BIT(EDGE_FILT1_COEFF3,        8)
REGDEF_END(EDGE_FILTER1_COEFF_REGISTER0)

/*

*/
#define EDGE_FILTER1_COEFF_REGISTER1_OFS 0x0090
REGDEF_BEGIN(EDGE_FILTER1_COEFF_REGISTER1)
REGDEF_BIT(EDGE_FILT1_COEFF4,        8)
REGDEF_BIT(EDGE_FILT1_COEFF5,        8)
REGDEF_BIT(EDGE_FILT1_COEFF6,        8)
REGDEF_BIT(EDGE_FILT1_COEFF7,        8)
REGDEF_END(EDGE_FILTER1_COEFF_REGISTER1)

/*

*/
#define EDGE_FILTER1_COEFF_REGISTER2_OFS 0x0094
REGDEF_BEGIN(EDGE_FILTER1_COEFF_REGISTER2)
REGDEF_BIT(EDGE_FILT1_COEFF8,        8)
REGDEF_BIT(EDGE_FILT1_COEFF9,        8)
REGDEF_BIT(EDGE_FILT1_COEFF10,       8)
REGDEF_BIT(EDGE_FILT1_COEFF11,       8)
REGDEF_END(EDGE_FILTER1_COEFF_REGISTER2)

/*

*/
#define EDGE_FILTER1_COEFF_REGISTER3_OFS 0x0098
REGDEF_BEGIN(EDGE_FILTER1_COEFF_REGISTER3)
REGDEF_BIT(EDGE_FILT1_COEFF12,       8)
REGDEF_BIT(EDGE_FILT1_COEFF13,       8)
REGDEF_BIT(EDGE_FILT1_COEFF14,       8)
REGDEF_BIT(EDGE_FILT1_COEFF15,       8)
REGDEF_END(EDGE_FILTER1_COEFF_REGISTER3)

/*

*/
#define EDGE_FILTER1_COEFF_REGISTER4_OFS 0x009C
REGDEF_BEGIN(EDGE_FILTER1_COEFF_REGISTER4)
REGDEF_BIT(EDGE_FILT1_COEFF16,       8)
REGDEF_BIT(EDGE_FILT1_COEFF17,       8)
REGDEF_BIT(EDGE_FILT1_COEFF18,       8)
REGDEF_BIT(EDGE_FILT1_COEFF19,       8)
REGDEF_END(EDGE_FILTER1_COEFF_REGISTER4)

/*

*/
#define EDGE_FILTER1_COEFF_REGISTER5_OFS 0x00A0
REGDEF_BEGIN(EDGE_FILTER1_COEFF_REGISTER5)
REGDEF_BIT(EDGE_FILT1_COEFF20,       8)
REGDEF_BIT(EDGE_FILT1_COEFF21,       8)
REGDEF_BIT(EDGE_FILT1_COEFF22,       8)
REGDEF_BIT(EDGE_FILT1_COEFF23,       8)
REGDEF_END(EDGE_FILTER1_COEFF_REGISTER5)

/*

*/
#define EDGE_FILTER1_COEFF_REGISTER6_OFS 0x00A4
REGDEF_BEGIN(EDGE_FILTER1_COEFF_REGISTER6)
REGDEF_BIT(EDGE_FILT1_COEFF24,               8)
REGDEF_BIT(,                                 8)
REGDEF_BIT(EDGE_ALPHA_BLENDING_FACTOR,       8)
REGDEF_BIT(,                                 8)
REGDEF_END(EDGE_FILTER1_COEFF_REGISTER6)

/*

*/
#define THRES_REGISTER0_OFS              0x00A8
REGDEF_BEGIN(THRES_REGISTER0)
REGDEF_BIT(THRES_MODE,                       6)
REGDEF_BIT(,                                26)
REGDEF_END(THRES_REGISTER0)

/*

*/
#define THRES_REGISTER1_OFS              0x00AC
REGDEF_BEGIN(THRES_REGISTER1)
REGDEF_BIT(THRES_LOW_TH,                    16)
REGDEF_BIT(,                                16)
REGDEF_END(THRES_REGISTER1)

/*

*/
#define THRES_REGISTER2_OFS              0x00B0
REGDEF_BEGIN(THRES_REGISTER2)
REGDEF_BIT(THRES_HIGH_TH,                   16)
REGDEF_BIT(,                                16)
REGDEF_END(THRES_REGISTER2)

/*

*/
#define THRES_REGISTER3_OFS              0x00B4
REGDEF_BEGIN(THRES_REGISTER3)
REGDEF_BIT(THRES_OUT_MIN_VAL,                8)
REGDEF_BIT(,                                24)
REGDEF_END(THRES_REGISTER3)

/*

*/
#define THRES_REGISTER4_OFS              0x00B8
REGDEF_BEGIN(THRES_REGISTER4)
REGDEF_BIT(THRES_OUT_MID_VAL,                8)
REGDEF_BIT(,                                24)
REGDEF_END(THRES_REGISTER4)

/*

*/
#define THRES_REGISTER5_OFS              0x00BC
REGDEF_BEGIN(THRES_REGISTER5)
REGDEF_BIT(THRES_OUT_MAX_VAL,                8)
REGDEF_BIT(,                                24)
REGDEF_END(THRES_REGISTER5)

/*

*/
#define POSTPROC_REGISTER0_OFS            0x00C0
REGDEF_BEGIN(POSTPROC_REGISTER0)
REGDEF_BIT(POSTPROC_MODE,                   1)
REGDEF_BIT(,                                3)
REGDEF_BIT(POSTPROC_MORPH_OP,               1)
REGDEF_BIT(,                                3)
REGDEF_BIT(POSTPROC_MORPH_MASK_EN,         24)
REGDEF_END(POSTPROC_REGISTER0)

/*

*/
#define POSTPROC_REGISTER1_OFS            0x00C4
REGDEF_BEGIN(POSTPROC_REGISTER1)
REGDEF_BIT(POSTPROC_HYST_LOW_TH,            8)
REGDEF_BIT(,                                8)
REGDEF_BIT(POSTPROC_HYST_HIGH_TH,           8)
REGDEF_BIT(,                                8)
REGDEF_END(POSTPROC_REGISTER1)

/*

*/
#define INTEGRAL_IMG_REGISTER0_OFS        0x00C8
REGDEF_BEGIN(INTEGRAL_IMG_REGISTER0)
REGDEF_BIT(INTEGRAL_IN_FMT,                 2)
REGDEF_BIT(,                                2)
REGDEF_BIT(INTEGRAL_OUT_FMT,                2)
REGDEF_BIT(,                                2)
REGDEF_BIT(,                               24)
REGDEF_END(INTEGRAL_IMG_REGISTER0)

/*

*/
#define CSC_REGISTER0_OFS                 0x00CC
REGDEF_BEGIN(CSC_REGISTER0)
REGDEF_BIT(Reserved,        32)
REGDEF_END(CSC_REGISTER0)

/*

*/
#define CSC_COEF_REGISTER0_OFS            0x00D0
REGDEF_BEGIN(CSC_COEF_REGISTER0)
REGDEF_BIT(Reserved,        32)
REGDEF_END(CSC_COEF_REGISTER0)

/*

*/
#define CSC_COEF_REGISTER1_OFS            0x00D4
REGDEF_BEGIN(CSC_COEF_REGISTER1)
REGDEF_BIT(Reserved,        32)
REGDEF_END(CSC_COEF_REGISTER1)

/*

*/
#define CSC_COEF_REGISTER2_OFS            0x00D8
REGDEF_BEGIN(CSC_COEF_REGISTER2)
REGDEF_BIT(Reserved,        32)
REGDEF_END(CSC_COEF_REGISTER2)

/*

*/
#define CSC_COEF_REGISTER3_OFS            0x00DC
REGDEF_BEGIN(CSC_COEF_REGISTER3)
REGDEF_BIT(Reserved,        32)
REGDEF_END(CSC_COEF_REGISTER3)

/*

*/
#define CSC_COEF_REGISTER4_OFS            0x00E0
REGDEF_BEGIN(CSC_COEF_REGISTER4)
REGDEF_BIT(Reserved,        32)
REGDEF_END(CSC_COEF_REGISTER4)

/*

*/
#define CSC_COEF_REGISTER5_OFS            0x00E4
REGDEF_BEGIN(CSC_COEF_REGISTER5)
REGDEF_BIT(Reserved,        32)
REGDEF_END(CSC_COEF_REGISTER5)

/*

*/
#define CSC_COEF_REGISTER6_OFS            0x00E8
REGDEF_BEGIN(CSC_COEF_REGISTER6)
REGDEF_BIT(Reserved,        32)
REGDEF_END(CSC_COEF_REGISTER6)

/*

*/
#define CSC_COEF_REGISTER7_OFS            0x00EC
REGDEF_BEGIN(CSC_COEF_REGISTER7)
REGDEF_BIT(Reserved,        32)
REGDEF_END(CSC_COEF_REGISTER7)

/*

*/
#define CSC_COEF_REGISTER8_OFS            0x00F0
REGDEF_BEGIN(CSC_COEF_REGISTER8)
REGDEF_BIT(Reserved,        32)
REGDEF_END(CSC_COEF_REGISTER8)

/*

*/
#define CSC_BIAS_REGISTER0_OFS            0x00F4
REGDEF_BEGIN(CSC_BIAS_REGISTER0)
REGDEF_BIT(Reserved,        32)
REGDEF_END(CSC_BIAS_REGISTER0)

/*

*/
#define CSC_BIAS_REGISTER1_OFS            0x00F8
REGDEF_BEGIN(CSC_BIAS_REGISTER1)
REGDEF_BIT(Reserved,        32)
REGDEF_END(CSC_BIAS_REGISTER1)

/*

*/
#define CSC_BIAS_REGISTER2_OFS            0x00FC
REGDEF_BEGIN(CSC_BIAS_REGISTER2)
REGDEF_BIT(Reserved,        32)
REGDEF_END(CSC_BIAS_REGISTER2)

/*

*/
#define AXI_REGISTER0_OFS        0x0100
REGDEF_BEGIN(AXI_REGISTER0)
REGDEF_BIT(IN0_CH_EN,        1)
REGDEF_BIT(IN1_CH_EN,        1)
REGDEF_BIT(OUT0_CH_EN,        1)
REGDEF_BIT(,        5)
REGDEF_BIT(IN0_LOCK_DIS,        1)
REGDEF_BIT(IN1_LOCK_DIS,        1)
REGDEF_BIT(OUT0_LOCK_DIS,        1)
REGDEF_BIT(,       5)
REGDEF_BIT(WRITE_CH_OUTSTANDING_NUM,        8)
REGDEF_BIT(READ_CH_OUTSTANDING_NUM,        8)
REGDEF_END(AXI_REGISTER0)

/*

*/
#define AXI_REGISTER1_OFS        0x0104
REGDEF_BEGIN(AXI_REGISTER1)
REGDEF_BIT(AXI_BUS_DISABLE,        1)
REGDEF_BIT(,        26)
REGDEF_BIT(AXI_BUS_IDLE,        1)
REGDEF_BIT(,        4)
REGDEF_END(AXI_REGISTER1)

/*

*/
#define IVE_RESERVED_REGISTER2_OFS        0x0108
REGDEF_BEGIN(IVE_RESERVED_REGISTER2)
REGDEF_BIT(Reserved,        32)
REGDEF_END(IVE_RESERVED_REGISTER2)

/*

*/
#define IVE_RESERVED_REGISTER3_OFS        0x010C
REGDEF_BEGIN(IVE_RESERVED_REGISTER3)
REGDEF_BIT(Reserved,        32)
REGDEF_END(IVE_RESERVED_REGISTER3)

/*

*/
#define IVE_RESERVED_REGISTER4_OFS        0x0110
REGDEF_BEGIN(IVE_RESERVED_REGISTER4)
REGDEF_BIT(Reserved,        32)
REGDEF_END(IVE_RESERVED_REGISTER4)

/*

*/
#define IVE_RESERVED_REGISTER5_OFS        0x0114
REGDEF_BEGIN(IVE_RESERVED_REGISTER5)
REGDEF_BIT(Reserved,        32)
REGDEF_END(IVE_RESERVED_REGISTER5)

/*

*/
#define IVE_RESERVED_REGISTER6_OFS        0x0118
REGDEF_BEGIN(IVE_RESERVED_REGISTER6)
REGDEF_BIT(Reserved,        32)
REGDEF_END(IVE_RESERVED_REGISTER6)

/*

*/
#define IVE_RESERVED_REGISTER7_OFS        0x011C
REGDEF_BEGIN(IVE_RESERVED_REGISTER7)
REGDEF_BIT(Reserved,        32)
REGDEF_END(IVE_RESERVED_REGISTER7)

/*

*/
#define IVE_RESERVED_REGISTER8_OFS        0x0120
REGDEF_BEGIN(IVE_RESERVED_REGISTER8)
REGDEF_BIT(Reserved,        32)
REGDEF_END(IVE_RESERVED_REGISTER8)

/*

*/
#define IVE_RESERVED_REGISTER9_OFS        0x0124
REGDEF_BEGIN(IVE_RESERVED_REGISTER9)
REGDEF_BIT(Reserved,        32)
REGDEF_END(IVE_RESERVED_REGISTER9)

/*

*/
#define IVE_RESERVED_REGISTER10_OFS       0x0128
REGDEF_BEGIN(IVE_RESERVED_REGISTER10)
REGDEF_BIT(Reserved,        32)
REGDEF_END(IVE_RESERVED_REGISTER10)

/*

*/
#define IVE_RESERVED_REGISTER11_OFS       0x012C
REGDEF_BEGIN(IVE_RESERVED_REGISTER11)
REGDEF_BIT(Reserved,        32)
REGDEF_END(IVE_RESERVED_REGISTER11)

/*

*/
#define IVE_RESERVED_REGISTER12_OFS       0x0130
REGDEF_BEGIN(IVE_RESERVED_REGISTER12)
REGDEF_BIT(Reserved,        32)
REGDEF_END(IVE_RESERVED_REGISTER12)

/*

*/
#define IVE_RESERVED_REGISTER13_OFS       0x0134
REGDEF_BEGIN(IVE_RESERVED_REGISTER13)
REGDEF_BIT(Reserved,        32)
REGDEF_END(IVE_RESERVED_REGISTER13)

/*

*/
#define IVE_RESERVED_REGISTER14_OFS       0x0138
REGDEF_BEGIN(IVE_RESERVED_REGISTER14)
REGDEF_BIT(Reserved,        32)
REGDEF_END(IVE_RESERVED_REGISTER14)

/*

*/
#define IVE_RESERVED_REGISTER15_OFS       0x013C
REGDEF_BEGIN(IVE_RESERVED_REGISTER15)
REGDEF_BIT(Reserved,        32)
REGDEF_END(IVE_RESERVED_REGISTER15)

/*

*/
#define IVE_RESERVED_REGISTER16_OFS       0x0140
REGDEF_BEGIN(IVE_RESERVED_REGISTER16)
REGDEF_BIT(Reserved,        32)
REGDEF_END(IVE_RESERVED_REGISTER16)

/*

*/
#define IVE_RESERVED_REGISTER17_OFS       0x0144
REGDEF_BEGIN(IVE_RESERVED_REGISTER17)
REGDEF_BIT(Reserved,        32)
REGDEF_END(IVE_RESERVED_REGISTER17)

/*

*/
#define IVE_RESERVED_REGISTER18_OFS       0x0148
REGDEF_BEGIN(IVE_RESERVED_REGISTER18)
REGDEF_BIT(Reserved,        32)
REGDEF_END(IVE_RESERVED_REGISTER18)

/*

*/
#define IVE_RESERVED_REGISTER19_OFS       0x014C
REGDEF_BEGIN(IVE_RESERVED_REGISTER19)
REGDEF_BIT(Reserved,        32)
REGDEF_END(IVE_RESERVED_REGISTER19)

/*

*/
#define IVE_RESERVED_REGISTER20_OFS       0x0150
REGDEF_BEGIN(IVE_RESERVED_REGISTER20)
REGDEF_BIT(Reserved,        32)
REGDEF_END(IVE_RESERVED_REGISTER20)

/*

*/
#define IVE_RESERVED_REGISTER21_OFS       0x0154
REGDEF_BEGIN(IVE_RESERVED_REGISTER21)
REGDEF_BIT(Reserved,        32)
REGDEF_END(IVE_RESERVED_REGISTER21)

/*

*/
#define IVE_RESERVED_REGISTER22_OFS       0x0158
REGDEF_BEGIN(IVE_RESERVED_REGISTER22)
REGDEF_BIT(Reserved,        32)
REGDEF_END(IVE_RESERVED_REGISTER22)

/*

*/
#define IVE_RESERVED_REGISTER23_OFS       0x015C
REGDEF_BEGIN(IVE_RESERVED_REGISTER23)
REGDEF_BIT(Reserved,        32)
REGDEF_END(IVE_RESERVED_REGISTER23)

/*

*/
#define IVE_HIST_REGISTER0_OFS            0x0160
REGDEF_BEGIN(IVE_HIST_REGISTER0)
REGDEF_BIT(HIST_MODE,       2)
REGDEF_BIT(,               30)
REGDEF_END(IVE_HIST_REGISTER0)

/*

*/
#define IVE_HIST_REGISTER1_OFS            0x0164
REGDEF_BEGIN(IVE_HIST_REGISTER1)
REGDEF_BIT(HIST_CDF_MIN,       32)
REGDEF_END(IVE_HIST_REGISTER1)

/*

*/
#define NCC_OUTPUT_REGISTER0_OFS          0x0168
REGDEF_BEGIN(NCC_OUTPUT_REGISTER0)
REGDEF_BIT(NCC_NUMERATOR_LSB,        32)
REGDEF_END(NCC_OUTPUT_REGISTER0)

/*

*/
#define NCC_OUTPUT_REGISTER1_OFS          0x016C
REGDEF_BEGIN(NCC_OUTPUT_REGISTER1)
REGDEF_BIT(NCC_NUMERATOR_MSB,        32)
REGDEF_END(NCC_OUTPUT_REGISTER1)

/*

*/
#define NCC_OUTPUT_REGISTER2_OFS          0x0170
REGDEF_BEGIN(NCC_OUTPUT_REGISTER2)
REGDEF_BIT(NCC_QUAD_SUM0_LSB,        32)
REGDEF_END(NCC_OUTPUT_REGISTER2)

/*

*/
#define NCC_OUTPUT_REGISTER3_OFS          0x0174
REGDEF_BEGIN(NCC_OUTPUT_REGISTER3)
REGDEF_BIT(NCC_QUAD_SUM0_MSB,        32)
REGDEF_END(NCC_OUTPUT_REGISTER3)

/*

*/
#define NCC_OUTPUT_REGISTER4_OFS          0x0178
REGDEF_BEGIN(NCC_OUTPUT_REGISTER4)
REGDEF_BIT(NCC_QUAD_SUM1_LSB,        32)
REGDEF_END(NCC_OUTPUT_REGISTER4)

/*

*/
#define NCC_OUTPUT_REGISTER5_OFS          0x017C
REGDEF_BEGIN(NCC_OUTPUT_REGISTER5)
REGDEF_BIT(NCC_QUAD_SUM1_MSB,        32)
REGDEF_END(NCC_OUTPUT_REGISTER5)

/*

*/
#define DMA_COPY_REGISTER0_OFS            0x0180
REGDEF_BEGIN(DMA_COPY_REGISTER0)
REGDEF_BIT(DMA_SOBEL_OUT_SEL,          1)
REGDEF_BIT(DMA_IN_FMT,                2)
REGDEF_BIT(,                          1)
REGDEF_BIT(DMA_HOR_SEG_SZ,            5)
REGDEF_BIT(,                          7)
REGDEF_BIT(DMA_VER_SEG_SZ,           13)
REGDEF_BIT(,                          3)
REGDEF_END(DMA_COPY_REGISTER0)

/*

*/
#define DMA_COPY_REGISTER1_OFS            0x0184
REGDEF_BEGIN(DMA_COPY_REGISTER1)
REGDEF_BIT(DMA_ELEMENT_SZ,            5)
REGDEF_BIT(,                         11)
REGDEF_BIT(DMA_VER_SEG_NUM,          13)
REGDEF_BIT(,                          3)
REGDEF_END(DMA_COPY_REGISTER1)

/*

*/
#define IMGOP_REGISTER0_OFS               0x0188
REGDEF_BEGIN(IMGOP_REGISTER0)
REGDEF_BIT(IMGOP_MODE,                3)
REGDEF_BIT(,                          1)
REGDEF_BIT(IMGOP_SUB_MODE,            1)
REGDEF_BIT(,                         27)
REGDEF_END(IMGOP_REGISTER0)

/*

*/
#define BIT16_TO_BIT8_REGISTER0_OFS       0x018C
REGDEF_BEGIN(BIT16_TO_BIT8_REGISTER0)
REGDEF_BIT(BIT16_TO_BIT8_MODE,        3)
REGDEF_BIT(,                         29)
REGDEF_END(BIT16_TO_BIT8_REGISTER0)

/*

*/
#define OPERATION_REGISTER0_OFS           0x0190
REGDEF_BEGIN(OPERATION_REGISTER0)
REGDEF_BIT(OP_COEF_A,                32)
REGDEF_END(OPERATION_REGISTER0)

/*

*/
#define OPERATION_REGISTER1_OFS           0x0194
REGDEF_BEGIN(OPERATION_REGISTER1)
REGDEF_BIT(OP_COEF_B,                32)
REGDEF_END(OPERATION_REGISTER1)

/*

*/
#define OPERATION_REGISTER2_OFS           0x0198
REGDEF_BEGIN(OPERATION_REGISTER2)
REGDEF_BIT(OP_COEF_BIAS,              8)
REGDEF_BIT(,                         24)
REGDEF_END(OPERATION_REGISTER2)

/*

*/
#define OPERATION_REGISTER3_OFS           0x019C
REGDEF_BEGIN(OPERATION_REGISTER3)
REGDEF_BIT(OP_SHIFT_BIT,              5)
REGDEF_BIT(,                         27)
REGDEF_END(OPERATION_REGISTER3)

/*

*/
#define CCL_REGISTER0_OFS                 0x01A0
REGDEF_BEGIN(CCL_REGISTER0)
REGDEF_BIT(Reserved,        32)
REGDEF_END(CCL_REGISTER0)

/*

*/
#define CCL_REGISTER1_OFS                 0x01A4
REGDEF_BEGIN(CCL_REGISTER1)
REGDEF_BIT(Reserved,        32)
REGDEF_END(CCL_REGISTER1)

/*

*/
#define ST_CORNER_REGISTER0_OFS           0x01A8
REGDEF_BEGIN(ST_CORNER_REGISTER0)
REGDEF_BIT(ST_MODE,                   1)
REGDEF_BIT(,                 		  3)
REGDEF_BIT(ST_THRESHOLD_MODE,  		  1)
REGDEF_BIT(,                          27)
REGDEF_END(ST_CORNER_REGISTER0)

/*

*/
#define ST_CORNER_REGISTER1_OFS           0x01AC
REGDEF_BEGIN(ST_CORNER_REGISTER1)
REGDEF_BIT(ST_QUALITY_LEVEL,          8)
REGDEF_BIT(ST_MIN_DIST,               8)
REGDEF_BIT(ST_BLK_DIST,               8)
REGDEF_BIT(,                          8)
REGDEF_END(ST_CORNER_REGISTER1)

/*

*/
#define ST_CORNER_REGISTER2_OFS           0x01B0
REGDEF_BEGIN(ST_CORNER_REGISTER2)
REGDEF_BIT(ST_STRIPE_BLK_X_NUM,      10)
REGDEF_BIT(,                         22)
REGDEF_END(ST_CORNER_REGISTER2)

/*

*/
#define ST_CORNER_REGISTER3_OFS           0x01B4
REGDEF_BEGIN(ST_CORNER_REGISTER3)
REGDEF_BIT(ST_BLK_X_NUM,             10)
REGDEF_BIT(,                          6)
REGDEF_BIT(ST_BLK_Y_NUM,             10)
REGDEF_BIT(,                          6)
REGDEF_END(ST_CORNER_REGISTER3)

/*

*/
#define ST_CORNER_REGISTER4_OFS           0x01B8
REGDEF_BEGIN(ST_CORNER_REGISTER4)
REGDEF_BIT(ST_FIRST_STAGE_POINT_NUM, 20)
REGDEF_BIT(,                         12)
REGDEF_END(ST_CORNER_REGISTER4)

/*

*/
#define ST_CORNER_REGISTER5_OFS           0x01BC
REGDEF_BEGIN(ST_CORNER_REGISTER5)
REGDEF_BIT(ST_THRESHOLD_VAL, 		24)
REGDEF_BIT(Reserved,        		 8)
REGDEF_END(ST_CORNER_REGISTER5)

/*

*/
#define ST_CORNER_REGISTER6_OFS           0x01C0
REGDEF_BEGIN(ST_CORNER_REGISTER6)
REGDEF_BIT(Reserved,        32)
REGDEF_END(ST_CORNER_REGISTER6)

/*

*/
#define LBP_REGISTER0_OFS                 0x01C4
REGDEF_BEGIN(LBP_REGISTER0)
REGDEF_BIT(LBP_CMP_MODE,              1)
REGDEF_BIT(,                         15)
REGDEF_BIT(LBP_THRESHOLD,             8)
REGDEF_BIT(,                          8)
REGDEF_END(LBP_REGISTER0)

/*

*/
#define VA_REGISTER0_OFS                  0x01C8
REGDEF_BEGIN(VA_REGISTER0)
REGDEF_BIT(,                         32)
REGDEF_END(VA_REGISTER0)

/*

*/
#define MAP_REGISTER0_OFS                 0x01CC
REGDEF_BEGIN(MAP_REGISTER0)
REGDEF_BIT(MAP_MODE,                  1)
REGDEF_BIT(,                          7)
REGDEF_BIT(MAP_INDEX_SHIFT,           4)
REGDEF_BIT(,                         20)
REGDEF_END(MAP_REGISTER0)


typedef struct {
	union {
		struct {
			unsigned ive_rst           : 1;     // bits : 0
			unsigned ive_start         : 1;     // bits : 1
			unsigned                   : 26;
			unsigned ll_fire           : 1;     // bits : 28
			unsigned                   : 3;
		} bit;
		UINT32 word;
	} reg_0; // 0x0000

	union {
		struct {
			unsigned inte_frm_end              : 1;       // bits : 0
			unsigned                           : 7;		  
			unsigned inte_ll_end               : 1;       // bits : 8
			unsigned inte_ll_error             : 1;       // bits : 9
			unsigned inte_ll_job_end           : 1;       // bits : 10
			unsigned                           : 5;
			unsigned                           : 16;
		} bit;
		UINT32 word;
	} reg_1; // 0x0004

	union {
		struct {
			unsigned ints_frm_end              : 1;       // bits : 0
			unsigned                           : 7;		
			unsigned ints_ll_end               : 1;       // bits : 8
			unsigned ints_ll_error             : 1;       // bits : 9
			unsigned ints_ll_job_end           : 1;       // bits : 10
			unsigned                           : 5;
			unsigned                           : 16;
		} bit;
		UINT32 word;
	} reg_2; // 0x0008

	union {
		struct {
			unsigned ll_terminate        : 1;       // bits : 0
			unsigned                     : 31;
		} bit;
		UINT32 word;
	} reg_3; // 0x000c

	union {
		struct {
			unsigned ive_mode         : 8;			// bits : 7_0
			unsigned gen_filt_en      : 1;			// bits : 8
			unsigned orst_filt_en     : 1;			// bits : 9
			unsigned edge_filt_en     : 1;			// bits : 10
			unsigned non_max_sup_en   : 1;			// bits : 11
			unsigned thres_en         : 1;			// bits : 12
			unsigned postproc_en      : 1;			// bits : 13
			unsigned integral_en      : 1;     	 	// bits : 14
			unsigned map_en           : 1;     		// bits : 15
			unsigned canny_out_sel    : 4;			// bits : 19_16
			unsigned                  : 12;
		} bit;
		UINT32 word;
	} reg_4; // 0x0010

	union {
		struct {
			unsigned dram_in_saddr0   : 32;      	// bits : 31_0
		} bit;
		UINT32 word;
	} reg_5; // 0x0014

	union {
		struct {
			unsigned dram_in_saddr0_msb : 4;		// bits : 3_0
			unsigned                    : 28;
		} bit;
		UINT32 word;
	} reg_6; // 0x0018

	union {
		struct {
			unsigned                   : 2;
			unsigned dram_in_saddr1    : 30;		// bits : 31_2
		} bit;
		UINT32 word;
	} reg_7; // 0x001c

	union {
		struct {
			unsigned dram_in_saddr1_msb   : 4;    	// bits : 3_0
			unsigned                      : 28;
		} bit;
		UINT32 word;
	} reg_8; // 0x0020

	union {
		struct {
			unsigned dram_in_lofs0    : 20;			// bits : 19_0
			unsigned                  : 12;
		} bit;
		UINT32 word;
	} reg_9; // 0x0024

	union {
		struct {
			unsigned dram_in_lofs1    : 20;			// bits : 19_2
			unsigned                  : 12;
		} bit;
		UINT32 word;
	} reg_10; // 0x0028

	union {
		struct {
			unsigned dram_out_saddr0  : 32;			// bits : 31_0
		} bit;
		UINT32 word;
	} reg_11; // 0x002c

	union {
		struct {
			unsigned dram_out_saddr0_msb : 4;		// bits : 3_0
			unsigned                   : 28;
		} bit;
		UINT32 word;
	} reg_12; // 0x0030

	union {
		struct {
			unsigned                   : 2;
			unsigned dram_in_saddr2    : 30;		// bits : 31_2		
		} bit;
		UINT32 word;
	} reg_13; // 0x0034

	union {
		struct {
			unsigned dram_in_saddr2_msb   : 4;    	// bits : 3_0
			unsigned                      : 28;
		} bit;
		UINT32 word;
	} reg_14; // 0x0038

	union {
		struct {			
			unsigned 					  : 32;			// bits : 31_2
		} bit;
		UINT32 word;
	} reg_15; // 0x003c

	union {
		struct {			
			unsigned                     : 32;
		} bit;
		UINT32 word;
	} reg_16; // 0x0040

	union {
		struct {
			unsigned dram_out_lofs0         : 20;	// bits : 19_0
			unsigned                        : 12;
		} bit;
		UINT32 word;
	} reg_17; // 0x0044

	union {
		struct {
			unsigned dram_in_lofs2         : 20;	// bits : 19_0
			unsigned                        : 12;	
		} bit;
		UINT32 word;
	} reg_18; // 0x0048

	union {
		struct {
			unsigned                        : 32;			
		} bit;
		UINT32 word;
	} reg_19; // 0x004c

	union {
		struct {
			unsigned dram_in_llc        : 32;		// bits : 31_0
		} bit;
		UINT32 word;
	} reg_20; // 0x0050

	union {
		struct {
			unsigned dram_in_llc_msb     : 4;		// bits : 3_0
			unsigned                     : 28;
		} bit;
		UINT32 word;
	} reg_21; // 0x0054

	union {
		struct {
			unsigned img_width        	: 14;		// bits : 13_0
			unsigned                  	: 2;
			unsigned img_height       	: 13;		// bits : 28_16
			unsigned                  	: 3;
		} bit;
		UINT32 word;
	} reg_22; // 0x0058

	union {
		struct {
			unsigned gen_filt_coeff0        : 8;		// bits : 7_0
			unsigned gen_filt_coeff1        : 8;		// bits : 15_8
			unsigned gen_filt_coeff2        : 8;		// bits : 23_16
			unsigned gen_filt_coeff3        : 8;		// bits : 31_24

		} bit;
		UINT32 word;
	} reg_23; // 0x005c

	union {
		struct {
			unsigned gen_filt_coeff4        : 8;		// bits : 7_0
			unsigned gen_filt_coeff5        : 8;		// bits : 15_8
			unsigned gen_filt_coeff6        : 8;		// bits : 23_16
			unsigned gen_filt_coeff7        : 8;		// bits : 31_24
		} bit;
		UINT32 word;
	} reg_24; // 0x0060

	union {
		struct {
			unsigned gen_filt_coeff8        : 8;		// bits : 7_0
			unsigned gen_filt_coeff9        : 8;		// bits : 15_8
			unsigned gen_in_fmt             : 2;		// bits : 17_16
			unsigned                        : 6;
			unsigned gen_in_sub_ratio       : 2;		// bits : 25_24
			unsigned                        : 6;
		} bit;
		UINT32 word;
	} reg_25; // 0x0064

	union {
		struct {
			unsigned orst_filt_mode       : 2;		// bits : 1_0
			unsigned                      : 30;		// bits : 31_2
		} bit;
		UINT32 word;
	} reg_26; // 0x0068

	union {
		struct {
			unsigned edge_mode            : 2;		// bits : 1_0
			unsigned                      : 2;
			unsigned edge_angle_slp_fact  : 8;		// bits : 11_4
			unsigned edge_kernel_mode     : 1;		// bits : 12
			unsigned                      : 3;
			unsigned edge_shift_bit       : 5;		// bits : 20_16
			unsigned                      : 3;
			unsigned edge_mag_th          : 8;		// bits : 31_24
		} bit;
		UINT32 word;
	} reg_27; // 0x006c

	union {
		struct {
			unsigned edge_filt0_coeff0      : 8;		// bits : 7_0
			unsigned edge_filt0_coeff1      : 8;		// bits : 15_8
			unsigned edge_filt0_coeff2      : 8;		// bits : 23_16
			unsigned edge_filt0_coeff3      : 8;		// bits : 31_24
		} bit;
		UINT32 word;
	} reg_28; // 0x0070

	union {
		struct {
			unsigned edge_filt0_coeff4      : 8;		// bits : 7_0
			unsigned edge_filt0_coeff5      : 8;		// bits : 15_8
			unsigned edge_filt0_coeff6      : 8;		// bits : 23_16
			unsigned edge_filt0_coeff7      : 8;		// bits : 31_24
		} bit;
		UINT32 word;
	} reg_29; // 0x0074

	union {
		struct {
			unsigned edge_filt0_coeff8      : 8;		// bits : 7_0
			unsigned edge_filt0_coeff9      : 8;		// bits : 15_8
			unsigned edge_filt0_coeff10     : 8;		// bits : 23_16
			unsigned edge_filt0_coeff11     : 8;		// bits : 31_24
		} bit;
		UINT32 word;
	} reg_30; // 0x0078

	union {
		struct {
			unsigned edge_filt0_coeff12      : 8;		// bits : 7_0
			unsigned edge_filt0_coeff13      : 8;		// bits : 15_8
			unsigned edge_filt0_coeff14      : 8;		// bits : 23_16
			unsigned edge_filt0_coeff15      : 8;		// bits : 31_24
		} bit;
		UINT32 word;
	} reg_31; // 0x007c

	union {
		struct {
			unsigned edge_filt0_coeff16      : 8;		// bits : 7_0
			unsigned edge_filt0_coeff17      : 8;		// bits : 15_8
			unsigned edge_filt0_coeff18      : 8;		// bits : 23_16
			unsigned edge_filt0_coeff19      : 8;		// bits : 31_24
		} bit;
		UINT32 word;
	} reg_32; // 0x0080

	union {
		struct {
			unsigned edge_filt0_coeff20      : 8;		// bits : 7_0
			unsigned edge_filt0_coeff21      : 8;		// bits : 15_8
			unsigned edge_filt0_coeff22      : 8;		// bits : 23_16
			unsigned edge_filt0_coeff23      : 8;		// bits : 31_24
		} bit;
		UINT32 word;
	} reg_33; // 0x0084

	union {
		struct {
			unsigned edge_filt0_coeff24      : 8;		// bits : 7_0
			unsigned                         : 24;
		} bit;
		UINT32 word;
	} reg_34; // 0x0088

	union {
		struct {
			unsigned edge_filt1_coeff0      : 8;		// bits : 7_0
			unsigned edge_filt1_coeff1      : 8;		// bits : 15_8
			unsigned edge_filt1_coeff2      : 8;		// bits : 23_16
			unsigned edge_filt1_coeff3      : 8;		// bits : 31_24
		} bit;
		UINT32 word;
	} reg_35; // 0x008c

	union {
		struct {
			unsigned edge_filt1_coeff4      : 8;		// bits : 7_0
			unsigned edge_filt1_coeff5      : 8;		// bits : 15_8
			unsigned edge_filt1_coeff6      : 8;		// bits : 23_16
			unsigned edge_filt1_coeff7      : 8;		// bits : 31_24
		} bit;
		UINT32 word;
	} reg_36; // 0x0090

	union {
		struct {
			unsigned edge_filt1_coeff8       : 8;		// bits : 7_0
			unsigned edge_filt1_coeff9       : 8;		// bits : 15_8
			unsigned edge_filt1_coeff10      : 8;		// bits : 23_16
			unsigned edge_filt1_coeff11      : 8;		// bits : 31_24
		} bit;
		UINT32 word;
	} reg_37; // 0x0094

	union {
		struct {
			unsigned edge_filt1_coeff12      : 8;		// bits : 7_0
			unsigned edge_filt1_coeff13      : 8;		// bits : 15_8
			unsigned edge_filt1_coeff14      : 8;		// bits : 23_16
			unsigned edge_filt1_coeff15      : 8;		// bits : 31_24
		} bit;
		UINT32 word;
	} reg_38; // 0x0098

	union {
		struct {
			unsigned edge_filt1_coeff16      : 8;		// bits : 7_0
			unsigned edge_filt1_coeff17      : 8;		// bits : 15_8
			unsigned edge_filt1_coeff18      : 8;		// bits : 23_16
			unsigned edge_filt1_coeff19      : 8;		// bits : 31_24
		} bit;
		UINT32 word;
	} reg_39; // 0x009c

	union {
		struct {
			unsigned edge_filt1_coeff20      : 8;		// bits : 7_0
			unsigned edge_filt1_coeff21      : 8;		// bits : 15_8
			unsigned edge_filt1_coeff22      : 8;		// bits : 23_16
			unsigned edge_filt1_coeff23      : 8;		// bits : 31_24
		} bit;
		UINT32 word;
	} reg_40; // 0x00a0

	union {
		struct {
			unsigned edge_filt1_coeff24           : 8;  // bits : 7_0
			unsigned                              : 8;
			unsigned edge_alpha_blending_factor   : 8;  // bits : 23_16
			unsigned                              : 8;
		} bit;
		UINT32 word;
	} reg_41; // 0x00a4

	union {
		struct {
			unsigned thres_mode           : 6;		// bits : 0_5
			unsigned                      : 26;
		} bit;
		UINT32 word;
	} reg_42; // 0x00a8

	union {
		struct {
			unsigned thres_low_th        : 16;		// bits : 15_0
			unsigned                     : 16;
		} bit;
		UINT32 word;
	} reg_43; // 0x00ac

	union {
		struct {
			unsigned thres_high_th        : 16;		// bits : 15_0
			unsigned                      : 16;
		} bit;
		UINT32 word;
	} reg_44; // 0x00b0

	union {
		struct {
			unsigned thres_out_min_val    : 8;		// bits : 7_0
			unsigned                      : 24;
		} bit;
		UINT32 word;
	} reg_45; // 0x00b4

	union {
		struct {
			unsigned thres_out_mid_val    : 8;		// bits : 7_0
			unsigned                      : 24;
		} bit;
		UINT32 word;
	} reg_46; // 0x00b8

	union {
		struct {
			unsigned thres_out_max_val    : 8;		// bits : 7_0
			unsigned                      : 24;
		} bit;
		UINT32 word;
	} reg_47; // 0x00bc

	union {
		struct {
			unsigned postproc_mode          : 1;		// bits : 0
			unsigned                        : 3;
			unsigned postproc_morph_op      : 1;		// bits : 4
			unsigned                        : 3;
			unsigned postproc_morph_mask_en : 24;		// bits : 31_8
		} bit;
		UINT32 word;
	} reg_48; // 0x00c0

	union {
		struct {
			unsigned postproc_hyst_low_th  : 8;		// bits : 7_0
			unsigned                       : 8;		
			unsigned postproc_hyst_high_th : 8;		// bits : 23_16
			unsigned                       : 8;	
		} bit;
		UINT32 word;
	} reg_49; // 0x00c4

	union {
		struct {
			unsigned integral_in_fmt            : 2;		// bits : 1_0
			unsigned                            : 2;
			unsigned integral_out_fmt           : 2;		// bits : 5_4
			unsigned                            : 2;
			unsigned                            : 24;	
		} bit;
		UINT32 word;
	} reg_50; // 0x00c8

	union {
		struct {
			
			unsigned                     : 32;
		} bit;
		UINT32 word;
	} reg_51; // 0x00cc

	union {
		struct {
			unsigned                       : 32;
		} bit;
		UINT32 word;
	} reg_52; // 0x00d0

	union {
		struct {
			unsigned                       : 32;
		} bit;
		UINT32 word;
	} reg_53; // 0x00d4

	union {
		struct {
			unsigned                       : 32;
		} bit;
		UINT32 word;
	} reg_54; // 0x00d8

	union {
		struct {
			unsigned                       : 32;
		} bit;
		UINT32 word;
	} reg_55; // 0x00dc

	union {
		struct {
			unsigned                       : 32;
		} bit;
		UINT32 word;
	} reg_56; // 0x00e0

	union {
		struct {
			unsigned                       : 32;
		} bit;
		UINT32 word;
	} reg_57; // 0x00e4

	union {
		struct {
			unsigned                       : 32;
		} bit;
		UINT32 word;
	} reg_58; // 0x00e8

	union {
		struct {
			unsigned                       : 32;
		} bit;
		UINT32 word;
	} reg_59; // 0x00ec

	union {
		struct {
			unsigned                       : 32;
		} bit;
		UINT32 word;
	} reg_60; // 0x00f0

	union {
		struct {
			unsigned                       : 32;
		} bit;
		UINT32 word;
	} reg_61; // 0x00f4

	union {
		struct {
			unsigned                       : 32;
		} bit;
		UINT32 word;
	} reg_62; // 0x00f8

	union {
		struct {
			unsigned                       : 32;
		} bit;
		UINT32 word;
	} reg_63; // 0x00fc

	union {
		struct {
			unsigned in0_ch_en         : 1;      // bits : 0
			unsigned in1_ch_en         : 1;      // bits : 1
			unsigned out0_ch_en        : 1;      // bits : 2
			unsigned                   : 5;        // bits : 7_5
			unsigned in0_lock_dis      : 1;      // bits : 8
			unsigned in1_lock_dis      : 1;      // bits : 9
			unsigned out0_lock_dis     : 1;      // bits : 10
			unsigned                   : 5;      // bits : 15_13
			unsigned write_ch_outstand_num : 8;      // bits : 23_16
			unsigned read_ch_outstand_num : 8;      // bits : 31_24
		} bit;
		UINT32 word;
	} reg_64; // 0x0100

	union {
		struct {
			unsigned axi_bus_disable   : 1;      // bits : 0
			unsigned                   : 26;        // bits : 26_1
			unsigned axi_bus_idle      : 1;      // bits : 27
			unsigned                   : 4;        // bits : 31_28
		} bit;
		UINT32 word;
	} reg_65; // 0x0104

	union {
		struct {
			unsigned                      : 32;
		} bit;
		UINT32 word;
	} reg_66; // 0x0108

	union {
		struct {
			unsigned                      : 32;
		} bit;
		UINT32 word;
	} reg_67; // 0x010c

	union {
		struct {
			unsigned                      : 32;
		} bit;
		UINT32 word;
	} reg_68; // 0x0110

	union {
		struct {
			unsigned                      : 32;
		} bit;
		UINT32 word;
	} reg_69; // 0x0114

	union {
		struct {
			unsigned                      : 32;
		} bit;
		UINT32 word;
	} reg_70; // 0x0118

	union {
		struct {
			unsigned                      : 32;
		} bit;
		UINT32 word;
	} reg_71; // 0x011c

	union {
		struct {
			unsigned                      : 32;
		} bit;
		UINT32 word;
	} reg_72; // 0x0120

	union {
		struct {
			unsigned                      : 32;
		} bit;
		UINT32 word;
	} reg_73; // 0x0124

	union {
		struct {
			unsigned                      : 32;
		} bit;
		UINT32 word;
	} reg_74; // 0x0128

	union {
		struct {
			unsigned                      : 32;
		} bit;
		UINT32 word;
	} reg_75; // 0x012c

	union {
		struct {
			unsigned                      : 32;
		} bit;
		UINT32 word;
	} reg_76; // 0x0130

	union {
		struct {
			unsigned                      : 32;
		} bit;
		UINT32 word;
	} reg_77; // 0x0134

	union {
		struct {
			unsigned                      : 32;
		} bit;
		UINT32 word;
	} reg_78; // 0x0138

	union {
		struct {
			unsigned                      : 32;
		} bit;
		UINT32 word;
	} reg_79; // 0x013c

	union {
		struct {
			unsigned                      : 32;
		} bit;
		UINT32 word;
	} reg_80; // 0x0140

	union {
		struct {
			unsigned                      : 32;
		} bit;
		UINT32 word;
	} reg_81; // 0x0144

	union {
		struct {
			unsigned                      : 32;
		} bit;
		UINT32 word;
	} reg_82; // 0x0148

	union {
		struct {
			unsigned                      : 32;
		} bit;
		UINT32 word;
	} reg_83; // 0x014c

	union {
		struct {
			unsigned                      : 32;
		} bit;
		UINT32 word;
	} reg_84; // 0x0150

	union {
		struct {
			unsigned                      : 32;
		} bit;
		UINT32 word;
	} reg_85; // 0x0154

	union {
		struct {
			unsigned                      : 32;
		} bit;
		UINT32 word;
	} reg_86; // 0x0158

	union {
		struct {
			unsigned                      : 32;
		} bit;
		UINT32 word;
	} reg_87; // 0x015c

	union {
		struct {
			unsigned hist_mode     : 2;		// bits : 1_0
			unsigned               : 30;
		} bit;
		UINT32 word;
	} reg_88; // 0x0160

	union {
		struct {
			unsigned hist_cdf_min     : 32;	// bits : 31_0
		} bit;
		UINT32 word;
	} reg_89; // 0x0164

	union {
		struct {
			unsigned ncc_numerator_lsb : 32;	// bits : 31_0
		} bit;
		UINT32 word;
	} reg_90; // 0x0168

	union {
		struct {
			unsigned ncc_numerator_msb : 32;	// bits : 31_0
		} bit;
		UINT32 word;
	} reg_91; // 0x016c

	union {
		struct {
			unsigned ncc_quad_sum0_lsb : 32;	// bits : 31_0
		} bit;
		UINT32 word;
	} reg_92; // 0x0170

	union {
		struct {
			unsigned ncc_quad_sum0_msb : 32;	// bits : 31_0
		} bit;
		UINT32 word;
	} reg_93; // 0x0174

	union {
		struct {
			unsigned ncc_quad_sum1_lsb : 32;	// bits : 31_0
		} bit;
		UINT32 word;
	} reg_94; // 0x0178

	union {
		struct {
			unsigned ncc_quad_sum1_msb : 32;	// bits : 31_0
		} bit;
		UINT32 word;
	} reg_95; // 0x017c

	union {
		struct {
			unsigned dma_sobel_out_sel  : 1;
			unsigned dma_in_fmt 	 : 2;
			unsigned                 : 1;
			unsigned dma_hor_seg_sz  : 5;		// bits : 8_4
			unsigned                 : 7;
			unsigned dma_ver_seg_sz  : 13;	    // bits : 28_16
			unsigned                 : 3;
		} bit;
		UINT32 word;
	} reg_96; // 0x0180

	union {
		struct {
			unsigned dma_element_sz  : 5;		// bits : 4_0
			unsigned                 : 11;
			unsigned dma_ver_seg_num : 13;		// bits : 28_16
			unsigned                 : 3;
		} bit;
		UINT32 word;
	} reg_97; // 0x0184

	union {
		struct {
			unsigned imgop_mode      : 3;		// bits : 2_0
			unsigned                 : 1;
			unsigned imgop_sub_mode  : 1;		// bits : 4
			unsigned                 : 27;
		} bit;
		UINT32 word;
	} reg_98; // 0x0188

	union {
		struct {
			unsigned bit16_to_bit8_mode : 3;		// bits : 2_0
			unsigned                    : 29;
		} bit;
		UINT32 word;
	} reg_99; // 0x018c

	union {
		struct {
			unsigned op_coef_a : 32;		// bits : 31_0
		} bit;
		UINT32 word;
	} reg_100; // 0x0190

	union {
		struct {
			unsigned op_coef_b : 32;		// bits : 31_0
		} bit;
		UINT32 word;
	} reg_101; // 0x0194

	union {
		struct {
			unsigned op_coef_bias : 8;		// bits : 7_0
			unsigned              : 24;
		} bit;
		UINT32 word;
	} reg_102; // 0x0198

	union {
		struct {
			unsigned op_shift_bit : 5;		// bits : 4_0
			unsigned              : 27;
		} bit;
		UINT32 word;
	} reg_103; // 0x019c

	union {
		struct {			
			unsigned                    : 32;			
		} bit;
		UINT32 word;
	} reg_104; // 0x01a0

	union {
		struct {			
			unsigned                   : 32;
		} bit;
		UINT32 word;
	} reg_105; // 0x01a4

	union {
		struct {
			unsigned st_mode           : 1;			// bits : 0
			unsigned                   : 3;
			unsigned st_threshold_mode : 1;			// bits : 0
			unsigned                   : 27;
		} bit;
		UINT32 word;
	} reg_106; // 0x01a8

	union {
		struct {
			unsigned st_quality_level  : 8;			// bits : 7_0
			unsigned 			       : 8;			// bits : 15_8
			unsigned st_blk_dist       : 8;			// bits : 23_16
			unsigned                   : 8;
		} bit;
		UINT32 word;
	} reg_107; // 0x01ac

	union {
		struct {
			unsigned st_stripe_blk_x_num  : 10;		// bits : 9_0
			unsigned                      : 22;
		} bit;
		UINT32 word;
	} reg_108; // 0x01b0

	union {
		struct {
			unsigned st_blk_x_num      : 10;		// bits : 9_0
			unsigned                   : 6;
			unsigned st_blk_y_num      : 10;		// bits : 25_16
			unsigned                   : 6;
		} bit;
		UINT32 word;
	} reg_109; // 0x01b4

	union {
		struct {
			unsigned st_first_stage_point_num : 20;  // bits : 19_0
			unsigned                          : 12;
		} bit;
		UINT32 word;
	} reg_110; // 0x01b8

	union {
		struct {
			unsigned st_threshold_val : 24;  // bits : 19_0
			unsigned                  : 8;
		} bit;
		UINT32 word;
	} reg_111; // 0x01bc

	union {
		struct {
			unsigned                    : 32;
		} bit;
		UINT32 word;
	} reg_112; // 0x01c0

	union {
		struct {
			unsigned lbp_cmp_mode  : 1;			// bits : 0
			unsigned               : 15;
			unsigned lbp_threshold : 8;			// bits : 23_16
			unsigned               : 8;
		} bit;
		UINT32 word;
	} reg_113; // 0x01c4

	union {
		struct {
			unsigned                 : 32;
		} bit;
		UINT32 word;
	} reg_114; // 0x01c8

	union {
		struct {
			unsigned map_mode        : 1;		// bits : 0
			unsigned                 : 7;
			unsigned map_index_shift : 4;		// bits : 11_8
			unsigned                 : 20;
		} bit;
		UINT32 word;
	} reg_115; // 0x01cc
} NT98538_IVE_REGISTER_STRUCT;







#ifdef __cplusplus
}
#endif


#endif

