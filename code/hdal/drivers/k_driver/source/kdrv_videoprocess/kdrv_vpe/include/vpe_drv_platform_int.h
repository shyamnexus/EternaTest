#ifndef _VPE_DRV_PLATFORM_INT_H_
#define _VPE_DRV_PLATFORM_INT_H_
#include <kwrap/type.h>
#include <kwrap/nvt_type.h>
#include <kwrap/list.h>
#include <kwrap/util.h>
#include <kwrap/spinlock.h>
#if defined(__LINUX)
#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/string.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/slab.h>
#include <linux/clk.h>
#include <linux/clk-provider.h>
#include <linux/string.h>
#include <linux/soc/nvt/nvt-pcie-lib.h>
#else
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "io_address.h"
#include "interrupt.h"
#include "comm/compiler.h"
#include "comm/libfdt.h"
#include <plat/rtosfdt.h>

typedef INT8 s8;
typedef INT16 s16;
typedef INT32 s32;
typedef INT64 s64;
typedef UINT8 u8;
typedef UINT16 u16;
typedef UINT32 u32;
typedef UINT64 u64;

typedef s32 atomic_t;

typedef UINT32 nvtpcie_chipid_t;

#ifndef CHIP_RC
#define CHIP_RC 0
#endif

#ifndef ENOMEM
#define ENOMEM -1
#endif
#endif
#endif //_VPE_DRV_PLATFORM_INT_H_
