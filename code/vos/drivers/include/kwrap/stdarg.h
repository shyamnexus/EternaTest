#ifndef _VOS_STDARG_H_
#define _VOS_STDARG_H_

#if defined(__LINUX) && defined(__KERNEL__)

#include <linux/version.h>

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 15, 0)
#include <linux/stdarg.h>
#else
#include <stdarg.h>
#endif

#endif

#endif /* _VOS_STDARG_H_ */

