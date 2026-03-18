#ifndef __NVT_PINMUX_H__
#define __NVT_PINMUX_H__

#if defined(CONFIG_PINCTRL_NA51102)
#include "na51102/na51102_pinmux.h"
#elif defined(CONFIG_PINCTRL_NS02201)
#include "ns02201/ns02201_pinmux.h"
#elif defined(CONFIG_PINCTRL_NA51055)
#include "na51055/na51055_pinmux.h"
#elif defined(CONFIG_PINCTRL_NS02302)
#include "ns02302/ns02302_pinmux.h"
#elif defined(CONFIG_PINCTRL_NS02301)
#include "ns02301/ns02301_pinmux.h"
#elif defined(CONFIG_PINCTRL_NA51089)
#include "na51089/na51089_pinmux.h"
#elif defined(CONFIG_PINCTRL_NA51090)
#include "na51090/na51090_pinmux.h"
#elif defined(CONFIG_PINCTRL_NA51103)
#include "na51103/na51103_pinmux.h"
#elif defined(CONFIG_PINCTRL_NS02401)
#include "ns02401/ns02401_pinmux.h"
#endif

#endif /* __NVT_PINMUX_H__ */
