#ifndef _VOS_KER_PLATFORM_H_
#define _VOS_KER_PLATFORM_H_

#define _ALIGNED(x) __attribute__((aligned(x)))
#define _PACKED_BEGIN
#define _PACKED_END __attribute__ ((packed))
#define _INLINE static inline
#define _ASM_NOP __asm__("nop");
#define _SECTION(sec)

//BSP identifier (only used by linux kernel)
#if defined(__LINUX) && defined(__KERNEL__)
#if defined(CONFIG_NVT_IVOT_PLAT_NA51055)
#define _BSP_NA51055_
#elif defined(CONFIG_NVT_IVOT_PLAT_NA51068)
#define _BSP_NA51068_
#elif defined(CONFIG_NVT_IVOT_PLAT_NA51089)
#define _BSP_NA51089_
#elif defined(CONFIG_NVT_IVOT_PLAT_NA51090)
#define _BSP_NA51090_
#elif defined(CONFIG_NVT_IVOT_PLAT_NA51102)
#define _BSP_NA51102_
#elif defined(CONFIG_NVT_IVOT_PLAT_NA51103)
#define _BSP_NA51103_
#elif defined(CONFIG_NVT_IVOT_PLAT_NS02201)
#define _BSP_NS02201_
#elif defined(CONFIG_NVT_IVOT_PLAT_NS02301)
#define _BSP_NS02301_
#elif defined(CONFIG_NVT_IVOT_PLAT_NS02302)
#define _BSP_NS02302_
#elif defined(CONFIG_NVT_IVOT_PLAT_NS02401)
#define _BSP_NS02401_
#else
#error Unknown platform (VOS kernel-space)
//VOS kernel-space header: code/vos/drivers/include/kwrap/platform.h
//VOS user-space header: code/vos/include/kwrap/platform.h
#endif
#endif

//todo identifier
#define _TODO           0

#endif //_VOS_KER_PLATFORM_H_
