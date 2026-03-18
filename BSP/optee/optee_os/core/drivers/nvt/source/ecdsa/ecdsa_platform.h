#ifndef __RSA_PLATFORM_H_
#define __RSA_PLATFORM_H__

extern void ecdsa_platform_create_resource(void);
extern void ecdsa_platform_release_resource(void);
extern ER   ecdsa_platform_sem_wait(void);
extern ER   ecdsa_platform_sem_signal(void);

#endif  /* __RSA_PLATFORM_H__ */
