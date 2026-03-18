#ifndef _KDRV_BUILTIN_DEBUG_H
#define _KDRV_BUILTIN_DEBUG_H

// only Linux kdrv_builtin use pr_err instead of vk_print
#if defined(__KERNEL__) && defined(_KDRV_BUILTIN_)
#undef DBG_ERR
#define DBG_ERR(fmt, args...) pr_err("%s:" fmt, __func__, ##args)
#undef  NVT_PRINT_ERR
#define NVT_PRINT_ERR(fmt, args...) pr_err(fmt, ##args)
#endif

#endif