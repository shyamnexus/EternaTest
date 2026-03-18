#ifndef __HASH_PLATFORM_H_
#define __HASH_PLATFORM_H_

#include <kwrap/flag.h>
#include <kwrap/nvt_type.h>
#include <kwrap/error_no.h>

/**********************************************************************************************
 * Public Function Prototype
 **********************************************************************************************/
extern void hash_platform_create_resource(void);
extern void hash_platform_release_resource(void);
extern ER   hash_platform_sem_wait(void);
extern ER   hash_platform_sem_signal(void);

#endif  /* __HASH_PLATFORM_H_ */
