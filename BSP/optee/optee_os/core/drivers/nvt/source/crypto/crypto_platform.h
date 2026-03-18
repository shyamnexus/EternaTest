#ifndef __CRYPTO_PLATFORM_H_
#define __CRYPTO_PLATFORM_H__

#include <kwrap/flag.h>
#include <kwrap/nvt_type.h>
#include <kwrap/error_no.h>

/**********************************************************************************************
 * Public Function Prototype
 **********************************************************************************************/
extern void crypto_platform_create_resource(void);
extern void crypto_platform_release_resource(void);
extern ER   crypto_platform_sem_wait(void);
extern ER   crypto_platform_sem_signal(void);

#endif  /* __CRYPTO_PLATFORM_H__ */
