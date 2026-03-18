#ifndef _VOS_LIBC_STDIO_H_
#define _VOS_LIBC_STDIO_H_

#if defined(__LINUX) && defined(__KERNEL__)
#include <linux/version.h>
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 15, 0))
#define VOS_LINUX_OWN_STDARG 1
#else
#define VOS_LINUX_OWN_STDARG 0
#endif
#else //not Linux kernel
#define VOS_LINUX_OWN_STDARG 0
#endif

#if VOS_LINUX_OWN_STDARG
#include <linux/stdarg.h>
#else
#include <stdarg.h>
#endif

extern int  sscanf_s(const char *buf, const char *fmt, ...);
extern int  vsscanf_s(const char *buf, const char *fmt, va_list args);

#endif
