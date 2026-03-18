#ifndef __RSA_PLATFORM_H_
#define __RSA_PLATFORM_H__

#include <kwrap/flag.h>
#include <kwrap/nvt_type.h>
#include <kwrap/error_no.h>

/**********************************************************************************************
 * Public Function Prototype
 **********************************************************************************************/
extern void rsa_platform_create_resource(void);
extern void rsa_platform_release_resource(void);
extern ER   rsa_platform_sem_wait(void);
extern ER   rsa_platform_sem_signal(void);

#endif  /* __RSA_PLATFORM_H__ */
