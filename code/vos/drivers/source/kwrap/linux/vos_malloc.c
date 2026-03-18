/*-----------------------------------------------------------------------------*/
/* Include Header Files                                                        */
/*-----------------------------------------------------------------------------*/
#include <linux/slab.h>
#include <linux/vmalloc.h>

#include <kwrap/debug.h>
#include <kwrap/malloc.h>

/*-----------------------------------------------------------------------------*/
/* Local Types Declarations                                                    */
/*-----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------*/
/* Local Global Variables                                                      */
/*-----------------------------------------------------------------------------*/
STATIC_ASSERT(sizeof(vk_gfp_t) == sizeof(gfp_t));

/*-----------------------------------------------------------------------------*/
/* Interface Functions                                                         */
/*-----------------------------------------------------------------------------*/
void *vk_kzalloc(size_t size, vk_gfp_t flags)
{
	return kzalloc(size, flags);
}
EXPORT_SYMBOL(vk_kzalloc);

void *vk_kmalloc(size_t size, vk_gfp_t flags)
{
	return kmalloc(size, flags);
}
EXPORT_SYMBOL(vk_kmalloc);

void vk_kfree(const void *x)
{
	kfree(x);
}
EXPORT_SYMBOL(vk_kfree);

void *vk_vmalloc(unsigned long size)
{
	return vmalloc(size);
}
EXPORT_SYMBOL(vk_vmalloc);

void vk_vfree(const void *addr)
{
	vfree(addr);
}
EXPORT_SYMBOL(vk_vfree);