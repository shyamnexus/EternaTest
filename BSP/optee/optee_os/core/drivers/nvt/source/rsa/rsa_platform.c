#include <platform_config.h>
#include <kernel/mutex.h>
#include "rsa_platform.h"

static int is_create = 0;
static struct mutex SEMID_RSA;

void rsa_platform_create_resource(void)
{
	if (!is_create) {
		mutex_init(&SEMID_RSA);
		is_create = 1;
	}
}

void rsa_platform_release_resource(void)
{
	if (is_create) {
		mutex_destroy(&SEMID_RSA);
		is_create = 0;
	}
}

ER rsa_platform_sem_wait(void)
{
	mutex_lock(&SEMID_RSA);
	return E_OK;
}

ER rsa_platform_sem_signal(void)
{
	mutex_unlock(&SEMID_RSA);
	return E_OK;
}
