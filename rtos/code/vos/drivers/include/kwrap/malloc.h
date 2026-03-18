#ifndef _VOS_MALLOC_H_
#define _VOS_MALLOC_H_
#ifdef __cplusplus
extern "C" {
#endif

/*-----------------------------------------------------------------------------*/
/* Including Files                                                             */
/*-----------------------------------------------------------------------------*/
#include <kwrap/nvt_type.h>


#if (defined(__UITRON) || defined(__ECOS))
#elif defined(__FREERTOS)
#elif defined(__LINUX) && defined(__KERNEL__)

/*----------------------------------------------------------------------------*/
/* Type declaration (Linux)                                                   */
/*----------------------------------------------------------------------------*/
typedef unsigned vk_gfp_t;

/*----------------------------------------------------------------------------*/
/* Normal type API (Linux)                                                    */
/*----------------------------------------------------------------------------*/
void *vk_kzalloc(size_t size, vk_gfp_t flags);
void *vk_kmalloc(size_t size, vk_gfp_t flags);
void vk_kfree(const void *x);
void *vk_vmalloc(unsigned long size);
void vk_vfree(const void *addr);

#else
#error Not supported OS
#endif

#ifdef __cplusplus
}
#endif

#endif /* _VOS_MALLOC_H_ */

