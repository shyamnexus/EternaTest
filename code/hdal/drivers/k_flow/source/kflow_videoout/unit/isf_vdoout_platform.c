#include "isf_vdoout_platform.h"
#include <linux/slab.h>
#include "isf_vdoout_dbg.h"
#include "isf_vdoout_int.h"

//#ifdef DEBUG
module_param_named(isf_vdoout_debug_level, isf_vdoout_debug_level, int, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(isf_vdoout_debug_level, "vdoout debug level");
//#endif

void* vdoout_alloc(int size)
{
	return kmalloc(size, GFP_ATOMIC);
}

void vdoout_free(void *buf)
{
	kfree(buf);
}