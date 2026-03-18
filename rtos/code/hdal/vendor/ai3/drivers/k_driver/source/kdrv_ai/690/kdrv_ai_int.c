#include "kdrv_ai_int.h"

#if defined(__FREERTOS)
#include "kwrap/debug.h"
#include <string.h>
#include "efuse_protected.h"
#else
#include "kdrv_ai_dbg.h"
#include <linux/string.h>
#include <plat/efuse_protected.h>
#endif

#include "kwrap/error_no.h"

//TODO

