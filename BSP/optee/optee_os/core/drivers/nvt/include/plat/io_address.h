#if defined(__FREERTOS) || defined(__OPTEEOS)
#if defined(_BSP_NA51102_)
#if defined(CFG_ARM64_core)
#include "io_address_na51102_a64.h"
#endif
#elif defined(_BSP_NA51103_)
#if defined(CFG_ARM32_core)
#include "io_address_na51103.h"
#else
#include "io_address_na51103_a64.h"
#endif
#elif defined(_BSP_NA51090_)
#if defined(CFG_ARM64_core)
#include "io_address_na51090_a64.h"
#endif
#elif defined(_BSP_NS02201_)
#if defined(CFG_ARM64_core)
#include "io_address_ns02201.h"
#endif
#elif defined(_BSP_NS02401_)
#if defined(CFG_ARM64_core)
#include "io_address_ns02401.h"
#endif
#elif defined(_BSP_NA51089_) || defined(_BSP_NA51055_)
#if defined(CFG_ARM32_core)
#include "io_address_na51089.h"
#endif
#elif defined(_BSP_NS02301_)
#if defined(CFG_ARM32_core)
#include "io_address_ns02301.h"
#endif
#elif defined(_BSP_NS02302_)
#if defined(CFG_ARM64_core)
#include "io_address_ns02302.h"
#endif
#endif
#endif
