#if defined(__FREERTOS)
#if defined(_BSP_NA51089_)
#include "crypto_protected_na51089.h"
#elif defined(_BSP_NA51000_)
#include "crypto_protected_na51000.h"
#elif defined(_BSP_NA51102_)
#include "crypto_protected_na51102.h"
#elif defined(_BSP_NS02302_)
#include "crypto_protected_ns02302.h"
#endif
#else
#if defined(_BSP_NA51089_)
//#include "efuse_protected_na51055.h"
#endif
#endif
