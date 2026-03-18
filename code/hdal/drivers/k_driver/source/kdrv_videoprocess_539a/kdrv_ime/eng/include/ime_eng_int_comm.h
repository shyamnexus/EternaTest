#ifndef _IME_ENG_INT_COMM_H_
#define _IME_ENG_INT_COMM_H_

#ifdef __cplusplus
extern "C" {
#endif


#ifdef __KERNEL__
#include <linux/version.h>


#include "kwrap/type.h"
//#include <mach/rcw_macro.h>

#if (LINUX_VERSION_CODE > KERNEL_VERSION(5, 10, 0))
#include <linux/soc/nvt/rcw_macro.h>
#else
#include "mach/rcw_macro.h"
#endif

#include "kwrap/error_no.h"
#elif defined(__FREERTOS)
#include "kwrap/type.h"
#include "rcw_macro.h"
#include "kwrap/error_no.h"
#else
#endif


#define IME_GET_API_EN    0


#if 0
#define IME_ALIGN_FLOOR(value, base)  ((value) & ~((base)-1))                   ///< Align Floor
#define IME_ALIGN_ROUND(value, base)  IME_ALIGN_FLOOR((value) + ((base)/2), base)   ///< Align Round
#define IME_ALIGN_CEIL(value, base)   IME_ALIGN_FLOOR((value) + ((base)-1), base)   ///< Align Ceil

#define IME_ALIGN_ROUND_64(a)       IME_ALIGN_ROUND(a, 64)  ///< Round Off to 64
#define IME_ALIGN_ROUND_32(a)       IME_ALIGN_ROUND(a, 32)  ///< Round Off to 32
#define IME_ALIGN_ROUND_16(a)       IME_ALIGN_ROUND(a, 16)  ///< Round Off to 16
#define IME_ALIGN_ROUND_8(a)        IME_ALIGN_ROUND(a, 8)   ///< Round Off to 8
#define IME_ALIGN_ROUND_4(a)        IME_ALIGN_ROUND(a, 4)   ///< Round Off to 4

#define IME_ALIGN_CEIL_64(a)        IME_ALIGN_CEIL(a, 64)   ///< Round Up to 64
#define IME_ALIGN_CEIL_32(a)        IME_ALIGN_CEIL(a, 32)   ///< Round Up to 32
#define IME_ALIGN_CEIL_16(a)        IME_ALIGN_CEIL(a, 16)   ///< Round Up to 16
#define IME_ALIGN_CEIL_8(a)         IME_ALIGN_CEIL(a, 8)    ///< Round Up to 8
#define IME_ALIGN_CEIL_4(a)         IME_ALIGN_CEIL(a, 4)    ///< Round Up to 4

#define IME_ALIGN_FLOOR_64(a)       IME_ALIGN_FLOOR(a, 64)  ///< Round down to 64
#define IME_ALIGN_FLOOR_32(a)       IME_ALIGN_FLOOR(a, 32)  ///< Round down to 32
#define IME_ALIGN_FLOOR_16(a)       IME_ALIGN_FLOOR(a, 16)  ///< Round down to 16
#define IME_ALIGN_FLOOR_8(a)        IME_ALIGN_FLOOR(a, 8)   ///< Round down to 8
#define IME_ALIGN_FLOOR_4(a)        IME_ALIGN_FLOOR(a, 4)   ///< Round down to 4
#else
#define IME_ALIGN_ROUND_128(a)      (((a) + 0x40) & ~0x7F)
#define IME_ALIGN_ROUND_64(a)       (((a) + 0x20) & ~0x3F)
#define IME_ALIGN_ROUND_32(a)       (((a) + 0x10) & ~0x1F)
#define IME_ALIGN_ROUND_16(a)       (((a) + 0x08) & ~0x0F)
#define IME_ALIGN_ROUND_8(a)        (((a) + 0x04) & ~0x07)
#define IME_ALIGN_ROUND_4(a)        (((a) + 0x02) & ~0x03)
#define IME_ALIGN_ROUND_2(a)        (((a) + 0x01) & ~0x01)

#define IME_ALIGN_CEIL_128(a)       (((a) + 0x7F) & ~0x7F)
#define IME_ALIGN_CEIL_64(a)        (((a) + 0x3F) & ~0x3F)
#define IME_ALIGN_CEIL_32(a)        (((a) + 0x1F) & ~0x1F)
#define IME_ALIGN_CEIL_16(a)        (((a) + 0x0F) & ~0x0F)
#define IME_ALIGN_CEIL_8(a)         (((a) + 0x07) & ~0x07)
#define IME_ALIGN_CEIL_4(a)         (((a) + 0x03) & ~0x03)
#define IME_ALIGN_CEIL_2(a)         (((a) + 0x01) & ~0x01)

#define IME_ALIGN_FLOOR_128(a)      ((a) & ~0x7F)
#define IME_ALIGN_FLOOR_64(a)       ((a) & ~0x3F)
#define IME_ALIGN_FLOOR_32(a)       ((a) & ~0x1F)
#define IME_ALIGN_FLOOR_16(a)       ((a) & ~0x0F)
#define IME_ALIGN_FLOOR_8(a)        ((a) & ~0x07)
#define IME_ALIGN_FLOOR_4(a)        ((a) & ~0x03)
#define IME_ALIGN_FLOOR_2(a)        ((a) & ~0x01)

#endif


/**
    IME enum - function enalbe/disable
*/
typedef enum _IME_FUNC_EN {
	IME_FUNC_DISABLE = 0,   ///< function disable
	IME_FUNC_ENABLE  = 1,  ///< function enable
	ENUM_DUMMY4WORD(IME_FUNC_EN)
} IME_FUNC_EN;




#ifdef __cplusplus
}
#endif

#endif

