#if defined(__FREERTOS) || defined(__OPTEEOS)
#if defined(_BSP_NA51102_)
#include "interrupt_na51102.h"
#elif defined(_BSP_NA51103_)
#include "interrupt_na51103.h"
#elif defined(_BSP_NA51090_)
#include "interrupt_na51090.h"
#elif defined(_BSP_NS02201_)
#include "interrupt_ns02201.h"
#elif defined(_BSP_NA51089_) || defined(_BSP_NA51055_)
#include "interrupt_na51089.h"
#elif defined(_BSP_NS02301_)
#include "interrupt_ns02301.h"
#elif defined(_BSP_NS02302_)
#include "interrupt_ns02302.h"
#elif defined(_BSP_NS02401_)
#include "interrupt_ns02401.h"
#endif
#endif
